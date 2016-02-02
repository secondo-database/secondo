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

1.1 Headerfile "GTree[_]Tree.h"[4]

January-May 2008, Mirko Dibbert

*/
#ifndef __GTREE_TREE_H__
#define __GTREE_TREE_H__

#include <stack>
#include "GTree_Header.h"
#include "GTree_FileNode.h"
#include "GTree_TreeManager.h"

namespace gtree
{

/*
Class ~Tree~

This is the base class for trees.

*/
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
Creates a new node on root level and inserts the given entries.

*/
    inline NodePtr createRoot(
            NodeTypeId type, InternalEntry *e1, InternalEntry *e2);

/*
Reads the specified node from file.

*/
    inline NodePtr getNode(
        SmiRecordId nodeId, unsigned level = 0) const;

/*
Method initPath (initiates a new tree manager path from root)

*/
    inline void initPath(SmiRecordId root, int level);

/*
Method initPath (initiates a new tree manager path from header.root)

*/
    inline void initPath();

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

protected:
    NodeManager* nodeMngr;    /* reference to the tree file and the
                                 node prototypes */
    TTreeManager* treeMngr;   // reference to the tree manager
}; // class Tree


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/*
Tree constructor (new file):

*/
template <class THeader, class TTreeManager>
gtree::Tree<THeader, TTreeManager>::Tree(bool _temporary)
        : header(), file(true, PAGESIZE), temporary(_temporary),
          nodeMngr(new NodeManager(&file)),
          treeMngr(new TTreeManager(nodeMngr))
{
#ifdef __GTREE_DEBUG
    Msg::showDbgMsg();
#endif

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
gtree::Tree<THeader, TTreeManager>::Tree(SmiFileId fileId)
        : header(), file(true), temporary(false),
        nodeMngr(new NodeManager(&file)),
        treeMngr(new TTreeManager(nodeMngr))
{
#ifdef __GTREE_DEBUG
    Msg::showDbgMsg();
#endif
    bool t1 = file.Open(fileId);
    assert(t1);

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
gtree::Tree<THeader, TTreeManager>::Tree(
    const Tree<THeader, TTreeManager>& tree)
        : file(true, PAGESIZE), temporary(tree.temporary),
        nodeMngr(new NodeManager(&file)),
        treeMngr(new TTreeManager(nodeMngr))
{
#ifdef __GTREE_DEBUG
    Msg::showDbgMsg();
#endif

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

    nodeMngr->copyPrototypes(tree.nodeMngr);

    // disable node cache, while copying the tree structure
    treeMngr->disableCache();

    std::stack<std::pair<NodePtr, NodePtr> > remaining;
    std::stack<unsigned> indizes;
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
            remaining.push(std::pair<NodePtr, NodePtr>(source, target));
            indizes.push(curIndex);
        }

        // push chield node to stack
        remaining.push(std::pair<NodePtr, NodePtr>(chield, newChield));
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
                        std::pair<NodePtr, NodePtr>(source, target));
                indizes.push(curIndex);
            }

            // push chield node to stack
            remaining.push(
                    std::pair<NodePtr, NodePtr>(chield, newChield));

            indizes.push(0);
        }
    }

    treeMngr->enableCache();
} // tree copy constructor

/*
Tree destructor:

*/
template <class THeader, class TTreeManager>
gtree::Tree<THeader, TTreeManager>::~Tree()
{
    delete treeMngr;
    delete nodeMngr;

    if (temporary)
        deleteFile();

    if (file.IsOpen())
    {
        writeHeader();
        file.Close();
    }

#ifdef __GTREE_DEBUG
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
void gtree::Tree<THeader, TTreeManager>::readHeader()
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
void gtree::Tree<THeader, TTreeManager>::writeHeader()
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
void gtree::Tree<THeader, TTreeManager>::deleteFile()
{
    if (file.IsOpen())
        file.Close();

    file.Drop();
}

/*
Method ~fileId~:

*/
template <class THeader, class TTreeManager>
SmiFileId gtree::Tree<THeader, TTreeManager>::fileId()
{ return file.GetFileId(); }

/*
Method ~internalCount~:

*/
template <class THeader, class TTreeManager>
unsigned gtree::Tree<THeader, TTreeManager>::internalCount()
{ return header.internalCount; }

/*
Method ~leafCount~:

*/
template <class THeader, class TTreeManager>
unsigned gtree::Tree<THeader, TTreeManager>::leafCount()
{ return header.leafCount; }

/*
Method ~entryCount~:

*/
template <class THeader, class TTreeManager>
unsigned gtree::Tree<THeader, TTreeManager>::entryCount()
{ return header.entryCount; }

/*
Method ~height~:

*/
template <class THeader, class TTreeManager>
unsigned gtree::Tree<THeader, TTreeManager>::height()
{ return header.height; }

/*
Method ~openTrees~:

*/
template <class THeader, class TTreeManager>
unsigned gtree::Tree<THeader, TTreeManager>::openTrees()
{ return ObjCounter<generalTrees>::openObjects(); }

/*
Method ~openNodes~:

*/
template <class THeader, class TTreeManager>
unsigned gtree::Tree<THeader, TTreeManager>::openNodes()
{ return ObjCounter<generalTreeNodes>::openObjects(); }

/*
Method ~openEntries~:

*/
template <class THeader, class TTreeManager>
unsigned gtree::Tree<THeader, TTreeManager>::openEntries()
{ return ObjCounter<generalTreeEntries>::openObjects(); }

/*
Method ~addNodePrototype~:

*/
template <class THeader, class TTreeManager>
void gtree::Tree<THeader, TTreeManager>::addNodePrototype(
        NodeBase* node)
{ nodeMngr->addPrototype(node); }

/*
Method ~createNode~:

*/
template <class THeader, class TTreeManager>
NodePtr gtree::Tree<THeader, TTreeManager>::
        createNode(NodeTypeId type, unsigned level /* = 0 */)
{
    if (level == 0)
        ++header.leafCount;
    else
        ++header.internalCount;
    return treeMngr->createNode(type, level);
}

/*
Method ~createNeighbourNode~:

*/
template <class THeader, class TTreeManager>
NodePtr gtree::Tree<THeader, TTreeManager>::
        createNeighbourNode(NodeTypeId type)
{
    if (treeMngr->curNode()->isLeaf())
        ++header.leafCount;
    else
        ++header.internalCount;

    return treeMngr->createNeighbourNode(type);
}

/*
Method ~createRoot~:

*/
template <class THeader, class TTreeManager>
NodePtr gtree::Tree<THeader, TTreeManager>::createRoot(
        NodeTypeId type)
{
    NodePtr root(treeMngr->createNode(type, header.height));
    header.root = root->getNodeId();
    if (header.height == 0)
    {
        ++header.leafCount;
    }
    else
    {
        ++header.internalCount;
    }
    ++header.height;

    return root;
}

/*
Method ~createRoot~:

*/
template <class THeader, class TTreeManager>
NodePtr gtree::Tree<THeader, TTreeManager>::createRoot(
        NodeTypeId type, InternalEntry *e1, InternalEntry *e2)
{
    NodePtr root(treeMngr->createNode(type, header.height));
    treeMngr->insert(root, e1);
    treeMngr->insert(root, e2);
    header.root = root->getNodeId();
    ++header.height;
    ++header.internalCount;

    return root;
}

/*
Method ~getNode~:

*/
template <class THeader, class TTreeManager>
NodePtr gtree::Tree<THeader, TTreeManager>:: getNode(
        SmiRecordId nodeId, unsigned level) const
{ return treeMngr->getNode(nodeId, level); }

/*
Method initPath:

*/
template <class THeader, class TTreeManager>
void gtree::Tree<THeader, TTreeManager>::initPath(
        SmiRecordId root, int level)
{
    treeMngr->initPath(root, level);
}

/*
Method initPath:

*/
template <class THeader, class TTreeManager>
void gtree::Tree<THeader, TTreeManager>::initPath()
{
    treeMngr->initPath(header.root, header.height-1);
}


} // namespace gtree
#endif // #define __GTREE_TREE_H__
