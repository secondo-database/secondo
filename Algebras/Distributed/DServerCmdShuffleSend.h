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
[1] Class DServerCmdShuffleSend Definition

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmdShuffleSend~ is responsible to initiate the sender
functionality on a server. It creates for each new index a tuplequeue,
which runs in its own thread.
When reading the relation, each tuple is assigned a new index and put
in the corresponding tuple queue. The receiver function connects to the
appropriate tuple queue and the tuple is send to the receiver.

The class ~DServerCmdShuffleSendParam~ holds the necessary parameters
for this command.

*/
/*
1 Preliminaries

1.1 Defines

*/
#ifndef H_DSERVERCMDSHUFFLESEND_H
#define H_DSERVERCMDSHUFFLESEND_H
/*
1.2 Includes

*/
#include "DServerCmd.h"

/*
2 Class ~DServerCmdShuffleSendParam~

This class contains the parameters for the class ~DServerCmdShuffleSend~.

  * derives from the class ~DServerParam~

Provided Parameters:

  * string m[_]sendFunc - the function to calculate the new darray index

  * int m[_]basePortNr - the base port for sending.

*/

class DServerCmdShuffleSendParam : public DServerParam
{
/*
2.1 Private Default Constructor

  * may not be used!

*/
  DServerCmdShuffleSendParam() {}

/*
2.1 Constructor

  * const string[&] inSendFunc - the sender function

  * int inBasePortNr - base port number

*/
public:
  DServerCmdShuffleSendParam(const string& inSendFunc,
                             int inBasePortNr)
  : DServerParam()
  , m_sendFunc(inSendFunc)
  , m_basePortNr(inBasePortNr)
  {}

/*
2.3 Copy - Constructor

*/
  DServerCmdShuffleSendParam(const DServerCmdShuffleSendParam &inP)
    : DServerParam(inP)
    , m_sendFunc(inP.m_sendFunc)
    , m_basePortNr(inP.m_basePortNr) {}
/*
2.4 Destructor

*/
  virtual ~DServerCmdShuffleSendParam() {}
/*
2.5 Getter Methods

2.5.1 Method ~const string[&] getSendFunc const~

  * returns const string[&] - the function to calculate the new darray index

*/
  const string& getSendFunc() const { return m_sendFunc; }
/*
2.5.2 Method ~int getBasePortNr const~

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
  string m_sendFunc;
  int m_basePortNr;
/*
2.7 End of Class

*/
};

/*
3 Class ~DServerCmdShuffleSend~

The class ~DServerCmdShuffleSend~ provides the run method to 
send out a tuple stream to a new darray. The new indexes are
calculated using the send - function.

  * derives from the class ~DServerCmd~

*/


class DServerCmdShuffleSend : public DServerCmd
{

public:
/*
3.1 Constructor

*/

  DServerCmdShuffleSend(DServer *inWorker,
                        int inSenderIdx)
    : DServerCmd(DServerCmd::DS_CMD_OPEN_SHUFFLE_SEND,
                 inWorker, inSenderIdx) 
  {  }

/*
3.2 Destructor

*/

  virtual ~DServerCmdShuffleSend() {}


/*
3.3 Getter Methods

3.3.1 Method ~const string[&] getSendFunc const~

  * returns const string[&] - the send function to calculate the 
new darray index

*/
  const string& getSendFunc() const
  {
    const DServerCmdShuffleSendParam *p = 
      DServerCmd::getParam<DServerCmdShuffleSendParam>() ;
    return p -> getSendFunc();
  }

/*
3.3.2 Method ~int getBasePortNr const~

  * returns int - the base port number

*/
  int getBasePortNr() const
  {
    const DServerCmdShuffleSendParam *p = 
      DServerCmd::getParam<DServerCmdShuffleSendParam>() ;
    return p -> getBasePortNr();
  }

/*
3.3.3 Method ~string getInfo const~

  * returns string - an infromation string

*/
  string getInfo() const
  {
    string port = int2Str((getBasePortNr()+getIndex()));
    string dIndexFunction = 
      stringutils::replaceAll(getSendFunc(), "d_idx", "int " + getIndexStr());

    return string("Suffle Send: cmd: (query (d_send_shuffle (feed r" + 
                  getWorker() -> getName() + getIndexStr() + 
                  ") (fun (tuple1 TUPLE) " + dIndexFunction + ") " +
                  getWorker() -> getMasterHostIP_()  + " p" + port + ") ");
  }

/*
3.4 Running

3.4.1 Method ~void run~

*/
  void run();

/*
3.5 Private Section

*/
private:

/*
3.5.1 Private Methods

*/
// n/a

/*
3.5.2 Private Members

*/
// n/a
/*
3.6 End of Class

*/
};

#endif // H_DSERVERCMDSHUFFLESEND_H
