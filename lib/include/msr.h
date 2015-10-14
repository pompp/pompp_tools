#ifndef _MSR_H_
#define _MSR_H_

#include <stdio.h>
#include <stdint.h>

extern int MSR_Init( const int npackages, const int ncores_per_package );
extern int MSR_Get_lcore_id( const int pkg );
extern int MSR_Get_ncores_per_pkg();
extern int MSR_Get_npkgs_per_node();
extern int MSR_Get_ncores_per_node();

extern int wrmsr( const int core_id, uint32_t reg, uint64_t data );
extern int rdmsr( const int core_id, uint32_t reg, uint64_t *data );
extern void get_cpuid( int param,
	unsigned int *eax, unsigned int *ebx,
	unsigned int *ecx, unsigned int *edx );
extern void get_cpuid2( int param0, int param1,
	unsigned int *eax, unsigned int *ebx,
	unsigned int *ecx, unsigned int *edx );
extern uint64_t rdtsc();

#endif
