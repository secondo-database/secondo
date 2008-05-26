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

1 The general-gree (gtree) framework

January-May 2008, Mirko Dibbert

This framework provides some functionality to build trees of various types, e.g. m-trees or x-trees (see MTreeAlgebra and XTreeAlgebra for examples). All framework classes belong to the "gtree"[4] namespace.

The framework consists of the following files and classes:

  * GTree.h (this file)\\
    includes all other framework files

  * GTree[_]Msg.h\\
    class "Msg"[4] - contains static methods to print warning and error messages

  * GTree[_]Config.h\\
    contains some constants, that could be used to configurate the framework

  * GTree[_]EntryBase.h, GTree[_]InternalEntry.h, GTree[_]LeafEntry.h\\
    theese files contain the base classes for node entries

  * GTree[_]Config.h\\
    class "NodeConfig"[4] (provides some config data for each node type)

  * GTree[_]NodeBase.h, GTree[_]GenericNodeBase.h, GTree[_]Internal[_]Node.h, GTree[_]LeafNode.h\\
    theese files contain the base classes for nodes

  * GTree[_]NodeManager.h\\
    class "NodeManager"[4] (manages node prototypes, could create new nodes of a specific type)

  * GTree[_]FileNode.h\\
    class "FileNode"[4] (wraps NodeBase pointest to extend them with a persistence mechanism)

  * GTree[_]TreeManager.h\\
    class "TreeManager"[4]

  * GTree[_]TreeManager.h\\
    class "TreeManager"[4]

  * GTree[_]Header.h\\
    class "Header"[4] (default tree header)

  * GTree[_]Tree.h\\
    class "Tree"[4] (base class for the trees)


The basic procedure to implement such trees is as follows:

  1. Define a tree class, which must be derived from "Tree"[4]. The first template parameter must refer to the header class for the tree file, which should be derived from "Header"[4]. This class will automatically be stored to the first page(s) of the file. The only restriction is, that it may not contain any dynamic members (pointers, strings, stl-types, etc), duo to the used persistence mechanism.

  2. Define the entry classes, which should be derived from "InternalEntry"[4] or "LeafEntry"[4]. Each of these classes must provide at least the following methods:

---- size_t size()
     void write(char *const buffer, int &offset) const
     void read(const char *const buffer, int &offset)
----
    The "write"[4] and "read"[4] method should copy the members to/from "buffer"[4] and increase "offset"[4]. The "size"[4] method must return the number of bytes, that the read/write methods will copy (ensure, that the "size"[4] method returns the correct value, since errors could easily happen here, in particular if further members are added later in the development process). All mentioned methods must call the respective method of the base class.

  3. Define the node classes, which should be derived from "InternalNode"[4] or "LeafNode"[4]. If no further functionality is needed, the "gtree"[4] classes could be used directly (e.g. done in the MTreeAlgebra). Otherwhise they could be extended with individual members (e.g. done in the XTreeAlgebra). If only new methods and non-persistent members should be added, the XTreeAlgebra nodes could refer as example. If new members must be stored, the read/write methods must be overwritten, and the "emptySize"[4] parameter of the base class constructor must be called with the size of the new members in bytes (default = 0). Example:\newpage

---- class myNode : public gtree::InternalNode
     {
       public:
         myNode(gtree::NodeConfigPtr config)
             : gtree::InternalNode(config, sizeof(newMember))
         {}

         myNode(const myNode &node)
            : gtree::InternalNode(node),
              newMember(node.newMember)
        {}

        virtual myNode *clone() const
        { return new myNode(*this); }

         void write(char *const buffer, int &offset) const
         {
            gtree::InternalNode::write(buffer, offset);
            memcpy(buffer, &newMember, sizeof(int));
            offset += sizeof(int);
         }

         void read(const char *const buffer, int &offset)
         {
            gtree::InternalNode::read(buffer, offset);
            memcpy(&newMember, buffer, sizeof(int));
            offset += sizeof(int);
         }

       private:
         int newMember;
     };
----

If the default node classes are not sufficient (e.g. since storing the entries into a vector), it is also possible to create complete new node types, since the default classes are not used directely anywhere in the framework. The only restriction is, that these classes must be derived from the "NodeBase"[4] class.

  4. Add node prototypes to the framework (e.g. in the constructor or an initialize method) with the "addNodePrototype" method of the "Tree"[4] class. These prototypes are used to create new nodes (by applying the copy constructor) of the respective node. If the node cache should be used, the "treeMngr->enableCache()"[4] should also be called at this point (if some node types should not be cached, set the cacheable flag of the resp. "NodeConfig"[4] object to "false"[4]). It is recommended to define a "NodeTypeId"[4] constant for each node type for easier creation of the nodes.

Now, the framework is ready to be used. For further details see the mentioned files (in particular "GTree[_]Tree.h"[4] and "GTree[_]TreeManager.h"[4]) and refer the MTree- and XTreeAlgebra for some practical examples.

Generally, the first action will be the creation of a new root node, which could easily be done by calling the "createRoot"[4] method. This method is implemented in two variants, whereas the second one allows to insert two (internal) entries into the new root, e.g. usefull if a new root must be created during node splits. The resp. counters in the "Header"[4] class  ("height"[4], "internalEntry"[4] and "leafEntry"[4]) will automatically be incremented within these methods.

To traverse the tree use "treeMngr->initPath()"[4] to init a new path. Now, the resp. "treeMngr"[4] methods ("getChield"[4], "getParent"[4], "hasChield"[4], "hasParent"[4], "curNode"[4], etc.) could be used to traverse the tree. If a node must be split, the "createNeighbourNode"[4] could be used to create a new node on the same level as "treeMngr->curNode()"[4].

The last basic one needs to know is the cast mechanism: To access node specific methods, the node object needs to be casted into the appropriate type to access the entry and specific methods. Since usually one will store "NodePtr" pointers (which is equal to "SmartPtr<FileNode>"[4]), the easiest way to do this ist the "cast"[4] method of the "FileNode"[4] class. For example, casting "NodePtr p"[4] to "MyNodeClass"[4] could be done as follows:

---- SmartPtr<MyNodeClass> myNode = p->cast<MyNodeClass>();
     MyEntryClass *e myNode->entry(i);
----
(The "entry"[4] method of the default node classes return pointers of the assigned entry type)

*/
#ifndef __GTREE_H__
#define __GTREE_H__

#include "GTree_Tree.h"
#include "GTree_LeafNode.h"
#include "GTree_InternalNode.h"

#endif // #define __GTREE_H__
