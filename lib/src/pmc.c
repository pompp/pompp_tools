/*
 * The PomPP software including libraries and tools is developed by the
 * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
 * research project supported by the JST, CREST research program.
 * The copyrights for code used under license from other parties are
 * included in the corresponding files.
 * 
 * Copyright (c) 2015, The PomPP research team.
 */

/* This file contains the codes to used the Performance Monitoring
 * Counters (PMCs) equipped on the Intel processors.
 *
 * Note that these codes don't care the overflow of the counter
 * because the bit length of each PMC has 48 bit which is enough
 * to count performance events over several days.
 * */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ric-def.h"
#include "msr.h"

#include "affinity.h"

#define IA32_PMC0	0x0C1
#define IA32_EVTSEL0	0x186
#define IA32_A_PMC0	0x4C1

#define IA32_PERF_STATUS	0x198
#define IA32_PERF_CTL		0x199

#define IA32_FPMC_PMC0		0x309
#define IA32_FPMC_CTR_CTRL	0x38D
#define IA32_PERF_GLOBAL_STATUS	0x38E
#define IA32_PERF_GLOBAL_CTRL	0x38F
#define IA32_PERF_GLOBAL_OVF_CTR 0x390

#define IA32_PERF_CAPABILITIES	0x345

#define ENABLE_PERF_MASK	0x00630000
#define DISABLE_PERF_MASK	0x00230000

#define OVF_CLEAR_MASK		0xC0000007000000FF
#define OVF_CLEAR_MASK2         0xC00000070000000F
#define ENABLE_GLOBAL_MASK	0x07000000FF
#define ENABLE_GLOBAL_MASK2     0x070000000F
#define DISABLE_GLOBAL_MASK	0x0000000000
#define ENABLE_FPMC_MASK	0x0777
#define DISABLE_FPMC_MASK	0x0000

/* =============================================================
 * Getting the basis information to use PMC
 * ============================================================= */
static int NGPMC = 0;	// # of general purpose PMC registors
static int NFPMC = 0;	// # of fixed function PCM registors

static void get_perf_info( const int flag ) {
    unsigned int eax, ebx, ecx, edx;
    unsigned int nperf, nwidth, np2, nw2, ver;
    get_cpuid( 0x0a, &eax, &ebx, &ecx, &edx );
    ver    = (0x00ff & eax );
    nperf  = (0x0000ff00 & eax) >> 8;
    nwidth = (0x00ff0000 & eax) >> 16;
    np2    = (0x001f & edx);
    nw2    = (0x1fe0 & edx)>>5;
    /* to reduce output
    if ( flag && FP_CONF != NULL ) {
	fprintf(FP_CONF,
		"##PMC --------- information of perfmon ---------\n");
	fprintf(FP_CONF,
		"##PMC version of arch. perf. mon          = %2d\n", ver );
	fprintf(FP_CONF,
		"##PMC # of performance monitor registers  = %2d\n",
		nperf );
	fprintf(FP_CONF,
		"##PMC bit width of each perf-mon register = %2d\n",
		nwidth);
	fprintf(FP_CONF,
		"##PMC # of fixed perf mon registers       = %2d\n", np2 );
	fprintf(FP_CONF,
		"##PMC bit width of fixed perf-mon register= %2d\n", nw2 );
	fprintf(FP_CONF, "##PMC core cycle            = %s\n",
		((0x01&ebx)==0 ? "YES" : "NO") );
	fprintf(FP_CONF, "##PMC instruction count     = %s\n",
		((0x02&ebx)==0 ? "YES" : "NO") );
	fprintf(FP_CONF, "##PMC reference cycle       = %s\n",
		((0x04&ebx)==0 ? "YES" : "NO") );
	fprintf(FP_CONF, "##PMC LLC reference         = %s\n",
		((0x08&ebx)==0 ? "YES" : "NO") );
	fprintf(FP_CONF, "##PMC LLC miss count        = %s\n",
		((0x10&ebx)==0 ? "YES" : "NO") );
	fprintf(FP_CONF, "##PMC Branch instruction    = %s\n",
		((0x20&ebx)==0 ? "YES" : "NO") );
	fprintf(FP_CONF, "##PMC Branch mispredictions = %s\n",
		((0x40&ebx)==0 ? "YES" : "NO") );
	fprintf(FP_CONF, "##PMC Invariant TSC support = %s\n",
		((0x0100&edx)!=0 ? "YES":"NO"));
	fprintf(FP_CONF,
		"##PMC ------------------------------------------\n");
    }
    */

    NGPMC   = nperf;
    NFPMC   = np2;
    // information of time stamp counter
    get_cpuid( 0x80000007, &eax, &ebx, &ecx, &edx );
}

/* ================================================================
 * ================================================================ */
static uint32_t *EVENT_NUM = NULL;
static uint32_t *UMASK     = NULL;

static char **GPMC_NAME   = NULL;
static char **FPMC_NAME    = NULL;

static uint64_t **GPMC_VAL   = NULL;
static uint64_t **FPMC_VAL   = NULL;

static uint64_t *SUMS = NULL;	// temporary array used totaling values

static int EVENT_MODE;

static int NPKGS = 0;	// # of CPUs (packages) used by this process
static int LPKG_ID = -1;// leading package ID used by this process
static int LCORE_ID = -1;// leading core ID used by this process
static int NCORES = 0;	// # of cores used by this process
static int NCORES_PER_PKG   = 0;// # of cores per package
static double DINV_NCPP;// reciprocal of the # of cores per package

/* Clear PMCs and setting their values to zeros */
static void PMC_Clear() {
    uint32_t reg;
    uint64_t data;
    int core_id, i, cid0, cid1;
    cid0 = LCORE_ID;
    cid1 = cid0 + NCORES;
    /* Setting for general purpose PMCs (GPMCs) */
    for ( core_id=cid0; core_id<cid1; core_id++ ) {
	// Make all PMCs disable via a control register
	reg  = (uint32_t)IA32_PERF_GLOBAL_CTRL;
	data = (uint64_t)DISABLE_GLOBAL_MASK;
	wrmsr( core_id, reg, data );
	for ( i=0; i<NGPMC; i++ ) {
	    // make the PMC disable via status registers for each PMC
	    reg = (uint32_t)(IA32_EVTSEL0 + i);
	    data = (uint64_t)DISABLE_PERF_MASK
		| (uint64_t)(UMASK[i]<<8) | (uint64_t)EVENT_NUM[i];
	    wrmsr( core_id, reg, data );
	    // write value 0 to PMC
	    reg = (uint32_t)(IA32_A_PMC0 + i );
	    data = (uint64_t)0x00;
	    wrmsr( core_id, reg, data );
	    // make the PMC enable via status registers for each PMC
	    reg = (uint32_t)(IA32_EVTSEL0 + i);
	    data = (uint64_t)ENABLE_PERF_MASK
		| (uint64_t)(UMASK[i]<<8) | (uint64_t)EVENT_NUM[i];
	    wrmsr( core_id, reg, data );
	}
	/* Setting for fixed purpose PMCs (FPMCs) */
	// Make all PMCs disable via a control register
	reg = (uint32_t)IA32_FPMC_CTR_CTRL;
	data = (uint64_t)DISABLE_FPMC_MASK;
	wrmsr( core_id, reg, data );
	// write value 0 to PMC
	for ( i=0; i<NFPMC; i++ ) {
	    reg = (uint32_t)(IA32_FPMC_PMC0+i);
	    data = (uint64_t)0x00;
	    wrmsr( core_id, reg, data );
	}
	// make the PMC enable via status registers for each PMC
	reg = (uint32_t)IA32_FPMC_CTR_CTRL;
	data = (uint64_t)ENABLE_FPMC_MASK;
	wrmsr( core_id, reg, data );
	// Clear the overflow flag
	reg = (uint32_t)IA32_PERF_GLOBAL_OVF_CTR;
	data =
	    (uint64_t)(NGPMC>4 ? OVF_CLEAR_MASK : OVF_CLEAR_MASK2 );
	wrmsr( core_id, reg, data );
	// Make all GPMCs and FPMCs enable via control registers
	reg = (uint32_t)IA32_PERF_GLOBAL_CTRL;
	data =
	    (uint64_t)(NGPMC>4 ? ENABLE_GLOBAL_MASK : ENABLE_GLOBAL_MASK2);
	wrmsr( core_id, reg, data );
    }
}

/* Finalize the use of PMCs */
static void PMC_Finalize() {
    int core_id, cid0, cid1;
    uint32_t reg;
    uint64_t data;
    dealloc_cmatrix( GPMC_NAME );
    dealloc_cmatrix( FPMC_NAME );
    dealloc_u64_matrix( GPMC_VAL );
    dealloc_u64_matrix( FPMC_VAL );
    if ( SUMS )      free( SUMS );
    if ( EVENT_NUM ) free( EVENT_NUM );
    if ( UMASK     ) free( UMASK     );
    GPMC_NAME = NULL;
    FPMC_NAME = NULL;
    GPMC_VAL  = NULL;
    FPMC_VAL  = NULL;
    SUMS      = NULL;
    EVENT_NUM = NULL;
    UMASK     = NULL;
    /* Make all GPMCs and FPMCs disable via control registers */
    cid0 = LCORE_ID;
    cid1 = cid0 + NCORES;
    for ( core_id=cid0; core_id<cid1; core_id++ ) {
	reg = (uint32_t)IA32_PERF_GLOBAL_CTRL;
	data = (uint64_t)DISABLE_GLOBAL_MASK;
	wrmsr( core_id, reg, data );
    }
    NGPMC = NFPMC = 0;
    //
    NPKGS          = 0;
    LPKG_ID        = -1;
    LCORE_ID       = -1;
    NCORES         = 0;
    NCORES_PER_PKG = 0;
}

/* Initialize the use of PMCs */ 
static int PMC_Init() {
    uint64_t data;
    int npkg_per_node, core_id, cid0, cid1;
    int myrank, nprocs, root=0, event_mode;
    char *p;
    static int called = false;
    if ( called ) return 0;

    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );

    if ( myrank == root ) {
	event_mode = 0;
	if ( (p=getenv("POMPP_EVENT_MODE")) ) {
	    event_mode = atoi( p );
	    if ( event_mode < 0 || event_mode > 1 ) event_mode = 0;
	}
    }
    MPI_Bcast( &event_mode, 1, MPI_INT, root, MPI_COMM_WORLD );
    NCORES_PER_PKG = MSR_Get_ncores_per_pkg();
    npkg_per_node = MSR_Get_npkgs_per_node();
    POMPP_Get_pkg_info( 0, &NPKGS, &LPKG_ID );
    DINV_NCPP = 1.e0 / (double)NCORES_PER_PKG;
    NCORES   = NPKGS * NCORES_PER_PKG;
    LCORE_ID = MSR_Get_lcore_id( LPKG_ID );
    get_perf_info( (myrank==root ));
    EVENT_MODE = event_mode;

    // enable performance monitor counters
    cid0 = LCORE_ID;
    cid1 = cid0 + NCORES;
    for ( core_id=cid0; core_id<cid1; core_id++ ) {
	data = (uint64_t)(0x07)<<8 | (uint64_t)(0x07)<<4 |
	    (uint64_t)(0x07);
	wrmsr( core_id, (uint32_t)IA32_FPMC_CTR_CTRL, data );
	data = (uint64_t)(0x07)<<32 |
	    (uint64_t)(NGPMC>4 ? 0xff : 0x0f);
	wrmsr( core_id, (uint32_t)IA32_PERF_GLOBAL_CTRL, data );
	data = (uint64_t)0x00;
	wrmsr( core_id, (uint32_t)IA32_PERF_GLOBAL_OVF_CTR, data );
    }
    // allocate needed memory to use general purpose PMCs
    EVENT_NUM  = (uint32_t*)malloc( sizeof(uint32_t) * NGPMC );
    UMASK      = (uint32_t*)malloc( sizeof(uint32_t) * NGPMC );
    GPMC_NAME  = alloc_cmatrix( NGPMC, MAXSTRLEN );
    FPMC_NAME  = alloc_cmatrix( NFPMC, MAXSTRLEN );
    GPMC_VAL   = alloc_u64_matrix( NCORES, NGPMC );
    FPMC_VAL   = alloc_u64_matrix( NCORES, NFPMC );
    SUMS       = (uint64_t*)malloc( sizeof(uint64_t) *
	    (NGPMC>NFPMC? NGPMC : NFPMC) );
    // Setting PMC events to be measured and their symbolic name
    strcpy( FPMC_NAME[0], "INST" );
    strcpy( FPMC_NAME[1], "CORE_CYC" );
    strcpy( FPMC_NAME[2], "REF_CYC" );
    EVENT_MODE = event_mode;
    if ( event_mode == 0 ) {
	// 0
	EVENT_NUM[0] = (uint32_t)0x2E;
	UMASK[0]     = (uint32_t)0x4F;
	strcpy( GPMC_NAME[0], "LLC_REF." );
	// 1
	EVENT_NUM[1] = (uint32_t)0x2E;
	UMASK[1]     = (uint32_t)0x41;
	strcpy( GPMC_NAME[1], "LLC_MISS" );
	// 2
	EVENT_NUM[2] = (uint32_t)0xD0;
	UMASK[2]     = (uint32_t)0x81;
	strcpy( GPMC_NAME[2], "LD_INST" );
	// 3
	EVENT_NUM[3] = (uint32_t)0xD0;
	UMASK[3]     = (uint32_t)0x82;
	strcpy( GPMC_NAME[3], "SR_INST" );
	if ( NGPMC > 4 ) {
	    // 4
	    EVENT_NUM[4] = (uint32_t)0x10;
	    UMASK[4]     = (uint32_t)0x80;
	    strcpy( GPMC_NAME[4], "SSE_SCD" );
	    // 5
	    EVENT_NUM[5] = (uint32_t)0x10;
	    UMASK[5]     = (uint32_t)0x10;
	    strcpy( GPMC_NAME[5], "SSE_PKD" );
	    // 6
	    EVENT_NUM[6] = (uint32_t)0x11;
	    UMASK[6]     = (uint32_t)0x02;
	    strcpy( GPMC_NAME[6], "256_PKD" );
	    /*// 7
	    EVENT_NUM[7] = (uint32_t)0xA2;
	    UMASK[7]     = (uint32_t)0x01;
	    strcpy( GPMC_NAME[7], "RES_STL" );*/
	    // 7
	    EVENT_NUM[7] = (uint32_t)0x87;
	    UMASK[7]     = (uint32_t)0x04;
	    strcpy( GPMC_NAME[7], "STLIQFUL" );
	}
    } else if ( event_mode == 1 ) {
	// 0
	EVENT_NUM[0] = (uint32_t)0xA2;
	UMASK[0]     = (uint32_t)0x01;
	strcpy( GPMC_NAME[0], "STL_ANY" );
	// 1
	EVENT_NUM[1] = (uint32_t)0xA2;
	UMASK[1]     = (uint32_t)0x02;
	strcpy( GPMC_NAME[1], "STL_LOAD" );
	// 2
	EVENT_NUM[2] = (uint32_t)0xA2;
	UMASK[2]     = (uint32_t)0x04;
	strcpy( GPMC_NAME[2], "STL_RSFULL" );
	// 3
	EVENT_NUM[3] = (uint32_t)0xA2;
	UMASK[3]     = (uint32_t)0x08;
	strcpy( GPMC_NAME[3], "STL_STORE" );
	if ( NGPMC > 4 ) {
	    // 4
	    EVENT_NUM[4] = (uint32_t)0xA2;
	    UMASK[4]     = (uint32_t)0x10;
	    strcpy( GPMC_NAME[4], "STL_ROBFULL" );
	    // 5
	    EVENT_NUM[5] = (uint32_t)0xA2;
	    UMASK[5]     = (uint32_t)0x20;
	    strcpy( GPMC_NAME[5], "STL_FPCW" );
	    // 6
	    EVENT_NUM[6] = (uint32_t)0xA2;
	    UMASK[6]     = (uint32_t)0x40;
	    strcpy( GPMC_NAME[6], "STL_MXCSR" );
	    // 7
	    EVENT_NUM[7] = (uint32_t)0xA2;
	    UMASK[7]     = (uint32_t)0x80;
	    strcpy( GPMC_NAME[7], "STL_OTHER" );
	}
    }
    PMC_Clear();
    atexit( PMC_Finalize );
    called = true;
    //
    if ( FP_CONF ) {
	fprintf(FP_CONF, "##PMC event_mode= %s\n",
		(event_mode==0? "performance" : "stall" ) );
    }

    return 0;
}

/* Getting the all PMC values for each core and assign values to
 * arrayes GPMC_VAL and FPMC_VAL.
 * GPMC_VAL[core_id][iev]
 * FPMC_VAL[core_id][iev]
 * */
static int pmc_read() {
    int core_id, i, cid0, cid1;
    uint32_t reg;
    uint64_t data;
    cid0 = LCORE_ID;
    cid1 = cid0 + NCORES;
    for ( core_id=cid0; core_id<cid1; core_id++ ) {
	for ( i=0; i<NGPMC; i++ ) {
	    reg = (uint32_t)(IA32_PMC0 + i);
	    rdmsr( core_id, reg, &data );
	    GPMC_VAL[core_id-cid0][i] =
		(uint64_t)0x0000FFFFFFFFFFFF & (uint64_t)data;
	}
	for ( i=0; i<NFPMC; i++ ) {
	    reg = (uint32_t)(IA32_FPMC_PMC0 + i);
	    rdmsr( core_id, reg, &data );
	    FPMC_VAL[core_id-cid0][i] =
		(uint64_t)0x0000FFFFFFFFFFFF & (uint64_t)data;
	}
    }
    return 0;
}

/* Getting the PMC values for all packages used by this MPI process.
 * These values are accumulated from the call of PMC_Init function,
 * and we assume they don't overflow.
 * Obtained values are the sum of all cores' in the package except
 * the clock cycles and stall cycles.
 * */
int PMC_Read( uint64_t **GPMC_PKG, uint64_t **FPMC_PKG ) {
    int ipkg, cid0, cid1, iev, ic;
    static int called = false;
    if ( called == false ) {
	PMC_Init();
	called = true;
    }
    pmc_read();
    // Find the sum of PMC values for all cores in a package
    for ( ipkg=0, cid0=0; ipkg<NPKGS; ipkg++, cid0+=NCORES_PER_PKG ) {
	cid1 = cid0 + NCORES_PER_PKG;
	// for GPMCs
	for ( iev=0; iev<NGPMC; iev++ ) SUMS[iev] = 0;
	for ( ic=cid0; ic<cid1; ic++ ) {
	    for ( iev=0; iev<NGPMC; iev++ ) SUMS[iev] += GPMC_VAL[ic][iev];
	}
	for ( iev=0; iev<NGPMC; iev++ ) GPMC_PKG[ipkg][iev] = SUMS[iev];
	// for FPMCs
	for ( iev=0; iev<NFPMC; iev++ ) SUMS[iev] = 0;
	for ( ic=cid0; ic<cid1; ic++ ) {
	    for ( iev=0; iev<NFPMC; iev++ ) SUMS[iev] += FPMC_VAL[ic][iev];
	}
	for ( iev=0; iev<NFPMC; iev++ ) FPMC_PKG[ipkg][iev] = SUMS[iev];
    }
    // Take average of clock cycles and stall cycle
    if ( EVENT_MODE == 0 && NGPMC>4 ) {
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) GPMC_PKG[ipkg][7] *= DINV_NCPP;
    } else if ( EVENT_MODE == 1 && NGPMC>4 ) {
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	    for ( iev=0; iev<NGPMC; iev++ )
		GPMC_PKG[ipkg][iev] *= DINV_NCPP;
	}
    }
    for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	FPMC_PKG[ipkg][1] *= DINV_NCPP;
	FPMC_PKG[ipkg][2] *= DINV_NCPP;
    }
    return 0;
}

/* Get # of GPMC and FPMC and their symbolic names */
int PMC_Get_data( int *ngpmc, int *nfixed,
	char ***gpmc_name, char ***fpmc_name ) {
    static int called = false;
    if ( called == false ) {
	PMC_Init();
	called = true;
    }
    *ngpmc  = NGPMC;
    *nfixed = NFPMC;
    *gpmc_name = GPMC_NAME;
    *fpmc_name = FPMC_NAME;
    return 0;
}

