/*
\def\CC{C\raise.22ex\hbox{{\footnotesize +}}\raise.22ex\hbox{\footnotesize +}\xs
pace}
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

#include <iostream>
#include <list>

#include <SWI-Prolog.h>

#include "SecondoInterface.h"

SecondoInterface* si = 0;
NestedList* plnl = 0;

int lastErrorCode;
string lastErrorMessage;

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
        PL_put_atom_chars(result, "text_atom");
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
  unsigned int len;

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

/*

4 Function pl_print_term_le

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
    plnl->WriteListExpr(listLE);
    PL_succeed;
  }
}

/*

4 Function pl_get_error_info

Get error code (an integer) and error message (a string)
of the last issued Secondo command.

*/
static foreign_t
pl_get_error_info(term_t errorCode, term_t errorMessage)
{
  if(PL_unify_integer(errorCode, lastErrorCode) != 0
     && PL_unify_atom_chars(errorMessage, lastErrorMessage.c_str()) != 0)
  {
    PL_succeed;
  }
  else
  {
    PL_fail;
  }
}

/*

4 Function pl_call_secondo

Call Secondo. The first argument must either be an atom
representing a query in text format or it must be
a PROLOG list. The result is a PROLOG nested list
which is unified with the second argument. The command level is
set to executable. If something goes wrong, the predicate fails and
error information can be obtained via predicate secondo_error_info.

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
  }

  if(!error)
  {
    si->Secondo(commandStr,
                commandLE,
                commandLevel,
                false,
                false,
                resultList,
                lastErrorCode,
                errorPos,
                lastErrorMessage);

    if(lastErrorCode != 0)
    {
      PL_fail;
    }
    else
    {
      if(PL_unify(result, ListExprToTerm(resultList, plnl)) != 0)
      {
        PL_succeed;
      }
    }
  }

  PL_fail;
}

PL_extension predicates[] =
{
  { "secondo", 2, pl_call_secondo, 0 },
  { "secondo_error_info", 2, pl_get_error_info, 0 },
  { "secondo_print_le", 1, pl_print_term_le, 0 },
  { NULL, 0, NULL, 0 } /* terminating line */
};

/*

7 Function GetConfigFileNameFromArgV

Attempts to retrieve the configuration file name via the
enviroment. If it is found, it is returned. Otherwise,
search the arguments of the main function for two sucessive
strings of the form -c FileName. These two strings are then
removed from the argument vector and the argument count is
reduced by 2. Returns the FileName on success and NULL otherwise.

*/
char* GetConfigFileNameFromArgV(int& argc, char** argv)
{
  int i = 0;
  int j;
  char* result;

  if((result = getenv("SECONDO_CONFIG")) != 0)
  {
    return result;
  }

  while(i < argc - 1 && strcmp(argv[i], "-c") != 0)
  {
    ++i;
  }

  if(i < argc - 1)
  {
    result = argv[i + 1];
    for(j = i + 2; j < argc; j++)
    {
      argv[j - 2] = argv[j];
    }
    argc -= 2;
    return result;
  }
  else
  {
    return 0;
  }
}

/*

8 Function StartSecondoC

Starts Secondo. Assumes that the argument is the name of the
configuration file. Return true iff successful.

*/
bool
StartSecondoC(char* configFileName)
{
  string user = "";
  string pswd = "";
  string host = "";
  string port = "";
  string configFile = configFileName;

  si = new SecondoInterface();

  if(si->Initialize(user, pswd, host, port, configFile))
  {
    plnl = si->GetNestedList();
    return true;
  }
  else
  {
    delete si;
    si = 0;
    cout
      << "Error while starting Secondo with config file "
      << configFile << "." << endl;
    return false;
  }
}

int
main(int argc, char **argv)
{
  char* configFile;
  int success;

  /* Start Secondo and remove Secondo command line arguments
     from the argument vector .*/
  configFile = GetConfigFileNameFromArgV(argc, argv);
  if(configFile == 0 || !StartSecondoC(configFile))
  {
    cout << "Usage : SecondoPL -c ConfigFileName" << endl;
    exit(1);
  }

  /* Start PROLOG interpreter with our extensions. */
  PL_register_extensions(predicates);

  if(!PL_initialise(argc, argv))
  {
    PL_halt(1);
  }

  PL_install_readline();
  success = PL_toplevel();

  /* PROLOG interpreter has terminated, shutdown Secondo */
  if(si != 0)
  {
    si->Terminate();
    delete si;
  };
  cout << "Terminating ... " << endl;

  PL_halt(success ? 0 : 1);
}
