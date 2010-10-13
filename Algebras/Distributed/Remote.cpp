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
#include <sstream>
#include "SocketIO.h"
#include "Profiles.h"
#include "CSProtocol.h"
#include "RelationAlgebra.h"
#include "zthread/Runnable.h"
#include "zthread/Thread.h"
#include <netdb.h>
#include <iostream>



using namespace std;


void extractIds(const ListExpr,int&,int&);
string toString_d(int);

/*const string HostIP = "84.74.163.156";
const string HostIP_ = "h84_74_163_156";*/
string HostIP;
string HostIP_;

DServer::DServer(string n_host,int n_port,string n_name,ListExpr n_type)
{
     
        host = n_host;
        port = n_port;
        name = n_name;
        type = n_type;
     
        rel_open = false;
     
        num_childs = 0;
     char* g = new char[20];
     char* szIPAddress = new char[15];
     gethostname(g,20);
     cout << "Eigener Hostname ist: " << g << endl;
     //PHOSTENT host;
     //host = gethostbyname(g);
     //szIPAddress = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list); 
     //cout << "Eigene IP-Adresse ist: " << szIPAddress << endl;
        
        string line;
        
        server = Socket::Connect( host, toString_d(port), 
                                                Socket::SockGlobalDomain );
        if(server!=0 && server->IsOk())
        {
                iostream& iosock = server->GetSocketStream();
                csp = new CSProtocol(nl, iosock);
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
                                        //cout << "ALLES SUPER!!!";
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
        
     string ip;
     ip = server->GetSocketAddress();
     HostIP = ip;
     HostIP_ = "h" + replaceAll(ip,".","_");
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

void DServer::setCmd(string n_cmd, ListExpr n_arg,Word* n_array)
{
        
        cmd = n_cmd;
        arg = n_arg;
        elements = n_array;
}

void DServer::run()
{

        int arg2;
        ListExpr akt;
        
        if(cmd=="write")
        {
                int algID,typID;
                extractIds(type,algID,typID);
                string daten;
                
                
      
                do {
                if(!nl->IsAtom(arg)) akt = nl->First(arg);
                else akt = arg;
                arg2 = nl->IntValue(akt);
        
                iostream& iosock = server->GetSocketStream();
               string port =toString_d((1800+arg2)); 
               string com = "let r" + name + nl->ToString(akt) + 
                     " = " + "receiveD(" + HostIP_ + ",p" + port + ")";
                
                
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
                
                recF.Open("send");// + nl->ToString(arg));
                recF.AppendRecord(recID,rec);
                size_t size = 0;
                am->SaveObj(algID,typID,rec,size,type,elements[arg2]);
                
                char* buffer = new char[size]; 
                rec.Read(buffer,size,0);
                
                cbsock << "<SIZE>" << endl << size << endl << "</SIZE>" << endl;
                
                worker->Write(buffer,size);
                
                getline(cbsock,line);
                if(line!="<FINISH>") cout << "FEHLER";
                
                rec.Truncate(3);
                recF.DeleteRecord(recID);
                recF.Close();

                worker->Close();delete worker;worker=0;
               
                //string line;
                getline(iosock,line);
                
                if(line=="<SecondoResponse>")
                        do
                        {getline(iosock,line);}
                        while(line.find("</SecondoResponse") == string::npos);
                
                else cout << "DATENFEHLER";
                
                if(!nl->IsAtom(arg) && !nl->IsEmpty(arg)) arg = nl->Rest(arg);
                }
                while(/*!nl->IsAtom(arg) &&*/ !nl->IsEmpty(arg));
                //Daten auf den Server schreiben
        }
        
        if(cmd=="read")
        {
                int algID,typID;
                extractIds(type,algID,typID);
                
                string daten;
                
                do {
                if(!nl->IsAtom(arg)) akt = nl->First(arg);
                else akt = arg;
                arg2 = nl->IntValue(akt);
                ListExpr ls;string port =toString_d((1500+arg2));
                
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
                     
                     string n = "receive";// + nl->ToString(arg);
                     recF.Open("rec");
                     recF.AppendRecord(recID,rec);
                     rec.Write(buffer,size,0);
                     Word w;
                     size_t s = 0;
     
                     am->OpenObj(algID,typID,rec,s,type,w);
               
                     elements[arg2].addr = w.addr;
                     recF.DeleteRecord(recID);
                     recF.Close();
                    worker->Close();delete worker;worker=0;
                    //delete buffer;
                     //elements[arg2].addr = buffer;

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
                        
                        
                        ListExpr errorInfo = nl->OneElemList
                                                (nl->SymbolAtom("ERRORS"));
                        bool correct;
                        //elements[arg2] = ((am->InObj(algID,typID))
                           //             ( type, ls, 1, errorInfo, correct));
                }
                else cout << "DATENFEHLER LESEN";
                
                if(!nl->IsAtom(arg) && !nl->IsEmpty(arg)) arg = nl->Rest(arg);
                }
                while(!nl->IsAtom(arg) && !nl->IsEmpty(arg));
                //recF.Remove();
         }
        
        if(cmd=="delete")
        {
                do
                {
                    if(!nl->IsAtom(arg)) akt=nl->First(arg);
                     else akt = arg;
                     arg2 = nl->IntValue(akt);
                     //if(arg2 == 1) ZThread::Thread::sleep(1000);
               string line;
                iostream& iosock = server->GetSocketStream();
                iosock << "<Secondo>" << endl << "1" << endl << "delete r" 
                        << name << toString_d(arg2) << endl << "</Secondo>" 
                        << endl;
                do
                                getline(iosock,line);
                        while(line.find("</SecondoResponse>") == string::npos);
                if(!nl->IsAtom(arg) && !nl->IsEmpty(arg)) arg = nl->Rest(arg);
                }
                while(!nl->IsAtom(arg) && !nl->IsEmpty(arg));
          
        }
        
        if(cmd=="copy")
        {
                string line;
                iostream& iosock = server->GetSocketStream();
                string to = nl->StringValue(nl->First(arg));
                ListExpr list = nl->Second(arg);
                
                while(!nl->IsAtom(list))
                {
                        string cmd;
                        cmd = "let r" + to + nl->ToString(nl->First(list))
                                        + " = r" + name 
                                        + nl->ToString(nl->First(list));
                        iosock << "<Secondo>" << endl << "1" << endl 
                                << cmd<< endl << "</Secondo>" << endl;
                        list = nl->Rest(list);
                        do
                        {getline(iosock,line); /*cout << line;*/}
                        while(line.find("</SecondoResponse>") == string::npos);
                }
                
                string cmd;
                cmd = "let r" + to + nl->ToString(list) + " = r" 
                                + name + nl->ToString(list);
                iosock << "<Secondo>" << endl << "1" << endl << cmd 
                                << endl << "</Secondo>" << endl;
                do
                {getline(iosock,line);/*cout<<line;*/}
                        while(line.find("</SecondoResponse>") == string::npos);
                
                        
        }
        
        if(cmd=="execute")
        {
                string line;
                iostream& iosock = server->GetSocketStream();
                string to = nl->StringValue(nl->First(nl->First(arg)));
                string com = nl->StringValue(nl->Second(nl->First(arg)));
                ListExpr list = nl->Second(arg);
                
                while(!nl->IsAtom(list) && !nl->IsEmpty(list))
                {
                        string cmd;
                        string com_a = replaceAll(com,".","r" + name
                         + nl->ToString(nl->First(list)));
                        cmd = "let r" + to + nl->ToString(nl->First(list))
                                        + " = " + com_a;
                        iosock << "<Secondo>" << endl << "1" << endl 
                                << cmd<< endl << "</Secondo>" << endl;
                        list = nl->Rest(list);
                        do
                        {getline(iosock,line); /*cout << line;*/}
                        while(line.find("</SecondoResponse>") == string::npos);
                }
                
                string cmd;
                string com_a = replaceAll(com,".","r" + name
                                   + nl->ToString(list));
                        cmd = "let r" + to + nl->ToString(list)
                                        + " = " + com_a;
                iosock << "<Secondo>" << endl << "1" << endl << cmd 
                                << endl << "</Secondo>" << endl;
                do
                {getline(iosock,line);/*cout<<line;*/}
                        while(line.find("</SecondoResponse>") == string::npos);
                
                        
        }
        
        if(cmd=="open_write_rel")
        {
             //Initializes the writing of a tuple-stream, 
             //the d_receive_rel operator is started on the remote worker
             
             string line;
             iostream& iosock = server->GetSocketStream();
              int  arg2 = nl->IntValue(arg);
              string port =toString_d((1800+arg2)); 
               string com = "let r" + name + nl->ToString(arg) + 
                     " = " + "d_receive_rel(" + HostIP_ + ",p" + port + ")";
                
                
                iosock << "<Secondo>" << endl << "1" << endl << "delete r" 
                        << name << nl->ToString(arg)  << endl << "</Secondo>" 
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
                        
                //Tuple *tpl = (Tuple*)elements[0].addr;
                //TupleType* tt = tpl->GetTupleType();
                        
                                        
                getline(cbsock1,line);
                if(line!="<CLOSE>") cout << "FEHLER";
                cbworker->Close();delete cbworker;
                        
                cbworker = gate->Accept();
                iostream& cbsock2 = cbworker->GetSocketStream();
                
                cbsock2 << "<TYPE>" << endl << nl->ToString(type) 
                        << endl << "</TYPE>" << endl;
                //cbsock2.write((char*)tt,sizeof(TupleType));
                
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
               
               tpl->WriteToBin(buffer,cS,eS,fS);
               
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
               string line;

               iostream& cbsock = cbworker->GetSocketStream();
               iostream& iosock = server->GetSocketStream(); 
               
               cbsock << "<CLOSE>" << endl;
               cbworker->ShutDown(); //delete cbworker; cbworker = 0;
               
                getline(iosock,line);
                
                if(line=="<SecondoResponse>")
                        do
               {getline(iosock,line);}
                        while(line.find("</SecondoResponse") == string::npos);
                
                else cout << "DATENFEHLER, keine Antwort";
                
                rel_open = false;
           }

               
           if(cmd == "read_rel")
           {
                    
                int algID,typID;
                ListExpr akt;
                extractIds(type,algID,typID);
          
          do
          {
          if(!nl->IsAtom(arg)) akt = nl->First(arg);
          else akt = arg;
                
                arg2 = nl->IntValue(akt);
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
                     
                     //iosock << toString_d(num_blocks);
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
                
                //elements[arg2].addr = rel; //delete rel;
                
                gate->Close(); delete gate; gate=0;
                worker->Close(); delete worker; worker=0;
                
                
                getline(iosock,line);
      
                ListExpr ls;
                if(line=="<SecondoResponse>")
                {
                        nl->ReadBinaryFrom(iosock, ls);
                        string debug_out = nl->ToString(ls);
                        //cout << debug_out;
                        
                        do
                                getline(iosock,line);
                        while(line.find("</SecondoResponse>") == string::npos);
                        
                        ls = nl->Second(nl->Fourth(ls));
                        
                        
                        ListExpr errorInfo = nl->OneElemList
                                                (nl->SymbolAtom("ERRORS"));
                        bool correct;
                        //elements[arg2] = ((am->InObj(algID,typID))
                           //             ( type, ls, 1, errorInfo, correct));
                }
                else cout << "DATENFEHLER LESEN";
          
          if(!nl->IsEmpty(arg) && !nl->IsAtom(arg)) arg=nl->Rest(arg);
          }while(!nl->IsEmpty(arg) && !nl->IsAtom(arg));
           }
        
                     
                     
     //ZThread::Thread::cancel();
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
        
        for(int i = 0; i<size; i++)
        {
            serverlist[i] = new DServer(nl->StringValue(nl->First(elem)),
                                                nl->IntValue(nl->Second(elem)), 
                                                        name,
                                                        type);
                if(i < size-1){elem = nl->First(serverlist_n);
                serverlist_n = nl->Rest(serverlist_n);}
        }
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

ListExpr DServerManager::getIndexList(int id)
{
        ListExpr res = nl->TheEmptyList();
        for(int i = id; i<array_size; i+=size)
        {
                res = nl->Cons(nl->IntAtom(i),res);
        }
        return res;
}
                
ListExpr DServerManager::getNamedIndexList(int id)
{
        ListExpr res = nl->TheEmptyList();
        for(int i = id; i<array_size; i+=size)
        {
                res = nl->Cons(nl->TwoElemList(nl->StringAtom(name),
                                        nl->IntAtom(i)), res);
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
     
     do
     { 
          if(!nl->IsAtom(arg)) akt = nl->First(arg);
          else akt = arg;
          index = nl->IntValue(akt);
     
     GenericRelation* rel = (Relation*)elements[index].addr;
        GenericRelationIterator* iter = rel->MakeScan();
     
     
     Tuple* t;
     
     
     Word* word = new Word[1];

     t = iter->GetNextTuple();
     word[0].addr = t;
     worker->setCmd("open_write_rel",nl->IntAtom(index),word);
     worker->run();
     
     while(t != 0)
     {
          
          word[0].addr = t;t->IncReference();
          worker->setCmd("write_rel",nl->TheEmptyList(),word);
          
          worker->run();
          t->DeleteIfAllowed();
      
          t = iter->GetNextTuple();
     
    }
     
     worker->setCmd("close_write_rel",nl->TheEmptyList(),0);
     worker->run();
     
     
     delete iter;
     
     if(!nl->IsAtom(arg) && !nl->IsEmpty(arg)) arg=nl->Rest(arg);
     }while(!nl->IsAtom(arg) && !nl->IsEmpty(arg));
     
}
                        
void DServerExecutor::run()
{server->run(); }