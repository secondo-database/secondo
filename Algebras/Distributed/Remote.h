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
 
#include <deque>
#include "StandardTypes.h"
#include "SocketIO.h" 
#include "zthread/Runnable.h"
#include "zthread/Thread.h"
#include "zthread/Guard.h"
#include "zthread/Condition.h"
#include "zthread/Mutex.h"
#include "RelationAlgebra.h"
#include "TupleFifoQueue.h"
#include "DBAccessGuard.h"
#include "ThreadedMemoryCntr.h"
//#include "StopWatch.h"

#define SINGLE_THREAD 1

using namespace std;


class DServer
{
  DServer() : m_cmd(NULL) {}
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
                                   // the worker and puts it into a 
                                   // relation on the server
                 DS_CMD_READ_TB_REL,    // reads a tuple from a relation on
                                        // the worker and puts it into a 
                                        // tuplebuffer on the server
                 
  };

  static Word ms_emptyWord;

  static void Debug(const string&, const string&);

  DServer(string,int,string,ListExpr);
  virtual ~DServer();

  void Terminate();
  bool connectToWorker();

  void setCmd(CmdType inCmdType,
              const list<int>* inIndex, 
              vector<Word>* inElements = 0,
              vector<string>* inFromNames = 0);

  void run();
                          
  bool Multiply(int count);
  const vector<DServer*> & getChilds() const { return m_childs; }
  void DestroyChilds();
            
  int getNumChilds() const { return m_numChilds;}
      
  bool checkServer(bool writeError) const;
  string getErrorText() { return m_errorText; }
  void int2Str(int i, string& ret) const
  {
    std::stringstream out;
    out << i;
    ret = out.str();
  }

  const string& getServerHostName() const { return m_host; }

  string getServerPortStr() const 
  {
    string ret;
    int2Str(m_port, ret);
    return ret;
  }
  int getServerPort() const { return m_port; }


  void setErrorText(const string& inErrText)
  { 
    m_error = true;
    m_errorText = inErrText; 
  }
public:
  class RemoteCommand
  {
  public:
    enum RunType { RC_NONE, RC_PARALEL, RC_SEQUENTIELL };
   
    RemoteCommand(CmdType inCmdType,
                  const list<int>* inDarrayIndex,
                  vector<Word>* inElements,
                  vector<string>* inFromNames)
      : m_cmdType( inCmdType )
      , m_elements( inElements )
      , m_runType( RC_NONE) 
    {
      if (inDarrayIndex != 0)
        m_darrayIndex = *inDarrayIndex;

      if (inFromNames != 0)
        m_fromNames = *inFromNames;
    }

    virtual ~RemoteCommand()
    {
    }
    CmdType getCmdType() const { return m_cmdType; }
    list<int>* getDArrayIndex() { return &m_darrayIndex; }
    vector<Word>* getElements() const { return m_elements; }
    const vector<string>& getFromNames() const { return m_fromNames; }

  private:
    RemoteCommand(const RemoteCommand&) {} 
    RemoteCommand() 
      : m_cmdType( DS_CMD_NONE )
      , m_elements( NULL )
      , m_runType( RC_NONE ) {}

    RunType getRunType() const { return m_runType; }

    // members
    CmdType m_cmdType;
    list<int> m_darrayIndex;
    vector<Word>* m_elements;
    vector<string> m_fromNames;
    RunType m_runType;
    
  };
public:
  

  void setCmd(RemoteCommand* rc) { m_cmd = rc; }

  friend ostream& operator << (ostream&, RemoteCommand&) ;
  void print() const;

  bool isRelOpen() const { return rel_open; }
  void setRelOpen() { rel_open = true; }
  void setRelClose() { rel_open = false; }
  bool isShuffleOpen() const { return m_shuffle_open; }
  void setShuffleOpen() { m_shuffle_open = true; }
  void setShuffleClose() { m_shuffle_open = false; }



  const string& getMasterHostIP() const;
  const string& getMasterHostIP_() const;
  const string& getName() const { return name; }

  Socket *getServer() { return m_server; }
  const Socket *getServer() const { return m_server; }

  ListExpr getTType() const { return m_type; }
  const string& getTTypeStr() const { return m_typeStr; }
private:

  string m_host;
  string name;

  RemoteCommand* m_cmd;

  int m_port;
  ListExpr m_type;
  string m_typeStr;

  Socket* m_server;
         
  Socket* cbworker;

  vector<DServer*> m_childs;
  int m_numChilds;
                  
  bool rel_open;
  bool m_shuffle_open;
   
  string m_errorText;
  bool m_error;
};

class DServerManager
{
   public:
  DServerManager() : m_status(false) {}
      DServerManager(ListExpr serverlist_n, 
                     string name_n, 
                     ListExpr inType, int sizeofarray);

  bool isOk() const { return m_status; }
      ~DServerManager();
  
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

  list<int>& getIndexList(int id) { return m_idIndexMap[id]; }
        
/*

2.5 getNoOfServers

returns the number of DServer-Objects controlled by the DServerManager

*/
     
  int getNoOfServers() const { return size; }

/*
2.6 checkServers

returns false, if server were not created correctly

*/   
  bool checkServers(bool writeError) const;

  const string& getErrorText() const { return errorText; }
  void  setErrorText( const string& txt) { errorText = txt; }
     
        
private:
  //StopWatch m_watch;
  vector<DServer*> m_serverlist;
  
  int size;
  int array_size;
  string name;
  
  map<int, list<int> > m_idIndexMap;

  string errorText;
  bool m_status;
  
};

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
  MemCntr* m_memCntr;

public:
  DServerMultiCommand(int i, DServer* s, MemCntr* inMemCntr) :
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
   
  void run()
  {
    if (!server->Multiply(count))
      {
      cerr << "Error multiplying Servers:" 
           << server -> getErrorText() << endl;
      }
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
     
                  
                  
ostream& operator << (ostream &out, DServer::RemoteCommand& rc);

#endif
