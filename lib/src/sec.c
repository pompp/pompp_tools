/*
 * The PomPP software including libraries and tools is developed by the
 * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
 * research project supported by the JST, CREST research program.
 * The copyrights for code used under license from other parties are
 * included in the corresponding files.
 * 
 * Copyright (c) 2015, The PomPP research team.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "ric-def.h"

#include "rapl.h"
#include "freq.h"
#include "pmc.h"
#include "etime.h"
#include "affinity.h"

#define UNDEF_ETIME	-1.e0

extern int POMPP_Initialized();

/* Declaration of arrayes and variables used by functions defined in
 * this file */
// # of packages and leading package ID in the node used by this process
static int NPKGS, LPKG_ID;
// Values at initialization of library
static uint64_t ***GPMC0 = NULL;	// [sec][pkg][iev]
static uint64_t ***FPMC0 = NULL;	// [sec][pkg][iev]
static double **FREQxTIME0    = NULL;	// [sec][pkg]
static double *ETIME0 = NULL;		// [sec]
static uint64_t **ENERGY_PKG0 = NULL;	// [sec][pkg]
static uint64_t **ENERGY_PP00 = NULL;	// [sec][pkg]
static uint64_t **ENERGY_DRAM0 = NULL;	// [sec][pkg]
// Values at finalization of library
static uint64_t ***GPMC1 = NULL;	// [sec][pkg][iev]
static uint64_t ***FPMC1 = NULL;	// [sec][pkg][iev]
static double **FREQxTIME1    = NULL;	// [sec][pkg]
static double *ETIME1 = NULL;		// [sec]
static uint64_t **ENERGY_PKG1 = NULL;	// [sec][pkg]
static uint64_t **ENERGY_PP01 = NULL;	// [sec][pkg]
static uint64_t **ENERGY_DRAM1 = NULL;	// [sec][pkg]

// variables related to sections
static char **SEC_NAMES = NULL;
static int *DEFINED     = NULL;
static double **CPU_CAPS = NULL;
static double **DRAM_CAPS = NULL;
static double DEFAULT_CPU_CAP;
static double DEFAULT_DRAM_CAP;

// for deal with overlap (nest)
static double **CPU_CAP_INIT  = NULL;
static double **DRAM_CAP_INIT = NULL;

// for nest
static int NEST = 0;
static int *NEST_LEVEL = NULL;
static double **CPU_CAP_NEST  = NULL;	// [nest][pkg]
static double **DRAM_CAP_NEST = NULL;	// [nest][pkg]

static int NEST_FUNC = 0;
static int *NEST_FUNCS = NULL;

// coefficient to convert the value of energy
static double ESU;

// for file I/O
static FILE** FPS = NULL;
static char** FNAMES = NULL;	// list of output file names

static int PROF_MODE;

// variables related to PMCs
static int NGPMC, NFPMC;
static char **GPMC_EVENT_NAME = NULL;
static char **FPMC_EVENT_NAME = NULL;

// minimim execution time for sections of which the performance data
// to be measured
static double MIN_ETIME;

/* Allocate the memory of arrayes used to store section data */
static int SEC_Alloc_section() {
    int i;
    static int called = false;
    if ( !called ) {
	SEC_NAMES = alloc_cmatrix( MAXNSEC, MAXSECLEN );
	CPU_CAPS  = alloc_dmatrix( MAXNSEC, NPKGS );
	DRAM_CAPS = alloc_dmatrix( MAXNSEC, NPKGS );
	CPU_CAP_NEST = alloc_dmatrix( MAXNSEC, NPKGS );
	DRAM_CAP_NEST = alloc_dmatrix( MAXNSEC, NPKGS );
	NEST_LEVEL = (int*)malloc( sizeof(int) * MAXNSEC );
	DEFINED = (int*)malloc( sizeof(int) * MAXNSEC );
	CPU_CAP_INIT = alloc_dmatrix( MAXNSEC, NPKGS );
	DRAM_CAP_INIT = alloc_dmatrix( MAXNSEC, NPKGS );
	NEST_FUNCS = (int*)malloc( sizeof(int) * MAXNSEC );
	// 初期化
	for ( i=0; i<MAXNSEC; i++ ) {
	    SEC_NAMES[i][0] = '\0';
	    NEST_LEVEL[i]   = -1;
	    DEFINED[i]      = false;
	    NEST_FUNCS[i]   = -1;
	}
	memset( CPU_CAP_INIT[0], '\0', sizeof(double)*MAXNSEC*NPKGS );
	memset( DRAM_CAP_INIT[0], '\0', sizeof(double)*MAXNSEC*NPKGS );
	memset( CPU_CAPS[0], '\0', sizeof(double)*MAXNSEC*NPKGS );
	memset( DRAM_CAPS[0], '\0', sizeof(double)*MAXNSEC*NPKGS );
	NEST = 0;
	NEST_FUNC = 0;
	/* to reduce output
	if ( FP_CONF ) {
	    fprintf(FP_CONF,
		    "##SEC max. # of sections            = %d\n",
		    MAXNSEC );
	    fprintf(FP_CONF,
		    "##SEC max. length of a section name = %d\n",
		    MAXSECLEN );
	} 
	*/
	called = true;
    }
    return 0;
}

/* Return leading address of array stored section names */
char** SEC_Getadd_section_names() {
    SEC_Alloc_section();
    return SEC_NAMES;
}

/* Assigned the value of CPU power cap for all sections and all package
 * used by this process */
void SEC_Put_cpu_caps( const int nsec, const int npkgs, double **src ) {
    int isec, ipkg;
    SEC_Alloc_section();
    for ( isec=0; isec<nsec; isec++ ) {
	for ( ipkg=0; ipkg<npkgs; ipkg++ )
	    CPU_CAPS[isec][ipkg] = src[isec][ipkg];
    }
}

/* Assigned the value of DRAM power cap for all sections and all package
 * used by this process */
void SEC_Put_dram_caps( const int nsec, const int npkgs, double **src ) {
    int isec, ipkg;
    SEC_Alloc_section();
    for ( isec=0; isec<nsec; isec++ ) {
	for ( ipkg=0; ipkg<npkgs; ipkg++ )
	    DRAM_CAPS[isec][ipkg] = src[isec][ipkg];
    }
}

/* Finalize the functionality defined in this file */
static void SEC_Finalize() {
    int i, nsec, ipkg;
    nsec = 0;
    for ( i=0; i<MAXNSEC; i++ ) {
	if ( SEC_NAMES[i][0] != '\0' ) nsec++;
    }

    if ( GPMC0 ) dealloc_u64_tensor( GPMC0 );
    if ( FPMC0 ) dealloc_u64_tensor( FPMC0 );
    if ( FREQxTIME0 ) dealloc_dmatrix( FREQxTIME0 );
    if ( ENERGY_PKG0 ) dealloc_u64_matrix( ENERGY_PKG0 );
    if ( ENERGY_PP00 ) dealloc_u64_matrix( ENERGY_PP00 );
    if ( ENERGY_DRAM0 ) dealloc_u64_matrix( ENERGY_DRAM0 );
    if ( ETIME0 ) free( ETIME0 );

    if ( GPMC1 ) dealloc_u64_tensor( GPMC1 );
    if ( FPMC1 ) dealloc_u64_tensor( FPMC1 );
    if ( FREQxTIME1 ) dealloc_dmatrix( FREQxTIME1 );
    if ( ENERGY_PKG1 ) dealloc_u64_matrix( ENERGY_PKG1 );
    if ( ENERGY_PP01 ) dealloc_u64_matrix( ENERGY_PP01 );
    if ( ENERGY_DRAM1 ) dealloc_u64_matrix( ENERGY_DRAM1 );
    if ( ETIME1 ) free( ETIME1 );

    if ( SEC_NAMES ) dealloc_cmatrix( SEC_NAMES );
    if ( CPU_CAPS  ) dealloc_dmatrix( CPU_CAPS );
    if ( DRAM_CAPS  ) dealloc_dmatrix( DRAM_CAPS );
    if ( CPU_CAP_NEST ) dealloc_dmatrix( CPU_CAP_NEST );
    if ( DRAM_CAP_NEST ) dealloc_dmatrix( DRAM_CAP_NEST );
    if ( NEST_LEVEL ) free( NEST_LEVEL );
    if ( DEFINED ) free( DEFINED );
    if ( CPU_CAP_INIT ) dealloc_dmatrix( CPU_CAP_INIT );
    if ( DRAM_CAP_INIT ) dealloc_dmatrix( DRAM_CAP_INIT );

    if ( NEST_FUNCS ) free ( NEST_FUNCS );

    GPMC0        = NULL;
    FPMC0        = NULL;
    FREQxTIME0   = NULL;
    ENERGY_PKG0  = NULL;
    ENERGY_PP00  = NULL;
    ENERGY_DRAM0 = NULL;
    ETIME0       = NULL;
    GPMC1        = NULL;
    FPMC1        = NULL;
    FREQxTIME1   = NULL;
    ENERGY_PKG1  = NULL;
    ENERGY_PP01  = NULL;
    ENERGY_DRAM1 = NULL;
    ETIME1       = NULL;

    SEC_NAMES    = NULL;
    CPU_CAPS     = NULL;
    DRAM_CAPS    = NULL;
    CPU_CAP_NEST = NULL;
    NEST_LEVEL   = NULL;
    DRAM_CAP_NEST = NULL;
    NEST_LEVEL   = NULL;
    DEFINED      = NULL;

    CPU_CAP_INIT  = NULL;
    DRAM_CAP_INIT = NULL;

    NEST_FUNCS    = NULL;

    if ( FPS ) {
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) fclose( FPS[ipkg] );
	free( FPS );
	FPS = NULL;
    }
    if ( nsec == 0 && FNAMES != NULL ) {
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) remove( FNAMES[ipkg] );
	dealloc_cmatrix( FNAMES );
	FNAMES = NULL;
    }
}

/* Initialize the functionality provided by functions in this file */
int SEC_Init( const int prof_mode, const char *file_header,
	const double cpu_cap0, const double dram_cap0 ) {
    int i, ipkg, pkg_id, iev, myrank, root=0, len;
    char secfile[MAXSTRLEN], *p, hostname[MPI_MAX_PROCESSOR_NAME];

    static int called = false;
    if ( called ) return 0;
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    if ( myrank == root ) {
	MIN_ETIME = 0.01e0;
	if ( (p=getenv("POMPP_MIN_ETIME")) ) {
	    MIN_ETIME = atof( p );
	    if ( MIN_ETIME < 0.e0 ) MIN_ETIME = 0.01e0;
	}
    }
    MPI_Bcast( &MIN_ETIME, 1, MPI_DOUBLE, root, MPI_COMM_WORLD );
    if ( FP_CONF ) {
	fprintf( FP_CONF, "##SEC min. exec. time of section = %5.1e\n",
		MIN_ETIME );
    }

    POMPP_Get_pkg_info( 0, &NPKGS, &LPKG_ID );
    PMC_Get_data( &NGPMC, &NFPMC, &GPMC_EVENT_NAME, &FPMC_EVENT_NAME );

    MPI_Get_processor_name( hostname, &len );
    PROF_MODE = prof_mode;
    // Open files for output the profile information
    if ( PROF_MODE != NO_PROF ) {
	FPS    = (FILE**)malloc( sizeof(FILE*) * NPKGS );
	FNAMES = alloc_cmatrix( NPKGS, MAXSTRLEN );
	for ( ipkg=0, pkg_id=LPKG_ID; ipkg<NPKGS; ipkg++, pkg_id++ ) {
	    sprintf( FNAMES[ipkg], "%s-%d-%d.sec",
		    file_header, myrank, pkg_id );
	    if ( (FPS[ipkg]=fopen( FNAMES[ipkg], "a")) == NULL ) {
		if ( (FPS[ipkg]=fopen( FNAMES[ipkg], "w")) == NULL ) {
		    printf("ERROR: rank=%d: file \"%s\" can\'t be opened\n",
			    myrank, FNAMES[ipkg] );
		    fflush( stdout );
		    return -1;
		}
	    }
	    fprintf( FPS[ipkg], "##C %.3f %.3f %.3f %.3f  %d %d %d %s\n",
		    RAPL_Get_pkg_tdp(), RAPL_Get_dram_tdp(),
		    cpu_cap0, dram_cap0, NPKGS, LPKG_ID,
		    pkg_id, hostname );
	    fflush( FPS[ipkg] );
	}	// for ( ipkg );
    }
    if ( PROF_MODE == SEC_PROF ) {
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	    fprintf( FPS[ipkg],
		    "##S %3s %16s %7s %7s %7s %7s %6s %7s %7s %6s %4s",
		    "NST",
		    "SEC", "START", "END", "E-TIME", "PKGCAP", "DRMCAP",
		    "PKGPWR", "PP0PWR", "DRMPWR", "FREQ" );
	    for ( iev=0; iev<NGPMC; iev++ )
		fprintf( FPS[ipkg], " %14s", GPMC_EVENT_NAME[iev] );
	    for ( iev=0; iev<NFPMC; iev++ )
		fprintf( FPS[ipkg], " %14s", FPMC_EVENT_NAME[iev] );
	    fprintf( FPS[ipkg], "\n" );
	}
    } else if ( PROF_MODE == ENE_PROF ) {
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	    fprintf( FPS[ipkg], "##S %3s %16s %8s %8s %4s\n",
		    "NST", "SEC", "START", "END", "FREQ" );
	}
    }

    DEFAULT_CPU_CAP  = cpu_cap0;
    DEFAULT_DRAM_CAP = dram_cap0;
    for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	CPU_CAP_NEST[NEST][ipkg]  = DEFAULT_CPU_CAP;
	DRAM_CAP_NEST[NEST][ipkg] = DEFAULT_DRAM_CAP;
    }
    NEST_LEVEL[NEST]    = NEST;
    NEST++;

    GPMC0        = alloc_u64_tensor( MAXNSEC, NPKGS, NGPMC );
    FPMC0        = alloc_u64_tensor( MAXNSEC, NPKGS, NFPMC );
    FREQxTIME0   = alloc_dmatrix( MAXNSEC, NPKGS );
    ENERGY_PKG0  = alloc_u64_matrix( MAXNSEC,  NPKGS );
    ENERGY_PP00  = alloc_u64_matrix( MAXNSEC,  NPKGS );
    ENERGY_DRAM0 = alloc_u64_matrix( MAXNSEC,  NPKGS );
    ETIME0       = (double*)malloc( sizeof(double) * MAXNSEC );

    GPMC1        = alloc_u64_tensor( MAXNSEC, NPKGS, NGPMC );
    FPMC1        = alloc_u64_tensor( MAXNSEC, NPKGS, NFPMC );
    FREQxTIME1   = alloc_dmatrix( MAXNSEC, NPKGS );
    ENERGY_PKG1  = alloc_u64_matrix( MAXNSEC,  NPKGS );
    ENERGY_PP01  = alloc_u64_matrix( MAXNSEC,  NPKGS );
    ENERGY_DRAM1 = alloc_u64_matrix( MAXNSEC,  NPKGS );
    ETIME1       = (double*)malloc( sizeof(double) * MAXNSEC );

    ESU = RAPL_Get_energy_unit();
    for ( i=0; i<MAXNSEC; i++ ) {
	ETIME0[i] = UNDEF_ETIME;	// Setting etime to "UNDEFINED"
	if ( SEC_NAMES[i][0] != '\0' ) DEFINED[i] = true;
    }
    atexit( SEC_Finalize );
    called = true;
    return 0;
}

/* Return the section ID by name.
 * If the section with given name hasn't been defined,
 * append the section name and return its section ID. */
static int POMPP_Get_section_id( const char *secname ) {
    int i;
    // return section ID if defined
    for ( i=0; i<MAXNSEC; i++ )
	if (  strcmp( SEC_NAMES[i], secname ) == 0 ) return i;
    // append new section and return its ID if not defined
    for ( i=0; i<MAXNSEC; i++ ) {
	if ( SEC_NAMES[i][0] == '\0' ) {
	    strcpy( SEC_NAMES[i], secname );
	    return i;
	}
    }
    //
    dbg("ERROR: No enough space to store section name\n");
    return -1;
}

/* Apply power cap for target section */
static int SEC_Set_power_cap( const int id ) {
    int ipkg, flag;
    double pkg, dram;
    if ( DEFINED[id] ) {
	flag = false;
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	    pkg  = CPU_CAPS[id][ipkg];
	    dram = DRAM_CAPS[id][ipkg];
	    if ( pkg > 0.e0 ) {
		if ( dram > 0.e0 ) {
		    CPU_CAP_NEST[NEST][ipkg] = pkg;
		    DRAM_CAP_NEST[NEST][ipkg] = dram;
		    flag = true;
		} else {
		    CPU_CAP_NEST[NEST][ipkg]  = pkg;
		    dram = DRAM_CAP_NEST[NEST][ipkg]
			= DRAM_CAP_NEST[NEST-1][ipkg];
		    flag = true;
		}
	    } else {
		if ( dram > 0.e0 ) {
		    pkg = CPU_CAP_NEST[NEST][ipkg]
			= CPU_CAP_NEST[NEST-1][ipkg];
		    DRAM_CAP_NEST[NEST][ipkg] = dram;
		    flag = true;
		} else {
		    return 0;
		}
	    }
	    CPU_CAP_INIT[id][ipkg]  = pkg;
	    DRAM_CAP_INIT[id][ipkg] = dram;
	}	// for ( ipkg )
	if ( flag ) {
	    NEST_LEVEL[id] = NEST;
	    NEST++;
	}
	RAPL_Set_power_caps( RAPL_PKG, CPU_CAPS[id] );
	RAPL_Set_power_caps( RAPL_DRAM, DRAM_CAPS[id] );
    } else {
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	    CPU_CAP_INIT[id][ipkg]  = CPU_CAP_NEST[NEST-1][ipkg];
	    DRAM_CAP_INIT[id][ipkg] = DRAM_CAP_NEST[NEST-1][ipkg];
	}
	NEST_LEVEL[id] = -1;
    }
    return 0;
}

/* Get the values of power cap and nest level of target section */
static int SEC_Get_power_caps( const int id, double **cpu_caps,
	double **dram_caps, int *level ) {
    if ( id < 0 || id >= MAXNSEC ) return -1;
    *cpu_caps  = CPU_CAP_INIT[id];
    *dram_caps = DRAM_CAP_INIT[id];
    *level     = NEST_LEVEL[id];
    return 0;
}

/* Release power cap of target section */
static int SEC_Unset_power_cap( const int id ) {
    int nest_level, ipkg, flag;
    double pkg, dram;
    if ( DEFINED[id] ) {
	flag = true;
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	    if ( CPU_CAPS[id][ipkg] > 0.e0 || DRAM_CAPS[id][ipkg] > 0.e0 ) {
		flag = false;
		break;
	    }
	}
	if ( flag ) return 0;
	nest_level = NEST_LEVEL[id];
	// Make the power cappings invalid
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	    CPU_CAP_NEST[nest_level][ipkg] = 0.e0;
	    DRAM_CAP_NEST[nest_level][ipkg] = 0.e0;
	}
	// check the nest level of target section and control it
	NEST_LEVEL[id] = -1;
	if ( nest_level == (NEST-1) ) {
	    ipkg = 0;
	    for ( ; NEST>1; NEST-- ) {
		if ( CPU_CAP_NEST[NEST-1][ipkg] > 0.e0 ) break;
	    }
	    if ( NEST < 1 ) {
		dbg("Waring: NEST is below 1 (%d)\n", NEST );
	    } else {
		RAPL_Set_power_caps( RAPL_PKG,   CPU_CAP_NEST[NEST-1] );
		RAPL_Set_power_caps( RAPL_DRAM, DRAM_CAP_NEST[NEST-1] );
	    }
	}
    }
    for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	CPU_CAP_INIT[id][ipkg]  = 0.e0;
	DRAM_CAP_INIT[id][ipkg] = 0.e0;
    }
    return 0;
}

/* Start to apply power capping and to measure the performance data */
int POMPP_Start_section( const char *secname ) {
    int id, retval=0;
    if ( POMPP_Initialized() == false ) {
	dbg("Warning: RIC library has not been initialized, yet\n");
	return -1;
    }
#pragma omp master
    {
	id = POMPP_Get_section_id( secname );
	if ( id >= 0 ) {
	    if ( PROF_MODE == SEC_PROF ) {
		ETIME0[id] = ETIME_Get_etime();
		FREQ_Get_freq_time( FREQxTIME0[id] );
		RAPL_Read( ENERGY_PKG0[id], ENERGY_PP00[id],
			ENERGY_DRAM0[id] );
		PMC_Read( GPMC0[id], FPMC0[id] );
	    } else if ( PROF_MODE == ENE_PROF ) {
		ETIME0[id] = ETIME_Get_etime();
		FREQ_Get_freq_time( FREQxTIME0[id] );
	    }
	    NEST_FUNCS[id] = NEST_FUNC;
	    NEST_FUNC++;
	    
	    SEC_Set_power_cap( id );
	} else {
	    dbg("Warning: No vacant space for new section.\n");
	    retval=-1;
	}
    }	// pragma omp master
    return retval;
}

/* Stop to apply power capping and obtain the performance data in this
 * section */
int POMPP_Stop_section( const char *secname ) {
    int retval=0;
    if ( POMPP_Initialized() == false ) return 0;
#pragma omp master
    {
	int iev, ipkg, id, lv;
	double etime, pkg_pow, pp0_pow, dram_pow, coe, inv_etime;
	double *cap_pkgs, *cap_drams;
	float freq;
	id = POMPP_Get_section_id( secname );
	SEC_Get_power_caps( id, &cap_pkgs, &cap_drams, &lv );
	if ( id < 0 || ETIME0[id] == UNDEF_ETIME ) {
	    dbg("Warning: POMPP_Start_section(%s) has not been called.\n",
		    secname );
	    retval = -1;
	} else if ( PROF_MODE == SEC_PROF ) {
	    PMC_Read( GPMC1[id], FPMC1[id] );
	    RAPL_Read( ENERGY_PKG1[id], ENERGY_PP01[id],
		    ENERGY_DRAM1[id] );
	    FREQ_Get_freq_time( FREQxTIME1[id] );
	    ETIME1[id] = ETIME_Get_etime();
	    etime      = ETIME1[id]-ETIME0[id];
	    inv_etime  = 1.e0 / etime;
	    coe        = ESU * inv_etime;
	    for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
		if ( etime > MIN_ETIME ) {
		    pkg_pow = coe *
			(double)(ENERGY_PKG1[id][ipkg] -
				ENERGY_PKG0[id][ipkg]);
		    pp0_pow = coe *
			(double)(ENERGY_PP01[id][ipkg] -
				ENERGY_PP00[id][ipkg]);
		    dram_pow = coe * (double)(
			    ENERGY_DRAM1[id][ipkg] -
			    ENERGY_DRAM0[id][ipkg] );
		    freq = (float)( (FREQxTIME1[id][ipkg]-
				FREQxTIME0[id][ipkg]) * inv_etime );
		} else {
		    pkg_pow = 0.e0;
		    pp0_pow = 0.e0;
		    dram_pow = 0.e0;
		    freq = 0.0;
		}
		fprintf( FPS[ipkg], "##T %3d %16s ",
			NEST_FUNCS[id], secname );
		fprintf( FPS[ipkg], "%7.3f %7.3f %7.3f %7.3f %6.3f %7.3f"
			" %7.3f %6.3f %4.2f",
			ETIME0[id], ETIME1[id], etime,
			cap_pkgs[ipkg], cap_drams[ipkg],
			pkg_pow, pp0_pow, dram_pow,
			freq );
		for ( iev=0; iev<NGPMC; iev++ )
		    fprintf( FPS[ipkg], " %14lld", (long long int)
				(GPMC1[id][ipkg][iev]-GPMC0[id][ipkg][iev])
			    );
		for ( iev=0; iev<NFPMC; iev++ )
		    fprintf( FPS[ipkg], " %14lld", (long long int)
				(FPMC1[id][ipkg][iev]-FPMC0[id][ipkg][iev])
			   );
		fprintf( FPS[ipkg], "\n" );
	    }	// for ( ipkg )
	    ETIME0[id] = UNDEF_ETIME;
	} else if ( PROF_MODE == ENE_PROF ) {
	    FREQ_Get_freq_time( FREQxTIME1[id] );
	    ETIME1[id] = ETIME_Get_etime();
	    etime = ETIME1[id]-ETIME0[id];
	    inv_etime  = 1.e0 / etime;
	    for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
		if ( etime > MIN_ETIME ) {
		    freq = (float)(
			    (FREQxTIME1[id][ipkg]-FREQxTIME0[id][ipkg]) *
			    inv_etime );
		 } else {
		     freq = 0.0;
		 }

		fprintf( FPS[ipkg], "##T %3d %16s %8.3f %8.3f %4.2f\n",
			NEST_FUNCS[id],
			secname, ETIME0[id], ETIME1[id], freq );
	    }
	    ETIME0[id] = UNDEF_ETIME;
	}
	NEST_FUNC--;
	NEST_FUNCS[id] = -1;

	SEC_Unset_power_cap( id );
    }	// pragma omp master

    return retval;
}

void pompp_start_section_( const char *secname ) {
    POMPP_Start_section( secname );
}

void pompp_stop_section_( const char *secname ) {
    POMPP_Stop_section( secname );
}
