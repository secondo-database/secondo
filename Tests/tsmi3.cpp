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

*/
#include <iostream>
#include <string>
using namespace std;
#include "SecondoSMI.h"
#include "SmiBDB.h"

void pause()
{
  char buf[80];
  cout << "<<< Press return to continue >>>" << endl;
  cin.getline( buf, sizeof(buf) );
}

void Initialize()
{
  SmiKeyedFile kf( SmiKey::String, true );
  if ( kf.Open( "SecondoCatalog" ) )
  {
    cout << "String KeyedFile created FileId=" << kf.GetFileId() << endl;
    cout << "KeyedFile name   =" << kf.GetName() << endl;
    cout << "KeyedFile context=" << kf.GetContext() << endl;
    cout << "(Returncodes: 1 = ok, 0 = error )" << endl;
    SmiKeyedFileIterator it;
    SmiRecord r;
    SmiKey key;
    cout << "Insert: " << kf.InsertRecord( SmiKey( "Anton" ), r ) << endl;
    cout << "Insert (Anton,Antonia): " << r.Write( "Antonia", 8 ) << endl;
    cout << "Insert: " << kf.InsertRecord( SmiKey( "Berta" ), r ) << endl;
    cout << "Insert (Berta,Bernhard): " << r.Write( "Bernhard", 9 ) << endl;
    cout << "Insert: " << kf.InsertRecord( SmiKey( "Caesar" ), r ) << endl;
    cout << "Insert (Caesar,Cecilie): " << r.Write( "Cecilie", 8 ) << endl;
    cout << "Close: " << kf.Close() << endl;
    cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
    cout << "Begin: " << SmiEnvironment::BeginTransaction() << endl;
  }
  else
  {
    cout << "KeyedFile open failed:" << SmiEnvironment::GetLastErrorCode() << endl;
  }
}

void TestKeyedFiles()
{
  char buffer[30];
  SmiKeyedFile kf( SmiKey::String, true );
  if ( kf.Open( "SecondoCatalog" ) )
  {
    cout << "String KeyedFile created FileId=" << kf.GetFileId() << endl;
    cout << "KeyedFile name   =" << kf.GetName() << endl;
    cout << "KeyedFile context=" << kf.GetContext() << endl;
    cout << "(Returncodes: 1 = ok, 0 = error )" << endl;
    SmiKeyedFileIterator it;
    SmiRecord r;
    SmiKey key;
    string cmd, ckey, cval;
    bool swUpdate = false;
    do
    {
      cout << endl << "cmd key val > ";
      cin >> cmd >> ckey >> cval;
      if ( cmd == "s" )
      {
        if ( swUpdate )
          cout << "Select first '"<< ckey << "': " << kf.SelectRecord( SmiKey( ckey ), r, SmiFile::Update ) << endl;
        else
          cout << "Select first '"<< ckey << "': " << kf.SelectRecord( SmiKey( ckey ), r ) << endl;
        cout << "RC=" << SmiEnvironment::GetLastErrorCode() << endl;
        cout << "Read " << r.Read( buffer, 20 ) << endl;
        cout << "buffer = " << buffer << endl;
        cout << "RC=" << SmiEnvironment::GetLastErrorCode() << endl;
      }
      else if ( cmd == "i" )
      {
        cout << "Insert: " << kf.InsertRecord( SmiKey( ckey ), r ) << endl;
        cout << "RC=" << SmiEnvironment::GetLastErrorCode() << endl;
        cout << "Insert (" << ckey << "," << cval << "): " << r.Write( cval.c_str(), cval.length()+1 ) << endl;
        cout << "RC=" << SmiEnvironment::GetLastErrorCode() << endl;
      }
      else if ( cmd == "u" )
      {
        cout << "Select first '" << ckey << "': " << kf.SelectRecord( SmiKey( ckey ), r, SmiFile::Update ) << endl;
        cout << "RC=" << SmiEnvironment::GetLastErrorCode() << endl;
        cout << "Write " << r.Write( cval.c_str(), cval.length()+1 ) << endl;
        cout << "Read " << r.Read( buffer, 20 ) << endl;
        cout << "buffer = " << buffer << endl;
        cout << "RC=" << SmiEnvironment::GetLastErrorCode() << endl;
//        swUpdate = true;
      }
      else if ( cmd == "c" )
      {
        cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
        cout << "Begin: " << SmiEnvironment::BeginTransaction() << endl;
        swUpdate = false;
      }
    }
    while ( cmd != "q" );

    cout << "Close: " << kf.Close() << endl;
    cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
    pause();
    cout << "Begin: " << SmiEnvironment::BeginTransaction() << endl;
  }
  else
  {
    cout << "KeyedFile open failed:" << SmiEnvironment::GetLastErrorCode() << endl;
  }
}

// --- Main

int main( int argc, char* argv[] )
{
  SmiError rc;
  bool ok;

  rc = SmiEnvironment::StartUp( SmiEnvironment::MultiUser,
                                "SecondoConfig.ini", cerr );
  cout << "StartUp rc=" << rc << endl;
  if ( rc == 1 )
  {
    ok = SmiEnvironment::OpenDatabase( "test" );
    if ( ok )
    {
      cout << "OpenDatabase test ok." << endl;
    }
    else
    {
      cout << "OpenDatabase test failed, try to create." << endl;
      ok = SmiEnvironment::CreateDatabase( "test" );
      if ( ok )
        cout << "CreateDatabase test ok." << endl;
      else
        cout << "CreateDatabase test failed." << endl;
    }
    pause();
    if ( ok )
    {
      if (argc > 1) Initialize();
      cout << "*** Test String Keyed Files ***" << endl;
      TestKeyedFiles();
      pause();
      cout << "*** Closing Database ***" << endl;
      if ( SmiEnvironment::CloseDatabase() )
        cout << "CloseDatabase test ok." << endl;
      else
        cout << "CloseDatabase test failed." << endl;
    }
  }
  rc = SmiEnvironment::ShutDown();
  cout << "ShutDown rc=" << rc << endl;
}

