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
[1] Class DServerCmdShuffleMultiConn Definition

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmdShuffleMultiConn~ is used in sender and 
receiver functionality of the dshuffle command. It transfers 
the host name and port number information to each sender and 
reveiver function. After that it sets a command, which multiplys 
the connections and let the receiver functions connect with the sender 
functions, one for each pair of old and new index. Then it waits until all 
functions have finished, and, in  case of an error, collects the error message 
and sends it to the worker.

The class ~DServerCmdShuffleMultiConnParam~ provides the necessary data for this
function.

1 Preliminaries

1.1 Defines

*/

#ifndef H_DSERVERCMDSHUFFLEMULTIPLECONN_H
#define H_DSERVERCMDSHUFFLEMULTIPLECONN_H
/*
1.2 Includes

*/
#include "DServerThreadRunner.h"

/*
2 Class ~DServerCmdShuffleMultiConnParam~

This class contains the parameters for the class ~DServerCmdShuffleMultiConn~.

  * derived from the class ~DServerParam~

Provided Parameters:

  * enum MultiConnType m[_]type - the type of the function (sender or receiver)
  
  * int m[_]basePortNr - base port number

  * m[_]hostList - list of available host

  * m[_]portList - list of availabel ports

*/

class DServerCmdShuffleMultiConnParam : public DServerParam
{
/*
2.1 Private Default Constructor

  * may not be used!

*/
  DServerCmdShuffleMultiConnParam() {}

/*
2.1 Public Enumeration

denotes the type of this function (sender or receiver)

*/
public:
  enum MultiConnType { DSC_SMC_P_NONE,
                       DSC_SMC_P_SENDER,
                       DSC_SMC_P_RECEIVER,
  };
/*
2.2 Constructor

  * MultiConnType inType - function type

  * int inBasePortNr - base port number

  * const vector[<]vector[<]string[>][>] inHostList - list of available
  host names

  * const vector[<]vector[<]int[>][>] inPortList - list of available 
  port numbers

*/
  DServerCmdShuffleMultiConnParam(MultiConnType inType,
                                  int inBasePortNr,
                                  const vector<vector<string> > &inHostList,
                                  const vector<vector<int> >& inPortList)
  : DServerParam()
  , m_type(inType)
  , m_basePortNr(inBasePortNr)
  {
    copyList<string>(inHostList, m_hostList);
    copyList<int>(inPortList, m_portList);
  }

/*
2.3 Copy - Constructor

*/
  DServerCmdShuffleMultiConnParam(const DServerCmdShuffleMultiConnParam &inP)
    : DServerParam(inP)
    , m_type(inP.m_type) 
    , m_basePortNr(inP.m_basePortNr)
  { 
    copyList<string>(inP.m_hostList, m_hostList);
    copyList<int>(inP.m_portList, m_portList);
  }

/*
2.4 Destructor

*/
  virtual ~DServerCmdShuffleMultiConnParam(){}
/*
2.5 Getter Methods

2.5.1 Method ~const string[&] getHost const~

returns for a certain index combination of the original and the new darray the
corret host name

  * int srvIdx - source darray index

  * int inIdx - new darray index

  * returns const string[&] - the host name

*/
  
  const string& getHost(int srvIdx, unsigned long inIdx) const 
  { return m_hostList[srvIdx][inIdx]; }
/*
2.5.2 Method ~const int getPort const~

returns for a certain index combination of the original and the new darray the
corret port number

  * int srvIdx - source darray index

  * int inIdx - new darray index

  * returns int - the host name

*/
  int getPort(int srvIdx, unsigned long inIdx) const
  { return m_portList[srvIdx][inIdx]; }

/*
2.5.3 Method ~unsigned long getSize const~

returns for an index of the original darray the size of available host or port
combinations

  * int srvIdx - source darray index

  * returns unsigned long - the amount of host/port combinations

*/
  int getSize(int srvIdx) const
  { return m_size[srvIdx]; }

/*
2.5.4 Method ~MultiConnType getType~

returns the type of the current function (sender or receiver)

  * returns MultiConnType - function type

*/
  MultiConnType getType() const
  {
    assert(m_type != DSC_SMC_P_NONE);
    return m_type;
  } 

/*
2.5.5 Method ~string getTypeStr~

returns the type of the current function (sender or receiver)
as string

  * returns string - function type as string

*/
  string getTypeStr() const
  {
    string ret_val = "ERROR - unknown type!";
    switch(m_type)
      {
      case  DSC_SMC_P_NONE:
        ret_val = "ERROR = type is NONE!";
        break;
      case DSC_SMC_P_SENDER:
        ret_val = "SENDER";
        break;
      case DSC_SMC_P_RECEIVER:
        ret_val = "RECEIVER";
        break;
      default:
        break;
      }
    return ret_val;
  }
/*
2.5.6 Method ~int getBasePortNr const~

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
  template<class T>
  void copyList(const vector<vector<T> > &inL,
                vector<vector<T> > &outL)
  {
    const unsigned long s1 = inL.size();
    outL.resize(s1);
    m_size.resize(s1);
    for (unsigned long i = 0; i < s1; ++i)
      {
        outL[i] = inL[i];
        m_size[i] = inL[i].size();
      }
  }
/*
2.6.1 Private Members

*/
  MultiConnType m_type;
  int m_basePortNr;
  vector<vector<string> > m_hostList;
  vector<vector<int> > m_portList;
  vector<int> m_size;
  
/*
2.7 End of Class

*/
};

/*
3 Class DServerCmdShuffleMultiConn

The class ~DServerCmdShuffleMultiConn~ is used in both sender and receiver
functions of the dshuffle command. I forwards the host name and port number
information to each function. Then it initiates the sending of tuples and
finally checks for errors.

The parameters are taken from the class ~DServerCmdShuffleMultiConnParam~.

  * derives from the class ~DServerThreadRunner~

*/
class DServerCmdShuffleMultiConn : public DServerThreadRunner
{
{
/*
3.1 Private Default Constructor

  * inherited from the class ~DServerThreadRunner~

  * may not be used!

3.2 Constructors

  * DServer[ast] inWorker - pointer to the DServer class, 
    representing the worker

  * int inIndex - Darray index of the receiving worker

*/
public:
  DServerCmdShuffleMultiConn(DServer *inWorker, int inIndex)
    : DServerThreadRunner (inWorker, inIndex) {}

/*
3.3 Destructor

*/
  virtual ~DServerCmdShuffleMultiConn() {}

/*
3.4 Getter Methods

3.4.1 Method ~const string[&] getHost const~
returns host name for the data transfer of  this darray index 
and another darray index

   * unsigned long inIdx - index of another darray
   
   * returns const string[&] - the host name

*/
  const string& getHost(unsigned long inIdx) const
  {
    const DServerCmdShuffleMultiConnParam *p = 
      getParam<DServerCmdShuffleMultiConnParam>() ;
    return p -> getHost(getIndex(), inIdx);
  }
/*
3.4.2 Method ~int getPort const~

returns port number for the data transfer of  this darray index 
and another darray index

   * unsigned long inIdx - index of another darray
   
   * returns int - the port number

*/
  const int getPort(unsigned long inIdx) const
  {
    const DServerCmdShuffleMultiConnParam *p = 
      getParam<DServerCmdShuffleMultiConnParam>() ;
    return p -> getPort(getIndex(), inIdx);
  }
  
/*
3.4.3 Method ~int getSize const~

returns the size of other darrays with regards to this darray index

   * returns int - the size

*/
  int getSize() const
  {
    const DServerCmdShuffleMultiConnParam *p = 
      getParam<DServerCmdShuffleMultiConnParam>() ;
    return p -> getSize(getIndex());
  }
    
/*
3.4.4 Method ~DServerCmdShuffleMultiConnParam::MultiConnType getType const~

returns the type of this function (sender of receiver)

   * returns DServerCmdShuffleMultiConnParam::MultiConnType - the type

*/
  DServerCmdShuffleMultiConnParam::MultiConnType getType() const
  {
    const DServerCmdShuffleMultiConnParam *p = 
     getParam<DServerCmdShuffleMultiConnParam>() ;
    return p -> getType();
  } 
    
/*
3.4.5 Method ~string getTypeStr const~

returns the type of this function (sender of receiver) as string

   * returns string - the type in string representation

*/
  string getTypeStr() const
  {
    const DServerCmdShuffleMultiConnParam *p = 
     getParam<DServerCmdShuffleMultiConnParam>() ;
    return p -> getTypeStr();
  } 
    
/*
3.4.5 Method ~int getBasePortNr const~

returns the base port number for this function

   * returns int - the base port number

*/
  int getBasePortNr() const
  {
    const DServerCmdShuffleMultiConnParam *p = 
     getParam<DServerCmdShuffleMultiConnParam>() ;
    return p -> getBasePortNr();
  }
  
/*
3.4.1 Method ~string getInfo const~

  * returns string - an infromation string

*/
  string getInfo() const
  {
    string port = int2Str((getBasePortNr()+getIndex()));
    return string("SHUFFLE MULTIPLY: cmd on: " + 
                  getWorker() -> getMasterHostIP() + ":" + port);
  }

/*
3.5 Running

3.5.1 Method ~void run~

*/

  void run();
    
/*

3.7 Private Section

*/
private:

/*

3.7.1 Private Methods

*/
// n/a

/*

3.7.2 Private Members

*/
  // n/a
/*

3.8 End of Class

*/
};

#endif // H_DSERVERCMDSHUFFLEMULTIPLECONN_H
