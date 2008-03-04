/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% This file belongs to the GeneralTreeAlgebra framework (GTAF)           %
% Class descriptions and usage details could be found in gtaf.pdf        %
%                                                                        %
% (if this file does not exist, use "make docu" in the parent directory)
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

1.1 Headerfile "GTAF[_]NodeConfig.h"[4]

January-February 2008, Mirko Dibbert

1.1.1 Includes and defines

*/
#ifndef __GTAF_NODE_CONFIG_H
#define __GTAG_NODE_CONFIG_H

#include "GTAF_Errors.h"

namespace gtaf
{

/********************************************************************
1.1.1 Class "NodeConfig"[4]

********************************************************************/

class NodeConfig
{

public:
/*
Constructor (setting minEntries to 0 would lead to nodes of (potentially) unlimmeted size).

*/
    NodeConfig(NodeTypeId type,
               unsigned priority = 0,
               unsigned minEntries = 2,
               unsigned maxEntries = numeric_limits<unsigned>::max(),
               unsigned minPages = 1,
               unsigned maxPages = 1,
               bool cacheable = true);

/*
Destructor

*/
    inline ~NodeConfig()
    {}

/*
Returns the type-id of the node.

*/
    inline NodeTypeId type() const;

/*
Returns the priority of the node.

*/
    inline unsigned priority() const;

/*
Returns the minimum count of entries in the node.

*/
    inline unsigned minEntries() const;

/*
Returns the maximum count of entries in node.

*/
    inline unsigned maxEntries() const;

/*
Returns the minimum count of pages of the node.

*/
    inline unsigned minPages() const;

/*
Returns the maximum count of pages of the node.

*/
    inline unsigned maxPages() const;

/*
Returns "true"[4] if the node could be stored in the node cache.

*/
    inline bool cacheable() const;

private:
    NodeTypeId m_type;     // typeid of the node
    unsigned m_priority;   // priority of the node
    unsigned m_minEntries; // min. count of entries per node
    unsigned m_maxEntries; // max. count of entries per node
    unsigned m_minPages;   // min. size of the node in memory pages
    unsigned m_maxPages;   // max. size of the node in memory pages
    bool m_cacheable;      // cacheable flag
};

/********************************************************************
1.1.1 Implementation of class "NodeConfig"[4] (inline methods)

********************************************************************/
/*
Method ~type~

*/
NodeTypeId
NodeConfig::type() const
{ return m_type; }

/*
Method ~priority~

*/
unsigned
NodeConfig::priority() const
{ return m_priority; }

/*
Method ~minEntries~

*/
unsigned
NodeConfig::minEntries() const
{ return m_minEntries; }

/*
Method ~maxEntries~

*/
unsigned
NodeConfig::maxEntries() const
{ return m_maxEntries; }

/*
Method ~minPages~

*/
unsigned
NodeConfig::minPages() const
{ return m_minPages; }

/*
Method ~maxPages~

*/
unsigned
NodeConfig::maxPages() const
{ return m_minPages; }

/*
Method ~cacheable~

*/
bool
NodeConfig::cacheable() const
{ return m_cacheable; }

} // namespace gtaf
#endif // #ifndef __GTAF_NODE_CONFIG_H
