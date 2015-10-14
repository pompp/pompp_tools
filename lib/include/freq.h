#ifndef _FREQ_H_
#define _FREQ_H_

#include <stdio.h>

extern int FREQ_Init( const int npkgs, const int lpkg_id );
extern void FREQ_Update();
extern void FREQ_Get_freq_time( double freq_time[] );

#endif
