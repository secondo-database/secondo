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


[1] Header File of the Raster2 Algebra


[TOC]

1 Overview

This header file essentially contains the definition of the struct ~Rectangle~.
This class corresponds to the memory representation for the type constructor
~rect~, ~rect3~ and ~rect4~ which are 2-dimensional, 3-dimensional, 4-dimensional
or 8-dimensional rectangles alligned with the axes of each dimension. A rectangle
in such a way can be represented by four, six or eight numbers (two for each dimension).

2 Defines and includes

*/

#ifndef RASTER2_RASTER2ALGEBRA_H
#define RASTER2_RASTER2ALGEBRA_H

using namespace std;

#include <cmath>
#include <limits.h>
#include <iostream>
#include "stdarg.h"
#include "Attribute.h"
#include "Messages.h"
#include "Geoid.h"
#include "ListUtils.h"
#include "RTreeAlgebra.h"
#include "SecondoSMI.h"
#include "StandardTypes.h"

#include <stdio.h>

namespace raster2
{
  
class Raster2Algebra : public Algebra
{
  public:
  /*
  constructors

  */
  Raster2Algebra();
  
  /*
  destructor
    
  */
    
  virtual ~Raster2Algebra();
};

}

#endif // RASTER2_RASTER2ALGEBRA_H
