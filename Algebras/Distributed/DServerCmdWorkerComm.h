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

[1] Class DServerCmdWorkerCommunication Definition

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmdWorkerCommunication~ is a helper class for
the ~DSeverCmd~ class. It implements the communication functinonality
with a SECONDO instance at the worker. It can send and receive a 
SECONDO command.
This class extends the base class ~DServerCommunication~,
which provides basic functionality.
The subclass ~DServerCmdWorkerCommunicationThreaded~ is used for submitting
a SECONDO command in a separate thread.

*/

/*
1 Preliminaries

1.1 Defines

*/
#ifndef H_DSERVERCMDWORKERCOMM_H
#define H_DSERVERCMDWORKERCOMM_H
/*
1.2 Debug Output

uncomment the following line, if debug output should
be written to stdout

*/
//#define DS_CMD_WORKER_COMM 1
/*
1.3 Includes

*/
#include "Remote.h"
#include "DServerCmdCommunication.h"
#include "zthread/ThreadedExecutor.h"

/*
2 Class ~DServerCmdWorkerCommunication~

The class ~DServerCmdWorkerCommunication~ provides the basic functionality
to communicate with a remote SECONDO instance at a worker.

  * derives from class ~DServerCmdCommunication~

*/

class DServerCmdWorkerCommunication :
  public DServerCmdCommunication
{
/*

2.2 Private Default Constructor

  * may not be used!

*/
protected:
  DServerCmdWorkerCommunication()
    : DServerCmdCommunication(/*"DS_CMD_WORKER"*/)
    , m_worker (NULL)
    , m_workerIoStrOpen(false)
    , m_error (false)
    , m_exec (NULL) {}

/*

2.3 Constructor

  * DServer[ast] inWorker - pointer to the worker object

*/
public:
  DServerCmdWorkerCommunication(DServer *inWorker)
    :DServerCmdCommunication(/*"DS_CMD_WORKER"*/)
    , m_worker (inWorker)
    , m_workerIoStrOpen(false)
    , m_error (false)
    , m_exec (NULL) {}
/*
2.4 Destructor

*/
  virtual ~DServerCmdWorkerCommunication() 
  { 
    if (m_exec != NULL)
      {
        //somebody has already finished
        //the execution, but is not interested
        //in the result
        waitForSecondoResultThreaded(); 
      }
    
  } 

/*
2.5 Worker

2.5.1 Method ~bool checkWorkerAvailable~

  * returns true, if worker is running
  
*/
  bool checkWorkerAvailable() const;

/*
2.6 Error handling

2.6.1 Method ~void setErrorText~
sets the internal error state to ~true~ 
and provides an error message

  * const string[&] inErrMsg - the error message
  
*/
  void setCmdErrorText(const string& inErrMsg) 
  {
    m_error = true;
    m_errorText = inErrMsg;
  }
/*

2.6.2 Method ~bool hasCmdError~
returns the error state

  * returns false - no error, true - error occurred
  
*/

  bool hasCmdError() const { return m_error; }


/*
2.6.3 Method ~const string[&] getCmdErrorText~
returns the error string

  * returns const string[&] - the error message
  
*/
  const string& getCmdErrorText() const { return m_errorText; }

/*
2.7 Command Result

2.7.1 Method ~void setCmdResult~
sets the command's result provided by the SECONDO instance

  * const string[&] inResult - the result string
  
*/
  void setCmdResult(const string& inResult) { m_cmdResult = inResult; }


/*

2.7.2 Method ~const string[&] getCmdResult~
returns the result string provided by the SECONDO instance

  * returns const string[&] - the result string
  
*/
  const string& getCmdResult() const { return m_cmdResult; }
 

/*
2.8 Opening Socket Communication

2.8.1 Method ~bool startSocketCommunication~

  * returns true - success

*/
  bool startWorkerStreamCommunication();

/*
2.9 Closing Socket communication

2.9.1 Method ~bool closeWorkerSocketCommunication~

  * returns true - success

*/
  bool closeWorkerStreamCommunication();

/*
2.10 Sending

2.10.1 Method ~bool sendSecondoCmdToWorkerSOS~

sends a regular command to SECONDO (e.g. ``query 1'')
using SOS syntax

  * const string[&] inCmd - SECONDO command

  * returns true - success

*/
  bool sendSecondoCmdToWorkerSOS(const string& inCmd,
                                 bool useThreads = false)
  {
    return sendSecondoCmdToWorker(inCmd, 1, useThreads);
  }

/*
2.10.2 Method ~bool sendSecondoCmdToWorkerNL~

sends a command to SECONDO in nested list format(e.g. ``(query 1)''
using nested list syntax

  * const string[&] inCmd - SECONDO command

  * returns true - success

*/
  bool sendSecondoCmdToWorkerNL(const string& inCmd,
                                bool useThreads = false)
  {
    return sendSecondoCmdToWorker(inCmd, 0, useThreads);
  }


/*
2.10.3 Method ~bool waitForSecondoResultThreaded~
awaits the end of the thread, in case the command
was submitted in a separate thread.

  * returns true - thread has finished, fals - no thread is running

*/
  bool waitForSecondoResultThreaded()
  {
    if (m_exec != NULL)
      {
        m_exec -> wait();
        delete m_exec;
        m_exec = NULL;
        return true;
      }
    return false;
  }


/*
2.11 Receiving

2.11.1 Method ~bool receiveLineFromWorker~

receives one line of data from the worker

  * string[&] outLine - message received

  * returns true - success

*/
  bool receiveLineFromWorker(string &outLine)
  {
    return receiveIOS(outLine);
  }

/*
2.12 Protected Section

*/
protected:

/*
2.12.1 Methode ~waitForSecondoResult~
retrieves the result from the SECONDO instance

  * const string[&] inCmd - the command string for error reporting

  * bool debugOut - flag to write debug output

*/
  bool waitForSecondoResult(const string& inCmd,
                            bool debugOut = false)
  {
    bool ret = true;
    string outErr = "";
    string line;
    while (line.find("</SecondoResponse>") == string::npos &&
           receiveLineFromWorker(line))
      {
        if (debugOut)
          cout << "SECONDO RESULT:" << line << endl;

        if (ret && 
            (line.find("error") != string::npos ||
             line.find("Error") != string::npos||
             line.find("ERROR") != string::npos) )
          {
            ret = false;
          }
        if (line.find("SecondoResponse") == string::npos &&
            line.find("bnl") == string::npos)
          outErr += line + "\n";
        
      }

    setCmdResult(outErr);

    if (!ret)
      {
        outErr = "SECONDO command: '" + inCmd + "'\n" + outErr;
        setCmdErrorText(outErr);
#ifdef DS_CMD_WORKER_COMM
        cout << "--------------------" << endl
             << "GOT ERROR MSG:" << endl << outErr << endl 
             << "--------------------" << endl;
#endif
      }

    return ret;
  }

/*
2.12.2 Method ~void setStreamOpen~
sets internal flag, that stream is open

*/
  void setStreamOpen() { m_workerIoStrOpen = true; }

/*
2.12.3 Method ~void setStreamClose~
sets internal flag, that stream is closed

*/
  void setStreamClose() { m_workerIoStrOpen = false; }

/*
2.13 Private Section

*/

private:

/*
2.13.1 Private Methods

*/
/*
Method ~bool sendSecondoCmdToWorker~
wrapper method to submit the command and awaits 
SECONDO result in no-thread case
 
  * const string[&] inCmd - command string

  * int inFlag - 0:nested list format, 1:regular SOS fromat
  
  * bool useThreads - true: command is started in a separate thread

  * returns: true - success; false - error

*/
  bool sendSecondoCmdToWorker(const string& inCmd,
                              int inFlag,
                              bool useThreads)
  {
   bool ret_val =
     sendSecondoCmdToWorkerThreaded(inCmd, inFlag, useThreads);

   if (ret_val && !useThreads)
     ret_val = waitForSecondoResult(inCmd);

   return ret_val;
  }

/*
Method ~bool sendSecondoCmdToWorkerThreaded~
sends the command to
 
  * const string[&] inCmd - command string

  * int inFlag - 0:nested list format, 1:regular SOS fromat
  
  * bool useThreads - true: command is started in a separate thread

  * returns: true - success; false - error

*/
  bool sendSecondoCmdToWorkerThreaded(const string& inCmd,
                                      int Flag,
                                      bool useThreads);

/*

2.13.2 Private Members

*/
  DServer *m_worker;

  bool m_workerIoStrOpen;

  bool m_error;
  string m_errorText;

  string m_cmdResult;

  ZThread::ThreadedExecutor *m_exec;
  
/*

2.14 End of Class

*/
};


/*
3 Class ~DServerCmdWorkerCommunicationThreaded~

The class ~DServerCmdWorkerCommunicationThreaded~ provides
a run method to submit a SECONDO command in a separate
thread. It uses the command channedl from the issuing
~DServerCmdWorkerCommunication~ object. It also pushes
error messages and the SECONDO result back to the caller.
For running as a thread it provides the ~run~ methode

  * derives from class ~DServerCmdWorkerCommunication~

  * derives from class ~ZThread::Runnable~

*/

class DServerCmdWorkerCommunicationThreaded 
  : protected DServerCmdWorkerCommunication
  , public ZThread::Runnable
{
/*

3.1 Constructor

  * DServerCmdCommunication[ast] inComm - pointer to the calling object

  * const string[&] inCmd - the SECONDO command

  * int inFlag - 0:nested list format, 1:regular SOS fromat

*/
public:
  DServerCmdWorkerCommunicationThreaded(DServerCmdWorkerCommunication *inComm,
                                        const string& inCmd, int inFlag)
    : DServerCmdWorkerCommunication()
    , m_caller(inComm)
    , m_cmd (inCmd)
    , m_flag (inFlag) {}

/*
3.2 Destructor

*/

  virtual ~DServerCmdWorkerCommunicationThreaded() {}

/*
3.3 Method ~void run~

*/
  void run();

/*
3.4 Private Section

*/
private:


/*
3.4.1 Private Methodes

*/
// n/a


/*
3.4.2 Privtae Members

*/
  DServerCmdWorkerCommunication* m_caller;
  string m_cmd;
  int m_flag;


/*
3.5 End of Class

*/
};
#endif // H_DSERVERCMDWORKERCOMM_H
