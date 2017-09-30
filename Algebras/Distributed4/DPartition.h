/*
----
This file is part of SECONDO.

Copyright (C) 2017, Faculty of Mathematics and Computer Science, Database
Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

SECONDO is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
SECONDO; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [10] title: [{\Large \bf] [}]
//characters [1] tt: [\texttt{] [}]
//[secondo] [{\sc Secondo}]

[10] Definition of Class DPartition

2017-08-14: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 About this Class

This class provides all the logic for the [secondo] type ~dpartition~. The
individual methods, static functions, and supporting functions are documented
with the implementation, in "DPartition.cpp"[1].

The type ~dpartition~ provides partitioning of values into slots of a ~darray~
or ~dfarray~. There are two cases that need to be kept apart here. If the
~d[f]array~ manages a distributed relation, partitioning is performed with
respect to a specific attribute of that relation. The attribute and its type
become part of the subtype of ~dpartition~ "(dpartition (Code int))"[1]. If the
~darray~ manages a collection of values (~vector~, ~set~, ~multiset~), just the
type of the values contained in the collection is part of the subtype of
~dpartition~ "(dpartition int)"[1].

*/
#ifndef ALGEBRAS_DISTRIBUTED4_DPARTITION_H
#define ALGEBRAS_DISTRIBUTED4_DPARTITION_H

#include "../Distributed2/DArray.h"

namespace distributed4 {
  class DPartition {
    protected:
/*
2 Member Variables

*/
      std::map<double,uint32_t> partitioning;
      //TODO: for multi-dimensional partitions, use either a vector of vectors
      //(of vectors) or a map<int,int> of maps<int,int> (of maps<int,int>).
      std::string darrayname;
/*
"partitioning"[1] contains all necessary boundary information to correlate any
value with a slot. Each entry maps a lower value boundary to a slot number. The
lowest entry in "partitioning"[1] maps values beginning with negative infinity
to a slot. This entry must always exist.

"darrayname"[1] is the name of the ~d[f]array~ managing the slots of the data
being partitioned.

*/
    public:
      DPartition();
      DPartition(const std::map<double,uint32_t>&, const std::string&);
      DPartition(const NList&);
      uint32_t slot(double) const;
      ListExpr listExpr() const;
      void print(std::ostream&) const;
      static std::string BasicType();
      static bool checkType(ListExpr, ListExpr&);
  };

  std::ostream& operator<<(std::ostream&, const std::map<double,uint32_t>&);
  std::ostream& operator<<(std::ostream&, const DPartition&);
}

#endif
