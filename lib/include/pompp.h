#ifndef _RIC_POMPP_H_
#define _RIC_POMPP_H_

#include <stdio.h>

extern int  POMPP_Init( int argc, char *argv[] );
extern int  POMPP_Start_section( const char *secname );
extern int  POMPP_Stop_section( const char *secname );
extern void POMPP_Finalize();

#endif
