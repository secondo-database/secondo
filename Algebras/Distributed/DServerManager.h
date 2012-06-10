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
[1] Class DServerManager Definition

\begin{center}
March 2012 Thomas Achmann

November 2010 Tobias Timmerscheidt
\end{center}

[TOC]

0 Description

0.1 March 2012

Class ~DServerManager~ exists now in its own .h and .cpp file. Added methods
to calculate the number of child workers depending on the size of the
darray and the number of available workers.

0.2 November 2010

Header-File for Remote.cpp
Contains definitions of DServer, DServerManager, DServerExecutor
RelationWriter and DServerCreator

1 Preliminaries

1.1 Defines

*/
#ifndef H_DSERVERMANAGER_H
#define H_DSERVERMANAGER_H

/*
1.2 Includes

*/
#include "StandardTypes.h"

class DServer; 

class DServerManager
{
   public:
  DServerManager() : m_error(false), m_status(false) {}
      DServerManager(ListExpr serverlist_n, 
                     string name_n, 
                     ListExpr inType, int sizeofarray);

  virtual ~DServerManager();

  bool isOk() const { return m_status; }
  
/*

2.1 getServerByIndex

returns a pointer to the DServer that holds a certain element of the
underlying distributed array

*/

  DServer* getServerByIndex(int index) const 
    { return m_serverlist[index % size]; }
  
/*

2.2 getServerbyID

returns the pointer to a DServer

*/

  DServer* getServerbyID(int id) const { return m_serverlist[id]; }
     
/*

2.3 getMultipleServerIndex

returns -1, if the parent DServer is the appropriate object for the element

*/

  int getMultipleServerIndex(int index) const {  return (index / size) - 1; }
                
/*

2.4 getIndexList

returns a list of indices that correspond to the elements of the underlying
distributed array, which are controlled by the DServer with the given index

*/

  vector<int>& getIndexList(int id) { return m_idIndexMap[id]; }
        
/*

2.5 getNoOfWorkers

returns the number of DServer-Objects controlled by the DServerManager

*/
  int getNoOfWorkers() const 
  { 
    return size; 
  }

/*

2.5 getNoOfMultiWorkers

returns the number of DServer-Objects controlled by the DServerManager
it more than one worker is required per server object

*/

  int getNoOfMultiWorkers(int inArraySize) const 
  { 
    if (size > inArraySize)
      return inArraySize;

    return size; 
  }

  int getRelativeNrOfChildsPerWorker (int inArraySize) const
  {
    if (size > inArraySize)
      return 1;

    return int(inArraySize / size);
  }
  
  int getNrOfWorkersWithMoreChilds (int inArraySize) const
  {
    if (size > inArraySize)
      return 0;

    return inArraySize - (((int)inArraySize / size) * size);
  }

/*
2.6 checkServers

returns false, if server were not created correctly

*/   
  bool checkServers(bool writeError) const;

  const string& getErrorText() const { return m_errorText; }

  bool  hasError() const { return m_error; }
  void  setErrorText( const string& txt) 
  { 
    m_error = true;
    m_errorText = txt; 
  }
     
        
private:
  //StopWatch m_watch;
  vector<DServer*> m_serverlist;
  
  int size;
  int array_size;
  string name;
  
  map<int, vector<int> > m_idIndexMap;

  string m_errorText;
  bool m_error;
  bool m_status;
  
};

#endif // H_DSERVERMANAGER_H
