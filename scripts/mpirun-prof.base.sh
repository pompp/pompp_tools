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
app_data_tmp=$top/app-data/tmp
machine=$bin_dir/machine.py

mkdir -p $app_data_dir
mkdir -p $app_data_tmp
rm -f $app_data_tmp/*


MPIRUN=mpirun
NODEFILE=$PBS_NODEFILE

NPROCS=0
NODEFILE=""
APP=""

NITERA=1
PWR_TABLE=$top/etc/pwr-spec-table.txt


MPI_ARGS=""
all_args=($@)
iarg=0
while [ $iarg -lt $# ]
do
    arg=${all_args[$iarg]}
    case $arg in
	-itera)
	    iarg=$(($iarg + 1))
	    arg=${all_args[$iarg]}
	    NITERA=$arg
	    ;;
	-app)
	    iarg=$(($iarg + 1))
	    arg=${all_args[$iarg]}
	    APP=$arg
	    ;;
	-table)
	    iarg=$(($iarg + 1))
	    arg=${all_args[$iarg]}
	    PWR_TABLE=$arg
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
	    MPI_ARGS="$MPI_ARGS $NODEFILE"
	    ;;
	*)
	    MPI_ARGS="$MPI_ARGS $arg"
	    ;;
    esac
    iarg=$(($iarg + 1))
done

if [ $NITERA -lt 1 ]; then
    NITERA=1
fi
if [ "x$APP" = "x" ]; then
    echo "ERROR: No name of application is specified"
    exit 1
fi

echo "NITERA   = $NITERA"
echo "APP      = $APP"
echo "power spec. table  = $PWR_TABLE"
echo "MPIRUN   = $MPIRUN"
echo "NODEFILE = $NODEFILE"
echo "MPI_ARGS = $MPI_ARGS"

#export POMPP_NPKGS_PER_NODE=`$machine npkgs_per_node`
#export POMPP_NCORES_PER_PKG=`$machine ncores_per_pkg`
export POMPP_INTERVAL_MSEC=50
export POMPP_FREQ_COUNT=1
export POMPP_EVENT_MODE=0
export POMPP_MULTI_SOCKET=1	# ?
export POMPP_PROF_MODE=section
export POMPP_CPU_POWER=`$machine pkg_tdp`
export POMPP_DRAM_POWER=`$machine drm_tdp`
MAX_FREQ=`$machine freq_max`
MIN_FREQ=`$machine freq_min`
#
nocap_header=$app_data_tmp/$APP-nocap
min_freq_header=$app_data_tmp/$APP-min-freq

export POMPP_CPUFREQ=`expr $MAX_FREQ \* 100000`
export POMPP_FILE_HEADER=$nocap_header
if [ $record -eq 1 ]; then
    echo "####S exceution of profiling mode without power cap" >> $EXEC_STATUS
fi
for ((itera=0;itera<$NITERA;++itera))
do
    if [ $record -eq 1 ]; then
	echo -n "##T itera= $itera from `date -Iseconds | cut -b1-19`" >> \
	    $EXEC_STATUS
	date +%s > $LAST_UPDATE
    fi
    $MPIRUN $MPI_ARGS
    #echo "$MPIRUN $MPI_ARGS"
    if [ $record -eq 1 ]; then
	echo " to `date -Iseconds | cut -b1-19`" >> $EXEC_STATUS
	date +%s > $LAST_UPDATE
    fi
done
if [ $record -eq 1 ]; then
    echo "####E exceution of profiling mode without power cap" >> $EXEC_STATUS
fi
unset POMPP_CPUFREQ
unset POMPP_FILE_HEADER

export POMPP_CPUFREQ=`expr $MIN_FREQ \* 100000`
export POMPP_FILE_HEADER=$min_freq_header
if [ $record -eq 1 ]; then
    echo "####S exceution of profiling mode with min. freq." >> $EXEC_STATUS
fi
for ((itera=0;itera<$NITERA;++itera))
do
    if [ $record -eq 1 ]; then
	echo -n "##T itera= $itera from `date -Iseconds | cut -b1-19`" >> \
	    $EXEC_STATUS
	date +%s > $LAST_UPDATE
    fi
    $MPIRUN $MPI_ARGS
    #echo "$MPIRUN $MPI_ARGS"
    if [ $record -eq 1 ]; then
	echo " to `date -Iseconds | cut -b1-19`" >> $EXEC_STATUS
	date +%s > $LAST_UPDATE
    fi
done

if [ $record -eq 1 ]; then
    echo "####E exceution of profiling mode with min. freq." >> $EXEC_STATUS
fi
unset POMPP_CPUFREQ
unset POMPP_FILE_HEADER


$bin_dir/mk-app-data.py $PWR_TABLE $nocap_header $min_freq_header > \
    $app_data_dir/$APP.pdat
#
rm -f $nocap_header*.sec $min_freq_header*.sec
