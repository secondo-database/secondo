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

//paragraph [23]  table3columns:  [\begin{quote}\begin{tabular}{lll}] [\end{tabular}\end{quote}]
//[--------]  [\hline]
//characters  [1] verbatim: [\verb|]  [|]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [$\leq$]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File LogMsg.h 

December 2003 M. Spiekermann 

August 2004 M. Spiekermann. Implemented class ~CMsg~

Nov 2004 M. Spiekermann. Tabstops were replaced by spaces since different
setting of tabstops in editors will mess the code formatting.

Dec 2004, M. Spiekermann. New function ~setFlag~ implemented.

Jan 2005, M. Spiekermann. Some "stringstream << ends"[1] have been removed. The
~ends~ function adds a byte of value 0 to the ~stringstreams~ and also into
their resulting strings. This caused a hard to find bug (Value = is a
whitespace) in nested list text atoms since value 0 is the only one which is
used internally in text records to distinguish between empty and non empty
space. 

May 2005, M. Spiekermann. cmsg.info overloaded.

August 2005, M. Spiekermann. Redirection of output corrected. Moreover, strings
containing only white spaces will be ignored. Errors will be printed in color,
if the appropriate flag is set.

1.1 Overview

This file declares a class ~RTFlag~ (Runtime Flag) and a preprocessor Macro
~LOGMSG~. It can be used to identify a bool value with a string constant. The
value for a given flag name is true, when it appears in the file
"SecondoConfig.ini"[1] in the list of the values for the key RTFlag. It can be
used for trace messages or runtime configuration. This is good for testing or
comparing different implementations without recompilation. The alternative of
defining and reading in new keys in "SecondoConfig.ini" is much more
complicated.  The macro  or the class are used as presented below. 

----LOGMSG("MyFlagName", cerr << "variable x:" << x << endl;)

    if ( RTFlag::isActive("MyFlagName") ) {
    
    ... code which should only be executed in this case

    } 
----

All flags should be documented in the configuration file. However, the
mechanism is quite simple, take care to use the same string constants in your
code and in the configuration file otherwise you will get in trouble. 

Sending information to ~cout~ or ~cerr~ is not a good idea, since in a client
server setup these information will not transfered to the client. Hence a
global message object ~cmsg~ of class ~CMsg~ will be used as transmitter for
messages. There are four methods which return the reference to an ostream
object.  Additonally, you can easily send data to a file. Currently, the
messages are not send to a client via the socket communication but this can
easily be integrated later.

Here are some examples how to use the interface:

---- cmsg.info() << "Non critical information send to cout" << endl;
     cmsg.send();
     cmsg.warning() << "More critical information send to cout" << endl;
     cmsg.send();
     cmsg.error() << "Error information send to cerr" << endl;
     cmsg.send();
     cmsg.file() << "Information send to the file secondo.log" << endl;
     cmsg.send();
     cmsg.file("my-logfile.log") << "Information send to my-logfile.log" << endl;
----

Before changing the output channel it is important to clear the messsage buffer
with the ~send~ method.

*/


#ifndef CLASS_RTFLAG_H
#define CLASS_RTFLAG_H



// some macros which may be useful for tracing the program execution
#ifdef TRACE_ON
#define ETRACE(a) { a }
#define TRACE(a) {cout << a << ":" << endl;}
#define NTRACE(n,a) { static int ctr=0; ctr++; \
                      if ( (ctr % n)  == 0) \
                       {cout << ctr << " - " << a << ":" << endl; }}
#define SHOW(a) {cout << "  " << #a << " = " << a << endl;} 
#else
#define ETRACE(a)
#define TRACE(a) 
#define NTRACE(n,a) 
#define SHOW(a) 
#endif



#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <CharTransform.h>

#include "FileSystem.h"

using namespace std;

#ifndef LOGMSG_OFF
#define LOGMSG(a, b) if ( RTFlag::isActive(a) ) { b }
#endif


class RTFlag {

public:

  static void initByString( const string& keyList );

  static void showActiveFlags(ostream& os);

  inline static bool isActive( const string& key ) { 
    
    if ( (it=flagMap.find( key )) != flagMap.end() ) { 

      return it->second;  
    } 
    else { 

      return false; 
    }
  }

  inline static void setFlag( const string& key, const bool value ) {

    if ( (it=flagMap.find( key )) != flagMap.end() ) { 

      it->second = value;
    } 
    else { 

      flagMap[key] = value;
      cerr << "New Flag added!" << endl;
      showActiveFlags(cout);
    }
  }

private:

  RTFlag(){}
  ~RTFlag(){}

  static map<string,bool> flagMap;
  static map<string,bool>::iterator it;

};




class CMsg {

public:
  
  CMsg() : 
    stdOutput(1), 
    fp(new ofstream()),
    logFileStr("secondo.log"),
    prefix("tmp/")
  {
    files[logFileStr] = fp;
    fp->open((prefix + logFileStr).c_str()); 
    buffer.str("");
    allErrors.str("");
    devnull.str("");
  }
  ~CMsg() // close open files
  {
    for ( map<string,ofstream*>::iterator it = files.begin();
          it != files.end();
          it++ )
    {
       it->second->close();
       delete it->second;
    }     
  }

  inline ostream& file() 
  {
    fp = files[logFileStr]; 
    stdOutput = 3;  
    return buffer; 
  }

  inline ostream& file(const string& fileName) 
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

  inline ostream& info(const string& key) {
  
    if (RTFlag::isActive(key)) {
      stdOutput = 1; return buffer;
    } else { 
      stdOutput = 0; return devnull; 
    }
  }
  inline ostream& info()    { stdOutput = 1; return buffer; }
  inline ostream& warning() { stdOutput = 1; return buffer; }
  inline ostream& error()   { stdOutput = 2; return buffer; } 

  inline void send() {
    
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
      allErrors << "Error: " << buffer.str();
      break;
    }
    case 1: 
    {
      cout << buffer.str();
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

  inline string getErrorMsg() {

    string result = allErrors.str();
    allErrors.str("");
    allErrors.clear(); 
  
    if ( isSpaceStr(result) ) {
      result = "";
    }

    return result;
  }

private:

  int stdOutput;
  ofstream* fp;
  stringstream buffer;
  stringstream allErrors;
  stringstream devnull;
  const string logFileStr;
  const string prefix;
  map<string,ofstream*> files;
  
};

// defined in Application.cpp
extern CMsg cmsg;


/*

4 Class ErrorReporter

This class contains only static member functions. These functions 
permit reporting an error message (~ReportError~) and
retrieving it (~GetErrorMessage~). Once an error message has been
retrieved, it is removed. If there is no error message, the function
~RemoveErrorMessage~ sets its argument to ~""~. 

It should be used only for reporting ~type error~ messages.
An example of the usage of function ~ReportError~ is given in the 
type mapping function of operator ~feed~ in the relational algebra.

*/

class ErrorReporter
{
private:
  static bool receivedMessage;
  static string message;

public:
  static bool FreezeMessage;
  static bool TypeMapError;
  static void Reset() { TypeMapError=false; message=""; }
  static void ReportError(string msg);
  static void ReportError(char* msg);
  static void GetErrorMessage(string& msg);
};


/*
A Base class for Exceptions. All specific exception classes should be inherit
this class.

*/

class SecondoException {

  public:
  string msgStr;

  SecondoException() : msgStr("Unknown Error") {}
  SecondoException(const string& Msg) : msgStr(Msg) {}
  const string msg() { return msgStr; }
  
};


#endif
