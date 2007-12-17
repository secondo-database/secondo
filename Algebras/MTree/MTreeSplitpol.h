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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

2.6 Class ~MT::Splitpol~ (file: MTSplitpol.h)

December 2007, Mirko Dibbert

2.6.1 Class description

This class contains all defined promote- and partition-functions. If a node
should be splitted, the promote function select two entries, which should be
used as routing entries in the parent node. Afterwards, the partition
function divides the origin entries to two entry-vectors, whereas each contains
one of the promoted entries.

The partition functions must store the promoted elements as first elements in
the new entry vectors, which is e.g. needed in "MLB[_]PROM"[4] to determine the
entry which is used as routing entry in the parent node.

2.6.2 Class definition

*/
#ifndef SPLITPOL_H
#define SPLITPOL_H

#include <vector>
#include "MTreeEntry.h"
#include "MetricRegistry.h"

namespace MT
{

enum PROMOTE
{ RANDOM, m_RAD, mM_RAD, M_LB_DIST };
/*
Enumeration of the implemented promote functions:

  * "RANDOM"[4] : Promotes two random entries.

  * "m[_]RAD"[4] : Promotes the entries which minimizes the sum of
  both covering radii.

  * "mM[_]RAD:"[4] Promotes the entries which minimizes the maximum of
  both covering radii.

  * "M[_]LB[_]DIST"[4] : Promotes as first entry the previously promoted
  element, which should be equal to the parent entry. As second entry, the one
  with maximum distance to parent would be promoted.

*/

enum PARTITION
{ GENERALIZED_HYPERPLANE, BALANCED };
/*
Enumeration of the implemented partition functions.

Let "p_1"[2], "p_2"[2] be the promoted items and "N_1"[2], "N_2"[2] be the nodes
containing "p_1"[2] and "p_2"[2]:

  * "GENERALIZED[_]HYPERPLANE"[4] The algorithm assign an entry "e"[2] as
  follows: if "d(e,p_1) \leq d(e,p_2)"[2], "e"[2] is assigned to "N_1"[2],
  otherwhise it is assigned to "N_2"[2].

  * "BALANCED"[4] : This algorithm alternately assigns the nearest neighbour
  of "p_1"[2] and "p_2"[2], which has not yet been assigned, to "N_1"[2] and
  "N_2"[2], respectively.

*/

class Splitpol
{

public:
  Splitpol( PROMOTE promId, PARTITION partId, TMetric metric );
/*
Constructor.

*/

  inline void apply( vector<Entry*>* entries,
                     vector<Entry*>* entriesL,
                     vector<Entry*>* entriesR,
                     bool isLeaf )
  {
    m_entries = entries;
    m_entriesL = entriesL;
    m_entriesR = entriesR;
    m_isLeaf = isLeaf;

    (this->*promFun)();
    (this->*partFun)();
    m_promL = new Entry( *((*entries)[ m_promLId ]) );
    m_promR = new Entry( *((*entries)[ m_promRId ]) );
    m_promL->setRad( m_radL );
    m_promR->setRad( m_radR );
  }
/*
This function applies the split policy, which had been selected in the
constructor. As result, the elements of "entries"[4] are splitted to
"entriesL"[4] and "entriesR"[4]. The "isLeaf"[4] flag must be true, if
the splitted node was is leaf.

The respective routing entries could be obtained by the methods "getPromL"[4]
and "getPromR"[4].

*/

  inline Entry* getPromL()
  {
    return m_promL;
  }
/*
Returns the routing entry for left node (distance to parent and
pointer to chield node are not yet set).

*/

  inline Entry* getPromR()
  {
    return m_promR;
  }
/*
Returns the routing entry for right node (distance to parent and
pointer to chield node are not yet set).

*/

private:
  vector<Entry*>* m_entries;  // contains the original entry-vector
  vector<Entry*>* m_entriesL; // return vector for left node
  vector<Entry*>* m_entriesR; // return vector for right node
  Entry* m_promL;             // promoted Entry for left node
  Entry* m_promR;             // promoted Entry for right node
  size_t m_promLId;           // index of the left promoted entry
  size_t m_promRId;           // index of the right promoted entry
  double m_radL, m_radR;      // covering radii of the prom-entries
  bool m_isLeaf;              // true, if the splitted node is a leaf
  double* m_distances;        // array of precmputed distances

  TMetric m_metric;
/*
Contains the selected metric.

*/

  void ( Splitpol::*promFun )();
/*
Contains the selected promote function.

*/

  void ( Splitpol::*partFun )();
/*
Contains the selected partiton function.

*/

struct BalancedPromEntry
{
  Entry* entry;
  double distToL, distToR;

  BalancedPromEntry(
      Entry* entry_, double distToL_, double distToR_ )
  : entry( entry_ ), distToL( distToL_ ), distToR( distToR_ ) {}
};
/*
This struct is used in Balanced[_]Part as entry in the entry-list.

*/

/*
Promote functions:

The following methods promote two objects in the entries list and store their
indizes in "m[_]promL"[4] and "m[_]promR"[4].

*/

  void Rand_Prom();
/*
This method promtes two randomly selected elements.

*/

  void MRad_Prom();
/*
Promotes the entries which minimizes the sum of both covering radii.

*/

  void MMRad_Prom();
/*
Promotes the entries which minimizes the maximum of both covering radii.

*/

  void MLB_Prom();
/*
Promotes as first entry the previously promoted element, which should be equal
to the parent entry. As second entry, the one with maximum distance to parent
would be promoted.

*/

/*
Partition functions:

The following methods splits the entries in "m[_]entries"[4] to "m[_]entriesL"[4]
and "m[_]entriesR"[4].

*/
  void Hyperplane_Part();
/*
Assign an entry e to the entry vector, which has the nearest distance between e
and the respective promoted element.

*/

  void Balanced_Part();
/*
Alternately assigns the nearest neighbour of "m[_]promL"[4] and "m[_]promR"[4],
which has not yet been assigned, to "m[_]entriesL"[4] and "m[_]entriesR"[4],
respectively.

*/

}; // class Splitpol

} // namespace MTree

#endif
