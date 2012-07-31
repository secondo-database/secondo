/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[&] [\&]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]
//[ast] [\ensuremath{\ast}]

*/

/*
[1] Class DServer Definition

\begin{center}
April 2012 Thomas Achmann

November 2010 Tobias Timmerscheidt
\end{center}

[TOC]

0 Description

0.1 April 2012

Class ~DServer~ exists now in its own .h and .cpp file. Added many
getter methods. Changed the command description from string to enum.
Added an interanl class to represent the command and its parameters.

0.2 November 2010

Header-File for Remote.cpp
Contains definitions of DServer, DServerManager
RelationWriter and DServerCreator

1 Preliminaries

1.1 Defines

*/
#ifndef H_DSERVER_H
#define H_DSERVER_H

/*
1.2 Includes

*/
#include "StandardTypes.h"

/*
1.3 Forward Declarations

*/
class Socket;

/*
2 Class ~DServer~

This class ~DServer~ represents one worker as SECONDO instance. 

*/
class DServer
{
/*
2.1 Private Default Constructor

  * may not be used! (internal usage only)

*/
  DServer() 
  : m_server(NULL)
  , m_error(false) {}

public:

/*
2.3 Constructor

  * const string[&] inHostName - host name, where the worker runs

  * int inPortNumber - port number, where the worker listens to

  * string inName - name of the local data at the worker 

  * ListExpr inType 

*/
  DServer(const string& inHostName,
          int inPortNumber,
          string inName, // must not be 'const string&' !
          ListExpr inType);
/*
2.4 Destructor

*/
  virtual ~DServer() {}
/*
2.5 Static Variables

*/
  static Word ms_emptyWord;

/*
2.5 Internal class ~RemoteCommand~

represents the parameters for command, which is run on a worker.

*/
public:

/*

2.6 Setup the Worker

2.6.1 Method ~bool connectToWorker~

connects to a SECONDO instance at a worker host

  * returns bool - true: success; 

*/
  bool connectToWorker();

/*
2.6.2 Method ~bool Multiply~

multiplys the SECONDO instances at the worker. This is used to 
store data of multiple darray indexes at one worker

  * int count - number of additional instances
 
  * returns bool - true: success

*/
  bool Multiply(int count);

/*
2.6.3 Method ~bool checkServer~

performs a check on the worker

  * bool writeError - writes an error to stderr
 
  * returns bool - true: success

*/
  bool checkServer(bool writeError) const;

/*
2.7 Stopping the Worker

2.7.1 Method ~void DestroyChilds~

stops workers created by the ~Multiply~ method

*/
  void DestroyChilds();
/*
2.7.2 Method ~void Terminate~

stops the SECONDO instance on the worker

*/
  void Terminate();


/*
2.9 Error Handling

2.9.1 Method ~void setErrorText~

also sets internal error flag to true

  * string inErrText - the error message

*/         
  void setErrorText(string inErrText)
  { 
    m_error = true;
    m_errorText = inErrText; 
  }

/*
2.9.2 Method ~bool hasError~

  * returns bool - true: if error flag is set

*/         
  bool hasError() const { return m_error; }

/*
2.9.3 Method ~const string[&] getErrorText~

  * returns const string[&] - the error text

*/    
  const string& getErrorText() const { return m_errorText; }

/*
2.10 Getter Methods

2.10.1 Method ~Socket[ast] getServer~

returns a pointer to the TCP/IP connection of the worker

  * returns Socket[ast] - the socket connection

*/
  Socket *getServer() { return m_server; }

/*
2.10.2 Method ~int getNumChilds const~

returns the number of child workers created by the ~multiply~ method.

  * returns int - the number of child workers

*/
  int getNumChilds() const { return m_childs.size();}

/*
2.10.3 Method ~const vector[<]DServer[ast][>][&] getChilds const~

returns the child workers created by the ~multiply~ method.

  * returns const vector[<]DServer[ast][>][&] - the child workers

*/
  const vector<DServer*> & getChilds() const { return m_childs; }
    
/*
2.10.4 Method ~const string[&] getServerHostName const~

  * returns const string[&] - the host name where the worker is run

*/  
  const string& getServerHostName() const { return m_host; }
      
/*
2.10.5 Method ~int getServerPort const~

  * returns int - the port number where the worker is listening

*/  
  int getServerPort() const { return m_port; }

/*
2.10.6 Method ~string getServerPortStr const~

  * returns string - the port number where the worker is listening (as string)

*/  
  string getServerPortStr() const { return int2Str(m_port); }

/*
2.10.7 Method ~ListExpr getTType const~

  * returns ListExpr - the type of the darray in nested list format

*/  

  ListExpr getTType() const { return m_type; }
/*
2.10.8 Method ~const string[&] getTTypeStr const~

  * returns const string[&] - the type of the darray as string 
    representation of the nested list format

*/  
  const string& getTTypeStr() const { return m_typeStr; }

/*
2.10.9 Method ~const string[&] getName const~

  * returns const string[&] - the name of darray data stored at the worker

*/  
  const string& getName() const { return m_name; }

/*
2.10.10 Method ~const string[&] getMasterHostIP const~

  * returns const string[&] - the host TCP/IP adress of the master server
  
  * format: xxx.xxx.xxx.xxx

*/  
  const string& getMasterHostIP() const;

/*
2.10.11 Method ~const string[&] getMasterHostIP[_] const~

  * returns const string[&] - the host TCP/IP adress of the master server
  
  * format: hxxx[_]xxx[_]xxx[_]xxx

*/  
  const string& getMasterHostIP_() const;

/*
2.13 Print Output

2.13.1 Method ~void print~

*/  
  void print() const;


/*
2.14 Private Section

*/
private:
/*
2.14.1 Private Methods

*/
// n/a

/*
2.14.2 Private Members

*/
  string m_host; // host name of the worker
  int m_port;    // port number, the worker listens to

  string m_name;   // name of the SECONDO object at the worker

  ListExpr m_type;  // darray type in nested list format
  string m_typeStr; // darray type in nested list format 
    
  Socket* m_server; // stored TCP/IP connection to the worker
                    
  // multiplying worker instances
  vector<DServer*> m_childs;
        
  // error handling
  string m_errorText;
  bool m_error;
/*
2.15 End of Class

*/
};
       

#endif // H_DSERVER_H
