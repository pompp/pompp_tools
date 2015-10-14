/*
 * The PomPP software including libraries and tools is developed by the
 * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
 * research project supported by the JST, CREST research program.
 * The copyrights for code used under license from other parties are
 * included in the corresponding files.
 * 
 * Copyright (c) 2015, The PomPP research team.
 */

/* Setting the CPU frequency via CPUfreqlibs.
 * The "flock" function is used to set the governor exclusively.
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/file.h>	// for "flock" function

#include "ric-def.h"
#include "msr.h"
#include "affinity.h"

static char const * cpufreq_base_str = "/sys/devices/system/cpu/cpu";
static char const * cpufreq_governor_fname = "/cpufreq/scaling_governor";
static char const * cpufreq_set_fname = "/cpufreq/scaling_setspeed";
static char const * cpufreq_governor_str = "userspace";
static char const * cpufreq_governor_reset_str = "ondemand";
static char const * cpufreq_scaling_min_freq = "/cpufreq/scaling_min_freq";
static char const * cpufreq_min_freq_str = "1200000";

static int FREQ_CTRLD= false;

static int NPKGS   = 0;
static int LPKG_ID = -1;

/* Finalize the use of CPUfreqlibs */
void CPUFREQ_Finalize() {
    char fname[256];
    FILE *fp;
    // debug
    int myrank, len;
    // for "flock"
    char gov_status[256];
    FILE *fp0;

    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    if ( FREQ_CTRLD == false ) return;
    /* Setting governors of all cores to "ondemand" exclusively using
     * "flock" function if the governors are set to "userspace" */
    sprintf( fname, "%s%d%s", cpufreq_base_str, 0,
	    cpufreq_governor_fname );
    if ( (fp0=fopen(fname, "r+")) == NULL) {
	fprintf(stderr, "File (%s) cannot be opend to write...\n",
		fname);
	return;
    }
    flock( fileno(fp0), LOCK_EX );
    fseek( fp0, 0L, SEEK_SET );
    fgets( gov_status, 255, fp0 );
    if ( strncmp( gov_status, "userspace", 9 ) == 0 ) {
	int ncores, cid;
	ncores = MSR_Get_ncores_per_node();
	fseek( fp0, 0L, SEEK_SET );
	fwrite( cpufreq_governor_reset_str, sizeof(char),
		strlen(cpufreq_governor_reset_str), fp0 );
	fflush( fp0 );
	for ( cid=1; cid<ncores; cid++ ) {
	    sprintf(fname, "%s%d%s", cpufreq_base_str, cid,
		    cpufreq_governor_fname );
	    if ( ( fp=fopen(fname, "r+")) == NULL ) {
		fprintf(stderr, "File (%s) cannot be opend to write...\n",
			fname);
		return;
	    }
	    fwrite( cpufreq_governor_reset_str, sizeof(char),
		    strlen(cpufreq_governor_reset_str), fp );
	    fflush( fp );
	    fclose(fp);
	}
    }
    flock( fileno(fp0), LOCK_UN );
    fclose( fp0 );
    FREQ_CTRLD = false;
    NPKGS   = 0;
    LPKG_ID = -1;
}

/* Initialize and set CPU frequency using CPUfreqlibs */
int CPUFREQ_Init( const int cpufreq ) {
    int myrank, root=0;
    char fname[256], line[256];
    char env_str[256], ttt[256];
    FILE *fp;
    size_t len, len0;
    // for "flock"
    char gov_status[256];
    FILE *fp0;

    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    FREQ_CTRLD = (cpufreq > 0 ? true : false );
    if ( myrank == root ) printf("##CF CPUFREQ= %d\n", cpufreq );

    if ( FREQ_CTRLD == false ) return 0;
    /* Setting the governors of all cores to "userspace" exclusively
     * using "flock" function if the governors are set to "ondemand" */
    sprintf(fname, "%s%d%s", cpufreq_base_str, 0, cpufreq_governor_fname);
    if ( (fp0=fopen(fname, "r+")) == NULL) {
	fprintf(stderr, "File (%s) cannot be opend to write...\n",
		fname );
	return -1;
    }
    flock( fileno(fp0), LOCK_EX );
    fseek( fp0, 0L, SEEK_SET );
    fgets( gov_status, 255, fp0 );
    if ( strncmp( gov_status, "ondemand", 8 ) == 0 ) {
	int ncores, cid;
	ncores = MSR_Get_ncores_per_node();
	fseek( fp0, 0L, SEEK_SET );
	fwrite(cpufreq_governor_str, sizeof(char),
		strlen(cpufreq_governor_str)+1, fp0 );
	fflush( fp0 );
	for ( cid=1; cid<ncores; cid++ ) {
	    sprintf( fname, "%s%d%s", cpufreq_base_str, cid,
		    cpufreq_governor_fname );
	    if ( (fp=fopen(fname, "r+")) == NULL) {
		fprintf(stderr, "File (%s) cannot be opend to write...\n",
			fname);
		return -1;
	    }
	    fwrite( cpufreq_governor_str, sizeof(char),
		    strlen(cpufreq_governor_str)+1, fp );
	    fflush( fp );
	    fclose(fp);
	}
	/* Setting lowest CPU frequencies for all cores */
	for ( cid=0; cid<ncores; cid++ ) {
	    sprintf(fname, "%s%d%s", cpufreq_base_str, cid,
		    cpufreq_scaling_min_freq );
	    if ( (fp=fopen(fname, "r+")) == NULL) {
		fprintf(stderr, "File (%s) cannot be opend to write...\n",
			fname );
		return -1;
	    }
	    sprintf( ttt, "%d", 1200000 );
	    fwrite( ttt, sizeof(char), strlen(ttt)+1, fp );
	    fclose( fp );
	}
	/* Setting CPU frequency */
	for ( cid=0; cid<ncores; cid++ ) {
	    sprintf(fname, "%s%d%s", cpufreq_base_str, cid,
		    cpufreq_set_fname);
	    if ( (fp=fopen(fname, "r+")) == NULL ) {
		fprintf(stderr, "File (%s) cannot be opend to write...\n",
			fname);
		return -1;
	    }
	    sprintf(env_str, "%d", cpufreq );
	    len0 = strlen(env_str)+1;
	    len = fwrite(env_str, sizeof(char), strlen(env_str) + 1, fp);
	    fflush( fp );
	    fclose( fp );
	}
    }
    flock( fileno(fp0), LOCK_UN );
    fclose( fp0 );
    return 0;
}
