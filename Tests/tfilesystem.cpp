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
#include <fstream>
#include <iostream>
#include <string>

#include "SecondoConfig.h"
#include "FileSystem.h"

using namespace std;

void pause()
{
  char buf[80];
  cout << "<<< Press return to continue >>>" << endl;
  cin.getline( buf, sizeof(buf) );
}

static string folder[4] = { "test", 
                            string("test") + PATH_SLASH + "a",
                            string("test") + PATH_SLASH + "b",
                            string("test") + PATH_SLASH + "b" + PATH_SLASH + "c"
                          };

bool initialize()
{
  bool ok = true;
  for (int j = 0; j < 4; j++)
  {
    ok = ok && FileSystem::CreateFolder( folder[j] );
  }
  if ( ok )
  {
    FileSystem::SetCurrentFolder( "test" );
    ofstream out( "test.dat" );
    out << "This is a test file" << endl;
    out.close();
    FileSystem::CopyFile( "test.dat", string("a")+PATH_SLASH+"atest1.dat" ); 
    FileSystem::CopyFile( "test.dat", string("a")+PATH_SLASH+"atest2.dat" ); 
    FileSystem::CopyFile( "test.dat", string("a")+PATH_SLASH+"atest3.dat" ); 
    FileSystem::CopyFile( "test.dat", string("b")+PATH_SLASH+"btest.dat" ); 
    FileSystem::CopyFile( "test.dat", string("b")+PATH_SLASH+"c"+PATH_SLASH+"ctest.dat" ); 
  }
  return ok;
}

int main()
{
  if ( !initialize() )
  {
    cout << "Initialization failed!" << endl;
    return (0);
  }

  cout << "Current folder: " << FileSystem::GetCurrentFolder() << endl;
  if ( FileSystem::FileOrFolderExists( "xyz" ) )
    cout << "Folder xyz exists" << endl;
  else
    cout << "Folder xyz does not exist" << endl;
  if ( FileSystem::CreateFolder( "xyz" ) )
  {
    cout << "CreateFolder xyz ok" << endl;
    if ( FileSystem::CreateFolder( string("xyz")+PATH_SLASH+"abc" ) )
      cout << "CreateFolder xyz/abc ok" << endl;
    if ( FileSystem::CopyFile( "test.dat", string("xyz")+PATH_SLASH+"abc"+PATH_SLASH+"test.dat" ) )
      cout << "Copy test.dat -> xyz/abc/test.dat ok" << endl;
    else
      cout << "Copy test.dat -> xyz/abc/test.dat not ok" << endl;
  }
  else
    cout << "CreateFolder xyz failed" << endl;
  pause();
  if ( FileSystem::CopyFile( "xyz", "uvw" ) )
    cout << "Copy xyz -> uvw ok" << endl;
  else
    cout << "Copy xyz -> uvw not ok" << endl;
  
  if ( FileSystem::RenameFileOrFolder( string("xyz")+PATH_SLASH+"abc"+PATH_SLASH+"test.dat",
                                       string("xyz")+PATH_SLASH+"abc"+PATH_SLASH+"test2.dat" ) )
    cout << "Rename File test.dat -> test2.dat ok" << endl;
  else
    cout << "Rename File test.dat -> test2.dat not ok" << endl;

  if ( FileSystem::FileOrFolderExists( string("xyz")+PATH_SLASH+"abc"+PATH_SLASH+"test2.dat" ) )
    cout << "File xyz/abc/test2.dat exists" << endl;
  else
    cout << "File xyz/abc/test2.dat does not exist" << endl;
  cout << "Attribs of b/btest.dat " << FileSystem::GetFileAttributes( string("b")+PATH_SLASH+"btest.dat" ) << endl;
  FileAttributes attr = FileSystem::GetFileAttributes( string("a")+PATH_SLASH+"atest1.dat" );
  cout << "Attribs of a/atest1.dat " << attr << endl;
  cout << "Set Attrib attr & 2 " << FileSystem::SetFileAttributes( string("a")+PATH_SLASH+"atest1.dat", attr | 2 ) << endl;
  cout << "Attribs of a\\atest1.dat " << FileSystem::GetFileAttributes( string("a")+PATH_SLASH+"atest1.dat" ) << endl;

  FilenameList fnl;
  if ( FileSystem::FileSearch( "b", fnl, 0, 3 ) )
  {
    vector<string>::const_iterator iter = fnl.begin();
    while ( iter != fnl.end() )
    {
      cout << *iter << endl;
      iter++;
    }
  }
  else
    cout << "FileSearch failed" << endl;
  if ( FileSystem::EraseFolder( "b" ) )
    cout << "Erase Folder b ok" << endl;
  else
    cout << "Erase Folder b not ok" << endl;

  if ( FileSystem::DeleteFileOrFolder( string("xyz")+PATH_SLASH+"abc"+PATH_SLASH+"test2.dat" ) )
    cout << "Delete File test2.dat ok" << endl;
  else
    cout << "Delete File test2.dat failed" << endl;
  pause();
  if ( FileSystem::RenameFileOrFolder( string("xyz")+PATH_SLASH+"abc", string("xyz")+PATH_SLASH+"def" ) )
    cout << "Rename Folder xyz/abc -> xyz/def ok" << endl;
  else
    cout << "Rename Folder xyz/abc -> xyz/def not ok" << endl;
  if ( FileSystem::RenameFileOrFolder( "xyz", "uvw" ) )
    cout << "Rename Folder xyz -> uvw ok" << endl;
  else
    cout << "Rename Folder xyz -> uvw not ok" << endl;
  if ( FileSystem::DeleteFileOrFolder( string("uvw")+PATH_SLASH+"def" ) )
  {
    cout << "DeleteFolder ok" << endl;
    if ( FileSystem::DeleteFileOrFolder( "uvw" ) )
      cout << "DeleteFolder ok" << endl;
  }
  else
  {
    cout << "DeleteFolder failed" << endl;
  }
  FileSystem::SetCurrentFolder( ".." );
  FileSystem::EraseFolder( "test" );

  if (FileSystem::SetCurrentFolder( "g:/temp" ) )
  {
    FileSystem::CreateFolder( "subdir" );
  }
  else
    cout << "set g:/temp failed" << endl;

  return 0;
}

