#ifndef _RAPL_H_
#define _RAPL_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum _dev_type_ { RAPL_PKG, RAPL_PP0, RAPL_DRAM } devtype;

extern void RAPL_Finalize();
extern int RAPL_Init( const char* file_header, const int prof_mode );

// functions to get various setting parameters
extern double RAPL_Get_pkg_max_power();
extern double RAPL_Get_pkg_min_power();
extern double RAPL_Get_pkg_tdp();
extern double RAPL_Get_dram_max_power();
extern double RAPL_Get_dram_min_power();
extern double RAPL_Get_dram_tdp();
extern double RAPL_Get_energy_unit();

// functions concerning the power capping
extern int RAPL_Release_power_cap( const int core_id, devtype dev );

extern void RAPL_Release_all_power_cap( const int pkg_id );

extern void RAPL_Release_all_package_power_cap();

extern int RAPL_Set_power_caps( devtype dev, double *limit_val );

extern double RAPL_Get_power_cap( devtype dev, const int pkg_id );

// update and read energy data
extern void RAPL_Update();
extern void RAPL_Read( uint64_t energy_pkg[],
	uint64_t energy_pp0[], uint64_t energy_dram[] );

#endif
