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

Last change: Nov. 2004, M. Spiekermann 

*/

#include <iostream>
#include <string>
#include "SecondoSMI.h"

#include "StopWatch.h"
#include "LogMsg.h"
#include "Profiles.h"

using namespace std;

int REC_SIZE = 200;
int MAX_RECORDS = 1000000;

void pause()
{
  char buf[80];
  cout << "<<< Press return to continue >>>" << endl;
  cin.getline( buf, sizeof(buf) );
}

void TestRecordFiles( bool makeFixed )
{
  string filename = (makeFixed) ? "testfile_fix" : "testfile_var";
  SmiSize reclen = (makeFixed) ? 20 : 0;
  SmiRecordFile rf( makeFixed, reclen );
  if ( rf.Open( filename ) )
  {
    cout << "RecordFile successfully created/opened: " 
         << rf.GetFileId() << endl;
    cout << "RecordFile name   =" << rf.GetName() << endl;
    cout << "RecordFile context=" << rf.GetContext() << endl;
    cout << "(Returncodes: 1 = ok, 0 = error )" << endl;
    SmiRecord r;
    SmiRecordId rid;
    cout << "Append: " << rf.AppendRecord( rid, r ) << endl;
    cout << "Recno = " << rid << endl;
    cout << "Write " << r.Write( "Emilio", 7 ) << endl;
    cout << "Append: " << rf.AppendRecord( rid, r ) << endl;
    cout << "Recno = " << rid << endl;
    cout << "Write " << r.Write( "Juan", 5 ) << endl;
    cout << "Append: " << rf.AppendRecord( rid, r ) << endl;
    cout << "Recno = " << rid << endl;
    cout << "Write " << r.Write( "Xavieria", 9 ) << endl;
    rid = 1;
    cout << "Select Record 1: " 
         << rf.SelectRecord( rid, r, SmiFile::ReadOnly ) << endl;
    char buffer[30];
    cout << "Read " << r.Read( buffer, 20 ) << endl;
    cout << "buffer = " << buffer << endl;
    rid = 2;
    cout << "Select Record 2: " 
         << rf.SelectRecord( rid, r, SmiFile::Update ) << endl;
    cout << "Read " << r.Read( buffer, 20 ) << endl;
    cout << "buffer = " << buffer << endl;
    cout << "Write " << r.Write( " Carlos", 8, 4 ) << endl;
    cout << "Read " << r.Read( buffer, 20 ) << endl;
    cout << "buffer = " << buffer << endl;
    cout << "Write " << r.Write( "Don", 3 ) << endl;
    cout << "Read " << r.Read( buffer, 20 ) << endl;
    cout << "buffer = " << buffer << endl;
    cout << "Truncate " << r.Truncate( 3 ) << endl;
    int len = r.Read( buffer, 20 );
    cout << "Read " << len << endl;
    buffer[len] = '\0';
    cout << "buffer = " << buffer << endl;
    rid = 3;
    cout << "Select Record 3: " 
         << rf.SelectRecord( rid, r, SmiFile::ReadOnly ) << endl;
    cout << "Read " << r.Read( buffer, 20 ) << endl;
    cout << "buffer = " << buffer << endl;
    SmiRecordFileIterator it;
    cout << "SelectAll: " << rf.SelectAll( it, SmiFile::ReadOnly ) << endl;
    while ( it.Next( r ) )
    {
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "buffer = " << buffer << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;
    cout << "Delete record: " << rf.DeleteRecord( 2 ) << endl;
    cout << "SelectAll: " << rf.SelectAll( it, SmiFile::ReadOnly ) << endl;
    while ( it.Next( r ) )
    {
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "buffer = " << buffer << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;
    cout << "Close: " << rf.Close() << endl;
    cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
    pause();
    cout << "Begin: " << SmiEnvironment::BeginTransaction() << endl;
    cout << "Drop: " << rf.Drop() << endl;
    cout << "Abort: " << SmiEnvironment::AbortTransaction() << endl;
    pause();
    cout << "Begin: " << SmiEnvironment::BeginTransaction() << endl;
  }
  else
  {
    cout << "RecordFile create failed:" 
         << SmiEnvironment::GetLastErrorCode() << endl;
  }
}

void TestFileScan( )
{
  string filename = "testfile_fix_prefIter";
  SmiSize reclen = REC_SIZE;
  SmiRecordFile rf( false, reclen );
  char dummyRecIn[REC_SIZE+1];
  char dummyRecOut[REC_SIZE+1];
  
  // initialize dummyRec
  char c = 'a';
  for (int k=0; k < REC_SIZE; k++) {
    dummyRecIn[k] = c;
    c++;
    if ( c > 'z' ) {
      c = 'a';
    }
  }
  dummyRecIn[REC_SIZE] = 0; // terminate char vector
  //cout << "dummyRecIn: " << endl
  //     << dummyRecIn << endl;
  

  if ( !rf.Open( filename ) )
  {
    cout << "RecordFile create failed:" 
         << SmiEnvironment::GetLastErrorCode() << endl;
    return;
  }

  cout << "RecordFile successfully created/opened: " << rf.GetFileId() << endl;
  cout << "RecordFile name   =" << rf.GetName() << endl;
  cout << "RecordFile context=" << rf.GetContext() << endl;

  SmiRecord r;
  SmiRecordId rid;
  
  cout << "Creating a Record file with " << MAX_RECORDS 
       << " records of size " << REC_SIZE << " bytes." << endl;

  StopWatch appendTime;
  for (int i=0; i < MAX_RECORDS; i++) { 
    rf.AppendRecord( rid, r );
    //cout << "Recno = " << rid << endl;
    r.Write( dummyRecIn, REC_SIZE ); 
  }
  cout << appendTime.diffTimes() << endl;  

  /*
  for (int j=1; j < 4; j++) {
    SmiRecordFileIterator it;
    StopWatch scan1Time;
    cout << j << ". Scan using SmiRecordFileIterator: " 
         << rf.SelectAll( it, SmiFile::ReadOnly ) << endl;

    while ( it.Next( r ) )
    {
      //r.Read( dummyRecOut, reclen );
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;
    cout << scan1Time.diffTimes() << endl;
  }
  */

  for (int j=1; j < 4; j++) {
    StopWatch scan2Time;
    cout << j << ". Scan using PreFetchingIterator: " << endl;
    PrefetchingIterator* it2 = rf.SelectAllPrefetched();

    while ( it2->Next() )
    {
      //it2->ReadCurrentData( dummyRecOut, reclen );
    }
    cout << "EndOfScan" << endl;
    cout << scan2Time.diffTimes() << endl;
    delete it2;
  }

 
  SmiKeyedFile kf( SmiKey::Integer, true );
  assert( kf.Create() ); 
  SmiKey key;
  cout << "Integer KeyedFile created FileId=" << kf.GetFileId() << endl;
  cout << "KeyedFile name   =" << kf.GetName() << endl;
  cout << "KeyedFile context=" << kf.GetContext() << endl;

  //cout << "Insert: " << kf.InsertRecord( SmiKey( 4711l ), r ) << endl;
  //cout << "Insert (4711,Emilio1): " << r.Write( "Emilio1", 8 ) << endl;

  cout << "Creating a Record file with " << MAX_RECORDS 
       << " records of size " << REC_SIZE << " bytes." << endl;

  StopWatch insertTime;
  for (long int i=0; i < MAX_RECORDS; i++) { 
    kf.InsertRecord( SmiKey(i), r );
    r.Write( dummyRecIn, REC_SIZE ); 
  }
  cout << insertTime.diffTimes() << endl;  

  for (int j=1; j < 4; j++) {
    StopWatch scan2Time;
    cout << j << ". Scan using PreFetchingIterator: " << endl;
    PrefetchingIterator* it2 = kf.SelectAllPrefetched();

    while ( it2->Next() )
    {
      //it2->ReadCurrentData( dummyRecOut, reclen );
    }
    cout << "EndOfScan" << endl;
    cout << scan2Time.diffTimes() << endl;
    delete it2;
  }


  //cout << "Closing and dropping the RecordFile ..." << endl;
  //cout << "Close: " << rf.Close() << endl;
  //cout << "Drop: " << rf.Drop() << endl;
}

/*
2 Keyed Files

*/


void TestKeyedFiles( bool makeUnique )
{
  char buffer[30];
  SmiKeyedFile kf( SmiKey::String, makeUnique );
  if ( kf.Create() )
  {
    cout << "String KeyedFile created FileId=" << kf.GetFileId() << endl;
    cout << "KeyedFile name   =" << kf.GetName() << endl;
    cout << "KeyedFile context=" << kf.GetContext() << endl;
    cout << "(Returncodes: 1 = ok, 0 = error )" << endl;
    SmiKeyedFileIterator it;
    SmiRecord r;
    SmiKey key;
    cout << "Insert: " << kf.InsertRecord( SmiKey( "Dora" ), r ) << endl;
    cout << "Insert (Dora,Emilio1): " << r.Write( "Emilio1", 8 ) << endl;
    if ( kf.InsertRecord( SmiKey( "Dora" ), r ) )
    {
      cout << "Insert (Dora,Emilio2): " << r.Write( "Emilio2", 8 ) << endl;
    }
    else
    {
      cout << "Second insert for 'Dora' failed" << endl;
    }
    cout << "Insert: " << kf.InsertRecord( SmiKey( "Anton" ), r ) << endl;
    cout << "Insert (Anton,Juan): " << r.Write( "Juan", 5 ) << endl;
    cout << "Insert: " << kf.InsertRecord( SmiKey( "Berta" ), r ) << endl;
    cout << "Insert (Berta,Xavieria): " << r.Write( "Xavieria", 9 ) << endl;
    cout << "Insert: " << kf.InsertRecord( SmiKey( "Hugo" ), r ) << endl;
    cout << "Insert (Hugo,Gesine): " << r.Write( "Gesine", 7 ) << endl;

    cout << "Select first 'Dora': " 
         << kf.SelectRecord( SmiKey( "Dora" ), r, SmiFile::Update ) << endl;
    cout << "Read " << r.Read( buffer, 20 ) << endl;
    cout << "buffer = " << buffer << endl;
    cout << "Write " << r.Write( " Canneloni", 11, 7 ) << endl;
    cout << "Read " << r.Read( buffer, 20 ) << endl;
    cout << "buffer = " << buffer << endl;

    cout << "Select all 'Dora': " 
         << kf.SelectRecord( SmiKey( "Dora" ), it, SmiFile::ReadOnly ) << endl;
    string keyval;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectAll Keyed: " << kf.SelectAll( it, SmiFile::ReadOnly, true )
         << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectRange Keyed: " 
         << kf.SelectRange( SmiKey( "Berta" ), 
                            SmiKey( "Dora" ), it, SmiFile::ReadOnly ) 
         << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectLeftRange Keyed: " 
         << kf.SelectLeftRange( SmiKey( "Berta" ), it, SmiFile::ReadOnly ) 
         << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectRightRange Keyed: " 
         << kf.SelectRightRange( SmiKey( "Dora" ), it, SmiFile::ReadOnly ) 
         << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "Delete 'Dora' " << kf.DeleteRecord( SmiKey( "Dora" ) ) << endl;
    cout << "SelectAll Keyed: " << kf.SelectAll( it, SmiFile::ReadOnly, true )
         << endl;
    bool first = true;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
      if ( first )
      {
        cout << "DeleteCurrent: " << it.DeleteCurrent() << endl;
        first = false;
      }
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Restart: " << it.Restart() << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "Close: " << kf.Close() << endl;
    cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
    pause();
    cout << "Begin: " << SmiEnvironment::BeginTransaction() << endl;
    cout << "Drop: " << kf.Drop() << endl;
    cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
    pause();
    cout << "Begin: " << SmiEnvironment::BeginTransaction() << endl;
  }
  else
  {
    cout << "KeyedFile create failed:" 
         << SmiEnvironment::GetLastErrorCode() << endl;
  }
}

void TestIntegerKeyedFiles( bool makeUnique )
{
  char buffer[30];
  SmiKeyedFile kf( SmiKey::Integer, makeUnique );
  if ( kf.Create() )
  {
    cout << "Integer KeyedFile created FileId=" << kf.GetFileId() << endl;
    cout << "KeyedFile name   =" << kf.GetName() << endl;
    cout << "KeyedFile context=" << kf.GetContext() << endl;
    cout << "(Returncodes: 1 = ok, 0 = error )" << endl;
    SmiKeyedFileIterator it;
    SmiRecord r;
    SmiKey key;
    cout << "Insert: " << kf.InsertRecord( SmiKey( 4711l ), r ) << endl;
    cout << "Insert (4711,Emilio1): " << r.Write( "Emilio1", 8 ) << endl;
    if ( kf.InsertRecord( SmiKey( 4711l ), r ) )
    {
      cout << "Insert (4711,Emilio2): " << r.Write( "Emilio2", 8 ) << endl;
    }
    else
    {
      cout << "Second insert for 'Dora' failed" << endl;
    }
    cout << "Insert: " << kf.InsertRecord( SmiKey( 1248l ), r ) << endl;
    cout << "Insert (1248,Juan): " << r.Write( "Juan", 5 ) << endl;
    cout << "Insert: " << kf.InsertRecord( SmiKey( 3505l ), r ) << endl;
    cout << "Insert (3505,Xavieria): " << r.Write( "Xavieria", 9 ) << endl;
    cout << "Insert: " << kf.InsertRecord( SmiKey( 7447l ), r ) << endl;
    cout << "Insert (7447,Gesine): " << r.Write( "Gesine", 7 ) << endl;

    cout << "Select first '4711': " 
         << kf.SelectRecord( SmiKey( 4711l ), r, SmiFile::ReadOnly ) << endl;
    cout << "Read " << r.Read( buffer, 20 ) << endl;
    cout << "buffer = " << buffer << endl;

    cout << "Select all '4711': " 
         << kf.SelectRecord( SmiKey( 4711l ), it, SmiFile::ReadOnly ) << endl;
    long keyval;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectAll Keyed: " << kf.SelectAll( it, SmiFile::ReadOnly, true )
         << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectRange Keyed: " 
         << kf.SelectRange( SmiKey( 3505l ), 
                             SmiKey( 4711l ), it, SmiFile::ReadOnly )
         << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectLeftRange Keyed: " << 
            kf.SelectLeftRange( SmiKey( 3505l ), it, SmiFile::ReadOnly ) 
         << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectRightRange Keyed: " << 
            kf.SelectRightRange( SmiKey( 4711l ), it, SmiFile::ReadOnly ) 
         << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "Delete 'Dora' " << kf.DeleteRecord( SmiKey( 4711l ) ) << endl;
    cout << "SelectAll Keyed: " << kf.SelectAll( it, SmiFile::ReadOnly, true )
         << endl;
    bool first = true;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
      if ( first )
      {
        cout << "DeleteCurrent: " << it.DeleteCurrent() << endl;
        first = false;
      }
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Restart: " << it.Restart() << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "Close: " << kf.Close() << endl;
    cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
    pause();
    cout << "Begin: " << SmiEnvironment::BeginTransaction() << endl;
    cout << "Drop: " << kf.Drop() << endl;
    cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
    pause();
    cout << "Begin: " << SmiEnvironment::BeginTransaction() << endl;
  }
  else
  {
    cout << "KeyedFile create failed:" 
         << SmiEnvironment::GetLastErrorCode() << endl;
  }
}

void TestFloatKeyedFiles( bool makeUnique )
{
  char buffer[30];
  SmiKeyedFile kf( SmiKey::Float, makeUnique );
  if ( kf.Create() )
  {
    cout << "Float KeyedFile created FileId=" << kf.GetFileId() << endl;
    cout << "KeyedFile name   =" << kf.GetName() << endl;
    cout << "KeyedFile context=" << kf.GetContext() << endl;
    pause();
    SmiKeyedFileIterator it;
    SmiRecord r;
    SmiKey key;
    cout << "Insert: " << kf.InsertRecord( SmiKey( 47.11 ), r ) << endl;
    cout << "Insert (47.11,Emilio1): " << r.Write( "Emilio1", 8 ) << endl;
    if ( kf.InsertRecord( SmiKey( 47.11 ), r ) )
    {
      cout << "Insert (47.11,Emilio2): " << r.Write( "Emilio2", 8 ) << endl;
    }
    else
    {
      cout << "Second insert for 'Dora' failed" << endl;
    }
    cout << "Insert: " << kf.InsertRecord( SmiKey( 12.48 ), r ) << endl;
    cout << "Insert (12.48,Juan): " << r.Write( "Juan", 5 ) << endl;
    cout << "Insert: " << kf.InsertRecord( SmiKey( 35.05 ), r ) << endl;
    cout << "Insert (35.05,Xavieria): " << r.Write( "Xavieria", 9 ) << endl;
    cout << "Insert: " << kf.InsertRecord( SmiKey( 74.47 ), r ) << endl;
    cout << "Insert (74.47,Gesine): " << r.Write( "Gesine", 7 ) << endl;

    cout << "Select first '47.11': " 
         << kf.SelectRecord( SmiKey( 47.11 ), r, SmiFile::ReadOnly ) << endl;
    cout << "Read " << r.Read( buffer, 20 ) << endl;
    cout << "buffer = " << buffer << endl;

    cout << "Select all '47.11': " 
         << kf.SelectRecord( SmiKey( 47.11 ), it, SmiFile::ReadOnly ) << endl;
    double keyval;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectAll Keyed: " 
         << kf.SelectAll( it, SmiFile::ReadOnly, true ) << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectRange Keyed: " << 
            kf.SelectRange( SmiKey( 35.05 ), 
                            SmiKey( 47.11 ), it, SmiFile::ReadOnly ) << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectLeftRange Keyed: " << 
            kf.SelectLeftRange( SmiKey( 35.05 ), it, SmiFile::ReadOnly ) 
         << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectRightRange Keyed: " << 
            kf.SelectRightRange( SmiKey( 47.11 ), it, SmiFile::ReadOnly )
         << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "Delete 'Dora' " << kf.DeleteRecord( SmiKey( 47.11 ) ) << endl;
    cout << "SelectAll Keyed: " << kf.SelectAll( it, SmiFile::ReadOnly, true )
         << endl;
    bool first = true;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
      if ( first )
      {
        cout << "DeleteCurrent: " << it.DeleteCurrent() << endl;
        first = false;
      }
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Restart: " << it.Restart() << endl;
    while ( it.Next( key, r ) )
    {
      cout << "GetKey " << key.GetKey( keyval ) << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "Close: " << kf.Close() << endl;
    cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
    pause();
    cout << "Begin: " << SmiEnvironment::BeginTransaction() << endl;
    cout << "Drop: " << kf.Drop() << endl;
    cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
    pause();
    cout << "Begin: " << SmiEnvironment::BeginTransaction() << endl;
  }
  else
  {
    cout << "KeyedFile create failed:" 
         << SmiEnvironment::GetLastErrorCode() << endl;
  }
}

struct Component
{
  double ik;
};

void EasyMapping( const void*    inKey,
                  const SmiSize  inLen,
                        void*    outKey,
                  const SmiSize  maxOutLen,
                        SmiSize& outLen,
                  const bool     doMap )
{
  Component* cp;
  if ( doMap )
  {
    cp = (Component*) inKey;
    SmiKey::Map( cp->ik, outKey );
    outLen = sizeof(double);
  }
  else
  {
    cp = (Component*) outKey;
    SmiKey::Unmap( inKey, cp->ik );
    outLen = sizeof(Component);
  }
}

/*
3 Keyed Files

*/

void TestCompKeyedFiles( bool makeUnique )
{
  cout << "Interface has been changed!. Code must be revised." << endl;

#ifdef COMPILE_THIS_CODE
  Component comp1 = { 47.11 };
  Component comp2 = { -12.48 };
  Component comp3 = { 35.05 };
  Component comp4 = { 74.47 };
  SmiKey key1( comp1 );
  SmiKey key2( comp2 );
  SmiKey key3( comp3 );
  SmiKey key4( comp4 );
  
  char buffer[30];
  SmiKeyedFile kf( SmiKey::Composite, makeUnique );
  if ( kf.Create() )
  {
    cout << "Composite KeyedFile created, FileId=" << kf.GetFileId() << endl;
    cout << "KeyedFile name   =" << kf.GetName() << endl;
    cout << "KeyedFile context=" << kf.GetContext() << endl;
    cout << "(Returncodes: 1 = ok, 0 = error )" << endl;
    SmiKeyedFileIterator it;
    SmiRecord r;
    SmiKey key( EasyMapping ); // !!! Important !!!
    cout << "Insert: " << kf.InsertRecord( key1, r ) << endl;
    cout << "Insert (47.11,Emilio1): " << r.Write( "Emilio1", 8 ) << endl;
    if ( kf.InsertRecord( key1, r ) )
    {
      cout << "Insert (47.11,Emilio2): " << r.Write( "Emilio2", 8 ) << endl;
    }
    else
    {
      cout << "Second insert for '47.11' failed" << endl;
    }
    cout << "Insert: " << kf.InsertRecord( key2, r ) << endl;
    cout << "Insert (12.48,Juan): " << r.Write( "Juan", 5 ) << endl;
    cout << "Insert: " << kf.InsertRecord( key3, r ) << endl;
    cout << "Insert (35.05,Xavieria): " << r.Write( "Xavieria", 9 ) << endl;
    cout << "Insert: " << kf.InsertRecord( key4, r ) << endl;
    cout << "Insert (74.47,Gesine): " << r.Write( "Gesine", 7 ) << endl;

    cout << "Select FIRST '47.11': " 
         << kf.SelectRecord( key1, r, SmiFile::ReadOnly ) << endl;
    cout << "Read " << r.Read( buffer, 20 ) << endl;
    cout << "buffer = " << buffer << endl;

    cout << "Select ALL '47.11': " 
         << kf.SelectRecord( key1, it, SmiFile::ReadOnly ) << endl;
    Component keyval;
    SmiSize   complen = sizeof(Component);
    SmiSize   reallen;
    while ( it.Next( key, r ) )
    {
      cout << "KeyType " << key.GetType() << endl;
      cout << "GetKey " << key.GetKey( (void*) &keyval, complen, reallen ) 
           << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval.ik << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectAll Keyed: " 
         << kf.SelectAll( it, SmiFile::ReadOnly, true ) << endl;
    while ( it.Next( key, r ) )
    {
      cout << "KeyType " << key.GetType() << endl;
      cout << "GetKey " << key.GetKey( (void*) &keyval, complen, reallen ) 
           << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval.ik << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectRange Keyed: " << 
            kf.SelectRange( key3, key1, it, SmiFile::ReadOnly ) << endl;
    while ( it.Next( key, r ) )
    {
      cout << "KeyType " << key.GetType() << endl;
      cout << "GetKey " << key.GetKey( (void*) &keyval, complen, reallen ) 
           << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval.ik << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectLeftRange Keyed: " << 
            kf.SelectLeftRange( key3, it, SmiFile::ReadOnly ) << endl;
    while ( it.Next( key, r ) )
    {
      cout << "KeyType " << key.GetType() << endl;
      cout << "GetKey " << key.GetKey( (void*) &keyval, complen, reallen ) 
           << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval.ik << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "SelectRightRange Keyed: " << 
            kf.SelectRightRange( key1, it, SmiFile::ReadOnly ) << endl;
    while ( it.Next( key, r ) )
    {
      cout << "KeyType " << key.GetType() << endl;
      cout << "GetKey " << key.GetKey( (void*) &keyval, complen, reallen ) 
           << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval.ik << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "Delete 'Dora' " << kf.DeleteRecord( key1 ) << endl;
    cout << "SelectAll Keyed: " << kf.SelectAll( it, SmiFile::ReadOnly, true )
         << endl;
    bool first = true;
    while ( it.Next( key, r ) )
    {
      cout << "KeyType " << key.GetType() << endl;
      cout << "GetKey " << key.GetKey( (void*) &keyval, complen, reallen ) 
           << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval.ik << "," << buffer << ")" << endl;
      if ( first )
      {
        cout << "DeleteCurrent: " << it.DeleteCurrent() << endl;
        first = false;
      }
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Restart: " << it.Restart() << endl;
    while ( it.Next( key, r ) )
    {
      cout << "KeyType " << key.GetType() << endl;
      cout << "GetKey " << key.GetKey( (void*) &keyval, complen, reallen ) 
           << endl;
      cout << "Read " << r.Read( buffer, 20 ) << endl;
      cout << "(" << keyval.ik << "," << buffer << ")" << endl;
    }
    cout << "EndOfScan=" << it.EndOfScan() << endl;
    cout << "Finish cursor it: " << it.Finish() << endl;

    cout << "Close: " << kf.Close() << endl;
    cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
    pause();
    cout << "Begin: " << SmiEnvironment::BeginTransaction() << endl;
    cout << "Drop: " << kf.Drop() << endl;
    cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
    pause();
    cout << "Begin: " << SmiEnvironment::BeginTransaction() << endl;
  }
  else
  {
    cout << "KeyedFile create failed:" << SmiEnvironment::GetLastErrorCode() 
         << endl;
  }
#endif
}

// --- Main

int main( int argc, char* argv[] )
{
  if ( argc == 1 ) {
    cout << "Usage: " << argv[0] 
         << " -mode [REC_SIZE=" << REC_SIZE
         << " MAX_RECORDS=" << MAX_RECORDS << "]" << endl 
         << "options are listed below:" << endl 
         << "  -1:  Test SmiRecordFiles (fixed record length)" << endl 
         << "  -2:  Test SmiRecordFiles (variable record length)" << endl 
         << "  -3:  Test SmiKeyedFiles (String unique keys)" << endl 
         << "  -4:  Test SmiKeyedFiles (String duplicate keys)" << endl 
         << "  -5:  Test SmiKeyedFiles (Integer duplicate keys)" << endl 
         << "  -6:  Test SmiKeyedFiles (Float duplicate keys)" << endl 
         << "  -7:  Test SmiKeyedFiles (Composite duplicate keys)" << endl
         << "  -8 [REC_SIZE MAX_RECORDS]:  " 
         << "Performance tests for scanning RecordFiles" << endl;
    exit(1); 
  }

  const int mode = -1 * atoi(argv[1]);
  
  if (mode == 8) {
    if (argc == 4) {
      REC_SIZE = atoi(argv[2]);
      MAX_RECORDS = atoi(argv[3]);
    } 
    assert(REC_SIZE < (4096 - 100));
  }



  const char* paramFile="SecondoConfig.ini";

  // initialize runtime flags
  string logMsgList = SmiProfile::GetParameter( "Environment", "RTFlags",
                                                           "", paramFile );    
  RTFlag::initByString(logMsgList);
  RTFlag::showActiveFlags(cout);

  SmiError rc;
  bool ok;

  if ( RTFlag::isActive("SMI:NoTransactions") ) {
    rc = SmiEnvironment::StartUp( SmiEnvironment::SingleUserSimple,
                                  paramFile, cerr );
  } else {
    rc = SmiEnvironment::StartUp( SmiEnvironment::SingleUser,
                                  paramFile, cerr );
  }

  cout << "StartUp rc=" << rc << endl;
  if ( rc == 1 )
  {
    string dbname;
    cout << "*** Start list of databases ***" << endl;
    SmiEnvironment::ListDatabases( dbname );
    cout << dbname << endl;
    cout << "*** End list of databases ***" << endl;
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
      if ( SmiEnvironment::CloseDatabase() )
        cout << "CloseDatabase test ok." << endl;
      else
        cout << "CloseDatabase test failed." << endl;
      if ( ok = SmiEnvironment::OpenDatabase( "test" ) )
        cout << "OpenDatabase test ok." << endl;
      else
        cout << "OpenDatabase test failed." << endl;
    }
    pause();
    if ( ok )
    {
      cout << "Begin Transaction: " 
           << SmiEnvironment::BeginTransaction() << endl;

      switch (mode) {
      case 1 : {
      cout << "*** Test Record Files with fixed length records ***" << endl;
      TestRecordFiles( true );
      break;
      }
      case 2 : {
      cout << "*** Test Record Files with variable length records ***" << endl;
      TestRecordFiles( false );
      break;
      }
      case 3 : {
      cout << "*** Test String Keyed Files (Unique keys) ***" << endl;
      TestKeyedFiles( true );
      break;
      }
      case 4 : {
      cout << "*** Test String Keyed Files (Duplicate keys) ***" << endl;
      TestKeyedFiles( false );
      break;
      }
      case 5 : {
      cout << "*** Test Integer Keyed Files (Duplicate keys) ***" << endl;
      TestIntegerKeyedFiles( false );
      break;
      }
      case 6 : {
      cout << "*** Test Float Keyed Files (Duplicate keys) ***" << endl;
      TestFloatKeyedFiles( false );
      break; 
      }
      case 7 : {
      cout << "*** Test Composite Keyed Files (Duplicate keys) ***" << endl;
      TestCompKeyedFiles( false );
      break;
      }
      case 8 : {
      cout << "*** Performance test of scanning RecordFiles ***" << endl;
      TestFileScan();
      break;
      }

      default: {
      cout << "Error: Unkown mode!" << endl;
      }
      }
 
      cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
      cout << "*** Closing Database ***" << endl;
      if ( SmiEnvironment::CloseDatabase() )
        cout << "CloseDatabase test ok." << endl;
      else
        cout << "CloseDatabase test failed." << endl;
      pause();
      if ( SmiEnvironment::EraseDatabase( "test" ) )
        cout << "EraseDatabase test ok." << endl;
      else
        cout << "EraseDatabase test failed." << endl;
    }
  }
  rc = SmiEnvironment::ShutDown();
  cout << "ShutDown rc=" << rc << endl;
}

