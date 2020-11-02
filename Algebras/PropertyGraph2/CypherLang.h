/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen
Faculty of Mathematic and Computer Science,
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

#ifndef CYPHERLANG_H
#define CYPHERLANG_H
#include <string>
#include <map>
#include <list>
#include "CypherParser.h"


//
#define YY_DECL yy::cypher_parser::symbol_type yylex(CypherLanguage& driver)
YY_DECL;

//

enum PathPartType { NODE, EDGE };
class Prop
{
public:
   std::string name="";
   std::string value="";
};
class PathPart
{
public:
   PathPartType type;
   std::string Alias="";
   std::string TypeName="";
   std::string direction="";
   std::list<Prop> Properties;
};
class MatchPath
{
public:
   std::list<PathPart> parts;
};
class Filter
{
public:
   std::string name="";
   std::string alias="";
   std::string op="";
   std::string value="";
};
class ReturnField
{
public:
   std::string name="";
   std::string alias="";
   std::string fieldname="";
};

// 
class CypherLanguage {
public:
  CypherLanguage ();
  virtual ~CypherLanguage ();

  void dump();
  std::string dumpAsListExpr();

  int counter=100;
  string _errormsg="";
  
  // match part
  std::list<MatchPath> Paths;
  std::string matchAlias="";
  std::string matchTypename="";
  MatchPath *matchCurrentPath=NULL;
  PathPart *matchCurrentPathPart=NULL;
  std::string matchEdgeDirection="";
  std::list<Prop> matchProperties;
  void pushMatchNode();
  void pushMatchEdge();
  void pushMatchPath();
  void pushMatchProperties();
  void addMatchProperty(std::string name, std::string value);
  void matchClear();

  // return part
  list<ReturnField> returnfields;
  std::string returnOutputName="";
  std::string returnNodeAlias="";
  std::string returnNodePropertyName="";
  void pushReturnPart();

  // where part
  std::list<Filter> filters;
  std::string whereFieldAlias="";
  std::string whereFieldName="";
  std::string whereValue="";
  std::string whereOperator="";
  void pushWherePart();

  // parser infrastructure
  void scan_begin ();
  void scan_end ();
  bool trace_scanning;
  bool trace_parsing;
  int parse (const std::string& text);
  string text;

  // Error handling.
  void error (const yy::location& l, const std::string& m);
  void error (const std::string& m);
};
#endif // CYPHERLANG_H


