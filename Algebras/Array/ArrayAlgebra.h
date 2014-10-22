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

*/

#ifndef SEC_ARRAYALGEBRA_H
#define SEC_ARRAYALGEBRA_H

#include "NestedList.h"
#include "ListUtils.h"

namespace arrayalgebra{

class Array
{
  public:
    Array(int, int, int, Word*);
    Array();
    ~Array();
    void initialize(int, int, int, Word*);
    bool isDefined();
    int getSize();
    int getElemAlgId();
    int getElemTypeId();
    Word getElement(int);
    void setElement(int, Word);

    static const string BasicType() { return "array"; }
    static const bool checkType(const ListExpr list){
       bool implemented = false;
       assert(implemented);
       return false;
    }

  static Word genericClone( int algebraId,
         int typeId,
         ListExpr typeInfo,
         Word object );
  private:
    bool defined;
    int size;
    int elemAlgId;
    int elemTypeId;
    Word* array;
};

} 

#endif /* SEC_ARRAYALGEBRA_H */
