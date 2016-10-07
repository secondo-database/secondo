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

#include "UBool.h"
#include "RList.h"

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
    
    std::string ToString();
    RList toRList();
    void addUnit (Interval iv, bool val);
    
private:
    std::vector<UBool> units;

};

}

#endif  /* MBOOL_H */
