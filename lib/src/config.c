/*
 * The PomPP software including libraries and tools is developed by the
 * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
 * research project supported by the JST, CREST research program.
 * The copyrights for code used under license from other parties are
 * included in the corresponding files.
 * 
 * Copyright (c) 2015, The PomPP research team.
 */

/* ******************************************************************
 * Setting how to control and measure powers of CPU and DRAM
 * using the INTEL RAPL interface
 * ****************************************************************** */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define __MAIN__
#include "ric-def.h"
#include "msr.h"

#include "itimer.h"

#include "rapl.h"
#include "pmc.h"
#include "etime.h"
#include "freq.h"
#include "cpu-freq.h"
#include "affinity.h"
#include "sec.h"

#include <sys/types.h>
#include <unistd.h>
#include <sched.h>

#include <sys/types.h>
#include <sys/stat.h>

/* Setting how this library work.
 * 1. What kind profile is needed by the user
 * 2. The interval to measure values of powers, frequency, and so on
 * 3. ... etc.
 * */

#define UNDEFINED	-1

#define STATIC	0
#define DYNAMIC	1

static int CAPPING_MODE;	// capping mode
static int PROF_MODE = UNDEFINED;// profile species
static int INTERVAL_MSEC;	// interval (in ms)
static char FILE_HEADER[MAXSTRLEN];
static int  FREQ_COUNT;
static int  CPUFREQ;
static char **SEC_NAMES;	// array to store section names
static int PROF_ALL;

static int NSEC;		// # of sections

//
static int MULTI_SOCKET;

static char **KNOB_NAMES = NULL;	// identification names of knob
static double *KNOB_DEFAULT = NULL;	// default values of knob
static double ***KNOB_VALS = NULL;	// knob values [knob][sec][pkg]
static int NKNOBS = 0;			// # of species of knob

// default powers of CPU and DRAM
static double DEFAULT_CPU_POWER, DEFAULT_DRAM_POWER;

static int NPKGS, LPKG_ID;
static double ETIME0;	// elapsed time from timer initialization
static uint64_t *ENERGY_PKG0  = NULL;
static uint64_t *ENERGY_PP00  = NULL;
static uint64_t *ENERGY_DRAM0 = NULL;
static uint64_t *ENERGY_PKG1  = NULL;
static uint64_t *ENERGY_PP01  = NULL;
static uint64_t *ENERGY_DRAM1 = NULL;
static double *ENERGY_VAL     = NULL;
static uint64_t **GPMC0 = NULL;	// [ipkg][iev]
static uint64_t **FPMC0 = NULL;	// [ipkg][iev]
static uint64_t **GPMC1 = NULL;	// [ipkg][iev]
static uint64_t **FPMC1 = NULL;	// [ipkg][iev]

static int INITIALIZED = false;	// POMPP_Init is called or not

int POMPP_Initialized() { return INITIALIZED; }

/** Read setting values from configuration file */
static int POMPP_Read_config_file() {
    int myrank, nprocs, root=0;
    int ntok, i, nknob_prev, nsec, iknob, flag_conf, pkg_max;
    char line[MAXSTRLEN], config[MAXSTRLEN], config_header[MAXSTRLEN];
    char **token, *p;
    FILE *fp;
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    token     = alloc_cmatrix( MAXTOKEN, MAXTOKLEN );
    SEC_NAMES = SEC_Getadd_section_names();
    KNOB_NAMES = alloc_cmatrix( MAXKNOBS, MAXKNOBLEN );
    KNOB_VALS  = alloc_dtensor( MAXKNOBS, MAXNSEC, NPKGS );
    KNOB_DEFAULT = (double*)malloc( sizeof(double) * MAXKNOBS );
    for ( i=0; i<MAXNSEC; i++ ) SEC_NAMES[i][0] = '\0';
    for ( i=0; i<NKNOBS; i++ ) {
	KNOB_NAMES[i][0] = '\0';
	KNOB_DEFAULT[i] = 0.e0;
    }
    memset( KNOB_VALS[0][0], '\0', sizeof(double)*MAXKNOBS*MAXNSEC*NPKGS );

    if ( myrank == root ) {
	/* assignemt of initial values */
	PROF_MODE    = NO_PROF;
	FREQ_COUNT   = false;
	INTERVAL_MSEC= 10;
	CPUFREQ      = 0;
	NKNOBS       = 0;
	MULTI_SOCKET = 0;
	FILE_HEADER[0] = '\0';
	PROF_ALL     = false;
	/* Getting values from environment variables */
	// defualt powers of CPU and DRAM
	DEFAULT_CPU_POWER = DEFAULT_DRAM_POWER = 0.e0;
	if ( (p=getenv("POMPP_CPU_POWER")) ) DEFAULT_CPU_POWER  = atof(p);
	if ( (p=getenv("POMPP_DRAM_POWER")) )
	    DEFAULT_DRAM_POWER = atof(p);

	// Target CPU frequency (kHz) to be setting by CPUfreqlibs
	if ( (p=getenv("POMPP_CPUFREQ")) ) CPUFREQ = atoi(p);

	// flag whether CPU frequency has to be measured or not
	if ( (p=getenv("POMPP_FREQ_COUNT")) )
	    FREQ_COUNT = ( atoi(p)>0 ? true : false );

	// interval to measure counters such as CPU frequency, energy, etc.
	// (in ms)
	if ( (p=getenv("POMPP_INTERVAL_MSEC")) ) INTERVAL_MSEC = atoi( p );
	if ( INTERVAL_MSEC < 10 ) INTERVAL_MSEC = 10;

	// profile mode
	if ( (p=getenv("POMPP_PROF_MODE")) ) {
	    strcpy( line, p );
	    misc_tolower( line );
	    if ( strcmp(line, "noprof") == 0 )       PROF_MODE = NO_PROF;
	    else if ( strcmp(line, "section") == 0 ) PROF_MODE = SEC_PROF;
	    else if ( strcmp(line, "energy") == 0 )  PROF_MODE = ENE_PROF;
	}
	// header of output file name
	if ( (p=getenv("POMPP_FILE_HEADER")) ) strcpy( FILE_HEADER, p );

	// socket by socket control is needed or not
	if ( (p=getenv("POMPP_MULTI_SOCKET")) )
	    MULTI_SOCKET = ( atoi(p) != 0 ? true : false );
	// entire app. performance data is needed or not
	if ( (p=getenv("POMPP_PROF_ALL")) )
	    PROF_ALL = ( atoi(p) > 0 ? true : false );
    }	// if ( myrank == root )
    MPI_Bcast( &PROF_MODE,          1, MPI_INT, root, MPI_COMM_WORLD );
    MPI_Bcast( &INTERVAL_MSEC,      1, MPI_INT, root, MPI_COMM_WORLD );
    MPI_Bcast( &FREQ_COUNT,         1, MPI_INT, root, MPI_COMM_WORLD );
    MPI_Bcast( &CPUFREQ,            1, MPI_INT, root, MPI_COMM_WORLD );
    MPI_Bcast( &MULTI_SOCKET,       1, MPI_INT, root, MPI_COMM_WORLD );
    MPI_Bcast( &PROF_ALL,           1, MPI_INT, root, MPI_COMM_WORLD );
    MPI_Bcast( &DEFAULT_CPU_POWER,  1, MPI_DOUBLE, root, MPI_COMM_WORLD );
    MPI_Bcast( &DEFAULT_DRAM_POWER, 1, MPI_DOUBLE, root, MPI_COMM_WORLD );
    MPI_Bcast( FILE_HEADER, MAXSTRLEN, MPI_CHAR, root, MPI_COMM_WORLD );

    flag_conf = false;
    if ( MULTI_SOCKET || myrank == root ) {
	if ( p=getenv("POMPP_CONFIG")) {
	    if ( FP_CONF ) fprintf(FP_CONF, "##CONF CONFIG HEADER=%s\n", p);
	    flag_conf = true;
	    strcpy( config_header, p );
	}
    }

    int npkgs_per_node, node_id, ipkg, sockid;
    npkgs_per_node = MSR_Get_npkgs_per_node();
    if ( flag_conf ) {
	pkg_max=NPKGS;
	if ( MULTI_SOCKET ) node_id = NODE_Get_id();
	else pkg_max=1;
	for ( ipkg=0; ipkg<pkg_max; ipkg++ ) {
	    if ( MULTI_SOCKET ) {
		sockid = npkgs_per_node * node_id + ipkg + LPKG_ID;
		sprintf( config, "%s-%d.conf", config_header, sockid );
	    } else {
		strcpy( config, config_header );
	    }
	    if ( (fp=fopen( config, "r" )) ) {
		nknob_prev = 0;
		nsec = 0;
		while ( fgets( line, MAXSTRLEN, fp ) != NULL ) {
		    del_prepost_space( line );
		    ntok = misc_split( line, " \t\n\r", token, MAXTOKEN,
			    MAXTOKLEN );
		    if ( strncmp( line, "# ", 2 ) == 0 ) continue;
		    // reading cap values for each section
		    else if ( strncmp( line, "#-s", 3 ) == 0 ) {
			if ( nsec >= MAXNSEC ) {
			    dbg("error: # of sections exceeds max. # of"
				    " sections(%d)\n", MAXNSEC );
			    MPI_Abort( MPI_COMM_WORLD, 20 );
			}
			strncpy( SEC_NAMES[nsec], token[1], MAXSECLEN );
			SEC_NAMES[nsec][MAXSECLEN-1] = '\0';
			for ( i=2, iknob=0; i<ntok; i++, iknob++ ) {
			    if ( iknob >= MAXKNOBS ) {
				dbg("error: # of knobs for %d-th section"
					" exceeds max. # of knobs(%d)\n",
					(nsec+1), MAXKNOBS );
				MPI_Abort( MPI_COMM_WORLD, 21 );
			    }
			    KNOB_VALS[iknob][nsec][ipkg] = atof( token[i] );
			}
			if ( nsec > 0 ) {
			    if ( nknob_prev != iknob ) {
				dbg("error: # of knobs for %d-th"
					" section differs from one of"
					" previous section"
					" (%d vs %d)\n", (nsec+1),
					iknob, nknob_prev );
				MPI_Abort( MPI_COMM_WORLD, 22 );
			    }
			}
			nknob_prev = iknob;
			nsec++;
		    // reading default values for each knob
		    } else if ( strncmp( line, "#-d", 3 ) == 0 ) {
			for ( i=1; i<ntok; i++ )
			    KNOB_DEFAULT[i-1] = atof( token[i] );
		    // reading identification names for each knob
		    } else if ( strncmp( line, "#-k", 3 ) == 0 ) {
			for ( i=1; i<ntok; i++ ) {
			    if ( NKNOBS >= MAXKNOBS ) {
				dbg("error: # of knobs exceeds max. # of"
					" knobs(%d)\n", MAXKNOBS );
				MPI_Abort( MPI_COMM_WORLD, 19 );
			    }
			    strncpy( KNOB_NAMES[NKNOBS], token[i],
				    MAXKNOBLEN );
			    KNOB_NAMES[NKNOBS][MAXKNOBLEN-1] = '\0';
			    NKNOBS++;
			}
		    }
		}	// while (fgets( fp, MAXSTRLEN, line ) != NULL )
		fclose( fp );
		NSEC = nsec;
	    } else { 
		dbg("error: config file (%s) cannot be opened", config );
		MPI_Abort( MPI_COMM_WORLD, 1000);
	    }
	    // checking # of knobs
	    if ( NKNOBS != nknob_prev ) {
		dbg("error: Inconsistency in # of knobs between #-s lines"
			" and #-k line (%d vs %d)\n", nknob_prev, NKNOBS );
		MPI_Abort( MPI_COMM_WORLD, 23 );
	    }
	}	// for ( ipkg )
    }	// if ( flag_conf )
    if ( MULTI_SOCKET == false && flag_conf ) {
	MPI_Bcast( &NKNOBS,             1, MPI_INT, root, MPI_COMM_WORLD );
	MPI_Bcast( KNOB_DEFAULT, MAXKNOBS, MPI_DOUBLE, root,
		MPI_COMM_WORLD );
	MPI_Bcast( SEC_NAMES[0],  MAXNSEC*MAXSECLEN, MPI_CHAR, root,
		MPI_COMM_WORLD);
	MPI_Bcast( KNOB_NAMES[0], MAXKNOBS*MAXKNOBLEN, MPI_CHAR, root,
		MPI_COMM_WORLD );
	MPI_Bcast( KNOB_VALS[0][0], MAXKNOBS*MAXNSEC*NPKGS, MPI_DOUBLE,
		root, MPI_COMM_WORLD );
	MPI_Bcast( &NSEC, 1, MPI_INT, root, MPI_COMM_WORLD );
    }

    dealloc_cmatrix( token );
    return 0;
}

/* Checking values obtained from configuration file and environment
 * variables */
static int POMPP_Check_parameters() {
    int myrank, nprocs, root=0;
    int iknob, isec, ipkg;
    double TDP_CPU, TDP_DRAM;
    char tmp[MAXSTRLEN];
    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    TDP_CPU  = RAPL_Get_pkg_tdp();
    TDP_DRAM = RAPL_Get_dram_tdp();
    // Counting the # of sections
    for ( isec=0; isec<MAXNSEC; isec++ )
	if ( SEC_NAMES[isec][0] == '\0' ) break;

    // ckecking the values of knobs (CPU and DRAM)
    for ( iknob=0; iknob<NKNOBS; iknob++ ) {
	strncpy( tmp, KNOB_NAMES[iknob], MAXKNOBLEN );
	misc_tolower( tmp );
	if ( strcmp("cpu", tmp) == 0 || strcmp("pkg", tmp) == 0 ) {
	    DEFAULT_CPU_POWER = KNOB_DEFAULT[iknob];
	    SEC_Put_cpu_caps( NSEC, NPKGS, KNOB_VALS[iknob] );
	} else if ( strcmp( "dram", tmp ) == 0 ) {
	    DEFAULT_DRAM_POWER = KNOB_DEFAULT[iknob];
	    SEC_Put_dram_caps( NSEC, NPKGS, KNOB_VALS[iknob] );
	} else {
	    if ( myrank == root )
		dbg("warning: knob (\"%s\") is not supported, yet\n",
			KNOB_NAMES[iknob] );
	}
    }
    // Set default powers of CPU and DRAM to their TDPs
    // if those values have not been set.
    if ( DEFAULT_CPU_POWER <= 0.e0 ||
	    DEFAULT_CPU_POWER > RAPL_Get_pkg_max_power() )
	DEFAULT_CPU_POWER = TDP_CPU;
    if ( DEFAULT_DRAM_POWER <= 0.e0 ||
	    DEFAULT_DRAM_POWER > RAPL_Get_dram_max_power() )
	DEFAULT_DRAM_POWER = TDP_DRAM;

    // Check power capping mode
    if ( NKNOBS < 1 || NSEC < 1 ) CAPPING_MODE = STATIC;
    else                          CAPPING_MODE = DYNAMIC;

    // Has the header of output file name beed assigned or not
    if ( PROF_MODE == SEC_PROF ) {
	if ( FILE_HEADER[0] == '\0' ) {
	    if ( myrank == root )
		printf("conf: warning: No file header is set while "
			" profiling mode has been set to \"sec\"\n"
			"   File header is set to \"section-prof\".\n" );
	    strcpy( FILE_HEADER, "section-prof" );
	}
    } else if ( PROF_MODE == ENE_PROF ) {
	if ( FILE_HEADER[0] == '\0' ) {
	    if ( myrank == root )
		printf("conf: warning: No file header is set while "
			" profiling mode has been set to \"energy\"\n"
			"   File header is set to \"energy-prof\".\n" );
	    strcpy( FILE_HEADER, "energy-prof" );
	}
    }
    if ( PROF_MODE != NO_PROF && FILE_HEADER[0] == '\0' ) {
	if ( myrank == root )
	    printf("conf: warning: File header of section data must "
		    "be set regardless of profile mode.\n"
		    "   File header is set to \"section-prof\".\n" );
	strcpy( FILE_HEADER, "section-prof" );
    }

    // interval to measure energies and CPU frequency
    if ( PROF_MODE == NO_PROF ) INTERVAL_MSEC = 60000;
    return 0;
}

/* output the set values for output file */
static void POMPP_Show_setting() {
    int i, k;
    double val;
    fprintf(FP_CONF,
	    "##CONF ------------------ CONFIGURATION -----------------\n");
    fprintf(FP_CONF, "##CONF POWER CAPPING MODE     = %s\n",
	    (CAPPING_MODE == STATIC ? "static" : "dynamic" ) );
    fprintf(FP_CONF, "##CONF PROFILE MODE           = ");
    if      ( PROF_MODE == NO_PROF  ) fprintf(FP_CONF, "no_prof\n");
    else if ( PROF_MODE == SEC_PROF ) fprintf(FP_CONF, "section\n");
    else                              fprintf(FP_CONF, "energy\n");
    fprintf(FP_CONF, "##CONF INTERVAL (msec)        = %d\n",
	    INTERVAL_MSEC );
    fprintf(FP_CONF, "##CONF default CPU power (W)  = %7.3f\n",
	    DEFAULT_CPU_POWER );
    fprintf(FP_CONF, "##CONF default DRAM power (W) = %7.3f\n",
	    DEFAULT_DRAM_POWER );
    fprintf(FP_CONF, "##CONF FREQ COUNT             = %s\n",
	    (FREQ_COUNT ? "yes" : "no" ) );
    fprintf(FP_CONF, "##CONF profile all section    = %s\n",
	    (PROF_ALL ? "yes" : "no" ) );
    if ( PROF_MODE != NO_PROF )
	fprintf(FP_CONF, "##CONF file header            = %s\n",
		FILE_HEADER );
    if ( CAPPING_MODE == DYNAMIC ) {
	fprintf(FP_CONF, "##CONF %7s", "section" );
	for ( i=0; i<NKNOBS; i++ ) fprintf(FP_CONF, " %7s", KNOB_NAMES[i] );
	fprintf(FP_CONF, "\n");
	for ( i=0; i<NSEC; i++ ) {
	    fprintf(FP_CONF, "##CONF %7s", SEC_NAMES[i] );
	    for ( k=0; k<NKNOBS; k++ ) {
		val = KNOB_VALS[0][k][i];
		if      ( val < 1.e3 ) fprintf(FP_CONF, " %7.3f", val );
		else if ( val < 1.e4 ) fprintf(FP_CONF, " %7.2f", val );
		else if ( val < 1.e5 ) fprintf(FP_CONF, " %7.1f", val );
		else                   fprintf(FP_CONF, " %7d", (int)val );
	    }
	    fprintf(FP_CONF, "\n");
	}
    }
    fprintf(FP_CONF,
	    "##CONF --------------------------------------------------\n");
}

static int NGPMC, NFPMC;
static char **GPMC_EVENT_NAME;
static char **FPMC_EVENT_NAME;

/* Finalize this library just before MPI_Finalize */
void POMPP_Finalize() {
    int nprocs, myrank, root=0, tag=0;
    int ipkg, pkg_id, iev, lpkg_id, irank;
    double etime, eunit;
    MPI_Status status;
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    etime = ETIME_Get_etime() - ETIME0;
    PMC_Read( GPMC1, FPMC1 );
    RAPL_Read( ENERGY_PKG1, ENERGY_PP01, ENERGY_DRAM1 );
    eunit = RAPL_Get_energy_unit();
    eunit /= etime;
    MPI_Barrier( MPI_COMM_WORLD );
    if ( myrank == root && FP_CONF && PROF_ALL ) {
	fprintf(FP_CONF, "##T TOTAL PERFORMANCE DATA\n");
	for ( ipkg=0, pkg_id=LPKG_ID; ipkg<NPKGS; ipkg++, pkg_id++ ) {
	    ENERGY_VAL[ipkg*3+0] = eunit *
		(ENERGY_PKG1[ipkg] - ENERGY_PKG0[ipkg]);
	    ENERGY_VAL[ipkg*3+1] = eunit *
		(ENERGY_PP01[ipkg] - ENERGY_PP00[ipkg]);
	    ENERGY_VAL[ipkg*3+2] = eunit *
		(ENERGY_DRAM1[ipkg] - ENERGY_DRAM0[ipkg]);
	    for ( iev=0; iev<NGPMC; iev++ )
		GPMC1[ipkg][iev] -= GPMC0[ipkg][iev];
	    for ( iev=0; iev<NFPMC; iev++ )
		FPMC1[ipkg][iev] -= FPMC0[ipkg][iev];
	    fprintf(FP_CONF, "##A RANK= %4d   PKG_ID= %2d    ETIME= %.3f\n",
		    root, pkg_id, etime );
	    fprintf(FP_CONF, "##B PKG= %.3f   PP0= %.3f   DRAM= %.3f\n",
		    ENERGY_VAL[ipkg*3+0], ENERGY_VAL[ipkg*3+1],
		    ENERGY_VAL[ipkg*3+2] );
	    for ( iev=0; iev<NGPMC; iev++ )
		fprintf(FP_CONF, "##G %8s %16llu\n", GPMC_EVENT_NAME[iev],
			(unsigned long long)GPMC1[ipkg][iev] );
	    for ( iev=0; iev<NFPMC; iev++ )
		fprintf(FP_CONF, "##G %8s %16llu\n", FPMC_EVENT_NAME[iev],
			(unsigned long long)FPMC1[ipkg][iev] );
	}
	for ( irank=0; irank<nprocs; irank++ ) {
	    if ( irank == root ) continue;
	    MPI_Recv( &etime, 1, MPI_DOUBLE, irank, tag, MPI_COMM_WORLD,
		    &status );
	    MPI_Recv( &lpkg_id, 1, MPI_INT, irank, tag, MPI_COMM_WORLD,
		    &status );
	    MPI_Recv( ENERGY_VAL, NPKGS*3, MPI_DOUBLE, irank, tag,
		    MPI_COMM_WORLD, &status );
	    MPI_Recv( GPMC1[0], NGPMC*NPKGS, MPI_UNSIGNED_LONG_LONG,
		    irank, tag, MPI_COMM_WORLD, &status );
	    MPI_Recv( FPMC1[0], NFPMC*NPKGS, MPI_UNSIGNED_LONG_LONG,
		    irank, tag, MPI_COMM_WORLD, &status );
	    for ( ipkg=0, pkg_id=lpkg_id; ipkg<NPKGS; ipkg++, pkg_id++ ) {
		fprintf(FP_CONF,
			"##A RANK= %4d   PKG_ID= %2d    ETIME= %.3f\n",
			irank, pkg_id, etime );
		fprintf(FP_CONF, "##B PKG= %.3f   PP0= %.3f   DRAM= %.3f\n",
			ENERGY_VAL[ipkg*3+0], ENERGY_VAL[ipkg*3+1],
			ENERGY_VAL[ipkg*3+2] );
		for ( iev=0; iev<NGPMC; iev++ )
		    fprintf(FP_CONF, "##G %8s %16llu\n",
			    GPMC_EVENT_NAME[iev],
			    (unsigned long long)GPMC1[ipkg][iev] );
		for ( iev=0; iev<NFPMC; iev++ )
		    fprintf(FP_CONF,
			    "##G %8s %16llu\n", FPMC_EVENT_NAME[iev],
			    (unsigned long long)FPMC1[ipkg][iev] );
	    }
	}
    } else if ( PROF_ALL ) {
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	    ENERGY_VAL[ipkg*3+0] = eunit *
		(ENERGY_PKG1[ipkg] - ENERGY_PKG0[ipkg]);
	    ENERGY_VAL[ipkg*3+1] = eunit *
		(ENERGY_PP01[ipkg] - ENERGY_PP00[ipkg]);
	    ENERGY_VAL[ipkg*3+2] = eunit *
		(ENERGY_DRAM1[ipkg] - ENERGY_DRAM0[ipkg]);
	    for ( iev=0; iev<NGPMC; iev++ )
		GPMC1[ipkg][iev] -= GPMC0[ipkg][iev];
	    for ( iev=0; iev<NFPMC; iev++ )
		FPMC1[ipkg][iev] -= FPMC0[ipkg][iev];
	}
	MPI_Send( &etime, 1, MPI_DOUBLE, root, tag, MPI_COMM_WORLD );
	MPI_Send( &LPKG_ID, 1, MPI_INT, root, tag, MPI_COMM_WORLD );
	MPI_Send( ENERGY_VAL, NPKGS*3, MPI_DOUBLE, root, tag,
		MPI_COMM_WORLD );
	MPI_Send( GPMC1[0], NGPMC*NPKGS, MPI_UNSIGNED_LONG_LONG, root,
		tag, MPI_COMM_WORLD );
	MPI_Send( FPMC1[0], NFPMC*NPKGS, MPI_UNSIGNED_LONG_LONG, root,
		tag, MPI_COMM_WORLD );
    }
    if ( ENERGY_PKG0 ) free( ENERGY_PKG0 );
    if ( ENERGY_PP00 ) free( ENERGY_PP00 );
    if ( ENERGY_DRAM0 ) free( ENERGY_DRAM0 );
    if ( ENERGY_PKG1 ) free( ENERGY_PKG1 );
    if ( ENERGY_PP01 ) free( ENERGY_PP01 );
    if ( ENERGY_DRAM1 ) free( ENERGY_DRAM1 );
    if ( KNOB_DEFAULT ) free( KNOB_DEFAULT );
    dealloc_u64_matrix( GPMC0 );
    dealloc_u64_matrix( FPMC0 );
    dealloc_u64_matrix( GPMC1 );
    dealloc_u64_matrix( FPMC1 );
    dealloc_cmatrix( KNOB_NAMES );
    dealloc_dtensor( KNOB_VALS );
    if ( ENERGY_VAL ) free( ENERGY_VAL );
    ENERGY_PKG0 = NULL;
    ENERGY_PP00 = NULL;
    ENERGY_DRAM0 = NULL;
    ENERGY_PKG1 = NULL;
    ENERGY_PP01 = NULL;
    ENERGY_DRAM1 = NULL;
    GPMC0 = NULL;
    FPMC0 = NULL;
    GPMC1 = NULL;
    FPMC1 = NULL;
    ENERGY_VAL = NULL;

    KNOB_NAMES = NULL;
    KNOB_VALS  = NULL;
    KNOB_DEFAULT = NULL;
    RAPL_Release_all_package_power_cap();
    RAPL_Finalize();
    //
    CPUFREQ_Finalize();
    if ( FP_CONF && FP_CONF != stdout ) {
	fclose( FP_CONF );
	FP_CONF = NULL;
    }
    INITIALIZED = false;
}

/* Initialize this library just after the MPI_Init function */
int POMPP_Init( int argc, char *argv[] ) {
    int myrank, nprocs, root=0;
    int iarg, offset, ipkg;
    char tmp[MAXSTRLEN], suffix[MAXSTRLEN], std[MAXSTRLEN];
    double et0, et1;
    double *cpu_caps, *dram_caps;
    //
    mode_t mask;
    et0 = MPI_Wtime();
    //
    mask = ~(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    umask( mask );

    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    FP_CONF = NULL;
    if ( myrank == root ) {
	offset = 0;
	suffix[0] = '\0';
	std[0] = '\0';
	for ( iarg=1; iarg<argc; iarg++ ) {
	    if ( strncmp( argv[iarg], "POMPP_PROC_OFFSET", 17 ) == 0 ) {
		strncpy( tmp, &(argv[iarg][18]), MAXSTRLEN );
		tmp[MAXSTRLEN-1] = '\0';
		offset = atoi( tmp );
		if ( offset < 0 ) offset = 0;
	    } else if ( strncmp( argv[iarg], "POMPP_SUFFIX", 12 ) == 0 ) {
		strncpy( suffix, &(argv[iarg][13]), MAXSTRLEN );
		suffix[MAXSTRLEN-1] = '\0';
	    } else if ( strncmp( argv[iarg], "POMPP_STDOUT", 12 ) == 0 ) {
		strncpy( std, &(argv[iarg][13]), MAXSTRLEN );
		std[MAXSTRLEN-1] = '\0';
	    }
	}
	if ( std[0] != '\0' ) FP_CONF=fopen( std, "w" );
	if ( FP_CONF == NULL ) FP_CONF = stdout;
    }
    MPI_Bcast( &offset, 1, MPI_INT, root, MPI_COMM_WORLD );
    MPI_Bcast( suffix, MAXSTRLEN, MPI_CHAR, root, MPI_COMM_WORLD );
    if ( FP_CONF ) {
	fprintf(FP_CONF,
		"##CONF filename of basic conf. information = %s\n", std );
	fprintf(FP_CONF, "##CONF proc. offset= %d\n", offset );
	if ( suffix[0] != '\0' )
	    fprintf(FP_CONF, "##CONF suffix= %s\n", suffix );
    }
    // Setting processor affinity
    POMPP_Get_pkg_info( offset, &NPKGS, &LPKG_ID );

    // Read environmet variables and configuration file
    if ( POMPP_Read_config_file() != 0 ) {
	printf("ERROR: Failure in POMPP_Read_config_file()\n");
	return -1;
    }
    if ( POMPP_Check_parameters() != 0 ) {
	printf("ERROR: Failire in POMPP_Check_parameters\n");
	return -1;
    }

    // append suffix to header of output file name
    if ( suffix[0] != '\0' ) {
	strcat( FILE_HEADER, "-" );
	strcat( FILE_HEADER, suffix );
	if ( myrank == root )
	    printf("##CONF file header = %s\n", FILE_HEADER );
    }
    /* ========== various initializations ============= */
    // Initialize CPUfreqlibs (setting CPU frequency)
    CPUFREQ_Init( CPUFREQ );

    // Initialize timer used in this library
    ETIME_Init();

    // Initialize RAPL interface
    RAPL_Init( FILE_HEADER, PROF_MODE );

    // Initialize functions to get the information of each section
    PMC_Get_data( &NGPMC, &NFPMC, &GPMC_EVENT_NAME, &FPMC_EVENT_NAME );
    SEC_Init( PROF_MODE, FILE_HEADER,
	    DEFAULT_CPU_POWER, DEFAULT_DRAM_POWER );

    // Initialize the frequency counting
    if ( FREQ_COUNT ) FREQ_Init( NPKGS, LPKG_ID );


    /* setting defualt power cap values */
    cpu_caps  = (double*)malloc( sizeof(double) * NPKGS );
    dram_caps = (double*)malloc( sizeof(double) * NPKGS );
    for ( ipkg=0; ipkg<NPKGS; ipkg++ ) {
	cpu_caps[ipkg] = DEFAULT_CPU_POWER;
	dram_caps[ipkg] = DEFAULT_DRAM_POWER;
    }
    RAPL_Set_power_caps( RAPL_PKG, cpu_caps );
    RAPL_Set_power_caps( RAPL_DRAM, dram_caps );

    /* Preparation to measure average performance of entire application */
    ENERGY_PKG0  = (uint64_t*)malloc( sizeof(uint64_t) * NPKGS );
    ENERGY_PP00  = (uint64_t*)malloc( sizeof(uint64_t) * NPKGS );
    ENERGY_DRAM0 = (uint64_t*)malloc( sizeof(uint64_t) * NPKGS );
    ENERGY_PKG1  = (uint64_t*)malloc( sizeof(uint64_t) * NPKGS );
    ENERGY_PP01  = (uint64_t*)malloc( sizeof(uint64_t) * NPKGS );
    ENERGY_DRAM1 = (uint64_t*)malloc( sizeof(uint64_t) * NPKGS );
    GPMC0 = alloc_u64_matrix( NPKGS, NGPMC );
    GPMC1 = alloc_u64_matrix( NPKGS, NGPMC );
    FPMC0 = alloc_u64_matrix( NPKGS, NFPMC );
    FPMC1 = alloc_u64_matrix( NPKGS, NFPMC );
    ENERGY_VAL = (double*)malloc( sizeof(double) * NPKGS * 3 );
    ETIME0 = ETIME_Get_etime();
    RAPL_Read( ENERGY_PKG0, ENERGY_PP00, ENERGY_DRAM0 );
    PMC_Read( GPMC0, FPMC0 );

    // Setting for interval timer
    if ( PROF_MODE == SEC_PROF ) {
	ITIMER_Append( RAPL_Update );
	if ( FREQ_COUNT ) ITIMER_Append( FREQ_Update );
    } else if ( PROF_MODE == ENE_PROF ) {
	ITIMER_Append( RAPL_Update );
	if ( FREQ_COUNT ) ITIMER_Append( FREQ_Update );
    } else {
	ITIMER_Append( RAPL_Update );
    }
    ITIMER_Start( INTERVAL_MSEC );
    MPI_Barrier( MPI_COMM_WORLD );
    et1 = MPI_Wtime();
    if ( FP_CONF ) {
	POMPP_Show_setting();
	fprintf(FP_CONF, "##CONF e-time for initialization = %5.3f\n\n\n",
		(et1-et0) );
    }
    INITIALIZED = true;

    return 0;
}

void pompp_init_() {
    POMPP_Init( 0, NULL );
}

void pompp_finalize_() {
    POMPP_Finalize();
}
