/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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
*/


/*
[1]  Header File of the DistributedAlgebra

November 2010 Tobias Timmerscheidt
April    2011 Thomas Achmann

This algebra implements a distributed array. This type of array
keeps its element on remote servers, called worker. Upon creation
of the array all elements are transfered to the respective workers.
The list of workers must be specified in terms of a relation in any
operator that gives back a darray.
Operations on the darray-elements are carried out on the remote machines.


1. Preliminaries

1.1 Includes

*/
#ifndef _DISTRIBUTEDALGEBRA_H_
#define _DISTRIBUTEDALGEBRA_H_

#include <iostream>
#include <fstream>
#include <vector>
#include "TupleFifoQueue.h"
//#include "StopWatch.h"


/*
3. Type Constructor ~DArray~

3.1 Data Structure - Class ~DArray~

*/


class DArray
{
public:
  //StopWatch m_watch;
  DArray(bool isDefined = true);
  DArray(ListExpr, const string& inName,int,ListExpr);
  virtual ~DArray();

  bool initialize(ListExpr, const string& inName,
                  int,ListExpr,const vector<Word>& );
  bool initialize(ListExpr, const string& inName,
                  int,ListExpr);

  //Returns the content of m_elements[int]
  const Word& get(int);
  //Sets m_elements[int] and sends the object to the respective worker
  void set(Word,int);

  int getAlgID() const { return alg_id; }
  int getTypID() const { return typ_id; }
  ListExpr getType() const { return m_type; }

  ListExpr getServerList() const { return m_serverlist; }

  const string& getName() const { return m_name; }

  bool IsDefined() const { return m_defined; }
  void SetDefined(bool def) { m_defined = def; }
  void SetUndefined() { m_defined = false; }

  int getSize() const { return size; }

  DServerManager* getServerManager() const {return m_serverManager;}

  const string& getHostNameByIndex(int inIdx);

  //Is needed to provide DServer-objects with a pointer to the elements-array
  const vector<Word>& getElements() const {return m_elements;}


  //Retrieves the element int/all elements from the worker
  //refresh must be called before calling get()
  void refresh(int);
  void refresh();
  void refresh(TFQ tbOut);
  
  bool refreshTBRunning() 
  { ZThread::Guard<ZThread::Mutex> g(ms_rTBlock); return m_tbRunning; };
  void initTBRefresh() 
  {  ZThread::Guard<ZThread::Mutex> g(ms_rTBlock); m_tbRunning = true; }
  void tbRefreshDone() 
  {  ZThread::Guard<ZThread::Mutex> g(ms_rTBlock); m_tbRunning = false; }
  
  //Deletes all the remote elements on the workers
  void remove();

  //Persistens Storage functions for the type constructor
  static Word In( const ListExpr inTypeInfo , const ListExpr instance ,
                  const int errorPos , ListExpr& errorInfo ,
                  bool& correct );
  static ListExpr Out( ListExpr inTypeInfo , Word value );
  static Word Create( const ListExpr inTypeInfo );
  static void Delete( const ListExpr inTypeInfo , Word& w );
  static void Close( const ListExpr inTypeInfo, Word& w );
  static Word Clone( const ListExpr inTypeInfo , const Word& w );
  static bool KindCheck( ListExpr inType , ListExpr& errorInfo );
  static int SizeOfObj();
  static bool Open( SmiRecord& valueRecord ,
                    size_t& offset , const ListExpr inTypeInfo,
                    Word& value );
  static bool Save( SmiRecord& valueRecord , size_t& offset ,
                    const ListExpr inTypeInfo , Word& value );


  //Static no of existing DArray-Instances, used for naming
  static int no;

  bool isRelType() {return isRelation;}

  static const string BasicType() { return "darray"; }

  static const bool checkType(ListExpr inType){
    ListExpr errorInfo = listutils::emptyErrorInfo();
    return KindCheck(inType, errorInfo);
  }

  bool hasError() const;
  string getErrorText() const;

  bool destroyAnyChilds();
  bool multiplyWorkers(vector<DServer*>* outServerList = NULL);

  template <class T, class P>
  bool runCommand(const P& inParam);
   

  template <class T, class P>
  bool runCommandThreaded(ZThread::ThreadedExecutor& inExec,
                          const P& inParam,
                          vector<DServer *>* inServerList = NULL,
                          bool inWaitForThreadToEnd = true);
private:

  //Sends the relation in elements[index] to the respective worker
  void WriteRelation(int index);

  //Is the DArray defined (posseses a name, size, serverlist, type?!)
  bool m_defined;

  //Is a certain element present on the master?
  // std::vector<bool> is broken!
  // using 1: present; 0: not present
  vector<int> m_present;
  bool isRelation;
  int size;
  int alg_id;
  int typ_id;
  string m_name;
  ListExpr m_type;

  ListExpr m_serverlist;

  DServerManager* m_serverManager;

  vector<Word> m_elements;

  bool m_tbRunning;
  static ZThread::Mutex ms_rTBlock;
};

#endif // _DISTRIBUTEDALGEBRA_H_
