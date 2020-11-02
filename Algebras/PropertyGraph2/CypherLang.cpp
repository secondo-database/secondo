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

#include "CypherLang.h"
#include "CypherParser.h"
#include <sstream>
#include <set>

CypherLanguage::CypherLanguage () : trace_scanning (false), 
    trace_parsing (false) {
}

CypherLanguage::~CypherLanguage () {
}

int CypherLanguage::parse (const std::string &text_) {
  text = text_;
  scan_begin ();
  yy::cypher_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = parser.parse ();
  scan_end ();
  return res;
}

void CypherLanguage::error (const yy::location& l, const std::string& m) {
  stringstream s;
  s << l << ": " << m << std::endl;
  _errormsg= s.str();
}

void CypherLanguage::error (const std::string& m) {
  stringstream s;
  s <<  m << std::endl;
  _errormsg= s.str();
}

void CypherLanguage::matchClear()
{
   matchAlias="";
   matchTypename="";
   matchEdgeDirection="";
   matchProperties.clear();
}

void CypherLanguage::pushMatchProperties()
{
    for(auto&& p:matchProperties)
    {
      matchCurrentPathPart->Properties.push_back(p);
    }
    matchProperties.clear();
}

void CypherLanguage::pushMatchNode()
{
  // don't allow anonymous nodes
  if (matchAlias.empty()) matchAlias="anon"+to_string(counter++);

  PathPart pp;
  pp.type=PathPartType::NODE;
  pp.Alias=matchAlias;
  pp.TypeName=matchTypename;
  if (matchCurrentPath==NULL)
  {
       MatchPath p;
       Paths.push_back(p);
       matchCurrentPath=&Paths.back();
  }
  matchCurrentPath->parts.push_back(pp);
  matchCurrentPathPart=&matchCurrentPath->parts.back();
  pushMatchProperties();

  matchClear();

}

void CypherLanguage::addMatchProperty(string name, string value)
{
  Prop p;
  p.name=name;
  p.value=value;
  matchProperties.push_back(p);
}

void CypherLanguage::pushMatchPath()
{
    matchCurrentPathPart=NULL; 
    matchCurrentPath=NULL;
}

void CypherLanguage::pushMatchEdge()
{
  PathPart pp;
  pp.type=PathPartType::EDGE;
  pp.direction=matchEdgeDirection;
  pp.TypeName=matchTypename;
  pp.Alias=matchAlias;
  matchCurrentPath->parts.push_back(pp);
  matchCurrentPathPart=&matchCurrentPath->parts.back();

  pushMatchProperties();

  matchClear();

}

void CypherLanguage::pushReturnPart()
{
  // use nmode name as default outputname
  if (returnOutputName.empty()) returnOutputName=returnNodePropertyName;

  ReturnField f;
  f.name=returnNodePropertyName;
  f.alias=returnNodeAlias;
  f.fieldname=returnOutputName;

  returnfields.push_back(f);

  returnOutputName="";
  returnNodeAlias="";
  returnNodePropertyName="";

}

void CypherLanguage::pushWherePart()
{
  Filter f;
  f.alias=whereFieldAlias;;
  f.name=whereFieldName;
  f.op=whereOperator;
  f.value=whereValue;

  filters.push_back(f);

  whereFieldAlias="";
  whereFieldName="";
  whereOperator="";
  whereValue="";
}

string dumpProperties(stringstream &s, list<Prop> &props)
{
    if (props.size()>0)
    {
      s<<" ( ";
      for(auto&& prop : props)
      {
        s<<"("<<prop.name<<" "<<prop.value<<") ";
      }
      s<<")";
    }
    return "";
}

string CypherLanguage::dumpAsListExpr()
{
  stringstream s;
  set<string> allNodes;

  s << "( "<<endl;
  s << "   ( "<<endl;
  for(auto&& p : Paths)
  {
    for(auto&& pp : p.parts)
    {
        if (pp.type==PathPartType::NODE)
        {
           if (allNodes.find(pp.Alias)== allNodes.end())
           {
               allNodes.insert(pp.Alias);
               s << "      (" << pp.Alias;
               if (!pp.TypeName.empty()) s << " " << pp.TypeName;
               dumpProperties(s, pp.Properties);
               s << ")" << std::endl;
           }
        }
    }
  }
  for(auto&& p : Paths)
  {
    PathPart *p1=NULL, *p2=NULL, *pe;
    for(auto&& pp : p.parts)
    {
        if (pp.type==PathPartType::NODE)
        {
           if (p1==NULL) { p1=&pp; continue; }
           p2=&pp;
           if (pe->direction=="left")
              s << "      (" << p2->Alias << " "<< pe->Alias << " " << 
                   pe->TypeName << " " << p1->Alias;
           else   
              s << "      (" << p1->Alias << " " << pe->Alias << " " << 
                    pe->TypeName << " " << p2->Alias;
           dumpProperties(s, pe->Properties);
           s << ")" << std::endl;
           p1=p2;  // move forward
        }

        if (pp.type==PathPartType::EDGE)
        {
           pe=&pp; 
           continue;
        }
    }
  }
   s << "   ) "<<endl;
   s << "   ( "<<endl;
    for(auto&& f : filters)
    {
      s << "      ((" << f.alias << " " << f.name <<") "<<f.op<< " " << 
             f.value << ")" << std::endl;
    }

   s << "   ) "<<endl;
   s << "   ( "<<endl;
    for(auto&& f : returnfields)
    {
      s << "      ((" << f.alias << " " << f.name << ") "<< f.fieldname 
        << ")" << endl;
    }

   s << "   )"<<endl;
   s << ")"<<endl;

  return s.str();
}

void CypherLanguage::dump()
{
      cout << "MATCH" << std::endl;
      for(auto&& p : Paths)
      {
        cout << "  Path "  << std::endl;
        for(auto&& pp : p.parts)
        {
            if (pp.type==PathPartType::NODE)
            {
              cout << "    " <<"NODE" << std::endl;
             if (pp.Alias!="") cout << "    " <<"  alias:" <<  pp.Alias<< endl;
             if (pp.TypeName!="") cout << "    " <<"  nodetype:" <<  
                  pp.TypeName<< endl;
            }
            else
            {
               cout << "    " <<"EDGE" << endl;
               if (pp.direction!="") cout << "    " <<"  direction:" <<  
                     pp.direction << endl;
            }
            if (pp.Properties.size()>0)
            {
              cout << "      " <<"PROPS" << endl;
              for(auto&& prop : pp.Properties)
              {
                cout << "        " << prop.name << "=" << prop.value << endl;
              }
            }
        }
      }

      std::cout << "WHERE" <<  std::endl;
      for(auto&& f : filters)
      {
        std::cout << "  " << f.alias << "." << f.name <<" "<<f.op<< " " << 
           f.value << std::endl;
      }
      std::cout << "RETURN" << std::endl;
      for(auto&& f : returnfields)
      {
        std::cout << "  " << f.alias << "." << f.name << " as "<< 
            f.fieldname << std::endl;
      }

}

