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
#include "Processes.h"
#include "RelationAlgebra.h"
#include "zthread/Runnable.h"
#include "zthread/Thread.h"
#include "zthread/ThreadedExecutor.h"
#include "zthread/Mutex.h"
#include "StringUtils.h"
#include <iostream>


//#define DS_CMD_CLOSE_WRITE_REL_DEBUG 1
//#define DS_CMD_OPEN_WRITE_REL_DEBUG 1
//#define DS_CMD_WRITE_REL_DEBUG 1

using namespace std;


void extractIds(const ListExpr,int&,int&);
string toString_d(int);

//IP-Adress of this system (Master)
string HostIP;
string HostIP_;

//Synchronisation of access to Flobs
ZThread::Mutex Flob_Mutex;
//Synchronisation of access to read relations
ZThread::Mutex Rel1_Mutex;
ZThread::Mutex Rel2_Mutex;
//Synchronisation of access to read non relations and write commands
ZThread::Mutex Cmd_Mutex;
//Synchronisation of access to nl -> ToString in DServer::DServer
ZThread::Mutex Type_Mutex;

/*

1 Class DServer

1.1 Constructor

Constructs a DServer
A TCP-connection to the corresponding worker ist opened

*/

//#define DEBUG_DISTRIBUTE 1

Word DServer::ms_emptyWord;

void DServer::Debug(const string& tag, const string& out)
{
#if defined (DEBUG_DISTRIBUTE)
  cout << tag << ": " << out << endl;
#endif
}


DServer::DServer(string n_host,int n_port,string n_name,
                 ListExpr inType)
{
   host = n_host;
   port = n_port;
   name = n_name;
   m_type = inType;

   Type_Mutex.acquire();
   m_typeStr = nl -> ToString(m_type);
   Type_Mutex.release();

   errorText = "OK";
   
   rel_open = false;
   m_cmd = NULL;
   m_numChilds = 0;
}

bool
DServer::connectToWorker()
{        
  string line;
  server = Socket::Connect( host, toString_d(port), 
                      Socket::SockGlobalDomain,
                      5,
                      1);
      
  if(server!=0 && server->IsOk())
    {
      DServer::Debug("DS-Conn", "starting ...");
      
      iostream& iosock = server->GetSocketStream();
      
      if (!server -> IsOk())
      {
        cout << "Error: Faild to establish socket for host " 
             << host << ":" << port << "!" << endl;
      }
      
      do
      {
        getline( iosock, line );
              
        DServer::Debug("DSConn-Rec1", line);
      } while (line.empty());
      
      if (!server -> IsOk())
      {
        cout << "Error: Faild to establish socket for connection"
             << " to host " << host << ":" << port << "!" << endl;
      }
      
      if(line=="<SecondoOk/>")
      {
        iosock << "<Connect>" << endl << endl 
             << endl << "</Connect>" << endl;
        
        getline( iosock, line );
          
        DServer::Debug("DSConn-Rec2", line);
        if( line == "<SecondoIntro>")
          {
            do
            {
              getline( iosock, line);
                  
              DServer::Debug("DSConn-Rec3", line);
              
            }  while(line != "</SecondoIntro>");
            
            DServer::Debug("DSConn", "... done.");
              
          }
        else 
          errorText = 
            "Unexpected response from worker (No <SecondoIntro/>)";
      }
      else 
      errorText = "Unexpected response from worker (No <SecondoOk/>)";
    }
  else 
    errorText = "Connection to the worker couldn't be established!";
  
  if (server == 0)
    {
      // should never happen
      // Socket::Connect always returns a pointer!
      assert(0);
      return false;
    }
      
  if (!(server -> IsOk()))
    { 
      cout << "Cannot Connect to Server:" 
         << host << ":" << toString_d(port) << endl;
      cout << server -> GetErrorText() << endl;

      delete server;
      server = 0;

      return false;
    } // if (!(server -> IsOk()))

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
   
         //cout << "   " << line << endl;
         /*if (line[line.size() - 1] == '\r')
            line.resize(line.size() - 1);*/
         if(line.find("ERROR") != string::npos)
         {
           errorText = 
             "Opening of database \"distributed\" on worker failed!";
           return false;
         }
                        
      }
      while(line.find("</SecondoResponse>") == string::npos);
   }
   else 
      errorText = "Unexpected response from worker (No <SecondoResponse>)";
        

   HostIP = server->GetSocketAddress();
   HostIP_ = "h" + stringutils::replaceAll(HostIP,".","_");
   
   cout << "Connection to Worker on " << host << " established." << endl;
   return true;
}

/*

1.2 Implementation of Terminate

Breakes the connection to the remote system

*/

void DServer::Terminate()
{
   if(server != 0)
   {
     if (!(server->IsOk()))
       {
         cout << "Error: Cannot close connection to " << host << "!" << endl;
         cout << server -> GetErrorText() << endl;
       }
     else
       {
         iostream& iosock = server->GetSocketStream();
         iosock << "<Disconnect/>" << endl;
       server->Close();
       }
     delete server;
     server=0;
   }
   else
     {
       cout << "Error: No Server Running!" << endl;
     }

   if (m_cmd != NULL)
     delete m_cmd;
   m_cmd = NULL;
}

/*

1.3 Implementation of setCmd

Sets new paramaters that are needed for the run-method

*/

void DServer::setCmd(CmdType inCmdType, 
                     const list<int>* inIndex,
                     vector<Word>* inElements,
                     vector<string>* inFromNames)
{
  if(m_cmd == NULL)
    {
      m_cmd =  new RemoteCommand(inCmdType, inIndex, 
                                 inElements, inFromNames);
 
      //cout << this << " GOOD: DServer::setCmd" << (*m_cmd) << endl;
    }
  else
    {
      //cout << this << " BAD: DServer::setCmd" << (*m_cmd) << endl;
      assert(0);
    }
}

/*

1.4 Implementation of run

Performs the specified operation on the remote system

*/

void DServer::run()
{
  // cout << this << " DS - starting:"  << endl;
  assert(m_cmd != NULL);
    
  int arg2;
  string sendCmd;

  string myTupleType;
  
  //cout << "Running:" << (*m_cmd) << " T:" << endl;
  //cout << nl -> ToString(m_type) << endl;

   if(m_cmd -> getCmdType() == DS_CMD_WRITE)
   {
      if(rel_open) 
        return;
      
      int algID,typID;
      extractIds(m_type,algID,typID);
      
      string daten;
                
      TypeConstructor* t = am->GetTC(algID,typID);
      
      while(!m_cmd -> getDArrayIndex() -> empty())
      {

         arg2 = m_cmd -> getDArrayIndex() ->front();
         m_cmd -> getDArrayIndex() ->pop_front();
        
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

         if (!(gate -> IsOk()))
           {
             errorText = "Could not open connection to worker!\n";
             errorText += gate -> GetErrorText();
             return;
           }

         Socket* worker = gate->Accept();

         if (worker == NULL)
           {
             errorText = "Could not connect to worker!";
             return;
           }

         iostream& cbsock1 = worker->GetSocketStream();

         //The element-type is sent to the type-mapping-fcts of receiveD
         cbsock1 << "<TYPE>" << endl << m_typeStr
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

         cbsock << "<TYPE>" << endl << m_typeStr
                    << endl << "</TYPE>" << endl;

         //The element is converted into a binary stream of data
         SmiRecordFile recF(false,0);
         SmiRecord rec;
         SmiRecordId recID;
         Cmd_Mutex.acquire();       
         recF.Open("send");
         recF.AppendRecord(recID,rec);
         size_t size = 0;
         am->SaveObj(algID,typID,rec,size,m_type,
                     (*(m_cmd -> getElements()))[arg2]);
         char* buffer = new char[size]; 
         rec.Read(buffer,size,0);
         //rec.Truncate(3);
         recF.DeleteRecord(recID);
         recF.Close();
         Cmd_Mutex.release();
         //Size of the binary data is sent
         cbsock << "<SIZE>" << endl << size << endl << "</SIZE>" << endl;
                
         //The actual data are sent
         worker->Write(buffer,size);
 
         delete [] buffer ;

         Attribute* a;
         if(t->NumOfFLOBs() > 0 ) 
            a = static_cast<Attribute*>((am->Cast(algID,typID))
                              (((*(m_cmd -> getElements()))[arg2]).addr));
         
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
        
   if(m_cmd -> getCmdType() == DS_CMD_READ)
   {
      if(rel_open) return;

      int algID,typID;
      extractIds(m_type,algID,typID);
                
      string daten;
                
 
      while(!m_cmd -> getDArrayIndex() -> empty())
        {
          arg2 = m_cmd -> getDArrayIndex() -> front();
          m_cmd -> getDArrayIndex() -> pop_front();
          string port =toString_d((1500+arg2));
          
          //The sendD-operator on the worker is started       
          iostream& iosock = server->GetSocketStream();
          
          DServer::Debug("DSR-READ send iosock",  "<Secondo> 1 query sendD (" + 
                         HostIP_ + ",p" + port + ",r" + name +
                         toString_d(arg2) + ") </Secondo>");

          iosock << "<Secondo>" << endl << "1" << endl 
                 << "query sendD (" << HostIP_ << ",p" << port << ",r" 
                 << name << toString_d(arg2) << ")" <<  endl 
                 << "</Secondo>" << endl;
      
          
          //Callback-connection is received
          Socket* gate = Socket::CreateGlobal( HostIP, port);
          
          Socket* worker = gate->Accept();
          gate->Close();delete gate;gate=0;
          
          iostream& cbsock = worker->GetSocketStream();
          cbsock << "<TYPE>" << endl << m_typeStr
                 << endl << "</TYPE>" << endl;
          
          DServer::Debug("DSR-READ send cbsock","<TYPE>" + 
                         m_typeStr + "</TYPE>");
          DServer::Debug("DSR-READ"," ... done");

          string line;
          getline(cbsock,line);
          
          DServer::Debug( "DSR-READ1", line);

          if(line=="<SIZE>")
            {
              //Size of binary data is received
              getline(cbsock,line);
              DServer::Debug("DSR-READ2", line);
              int size = atoi(line.data());
              getline(cbsock,line);
              
              DServer::Debug("DSR-READ3", line);
              //The actual data is received...       
              char* buffer = new char[size];
              memset(buffer,0,size);
              cbsock.read(buffer,size);
              
              //... and converted back to a secondo-object
              SmiRecordFile recF(false,0);
              SmiRecord rec;
              SmiRecordId recID;
             Cmd_Mutex.acquire(); 
              recF.Open("rec");
              recF.AppendRecord(recID,rec);
              rec.Write(buffer,size,0);
              
              size_t s = 0;
              am->OpenObj(algID,typID,rec,s,m_type,
                          (*(m_cmd -> getElements()))[arg2]);
              
              recF.DeleteRecord(recID);
              recF.Close();
              Cmd_Mutex.release();
              delete [] buffer;
              
              getline(cbsock,line);
              
              DServer::Debug("DSR-READ4", line);
              int flobs=0;
              
              //The threads must not write a flob on the same time
              Flob_Mutex.acquire();
              
              while(line=="<FLOB>")
                { 
                  getline(cbsock,line);
                  DServer::Debug("DSR-READ5", line);
                  if(line!="<SIZE>")
                    errorText = (string)"Unexpected Response from" 
                      + " worker (<SIZE> expected)!";
                  
                  //Size of the flob is received
                  getline(cbsock,line);
                  DServer::Debug("DSR-READ6", line);
                  SmiSize si = atoi(line.data());
                  getline(cbsock,line);
                  
                  DServer::Debug("DSR-READ7", line);
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
                    ((am->Cast(algID,typID))
                     ((*(m_cmd -> getElements()))[arg2].addr));
                  
                  //Flob data is written
                  Flob*  f = a->GetFLOB(flobs);
                  f->write(buf,si,0);
                  
                  
                  delete buf;
                  
                  getline(cbsock,line);
                  DServer::Debug("DSR-READ8", line);
                  if(line!="</FLOB>") 
                    errorText = (string)"Unexpected Response from " 
                      + "worker (</SIZE> expected)!";
                  
                  getline(cbsock,line);
                  DServer::Debug("DSR-READ9", line);
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
              DServer::Debug("DSR-READ10", line);
              if(line.find("error") != string::npos) 
                errorText = "Worker reports error on sending an element!";
            }
          while(line.find("</SecondoResponse>") == string::npos);
          
        }
   
   }
   if(m_cmd -> getCmdType() == DS_CMD_DELETE)
   {
      if(rel_open) return;

      while(!m_cmd -> getDArrayIndex() ->empty())
      {
         arg2 = m_cmd -> getDArrayIndex() ->front();
         m_cmd -> getDArrayIndex() ->pop_front();
         
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
        
   if(m_cmd -> getCmdType() == DS_CMD_COPY)
   {
      if(rel_open) return;
      string line;
      iostream& iosock = server->GetSocketStream();

      string to = ((string*)((*(m_cmd -> getElements()))[0].addr))->data();
               
      while(!m_cmd -> getDArrayIndex() -> empty())
      {
         arg2 = m_cmd -> getDArrayIndex() ->front();
         m_cmd -> getDArrayIndex() ->pop_front();
         
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
        
   if(m_cmd -> getCmdType() == DS_CMD_EXEC)
   {
      
      if(rel_open) return;
      string line;
      iostream& iosock = server->GetSocketStream();

      string com = ((string*)((*(m_cmd -> getElements()))[0].addr))->data();
      string name = ((string*)((*(m_cmd -> getElements()))[1].addr))->data();
      vector<string> froms = m_cmd -> getFromNames();
      
      while(!m_cmd -> getDArrayIndex() ->empty())
      {
         arg2 = m_cmd -> getDArrayIndex() ->front();
         m_cmd -> getDArrayIndex() ->pop_front();
         
         string com_a = com;
         for (int i = froms.size(); i-- > 0;)
           {
             // setup replace string
             string rpl = "!";
             for (int k =0; k < i; ++k)
               rpl += "!";

             com_a = stringutils::replaceAll(com_a, rpl,
                                             "r" +
                                             froms[i] + toString_d(arg2));
           }
         
         //A command is executed on the worker
         string cmd;
         cmd = "(let r" + name + toString_d(arg2) + " = " + com_a + ")";
         
         
         DServer::Debug("DSR-EXEC", cmd);

         iosock << "<Secondo>" << endl << "0" << endl 
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
     
   
   if(m_cmd -> getCmdType() == DS_CMD_OPEN_WRITE_REL)
   {
     assert(server != 0);
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG
     cout << this << " DS_CMD_OPEN_WRITE_REL - start" << endl;
#endif
      if(rel_open) 
      {
        //cout << this 
        // << " DS_CMD_OPEN_WRITE_REL - rel is open ! - done" << endl;
        return;
      }

      //Initializes the writing of a tuple-stream, 
      //the d_receive_rel operator is started on the remote worker
             
      string line;
      iostream& iosock = server->GetSocketStream();
              
      int arg2 = m_cmd -> getDArrayIndex() ->front();
      
      string port =toString_d((1800+arg2)); 
      string com = "let r" + name + toString_d(arg2) + 
                     " = " + "d_receive_rel(" + HostIP_ + ",p" + port + ")";
          
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
      cout << this << " OR Send IO:" << "<Secondo>" << endl 
           << "1" << endl << "delete r" 
           << name << toString_d(arg2)  << endl 
           << "</Secondo>" << endl;
#endif     
      //The element is deleted on the worker, if already exists
      iosock << "<Secondo>" << endl << "1" << endl << "delete r" 
               << name << toString_d(arg2)  << endl << "</Secondo>" 
               << endl;
                
      do
      {
        getline(iosock,line);

#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
        cout << this << " OR Got IO:" << line << endl;
#endif
      }
      while(line.find("</SecondoResponse") == string::npos);
            
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
        cout << this << " OR Send IO:" << "<Secondo>" << endl << "1" << endl 
                  << com << endl << "</Secondo>" << endl;
#endif
      //The d_receive_rel operator is invoked
      iosock << "<Secondo>" << endl << "1" << endl 
                  << com << endl << "</Secondo>" << endl;
                
      //The callback connection is opened          
      Socket* gate = Socket::CreateGlobal( HostIP, port);

      cbworker = gate->Accept();

      //Relation type is sent to type-mapping-fct of the d_receive_rel operator
      iostream& cbsock1 = cbworker->GetSocketStream();
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
        cout << this << " OR Send CB1:" 
             << "<TYPE>" << endl << m_typeStr 
             << endl << "</TYPE>" << endl;
#endif
      cbsock1 << "<TYPE>" << endl << m_typeStr
              << endl << "</TYPE>" << endl;
                        
                                        
      getline(cbsock1,line);
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
        cout << this << " OR Got CB1:" << line << endl;
#endif
      if(line!="<CLOSE>")
         errorText = "Unexpected Response from worker (<Close> expected)!";
      
      cbworker->Close();

      delete cbworker;
                        
      //The callback connection from the value-mapping is opened and stored
      cbworker = gate->Accept();
      iostream& cbsock2 = cbworker->GetSocketStream();
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
        cout << this << " OR Send CB2:" 
             << "<TYPE>" << endl << m_typeStr
             << endl << "</TYPE>" << endl;
#endif
      cbsock2 << "<TYPE>" << endl << m_typeStr
                     << endl << "</TYPE>" << endl;
                
      gate->Close();
      delete gate;
      gate=0;
                        
      rel_open = true;

#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
      cout << this << " DS_CMD_OPEN_WRITE_REL - done" << endl;
#endif
               
   }
          
   if(m_cmd -> getCmdType() == DS_CMD_WRITE_REL)
   {
#ifdef DS_CMD_WRITE_REL_DEBUG 
     cout << this << " DS_CMD_WRITE_REL" << endl;
#endif
      //Writes a single tuple to an open tuple stream to the worker
      if(!rel_open) return;
               
      string line;
               
      Tuple *tpl = (Tuple*)(*(m_cmd -> getElements()))[0].addr;
      
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
#ifdef DS_CMD_WRITE_REL_DEBUG 
        cout << this << " WR Send CB:" << "<TUPLE>" << endl << toString_d(size) 
                  << endl << "</TUPLE>" << endl;
#endif
      cbsock << "<TUPLE>" << endl << toString_d(size) 
                  << endl << "</TUPLE>" << endl;
      
      getline(cbsock,line); 
#ifdef DS_CMD_WRITE_REL_DEBUG 
        cout << this << " WR Got CB:" << line << endl;
#endif
      if(line!= "<OK>") 
         errorText = "Worker unable to receive tuple!";
               
               
      getline(cbsock,line); 
#ifdef DS_CMD_WRITE_REL_DEBUG 
        cout << this << " WR Got CB:" << line << endl;
#endif
      if(atoi(line.data()) != num_blocks) 
         errorText = "Worker calculated wrong number of blocks!";
      
      getline(cbsock,line);
#ifdef DS_CMD_WRITE_REL_DEBUG 
        cout << this << " WR Got CB:" << line << endl;
#endif
      //Send the tuple data to the worker
      for(int i = 0; i<num_blocks;i++)
         cbsock.write(buffer+i*1024,1024);
       
      delete buffer;
               
      tpl->DeleteIfAllowed();
          
   }
          
   if(m_cmd -> getCmdType() == DS_CMD_CLOSE_WRITE_REL)
   {
#ifdef DS_CMD_CLOSE_WRITE_REL_DEBUG
      cout << this << " CR Start: DS_CMD_CLOSE_WRITE_REL" << endl;
#endif
      //Closes an open tuple stream to the worker
      if(!rel_open) return;
      
      string line;

      iostream& cbsock = cbworker->GetSocketStream();
      iostream& iosock = server->GetSocketStream(); 
               
      //Sends the close signal and receive <FINISH>
#ifdef DS_CMD_CLOSE_WRITE_REL_DEBUG
      cout << this << " CR Send CB: <CLOSE>" << endl;
#endif

      cbsock << "<CLOSE>" << endl;
      getline(cbsock,line);
#ifdef DS_CMD_CLOSE_WRITE_REL_DEBUG
      cout << this << " CR Got CB: " << line << endl;
      int cnt = 0;
#endif
      if(line != "<FINISH>")
         errorText = "Unexpected Response from worker! (<FINISH> expected)";
      
      cbworker->Close(); delete cbworker; cbworker = 0;
      do
      {
         getline(iosock,line);
#ifdef DS_CMD_CLOSE_WRITE_REL_DEBUG
         cout << this << " CR Got IO: " << line << endl;
         assert(cnt++ < 10);
#endif
         if(line.find("errror") != string::npos)
           errorText = "Worker reports error on closing relation!";
      }
      while(line.find("</SecondoResponse") == string::npos);

      rel_open = false;
   }
   

   if(m_cmd -> getCmdType() == DS_CMD_READ_REL)
   {
      //Reads an entire relation from the worker
      int algID,typID;
      extractIds(m_type,algID,typID);

      Rel2_Mutex.acquire(); 
      TupleType* tt = new TupleType(nl->Second(m_type));
      Rel2_Mutex.release(); 

      while(!m_cmd -> getDArrayIndex() ->empty())
      {
       
         arg2 = m_cmd -> getDArrayIndex() ->front();
         m_cmd -> getDArrayIndex() ->pop_front();
         
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

         GenericRelation* rel = 
           (Relation*)(*(m_cmd -> getElements()))[arg2].addr;

                
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
              {
                cbsock.read(buffer+i*1024,1024);
              }
            Tuple* t = new Tuple(tt);
                     
            //transform to tuple and append to relation
            t->ReadFromBin(buffer);

            Rel1_Mutex.acquire(); 
            rel->AppendTuple(t);
            Rel1_Mutex.release(); 

            t->DeleteIfAllowed();
              

            delete buffer;
                     
            getline(cbsock,line);
         }
                
         if(line != "<CLOSE>") 
            errorText = (string)"Unexpected Response from worker! " 
                              + "(<CLOSE> or <TUPLE> expected)";
                

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

       
      delete tt;   
          
   }

   delete m_cmd;

   m_cmd = NULL;

   status = 0;   

   //cout << this << " DS - done" << endl;       
   return;
}

/*

1.5 Multiply

Clones the DServer object
Makes more connections to the remote system available

*/

bool DServer::Multiply(int count)
{
  if(getNumChilds() > 0) 
      return false;

  if(count < 1) return true;

  m_numChilds = count - 1;
   for(int i = 0;i<m_numChilds;i++)
   {
     DServer* ds =  new DServer(host,port,name,m_type);
     m_childs.push_back( ds );
     try
        {
          if (!(ds -> connectToWorker()))
            return false;
              
        }
      catch(const exception &e)
        {
          cout << "Error starting DServer on " 
               << host << ":" << port << endl;
          return false;
        }
   }

   return true;
     
}

/*

1.6 DestroyChilds

Eliminates all the cloned DServer-Objects

*/

void DServer::DestroyChilds()
{
  for(int i = 0;i<getNumChilds(); i++)
   {
      m_childs[i]->Terminate();
      delete m_childs[i];
   }
  m_numChilds = 0;
  m_childs.clear();
}
/*

1.7 Destructor

delete pipe to SecondoMonitor, if it exists

*/

DServer::~DServer()
{
 
}

bool
DServer::checkServer() const
{
  if (server == 0)
    return false;

  if(!(server->IsOk()))
    return false;

  if (getNumChilds() != m_childs.size())
    return false;

  return true;
}
/*

2 Implementation of the class DServerManager

2.1 Constructor

Creates a new DServerManager specified by a serverlist
All DServer-Obects are created

*/

DServerManager::DServerManager(ListExpr serverlist_n, 
                               string name_n, 
                               ListExpr inType,
                               int sizeofarray)
{
   cout << "Connecting to Workers..." << endl;
   
   array_size = sizeofarray;
   name = name_n;
   errorText = "OK";
   size = nl->ListLength(serverlist_n);
   
   if(size==-1) 
      size=1;
                
   ListExpr elem = nl->First(serverlist_n);
   serverlist_n = nl->Rest(serverlist_n);
        
   try
     {
       ZThread::ThreadedExecutor exec;
       //create DServer-objects
       for(int i = 0; i<size; i++)
         {
           DServerCreator* c = 
             new DServerCreator( nl->StringValue(nl->First(elem)),
                                 nl->IntValue(nl->Second(elem)), 
                                 name,
                                 inType);

           m_serverlist.push_back( c -> createServer() );

           exec.execute(c);
      
           if(i < size-1)
             {
               elem = nl->First(serverlist_n);
               serverlist_n = nl->Rest(serverlist_n);
             }
   
         }

       exec.wait();
     } 
   catch(ZThread::Synchronization_Exception& e) 
     {
       errorText = string("Could not create DServers!\nError:") + 
         string(e.what());
       cerr << e.what() << endl;
       return;
     }

   for(int i = 0; i< size; i++)
      errorText = m_serverlist[i]->getErrorText();

   
   for (int id = 0; id < size; ++id)
     {
       list<int> insertList;
       for(int i = id; i<array_size; i+=size)
         insertList.push_front(i);
       m_idIndexMap[id] = insertList;
     }
}

/*

2.2 Destructor

All DServer-Objects controlled by the DServerManager are destoyed.

*/

DServerManager::~DServerManager()
{
   for(int i = 0; i<size; i++)
   {
      if(m_serverlist[i] != 0) 
         m_serverlist[i]->Terminate();
      delete m_serverlist[i];
   }
        
   m_serverlist.clear();
   m_idIndexMap.clear();
}

/*
2.6 checkServers

returns false, if server were not created correctly

*/ 
bool 
DServerManager::checkServers() const
{ 
  if (m_serverlist.size() != size)
    return false;

  for(int i = 0; i<size; i++)
    {
      if (!m_serverlist[i]->checkServer())
        {
          return false;
        }
    }
  return true;
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

      GenericRelation* rel = (Relation*)(*m_elements)[index].addr;
      GenericRelationIterator* iter = rel->MakeScan();
      
      Tuple* t;
     
     
      vector<Word> word(1);

      t = iter->GetNextTuple();
     
      //open tuple stream to the worker
      word[0].addr = t;
      worker->setCmd(DServer::DS_CMD_OPEN_WRITE_REL,l,&word);
      worker->run();
     
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

6 Class DServerCreator

*/

DServerCreator::DServerCreator
(string h, int p, string n, ListExpr t)
{
  /*
    string s_type = nl->ToString(t);
    nl->ReadFromString(s_type,m_type);
  */
  assert(!(nl -> ToString(t).empty()));
  m_type = t;
  
  host = h; port = p; name = n;
}

DServer*
DServerCreator::createServer()
{
  m_server = new DServer(host,port,name,m_type);
  return m_server;
}


void DServerCreator::run()
{
  m_server -> connectToWorker();
}

                  
ostream& operator << (ostream &out, DServer::RemoteCommand& rc)
{
  out << "RC:";
  switch (rc.getCmdType())
    { 
    case DServer::DS_CMD_NONE:      // undefined
      out << " NONE";
      break;
    case DServer::DS_CMD_WRITE:     // writes an element to the worker
      out << " WRITE";
      break;
    case DServer::DS_CMD_READ:      // reads an element from the worker
      out << " READ";
      break;
    case DServer::DS_CMD_DELETE:    // deletes an element on the worker
      out << " DELETE";
      break;
    case DServer::DS_CMD_COPY:      // copies an element on the worker
      out << " COPY";
      break;
    case DServer::DS_CMD_EXEC:      // exectues a command on each 
                                    // element on the worker
      out << " EXECUTE";
      break;
    case DServer::DS_CMD_OPEN_WRITE_REL: // opens a relation on 
                                         // the worker to add elements
      out << " OPEN WRITE RELATION";
      break;
    case DServer::DS_CMD_WRITE_REL: // writes a singel tuple to a 
                                    // relation on the worker
      out << " WRTIE RELATION";
      break;
    case DServer::DS_CMD_CLOSE_WRITE_REL: // closes a relation on the worker
      out << " CLOSE WRITE RELATION";
      break;
    case DServer::DS_CMD_READ_REL:  // reads a tuple from a relation 
                                    // on the worker
      out << " READ RELATION";
      break;
    default:
      out << " ERROR (undefined)";
      break;
    }
  out <<"  L:";
  if ( rc.getDArrayIndex() != NULL)
    {
      for (list<int>::const_iterator idx = rc.getDArrayIndex() -> begin();
         idx != rc.getDArrayIndex() -> end(); ++idx)

      out << *idx << " ";
    }
  else
    cout << "NULL";

  out << " W:";
  if (rc.getElements() != NULL)
    cout << rc.getElements() -> size();
  else
    cout << "NULL";

  return out;
}

