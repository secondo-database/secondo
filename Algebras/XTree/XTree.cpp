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

1 Implementation file "XTree.cpp"[4]

January-May 2008, Mirko Dibbert

*/
#include "XTree.h"

using namespace xtreeAlgebra;
using namespace gtree;
using namespace gta;
using namespace std;

/*
Method ~registerNodePrototypes~:

*/
void XTree::registerNodePrototypes()
{
    gtree::NodeConfigPtr internalConfig =
          new NodeConfig(config.internalNodeConfig);

/*
The maximum page size needs to be limited, due to some problems with the actual implementation of the general-tree framework with handling very huge nodes which span several hundreds of pages.

*/
    gtree::NodeConfigPtr supernodeConfig =
            new NodeConfig(
                    SUPERNODE, supernodePrio,
                    2,                               // min entries
                    numeric_limits<unsigned>::max(), // max entries
                    500,                             // max pages
                    supernodeCacheable);

    addNodePrototype(new InternalNode(internalConfig,
        internalConfig, supernodeConfig));

    addNodePrototype(new InternalNode(supernodeConfig,
        internalConfig, supernodeConfig));

    addNodePrototype(new LeafNode(
        new NodeConfig(config.leafNodeConfig)));
}



/*
Method ~initialize~ :

*/
void XTree::initialize()
{
    if (nodeCacheEnabled)
        treeMngr->enableCache();

    config = XTreeConfigReg::getConfig(header.configName);
}



/*
Method ~initialize~ :

*/
void XTree::initialize(
        unsigned dim,
        const string &configName,
        const string &typeName,
        int getdataType,
        const string &getdataName)
{
    if (isInitialized())
        return;

    header.dim = dim;
    strcpy(header.configName, configName.c_str());
    strcpy(header.typeName, typeName.c_str());
    strcpy(header.getdataName, getdataName.c_str());
    header.getdataType = getdataType;
    initialize();
    header.initialized = true;
    registerNodePrototypes();
    createRoot(LEAF);
}



/*
Method "overlapMinimalSplit"[4]:

*/
unsigned XTree::overlapMinimalSplit(
            vector<InternalEntry*> *in,
            vector<InternalEntry*> *out1,
            vector<InternalEntry*> *out2)
{
    unsigned n = in->size();
    unsigned minEntries = 1;
    vector<unsigned> axes(header.dim);

    SplitHist hist(*(*in)[0]->history());
    for (unsigned i = 1; i < n; ++i)
        hist &= *(*in)[i]->history();

    for (unsigned i = 0; i < header.dim; ++i)
        if(hist[i])
            axes.push_back(i);

    // at this point, axes contains the split axes, that had been
    // used as split axes for all entries (at least the split axes
    // used in the root of the split-tree)

    HRect *bbox1_lb, *bbox2_lb, *bbox1_ub, *bbox2_ub;
    SortedBBox<InternalEntry> sorted_lb[n], sorted_ub[n];

    /////////////////////////////////////////////////////////////////
    // chose split axis
    /////////////////////////////////////////////////////////////////
    unsigned split_axis = 0;
    double marginSum;
    double minMarginSum = numeric_limits<double>::infinity();
    for(vector<unsigned>::iterator iter = axes.begin();
        iter != axes.end(); ++iter)
    {
        // sort entries by lower/upper bound for actual dimension
        for (unsigned i = 0; i < n; ++i)
        {
            sorted_lb[i].dim = sorted_ub[i].dim = *iter;
            sorted_lb[i].index = sorted_ub[i].index = i;
            sorted_lb[i].bbox = sorted_ub[i].bbox = (*in)[i]->bbox();
        }
        qsort(sorted_lb, n, sizeof(SortedBBox<InternalEntry>),
                SortedBBox<InternalEntry>::sort_lb);
        qsort(sorted_ub, n, sizeof(SortedBBox<InternalEntry>),
                SortedBBox<InternalEntry>::sort_ub);

        for (unsigned k = 0; k < n - 2*minEntries + 1; ++k)
        { // for all distributions
            // compute bounding boxes for actual distribution
            unsigned pos = 0;

            bbox1_lb = new HRect(*(sorted_lb[pos].bbox));
            bbox1_ub = new HRect(*(sorted_ub[pos].bbox));
            ++pos;

            while(pos < minEntries+k)
            {
                bbox1_lb->unite(sorted_lb[pos].bbox);
                bbox1_ub->unite(sorted_ub[pos].bbox);
                ++pos;
            }

            bbox2_lb = new HRect(*(sorted_lb[pos].bbox));
            bbox2_ub = new HRect(*(sorted_ub[pos].bbox));
            ++pos;

            while(pos < n)
            {
                bbox2_lb->unite(sorted_lb[pos].bbox);
                bbox2_ub->unite(sorted_ub[pos].bbox);
                ++pos;
            }

            // compute marginSum
            marginSum = bbox1_lb->margin();
            marginSum += bbox1_ub->margin();
            marginSum += bbox2_lb->margin();
            marginSum += bbox2_ub->margin();

            if (marginSum < minMarginSum)
            {
                minMarginSum = marginSum;
                split_axis = *iter;
            }

            delete bbox1_lb;
            delete bbox1_ub;
            delete bbox2_lb;
            delete bbox2_ub;
        }

    }

    /////////////////////////////////////////////////////////////////
    // chose split index
    /////////////////////////////////////////////////////////////////
    unsigned split_index = minEntries;
    double overlap;
    double minOverlap = numeric_limits<double>::infinity();
    bool lb;

    #ifdef __XTREE_SPLIT_USE_MIN_DEADSPACE
    double minDeadspace = numeric_limits<double>::infinity();
    #else
    double area;
    double minArea = numeric_limits<double>::infinity();
    #endif

    // sort entries by lower/upper bound for actual dimension
    for (unsigned i = 0; i < n; ++i)
    {
        sorted_lb[i].dim = sorted_ub[i].dim = split_axis;
        sorted_lb[i].index = sorted_ub[i].index = i;
        sorted_lb[i].bbox = sorted_ub[i].bbox = (*in)[i]->bbox();
        sorted_lb[i].entry = sorted_ub[i].entry = (*in)[i];
    }
    qsort(sorted_lb, n, sizeof(SortedBBox<InternalEntry>),
            SortedBBox<InternalEntry>::sort_lb);
    qsort(sorted_ub, n, sizeof(SortedBBox<InternalEntry>),
            SortedBBox<InternalEntry>::sort_ub);

    #ifdef __XTREE_SPLIT_USE_MIN_DEADSPACE
    double deadspace_lb = 0.0;
    double deadspace_ub = 0.0;
    #endif

    for (unsigned k = 0; k < n - 2*minEntries + 1; ++k)
    { // for all distributions
        // compute bounding boxes for actual distribution
        unsigned pos = 0;

        bbox1_lb = new HRect(*(sorted_lb[pos].bbox));
        bbox1_ub = new HRect(*(sorted_ub[pos].bbox));
        #ifdef __XTREE_SPLIT_USE_MIN_DEADSPACE
        deadspace_lb -= sorted_lb[pos].bbox->area();
        deadspace_ub -= sorted_ub[pos].bbox->area();
        #endif
        ++pos;

        while(pos < minEntries+k)
        {
            bbox1_lb->unite(sorted_lb[pos].bbox);
            bbox1_ub->unite(sorted_ub[pos].bbox);
            #ifdef __XTREE_SPLIT_USE_MIN_DEADSPACE
            deadspace_lb -= sorted_lb[pos].bbox->area();
            deadspace_ub -= sorted_ub[pos].bbox->area();
            #endif
            ++pos;
        }

        bbox2_lb = new HRect(*(sorted_lb[pos].bbox));
        bbox2_ub = new HRect(*(sorted_ub[pos].bbox));
        #ifdef __XTREE_SPLIT_USE_MIN_DEADSPACE
        deadspace_lb -= sorted_lb[pos].bbox->area();
        deadspace_ub -= sorted_ub[pos].bbox->area();
        #endif
        ++pos;

        while(pos < n)
        {
            bbox2_lb->unite(sorted_lb[pos].bbox);
            bbox2_ub->unite(sorted_ub[pos].bbox);
            #ifdef __XTREE_SPLIT_USE_MIN_DEADSPACE
            deadspace_lb -= sorted_lb[pos].bbox->area();
            deadspace_ub -= sorted_ub[pos].bbox->area();
            #endif
            ++pos;
        }

#ifdef __XTREE_SPLIT_USE_MIN_DEADSPACE
// use minimal deadspace to resolve ties
        // compute overlap and area of lb_sort
        overlap = bbox1_lb->overlap(bbox2_lb);
        deadspace_lb += bbox1_lb->area() + bbox2_lb->area();
        if ((overlap < minOverlap) ||
           ((overlap == minOverlap) && (deadspace_lb < minDeadspace)))
        {
            minOverlap = overlap;
            minDeadspace = deadspace_lb;
            split_index = minEntries+k;
            lb = true;
        }

        // compute overlap and area of ub_sort
        overlap = bbox1_ub->overlap(bbox2_ub);
        deadspace_ub += bbox1_ub->area() + bbox2_ub->area();
        if ((overlap < minOverlap) ||
           ((overlap == minOverlap) && (deadspace_ub < minDeadspace)))
        {
            minOverlap = overlap;
            minDeadspace = deadspace_ub;
            split_index = minEntries+k;
            lb = false;
        }
#else
// use minimal area to resolve ties
        overlap = bbox1_ub->overlap(bbox2_lb);
        area = bbox1_lb->area() + bbox2_lb->area();
        if ((overlap < minOverlap) ||
                ((overlap == minOverlap) && (area < minArea)))
        {
            minOverlap = overlap;
            minArea = area;
            split_index = minEntries+k;
            lb = true;
        }

        // compute overlap and area of ub_sort
        overlap = bbox1_ub->overlap(bbox2_ub);
        area = bbox1_ub->area() + bbox2_ub->area();
        if ((overlap < minOverlap) ||
                ((overlap == minOverlap) && (area < minArea)))
        {
            minOverlap = overlap;
            minArea = area;
            split_index = minEntries+k;
            lb = false;
        }
#endif

        delete bbox1_lb;
        delete bbox1_ub;
        delete bbox2_lb;
        delete bbox2_ub;
    }

    // compute distribution
    if (lb)
    {
        unsigned i;
        for (i = 0; i < split_index; ++i)
            out1->push_back(sorted_lb[i].entry);
        for (; i < n; ++i)
            out2->push_back(sorted_lb[i].entry);
    }
    else
    {
        unsigned i;
        for (i = 0; i < split_index; ++i)
            out1->push_back(sorted_ub[i].entry);
        for (; i < n; ++i)
            out2->push_back(sorted_ub[i].entry);
    }
    return split_axis;
} // method overlapMinimalSplit



/*
Method ~split~:

*/
void XTree::split()
{
    unsigned split_axis;
    while (true)
    {
        if (treeMngr->curNode()->isLeaf())
        { // leaf node, using topologicalSplit
            LeafNodePtr curLeafNode =
                    treeMngr->curNode()->cast<LeafNode>();
            vector<LeafEntry*> *entries1 = new vector<LeafEntry*>;
            vector<LeafEntry*> *entries2 = new vector<LeafEntry*>;

            split_axis = topologicalSplit<LeafEntry>(
                    curLeafNode->entries(), entries1, entries2);

            NodePtr newNode(createNeighbourNode(LEAF));
            LeafNodePtr newLeafNode = newNode->cast<LeafNode>();

            curLeafNode->entries()->swap(*entries1);
            newLeafNode->entries()->swap(*entries2);
            delete entries1;
            delete entries2;

            treeMngr->recomputeSize(treeMngr->curNode());
            treeMngr->recomputeSize(newNode);

            HRect *bbox1 = curLeafNode->bbox();
            HRect *bbox2 = newLeafNode->bbox();

            if (treeMngr->hasParent())
            { // update parent node
                // update parent entry
                InternalEntry *entry1 =
                        treeMngr->parentEntry<InternalNode>();
                entry1->replaceHRect(bbox1);
                entry1->history()->set(split_axis);

                // insert new entry into parent node
                InternalEntry *entry2 = new InternalEntry(bbox2,
                        newNode->getNodeId(), entry1->history());
                if (!treeMngr->insert(
                        treeMngr->parentNode(), entry2))
                {
                    return;
                }

                // parent node needs to be splitted
                treeMngr->getParent();
            }
            else
            {   // insert new root
                InternalEntry *entry1 = new InternalEntry(
                        bbox1, treeMngr->curNode()->getNodeId());
                InternalEntry *entry2 = new InternalEntry(
                        bbox2, newNode->getNodeId());
                entry1->history()->set(split_axis);
                entry2->history()->set(split_axis);
                createRoot(INTERNAL, entry1, entry2);
                return;
            }
        }
        else
        { // internal node
            InternalNodePtr curInternalNode =
                    treeMngr->curNode()->cast<InternalNode>();

            vector<InternalEntry*> *entries1 =
                    new vector<InternalEntry*>;
            vector<InternalEntry*> *entries2 =
                    new vector<InternalEntry*>;

            // try topological split
            split_axis = topologicalSplit<InternalEntry>(
                    curInternalNode->entries(),
                    entries1, entries2);

            HRect *bbox1 = InternalNode::bbox(entries1);
            HRect *bbox2 = InternalNode::bbox(entries2);
            double overlap = bbox1->overlap(bbox2);
            double overlap_rel;
            if (overlap)
            {
                overlap_rel = overlap /
                    (bbox1->area() + bbox2->area() + overlap);
            }
            else
                overlap_rel = 0;

            if (overlap_rel > MAX_OVERLAP)
            {   // topological split fails, try overlap minimal split
                delete bbox1;
                delete bbox2;
                entries1->clear();
                entries2->clear();
                overlapMinimalSplit(
                        curInternalNode->entries(),
                        entries1, entries2);

                unsigned maxEntries =
                        curInternalNode->entries()->size() - 1;
                unsigned minEntries = static_cast<unsigned>
                        (maxEntries * MIN_FANOUT);
                if ((entries1->size() < minEntries) ||
                        (entries2->size() < minEntries))
                {   // overlapMinimalSplit also fails
                    // change current node type to supernode type
                    ++header.supernodeCount;
                    curInternalNode->setSupernode();
                    delete entries1;
                    delete entries2;
                    return;
                }
                else
                { // overlapMinimalSplit succeed, compute bboxes
                    bbox1 = InternalNode::bbox(entries1);
                    bbox2 = InternalNode::bbox(entries2);
                }
            }

            // split succeed
            NodePtr newNode(createNeighbourNode(INTERNAL));
            InternalNodePtr newInternalNode =
                    newNode->cast<InternalNode>();
            curInternalNode->entries()->swap(*entries1);
            newInternalNode->entries()->swap(*entries2);
            delete entries1;
            delete entries2;

            treeMngr->recomputeSize(treeMngr->curNode());
            treeMngr->recomputeSize(newNode);

            if (treeMngr->hasParent())
            { // update parent node
                // update parent entry
                InternalEntry *entry1 =
                        treeMngr->parentEntry<InternalNode>();
                entry1->replaceHRect(bbox1);
                entry1->history()->set(split_axis);

                // insert new entry into parent node
                InternalEntry *entry2 = new InternalEntry(bbox2,
                        newNode->getNodeId(), entry1->history());
                if (!treeMngr->insert(
                        treeMngr->parentNode(), entry2))
                {
                    return;
                }

                // parent node needs to be splitted
                treeMngr->getParent();
            }
            else
            {   // insert new root
                InternalEntry *entry1 = new InternalEntry(
                        bbox1, treeMngr->curNode()->getNodeId());
                InternalEntry *entry2 = new InternalEntry(
                        bbox2, newNode->getNodeId());
                entry1->history()->set(split_axis);
                entry2->history()->set(split_axis);
                createRoot(INTERNAL, entry1, entry2);
                return;
            }
        }
    } // while
}



/*
Method ~chooseSubtree~:

*/
int XTree::chooseSubtree(HRect *bbox)
{
    vector<SearchBestPathEntry> entriesIn;
    vector<SearchBestPathEntry> entriesOut;
    InternalNodePtr node = treeMngr->curNode()->cast<InternalNode>();
    vector<SearchBestPathEntry>::iterator iter, best;

    for(unsigned i=0; i<node->entryCount(); ++i)
    {
        if (node->entry(i)->bbox()->contains(bbox))
        {
            entriesIn.push_back(
                SearchBestPathEntry(node->entry(i), i));
        }
        else
        {
            entriesOut.push_back(
                SearchBestPathEntry(node->entry(i), i));
        }
    }

    if (!entriesIn.empty())
    {   // entry is already contained in all bounding boxes of
        // entriesIn - choose entry with smallest area
        best = entriesIn.begin();
        if (entriesIn.size() > 1)
        {
            iter = best;
            double minArea = iter->entry->bbox()->area();
            ++iter;
            while(iter != entriesIn.end())
            {
                double area = iter->entry->bbox()->area();
                if (area < minArea)
                {
                    minArea = area;
                    best = iter;
                }
                ++iter;
            }
        }
    }
    else
    { // entry is not contained in any existing bounding box
        if (treeMngr->curLevel() > 1)
        { // current node does not point to leaf nodes
            // chose e with least area enlargement
            // resolve ties by chosing entry with smallest area
            best = entriesOut.begin();
            if (entriesOut.size() > 1)
            {
                iter = best;
                HRect *curBBox = iter->entry->bbox();
                double minArea = curBBox->area();
                double minAreaEnlarge =
                        curBBox->Union(bbox).area() - minArea;
                ++iter;
                while(iter != entriesOut.end())
                {
                    curBBox = iter->entry->bbox();
                    double area = curBBox->area();
                    double areaEnlarge =
                            curBBox->Union(bbox).area() - area;
                    if ((areaEnlarge < minAreaEnlarge) ||
                            ((areaEnlarge == minAreaEnlarge) &&
                            (area < minArea)))
                    {
                        minArea = area;
                        minAreaEnlarge = areaEnlarge;
                        best = iter;
                    }
                    ++iter;
                }
            }
        }
        else
        { // current node points to leaf nodes
            // chose e with least overlap enlargement
            // resolve ties by chosing entry with smallest area enl.
            // resolve further ties by choosing e with smallest area
            best = entriesOut.begin();
            if (entriesOut.size() > 1)
            {
                iter = best;
                HRect *curBBox = iter->entry->bbox();
                HRect curBBox2 = curBBox->Union(bbox);
                double minArea = curBBox->area();
                double minAreaEnlarge =
                        curBBox2.area() - minArea;
                double minOverlapEnlarge = 0.0;
                for (unsigned i = 0; i < entriesOut.size()-1; ++i)
                    if (i != iter->index)
                    {
                        minOverlapEnlarge +=
                            curBBox2.overlap(iter->entry->bbox()) -
                            curBBox->overlap(iter->entry->bbox());
                    }
                ++iter;
                while(iter != entriesOut.end())
                {
                    curBBox = iter->entry->bbox();
                    curBBox2 = curBBox->Union(bbox);
                    double area = curBBox->area();
                    double areaEnlarge =
                            curBBox2.area() - area;
                    double overlapEnlarge = 0.0;
                    for (unsigned i=0; i < entriesOut.size()-1; ++i)
                        if (i != iter->index)
                        {
                            overlapEnlarge +=
                                curBBox2.overlap(iter->entry->bbox()) -
                                curBBox->overlap(iter->entry->bbox());
                        }
                    if ((overlapEnlarge < minOverlapEnlarge) ||
                        ((overlapEnlarge == minOverlapEnlarge) &&
                         (areaEnlarge < minAreaEnlarge)) ||
                        ((overlapEnlarge  == minOverlapEnlarge) &&
                         (areaEnlarge == minAreaEnlarge) &&
                         (area < minArea)))
                    {
                        minArea = area;
                        minAreaEnlarge = areaEnlarge;
                        minOverlapEnlarge = overlapEnlarge;
                        best = iter;
                    }
                    ++iter;
                }
            }
        }
        // update size of chosen bounding box
        best->entry->bbox()->unite(bbox);
        node->setModified();
    }
    return best->index;
}



/*
Method ~insert~:

*/
void XTree::insert(LeafEntry *entry)
{
    #ifdef __XTREE_DEBUG
    assert(isInitialized());
    #endif

    #ifdef __XTREE_PRINT_INSERT_INFO
    if ((header.entryCount % insertInfoInterval) == 0)
    {
        const string clearline = "\r" + string(70, ' ') + "\r";
        cmsg.info() << clearline
                    << header.entryCount << " entries, "
                    << header.internalCount << "/"
                    << header.leafCount << " dir/leaf nodes ("
                    << header.supernodeCount << " supernodes)";
        if(nodeCacheEnabled)
        {
            cmsg.info() << ", cache used: "
                        << treeMngr->cacheSize()/1024 << " kb";
        }
        cmsg.send();
    }
    #endif

    initPath();

    // select leaf node, into which the new entry should be inserted
    while (!treeMngr->curNode()->isLeaf())
    {
        treeMngr->getChield(chooseSubtree(entry->bbox()));
        #ifdef __XTREE_DEBUG
        if (treeMngr->curNode()->isLeaf())
            assert(treeMngr->curLevel() == 0);
        #endif
    }

    // insert entry into leaf, split if neccesary
    if (treeMngr->insert(treeMngr->curNode(), entry))
        split();

    ++header.entryCount;
}



/*
Method ~rangeSearch~:

*/
void XTree::rangeSearch(
        HPoint *p, const double &rad, list<TupleId> *results)
{
    #ifdef __XTREE_DEBUG
    assert(isInitialized());
    #endif

    results->clear();
    list< pair<double, TupleId> > resultList;
    stack<SmiRecordId> remainingNodes;
    remainingNodes.push(header.root);

    #ifdef __XTREE_ANALYSE_STATS
    unsigned entryCount = 0;
    unsigned pageCount = 0;
    unsigned nodeCount = 0;
    unsigned distComputations = 0;
    #endif

    NodePtr node;
    while(!remainingNodes.empty())
    {
        node = getNode(remainingNodes.top());
        remainingNodes.pop();

        #ifdef __XTREE_ANALYSE_STATS
        ++nodeCount;
        pageCount += node->pagecount();
        #endif

        if(node->isLeaf())
        {
            LeafNodePtr curNode = node->cast<LeafNode>();
            LeafNode::iterator it;
            for(it = curNode->begin(); it != curNode->end(); ++it)
            {
                #ifdef __XTREE_ANALYSE_STATS
                ++entryCount;
                ++distComputations;
                #endif
                double dist = (*it)->dist(p);
                if (dist <= rad)
                {
                    resultList.push_back(pair<double, TupleId>(
                            dist, (*it)->tid()));
                }
            }
        }
        else
        {
            InternalNodePtr curNode = node->cast<InternalNode>();
            InternalNode::iterator it;
            for(it = curNode->begin(); it != curNode->end(); ++it)
            {
                #ifdef __XTREE_ANALYSE_STATS
                ++distComputations;
                #endif
                double dist = SpatialDistfuns::
                        minDist(p, (*it)->bbox());
                if (dist <= rad)
                {
                    remainingNodes.push((*it)->chield());
                }
            }
        } // else
    } // while

    delete p;

    resultList.sort();
    list<pair<double, TupleId> >::iterator it = resultList.begin();
    while (it != resultList.end())
    {
        results->push_back(it->second);
        ++it;
    }

    #ifdef __XTREE_PRINT_STATS_TO_FILE
    cmsg.file("xtree.log")
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

    #ifdef __XTREE_PRINT_SEARCH_INFO
    unsigned maxNodes = header.internalCount + header.leafCount;
    unsigned maxEntries = header.entryCount;
    unsigned maxDistComputations = maxEntries + (maxNodes-1);
    cmsg.info()
        << "pages accessed        : " << pageCount << "\n"
        << "distance computations : " << distComputations
        << "\t(max " << maxDistComputations << ")\n"
        << "nodes analyzed        : " << nodeCount
        << "\t(max " << maxNodes << ")\n"
        << "entries analyzed      : " << entryCount
        << "\t(max " << maxEntries << ")\n\n";
    cmsg.send();
    #endif
} // rangeSearch


/*
Method ~windowIntersects~:

*/
void XTree::windowIntersects(HRect *r, list<TupleId> *results)
{
    #ifdef __XTREE_DEBUG
    assert(isInitialized());
    #endif

    results->clear();
    stack<SmiRecordId> remainingNodes;
    remainingNodes.push(header.root);

    #ifdef __XTREE_ANALYSE_STATS
    unsigned entryCount = 0;
    unsigned pageCount = 0;
    unsigned nodeCount = 0;
    #endif

    NodePtr node;
    while(!remainingNodes.empty())
    {
        node = getNode(remainingNodes.top());
        remainingNodes.pop();

        #ifdef __XTREE_ANALYSE_STATS
        ++nodeCount;
        pageCount += node->pagecount();
        #endif

        if(node->isLeaf())
        {
            LeafNodePtr curNode = node->cast<LeafNode>();
            LeafNode::iterator it;
            for(it = curNode->begin(); it != curNode->end(); ++it)
            {
                #ifdef __XTREE_ANALYSE_STATS
                ++entryCount;
                #endif

                if (r->intersects((*it)->bbox()))
                    results->push_back((*it)->tid());
            } // for
        }
        else
        {
            InternalNodePtr curNode = node->cast<InternalNode>();
            InternalNode::iterator it;
            for(it = curNode->begin(); it != curNode->end(); ++it)
            {
                if (r->intersects((*it)->bbox()))
                    remainingNodes.push((*it)->chield());
            } // for
        } // else
    } // while

    delete r;

    #ifdef __XTREE_PRINT_STATS_TO_FILE
    cmsg.file("xtree.log")
        << "windowintersects" << "\t"
        << header.internalCount << "\t"
        << header.leafCount << "\t"
        << header.entryCount << "\t"
        << nncount << "\t"
        << pageCount << "\t"
        << nodeCount << "\t"
        << entryCount << "\t"
        << results->size() << "\t\n";
    cmsg.send();
    #endif

    #ifdef __XTREE_PRINT_SEARCH_INFO
    unsigned maxNodes = header.internalCount + header.leafCount;
    unsigned maxEntries = header.entryCount;
    cmsg.info()
        << "pages accessed        : " << pageCount << "\n"
        << "nodes analyzed        : " << nodeCount
        << "\t(max " << maxNodes << ")\n"
        << "entries analyzed      : " << entryCount
        << "\t(max " << maxEntries << ")\n\n";
    cmsg.send();
    #endif
} // windowIntersects



/*
Method ~nnSearch~:

*/
void XTree::nnSearch(HPoint *p, int nncount, list<TupleId> *results)
{
    #ifdef __XTREE_DEBUG
    assert(isInitialized());
    #endif

    #ifdef __XTREE_ANALYSE_STATS
    unsigned entryCount = 0;
    unsigned pageCount = 0;
    unsigned nodeCount = 0;
    unsigned distComputations = 0;
    #endif

    results->clear();

    // init nearest neighbours array
    list<NNEntry> nearestNeighbours;
    for (int i = 0; i < nncount; ++i)
    {
        nearestNeighbours.push_back(
            NNEntry(0, numeric_limits<double>::infinity()));
    }

    vector<RemainingNodesEntryNNS> remainingNodes;


    remainingNodes.push_back(
        RemainingNodesEntryNNS(header.root, 0));

    while(!remainingNodes.empty())
    {
        // read node with smallest minDist
        NodePtr node = getNode(remainingNodes.front().nodeId);
        double rad = nearestNeighbours.back().dist;

        #ifdef __XTREE_ANALYSE_STATS
        pageCount += node->pagecount();
        ++nodeCount;
        #endif

        // remove entry from remainingNodes heap
        pop_heap(remainingNodes.begin(), remainingNodes.end(),
                greater<RemainingNodesEntryNNS>());
        remainingNodes.pop_back();

        if (node->isLeaf())
        { // leaf node
            LeafNodePtr curNode = node->cast<LeafNode>();
            LeafNode::iterator it;
            for(it = curNode->begin(); it != curNode->end(); ++it)
            {
                #ifdef __XTREE_ANALYSE_STATS
                ++entryCount;
                ++distComputations;
                #endif
                double dist = (*it)->dist(p);
                if (dist <= rad)
                {
                    // insert entry into nn-array
                    list<NNEntry>::iterator nnIter;
                    nnIter = nearestNeighbours.begin();
                    while ((dist > nnIter->dist) &&
                            (nnIter != nearestNeighbours.end()))
                    {
                        ++nnIter;
                    }

                    bool done = false;
                    if (nnIter != nearestNeighbours.end())
                    {
                        TupleId tid = (*it)->tid();
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
                            ++nnIter;
                        } // while
                    } // if

                    rad = nearestNeighbours.back().dist;
                    vector<RemainingNodesEntryNNS>::iterator it
                            = remainingNodes.begin();

                    while (it != remainingNodes.end())
                    {
                        if ((*it).minDist > rad)
                        {
                            swap(*it, remainingNodes.back());
                            remainingNodes.pop_back();
                        }
                        else
                            ++it;
                    }
                    make_heap(remainingNodes.begin(),
                            remainingNodes.end(),
                            greater<RemainingNodesEntryNNS>());
                } // if (dist <= rad)
            } // for
        }
        else
        { // internal node
            InternalNodePtr curNode = node->cast<InternalNode>();
            InternalNode::iterator it;
            for(it = curNode->begin(); it != curNode->end(); ++it)
            {
                #ifdef __XTREE_ANALYSE_STATS
                ++distComputations;
                #endif
                double minDist = SpatialDistfuns::
                        minDist(p, (*it)->bbox());
                double minMaxDist = SpatialDistfuns::
                        minMaxDist(p, (*it)->bbox());
                if (minDist <= rad)
                {
                    // insert new entry into remainingNodes heap
                    remainingNodes.push_back(RemainingNodesEntryNNS(
                        (*it)->chield(), minDist));
                    push_heap(
                            remainingNodes.begin(),
                            remainingNodes.end(),
                            greater<RemainingNodesEntryNNS>());

                    if (minMaxDist < rad)
                    {
                        // update nearesNeighbours
                        list<NNEntry>::iterator nnIter;
                        nnIter = nearestNeighbours.begin();

                        while ((minMaxDist > (*nnIter).dist) &&
                                (nnIter != nearestNeighbours.end()))
                        {
                            ++nnIter;
                        }

                        if (((*nnIter).tid == 0) &&
                            (nnIter != nearestNeighbours.end()))
                        {
                            if (minMaxDist != (*nnIter).dist)
                            {
                                nearestNeighbours.insert(
                                    nnIter, NNEntry(0, minMaxDist));
                                nearestNeighbours.pop_back();
                            }
                        }

                        rad = nearestNeighbours.back().dist;

                        vector<RemainingNodesEntryNNS>::iterator it =
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
                            make_heap(
                                    remainingNodes.begin(),
                                    remainingNodes.end(),
                                    greater<RemainingNodesEntryNNS>());
                    }
                }
            }
        }
    } // while

    list<NNEntry>::iterator it;
    for (it = nearestNeighbours.begin();
         it != nearestNeighbours.end(); ++it)
    {
        if ((*it).tid != 0)
        {
            results->push_back((*it).tid);
        }
    }

    delete p;
    #ifdef __XTREE_PRINT_STATS_TO_FILE
    cmsg.file("xtree.log")
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

    #ifdef __XTREE_PRINT_SEARCH_INFO
    unsigned maxNodes = header.internalCount + header.leafCount;
    unsigned maxEntries = header.entryCount;
    unsigned maxDistComputations = maxEntries + (maxNodes-1);
    cmsg.info()
        << "pages accessed        : " << pageCount << "\n"
        << "distance computations : " << distComputations
        << "\t(max " << maxDistComputations << ")\n"
        << "nodes analyzed        : " << nodeCount
        << "\t(max " << maxNodes << ")\n"
        << "entries analyzed      : " << entryCount
        << "\t(max " << maxEntries << ")\n\n";
    cmsg.send();
    #endif
} // nnSearch

/*
Method ~nnscan[_]init~:

*/
void XTree::nnscan_init(HPoint *p) 
{
    #ifdef __XTREE_DEBUG
    assert(isInitialized());
    #endif

    nnscan_ref = p;
    nnscan_queue.clear();
    nnscan_queue.push_back(NNScanEntry(true,header.root, 0));      
};

/*
Method ~nnscan[_]next~:

*/
TupleId XTree::nnscan_next() 
{
    while (!nnscan_queue.empty())
    {
	NNScanEntry e = nnscan_queue.front();
        pop_heap(nnscan_queue.begin(), nnscan_queue.end(),
                greater<NNScanEntry>());
        nnscan_queue.pop_back();
	if (e.isNodeId)
        { // node entry
            NodePtr node = getNode(e.nodeId);
            if (node->isLeaf())
            { // leaf node
                LeafNodePtr curNode = node->cast<LeafNode>();
                LeafNode::iterator it;
                for(it = curNode->begin(); it != curNode->end(); ++it)
                {
                    double dist = (*it)->dist(nnscan_ref);
                    nnscan_queue.push_back(NNScanEntry(false,
                                                       (*it)->tid(), dist));
                    push_heap(
                            nnscan_queue.begin(),
                            nnscan_queue.end(),
                            greater<NNScanEntry>());
                }
            }
            else
            { // internal node
                InternalNodePtr curNode = node->cast<InternalNode>();
                InternalNode::iterator it;
                for(it = curNode->begin(); it != curNode->end(); ++it)
                {
                    double dist = SpatialDistfuns::minDist(
                                                     nnscan_ref, (*it)->bbox());
                    nnscan_queue.push_back(NNScanEntry(true,
                                                       (*it)->chield(), dist));
                    push_heap(
                            nnscan_queue.begin(),
                            nnscan_queue.end(),
                            greater<NNScanEntry>());
                }
            }
        }
        else
        { // object entry
            return e.tid;
        }
    }

    // all indized entries processed
    return 0;
};

/*
Method ~nnscan[_]cleanup~:

*/
void XTree::nnscan_cleanup()
{
    delete nnscan_ref;
}
