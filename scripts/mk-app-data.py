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

def read_average( fin, nsec ):
    (ev_pos, ev_name, ev_frac, ev_type) = ric.get_event_list()
    nev      = len( ev_pos )
    sec_pos  = ric.get_pos("sec")
    etm_pos  = ric.get_pos("etime")
    VALUES = [[[0.0 for i in range(nev)] for j in range(nsec)] for k in
            range(MAX_NITERA)]

    ntmp = 0
    itera = -1
    for line in fin:
        if line.startswith("##T "):
            toks    = line.split()
            secname = toks[sec_pos]
            etime   = float(toks[etm_pos])
            isec    = ric.get_sec_id( secname )
            for iev in range(nev):
                ipos = ev_pos[iev]
                val = float(toks[ipos])
                if ev_type[iev] == 1 : val *= etime
                VALUES[itera][isec][iev] += val
            ntmp += 1
        elif line.startswith("##C "):
            if itera == -1 or ntmp>0:
                ntmp   = 0
                itera += 1
        #
    if ntmp > 0: itera += 1
    nitera   = itera
    secnames = ric.get_sec_names()
    etm_pos  = ric.get_lpos( "etime" )
    for itera in range(nitera):
        for isec in range(nsec):
            etime = VALUES[itera][isec][etm_pos]
            for iev in range(nev):
                if ev_type[iev] == 1: VALUES[itera][isec][iev] /= etime
    aves = [[0.0 for i in range(nev)] for j in range(nsec)]
    for itera in range(nitera):
        for isec in range(nsec):
            for iev in range(nev):
                aves[isec][iev] += VALUES[itera][isec][iev]
    for isec in range(nsec):
        for iev in range(nev):
            aves[isec][iev] /= nitera
    return (nsec, nev, aves)


def read_powers_all( header, nsec, npkgs ):
    ric.init_event_list( 3 )
    pkgpwr_pos = ric.get_lpos("pkgpwr")
    drmpwr_pos = ric.get_lpos("drmpwr")
    secfiles = ric.get_sec_file_list( header )
    pkgpwr = [ [0.0 for i in range(nsec)] for j in range(npkgs) ]
    drmpwr = [ [0.0 for i in range(nsec)] for j in range(npkgs) ]

    for secfile in secfiles:
        with open(secfile) as f:
            for line in f:
                if line.startswith("##C "):
                    pkg_id = machine.get_pkg_id( line )
                    break
            f.seek(0)
            (lnsec, lnev, lpwr) = read_average( f, nsec )
            for isec in range(nsec):
                pkgpwr[pkg_id][isec] = lpwr[isec][pkgpwr_pos]
                drmpwr[pkg_id][isec] = lpwr[isec][drmpwr_pos]
            f.close()
    #
    return (pkgpwr, drmpwr)

if __name__ == '__main__':
    debug=False
    #
    argc = len(sys.argv)
    if argc < 4:
        print "Usage: %s ubmt_table app_max app_min" % sys.argv[0]
        sys.exit(1)
    ubmt_table     = sys.argv[1]
    app_max_header = sys.argv[2]
    app_min_header = sys.argv[3]
    if debug:
        print "##C ubmt_table       = %s" % ubmt_table
        print "##C app_max_header   = %s" % app_max_header
        print "##C app_min_header   = %s" % app_min_header

    npkgs          = machine.get_npkgs()
    (rmaxpkg, rmaxdrm, rminpkg, rmindrm) = read.read_ubmt_table(ubmt_table)
    #
    secfiles = ric.get_sec_file_list( app_max_header )
    ric.reset_sec()
    nsec     = ric.get_nsec_ric( secfiles )
    secnames = ric.get_sec_names()
    (app_maxpkg, app_maxdrm) = read_powers_all(app_max_header, nsec, npkgs)
    (app_minpkg, app_mindrm) = read_powers_all(app_min_header, nsec, npkgs)

    ave_ref_maxpkg = [0.0 for i in range(nsec)]
    ave_ref_maxdrm = [0.0 for i in range(nsec)]
    ave_ref_minpkg = [0.0 for i in range(nsec)]
    ave_ref_mindrm = [0.0 for i in range(nsec)]
    ave_app_maxpkg = [0.0 for i in range(nsec)]
    ave_app_maxdrm = [0.0 for i in range(nsec)]
    ave_app_minpkg = [0.0 for i in range(nsec)]
    ave_app_mindrm = [0.0 for i in range(nsec)]
    for ipkg in range(npkgs):
        for isec in range(nsec):
            if app_maxpkg[ipkg][isec]>0.0 and app_minpkg[ipkg][isec]>0.0:
                if rmaxpkg[ipkg]==0.0 or rminpkg[ipkg]==0:
                    sys.stderr.write( \
                            "ERROR: No reference data for pkg=%d\n" % ipkg)
                    print "0"
                    sys.exit(3)
                ave_ref_maxpkg[isec] += rmaxpkg[ipkg]
                ave_ref_maxdrm[isec] += rmaxdrm[ipkg]
                ave_ref_minpkg[isec] += rminpkg[ipkg]
                ave_ref_mindrm[isec] += rmindrm[ipkg]
                ave_app_maxpkg[isec] += app_maxpkg[ipkg][isec]
                ave_app_maxdrm[isec] += app_maxdrm[ipkg][isec]
                ave_app_minpkg[isec] += app_minpkg[ipkg][isec]
                ave_app_mindrm[isec] += app_mindrm[ipkg][isec]

    for isec in range(nsec):
        ave_app_maxpkg[isec] /= ave_ref_maxpkg[isec]
        ave_app_maxdrm[isec] /= ave_ref_maxdrm[isec]
        ave_app_minpkg[isec] /= ave_ref_minpkg[isec]
        ave_app_mindrm[isec] /= ave_ref_mindrm[isec]

    maxstrlen=len(secnames[0])
    for isec in range(1, nsec):
        strlen=len(secnames[isec])
        if strlen > maxstrlen: maxstrlen=strlen
    #
    for isec in range(nsec):
        print "##S %*s %10.6f %10.6f  %10.6f %10.6f" % \
                (maxstrlen, secnames[isec],
                        ave_app_maxpkg[isec], ave_app_maxdrm[isec],
                        ave_app_minpkg[isec], ave_app_mindrm[isec])
    #
