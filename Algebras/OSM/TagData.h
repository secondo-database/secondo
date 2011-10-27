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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Header File of the TagData

June-November, 2011. Thomas Uchdorf

[TOC]

1 Overview

This header file essentially contains the definition of the class
~TagData~.

2 Defines and includes

*/
// [...]
#ifndef __TAG_DATA_H__
#define __TAG_DATA_H__

// --- Including header-files
#include "Element.h"
#include <string>

class TagData {

public:

    // --- Constructors
    // Constructor
    TagData ();
    // Destructor
    ~TagData ();

    // Class-functions
    static TagData createTagFromElement (const Element &element,
        const std::string &key);

    // --- Methods
    void setKey (const std::string & key);

    void setValue (const std::string & value);

    const std::string & getKey () const;

    const std::string & getValue () const;

    void print () const;
protected:

    // --- Members
    std::string m_key;
    
    std::string m_value;
};

std::ostream &operator<<(std::ostream &ostr, const TagData &tag);

void printTag (const TagData &tag);

#endif /*__TAG_DATA_H__ */
