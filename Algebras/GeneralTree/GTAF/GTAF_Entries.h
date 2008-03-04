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

1.1 Headerfile "GTAF[_]Entries.h"[4]

January-February 2008, Mirko Dibbert

1.1.1 Includes and defines

*/
#ifndef __GTAF_ENTRIES_H
#define __GTAF_ENTRIES_H

#include "GTAF_FileNode.h"

namespace gtaf
{

/********************************************************************
1.1.1 Class InternalEntry

This class should be used as base class for internal entries.

********************************************************************/

class InternalEntry : public EntryBase
{

public:
/*
Default constructor.

*/
    inline InternalEntry(SmiRecordId chield = 0)
            : m_chield(chield)
    {}


/*
Constructor (sets chield-id to the record-id of "node"[4]).

*/
    inline InternalEntry(NodePtr node)
            : m_chield(node->getNodeId())
    {}


/*
Default copy constructor.

*/
    inline InternalEntry(const InternalEntry& e)
            : m_chield(e.m_chield)
    {}


/*
Destructor.

*/
    inline ~InternalEntry()
    {}

/*
Writes the entry object to buffer and increses offset. (should be called from inheritting classes)

*/
    inline void write(char* const buffer, int& offset) const
    {
        // write pointer to chield node
        memcpy(buffer + offset, &m_chield, sizeof(SmiRecordId));
        offset += sizeof(SmiRecordId);
    }


/*
Reads the entry object from buffer and increses offset. (should be called from inheritting classes)

*/
    inline void read(const char* const buffer, int& offset)
    {
        // write pointer to chield node
        memcpy(&m_chield, buffer + offset, sizeof(SmiRecordId));
        offset += sizeof(SmiRecordId);
    }


/*
Returns the size of the entry on disc.

*/
    inline size_t size()
    { return sizeof(SmiRecordId); }


/*
Returns the record-id of the chield node.

*/
    inline SmiRecordId chield() const
    { return m_chield; }


/*
Sets the chield record-id to "nodeId"[4].

*/
    inline void setChield(SmiRecordId nodeId)
    { m_chield = nodeId; }

/*
Sets the chield record-id to the record-id of "node"[4].

*/
    inline void setChield(NodePtr node)
    { m_chield = node->getNodeId(); }


private:
    SmiRecordId m_chield; // pointer to chield node
};

/********************************************************************
1.1.1 Class LeafEntry

This class should be used as base class for leaf entries.

********************************************************************/

class LeafEntry : public EntryBase
{

public:
/*
Default constructor.

*/
    inline LeafEntry()
    {}

/*
Default copy constructor.

*/
    inline LeafEntry(const LeafEntry& e)
    {}

/*
Destructor.

*/
    inline ~LeafEntry()
    {}

/*
Writes the entry object to buffer and increses offset. (should be called from inheritting classes)

*/
    inline void write(char* const buffer, int& offset) const
    {}

/*
Reads the entry object from buffer and increses offset. (should be called from inheritting classes)

*/
    inline void read(const char* const buffer, int& offset)
    {}


/*
Returns the size of the entry on disc.

*/
    inline size_t size()
    { return 0; }
};

}; // namespace gtaf
#endif // #ifndef __GTAF_ENTRIES_H
