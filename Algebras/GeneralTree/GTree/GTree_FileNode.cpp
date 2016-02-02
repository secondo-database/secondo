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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

1 Implementation file "GTree[_]FileNode.cpp"[4]

January-May 2008, Mirko Dibbert

*/
#include "GTree_FileNode.h"

using namespace gtree;
/*
Method ~getNodeId~:

*/
SmiRecordId
FileNode::getNodeId()
{
    // append new record, if no record exists
    if (!m_nodeId)
    {
        SmiRecord record;
        m_nodeMngr->file()->AppendRecord(m_nodeId, record);
    }

    return m_nodeId;
}

/*
Method ~put~:

*/
void
FileNode::put()
{
    if (!m_node->isModified())
        return;

    if (!m_nodeId)
        Msg::noNodeRecord_Error();

    // add/remove extension pages, if pagecount has been changed
    if (m_node->m_pagecount != (m_extPageCnt + 1))
    {
        // remove extension pages, if neccesary
        while (m_extPageCnt + 1 > m_node->m_pagecount)
        {
            m_nodeMngr->file()->DeleteRecord(
                    m_extensionId + (m_extPageCnt - 1));
            --m_extPageCnt;
        }

/*
Append node records, if neccessary (could only happen, if no records has been removed in the previously while-loop). All existing extension pages would be removed before, to assure that the extension pages are located in a continues area, to avoid the need to store the record-id of every extension record (furthermore, continues data could usually be read faster from disc than fragmented data).

*/
        if (m_extPageCnt + 1 < m_node->m_pagecount)
        {
            // remove all extension records
            for (SmiRecordId rec_no = m_extensionId;
                    rec_no < m_extensionId + m_extPageCnt; ++rec_no)
            {
                m_nodeMngr->file()->DeleteRecord(rec_no);
            }

            // add new extension records
            SmiRecordId rec_no;
            SmiRecord rec;
            m_nodeMngr->file()->AppendRecord(rec_no, rec);
            m_extensionId = rec_no;
            m_extPageCnt = 1;

            for (; m_extPageCnt < m_node->pagecount() - 1;
                    ++m_extPageCnt)
            {
                m_nodeMngr->file()->AppendRecord(rec_no, rec);
                assert(rec_no == m_extensionId + m_extPageCnt);
            }
        }

    }

    // create write buffer
    char buffer[PAGESIZE * m_node->m_pagecount];

    // write count of extension pages to buffer
    std::memcpy(buffer, &m_extPageCnt, sizeof(unsigned));
    int offset = sizeof(unsigned);

    // write id of first extension page to buffer
    std::memcpy(buffer + offset, &m_extensionId, sizeof(SmiRecordId));
    offset += sizeof(SmiRecordId);

    // write node type-id to buffer
    NodeTypeId type = m_node->typeId();
    std::memcpy(buffer + offset, &type, sizeof(size_t));
    offset += sizeof(NodeTypeId);

    // write node to buffer
    m_node->write(buffer, offset);

    // write header record
    SmiRecord record;
    m_nodeMngr->file()->SelectRecord(m_nodeId, record, SmiFile::Update);
    record.Write(buffer, PAGESIZE, 0);

    // write extension records, if exist
    for (unsigned i = 0; i < m_extPageCnt; ++i)
    {
        m_nodeMngr->file()->SelectRecord(
            m_extensionId + i , record, SmiFile::Update);
        record.Write(buffer + ((i + 1)*PAGESIZE), PAGESIZE, 0);
        offset += PAGESIZE;
    }

    // reset modified flag
    m_node->resetModified();
}

/*
Method ~get~:

*/
void
FileNode::get(SmiRecordId nodeId)
{
    // write current node to disc, if exist
    if (m_node.defined())
        put();

    assert(nodeId);

    // read header page
    char headerBuf[PAGESIZE];
    SmiRecord record;
    m_nodeMngr->file()->SelectRecord(nodeId, record, SmiFile::ReadOnly);
    record.Read(headerBuf, PAGESIZE, 0);

    // read count of extension pages
    std::memcpy(&m_extPageCnt, headerBuf, sizeof(unsigned));
    int offset = sizeof(unsigned);

    // read id of first extension page (=0, if no exensions exist)
    std::memcpy(&m_extensionId, headerBuf + offset, sizeof(SmiRecordId));
    offset += sizeof(SmiRecordId);

    // create read buffer
    char buffer[PAGESIZE * (m_extPageCnt+1)];

    // copy header buffer into buffer
    std::memcpy(buffer, headerBuf, PAGESIZE);

    // read extension pages, if exist
    for (unsigned i = 0; i < m_extPageCnt; ++i)
    {
        m_nodeMngr->file()->SelectRecord(
            m_extensionId + i, record, SmiFile::ReadOnly);
        record.Read(buffer + ((i + 1)*PAGESIZE), PAGESIZE, 0);
    }

    // read node type from buffer
    NodeTypeId typeId;
    std::memcpy(&typeId, buffer + offset, sizeof(NodeTypeId));
    offset += sizeof(NodeTypeId);

    // create new node of type "typeId"[4]
    createNode(typeId);
    m_nodeId = nodeId;

    // read the node values from buffer
    m_node->read(buffer, offset);

    // set m_pagecount and update m_size
    m_node->m_pagecount = m_extPageCnt + 1;

    // reset modified flag
    m_node->resetModified();
}

/*
Method ~drop~:

*/
void
FileNode::drop()
{
    m_node->clear();
    m_node->resetModified();

    if (!m_nodeId)
    {
        // no record(s) created or record(s) already removed
        return;
    }

    // remove extension pages
    for (SmiRecordId rec_no = m_extensionId;
            rec_no < m_extensionId + m_extPageCnt; ++rec_no)
    {
        m_nodeMngr->file()->DeleteRecord(rec_no);
    }

    // remove header page
    m_nodeMngr->file()->DeleteRecord(m_nodeId);
    m_nodeId = 0;
}



