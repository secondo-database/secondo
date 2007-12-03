/*
//[_] [\_]
//characters      [2]   formula:    [$]   [$]

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

2.6 Class ~Splitpol~

December 2007, Mirko Dibbert

2.6.1 Class description

TODO enter class description

2.6.2 Definition part (file: MTSplitpol.h)

*/
#ifndef SPLITPOL_H
#define SPLITPOL_H

#include "MTNode.h"

namespace MT
{

enum PROMOTE
{ RANDOM, m_RAD, mM_RAD, M_LB_DIST };
/*
Enumeration of the implemented promote functions:

  * "RANDOM"[4] : This Algorithm promotes two random entries.

  * "m[_]RAD"[4] : TODO

  * "mM[_]RAD:"[4] TODO

  * "M[_]LB[_]DIST"[4] : TODO

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
  vector<Entry*>* m_entries;  // contains the original entry-vector
  vector<Entry*>* m_entriesL; // return vector for left node
  vector<Entry*>* m_entriesR; // return vector for right node
  Entry* m_promL;             // promoted Entry for left node
  Entry* m_promR;             // promoted Entry for right node
  size_t m_promLId;           // index of the left promoted entry
  size_t m_promRId;           // index of the right promoted entry
  double m_radL, m_radR;      // covering radii of the prom-entries
  bool m_isLeaf;              // true, if the splitted node is a leaf

  vector< vector<double> > m_distances;
  bool m_distancesDefined;

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

    m_distancesDefined = false;

    (this->*promFun)();
    (this->*partFun)();
    m_promL = new Entry( *((*entries)[ m_promLId ]) );
    m_promR = new Entry( *((*entries)[ m_promRId ]) );
    m_promL->setRad( m_radL );
    m_promR->setRad( m_radR );
  }

/*
This function applies the split policy, which had been selected in the
constructor, to the entry lists. As result there are two lists created in
the supp object, which could be obtained by the respective methods.

*/

  inline Entry* getPromL()
  {
    return m_promL;
  }

  inline Entry* getPromR()
  {
    return m_promR;
  }

private:
/*
Promote functions:

The following methods promote two objects in the entries list and return them in
"promL"[4] and "promR"[4]. The promoted entries will be deleted from entries
list and returned in "promL"[4] and "promR"[4].

*/

  void Rand_Prom();
/*
This method promtes two randomly selected elements.

*/

  void MRad_Prom();
/*
TODO : enter method description

*/

  void MMRad_Prom();
/*
TODO : enter method description

*/

  void MLB_Prom();
/*
TODO : enter method description

*/

/*
Partition functions:

The following methods divide the entries of the first list into two lists, which
would be returned in "entries1"[4] and "entries2"[4]. The covering radii of the
new lists will be returned in rad1 and rad2, respectively.


*/
  void Hyperplane_Part();
/*
TODO : enter method description

*/

  void Balanced_Part();
/*
TODO : enter method description

*/

}; // class Splitpol

} // namespace MTree

#endif
