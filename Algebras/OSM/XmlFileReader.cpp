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
~XmlFileReader~.

For more detailed information see XmlFileReader.h.

2 Defines and Includes

*/

#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "XmlFileReader.h"
#include "XmlParserInterface.h"
#include <iostream>
#include <cassert>

// --- Constructors
// Default-Constructor
XmlFileReader::XmlFileReader ()
  : m_fileName (), m_parser (NULL), m_elements (), m_reader (NULL)
{
    // empty
}

// Constructor
XmlFileReader::XmlFileReader (const std::string &fileName,
    XmlParserInterface *parser)
  : m_fileName (fileName), m_parser (parser), m_elements (), m_reader (NULL)
{
    open ();
}

// Destructor
XmlFileReader::~XmlFileReader ()
{
    close ();
}

// --- Methods
void XmlFileReader::setFileName (const std::string &fileName)
{
    m_fileName = fileName;
}

const std::string & XmlFileReader::getFileName () const
{
    return m_fileName;
}

void XmlFileReader::setXmlParser (XmlParserInterface *parser)
{
    // Do not call this function twice (otherwise please manage the memory
    // yourself)!
    assert (m_parser == NULL);
    m_parser = parser;
}

#ifdef WITH_LIBXML2_SUPPORT
void XmlFileReader::open ()
{
    assert (!m_reader);
    xmlInitParser();
    m_reader = new xmlTextReaderPtr ();
    const char *fileName = getFileName ().c_str();
    (*m_reader) = xmlReaderForFile(fileName, NULL, 0);
    //(*m_reader) = xmlReaderForFd(
    //    XmlFdCache::getInstance ().getFd (fileName), NULL, NULL, 0);
    //ifstream infile;
    // Opening a file for reading
    //infile.open (fileName, fstream::in);
    // Determining the file size
    //infile.seekg (0, ios::end);
    //length = infile.tellg();
    // Setting the position of the file descriptor
    //infile.seekg (0, ios::beg);
    //infile.close();
    assert (m_reader);
}

void XmlFileReader::close ()
{
    assert (m_reader);
    xmlFreeTextReader(*m_reader);
    delete m_reader;
    m_reader = NULL;
    xmlCleanupParser();
    assert (!m_reader);
}

void XmlFileReader::readXmlFile ()
{
    xmlInitParser();
    const char *fileName = getFileName ().c_str();
    xmlTextReaderPtr reader;
    int ret;

    reader = xmlReaderForFile(fileName, NULL, 0);
    if (reader != NULL) {
        ret = xmlTextReaderRead(reader);
        while (ret == 1) {
            processXmlNode(reader);
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0) {
            std::cerr << "Could not parse \"" << fileName << "\"" << std::endl;
        }
    } else {
        std::cerr << "Could not open \"" << fileName << "\"" << std::endl;
    }
    xmlCleanupParser();
}

void XmlFileReader::getNext ()
{
    assert (m_reader != NULL);
    const char *fileName = getFileName ().c_str();
    int ret;
    bool found = false;

    if ((*m_reader) != NULL) {
        ret = xmlTextReaderRead(*m_reader);
        while (!found && ret == 1) {
            processXmlNode(*m_reader);
            ret = xmlTextReaderRead(*m_reader);
            found = foundInterestingElement ();
        }
        if (!found && ret != 0) {
            std::cerr << "Could not parse \"" << fileName << "\"" << std::endl;
        }
    } else {
        std::cerr << "Could not open \"" << fileName << "\"" << std::endl;
    }
}

void XmlFileReader::processXmlNode(xmlTextReaderPtr reader)
{
    //TODO Remove the artifical limit of less than 200 lines
    //if (xmlTextReaderGetParserLineNumber (reader) > 200)  {
    //    return;
    //}
    std::string elementName = "";
    std::string elementValue = "";
    std::string attributeName = "";
    std::string attributeValue = "";
    int elementLevel = xmlTextReaderDepth(reader);
    int nodeType = xmlTextReaderNodeType(reader);

    std::vector<std::string> attributeNames;
    std::vector<std::string> attributeValues;

    if (nodeType ==  (int)XML_READER_TYPE_ELEMENT)  {
        elementName = (char *)xmlTextReaderConstName(reader);
        // Fetching the attributes
        while (xmlTextReaderMoveToNextAttribute(reader))  {
            attributeName = (char *)xmlTextReaderConstName(reader);
            attributeValue = (char *)xmlTextReaderConstValue(reader);
            attributeNames.push_back (attributeName); 
            attributeValues.push_back (attributeValue); 
        }
        xmlTextReaderMoveToElement(reader);
        // Checking whether the tag looks like this:
        // <sometag attrib_1 = "" ... attrib_n="" />
        if (xmlTextReaderIsEmptyElement (reader))  {
            // found empty element
            Element element (elementName, attributeNames, attributeValues, 
                elementValue, elementLevel);
            pushEmptyElementToStack (element);
            attributeNames.clear ();
            attributeValues.clear ();
        } else {
            // found start of element that may be empty but consists of both a
            // start and an end tag
            Element element (elementName, attributeNames, attributeValues,
                elementValue, elementLevel);
            pushElementToStack (element);
            attributeNames.clear ();
            attributeValues.clear ();
        }
    } else if (nodeType ==  (int)XML_READER_TYPE_TEXT)  {
        // found text
        elementValue = (char *)xmlTextReaderConstValue(reader);
    } else if (nodeType == (int)XML_READER_TYPE_END_ELEMENT)  {
        // found end of element
        elementName = (char *)xmlTextReaderConstName(reader);
        Element element (elementName, attributeNames, attributeValues,
            elementValue, elementLevel);
        popElementFromStack (element);
        attributeNames.clear ();
        attributeValues.clear ();
    }
}
#else
void XmlFileReader::open ()
{
    // empty
}

void XmlFileReader::close ()
{
    // empty
}

void XmlFileReader::readXmlFile ()
{
    std::cerr << "libxml2 is not supported!" << std::endl;
}

void XmlFileReader::getNext ()
{
    std::cerr << "libxml2 is not supported!" << std::endl;
}
#endif /* WITH_LIBXML2_SUPPORT */

void XmlFileReader::pushEmptyElementToStack (const Element &element)
{
    if (isElementInteresting (element))  {
        pushElementToStack (element);
        popElementFromStack (element);
    }
}

void XmlFileReader::pushElementToStack (const Element &element)
{
    if (isElementInteresting (element))  {
        m_elements.push (element);
        assert (m_parser != NULL);
        m_parser->pushedElementToStack (element);
    }
}

void XmlFileReader::popElementFromStack (const Element &element)
{
    if (isElementInteresting (element))  {
        Element top = m_elements.top ();
        m_elements.pop ();
        assert (top.getName () == element.getName ());
        assert (m_parser != NULL);
        m_parser->poppedElementFromStack (top);
    }
}

bool XmlFileReader::isElementInteresting (const Element &element) const
{
    assert (m_parser != NULL);
    return m_parser->isElementInteresting (element);
}

bool XmlFileReader::foundInterestingElement () const
{
    assert (m_parser != NULL);
    return m_parser->foundInterestingElement ();
}
