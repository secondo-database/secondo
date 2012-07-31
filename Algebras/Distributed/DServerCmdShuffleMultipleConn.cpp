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
[1] Class DServerCmdShuffleMultipleConn


\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

Implementation of the class ~DServerCmdShuffleMultipleConn~

1 Preliminaries

1.1 Includes

*/
#include "DServerCmdShuffleMultipleConn.h"
#include "DServerCmdCallBackComm.h"

/*
1.2 Debug Output

uncomment the following line for debug output

*/
//#define DS_CMD_OPEN_MULTICONN_SHUFFLE_DEBUG 1

/*
2 Implementation

2.1 Method ~void run~

run method of the thread

*/

void
DServerCmdShuffleMultiConn::run()
{
#ifdef DS_CMD_OPEN_MULTICONN_SHUFFLE_DEBUG
  cout << getTypeStr() + "_" + getIndexStr()
       << " DS_CMD_OPEN_MULTICONN_SHUFFLE - start:"  <<  endl;
     
       
  assert(isReceiver() || isSender());

#endif

      //Initializes the writing of a tuple-stream, 
      //the d_receive_rel operator is started on the remote worker
             
     string line;
     string port = int2Str((getBasePortNr()+getIndex()));
     //The callback connection is opened
     //to the receiveShuffleTypeMap function
     //or to the sendShuffleValueMap function
     DServerCmdCallBackCommunication* callBack =
       new DServerCmdCallBackCommunication(getWorker() -> 
                                           getMasterHostIP(), 
                                           port
#ifdef DS_CMD_OPEN_MULTICONN_SHUFFLE_DEBUG
                                           ,getTypeStr() + "_" +getIndexStr()
#endif
                                           );

     if (!(callBack -> startSocket()))
       {
         setErrorText(callBack -> getErrorText());
         delete callBack;
         return;
       }

     //cout << getTypeStr() + "_" + getIndexStr()
     //     << " start communication" << endl;

     if (!(callBack -> startSocketCommunication()))
       {
         setErrorText(callBack -> getErrorText());
         delete callBack;
         return;
       }

     // for the receiver we first have to send 
     // data to the typemapping function
     if (isReceiver())
       {

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
       } // if (getType() == 
         //             DServerCmdShuffleMultiConnParam::DSC_SMC_P_RECEIVER)

     // sending size of sourceWorkers
     if (!(callBack -> sendTagToCallBack("STARTMULTIPLYCONN")))
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
            
    // sending size of sourceWorkers
  if (!(callBack -> sendTextToCallBack("SRCWSIZE",  
                                       getSize())))
    {
      setErrorText(callBack -> getErrorText());
      delete callBack;
      return;
    }

  //if (isReceiver())
  // cout << getTypeStr() + "_" + getIndexStr()
  //     << " sending size:" << getSize() << endl;

  for (long i = 0; i <  (long) getSize(); i++)
    {
      //cout << getTypeStr() + "_" + getIndexStr()
      //   << " sending " << i << ": " 
      //   << getHost(i) << ":" << getPort(i) << endl;

      // sending source worker host name
      if (!(callBack -> sendTextToCallBack("SRCWHOST",  
                                           getHost(i), true)))
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
                                           getPort(i))))
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
      if (i < getSize() - 1)
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

  //awaiting OK
  if (!(callBack -> getTagFromCallBack("READY")))
    {
      setErrorText("Could not initiate dshuffle");
      delete callBack;
      return;
    }

  bool noErr = true;

  //awaiting DONE
  if (!(callBack -> getTagFromCallBackTF("DONE","ERROR", noErr)))
    {
      string errMsg;
      if (!(callBack -> getTextFromCallBack("ERRORDESC", errMsg)))
        {
          setErrorText("Could not receive correct error message!");
          delete callBack;
          return;
        }

      setErrorText(errMsg);
    }

  //cout << getTypeStr() + "_" + getIndexStr()
  //      << " done communication" << endl;

  if (!noErr)
    {
      setErrorText("Could not multiply connections for dshuffle");
      delete callBack;
      return;
    }

  delete callBack;
#ifdef DS_CMD_OPEN_MULTICONN_SHUFFLE_DEBUG
  cout << (unsigned long)(this) 
       << " DS_CMD_OPEN_MULTICONN_SHUFFLE - done" << endl;
#endif   

} // run()
