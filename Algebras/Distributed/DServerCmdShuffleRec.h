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
[1] DServerCmdShuffleRec

\begin{center}
March 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmdShuffleRec~ is responsible to initiate
the receiver functionality on the workers for the Shuffle command.
Therefore it starts the ``d[_]receive[_]shuffle'' command at a
SECONDO instance on the worker. It sets up a callback communication
with the typemapping function of this command, transfers the
information about host and ports from the shuffle-sender workers.

The class derives from the DServerCmd command for.

*/

/*

1 Preliminaries

1.1 Defines

*/

#ifndef H_DSERVERCMDSHUFFLEREC_H
#define H_DSERVERCMDSHUFFLEREC_H

/*

1.2 Includes

*/

#include "DServerCmd.h"


/*

2 Class DServerCmdShuffleRec

  * derives from the class ~DServerCmd~

*/


class DServerCmdShuffleRec : public DServerCmd
{
/*

2.1 Private default Constructor

may not be used!

*/

  DServerCmdShuffleRec() : DServerCmd(NULL, -1) {}

/*

2.1 Constructor

  * DServer[ast] inWorker - pointer to the DServer class, 
    representing the worker

  * int inIndex - Darray index of the receiving worker

*/

public:
  DServerCmdShuffleRec(DServer *inWorker,
                       int inIndex)
    : DServerCmd(inWorker, inIndex) 
    , m_srcSize(-1)
  { 
    setCmdType(DServerCmd::DS_CMD_OPEN_SHUFFLE_REC);
  }

/*

2.2 Destructor

*/

  virtual ~DServerCmdShuffleRec() {}


/*

2.3 Source worker host names and ports

2.3.1 Method ~void setSourceHostPort~
    
Sets the host name and the port of the source workers
(sender workers)

*/

  void setSourceHostPorts(const vector<string>& inHostNames,
                          const vector<int>& inPortToNrs)
  {
    if (inHostNames.size() == inPortToNrs.size())
      for (unsigned long i = 0; i < inHostNames.size(); ++i)
        setSourceHostPort(inHostNames[i], inPortToNrs[i]);
  }

/*

2.3.2 Method ~unsigned long getSourceWorkerSize~

returns the number of host names incl. ports of the senders

*/

  unsigned long getSourceWorkerSize() const { return m_srcSize; }

/*

2.3.2 Method ~unsigned long getSourceWorkerHost~

returns the host name of a specific index

  * unsigned long i - index of the the host name array

*/
  string getSourceWorkerHost(unsigned long i) const
  { 
    string retVal;
    if (i >= 0 && i < m_srcSize)
      retVal = m_srcHostNames[i];
    return retVal;
  }

/*

2.3.2 Method ~unsigned long getSourceWorkerPort~

returns the port of a specific index

  * unsigned long i - index of the the host name array

*/
  int getSourceWorkerHostToPort(unsigned long i) const
  {
    int retVal = -1;
    if (i >=0 && i < m_srcSize)
      retVal = m_srcHostToPorts[i];
    return retVal;
  }

/*

2.4 Method ~void run~

run method of the thread

*/

  void run();
  
/*

2.5 Private section

*/
private:

/*

2.5.1 Private methods

*/

  void setSourceHostPort(const string& inHostName,
                         int inPortToNr)
  {
    m_srcHostNames.push_back(inHostName);
    m_srcHostToPorts.push_back(inPortToNr);
    m_srcSize = m_srcHostNames.size();
  }

/*

2.5.2 Private members

*/
  vector<string> m_srcHostNames;
  vector<int>    m_srcHostToPorts;
  unsigned long  m_srcSize;
/*

2.6 End of class

*/


};

#endif // H_DSERVERCMDSHUFFLEREC_H
