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
[1] DServerCmd


\begin{center}
March 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmd~ is the base class to run commands on the 
worker. There exists for each command its own subclass, which
is responsible for running the command.
This class holds common datastructures for all subclasses
and also serves as interface class.

*/

/*

1 Preliminaries

1.1 Defines

*/

#if 0 // unused until all is checked in!

#ifndef H_DSERVERCMD_H
#define H_DSERVERCMD_H
/*

1.2 Debug output

uncomment the following line, if debug output should
be written to stdout

*/
//#define DS_CMD_DEBUG 1

/*

1.3 Includes

*/

#include "Remote.h"
#include "DServerCmdWorkerComm.h"

/*

1.2 Extern defintions

*/

extern string toString_d(int);

/*

2 Class ~DServerCmd~

Derives from the class ~DServerCmdWorkerCommunication~ to handele
the communicatoin with the remote SECONDO instance at the worker.
It is also derived from the class ~ZThread::Runnable~ to be used
in threads.

*/


class DServerCmd 
  : public ZThread::Runnable
  , public DServerCmdWorkerCommunication

{
/*

2.2 Private default constructor

  * may not be used!

*/

  DServerCmd() {}



/* 

2.3 Public enumeration 

  *  enum CmdType - distinguish different commands

*/

public:
  enum CmdType { DS_CMD_NONE = 0,  // undefined
                 DS_CMD_WRITE,     // writes an element to the worker
                 DS_CMD_READ,      // reads an element from the worker
                 DS_CMD_DELETE,    // deletes an element on the worker
                 DS_CMD_COPY,      // copies an element on the worker
                 DS_CMD_EXEC,      // exectues a command on each element on
                                   // the worker
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
                 DS_CMD_OPEN_SHUFFLE_REC,  // opens sockets on the destination
                                        // workers to be ready to receive
                                        // the data for the new DArray
                 DS_CMD_OPEN_SHUFFLE_SEND,  // opens sockets on the destination
                                        // workers to be ready to receive
                                        // the data for the new DArray
                 DS_CMD_SHUFFLE_MULTI_CONN,
                 DS_CMD_SHUFFLE_CLOSE,
                 
  };
/*

2.2 Constructor

  * DServer[ast] inWorker - pointer to the DServer class, 
    representing the worker

  * int inIndex - Darray index of the receiving worker

*/

  DServerCmd (DServer * inWorker, int inIndex) 
    : DServerCmdWorkerCommunication(inWorker)
    , m_elements( NULL )
    , m_cmdType(DS_CMD_NONE)
    , m_index(inIndex) {}

/*

2.3 Destructor

*/

  virtual ~DServerCmd() {}

/*

2.4 Method ~void setCmdType~

  * CmdType inType - the type of command, which is represented by this object

*/

  void setCmdType(CmdType inType) { m_cmdType = inType; }


/*

2.5 Method ~void setParams~

  * const list[<]int[>][ast] inIndex - list of darray indices

  * vector[<]Word[>][ast] inElements - list of ``Word'' objects for the commands
  * vector[<]string[>][ast] inFromNames - list of names

*/

  void setParams(const list<int>* inIndex, 
                 vector<Word>* inElements = 0,
                 vector<string>* inFromNames = 0)
  {
    m_elements = inElements;
    
    if (inIndex != 0)
      m_darrayIndex = *inIndex;

    if (inFromNames != 0)
      m_fromNames = *inFromNames;
  }

/*

 2.6 Method ~void run~

*/

  void run() = 0;

/*

2.7 Getter methods

2.7.1 Method ~CmdType getCmdType~

returns the command type of this object

*/
  CmdType getCmdType() const { return m_cmdType; }

/*
2.7.1 Method ~const list[<]int[>][ast] getDArrayIndex~

returns the list of darray indices

*/
  const list<int>* getDArrayIndex() const { return &m_darrayIndex; }
  int getDArrayIndexFront() 
  { 
    assert (!m_darrayIndex.empty());
    int retVal = m_darrayIndex.front();
    m_darrayIndex.pop_front();
    return retVal;
  }

/*
2.7.2 Method ~vector[<]Word[>][ast] getElements~

returns the list of Word objects

*/
  vector<Word>* getElements() const { return m_elements; }

/*
2.7.3 Method ~vector[<]string[>][&] getFromNames~

returns the list of from names

*/
  const vector<string>& getFromNames() const { return m_fromNames; }

/*
2.7.4 Method ~int getIndex~

returns the Index of this worker

*/
  int getIndex() const { return m_index; }

/*
2.7.4 Method ~string getIndexStr~

returns the Index of this worker in string representation

*/
  string getIndexStr() const { return toString_d(m_index); }

/*

2.8 Error handeluing

2.8.1 Method ~setErrorText~

  * const string[&] inErrTxt - the error msg

*/

  void setErrorText(const string& inErrTxt)
  {
    getWorker() -> setErrorText(inErrTxt);
  }


/*

2.9 Protected section

*/
protected:

/*
2.9.1 Protected members

*/
  CmdType m_cmdType;
  int m_index;
  
/*

2.10 Private section

*/
private:
/*

2.10.1Private methods

*/

// n/a

/*

2.10.2 Private members

*/
  list<int> m_darrayIndex;
  vector<Word>* m_elements;
  vector<string> m_fromNames;
/*

2.11 End of class

*/
};

#endif // H_DSERVERCMD_H
#endif // if 0
