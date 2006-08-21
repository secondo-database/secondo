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

August 2004, M. Spiekermann. InitRTFlags introduced. This removes 
some code redundancies. 

Nov 2004, M. Spiekermann. The CMsg instance was moved to the file 
Application.cpp since not all applications are linked with
SecondoInterfaceGeneral.o

February 2006, M. Spiekermann. Function ~WriteErrorList~ was moved into
this implementation file since it was implemented in the SecondoTTY and in
th TestRunner applicaton.

*/

#include <map>

#include "SecondoInterface.h"
#include "SecondoSystem.h"
#include "QueryProcessor.h"
#include "LogMsg.h"
#include "Profiles.h"

using namespace std;

NestedList*
SecondoInterface::GetNestedList()
{
  return (al);
}


void
SecondoInterface::InitRTFlags(const string& configFile) {

  // initialize runtime flags
  string logMsgList = SmiProfile::GetParameter( "Environment", 
                                                "RTFlags", "", configFile );    
  RTFlag::initByString(logMsgList);
  RTFlag::showActiveFlags(cout);
	
}

/*
1.4 Procedure ~GetErrorMessage~

*/
string
SecondoInterface::GetErrorMessage( const int errorCode )
{

  typedef map<int,string> ErrorMap;
  static ErrorMap errors;
  
  errors[ERR_NO_ERROR]            
   = "No Error!";
                           
  errors[ERR_CMD_NOT_RECOGNIZED]  
   = "Command not recognized.";
                           
  errors[ERR_IN_QUERY_EXPR]      
   = "Error in (query) expression.";
                           
  errors[ERR_EXPR_NOT_EVALUABLE]  
   = "Expression not evaluable. (Operator not recognized or stream?)";

  errors[ERR_NO_OBJ_CREATED]   
   = "Error in type expression. No object created.";
                           
  errors[ERR_NO_TYPE_DEFINED]  
   = "Error in type expression. No type defined.";
                           
  errors[ERR_NO_DATABASE_OPEN] 
   = "No database open.";
                           
  errors[ERR_DATABASE_OPEN]    
   = "A database is open.";
                           
  errors[ERR_CREATE_DATABASE]    
   = "Creating database failed.";
  
  errors[ERR_DELETE_DATABASE]    
   = "Deleting database failed.";
  
  errors[ERR_UNDEF_OBJ_VALUE]  
   = "Undefined object value in (query) expression";
                           
  errors[ERR_SYNTAX_ERROR]     
   = "Syntax error in command/expression";
                           
  errors[ERR_IDENT_RESERVED]   
   = "Identifier reserved for system use. ";
  
  errors[ERR_UPDATE_FOR_DERIVED_OBJ_UNSUPPORTED] 
   = "Update of derived objects not supported. ";

  errors[ERR_IDENT_USED]             
   = "Identifier already used.";
                           
  errors[ERR_IDENT_UNKNOWN_TYPE]     
   = "Identifier is not a known type name.";
                           
  errors[ERR_IDENT_UNKNOWN_OBJ]      
   = "Identifier is not a known object name.";
                           
  errors[ERR_EXPR_TYPE_NEQ_OBJ_TYPE]
   = "Type of expression is different from type of object.";
                           
  errors[ERR_TYPE_NAME_USED_BY_OBJ]  
   = "Type name is used by an object. Type not deleted.";

  errors[ERR_TRANSACTION_ACTIVE]       
   = "Transaction already active.";
                           
  errors[ERR_NO_TRANSACTION_ACTIVE]    
   = "No transaction active.";
                           
  errors[ERR_BEGIN_TRANSACTION_FAILED] 
   = "Begin transaction failed.";
                           
  errors[ERR_COMMIT_OR_ABORT_FAILED]   
   = "Commit/Abort transaction failed.";

  errors[ERR_IN_DEFINITIONS_FILE]     
   = "Error in type or object definitions in file.";
                           
  errors[ERR_IDENT_UNKNOWN_DB_NAME]   
   = "Identifier is not a known database name.";
                           
  errors[ERR_PROBLEM_IN_WRITING_TO_FILE] 
   = "Problem in writing to file.";
                           
  errors[ERR_DB_NAME_NEQ_IDENT]       
   = "Database name in file different from identifier.";
                           
  errors[ERR_PROBLEM_IN_READING_FILE] 
   = "Problem in reading from file.";
                           
  errors[ERR_IN_LIST_STRUCTURE_IN_FILE] 
   = "Error in the list structure in the file.";

  errors[ERR_CMD_NOT_YET_IMPL] 
   = "Command not yet implemented.";
                           
  errors[ERR_CMD_LEVEL_NOT_YET_IMPL] 
   = "Command level not yet implemented.";
                           
  errors[ERR_CMD_NOT_IMPL_AT_THIS_LEVEL] 
   = "Command not yet implemented at this level.";

  errors[ERR_IN_TYPE_DEFINITION] 
   = "Error in type definition. ";   // (40 i)
                           
  errors[ERR_NAME_DOUBLY_DEFINED] 
   = "Type name doubly defined. ";  // (41 i n)
                           
  errors[ERR_IN_TYPE_EXPRESSION] 
   = "Error in type expression. ";   // (42 i n)

  errors[ERR_IN_OBJ_DEFINITION] 
   = "Error in object definition.";  // (50 i)
                           
  errors[ERR_OBJ_NAME_DOUBLY_DEFINED] 
   = "Object name doubly defined.";  // (51 i n)
                           
  errors[ERR_WRONG_TYPE_EXPR_FOR_OBJ] 
   = "Wrong type expression for object.";  // (52 i n)
                           
  errors[ERR_WRONG_LIST_REP_FOR_OBJ] 
   = "Wrong list representation for object."; // (53 i n)

  errors[ERR_KIND_DOES_NOT_MATCH_TYPE_EXPR] 
  = "Kind does not match type expression.";  // (60 k t)
                          
 errors[ERR_SPECIFIC_KIND_CHECKING_ERROR] 
  = "Specific kind checking error for kind."; 
 // (61 k j ...)

 errors[ERR_IN_VALUELIST_TC_V] 
  = "Value list is not a representation for type constructor."; 
 // (70 tc v)
                          
 errors[ERR_SPECIFIC_FOR_TYPE_CONSTRUCTOR] 
  = "Specific error for type constructor in value list.";  
 // (71 tc j ...)
                          
 errors[ERR_IN_VALUELIST_TC] 
  = "Value list is not a representation for type constructor.";
                          
 errors[ERR_AT_POS_IN_VALUELIST] 
  = "Error at a position within value list for type constructor.";

 errors[ERR_IN_SECONDO_PROTOCOL] 
  = "Secondo protocol error.";
                          
 errors[ERR_CONNECTION_TO_SERVER_LOST] 
  = "Connection to Secondo server lost.";
 
 errors[ERR_IDENT_UNKNOWN_DB_OBJECT] 
  = "Identifier of object is not a known database object. ";
                          
 errors[ERR_OBJ_NAME_IN_FILE_NEQ_IDENT] 
  = "Object name in file different from indentifier for object. ";
                          
 errors[ERR_IDENT_ALREADY_KNOWN_IN_DB] 
  = "Identifier of object already known in the database. ";
                          
 errors[ERR_ALGEBRA_UNKNOWN] 
  = "Algebra not known or currently not included.";
                          
 errors[ERR_UNKNOWN_RETURN_CODE]
  = "A function call returned with an unknown message. ";

  ErrorMap::const_iterator errPos = errors.find( errorCode );
  if ( errPos != errors.end() )
  {
    return (string("Secondo: ") + errPos->second);
  } 
  
  cerr  << " Unknown Error! No message for error code No. " 
        << errorCode << "found."; 
  assert( false );
  return "";
}

/*
10 WriteErrorList

This Function prints an errortext. The expected list format is 

----
((e1 ...) (e2 ...) ... (eN ...))
----

each $e_i$ is an integer and must represent a valid SECONDO error code.

*/

void
SecondoInterface::WriteErrorList ( ListExpr list, ostream& os /* = cerr */ )
{
  bool ok = true;
  int errorCode = 0;
  string errorText ="";
  const ListExpr errList = list;
  NestedList* nl = GetNestedList();
  
  if ( !nl->IsEmpty( list ) )
  {
    list = nl->Rest( list );
    while (!nl->IsEmpty( list ))
    {
      ListExpr first;
      if (!nl->IsAtom( list)) 
      {
        first=nl->First(list);
      } 
      else
      { 
        os << "Error: The list has not the expected format!" << endl;
        ok = false;
        break;
      }
      nl->WriteListExpr( first, os );

      ListExpr listErrorCode = nl->Empty();
      if (!nl->IsAtom(first))
        listErrorCode = nl->First( first );
      else
        listErrorCode = first;

      if (!(nl->AtomType(listErrorCode) == IntType))
      { 
        os << "Error: Integer atom expected" << endl;
        ok = false;
        break;
      }  
      errorCode = nl->IntValue( listErrorCode );
      errorText = GetErrorMessage( errorCode );
      os << "=> " << errorText << endl;
      list = nl->Rest( list );
    }
  }

  if (!ok) 
  {
    os << "Received error list:" << endl;
    nl->WriteListExpr(errList);
  }  
}

