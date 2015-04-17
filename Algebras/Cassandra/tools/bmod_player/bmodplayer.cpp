/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the Systems of the GNU General Public License as published by
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


1 Realtime player for BerlinMod GPS Data


1.1 Includes

*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "../timer.h"

/*
1.2 Defines

*/
#define CMDLINE_INPUTFILE        1<<0
#define CMDLINE_DESTHOST         1<<1
#define CMDLINE_DESTPORT         1<<2
#define CMDLINE_SIMULATION_MODE  1<<3

#define QUEUE_ELEMENTS 100
#define DELIMITER ","

#define SIMULATION_MODE_ADAPTIVE 1
#define SIMULATION_MODE_FIXED    2

using namespace std;

/*
1.3 Structs

*/
struct Configuration {
   string inputfile;
   string desthost;
   size_t destport;
   short simulationmode;
   timeval start;
};

struct Statistics {
   size_t read;
   size_t send;
   bool done;
};

struct InputData {
    size_t moid;
    size_t tripid;
    tm time_start;
    tm time_end;
    float x_start;
    float y_start;
    float x_end;
    float y_end;
};

struct Position {
   size_t moid;
   size_t tripid;
   tm time;
   float x;
   float y;
};

struct QueueSync {
   pthread_mutex_t queueMutex;
   pthread_cond_t queueCondition;
};

/*
2.0 Abstract Producer class - reads berlin mod csv data 

*/
class AbstractProducer {
public:

    AbstractProducer(Configuration *myConfiguration, 
        Statistics *myStatistics, QueueSync *myQueueSync) : 
        queueSync(myQueueSync) ,
        configuration(myConfiguration), statistics(myStatistics) {
   
    }
    
    virtual ~AbstractProducer() {

    }

    bool parseCSVDate(struct tm &tm, string date) {
      if (strptime(date.c_str(), "%Y-%m-%d %H:%M:%S.", &tm)) {
         return true;
      }
   
      if (strptime(date.c_str(), "%Y-%m-%d %H:%M", &tm)) {
         return true;
      }
   
      if (strptime(date.c_str(), "%Y-%m-%d %H", &tm)) {
         return true;
      }
   
      if (strptime(date.c_str(), "%Y-%m-%d", &tm)) {
         return true;
      }
   
      return false;
   }

   bool parseInputData() {
   
      if( access( configuration -> inputfile.c_str(), F_OK ) == -1 ) {
         cerr << "Unable to open Input file: " 
              << configuration -> inputfile << endl;
         return false;
      }
   
      string line;
      ifstream myfile(configuration -> inputfile.c_str());
   
      if (! myfile.is_open()) {
         cerr << "Unable to open file: " << configuration -> inputfile << endl;
         return false;
      }
   
      while ( getline (myfile,line) ) {
          
         vector<std::string> lineData;
         stringstream lineStream(line);
         string cell;

         while(getline(lineStream,cell,',')) {
            lineData.push_back(cell);
         }
                    
         if(lineData.size() != 8) {
            cerr << "Invalid line: " << line << " skipping" << endl;
            continue;
         }
          
         handleCSVLine(lineData);
      }
   
      myfile.close();
   
      handleInputEnd();
   
      return true;
   }
   
   // Abstract methods
   virtual bool handleCSVLine(vector<std::string> &lineData) = 0;
   virtual void handleInputEnd() = 0;
   
protected:
   QueueSync *queueSync;
   Configuration *configuration;
   Statistics *statistics;


private:
};

/*
2.1 Fixed producer class - produced a queue with points

*/
class FixedProducer : public AbstractProducer {

public:
   
   FixedProducer(Configuration *myConfiguration, Statistics *myStatistics, 
        vector<Position*> *myData, QueueSync *myQueueSync) : 
        AbstractProducer(myConfiguration, myStatistics, myQueueSync), 
        data(myData) {
      
   }

   virtual bool handleCSVLine(vector<std::string> &lineData) {
   
      // Skip CSV header
      if(lineData[0] == "Moid") {
         return true;     
      }
      
      statistics->read++;
   
      // 2007-06-08 08:32:26.781
      struct tm tm1;
      struct tm tm2;
   
      if (! parseCSVDate(tm1, lineData[2])) {
         cerr << "Unable to parse start date: " << lineData[2] << endl;
         return false;
      }
   
      if (! parseCSVDate(tm2, lineData[3])) {
         cerr << "Unable to parse end date: " << lineData[3] << endl;
         return false;
      }
   
      Position *pos1 = new Position;
      Position *pos2 = new Position;
   
      pos1 -> moid = atoi(lineData[0].c_str());   
      pos2 -> moid = atoi(lineData[0].c_str());   
      
      pos1 -> tripid = atoi(lineData[1].c_str());
      pos2 -> tripid = atoi(lineData[1].c_str());
      
      pos1 -> time = tm1;
      pos2 -> time = tm2;
      
      pos1 -> x = atof(lineData[4].c_str());
      pos1 -> y = atof(lineData[5].c_str());
      
      pos2 -> x = atof(lineData[6].c_str());
      pos2 -> y = atof(lineData[7].c_str());

      putDataIntoQueue(pos1, pos2);
      
      return true;
   }
   
   void putDataIntoQueue(Position *pos1, Position *pos2) {
      pthread_mutex_lock(&queueSync->queueMutex);
      
      if(data->size() >= QUEUE_ELEMENTS) {
         pthread_cond_wait(&queueSync->queueCondition, 
                           &queueSync->queueMutex);
      }
      
      bool wasEmpty = data->empty();
      
      data->push_back(pos1);
      
      std::vector<Position*>::iterator insertPos
          = lower_bound (data->begin(), data->end(), pos2);
      
      data->insert(insertPos, pos2);
      
      if(wasEmpty) {
         pthread_cond_broadcast(&queueSync->queueCondition);
      }
      pthread_mutex_unlock(&queueSync->queueMutex);
   }
   
   virtual void handleInputEnd() {
      // Add terminal token
      data->push_back(NULL);
   }

private:
   vector<Position*> *data;
};

/*
2.1 Adaptive producer class - produced a queue with ranges

*/
class AdapiveProducer : public AbstractProducer {

public:
   
   AdapiveProducer(Configuration *myConfiguration, Statistics *myStatistics, 
        vector<InputData*> *myData, QueueSync *myQueueSync) : 
        AbstractProducer(myConfiguration, myStatistics, myQueueSync), 
        data(myData) {
      
   }

   virtual bool handleCSVLine(vector<std::string> &lineData) {
   
      // Skip CSV header
      if(lineData[0] == "Moid") {
         return true;     
      }
      
      statistics->read++;
   
      // 2007-06-08 08:32:26.781
      struct tm tm1;
      struct tm tm2;
   
      if (! parseCSVDate(tm1, lineData[2])) {
         cerr << "Unable to parse start date: " << lineData[2] << endl;
         return false;
      }
   
      if (! parseCSVDate(tm2, lineData[3])) {
         cerr << "Unable to parse end date: " << lineData[3] << endl;
         return false;
      }
   
      InputData *inputdata = new InputData;
   
      inputdata -> moid = atoi(lineData[0].c_str());   
      inputdata -> tripid = atoi(lineData[1].c_str());
      inputdata -> time_start = tm1;
      inputdata -> time_end = tm2;
      inputdata -> x_start = atof(lineData[4].c_str());
      inputdata -> y_start = atof(lineData[5].c_str());
      inputdata -> x_end = atof(lineData[6].c_str());
      inputdata -> y_end = atof(lineData[7].c_str());

      putDataIntoQueue(inputdata);

      return true;
   }
   
   void putDataIntoQueue(InputData *inputdata) {
      pthread_mutex_lock(&queueSync->queueMutex);
      
      if(data->size() >= QUEUE_ELEMENTS) {
         pthread_cond_wait(&queueSync->queueCondition, 
                           &queueSync->queueMutex);
      }
      
      bool wasEmpty = data->empty();
      
      data->push_back(inputdata);
      
      if(wasEmpty) {
         pthread_cond_broadcast(&queueSync->queueCondition);
      }
      pthread_mutex_unlock(&queueSync->queueMutex);
   }
   
   
   virtual void handleInputEnd() {
      // Add terminal token
      data->push_back(NULL);
   }
   
   
private:
   vector<InputData*> *data;
};

void printHelpAndExit(char *progName) {
   cerr << "Usage: " << progName << " -i <inputfile> ";
   cerr << "-h <hostname> -p <port> -s <adaptive|fixed>" << endl;
   cerr << endl;
   cerr << "-i is the CVS file with the trips to simulate" << endl;
   cerr << "-h specifies the hostname to connect to" << endl;
   cerr << "-p specifies the port to connect to" << endl;
   cerr << "-s sets the simulation mode" << endl;
   cerr << endl;
   cerr << "For example: " << progName << " -i trips.csv ";
   cerr << "-h localhost -p 10000 -s adaptive" << endl;
   exit(-1);
}

void parseParameter(int argc, char *argv[], Configuration *configuration) {
   
   unsigned int flags = 0;
   int option = 0;
   
   while ((option = getopt(argc, argv,"i:h:p:s:")) != -1) {
       switch (option) {
          case 'i':
             flags |= CMDLINE_INPUTFILE;
             configuration->inputfile = string(optarg);
          break;
          
          case 'h':
             flags |= CMDLINE_DESTHOST;
             configuration->desthost = string(optarg);
          break;
          
          case 'p':
             flags |= CMDLINE_DESTPORT;
             configuration->destport = atoi(optarg);
          break;

          case 's':
             flags |= CMDLINE_SIMULATION_MODE;

             if(strcmp(optarg,"adaptive") == 0) {
                configuration->simulationmode = SIMULATION_MODE_ADAPTIVE;
             } else if(strcmp(optarg,"fixed") == 0) {
                configuration->simulationmode = SIMULATION_MODE_FIXED;
             } else {
                 cerr << "Unknown simulation mode: " << optarg << endl;
                 cerr << endl;
                 printHelpAndExit(argv[0]);
             }
          break;
          
          default:
            printHelpAndExit(argv[0]);
       }
   }
   
   unsigned int requiredFalgs = CMDLINE_INPUTFILE |
                                CMDLINE_DESTHOST |
                                CMDLINE_DESTPORT |
                                CMDLINE_SIMULATION_MODE;
   
   if(flags != requiredFalgs) {
      printHelpAndExit(argv[0]);
   }
}

/*
3.0 Consumer class - consumes berlin mod data and write it to a tcp socket

*/

class Consumer {  
   
public:
   
   Consumer(Configuration *myConfiguration, Statistics *myStatistics, 
           vector<InputData*> *myQueue) 
      : configuration(myConfiguration), statistics(myStatistics), 
        queue(myQueue), socketfd(-1), ready(false) {
      
   }
   
   virtual ~Consumer() {
      closeSocket();
   }
   
   /*
   2.4 Open the network socket

   */
   bool openSocket() {
  
      struct hostent *server;
      struct sockaddr_in server_addr;
   
      socketfd = socket(AF_INET, SOCK_STREAM, 0); 

      if(socketfd < 0) {
         cerr << "Error opening socket" << endl;
         return false;
      }   
   
      // Resolve hostname
      server = gethostbyname(configuration->desthost.c_str());
   
      if(server == NULL) {
         cerr << "Error resolving hostname: " 
              << configuration->desthost << endl;
         return -1; 
      }   
   
      // Connect
      memset(&server_addr, 0, sizeof(server_addr));
      server_addr.sin_family = AF_INET;
      server_addr.sin_port = htons(configuration->destport);
   
      server_addr.sin_addr.s_addr = 
        ((struct in_addr *)server->h_addr_list[0])->s_addr;
  
      if(connect(socketfd, (struct sockaddr*) &server_addr, 
            sizeof(struct sockaddr)) < 0) {

         cerr << "Error in connect() " << endl;
         return false;
      }   
   
      ready = true;
      return true;
   }
   
   void closeSocket() {
      if(socketfd == -1) {
         return;
      }
      
      shutdown(socketfd, 2);
      socketfd = -1;
   }
   
   bool sendData(string &buffer) {
      int ret = 0;
      int toSend = buffer.length();
      const char* buf = buffer.c_str();

      for (int n = 0; n < toSend; ) {
          ret = write(socketfd, (char *)buf + n, toSend - n);
          if (ret < 0) {
               if (errno == EINTR || errno == EAGAIN) {
                  continue;
               }
               break;
          } else {
              n += ret;
          }
      }
      
      // All data was written successfully
      if(ret > 0) {
         return true;
      }
      
      return false;
   }
   
   InputData* getQueueElement() {
      InputData *element = queue->back();
      queue -> pop_back();
      return element;
   }
   
   void dataConsumer() {
      char dateBuffer[80];
      string buffer;
      stringstream ss;
      
      InputData *element = getQueueElement();
   
      while(element != NULL) {
         if(ready) {
            ss.str("");
             
            strftime(dateBuffer,80,"%d-%m-%Y %I:%M:%S",&element->time_start);

            ss << dateBuffer << DELIMITER;            
            ss << element->moid << DELIMITER;
            ss << element->tripid << DELIMITER;
            ss << element->x_start << DELIMITER;
            ss << element->y_start << "\n";
            
            buffer.clear();
            buffer = ss.str();
            
            bool res = sendData(buffer);

            if(res == true) {
               statistics->send++;
            } else {
               cerr << "Error occurred while calling write on socket" << endl;
            }

         } else {
            cerr << "Socket not ready, ignoring line" << endl;
         }
         
         delete element;
         
         element = getQueueElement();     
      }
      
      statistics -> done = true;
      cout << "Consumer Done" << endl;
   }  
   
private:
   Configuration *configuration;
   Statistics *statistics;
   vector<InputData*> *queue;
   int socketfd;
   bool ready;
};

/*
4.0 - Simulator 

*/
class Simulator {

public:

   Simulator(Configuration *myConfiguration) 
     : configuration(myConfiguration) {

   }

   virtual ~Simulator() {
   }
   
   virtual bool simulate() = 0;

private:
   Configuration *configuration;
};

/*
5.0 - Fixed Simulator 

*/
class FixedSimulator : public Simulator {
   
public:
   
   bool simulate() {
      return false;
   }
   
private:

};

/*
6.0 - Adaptive Simulator 

*/
class AdaptiveSimulator : public Simulator {
   
public:
   
   AdaptiveSimulator(Configuration *myConfiguration) 
     : Simulator(myConfiguration) {
        
   }
   
   bool simulate() {
      return false;
   }
   
private:

};

void* startConsumerThreadInternal(void *ptr) {
  Consumer* consumer = (Consumer*) ptr;
  
  bool res = consumer->openSocket();
  
  if(! res) {
     cerr << "Unable to open socket!" << endl;
     exit(EXIT_FAILURE);
  }
  
  consumer -> dataConsumer();
  
  return NULL;
}

void* startProducerThreadInternal(void *ptr) {
   AbstractProducer* producer = (AbstractProducer*) ptr;
   
   bool result = producer -> parseInputData();
   
   if(! result) {
      cerr << "Unable to parse input data" << endl;
      exit(EXIT_FAILURE);
   }
   
   return NULL;
}

int main(int argc, char *argv[]) {
   
   Configuration *configuration = new Configuration();
   gettimeofday(&configuration->start, NULL);
   
   Statistics *statistics = new Statistics(); 
   statistics->done = false;
   
   Timer timer;
   QueueSync queueSync;
   
   pthread_mutex_init(&queueSync.queueMutex, NULL);
   pthread_cond_init(&queueSync.queueCondition, NULL);
   
   pthread_t readerThread;
   pthread_t writerThread;
   
   parseParameter(argc, argv, configuration);
   
   vector<InputData*> inputData(QUEUE_ELEMENTS);
   
   Consumer consumer(configuration, statistics, &inputData);
   AdapiveProducer producer(configuration, statistics, 
                            &inputData, &queueSync);

   pthread_create(&readerThread, NULL, 
                  &startProducerThreadInternal, &producer);

   pthread_create(&writerThread, NULL, 
                  &startConsumerThreadInternal, &consumer);
   
   timer.start();
   
   while(statistics->done == false) {
      cout << "\r\033[2K" << "Sec: " << timer.getDiff() / (1024 * 1024);
      cout << " \033[1m Read:\033[0m " << statistics -> read;
      cout << " \033[1m Send:\033[0m " << statistics -> send;
      cout.flush();
      usleep(1000 * 1000);
   }
   
   pthread_join(readerThread, NULL);
   pthread_join(writerThread, NULL);
   
   pthread_mutex_destroy(&queueSync.queueMutex);
   pthread_cond_destroy(&queueSync.queueCondition);

   if(statistics != NULL) {
      delete statistics;
      statistics = NULL;
   }
   
   if(configuration != NULL) {
      delete configuration;
      configuration = NULL;
   }
   
   while(! inputData.empty()) {
      InputData *entry = inputData.back();
      inputData.pop_back();
      
      if(entry != NULL) {
         delete entry;
         entry = NULL;
      }
   }
   
   return EXIT_SUCCESS;
}
