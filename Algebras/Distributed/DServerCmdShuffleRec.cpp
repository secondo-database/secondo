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

Implementation of the class ~DServerCmdShuffleRec~

*/

/*

1 Preliminaries

1.1 Includes

*/


#if 0 // unused until all is checked in!

#include "DServerCmdShuffleRec.h"
#include "DServerCmdCallBackComm.h"

/*

1.2 Extern definitions

*/
extern string toString_d(int);

/*

1.3 Debug output

uncomment the following line for debug output

*/
//#define DS_CMD_OPEN_REC_SHUFFLE_DEBUG 1

/*

2 Implementation

2.1 Method ~void run~

run method of the thread

*/

void
DServerCmdShuffleRec::run()
{
  if (!checkWorkerAvailable())
    return;

  if (getWorker() -> isShuffleOpen())
    {
      cerr << "A shuffle operation is already opened for receiving!" << endl;
      return;
    }

#ifdef DS_CMD_OPEN_REC_SHUFFLE_DEBUG
  cout << (unsigned long)(this) << " DS_CMD_OPEN_REC_SHUFFLE - start" << endl;
#endif

  //Initializes the writing of a tuple-stream, 
  //the d_receive_shuffle operator is started on the remote worker
             
  string line;
  if (!startWorkerStreamCommunication())
    {
      return;
    }
            
  string port =toString_d((1800+m_index)); 
  string com = "let r" + getWorker() -> getName() + 
    toString_d(m_index) + 
    " = " + "d_receive_shuffle(" + 
    getWorker() -> getMasterHostIP_()  + ",p" + port + ")";
                
  // initiate d_receive_shuffle command on the destination
  // worker

  //The d_receive_rel operator is invoked
  string err;
  if (!sendSecondoCmdToWorker1(com, err, false)) // no answer from TypeMap!
    {
      err = "Could not start receiver function on worker!\n" + err;
      setErrorText(err);
      return;
    }

  //The callback connection is opened
  //to the receiveShuffleTypeMap function
  DServerCmdCallBackCommunication* callBack =
    new DServerCmdCallBackCommunication(getWorker() -> 
                                        getMasterHostIP(), 
                                        port
                                        ,toString_d(m_index) + 
                                        " DSC_SHUFF_REC"
                                        );

  if (!(callBack -> startSocket()))
    {
      setErrorText(callBack -> getErrorText());
      delete callBack;
      return;
    }

  if (!(callBack -> startSocketCommunication()))
    {
      setErrorText(callBack -> getErrorText());
      delete callBack;
      return;
    }

  if (!(callBack -> sendTextToCallBack("TYPE",  
                                       getWorker() -> 
                                       getTTypeStr() )))
    {
      setErrorText(callBack -> getErrorText());
      delete callBack;
      return;
    }

  if (!(callBack -> getTagFromCallBack("CLOSE")))
    {
      setErrorText(callBack -> getErrorText());
      delete callBack;
      return;
    }
      
  // stop communication w/ d_receive_shuffle TypeMap
  callBack -> closeSocketCommunication();


  //The callback connection from the value-mapping 
  // is opened and stored
  if (!(callBack -> startSocketCommunication()))
    {
      setErrorText(callBack -> getErrorText());
      delete callBack;
      return;
    }

    // sending size of sourceWorkers
  if (!(callBack -> sendTextToCallBack("SRCWSIZE",  
                                       getSourceWorkerSize())))
    {
      setErrorText(callBack -> getErrorText());
      delete callBack;
      return;
    }

  for (unsigned long i = 0; i <  getSourceWorkerSize(); i++)
    {
      // sending source worker host name
      if (!(callBack -> sendTextToCallBack("SRCWHOST",  
                                           getSourceWorkerHost(i))))
        {
          setErrorText(callBack -> getErrorText());
          delete callBack;
          return;
        }

      // wait for OK
      if (!(callBack -> getTagFromCallBack("OK")))
        {
          setErrorText(callBack -> getErrorText());
          delete callBack;
          return;
        }
            
      // sending NEXT to proceed
      if (!(callBack -> sendTagToCallBack("NEXT")))
        {
          setErrorText(callBack -> getErrorText());
          delete callBack;
          return;
        }
      
      // sending source worker to-host
      if (!(callBack -> sendTextToCallBack("SRCWTPORT",  
                                           getSourceWorkerHostToPort(i))))
        {
          setErrorText(callBack -> getErrorText());
          delete callBack;
          return;
        }

      // wait for OK
      if (!(callBack -> getTagFromCallBack("OK")))
        {
          setErrorText(callBack -> getErrorText());
          delete callBack;
          return;
        }

      // sending NEXT to proceed, if there are more
      if (i < getSourceWorkerSize() - 1)
        {
          if (!(callBack -> sendTagToCallBack("NEXT")))
            {
              setErrorText(callBack -> getErrorText());
              delete callBack;
              return;
            }
        }

    } // for (unsigned long i = 0; i <  getSourceWorkerSize(); i++)

  if (!(callBack -> sendTagToCallBack("DONE")))
    {
      setErrorText(callBack -> getErrorText());
      delete callBack;
      return;
    }

  getWorker() -> 
    saveWorkerCallBackConnection(
         callBack -> getSocketCommunicationForSaving());

  delete callBack;
                        
  getWorker() -> setShuffleOpen();

#ifdef DS_CMD_OPEN_REC_SHUFFLE_DEBUG
  cout << (unsigned long)(this) << " DS_CMD_OPEN_REC_SHUFFLE - done" << endl;
#endif   

} // run()

#endif // if 0
