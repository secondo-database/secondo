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

[1] Implementation

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include <iostream>

#include "IntSegContainer.h"
#include "IntersectionSegment.h"
#include "PFace.h"


namespace mregionops2 {
/*

1 Struct IntSegCompare

*/

bool IntSegCompare::operator()(const IntersectionSegment* const& s1,
        const IntersectionSegment* const& s2) const {

    // We sort by (t_start, w_start, IsLeft())
    // Precondition: s1->GetStartT() < s1->GetEndT() && 
    //               s2->GetStartT() < s2->GetEndT()

    if (s1->GetStartT() < s2->GetStartT())
        return true;

    if (s1->GetStartT() > s2->GetStartT())
        return false;
       
    if (s1->GetStartW() < s2->GetStartW())
        return true;

    if (s1->GetStartW() >  s2->GetStartW())
        return false;
    
    if (*(s1->GetEndWT()) == *(s2->GetEndWT()))
        return false;
    
    return s1->GetEndWT()->IsLeft(*(s2->GetStartWT()), *(s2->GetEndWT()));
    
}

IntSegContainer::~IntSegContainer() {

    set<IntersectionSegment*>::iterator iter;

    for (iter = intSegs.begin(); iter != intSegs.end(); ++iter) {

        delete *iter;
    }
}

void IntSegContainer::AddIntSeg(IntersectionSegment* seg) {

    set<IntersectionSegment*>::iterator iter;

    iter = intSegs.find(seg);
    if (iter != intSegs.end())
    {
     cout << "intersection segment is already defined" << endl;
     seg->Print();
     cout << endl;
     (*iter)->Print();
     cout << endl;
     (*iter)->UpdateWith(seg);
     cout << "intersection segment after update" << endl;
     (*iter)->Print();
     cout << endl;
     delete(seg);
     return;
    }
    
    intSegs.insert(seg);
}

void IntSegContainer::FinalizeIntSegs()
{
  set<IntersectionSegment*>::const_iterator iter;

  for (iter = intSegs.begin(); iter != intSegs.end(); ++iter)
  {
    (*iter)->Finalize();
  }
}

void IntSegContainer::FillIntSegsTable(unsigned int count)
{
// Nr from timeslot pick all InterSegs which are inside
  set<IntersectionSegment*>::const_iterator iter;

  for (iter = intSegs.begin(); iter != intSegs.end(); ++iter)
  {
    unsigned int ind = 0;
    
    while ((ind < count) &&
    ((*iter)->GetStartT() >= (*iter)->GetPFace()->GetTimeIntervalEnd(ind)))
    ind++;
    
    while ((ind < count) &&
    ((*iter)->GetEndT() > (*iter)->GetPFace()->GetTimeIntervalStart(ind)))
    {
    (*iter)->GetPFace()->AddToIntSegsTable(ind, *iter);
    ind++;
    }

  }
}

void IntSegContainer::InvertAreaDirections()
{
  set<IntersectionSegment*>::const_iterator iter;

  for (iter = intSegs.begin(); iter != intSegs.end(); ++iter)
  {
    (*iter)->InvertAreaDirection();
  }
}

void IntSegContainer::Print() const {
    
    if (intSegs.empty()) {
        
        cout << "Empty." << endl;
        return;
    }

    set<IntersectionSegment*>::const_iterator iter;

    for (iter = intSegs.begin(); iter != intSegs.end(); ++iter) {
        (*iter)->Print();
        cout << endl;
    }
}

}
