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

#ifndef GISALGEBRA_GISALGEBRA_H
#define GISALGEBRA_GISALGEBRA_H

/*
declaration of namespace GISAlgebra

*/

namespace GISAlgebra
{
  
/*
Class GISAlgebra integrates GIS Algebra datatypes and operators into SECONDO.

author: Jana Stehmann

*/

class GISAlgebra : public Algebra
{
  public:

  /*
  Constructor GISAlgebra initializes GIS Algebra by adding type constructors
  of GIS Algebra datatypes and by adding operators to GIS Algebra.

  author: Jana Stehmann
  parameters: -
  return value: -
  exceptions: -

  */

  GISAlgebra();
  
  /*
  Destructor ~GISAlgebra deinitializes GIS Algebra.

  author: Jana Stehmann
  parameters: -
  return value: -
  exceptions: -

  */
    
  virtual ~GISAlgebra();
};

}

#endif // GISALGEBRA_GISALGEBRA_H
