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
~TagData~.

For more detailed information see TagData.h.

2 Defines and Includes

*/
// [...]
#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "TagData.h"
#include <iostream>
#include <cassert>

// --- Constructors
// Constructor
TagData::TagData ()
  : m_key (), m_value ()
{
   // empty
}

// Destructor
TagData::~TagData ()
{
   // empty
}

// --- Methods
TagData TagData::createTagFromElement (const Element &element,
                                       const std::string &key)
{
    TagData tag;
    tag.setKey (key);
    std::vector<std::string>::const_iterator itAttrNames;
    std::vector<std::string>::const_iterator itAttrValues;
    std::vector<std::string> const & attributeNames =
        element.getAttributeNames ();
    std::vector<std::string> const & attributeValues =
        element.getAttributeValues ();
    assert (attributeNames.size () == attributeValues.size ());
    if (attributeNames.size () == 2)  {
        itAttrNames = attributeNames.begin (),
         itAttrValues = attributeValues.begin ();
        if ((*itAttrNames) == "k" && (*itAttrValues) == tag.getKey ())  {
            ++itAttrNames, ++itAttrValues;
            if ((*itAttrNames) == "v")  {
                tag.setValue (*itAttrValues);
            } 
        }
    }
    return tag;
}

void TagData::setKey (const std::string & key)
{
    m_key = key;
}

void TagData::setValue (const std::string & value)
{
    m_value = value;
}

const std::string & TagData::getKey () const
{
    return m_key;
}

const std::string & TagData::getValue () const
{
    return m_value;
}

void TagData::print () const
{
    printTag (*this);
}

std::ostream &operator<<(std::ostream &ostr, const TagData &tag)
{
    ostr << "Key = " << tag.getKey ();
    ostr << ", Value = " << tag.getValue ();
    return ostr;
}

void printTag (const TagData &tag)
{
    std::cout << tag << std::endl;
}
