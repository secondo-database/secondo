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

July 2004 M. Spiekermann. 

This additonal header file was introduced to separate the memory-based and
Berkeley-DB record based implementations of class CTable more cleanly. Some
Secondo code includes CTable.h and NestedList.h simultaneously but the
interface is not completeley the same. Moreover it is a template class and its
code is generated at compile time. To avoid strange effects this include file
renames the class CTable to MemCTable. 

Hence when you want to use a vector based implementation of class CTable please
use class MemCTable. Class CTable is preserved for internal use inside class
NestedList.

*/


#ifndef MCTABLE_H
#define MCTABLE_H

#ifdef CTABLE_PERSISTENT
#undef CTABLE_PERSISTENT
#endif

#undef CTABLE_H
#define CTable MemCTable
#include "CTable.h"
#undef CTable
#undef CTABLE_H

#endif
