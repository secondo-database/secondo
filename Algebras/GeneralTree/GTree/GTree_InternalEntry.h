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

1.1 Headerfile "GTree[_]InternalEntry.h"[4]

January-May 2008, Mirko Dibbert

*/
#ifndef __GTREE_INTERNAL_ENTRY_H__
#define __GTREE_INTERNAL_ENTRY_H__

#include "GTree_EntryBase.h"
#include "GTree_FileNode.h"

namespace gtree
{

/********************************************************************
Class ~InternalEntry~

This class should be used as base class for all internal entries. The derived classes must overwrite the "read"[4], "write"[4] and "size"[4] method and call the resp. methods of the base class.

********************************************************************/
class InternalEntry
            : public EntryBase
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
    inline InternalEntry(const InternalEntry &e)
            : m_chield(e.m_chield)
    {}

/*
Destructor.

*/
    inline ~InternalEntry()
    {}

/*
Writes the entry object to buffer and increses offset.

*/
    inline void write(char *const buffer, int &offset) const
    {
        std::memcpy(buffer+offset, &m_chield, sizeof(SmiRecordId));
        offset += sizeof(SmiRecordId);
    }

/*
Reads the entry object from buffer and increses offset.

*/
    inline void read(const char *const buffer, int &offset)
    {
        std::memcpy(&m_chield, buffer+offset, sizeof(SmiRecordId));
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
    SmiRecordId chield() const
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
}; // class InternalEntry

} // namespace gtree
#endif // #define __GTREE_INTERNAL_ENTRY_H__
