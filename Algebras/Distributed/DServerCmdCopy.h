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
[1] Class DServerCmdCopy Definition

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmdCopy~ provides the functionality to copy the data of a
darray index to antoher darray, keeping the index. It is used in the 
DArray::Clone() method and in the implementation of the ~put~ operator.
This class derives from ~DServerCmd~ class, which provides the basic
communication and run functionality.

The class ~DServerCmdCopyParam~ is a data class for the parameters used 
during the execution of a ~DServerCmdCopy~ object.

*/

/*
1 Preliminaries

1.1 Defines

*/
#ifndef H_DSERVERCMDCOPY_H
#define H_DSERVERCMDCOPY_H

/*
1.2 Includes

*/
#include "DServerCmd.h"

/*
2 Class ~DServerCmdCopyParam~

The class ~DServerCmdCopyParam~ contains the parameters used in
the ~run~ - method of the class ~DServerCmdCopy~.

  * derives from class ~DServerParam~

Provided parameters:

  * string m[_]newName - new DArray data name 
  
  * int m[_]replaceIndex - index, which will be replaced by the put operator

*/
class DServerCmdCopyParam 
  : public DServerParam
{
/*
2.1 Private Default Constructor

  * may not be used!

*/
  DServerCmdCopyParam() {}
/*
2.2 Constructor

  * const string[&] inNewName - new name for the darray index data

  * int inReplaceIndex - data index, which is replaced in the put operator
  (default: -1 - all data is copied)

*/
public:
  DServerCmdCopyParam(const string& inNewName,
                      int inReplaceIndex = -1)
    : DServerParam()  
    , m_newName(inNewName)
    , m_replaceIndex(inReplaceIndex)
  {}
/*
2.3 Copy - Constructor

*/
  DServerCmdCopyParam(const DServerCmdCopyParam & inP)
    : DServerParam(inP)
    , m_newName(inP.m_newName)
    , m_replaceIndex(inP.m_replaceIndex) {}
/*
2.4 Destructor

*/
  virtual ~DServerCmdCopyParam() {}
/*
2.5 Getter Methods

2.5.1 Method ~const string[&] getNewName const~

  * returns const string[&] - the new darray data name

*/
  const string& getNewName() const { return m_newName; }
/*
2.5.2 Method ~int getReplaceIndex const~

  * returns int - index, which should be replaced by the ~put~ operator

*/
  int getReplaceIndex() const { return m_replaceIndex; }

/*
2.6 Private Section

*/
private:
/*
2.6.1 Private Methods

*/
// n/a

/*
2.6.1 Private Members

*/
  string m_newName;
  int m_replaceIndex;
/*
2.7 End of Class

*/
};

/* 
3 Class ~DServerCmdCopy~

The class ~DServerCmdCopy~ provides the functionality of copying data of a
darray index to an index of another darray. It is also used for the ~put~
operator.This operator also performs a copy of the darray data, skipping 
data of one index and replaceing it with new data

  * derives from the class ~DServerCmd~

*/
class DServerCmdCopy 
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

  DServerCmdCopy(DServer *inWorker, int inIndex)
    : DServerCmd(DServerCmd::DS_CMD_COPY, inWorker, inIndex)
  {}

/*
3.2 Destructor

*/

  virtual ~DServerCmdCopy() {}


/*
3.3 Getter Methods

3.3.1 Method ~const string[&] getNewName const~

  * returns const string[&] - the new darray name

*/
  
  const string& getNewName() const 
  { 
    const DServerCmdCopyParam *p = 
      DServerCmd::getParam<DServerCmdCopyParam>() ;
    return p -> getNewName();
  }
/*
3.3.2 Method ~int getReplaceIndex const~

  * returns int - the index, which will be replaced
(is -1, if whole darray is copied)

*/
  int getReplaceIndex() const 
  { 
    const DServerCmdCopyParam *p = 
      DServerCmd::getParam<DServerCmdCopyParam>() ;
    return p -> getReplaceIndex(); 
  }

/*
2.3.3 Method ~string getInfo const~

  * returns string - an informaiton string

*/
  string getInfo() const 
  {
    return string("CMD-Copy from:"  + getWorker() -> getName() + 
                  getIndexStr() + " to " +  getNewName() + getIndexStr());
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

#endif // H_DSERVERCMDCOPY_H
