/*    
Implementation of the Secondo Server Module

2002 U. Telle. 
 
2003-2004 M. Spiekermann. Minor modifications for messages,
binary encoded list transfer and error handling of socket streams.

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

static istream&
skipline( istream&  strm )
{
  strm.ignore( INT_MAX, '\n' );
  return strm;
}

class SecondoServer;
typedef void (SecondoServer::*ExecCommand)();

class SecondoServer : public Application
{
 public:
  SecondoServer( const int argc, const char** argv ) : Application( argc, argv ) {};
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
  bool SendFile( const string& clientFileName,
                 const string& serverFileName );
 private:
  Socket*           client;
  SecondoInterface* si;
  NestedList*       nl;
  string            parmFile;
  bool              quit;
  string            registrar;
  string            user;
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
  
  iosock << "<SecondoResponse>" << endl;

  if ( !RTFlag::isActive("Server:BinaryTransfer") ) {
  
    StopWatch* sendTime = 0;
    LOGMSG( "Server:SendTimeMsg",
      sendTime = new StopWatch();
      cerr << "Sending list as textual representation ... ";
    )

    //string resultStr;
    //nl->WriteToString( resultStr, list );
    //iosock << resultStr << endl;
    nl->WriteStringTo(list,iosock);  
    iosock << endl;

    LOGMSG( "Server:SendTimeMsg",
      cerr << sendTime->diffReal() << " " << sendTime->diffCPU() << endl;
    )
    if ( sendTime ) { delete sendTime; } 

  } else {

    if ( RTFlag::isActive("Server:ResultFile") ) {
      ofstream file("result.bnl", ios::out|ios::trunc|ios::binary);
      nl->WriteBinaryTo(list,file);
      file.close();
    }
    
    StopWatch* sendTime = 0;
    LOGMSG( "Server:SendTimeMsg",
      sendTime = new StopWatch();
      cerr << "Sending list as binary representation ... ";
    )

    nl->WriteBinaryTo(list,iosock);
    
    LOGMSG( "Server:SendTimeMsg",
      cerr << sendTime->diffReal() << " " << sendTime->diffCPU() << endl;;
    ) 
    if ( sendTime ) { delete sendTime; } 

  }
  iosock << "</SecondoResponse>" << endl;

}

void
SecondoServer::CallSecondo()
{
  string line, cmdText, cmdEnd;
  iostream& iosock = client->GetSocketStream();
  int level;
  iosock >> level >> skipline;
  bool ready;
  do
  {
    getline( iosock, line );
    ready = (line == "</Secondo>");
    if ( !ready )
    {
      cmdText += line;
    }
  }
  while (!ready && !iosock.fail());
  ListExpr commandLE = nl->TheEmptyList();
  ListExpr resultList = nl->TheEmptyList();
  int errorCode, errorPos;
  string errorMessage;
  si->Secondo( cmdText, commandLE, level, true, false, 
               resultList, errorCode, errorPos, errorMessage );
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
    nl->ReadFromString( typeStr, typeList );
    AlgebraLevel level = (AlgebraLevel) nl->IntValue( nl->First( typeList ) );
    ListExpr list = si->NumericTypeExpr( level, nl->Second( typeList ) );
    nl->WriteToString( typeStr, list );
    iosock << "<NumericTypeResponse>" << endl
           << typeStr << endl
           << "</NumericTypeResponse>" << endl;
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
  AlgebraLevel level;
  string name, cmdEnd;
  int intLevel, algebraId, typeId;
  iostream& iosock = client->GetSocketStream();
  iosock.clear();
  iosock >> intLevel >> name >> skipline;
  level = (AlgebraLevel) intLevel;
  getline( iosock, cmdEnd );
  if ( cmdEnd == "</GetTypeId>" )
  {
    bool ok = si->GetTypeId( level, name, algebraId, typeId );
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
    nl->ReadFromString( typeStr, typeList );
    AlgebraLevel level = (AlgebraLevel) nl->IntValue( nl->First( typeList ) );
    si->LookUpTypeExpr( level, nl->Second( typeList ), name, algebraId, typeId );
    iosock << "<LookUpTypeResponse>" << endl
           << "((" << name << ") " << algebraId << " " << typeId << ")" << endl
           << "</LookUpTypeResponse>" << endl;
  }
  else
  {
    iosock << "<SecondoError>" << endl
           << "SECONDO-0080 Protocol error: </LookUpType> expected." << endl
           << "</SecondoError>" << endl;
  }
}

bool
SecondoServer::SendFile( const string& clientFileName,
                         const string& serverFileName )
{
  bool ok = false;
  iostream& iosock = client->GetSocketStream();
  iosock << "<ReceiveFile>" << endl
         << clientFileName << endl
         << "</ReceiveFile>" << endl;
  string line;
  getline( iosock, line );
  if ( line == "<ReceiveFileReady/>" )
  {
    ifstream serverFile( serverFileName.c_str() );
    iosock << "<ReceiveFileData>" << endl;
    while (!serverFile.eof() && !iosock.fail())
    {
      getline( serverFile, line );
      iosock << line << endl;
    }
    iosock << "</ReceiveFileData>" << endl;
    serverFile.close();
  }
  else
  {
    // in case we get here line should be '<ReceiveFileError/>'
    ok = false;
  }
  return (ok);
}

void
SecondoServer::CallObjectSave()
{
  string clientFileName, serverFileName, objName, cmdEnd;
  iostream& iosock = client->GetSocketStream();
  iosock >> clientFileName >> skipline;
  iosock >> objName >> skipline;
  getline( iosock, cmdEnd );
  if ( cmdEnd == "</ObjectSave>" )
  {
    // serverFileName erzeugen
    ostringstream os;
    os << "SObjectSave" << GetOwnProcessId();
    serverFileName = os.str();
    string cmdText = "(save " + objName + " to " + serverFileName + ")";
    ListExpr commandLE = nl->TheEmptyList();
    ListExpr resultList = nl->TheEmptyList();
    int errorCode, errorPos;
    string errorMessage;
    si->Secondo( cmdText, commandLE, 0, true, false, 
                 resultList, errorCode, errorPos, errorMessage );
    if ( errorCode == 0 )
    {
      SendFile( clientFileName, serverFileName );
    }
    WriteResponse( errorCode, errorPos, errorMessage, resultList );
    FileSystem::DeleteFileOrFolder( serverFileName );
  }
  else
  {
    WriteResponse( 80, 0, "Protocol error: </ObjectSave> expected.", nl->TheEmptyList() );
  }
}

void
SecondoServer::CallDbSave()
{
  string clientFileName, serverFileName, cmdEnd;
  iostream& iosock = client->GetSocketStream();
  iosock >> clientFileName >> skipline;
  getline( iosock, cmdEnd );
  if ( cmdEnd == "</DbSave>" )
  {
    // serverFileName erzeugen
    ostringstream os;
    os << "SdbSave" << GetOwnProcessId();
    serverFileName = os.str();
    string cmdText = "(save database to " + serverFileName + ")";
    ListExpr commandLE = nl->TheEmptyList();
    ListExpr resultList = nl->TheEmptyList();
    int errorCode, errorPos;
    string errorMessage;
    si->Secondo( cmdText, commandLE, 0, true, false, 
                 resultList, errorCode, errorPos, errorMessage );
    if ( errorCode == 0 )
    {
      SendFile( clientFileName, serverFileName );
    }
    WriteResponse( errorCode, errorPos, errorMessage, resultList );
    FileSystem::DeleteFileOrFolder( serverFileName );
  }
  else
  {
    WriteResponse( 80, 0, "Protocol error: </DbSave> expected.", nl->TheEmptyList() );
  }
}

bool
SecondoServer::ReceiveFile( const string& clientFileName,
                            const string& serverFileName )
{
  bool ok = false;
  iostream& iosock = client->GetSocketStream();
  iosock << "<SendFile>" << endl
         << clientFileName << endl
         << "</SendFile>" << endl;
  string line;
  getline( iosock, line );
  if ( line == "<SendFileData>" )
  {
    ofstream serverFile( serverFileName.c_str() );
    bool ready = false;
    while (!ready && !iosock.fail())
    {
      getline( iosock, line );
      ready = (line == "</SendFileData>");
      if ( !ready )
      {
        serverFile << line << endl;
      }
    }
    serverFile.close();
    ok = true;
  }
  return (ok);
}

void
SecondoServer::CallObjectRestore()
{
  string objName, clientFileName, serverFileName, cmdEnd;
  iostream& iosock = client->GetSocketStream();
  iosock >> objName >> clientFileName >> skipline;
  getline( iosock, cmdEnd );
  if ( cmdEnd == "</ObjectRes>" )
  {
    ostringstream os;
    os << "SObjectRest" << GetOwnProcessId();
    serverFileName = os.str();
    if ( ReceiveFile( clientFileName, serverFileName ) )
    {
      string cmdText = "(restore " + objName +
                       " from " + serverFileName + ")";
      ListExpr commandLE = nl->TheEmptyList();
      ListExpr resultList = nl->TheEmptyList();
      int errorCode, errorPos;
      string errorMessage;
      si->Secondo( cmdText, commandLE, 0, true, false, 
                   resultList, errorCode, errorPos, errorMessage );
      WriteResponse( errorCode, errorPos, errorMessage, resultList );
    }
    else
    {
      WriteResponse( 80, 0, "Protocol error: File not received correctly.", nl->TheEmptyList() );
    }
    FileSystem::DeleteFileOrFolder( serverFileName );
  }
  else
  {
    WriteResponse( 80, 0, "Protocol error: </ObjectRes> expected.", nl->TheEmptyList() );
  }
}

void
SecondoServer::CallDbRestore()
{
  string dbName, clientFileName, serverFileName, cmdEnd;
  iostream& iosock = client->GetSocketStream();
  iosock >> dbName >> clientFileName >> skipline;
  getline( iosock, cmdEnd );
  if ( cmdEnd == "</DbRestore>" )
  {
    ostringstream os;
    os << "SdbRest" << GetOwnProcessId();
    serverFileName = os.str();
    if ( ReceiveFile( clientFileName, serverFileName ) )
    {
      string cmdText = "(restore database " + dbName +
                       " from " + serverFileName + ")";
      ListExpr commandLE = nl->TheEmptyList();
      ListExpr resultList = nl->TheEmptyList();
      int errorCode, errorPos;
      string errorMessage;
      si->Secondo( cmdText, commandLE, 0, true, false, 
                   resultList, errorCode, errorPos, errorMessage );
      WriteResponse( errorCode, errorPos, errorMessage, resultList );
    }
    else
    {
      WriteResponse( 80, 0, "Protocol error: File not received correctly.", nl->TheEmptyList() );
    }
    FileSystem::DeleteFileOrFolder( serverFileName );
  }
  else
  {
    WriteResponse( 80, 0, "Protocol error: </DbRestore> expected.", nl->TheEmptyList() );
  }
}

void
SecondoServer::Connect()
{
  iostream& iosock = client->GetSocketStream();
  string line;
  getline( iosock, line );
  ListExpr userinfo;
  user = "-UNKNOWN-";
  if ( nl->ReadFromString( line, userinfo ) )
  {
    if ( nl->First( userinfo ) != nl->TheEmptyList() )
    {
      user = nl->SymbolValue( nl->First( nl->First( userinfo ) ) );
    }
  }
  nl->Destroy( userinfo );
  getline( iosock, line );
  iosock << "<SecondoIntro>" << endl
         << "You are connected with a Secondo server." << endl
         << "</SecondoIntro>" << endl;
  Messenger messenger( registrar );
  string answer;
  ostringstream os;
  os << "LOGIN " << user << " " << GetOwnProcessId();
  messenger.Send( os.str(), answer );
  SmiEnvironment::SetUser( user );
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
  registrar = SmiProfile::GetParameter( "Environment", "RegistrarName", "SECONDO_REGISTRAR", parmFile );
  si = new SecondoInterface();
  if ( si->Initialize( "", "", "", "", parmFile, true ) )
  {
    map<string,ExecCommand> commandTable;
    map<string,ExecCommand>::iterator cmdPos;
    commandTable["<Secondo>"]     = &SecondoServer::CallSecondo;
    commandTable["<NumericType>"] = &SecondoServer::CallNumericType;
    commandTable["<GetTypeId>"]   = &SecondoServer::CallGetTypeId;
    commandTable["<LookUpType>"]  = &SecondoServer::CallLookUpType;
    commandTable["<DbSave>"]      = &SecondoServer::CallDbSave;
    commandTable["<ObjectSave>"]  = &SecondoServer::CallObjectSave;
    commandTable["<ObjectRes>"]   = &SecondoServer::CallObjectRestore;
    commandTable["<DbRestore>"]   = &SecondoServer::CallDbRestore;
    commandTable["<Connect>"]     = &SecondoServer::Connect;
    commandTable["<Disconnect/>"] = &SecondoServer::Disconnect;


    string logMsgList = SmiProfile::GetParameter( "Environment", "RTFlags", "", parmFile );
    RTFlag::initByString(logMsgList);

    nl = si->GetNestedList();
    client = GetSocket();
    if ( client != 0 )
    {
      quit = false;

      iostream& iosock = client->GetSocketStream();
			ios_base::iostate s = iosock.exceptions();
      iosock.exceptions(ios_base::failbit|ios_base::badbit|ios_base::eofbit);
      iosock << "<SecondoOk/>" << endl;

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
                 << "SECONDO-0080 Protocol error: Unknown command." << endl
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
        cerr << endl << "I/O error on socket stream object while sending response!" << endl;
        if ( !client->IsOk() ) {
           cerr << "Socket Error: " << client->GetErrorText() << endl;  
         }
       quit = true; 
      }

      } while (!iosock.fail() && !quit);
      
			iosock.exceptions(s);
			
      client->Close();
      delete client;
      Messenger messenger( registrar );
      string answer;
      ostringstream os;
      os << "LOGOUT " << user << " " << GetOwnProcessId();
      messenger.Send( os.str(), answer );
    }
    else
    {
      rc = -2;
    }
  }
  else
  {
    rc = -1;
  }
  si->Terminate();
  delete si;
  return (rc);
}

int main( const int argc, const char* argv[] )
{
  SecondoServer* appPointer = new SecondoServer( argc, argv );
  int rc = appPointer->Execute();
  delete appPointer;
  return (rc);
}

