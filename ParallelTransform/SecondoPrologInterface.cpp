/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
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

#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <list>
#include <unistd.h>
#include "stdlib.h"

// Zugriff auf Transformationen für Prolog Syntax
#include "SWI-Prolog.h"

#include "NestedList.h"
#include "SecondoPrologInterface.h"
#include "Profiles.h"
#include "LogMsg.h"
#include "License.h"
#include "TTYParameter.h"
#include "NList.h"

using namespace std;

/**************************************************************************

1 Implementation of class SecondoInterfaceTTY

*/
static int returnCodeProlog = 0;


string ReplaceString(std::string subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

void
SecondoPrologInterface::startPrologEnginge()
{
    /* initialise Prolog */
    char *plav[2];
    /* make the argument vector for Prolog */

    plav[0] = (char *) "../ParallelTransform/Transform.pl";
    plav[1] = NULL;
    
    if(!returnCodeProlog) {

        returnCodeProlog = PL_initialise(1, plav);        
    
        predicate_t consult = PL_predicate("consult", 1, "system");
        term_t file = PL_new_term_ref();
        PL_put_atom_chars(file, "../ParallelTransform/TransformFacts.pl");
        PL_call_predicate(NULL, PL_Q_NORMAL, consult, file);
        PL_put_atom_chars(file, "../ParallelTransform/Transform.pl");
        PL_call_predicate(NULL, PL_Q_NORMAL, consult, file);
        PL_put_atom_chars(file, "../ParallelTransform/CommonSecondo.pl");
        PL_call_predicate(NULL, PL_Q_NORMAL, consult, file);

    }

}

ListExpr
SecondoPrologInterface::callPrologQueryTransform(ListExpr expr, NestedList* nl)
{
    bool error = false;
    term_t prologList = ListExprToTermForInterface(expr, nl);
    term_t result = PL_new_term_ref();
    // Call Prolog predicate Transform

      { predicate_t pred = PL_predicate("query_to_parallel", 2, "user");
        term_t t0 = PL_new_term_refs(2);
        
        if(! PL_put_term(t0+0, prologList)) {
          std::cerr << "Unable to put term: " << prologList << endl;
        }
        //int rval;

        if(PL_call_predicate(NULL, PL_Q_NORMAL, pred, t0)) {
          if(!PL_put_term(result, t0+1)) {
            std::cerr << "Unable to put term: " << result << endl;
          }
        }

        //PL_halt(rval ? 0 : 1);
        
      }
    
    ListExpr resultList = TermToListExpr(result, nl, error);
    return resultList;
}

/*
Prolog transform Method
*/
term_t
SecondoPrologInterface::ListExprToTermForInterface(ListExpr expr, 
                                                   NestedList* nl)
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
    int ret = 0;
    switch(nl->AtomType(expr))
    {
      case IntType:
        intValue = nl->IntValue(expr);
        ret = PL_put_integer(result, intValue);
        break;

      case RealType:
        realValue = nl->RealValue(expr);
        ret = PL_put_float(result, realValue);
        break;

      case BoolType:
        boolValue = nl->BoolValue(expr);
        ret = PL_put_atom_chars(result, boolValue ? "true" : "false");
        break;

      case StringType:
        stringValue = nl->StringValue(expr);
        stringRepr = string("'\"") + stringValue + string("\"'");
        ret = PL_put_atom_chars(result, stringRepr.c_str());
        break;

      case SymbolType:
        stringValue = nl->SymbolValue(expr);    
    {
        char cValue[stringValue.size()];
        strcpy(cValue, stringValue.c_str());
        char c = cValue[0];
        stringRepr = "";
        if(isupper(c)) {
            stringRepr = string("'<") + stringValue + string(">'");
            ret = PL_put_atom_chars(result, stringRepr.c_str());
                break;
        }
    }

    if(! ret) {
       cerr << "Unable to put atom for prolog" << endl;
    }

    //transform(stringValue.begin(), stringValue.end(), 
  //stringValue.begin(), ::tolower);
        PL_put_atom_chars(result, stringValue.c_str());
        break;

      case TextType:
        scan = nl->CreateTextScan(expr);
        nl->GetText(scan, nl->TextLength(expr) + 2, stringValue);
        nl->DestroyTextScan(scan);
    transform(stringValue.begin(), stringValue.end(), 
            stringValue.begin(), ::tolower);
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
      elem = ListExprToTermForInterface(*iter, nl);
      int ret = PL_cons_list(result, elem, result);
      if(! ret) {
        cerr << "Unable to execute PL_cons_list" << endl;
      }
    }
  }

  return result;
}

/*

4 Function TermToListExpr

Converts a PROLOG term to a ListExpr.

*/
ListExpr
SecondoPrologInterface::TermToListExpr(term_t t, NestedList* nl, bool& error)
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
  int ret = 0;

  switch(PL_term_type(t))
  {
    case PL_VARIABLE:
      error = true;
      result = nl->TheEmptyList();
      break;

    case PL_INTEGER:
      ret = PL_get_long(t, &intValue);
      result = nl->IntAtom(intValue);
      break;

    case PL_FLOAT:
      ret = PL_get_float(t, &realValue);
      result = nl->RealAtom(realValue);
      break;

    case PL_ATOM:
      ret = PL_get_atom_chars(t, &charValue);
      result = AtomToListExpr(nl, charValue, error);
      break;

    case PL_STRING:
      ret = PL_get_string_chars(t, &charValue, &len);
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

  if(! ret) {
     cerr << "Unable to get data from prolog" << endl;
  }

  return result;
}


/*

4 Function AtomToListExpr

Converts a PROLOG atom (represented as a string) to
a ListExpr.

*/
ListExpr
SecondoPrologInterface::AtomToListExpr(NestedList* nl, char* str, bool& error)
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
    atomStr = str;
    atomStr = ReplaceString(atomStr, "\"", "");      
    result = nl->SymbolAtom(atomStr);

  }
  else if(str[0] == '\'')
  {
    atomStr = str;
    if(str[1] == '<') {
        atomStr = ReplaceString(atomStr, "<", "");
        atomStr = ReplaceString(atomStr, ">", "");    
        atomStr = ReplaceString(atomStr, "\'", "");
        result = nl->SymbolAtom(atomStr);
    } else {
        atomStr = ReplaceString(atomStr, "\"", "");
        atomStr = ReplaceString(atomStr, "\'", "");
        result = nl->StringAtom(atomStr);
    }
    
  }
  else
  {
    result = nl->SymbolAtom(string(str));
  }

  return result;
}

