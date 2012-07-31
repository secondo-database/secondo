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

Implementation of the class ~DServerCmdWriterel~

1 Preliminaries

1.1 Debug Output

uncomment the following line

*/
//#define DS_CMD_WRITEREL_DEBUG 1

/*
1.2 Includes

*/

#include "DServerCmdWriteRel.h"
#include "DServerCmdCallBackComm.h"
#include "DBAccessGuard.h"

/*
  
2 Implementation

2.1 Method ~void run~

implements the actual copy functionality of data from one darray index
to an index of another darray on the same worker.
 
*/

void
DServerCmdWriteRel::run()
{ 
#if DS_CMD_WRITEREL_DEBUG
  cout << "DServerCmdWriteRel::run" << getIndexStr() << endl;
#endif

  if (!checkWorkerAvailable())
    return;

  if (!startWorkerStreamCommunication())
    {
      setErrorText("Could not initiate communication!");
      return;
    }

  while(nextIndex())
    {
      const int curIdx = getIndex();

      string master_port =int2Str((1800+curIdx));
          
      string sendCmd = 
        "let r" + getWorker() -> getName() + int2Str(curIdx) +
        " = " + "d_receive_rel(" + getWorker() -> getMasterHostIP_() + 
        ",p" + master_port + ")";

#if DS_CMD_WRITEREL_DEBUG
      cout << "Sending:" << sendCmd << endl;
#endif
      //The sendD-operator on the worker is started 
      if (!sendSecondoCmdToWorkerSOS(sendCmd, true))
        {
          string errMsg;
          if (hasCmdError())
            errMsg = getCmdErrorText();
          else
            errMsg = "Could not write data from worker!";

          setErrorText(errMsg);
        
          waitForSecondoResultThreaded();
          return;
        }

      // open communication with d_receive_rel - TypeMap
      DServerCmdCallBackCommunication callBack(getWorker() ->getMasterHostIP(),
                                               master_port);
      callBack.startSocket();
      callBack.startSocketCommunication();

      // send TYPE to receiveD - TypeMap
      if (!callBack.sendTextToCallBack("TYPE", getWorker() -> getTTypeStr()))
        {;
          waitForSecondoResultThreaded();
          return;
        }

      // await "CLOSE" tag
      if (!callBack.getTagFromCallBack("CLOSE"))
        {
          waitForSecondoResultThreaded();
          return;
        }
      // stop communication w/ reveive TypeMap
      callBack.closeSocketCommunication();

      //The callback connection from the value-mapping 
      // is opened and stored
      if (!(callBack.startSocketCommunication()))
        {
          setErrorText(callBack.getErrorText());
          waitForSecondoResultThreaded();
          return;
        }
      
      // send TYPE to receiveD - TypeMap
      if (!callBack.sendTextToCallBack("TYPE", getWorker() -> getTTypeStr()))
        {
          setErrorText(callBack.getErrorText());
          waitForSecondoResultThreaded();
          return;
        }

      // send INTYPE to receiveD - TypeMap
      if (!callBack.sendTextToCallBack("INTYPE", getSendType()))
        {
          setErrorText(callBack.getErrorText());
          waitForSecondoResultThreaded();
          return;
        }

      // send INTYPE to receiveD - TypeMap
      if (!callBack.sendTextToCallBack("DELIDX", getDelIndex()))
        {
          setErrorText(callBack.getErrorText());
          waitForSecondoResultThreaded();
          return;
        }

      if (!callBack.getTagFromCallBack("GOTALL"))
        { 
          setErrorText(callBack.getErrorText());
          waitForSecondoResultThreaded();
          return;
        }

      bool allOk = true;
      if (sendingRelation())
        {
          allOk = sendRelation(callBack);
        }
      else
        {
          allOk = sendTupleQueue(callBack);
        }

      if (allOk && // don't send close in case of error!
          !callBack.sendTagToCallBack("CLOSE"))
        { 
          waitForSecondoResultThreaded();
          return;
        }

      if (!callBack.getTagFromCallBack("FINISH"))
        { 
          setErrorText(callBack.getErrorText());
          waitForSecondoResultThreaded();
          return;
        }
   
      callBack.closeCallBackCommunication();

      if (!waitForSecondoResultThreaded())
        {
          string errMsg;
          if (hasCmdError())
            errMsg = getCmdErrorText();
          else
            errMsg = "Could not write data to worker!";
       
          setErrorText(errMsg);
        }
    }

  if (!closeWorkerStreamCommunication())
    {
      setErrorText("Could not stop communication!");
      return;
    }

#if DS_CMD_WRITEREL_DEBUG
  cout << "DServerCmdWriteRel::run DONE" << endl;
#endif
} // run()

bool
DServerCmdWriteRel::sendRelation(DServerCmdCallBackCommunication &callBack)
{
  //create relation iterator
  GenericRelation* rel = 
    (Relation*)(getInElements() -> operator [] (getIndex())).addr;
  GenericRelationIterator* iter = rel->MakeScan();
  Tuple* t = NULL;
  bool allOK = true;
  while (allOK &&
         ((t = iter->GetNextTuple()) != 0))
    { 
      if (!callBack.sendTagToCallBack("TUPLE"))
        { 
          waitForSecondoResultThreaded();
          return false;
        }

      callBack.writeTupleToCallBack(t); 
      
      DBAccessGuard::getInstance() -> T_DeleteIfAllowed(t);

      if (!callBack.getTagFromCallBackTF("OK", "ERROR", allOK))
        {
          allOK = false;
        }
    } 

  delete iter;
  return allOK;
}

bool 
DServerCmdWriteRel::sendTupleQueue(DServerCmdCallBackCommunication &callBack)
{ 
  Tuple* t = NULL;
  bool allOK = true;
  DServerMultiCommand* fifoQueue =  getTupleQueue(getIndex());
  
  while (allOK && 
         ((t = fifoQueue -> GetTuple()) != 0))
    { 
      if (!callBack.sendTagToCallBack("TUPLE"))
        { 
          waitForSecondoResultThreaded();
          return false;
        }

      callBack.writeTupleToCallBack(t); 
       
      DBAccessGuard::getInstance() -> T_DeleteIfAllowed(t);

      if (!callBack.getTagFromCallBackTF("OK", "ERROR", allOK))
        {
          allOK = false;
        }
    }

  return allOK;
}
