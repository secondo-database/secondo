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

1 Headerfile "MTreeSplitpol.h"[4]

January-February 2008, Mirko Dibbert

1.1 Overview

This headerfile contains all defined promote- and partition-functions. If a node should be splitted, the promote function select two entries, which should be used as routing entries in the parent node. Afterwards, the partition function divides the origin entries to two entry-vectors, whereas each contains one of the promoted entries.

The partition functions must store the promoted elements as first elements in the new entry vectors, which is e.g. needed in "MLB[_]PROM"[4] to determine the entry which is used as routing entry in the parent node.

1.1 Includes and defines

*/
#ifndef SPLITPOL_H
#define SPLITPOL_H

#include <vector>
#include "MTreeBase.h"

namespace mtreeAlgebra {

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

/*
This struct is used in Balanced[_]Part as entry in the entry-list.

*/
template<class EntryT>
struct BalancedPromEntry
{
  EntryT* entry;
  DFUN_RESULT distToL, distToR;

  BalancedPromEntry(
      EntryT* entry_, DFUN_RESULT distToL_, DFUN_RESULT distToR_)
  : entry(entry_), distToL(distToL_), distToR(distToR_) {}
};



class Splitpol; // forwar declaration
/*
Class "GenericSplitpol"[4]:

This template class contains the defined promote- and partitions functions and is desinged as template class to avoid typecasts for every access to the nodes.

*/
template<class NodeT, class EntryT>
class GenericSplitpol
{
friend class Splitpol;

/*
Constructor.

*/
  GenericSplitpol(PROMOTE promId, PARTITION partId, Distfun metric);

/*
This function applies the split policy, which had been selected in the constructor. After that, the nodes "[_]lhs"[4] and "[_]rhs"[4] contain the entries. The promoted entries are stored in the "promL"[4] and "promR"[4] members, which are accessed from the "Splitpol::apply"[4] method after the split.

*/
  inline void apply(SmartPtr<NodeT> _lhs, SmartPtr<NodeT> _rhs,
                    SmiRecordId lhs_id, SmiRecordId rhs_id,
                    bool _isLeaf)
  {
    isLeaf = _isLeaf;
    lhs = _lhs;
    rhs = _rhs;

    // create empty vector and swap it with current entry vector
    entries = new vector<EntryT*>();
    entries->swap(*lhs->entries());

    entriesL = lhs->entries();
    entriesR = rhs->entries();

    (this->*promFun)();
    (this->*partFun)();

    promL = new InternalEntry(*((*entries)[promLId]), radL, lhs_id);
    promR = new InternalEntry(*((*entries)[promRId]), radR, rhs_id);
    delete entries;
  }

  vector<EntryT*>* entries;  // contains the original entry-vector
  vector<EntryT*>* entriesL; // new entry vector for left node
  vector<EntryT*>* entriesR; // new entry vector for right node

  unsigned promLId; // index of the left promoted entry
  unsigned promRId; // index of the right promoted entry

  InternalEntry* promL; // promoted Entry for left node
  InternalEntry* promR; // promoted Entry for right node

  DFUN_RESULT radL, radR; // covering radii of the prom-entries
  DFUN_RESULT* distances; // array of precmputed distances

  bool isLeaf; // true, if the splitted node is a leaf node

  SmartPtr<NodeT> lhs, rhs; // contains the origin and the new node

  Distfun metric; // selected metric.
  void (GenericSplitpol::*promFun)(); // selected promote function
  void (GenericSplitpol::*partFun)(); // selected partition function

/*
Promote functions:

The following methods promote two objects in the entries list and store their indizes in "m[_]promL"[4] and "m[_]promR"[4].

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
Promotes as first entry the previously promoted element, which should be equal to the parent entry. As second entry, the one with maximum distance to parent would be promoted.

*/

/*
Partition functions:

The following methods splits the entries in "m[_]entries"[4] to "m[_]entriesL"[4] and "m[_]entriesR"[4].

*/
  void Hyperplane_Part();
/*
Assign an entry e to the entry vector, which has the nearest distance between e and the respective promoted element.

*/

  void Balanced_Part();
/*
Alternately assigns the nearest neighbour of "m[_]promL"[4] and "m[_]promR"[4], which has not yet been assigned, to "m[_]entriesL"[4] and "m[_]entriesR"[4], respectively.

*/

}; // class Splitpol



/********************************************************************
1.1 Class "Splitpol"[4]

********************************************************************/
class Splitpol
{

public:
/*
Constructor.

*/
  Splitpol(PROMOTE promId, PARTITION partId, Distfun _metric)
  : internalSplit(promId, partId, _metric),
    leafSplit(promId, partId, _metric)
  {}

/*
This function splits the "lhs"[4] node. "rhs"[4] should be an empty node of the same type.

*/
  inline void apply(NodePtr lhs, NodePtr rhs, bool isLeaf)
  {
    if (isLeaf)
    {
      leafSplit.apply(lhs->cast<LeafNode>(), rhs->cast<LeafNode>(),
                      lhs->getNodeId(), rhs->getNodeId(), isLeaf);
      promL = leafSplit.promL;
      promR = leafSplit.promR;
    }
    else
    {
      internalSplit.apply(lhs->cast<InternalNode>(), rhs->cast<InternalNode>(),
                      lhs->getNodeId(), rhs->getNodeId(), isLeaf);
      promL = internalSplit.promL;
      promR = internalSplit.promR;
    }
  }

  inline InternalEntry* getPromL()
  { return promL; }

/*
Returns the routing entry for left node (distance to parent and pointer to chield node needs to be set in from the caller).

*/

  inline InternalEntry* getPromR()
  { return promR; }
/*
Returns the routing entry for right node (distance to parent and pointer to chield node needs to be set in from the caller).

*/

private:
  GenericSplitpol<InternalNode, InternalEntry> internalSplit;
  GenericSplitpol<LeafNode, LeafEntry> leafSplit;
  InternalEntry* promL; // promoted Entry for left node
  InternalEntry* promR; // promoted Entry for right node

}; // class Splitpol



/********************************************************************
1.1 Implementation part for "GenericSplitpol"[4] methods

********************************************************************/

/*
GenericSplitpol Constructor:

*/
template<class NodeT, class EntryT>
GenericSplitpol<NodeT, EntryT>::GenericSplitpol(
    PROMOTE promId, PARTITION partId, Distfun _metric)
: distances(0)
{
  metric = _metric;
  srand(time(0)); // needed for Rand_Prom

  // init promote function
  switch (promId)
  {
    case RANDOM:
      promFun = &GenericSplitpol::Rand_Prom;
      break;

    case m_RAD:
      promFun = &GenericSplitpol::MRad_Prom;
      break;

    case mM_RAD:
      promFun = &GenericSplitpol::MMRad_Prom;
      break;

    case M_LB_DIST:
      promFun = &GenericSplitpol::MLB_Prom;
      break;
  }

  // init partition function
  switch (partId)
  {
    case GENERALIZED_HYPERPLANE:
      partFun = &GenericSplitpol::Hyperplane_Part;
      break;

    case BALANCED:
      partFun = &GenericSplitpol::Balanced_Part;
      break;
  }
}

/*
Method ~RandProm~ :

*/
template<class NodeT, class EntryT>
void GenericSplitpol<NodeT, EntryT>::Rand_Prom()
{
  unsigned pos1 = rand() % entries->size();
  unsigned pos2 = rand() % entries->size();
  if (pos1 == pos2)
  {
    if (pos1 == 0)
      pos1++;
    else
      pos1--;
  }

  promLId = pos1;
  promRId = pos2;
}

/*
Method ~MRad[_]Prom~ :

*/
template<class NodeT, class EntryT>
void GenericSplitpol<NodeT, EntryT>::MRad_Prom()
{
  // precompute distances
  distances = new DFUN_RESULT[entries->size() * entries->size()];
  for (unsigned i=0; i< entries->size(); i++)
    distances[i*entries->size() + i] = 0;

  for (unsigned i=0; i < (entries->size()-1); i++)
    for (unsigned j=(i+1); j < entries->size(); j++)
    {
      DFUN_RESULT dist;
      (*metric)((*entries)[i]->data(),
                  (*entries)[j]->data(), dist);
      distances[i*entries->size() + j] = dist;
      distances[j*entries->size() + i] = dist;
    }

  bool first = true;
  DFUN_RESULT minRadSum;
  unsigned bestPromLId = 0;
  unsigned bestPromRId = 1;

  for (unsigned i=0; i < (entries->size()-1); i++)
    for (unsigned j=(i+1); j < entries->size(); j++)
    {
      // call partition function with promoted elements i and j
      promLId = i;
      promRId = j;
      (this->*partFun)();

      if (first)
      {
        minRadSum = (radL + radR);
        first = false;
      }
      else
      {
        if ((radL + radR) < minRadSum)
        {
          minRadSum = (radL + radR);
          bestPromLId = i;
          bestPromRId = j;
        }
      }
    }

  promLId = bestPromLId;
  promRId = bestPromRId;

  // remove array of precomputed distances
  delete[] distances;
  distances = 0;
}

/*
Method ~MMRadProm~ :

*/
template<class NodeT, class EntryT>
void GenericSplitpol<NodeT, EntryT>::MMRad_Prom()
{
  // precompute distances
  distances = new DFUN_RESULT[entries->size() * entries->size()];
  for (unsigned i=0; i< entries->size(); i++)
    distances[i*entries->size() + i] = 0;

  for (unsigned i=0; i < (entries->size()-1); i++)
    for (unsigned j=(i+1); j < entries->size(); j++)
    {
      DFUN_RESULT dist;
      (*metric)((*entries)[i]->data(),
                  (*entries)[j]->data(), dist);
      distances[i*entries->size() + j] = dist;
      distances[j*entries->size() + i] = dist;
    }

  bool first = true;
  DFUN_RESULT minMaxRad;
  unsigned bestPromLId = 0;
  unsigned bestPromRId = 1;

  for (unsigned i=0; i < (entries->size()-1); i++)
    for (unsigned j=(i+1); j < entries->size(); j++)
    {
      // call partition function with promoted elements i and j
      promLId = i;
      promRId = j;
      (this->*partFun)();

      if (first)
      {
        minMaxRad = max(radL, radR);
        first = false;
      }
      else
      {
        if (max(radL, radR) < minMaxRad)
        {
          minMaxRad = max(radL, radR);
          bestPromLId = i;
          bestPromRId = j;
        }
      }
    }

  promLId = bestPromLId;
  promRId = bestPromRId;

  // remove array of precomputed distances
  delete[] distances;
  distances = 0;
}

/*
Method ~MLBProm~ :

*/
template<class NodeT, class EntryT>
void GenericSplitpol<NodeT, EntryT>::MLB_Prom()
{
  #ifdef MTREE_DEBUG
  assert ((*entries)[0]->dist() == 0);
  #endif

  promLId = 0;
  promRId = 1;
  DFUN_RESULT maxDistToParent = (*entries)[1]->dist();
  for (unsigned i=2; i < entries->size(); i++)
  {
    DFUN_RESULT dist = (*entries)[i]->dist();
    if (dist > maxDistToParent)
    {
      maxDistToParent = dist;
      promRId = i;
    }
  }
}

/*
Method ~HyperplanePart~ :

*/
template<class NodeT, class EntryT>
void GenericSplitpol<NodeT, EntryT>::Hyperplane_Part()
{
  entriesL->clear();
  entriesR->clear();

  entriesL->push_back((*entries)[promLId]);
  entriesR->push_back((*entries)[promRId]);

  (*entries)[promLId]->setDist(0);
  (*entries)[promRId]->setDist(0);

  radL = (*entries)[promLId]->rad();
  radR = (*entries)[promRId]->rad();

  for (size_t i=0; i < entries->size(); i++)
  {
    if ((i != promLId) && (i != promRId))
    {
      // determine distances to promoted elements
      DFUN_RESULT distL, distR;
      if (distances)
      {
          unsigned distArrOffset = i * entries->size();
          distL = distances[distArrOffset + promLId];
          distR = distances[distArrOffset + promRId];
      }
      else
      {
        (*metric)(((*entries)[i])->data(),
                     ((*entries)[promLId])->data(), distL);
        (*metric)(((*entries)[i])->data(),
                     ((*entries)[promRId])->data(), distR);
      }

      /* push entry i to list with nearest promoted entry and update
         distance to parent and covering radius */
      if (distL < distR)
      {
        if (isLeaf)
          radL = max(radL, distL);
        else
          radL = max(radL, distL + (*entries)[i]->rad());

        entriesL->push_back((*entries)[i]);
        entriesL->back()->setDist(distL);
      }
      else
      {
        if (isLeaf)
          radR = max(radR, distR);
        else
          radR = max(radR, distR + (*entries)[i]->rad());

        entriesR->push_back((*entries)[i]);
        entriesR->back()->setDist(distR);
      }
    }
  }
}

/*
Method ~BalancedPart~ :

*/
template<class NodeT, class EntryT>
void GenericSplitpol<NodeT, EntryT>::Balanced_Part()
{
  entriesL->clear();
  entriesR->clear();

  entriesL->push_back((*entries)[promLId]);
  entriesR->push_back((*entries)[promRId]);

  (*entries)[promLId]->setDist(0);
  (*entries)[promRId]->setDist(0);

  radL = (*entries)[promLId]->rad();
  radR = (*entries)[promRId]->rad();

  /* copy entries into entries (the list contains the entries
     together with its distances to the promoted elements */
  list<BalancedPromEntry<EntryT> > entriesCpy;
  for (size_t i=0; i < entries->size(); i++)
  {
    if ((i != promLId) && (i != promRId))
    {
      DFUN_RESULT distL, distR;
      if (distances)
      {
          unsigned distArrOffset = i * entries->size();
          distL = distances[distArrOffset + promLId];
          distR = distances[distArrOffset + promRId];
      }
      else
      {
        DistData* data = (*entries)[i]->data();
        DistData* dataL = (*entries)[promLId]->data();
        DistData* dataR = (*entries)[promRId]->data();
        (*metric)(data, dataL, distL);
        (*metric)(data, dataR, distR);
      }

      entriesCpy.push_back(
          BalancedPromEntry<EntryT> (((*entries)[i]), distL, distR));
    }
  }

  /* Alternately assign the nearest neighbour of promL resp.
     promR to entriesL resp. entriesR and remove it from
     entries. */
  bool assignLeft = true;
  while (!entriesCpy.empty())
  {
    if (assignLeft)
    {
      typename list<BalancedPromEntry<EntryT> >::iterator
          nearestPos = entriesCpy.begin();

      typename list<BalancedPromEntry<EntryT> >::iterator
          iter = entriesCpy.begin();

      while (iter  != entriesCpy.end())
      {
        if ((*iter).distToL < (*nearestPos).distToL)
        {
          nearestPos = iter;
        }
        iter++;
      }

      DFUN_RESULT distL = (*nearestPos).distToL;
      if (isLeaf)
        radL = max(radL, distL);
      else
        radL = max(radL, distL + (*nearestPos).entry->rad());

      entriesL->push_back((*nearestPos).entry);
      entriesL->back()->setDist(distL);
      entriesCpy.erase (nearestPos);
    }
    else
    {
      typename list<BalancedPromEntry<EntryT> >::iterator
          nearestPos = entriesCpy.begin();

      typename list<BalancedPromEntry<EntryT> >::iterator
          iter = entriesCpy.begin();

      while (iter  != entriesCpy.end())
      {
        if ((*iter).distToL < (*nearestPos).distToR)
        {
          nearestPos = iter;
        }
        iter++;
      }
      DFUN_RESULT distR = (*nearestPos).distToR;
      if (isLeaf)
        radR = max(radR, distR);
      else
        radR = max(radR, distR + (*nearestPos).entry->rad());

      entriesR->push_back((*nearestPos).entry);
      entriesR->back()->setDist(distR);
      entriesCpy.erase (nearestPos);
    }
      assignLeft = !assignLeft;
  }
}

} // namespace mtreeAlgebra
#endif
