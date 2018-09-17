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


#include "DebugWriter.h"
#include <iostream>
#include <fstream>
#include <sstream>

//#define TRACE_ON 1
#undef TRACE_ON
#include "LogMsg.h"
#include "SecondoInterface.h"
#include "SecondoInterfaceCS.h"
#include "SocketIO.h"
#include "Profiles.h"
#include "CSProtocol.h"
#include "StringUtils.h"
#include "satof.h"


using namespace std;


DebugWriter dwriter;

SecondoInterfaceCS::SecondoInterfaceCS(bool isServer, /*= false*/
                                       NestedList* _nl, /*=0 */
                                       bool _verbose /*=true*/ ):
 SecondoInterface(isServer,_nl),
 MessageHandler() {
    server = 0;
    csp=0;
    externalNL = _nl!=0;
    maxAttempts = DEFAULT_CONNECT_MAX_ATTEMPTS;
    timeout = DEFAULT_RECONNECT_TIMEOUT;
    server_pid = -1;
    debugSecondoMethod = false;
    verbose = _verbose;
    secHost = "";
    secPort= "";
    secConfig ="";
    user = "";
    pswd = "";
    multiUser = false;
    traceSocketIn = 0;
    traceSocketOut = 0;
    

 }


SecondoInterfaceCS::~SecondoInterfaceCS()
{
  if ( initialized )
  {
    Terminate();
  }
  messageListener.clear();
}


int SecondoInterfaceCS::initNo = 0;


bool
SecondoInterfaceCS::Initialize( const string& user, const string& pswd,
                              const string& host, const string& port,
                              const string& parmFile, 
                              const string& home,
                              string& errorMsg,
                              const bool multiUser)
{
  this->user = user;
  this->pswd = pswd;
  this->multiUser = multiUser;
  secHost = host;
  secPort = port;
  secConfig = parmFile;
  string line = "";
  server_pid = -1; 

  if ( !initialized )
  {
    if(verbose){
       cout << "Initializing the Secondo system ..." << endl;
    }

    // initialize runtime flags
    if(RTFlag::empty()){
       InitRTFlags(parmFile);
    }

    
    debugSecondoMethod = RTFlag::isActive("SI:DebugSecondoMethod");

    string traceIn = SmiProfile::GetParameter("Environment", "TraceClientIn",
                                              "",parmFile);
    string traceOut = SmiProfile::GetParameter("Environment", "TraceClientOut",
                                               "",parmFile);

    // remove old trace streams
    if(traceSocketIn){
      if(traceSocketIn == traceSocketOut){
         traceSocketOut = 0;
      }
      delete traceSocketIn;
      traceSocketIn = 0;
    }
    if(traceSocketOut){
      delete traceSocketOut;
      traceSocketOut = 0;
    }
  
    // create new trace streams if entry in SecondoConfig.ini found  
 
    if(!traceIn.empty()){
      string fname = traceIn + "_" + stringutils::int2str(WinUnix::getpid())
                   + "_" + stringutils::int2str(initNo) + ".log";
      traceSocketIn = new ofstream(fname.c_str(),ios::binary | ios::app);
      if(!traceSocketIn->good()){
        delete traceSocketIn;
        traceSocketIn = 0;
      }
    }
    if(!traceOut.empty()){
       if(traceIn==traceOut){
          traceSocketOut = traceSocketIn;
       } else {
          string fname = traceOut + "_" 
                       + stringutils::int2str(WinUnix::getpid()) + "_"
                       + stringutils::int2str(initNo)+".log";
          traceSocketOut = new ofstream(fname.c_str(),ios::binary | ios::app);
          if(!traceSocketOut->good()){
            delete traceSocketOut;
            traceSocketOut = 0;
          }
       }
    }
    initNo++;
    
    if(verbose){
       if(externalNL){
          cout << "use already existing nested list storage" << endl;
      } else {
         cout << "use newly created nested list storage" << endl;
      }
    }

    // Connect with server, needed host and port
    if ( secHost.empty() && !parmFile.empty() ) 
    {
        secHost = SmiProfile::GetParameter( "Environment",
                                            "SecondoHost",
                                            "", parmFile );
    }
    if(secPort.empty() && !parmFile.empty())
    {
        secPort = SmiProfile::GetParameter( "Environment",
                                            "SecondoPort",
                                            "", parmFile );
    }


    if ( secHost.length() > 0 && secPort.length() > 0 )
    { 
      if(verbose)
      {
        cout << "Connecting with Secondo server '" << secHost << "' on port "
             << secPort << " ..." << endl;
      }
      server = Socket::Connect( secHost, secPort, Socket::SockGlobalDomain,
                                maxAttempts, timeout, 
                                traceSocketIn, traceSocketOut, false );

      if(!server) {
         cout << "Socket::Connect failed" << endl;
      }
      if( server && !server->IsOk()){
         cout << "got a server but the server is not ok" << endl;
      }


      if ( server != 0 && server->IsOk() )
      {
        iostream& iosock = server->GetSocketStream();
        try{
           iosock.clear();
        } catch(ios_base::failure ex){
           cout << "probem during clear " << ex.what() << endl;
        }
        if(csp!=0){
          delete csp;
        }
        csp = new CSProtocol(nl, iosock);

        try{
          getline( iosock, line );
          if ( line == "<SecondoOk/>" )
          {
            iosock << "<Connect>" << endl
                   << user <<  endl
                   << pswd  << endl
                   << "</Connect>" << endl;
            getline( iosock, line );
            if ( line == "<SecondoIntro>" )
            {
              do
              {
                getline( iosock, line );
                if ( line != "</SecondoIntro>" )
                {
                  if(verbose){
                    cout << line << endl;
                  }
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
              cout << "accepted are" << "<SecondoIntro>" << endl;
              cout << "         and" << "<SecondoError>" << endl;
            }
          }
          else if ( line == "<SecondoError>" ) {
            getline( iosock, line );
            cout << "Server-Error: " << line << endl;
            getline( iosock, line );
          } else {
            cout << "Unidentifiable response from server: " << line << endl;
            cout << "accepted are" << "<SecondoOk/>" << endl;
            cout << "         and" << "<SecondoError>" << endl;
          }
        } catch(...){
           cout << "some exception occurred" << endl;
        }
      } else {
        cout << "connect failed." << endl;
      }
      if ( !initialized && server != 0) {
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
  if(initialized){
    getPid();
  }
  return (initialized);
}

void
SecondoInterfaceCS::Terminate()
{
  server_pid = -1;
  if ( server != 0 )
  {
    iostream& iosock = server->GetSocketStream();
    try{
       iosock << "<Disconnect/>" << endl;
       server->Close();
    } catch(...) {}
    try{
        delete server;
    } catch(...){
       cerr << "Exception during deleting server" << endl;
    } 
    server = 0;

  }

  if (csp != 0){
     delete csp;
     csp = 0;
  }

  if (initialized)
  {
    if(!externalNL){
      delete nl;
      nl = 0;
    }
    al = 0;
    initialized = false;
  }
  if(traceSocketIn){
    if(traceSocketIn==traceSocketOut){
        traceSocketOut = 0;
    }
    delete traceSocketIn;
    traceSocketIn = 0;
  }
  if(traceSocketOut){
    delete traceSocketOut;
    traceSocketOut = 0;
  }






}


std::string SecondoInterfaceCS::getHost() const{
  return server?server->GetSocketAddress():"unknown";
}

bool SecondoInterfaceCS::connectionOK() const{
  if(!server) return false;
  if(!server->IsOk()) return false;
  iostream& iosock = server->GetSocketStream();
  return iosock.good();
}







/**************************************************************************
3.1 The Secondo Procedure

*/

void
SecondoInterfaceCS::Secondo( const string& commandText,
                           const ListExpr commandLE,
                           const int commandType,
                           const bool commandAsText,
                           const bool resultAsText,
                           ListExpr& resultList,
                           int& errorCode,
                           int& errorPos,
                           string& errorMessage,
                           const string& resultFileName /*="SecondoResult"*/,
                           const bool isApplicationLevelCommand /* = true */ )
{
/*
~Secondo~ reads a command and executes it; it possibly returns a result.
The command is one of the set of SECONDO commands.

For an explanation of the error codes refer to SecondoInterface.h

*/
 int pid = getPid();

  dwriter.write(debugSecondoMethod, cout, this, pid, 
                 "called Secondo "); 


  string cmdText="";
  ListExpr list;
  string filename="", dbName="", objName="", typeName="";
  errorMessage = "";
  errorCode    = 0;
  resultList   = nl->TheEmptyList();

  if ( server == 0 )
  {
    dwriter.write(debugSecondoMethod, cout, this, pid, "server does not exist");
    errorCode = ERR_IN_SECONDO_PROTOCOL;
    errorMessage = "no connection found";
    errorPos = 0;
    return;
  } else {
    switch (commandType)
    {
      case 0:  // list form
      {
        dwriter.write(debugSecondoMethod, cout, this, pid, "commandType = 0");
        if ( commandAsText )
        {
          dwriter.write(debugSecondoMethod, cout, this, pid, "commandAsText");
          cmdText = commandText;
        }
        else
        {
          dwriter.write(debugSecondoMethod, cout, this, pid, 
                        "write list to string");
          bool ok = nl->WriteToString( cmdText, commandLE );
          dwriter.write(debugSecondoMethod, cout, this, pid, "write list ",ok);
          if(!ok) {
            // syntax error in command/expression
            errorCode = ERR_SYNTAX_ERROR;
          }
        }
        break;
      }

      case 1:  // text form
      {
        dwriter.write(debugSecondoMethod, cout, this, pid, "commandType 1");
        cmdText = commandText;
        break;
      }
      default:
      {
        dwriter.write(debugSecondoMethod, cout, this, pid, 
                      "unknown commandType");
       // Command type not implemented
        errorCode = ERR_CMD_LEVEL_NOT_YET_IMPL;
      }
    } // switch
  }

  string line;

  iostream& iosock = server->GetSocketStream();
  ios_base::iostate s = iosock.exceptions(); //store execption mask

  try{
     iosock.exceptions(ios_base::failbit|ios_base::badbit|ios_base::eofbit);
  } catch(...){
     dwriter.write(debugSecondoMethod, cout, this, pid, 
                      "error during setting exception flags");
     iosock.exceptions(s);
     errorCode = ERR_CONNECTION_TO_SERVER_LOST;
     return;
  }

  if ( iosock.fail() )
  {
    errorCode = ERR_CONNECTION_TO_SERVER_LOST;
  }
  if ( errorCode != 0 )
  {
    iosock.exceptions(s);
    stringstream tmp;
    tmp << "there is some error " << errorCode;
    dwriter.write(debugSecondoMethod, cout, this, pid, tmp.str());
    return;
  }

  string::size_type posDatabase = cmdText.find( "database " );
  string::size_type posSave     = cmdText.find( "save " );
  string::size_type posRestore  = cmdText.find( "restore " );
  string::size_type posTo       = cmdText.find( "to " );
  string::size_type posFrom     = cmdText.find( "from " );
  dwriter.write(debugSecondoMethod, cout, this, pid, 
                "try to find out kind of cmd");

  if ( posDatabase != string::npos &&
       posSave     != string::npos &&
       posTo       != string::npos &&
       posSave < posDatabase && posDatabase < posTo )
  {
    if ( commandType == 1 )
    {
      cmdText = string( "(" ) + commandText + ")";
    }
   
    dwriter.write(debugSecondoMethod, cout, this, pid, "read list from string");
    bool ok = nl->ReadFromString( cmdText, list );
    dwriter.write(debugSecondoMethod, cout, this, pid, "read list",ok);

    if(ok) {
      dwriter.write(debugSecondoMethod, cout, this, pid, 
                     "check for save database");
      if ( nl->ListLength( list ) == 4 &&
           nl->IsEqual( nl->First( list ), "save" ) &&
           nl->IsEqual( nl->Second( list ), "database" ) &&
           nl->IsEqual( nl->Third( list ), "to" ) &&
           nl->IsAtom( nl->Fourth( list )) &&
           ((nl->AtomType( nl->Fourth( list )) == SymbolType) ||
            (nl->AtomType( nl->Fourth( list )) == SymbolType) ||
            (nl->AtomType( nl->Fourth( list )) == SymbolType) ))
      {

        dwriter.write(debugSecondoMethod, cout, this, pid, 
                      "save database recognized");
        
        switch(nl->AtomType(nl->Fourth(list))){
          case SymbolType : filename = nl->SymbolValue(nl->Fourth(list));
                             break;
          case StringType : filename = nl->StringValue(nl->Fourth(list));
                             break;
          case TextType : nl->Text2String(nl->Fourth(list),filename);
                             break;
        }
        
        dwriter.write(debugSecondoMethod, cout, this, pid, 
                      "start communication");
        // request for save database
        iosock << "<DbSave/>" << endl;

        errorCode = csp->ReadResponse( resultList,
                                       errorCode, errorPos,
                                       errorMessage , this, id,
                                       debugSecondoMethod, this, pid );

        dwriter.write(debugSecondoMethod, cout, this, pid, 
                       "communication finished");

        if (errorCode == ERR_NO_ERROR) {
          dwriter.write(debugSecondoMethod, cout, this, pid, 
                         "dbfile received, save to disk");
          nl->WriteToFile( filename.c_str(), resultList );
          resultList=nl->TheEmptyList();
          dwriter.write(debugSecondoMethod, cout, this, pid, 
                        "dbfile written to disk");
        } else if(errorCode == ERR_IN_SECONDO_PROTOCOL ){
           dwriter.write(true, cout, this, pid,
               "Remote server possible crashed " + getConnectionInfo());
        }
      }
      else
      {
        dwriter.write(debugSecondoMethod, cout, this, pid,
                       "not a avlid save db");
        // Not a valid 'save database' command
        errorCode = 1;
      }
    }
    else
    {
      dwriter.write(debugSecondoMethod, cout, this, pid, "could not read list");
      // Syntax error in list
      errorCode = ERR_SYNTAX_ERROR;
    }
  }
  else if ( posSave != string::npos && // save object to filename
            posTo   != string::npos &&
            posDatabase == string::npos &&
            posSave < posTo )
  {
   dwriter.write(debugSecondoMethod, cout, this, pid, "check for save obj");


    if ( commandType == 1 )
    {
      cmdText = string( "(" ) + commandText + ")";
    }
    dwriter.write(debugSecondoMethod, cout, this, pid, "parse list");

    bool ok =  nl->ReadFromString( cmdText, list );
    dwriter.write(debugSecondoMethod, cout, this, pid, "parsing list",ok);
    
    if(ok)
    {
      if ( nl->ListLength( list ) == 4 &&
           nl->IsEqual( nl->First( list ), "save" ) &&
           nl->IsAtom( nl->Second( list )) &&
          (nl->AtomType( nl->Fourth( list )) == SymbolType) &&
           nl->IsEqual( nl->Third( list ), "to" ) &&
           nl->IsAtom( nl->Fourth( list )) &&
          ((nl->AtomType( nl->Fourth( list )) == SymbolType) ||
           (nl->AtomType( nl->Fourth( list )) == StringType) ||
           (nl->AtomType( nl->Fourth( list )) == TextType)))
      {
        dwriter.write(debugSecondoMethod, cout, this, pid, "valid save obj");
        switch(nl->AtomType(nl->Fourth(list))){
         case SymbolType : filename = nl->SymbolValue(nl->Fourth(list));
                           break;
         case StringType : filename = nl->StringValue(nl->Fourth(list));
                           break;
         case TextType : nl->Text2String(nl->Fourth(list),filename);
                           break;
        }
        dwriter.write(debugSecondoMethod, cout, this, pid, 
                      "file name extracted");
        objName = nl->SymbolValue( nl->Second( list ) );

        dwriter.write(debugSecondoMethod, cout, this, pid, 
                       "start communication" );

        // request list representation for object
        iosock << "<ObjectSave>" << endl
               << objName << endl
               << "</ObjectSave>" << endl;

        errorCode = csp->ReadResponse( resultList,
                                       errorCode, errorPos,
                                       errorMessage , this, id, 
                                       debugSecondoMethod, this, pid  );
        dwriter.write(debugSecondoMethod, cout, this, pid, 
                      "communication finished" );

        if (errorCode == ERR_NO_ERROR) {
          dwriter.write(debugSecondoMethod, cout, this, pid, 
                        "write file "+ filename );
          
          nl->WriteToFile( filename.c_str(), resultList );
          resultList=nl->TheEmptyList();
          dwriter.write(debugSecondoMethod, cout, this, pid, "file written");
        } else if(errorCode == ERR_IN_SECONDO_PROTOCOL ){
           dwriter.write(true, cout, this, pid,
               "Remote server possible crashed " + getConnectionInfo());
        }

      }
      else
      {
        dwriter.write(debugSecondoMethod, cout, this, pid, 
                      "invalid save obj cmd");
        // Not a valid 'save database' command
        errorCode = 1;
      }
    }
    else
    {
      dwriter.write(debugSecondoMethod, cout, this, pid, 
                     "error in parsing list");
      // Syntax error in list
      errorCode = ERR_SYNTAX_ERROR;
    }
  }

  else if ( posRestore  != string::npos &&
            posDatabase == string::npos &&
            posFrom     != string::npos &&
            posRestore < posFrom )
  {
    dwriter.write(debugSecondoMethod, cout, this, pid, "check for restore db");
    if ( commandType == 1 )
    {
      cmdText = string( "(" ) + commandText + ")";
    }
    dwriter.write(debugSecondoMethod, cout, this, pid, "parse list");
    bool ok = nl->ReadFromString( cmdText, list ) ;
    dwriter.write(debugSecondoMethod, cout, this, pid, "parsing list", ok);
    if ( ok )
    {
      if ( nl->ListLength( list ) == 4 &&
           nl->IsEqual( nl->First( list ), "restore" ) &&
           nl->IsAtom( nl->Second( list )) &&
          (nl->AtomType( nl->Second( list )) == SymbolType) &&
           nl->IsEqual( nl->Third( list ), "from" ) &&
           nl->IsAtom( nl->Fourth( list )) &&
          ((nl->AtomType( nl->Fourth( list )) == SymbolType) ||
           (nl->AtomType( nl->Fourth( list )) == StringType) ||
           (nl->AtomType( nl->Fourth( list )) == TextType))   )
      {
        dwriter.write(debugSecondoMethod, cout, this, pid, "valid restore obj");
        // send object symbol
        string filename ="";
        switch(nl->AtomType(nl->Fourth(list))){
           case SymbolType : filename = nl->SymbolValue(nl->Fourth(list));
                             break;
           case StringType : filename = nl->StringValue(nl->Fourth(list));
                             break;
           case TextType :  nl->Text2String(nl->Fourth(list),filename);
                             break;
           default : assert(false);
        }
        dwriter.write(debugSecondoMethod, cout, this, pid, 
                      "file name extracted, start communication");
        iosock << csp->startObjectRestore << endl
               << nl->SymbolValue( nl->Second( list ) ) << endl;

        // send file data
        csp->SendFile(filename);

        // send end tag
        iosock << csp->endObjectRestore << endl;

        errorCode = csp->ReadResponse( resultList,
                                       errorCode, errorPos,
                                       errorMessage , this, id, 
                                       debugSecondoMethod, this, pid );
        dwriter.write(debugSecondoMethod, cout, this, pid, 
                       "communication finished");
        if(errorCode == ERR_IN_SECONDO_PROTOCOL ){
           dwriter.write(true, cout, this, pid,
               "Remote server possible crashed " + getConnectionInfo());
        }
      }
      else
      {
        dwriter.write(debugSecondoMethod, cout, this, pid, 
                      "invalid restore obj");
        // Not a valid 'restore object' command
        errorCode = ERR_CMD_NOT_RECOGNIZED;
      }
    }
    else
    {
      dwriter.write(debugSecondoMethod, cout, this, pid, 
                    "error in parsing list");
      // Syntax error in list
      errorCode = ERR_SYNTAX_ERROR;
    }
  }

  else if ( posDatabase != string::npos &&
            posRestore  != string::npos &&
            posFrom     != string::npos &&
            posRestore < posDatabase && posDatabase < posFrom )
  {
    dwriter.write(debugSecondoMethod, cout, this, pid, "check for restore db");

    if ( commandType == 1 )
    {
      cmdText = string( "(" ) + commandText + ")";
    }
    dwriter.write(debugSecondoMethod, cout, this, pid, "parse list");
    bool ok = nl->ReadFromString( cmdText, list );
    dwriter.write(debugSecondoMethod, cout, this, pid, "parsing  list",ok);

    if (ok) {
      if ( nl->ListLength( list ) == 5 &&
           nl->IsEqual( nl->First( list ), "restore" ) &&
           nl->IsEqual( nl->Second( list ), "database" ) &&
           nl->IsAtom( nl->Third( list )) &&
          (nl->AtomType( nl->Third( list )) == SymbolType) &&
           nl->IsEqual( nl->Fourth( list ), "from" ) &&
           nl->IsAtom( nl->Fifth( list )) &&
          ((nl->AtomType( nl->Fifth( list )) == SymbolType) ||
           (nl->AtomType( nl->Fifth( list )) == StringType) ||
           (nl->AtomType( nl->Fifth( list )) == TextType))   )
      {
         dwriter.write(debugSecondoMethod, cout, this, pid, "valid restore db");
         filename =  "";
         switch(nl->AtomType(nl->Fifth(list))){
            case SymbolType : filename = nl->SymbolValue(nl->Fifth(list));
                              break;
            case StringType : filename = nl->StringValue(nl->Fifth(list));
                              break;
            case TextType : nl->Text2String(nl->Fifth(list),filename);
                              break;
            default : assert(false);
          }
          dwriter.write(debugSecondoMethod, cout, this, pid, 
                        "filename extracted, start communicatioon");
         // send object symbol
         iosock << csp->startDbRestore << endl
                << nl->SymbolValue( nl->Third( list ) ) << endl;

        // send file data
        csp->SendFile(filename);

        // send end tag
        iosock << csp->endDbRestore << endl;

        errorCode = csp->ReadResponse( resultList,
                                       errorCode, errorPos,
                                       errorMessage , this, id, 
                                       debugSecondoMethod, this, pid );

        dwriter.write(debugSecondoMethod, cout, this, pid, 
                      "communicatioon finished");
        if(errorCode == ERR_IN_SECONDO_PROTOCOL ){
           dwriter.write(true, cout, this, pid,
               "Remote server possible crashed " + getConnectionInfo());
        }
      }
      else
      {
        dwriter.write(debugSecondoMethod, cout, this, pid, 
                      "invalid restore db");
        // Not a valid 'restore database' command
        errorCode = ERR_CMD_NOT_RECOGNIZED;
      }
    }
    else
    {
      dwriter.write(debugSecondoMethod, cout, this, pid, 
                     "error in parsing list");
      // Syntax error in list
      errorCode = ERR_SYNTAX_ERROR;
    }
  }
  else
  {
    dwriter.write(debugSecondoMethod, cout, this, pid, "usual secondo command");

    try {
       // Send Secondo command
       dwriter.write(debugSecondoMethod, cout, this, pid, 
                     " send command to server");
       iosock << "<Secondo>" << endl;
       dwriter.write(debugSecondoMethod, cout, this, pid, "<Secondo> send ");
       iosock  << commandType << endl;
       dwriter.write(debugSecondoMethod, cout, this, pid, "CommandType send ");
       iosock <<  cmdText << endl;
       dwriter.write(debugSecondoMethod, cout, this, pid, "CommandText send ");
       iosock << "</Secondo>" << endl;
       dwriter.write(debugSecondoMethod, cout, this, pid, 
                     "command transmitted completely, read response");
 
       // Receive result
       errorCode = csp->ReadResponse( resultList,
                                   errorCode, errorPos,
                                   errorMessage , this, id, 
                                   debugSecondoMethod, this, pid );

       dwriter.write(debugSecondoMethod, cout, this, pid, "response received");
       if(errorCode == ERR_IN_SECONDO_PROTOCOL ){
          dwriter.write(true, cout, this, pid,
              "Remote server possible crashed " + getConnectionInfo());
       } else {
         dwriter.write(debugSecondoMethod, cout, this, pid, 
                       "command processed");
       }

    } catch (ifstream::failure &ex) {
         errorCode = ERR_SYSTEM_DIED;
         dwriter.write(true, cout, this, pid,
          "Remote server possible crashed (exception) '" 
          + string(ex.what()) + "' " + getConnectionInfo());
    }
    
    dwriter.write(debugSecondoMethod, cout, this, pid, 
                  "usual secondo command finished");
  }

  iosock.exceptions(s);
  if ( resultAsText )
  {
    dwriter.write(debugSecondoMethod, cout, this, pid, "write result to file");
    nl->WriteToFile( resultFileName, resultList );
  }
  dwriter.write(debugSecondoMethod, cout, this, pid, "secondo method finished");
}

/*
1.3 Procedure ~NumericTypeExpr~

*/
ListExpr
SecondoInterfaceCS::NumericTypeExpr( const ListExpr type )
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
SecondoInterfaceCS::GetTypeId( const string& name,
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
SecondoInterfaceCS::LookUpTypeExpr( ListExpr type, string& name,
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
SecondoInterfaceCS::SetDebugLevel( const int level )
{
}


int SecondoInterfaceCS::sendFile( const string& localfilename,
                                  const string& serverFileName,
                                  const bool allowOverwrite ){

   if(localfilename.empty() || serverFileName.empty()){
        return ERR_INVALID_FILE_NAME;
   }

   iostream& iosock = server->GetSocketStream();

   iosock << csp->startFileTransfer << endl;
   iosock << serverFileName << endl;
   string allow = allowOverwrite?"<ALLOW_OVERWRITE>":"<DISALLOW_OVERWRITE>";
   iosock << allow << endl;
   iosock.flush();

   string line;
   getline(iosock,line);

   if(line == "<SecondoError>"){
      int errCode = ERR_FILE_EXISTS;
      iosock >> errCode;
      csp->skipRestOfLine();
      getline(iosock,line);
      if(line != "</SecondoError>"){
        return ERR_IN_SECONDO_PROTOCOL;
      }
      return errCode;
   }

   if(line != "<SecondoOK>"){
     return ERR_IN_SECONDO_PROTOCOL;
   }


   csp->SendFile(localfilename);
   iosock << csp->endFileTransfer << endl;
   ListExpr resultList;
   int errorCode; 
   int errorPos;
   string errorMessage;
   errorCode = csp->ReadResponse( resultList,
                                  errorCode, errorPos,
                                  errorMessage , this, id        );
   if(errorCode == ERR_IN_SECONDO_PROTOCOL ){
       dwriter.write(true, cout, this, -1,
          "Remote server possible crashed " + getConnectionInfo());
   }
   return errorCode;
}

int SecondoInterfaceCS::requestFile(const string& serverFilename,
                                    const string& localFilename,
                                    const bool allowOverwrite ){
  if(!allowOverwrite) {
     std::ifstream in(localFilename.c_str());
     if(in.good()){
       in.close();
       return ERR_FILE_EXISTS;
     } 
   }

   iostream& iosock = server->GetSocketStream();
   iosock << csp->startRequestFile << endl;
   iosock << serverFilename << endl;
   iosock << csp->endRequestFile << endl;
   iosock.flush();
   string line;
   getline(iosock,line);
   if(line=="<SecondoError>"){
      int code= -1;
      iosock >> code;
      csp->skipRestOfLine();
      getline(iosock,line);
      if(line!="</SecondoError>"){
         return ERR_IN_SECONDO_PROTOCOL;
      } else {
        return code;
      }
   }
   if(line!="<SecondoOK>"){
     return ERR_IN_SECONDO_PROTOCOL;
   }

   // the answer may be SecondoError or SecondoResponse
   bool ok = csp->ReceiveFile(localFilename);
   if(ok){
       return 0;
   } else {
       return ERR_IN_FILETRANSFER;
   }
}


string SecondoInterfaceCS::getRequestFileFolder(){
   iostream& iosock = server->GetSocketStream();
   iosock << "<REQUEST_FILE_FOLDER>" << endl;
   iosock.flush();
   string line;
   getline(iosock,line);
   return line; 
}

string SecondoInterfaceCS::getRequestFilePath(){
   if(!server){
     return "";
   }
   iostream& iosock = server->GetSocketStream();
   iosock << "<REQUEST_FILE_PATH>" << endl;
   iosock.flush();
   string line;
   getline(iosock,line);
   return line; 
}

string SecondoInterfaceCS::getSendFileFolder(){
   iostream& iosock = server->GetSocketStream();
   iosock << "<SEND_FILE_FOLDER>" << endl;
   iosock.flush();
   string line;
   getline(iosock,line);
   return line; 
}

string SecondoInterfaceCS::getSendFilePath(){
   iostream& iosock = server->GetSocketStream();
   iosock << "<SEND_FILE_PATH>" << endl;
   iosock.flush();
   string line;
   getline(iosock,line);
   return line; 
}

int SecondoInterfaceCS::getPid(){

   if(server_pid > 0){
     return server_pid;
   }
   if(!server){
     return 0;
   }

   iostream& iosock = server->GetSocketStream();
   iosock << "<SERVER_PID>" << endl;
   iosock.flush();
   string line;
   getline(iosock,line);

   bool correct;
   int res =  stringutils::str2int<int>(line, correct); 
   if(correct){
     server_pid = res;
   } else { // error
     server_pid = -1;
   }
   return server_pid;
}


std::string SecondoInterfaceCS::getHome(){
   iostream& iosock = server->GetSocketStream();
   iosock << "<GET_HOME>" << endl;
   iosock.flush();
   string line;
   getline(iosock,line);
   return line;
}

bool SecondoInterfaceCS::setHeartbeat(const int heart1, const int heart2){
   iostream& iosock = server->GetSocketStream();
   iosock << "<HEARTBEAT>" << endl;
   iosock << stringutils::int2str(heart1) << endl;
   iosock << stringutils::int2str(heart2) << endl;
   iosock << "</HEARTBEAT>" << endl;
   iosock.flush();
   string line;
   getline(iosock,line);
   return line=="<YES>";
}

bool SecondoInterfaceCS::cancelQuery(int pid){
   iostream& iosock = server->GetSocketStream();
   iosock << "<CANCEL_QUERY>" << endl;
   iosock << pid << endl;
   iosock << "</CANCEL_QUERY>" << endl; 
   
   string line;
   getline(iosock,line);
   
   if(line.compare(0, 5, "<OK/>") == 0) {
       return true;
   } else {
       return false;
   }
}

bool SecondoInterfaceCS::getOperatorIndexes(
            const string name,
            const ListExpr argList,
            ListExpr& resList,
            int& algId,
            int& opId,
            int& funId,
            NestedList* listStorage){




    iostream& iosock = server->GetSocketStream();
    iosock << "<REQUESTOPERATORINDEXES>" << endl;
    iosock << name << endl;
    bool ok = listStorage->WriteBinaryTo(argList, iosock);
    //iosock << endl;
    iosock << "</REQUESTOPERATORINDEXES>" << endl;
    if(!ok){
       cerr << "problem in sednig binary nested list" << endl;
       return false;
    }
    string line;
    getline( iosock, line );

    if(line=="<OPERATORINDEXESRESPONSE>"){

       getline(iosock,line);
       bool res = atoi(line.c_str())>0;
       getline(iosock,line);
       algId = atoi(line.c_str());
       getline(iosock,line);
       opId = atoi(line.c_str());
       getline(iosock, line);
       funId = atoi(line.c_str());
       ok = listStorage->ReadBinaryFrom(iosock,resList);
       getline(iosock,line); 
       while(line!="</OPERATORINDEXESRESPONSE>"){
          getline(iosock,line);
       }
       return res;
    } else {
        // Ignore error response
        do
        {
          getline( iosock, line );
          cerr << line << endl;
   
        }
        while ( ( line != "</SecondoError>" )  && !iosock.fail() );
        return false;
    }
}

/*
~getCosts~

The next functions return costs for a specified operator when number of tuples
and size of a single tuple is given. If the operator does not provide a
cost estimation function or the getCost function is not implemented,
the return value is false.

*/

bool SecondoInterfaceCS::getCosts(
              const int algId,
              const int opId,
              const int funId,
              const size_t noTuples,
              const size_t sizeOfTuple,
              const size_t noAttributes,
              const double selectivity,
              const size_t memoryMB,
              size_t& costs) {
    iostream& iosock = server->GetSocketStream();
    iosock << "<GETCOSTS>" << endl;
    iosock << "1" << endl;
    iosock << algId << endl;
    iosock << opId << endl;
    iosock << funId << endl;
    iosock << noTuples << endl;
    iosock << sizeOfTuple << endl;
    iosock << noAttributes<< endl;
    iosock << selectivity << endl;
    iosock << memoryMB << endl;
    iosock << "</GETCOSTS>" << endl;

    string line;
    getline(iosock,line);
    if(line == "<COSTRESPONSE>"){
       getline(iosock,line);
       bool ok = atoi(line.c_str()) != 0;
       getline(iosock,line);
       costs = atoi(line.c_str());
       do{
         getline(iosock,line);
       } while( line != "</COSTRESPONSE>");
       return ok;
    } else {
        do
        {
          getline( iosock, line );
          cerr << line << endl;
   
        }
        while ( ( line != "</SecondoError>" )  && !iosock.fail() );
        return false;
    }
}


bool SecondoInterfaceCS::getCosts(const int algId,
              const int opId,
              const int funId,
              const size_t noTuples1,
              const size_t sizeOfTuple1,
              const size_t noAttributes1,
              const size_t noTuples2,
              const size_t sizeOfTuple2,
              const size_t noAttributes2,
              const double selectivity,
              const size_t memoryMB,
              size_t& costs){
    iostream& iosock = server->GetSocketStream();
    iosock << "<GETCOSTS>" << endl;
    iosock << "2" << endl;
    iosock << algId << endl;
    iosock << opId << endl;
    iosock << funId << endl;
    iosock << noTuples1 << endl;
    iosock << sizeOfTuple1 << endl;
    iosock << noAttributes1 << endl;
    iosock << noTuples2 << endl;
    iosock << sizeOfTuple2 << endl;
    iosock << noAttributes2 << endl;
    iosock << selectivity << endl;
    iosock << memoryMB << endl;
    iosock << "</GETCOSTS>" << endl;
    string line;
    getline(iosock,line);
    if(line == "<COSTRESPONSE>"){
       getline(iosock,line);
       bool ok = atoi(line.c_str()) != 0;
       getline(iosock,line);
       costs = atoi(line.c_str());
       do{
         getline(iosock,line);
       } while( line != "</COSTRESPONSE>");
       return ok;
    } else {
        do
        {
          getline( iosock, line );
          cerr << line << endl;
   
        }
        while ( ( line != "</SecondoError>" )  && !iosock.fail() );
        return false;
    }
}

/*
~getLinearParams~

Retrieves the parameters for estimating the cost function of an operator
in a linear way.

*/
bool SecondoInterfaceCS::getLinearParams( const int algId,
                      const int opId,
                      const int funId,
                      const size_t noTuples1,
                      const size_t sizeOfTuple1,
                      const size_t noAttributes1,
                      const double selectivity,
                      double& sufficientMemory,
                      double& timeAtSuffMemory,
                      double& timeAt16MB) {

    iostream& iosock = server->GetSocketStream();
    iosock << "<GETLINEARCOSTFUN>" << endl;
    iosock << "1" << endl;
    iosock << algId << endl;
    iosock << opId << endl;
    iosock << funId << endl;
    iosock << noTuples1 << endl;
    iosock << sizeOfTuple1 << endl;
    iosock << noAttributes1 << endl;
    iosock << selectivity << endl;
    iosock << "</GETLINEARCOSTFUN>" << endl;
    string line;
    getline(iosock,line);
    if(line == "<LINEARCOSTFUNRESPONSE>"){
       getline(iosock,line);
       bool ok = atoi(line.c_str()) != 0;
       getline(iosock,line);
       sufficientMemory = satof( line.c_str());
       getline(iosock,line);
       timeAtSuffMemory = satof( line.c_str());
       getline(iosock,line);
       timeAt16MB = satof(line.c_str()); 
       do{
         getline(iosock,line);
       } while( line != "</LINEARCOSTFUNRESPONSE>");
       return ok;
    } else {
        do
        {
          getline( iosock, line );
          cerr << line << endl;
   
        }
        while ( ( line != "</SecondoError>" )  && !iosock.fail() );
        return false;
    }
}


bool SecondoInterfaceCS::getLinearParams( const int algId,
                      const int opId,
                      const int funId,
                      const size_t noTuples1,
                      const size_t sizeOfTuple1,
                      const size_t noAttributes1,
                      const size_t noTuples2,
                      const size_t sizeOfTuple2,
                      const size_t noAttributes2,
                      const double selectivity,
                      double& sufficientMemory,
                      double& timeAtSuffMemory,
                      double& timeAt16MB) {
    iostream& iosock = server->GetSocketStream();
    iosock << "<GETLINEARCOSTFUN>" << endl;
    iosock << "2" << endl;
    iosock << algId << endl;
    iosock << opId << endl;
    iosock << funId << endl;
    iosock << noTuples1 << endl;
    iosock << sizeOfTuple1 << endl;
    iosock << noAttributes1 << endl;
    iosock << noTuples2 << endl;
    iosock << sizeOfTuple2 << endl;
    iosock << noAttributes2 << endl;
    iosock << selectivity << endl;
    iosock << "</GETLINEARCOSTFUN>" << endl;
    string line;
    getline(iosock,line);
    if(line == "<LINEARCOSTFUNRESPONSE>"){
       getline(iosock,line);
       bool ok = atoi(line.c_str()) != 0;
       getline(iosock,line);
       sufficientMemory = satof( line.c_str());
       getline(iosock,line);
       timeAtSuffMemory = satof( line.c_str());
       getline(iosock,line);
       timeAt16MB = satof(line.c_str()); 
       do{
         getline(iosock,line);
       } while( line != "</LINEARCOSTFUNRESPONSE>");
       return ok;
    } else {
        do
        {
          getline( iosock, line );
          cerr << line << endl;
   
        }
        while ( ( line != "</SecondoError>" )  && !iosock.fail() );
        return false;
    }
}

/*
~getFunction~

Returns an approximation of the cost function of a specified value mapping as
a parametrized function.

*/
bool SecondoInterfaceCS::getFunction(const int algId,
                 const int opId,
                 const int funId,
                 const size_t noTuples1,
                 const size_t sizeOfTuple1,
                 const size_t noAttributes1,
                 const double selectivity,
                 int& funType,
                 double& sufficientMemory,
                 double& timeAtSuffMemory,
                 double& timeAt16MB,
                 double& a, double& b, double&c, double& d) {
    iostream& iosock = server->GetSocketStream();
    iosock << "<GETCOSTFUN>" << endl;
    iosock << "1" << endl;
    iosock << algId << endl;
    iosock << opId << endl;
    iosock << funId << endl;
    iosock << noTuples1 << endl;
    iosock << sizeOfTuple1 << endl;
    iosock << noAttributes1 << endl;
    iosock << selectivity << endl;
    iosock << "</GETCOSTFUN>" << endl;
    string line;
    getline(iosock,line);
    if(line == "<COSTFUNRESPONSE>"){
       getline(iosock,line);
       bool ok = atoi(line.c_str()) != 0;
       getline(iosock,line);
       funType = atoi(line.c_str()); 
       getline(iosock,line);
       sufficientMemory = satof( line.c_str());
       getline(iosock,line);
       timeAtSuffMemory = satof( line.c_str());
       getline(iosock,line);
       timeAt16MB = satof(line.c_str()); 
       getline(iosock,line);
       a = satof(line.c_str());
       getline(iosock,line);
       b = satof(line.c_str());
       getline(iosock,line);
       c = satof(line.c_str());
       getline(iosock,line);
       d = satof(line.c_str());
       do{
         getline(iosock,line);
       } while( line != "</COSTFUNRESPONSE>");
       return ok;
    } else {
        do
        {
          getline( iosock, line );
          cerr << line << endl;
   
        }
        while ( ( line != "</SecondoError>" )  && !iosock.fail() );
        return false;
    }
}

bool SecondoInterfaceCS::getFunction(const int algId,
                 const int opId,
                 const int funId,
                 const size_t noTuples1,
                 const size_t sizeOfTuple1,
                 const size_t noAttributes1,
                 const size_t noTuples2,
                 const size_t sizeOfTuple2,
                 const size_t noAttributes2,
                 const double selectivity,
                 int& funType,
                 double& sufficientMemory,
                 double& timeAtSuffMemory,
                 double& timeAt16MB,
                 double& a, double& b, double&c, double& d) {

    iostream& iosock = server->GetSocketStream();
    iosock << "<GETCOSTFUN>" << endl;
    iosock << "2" << endl;
    iosock << algId << endl;
    iosock << opId << endl;
    iosock << funId << endl;
    iosock << noTuples1 << endl;
    iosock << sizeOfTuple1 << endl;
    iosock << noAttributes1 << endl;
    iosock << noTuples2 << endl;
    iosock << sizeOfTuple2 << endl;
    iosock << noAttributes2 << endl;
    iosock << selectivity << endl;
    iosock << "</GETCOSTFUN>" << endl;
    string line;
    getline(iosock,line);
    if(line == "<COSTFUNRESPONSE>"){
       getline(iosock,line);
       bool ok = atoi(line.c_str()) != 0;
       getline(iosock,line);
       funType = atoi(line.c_str()); 
       getline(iosock,line);
       sufficientMemory = satof( line.c_str());
       getline(iosock,line);
       timeAtSuffMemory = satof( line.c_str());
       getline(iosock,line);
       timeAt16MB = satof(line.c_str()); 
       getline(iosock,line);
       a = satof(line.c_str());
       getline(iosock,line);
       b = satof(line.c_str());
       getline(iosock,line);
       c = satof(line.c_str());
       getline(iosock,line);
       d = satof(line.c_str());
       do{
         getline(iosock,line);
       } while( line != "</COSTFUNRESPONSE>");
       return ok;
    } else {
        do
        {
          getline( iosock, line );
          cerr << line << endl;
   
        }
        while ( ( line != "</SecondoError>" )  && !iosock.fail() );
        return false;
    }

}


void SecondoInterfaceCS::setMaxAttempts(int a){
   if(a>0 && a < 1000){
      maxAttempts = a;
   }
}
   
void SecondoInterfaceCS::setTimeout(int t){
   if(t>0 && t < 60){
      timeout = t;
   }
}

std::string SecondoInterfaceCS::getConnectionInfo() const{
  stringstream ss;
  ss << "Host: " << secHost << ", Port: " << secPort 
     << ", Servers PID:" << server_pid;
  return ss.str();
}


bool SecondoInterfaceCS::handleMsg(NestedList* nl, ListExpr msg, int source){

   bool ok = false;
   for(size_t i=0;i<messageListener.size();i++){
     ok = ok || messageListener[i]->handleMsg(nl, msg, source);
   }
   return ok;
}

void SecondoInterfaceCS::killConnection(){
    cout << "called " << __PRETTY_FUNCTION__ << endl;

    if(server){
       try{
         iostream& iosock = server->GetSocketStream();
         iosock.setstate(ios_base::failbit|ios_base::badbit|ios_base::eofbit);
       } catch(...){}
    }
}




