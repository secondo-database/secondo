/*
----
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, 
Faculty of Mathematics and Computer Science,
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


[TOC]

1 Helper classes and methods

1.1 Some forward declaration of helper methods

*/

#include "IntSegContainer.h"

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
    
    // s1->GetStartT() == s2->GetStartT()
    
    if (s1->GetStartW() < s2->GetStartW())
        return true;

    if (s1->GetStartW() >  s2->GetStartW())
        return false;

    // s1->GetStartW() == s2->GetStartW()
    
    if (*(s1->GetEndWT()) == *(s2->GetEndWT()))
        return true;
    
    //return s1->IsLeftOf(s2);
return s1->GetEndWT()->IsLeft(*(s2->GetStartWT()), *(s2->GetEndWT()));
    
}


/*
1 Class IntSegContainer

*/

IntSegContainer::~IntSegContainer() {

 set<IntersectionSegment*>::iterator iter;

 for (iter = intSegs.begin(); iter != intSegs.end(); ++iter) {

    delete *iter;
    }

}


void IntSegContainer::Print() const {
    
    if (intSegs.empty()) {
        cout << "Empty." << endl;
        return;
    }

    set<IntersectionSegment*>::const_iterator iter;

    for (iter = intSegs.begin(); iter != intSegs.end(); ++iter) {

        //cout << (*iter)->GetID() << " ";
        (*iter)->Print();
        cout << endl;
    }

}

void IntSegContainer::PrintActive() const {

}
}
