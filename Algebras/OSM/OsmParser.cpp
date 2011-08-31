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
~OsmParser~.

For more detailed information see OsmParser.h.

2 Defines and Includes

*/

#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "OsmParser.h"
#include "OsmImportOperator.h"
#include "XmlFileReader.h"
#include "TagData.h"
#include "NdData.h"
#include "MemberData.h"
#include "BitOperations.h"
#include "ScalingEngine.h"
#include <StringUtils.h>
#include <sstream>
#include <cassert>

// --- Class-variables
const int OsmParser::IN_NODE = 1;
const int OsmParser::IN_TAG = 2;
const int OsmParser::IN_WAY = 3;
const int OsmParser::IN_RELATION = 4;
const int OsmParser::IN_ND = 5;
const int OsmParser::IN_MEMBER = 6;

// --- Constructors
// Default-Constructor
OsmParser::OsmParser ()
  : XmlParserInterface (), m_fileName (), m_readerState (ReaderStateUnknown),
    m_currentNode (), m_currentWay (), m_currentRestriction (), m_reader (NULL),
    m_foundNode (false), m_foundWay (false), m_foundRestriction (false)
{
    // empty
}

// Constructor
OsmParser::OsmParser (const std::string &fileName)
  : XmlParserInterface (), m_fileName (fileName),
    m_readerState (ReaderStateUnknown), m_currentNode (), 
    m_currentWay (), m_currentRestriction (), m_reader (NULL),
    m_foundNode (false), m_foundWay (false), m_foundRestriction (false)
{
    // empty
}

// Destructor
OsmParser::~OsmParser ()
{
    if (m_reader)  {
        closeOsmFile ();
    }
}

// --- Methods
void OsmParser::setFileName (const std::string &fileName)
{
    m_fileName = fileName;
}

const std::string & OsmParser::getFileName () const
{
    return m_fileName;
}

void OsmParser::readOsmFile ()
{
    XmlFileReader reader (getFileName (), this);
    reader.readXmlFile ();
}

void OsmParser::openOsmFile ()
{
    m_reader = new XmlFileReader (getFileName (), this);
    assert (m_reader);
}

void OsmParser::getNext (std::vector<std::string> *values, int *elementType)
{
    assert (m_reader);
    m_reader->getNext ();
    getInterestingElement (values, elementType);
}

void OsmParser::closeOsmFile ()
{
    assert (m_reader);
    delete m_reader;
    m_reader = NULL;
    assert (!m_reader);
}

void OsmParser::getInterestingElement (std::vector<std::string> *values,
    int *elementType)
{
    assert (values);
    assert (elementType);
    values->clear ();
    if (isFoundNode ())  {
        (*values) = m_currentNode.getValues ();
        (*elementType) = OsmImportOperator::ELEMENT_TYPE_NODE;
    } else if (isFoundWay ())  {
        (*values) = m_currentWay.getValues ();
        (*elementType) = OsmImportOperator::ELEMENT_TYPE_WAY;
    } else if (isFoundRestriction ())  {
        (*values) = m_currentRestriction.getValues ();
        (*elementType) = OsmImportOperator::ELEMENT_TYPE_RESTRICTION;
    }
    // Resetting the boolean members used for iterative access
    setFoundNode (false);
    setFoundWay (false);
    setFoundRestriction (false);
}

void OsmParser::createNodeFromElement (const Element &element)
{
    m_currentNode = NodeData ();
    //m_currentNode.print ();
}

void OsmParser::updateNodeFromElement (const Element &element)
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
            m_currentNode.setId (
                OsmImportOperator::convStrToInt (*itAttrValues));
            foundId = true;
        } else if ((*itAttrNames) == "lat")  {
            m_currentNode.setLat (
                OsmImportOperator::convStrToDbl (*itAttrValues) *
                ScalingEngine::getInstance ().getScaleFactorX ());
            foundLat = true;
        } else if ((*itAttrNames) == "lon")  {
            m_currentNode.setLon (
                OsmImportOperator::convStrToDbl (*itAttrValues) *
                ScalingEngine::getInstance ().getScaleFactorY ());
            foundLon = true;
        }
        if (foundId && foundLat && foundLon)  {
            break;
        }
    }
}

void OsmParser::addTagElementToNode (const Element &element)
{
    TagData amenity = TagData::createTagFromElement (element, "amenity");
    TagData name = TagData::createTagFromElement (element, "name");
    if (amenity.getValue () != "")
        m_currentNode.setAmenity (amenity.getValue ());
    if (name.getValue () != "")
        m_currentNode.setName (name.getValue ());
}

void OsmParser::createWayFromElement (const Element &element)
{
    m_currentWay = WayData ();
    //m_currentWay.print ();
}

void OsmParser::updateWayFromElement (const Element &element)
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
            m_currentWay.setId (
                OsmImportOperator::convStrToInt (*itAttrValues));
            foundId = true;
        }
        if (foundId)  {
            break;
        }
    }
}

void OsmParser::addNdElementToWay (const Element &element)
{
    NdData nd = NdData::createNdFromElement (element);
    m_currentWay.addRef (nd.getRef ());
}

void OsmParser::addTagElementToWay (const Element &element)
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
        m_currentWay.setMaxSpeed (
            OsmImportOperator::convStrToInt (maxSpeed.getValue ()));
    if (oneWay.getValue () != "")
        m_currentWay.setOneWay (
            OsmImportOperator::convStrToInt (oneWay.getValue ()));
    if (layer.getValue () != "")
        m_currentWay.setLayer (
            OsmImportOperator::convStrToInt (layer.getValue ()));
    if (bridge.getValue () != "")
        m_currentWay.setBridge (bridge.getValue ());
    if (tunnel.getValue () != "")
        m_currentWay.setTunnel (tunnel.getValue ());
    if (ref.getValue () != "")
        m_currentWay.setRef (ref.getValue ());
}

void OsmParser::createRelationFromElement (const Element &element)
{
    m_currentRestriction = RestrictionData ();
    //m_currentRestriction.print ();
}

void OsmParser::updateRelationFromElement (const Element &element)
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
            m_currentRestriction.setId (
                OsmImportOperator::convStrToInt (*itAttrValues));
            foundId = true;
        }
        if (foundId)  {
            break;
        }
    }
}

void OsmParser::addTagElementToRelation (const Element &element)
{
    TagData restriction = TagData::createTagFromElement (element,
        "restriction");
    TagData type = TagData::createTagFromElement (element, "type");
    if (restriction.getValue () != "")
        m_currentRestriction.setRestriction (restriction.getValue ());
    if (type.getValue () != "")
        m_currentRestriction.setType (type.getValue ());
}

void OsmParser::addMemberElementToRelation (const Element &element)
{
    MemberData member = MemberData::createMemberFromElement (element);
    if (member.getRole () == "from" && member.getType () == "way")  {
        m_currentRestriction.setFrom (member.getRef ());
    } else if (member.getRole () == "via" && member.getType () == "node")  {
        m_currentRestriction.setVia (member.getRef ());
    } else if (member.getRole () == "to" && member.getType () == "way")  {
        m_currentRestriction.setTo (member.getRef ());
    }
}

void OsmParser::prepareElement (const Element &element)
{
    if (element.getLevel () == 1 && element.getName () == "node")  {
        createNodeFromElement (element);
    } else if (element.getLevel () == 1 && element.getName () == "way")  {
        createWayFromElement (element);
    } else if (element.getLevel () == 1 && element.getName () == "relation")  {
        createRelationFromElement (element);
    } 
}

void OsmParser::finalizeElement (const Element &element)
{
    int const & curState = getReaderState ();
    if (element.getLevel () == 1 && element.getName () == "node"/* &&
        is_bit_set(curState,IN_NODE) != 0*/)  {
        updateNodeFromElement (element);
        setFoundNode ();
    } else if (element.getLevel () == 2 && element.getName () == "tag" &&
               is_bit_set(curState,IN_NODE) != 0)  {
        addTagElementToNode (element); 
    } else if (element.getLevel () == 1 && element.getName () == "way")  {
        updateWayFromElement (element);
        setFoundWay ();
    } else if (element.getLevel () == 2 && element.getName () == "nd" &&
               is_bit_set(curState,IN_WAY) != 0)  {
        addNdElementToWay (element); 
    } else if (element.getLevel () == 2 && element.getName () == "tag" &&
               is_bit_set(curState,IN_WAY) != 0)  {
        addTagElementToWay (element); 
    } else if (element.getLevel () == 2 && element.getName () == "tag" &&
               is_bit_set(curState,IN_RELATION) != 0)  {
        addTagElementToRelation (element); 
    } else if (element.getLevel () == 2 && element.getName () == "member" &&
               is_bit_set(curState,IN_RELATION) != 0)  {
        addMemberElementToRelation (element); 
    } else if (element.getLevel () == 1 && element.getName () == "relation")  {
        updateRelationFromElement (element);
        if (m_currentRestriction.getType () == "restriction")  {
            setFoundRestriction ();
        }
    } 
}

const int & OsmParser::getReaderState () const
{
    return m_readerState;
}

void OsmParser::setReaderState (const int & readerState)
{
    m_readerState = readerState;
}

void OsmParser::updateState (const Element &element, bool up)
{
    int const & curState = getReaderState ();
    int newState = curState;

    if (up) {

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
        } else if (element.getLevel () == 2 &&
                element.getName () == "tag" &&
                is_bit_set(curState,IN_RELATION) != 0)  {
            // /relation/tag
            set_bit(newState,IN_TAG);
            assert (newState == ReaderStateInRelationTag);
        } else if (element.getLevel () == 2 &&
                element.getName () == "member" &&
                is_bit_set(curState,IN_RELATION) != 0)  {
            // /relation/member
            set_bit(newState,IN_MEMBER);
            assert (newState == ReaderStateInRelationMember);
        } else  {
            assert (false);
        }
    } else  {
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
        } else if (curState == ReaderStateInRelationTag)  {
            // cur: /relation/tag
            // new: /relation
            unset_bit(newState,IN_TAG);
            assert (newState == ReaderStateInRelation);
        } else if (curState == ReaderStateInRelationMember)  {
            // cur: /relation/member
            // new: /relation
            unset_bit(newState,IN_MEMBER);
            assert (newState == ReaderStateInRelation);
        } else if (curState == ReaderStateInNode ||
                   curState == ReaderStateInWay ||
                   curState == ReaderStateInRelation )  {
            newState = ReaderStateUnknown;
        } else {
            assert (false);
        }
    }

    // Propagating the state
    setReaderState (newState);
}

void OsmParser::setFoundNode (const bool &foundNode)
{
    m_foundNode = foundNode;
    /*if (m_foundNode)  {
        m_currentNode.print ();//TEST
    }*/
}

void OsmParser::setFoundWay (const bool &foundWay)
{
    m_foundWay = foundWay;
    /*if (m_foundWay)  {
        m_currentWay.print ();//TEST
    }*/
}

void OsmParser::setFoundRestriction (const bool &foundRestriction)
{
    m_foundRestriction = foundRestriction;
    /*if (m_foundRestriction)  {
        m_currentRestriction.print ();//TEST
    }*/
}

bool OsmParser::isFoundNode () const
{
    return m_foundNode;
}

bool OsmParser::isFoundWay () const
{
    return m_foundWay;
}

bool OsmParser::isFoundRestriction () const
{
    return m_foundRestriction;
}

void OsmParser::pushedElementToStack (const Element &element)
{
    prepareElement (element);
    updateState (element, true);
}

void OsmParser::poppedElementFromStack (const Element &element)
{
    finalizeElement (element);
    updateState (element, false);
}

bool OsmParser::isElementInteresting (const Element &element) const
{
    return (element.getLevel () == 1 && element.getName () == "node") ||
        (element.getLevel () == 1 && element.getName () == "way") ||
        (element.getLevel () == 1 && element.getName () == "relation") ||
        (element.getLevel () == 2 && element.getName () == "tag" &&
         is_bit_set(getReaderState (),IN_NODE) != 0) ||
        (element.getLevel () == 2 && element.getName () == "nd" &&
         is_bit_set(getReaderState (),IN_WAY) != 0) ||
        (element.getLevel () == 2 && element.getName () == "tag" &&
         is_bit_set(getReaderState (),IN_WAY) != 0) ||
        (element.getLevel () == 2 && element.getName () == "tag" &&
         is_bit_set(getReaderState (),IN_RELATION) != 0) ||
        (element.getLevel () == 2 && element.getName () == "member" &&
         is_bit_set(getReaderState (),IN_RELATION) != 0);
}

bool OsmParser::foundInterestingElement () const
{
    return m_foundNode || m_foundWay || m_foundRestriction;
}
