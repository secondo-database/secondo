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
[1] Class DServerCmdShuffleRec Definition

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmdShuffleRec~ is responsible to initiate
the receiver functionality on the workers for the Shuffle command.
Therefore it starts the ``d[_]receive[_]shuffle'' command at a
SECONDO instance on the worker. It sets up a callback communication
with the typemapping function of this command, transfers the
information about host and ports from the shuffle-sender workers.

The class ~DServerCmdShuffleRecParam~ holds the necessary parameters
for this command.

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
2 Class ~DServerCmdShuffleRecParam~

This class contains the parameters for the class ~DServerCmdShuffleRec~.

  * derived from the class ~DServerParam~

Provided Parameters:

  * int m[_]basePortNr - the base port for sending.

*/

class DServerCmdShuffleRecParam : public DServerParam
{
/*
2.1 Private Default Constructor

  * may not be used!

*/
  DServerCmdShuffleRecParam()  {}

/*
2.1 Constructor

  * int inBasePortNr - base port number

*/
public:
  DServerCmdShuffleRecParam(int inBasePortNr)
  : DServerParam() 
  , m_basePortNr(inBasePortNr)
  {}

/*
2.3 Copy - Constructor

*/
  DServerCmdShuffleRecParam(const DServerCmdShuffleRecParam &inP)
    : DServerParam(inP)
    , m_basePortNr(inP.m_basePortNr) {}

/*
2.4 Destructor

*/
  virtual ~DServerCmdShuffleRecParam() {}
/*
2.5 Getter Methods

2.5.1 Method ~int getBasePortNr const~

  * returns int - the base port

*/
  int getBasePortNr() const { return m_basePortNr; }

/*
2.6 Private Section

*/
private:
/*
2.6.1 Private Methods

*/
// n/a

/*
2.6.1 Private Members

*/
  int m_basePortNr;
/*
2.7 End of Class

*/
};

/*
3 Class DServerCmdShuffleRec

The class ~DServerCmdShuffleRec~ is responsible to create the local
darray storage on a worker for a specific index. For this it initates
the d[_]receive[_]shuffle command on the worker, connects to all
instances of the d[_]send[_]shuffle command with the corresponding
index, receivs data and stores it in the local database.

Necessary parameters are provide by the class ~DServerCmdShuffleRecParam~.

  * derives from the class ~DServerCmd~

*/


class DServerCmdShuffleRec : public DServerCmd
{
/*

3.1 Private default Constructor

  * inherited from the class ~DServerCmd~

  * may not be used!

*/


/*

3.2 Constructor

  * DServer[ast] inWorker - pointer to the DServer class, 
    representing the worker

  * int inIndex - Darray index of the receiving worker

*/

public:
  DServerCmdShuffleRec(DServer *inWorker,
                       int inIndex)
    : DServerCmd(DServerCmd::DS_CMD_OPEN_SHUFFLE_REC,
                 inWorker, inIndex) { }

/*

3.2 Destructor

*/

  virtual ~DServerCmdShuffleRec() {}

/*
3.5 Getter Methods

3.5.1 Method ~int getBasePortNr const~

  * returns int - the base port number

*/
  int getBasePortNr() const
  {
    const DServerCmdShuffleRecParam *p = 
      DServerCmd::getParam<DServerCmdShuffleRecParam>() ;
    return p -> getBasePortNr();
  }

/*
3.5.2 Method ~string getInfo const~

  * returns string - an infromation string

*/
  string getInfo() const
  {
    string port = int2Str((getBasePortNr()+getIndex()));
    return string("Suffle REC: cmd: let r" + getWorker() -> getName() + 
                  getIndexStr() + 
    " = " + "d_receive_shuffle(" + 
    getWorker() -> getMasterHostIP_()  + ",p" + port + ")");
  }

/*
3.6 Running

3.6.1 Method ~void run~

*/
  void run();
  
/*

2.5 Private Section

*/
private:

/*

2.5.1 Private Methods

*/
// n/a

/*

2.5.2 Private Members

*/
  // n/a
/*

2.6 End of Class

*/


};

#endif // H_DSERVERCMDSHUFFLEREC_H
