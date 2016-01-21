/*
----
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, Department of Computer Science,
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
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]

[1] Implementation of the Vector-based Type Map Utilities

\tableofcontents

\noindent

1 Introduction

This file implements utility functions for vector-based type maps.


2 Includes

*/

#include "VectorTypeMapUtils.h"


namespace mappings {

/*
3 Implementation of Local Helper Functions

*/
inline NList toNList(const TypeNameVector& vec)
{
  if (vec.size() == 0)
    return NList();

  NList res(vec[0]);
  for (size_t i = 1; i < vec.size(); ++i)
    res.append(vec[i]);
  return res;
}

inline NList toNListEncloseSingleAtom(const TypeNameVector& vec)
{
  if (vec.size() == 1)
    return NList(vec[0]).enclose();

  return toNList(vec);
}


/*
4 Implementation of Utility Functions

*/
ListExpr vectorTypeMap(const VectorTypeMaps& maps, const ListExpr args)
{
  const NList type(args);
  for (const VectorTypeMap& tm : maps)
    if (type == toNListEncloseSingleAtom(tm.first))
      return toNList(tm.second).listExpr();

  std::ostringstream os;
  os << "Argument types do not match. Expecting";
  if (maps.size() == 1)
    os << "\n";
  else
    os << " one of\n";
  for (const VectorTypeMap& tm : maps)
    os << "  " << toNListEncloseSingleAtom(tm.first) << ",\n";
  os << "but got " << type << ".";

  return NList::typeError(os.str());
}

int vectorSelect(const VectorTypeMaps& maps, const ListExpr args)
{
  const NList type(args);
  for (size_t i = 0; i < maps.size(); ++i)
    if (type == toNListEncloseSingleAtom(maps[i].first))
      return i;
  assert(false);
}

} //-- namespace mappings
