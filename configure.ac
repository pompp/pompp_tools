#
# configure.ac for PomPP Instrumentation Tool and Power Control Lib.
# 
AC_INIT([pompp_tools], [0.1], [yasutaka.wada@meisei-u.ac.jp])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([src/pompp_inst.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_AWK
AC_PROG_SED
AC_PROG_CXX
AC_PROG_CC

# Checks for header files.
AC_CHECK_HEADERS([limits.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
#AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([pow])

# Check for other Func/Header/Type requirements
AC_CHECK_FUNCS([atexit])
AC_CHECK_FUNCS([clock_gettime])
AC_CHECK_FUNCS([gethostname])
AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([sqrt])
AC_CHECK_FUNCS([strcasecmp])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strdup])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([sys/file.h])
AC_CHECK_HEADERS([sys/time.h])
AC_TYPE_MODE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_FUNC_MALLOC

# Options to specify PDT location
AC_ARG_WITH(pdt-dir,
	[AS_HELP_STRING([--with-pdt-dir=DIR], [Specify PDT library/header files location.])],
	[
LDFLAGS="$LDFLAGS -L$withval/lib"
CPPFLAGS="$CPPFLAGS -I$withval/include"
	])
AC_ARG_WITH(pdt-libdir,
	[AS_HELP_STRING([--with-pdt-libdir=DIR], [Specify PDT library files location.])],
	[LDFLAGS="$LDFLAGS -L$withval"])
AC_ARG_WITH(pdt-incdir,
	[AS_HELP_STRING([--with-pdt-incdir=DIR], [Specify PDT header files location.])],
	[CPPFLAGS="$CPPFLAGS -I$withval"])

# Check PDT lib/header
AC_CHECK_LIB(pdb, pdb)
AC_CHECK_HEADERS(pdbAll.h)

# Options to swithc MPI usage
AC_ARG_WITH(mpi,
	[AS_HELP_STRING([--with-mpi], [Enable MPI.(Default: disabled)])],
	[USE_MPI=yes])

AC_ARG_WITH(mpicc,
	[AS_HELP_STRING([--with-mpicc=ARG], [Enable MPI with the MPI compiler.])],
	[
USE_MPI=yes
SET_MPICC=yes
MY_MPICC=$withval
])
AM_CONDITIONAL([COND_MPI], [test "$USE_MPI" = yes])

# Options to use Intel Compiler
AC_ARG_WITH(intel-compiler,
	[AS_HELP_STRING([--with-intel-compiler], [use Intel compiler. (Default: disabled)])],
	[USE_INTEL=yes])
AM_CONDITIONAL([COND_INTEL], [test "$USE_INTEL" = yes])

# Set MPI Compiler
CC_TO_USE=$CC
CXX_TO_USE=$CXX
if test "$USE_MPI" = "yes";
   then
   if test "$SET_MPICC" = "yes";
      then
      CC_TO_USE=$MY_MPICC
      CXX_TO_USE=$CXX
   elif test "$USE_INTEL" = "yes";
      then
      CC_TO_USE=mpiicc
      CXX_TO_USE=icpc
   else
      CC_TO_USE=mpicc
      CXX_TO_USE=$CXX
   fi
else
   if test "$USE_INTEL" = "yes";
      then
      CC_TO_USE=icc
      CXX_TO_USE=icpc
   else
      CC_TO_USE=$CC
      CXX_TO_USE=$CXX
   fi
fi
AC_CHECK_PROG(CC_TO_USE, $CC_TO_USE, [$CC_TO_USE not found.])
AC_SUBST(CC_TO_USE)
AC_CHECK_PROG(CXX_TO_USE, $CXX_TO_USE, [$CXX_TO_USE not found.])
AC_SUBST(CXX_TO_USE)

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 lib/src/Makefile
		 lib/include/Makefile
		 scripts/Makefile
		 etc/Makefile])
AC_PROG_RANLIB
AC_OUTPUT
