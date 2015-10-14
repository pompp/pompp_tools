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
import glob

_SECNAMES = []
_MAX_NSEC = 64
MAX_NITERA=32

def get_max_nsec():
    return _MAX_NSEC

def get_sec_id( secname ):
    global _SECNAMES
    sec_id = -1
    for isec in range( len(_SECNAMES ) ):
        if secname == _SECNAMES[isec]:
            sec_id = isec
            break
    if sec_id == -1:
        _SECNAMES.append( secname )
        sec_id = len( _SECNAMES ) - 1
    if sec_id >= _MAX_NSEC:
        print "ERROR: Too many number of section (%d)" % sec_id
        sys.exit(1)
    return sec_id

def get_max_sec_len():
    max_len = 0
    for isec in range( len(_SECNAMES ) ):
        tlen = len( _SECNAMES[isec] )
        if tlen > max_len: max_len = tlen
    return max_len

def get_sec_names():
    return _SECNAMES

def get_nsec():
    return len(_SECNAMES)

def reset_sec():
    global _SECNAMES
    _SECNAMES = []



_EVENT_NAME0 = [ "DESC", "NST", "SEC", "STRAT", "END",
        "E-TIME", "PKGCAP", "DRMCAP", "PKGPWR", "PP0PWR",
        "DRMPWR", "FREQ", "LLC_REF", "LLC_MISS", "LD_INST",
        "SR_INST", "SSE_SCD", "SSE_PKD", "256_PKD", "STLIQFUL",
        "INST", "CORE_CYC", "REF_CYC" ]
_EVENT_NAME = ["desc", "nest", "sec", "start", "end",
        "etime", "pkgcap", "drmcap", "pkgpwr", "pp0pwr",
        "drmpwr", "freq", "LLC_ref", "LLC_miss", "load",
        "store", "nosimd", "simd2", "simd4", "stall",
        "inst", "cyc", "ref_cyc" ]

_EVENT_TYPE = [ 0, 0, 0, 0, 0,
        0, 1, 1, 1, 1,
        1, 1, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0 ]

_EVENT_FRAC = [ 0, 0, 0, 3, 3,
        3, 3, 3, 3, 3,
        3, 2, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0 ]

_EVENT_NEED = [ 0, 0, 0, 0, 0,
        1, 0, 0, 1, 1,
        1, 1, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0 ]

NEVENT = 0

TYPE_NOW = -1

EVENT_POS = []

EVENT_NAME = []

EVENT_FRAC = []

EVENT_TYPE = []

def init_event_list( ntype ):
    global _EVENT_NEED, TYPE_NOW
    global EVENT_NAME, EVENT_POS, EVENT_TYPE, EVENT_FRAC, NEVENT
    if TYPE_NOW != ntype:
        if ntype == 1:
            _EVENT_NEED = [ 0, 0, 0, 0, 0,
                    1, 1, 0, 1, 1,
                    1, 1, 0, 0, 0,
                    0, 0, 0, 0, 0,
                    0, 0, 0 ]
        elif ntype == 2:
            _EVENT_NEED = [ 0, 0, 0, 0, 0,
                    1, 0, 0, 1, 1,
                    1, 1, 1, 1, 1,
                    1, 1, 1, 1, 1,
                    1, 1, 0 ]
        elif ntype == 3:
            _EVENT_NEED = [ 0, 0, 0, 0, 0,
                    1, 0, 0, 1, 0,
                    1, 0, 0, 0, 0,
                    0, 0, 0, 0, 0,
                    0, 0, 0 ]
        elif ntype == 4:
            _EVENT_NEED = [ 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0,
                    0, 1, 0, 0, 0,
                    0, 0, 0, 0, 0,
                    0, 0, 0 ]
        elif ntype == 5:
            _EVENT_NEED = [ 0, 0, 0, 0, 0,
                    1, 0, 0, 1, 0,
                    1, 1, 0, 0, 0,
                    0, 0, 0, 0, 0,
                    0, 0, 0 ]
        elif ntype == 6:
            _EVENT_NEED = [ 0, 0, 0, 0, 0,
                    1, 1, 0, 1, 0,
                    1, 1, 0, 0, 0,
                    0, 0, 0, 0, 0,
                    0, 0, 0 ]
        elif ntype == 7:
            _EVENT_NEED = [ 0, 0, 0, 0, 0,
                    1, 0, 0, 1, 0,
                    1, 0, 0, 0, 0,
                    0, 0, 0, 0, 0,
                    1, 1, 0 ]

        EVENT_POS  = []
        EVENT_TYPE = []
        EVENT_FRAC = []
        EVENT_NAME = []
        for ipos in range( len(_EVENT_NEED) ):
            if _EVENT_NEED[ipos] == 1:
                event_name = _EVENT_NAME[ipos]
                if event_name == "sec" : continue
                EVENT_POS.append( ipos )
                EVENT_TYPE.append( _EVENT_TYPE[ipos] )
                EVENT_FRAC.append( _EVENT_FRAC[ipos] )
                EVENT_NAME.append( event_name )
        NEVENT = len( EVENT_POS )
        TYPE_NOW = ntype

def get_event_list():
    if NEVENT == 0: init_event_list( 0 )
    return (EVENT_POS, EVENT_NAME, EVENT_FRAC, EVENT_TYPE)

def get_pos( event ):
    for ipos in range( len(_EVENT_NAME) ):
        if event == _EVENT_NAME[ipos]: return ipos
    return -1

def get_lpos( event ):
    for ipos in range( len( EVENT_NAME ) ):
        if event == EVENT_NAME[ipos]: return ipos
    return -1

def get_nev(): return NEVENT


def get_frac( event ):
    pos = get_pos( event )
    return _EVENT_FRAC[pos]

def get_sec_file_list( header ):
    files = []
    while True:
        tmp = glob.glob(header+'-[0-9]-[0-9].sec')
        if len(tmp) > 0:
            tmp.sort()
            files.extend( tmp )
        else: break
        tmp = glob.glob(header+'-[1-9][0-9]-[0-9].sec')
        if len(tmp) > 0:
            tmp.sort()
            files.extend( tmp )
        else: break
        tmp = glob.glob(header+'-[1-9][0-9][0-9]-[0-9].sec')
        if len(tmp) > 0:
            tmp.sort()
            files.extend( tmp )
        else: break
        tmp = glob.glob(header+'-[1-9][0-9][0-9][0-9]-[0-9].sec')
        if len(tmp) > 0:
            tmp.sort()
            files.extend( tmp )
        else: break
        tmp = glob.glob(header+'-[1-9][0-9][0-9][0-9][0-9]-[0-9].sec')
        if len(tmp) > 0:
            tmp.sort()
            files.extend( tmp )
        else: break
        break
    return files

def get_nsec_ric( secfiles ):
    sec_pos  = get_pos("sec")
    for secfile in secfiles:
        try:
            f = open( secfile, "r" )
        except IOError:
            print "ERROR: Failure in opening file (%s)" % secfile
            sys.exit(1)
        #
        for line in f:
            if line.startswith("##T "):
                toks = line.split()
                secname = toks[sec_pos]
                secid = get_sec_id( secname )
        f.close()
    #
    return get_nsec()

def read_data_by_trial( f ):
    (ev_pos, ev_name, ev_frac, ev_type) = get_event_list()
    nev      = get_nev()
    sec_pos  = get_pos("sec")
    etm_pos  = get_pos("etime")
    nsec = get_nsec()
    values = [[[0.0 for i in range(nev)] for j in range(nsec)] for k in
            range(MAX_NITERA)]

    ntmp = 0
    itera = -1
    for line in f:
        if line.startswith("##T "):
            toks    = line.split()
            secname = toks[sec_pos]
            etime   = float(toks[etm_pos])
            isec    = get_sec_id( secname )
            for iev in range(nev):
                ipos = ev_pos[iev]
                val = float(toks[ipos])
                if ev_type[iev] == 1 : val *= etime
                values[itera][isec][iev] += val
            ntmp += 1
        elif line.startswith("##C "):
            if itera == -1 or ntmp>0:
                ntmp   = 0
                itera += 1
        #
    if ntmp > 0: itera += 1

    nitera   = itera
    secnames = get_sec_names()
    etm_pos  = get_lpos( "etime" )

    for itera in range(nitera):
        for isec in range(nsec):
            etime = values[itera][isec][etm_pos]
            for iev in range(nev):
                if ev_type[iev] == 1: values[itera][isec][iev] /= etime
    #
    return ( nitera, values )

def etime_pwrs_freq_by_trial( f ):
    #
    init_event_list( 5 )
    etm_pos = get_lpos("etime")
    pkg_pos = get_lpos("pkgpwr")
    drm_pos = get_lpos("drmpwr")
    frq_pos = get_lpos("freq")
    nsec    = get_nsec()
    #
    (nitera, values) = read_data_by_trial( f )
    etms = [[0.0 for isec in range(nsec)] for itera in range(nitera)]
    pkgs = [[0.0 for isec in range(nsec)] for itera in range(nitera)]
    drms = [[0.0 for isec in range(nsec)] for itera in range(nitera)]
    tots = [[0.0 for isec in range(nsec)] for itera in range(nitera)]
    frqs = [[0.0 for isec in range(nsec)] for itera in range(nitera)]
    for itera in range(nitera):
        for isec in range(nsec):
            etms[itera][isec] = values[itera][isec][etm_pos]
            pkgs[itera][isec] = values[itera][isec][pkg_pos]
            drms[itera][isec] = values[itera][isec][drm_pos]
            frqs[itera][isec] = values[itera][isec][frq_pos]
            tots[itera][isec] = values[itera][isec][pkg_pos] \
                    + values[itera][isec][drm_pos]
    #
    return (nitera, etms, pkgs, drms, tots, frqs )

if __name__ == '__main__':
    if NEVENT == 0: init_needed_event_list( 2 )
    (event_pos, event_name, event_frac, event_type) = \
            get_needed_event_list()
    sec_pos = get_pos("sec0")
    print "sec= %d" % sec_pos
    for iev in range(len(event_pos)):
        print "pos=%2d  name=%8s  frac=%d  type=%d" % \
                (event_pos[iev], event_name[iev], event_frac[iev],
                        event_type[iev] )


