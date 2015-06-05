/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2004-2007, University in Hagen, Department of Computer Science,
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

*/

#ifndef SECONDODEPENDENCIES_H_
#define SECONDODEPENDENCIES_H_


#ifndef UNIT_TEST
#include "SecondoSMI.h"
#include "Attribute.h"
#include "NestedList.h"
#include "NList.h"
#include "Symbols.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "StringUtils.h"
#include "ListUtils.h"
#include "Serialize.h"
#include "TypeMapUtils.h"
#include "../Raster2/util/types.h"
#include "../Raster2/stype.h"
#include "../Raster2/sreal.h"
#include "../Raster2/sint.h"
#include "../RTree/RTreeAlgebra.h"
extern NestedList* nl;
#endif


#endif /* SECONDODEPENDENCIES_H_ */
