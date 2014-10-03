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

[2] Implementation with exakt dataype, 

April - November 2014, S. Schroer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "SourceUnitPair2.h"
#include "PFace.h"

namespace mregionops2 {

/***********************************

4 Class SourceUnitPair2

***********************************/

SourceUnitPair2::SourceUnitPair2(MRegion2* const _unitA, 
                                 int _aPos, 
                                 precTimeInterval _interval,
                                 MRegion2* const _unitB,
                                 int _bPos,
                                 const SetOp _operation,
                                 MRegion2* const _resultMRegion) :

		    unitA(true, _unitA,  _aPos, _interval,this),
		    aPos(_aPos),
		    interval(_interval),
		    unitB(false, _unitB,  _bPos, _interval, this),
		    bPos(_bPos),
		    op(_operation),
		    resultMRegion(_resultMRegion)
{

// zwei Timestamps für Intervallgrenzen in SET
timestamps.insert(interval.start);
timestamps.insert(interval.end);

}     
                  
/***********************************

4.1 SourceUnitPair2::Operate()

***********************************/
void SourceUnitPair2::Operate() {
    
   cout << "SourceUnitPair2::Operate start with aPos, 
           bPos " << aPos << " " << bPos << endl;
   
   if (!(aPos == -1) && !(bPos == -1))
   {

// URegion to aPos=actual timeslot from MRegion2 A           
// in aArray are stored the segments from MRegion2
// aArray = unitA->GetMSegmentData2();
// calculate schnittsegment plus edge

   cout << "SourceUnitPair2::Operate Step1 CreatePFaces" << endl;
   CreatePFaces();

   cout << "SourceUnitPair2::Operate Step1 ComputeIntSegs" << endl;
   ComputeIntSegs(); 

   PrintPFaces();



    } 
    
   else 
    {
        cout << "SourceUnitPair2::Operate else " << endl;   
     
        unitA.IsEmpty() || unitB.IsEmpty() || 
//      (!s && op != UNION && !HasOverlappingBoundingRect());
	(op != UNION && !HasOverlappingBoundingRect());

        switch (op) {       
        case MINUS:
            cout << "SourceUnitPair2::Operate MINUS " << endl;  
//          Result is unit a:
//          unitA.AddToMRegion(resultMRegion);
            break;
        case INTERSECTION:
            cout << "SourceUnitPair2::Operate INTERSECTION " << endl; 
//          Result is empty:
//          Nothing to do.
            break;           
        case UNION:
            cout << "SourceUnitPair2::Operate UNION " << endl; 
//          assert(unitA.IsEmpty() || unitB.IsEmpty());          
//          unitA.AddToMRegion(resultMRegion);
//          unitB.AddToMRegion(resultMRegion);
            break;
        case INSIDE:
            cout << "SourceUnitPair2::Operate INSIDE " << endl; 
            break;
        case INTERSECT:
            cout << "SourceUnitPair2::Operate INTERSECT " << endl; 
            break;
        }
    }
}

/***********************************

4.1 SourceUnitPair2::ComputeOverlapRect()

***********************************/
void SourceUnitPair2::ComputeOverlapRect() {
    cout << "SourceUnitPair2::ComputeOverlapRect " << endl;
}

/***********************************

4.1 SourceUnitPair2::CreatePFaces()

***********************************/

void SourceUnitPair2::CreatePFaces() {

    cout << "SourceUnitPair2::CreatePFaces unitA " << endl;          
    unitA.CreatePFaces();

    cout << "SourceUnitPair2::CreatePFaces unitB " << endl;          
    unitB.CreatePFaces();  
}

/***********************************

4.1 SourceUnitPair2::PrintPFaces()

***********************************/
void SourceUnitPair2::PrintPFaces() {

    cout << "SourceUnitPair2::PrintPFaces" << endl; 
    cout << endl;
    cout << "*********************************************" << endl;
    cout << "PFaces of Unit A:" << endl;
    cout << "*********************************************" << endl;
    cout << endl;

    unitA.PrintPFaces();

    cout << endl;
    cout << "*********************************************" << endl;
    cout << "PFaces of Unit B:" << endl;
    cout << "*********************************************" << endl;
    cout << endl;

    unitB.PrintPFaces();
}
/***********************************

4.1 SourceUnitPair2::ToVrmlFile()

***********************************/
void SourceUnitPair2::ToVrmlFile(bool a, bool b, bool res) {   
     cout << "SourceUnitPair2::ToVrmlFile" << endl; 
}
/***********************************

4.1 SourceUnitPair2::ComputeIntSegs()

***********************************/
void SourceUnitPair2::ComputeIntSegs() {  
    cout << "SourceUnitPair2::ComputeIntSegs" << endl;

    vector<PFace*>::iterator iterA;
    vector<PFace*>::iterator iterB;

//  for each p-face in A do
    for (iterA = unitA.pFaces.begin(); iterA
                            != unitA.pFaces.end(); iterA++) {

//      for each p-face in B do
        for (iterB = unitB.pFaces.begin(); iterB
                                != unitB.pFaces.end(); iterB++) {
            (*iterA)->Intersection(**iterB);
        }
    }

//  Vector mit allen Menge von t-Werten der Schnittsegmentendpunkte 
//  in sortierter Reihenfolge füllen
    SetTimestampVector();

    unitA.FinalizeIntSegs();
    unitB.FinalizeIntSegs();

//  Vorbereitung für Zusammenbau
    unitA.CollectRelevantPFaces();
    unitB.CollectRelevantPFaces();

};

void SourceUnitPair2::SetTimestampVector() 
{
   timestampVector.reserve(timestamps.size());

// Index through SET-Structur
   set<mpq_class>::iterator iter;
   
// Loop through SET Struktur 
   for (iter = timestamps.begin(); iter != timestamps.end(); iter++)
   {
// write SET-Element in vector
       timestampVector.push_back(*iter);
   }
}

}
