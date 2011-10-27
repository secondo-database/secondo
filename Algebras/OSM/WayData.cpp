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
~WayData~.

For more detailed information see WayData.h.

2 Defines and Includes

*/
// [...]
#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "WayData.h"
#include "OsmImportOperator.h"
#include <iostream>

// --- Constructors
// Constructor
WayData::WayData ()
  : m_id (0),
    m_refs (),
    m_highway (OsmImportOperator::getUndefinedStr ()),
    m_name (OsmImportOperator::getUndefinedStr ()),
    m_maxSpeed (0),
    m_oneWay (0),
    m_layer (0),
    m_bridge (OsmImportOperator::getUndefinedStr ()),
    m_tunnel (OsmImportOperator::getUndefinedStr ()),
    m_ref (OsmImportOperator::getUndefinedStr ())
{
   // empty
}

// Destructor
WayData::~WayData ()
{
   // empty
}

// --- Methods
const int & WayData::getId () const
{
    return m_id;
}

void WayData::setId (const int &id)
{
    m_id = id;
}

const std::vector<int> & WayData::getRefs () const
{
    return m_refs;
}

void WayData::addRef (const int &ref)
{
    m_refs.push_back (ref);
}

void WayData::print () const
{
    printWay (*this);
}

const std::string & WayData::getHighway () const{
    return m_highway;
}

const std::string & WayData::getName () const{
    return m_name;
}

const int & WayData::getMaxSpeed () const{
    return m_maxSpeed;
}

const int & WayData::getOneWay () const{
    return m_oneWay;
}

const int & WayData::getLayer () const{
    return m_layer;
}

const std::string & WayData::getBridge () const{
    return m_bridge;
}

const std::string & WayData::getTunnel () const{
    return m_tunnel;
}

const std::string & WayData::getRef () const{
    return m_ref;
}

void WayData::setHighway (const std::string & highway)
{
    m_highway = highway;
}

void WayData::setName (const std::string & name)
{
    m_name = name;
}

void WayData::setMaxSpeed (const int & maxSpeed)
{
    m_maxSpeed = maxSpeed;
}

void WayData::setOneWay (const int & oneWay)
{
    m_oneWay = oneWay;
}

void WayData::setLayer (const int & layer)
{
    m_layer = layer;
}

void WayData::setBridge (const std::string & bridge)
{
    m_bridge = bridge;
}

void WayData::setTunnel (const std::string & tunnel)
{
    m_tunnel = tunnel;
}

void WayData::setRef (const std::string & ref)
{
    m_ref = ref;
}

const std::vector<std::string> & WayData::getValues ()
{
    m_values.clear ();
    m_values.push_back (OsmImportOperator::convIntToStr (getId ()));
    m_values.push_back (OsmImportOperator::convIntToStr (getMaxSpeed ()));
    m_values.push_back (OsmImportOperator::convIntToStr (getOneWay ()));
    m_values.push_back (OsmImportOperator::convIntToStr (getLayer ()));
    m_values.push_back (getHighway ());
    m_values.push_back (getName ());
    m_values.push_back (getBridge ());
    m_values.push_back (getTunnel ());
    m_values.push_back (getRef ());
    m_values.push_back (OsmImportOperator::convIntVecToStr (getRefs ()));
    return m_values; 
}

std::ostream &operator<<(std::ostream &ostr, const WayData &way)
{
    ostr << "WAY Id = " << way.getId ();
    ostr << ", Highway = " << way.getHighway ();
    ostr << ", Name = " << way.getName ();
    ostr << ", MaxSpeed = " << way.getMaxSpeed ();
    ostr << ", OneWay = " << way.getOneWay ();
    ostr << ", Layer = " << way.getLayer ();
    ostr << ", Bridge = " << way.getBridge ();
    ostr << ", Tunnel = " << way.getTunnel ();
    ostr << ", Ref = " << way.getRef ();
    std::vector<int> const & refs = way.getRefs ();
    std::vector<int>::const_iterator itRef;
    for (itRef = refs.begin (); itRef != refs.end (); ++itRef)  {
        ostr << ", NodeRef = " << (*itRef);
    }
    return ostr;
}

void printWay (const WayData &way)
{
   std::cout << way << std::endl;
}
