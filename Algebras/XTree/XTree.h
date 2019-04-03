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

1.1 Headerfile "XTree.h"[4]

January-May 2008, Mirko Dibbert

1.1.1 Overview

This file contains the "XTree"[4] class and some auxiliary structures.

1.1.2 includes and defines

*/
#ifndef __XTREE_H__
#define __XTREE_H__

#include "XTreeConfig.h"
#include "XTreeAlgebra.h"
#include "XTreeBase.h"

namespace xtreeAlgebra
{

/********************************************************************
1.1.1 Struct "SearchBestPathEntry"[4]:

This struct is needed in the "xtree::insert"[4] method.

********************************************************************/
struct SearchBestPathEntry
{
    SearchBestPathEntry(InternalEntry* _entry, unsigned _index)
        : entry(_entry), index(_index)
    {}

    InternalEntry *entry;
    unsigned index;
}; // struct SerachBestPathEntry

/********************************************************************
1.1.1 Struct "RemainingNodesEntryNNS"[4]:

This struct is needed in the "xtree::nnSearch"[4] method.

********************************************************************/
struct RemainingNodesEntryNNS
{
    SmiRecordId nodeId;
    double minDist;

    RemainingNodesEntryNNS(
            SmiRecordId _nodeId, double _minDist)
        : nodeId(_nodeId), minDist(_minDist)
    {}

    bool operator > (const RemainingNodesEntryNNS& op2) const
    { return (minDist > op2.minDist); }
};

/********************************************************************
1.1.1 Struct "NNEntry"[4]:

This struct is needed in the "xtree::nnSearch"[4] method.

********************************************************************/
struct NNEntry
{
    TupleId tid;
    double dist;

    NNEntry(TupleId _tid, double _dist)
    : tid(_tid), dist(_dist)
    {}

    bool operator < (const NNEntry& op2) const
    {
        if (((tid == 0) && (op2.tid == 0)) ||
            ((tid != 0) && (op2.tid != 0)))
        {
            return (dist < op2.dist);
        }
        else  if ((tid == 0) && (op2.tid != 0))
        {
            return true;
        }
        else // ((tid != 0) && (op2.tid == 0))
        {
            return false;
        }
    }
};

/********************************************************************
1.1.1 Struct "NNEntry"[4]:

This struct is used for the nnscan methods.

********************************************************************/
struct NNScanEntry
{
    bool isNodeId;
    union
    {
        SmiRecordId nodeId;
        TupleId tid;
    };
    double dist;

 //    NNScanEntry(TupleId _tid, double _dist)
 //    : isNodeId(false), tid(_tid), dist(_dist)
 //    {}

    NNScanEntry(const bool _isNodeId,
                const SmiRecordId _nodeId,
                const double _dist)
    : isNodeId(_isNodeId), nodeId(_nodeId), dist(_dist)
    {}

    bool operator > (const NNScanEntry& op2) const
    {
        if (dist > op2.dist)
            return true;
        else if (dist < op2.dist)
            return false;

        // dist == op2.dist
        if (isNodeId)
        {
            if (op2.isNodeId)
            { // isNodeId && op2.isNodeId
                if (nodeId > op2.nodeId)
                    return true;
                else
                    return false;
            }
            else
            { // isNodeId && !op2.isNodeId
                return false;
            }
        }
        else
        { // !isNodeId
            if (op2.isNodeId)
            { // !isNodeId && op2.isNodeId
                return true;
            }
            else
            { // !isNodeId && !op2.isNodeId
                if (tid > op2.tid)
                    return true;
                else
                    return false;
            }
        }
    }
};

/********************************************************************
1.1 Struct "Header"[4]

********************************************************************/
struct Header
    : public gtree::Header
{
    Header()
        : gtree::Header(),
          supernodeCount(0), dim(0), initialized(false)
    {
        configName[0] = '\0';
        typeName[0] = '\0';
        getdataName[0] = '\0';
    }

    unsigned supernodeCount;
    STRING_T configName;
    STRING_T typeName;
    int getdataType;
    STRING_T getdataName;
    unsigned dim;
    bool initialized;
}; // struct Header

/********************************************************************
1.1 Class "XTree"[4]

********************************************************************/
class XTree
    : public gtree::Tree<Header>
{
public:
/*
Default Constructor, creates a new x-tree.

*/
    inline XTree(bool temporary = false)
        : gtree::Tree<Header>(temporary)
    {}

/*
Constructor, opens an existing tree.

*/
    inline XTree(const SmiFileId fileId)
        : gtree::Tree<Header>(fileId)
    {
        if (header.initialized)
        {
            initialize();
            registerNodePrototypes();
        }
    }

/*
Default copy constructor

*/
    inline XTree(const XTree &xtree)
        : gtree::Tree<Header>(xtree)
    {
        if (xtree.isInitialized())
            initialize();
    }

/*
Destructor

*/
    inline ~XTree()
    {}

/*
Initializes a new created x-tree. This method must be called, before a new tree could be used.

*/
    void initialize(
        unsigned dim,
        const std::string &configName,
        const std::string &typeName,
        int getdataType,
        const std::string &getdataName);

/*
Creates a new LeafEntry from "bbox"[4] and inserts it into the xtree.

*/
    void insert(LeafEntry *entry);

/*
Returns all entries, wich have a maximum (eucledean) distance of "rad"[4] to the given point in the result list (for spatial data, the distance to the center of the bounding box is used).

*/
    void rangeSearch(
            gta::HPoint *p, const double &rad, std::list<TupleId> *results);

/*
Returns all entries, wich intersect the given hyper rectangle in the result list.

*/
    void windowIntersects(gta::HRect *r, std::list<TupleId> *results);

/*
Returns the "nncount"[4] nearest neighbours ot the point in the result list.

*/
    void nnSearch(gta::HPoint *p, int nncount, std::list<TupleId> *results);

/*
These methods are used for the nnscan operator, which returns a ranking of the indized elements, based on their distance to to the reference object "p"[4].

*/
    void nnscan_init(gta::HPoint *p);
    TupleId nnscan_next();
    void nnscan_cleanup();

/*
Returns the count of all supernodes.

*/
    inline unsigned supernodeCount()
    { return header.supernodeCount;}

/*
Returns the dimension of the assigned bounding boxes.

*/
    inline unsigned dim()
    { return header.dim; }

/*
Returns the name of the used "XTreeConfig"[4] object.

*/
    inline std::string configName()
    { return header.configName; }

/*
Returns the name of the assigned type constructor

*/
    inline std::string typeName()
    { return header.typeName; }

/*
Returns the type of the assigned getdata function (gethpoint or gethrect).

*/
    inline int getdataType()
    { return header.getdataType; }

/*
Returns the name of the assigned getdata function.

*/
    inline std::string getdataName()
    { return header.getdataName; }

/*
Returns true, if the x-tree has already been initialized.

*/
    inline bool isInitialized() const
    { return header.initialized; }

/*
Prints some infos about the tree to cmsg.info().

*/
    void printTreeInfos()
    {
        cmsg.info() << endl
            << "<xtree infos>" << endl
            << "   dimension                 : "
            << dim() << endl
            << "   entries                   : "
            << entryCount() << endl
            << "   height                    : "
            << height() << endl
            << "   directory nodes           : "
            << internalCount() << endl
            << "   leaf nodes                : "
            << leafCount() << endl
            << "   supernodes                : "
            << supernodeCount() << endl
            << "   assigned config           : "
            << configName() << endl
            << "   assigned type             : "
            << header.typeName << endl
            << "   assigned getdata function : "
            << header.getdataName << endl
            << endl << endl;
        cmsg.send();
    }

    static const std::string BasicType() { return "xtree"; }
    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }

private:
    XTreeConfig config; // assigned XTreeConfig object

/*
Adds prototypes for the avaliable node types.

*/
    void registerNodePrototypes();

/*
Initializes "config"[4] and calls the "registerNodePrototypes" method. This method needs an initialized header to work.

*/
    void initialize();

/*
Splits a node.

*/
    void split();

/*
Topological Split.

*/
    template<class TEntry>
    unsigned topologicalSplit(
            std::vector<TEntry*> *in,
            std::vector<TEntry*> *out1,
            std::vector<TEntry*> *out2);

/*
Overlap minimal split.

*/
    unsigned overlapMinimalSplit(
            std::vector<InternalEntry*> *in,
            std::vector<InternalEntry*> *out1,
            std::vector<InternalEntry*> *out2);

/*
Selects one of the chields of "treeMngr->curNode"[4] as next node in the path.

*/
    int chooseSubtree(gta::HRect *bbox);

    std::vector<NNScanEntry> nnscan_queue;
    gta::HPoint *nnscan_ref;
}; // class XTree



/********************************************************************
1.1 Struct ~SortedHRect~

Auxiliary structure for "topologicalSplit"[4] and "overlapMinimalSplit"[4].

********************************************************************/
template <class TEntry>
struct SortedBBox
{
    unsigned dim;
    unsigned index;
    gta::HRect *bbox;
    TEntry *entry;

/*
Sort function for lower bound sort.

*/
    static int sort_lb(const void *lhs, const void *rhs)
    {
        const SortedBBox<TEntry> *s1 =
                static_cast<const SortedBBox<TEntry>*>(lhs);
        const SortedBBox<TEntry> *s2 =
                static_cast<const SortedBBox<TEntry>*>(rhs);

        unsigned dim = s1->dim;
        double diff = s1->bbox->lb(dim) - s2->bbox->lb(dim);

        if (diff < 0.0)
            return -1;
        else if (diff == 0.0)
            return 0;
        else
            return 1;
    }

/*
Sort function for upper bound sort.

*/
    static int sort_ub(const void *lhs, const void *rhs)
    {
        const SortedBBox<TEntry> *s1 =
                static_cast<const SortedBBox<TEntry>*>(lhs);
        const SortedBBox<TEntry> *s2 =
                static_cast<const SortedBBox<TEntry>*>(rhs);

        unsigned dim = s1->dim;
        double diff = s1->bbox->ub(dim) - s2->bbox->ub(dim);

        if (diff < 0.0)
            return -1;
        else if (diff == 0.0)
            return 0;
        else
            return 1;
    }
};

/********************************************************************
Method ~topologicalSplit~:

********************************************************************/
template<class TEntry>
unsigned XTree::topologicalSplit(
        std::vector<TEntry*>* in,
        std::vector<TEntry*>* out1,
        std::vector<TEntry*>* out2)
{
    unsigned n = in->size();
    unsigned minEntries = static_cast<unsigned>(n * 0.4);
    if (minEntries == 0)
        minEntries = 1;

    gta::HRect *bbox1_lb, *bbox2_lb, *bbox1_ub, *bbox2_ub;
    SortedBBox<TEntry> sorted_lb[n], sorted_ub[n];


    /////////////////////////////////////////////////////////////////
    // chose split axis
    /////////////////////////////////////////////////////////////////
    unsigned split_axis = 0;
    double marginSum;
    double minMarginSum = std::numeric_limits<double>::infinity();
    for(unsigned d = 0; d < header.dim; ++d)
    {
        // sort entries by lower/upper bound for actual dimension
        for (unsigned i = 0; i < n; ++i)
        {
            sorted_lb[i].dim = sorted_ub[i].dim = d;
            sorted_lb[i].index = sorted_ub[i].index = i;
            sorted_lb[i].bbox = sorted_ub[i].bbox = (*in)[i]->bbox();
        }
        qsort(sorted_lb, n, sizeof(SortedBBox<TEntry>),
                SortedBBox<TEntry>::sort_lb);
        qsort(sorted_ub, n, sizeof(SortedBBox<TEntry>),
                SortedBBox<TEntry>::sort_ub);

        for (unsigned k = 0; k < n - 2*minEntries + 1; ++k)
        { // for all possible distributions
            // compute bounding boxes for actual distribution
            unsigned pos = 0;

            bbox1_lb = new gta::HRect(*(sorted_lb[pos].bbox));
            bbox1_ub = new gta::HRect(*(sorted_ub[pos].bbox));
            ++pos;
            while(pos < minEntries+k)
            {
                bbox1_lb->unite(sorted_lb[pos].bbox);
                bbox1_ub->unite(sorted_ub[pos].bbox);
                ++pos;
            }

            bbox2_lb = new gta::HRect(*(sorted_lb[pos].bbox));
            bbox2_ub = new gta::HRect(*(sorted_ub[pos].bbox));
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
                split_axis = d;
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
    double minOverlap = std::numeric_limits<double>::infinity();
    bool lb = false;

    #ifdef XTREE_SPLIT_USE_MIN_DEADSPACE
    double minDeadspace = std::numeric_limits<double>::infinity();
    #else
    double area;
    double minArea = std::numeric_limits<double>::infinity();
    #endif

    // sort entries by lower/upper bound for actual dimension
    for (unsigned i = 0; i < n; ++i)
    {
        sorted_lb[i].dim = sorted_ub[i].dim = split_axis;
        sorted_lb[i].index = sorted_ub[i].index = i;
        sorted_lb[i].bbox = sorted_ub[i].bbox = (*in)[i]->bbox();
        sorted_lb[i].entry = sorted_ub[i].entry = (*in)[i];
    }
    qsort(sorted_lb, n, sizeof(SortedBBox<TEntry>),
            SortedBBox<TEntry>::sort_lb);
    qsort(sorted_ub, n, sizeof(SortedBBox<TEntry>),
            SortedBBox<TEntry>::sort_ub);

    #ifdef XTREE_SPLIT_USE_MIN_DEADSPACE
    double deadspace_lb = 0.0;
    double deadspace_ub = 0.0;
    #endif

    for (unsigned k = 0; k < n - 2*minEntries + 1; ++k)
    { // for all possible distributions
        // compute bounding boxes for actual distribution
        unsigned pos = 0;

        bbox1_lb = new gta::HRect(*(sorted_lb[pos].bbox));
        bbox1_ub = new gta::HRect(*(sorted_ub[pos].bbox));
        #ifdef XTREE_SPLIT_USE_MIN_DEADSPACE
        deadspace_lb -= sorted_lb[pos].bbox->area();
        deadspace_ub -= sorted_ub[pos].bbox->area();
        #endif
        ++pos;
        while(pos < minEntries+k)
        {
            bbox1_lb->unite(sorted_lb[pos].bbox);
            bbox1_ub->unite(sorted_ub[pos].bbox);
            #ifdef XTREE_SPLIT_USE_MIN_DEADSPACE
            deadspace_lb -= sorted_lb[pos].bbox->area();
            deadspace_ub -= sorted_ub[pos].bbox->area();
            #endif
            ++pos;
        }

        bbox2_lb = new gta::HRect(*(sorted_lb[pos].bbox));
        bbox2_ub = new gta::HRect(*(sorted_ub[pos].bbox));
        #ifdef XTREE_SPLIT_USE_MIN_DEADSPACE
        deadspace_lb -= sorted_lb[pos].bbox->area();
        deadspace_ub -= sorted_ub[pos].bbox->area();
        #endif
        ++pos;
        while(pos < n)
        {
            bbox2_lb->unite(sorted_lb[pos].bbox);
            bbox2_ub->unite(sorted_ub[pos].bbox);
            #ifdef XTREE_SPLIT_USE_MIN_DEADSPACE
            deadspace_lb -= sorted_lb[pos].bbox->area();
            deadspace_ub -= sorted_ub[pos].bbox->area();
            #endif
            ++pos;
        }

#ifdef XTREE_SPLIT_USE_MIN_DEADSPACE
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
        overlap = bbox1_ub->overlap(bbox2_ub);
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
 }

} // namespace xtreeAlgebra
#endif // #ifndef __XTREE_H__
