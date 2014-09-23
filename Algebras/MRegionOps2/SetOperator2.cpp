/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]

[1] Implementation of the MRegionOps2Algebra

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype

April - November 2014, S. Schroer for master thesis.

[TOC]

1 Introduction

This file essentially contains the implementation of the algebra class and
the type- and value mapping functions of the three set operators
~intersection~, ~union~ and ~minus~ with the signature \\
movingregion2 [x] movingregion2 [->] movingregion2 \\
used in the MovingRegion Algebra.

2 Defines and Includes

*/

#include "SetOperator2.h"
#include "SourceUnitPair2.h"

namespace mregionops2 {


/***********************************

3 Class SetOperator2

***********************************/

/***********************************

3.1 SetOperator2::Intersection()

***********************************/

void SetOperator2::Intersection() {
   
 Operate(INTERSECTION);
 cout << ("INTERSECTION inside SetOperator2") << endl;

}

/***********************************

3.1 SetOperator2::Union()

***********************************/

void SetOperator2::Union() {

 Operate(UNION);
 cout << ("UNION inside SetOperator2") << endl;

}

/***********************************

3.1 SetOperator2::Minus()

***********************************/

void SetOperator2::Minus() {

 Operate(MINUS);
 cout << ("MINUS inside SetOperator2") << endl;

}

/***********************************

3.1 SetOperator2::Inside()

***********************************/

void SetOperator2::Inside() {

 Operate(INSIDE);
 cout << ("INSIDE inside SetOperator2") << endl;

}

/***********************************

3.1 SetOperator2::INTERSECT()

***********************************/

void SetOperator2::Intersect() {

 Operate(INTERSECT);
 cout << ("INTERSECT inside SetOperator2") << endl;

}

/***********************************

3.1 SetOperator2::Operate()

***********************************/

void SetOperator2::Operate(const SetOp op) {
    
    if (!a->IsDefined() || !b->IsDefined()) {
        res->SetDefined(false);
        return;
    }
   
    // Compute the RefinementPartition3 of the two 
    // MRegions2 (from Temporal Algebra)
    // sum from the time intervals from both regions 
    // are the t-value for further calculation
    // the Objekt from typ RefinementPartione called rp gets new arguments
    // rp.size = number ob time-slots
    // to the i-time-slot is aPos the pointer to referenz in slot a, 
    // bPos the referenz to slot in b
    
    RefinementPartition3 rp(*a, *b);
    cout << "RefinementPartition3 with rp-Size = " << rp.Size() 
         << " units created.";
    
    precTimeInterval interval;
    int aPos;
    int bPos;
    bool aIsEmpty;
    bool bIsEmpty;
    URegionEmb2 unitA;
    URegionEmb2 unitB; 
    URegionEmb2 unitARestrict;
    URegionEmb2 unitBRestrict;  
    SourceUnitPair2* so;
    
    cout << ("SetOperator2 inside Operate after RefinementPartition") << endl;

//  init
    res->Clear();
    ((DbArray<MSegmentData2>*)res->GetFLOB(1))->clean();
 
    res->StartBulkLoad();
    
//  rp = sum of time-slot from both MRegions
    for (unsigned int i = 0; i < rp.Size(); i++) {
    cout << ("SetOperator2 inside Operate timeslots = ") << i << endl;
        
        // For each interval of the refinement partition...
        // aPos, bPos = position in MRegions as defined
        // time interval from RefinementPartion3
        // use i-te time-slot from rp
        // interval = interval (starttime - endtime)

        rp.Get(i, interval, aPos, bPos);
        cout << ("SetOperator2 inside Operate i ") << i << endl;      
        // new datenfield intervalAsPeriod to store interval
        // Periods intervalAsPeriod(1);
        // intervalAsPeriod.Add(interval);
        
        // if aPos, bPos empty than aIsEmpty, bIsEmpty = True
        aIsEmpty = (aPos == -1);
        bIsEmpty = (bPos == -1);
        
        // if aPos AND bPos empty than END
        assert(!(aIsEmpty && bIsEmpty));
        
        if (aIsEmpty || bIsEmpty) {
            if (op == INTERSECTION) {
               // Result is empty: nothing to do
cout << ("SetOperator2 inside Operate - aIsEmpty || bIsEmpty ") << endl; 
               continue;
            }
            if (op == MINUS && aIsEmpty) {
               // Result is empty: nothing to do
cout << ("SetOperator2 inside Operate - MINUS && aIsEmpty ") << endl; 
               continue;
            }
        }

cout << ("SetOperator2 inside Operate - aPos and bPos not empty") << endl; 

        // pair from SourceUnits inside a, b        
        so = new SourceUnitPair2(a, aPos, interval,
                                 b, bPos, op, res);

        // call methode "operate"
        so->Operate();
       
        // delete all objects
        delete so;
        
    }
             
    res->EndBulkLoad(false);
};

}
