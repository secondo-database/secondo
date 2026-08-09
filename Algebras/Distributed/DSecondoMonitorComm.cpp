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
#include "SecondoInterfaceCS.h"

bool
DSecondoMonitorCommunication::openConnection()
{
  if (m_interface != NULL)
    return false;

  m_nl = new NestedList();
  m_interface = new SecondoInterfaceCS(true, m_nl, true);
  m_interface->setMaxAttempts(5);
  m_interface->setTimeout(1);

  string errMsg = "";
  if (!m_interface->Initialize("", "", getHostName(), getPortNr(),
                               string(""), string(""), errMsg, true))
    {
      setErrorText("Cannot connect to SECONDO Monitor on " +
                   getPortNr() + "@" + getHostName() + " : " + errMsg);
      delete m_interface;
      m_interface = NULL;
      delete m_nl;
      m_nl = NULL;
      return false;
    }

  return true;
}

void
DSecondoMonitorCommunication::closeConnection()
{
  if (m_interface != NULL)
    {
      m_interface->Terminate();
      delete m_interface;
      m_interface = NULL;
      delete m_nl;
      m_nl = NULL;
    }
}
