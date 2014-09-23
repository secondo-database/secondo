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

[1] Implementation of the MRegionOpsAlgebra

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype, 

April - November 2014, S. Schroer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "IntersectionSegment.h"
#include "PFace.h"
#include <iostream>


namespace mregionops2 {

bool IntSegWCompare::operator()(const IntersectionSegment* const& s1,
        const IntersectionSegment* const& s2) const {

// &&& to be implemented
  if (s1->GetIntervalStartW() < s2->GetIntervalStartW()) return true;

  if (s2->GetIntervalStartW() < s1->GetIntervalStartW()) return false;

  if (s1->GetIntervalEndW() < s2->GetIntervalEndW()) return true;

  return false;    
}
IntersectionSegment::IntersectionSegment(const Segment3D& s, PFace* _pface,
                              Angle leftAng, Angle rightAng, Direction dir) 
    {

    // The startpoint's t-coord is always lower or equal to the
    // endpoint's t-coord.
    // Note: We don't care for x and y!

    if (s.GetStart().GetT() <= s.GetEnd().GetT()) {

        startXYT = s.GetStart();
        endXYT = s.GetEnd();

    } else 
    {

        startXYT = s.GetEnd();
        endXYT = s.GetStart();
    }
    
    pFace = _pface;
    SetWCoords();

    rightNeighbour = rightAng;
    rightNeighbourDir = dir;
    leftNeighbour = leftAng;
    leftNeighbourDir = dir;

    if (rightNeighbour.IsZero()) rightNeighbour = Angle();
    if (leftNeighbour.IsZero()) leftNeighbour = Angle();
    
}

void IntersectionSegment::UpdateWith(IntersectionSegment* seg)
{
  if (seg->rightNeighbour < rightNeighbour)
  {
    rightNeighbour = seg->rightNeighbour;
    rightNeighbourDir = seg->rightNeighbourDir;
  }

  if (seg->leftNeighbour < leftNeighbour)
  {
    leftNeighbour = seg->leftNeighbour;
    leftNeighbourDir = seg->leftNeighbourDir;
  }
}

void IntersectionSegment::SetWCoords() {

    SetStartWT(GetPFace()->TransformToWT(*GetStartXYT()));
    SetEndWT(GetPFace()->TransformToWT(*GetEndXYT()));
}

AreaDirection IntersectionSegment::GetAreaDirection()

{
  return areaDirection;
}

void IntersectionSegment::Finalize()
{
  SetOp op = pFace->GetOperation();
  if ((op == UNION) || (op == INTERSECTION))
  {
    if (rightNeighbourDir == NEGATIVE)
    {
      areaDirection = RIGHT;
      if (leftNeighbourDir == POSITIVE) areaDirection = BOTH;
    }
    else
    {
      areaDirection = NONE;
      if (leftNeighbourDir == POSITIVE) areaDirection = LEFT;
    }
  }
  else
  {
    if (rightNeighbourDir == POSITIVE)
    {
      areaDirection = RIGHT;
      if (leftNeighbourDir == NEGATIVE) areaDirection = BOTH;
    }
    else
    {
      areaDirection = NONE;
      if (leftNeighbourDir == NEGATIVE) areaDirection = LEFT;
    }
  }
}

void IntersectionSegment::ComputeIntervalW(mpq_class startTime, 
                                           mpq_class endTime)
{
 
//  compute IntervalW
//  Interval: startTime - endTime
//  Segment: GetStartT - GetEndT

    intervalStartW = Point2D(startWT, endWT, 
                     (startTime-GetStartT())/(GetEndT()-GetStartT())).GetW();
    intervalEndW = Point2D(startWT, endWT, 
                     (endTime-GetStartT())/(GetEndT()-GetStartT())).GetW();
}


void IntersectionSegment::Print() {

    Point3D p1 = *GetStartXYT();
    Point3D p2 = *GetEndXYT();
    cout << p1 << " -> " << p2 << endl;
    Point2D p21 = *GetStartWT();
    Point2D p22 = *GetEndWT();
    cout << p21 << " -> " << p22 << endl;
    Angle a = rightNeighbour;
    cout << "right neighbour angle: " << a << endl;
    cout << "right neighbour direction: " << rightNeighbourDir << endl;
    a = leftNeighbour;
    cout << "left neighbour angle: " << a << endl;
    cout << "left neighbour direction: " << leftNeighbourDir << endl;
    AreaDirection ad = GetAreaDirection();
    cout << "area direction: " << ad << endl;
}

void IntersectionSegment::InvertAreaDirection()
{
  switch (areaDirection)
  {
    case LEFT:
     areaDirection = RIGHT;
     break;
    case RIGHT:
     areaDirection = LEFT;
     break;
    default:
     break;
  }
}

bool IntersectionSegment::IsLeftOf(const IntersectionSegment* intSeg) const {
    
    // Precondition: 
    // this->GetStartT() is inside the interval 
    // [intSeg->GetStartT(), intSeg->GetEndT()]
    // and this and intSeg don't intersect in their interior.
       
const mpq_class sideOfStart = GetStartWT()->WhichSide(*intSeg->GetStartWT(), 
                                                       *intSeg->GetEndWT());
    // Null anstatt eps vs -eps
    if (sideOfStart > 0)
        return true;
    
    if (sideOfStart < 0)
        return false;
    
const mpq_class sideOfEnd = GetEndWT()->WhichSide(*intSeg->GetStartWT(), 
                                                   *intSeg->GetEndWT());
    
    return sideOfEnd > 0;
}


} // end namespace
