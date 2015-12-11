/******************************************************************************
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

//paragraph [1] Title: [{\Large \bf] [}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Declaration of UploadUnit

May 2010, Daniel Brockmann

1 Overview

The UploadUnit is a spatio-temporal point with the following attributes:

- Moving object id (id)
- Time instant (t)
- 2D position information (pos)

It represents the current position and time of a moving object.

******************************************************************************/

#ifndef __UPLOAD_UNIT_H__
#define __UPLOAD_UNIT_H__


#include "NestedList.h"
#include "ListUtils.h"

/******************************************************************************

2 Definition of UnitPos

The UnitPos structure is a position information in dependence on a 2D point.

******************************************************************************/

struct UnitPos
{
  UnitPos() {}
  UnitPos( double X, double Y ): x( X ), y( Y ) {}
  double x; // x coordinate
  double y; // y coordinate
};


/******************************************************************************

3 Declaration of class UploadUnit

******************************************************************************/

class UploadUnit : public Attribute
{
  public:
    // Basic constructor
    UploadUnit( int ID, Instant T, UnitPos POS);
    // Copy constructor
    UploadUnit( const UploadUnit& UNIT );
    // Standard constructor
    UploadUnit() {}
    // Destructor
   ~UploadUnit() {}

    // The mandatory set of algebra support functions
    static Word    In( const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct );
    static ListExpr Out( ListExpr typeInfo, Word value );
    static Word     Create( const ListExpr typeInfo );
    static void     Delete( const ListExpr typeInfo, Word& w );
    static bool     Open( SmiRecord& valueRecord, size_t& offset,
                          const ListExpr typeInfo, Word& value );
    static bool     Save( SmiRecord& valueRecord, size_t& offset,
                          const ListExpr typeInfo, Word& w );

    static void     Close( const ListExpr typeInfo, Word& w );
    static Word     Clone( const ListExpr typeInfo, const Word& w );
    static bool     KindCheck( ListExpr type, ListExpr& errorInfo );
    static int     SizeOfObj();
    static void* Cast(void* addr);
    static ListExpr Property();

    // type name used in Secondo
    inline static const std::string BasicType() { return "uploadunit";}
    static const bool checkType(const ListExpr type){
       return listutils::isSymbol(type, BasicType());
    }

    // Methods for the abstract Attribute class
    int NumOfFLOBs() const;
    Flob *GetFLOB(const int i);
    int Compare(const Attribute*) const;
    bool Adjacent(const Attribute*) const;
    UploadUnit* Clone() const;
    size_t Sizeof() const;
    size_t HashValue() const;
    void CopyFrom(const Attribute* right);
    std::ostream& Print( std::ostream& os ) const;

    // Returns the moving object id
    int GetID()   const;
    // Returns the instant of time
    Instant GetTime() const;
    // Returns the position information
    UnitPos GetPos()  const;

  private:
    int      id;   // moving object id
    Instant  t;    // instant of time
    UnitPos  pos;  // position information
};

#endif
