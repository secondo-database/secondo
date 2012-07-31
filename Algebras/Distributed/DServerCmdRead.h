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
[1] Class DServerCmdRead Definition

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmdRead~ reads the data of an DArray of atomic values
and stores it on the master for further processing (e.g. display function).

The class ~DServerCmdReadParam~ is a data class for the parameters used 
during the execution of a ~DServerCmdRead~ object.

*/

/*
1 Preliminaries

1.1 Defines

*/
#ifndef H_DSERVERCMDREAD_H
#define H_DSERVERCMDREAD_H

/*
1.2 Includes

*/
#include "DServerCmd.h"

/*
2 Class ~DServerCmdReadParam~

The class ~DServerCmdReadParam~ contains the parameters used in
the ~run~ - method of the class ~DServerCmdRead~.

  * derives from class ~DServerParam~

Provided parameters:

  * vector[<]Word[>][ast] m[_]outElements - pointer to the global 
storage container for the elements of this darray

  * vector[<]bool[>][ast] m[_]outIsPresent - pointer of the global
container, indicating if a element is present at the master

*/
class DServerCmdReadParam 
  : public DServerParam
{
/*
2.1 Private Default Constructor

  * may not be used!

*/
  DServerCmdReadParam() {}
/*
2.2 Constructor
used, if multiple indexes are transferred

  * vector[<]Word[>][ast] outElements - pointer to the global 
storage container for the elements of this darray

  * vector[<]bool[>][ast] outIsPresent - pointer of the global
container, indicating if a element is present at the master


*/
public:
  DServerCmdReadParam(vector<Word>* outElements,
                      vector<bool>* outIsPresent,
                      ListExpr inDaType);
  
  
/*
2.4 Copy - Constructor

*/
  DServerCmdReadParam(const DServerCmdReadParam & inP)
    : DServerParam(inP)
    , m_outElements(inP.m_outElements)
    , m_outIsPresent(inP.m_outIsPresent)
    , m_algID (inP.m_algID)
    , m_typeID (inP.m_typeID)
    , m_ttype (inP.m_ttype) {}

/*
2.5 Destructor

*/
  virtual ~DServerCmdReadParam() {}

/*
2.6 Getter Methods

2.6.1 Method ~vector[<]Word[>][ast] getOutElements const~

  * returns vector[<]Word[>][ast] - pointer to the global storage array

*/
  vector<Word>* getOutElements() const { return m_outElements; }
/*
2.6.2 Method ~vector[<]bool[>][ast] getOutIsPresent const~

  * returns vector[<]bool[>][ast] - pointer to the global storage array,
indicating if the data is present on the master

*/
  vector<bool>* getOutIsPresent() const { return m_outIsPresent; }

  int getAlgId() const { return m_algID; }

  int getTypId() const { return m_typeID; }

  ListExpr getTType() const { return m_ttype; }

/*
2.5.7 Method ~bool useChilds() const~

  * this function does not use childs

*/
  bool useChilds() const { return false; }

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
  vector<Word>* m_outElements;
  vector<bool>* m_outIsPresent;
  int m_algID;
  int m_typeID;
  ListExpr m_ttype;

/*
2.7 End of Class

*/
};

/* 
3 Class ~DServerCmdRead~

The class ~DServerCmdRead~ provides the functionality of reading
data of atomic darray element(s) from the workers

  * derives from the class ~DServerCmd~

*/
class DServerCmdRead 
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

  DServerCmdRead()
    : DServerCmd(DServerCmd::DS_CMD_READ)
  {}

/*
3.2 Destructor

*/

  virtual ~DServerCmdRead() {}


/*
3.3 Getter Methods

3.3.1 Method ~vector[<]Word[>][ast] getOutElements const~

  * returns vector[<]Word[>][ast] - pointer to the global storage array

*/
  
  vector<Word>* getOutElements() const 
  {
    const DServerCmdReadParam *p = 
      DServerCmd::getParam<DServerCmdReadParam>() ;
    return p -> getOutElements();
  }

/*
3.3..2 Method ~vector[<]bool[>][ast] getOutIsPresent const~

  * returns vector[<]bool[>][ast] - pointer to the global storage array,
indicating if the data is present on the master

*/
  vector<bool>* getOutIsPresent() const 
  {
    const DServerCmdReadParam *p = 
      DServerCmd::getParam<DServerCmdReadParam>() ;
    return p -> getOutIsPresent();
  }

  int getAlgId() const {  const DServerCmdReadParam *p = 
      DServerCmd::getParam<DServerCmdReadParam>() ;
    return p -> getAlgId(); }

  int getTypId() const {  const DServerCmdReadParam *p = 
      DServerCmd::getParam<DServerCmdReadParam>() ;
    return p -> getTypId(); }

  ListExpr getTType() const {  const DServerCmdReadParam *p = 
      DServerCmd::getParam<DServerCmdReadParam>() ;
    return p -> getTType(); }
/*
3.3.5 Method ~string getInfo const~

  * returns string - an informaiton string

*/
  string getInfo() const 
  {
    return string("CMD-Read from:"  + getWorker() -> getName() + 
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

#endif // H_DSERVERCMDREAD_H
