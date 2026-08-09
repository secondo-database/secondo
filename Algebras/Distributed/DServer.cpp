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
[1] Class DServer Implementation

\begin{center}
April 2012 Thomas Achmann


November 2010 Tobias Timmerscheidt
\end{center}

[TOC]

0 Description

Implementation of the class ~DServer~



1 Preliminaries

1.1 Includes

*/
#include "DServer.h"

#include "SocketIO.h"
#include "zthread/Mutex.h"
#include "TupleFifoQueue.h"
#include "DBAccessGuard.h"
#include "ThreadedMemoryCntr.h"

/*
1.2 Debug Output

uncoment the following lines for debug output

*/

//#define DS_CMD_CLOSE_WRITE_REL_DEBUG 1
//#define DS_CMD_OPEN_WRITE_REL_DEBUG 1
//#define DS_CMD_WRITE_REL_DEBUG 1
//#define DS_CMD_READ_REL_DEBUG 1
//#define DS_CMD_READ_TB_REL_DEBUG 1
//#define DS_CMD_DELETE_DEBUG 1

/*
1.3 Global Definitions

1.3.1 Variables

*/
//IP-Adress of this system (Master)
string HostIP;
string HostIP_;

/*
1.3.2 Thread Mutex

*/
//Synchronisation of access to Flobs
ZThread::Mutex Flob_Mutex;
//Synchronisation of access to read non relations and write commands
ZThread::Mutex Cmd_Mutex;


/*
2 Implementation

2.1 Static Variables 

Initialisation

*/
Word DServer::ms_emptyWord;


/*
2.2 Constructor

  * const string[&] inHostName - host name, where the worker runs

  * int inPortNumber - port number, where the worker listens to

  * ListExpr inType 

*/
DServer::DServer(const string& inHostName, int inPortNumber,
                 string inName, ListExpr inType)
  : m_host(inHostName)
  , m_port(inPortNumber)
  , m_name(inName)
  , m_interface(NULL)
  , m_interfaceNl(NULL)
  , m_error(false)
{
   m_type = inType;
   DBAccess::getInstance() -> NL_ToString(m_type, m_typeStr);
}
/*

2.3 Method ~bool connectToWorker~

connects to a SECONDO instance at a worker host

  * returns bool - true: success; 

*/
bool
DServer::connectToWorker()
{
  // One ordinary client of the client/server protocol, exactly as
  // Algebras/Distributed2 does it (see ConnectionInfo). What used to be here
  // was a second, partial implementation of the same protocol: it wrote the
  // <Connect> and <Secondo> tags by hand, ignored the <SecondoIntro> block
  // entirely -- so it never learned the transfer mode or the protocol version
  // and cannot connect to a current server at all -- and decided whether a
  // command had succeeded by looking for the substring "ERROR" in the reply.
  //
  // The interface brings its own NestedList because workers are driven from
  // several threads; see the member declaration.
  m_interfaceNl = new NestedList();
  m_interface = new SecondoInterfaceCS(true, m_interfaceNl, true);
  m_interface->setMaxAttempts(10);
  m_interface->setTimeout(1);

  string errMsg = "";
  if (!m_interface->Initialize("", "", getServerHostName(),
                               getServerPortStr(),
                               string(""), string(""), errMsg, true))
    {
      setErrorText("Connection to the worker couldn't be established: " +
                   errMsg);
      delete m_interface;
      m_interface = 0;
      delete m_interfaceNl;
      m_interfaceNl = 0;
      return false;
    }

  ListExpr resultList = m_interfaceNl->TheEmptyList();
  int errorCode = 0, errorPos = 0;
  string errorMessage = "";
  m_interface->Secondo("open database distributed", resultList, 1,
                       true, false, resultList,
                       errorCode, errorPos, errorMessage);
  if (errorCode != 0)
    {
      // The outcome is the error code the server reported, not a substring
      // search over the reply text -- which is what made this code report
      // success for any answer that happened not to contain "ERROR".
      setErrorText(string("Opening of database \"distributed\" ") +
                   "on worker failed: " + errorMessage);
      return false;
    }

  HostIP = getServerAddress();
  HostIP_ = "h" + stringutils::replaceAll(HostIP,".","_");

  //cout << "Connection to Worker on " << host << " established." << endl;
  return true;
}

/*
2.3.1 Method ~string getServerAddress~

*/
std::string
DServer::getServerAddress() const
{
  return m_interface != 0 ? m_interface->getServerAddress() : std::string("");
}

/*
2.4 Method ~void Terminate~

stops the SECONDO instance on the worker

*/

void DServer::Terminate()
{
  //cout << "TERMINATE" << endl;
  if (m_interface != 0)
    {
      // Terminate sends <Disconnect/> and closes the socket, which is what the
      // hand-written teardown here used to do by itself.
      m_interface->Terminate();
      delete m_interface;
      m_interface = 0;
      delete m_interfaceNl;
      m_interfaceNl = 0;
    }
  else
    {
      cout << "ERROR: No Server Running!" << endl;
    }

}
/*
2.6 Method ~const string[&] getMasterHostIP const~

  * returns const string[&] - the host TCP/IP adress of the master server
  
  * format: xxx.xxx.xxx.xxx

*/  
const string& DServer::getMasterHostIP() const { return HostIP; }

/*
2.7 Method ~const string[&] getMasterHostIP[_] const~

  * returns const string[&] - the host TCP/IP adress of the master server
  
  * format: hxxx[_]xxx[_]xxx[_]xxx

*/  
const string& DServer::getMasterHostIP_() const { return HostIP_; }

/*
2.10 Method ~bool Multiply~

multiplys the SECONDO instances at the worker. This is used to 
store data of multiple darray indexes at one worker

  * int count - number of additional instances
 
  * returns bool - true: success

*/
bool DServer::Multiply(int count)
{
  //cout << "MULTIPLY" << endl;

  if(getNumChilds() > 0) 
      return false;

  if(count < 1) 
    {
      m_childs.clear();
      return true;
    }

   for(int i = 0;i<count - 1;i++)
   {
     DServer* ds =  new DServer(getServerHostName(),
                                getServerPort(),
                                m_name,m_type);
     m_childs.push_back( ds );
     try
        {
          if (!(ds -> connectToWorker()))
            return false;
              
        }
      catch(const exception &e)
        {
          cout << "Error starting DServer on " 
               << getServerHostName() << ":" << getServerPortStr() << endl;
          return false;
        }
   }

   return true;
     
}
/*
2.11 Method ~void DestroyChilds~

stops workers created by the ~Multiply~ method

*/
void DServer::DestroyChilds()
{
  //cout << "DESTROY" << endl;
  for(int i = 0;i<getNumChilds() && i < (int)m_childs.size(); i++)
   {
     if (m_childs[i] != NULL)
       {
         m_childs[i]->Terminate();
         delete m_childs[i];
         m_childs[i] = NULL;
       }
   }
  m_childs.clear();
}

/*
2.12 Method ~bool checkServer~

performs a check on the worker

  * bool writeError - writes an error to stderr
 
  * returns bool - true: success

*/
bool
DServer::checkServer(bool writeError) const
{
  if (m_interface == 0)
    {
      if (writeError)
        cerr << "ERROR: Not connected to worker on "
             << getServerHostName() << ":" << getServerPortStr() << endl;
      return false;
    }

  if (!m_interface->isInitialized())
    {
      if (writeError)
        cerr << "ERROR: Could not establish connection to worker on "
             << getServerHostName() << ":" << getServerPortStr() << endl;
      return false;
    }

  if ((unsigned int)getNumChilds() != m_childs.size())
    {
      if (writeError)
        cerr << "ERROR: Workers are not setup correctly, restart cluster" 
             << endl;
      return false;
    }

  return true;
}
    
/*
2.13 Method ~void print~

*/  
void DServer::print() const
{
  cout << (unsigned long)(this) << " : " << " " 
       << getServerHostName() << " " << getServerPortStr() << endl;
}
