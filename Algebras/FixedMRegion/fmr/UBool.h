/* 
----
 * This file is part of libfmr
 * 
 * File:   UBool.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 14, 2016, 2:03 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class UPoint

[TOC]

1 Overview

Header file with the class definition for the class ~UBool~

2 Includes and definitions

*/

#ifndef FMR_UBOOL_H
#define FMR_UBOOL_H

#include <sstream>
#include <string>

#include "Interval.h"
#include "RList.h"

namespace fmr {

/*
3 Definition of class UBool

*/
class UBool {
public:
    // Constructors
    UBool() {}
    UBool(Interval iv, bool val) : iv(iv), val(val) {}
    virtual ~UBool() {}
    
    // Methods
    bool operator<(const UBool& ub) const;
    RList toRList();
    std::string ToString();
    
    // Fields
    Interval iv;
    bool val;
};

}

#endif  /* UBOOL_H */

