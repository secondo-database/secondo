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

[1] Header File of the XmlFileReader

June-November, 2011. Thomas Uchdorf

[TOC]

1 Overview

This header file essentially contains the definition of the class
~XmlFileReader~.

2 Defines and includes

*/
#ifndef __XML_FILE_READER_H__
#define __XML_FILE_READER_H__

// --- Including header-files
#include <string>
#include <stack>
#include "Element.h"

//#define WITH_LIBXML2_SUPPORT
#ifdef WITH_LIBXML2_SUPPORT
#include "libxml/xmlreader.h"
#endif

class XmlParserInterface;

// --- Including header-files
class XmlFileReader {

    public:

        // --- Constructors
        // Default-Constructor
        XmlFileReader ();
        // Constructor
        XmlFileReader (const std::string &fileName);
        // Destructor
        ~XmlFileReader ();

        // --- Methods
        void setFileName (const std::string &fileName);
        const std::string & getFileName () const;
        void setXmlParser (XmlParserInterface *parser);
        void readXmlFile ();

    protected:

        // --- Methods
#ifdef WITH_LIBXML2_SUPPORT
        void processXmlNode (xmlTextReaderPtr reader);
#endif
        void pushEmptyElementToStack (const Element &element);
        void pushElementToStack (const Element &element);
        void popElementFromStack (const Element &element);
        bool isElementInteresting (const Element &element) const;

        // --- Members
        std::string m_fileName;

        XmlParserInterface *m_parser;

        // Stack that only contains relevant elements
        std::stack<Element> m_elements;
};

#endif /* __XML_FILE_READER_H__ */
