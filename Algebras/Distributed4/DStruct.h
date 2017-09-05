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

[10] Class Template DStruct

2017-08-14: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 About this Template

This class template provides all the logic for the [secondo] type ~dstruct~.
Even though the actual implementation is in "DStruct.ipp"[1], its function is
completely documented here. Comments in the implementation file explain
implementation details.

The type "DStruct"[1] is realized as a class template because it is used to
structure data according to some value type. This value type can vary in the
number of dimensions (~point~/~ipoint~) as well as in its representation of
values (~real~/~int~ or ~ipoint~/~upoint~).

Most importantly, the data is partitioned into slots by grouping values based
on their position in their specific space. The templated type ~T~ is what makes
that possible.

2 Preliminary Setup

3 Operators Depended on by DStruct

"<< map<A,B>"[1] allows pushing the pairs in a map to a character stream in an
understandable way.

"<< vector<T>"[1] allows pushing the values in a vector to a character stream.

4 Member Variables

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

"slotbasename"[1] is the name under which the slots are stored on a worker. The
complete slot name as stored on a worker is put together by the operators of
~Distributed2Algebra~ as "slotbasename + ["]\_["] + i"[1], where ~i~ is the slot
number being accessed.

5 Constructors

Default Constructor. This constructor creates a completely empty (and therefore
unusable) instance. Its only use is the ~cast~ function.

Copy Constructor. The contents of the member variables of the passed
"DStruct"[1] are copied over.

Copy Constructor. The passed "map"[1], "vector"[1]s and "string"[1] are copied
to the respective member variables of the newly constructed "DStruct"[1].

Copy Constructor. The passed "map"[1] and the member variables of the passed
"DArray"[][1] are copied over.

6 Member Methods

TODO

For easier debugging and/or output of the contents of a DStruct, this method
will push all details to a passed ostream. This makes it usable by
"operator<<"[1], which is also defined.

7 Static Methods

"BasicType"[1] is required by [secondo]. It returns [secondo]'s basic type for
this class.

"checkType"[1] is required by [secondo] as well. It checks the passed ListExpr
to make sure that it is a valid type description for this class. It doesn't
only check the main type (~dstruct~) but the complete type expression.

8 Operators Depending on DStruct

"operator<<"[1] allows something along the lines of the following:

---- DStruct ds;
     cout << "debug: " << ds;
----

It uses the member method "print"[1].

9 Implementation

As this is a class template, it is necessary to include the implementation in
the header file. In order to keep the code base as easy to understand as
possible, the implementation is kept in a separate file and included at this
point.

*/
#ifndef ALGEBRAS_DISTRIBUTED4_DSTRUCT_H
#define ALGEBRAS_DISTRIBUTED4_DSTRUCT_H

#include "DArray.h"

namespace distributed4 {
  class DStruct {
    protected:
      std::map<double,uint32_t> partitioning;
      //TODO: for multi-dimensional partitions, use either a vector of vectors
      //(of vectors) or a map<int,int> of maps<int,int> (of maps<int,int>).
      std::vector<uint32_t> allocation;
      std::vector<distributed2::DArrayElement> workers;
      std::string slotbasename;

    public:
      DStruct();
      DStruct(const DStruct&);
      DStruct(const std::map<double,uint32_t>&, const std::vector<uint32_t>&,
          const std::vector<distributed2::DArrayElement>&, const std::string&);
      DStruct(const std::map<double,uint32_t>&, const distributed2::DArray&);
      DStruct(ListExpr);
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
  std::ostream& operator<<(std::ostream&, const DStruct&);
}

#endif
