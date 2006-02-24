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

August 2004, M. Spiekermann. InitRTFlags introduced. This removes 
some code redundancies. 

Nov 2004, M. Spiekermann. The CMsg instance was moved to the file 
Application.cpp since not all applications are linked with
SecondoInterfaceGeneral.o


*/

using namespace std;

#include "SecondoInterface.h"
#include "SecondoSystem.h"
#include "QueryProcessor.h"
#include "LogMsg.h"
#include "Profiles.h"

NestedList*
SecondoInterface::GetNestedList()
{
  return (al);
}


void
SecondoInterface::InitRTFlags(const string& configFile) {

  // initialize runtime flags
  string logMsgList = SmiProfile::GetParameter( "Environment", 
                                                "RTFlags", "", configFile );    
  RTFlag::initByString(logMsgList);
  RTFlag::showActiveFlags(cout);
	
}


/*
3.2 Error Messages

For a description of error handling see the definition module. 
Procedure ~InitErrorMessages~ should be copied after any changes into the definition module.

*/

bool SecondoInterface::errMsgInitialized = false;
map<int,string> SecondoInterface::errors;


/*
1.4 Procedure ~GetErrorMessage~

*/
string
SecondoInterface::GetErrorMessage( const int errorCode )
{
  if ( !errMsgInitialized )
  {
    InitErrorMessages();
    errMsgInitialized = true;
  }
  stringstream defaultMsg;
  defaultMsg << "(No " << errorCode <<") ";

  map<int,string>::iterator errPos = errors.find( errorCode );
  if ( errPos != errors.end() )
  {
    return (defaultMsg.str() + errPos->second);
  } 
  else
  {
    defaultMsg  
      << " Unknown Error! No message for error code No. " 
      << errorCode << "found."; 
  }
  return defaultMsg.str();
}

