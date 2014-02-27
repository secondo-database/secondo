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

1 Headerfile "MTree.h"[4]

January-May 2008, Mirko Dibbert

1.1 Overview

This file contains the "MTree"[4] class and some auxiliary structures.

1.1 Includes and defines

*/
#ifndef __MTREE_H__
#define __MTREE_H__

#include "MTreeBase.h"
#include "MTreeSplitpol.h"
#include "MTreeConfig.h"
#include "SecondoInterface.h"
#include "AlgebraManager.h"

extern SecondoInterface* si;
extern AlgebraManager* am;

namespace mtreeAlgebra
{

/*
1.1 Struct "SearchBestPathEntry"[4]:

This struct is needed in the "insert"[4] method of "mtree"[4].

*/
struct SearchBestPathEntry
{
    SearchBestPathEntry(
            InternalEntry* _entry, double _dist,
            unsigned _index) :
        entry(_entry), dist(_dist), index(_index)
    {}

    mtreeAlgebra::InternalEntry* entry;
    double dist;
    unsigned index;
};

/********************************************************************
1.1.1 Struct "SearchBestPathEntry"[4]:

This struct is needed in the "mtree::rangeSearch"[4] method.

********************************************************************/
struct RemainingNodesEntry
{
    SmiRecordId nodeId;
    double dist;

    RemainingNodesEntry(
            SmiRecordId _nodeId, double _dist) :
        nodeId(_nodeId), dist (_dist)
    {}
};

/********************************************************************
1.1.1 Struct "SearchBestPathEntry"[4]:

This struct is needed in the "mtree::nnSearch"[4] method.

********************************************************************/
struct RemainingNodesEntryNNS
{
    SmiRecordId nodeId;
    double minDist;
    double distQueryParent;

    RemainingNodesEntryNNS(
            SmiRecordId _nodeId, double _distQueryParent,
            double _minDist) :
        nodeId(_nodeId), minDist(_minDist),
        distQueryParent(_distQueryParent)
    {}

    bool operator > (const RemainingNodesEntryNNS& op2) const
    { return (minDist > op2.minDist); }
};

/********************************************************************
1.1.1 Struct "NNEntry"[4]:

This struct is needed in the "mtree::nnSearch"[4] method.

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
1.1 Struct "Header"[4]

********************************************************************/
struct Header
    : public gtree::Header
{
    Header()
        : gtree::Header(), initialized(false)
    {
        distfunName[0] = '\0';
        configName[0] = '\0';
    }

    STRING_T distfunName; // name of the used metric
    STRING_T configName;  // name of the MTreeConfig object
    DistDataId dataId;    // id of the used distdata type
    bool initialized;     // true, if the mtree has been initialized
};

/********************************************************************
1.1 Class "MTree"[4]

********************************************************************/
class MTree
    : public gtree::Tree<Header>
{

public:
/*
Default Constructor, creates a new m-tree.

*/
    inline MTree(bool temporary = false)
        : gtree::Tree<Header>(temporary), splitpol(0)
    {}

/*
Constructor, opens an existing tree.

*/
    inline MTree(const SmiFileId fileId)
        : gtree::Tree<Header>(fileId), splitpol(0)
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
    inline MTree(const MTree& mtree)
        : gtree::Tree<Header>(mtree), splitpol(0)
    {
        if (mtree.isInitialized())
            initialize();
    }

/*
Destructor

*/
    inline ~MTree()
    {
        if (splitpol)
            delete splitpol;
    }

/*
Initializes a new created m-tree. This method must be called, before a new tree could be used.

*/
    void initialize(
            DistDataId dataId,
            const string &distfunName,
            const string &configName);

/*
Creates a new LeafEntry from "attr "[4] and inserts it into the mtree.

*/
    void insert(Attribute *attr, TupleId tupleId);

/*
Creates a new LeafEntry from "data"[4] and inserts it into the mtree.

*/
    void insert(DistData* data, TupleId tupleId);

/*
Inserts a new entry into the mtree.

*/
    void insert(LeafEntry* entry, TupleId tupleId);


/*
Returns all entries, wich have a maximum distance of "rad"[4] to the given "Attribute"[4] object in the result list.

*/
    inline void rangeSearch(
            Attribute *attr, const double &rad,
            list<TupleId> *results)
    { rangeSearch(df_info.getData(attr), rad, results); }

/*
Returns all entries, wich have a maximum distance of "rad"[4] to the given "DistData"[4] object in the result list.

*/
    void rangeSearch(
            DistData *data, const double &rad,
            list<TupleId> *results);


/*
Returns the "nncount"[4] nearest neighbours ot the "Attribute"[4] object in the result list.

*/
    inline void nnSearch(
            Attribute *attr, int nncount, list<TupleId> *results)
    { nnSearch(df_info.getData(attr), nncount, results); }

/*
Returns the "nncount"[4] nearest neighbours ot the "DistData"[4] object in the result list.

*/
    void nnSearch(
            DistData *data, int nncount, list<TupleId> *results);

/*
Returns the name of the assigned type constructor.

*/
    inline string typeName()
    { return df_info.data().typeName(); }

/*
Returns the name of the assigned distance function.

*/
    inline string distfunName()
    { return header.distfunName; }

/*
Returns the name of the assigned distdata type.

*/
    inline string dataName()
    { return df_info.data().name(); }

/*
Returns the id of the assigned distdata type.

*/
    inline DistDataId& dataId()
    { return header.dataId; }

/*
Returns the name of the used "MTreeConfig"[4] object.

*/
    inline string configName()
    { return header.configName; }

/*
Returns true, if the m-tree has already been initialized.

*/
    inline bool isInitialized() const
    { return header.initialized; }

/*
Prints some infos about the tree to cmsg.info().

*/
    void printTreeInfos()
    {
        cmsg.info() << endl
            << "<mtree infos>" << endl
            << "   entries                : "
            << entryCount() << endl
            << "   height                 : "
            << height() << endl
            << "   internal nodes         : "
            << internalCount() << endl
            << "   leaf nodes             : "
            << leafCount() << endl
            << "   assigned config        : "
            << configName() << endl
            << "   assigned type          : "
            << typeName() << endl
            << "   assigned distfun       : "
            << header.distfunName << endl
            << "   assigned distdata type : "
            << df_info.data().name() << endl
            << endl;
        cmsg.send();
    }

    static const string BasicType() { return "mtree"; }
    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }

private:
    Splitpol* splitpol;  // reference to chosen split policy
    DistfunInfo df_info; // assigned DistfunInfo object
    MTreeConfig config;  // assigned MTreeConfig object

/*
Adds prototypes for the avaliable node types.

*/
    void registerNodePrototypes();

/*
Initializes distfunInfo splitpol objects and calls the "registerNodePrototypes" method. This method needs an initialized header to work.

*/
    void initialize();

/*
Splits an node by applying the split policy defined in the MTreeConfing object.

*/
    void split();
}; // MTree

} // namespace mrteeAlgebra
#endif // ifdef __MTREE_H__
