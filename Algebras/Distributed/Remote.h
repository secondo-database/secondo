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

static int DServerCnt;

class DServer
{
public:

  enum CmdType { DS_CMD_NONE = 0,  // undefined
                 DS_CMD_WRITE,     // writes an element to the worker
                 DS_CMD_READ,      // reads an element from the worker
                 DS_CMD_DELETE,    // deletes an element on the worker
                 DS_CMD_COPY,      // copies an element on the worker
                 DS_CMD_EXEC,      // exectues a command on each element on
                                   // the worker
                 DS_CMD_OPEN_WRITE_REL, // opens a relation on the worker to
                                        // add elements
                 DS_CMD_WRITE_REL, // writes a singel tuple to a relation 
                                   // on the worker
                 DS_CMD_CLOSE_WRITE_REL, // closes a relation on the worker
                 DS_CMD_READ_REL,  // reads a tuple from a relation on
                                   // the worker
                 
  };
  DServer(string,int,string,ListExpr);
  void Terminate();
  bool connectToWorker();

  void setCmd(CmdType inCmdType,
              list<int>* inArgs, 
              vector<Word>* inElements = 0);

  void run();
                          
  bool Multiply(int count);
  const vector<DServer*> & getChilds() const { return m_childs; }
  void DestroyChilds();
            
  int status;
  int getNumChilds() { return m_childs.size();}
      
  string getErrorText() { return errorText; }
         
private:
  class RemoteCommand
  {
  public:
    explicit RemoteCommand(CmdType inCmdType,
                           list<int>* inArgs, 
                           vector<Word>* inElements)
      : m_cmdType( inCmdType )
      , m_args( inArgs )
      , m_elements( inElements ) {}

    CmdType       getCmdType() const { return m_cmdType; }
    list<int>*    getArgs() const { return m_args; }
    vector<Word>* getElements() const { return m_elements; }

  private:
    // methods
    RemoteCommand() {} // no to be used!
    RemoteCommand(const RemoteCommand&) {} // not to be used!

    // members
    CmdType m_cmdType;
    list<int>* m_args;
    vector<Word>* m_elements;
  };

  string host,name;

  RemoteCommand* m_cmd;

  int port;
  ListExpr type;

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
