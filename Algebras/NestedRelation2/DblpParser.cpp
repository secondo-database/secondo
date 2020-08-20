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

*/

#include "DblpParser.h"
#include "ARel.h"
#include "NRel.h"
#include "../FText/FTextAlgebra.h"

using namespace std;
namespace nr2a {

/*
Various mappings used for parsing data from the given XML file.

*/
/*static*/map<string, popMethodType> *DblpParser::parserMapping = NULL;
/*static*/map<string, int> *DblpParser::attributeMapping = NULL;
/*static*/makeAttributeMethodType
  DblpParser::creationMapping[DblpParser::c_attributeCount] = { NULL };

/*
Constructor for building a DBLP Parser.

*/
DblpParser::DblpParser(NRel *nrel, std::set<std::string> *stopwords,
    DblpImportLocalInfo *info)
    : m_currentTuple(NULL), m_info(info)
{
  buildMapping();
  m_stopwords = stopwords;
  m_nrel = nrel;
  m_tupleType = SecondoSystem::GetCatalog()->NumericType(nl->Second(
      DblpParser::BuildResultType()));
  m_authorsType = nl->Second(nl->Nth(15, nl->Second(m_tupleType)));
  m_keywordsType = nl->Second(nl->Nth(16, nl->Second(m_tupleType)));
  prepareTuple();
}

/*
The parser collects the data for the subrelations in memory. They get written,
if they are complete and afterwards get built new immediately (For the sake of
simplicity only the events fired for closing elements are parsed). This
destroys the unused objects built after the last element.

*/
DblpParser::~DblpParser()
{
  m_authorsList->DeleteIfAllowed();
  m_keywords->DeleteIfAllowed();
  if (m_currentTuple != NULL)
  {
    m_currentTuple->DeleteIfAllowed();
    m_currentTuple = NULL;
  }
}

/*
No events about the beginning of elements are parsed any further.

*/
/*virtual*/void DblpParser::pushedElementToStack(const Element &element)
{
  std::map<std::string, popMethodType>::const_iterator it =
      DblpParser::parserMapping->find(element.getName());
  if (it != DblpParser::parserMapping->end())
  {
    m_currentText = "";
  }
}

/*
The events about elements being closed are analysed and mapped to functions
to process them according to their content.

*/
/*virtual*/void DblpParser::poppedElementFromStack
  (const Element &element)
{
  //call the method appropriate for the given attribute (selected by its name)
  std::map<std::string, popMethodType>::const_iterator it =
      DblpParser::parserMapping->find(element.getName());
  if (it != DblpParser::parserMapping->end())
  {
    popMethodType method = it->second;
    if (method != NULL)
    {
      (this->*(method))(element);
    }
    m_currentText = "";
  }
}

/*
All elements in the DBLP will be read.

*/
/*virtual*/bool DblpParser::isElementInteresting
  (const Element &element) const
{
  return true;
}

/*virtual*/bool DblpParser::foundInterestingElement() const
{
  assert(false);
  return true;
}

/*
If text between markup is found this will be stored and mapped to an attribute
of the resulting definition if the element containing the text is closed.

Given "<year>2012</year>" as input the string "2012" will be used to call this
function, but it is not yet clear, that it will be a year, so it is stored
internally. The function, that processes the closing markup "</year>" will
use the stored string and process it further.

*/
/*virtual*/void DblpParser::processedText(const string &text)
{
  m_currentText.append(text);
}

/*
Next is the function to process an entry for a document. It writes the
collected authors and keywords to the resulting nested relation. If some of the
atomic attributes are not defined they get set to default values.

*/
void DblpParser::poppedDocument(const Element &element)
{
  setAttribute(0, new CcString(true, element.getName()));
  setAttribute(1, new FText(true, m_authorsText));
  setAttribute(14, m_authorsList);
  setAttribute(15, m_keywords);  
  setAttribute(16, new CcInt(true, m_tuple_counter));
  
  m_tuple_counter++;

  for (int index = 0; index < c_attributeCount; index++)
  {
    if (m_attributeSet[index] == false)
    {
      if ((index < 1) || (index > 13)) // Range of atomic values
      {
        //This parser's destructor is called while unwindig the stack
        throw Nr2aException("Non-optional attribute unset in document "
            "element ending");
      }
      else
      {
        const makeAttributeMethodType &attributeCreation =
            creationMapping[index];
        m_currentTuple->PutAttribute(index, attributeCreation(NULL));
      }
    }
  }

  m_nrel->AppendTuple(m_currentTuple);
  m_info->UnitProcessed();
  prepareTuple();
}

/*
If authors are found their name gets converted to lower case letters and
both representations are added to a subrelation collecting them. The
representation, which has not been converted is also concatenated to a
text attribute containing all authors as a comma separated enumeration.

*/
void DblpParser::poppedAuthor(const Element &element)
{
  Tuple *tuple = new Tuple(nl->Second(m_authorsType));
  string name = m_currentText;
  tuple->PutAttribute(0, new CcString(true, name));
  stringutils::toLower(name);
  int pos = name.find_last_of(' ');
  if (pos != -1)
    name = name.substr(pos + 1, name.length() - pos);
  tuple->PutAttribute(1, new CcString(true, name));
  m_authorsList->AppendTuple(tuple);

  if(!m_authorsText.empty())
  {
    m_authorsText.append(", ");
  }
  m_authorsText.append(m_currentText);
}

/*
Titles get parsed for keywords, while stopwords are ignored. The result is
collected into a subrelation.

*/
void DblpParser::poppedTitle(const Element &element)
{
  string title = m_currentText;

  vector<string> keywords;
  extractKeywords(title, keywords);
  for(unsigned int i=0; i<keywords.size(); i++)
  {
    string keyword = keywords[i];
    Attribute *attribute = createString(&keyword);
    Tuple * tuple = new Tuple(nl->Second(m_keywordsType));
    tuple->PutAttribute(0, attribute);
    m_keywords->AppendTuple(tuple);
  }

  setAttribute(2, (new FText(true, title)));
}

/*
The other atomic elements get processed in a more generic manner using the
mappings defined. At first the attributeMapping maps the found XML elements'
name to the attribute index of the resulting relation. After that the
creationMapping maps to a function yielding an attribute value suitable for
the nth attribute's type.

Example:
1st: The read element "<year>" is mapped to the 4"th"[5] attribute by using
     attributeMapping
2nd: The index 4 is mapped to the function createString by creationMapping
3rd: createString yields a CcString containing "2012" for the current input
     text "2012"
4th: The CcString "2012" is stored in attribute 4 of the current tuple
5th: The current text is cleared

*/
void DblpParser::poppedSimpleElement(const Element &element)
{
  map<string, int>::const_iterator it =
      DblpParser::attributeMapping->find(element.getName());
  if (it != DblpParser::attributeMapping->end())
  {
    const int &index = it->second;
    const makeAttributeMethodType &attributeCreation =
        creationMapping[index];
    setAttribute(index, attributeCreation(&m_currentText));
    m_currentText = "";
  }

}

/*
Helper method to set an attribute's content. It marks the attribute as set, for
unmarked attributes get set to default values in poppedDocument.

*/
void DblpParser::setAttribute(const int index, Attribute *attribute)
{
  m_currentTuple->PutAttribute(index, attribute);
  m_attributeSet[index] = true;
}
/*
Yields a CcString containing the given input. If the input is NULL a default
string is returned.

*/
/*static*/Attribute *DblpParser::createString(const string *content)
{
  if (content != NULL)
  {
    return (new CcString(true, *content));
  }
  else
  {
    return (new CcString(false, ""));
  }
}

/*
Yields an FText containing the given input. If the input is NULL a default
text is returned.

*/
/*static*/Attribute *DblpParser::createText(const string *content)
{
  if (content != NULL)
  {
    return (new FText(true, *content));
  }
  else
  {
    return (new FText(false, ""));
  }
}

/*
Helper function extracting keywords from a document's title. It uses a set of
characters usually not contained in words or expressions. Numbers are not
contained, to support "words" containing numbers like "libxml2" or "NF2".

*/
void DblpParser::extractKeywords(const string title,
    std::vector<std::string> &result) const
{
  const int c_maxStringLength = MAX_STRINGSIZE;
  const int c_delimCount = 36;
  const char c_keywordDelimiters[c_delimCount] = {
      ' ', '\t', '\n', '\r', '\f',
      ',', ';', ':', '"', '.', '(', ')', '{', '}', '?', '!', '\'', '&', '$',
      '%', '[', ']', '=', '<', '>', '\\', '/', '*', '+', '~', '#', '-', '^',
      '@', '_', '\0' };

  assert(strlen(c_keywordDelimiters) + 1 == c_delimCount);

  const char * input = title.c_str();
  const int length = title.length();
  int currentPos = 0;
  int tokenLength = 0;
  char tokenBuffer[c_maxStringLength];

  int posInDelims = 0;

  while (currentPos < length)
  {
    while ((posInDelims < c_delimCount)
        && (c_keywordDelimiters[posInDelims] != input[currentPos]))
    {
      posInDelims++;
    }
    if (posInDelims < c_delimCount)
    {
      extractKeyword(&tokenBuffer[0], tokenLength, result);
    }
    else
    {
      if (tokenLength < c_maxStringLength)
      {
        tokenBuffer[tokenLength++] = input[currentPos];
      }
    }
    posInDelims = 0;
    currentPos++;
  }
  extractKeyword(&tokenBuffer[0], tokenLength, result);
}

/*
Small helper function checking whether a found token is a stopword or not.

*/
void DblpParser::extractKeyword(char *tokenBuffer, int &tokenLength,
    std::vector<std::string> &keywords) const
{
  if (tokenLength > 0)
  {
    tokenBuffer[tokenLength] = '\0';
    string keyword = tokenBuffer;
    std::set<std::string>::const_iterator iter = m_stopwords->find(keyword);
    if (iter == m_stopwords->end())
    {
      keywords.push_back(keyword);
    }
    tokenLength = 0;
  }
}

/*
Simple function mapping the name of an XML element to the name of an attribute
in the resulting relation.

*/
string mapElementNameToAttributeName(const string & elementName)
{
  string result = elementName.substr(0, 1);
  stringutils::toUpper(result);
  result.append(elementName.substr(1));
  return result;
}

/*
 A helper function constructing the type of the nested relation used for
 storing the dump of the DBLP.

*/
/*static*/ListExpr DblpParser::BuildResultType()
{
  ListBuilder authorsType;
  authorsType.AppendAttribute("Name", Symbols::STRING());
  authorsType.AppendAttribute("Lclastname", Symbols::STRING());

  ListBuilder keywordsType;
  keywordsType.AppendAttribute("Keyword", Symbols::STRING());

  ListBuilder resultType;
  resultType.AppendAttribute("Type", Symbols::STRING());
  resultType.AppendAttribute("Authors", Symbols::TEXT());
  resultType.AppendAttribute("Title", Symbols::TEXT());
  resultType.AppendAttribute("Booktitle", Symbols::TEXT());
  resultType.AppendAttribute("Pages", Symbols::STRING());
  resultType.AppendAttribute("Year", Symbols::STRING());
  resultType.AppendAttribute("Journal", Symbols::TEXT());
  resultType.AppendAttribute("Volume", Symbols::STRING());
  resultType.AppendAttribute("Number", Symbols::STRING());
  resultType.AppendAttribute("Month", Symbols::STRING());
  resultType.AppendAttribute("Url", Symbols::TEXT());
  resultType.AppendAttribute("School", Symbols::TEXT());
  resultType.AppendAttribute("Publisher", Symbols::TEXT());
  resultType.AppendAttribute("Isbn", Symbols::STRING());
  resultType.AppendAttribute("AuthorsList", authorsType.GetARel());
  resultType.AppendAttribute("Keywords", keywordsType.GetARel());
  resultType.AppendAttribute("DocId", Symbols::INT());

  return resultType.GetNRel();
}

/*
Function used to create the needed mappings during initialising.

*/
/*static*/void DblpParser::buildMapping()
{
  if (parserMapping == NULL)
  {
    std::map<std::string, popMethodType> &map =
        *(new std::map<std::string, popMethodType>());

    //The root element
    map["dblp"] = NULL;

    //Document types
    map["article"] = &DblpParser::poppedDocument;
    map["proceedings"] = &DblpParser::poppedDocument;
    map["inproceedings"] = &DblpParser::poppedDocument;
    map["incollection"] = &DblpParser::poppedDocument;
    map["book"] = &DblpParser::poppedDocument;
    map["phdthesis"] = &DblpParser::poppedDocument;
    map["mastersthesis"] = &DblpParser::poppedDocument;
    map["www"] = &DblpParser::poppedDocument;

    //Document's attributes needing special treatment.
    map["author"] = &DblpParser::poppedAuthor;
    map["title"] = &DblpParser::poppedTitle;
    map["ee"] = NULL;

    //Other elements, which are treated all the same
    map["booktitle"] = &DblpParser::poppedSimpleElement;
    map["pages"] = &DblpParser::poppedSimpleElement;
    map["year"] = &DblpParser::poppedSimpleElement;
    map["journal"] = &DblpParser::poppedSimpleElement;
    map["volume"] = &DblpParser::poppedSimpleElement;
    map["number"] = &DblpParser::poppedSimpleElement;
    map["month"] = &DblpParser::poppedSimpleElement;
    map["url"] = &DblpParser::poppedSimpleElement;
    map["school"] = &DblpParser::poppedSimpleElement;
    map["publisher"] = &DblpParser::poppedSimpleElement;
    map["isbn"] = &DblpParser::poppedSimpleElement;

    std::map<std::string, int> &mapAttr =
        *(new std::map<std::string, int>());

    mapAttr["title"] = 2;
    mapAttr["booktitle"] = 3;
    mapAttr["pages"] = 4;
    mapAttr["year"] = 5;
    mapAttr["journal"] = 6;
    mapAttr["volume"] = 7;
    mapAttr["number"] = 8;
    mapAttr["month"] = 9;
    mapAttr["url"] = 10;
    mapAttr["school"] = 11;
    mapAttr["publisher"] = 12;
    mapAttr["isbn"] = 13;

    creationMapping[0] = NULL;
    creationMapping[1] = createText;
    creationMapping[2] = createText;
    creationMapping[3] = createText;
    creationMapping[4] = createString;
    creationMapping[5] = createString;
    creationMapping[6] = createText;
    creationMapping[7] = createString;
    creationMapping[8] = createString;
    creationMapping[9] = createString;
    creationMapping[10] = createText;
    creationMapping[11] = createText;
    creationMapping[12] = createText;
    creationMapping[13] = createString;
    creationMapping[14] = NULL;
    creationMapping[15] = NULL;
    creationMapping[16] = NULL;

    parserMapping = &map;
    attributeMapping = &mapAttr;
  }
}

/*
This method is used to prepare a new tuple used to store the contents of the
elements found next.

*/
void DblpParser::prepareTuple()
{
  m_currentTuple = new Tuple(m_tupleType);
  m_authorsList = new ARel(m_authorsType);
  m_authorsText.clear();
  m_keywords = new ARel(m_keywordsType);
  for (int i = 0; i < c_attributeCount; i++)
  {
    m_attributeSet[i] = false;
  }
}

} /* namespace nr2a*/
