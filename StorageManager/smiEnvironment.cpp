/*

1 Implementation of SmiEnvironment (implementation independent part)

January 2002 Ulrich Telle

*/

using namespace std;

#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>

#include "SecondoSMI.h"
#include "SmiCodes.h"
#include "Profiles.h"
#include "FileSystem.h"
#include "Messenger.h"
#include "CharTransform.h"

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
SmiEnvironment::SetError( const SmiError smiErr, const string& errMsg )
{
  lastError   = smiErr;
  lastMessage = "SecondoSMI: " + errMsg;
}

void
SmiEnvironment::SetError( const SmiError smiErr, const char* errMsg )
{
  lastError   = smiErr;
  lastMessage = string("SecondoSMI: ") + errMsg;
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
    transform( database.begin(), database.end(), database.begin(), ToUpperProperFunction );
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

