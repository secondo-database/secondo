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

SECONDO is free software; you can redistribute iter and/or modify
iter under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that iter will be useful,
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

1.1 Headerfile "GTAF[_]NodeSupp.h"[4]

January-February 2008, Mirko Dibbert

1.1.1 Includes and defines

*/
#ifndef __GTAF_NODE_SUPP_H
#define __GTAF_NODE_SUPP_H

#include "GTAF_Base.h"

namespace gtaf
{

/********************************************************************
1.1.1 Class "NodeSupp"[4]

********************************************************************/
class NodeSupp
{
    typedef map<NodeTypeId, NodeBase*>::iterator iterator;

public:
/*
Constructor.

*/
    NodeSupp(SmiRecordFile* treeFile);

/*
Destructor.

*/
    ~NodeSupp();

/*
Adds a new prototype node.

*/
    inline void addPrototype(NodeBase* node);

/*
Returns a copy of the prototype node with id "type"[4]. If such a node does not exist, it prints an error message and halts the dbms or returns 0 (if assertions are disabled).

*/
    NodeBase* createNode(NodeTypeId type);

/*
Adds all prototype nodes from the "src"[4] objects to this object (used in the "Tree"[4] copy constructor).

*/
    void copyPrototypes(NodeSupp* src);

/*
Returns a reference to the tree file.

*/
    inline SmiRecordFile* file() const;

private:
    SmiRecordFile* m_file;
    // reference to the tree file

    map<NodeTypeId, NodeBase*> m_prototypes;
    // prototypes for all registered nodes
};

/********************************************************************
1.1.1 Implementation of class "NodeSupp"[4] (inline methods)

********************************************************************/
/*
Method ~addPrototype~:

*/
void
NodeSupp::addPrototype(NodeBase* node)
{
    m_prototypes[node->typeId()] = node;
}

/*
Method ~file~:

*/
SmiRecordFile*
NodeSupp::file() const
{
    return m_file;
}

} // namespace gtaf
#endif // #ifdef __GTAF_NODE_SUPP_H
