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

This file contains the implementation of DServer, DServerManager,
DServerCreator, DServerExecutor and RelationWriter

*/

#include "Remote.h"
#include "DServer.h"
#include "DBAccessGuard.h"
#include "SocketIO.h"
#include "Processes.h"
#include "zthread/Runnable.h"
#include "zthread/Thread.h"
#ifndef SINGLE_THREAD1
#include "zthread/ThreadedExecutor.h"
#endif
#include "zthread/Mutex.h"
#include "StringUtils.h"
#include <iostream>
#include "ThreadedMemoryCntr.h"

using namespace std; 


/*

3 Class RelationWriter

3.1 run

copies a relation to a remote system
can run as a thread of its own

*/
        
void RelationWriter::run()
{
  int index;
     
  while(!arg.empty())
    { 

      index = arg.back();
      arg.pop_back();
        
      //create relation iterator

      GenericRelation* rel = (Relation*)(*m_elements)[index].addr;
      GenericRelationIterator* iter = rel->MakeScan();
      
      Tuple* t;
     
      string attrIndex("EMPTY"), rec_type("EMPTY");
      //open tuple stream to worker
      vector<int> l;
      l.push_back(index);
      vector<Word> open_words(2);
      open_words[0].addr = &attrIndex;
      open_words[1].addr = &rec_type;
     
      //open tuple stream to the worker
      worker->setCmd(DServer::DS_CMD_OPEN_WRITE_REL,&l,&open_words);
      worker->run();
     
      vector<Word> word(1);

      t = iter->GetNextTuple();

      unsigned long cnt = 0;
      //send tuples
      while(t != 0)
        {
          word[0].addr = t;
          t->IncReference();
          worker->setCmd(DServer::DS_CMD_WRITE_REL,0,&word);
          
          worker->run();
          t->DeleteIfAllowed();
          t = iter->GetNextTuple();
     
        }
     
      //close tuple stream
      worker->setCmd(DServer::DS_CMD_CLOSE_WRITE_REL,0);
      worker->run();
     
     
      delete iter;
    }
     
}

/*

5 Class DServerExecutor

*/
                        
void DServerExecutor::run()
{ 
  server->run(); 
}
/*

5 Class DServerMultiCommand

*/
void  DServerMultiCommand::run()
{
  //cout << "Starting DMC:" << m_index << " " << m_runit << endl;
  Tuple *t;
  vector<Word> w(1);
  while(m_runit || !m_tfq.empty())
    {
      //cout << m_index << ": got " << tb -> GetNoAttributes()  << endl;

      //cout << "Sending:" << m_index << endl;
      t = m_tfq.get();
      if (t != NULL)
        {
          w[0] = SetWord(t);
          m_server->setCmd(DServer::DS_CMD_WRITE_REL, 
                       0, &w, 0);
          m_server -> run();
          m_memCntr -> put_back(t -> GetSize());
          DBAccess::getInstance() -> T_DeleteIfAllowed(t);
        }
    }
  //cout << "DMC:" << m_index << " ... done" << endl;
}

/*

7 Class DServerCreator

*/

DServer*
DServerCreator::createServer()
{
  m_server = new DServer(m_host, m_port,m_name,m_type);
  //cout << "Server: "<< port << "@" << host << " created" << endl;
  return m_server;
}


void DServerCreator::run()
{ 
  //cout << "Connecting Server: "<< m_host << ":" << m_port << endl;
  m_server -> connectToWorker();
}

void 
DServerMultiplyer::run()
{
  if (!server->Multiply(count))
    {
      cerr << "Error multiplying Servers:" 
           << server -> getErrorText() << endl;
    }
}
