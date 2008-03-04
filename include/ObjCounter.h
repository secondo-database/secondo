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

//paragraph [11] Title: [{\Large \bf \begin{center}] [\end{center}}]
//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

[11] Headerfile "ObjCounter.h"[4]

January-February 2008, Mirko Dibbert

1 Overview

This headerfile contains the "ObjCntr"[4] class, which could be used as base class for all classes, that should be equipped with a counter for open objects, which is useful for debugging purposes to locate memory leaks in dynamic data structures. The counter will be automatically in-/decreased on object construction/destruction.

Since each counter needs a unique id, it is recommended to define a unique constant for this purpose in this file to avoid undesignedly multiple use of the same counter.

The counter must be enabled by defining

---- #define ENABLE_OBJECT_COUNT
----
before including this file. If not defined, the "openObjects"[4] method will allways return 0 and no counter variable will be defined.
\newpage

Usage:

---- class MyClass : public ObjCounter<id>;
----
equippes class "MyClass"[4] with an object counter, wich could be accessed by

---- ObjCounter<id>::openObjects()
----
An example can be found in the general tree algebra framework (file "GTAF.h"[4]).

2 Defines

*/
#ifndef __OBJECT_COUNTER_H
#define __OBJECT_COUNTER_H

// constants for counter id's
const unsigned generalTrees       = 1000;
const unsigned generalTreeNodes   = 1001;
const unsigned generalTreeEntries = 1002;

/*
3 Class "ObjCntr"[4]

*/
template<unsigned counterId>
class ObjCounter
{
public:
  static inline unsigned openObjects()
  {
    #ifdef ENABLE_OBJECT_COUNT
    return m_objectsOpen;
    #else
    return 0;
    #endif
  }
/*
Returns the count of open objects.

*/

protected:
  inline ObjCounter()
  {
    #ifdef ENABLE_OBJECT_COUNT
    ++m_objectsOpen;
    #endif
  }
/*
Default constructor.

*/

  inline ObjCounter(const ObjCounter& e)
  {
    #ifdef ENABLE_OBJECT_COUNT
    ++m_objectsOpen;
    #endif
  }
/*
Default copy constructor.

*/

  inline ~ObjCounter()
  {
    #ifdef ENABLE_OBJECT_COUNT
    --m_objectsOpen;
    #endif
  }
/*
Destructor.

*/

private:
  #ifdef ENABLE_OBJECT_COUNT
  static unsigned m_objectsOpen; // count of open entry objects
  #endif
};

#ifdef ENABLE_OBJECT_COUNT
template<unsigned counterId>
unsigned ObjCounter<counterId>::m_objectsOpen = 0;
#endif

#endif // #ifndef __OBJECT_COUNTER_H
