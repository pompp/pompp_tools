#ifndef _SECTION_H_
#define _SECTION_H_

#include <stdio.h>
#include <stdlib.h>

extern char** SEC_Getadd_section_names();
extern void SEC_Put_cpu_caps( const int nsec, const int nsock,
	double **src );
extern void SEC_Put_dram_caps( const int nsec, const int nsock,
	double **src );
/*extern void SEC_Put_cpu_caps( const int nsec, const double src[] );
extern void SEC_Put_dram_caps( const int nsec, const double src[] );*/

extern int SEC_Init( const int prof_mode, const char *file_header,
	const double cpu_cap0, const double dram_cap0 );

/*extern int CONF_Get_section_id( const char *secname );
extern int SEC_Set_power_cap( const int id );
extern int SEC_Unset_power_cap( const int id );*/


#endif
