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

Jan 2005, M. Spiekermann. Changes in getParameter.

*/

using namespace std;

#include <string>
#include <iostream>
#include <fstream>

#include <stdio.h>
#include <unistd.h>
#include "Profiles.h"

#ifdef SECONDO_WIN32
#include <windows.h>
#endif

static bool              CmpSec( const string& line, const string& appName );
static string::size_type CmpKey( const string& line, const string& keyName );

// --- GetParameter : Gets a string parameter from a profile

string SmiProfile::GetParameter( const string& sectionName, 
                                 const string& keyName,
                                 const string& defaultValue,
                                 const string& fileName )
{
  string resultString( "" );
  string line;
  ifstream inf( fileName.c_str() );

  if (inf)
  {
    while ( getline( inf, line ) )
    {
      if ( CmpSec( line, sectionName ) )
      {
        string::size_type valuePos;
        while ( getline( inf, line ) )
        {
          if ( line.length() > 0 && line[0] == '[' )  // next section
            break;
          valuePos = CmpKey( line, keyName );
          if ( valuePos != string::npos )
          {
            if ( line.find("+=") != string::npos ) { 
              resultString += ',';
            } 
            resultString += 
              line.substr( valuePos, line.find_last_not_of( " \t\r\n" )-valuePos+1 );
          }
        }
        break;
      }
    }
  }
  inf.close();

  // if key string not found, return default

  if ( resultString.length() == 0 )
  {
    resultString = defaultValue;
  }

  // overrule parameter if environment variable is set
  char* envStr = 0;
  envStr= getenv( ("SECONDO_PARAM_"+keyName).c_str() );
  if (envStr) {
      resultString = string(envStr);
  }

  return resultString;
}

// --- GetParameter : Gets an integer parameter from a profile

long SmiProfile::GetParameter ( const string& sectionName,
                                const string& keyName, long defaultValue,
                                const string& fileName )
{
  long value = defaultValue;
  string txt = GetParameter( sectionName, keyName, "", fileName );

  if (txt.length() > 0)
  {
    value = atol( txt.c_str() );
  }

  // overrule parameter if environment variable is set
  char* envStr = 0;
  envStr= getenv( ("SECONDO_PARAM"+keyName).c_str() );
  if (envStr) {
      value = atol(envStr);
  }

  return value;
}

// --- SetParameter : Add or replace a string parameter in a profile
//                    Delete key if value is empty
//                    Delete section if key name and key value are empty

bool SmiProfile::SetParameter ( const string& sectionName,
                                const string& keyName,
                                const string& keyValue,
                                const string& fileName )
{
  bool ok = false;
  string line;
  ifstream inf( fileName.c_str() );
  string tmp;

  if ( inf )
  {
#ifdef SECONDO_WIN32
    char tmpPath[MAX_PATH];
    char tmpFile[MAX_PATH];
    if ( GetTempPath( MAX_PATH, tmpPath ) == 0 )
    {
      inf.close();
      return false;
    }
    if ( GetTempFileName( tmpPath, "pro", 0, tmpFile ) == 0 )
    {
      inf.close();
      return false;
    }
    tmp = tmpFile;
    ofstream outf( tmpFile );
#else
    int tmpFd;
    char tmpFile[16];
    strcpy (tmpFile, "./profileXXXXXX" );
    if ( (tmpFd = mkstemp( tmpFile )) == -1 )
    {
      inf.close();
      return false;
    }
    tmp = tmpFile;
    ofstream outf( tmp.c_str() );
#endif
    outf.open( tmp.c_str() );
    if ( outf )
    {
      while( getline( inf, line ) )
      {
        if ( CmpSec( line, sectionName ) )
          break;
        outf << line << endl;
      }
      if ( inf.eof() )
      {
        // write new section
        if ( keyName.length() > 0 )
        {
          outf << '[' << sectionName << ']' << endl;
          outf << keyName << '=' << keyValue << endl;
          ok = true;
        }
      }
      else  // section found
      {
        if ( keyName.length() == 0 )
        {
          // remove section but keep comments
          while(getline(inf, line) )
          {
            // exit if next section
            if ( line.length() > 0 && line[0] == '[' )
              break;
            if ( line.length() > 0 && line[0] == ';' )
              outf << line << endl;
          }
        }
        else
        {
          // change only key, or add new key
          outf << line << endl;
          while ( true )
          {
            if ( !getline(inf, line) )
              line = "\0";
            if ( CmpKey( line, keyName ) != string::npos ||
                (line[0] == '[') || (line[0] == '\0') )
            {
              // write new key line
              outf << keyName << '=' << keyValue << endl;
              if ( line.length() > 0 && line[0] == '[' )
                outf << line << endl;
              break ;
            }
            outf << line << endl;
          }
        }
        // copy rest of file
        while ( getline( inf, line ) )
          outf << line << endl;

        ok = true;
      }
      // close files, copy tmpfile and delete
      inf.close();
      outf.close();
      remove( fileName.c_str() );
      inf.open( tmp.c_str() );
      outf.open( fileName.c_str() );
      while ( getline( inf, line ) )
        outf << line << endl;
      inf.close();
      outf.close();
      remove( tmp.c_str() );
    }

#ifndef SECONDO_WIN32
    close(tmpFd);
#endif
    outf.close();
  }
  else if ( sectionName.length() > 0 && keyName.length() > 0 )
  {
    // -- no file found, create a new
    ofstream outf;
    outf.open( fileName.c_str() );
    if ( outf )
    {
      outf << '[' << sectionName << ']' << endl;
      outf << keyName << '=' << keyValue << endl;
      ok = true;
    }
    outf.close();
  }
  inf.close();
  return ok;
}

// --- CmpSec : Local routine to check if line contains section name.
//              Returns TRUE if found.

static bool CmpSec( const string& line, const string& name )
{
  int lineLen = line.length();
  int nameLen = name.length();
  int i = 0;
  int n = 0;
  bool ok = false;

  if (line[i++] == '[')
  {
    for (;;)
    {
      if ( i >= lineLen ) break ;
      if ( n >= nameLen )
      {
        ok = ( line[i] == ']' );
        break;
      }
      if ( toupper(line[i]) != toupper(name[n]) ) break ;
      i++, n++;
    }
  }
  return ok;
}

// --- CmpKey : Local routine to find key name on line.
//              If found, the start position of the string value is returned.

static string::size_type CmpKey( const string& line, const string& keyName )
{
  string::size_type lineLen = line.length();
  string::size_type nameLen = keyName.length();
  string::size_type i = 0;
  string::size_type n = 0;
  string::size_type outPos = string::npos;
  string::size_type argPos = line.find( '=' );
  if ( argPos != string::npos && argPos < lineLen-1 )
  {
    i = line.find_first_not_of( " \t" );
    if ( i < argPos )
    {
      for (;;)
      {
        if ( n >= nameLen )
        {
          string::size_type operatorPos=line.find_first_not_of( " \t", i );
          if (  line[operatorPos] == '+' || line[operatorPos] == '=') {
            outPos = line.find_first_not_of( " \t\r\n", argPos+1 );
          }
          break;
        }
        if ( i >= argPos ) break ;
        if ( toupper(line[i]) != toupper(keyName[n]) ) break ;
        i++, n++;
      }
    }
  }
  return outPos;
}

// --- end of profiles.cpp ---

