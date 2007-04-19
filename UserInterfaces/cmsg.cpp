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

April 2007, M. Spiekermann. Some code moved from LogMsg.h into this file

*/

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "CharTransform.h"
#include "FileSystem.h"
#include "LogMsg.h"

using namespace std;

ostream* traceOS = &cerr;

/*
A global pointer to an ~ostream~ object.

*/

CMsg cmsg;

/*

Here we define a global instance ~cmsg~ which can be used in any other code
file.  Since it is not specified when the constructor will be called (we
observed problems on Mac OSX with gcc 4.0)the initialize function will be
called 

*/

CMsg::CMsg()
{
  init(); 
}
  

CMsg::~CMsg() // close open files
{
  for ( map<string,ofstream*>::iterator it = files.begin();
	it != files.end();
	it++ )
  {
     it->second->close();
     delete it->second;
  }
}


void CMsg::init()
{
  { 
    stdOutput = 1; 
    fp = new ofstream();
    logFileStr = "secondo.log";
    prefix = "tmp/";
    files[logFileStr] = fp;
    fp->open((prefix + logFileStr).c_str()); 
    buffer.str("");
    allErrors.str("");
    devnull.str("");
  }
}

ostream& CMsg::file() 
{
  fp = files[logFileStr]; 
  stdOutput = 3;  
  return buffer; 
}

ostream& CMsg::file(const string& fileName) 
{ 
  map<string,ofstream*>::iterator it = files.find(fileName);
  
  if  ( it != files.end() ) {
  
    fp = it->second;
    
  } else {
  
    fp = new ofstream();
    files[fileName] = fp;
    fp->open((prefix + fileName).c_str());
  }
  //stdOutput = 3;    
  return *fp; 
}

ostream& CMsg::info(const string& key) {

  if (RTFlag::isActive(key)) {
    stdOutput = 1; return buffer;
  } else { 
    stdOutput = 0; return devnull; 
  }
}

void CMsg::send() {
  
  if ( isSpaceStr( buffer.str() ) ) {
    buffer.str("");
    buffer.clear();
    return;
  }

  switch (stdOutput) { 

  case 3:
  {
    (*fp) << buffer.str();
     break;
  }
  case 2: 
  {
    cerr << color(red) << "Error: " << buffer.str() << color(normal);
    cerr.flush();
    allErrors << "Error: " << buffer.str();
    break;
  }
  case 1: 
  {
    cout << buffer.str();
    cout.flush();
    break;
  }
  case 0:
  {
    devnull.str("");
    devnull.clear();
    break;
  } 
  default :
  {
    allErrors << buffer.str();
  }
  }

  buffer.str("");
  buffer.clear();
}

string CMsg::getErrorMsg() {

  string result = allErrors.str();
//  allErrors.str("");
//  allErrors.clear(); 

  if ( isSpaceStr(result) ) {
    result = "";
  }

  return result;
}

void CMsg::resetErrors(){
  allErrors.str("");
  allErrors.clear();
}

  
