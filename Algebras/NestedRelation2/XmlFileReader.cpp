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

The content of this file is based on the OSM algebra of SECONDO
created by Thomas Uchdorf.

The implemented file reader depends on libxml2. If the dependency is not
fulfilled it writes an error message to the cerr output.

*/

// [...]
#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "XmlFileReader.h"
#include "XmlParserInterface.h"
#include <iostream>
#include <cassert>
#include <libxml/xmlreader.h>
#include <libxml/parser.h>
#include <Nr2aException.h>

#include "DblpImportLocalInfo.h"

XmlFileReader::XmlFileReader()
    : m_fileName(), m_parser(NULL), m_elements(), m_reader(NULL)
{
  // empty
}

XmlFileReader::XmlFileReader(const std::string &fileName,
    XmlParserInterface *parser, nr2a::DblpImportLocalInfo *info)
    : m_fileName(fileName), m_parser(parser), m_elements(), m_reader(NULL),
        m_info(info)
{
  open();
}

XmlFileReader::~XmlFileReader()
{
  close();
}

/*
Getter and setter for the filename of the XML file to process.

*/
void XmlFileReader::setFileName(const std::string &fileName)
{
  m_fileName = fileName;
}

const std::string & XmlFileReader::getFileName() const
{
  return m_fileName;
}

/*
To attach a parser to this reader use the following function.

*/
void XmlFileReader::setXmlParser(XmlParserInterface *parser)
{
  // Do not call this function twice (otherwise please manage the memory
  // yourself)!
  assert(m_parser == NULL);
  m_parser = parser;
}

#ifdef WITH_LIBXML2_SUPPORT
/*
This fucntions open and close an XML file.

*/
void XmlFileReader::open()
{
  assert(!m_reader);
  xmlInitParser();
  m_reader = new xmlTextReaderPtr();
  const char *fileName = getFileName().c_str();
  (*m_reader) = xmlReaderForFile(fileName, NULL, 0);
  assert(m_reader);
}

void XmlFileReader::close()
{
  assert(m_reader);
  xmlFreeTextReader(*m_reader);
  delete m_reader;
  m_reader = NULL;
  xmlCleanupParser();
  assert(!m_reader);
}

/*
This function starts reading an XML file.  It configures a reader through the
libxml2 API.

*/
int XmlFileReader::readXmlFile()
{
  int result = 0;
  xmlInitParser();
  const char *fileName = getFileName().c_str();
  xmlTextReaderPtr reader;
  int ret = 1;

  reader = xmlReaderForFile(fileName, NULL,
      XML_PARSE_HUGE | // Very big input is no reason to complain
      XML_PARSE_NONET | // Do not attempt to download schema etc.
      XML_PARSE_DTDLOAD | // Load the DTD
      XML_PARSE_DTDVALID | // Validate XML against the DTD
      XML_PARSE_NOENT | // Map entities as specified in DTD
      XML_PARSE_NOERROR | // Do not output errors to cout
      XML_PARSE_NOWARNING); // Do not output warnings to cout
  if (reader != NULL)
  {
    m_info->SetReader(reader);
    while (ret == 1)
    {
      ret = xmlTextReaderRead(reader);
      try
      {
        processXmlNode(reader);
      }
      catch (nr2a::Nr2aException e)
      {
        int lineNumber = xmlTextReaderGetParserLineNumber(reader);
        throw nr2a::Nr2aParserException(e.what(), lineNumber);
      }
    }
    xmlErrorPtr errorPtr = xmlGetLastError();
    if (errorPtr != NULL)
    {
      throw nr2a::Nr2aParserException(errorPtr->message, errorPtr->line);
    }
    xmlFreeTextReader(reader);
  }
  else
  {
    result = c_fileOpenError;
  }
  xmlCleanupParser();
  return result;
}

/*
Iterates the XML nodes.

*/
void XmlFileReader::getNext()
{
  assert(m_reader != NULL);
  const char *fileName = getFileName().c_str();
  int ret;
  bool found = false;

  if ((*m_reader) != NULL)
  {
    ret = xmlTextReaderRead(*m_reader);
    while (!found && ret == 1)
    {
      processXmlNode(*m_reader);
      ret = xmlTextReaderRead(*m_reader);
      found = foundInterestingElement();
    }
    if (!found && ret != 0)
    {
      std::cerr << "Could not parse \"" << fileName << "\"" << std::endl;
    }
  }
  else
  {
    std::cerr << "Could not open \"" << fileName << "\"" << std::endl;
  }
}

/*
This function processes an XML node by calling methods handling several
abstract events.

*/
void XmlFileReader::processXmlNode(xmlTextReaderPtr reader)
{
  std::string elementName = "";
  std::string elementValue = "";
  std::string attributeName = "";
  std::string attributeValue = "";
  int elementLevel = xmlTextReaderDepth(reader);
  const int line = xmlTextReaderGetParserLineNumber(reader);
  int nodeType = xmlTextReaderNodeType(reader);

  std::vector<std::string> attributeNames;
  std::vector<std::string> attributeValues;

  if (nodeType == (int) XML_READER_TYPE_ELEMENT)
  {
    elementName = (char *) xmlTextReaderConstName(reader);
    // Fetching the attributes
    while (xmlTextReaderMoveToNextAttribute(reader))
    {
      attributeName = (char *) xmlTextReaderConstName(reader);
      attributeValue = (char *) xmlTextReaderConstValue(reader);
      attributeNames.push_back(attributeName);
      attributeValues.push_back(attributeValue);
    }
    xmlTextReaderMoveToElement(reader);
    // Checking whether the tag looks like this:
    // <sometag attrib_1 = "" ... attrib_n="" />
    if (xmlTextReaderIsEmptyElement(reader))
    {
      // found empty element
      Element element(elementName, attributeNames, attributeValues,
          elementValue, elementLevel, line);
      pushEmptyElementToStack(element);
      attributeNames.clear();
      attributeValues.clear();
    }
    else
    {
      // found start of element that may be empty but consists of both a
      // start and an end tag
      Element element(elementName, attributeNames, attributeValues,
          elementValue, elementLevel, line);
      pushElementToStack(element);
      attributeNames.clear();
      attributeValues.clear();
    }
  }
  else if (nodeType == (int) XML_READER_TYPE_TEXT)
  {
    // found text
    elementValue = (char *) xmlTextReaderConstValue(reader);
    processText(elementValue);
  }
  else if (nodeType == (int) XML_READER_TYPE_END_ELEMENT)
  {
    // found end of element
    elementName = (char *) xmlTextReaderConstName(reader);
    Element element(elementName, attributeNames, attributeValues, elementValue,
        elementLevel, line);
    popElementFromStack(element);
    attributeNames.clear();
    attributeValues.clear();
  }
  else if (nodeType == (int) XML_READER_TYPE_ENTITY_REFERENCE)
  {
    elementName = (char *) xmlTextReaderConstName(reader);
    processEntityReference(elementName);
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
#endif /* WITH_LIBXML2_SUPPORT*/

/*
Methods for handling abstract events. They mostly just send them to the parser.

*/
void XmlFileReader::pushEmptyElementToStack(const Element &element)
{
  if (isElementInteresting(element))
  {
    pushElementToStack(element);
    popElementFromStack(element);
  }
}

void XmlFileReader::pushElementToStack(const Element &element)
{
  if (isElementInteresting(element))
  {
    m_elements.push(element);
    assert(m_parser != NULL);
    m_parser->pushedElementToStack(element);
  }
}

void XmlFileReader::popElementFromStack(const Element &element)
{
  if (isElementInteresting(element))
  {
    Element top = m_elements.top();
    m_elements.pop();
    assert(top.getName() == element.getName());
    assert(m_parser != NULL);
    m_parser->poppedElementFromStack(top);
  }
}

void XmlFileReader::processText(const std::string &text)
{
  m_parser->processedText(text);
}

void XmlFileReader::processEntityReference(const std::string &name)
{
  m_parser->processedEntityReference(name);
}

bool XmlFileReader::isElementInteresting(const Element &element) const
{
  assert(m_parser != NULL);
  return m_parser->isElementInteresting(element);
}

bool XmlFileReader::foundInterestingElement() const
{
  assert(m_parser != NULL);
  return m_parser->foundInterestingElement();
}
