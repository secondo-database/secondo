/*
----
This file is part of SECONDO.

Copyright (C) 2016, 
University in Hagen, 
Faculty of Mathematocs and Computer Science,
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
%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.4"
%define parser_class_name {cypher_parser}

%output  "CypherParser.cpp"
%defines "CypherParser.h"

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
  #include <string>
  using namespace std;
  class CypherLanguage;
}

// The parsing context.
%param { CypherLanguage& driver }

%locations
%initial-action {
  // Initialize the initial location.
  @$.begin.filename = @$.end.filename = &driver.text;
};

%define parse.trace
%define parse.error verbose

%code {
  #include "CypherLang.h"
}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"

  LNPAREN  "("
  RNPAREN  ")"
  LEPAREN  "["
  REPAREN  "]"
  LPPAREN  "{"
  RPPAREN  "}"
  COMMA   "," 
  DOT     "." 
  OPEQUAL "=" 
  OPNOTEQUALTHAN "<>" 
  OPLESSERTHAN "<" 
  OPBIGGERTHAN ">" 
  MATCH   "match"
  AND 
  STARTS
  WITH 
  CONTAINS
  WHERE 
  AS 
  RETURN
  DASH
  LEFTDASH
  RIGHTDASH
  COLON
;

%token <string> IDENTIFIER "identifier"
%token <string> STRINGLITERAL 
%token <int> NUMBER "number"


%printer { yyoutput << $$; } <*>;

%%

%start start;
start:  MATCH matchpart RETURN returnpart
        | MATCH matchpart WHERE wherepart RETURN returnpart
         ;

//-------------------------------------
// MATCH PART
//-------------------------------------
matchpart: 
        matchsinglepath
        |
        matchpart COMMA  matchsinglepath
        ;

matchsinglepath: 
        matchsinglepath2                          { driver.pushMatchPath(); }
        ;

matchsinglepath2: 
        nodepart 
        |
        nodepart nodepartconn matchsinglepath2     
        ;

nodepart:
        LNPAREN 
        nodepartinner                             { driver.pushMatchNode(); }
        RNPAREN
        ;

nodepartinner:
        %empty
        |
        alias proplist   
        | 
        alias COLON typename proplist
        | 
        COLON typename proplist
        ;

nodepartconn:
        DASH edgeinner DASH        { driver.matchEdgeDirection="";   
                                     driver.pushMatchEdge(); }
        |
        LEFTDASH edgeinner DASH    { driver.matchEdgeDirection="left";   
                                     driver.pushMatchEdge(); }
        |
        DASH edgeinner RIGHTDASH    { driver.matchEdgeDirection="right";   
                                      driver.pushMatchEdge(); }
        ;

edgeinner:
        %empty
        |
        LEPAREN edgealiasname proplist REPAREN   
        ;

edgealiasname:
        alias    
        | 
        alias COLON typename
        | 
        COLON typename
        ;

alias:
        IDENTIFIER                 {  driver.matchAlias=$1;  }
        ;

typename:
        IDENTIFIER                 {  driver.matchTypename=$1;  }
        ;


proplist:
        %empty
        |
        LPPAREN proplistinner RPPAREN  
        ;

proplistinner:
        proplistinneritem 
        |
        proplistinneritem COMMA proplistinner;
        ;

proplistinneritem:
        IDENTIFIER COLON STRINGLITERAL        
                                  { driver.addMatchProperty($1,"\""+$3+"\"");  }
        |
        IDENTIFIER COLON NUMBER               
                                 { driver.addMatchProperty($1,to_string($3));  }
        ;

//-------------------------------------
// WHERE PART
//-------------------------------------
wherepart: 
        wherepartlist
        ;

wherepartlist:
        wherepartclause            { driver.pushWherePart(); }
        | 
        wherepartlist 
        AND 
        wherepartclause            { driver.pushWherePart(); }
        ;  

wherepartclause: 
        whereidentifier 
        whereoperator  
        wherevalue
        ;

whereidentifier:
        IDENTIFIER "." IDENTIFIER
                                   {   driver.whereFieldAlias=$1;
                                       driver.whereFieldName=$3;
                                   }
        ;

 whereoperator:
        OPEQUAL                    { driver.whereOperator="=";}
        | 
        OPNOTEQUALTHAN             { driver.whereOperator="<>";}
        | 
        OPLESSERTHAN               { driver.whereOperator="<";}
        | 
        OPBIGGERTHAN               { driver.whereOperator=">";}
        | 
        STARTS WITH                { driver.whereOperator="startswith";}
        | 
        CONTAINS                   { driver.whereOperator="contains";}
        ;
 
 wherevalue:
        NUMBER                     { driver.whereValue=to_string($1);}
        | 
        STRINGLITERAL              { driver.whereValue="\""+$1+"\"";}
        ;

     


//-------------------------------------
// RETURN PART
//-------------------------------------
returnpart: 
        returnclauselist
        ;

returnclauselist:
        returnclausepart           { driver.pushReturnPart(); }
        | 
        returnclauselist 
        "," 
        returnclausepart           { driver.pushReturnPart(); }
        ;  
        
returnclausepart:
        returnidentifiername                   
        |
        returnidentifiername AS returnasname 
        ; 

returnasname:
        IDENTIFIER                 {  driver.returnOutputName=$1;  }
        ;

returnidentifiername:
        IDENTIFIER "." IDENTIFIER  {   driver.returnNodeAlias=$1;
                                       driver.returnNodePropertyName=$3;
                                   }
        ;
        

%%

void yy::cypher_parser::error (const location_type& l,const string& m) {
  driver.error (l, m);
}

