/*
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

1 Header File: ThreadManager

May 2002, Ulrich Telle. Port to C++.

1.1 Overview

This module manages a set of databases. A database consists of a set of 
named types and a set of objects with given type name or type expressions. 
Objects can be persistent or not. Persistent objects are implemented 
by the ~Storage Management Interface~. When a database is opened, 
a catalog with informations about types, type constructors, operators, 
and objects of the database is loaded. Furthermore the catalog is loaded 
into memory by calling the procedures of the module ~Algebra Manager~.

1.2 Imports

*/

#ifndef SECONDO_PARTHREAD_PARNODEINFO_H
#define SECONDO_PARTHREAD_PARNODEINFO_H

namespace parthread
{

class ExecutionContext;
class ExecutionContextEntity;
class ConcurrentTupleBufferReader;

class ParNodeInfo
{
public:
    ParNodeInfo(parthread::ExecutionContext *connectedContext)
        : m_reader(0), m_currentEntity(0), m_connectedContext(connectedContext)
    {
    }

     //nothing to delete, other codeparts are responsible for deleting the 
     //members
    ~ParNodeInfo() = default;
    
    //reference to the context related to the current entity
    ExecutionContext *ConnectedContext()
    {
        return m_connectedContext;
    }

    //gets or sets a reference to the current entity of this par node
    ExecutionContextEntity *CurrentEntity()
    {
        return m_currentEntity;
    }

    void CurrentEntity(ExecutionContextEntity *entity)
    {
        m_currentEntity = entity;
    }
    
    //gets or sets the tuple reader for this par node and entity
    ConcurrentTupleBufferReader *TupleReader()
    {
        return m_reader;
    }

    void TupleReader(ConcurrentTupleBufferReader *reader)
    {
        m_reader = reader;
    }

private:
    ConcurrentTupleBufferReader *m_reader;
    ExecutionContextEntity *m_currentEntity;
    ExecutionContext *m_connectedContext;

};

} // namespace parthread
#endif
