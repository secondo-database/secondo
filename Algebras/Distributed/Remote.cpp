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
#include "SocketIO.h"
#include "RelationAlgebra.h"
#include "zthread/Runnable.h"
#include "zthread/Thread.h"
#include "zthread/ThreadedExecutor.h"
#include "zthread/Mutex.h"
#include <iostream>



using namespace std;


void extractIds(const ListExpr,int&,int&);
string toString_d(int);

//IP-Adress of this system (Master)
string HostIP;
string HostIP_;

//Synchronisation of access to Flobs
ZThread::Mutex Flob_Mutex;

/*

1 Class DServer

1.1 Constructor

Constructs a DServer
A TCP-connection to the corresponding worker ist opened

*/

DServer::DServer(string n_host,int n_port,string n_name,ListExpr n_type)
{
     
   host = n_host;
   port = n_port;
   name = n_name;
   type = n_type;
   
   errorText = "OK";
   
     
   rel_open = false;
     
   num_childs = 0;
        
   string line;
        
   server = Socket::Connect( host, toString_d(port), 
                                          Socket::SockGlobalDomain );
   
   if(server!=0 && server->IsOk())
   {
      iostream& iosock = server->GetSocketStream();

      getline( iosock, line );
                
      if(line=="<SecondoOk/>")
      {
         iosock << "<Connect>" << endl << endl 
                     << endl << "</Connect>" << endl;
         
         getline( iosock, line );
                        
         if( line == "<SecondoIntro>")
         {
            do
               getline( iosock, line);
            while(line != "</SecondoIntro>");
                                   
         }
         else 
            errorText = "Unexpected response from worker (No <SecondoIntro/>)";
      }
      else 
         errorText = "Unexpected response from worker (No <SecondoOk/>)";
   }
   else 
      errorText = "Connection to the worker couldn't be established!";
        
   iostream& iosock = server->GetSocketStream();
   
   iosock << "<Secondo>" << endl << "1" << endl 
            << "open database distributed" << endl 
            << "</Secondo>" << endl;
   
   getline( iosock, line );
        
   if(line=="<SecondoResponse>")
   {
      do
      {
         getline( iosock, line );
         /*if (line[line.size() - 1] == '\r')
            line.resize(line.size() - 1);*/
         if(line.find("error") != string::npos)
            errorText = "Opening of database \"distributed\" on worker failed!";
                        
      }
      while(line.find("</SecondoResponse>") == string::npos);
   }
   else 
      errorText = "Unexpected response from worker (No <SecondoResponse>)";
        

   HostIP = server->GetSocketAddress();
   HostIP_ = "h" + replaceAll(HostIP,".","_");
   
   cout << "Connection to Worker on " << host << " established." << endl;
}

/*

1.2 Implementation of Terminate

Breakes the connection to the remote system

*/

void DServer::Terminate()
{
   if(server != 0)
   {
      iostream& iosock = server->GetSocketStream();
      iosock << "<Disconnect/>" << endl;
      server->Close();
      delete server;
      server=0;
   }
}

/*

1.3 Implementation of setCmd

Sets new paramaters that are needed for the run-method

*/

void DServer::setCmd(string n_cmd, list<int>* n_arg,Word* n_array)
{
        
   cmd = n_cmd;
   arg = n_arg;
   elements = n_array;
   
}

/*

1.4 Implementation of run

Performs the specified operation on the remote system

*/

void DServer::run()
{
   
   int arg2;

   if(cmd=="write")
   {

      if(rel_open) return;
      
      int algID,typID;
      extractIds(type,algID,typID);
      
      string daten;
                
      TypeConstructor* t = am->GetTC(algID,typID);
      
      while(!arg->empty())
      {

         arg2 = arg->front();
         arg->pop_front();
        
         iostream& iosock = server->GetSocketStream();
         string port =toString_d((1800+arg2)); 

         //Element is deleted on the worker, if it exists already
         iosock << "<Secondo>" << endl << "1" << endl << "delete r" 
                  << name << toString_d(arg2)  << endl << "</Secondo>" 
                  << endl;
         
         string line;

         do
         {
            getline(iosock,line);
            if(line.find("error") != string::npos) 
               errorText = "Error while deleting element on the worker";
         }
         while(line.find("</SecondoResponse") == string::npos);  

         //The receiveD-Operator on the worker is called
         string com = "let r" + name + toString_d(arg2) + 
                        " = " + "receiveD(" + HostIP_ + ",p" + port + ")";
         
         iosock << "<Secondo>" << endl << "1" << endl 
                  << com << endl << "</Secondo>" << endl;
                
         //Callback-connection request from the worker is received
         Socket* gate = Socket::CreateGlobal( HostIP, port);

         Socket* worker = gate->Accept();

         iostream& cbsock1 = worker->GetSocketStream();
                
         //The element-type is sent to the type-mapping-fcts of receiveD
         cbsock1 << "<TYPE>" << endl << nl->ToString(type) 
                     << endl << "</TYPE>" << endl;
                
         getline(cbsock1,line);
         if(line!="<CLOSE>") 
            errorText = (string)"Unexpected response from" 
         + " worker (No <Close> after type transmission)!";
         
         //Connection is closed and new connection with value-mapping
         //on the worker is established
         worker->Close();delete worker;
                        
         worker = gate->Accept(); 
         gate->Close();delete gate;gate=0;
         
         iostream& cbsock = worker->GetSocketStream();
                
         cbsock << "<TYPE>" << endl << nl->ToString(type) 
                    << endl << "</TYPE>" << endl;

         //The element is converted into a binary stream of data
         SmiRecordFile recF(false,0);
         SmiRecord rec;
         SmiRecordId recID;
                
         recF.Open("send");
         recF.AppendRecord(recID,rec);
         size_t size = 0;
         
         am->SaveObj(algID,typID,rec,size,type,elements[arg2]);
                
         char* buffer = new char[size]; 
         rec.Read(buffer,size,0);
         //rec.Truncate(3);
         recF.DeleteRecord(recID);
         recF.Close();
                
         //Size of the binary data is sent
         cbsock << "<SIZE>" << endl << size << endl << "</SIZE>" << endl;
                
         //The actual data are sent
         worker->Write(buffer,size);
         
         delete buffer;
      
         Attribute* a;
         if(t->NumOfFLOBs() > 0 ) 
            a = static_cast<Attribute*>((am->Cast(algID,typID))
                  ((elements[arg2]).addr));
         
         //Flobs are sent to worker
         for(int i = 0; i < t->NumOfFLOBs(); i++)
         {
            Flob* f = a->GetFLOB(i);
         
            //Flob is converted to binary data
            SmiSize si = f->getSize();
            int n_blocks = si / 1024 + 1;
            char* buf = new char[n_blocks*1024];
            memset(buf,0,1024*n_blocks);
         
            f->read(buf,si,0);
         
            //Size of the Flob is sent
            cbsock << "<FLOB>" << endl << "<SIZE>" << endl 
                       << si << endl << "</SIZE>" << endl;
            
            //Flob data is sent
            for(int j = 0; j<n_blocks;j++)
               worker->Write(buf+j*1024,1024);
            
            cbsock << "</FLOB>" << endl;
            
            delete buf;
         }
      
         cbsock << "<CLOSE>" << endl;
         
                
         getline(cbsock,line);
         
         if(line!="<FINISH>")
            errorText = (string)"Unexpected response from worker" 
                              + " (No <FINISH> after value transmission";
                

         worker->Close();delete worker;worker=0;

         do
         {
            getline(iosock,line);
            if(line.find("error") != string::npos) 
               errorText = "Worker reports an error on storing an element!";
            
         }
         while(line.find("</SecondoResponse") == string::npos);
 

      }

                
   }
        
   if(cmd=="read")
   {
      if(rel_open) return;
      int algID,typID;
      extractIds(type,algID,typID);
                
      string daten;
                
      while(!arg->empty())
      {

         arg2 = arg->front();
         arg->pop_front();
         string port =toString_d((1500+arg2));
         
         //The sendD-operator on the worker is started       
         iostream& iosock = server->GetSocketStream();
         iosock << "<Secondo>" << endl << "1" << endl 
                  << "query sendD (" << HostIP_ << ",p" << port << ",r" 
                  << name << toString_d(arg2) << ")" <<  endl 
                  << "</Secondo>" << endl;
                
          
         //Callback-connection is received
         Socket* gate = Socket::CreateGlobal( HostIP, port);

         Socket* worker = gate->Accept();
         gate->Close();delete gate;gate=0;

         iostream& cbsock = worker->GetSocketStream();
         cbsock << "<TYPE>" << endl << nl->ToString(type)
                     << endl << "</TYPE>" << endl;
                             
         string line;
         getline(cbsock,line);
                
         if(line=="<SIZE>")
         {
            //Size of binary data is received
            getline(cbsock,line);
            int size = atoi(line.data());
            getline(cbsock,line);

             //The actual data is received...       
            char* buffer = new char[size];
            memset(buffer,0,size);
            cbsock.read(buffer,size);
            
            //... and converted back to a secondo-object
            SmiRecordFile recF(false,0);
            SmiRecord rec;
            SmiRecordId recID;
                     
            recF.Open("rec");
            recF.AppendRecord(recID,rec);
            rec.Write(buffer,size,0);
            
            size_t s = 0;
            am->OpenObj(algID,typID,rec,s,type,elements[arg2]);
               
            recF.DeleteRecord(recID);
            recF.Close();
            
            delete buffer;
            
            getline(cbsock,line);
            
            int flobs=0;
            
            //The threads must not write a flob on the same time
            Flob_Mutex.acquire();
           
            while(line=="<FLOB>")
            { 
               getline(cbsock,line);
               if(line!="<SIZE>")
                  errorText = (string)"Unexpected Response from" 
                                 + " worker (<SIZE> expected)!";
               
               //Size of the flob is received
               getline(cbsock,line);
               SmiSize si = atoi(line.data());
               getline(cbsock,line);
               
               if(line!="</SIZE>")
                  errorText = (string)"Unexpected Response from " 
                                 + "worker (</SIZE> expected)!";
            
               int n_blocks = si / 1024 + 1;
               
               //Data of the flob is received
               char* buf = new char[n_blocks*1024];
               memset(buf,0,1024*n_blocks);
               
               for(int i = 0; i< n_blocks; i++)
                  cbsock.read(buf+1024*i,1024);
            
            
               Attribute* a = static_cast<Attribute*>
                              ((am->Cast(algID,typID))(elements[arg2].addr));
            
               //Flob data is written
               Flob*  f = a->GetFLOB(flobs);
               f->write(buf,si,0);
            
            
               delete buf;
            
               getline(cbsock,line);
               if(line!="</FLOB>") 
                  errorText = (string)"Unexpected Response from " 
                                    + "worker (</SIZE> expected)!";
            
               getline(cbsock,line);
               flobs++;
            }
            
            Flob_Mutex.release();
         
            if(line!="<CLOSE>")
               errorText = (string)"Unexpected Response from " 
                                 + "worker (<CLOSE> expected)!";
                         
            iosock << "<FINISH>" << endl;
         
            worker->Close();delete worker;worker=0;

         }
         else
            errorText = "Unexpected response from worker (<SIZE> expected)!";
                
         do
         {
            getline(iosock,line);
            if(line.find("error") != string::npos) 
               errorText = "Worker reports error on sending an element!";
         }
         while(line.find("</SecondoResponse>") == string::npos);

      }

   }
        
   if(cmd=="delete")
   {
      if(rel_open) return;
      while(!arg->empty())
      {
         arg2 = arg->front();
         arg->pop_front();
         
         //Element is deleted on the worker
         string line;
         iostream& iosock = server->GetSocketStream();
         iosock << "<Secondo>" << endl << "1" << endl << "delete r" 
                  << name << toString_d(arg2) << endl << "</Secondo>" 
                  << endl;

         do
         {
            getline(iosock,line);
            if(line.find("error") != string::npos)
               errorText = "Worker reports error on deleteing element!";
         }
         while(line.find("</SecondoResponse>") == string::npos);

      }
   
   }
        
   if(cmd=="copy")
   {
      if(rel_open) return;
      string line;
      iostream& iosock = server->GetSocketStream();

      string to = ((string*)(elements[0].addr))->data();
               
      while(!arg->empty())
      {
         arg2 = arg->front();
         arg->pop_front();
         
         //Element is copied on the worker
         string cmd;
         cmd = "let r" + to + toString_d(arg2) + " = r" 
                        + name + toString_d(arg2);
         iosock << "<Secondo>" << endl << "1" << endl 
                     << cmd<< endl << "</Secondo>" << endl;

         do
         {
            getline(iosock,line);
            if(line.find("error") != string::npos)
               errorText = "Worker reports error on copying element!";
         }
         while(line.find("</SecondoResponse>") == string::npos);

      }
   
   }
        
   if(cmd=="execute")
   {
      if(rel_open) return;
      string line;
      iostream& iosock = server->GetSocketStream();
      string to = ((string*)(elements[0].addr))->data();
      string com = ((string*)(elements[1].addr))->data();

      while(!arg->empty())
      {
         arg2 = arg->front();
         arg->pop_front();
         
         string com_a = replaceAll(com,"!","r" + name + toString_d(arg2));
         
         //A command is executed on the worker
         string cmd;
         cmd = "let r" + to + toString_d(arg2) + " = " + com_a;
         
         iosock << "<Secondo>" << endl << "1" << endl 
                     << cmd<< endl << "</Secondo>" << endl;

         do
         {
            getline(iosock,line);
            if(line.find("error") != string::npos)
               errorText = "Worker reports error on executing operation!";
         }
         while(line.find("</SecondoResponse>") == string::npos);

      }
   }
     
   
   if(cmd=="open_write_rel")
   {
      if(rel_open) return;
      //Initializes the writing of a tuple-stream, 
      //the d_receive_rel operator is started on the remote worker
             
      string line;
      iostream& iosock = server->GetSocketStream();
              
      int arg2 = arg->front();
      
      string port =toString_d((1800+arg2)); 
      string com = "let r" + name + toString_d(arg2) + 
                     " = " + "d_receive_rel(" + HostIP_ + ",p" + port + ")";
                
      //The element is deleted on the worker, if already exists
      iosock << "<Secondo>" << endl << "1" << endl << "delete r" 
               << name << toString_d(arg2)  << endl << "</Secondo>" 
               << endl;
                
      do
         getline(iosock,line); 
      while(line.find("</SecondoResponse") == string::npos);
            
      //The d_receive_rel operator is invoked
      iosock << "<Secondo>" << endl << "1" << endl 
                  << com << endl << "</Secondo>" << endl;
                
      
      //The callback connection is opened          
      Socket* gate = Socket::CreateGlobal( HostIP, port);

      cbworker = gate->Accept();
       
      //Relation type is sent to type-mapping-fct of the d_receive_rel operator
      iostream& cbsock1 = cbworker->GetSocketStream();
      cbsock1 << "<TYPE>" << endl << nl->ToString(type) 
                  << endl << "</TYPE>" << endl;
                        
                                        
      getline(cbsock1,line);
      if(line!="<CLOSE>")
         errorText = "Unexpected Response from worker (<Close> expected)!";
      
      cbworker->Close();delete cbworker;
                        
      //The callback connection from the value-mapping is opened and stored
      cbworker = gate->Accept();
      iostream& cbsock2 = cbworker->GetSocketStream();
                
      cbsock2 << "<TYPE>" << endl << nl->ToString(type) 
                     << endl << "</TYPE>" << endl;
                
      gate->Close();delete gate;gate=0;
                        
      rel_open = true;
               
   }
          
   if(cmd == "write_rel")
   {
      //Writes a single tuple to an open tuple stream to the worker
      if(!rel_open) return;
               
      string line;
               
      Tuple *tpl = (Tuple*)elements[0].addr;
      
      //Get the tuple size
      size_t cS,eS,fS,size;
      size = tpl->GetBlockSize(cS,eS,fS);
               
               
      int num_blocks = (size / 1024) + 1;
               
      char* buffer = new char[num_blocks*1024];
      memset(buffer,0,1024*num_blocks);
               
      //Get the binary data of the tuple
      Flob_Mutex.acquire();
      tpl->WriteToBin(buffer,cS,eS,fS);
      Flob_Mutex.release();
               
      iostream& cbsock = cbworker->GetSocketStream();
               
      //Send the size of the tuple to the worker
      cbsock << "<TUPLE>" << endl << toString_d(size) 
                  << endl << "</TUPLE>" << endl;
      
      getline(cbsock,line); 
      if(line!= "<OK>") 
         errorText = "Worker unable to receive tuple!";
               
               
      getline(cbsock,line); 
      
      if(atoi(line.data()) != num_blocks) 
         errorText = "Worker calculated wrong number of blocks!";
      
      getline(cbsock,line);
               
      //Send the tuple data to the worker
      for(int i = 0; i<num_blocks;i++)
         cbsock.write(buffer+i*1024,1024);
       
      delete buffer;
               
      tpl->DeleteIfAllowed();
          
   }
          
   if(cmd == "close_write_rel")
   {
      //Closes an open tuple stream to the worker
      if(!rel_open) return;
      
      string line;

      iostream& cbsock = cbworker->GetSocketStream();
      iostream& iosock = server->GetSocketStream(); 
               
      //Sends the close signal and receive <FINISH>
      cbsock << "<CLOSE>" << endl;
      getline(cbsock,line);
      if(line != "<FINISH>")
         errorText = "Unexpected Response from worker! (<FINISH> expected)";
      
      cbworker->Close(); delete cbworker; cbworker = 0;

      do
      {
         getline(iosock,line);
         if(line.find("errror") != string::npos)
            errorText = "Worker reports error on closing relation!";
      }
      while(line.find("</SecondoResponse") == string::npos);

      rel_open = false;
   }
   

   if(cmd == "read_rel")
   {
      //Reads an entire relation from the worker
      int algID,typID;
      extractIds(type,algID,typID);
          
      while(!arg->empty())
      {
       
         arg2 = arg->front();
         arg->pop_front();
         
         string line;        
         string port =toString_d((1300+arg2));
                
         //start execution of d_send_rel
         iostream& iosock = server->GetSocketStream();
         iosock << "<Secondo>" << endl << "1" << endl 
                  << "query d_send_rel (" << HostIP_ << ",p" << port << ",r"
                  << name << toString_d(arg2) << ")" <<  endl 
                  << "</Secondo>" << endl;
                
         //Open the callback connection
         Socket* gate = Socket::CreateGlobal( HostIP, port);
         Socket* worker = gate->Accept();

         iostream& cbsock = worker->GetSocketStream();
                
         GenericRelation* rel = (Relation*)elements[arg2].addr;
         TupleType* tt = new TupleType(nl->Second(type));
                
         //receive tuples
         getline(cbsock,line);
         while(line=="<TUPLE>")
         {
            //receive size of tuple
            getline(cbsock,line);
            size_t size = atoi(line.data());
                     
            getline(cbsock,line);
                     
            int num_blocks = (size / 1024) + 1;
                  
            char* buffer = new char[num_blocks*1024];
            memset(buffer,0,num_blocks*1024);
                     
            //receive tuple data
            for(int i = 0; i < num_blocks; i++)
               cbsock.read(buffer+i*1024,1024);
                     
            Tuple* t = new Tuple(tt);
                     
            //transform to tuple and append to relation
            t->ReadFromBin(buffer);
            rel->AppendTuple(t);
                     
            t->DeleteIfAllowed();
            delete buffer;
                     
            getline(cbsock,line);
         }
                
         if(line != "<CLOSE>") 
            errorText = (string)"Unexpected Response from worker! " 
                              + "(<CLOSE> or <TUPLE> expected)";
                

         delete tt;
         gate->Close(); delete gate; gate=0;
         worker->Close(); delete worker; worker=0;

         do
         {
            getline(iosock,line);
            if(line.find("error") != string::npos)
               errorText = "worker reports error on sending relation!";
         }
         while(line.find("</SecondoResponse>") == string::npos);

      }
          
          
   }
        
   status = 0;
   delete arg;
           
   return;
}

/*

1.5 Multiply

Clones the DServer object
Makes more connections to the remote system available

*/

bool DServer::Multiply(int count)
{
   if(num_childs > 0) 
      return false;
     
   num_childs = count;
   childs = new DServer*[num_childs];
     
   for(int i = 0;i<num_childs;i++)
   {
      childs[i] = new DServer(host,port,name,type);
   }
   
   return true;
     
}

/*

1.6 DestroyChilds

Eliminates all the cloned DServer-Objects

*/

void DServer::DestroyChilds()
{
   for(int i = 0;i<num_childs;i++)
   {
      childs[i]->Terminate();
      delete childs[i];
   }
     
   delete childs; childs = 0;
     
   num_childs = 0;
}


/*

2 Implementation of the class DServerManager

2.1 Constructor

Creates a new DServerManager specified by a serverlist
All DServer-Obects are created

*/

DServerManager::DServerManager(ListExpr serverlist_n, 
                                                      string name_n, 
                                                      ListExpr type,
                                                      int sizeofarray)
{
   cout << "Connecting to Workers..." << endl;
   
   array_size = sizeofarray;
   name = name_n;
   errorText = "OK";
   size = nl->ListLength(serverlist_n);
   
   if(size==-1) 
      size=1;
        
   serverlist = new DServer*[size];
        
   ListExpr elem = nl->First(serverlist_n);
   serverlist_n = nl->Rest(serverlist_n);
        
   ZThread::ThreadedExecutor exec;
   //create DServer-objects
   for(int i = 0; i<size; i++)
   {
      DServerCreator* c = new DServerCreator(&serverlist[i],
                                          nl->StringValue(nl->First(elem)),
                                          nl->IntValue(nl->Second(elem)), 
                                          name,
                                          type);
      exec.execute(c);
      
      if(i < size-1)
      {
         elem = nl->First(serverlist_n);
         serverlist_n = nl->Rest(serverlist_n);
      }
   
   }
   exec.wait();
   
   for(int i = 0; i< size; i++)
      errorText = serverlist[i]->geterrorText();
}

/*

2.2 Destructor

All DServer-Objects controlled by the DServerManager are destoyed.

*/

DServerManager::~DServerManager()
{
   for(int i = 0; i<size; i++)
   {
      if(serverlist[i] != 0) 
         serverlist[i]->Terminate();
      delete serverlist[i];
   }
        
   delete serverlist;
}

/*

2.3 getServerbyID

returns the pointer to a DServer

*/

DServer* DServerManager::getServerbyID(int id)
{
   return serverlist[id];
}

/*

2.4 getServerByIndex

returns a pointer to the DServer that holds a certain element of the
underlying distributed array

*/

DServer* DServerManager::getServerByIndex(int index)
{
   return serverlist[index % size];
}

/*

2.5 getIndexList

returns a list of indices that correspond to the elements of the underlying
distributed array, which are controlled by the DServer with the given index

*/

list<int>* DServerManager::getIndexList(int id)
{
   list<int>* res = new list<int>;
   
   for(int i = id; i<array_size; i+=size)
      res->push_front(i);

   return res;
}

/*

2.6 getNoOfServers

returns the number of DServer-Objects controlled by the DServerManager

*/
                
        
int DServerManager::getNoOfServers() {return size;}

/*

2.7 getMultipleServerIndex

returns -1, if the parent DServer is the appropriate object for the element

*/

int DServerManager::getMultipleServerIndex(int index)
{
     return (index / size) - 1;
}


/*

3 Class RelationWriter

3.1 run

copies a relation to a remote system
can run as a thread of its own

*/
        
void RelationWriter::run()
{
   int index;
     
   while(!arg->empty())
   { 

      index = arg->front();
      arg->pop_front();
        
      list<int>* l = new list<int>;
      l->push_front(index);
     
      //create relation iterator
      GenericRelation* rel = (Relation*)elements[index].addr;
      GenericRelationIterator* iter = rel->MakeScan();
      
      Tuple* t;
     
     
      Word* word = new Word[1];

      t = iter->GetNextTuple();
     
      //open tuple stream to the worker
      word[0].addr = t;
      worker->setCmd("open_write_rel",l,word);
      worker->run();
     
     //send tuples
     while(t != 0)
     {
          
         word[0].addr = t;
         t->IncReference();
         worker->setCmd("write_rel",0,word);
          
         worker->run();
         t->DeleteIfAllowed();
      
         t = iter->GetNextTuple();
     
      }
     
      //close tuple stream
      worker->setCmd("close_write_rel",0,0);
      worker->run();
     
     
      delete iter;
   }
     
}

/*

5 Class DServerExecutor

*/
                        
void DServerExecutor::run()
{server->run(); }

/*

6 Class DServerCreator

*/

DServerCreator::DServerCreator
(DServer** s, string h, int p, string n, ListExpr t)
{
   string s_type = nl->ToString(t);
   nl->ReadFromString(s_type,type);
   server = s; host = h; port = p; name = n;
}

void DServerCreator::run()
{
   *server = new DServer(host,port,name,type);
}
