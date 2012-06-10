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

Implementation of the class ~DServerCmdExecute~

1 Preliminaries

1.1 Debug Output

uncomment the following line

*/
//#define DS_CMD_COPY_DEBUG 1

/*
1.2 Includes

*/

#include "DServerCmdRead.h"
#include "DServerCmdCallBackComm.h"


//Uses Function from ArrayAlgebra
extern void extractIds(const ListExpr,int&,int&);

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
DServerCmdRead::run()
{ 
#if DS_CMD_COPY_DEBUG
  cout << "DServerCmdRead::run" << getIndexStr() << endl;
#endif

  if (!checkWorkerAvailable())
    return;

  if(getWorker() -> isRelOpen()) return;

  if (!startWorkerStreamCommunication())
    {
      setErrorText("Could not initiate communication!");
      return;
    }

  int algID,typID;
  extractIds(getWorker() -> getTType(),algID,typID);

  const unsigned long idxSize = getIndexListSize();

  for (unsigned long idx = 0; idx < idxSize; idx ++)
    {
      const int curIdx = getIndexAt(idx);

      Cmd_Mutex.acquire(); 
      (*(getOutIsPresent()))[curIdx] = false; // not here yet
      Cmd_Mutex.release(); 

      string master_port =int2Str((1500+curIdx));
          
      string sendCmd = "query sendD ("  + 
        getWorker() -> getMasterHostIP_() + ",p" + master_port + 
        ",r" + getWorker() -> getName() + int2Str(curIdx) + ")";

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

      DServerCmdCallBackCommunication callBack(getWorker() ->getMasterHostIP(),
                                               master_port);
      callBack.startSocket();
      callBack.startSocketCommunication();

      if (!callBack.sendTextToCallBack("TYPE", getWorker() -> getTTypeStr()))
        {;
          waitForSecondoResultThreaded();
          return;
        }

      bool errFlag;
      if (!callBack.getTagFromCallBackTF("DATA", "UNDEFINED", errFlag))
        { 
          waitForSecondoResultThreaded();
          return;
        }
      string line;
      if (!callBack.getTextFromCallBack("SIZE", line))
        {
          waitForSecondoResultThreaded();
          return;
        }

      int size = atoi(line.data());
              
      //The actual data is received...       
      char* buffer = new char[size];

      callBack.Read(buffer,size);
              
      //... and converted back to a secondo-object
      SmiRecordFile recF(false,0);
      SmiRecord rec;
      SmiRecordId recID;
      Cmd_Mutex.acquire(); 
      recF.Open("rec");
      recF.AppendRecord(recID,rec);
      rec.Write(buffer,size,0);
              
      size_t s = 0;
      am->OpenObj(algID,typID,rec,s,getWorker() -> getTType(),
                  (*(getOutElements()))[curIdx]);
      
      recF.DeleteRecord(recID);
      recF.Close();
      Cmd_Mutex.release();
      delete [] buffer;
              
      bool hasNoFlobErr = true;

      int flobs=0;

      while(callBack.getTagFromCallBackTF("FLOB", "CLOSE", hasNoFlobErr))
        {

          //Size of the flob is received
          if (!callBack.getTextFromCallBack("FSIZE", line))
            {
              setErrorText(string("Unexpected Response from ") +
                           "worker (<FSIZE> expected)!");        
              Attribute* a = static_cast<Attribute*>
                ((am->Cast(algID,typID))
                 ((*(getOutElements()))[curIdx].addr));

              a -> SetDefined(false);
              waitForSecondoResultThreaded();
              return;
            }

          //The threads must not write a flob on the same time
          Flob_Mutex.acquire();
          SmiSize si = atoi(line.data());

          int n_blocks = si / 1024 + 1;
                  
          //Data of the flob is received
          char* buf = new char[n_blocks*1024];
          memset(buf,0,1024*n_blocks);
                  
          for(int i = 0; i< n_blocks; i++)
            callBack.Read(buf+1024*i,1024);
                  
          Attribute* a = static_cast<Attribute*>
            ((am->Cast(algID,typID))
             ((*(getOutElements()))[curIdx].addr));
          
          //Flob data is written
          Flob*  f = a->GetFLOB(flobs);
          f->write(buf,si,0);
                  
                  
          delete []  buf;
          Flob_Mutex.release();
                  
          flobs++;
        }

      if (!hasNoFlobErr)
        {
              cout << "ERROR: " << callBack.getErrorText() << endl;
        }

      callBack.sendTagToCallBack("FINISH");
              
      callBack.closeCallBackCommunication();

      if (!waitForSecondoResultThreaded())
        {
          string errMsg;
          if (hasCmdError())
            errMsg = getCmdErrorText();
          else
            errMsg = "Could not read data from worker!";
       
          cout << "READ: NO END!" << endl;

          setErrorText(errMsg);
        }
      else
        {
          Cmd_Mutex.acquire(); 
          (*(getOutIsPresent()))[curIdx] = true; //got it
          Cmd_Mutex.release(); 
        }
    } // while(!m_cmd -> getDArrayIndex() -> empty())

#if DS_CMD_COPY_DEBUG
  cout << "DServerCmdRead::run DONE" << endl;
#endif
} // run()
