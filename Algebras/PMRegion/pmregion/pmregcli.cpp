/* 
 * This file is part of libpmregion
 * 
 * File:   pmregcli.cpp
 * Author: Florian Heinz <fh@sysv.de>

 1 pmregcli
   CLI program for processing polyhedral moving regions
  
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <map>
#include "PMRegion_internal.h"


using namespace std;
using namespace pmr;

static int iters = 0;

static void usage (void) {
   cerr << "Usage: pmregcli <command> [param_1] [param_2] ... [param_n]"
        << endl
        << "      === PMRegion operations === " << endl
        << "             atinstant     <pmregfile> <instant>" << endl
        << "             perimeter     <pmregfile>" << endl
        << "             area          <pmregfile>" << endl
        << "             traversedarea <pmregfile>" << endl
        << "             mpointinside  <pmregfile> <mpointfile>" << endl
        << "             intersection  <pmregfile> <pmregfile>" << endl
        << "             union         <pmregfile> <pmregfile>" << endl
        << "             difference    <pmregfile> <pmregfile>" << endl
        << "             intersects    <pmregfile> <pmregfile>" << endl
        << "      === Conversion operations === " << endl
        << "             pmreg2mreg    <pmregfile>" << endl
//        << "             pmreg2mreg2   <pmregfile>" << endl
        << "             pmreg2off     <pmregfile>" << endl
        << "             off2pmreg     <offfile>" << endl
        << "             mreg2pmreg    <mregfile>" << endl
        << "             reg2pmreg     <regfile> <instant1> <instant2> <xoff>"
        << endl
        << "     === Spatiotemporal coverage analysis ===" << endl
#ifndef PMREGION_DISABLE_COVERDURATION
        << "             createcdpoly    <pmreg> [basereg]" << endl
        << "             createccdpoly   <pmreg> [basereg]" << endl
        << "             createicdpoly   <pmreg> <duration/msecs> [basereg]"
        << endl
        << "             restrictcdpoly  <pmreg> <basereg>" << endl
        << "             coverduration   <cdpoly> <x> <y>" << endl
        << "             coveredlonger   <cdpoly> <duration/msecs>" << endl
        << "             coveredshorter  <ccdpoly> <msecs>" << endl
        << "             intervalcovered <icdpoly> <msecs>" << endl
        << "             avgcover        <cdpoly> [basereg]" << endl
#else
        << "             (disabled due to too old CGAL library) " << endl
#endif
        << "     === Miscellaneous commands ===" << endl
        << "             decomposemreg  <mregfile>  [0:isolate 1:debranch "
                            "2:keeplargest] [r:relation s:separate]" << endl
        << "             decomposepmreg <pmregfile> [0:isolate 1:debranch "
                            "2:keeplargest]" << endl
        << "             analyze        <pmreg>" << endl
#ifndef PMREGION_DISABLE_COVERDURATION
        << "             openscad       <pmregfile>" << endl
#endif
        << "             deter1         <csv>" << endl
        ;
    exit(1);
}

static RList file2rlist(char *fname) {
    ifstream i1(fname, std::fstream::in);
    if (i1.fail()) {
        cerr << "Could not open file '" << fname << "'" << endl;
        exit(EXIT_FAILURE);
    }
    RList ret = RList::parse(i1);

    return ret;
}

static RList* file2rlistp(char *fname) {
    ifstream i1(fname, std::fstream::in);
    if (i1.fail()) {
        cerr << "Could not open file '" << fname << "'" << endl;
        exit(EXIT_FAILURE);
    }
    RList* ret = RList::parsep(i1);

    return ret;
}


static void atinstant(char **param) {
    RList rl = file2rlist(param[0]);
    double instant = parsetime(param[1]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    while (iters--)
        pmreg.atinstant(instant);
    cout << pmreg.atinstant(instant).ToString() << endl;
}

static void atinstant2(char **param) {
    RList rl = file2rlist(param[0]);
    double instant = parsetime(param[1]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    while (iters--)
        pmreg.atinstant2(instant);
    cerr << "Calculating atinstant..." << endl;
    cout << pmreg.atinstant2(instant).ToString() << endl;
}

#ifndef PMREGION_DISABLE_COVERDURATION

static void createcdpoly(char **param, int argc) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
//    Nef_polyhedron np(pmreg.polyhedron);
//    pmreg.polyhedron = nef2polyhedron(np);
    PMRegion cdpoly;
    do {
        if (argc == 2) {
            RList basereg = file2rlist(param[1]);
            cdpoly = pmreg.createcdpoly(basereg);
        } else {
            cdpoly = pmreg.createcdpoly();
        }
    } while (iters--);

    cout << cdpoly.toRList().ToString() << endl;    
}

static void restrictcdpoly(char **param, int argc) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    PMRegion cdpoly;
    RList basereg = file2rlist(param[1]);
    cdpoly = pmreg.restrictcdpoly(basereg);
    
    cout << cdpoly.toRList().ToString() << endl;    
}

static void createccdpoly(char **param, int argc) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    PMRegion ccdpoly;
    do {
        if (argc == 2) {
            RList basereg = file2rlist(param[1]);
            ccdpoly = pmreg.createccdpoly(basereg);
        } else {
            ccdpoly = pmreg.createccdpoly();
        }
    } while (iters--);
    
    cout << ccdpoly.toRList().ToString() << endl;    
}

static void createicdpoly(char **param, int argc) {
    RList rl = file2rlist(param[0]);
    double duration = atof(param[1]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    PMRegion icdpoly;
    do {
        if (argc == 3) {
            RList basereg = file2rlist(param[2]);
            icdpoly = pmreg.createicdpoly(duration, basereg);
        } else {
            icdpoly = pmreg.createicdpoly(duration);
        }
    } while (iters--);
    
    cout << icdpoly.toRList().ToString() << endl;    
}

static void coverduration (char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion cdpoly = PMRegion::fromRList(rl);
    double x = atof(param[1]);
    double y = atof(param[2]);
    while (iters--)
        cdpoly.coverduration(x, y);
    cout << cdpoly.coverduration(x, y) << endl;
}

static void coveredlonger (char **param) {
    RList rl = file2rlist(param[0]);
    double duration = atof(param[1]);
    PMRegion cdpoly = PMRegion::fromRList(rl);
    while (iters--)
        cdpoly.coveredlonger(duration);
    cout << cdpoly.coveredlonger(duration).ToString() << endl;
}

static void coveredshorter (char **param) {
    RList rl = file2rlist(param[0]);
    double duration = atof(param[1]);
    PMRegion cdpoly = PMRegion::fromRList(rl);
    while (iters--)
        cdpoly.coveredshorter(duration);
    cout << cdpoly.coveredshorter(duration).ToString() << endl;
}

static void intervalcovered (char **param) {
    RList rl = file2rlist(param[0]);
    double duration = atof(param[1]);
    PMRegion cdpoly = PMRegion::fromRList(rl);
    while (iters--)
        cdpoly.intervalcovered(duration);
    cout << cdpoly.intervalcovered(duration).ToString() << endl;
}

static void avgcover (char **param, int argc) {
    RList rl = file2rlist(param[0]);
    PMRegion cdpoly = PMRegion::fromRList(rl);
    if (argc == 1) {
        char buf[100];
        sprintf(buf, "%.20f", CGAL::to_double(cdpoly.avgcover()));

        cout << buf << endl;
    } else {
        RList baseregion = file2rlist(param[1]);
        char buf[100];
        sprintf(buf, "%.20f", CGAL::to_double(cdpoly.avgcover(baseregion)));

        cout << buf << endl;
    }
}

static void openscad (char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    pmreg.openscad(param[0]);
    PMRegion cd = pmreg.createcdpoly();
    cd.openscad(((string)param[0])+"cd");
}

#endif /* PMREGION_DISABLE_COVERDURATION */

static void minmaxz(char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    pair<Kernel::FT, Kernel::FT> minmax = pmreg.minmaxz();
    cout << ::CGAL::to_double(minmax.first) << " " <<
        ::CGAL::to_double(minmax.second) << endl;
}

static void perimeter(char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    while (iters--)
        pmreg.perimeter();
    cout << pmreg.perimeter().rl.ToString() << endl;
}

static void area(char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    while (iters--)
        pmreg.area();
    cout << pmreg.area().rl.ToString() << endl;
}

static void area2(char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    while (iters--)
        pmreg.area2();
    cout << pmreg.area2().rl.ToString() << endl;
}

static void pmreg2mreg(char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    while (iters--)
        pmreg.toMRegion();
    cout << pmreg.toMRegion().ToString() << endl;
}

static void pmreg2mreg2(char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    while (iters--)
        pmreg.toMRegion();
    cout << pmreg.toMRegion2().ToString() << endl;
}

static void pmreg2off(char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    while (iters--)
        pmreg.toOFF();
    cout << pmreg.toOFF() << endl;
}

static void off2pmreg(char **param) {
    Polyhedron p;
    ifstream i(param[0], ifstream::in);
    i >> p;
    PMRegion pmreg(p);
    cout << pmreg.toRList().ToString() << endl;
}

static void off2mreg(char **param) {
    Polyhedron p;
    ifstream i(param[0], ifstream::in);
    i >> p;
    PMRegion pmreg(p);
    cout << pmreg.toMRegion().ToString() << endl;
}

static void mreg2pmreg(char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromMRegion(rl);
    while (iters--)
        pmreg.toRList();
    cout << pmreg.toRList().ToString() << endl;
}

static void analyze (char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    pmreg.analyze();
}

static void reg2pmreg(char **param) {
    RList rl = file2rlist(param[0]);
    double inst1 = parsetime(param[1]);
    double inst2 = parsetime(param[2]);
    double xoff = atof(param[3]);
    PMRegion pmreg = PMRegion::fromRegion(rl, inst1, inst2, xoff);
    cout << pmreg.toRList().ToString() << endl;
}

static void intersects (char **param) {
    RList reg1 = file2rlist(param[0]);
    RList reg2 = file2rlist(param[1]);
    PMRegion pmreg1 = PMRegion::fromRList(reg1);
    PMRegion pmreg2 = PMRegion::fromRList(reg2);
    while (iters--)
        pmreg1.intersects(pmreg2);
    cout << pmreg1.intersects(pmreg2).rl.ToString() << endl;
}

static void intersection (char **param) {
    RList reg1 = file2rlist(param[0]);
    RList reg2 = file2rlist(param[1]);

    PMRegion pmreg1 = PMRegion::fromRList(reg1);
    PMRegion pmreg2 = PMRegion::fromRList(reg2);

    while (iters--)
        pmreg1 * pmreg2;
    PMRegion is = pmreg1 * pmreg2;
    cout << is.toRList().ToString() << endl;
}

static void do_union (char **param) {
    RList reg1 = file2rlist(param[0]);
    RList reg2 = file2rlist(param[1]);

    PMRegion pmreg1 = PMRegion::fromRList(reg1);
    PMRegion pmreg2 = PMRegion::fromRList(reg2);

    while (iters--)
        pmreg1 + pmreg2;
    PMRegion is = pmreg1 + pmreg2;
    cout << is.toRList().ToString() << endl;
}

static void mpointinside (char **param) {
    RList reg = file2rlist(param[0]);
    RList mpoint = file2rlist(param[1]);

    PMRegion pmreg = PMRegion::fromRList(reg);
    while (iters--)
        pmreg.mpointinside(mpoint);
    MBool mbool = pmreg.mpointinside(mpoint);
    cout << mbool.rl.ToString() << endl;
}

static void difference (char **param) {
    RList reg1 = file2rlist(param[0]);
    RList reg2 = file2rlist(param[1]);

    PMRegion pmreg1 = PMRegion::fromRList(reg1);
    PMRegion pmreg2 = PMRegion::fromRList(reg2);

    while (iters--)
        pmreg1 - pmreg2;
    PMRegion is = pmreg1 - pmreg2;
    cout << is.toRList().ToString() << endl;
}

static void traversedarea (char **param) {
    RList reg = file2rlist(param[0]);

    PMRegion pmreg = PMRegion::fromRList(reg);

    while (iters--)
        pmreg.traversedarea();
    cout << pmreg.traversedarea().ToString() << endl;
}

static void decomposemreg(char **param) {
    int steps = atoi(param[0]);

    cerr << "Reading file " << param[0] << endl;
    RList* rl = file2rlistp(param[0]);
    cerr << "Done, starting decompose..." << endl;

    vector<RList> mregs;
    RList ret = decompose_mreg(&rl->items[0], steps, mregs);
    if (param[1][0] == 'r') {
        cout << ret.ToString() << endl;
    } else if (param[1][0] == 'i') {
        for (unsigned int i = 0; i < mregs.size(); i++) {
            char fname[100];
            sprintf(fname, "mregs.%d", i);
        cerr << "Writing output file " << fname << endl;
            mregs[i].toFile(fname);
        }
    }

    delete rl;
    cout << ret.ToString() << endl;
}

static void decomposepmreg(char **param) {
    int steps = atoi(param[1]);

    cerr << "Reading file " << param[0] << endl;
    RList* rl = file2rlistp(param[0]);
    cerr << "Done, starting decompose..." << endl;

    vector<RList> mregs;
    PMRegion pmreg = PMRegion::fromRList(rl->items[0]);
    delete rl;
    vector<PMRegion> res = decompose_pmreg(&pmreg, steps, mregs);
    for (unsigned int i = 0; i < res.size(); i++) {
        char fname[100];
        sprintf(fname, "decompose.%d", i);
    cerr << "Writing output file " << fname << endl;
        res[i].toFile(fname);
    }
}

// Program entry point
int main (int argc, char **argv) {
    if (argc < 2)
        usage();

    // For benchmarking
    if (getenv("ITERS")) {
        iters = atoi(getenv("ITERS"));
        cerr << "Performing " << iters << " iterations" << endl;
    }

    char *cmd = argv[1];
    int nrparam = argc - 2;
    char **param = argv+2;

    if (!strcmp(cmd, "pmreg2mreg") && nrparam == 1) {
        pmreg2mreg2(param);
    } else if (!strcmp(cmd, "pmreg2mregold") && nrparam == 1) {
        pmreg2mreg(param);
    } else if (!strcmp(cmd, "pmreg2off") && nrparam == 1) {
        pmreg2off(param);
    } else if (!strcmp(cmd, "off2pmreg") && nrparam == 1) {
        off2pmreg(param);
    } else if (!strcmp(cmd, "off2mreg") && nrparam == 1) {
        off2mreg(param);
    } else if (!strcmp(cmd, "atinstant") && nrparam == 2) {
        atinstant2(param);
    } else if (!strcmp(cmd, "atinstantold") && nrparam == 2) {
        atinstant(param);
    } else if (!strcmp(cmd, "minmaxz") && nrparam == 1) {
        minmaxz(param);
    } else if (!strcmp(cmd, "perimeter") && nrparam == 1) {
        perimeter(param);
    } else if (!strcmp(cmd, "area") && nrparam == 1) {
        area(param);
    } else if (!strcmp(cmd, "area2") && nrparam == 1) {
        area2(param);
    } else if (!strcmp(cmd, "traversedarea") && nrparam == 1) {
        traversedarea(param);
    } else if (!strcmp(cmd, "intersects") && nrparam == 2) {
        intersects(param);
    } else if (!strcmp(cmd, "mreg2pmreg") && nrparam == 1) {
        mreg2pmreg(param);
    } else if (!strcmp(cmd, "reg2pmreg") && nrparam == 4) {
        reg2pmreg(param);
    } else if (!strcmp(cmd, "intersection") && nrparam == 2) {
        intersection(param);
    } else if (!strcmp(cmd, "union") && nrparam == 2) {
        do_union(param);
    } else if (!strcmp(cmd, "mpointinside") && nrparam == 2) {
        mpointinside(param);
    } else if (!strcmp(cmd, "difference") && nrparam == 2) {
        difference(param);
    } else if (!strcmp(cmd, "analyze") && nrparam == 1) {
        analyze(param);
#ifndef PMREGION_DISABLE_COVERDURATION
    } else if (!strcmp(cmd, "createcdpoly") && (nrparam == 1 || nrparam == 2)) {
        createcdpoly(param, nrparam);
    } else if (!strcmp(cmd, "restrictcdpoly") && nrparam == 2) {
        restrictcdpoly(param, nrparam);
    } else if (!strcmp(cmd, "createccdpoly") && (nrparam == 1 || nrparam == 2)){
        createccdpoly(param, nrparam);
    } else if (!strcmp(cmd, "createicdpoly") && (nrparam == 2 || nrparam == 3)){
        createicdpoly(param, nrparam);
    } else if (!strcmp(cmd, "coverduration") && nrparam == 3) {
        coverduration(param);
    } else if (!strcmp(cmd, "coveredlonger") && nrparam == 2) {
        coveredlonger(param);
    } else if (!strcmp(cmd, "coveredshorter") && nrparam == 2) {
        coveredshorter(param);
    } else if (!strcmp(cmd, "intervalcovered") && nrparam == 2) {
        intervalcovered(param);
    } else if (!strcmp(cmd, "avgcover") && (nrparam == 1 || nrparam == 2)) {
        avgcover(param, nrparam);
    } else if (!strcmp(cmd, "openscad") && nrparam == 1) {
        openscad(param);
#endif
    } else if (!strcmp(cmd, "decomposemreg") && nrparam == 3 ) {
        decomposemreg(param);
    } else if (!strcmp(cmd, "decomposepmreg") && nrparam == 2 ) {
        decomposepmreg(param);
    } else {
        usage();
    }

    exit(0);
}

