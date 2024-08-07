/* 
----
 * This file is part of libfmr
 * 
 * File:   MBool.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 14, 2016, 2:05 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class MBool

[TOC]

1 Overview

Header file with the class definition for the class ~MBool~

2 Includes and definitions

*/

#ifndef FMR_MBOOL_H
#define FMR_MBOOL_H

#include "fmr_UBool.h"
#include "fmr_RList.h"

#include <vector>
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>

namespace fmr {

/*
3 Declaration for the class ~MBool~

*/
class MBool {
public:
    MBool() {}
    virtual ~MBool() {}
    
    void addUnit (Interval iv, bool val);
    
    std::string ToString();
    RList toRList();
    
    std::vector<UBool> units;
};

}

#endif  /* MBOOL_H */
