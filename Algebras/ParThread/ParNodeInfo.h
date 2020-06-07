/*
---- 
This file is part of SECONDO.

Copyright (C) 2019, University in Hagen, Department of Computer Science, 
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

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[Contents] [\tableofcontents]

1 Header File: ParNodeInfo

September 2019, Fischer Thomas

1.1 Overview

The ParNodeInfo is the data structure defined for the local2 space of the ~par~-
operator.

*/

#ifndef SECONDO_PARTHREAD_PARNODEINFO_H
#define SECONDO_PARTHREAD_PARNODEINFO_H

namespace parthread
{
/*
1.2 Prototypes

*/
    class ExecutionContext;
    class ExecutionContextEntity;
    class ConcurrentTupleBufferReader;

    class ParNodeInfo
    {
    public:
/*
1.3 Initialization and destruction

The ParNodeInfo is initialized with the object of the connected  
execution context. This allows the par-operator to call the 
following subtree.

*/
        ParNodeInfo(parthread::ExecutionContext *connectedContext)
            : m_reader(0), m_currentEntity(0), 
              m_connectedContext(connectedContext)
        {
        }

        ~ParNodeInfo() = default;
/*
The destructor is not needed. Other codeparts are responsible for deleting 
the members this data structure

1.4 Properties

*/

        
        ExecutionContext *ConnectedContext()
        {
            return m_connectedContext;
        }
/*
Gets a reference to the execution context connected to this ~par~-node.
The execution context is the same for all entities of this node.

*/

        ExecutionContextEntity *CurrentEntity()
        {
            return m_currentEntity;
        }

        void CurrentEntity(ExecutionContextEntity *entity)
        {
            m_currentEntity = entity;
        }
/*
Gets and sets a reference to the entity containing the subtree where
this par node is a leaf node. 

*/

        ConcurrentTupleBufferReader *TupleReader()
        {
            return m_reader;
        }

        void TupleReader(ConcurrentTupleBufferReader *reader)
        {
            m_reader = reader;
        }
/*
Gets and sets the tuple reader. The reader is unique for this
~par~ node and instance. It represents the connection to the tuple-
buffer and is used by the par-operator to retrieve tuples from the 
connected context. 

*/

    private:
        ConcurrentTupleBufferReader *m_reader;
        ExecutionContextEntity *m_currentEntity;
        ExecutionContext *m_connectedContext;
    };

} // namespace parthread
#endif
