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

[1] Class DServerShuffleSender Implementation

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

Implementation of the class ~DServerShuffleSender~

1 Preliminaries

1.1 Includes

*/
#include "DServerShuffleSender.h"
#include "DServerCmdCallBackComm.h"
#include "RelationAlgebra.h"
#include "ThreadedMemoryCntr.h" 

/*
1.2 Debug Output

uncomment the following line for debug output

*/
//#define SHUFFLE_SENDER_DEBUG 1

/*
2 Implementation

2.1 Method ~void run~

run method of the thread

*/


void  DServerShuffleSender::run()
{
#ifdef SHUFFLE_SENDER_DEBUG
  cout << "Starting Sender:" << " " << m_runit 
       << "for " << m_destHost << ":" << m_toPort << endl;
#endif
  DServerCmdCallBackCommunication *dscCallBack = 
     new DServerCmdCallBackCommunication(m_destHost, m_toPort

#ifdef SHUFFLE_SENDER_DEBUG
                                         , "SENDER RUN "
#endif
                                         );

  if (!dscCallBack -> startSocket())
    {
      cerr << "Error Connecting Sender to " 
           << m_destHost << ":" << m_toPort << endl;
    }

  else if(dscCallBack -> startSocketCommunication())
     {
       // no connected to 
       Tuple *t;
       while(m_runit || !m_tfq.empty())
         {
           t = m_tfq.get();
           
           if (t != NULL)
             {
               dscCallBack -> sendTagToCallBack("SENDTUPLE");
               dscCallBack -> writeTupleToCallBack(t);
               m_memCntr -> put_back(t -> GetSize());
             }
           else 
             {
               dscCallBack -> sendTagToCallBack("NOTUPLE");
             }
           
           if (m_runit ||  !m_tfq.empty())
             {
               dscCallBack -> sendTagToCallBack("NEXT");
             }
           else
             {
               dscCallBack -> sendTagToCallBack("END");
             }
         } // while
       
            
       dscCallBack -> getTagFromCallBack("CLOSE");
     } // dsc
   else
     {
       cerr << "SHUFFLE SEND: COULD NOT CREATE GLOBAL SOCKET!" << endl;
     }

   delete dscCallBack;

#ifdef SHUFFLE_SENDER_DEBUG
   cout << "SHUFFLE SENDER IS DONE! "
        << " to " << m_toPort << "@" << m_destHost << endl;
#endif
} //
 
