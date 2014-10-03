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
//[pow] [\verb+^+]

[1] Headerfile of the Point and Vector classes

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype

April - November 2014, S. Schroer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "SourceUnit2.h"
#include "PFace.h"
#include "SourceUnitPair2.h"

namespace mregionops2 {

/***********************************

5 Class SourceUnit2

***********************************/

SourceUnit2::SourceUnit2(const bool _isUnitA, 
             MRegion2* const _mRegion,
             const int _pos,
	     precTimeInterval _interval,
             SourceUnitPair2* const _parent) :
                 
             isUnitA(_isUnitA),
             pos(_pos),
             mRegion(_mRegion), 
             interval(_interval),
             parent(_parent)
{

}
/***********************************

5.1 SourceUnit2::CreatePFaces()

***********************************/

void SourceUnit2::CreatePFaces() {

   cout << "SourceUnit2::CreatePFaces 1" << endl;

   URegionEmb2 uRegion;
   MSegmentData2 segment, restrSegment;
   MSegmentData2 dms;
   PreciseMSegmentData pdms;
   const DbArray<MSegmentData2>* segmentData = mRegion->GetMSegmentData2();
   const DbArray<PreciseMSegmentData>* preciseSegmentData 
         = mRegion->GetPreciseMSegmentData();
   const DbArray<int>* preciseCoordinates = mRegion->GetPreciseCoordinates();
   URegionEmb2 ur;
     
// from MovingRegion3 include, it's another version as inside MovingRegion2
// in MovingRegion2 the sFac is 1
// reason: if scalefactor is different - OPEN what's to do?
// mpz_t sFactor;
// mpz_init(sFactor);
// uint sfactor;
// Skalierungsfaktor für den ganzzahligen Anteil
   int sFac = mRegion->GetScaleFactor();

// old Segement URegion
   Point2D old_is, old_ie, old_fs, old_fe; 

// new Segment derURegion
   Point2D new_is, new_ie, new_fs, new_fe;

// mRegion is a pointer to MRegion2
// pos = aPos or bPos 
   mRegion->Get(pos, uRegion); 

// old Intervall
   precTimeInterval told(uRegion.timeInterval, uRegion.pInterval, 
                         mRegion->GetPreciseInstants());
// new Intervall
   precTimeInterval tnew = interval;

// all segments from uregion  
   for (int i = 0; i < uRegion.GetSegmentsNum(); i++) 
   {      
        cout << "SourceUnit2::Segmente der Uregion i - Segment "
             << i << "  " << uRegion.GetSegmentsNum() << endl;

        uRegion.GetSegment(segmentData, i, dms);
        uRegion.GetPreciseSegment(preciseSegmentData, i, pdms);
        unsigned int cycleNo = dms.GetCycleNo();
        unsigned int faceNo  = dms.GetFaceNo();
        SetCycleStatus(faceNo, cycleNo, NOTYETKNOWN);

//      one mregion in use
//      store pfaces in datastructure
//      preciseSegmentData Nachkommaanteil
//      MSegmentData2 Vorkommaanteile
//      exakt points for old points

        mpq_class isx = dms.GetInitialStartX() 
                        + pdms.GetInitialStartX(preciseCoordinates);
        mpq_class isy = dms.GetInitialStartY() 
                        + pdms.GetInitialStartY(preciseCoordinates);
        mpq_class iex = dms.GetInitialEndX() 
                        + pdms.GetInitialEndX(preciseCoordinates);
        mpq_class iey = dms.GetInitialEndY() 
                        + pdms.GetInitialEndY(preciseCoordinates);
            
        mpq_class fsx = dms.GetFinalStartX() 
                        + pdms.GetFinalStartX(preciseCoordinates);
        mpq_class fsy = dms.GetFinalStartY() 
                        + pdms.GetFinalStartY(preciseCoordinates);
        mpq_class fex = dms.GetFinalEndX() 
                        + pdms.GetFinalEndX(preciseCoordinates);
        mpq_class fey = dms.GetFinalEndY() 
                        + pdms.GetFinalEndY(preciseCoordinates);

        isx = isx * sFac;
        isy = isy * sFac;
        iex = iex * sFac;
        iey = iey * sFac;
        fsx = fsx * sFac;
        fsy = fsy * sFac;
        fex = fex * sFac;
        fey = fey * sFac;

/*     
        old Segement URegion
        old_is, old_ie, old_fs, old_fe; 
*/
        old_is = Point2D(isx, isy);
        old_ie = Point2D(iex, iey);
        old_fs = Point2D(fsx, fsy);
        old_fe = Point2D(fex, fey);

        if (told.start == tnew.start)
        {
          new_is = old_is;
          new_ie = old_ie;  
        } 
        else if (told.end == tnew.start)
        {
          new_is = old_fs;
          new_ie = old_fe;
        }    
        else 
        {
          mpq_class ratio = (tnew.start - told.start) / (told.end - told.start);
          new_is = Point2D(old_is, old_fs, ratio);
          new_ie = Point2D(old_ie, old_fe, ratio);
        }

        if (told.start == tnew.end)
        {
          new_fs = old_is;
          new_fe = old_ie;  
        } 
        else if (told.end == tnew.end)
        {
          new_fs = old_fs;
          new_fe = old_fe;
        }    
        else 
        {
          mpq_class ratio = (tnew.end - told.start) / (told.end - told.start);
          new_fs = Point2D(old_is, old_fs, ratio);
          new_fe = Point2D(old_ie, old_fe, ratio);
        }

        PFace* pFace = new PFace(this, new_is, new_ie, 
                                 new_fs, new_fe, dms.GetInsideAbove(), 
                                 cycleNo, faceNo);
        pFaces.push_back(pFace);
   }  
}

void SourceUnit2::AddTimestamp(mpq_class tim) {
  parent->AddTimestamp(tim);
}

unsigned int SourceUnit2::GetTimeIntervalCount() {
  return parent->GetTimeIntervalCount();
}

mpq_class SourceUnit2::GetTimeIntervalStart(unsigned int i) {
  return parent->GetTimeIntervalStart(i);
}

mpq_class SourceUnit2::GetTimeIntervalEnd(unsigned int i) {
  return parent->GetTimeIntervalEnd(i);
}

void SourceUnit2::SetCycleStatus(unsigned int pfaceNo, 
                                 unsigned int cycleNo,  
                                 CycleStatus stat)
{ 

    // if entry with key exist actualize status
    // else insert neu CyleInfo 

     pair<unsigned int, unsigned int> key(pfaceNo, cycleNo);
     map<pair<unsigned int, unsigned int>, CycleInfo*>::iterator it;
 
     it = cycleInfo.find(key);

     if (it == cycleInfo.end()) {

	 CycleInfo* newentry = new CycleInfo();
         newentry->status = stat;
         cycleInfo.insert(pair<pair<unsigned int, 
                          unsigned int>, 
                          CycleInfo*>(key, newentry ));   
     }
     else
     {
         it->second->status = stat;
     }
}  


void SourceUnit2::FinalizeIntSegs()
{
  vector<PFace*>::iterator iter;

  for (iter = pFaces.begin(); iter != pFaces.end(); iter++)
  {
    (*iter)->FinalizeIntSegs();
  }
}

SourceUnit2::~SourceUnit2() {
    
    vector<PFace*>::iterator iter;
    
    for (iter = pFaces.begin(); iter != pFaces.end(); iter++) {
        delete *iter;
    }
}

const SetOp SourceUnit2::GetOperation() const {
	return parent->GetOperation();
}



void SourceUnit2::CollectRelevantPFaces() {

   vector<PFace*>::iterator iter;

// for all pfaces in one Cycle - woher weiß ich die Anz Zyklen    
   for (iter = pFaces.begin(); iter != pFaces.end(); iter++) {

//     if ( MAP (iter, Cyclus_no))  Frage nach cyclusinfo = HASINTSEGS
//         JA alle ss des Zyklus auf HASINTSEGS setzen
//         Nein alle ss des Zyklus auf NOTYETKNOWN setzen
//



}


/***********************************

5.1 SourceUnit2::PrintPFaces()

***********************************/

void SourceUnit2::PrintPFaces() 

{
     cout << "inside PrintPFaces " << endl;

     for (vector<PFace*>::iterator iter = pFaces.begin(); 
                iter != pFaces.end(); iter++) {
        (*iter)->Print();
     }
}

}

