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
//[pow] [\verb+^+]

[1] Implementation

Oktober 2014 - Maerz 2015, S. Schroer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "Angle.h"

namespace mregionops2 {

Angle Angle::GetOpposite()
{
  
  mpq_class a = angle + Maxvalue / 2;
  Normalize();
  
  return Angle(a);
}

void Angle::Normalize()
{

  // Angles lie between 0 and 4
  while (angle >= Maxvalue) angle = angle - Maxvalue;
  while (angle < 0) angle = angle + Maxvalue;

}

}  /* end namespace mregionops2 */







