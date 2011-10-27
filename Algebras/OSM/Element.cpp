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
~Element~.

For more detailed information see Element.h.

2 Defines and Includes

*/
// [...]
#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "Element.h"
#include <iostream>
#include <cassert>

// --- Constructors
// Default-Constructor
Element::Element ()
  : m_name (), m_attributeNames (), m_attributeValues (), m_value (),
    m_level (0)
{
   // empty
}
// Constructor
Element::Element (std::string name, 
                  std::vector<std::string> attributeNames,
                  std::vector<std::string> attributeValues,
                  std::string value,
                  int level)
  : m_name (name), m_attributeNames (attributeNames),
  m_attributeValues (attributeValues), m_value (value), m_level (level)
{
   // empty
}
// Destructor
Element::~Element ()
{
   // empty
}

// --- Methods
void Element::setName (const std::string &name)
{
    m_name = name;
}

void Element::setValue (const std::string &value)
{
    m_value = value;
}

void Element::setAttributeNames (const std::vector<std::string> &attributeNames)
{
    m_attributeNames = attributeNames;
}

void Element::setAttributeValues (const std::vector<std::string> &
    attributeValues)
{
    m_attributeValues = attributeValues;
}

void Element::setLevel (const int &level)
{
    m_level = level;
}

const std::string & Element::getName () const
{
    return m_name;
}

const std::string & Element::getValue () const
{
    return m_value;
}

const std::vector<std::string> & Element::getAttributeNames () const
{
    return m_attributeNames;
}

const std::vector<std::string> & Element::getAttributeValues () const
{
    return m_attributeValues;
}

const int & Element::getLevel () const
{
    return m_level;
}

void Element::print () const
{
    printElement (*this);
}

void printElement (const Element &element)
{
    // Printing the processed element
    std::cout << element.getName () << std::endl;
    std::cout << "=" << element.getValue () << std::endl;
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
        std::cout << "- " << (*itAttrNames) << " = " << (*itAttrValues);
        std::cout << std::endl;
    }
}
