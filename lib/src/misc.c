/*
 * The PomPP software including libraries and tools is developed by the
 * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
 * research project supported by the JST, CREST research program.
 * The copyrights for code used under license from other parties are
 * included in the corresponding files.
 * 
 * Copyright (c) 2015, The PomPP research team.
 */

/* Definition the following functions,
 * 1. 2-D or 3-D array allocations
 * 2. String manipulations
 * */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "ric-def.h"

/* allocate 2-D array of double data */
double** alloc_dmatrix( const int m, const int n ) {
    int i;
    double **ip;
    ip = (double**)malloc( sizeof(double*) * m );
    ip[0] = (double*)malloc( sizeof(double) * m * n );
    for ( i=1; i<m; i++ ) ip[i] = ip[i-1] + n;
    return ip;
}

/* free 2-D array allocated by alloc_dmatrix function */
void dealloc_dmatrix( double **ip ) {
    if ( ip ) {
	if ( ip[0] ) free( ip[0] );
	free( ip );
    }
}

/* allocate 2-D array of char data */
char** alloc_cmatrix( const int ntok, const int maxtoklen ) {
    int i;
    char **s;
    s    = (char**)malloc( sizeof(char*) * ntok );
    s[0] = (char*)malloc( sizeof(char) * ntok * maxtoklen );
    for ( i=1; i<ntok; i++ ) s[i] = s[i-1] + maxtoklen;
    for ( i=0; i<ntok; i++ ) s[i][0] = '\0';
    return s;
}

/* free 2-D array allocated by alloc_cmatrix function */
void dealloc_cmatrix( char **s ) {
    if ( s != NULL ) {
	if ( s[0] != NULL ) free( s[0] );
	free( s );
    }
}

/* allocate 2-D array of unsigned 64bit integer data */
uint64_t** alloc_u64_matrix( const int m, const int n ) {
    uint64_t **ip;
    int i;
    ip    = (uint64_t**)malloc( sizeof(uint64_t*) * m );
    ip[0] = (uint64_t*)malloc( sizeof(uint64_t) * m * n );
    for ( i=1; i<m; i++ ) ip[i] = ip[i-1] + n;
    return ip;
}

/* free 2-D array allocated by alloc_u64_matrix function */
void dealloc_u64_matrix( uint64_t **ip ) {
    if ( ip ) {
	if ( ip[0] ) free( ip[0] );
	free( ip );
    }
}

/* allocate 3-D array of unsigned 64bit integer data */
uint64_t*** alloc_u64_tensor( const int k, const int m, const int n ) {
    uint64_t ***ip;
    int i, j;
    ip       = (uint64_t***)malloc( sizeof(uint64_t**) * k );
    ip[0]    = (uint64_t** )malloc( sizeof(uint64_t* ) * k * m );
    ip[0][0] = (uint64_t*  )malloc( sizeof(uint64_t  ) * k * m * n );
    for ( i=1; i<k; i++ ) ip[i] = ip[i-1] + m;
    for ( i=1; i<m; i++ ) ip[0][i] = ip[0][i-1] + n;
    for ( i=1; i<k; i++ ) {
	for ( j=0; j<m; j++ ) ip[i][j] = ip[i-1][j] + m * n;
    }
    return ip;
}

/* free 3-D array allocated by alloc_u64_tensor function */
void dealloc_u64_tensor( uint64_t ***ip ) {
    if ( ip ) {
	if ( ip[0] ) {
	    if ( ip[0][0] ) free( ip[0][0] );
	    free( ip[0] );
	}
	free( ip );
    }
}

/* allocate 3-D array of double data */
double ***alloc_dtensor( const int n1, const int n2,
	const int n3 ) {
    double ***ip;
    int i, j;
    ip = (double***)malloc( sizeof(double**) * n1 );
    ip[0] = (double**)malloc( sizeof(double*) * n1 * n2 );
    for ( i=1; i<n1; i++ ) ip[i] = ip[i-1] + n2;
    ip[0][0] = (double*)malloc( sizeof(double) * n1 * n2 * n3 );
    for ( i=1; i<n1; i++ ) ip[i][0] = ip[i-1][0] + n2*n3;
    for ( i=0; i<n1; i++ ) {
	for ( j=1; j<n2; j++ ) ip[i][j] = ip[i][j-1] + n3;
    }
    return ip;
}

/* free 3-D array allocated by alloc_dtensor function */
void dealloc_dtensor( double ***ip ) {
    if ( ip ) {
	if ( ip[0] ) {
	    if ( ip[0][0] ) free( ip[0][0] );
	    free( ip[0] );
	}
	free( ip );
    }
}

/* allocate 3-D array of char data */
char ***alloc_ctensor( const int n1, const int n2,
	const int n3 ) {
    char ***ip;
    int i, j;
    ip = (char***)malloc( sizeof(char**) * n1 );
    ip[0] = (char**)malloc( sizeof(char*) * n1 * n2 );
    for ( i=1; i<n1; i++ ) ip[i] = ip[i-1] + n2;
    ip[0][0] = (char*)malloc( sizeof(char) * n1 * n2 * n3 );
    for ( i=1; i<n1; i++ ) ip[i][0] = ip[i-1][0] + n2*n3;
    for ( i=0; i<n1; i++ ) {
	for ( j=1; j<n2; j++ ) ip[i][j] = ip[i][j-1] + n3;
    }
    return ip;
}

/* free 3-D array allocated by alloc_ctensor function */
void dealloc_ctensor( char ***ip ) {
    if ( ip ) {
	if ( ip[0] ) {
	    if ( ip[0][0] ) free( ip[0][0] );
	    free( ip[0] );
	}
	free( ip );
    }
}

/* allocate 2-D array of int data */
int **alloc_imatrix( const int n1, const int n2 ) {
    int **ip;
    int i;
    ip = (int**)malloc( sizeof(int*) * n1 );
    ip[0] = (int*)malloc( sizeof(int) * n1 * n2 );
    for ( i=1; i<n1; i++ ) ip[i] = ip[i-1] + n2;
    return ip;
}

/* free 2-D array allocated by alloc_imatrix function */
void dealloc_imatrix( int **ip ) {
    if ( ip ) {
	if ( ip[0] ) free( ip[0] );
	free( ip );
    }
}

// ============================================================

extern int num_width( const int num );

/* Delete nongraphic charactors from leading and tail of target string */
void del_prepost_space( char s[] ) {
    int n, c, start, end;
    char strtmp[MAXSTRLEN];
    strcpy( strtmp, s );
    n = strlen( s );
    // search start & end points
    for ( start=0; start<n; start++ ) if ( isgraph(strtmp[start]) ) break;
    for ( end=n-1; end>=0; end-- ) if ( isgraph(strtmp[end]) ) break;
    c = end - start + 1;
    if ( c > 0 ) {
	strncpy( s, &strtmp[start], c );
	s[c] = '\0';
    } else {
	s[0] = '\0';
    }
}

/* Conver all charactors in target string to lower case in place */
void misc_tolower( char s[] ) {
    int i, n;
    n = strlen( s );
    for ( i=0; i<n; i++ ) s[i] = tolower( s[i] );
}

/* Split the target string to tokens with the given separators */
int misc_split( const char s[], const char sep[], char **token,
	const int maxtoken, const int maxtoklen ) {
    char *p, strtmp[MAXSTRLEN];
    int count=0, len;
    strcpy( strtmp, s );
    p = strtok( strtmp, sep );
    if ( p == NULL ) return 0;
    len = strlen( p );
    if ( len >= maxtoklen ) len = maxtoklen - 1;
    strncpy( token[count], p, len );
    token[count][len] = '\0';
    count++;
    while ( (p=strtok( NULL, sep )) != NULL && count < maxtoken ) {
	len = strlen( p );
	if ( len >= maxtoklen ) len = maxtoklen - 1;
	strncpy( token[count], p, len );
	token[count][len] = '\0';
	count++;
    }
    return count;
}
