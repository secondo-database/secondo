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
[1] Class DSecondoMonitorCommunication Definition


\begin{center}
June 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DSecondoMonitorCommunication~ handles the direct communication
with an Secondo Monitor instance at a remote host. 

*/

/*
1 Preliminaries

1.1 Defines

*/

#ifndef H_DSECONDOMONITORCOMM_H
#define H_DSECONDOMONITORCOMM_H
/*
1.2 Debug Output

uncomment the following line, if debug output should
be written to stdout

*/
//#define DS_SECMON_DEBUG 1

/*
1.3 Includes

*/

#include "DServerCmdWorkerComm.h"

class Socket;

/*
2 Class ~DSecondoMonitorCommunication~

Derives from the class ~DSecondoMonitorCommunication~ to handele
the communicatoin with the remote SECONDO Monitor instance


  * derives from the class ~DServerCmdWorkerCommunication~

*/


class DSecondoMonitorCommunication
  : public DServerCmdWorkerCommunication

{

/*
2.2 Private Default Constructor

  * may not be used!

*/

  DSecondoMonitorCommunication()
    : DServerCmdWorkerCommunication(NULL)
    , m_server(NULL) {}

/*
2.2 Constructor

  * const string[&] inHostName - host name of the remote computer

  * const string[&] inPortNumber - port number of the remote
SECONDO monitor instance

*/
public:
  DSecondoMonitorCommunication(const string& inHostName, 
                               const string& inPortNumber) 
    : DServerCmdWorkerCommunication(NULL)
    , m_hostName(inHostName)
    , m_portNr(inPortNumber)
    , m_server(NULL) {}

/*
2.3 Destructor

*/

  virtual ~DSecondoMonitorCommunication() { closeConnection(); }

/*
2.4 Getter Methods

2.4.1 Method ~const string[&] getHostName~

   * returns const string[&] - the host name

*/
  const string& getHostName() const { return m_hostName; }

/*
2.4.2 Method ~const string[&] getPortNr~

   * returns const string[&] - the port number

*/
  const string& getPortNr() const { return m_portNr; }
/*
2.5 Error handling

2.5.1 Method ~const string[&] getErrorText~

   * returns const string[&] - the error message

*/  

  const string& getErrorText() const { return m_errorMsg; }
/*
2.6 Connection to SECONDO monitor

2.6.1 Method ~bool openConnection~

   * returns true - success; false - error

*/  

  bool openConnection();

/*
2.10 Private Section

*/
private:
/*
2.10.1 Private Methods

*/
  void setErrorText(const string& inErrMsg) { m_errorMsg = inErrMsg; }

  void closeConnection() ;

/*
2.10.2 Private Members

*/

  string m_hostName;
  string m_portNr;
  Socket* m_server;
  
  string m_errorMsg;

/*
2.11 End of Class

*/
};

#endif // H_DSECONDOMONITORCOMM_H
