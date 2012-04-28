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
[1] Class DServerCmd Definition


\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmd~ is a base class for classes, which implement
commands to be run on each index of a Darray on a worker.
For this purpose this class provides the communication and the 
threading functionality. It also stores needed parameters for a command.

*/

/*
1 Preliminaries

1.1 Defines

*/

#ifndef H_DSERVERCMD_H
#define H_DSERVERCMD_H
/*
1.2 Debug Output

uncomment the following line, if debug output should
be written to stdout

*/
//#define DS_CMD_DEBUG 1

/*
1.3 Includes

*/

#include "DServerThreadRunner.h"
#include "DServerCmdWorkerComm.h"

/*
2 Class ~DServerCmd~

Derives from the class ~DServerCmdWorkerCommunication~ to handele
the communicatoin with the remote SECONDO instance at the worker.
To be used in threads, it also derived from the class ~ZThread::Runnable~.

  * derives from the class ~DServerThreadRunner~

  * derives from the class ~DServerCmdWorkerCommunication~

*/


class DServerCmd 
  : public DServerThreadRunner
  , public DServerCmdWorkerCommunication

{

/*
2.2 Private Default Constructor

  * may not be used!

*/

  DServerCmd()
    : DServerThreadRunner(NULL, -1)
    , DServerCmdWorkerCommunication(NULL) {}


/* 
2.3 Public Enumeration 

each command should be listed here.

  *  enum CmdType - distinguish different commands

*/

public:
  enum CmdType { DS_CMD_NONE = 0,  // undefined
                 DS_CMD_WRITE,     // writes an atomic element to the worker
                 DS_CMD_READ,      // reads an atomic element from the worker
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
                 
  };
/*
2.2 Constructor

  * DServer[ast] inWorker - pointer to the DServer class, 
    representing the worker

  * int inIndex - Darray index to specify the data on the worker

*/

  DServerCmd (CmdType inType, DServer * inWorker, int inIndex) 
    : DServerThreadRunner(inWorker, inIndex)
    , DServerCmdWorkerCommunication(inWorker)
    , m_cmdType(inType){}

/*
2.3 Destructor

*/

  virtual ~DServerCmd() { }

/*
2.7 Getter Methods

2.7.1 Method ~CmdType getCmdType~

   * returns CmdType - the command type of this object

*/
  CmdType getCmdType() const { return m_cmdType; }

/*
2.10 Private Section

*/
private:
/*
2.10.1 Private Methods

*/

// n/a

/*
2.10.2 Private Members

*/

  CmdType m_cmdType;

/*
2.11 End of Class

*/
};

#endif // H_DSERVERCMD_H
