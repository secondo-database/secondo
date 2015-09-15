/*
---- 
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

#include <stdlib.h>
#include <assert.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <cstring>

#include "Environment.h"
#include "TTYParameter.h"
#include "FileSystem.h"
#include "LogMsg.h"
#include "WinUnix.h"

using namespace std;

extern CMsg cmsg;

Environment*
Environment::instance = 0;

Environment::Environment()
{
  keyMap["SEC_pScale"] = Float; 
  keyMap["SEC_pMinRead"] = Int;
  keyMap["SEC_pMaxRead"] = Int;
  keyMap["SEC_pAllowHints"] = Bool;
  keyMap["SECONDO_PLATFORM"] = String; 
  keyMap["SECONDO_BUILD_DIR"] = String; 

  init(); 
}

void
Environment::init() {

  map<string, Type>::iterator it = keyMap.begin();
  for(; it != keyMap.end(); it++) {

    string key = it->first;	  
    char* value = getenv( key.c_str() );

    if ( value != 0 ) {    
    switch (it->second) {

      case Int:   { int v = atoi(value); 
		    intMap[key] = v;
		    break; } 

      case Float: { float v = atof(value); 
		    floatMap[key] = v;
		    break; }

      case Bool:  { string val(value);
		    bool v = (val == "true" || val == "TRUE") ? true 
			                                      : false; 
                    cout << "Bool: " << key << ", " << value << endl;
		    boolMap[key] = v;
		    break; }

      case String: { cerr << "Env: " << key << " = " << value << endl; 
			   stringMap[key] = value; break; }

      default: assert(false);		   
    }	      
    }
  }	  

}	

/*
Implementation of TTYParameter

*/

const bool TTYParameter::needIdent = false;
 
bool TTYParameter::removeFirstArg(const string& expected)
{
  if (numArgs < 2)
    return false;	    
  
  string value(argValues[1]);
  if ( value == expected ) 
  { 
    numArgs--;
    argValues[1] = argValues[0];
    argValues = &(argValues[1]);
    return true;
  }  
  return false;
}

bool TTYParameter::getEnvValue(const string& var, string& value)
{	  
  char* envValue=0;
  if ( value.empty() )
  {
    envValue = getenv( var.c_str() );
    if ( envValue != 0 )
    {
      value = envValue;
      cout << "Using " << var << " = " << value << endl;
      return true;
    }
  }
  return false;
}

  
/*
removes the first argument if present.

*/
  

TTYParameter::TTYParameter(const int argc, char** argv)
{
  parmFile      = "";
  user          = "";
  pswd          = "";
  host          = "";
  port          = "";
  iFileName     = "";
  oFileName     = "";
  num           = "0";
  coverage      = false;

  numArgs = argc;
  argValues = argv;

  runMode = TTY;
  runExamples = false;
} 


void TTYParameter::Print(ostream& os)
{
  os << "parmFile  = " << parmFile << endl;
  os << "user      = " << user << endl;
  os << "pswd      = " << pswd << endl;
  os << "host      = " << host << endl;
  os << "port      = " << port << endl;
  os << "iFileName = " << iFileName << endl;
  os << "oFileName = " << oFileName << endl;
  os << "num       = " << num << endl;
  os << "numArgs   = " << numArgs << endl;
  os << "runMode   = " << runMode << endl;
  os << "runExamples = " << runExamples << endl;
}	  

/*
1.1 CheckConfiguration

This function checks the Secondo configuration. First it looks for the name
of the configuration file on the command line. If no file name was given on
the command line or a file with the given name does not exist, the environment
variable SECONDO\_CONFIG is checked. If this variable is defined it should point
to a directory where the configuration file can be found. If the configuration
file is not found there, the current directory will be checked. If no configuration
file can be found the program terminates.

If a valid configuration file was found initialization continues.

*/

bool
TTYParameter::CheckConfiguration()
{
  bool ok = true;

  int i = 1;
  string argSwitch = "", argValue = "";
  bool argOk = false;

  static const string availOptions = 
  "Use option -? or --help to get information about available options."; 
 
  stringstream usageMsg;
  usageMsg << 
  "Usage: Secondo{BDB|CS} [options]\n" <<
  "\n" <<
  "Operation mode switches (1st parameter):\n" <<
  "----------------------------------------\n" <<
  "  -test      : TestRunner mode\n" <<
  "  -pl        : Optimizer mode\n" <<
  "  -srv       : Server mode (SecondoBDB only!)\n" <<
  "\n" <<
  "Options:                                             (Environment-Var.)\n" <<
  "-----------------------------------------------------------------------\n" <<
  "  -c config  : Secondo configuration file            (SECONDO_CONFIG)\n" <<
  "  -i input   : Name of input file  (default: stdin)\n" <<
  "  -o output  : Name of output file (default: stdout)\n" <<
  "  -u user    : User id                               (SECONDO_USER)\n" <<
  "  -s pswd    : Password                              (SECONDO_PSWD)\n" <<
  "\n" <<
  "CS only:\n" <<
  "-----------------------------------------------------------------------\n" <<
  "  -h host    : Host address of Secondo server        (SECONDO_HOST)\n" <<
  "  -p port    : Port of Secondo server                (SECONDO_PORT)\n" <<
  "\n" << 
  "Mode -test only:\n" <<
  "-----------------------------------------------------------------------\n" <<
  "  -num N     : run only the specified test number \n" <<
  "  -coverage  : check if all overloaded value mappings are called \n" <<
  "\n" <<
  "Mode -pl only:\n" <<
  "-----------------------------------------------------------------------\n" <<
  "  -G#[km]       : global stack size = #\n" <<
  "  -L#[km]       : local stack size = #\n" <<
  "  ...           : other prolog options see man pl \n" <<
  "\n" <<
  "Note: Command line options overrule environment variables.\n";

  // check comamnd options 
  while (i < numArgs)
  {
    argSwitch = argValues[i];
    if ( i < numArgs-1)
    {
      argValue  = argValues[i+1];
      argOk = (argValue[0] != '-');
    }
    else
    {
      argValue = "";
      argOk = false;
    }
    if ( argSwitch == "-?" || argSwitch == "--help")  // Help
    {
      cout << usageMsg.str() << endl;
      ok = false;
    }
    else if ( argOk && argSwitch == "-c" )  // Configuration file
    {
      parmFile = argValue;
    }
    else if ( argOk && argSwitch == "-i" )  // Input file
    {
      iFileName = argValue;
    }
    else if ( argOk && argSwitch == "-o" )  // Output file
    {
      oFileName = argValue;
    }
    else if ( argOk && argSwitch == "-u" )  // User id
    {
      user = argValue;
    }
    else if ( argOk && argSwitch == "-s" )  // Password
    {
      pswd = argValue;
    }
    else if ( argOk && argSwitch == "-h" )  // Host
    {
      host = argValue;
    }
    else if ( argOk && argSwitch == "-p" )  // Port
    {
      port = argValue;
    }
    else if ( argOk && argSwitch == "-num" )  // Number of test case
    {
      num = argValue;
    }
    else if ( argSwitch == "-coverage" )  // Switch on coverage tests 
    {
      coverage = true;
    }
    else if ( argSwitch == "-e" )  // Expecting example file 
    {
      runExamples = true;
    }
    else if ( argSwitch == "-test" )  // TestRunner mode
    {
      runMode = Test;
    }
    else if ( argSwitch == "-pl" )  // Optimizer mode
    {
      runMode = Optimizer;
    }
    else if ( argSwitch == "-srv" )  // Server mode
    {
      runMode = Server;
    }
    else
    {
      if (runMode != Optimizer) 
      {	      
        cout << "Error: Invalid option: '" << argSwitch << "'." << endl;
        if ( argOk )
        {
          cout << "  having option value: '" << argValue << "'." << endl;
        }
        ok = false;
      }
      else
      {
        // save unknown arguments in order to pass them 
	// to the prolog engine
        plargs.push_back( argSwitch );		
	if (argOk) {
          plargs.push_back( argValue );		
        }
      }	      
    }
    i++;
    if ( argOk )
    {
      i++;
    }
  }

  if((runMode==Optimizer) && (plargs.size()==0) ){
    plargs.push_back(argValues[0]);
  }
 
  if (!ok) {
    cout << availOptions << endl;
    return false;
  }  
  
  // check if parameter values are empty and environment variables are set
  getEnvValue("SECONDO_CONFIG", parmFile);
  getEnvValue("SECONDO_USER", user);
  getEnvValue("SECONDO_PSWD", pswd);
  getEnvValue("SECONDO_HOST", host);
  getEnvValue("SECONDO_PORT", port);
  
  if ( needIdent ) // Is user identification needed?
  {
    int count = 0;
    while (count <= 3 && user.length() == 0)
    {
      count++;
      cout << "Enter user id: ";
      getline( cin, user );
    }
    ok = user.length() > 0;
    if ( !ok )
    {
      cout << "Error: No user id specified." << endl;
    }
    if ( ok && pswd.length() == 0 )
    {
      count = 0;
      while (count <= 3 && user.length() == 0)
      {
        count++;
        cout << "Enter password: ";
        getline( cin, pswd );
      }
      if ( pswd.length() == 0 )
      {
        cout << "Error: No password specified." << endl;
        ok = false;
      }
    }
  }
//  else
//  {
//    user = "SECONDO";
//    pswd = "SECONDO";
//  }

  // check if parmfile is no present try default
  if ( parmFile.empty() )
  {
    string cwd = FileSystem::GetCurrentFolder();
    FileSystem::AppendSlash( cwd );
    parmFile = cwd + "SecondoConfig.ini";
    cmsg.warning() << "Warning: No configuration file specified trying " 
                   << parmFile << endl;
  } 

  bool found = FileSystem::FileOrFolderExists( parmFile );
  if ( !found ) // try environment variable 
  {
    cmsg.error() << "Configuration file does not exist" << endl;
    ok = false;
  }
  else
  {
    cmsg.info() << "Using configuration file" << parmFile << endl; 
    WinUnix::setenv("SECONDO_CONFIG", parmFile.c_str());
  } 
  cmsg.send();

  if ( !ok )
  {
    cout << availOptions << endl;
  }
  return (ok);
}

char** TTYParameter::Get_plargs(int& argc)
{
  argc = plargs.size();
  char** argv = new char*[argc];

  for (int i=0; i < argc; i++) 
  {
    argv[i] = strdup( plargs[i].c_str() );  
  }  

  return argv;
}

