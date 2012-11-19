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

This is the source code of a PROLOG shell which has
support for calling Secondo from PROLOG.

2 Includes and defines

*/

#include <string.h>
#include <iostream>
#include <list>

#include "stdlib.h"

#include "SWI-Prolog.h"
#include "SecondoPL.h"

using namespace std;

// NVK ADDED
#ifdef SECONDO_USE_MEMORY_ALLOCATION
#include "MemoryOptimization.h"
//#include "QueryProcessor.h"
#endif
// NVK ADDED END

#include "NestedList.h"
#include "SecondoInterface.h"
#include "Profiles.h"
#include "LogMsg.h"
#include "License.h"
#include "TTYParameter.h"
#include "NList.h"


#include "../OptParser/OptimizerChecker.h"


#ifdef SECONDO_USE_ENTROPY
#include "../Optimizer/Entropy/entropy.h"
#endif

SecondoInterface* si = 0;
NestedList* plnl = 0;

int lastErrorCode = 0;
string lastErrorMessage = "";

/*

3 Function handle\_exit   

This function is registerd as exit handler in the main function.

*/
void handle_exit(void) {
  
  /* PROLOG interpreter has terminated, shutdown Secondo */
  if(si != 0)
  {
    try {
      si->Terminate();
      delete si;
      si = 0;
    }
    catch (SecondoException e)
    {
       cerr << e.msg() << endl;    
    }
  };

}


/*

3 Function ListExprToTerm

Converts a Secondo list expression to a PROLOG term.

*/
term_t
ListExprToTerm(ListExpr expr, NestedList* nl)
{
  ListExpr current;
  int length;
  int i;
  std::list<ListExpr> listElements;
  std::list<ListExpr>::iterator iter;

  long intValue;
  double realValue;
  bool boolValue;
  string stringValue;
  string stringRepr;
  TextScan scan;

  term_t elem;
  term_t result = PL_new_term_ref();

  if(nl->IsAtom(expr))
  {
    switch(nl->AtomType(expr))
    {
      case IntType:
        intValue = nl->IntValue(expr);
        PL_put_integer(result, intValue);
        break;

      case RealType:
        realValue = nl->RealValue(expr);
        PL_put_float(result, realValue);
        break;

      case BoolType:
        boolValue = nl->BoolValue(expr);
        PL_put_atom_chars(result, boolValue ? "true" : "false");
        break;

      case StringType:
        stringValue = nl->StringValue(expr);
        stringRepr = string("\"") + stringValue + '\"';
        PL_put_atom_chars(result, stringRepr.c_str());
        break;

      case SymbolType:
        stringValue = nl->SymbolValue(expr);
        PL_put_atom_chars(result, stringValue.c_str());
        break;

      case TextType:
        scan = nl->CreateTextScan(expr);
        nl->GetText(scan, nl->TextLength(expr) + 2, stringValue);
        nl->DestroyTextScan(scan);
        PL_put_atom_chars(result, stringValue.c_str());
        break;

      default:
        /* this should not happen */
        assert(false);
    }
  }
  else
  {
    PL_put_nil(result);
    current = expr;
    length = nl->ListLength(current);

    for(i = 0; i < length; i++)
    {
      listElements.push_front(nl->First(current));
      current = nl->Rest(current);
    }

    for(iter = listElements.begin(); iter != listElements.end(); iter++)
    {
      elem = ListExprToTerm(*iter, nl);
      PL_cons_list(result, elem, result);
    }
  }

  return result;
}

/*

4 Function AtomToListExpr

Converts a PROLOG atom (represented as a string) to
a ListExpr.

*/
ListExpr
AtomToListExpr(NestedList* nl, char* str, bool& error)
{
  ListExpr result;
  string atomStr;

  error = false;

  if(strcmp(str, "true") == 0)
  {
    result = nl->BoolAtom(true);
  }
  else if(strcmp(str, "false") == 0)
  {
    result = nl->BoolAtom(false);
  }
  else if(str[0] == '\"')
  {
    str++;
    atomStr = str;

    if(atomStr.size() > 0 && atomStr[atomStr.size() - 1] == '\"')
    {
      atomStr.erase(atomStr.size() - 1);
      result = nl->StringAtom(atomStr);
    }
    else
    {
      error = true;
      result = nl->TheEmptyList();
    }
  }
  else
  {
    result = nl->SymbolAtom(string(str));
  }

  return result;
}

/*

4 Function TermToListExpr

Converts a PROLOG term to a ListExpr.

*/
ListExpr
TermToListExpr(term_t t, NestedList* nl, bool& error)
{
  long intValue;
  double realValue;
  char* charValue;
  char* strValue;
  size_t len;

  term_t head;
  term_t list;

  std::list<ListExpr> elementList;
  std::list<ListExpr>::iterator iter;

  ListExpr result;

  error = false;

  switch(PL_term_type(t))
  {
    case PL_VARIABLE:
      error = true;
      result = nl->TheEmptyList();
      break;

    case PL_INTEGER:
      PL_get_long(t, &intValue);
      result = nl->IntAtom(intValue);
      break;

    case PL_FLOAT:
      PL_get_float(t, &realValue);
      result = nl->RealAtom(realValue);
      break;

    case PL_ATOM:
      PL_get_atom_chars(t, &charValue);
      result = AtomToListExpr(nl, charValue, error);
      break;

    case PL_STRING:
      PL_get_string_chars(t, &charValue, &len);
      strValue = new char[len + 1];
      strValue[len] = 0;
      memcpy(strValue, charValue, len);
      result = AtomToListExpr(nl, strValue, error);
      delete[] strValue;
      break;

    case PL_TERM:
      if(PL_is_list(t))
      {
        result = nl->TheEmptyList();
        head = PL_new_term_ref();
        list = PL_copy_term_ref(t);

        while(PL_get_list(list, head, list))
        {
          elementList.push_front(TermToListExpr(head, nl, error));
          if(error)
            break;
        }

        if(!error)
        {
          for(iter = elementList.begin(); iter != elementList.end(); iter++)
          {
            result = nl->Cons(*iter, result);
          }
        }
        else
        {
          result = nl->TheEmptyList();
        }
      }
      else
      {
        error = true;
        result = nl->TheEmptyList();
      }
      break;

    default:
      assert(false); /* should not happen */
  }

  return result;
}

// some code below can only be translated
// when the Optimizaton-Library (OPT++) is
// present
#ifdef SECONDO_USE_ENTROPY

/*

4 Function FloatListToVector

Converts a PROLOG list of float numbers to a Vector of float numbers (double).

*/
void
FloatListToVector(term_t t, std::vector<double>& v, bool& error)
{
  error = 1;
  if( PL_is_list(t) )
  {
    term_t head = PL_new_term_ref();
    term_t list = PL_copy_term_ref(t);

    v.clear();
    while( PL_get_list(list, head, list) )
    {
      double d;

      if ( PL_get_float(head, &d) )
        v.push_back(d);
      else
        return;
    }

    error = 0;
  }
}

/*

4 Function FloatListPairToVectorPair

Converts a PROLOG list of (integer,float) pair to a vector of (int,float) pair.

*/
void
FloatListPairToVectorPair( term_t t, 
                           ProbabilityPairVec& v, 
                           bool& error                        )
{
  error = 1;
  if( PL_is_list(t) )
  {
    // Outer list
    term_t o_head = PL_new_term_ref();
    term_t o_list = PL_copy_term_ref(t);

    v.clear();
    while( PL_get_list(o_list, o_head, o_list) )
    {
      ProbabilityPair p;
      term_t i_head = PL_new_term_ref();
      term_t i_list = PL_copy_term_ref(o_head);

      PL_get_list(i_list, i_head, i_list);
      if ( PL_get_integer(i_head, &p.first) )
        if (    PL_get_list(i_list, i_head, i_list) 
             && PL_get_float(i_head, &p.second)     )
          v.push_back(p);
        else
          return;
      else
        return;
    }

    error = 0;
  }
}

/*
4 Function FloatVectorToList

Converts a Vector of float numbers to a PROLOG list of float numbers (double).

*/
void
FloatVectorToList(ProbabilityVec& v,term_t& t, bool& error)
{
  term_t list = PL_copy_term_ref(t);
  term_t head = PL_new_term_ref();
  ProbabilityVec::iterator iter;

  error = 1;
  for( iter = v.begin(); iter != v.end(); iter++ )
  {
    if ( !PL_unify_list(list, head, list) || !PL_unify_float(head, *iter) )
      return;
  }

  error = !PL_unify_nil(list);
}

/*
4 Function FloatVectorPairToListPair

Converts a vector of (int,float) numbers to a PROLOG list of (integer,float).

*/
void
FloatVectorPairToListPair( ProbabilityPairVec& v,
                           term_t& t, bool& error             )
{
  // Outer list
  term_t o_list = PL_copy_term_ref(t);
  term_t o_head = PL_new_term_ref();
  ProbabilityPairVec::iterator iter;

  error = 1;
  for( iter = v.begin(); iter != v.end(); iter++ )
  {
    if ( !PL_unify_list(o_list, o_head, o_list) ) return;

    term_t i_list = PL_copy_term_ref(o_head);
    term_t i_head = PL_new_term_ref();

    if ( !PL_unify_list(i_list, i_head, i_list) ) return;
    if ( !PL_unify_integer(i_head, iter->first) ) return;
    if ( !PL_unify_list(i_list, i_head, i_list) ) return;
    if ( !PL_unify_float(i_head, iter->second) ) return;
    if ( !PL_unify_nil(i_list) ) return;
  }

  error = !PL_unify_nil(o_list);
}

#endif

/*

4 Function pl\_print\_term\_le

Converts a PROLOG term to a ListExpr and then prints
that ListExpr using the routines from NestedList.

*/
static foreign_t
pl_print_term_le(term_t term)
{
  ListExpr listLE;
  bool error;

  listLE = TermToListExpr(term, plnl, error);
  if(error)
  {
    PL_fail;
  }
  else
  {
    // for debugging
    // plnl->WriteListExpr(listLE);
    PL_succeed;
  }
}

/*

4 Function pl\_get\_error\_info

Get error code (an integer) and error message (a string)
of the last issued Secondo command.

*/
static foreign_t
pl_get_error_info(term_t errorCode, term_t errorMessage)
{
  int unify1 = PL_unify_integer(errorCode, lastErrorCode);
 
  string msg = SecondoInterface::GetErrorMessage(lastErrorCode) + "\n" 
               + lastErrorMessage;
  
  if( (unify1 != 0) && PL_unify_atom_chars( errorMessage, msg.c_str()) != 0)
  {
    PL_succeed;
  }
  else
  {
    PL_fail;
  }
}

/*

4 Function pl\_call\_secondo

Call Secondo. The first argument must either be an atom
representing a query in text format or it must be
a PROLOG list. The result is a PROLOG nested list
which is unified with the second argument. The command level is
set to executable. If something goes wrong, the predicate fails and
error information can be obtained via predicate secondo\_error\_info.

*/
static foreign_t
pl_call_secondo(term_t command, term_t result)
{
  bool error = false;
  char* commandCStr;
  string commandStr;
  int commandLevel;

  ListExpr commandLE = plnl->TheEmptyList();
  ListExpr resultList = plnl->TheEmptyList();
  int errorPos;

  if(PL_get_atom_chars(command, &commandCStr))
  {
    error = false;
    commandStr = commandCStr;
    /* executable command in text syntax */
    commandLevel = 1;
  }
  else
  {
    commandLE = TermToListExpr(command, plnl, error);
    /* executable command in nested list syntax */
    commandLevel = 0;
    if (error)
    {
      cerr << "SecondoPL: TermToListExpr() failed." << endl; 
      PL_fail;
    }       
  }

  lastErrorCode = 0;
  si->Secondo(commandStr,
              commandLE,
              commandLevel,
              false,
              false,
              resultList,
              lastErrorCode,
              errorPos,
              lastErrorMessage);

  // reset NestedList pointer for the NList interface
  NList::setNLRef(plnl);

  if(lastErrorCode != 0)
  {
      PL_fail;
  }
  else
  {
    if(PL_unify(result, ListExprToTerm(resultList, plnl)) != 0)
    {
      plnl->initializeListMemory();
      PL_succeed;
    }
    else
    {
      cerr << "SecondoPL: Predicate secondo/2 failed, but error code was 0."
           << endl; 
      plnl->WriteListExpr(resultList);
    }
  }

  PL_fail;
}



static foreign_t getOperatorIndexes(
      term_t nameP,    // input: name of operator
      term_t argList,  // input: argumentlist
      term_t resList,  // output: resultlist
      term_t algId,    // output: algebra id
      term_t opId,     // output: operator id
      term_t funId){   // output: value mapping id
   // convert operator name into a c++ string
   char* nameC;

   if(!PL_get_atom_chars(nameP, &nameC)){ // name not given
     cout << "cannot convert name" << endl;
     PL_fail;
   }
	 // NVK REMOVED MA: to much output
   //cout << "name = '" << nameC << "'" << endl;
   // convert argument list from prolog to C++
   bool error = false;
   ListExpr argListC = TermToListExpr(argList, si->GetNestedList(), error);
   if(error){
     cout << "cannot convert arglist" << endl;
     PL_fail;    
   }
   // NVK removed:
   // cout << "arglist = " << si->GetNestedList()->ToString(argListC) << endl; 
   int algIdC;
   int opIdC;
   int funIdC;
   ListExpr resListC;
   // ask System for correct values
   bool ok = si->getOperatorIndexes(nameC, argListC, resListC, 
                                    algIdC, opIdC, funIdC,si->GetNestedList()); 
   if(!ok){
      //cout << "Operator not available for arguments" << endl; // NVK
      PL_fail;
   }
   // convert results back to prolog terms
   PL_unify(resList, ListExprToTerm(resListC, si->GetNestedList()));
   PL_unify_integer(algId,algIdC);
   PL_unify_integer(opId,opIdC);
   PL_unify_integer(funId,funIdC); 
   // todo: error handling in conversion
   PL_succeed;
}


/*
~getCosts~

The next functions return costs for a specified operator when number of tuples
and size of a single tuple is given. If the operator does not provide a
cost estimation function or the getCost function is not implemented,
the return value is false.

*/

static foreign_t  getCosts7(
              term_t algId,
              term_t opId,
              term_t funId,
              term_t noTuples,
              term_t sizeOfTuple,
              term_t memoryMB,
              term_t costs) {
   // convert arguments to C
   int algIdC,opIdC,funIdC,noTuplesC,sizeOfTupleC,memoryMBC;
   size_t costsC = 0;
   if(!PL_get_integer(algId, &algIdC)){
     cerr << "algId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(opId, &opIdC)){
     cerr << "opId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(funId, &funIdC)){
     cerr << "funId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(noTuples, &noTuplesC)){
     cerr << "noTuples is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(sizeOfTuple, &sizeOfTupleC)){
     cerr << "sizeOfTuple is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(memoryMB, &memoryMBC)){
     cerr << "sizeOfTuple is not an integer" << endl;
     PL_fail;
   }
   if(!si->getCosts(algIdC,opIdC,funIdC,
                     (size_t) noTuplesC, (size_t) sizeOfTupleC,
                     (size_t) memoryMBC, costsC)){
      PL_fail;
   }  else {
      int intCosts = (int) costsC;
      PL_unify_integer(costs,intCosts);
      PL_succeed;
   }
}


static foreign_t getCosts9(
              term_t algId,
              term_t opId,
              term_t funId,
              term_t noTuples1,
              term_t sizeOfTuple1,
              term_t noTuples2,
              term_t sizeOfTuple2,
              term_t memoryMB,
              term_t costs) {
   // convert arguments to C
   int algIdC,opIdC,funIdC,noTuples1C,sizeOfTuple1C,
       noTuples2C,sizeOfTuple2C,memoryMBC;
   size_t costsC = 0;
   if(!PL_get_integer(algId, &algIdC)){
     cerr << "algId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(opId, &opIdC)){
     cerr << "opId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(funId, &funIdC)){
     cerr << "funId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(noTuples1, &noTuples1C)){
     cerr << "noTuples1 is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(sizeOfTuple1, &sizeOfTuple1C)){
     cerr << "sizeOfTuple1 is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(noTuples2, &noTuples2C)){
     cerr << "noTuples2 is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(sizeOfTuple2, &sizeOfTuple2C)){
     cerr << "sizeOfTuple2 is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(memoryMB, &memoryMBC)){
     cerr << "sizeOfTuple is not an integer" << endl;
     PL_fail;
   }
   if(!si->getCosts(algIdC,opIdC,funIdC,
                     (size_t) noTuples1C, (size_t) sizeOfTuple1C,
                     (size_t) noTuples2C, (size_t) sizeOfTuple2C,
                     (size_t) memoryMBC, costsC)){
      PL_fail;
   }  else {
      int intCosts = (int) costsC;
      PL_unify_integer(costs,intCosts);
      PL_succeed;
   }
}

/*
~getLinearParams~

Retrieves the parameters for estimating the cost function of an operator
in a linear way.

*/
static foreign_t getLinearParams8( 
                      term_t algId,
                      term_t opId,
                      term_t funId,
                      term_t noTuples1,
                      term_t sizeOfTuple1,
                      term_t sufficientMemory,
                      term_t timeAtSuffMemory,
                      term_t timeAt16MB) {
    
   int algIdC,opIdC,funIdC,noTuples1C,sizeOfTuple1C;
   if(!PL_get_integer(algId, &algIdC)){
     cerr << "algId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(opId, &opIdC)){
     cerr << "opId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(funId, &funIdC)){
     cerr << "funId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(noTuples1, &noTuples1C)){
     cerr << "noTuples1 is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(sizeOfTuple1, &sizeOfTuple1C)){
     cerr << "sizeOfTuple1 is not an integer" << endl;
     PL_fail;
   }
   double sufficientMemoryC = 0;
   double timeAtSuffMemoryC = 0;
   double timeAt16MBC = 0;
   if(!si->getLinearParams(algIdC,opIdC,funIdC,noTuples1C,sizeOfTuple1C,
                           sufficientMemoryC,timeAtSuffMemoryC,timeAt16MBC)){
      PL_fail;
   } else {
      PL_unify_float(sufficientMemory,sufficientMemoryC);
      PL_unify_float(timeAtSuffMemory, timeAtSuffMemoryC);
      PL_unify_float(timeAt16MB, timeAt16MBC);
      PL_succeed;
   }
}


static foreign_t getLinearParams10( 
          term_t algId,
          term_t opId,
          term_t funId,
          term_t noTuples1,
          term_t sizeOfTuple1,
          term_t noTuples2,
          term_t sizeOfTuple2,
          term_t sufficientMemory,
          term_t timeAtSuffMemory,
          term_t timeAt16MB) {

   int algIdC, opIdC, funIdC, noTuples1C, noTuples2C, 
       sizeOfTuple2C, sizeOfTuple1C;
   if(!PL_get_integer(algId, &algIdC)){
     cerr << "algId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(opId, &opIdC)){
     cerr << "opId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(funId, &funIdC)){
     cerr << "funId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(noTuples1, &noTuples1C)){
     cerr << "noTuples1 is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(sizeOfTuple1, &sizeOfTuple1C)){
     cerr << "sizeOfTuple1 is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(noTuples2, &noTuples2C)){
     cerr << "noTuples2 is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(sizeOfTuple2, &sizeOfTuple2C)){
     cerr << "sizeOfTuple2 is not an integer" << endl;
     PL_fail;
   }
   double sufficientMemoryC = 0;
   double timeAtSuffMemoryC = 0;
   double timeAt16MBC = 0;
   if(!si->getLinearParams(algIdC,opIdC,funIdC,noTuples1C,
                          sizeOfTuple1C,noTuples2C,sizeOfTuple2C,
                           sufficientMemoryC,timeAtSuffMemoryC,timeAt16MBC)){
      PL_fail;
   } else {
      PL_unify_float(sufficientMemory,sufficientMemoryC);
      PL_unify_float(timeAtSuffMemory, timeAtSuffMemoryC);
      PL_unify_float(timeAt16MB, timeAt16MBC);
      PL_succeed;
   }
}


/*
~getFunction~

Returns an approximation of the cost function of a specified value mapping as
a parametrized function.
dlist will be a list containing 7 double values
(sufficientMemory, timeAtSuffMemory, timeAt16MB, a,b,c,d)


*/
static foreign_t getFunction7(
                 term_t algId,
                 term_t opId,
                 term_t funId,
                 term_t noTuples1,
                 term_t sizeOfTuple1,
                 term_t funType,
                 term_t dlist) {

   int algIdC,opIdC,funIdC,noTuples1C,sizeOfTuple1C;
   if(!PL_get_integer(algId, &algIdC)){
     cerr << "algId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(opId, &opIdC)){
     cerr << "opId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(funId, &funIdC)){
     cerr << "funId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(noTuples1, &noTuples1C)){
     cerr << "noTuples1 is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(sizeOfTuple1, &sizeOfTuple1C)){
     cerr << "sizeOfTuple1 is not an integer" << endl;
     PL_fail;
   }
   int funTypeC = -1;
   double sufficientMemoryC = 0;
   double timeAtSuffMemoryC = 0;
   double timeAt16MBC = 0;
   double aC,bC,cC,dC;
   aC = bC = cC = dC = 0;

   if(!si->getFunction(algIdC,opIdC,funIdC,noTuples1C,sizeOfTuple1C,
              funTypeC, sufficientMemoryC,timeAtSuffMemoryC,timeAt16MBC,
              aC,bC,cC,dC)){
      PL_fail;
   } else {
      PL_unify_integer(funType,funTypeC);
      NestedList* nl = si->GetNestedList();
      ListExpr clist = nl->OneElemList(nl->RealAtom(sufficientMemoryC));
      ListExpr last = clist;
      last = nl->Append(last, nl->RealAtom(timeAtSuffMemoryC));
      last = nl->Append(last, nl->RealAtom(timeAt16MBC));
      last = nl->Append(last, nl->RealAtom(aC));
      last = nl->Append(last, nl->RealAtom(bC));
      last = nl->Append(last, nl->RealAtom(cC));
      last = nl->Append(last, nl->RealAtom(dC));
      PL_unify(dlist, ListExprToTerm(clist, nl));
      PL_succeed;
   }
}
                      

static foreign_t getFunction9(
                 term_t algId,
                 term_t opId,
                 term_t funId,
                 term_t noTuples1,
                 term_t sizeOfTuple1,
                 term_t noTuples2,
                 term_t sizeOfTuple2,
                 term_t funType,
                 term_t dlist ){

   int algIdC,opIdC,funIdC,noTuples1C,sizeOfTuple1C,noTuples2C,sizeOfTuple2C;
   if(!PL_get_integer(algId, &algIdC)){
     cerr << "algId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(opId, &opIdC)){
     cerr << "opId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(funId, &funIdC)){
     cerr << "funId is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(noTuples1, &noTuples1C)){
     cerr << "noTuples1 is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(sizeOfTuple1, &sizeOfTuple1C)){
     cerr << "sizeOfTuple1 is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(noTuples2, &noTuples2C)){
     cerr << "noTuples2 is not an integer" << endl;
     PL_fail;
   }
   if(!PL_get_integer(sizeOfTuple2, &sizeOfTuple2C)){
     cerr << "sizeOfTuple2 is not an integer" << endl;
     PL_fail;
   }
   int funTypeC = -1;
   double sufficientMemoryC = 0;
   double timeAtSuffMemoryC = 0;
   double timeAt16MBC = 0;
   double aC,bC,cC,dC;
   aC = bC = cC = dC = 0;

   if(!si->getFunction(algIdC,opIdC,funIdC,noTuples1C,sizeOfTuple1C, 
              noTuples2C, sizeOfTuple2C,
              funTypeC, sufficientMemoryC,timeAtSuffMemoryC,timeAt16MBC,
              aC,bC,cC,dC)){
      PL_fail;
   } else {
      PL_unify_integer(funType,funTypeC);
      NestedList* nl = si->GetNestedList();
      ListExpr clist = nl->OneElemList(nl->RealAtom(sufficientMemoryC));
      ListExpr last = clist;
      last = nl->Append(last, nl->RealAtom(timeAtSuffMemoryC));
      last = nl->Append(last, nl->RealAtom(timeAt16MBC));
      last = nl->Append(last, nl->RealAtom(aC));
      last = nl->Append(last, nl->RealAtom(bC));
      last = nl->Append(last, nl->RealAtom(cC));
      last = nl->Append(last, nl->RealAtom(dC));
      PL_unify(dlist, ListExprToTerm(clist, nl));
      PL_succeed;
   }
}
                      






/*
This functions check a sql construct (given as an atom)
to be valid using an external parser. On succeed, the
function will fail. Otherwise, the function succeeds and
returns the error message using the second argument.

*/

static foreign_t
pl_check_syntax(term_t command, term_t result)
{

  char* commandstr = 0;

  int ok1 = PL_get_chars(command,&commandstr, CVT_ALL);
  if(!ok1){
     if(!PL_unify_atom_chars(result,"cannot create command")){
         cerr << "Cannot return result, please use an "
              << "Variable as the second argument" << endl;
     }
     PL_succeed;
   }
 
   char* errorMessage = 0;
   bool ok = checkOptimizerQuery(commandstr, errorMessage);

  if(ok){
     if(errorMessage){
        free(errorMessage);
     }
     PL_fail;
  } else {
     if(errorMessage){
        if(!PL_unify_atom_chars(result,errorMessage)){
           cerr << "cannot return the error message, may be "
                << " the second arguemnt is not a variable" << endl;
           cerr << "errorMessage = " << errorMessage << endl;
        }
        free(errorMessage);
     } else {
        if(!PL_unify_atom_chars(result,"No error Message available")){
           cerr << "cannot return the error message, may be "
               << " the second arguemnt is not a variable" << endl;
           cerr << "Internal error, no success, but no error message" << endl;
        }

     }
     PL_succeed;
  }
}


/*
Transfers an option the the sql checker parser. fails if the option is unknown. Or
one of the arguments could not be evaluated

*/

static foreign_t
pl_set_sql_check_option(term_t optionName, term_t enable)
{
  char* optionNameStr = 0;

  // get the option name
  if(! PL_get_chars(optionName,&optionNameStr, CVT_ALL)){
     PL_fail;
   }
 
   // try to get the boolean parameter
   int enableBool;
   if(!PL_get_bool(enable,&enableBool)){
      PL_fail;
   }

   bool ok = setSqlParserOption(optionNameStr, enableBool);
   if(ok){
     PL_succeed;
   } else {
     PL_fail;
   }
}




/*

4 Function pl\_maximize\_entropy

Function to compute the remaining conditional probabilities. 
Syntax:

----
    maximize_entropy( [[1, p1] [2, p2] ... ], [[3, cp1], [5, cp2] ...], R )
----  

pi, cpi are floating point values.

*/
#ifdef SECONDO_USE_ENTROPY



static foreign_t
pl_maximize_entropy(term_t predicates, term_t probabilities, term_t result)
{
  // Type checking.
  if( !PL_is_list(predicates) || !PL_is_list(probabilities) )
    PL_fail;
  else
  {
    //ProbabilityVec vectorPredicates;
    ProbabilityPairVec vectorPredicates, vectorProbabilities, vectorResult;
    bool error1=false, error2=false;

    //FloatListToVector(predicates, vectorPredicates, error1);
    FloatListPairToVectorPair(predicates, vectorPredicates, error1);
    FloatListPairToVectorPair(probabilities, vectorProbabilities, error2);
    if( error1 || error2 )
      PL_fail;
    else
    {
      bool error=false;
      try
      {
        maximize_entropy( vectorPredicates, 
                          vectorProbabilities, 
                          vectorResult         );

        FloatVectorPairToListPair(vectorResult, result, error);
      }
      catch(...)
      {
        PL_fail;
      }

      if( error )
        PL_fail;
    }

    PL_succeed;
  }
}

#endif



/*
NVK ADDED MA
Forwards the GlobalMemory property to the prolog environment.

This should be improved by someone who is more familiar with secondo's kernel.
The code in comments won't work, as soon as i include the QueryProcessor.h, secondo won't compile successfully anymore.
Note that GetGlobalMemory function needs to be implemented fist within QueryProcessor.cpp and QueryProcessor.h.

*/
static foreign_t pl_global_memory(term_t value) {
  string sbd = getenv("SECONDO_BUILD_DIR");
  int globalMem = 512; // Unit: MiB
  if(sbd.length()>0) {
    globalMem = SmiProfile::GetParameter("QueryProcessor", "GlobalMemory", 512,
      sbd + "/bin/SecondoConfig.ini");
  }
  else
    PL_fail;
  /*
  QueryProcessor& qp = *SecondoSystem::GetQueryProcessor();
  size_t globalMem=qp.GetGlobalMemory();
  */
  int unify1 = PL_unify_integer(value, globalMem);
  if(unify1 != 0) {
    PL_succeed;
  }
  else {
    PL_fail;
  }
}
// NVK ADDED MA END

PL_extension predicates[] =
{
  { "secondo", 2, (void*)pl_call_secondo, 0 },
  { "secondo_error_info", 2, (void*)pl_get_error_info, 0 },
  { "secondo_print_le", 1, (void*)pl_print_term_le, 0 },
  { "check_syntax",2, (void*)pl_check_syntax,0},
  { "set_sql_check_option",2,(void*)pl_set_sql_check_option,0},
  { "getOpIndexes",6,(void*)getOperatorIndexes,0},
  { "getCosts",7,(void*)getCosts7,0},
  { "getCosts",9,(void*)getCosts9,0},
  { "getLinearCostFun",8,(void*)getLinearParams8,0},
  { "getLinearCostFun",10,(void*)getLinearParams10,0},
  { "getCostFun",7,(void*)getFunction7,0},
  { "getCostFun",9,(void*)getFunction9,0},
#ifdef SECONDO_USE_ENTROPY
  { "maximize_entropy", 3, (void*)pl_maximize_entropy, 0 },
#endif
  // NVK ADDED MA
#ifdef SECONDO_USE_MEMORY_ALLOCATION
  { "secondo_global_memory", 1, (void*)pl_global_memory, 0 },
  { "memoryOptimization", 6, (void*)pl_memoryOptimization, 0 },
#endif
  // NVK ADDED MA END

  { 0, 0, 0, 0 } /* terminating line */
};


/*

8 Function StartSecondoC

Starts Secondo. Assumes that the argument is the name of the
configuration file. Return true iff successful.

*/
bool
StartSecondoC(TTYParameter& tp)
{
  if ( !tp.CheckConfiguration() ) {
    return false;
  }  
  //tp.Print(cout);  

  si = new SecondoInterface();
  string errorMsg("");
  if(si->Initialize(tp.user, tp.pswd, tp.host, tp.port, tp.parmFile, errorMsg))
  {
    plnl = si->GetNestedList();
    NList::setNLRef(plnl);
    return true;
  }
  else
  {
    delete si;
    si = 0;
    cout
      << "Error while starting Secondo with config file "
      << tp.parmFile << "." << endl
      << errorMsg << endl;
    return false;
  }
}


/* 

9. registerSecondo

This function registers the secondo predicate at the prolog engine.

*/

int registerSecondo(){
  
  //cout << "register secondo" << endl;
  
  TTYParameter tp(0,0);

  atexit(handle_exit);

  if( !StartSecondoC(tp) )
  {
    return -1;
  }

  /* Start PROLOG interpreter with our extensions. */
  PL_register_extensions(predicates);
  return 0;
}



