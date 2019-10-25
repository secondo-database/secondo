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
        << "             atinstant     <pmregfile> <instant>" << endl
        << "             perimeter     <pmregfile>" << endl
        << "             area          <pmregfile>" << endl
        << "             traversedarea <pmregfile>" << endl
#if CGAL_VERSION_NR >= 1041400000
        << "             coverduration <pmregfile>" << endl
        << "             coverlength   <scalarfile> <px> <py>" << endl
#endif
        << "             mpointinside  <pmregfile> <mpointfile>" << endl
        << "             intersection  <pmregfile> <pmregfile>" << endl
        << "             union         <pmregfile> <pmregfile>" << endl
        << "             difference    <pmregfile> <pmregfile>" << endl
        << "             intersects    <pmregfile> <pmregfile>" << endl
        << "             pmreg2mreg    <pmregfile>" << endl
        << "             pmreg2off     <pmregfile>" << endl
        << "             mreg2pmreg    <mregfile>" << endl
        ;
    exit(1);
}

static RList file2rlist(char *fname) {
    ifstream i1(fname, std::fstream::in);
    RList ret = RList::parse(i1);

    return ret;
}

static void atinstant(char **param) {
    RList rl = file2rlist(param[0]);
    double instant = atof(param[1]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    while (iters--)
        pmreg.atinstant(instant);
    cout << pmreg.atinstant(instant).ToString() << endl;
}

#if CGAL_VERSION_NR >= 1041400000
static void coverduration(char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    cout << pmreg.coverduration2().toRList().ToString() << endl;
}

static void coverlength(char **param) {
    RList rl = file2rlist(param[0]);
    ScalarField field = ScalarField::fromRList(rl);
    double x = strtod(param[1], NULL);
    double y = strtod(param[2], NULL);
    cout << field.value(Point2d(x, y)) << endl;
}
#endif

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

static void pmreg2mreg(char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    while (iters--)
        pmreg.toMRegion();
    cout << pmreg.toMRegion().ToString() << endl;
}

static void pmreg2off(char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromRList(rl);
    while (iters--)
        pmreg.toOFF();
    cout << pmreg.toOFF() << endl;
}

static void mreg2pmreg(char **param) {
    RList rl = file2rlist(param[0]);
    PMRegion pmreg = PMRegion::fromMRegion(rl);
    while (iters--)
        pmreg.toRList();
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

// Program entry point
int main (int argc, char **argv) {
    if (argc < 2)
        usage();

    // For benchmarking
    if (getenv("ITERS"))
        iters = atoi(getenv("ITERS"));

    char *cmd = argv[1];
    int nrparam = argc - 2;
    char **param = argv+2;

    if (!strcmp(cmd, "pmreg2mreg") && nrparam == 1) {
        pmreg2mreg(param);
    } else if (!strcmp(cmd, "pmreg2off") && nrparam == 1) {
        pmreg2off(param);
    } else if (!strcmp(cmd, "atinstant") && nrparam == 2) {
        atinstant(param);
#if CGAL_VERSION_NR >= 1041400000
    } else if (!strcmp(cmd, "coverduration") && nrparam == 1) {
        coverduration(param);
    } else if (!strcmp(cmd, "coverlength") && nrparam == 3) {
        coverlength(param);
#endif
    } else if (!strcmp(cmd, "perimeter") && nrparam == 1) {
        perimeter(param);
    } else if (!strcmp(cmd, "area") && nrparam == 1) {
        area(param);
    } else if (!strcmp(cmd, "traversedarea") && nrparam == 1) {
        traversedarea(param);
    } else if (!strcmp(cmd, "intersects") && nrparam == 2) {
        intersects(param);
    } else if (!strcmp(cmd, "mreg2pmreg") && nrparam == 1) {
        mreg2pmreg(param);
    } else if (!strcmp(cmd, "intersection") && nrparam == 2) {
        intersection(param);
    } else if (!strcmp(cmd, "union") && nrparam == 2) {
        do_union(param);
    } else if (!strcmp(cmd, "mpointinside") && nrparam == 2) {
        mpointinside(param);
    } else if (!strcmp(cmd, "difference") && nrparam == 2) {
        difference(param);
    } else {
        usage();
    }

    exit(0);
}

