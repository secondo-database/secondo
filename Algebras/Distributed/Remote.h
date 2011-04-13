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

November 2010 Tobias Timmerscheidt

Header-File for Remote.cpp
Contains definitions of DServer, DServerManager, DServerExecutor
RelationWriter and DServerCreator

*/



#ifndef H_REMOTE_H
#define H_REMOTE_H


#include "StandardTypes.h"
#include "SocketIO.h"
#include "zthread/Runnable.h"
#include "zthread/Thread.h"

using namespace std;



class DServer
{
public:
  DServer(string,int,string,ListExpr);
  void Terminate();
  bool connectToWorker();

  void setCmd(const string&,
              list<int>*, vector<Word>* = 0);

  void run();
                          
  bool Multiply(int count);
  const vector<DServer*> & getChilds() const { return m_childs; }
  void DestroyChilds();
            
  int status;
  int getNumChilds() { return m_childs.size();}
      
  string getErrorText() { return errorText; }
         
private:
  string host,name,cmd;
  int port;
  ListExpr type;
  list<int>* arg;
         
  vector<Word>* m_elements;

  Socket* server;
         
  Socket* cbworker;

  vector<DServer*> m_childs;
                  
  bool rel_open;
   
  string errorText;
};

class DServerManager
{
   public:
      DServerManager(ListExpr serverlist_n, 
                              string name_n, ListExpr type, int sizeofarray);
      ~DServerManager();
      DServer* getServerByIndex(int index) const;
      DServer* getServerbyID(int id) const;
     
      int getMultipleServerIndex(int index);
                

      list<int>* getIndexList(int id);
        
      int getNoOfServers();
   
      const string& getErrorText() const { return errorText; }
  void  setErrorText( const string& txt) { errorText = txt; }
     
        
   private:
                
  vector<DServer*> m_serverlist;

      int size;
      int array_size;
      string name;
   
      string errorText;
};

class DServerExecutor : public ZThread::Runnable
{
   DServer* server;
   public:
   DServerExecutor(DServer* s) {server=s;}
     
   void run();
};

class DServerCreator : public ZThread::Runnable
{
public:
  DServerCreator(string h, int p, string n, ListExpr t);
   
  DServer* createServer();
  void run();
   
private:
  DServer* m_server;
  string host,name;
  int port;
  ListExpr type;
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
   
  void run()
  {
    server->Multiply(count);
  }
};
   
   

class RelationWriter : public ZThread::Runnable
{
   DServer* worker;
  vector<Word>* m_elements;
   list<int>* arg;
   
   public:
  RelationWriter(DServer* s, vector<Word>* e, list<int>* a) 
    : worker(s)
    , m_elements(e)
    , arg(a) {}
     
      void run();

  
};
     
                  
                  
                  


#endif
