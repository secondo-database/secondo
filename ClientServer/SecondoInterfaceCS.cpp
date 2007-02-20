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

1 The Implementation-Module SecondoInterface

April 2002 Ulrich Telle Client/Server version of the ~SecondoInterface~.

April 29 2003 Hoffmann Client/Server adaption for single objects save and
restore commands.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable level remains.

February 2006, M. Spiekermann. Bug fix in the save and restore commands.  The
protocol for these commands has been changed. The implementation of the client
server communication was partly encapsulated into class ~CSProtocol~ which
provides functions useful for the client and for the server implementation.
[TOC]

*/

using namespace std;

#include <iostream>
#include <fstream>
#include <sstream>

#include "SecondoInterface.h"
#include "SocketIO.h"
#include "Profiles.h"
#include "LogMsg.h"
#include "CSProtocol.h"


SecondoInterface::SecondoInterface() : 
  initialized( false ), 
  activeTransaction( false ),
  isCSImpl( true ), 
  server( 0 )
{
  nl = new NestedList();
  al = nl;
  csp = 0;
}

SecondoInterface::~SecondoInterface()
{
  if ( initialized )
  {
    Terminate();
  }
  delete nl;
  al = 0;
}


bool
SecondoInterface::Initialize( const string& user, const string& pswd,
                              const string& host, const string& port,
                              string& parmFile, const bool multiUser )
{
  string secHost = host;
  string secPort = port;
  string line = "";
  if ( !initialized )
  {
    cout << "Initializing the Secondo system ..." << endl;

    // initialize runtime flags
    InitRTFlags(parmFile);

    cout << "Setting up temporary Berkeley-DB envinronment" << endl;
    bool ok=false;
    if ( SmiEnvironment::SetHomeDir(parmFile) )
    {
      SmiEnvironment::SetUser( user ); // TODO: Check for valid user/pswd
      ok = true;
    }
    else
    {
      string errMsg;
      SmiEnvironment::GetLastErrorCode( errMsg );
      cout << "Error: " << errMsg << endl;
      ok=false;
    }

    if (ok)
      ok = (SmiEnvironment::CreateTmpEnvironment( cerr ) == 0);

    if (!ok) {
      cerr << "Error: No Berkeley-DB environment! " 
           << "Persistent nested lists not available!"
           << endl;
    }      
    
    // Connect with server, needed host and port
    if ( secHost.length() == 0 || secPort.length() == 0 )
    {
      if ( parmFile.length() != 0 )
      {
        secHost = SmiProfile::GetParameter( "Environment", 
                                            "SecondoHost", 
                                            "", parmFile );
        
        secPort = SmiProfile::GetParameter( "Environment", 
                                            "SecondoPort", 
                                            "", parmFile );
      }
    }
    if ( secHost.length() > 0 && secPort.length() > 0 )
    {
      cout << "Connecting with Secondo server '" << secHost << "' on port "
           << secPort << " ..." << endl;
      server = Socket::Connect( secHost, secPort, Socket::SockGlobalDomain );
      if ( server != 0 && server->IsOk() )
      {
        iostream& iosock = server->GetSocketStream();
        csp = new CSProtocol(nl, iosock);
        getline( iosock, line );
        if ( line == "<SecondoOk/>" )
        {
          iosock << "<Connect>" << endl
                 << "((" << user << ") (" << pswd << "))" << endl
                 << "</Connect>" << endl;
          getline( iosock, line );
          if ( line == "<SecondoIntro>" )
          {
            do
            {
              getline( iosock, line );
              if ( line != "</SecondoIntro>" )
              {
                cout << line << endl;
              }
            }
            while (line != "</SecondoIntro>");
            initialized = true;
          }
          else if ( line == "<SecondoError>" )
          {
            getline( iosock, line );
            cout << "Server-Error: " << line << endl;
            getline( iosock, line );
          }
          else
          {
            cout << "Unidentifiable response from server: " << line << endl;
          }
        }
        else if ( line == "<SecondoError>" )
        {
          getline( iosock, line );
          cout << "Server-Error: " << line << endl;
          getline( iosock, line );
        }
        else
        {
          cout << "Unidentifiable response from server: " << line << endl;
        }
      }
      else
      {
        cout << "failed." << endl;
      }
      if ( !initialized && server != 0)
      {
        server->Close();
        delete server;
        server = 0;
      }
    }
    else
    {
      cout << "Invalid or missing host (" << secHost << ") and/or port ("
           << secPort << ")." << endl;
    }
  }
  return (initialized);
}

void
SecondoInterface::Terminate()
{
  if ( server != 0 )
  {
    iostream& iosock = server->GetSocketStream();
    iosock << "<Disconnect/>" << endl;
    server->Close();
    delete server;
    server = 0;
  }
}




/************************************************************************** 
3.1 The Secondo Procedure 

*/

void
SecondoInterface::Secondo( const string& commandText,
                           const ListExpr commandLE,
                           const int commandType,
                           const bool commandAsText,
                           const bool resultAsText,
                           ListExpr& resultList,
                           int& errorCode,
                           int& errorPos,
                           string& errorMessage,
                           const string& resultFileName /*="SecondoResult"*/)
{
/*
~Secondo~ reads a command and executes it; it possibly returns a result.
The command is one of a set of SECONDO commands. 

Error Codes: see definition module.

If value 0 is returned, the command was executed without error.

*/

  string cmdText="";
  ListExpr list, errorList, errorInfo;
  string filename="", dbName="", objName="", typeName="";
  errorMessage = "";
  errorCode    = 0;
  errorList    = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  errorInfo    = errorList;
  resultList   = nl->TheEmptyList();

  if ( server == 0 )
  {
    errorCode = ERR_IN_SECONDO_PROTOCOL;
  }
  else
  {
    switch (commandType)
    {
      case 0:  // list form
      {
        if ( commandAsText )
        {
          cmdText = commandText;
        }
        else
        {
          if ( !nl->WriteToString( cmdText, commandLE ) )
          {
            // syntax error in command/expression
            errorCode = ERR_SYNTAX_ERROR;  
          }
        }
        break;
      }
      case 1:  // text form
      {
        cmdText = commandText;
        break;
      }
      default:
      {
       // Command type not implemented
        errorCode = ERR_CMD_LEVEL_NOT_YET_IMPL; 
      }
    } // switch
  }

  string line;
  iostream& iosock = server->GetSocketStream();

  ios_base::iostate s = iosock.exceptions();
  iosock.exceptions(ios_base::failbit|ios_base::badbit|ios_base::eofbit);
  
  if ( iosock.fail() )
  {
    errorCode = ERR_CONNECTION_TO_SERVER_LOST;
  }
  if ( errorCode != 0 )
  {
    return;
  }

  string::size_type posDatabase = cmdText.find( "database " );
  string::size_type posSave     = cmdText.find( "save " );
  string::size_type posRestore  = cmdText.find( "restore " );
  string::size_type posTo       = cmdText.find( "to " );
  string::size_type posFrom     = cmdText.find( "from " );

  if ( posDatabase != string::npos &&
       posSave     != string::npos &&
       posTo       != string::npos &&
       posSave < posDatabase && posDatabase < posTo )
  {
    if ( commandType == 1 )
    {
      cmdText = string( "(" ) + commandText + ")";
    }
    if ( nl->ReadFromString( cmdText, list ) )
    {
      if ( nl->ListLength( list ) == 4 &&
           nl->IsEqual( nl->First( list ), "save" ) &&
           nl->IsEqual( nl->Second( list ), "database" ) &&
           nl->IsEqual( nl->Third( list ), "to" ) &&
           nl->IsAtom( nl->Fourth( list )) && 
          (nl->AtomType( nl->Fourth( list )) == SymbolType) )
      {
        filename = nl->SymbolValue( nl->Fourth( list ) );

        // request for save database
        iosock << "<DbSave/>" << endl;
        
        errorCode = csp->ReadResponse( resultList, 
                                       errorCode, errorPos, 
                                       errorMessage         );

        if (errorCode == ERR_NO_ERROR) {
          nl->WriteToFile( filename.c_str(), resultList );
          resultList=nl->TheEmptyList();
        }  
      }
      else
      {
        // Not a valid 'save database' command
        errorCode = 1;
      }
    }
    else
    {
      // Syntax error in list
      errorCode = ERR_SYNTAX_ERROR;
    }
  }
  else if ( posSave != string::npos && // save object to filename
            posTo   != string::npos &&
            posDatabase == string::npos &&                        
            posSave < posTo )
  {
    if ( commandType == 1 )
    {
      cmdText = string( "(" ) + commandText + ")";
    }
    if ( nl->ReadFromString( cmdText, list ) )
    {
      if ( nl->ListLength( list ) == 4 &&
           nl->IsEqual( nl->First( list ), "save" ) &&
           nl->IsAtom( nl->Second( list )) &&
          (nl->AtomType( nl->Fourth( list )) == SymbolType) &&
           nl->IsEqual( nl->Third( list ), "to" ) &&
           nl->IsAtom( nl->Fourth( list )) && 
          (nl->AtomType( nl->Fourth( list )) == SymbolType) )
      {
        filename = nl->SymbolValue( nl->Fourth( list ) );
        objName = nl->SymbolValue( nl->Second( list ) );
        
        // request list representation for object
        iosock << "<ObjectSave>" << endl
               << objName << endl
               << "</ObjectSave>" << endl;
        
        errorCode = csp->ReadResponse( resultList, 
                                       errorCode, errorPos, 
                                       errorMessage         );

        if (errorCode == ERR_NO_ERROR) {
          cout << "writing file " << filename << endl;
          nl->WriteToFile( filename.c_str(), resultList );
          resultList=nl->TheEmptyList();
        }  
      }
      else
      {
        // Not a valid 'save database' command
        errorCode = 1;
      }
    }
    else
    {
      // Syntax error in list
      errorCode = ERR_SYNTAX_ERROR;
    }
  }
  
  else if ( posRestore  != string::npos &&
            posDatabase == string::npos &&
            posFrom     != string::npos &&
            posRestore < posFrom )
  {
    if ( commandType == 1 )
    {
      cmdText = string( "(" ) + commandText + ")";
    }
    if ( nl->ReadFromString( cmdText, list ) )
    {
      if ( nl->ListLength( list ) == 4 &&
           nl->IsEqual( nl->First( list ), "restore" ) && 
           nl->IsAtom( nl->Second( list )) && 
          (nl->AtomType( nl->Second( list )) == SymbolType) &&
           nl->IsEqual( nl->Third( list ), "from" ) &&
           nl->IsAtom( nl->Fourth( list )) && 
          (nl->AtomType( nl->Fourth( list )) == SymbolType) )
      {
        // send object symbol 
        iosock << csp->startObjectRestore << endl
               << nl->SymbolValue( nl->Second( list ) ) << endl;

        // send file data
        filename = nl->SymbolValue( nl->Fourth( list ) ); 
        csp->SendFile(filename);
        
        // send end tag
        iosock << csp->endObjectRestore << endl;
        
        errorCode = csp->ReadResponse( resultList, 
                                       errorCode, errorPos, 
                                       errorMessage         );
      }
      else
      {
        // Not a valid 'restore object' command
        errorCode = ERR_CMD_NOT_RECOGNIZED;
      }
    }
    else
    {
      // Syntax error in list
      errorCode = ERR_SYNTAX_ERROR;
    }
  }
  
  else if ( posDatabase != string::npos &&
            posRestore  != string::npos &&
            posFrom     != string::npos &&
            posRestore < posDatabase && posDatabase < posFrom )
  {
    if ( commandType == 1 )
    {
      cmdText = string( "(" ) + commandText + ")";
    }
    if ( nl->ReadFromString( cmdText, list ) )
    {
      if ( nl->ListLength( list ) == 5 &&
           nl->IsEqual( nl->First( list ), "restore" ) && 
           nl->IsEqual( nl->Second( list ), "database" ) &&
           nl->IsAtom( nl->Third( list )) && 
          (nl->AtomType( nl->Third( list )) == SymbolType) &&
           nl->IsEqual( nl->Fourth( list ), "from" ) &&
           nl->IsAtom( nl->Fifth( list )) && 
          (nl->AtomType( nl->Fifth( list )) == SymbolType) )
      {
        // send object symbol 
        iosock << csp->startDbRestore << endl
               << nl->SymbolValue( nl->Third( list ) ) << endl;

        // send file data
        filename = nl->SymbolValue( nl->Fifth( list ) ); 
        csp->SendFile(filename);
        
        // send end tag
        iosock << csp->endDbRestore << endl;

        errorCode = csp->ReadResponse( resultList, 
                                       errorCode, errorPos, 
                                       errorMessage         );
      }
      else
      {
        // Not a valid 'restore database' command
        errorCode = ERR_CMD_NOT_RECOGNIZED;
      }
    }
    else
    {
      // Syntax error in list
      errorCode = ERR_SYNTAX_ERROR;
    }
  }
  else
  {
    // Send Secondo command
    iosock << "<Secondo>" << endl
           << commandType << endl
           << cmdText << endl
           << "</Secondo>" << endl;
    // Receive result
    errorCode = csp->ReadResponse( resultList, 
                                   errorCode, errorPos, 
                                   errorMessage         );
  }

  iosock.exceptions(s);
  if ( resultAsText )
  {
    nl->WriteToFile( resultFileName, resultList );
  }
}

/*
1.3 Procedure ~NumericTypeExpr~

*/
ListExpr
SecondoInterface::NumericTypeExpr( const ListExpr type )
{
  ListExpr list = nl->TheEmptyList();
  if ( server != 0 )
  {
    string line;
    if ( nl->WriteToString( line, type ) )
    {
      iostream& iosock = server->GetSocketStream();
      iosock << "<NumericType>" << endl
             << line << endl
             << "</NumericType>" << endl;
      getline( iosock, line );
      if ( line == "<NumericTypeResponse>" )
      {
        getline( iosock, line );
        nl->ReadFromString( line, list );
        getline( iosock, line ); // Skip end tag '</NumericTypeResponse>'
      }
      else
      {
        // Ignore error response
        do
        {
          getline( iosock, line );
        }
        while ( line != "</SecondoError>" && !iosock.fail() );
      }
    }
  }
  return (list);
}



bool
SecondoInterface::GetTypeId( const string& name,
                             int& algebraId, int& typeId )
{
  bool ok = false;
  if ( server != 0 )
  {
    string line;
    iostream& iosock = server->GetSocketStream();
    iosock << "<GetTypeId>" << endl
           << name << endl
           << "</GetTypeId>" << endl;
    getline( iosock, line );
    if ( line == "<GetTypeIdResponse>" )
    {
      iosock >> algebraId >> typeId;
      csp->skipRestOfLine();
      getline( iosock, line ); // Skip end tag '</GetTypeIdResponse>'
      ok = !(algebraId == 0 && typeId == 0);
    }
    else
    {
      // Ignore error response
      do
      {
        getline( iosock, line );
      }
      while ( line == "</SecondoError>" || iosock.fail() );
      ok = false;
    }
  }
  return (ok);
}

bool
SecondoInterface::LookUpTypeExpr( ListExpr type, string& name,
                                  int& algebraId, int& typeId )
{
  bool ok = false;
  if ( server != 0 )
  {
    string line;
    if ( nl->WriteToString( line, type ) )
    {
      iostream& iosock = server->GetSocketStream();
      iosock << "<LookUpType>" << endl
             << line << endl
             << "</LookUpType>" << endl;
      getline( iosock, line );
      if ( line == "<LookUpTypeResponse>" )
      {
        ListExpr list;
        getline( iosock, line );
        nl->ReadFromString( line, list );
        if ( !nl->IsEmpty( nl->First( list ) ) )
        {
          name = nl->SymbolValue( nl->First( nl->First( list ) ) );
        }
        else
        {
          name = "";
        }
        algebraId = nl->IntValue( nl->Second( list ) );
        typeId    = nl->IntValue( nl->Third( list ) );
        getline( iosock, line );
        nl->Destroy( list );
        ok = true;
      }
      else
      {
        // Ignore error response
        do
        {
          getline( iosock, line );
        }
        while ( line == "</SecondoError>" || iosock.fail() );
      }
    }
  }
  if ( !ok )
  {
    name      = "";
    algebraId = 0;
    typeId    = 0;
  }
  return (ok);
}


void
SecondoInterface::SetDebugLevel( const int level )
{
}
