/*
----
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

//paragraph [1] title: [{\Large \bf ] [}]
//[ae] [\"{a}]
//[ue] [\"{u}]

*/

#ifndef TYPES_H
#define TYPES_H

#include <list>
#include <map>
#include <stack>
#include <string>
#include <iostream>
#include <utility>
#include "OptSecUtils.h"
#include "SWI-Prolog.h"

/*
Use the prolog nested list plnl.

*/
extern NestedList* plnl;

const static string OPSTACKSEPARATOR = "---";
/*
Class which represents a multipurpose structure which passes information from the parser leafs up to the root of the parser. It also includes the needed checks.

*/

class OptParseStruct {
  std::string errorMessages;
  std::set<std::string> newNames;
  std::set<std::string> usedAliases;
  // attributename, alias (empty String if no alias)
  std::multimap<std::string, std::string> myAttributes;
  // relationname, alias
  std::multimap<std::string, std::string> myRelations;
  // groupybclauseaattribuename, alias
  std::multimap<std::string, std::string> myGroupClauseAttributes;
  // stores attributes and alias from within aggregation op stack of strings
  std::multimap<std::string, std::string> myGroupOpAttributes;
  // stores operators and parameters in a stack of strings
  std::stack<std::string> myOperators;
  // is there a groupclose in the query
  bool groupbyclause;
  // is there an aggregationoperator alias in the query
  bool aggregationalias;
  // is there a aggregationoperator
  bool aggregationoperator;

public:
  OptParseStruct();
  ~OptParseStruct();

  void mergeStruct(OptParseStruct *optstruct);

/*
~addErrorMessage~

Adds an errormessage to OptParseStruct, which indicates that something is wrong with the optimizer statement.
This message is being collected by start production sqlclause. This is done this way, so the user can be
informend about multiple issues of the query.

*/

  void addErrorMessage(std::string errormessage) {
    if (errormessage.size() > 0) {
      //debug
      cout << "ErrorMessage : " << errormessage << endl;
      errorMessages = errorMessages + errormessage;
    }
  }

/*
~addNewName~

Adds a new name to the OptParseStruct which can be an alias or a new object name

*/
  void addNewName(std::string newname);

/*
~addAttribute~

Adds the name and the alias of an attribute to the OptParseStruct.

*/
  void addAttribute(std::string attributename, std::string attributealias);

  void addAttribute(std::string attributename);

/*
~addGroupClauseAttribute~

Adds the name and the alias of an groupattribute in the groupbyclause to the OptParseStruct.

*/
 void addGroupClauseAttribute(std::string attributename, 
 std::string attributealias);

 void addGroupClauseAttribute(std::string attributename);

/*
~setGroupClause~

Makes it possible to check if there is a groupby clause in the query.

*/
 void setGroupClause(bool value) {
    groupbyclause = value;
 }
 
/*
~setAggregationAlias~

Makes it possible to check if there is a alias for a aggregationterm was used in the query.

*/
 void setAggregationAlias(bool value) {
    aggregationalias = value;
 } 
 
/*
~setAggregationoperatorUsed~

Makes it possible to check if there was an aggregationoperator used in the query.

*/
 void setAggregationoperatorUsed(bool value) {
    aggregationoperator = value;
 } 
 
/*
~addGroupAttribute~

  Adds the name and the alias of an groupattribute in selclause to the OptParseStruct.

*/
void addGroupAttribute(std::string attributename, std::string attributealias);

void addGroupAttribute(std::string attributename);

/*
~addRelation~

  Adds the name and of a Relation to the OptParseStruct.

*/
  void addRelation(std::string relationname);
  void addRelation(std::string relationname, std::string relationalias);

  void addOperator(std::string operatorname,
      std::list<std::string> parameters);

/*
~addUsedAlias~

This methods adds aliases to the usedAlias set and generates an error if the alias was already used.

*/

  void addUsedAlias(string aliasname);

/*
~addOperator~
Takes a string and checks if it is an operator an adds it to  myOperators, if op does exist.

*/
  void addToOpStack(string operatorname);

/*
~checkAttributes~

check if attributes in myAttributes do exist in the given Relations

*/
  void checkAttributes();

/*
~appendToOperators~

This method takes a stack<string> and appends it to the bottom of the local 
Operators stack.

*/
  void appendToOpStack(stack<string> append);
  void addOperatorSeparator();

/*
~reverseOpStack~

Basic method to reverse a string stack.

*/
  stack<string> reverseOpStack(stack<string> opstack);

/*
~checkOpStack~

A recursive Function which takes a string stack and tries to return

*/

  string checkOpStack(stack<string> operators);

/*
~getAttributeType~

This method tries to determine the type of the attribute by checking 
the given relations.

*/

  string getAttributeType(string attribute);

/*
~checkOperators~

This method is being called to check the myOperators stack if it does return 
one valid type. Calls the recursive ~checkOpStack~ method to check for nested 
Operators. Is being called in the Optparser.y when given relations are 
available.

*/
  void checkOperators();


/*
~checkAggregation~

This method is being called to check if the the structure of the aggregation
operators is correct.

*/
  void checkAggregation();

  void dumpAllInfo();

  void subqueriesAllowed();

  bool optimizerOption(std::string name);

  /* getter and setter below */
  std::string getErrorMessages() const {
    return errorMessages;
  }

  std::multimap<std::string, std::string> getMyAttributes() const {
    return myAttributes;
  }

  std::multimap<std::string, std::string> getMyGroupClauseAttributes() const {
    return myGroupClauseAttributes;
  }
  
    std::multimap<std::string, std::string> getMyGroupOpAttributes() const {
    return myGroupOpAttributes;
  }
  

  std::multimap<std::string, std::string> getMyRelations() const {
    return myRelations;
  }

  std::stack<std::string> getMyOperators() const {
    return myOperators;
  }

  std::set<std::string> getNewNames() const {
    return newNames;
  }

  std::set<std::string> getUsedAliases() const {
    return usedAliases;
  }
  
  bool getGroupbyClauseUsed() const {
    return groupbyclause;
  }
  
  bool getAggregationAlias() const {
    return aggregationalias;
  }
  
  bool getAggregationoperatorUsed() const {
    return aggregationoperator;
  }
  
};
  

#endif

