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

May, 2007 Leonardo Azevedo, Rafael Brand

*/

//---------------------------------------------------------------------------
#ifndef PlanoH
#define PlanoH

#include "BBox.h"
//#include "../Signature/rotinas.h"

#define PRODUCT 500 

struct Plane : public BBox2D
{
  long sizeOfCell;
  unsigned numberOfCellsX() const;
  unsigned numberOfCellsY() const;
  unsigned potency() const;
  Plane() : BBox2D() {};
  Plane( Coordinate min, Coordinate max, long cellSize ) :
    BBox2D( min, max ) , sizeOfCell( cellSize ) {};
  static Coordinate findCell(Coordinate point, Plane plane);
};
//---------------------------------------------------------------------------
#endif
