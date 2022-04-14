/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics and Computer Science,
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

1 File System Management

May 2002 Ulrich Telle

Sept 2004 M. Spiekermann. Bugs in ~GetparentFolder~ and ~AppendSlash~ corrected.

Sept 2006 M. Spiekermann. When windows.h is included many WIN-API functions
like ~CopyFile~ are defined as a macro and mapped to ~CopyFileA~ or
~CopyFileB~. This is very awful since code parts using the
same name like class member functions are also renamed which causes strange
linker errors!

June 2009 Sven Jungnickel new function MakeTemp() added.

*/


#include "SecondoException.h"
#include "SecondoConfig.h"
#include <iostream>
#include <cassert>

#ifdef SECONDO_WIN32
#include <io.h>
#include <windows.h>
#include <time.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

#include "FileSystem.h"
#include "LogMsg.h"
#include <stdlib.h>
#include <stdio.h>
#include <cstring>

#include <sstream>
#include <stdexcept>

#include "WinUnix.h"

#include <stack>

#ifdef THREAD_SAFE
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#endif

using namespace std;

/*
2 ~FileErr~ a class representing Nested-List exceptions

*/
class FileErr : public SecondoException {

  public:

  FileErr(const string& Msg, const int rc) : SecondoException(Msg), rc(rc) {
  }

  const string msg() {
    /*
    stringstream tmp;
    tmp << msgStr << "( rc = " << rc ")";
    return tmp.str(); */
    return msgStr;
  }

  int getRc() {
     return rc;
  }

  private:
  int rc;
};



#ifdef SECONDO_WIN32
const FileAttributes attrMask = ( FILE_ATTRIBUTE_HIDDEN   |
                                  FILE_ATTRIBUTE_READONLY |
                                  FILE_ATTRIBUTE_SYSTEM );
/*
~attrMask~ is used to remove file attributes from a file, so that the
file may be deleted.

*/
#endif

const static char* currentFolder = ".";
const static char* parentFolder  = "..";
/*
~currentFolder~ and ~parentFolder~ are filenames that should never be added
to file lists for a folder.

*/

string
FileSystem::GetCurrentFolder()
{
  char str[500];
  string folder = "";
#ifdef SECONDO_WIN32
  int count = ::GetCurrentDirectory( 497, str );
  if ( count > 0 && count < 498 )
  {
    folder = string( str );
  }
#else
  char* buf = ::getcwd( str, 499 );
  if ( buf == str )
  {
    folder = buf;
  }
#endif
  return (folder);
}


string
FileSystem::GetParentFolder( const string& folder, int level /* =1 */)
{
  string parent = folder;
  string::size_type n = parent.find_last_of(PATH_SLASH[0]);

  if ( n != string::npos ) { // erase path information
    parent.erase(n);
  } else {
     return ""; // no parent found
  }

  if (level-1) {
    return GetParentFolder(parent,level-1);
  } else {
    return parent;
  }
}


bool
FileSystem::SetCurrentFolder( const string& folder )
{
#ifdef SECONDO_WIN32
  return (::SetCurrentDirectory( folder.c_str() ) != 0);
#else
  return (chdir( folder.c_str() ) ==0);
#endif
}

bool FileSystem::CreateFolderEx(string pathname){

  size_t pos = 0;
  bool ret_val = true;

   while(ret_val && pos != std::string::npos) {
     pos = pathname.find(PATH_SLASH[0], pos + 1);
     if(pos==std::string::npos){
        ret_val = CreateFolder(pathname.substr(0, pos));
     } else {
        CreateFolder(pathname.substr(0, pos));
     }
   }
  return ret_val;
}



bool
FileSystem::CreateFolder( const string& folder )
{
#ifdef SECONDO_WIN32
  return (::CreateDirectory( folder.c_str(), NULL ) != 0);
#else
  return (::mkdir( folder.c_str(), S_IRWXU | S_IRWXG | S_IRWXO ) == 0);
#endif
}





bool
FileSystem::IsDirectory( const string& fileName )
{
  try {
  if (fileName == "") {
    return false;
  }
  FileAttributes fileAttribs = Get_FileAttributes( fileName );
#ifdef SECONDO_WIN32
  int isFolder = (fileAttribs & FILE_ATTRIBUTE_DIRECTORY);
#else
  int isFolder = S_ISDIR( fileAttribs );
#endif
    return isFolder != 0;
  } catch (FileErr &f) {
    cerr << f.msg();
    return false;
  }
}

bool
FileSystem::DeleteFileOrFolder( const string& fileName )
{
  int rc=0;
  const string errMsg = "Can't remove element \"" + fileName + "\"! ";
  try {
  if (fileName == "")
    throw FileErr(errMsg, rc);

  FileAttributes fileAttribs = Get_FileAttributes( fileName );
#ifdef SECONDO_WIN32
  int isFolder = (fileAttribs & FILE_ATTRIBUTE_DIRECTORY);
#else
  int isFolder = S_ISDIR( fileAttribs );
#endif
   // Is this a folder (directory) or file?
  if ( isFolder )
  {
    // Remove the directory.
#ifdef SECONDO_WIN32
    rc = ::RemoveDirectory( fileName.c_str() );
    if (!(rc != 0))
      throw FileErr(errMsg, rc);
#else
    rc = ::rmdir( fileName.c_str() );
    if (!(rc == 0))
      throw FileErr(errMsg, rc);
#endif
  }
  else
  {
    // Delete the file
#ifdef SECONDO_WIN32
    rc = ::DeleteFile( fileName.c_str() );
    if (!(rc != 0))
      throw FileErr(errMsg, rc);
#else
    rc = ::unlink( fileName.c_str() );
    if (!(rc == 0))
      throw FileErr(errMsg, rc);
#endif
  }
  } catch (FileErr &f) {
    cerr << f.msg();
    return false;
  }
  return true;
}

bool
FileSystem::EraseFolder( const string& folder, uint16_t maxLevels )
{
#ifdef SECONDO_WIN32
  // Determine file type
  FileAttributes fileAttribs = Get_FileAttributes( folder );
  if ( fileAttribs & FILE_ATTRIBUTE_DIRECTORY )
  {
    // Remove the folder (directory):
    // (1) Get the list of filenames within the folder (directory).
    FilenameList filenameList;
    FileSearch( folder, filenameList, 0, maxLevels );

    // (2) Remove all files (and subfolders) in the folder
    vector<string>::const_iterator iter = filenameList.begin();
    while ( iter != filenameList.end() )
    {
      // Remove any file protection attributes.
      vector<string>::const_reference filename = *iter;
      UnprotectFile( filename );
      DeleteFileOrFolder( filename );
      iter++;
    }
  }
  // Remove any file protection attributes.
  UnprotectFile( folder );
  // Remove the requested folder.
  return (DeleteFileOrFolder( folder ));
#else
  string command = "rm -rf " + folder;
  int syserr = ::system( command.c_str() );
  return (syserr != 127 && syserr != -1);
#endif
}

bool
FileSystem::RenameFileOrFolder( const string& currentName,
                                const string& newName )
{
#ifdef SECONDO_WIN32
  return (::MoveFile( currentName.c_str(), newName.c_str() ) != 0);
#else
  string command = "mv " + currentName + " " + newName;
  int syserr = ::system( command.c_str() );
  return (syserr == 0);
#endif
}

bool
FileSystem::Copy_File( const string& source, const string& dest )
{
#ifdef SECONDO_WIN32
  return (::CopyFile( source.c_str(), dest.c_str(), FALSE ) != 0);
#else
  string command = "cp " + source + " " + dest;
  int syserr = ::system( command.c_str() );
  return (syserr != 127 && syserr != -1);
#endif
}

bool
FileSystem::FileOrFolderExists( const string& fileName )
{
  return (access( fileName.c_str(), 0 ) != -1);
}

FileAttributes
FileSystem::Get_FileAttributes( const string& fileName )
{
  FileAttributes attribs = 0;
#ifdef SECONDO_WIN32
  attribs = ::GetFileAttributes( fileName.c_str() );
  if ( (void*) attribs == (void*)INVALID_HANDLE_VALUE ) {
    attribs = 0;
  }
#else
  struct stat filestatus;
  int rc = ::lstat( fileName.c_str(), &filestatus );
  if ( rc == 0 )
    attribs = filestatus.st_mode;
  else
    attribs = 0;
#endif
  return (attribs);
}

bool
FileSystem::Set_FileAttributes( const string& fileName, FileAttributes attribs )
{
#ifdef SECONDO_WIN32
  return (::SetFileAttributes( fileName.c_str(), attribs ) != 0);
#else
  return (::chmod( fileName.c_str(), attribs ) != -1);
#endif
}
int32_t
FileSystem::GetFileSize( const string& fileName )
{
  if(!FileOrFolderExists(fileName)) {
    return -1;
  }
  std::ifstream f;
  f.open(fileName.c_str(), std::ios_base::binary | std::ios_base::in);
  if (!f.good() || f.eof() || !f.is_open()) { return 0; }
  f.seekg(0, std::ios_base::beg);
  std::ifstream::pos_type begin_pos = f.tellg();
  f.seekg(0, std::ios_base::end);
  return static_cast<int32_t>(f.tellg() - begin_pos);
}

bool
FileSystem::FileSearch( const string& folder,
                        FilenameList& filenameList,
                        const string* searchName,
                        uint16_t maxLevels,
                        bool includeFolders,
                        bool fullPath,
                        FileSearchCallbackFunc fileSearchCallback )
{
  // Save the current folder and change to folder where the search starts.
  string oldFolder = GetCurrentFolder();
  if(!SetCurrentFolder( folder ))
  {
    SetCurrentFolder(oldFolder);
    return false;
  }

  // Get absolute pathname of the current folder.
  string absoluteFolder = GetCurrentFolder();
  if ( maxLevels ) maxLevels--;

#ifdef SECONDO_WIN32
  WIN32_FIND_DATA findData = { 0 };
  HANDLE findHandle = ::FindFirstFile( "*", &findData );

  if ( findHandle == INVALID_HANDLE_VALUE )
  {
    SetCurrentFolder( oldFolder );
    return false;
  }

  // Add file entries to FileList.
  do
  {
    // Skip the entries for the current folder and the parent folder
    if ( strcmp( findData.cFileName, currentFolder ) &&
         strcmp( findData.cFileName, parentFolder ))
    {
      string pathName = absoluteFolder;
      AppendSlash( pathName );
      pathName += findData.cFileName;
/*
The names of files in subfolders are inserted into the list before
the names of subfolders are added. This ensures that folders are empty
when the entries in the list are used in ascending index sequence to
delete each file.

*/
      int isFolder = findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
      if ( maxLevels && isFolder )
      {
        if ( !FileSearch( pathName, filenameList, searchName,
                          maxLevels, includeFolders, fullPath,
                          fileSearchCallback ) )
        {
          SetCurrentFolder( oldFolder );
          return false;
        }
      }

      // Subfolders will only be added to the list if requested

      if ( !isFolder || (isFolder && includeFolders) )
      {
        if ( !searchName ||
            (searchName && *searchName == string( findData.cFileName )) )
        {
          if ( !fileSearchCallback ||
              (fileSearchCallback &&
               fileSearchCallback( absoluteFolder, findData.cFileName,
                                   findData.dwFileAttributes )) )
          {
            if ( fullPath )
            {
              filenameList.push_back( pathName );
            }
            else
            {
              filenameList.push_back( findData.cFileName );
            }
          }
        }
      }
    }
    if ( !::FindNextFile( findHandle, &findData ) )
    {
      DWORD nexterr = ::GetLastError();
      if ( nexterr == ERROR_NO_MORE_FILES )
      {
        // Last entry was found, get out of the loop
        break;
      }
      else
      {
        SetCurrentFolder( oldFolder );
        return false;
      }
    }
  }
  while ( true );

  if ( !::FindClose( findHandle ) )
  {
    SetCurrentFolder( oldFolder );
    return false;
  }
#else
  DIR* dir = ::opendir( absoluteFolder.c_str() );
  if ( dir )
  {
    dirent* dirEntry = 0;
    while ( (dirEntry = ::readdir( dir )) != 0 )
    {
      if ( strcmp( dirEntry->d_name, currentFolder ) == 0 ) continue;
      if ( strcmp( dirEntry->d_name, parentFolder ) == 0 )  continue;
      string pathName = absoluteFolder;
      AppendSlash( pathName );
      pathName += dirEntry->d_name;
      FileAttributes fileAttribs = Get_FileAttributes( pathName );
/*
The names of files in subfolders are inserted into the list before
the names of subfolders are added. This ensures that folders are empty
when the entries in the list are used in ascending index sequence to
delete each file.

*/
      int isFolder = S_ISDIR( fileAttribs );
      if ( maxLevels && isFolder )
      {
        FileSearch( pathName, filenameList, searchName,
                    maxLevels, includeFolders, fullPath,
                    fileSearchCallback );
      }

      // Subfolders will only be added to the list if requested

      if ( S_ISLNK( fileAttribs ) == 0 &&
          (!isFolder || (isFolder && includeFolders)) )
      {
        if ( !searchName ||
            (searchName && *searchName == string(dirEntry->d_name)) )
        {
          if ( !fileSearchCallback ||
              (fileSearchCallback &&
               fileSearchCallback( absoluteFolder,
                                   dirEntry->d_name, fileAttribs )) )
          {
            if ( fullPath )
            {
              filenameList.push_back( pathName );
            }
            else
            {
              filenameList.push_back( dirEntry->d_name );
            }
          }
        }
      }
    }
    ::closedir( dir );
  }
#endif
  // Restore current directory.
  SetCurrentFolder( oldFolder );
  return true;
}

bool
FileSystem::SearchPath( const string& fileName, string& foundFile )
{
  bool ok = false;
#ifdef SECONDO_WIN32
  char buffer[MAX_PATH];
  char* filepart;
  if ( ::SearchPath( NULL, fileName.c_str(), NULL,
                     MAX_PATH, buffer, &filepart ) == 0 )
  {
    if ( ::SearchPath( NULL, fileName.c_str(), ".exe",
                       MAX_PATH, buffer, &filepart ) != 0 )
    {
      foundFile = buffer;
      ok = true;
    }
  }
  else
  {
    foundFile = buffer;
    ok = true;
  }
#else
  if ( fileName[0] == PATH_SLASH[0] )
  {
    // file name is fully qualified
    foundFile = fileName;
    ok = FileOrFolderExists( fileName );
  }
  else
  {
    // 1. Search in current directory
    string cwd = GetCurrentFolder();
    AppendSlash( cwd );
    foundFile = cwd + fileName;
    ok = FileOrFolderExists( foundFile );
    if ( !ok )
    {
      // 2. Search path if file name was not partially qualified
      if ( fileName.find( PATH_SLASH ) != std::string::npos )
      {
        char* envPath = getenv( "PATH" );
        if ( envPath != 0 )
        {
          string path;
          string pathRest = envPath;
          string::size_type delim;
          while ( pathRest.length() > 0 )
          {
            delim = pathRest.find( ":" );
            if ( delim != string::npos )
            {
              path = pathRest.substr( 0, delim-1 );
              pathRest = pathRest.substr( delim+1 );
            }
            else
            {
              path = pathRest;
              pathRest = "";
            }
            AppendSlash( path );
            foundFile = path + fileName;
            ok = FileOrFolderExists( foundFile );
            if ( ok ) break;
          }
        }
      }
    }
  }
#endif
  if ( !ok )
  {
    foundFile = "";
  }
  return (ok);
}

string
FileSystem::MakeTemp(const string& templ)
{
  static int ctr = 0;
  #ifdef THREAD_SAFE
     static boost::mutex mtx;
     boost::lock_guard<boost::mutex> guard(mtx);  
  #endif

  // append CPU clock and placeholder for mktemp function
  
  stringstream ss;
  ss << templ << clock() << "-" << WinUnix::getpid() << "-" << ctr++;

  return ss.str();
}

void
FileSystem::AppendItem( string& pathName, const string& item )
{
  string::size_type n = pathName.find_last_of(PATH_SLASH[0]);

  if ( n != pathName.length() ) // last character is not a path separator
  {
    pathName += PATH_SLASH;
    pathName += item;
  }
}

void
FileSystem::AppendSlash( string& pathName )
{
  AppendItem(pathName,"");
}

string FileSystem::Basename(const string& pathName){
   string path_sep = PATH_SLASH;
   #ifdef SECONDO_WIN32
      path_sep += "/";
   #endif 
   string::size_type n = pathName.find_last_of(path_sep);

   if(n == string::npos){
     return pathName;
   }
   if(n + 1 == pathName.length()){
     if(n==0){
        return pathName;
     } else {
        return Basename(pathName.substr(0,n));
     }
   }
   return pathName.substr(n+1);
}



#ifdef SECONDO_WIN32
void
FileSystem::UnprotectFile( const string& fileName )
{
  assert( &fileName );
  FileAttributes fileAttribs = Get_FileAttributes( fileName );
  if ( fileAttribs & attrMask )
  {
    // File has attributes that must be cleared before it may be deleted.
    fileAttribs &= ~attrMask;
    Set_FileAttributes( fileName, fileAttribs );
  }
}
#endif

