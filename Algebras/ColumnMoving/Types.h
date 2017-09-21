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

1 Types.h

*/

#pragma once

#include "AttrArrayType.h"
#include "Sources/IInts.h"
#include "Sources/MInts.h"
#include "Sources/IPoints.h"
#include "Sources/MPoints.h"
#include "Sources/IRegions.h"
#include "Sources/MRegions.h"
#include "Sources/IReals.h"
#include "Sources/MReals.h"
#include "Sources/IBools.h"
#include "Sources/MBools.h"
#include "Sources/IStrings.h"
#include "Sources/MStrings.h"

namespace ColumnMovingAlgebra {
/*
In this file we declare the types of our attribut arrays. The types are
declared by typedefs of the generic class ~AttrArrayType~, which provides
a generic implementation for our types. As template parameter we give
the class that implements the attribut array, the corresponding class
for row oriented relations and strings providing information for
the user interface.

*/

  extern char iintsName[]; 
  extern char iintsInfo[];
  extern char iintsExample[];
  typedef AttrArrayType<IInts, temporalalgebra::IInt, iintsName, iintsInfo, 
    iintsExample> IIntsType;

  extern char mintsName[]; 
  extern char mintsInfo[];
  extern char mintsExample[];
  typedef AttrArrayType<MInts, temporalalgebra::MInt, mintsName, mintsInfo, 
    mintsExample> MIntsType;

  extern char ipointsName[]; 
  extern char ipointsInfo[];
  extern char ipointsExample[];
  typedef AttrArrayType<IPoints, temporalalgebra::IPoint, ipointsName, 
    ipointsInfo, ipointsExample> IPointsType;

  extern char mpointsName[]; 
  extern char mpointsInfo[];
  extern char mpointsExample[];
  typedef AttrArrayType<MPoints, temporalalgebra::MPoint, mpointsName, 
    mpointsInfo, mpointsExample> MPointsType;

  extern char iregionsName[]; 
  extern char iregionsInfo[];
  extern char iregionsExample[];
  typedef AttrArrayType<IRegions, temporalalgebra::IRegion, iregionsName, 
    iregionsInfo, iregionsExample> IRegionsType;

  extern char mregionsName[]; 
  extern char mregionsInfo[];
  extern char mregionsExample[];
  typedef AttrArrayType<MRegions, temporalalgebra::MRegion, mregionsName, 
    mregionsInfo, mregionsExample> MRegionsType;

  extern char irealsName[]; 
  extern char irealsInfo[];
  extern char irealsExample[];
  typedef AttrArrayType<IReals, temporalalgebra::IReal, irealsName, 
    irealsInfo, irealsExample> IRealsType;

  extern char mrealsName[]; 
  extern char mrealsInfo[];
  extern char mrealsExample[];
  typedef AttrArrayType<MReals, temporalalgebra::MReal, mrealsName, 
    mrealsInfo, mrealsExample> MRealsType;

  extern char iboolsName[]; 
  extern char iboolsInfo[];
  extern char iboolsExample[];
  typedef AttrArrayType<IBools, temporalalgebra::IBool, iboolsName, 
    iboolsInfo, iboolsExample> IBoolsType;

  extern char mboolsName[]; 
  extern char mboolsInfo[];
  extern char mboolsExample[];
  typedef AttrArrayType<MBools, temporalalgebra::MBool, mboolsName, 
    mboolsInfo, mboolsExample> MBoolsType;

  extern char istringsName[]; 
  extern char istringsInfo[];
  extern char istringsExample[];
  typedef AttrArrayType<IStrings, temporalalgebra::IString, istringsName, 
    istringsInfo, istringsExample> IStringsType;

  extern char mstringsName[]; 
  extern char mstringsInfo[];
  extern char mstringsExample[];
  typedef AttrArrayType<MStrings, temporalalgebra::MString, mstringsName, 
    mstringsInfo, mstringsExample> MStringsType;

}

