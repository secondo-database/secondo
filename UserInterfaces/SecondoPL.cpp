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


// NVK ADDED
#ifdef SECONDO_USE_MEMORY_ALLOCATION
#include "MemoryOptimization.h"
//#include "QueryProcessor.h"
#endif
// NVK ADDED END


#include "NestedList.h"
#include "SecondoInterface.h"

#if !defined(SECONDO_CLIENT_SERVER) && !defined(REPLAY)
#include "SecondoInterfaceTTY.h"
#elif defined(SECONDO_CLIENT_SERVER)
#include "SecondoInterfaceCS.h"
#elif defined(REPLAY)
#include "SecondoInterfaceREPLAY.h"
#else
#include "SecondoInterfaceCS.h"
#endif

#include "Profiles.h"
#include "LogMsg.h"
#include "License.h"
#include "TTYParameter.h"
#include "NList.h"

#include "../OptParser/OptimizerChecker.h"


#ifdef SECONDO_USE_ENTROPY
#include "../Optimizer/Entropy/entropy.h"
#endif


using namespace std;

SecondoInterface* si = 0;
NestedList* plnl = 0;

int lastErrorCode = 0;
string lastErrorMessage = "";

/*

3 Function handle\_exit   

This function is registered as exit handler in the main function.

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
    catch (SecondoException &e)
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
        if(!PL_put_integer(result, intValue)){
          assert(false);
        }
        break;

      case RealType:
        realValue = nl->RealValue(expr);
        if(!PL_put_float(result, realValue)){
          assert(false);
        }
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
      if(!PL_cons_list(result, elem, result)){
        assert(false);
      }
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
      if(!PL_get_long(t, &intValue)){
        cerr << "PL_get_long failed" << endl;
      }
      result = nl->IntAtom(intValue);
      break;

    case PL_FLOAT:
      if(!PL_get_float(t, &realValue)){
        cerr << "PL_get_float failed" << endl;
      }
      result = nl->RealAtom(realValue);
      break;

    case PL_ATOM:
      if(!PL_get_atom_chars(t, &charValue)){
        cerr << "PL_get_atom failed" << endl;
      }
      result = AtomToListExpr(nl, charValue, error);
      break;

    case PL_STRING:
      if(!PL_get_string_chars(t, &charValue, &len)){
        cerr << "PL_get_string failed" << endl;
      }
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
  bool error;

  TermToListExpr(term, plnl, error);
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

   // convert argument list from prolog to C++
   bool error = false;
   ListExpr argListC = TermToListExpr(argList, si->GetNestedList(), error);
   if(error){
     cout << "cannot convert arglist" << endl;
     PL_fail;    
   }
   int algIdC;
   int opIdC;
   int funIdC;
   ListExpr resListC;
   // ask System for correct values
   bool ok = si->getOperatorIndexes(nameC, argListC, resListC, 
                                    algIdC, opIdC, funIdC,si->GetNestedList()); 
   if(!ok){
      //cout << "OPerator not available for arguments" << endl;
      PL_fail;
   }
   // convert results back to prolog terms
   if(!PL_unify(resList, ListExprToTerm(resListC, si->GetNestedList()))){
      cerr << "PL_unify failed" << endl;
   }
   if(!PL_unify_integer(algId,algIdC)){
     cerr << "PL_unify_interger failed" << endl;
   }
   if(!PL_unify_integer(opId,opIdC)){
     cerr << "PL_unify_integer failed" << endl;
   }
   if(!PL_unify_integer(funId,funIdC)){
     cerr << "PL_unify_integer failed" << endl;
   }
   // todo: error handling in conversion
   PL_succeed;
}

/*
~readIntList~

if ~t~ is a list consisting of size integer values, these integer 
values are written into the buffer res which must have enough space 
for these integers. If this is successful, true is returns, 
false otherwise.

*/

static bool readIntList(term_t t, int* res,size_t size){

  if(!PL_is_list(t)){
     return false;
  }
  term_t head = PL_new_term_ref();
  term_t list = PL_copy_term_ref(t);
  size_t pos = 0;
  while(PL_get_list(list,head,list )){
     if(pos>size){
        cerr << "list has more than " << size << " elements" << endl;
        return false;;
     }
     if(!PL_get_integer(head, &res[pos])){
        cerr << "element " << pos << " is not an integer" << endl;
        return false;;
     }
     pos++;
  }
  if(pos<size){
     cerr << "list has less than " << size << " elements" << endl;
     return false;
  }
  return true;
}



/*
~getCosts~

The next functions return costs for a specified operator when number of tuples
and size of a single tuple is given. If the operator does not provide a
cost estimation function or the getCost function is not implemented,
the return value is false.

The arguments of this functions are:

  ( [ algId, opId, funId ] ,
    [ noTuples, sizeOfTuple, noAttributes ],
    selectivity, memoryMB, costs )

Where costs is a return parameter. The first arguments are coded in a list
because prolog supports only 10 arguments for foreign functions. For this 
functions this limits will be sufficient, but for versions using two input 
streams not. For compability with other functions, we also here use lists
of integer values.

*/




static foreign_t  getCosts5(
              term_t VM_descr,
              term_t stream1_descr,
              term_t selectivity,
              term_t memoryMB,
              term_t costs) {
   // convert arguments to C
   int memoryMBC;
   double selectivityC;
   size_t costsC = 0;
   int VM_descrC[3];
   int stream1_descrC[3];

   if(!readIntList(VM_descr,VM_descrC,3)){
     cerr << " invalid [algid, opid, vmid] list " << endl;
     PL_fail;
   } 
   if(!readIntList(stream1_descr,stream1_descrC,3)){
     cerr << "invalid stream description" << endl;
     cerr << "must be [noTuples, sizeOfTuple, noAttributes] " << endl;
     PL_fail;
   }

   if(!PL_get_float(selectivity,&selectivityC)){
     cerr << "selectivity is not a float" << endl;
     PL_fail;   
   }
   if(!PL_get_integer(memoryMB, &memoryMBC)){
     cerr << "memoryMB is not an integer" << endl;
     PL_fail;
   }
   if(!si->getCosts( (size_t) VM_descrC[0], 
                     (size_t) VM_descrC[1],
                     (size_t) VM_descrC[2],
                     (size_t) stream1_descrC[0], 
                     (size_t) stream1_descrC[1], 
                     (size_t) stream1_descrC[2], 
                     selectivityC,
                     (size_t) memoryMBC, costsC)){
      PL_fail;
   }  else {
      int intCosts = (int) costsC;
      if(!PL_unify_integer(costs,intCosts)){
       cerr << "PL_unify_integer failed" << endl;
      }
      PL_succeed;
   }
}


static foreign_t getCosts6(
              term_t VM_descr,
              term_t stream1_descr,
              term_t stream2_descr,
              term_t selectivity,
              term_t memoryMB,
              term_t costs) {

   int memoryMBC;
   double selectivityC;
   size_t costsC = 0;
   int VM_descrC[3];
   int stream1_descrC[3];
   int stream2_descrC[3];

   if(!readIntList(VM_descr,VM_descrC,3)){
     cerr << " invalid [algid, opid, vmid] list " << endl;
     PL_fail;
   } 
   if(!readIntList(stream1_descr,stream1_descrC,3)){
     cerr << "invalid stream1 description" << endl;
     cerr << "must be [noTuples, sizeOfTuple, noAttributes] " << endl;
     PL_fail;
   }
   if(!readIntList(stream2_descr,stream2_descrC,3)){
     cerr << "invalid stream2 description" << endl;
     cerr << "must be [noTuples, sizeOfTuple, noAttributes] " << endl;
     PL_fail;
   }

   if(!PL_get_float(selectivity,&selectivityC)){
     cerr << "selectivity is not a float" << endl;
     PL_fail;
   }
   if(!PL_get_integer(memoryMB, &memoryMBC)){
     cerr << "memory is not an integer" << endl;
     PL_fail;
   }

   if(!si->getCosts(
          (size_t) VM_descrC[0], (size_t) VM_descrC[1], (size_t) VM_descrC[2],
          (size_t) stream1_descrC[0], 
          (size_t) stream1_descrC[1], 
          (size_t) stream1_descrC[2], 
          (size_t) stream2_descrC[0], 
          (size_t) stream2_descrC[1], 
          (size_t) stream2_descrC[2], 
           selectivityC, (size_t) memoryMBC, costsC)){
      PL_fail;
   }  else {
      int intCosts = (int) costsC;
      if(!PL_unify_integer(costs,intCosts)){
        cerr << "PL_unify_integer failed" << endl;
      }
      PL_succeed;
   }
}

/*
~getLinearParams~

Retrieves the parameters for estimating the cost function of an operator
in a linear way.

*/
static foreign_t getLinearParams6( 
                      term_t VM_descr,
                      term_t stream1_descr,
                      term_t selectivity,
                      term_t sufficientMemory,
                      term_t timeAtSuffMemory,
                      term_t timeAt16MB) {
    
   int VM_descrC[3];
   if(!readIntList(VM_descr,VM_descrC,3)){
      cerr << "invalid [algId, opId, funId] list" << endl;
      PL_fail;
   }   
   int stream1_descrC[3];
   if(!readIntList(stream1_descr,stream1_descrC,3)){
     cerr << "invalid [noTuples, sizeOfTuples, noAttribute] list" << endl;
     PL_fail;
   }
   double selectivityC;
   if(!PL_get_float(selectivity, &selectivityC)){
     cerr << "selectivity is not a float" << endl;
     PL_fail;
   }
   double sufficientMemoryC = 0;
   double timeAtSuffMemoryC = 0;
   double timeAt16MBC = 0;
   if(!si->getLinearParams(
       VM_descrC[0], VM_descrC[1], VM_descrC[2],
       stream1_descrC[0], stream1_descrC[1], stream1_descrC[2],
       selectivity, sufficientMemoryC,timeAtSuffMemoryC,timeAt16MBC)){
      PL_fail;
   } else {
     if(
           !PL_unify_float(sufficientMemory,sufficientMemoryC)
        || !PL_unify_float(timeAtSuffMemory, timeAtSuffMemoryC)
        || !PL_unify_float(timeAt16MB, timeAt16MBC)){
        cerr << "PL_unify_float failed" << endl;
      }
      PL_succeed;
   }
}


static foreign_t getLinearParams7( 
          term_t VM_descr,
          term_t stream1_descr,
          term_t stream2_descr,
          term_t selectivity,
          term_t sufficientMemory,
          term_t timeAtSuffMemory,
          term_t timeAt16MB) {

   int VM_descrC[3];
   if(!readIntList(VM_descr, VM_descrC, 3)){
     cerr << "invalid [algId, opId, funId] list" << endl;
     PL_fail;
   }
   int stream1_descrC[3];
   if(!readIntList(stream1_descr, stream1_descrC,3)){
     cerr << "invalid [noTuples1, tupleSize1, noAttributes1 ] list" << endl;
     PL_fail;
   }
   int stream2_descrC[3];
   if(!readIntList(stream2_descr, stream2_descrC,3)){
     cerr << "invalid [noTuples2, tupleSize2, noAttributes2 ] list" << endl;
     PL_fail;
   }
   double selectivityC;
   if(!PL_get_float(selectivity, &selectivityC)){
     cerr << "selectivity  is not a float" << endl;
     PL_fail;
   }
   double sufficientMemoryC = 0;
   double timeAtSuffMemoryC = 0;
   double timeAt16MBC = 0;
   if(!si->getLinearParams(
      VM_descrC[0], VM_descrC[1], VM_descrC[2],
      stream1_descrC[0], stream1_descrC[1], stream1_descrC[2],
      stream2_descrC[0], stream2_descrC[1], stream2_descrC[2],
      selectivityC, sufficientMemoryC,timeAtSuffMemoryC,timeAt16MBC)){
      PL_fail;
   } else {
     if(   !PL_unify_float(sufficientMemory,sufficientMemoryC)
        || !PL_unify_float(timeAtSuffMemory, timeAtSuffMemoryC)
        || !PL_unify_float(timeAt16MB, timeAt16MBC)){
        cerr << "PL_unify_floar failed" << endl;
      }
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
static foreign_t getFunction5(
                 term_t VM_descr,
                 term_t stream1_descr,
                 term_t selectivity,
                 term_t funType,
                 term_t dlist) {

   int VM_descrC[3];
   if(!readIntList(VM_descr, VM_descrC, 3)){
      cerr << "invalid [algId, opId, funId] list" << endl;
      PL_fail;
   }
   int stream1_descrC[3];
   if(!readIntList(stream1_descr, stream1_descrC,3)){
      cerr << "invalid [noTuples1, noTuples1, noAttributes1 ] list" << endl;
      PL_fail;
   }
   double selectivityC;
   if(!PL_get_float(selectivity, &selectivityC)){
     cerr << "selectivity is not a float" << endl;
     PL_fail;
   }
   int funTypeC = -1;
   double sufficientMemoryC = 0;
   double timeAtSuffMemoryC = 0;
   double timeAt16MBC = 0;
   double aC,bC,cC,dC;
   aC = bC = cC = dC = 0;

   if(!si->getFunction(
            VM_descrC[0], VM_descrC[1], VM_descrC[2],
            stream1_descrC[0], stream1_descrC[1], stream1_descrC[2],
            selectivityC, 
            funTypeC, sufficientMemoryC,timeAtSuffMemoryC,timeAt16MBC,
            aC,bC,cC,dC)){
      PL_fail;
   } else {
      if(!PL_unify_integer(funType,funTypeC)){
        cerr << "PL_unify_integer failed" << endl;
      }
      NestedList* nl = si->GetNestedList();
      ListExpr clist = nl->OneElemList(nl->RealAtom(sufficientMemoryC));
      ListExpr last = clist;
      last = nl->Append(last, nl->RealAtom(timeAtSuffMemoryC));
      last = nl->Append(last, nl->RealAtom(timeAt16MBC));
      last = nl->Append(last, nl->RealAtom(aC));
      last = nl->Append(last, nl->RealAtom(bC));
      last = nl->Append(last, nl->RealAtom(cC));
      last = nl->Append(last, nl->RealAtom(dC));
      if(!PL_unify(dlist, ListExprToTerm(clist, nl))){
        cerr << "PL_unify failed" << endl;
      }
      PL_succeed;
   }
}
                      

static foreign_t getFunction6(
                 term_t VM_descr,
                 term_t stream1_descr,
                 term_t stream2_descr,
                 term_t selectivity,
                 term_t funType,
                 term_t dlist ){

   int VM_descrC[3];
   if(!readIntList(VM_descr,VM_descrC,3)){
     cerr << "invalid [algId, opId, funId] list" << endl;
     PL_fail;
   }
   int stream1_descrC[3];
   if(!readIntList(stream1_descr, stream1_descrC, 3)){
     cerr << "invalid [noTuples1, noTuples1, noAttributes1] list" << endl;
     PL_fail;
   }
   int stream2_descrC[3];
   if(!readIntList(stream2_descr, stream2_descrC, 3)){
     cerr << "invalid [noTuples2, noTuples2, noAttributes2] list" << endl;
     PL_fail;
   }
   double selectivityC;
   if(!PL_get_float(selectivity, &selectivityC)){
     cerr << "selectivity  is not a float" << endl;
     PL_fail;
   }
   int funTypeC = -1;
   double sufficientMemoryC = 0;
   double timeAtSuffMemoryC = 0;
   double timeAt16MBC = 0;
   double aC,bC,cC,dC;
   aC = bC = cC = dC = 0;

   if(!si->getFunction(
             VM_descrC[0], VM_descrC[1], VM_descrC[2],
             stream1_descrC[0], stream1_descrC[1], stream1_descrC[2],
             stream2_descrC[0], stream2_descrC[1], stream2_descrC[2],
             selectivity,
              funTypeC, sufficientMemoryC,timeAtSuffMemoryC,timeAt16MBC,
              aC,bC,cC,dC)){
      PL_fail;
   } else {
      if(!PL_unify_integer(funType,funTypeC)){
        cerr << "PL_unify_integer failed" << endl;
      }
      NestedList* nl = si->GetNestedList();
      ListExpr clist = nl->OneElemList(nl->RealAtom(sufficientMemoryC));
      ListExpr last = clist;
      last = nl->Append(last, nl->RealAtom(timeAtSuffMemoryC));
      last = nl->Append(last, nl->RealAtom(timeAt16MBC));
      last = nl->Append(last, nl->RealAtom(aC));
      last = nl->Append(last, nl->RealAtom(bC));
      last = nl->Append(last, nl->RealAtom(cC));
      last = nl->Append(last, nl->RealAtom(dC));
      if(!PL_unify(dlist, ListExprToTerm(clist, nl))){
        cerr << "PL_unify failed" << endl;
      }
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
Transfers an option the the sql checker parser. Fails, if the option is unknown
or one of the arguments could not be evaluated.

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

  /*
static foreign_t pl_global_memory(term_t value) {
  string sbd = getenv("SECONDO_BUILD_DIR");
  int globalMem = 512; // Unit: MiB
  if(sbd.length()>0) {
    globalMem = SmiProfile::GetParameter("QueryProcessor", "GlobalMemory", 512,
      sbd + "/bin/SecondoConfig.ini");
  }
  else
    PL_fail;
  int unify1 = PL_unify_integer(value, globalMem);
  if(unify1 != 0) {
    PL_succeed;
  }
  else {
    PL_fail;
  }
}
  */
// NVK ADDED MA END


#if PLVERSION>60000
#define  PL_FUNCTION_T pl_function_t
#else
#define PL_FUNCTION_T void*
#endif


PL_extension predicates[] =
{
  { "secondo", 2, (PL_FUNCTION_T)pl_call_secondo, 0 },
  { "secondo_error_info", 2, (PL_FUNCTION_T)pl_get_error_info, 0 },
  { "secondo_print_le", 1, (PL_FUNCTION_T)pl_print_term_le, 0 },
  { "check_syntax",2, (PL_FUNCTION_T)pl_check_syntax,0},
  { "set_sql_check_option",2,(PL_FUNCTION_T)pl_set_sql_check_option,0},
  { "getOpIndexes",6,(PL_FUNCTION_T)getOperatorIndexes,0},
  { "getCosts",5,(PL_FUNCTION_T)getCosts5,0},
  { "getCosts",6,(PL_FUNCTION_T)getCosts6,0},
  { "getLinearCostFun",6,(PL_FUNCTION_T)getLinearParams6,0},
  { "getLinearCostFun",7,(PL_FUNCTION_T)getLinearParams7,0},
  { "getCostFun",5,(PL_FUNCTION_T)getFunction5,0},
  { "getCostFun",6,(PL_FUNCTION_T)getFunction6,0},
#ifdef SECONDO_USE_ENTROPY
  { "maximize_entropy", 3, (PL_FUNCTION_T)pl_maximize_entropy, 0 },
#endif
  // NVK ADDED MA
#ifdef SECONDO_USE_MEMORY_ALLOCATION
  { "secondo_global_memory", 1, (PL_FUNCTION_T)pl_global_memory, 0 },
  { "memoryOptimization", 6, (PL_FUNCTION_T)pl_memoryOptimization, 0 },
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
  #if !defined(SECONDO_CLIENT_SERVER) && !defined(REPLAY)
  si = new SecondoInterfaceTTY();
  #elif defined(SECONDO_CLIENT_SERVER)
  si = new SecondoInterfaceCS(false,0,true);
  #elif defined(REPLAY)
  si = new SecondoInterfaceREPLAY();
  #else
  si = new SecondoInterfaceCS(false,0,true);
  #endif
  string errorMsg("");
  if(si->Initialize(tp.user, tp.pswd, tp.host, tp.port, tp.parmFile, tp.home,
                    errorMsg))
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


/*

10. Embedding the optimizer into an already running SECONDO process

The following functions let a server process (a forked ~SecondoBDB -srv~) run
the optimizer in-process, so it can accept the SQL dialect over the network and
turn it into an executable plan. In contrast to ~registerSecondo~ they do NOT
create a second ~SecondoInterface~/SMI environment: the server hands us its own
interface, which the optimizer's ~secondo/2~ callbacks then reuse for catalog
and statistics access against the very database the plan will run on.

*/

static bool embeddedOptimizerInitialized = false;

bool embeddedOptimizerReady() {
  return embeddedOptimizerInitialized;
}

bool initEmbeddedOptimizer(SecondoInterface* serverSi) {
  if ( embeddedOptimizerInitialized ) {
    return true;
  }
  if ( serverSi == 0 ) {
    return false;
  }

  // Reuse the server's interface for the optimizer's secondo/2 callbacks
  // instead of opening a second SMI environment.
  si = serverSi;
  plnl = serverSi->GetNestedList();
  NList::setNLRef(plnl);

  // Register the foreign predicates (secondo/2, getCosts, ...).
  PL_register_extensions(predicates);

  // Start the Prolog engine once for this process.
  char* plav[2];
  plav[0] = (char*) "SecondoBDB";
  plav[1] = NULL;
  if ( !PL_initialise(1, plav) ) {
    si = 0;
    plnl = 0;
    return false;
  }

  // Force the Prolog libraries to be loaded (see SecondoPLMode).
  {
    term_t t0 = PL_new_term_refs(2);
    PL_put_atom_chars(t0, "x");
    if ( !PL_put_list_chars(t0 + 1, "") ) {
      cerr << "SecondoPL: problem initializing the prolog interface" << endl;
    }
    predicate_t p = PL_predicate("member", 2, "");
    PL_call_predicate(NULL, PL_Q_NORMAL, p, t0);
  }

  // Preload library(error) before restricting autoloading below. SWI's
  // listing.pl (pulled in while the optimizer loads) uses error:must_be/2 at
  // term-expansion time; with autoloading restricted to the user module that
  // predicate would otherwise fail to resolve and spam "exception handler
  // failed to define error:must_be/2" errors during optimizer startup.
  {
    term_t errLib = PL_new_term_ref();
    term_t errArg = PL_new_term_ref();
    PL_put_atom_chars(errArg, "error");
    if ( PL_cons_functor(errLib, PL_new_functor(PL_new_atom("library"), 1),
                         errArg) ) {
      predicate_t useModule = PL_predicate("use_module", 1, "");
      PL_call_predicate(NULL, PL_Q_NORMAL, useModule, errLib);
    }
  }

#if PLVERSION > 80000
  // Restrict autoloading for SWI-Prolog 8.x/9.x compatibility.
  {
    term_t t = PL_new_term_refs(2);
    PL_put_atom_chars(t, "autoload");
    PL_put_atom_chars(t + 1, "user");
    predicate_t p = PL_predicate("set_prolog_flag", 2, "");
    PL_call_predicate(NULL, PL_Q_NORMAL, p, t);
  }
#endif

  // The optimizer program lives in $SECONDO_BUILD_DIR/Optimizer and consults
  // its parts relative to that directory. A forked server runs in bin/, so
  // steer Prolog's working directory there (without changing the process CWD,
  // which would disturb the storage manager).
  const char* buildDir = getenv("SECONDO_BUILD_DIR");
  if ( buildDir != 0 ) {
    string optDir = string(buildDir) + "/Optimizer";
    term_t t = PL_new_term_refs(2);
    PL_put_variable(t);
    PL_put_atom_chars(t + 1, optDir.c_str());
    predicate_t p = PL_predicate("working_directory", 2, "");
    PL_call_predicate(NULL, PL_Q_NORMAL, p, t);
  }

  // Load the optimizer.
  {
    term_t a0 = PL_new_term_refs(1);
    predicate_t p = PL_predicate("consult", 1, "");
    PL_put_atom_chars(a0, "auxiliary");
    PL_call_predicate(NULL, PL_Q_NORMAL, p, a0);
    PL_put_atom_chars(a0, "calloptimizer");
    PL_call_predicate(NULL, PL_Q_NORMAL, p, a0);
  }

  embeddedOptimizerInitialized = true;
  return true;
}

// Run the optimizer's own secondo/1 predicate (auxiliary.pl) with a single
// command atom. Unlike the foreign secondo/2, this clause also maintains the
// optimizer's databaseName fact and catalog on "open database" / "close
// database". Returns true if the goal succeeded.
static bool runOptimizerSecondo1(const string& command) {
  fid_t fid = PL_open_foreign_frame();
  term_t a = PL_new_term_refs(1);
  PL_put_atom_chars(a, command.c_str());
  predicate_t p = PL_predicate("secondo", 1, "");
  int rc = PL_call_predicate(NULL, PL_Q_CATCH_EXCEPTION, p, a);
  PL_discard_foreign_frame(fid);
  return rc != 0;
}

// True if the optimizer already tracks database dbLower (its databaseName fact,
// which it stores lower-cased).
static bool optimizerHasDatabase(const string& dbLower) {
  fid_t fid = PL_open_foreign_frame();
  term_t a = PL_new_term_refs(1);
  PL_put_atom_chars(a, dbLower.c_str());
  predicate_t p = PL_predicate("databaseName", 1, "");
  int rc = PL_call_predicate(NULL, PL_Q_CATCH_EXCEPTION, p, a);
  PL_discard_foreign_frame(fid);
  return rc != 0;
}

bool embeddedOptimizerUseDatabase(const string& dbName) {
  if ( !embeddedOptimizerInitialized ) {
    return false;
  }
  string dbLower = dbName;
  for ( size_t i = 0; i < dbLower.size(); i++ ) {
    dbLower[i] = tolower((unsigned char) dbLower[i]);
  }
  if ( optimizerHasDatabase(dbLower) ) {
    return true;  // schema already loaded for this database
  }
  // The database is open at the kernel, but the optimizer has not loaded its
  // schema. Drive the optimizer's open-database logic (which asserts
  // databaseName and runs updateCatalog) exactly as the OptServer does: close
  // the currently open database, then reopen it through the optimizer. The
  // database ends up open again, transparently to the client.
  runOptimizerSecondo1("close database");
  runOptimizerSecondo1(string("open database ") + dbName);
  return optimizerHasDatabase(dbLower);
}

/*
SQL text normalization. The clients used to do this each for themselves (the
JavaGUI in ~rewriteForOptimizer~/~varToLowerCase~/~replacePoint~), which made the
accepted syntax depend on which client you used. Now that the SQL is optimized
server side it belongs here, so every client -- TTY, GUI, WebUI, JDBC -- accepts
the same input.

*/

static bool isIdentChar(char c) {
  return isalnum((unsigned char) c) || c == '_';
}

// The optimizer turns identifiers into Prolog atoms, which must start lower
// case. Lower-case the FIRST character of each identifier only (the rest is
// left alone, exactly as the JavaGUI does). Quoted strings ("...") and text
// constants ('...') are never touched.
static string sqlIdentsToLower(const string& str) {
  string buf;
  buf.reserve(str.size());
  int state = 0;   // 0 outside, 1 in "..", 2 in '..', 3 inside an identifier
  for ( size_t i = 0; i < str.size(); i++ ) {
    char c = str[i];
    switch ( state ) {
      case 1:
        if ( c == '"' )  { state = 0; }
        buf += c;
        break;
      case 2:
        if ( c == '\'' ) { state = 0; }
        buf += c;
        break;
      case 3:
        if ( isIdentChar(c) ) {
          buf += c;
        } else {
          buf += c;
          if      ( c == '"' )  { state = 1; }
          else if ( c == '\'' ) { state = 2; }
          else                  { state = 0; }
        }
        break;
      default:
        if ( c == '"' )  { state = 1; buf += c; }
        else if ( c == '\'' ) { state = 2; buf += c; }
        else if ( isalpha((unsigned char) c) ) {
          buf += (char) tolower((unsigned char) c);
          state = 3;
        }
        else { buf += c; }
        break;
    }
  }
  return buf;
}

// Users commonly write attribute access as "r.name"; the SQL dialect spells it
// "r:name". Replace a '.' that sits directly inside an identifier and is
// followed by a letter -- so decimal numbers such as 3.14 stay untouched.
static string sqlDotToColon(const string& str) {
  string buf;
  buf.reserve(str.size());
  int state = 0;   // 0 outside, 1 in "..", 2 in '..', 3 inside an identifier
  for ( size_t i = 0; i < str.size(); i++ ) {
    char c = str[i];
    switch ( state ) {
      case 1:
        if ( c == '"' )  { state = 0; }
        buf += c;
        break;
      case 2:
        if ( c == '\'' ) { state = 0; }
        buf += c;
        break;
      case 3:
        if ( isIdentChar(c) ) { buf += c; }
        else if ( c == '"' )  { state = 1; buf += c; }
        else if ( c == '\'' ) { state = 2; buf += c; }
        else if ( c == '.' ) {
          if ( i + 1 < str.size() && isalpha((unsigned char) str[i+1]) ) {
            buf += ':';
          } else {
            buf += c;
          }
          state = 0;
        }
        else { buf += c; state = 0; }
        break;
      default:
        if ( c == '"' )  { state = 1; buf += c; }
        else if ( c == '\'' ) { state = 2; buf += c; }
        else if ( isalpha((unsigned char) c) ) { state = 3; buf += c; }
        else { buf += c; }
        break;
    }
  }
  return buf;
}

static string normalizeSqlText(const string& sql) {
  return sqlDotToColon( sqlIdentsToLower( sql ) );
}

// Lower-cased first whitespace/'('-delimited token of cmd ("" if none).
static string firstToken(const string& cmd) {
  size_t s = cmd.find_first_not_of(" \t\n\r\f\v");
  if ( s == string::npos ) {
    return "";
  }
  size_t e = s;
  while ( e < cmd.size() && isalpha((unsigned char) cmd[e]) ) {
    e++;
  }
  string tok = cmd.substr(s, e - s);
  for ( size_t i = 0; i < tok.size(); i++ ) {
    tok[i] = tolower((unsigned char) tok[i]);
  }
  return tok;
}

// Recognize "let <ident> = <select|union|intersection> ...". Only the SQL
// on the right-hand side can be optimized; the generated plan is re-wrapped
// as "let <ident> = <plan>" afterwards. The identifier keeps its spelling
// (it names the object being created), so it is split off before normalization.
static bool splitLetPrefix(const string& sql, string& ident, string& rest) {
  size_t p = sql.find_first_not_of(" \t\n\r\f\v");
  if ( p == string::npos || sql.compare(p, 3, "let") != 0 ) {
    return false;
  }
  size_t q = p + 3;
  if ( q >= sql.size() || !isspace((unsigned char) sql[q]) ) {
    return false;
  }
  q = sql.find_first_not_of(" \t\n\r\f\v", q);
  if ( q == string::npos || !isalpha((unsigned char) sql[q]) ) {
    return false;
  }
  size_t idStart = q;
  while ( q < sql.size() && isIdentChar(sql[q]) ) {
    q++;
  }
  string id = sql.substr(idStart, q - idStart);
  q = sql.find_first_not_of(" \t\n\r\f\v", q);
  if ( q == string::npos || sql[q] != '=' ) {
    return false;
  }
  q = sql.find_first_not_of(" \t\n\r\f\v", q + 1);
  if ( q == string::npos ) {
    return false;
  }
  string tail = sql.substr(q);
  string tok = firstToken(tail);
  if ( tok != "select" && tok != "union" && tok != "intersection" ) {
    return false;   // not an SQL right-hand side: leave it to the kernel
  }
  ident = id;
  rest = tail;
  return true;
}

bool embeddedOptimizerSqlChangesCatalog(const string& sql) {
  string ident = "", rest = "";
  if ( splitLetPrefix( sql, ident, rest ) ) {
    return true;             // "let X = select ..." creates a new object
  }
  string first = firstToken( sql );
  if ( first == "drop" ) {
    return true;             // drop table/index
  }
  if ( first == "create" ) {
    return true;             // create table/index
  }
  return false;
}

bool embeddedOptimizerRefreshCatalog(string& errMsg) {
  string output = "";
  return embeddedOptimizerRunGoal( "updateCatalog", output, errMsg );
}

/*
~sqlToPlan/3~ reports a failure as the atom ~::ERROR::~ followed by the message
*written back as a Prolog term* (~term\_to\_atom/2~), so what arrives here is
quoted and escaped, e.g.

----
'\\n\\nSQL ERROR (usually a user error): Unknown symbol: \\'x\\' is not recognized!'
----

Undo that by reading the text back as a term and taking its text: that reverses
~writeq~ exactly, whatever escapes it used. Only the Secondo Javagui ever
decoded this (its ~formatOptimizerError~ hand-rolled four of the escapes), so
every other client printed the raw form; doing it here gives all of them --
GUI, JDBC, both TTYs -- a readable message and keeps the decoding in one place.

*/
static string prologQuotedToPlain(const string& quoted) {
  string plain = quoted;

  fid_t fid = PL_open_foreign_frame();
  term_t t = PL_new_term_ref();
  // PL_chars_to_term expects a term closed by a full stop, as in
  // embeddedOptimizerRunGoal below.
  string readable = quoted + " .";
  char* text = 0;
  if ( PL_chars_to_term( readable.c_str(), t )
       && PL_get_chars( t, &text, CVT_ATOM | CVT_STRING ) ) {
    plain = text;
  } else {
    // A syntax error leaves an exception pending; drop it so it cannot
    // surface at the next Prolog call.
    PL_clear_exception();
  }
  PL_discard_foreign_frame(fid);

  size_t b = plain.find_first_not_of(" \t\r\n");
  if ( b == string::npos ) {
    return quoted;                     // nothing but whitespace: keep the raw
  }
  size_t e = plain.find_last_not_of(" \t\r\n");
  return plain.substr(b, e - b + 1);
}

bool embeddedSqlToPlan(const string& sql, string& plan, double& costs,
                       string& errMsg, bool& alreadyExecuted) {
  plan = "";
  costs = 0.0;
  errMsg = "";
  alreadyExecuted = false;
  if ( !embeddedOptimizerInitialized ) {
    errMsg = "optimizer not initialized";
    return false;
  }

  // "let <ident> = <sql>": optimize only the right-hand side and re-wrap below.
  string letIdent = "", sqlPart = sql;
  bool isLet = splitLetPrefix( sql, letIdent, sqlPart );

  string normalized = normalizeSqlText( sqlPart );

  fid_t fid = PL_open_foreign_frame();
  term_t a0 = PL_new_term_refs(3);      // a0 = sql, a0+1 = plan, a0+2 = costs
  PL_put_atom_chars(a0, normalized.c_str());
  predicate_t p = PL_predicate("sqlToPlan", 3, "");
  bool ok = true;
  // Decoded only after the query is closed, so the nested read stays
  // outside it.
  string rawError = "";
  bool haveRawError = false;

  try {
    qid_t id = PL_open_query(NULL, PL_Q_CATCH_EXCEPTION, p, a0);
    if ( PL_next_solution(id) ) {
      char* res = 0;
      if ( PL_get_atom_chars(a0 + 1, &res) ) {
        string answer(res);
        if ( answer.compare(0, 9, "::ERROR::") == 0 ) {
          rawError = answer.substr(9);
          haveRawError = true;
          ok = false;
        } else {
          // sqlToPlan yields the estimate as a Prolog number -- an integer 0
          // where it has none, a float otherwise -- and PL_get_float reads
          // both. A non-number would be a contract violation; leave costs at
          // 0.0 rather than failing the optimization over it.
          double c = 0.0;
          if ( PL_get_float(a0 + 2, &c) ) {
            costs = c;
          }
          if ( answer == "done" ) {
            // A create/drop command: the optimizer already executed it and
            // there is no plan to run. Keep the sentinel for the wire.
            plan = "done";
            alreadyExecuted = true;
          } else {
            // sqlToPlan returns a bare plan expression; make it a command.
            plan = isLet ? ("let " + letIdent + " = " + answer)
                         : ("query " + answer);
          }
        }
      } else {
        errMsg = "optimization failed";
        ok = false;
      }
    } else {
      errMsg = "optimization failed (no plan found)";
      ok = false;
    }
    PL_close_query(id);
  } catch (...) {
    errMsg = "exception during optimization";
    ok = false;
    haveRawError = false;
  }

  if ( haveRawError ) {
    errMsg = prologQuotedToPlain( rawError );
  }

  PL_discard_foreign_frame(fid);
  return ok;
}

bool embeddedOptimizerRunGoal(const string& goal, string& output,
                              string& errMsg) {
  output = "";
  errMsg = "";
  if ( !embeddedOptimizerInitialized ) {
    errMsg = "optimizer not initialized";
    return false;
  }

  // PL_chars_to_term expects a term closed by a full stop. Trim trailing
  // whitespace and a single trailing '.'/';' (the TTY delimiter), then append
  // the terminator ourselves so callers can pass "showOptions",
  // "setOption(subqueries)", "showOptions." or "showOptions;" alike.
  string goalText = goal;
  while ( !goalText.empty()
          && (goalText[goalText.size()-1] == '\n'
              || goalText[goalText.size()-1] == '\r'
              || goalText[goalText.size()-1] == '\t'
              || goalText[goalText.size()-1] == ' ') ) {
    goalText.erase(goalText.size()-1);
  }
  if ( !goalText.empty()
       && (goalText[goalText.size()-1] == '.'
           || goalText[goalText.size()-1] == ';') ) {
    goalText.erase(goalText.size()-1);
  }
  if ( goalText.empty() ) {
    errMsg = "empty optimizer directive";
    return false;
  }
  goalText += " .";

  fid_t fid = PL_open_foreign_frame();
  bool ok = true;

  try {
    // Build with_output_to(string(S), Goal) so the directive's write/1 output
    // (the showOptions listing, the setOption/delOption confirmation lines) is
    // captured into S instead of going to the server/standalone stdout.
    term_t args = PL_new_term_refs(2);   // args+0 = string(S), args+1 = Goal
    term_t sVar = PL_new_term_ref();     // S (unbound; bound to the output)
    if ( !PL_cons_functor(args, PL_new_functor(PL_new_atom("string"), 1),
                          sVar) ) {
      errMsg = "internal error building output sink";
      PL_discard_foreign_frame(fid);
      return false;
    }
    if ( !PL_chars_to_term(goalText.c_str(), args + 1) ) {
      errMsg = "could not parse optimizer directive";
      PL_discard_foreign_frame(fid);
      return false;
    }

    predicate_t p = PL_predicate("with_output_to", 2, "");
    qid_t id = PL_open_query(NULL, PL_Q_CATCH_EXCEPTION, p, args);
    int rc = PL_next_solution(id);

    // Read the captured output regardless of success/failure.
    char* out = 0;
    if ( PL_get_chars(sVar, &out, CVT_ALL) ) {
      output = out;
    }
    if ( rc == 0 ) {
      ok = false;
      term_t ex = PL_exception(id);
      if ( ex ) {
        char* exChars = 0;
        if ( PL_get_chars(ex, &exChars, CVT_WRITE) ) {
          errMsg = exChars;
        } else {
          errMsg = "optimizer directive raised an exception";
        }
      } else if ( output.empty() ) {
        errMsg = "optimizer directive failed";
      }
    }
    PL_close_query(id);
  } catch (...) {
    errMsg = "exception during optimizer directive";
    ok = false;
  }

  PL_discard_foreign_frame(fid);
  return ok;
}


