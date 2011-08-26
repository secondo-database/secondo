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
~NodeData~.

For more detailed information see NodeData.h.

2 Defines and Includes

*/

#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "NodeData.h"
#include "OsmImportOperator.h"
#include <iostream>
#include <sstream>

// --- Constructors
// Constructor
NodeData::NodeData ()
  : m_id (0), m_lon (0.), m_lat (0.),
    m_amenity (OsmImportOperator::getUndefinedStr ()),
    m_name (OsmImportOperator::getUndefinedStr ()),
    m_values ()
{
   // empty
}

// Destructor
NodeData::~NodeData ()
{
   // empty
}

// --- Methods
void NodeData::setId (const int & id)
{
    m_id = id;
}

void NodeData::setLon (const double & lon)
{
    m_lon = lon;
}

void NodeData::setLat (const double & lat)
{
    m_lat = lat;
}

void NodeData::setAmenity (const std::string & amenity)
{
    m_amenity = amenity;
}

void NodeData::setName (const std::string & name)
{
    m_name = name;
}

const int & NodeData::getId () const
{
    return m_id;
}

const double & NodeData::getLon () const
{
    return m_lon;
}

const double & NodeData::getLat () const
{
    return m_lat;
}

const std::string & NodeData::getAmenity () const
{
    return m_amenity;
}

const std::string & NodeData::getName () const
{
    return m_name;
}

const std::vector<std::string> & NodeData::getValues ()
{
    m_values.clear ();
    std::ostringstream strId;
    strId << getId ();
    std::ostringstream strLon;
    strLon << getLon ();
    std::ostringstream strLat;
    strLat << getLat ();
    m_values.push_back (strId.str ());
    m_values.push_back (strLon.str ());
    m_values.push_back (strLat.str ());
    m_values.push_back (getAmenity ());
    m_values.push_back (getName ());
    return m_values; 
}
 
void NodeData::print () const
{
    printNode (*this);
}

void printNode (const NodeData &node)
{
    std::cout << "NODE Id = " << node.getId ();
    std::cout << ", Lon = " << node.getLon ();
    std::cout << ", Lat = " << node.getLat ();
    std::cout << ", Amenity = " << node.getAmenity ();
    std::cout << ", Name = " << node.getName () << std::endl;
}
