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

December 07, 2017

Author: Nicolas Napp

\tableofcontents

1 Header File: CRel2Debug.h

*/
#ifndef CREL2DEBUG_
#define CREL2DEBUG_

//#define CREL2DEBUG // Comment out before deployment.

#ifdef CREL2DEBUG
  #define Assert(condition, except) if(!(condition)) throw except;
  #define debugout(output) std::cout << output << std::endl;
#else
  #define Assert(condition, except) ; // no instructions
  #define debugout(output) ; // no instructions
#endif

#endif /* CREL2DEBUG_ */
