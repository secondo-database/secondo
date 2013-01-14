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
    
Implementation of the Secondo Server Module

2002 U. Telle. 
 
2003-2004 M. Spiekermann. Minor modifications for messages,
binary encoded list transfer and error handling of socket streams.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hybrid~). Only the executable
level remains.

February 2006. M. Spiekermann. Changes in the client server protocol. Now
the ~restore~ and ~save~ object/database commands work correctly. Moreover the
code was simplified and restructured. As a consequence protocol related procedures 
are implemented in class ~CSProtocol~ and can be used by inside the 
~TTYCS~ and the ~Server~ code. 



*/

#include <cstdlib>
#include <string>
#include <algorithm>
#include <map>
#include <climits>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include "Application.h"
#include "SocketIO.h"
#include "Messenger.h"
#include "AlgebraTypes.h"
#include "SecondoSystem.h"
#include "SecondoCatalog.h"
#include "SecondoInterface.h"
#include "FileSystem.h"
#include "Profiles.h"
#include "LogMsg.h"
#include "StopWatch.h"

#include "CSProtocol.h"
#include "NList.h"



class SecondoServer;
typedef void (SecondoServer::*ExecCommand)();

class SecondoServer : public Application
{
 public:
  SecondoServer( const int argc, const char** argv ) : 
     Application( argc, argv ) 
  {};
  virtual ~SecondoServer() {};
  int Execute();
  void CallSecondo();
  void CallNumericType();
  void CallGetTypeId();
  void CallLookUpType();
  void CallDbSave();
  void CallObjectSave();
  void CallDbRestore();
  void CallObjectRestore();
  void Connect();
  void Disconnect();
  void WriteResponse( const int errorCode, const int errorPos,
                      const string& errorMessage, ListExpr resultList );
  bool ReceiveFile( const string& clientFileName,
                    const string& serverFileName );
  void CallGetOperatorIndexes();
  void CallGetCosts();
  void CallGetLinearCostFun();
  void CallGetCostFun();
  
 private:

  void CallRestore(const string& tag, bool database=false);
  void CallSave(const string& tag, bool database=false);
  string CreateTmpName(const string& prefix);
  
  Socket*           client;
  SecondoInterface* si;
  NestedList*       nl;
  string            parmFile;
  bool              quit;
  string            registrar;
  string            user;
  string            pswd;

  CSProtocol* csp;
};


void
SecondoServer::WriteResponse( const int errorCode, const int errorPos,
                              const string& errorMessage, ListExpr resultList )
{
  ListExpr msg = nl->TextAtom();
  nl->AppendText( msg, errorMessage );
  
  ListExpr list = nl->FourElemList(
                    nl->IntAtom( errorCode ),
                    nl->IntAtom( errorPos ),
                    msg,
                    resultList );

  iostream& iosock = client->GetSocketStream();
 
  csp->IgnoreMsg(true); 
  iosock << "<SecondoResponse>" << endl;
  csp::sendList(iosock, NList(list));  
  iosock << "</SecondoResponse>" << endl;
  
}

void
SecondoServer::CallSecondo()
{
  string line="", cmdText="", cmdEnd="";
  iostream& iosock = client->GetSocketStream();
  int type=0;
  iosock >> type;
  csp->skipRestOfLine();
  csp->IgnoreMsg(false);
  bool ready=false;
  do
  {
    getline( iosock, line );
    ready = (line == "</Secondo>");
    if ( !ready )
    {
      cmdText += line + "\n";
    }
  }
  while (!ready && !iosock.fail());
  ListExpr commandLE = nl->TheEmptyList();
  ListExpr resultList = nl->TheEmptyList();
  int errorCode=0, errorPos=0;
  string errorMessage="";
  si->Secondo( cmdText, commandLE, type, true, false, 
               resultList, errorCode, errorPos, errorMessage );
  NList::setNLRef(nl);
  WriteResponse( errorCode, errorPos, errorMessage, resultList );
}

void
SecondoServer::CallNumericType()
{
  string typeStr, cmdEnd;
  iostream& iosock = client->GetSocketStream();
  iosock.clear();
  getline( iosock, typeStr );
  getline( iosock, cmdEnd );
  if ( cmdEnd == "</NumericType>" )
  {
    ListExpr typeList = 0;
    nl->ReadFromString( "("+typeStr+")", typeList );
    ListExpr list = si->NumericTypeExpr( nl->First(typeList) );
    nl->WriteToString( typeStr, list );
    iosock << "<NumericTypeResponse>" << endl
           << typeStr << endl
           << "</NumericTypeResponse>" << endl;
    nl->Destroy(list);
    nl->Destroy(typeList);
  }
  else
  {
    iosock << "<SecondoError>" << endl
           << "SECONDO-0080 Protocol error: </NUMERICTYPE> expected." << endl
           << "</SecondoError>" << endl;
  }
}

void
SecondoServer::CallGetTypeId()
{
  string name, cmdEnd;
  int algebraId, typeId;
  iostream& iosock = client->GetSocketStream();
  iosock.clear();
  iosock >> name;
  csp->skipRestOfLine();
  getline( iosock, cmdEnd );
  if ( cmdEnd == "</GetTypeId>" )
  {
    bool ok = si->GetTypeId( name, algebraId, typeId );
    if ( ok )
    {
      iosock << "<GetTypeIdResponse>" << endl
             << algebraId << " " << typeId << endl
             << "</GetTypeIdResponse>" << endl;
    }
    else
    {
      iosock << "<GetTypeIdResponse>" << endl
             << 0 << " " << 0 << endl
             << "</GetTypeIdResponse>" << endl;
    }
  }
  else
  {
    iosock << "<SecondoError>" << endl
           << "SECONDO-0080 Protocol error: </GetTypeId> expected." << endl
           << "</SecondoError>" << endl;
  }
}

void
SecondoServer::CallLookUpType()
{
  string name, typeStr, cmdEnd;
  int algebraId, typeId;
  iostream& iosock = client->GetSocketStream();
  getline( iosock, typeStr );
  getline( iosock, cmdEnd );
  if ( cmdEnd == "</LookUpType>" )
  {
    ListExpr typeList;
    nl->ReadFromString( "(" + typeStr + ")", typeList );
    si->LookUpTypeExpr( nl->First(typeList), name, algebraId, typeId );

    iosock << "<LookUpTypeResponse>" << endl
           << "((" << name << ") " << algebraId << " " << typeId << ")" << endl
           << "</LookUpTypeResponse>" << endl;
    nl->Destroy(typeList);
  }
  else
  {
    iosock << "<SecondoError>" << endl
           << "SECONDO-0080 Protocol error: </LookUpType> expected." << endl
           << "</SecondoError>" << endl;
  }
}


void SecondoServer::CallGetOperatorIndexes(){

   string name;
   ListExpr args;
   iostream& iosock = client->GetSocketStream();
   getline(iosock,name);
   AlgebraManager* am = SecondoSystem::GetAlgebraManager();
   //NestedList* nl = am->getListStorage();
   NestedList* nl1 = SecondoSystem::GetNestedList(); 

   nl1->ReadBinaryFrom(iosock, args);

   string cmdEnd;
   getline(iosock, cmdEnd);
   if(cmdEnd=="</REQUESTOPERATORINDEXES>"){
       iosock << "<OPERATORINDEXESRESPONSE>" << endl;
       int algId;
       int OpId;
       int funId;
       ListExpr resList;
    
       NList::setNLRef(nl1); 
       bool ok = am->findOperator(name,args,resList, algId, OpId,funId);
       NList::setNLRef(nl);
      
       stringstream ss;
       ss << (ok?"1":"0") << endl;
       ss << algId << endl << OpId << endl << funId << endl;
       iosock << ss.str();
       nl1->WriteBinaryTo(resList, iosock);
       iosock << endl;
       iosock << "</OPERATORINDEXESRESPONSE>" << endl;
   } else {
     
    iosock << "<SecondoError>" << endl
           << "SECONDO-0080 Protocol error: "
           << "</REQUESTOPERATORINDEXES> expected." << endl
           << " received '" << cmdEnd << "'" << endl
           << "</SecondoError>" << endl;
   }
}

void SecondoServer::CallGetCosts(){

   AlgebraManager* am = SecondoSystem::GetAlgebraManager();
   iostream& iosock = client->GetSocketStream();
   string line;
   getline(iosock,line);

   int noStreams = atoi(line.c_str());
   if((noStreams!=1) && (noStreams!=2)){
    iosock << "<SecondoError>" << endl
           << "SECONDO-0080 Protocol error: getCosts "
           << "can handle only 2 streams" << endl
           << "</SecondoError>" << endl;
     return;
   }
   // operator identifier
   getline(iosock,line);
   int algId = atoi(line.c_str());
   getline(iosock,line);
   int opId = atoi(line.c_str());
   getline(iosock,line);
   int funId = atoi(line.c_str());
   // informations about the first tuple stream
   // there is at least one
   getline(iosock,line);
   int noTuples1 =  atoi(line.c_str());
   getline(iosock,line);
   int sizeOfTuple1 = atoi(line.c_str());
   getline(iosock,line);
   int noAttributes1 = atoi(line.c_str());
   size_t costs;
   bool ok;
   if(noStreams==1){ // there is only 1 stream
      getline(iosock,line);
      double selectivity = atof(line.c_str());
      getline(iosock,line);
      int memoryMB = atoi(line.c_str());
      ok = am->getCosts(algId,opId,funId,
                        noTuples1,sizeOfTuple1, noAttributes1, 
                        selectivity,memoryMB,costs);
   } else {
      // get information about stream 2
      getline(iosock,line);
      int noTuples2 = atoi(line.c_str());
      getline(iosock,line);
      int sizeOfTuple2 = atoi(line.c_str());
      getline(iosock,line);
      int noAttributes2 = atof(line.c_str());
      getline(iosock,line);
      double selectivity = atof(line.c_str());
      getline(iosock,line);
      int memoryMB = atoi(line.c_str());

      ok = am->getCosts(algId,opId,funId,
                        noTuples1,sizeOfTuple1, noAttributes1,
                        noTuples2,sizeOfTuple2, noAttributes2,
                        selectivity, memoryMB, costs);
   }
 
    
   getline(iosock,line);
   if(line!="</GETCOSTS>"){
      iosock << "<SecondoError>" << endl
             << "SECONDO-0080 Protocol error: </GETCOSTS> "
             << " expected" << endl
             << " received '" << line << "'" << endl
             << "</SecondoError>" << endl;
      return;
   }
   iosock << "<COSTRESPONSE>" << endl;
   stringstream ss;
   ss << (ok?"1":"0") << endl;
   ss << costs << endl;
   iosock << ss.str();
   iosock << "</COSTRESPONSE>" << endl;   
   iosock.flush();
}


void SecondoServer::CallGetLinearCostFun(){
   
   AlgebraManager* am = SecondoSystem::GetAlgebraManager();
   iostream& iosock = client->GetSocketStream();
   
   string line;
   getline(iosock,line);
   int noStreams = atoi(line.c_str());
   if((noStreams!=1) && (noStreams!=2)){
    iosock << "<SecondoError>" << endl
           << "SECONDO-0080 Protocol error: getCosts "
           << "can handle only 2 streams" << endl
           << "</SecondoError>" << endl;
     return;
   }
   getline(iosock,line);
   int algId = atoi(line.c_str());
   getline(iosock,line);
   int opId = atoi(line.c_str());
   getline(iosock,line);
   int funId = atoi(line.c_str());
   getline(iosock,line);
   int noTuples1 =  atoi(line.c_str());
   getline(iosock,line);
   int sizeOfTuple1 = atoi(line.c_str());
   getline(iosock,line);
   int noAttributes1 = atoi(line.c_str());
   
   




   bool ok;
   double sufficientMemory;
   double timeAtSuffMemory;
   double timeAt16MB;
   if(noStreams==1){
      getline(iosock,line);
      double selectivity = atof(line.c_str());
      ok = am->getLinearParams(algId,opId,funId,
                               noTuples1,sizeOfTuple1, noAttributes1,
                                selectivity,
                               sufficientMemory,timeAtSuffMemory,timeAt16MB);
   } else {
      getline(iosock,line);
      int noTuples2 = atoi(line.c_str());
      getline(iosock,line);
      int sizeOfTuple2 = atoi(line.c_str());
      getline(iosock,line);
      int noAttributes2 = atoi(line.c_str());
      getline(iosock,line);
      double selectivity = atof(line.c_str());

      ok = am->getLinearParams(algId, opId, funId,
                               noTuples1, sizeOfTuple1, noAttributes1,
                               noTuples2, sizeOfTuple2, noAttributes2,
                               selectivity,
                               sufficientMemory,timeAtSuffMemory,timeAt16MB);
   }
   getline(iosock,line);
   if(line!="</GETLINEARCOSTFUN>"){
      iosock << "<SecondoError>" << endl
             << "SECONDO-0080 Protocol error: </GETCOSTS> "
             << " expected" << endl
             << " received '" << line << "'" << endl
             << "</SecondoError>" << endl;
      return;
   }
   stringstream ss;
   ss << "<LINEARCOSTFUNRESPONSE>" << endl;
   ss << (ok?"1":"0") << endl;
   ss << sufficientMemory << endl;
   ss << timeAtSuffMemory << endl;
   ss << timeAt16MB << endl;
   ss << "</LINEARCOSTFUNRESPONSE>" << endl;   
   iosock << ss.str();
   iosock.flush();
}


void SecondoServer::CallGetCostFun(){
   
   AlgebraManager* am = SecondoSystem::GetAlgebraManager();
   iostream& iosock = client->GetSocketStream();
   string line;
   getline(iosock,line);
   int noStreams = atoi(line.c_str());
   if((noStreams!=1) && (noStreams!=2)){
    iosock << "<SecondoError>" << endl
           << "SECONDO-0080 Protocol error: getCosts "
           << "can handle only 2 streams" << endl
           << "</SecondoError>" << endl;
     return;
   }
   getline(iosock,line);
   int algId = atoi(line.c_str());
   getline(iosock,line);
   int opId = atoi(line.c_str());
   getline(iosock,line);
   int funId = atoi(line.c_str());
   getline(iosock,line);
   int noTuples1 =  atoi(line.c_str());
   getline(iosock,line);
   int sizeOfTuple1 = atoi(line.c_str());
   getline(iosock,line);
   int noAttributes1 = atoi(line.c_str());
   bool ok;
   int funType;
   double sufficientMemory;
   double timeAtSuffMemory;
   double timeAt16MB;
   double a;
   double b;
   double c;
   double d;
   if(noStreams==1){
      getline(iosock,line);
      double selectivity  =  atof(line.c_str());
      ok = am->getFunction(algId, opId, funId,
                           noTuples1, sizeOfTuple1, noAttributes1,
                           selectivity,
                           funType,sufficientMemory,timeAtSuffMemory,timeAt16MB,
                           a,b,c,d);
   } else { // nostreams ==2
      getline(iosock,line);
      int noTuples2 = atoi(line.c_str());
      getline(iosock,line);
      int sizeOfTuple2 = atoi(line.c_str());
      getline(iosock,line);
      int noAttributes2 = atoi(line.c_str());
      getline(iosock,line);
      double selectivity = atof(line.c_str()); 

      ok = am->getFunction(algId, opId, funId,
                           noTuples1, sizeOfTuple1, noAttributes1,
                           noTuples2, sizeOfTuple2, noAttributes2,
                           selectivity,
                           funType,sufficientMemory,timeAtSuffMemory,timeAt16MB,
                           a,b,c,d);
   }
   getline(iosock,line);
   if(line!="</GETCOSTFUN>"){
      iosock << "<SecondoError>" << endl
             << "SECONDO-0080 Protocol error: </GETCOSTS> "
             << " expected" << endl
             << " received '" << line << "'" << endl
             << "</SecondoError>" << endl;
      return;
   }
   stringstream ss;
   ss << "<COSTFUNRESPONSE>" << endl;
   ss << (ok?"1":"0") << endl;
   ss << funType << endl;
   ss << sufficientMemory << endl;
   ss << timeAtSuffMemory << endl;
   ss << timeAt16MB << endl;
   ss << a << endl << b << endl << c << endl << d << endl;
   ss << "</COSTFUNRESPONSE>" << endl;   
   iosock << ss.str();
   iosock.flush(); 
}

string
SecondoServer::CreateTmpName(const string& prefix)
{
  ostringstream os;
  os << prefix << GetOwnProcessId();
  return os.str();
}       


void
SecondoServer::CallSave(const string& tag, bool database /*=false*/)
{

  //cout << "Begin CallSave()" << endl;
 
  // parameters for Secondo interface call
  SI_Error errorCode=ERR_NO_ERROR;
  int errorPos=0;
  string errorMessage="";
  ListExpr commandLE = nl->TheEmptyList();
  ListExpr resultList = nl->TheEmptyList();

  string cmdText="";
  string serverFileName = CreateTmpName(tag);
  // Read client message and construct Secondo command
  if (!database)
  {
    // read in object or database name
    iostream& iosock = client->GetSocketStream();
    string name = "";
    iosock >> name;
    csp->skipRestOfLine();

    if(!csp->nextLine("</"+tag+">", errorMessage))
      errorCode = ERR_IN_SECONDO_PROTOCOL;
      
    cmdText= "(save " + name + " to " + serverFileName + ")";
  }
  else
  {
    cmdText = "(save database to " + serverFileName + ")";
  }  
  
  // create file on server 
  if (errorCode == ERR_NO_ERROR)
  {
   
    si->Secondo( cmdText, commandLE, 0, true, false, 
                 resultList, errorCode, errorPos, errorMessage );
    NList::setNLRef(nl); 

  } 

  // If successful create list from file
  if (errorCode == ERR_NO_ERROR)
  {
    nl->ReadFromFile( serverFileName, resultList );
  } 
   
  WriteResponse( errorCode, errorPos, errorMessage, resultList );
  //FileSystem::DeleteFileOrFolder( serverFileName );

  //cout << "End CallSave()" << endl;
}


void
SecondoServer::CallDbSave()
{
  CallSave("DbSave", true);
}


void
SecondoServer::CallObjectSave()
{
  CallSave("ObjectSave");
}


void
SecondoServer::CallRestore(const string& tag, bool database/*=false*/)
{
  // parameters for Secondo interface call
  SI_Error errorCode=0;
  int errorPos=0;
  string errorMessage="";
  ListExpr commandLE = nl->TheEmptyList();
  ListExpr resultList = nl->TheEmptyList();
  
  // read in object or database name
  iostream& iosock = client->GetSocketStream();
  string name = "";
  iosock >> name;
  csp->skipRestOfLine();
  
  //cout << "Begin CallRestore()" << endl;
  
  string serverFileName= CreateTmpName(tag);
  if ( csp->ReceiveFile( serverFileName ) )
  {
    string cmdText="";

    // construct Secondo command
    if (database)
    {
      cmdText = "(restore database " + name +
                " from " + serverFileName + ")";
            
    }
    else
    {       
      cmdText= "(restore " + name +
               " from " + serverFileName + ")";
    } 
    
    si->Secondo( cmdText, commandLE, 0, true, false, 
                 resultList, errorCode, errorPos, errorMessage );
    NList::setNLRef(nl);
  }
  else
  {
    
    errorCode = ERR_IN_SECONDO_PROTOCOL;
    errorMessage = "Protocol-Error: File not received correctly."; 
    resultList = nl->TheEmptyList();
  }
  FileSystem::DeleteFileOrFolder(serverFileName);
  
  if (!csp->nextLine("</"+tag+">", errorMessage) )
  {
    errorCode = ERR_IN_SECONDO_PROTOCOL;
  }
  WriteResponse( errorCode, errorPos, errorMessage, resultList );
  //FileSystem::DeleteFileOrFolder( serverFileName );

  //cout << "End CallRestore()" << endl;
}


void
SecondoServer::CallObjectRestore()
{
  CallRestore("ObjectRestore");    
}       

void
SecondoServer::CallDbRestore()
{
  CallRestore("DbRestore",true);        
}       


void
SecondoServer::Connect()
{
  iostream& iosock = client->GetSocketStream();
  string line;
  getline( iosock, user );
  getline( iosock, pswd );
  cout << "user = " << user << endl;
  cout << "passwd = " << pswd << endl;
  getline( iosock, line ); //eat up </USER> ?
  
}

void
SecondoServer::Disconnect()
{
  quit = true;
}

int
SecondoServer::Execute()
{
  int rc = 0;
  parmFile = (GetArgCount() > 1) ? GetArgValues()[1] : "SecondoConfig.ini";
  registrar = SmiProfile::GetUniqueSocketName( parmFile );
  si = new SecondoInterface(true);
  cout << "Initialize the secondo interface " << endl;

  map<string,ExecCommand> commandTable;
  map<string,ExecCommand>::iterator cmdPos;
  commandTable["<Secondo>"]     = &SecondoServer::CallSecondo;
  commandTable["<NumericType>"] = &SecondoServer::CallNumericType;
  commandTable["<GetTypeId>"]   = &SecondoServer::CallGetTypeId;
  commandTable["<LookUpType>"]  = &SecondoServer::CallLookUpType;
  commandTable["<DbSave/>"]      = &SecondoServer::CallDbSave;
  commandTable["<ObjectSave>"]  = &SecondoServer::CallObjectSave;
  commandTable["<ObjectRestore>"]   = &SecondoServer::CallObjectRestore;
  commandTable["<DbRestore>"]   = &SecondoServer::CallDbRestore;
  commandTable["<Connect>"]     = &SecondoServer::Connect;
  commandTable["<Disconnect/>"] = &SecondoServer::Disconnect;
  commandTable["<REQUESTOPERATORINDEXES>"] = 
                                    &SecondoServer::CallGetOperatorIndexes;
  commandTable["<GETCOSTS>"] = &SecondoServer::CallGetCosts;
  commandTable["<GETLINEARCOSTFUN>"] = &SecondoServer::CallGetLinearCostFun;
  commandTable["<GETCOSTFUN>"] = &SecondoServer::CallGetCostFun;

  string logMsgList = SmiProfile::GetParameter( "Environment", 
                                                "RTFlags", "", parmFile );
  RTFlag::initByString(logMsgList);


  client = GetSocket();
  if(!client){
    rc = -2;
  } else {
    iostream& iosock = client->GetSocketStream();
    csp = new CSProtocol(nl, iosock, true);
    //si->SetProtocolPtr(csp);
    
    ios_base::iostate s = iosock.exceptions();
    iosock.exceptions(ios_base::failbit|ios_base::badbit|ios_base::eofbit);
    iosock << "<SecondoOk/>" << endl;
    quit = false;
    // first connect to get the user and password information
    try {
        string cmd;
        getline( iosock, cmd );
        cmdPos = commandTable.find( cmd );
        if ( cmdPos != commandTable.end() )
        {
           (*this.*(cmdPos->second))();
        }
        else
        {
          iosock << "<SecondoError>" << endl
                 << "SECONDO-0080 Protocol-Error: Start tag \"" 
                 << cmd << "\" unknown!" << endl
                 << "</SecondoError>" << endl;
        }
        if ( Application::Instance()->ShouldAbort() )
        {
          iosock << "<SecondoError>" << endl
                 << "SECONDO-9999 Server going down. Disconnecting." << endl
                 << "</SecondoError>" << endl;
          quit = true;
        }
    
      } catch (ios_base::failure) {
        cerr << endl 
             << "I/O error on socket stream object!" 
             << endl;
        if ( !client->IsOk() ) {
           cerr << "Socket Error: " << client->GetErrorText() << endl;  
         }
       quit = true; 
      }

    cout << "Try to initialize the secondo system " << endl; 
    string errorMsg("");
    if ( si->Initialize( user, pswd, "", "", parmFile,errorMsg,  true ) )
    {
       cout << "initialization successful" << endl;
       iosock << "<SecondoIntro>" << endl
              << "You are connected with a Secondo server." << endl
              << "</SecondoIntro>" << endl;
       //Messenger messenger( registrar );
       //string answer;
       //ostringstream os;
       //os << "LOGIN " << user << " " << GetOwnProcessId();
       //messenger.Send( os.str(), answer );
       SmiEnvironment::SetUser( user );

      // initialization successfull, send ok and wait for further requests



      nl = si->GetNestedList();
      NList::setNLRef(nl);


      do {
      try {
        string cmd;
        getline( iosock, cmd );
        cmdPos = commandTable.find( cmd );
        if ( cmdPos != commandTable.end() )
        {
           (*this.*(cmdPos->second))();
        }
        else
        {
          iosock << "<SecondoError>" << endl
                 << "SECONDO-0080 Protocol-Error: Start tag \"" 
                 << cmd << "\" unknown!" << endl
                 << "</SecondoError>" << endl;
        }
        if ( Application::Instance()->ShouldAbort() )
        {
          iosock << "<SecondoError>" << endl
                 << "SECONDO-9999 Server going down. Disconnecting." << endl
                 << "</SecondoError>" << endl;
          quit = true;
        }
    
      } catch (ios_base::failure) {
        cerr << endl 
             << "I/O error on socket stream object!" 
             << endl;
        if ( !client->IsOk() ) {
           cerr << "Socket Error: " << client->GetErrorText() << endl;  
         }
       quit = true; 
      }

      } while (!iosock.fail() && !quit);
      
      iosock.exceptions(s);

      client->Close();
      delete client;
      //os << "LOGOUT " << user << " " << GetOwnProcessId();
      //messenger.Send( os.str(), answer );
    } else {
       iosock << "<SecondoError>" << endl
              << "Initialization failed (username, password correct?)" << endl
              << errorMsg  
              << "</SecondoError>" << endl;
      rc = -1;
    }
  }

  si->Terminate();
  delete si;
  return (rc);
}

int SecondoServerMode( const int argc, const char* argv[] )
{
  const char* fname = "SecondoServer.msg";	
  ofstream fmsg;
  fmsg.open(fname, ios::app);

  cout << "Redirecting server output to file " << fname << endl;

  streambuf* backup1 = cout.rdbuf();   // back up cout's streambuf
  streambuf* backup2 = cerr.rdbuf();   

  cout.rdbuf(fmsg.rdbuf());   // assign streambuf to cout and cerr
  cerr.rdbuf(fmsg.rdbuf());

  SecondoServer* appPointer = new SecondoServer( argc, argv );
  int rc = appPointer->Execute();
  delete appPointer;

  fmsg.close();
  cout.rdbuf(backup1);
  cerr.rdbuf(backup2);

  return (rc);
}

