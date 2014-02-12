/*
----
This file is part of SECONDO.

Copyright (C) 2014, University in Hagen,
Faculty of Mathematics and Computer Science,
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
//[ue] [\"u]
//[ae] [\"a]
//[_] [\_]
//[TOC] [\tableofcontents]

[1] LoadBalancer

*/

/*

[TOC]

1 Overview

This is a TCP load balancer. The load balancer provides different
sheduling stategies:

rr = Round robin
trr = Thraded round robin
rtrr = Reliable thraded round robin


2 Defines, includes, and constants

*/

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

using namespace std;


/* 
3 Class - Generic DataScheduler, sendData must been overwriten in subclasses

*/
class DataSheduler {
 
public:
    virtual void sendData(string data) { }
    
};

/* 
4 Class for Helper functions

*/
class SocketHelper {
  
public:
  static void setSocketToBlockingMode(int socketfd) {
    // Set socket to blocking mode
    #ifdef WIN32
      unsigned long blocking = 1;
      ioctlsocket(socketfd, FIONBIO, &blocking) == 0);
    #else
      int flags = fcntl(socketfd, F_GETFL, 0);
      flags = (flags&~O_NONBLOCK);
      fcntl(socketfd, F_SETFL, flags);
    #endif
  }
};

/* 
5 Load Balancer listener, opens a tcp port and read data

*/
class LoadBalancerListener {

   public:
      LoadBalancerListener(int myListenPort, DataSheduler* myDataSheduler) {
         listenPort = myListenPort;
         dataReceiver = myDataSheduler;
         listenfd = 0;
         connfd = 0;
      }
      
      virtual ~LoadBalancerListener() {
          close();
      }
      
      virtual void openSocket() {
         cout << "Opening server socket" << endl;
         
         listenfd = socket(AF_INET, SOCK_STREAM, 0);

         memset(&serv_addr, 0, sizeof(serv_addr));
         memset(&client_addr, 0, sizeof(client_addr));

         serv_addr.sin_family = AF_INET;
         serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
         serv_addr.sin_port = htons(listenPort); 
      
         // Bind our socket
         if ((bind(listenfd, (struct sockaddr *) 
             &serv_addr, sizeof(serv_addr)) ) < 0) {
           
            cerr << "Bind failed" << endl;
            return;
         }
      
         // Listen
         if(( listen(listenfd, 10)) < 0 ){
            cerr << " Listen failed " << endl;
            return;
         }
         
         unsigned int clientlen = sizeof(client_addr);
     
         // Accept connection
         if(! (connfd = accept(listenfd, 
            (struct sockaddr *) &client_addr, 
            &clientlen))) {
                 
            cerr << "accept failed" << endl;
            return;
         } 
         
	 // set blocking mode
         SocketHelper::setSocketToBlockingMode(connfd);
      }
      
   bool isSocketOpen() {
      return listenfd != 0;
   }
   
   // Close client socket and server socket
   void close() {
     if(listenfd != 0) {
       shutdown(listenfd, 2);
       listenfd = 0;
     }
     
     if(connfd != 0) {
       shutdown(connfd, 2);
       connfd = 0;
     }
   }
   
   void readData(string* result) {
     
      string::iterator pos;
    
      // Read data until we got a "\n"
      while((pos = find(buffer.begin(), buffer.end(), '\n')) == buffer.end()) {
      
       if(isSocketOpen()) {
          char buf[1024];
          memset(buf, 0, sizeof(buf));
          cout << "read()" << endl;
          size_t bytesRead = read(connfd, buf, sizeof(buf));
   
          // End of transmisson ?
          if((buffer.compare("\004") == 0) || (bytesRead <= 0)) {
            cout << "End of transmisson, close listen socket" << endl;
            close();
            *result = string("\004");
            return;
          }
   
          buffer += buf;
       }
    }
       
    // Split buffer on "\n"
    *result = string (buffer.begin(), pos + 1);
    buffer = string (pos + 1, buffer.end());
    
    cout << "Got: " << *result << endl;
  }

  // Server main method
  // Read a line and send it to the sheduler
  void run() {
    while(isSocketOpen()) {
      string line;
      readData(&line);
      dataReceiver->sendData(line);
    }
  }
  
      
   protected:
      int listenPort;                 // The port we listen to
      string buffer;                  // Buffer for IO handling

      int listenfd;                   // FD for server listen
      int connfd;                     // FD for client handling
      struct sockaddr_in serv_addr;   // Server address  
      struct sockaddr_in client_addr; // Client address
      DataSheduler* dataReceiver;     // Send all data to this instance
};

/* 
6 Class for target server, opens a tcp connection for writing

*/
class TargetServer {
  
public:
  TargetServer(string connString) {
    hostname = string("");
    port = 0;
    socketfd = 0;
    
    size_t pos = connString.find(":");
    
    if(pos == string::npos) {
      cerr << "Invalid connection url: " << connString;
      return;
    }
    
    hostname = connString.substr(0, pos);
    port = atoi(connString.substr(pos + 1, connString.size()).c_str());    
  }
  
  virtual ~TargetServer() {
    if(isSocketOpen()) {
      close();
    }
  }
  
  int getSocketFd() {
    return socketfd;
  }
  
  // Open tcp connection to target server
  bool open() {
    cout << "Open TCP connection to server: " << hostname  
         << " Port " << port << endl;

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(socketfd < 0) {
      cerr << "Error opening socket" << endl;
      socketfd = 0;
      return false;
    }
    
    server = gethostbyname(hostname.c_str());
    
    if(server == NULL) {
      cerr << "Error resolving hostname: " << hostname << endl;
      socketfd = 0;
      return false;
    }
   
    // Prepare server_addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    server_addr.sin_addr.s_addr = 
      ((struct in_addr *)server->h_addr_list[0])->s_addr;

    // Connect to target server
    if(connect(socketfd, (struct sockaddr*) 
      &server_addr, sizeof(struct sockaddr)) < 0) {

      cerr << "Error in connect() " << endl;
      socketfd = 0;
      return false;
    }
    
    // Send socket to blocking mode
    SocketHelper::setSocketToBlockingMode(socketfd);

    return true;
  }
  
  // Are we accepting new data?
  virtual bool isReady() {
    return isSocketOpen();
  }
  
  // Close TCP-Connection to target server
  void close() {
    cout << "Shutdown connection to server: " << hostname 
         << " Port " << port << endl;
    
    if(! isSocketOpen()) {
      return;
    }
    
    shutdown(socketfd, 2);
    socketfd = 0;
  }
  
  // sendData to socket
  // this method can be overwritten in
  // subclasses
  virtual void sendData(string data) {
    _sendData(data);
  }
    
  bool isSocketOpen() {
    return socketfd != 0;
  }
  
protected:
  
  void _sendData(string data) {
    if(isSocketOpen()) {
       write(socketfd, data.c_str(), strlen(data.c_str()));
    }
  }
  
  string hostname;                   // Hostname
  int port;                          // Port
  int socketfd;                      // Socket
  struct hostent *server;            // Server name
  struct sockaddr_in server_addr;    // Server addr
  
};

/* 
7 Threaded Target Server - Same function as TargetServer
  but with Multithreading support

*/
class ThreadedTargetServer : public TargetServer {

public:
  
  ThreadedTargetServer(string connString) : TargetServer(connString) {
    pthread_mutex_init(&queueMutex, NULL);
    pthread_cond_init(&queueCondition, NULL);
  }
  
  virtual ~ThreadedTargetServer() {
    while(! myQueue.empty() ) {
      string *line = myQueue.front();
      delete line;
      myQueue.pop();
    }
  }
   
  virtual void run() {
    
    while(true) {
      
      // If the queue is emptry, wait for
      // new data
      pthread_mutex_lock(&queueMutex);
      while(myQueue.empty()) {
        pthread_cond_wait(&queueCondition, &queueMutex);
      }
      pthread_mutex_unlock(&queueMutex);
      
      // Remove fist line from queue
      pthread_mutex_lock(&queueMutex);
      string* data = myQueue.front();
      myQueue.pop();
      pthread_mutex_unlock(&queueMutex);
      
      // End of Transmission? => Exit
      if(data -> compare("\004") == 0) {
         _sendData(*data);
         cout << "Got EOT, exiting thread" << endl;
         delete data;
         exitThread();
      }
      
      // Send Data to target server
      _sendData(*data);

      // Callback
      tupelSend();

      delete data;
    }
  }
  
  // Template method, can be used in subclasses
  virtual void tupelSend() {
  }
  
  // Exit thread
  void exitThread() {
    pthread_mutex_destroy(&queueMutex);
    pthread_cond_destroy(&queueCondition);
    pthread_exit(NULL);
  }
  
  // Insert data into queue
  // Called from sheduler
  virtual void sendData(string data) {
    pthread_mutex_lock(&queueMutex);
    bool wasEmpty = myQueue.empty();
    myQueue.push(new string(data));
    
    // Wakeup consumer
    if(wasEmpty) {
      pthread_cond_signal(&queueCondition);
    }
    
    pthread_mutex_unlock(&queueMutex);
  }
  
protected:
  queue<string*> myQueue;
  pthread_mutex_t queueMutex;
  pthread_cond_t queueCondition;
};

/* 
8 Reliable Threaded Target Server - Same function as ThreadedTargetServer
  but after n tupels a ACK-Char is expected

*/
class ReliableThreadedTargetServer : public ThreadedTargetServer {
  
public:
  ReliableThreadedTargetServer(string connString, int myAcknowledgeAfter) 
      : ThreadedTargetServer(connString) {
   
    acknowledgeAfter = myAcknowledgeAfter;
    sendTupel = 0;
  }
  
  // Are we accepting new data?
  virtual bool isReady() {
    return isSocketOpen() && (sendTupel < acknowledgeAfter);
  }
  
  // Wait for acknowledge after n tuples send
  virtual void tupelSend() {
    ++sendTupel;
    
    // Wait for acknowledge
    if(sendTupel >= acknowledgeAfter) {
      char buffer[256];
      size_t bytesRead = read(socketfd, buffer, sizeof(buffer));
      
      // Ack?
      if(buffer[0] == '\006') {
         sendTupel = 0;
      } else {
         cout << "Got something different back from targetServer" << endl;
      }
    
    }
  }
  
private:
  int acknowledgeAfter;
  int sendTupel;
};

/* 
9 Logging only data sheduler
  For testing purposes

*/
class LoggingDataSheduler : public DataSheduler {
public:
  virtual void sendData(string data) {
    cout << "DataSheduler: got " << data << endl;
  }
};

/* 
10 Round Robin Data Sheduler


*/
class RRDataSheduler : public DataSheduler {
 
public:
  
  RRDataSheduler(vector<TargetServer*>* myServerList) {
    serverList = myServerList;
    lastServer = 0;
  }
  
  virtual void sendData(string data) {
    
    // cout << "DataSheduler: got " << data << " to " << lastServer << endl;
    
    // End of Transmission?
    if(data.compare("\004") == 0) {
      cout << "Got EOT, send EOT to all Threads" << endl;
      
      // Send EOT to all Threads
      for(vector<TargetServer*>::iterator iter = serverList->begin(); 
          iter != serverList->end(); ++iter) {
   
         TargetServer* ts = *iter;
         ts -> sendData(data);
      }
      
      return;
    }
    
    // No target server known?
    if(serverList -> empty()) {
      cout << "Server list is empty, ignore data" << endl;
      return;
    }
    
    TargetServer* ts;
    int tryCount = 0;
    
    // Find next ready server
    do {
      ts = serverList->at(lastServer);
      
      ++lastServer;
      ++tryCount;
      
      // Switch back to first server
      if(lastServer >= serverList->size()) {
         lastServer = 0;
      }
      
      // We contacted every server two times
      // But no one was ready
      if(tryCount > 2 * serverList->size()) {
        cout << "Could not find a ready server, IGNORING DATA" << endl;
        return;
      }
      
    } while(! ts -> isReady() );
    
    ts -> sendData(data);
  }
  
private:
  vector<TargetServer*>* serverList;
  size_t lastServer;
};

/* 
11 Helper function for starting the load balancer listener


*/
void* startlbserver(void* ptr) {
   LoadBalancerListener* lb = (LoadBalancerListener*) ptr;
   lb -> openSocket();
   lb -> run(); // Blocks until all data are read
   pthread_exit(NULL);
}

/* 
12 Helper function for starting a threaded target server

*/
void* startThreadedTargerServer(void *ptr) {
  ThreadedTargetServer* tss = (ThreadedTargetServer*) ptr;
  tss -> run();
}

/* 
13 Print Help


*/
void printHelp(char* progName) {
  cerr << "Usage: " << progName << " <ListenPort> <Mode> <ServerList>" << endl;
  cerr << endl;
  cerr << "Where <Mode> is rr or fs-n" << endl;
  cerr << "rr = round robin" << endl;
  cerr << "trr = multi-thraded round robin" << endl;
  cerr << "rtrr-n = reliable multi-threaded rr" << endl:
  cerr << "         acknowledge every n lines" << endl;
  cerr << "e.g. rtrr-10 " << endl;
  cerr << endl;
  cerr << "Example: " << progName << " 10000 rr 192.168.1.1:10001 " 
       << "192.168.1.2:10001 192.168.1.3:10001" << endl;
  cerr << endl;
}

/* 
14 Destroy all Server in provieded serverList


*/
void destroyServerList(vector<TargetServer*>* serverList) {
  for(vector<TargetServer*>::iterator iter = serverList->begin(); 
      iter != serverList->end(); ++iter) {
    
    if(*iter != 0) {
      (*iter) -> close();
      delete *iter;
    }
    
  }
  
  serverList->clear();
}

/* 
15 Parse Server argument list


*/
void parseServerList(int argc, char* argv[], 
           vector<TargetServer*>* serverList) {
  
   string mode = string(argv[2]);

   // Process destination servers
   for(int i = 3; i < argc; i++) {
     cout << "Processing : " << argv[i] << endl;
     TargetServer* ts;
     
     if(mode.compare("rr") == 0) {
       ts = new TargetServer(argv[i]);
     } else if(mode.compare("trr") == 0) {
       ts = new ThreadedTargetServer(argv[i]);
     } else if(mode.compare(0, 5, "rtrr-") == 0) {
        int acknowledgeAfter = atoi((mode.substr(5, mode.length())).c_str());
        ts = new ReliableThreadedTargetServer(argv[i], acknowledgeAfter); 
     } else {
          printHelp(argv[0]);
         exit(EXIT_FAILURE);
     }
     
     // Open the TCP-Socket to target server
     bool result = ts -> open();
     
     if(result == false) {
       cout << "Unable to open connection to: " << argv[i] 
             << " ignoring" << endl;
     } else {
        serverList->push_back(ts);
     }
   }
}

/* 
17 Start a threaded loadbalancer

*/
void startThreadedServer(string &mode, vector<TargetServer*> &serverList, 
          char *argv[], int listenPort) {
  
  if((mode.compare("trr") == 0)) {
    cout << "Mode is threaded round robin" << endl;
  } else {
       
    if(mode.length() <= 5) {
       cout << "Missing acknowledge after value" << endl;
       printHelp(argv[0]);
       exit(EXIT_FAILURE);
    }
       
    cout << "Mode is reliable thraded round robin" << endl;
    int acknowledgeAfter = atoi((mode.substr(5, mode.length())).c_str());
    cout << "Acknowledge after: " << acknowledgeAfter << endl;
  }
     
  RRDataSheduler dataReceiver(&serverList);
  LoadBalancerListener lb(listenPort, &dataReceiver);  
  vector<pthread_t> threads;
     
  // Start target server threads
  for(vector<TargetServer*>::iterator it = serverList.begin(); 
      it != serverList.end(); ++it) {

     pthread_t targetThread;
     pthread_create(&targetThread, NULL, &startThreadedTargerServer, *it);
     threads.push_back(targetThread);
  }
     
  // Start listener thread
  pthread_t listenerThread;
  pthread_create(&listenerThread, NULL, &startlbserver, &lb);
  threads.push_back(listenerThread);
     
  // Wait for threads to finish
  while(! threads.empty() ) {
    pthread_t thread = threads.back();
    threads.pop_back();
    pthread_join(thread, NULL);
  }
  threads.clear();
}

/* 
17 Main method

*/
int main(int argc, char* argv[]) {

   if(argc < 3) {
      printHelp(argv[0]);
      return EXIT_FAILURE;
   }
      
   // Process parameter
   int listenPort = atoi(argv[1]);
   string mode = string(argv[2]);

   // Process Server list
   vector<TargetServer*> serverList;
   parseServerList(argc, argv, &serverList);

   cout << "Starting load balancer on port: " << listenPort << endl;
   
   if(mode.compare("rr") == 0) {
     cout << "Mode is round robin" << endl;
     
     RRDataSheduler dataReceiver(&serverList);
     LoadBalancerListener lb(listenPort, &dataReceiver);  
     lb.openSocket();
     lb.run();
   } else if ((mode.compare("trr") == 0) 
              || (mode.compare(0, 5, "rtrr-") == 0) ) {

     startThreadedServer(mode, serverList, argv, listenPort);

   } else {
      cerr << "Error: unknown distribution mode: " << mode << endl << endl;
      printHelp(argv[0]);
      destroyServerList(&serverList);
      return EXIT_FAILURE;
   }
   
   destroyServerList(&serverList);
   
   return EXIT_SUCCESS;
}

