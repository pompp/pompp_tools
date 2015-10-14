#ifndef _RIC_DEF_H_
#define _RIC_DEF_H_

#include <stdio.h>
#include <stdint.h>

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

#ifndef dbg
#define dbg(...) \
    ( printf("%s %u @%s:",__FILE__, __LINE__, __func__), \
      printf(" "__VA_ARGS__), fflush(stdout) )
#endif

#ifdef USE_MPI
#include <mpi.h>
#else
#include "mpi-dummy.h"
#endif

#ifdef _OPENMP
#include <omp.h>
#else
static int omp_get_max_threads() { return 1; }
static int omp_get_num_threads() { return 1; }
static int omp_get_thread_num() { return 0; }
static int omp_in_parallel() { return false; }
#endif

#ifdef __MAIN__
FILE *FP_CONF = NULL;
#else
extern FILE *FP_CONF;	// file descriptor of output config. data
#endif

// code of processor binding mode of MPI
#define MPI_COMPACT 0
#define MPI_SCATTER 1

// code of profiling mode
#define NO_PROF		0
#define SEC_PROF	1
#define ENE_PROF	2

#define MAXSTRLEN	512	// max. length of general string
#define MAXTOKLEN	128	// max. length of separated token
#define MAXTOKEN	64	// max. number of tokens in a string

// section
#define MAXNSEC		128	// max. number of sections
#define MAXSECLEN	48	// max. length of section name (+1)
// knob
#define MAXKNOBS	8	// max. number of knob
#define MAXKNOBLEN	8	// max. length of knob name (+1)

extern char** alloc_cmatrix( const int ntok, const int maxtoklen );
extern uint64_t** alloc_u64_matrix( const int m, const int n );
extern uint64_t*** alloc_u64_tensor(const int k, const int m, const int n );
extern double ***alloc_dtensor( const int k, const int m, const int n );
extern char ***alloc_ctensor( const int k, const int m, const int n );
extern int **alloc_imatrix( const int m, const int n );
extern double** alloc_dmatrix( const int m, const int n );

extern void dealloc_cmatrix( char **s );
extern void dealloc_u64_matrix( uint64_t **ip );
extern void dealloc_u64_tensor( uint64_t ***ip );
extern void dealloc_dtensor( double ***ip );
extern void dealloc_ctensor( char ***ip );
extern void dealloc_imatrix( int **ip );
extern void dealloc_dmatrix( double **ip );

extern void del_prepost_space( char s[] );
extern void misc_tolower( char s[] );
extern int misc_split( const char s[], const char sep[], char **token,
	const int maxtoken, const int maxtoklen );

#endif
