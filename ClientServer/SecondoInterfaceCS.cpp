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

using namespace std;

SecondoInterfaceCS::SecondoInterfaceCS(bool isServer, /*= false*/
                                       NestedList* _nl /*=0 */):
 SecondoInterface(isServer,_nl) {
    externalNL = _nl!=0;
    maxAttempts = DEFAULT_CONNECT_MAX_ATTEMPTS;
    timeout = DEFAULT_RECONNECT_TIMEOUT;
 }


SecondoInterfaceCS::~SecondoInterfaceCS()
{
  if ( initialized )
  {
    Terminate();
  }
}


bool
SecondoInterfaceCS::Initialize( const string& user, const string& pswd,
                              const string& host, const string& port,
                              string& parmFile, string& errorMsg,
                              const bool multiUser )
{
  string secHost = host;
  string secPort = port;
  string line = "";
  if ( !initialized )
  {
    cout << "Initializing the Secondo system ..." << endl;

    // initialize runtime flags
    InitRTFlags(parmFile);

    bool ok=false;
    if(externalNL){
      cout << "use already existing nested list storage" << endl;
    } else {
      cout << "Setting up temporary Berkeley-DB envinronment" << endl;
      if ( SmiEnvironment::SetHomeDir(parmFile) ) {
           SmiEnvironment::SetUser( user ); // TODO: Check for valid user/pswd
           ok = true;
      } else {
           string errMsg;
           SmiEnvironment::GetLastErrorCode( errMsg );
            cout  << "Error: " << errMsg << endl;
            errorMsg += errMsg +"\n";
            ok=false;
       }

       if (ok)
          ok = (SmiEnvironment::CreateTmpEnvironment( cerr ) == 0);

       if (!ok) {
           cerr << "Error: No Berkeley-DB environment! "
                << "Persistent nested lists not available!"
               << endl;
       }
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
      server = Socket::Connect( secHost, secPort, Socket::SockGlobalDomain,
                                maxAttempts, timeout );
      if ( server != 0 && server->IsOk() )
      {
        iostream& iosock = server->GetSocketStream();
        if(csp!=0){
          delete csp;
        }
        csp = new CSProtocol(nl, iosock);
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
SecondoInterfaceCS::Terminate()
{
  if ( server != 0 )
  {
    iostream& iosock = server->GetSocketStream();
    iosock << "<Disconnect/>" << endl;
    server->Close();
    delete server;
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
      SmiEnvironment::DeleteTmpEnvironment();
    }
    al = 0;
    initialized = false;
  }
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

  string cmdText="";
  ListExpr list;
  string filename="", dbName="", objName="", typeName="";
  errorMessage = "";
  errorCode    = 0;
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
           ((nl->AtomType( nl->Fourth( list )) == SymbolType) ||
            (nl->AtomType( nl->Fourth( list )) == SymbolType) ||
            (nl->AtomType( nl->Fourth( list )) == SymbolType) ))
      {
        switch(nl->AtomType(nl->Fourth(list))){
          case SymbolType : filename = nl->SymbolValue(nl->Fourth(list));
                             break;
          case StringType : filename = nl->StringValue(nl->Fourth(list));
                             break;
          case TextType : nl->Text2String(nl->Fourth(list),filename);
                             break;
        }
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
          ((nl->AtomType( nl->Fourth( list )) == SymbolType) ||
           (nl->AtomType( nl->Fourth( list )) == StringType) ||
           (nl->AtomType( nl->Fourth( list )) == TextType)))
      {
        switch(nl->AtomType(nl->Fourth(list))){
         case SymbolType : filename = nl->SymbolValue(nl->Fourth(list));
                           break;
         case StringType : filename = nl->StringValue(nl->Fourth(list));
                           break;
         case TextType : nl->Text2String(nl->Fourth(list),filename);
                           break;
        }

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
          ((nl->AtomType( nl->Fourth( list )) == SymbolType) ||
           (nl->AtomType( nl->Fourth( list )) == StringType) ||
           (nl->AtomType( nl->Fourth( list )) == TextType))   )
      {
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
        iosock << csp->startObjectRestore << endl
               << nl->SymbolValue( nl->Second( list ) ) << endl;

        // send file data
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
          ((nl->AtomType( nl->Fifth( list )) == SymbolType) ||
           (nl->AtomType( nl->Fifth( list )) == StringType) ||
           (nl->AtomType( nl->Fifth( list )) == TextType))   )
      {
        // send object symbol
        iosock << csp->startDbRestore << endl
               << nl->SymbolValue( nl->Third( list ) ) << endl;

        // send file data
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
                                   const string& serverFileName){
   if(localfilename.empty() || serverFileName.empty()){
        return ERR_INVALID_FILE_NAME;
   }

   iostream& iosock = server->GetSocketStream();
   iosock << csp->startFileTransfer << endl;
   iosock << serverFileName << endl;
   csp->SendFile(localfilename);
   iosock << csp->endFileTransfer << endl;
   ListExpr resultList;
   int errorCode; 
   int errorPos;
   string errorMessage;
   errorCode = csp->ReadResponse( resultList,
                                  errorCode, errorPos,
                                  errorMessage         );
   return errorCode;
}

int SecondoInterfaceCS::requestFile(const string& serverFilename,
                                    const string& localFilename){
   iostream& iosock = server->GetSocketStream();
   iosock << csp->startRequestFile << endl;
   iosock << serverFilename << endl;
   iosock << csp->endRequestFile << endl;
   // the answer may be SecondoError or SecondoResponse
   bool ok = csp->ReceiveFile(localFilename);
   if(ok){
       return 0;
   } else {
       return ERR_IN_FILETRANSFER;
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
       sufficientMemory = atof( line.c_str());
       getline(iosock,line);
       timeAtSuffMemory = atof( line.c_str());
       getline(iosock,line);
       timeAt16MB = atof(line.c_str()); 
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
       sufficientMemory = atof( line.c_str());
       getline(iosock,line);
       timeAtSuffMemory = atof( line.c_str());
       getline(iosock,line);
       timeAt16MB = atof(line.c_str()); 
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
       sufficientMemory = atof( line.c_str());
       getline(iosock,line);
       timeAtSuffMemory = atof( line.c_str());
       getline(iosock,line);
       timeAt16MB = atof(line.c_str()); 
       getline(iosock,line);
       a = atof(line.c_str());
       getline(iosock,line);
       b = atof(line.c_str());
       getline(iosock,line);
       c = atof(line.c_str());
       getline(iosock,line);
       d = atof(line.c_str());
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
       sufficientMemory = atof( line.c_str());
       getline(iosock,line);
       timeAtSuffMemory = atof( line.c_str());
       getline(iosock,line);
       timeAt16MB = atof(line.c_str()); 
       getline(iosock,line);
       a = atof(line.c_str());
       getline(iosock,line);
       b = atof(line.c_str());
       getline(iosock,line);
       c = atof(line.c_str());
       getline(iosock,line);
       d = atof(line.c_str());
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

