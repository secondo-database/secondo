/*
---- 
This file is part of SECONDO.

Copyright (C) 2002-2007, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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


February 2007, M. Spiekermann: Code moved from Utifunctions.cpp into this
file in order to avoid interdependencies of the Tools/Utilities code with
other Secondo modules. See makefile.libs for details.


1 Class ~NList~

The private reference to a nested list instance.

*/

#include "NList.h"

NestedList* NList::nlGlobal = 0;

/*
Implementation of the outstream-operator __<<__.
   
*/

ostream& operator<<(ostream& os, const NList& n) { 
  os << n.convertToString(); 
  return os;
}
 
