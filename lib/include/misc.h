#ifndef _MISC_H_
#define _MISC_H_

#include <stdio.h>
#include <stdint.h>

#define MAXSTRLEN	512
#define MAXTOKLEN	128
#define MAXTOKEN	64

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
