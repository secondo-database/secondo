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
[1] Class DServerThreadRunner Definition


\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerThreadRunner~ is a base class for all classes derived
from the class ~DServerCmd~ which should run threaded. Therefor it derives from
the  ~ZThread::Runnable~ class and defines the pure virtual ~run~ method.
It allows derived classes to access and modify the data on a remote worker of a
specific darray index. It also derives from the class ~DServerParamStorage~
to use specific parameters while modifying the darray data on the worker.
This class is defined pure virtual and serves as an interface 
definition for derived classes. As only data members it stores a pointer
to the worker object where the darray index data is stored and an
integer index to denote the darray index.

*/

/*

1 Preliminaries

1.1 Defines

*/

#ifndef H_DSERVERTHREADRUNNER_H
#define H_DSERVERTHREADRUNNER_H

/*
1.2 Includes

*/


#include "Remote.h"
#include "DServerParamStorage.h"

/*
1.3 Extern defintions

*/
extern string toString_d(int);

/*
2 Class ~DServerThreadRunner~

This class provides the basic functionality for running as thread. It stores
and index of the darray, where it is run on and also 
stores a pointer for the worker, where the darray data of this index is 
located. It also derives from the class ~DServerParamStorage~ to store
parameters used in the command.

  * derives from class ~ZThread::Runnable~

  * derives from class ~DServerParamStorage~

*/
class DServerThreadRunner 
  : public ZThread::Runnable
  , public DServerParamStorage
{
/*
2.1 Private Default Constructor

  * may not be used!

*/
  DServerThreadRunner() 
  : DServerParamStorage()
  , m_worker(NULL)
  , m_index(-1) {}
/*
2.2 Constructor

  * DServer[ast] inWorker - pointer to the worker object, where data is stored
  
  * int inIndex - data index of the darray

*/
public:
  DServerThreadRunner(DServer *inWorker, 
                      int inIndex)
    : DServerParamStorage()
    , m_worker(inWorker)
    , m_index(inIndex) {}
/*
2.2 Destructor

*/
  virtual ~DServerThreadRunner() {}
  
/*
2.3 Getter Methods

2.3.1 Method ~DServer[ast] getWorker const~

  * returns DServer[ast] - pointer to the worker object

*/
  DServer* getWorker() const { return m_worker; }

/*
2.3.2 Method ~int getIndex const~

  * returns int - the index of the darray

*/
  int getIndex() const { return m_index; }

/*
2.3.3 Method ~string getIndexStr const~

  * returns string - the index of the darray as string

*/
  string getIndexStr() const { return toString_d(m_index); }

/*
2.3.4 Method ~virtual string getInfo const = 0~

interface definition to retrieve an infromation string

  * returns string - an information string

*/
  virtual string getInfo() const = 0;

/*
2.6 Method ~virtual void run = 0~

interface definition for the run method

*/

  virtual void run() = 0;

/*
2.7 Error Handling

2.7.1 Method ~void setErrorText~

forwards the error message to the worker

  * const string[&] inErrTxt - the error msg

*/

  void setErrorText(const string& inErrTxt)
  {
    getWorker() -> setErrorText(inErrTxt);
  }
  
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
  DServer* m_worker;
  int m_index;
/*
2.9 End of Class 

*/
};

#endif // H_DSERVERTHREADRUNNER_H
