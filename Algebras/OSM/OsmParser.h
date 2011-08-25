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

[1] Header File of the OsmParser

June-November, 2011. Thomas Uchdorf

[TOC]

1 Overview

This header file essentially contains the definition of the class
~OsmParser~.

2 Defines and includes

*/
#ifndef __OSM_PARSER_H__
#define __OSM_PARSER_H__

// --- Including header-files
#include <string>
#include <stack>
#include <vector>
#include <iostream>
#include <fstream>
#include "Element.h"
#include "NodeData.h"
#include "WayData.h"
#include "RestrictionData.h"
#include "XmlParserInterface.h"

class XmlFileReader;

enum ReaderStates {
    ReaderStateUnknown = 0x0000,         //00000000
    ReaderStateInNode = 0x0001,          //00000001
    ReaderStateInTag = 0x0002,           //00000010
    ReaderStateInWay = 0x0004,           //00000100
    ReaderStateInRelation = 0x0008,      //00001000
    ReaderStateInNd = 0x0010,            //00010000
    ReaderStateInMember = 0x0020,        //00100000
    ReaderStateInNodeTag = 0x0003,       //00000011
    ReaderStateInWayNd = 0x0014,         //00010100
    ReaderStateInWayTag = 0x0006,        //00000110
    ReaderStateInRelationTag = 0x000A,   //00001010
    ReaderStateInRelationMember = 0x0028 //00101000
};

class OsmParser : public XmlParserInterface
{

    public:

        // --- Constructors
        // Default-Constructor
        OsmParser ();
        // Constructor
        OsmParser (const std::string &fileName);
        // Destructor
        virtual ~OsmParser ();

        // --- Methods
        void setFileName (const std::string &fileName);
        const std::string & getFileName () const;
        void readOsmFile ();

        void openOsmFile ();
        void getNext (std::vector<std::string> *values, int *elementType);
        void getInterestingElement (std::vector<std::string> *values,
            int *elementType);
        void closeOsmFile ();

    protected:

        // --- Methods
        void pushEmptyElementToStack (const Element &element);
        void pushElementToStack (const Element &element);
        void popElementFromStack (const Element &element);
        void createNodeFromElement (const Element &element);
        void updateNodeFromElement (const Element &element);
        void addTagElementToNode (const Element &element);
        void createWayFromElement (const Element &element);
        void updateWayFromElement (const Element &element);
        void addNdElementToWay (const Element &element);
        void addTagElementToWay (const Element &element);
        void createRelationFromElement (const Element &element);
        void updateRelationFromElement (const Element &element);
        void addTagElementToRelation (const Element &element);
        void addMemberElementToRelation (const Element &element);
        void prepareElement (const Element &element);
        void finalizeElement (const Element &element);
        const int & getReaderState () const;
        void setReaderState (const int & readerState);
        void updateState (const Element &element, bool up);

        void setFoundNode (const bool &foundNode = true);
        void setFoundWay (const bool &foundWay = true);
        void setFoundRestriction (const bool &foundRestriction = true);
        bool isFoundNode () const;
        bool isFoundWay () const;
        bool isFoundRestriction () const;

        // --- Functions of the parser interface 
        virtual void pushedElementToStack (const Element &element);
        virtual void poppedElementFromStack (const Element &element);
        virtual bool isElementInteresting (const Element &element) const;
        virtual bool foundInterestingElement () const;        

        // --- Members
        std::string m_fileName;
        int m_readerState;
        NodeData m_currentNode;
        WayData m_currentWay;
        RestrictionData m_currentRestriction;
        XmlFileReader *m_reader;
        bool m_foundNode;
        bool m_foundWay;
        bool m_foundRestriction;

        // --- Constants
        static const int IN_NODE;
        static const int IN_TAG;
        static const int IN_WAY;
        static const int IN_RELATION;
        static const int IN_ND;
        static const int IN_MEMBER;

};

#endif /* __OSM_PARSER_H__ */
