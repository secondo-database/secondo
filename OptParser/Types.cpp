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

\vspace{1cm}
\centerline{\LARGE \bf  SecondoPL}

\begin{center}
\footnotesize
\tableofcontents
\end{center}

1 Overview

This is the source code of the data structure passed up in the 
bison parser tree.

2 Includes and defines

*/

#include "Types.h"

OptParseStruct::OptParseStruct()
{
  groupbyclause = false;
  aggregationalias = false;
  aggregationoperator = false;
}

OptParseStruct::~OptParseStruct()
{

}
/*
~mergeOptParseStruct~

The method ~mergeStruct~ joins two OptParseStruct classes so the data 
can be passed up in the Parsertree.

*/

void OptParseStruct::mergeStruct(OptParseStruct *optstruct)
{
  if (optstruct != NULL)
  {  
  // merge ErrorMessages
  std::string mErrorMessages = optstruct->getErrorMessages();
  addErrorMessage(mErrorMessages);

  // merge NewNames
  std::set<std::string> mNewNames = optstruct->getNewNames();
  newNames.insert(mNewNames.begin(), mNewNames.end());

  // merge usedAliases
  std::set<std::string> mUsedAliases = optstruct->getUsedAliases();
  usedAliases.insert(mUsedAliases.begin(), mUsedAliases.end());

  // merge Attributes  
  std::multimap<std::string, std::string> mAttributes =
      optstruct->getMyAttributes();
  myAttributes.insert(mAttributes.begin(), mAttributes.end());

  // merge myGroupClauseAttributes
  std::multimap<std::string, std::string> mGroupClauseAttributes =
      optstruct->getMyGroupClauseAttributes();
  myGroupClauseAttributes.insert(mGroupClauseAttributes.begin(), 
      mGroupClauseAttributes.end());
  
  // merge Relations
  std::multimap<std::string, std::string> mRelations =
      optstruct->getMyRelations();
  myRelations.insert(mRelations.begin(), mRelations.end());

  // merge Operator stacks
  std::stack<std::string> mOperators = optstruct->getMyOperators();
  appendToOpStack(mOperators);
  
  // merge groupclause
  bool groupbyClauseUsed = optstruct->getGroupbyClauseUsed();
  if (groupbyClauseUsed)
  {
     setGroupClause(true);
  }
 
  // merge aggregationalias
  bool aggregationAliasUsed = optstruct->getAggregationAlias();
  if (aggregationAliasUsed)
  {
    setAggregationAlias(true); 
  }
  
  // merge aggregationoperator
  bool aggregationoperatorUsed = optstruct->getAggregationoperatorUsed();
  if (aggregationoperatorUsed)
  {
    setAggregationoperatorUsed(true); 
  }
  }
}

void OptParseStruct::addNewName(std::string newname)
{
  if (newNames.find(newname) != newNames.end())
  {
    string erromessage = "Name " + newname + " was already used. ";
    addErrorMessage(erromessage);
  }
  else
  {
    newNames.insert(newname);
  }

}

void OptParseStruct::addRelation(std::string relationname,
    std::string relationalias)
{
  ListExpr type = plnl->TheEmptyList();
  string realname;
  if (!optutils::isObject(relationname, realname, type))
  {
    string errormessage = "Object " + string(relationname)
        + " not known in the database.";
    addErrorMessage(errormessage);
  }
  else
  {
    //cout << "*** Object " << string(relationname)
    //    << " is known in the database \n" << endl;
  }

  if (!optutils::isRelDescription(type, "rel"))
  {
    string errormessage = "The object " + relationname
        + " is not a relation.";
    addErrorMessage(errormessage);
  }
  else
  {
    //cout << "*** Object " << string(relationname) << " is is a relation \n"
    //    << endl;
  }
  myRelations.insert(std::pair<std::string, std::string>(relationname,
      relationalias));
}

void OptParseStruct::addRelation(std::string relationname)
{
  addRelation(relationname, "");
}

/*
~addGroupClauseAttribute~

Adds the name and the alias of an groupattribute in the 
groupbyclause to the OptParseStruct.

*/
void OptParseStruct::addGroupClauseAttribute(std::string attributename, 
     std::string attributealias)
{
     myGroupClauseAttributes.insert(std::pair<std::string, 
     std::string>(attributename, attributealias));
}

void OptParseStruct::addGroupClauseAttribute(std::string attributename)
{
    addGroupClauseAttribute(attributename, "");
}

/*
~addGroupAttribute~

Adds the name and the alias of an attribute to the OptParseStruct.

*/

void OptParseStruct::addGroupAttribute(std::string attributename, 
                     std::string attributealias)
{
     myGroupOpAttributes.insert(std::pair<std::string, 
     std::string>(attributename, attributealias));
}

void OptParseStruct::addGroupAttribute(std::string attributename)
{
     addGroupAttribute(attributename, "");
}

/*
~addAttribute~

Adds the name and the alias of an attribute to the OptParseStruct.

*/
void OptParseStruct::addAttribute(std::string attributename,
    std::string attributealias)
{
  // check if attributename is a parameterless 
  // operator like now, which may not have a qualifier
  list<string> parameters;
  if ((optutils::isOperator(attributename)))
  {
    // Relation alias is not allowed with an parameterless operator
    if (attributealias.size() > 0)
    {
      string errormessage = "Operator " + attributename
          + " may not be qualified with " + attributealias
          + " because it is an operator. ";
      addErrorMessage(errormessage);
    }
    // need to push  type of parameterless operator as optStruct
    std::list<std::string> parameters;
    string type = optutils::getOperatorType(attributename, parameters);
    myOperators.push(type.substr(1, type.length() - 2));

    //cout << "**** OptParseStruct::addAttribute pushed to operatorstack : "
    //    << type << endl;
  }
  else
  {
    myAttributes.insert(std::pair<std::string, std::string>(attributename,
        attributealias));
    string opstack;
    if (attributealias.size() > 0)
    {
      opstack = "attribute:" + attributealias + ":" + attributename;
    }
    else
    {
      opstack = "attribute:" + attributename;
    }
    //std::cout
    //    << "**** OptParseStruct::addAttribute pushed to operatorstack : "
    //    << opstack << endl;

    myOperators.push(opstack);
  }
}
void OptParseStruct::addAttribute(std::string attributename)
{
  addAttribute(attributename, "");
}

/*
~addUsedAlias~

This methods adds aliases to the usedAlias set and generates an error 
if the alias was already used.

*/

void OptParseStruct::addUsedAlias(string aliasname)
{
  //debug
  //for (std::set<std::string>::iterator it = usedAliases.begin(); it
  //    != usedAliases.end(); it++)
  //{
    //std::cout << "------ usedAliases : " << *it << std::endl;
  //}

  //std::cout << "*** OptParseStruct::addUsedAlias : aliasname " << aliasname
  //    << endl;
  //debug end

  if (usedAliases.find(aliasname) != usedAliases.end())
  {
    string erromessage = "Alias " + aliasname + " was already used. ";
    addErrorMessage(erromessage);

  }
  else
  {
    usedAliases.insert(aliasname);
  }
}

/*
~addToOpStack~

Takes a string and checks if it is an operator an adds it to myOpStack, 
if op does exist.

*/

void OptParseStruct::addToOpStack(string opStackEntry)
{
  if (optutils::strequal(opStackEntry.substr(0, 8), "attribute:", true))
  {
    //std::cout << "adding attribute " << opStackEntry
    //    << " to operatorstack " << std::endl;
    myOperators.push(opStackEntry);
  }
  else
  {
    if (optutils::isOperator(opStackEntry))
    {
      string op = "operator:" + opStackEntry;
      myOperators.push(op);
      //std::cout << "Found operator " + opStackEntry << std::endl;
    }
    else
    // else it must be a type
    {
      //std::cout << "adding type " << opStackEntry << " to operatorstack "
      //    << std::endl;
      myOperators.push(opStackEntry);
    }
  }
}

void OptParseStruct::checkAttributes()
{

  // get some infos about objects and relations from the database
  std::set<std::string> dbObjectNames;
  optutils::getDatabaseObjectNames(dbObjectNames);

  //get attributenames which exist for given relations, 
  // put them into map<string,set<string> >
  map<string, set<std::string> > dbAttributeNames; 
  // <--- will contain object names from given relations with attributes
  for (map<string, string>::iterator it = myRelations.begin(); it
      != myRelations.end(); ++it)
  {
    set<string> attributeNameList;
    optutils::getAttributeNames((*it).first, attributeNameList);
    //debug
  //for (set<std::string>::iterator itCheck = attributeNameList.begin(); itCheck
    //    != attributeNameList.end(); itCheck++)
   // {
    //  std::cout << (*itCheck) << std::endl;
   // }
    // debug end
    string relationname = (*it).first;
    dbAttributeNames.insert(std::pair<std::string, std::set<std::string> >(
        relationname, attributeNameList));
  }

  // iterate over all attributes
  for (std::multimap<std::string, std::string>::iterator attributeIT =
      myAttributes.begin(); attributeIT != myAttributes.end(); attributeIT++)
  {
    string attributename = (*attributeIT).first;
    //std::cout << "*** checking OptParseStruct::checkAttributes() "
    //    << attributename << std::endl;

    //  check if attributename is an objectname
    if (dbObjectNames.find(attributename) != dbObjectNames.end())
    {
      string errormessage = attributename
          + " is an objectname. ";
      //std::cout << errormessage << std::endl;
      addErrorMessage(errormessage);
    }

    // check if alias is an objectname
    string aliasname = (*attributeIT).second;
    //std::cout << "*** checking OptParseStruct::checkAttributes() "
        //<< aliasname << std::endl;
    if (aliasname.size() > 0 and dbObjectNames.find(aliasname)
        != dbObjectNames.end())
    {
      string errormessage = "Alias " + aliasname
          + " is an objectname which cannot be used. ";
      //std::cout << errormessage << std::endl;
      addErrorMessage(errormessage);
    }

    // check if attribute has an alias
    if (aliasname.size() == 0)
    {
      //std::cout << "*** checkAttributes " << std::endl;
      bool foundInRelWithoutAlias = false;
      // check in all relations without an alias if 
      // there is an attribute with this name
      for (map<string, set<std::string> >::iterator it =
          dbAttributeNames.begin(); it != dbAttributeNames.end(); it++)
      {
        //std::cout << "*** checkAttributes : Relation  " << (*it).first
        //    << "-- Attribute : " << attributename
        //    << " in relation " << std::endl;
        //debug
        //std::cout << "*** Attributes in " << (*it).first << " : "
        //    << std::endl;
      //for (set<std::string>::iterator itCheck = (*it).second.begin(); itCheck
        //    != (*it).second.end(); itCheck++)
        //{
        //  std::cout << (*itCheck) << std::endl;
        //}
        // debug end

        if ((*it).second.find(attributename) != (*it).second.end())
        {
          //std::cout << "*** checkAttributes : found attribute "
          //    << attributename << " in relation " << std::endl;

          // found relation with attribute, but now there is 
          // need to check if it has got an alias
          pair<std::multimap<std::string, std::string>::iterator,
              std::multimap<std::string, std::string>::iterator>
              rangepair;
          rangepair = myRelations.equal_range((*it).first);
          for (std::multimap<std::string, std::string>::iterator
              itRel = rangepair.first; itRel != rangepair.second; itRel++)
          {
            //cout << " Alias : " << (*itRel).second << std::endl;
            if ((*itRel).second.size() == 0)
            {

              //std::cout << "found " << attributename
              //    << " in Relation " << (*it).first
              //    << std::endl;
              foundInRelWithoutAlias = true;
            }
          }
          if (!foundInRelWithoutAlias)
          {
            string errormessage = "Found " + attributename
                + " in Relation " + (*it).first
                + " but this Relation has an Alias defined.";
            addErrorMessage(errormessage);
          }
        }
        else
        {
          string errormessage =
              "Did not find any Relation which contained "
                  + attributename + " and had no alias.";
          addErrorMessage(errormessage);
        }
      }
    }
    // attribute with an alias
    /*
     * search for alias in myRelation, if found check for attribute
     * if not found check for check all relations for attributes
     *
     */
    else
    {
      bool foundAttrWithAliasInRel = false;
      for (std::multimap<std::string, std::string>::iterator itAlias =
          myRelations.begin(); itAlias != myRelations.end(); itAlias++)
      {
        string myAlias = (*itAlias).second;
        string myRel = (*itAlias).first;
        if ((*itAlias).second == aliasname)
        {
          // check if attribute is in relation with that alias
          for (map<string, set<std::string> >::iterator it =
              dbAttributeNames.begin(); it
              != dbAttributeNames.end(); it++)
          {
            if ((*it).second.find(attributename)
                != (*it).second.end() and ((*it).first
                == (*itAlias).first))
            {
              foundAttrWithAliasInRel = true;
            }
          }
          // found alias, but it does not have the attribute
          if (!foundAttrWithAliasInRel)

          {
            string errormessage = "Found Relation which had alias "
                + aliasname + " but it had no attribute "
                + attributename + ".";

          }

        }

      }
      if (!foundAttrWithAliasInRel)
      {
        string errormessage =
            "Did not find any given Relation which contained "
                + attributename + " and had alias " + aliasname
                + ". ";
        addErrorMessage(errormessage);
      }
    }
  }
}

/*
~appendToOperators~

This method takes a stack<string> and appends it to the bottom of the 
local Operators stack.

*/
void OptParseStruct::appendToOpStack(stack<string> append)
{
  //std::cout << "*** OptParseStruct::appendToOpStack : " << std::endl;
  if (!append.empty())
  {
  stack<string> returnStack;
  stack<string> reverseStack;
  while (!append.empty())
  {
    string element = append.top();
    reverseStack.push(element);
    append.pop();
  }

  while (!reverseStack.empty())
  {
    returnStack.push(reverseStack.top());
    reverseStack.pop();
  }

  while (!myOperators.empty())
  {
    reverseStack.push(myOperators.top());
    myOperators.pop();
  }

  while (!reverseStack.empty())
  {
    returnStack.push(reverseStack.top());
    reverseStack.pop();
  }
  myOperators = returnStack;
  }
}

void OptParseStruct::addOperatorSeparator()
{
  std::stack<string> expressionend;
  expressionend.push(OPSTACKSEPARATOR);
  appendToOpStack(expressionend);
}

stack<string> OptParseStruct::reverseOpStack(stack<string> opstack)
{
  stack<string> reverse;
  while (!opstack.empty())
  {
    reverse.push(opstack.top());
    opstack.pop();
  }
  return reverse;
}


string OptParseStruct::checkOpStack(stack<string> operators)
{
  std::stack<string> optemp;
  while (operators.size() > 0)
  {
    string element = operators.top();
    optemp.push(element);
    operators.pop();
  }

  while (optemp.size() > 0)
  {
    string element = optemp.top();
    operators.push(element);
    optemp.pop();
  }
  // debug end

  string type;

  std::stack<std::string> reversestack = reverseOpStack(operators);
  string operatorname;
  //std::cout << "Size of reversestack : " << reversestack.size() << std::endl;
  // parameterlist for checkOperatorTypeMap2
  std::list<std::string> parameters;
  // reduce the stack until an error occurs or there is only one element left
  while (reversestack.size() > 1)
  {
    //size_t pos = reversestack.top().find_firstof(":");
    // if topelement is operator then getOperatorType is called with it
    string element = reversestack.top();
    //cout << "aktuelles Element : " << element << endl;
    if (element.find_first_of(":") == 8)
    {
      operatorname = element.substr(9);
      type = optutils::getOperatorType(operatorname, parameters);
      //cout << "Type by getOperatorType = " << type << endl;
      if (type.find("undefined") != std::string::npos)
      {
        string errormessage = " Operator " + operatorname
            + " did not accept ";
        for (std::list<std::string>::iterator paramIT =
            parameters.begin(); paramIT != parameters.end(); paramIT++)
        {
          errormessage = errormessage + (*paramIT) + " ";
        }
        errormessage = errormessage + "as parameters.";
        addErrorMessage(errormessage);
      }
      else
      {
        parameters.clear();
        reversestack.pop();
        reversestack.push(type);
      }
    }
    else
    {
      if (element.find_first_of(":") == 9)
      {
        string type = getAttributeType(element.substr(10));
        reversestack.pop();
      }
      else
      {
        //top element should be type
        parameters.push_back(element);
        reversestack.pop();
      }
    }
  }
  /*std::cout << "*** OptParseStruct::checkOpStack returntype: " << type
      << std::endl;*/

  return type;
}

/*
~getAttributeType~

This method tries to determine the type of the attribute by 
checking the given relations.

*/

string OptParseStruct::getAttributeType(string attribute)
{
  size_t colonpos = attribute.find(":");
  string alias;
  string attributename;
  if (colonpos != std::string::npos)
  {
    alias = attribute.substr(0, int(colonpos) - 1);
    attributename = attribute.substr(int(colonpos));
    //std::cout << "alias : " << alias << std::endl;
    //std::cout << "attributename : " << attributename << std::endl;
    // search for alias in myRelations
    string relation;
    for (std::multimap<std::string, std::string>::iterator opIT =
        myRelations.begin(); opIT != myRelations.end(); opIT++)
    {
      if (optutils::strequal((*opIT).second, alias, false))
        relation = (*opIT).first;
    }
    if (relation.size() > 0)
    {
      return optutils::getAttributeType(attributename, relation);
    }
    else
    {
      // maybe too much, because should also be marked by checkattributes
      string errormessage = " Could no determine the type of " + alias
          + ":" + attribute;
      addErrorMessage(errormessage);
      return "undefined";
    }
  }
  // got no alias
  else
  {
    attributename = attribute;
    string relationname;
    for (map<string, string>::iterator it = myRelations.begin(); it
        != myRelations.end(); ++it)
    {
      set<string> attributeNameList;
      optutils::getAttributeNames((*it).first, attributeNameList);

    for (set<std::string>::iterator itCheck = attributeNameList.begin(); itCheck
          != attributeNameList.end(); itCheck++)
      {
        //std::cout << (*itCheck) << std::endl;
        if (optutils::strequal((*itCheck), attributename, false)
            and ((*it).second.size() == 0))
        {
          relationname = (*it).first;
          break;
        }
      }
      if (relationname.size() > 0)
      {
        return optutils::getAttributeType(attributename, relationname);
      }
      else
      {
        return "unknown";
      }
    }
  }
  return "error";
}

/*
~checkOperators~

This method is being called to check the myOperators stack if it does return 
one valid type. Calls the recursive ~checkOpStack~
method to check for nested Operators. Is being called in the Optparser.y when 
given relations are available.

*/
void OptParseStruct::checkOperators()
{ 
  /*
  std::cout << "*** OptParseStruct::checkOperators() " << std::endl;
  std::cout << "*** current stack :" << std::endl;

  //debug
  std::stack<std::string> savestack;
  while (!myOperators.empty())
  {
    savestack.push(myOperators.top());
    std::cout << "*** stack content : " << myOperators.top() << std::endl;
    myOperators.pop();
  }
  // push stuff back onto stack
  while (!savestack.empty())
  {
    myOperators.push(savestack.top());
    savestack.pop();
  }
  // debug end
  */

  if (!myOperators.empty())
  {
    //    checkOpStack(myOperators);

    // split stack by OPSTACKSEPARATOR so test are 
    // individual for each <resultlist> entries
    stack<string> opstack;
    while (!myOperators.empty())
    {
      if (!optutils::strequal(myOperators.top(), OPSTACKSEPARATOR, false))
      {
        opstack.push(myOperators.top());
        myOperators.pop();
      }
      else
      {
        // pass stack part to recursive function and call 
        // checkOpStack if top element is operator
        if (reverseOpStack(opstack).top().find(":") == 8)
        {
          checkOpStack(reverseOpStack(opstack));
        }
        // remove OPSTACKSEPARATOR
        myOperators.pop();
        // clear stack
        while (!opstack.empty())
        {
          opstack.pop();
        }
      } 
      if (myOperators.empty() and opstack.size() > 0 and reverseOpStack(
          opstack).top().find(":") == 8)
      {
        checkOpStack(reverseOpStack(opstack));
      }
    }
  }
  else
  {
    //std::cout << "No Operators to check. " << std::endl;
  }
}

/*
~checkAggregation~

This method is being called to check if the the structure of the aggregation
operators is correct.

*/
void OptParseStruct::checkAggregation()
{
  // if there is a groupby clause, you must use alias for aggregation
  if(groupbyclause && !aggregationalias && aggregationoperator)
  {
     addErrorMessage("If a groupby clause is used, you must use aliases \\
for aggregationoperators.");
  }
  
  // if there is a no groupby clause, you must not use alias for aggregation
  if(!groupbyclause && aggregationalias && aggregationoperator)
  {
     addErrorMessage("If a no groupby clause is used, you may not use \\
aliases for aggregation.");
  }
  // if there is a groupby clause, at least one aggregationoperator
  if(groupbyclause && !aggregationoperator)
  {
     addErrorMessage("If a groupby clause is used, you must use at least"
                     " one aggregationoperator.");
  }
  
}
 
/*
~subqueriesAllowed~

This method checks wether the subqueries option is enabled.

*/
void OptParseStruct::subqueriesAllowed()
{
  if (!optimizerOption("subqueries"))
  {
    addErrorMessage("Subqueries are not enabled at the moment. ");
  }
}

bool OptParseStruct::optimizerOption(std::string name)
{
  predicate_t p;
  p = PL_predicate("optimizerOption", 1, NULL);
  term_t h0 = PL_new_term_refs(1);
  const char * expression = name.c_str();
  PL_put_atom_chars(h0, expression);
  int rval;
  rval = PL_call_predicate(NULL, PL_Q_NORMAL, p, h0);
  return rval;
}

//first get all attributes for the given relations so its done only once.

void OptParseStruct::dumpAllInfo()
{

  std::cout
      << "---------- Information accumulated in OptParseStruct -----------"
      << endl;

  std::cout << "---------- errorMessages -----------" << endl;
  std::cout << "errorMessages :\n" << errorMessages << endl;

  std::cout << "---------- newNames -----------" << endl;
  std::set<std::string>::iterator newnamesIT;
 for (newnamesIT = newNames.begin(); newnamesIT != newNames.end(); newnamesIT++)
  {
    cout << *newnamesIT << endl;
  }
  std::cout << "---------- usedAliases -----------" << endl;
  std::set<std::string>::iterator usedAliasesIT;
  for (usedAliasesIT = usedAliases.begin(); usedAliasesIT
      != usedAliases.end(); usedAliasesIT++)
  {
    cout << *usedAliasesIT << endl;
  }
  std::cout << "---------- attributes -----------" << endl;
  for (map<string, string>::iterator ii = myAttributes.begin(); ii
      != myAttributes.end(); ++ii)
  {
    if (((*ii).second).size() > 0)
    {
      std::cout << (*ii).second << ":" << (*ii).first << std::endl;
    }
    else
    {
      std::cout << (*ii).first << std::endl;
    }
  }

  std::cout << "---------- groupbyattributes -----------" << endl;
  for (map<string, string>::iterator ii = myGroupOpAttributes.begin(); ii
      != myGroupOpAttributes.end(); ++ii)
  {
    if (((*ii).second).size() > 0)
    {
      std::cout << (*ii).second << ":" << (*ii).first << std::endl;
    }
    else
    {
      std::cout << (*ii).first << std::endl;
    }
  }


  std::cout << "---------- groupclauseattributes -----------" << endl;
  for (map<string, string>::iterator ii = myGroupClauseAttributes.begin(); ii
      != myGroupClauseAttributes.end(); ++ii)
  {
    if (((*ii).second).size() > 0)
    {
      std::cout << (*ii).second << ":" << (*ii).first << std::endl;
    }
    else
    {
      std::cout << (*ii).first << std::endl;
    }
  }

  std::cout << "---------- relations -----------" << endl;
  for (map<string, string>::iterator ii = myRelations.begin(); ii
      != myRelations.end(); ++ii)
  {
    if (((*ii).second).size() > 0)
    {
      std::cout << (*ii).first << " as " << (*ii).second << std::endl;
    }
    else
    {
      std::cout << (*ii).first << std::endl;
    }
  }
  std::cout << "---------- operators -----------" << endl;

    string op;
    stack<string> backin;
    while (!myOperators.empty())
    {
      op = myOperators.top();
      cout << op << endl;
      backin.push(op);
      myOperators.pop();
    }
    while (!backin.empty())
    {
      myOperators.push(backin.top());
      backin.pop();
    }
  std::cout << "---------- aggregation and groupbyclause -----------" << endl;
  if(groupbyclause)
  {
    std::cout << "groupbyclause is true" << endl; 
  }  
  if(aggregationalias)
  {
    std::cout << "aggregationalias is true" << endl; 
  }  
  if(aggregationoperator)
  {
    std::cout << "aggregationalias is true" << endl; 
  }  


}

