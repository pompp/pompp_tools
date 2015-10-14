/*
 * The PomPP software including libraries and tools is developed by the
 * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
 * research project supported by the JST, CREST research program.
 * The copyrights for code used under license from other parties are
 * included in the corresponding files.
 * 
 * Copyright (c) 2015, The PomPP research team.
 */

/* The code of a function to get the unique node ID in the system.
 * This is specific code for HA8K supercomputer system in Kyushu Univ.
 * This code MUST be modified for the system you will use.
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_MPI
#include <mpi.h>
#define MPI_MODE 1
#else
#include "mpi-dummy.h"
#define MPI_MODE 0
#endif

#include "ric-def.h"

/* Provide unique node ID in the target (cluster) system of 
 * this MPI process.
 * In the HA8K system, last three characters of the host name indicate
 * the node number in the system, which can be used as the unique node ID.
 * Thus unique node ID of HA8K system is obtained from the host name.
 * */
int NODE_Get_id() {
    int len;
    char hostname[MPI_MAX_PROCESSOR_NAME], snum[5];
    MPI_Get_processor_name( hostname, &len );
    strncpy( snum, &hostname[6], 3 );
    snum[3] = '\0';
    return (atoi( snum )-1);
}

/*
// main for testing
int main( int argc, char *argv[] ) {
    int gid, myrank, nprocs, irank, tag=0;
    MPI_Status status;
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    gid = NODE_Get_id();
    if ( myrank == 0 ) {
	printf(" rank= %2d ID= %d\n", 0, gid );
	for ( irank=1; irank<nprocs; irank++ ) {
	    MPI_Recv(&gid, 1, MPI_INT, irank, tag, MPI_COMM_WORLD, &status);
	    printf(" rank= %2d ID= %d\n", irank, gid );
	}
    } else {
	MPI_Send(&gid, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}
*/
