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

#include "DServerCmdExecute.h"

/*
  
2 Implementation

2.1 Method ~void run~

implements the actual execute - functionality on the data of input darray
object, returning a new darray object.
 
*/

void
DServerCmdExecute::run()
{ 
#if DS_CMD_COPY_DEBUG
  cout << "DServerCmdExecute::run" << endl;
#endif

  if (!checkWorkerAvailable())
    return;

  if(getWorker() -> isRelOpen()) return;

  if (!startWorkerStreamCommunication())
    {
      setErrorText("Could not initiate communication!");
      return;
    }

  string execCmd = getCmd();
  const unsigned long soSize = getSourceObjectSize();

  for (unsigned long i = soSize; i-- > 0;)
    {
      const string& sourceObject = getSourceObject(i);
      // setup replace string
      string rpl = "!";
      for (unsigned long k =0; k < i; ++k)
        rpl += "!";
      
      execCmd = stringutils::replaceAll(execCmd, rpl,
                                        "r" +
                                        sourceObject + getIndexStr());
    }


  execCmd = 
    stringutils::replaceAll(execCmd, "d_idx", 
                            "int " + getIndexStr());

  //A command is executed on the worker and assigned
  //to the new DArray Objekt on that worker
  string cmd;
  cmd = "(let r" + getNewName() + getIndexStr() + " = " + execCmd + ")";

  if (!sendSecondoCmdToWorkerNL(cmd))
    { 
      string err;
      if (hasCmdError())
        err = getCmdErrorText();
      else
        err = "Could not execute this command on worker:\n" + cmd;
      setErrorText(err);
    }

#if DS_CMD_COPY_DEBUG
  cout << "DServerCmdExecute::run DONE" << endl;
#endif
} // run()
