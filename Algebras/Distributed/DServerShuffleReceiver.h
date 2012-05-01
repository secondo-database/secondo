
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
[1] Class DServerShuffleReceiver Definition

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description


The class DServerShuffleReceiver gets a tuple from the sender worker
and stores it in the relation.


1 Preliminaries

1.1 Defines

*/
#ifndef H_DSERVERSHUFFLERECEIVER_H
#define H_DSERVERSHUFFLERECEIVER_H

/*
1.2 Includes

*/

#include "zthread/Runnable.h"
#include "ThreadedMemoryCntr.h"
#include "NList.h"
/*
1.3 Forward Declarations

*/
class GenericRelation;

/*
2 Class DServerShuffleReceiver

The class ~DServerShuffleReceiver~ is receiving tuples from
the sender functions and stores the daata in a relation.

  * derives from the class ~ZThread::Runnable~

*/

class DServerShuffleReceiver : public ZThread::Runnable
{
public:
/*
2.1 Default Constructor

  * Automatically created

2.2 Constructor

  * const string[&] inDestHost - the host name of the worker, where 
the new darray index data is retrieved

  * const string[&] inToPort - the corresponding port number (as string)

  * GenericRelation[ast] inRel - a pointer to a the relation, where the data 
is stored

  * ListExpr inResultType - the result type as list expression

*/
  DServerShuffleReceiver(const std::string& inDestHost,
                         const std::string& inToPort,
                         GenericRelation *inRel,
                         ListExpr inResultType) 
    : m_destHost(inDestHost)
    , m_toPort(inToPort)
    , m_runit(true)
    , m_rel(inRel)
    , m_recType(inResultType){}

/*
2.3 Destructor

*/
  virtual ~DServerShuffleReceiver() {}

/*
2.6 Running

2.6.1 Method ~void run~

*/

  void run();

/*
2.8 Private Section

*/
private:

/*
2.8.1 Private Methods

*/

// n/a

/*
2.8.1 Private Members

*/
  std::string m_destHost;
  std::string m_toPort;
  bool m_runit;
  GenericRelation *m_rel;
  ListExpr m_recType;

/*
2.9 End of Class 

*/

};

#endif // H_DSERVERSHUFFLERECEIVER_H
