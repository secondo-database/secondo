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

Implementation of the class ~DServerCmdWrite~

1 Preliminaries

1.1 Debug Output

uncomment the following line

*/
//#define DS_CMD_WRITE_DEBUG 1

/*
1.2 Includes

*/

#include "DServerCmdWrite.h"
#include "DServerCmdCallBackComm.h"


//Uses Function from ArrayAlgebra
namespace arrayalgebra{
   void extractIds(const ListExpr,int&,int&);
}

// From file DServer.h
extern ZThread::Mutex Cmd_Mutex;
extern ZThread::Mutex Flob_Mutex;

/*
  
2 Implementation

2.1 Method ~void run~

implements the actual copy functionality of data from one darray index
to an index of another darray on the same worker.
 
*/

void
DServerCmdWrite::run()
{ 
#if DS_CMD_WRITE_DEBUG
  cout << "DServerCmdWrite::run" << getIndexStr() << endl;
#endif

  if (!checkWorkerAvailable())
    return;

  if (!startWorkerStreamCommunication())
    {
      setErrorText("Could not initiate communication!");
      return;
    }

  int algID,typID;
  arrayalgebra::extractIds(getWorker() -> getTType(),algID,typID);
  TypeConstructor* tc = am->GetTC(algID,typID);

  while (nextIndex())
    {
      const int curIdx = getIndex();

      string master_port =int2Str((1800+curIdx));
          
      string sendCmd = 
        "let r" + getWorker() -> getName() + int2Str(curIdx) +
        " = " + "receiveD(" + getWorker() -> getMasterHostIP_() + 
        ",p" + master_port + ")";

#if DS_CMD_WRITE_DEBUG
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

      // open communication with receiveD - TypeMap
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

      // type map has finished

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

      if (!callBack.getTagFromCallBack("GOTTYPE"))
        { 
          setErrorText(callBack.getErrorText());
          waitForSecondoResultThreaded();
          return;
        }

      //The element is converted into a binary stream of data
      SmiRecordFile recF(false,0);
      SmiRecord rec;
      SmiRecordId recID;
      Cmd_Mutex.acquire();       
      recF.Open("send");
      recF.AppendRecord(recID,rec);
      size_t size = 0;
      am->SaveObj(algID,typID,rec,size,getWorker() -> getTType(),
                  (*(getInElements()))[curIdx]);
      char* buffer = new char[size]; 
      rec.Read(buffer,size,0);
      //rec.Truncate(3);
      recF.DeleteRecord(recID);
      recF.Close();
      Cmd_Mutex.release();

      //Size of the binary data is sent
      if (!callBack.sendTextToCallBack("SIZE", size))
        { 
          waitForSecondoResultThreaded();
          return;
        }

#if DS_CMD_WRITE_DEBUG
      cout << "Send Size:" << size << endl;
#endif

      //The actual data are sent
      if (!callBack.Write(buffer,size))
        { 
          waitForSecondoResultThreaded();
          return;
        }
 
      delete [] buffer ;

      Attribute* a;
      if(tc->NumOfFLOBs() > 0 ) 
        a = static_cast<Attribute*>((am->Cast(algID,typID))
                (((*(getInElements()))[curIdx]).addr));
         
      //Flobs are sent to worker
      for(int i = 0; i < tc->NumOfFLOBs(); i++)
        {
          //send FLOB Tag as info
          if (!callBack.sendTagToCallBack("FLOB"))
            { 
              waitForSecondoResultThreaded();
              return;
            }

          Flob* f = a->GetFLOB(i);
         
          //Flob is converted to binary data
          SmiSize si = f->getSize();
          int n_blocks = si / 1024 + 1;
          char* buf = new char[n_blocks*1024];
          memset(buf,0,1024*n_blocks);
         
          f->read(buf,si,0);
 
#if DS_CMD_WRITE_DEBUG
          cout << "Send Flob - Size:" << si << endl;
#endif
          //Size of the binary data is sent
          if (!callBack.sendTextToCallBack("FLOBSIZE", si))
            { 
              waitForSecondoResultThreaded();
              return;
            }

         
          //Flob data is sent
          for(int j = 0; j<n_blocks;j++)
            callBack.Write(buf+j*1024,1024);
            
          delete [] buf;

          //send FLOB Tag as info
          bool noErr = true;
          if (!callBack.getTagFromCallBackTF("GOTFLOB", "ERROR", noErr))
            { 
              waitForSecondoResultThreaded();
              return;
            }

          if (!noErr)
            { 
              waitForSecondoResultThreaded();
              return;
            }
        } //
         

      if (!callBack.sendTagToCallBack("CLOSE"))
        { 
          waitForSecondoResultThreaded();
          return;
        }

      if (!callBack.getTagFromCallBack("FINISH")) 
        { 
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
    } // while(nextIndex())

  if (!closeWorkerStreamCommunication())
    {
      setErrorText("Could not stop communication!");
      return;
    }

#if DS_CMD_WRITE_DEBUG
  cout << "DServerCmdWrite::run DONE" << endl;
#endif
} // run()
