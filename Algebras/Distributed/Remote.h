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

March 2010 Tobias Timmerscheidt

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

      void setCmd(string,list<int>*,Word*);
      void run();
                  
                          
      bool Multiply(int count);
      DServer** getChilds() { return childs; }
      void DestroyChilds();
            
      int status;
      int getNumChilds() { return num_childs;}
      
      string geterrorText() { return errorText; }
         
   private:
      string host,name,cmd;
      int port;
      ListExpr type;
      list<int>* arg;
         
      Word* elements;

      Socket* server;
         
      Socket* cbworker;
      
      int num_childs;
      DServer** childs;
                  
      bool rel_open;
   
      string errorText;
};

class DServerManager
{
   public:
      DServerManager(ListExpr serverlist_n, 
                              string name_n, ListExpr type, int sizeofarray);
      ~DServerManager();
      DServer* getServerByIndex(int index);
      DServer* getServerbyID(int id);
     
      int getMultipleServerIndex(int index);
                

      list<int>* getIndexList(int id);
        
      int getNoOfServers();
   
      string geterrorText() { return errorText; }
     
        
   private:
                
      DServer** serverlist;

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
      DServerCreator(DServer** s, string h, int p, string n, ListExpr t);
   
      void run();
   
   private:
      DServer** server;
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
   Word* elements;
   list<int>* arg;
   
   public:
      RelationWriter(DServer* s, Word* e, list<int>* a) 
         {worker=s; elements = e; arg = a;}
     
      void run();
};
     
                  
                  
                  


#endif
