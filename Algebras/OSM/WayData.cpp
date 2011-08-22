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

#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "WayData.h"
#include <iostream>

// --- Constructors
// Constructor
WayData::WayData ()
  : m_id (0), m_refs (), m_highway (), m_name (), m_maxSpeed (0), m_oneWay (0),
    m_layer (0), m_bridge (), m_tunnel (), m_ref ()
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

void printWay (const WayData &way)
{
    std::cout << "WAY Id = " << way.getId ();
    std::cout << ", Highway = " << way.getHighway ();
    std::cout << ", Name = " << way.getName ();
    std::cout << ", MaxSpeed = " << way.getMaxSpeed ();
    std::cout << ", OneWay = " << way.getOneWay ();
    std::cout << ", Layer = " << way.getLayer ();
    std::cout << ", Bridge = " << way.getBridge ();
    std::cout << ", Tunnel = " << way.getTunnel ();
    std::cout << ", Ref = " << way.getRef ();
    std::vector<int> const & refs = way.getRefs ();
    std::vector<int>::const_iterator itRef;
    for (itRef = refs.begin (); itRef != refs.end (); ++itRef)  {
        std::cout << ", NodeRef = " << (*itRef);
    }
    std::cout << std::endl;
}
