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


[1] Header File of the FMRegionAlgebra

September, 2016. Florian Heinz <fh@sysv.de>

[TOC]

1 Overview

This header file contains the definition of the classes ~FMRegion~ and
~CRegion~, which are essentially wrappers for the corresponding classes
~FMRegion~ and ~Region2~ of the libfmr.

2 Defines and includes

*/
#ifndef __FMREGION_ALGEBRA_H__
#define __FMREGION_ALGEBRA_H__


#include <cmath>
#include <limits.h>
#include <iostream>
#include "stdarg.h"
#include "Attribute.h"
#include "Messages.h"
#include "Geoid.h"
#include "ListUtils.h"
#include "Algebra.h"

#include "fmr/RList.h"
#include "fmr/Region2.h"

#include <stdio.h>

/*
3 Conversion functions

These functions convert between Secondo NestedLists and libfmr RLists.
 
*/
ListExpr RList2NL (fmr::RList r);
fmr::RList NL2RList (ListExpr l);

namespace fmregion {

/*
4 Class ~FMRegion~

Wraps a libfmr ~FMRegion~, which represents a moving and rotating region.
  
*/
class FMRegion : public Attribute {
public:
    FMRegion();
    ~FMRegion();
    
    size_t Sizeof() const { return sizeof(*this); }
    int Compare(const Attribute *) const { return 0; } 
    bool Adjacent(const Attribute *) const { return false; }
    FMRegion* Clone () const;
    size_t HashValue () const { return 1; }
    void CopyFrom (const Attribute *right) { *this = *( (FMRegion*) right); }

    static const std::string BasicType() { return "fmregion"; }
    static const bool checkType(const ListExpr list){
        return listutils::isSymbol(list, BasicType());
    } 
    
    fmr::FMRegion *fmr; // The native libfmr FMRegion object
private:
};

/*
5 Class ~CRegion~

Wraps a libfmr ~Region2~, which represents a static region with curved line
segments.
  
*/
class CRegion : public Attribute {
public:
    CRegion();
    ~CRegion();
    
    size_t Sizeof() const { return sizeof(*this); }
    int Compare(const Attribute *) const { return 0; } 
    bool Adjacent(const Attribute *) const { return false; }
    CRegion* Clone () const;
    size_t HashValue () const { return 1; }
    void CopyFrom (const Attribute *right) { *this = *( (CRegion*) right); }
    
    static const std::string BasicType() { return "cregion"; }
    static const bool checkType(const ListExpr list){
        return listutils::isSymbol(list, BasicType());
    } 
    
    fmr::Region2 *reg; // The native libfmr Region2 object
private:
};

/*
6 Class ~FixedMRegionAlgebra~
 
Instantiates the FixedMRegionAlgebra.
  
*/
class FixedMRegionAlgebra : public Algebra {
        public:
            FixedMRegionAlgebra();
};

}

#endif