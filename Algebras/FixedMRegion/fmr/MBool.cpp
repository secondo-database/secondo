/* 
 * This file is part of libfmr
 * 
 * File:   MBool.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 14, 2016, 2:05 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Class MBool

[TOC]

1 Overview

The class MBool represents a truth value over one or more time
intervals. Such a pair (value,interval) is called a *unit* (see class ~UBool~)

*/

#include "MBool.h"

using namespace fmr;

/*
2 ~addUnit~

Adds a new unit to this ~MBool~, which represents the
truth value ~val~ over the interval ~iv~.

*/
void MBool::addUnit (Interval iv, bool val) {
    units.push_back(UBool(iv, val));
    std::sort(units.begin(), units.end());

    std::vector<UBool>::iterator ui = units.begin();
    while (ui != units.end()) {
        std::vector<UBool>::iterator ui2 = ui + 1;
        if (ui2 == units.end())
            break;
        if ((ui->iv.end == ui2->iv.start) &&
            (ui->iv.rc || ui2->iv.lc) &&
            (ui->val == ui2->val)) {
            ui->iv.end = ui2->iv.end;
            ui->iv.rc = ui2->iv.rc;
            ui = units.erase(ui2);
        } else {
            ui++;
        }
    }
}

/*
3 ~ToString~

Returns a string representation of this MBool.

*/
std::string MBool::ToString() {
    std::stringstream ss;
    
    for (int i = 0; i < units.size(); i++) {
        ss << units[i].ToString() << "\n";
    }
    
    return ss.str();
}

/*
4 ~toRList~

Returns an RList representation of this MBool.

*/
RList MBool::toRList() {
    RList ret;
    for (int i = 0; i < units.size(); i++) {
        ret.append(units[i].toRList());
    }
    
    return ret;
}
