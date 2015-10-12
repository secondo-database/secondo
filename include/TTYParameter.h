/*
---- 
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty for Mathematics 
and Computer Science, Database Systems for New Applications.

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

Dec 2007, M. Spiekermann. Struct TTYParameter outsourced into this separate header file. 
This circumvents some linker errors about doubly defined symbols which occured on win32. 

*/

#ifndef TTYParameter_H
#define TTYParameter_H

#include <string>
#include <vector>

using namespace std;

/*
1 TTYParameter
   
The struct ~TTYParameter~ encapsulates the processing of command options and environment 
variables.
   
*/

struct TTYParameter
{
  private: 
  static const bool needIdent;
 
  bool removeFirstArg(const string& expected);

  bool getEnvValue(const string& var, string& value);

  vector<string> plargs;

  public:
  int numArgs;
  char** argValues;
  
  string parmFile;
  string user;
  string pswd;
  string host;
  string port;
  string replayFile;
  string iFileName;
  string oFileName;
  string num;
  bool coverage;
  
  typedef enum {Test, Optimizer, Server, TTY} RunMode;
  RunMode runMode;
  
  bool runExamples;

  TTYParameter(const int argc, char** argv);

  bool isTestRunnerMode() 
  { 
    bool rc = removeFirstArg("-test"); 
    if (rc) {
      runMode = Test;
    }
    return rc;    
  } 
   
  bool isPLMode() 
  { 
    bool rc = removeFirstArg("-pl"); 
    if (rc) {
      runMode = Optimizer;
    }
    return rc ;   
  } 
 
  bool isServerMode() 
  { 
    bool rc = removeFirstArg("-srv"); 
    if (rc) {
      runMode = Server;
    }
    return rc ;   
  } 
	  
  void Print(ostream& os);

  void Appendto_plargs(const string& arg) { plargs.push_back(arg); }
  
  char** Get_plargs(int& argc);


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

  bool CheckConfiguration();

};

#endif


