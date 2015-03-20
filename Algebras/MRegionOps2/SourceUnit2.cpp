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

[1] Codefile of SourceUnit class

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/
#include "SourceUnit2.h"
#include "PFace.h"
#include "SourceUnitPair2.h"
#include <stdlib.h>
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
    myDebugId=0;
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
// scalefactor for ganzzahligen Anteil
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

//     
//      old Segement URegion
//      old_is, old_ie, old_fs, old_fe; 
//
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
    pFace->SetDebugId(myDebugId++);
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
    cout << "insert new" << endl;
     CycleInfo* newentry = new CycleInfo();
         newentry->status = stat;
         cycleInfo.insert(pair<pair<unsigned int, 
                          unsigned int>, 
                          CycleInfo*>(key, newentry ));   
     }
     else
     {
    cout << "actualize status" << endl;
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



void SourceUnit2::CollectRelevantPFaces(vector<PFace*>* storage)
{
    string mystring; //just for stopping the console, debugging

    cout << "SourceUnit2::CollectRelevantPFaces started for operation: ";
    switch(GetOperation())
    {
        case INTERSECTION: cout << "INTERSECTION/Schnittmenge\n"; break;
        case UNION: cout << "UNION/Vereinigung\n"; break;
        case MINUS: cout << "MINUS\n"; break;
        case INSIDE: cout << "INSIDE\n"; break;
        case INTERSECT: cout << "INTERSECT\n"; break;
    }


    vector<PFace*>::iterator iter;
    for (iter = pFaces.begin(); iter != pFaces.end(); iter++)
    {

        //(*iter)->Print();
        
        switch(GetOperation())
        {
            case INTERSECTION: 
                cout << "INTERSECTION/Schnittmenge\n";
                if((*iter)->HasIntersegs() == true)
                {
                    cout << "PFace has intersegs --> is relevant\n";
                    storage->push_back(*iter);
                }
                else
                {
                    if(IsPFaceInsidePartner(*iter)==true)
                    {

                        storage->push_back(*iter);
                    }
                    else

                }
                break;

            case UNION:
if((*iter)->HasIntersegs() == false && IsPFaceInsidePartner(*iter)==false)
                {

                    storage->push_back(*iter);
                }
            else if ((*iter)->HasIntersegs() == true)
                {
                    cout << "PFace has intersegs --> is relevant\n";
                    storage->push_back(*iter);
                }
            else

                break;
            case MINUS: 
                if(IsUnitA()==true && (*iter)->HasIntersegs() == false)
                {
                    
                    storage->push_back(*iter);
                }
                if(IsUnitB()==true)
                {
                    if ((*iter)->HasIntersegs() == true)
                    {
                    
                        storage->push_back(*iter);
                    }
                    else
                    {
                        if(IsPFaceInsidePartner(*iter)==true)
                        {

                            storage->push_back(*iter);
                        }                    
                        else
                        {

                        }                    
                    }
                }
        }
        
    }

    for (iter = pFaces.begin(); iter != pFaces.end(); iter++)
    {
     //is pface in map?
pair<unsigned int, unsigned int> key((*iter)->Get_faceNo(),
(*iter)->Get_cycleNo());
map<pair<unsigned int, unsigned int>, CycleInfo*>::iterator it;
        it = cycleInfo.find(key);
        if (it != cycleInfo.end())
        {

        }
        else
        {
            
        }
    }


// show content:
   map<pair<unsigned int, unsigned int>, CycleInfo*>::iterator it;

    for (it = cycleInfo.begin(); it != cycleInfo.end(); ++it)
    {
        
    }

cout << "SourceUnit2::CollectRelevantPFaces finished" << endl;
}

bool SourceUnit2::IsPFaceInsidePartner(PFace* pface) 
{
    //cout << "IsPFaceInsidePartner()\n";

    bool result = false;
    if(IsUnitA())
        cout << "i'm A  -> ";
    else
        cout << "i'm B  -> ";

    SourceUnit2* partner = GetPartner();
    if(partner->IsUnitA())
        cout << "and my partner is A\n";
    else
        cout << "and my partner is B\n";
    
    //startpoint instead MidPoint, because intersection exist
    const Point3D p = pface->GetA_XYT();

    const Point p2D(true, p.GetX().get_d(), p.GetY().get_d());
    if(IsUnitA()==true && partner->IsUnitB()==true)
    
    else if(IsUnitB()==true && partner->IsUnitA()==true)
    
    else
    cout << "You should never see this message!\n";        
    
    cout << "Partner Unit has " << partner->GetPFaceCount() << " pFaces\n";    
    vector<PFace*> pFacesOfPartner = partner->GetPFaces();

    int polygonCorners = partner->GetPFaceCount();
    int cornerIndex = 0;
    float polyX[polygonCorners];
    float polyY[polygonCorners];

    vector<PFace*>::iterator iter;
    for (iter = partner->pFaces.begin(); iter != partner->pFaces.end(); iter++)
    {
        Point3D startPoint = (*iter)->GetA_XYT();
        polyX[cornerIndex] = (float)startPoint.GetX().get_d();
        polyY[cornerIndex] = (float)startPoint.GetY().get_d();
        cornerIndex++;
    }
    result =pointInPolygon(polyX,polyY,(float)p.GetX().get_d(), 
(float)p.GetY().get_d(), polygonCorners);

    if (result == true)
        cout << "point is INSIDE of partner unit\n"; 
    else
        cout << "point is OUTSIDE of partner unit\n";

    return result;
}

//http://alienryderflex.com/polygon/
bool SourceUnit2::pointInPolygon(float  polyX[],float  polyY[],float  x, 
float y, int polyCorners)
{
  int   i, j=polyCorners-1 ;
  bool  oddNodes=false      ;

  for (i=0; i<polyCorners; i++) {
    if ((polyY[i]< y && polyY[j]>=y
    ||   polyY[j]< y && polyY[i]>=y)
    &&  (polyX[i]<=x || polyX[j]<=x)) {
oddNodes^=(polyX[i]+(y-polyY[i])/(polyY[j]-polyY[i])*(polyX[j]-polyX[i])<x); }
    j=i; }

  return oddNodes; }


bool SourceUnit2::HasIntersecs()
{
    bool result =false;
        vector<PFace*>::iterator iter;
    for (iter = pFaces.begin(); iter != pFaces.end(); iter++)
    {
        if((*iter)->HasIntersegs() == true)
        {
            result=true;
            break;
        }
    }
    return result;
}

bool SourceUnit2::IsInsidePartner()
{
    bool result =true;
        vector<PFace*>::iterator iter;
    for (iter = pFaces.begin(); iter != pFaces.end(); iter++)
    {
        if(IsPFaceInsidePartner(*iter)==false)
        {
            result=false;
            break;
        }
    }
    return result;
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
SourceUnit2* SourceUnit2::GetPartner()
{
    return partner;
}
int SourceUnit2::GetPFaceCount()
{
    return pFaces.size();
}
vector<PFace*> SourceUnit2::GetPFaces() 
{
    return pFaces;
}
void SourceUnit2::AddToMRegion(MRegion2* const target) {

    if (IsEmpty())
        return;

DbArray<MSegmentData>* targetArray = (DbArray<MSegmentData>*)target->GetFLOB(1);
    
    const int segmentsStartPos = targetArray->Size();

    precTimeInterval tmp =     GetTimeInterval();
    Instant starttime(instanttype);
    Instant endtime(instanttype);
    starttime.ReadFrom(tmp.start.get_d());
    endtime.ReadFrom(tmp.end.get_d());
    const Interval<Instant> interval(starttime, endtime, true, false);

    URegionEmb targetUnit(interval, segmentsStartPos);
    
    MSegmentData segment;
      
    URegionEmb2 ur;
    mRegion->Get(0, ur);
    for (int i = 0; i < ur.GetSegmentsNum(); i++)
    {
    cout << "AddToMRegion() index:" << i << endl;

//        uRegion->GetSegment(array, i, segment);
//        targetUnit.PutSegment(targetArray, i, segment, true);
    }
    target->Add(ur);
}

}

