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
[1] Class DServerCmdShuffleRec Implementation

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

Implementation of the class ~DServerCmdShuffleRec~

1 Preliminaries

1.1 Includes

*/
#include "DServerCmdShuffleRec.h"

/*
1.2 Debug Output

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
            
  if (!startWorkerStreamCommunication())
    {
      return;
    }
            
  string port = int2Str((getBasePortNr()+getIndex())); 
  string com = "let r" + getWorker() -> getName() + getIndexStr() +
    " = " + "d_receive_shuffle(" + 
    getWorker() -> getMasterHostIP_()  + ",p" + port + ")";
                

#ifdef DS_CMD_OPEN_REC_SHUFFLE_DEBUG
  cout << (unsigned long)(this) << " DS_CMD_OPEN_REC_SHUFFLE - on" 
       << getWorker() -> getMasterHostIP_() << ":" << port << ":" << endl
       << com << endl;
#endif
  // initiate d_receive_shuffle command on the destination
  // worker

  string err;

  //The d_receive_rel operator is invoked
  // this command is blocking, until DShuffle is done!
  if (!sendSecondoCmdToWorker1(com, err)) 
    {
      err = "Could not start receiver function on worker!\n" + err;
      cout << "ERROR: " << err << endl;
      setErrorText(err);
      return;
    }

#ifdef DS_CMD_OPEN_REC_SHUFFLE_DEBUG
  cout << (unsigned long)(this) << " DS_CMD_OPEN_REC_SHUFFLE - done" << endl;
#endif   

} // run()
