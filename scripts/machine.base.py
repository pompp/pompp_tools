#!/usr/bin/env python

#
# The PomPP software including libraries and tools is developed by the
# PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
# research project supported by the JST, CREST research program.
# The copyrights for code used under license from other parties are
# included in the corresponding files.
# 
# Copyright (c) 2015, The PomPP research team.
#

import sys
import os

NPKGS_PER_NODE=0    # No. sockets per node
NCORES_PER_PKG=0    # No. cores per socket
NNODES=0
MAX_FREQ=0
MIN_FREQ=0
PKG_TDP=0.0
DRM_TDP=0.0
PKG_MIN=0.0
DRM_MIN=0.0
MODULE_MIN=0.0

INITIALIZED=False

def init_machine():
    global NPKGS_PER_NODE, NNODES, MAX_FREQ, MIN_FREQ, NCORES_PER_PKG
    global PKG_TDP, PKG_MIN, DRM_TDP, DRM_MIN, INITIALIZED
    global MODULE_MIN
    if INITIALIZED == False:
        setting_file="TOPDIR/etc/machine.conf"
        try:
            f = open(setting_file)
        except IOError:
            print "ERROR: Faulure to open setting file %s" % setting_file
            sys.exit(1)
        #
        for line in f:
            line=line.lstrip()
            line=line.replace("=", " ")
            toks = line.split()
            if toks[0].startswith("POMPP_NPKGS_PER_NODE"):
                NPKGS_PER_NODE = int(toks[1])
            elif toks[0].startswith("POMPP_NCORES_PER_PKG"):
                NCORES_PER_PKG = int(toks[1])
            elif toks[0].startswith("POMPP_TOTAL_NODES"):
                NNODES         = int(toks[1])
            elif toks[0].startswith("POMPP_MAX_FREQ"):
                MAX_FREQ = int(toks[1])
            elif toks[0].startswith("POMPP_MIN_FREQ"):
                MIN_FREQ = int(toks[1])
            elif toks[0].startswith("POMPP_PKG_TDP"):
                PKG_TDP = float(toks[1])
            elif toks[0].startswith("POMPP_DRAM_TDP"):
                DRM_TDP = float(toks[1])
            elif toks[0].startswith("POMPP_PKG_MIN"):
                PKG_MIN = float(toks[1])
            elif toks[0].startswith("POMPP_DRAM_MIN"):
                DRM_MIN = float(toks[1])
            elif toks[0].startswith("POMPP_MODULE_MIN"):
                MODULE_MIN = float(toks[1])
        #
        f.close()

        err_mgs=""
        if NPKGS_PER_NODE < 1:
            err_msg += "Illegal NPKGS_PER_NODE=%d\n" % NPKGS_PER_NODE
        if NCORES_PER_PKG < 1:
            err_msg += "Illegal NCORES_PER_PKG=%d\n" % NCORES_PER_PKG
        if NNODES < 1:
            err_msg += "Illegal NNODES=%d\n" % NNODES
        if MAX_FREQ < 1:
            err_msg += "Illegal MAX_FREQ=%d\n" % MAX_FREQ
        if MIN_FREQ < 1:
            err_msg += "Illegal MIN_FREQ=%d\n" % MIN_FREQ
        if MIN_FREQ > MAX_FREQ:
            err_msg += "MIN_FREQ is greater than MAX_FREQ (%d vs %d)\n" % \
                    (MIN_FREQ, MAX_FREQ)
        if PKG_TDP < 0.1:
            err_msg += "Illegal PKG_TDP=%.3f\n" % PKG_TDP
        if DRM_TDP < 0.1:
            err_msg += "Illegal DRM_TDP=%.3f\n" % DRM_TDP
        if PKG_MIN < 0.1:
            err_msg += "Illegal PKG_MIN=%.3f\n" % PKG_MIN
        if DRM_MIN < 0.1:
            err_msg += "Illegal DRM_MIN=%.3f\n" % DRM_MIN
        if PKG_MIN > PKG_TDP:
            err_msg += "PKG_MIN is greater than PKG_TDP"
        if DRM_MIN > DRM_TDP:
            err_msg += "DRM_MIN is greater than DRM_TDP"
        if MODULE_MIN < 0.1:
            err_msg += "Illegal MODULE_MIN=%.3f\n" % MODULE_MIN
        if len(err_mgs) > 0:
            print "ERROR:" + err_mgs
            sys.exit(1)
        INITIALIZED=True

def get_tdp_pkg():
    init_machine()
    return PKG_TDP

def get_tdp_drm():
    init_machine()
    return DRM_TDP

def get_min_drm():
    init_machine()
    return DRM_MIN

def get_min_pkg():
    init_machine()
    return PKG_MIN

def get_min_module():
    init_machine()
    return MODULE_MIN

def get_npkgs_per_node():
    init_machine()
    return NPKGS_PER_NODE

def get_node_number( hostname ):
    snid=hostname[6:9]
    nid = int( snid ) - 1
    return nid

def get_nnodes():
    init_machine()
    return NNODES

def get_ncores_per_pkg():
    init_machine()
    return NCORES_PER_PKG

def get_npkgs():
    init_machine()
    return NPKGS_PER_NODE * NNODES

def get_max_freq():
    init_machine()
    return MAX_FREQ

def get_min_freq():
    init_machine()
    return MIN_FREQ


def get_pkg_id( cline ):
    npkgs_per_node=get_npkgs_per_node()
    toks = cline.split()
    hostname = toks[8]
    local_pkg_id = int( toks[6] )
    node_id = get_node_number( hostname )
    pkg_id = node_id * npkgs_per_node + local_pkg_id
    return pkg_id

#
def get_pkgid( f ):
    pkgid = -1
    f.seek(0)
    for line in f:
        toks = line.split()
        if line.startswith("##C "):
            pkgid = get_pkg_id( line )
    #
    if pkgid < 0:
        print "ERROR: No data to determin module ID"
        sys.exit(1)
    return pkgid
#
def needed_pkg_list( pbs_nodefile, nprocs ):
    try:
        f=open(pbs_nodefile, "r")
    except IOError:
        print "ERROR in needed_pkg_list:",
        print "PBS_NODEFILE cannot be opened"
        sys.exit(1)
    #
    npkgs_per_node = get_npkgs_per_node()
    npkgs          = get_npkgs()
    pkg_list       = [0 for ipkg in range(npkgs)]
    iproc=0
    lpkgid=0
    for line in f:
        lpkgid = iproc % npkgs_per_node
        nodeid=get_node_number(line)
        pkgid = nodeid * npkgs_per_node + lpkgid
        pkg_list[pkgid] = 1
        iproc += 1
        if iproc >= nprocs:
            break
    #
    f.close()
    return pkg_list

if __name__ == '__main__':
    debug = False
    #debug = True
    if debug:
        pkg_tdp = get_tdp_pkg()
        drm_tdp = get_tdp_drm()
        pkg_min = get_min_pkg()
        drm_min = get_min_drm()
        npkgs_per_node=get_npkgs_per_node()
        npkgs  = get_npkgs()
        nnodes = get_nnodes()
        freq_max = get_max_freq()
        freq_min = get_min_freq()
        print "TDP of CPU              = %.2f" % get_tdp_pkg()
        print "TDP of DRAM             = %.2f" % get_tdp_drm()
        print "Min power of CPU        = %.2f" % get_min_pkg()
        print "Min power of DRAM       = %.2f" % get_min_drm()
        print "Min module power        = %.2f" % get_min_module()
        print "# of pkgs per node      = %d" % get_npkgs_per_node()
        print "# of pkgs in system     = %d" % get_npkgs()
        print "# of nodes in system    = %d" % get_nnodes()
        print "# of cores per CPU      = %d" % get_ncores_per_pkg()
        print "Max CPU clock frequency = %d (100MHz)" % get_max_freq()
        print "Min CPU clock frequency = %d (100MHz)" % get_min_freq()
    else:
        if len(sys.argv) < 2:
            print "Usage: %s name_of_element" % sys.argv[0]
            sys.exit(1)
        name=sys.argv[1].lower()
        if   name == "pkg_tdp"        : print "%.3f" % get_tdp_pkg()
        elif name == "pkg_min"        : print "%.3f" % get_min_pkg()
        elif name == "drm_tdp"        : print "%.3f" % get_tdp_drm()
        elif name == "drm_min"        : print "%.3f" % get_min_drm()
        elif name == "module_min"     : print "%.3f" % get_min_module()
        elif name == "npkgs_per_node" : print "%d" % get_npkgs_per_node()
        elif name == "ncores_per_pkg" : print "%d" % get_ncores_per_pkg()
        elif name == "nnodes"         : print "%d" % get_nnodes()
        elif name == "freq_max"       : print "%d" % get_max_freq()
        elif name == "freq_min"       : print "%d" % get_min_freq()
        else                          : print "-1"

