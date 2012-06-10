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

[1] Class DServerShuffleReceiver Implementation

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

Implementation of the class ~DServerShuffleReceiver~

1 Preliminaries

1.1 Includes

*/
#include "DServerShuffleReceiver.h"
#include "DServerCmdCallBackComm.h"
#include "RelationAlgebra.h"
#include "DBAccessGuard.h"

/*
1.2 Debug Output

uncomment the following line for debug output

*/
//#define SHUFFLE_RECEIVE_DEBUG 1

/*
2 Implementation

2.1 Method ~void run~

run method of the thread

*/

void  DServerShuffleReceiver::run()
{
#ifdef SHUFFLE_RECEIVE_DEBUG
  string recType;
  DBAccessGuard::getInstance() -> NL_ToString(m_recType, recType);
  cout << "Starting Receiver: " <<  m_runit
       << " for " << m_destHost << ":" << m_toPort 
       << " TT:" << recType << endl;

  int dbg_count = 0;
#endif
  DServerCmdCallBackCommunication *dscCallBack = 
    new DServerCmdCallBackCommunication(m_destHost, m_toPort
#ifdef SHUFFLE_RECEIVE_DEBUG
                                        , "RECEIVER RUN "
#endif
                                        );

  if (dscCallBack -> createGlobalSocket())
    {
      bool noError = true;

      TupleType *tt = DBAccess::getInstance() -> TT_New(m_recType);

      while(m_runit)
        {
          m_runit = 
            dscCallBack -> 
               getTagFromCallBackTF("SENDTUPLE", "NOTUPLE", noError);

          if (m_runit)
            {
              dscCallBack -> readTupleFromCallBack(tt, m_rel);
#ifdef SHUFFLE_RECEIVE_DEBUG 
              cout << "Rec:" << m_destHost << ":" << m_toPort 
                   << " - " << dbg_count << endl;
              dbg_count ++;
#endif
            }
          else
            {
              m_runit = false;
            }
           
          if (noError)
            {
              m_runit = 
                dscCallBack -> 
                   getTagFromCallBackTF("NEXT", "END", noError);

              if (!noError)
                {
                  m_runit = false;
                }
            }
          else
            {
              m_runit = false;
            }
        } // while
    
      DBAccess::getInstance() -> TT_DeleteIfAllowed(tt);
      dscCallBack -> sendTagToCallBack("CLOSE");

    } // dsc

  delete dscCallBack;

#ifdef SHUFFLE_RECEIVE_DEBUG
  cout << "RECEIVER IS DONE from " << m_toPort << "@" << m_destHost << endl;
#endif
} 
 
