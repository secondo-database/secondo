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
~RestrictionData~.

For more detailed information see RestrictionData.h.

2 Defines and Includes

*/

#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "RestrictionData.h"
#include <iostream>

// --- Constructors
// Constructor
RestrictionData::RestrictionData ()
  : m_id (0), m_from (0), m_to (0), m_restriction (), m_type ()
{
    // empty
}

// Destructor
RestrictionData::~RestrictionData ()
{
    // empty
}

// --- Methods
void RestrictionData::setId (const int & id)
{
    m_id = id;
}

void RestrictionData::setFrom (const int & from)
{
    m_from = from;
}

void RestrictionData::setVia (const int & via)
{
    m_via = via;
}

void RestrictionData::setTo (const int & to)
{
    m_to = to;
}

void RestrictionData::setRestriction (const std::string & restriction)
{
    m_restriction = restriction;
}

void RestrictionData::setType (const std::string & type)
{
    m_type = type;
}

const int & RestrictionData::getId () const
{
    return m_id;
}

const int & RestrictionData::getFrom () const
{
    return m_from;
}

const int & RestrictionData::getVia () const
{
    return m_via;
}

const int & RestrictionData::getTo () const
{
    return m_to;

}

const std::string & RestrictionData::getRestriction () const
{
    return m_restriction;
}

const std::string & RestrictionData::getType () const
{
    return m_type;
}

void RestrictionData::print () const
{
    printRestriction (*this);
}

void printRestriction (const RestrictionData &restriction)
{
    std::cout << "RESTRICTION Id = " << restriction.getId ();
    std::cout << ", From = " << restriction.getFrom ();
    std::cout << ", Via = " << restriction.getVia ();
    std::cout << ", To = " << restriction.getTo ();
    std::cout << ", Restriction = " << restriction.getRestriction ();
    std::cout << ", Type = " << restriction.getType () << std::endl;
}
