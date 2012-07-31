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
[1] Class DServerCmdDelete Implementation

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

Implementation of the class ~DServerCmdDelete~

1 Preliminaries

1.1 Debug Output

uncomment the following line

*/
//#define DS_CMD_DELETE_DEBUG 1

/*
1.2 Includes

*/

#include "DServerCmdDelete.h"

/*
  
2 Implementation

2.1 Method ~void run~

implements the actual delete functionality of the data of  one darray index
 
*/

void
DServerCmdDelete::run()
{ 
#if DS_CMD_DELETE_DEBUG
  cout << "DServerCmdDelete::run" << endl;
#endif

  if (!checkWorkerAvailable())
    return;

  if (!startWorkerStreamCommunication())
    {
      setErrorText("Could not initiate communication!");
      return;
    }

  while (nextIndex())
    {
      //Element is copied on the worker
      string cmd = "delete r" + getWorker() -> getName() + getIndexStr(); 

      bool retVal = sendSecondoCmdToWorkerSOS(cmd);
      if (!retVal)
        { 
          string err;
          if (hasCmdError())
            err = getCmdErrorText();
          else
            err = "Could not start delete function on worker!";
          setErrorText(err);
          return;
        }
    }

  if (!closeWorkerStreamCommunication())
    {
      setErrorText("Could not stop communication!");
      return;
    }

#if DS_CMD_DELETE_DEBUG
  cout << "DServerCmdDelete::run DONE" << endl;
#endif
} // run()
