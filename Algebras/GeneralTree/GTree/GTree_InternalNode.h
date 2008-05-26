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

1.1 Headerfile "GTree[_]InternalNode.h"[4]

January-May 2008, Mirko Dibbert

*/
#ifndef __GTREE_INTERNAL_NODE_H__
#define __GTREE_INTERNAL_NODE_H__

#include "GTree_GenericNodeBase.h"

namespace gtree
{

/********************************************************************
Class ~InternalNode~

This class should be used as base for all internal nodes.

********************************************************************/
template<class TEntry>
class InternalNode
    : public GenericNodeBase<TEntry>
{

public:
/*
Default constructor.

*/
    inline InternalNode(NodeConfigPtr config, unsigned emptySize = 0)
            : GenericNodeBase<TEntry>(config, emptySize)
    {}

/*
Default copy constructor.

*/
    inline InternalNode(const InternalNode& node)
            : GenericNodeBase<TEntry>(node)
    {}

/*
Virtual destructor.

*/
    inline virtual ~InternalNode()
    {}

/*
Returns a reference to a copy of the node.

*/
    virtual InternalNode *clone() const
    { return new InternalNode(*this); }

/*
Returns the record id of the i-th chield node.

*/
    virtual SmiRecordId chield(unsigned i) const
    { return (GenericNodeBase<TEntry>::entry(i))->chield(); }

/*
Returns false, to indicate that this node is an internal node.

*/
    virtual bool isLeaf() const
    { return false; }
};

} // namespace gtree
#endif // #define __GTREE_INTERNAL_NODE_H__
