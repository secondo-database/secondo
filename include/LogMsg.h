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

May 2005, M. Spiekermann. Function CMsg::info overloaded.

August 2005, M. Spiekermann. Redirection of output corrected. Moreover, strings
containing only white spaces will be ignored. Errors will be printed in color,
if the appropriate flag is set.

April 2007, M. Spiekermann. Implementation parts moved to .cpp files.

1.1 Overview

This file should collect all methods or techniques used in SECONDO for managing
errors and trace messages. This is still work in progress! 

This file declares a class ~RTFlag~ (Runtime Flag) and a preprocessor Macro
~LOGMSG~. It can be used to identify a bool value with a std::string constant.
The value for a given flag name is true, when it appears in the file
"SecondoConfig.ini"[1] in the list of the values for the key RTFlag. It can be
used for trace messages or runtime configuration. This is good for testing or
comparing different implementations without recompilation. The alternative of
defining and reading in new keys in "SecondoConfig.ini" is much more
complicated.  The macro  or the class are used as presented below. 

----LOGMSG("MyFlagName", cerr << "variable x:" << x << std::endl;)

    if ( RTFlag::isActive("MyFlagName") ) {
    
    ... code which should only be executed in this case

    } 
----

All flags should be documented in the configuration file. However, the
mechanism is quite simple, take care to use the same std::string constants
in your code and in the configuration file otherwise you will get in trouble. 

Sending information to ~cout~ or ~cerr~ is not a good idea, since in a client
server setup these information will not transfered to the client. Hence a
global message object ~cmsg~ of class ~CMsg~ will be used as transmitter for
messages. There are four methods which return the reference to an std::ostream
object.  Additonally, you can easily send data to a file. Currently, the
messages are not send to a client via the socket communication but this can
easily be integrated later.

Here are some examples how to use the interface:

---- cmsg.info() << "Non critical information send to cout" << std::endl;
     cmsg.send();
     cmsg.warning() << "More critical information send to cout" << std::endl;
     cmsg.send();
     cmsg.error() << "Error information send to cerr" << std::endl;
     cmsg.send();
     cmsg.file() << "Information send to the file secondo.log" << std::endl;
     cmsg.send();
     cmsg.file("my-logfile.log") << "Information send to my-logfile.log" 
              << std::endl;
----

Before changing the output channel it is important to clear the messsage buffer
with the ~send~ method.

*/


#ifndef CLASS_LOGMSG_H
#define CLASS_LOGMSG_H

#ifndef LOGMSG_OFF
#define LOGMSG(a, b) if ( RTFlag::isActive(a) ) { b }
#endif

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

extern std::ostream* traceOS;

class RTFlag {

public:

  static void initByString( const std::string& keyList );

  static void showActiveFlags(std::ostream& os);

  static bool isActive( const std::string& key );

  static void setFlag( const std::string& key, const bool value );

  static bool empty() {
     return flagMap.empty();
  }

private:

  RTFlag(){}
  ~RTFlag(){}
  static std::map<std::string,bool> flagMap;

};


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
  static std::string message;

public:
  static bool FreezeMessage;
  static bool TypeMapError;
  static void Reset() { TypeMapError=false; message=""; }
  static void ReportError(const std::string msg);
  static void ReportError(const char* msg);
  static void GetErrorMessage(std::string& msg);
};


class CMsg {

public:
  
  CMsg();
  ~CMsg(); 

  std::ostream& file(); 
  std::ostream& file(const std::string& fileName); 

  // condintinal output of messages
  std::ostream& info(const std::string& key);

  inline std::ostream& info()    { stdOutput = 1; return buffer; }
  inline std::ostream& warning() { stdOutput = 1; return buffer; }
  inline std::ostream& error()   { stdOutput = 2; return buffer; } 
  
  // More specific error channels
  void typeError(const std::string& msg)   { ErrorReporter::ReportError(msg);}
  void inFunError(const std::string& msg)  { error() << "InFun: " << msg 
                                                     << std::endl; }
  void otherError(const std::string& msg)  { error() << "Other: " << msg 
                                                     << std::endl; }

/*
 
The send method will flush the stored messages to the underlying stream.  In
the future it may also send the message to clients using socket communication.

*/   
  void send();

/*
Retrieving and cleaning stored errors.

*/  
  std::string getErrorMsg();

  void resetErrors();

private:

  void init();
  int stdOutput;
  std::ofstream* fp;
  std::stringstream buffer;
  std::stringstream allErrors;
  std::stringstream devnull;
  std::string logFileStr;
  std::string prefix;
  std::map<std::string,std::ofstream*> files;
  
};

// defined in cmsg.cpp
extern CMsg cmsg;



#endif
