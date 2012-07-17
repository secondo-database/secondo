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
[1] Class DServerCmdWrite Definition

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmdWrite~ writes the data of an DArray of atomic values
and stores it on the workres.

The class ~DServerCmdWriteParam~ is a data class for the parameters used 
during the execution of a ~DServerCmdWrite~ object.

*/

/*
1 Preliminaries

1.1 Defines

*/
#ifndef H_DSERVERCMDWRITE_H
#define H_DSERVERCMDWRITE_H

/*
1.2 Includes

*/
#include "DServerCmd.h"

/*
2 Class ~DServerCmdWriteParam~

The class ~DServerCmdWriteParam~ contains the parameters used in
the ~run~ - method of the class ~DServerCmdWrite~.

  * derives from class ~DServerParam~

Provided parameters:

  * vector[<]Word[>][ast] m[_]outElements - pointer to the global 
storage container for the elements of this darray

  * vector[<]bool[>][ast] m[_]outIsPresent - pointer of the global
container, indicating if a element is present at the master

  * vector[<]vector[<]int[>] [>] m[_]indexList - list of indexes to be 
transferred

This data structure maps the index of the DServerCmdWrite Object to the
indexes, which shall be transferred within the ~run~ methode. This is 
necessary, because for a DArray of atomic values these indexes differ.
One DServerCmdWrite object can handle multiple DArray indexes, if
data consists of atomic values.

Thus, if the atomic data of only one index is transferred, it 
is stored directly in the following variable.

  * int m[_]singleIndex - single DArray to avoid complex data structure

*/
class DServerCmdWriteParam 
  : public DServerParam
{
/*
2.1 Private Default Constructor

  * may not be used!

*/
  DServerCmdWriteParam() {}
/*
2.2 Constructor
used, if multiple indexes are transferred

  * const vector[<]vector[<]int[>] [>] inIndexList - list of indexs to be 
transferred

  * vector[<]Word[>][ast] outElements - pointer to the global 
storage container for the elements of this darray


*/
public:
  DServerCmdWriteParam(const vector<vector<int> >& inIndexList,
                       vector<Word>* inElements)
    : DServerParam()  
    , m_inElements(inElements)
    , m_indexList(inIndexList)
    , m_singleIndex(-1) {}  
/*
2.3 Constructor
used, if single index is transferred

  * int inIndex - DArray index

  * vector[<]Word[>][ast] outElements - pointer to the global 
storage container for the elements of this darray


*/
  DServerCmdWriteParam(int inIndex,
                       vector<Word>* inElements)
    : DServerParam()  
    , m_inElements(inElements)
    , m_singleIndex(inIndex) {} 
  
/*
2.4 Copy - Constructor

*/
  DServerCmdWriteParam(const DServerCmdWriteParam & inP)
    : DServerParam(inP)
    , m_inElements(inP.m_inElements)
    , m_indexList(inP.m_indexList)
    , m_singleIndex(inP.m_singleIndex) {}

/*
2.5 Destructor

*/
  virtual ~DServerCmdWriteParam() {}

/*
2.6 Getter Methods

2.6.1 Method ~vector[<]Word[>][ast] getinElements const~

  * returns vector[<]Word[>][ast] - pointer to the global storage array

*/
  vector<Word>* getInElements() const { return m_inElements; }


/*
2.6.3 Method ~int getIndexAt const~
automatically falls back to the single index, if available

  * unsigned long j - the index of this DServerCmdWrite object

  * unisigned long i - the position of the container, which is transferred

  * returns int - the DArray index, which shall be transferred.

*/
  const int getIndexAt(unsigned long j, unsigned long i) const 
  { if (hasSingleIndex()) return getSingleIndex(); return m_indexList[j][i]; }

/*
2.6.4 Method ~unsigned long getIndexListSize const~
automatically falls back to 1, if onlye one index is set

  * unsigned long j - the index of this DServerCmdWrite object

  * returns unsigned long - the size of indexes, which are transferred

*/
  unsigned long getIndexListSize(unsigned long i) const 
  {
    if (hasSingleIndex()) return 1;
    return m_indexList[i].size(); 
  }


/*
2.6 Private Section

*/
private:
/*
2.6.1 Private Methods

*/

  bool hasSingleIndex() const { return m_singleIndex > -1; }

  int getSingleIndex() const { return m_singleIndex; }

/*
2.6.1 Private Members

*/
  vector<Word>* m_inElements;
  vector< vector<int> > m_indexList;
  int m_singleIndex;

/*
2.7 End of Class

*/
};

/* 
3 Class ~DServerCmdWrite~

The class ~DServerCmdWrite~ provides the functionality of writing data of
atomic DArray elements to the workers

  * derives from the class ~DServerCmd~

*/
class DServerCmdWrite 
  : public DServerCmd
{
/*
3.1 Private Default Constructor

  * inherited from the class ~DServerCmd~

  * may not be used!

*/
public:
/*
3.1 Constructor

*/

  DServerCmdWrite(DServer *inWorker, int inIndex)
    : DServerCmd(DServerCmd::DS_CMD_WRITE, inWorker, inIndex)
  {}

/*
3.2 Destructor

*/

  virtual ~DServerCmdWrite() {}


/*
3.3 Getter Methods

3.3.1 Method ~vector[<]Word[>][ast] getOutElements const~

  * returns vector[<]Word[>][ast] - pointer to the global storage array

*/
  
  vector<Word>* getInElements() const 
  {
    const DServerCmdWriteParam *p = 
      DServerCmd::getParam<DServerCmdWriteParam>() ;
    return p -> getInElements();
  }

/*
3.3.3 Method ~int getIndexAt const~
automatically falls back to the single index, if available

  * unisigned long i - the position of the container, which is transferred

  * returns int - the DArray index, which shall be transferred.

*/
const int getIndexAt(unsigned long i) const 
  { 
    const DServerCmdWriteParam *p = 
      DServerCmd::getParam<DServerCmdWriteParam>() ;
    return p -> getIndexAt(getIndex(), i); 
  }


/*
3.3.4 Method ~unsigned long getIndexListSize const~
automatically falls back to 1, if onlye one index is set

  * returns unsigned long - the size of indexes, which are transferred

*/
unsigned long getIndexListSize() const 
  { 
    const DServerCmdWriteParam *p = 
      DServerCmd::getParam<DServerCmdWriteParam>() ;
    return p -> getIndexListSize(getIndex()); 
  }

/*
3.3.5 Method ~string getInfo const~

  * returns string - an informaiton string

*/
  string getInfo() const 
  {
    return string("CMD-Write to:"  + getWorker() -> getName() + 
                  getIndexStr());
  }

/*

3.4 Running

3.4.1 Method ~void run~

method definition

*/

  void run();

/*
3.5 Private Section

*/
private:

/*
3.5.1 Private Methods

*/
// n/a

/*
3.5.2 Private Members

*/
// n/a

/*
3.6 End of Class

*/

};

#endif // H_DSERVERCMDWRITE_H
