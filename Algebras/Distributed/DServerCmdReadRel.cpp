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
[1] Class DServerCmdExecute Implementation

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

Implementation of the class ~DServerCmdReadrel~

1 Preliminaries

1.1 Debug Output

uncomment the following line

*/
//#define DS_CMD_READREL_DEBUG 1

/*
1.2 Includes

*/

#include "DServerCmdReadRel.h"
#include "DServerCmdCallBackComm.h"
#include "DBAccessGuard.h"

#include "TupleFifoQueue.h"
#include "ThreadedMemoryCntr.h"

// From file DServer.h
extern ZThread::Mutex Cmd_Mutex;

class RecReadContainer 
  : public DServerCmdCallBackCommunication::ReadTupleContainer
{
public:
  RecReadContainer(TupleFifoQueue* inTfq,
                   ThreadedMemoryCounter* inMemCntr)
 
    : DServerCmdCallBackCommunication::ReadTupleContainer()
    , m_tfq(inTfq)
    , m_memCntr(inMemCntr) {}

  virtual ~RecReadContainer() {}

  bool storeTuple(Tuple *t) const
  {
    if (t != NULL)
      {
        DBAccessGuard::getInstance() -> T_IncReference(t);
        if (m_memCntr != NULL)
          m_memCntr -> request(t -> GetSize());
        m_tfq -> put(t);
      }
    return true;
  }

private:
  TupleFifoQueue* m_tfq;
  ThreadedMemoryCounter* m_memCntr;

};


/*
  
2 Implementation

2.1 Method ~void run~

implements the actual copy functionality of data from one darray index
to an index of another darray on the same worker.
 
*/

void
DServerCmdReadRel::run()
{ 
#if DS_CMD_READREL_DEBUG
  cout << "DServerCmdReadRel::run" << getIndexStr() << endl;
#endif

  if (!checkWorkerAvailable())
    return;

  if (!startWorkerStreamCommunication())
    {
      setErrorText("Could not initiate communication!");
      return;
    }

  const int curIdx = getIndex();
   
  if (receivingRelation())
    {
      Cmd_Mutex.acquire(); 
      (*(getOutIsPresent()))[curIdx] = false; // not here yet
      Cmd_Mutex.release(); 
    }

  string master_port =int2Str((1300+curIdx));
          
  string sendCmd = 
    "query d_send_rel (" + getWorker() -> getMasterHostIP_() + 
    ",p" + master_port + 
    ",r" + getWorker() -> getName() + int2Str(curIdx) + ")";

#if DS_CMD_READREL_DEBUG
  cout << "sending:" << sendCmd << endl;
#endif
  //The sendD-operator on the worker is started 
  if (!sendSecondoCmdToWorkerSOS(sendCmd, true))
    {
      string errMsg;
      if (hasCmdError())
        errMsg = getCmdErrorText();
      else
        errMsg = "Could not read data from worker!";

      setErrorText(errMsg);
        
      waitForSecondoResultThreaded();
      return;
    }

  // open communication with d_receive_rel - TypeMap
  DServerCmdCallBackCommunication callBack(getWorker() ->getMasterHostIP(),
                                           master_port);
  callBack.startSocket();
  callBack.startSocketCommunication();

      
  bool allOk = true;
     
  if (receivingRelation())
    {
      allOk = recRelation(callBack);
    }
  else
    {
      allOk = recTupleQueue(callBack);
    }
     
  callBack.closeCallBackCommunication();

  if (!waitForSecondoResultThreaded())
    {
      string errMsg;
      if (hasCmdError())
        errMsg = getCmdErrorText();
      else
        errMsg = "Could not read data to worker!";
       
      setErrorText(errMsg);
    }
  else
    {   
      if (receivingRelation())
        {
          Cmd_Mutex.acquire(); 
          (*(getOutIsPresent()))[curIdx] = true; //got it
          Cmd_Mutex.release();
        }
    }

  if (!closeWorkerStreamCommunication())
    {
      setErrorText("Could not stop communication!");
      return;
    }

#if DS_CMD_READREL_DEBUG
  cout << "DServerCmdReadRel::run DONE" << endl;
#endif
} // run()

bool
DServerCmdReadRel::recRelation(DServerCmdCallBackCommunication &callBack)
{
  //create relation iterator
  GenericRelation* mRel = 
    (Relation*)(getOutElements() -> operator [] (getIndex())).addr;

  ReceiveRelContainer recRel (mRel);
  
  bool  allOK = true;
  bool runIt = 
    callBack.getTagFromCallBackTF("NEXTTUPLE", "CLOSE", allOK);
  
  while (allOK && runIt)
    {
      
      allOK = callBack.readTupleFromCallBack(getTType(),
                                             &recRel);
       if (allOK)
        {
          callBack.sendTagToCallBack("OK");
          runIt = 
            callBack.getTagFromCallBackTF("NEXTTUPLE", "CLOSE", allOK); 
        }
      else
        {
          callBack.sendTagToCallBack("ERROR");
        }
    }
      
  return allOK;
}

bool 
DServerCmdReadRel::recTupleQueue(DServerCmdCallBackCommunication &callBack)
{ 
  bool allOK = true;
  
  RecReadContainer recTfqRead(getTFQ(), getMemCntr());
  
  bool runIt = 
    callBack.getTagFromCallBackTF("NEXTTUPLE", "CLOSE", allOK);
  
  while (allOK && runIt)
    { 
      
      allOK = callBack.readTupleFromCallBack(getTType(),
                                             &recTfqRead); 
      if (allOK)
        {
          callBack.sendTagToCallBack("OK");
          runIt = 
            callBack.getTagFromCallBackTF("NEXTTUPLE", "CLOSE", allOK); 
        }
      else
        {
          callBack.sendTagToCallBack("ERROR");
        }
    }

  return allOK;
}
