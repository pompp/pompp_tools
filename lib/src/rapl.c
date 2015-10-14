/*
 * The PomPP software including libraries and tools is developed by the
 * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
 * research project supported by the JST, CREST research program.
 * The copyrights for code used under license from other parties are
 * included in the corresponding files.
 * 
 * Copyright (c) 2015, The PomPP research team.
 */

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ric-def.h"
#include "msr.h"
#include "etime.h"
#include "affinity.h"

#define MSR_PLATFORM_INFO	0xCE
/* ==== Running Average Power Limit (RAPL) ==== */
#define POWER_LIMIT	0x00
#define ENERGY_STATUS	0x01
#define POLICY		0x02
#define PERF_STATUS	0x03
#define POWER_INFO	0x04
#define MSR_PKG		0x610
#define MSR_DRAM	0x618
#define MSR_PP0		0x638
#define MSR_PP1		0x640

/* Sandy Bridge processor */
#define MSR_RAPL_POWER_UNIT	0x606

#define MSR_PKG_POWER_LIMIT	(MSR_PKG+POWER_LIMIT)
#define MSR_PKG_ENERGY_STATUS	(MSR_PKG+ENERGY_STATUS)
#define MSR_PKG_POWER_INFO	(MSR_PKG+POWER_INFO)

#define MSR_PP0_POWER_LIMIT	(MSR_PP0+POWER_LIMIT)
#define MSR_PP0_ENERGY_STATUS	(MSR_PP0+ENERGY_STATUS)
#define MSR_PP0_POLICY		(MSR_PP0+POLICY)
#define MSR_PP0_PERF_STATUS	(MSR_PP0+PERF_STATUS)

/* for 2nd gen. Core i7 */
#define MSR_PP1_POWER_LIMIT	(MSR_PP1+POWER_LIMIT)
#define MSR_PP1_ENERGY_STATUS	(MSR_PP1+ENERGY_STATUS)
#define MSR_PP1_POLICY		(MSR_PP1+POLICY)

/* for Xeon E5 */
#define MSR_PKG_PERF_STATUS	(MSR_PKG+PERF_STATUS)

#define MSR_DRAM_POWER_LIMIT	(MSR_DRAM+POWER_LIMIT)
#define MSR_DRAM_ENERGY_STATUS	(MSR_DRAM+ENERGY_STATUS)
#define MSR_DRAM_PERF_STATUS	(MSR_DRAM+PERF_STATUS)
#define MSR_DRAM_POWER_INFO	(MSR_DRAM+POWER_INFO)

#define IA32_PERF_STATUS	0x198

/* ------------------------------------------------
 * Setting for RAPL
 * ------------------------------------------------ */
typedef enum _dev_type_ { RAPL_PKG, RAPL_PP0, RAPL_DRAM } devtype;

static double _ESU_;	// energy status unit (in Joules)
static double _PU_;	// power status unit (in Watts)
static double _TU_;	// time unit (int sec.)
static int    _IESU_;
static int    _IPU_;
static int    _ITU_;

static double PKG_MAX_POWER;
static double PKG_MIN_POWER;
static double PKG_TSP;
static double PKG_MAX_TIME_WIN;

static double DRAM_MAX_POWER;
static double DRAM_MIN_POWER;
static double DRAM_TSP;
static double DRAM_MAX_TIME_WIN;

static uint32_t *ENERGY_PKG_PREV = NULL;
static uint32_t *ENERGY_PP0_PREV = NULL;
static uint32_t *ENERGY_DRAM_PREV = NULL;

static uint32_t *ENERGY_PKG_CUR = NULL;
static uint32_t *ENERGY_PP0_CUR = NULL;
static uint32_t *ENERGY_DRAM_CUR = NULL;

static double *PKG_CAP_PREV = NULL;
static double *DRAM_CAP_PREV = NULL;

static int NPKGS = 0;
static int LPKG_ID = -1;
static int LCORE_ID = -1;
static int NCORES = 0;

static uint64_t *ENERGY_PKG_TOTAL = NULL;
static uint64_t *ENERGY_PP0_TOTAL = NULL;
static uint64_t *ENERGY_DRAM_TOTAL = NULL;


/** Arrays and variables used in the energy profiling mode */
#define MAXITERA	20000
static int *NITERA = NULL;		// [pkg]
static uint64_t **DE_PKG = NULL;	// [pkg][itera]
static uint64_t **DE_PP0 = NULL;	// [pkg][itera]
static uint64_t **DE_DRAM = NULL;	// [pkg][itera]
static double  *ETIME = NULL;		// [itera]
static FILE **FPE = NULL;

/* Retrieve powers of CPU, cores (P0 plane), and DRAM from MSRs */
static int RAPL_Read_energy_counts( const int core_id,
	uint32_t *pkg_val, uint32_t *pp0_val, uint32_t *dram_val ) {
    uint64_t data;
    uint32_t *p;
    p = (uint32_t*)(&data);
    // PKG energy
    rdmsr( core_id, (uint32_t)MSR_PKG_ENERGY_STATUS, &data );
    *pkg_val = p[0];
    // PP0 energy
    rdmsr( core_id, (uint32_t)MSR_PP0_ENERGY_STATUS, &data );
    *pp0_val = p[0];
    // DRAM energy
    rdmsr( core_id, (uint32_t)MSR_DRAM_ENERGY_STATUS, &data );
    *dram_val = p[0];
    return 0;
}

/* Write the energy profile data to the file */
static void RAPL_Write_energy_profile( FILE *fp, const int nitera,
	const double etime[], const uint64_t de_pkg[],
	const uint64_t de_pp0[], const uint64_t de_dram[] ) {
    int itera;
    for ( itera=0; itera<nitera; itera++ )
	fprintf( fp, "##E %8.3f %6d %6d %6d\n",
			    etime[itera], (int)de_pkg[itera],
			    (int)de_pp0[itera], (int)de_dram[itera] );
}

/* Initialize the use of RAPL interface */
static void rapl_init() {
    uint64_t data;
    uint32_t reg;
    uint16_t *p;
    unsigned char *c;
    int pu, esu, tu;
    int imaxp, iminp, itsp, imtw;
    double ct, cp, ce;
    static int called = false;
    if ( called ) return;
    // get unit of power, energy and time in RAPL
    reg = MSR_RAPL_POWER_UNIT;
    rdmsr( 0, reg, &data );
    c = (unsigned char*)(&data);
    _IPU_ = pu  = ( c[0] & 0x0f );	// power unit (1/2)^PU Watts
    _IESU_= esu = ( c[1] & 0x1f );// energy status unit (1/2)^ESU Joules
    _ITU_ = tu  = ( c[2] & 0x0f );	// time unit (1/2)^TU sec.
    cp  = pow( 0.5e0, (double)pu );
    ce  = pow( 0.5e0, (double)esu );
    ct  = pow( 0.5e0, (double)tu );

    // get PKG power information
    reg = MSR_PKG_POWER_INFO;
    rdmsr( 0, reg, &data );
    p = (uint16_t*)(&data);
    itsp  = ( p[0] & 0x7fff );
    iminp = ( p[1] & 0x7fff );
    imaxp = ( p[2] & 0x7fff );
    imtw  = ( p[3] & 0x003f );
    PKG_MAX_POWER = (double)imaxp*cp;
    PKG_MIN_POWER = (double)iminp*cp;
    PKG_TSP       = (double)itsp*cp;
    PKG_MAX_TIME_WIN = (double)imtw*ct;

    // get DRAM power information
    reg = MSR_DRAM_POWER_INFO;
    rdmsr( 0, reg, &data );
    p = (uint16_t*)(&data);
    itsp  = ( p[0] & 0x7fff );
    iminp = ( p[1] & 0x7fff );
    imaxp = ( p[2] & 0x7fff );
    imtw  = ( p[3] & 0x003f );
    DRAM_MAX_POWER = (double)imaxp*cp;
    DRAM_MIN_POWER = (double)iminp*cp;
    DRAM_TSP       = (double)itsp*cp;
    DRAM_MAX_TIME_WIN = (double)imtw*ct;

    _ESU_ = ce;
    _PU_  = cp;
    _TU_  = ct;
    called = true;
}

/* Finalize the use of RAPL interface */
void RAPL_Finalize() {
    int ipkg;
    if ( ETIME ) {
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	    if ( NITERA[ipkg] > 0 ) {
		RAPL_Write_energy_profile( FPE[ipkg], NITERA[ipkg],
			ETIME, DE_PKG[ipkg], DE_PP0[ipkg], DE_DRAM[ipkg] );
		fclose( FPE[ipkg] );
	    }
	    NITERA[ipkg] = 0;
	}
	free( FPE );
	FPE = NULL;
    }
    if ( ENERGY_PKG_PREV ) free( ENERGY_PKG_PREV );
    if ( ENERGY_PP0_PREV ) free( ENERGY_PP0_PREV );
    if ( ENERGY_DRAM_PREV ) free( ENERGY_DRAM_PREV );

    if ( ENERGY_PKG_CUR ) free( ENERGY_PKG_CUR );
    if ( ENERGY_PP0_CUR ) free( ENERGY_PP0_CUR );
    if ( ENERGY_DRAM_CUR ) free( ENERGY_DRAM_CUR );

    if ( ENERGY_PKG_TOTAL ) free( ENERGY_PKG_TOTAL );
    if ( ENERGY_PP0_TOTAL ) free( ENERGY_PP0_TOTAL );
    if ( ENERGY_DRAM_TOTAL ) free( ENERGY_DRAM_TOTAL );

    if ( DE_PKG ) dealloc_u64_matrix( DE_PKG );
    if ( DE_PP0 ) dealloc_u64_matrix( DE_PP0 );
    if ( DE_DRAM ) dealloc_u64_matrix( DE_DRAM );
    if ( ETIME ) free( ETIME );
    if ( NITERA ) free( NITERA );
    if ( PKG_CAP_PREV ) free( PKG_CAP_PREV );
    if ( DRAM_CAP_PREV ) free( DRAM_CAP_PREV );

    ENERGY_PKG_PREV = NULL;
    ENERGY_PP0_PREV = NULL;
    ENERGY_DRAM_PREV = NULL;
    ENERGY_PKG_CUR = NULL;
    ENERGY_PP0_CUR = NULL;
    ENERGY_DRAM_CUR = NULL;
    ENERGY_PKG_TOTAL = NULL;
    ENERGY_PP0_TOTAL = NULL;
    ENERGY_DRAM_TOTAL = NULL;
    PKG_CAP_PREV = NULL;
    DRAM_CAP_PREV = NULL;
    DE_PKG  = NULL;
    DE_PP0  = NULL;
    DE_DRAM = NULL;
    ETIME   = NULL;
    NITERA  = NULL;
    NPKGS = 0;
    LPKG_ID = -1;
    LCORE_ID = -1;
    NCORES = 0;
}

int RAPL_Init( const char* file_header, const int prof_mode ) {
    int ipkg, pkg_id, core_id, ncpp, myrank, root=0;
    char enefile[MAXSTRLEN];
    static int called = false;
    if ( called ) return 0;
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );

    POMPP_Get_pkg_info( 0, &NPKGS, &LPKG_ID );
    rapl_init();
    PKG_CAP_PREV = (double*)malloc( sizeof(double) * NPKGS );
    DRAM_CAP_PREV = (double*)malloc( sizeof(double) * NPKGS );
    for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	PKG_CAP_PREV[ipkg]  = PKG_TSP;
	DRAM_CAP_PREV[ipkg] = DRAM_TSP;
    }
    /* to reduce output
    if ( FP_CONF ) {
	fprintf(FP_CONF, "##RAPL Power Unit        = %d\n", _IPU_ );
	fprintf(FP_CONF, "##RAPL Energy State Unit = %d\n", _IESU_ );
	fprintf(FP_CONF, "##RAPL Time Unit         = %d\n", _ITU_ );
	fprintf(FP_CONF, "##RAPL ------ PKG information ------\n");
	fprintf(FP_CONF, "##RAPL maximum time window (s) = %8.3e\n",
		PKG_MAX_TIME_WIN );
	fprintf(FP_CONF, "##RAPL minimum power (W)       = %5.1f\n",
		PKG_MIN_POWER );
	fprintf(FP_CONF, "##RAPL maximum power (W)       = %5.1f\n",
		PKG_MAX_POWER );
	fprintf(FP_CONF, "##RAPL thermal spec power (W)  = %5.1f\n",
		PKG_TSP );
	fprintf(FP_CONF, "##RAPL ------ DRAM information ------\n");
	fprintf(FP_CONF, "##RAPL maximum time window (s) = %8.3e\n",
		DRAM_MAX_TIME_WIN );
	fprintf(FP_CONF, "##RAPL minimum power (W)       = %5.1f\n",
		DRAM_MIN_POWER );
	fprintf(FP_CONF, "##RAPL maximum power (W)       = %5.1f\n",
		DRAM_MAX_POWER );
	fprintf(FP_CONF, "##RAPL thermal spec power (W)  = %5.1f\n",
		DRAM_TSP );
    }
    */

    // previous energy values (values of counters, themselves)
    ENERGY_PKG_PREV  = (uint32_t*)malloc( sizeof(uint32_t) * NPKGS );
    ENERGY_PP0_PREV  = (uint32_t*)malloc( sizeof(uint32_t) * NPKGS );
    ENERGY_DRAM_PREV = (uint32_t*)malloc( sizeof(uint32_t) * NPKGS );
    // energy values just after retrieve(values of counters, themselves)
    ENERGY_PKG_CUR  = (uint32_t*)malloc( sizeof(uint32_t) * NPKGS );
    ENERGY_PP0_CUR  = (uint32_t*)malloc( sizeof(uint32_t) * NPKGS );
    ENERGY_DRAM_CUR = (uint32_t*)malloc( sizeof(uint32_t) * NPKGS );
    // accumulated energy values from initialization
    ENERGY_PKG_TOTAL  = (uint64_t*)malloc( sizeof(uint64_t) * NPKGS );
    ENERGY_PP0_TOTAL  = (uint64_t*)malloc( sizeof(uint64_t) * NPKGS );
    ENERGY_DRAM_TOTAL = (uint64_t*)malloc( sizeof(uint64_t) * NPKGS );
    for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	ENERGY_PKG_TOTAL[ipkg]  = 0;
	ENERGY_PP0_TOTAL[ipkg]  = 0;
	ENERGY_DRAM_TOTAL[ipkg] = 0;
    }

    if ( prof_mode == ENE_PROF ) {
	DE_PKG  = alloc_u64_matrix( NPKGS, MAXITERA );
	DE_PP0  = alloc_u64_matrix( NPKGS, MAXITERA );
	DE_DRAM = alloc_u64_matrix( NPKGS, MAXITERA );
	ETIME   = (double*)malloc( sizeof(double) * MAXITERA );
	NITERA  = (int*)malloc( sizeof(int) * NPKGS );
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) NITERA[ipkg] = 0;
	for ( ipkg=0, pkg_id=LPKG_ID; ipkg<NPKGS; ipkg++, pkg_id++ ) {
	    core_id = MSR_Get_lcore_id( pkg_id );
	    RAPL_Read_energy_counts( core_id, &ENERGY_PKG_PREV[ipkg],
		    &ENERGY_PP0_PREV[ipkg], &ENERGY_DRAM_PREV[ipkg] );
	}
	FPE = (FILE**)malloc( sizeof(FILE*) * NPKGS );
	for ( ipkg=0, pkg_id=LPKG_ID; ipkg<NPKGS; ipkg++, pkg_id++ ) {
	    sprintf( enefile, "%s-%d-%d.ene", file_header, myrank, pkg_id );
	    if ( (FPE[ipkg]=fopen( enefile, "w")) == NULL ) {
		printf("ERROR: rank=%d: file \"%s\" can\'t be opened\n",
			myrank, enefile );
		fflush(stdout);
		return -1;
	    }
	    /* output the unit of energy (J/count) */
	    fprintf( FPE[ipkg], "##C %16.8e\n", _ESU_ );
	    /* output the the symbolic name of energy values */
	    fprintf( FPE[ipkg], "##H %8s %6s %6s %6s\n",
		    "ETIME", "PKG", "PP0", "DRAM" );
	}
    }

    ncpp     = MSR_Get_ncores_per_pkg();
    LCORE_ID = LPKG_ID * ncpp;
    NCORES   = NPKGS   * ncpp;
    called = true;
    return 0;
}

/* Return the maximum power of CPU (package) obtained from MSR */
double RAPL_Get_pkg_max_power() {
    rapl_init();
    return PKG_MAX_POWER;
}

/* Return the minimum power of CPU (package) obtained from MSR */
double RAPL_Get_pkg_min_power() {
    rapl_init();
    return PKG_MIN_POWER;
}

/* Return the maximum power of DRAM obtained from MSR */
double RAPL_Get_dram_max_power() {
    rapl_init();
    return DRAM_MAX_POWER;
}

/* Return the minimum power of DRAM obtained from MSR */
double RAPL_Get_dram_min_power() {
    rapl_init();
    return DRAM_MIN_POWER;
}

/* Return the TDP of CPU (package) obtained from MSR */
double RAPL_Get_pkg_tdp() {
    rapl_init();
    return PKG_TSP;
}

/** Return the TDP of DRAM obtained from MSR */
double RAPL_Get_dram_tdp() {
    rapl_init();
    return DRAM_TSP;
}

/* Return the unit of energy (J/count) set on the MSR */
double RAPL_Get_energy_unit() {
    rapl_init();
    return _ESU_;
}

/* ===============================================================
 * Definition of functions to use control the power using RAPL
 * =============================================================== */
/* Release the power capping for specified domain of specified core */
int RAPL_Release_power_cap( const int core_id, devtype dev ) {
    uint32_t reg;
    uint64_t data;
    if ( NPKGS == 0 ) dbg("WARNING: RAPL_Init has not called, yet.\n");
    if        ( dev == RAPL_PKG ) {
	reg = MSR_PKG_POWER_LIMIT;
	data = (uint64_t)0x6851400148410;
    } else if ( dev == RAPL_PP0 ) {
	reg = MSR_PP0_POWER_LIMIT;
	data = (uint64_t)0;
    } else if ( dev == RAPL_DRAM ) {
	reg = MSR_DRAM_POWER_LIMIT;
	data = (uint64_t)0;
    }
    wrmsr( core_id, reg, data );
    return 0;
}

/* Release the power capping for all domain of specified package */
void RAPL_Release_all_power_cap( const int pkg_id ) {
    int core_id, cid0, cid1, ipkg;
    cid0 = MSR_Get_lcore_id( pkg_id );
    cid1 = cid0 + MSR_Get_ncores_per_pkg();
    for ( core_id=cid0; core_id<cid1; core_id++ ) {
	RAPL_Release_power_cap( core_id, RAPL_PKG );
	RAPL_Release_power_cap( core_id, RAPL_PP0 );
	RAPL_Release_power_cap( core_id, RAPL_DRAM );
    }
    ipkg = pkg_id - LPKG_ID;
    PKG_CAP_PREV[ipkg]  = PKG_TSP;
    DRAM_CAP_PREV[ipkg] = DRAM_TSP;
}

static void RAPL_Release_all_power_cap2( const int pkg_id ) {
    int core_id, cid0, cid1, ipkg;
    cid0 = MSR_Get_lcore_id( pkg_id );
    cid1 = cid0 + MSR_Get_ncores_per_pkg();
    for ( core_id=cid0; core_id<cid1; core_id++ ) {
	RAPL_Release_power_cap( core_id, RAPL_PKG );
	RAPL_Release_power_cap( core_id, RAPL_PP0 );
	RAPL_Release_power_cap( core_id, RAPL_DRAM );
    }
    ipkg = pkg_id - LPKG_ID;
    PKG_CAP_PREV[ipkg]  = PKG_TSP;
    DRAM_CAP_PREV[ipkg] = DRAM_TSP;
}
/* Release power cap for all domain and all packages used by this process */
void RAPL_Release_all_package_power_cap() {
    int pkg_id;
    for ( pkg_id=LPKG_ID; pkg_id<(LPKG_ID+NPKGS); pkg_id++ )
	RAPL_Release_all_power_cap2( pkg_id );
}

/* Apply power capping for specified domain of all packages used by
 * this process */
int RAPL_Set_power_caps( devtype dev, double *limit_val ) {
    uint32_t reg;
    uint64_t data, limit;
    int icore, cid0, cid1, pkg_id, ierr, ipkg, ncores_per_pkg;
    int isame;
    double tdp;
    if ( NPKGS == 0 ) dbg("WARNING: RAPL_Init has not called, yet.\n");
    ierr = 0;
    for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	if ( limit_val[ipkg] <= 0.e0 ) ierr++;
    }
    if ( ierr != 0 ) return -1;
    if ( dev == RAPL_PKG ) {
	// given capping value is same as previous one ?
	isame = true;
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	    if ( limit_val[ipkg] != PKG_CAP_PREV[ipkg] ) {
		isame = false;
		break;
	    }
	}
	if ( isame ) return 0;
	// CPU power cap is same as its TDP ?
	tdp = RAPL_Get_pkg_tdp();
	isame = true;
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	    if ( limit_val[ipkg] != tdp ) {
		isame = false;
		break;
	    }
	}
	if ( isame ) {
	    for ( pkg_id=LPKG_ID; pkg_id<(LPKG_ID+NPKGS); pkg_id++ )
		RAPL_Release_power_cap( pkg_id, RAPL_PKG );
	    return 0;
	}
    } else if ( dev == RAPL_DRAM ) {
	isame = true;
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	    if ( limit_val[ipkg] != DRAM_CAP_PREV[ipkg] ) {
		isame= false;
		break;
	    }
	}
	if ( isame ) return 0;
	tdp = RAPL_Get_dram_tdp();
	isame = true;
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	    if ( limit_val[ipkg] != tdp ) {
		isame = false;
		break;
	    }
	}
	if ( isame ) {
	    for ( pkg_id=LPKG_ID; pkg_id<(LPKG_ID+NPKGS); pkg_id++ )
		RAPL_Release_power_cap( pkg_id, RAPL_DRAM );
	    return 0;
	}
    }
    // Applying the power cap
    ncores_per_pkg = MSR_Get_ncores_per_pkg();
    for ( ipkg=0, pkg_id=LPKG_ID; ipkg<NPKGS; ipkg++, pkg_id++ ) {
	limit = (uint64_t)( limit_val[ipkg] / _PU_ );
	if        ( dev == RAPL_PKG ) {
	    reg = MSR_PKG_POWER_LIMIT;
	    data = (uint64_t)0x6051400018000 | limit;
	    PKG_CAP_PREV[ipkg] = limit_val[ipkg];
	} else if ( dev == RAPL_PP0 ) {
	    reg = MSR_PP0_POWER_LIMIT;
	    data = (uint64_t)0x18000 | limit;
	} else if ( dev == RAPL_DRAM ) {
	    reg = MSR_DRAM_POWER_LIMIT;
	    data = (uint64_t)0x8000 | limit;
	    DRAM_CAP_PREV[ipkg] = limit_val[ipkg];
	}
	cid0 = MSR_Get_lcore_id( pkg_id );
	cid1 = cid0 + ncores_per_pkg;
	for ( icore=cid0; icore<cid1; icore++ ) wrmsr(icore, reg, data);
    }
    return 0;
}

/* Return the value of power cap for specified device of specified package
 * */
double RAPL_Get_power_cap( devtype dev, const int pkg_id ) {
    int ipkg;
    ipkg = pkg_id - LPKG_ID;
    if      ( dev == RAPL_PKG  ) return PKG_CAP_PREV[ipkg];
    else if ( dev == RAPL_DRAM ) return DRAM_CAP_PREV[ipkg];
    return 0.e0;
}

/* ==============================================================
 * Definition of functions to measure the energy consumptions
 * ============================================================== */
/* Update accumulated energy values of packages used by this process.
 * (Only update, don't read)*/
void RAPL_Update() {
    int core_id, ipkg, pkg_id, nitera;
    uint32_t val_pkg, val_pp0, val_dram, dpkg, dpp0, ddram;
    double etime;
    etime = ETIME_Get_etime();
    for ( ipkg=0, pkg_id=LPKG_ID; ipkg<NPKGS; ipkg++, pkg_id++ ) {
	core_id = MSR_Get_lcore_id( pkg_id );
	RAPL_Read_energy_counts( core_id, &val_pkg, &val_pp0, &val_dram );

	// PKG
	if ( val_pkg < ENERGY_PKG_PREV[ipkg] )
	    dpkg = (~ENERGY_PKG_PREV[ipkg]+1) + val_pkg;
	else
	    dpkg = val_pkg - ENERGY_PKG_PREV[ipkg];
	ENERGY_PKG_TOTAL[ipkg] += dpkg;
	ENERGY_PKG_PREV[ipkg] = val_pkg;
	// PP0
	if ( val_pp0 < ENERGY_PP0_PREV[ipkg] )
	    dpp0 = (~ENERGY_PP0_PREV[ipkg]+1) + val_pp0;
	else
	    dpp0 = val_pp0 - ENERGY_PP0_PREV[ipkg];
	ENERGY_PP0_TOTAL[ipkg] += dpp0;
	ENERGY_PP0_PREV[ipkg] = val_pp0;
	// DRAM
	if ( val_dram < ENERGY_DRAM_PREV[ipkg] )
	    ddram = (~ENERGY_DRAM_PREV[ipkg]+1) + val_dram;
	else
	    ddram = val_dram - ENERGY_DRAM_PREV[ipkg];
	ENERGY_DRAM_TOTAL[ipkg] += ddram;
	ENERGY_DRAM_PREV[ipkg] = val_dram;

	if ( ETIME ) {
	    nitera = NITERA[ipkg];
	    ETIME[nitera] = etime;
	    DE_PKG[ipkg][nitera] = dpkg;
	    DE_PP0[ipkg][nitera] = dpp0;
	    DE_DRAM[ipkg][nitera] = ddram;
	    NITERA[ipkg]++;
	    if ( NITERA[ipkg] >= MAXITERA ) {
		RAPL_Write_energy_profile( FPE[ipkg], NITERA[ipkg],
			ETIME, DE_PKG[ipkg], DE_PP0[ipkg], DE_DRAM[ipkg] );
		NITERA[ipkg] = 0;
	    }
	}
    }
}

/* Return current energy values of all packages used by this process */
void RAPL_Read( uint64_t energy_pkg[], uint64_t energy_pp0[],
	uint64_t energy_dram[] ) {
    int ipkg;
    RAPL_Update();
    for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	energy_pkg[ipkg] = ENERGY_PKG_TOTAL[ipkg];
	energy_pp0[ipkg] = ENERGY_PP0_TOTAL[ipkg];
	energy_dram[ipkg] = ENERGY_DRAM_TOTAL[ipkg];
    }
}
