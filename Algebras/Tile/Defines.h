/*
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

*/

#ifndef TILEALGEBRA_DEFINES_H
#define TILEALGEBRA_DEFINES_H

namespace TileAlgebra
{
  
/*
definition of TILEALGEBRA\_TRACE Macro

*/

#define TILEALGEBRA_TRACE cout << __PRETTY_FUNCTION__ \
                               << " of object " << this \
                               << " in " << __FILE__ \
                               << " line " << __LINE__ \
                               << endl;
                           
/*
definition of TILEALGEBRA\_STATIC\_TRACE Macro

*/

#define TILEALGEBRA_STATIC_TRACE cout << __PRETTY_FUNCTION__ \
                                      << " in " << __FILE__ \
                                      << " line " << __LINE__ \
                                      << endl;

}

#endif // TILEALGEBRA_DEFINES_H
