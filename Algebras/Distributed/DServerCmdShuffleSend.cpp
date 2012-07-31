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
[1] Class DServerCmdShuffleSend Implementation

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

Implementation of the class ~DServerCmdShuffleSend~

1. Preliminaries

1.1 Debug Output

uncomment the following line

*/
//#define DS_CMD_OPEN_SEND_SHUFFLE_DEBUG 1
/*
1.2 Includes

*/
#include "DServerCmdShuffleSend.h"

/*
  
2 Implementation

2.1 Method ~void run~

implements the actual send functionality for the dshuffle operator
 
*/

void
DServerCmdShuffleSend::run()
{
  if (!checkWorkerAvailable())
    return;

#ifdef DS_CMD_OPEN_SEND_SHUFFLE_DEBUG
  cout << (unsigned long)(this) << " DS_CMD_OPEN_SEND_SHUFFLE - start" 
       << endl;
#endif

  //Initializes the writing of a tuple-stream, 
  //the d_send_rel operator is started on the remote worker
             
  if (!startWorkerStreamCommunication())
    {
      return;
    }
            
  string port = int2Str((getBasePortNr()+getIndex()));
      
  string dIndexFunction = 
    stringutils::replaceAll(getSendFunc(), "d_idx", 
                            "int " + getIndexStr());

      
     
  string com = "(query (d_send_shuffle (feed r" + 
    getWorker() -> getName() + getIndexStr() + 
    ") (fun (tuple1 TUPLE) " + dIndexFunction + ") " +
    getWorker() -> getMasterHostIP_()  + " p" + port + ") )";

#ifdef DS_CMD_OPEN_SEND_SHUFFLE_DEBUG
  cout << (unsigned long)(this) << " DS_CMD_OPEN_SEND_SHUFFLE - send " 
       << getIndex() << ": " << com << endl;
#endif
      
  // initiate d_send_shuffle command on the destination
  // worker

  //The d_send_rel operator is invoked
     
  // this command is blocking, until DShuffle is done!
  if (!sendSecondoCmdToWorkerNL(com))
    {
      string err;
      if (hasCmdError())
        err = getCmdErrorText();
      else
        err = "Could not start shuffle - sender function on worker!";
      getWorker() -> setErrorText(err);
      return;
    }
           
#ifdef DS_CMD_OPEN_SEND_SHUFFLE_DEBUG
  cout << (unsigned long)(this) << " DS_CMD_OPEN_SEND_SHUFFLE - done"
       << endl;
#endif   

} // run()
