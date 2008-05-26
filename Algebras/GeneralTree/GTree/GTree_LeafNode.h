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

1.1 Headerfile "GTree[_]LeafNode.h"[4]

January-May 2008, Mirko Dibbert

*/
#ifndef __GTREE_LEAF_NODE_H__
#define __GTREE_LEAF_NODE_H__

#include "GTree_GenericNodeBase.h"

namespace gtree
{

/********************************************************************
Class ~LeafNode~

This class should be used as base for all leaf nodes.

********************************************************************/
template<class TEntry>
class LeafNode
    : public GenericNodeBase<TEntry>
{

public:
/*
Default constructor.

*/
    inline LeafNode(NodeConfigPtr config, unsigned emptySize = 0)
            : GenericNodeBase<TEntry>(config, emptySize)
    {}

/*
Default copy constructor.

*/
    inline LeafNode(const LeafNode& node)
            : GenericNodeBase<TEntry>(node)
    {}

/*
Virtual destructor.

*/
    inline virtual ~LeafNode()
    {}

/*
Returns a reference to a copy of the node.

*/
    virtual LeafNode *clone() const
    { return new LeafNode(*this); }

/*
Returns 0, since leaf does not have any chield nodes.

*/
    virtual SmiRecordId chield(unsigned i) const
    { return 0; }

/*
Returns true, to indicate that this node is a leaf node.

*/
    virtual bool isLeaf() const
    { return true; }
};

} // namespace gtree
#endif // #define __GTREE_LEAF_NODE_H__
