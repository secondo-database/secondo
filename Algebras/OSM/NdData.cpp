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
~NdData~.

For more detailed information see NdData.h.

2 Defines and Includes

*/
// [...]
#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "NdData.h"
#include "OsmImportOperator.h"
#include <iostream>
#include <cassert>

// --- Constructors
// Constructor
NdData::NdData ()
  : m_ref (0)
{
   // empty
}

// Destructor
NdData::~NdData ()
{
   // empty
}

// --- Class-function
NdData NdData::createNdFromElement (const Element &element)
{
    NdData nd;
    std::vector<std::string>::const_iterator itAttrNames;
    std::vector<std::string>::const_iterator itAttrValues;
    std::vector<std::string> const & attributeNames =
        element.getAttributeNames ();
    std::vector<std::string> const & attributeValues =
        element.getAttributeValues ();
    assert (attributeNames.size () == attributeValues.size ());
    for (itAttrNames = attributeNames.begin (),
            itAttrValues = attributeValues.begin ();
            itAttrNames != attributeNames.end ();
            ++itAttrNames,++itAttrValues)  {
        if ((*itAttrNames) == "ref")  {
            nd.setRef (OsmImportOperator::convStrToInt (*itAttrValues));
            break;
        }
    }
    return nd;
}

// --- Methods
void NdData::setRef (const int & ref)
{
    m_ref = ref;
}

const int & NdData::getRef () const
{
    return m_ref;
}

void NdData::print () const
{
    printNd (*this);
}

std::ostream &operator<<(std::ostream &ostr, const NdData &nd)
{
    ostr << "Ref = " << nd.getRef ();
    return ostr;
}

void printNd (const NdData &nd)
{
    std::cout << nd;
}
