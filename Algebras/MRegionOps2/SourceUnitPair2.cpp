/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen,
Department of Computer Science,
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
//[pow] [\verb+^+]

[1] Codefile of SourceUnitPair class

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

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
            resultMRegion(_resultMRegion),
        resultUnitFactory(_resultMRegion,this)
{

// two timestamps für intervall in SET
timestamps.insert(interval.start);
timestamps.insert(interval.end);
unitA.SetPartner(&unitB);
unitB.SetPartner(&unitA);
specialOperationsResult=false;
}     
                  
/***********************************

4.1 SourceUnitPair2::Operate()

***********************************/
void SourceUnitPair2::Operate() {
    

 
   if (!(aPos == -1) && !(bPos == -1))
   {

// URegion to aPos=actual timeslot from MRegion2 A           
// in aArray are stored the segments from MRegion2
// aArray = unitA->GetMSegmentData2();
// calculate schnittsegment plus edge

   //cout << "SourceUnitPair2::Operate Step1 CreatePFaces start" << endl;
   CreatePFaces();
   //cout << "SourceUnitPair2::Operate Step1 CreatePFaces end" << endl;

   //cout << "SourceUnitPair2::Operate Step1 ComputeIntSegs start" << endl;
   ComputeIntSegs(); 
   //cout << "SourceUnitPair2::Operate Step1 ComputeIntSegs end" << endl;

   //cout << "SourceUnitPair2::Operate PrintPFaces start" << endl;
   //PrintPFaces();
   //cout << "SourceUnitPair2::Operate PrintPFaces end" << endl;

   switch (op) {       
        case INSIDE:
            cout << "SourceUnitPair2::Operate INSIDE " << endl; 
            if(unitA.IsInsidePartner() == true)
            {
                cout << "unit a is completely inside unit b\n";
                specialOperationsResult=true;
            }    
            else
            {
                cout << "unit a NOT is completely inside unit b\n";
                specialOperationsResult=false;
            }
            break;
        case INTERSECT:
            cout << "SourceUnitPair2::Operate INTERSECT " << endl; 
            if(unitA.HasIntersecs() == true)
            {
                cout << "unit a intersects unit b\n";
                specialOperationsResult=true;
            }
            else
            {
                cout << "unit a doesn't intersect unit b\n";
                specialOperationsResult=false;
            }    
            break;
    default:
        vector<PFace*>::iterator iter;
 for (iter = myRelevantPFaces.begin(); iter != myRelevantPFaces.end(); iter++)
        {
            cout << "myRelevantPFaces: ";
            (*iter)->PrintIdentifier();
        }

        vector<mpq_class>::iterator iter2;
        unsigned int counter=0;
        
        Instant starttime(instanttype);
        Instant endtime(instanttype);
        unsigned int index = 0;
        double start = -1;
        double end = -1;
 for (iter2 = timestampVector.begin(); iter2 != timestampVector.end(); iter2++)
        {
            cout << "timestampVector[" << counter << "] = " << *iter2 << endl;
            mpq_class tmp = *iter2;
            if(counter == 0)
            {
                start = tmp.get_d();
                cout << "start get_d()=" << start << " --> ";
                Instant starttime(start);
                counter=1;
                cout << "we have a start time\n";
            }        
            else
            {
                end=tmp.get_d();
                cout << "end get_d()=" << end << " --> ";
                Instant endtime(end);

                cout << "we have an end time\n";
                counter = 0;
           const Interval<Instant> interval(starttime, endtime, true, false);

                BuildNewResultUnit(start,end,index);
                index++;

                const bool MERGE_RESULT_MSEGMENTS = false;
                resultUnit->EndBulkLoad(MERGE_RESULT_MSEGMENTS);
                //ConstructResultUnitAsURegionEmb();
                //delete resultUnit;
            }
        }
    break;
    }
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
            unitA.AddToMRegion(resultMRegion);
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


void SourceUnitPair2::ConstructResultUnitAsURegionEmb()
{
    cout << "SourceUnitPair2::ConstructResultUnitAsURegionEmb() started\n";

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
    cout << "SourceUnitPair2::ComputeIntSegs started" << endl;

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

//  Vector with all t-values from Schnittsegmentendpunkte 
//  insert sorted
    SetTimestampVector();


    unitA.FinalizeIntSegs();
    unitB.FinalizeIntSegs();


//  preparation for result MRegion
    unitA.CollectRelevantPFaces(&myRelevantPFaces);
    unitB.CollectRelevantPFaces(&myRelevantPFaces);
cout << "SourceUnitPair2::ComputeIntSegs finished" << endl;
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

const bool MERGE_RESULT_MSEGMENTS = false;

void SourceUnitPair2::BuildNewResultUnit(double startT, 
double endT, unsigned int index) 
{


    Instant starttime(instanttype);
    Instant endtime(instanttype);
    starttime.ReadFrom((double)1);
    endtime.ReadFrom((double)3);
    const Interval<Instant> interval(starttime, endtime, true, false);
    resultUnit = new ResultUnit(interval);
    resultUnit->StartBulkLoad();

/*
    vector<PFace*>::iterator iter;
    for (iter = myRelevantPFaces.begin(); iter != myRelevantPFaces.end(); iter++)
    {
        cout << "processing relevant pface\n";    
        if((*iter)->HasIntersegs()==true)
        {
            vector<IntersectionSegment*> segments = (*iter)->getIntersectionSegmentByInterval(index);
        }
    }
*/
    Point3D as(1,1,1);
    Point3D am(1,1,2);
    Point3D ae(1,1,3);


    Point3D bs(1,2,1);
    Point3D bm(1,2,2);
    Point3D be(1,2,3);

    Point3D cs(2,1,1);
    Point3D cm(2,1,2);
    Point3D ce(2,1,3);
    

    //between point a and b
    Segment3D a2b(as, bs);
    Segment3D b2c(bs, cs);
    Segment3D c2a(cs, as);
    resultUnit->AddSegment(a2b);
    resultUnit->AddSegment(b2c);
    resultUnit->AddSegment(c2a);

    resultUnit->EndBulkLoad(MERGE_RESULT_MSEGMENTS);
/* there is no IsEmpty implemented yet
    if (resultUnit->IsEmpty())
        cout << "(embedded) ConstructResultUnitAsURegionEmb ResultUnit is empty\n";
    else
        cout << "(embedded) ConstructResultUnitAsURegionEmb ResultUnit is NOT empty\n";
*/
    cout << "adding resultUnit to final result...";

DbArray<MSegmentData>* array=(DbArray<MSegmentData>*)resultMRegion->GetFLOB(1);
/*    
    URegionEmb2* ure = resultUnit->ConvertToURegionEmb(array,interval);
    resultMRegion->Add(*ure);
    delete ure;
*/
    cout << "done\n";
    delete resultUnit;
    cout << "BuildNewResultUnit() finished\n";
}


}
