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

import machine

def read_ubmt_table( ubmt_file ):
    try:
        f=open( ubmt_file, "r" )
    except IOError:
        print "ERROR: in read_ubmt_table: u-bench data",
        print "(%s) cannot be opened" % ubmt_file
        sys.exit(1)
    #
    nread=0
    npkgs = machine.get_npkgs()
    pkg_max = [ 0.0 for ipkg in range(npkgs) ]
    drm_max = [ 0.0 for ipkg in range(npkgs) ]
    pkg_min = [ 0.0 for ipkg in range(npkgs) ]
    drm_min = [ 0.0 for ipkg in range(npkgs) ]
    for line in f:
        toks = line.split()
        pkgid = int(toks[0])
        pkg_max[pkgid] = float( toks[1] )
        drm_max[pkgid] = float( toks[2] )
        pkg_min[pkgid] = float( toks[3] )
        drm_min[pkgid] = float( toks[4] )
        nread += 1
    if nread == 0:
        pkg_max = []
        drm_max = []
        pkg_min = []
        drm_min = []
    return (pkg_max, drm_max, pkg_min, drm_min)

def read_app_data( app_data ):
    # initialization
    secnames   = []
    app_maxpkg = []
    app_maxdrm = []
    app_minpkg = []
    app_mindrm = []
    try:
        f = open( app_data )
    except IOError:
        print "ERROR: Failure to open %s" % app_data
        sys.exit(1)
    #
    for line in f:
        if line.startswith("##S "):
            toks = line.split()
            secnames.append( toks[1] )
            app_maxpkg.append( float( toks[2] ) )
            app_maxdrm.append( float( toks[3] ) )
            app_minpkg.append( float( toks[4] ) )
            app_mindrm.append( float( toks[5] ) )
    #
    return (secnames, app_maxpkg, app_maxdrm, app_minpkg, app_mindrm)
