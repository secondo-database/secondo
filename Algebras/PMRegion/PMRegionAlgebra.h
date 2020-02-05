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


[1] Header File of the PMRegionAlgebra

June, 2018. Florian Heinz <fh@sysv.de>

[TOC]

1 Overview

This header file contains the definition of the class ~PMRegion~
, which is essentially a wrapper for the corresponding class
~PMRegion~ of the libpmregion.

2 Defines and includes

*/
#ifndef __PMREGION_ALGEBRA_H__
#define __PMREGION_ALGEBRA_H__


#include <cmath>
#include <limits.h>
#include <iostream>
#include "stdarg.h"
#include "Attribute.h"
#include "Messages.h"
#include "ListUtils.h"
#include "Algebra.h"

#include "pmregion/PMRegion_internal.h"
#include "pmregion/PMRegion.h"

#include <stdio.h>


/*
3 Conversion functions

These functions convert between Secondo NestedLists and libpmregion RLists.
 
*/
ListExpr RList2NL (pmr::RList r);
pmr::RList NL2RList (ListExpr l);

namespace pmregion {

class PMPoint {
	public:
		double x, y, z;
		PMPoint (double x, double y, double z) : x(x), y(y), z(z) {}
		PMPoint () {}
};

class PMFace {
	public:
		int p1, p2, p3;
		PMFace (int p1, int p2, int p3) : p1(p1), p2(p2), p3(p3) {}
		PMFace () {}
};


/*
4 Class ~PMRegion~

Wraps a libfmr ~PMRegion~, which represents a moving region based on
polyhedra
  
*/
class PMRegion : public Attribute {
public:
    DbArray<PMPoint> points;
    DbArray<PMFace> faces;

    PMRegion() : _pmr(NULL) { cerr << "created pmregion" << endl; };
    PMRegion(bool dummy);
    ~PMRegion();
    
    size_t Sizeof() const { return sizeof(*this); }
    int Compare(const Attribute *) const { return 0; } 
    bool Adjacent(const Attribute *) const { return false; }
    PMRegion* Clone () const;
    size_t HashValue () const { return 1; }
    void CopyFrom (const Attribute *right) {
	    *this = *( (PMRegion*) right);
	    this->_pmr = NULL;
    }
    Flob* GetFLOB(const int i);
    void UpdateFLOBs();
    void UpdateFLOBs(pmr::RList& _rl);
    int NumOfFLOBs() const { return 2; }

    static const std::string BasicType() { return "pmregion"; }
    static const bool checkType(const ListExpr list){
        return listutils::isSymbol(list, BasicType());
    } 
    
    pmr::PMRegion* pmr();
    void pmr(pmr::PMRegion *p) {if ( _pmr) delete _pmr; _pmr = p;}
    void pmr(pmr::PMRegion& p) {if (!_pmr) _pmr=new pmr::PMRegion(); *_pmr=p;}
private:
    pmr::PMRegion *_pmr; // The native libpmregion PMRegion object
};

/*
6 Class ~PMRegionAlgebra~
 
Instantiates the PMRegionAlgebra.
  
*/
class PMRegionAlgebra : public Algebra {
        public:
            PMRegionAlgebra();
};

}

#endif
