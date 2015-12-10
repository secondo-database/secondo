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

1 Implementation file "MTree.cpp"[4]

January-May 2008, Mirko Dibbert

*/
#include "MTree.h"

using namespace mtreeAlgebra;

/*
Function ~nearlyEqual~:

This auxiliary function is used in the search methods of the mtree class and returns true, if both numbers are "infiniy"[4] or nearly equal.

*/
template <typename FloatType>
inline bool nearlyEqual(FloatType a, FloatType b)
{
    FloatType infinity = std::numeric_limits<FloatType>::infinity();
    if (a == infinity)
        return (b == infinity);
    else if (b == infinity)
        return false;

    const FloatType scale = std::max(fabs(a), fabs(b));
    return  fabs(a - b) <=
            scale * 3 * std::numeric_limits<FloatType>::epsilon();
}

/*
Method ~registerNodePrototypes~:

*/
void MTree::registerNodePrototypes()
{
    addNodePrototype(new InternalNode(
        new gtree::NodeConfig(config.internalNodeConfig)));

    addNodePrototype(new LeafNode(
        new gtree::NodeConfig(config.leafNodeConfig)));
}

/*
Method ~initialize~:

*/
void MTree::initialize()
{
    if (nodeCacheEnabled)
        treeMngr->enableCache();

    config = MTreeConfigReg::getConfig(header.configName);
    df_info = gta::DistfunReg::getInfo(header.distfunName, header.dataId);
    splitpol = new Splitpol(
        config.promoteFun, config.partitionFun, df_info.distfun());
}

/*
Method ~initialize~ :

*/
void MTree::initialize(
        gta::DistDataId dataId,
        const std::string &distfunName,
        const std::string &configName)
{
    if (isInitialized())
        return;

    header.dataId = dataId;
    strcpy(header.distfunName, distfunName.c_str());
    strcpy(header.configName, configName.c_str());
    initialize();
    header.initialized = true;
    registerNodePrototypes();
    createRoot(LEAF);
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
        gtree::NodePtr newNode(createNeighbourNode(
            treeMngr->curNode()->isLeaf() ? LEAF : INTERNAL));

        // update node count, split node
        splitpol->apply(
                treeMngr->curNode(), newNode,
                treeMngr->curNode()->isLeaf());

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
                double distL, distR;
                gta::DistData* data =
                    treeMngr->parentEntry<InternalNode>()->data();
                df_info.dist(promL->data(), data, distL);
                df_info.dist(promR->data(), data, distR);
                promL->setDist(distL);
                promR->setDist(distR);
            }
        }
        else
        {   // insert new root
            createRoot(INTERNAL, promL, promR);
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
    catch (std::bad_alloc&)
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
        catch (std::bad_alloc&)
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
MTree::insert(gta::DistData* data, TupleId tupleId)
{
    // create new leaf entry
    LeafEntry* entry;
    try
    {
        entry = new LeafEntry(tupleId, data);
    }
    catch (std::bad_alloc&)
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
        catch (std::bad_alloc&)
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
void MTree::insert(LeafEntry *entry, TupleId tupleId)
{
    #ifdef __MTREE_DEBUG
    assert(isInitialized());
    #endif

    #ifdef __MTREE_PRINT_INSERT_INFO
    if ((header.entryCount % insertInfoInterval) == 0)
    {
        const std::string clearline = "\r" + std::string(70, ' ') + "\r";
        cmsg.info() << clearline
                    << header.entryCount << " entries, "
                    << header.internalCount << "/"
                    << header.leafCount << " dir/leaf nodes";
        if(nodeCacheEnabled)
        {
            cmsg.info() << ", cache used: "
                        << treeMngr->cacheSize()/1024 << " kb";
        }
        cmsg.send();
    }
    #endif

    initPath();

    // descent tree until leaf level
    while (!treeMngr->curNode()->isLeaf())
    { /* find best path (follow the entry with the nearest dist to
         new entry or the smallest covering radius increase) */
        std::list<SearchBestPathEntry> entriesIn;
        std::list<SearchBestPathEntry> entriesOut;

        #ifdef __MTREE_DEBUG
        if (treeMngr->curNode()->isLeaf())
            assert(treeMngr->curLevel() == 0);
        #endif

        InternalNodePtr node =
            treeMngr->curNode()->cast<InternalNode>();

        for(unsigned i = 0; i < node->entryCount(); ++i)
        {
            double dist;
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
        std::list<SearchBestPathEntry>::iterator best;

        if (!entriesIn.empty())
        { // select entry with nearest dist to new entry
            // (covering radius must not be increased)
            best = entriesIn.begin();
            std::list<SearchBestPathEntry>::iterator it;
            for (it = entriesIn.begin(); it != entriesIn.end(); ++it)
            {
                if (it->dist < best->dist)
                    best = it;
            }
        }
        else
        { // select entry with minimal radius increase
            double dist;
            df_info.dist(entriesOut.front().entry->data(),
                    entry->data(), dist);
            double minIncrease =
                    dist - entriesOut.front().entry->rad();
            double minDist = dist;

            best = entriesOut.begin();
            std::list<SearchBestPathEntry>::iterator it;
            for (it = entriesIn.begin(); it != entriesIn.end(); ++it)
            {
                df_info.dist(it->entry->data(), entry->data(), dist);
                double increase = dist - it->entry->rad();
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
        double dist;
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
void MTree::rangeSearch(
        gta::DistData *data, const double &rad,
        std::list<TupleId> *results)
{
  #ifdef __MTREE_DEBUG
  assert(isInitialized());
  #endif

  results->clear();
  std::list< std::pair<double, TupleId> > resultList;

  std::stack<RemainingNodesEntry> remainingNodes;
  remainingNodes.push(RemainingNodesEntry(header.root, 0));

  #ifdef __MTREE_ANALYSE_STATS
  unsigned entryCount = 0;
  unsigned pageCount = 0;
  unsigned nodeCount = 0;
  unsigned distComputations = 0;
  #endif

  gtree::NodePtr node;

  while(!remainingNodes.empty())
  {
    node = getNode(remainingNodes.top().nodeId);
    double distQueryParent = remainingNodes.top().dist;
    remainingNodes.pop();

    #ifdef __MTREE_ANALYSE_STATS
    pageCount += node->pagecount();
    ++nodeCount;
    #endif

    if(node->isLeaf())
    {
      LeafNodePtr curNode = node->cast<LeafNode>();
      for(LeafNode::iterator it = curNode->begin();
          it != curNode->end(); ++it)
      {
        double dist = (*it)->dist();
        double distDiff = fabs(distQueryParent - dist);
        if ((distDiff  < rad) ||
             nearlyEqual<double>(distDiff, rad))
        {
          #ifdef __MTREE_ANALYSE_STATS
          ++entryCount;
          ++distComputations;
          #endif

          double distQueryCurrent;
          df_info.dist(data, (*it)->data(), distQueryCurrent);
          if ((distQueryCurrent < rad) ||
              nearlyEqual<double>(distQueryCurrent, rad))
          {
            resultList.push_back(std::pair<double, TupleId>(
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
        double dist = (*it)->dist();
        double radSum = rad + (*it)->rad();
        double distDiff = fabs(distQueryParent - dist);
        if ((distDiff  < radSum) ||
             nearlyEqual<double>(distDiff, radSum))
        {
          #ifdef __MTREE_ANALYSE_STATS
          ++distComputations;
          #endif

          double newDistQueryParent;
          df_info.dist(data, (*it)->data(), newDistQueryParent);
          if ((newDistQueryParent < radSum) ||
              nearlyEqual<double>(newDistQueryParent, radSum))
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
  std::list<std::pair<double, TupleId> >::iterator it = resultList.begin();
  while (it != resultList.end())
  {
    results->push_back(it->second);
    ++it;
  }

  #ifdef __MTREE_PRINT_STATS_TO_FILE
  cmsg.file("mtree.log")
      << "rangesearch" << "\t"
      << header.internalCount << "\t"
      << header.leafCount << "\t"
      << header.entryCount << "\t"
      << nncount << "\t"
      << distComputations << "\t"
      << pageCount << "\t"
      << nodeCount << "\t"
      << entryCount << "\t"
      << results->size() << "\t\n";
  cmsg.send();
  #endif

  #ifdef __MTREE_PRINT_SEARCH_INFO
  unsigned maxNodes = header.internalCount + header.leafCount;
  unsigned maxEntries = header.entryCount;
  unsigned maxDistComputations = maxEntries + (maxNodes-1);
  cmsg.info()
      << "pages accessed        : " << pageCount << "\n"
      << "distance computations : " << distComputations << "\t(max "
      << maxDistComputations << ")\n"
      << "nodes analyzed        : " << nodeCount << "\t(max "
      << maxNodes << ")\n"
      << "entries analyzed      : " << entryCount << "\t(max "
      << maxEntries << ")\n\n";
  cmsg.send();
  #endif
}

/*
Method ~nnSearch~ :

*/
void MTree::nnSearch(
        gta::DistData *data, int nncount, std::list<TupleId> *results)
{
  #ifdef __MTREE_DEBUG
  assert(isInitialized());
  #endif

  results->clear();

  // init nearest neighbours array
  std::list<NNEntry> nearestNeighbours;
  for (int i = 0; i < nncount; ++i)
  {
    nearestNeighbours.push_back(
        NNEntry(0, std::numeric_limits<double>::infinity()));
  }

  std::vector<RemainingNodesEntryNNS> remainingNodes;

  #ifdef __MTREE_ANALYSE_STATS
  unsigned entryCount = 0;
  unsigned pageCount = 0;
  unsigned nodeCount = 0;
  unsigned distComputations = 0;
  #endif

  remainingNodes.push_back(
      RemainingNodesEntryNNS(header.root, 0, 0));

  while(!remainingNodes.empty())
  {
    // read node with smallest minDist
    gtree::NodePtr node = getNode(remainingNodes.front().nodeId);
    double distQueryParent =
            remainingNodes.front().distQueryParent;
    double rad = nearestNeighbours.back().dist;

    #ifdef __MTREE_ANALYSE_STATS
    pageCount += node->pagecount();
    ++nodeCount;
    #endif

    // remove entry from remainingNodes heap
    pop_heap(remainingNodes.begin(), remainingNodes.end(),
              std::greater<RemainingNodesEntryNNS>());
    remainingNodes.pop_back();

    if (node->isLeaf())
    {
      LeafNodePtr curNode = node->cast<LeafNode>();
      LeafNode::iterator it;
      for(it = curNode->begin(); it != curNode->end(); ++it)
      {
        double distDiff = fabs(distQueryParent - (*it)->dist());
        if ((distDiff < rad) ||
             nearlyEqual<double>(distDiff, rad))
        {
          #ifdef __MTREE_ANALYSE_STATS
          ++entryCount;
          ++distComputations;
          #endif

          double distQueryCurrent;
          df_info.dist(data, (*it)->data(), distQueryCurrent);

          if ((distQueryCurrent < rad) ||
               nearlyEqual<double>(distQueryCurrent, rad))
          {

            std::list<NNEntry>::iterator nnIter;
            nnIter = nearestNeighbours.begin();

            while ((distQueryCurrent > nnIter->dist) &&
                    (nnIter != nearestNeighbours.end()))
            {
              ++nnIter;
            }

            bool done = false;
            if (nnIter != nearestNeighbours.end())
            {
              TupleId tid = (*it)->tid();
              double dist = distQueryCurrent;

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
                  std::swap(dist, nnIter->dist);
                  std::swap(tid, nnIter->tid);
                }
                ++nnIter;
              }
            }

            rad = nearestNeighbours.back().dist;

            std::vector<RemainingNodesEntryNNS>::iterator
                it = remainingNodes.begin();

            while (it != remainingNodes.end())
            {
              if ((*it).minDist > rad)
              {
                std::swap(*it, remainingNodes.back());
                remainingNodes.pop_back();
              }
              else
                ++it;
            }
            make_heap(remainingNodes.begin(),
                       remainingNodes.end(),
                       std::greater<RemainingNodesEntryNNS>());

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
        double distDiff = fabs(distQueryParent - (*it)->dist());
        double radSum = rad + (*it)->rad();
        if ((distDiff < radSum) ||
             nearlyEqual<double>(distDiff, radSum))
        {
          #ifdef __MTREE_ANALYSE_STATS
          ++distComputations;
          #endif

          double newDistQueryParent;
          df_info.dist(data, (*it)->data(), newDistQueryParent);

          double minDist, maxDist;
          minDist = std::max(newDistQueryParent - (*it)->rad(),
                        static_cast<double>(0));
          maxDist = newDistQueryParent + (*it)->rad();

          if ((minDist < rad) ||
               nearlyEqual<double>(minDist, rad))
          {
            // insert new entry into remainingNodes heap
            remainingNodes.push_back(RemainingNodesEntryNNS(
                (*it)->chield(), newDistQueryParent, minDist));
            push_heap(remainingNodes.begin(), remainingNodes.end(),
                std::greater<RemainingNodesEntryNNS>());

            if (maxDist < rad)
            {
              // update nearesNeighbours
              std::list<NNEntry>::iterator nnIter;
              nnIter = nearestNeighbours.begin();

              while ((maxDist > (*nnIter).dist) &&
                      (nnIter != nearestNeighbours.end()))
              {
                ++nnIter;
              }

              if (((*nnIter).tid == 0) &&
                   (nnIter != nearestNeighbours.end()))
              {
                if (!nearlyEqual<double>(
                        maxDist, (*nnIter).dist))
                {
                  nearestNeighbours.insert(
                      nnIter, NNEntry(0, maxDist));
                  nearestNeighbours.pop_back();
                }
              }

              rad = nearestNeighbours.back().dist;

              std::vector<RemainingNodesEntryNNS>::iterator it =
                  remainingNodes.begin();

              while (it != remainingNodes.end())
              {
                if ((*it).minDist > rad)
                {
                  it = remainingNodes.erase(it);
                }
                else
                  ++it;
              }
              make_heap(remainingNodes.begin(),
                         remainingNodes.end(),
                         std::greater<RemainingNodesEntryNNS>());
            }
          }
        }
      }
    }
  } // while

  delete data;
  std::list< NNEntry >::iterator it;
  for (it = nearestNeighbours.begin();
        it != nearestNeighbours.end(); ++it)
  {
    if ((*it).tid != 0)
    {
      results->push_back((*it).tid);
    }
  }

  #ifdef __MTREE_PRINT_STATS_TO_FILE
  cmsg.file("mtree.log")
      << "nnsearch" << "\t"
      << header.internalCount << "\t"
      << header.leafCount << "\t"
      << header.entryCount << "\t"
      << nncount << "\t"
      << distComputations << "\t"
      << pageCount << "\t"
      << nodeCount << "\t"
      << entryCount << "\t"
      << results->size() << "\t\n";
  cmsg.send();
  #endif

  #ifdef __MTREE_PRINT_SEARCH_INFO
  unsigned maxNodes = header.internalCount + header.leafCount;
  unsigned maxEntries = header.entryCount;
  unsigned maxDistComputations = maxEntries + (maxNodes-1);
  cmsg.info()
      << "pages accessed        : " << pageCount << "\n"
      << "distance computations : " << distComputations << "\t(max "
      << maxDistComputations << ")\n"
      << "nodes analyzed        : " << nodeCount << "\t(max "
      << maxNodes << ")\n"
      << "entries analyzed      : " << entryCount << "\t(max "
      << maxEntries << ")\n\n";
  cmsg.send();
  #endif
}
