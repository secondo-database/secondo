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

[10] Definition of Class DTable

2017-08-14: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 About this Class

This class provides all the logic for the [secondo] type ~dtable~. The
individual methods, static functions, and supporting functions are documented
with the implementation, in "DTable.cpp"[1].

The type ~dtable~ provides partitioning of values into slots and allocation
of slots to workers. The allocation of slots to workers mimics the behavior of
~darray~, but is extended by the ability to modify that allocation. The
partitioning of the related data is the real benefit of this type. It
partitions relations according to a specified attribute. This attribute might
contain ~point~, ~ipoint~, ~upoint~, ~real~, ~int~, or any other value that is
comparable in defined dimensions.

*/
#ifndef ALGEBRAS_DISTRIBUTED4_DTABLE_H
#define ALGEBRAS_DISTRIBUTED4_DTABLE_H

#include "DArray.h"

namespace distributed4 {
  class DTable {
    protected:
/*
2 Member Variables

*/
      std::map<double,uint32_t> partitioning;
      //TODO: for multi-dimensional partitions, use either a vector of vectors
      //(of vectors) or a map<int,int> of maps<int,int> (of maps<int,int>).
      std::vector<uint32_t> allocation;
      std::vector<distributed2::DArrayElement> workers;
      std::string slotbasename;
/*
"partitioning"[1] contains all necessary boundary information to correlate any
value with a slot. Each entry maps a lower value boundary to a slot number. Any
value below the lowest boundary value in "partitioning"[1] is somewhere between
negative infinity and that boundary value. It is implicitly mapped to slot 0.
As a result, slot 0 may not be mapped to in "partitioning"[1].

"allocation"[1] represents the allocation of slots to workers. Each entry in
allocation has an index number, which doubles as a slot number. The value at
that index is an index into the "workers"[1] vector indicating which worker the
slot is on.

"workers"[1] is just a alist of workers. Each entry contains all the
information needed by ~Distributed2Algebra~ to manage the connections.

"slotbasename"[1] is the name under which the slots are stored on a worker.
The complete slot name as stored on a worker is put together by the operators
of ~Distributed2Algebra~ as "slotbasename + ["]\_["] + i"[1], where ~i~ is the
slot number being accessed.

*/
    public:
      DTable();
      DTable(const DTable&);
      DTable(const std::map<double,uint32_t>&, const std::vector<uint32_t>&,
          const std::vector<distributed2::DArrayElement>&, const std::string&);
      DTable(const std::map<double,uint32_t>&, const distributed2::DArray&);
      DTable(const NList&);
      ListExpr listExpr();
      void addWorker(const std::string&, int, const std::string&);
      void removeWorker();
      uint32_t slot(double) const;
      distributed2::DArray getDArray() const;
      void print(std::ostream&) const;
      static std::string BasicType();
      static bool checkType(ListExpr, ListExpr&);
  };

  std::ostream& operator<<(std::ostream&, const std::map<double,uint32_t>&);
  template<typename T> std::ostream& operator<<(std::ostream&, const
      std::vector<T>&);
  std::ostream& operator<<(std::ostream&, const DTable&);
}

#endif
