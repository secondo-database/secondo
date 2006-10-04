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

Oct 2006, M. Spiekermann. Many changes in order to get it work again.

*/


#include <iostream>
#include <string>

#include "SecondoSMI.h"
#include "SmiBDB.h"

using namespace std;

void Pause()
{
  char buf[80];
  cout << "<<< Press return to continue >>>" << endl;
  cin.getline( buf, sizeof(buf) );
}


void ShowOptions(const string& prog)
{
  cout << "Usage: " << prog << " [-ch]" << endl;
  cout << endl;
  cout << "-c     Create some records" << endl;
  cout << "-h     This help message" << endl;
  cout << endl;
} 

void ShowSmiError()    
{ 
    string msg=""; 
    SmiEnvironment::GetLastErrorCode(msg);
    cerr << msg << endl;
}   


bool CheckSmiError()    
{ 
  SmiError err = SmiEnvironment::GetLastErrorCode();
  
  if (err != E_SMI_OK) {
   
    ShowSmiError();
    return true;
  }  
  return false;
}   



typedef enum {begin, commit} TMode;

void Transaction(TMode m)
{   
    bool rc=false;
    string modeStr="Commit";
    
    if (m == begin) {
      modeStr = "Begin";
      rc = SmiEnvironment::BeginTransaction();
    }  
    else
    {
      rc = SmiEnvironment::CommitTransaction();
    }    

    if (rc == false)
      ShowSmiError();
}    


void InsertRec(SmiKeyedFile& kf, const string& key, const string& val)
{ 
 
  cout << "Create new record for key " << key << endl;
  SmiRecord r; 
  bool rc = kf.InsertRecord( SmiKey( key ), r );

  if (rc == false)
  { 
    ShowSmiError();
    return;
  }  
  
  cout << "Insert value " << val << endl;
  rc = r.Write( val.c_str(), val.length()+1 );
  
  if (rc == false)
    ShowSmiError();
}


void SelectRec(SmiKeyedFile& kf, SmiRecord& r, const string& key, bool update)
{ 
  cout << "Select Record with key = '" << key << "'" << endl;
  bool rc = false;

  if (update)
  { 
    // select for update
    rc = kf.SelectRecord( SmiKey( key ), r, SmiFile::Update );
  }  
  else
  { 
    rc = kf.SelectRecord( SmiKey( key ), r );
  }  
   
  if (rc == false)
    ShowSmiError();
}  


void ReadRec(SmiRecord& r)
{ 
  cout << "Reading record [bytes = ";
  char buffer [30];
  
  int bytes = r.Read( buffer, 20 );
  cout << bytes << "]" << endl;
  
  if ( CheckSmiError() )
    return;  
  
  cout << "buffer = " << buffer << endl;
}

void Initialize()
{
  SmiKeyedFile kf( SmiKey::String, true );
  if ( kf.Open( "SecondoCatalog" ) )
  {
    Transaction(begin);
    cout << "String KeyedFile created FileId=" << kf.GetFileId() << endl;
    cout << "KeyedFile name   =" << kf.GetName() << endl;
    cout << "KeyedFile context=" << kf.GetContext() << endl;
    
    InsertRec(kf, "Anton", "Antonia");
    InsertRec(kf, "Berta", "Bernhard");
    InsertRec(kf, "Ceasar", "Cecilie");
    
    Transaction(commit);
    cout << "SmiKeyedFile:.Close: rc=" << kf.Close() << endl;
  }
  else
  {
    cerr << "SmiKeyedFile::Open failed:" << endl;
    CheckSmiError();
  }
}

void TestKeyedFiles()
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
    
    string cmd="", ckey="", cval="";
    
    Transaction(begin);
    do
    {
      cout << endl 
           << "Cmd: [(s)elect, (i)nsert, (u)pdate, (c)ommit, (q)uit]: ";
      cin >> cmd;

      if ( cmd == "q" )
      {
        cout << "Stop reading commands." << endl; 
      } 

      if ( cmd == "s" || cmd == "i" || cmd == "u")
      { 
        cout << "Key  : "; 
        cin >> ckey;
      }  

      if ( cmd == "i" || cmd == "u")
      { 
        cout << "Value: ";
        cin >> cval;
      }  

      if ( cmd == "s" )
      {
        SelectRec( kf, r, ckey, false );
        ReadRec(r);
      }
      else if ( cmd == "i" )
      {
        InsertRec(kf, ckey, cval);
      }
      else if ( cmd == "u" )
      {
        SelectRec( kf, r, ckey, true);
        cout << "Write Record [bytes = " 
             << r.Write( cval.c_str(), cval.length()+1 ) << "]";
        CheckSmiError();
      }
      else if ( cmd == "c" )
      {
        Transaction(commit);
        Transaction(begin);
      }
    }
    while ( cmd != "q" );

    cout << "Close: " << kf.Close() << endl;
    Transaction(commit);
  }
  else
  {
    cout << "KeyedFile open failed:" << endl;
    CheckSmiError();
  }
}

// --- Main

int main( int argc, char* argv[] )
{

  bool createRecords = false; 
   
  if ( argc <= 2)
  {
   string opt(argv[1]);
   if (opt == "-?" || opt == "-h") {
     ShowOptions(argv[0]);
     exit(0);
   }  
   if ( opt == "-c")
     createRecords = true;
   else
   { 
     cerr << "Unrecognized option!" << endl;
     ShowOptions(argv[0]);
     exit(1); 
   }  
  }  

  if (argc > 2) {
    ShowOptions(argv[0]);
    exit(1);
  }  
   
  cout << "(Returncodes: 1 = ok, 0 = error )" << endl;
  
  bool ok = false;
  SmiError rc = SmiEnvironment::StartUp( SmiEnvironment::MultiUser,
                                         "SecondoConfig.ini", cerr );
  cout << "StartUp rc=" << rc << endl;
  if ( rc == 1 )
  {
    string dbName="test";
    ok = SmiEnvironment::OpenDatabase( dbName );
    cout << "OpenDatabase " << dbName;
    if ( ok )
    {
      cout << " ok." << endl;
    }
    else
    {
      cout << " failed, try to create." << endl;
      ok = SmiEnvironment::CreateDatabase( dbName );
      cout << "CreateDatabase " << dbName;
      if ( ok )
        cout << " ok." << endl;
      else
        cout << " failed." << endl;
    }
    Pause();
    if ( ok )
    {
      if (createRecords) 
        Initialize();

      cout << "*** Test String Keyed Files ***" << endl;
      TestKeyedFiles();
      Pause();
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

