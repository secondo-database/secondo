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

January-March 2008, Mirko Dibbert

1 Implementation file "MTree.cpp"[4]

January-February 2008, Mirko Dibbert

1.1 Overview

This file contains the implementation of the "MTree"[4] class.

*/
#include <stack>
#include "MTree.h"

using namespace mtreeAlgebra;

/*
Function ~nearlyEqual~:

This auxiliary function is used in the search methods of the mtree class and returns true, if both numbers are "infiniy"[4] or nearly equal.

*/
template <typename FloatType>
inline bool nearlyEqual(FloatType a, FloatType b)
{
    FloatType infinity = numeric_limits<FloatType>::infinity();
    if (a == infinity)
        return (b == infinity);
    else if (b == infinity)
        return false;

    const FloatType scale = max(fabs(a), fabs(b));
    return  fabs(a - b) <=
            scale * 3 * numeric_limits<FloatType>::epsilon();
}

/*
Default Constructor:

*/
MTree::MTree(bool temporary) :
        gtaf::Tree<Header>(temporary), splitpol(false)
{}

/*
Constructor (load m-tree):

*/
MTree::MTree(const SmiFileId fileId) :
        gtaf::Tree<Header>(fileId), splitpol(false)
{
    if (header.initialized)
    {
        initialize();
        registerNodePrototypes();
    }
}

/*
Copy constructor:

*/
MTree::MTree(const MTree& mtree) :
        gtaf::Tree<Header>(mtree), splitpol(false)
{
    if (mtree.isInitialized())
        initialize();
}

/*
Method ~registerNodePrototypes~:

*/
void
MTree::registerNodePrototypes()
{
    // add internal node prototype
    addNodePrototype(new InternalNode(
        new NodeConfig(config.internalNodeConfig)));

    // add leaf node prototype
    addNodePrototype(new LeafNode(
        new NodeConfig(config.leafNodeConfig)));
}

/*
Method ~initialize~:

*/
void
MTree::initialize()
{
    // init DistfunInfo object
    df_info = DistfunReg::getInfo(header.distfunName, header.dataId);

    // init MTreeConfig object
    config = MTreeConfigReg::getConfig(header.configName);

    // init Splitpol object
    splitpol = new Splitpol(
        config.promoteFun, config.partitionFun, df_info.distfun());

    if (nodeCacheEnabled)
        treeMngr->enableCache();
}

/*
Method ~initialize~ :

*/
void
MTree::initialize(DistDataId dataId, const string& distfunName,
                  const string& configName)
{
    if (isInitialized())
        return;

    // copy values to header
    header.dataId = dataId;
    strcpy(header.distfunName, distfunName.c_str());
    strcpy(header.configName, configName.c_str());

    initialize();
    header.initialized = true;

    registerNodePrototypes();

    //create root node
    NodePtr root(createLeaf(Leaf));
    header.root = root->getNodeId();
    ++header.leafCount;
    ++header.height;
}

/*
Method ~split~ :

*/
void MTree::split()
{
    bool done = false;
    while (!done)
    {
        // create new node on current level
        NodePtr newNode(createNeighbourNode(
            treeMngr->curNode()->isLeaf() ? Leaf : Internal));

        // update node count, split node
        if (treeMngr->curNode()->isLeaf())
        {
            ++header.leafCount;
            splitpol->apply(treeMngr->curNode(), newNode, true);
        }
        else
        {
            ++header.internalCount;
            splitpol->apply(treeMngr->curNode(), newNode, false);
        }

        #ifdef MTREE_PRINT_SPLIT_INFO
        cmsg.info() << "\nsplit: splitted nodes contain "
                    << treeMngr->curNode()->entryCount() << " / "
                    << newNode->entryCount() << " entries." << endl;
        cmsg.send();
        #endif

        // set modified flag to true and recompute node size
        treeMngr->recomputeSize(treeMngr->curNode());
        treeMngr->recomputeSize(newNode);

        // retrieve promote entries
        InternalEntry* promL = splitpol->getPromL();
        InternalEntry* promR = splitpol->getPromR();

        // insert promoted entries into routing nodes
        if (treeMngr->hasParent())
        {
            treeMngr->replaceParentEntry(promL);

            // insert promR
            if (!treeMngr->insert(treeMngr->parentNode(), promR))
                done = true;

            treeMngr->getParent();
            if (treeMngr->hasParent())
            {
                // update dist from promoted entries to their parents
                DFUN_RESULT distL, distR;
                DistData* data =
                    treeMngr->parentEntry<InternalNode>()->data();
                df_info.dist(promL->data(), data, distL);
                df_info.dist(promR->data(), data, distR);
                promL->setDist(distL);
                promR->setDist(distR);
            }
        }
        else
        {   // insert new root
            NodePtr newRoot(createRoot(Internal));
            ++header.height;
            ++header.internalCount;
            treeMngr->insert(newRoot, promL);
            treeMngr->insert(newRoot, promR);
            header.root = newRoot->getNodeId();
            done = true;
        }
    } // while
}

/*
Method ~intert~ ("Attribute"[4] objects):

*/
void
MTree::insert(Attribute* attr, TupleId tupleId)
{
    // create new leaf entry
    LeafEntry* entry;
    try
    {
        entry = new LeafEntry(tupleId, df_info.getData(attr));
    }
    catch (bad_alloc&)
    {
        cmsg.warning() << "Not enough memory to create new entry, "
                        << "disabling node cache... "
                        << endl;
        cmsg.send();
        treeMngr->disableCache();

        try
        {
            entry = new LeafEntry(tupleId, df_info.getData(attr));
        }
        catch (bad_alloc&)
        {
            cmsg.error() << "Not enough memory to create new entry!"
                        << endl;
            cmsg.send();
        }
    }
    insert(entry, tupleId);
}

/*
Method ~insert~ ("DistData"[4] objects):

*/
void
MTree::insert(DistData* data, TupleId tupleId)
{
    // create new leaf entry
    LeafEntry* entry;
    try
    {
        entry = new LeafEntry(tupleId, data);
    }
    catch (bad_alloc&)
    {
        cmsg.warning() << "Not enough memory to create new entry, "
                        << "disabling node cache... "
                        << endl;
        cmsg.send();
        treeMngr->disableCache();

        try
        {
            entry = new LeafEntry(tupleId, data);
        }
        catch (bad_alloc&)
        {
            cmsg.error() << "Not enough memory to create new entry!"
                        << endl;
            cmsg.send();
        }
    }
    insert(entry, tupleId);
}

/*
Method ~insert~ ("LeafEntry"[4] objects):

*/
void MTree::insert(LeafEntry* entry, TupleId tupleId)
{
    #ifdef MTREE_DEBUG
    assert(isInitialized());
    #endif

    #ifdef MTREE_PRINT_INSERT_INFO
    if ((header.entryCount % insertInfoInterval) == 0)
    {
        const string clearline = "\r" + string(70, ' ') + "\r";
        cmsg.info() << clearline
                    << "entries: " << header.entryCount
                    << ", routing/leaf nodes: "
                    << header.internalCount << "/"
                    << header.leafCount;
        if(nodeCacheEnabled)
        {
            cmsg.info() << ", cache used: "
                        << treeMngr->cacheSize()/1024 << " kb";
        }
        cmsg.send();
    }
    #endif

    // init path
    treeMngr->initPath(header.root, header.height-1);

    // descent tree until leaf level
    while (!treeMngr->curNode()->isLeaf())
    { /* find best path (follow the entry with the nearest dist to
         new entry or the smallest covering radius increase) */
        list<SearchBestPathEntry> entriesIn;
        list<SearchBestPathEntry> entriesOut;

        InternalNodePtr node =
            treeMngr->curNode()->cast<InternalNode>();

        for(unsigned i=0; i<node->entryCount(); ++i)
        {
            DFUN_RESULT dist;
            df_info.dist(node->entry(i)->data(), entry->data(), dist);
            if (dist <= node->entry(i)->rad())
            {
                entriesIn.push_back(
                    SearchBestPathEntry(node->entry(i), dist, i));
            }
            else
            {
                entriesOut.push_back(
                    SearchBestPathEntry(node->entry(i), dist, i));
            }
        }
        list<SearchBestPathEntry>::iterator best;

        if (!entriesIn.empty())
        { // select entry with nearest dist to new entry
            // (covering radius must not be increased)
            best = entriesIn.begin();
            list<SearchBestPathEntry>::iterator it;
            for (it = entriesIn.begin(); it != entriesIn.end(); ++it)
            {
                if (it->dist < best->dist)
                    best = it;
            }
        }
        else
        { // select entry with minimal radius increase
            DFUN_RESULT dist;
            df_info.dist(entriesOut.front().entry->data(),
                    entry->data(), dist);
            DFUN_RESULT minIncrease =
                    dist - entriesOut.front().entry->rad();
            DFUN_RESULT minDist = dist;

            best = entriesOut.begin();
            list<SearchBestPathEntry>::iterator it;
            for (it = entriesIn.begin(); it != entriesIn.end(); ++it)
            {
                df_info.dist(it->entry->data(), entry->data(), dist);
                DFUN_RESULT increase = dist - it->entry->rad();
                if (increase < minIncrease)
                {
                    minIncrease = increase;
                    best = it;
                    minDist = dist;
                }
            }

            // update increased covering radius
            best->entry->setRad(minDist);
            node->setModified();
        }
        treeMngr->getChield(best->index);
    }

    //   compute distance from entry to parent node, if exist
    if (treeMngr->hasParent())
    {
        DFUN_RESULT dist;
        df_info.dist(entry->data(),
                treeMngr->parentEntry<InternalNode>()->data(), dist);
        entry->setDist(dist);
    }

    // insert entry into leaf, split if neccesary
    if (treeMngr->insert(treeMngr->curNode(), entry))
        split();

    ++header.entryCount;
}

/*
Method ~rangeSearch~ :

*/
void MTree::rangeSearch(DistData* data,
                        const DFUN_RESULT& searchRad,
                        list<TupleId>* results)
{
  #ifdef MTREE_DEBUG
  assert(isInitialized());
  #endif
    cout << treeMngr->cacheSize()/1024 << " kb, open nodes: "
         << openNodes() << "/" << openEntries() << "\t";

  results->clear();
  list< pair<DFUN_RESULT, TupleId> > resultList;

  stack<RemainingNodesEntry> remainingNodes;
  remainingNodes.push(RemainingNodesEntry(header.root, 0));

  #ifdef MTREE_PRINT_SEARCH_INFO
  unsigned entryCount = 0;
  unsigned nodeCount = 0;
  unsigned distComputations = 0;
  #endif

  NodePtr node;

  while(!remainingNodes.empty())
  {
    #ifdef MTREE_PRINT_SEARCH_INFO
    nodeCount++;
    #endif

    node = getNode(remainingNodes.top().nodeId);
    DFUN_RESULT distQueryParent = remainingNodes.top().dist;
    remainingNodes.pop();

    if(node->isLeaf())
    {
      LeafNodePtr curNode = node->cast<LeafNode>();
      for(LeafNode::iterator it = curNode->begin();
          it != curNode->end(); ++it)
      {
        DFUN_RESULT dist = (*it)->dist();
        DFUN_RESULT distDiff = fabs(distQueryParent - dist);
        if ((distDiff  < searchRad) ||
             nearlyEqual<DFUN_RESULT>(distDiff, searchRad))
        {
          #ifdef MTREE_PRINT_SEARCH_INFO
          entryCount++;
          distComputations++;
          #endif

          DFUN_RESULT distQueryCurrent;
          df_info.dist(data, (*it)->data(), distQueryCurrent);
          if ((distQueryCurrent < searchRad) ||
              nearlyEqual<DFUN_RESULT>(distQueryCurrent, searchRad))
          {
            resultList.push_back(pair<DFUN_RESULT, TupleId>(
                distQueryCurrent, (*it)->tid()));
          }
        } // if
      } // for
    } else
    {
      InternalNodePtr curNode = node->cast<InternalNode>();
      for(InternalNode::iterator it = curNode->begin();
          it != curNode->end(); ++it)
      {
        DFUN_RESULT dist = (*it)->dist();
        DFUN_RESULT radSum = searchRad + (*it)->rad();
        DFUN_RESULT distDiff = fabs(distQueryParent - dist);
        if ((distDiff  < radSum) ||
             nearlyEqual<DFUN_RESULT>(distDiff, radSum))
        {
          #ifdef MTREE_PRINT_SEARCH_INFO
          distComputations++;
          #endif

          DFUN_RESULT newDistQueryParent;
          df_info.dist(data, (*it)->data(), newDistQueryParent);
          if ((newDistQueryParent < radSum) ||
              nearlyEqual<DFUN_RESULT>(newDistQueryParent, radSum))
          {
            remainingNodes.push(RemainingNodesEntry(
                (*it)->chield(), newDistQueryParent));
          }
        } // if
      } // for
    } // else
  } // while

  delete data;

  resultList.sort();
  list<pair<DFUN_RESULT, TupleId> >::iterator it = resultList.begin();
  while (it != resultList.end())
  {
    results->push_back(it->second);
    it++;
  }

  #ifdef MTREE_PRINT_SEARCH_INFO
  unsigned maxNodes = header.internalCount + header.leafCount;
  unsigned maxEntries = header.entryCount;
  unsigned maxDistComputations = maxNodes + maxEntries - 1;
  cmsg.info()
      << "Distance computations : " << distComputations << "\t(max "
      << maxDistComputations << ")" << endl
      << "Nodes analyzed        : " << nodeCount << "\t(max "
      << maxNodes << ")" << endl
      << "Entries analyzed      : " << entryCount << "\t(max "
      << maxEntries << ")" << endl << endl;
  cmsg.send();
  #endif
}

/*
Method ~nnSearch~ :

*/
void MTree::nnSearch(DistData* data, int nncount,
                          list<TupleId>* results)
{
  #ifdef MTREE_DEBUG
  assert(isInitialized());
  #endif

  results->clear();

  // init nearest neighbours array
  list< NNEntry > nearestNeighbours;
  for (int i=0; i<nncount; i++)
  {
    nearestNeighbours.push_back(
        NNEntry(0, numeric_limits<DFUN_RESULT>::infinity()));
  }

  vector< RemainingNodesEntryNNS > remainingNodes;

  #ifdef MTREE_PRINT_SEARCH_INFO
  unsigned entryCount = 0;
  unsigned nodeCount = 0;
  unsigned distComputations = 0;
  #endif

  remainingNodes.push_back(
      RemainingNodesEntryNNS(header.root, 0, 0));


  while(!remainingNodes.empty())
  {
    #ifdef MTREE_PRINT_SEARCH_INFO
    nodeCount++;
    #endif

    // read node with smallest minDist
    NodePtr node = getNode(remainingNodes.front().nodeId);
    DFUN_RESULT distQueryParent =
            remainingNodes.front().distQueryParent;
    DFUN_RESULT searchRad = nearestNeighbours.back().dist;

    // remove entry from remainingNodes heap
    pop_heap(remainingNodes.begin(), remainingNodes.end(),
              greater< RemainingNodesEntryNNS >());
    remainingNodes.pop_back();

    if (node->isLeaf())
    {
      LeafNodePtr curNode = node->cast<LeafNode>();
      for(LeafNode::iterator it = curNode->begin();
                             it != curNode->end(); ++it)
      {
        DFUN_RESULT distDiff = fabs(distQueryParent - (*it)->dist());
        if ((distDiff < searchRad) ||
             nearlyEqual<DFUN_RESULT>(distDiff, searchRad))
        {
          #ifdef MTREE_PRINT_SEARCH_INFO
          entryCount++;
          distComputations++;
          #endif

          DFUN_RESULT distQueryCurrent;
          df_info.dist(data, (*it)->data(), distQueryCurrent);

          if ((distQueryCurrent < searchRad) ||
               nearlyEqual<DFUN_RESULT>(distQueryCurrent, searchRad))
          {

            list<NNEntry>::iterator nnIter;
            nnIter = nearestNeighbours.begin();

            while ((distQueryCurrent > nnIter->dist) &&
                    (nnIter != nearestNeighbours.end()))
            {
              nnIter++;
            }

            bool done = false;
            if (nnIter != nearestNeighbours.end())
            {
              TupleId tid = (*it)->tid();
              DFUN_RESULT dist = distQueryCurrent;

              while (!done && (nnIter != nearestNeighbours.end()))
              {
                if (nnIter->tid == 0)
                {
                  nnIter->dist = dist;
                  nnIter->tid = tid;
                  done = true;
                }
                else
                {
                  swap(dist, nnIter->dist);
                  swap(tid, nnIter->tid);
                }
                nnIter++;
              }
            }

            searchRad = nearestNeighbours.back().dist;

            vector<RemainingNodesEntryNNS>::iterator
                it = remainingNodes.begin();

            while (it != remainingNodes.end())
            {
              if ((*it).minDist > searchRad)
              {
                swap(*it, remainingNodes.back());
                remainingNodes.pop_back();
              }
              else
                it++;
            }
            make_heap(remainingNodes.begin(),
                       remainingNodes.end(),
                       greater<RemainingNodesEntryNNS>());

          } // if
        } // if
      } // for
    } // if
    else
    {
      InternalNodePtr curNode = node->cast<InternalNode>();
      for(InternalNode::iterator it = curNode->begin();
          it != curNode->end(); ++it)
      {
        DFUN_RESULT distDiff = fabs(distQueryParent - (*it)->dist());
        DFUN_RESULT radSum = searchRad + (*it)->rad();
        if ((distDiff < radSum) ||
             nearlyEqual<DFUN_RESULT>(distDiff, radSum))
        {
          #ifdef MTREE_PRINT_SEARCH_INFO
          distComputations++;
          #endif

          DFUN_RESULT newDistQueryParent;
          df_info.dist(data, (*it)->data(), newDistQueryParent);

          DFUN_RESULT minDist, maxDist;
          minDist = max(newDistQueryParent - (*it)->rad(),
                        static_cast<DFUN_RESULT>(0));
          maxDist = newDistQueryParent + (*it)->rad();

          if ((minDist < searchRad) ||
               nearlyEqual<DFUN_RESULT>(minDist, searchRad))
          {
            // insert new entry into remainingNodes heap
            remainingNodes.push_back(RemainingNodesEntryNNS(
                (*it)->chield(), newDistQueryParent, minDist));
            push_heap(remainingNodes.begin(), remainingNodes.end(),
                greater<RemainingNodesEntryNNS>());

            if (maxDist < searchRad)
            {
              // update nearesNeighbours
              list<NNEntry>::iterator nnIter;
              nnIter = nearestNeighbours.begin();

              while ((maxDist > (*nnIter).dist) &&
                      (nnIter != nearestNeighbours.end()))
              {
                nnIter++;
              }

              if (((*nnIter).tid == 0) &&
                   (nnIter != nearestNeighbours.end()))
              {
                if (!nearlyEqual<DFUN_RESULT>(
                        maxDist, (*nnIter).dist))
                {
                  nearestNeighbours.insert(
                      nnIter, NNEntry(0, maxDist));
                  nearestNeighbours.pop_back();
                }
              }

              searchRad = nearestNeighbours.back().dist;

              vector<RemainingNodesEntryNNS>::iterator it =
                  remainingNodes.begin();

              while (it != remainingNodes.end())
              {
                if ((*it).minDist > searchRad)
                {
                  it = remainingNodes.erase(it);
                }
                else
                  it++;
              }
              make_heap(remainingNodes.begin(),
                         remainingNodes.end(),
                         greater<RemainingNodesEntryNNS>());
            }
          }
        }
      }
    }
  } // while

  delete data;
  list< NNEntry >::iterator it;
  for (it = nearestNeighbours.begin();
        it != nearestNeighbours.end(); it++)
  {
    if ((*it).tid != 0)
    {
      results->push_back((*it).tid);
    }
  }

  #ifdef MTREE_PRINT_SEARCH_INFO
  unsigned maxNodes = header.internalCount + header.leafCount;
  unsigned maxEntries = header.entryCount;
  unsigned maxDistComputations = maxNodes + maxEntries - 1;
  cmsg.info()
      << "Distance computations : " << distComputations << "\t(max "
      << maxDistComputations << ")" << endl
      << "Nodes analyzed        : " << nodeCount << "\t(max "
      << maxNodes << ")" << endl
      << "Entries analyzed      : " << entryCount << "\t(max "
      << maxEntries << ")" << endl << endl;
  cmsg.send();
  #endif
}
