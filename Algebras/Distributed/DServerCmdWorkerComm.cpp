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

[1] Class DServerCmdWorkerCommunication Implementation

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

Implementation of the class ~DServerCmdWorkerCommunication~


1 Preliminaries

1.1 Includes

*/

#include "DServer.h"
#include "DServerCmdWorkerComm.h"
#include "SocketIO.h"
#include "SecondoInterfaceCS.h"

/*
  
2 Implementation

2.1 Method ~bool checkWorkerAvailable~

  * returns true, if worker is running
  
*/

bool 
DServerCmdWorkerCommunication::checkWorkerAvailable() const
{
  if ( m_worker -> getServer() == 0 ||
       !(m_worker -> getServer() -> isInitialized()))
    {
      return false;
    }

  return true;
}


/*
2.2 Method ~bool startSocketCommunication~

  * returns true - success

*/
bool
DServerCmdWorkerCommunication::startWorkerStreamCommunication()
{
  if (m_workerIoStrOpen)
    {
      setCmdErrorText("Communication to worker already opened!");
      return false;
    }

  if (m_worker == NULL)
    {
      setCmdErrorText("No worker assigned yet!");
      return false;
    }

  if (m_worker -> getServer() == 0)
    {
      setCmdErrorText("No server assigned yet!");
      return false;
    }

  if (!m_worker -> getServer() -> isInitialized())
    {
      setCmdErrorText("Could not initiate communication to worker!");
      return false;
    }

  m_workerIoStrOpen = true;
  return true;
}


/*
2.3  Method ~bool closeWorkerSocketCommunication~

  * returns true - success

*/
bool
DServerCmdWorkerCommunication::closeWorkerStreamCommunication()
{
  return true;

  if (!m_workerIoStrOpen)
    {
      const string errMsg = 
        "Cannot close worker cmd - connection " + 
        m_worker -> getServerHostName() +  ":" +
        int2Str(m_worker -> getServerPort()) + 
        " : no stream opened!";
      setCmdErrorText(errMsg);
      return false;
    }

#ifdef DS_CMD_WORKER_COMM
  std::cout << "CLOSING WORKER connection "
       << m_worker -> getServerHostName() << ":"
       << m_worker -> getServerPort() << std::endl;
#endif

  m_worker -> getServer() -> Terminate();

  return true;
}

/*
Private method ~bool sendSecondoCmdToWorkerThreaded~
sends the command to
 
  * const string[&] inCmd - command string

  * int inFlag - 0:nested list format, 1:regular SOS fromat
  
  * bool useThreads - true: command is started in a separate thread

  * returns: true - success; false - error

*/
bool 
DServerCmdWorkerCommunication::
       sendSecondoCmdToWorkerThreaded(const string& inCmd,
                                      int inFlag,
                                      bool useThreads)
{
#ifdef DS_CMD_WORKER_COMM
  std::cout << (unsigned long)this << "SecondoCmd:"  
       << inFlag << " - "<< inCmd << " as " << std::endl;
  if (useThreads)
    std::cout << " Thread";
  else
    std::cout << " NO Thread";
  std::cout << std::endl;
#endif
  
  bool ret = true;
  if (useThreads)
      {
        DServerCmdWorkerCommunicationThreaded* commThread =
          new DServerCmdWorkerCommunicationThreaded(this,
                                                    m_worker,
                                                    inCmd,
                                                    inFlag);
        assert (m_exec == NULL);
        m_exec = new ZThread::ThreadedExecutor();
        m_exec -> execute(commThread);
      }
  else
    {
      assert (m_exec == NULL);

      ret = runSecondoCmd(inCmd, inFlag);
    }
  
  return ret;
}

/*
3.3 Method ~void run~

*/
void
DServerCmdWorkerCommunicationThreaded::run()
{
  setStreamOpen();

  const bool ret = runSecondoCmd(m_cmd, m_flag);

  if (!ret || hasCmdError())
    m_caller -> setCmdErrorText(getCmdErrorText());

  m_caller -> setCmdResult(getCmdResult());
}

/*
3.4 Method ~bool runSecondoCmd~

*/
bool
DServerCmdWorkerCommunication::runSecondoCmd(const string& inCmd, int inFlag)
{
  SecondoInterfaceCS* si = commInterface();
  NestedList* wnl = commNestedList();
  if (si == 0 || wnl == 0)
    {
      setCmdErrorText("No connection assigned yet!");
      return false;
    }

  ListExpr resultList = wnl -> TheEmptyList();
  int errorCode = 0;
  int errorPos = 0;
  string errorMessage = "";

  si -> Secondo(inCmd, resultList, inFlag, true, false,
                resultList, errorCode, errorPos, errorMessage);

  if (errorCode != 0)
    {
      string outErr = "SECONDO command: '" + inCmd + "'\n" + errorMessage;
      setCmdResult(errorMessage);
      setCmdErrorText(outErr);
      return false;
    }

  // Callers only ever show this or search it for a name, so the printed list
  // is what they want; nothing downstream parses it back.
  setCmdResult(wnl -> ToString(resultList));
  return true;
}
