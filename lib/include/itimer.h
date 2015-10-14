#ifndef _ITIMER_H_
#define _ITIMER_H_

#include <stdio.h>

extern int ITIMER_Append( void (*func)(void) );
extern int ITIMER_Start( const int interval_msec );

#endif
