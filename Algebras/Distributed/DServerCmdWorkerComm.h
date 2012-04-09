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
[1] DServerCmdWorkerCommunication

\begin{center}
March 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmdWorkerCommunication~ is a helper class for
the DSeverCmd class. It implements the communication functinonality
with a SECONDO instance at the worker. 
It derives from the class DServer Communication,
which provides basic functionality.

*/

/*

1 Preliminaries

1.1 Defines

*/

#if 0 // unused until all is checked in!

#ifndef H_DSERVERCMDWORKERCOMM_H
#define H_DSERVERCMDWORKERCOMM_H
/*

1.2 Debug output

uncomment the following line, if debug output should
be written to stdout

*/
//#define DS_CMD_WORKER_COMM 1
/*

1.3 Includes

*/
#include "DServerCmdCommunication.h"


/*

2 Class ~DServerCmdWorkerCommunication~

*/


class DServerCmdWorkerCommunication :
  public DServerCmdCommunication
{
/*

2.2 Private default constructor

  * may not be used!

*/
  DServerCmdWorkerCommunication() {}
/*

2.3 Constructor

  * DServer[ast] inWorker - pointer to the worker socket

*/
public:
  DServerCmdWorkerCommunication(DServer *inWorker)
    :DServerCmdCommunication(/*"DS_CMD_WORKER"*/)
    , m_worker (inWorker)
    , m_workerIoStrOpen(false) {}
/*

2.4 Destructor

*/
  virtual ~DServerCmdWorkerCommunication() {}

/*
  
2.5 Worker

2.5.1 Method ~bool checkWorkerAvailable~

  * returns true, if worker is running
  
*/

  bool checkWorkerAvailable() const;

/*

2.5.2 Method ~DServer[ast] getWorker~

returns a pointer to the worker

*/
  DServer* getWorker() { return m_worker; }

/*

2.6 Opening socket communication

2.6.1 Method ~bool startSocketCommunication~

  * returns true - success

*/

  bool startWorkerStreamCommunication()
  {
    if (m_workerIoStrOpen)
      {
        m_worker -> 
          setErrorText("Communication to worker already opened!");
        return false;
      }

    if (m_worker == NULL)
      {
        m_worker -> setErrorText("No worker assigned yet!");
        return false;
      }

    if (m_worker -> getServer() == 0)
      {
        m_worker -> setErrorText("No server assigned yet!");
        return false;
      }

    if (!setStream(m_worker -> getServer() -> GetSocketStream()))
      { 
         m_worker -> 
           setErrorText("Could not initiate communication to worker!");
         return false;
      }

    m_workerIoStrOpen = true;
    return true;
  }

/*

2.7 Closing socket communication

2.7.1 Method ~bool closeWorkerSocketCommunication~

*/

  bool closeWorkerStreamCommunication()
  {
    if (!m_workerIoStrOpen)
      {
        cout << "ERROR: CLOSING WORKER connection "
             << m_worker -> getServerHostName() << ":"
             << m_worker -> getServerPort()
             << " : no stream opened!" << endl;
        return false;
      }
#ifdef DS_CMD_WORKER_COMM
    cout << "CLOSING WORKER connection "
         << m_worker -> getServerHostName() << ":"
         << m_worker -> getServerPort() << endl;
#endif
    m_worker -> getServer() -> Close();

    return true;
  }

/*

2.8 Sending

2.8.1 Method ~bool sendTagToWorker~

  * const string[&] inTag - tag to be sent (e.g. ``DONE'')

  * returns true - success

*/

  bool sendTagToWorker(const string& inTag)
  {
    return sendIOS("<" + inTag + "/>", true);
  }

/*
2.8.2 Method ~bool sendTextToWorker~

  * const string[&] inTag - tag to be sent (e.g. ``NAME'')

  * const string[&] inTag - text message to be sent

  * returns true - success

*/
  bool sendTextToWorker(const string& inTag,
                        const string& inText)
  {
    return sendIOS("<" + inTag + ">",
                   inText,
                   "<" + inTag + "/>");
  }

/*
2.8.3 Method ~bool sendSecondoCmdToWorker1~

sends a regular command to SECONDO (e.g. ``query 1''

  * const string[&] inCmd - SECONDO command

  * string[&] outError - message in case of error

  * bool expectingAnswer - true: expecting [<]Secondo[>] ... answer

  * returns true - success

*/
  bool sendSecondoCmdToWorker1(const string& inCmd,
                               string& outErr,
                               bool expectingAnswer = true)
  {
    return sendSecondoCmdToWorkerCnt(inCmd, 1, outErr, expectingAnswer);
  }

/*
2.8.4 Method ~bool sendSecondoCmdToWorker0~

sends a command to SECONDO in nested list format(e.g. ``(query 1)''

  * const string[&] inCmd - SECONDO command

  * string[&] outError - message in case of error

  * bool expectingAnswer - true: expecting [<]Secondo[>] ... answer

  * returns true - success

*/
  bool sendSecondoCmdToWorker0(const string& inCmd,
                               string& outErr,
                               bool expectingAnswer = true)
  {
    return sendSecondoCmdToWorkerCnt(inCmd, 0, outErr, expectingAnswer);
  }

/*
2.9 Receiving

2.9.1 Method ~bool receiveLineFromWorker~

receives one line of data from the worker

  * string[&] outLine - message received

  * returns true - success

*/
  bool receiveLineFromWorker(string &outLine)
  {
    return receiveIOS(outLine);
  }

/*

2.10 Private section

*/

private:

/*

2.10.1 Private methods

*/
  bool sendCmdToSecondo(int inCmdType,
                        const string& inCmd)
  {
    return sendSecondoCmd( inCmdType, inCmd);
  }

  bool sendSecondoCmdToWorkerCnt(const string& inCmd,
                                 int inCnt,
                                 string& outErr,
                                 bool expectingAnswer = true)
  {
#ifdef DS_CMD_WORKER_COMM
    cout << "SecondoCmd:" << inCmd << endl;
#endif
    bool ret = sendCmdToSecondo(inCnt, inCmd);

    if (ret)
      {
        outErr = "";
        string line;
        while (expectingAnswer &&
               line.find("</SecondoResponse>") == string::npos &&
               receiveLineFromWorker(line))
          {
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

      }
#ifdef DS_CMD_WORKER_COMM
    if (!ret)
      {
        cout << "--------------------" << endl
             << "GOT ERROR MSG:" << endl << outErr << endl 
             << "--------------------" << endl;
      }
#endif

    if (ret)
      outErr = "";

    return ret;
  }

/*

2.10.2 Private members

*/
  DServer *m_worker;

  bool m_workerIoStrOpen;

/*

2.11 End of class

*/
};

#endif // H_DSERVERCMDWORKERCOMM_H
#endif // if 0
