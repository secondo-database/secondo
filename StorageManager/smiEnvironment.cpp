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

SmiError
SmiEnvironment::CheckLastErrorCode()
{
  return lastError;
}

SmiError
SmiEnvironment::GetLastErrorCode()
{
  SmiError smiErr = lastError;
  lastError = 0;
  lastMessage = "";
  return smiErr;
}

SmiError
SmiEnvironment::GetLastErrorCode( string& errorMessage )
{
  SmiError smiErr = lastError;
  errorMessage = lastMessage;
  lastError = 0;
  lastMessage = "";
  return smiErr;
}


void 
SmiEnvironment::SetError( const SmiError smiErr )
{ 
  lastError   = smiErr;
  lastMessage = "SecondoSMI: ";
//  lastMessage = "SecondoSMI: " + Err2Msg(smiErr);
}
                        
void 
SmiEnvironment::SetError( const SmiError smiErr, const string& errMsg )
{ 
  lastError   = smiErr;
  lastMessage = "SecondoSMI: " + errMsg;
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
SmiEnvironment::RegisterDatabase( const string& dbname )
{
  bool ok = true;
  if ( !singleUserMode )
  {
    Messenger messenger( registrar );
    string answer;
    string msg = string( "REGISTER " ) + dbname;
    if ( uid != "" )
    {
      msg += string( " " ) + uid;
    }
    else
    {
      msg += string( " -UNKNOWN-" );
    }
    ok = messenger.Send( msg, answer );
    if ( ok && answer.length() > 0 && answer[0] == '0' )
    {
      SetError( E_SMI_OK );
    }
    else
    {
      SetError( E_SMI_DB_REGISTER, answer );
      ok = false;
    }
  }
  return (ok);
}

bool
SmiEnvironment::UnregisterDatabase( const string& dbname )
{
  bool ok = true;
  if ( !singleUserMode )
  {
    Messenger messenger( registrar );
    string answer;
    string msg = string( "UNREGISTER " ) + dbname;
    if ( uid != "" )
    {
      msg += string( " " ) + uid;
    }
    else
    {
      msg += string( " -UNKNOWN-" );
    }
    ok = messenger.Send( msg, answer );
    if ( ok && answer.length() > 0 && answer[0] == '0' )
    {
      SetError( E_SMI_OK );
    }
    else
    {
      SetError( E_SMI_DB_UNREGISTER, answer );
      ok = false;
    }
  }
  return (ok);
}

bool
SmiEnvironment::LockDatabase( const string& dbname )
{
  bool ok = true;
  if ( !singleUserMode )
  {
    Messenger messenger( registrar );
    string answer;
    string msg = string( "LOCK " ) + dbname;
    if ( uid != "" )
    {
      msg += string( " " ) + uid;
    }
    else
    {
      msg += string( " -UNKNOWN-" );
    }
    ok = messenger.Send( msg, answer );
    if ( ok && answer.length() > 0 && answer[0] == '0' )
    {
      SetError( E_SMI_OK );
    }
    else
    {
      SetError( E_SMI_DB_LOCK, answer );
      ok = false;
    }
  }
  return (ok);
}

bool
SmiEnvironment::UnlockDatabase( const string& dbname )
{
  bool ok = true;
  if ( !singleUserMode )
  {
    Messenger messenger( registrar );
    string answer;
    string msg = string( "LOCK " ) + dbname;
    if ( uid != "" )
    {
      msg += string( " " ) + uid;
    }
    else
    {
      msg += string( " -UNKNOWN-" );
    }
    ok = messenger.Send( msg, answer );
    if ( ok && answer.length() > 0 && answer[0] == '0' )
    {
      SetError( E_SMI_OK );
    }
    else
    {
      SetError( E_SMI_DB_UNLOCK, answer );
      ok = false;
    }
  }
  return (ok);
}

/*
Translate SMI error codes into message strings

*/
const string 
SmiEnvironment::Err2Msg( SmiError code)
{
  if( !errorMapInitialized )
  {
    errorMap[E_SMI_OK] = "Ok!";
    errorMap[E_SMI_STARTUP] = "E_SMI_STARTUP";
    errorMap[E_SMI_SHUTDOWN] = "E_SMI_SHUTDOWN";
    errorMap[E_SMI_DB_CREATE] = "[E_SMI_DB_CREATE";
    errorMap[E_SMI_DB_OPEN] = "E_SMI_DB_OPEN";
    errorMap[E_SMI_DB_CLOSE] = "E_SMI_DB_CLOSE";
    errorMap[E_SMI_DB_ERASE] = "E_SMI_DB_ERASE";
    errorMap[E_SMI_DB_NOTOPEN] = "E_SMI_DB_NOTOPEN";
    errorMap[E_SMI_DB_NOTCLOSED] = "E_SMI_DB_NOTCLOSED";
    errorMap[E_SMI_DB_INVALIDNAME] = 
     "A database name must have less than 16 characters.";
    errorMap[E_SMI_DB_EXISTING] = "The database is already known.";
    errorMap[E_SMI_DB_NOTEXISTING] = "The database is unknown.";
    errorMap[E_SMI_DB_LOOKUP] = "E_SMI_DB_LOOKUP";
    errorMap[E_SMI_DB_INSERT_CATALOG] = "E_SMI_DB_INSERT_CATALOG";
    errorMap[E_SMI_DB_DELETE_CATALOG] = "E_SMI_DB_DELETE_CATALOG";
    errorMap[E_SMI_DB_UPDATE_CATALOG] = "E_SMI_DB_UPDATE_CATALOG";
    errorMap[E_SMI_DB_NOTFOUND] = "E_SMI_DB_NOTFOUND";
    errorMap[E_SMI_DB_NOTLOCKED] = "E_SMI_DB_NOTLOCKED";
    errorMap[E_SMI_DB_REGISTER] = "E_SMI_DB_REGISTER";
    errorMap[E_SMI_DB_UNREGISTER] = "E_SMI_DB_UNREGISTER";
    errorMap[E_SMI_DB_LOCK] = "E_SMI_DB_LOCK";
    errorMap[E_SMI_DB_UNLOCK] = "E_SMI_DB_UNLOCK";

    errorMap[E_SMI_TXN_BEGIN] = "E_SMI_TXN_BEGIN";
    errorMap[E_SMI_TXN_COMMIT] = "E_SMI_TXN_COMMIT";
    errorMap[E_SMI_TXN_ABORT] = "E_SMI_TXN_ABORT";
    errorMap[E_SMI_TXN_RUNNING] = "E_SMI_TXN_RUNNING";
    errorMap[E_SMI_TXN_NOTRUNNING] = "E_SMI_TXN_NOTRUNNING";

    errorMap[E_SMI_CATALOG_LOOKUP] = "E_SMI_CATALOG_LOOKUP";
    errorMap[E_SMI_CATALOG_INSERT] = "E_SMI_CATALOG_INSERT";
    errorMap[E_SMI_CATALOG_DELETE] = "E_SMI_CATALOG_DELETE";
    errorMap[E_SMI_CATALOG_KEYEXIST] = "E_SMI_CATALOG_KEYEXIST";
    errorMap[E_SMI_CATALOG_NOTFOUND] = "E_SMI_CATALOG_NOTFOUND";

    errorMap[E_SMI_FILE_INVALIDNAME] = "E_SMI_FILE_INVALIDNAME";
    errorMap[E_SMI_FILE_NOFILEID] = "E_SMI_FILE_NOFILEID";
    errorMap[E_SMI_FILE_BADCONTEXT] = "E_SMI_FILE_BADCONTEXT";
    errorMap[E_SMI_FILE_CREATE] = "E_SMI_FILE_CREATE";
    errorMap[E_SMI_FILE_OPEN] = "E_SMI_FILE_OPEN";
    errorMap[E_SMI_FILE_CLOSE] = "E_SMI_FILE_CLOSE";
    errorMap[E_SMI_FILE_KEYEXIST] = "E_SMI_FILE_KEYEXIST";
    errorMap[E_SMI_FILE_ISTEMP] = "E_SMI_FILE_ISTEMP";

    errorMap[E_SMI_RECORD_NOTINIT] = "E_SMI_RECORD_NOTINIT";
    errorMap[E_SMI_RECORD_READ] = "E_SMI_RECORD_READ";
    errorMap[E_SMI_RECORD_WRITE] = "E_SMI_RECORD_WRITE";
    errorMap[E_SMI_RECORD_SELECT] = "E_SMI_RECORD_SELECT";
    errorMap[E_SMI_RECORD_APPEND] = "E_SMI_RECORD_APPEND";
    errorMap[E_SMI_RECORD_DELETE] = "E_SMI_RECORD_DELETE";
    errorMap[E_SMI_RECORD_SELECTALL] = "E_SMI_RECORD_SELECTALL";
    errorMap[E_SMI_RECORD_INSERT] = "E_SMI_RECORD_INSERT";
    errorMap[E_SMI_RECORD_TRUNCATE] = "E_SMI_RECORD_TRUNCATE";
    errorMap[E_SMI_RECORD_READONLY] = "E_SMI_RECORD_READONLY";
    errorMap[E_SMI_RECORD_FINISH] = "E_SMI_RECORD_FINISH";

    errorMap[E_SMI_CURSOR_NOTOPEN] = "E_SMI_CURSOR_NOTOPEN";
    errorMap[E_SMI_CURSOR_NEXT] = "E_SMI_CURSOR_NEXT";
    errorMap[E_SMI_CURSOR_ENDOFSCAN] = "E_SMI_CURSOR_ENDOFSCAN";
    errorMap[E_SMI_CURSOR_DELETE] = "E_SMI_CURSOR_DELETE";
    errorMap[E_SMI_CURSOR_FINISH] = "E_SMI_CURSOR_FINISH";

    errorMap[E_SMI_CURSOR_NOTOPEN] = "E_SMI_CURSOR_NOTOPEN";
    errorMap[E_SMI_CURSOR_NEXT] = "E_SMI_CURSOR_NEXT";
    errorMap[E_SMI_CURSOR_ENDOFSCAN] = "E_SMI_CURSOR_ENDOFSCAN";
    errorMap[E_SMI_CURSOR_DELETE] = "E_SMI_CURSOR_DELETE";
    errorMap[E_SMI_CURSOR_FINISH] = "E_SMI_CURSOR_FINISH";
  }

  map<SmiError, string>::const_iterator it = errorMap.find(code);
  if (it != errorMap.end() )
  {
     return it->second;
  } 
  cerr  << " Unknown Error! No message for error code No. "
        << code << "found.";
  assert(false);
  return "";
}
