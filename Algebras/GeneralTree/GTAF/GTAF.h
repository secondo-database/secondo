/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% This file belongs to the GeneralTreeAlgebra framework (GTAF)           %
% Class descriptions and usage details could be found in gtaf.pdf        %
%                                                                        %
% (if this file does not exist, use "make docu" in the parent directory) %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
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

1.1 Headerfile "GTAF.h"[4]

January-February 2008, Mirko Dibbert

1.1.1 Includes and defines

*/
#ifndef __GENERAL_TREE_H
#define __GENERAL_TREE_H

#include "GTAF_Manager.h"
#include "GTAF_Nodes.h"

using namespace std;

namespace gtaf
{

/********************************************************************
1.1.1 Struct Header

********************************************************************/

struct Header
{
    Header()
            : root(0), height(0), entryCount(0),
            internalCount(0), leafCount(0)
    {}

    ~Header()
    {}

    SmiRecordId root;       // page of the root node
    unsigned height;        // height of the tree
    unsigned entryCount;    // count of the entries in the tree
    unsigned internalCount; // count of Internal nodes in the tree
    unsigned leafCount;     // count of leaf nodes in the tree
};

/********************************************************************
1.1.1 Class Tree

********************************************************************/
template <class THeader = Header, class TTreeManager = TreeManager>
class Tree : public ObjCounter<generalTrees>
{

public:
/*
Default constructor.

*/
    Tree(bool temporary = false);

/*
Constructor (reads a previously stored tree from file).

*/
    Tree(SmiFileId fileId);

/*
Default copy constructor.

*/
    Tree(const Tree& tree);

/*
Destructor.

*/
    ~Tree();

/*
Deletes the tree file.

*/
    inline void deleteFile();

/*
Returns the file id of the "SmiRecordFile"[4], that contains the tree.

*/
    inline SmiFileId fileId();

/*
Returns the count of all Internal nodes.

*/
    inline unsigned internalCount();

/*
Returns the count of all leafes.

*/
    inline unsigned leafCount();

/*
Returns the count of all entries, stored in the tree.

*/
    inline unsigned entryCount();

/*
Returns the height of the tree.

*/
    inline unsigned height();

/*
Returns the count of open general trees.

*/
    inline unsigned openTrees();

/*
Returns the count of open general tree nodes.

*/
    inline unsigned openNodes();

/*
Returns the count of open general tree entries.

*/
    inline unsigned openEntries();

protected:
/*
Adds a new protoype node.

*/
    inline void addNodePrototype(NodeBase* node);

/*
Creates a new node.

*/
    inline NodePtr createNode(NodeTypeId type, unsigned level = 0);

/*
Creates a new node on the same level as "treeMngr->curNode()"[4].

*/
    inline NodePtr createNeighbourNode(NodeTypeId type);

/*
Creates a new node on root level.

*/
    inline NodePtr createRoot(NodeTypeId type);

/*
Creates a new node on leaf level.

*/
    inline NodePtr createLeaf(NodeTypeId type);

/*
Reads the specified node from file.

*/
    inline NodePtr getNode(
        SmiRecordId nodeId, unsigned level = 0) const;

private:
/*
Reads the header from "file"[4].

*/
    void readHeader();

/*
Writes the header to "file"[4].

*/
    void writeHeader();

protected:
    THeader header;           // contains the header of the tree file
    SmiRecordFile file;       // contais the tree file

private:
    bool temporary;           /* true, if the file should be dropped
                               when the tree is deleted. */
    unsigned headerPageCount; // count of header pages
    static bool dbgMsgShown;  /* true, if the "debug mode enabled"
                               message has already been shown */

protected:
    NodeSupp* nodeSupp;       /* reference to the tree file and the
                                 node prototypes */
    TTreeManager* treeMngr;   // reference to the tree manager
};

// with dbgMsgShown == false, a "debug mode enabled" message will be
// shown, if the tree constructor is called first time.
#ifdef GTAF_DEBUG
template <class THeader, class TTreeManager>
bool gtaf::Tree<THeader, TTreeManager>::dbgMsgShown = false;
#else
template <class THeader, class TTreeManager>
bool gtaf::Tree<THeader, TTreeManager>::dbgMsgShown = true;
#endif

/********************************************************************
1.1.1 Implementation of class "Tree"[4]

********************************************************************/
/*
Tree constructor (new file):

*/
template <class THeader, class TTreeManager>
gtaf::Tree<THeader, TTreeManager>::Tree(bool _temporary)
        : header(), file(true, PAGESIZE), temporary(_temporary),
        nodeSupp(new NodeSupp(&file)),
        treeMngr(new TTreeManager(nodeSupp))
{
    if (!dbgMsgShown)
        { Msg::showDbgMsg(); dbgMsgShown = true; }

    file.Create();

    // create header page(s), compute headerPageCount
    SmiRecordId headerId;
    SmiRecord headerRecord;
    headerPageCount = 0;

    while (sizeof(THeader) > headerPageCount*PAGESIZE)
    {
        ++headerPageCount;
        file.AppendRecord(headerId, headerRecord);
        assert(headerId == headerPageCount);
    }
}

/*
Tree constructor (reads from file):

*/
template <class THeader, class TTreeManager>
gtaf::Tree<THeader, TTreeManager>::Tree(SmiFileId fileId)
        : header(), file(true), temporary(false),
        nodeSupp(new NodeSupp(&file)),
        treeMngr(new TTreeManager(nodeSupp))
{
    if (!dbgMsgShown)
        { Msg::showDbgMsg(); dbgMsgShown = true; }

    assert(file.Open(fileId));

    // compute headerPageCount
    headerPageCount = 0;

    while (sizeof(THeader) > headerPageCount*PAGESIZE)
        ++headerPageCount;

    readHeader();
}

/*
Tree copy constructor:

*/
template <class THeader, class TTreeManager>
gtaf::Tree<THeader, TTreeManager>::Tree(
    const Tree<THeader, TTreeManager>& tree)
        : file(true, PAGESIZE), temporary(tree.temporary),
        nodeSupp(new NodeSupp(&file)),
        treeMngr(new TTreeManager(nodeSupp))
{
    if (!dbgMsgShown)
        { Msg::showDbgMsg(); dbgMsgShown = true; }

    file.Create();

    // create header page(s), compute headerPageCount
    SmiRecordId headerId;
    SmiRecord headerRecord;
    headerPageCount = 0;

    while (sizeof(THeader) > headerPageCount*PAGESIZE)
    {
        ++headerPageCount;
        file.AppendRecord(headerId, headerRecord);
        assert(headerId == headerPageCount);
    }

    // copy header
    memcpy(&header, &tree.header, sizeof(THeader));

    nodeSupp->copyPrototypes(tree.nodeSupp);

    // disable node cache, while copying the tree structure
    treeMngr->disableCache();

    stack<pair<NodePtr, NodePtr> > remaining;

    stack<unsigned> indizes;

    unsigned curIndex = 0;

    // read tree root and create new root
    NodePtr source = tree.getNode(tree.header.root);

    NodePtr target = createNode(source->typeId());

    header.root = target->getNodeId();

    // copy tree structure (copies the tree node by node and updates
    // the chield node pointers, needs enough memory to hold
    // header.height nodes open at one time)
    if (source->isLeaf())
    {
        for (unsigned i = 0; i < source->entryCount(); ++i)
            target->insertCopy(source->baseEntry(i));
    }
    else
    {
        NodePtr chield = tree.getNode(source->chield(curIndex));
        NodePtr newChield = createNode(chield->typeId());
        target->insertCopy(source->baseEntry(curIndex));
        static_cast<InternalEntry*>(target->baseEntry(curIndex))->
        setChield(newChield);

        // push current node to stack, if
        // further entries are remaining
        if (++curIndex < source->entryCount())
        {
            remaining.push(pair<NodePtr, NodePtr>(source, target));
            indizes.push(curIndex);
        }

        // push chield node to stack
        remaining.push(pair<NodePtr, NodePtr>(chield, newChield));

        indizes.push(0);
    }

    while (!remaining.empty())
    {
        source = remaining.top().first;
        target = remaining.top().second;
        curIndex = indizes.top();
        remaining.pop();
        indizes.pop();

        if (source->isLeaf())
        {
            for (unsigned i = 0; i < source->entryCount(); ++i)
                target->insertCopy(source->baseEntry(i));
        }
        else
        {
            NodePtr chield = tree.getNode(source->chield(curIndex));
            NodePtr newChield = createNode(chield->typeId());
            target->insertCopy(source->baseEntry(curIndex));
            static_cast<InternalEntry*>(target->
                    baseEntry(curIndex))->setChield(newChield);

            // push current node to stack, if
            // further entries are remaining
            if (++curIndex < source->entryCount())
            {
                remaining.push(
                        pair<NodePtr, NodePtr>(source, target));
                indizes.push(curIndex);
            }

            // push chield node to stack
            remaining.push(
                    pair<NodePtr, NodePtr>(chield, newChield));

            indizes.push(0);
        }
    }

    treeMngr->enableCache();
} // tree copy constructor

/*
Tree destructor:

*/
template <class THeader, class TTreeManager>
gtaf::Tree<THeader, TTreeManager>::~Tree()
{
    delete treeMngr;
    delete nodeSupp;

    if (temporary)
        deleteFile();

    if (file.IsOpen())
    {
        writeHeader();
        file.Close();
    }

#ifdef GTAF_DEBUG
    /* print count of open objects only for the last remaining tree
      (otherwhise the warning message would appear allways, if more
      than one tree exists, e.g. when calling the copy constructor)*/
    if (openTrees() == 1)
    {
        if (openNodes())
            Msg::memoryLeak_Warning(openNodes(), "nodes");

        if (openEntries())
            Msg::memoryLeak_Warning(openEntries(), "entries");
    }
#endif
} // Tree destructor

/*
Method ~readHeader~:

*/
template <class THeader, class TTreeManager>
void
gtaf::Tree<THeader, TTreeManager>::readHeader()
{
    // create buffer
    char buffer[headerPageCount*PAGESIZE];
    memset(buffer, 0, headerPageCount*PAGESIZE);

    // read header-data from file
    SmiRecord record;
    SmiRecordId curPage = 1;

    for (unsigned i = 0; i < headerPageCount; ++i)
    {
        file.SelectRecord(curPage, record, SmiFile::ReadOnly);
        record.Read(buffer + (i*PAGESIZE), PAGESIZE, 0);
        ++curPage;
    }

    // copy buffer to header
    memcpy(&header, buffer, sizeof(THeader));
} // Tree::readHeader

/*
Method ~writeHeader~:

*/
template <class THeader, class TTreeManager>
void
gtaf::Tree<THeader, TTreeManager>::writeHeader()
{
    // create buffer and copy header to buffer
    char buffer[headerPageCount*PAGESIZE];
    memset(buffer, 0, headerPageCount*PAGESIZE);
    memcpy(buffer, &header, sizeof(THeader));

    // write header-data to file
    SmiRecord record;
    SmiRecordId curPage = 1;

    for (unsigned i = 0; i < headerPageCount; ++i)
    {
        file.SelectRecord(curPage, record, SmiFile::Update);
        record.Write(buffer + (i*PAGESIZE), PAGESIZE, 0);
        ++curPage;
    }
} // Tree::writeHeader

/*
Method ~deleteFile~:

*/
template <class THeader, class TTreeManager>
void
gtaf::Tree<THeader, TTreeManager>::deleteFile()
{
    if (file.IsOpen())
        file.Close();

    file.Drop();
}

/*
Method ~fileId~:

*/
template <class THeader, class TTreeManager>
SmiFileId
gtaf::Tree<THeader, TTreeManager>::fileId()
{ return file.GetFileId(); }

/*
Method ~internalCount~:

*/
template <class THeader, class TTreeManager>
unsigned
gtaf::Tree<THeader, TTreeManager>::internalCount()
{ return header.internalCount; }

/*
Method ~leafCount~:

*/
template <class THeader, class TTreeManager>
unsigned
gtaf::Tree<THeader, TTreeManager>::leafCount()
{ return header.leafCount; }

/*
Method ~entryCount~:

*/
template <class THeader, class TTreeManager>
unsigned
gtaf::Tree<THeader, TTreeManager>::entryCount()
{ return header.entryCount; }

/*
Method ~height~:

*/
template <class THeader, class TTreeManager>
unsigned
gtaf::Tree<THeader, TTreeManager>::height()
{ return header.height; }

/*
Method ~openTrees~:

*/
template <class THeader, class TTreeManager>
unsigned
gtaf::Tree<THeader, TTreeManager>::openTrees()
{ return ObjCounter<generalTrees>::openObjects(); }

/*
Method ~openNodes~:

*/
template <class THeader, class TTreeManager>
unsigned
gtaf::Tree<THeader, TTreeManager>::openNodes()
{ return ObjCounter<generalTreeNodes>::openObjects(); }

/*
Method ~openEntries~:

*/
template <class THeader, class TTreeManager>
unsigned
gtaf::Tree<THeader, TTreeManager>::openEntries()
{ return ObjCounter<generalTreeEntries>::openObjects(); }

/*
Method ~addNodePrototype~:

*/
template <class THeader, class TTreeManager>
void
gtaf::Tree<THeader, TTreeManager>::addNodePrototype(NodeBase* node)
{ nodeSupp->addPrototype(node); }

/*
Method ~createNode~:

*/
template <class THeader, class TTreeManager>
NodePtr
gtaf::Tree<THeader, TTreeManager>::
        createNode(NodeTypeId type, unsigned level)
{ return treeMngr->createNode(type, level); }

/*
Method ~createNeighbourNode~:

*/
template <class THeader, class TTreeManager>
NodePtr
gtaf::Tree<THeader, TTreeManager>::
        createNeighbourNode(NodeTypeId type)
{ return treeMngr->createNeighbourNode(type); }

/*
Method ~creatRoot~:

*/
template <class THeader, class TTreeManager>
NodePtr
gtaf::Tree<THeader, TTreeManager>::createRoot(NodeTypeId type)
{ return treeMngr->createNode(type, header.height); }

/*
Method ~createLeaf~:

*/
template <class THeader, class TTreeManager>
NodePtr
gtaf::Tree<THeader, TTreeManager>::createLeaf(NodeTypeId type)
{ return treeMngr->createNode(type, 0); }

/*
Method ~getNode~:

*/
template <class THeader, class TTreeManager>
NodePtr
gtaf::Tree<THeader, TTreeManager>::
        getNode(SmiRecordId nodeId, unsigned level) const
{ return treeMngr->getNode(nodeId, level); }

} // end namespace gtaf
#endif // #ifndef __GENERAL_TREE_H
