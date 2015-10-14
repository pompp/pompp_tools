/*
 * The PomPP software including libraries and tools is developed by the
 * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
 * research project supported by the JST, CREST research program.
 * The copyrights for code used under license from other parties are
 * included in the corresponding files.
 * 
 * Copyright (c) 2015, The PomPP research team.
 */

/* Functions to read from and write to the MSRs of Intel processors */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// for open, close
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ric-def.h"

#define MAXFILENAME	64	// max. length of device file name

static int NCORES_PER_PKG=0;	// # of cores per CPU (package)
static int NPKGS_PER_NODE=0;	// # of packages per node
static int NCORES_PER_NODE=0;	// # of cores per node

/* static variables used in read/write MSR registers */
static char **msr_file_name;	// device file names for each core
static int  *msr_file_desc;	// devive file descriptors for each core
static int *LCORE_ID;		// leading core IDs for each package

static char *machine_setting="/homes/wada/x86_64/etc/machine.conf";

/* remove nongraphic charactors from the head of target string */
static int lstrip( char *s ) {
    int len, ipos, i;
    len = strlen( s );
    if ( len < 1 ) return 0;
    for ( ipos=0; ipos<len; ipos++ ) if ( s[ipos] >= ' ' ) break;
    if ( ipos == 0 ) return 0;
    for ( i=ipos; i<len; i++ ) s[i-ipos] = s[i];
    s[len-ipos] = '0';
    return 0;
}

/* replace charactor c1 to c2 in target string s */
static int replace( char *s, const char c1, const char c2 ) {
    int i, len;
    len = strlen( s );
    for (i=0; i<len; i++ ) if ( s[i] == c1 ) s[i] = c2;
    return 0;
}

static int POMPP_Read_machine_setting( int* npkgs_per_node,
	int* ncores_per_pkg ) {
    FILE *fp;
    char line[MAXSTRLEN], *p;
    *npkgs_per_node = 0;
    *ncores_per_pkg = 0;
    if ( (fp=fopen(machine_setting, "r")) == NULL ) {
	fprintf( stderr, "Warning: Failure to open machine setting file"
		" %s\n", machine_setting );
	return 0;
    }
    while ( fgets(line, (MAXSTRLEN-1), fp) != NULL ) {
	lstrip( line );
	replace( line, '=', ' ' );
	p = strtok( line, " \t");
	if ( strcmp( p, "POMPP_NPKGS_PER_NODE" ) == 0 ) {
	    p = strtok( NULL, " \t");
	    *npkgs_per_node = atoi( p );
	    // debug
	    printf("# of PKGS per node = %d\n", *npkgs_per_node );
	} else if ( strcmp( p, "POMPP_NCORES_PER_PKG" ) == 0 ) {
	    p = strtok( NULL, " \t");
	    *ncores_per_pkg = atoi( p );
	    // debug
	    printf("# of CORES per PKG = %d\n", *ncores_per_pkg );
	}
    }
    // check parameters
    if ( *npkgs_per_node < 1 ) *npkgs_per_node = 0;
    if ( *ncores_per_pkg < 1 ) *ncores_per_pkg = 0;
    return 0;

}

static void finalize_msr() {
    int core_id;
    for ( core_id=0; core_id<NCORES_PER_NODE; core_id++ )
	close( msr_file_desc[core_id] );
    free( msr_file_desc );
    free( LCORE_ID );
    dealloc_cmatrix( msr_file_name );
}

/* initialization function called from
 * out of the thread parallel region */
static int init_msr() {
    int core_id, fd, pkg_id;
    int myrank, root=0;
    char *p;
    int npkgs_per_node, ncores_per_pkg;
    static int called = false;
    if ( called ) return 0;
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    if ( myrank == root ) {
	// read machine setting file
	POMPP_Read_machine_setting( &NPKGS_PER_NODE, &NCORES_PER_PKG );
	// obtain the # of packages per node
	if ( (p=getenv("POMPP_NPKGS_PER_NODE")) ) NPKGS_PER_NODE=atoi( p );
	if ( NPKGS_PER_NODE < 1 ) {
	    dbg("ERROR: Illegal # of packages per node (%d)\n",
		    NPKGS_PER_NODE );
	    MPI_Abort( MPI_COMM_WORLD, 1 );
	}
	// obtain the # of cores per package
	if ( (p=getenv("POMPP_NCORES_PER_PKG")) ) NCORES_PER_PKG=atoi( p );
	if ( NCORES_PER_PKG < 1 ) {
	    dbg("ERROR: Illegal # of cores per package (%d)\n",
		    NCORES_PER_PKG );
	    MPI_Abort( MPI_COMM_WORLD, 2 );
	}
	/* to reduce output
	if ( FP_CONF ) {
	    fprintf(FP_CONF, "##MSR  # of packages per node = %d\n",
		    NPKGS_PER_NODE );
	    fprintf(FP_CONF, "##MSR  # of cores per package = %d\n",
		    NCORES_PER_PKG );
	}
	*/
    }
    MPI_Bcast( &NPKGS_PER_NODE, 1, MPI_INT, root, MPI_COMM_WORLD );
    MPI_Bcast( &NCORES_PER_PKG, 1, MPI_INT, root, MPI_COMM_WORLD );

    NCORES_PER_NODE = NPKGS_PER_NODE * NCORES_PER_PKG;
    msr_file_name = alloc_cmatrix( NCORES_PER_NODE, MAXFILENAME );
    msr_file_desc = (int*)malloc( sizeof(int) * NCORES_PER_NODE );
    LCORE_ID = (int*)malloc( sizeof(int) * NPKGS_PER_NODE );
    for ( core_id=0; core_id<NCORES_PER_NODE; core_id++ ) {
	sprintf( msr_file_name[core_id], "/dev/cpu/%d/msr", core_id );
	fd = open( msr_file_name[core_id], O_RDWR );
	if ( fd == -1 ) {
	    printf("MSR device file %s cannot be opened\n",
		    msr_file_name[core_id] );
	    return -1;
	}
	msr_file_desc[core_id] = fd;
    }
    for ( pkg_id=0; pkg_id<NPKGS_PER_NODE; pkg_id++ )
	LCORE_ID[pkg_id] = pkg_id * NCORES_PER_PKG;
    atexit( finalize_msr );
    called = true;
    return 0;
}

/** Provide leading core ID of the package */
int MSR_Get_lcore_id( const int pkg_id ) {
    init_msr();
    if ( pkg_id<0 || pkg_id>=NPKGS_PER_NODE ) {
	dbg("ERROR: Illegal package ID (%d)\n", pkg_id );
	exit(1);
    }
    return LCORE_ID[pkg_id];
}

/** Provide the # of cores per node */
int MSR_Get_ncores_per_node() {
    init_msr();
    return NCORES_PER_NODE;
}

/** Provide the # og cores per package */
int MSR_Get_ncores_per_pkg() {
    init_msr();
    return NCORES_PER_PKG;
}

/** Provide the # of packages per node */
int MSR_Get_npkgs_per_node() {
    init_msr();
    return NPKGS_PER_NODE;
}

/* write data to reg-th MSR register */
int wrmsr( const int core_id, uint32_t reg, uint64_t data ) {
    int fd;
    size_t sz;
    init_msr();
    if ( core_id < 0 || core_id >= NCORES_PER_NODE ) {
	dbg("Illegal number of ID of core (%d)\n", core_id );
	return -1;
    }
    fd = msr_file_desc[core_id];
    if ( (sz=pwrite( fd, &data, sizeof(data), reg )) != sizeof(data) ) {
	dbg( "in pwrite:\n"
		"    msr_file_name = %s\n"
		"    reg           = 0x%03x\n"
		"    data          = 0x%04x\n"
		"    core id       = %d\n"
		" (%d vs. %d)\n",
		msr_file_name[core_id], reg, (uint32_t)data, core_id,
		(int)sz, (int)sizeof(data) );
	return -1;
    }
    return 0;
}

/* read data from reg-th MSR register */
int rdmsr( const int core_id, uint32_t reg, uint64_t *data ) {
    int fd;
    init_msr();
    if ( core_id < 0 || core_id >= NCORES_PER_NODE ) {
	dbg("Illegal number of ID of core (%d)\n", core_id );
	return -1;
    }
    fd = msr_file_desc[core_id];
    if ( pread( fd, data, sizeof(*data), reg ) != sizeof(*data) ) {
	dbg( "in pread:\n"
		"    msr_file_name = %s\n"
		"    reg           = 0x%03x\n"
		"    data          = 0x%04x\n",
		msr_file_name[core_id], reg, (uint32_t)(*data) );
	return -1;
    }
    return 0;
}

/* get information using CPUID command */
void get_cpuid( int param,
	unsigned int *eax, unsigned int *ebx,
	unsigned int *ecx, unsigned int *edx ) {
    __asm__( "cpuid"
	    : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
	    : "0" (param) );
}

void get_cpuid2( int param0, int param1,
	unsigned int *eax, unsigned int *ebx,
	unsigned int *ecx, unsigned int *edx ) {
    __asm__( "cpuid"
	    : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
	    : "a" (param0), "c" (param1) );
}

/* Read time stamp counter */
uint64_t rdtsc() {
    uint64_t data;
    __asm__("rdtsc" : "=A" (data) );
    return data;
}
