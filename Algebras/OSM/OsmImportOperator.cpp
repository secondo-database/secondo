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

[1] Implementation of the OSM Algebra

June-November, 2011. Thomas Uchdorf

[TOC]

1 Overview

This implementation file contains the implementation of the class
~OsmImportOperator~.

For more detailed information see OsmImportOperator.h.

2 Defines and Includes

*/

#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "OsmImportOperator.h"
#include "OsmParser.h"
#include "RelationAlgebra.h"
#include "../FText/FTextAlgebra.h"
#include <iostream>

const int OsmImportOperator::ELEMENT_TYPE_UNKNOWN = 0x0;
const int OsmImportOperator::ELEMENT_TYPE_NODE = 0x1;
const int OsmImportOperator::ELEMENT_TYPE_WAY = 0x2;
const int OsmImportOperator::ELEMENT_TYPE_RESTRICTION = 0x4;

const char * OsmImportOperator::ATTRIBUTE_VALUE_UNDEFINED = "VALUE_UNDEFINED";
const char * OsmImportOperator::ATTRIBUTE_VALUE_UNKNOWN = "VALUE_UNKNOWN";

// --- Constructors
// Default-Constructor
OsmImportOperator::OsmImportOperator ()
  : m_parser (NULL), m_elementType (ELEMENT_TYPE_UNKNOWN), m_tupleType (NULL),
    m_tupleTemplate (NULL), m_attributeTemplates (NULL)
{
    // empty
}

// Constructor
OsmImportOperator::OsmImportOperator (const std::string &fileName,
    const std::string &elementType,
    ListExpr type)
  : m_parser (NULL), m_elementType (ELEMENT_TYPE_UNKNOWN), m_tupleType (NULL),
    m_tupleTemplate (NULL), m_attributeTemplates (NULL)
{
    m_parser = new OsmParser(fileName); 
    m_parser->openOsmFile ();

    if (elementType == "node")  {
        m_elementType = ELEMENT_TYPE_NODE;
    } else if (elementType == "way") {
        m_elementType = ELEMENT_TYPE_WAY;
    } else if (elementType == "restriction") {
        m_elementType = ELEMENT_TYPE_RESTRICTION;
    } else {
        assert (false);
        m_elementType = ELEMENT_TYPE_UNKNOWN;
    }

    ListExpr numType = nl->Second(
            SecondoSystem::GetCatalog()->NumericType((type)));
    m_tupleType = new TupleType(numType);
    m_tupleTemplate = new Tuple (m_tupleType);
    createAttributeTemplates (type);
}

// Destructor
OsmImportOperator::~OsmImportOperator ()
{
    if (m_parser)  {
        m_parser->closeOsmFile (); 
        delete m_parser;
    }

    if (m_tupleTemplate)  {
        delete m_tupleTemplate;
        m_tupleTemplate = NULL;
    }
    if(m_tupleType)  {
        m_tupleType->DeleteIfAllowed ();
        m_tupleType = NULL;
    }
    for (unsigned int i = 0; i < m_attributeTemplates.size (); ++i)  {
        delete m_attributeTemplates[i];
    }
    m_attributeTemplates.clear ();
}

// --- Class-functions
int OsmImportOperator::convStrToInt (const std::string &str)
{
    std::istringstream stream (str);
    int ret;
    stream.precision (10);
    stream >> ret;
    return ret;
}

double OsmImportOperator::convStrToDbl (const std::string &str)
{
    std::istringstream stream (str);
    double ret;
    stream.precision (10);
    stream >> ret;
    return ret;
}

ListExpr OsmImportOperator::getOsmNodeAttrList ()
{
    const int NUM_ATTR = 5;
    std::string attributeNames[NUM_ATTR] = {
        "id",
        "lon",
        "lat",
        "amenity",
        "name"
    }; 
    std::string attributeTypes[NUM_ATTR] = {
        CcInt::BasicType(),
        CcReal::BasicType(),
        CcReal::BasicType(),
        FText::BasicType(),
        FText::BasicType()
    }; 
    return OsmImportOperator::getAttrList (attributeNames, attributeTypes,
        NUM_ATTR);
}

ListExpr OsmImportOperator::getOsmWayAttrList ()
{
    const int NUM_ATTR = 10;
    std::string attributeNames[NUM_ATTR] = {
        "id",
        "maxSpeed",
        "oneWay",
        "layer",
        "highway",
        "name",
        "bridge",
        "tunnel",
        "ref",
        "nodeRefs"
    }; 
    std::string attributeTypes[NUM_ATTR] = {
        CcInt::BasicType(),
        CcInt::BasicType(),
        CcInt::BasicType(),
        CcInt::BasicType(),
        FText::BasicType(),
        FText::BasicType(),
        FText::BasicType(),
        FText::BasicType(),
        FText::BasicType(),
        FText::BasicType()
    }; 
    return OsmImportOperator::getAttrList (attributeNames, attributeTypes,
        NUM_ATTR);
}

ListExpr OsmImportOperator::getOsmRestrictionAttrList ()
{
    const int NUM_ATTR = 6;
    std::string attributeNames[NUM_ATTR] = {
        "id",
        "from",
        "via",
        "to",
        "restriction",
        "type"
    }; 
    std::string attributeTypes[NUM_ATTR] = {
        CcInt::BasicType(),
        CcInt::BasicType(),
        CcInt::BasicType(),
        CcInt::BasicType(),
        FText::BasicType(),
        FText::BasicType()
    }; 
    return OsmImportOperator::getAttrList (attributeNames, attributeTypes,
        NUM_ATTR);
}

ListExpr OsmImportOperator::getAttrList (std::string attributeNames [],
        std::string attributeTypes [],
        const int &NUM_ATTR)
{
    ListExpr attrList;
    ListExpr last;
    for (int i = 0; i < NUM_ATTR; ++i)  {
        std::string const & name = attributeNames[i];
        std::string const & type = attributeTypes[i];
        ListExpr attr = nl->TwoElemList(nl->SymbolAtom(name),
                nl->SymbolAtom(type));
        if(i == 0)  { // first element
            attrList = nl->OneElemList(attr);
            last = attrList;
        } else  {
            last = nl->Append(last,attr);
        }
    }
    return attrList;
}
        
const char * OsmImportOperator::getUndefinedStr ()
{
    return OsmImportOperator::ATTRIBUTE_VALUE_UNDEFINED;
}

const char * OsmImportOperator::getUnknownStr ()
{
    return OsmImportOperator::ATTRIBUTE_VALUE_UNKNOWN;
}

// --- Methods
Tuple * OsmImportOperator::getNext ()
{
    std::vector<std::string> values;
    int elementType;
    do {
        m_parser->getNext (&values, &elementType);
    } while (values.size () > 0 && elementType != m_elementType);
    if (values.size () > 0)  {
        return createTuple (values);
    } else {
        return NULL;
    }
}

Tuple * OsmImportOperator::createTuple (const vector<std::string> &values)
{
    Tuple* result = m_tupleTemplate->Clone();
    assert (values.size () == m_attributeTemplates.size ());
    vector<std::string>::const_iterator itValue;
    itValue = values.begin ();
    for(unsigned int i = 0; i < m_attributeTemplates.size (); ++i)  {
        Attribute* attr = m_attributeTemplates[i]->Clone();
        if ((*itValue) != OsmImportOperator::getUndefinedStr ())  {
            if ((*itValue) == "")  {
                attr->ReadFromString (OsmImportOperator::getUnknownStr ());
            } else  {
                attr->ReadFromString (*itValue);
            }
        } else {
            attr->SetDefined (false);
        }
        result->PutAttribute(i, attr);
        ++itValue;
    }
    return result;
}

void OsmImportOperator::createAttributeTemplates (ListExpr type)
{
    ListExpr attrList = nl->Second(nl->Second(type));
    int algebraId (0);
    int typeId (0);
    std::string typeName;
    bool ret (false);
    Word w;
    while(!nl->IsEmpty(attrList)){
        ListExpr attrType = nl->Second(nl->First(attrList));
        attrList = nl->Rest(attrList);
        ret = SecondoSystem::GetCatalog()->LookUpTypeExpr(attrType,
                typeName, algebraId, typeId);
        assert(ret);
        w = am->CreateObj(algebraId,typeId)(attrType);
        m_attributeTemplates.push_back(static_cast<Attribute*>(w.addr));
    }
}

const vector<Attribute*> & OsmImportOperator::getAttributeTemplates () const
{
    return m_attributeTemplates;
}

