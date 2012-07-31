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
[1] Class DServerCmdReadRel Definition

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmdReadRel~ reads the data of an DArray of a relation
and stores it on the workres.

The class ~DServerCmdReadRelParam~ is a data class for the parameters used 
during the execution of a ~DServerCmdReadRel~ object.

*/

/*
1 Preliminaries

1.1 Defines

*/
#ifndef H_DSERVERCMDREADREL_H
#define H_DSERVERCMDREADREL_H

/*
1.2 Includes

*/
#include "DServerCmd.h"
#include "DBAccessGuard.h"

class DServerCmdCallBackCommunication;
class ThreadedMemoryCounter;
class TupleFifoQueue;

/*
3 Class ~DServerCmdReadRelParam~

The class ~DServerCmdReadRelParam~ contains the parameters used in
the ~run~ - method of the class ~DServerCmdReadRel~.

  * derives from class ~DServerParam~

Provided parameters:

  * vector[<]Word[>][ast] m[_]outElements - pointer to the global 
storage container for the elements of this darray



*/
class DServerCmdReadRelParam 
  : public DServerParam
{
/*
3.1 Private Default Constructor

  * may not be used!

*/
  DServerCmdReadRelParam() {}
/*
3.2 Constructor for send relation

  * vector[<]Word[>][ast] inElements - pointer to the global 
storage container for the elements of this darray


*/
public:
  DServerCmdReadRelParam(vector<Word>* outElements,
                         vector<bool>* outIsPresent,
                         ListExpr inDaType)
    : DServerParam()  
    , m_outElements(outElements)
    , m_outIsPresent(outIsPresent)
    , m_tupleQueue(NULL)
    , m_memCntr(NULL)
  { m_ttype= DBAccessGuard::getInstance() -> TT_New(inDaType); }

/*
3.4 Constructor for ddistribute
used, if multiple indexes are transferred

  * vector[<]DServerMultiCommand[ast][>][ast] inTupleFifoQueues - 
pointer to the tuple fifo queues for each worker (child)

  * const string[&] inSendType - string format of the send type nested list

  * const string[&] inDelIndex - string format of the attribute index, which
will be deleted


*/
  
  DServerCmdReadRelParam(TupleFifoQueue*& inTFQ,
                         ThreadedMemoryCounter*& inMemCntr,
                         ListExpr inDaType)
    : DServerParam()  
    , m_outElements(NULL)
    , m_outIsPresent(NULL)
    , m_tupleQueue(inTFQ)
    , m_memCntr(inMemCntr) 
  { m_ttype= DBAccessGuard::getInstance() -> TT_New(inDaType); }

/*
3.4 Copy - Constructor

*/
  DServerCmdReadRelParam(const DServerCmdReadRelParam & inP)
    : DServerParam(inP)
    , m_outElements(inP.m_outElements)
    , m_outIsPresent(inP.m_outIsPresent)
    , m_tupleQueue(inP.m_tupleQueue)
    , m_memCntr(inP.m_memCntr)
    , m_ttype(inP.m_ttype)
  {m_ttype -> IncReference();}

/*
3.5 Destructor

*/
  virtual ~DServerCmdReadRelParam() 
  { DBAccessGuard::getInstance() -> TT_DeleteIfAllowed(m_ttype); }

/*
3.6 Getter Methods

3.6.1 Method ~vector[<]Word[>][ast] getOutElements const~

  * returns vector[<]Word[>][ast] - pointer to the global storage array

*/
  vector<Word>* getOutElements() const { return m_outElements; }

/*
3.6.1 Method ~vector[<]bool[>][ast] getOutIsPresent const~

  * returns vector[<]bool[>][ast] - pointer to the global storage array

*/
  vector<bool>* getOutIsPresent() const { return m_outIsPresent; }

/*
3.6.1 Method ~TupleFifoQueue[ast] getTFQ const~

  * returns TupleFifoQueue[ast] - pointer to the global tfq

*/

  TupleFifoQueue* getTFQ() const { return m_tupleQueue; }

/*
3.6.1 Method ~vector[<]Word[>][ast] getinElements const~

  * returns vector[<]Word[>][ast] - pointer to the global storage array

*/
  ThreadedMemoryCounter* getMemCntr() const {return m_memCntr; }

/*
3.6.3 Method ~TupleType[ast] getTType const~

  * returns const TupleType[ast] - the tuple type used for receiving 

*/

  TupleType* getTType() const { return m_ttype; }

  bool useChilds() const { return true; }


/*
3.6 Private Section

*/
private:
/*
3.6.1 Private Methods

*/
// n/a
/*
3.6.1 Private Members

*/
  vector<Word>* m_outElements;
  vector<bool>* m_outIsPresent;
  TupleFifoQueue* m_tupleQueue;
  ThreadedMemoryCounter* m_memCntr;
  TupleType* m_ttype;

/*
3.7 End of Class

*/
};

/* 
4 Class ~DServerCmdReadRel~

The class ~DServerCmdReadRel~ provides the functionality of writing data of
relational DArray elements to the workers

  * derives from the class ~DServerCmd~

*/
class DServerCmdReadRel
  : public DServerCmd
{
/*
4.1 Private Default Constructor

  * inherited from the class ~DServerCmd~

  * may not be used!

*/
public:
/*
4.1 Constructor

*/

  DServerCmdReadRel()
    : DServerCmd(DServerCmd::DS_CMD_READ_REL)
  {}

/*
4.2 Destructor

*/

  virtual ~DServerCmdReadRel() {}


/*
4.3 Getter Methods

4.3.1 Method ~vector[<]Word[>][ast] getOutElements const~

  * returns vector[<]Word[>][ast] - pointer to the global storage array

*/
  
  vector<Word>* getOutElements() const 
  {
    const DServerCmdReadRelParam *p = 
      DServerCmd::getParam<DServerCmdReadRelParam>() ;
    return p -> getOutElements();
  }
/*

4.3.1 Method ~vector[<]bool[>][ast] getOutIsPresent const~

  * returns vector[<]bool[>][ast] - pointer to the global storage array

*/

  vector<bool>* getOutIsPresent() const 
  {
    const DServerCmdReadRelParam *p = 
      DServerCmd::getParam<DServerCmdReadRelParam>() ;
    return p -> getOutIsPresent();
  }

/*
3.6.1 Method ~TupleFifoQueue[ast] getTFQ const~

  * returns TupleFifoQueue[ast] - pointer to the global tfq

*/

  TupleFifoQueue* getTFQ() const 
  {
    const DServerCmdReadRelParam *p = 
      DServerCmd::getParam<DServerCmdReadRelParam>() ;
    return p ->  getTFQ();
  }

/*
3.6.1 Method ~ThreadedMemoryCounter[ast] getMemCntr const~

  * returns ThreadedMemoryCounter[ast] - pointer to the memory coutner objecct

*/
  ThreadedMemoryCounter* getMemCntr() 
  {
    const DServerCmdReadRelParam *p = 
      DServerCmd::getParam<DServerCmdReadRelParam>() ;
    return p -> getMemCntr();
  }


/*
3.6.1 Method ~TupleType[ast] getTTypeconst~

  * returns TupleType[ast] - pointer to of the TupleType

*/
  TupleType* getTType() const
  {
    const DServerCmdReadRelParam *p = 
      DServerCmd::getParam<DServerCmdReadRelParam>() ;
    return p -> getTType();
  }
/*
4.3.8 Method ~string getInfo const~

  * returns string - an informaiton string

*/
  string getInfo() const 
  {
    return string("CMD-Read Rel to:"  + getWorker() -> getName() + 
                  getIndexStr());
  }

/*

4.4 Running

4.4.1 Method ~void run~

method definition

*/

  void run();

/*
4.5 Private Section

*/
private:

/*
4.5.1 Private Methods

*/

  bool receivingRelation() const {  return (getOutElements() != NULL); }

  bool recRelation(DServerCmdCallBackCommunication &callBack);

  bool recTupleQueue(DServerCmdCallBackCommunication &callBack);

// n/a

/*
4.5.2 Private Members

*/
// n/a

/*
4.6 End of Class

*/

};

#endif // H_DSERVERCMDREADREL_H
