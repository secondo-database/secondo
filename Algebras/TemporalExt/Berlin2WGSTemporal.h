 

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
//[_] [\_]

*/

#ifndef BERLIN2WGSTEMPORAL_H
#define BERLIN2WGSTEMPORAL_H

#include "TemporalAlgebra.h"

/*
This file is applied to convert BBBike coordinates into WGS84 coordinates.

The formula is taken from the BBBike sources.

http://bbbike.sourceforge.net/index.de.html

*/
class Berlin2WGSTemporal : public Berlin2WGS {
 public:
  Berlin2WGSTemporal() : Berlin2WGS() {}
  void convert(const IPoint* source, IPoint* result);
  void convert(const UPoint* source, UPoint* result);
  void convert(const MPoint* source, MPoint* result);
};

#endif