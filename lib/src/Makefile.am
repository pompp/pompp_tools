AUTOMAKE_OPTIONS = foreign

lib_LIBRARIES = libric.a
libric_a_SOURCES = msr.c rapl.c sec.c pmc.c config.c affinity.c cpu-freq.c etime.c freq.c itimer.c misc.c node.c

libric_a_CFLAGS = -Wall -I../include

CC=@CC_TO_USE@

CFLAGS += -DTOPDIR='"@prefix@/etc"'

if COND_MPI
  CFLAGS += -DUSE_MPI
endif

if COND_INTEL
  CFLAGS += -openmp
else
  CFLAGS += -fopenmp
endif

msr.c: msr_base.c
	$(SED) "s|TOPDIR|@prefix@|" $< >$@

CLEANFILES = msr.c

EXTRA_DIST = msr_base.c
