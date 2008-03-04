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

1.1 Implementation file "GTAF[_]NodeSupp.cpp"[4]

January-February 2008, Mirko Dibbert
\\[3ex]
This file implements the "NodeSupp"[4] class.

*/
#include "GTAF_NodeSupp.h"

using namespace gtaf;

/*
Constructor:

*/
NodeSupp::NodeSupp(SmiRecordFile* treeFile)
        : m_file(treeFile)
{}

/*
Destructor:

*/
NodeSupp::~NodeSupp()
{
    for (iterator iter = m_prototypes.begin();
            iter != m_prototypes.end(); ++iter)
    {
        delete iter->second;
    }
}

/*
Method ~copyPrototypes~:

*/
void
NodeSupp::copyPrototypes(NodeSupp* src)
{
    for (size_t i = 0; i < src->m_prototypes.size(); ++i)
        addPrototype(src->m_prototypes[i]->clone());
}


/*
Method ~createNode~:

*/
NodeBase*
NodeSupp::createNode(NodeTypeId type)
{
    iterator iter = m_prototypes.find(type);

    if (iter != m_prototypes.end())
    {
        return iter->second->clone();
    }
    else // node type not found in prototype map
    {
        Msg::undefinedNodeType_Error(type);
        return 0;
    }
}
