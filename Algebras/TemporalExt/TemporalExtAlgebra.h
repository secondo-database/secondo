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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header file of the TemporalExt Algebra

[TOC]

1 Overview and introduction

The type system of the TemporalExt Algebra can be seen below. All here declared
methods prototypes of the defined classes are implemented in file
TemporalExtAlgebra.cpp. All here declared classes are inherited from classes of
the TemporalAlgebra in order to avoid conflicts with other similar
named implementations.


2 Defines, includes, and constants

*/
#ifndef _TEMPORALEXT_ALGEBRA_H_
#define _TEMPORALEXT_ALGEBRA_H_

#include <iostream>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "NestedList.h"
//#include "DBArray.h"
#include "../../Tools/Flob/DbArray.h"
#include "RectangleAlgebra.h"
#include "DateTime.h"
#include "TemporalAlgebra.h"
#include "Berlin2WGSTemporal.h"

extern NestedList* nl;
extern QueryProcessor* qp;



/*
4 Declaration of classes

4.1 Class ~MPointExt~

This class is an extension of the class MPoint declared in the
TemporalAlgebra.h . It is adding following methods:

~Locations~, ~At~, ~Passes~ and Inside

in order to implement the operations:

mdirection:mapping(point) -> mapping(real)

locations:mapping(point) -> points

at:
mapping(point) x V -> mapping(point)

where
V in {points, line}

passes:
mapping(point) x C -> bool

where
C in {points, line}

Inside:
mpoint X region -> mbool

for supporting the value mapping functions for a part of the signature
belonging these operations.

*/

class MPointExt : public MPoint
{
public:

    MPointExt() : MPoint() {}

    void Locations( Points* result ) const;

    void At( Points* pts, MPoint &result ) const;

    void At( Line* ln, MPoint &result ) const;

    bool Passes( Points* pts ) const;

    bool Passes( Line* ln ) const;

    MBool Inside( const Region& r ) const;
};

/*
4.2 Class MappingExt

This is an extension of the class Mapping declared and implemented in
TemporalAlgebra.h and TemporalAlgebra.cpp respectively. It is adding the
followings methods:

~AtMin~, ~AtMax~, ~At~

in order to implement the operations:

atmin, atmax:mapping(T) -> mapping(T)

where T in {int, bool, string, real}

at:mapping(U) x range(U) -> mapping(U)

where U in {int, bool, string, real}

for supporting the value mapping functions for a part of the signature
belonging these operations.

*/

template<class Unit, class Alpha>
class MappingExt : public Mapping<Unit, Alpha>
{
public:
    MappingExt() : Mapping<Unit, Alpha>() {}

    void AtMin( Mapping<Unit, Alpha> &result ) const;

    void AtMax( Mapping<Unit, Alpha> &result ) const;

    void At( Range<Alpha>* inv, Mapping<Unit, Alpha> &result ) const;
};

/*
4.2 Class URealExt

This is an extension of the class UReal declared and implemented in
TemporalAlgebra.h. It is adding the followings methods:

~GetUnitMin~, ~GetUnitMax~, ~SetUnitMin~, ~SetUnitMax~

*/

class URealExt : public UReal
{
public:
    URealExt() : UReal() {}

    URealExt(bool is_defined):UReal(is_defined) { }

    double GetUnitMin() const { return unit_min; }

    double GetUnitMax() const { return unit_max; }

    void SetUnitMin( double min ) { unit_min = min; }

    void SetUnitMax( double max) { unit_max = max; }

private:
    double unit_min, unit_max;
};

/*
4.2 Class MRealExt

This is an extension of the class MReal declared and implemented in
TemporalAlgebra.h and TemporalAlgebra.cpp respectively. It is adding the
followings methods:

~AtMin~, ~AtMax~, ~At~ and ~Passes~

in order to implement the operations:

atmin, atmax:mapping(real) -> mapping(real)

at:mapping(real) x range(real) -> mapping(real)

passes:mapping(real) x real -> bool

for supporting the value mapping functions for a part of the signature
belonging these operations.

*/

class MRealExt : public MReal
{
public:
    MRealExt() : MReal() {}

    void At( CcReal val, MReal &result ) const;

    void At( RReal* inv, MReal &result ) const;

    void AtMin( MReal &result ) const;

    void AtMax( MReal &result ) const;

    bool Passes( CcReal val ) const;
};

#endif // _TEMPORALEXT_ALGEBRA_H_

