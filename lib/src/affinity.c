/*
 * The PomPP software including libraries and tools is developed by the
 * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
 * research project supported by the JST, CREST research program.
 * The copyrights for code used under license from other parties are
 * included in the corresponding files.
 * 
 * Copyright (c) 2015, The PomPP research team.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <limits.h>

#include "ric-def.h"
#include "msr.h"

static int LPKG_ID = -1;
static int NPKGS = 0;

/* Set Affinity for MPI process
 * (Private function)
 * */
static int POMPP_Set_affinity( const int offset ) {
    int nprocs, myrank, root=0;
    int cid0, NNODES, mode;
    int lpkg_id, nthreads, nodes;
    int npkgs_per_node, ncores_per_pkg;
    int ncores_per_node, npkgs_per_proc, nprocs_per_node;
    char *p;
    /* Getting basis information */
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    nthreads = omp_get_max_threads();
    npkgs_per_node  = MSR_Get_npkgs_per_node();
    ncores_per_pkg  = MSR_Get_ncores_per_pkg();
    ncores_per_node = MSR_Get_ncores_per_node();
    if ( myrank == root ) {
	mode = MPI_COMPACT;
	if ( (p=getenv("POMPP_MPI_ASSIGNMENT")) ) {
	    mode = atoi( p );
	    if ( mode != MPI_COMPACT && mode != MPI_SCATTER )
		mode = MPI_COMPACT;
	}
	NNODES = 0;
	if ( (p=getenv("POMPP_NNODES")) ) {
	    NNODES = atoi( p );
	    if ( NNODES < 1 ) NNODES = 0;
	}
    }
    MPI_Bcast( &mode, 1, MPI_INT, root, MPI_COMM_WORLD );
    MPI_Bcast( &NNODES, 1, MPI_INT, root, MPI_COMM_WORLD );
    /* --- Check parameters and settings --- */
    // The # of threads per process must be a multiple of the # of cores
    // on a processor.
    npkgs_per_proc = nthreads / ncores_per_pkg;
    if ( (nthreads % ncores_per_pkg) != 0 ) {
	dbg("ERROR: # of threads (%d) must be a multiple of"
		" # of cores per package (%d)\n",
		nthreads, ncores_per_pkg );
	MPI_Abort( MPI_COMM_WORLD, 10 );
    }
    // The # of cores on a node must be a multiple of # of threads
    // per process.
    // (This limitation will be removed)
    if ( (ncores_per_node % nthreads) != 0 ) {
	dbg("ERROR: # of cores in a node (%d) must be a multiple of"
		" # of threads per process (%d)\n",
		ncores_per_node, nthreads );
	MPI_Abort( MPI_COMM_WORLD, 11 );
    }
    nprocs_per_node = ncores_per_node / nthreads;
    /* Get the # of nodes used in this MPI job ("nnodes") */
    // Get "nnodes" from environment variable POMPP_NNODES (if it is set)
    if ( NNODES > 0 ) {
	// Check wether needed nodes is assigned or not
	if ( (nthreads*nprocs) > (NNODES*ncores_per_node) ) {
	    dbg("ERROR: No enough cores ( %d cores are needed, but"
		    " %d cores are available.\n", (nthreads*nprocs),
		    (NNODES*ncores_per_node) );
	    MPI_Abort( MPI_COMM_WORLD, 12 );
	}
	nodes = NNODES;
    } else { // The "nnodes" value is estimated by # of process, etc.
	nodes = (int)(nprocs/nprocs_per_node)
	    + ( (nprocs%nprocs_per_node)==0 ? 0 : 1 );
    }
    /* Determine the core assignment for each MPI process
     * (This may be conflict the setting by compiler runtime
     * and/or MPI library) */
    if ( mode == MPI_COMPACT ) {
	lpkg_id = ((myrank+offset) % nprocs_per_node) * npkgs_per_proc;
    } else if ( mode == MPI_SCATTER ) {
	lpkg_id = (int)((myrank+offset)/nodes)*npkgs_per_proc;
    }
    /* To reduce output
    // Output the information about the processor affinity
    if ( FP_CONF ) {
	fprintf(FP_CONF, "##AF MPI assignment mode    = %s\n",
		(mode==MPI_COMPACT? "compact" : "scatter" ) );
	fprintf(FP_CONF, "##AF # of nodes (given)     = %d\n", NNODES );
	fprintf(FP_CONF, "##AF # of nodes (decided)   = %d\n", nodes );
	fprintf(FP_CONF, "##AF # of packages per node = %d\n",
		npkgs_per_node );
	fprintf(FP_CONF, "##AF # of cores per package = %d\n",
		ncores_per_pkg );
	fprintf(FP_CONF, "##AF # of cores per node    = %d\n",
		ncores_per_node );
	fprintf(FP_CONF, "##AF # of max. threads      = %d\n", nthreads );
	fprintf(FP_CONF, "##AF process assign offset  = %d\n", offset );
    }
    */
    
    cid0 = lpkg_id * ncores_per_pkg;

    /* Set the thread affinity */
#pragma omp parallel
    {
	int mythread;
	cpu_set_t cpu;
	mythread = omp_get_thread_num();
	CPU_ZERO( &cpu );
	CPU_SET( cid0+mythread, &cpu );

	if ( sched_setaffinity( 0, sizeof(cpu), &cpu ) != 0 )
	    dbg("ERROR: rank=%d: Failure in sched_setaffinity\n", myrank );
    }

    return 0;
}

/* Settig the processor affinity (only at first call) and
 * getting the # of used CPUs and leading CPU ID in the node
 * of this MPI process
 * */
int POMPP_Get_pkg_info( const int offset, int *npkgs, int *lpkg_id ) {
    int npkgs_per_node, ncores_per_pkg;
    static int called = false;
    if ( called ) {
	*npkgs  = NPKGS;
	*lpkg_id = LPKG_ID;
	return 0;
    }
    POMPP_Set_affinity( offset );
    npkgs_per_node = MSR_Get_npkgs_per_node();
    ncores_per_pkg = MSR_Get_ncores_per_pkg();
    NPKGS = 0;
    LPKG_ID = INT_MAX;
#pragma omp parallel
    {
	int tid, pkg_id, lcore_id;
	cpu_set_t mask;
	tid = omp_get_thread_num();
	if ( (tid % ncores_per_pkg) == 0 ) {
	    sched_getaffinity( 0, sizeof(cpu_set_t), &mask );
	    for ( pkg_id=0; pkg_id<npkgs_per_node; pkg_id++ ) {
		lcore_id = MSR_Get_lcore_id( pkg_id );
		if ( CPU_ISSET( lcore_id, &mask ) ) {
#pragma omp critical
		    {
			if ( pkg_id < LPKG_ID ) LPKG_ID = pkg_id;
			NPKGS++;
		    }
		}
	    }
	}
    }
    *npkgs   = NPKGS;
    *lpkg_id = LPKG_ID;
    called = true;
    return 0;
}
