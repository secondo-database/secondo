/*

August 2004, M. Spiekermann. InitRTFlags introduced. This removes 
some code redundancies. 

Nov 2004, M. Spiekermann. The CMsg instance was moved to the file 
Application.cpp since not all applications are linked with
SecondoInterfaceGeneral.o

*/

using namespace std;

#include "SecondoInterface.h"
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
  string logMsgList = SmiProfile::GetParameter( "Environment", "RTFlags", "", configFile );    
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
SecondoInterface::GetErrorMessage( int& errorCode )
{
  if ( errorCode < 0) {
    return "";
  }
  if ( !errMsgInitialized )
  {
    InitErrorMessages();
    errMsgInitialized = true;
  }
  stringstream defaultMsg;
  defaultMsg << "Error " << errorCode <<": ";

  map<int,string>::iterator errPos = errors.find( errorCode );
  errorCode = -1;
  if ( errPos != errors.end() )
  {
    return (defaultMsg.str() + errPos->second);
  }
  defaultMsg  << " Unknown Error!" << ends;
  return defaultMsg.str();
}

