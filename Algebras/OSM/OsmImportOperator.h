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

[1] Header File of the OsmImportOperator

June-November, 2011. Thomas Uchdorf

[TOC]

1 Overview

This header file essentially contains the definition of the class
~OsmImportOperator~.

2 Defines and includes

*/
#ifndef __OSM_IMPORT_OPERATOR_H__
#define __OSM_IMPORT_OPERATOR_H__

// --- Including header-files
#include <vector>
#include <string>
#include "NestedList.h"

class OsmParser;
class TupleType;
class Tuple;
class Attribute;

class OsmImportOperator
{
    public:

        // --- Constructors
        // Default-Constructor
        OsmImportOperator ();
        // Constructor
        OsmImportOperator (const std::string &fileName,
                const std::string &elementType,
                ListExpr type);
        // Destructor
        ~OsmImportOperator ();

        // --- Class-functions
        static int convStrToInt (const std::string &str); 
        static double convStrToDbl (const std::string &str); 
        static ListExpr getOsmNodeAttrList ();
        static ListExpr getOsmWayAttrList ();
        static ListExpr getOsmRestrictionAttrList ();
        static ListExpr getAttrList (std::string attributeNames [],
                std::string attributeTypes [],
                const int &NUM_ATTR);
        static const char * getUndefinedStr ();
        static const char * getUnknownStr ();

        // --- Methods
        Tuple * getNext ();
        Tuple * createTuple (const std::vector<std::string> &values);
        void createAttributeTemplates (ListExpr type);
        const vector<Attribute *> & getAttributeTemplates () const;

        // --- Class-variables
        static const int ELEMENT_TYPE_UNKNOWN;
        static const int ELEMENT_TYPE_NODE;
        static const int ELEMENT_TYPE_WAY;
        static const int ELEMENT_TYPE_RESTRICTION;
        static const char * ATTRIBUTE_VALUE_UNDEFINED;
        static const char * ATTRIBUTE_VALUE_UNKNOWN;

    protected:

        // --- Members
        OsmParser* m_parser;
        int m_elementType;
        TupleType* m_tupleType;
        Tuple * m_tupleTemplate;
        vector<Attribute *> m_attributeTemplates;

};

#endif /*__OSM_IMPORT_OPERATOR_H__ */
