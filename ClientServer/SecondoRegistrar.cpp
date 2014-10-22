/*
---- 
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics & 
Computer Science, Database Systems for New Applications.

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

M. Spiekermann: Trace messages and error messages added. Some critical
C++ code re-implemented. Bug fix for command UNREGISTER.

*/

#include <string>
#include <algorithm>
#include <map>
#include <queue>

#include "Application.h"
#include "Processes.h"
#include "SocketIO.h"
#include "Profiles.h"
#include "CharTransform.h"
#include "LogMsg.h"
#include "Trace.h"

using namespace std;


class SecondoRegistrar : public Application
{
 public:
  SecondoRegistrar( const int argc, const char** argv ) : 
    Application( argc, argv ),  
    trace("REGISTRAR"), 
    EXIT_REGISTRAR_OK( 0 ),
    EXIT_REGISTRAR_NOQUEUE( 1 ),
    EXIT_REGISTRAR_ABORT( 2 )
  {};
  virtual ~SecondoRegistrar() {};
  int Execute();
  bool AbortOnSignal( int sig ) const;
 private:
  int  ProcessCommands();
  void ExecRegister();
  void ExecUnregister();
  void ExecLock();
  void ExecUnlock();
  void ExecLogin();
  void ExecLogout();
  void ExecLogMsg();
  void ExecShowMsgs();
  void ExecShowUsers();
  void ExecShowLocks();
  void ExecShowDatabases();

  Socket* msgSocket;
  Socket* request;
  multimap<string,ProcessId> dbUsers;
  multimap<string,string>    dbRegister;
  map<string,string>         dbLocks;
  std::queue<string>         logMsgs;

  Trace trace;

  const int EXIT_REGISTRAR_OK;
  const int EXIT_REGISTRAR_NOQUEUE;
  const int EXIT_REGISTRAR_ABORT;

  typedef enum {  REGISTER, UNREGISTER, 
	  LOCK, UNLOCK, 
	  LOGIN, LOGOUT, LOGMSG, 
	  SHOWMSGS, SHOWUSERS, SHOWLOCKS, SHOWDATABASES } cmdTok;	
};

bool
SecondoRegistrar::AbortOnSignal( int sig ) const
{
  if ( msgSocket != 0 )
  {
    msgSocket->CancelAccept();
  }
  return (true);
}

void
SecondoRegistrar::ExecRegister()
{
  iostream& ss = request->GetSocketStream();
  string database;
  string user;
  ss >> database >> user;
  map<string,string>::iterator posLock = dbLocks.find( database );
  if ( posLock == dbLocks.end() )
  {
    dbRegister.insert( make_pair( database, user ) );
    ss << "0 Registrar: Ok" << endl;
  }
  else
  {
    ss << "-1 Registrar: Database '" << database
       << "' is locked by " << posLock->second << endl;
  }
}

void
SecondoRegistrar::ExecUnregister()
{
  iostream& ss = request->GetSocketStream();
  string database;
  string user;
  ss >> database >> user;
  bool found = false;
  multimap<string,string>::iterator pos;
  for ( pos  = dbRegister.lower_bound( database );
        pos != dbRegister.upper_bound( database ); pos++ )
  {
    if ( pos->second == user )
    {
      dbRegister.erase( pos );
      found = true;
      break;
    }
  }
  if ( found )
  {
    ss << "0 Registrar: Ok" << endl;
  }
  else
  {
    ss << "-1 Registrar: Database '" << database
       << "' not registered by " << user << endl;
  }
}

void
SecondoRegistrar::ExecLock()
{
  iostream& ss = request->GetSocketStream();
  string database;
  string user;
  ss >> database >> user;
  map<string,string>::iterator posLock = dbLocks.find( database );
  if ( posLock == dbLocks.end() )
  {
    multimap<string,string>::iterator pos = dbRegister.find( database );
    if ( pos == dbRegister.end() )
    {
      dbLocks.insert( make_pair( database, user ) );
      ss << "0 Registrar: Ok" << endl;
    }
    else
    {
      ss << "-1 Registrar: Database '" << database << "' is in use" << endl;
    }
  }
  else
  {
    ss << "-1 Registrar: Database '" << database
       << "' is locked by " << posLock->second << endl;
  }
}

void
SecondoRegistrar::ExecUnlock()
{
  iostream& ss = request->GetSocketStream();
  string database;
  string user;
  ss >> database >> user;
  map<string,string>::iterator posLock = dbLocks.find( database );
  if ( posLock != dbLocks.end() )
  {
    dbLocks.erase( posLock );
    ss << "0 Registrar: Ok" << endl;
  }
  else
  {
    ss << "-1 Registrar: Database '" << database
       << "' was not locked by " << user << endl;
  }
}

void
SecondoRegistrar::ExecLogin()
{
  iostream& ss = request->GetSocketStream();
  string user;
  ProcessId pid;
  ss >> user >> pid;
  dbUsers.insert( make_pair( user, pid ) );
  ss << "0 Registrar: Login ok" << endl;
}

void
SecondoRegistrar::ExecLogout()
{
  iostream& ss = request->GetSocketStream();
  string user;
  ProcessId pid;
  ss >> user >> pid;
  bool found = false;
  multimap<string,ProcessId>::iterator pos;
  for ( pos  = dbUsers.lower_bound( user );
        pos != dbUsers.upper_bound( user ); pos++ )
  {
    if ( pos->second == pid )
    {
      dbUsers.erase( pos );
      found = true;
      break;
    }
  }
  if ( found )
  {
    ss << "0 Registrar: Ok" << endl;
  }
  else
  {
    ss << "-1 Registrar: User '" << user << "' not logged in" << endl;
  }
}

void
SecondoRegistrar::ExecLogMsg()
{
  iostream& ss = request->GetSocketStream();
  string msg;
  getline( ss, msg );
  logMsgs.push( msg );
  ss << "0 Registrar: Ok" << endl;
}

void
SecondoRegistrar::ExecShowMsgs()
{
  iostream& ss = request->GetSocketStream();
  ss << "1 Registrar: " << logMsgs.size() << " messages" << endl;
  while (!logMsgs.empty())
  {
    ss << "* " << logMsgs.front() << endl;
    logMsgs.pop();
  }
  ss << "0 Registrar: Ok" << endl;
}

void
SecondoRegistrar::ExecShowUsers()
{
  iostream& ss = request->GetSocketStream();
  ss << "1 Registrar: " << dbUsers.size() << " users" << endl;
  multimap<string,ProcessId>::iterator posUser;
  for ( posUser  = dbUsers.begin();
        posUser != dbUsers.end(); posUser++ )
  {
    ss << "* " << posUser->first << " " << posUser->second << endl;
  }
  ss << "0 Registrar: Ok" << endl;
}

void
SecondoRegistrar::ExecShowLocks()
{
  iostream& ss = request->GetSocketStream();
  ss << "1 Registrar: " << dbLocks.size() << " locks" << endl;
  map<string,string>::iterator pos;
  for ( pos  = dbLocks.begin();
        pos != dbLocks.end(); pos++ )
  {
    ss << "* " << pos->first << " " << pos->second << endl;
  }
  ss << "0 Registrar: Ok" << endl;
}

void
SecondoRegistrar::ExecShowDatabases()
{
  iostream& ss = request->GetSocketStream();
  ss << "1 Registrar: " << dbRegister.size() << " databases" << endl;
  multimap<string,string>::iterator pos;
  for ( pos  = dbRegister.begin();
        pos != dbRegister.end(); pos++ )
  {
    ss << "* " << pos->first << " " << pos->second << endl;
  }
  ss << "0 Registrar: Ok" << endl;
}

int
SecondoRegistrar::ProcessCommands()
{
  trace.enter(__FUNCTION__);

  map<string,cmdTok> commandTable;
  map<string,cmdTok>::iterator cmdPos;
  commandTable["REGISTER"]      = REGISTER; 
  commandTable["UNREGISTER"]    = UNREGISTER;
  commandTable["LOCK"]          = LOCK;
  commandTable["UNLOCK"]        = UNLOCK;
  commandTable["LOGIN"]         = LOGIN;
  commandTable["LOGOUT"]        = LOGOUT;
  commandTable["LOGMSG"]        = LOGMSG;
  commandTable["SHOWMSGS"]      = SHOWMSGS;
  commandTable["SHOWUSERS"]     = SHOWUSERS;
  commandTable["SHOWLOCKS"]     = SHOWLOCKS;
  commandTable["SHOWDATABASES"] = SHOWDATABASES;

  int rc = 0;
  while (rc == 0)
  {
    request = msgSocket->Accept();
    if ( request != 0 && request->IsOk() )
    {
      iostream& ss = request->GetSocketStream();
      string cmd;
      ss >> cmd;
      trace.out(cmd);	    
      transform( cmd.begin(), cmd.end(), cmd.begin(), ToUpperProperFunction );
      cmdPos = commandTable.find( cmd );
          
      if ( cmdPos != commandTable.end() )
      {
        switch (cmdPos->second) {

          case REGISTER:   ExecRegister(); break;

          case UNREGISTER: ExecUnregister(); break;

	  case LOCK:       ExecLock(); break;

	  case UNLOCK:     ExecUnlock(); break;

	  case LOGIN:      ExecLogin(); break;

	  case LOGOUT:     ExecLogout(); break;

	  case LOGMSG:     ExecLogMsg(); break;

	  case SHOWMSGS:   ExecShowMsgs(); break;

	  case SHOWUSERS:  ExecShowUsers(); break;

	  case SHOWLOCKS:  ExecShowLocks(); break;

	  case SHOWDATABASES: ExecShowDatabases(); break;

          default:
	    trace.out("Invalid Command");      
            ss << "-2 Registrar: Invalid Command " << cmd << endl;
	}	

      }
      else
      {
	trace.out("Invalid Command");      
        ss << "-2 Registrar: Invalid Command " << cmd << endl;
      }

    }
    delete request;
    if ( Application::Instance()->ShouldAbort() )
    {
      rc = EXIT_REGISTRAR_ABORT;
    }
  }
  if ( rc == EXIT_REGISTRAR_ABORT )
  {
    rc = EXIT_REGISTRAR_OK;
  }
  return (rc);
}

int
SecondoRegistrar::Execute()
{
  SetAbortMode( true );
  if (RTFlag::isActive("Registar:trace")) {
    trace.on();
  }  
  int rc = EXIT_REGISTRAR_NOQUEUE;
  string parmFile;
  if ( GetArgCount() > 1 )
  {
    parmFile = GetArgValues()[1];
  }
  else
  {
    parmFile = "SecondoConfig.ini";
  }
  trace.show( VAL(parmFile) );
  string msgQueue = SmiProfile::GetUniqueSocketName( parmFile );

  trace.show( VAL(msgQueue) );
  msgSocket = Socket::CreateLocal( msgQueue );
  trace.out("local socket created!");
  if ( msgSocket->IsOk() )
  {
    trace.out("local socket is ok!");
    rc = ProcessCommands();
  }
  else
  {
    cerr << "REGISTRAR: Error! local socket is *not* ok." << endl;
  }	  
  delete msgSocket;
  cerr << "REGISTRAR is going down with rc = " << rc << endl;
  return (rc);
}

int main( const int argc, const char* argv[] )
{
  SecondoRegistrar* appPointer = new SecondoRegistrar( argc, argv );
  int rc = appPointer->Execute();
  delete appPointer;
  return (rc);
}

