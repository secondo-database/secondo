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

The class is part of a SAX parser, processing an XML file sequentially. It
offers pblic methods to inform it about events occuring while moving through
the file in forward direction (like stream processing). In this methods it
reacts to those events dispatching them to more specialised private functions
processing the read data.

*/

#ifndef ALGEBRAS_NESTEDRELATION2_DBLPPARSER_H_
#define ALGEBRAS_NESTEDRELATION2_DBLPPARSER_H_

#include "DblpImportLocalInfo.h"
#include <string>
#include "Element.h"
#include "XmlParserInterface.h"

//An unordered map (hash map) might be more efficient here, but is available
//not before C++11
#include <map>

namespace nr2a {

class ARel;
class NRel;
class DblpParser;

typedef void (DblpParser::*popMethodType)(const Element & element);
typedef Attribute* (*makeAttributeMethodType)(
    const std::string *content);

class DblpParser : public XmlParserInterface
{
    static const int c_attributeCount = 15;

  public:
    DblpParser(NRel *nrel, std::set<std::string> *stopwords,
        DblpImportLocalInfo *info);
    virtual ~DblpParser();
    virtual void pushedElementToStack(const Element &element);
    virtual void poppedElementFromStack(const Element &element);
    virtual bool isElementInteresting(const Element &element) const;
    virtual bool foundInterestingElement() const;
    virtual void processedText(const std::string &text);

    static ListExpr BuildResultType();

 private:

    void poppedDocument(const Element &element);
    void poppedAuthor(const Element &element);
    void poppedTitle(const Element &element);
    void poppedSimpleElement(const Element &element);

    void setAttribute(const int index, Attribute *attribute);

    static Attribute *createString(const std::string *content);
    static Attribute *createText(const std::string *content);

    std::string simplifyTitle(const std::string title) const;
    void extractKeywords(const std::string title,
        std::vector<std::string> &result) const;
    std::string mapElementNameToAttributeIndex
        (const std::string & elementName);

    static void buildMapping();
    void prepareTuple();

    void extractKeyword(char *tokenBuffer, int &tokenLength,
        std::vector<std::string> &keywords) const;

    Tuple *m_currentTuple;
    NRel *m_nrel;
    ListExpr m_tupleType;
    ARel *m_authors;
    ListExpr m_authorsType;
    ARel *m_keywords;
    ListExpr m_keywordsType;
    bool m_attributeSet[c_attributeCount];
    std::string m_currentText;
    DblpImportLocalInfo *m_info;

    std::set<std::string> *m_stopwords;

    //Maps elements' names to functions handling this elements content
    static std::map<std::string, popMethodType> *parserMapping;
    //Maps simple elements' names (corresponding to attribute names) to
    //attributes' indexes
    static std::map<std::string, int> *attributeMapping;
    static makeAttributeMethodType creationMapping[c_attributeCount];
};

} /* namespace nr2a*/

#endif /* ALGEBRAS_NESTEDRELATION2_DBLPPARSER_H_*/
