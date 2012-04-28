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
[1] Class DServerCmdCopy Implementation

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

Implementation of the class ~DServerCmdCopy~

1 Preliminaries

1.1 Debug Output

uncomment the following line

*/
//#define DS_CMD_COPY_DEBUG 1

/*
1.2 Includes

*/

#include "DServerCmdCopy.h"

/*
  
2 Implementation

2.1 Method ~void run~

implements the actual copy functionality of data from one darray index
to an index of another darray on the same worker.
 
*/

void
DServerCmdCopy::run()
{ 
#if DS_CMD_COPY_DEBUG
  cout << "DServerCmdCopy::run" << endl;
#endif

  if (getIndex() == getReplaceIndex())
    {
#if DS_CMD_COPY_DEBUG
      cout << "DServerCmdCopy::run - substituing:" 
           << getReplaceIndex()<< endl;
#endif
      return;
    }

  if (!checkWorkerAvailable())
    return;

  if(getWorker() -> isRelOpen()) return;

  if (!startWorkerStreamCommunication())
    {
      setErrorText("Could not initiate communication!");
      return;
    }

  //Element is copied on the worker
  string cmd = "let r" + getNewName() + getIndexStr() + " = r" 
    + getWorker() -> getName() + getIndexStr(); 

  string err;
  if (!sendSecondoCmdToWorker1(cmd, err))
    { 
      err = "Could not start copy function on worker!\n" + err;
      setErrorText(err);
      return;
    }

#if DS_CMD_COPY_DEBUG
  cout << "DServerCmdCopy::run DONE" << endl;
#endif
} // run()
