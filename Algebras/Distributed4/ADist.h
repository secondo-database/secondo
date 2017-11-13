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

[10] Definition of Class ADist

Adaptive Distribution

2017-11-13: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 About this Class

This class provides all the logic for the [secondo] type ~adist~. The
individual member functions, static member functions, and supporting functions
are documented with the implementation, in "ADist.cpp"[1].

The type ~adist~ provides a means to manage a cluster of secondo instances that
use a common ~dpartition~ and associated ~d[f]array~. The ~adist~ is needed
only by a single instance known as the ~governor~. It is responsible for
applying both manual and automatic changes to the distributed structures and
synchronizing them to the other instances of the cluster (peers).

*/
#ifndef ALGEBRAS_DISTRIBUTED4_ADIST_H
#define ALGEBRAS_DISTRIBUTED4_ADIST_H

#include "ConnectionSession.h"
#include "DPartition.h"
#include <vector>
#include <string>
#include <memory>

namespace distributed4 {
  typedef std::vector<std::tuple<std::string,int,std::string>> peers_t;

  class ADist {
    protected:
/*
2 Member Variables

*/
      std::string dpartition_name;
      size_t slotsize;
      peers_t peers;
      std::vector<std::unique_ptr<ConnectionSession>> sessions;
      std::unique_ptr<DPartition> dpartition{readDPartition()};
      std::unique_ptr<distributed2::DArrayBase> darray{readDArray()};
/*
  * "dpartition\_name"[1] is the name of the managed ~dpartition~, which in
    turn manages the partitioning of the underlying ~d[f]array~.

  * "slotsize"[1] holds the maximum number of elements slots should reach
    before being automatically split. This value is also used as a basis for
    how small a slot may become before being considered for automatic merging
    with another slot (e.g.: "slotsize/4"[1]). These are not hard limits. They
    are rather to be understood as a trigger to take action. As such, the slot
    size may go above or below the limit until such action is taken.

  * "peers"[1] holds the information required to establish a connection to
    each peer.

  * "sessions"[1] caches a "ConnectionSession"[1] object for every peer in
    "peers"[1] after first use. This is a state variable with no persistent
    representation.

  * "dpartition"[1] caches the in-memory instance of the ~dpartition~ named in
    "dpartition\_name"[1]. This is a state variable with no persistent
    representation.

  * "darray"[1] caches the in-memory instance of the ~d[f]array~ named in
    "darray\_name"[1]. This is a state variable with no persistent
    representation.

*/
    public:
      ADist(const std::string&, size_t, const peers_t&);
      ADist(const NList&);
      ADist(const ADist&);

      ListExpr listExpr() const;
      void print(std::ostream&) const;
      std::string dpartitionName() const;
      std::string darrayName() const;

      void exec(const std::function<void(const
            std::unique_ptr<ConnectionSession>&)>&);
      void addPeer(const std::string&, int, const std::string&);
      void removePeer(size_t);
      void removePeer(const std::string&, int);
      void addWorker(const std::string&, int, const std::string&);
      void removeWorker(size_t);
      void removeWorker(const std::string&, int);
      void moveSlot(uint32_t, uint32_t);
      void moveSlot(uint32_t, const std::string&, int);
      uint32_t splitSlot(uint32_t);
      uint32_t mergeSlots(uint32_t, uint32_t);

      static std::string BasicType();
      static ListExpr Out(ListExpr, Word);
      static Word In(ListExpr, ListExpr, int, ListExpr&, bool&);
      static bool checkType(const NList&);
      static bool checkType(ListExpr);  // for consistency with other types
      static bool checkType(ListExpr, ListExpr&);  // for "Functions"

      struct Info: ConstructorInfo { Info(); };
      struct Functions: ConstructorFunctions<ADist> { Functions(); };

    protected:
      ADist();
      DPartition* readDPartition();
      distributed2::DArrayBase* readDArray();

    friend Word ConstructorFunctions<ADist>::Create(const ListExpr);
    friend void* ConstructorFunctions<ADist>::Cast(void*);
  };

  std::ostream& operator<<(std::ostream&, const peers_t&);
  std::ostream& operator<<(std::ostream&, const ADist&);
}

#endif
