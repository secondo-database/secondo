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

This is a TCP load balancer for csv data. The load balancer 
provides different scheduling stategies:

rr    = Round robin
trr   = Thraded round robin
lbtrr = Load based thraded round robin
qbts  = Queue based threaded scheduling

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

#include "timer.h"

using namespace std;

/*
2.1 Defines

*/

//#define LB_DEBUG
#define QUEUESIZE 100 

#define CMDLINE_PORT            1<<0
#define CMDLINE_MODE            1<<1
#define CMDLINE_SERVER          1<<2
#define CMDLINE_RELIABLE        1<<3

/*
2.2 Prototypes

*/

class TargetServer;

/*
2.3 Structs

*/
struct LBConfiguration {
  string mode;                       // Load balancing mode
  int listenPort;                    // Listen Port
  vector<TargetServer*> serverList;  // List with destination servers
  string programName;                // Name of this program
  bool reliable;                     // Reliable mode
  Timer timer;                       // Timer to measure the execution time
};

/* 
3 Class - Generic DataScheduler

method sendData must been overwriten in subclasses

*/
class DataScheduler {
 
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
      LoadBalancerListener(LBConfiguration &myConfiguration, 
                           DataScheduler* myDataScheduler) : 
      configuration(myConfiguration), dataReceiver(myDataScheduler), 
      listenfd(0), connfd(0) {
        
      }
      
      virtual ~LoadBalancerListener() {
          close();
      }
      
/*     
5.1 Open socket for receiving data

*/
      virtual bool openSocket() {
          cout << "[Info] Opening server socket" << endl;
          
          listenfd = socket(AF_INET, SOCK_STREAM, 0);

          memset(&serv_addr, 0, sizeof(serv_addr));
          memset(&client_addr, 0, sizeof(client_addr));

          serv_addr.sin_family = AF_INET;
          serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
          serv_addr.sin_port = htons(configuration.listenPort); 
      
          // Bind our socket
          if ((bind(listenfd, (struct sockaddr *) 
              &serv_addr, sizeof(serv_addr)) ) < 0) {
            
            cerr << "[Error] Bind failed" << endl;
            return false;
          }
      
          // Listen
          if(( listen(listenfd, 10)) < 0 ){
            cerr << "[Erorr] Listen failed " << endl;
            return false;
          }
          
          unsigned int clientlen = sizeof(client_addr);
      
          // Accept connection
          if(! (connfd = accept(listenfd, (struct sockaddr *) &client_addr, 
                &clientlen))) {
                  
            cerr << "[Error] Accept failed" << endl;
            return false;
          } 
          
          // set blocking mode
          SocketHelper::setSocketToBlockingMode(connfd);
          configuration.timer.start();

          cout << "[Info] Reciving data...." << endl;
          
          return true;
      }
       
/*
5.2 Is the socket open?

*/
      bool isSocketOpen() {
          return listenfd != 0;
      }
   
/*
5.3 Close client socket and server socket

*/
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

/* 
5.4 Read data from socket

*/
   void readData(string* result) {
     
      string::iterator pos;
    
      // Read data until we got a "\n"
      while((pos = find(buffer.begin(), buffer.end(), '\n')) == buffer.end()) {
      
       if(isSocketOpen()) {
          char buf[1024];
          memset(buf, 0, sizeof(buf));

#ifdef LB_DEBUG
          cout << "read()" << endl;
#endif

          size_t bytesRead = read(connfd, buf, sizeof(buf));
   
          // End of transmisson ?
          if((buffer.compare("\004") == 0) || (bytesRead <= 0)) {
            cout << "End of transmisson, close listen socket" << endl;
            close();
            *result = string("\004");
            return;
          }
          
          buffer += string(buf, bytesRead);
       }
    }
       
    // Split buffer on "\n"
    *result = string (buffer.begin(), pos + 1);
    buffer = string (pos + 1, buffer.end());

#ifdef LB_DEBUG
    cout << "Got: " << *result << endl;
#endif
  }

/*
5.5 Server main method
Read a line and send it to the scheduler

*/
  void run() {
    while(isSocketOpen()) {
      string line;
      readData(&line);
      dataReceiver->sendData(line);
    }
  }
  
      
   protected:
      LBConfiguration &configuration; // The loadbalancer configuration
      string buffer;                  // Buffer for IO handling

      int listenfd;                   // FD for server listen
      int connfd;                     // FD for client handling
      struct sockaddr_in serv_addr;   // Server address  
      struct sockaddr_in client_addr; // Client address
      DataScheduler* dataReceiver;     // Send all data to this instance
};

/* 
6 Class ~TargetServer~. This class opens a tcp
connection to server ~hostname~ on port ~port~. 

You can use the sendData method to send data
to the server.

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

/* 
6.1 Get the server socket

*/
  int getSocketFd() {
    return socketfd;
  }
  
/*
6.2 Open tcp connection to target server

*/
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
  
/*
6.3 Are we accepting new data?

*/
  virtual bool isReady() {
    return isSocketOpen();
  }
  
/*
6.4 Close TCP-Connection to target server

*/
  void close() {
    cout << "Shutdown connection to server: " << hostname 
         << " Port " << port << endl;
    
    if(! isSocketOpen()) {
      return;
    }
    
    shutdown(socketfd, 2);
    socketfd = 0;
  }
  
/*
6.5 sendData to socket
this method can be overwritten in subclasses

*/
  virtual void sendData(string data) {
    _sendData(data);
  }

/*
6.6 is the socket open?

*/
  bool isSocketOpen() {
    return socketfd != 0;
  }

/*
6.7 get our hostname

*/
  string getHostname() {
    return hostname;
  }
  
/*
6.8 get our port

*/  
  int getPort() {
    return port;
  }
  
protected:
  
/*
6.9 Send data to the socket

*/  
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
  
/*
7.1 The main loop of the thread. Read data from queue and 
dispatch the data to the socket

*/
  virtual void run() {
    
    while(true) {
      
      // If the queue is emptry, wait for
      // new data
      pthread_mutex_lock(&queueMutex);
      
      while(myQueue.empty()) {
        pthread_cond_wait(&queueCondition, &queueMutex);
      }
      
      // Remove fist line from queue
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
  
/* 
7.2 Template method, can be used in subclasses

*/
  virtual void tupelSend() {
  }
  
/*
7.3 Exit thread

*/
  void exitThread() {
    pthread_mutex_destroy(&queueMutex);
    pthread_cond_destroy(&queueCondition);
    pthread_exit(NULL);
  }

/*
7.4 Insert data into queue
This method is called from scheduler

*/
  virtual void sendData(string data) {
    pthread_mutex_lock(&queueMutex);
    bool wasEmpty = myQueue.empty();
    myQueue.push(new string(data));
    
    // Wakeup consumer
    if(wasEmpty) {
      pthread_cond_broadcast(&queueCondition);
    }
    
    pthread_mutex_unlock(&queueMutex);
  }
  
/*
7.5 Get the size of the queue

*/
  size_t getQueueSize() {
    size_t result;
    pthread_mutex_lock(&queueMutex);
    result = myQueue.size();
    pthread_mutex_unlock(&queueMutex);
    return result;
  }

/*
7.6 We are only accepting new data, when the socket is
open and the size of the queue is less then QUEUESIZE

*/
  virtual bool isReady() {
    
    
    // Wait for queue size to reduce
    while(getQueueSize() >= QUEUESIZE) {
      usleep(1000);
    }
    
    
    return isSocketOpen() && ( getQueueSize() < QUEUESIZE );
  }
  
protected:
  queue<string*> myQueue;
  pthread_mutex_t queueMutex;
  pthread_cond_t queueCondition;
};

/* 
8 Reliable Threaded Target Server: Same function as ThreadedTargetServer
  but this class waits after sending n tupels, for a ACK from destination

*/
class ReliableThreadedTargetServer : public ThreadedTargetServer {
  
public:
  ReliableThreadedTargetServer(string connString, int myAcknowledgeAfter) 
      : ThreadedTargetServer(connString) {
   
    acknowledgeAfter = myAcknowledgeAfter;
    sendTupel = 0;
  }
  
/*
8.1 Are we accepting new data?

*/
  virtual bool isReady() {
    return ThreadedTargetServer::isReady() && (sendTupel < acknowledgeAfter);
  }
  
/*
8.2 Wait for acknowledge after n tuples send

*/
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
9 Logging only data scheduler
  For testing purposes

*/
class LoggingDataScheduler : public DataScheduler {
public:
  virtual void sendData(string data) {
    cout << "DataScheduler: got " << data << endl;
  }
};

/* 
10.0 Round Robin Data Scheduler: Dispatch data to the
next server in round robin list. 

*/
class RRDataScheduler : public DataScheduler {
 
public:
  
  RRDataScheduler(LBConfiguration &myConfiguration) 
    : configuration(myConfiguration), lastServer(0), ignoredLines(0) {
      
    serverList = &(configuration.serverList);
  }
  
  ~RRDataScheduler() {
    cout << "Ignored lines: " << ignoredLines << endl;
  }

/*
10.1 Send data to the target server

*/
  virtual void sendData(string data) {
      
    TargetServer* ts = NULL;
    
    do {
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
      
      ts = getTargetServer();
      
      // No target server known?
      if(serverList -> empty() || ts == NULL) {
        
        if(configuration.reliable == false) {
          cout << "Could not find a ready server, IGNORING DATA:" << endl;
          ++ignoredLines;
          return;
        }
   
        usleep(1000);
      }
      
    } while(ts == NULL);
    
#ifdef LB_DEBUG
      cout << "DataScheduler: got " << data << " to " << ts -> getHostname() 
           << ":" << ts -> getPort() << endl;
 #endif
        
    ts -> sendData(data);
  }
  
protected:
  
  // Get the next server 
  virtual TargetServer* getTargetServer() {
    
    TargetServer* ts;
    int tryCount = 0;
    
    // Serverlist is emptry
    if(serverList->empty()) {
      return NULL;
    }
    
    // Find next ready server
    do {
      ts = serverList->at(lastServer);
          
      ++tryCount;
      lastServer = (lastServer + 1) % serverList->size();
      
      // We contacted every server two times
      // but no one was ready, break loop.
      if(tryCount > 2 * serverList->size()) {
        return NULL;
      }
      
    } while(! ts -> isReady() );
    
    return ts;
  }
  
  LBConfiguration &configuration;
  vector<TargetServer*>* serverList;
  size_t lastServer;
  size_t ignoredLines;
};


/*

10.1 Queue Based scheduler: Send data to the server with 
the shortest queue

*/
class QBDataScheduler : public RRDataScheduler {
  
public:
    QBDataScheduler(LBConfiguration &configuration) 
      : RRDataScheduler(configuration) {
    }

protected:

  // Get the next server 
  virtual TargetServer* getTargetServer() {
    
    ThreadedTargetServer* ts = NULL;
    
    // Dispatch data to the server with the shortest queue. 
    // Consider only servers with less then QUEUESIZE entries
    // to avoid a out of memory situation
    size_t queueSize = QUEUESIZE;
    
    // Search for the server with the smallest queue size
    for(vector<TargetServer*>::iterator iter = serverList->begin(); 
        iter != serverList->end(); ++iter) {
      
      TargetServer* server = *iter;
      ThreadedTargetServer* tserver = (ThreadedTargetServer*) server;
      size_t tServerQueueSize = tserver -> getQueueSize();
      if(tServerQueueSize < queueSize) {
        queueSize = tServerQueueSize;
        ts = tserver;
      }
    }
    
    return ts;
  }
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
void printHelpAndExit(string &progName) {
  cerr << "Usage: " << progName 
       << " -p <ListenPort> -m <Mode> -s <ServerList> -r {true|false}" << endl;
  cerr << endl;
  cerr << "Where <Mode> is rr, trr, lbtrr-n or qbts:" << endl;
  cerr << "rr      = round robin" << endl;
  cerr << "trr     = thraded round robin" << endl;
  cerr << "lbtrr-n = load based threaded rr" << endl;
  cerr << "            acknowledge every n lines" << endl;
  cerr << "            (e.g. lbtrr-10)" << endl;
  cerr << "qbts    = queue based threaded scheduling" << endl;
  cerr << endl;
  cerr << "-r = Reliable Data processing: " << endl;
  cerr << "     false: discard lines when all queues are full" << endl;
  cerr << "     true:  process every line. Slow down the reader if needed" 
       << endl;
  cerr << endl;
  cerr << "Example: " << progName << " -p 10000 -m rr -s 192.168.1.1:10001 " 
       << "-s 192.168.1.2:10001 -s 192.168.1.3:10001 -r true" << endl;
  cerr << endl;
  exit(EXIT_FAILURE);
}

/* 
14 Destroy all Server in provieded serverList

*/
void destroyServerList(LBConfiguration &configuration) {
  
  vector<TargetServer*>* serverList = &(configuration.serverList);
  
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
bool parseServerList(char* argument, LBConfiguration &configuration) {
  
     string mode = configuration.mode;
     TargetServer* ts;
     
     if(mode.compare("rr") == 0) {
       ts = new TargetServer(argument);
     } else if((mode.compare("trr") == 0) || (mode.compare("qbts") == 0)) {
       ts = new ThreadedTargetServer(argument);
     } else if(mode.compare(0, 6, "lbtrr-") == 0) {
        int acknowledgeAfter = atoi((mode.substr(6, mode.length())).c_str());
        ts = new ReliableThreadedTargetServer(argument, acknowledgeAfter); 
     } else {
          cout << "Unkown mode: " << mode << endl;
          return false;
     }
     
     // Open the TCP-Socket to target server
     bool result = ts -> open();
     
     if(result == false) {
       cout << "[Error] unable to open connection to: " << argument
             << " ignoring target server" << endl;
     } else {
        configuration.serverList.push_back(ts);
     }
   
   return true;
}

/* 
17 Start a threaded loadbalancer

*/
void startThreadedServer(LBConfiguration &configuration) {
  
  DataScheduler* dataScheduler;
  string mode = configuration.mode;
  vector<TargetServer*>* serverList = &(configuration.serverList);
  
  if((mode.compare("trr") == 0)) {
    cout << "Mode is threaded round robin" << endl;
    dataScheduler = new RRDataScheduler(configuration);
  } else if(mode.compare("qbts") == 0) {
     cout << "Mode is queue based threaded scheduling" << endl;
     dataScheduler = new QBDataScheduler(configuration);
  } else {
       
    if(mode.length() <= 5) {
       cout << "Missing acknowledge after value" << endl;
       printHelpAndExit(configuration.programName);
       exit(EXIT_FAILURE);
    }
    
    dataScheduler = new RRDataScheduler(configuration);   
    cout << "Mode is load based thraded round robin" << endl;
    int acknowledgeAfter = atoi((mode.substr(6, mode.length())).c_str());
    cout << "Acknowledge after: " << acknowledgeAfter << " lines" << endl;
  }
  
  LoadBalancerListener lb(configuration, dataScheduler);  
  vector<pthread_t> threads;
     
  // Start target server threads
  for(vector<TargetServer*>::iterator it = serverList->begin(); 
      it != serverList->end(); ++it) {

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
  delete dataScheduler;
  dataScheduler = NULL;
}

/*
18.0 Parse commandline

*/
void parseCommandline(int argc, char* argv[], 
                     LBConfiguration &configuration) {
   
   unsigned int flags = 0;
   int option = 0;
   
   while ((option = getopt(argc, argv,"p:m:s:r:")) != -1) {
     
     string optString = string(optarg);
     
     switch (option) {
      case 'p':
           configuration.listenPort = atoi(optarg);
           flags |= CMDLINE_PORT;
           break;
      case 'm':
           configuration.mode = optarg;
           flags |= CMDLINE_MODE;
           break;
      case 's':
           if(! parseServerList(optarg, configuration)) {
             printHelpAndExit(configuration.programName);
           }
           flags |= CMDLINE_SERVER;
           break;
      case 'r':      
           if( optString.compare("FALSE") == 0 || 
               optString.compare("false") == 0) {
             configuration.reliable = false;
             flags |= CMDLINE_RELIABLE;
           } 
           
           else if( optString.compare("TRUE") == 0 || 
             optString.compare("true") == 0) {
             configuration.reliable = true;
             flags |= CMDLINE_RELIABLE;
           }
           
           else {
             cerr << "[Error] Unkown parameter for reliable: " 
                  << optString << endl;
           }
           break;
      default:
        cerr << "[Error] Unkown option: " << (char) option << endl;
        printHelpAndExit(configuration.programName);
     }
   }
   
   unsigned int required_flags = CMDLINE_PORT | CMDLINE_MODE |
                                 CMDLINE_SERVER | CMDLINE_RELIABLE;
   
   if(required_flags != flags) {
      printHelpAndExit(configuration.programName);
   }
}

/* 
19 Main method

*/
int main(int argc, char* argv[]) {

   LBConfiguration configuration;
   configuration.programName = string(argv[0]);
   parseCommandline(argc, argv, configuration);

   cout << "[Info] Starting load balancer on port: " 
        << configuration.listenPort << endl;
   
   string mode = configuration.mode;
        
   if(mode.compare("rr") == 0) {
     
     cout << "[Info] Mode is: round robin" << endl;
     
     RRDataScheduler dataReceiver(configuration);
     LoadBalancerListener lb(configuration, &dataReceiver);  
     
     lb.openSocket();
     lb.run();
   } else if ((mode.compare("trr") == 0) 
              || (mode.compare(0, 6, "lbtrr-") == 0) 
              || (mode.compare("qbts") == 0)) {

     startThreadedServer(configuration);

   } else {
      cerr << "[Error] unknown distribution mode: " << mode << endl << endl;
      printHelpAndExit(configuration.programName);
      destroyServerList(configuration);
      return EXIT_FAILURE;
   }
   
   destroyServerList(configuration);
   
   cout << "Execution Time (ms): " << configuration.timer.getDiff() / 1000 
        << endl;
   
   return EXIT_SUCCESS;
}

