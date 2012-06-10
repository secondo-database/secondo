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
[1] Class DSecondoMonitorCommunication Implementation


\begin{center}
June 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DSecondoMonitorCommunication~ handles the direct communication
with an SECONDO Monitor instance at a remote host. It is used
to initiate a running SECONDO instance at the remote host and execute
commands at this instance. 

*/

/*
1 Preliminaries

1.1 Includes

*/

#include "DSecondoMonitorComm.h"
#include "SocketIO.h"

bool
DSecondoMonitorCommunication::openConnection()
{
  if (m_server != NULL)
    return false;

  m_server = Socket::Connect( getHostName(), getPortNr(), 
                              Socket::SockGlobalDomain,
                              5, // # of trys
                              1); // time in sec between trys

  if(m_server == 0 || !m_server->IsOk())
    {      
      string errMsg = "Cannot connect to SECONDO Monitor on " + 
        getPortNr() + "@" + getHostName();

      if (m_server != 0)
        {
          errMsg += " : " + m_server -> GetErrorText();
        }
      setErrorText(errMsg);
      return false;
    }
  
  setStream(m_server->GetSocketStream());

  string line;
  if (!receiveIOS(line))
    {
      setErrorText("Cannot access SECONDO Monitor on " + getPortNr() +
                   "@" + getHostName()); 
      return false;
    }

  if (line != "<SecondoOk/>")
    {
      setErrorText("Cannot access SECONDO Monitor on " + getPortNr() +
                   "@" + getHostName() + 
                   "\n Expected '<SecondoOk/> but got '" +
                   line + "'"); 
      return false;
    }

  // send '<Connect>'
  if (!sendIOS("<Connect>", false)) // no ack
    {
      setErrorText("Cannot access SECONDO Monitor on " + getPortNr() +
                   "@" + getHostName() + "\nFailed to send '<Connect>'"); 
      return false;
    }
  
  // send 'user'
  if (!sendIOS("", false)) // no ack
    {
      setErrorText("Cannot access SECONDO Monitor on " + getPortNr() +
                   "@" + getHostName() + "\nFailed to send 'user'"); 
      return false;
    }

  // send 'password'
  if (!sendIOS("", false)) // no ack
    {
      setErrorText("Cannot access SECONDO Monitor on " + getPortNr() +
                   "@" + getHostName() + "\nFailed to send 'password'"); 
      return false;
    }
  
  // send '</Connect>'
  if (!sendIOS("</Connect>", false)) // no ack
    {
      setErrorText("Cannot access SECONDO Monitor on " + getPortNr() +
        "@" + getHostName() + "\nFailed to send '</Connect>''"); 
      return false;
    }

  // expecting answer '<SecondoIntro>'
  if (!receiveIOS(line))
    {
      setErrorText("Lost connection to SECONDO Monitor on " + getPortNr() +
                   "@" + getHostName()); 
      return false;
    }

  if (line != "<SecondoIntro>")
    {
      setErrorText("Cannot access SECONDO Monitor on " + getPortNr() +
                   "@" + getHostName() + 
                   "\n Expected '<SecondoIntro> but got '" +
                   line + "'"); 
      return false;
    }
      
  do
    {
       // reading answer
      if (!receiveIOS(line))
        {
          setErrorText("Lost connection to SECONDO Monitor on " + getPortNr() +
                   "@" + getHostName()); 
          return false;
        }
              
    }  while(line != "</SecondoIntro>");
    
  
  if (!(m_server -> IsOk()))
    { 
      setErrorText("Cannot Connect to Worker");
      return false;
    } // if (!(m_server -> IsOk()))

  return true;

}

void
DSecondoMonitorCommunication::closeConnection()
{
  if (m_server != NULL)
    {
      if (m_server -> IsOk())
        sendIOS("<Disconnect/>", false);
      m_server -> Close();
      delete m_server;
    }
  m_server = NULL;
}
