/*
\newpage

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

January-May 2008, Mirko Dibbert

1.1 Overview

This headerfile contains all defined promote- and partition-functions. If a node should be splitted, the promote function select two entries, which should be used as routing entries in the parent node. Afterwards, the partition function divides the origin entries to two entry-vectors, whereas each contains one of the promoted entries.

The partition functions must store the promoted elements as first elements in the new entry vectors, which is e.g. needed in "MLB[_]PROM"[4] to determine the entry which is used as routing entry in the parent node.

1.1 Includes and defines

*/
#ifndef __SPLITPOL_H__
#define __SPLITPOL_H__

#include "MTreeBase.h"

namespace mtreeAlgebra
{


/*
This struct is used in Balanced[_]Part as entry in the entry-list.

*/
template<class TEntry>
struct BalancedPromEntry
{
  TEntry* entry;
  double distToL, distToR;

  BalancedPromEntry(
      TEntry* entry_, double distToL_, double distToR_)
  : entry(entry_), distToL(distToL_), distToR(distToR_) {}
};



class Splitpol; // forward declaration
/*
Class "GenericSplitpol"[4]:

This template class contains the defined promote- and partitions functions and is desinged as template class to avoid typecasts for every access to the nodes.

*/
template<class TNode, class TEntry>
class GenericSplitpol
{
friend class Splitpol;

/*
Constructor.

*/
  GenericSplitpol(PROMOTE promId, PARTITION partId, gta::Distfun metric);

/*
This function applies the split policy, which had been selected in the constructor. After that, the nodes "[_]lhs"[4] and "[_]rhs"[4] contain the entries. The promoted entries are stored in the "promL"[4] and "promR"[4] members, which are accessed from the "Splitpol::apply"[4] method after the split.

*/
  inline void apply(TNode* _lhs, TNode* _rhs,
                    SmiRecordId lhs_id, SmiRecordId rhs_id,
                    bool _isLeaf)
  {
    isLeaf = _isLeaf;
    lhs = _lhs;
    rhs = _rhs;

    // create empty vector and swap it with current entry vector
    entries = new std::vector<TEntry*>();
    entries->swap(*lhs->entries());

    entriesL = lhs->entries();
    entriesR = rhs->entries();

    (this->*promFun)();
    (this->*partFun)();

    promL = new InternalEntry(*((*entries)[promLId]), radL, lhs_id);
    promR = new InternalEntry(*((*entries)[promRId]), radR, rhs_id);
    delete entries;
  }

  std::vector<TEntry*>* entries;  // contains the original entry-vector
  std::vector<TEntry*>* entriesL; // new entry vector for left node
  std::vector<TEntry*>* entriesR; // new entry vector for right node

  unsigned promLId; // index of the left promoted entry
  unsigned promRId; // index of the right promoted entry

  InternalEntry* promL; // promoted Entry for left node
  InternalEntry* promR; // promoted Entry for right node

  double radL, radR; // covering radii of the prom-entries
  double* distances; // array of precmputed distances

  bool isLeaf; // true, if the splitted node is a leaf node

  TNode *lhs, *rhs; // contains the origin and the new node

  gta::Distfun metric; // selected metric.
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
  Splitpol(PROMOTE promId, PARTITION partId, gta::Distfun _metric)
  : internalSplit(promId, partId, _metric),
    leafSplit(promId, partId, _metric)
  {}

/*
This function splits the "lhs"[4] node. "rhs"[4] should be an empty node of the same type.

*/
  inline void apply(gtree::NodePtr lhs, gtree::NodePtr rhs, bool isLeaf)
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
template<class TNode, class TEntry>
GenericSplitpol<TNode, TEntry>::GenericSplitpol(
    PROMOTE promId, PARTITION partId, gta::Distfun _metric)
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
template<class TNode, class TEntry>
void GenericSplitpol<TNode, TEntry>::Rand_Prom()
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
template<class TNode, class TEntry>
void GenericSplitpol<TNode, TEntry>::MRad_Prom()
{
  // precompute distances
  distances = new double[entries->size() * entries->size()];
  for (unsigned i=0; i< entries->size(); i++)
    distances[i*entries->size() + i] = 0;

  for (unsigned i=0; i < (entries->size()-1); i++)
    for (unsigned j=(i+1); j < entries->size(); j++)
    {
      double dist;
      (*metric)((*entries)[i]->data(),
                  (*entries)[j]->data(), dist);
      distances[i*entries->size() + j] = dist;
      distances[j*entries->size() + i] = dist;
    }

  bool first = true;
  double minRadSum;
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
template<class TNode, class TEntry>
void GenericSplitpol<TNode, TEntry>::MMRad_Prom()
{
  // precompute distances
  distances = new double[entries->size() * entries->size()];
  for (unsigned i=0; i< entries->size(); i++)
    distances[i*entries->size() + i] = 0;

  for (unsigned i=0; i < (entries->size()-1); i++)
    for (unsigned j=(i+1); j < entries->size(); j++)
    {
      double dist;
      (*metric)((*entries)[i]->data(),
                  (*entries)[j]->data(), dist);
      distances[i*entries->size() + j] = dist;
      distances[j*entries->size() + i] = dist;
    }

  bool first = true;
  double minMaxRad;
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
        minMaxRad = std::max(radL, radR);
        first = false;
      }
      else
      {
        if (std::max(radL, radR) < minMaxRad)
        {
          minMaxRad = std::max(radL, radR);
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
template<class TNode, class TEntry>
void GenericSplitpol<TNode, TEntry>::MLB_Prom()
{
  #ifdef __MTREE_DEBUG
  assert ((*entries)[0]->dist() == 0);
  #endif

  promLId = 0;
  promRId = 1;
  double maxDistToParent = (*entries)[1]->dist();
  for (unsigned i=2; i < entries->size(); i++)
  {
    double dist = (*entries)[i]->dist();
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
template<class TNode, class TEntry>
void GenericSplitpol<TNode, TEntry>::Hyperplane_Part()
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
      double distL, distR;
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
          radL = std::max(radL, distL);
        else
          radL = std::max(radL, distL + (*entries)[i]->rad());

        entriesL->push_back((*entries)[i]);
        entriesL->back()->setDist(distL);
      }
      else
      {
        if (isLeaf)
          radR = std::max(radR, distR);
        else
          radR = std::max(radR, distR + (*entries)[i]->rad());

        entriesR->push_back((*entries)[i]);
        entriesR->back()->setDist(distR);
      }
    }
  }
}

/*
Method ~BalancedPart~ :

*/
template<class TNode, class TEntry>
void GenericSplitpol<TNode, TEntry>::Balanced_Part()
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
  std::list<BalancedPromEntry<TEntry> > entriesCpy;
  for (size_t i=0; i < entries->size(); i++)
  {
    if ((i != promLId) && (i != promRId))
    {
      double distL, distR;
      if (distances)
      {
          unsigned distArrOffset = i * entries->size();
          distL = distances[distArrOffset + promLId];
          distR = distances[distArrOffset + promRId];
      }
      else
      {
        gta::DistData* data = (*entries)[i]->data();
        gta::DistData* dataL = (*entries)[promLId]->data();
        gta::DistData* dataR = (*entries)[promRId]->data();
        (*metric)(data, dataL, distL);
        (*metric)(data, dataR, distR);
      }

      entriesCpy.push_back(
          BalancedPromEntry<TEntry> (((*entries)[i]), distL, distR));
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
      typename std::list<BalancedPromEntry<TEntry> >::iterator
          nearestPos = entriesCpy.begin();

      typename std::list<BalancedPromEntry<TEntry> >::iterator
          iter = entriesCpy.begin();

      while (iter  != entriesCpy.end())
      {
        if ((*iter).distToL < (*nearestPos).distToL)
        {
          nearestPos = iter;
        }
        iter++;
      }

      double distL = (*nearestPos).distToL;
      if (isLeaf)
        radL = std::max(radL, distL);
      else
        radL = std::max(radL, distL + (*nearestPos).entry->rad());

      entriesL->push_back((*nearestPos).entry);
      entriesL->back()->setDist(distL);
      entriesCpy.erase (nearestPos);
    }
    else
    {
      typename std::list<BalancedPromEntry<TEntry> >::iterator
          nearestPos = entriesCpy.begin();

      typename std::list<BalancedPromEntry<TEntry> >::iterator
          iter = entriesCpy.begin();

      while (iter  != entriesCpy.end())
      {
        if ((*iter).distToL < (*nearestPos).distToR)
        {
          nearestPos = iter;
        }
        iter++;
      }
      double distR = (*nearestPos).distToR;
      if (isLeaf)
        radR = std::max(radR, distR);
      else
        radR = std::max(radR, distR + (*nearestPos).entry->rad());

      entriesR->push_back((*nearestPos).entry);
      entriesR->back()->setDist(distR);
      entriesCpy.erase (nearestPos);
    }
      assignLeft = !assignLeft;
  }
}

} // namespace mtreeAlgebra
#endif // #ifndef __SPLITPOL_H__
