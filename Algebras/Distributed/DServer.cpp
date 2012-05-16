/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[&] [\&]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]
//[ast] [\ensuremath{\ast}]

*/

/*
[1] Class DServer Implementation

\begin{center}
April 2012 Thomas Achmann


November 2010 Tobias Timmerscheidt
\end{center}

[TOC]

0 Description

Implementation of the class ~DServer~



1 Preliminaries

1.1 Includes

*/
#include "DServer.h"

#include "SocketIO.h"
#include "zthread/Mutex.h"
#include "TupleFifoQueue.h"
#include "DBAccessGuard.h"
#include "ThreadedMemoryCntr.h"

/*
1.2 Debug Output

uncoment the following lines for debug output

*/

//#define DS_CMD_CLOSE_WRITE_REL_DEBUG 1
//#define DS_CMD_OPEN_WRITE_REL_DEBUG 1
//#define DS_CMD_WRITE_REL_DEBUG 1
//#define DS_CMD_READ_REL_DEBUG 1
//#define DS_CMD_READ_TB_REL_DEBUG 1
//#define DS_CMD_DELETE_DEBUG 1

/*
1.3 Global Definitions

1.3.1 Variables

*/
//IP-Adress of this system (Master)
string HostIP;
string HostIP_;

/*
1.3.2 Thread Mutex

*/
//Synchronisation of access to Flobs
ZThread::Mutex Flob_Mutex;
//Synchronisation of access to read non relations and write commands
ZThread::Mutex Cmd_Mutex;

/*
1.4 Extern Definitions

*/
extern void extractIds(const ListExpr,int&,int&);

/*
2 Implementation

2.1 Static Variables 

Initialisation

*/
Word DServer::ms_emptyWord;


/*
2.2 Constructor

  * const string[&] inHostName - host name, where the worker runs

  * int inPortNumber - port number, where the worker listens to

  * string inName - name of the local data at the worker 

  * ListExpr inType 

*/
DServer::DServer(const string& inHostName, int inPortNumber,
                 string inName, ListExpr inType)
  : m_host(inHostName)
  , m_port(inPortNumber)
  , name(inName)
  , m_cmd(NULL)
  , m_server(NULL)
  , m_cbworker(NULL)
  , m_numChilds(0)
  , m_rel_open(false)
  , m_shuffle_open(false)
  , m_error(false)
{
   m_type = inType;
   DBAccess::getInstance() -> NL_ToString(m_type, m_typeStr);
}
/*

2.3 Method ~bool connectToWorker~

connects to a SECONDO instance at a worker host

  * returns bool - true: success; 

*/
bool
DServer::connectToWorker()
{        
  //StopWatch watch;

  string line;
  m_server = Socket::Connect( getServerHostName(), getServerPortStr(), 
                              Socket::SockGlobalDomain,
                              5,
                              1);
      
  if(getServer()!=0 && getServer()->IsOk())
    {
      iostream& iosock = getServer()->GetSocketStream();
      
      if (!getServer() -> IsOk())
      {
        string err = "Faild to establish socket for host ";
        err += getServerHostName()+ ":";
        err +=  getServerPort();
        err += "!";
        setErrorText(err);
      }
      
      do
      {
        getline( iosock, line );
              
      } while (line.empty());
      
      if (!getServer() -> IsOk())
      {
        string err = "Faild to establish socket for connection to host ";
        err += getServerHostName() + ":";
        err += getServerPort() + "!";
        setErrorText(err);
      }
      
      if(line=="<SecondoOk/>")
      {
        iosock << "<Connect>" << endl
               << endl // user
               << endl // password 
               << "</Connect>" << endl;


        getline( iosock, line );

        if( line == "<SecondoIntro>")
          { 
            do
            {
              getline( iosock, line);
                                
            }  while(line != "</SecondoIntro>");

              
          }
        else 
          setErrorText("Unexpected response from worker (No <SecondoIntro/>)");
      }
      else 
        setErrorText("Unexpected response from worker (No <SecondoOk/>)");
    }
  else 
    setErrorText("Connection to the worker couldn't be established!");
  
  if (getServer() == 0)
    {
      // should never happen
      // Socket::Connect always returns a pointer!
      assert(0);
      return false;
    }
      
  if (!(getServer() -> IsOk()))
    { 
      cout << "Cannot Connect to Server:" 
           << getServerHostName() << ":" << getServerPortStr() << endl;
      cout << getServer() -> GetErrorText() << endl;

      delete m_server;
      m_server = 0;

      return false;
    } // if (!(server -> IsOk()))

   iostream& iosock = getServer()->GetSocketStream();
   
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
           setErrorText(string("Opening of database \"distributed\" ") +
                        "on worker failed!");
           return false;
         }
                        
      }
      while(line.find("</SecondoResponse>") == string::npos);
   }
   else 
     setErrorText("Unexpected response from worker (No <SecondoResponse>)");
        

   HostIP = getServer()->GetSocketAddress();
   HostIP_ = "h" + stringutils::replaceAll(HostIP,".","_");
   
   //cout << "Connection to Worker on " << host << " established." << endl;
   //cout << "ConnectTime:"  
   // << m_host <<":" << port << watch.diffTimes() << endl;
   return true;
}
/*
2.4 Method ~void Terminate~

stops the SECONDO instance on the worker

*/

void DServer::Terminate()
{
   if(getServer() != 0)
   {
     if (!(getServer()->IsOk()))
       {
         cout << "Error: Cannot close connection to " << m_host << "!" << endl;
         cout << getServer() -> GetErrorText() << endl;
       }
     else
       {
         iostream& iosock = getServer()->GetSocketStream();
         iosock << "<Disconnect/>" << endl;
         getServer()->Close();
       }
     delete m_server;
     m_server=0;
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
2.5 Method ~void setCmd~

creates a ~RemoteCommand~ obecjt and sets the parameters 
for a specific command

  * CmdType inCmdType - the type specifier of the command

  * const list[<]int[>][ast] inIndex - list of darray indexes to be worked on

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
2.6 Method ~const string[&] getMasterHostIP const~

  * returns const string[&] - the host TCP/IP adress of the master server
  
  * format: xxx.xxx.xxx.xxx

*/  
const string& DServer::getMasterHostIP() const { return HostIP; }

/*
2.7 Method ~const string[&] getMasterHostIP[_] const~

  * returns const string[&] - the host TCP/IP adress of the master server
  
  * format: hxxx[_]xxx[_]xxx[_]xxx

*/  
const string& DServer::getMasterHostIP_() const { return HostIP_; }

/*
2.8 Method ~void closeSavedWorkerCBConnection~

closes the saved callback communication socket

*/  
void DServer::closeSavedWorkerCBConnection()
{ 
  if (m_cbworker != NULL)
    {
      m_cbworker -> Close();
      delete m_cbworker;
    }
  m_cbworker = NULL;
}

/*
2.9 Method ~void run~

runs a command on a worker. Command and parameters are specified in
a ~RemoteCommand~ object.

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

  if ( getServer() == 0 || !(getServer() -> IsOk()))
    {
      delete m_cmd;
      m_cmd = NULL;
      cerr << "ERROR: Connection to server failed!" << endl;
      return;
    }

  if(m_cmd -> getCmdType() == DS_CMD_WRITE)
   {
     if(isRelOpen()) 
        return;
      
      int algID,typID;
      extractIds(m_type,algID,typID);
                
      TypeConstructor* tc = am->GetTC(algID,typID);
      
      while(!m_cmd -> getDArrayIndex() -> empty())
      {

         arg2 = m_cmd -> getDArrayIndex() ->front();
         m_cmd -> getDArrayIndex() ->pop_front();
        
         iostream& iosock = getServer()->GetSocketStream();
         string port =int2Str((1800+arg2)); 
         string line;

         //The receiveD-Operator on the worker is called
         string com = "let r" + name + int2Str(arg2) + 
                        " = " + "receiveD(" + HostIP_ + ",p" + port + ")";

         iosock << "<Secondo>" << endl << "1" << endl 
                  << com << endl << "</Secondo>" << endl;
                
         //Callback-connection request from the worker is received
         Socket* gate = Socket::CreateGlobal( HostIP, port);

         if (!(gate -> IsOk()))
           {
             setErrorText(string("Could not open connection to worker!\n") + 
                          gate -> GetErrorText());
             return;
           }

         Socket* worker = gate->Accept();

         if (worker == NULL)
           {
             setErrorText("Could not connect to worker!");
             return;
           }

         iostream& cbsock1 = worker->GetSocketStream();

         //The element-type is sent to the type-mapping-fcts of receiveD
         cbsock1 << "<TYPE>" << endl << m_typeStr
                     << endl << "</TYPE>" << endl;
                
         getline(cbsock1,line);

         if(line!="<CLOSE>") 
           setErrorText(string("Unexpected response from") +
                        " worker (No <CLOSE> after type transmission)!");
         
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
         if(tc->NumOfFLOBs() > 0 ) 
            a = static_cast<Attribute*>((am->Cast(algID,typID))
                              (((*(m_cmd -> getElements()))[arg2]).addr));
         
         //Flobs are sent to worker
         for(int i = 0; i < tc->NumOfFLOBs(); i++)
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
            
            delete [] buf;
         }
         cbsock << "<CLOSE>" << endl;
         
                
         getline(cbsock,line);

         if(line!="<FINISH>")
           {
             //cerr << "NO FINISH:" << line << endl;
             setErrorText(string("Unexpected response from worker") +
                          " (No <FINISH> tag after value transmission");
           }
                

         worker->Close();delete worker;worker=0;
         
         do
         {
            getline(iosock,line);
            if(line.find("error") != string::npos) 
              setErrorText("Worker reports an error on storing an element!");
            
         }
         while(line.find("</SecondoResponse") == string::npos);
 

      } // while (!m_cmd -> getDArrayIndex() -> empty())
                
   }
        
   else if(m_cmd -> getCmdType() == DS_CMD_READ)
   {
     if(isRelOpen()) return;

      int algID,typID;
      extractIds(m_type,algID,typID);
                               
      while(!m_cmd -> getDArrayIndex() -> empty())
        {
          arg2 = m_cmd -> getDArrayIndex() -> front();
          m_cmd -> getDArrayIndex() -> pop_front();
          string master_port =int2Str((1500+arg2));
          
          //The sendD-operator on the worker is started       
          iostream& iosock = getServer()->GetSocketStream();
          
          iosock << "<Secondo>" << endl << "1" << endl 
                 << "query sendD (" << HostIP_ << ",p" << master_port << ",r" 
                 << name << int2Str(arg2) << ")" <<  endl 
                 << "</Secondo>" << endl;
      
          
          //Callback-connection is received
          Socket* gate = Socket::CreateGlobal( HostIP, master_port);
          Socket* worker = gate->Accept();

          gate->Close();delete gate;gate=0;
          
          iostream& cbsock = worker->GetSocketStream();
          cbsock << "<TYPE>" << endl << m_typeStr
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
              
              int flobs=0;
              
              //The threads must not write a flob on the same time
              Flob_Mutex.acquire();
              
              while(line=="<FLOB>")
                { 
                  getline(cbsock,line);
                  if(line!="<SIZE>")
                    setErrorText(string("Unexpected Response from ") +
                                 "worker (<SIZE> expected)!");
                  
                  //Size of the flob is received
                  getline(cbsock,line);
                  SmiSize si = atoi(line.data());
                  getline(cbsock,line);
                  
                  if(line!="</SIZE>")
                    setErrorText(string("Unexpected Response from ") +
                                 "worker (</SIZE> expected)!");
                  
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
                  
                  
                  delete []  buf;
                  
                  getline(cbsock,line);
                  if(line!="</FLOB>") 
                    setErrorText(string("Unexpected Response from ") +
                                 "worker (</SIZE> expected)!");
                  
                  getline(cbsock,line);
                  flobs++;
                }
              
              Flob_Mutex.release();
              
              if(line!="<CLOSE>")
                setErrorText(string("Unexpected Response from ") + 
                             "worker (<CLOSE> expected)!");
             
              cbsock << "<FINISH>" << endl;
              
              worker->Close(); delete worker; worker=0;
                 
            }
          else
            setErrorText("Unexpected response from worker (<SIZE> expected)!");
      
          do
            {
              getline(iosock,line);
              if(line.find("error") != string::npos) 
                setErrorText("Worker reports error on sending an element!");
            }
          while(line.find("</SecondoResponse>") == string::npos);
          
        } // while(!m_cmd -> getDArrayIndex() -> empty())
   
    }
   else if(m_cmd -> getCmdType() == DS_CMD_DELETE)
    {
#ifdef DS_CMD_DELETE_DEBUG
      cout << this << " DS_CMD_DELETE - start  (Rel:" 
           << isRelOpen() << ")" << endl;
#endif
      if(isRelOpen()) 
        {
#ifdef DS_CMD_DELETE_DEBUG
          cout << this << "   rel is open - bailing out" << endl;
#endif
          return;
        }

      while(!m_cmd -> getDArrayIndex() ->empty())
        {
          arg2 = m_cmd -> getDArrayIndex() ->front();
          m_cmd -> getDArrayIndex() ->pop_front();
#ifdef DS_CMD_DELETE_DEBUG
          cout << (unsigned long)this << "   Deleting index:" << arg2 << endl;
#endif
          //Element is deleted on the worker
          string line;
          iostream& iosock = getServer()->GetSocketStream();
          iosock << "<Secondo>" << endl << "1" << endl << "delete r" 
                 << name << int2Str(arg2) << endl << "</Secondo>" 
                 << endl;

          do
            {
              getline(iosock,line);
              if(line.find("error") != string::npos)
                setErrorText("Worker reports error on deleteing element!");
            }
          while(line.find("</SecondoResponse>") == string::npos);

      }
   
   }
   else if(m_cmd -> getCmdType() == DS_CMD_EXEC)
   {
      
     if(isRelOpen()) return;
      string line;
      iostream& iosock = getServer()->GetSocketStream();

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
                                             froms[i] + int2Str(arg2));
           }
         
         //A command is executed on the worker
         string cmd;
         cmd = "(let r" + name + int2Str(arg2) + " = " + com_a + ")";
         
         iosock << "<Secondo>" << endl << "0" << endl 
                     << cmd<< endl << "</Secondo>" << endl;

         //cout << "CMDEXE:" << "<Secondo>" << endl << "0" << endl 
         //    << cmd<< endl << "</Secondo>" << endl;
         
         do
         {
            getline(iosock,line);
            //cout << "GOT:" << line << endl;

            if(line.find("error") != string::npos)
              setErrorText("Worker reports error on executing operation!");
         }
         while(line.find("</SecondoResponse>") == string::npos);

      }
   }
     
   
   else if(m_cmd -> getCmdType() == DS_CMD_OPEN_WRITE_REL)
   {
     assert(getServer() != 0);
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG
     cout << (unsigned long)(this) << " DS_CMD_OPEN_WRITE_REL - start" << endl;
#endif
     if(isRelOpen()) 
      {
        //cout << (unsigned long)(this) 
        // << " DS_CMD_OPEN_WRITE_REL - rel is open ! - done" << endl;
        return;
      }

      //Initializes the writing of a tuple-stream, 
      //the d_receive_rel operator is started on the remote worker
             
      string line;
      iostream& iosock = getServer()->GetSocketStream();
      
      int arg2 = m_cmd -> getDArrayIndex() ->front();

      string index_str=((string*)((*(m_cmd -> getElements()))[0].addr))->data();
      string rec_type=((string*)((*(m_cmd -> getElements()))[1].addr))->data();
      
      string port =int2Str((1800+arg2)); 
      string com = "let r" + name + int2Str(arg2) + 
                     " = " + "d_receive_rel(" + HostIP_ + ",p" + port + ")";
      
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
        cout << (unsigned long)(this) 
             << " OR Send IO:" 
             << "<Secondo>" << endl << "1" << endl 
             << com << endl << "</Secondo>" << endl;
#endif
      //The d_receive_rel operator is invoked
      iosock << "<Secondo>" << endl << "1" << endl 
                  << com << endl << "</Secondo>" << endl;
                
      //The callback connection to the type mapping
      //function of d_receive_rel is opened          
      Socket* gate = Socket::CreateGlobal( HostIP, port);

      Socket* cbworkerTM = gate->Accept();

      //Relation type is sent to type-mapping-fct of 
      //the d_receive_rel operator
      iostream& cbsockTM = cbworkerTM -> GetSocketStream();
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
        cout << (unsigned long)(this) << " OR Send CB1:" 
             << "<TYPE>" << endl << m_typeStr 
             << endl << "</TYPE>" << endl;
#endif
      cbsockTM << "<TYPE>" << endl << m_typeStr
              << endl << "</TYPE>" << endl;
                        
                                        
      getline(cbsockTM,line);
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
        cout << (unsigned long)(this) << " OR Got CB1:" << line << endl;
#endif
      if(line!="<CLOSE>")
        setErrorText("Unexpected Response from worker (<Close> expected)!");
      
      cbworkerTM->Close();

      delete cbworkerTM;
                        
      //The callback connection to the value-mapping 
      //function of the d_receive_rel function is 
      //opened and stored
      Socket *cbworkerVM = gate->Accept();
      iostream& cbsockVM = cbworkerVM->GetSocketStream();

      // sending result tuple type
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
        cout << (unsigned long)(this) << " OR Send CB2:" 
             << "<TYPE>" << endl << m_typeStr
             << endl << "</TYPE>" << endl;
#endif
      cbsockVM << "<TYPE>" << endl << m_typeStr
                     << endl << "</TYPE>" << endl;

      getline(cbsockVM,line);
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
        cout << (unsigned long)(this) << " OR Got CB2:" << line << endl;
#endif
      if(line!="<OK/>")
        setErrorText("Unexpected Response from worker (<Close> expected)!");
      
      // sending input tuple type
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
        cout << (unsigned long)(this) << " OR Send CB2:" 
             << "<INTYPE>" << endl << rec_type
             << endl << "</INTYPE>" << endl;
#endif
      cbsockVM << "<INTYPE>" << endl << rec_type
                     << endl << "</INTYPE>" << endl;
      getline(cbsockVM,line);
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
        cout << (unsigned long)(this) << " OR Got CB2:" << line << endl;
#endif
      if(line!="<OK/>")
        setErrorText("Unexpected Response from worker (<Close> expected)!"); 

      // sending darray index position
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
        cout << (unsigned long)(this) << " OR Send CB2:" 
             << "<INDEX>" << endl << index_str
             << endl << "</INDEX>" << endl;
#endif
      cbsockVM << "<INDEX>" << endl << index_str
                     << endl << "</INDEX>" << endl;
      getline(cbsockVM,line);
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
        cout << (unsigned long)(this) << " OR Got CB2:" << line << endl;
#endif
      if(line!="<OK/>")
        setErrorText("Unexpected Response from worker (<Close> expected)!");
      
      gate->Close();
      delete gate;
      gate=0;
                        
      setRelOpen();
      
      // ATTNENTION: CBWORKER IS KEPT ALIVE for  DS_CMD_WRITE_REL
      // only the gate is closed!
      saveWorkerCallBackConnection(cbworkerVM);
      cbworkerVM = NULL;
#ifdef DS_CMD_OPEN_WRITE_REL_DEBUG 
      cout << (unsigned long)(this) << " DS_CMD_OPEN_WRITE_REL - done" << endl;
#endif   
   }
          
   else if(m_cmd -> getCmdType() == DS_CMD_WRITE_REL)
   {
#ifdef DS_CMD_WRITE_REL_DEBUG 
     cout << (unsigned long)(this) << " DS_CMD_WRITE_REL" << endl;
#endif
      //Writes a single tuple to an open tuple stream to the worker
     if(!isRelOpen()) 
        {
#ifdef DS_CMD_WRITE_REL_DEBUG 
          cout << (unsigned long)(this) << " REL NOT OPEN" << endl;
#endif
          return;
        }

      string line;

      Tuple *tpl = (Tuple*)(*(m_cmd -> getElements()))[0].addr;
      
      //Get the tuple size
      size_t cS,eS,fS,size;
      size = tpl->GetBlockSize(cS,eS,fS);
      
      string str_size = int2Str(size);

      int num_blocks = (size / 1024) + 1;
    
      iostream& cbsock = getSavedWorkerCBConnection() -> GetSocketStream();
               
      //Send the size of the tuple to the worker
#ifdef DS_CMD_WRITE_REL_DEBUG 
        cout << (unsigned long)(this) 
             << " WR Send CB:" << "<TUPLE>" << endl << str_size 
             << endl << "</TUPLE>" << endl;
#endif
      cbsock << "<TUPLE>" << endl << str_size
                  << endl << "</TUPLE>" << endl;
      
      getline(cbsock,line); 
#ifdef DS_CMD_WRITE_REL_DEBUG
        cout << (unsigned long)(this) 
             << " WR Got CB:" << line << endl;
#endif
      if(line!= "<OK>") 
        {
          setErrorText("Worker unable to receive tuple!");
#ifdef DS_CMD_WRITE_REL_DEBUG1
        cout << (unsigned long)(this) 
             << " WR Got CB (EXPETING <OK>!):" << line << endl;
#endif
        }
               
               
      getline(cbsock,line); 
#ifdef DS_CMD_WRITE_REL_DEBUG
        cout << (unsigned long)(this) << " WR Got CBX:" << line << endl;
#endif
      if(atoi(line.data()) != num_blocks) 
        {
          setErrorText("Worker calculated wrong number of blocks!");
#ifdef DS_CMD_WRITE_REL_DEBUG1
          cout << (unsigned long)(this) 
               << " WR Got CB (EXPETING INT!):" 
               << line << endl;
#endif
        }
      getline(cbsock,line);
#ifdef DS_CMD_WRITE_REL_DEBUG
      cout << (unsigned long)(this) << " WR Got CB:" << line << endl;
#endif
      if(line!= "</OK>") 
        {
          setErrorText("Worker unable to receive tuple!");
#ifdef DS_CMD_WRITE_REL_DEBUG1
          cout << (unsigned long)(this) 
               << " WR Got CB (EXPETING </OK>!):" << line << endl;
#endif
        }
      char* buffer = new char[num_blocks*1024];
      memset(buffer,0,1024*num_blocks);
                
      //Get the binary data of the tuple
      DBAccess::getInstance() -> T_WriteToBin(tpl, buffer, cS, eS, fS);
        
      //Send the tuple data to the worker
      for(int i = 0; i<num_blocks;i++)
        getSavedWorkerCBConnection() -> Write(buffer+i*1024,1024);

      delete [] buffer;
      
   }
          
   else if(m_cmd -> getCmdType() == DS_CMD_CLOSE_WRITE_REL)
   {
#ifdef DS_CMD_CLOSE_WRITE_REL_DEBUG
      cout << (unsigned long)(this) 
           << " CR Start: DS_CMD_CLOSE_WRITE_REL" << endl;
#endif
      //Closes an open tuple stream to the worker
      if(!isRelOpen()) return;
      
      string line;

      iostream& cbsock = getSavedWorkerCBConnection() -> GetSocketStream();
      iostream& iosock = getServer()->GetSocketStream(); 
               
      //Sends the close signal and receive <FINISH>
#ifdef DS_CMD_CLOSE_WRITE_REL_DEBUG
      cout << (unsigned long)(this) << " CR Send CB: <CLOSE>" << endl;
#endif

      cbsock << "<CLOSE>" << endl;
      getline(cbsock,line);
#ifdef DS_CMD_CLOSE_WRITE_REL_DEBUG
      cout << (unsigned long)(this) << " CR Got CB: " << line << endl;
      int cnt = 0;
#endif
      if(line != "<FINISH>")
        {
          //cerr << "NO FINISH:" << line << endl;
          setErrorText(string("Unexpected Response from worker!") + 
                       "(<FINISH> tag expected)");
          delete m_cmd;
          m_cmd = NULL;
          return;
        }
      
      closeSavedWorkerCBConnection();
      
      do
      {
         getline(iosock,line);
#ifdef DS_CMD_CLOSE_WRITE_REL_DEBUG
         cout << (unsigned long)(this) << " CR Got IO: " << line << endl;
         assert(cnt++ < 10);
#endif
         if(line.find("errror") != string::npos)
           {
             setErrorText("Worker reports error on closing relation!");
             delete m_cmd;
             m_cmd = NULL;
             return;
           }
      }

      while(line.find("</SecondoResponse") == string::npos);

      setRelClose();
   }
   

   else if(m_cmd -> getCmdType() == DS_CMD_READ_REL)
     {
#ifdef DS_CMD_READ_REL_DEBUG
       cout << (unsigned long)(this) << " DS_CMD_READ_REL" << endl;
#endif
       //Reads an entire relation from the worker

       ListExpr tt_type = DBAccess::getInstance() -> NL_Second(m_type);

       TupleType *tt = 
         DBAccess::getInstance() -> TT_New(tt_type);

       while(!m_cmd -> getDArrayIndex() ->empty())
         {
           arg2 = m_cmd -> getDArrayIndex() ->front();
           m_cmd -> getDArrayIndex() ->pop_front();
         
           string line;        
           string port =int2Str((1300+arg2));
                
           //start execution of d_send_rel
           iostream& iosock = getServer()->GetSocketStream();
           iosock << "<Secondo>" << endl << "1" << endl 
                  << "query d_send_rel (" << HostIP_ << ",p" << port << ",r"
                  << name << int2Str(arg2) << ")" <<  endl 
                  << "</Secondo>" << endl;
                
           //Open the callback connection
           Socket* gate = Socket::CreateGlobal( HostIP, port);
           Socket* worker = gate->Accept();
           if (worker == NULL)
             {
               setErrorText("Cannot connect to worker at " + 
                            HostIP + ":" + port + 
                            " to submit send_rel command!");
             }
           else
             {
           iostream& cbsock = worker->GetSocketStream();

           GenericRelation* rel = 
             (Relation*)(*(m_cmd -> getElements()))[arg2].addr;

                
           //receive tuples
           getline(cbsock,line);
           if (line.empty())
             {
               setErrorText("ERROR: Unknown response from worker!");
               delete m_cmd;
               m_cmd = NULL;
               return;
             }
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
                     
               Tuple* t = //;new Tuple(tt);
                 DBAccess::getInstance() -> T_New(tt);
               

               //transform to tuple and append to relation
               DBAccess::getInstance() -> T_ReadFromBin(t, buffer);
               DBAccess::getInstance() -> REL_AppendTuple(rel, t);
               DBAccess::getInstance() -> T_DeleteIfAllowed(t);
            
               delete [] buffer;
                     
               getline(cbsock,line);
             }
                
           if(line != "<CLOSE>") 
             {
               setErrorText(string("Unexpected Response from worker! " ) + 
                            "(<CLOSE> or <TUPLE> expected)");
               
               delete m_cmd;
               m_cmd = NULL;
               return;
             }
           gate->Close(); delete gate; gate=0;
           worker->Close(); delete worker; worker=0;

           do
             {
               getline(iosock,line);
               assert(!line.empty());
               if(line.find("error") != string::npos)
                 {
                   setErrorText("worker reports error on sending relation!");
                   delete m_cmd;
                   m_cmd = NULL;
                   return;
                 }
             }
           while(line.find("</SecondoResponse>") == string::npos);

         }
         }
       DBAccess::getInstance() -> TT_DeleteIfAllowed(tt);
          
#ifdef DS_CMD_READ_REL_DEBUG
      cout << (unsigned long)(this) << " DS_CMD_READ_REL done" << endl;
#endif
   }

   else if(m_cmd -> getCmdType() == DS_CMD_READ_TB_REL)
     {
#ifdef DS_CMD_READ_TB_REL_DEBUG
       cout << (unsigned long)(this) << " DS_CMD_READ_TB_REL" << endl;
#endif
       //Reads an entire relation from the worker
       TFQ outQueue = (TFQ)(*(m_cmd -> getElements()))[0].addr;
       ThreadedMemoryCounter *memCntr = 
         (ThreadedMemoryCounter *)(*(m_cmd -> getElements()))[1].addr;

       ListExpr tt_type = DBAccess::getInstance() -> NL_Second(m_type);

       TupleType *tt = 
         DBAccess::getInstance() -> TT_New(tt_type);

#ifdef DS_CMD_READ_TB_REL_DEBUG
       if (m_cmd -> getDArrayIndex() ->empty()) 
         cout << (unsigned long)(this) 
              << " DS_CMD_READ_TB_REL - no indizes!"  << endl;
#endif
       while(!m_cmd -> getDArrayIndex() ->empty())
         {
           arg2 = m_cmd -> getDArrayIndex() ->front();
           m_cmd -> getDArrayIndex() ->pop_front();
         
           string line;        
           string port =int2Str((1300+arg2));
                
           //start execution of d_send_rel
           iostream& iosock = getServer()->GetSocketStream();
           iosock << "<Secondo>" << endl << "1" << endl 
                  << "query d_send_rel (" << HostIP_ << ",p" << port << ",r"
                  << name << int2Str(arg2) << ")" <<  endl 
                  << "</Secondo>" << endl;
                
           //Open the callback connection
           Socket* gate = Socket::CreateGlobal( HostIP, port);
           Socket* worker = gate->Accept();

           if (worker == NULL)
             {
               setErrorText("Cannot connect to worker at " + 
                            HostIP + ":" + port + 
                            " to submit send_rel command!");
             }
           else
             {
               iostream& cbsock = worker->GetSocketStream();
               
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
                   
                   Tuple* t = //new Tuple(tt);
                     DBAccess::getInstance() -> T_New(tt);
               
                   
                   //transform to tuple and append to relation
                   DBAccess::getInstance() -> T_ReadFromBin(t, buffer);
                   memCntr -> request(t -> GetSize());
                   outQueue -> put(t);
                   delete [] buffer;
                   
              

                   getline(cbsock,line);
                 } // while
                
               if(line != "<CLOSE>") 
                 setErrorText(string("Unexpected Response from worker! ") +  
                              "(<CLOSE> or <TUPLE> expected)");
               
               gate->Close(); delete gate; gate=0;
               worker->Close(); delete worker; worker=0;
               
               do
                 {
                   getline(iosock,line);
                   if(line.find("error") != string::npos)
                     setErrorText("worker reports error on sending relation!");
                 }
               while(line.find("</SecondoResponse>") == string::npos);
             } // else if 
         } // while(!m_cmd -> getDArrayIndex() ->empty())
         
       DBAccess::getInstance() -> TT_DeleteIfAllowed(tt);
         
#ifdef DS_CMD_READ_TB_REL_DEBUG
         cout << (unsigned long)(this) << " DS_CMD_READ_TB_REL done" << endl;
#endif
   }

 
   if (m_cmd != NULL)
     delete m_cmd;

   m_cmd = NULL;

   //cout << (unsigned long)(this) << " DS - done" << endl;       
   return;
   }    

/*
2.10 Method ~bool Multiply~

multiplys the SECONDO instances at the worker. This is used to 
store data of multiple darray indexes at one worker

  * int count - number of additional instances
 
  * returns bool - true: success

*/
bool DServer::Multiply(int count)
{
  if(getNumChilds() > 0) 
      return false;

  if(count < 1) 
    {
      m_numChilds = 0;
      m_childs.clear();
      return true;
    }

  m_numChilds = count - 1;

  //  cerr << "DServer::Multiply:" << m_numChilds << endl;
   for(int i = 0;i<m_numChilds;i++)
   {
     DServer* ds =  new DServer(getServerHostName(),
                                getServerPort(),
                                name,m_type);
     m_childs.push_back( ds );
     try
        {
          if (!(ds -> connectToWorker()))
            return false;
              
        }
      catch(const exception &e)
        {
          cout << "Error starting DServer on " 
               << getServerHostName() << ":" << getServerPortStr() << endl;
          return false;
        }
   }

   return true;
     
}
/*
2.11 Method ~void DestroyChilds~

stops workers created by the ~Multiply~ method

*/
void DServer::DestroyChilds()
{
  for(int i = 0;i<getNumChilds() && i < (int)m_childs.size(); i++)
   {
     if (m_childs[i] != NULL)
       {
         m_childs[i]->Terminate();
         delete m_childs[i];
         m_childs[i] = NULL;
       }
   }
  m_numChilds = 0;
  m_childs.clear();
}

/*
2.12 Method ~bool checkServer~

performs a check on the worker

  * bool writeError - writes an error to stderr
 
  * returns bool - true: success

*/
bool
DServer::checkServer(bool writeError) const
{
  if (m_server == 0)
    {
      if (writeError)
        cerr << "ERROR: Not connected to worker on " 
             << getServerHostName() << ":" << getServerPortStr() << endl;
      return false;
    }

  if(!(m_server->IsOk()))
    {
      if (writeError)
        cerr << "ERROR: Could not establish connection to worker on " 
             << getServerHostName() << ":" << getServerPortStr() << endl;
      return false;
    }

  if ((unsigned int)getNumChilds() != m_childs.size())
    {
      if (writeError)
        cerr << "ERROR: Workers are not setup correctly, restart cluster" 
             << endl;
      return false;
    }

  return true;
}
    
/*
2.13 Method ~void print~

*/  
void DServer::print() const
{
  cout << (unsigned long)(this) << " : " << " " 
       << getServerHostName() << " " << getServerPortStr() << endl;
}
       
/*
2.14 Operator ~ostream[&] [<][<]~

ostream operator for the internal class ~RemoteCommand~

  * ostream[&] - the output stream

  * RemoteCommand[&] - reference to the ~RemoteCommand~ object

  * returns ostream[&] - the output stream

*/         
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
      out << " WRITE RELATION";
      break;
    case DServer::DS_CMD_CLOSE_WRITE_REL: // closes a relation on the worker
      out << " CLOSE WRITE RELATION";
      break;
    case DServer::DS_CMD_READ_REL:  // reads a tuple from a relation 
                                    // on the worker and puts it into
                                    // a relation on the server
      out << " READ RELATION";
      break;
    case DServer::DS_CMD_READ_TB_REL:  // reads a tuple from a relation 
                                       // on the worker and puts it into 
                                       // a tuplebuffer on the host
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
    out << "NULL";

  out << " W:";
  if (rc.getElements() != NULL)
    out << rc.getElements() -> size();
  else
    out << "NULL";

  return out;
}

