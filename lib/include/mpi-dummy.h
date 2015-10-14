#ifndef _MPI_DUMMY_DEF_H_
#define _MPI_DUMMY_DEF_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#define MPI_CHAR	0
#define MPI_INT		1
#define MPI_DOUBLE	2
#define MPI_LONG_LONG_INT	3
#define MPI_UNSIGNED_LONG_LONG  4

#define MPI_COMM_WORLD	1

#define MPI_SUM		1

#define MPI_THREAD_SINGLE	0
#define MPI_THREAD_FUNNELED	1
#define MPI_THREAD_SERIALIZED	2
#define MPI_THREAD_MULTIPLE	3

#define MPI_MAX_PROCESSOR_NAME	512

typedef int MPI_Comm;
typedef int MPI_Status;

static int MPI_Init_thread( int *argc, char*** argv, int require,
	int* provided ) {
    *provided = require;
    return 0; }
static int MPI_Comm_size( MPI_Comm comm, int *nprocs ) {
    *nprocs = 1;
    return 0;
}

static int MPI_Comm_rank( MPI_Comm comm, int *myrank ) {
    *myrank = 0;
    return 0;
}

static void MPI_Abort( MPI_Comm comm, int errcode ) { exit( errcode ); }

static int MPI_Bcast( void* buf, int count, int type, int root,
	MPI_Comm comm ) { return 0; }

static int MPI_Barrier( MPI_Comm comm ) { return 0; }

static int MPI_Finalize() { return 0; }

static double MPI_Wtime() {
    struct timeval tv;
    gettimeofday( &tv, NULL );
    return (double)tv.tv_sec+(double)tv.tv_usec*1.e-6;
}

static int MPI_Recv( void* dest, int count, int dtype,
	int root, int tag, MPI_Comm comm, MPI_Status *status ) {
    return 0;
}

static int MPI_Send( void* src, int count, int dtype,
	int root, int tag, MPI_Comm comm ) {
    return 0;
}

static int MPI_Allreduce( void* src, void* buf, int count,
	int type, int op, MPI_Comm comm ) {
    int tsize;
    if ( type == MPI_DOUBLE ) {
	tsize = sizeof(double);
    } else if ( type == MPI_INT ) {
	tsize = sizeof(int);
    } else if ( type == MPI_CHAR ) {
	tsize = sizeof(char);
    } else if ( type == MPI_LONG_LONG_INT ) {
	tsize = sizeof(long long int);
    } else if ( type == MPI_UNSIGNED_LONG_LONG ) {
	tsize = sizeof(long long int);
    } else {
	printf("ERROR: Illegal type (%d)\n", type );
	return -1;
    }
    memcpy( buf, src, tsize*count );
    return 0;
}

static int MPI_Reduce( void* src, void* buf, int count,
	int type, int op, int root, MPI_Comm comm ) {
    int tsize;
    if ( type == MPI_DOUBLE ) {
	tsize = sizeof(double);
    } else if ( type == MPI_INT ) {
	tsize = sizeof(int);
    } else if ( type == MPI_CHAR ) {
	tsize = sizeof(char);
    } else if ( type == MPI_LONG_LONG_INT ) {
	tsize = sizeof(long long int);
    } else if ( type == MPI_UNSIGNED_LONG_LONG ) {
	tsize = sizeof(unsigned long long);
    } else {
	printf("ERROR: Illegal type (%d)\n", type );
	return -1;
    }
    memcpy( buf, src, tsize*count );
    return 0;
}

static int MPI_Get_processor_name( char *hostname, int *len ) {
    int ierr;
    ierr = gethostname( hostname, (size_t)MPI_MAX_PROCESSOR_NAME );
    (*len) = strlen( hostname );
    return ierr;
}

#endif
