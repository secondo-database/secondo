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
[1] Class DServerCmdWriteRel Definition

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmdWriteRel~ writes the data of an DArray of a relation
and stores it on the workres.

The class ~DServerCmdWriteRelParam~ is a data class for the parameters used 
during the execution of a ~DServerCmdWriteRel~ object.

*/

/*
1 Preliminaries

1.1 Defines

*/
#ifndef H_DSERVERCMDWRITEREL_H
#define H_DSERVERCMDWRITEREL_H

/*
1.2 Includes

*/
#include "DServerCmd.h"
#include "TupleFifoQueue.h"
#include "ThreadedMemoryCntr.h"

class DServerCmdCallBackCommunication;

/*
3 Class ~DServerMultiCommand~

The class ~DServerMultiCommand~ manages a TupleFifoQueue in combination
with an extern  ThreadedMemoryCounter Objetct. It is used for
running a threaed tuple queue for a singel worker 
in the class ~DServerCmdWriteRel~

Provided parameters:

  * TupleFifoQueue m[_]tfq - the tuple fifo queue

  * bool m[_]runit - flag, if the filler thread is still running

  * ThreadedMemoryCounter[ast] m[_]memCntr - pointer to theextern
ThreadedMemoryCounter Object



*/

class DServerMultiCommand
{
/*
3.2 Constructor 

  * ThreadedMemoryCounter[ast] inMemCntr - pointer to the 
external ThreadedMemoryCounter object


*/
public:
  DServerMultiCommand(ThreadedMemoryCounter* inMemCntr) :
    m_runit(true),
    m_memCntr(inMemCntr)
  { 
  }

/*
3.2 Destructor 

*/
  virtual ~DServerMultiCommand() {}

/*
3.3 public Methods

3.3.1  Method ~void AppendTuple~
  
  * Tuple[ast] t - the tuple to be inserted into the worker's queue

*/
  void AppendTuple(Tuple* t)
  {
    m_tfq.put(t);
  }

/*
3.3.2  Method ~void done~
  
  sets this class into a done state

  * sets the m[_]runit state into false

  * appends a NULL-pointer to the queue

*/
  void done()
  {
    m_runit = false;
    m_tfq.put(NULL); // dummy to wake up waiting threads
  }

/*
3.3.3 Method ~Tuple[ast] GetTuple~
  
  takes a tuple out of the queue and frees memory in the
m[_]memCntr objetc

  * returns Tuple[ast] - a pointer to a tuple, NULL when done

*/
  Tuple * GetTuple()
  { 
    Tuple *t = m_tfq.get();
    if (t != NULL)
      {
        m_memCntr -> put_back(t -> GetSize());
      }
    return t;
  }

/*
3.3.4 Method ~bool IsDone~
  
  * returns bool - true: still running

*/
  bool IsDone() const { return m_runit; }
  
/*
3.4 Private Section

*/
private:
/*
3.4.1 Private Methods

*/
//n/a
/*
3.4.2 Private Members

*/
  TupleFifoQueue m_tfq;
  bool m_runit;
  ThreadedMemoryCounter* m_memCntr;

/*
3.5 End of Class 

*/
};

/*
3 Class ~DServerCmdWriteRelParam~

The class ~DServerCmdWriteRelParam~ contains the parameters used in
the ~run~ - method of the class ~DServerCmdWriteRel~.

  * derives from class ~DServerParam~

Provided parameters:

  * vector[<]Word[>][ast] m[_]outElements - pointer to the global 
storage container for the elements of this darray



*/
class DServerCmdWriteRelParam 
  : public DServerParam
{
/*
3.1 Private Default Constructor

  * may not be used!

*/
  DServerCmdWriteRelParam() {}
/*
3.2 Constructor for send relation

  * vector[<]Word[>][ast] inElements - pointer to the global 
storage container for the elements of this darray


*/
public:
  DServerCmdWriteRelParam(vector<Word>* inElements)
    : DServerParam()  
    , m_inElements(inElements)
    , m_tupleQueue(NULL)
    , m_ddistrSendTypeStr("EMPTY")
    , m_delIndex("EMPTY")
    , m_usesChilds(false){}

/*
3.4 Constructor for ddistribute
used, if multiple indexes are transferred

  * vector[<]DServerMultiCommand[ast][>][ast] inTupleFifoQueues - 
pointer to the tuple fifo queues for each worker (child)

  * const string[&] inSendType - string format of the send type nested list

  * const string[&] inDelIndex - string format of the attribute index, which
will be deleted


*/
  DServerCmdWriteRelParam(vector<DServerMultiCommand *>* inTupleFifoQueues,
                          const string& inSendType,
                          const string& inDelIndex)
    : DServerParam()  
    , m_inElements(NULL)
    , m_tupleQueue(inTupleFifoQueues)
    , m_ddistrSendTypeStr(inSendType)
    , m_delIndex(inDelIndex)
    , m_usesChilds(true) {}
/*
3.4 Copy - Constructor

*/
  DServerCmdWriteRelParam(const DServerCmdWriteRelParam & inP)
    : DServerParam(inP)
    , m_inElements(inP.m_inElements)
    , m_tupleQueue(inP.m_tupleQueue)
    , m_ddistrSendTypeStr(inP.m_ddistrSendTypeStr)
    , m_delIndex(inP.m_delIndex)
    , m_usesChilds(inP.m_usesChilds){}

/*
3.5 Destructor

*/
  virtual ~DServerCmdWriteRelParam() {}

/*
3.6 Getter Methods

3.6.1 Method ~vector[<]Word[>][ast] getinElements const~

  * returns vector[<]Word[>][ast] - pointer to the global storage array

*/
  vector<Word>* getInElements() const { return m_inElements; }

/*
3.6.2 Method ~DServerMultiCommand[ast] getTupleQueue const~

  * int inIdx - the current index of the darray

  * returns vector[<]Word[>][ast] - pointer to the global storage array

*/
  DServerMultiCommand* getTupleQueue(int inIdx) const 
  { return m_tupleQueue -> operator [] (inIdx); }

/*
3.6.3 Method ~const string[&] getSendType const~

  * returns const string[&] - the tuple type used for sending 

*/
  const string & getSendType() const { return m_ddistrSendTypeStr; }

/*
3.6.4 Method ~const string[&] getDelIndex const~

  * returns const string[&] - the index, which will be deleted

*/
  const string & getDelIndex() const { return m_delIndex;}

/*
2.3.4 Method ~bool useChilds() const~

  * depends on the setup

*/
  bool useChilds() const { return m_usesChilds; }


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
  vector<Word>* m_inElements;
  vector<DServerMultiCommand*>* m_tupleQueue;
  string m_ddistrSendTypeStr;
  string m_delIndex;
  bool m_usesChilds;

/*
3.7 End of Class

*/
};

/* 
4 Class ~DServerCmdWriteRel~

The class ~DServerCmdWriteRel~ provides the functionality of writing data of
relational DArray elements to the workers

  * derives from the class ~DServerCmd~

*/
class DServerCmdWriteRel
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

  DServerCmdWriteRel()
    : DServerCmd(DServerCmd::DS_CMD_WRITE_REL)
  {}

/*
4.2 Destructor

*/

  virtual ~DServerCmdWriteRel() {}


/*
4.3 Getter Methods

4.3.1 Method ~vector[<]Word[>][ast] getInElements const~

  * returns vector[<]Word[>][ast] - pointer to the global storage array

*/
  
  vector<Word>* getInElements() const 
  {
    const DServerCmdWriteRelParam *p = 
      DServerCmd::getParam<DServerCmdWriteRelParam>() ;
    return p -> getInElements();
  }

/*
4.3.2 Method ~DServerMultiCommand[ast] getTupleQueue const~

  * int inIdx - the current index of the darray

  * returns vector[<]Word[>][ast] - pointer to the global storage array

*/
  DServerMultiCommand* getTupleQueue(int inIdx) const
{
    const DServerCmdWriteRelParam *p = 
      DServerCmd::getParam<DServerCmdWriteRelParam>() ;
    return p -> getTupleQueue(inIdx);
  }

/*
4.3.3 Method ~const string[&] getSendType const~

  * returns const string[&] - the tuple type used for sending 

*/
  const string & getSendType() const
  {
    const DServerCmdWriteRelParam *p = 
      DServerCmd::getParam<DServerCmdWriteRelParam>() ;
    return p -> getSendType() ;
  }

/*
4.3.4 Method ~const string[&] getDelIndex const~

  * returns const string[&] - the index, which will be deleted

*/
  const string & getDelIndex() const
 {
    const DServerCmdWriteRelParam *p = 
      DServerCmd::getParam<DServerCmdWriteRelParam>() ;
    return p -> getDelIndex();
 }

/*
4.3.8 Method ~string getInfo const~

  * returns string - an informaiton string

*/
  string getInfo() const 
  {
    return string("CMD-Write Rel to:"  + getWorker() -> getName() + 
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

  bool sendingRelation() const {  return (getInElements() != NULL); }

  bool sendRelation(DServerCmdCallBackCommunication &callBack);

  bool sendTupleQueue(DServerCmdCallBackCommunication &callBack);

// n/a

/*
4.5.2 Private Members

*/
// n/a

/*
4.6 End of Class

*/

};

#endif // H_DSERVERCMDWRITEREL_H
