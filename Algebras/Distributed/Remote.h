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
[1] DistributedAlgebra
April 2012 Thomas Achmann

moved classes DServer and DServerManager into their own files


November 2010 Tobias Timmerscheidt

Header-File for Remote.cpp
Contains definitions of DServer, DServerManager, DServerExecutor
RelationWriter and DServerCreator

*/



#ifndef H_REMOTE_H
#define H_REMOTE_H
 
#include "StandardTypes.h"
#include "zthread/Runnable.h"
#include "zthread/Condition.h"
#include "zthread/Mutex.h"
#include "RelationAlgebra.h"
#include "TupleFifoQueue.h"

class DServer;
class ThreadedMemoryCounter;

class DServerExecutor : public ZThread::Runnable
{
  DServer* server;
public:
  DServerExecutor(DServer* s) {server=s;}
     
  void run();
};

class DServerMultiCommand : public ZThread::Runnable
{
private:
  TupleFifoQueue m_tfq;
   int m_index;
  DServer* m_server;
  ZThread::FastMutex lock;
  ZThread::Condition cond;
  bool m_runit;
  ThreadedMemoryCounter* m_memCntr;

public:
  DServerMultiCommand(int i, DServer* s, ThreadedMemoryCounter* inMemCntr) :
    m_index(i),
    m_server(s),
    cond(lock),
    m_runit(true),
    m_memCntr(inMemCntr)
  { 
  }

  void AppendTuple(Tuple* t)
  {
    m_tfq.put(t);
  }

  void done()
  {
    m_runit = false;
    m_tfq.put(NULL); // dummy to wake up waiting threads
  }

  void run();
};


class DServerCreator : public ZThread::Runnable
{
public:
  DServerCreator(const string& h, int p, const string& n, ListExpr t)
    : m_host(h)
    , m_port(p)
    , m_name(n)
    , m_type(t) { assert(!(nl -> ToString(t).empty())); }
   
  DServer* createServer();
  void run();
   
private:
  DServer* m_server;
  string   m_host;
  int      m_port;
  string   m_name;
  ListExpr m_type;
};

class DServerMultiplyer : public ZThread::Runnable
{
  DServer* server;
  int count;
   
public:
  DServerMultiplyer(DServer* s, int c)
  {
    server=s; count = c;
  }
   
  void run();
};
   
   

class RelationWriter : public ZThread::Runnable
{
   DServer* worker;
  vector<Word>* m_elements;
   vector<int> arg;
   
   public:
  RelationWriter(DServer* s, vector<Word>* e, const vector<int>& a) 
    : worker(s)
    , m_elements(e)
    , arg(a) {}
     
      void run();

  
};

#endif
