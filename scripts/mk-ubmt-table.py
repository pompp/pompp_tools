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

import machine
import ric

import sys
from math import fabs

def read_ave_pwrs_with_freq( header, ifreq ):
    coef = 1.3
    pkg_max  = coef * machine.get_tdp_pkg()
    drm_max  = coef * machine.get_tdp_drm()
    secfiles = ric.get_sec_file_list( header )
    ric.reset_sec()
    nsec     = ric.get_nsec_ric( secfiles )
    npkgs    = machine.get_npkgs()
    tfreq    = 0.1 * float( ifreq )

    aves_pkg = [[0.0 for isec in range(nsec)] for ipkg in range(npkgs)]
    aves_drm = [[0.0 for isec in range(nsec)] for ipkg in range(npkgs)]
    for secfile in secfiles:
        try:
            f = open( secfile, "r" )
        except IOError:
            print "ERROR: Failure opening file %s" % secfile
            sys.exit(1)
        #
        pkgid = machine.get_pkgid( f )
        #
        f.seek(0)
        (nitera, etms, pkgs, drms, tots, frqs) = \
                ric.etime_pwrs_freq_by_trial( f )

        for isec in range(nsec):
            ave_pkg = 0.0
            ave_drm = 0.0
            neff = 0
            for itera in range(nitera):
                dif = fabs( tfreq - frqs[itera][isec] )
                if dif < 0.01:
                    pkg = pkgs[itera][isec]
                    drm = drms[itera][isec]
                    if pkg < pkg_max and drm < drm_max:
                        ave_pkg += pkg
                        ave_drm += drm
                        neff += 1
                else:   # debug
                    print "warning: %s iter=%d" % (secfile, itera)
            #
            if neff < 1:
                print "ERROR: No effective data in %s" % secfile
                f.close()
            #
            aves_pkg[pkgid][isec] = ave_pkg/float(neff)
            aves_drm[pkgid][isec] = ave_drm/float(neff)
        #
        f.close()
    #
    return ( aves_pkg, aves_drm )

def calc_effective_ave_pwrs( npkgs, nsec, pkgs, drms ):
    coef    = 1.3
    pkg_max = coef * machine.get_tdp_pkg()
    drm_max = coef * machine.get_tdp_drm()
    pwr_min = 0.5
    aves_pkg = [0.0 for isec in range(nsec)]
    aves_drm = [0.0 for isec in range(nsec)]
    for isec in range(nsec):
        ave_pkg = 0.0
        ave_drm = 0.0
        neff = 0
        for ipkg in range(npkgs):
            pkg = pkgs[ipkg][isec]
            drm = drms[ipkg][isec]
            if pkg > pkg_max:
                print "ERROR: Illegal CPU power (%10.3f)" % pkg
                sys.exit(1)
            if drm > drm_max:
                print "ERROR: Illegal DRAM power (%10.3f)" % drm
                sys.exit(1)
            if pkg > pwr_min and drm > pwr_min:
                ave_pkg += pkg
                ave_drm += drm
                neff += 1
        #
        if neff == 0:
            print "ERROR: No effective data"
            sys.exit(1)
        aves_pkg[isec] = ave_pkg / float(neff)
        aves_drm[isec] = ave_drm / float(neff)
    #
    return (aves_pkg, aves_drm)

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print "Usage: %s nocap-header minfreq-header table-name" % \
                sys.argv[0]
        sys.exit(1)
    #
    header_max = sys.argv[1]
    header_min = sys.argv[2]
    table_name = sys.argv[3]
    # debug
    print "nocap header    = %s" % header_max
    print "min-freq header = %s" % header_min
    print "ubmt table name = %s" % table_name
    #
    ifreq_max = machine.get_max_freq()
    ifreq_min = machine.get_min_freq()
    (pkgs_max, drms_max) = read_ave_pwrs_with_freq( header_max, ifreq_max )
    (pkgs_min, drms_min) = read_ave_pwrs_with_freq( header_min, ifreq_min )

    npkgs = machine.get_npkgs()
    nsec  = ric.get_nsec()
    (aves_pkg_max, aves_drm_max) = \
            calc_effective_ave_pwrs(npkgs, nsec, pkgs_max, drms_max)
    (aves_pkg_min, aves_drm_min) = \
            calc_effective_ave_pwrs(npkgs, nsec, pkgs_min, drms_min)

    for isec in range(nsec):
        rave_pkg = 1.0/aves_pkg_max[isec]
        rave_drm = 1.0/aves_drm_max[isec]
        for ipkg in range(npkgs):
            pkgs_max[ipkg][isec] *= rave_pkg
            drms_max[ipkg][isec] *= rave_drm
    for isec in range(nsec):
        rave_pkg = 1.0/aves_pkg_min[isec]
        rave_drm = 1.0/aves_drm_min[isec]
        for ipkg in range(npkgs):
            pkgs_min[ipkg][isec] *= rave_pkg
            drms_min[ipkg][isec] *= rave_drm

    try:
        f = open(table_name, "w")
    except IOError:
        print "ERROR: Failure opening file %s" % table_name
    for ipkg in range(npkgs):
        str=" %6d %8.6f %8.6f %8.6f %8.6f\n" % \
                (ipkg, pkgs_max[ipkg][0], drms_max[ipkg][0],
                        pkgs_min[ipkg][0], drms_min[ipkg][0])
        f.write( str )
    f.close()
