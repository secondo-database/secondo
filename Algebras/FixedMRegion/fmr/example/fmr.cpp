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
#include "satof.h"

#include <iostream>

using namespace std;
using namespace fmr;

static void usage ();
static double instant (std::string inst);
static RList readObjectFile (string filename, string objecttype);

static void atinstant (string fmregion, string inst) {
    RList fmrrl = readObjectFile(fmregion, "fmregion"); FMRegion fmr(fmrrl);
    cout << fmr.atinstant(instant(inst))
               .toRList().SecondoObject("region", "region")
               .ToString() << "\n";
}

static void inside (string fmregion, string mpoint) {
    RList fmrrl = readObjectFile(fmregion, "fmregion"); FMRegion fmr(fmrrl);
    RList mprl = readObjectFile(mpoint, "mpoint"); MPoint mp(mprl);
    
    cout << fmr.inside(mp)
               .toRList().SecondoObject("mbool", "mbool")
               .ToString() << "\n";
}

static void intersection (string fmregion, string mpoint) {
    RList fmrrl = readObjectFile(fmregion, "fmregion"); FMRegion fmr(fmrrl);
    RList mprl = readObjectFile(mpoint, "mpoint"); MPoint mp(mprl);
    
    cout << mp.intersection(fmr)
               .toRList().SecondoObject("mpoint", "mpoint")
               .ToString() << "\n";
}

static void setcenter (string fmregion, string _x, string _y) {
    double x = satof(_x.c_str());
    double y = satof(_y.c_str());
    RList fmrrl = readObjectFile(fmregion, "fmregion"); FMRegion fmr(fmrrl);
    Point c(x, y);
    fmr.setCenter(c);
    cout << fmr.toRList().SecondoObject("fmregion", "fmregion")
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

static void mkcad (string region, string rotations) {
    double rots = ((double)atoi(rotations.c_str()))/360.0;
    RList srcregrl = readObjectFile(region, "region"); Region srcreg(srcregrl);
    Point c = srcreg.centroid();
    BoundingBox srcbb = srcreg.boundingBox();
    Point p = srcbb.upperRight - srcbb.lowerLeft;
    double dist = (p.x > p.y) ? p.x : p.y;
    FMRegion fmr(srcreg, 
            TransformationUnit(c, Point(dist*M_PI*rots, 0), -2*M_PI*rots));
    Region reg = fmr.traversedArea().toRegion(200);
    RList ta = reg.toRList();
    RList face = ta[0][0];
    std::stringstream ss;
    BoundingBox bb = reg.boundingBox();
    Point d = bb.upperRight - bb.lowerLeft;
    double sc = 180/d.x;
    double sc2 = 180/d.y;
    if (sc2 < sc)
        sc = sc2;
    Point o = bb.lowerLeft - c;
    ss << "linear_extrude(5) {\n"
       << "  difference() {\n"
       << "    translate([" << (o.x*sc - 5.0) << "," << (o.y*sc - 5.0) << "])\n"
       << "      square([" << d.x*sc+10.0 << "," << d.y*sc+10.0 << "]);\n"
       << "    polygon([";
    for (unsigned int i = 0; i < face.size(); i++) {
        RList pt = face[i];
        ss << "["<<(pt[0].getNr()-c.x)*sc<<"," << (pt[1].getNr()-c.y)*sc << "]";
        if (i < face.size()-1)
            ss << ",";
    }
    double doff = -d.y*sc-5;
    ss << "]);\n"
       << "  }\n"
       << "  translate([0, " << doff << ", 0])\n"
       << "    polygon([";
    sc = sc/1.02;
    face = srcregrl[0][0];
    for (unsigned int i = 0; i < face.size(); i++) {
        RList pt = face[i];
        ss << "["<<(pt[0].getNr()-c.x)*sc<<"," << (pt[1].getNr()-c.y)*sc << "]";
        if (i < face.size()-1)
            ss << ",";
    }
    ss << "]);\n"
       << "}\n"
       << "$fn=20;\n"
       << "  translate([0, " << doff << ",0])\n"
       << "cylinder(d=2, h=20);\n";
    
    cout << ss.str();
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
    } else if (cmd == string("intersection")) {
        if (argc != 4) usage();
        intersection(argv[2], argv[3]);
    } else if (cmd == string("setcenter")) {
        if (argc != 5) usage();
        setcenter(argv[2], argv[3], argv[4]);
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
    } else if (cmd == string("mkcad")) {
        if (argc != 4) usage();
        mkcad(argv[2], argv[3]);
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
           "     intersection  <fmregion> <mpoint>\n"
           "     setcenter     <fmregion> <x> <y>\n"
           "     traversedarea <fmregion>\n"
           "     inside2       <cregion> #<point>\n"
           "     intersects    <cregion> <region>\n"
           "     interpolate   <region> <region>\n"
           "     toregion      <cregion> <nrsamples>\n"
           "     mkcad         <region> <rotations>\n"
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
