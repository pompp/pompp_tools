/*
 * The PomPP software including libraries and tools is developed by the
 * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
 * research project supported by the JST, CREST research program.
 * The copyrights for code used under license from other parties are
 * included in the corresponding files.
 * 
 * Copyright (c) 2015, The PomPP research team.
 */

/** Functions to use the interval timer (SIGALRM)
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <signal.h>

#include "ric-def.h"

#define MAX_NFUNC 8

static void (*FUNCS[MAX_NFUNC])(void);

static int NFUNC = 0;

/* Append the function to be called from the interval timer */
int ITIMER_Append( void (*func)(void) ) {
    NFUNC++;
    if ( NFUNC > MAX_NFUNC ) {
	printf("ERROR in ITIMER_Append: No enough space\n");
	return -1;
    }
    FUNCS[NFUNC-1] = func;
    return -1;
}

/* Execution all functions registrated via ITIMER_Append function */
static void exec_funcs( int sig ) {
    int ifunc;
    for ( ifunc=0; ifunc<NFUNC; ifunc++ ) FUNCS[ifunc]();
}

static unsigned int	INTERVAL_USEC = 10000; // interval (in usec)

static void dummy_code( int sig ) {
    printf("dummy (sig=%d)\n", sig );
}


static void set_interval( const unsigned int interval_usec ) {
    if ( interval_usec < 10000 ) INTERVAL_USEC = 10000;
    else                         INTERVAL_USEC = interval_usec;
}

/* Finalize the use of interval timer */
static void ITIMER_Finalize() {
    struct sigaction sa;
    memset( &sa, '\0', sizeof(sa) );
    sa.sa_handler = dummy_code;
    sa.sa_flags   = SA_ONSTACK;
    sigaction( SIGALRM, &sa, NULL );
    setitimer( ITIMER_REAL, NULL, NULL );	// stop itimer
    NFUNC = 0;
}

/* Initialize the use of interval timer */
int ITIMER_Start( const int interval_msec ) {
    int interval_usec;
    struct itimerval itv;
    struct timeval it_interval;
    struct sigaction sa;
    static int called = false;
    if ( NFUNC == 0 ) return 0;
    if ( called ) return 0;

    interval_usec = interval_msec * 1000;
    set_interval( interval_usec );

    memset( &sa, '\0', sizeof(sa) );
    sa.sa_handler = exec_funcs;
    sa.sa_flags   = SA_RESTART;
    sigaction( SIGALRM, &sa, NULL );

    it_interval.tv_sec  = (INTERVAL_USEC/1000000);
    it_interval.tv_usec = (INTERVAL_USEC%1000000);
    itv.it_interval     = it_interval;
    itv.it_value        = it_interval;
    setitimer( ITIMER_REAL, &itv, NULL );
    atexit( ITIMER_Finalize );
    called = true;
    return 0;
}
