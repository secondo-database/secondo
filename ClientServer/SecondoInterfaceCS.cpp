/*
1 The Implementation-Module SecondoInterface

April 2002 Ulrich Telle Client/Server version of the SecondoInterface

April 29 2003 Hoffmann Client/Server adaption for single objects save and 
restore commands.

\tableofcontents

*/

using namespace std;

#include <iostream>
#include <fstream>
#include <sstream>

#include "SecondoInterface.h"
#include "Profiles.h"

static istream&
skipline( istream&  strm )
{
  strm.ignore( INT_MAX, '\n' );
  return (strm);
}

SecondoInterface::SecondoInterface()
  : initialized( false ), activeTransaction( false ), server( 0 )
{
  nl = new NestedList();
  al = nl;
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
  string line;
  if ( !initialized )
  {
    cout << "Initializing the Secondo system ..." << endl;

    // Connect with server, needed host and port
    if ( secHost.length() == 0 || secPort.length() == 0 )
    {
      if ( parmFile.length() != 0 )
      {
        secHost = SmiProfile::GetParameter( "Environment", "SecondoHost", "", parmFile );
        secPort = SmiProfile::GetParameter( "Environment", "SecondoPort", "", parmFile );
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
                           const int commandLevel,
                           const bool commandAsText,
                           const bool resultAsText,
                           ListExpr& resultList,
                           int& errorCode,
                           int& errorPos,
                           string& errorMessage,
                           const string& resultFileName /* = "SecondoResult" */ )
{
/*
~Secondo~ reads a command and executes it; it possibly returns a result.
The command is one of a set of SECONDO commands. 

Error Codes: see definition module.

If value 0 is returned, the command was executed without error.

*/

  string cmdText;
  ListExpr list, errorList, errorInfo;
  string filename, dbName, objName, typeName;
  string listCommand;         /* buffer for command in list form */
  bool readResponse = false;

  errorMessage = "";
  errorCode    = 0;
  errorList    = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  errorInfo    = errorList;
  resultList   = nl->TheEmptyList();

  if ( server == 0 )
  {
    errorCode = 80;
  }
  else
  {
    switch (commandLevel)
    {
      case 0:  // executable, list form
      case 2:  // descriptive, list form
      {
        if ( commandAsText )
        {
          cmdText = commandText;
        }
        else
        {
          if ( !nl->WriteToString( cmdText, commandLE ) )
          {
            errorCode = 9;  // syntax error in command/expression
          }
        }
        break;
      }
      case 1:  // executable, text form
      case 3:  // descriptive, text form
      {
        cmdText = commandText;
        break;
      }
      default:
      {
        errorCode = 31;  // Command level not implemented
      }
    } // switch
  }
  string line;
  iostream& iosock = server->GetSocketStream();
  if ( iosock.fail() )
  {
    errorCode = 81;
  }
  if ( errorCode != 0 )
  {
    return;
  }

  string::size_type posDatabase = cmdText.find( "database" );
  string::size_type posSave     = cmdText.find( "save" );
  string::size_type posRestore  = cmdText.find( "restore" );
  string::size_type posTo       = cmdText.find( "to" );
  string::size_type posFrom     = cmdText.find( "from" );

  if ( posDatabase != string::npos &&
       posSave     != string::npos &&
       posTo       != string::npos &&
       posSave < posDatabase && posDatabase < posTo )
  {
    if ( commandLevel == 1 || commandLevel == 3 )
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
        iosock << "<DbSave>" << endl
               << filename << endl
               << "</DbSave>" << endl;
        getline( iosock, line );
        if ( line == "<ReceiveFile>" )
        {
          getline( iosock, filename );
          getline( iosock, line );          // Hope it is '</ReceiveFile>'
          ofstream restoreFile( filename.c_str() );
          if ( restoreFile )
          {
            iosock << "<ReceiveFileReady/>" << endl;
            getline( iosock, line );
            if ( line == "<ReceiveFileData>" )
            {
              while (line != "</ReceiveFileData>" && !iosock.fail())
              {
                getline( iosock, line );
                if ( line != "</ReceiveFileData>" )
                {
                  restoreFile << line << endl;
                }
              }
            }
            restoreFile.close();
          }
          else
          {
            iosock << "<ReceiveFileError/>" << endl;
          }
        }
        getline( iosock, line );
        readResponse = true;
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
      errorCode = 9;
    }
  }
  else if ( posSave != string::npos && // save object to filename
            posTo   != string::npos &&
	    posDatabase == string::npos &&                        
	    posSave < posTo )
  {
    if ( commandLevel == 1 || commandLevel == 3 )
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
        iosock << "<ObjectSave>" << endl
               << filename << endl
	       << objName << endl
               << "</ObjectSave>" << endl;
        getline( iosock, line );
        if ( line == "<ReceiveFile>" )
        {
          getline( iosock, filename );
          getline( iosock, line );          // Hope it is '</ReceiveFile>'
          ofstream restoreFile( filename.c_str() );
          if ( restoreFile )
          {
            iosock << "<ReceiveFileReady/>" << endl;
            getline( iosock, line );
            if ( line == "<ReceiveFileData>" )
            {
              while (line != "</ReceiveFileData>" && !iosock.fail())
              {
                getline( iosock, line );
                if ( line != "</ReceiveFileData>" )
                {
                  restoreFile << line << endl;
                }
              }
            }
            restoreFile.close();
          }
          else
          {
            iosock << "<ReceiveFileError/>" << endl;
          }
        }
        getline( iosock, line );
        readResponse = true;
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
      errorCode = 9;
    }
  }
  
  else if ( posRestore  != string::npos &&
            posDatabase == string::npos &&
            posFrom     != string::npos &&
            posRestore < posFrom )
  {
    if ( commandLevel == 1 || commandLevel == 3 )
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
        filename = nl->SymbolValue( nl->Fourth( list ) ); 
        iosock << "<ObjectRes>" << endl
               << nl->SymbolValue( nl->Second( list ) )
               << " " << filename << endl
               << "</ObjectRes>" << endl;
        getline( iosock, line );
        if ( line == "<SendFile>" )
        {
          getline( iosock, filename );
          getline( iosock, line );          // Hope it is '</SendFile>'
          ifstream restoreFile( filename.c_str() );
          if ( restoreFile )
          {
            iosock << "<SendFileData>" << endl;
            while (!restoreFile.eof() && !iosock.fail())
            {
              getline( restoreFile, line );
              iosock << line << endl;
            }
            iosock << "</SendFileData>" << endl;
            restoreFile.close();
          }
          else
          {
            iosock << "<SendFileError/>" << endl;
          }
        }
        getline( iosock, line );
        readResponse = true;
      }
      else
      {
        // Not a valid 'restore object' command
        errorCode = 1;
      }
    }
    else
    {
      // Syntax error in list
      errorCode = 9;
    }
  }
  
  else if ( posDatabase != string::npos &&
            posRestore  != string::npos &&
            posFrom     != string::npos &&
            posRestore < posDatabase && posDatabase < posFrom )
  {
    if ( commandLevel == 1 || commandLevel == 3 )
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
        filename = nl->SymbolValue( nl->Fifth( list ) ); 
        iosock << "<DbRestore>" << endl
               << nl->SymbolValue( nl->Third( list ) )
               << " " << filename << endl
               << "</DbRestore>" << endl;
        getline( iosock, line );
        if ( line == "<SendFile>" )
        {
          getline( iosock, filename );
          getline( iosock, line );          // Hope it is '</SendFile>'
          ifstream restoreFile( filename.c_str() );
          if ( restoreFile )
          {
            iosock << "<SendFileData>" << endl;
            while (!restoreFile.eof() && !iosock.fail())
            {
              getline( restoreFile, line );
              iosock << line << endl;
            }
            iosock << "</SendFileData>" << endl;
            restoreFile.close();
          }
          else
          {
            iosock << "<SendFileError/>" << endl;
          }
        }
        getline( iosock, line );
        readResponse = true;
      }
      else
      {
        // Not a valid 'restore database' command
        errorCode = 1;
      }
    }
    else
    {
      // Syntax error in list
      errorCode = 9;
    }
  }
  else
  {
    // Send Secondo command
    iosock << "<Secondo>" << endl
           << commandLevel << endl
           << cmdText << endl
           << "</Secondo>" << endl;
    // Receive result
    getline( iosock, line );
    readResponse = true;
  }
  if ( readResponse )
  {
    if ( line == "<SecondoResponse>" )
    {
      string result = "";
      do
      {
        getline( iosock, line );
        if ( line != "</SecondoResponse>" )
        {
          result += line + "\n";
        }
      }
      while (line != "</SecondoResponse>" && !iosock.fail());
      
      // Decode 'n'-character in text atoms from transmission via TCP/IP
      //if ( result.find("<text>") )
      //{
        //for (unsigned int i = 0; i <= result.length(); i++)
        //{
          //if ( result[i] == line_feed ) result[i] = '\n';
        //}
       //}
      
      nl->ReadFromString( result, resultList );
      errorCode = nl->IntValue( nl->First( resultList ) );
      errorPos  = nl->IntValue( nl->Second( resultList ) );
      TextScan ts = nl->CreateTextScan( nl->Third( resultList ) );
      nl->GetText( ts, nl->TextLength( nl->Third( resultList ) ), errorMessage );
      nl->DestroyTextScan( ts );
      resultList = nl->Fourth( resultList );
    }
    else if ( line == "<SecondoError>" )
    {
      errorCode = 80;
      getline( iosock, errorMessage );
      getline( iosock, line );
    }
    else
    {
      errorCode = 80;
    }
  }
  if ( resultAsText )
  {
    nl->WriteToFile( resultFileName, resultList );
  }
}

/*
1.3 Procedure ~NumericTypeExpr~

*/
ListExpr
SecondoInterface::NumericTypeExpr( const AlgebraLevel level, const ListExpr type )
{
  ListExpr list = nl->TheEmptyList();
  if ( server != 0 )
  {
    string line;
    if ( nl->WriteToString( line, type ) )
    {
      iostream& iosock = server->GetSocketStream();
      iosock << "<NumericType>" << endl
             << "(" << level << " " << line << ")" << endl
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
SecondoInterface::GetTypeId( const AlgebraLevel level,
                             const string& name,
                             int& algebraId, int& typeId )
{
  bool ok = false;
  if ( server != 0 )
  {
    string line;
    iostream& iosock = server->GetSocketStream();
    iosock << "<GetTypeId>" << endl
           << level << " " << name << endl
           << "</GetTypeId>" << endl;
    getline( iosock, line );
    if ( line == "<GetTypeIdResponse>" )
    {
      iosock >> algebraId >> typeId >> skipline;
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
SecondoInterface::LookUpTypeExpr( const AlgebraLevel level,
                                  ListExpr type, string& name,
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
             << "(" << level << " " << line << ")" << endl
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
SecondoInterface::StartCommand()
{
}

void
SecondoInterface::FinishCommand( int& errorCode )
{
}

void
SecondoInterface::SetDebugLevel( const int level )
{
}

