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

1 Implementation of SmiEnvironment (implementation independent part)

January 2002 Ulrich Telle

Nov. 2004 M. Spiekermann. Some functions implementations moved to the
header file in order to declare them as inline functions.

August 2006 M. Spiekermann. New function ~Err2Msg~ introduced. This function
translates the SMI error codes defined in ErrorCodes.h into a message string. 
Currently only a few codes are translated.

*/

using namespace std;

#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <map>
#include <iomanip>

#include "SecondoSMI.h"
#include "SmiCodes.h"
#include "Profiles.h"
#include "FileSystem.h"
#include "Messenger.h"
#include "CharTransform.h"
#include "LogMsg.h"

map<SmiError, string> SmiEnvironment::errorMap;
bool SmiEnvironment::errorMapInitialized = false;
/*
Initialization of the ~erroMap~ table

*/

SmiEnvironment::SmiType
SmiEnvironment::GetImplementationType()
{
  return (smiType);
}

SmiEnvironment*
SmiEnvironment::GetInstance()
{
  return (&instance);
}

bool
SmiEnvironment::IsDatabaseOpen()
{
  return (dbOpened);
}

string
SmiEnvironment::CurrentDatabase()
{
  if ( dbOpened )
  {
    return (database);
  }
  else
  {
    return ("");
  }
}


void
SmiEnvironment::ResetSmiErrors()
{
  numOfErrors=0;
  lastError = E_SMI_OK;
  lastMessage = "";
}

int
SmiEnvironment::GetNumOfErrors()
{
  return numOfErrors;
}

SmiError
SmiEnvironment::GetLastErrorCode()
{
  return lastError;
}

SmiError
SmiEnvironment::GetLastErrorCode( string& errorMessage )
{
  stringstream msg;
  msg << "\n" 
      << "--------------------------- \n"
      << "  Secondo-SMI Error Stack   \n"
      << "--------------------------- \n" << lastMessage << "\n\n";
  errorMessage = msg.str();     
      		 
  lastMessage = "";
  return lastError;
}


void 
SmiEnvironment::SetSmiError( const SmiError smiErr, 
		             const string& file, int pos )
{ 
  if (smiErr != E_SMI_OK)
    SetSmiError(smiErr, Err2Msg(smiErr), file, pos);
}
                        
void 
SmiEnvironment::SetSmiError( const SmiError smiErr, 
		             const string& errMsg, const string& file, int pos )
{ 
  static bool abortOnError = RTFlag::isActive("SMI:abortOnError");
  lastError = smiErr;
  if (smiErr != E_SMI_OK)
  { 
    if ( numOfErrors > 0 )
      lastMessage += "\n";
    stringstream msg;
    msg << errMsg << " -> [" << file << ":" << pos << "]";   
    lastMessage += msg.str();
    numOfErrors++;	

    if (abortOnError) {
      cerr << msg.str() << endl; 	    
      abort();	    
    }	    
  }  
}                      


bool
SmiEnvironment::SetDatabaseName( const string& dbname )
{
  static string alpha( "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
  static string alnum( alpha + "0123456789_" );
  bool ok = false;
  database = dbname;
  if ( database.length() > 0 )
  {
    transform( database.begin(), database.end(), 
               database.begin(), ToUpperProperFunction );
    string::size_type pos = database.find_first_not_of( alnum );
    ok = (pos == string::npos) &&
         (dbname[0] != '_') &&
         (dbname.length() <= SMI_MAX_DBNAMELEN);
  }
  return (ok);
}

bool
SmiEnvironment::SetUser( const string& userId )
{
  static string alpha( "abcdefghijklmnopqrstuvwxyz" );
  static string alnum( alpha + "0123456789_" );

  uid = userId;
  transform( uid.begin(), uid.end(), uid.begin(), ToLowerProperFunction );
  string::size_type pos = uid.find_first_not_of( alnum );

  return (pos == string::npos);
}

bool
SmiEnvironment::CallRegistrar( const string& dbname, 
		               const string& cmd,
	                       string& answer	       )
{
  bool ok = true;
  if ( !singleUserMode )
  {
    
    string blank(" ");
    string user("-UNKNOWN-");
    if ( uid != "" ) { 
      user = uid;	    
    }

    Messenger messenger( registrar );
    string answer("");
    string msg = cmd + blank + dbname + blank + user;
    ok = messenger.Send( msg, answer );

    if ( ok && answer.length() > 0 && answer[0] == '0' )
    {
      SetError( E_SMI_OK );
    }
    else
    {
      ok = false;
    }
  }
  return (ok);
}

bool
SmiEnvironment::RegisterDatabase( const string& dbname )
{
  string answer("");	
  bool ok = CallRegistrar(dbname, "REGISTER", answer);
  if (!ok)
    SetError2( E_SMI_DB_REGISTER, answer );
  return ok;
}  


bool
SmiEnvironment::UnregisterDatabase( const string& dbname )
{
  string answer("");	
  bool ok = CallRegistrar(dbname, "UNREGISTER", answer);
  if (!ok)
    SetError2( E_SMI_DB_UNREGISTER, answer );
  return ok;
}  

bool
SmiEnvironment::LockDatabase( const string& dbname )
{
  string answer("");	
  bool ok = CallRegistrar(dbname, "LOCK", answer);
  if (!ok)
    SetError2( E_SMI_DB_LOCK, answer );
  return ok;
}  

bool
SmiEnvironment::UnlockDatabase( const string& dbname )
{
  string answer("");	
  bool ok = CallRegistrar(dbname, "UNLOCK", answer);
  if (!ok)
    SetError2( E_SMI_DB_UNLOCK, answer );
  return ok;
}

/*
Translate SMI error codes into message strings

*/
const string 
SmiEnvironment::Err2Msg( SmiError code)
{
  if( !errorMapInitialized )
  {

    errorMap[ ERR_NO_ERROR ] = "ERR_NO_ERROR";
    errorMap[ ERR_CMD_NOT_RECOGNIZED ] = "ERR_CMD_NOT_RECOGNIZED";
    errorMap[ ERR_IN_QUERY_EXPR ] = "ERR_IN_QUERY_EXPR";
    errorMap[ ERR_EXPR_NOT_EVALUABLE ] = "ERR_EXPR_NOT_EVALUABLE";
    errorMap[ ERR_NO_OBJ_CREATED ] = "ERR_NO_OBJ_CREATED";
    errorMap[ ERR_NO_TYPE_DEFINED ] = "ERR_NO_TYPE_DEFINED";
    errorMap[ ERR_NO_DATABASE_OPEN ] = "ERR_NO_DATABASE_OPEN";
    errorMap[ ERR_DATABASE_OPEN ] = "ERR_DATABASE_OPEN";
    errorMap[ ERR_CREATE_DATABASE ] = "ERR_CREATE_DATABASE";
    errorMap[ ERR_DELETE_DATABASE ] = "ERR_DELETE_DATABASE";
    errorMap[ ERR_UNDEF_OBJ_VALUE ] = "ERR_UNDEF_OBJ_VALUE";
    errorMap[ ERR_SYNTAX_ERROR ] = "ERR_SYNTAX_ERROR";
 
    errorMap[ ERR_IDENT_USED ] = "ERR_IDENT_USED";
    errorMap[ ERR_IDENT_UNKNOWN_TYPE ] = "ERR_IDENT_UNKNOWN_TYPE";
    errorMap[ ERR_IDENT_UNKNOWN_OBJ ] = "ERR_IDENT_UNKNOWN_OBJ";
    errorMap[ ERR_EXPR_TYPE_NEQ_OBJ_TYPE ] = "ERR_EXPR_TYPE_NEQ_OBJ_TYPE";
    errorMap[ ERR_TYPE_NAME_USED_BY_OBJ ] = "ERR_TYPE_NAME_USED_BY_OBJ";
    errorMap[ ERR_IDENT_RESERVED ] = "ERR_IDENT_RESERVED";
    errorMap[ ERR_UPDATE_FOR_DERIVED_OBJ_UNSUPPORTED ] = "ERR_UPDATE_FOR_DERIVED_OBJ_UNSUPPORTED";
 
    errorMap[ ERR_TRANSACTION_ACTIVE ] = "ERR_TRANSACTION_ACTIVE"; 
    errorMap[ ERR_NO_TRANSACTION_ACTIVE ] = "ERR_NO_TRANSACTION_ACTIVE"; 
    errorMap[ ERR_BEGIN_TRANSACTION_FAILED ] = "ERR_BEGIN_TRANSACTION_FAILED"; 
    errorMap[ ERR_COMMIT_OR_ABORT_FAILED ] = "ERR_COMMIT_OR_ABORT_FAILED";
    errorMap[ ERR_IN_DEFINITIONS_FILE ] = "ERR_IN_DEFINITIONS_FILE";
    errorMap[ ERR_IDENT_UNKNOWN_DB_NAME ] = "ERR_IDENT_UNKNOWN_DB_NAME";
    errorMap[ ERR_PROBLEM_IN_WRITING_TO_FILE ] = "ERR_PROBLEM_IN_WRITING_TO_FILE";
    errorMap[ ERR_DB_NAME_NEQ_IDENT ] = "ERR_DB_NAME_NEQ_IDENT";
    errorMap[ ERR_PROBLEM_IN_READING_FILE ] = "ERR_PROBLEM_IN_READING_FILE";
    errorMap[ ERR_IN_LIST_STRUCTURE_IN_FILE ] = "ERR_IN_LIST_STRUCTURE_IN_FILE";
 
    errorMap[ ERR_CMD_NOT_YET_IMPL ] = "ERR_CMD_NOT_YET_IMPL";
    errorMap[ ERR_CMD_LEVEL_NOT_YET_IMPL ] = "ERR_CMD_LEVEL_NOT_YET_IMPL";
    errorMap[ ERR_CMD_NOT_IMPL_AT_THIS_LEVEL ] = "ERR_CMD_NOT_IMPL_AT_THIS_LEVEL";
 
    errorMap[ ERR_IN_TYPE_DEFINITION ] = "ERR_IN_TYPE_DEFINITION";  
    errorMap[ ERR_NAME_DOUBLY_DEFINED ] = "ERR_NAME_DOUBLY_DEFINED";
    errorMap[ ERR_IN_TYPE_EXPRESSION ] = "ERR_IN_TYPE_EXPRESSION";
   
    errorMap[ ERR_IN_OBJ_DEFINITION ] = "ERR_IN_OBJ_DEFINITION";
    errorMap[ ERR_OBJ_NAME_DOUBLY_DEFINED ] = "ERR_OBJ_NAME_DOUBLY_DEFINED";
    errorMap[ ERR_WRONG_TYPE_EXPR_FOR_OBJ ] = "ERR_WRONG_TYPE_EXPR_FOR_OBJ";
    errorMap[ ERR_WRONG_LIST_REP_FOR_OBJ ] = "ERR_WRONG_LIST_REP_FOR_OBJ";
 
    errorMap[ ERR_KIND_DOES_NOT_MATCH_TYPE_EXPR ] = "ERR_KIND_DOES_NOT_MATCH_TYPE_EXPR";
    errorMap[ ERR_SPECIFIC_KIND_CHECKING_ERROR ] = "ERR_SPECIFIC_KIND_CHECKING_ERROR";
 
    errorMap[ ERR_IN_VALUELIST_TC_V ] = "ERR_IN_VALUELIST_TC_V";
    errorMap[ ERR_SPECIFIC_FOR_TYPE_CONSTRUCTOR ] = "ERR_SPECIFIC_FOR_TYPE_CONSTRUCTOR";
    errorMap[ ERR_IN_VALUELIST_TC ] = "ERR_IN_VALUELIST_TC";
    errorMap[ ERR_AT_POS_IN_VALUELIST ] = "ERR_AT_POS_IN_VALUELIST";
 
    errorMap[ ERR_IN_SECONDO_PROTOCOL ] = "ERR_IN_SECONDO_PROTOCOL";
    errorMap[ ERR_CONNECTION_TO_SERVER_LOST ] = "ERR_CONNECTION_TO_SERVER_LOST";
    errorMap[ ERR_IDENT_UNKNOWN_DB_OBJECT ] = "ERR_IDENT_UNKNOWN_DB_OBJECT";
    errorMap[ ERR_OBJ_NAME_IN_FILE_NEQ_IDENT ] = "ERR_OBJ_NAME_IN_FILE_NEQ_IDENT";
    errorMap[ ERR_IDENT_ALREADY_KNOWN_IN_DB ] = "ERR_IDENT_ALREADY_KNOWN_IN_DB";
    errorMap[ ERR_ALGEBRA_UNKNOWN ] = "ERR_ALGEBRA_UNKNOWN";
 
    errorMap[ ERR_SYSTEM_ERROR ] = "ERR_SYSTEM_ERROR";
 
    errorMap[ ERR_UNKNOWN_RETURN_CODE ] = "ERR_UNKNOWN_RETURN_CODE";
 
    errorMap[ E_SMI_BDB               ] = "E_SMI_BDB";
 
    errorMap[ E_SMI_STARTUP           ] = "E_SMI_STARTUP";
    errorMap[ E_SMI_SHUTDOWN          ] = "E_SMI_SHUTDOWN";
    errorMap[ E_SMI_DB_CREATE         ] = "E_SMI_DB_CREATE";
    errorMap[ E_SMI_DB_OPEN           ] = "E_SMI_DB_OPEN";
    errorMap[ E_SMI_DB_CLOSE          ] = "E_SMI_DB_CLOSE";
    errorMap[ E_SMI_DB_ERASE          ] = "E_SMI_DB_ERASE";
    errorMap[ E_SMI_DB_NOTOPEN        ] = "E_SMI_DB_NOTOPEN";
    errorMap[ E_SMI_DB_NOTCLOSED      ] = "E_SMI_DB_NOTCLOSED";
    errorMap[ E_SMI_DB_INVALIDNAME    ] = "E_SMI_DB_INVALIDNAME";
    errorMap[ E_SMI_DB_EXISTING       ] = "E_SMI_DB_EXISTING";
    errorMap[ E_SMI_DB_NOTEXISTING    ] = "E_SMI_DB_NOTEXISTING";
    errorMap[ E_SMI_DB_LOOKUP         ] = "E_SMI_DB_LOOKUP";
    errorMap[ E_SMI_DB_INSERT_CATALOG ] = "E_SMI_DB_INSERT_CATALOG";
    errorMap[ E_SMI_DB_DELETE_CATALOG ] = "E_SMI_DB_DELETE_CATALOG";
    errorMap[ E_SMI_DB_UPDATE_CATALOG ] = "E_SMI_DB_UPDATE_CATALOG";
    errorMap[ E_SMI_DB_NOTFOUND       ] = "E_SMI_DB_NOTFOUND";
    errorMap[ E_SMI_DB_NOTLOCKED      ] = "E_SMI_DB_NOTLOCKED";
    errorMap[ E_SMI_DB_REGISTER       ] = "E_SMI_DB_REGISTER";
    errorMap[ E_SMI_DB_UNREGISTER     ] = "E_SMI_DB_UNREGISTER";
    errorMap[ E_SMI_DB_LOCK           ] = "E_SMI_DB_LOCK";
    errorMap[ E_SMI_DB_UNLOCK         ] = "E_SMI_DB_UNLOCK";
    errorMap[ E_SMI_DB_LOCK_DEADLOCK  ] = "E_SMI_DB_LOCK_DEADLOCK";
 
    errorMap[ E_SMI_TXN_BEGIN         ] = "E_SMI_TXN_BEGIN";
    errorMap[ E_SMI_TXN_COMMIT        ] = "E_SMI_TXN_COMMIT";
    errorMap[ E_SMI_TXN_ABORT         ] = "E_SMI_TXN_ABORT";
    errorMap[ E_SMI_TXN_RUNNING       ] = "E_SMI_TXN_RUNNING";
    errorMap[ E_SMI_TXN_NOTRUNNING    ] = "E_SMI_TXN_NOTRUNNING";
 
    errorMap[ E_SMI_CATALOG_LOOKUP    ] = "E_SMI_CATALOG_LOOKUP";
    errorMap[ E_SMI_CATALOG_INSERT    ] = "E_SMI_CATALOG_INSERT";
    errorMap[ E_SMI_CATALOG_DELETE    ] = "E_SMI_CATALOG_DELETE";
    errorMap[ E_SMI_CATALOG_KEYEXIST  ] = "E_SMI_CATALOG_KEYEXIST";
    errorMap[ E_SMI_CATALOG_NOTFOUND  ] = "E_SMI_CATALOG_NOTFOUND";
 
    errorMap[ E_SMI_FILE_INVALIDNAME  ] = "E_SMI_FILE_INVALIDNAME";
    errorMap[ E_SMI_FILE_NOFILEID     ] = "E_SMI_FILE_NOFILEID";
    errorMap[ E_SMI_FILE_BADCONTEXT   ] = "E_SMI_FILE_BADCONTEXT";
    errorMap[ E_SMI_FILE_CREATE       ] = "E_SMI_FILE_CREATE";
    errorMap[ E_SMI_FILE_OPEN         ] = "E_SMI_FILE_OPEN";
    errorMap[ E_SMI_FILE_CLOSE        ] = "E_SMI_FILE_CLOSE";
    errorMap[ E_SMI_FILE_KEYEXIST     ] = "E_SMI_FILE_KEYEXIST";
    errorMap[ E_SMI_FILE_ISTEMP       ] = "E_SMI_FILE_ISTEMP";
 
    errorMap[ E_SMI_RECORD_NOTINIT    ] = "E_SMI_RECORD_NOTINIT";
    errorMap[ E_SMI_RECORD_READ       ] = "E_SMI_RECORD_READ";
    errorMap[ E_SMI_RECORD_WRITE      ] = "E_SMI_RECORD_WRITE";
    errorMap[ E_SMI_RECORD_SELECT     ] = "E_SMI_RECORD_SELECT";
    errorMap[ E_SMI_RECORD_APPEND     ] = "E_SMI_RECORD_APPEND";
    errorMap[ E_SMI_RECORD_DELETE     ] = "E_SMI_RECORD_DELETE";
    errorMap[ E_SMI_RECORD_SELECTALL  ] = "E_SMI_RECORD_SELECTALL";
    errorMap[ E_SMI_RECORD_INSERT     ] = "E_SMI_RECORD_INSERT";
    errorMap[ E_SMI_RECORD_TRUNCATE   ] = "E_SMI_RECORD_TRUNCATE";
    errorMap[ E_SMI_RECORD_READONLY   ] = "E_SMI_RECORD_READONLY";
    errorMap[ E_SMI_RECORD_FINISH     ] = "E_SMI_RECORD_FINISH";
 
    errorMap[ E_SMI_CURSOR_NOTOPEN    ] = "E_SMI_CURSOR_NOTOPEN";
    errorMap[ E_SMI_CURSOR_NEXT       ] = "E_SMI_CURSOR_NEXT";
    errorMap[ E_SMI_CURSOR_ENDOFSCAN  ] = "E_SMI_CURSOR_ENDOFSCAN";
    errorMap[ E_SMI_CURSOR_DELETE     ] = "E_SMI_CURSOR_DELETE";
    errorMap[ E_SMI_CURSOR_FINISH     ] = "E_SMI_CURSOR_FINISH";
 
    errorMap[ E_SMI_PREFETCH_RANGE    ] = "E_SMI_PREFETCH_RANGE";
 
    errorMapInitialized = true;
  }

  map<SmiError, string>::const_iterator it = errorMap.find(code);
  if (it != errorMap.end() )
  {
     return it->second;
  } 
  stringstream ss;
  ss << "error code " << code << " not known " << endl;
  return ss.str();
}
