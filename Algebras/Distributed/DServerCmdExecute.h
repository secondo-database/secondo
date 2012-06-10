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
[1] Class DServerCmdExecute Definition

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmdExecute~ provides the functionality to copy the data of a
darray index to antoher darray, keeping the index. It is used in the 
DArray::Clone() method and in the implementation of the ~put~ operator.
This class derives from ~DServerCmd~ class, which provides the basic
communication and run functionality.

The class ~DServerCmdExecuteParam~ is a data class for the parameters used 
during the execution of a ~DServerCmdExecute~ object.

*/

/*
1 Preliminaries

1.1 Defines

*/
#ifndef H_DSERVERCMDEXECUTE_H
#define H_DSERVERCMDEXECUTE_H

/*
1.2 Includes

*/
#include "DServerCmd.h"

/*
2 Class ~DServerCmdExecuteParam~

The class ~DServerCmdExecuteParam~ contains the parameters used in
the ~run~ - method of the class ~DServerCmdExecute~.

  * derives from class ~DServerParam~

Provided parameters:

  * string m[_]newName - new DArray data name 
  
  * string m[_]cmd - the command, which is executed

  * vector[<]string[>] m[_]sourceObjects - DArrayObject names, which serve as
input to the command function

*/
class DServerCmdExecuteParam 
  : public DServerParam
{
/*
2.1 Private Default Constructor

  * may not be used!

*/
  DServerCmdExecuteParam() {}
/*
2.2 Constructor

  * const string[&] inNewName - new name for the darray index data

  * const string[&] inCmd - the command to be executed

  * const vector[<]string[>][&] inSourceObject - DArray names, which are used
as input in the command

*/
public:
  DServerCmdExecuteParam(const string& inNewName,
                         const string& inCmd,
                         const vector<string>& inSourceObjects)
    : DServerParam()  
    , m_newName(inNewName)
    , m_cmd(inCmd)
    , m_sourceObjects(inSourceObjects)
  {}
/*
2.3 Execute - Constructor

*/
  DServerCmdExecuteParam(const DServerCmdExecuteParam & inP)
    : DServerParam(inP)
    , m_newName(inP.m_newName)
    , m_cmd(inP.m_cmd)
    , m_sourceObjects(inP.m_sourceObjects) {}
/*
2.4 Destructor

*/
  virtual ~DServerCmdExecuteParam() {}
/*
2.5 Getter Methods

2.5.1 Method ~const string[&] getNewName const~

  * returns const string[&] - the new darray data name

*/
  const string& getNewName() const { return m_newName; }

/*
2.5.2 Method ~const string[&] getCommand const~

  * returns const string[&] - the command

*/
  const string& getCommand() const { return m_cmd; }

/*
2.5.3 Method ~const vector[<]string[>][&] getSourceObjects const~

  * returns const vector[<]string[>][&] - the array of source objects

*/
  const vector<string>& getSourceObjects() const 
  { return m_sourceObjects; }
/*
2.5.4 Method ~const string[&] getSourceObject const~

  * int i - an index of the source object array

  * returns const string[&] - the source object at the index position

*/
  const string& getSourceObject(unsigned long i) const 
  { return m_sourceObjects[i]; }

/*
2.5.5 Method ~unsigned long getSourceObjectSize const~

  * returns unsigned long - the size of the array of source objects

*/
  unsigned long getSourceObjectSize() const 
  { return m_sourceObjects.size(); }


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
  string m_cmd;
  vector<string> m_sourceObjects;
/*
2.7 End of Class

*/
};

/* 
3 Class ~DServerCmdExecute~

The class ~DServerCmdExecute~ provides the functionality of copying data of a
darray index to an index of another darray. It is also used for the ~put~
operator.This operator also performs a copy of the darray data, skipping 
data of one index and replaceing it with new data

  * derives from the class ~DServerCmd~

*/
class DServerCmdExecute 
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

  DServerCmdExecute(DServer *inWorker, int inIndex)
    : DServerCmd(DServerCmd::DS_CMD_COPY, inWorker, inIndex)
  {}

/*
3.2 Destructor

*/

  virtual ~DServerCmdExecute() {}


/*
3.3 Getter Methods

3.3.1 Method ~const string[&] getNewName const~

  * returns const string[&] - the new darray name

*/
  
  const string& getNewName() const 
  { 
    const DServerCmdExecuteParam *p = 
      DServerCmd::getParam<DServerCmdExecuteParam>() ;
    return p -> getNewName();
  }

/*
3.3.2 Method ~const string[&] getCmd const~

  * returns const string[&] - the command

*/
const string& getCmd() const 
  { 
    const DServerCmdExecuteParam *p = 
      DServerCmd::getParam<DServerCmdExecuteParam>() ;
    return p -> getCommand(); 
  }

/*
3.3.3 Method ~const vector[<]string[>][&] getSourceObjects const~

  * returns const vector[<]string[>][&] - the array of source objects

*/
const vector<string>& getSourceObjects() const 
  { 
    const DServerCmdExecuteParam *p = 
      DServerCmd::getParam<DServerCmdExecuteParam>() ;
    return p -> getSourceObjects(); 
  }
/*
3.3.4 Method ~const string[&] getSourceObject const~

  * int i - an index of the source object array

  * returns const string[&] - the source object at the index position

*/
const string& getSourceObject(unsigned long i) const 
  { 
    const DServerCmdExecuteParam *p = 
      DServerCmd::getParam<DServerCmdExecuteParam>() ;
    return p -> getSourceObject(i); 
  }

/*
3.3.5 Method ~unsigned long getSourceObjectSize const~

  * returns unsigned long - the size of the array of source objects

*/

unsigned long getSourceObjectSize() const 
  { 
    const DServerCmdExecuteParam *p = 
      DServerCmd::getParam<DServerCmdExecuteParam>() ;
    return p -> getSourceObjectSize(); 
  }

/*
3.3.3 Method ~string getInfo const~

  * returns string - an informaiton string

*/
  string getInfo() const 
  {
    return string("CMD-Execute from:"  + getWorker() -> getName() + 
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

#endif // H_DSERVERCMDEXECUTE_H
