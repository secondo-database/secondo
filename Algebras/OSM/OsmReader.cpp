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
~OsmReader~.

For more detailed information see OsmReader.h.

2 Defines and Includes

*/

#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "OsmReader.h"
#include "XmlFileReader.h"
#include "TagData.h"
#include "NdData.h"
#include "BitOperations.h"
#include <StringUtils.h>
#include <sstream>
#include <cassert>

// --- Class-variables
const int OsmReader::IN_NODE = 1;
const int OsmReader::IN_TAG = 2;
const int OsmReader::IN_WAY = 3;
const int OsmReader::IN_RELATION = 4;
const int OsmReader::IN_ND = 5;

// --- Constructors
// Default-Constructor
OsmReader::OsmReader ()
  : XmlParserInterface (), m_fileName (), m_readerState (ReaderStateUnknown),
    m_currentNode (), m_currentWay (), m_currentRestriction ()
{
    // empty
}

// Constructor
OsmReader::OsmReader (const std::string &fileName)
  : XmlParserInterface (), m_fileName (fileName),
    m_readerState (ReaderStateUnknown), m_currentNode (), 
    m_currentWay (), m_currentRestriction ()
{
    // empty
}

// Destructor
OsmReader::~OsmReader ()
{
    // empty
}

// --- Methods
void OsmReader::setFileName (const std::string &fileName)
{
    m_fileName = fileName;
}

const std::string & OsmReader::getFileName () const
{
    return m_fileName;
}

void OsmReader::readOsmFile ()
{
    std::cout << "Hello, World!" << std::endl;//TEST
    XmlFileReader reader (getFileName ());
    reader.setXmlParser (this);
    reader.readXmlFile ();
}

int OsmReader::convStrToInt (const std::string &str)
{
    //string strQuery = "num2string(" + str + ")";
    //Word result;
    //int worked = QueryProcessor::ExecuteQuery(strQuery, result);
    //assert(worked);
    //CcInt res = (Relation*)result.addr;
    //return res.getValue (); 
    std::istringstream stream (str);
    int ret;
    stream.precision (10);
    stream >> ret;
    return ret;
}

double OsmReader::convStrToDbl (const std::string &str)
{
    //string strQuery = "num2string(" + str + ")";
    //Word result;
    //int worked = QueryProcessor::ExecuteQuery(strQuery, result);
    //assert(worked);
    //CcInt res = (Relation*)result.addr;
    //return res.getValue ();
    std::istringstream stream (str);
    double ret;
    stream.precision (10);
    stream >> ret;
    return ret;
}

void OsmReader::createNodeFromElement (const Element &element)
{
    m_currentNode = NodeData ();
    //m_currentNode.print ();
}

void OsmReader::updateNodeFromElement (const Element &element)
{
    bool foundId = false;
    bool foundLat = false;
    bool foundLon = false;
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
        if ((*itAttrNames) == "id")  {
            m_currentNode.setId (convStrToInt (*itAttrValues));
            foundId = true;
        } else if ((*itAttrNames) == "lat")  {
            m_currentNode.setLat (convStrToDbl (*itAttrValues));
            foundLat = true;
        } else if ((*itAttrNames) == "lon")  {
            m_currentNode.setLon (convStrToDbl (*itAttrValues));
            foundLon = true;
        }
        if (foundId && foundLat && foundLon)  {
            break;
        }
    }
    //std::cout << "updateNodeFromElement () - ";//TEST
    //m_currentNode.print ();//TEST
}

void OsmReader::addTagElementToNode (const Element &element)
{
    TagData amenity = TagData::createTagFromElement (element, "amenity");
    TagData name = TagData::createTagFromElement (element, "name");
    if (amenity.getValue () != "")
        m_currentNode.setAmenity (amenity.getValue ());
    if (name.getValue () != "")
        m_currentNode.setName (name.getValue ());
    //std::cout << "addTagElementToNode () - ";//TEST
    //m_currentNode.print ();//TEST
}

void OsmReader::createWayFromElement (const Element &element)
{
    m_currentWay = WayData ();
    //m_currentWay.print ();
}

void OsmReader::updateWayFromElement (const Element &element)
{
    bool foundId = false;
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
        if ((*itAttrNames) == "id")  {
            m_currentWay.setId (convStrToInt (*itAttrValues));
            foundId = true;
        }
        if (foundId)  {
            break;
        }
    }
    //std::cout << "updateWayFromElement () - ";//TEST
    //m_currentWay.print ();//TEST
}

void OsmReader::addNdElementToWay (const Element &element)
{
    NdData nd = NdData::createNdFromElement (element);
    m_currentWay.addRef (nd.getRef ());
}

void OsmReader::addTagElementToWay (const Element &element)
{
    TagData highway = TagData::createTagFromElement (element, "highway");
    TagData name = TagData::createTagFromElement (element, "name");
    TagData maxSpeed = TagData::createTagFromElement (element, "maxspeed");
    TagData oneWay = TagData::createTagFromElement (element, "oneway");
    TagData layer = TagData::createTagFromElement (element, "layer");
    TagData bridge = TagData::createTagFromElement (element, "bridge");
    TagData tunnel = TagData::createTagFromElement (element, "tunnel");
    TagData ref = TagData::createTagFromElement (element, "ref");
    if (highway.getValue () != "")
        m_currentWay.setHighway (highway.getValue ());
    if (name.getValue () != "")
        m_currentWay.setName (name.getValue ());
    if (maxSpeed.getValue () != "")
        m_currentWay.setMaxSpeed (convStrToInt (maxSpeed.getValue ()));
    if (oneWay.getValue () != "")
        m_currentWay.setOneWay (convStrToInt (oneWay.getValue ()));
    if (layer.getValue () != "")
        m_currentWay.setLayer (convStrToInt (layer.getValue ()));
    if (bridge.getValue () != "")
        m_currentWay.setBridge (bridge.getValue ());
    if (tunnel.getValue () != "")
        m_currentWay.setTunnel (tunnel.getValue ());
    if (ref.getValue () != "")
        m_currentWay.setRef (ref.getValue ());
    //std::cout << "addTagElementToWay () - ";//TEST
    //m_currentWay.print ();//TEST
}

void OsmReader::prepareElement (const Element &element)
{
    if (element.getLevel () == 1 && element.getName () == "node")  {
        createNodeFromElement (element);
    } else if (element.getLevel () == 1 && element.getName () == "way")  {
        createWayFromElement (element);
    } else if (element.getLevel () == 1 && element.getName () == "relation")  {
    } 
}

void OsmReader::finalizeElement (const Element &element)
{
    int const & curState = getReaderState ();
    if (element.getLevel () == 1 && element.getName () == "node"/* &&
        is_bit_set(curState,IN_NODE) != 0*/)  {
        updateNodeFromElement (element);
        m_currentNode.print ();//TEST
    } else if (element.getLevel () == 2 && element.getName () == "tag" &&
               is_bit_set(curState,IN_NODE) != 0)  {
        addTagElementToNode (element); 
    } else if (element.getLevel () == 1 && element.getName () == "way")  {
        updateWayFromElement (element);
        m_currentWay.print ();//TEST
    } else if (element.getLevel () == 2 && element.getName () == "nd" &&
               is_bit_set(curState,IN_WAY) != 0)  {
        addNdElementToWay (element); 
    } else if (element.getLevel () == 2 && element.getName () == "tag" &&
               is_bit_set(curState,IN_WAY) != 0)  {
        addTagElementToWay (element); 
    } else if (element.getLevel () == 1 && element.getName () == "relation")  {
    } 
}

const int & OsmReader::getReaderState () const
{
    return m_readerState;
}

void OsmReader::setReaderState (const int & readerState)
{
    m_readerState = readerState;
}

void OsmReader::updateState (const Element &element, bool up)
{
    int const & curState = getReaderState ();
    int newState = curState;

    if (up) {
        //std::cout << "state up" << std::endl;//TEST
        //std::cout << "current state: " << curState << std::endl;//TEST

        if (element.getLevel () == 1 && element.getName () == "node")  {
            // /node
            newState = ReaderStateInNode;
        } else if (element.getLevel () == 1 &&
                element.getName () == "way")  {
            // /way
            newState = ReaderStateInWay;
        } else if (element.getLevel () == 1 &&
                element.getName () == "relation")  {
            // /relation
            newState = ReaderStateInRelation;
        } else if (element.getLevel () == 2 &&
                element.getName () == "tag" &&
                is_bit_set(curState,IN_NODE) != 0)  {
            // /node/tag
            set_bit(newState,IN_TAG);
            assert (newState == ReaderStateInNodeTag);
        } else if (element.getLevel () == 2 &&
                element.getName () == "nd" &&
                is_bit_set(curState,IN_WAY) != 0)  {
            // /way/nd
            set_bit(newState,IN_ND);
            assert (newState == ReaderStateInWayNd);
        } else if (element.getLevel () == 2 &&
                element.getName () == "tag" &&
                is_bit_set(curState,IN_WAY) != 0)  {
            // /way/tag
            set_bit(newState,IN_TAG);
            assert (newState == ReaderStateInWayTag);
        } else  {
            assert (false);
        }
    } else  {
        //std::cout << "state down" << std::endl;//TEST
        if (curState == ReaderStateInNodeTag)  {
            // cur: /node/tag
            // new: /node
            unset_bit(newState,IN_TAG);
            assert (newState == ReaderStateInNode);
        } else if (curState == ReaderStateInWayNd)  {
            // cur: /way/nd
            // new: /way
            unset_bit(newState,IN_ND);
            assert (newState == ReaderStateInWay);
        } else if (curState == ReaderStateInWayTag)  {
            // cur: /way/tag
            // new: /way
            unset_bit(newState,IN_TAG);
            assert (newState == ReaderStateInWay);
        } else if (curState == ReaderStateInNode ||
                   curState == ReaderStateInWay ||
                   curState == ReaderStateInRelation )  {
            newState = ReaderStateUnknown;
        } else {
            assert (false);
        }
    }
    //std::cout << "new state: " << newState << std::endl;//TEST
    //element.print ();//TEST

    // Propagating the state
    setReaderState (newState);
}

void OsmReader::pushedElementToStack (const Element &element)
{
    prepareElement (element);
    updateState (element, true);
}

void OsmReader::poppedElementFromStack (const Element &element)
{
    finalizeElement (element);
    updateState (element, false);
}

bool OsmReader::isElementInteresting (const Element &element) const
{
    return (element.getLevel () == 1 && element.getName () == "node") ||
        (element.getLevel () == 1 && element.getName () == "way") ||
        (element.getLevel () == 1 && element.getName () == "relation") ||
        (element.getLevel () == 2 && element.getName () == "tag" &&
         is_bit_set(getReaderState (),IN_NODE) != 0) ||
        (element.getLevel () == 2 && element.getName () == "nd" &&
         is_bit_set(getReaderState (),IN_WAY) != 0) ||
        (element.getLevel () == 2 && element.getName () == "tag" &&
         is_bit_set(getReaderState (),IN_WAY) != 0);
}

