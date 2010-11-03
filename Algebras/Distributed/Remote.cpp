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

*/

//Implements DServer
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

string HostIP;
string HostIP_;

ZThread::Mutex Flob_Mutex;

DServer::DServer(string n_host,int n_port,string n_name,ListExpr n_type)
{
     
        host = n_host;
        port = n_port;
        name = n_name;
   type = n_type;
   
     
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
                                do{
                                        getline( iosock, line);}
                                while(line != "</SecondoIntro>");
                                   
                        }
                        else cout << "SERVERERROR 2!!!";
                }
                else cout << "SERVERERROR!!!";
        }
        else cout << "CONNECTERROR";
        
        iostream& iosock = server->GetSocketStream();
        iosock << "<Secondo>" << endl << "1" << endl 
                << "open database distributed" << endl 
                << "</Secondo>" << endl;
        getline( iosock, line );
        
        if(line=="<SecondoResponse>")
        {
                do{
                        getline( iosock, line );
                        if (line[line.size() - 1] == '\r')
                        line.resize(line.size() - 1);
                        
                }while(line.find("</SecondoResponse>") == string::npos);
        }
        else cout << "ERROR3";
        

     HostIP = server->GetSocketAddress();
     HostIP_ = "h" + replaceAll(HostIP,".","_");
}

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

void DServer::setCmd(string n_cmd, list<int>* n_arg,Word* n_array)
{
        
        cmd = n_cmd;
        arg = n_arg;
        elements = n_array;
}

void DServer::run()
{

        int arg2;

        
        if(cmd=="write")
        {
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
               string com = "let r" + name + toString_d(arg2) + 
                     " = " + "receiveD(" + HostIP_ + ",p" + port + ")";
                
                
      if(t->NumOfFLOBs() > 0)
      {
         Attribute* a = static_cast<Attribute*>
                  ((am->Cast(algID,typID))
                  ((elements[arg2]).addr));
         Flob* f = a->GetFLOB(0);
         
         SmiSize si = f->getSize();
         char* buf = new char[si];
         
         f->read(buf,si,0);
         
      }	
         
                iosock << "<Secondo>" << endl << "1" << endl << "delete r" 
                        << name << toString_d(arg2)  << endl << "</Secondo>" 
                        << endl;
                string line;
                getline(iosock,line);
                if(line=="<SecondoResponse>")
                        do
                        getline(iosock,line); 
                        while(line.find("</SecondoResponse") == string::npos);
                
                //else cout << "DATENFEHLER";

                iosock << "<Secondo>" << endl << "1" << endl 
                                << com << endl << "</Secondo>" << endl;
                
                
                Socket* gate = Socket::CreateGlobal( HostIP, port);

                Socket* worker = gate->Accept();

                iostream& cbsock1 = worker->GetSocketStream();
                cbsock1 << "<TYPE>" << endl << nl->ToString(type) 
                        << endl << "</TYPE>" << endl;
                getline(cbsock1,line);
                if(line!="<CLOSE>") cout << "FEHLER";
                worker->Close();delete worker;
                        
                worker = gate->Accept(); gate->Close();delete gate;gate=0;
                iostream& cbsock = worker->GetSocketStream();
                
                cbsock << "<TYPE>" << endl << nl->ToString(type) 
                    << endl << "</TYPE>" << endl;

                SmiRecordFile recF(false,0);
                SmiRecord rec;
                SmiRecordId recID;
                
                recF.Open("send");
                recF.AppendRecord(recID,rec);
                size_t size = 0;
                am->SaveObj(algID,typID,rec,size,type,elements[arg2]);
                
                char* buffer = new char[size]; 
                rec.Read(buffer,size,0);
                

                cbsock << "<SIZE>" << endl << size << endl << "</SIZE>" << endl;
                
                worker->Write(buffer,size);
      delete buffer;
      
      Attribute* a;
      if(t->NumOfFLOBs() > 0 ) 
         a = static_cast<Attribute*>((am->Cast(algID,typID))
                  ((elements[arg2]).addr));
      for(int i = 0; i < t->NumOfFLOBs(); i++)
      {
         Flob* f = a->GetFLOB(i);
         
         SmiSize si = f->getSize();
         int n_blocks = si / 1024 + 1;
         char* buf = new char[n_blocks*1024];
         memset(buf,0,1024*n_blocks);
         
         f->read(buf,si,0);
         
         cbsock << "<FLOB>" << endl << "<SIZE>" << endl 
               << si << endl << "</SIZE>" << endl;
         for(int j = 0; j<n_blocks;j++)
            worker->Write(buf+j*1024,1024);
         cbsock << "</FLOB>" << endl;
      }
      
      cbsock << "<CLOSE>" << endl;
         
                
                getline(cbsock,line);
                if(line!="<FINISH>") cout << "FEHLER";
                
                rec.Truncate(3);
                recF.DeleteRecord(recID);
                recF.Close();

                worker->Close();delete worker;worker=0;
               
                
                getline(iosock,line);
                
                if(line=="<SecondoResponse>")
                        do
                        {getline(iosock,line);}
                        while(line.find("</SecondoResponse") == string::npos);
                
                else cout << "DATENFEHLER";
                

                }

                
        }
        
        if(cmd=="read")
        {
                int algID,typID;
                extractIds(type,algID,typID);
                
                string daten;
                
                while(!arg->empty())
               {

                ListExpr ls;
               arg2 = arg->front();
               arg->pop_front();
               string port =toString_d((1500+arg2));
                
                iostream& iosock = server->GetSocketStream();
                iosock << "<Secondo>" << endl << "1" << endl 
                     << "query sendD (" << HostIP_ << ",p" << port << ",r" 
                        << name << toString_d(arg2) << ")" <<  endl 
                         << "</Secondo>" << endl;
                
                     
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
                     getline(cbsock,line);
                     int size = atoi(line.data());
                     getline(cbsock,line);

                    
                    char* buffer = new char[size];memset(buffer,0,size);
                    cbsock.read(buffer,size);
                     SmiRecordFile recF(false,0);
                     SmiRecord rec;
                     SmiRecordId recID;
                     
                     string n = "receive";
                     recF.Open("rec");
                     recF.AppendRecord(recID,rec);
                     rec.Write(buffer,size,0);
                     Word w;
                     size_t s = 0;
     
                     am->OpenObj(algID,typID,rec,s,type,w);
               
                     elements[arg2].addr = w.addr;
                     recF.DeleteRecord(recID);
                     recF.Close();
            
           getline(cbsock,line);
           int flobs=0;
           
           Flob_Mutex.acquire();
           
           while(line=="<FLOB>")
         {
            getline(cbsock,line);
            if(line!="<SIZE>") cout << "ERROR";
            getline(cbsock,line);
            SmiSize si = atoi(line.data());
            getline(cbsock,line);
            if(line!="</SIZE>") cout << "ERROR";
            
            int n_blocks = si / 1024 + 1;
            char* buf = new char[n_blocks*1024];
            memset(buf,0,1024*n_blocks);
            for(int i = 0; i< n_blocks; i++)
               cbsock.read(buf+1024*i,1024);
            
            
            Attribute* a = static_cast<Attribute*>
               ((am->Cast(algID,typID))(w.addr));
            
            
            Flob*  f = a->GetFLOB(flobs);
            if(f->getSize() != si) cout << "Flob-Größe inkorrekt" <<endl;
            f->write(buf,si,0);
            
            
            delete buf;
            
            getline(cbsock,line);
            if(line!="</FLOB>") cout << "ERROR";
            
            getline(cbsock,line);
            flobs++;
            //receive FLOB
         }
         
         Flob_Mutex.release();
         
         if(line!="<CLOSE>") cout << "ERROR";
                         
                         iosock << "<FINISH>" << endl;
         
                    worker->Close();delete worker;worker=0;

                }
                else cout << "FEHLER BEI CALLBACK";
                
               
                
                getline(iosock,line);
      
                
                if(line=="<SecondoResponse>")
                {
                        nl->ReadBinaryFrom(iosock, ls);
                        string debug_out = nl->ToString(ls);
                                                
                        do
                                getline(iosock,line);
                        while(line.find("</SecondoResponse>") == string::npos);
                        
                        ls = nl->Second(nl->Fourth(ls));
                        
                        
                }
                else cout << "DATENFEHLER LESEN";
                

                }

         }
        
        if(cmd=="delete")
        {
                while(!arg->empty())
                {
                   arg2 = arg->front();
                   arg->pop_front();

               string line;
                iostream& iosock = server->GetSocketStream();
                iosock << "<Secondo>" << endl << "1" << endl << "delete r" 
                        << name << toString_d(arg2) << endl << "</Secondo>" 
                        << endl;
                do
                                getline(iosock,line);
                        while(line.find("</SecondoResponse>") == string::npos);

                }

        }
        
        if(cmd=="copy")
        {
                string line;
                iostream& iosock = server->GetSocketStream();

                string to = ((string*)(elements[0].addr))->data();
               
           

            while(!arg->empty())
                {
                        arg2 = arg->front();
                        arg->pop_front();
                        string cmd;
                        cmd = "let r" + to + nl->ToString(arg2)
                                        + " = r" + name 
                                        + nl->ToString(arg2);
                        iosock << "<Secondo>" << endl << "1" << endl 
                                << cmd<< endl << "</Secondo>" << endl;

                        do
                        {getline(iosock,line); }
                        while(line.find("</SecondoResponse>") == string::npos);
                }
                

                
                        
        }
        
        if(cmd=="execute")
        {
                string line;
                iostream& iosock = server->GetSocketStream();
                string to = ((string*)(elements[0].addr))->data();
                string com = ((string*)(elements[1].addr))->data();

                 while(!arg->empty())
                {
                        arg2 = arg->front();
                        arg->pop_front();
                        string cmd;
                        string com_a = replaceAll(com,"!","r" + name
                         + toString_d(arg2));
                        cmd = "let r" + to + toString_d(arg2)
                                        + " = " + com_a;
         cout << "Execute: " << cmd << endl;
                        iosock << "<Secondo>" << endl << "1" << endl 
                                << cmd<< endl << "</Secondo>" << endl;
                      
                        do
                        {getline(iosock,line);}
                        while(line.find("</SecondoResponse>") == string::npos);
                }
                
                
                
                        
        }
        
        if(cmd=="open_write_rel")
        {
             //Initializes the writing of a tuple-stream, 
             //the d_receive_rel operator is started on the remote worker
             
             string line;
             iostream& iosock = server->GetSocketStream();
              
              int arg2 = arg->front();
              string port =toString_d((1800+arg2)); 
               string com = "let r" + name + toString_d(arg2) + 
                     " = " + "d_receive_rel(" + HostIP_ + ",p" + port + ")";
                
                
                iosock << "<Secondo>" << endl << "1" << endl << "delete r" 
                        << name << toString_d(arg2)  << endl << "</Secondo>" 
                        << endl;
                
                getline(iosock,line);
                if(line=="<SecondoResponse>")
                        do
                        getline(iosock,line); 
                        while(line.find("</SecondoResponse") == string::npos);
                        
               iosock << "<Secondo>" << endl << "1" << endl 
                                << com << endl << "</Secondo>" << endl;
                
                
                Socket* gate = Socket::CreateGlobal( HostIP, port);

                cbworker = gate->Accept();
                        
                iostream& cbsock1 = cbworker->GetSocketStream();
                cbsock1 << "<TYPE>" << endl << nl->ToString(type) 
                        << endl << "</TYPE>" << endl;
                        
                                        
                getline(cbsock1,line);
                if(line!="<CLOSE>") cout << "FEHLER";
                cbworker->Close();delete cbworker;
                        
                cbworker = gate->Accept();
                iostream& cbsock2 = cbworker->GetSocketStream();
                
                cbsock2 << "<TYPE>" << endl << nl->ToString(type) 
                        << endl << "</TYPE>" << endl;
                
               gate->Close();delete gate;gate=0;
                        
               rel_open = true;
               
          }
          
          if(cmd == "write_rel")
          {
               if(!rel_open) return;
               
               string line;
               
               Tuple *tpl = (Tuple*)elements[0].addr;
               size_t cS,eS,fS,size;
               size = tpl->GetBlockSize(cS,eS,fS);
               
               
               int num_blocks = (size / 1024) + 1;
               
               char* buffer = new char[num_blocks*1024];
               memset(buffer,0,1024*num_blocks);
               
          Flob_Mutex.acquire();
               tpl->WriteToBin(buffer,cS,eS,fS);
          Flob_Mutex.release();
               
               iostream& cbsock = cbworker->GetSocketStream();
               
               cbsock << "<TUPLE>" << endl << toString_d(size) 
               << endl << "</TUPLE>" << endl;
               getline(cbsock,line); 
               if(line!= "<OK>") {cout << "Worker nicht bereit!"; return;}
               
               
               getline(cbsock,line); if(atoi(line.data()) != num_blocks) 
               {cout << "Falsche Blockzahl von Worker"; return;}
               getline(cbsock,line);
               
               for(int i = 0; i<num_blocks;i++)
               {
                    cbsock.write(buffer+i*1024,1024);
               }
               
               tpl->DeleteIfAllowed();
          
          }
          
          if(cmd == "close_write_rel")
          {
      if(rel_open){
               string line;

               iostream& cbsock = cbworker->GetSocketStream();
               iostream& iosock = server->GetSocketStream(); 
               
               cbsock << "<CLOSE>" << endl;
               getline(cbsock,line);
               if(line != "<FINISH>") cout << "ERROR: " << line << endl;
               cbworker->Close(); delete cbworker; cbworker = 0;
               
                getline(iosock,line);
                
                if(line=="<SecondoResponse>")
                        do
               {getline(iosock,line);}
                        while(line.find("</SecondoResponse") == string::npos);
                
                else cout << "DATENFEHLER, keine Antwort";
                
                rel_open = false;
           }}

               
           if(cmd == "read_rel")
           {
                    
                int algID,typID;
                ListExpr akt;
                extractIds(type,algID,typID);
          
          while(!arg->empty())
          {
       
             arg2 = arg->front();
             arg->pop_front();
                string line;        
                string port =toString_d((1300+arg2));
                
                char* buffer = 0;
                
                iostream& iosock = server->GetSocketStream();
                iosock << "<Secondo>" << endl << "1" << endl 
                     << "query d_send_rel (" << HostIP_ << ",p" << port << ",r" 
                        << name << toString_d(arg2) << ")" <<  endl 
                         << "</Secondo>" << endl;
                
                     
                Socket* gate = Socket::CreateGlobal( HostIP, port);

                Socket* worker = gate->Accept();

                iostream& cbsock = worker->GetSocketStream();
                
                GenericRelation* rel = (Relation*)elements[arg2].addr;
                
                getline(cbsock,line);
                while(line=="<TUPLE>")
                {
                    getline(cbsock,line);
                    size_t size = atoi(line.data());
                     
                     getline(cbsock,line);
                     
                     int num_blocks = (size / 1024) + 1;
                     
                     if(buffer!=0) delete buffer;
                     buffer = new char[num_blocks*1024];
                     memset(buffer,0,num_blocks*1024);
                     

                     for(int i = 0; i < num_blocks; i++)
                          cbsock.read(buffer+i*1024,1024);
                     
                    TupleType* tt = new TupleType(nl->Second(type));
                     
                    Tuple* t = new Tuple(tt);
                     
                     t->ReadFromBin(buffer);
                     rel->AppendTuple(t);
                     
                     t->DeleteIfAllowed();
                     
                     getline(cbsock,line);
                }
                
                if(line != "<CLOSE>") 
                     cout << "Fehlerhaftes Ende des Relationempfangs";
                

                
                gate->Close(); delete gate; gate=0;
                worker->Close(); delete worker; worker=0;
                
                
                getline(iosock,line);
      
                ListExpr ls;
                if(line=="<SecondoResponse>")
                {
                        nl->ReadBinaryFrom(iosock, ls);
                        string debug_out = nl->ToString(ls);

                        
                        do
                                getline(iosock,line);
                        while(line.find("</SecondoResponse>") == string::npos);
                        
                        ls = nl->Second(nl->Fourth(ls));
                        
                        
                        ListExpr errorInfo = nl->OneElemList
                                                (nl->SymbolAtom("ERRORS"));
                }
                else cout << "DATENFEHLER LESEN";
          
          
          }
           }
        
                     
                     
                status = 0;
           
           return;
}

bool DServer::Multiply(int count)
{
     if(num_childs > 0) return false;
     
     num_childs = count;
     childs = new DServer*[num_childs];
     
     for(int i = 0;i<num_childs;i++)
     {
          childs[i] = new DServer(host,port,name,type);
     }
     
     return true;
     
}

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

DServerManager::DServerManager(ListExpr serverlist_n, 
                                                      string name_n, 
                                                      ListExpr type,
                                                      int sizeofarray)
{
        array_size = sizeofarray;
        
        name = name_n;
        
        size = nl->ListLength(serverlist_n);
        if(size==-1) size=1;
        

        serverlist = new DServer*[size];
        
        ListExpr elem = nl->First(serverlist_n);
        serverlist_n = nl->Rest(serverlist_n);
        
   ZThread::ThreadedExecutor exec;
        for(int i = 0; i<size; i++)
        {
       DServerCreator* c = new DServerCreator(&serverlist[i],
                  nl->StringValue(nl->First(elem)),
                                                nl->IntValue(nl->Second(elem)), 
                  name,
                                                type);
      exec.execute(c);
            /*serverlist[i] = new DServer(nl->StringValue(nl->First(elem)),
                                                nl->IntValue(nl->Second(elem)), 
                                                        name,
                                                        type);*/
                if(i < size-1){elem = nl->First(serverlist_n);
                serverlist_n = nl->Rest(serverlist_n);}
        }
   exec.wait();
}

DServerManager::~DServerManager()
{
        for(int i = 0; i<size; i++)
        {
                if(serverlist[i] != 0) serverlist[i]->Terminate();
                delete serverlist[i];
        }
        
        delete serverlist;
}

DServer* DServerManager::getServerbyID(int id)
{
        return serverlist[id];
}

DServer* DServerManager::getServerByIndex(int index)
{
        return serverlist[index % size];
}

list<int>* DServerManager::getIndexList(int id)
{
       list<int>* res = new list<int>;
        for(int i = id; i<array_size; i+=size)
        {
               res->push_front(i);
        }
        return res;
}
                
        
int DServerManager::getNoOfServers() {return size;}

int DServerManager::getMultipleServerIndex(int index)
{
     return (index / size) - 1;
}
        
void RelationWriter::run()
{
     ListExpr akt;
     int index;
     
     while(!arg->empty())
     { 
          /*f(!nl->IsAtom(arg)) akt = nl->First(arg);
          else akt = arg;
          index = nl->IntValue(akt);*/
        index = arg->front();
        arg->pop_front();
        
   list<int>* l = new list<int>;
         l->push_front(index);
     
     GenericRelation* rel = (Relation*)elements[index].addr;
        GenericRelationIterator* iter = rel->MakeScan();
     
     
     Tuple* t;
     
     
     Word* word = new Word[1];

     t = iter->GetNextTuple();
     word[0].addr = t;
     worker->setCmd("open_write_rel",l,word);
     worker->run();
     
     while(t != 0)
     {
          
          word[0].addr = t;
          t->IncReference();
          worker->setCmd("write_rel",0,word);
          
          worker->run();
          t->DeleteIfAllowed();
      
          t = iter->GetNextTuple();
     
    }
     
     worker->setCmd("close_write_rel",0,0);
     worker->run();
     
     
     delete iter;
     
     //if(!nl->IsAtom(arg) && !nl->IsEmpty(arg)) arg=nl->Rest(arg);
     }//while(!nl->IsAtom(arg) && !nl->IsEmpty(arg));
     
}
                        
void DServerExecutor::run()
{server->run(); }

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
