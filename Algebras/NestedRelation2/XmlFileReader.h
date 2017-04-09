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

 The contained class reads an XML file sequentially and informs an attached
 parser about the structures found.

*/

// [...]
#ifndef __XML_FILE_READER_H__
#define __XML_FILE_READER_H__

// --- Including header-files
#include <string>
#include <stack>
#include "Element.h"

#define WITH_LIBXML2_SUPPORT
#ifdef WITH_LIBXML2_SUPPORT
#include <libxml/xmlreader.h>
#else
class xmlTextReaderPtr;
#endif

class XmlParserInterface;

#ifndef ALGEBRAS_NESTEDRELATION2_DBLPIMPORTLOCALINFO_DECLARED_
#define ALGEBRAS_NESTEDRELATION2_DBLPIMPORTLOCALINFO_DECLARED_
namespace nr2a {
  class DblpImportLocalInfo;
}
#endif

/*
This class acts as an object oriented wrapper around the API of libxml2. it is
based on the "XmlFileReader"[2] class of the OSM algebra.

*/
class XmlFileReader
{

  public:
    static const int c_success = 0;
    static const int c_fileOpenError = -1;
    static const int c_processingError = -2;
    static const int c_maxErrorLines = 100;

    XmlFileReader();
    XmlFileReader(const std::string &fileName,
        XmlParserInterface *parser, nr2a::DblpImportLocalInfo *info);
    ~XmlFileReader();

    void setFileName(const std::string &fileName);
    const std::string & getFileName() const;
    int readXmlFile();
    void getNext();

    void popElementFromStack(const Element &element);
#ifdef WITH_LIBXML2_SUPPORT
    void processXmlNode(xmlTextReaderPtr reader);
#endif
    std::string getErrorMessages();

  protected:
    void setXmlParser(XmlParserInterface *parser);
    void open();
    void close();

    void pushEmptyElementToStack(const Element &element);
    void pushElementToStack(const Element &element);
    void processText(const std::string &text);
    void processEntityReference(const std::string &name);
    bool isElementInteresting(const Element &element) const;

    bool foundInterestingElement() const;

    std::string m_fileName;

    XmlParserInterface *m_parser;

    std::stack<Element> m_elements;

    xmlTextReaderPtr *m_reader;
    nr2a::DblpImportLocalInfo *m_info;
  private:
    static void errorHandler(void *ctx, const char *msg, ...);
    std::string m_errorString;
    int m_errorCounter;
};

#endif /* __XML_FILE_READER_H__*/
