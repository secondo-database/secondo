/* 
 * This file is part of libfmr
 * 
 * File:   UBool.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 14, 2016, 2:03 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~UBool~

[TOC]

1 Overview

*/

#include "fmr_UBool.h"

using namespace fmr;

/*
2 ~operator<~

Defines an order relation on UBool by its start instant to order
a set of UBool chronologically.

*/
bool UBool::operator<(const UBool& ub) const {
    return iv.start < ub.iv.start;
}

/*
3 ~ToString~

Returns a string representation of this UBool.

*/
std::string UBool::ToString() {
    std::stringstream ss;
    
    ss << iv.ToString() << " " << val << "\n";
    
    return ss.str();
}

/*
4 ~toRList~

Returns an RList representation of this UBool.

*/
RList UBool::toRList() {
    RList ret;
    
    ret.append(iv.toRList());
    ret.append(val);

    return ret;
}
