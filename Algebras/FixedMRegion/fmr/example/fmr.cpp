/* 
 * File:   fmr.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on October 12, 2016, 3:08 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Example binary to demonstrate the usage of libfmr

[TOC]

1 Overview

*/

#include "fmr_FMRegion.h"
#include "fmr_Region.h"
#include "fmr_RList.h"
#include "fmr_Interval.h"

#include <iostream>

using namespace std;
using namespace fmr;

static void usage ();
static double instant (std::string inst);
static RList readObjectFile (string filename, string objecttype);

static void atinstant (string fmregion, string inst) {
    RList fmrrl = readObjectFile(fmregion, "fmregion"); FMRegion fmr(fmrrl);
    cout << fmr.atinstant(instant(inst))
               .toRList().SecondoObject("fmregion", "fmregion")
               .ToString() << "\n";
}

static void inside (string fmregion, string mpoint) {
    RList fmrrl = readObjectFile(fmregion, "fmregion"); FMRegion fmr(fmrrl);
    RList mprl = readObjectFile(mpoint, "mpoint"); MPoint mp(mprl);
    
    cout << fmr.inside(mp)
               .toRList().SecondoObject("mbool", "mbool")
               .ToString() << "\n";
}

static void traversedarea (string fmregion) {
    RList fmrrl = readObjectFile(fmregion, "fmregion"); FMRegion fmr(fmrrl);
    cout << fmr.traversedArea()
               .toRList().SecondoObject("cregion", "cregion")
               .ToString() << "\n";
}

static void inside2 (string cregion, string point) {
    RList cregrl = readObjectFile(cregion, "cregion"); CRegion creg(cregrl);
    
    double x, y;
    if (sscanf(point.c_str(), "%lf/%lf", &x, &y) != 2) {
        cerr << "Error parsing point! Format: <x>/<y>\n";
        exit(EXIT_FAILURE);
    }
    
    Point p(x, y);
    cout << (creg.inside(p) ? "TRUE" : "FALSE") << "\n";
}

static void intersects (string cregion, string region) {
    RList cregrl = readObjectFile(cregion, "cregion"); CRegion creg(cregrl);
    RList  regrl = readObjectFile( region,  "region"); Region   reg( regrl);
    
    cout << (creg.intersects(reg) ? "TRUE" : "FALSE") << "\n";
}

static void interpolate (string region1, string region2) {
    RList reg1rl = readObjectFile(region1,  "region"); Region reg1(reg1rl);
    RList reg2rl = readObjectFile(region2,  "region"); Region reg2(reg2rl);
    
    cout <<
            reg1.interpolate(reg2, Interval())
                .toRList().SecondoObject("fmregion", "fmregion")
                .ToString() << "\n";
}

static void toregion (string cregion, string nrsamples) {
    int nr = atoi(nrsamples.c_str());
    
    RList cregrl = readObjectFile(cregion, "cregion"); CRegion creg(cregrl);
    
    cout <<
            creg.toRegion(nr)
                .toRList().SecondoObject("region", "region")
                .ToString() << "\n";
}


int main (int argc, char **argv) {
    if (argc < 2)
        usage();
    
    string cmd = argv[1];
    
    if (cmd == string("atinstant")) {
        if (argc != 4) usage();
        atinstant(argv[2], argv[3]);
    } else if (cmd == string("inside")) {
        if (argc != 4) usage();
        inside(argv[2], argv[3]);
    } else if (cmd == string("traversedarea")) {
        if (argc != 3) usage();
        traversedarea(argv[2]);
    } else if (cmd == string("inside2")) {
        if (argc != 4) usage();
        inside2(argv[2], argv[3]);
    } else if (cmd == string("intersects")) {
        if (argc != 4) usage();
        intersects(argv[2], argv[3]);
    } else if (cmd == string("interpolate")) {
        if (argc != 4) usage();
        interpolate(argv[2], argv[3]);
    } else if (cmd == string("toregion")) {
        if (argc != 4) usage();
        toregion(argv[2], argv[3]);
    } else {
        usage();
    }
    
    exit(EXIT_SUCCESS);
}

static void usage () {
    cerr <<
           "Usage: fmr <command> [<param1> ... <paramn>]\n" <<
           "  where command is:\n"
           "     atinstant     <fmregion> #<instant>\n"
           "     inside        <fmregion> <mpoint>\n"
           "     traversedarea <fmregion>\n"
           "     inside2       <cregion> #<point>\n"
           "     intersects    <cregion> <region>\n"
           "     interpolate   <region> <region>\n"
           "     toregion      <cregion> <nrsamples>\n"
           "# denotes a literal argument, others are files with nested lists.\n"
           "Format instant: YYYY-MM-DD[-HH[:MM[:SS[.sss]]]], e.g. 2016-10-01\n"
           "Format point: <x>/<y>, e.g. 10.45/20.77\n";
    exit(EXIT_FAILURE);
}

static RList readObjectFile (string filename, string objecttype) {
    RList rl = RList::parseFile(filename);
    if (rl[3].getSym() != objecttype) {
        cerr << "File '" << filename << "' parse error: No "
             << objecttype << " object\n";
        exit(EXIT_FAILURE);
    }
    
    return rl[4];
}

static double instant (string inststr) {
    double inst = Interval::parsetime(inststr);
    if (isnan(inst)) {
        cerr << "Instant '" << inststr << "' could not be parsed!\n";
        exit(EXIT_FAILURE);
    }
    
    return inst;
}
