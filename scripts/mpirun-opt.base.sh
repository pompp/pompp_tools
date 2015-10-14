#!/bin/bash

#
# The PomPP software including libraries and tools is developed by the
# PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
# research project supported by the JST, CREST research program.
# The copyrights for code used under license from other parties are
# included in the corresponding files.
# 
# Copyright (c) 2015, The PomPP research team.
#


debug=1

record=1
if [ "x$EXEC_STATUS" = "x" ]; then
    record=0
fi
if [ "x$LAST_UPDATE" = "x" ]; then
    record=0
fi

top=TOPDIR
bin_dir=$top/bin
app_data_dir=$top/app-data
machine=$bin_dir/machine.py

MPIRUN=mpirun
NODEFILE=$PBS_NODEFILE

MIN_POWER=`$machine module_min`
TDP_CPU=`$machine pkg_tdp`
TDP_DRAM=`$machine drm_tdp`
MAX_POWER=`echo "$TDP_CPU + $TDP_DRAM" | bc`

MODULE_POWER=$MAX_POWER
MODE=RAPL
PROF=NO

HEADER=""
PWR_TABLE=$top/etc/pwr-spec-table.txt

APP=""

NPROCS=0

MPI_ARGS=""
all_args=($@)
iarg=0
while [ $iarg -lt $# ]
do
    arg=${all_args[$iarg]}
    case $arg in
	-power)
	    iarg=$(($iarg + 1))
	    arg=${all_args[$iarg]}
	    MODULE_POWER=$arg
	    ;;
	-mode)
	    iarg=$(($iarg + 1))
	    arg=${all_args[$iarg]}
	    MODE=$arg
	    ;;
	-prof)
	    iarg=$(($iarg + 1))
	    arg=${all_args[$iarg]}
	    PROF=$arg
	    ;;
	-prefix)
	    iarg=$(($iarg + 1))
	    arg=${all_args[$iarg]}
	    HEADER=$arg
	    ;;
	-table)
	    iarg=$(($iarg + 1))
	    arg=${all_args[$iarg]}
	    PWR_TABLE=$arg
	    ;;
	-app)
	    iarg=$(($iarg + 1))
	    arg=${all_args[$iarg]}
	    APP=$arg
	    ;;
	-n|-np)
	    MPI_ARGS="$MPI_ARGS $arg"
	    iarg=$(($iarg + 1))
	    arg=${all_args[$iarg]}
	    NPROCS=$arg
	    MPI_ARGS="$MPI_ARGS $arg"
	    ;;
	-machinefile)
	    MPI_ARGS="$MPI_ARGS $arg"
	    iarg=$(($iarg + 1))
	    arg=${all_args[$iarg]}
	    NODEFILE=$arg
	    MPI_ARGS="$MPI_ARGS $arg"
	    ;;
	*)
	    MPI_ARGS="$MPI_ARGS $arg"
	    ;;
    esac
    iarg=$(($iarg + 1))
done

flag=`echo "$MODULE_POWER < $MIN_POWER" | bc`
if [ $flag -eq 1 ]; then
    echo "ERROR: Too low module power is given ($MODULE_POWER)"
    exit 1
fi

PROF=${PROF^^}
if [ $PROF = "YES" ]; then
    PROF=SECTION
fi
if [ $PROF != "NO" -a $PROF != "SECTION" -a $PROF != "ENERGY" ]; then
    echo "ERROR: Illegal profile mode ($PROF)"
    exit 1
fi
if [ $PROF = "NO" ]; then
    HEADER=""
else
    if [ "x$HEADER" = "x" ]; then
	echo "ERROR: No header of filename for profile data is specified"
	exit 1
    fi
fi


if [ ! -r $PWR_TABLE ]; then
    echo "ERROR: Failure to open power specification table $PWR_TABLE"
    exit 1
fi

MODE=${MODE^^}
if [ $MODE != "RAPL" -a $MODE != "FREQ" ]; then
    echo "ERROR: Illegal power control mode $MODE ( \"RAPL\" or \"FREQ\" )"
    exit 1
fi


if [ "x$NODEFILE" = "x" ]; then
    echo "ERROR: No node file is specified"
    exit 1
fi
nodesize=`wc -l $NODEFILE | cut -d" " -f 1`
if [ $NPROCS -le 0 ]; then
    if [ "x$NODEFILE" = "x" ]; then
	echo "ERROR: Both # of procs and node file are not specified"
	exit 1
    fi
    NPROCS=`wc -l $NODEFILE | cut -d" " -f 1`
    if [ $NPROCS -le 0 ]; then
	echo "ERROR: # of nodes in $NODEFILE is less than 1 ($NPROCS)"
	exit 1
    fi
elif [ $NPROCS -gt $nodesize ]; then
    echo "ERROR: Too large # of processes ($NPROCS)"
    echo "    which is greater than # of node in nodefile ($nodesize)"
    exit 1
fi


if [ "x$APP" = "x" ]; then
    echo "ERROR: application species can\'t be determined"
    exit 1
fi
app_data=$app_data_dir/$APP.pdat
if [ ! -r $app_data ]; then
    echo "ERROR: No data for application $APP"
    exit 1
fi


if [ $debug -eq 1 ]; then
    echo "==== options ===="
    echo "mpirun command     = $MPIRUN"
    echo "module power cap   = $MODULE_POWER (W)"
    echo "power control mode = $MODE"
    echo "profile mode       = $PROF"
    if [ ! $PROF = "NO" ]; then
	echo "prefix of profile  = $HEADER"
    fi
    echo "power spec. table  = $PWR_TABLE"
    echo "excecutable        = $BIN"
    echo "application header = $APP"
    echo "# of procs         = $NPROCS"
    if [ $NODEFILE = $PBS_NODEFILE ]; then
	echo "node file          = \$PBS_NODEFILE"
    elif [ $NODEFILE = $QSUB_NODEINF ]; then
	echo "node file          = \$QSUB_NODEINF"
    else
	echo "node file          = $NODEFILE"
    fi
fi

#export POMPP_NPKGS_PER_NODE=`$machine npkgs_per_node`
#export POMPP_NCORES_PER_PKG=`$machine ncores_per_pkg`
export POMPP_INTERVAL_MSEC=50
export POMPP_FREQ_COUNT=1
export POMPP_EVENT_MODE=0
export POMPP_MULTI_SOCKET=1	# ?
if [ $PROF = "NO" ]; then
    export POMPP_PROF_MODE=noprof
elif [ $PROF = "SECTION" ]; then
    export POMPP_PROF_MODE=section
    export POMPP_FILE_HEADER=$HEADER
elif [ $PROF = "ENERGY" ]; then
    export POMPP_PROF_MODE=energy
    export POMPP_FILE_HEADER=$HEADER
fi
MAX_FREQ=`$machine freq_max`
MIN_FREQ=`$machine freq_min`
#
export POMPP_CPUFREQ=`expr $MAX_FREQ \* 100000`
export POMPP_CPU_POWER=`$machine pkg_tdp`
export POMPP_DRAM_POWER=`$machine drm_tdp`

if [ $MODE = "RAPL" ]; then
    #CONF=`mktemp -u`
    CONF=conf-temp
    export POMPP_CONFIG=$CONF
    $bin_dir/var-rapl-est.py $PWR_TABLE $app_data $NODEFILE \
	$NPROCS $MODULE_POWER $CONF > conf.log
elif [ $MODE = "FREQ" ]; then
    ifreq=`$bin_dir/var-freq-est.py $PWR_TABLE $app_data $NODEFILE \
	$NPROCS $MODULE_POWER`
    freq=`expr $ifreq \* 100000`
    export POMPP_CPUFREQ=$freq
    echo "CPU frequency      = $freq (KHz)"
fi


#set -x
if [ $record ]; then
    echo -n "##T itera= $itera from `date -Iseconds | cut -b1-19`" >> \
	$EXEC_STATUS
    date +%s > $LAST_UPDATE
fi
$MPIRUN $MPI_ARGS
#echo "$MPIRUN $MPI_ARGS"

if [ $record ]; then
    echo " to `date -Iseconds | cut -b1-19`" >> $EXEC_STATUS
    date +%s > $LAST_UPDATE
fi
#set +x


if [ $MODE = "RAPL" ]; then
    unset POMPP_CONFIG
    rm -f $CONF-*.conf
elif [ $MODE = "FREQ" ]; then
    unset POMPP_CPUFREQ
fi

