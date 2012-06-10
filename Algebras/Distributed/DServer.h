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
Contains definitions of DServer, DServerManager, DServerExecutor
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
  : m_cmd(NULL)
  , m_server(NULL)
  , m_cbworker(NULL)
  , m_error(false) {}

/*
2.2 Public Enumeration

denotes the type of command

*/
public:

  enum CmdType { DS_CMD_NONE = 0,  // undefined
                 DS_CMD_WRITE,     // writes an element to the worker
                 DS_CMD_DELETE,    // deletes an element on the worker
                 DS_CMD_OPEN_WRITE_REL, // opens a relation on the worker to
                                        // add elements
                 DS_CMD_WRITE_REL, // writes a singel tuple to a relation 
                                   // on the worker
                 DS_CMD_CLOSE_WRITE_REL, // closes a relation on the worker
                 DS_CMD_READ_REL,  // reads a tuple from a relation on
                                   // the worker and puts it into a 
                                   // relation on the server
                 DS_CMD_READ_TB_REL,    // reads a tuple from a relation on
                                        // the worker and puts it into a 
                                        // tuplebuffer on the server
                 
  };

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
  class RemoteCommand
  {
  public:
   
/*
2.5.1 Constructor

  * CmdType inCmdType - command type description
  
  * const list[<]int[>][ast] inDarrayIndex - list or darray indexes

  * vector [<]Word[>][ast] inElements - list of elements 

  * vector [<]string[>][ast] inFromNames - list of source darray names

*/


    RemoteCommand(CmdType inCmdType,
                  const vector<int>* inDarrayIndex,
                  vector<Word>* inElements,
                  vector<string>* inFromNames)

      : m_cmdType( inCmdType )
      , m_elements( inElements )
    {
      if (inDarrayIndex != 0)
        m_darrayIndex = *inDarrayIndex;

      if (inFromNames != 0)
        m_fromNames = *inFromNames;
    }
/*
2.5.2 Destructor

*/
    virtual ~RemoteCommand() {}

/*
2.5.3 Getter Methods

*/
    CmdType getCmdType() const { return m_cmdType; }
    vector<int>* getDArrayIndex() { return &m_darrayIndex; }
    vector<Word>* getElements() const { return m_elements; }
    const vector<string>& getFromNames() const { return m_fromNames; }

/*
2.5.4 Private Section

*/
  private:
    // no copy and paste!
    RemoteCommand(const RemoteCommand&) {} 
    RemoteCommand() 
      : m_cmdType( DS_CMD_NONE )
      , m_elements( NULL ) {}

    // members
    CmdType m_cmdType;
    vector<int> m_darrayIndex;
    vector<Word>* m_elements;
    vector<string> m_fromNames;
/*
2.5.4 End of Class

*/
  };

/*
2.6 Friend declarations

2.6.1 Operator ~ostream[&] [<][<]~

*/
  friend ostream& operator << (ostream&, RemoteCommand&) ;

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
2.8 Running Commands

2.8.1 Method ~void setCmd~

creates a ~RemoteCommand~ obecjt and sets the parameters 
for a specific command

  * CmdType inCmdType - the type specifier of the command

  * const list[<]int[>][ast] inIndex - list of darray indexes to be worked on

*/
  void setCmd(CmdType inCmdType,
              const vector<int>* inIndex, 
              vector<Word>* inElements = 0,
              vector<string>* inFromNames = 0);

/*
2.8.2 Method ~void setCommand~

sets the local pointer to an already constructed ~RemoteCommand~ object

*/
  void setCmd(RemoteCommand* rc) { m_cmd = rc; }

/*
2.8.2 Method ~void run~

runs a command on a worker. Command and parameters are specified in
a ~RemoteCommand~ object.

*/
  virtual void run();
    
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
  int getNumChilds() const { return m_numChilds;}

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
  const string& getName() const { return name; }

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
2.11 Callback Communication

2.11.1 Method ~void saveWorkerCallBackConnection~

stores a callback communication for later reuse

  * Socket[ast] - the callback communication socket

*/
  void  saveWorkerCallBackConnection(Socket *inCBWorker)
  {
    assert(m_cbworker == NULL);
    m_cbworker = inCBWorker;
  }

/*
2.11.2 Method ~Socket[ast] getSavedWorkerCBConnection~

  * returns Socket[ast] - the callback communication socket

*/  
  Socket* getSavedWorkerCBConnection()
  { 
    assert(m_cbworker != NULL);
    return m_cbworker;
  }

/*
2.11.3 Method ~void closeSavedWorkerCBConnection~

closes the saved callback communication socket

*/  
  void closeSavedWorkerCBConnection();

/*
2.12 Command Status Flags

for the command processing two status flags are used:

  * RelOpen: indicates, if a relation write operation is ongoing

  * ShuffleOpen: indicates, if a dshuffle operation is in process

2.12.1 Methods ~RelOpen~

*/
  bool isRelOpen() const { return m_rel_open; }
  void setRelOpen() { m_rel_open = true; }
  void setRelClose() { m_rel_open = false; }
/*
2.11.3 Methods ~ShuffleOpen~

*/  
  bool isShuffleOpen() const { return m_shuffle_open; }
  void setShuffleOpen() { m_shuffle_open = true; }
  void setShuffleClose() { m_shuffle_open = false; }

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

  string name;   // name of the SECONDO object at the worker

  ListExpr m_type;  // darray type in nested list format
  string m_typeStr; // darray type in nested list format 
                    // in string representation

  RemoteCommand* m_cmd; // pointer to the command object 
                        // (includes paramaeters)


  Socket* m_server; // stored TCP/IP connection to the worker

  // saves a TCP/IP connection for later reuse
  Socket* m_cbworker; 
                                                
  // multiplying worker instances
  vector<DServer*> m_childs;
  int m_numChilds;
                 
  // command operation flags
  bool m_rel_open;
  bool m_shuffle_open;
   
  // error handling
  string m_errorText;
  bool m_error;
/*
2.15 End of Class

*/
};
       
/*
2.16 Operator ~ostream[&] [<][<]~

ostream operator for the internal class ~RemoteCommand~

  * ostream[&] - the output stream

  * RemoteCommand[&] - reference to the ~RemoteCommand~ object

  * returns ostream[&] - the output stream

*/         
ostream& operator << (ostream &out, DServer::RemoteCommand& rc);

#endif // H_DSERVER_H
