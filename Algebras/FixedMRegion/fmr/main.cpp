/* 
----
 * This file is part of libfmr
 * 
 * File:   main.cpp
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 9, 2016, 2:35 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Test file

[TOC]

1 Testfile

Test the library.

*/

#include <cstdlib>
#include <iostream>
#include <memory>

#include "RList.h"
#include "FMRegion.h"
#include "Region.h"
#include "MPoint.h"
#include "Trochoid.h"
#include "TraversedAreaFactory.h"

using namespace std;
using namespace fmr;

/*
2 Test main routine

*/
#if 0
int main(int argc, char** argv) {
    RList l = RList::parseFile(argv[1]);
    FMRegion fmr(l[4]);
    Region2 r2 = fmr.traversedArea();
    
    cerr << r2.ToString();
    
    
    return 0;
}
#endif