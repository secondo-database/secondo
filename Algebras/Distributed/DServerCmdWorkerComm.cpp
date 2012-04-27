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
[1] Class DServerCmdWorkerCommunication Implementation

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

Implementation of the class ~DServerCmdWorkerCommunication~


1 Preliminaries

1.1 Includes

*/


#include "DServerCmdWorkerComm.h"

/*
  
2 Implementation

2.1 Method ~bool checkWorkerAvailable~

  * returns true, if worker is running
  
*/

bool 
DServerCmdWorkerCommunication::checkWorkerAvailable() const
{
  if ( m_worker -> getServer() == 0 ||
       !(m_worker -> getServer() -> IsOk()))
    {
      return false;
    }

  return true;
}
