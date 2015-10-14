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

import ric
import machine
import read

MAX_NITERA = 30

import sys

if __name__ == '__main__':
    debug=False
    #
    argc = len(sys.argv)
    if argc < 6:
        print "Usage: %s ubmt_table app_data" % sys.argv[0],
        print "pbs_nodefile nprocs module_pwr_cap"
        sys.exit(1)
    ubmt_table     = sys.argv[1]
    app_data       = sys.argv[2]
    pbs_nodefile   = sys.argv[3]
    nprocs         = int( sys.argv[4] )
    module_pwr_cap = float( sys.argv[5] )
    if debug:
        print "##C ubmt_table       = %s" % ubmt_table
        print "##C app_data         = %s" % app_data
        print "##C pbs_nodefile     = %s" % pbs_nodefile
        print "##C nprocs           = %d" % nprocs
        print "##C module power cap = %7.3f W" % module_pwr_cap

    npkgs          = machine.get_npkgs()
    (rmaxpkg, rmaxdrm, rminpkg, rmindrm) = read.read_ubmt_table(ubmt_table)
    #
    (secnames, app_maxpkg, app_maxdrm, app_minpkg, app_mindrm) = \
            read.read_app_data(app_data)
    nsec = len(secnames)
    #
    app_est_maxpkg=[[0.0 for isec in range(nsec)] for ipkg in range(npkgs)]
    app_est_maxdrm=[[0.0 for isec in range(nsec)] for ipkg in range(npkgs)]
    app_est_minpkg=[[0.0 for isec in range(nsec)] for ipkg in range(npkgs)]
    app_est_mindrm=[[0.0 for isec in range(nsec)] for ipkg in range(npkgs)]
    for ipkg in range(npkgs):
        if rmaxpkg[ipkg] == 0.0: continue
        for isec in range(nsec):
            app_est_maxpkg[ipkg][isec] = rmaxpkg[ipkg]*app_maxpkg[isec]
            app_est_maxdrm[ipkg][isec] = rmaxdrm[ipkg]*app_maxdrm[isec]
            app_est_minpkg[ipkg][isec] = rminpkg[ipkg]*app_minpkg[isec]
            app_est_mindrm[ipkg][isec] = rmindrm[ipkg]*app_mindrm[isec]
    # 
    pkg_list = machine.needed_pkg_list( pbs_nodefile, nprocs )
    Ptot     = module_pwr_cap * nprocs

    alpha = [0.0 for isec in range(nsec)]
    for isec in range(nsec):
        Pmax=0.0
        Pmin=0.0
        for ipkg in range(npkgs):
            if pkg_list[ipkg] == 1:
                Pmax+=app_est_maxpkg[ipkg][isec]+app_est_maxdrm[ipkg][isec]
                Pmin+=app_est_minpkg[ipkg][isec]+app_est_mindrm[ipkg][isec]
        #
        alpha[isec] = (Ptot-Pmin)/(Pmax-Pmin)

    fmax = machine.get_max_freq()
    fmin = machine.get_min_freq()
    for isec in range(nsec):
        freq = int(alpha[isec]*(fmax-fmin) + fmin)
        if freq > fmax: freq=fmax
        if freq < fmin: freq=fmin
        print "%d" % freq
