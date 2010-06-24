/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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
//[_] [\_]

[1] Implementation of the Indoor Algebra

June, 2010  Jianqiu Xu

[TOC]

1 Overview

This implementation file essentially contains the implementation of the classes ~Point~,
~Points~, ~Line~, and ~Region~ used in the Spatial Algebra. These classes
respectively correspond to the memory representation for the type constructors
~point~, ~points~, ~line~, and ~region~.

For more detailed information see SpatialAlgebra.h.

2 Defines and Includes

*/

#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

using namespace std;

#include "../../Tools/Flob/Flob.h"
#include "../../Tools/Flob/DbArray.h"
#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "Symbols.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "IndoorAlgebra.h"
#include "SecondoConfig.h"
#include "AlmostEqual.h"

#include <vector>
#include <queue>
#include <stdexcept>
#include <iostream>
#include <string.h>
#include <string>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <queue>
#include <iterator>
#include <sstream>
#include <limits>


extern NestedList* nl;
extern QueryProcessor* qp;


/*
Creation of the type constructor instance for floor3d

*/
ListExpr Floor3DProperty()
{
//  cout<<"Floor3DProperty()"<<endl;
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"thefloor(floor_height, polygon)");
  return nl->TwoElemList(
          nl->TwoElemList(nl->StringAtom("Creation"),
                          nl->StringAtom("Example Creation")),
          nl->TwoElemList(examplelist,
                         nl->StringAtom("let room1=thefloor(0, r)"))

      );
}

/*
OutPut function for floor3d

*/
ListExpr OutFloor3D(ListExpr typeInfo, Word value)
{
//  cout<<"OutFloor3D()"<<endl;
  Floor3D* fl = (Floor3D*)(value.addr);
  if(!fl->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  if( fl->IsEmpty() ){
    return (nl->TheEmptyList());
  }
  else{
    Region* cr = const_cast<Region*>(fl->GetRegion());
    Region *RCopy=new Region(*cr, true); // in memory

    RCopy->LogicSort();

    HalfSegment hs, hsnext;

    ListExpr regionNL = nl->TheEmptyList();
    ListExpr regionNLLast = regionNL;

    ListExpr faceNL = nl->TheEmptyList();
    ListExpr faceNLLast = faceNL;

    ListExpr cycleNL = nl->TheEmptyList();
    ListExpr cycleNLLast = cycleNL;

    ListExpr pointNL;

    int currFace = -999999, currCycle= -999999; // avoid uninitialized use
    Point outputP, leftoverP;

    for( int i = 0; i < RCopy->Size(); i++ ){
      RCopy->Get( i, hs );
      if (i==0){
        currFace = hs.attr.faceno;
        currCycle = hs.attr.cycleno;
        RCopy->Get( i+1, hsnext );

        if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
            ((hs.GetLeftPoint() == hsnext.GetRightPoint()))){
          outputP = hs.GetRightPoint();
          leftoverP = hs.GetLeftPoint();
        }
        else if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
                 ((hs.GetRightPoint() == hsnext.GetRightPoint()))){
          outputP = hs.GetLeftPoint();
          leftoverP = hs.GetRightPoint();
        }
        else{
          cerr << "\n" << __PRETTY_FUNCTION__ << ": Wrong data format --- "
               << "discontiguous segments!" << endl
               << "\ths     = " << hs     << endl
               << "\thsnext = " << hsnext << endl;
          return nl->SymbolAtom("undef");
        }

        pointNL = OutPoint( nl->TheEmptyList(), SetWord(&outputP) );
        if (cycleNL == nl->TheEmptyList())
        {
          cycleNL = nl->OneElemList(pointNL);
          cycleNLLast = cycleNL;
        }
        else
        {
          cycleNLLast = nl->Append( cycleNLLast, pointNL );
        }
      }
      else{
          if (hs.attr.faceno == currFace){
            if (hs.attr.cycleno == currCycle){
              outputP=leftoverP;
              if (hs.GetLeftPoint() == leftoverP)
                leftoverP = hs.GetRightPoint();
              else if (hs.GetRightPoint() == leftoverP){
                leftoverP = hs.GetLeftPoint();
              }else{
              cerr << "\n" << __PRETTY_FUNCTION__ << ": Wrong data format --- "
                  << "discontiguous segment in cycle!" << endl
                  << "\thh        = " << hs << endl
                  << "\tleftoverP = " << leftoverP << endl;
              return nl->SymbolAtom("undef");
              }

            pointNL=OutPoint( nl->TheEmptyList(),
                              SetWord( &outputP) );
            if (cycleNL == nl->TheEmptyList()){
              cycleNL=nl->OneElemList(pointNL);
              cycleNLLast = cycleNL;
            }
            else{
              cycleNLLast = nl->Append(cycleNLLast, pointNL);
            }
          }
          else{
            if (faceNL == nl->TheEmptyList())
            {
              faceNL = nl->OneElemList(cycleNL);
              faceNLLast = faceNL;
            }
            else
            {
              faceNLLast = nl->Append(faceNLLast, cycleNL);
            }
            cycleNL = nl->TheEmptyList();
            currCycle = hs.attr.cycleno;


            RCopy->Get( i+1, hsnext );
            if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
                ((hs.GetLeftPoint() == hsnext.GetRightPoint())))
            {
              outputP = hs.GetRightPoint();
              leftoverP = hs.GetLeftPoint();
            }
            else if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
                     ((hs.GetRightPoint() == hsnext.GetRightPoint())))
            {
              outputP = hs.GetLeftPoint();
              leftoverP = hs.GetRightPoint();
            }
            else
            {
              cerr << "\n" << __PRETTY_FUNCTION__ << ": Wrong data format --- "
                  << "discontiguous segments in cycle!" << endl
                  << "\ths     = " << hs     << endl
                  << "\thsnext = " << hsnext << endl;
              return nl->SymbolAtom("undef");
            }

            pointNL = OutPoint( nl->TheEmptyList(),
                                SetWord(&outputP) );
            if (cycleNL == nl->TheEmptyList())
            {
              cycleNL = nl->OneElemList(pointNL);
              cycleNLLast = cycleNL;
            }
            else
            {
              cycleNLLast = nl->Append(cycleNLLast, pointNL);
            }
          }
        }else{
          if (faceNL == nl->TheEmptyList())
          {
            faceNL = nl->OneElemList(cycleNL);
            faceNLLast = faceNL;
          }
          else
          {
            faceNLLast = nl->Append(faceNLLast, cycleNL);
          }
          cycleNL = nl->TheEmptyList();


          if (regionNL == nl->TheEmptyList())
          {
            regionNL = nl->OneElemList(faceNL);
            regionNLLast = regionNL;
          }
          else
          {
            regionNLLast = nl->Append(regionNLLast, faceNL);
          }
          faceNL = nl->TheEmptyList();

          currFace = hs.attr.faceno;
          currCycle = hs.attr.cycleno;


          RCopy->Get( i+1, hsnext );
          if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
             ((hs.GetLeftPoint() == hsnext.GetRightPoint())))
          {
            outputP = hs.GetRightPoint();
            leftoverP = hs.GetLeftPoint();
          }
          else if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
                  ((hs.GetRightPoint() == hsnext.GetRightPoint())))
          {
            outputP = hs.GetLeftPoint();
            leftoverP = hs.GetRightPoint();
          }
          else
          {
            cerr << "\n" << __PRETTY_FUNCTION__ << ": Wrong data format --- "
                << "discontiguous segments in cycle!" << endl
                << "\ths     = " << hs     << endl
                << "\thsnext = " << hsnext << endl;
            return nl->SymbolAtom("undef");
          }

          pointNL = OutPoint(nl->TheEmptyList(), SetWord(&outputP));
          if (cycleNL == nl->TheEmptyList())
          {
            cycleNL = nl->OneElemList(pointNL);
            cycleNLLast = cycleNL;
          }
          else
          {
            cycleNLLast = nl->Append(cycleNLLast, pointNL);
          }
        }
      }
    }

    if (faceNL == nl->TheEmptyList())
    {
      faceNL = nl->OneElemList(cycleNL);
      faceNLLast = faceNL;
    }
    else
    {
      faceNLLast = nl->Append(faceNLLast, cycleNL);
    }
    cycleNL = nl->TheEmptyList();


    if (regionNL == nl->TheEmptyList())
    {
      regionNL = nl->OneElemList(faceNL);
      regionNLLast = regionNL;
    }
    else
    {
      regionNLLast = nl->Append(regionNLLast, faceNL);
    }
    faceNL = nl->TheEmptyList();

    RCopy->DeleteIfAllowed();
//    return  regionNL;
    return nl->TwoElemList(nl->RealAtom(fl->GetHeight()), regionNL);
  }
}

/*
input function for data type floor3d
a float value for ground height and a region for space covered

*/
Word InFloor3D(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct)
{
//  cout<<"InFloor3D()"<<endl;
  if(nl->ListLength(instance) != 2){
    string strErrorMessage = "floor3d(): List length must be 2";
    errorInfo = nl->Append(errorInfo,nl->StringAtom(strErrorMessage));
    correct = false;
    return SetWord(Address(0));
  }

  if (nl->IsAtom(instance)){
     correct=false;
     return SetWord( Address(0) );
  }

  if(nl->IsEqual(instance,"undef")){
    correct=false;
    return SetWord(Address(0));
  }

  ListExpr height_list = nl->First(instance);
  if(!nl->IsAtom(height_list) || nl->AtomType(height_list) != RealType){
    string strErrorMessage = "floor3d(): height must be float type";
    errorInfo = nl->Append(errorInfo,nl->StringAtom(strErrorMessage));
    correct = false;
    return SetWord(Address(0));
  }
  float height = nl->RealValue(height_list);
//  cout<<"height "<<height<<endl;

  ListExpr RegionNL = nl->Second(instance);

  Region* cr = new Region( 0 );
  cr->StartBulkLoad();


//  ListExpr RegionNL = instance;
  ListExpr FaceNL, CycleNL;
  int fcno=-1;
  int ccno=-1;
  int edno=-1;
  int partnerno = 0;


    while( !nl->IsEmpty( RegionNL ) ){
      FaceNL = nl->First( RegionNL );
      RegionNL = nl->Rest( RegionNL);
      bool isCycle = true;

      //A face is composed by 1 cycle, and can have holes.
      //All the holes must be inside the face. (TO BE IMPLEMENTED0)
      //Region *faceCycle;

      fcno++;
      ccno=-1;
      edno=-1;

      if (nl->IsAtom( FaceNL ))
      {
        correct=false;
        return SetWord( Address(0) );
      }

      while (!nl->IsEmpty( FaceNL) ){
        CycleNL = nl->First( FaceNL );
        FaceNL = nl->Rest( FaceNL );

        ccno++;
        edno=-1;

        if (nl->IsAtom( CycleNL ))
        {
          correct=false;
          return SetWord( Address(0) );
        }

        if (nl->ListLength( CycleNL) <3)
        {
          cerr << __PRETTY_FUNCTION__ << ": A cycle must have at least 3 edges!"
               << endl;
          correct=false;
          return SetWord( Address(0) );
        }
        else{
          ListExpr firstPoint = nl->First( CycleNL );
          ListExpr prevPoint = nl->First( CycleNL );
          ListExpr flagedSeg, currPoint;
          CycleNL = nl->Rest( CycleNL );
          Points *cyclepoints= new Points( 8 ); // in memory
          Point *currvertex,p1,p2,firstP;
          Region *rDir = new Region(32);
          rDir->StartBulkLoad();
          currvertex = (Point*) InPoint ( nl->TheEmptyList(),
              firstPoint, 0, errorInfo, correct ).addr;
          if (!correct) {
             // todo: delete temp objects
             return SetWord( Address(0) );
          }
          cyclepoints->StartBulkLoad();
          (*cyclepoints) += (*currvertex);
          p1 = *currvertex;
          firstP = p1;
          cyclepoints->EndBulkLoad();
          delete currvertex;
          while ( !nl->IsEmpty( CycleNL) )
          {
//            cout<<"cycle "<<endl;
            currPoint = nl->First( CycleNL );
            CycleNL = nl->Rest( CycleNL );
            currvertex = (Point*) InPoint( nl->TheEmptyList(),
                  currPoint, 0, errorInfo, correct ).addr;
//            cout<<"curvertex "<<*currvertex<<endl;
            if (!correct) return SetWord( Address(0) );

            if (cyclepoints->Contains(*currvertex))
            {
              cerr<< __PRETTY_FUNCTION__ << ": The same vertex: "
                  <<(*currvertex)
                  <<" appears repeatedly within the current cycle!"<<endl;
              correct=false;
              return SetWord( Address(0) );
            }
            else
            {
              p2 = *currvertex;
              cyclepoints->StartBulkLoad();
              (*cyclepoints) += (*currvertex);
              cyclepoints->EndBulkLoad(true,false,false);
            }
            delete currvertex;

            flagedSeg = nl->TwoElemList
            (nl-> BoolAtom(true),
             nl->TwoElemList(prevPoint, currPoint));
            prevPoint=currPoint;
            edno++;
            //Create left dominating half segment
            HalfSegment * hs = (HalfSegment*)InHalfSegment
                      ( nl->TheEmptyList(), flagedSeg,
                       0, errorInfo, correct ).addr;
            if(!correct){
              if(hs){
                cerr << __PRETTY_FUNCTION__ << ": Creation of left dominating "
                     << "half segment (1) failed!" << endl;
                delete hs;
              }
              cr->DeleteIfAllowed();
              return SetWord( Address(0) );
            }
            hs->attr.faceno=fcno;
            hs->attr.cycleno=ccno;
            hs->attr.edgeno=edno;
            hs->attr.partnerno=partnerno;
            partnerno++;
            hs->attr.insideAbove = (hs->GetLeftPoint() == p1);
              //true (L-->R ),false (R--L)
            p1 = p2;

            if (( correct )&&( cr->InsertOk(*hs) ))
            {
              (*cr) += (*hs);
//              cout<<"cr+1 "<<*hs<<endl;
              if( hs->IsLeftDomPoint() )
              {
                (*rDir) += (*hs);
//                cout<<"rDr+1 "<<*hs<<endl;
                hs->SetLeftDomPoint( false );
              }
              else
              {
                hs->SetLeftDomPoint( true );
//                cout<<"rDr+2 "<<*hs<<endl;
                (*rDir) += (*hs);
              }
              (*cr) += (*hs);
//              cout<<"cr+2 "<<*hs<<endl;
              delete hs;
            }
            else
            {
              cerr<< __PRETTY_FUNCTION__ << ": Problematic HalfSegment: "
                  << endl;
              if(correct)
                cerr << "\nhs = " << (*hs) << " cannot be inserted." << endl;
              else
                cerr << "\nInvalid half segment description." << endl;
              correct=false;
              return SetWord( Address(0) );
            }

          }
          delete cyclepoints;

          edno++;
          flagedSeg= nl->TwoElemList
            (nl-> BoolAtom(true),
             nl->TwoElemList(firstPoint, currPoint));
          HalfSegment * hs = (HalfSegment*)InHalfSegment
                  ( nl->TheEmptyList(), flagedSeg,
                    0, errorInfo, correct ).addr;
          if(!correct){
            if(hs){
                cerr << __PRETTY_FUNCTION__ << ": Creation of "
                     << "half segment (2) failed!" << endl;
                delete hs;
            }
            cr->DeleteIfAllowed();
            return SetWord( Address(0) );
          }
          hs->attr.faceno=fcno;
          hs->attr.cycleno=ccno;
          hs->attr.edgeno=edno;
          hs->attr.partnerno=partnerno;
          hs->attr.insideAbove = (hs->GetRightPoint() == firstP);
          //true (L-->R ),false (R--L),
          //the order of typing is last point than first point.
          partnerno++;

          //The last half segment of the region
          if (( correct )&&( cr->InsertOk(*hs) ))
          {
            (*cr) += (*hs);
//             cout<<"cr+3 "<<*hs<<endl;
            if( hs->IsLeftDomPoint() )
            {
              (*rDir) += (*hs);
//              cout<<"rDr+3 "<<*hs<<endl;
              hs->SetLeftDomPoint( false );
            }
            else
            {
              hs->SetLeftDomPoint( true );
//              cout<<"rDr+4 "<<*hs<<endl;
              (*rDir) += (*hs);
            }
            (*cr) += (*hs);
//            cout<<"cr+4 "<<*hs<<endl;
            delete hs;
            rDir->EndBulkLoad(true, false, false, false);


            //To calculate the inside above attribute
            bool direction = rDir->GetCycleDirection();

            int h = cr->Size() - ( rDir->Size() * 2 );
            while ( h < cr->Size())
            {
              //after each left half segment of the region is its
              //correspondig right half segment
              HalfSegment hsIA;
              bool insideAbove;
              cr->Get(h,hsIA);
              /*
                The test for adjusting the inside above can be described
                as above, but was implemented in a different way that
                produces the same result.
                if ( (direction  && hsIA->attr.insideAbove) ||
                     (!direction && !hsIA->attr.insideAbove) )
                {
                  //clockwise and l-->r or
                  //counterclockwise and r-->l
                  hsIA->attr.insideAbove=false;
                }
                else
                  //clockwise and r-->r or
                  //counterclockwise and l-->r
                  true;

              */
              if (direction == hsIA.attr.insideAbove)
                insideAbove = false;
              else
                insideAbove = true;
              if (!isCycle)
                insideAbove = !insideAbove;
              HalfSegment auxhsIA( hsIA );
              auxhsIA.attr.insideAbove = insideAbove;
              cr->UpdateAttr(h,auxhsIA.attr);
              //Get right half segment
              cr->Get(h+1,hsIA);
              auxhsIA = hsIA;
              auxhsIA.attr.insideAbove = insideAbove;
              cr->UpdateAttr(h+1,auxhsIA.attr);
              h+=2;
            }

            //After the first face's cycle read the faceCycle variable is set.
            //Afterwards
            //it is tested if all the new cycles are inside the faceCycle.
            /*
            if (isCycle)
              faceCycle = new Region(rDir,false);
            else
              //To implement the test
            */
            rDir->DeleteIfAllowed();
            //After the end of the first cycle of the face,
            //all the following cycles are
            //holes, then isCycle is set to false.
            isCycle = false;

          }
          else
          {
            correct=false;
            return SetWord( Address(0) );
          }
        }
      }
    }

    cr->SetNoComponents( fcno+1 );
    cr->EndBulkLoad( true, true, true, false );

    correct = true;
//    return SetWord( cr );
    Floor3D* fl = new Floor3D(height, *cr);
    return SetWord(fl);

}

void CloseFloor3D(const ListExpr typeInfo, Word& w)
{
//  cout<<"CloseFloor3D()"<<endl;
//  ((Floor3D*)w.addr)->DeleteIfAllowed();
  delete static_cast<Floor3D*> (w.addr);
  w.addr = NULL;
}

Word CloneFloor3D(const ListExpr typeInfo, const Word& w)
{
//  cout<<"CloneFloor3D()"<<endl;
  Floor3D* fl = new Floor3D(*(Floor3D*)w.addr);
  return SetWord(fl);
}

Word CreateFloor3D(const ListExpr typeInfo)
{
//  cout<<"CreateFloor3D()"<<endl;
  return SetWord (new Floor3D(0.0));
}

void DeleteFloor3D(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeleteFloor3D()"<<endl;
  Floor3D* fl = (Floor3D*)w.addr;
  delete fl;
   w.addr = NULL;
}


int SizeOfFloor3D()
{
//  cout<<"SizeOfFloor3D()"<<endl;
  return sizeof(Floor3D);
}

bool CheckFloor3D(ListExpr type, ListExpr& errorInfo)
{
//  cout<<"CheckFloor3D()"<<endl;
  return nl->IsEqual(type, "floor3d");
}

/*
open function for floor3d

*/

bool OpenFloor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenFloor3D()"<<endl;
  value.addr = new Floor3D(valueRecord, offset, typeInfo);
  return value.addr != NULL;
}

/*
save function for floor3d

*/
bool SaveFloor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//    cout<<"SaveFloor3D()"<<endl;
    Floor3D* fl = (Floor3D*)value.addr;
    return fl->Save(valueRecord, offset, typeInfo);
}

/*
Constructor function for Floor3D

*/
Floor3D::Floor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo):
StandardSpatialAttribute<2>(true),reg(0)
{
//  cout<<"Floor3D(SmiRecord& , size_t& , const ListExpr) here"<<endl;
  valueRecord.Read(&floor_height, sizeof(float), offset);
  offset += sizeof(float);
  ListExpr xType = nl->SymbolAtom("region");
  ListExpr xNumericType =
    SecondoSystem::GetCatalog()->NumericType(xType);
  Region* r = (Region*)Attribute::Open(valueRecord,offset,xNumericType);
  reg = *r;
  delete r;
  SetDefined(true);
}

bool Floor3D::Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo)
{
//  cout<<"Save()"<<endl;
  valueRecord.Write(&floor_height, sizeof(float),offset);
  offset += sizeof(float);
  ListExpr xType = nl->SymbolAtom("region");
  ListExpr xNumericType =
    SecondoSystem::GetCatalog()->NumericType(xType);
  Attribute::Save(valueRecord, offset, xNumericType, &reg);
  return true;
}

/*
Creation of the type constructor instance for point3d

*/

ListExpr Point3DProperty()
{
//  cout<<"Point3DProperty()"<<endl;
  return nl->TwoElemList(
           nl->FourElemList(
             nl->StringAtom("Signature"),
             nl->StringAtom("Example Type List"),
             nl->StringAtom("List Rep"),
             nl->StringAtom("Example List")),
           nl->FourElemList(
             nl->StringAtom("-> DATA"),
             nl->StringAtom("point3d"),
             nl->StringAtom("(x y z)"),
             nl->StringAtom("(10 5 2)")));
}

/*
output function for data type point3d

*/
ListExpr OutPoint3D(ListExpr typeInfo, Word value)
{
  Point3D* p3d = (Point3D*)value.addr;
  if(p3d->IsDefined()){
      return nl->ThreeElemList(
              nl->RealAtom(p3d->GetX()),
              nl->RealAtom(p3d->GetY()),
              nl->RealAtom(p3d->GetZ())
         );
  }else
    return nl->SymbolAtom("undef");
}

/*
input function for data type point3d

*/
Word InPoint3D(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct)
{
  correct = true;
  if( nl->ListLength( instance ) == 3 ) {
    ListExpr first = nl->First(instance);
    ListExpr second = nl->Second(instance);
    ListExpr third = nl->Third(instance);

    correct = listutils::isNumeric(first) &&
              listutils::isNumeric(second) && listutils::isNumeric(third);
    if(!correct){
       return SetWord( Address(0) );
    } else {
      return SetWord(new Point3D(true, listutils::getNumValue(first),
                                       listutils::getNumValue(second),
                                        listutils::getNumValue(third)));
    }
  } else if( listutils::isSymbol( instance, "undef" ) ){
     return SetWord(new Point3D(false));
  }
  correct = false;
  return SetWord( Address(0) );
}

Word CreatePoint3D(const ListExpr typeInfo)
{
//  cout<<"CreatePoint3D()"<<endl;
  return SetWord (new Point3D(false));
}


void DeletePoint3D(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeletePoint3D()"<<endl;
//  ((Point3D*)w.addr)->DeleteIfAllowed();
  Point3D* p3d = (Point3D*)w.addr;
  delete p3d;
   w.addr = NULL;
}

void ClosePoint3D(const ListExpr typeInfo, Word& w)
{
//  cout<<"ClosePoint3D()"<<endl;
//  ((Point3D*)w.addr)->DeleteIfAllowed();
  Point3D* p3d = (Point3D*)w.addr;
  delete p3d;
  w.addr = NULL;
}


Word ClonePoint3D(const ListExpr typeInfo, const Word& w)
{
//  cout<<"CloneFloor3D()"<<endl;
  Point3D* p3d = new Point3D(*(Point3D*)w.addr);
  return SetWord(p3d);
}

void* CastPoint3D(void* addr)
{
  return new (addr)Point3D();
}

int SizeOfPoint3D()
{
//  cout<<"SizeOfPoint3D()"<<endl;
  return sizeof(Point3D);
}

bool CheckPoint3D(ListExpr type, ListExpr& errorInfo)
{
//  cout<<"CheckPoint3D()"<<endl;
  return nl->IsEqual(type, "point3d");
}

/*
save function for point3d

*/
bool SavePoint3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//    cout<<"SavePoint3D()"<<endl;
    Point3D* p3d = (Point3D*)value.addr;
    return p3d->Save(valueRecord, offset, typeInfo);
}

bool OpenPoint3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenPoint3D()"<<endl;
  value.addr = new Point3D(valueRecord, offset, typeInfo);
  return value.addr != NULL;
}

Point3D::Point3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo)
{
  valueRecord.Read(&x, sizeof(double), offset);
  offset += sizeof(double);
  valueRecord.Read(&y, sizeof(double), offset);
  offset += sizeof(double);
  valueRecord.Read(&z, sizeof(double), offset);
  offset += sizeof(double);
  SetDefined(true);
}

bool Point3D::Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo)
{
//  cout<<"Point3D::Save()"<<endl;
  valueRecord.Write(&x, sizeof(double),offset);
  offset += sizeof(double);
  valueRecord.Write(&y, sizeof(double),offset);
  offset += sizeof(double);
  valueRecord.Write(&z, sizeof(double), offset);
  offset += sizeof(double);
//  cout<<x<<" "<<y<<" "<<z<<endl;
  return true;
}

const string SpatialSpecTheFloor =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>float x region -> floor3d</text--->"
"<text>thefloor ( _, _ ) </text--->"
"<text>create a floor3d object.</text--->"
"<text>query thefloor (5.0, r)</text---> ) )";

const string SpatialSpecGetHeight =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>floor3d -> float</text--->"
"<text>getheight ( _ ) </text--->"
"<text>get the ground height of a floor3d object</text--->"
"<text>query getheight(floor3d_1)</text---> ) )";

const string SpatialSpecGetRegion =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>floor3d -> region</text--->"
"<text>getregion ( _ ) </text--->"
"<text>get the ground area of a floor3d object</text--->"
"<text>query getregion(floor3d_1)</text---> ) )";

/*
TypeMap function for operator thefloor

*/
ListExpr TheFloorTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "float x region expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsEqual(arg1, "real") && nl->IsEqual(arg2, "region"))
      return nl->SymbolAtom("floor3d");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator getheight

*/
ListExpr GetHeightTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "floor3d expected";
      return listutils::typeError(err);
  }

  if(nl->IsEqual(nl->First(args), "floor3d"))
      return nl->SymbolAtom("real");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator getregion

*/
ListExpr GetRegionTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "floor3d expected";
      return listutils::typeError(err);
  }

  if(nl->IsEqual(nl->First(args), "floor3d"))
      return nl->SymbolAtom("region");

  return nl->SymbolAtom("typeerror");
}

/*
ValueMap function for operator thefloor

*/
int TheFloorValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  float h = ((CcReal*)args[0].addr)->GetRealval();
  Region* r = (Region*)args[1].addr;
  result = qp->ResultStorage(s);
  Floor3D* fl = (Floor3D*)result.addr;
  fl->SetValue(h, r);
  return 0;
}

/*
ValueMap function for operator getheight

*/
int GetHeightValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Floor3D* fl = (Floor3D*)args[0].addr;
  result = qp->ResultStorage(s);
  CcReal* res = (CcReal*)result.addr;
  if(!fl->IsDefined()) res->Set(false,0.0);
  else
      res->Set(true,fl->GetHeight());
  return 0;
}

/*
ValueMap function for operator getregion

*/
int GetRegionValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Floor3D* fl = (Floor3D*)args[0].addr;
  result = qp->ResultStorage(s);
  Region* res = (Region*)result.addr;
  *res = Region(*fl->GetRegion());
  return 0;
}

Operator thefloor("thefloor",
    SpatialSpecTheFloor,
    TheFloorValueMap,
    Operator::SimpleSelect,
    TheFloorTypeMap
);

Operator getheight("getheight",
    SpatialSpecGetHeight,
    GetHeightValueMap,
    Operator::SimpleSelect,
    GetHeightTypeMap
);

Operator getregion("getregion",
    SpatialSpecGetRegion,
    GetRegionValueMap,
    Operator::SimpleSelect,
    GetRegionTypeMap
);

TypeConstructor floor3d(
    "floor3d", Floor3DProperty,
     OutFloor3D, InFloor3D,
     0, 0,
     CreateFloor3D, DeleteFloor3D,
     OpenFloor3D, SaveFloor3D,
     CloseFloor3D, ClonePoint3D,
     CastPoint3D,
     SizeOfFloor3D,
     CheckFloor3D
);


TypeConstructor point3d(
    "point3d", Point3DProperty,
     OutPoint3D, InPoint3D,
     0, 0,
     CreatePoint3D, DeletePoint3D,
     OpenPoint3D, SavePoint3D,
     ClosePoint3D, CloneFloor3D,
     Floor3D::Cast,
     SizeOfPoint3D,
     CheckPoint3D
);

/*
11 Creating the Algebra

*/

class IndoorAlgebra : public Algebra
{
 public:
  IndoorAlgebra() : Algebra()
  {
    AddTypeConstructor( &floor3d);
    AddTypeConstructor( &point3d);

    floor3d.AssociateKind("DATA");
    point3d.AssociateKind("DATA");

    //operators for new data type floor3d
    AddOperator(&thefloor);
    AddOperator(&getregion);
    AddOperator(&getheight);
  }
  ~IndoorAlgebra() {};
};


/*
12 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeIndoorAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new IndoorAlgebra());
}
