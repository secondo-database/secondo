/*

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
