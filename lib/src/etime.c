/*
 * The PomPP software including libraries and tools is developed by the
 * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
 * research project supported by the JST, CREST research program.
 * The copyrights for code used under license from other parties are
 * included in the corresponding files.
 * 
 * Copyright (c) 2015, The PomPP research team.
 */

/* ======================================================
 * Codes to measure the elapsed time
 * ====================================================== */
#define _GNU_SOURCE 1
#include <stdio.h>

#include <time.h>
#include <sys/time.h>

#include "ric-def.h"

static double ET0;
/* Getting high accurary time provided by OS */
static double get_etime() {
    struct timespec tp;
    clock_gettime( CLOCK_REALTIME, &tp );
    return (double)(tp.tv_sec + 1.e-9*tp.tv_nsec);
}

/* Initialize timer */
int ETIME_Init() {
    static int called = false;
    if ( called ) return 0;
    MPI_Barrier( MPI_COMM_WORLD );
    ET0 = get_etime();
    called = true;
    return 0;
}

/* Getting elapsed time from the timer initialization */
double ETIME_Get_etime() {
    double et;
    et = get_etime();
    return (et - ET0);
}
