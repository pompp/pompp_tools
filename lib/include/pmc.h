#ifndef _PMC_H_
#define _PMC_H_

#include <stdio.h>
#include <stdint.h>

extern int PMC_Read( uint64_t **GPMC_PKG, uint64_t **FPMC_PKG );

extern int PMC_Get_data( int *ngpmc, int *nfixed,
	char ***GPMC_EVENT_NAME, char ***FIXED_EVENT_NAME );

#endif
