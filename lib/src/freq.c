/*
 * The PomPP software including libraries and tools is developed by the
 * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
 * research project supported by the JST, CREST research program.
 * The copyrights for code used under license from other parties are
 * included in the corresponding files.
 * 
 * Copyright (c) 2015, The PomPP research team.
 */

/* =====================================================
 * Functions to measure average CPU frequency
 * ===================================================== */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "ric-def.h"
#include "msr.h"
#include "etime.h"

#define IA32_PERF_STATUS	0x198

static double	*FREQ_SUM = NULL;
static double	ET_PREV;
static int LPKG_ID = -1;
static int NPKGS = 0;

static void FREQ_Finalize() {
    if ( FREQ_SUM ) free( FREQ_SUM );
    FREQ_SUM = NULL;
}

/* Initialize function */
int FREQ_Init( const int npkgs, const int lpkg_id ) {
    int ipkg;
    static int called = false;
    if ( called ) return 0;
    NPKGS    = npkgs;
    LPKG_ID  = lpkg_id;
    FREQ_SUM = (double*)malloc( sizeof(double) * npkgs );
    for ( ipkg=0; ipkg<npkgs; ipkg++ ) FREQ_SUM[ipkg] = 0.e0;
    ET_PREV  = ETIME_Get_etime();
    atexit( FREQ_Finalize );
    called = true;
    return 0;
}

/* Update (accumulate) the "frequency(GHz) x time(sec)" values of
 * all CPU used by this MPI process */
void FREQ_Update() {
    int ipkg, pkg_id, core_id;
    double et, dt;
    float freq;
    uint64_t data;
    et        = ETIME_Get_etime();
    dt        = ( et - ET_PREV );
    for ( ipkg=0, pkg_id=LPKG_ID; ipkg<NPKGS; ipkg++, pkg_id++ ) {
	core_id = MSR_Get_lcore_id( pkg_id );
	rdmsr( core_id, (uint32_t)IA32_PERF_STATUS, &data );
	freq = ((data & 0xff00)>>8) * 0.1e0;
	FREQ_SUM[ipkg] += (double)freq*dt;
    }
    ET_PREV   = et;
}

/* the function to get the accumulated "frequency x time" value
 * of all CPUs used by this MPI process */
void FREQ_Get_freq_time( double freq_time[] ) {
    int ipkg;
    if ( NPKGS == 0 ) {	// if frequency is not counted
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) freq_time[ipkg] = 0.e0;
    } else {
	FREQ_Update();
	for ( ipkg=0; ipkg<NPKGS; ipkg++ ) freq_time[ipkg] = FREQ_SUM[ipkg];
    }
}
