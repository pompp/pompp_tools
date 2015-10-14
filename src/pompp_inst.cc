/*
 * The PomPP software including libraries and tools is developed by the
 * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
 * research project supported by the JST, CREST research program.
 * The copyrights for code used under license from other parties are
 * included in the corresponding files.
 * 
 * Copyright (c) 2015, The PomPP research team.
 */

/*
  PomPP Instrumentor

  Automatic API instrumentation tool for Power Measurement/Control Libraries
  Developed by The PomPP research team supported by JST/CREST.

  This software was developed by reference to "tau_instrumentor.cpp" provided
  with PDT. (http://www.cs.uoregon.edu/research/pdt/home.php)
  PDT is provided under the following License:

*****************************************************************************
**			Program Database Toolkit (PDT)                     **
**			http://www.cs.uoregon.edu/research/pdt             **
*****************************************************************************
**    Copyright 1997-2009						   **
**    University of Oregon                                                 **
**    Research Center Juelich, FZJ Germany		  		   **
**    Edison Design Group, Inc.                                            **
*****************************************************************************
**	
**  Permission to use, copy, modify, and distribute this software and its
**  documentation for any purpose and without fee is hereby granted,
**  provided that the above copyright notice appear in all copies and that
**  both that copyright notice and this permission notice appear in
**  supporting documentation, and that the name of University of Oregon (UO)
**  Research Center Juelich (FZJ) and Edison Design Group, Inc. (EDG)
**  not be used in advertising or publicity pertaining to distribution of
**  the software without specific, written prior permission.  The
**  University of Oregon, FZJ and EDG make no representations about the
**  suitability of this software for any purpose.  It is provided "as is"
**  without express or implied warranty.
**
**  UO, FZJ AND EDG DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
**  INCLUDING , AND ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, 
**  IN NO EVENT SHALL THE UNIVERSITY OF OREGON, FZJ OR EDG BE LIABLE FOR
**  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
**  RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
**  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
**  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*****************************************************************************
*/

#include "pdbAll.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <list>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <limits.h>
#include <typeinfo>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#undef DEBUG
#undef DEBUG_MORE

#define BUFSIZE 65536

using namespace std;

static char const * pompp_lib_init_c_str      = "POMPP_Init(argc, argv);";
static char const * pompp_lib_init_f_str      = "      call POMPP_Init()";
static char const * pompp_lib_finalize_c_str  = "POMPP_Finalize();";
static char const * pompp_lib_finalize_f_str  = "      call POMPP_Finalize()";
static char const * pompp_lib_start_str       = "POMPP_Start_section";
static char const * pompp_lib_stop_str        = "POMPP_Stop_section";

static bool profFileNameSpecified = false;
static string profFileName("pprof.log");

/* Data structure to handle input from pprof */
typedef struct pprof_data {
    float  data;          /* Proportion of Execution time */
    double exc_msec;      /* Execution time (w/ subfunctions)[msec] */
    double inc_msec;      /* Execution time (w/o subfunctions)[msec] */
    float  calls;         /* # of calls (average) */
    float  subs;          /* # of subfunctions (average) */
    double usec_per_call; /* Execution time per call[usec] */
    string name;          /* Function name */
    bool   inst;          /* Intrumentation target, or not */
} ProfData;

#define PPROF_FUNC 65536
static int pprof_func_num = PPROF_FUNC;
static list<ProfData> pprof_data_list;
static int max_call_threshold = INT_MAX;
static int min_call_threshold = 0;
static double time_threshold = 0.0;

typedef enum {BEGIN, END, RETURN, EXIT, } InstType;
const char* trimchars = " \t\v\r\n";

typedef struct inst_point {
    pdbItem *item;
    InstType type;
    string id;
    int line;
} InstPoint;

// Read Profiling data by TAU
static void Parse_Pprof_File(PDB &pdb)
{
    int i;
    int line_no = 0;
    string inbuf;
    string tmp;
    ProfData profdata;

    if (!profFileNameSpecified) {
        return;
    }

    ifstream istr(profFileName.c_str());
    if (!istr) {
        cerr << "Error: Cannot open '" << profFileName.c_str() << "'" << endl;
        exit(1);
    }

#ifdef DEBUG_MORE
    cerr << "Checking Pprof File..." << endl;
#endif
    getline(istr, inbuf);
#ifdef DEBUG_MORE
    cerr << "SKIP: " << inbuf << endl;
#endif
    while (getline(istr, inbuf) && !istr.eof()) {
        if (inbuf.find("FUNCTION SUMMARY (mean):") != string::npos) {
#ifdef DEBUG_MORE
            cerr << "SKIP: " << inbuf << endl;
#endif
            for (i = 0; i < 4; i++) {
                getline(istr, inbuf);
#ifdef DEBUG_MORE
                cerr << "SKIP: " << inbuf << endl;
#endif
            }
            break;
        }
#ifdef DEBUG_MORE
        else {
            cerr << "SKIP: " << inbuf << endl;
        }
#endif
    }
    while (getline(istr, inbuf) && !istr.eof()) {
        // [Time(%)] [Exclusive (msec)] [Inclusive (Total msec)] [# of Calls] [# of Subroutines] [Inclusive (usec/call)] [Name]
        boost::trim(inbuf);
        if (inbuf.empty()
            || inbuf.find("MPI") != string::npos
            || inbuf.find("TAU") != string::npos) {
            continue;
        }

        vector<string> sv;
        boost::algorithm::split(sv, inbuf, boost::algorithm::is_space(), boost::algorithm::token_compress_on);
        if (pdb.language() == PDB::LA_FORTRAN && sv.size() < 7) {
            cerr << "Illegal pprof file format..." << endl;
            exit(1);
        }
        for (i = 0; i < (int)sv.size(); i++) {
            sv[i] = boost::algorithm::replace_all_copy(sv[i], ",", "");
        }
        i = 0;
        profdata.data          = boost::lexical_cast<float>(sv[i++]);
        profdata.exc_msec      = boost::lexical_cast<double>(sv[i++]);
        profdata.inc_msec      = boost::lexical_cast<double>(sv[i++]);
        profdata.calls         = boost::lexical_cast<float>(sv[i++]);
        profdata.subs          = boost::lexical_cast<float>(sv[i++]);
        profdata.usec_per_call = boost::lexical_cast<double>(sv[i++]);
        if (pdb.language() == PDB::LA_C || pdb.language() == PDB::LA_CXX) {
            i++; // skip type
            vector<string> fsv;
            boost::algorithm::split(fsv, sv[i], boost::is_any_of("("));
            profdata.name = fsv[0];
        } else {
            profdata.name = sv[i];
        }

        if (profdata.data == 100.0
            && profdata.calls == 1.0) {
            // Skip Main function for Fortran
            profdata.inst = false;
        } else if (profdata.calls >= min_call_threshold
            && profdata.calls < max_call_threshold
            && profdata.usec_per_call >= time_threshold) {
            profdata.inst = true;
        } else {
            profdata.inst = false;
        }

        pprof_data_list.push_back(profdata);
        if (++line_no > pprof_func_num - 1) {
            break;
        }
    }
    istr.close();

#ifdef DEBUG
    cerr << "\"Time Ratio\"\t"
         << "\"Time Exc (msec)\"\t"
         << "\"Time Inc (msec)\"\t"
         << "\"# of Calls\"\t"
         << "\"# of Subroutines\"\t"
         << "\"Time per Calls (usec)\"\t"
         << "\"Name\"\t"
         << "\"For Inst or Not\"\t"
         << endl;
    list<ProfData>::iterator it = pprof_data_list.begin();
    while (it != pprof_data_list.end()) {
        cerr << (*it).data << "\t"
             << (*it).exc_msec << "\t"
             << (*it).inc_msec << "\t"
             << (*it).calls << "\t"
             << (*it).subs << "\t"
             << (*it).usec_per_call << "\t"
             << (*it).name << "\t";
        if ((*it).inst) {
            cerr << "for Inst";
        } else {
            cerr << "Not for Inst";
        }
        cerr << endl;
        it++;
    }
#endif
}

static bool check_inst_func(string name)
{
    if (!profFileNameSpecified) {
        return true;
    }

    list<ProfData>::iterator it = pprof_data_list.begin();
    while (it != pprof_data_list.end()) {
        if (name == (*it).name) {
            if ((*it).inst) {
                return true;
            } else {
                return false;
            }
        }
        it++;
    }
    return false;
}


// Read one line from the source file
bool getOneLine(ifstream &file, string &buf, int &lineno)
{
    if(!getline(file, buf)) {
        return false;
    }
    lineno++;
#ifdef DEBUG_MORE
    cerr << lineno << ": " << buf << endl;
#endif
    return true;
}

bool isInComment(int line, int col, pdbFile *pdbf)
{
    pdbFile::cmtvec commentvec;
    pdbLoc startloc;
    pdbLoc endloc;

    commentvec = pdbf->comments();
#ifdef DEBUG
    vector<pdbComment *>::iterator cmtit;
    for (cmtit = commentvec.begin(); cmtit != commentvec.end(); cmtit++) {
        startloc = (*cmtit)->cmtBegin();
        endloc = (*cmtit)->cmtEnd();
        cerr << "Comment from line:" << startloc.line() << " col:" << startloc.col() << endl;
        cerr << "\tto line:" << endloc.line() << " col:" << endloc.col() << endl;
    }
#endif
    return true;
}

static bool lineCmp(const InstPoint *ip1, const InstPoint *ip2)
{
    if (ip1 == ip2) {
        return false;
    }
    if ( ip1->line == ip2->line ) {
        if (ip1->type == ip2->type) {
            return false;
        } else if (ip1->type == BEGIN) {
            return true;
        } else if (ip2->type == BEGIN) {
            return false;
        } else {
            return true; 
        }
    } else {
        return (ip1->line < ip2->line);
    }
    return false; // No meaning...
}

void SortInstPoint(vector<InstPoint *> &pointvec)
{
    stable_sort(pointvec.begin(), pointvec.end(), lineCmp);
#ifdef DEBUG
    cerr << "Instrument Points after sorting with line no : " << endl;
    vector<InstPoint *>::iterator ipit;
    for (ipit = pointvec.begin(); ipit != pointvec.end(); ipit++) {
        switch ((*ipit)->type) {
        case BEGIN:
            cerr << "Line " << (*ipit)->line << ": BEGIN" << endl;
            break;
        case END:
            cerr << "Line " << (*ipit)->line << ": END" << endl;
            break;
        case RETURN:
            cerr << "Line " << (*ipit)->line << ": RETURN" << endl;
            break;
        case EXIT:
            cerr << "Line " << (*ipit)->line << ": EXIT" << endl;
            break;
        default:
            cerr << "Line " << (*ipit)->line << ": UNKNOWN" << endl;
            break;
        }
    }
#endif
}


// For C/C++ Source File
void CorrectInstPoint_CXX(PDB& pdb, pdbFile *pdbf, vector<InstPoint *> &pointvec)
{
    PDB::croutinevec crvec = pdb.getCRoutineVec();
    PDB::croutinevec::const_iterator crit;
    pdbRoutine::locvec return_locations;
    pdbRoutine::locvec::iterator rlit;
    pdbRoutine::callvec callees;
    pdbRoutine::callvec::iterator calleeit;
    InstPoint *ptr;

    // Check each function/routine
    for (crit = crvec.begin(); crit != crvec.end(); crit++) {
        if ((*crit)->location().file() == pdbf
            && (*crit)->bodyBegin().line() != 0
            && check_inst_func((*crit)->name())) {
            // Entry Point
#ifdef DEBUG
            cerr << "Entry Point of " << (*crit)->name() << ": Line " << (*crit)->bodyBegin().line() << endl;
#endif
            ptr = new InstPoint;
            ptr->item = *crit;
            ptr->type = BEGIN;
            ptr->id   = (*crit)->name();
            ptr->line = (*crit)->bodyBegin().line();
            pointvec.push_back(ptr);

            // Exit with abort, etc.
            // Get the list of callees, and check their name
            callees = (*crit)->callees();
            for (calleeit = callees.begin(); calleeit != callees.end(); calleeit++) {
                const pdbRoutine *call = (*calleeit)->call();
                if (call->name() == "abort" || call->name() == "exit") {
                    // Is it OK to consider only about abort and exit ?
#ifdef DEBUG
                    cerr << "Exit Point of " << (*crit)->name() << " (" << call->name() << "): Line " << (*calleeit)->line() << endl;
#endif
                    ptr = new InstPoint;
                    ptr->item = *crit;
                    ptr->type = EXIT;
                    ptr->id   = (*crit)->name();
                    ptr->line = (*calleeit)->line();
                    pointvec.push_back(ptr);
                }
            }

            // Return Statements
            return_locations = (*crit)->returnLocations();
            for (rlit = return_locations.begin(); rlit != return_locations.end(); rlit++) {
#ifdef DEBUG
                cerr << "Return Point of " << (*crit)->name() << ": Line " << (*rlit)->line() << endl;
#endif
                ptr = new InstPoint;
                ptr->item = *crit;
                ptr->type = RETURN;
                ptr->id   = (*crit)->name();
                ptr->line = (*rlit)->line();
                pointvec.push_back(ptr);
            }

            // End Point
#ifdef DEBUG
            cerr << "End Point of " << (*crit)->name() << ": Line " << (*crit)->bodyEnd().line() << endl;
#endif
            ptr = new InstPoint;
            ptr->item = *crit;
            ptr->type = END;
            ptr->id   = (*crit)->name();
            ptr->line = (*crit)->bodyEnd().line();
            pointvec.push_back(ptr);
        }
    }
    SortInstPoint(pointvec);
}

void OutputWithInstFuncs_CXX(PDB& pdb, pdbFile *pdbf, string outfilename, vector<InstPoint *> &pointvec)
{
    vector<InstPoint *>::iterator pit;
    ifstream srcfile;
    ofstream outfile;
    string linebuf;
    string tmpbuf;
    string substr;
    int lineno;

    srcfile.open(pdbf->name().c_str());
    if(!srcfile) {
        cerr << "Coudln't open source file..." << endl;
        exit(1);
    }
    outfile.open(outfilename.c_str());
    if(!outfile) {
        cerr << "Couldn't open output file..." << endl;
    }
    lineno = 0;
    pit = pointvec.begin();
    while (getOneLine(srcfile, linebuf, lineno)) {
        if (pointvec.empty()
            || pit == pointvec.end()) {
            outfile << linebuf << endl;
        } else if (linebuf.find("MPI_Init") != string::npos
                   || linebuf.find("mpi_init") != string::npos
                   || linebuf.find("MPI_init") != string::npos) {
            outfile << linebuf << endl;
            outfile << "\t" << pompp_lib_init_c_str << endl;
            outfile << "\t" << pompp_lib_start_str << "(\"main\");" << endl;
        } else if (linebuf.find("MPI_Finalize") != string::npos
                   || linebuf.find("mpi_finalize") != string::npos
                   || linebuf.find("MPI_finalize") != string::npos) {
            outfile << "\t" << pompp_lib_stop_str << "(\"main\");" << endl;
            outfile << "\t" << pompp_lib_finalize_c_str << endl;
            outfile << linebuf << endl;
        } else if (lineno == (*pit)->line
                   && (*pit)->id != "main" && (*pit)->id != "MAIN") {
            /* Instrumentation for functions other than main */
            switch ((*pit)->type) {
            case BEGIN:
            {
                outfile << "{" << endl;
                outfile << "/* Start Point (Line: " << (*pit)->line << ") */" << endl;
                outfile << pompp_lib_start_str << "(\"" << (*pit)->id << "\");" << endl;
                outfile << linebuf << endl;
                break;
            }
            case END:
            {
                outfile << linebuf << endl;
                outfile << "/* Stop/End Point (Line: " << (*pit)->line << ") */" << endl;
                outfile << pompp_lib_stop_str << "(\"" << (*pit)->id << "\");" << endl;
                outfile << "}" << endl;
                break;
            }
            case RETURN:
            {
                tmpbuf = linebuf;
                string::size_type start = tmpbuf.find("return");
                string::size_type trimend = tmpbuf.find_first_not_of(trimchars);
                if (start == trimend) {
                    /* This line is started with "return"  */
                    outfile << "/* Stop/Return Point (Line: " << (*pit)->line << ") */" << endl;
                    outfile << pompp_lib_stop_str << "(\"" << (*pit)->id << "\");" << endl;
                    outfile << linebuf << endl;
                } else {
                    outfile << tmpbuf.substr(0, start - 1);
                    outfile << " {" << endl;
                    outfile << "/* Stop/Return Point (Line: " << (*pit)->line << ") */" << endl;
                    outfile << pompp_lib_stop_str << "(\"" << (*pit)->id << "\");" << endl;
                    outfile << tmpbuf.substr(start) << endl;
                    outfile << "}" << endl;
                }
                break;
            }
            case EXIT:
            {
                string keyword;
                tmpbuf = linebuf;
                if (tmpbuf.find("abort") == string::npos) {
                    keyword = "exit";
                } else {
                    keyword = "abort";
                }
                string::size_type start = tmpbuf.find(keyword);
                string::size_type trimend = tmpbuf.find_first_not_of(trimchars);
                if (start == string::npos || start == trimend) {
                    /* This line is started with "return"  */
                    outfile << "/* Stop/Exit Point (Line: " << (*pit)->line << ") */" << endl;
                    outfile << pompp_lib_stop_str << "(\"" << (*pit)->id << "\");" << endl;
                    outfile << linebuf << endl;
                } else {
                    outfile << tmpbuf.substr(0, start - 1);
                    outfile << "{" << endl;
                    outfile << "/* Stop/Exit Point (Line: " << (*pit)->line << ") */" << endl;
                    outfile << pompp_lib_stop_str << "(\"" << (*pit)->id << "\");" << endl;
                    outfile << tmpbuf.substr(start) << endl;
                    outfile << "}" << endl;
                }
                break;
            }
            default:
            {
                outfile << "/* Unknown (Line: " << (*pit)->line << ") */" << endl;
                outfile << linebuf << endl;
                break;
            }
            }
            pit++;
        } else {
            outfile << linebuf << endl;
        }
    }
    srcfile.close();
    outfile.close();
    return;
}

int Inst_CXX_File(PDB& pdb, pdbFile *pdbf, string outfilename)
{
    vector<InstPoint *> pointvec;

#ifdef DEBUG
    cerr << "Source file name : " << pdbf->name() << endl;
    cerr << "Output file name : " << outfilename << endl;
#endif

    Parse_Pprof_File(pdb);

    CorrectInstPoint_CXX(pdb, pdbf, pointvec);

    OutputWithInstFuncs_CXX(pdb, pdbf, outfilename, pointvec);

    return 0;
}

int Inst_C_File(PDB& pdb, pdbFile *pdbf, string outfilename)
{
    // Same with C++
    return Inst_CXX_File(pdb, pdbf, outfilename);
}


// For Fortran Source File

pdbComment *FindCommentLine(pdbFile::cmtvec cmtvec, int line)
{
    vector<pdbComment *>::iterator cmit;
    pdbLoc start;
    pdbLoc end;

#ifdef DEBUG
    cerr << "# of Coments: " << cmtvec.size() << endl;
#endif
    for (cmit = cmtvec.begin(); cmit != cmtvec.end(); cmit++) {
        start = (*cmit)->cmtBegin();
        end   = (*cmit)->cmtEnd();
#ifdef DEBUG
        cerr << "Comment: from " << start.line() << " to " << end.line() << endl;
#endif
        if (start.line() <= line && line <= end.line()) {
            return (*cmit);
        }
    }
    return NULL;
}

pdbFile *search_Same_pdbFile(PDB& pdbc, pdbFile *pdbf)
{
    PDB::filevec::const_iterator it;
    for (it = pdbc.getFileVec().begin(); it != pdbc.getFileVec().end(); it++) {
        if ((*it)->name() == pdbf->name()) {
            return (*it);
        }
    }
    return NULL;
}

int SkipCommentsUpward(PDB& pdb, PDB& pdbc, pdbFile *pdbf, int line)
{
    // Applicable only for Fortran...
    pdbFile::cmtvec commentvec;
    pdbComment *cmt;
    pdbFile *pdbf_cmt;

#ifdef DEBUG
    cerr << "Cheking comments to be skipped... (for line" << line << ")" << endl;
#endif
    pdbf_cmt = search_Same_pdbFile(pdbc, pdbf);
    commentvec = pdbf_cmt->comments();
    cmt = FindCommentLine(commentvec, line);
    while (cmt != NULL) {
        line = cmt->cmtBegin().line() - 1;
#ifdef DEBUG
        cerr << "Comment Line: from " << cmt->cmtBegin().line() << " to " << cmt->cmtEnd().line() << endl;
#endif
        cmt = FindCommentLine(commentvec, line);
    }
    return line;
}

pdbPragma *FindPragmaLine(vector<pdbPragma *> pgmvec, int line)
{
    vector<pdbPragma *>::iterator pgmit;
    pdbLoc start;
    pdbLoc end;

#ifdef DEBUG
    cerr << "# of Pragma: " << pgmvec.size() << endl;
#endif
    for (pgmit = pgmvec.begin(); pgmit != pgmvec.end(); pgmit++) {
        start = (*pgmit)->prBegin();
        end   = (*pgmit)->prEnd();
#ifdef DEBUG
        cerr << "Pragma: from " << start.line() << " to " << end.line() << endl;
#endif
        if (start.line() <= line && line <= end.line()) {
            return (*pgmit);
        }
    }
    return NULL;
}

int SkipPragmaUpward(PDB& pdb, PDB& pdbc, pdbFile *pdbf, int line)
{
    vector<pdbPragma *> pragmavec = pdbc.getPragmaVec();
    vector<pdbPragma *>::iterator pgmit;
    pdbPragma *pgm;

    line--;
    pgm = FindPragmaLine(pragmavec, line);
    while (pgm != NULL) {
        line = pgm->prBegin().line() - 1;
#ifdef DEBUG
        cerr << "Pragma Line: from " << pgm->prBegin().line() << " to " << pgm->prEnd().line() << endl;
#endif
        pgm = FindPragmaLine(pragmavec, line);
    }
    return line + 1;
}

int SkipPragmaCommentsUpward(PDB& pdb, PDB& pdbc, pdbFile *pdbf, int line)
{
    // Applicable only for Fortran...
    pdbFile::cmtvec commentvec;
    pdbComment *cmt;
    pdbFile *pdbf_cmt;
    vector<pdbPragma *> pragmavec;
    vector<pdbPragma *>::iterator pgmit;
    pdbPragma *pgm;

    pragmavec = pdbc.getPragmaVec();
    pdbf_cmt = search_Same_pdbFile(pdbc, pdbf);
    commentvec = pdbf_cmt->comments();

#ifdef DEBUG
    cerr << "Cheking pragmas/comments to be skipped... (for line" << line << ")" << endl;
#endif

    line--;
    pgm = FindPragmaLine(pragmavec, line);
    cmt = FindCommentLine(commentvec, line);
    while (pgm != NULL || cmt != NULL) {
        if (pgm != NULL) {
            line = pgm->prBegin().line() - 1;
#ifdef DEBUG
            cerr << "Pragma Line: from " << pgm->prBegin().line() << " to " << pgm->prEnd().line() << endl;
#endif
        } else if (cmt != NULL) {
            line = cmt->cmtBegin().line() - 1;
#ifdef DEBUG
            cerr << "Comment Line: from " << cmt->cmtBegin().line() << " to " << cmt->cmtEnd().line() << endl;
#endif
        } else {
            cerr << "Error on checking pragmas/comments..." << endl;
            exit(1);
        }
        pgm = FindPragmaLine(pragmavec, line);
        cmt = FindCommentLine(commentvec, line);
    }
    return line + 1;
}


void CorrectInstPoint_F(PDB& pdb, PDB& pdbc, pdbFile *pdbf, vector<InstPoint *> &pointvec)
{
    PDB::froutinevec frvec = pdb.getFRoutineVec();
    PDB::froutinevec::const_iterator frit;
    pdbRoutine::locvec return_locations;
    pdbRoutine::locvec::iterator rlit;
    pdbRoutine::locvec stop_locations;
    pdbRoutine::locvec::iterator slit;
    InstPoint *ptr;

    // Check each function/routine
    for (frit = frvec.begin(); frit != frvec.end(); frit++) {
        if ((*frit)->location().file() == pdbf
            && (*frit)->kind() != pdbItem::RO_FSTFN
            && (*frit)->firstExecStmtLocation().file()
            && check_inst_func((*frit)->name())) {
            // Entry Point (Statement executed at first)
            ptr = new InstPoint;
            ptr->item = *frit;
            ptr->type = BEGIN;
            ptr->id   = (*frit)->name();
            ptr->line = SkipPragmaCommentsUpward(pdb, pdbc, pdbf, (*frit)->firstExecStmtLocation().line());
            pointvec.push_back(ptr);
#ifdef DEBUG
            cerr << "Entry Point of " << (*frit)->name() << ": Line " << ptr->line << endl;
#endif

            // Return Statements
            return_locations = (*frit)->returnLocations();
            for (rlit = return_locations.begin(); rlit != return_locations.end(); rlit++) {
                ptr = new InstPoint;
                ptr->item = *frit;
                ptr->type = RETURN;
                ptr->id   = (*frit)->name();
                ptr->line = (*rlit)->line();
                pointvec.push_back(ptr);
#ifdef DEBUG
                cerr << "Return Point of " << (*frit)->name() << ": Line " << (*rlit)->line() << endl;
#endif
            }

            // Exit Point
            stop_locations = (*frit)->stopLocations();
            for (slit = stop_locations.begin(); slit != stop_locations.end(); slit++) {
                ptr = new InstPoint;
                ptr->item = *frit;
                ptr->type = END;
                ptr->id   = (*frit)->name();
                ptr->line = (*slit)->line();
                pointvec.push_back(ptr);
#ifdef DEBUG
                cerr << "Exit Point of " << (*frit)->name() << ": Line " << (*slit)->line() << endl;
#endif
            }
        }
    }
    SortInstPoint(pointvec);
}

void OutputWithInstFuncs_F(PDB& pdb, pdbFile *pdbf, string outfilename, vector<InstPoint *> &pointvec)
{
    vector<InstPoint *>::iterator pit;
    ifstream srcfile;
    ofstream outfile;
    string linebuf;
    string tmpbuf;
    string substr;
    int lineno;

    srcfile.open(pdbf->name().c_str());
    if(!srcfile) {
        cerr << "Coudln't open source file..." << endl;
        exit(1);
    }
    outfile.open(outfilename.c_str());
    if(!outfile) {
        cerr << "Couldn't open output file..." << endl;
    }

    lineno = 0;
    pit = pointvec.begin();
    while (getOneLine(srcfile, linebuf, lineno)) {
        if (pointvec.empty()
            || pit == pointvec.end()) {
            outfile << linebuf << endl;
        } else if (linebuf.find("MPI_Init") != string::npos
                   || linebuf.find("mpi_init") != string::npos
                   || linebuf.find("MPI_init") != string::npos) {
            outfile << linebuf << endl;
            outfile << pompp_lib_init_f_str << endl;
            outfile << endl << "      call " << pompp_lib_start_str << "(\"main\")" << endl;
        } else if (linebuf.find("MPI_Finalize") != string::npos
                   || linebuf.find("mpi_finalize") != string::npos
                   || linebuf.find("MPI_finalize") != string::npos) {
            outfile << endl << "      call " << pompp_lib_stop_str << "(\"main\")" << endl;
            outfile << pompp_lib_finalize_f_str << endl;
            outfile << linebuf << endl;
        } else if (lineno == (*pit)->line
                   && (*pit)->id != "main" && (*pit)->id != "MAIN") {
            /* Instrumentation for functions other than main */
            switch ((*pit)->type) {
            case BEGIN:
            {
                outfile << "CPOMPP Start Point (Line: " << (*pit)->line << ")" << endl;
                outfile << "      call " << pompp_lib_start_str << "(\"" << (*pit)->id << "\");" << endl;
                outfile << linebuf << endl;
                break;
            }
            case END:
            {
                outfile << linebuf << endl;
                outfile << "CPOMPP Stop/End Point (Line: " << (*pit)->line << ")" << endl;
                outfile << "      call " << pompp_lib_stop_str << "(\"" << (*pit)->id << "\");" << endl;
                break;
            }
            case RETURN:
            {
                string keyword;
                tmpbuf = linebuf;
                if (tmpbuf.find("return") == string::npos) {
                    keyword = "stop";
                } else {
                    keyword = "return";
                }
                string::size_type start = tmpbuf.find(keyword);
                string::size_type trimend = tmpbuf.find_first_not_of(trimchars);
                if (start == string::npos || start == trimend) {
                    /* This line is started with "return"  */
                    outfile << "CPOMPP Stop/Return Point (Line: " << (*pit)->line << ")" << endl;
                    outfile << "      call " << pompp_lib_stop_str << "(\"" << (*pit)->id << "\");" << endl;
                    outfile << linebuf << endl;
                } else {
                    outfile << tmpbuf.substr(0, start - 1);
                    if (tmpbuf.find("if") != string::npos
                        || tmpbuf.find("IF") != string::npos) {
                        // if statement
                        outfile << " then" << endl;
                    } else {
                        // labels
                        outfile << " continue" << endl;
                    }
                    outfile << "CPOMPP Stop/Return Point (Line: " << (*pit)->line << ")" << endl;
                    outfile << "      call " << pompp_lib_stop_str << "(\"" << (*pit)->id << "\");" << endl;
                    outfile << "      " << tmpbuf.substr(start) << endl;
                    if (tmpbuf.find("if") != string::npos
                        || tmpbuf.find("IF") != string::npos) {
                        outfile << "      endif" << endl;
                    }
                }
                break;


                outfile << linebuf << endl;
                break;
            }
            case EXIT:
            {
                outfile << "CPOMPP Stop/Exit Point (Line: " << (*pit)->line << ")" << endl;
                outfile << "      call " << pompp_lib_stop_str << "(\"" << (*pit)->id << "\");" << endl;
                outfile << linebuf << endl;
                break;
            }
            default:
            {
                outfile << "CPOMPP Unknown (Line: " << (*pit)->line << ")" << endl;
                outfile << linebuf << endl;
                break;
            }
            }
            pit++;
        } else {
            outfile << linebuf << endl;
        }
    }
    srcfile.close();
    outfile.close();
    return;
}

int Inst_Fortran_File(PDB& pdb, PDB& pdbc, pdbFile *pdbf, string outfilename)
{
    vector<InstPoint *> pointvec;

#ifdef DEBUG
    cerr << "Source file name : " << pdbf->name() << endl;
    cerr << "Output file name : " << outfilename << endl;

    pdbFile::cmtvec cmtvec;
    vector<pdbComment *>::iterator cmit;
    cmtvec = pdbf->comments();
    for (cmit = cmtvec.begin(); cmit != cmtvec.end(); cmit++ )
    {
        cerr << "Comment : Line " << (*cmit)->cmtBegin().line() << " - " << (*cmit)->cmtEnd().line() << endl;
    }
#endif

    Parse_Pprof_File(pdb);

    CorrectInstPoint_F(pdb, pdbc, pdbf, pointvec);

    OutputWithInstFuncs_F(pdb, pdbf, outfilename, pointvec);

    return 0;
}

int main(int argc, char *argv[])
{
    int i;
    int retval;
    string srcfilename;
    string outputfilename;
    bool outputFileNameSpecified = false;
    char* pdbcfilename = NULL;
    bool pdbcFileNameSpecified = false;

    if (argc < 3) {
        cout << "Usage: pompp_inst <PDB file> <SRC file> [-pdb-cmt <PDB file (w/ comment)] [-prof <pprof file] [-minct #] [-maxct #] [-tt #] [-instnum #]" << endl;
        cout << "-pdb-cmt <filename> : Specify PDB file with comment (required to handle OpenMP directives in Fortran" << endl;
        cout << "-prof <filename>    : Spefify Profile data generated by TAU pprof to eneble selective instrumentation" << endl;
        cout << "-instnum #          : Specify the number of funcs to be instrumented" << endl;
        cout << "-minct #            : Specify the Min. threshold for the instrumentation by the # of calls of funcs" << endl;
        cout << "-maxct #            : Specify the Max. threshold for the instrumentation by the # of calls of funcs" << endl;
        cout << "-tt #               : Specify the threshold for the instrumentation by exec time (usec) per call" << endl;
        return 1;
    }

    for (i = 0; i < argc; i++) {
        switch (i) {
        case 0:
#ifdef DEBUG
            printf("Name of PDB file = %s\n", argv[1]);
#endif
            break;
        case 1:
            srcfilename = argv[2];
#ifdef DEBUG
            printf("Name of SRC file = %s\n", argv[2]);
#endif
            break;
        default:
            if (strcmp(argv[i], "-o") == 0) {
                i++;
                outputfilename = string(argv[i]);
                outputFileNameSpecified = true;
#ifdef DEBUG
                printf("output file = %s\n", argv[i]);
#endif
            }
            if (strcmp(argv[i], "-prof") == 0) {
                i++;
                profFileName = argv[i];
                profFileNameSpecified = true;
#ifdef DEBUG
                printf("Name of PPROF file = %s\n", argv[3]);
#endif
            }
            if (strcmp(argv[i], "-pdb-cmt") == 0) {
                i++;
                pdbcfilename = (char *)calloc(strlen(argv[i]), sizeof(char));
                strcpy(pdbcfilename, argv[i]);
                pdbcFileNameSpecified = true;
#ifdef DEBUG
                printf("PDB file for comments = %s\n", argv[i]);
#endif
            }
            if (strcmp(argv[i], "-instnum") == 0) {
                i++;
                pprof_func_num = atoi(argv[i]);
#ifdef DEBUG
                printf("# of Funcs for Instrumentation = %d\n", atoi(argv[i]));
#endif
            }
            if (strcmp(argv[i], "-minct") == 0) {
                i++;
                min_call_threshold = atoi(argv[i]);
#ifdef DEBUG
                printf("Intrumentation Min Threshold by # of calls = %d\n", min_call_threshold);
#endif
            }
            if (strcmp(argv[i], "-maxct") == 0) {
                i++;
                max_call_threshold = atoi(argv[i]);
#ifdef DEBUG
                printf("Intrumentation Max Threshold by # of calls = %d\n", max_call_threshold);
#endif
            }
            if (strcmp(argv[i], "-tt") == 0) {
                i++;
                time_threshold = atof(argv[i]);
#ifdef DEBUG
                printf("Intrumentation Threshold by exec time (usec) per call = %f\n", time_threshold);
#endif
            }
            break;
        }
    }

    PDB pdb(argv[1]);
    if (!pdb) {
        exit(1);
    }
    if (!outputFileNameSpecified) {
        outputfilename = string(srcfilename + string(".ins"));
    }

    // Iterate each file
    for (PDB::filevec::const_iterator it = pdb.getFileVec().begin();
         it != pdb.getFileVec().end();
         it++) {
        // Check if it is given file
        if (strcmp((*it)->name().c_str(), srcfilename.c_str()) != 0) {
            continue;
        }
        // Detect language
        PDB::lang_t lang = pdb.language();
        switch (lang) {
        case PDB::LA_C:
#ifdef DEBUG
            cerr << "Input : C file" << endl;
#endif
            retval = Inst_C_File(pdb, *it, outputfilename);
            break;
        case PDB::LA_CXX:
#ifdef DEBUG
            cerr << "Input : C++ file" << endl;
#endif
            retval = Inst_CXX_File(pdb, *it, outputfilename);
            break;
        case PDB::LA_FORTRAN:
#ifdef DEBUG
            cerr << "Input : FORTRAN file" << endl;
#endif
            if (pdbcFileNameSpecified) {
                PDB pdbc(pdbcfilename);
                if (!pdbc) {
                    exit(1);
                }
                retval = Inst_Fortran_File(pdb, pdbc, *it, outputfilename);
            } else {
                cerr << "Need to specify PDB file for comment with \"-pdb-cmt\" option" << endl;
                exit(1);
            }
            break;
        default:
            cerr << "Failed to detect language..." << endl;
            exit(1);
            break;
        }
        if (retval != 0) {
            cout << "Failed to instrument to \"" << srcfilename << "\"..." << endl;
        }
    }
    
    return 0;
}
