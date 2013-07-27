/*
----
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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

#ifndef TILEALGEBRA_TILEALGEBRA_H
#define TILEALGEBRA_TILEALGEBRA_H

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{
  
/*
Class TileAlgebra integrates Tile Algebra datatypes and operators into SECONDO.

author: Dirk Zacher

*/

class TileAlgebra : public Algebra
{
  public:

  /*
  Constructor TileAlgebra initializes Tile Algebra by adding type constructors
  of Tile Algebra datatypes and by adding operators to Tile Algebra.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  TileAlgebra();
  
  /*
  Destructor ~TileAlgebra deinitializes Tile Algebra.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */
    
  virtual ~TileAlgebra();
};

}

#endif // TILEALGEBRA_TILEALGEBRA_H
