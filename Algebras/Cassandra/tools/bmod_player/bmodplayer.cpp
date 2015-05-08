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
#define CMDLINE_STATISTICS       1<<1
#define CMDLINE_DESTHOST         1<<2
#define CMDLINE_DESTPORT         1<<3
#define CMDLINE_SIMULATION_MODE  1<<4
#define CMDLINE_BEGINTIME        1<<5
#define CMDLINE_ENDTIME          1<<6

#define QUEUE_ELEMENTS 10000
#define DELIMITER ","

#define SIMULATION_MODE_ADAPTIVE 1
#define SIMULATION_MODE_FIXED    2

using namespace std;

/*
1.3 Structs

*/
struct Configuration {
   string inputfile;
   string statisticsfile;
   string desthost;
   size_t destport;
   short simulationmode;
   time_t beginoffset;
   time_t endoffset;
   time_t programstart;
};

struct Statistics {
   unsigned long read;
   unsigned long send;
   unsigned long queuesize;
   bool done;
};

struct InputData {
    size_t moid;
    size_t tripid;
    tm time_start;
    tm time_end;
    time_t time_diff;
    float x_start;
    float y_start;
    float x_end;
    float y_end;
    float x_diff;
    float y_diff;
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
1.4 Compare functions for structs

*/
bool comparePositionTime(const Position* left, const Position* right) { 
   Position* left1  = const_cast<Position*>(left);
   Position* right1 = const_cast<Position*>(right);
   
   time_t left_time = mktime(&left1->time);
   time_t right_time = mktime(&right1->time);
   
   if(left_time <= right_time) {
      return true;
   }
   
   return false;
}


/*
2.0 Simulation class

*/
class Simulation {
   
public:
   
   Simulation(Configuration *myConfiguration) :
              configuration(myConfiguration) {
      
   }
   
   time_t getSimulationTime() {
      timeval curtime;
      
      gettimeofday(&curtime, NULL);
      time_t elapsedTime = (time_t) curtime.tv_sec 
                            - configuration->programstart;
      
      return elapsedTime + configuration->beginoffset;
   }
   
private:
   Configuration *configuration;
};


/*
2.0 Abstract Producer class - reads berlin mod csv data 

*/
class AbstractProducer {
public:

    AbstractProducer(Configuration *myConfiguration, 
        Statistics *myStatistics, QueueSync *myQueueSync) : 
        queueSync(myQueueSync) ,
        configuration(myConfiguration), statistics(myStatistics),
        jumpToOffsetDone(false) {
   
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
   
   bool isBeforeBeginOffset(vector<std::string> lineData) {
            
      if(jumpToOffsetDone == false && 
         configuration->beginoffset > 0) {
            
         struct tm tm1;
         memset(&tm1, 0, sizeof(struct tm));
   
         if (! parseCSVDate(tm1, lineData[2])) {
            return true;
         }
   
         if(mktime(&tm1) < configuration->beginoffset) {
            return true;
         }
         
         jumpToOffsetDone = true;      
      }   
      
      return false;
   }
   
   bool isAfterEndOffset(vector<std::string> lineData) {
      
      if(configuration->endoffset > 0) {
         struct tm tm1;
         memset(&tm1, 0, sizeof(struct tm));
      
         if (! parseCSVDate(tm1, lineData[2])) {
            return false;
         }
      
         if(mktime(&tm1) > configuration->endoffset) {
            return true;
         }
      }
      
      return false;
   }
   
   bool parseLineData(vector<std::string> &lineData, string &line) {
      stringstream lineStream(line);
      string cell;

      while(getline(lineStream,cell,',')) {
         lineData.push_back(cell);
      }
                 
      if(lineData.size() != 8) {
         cerr << "Invalid line: " << line << " skipping" << endl;
         return false;
      }
      
      return true;
   }

   bool parseInputData() {
   
      if( access( configuration -> inputfile.c_str(), F_OK ) == -1 ) {
         cerr << "Unable to open input file: " 
              << configuration -> inputfile << endl;
         return false;
      }
   
      string line;
      ifstream myfile(configuration -> inputfile.c_str());
   
      if (! myfile.is_open()) {
         cerr << "Unable to open input file: " 
              << configuration -> inputfile << endl;
         return false;
      }
   
      while ( getline (myfile,line) ) {
         vector<std::string> lineData;
         bool result = parseLineData(lineData, line);
         
         if(result == true) {
            
            // Skip data before begin offset
            if(isBeforeBeginOffset(lineData)) {
               continue;
            }
            
            if(isAfterEndOffset(lineData)) {
               break;
            }
            
            handleCSVLine(lineData);
         }
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
   bool jumpToOffsetDone;

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
      
        prepareQueue = new vector<Position*>();
   }
   
   virtual ~FixedProducer() {
      
      if(prepareQueue != NULL) {
         delete prepareQueue;
         prepareQueue = NULL;
      }
      
      if(data != NULL) {
         while(! data -> empty()) {
            Position *entry = data->back();
            data -> pop_back();
      
            if(entry != NULL) {
               delete entry;
               entry = NULL;
            }
         }
         
         delete data;
         data = NULL;
      }
   }

   virtual bool handleCSVLine(vector<std::string> &lineData) {
   
      // Skip CSV header
      if(lineData[0] == "Moid") {
         return true;     
      }
         
      // 2007-06-08 08:32:26.781
      struct tm tm1;
      struct tm tm2;
      
      // Init structs
      memset(&tm1, 0, sizeof(struct tm));
      memset(&tm2, 0, sizeof(struct tm));
   
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
      
      statistics->read = statistics->read + 2;
      
      return true;
   }
   
   void insertIntoQueue(Position *pos) {
      std::vector<Position*>::iterator insertPos
          = upper_bound (prepareQueue->begin(), prepareQueue->end(), 
                         pos, comparePositionTime);
      
      prepareQueue->insert(insertPos, pos);
   }
   
   void printPositionTime(Position *position) {
      char dateBuffer[80];
      strftime(dateBuffer,80,"%d-%m-%Y %H:%M:%S",&position->time);
      cout << "Time is: " << dateBuffer << endl;
   }
   
   void syncQueues(Position *position) {
      pthread_mutex_lock(&queueSync->queueMutex);
      
      bool wasEmpty = data->empty();
      
      while(prepareQueue->size() > 0 && 
            (position == NULL || 
             comparePositionTime(position, prepareQueue->front()) == false)) {
               
         if(data->size() >= QUEUE_ELEMENTS) {
            pthread_cond_wait(&queueSync->queueCondition, 
                              &queueSync->queueMutex);
         }
         
         data->push_back(prepareQueue->front());
         prepareQueue -> erase(prepareQueue->begin());
      }
      
      if(wasEmpty) {
         pthread_cond_broadcast(&queueSync->queueCondition);
      }
      
      // Update statistics 
      statistics->queuesize = data->size();
      
      pthread_mutex_unlock(&queueSync->queueMutex);
   }
   
   void putDataIntoQueue(Position *pos1, Position *pos2) {
      
      insertIntoQueue(pos1);
      insertIntoQueue(pos2);
      
      // Move data from prepare queue to real queue
      if(comparePositionTime(pos1, prepareQueue->front()) == false) {
         syncQueues(pos1);
      } 

   }
   
   virtual void handleInputEnd() {
      // Move data from pending queue to final queue
      syncQueues(NULL);
      
      // Add terminal token for consumer
      data->push_back(NULL);
   }

private:
   vector<Position*> *prepareQueue;
   vector<Position*> *data;
};

/*
2.2 Adaptive producer class - produced a queue with ranges

*/
class AdapiveProducer : public AbstractProducer {

public:
   
   AdapiveProducer(Configuration *myConfiguration, Statistics *myStatistics, 
        Simulation *mySimulation, vector<InputData*> *myData, 
        QueueSync *myQueueSync) : 
        AbstractProducer(myConfiguration, myStatistics, myQueueSync), 
        simulation (mySimulation), data(myData) {
   
   }
   
   virtual ~AdapiveProducer() {
      if(data != NULL) {
         while(! data -> empty()) {
            InputData *entry = data->back();
            data -> pop_back();
      
            if(entry != NULL) {
               delete entry;
               entry = NULL;
            }
         }
         
         delete data;
         data = NULL;
      }
   }
   
   void formatData(struct tm *tm, char *buffer, size_t bufferLength) {
     strftime(buffer, bufferLength, "%Y-%m-%d %H:%M:%S", tm);
   }
   
   void waitForLineRead(struct tm &lineDate) {
      
      time_t lineDiff = 0;
      
      do {
          time_t simulationTime = simulation -> getSimulationTime();
          lineDiff = mktime(&lineDate) - simulationTime;
          
          //char buffer[80];
          //formatData(&lineDate, buffer, sizeof(buffer));
          //cout << "Time of line: " << buffer 
          //     <<  " " << mktime(&lineDate) << endl;
        
          //time_t simulationTime = simulationTime;
          //formatData(gmtime(&simulationTime), buffer, sizeof(buffer));
          
          // cout << "Simulation time is: " 
          //      << buffer << " " << simulationTime << endl;
          
          if(lineDiff > 0) {
             usleep(10000);
          }

      } while(lineDiff > 0);
      
   }

   virtual bool handleCSVLine(vector<std::string> &lineData) {
   
      // Skip CSV header
      if(lineData[0] == "Moid") {
         return true;     
      }
      
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
      
      // Set begin offset, if not specified via command line argument
      // This value it's required to determine the begin of the simulation
      if(configuration->beginoffset == 0) {
         configuration->beginoffset = mktime(&tm1);
      }
      
      // Wait with the processing of the line, until the simulation has 
      // reached this time
      waitForLineRead(tm1);
   
      InputData *inputdata = new InputData;
   
      inputdata -> moid = atoi(lineData[0].c_str());   
      inputdata -> tripid = atoi(lineData[1].c_str());
      inputdata -> time_start = tm1;
      inputdata -> time_end = tm2;
      inputdata -> x_start = atof(lineData[4].c_str());
      inputdata -> y_start = atof(lineData[5].c_str());
      inputdata -> x_end = atof(lineData[6].c_str());
      inputdata -> y_end = atof(lineData[7].c_str());
      
      inputdata -> x_diff = inputdata->x_end - inputdata->x_start;
      inputdata -> y_diff = inputdata->y_end - inputdata->y_end;
      inputdata -> time_diff = mktime(&(inputdata->time_end)) 
                                - mktime(&(inputdata->time_start));

      putDataIntoQueue(inputdata);
      
      statistics->read++;

      return true;
   }
   
   void putDataIntoQueue(InputData *inputdata) {
      pthread_mutex_lock(&queueSync->queueMutex);
      
      data->push_back(inputdata);
      
      // Update statistics 
      statistics->queuesize = data->size();
      
      pthread_mutex_unlock(&queueSync->queueMutex);
   }
   
   
   virtual void handleInputEnd() {
      // Add terminal token
      data->push_back(NULL);
   }
   
   
private:
   Simulation *simulation;
   vector<InputData*> *data;
};


/*
3.0 Consumer class - consumes berlin mod data and write it to a tcp socket

*/
class AbstractConsumer {  
   
public:
   
   AbstractConsumer(Configuration *myConfiguration, Statistics *myStatistics, 
           QueueSync* myQueueSync) : configuration(myConfiguration), 
           statistics(myStatistics), queueSync(myQueueSync), socketfd(-1), 
        ready(false) {
      
   }
   
   virtual ~AbstractConsumer() {
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
   
   virtual void dataConsumer() = 0;
   

protected:
   Configuration *configuration;
   Statistics *statistics;
   QueueSync *queueSync;
   int socketfd;
   bool ready;
   
private:
};


/*
3.1 FixedConsumer class

*/
class AdaptiveConsumer : public AbstractConsumer {

public:
   
   AdaptiveConsumer(Configuration *myConfiguration, Statistics *myStatistics, 
           Simulation *mySimulation, vector<InputData*> *myQueue, 
           QueueSync* myQueueSync) 
      : AbstractConsumer(myConfiguration, myStatistics, myQueueSync), 
           simulation(mySimulation), queue(myQueue) {
      
   }

/*
5.0.1 Remove all Elements from working queue that are out dated
   
*/
   void removeOldElements() {
      pthread_mutex_lock(&queueSync->queueMutex);
      
      time_t currentSimulationTime = simulation->getSimulationTime();
      
      for(vector<InputData*>::iterator it = queue -> begin(); 
          it != queue -> end(); ) {
         
         InputData *element = *it;
         
         if(mktime(&(element->time_end)) < currentSimulationTime) {
            it = queue -> erase(it);
            delete element;
         } else {
            it++;
         }
      } 
  
      pthread_mutex_unlock(&queueSync->queueMutex);
   }
   
   virtual void dataConsumer() {
      float posx;
      float posy;
      float diff;
      
      char dateBuffer[80];
      string buffer;
      stringstream ss;
      time_t currentSimulationTimeRun;
      InputData *element;
      size_t counter;
      
      currentSimulationTimeRun = 0;
      
      while(true) {
         removeOldElements();
         
         if(! ready) {
             cerr << "Socket not ready, skipping simulation run" << endl;
             continue;
         }
         
         // Wait for next second
         while(currentSimulationTimeRun == simulation->getSimulationTime()) {
            usleep(1000);
         }
         
         pthread_mutex_lock(&queueSync->queueMutex);
         counter = 0;
         currentSimulationTimeRun = simulation->getSimulationTime();
         
         strftime(dateBuffer,80,"%d-%m-%Y %H:%M:%S", 
                  gmtime(&currentSimulationTimeRun));
      
         for(vector<InputData*>::iterator it = queue -> begin(); 
             it != queue -> end(); it++) {
         
            element = *it;
         
            ss.str("");
             
            diff = 0.1;
            posx = element->x_start;
            posy = element->y_start;

            ss << dateBuffer << DELIMITER;
            ss << element->moid << DELIMITER;
            ss << element->tripid << DELIMITER;
            ss << posx << DELIMITER;
            ss << posy << "\n";
            
            buffer.clear();
            buffer = ss.str();
            
            bool res = sendData(buffer);

            if(res == true) {
               statistics->send++;
            } else {
               cerr << "Error occurred while calling write on socket" << endl;
            }
            
            // Check simulation time
            if(counter % 10 == 0) {
               if(currentSimulationTimeRun < simulation->getSimulationTime()) {
                  break;
               }
            }
                
            counter++;
         }    
         pthread_mutex_unlock(&queueSync->queueMutex);
      }
      
      statistics -> done = true;
      cout << "Consumer Done" << endl;
   }  
   
private:
      Simulation *simulation;
      vector<InputData*> *queue;
};

/*
3.2 AdaptiveConsumer class

*/
class FixedConsumer : public AbstractConsumer {
public:
   FixedConsumer(Configuration *myConfiguration, Statistics *myStatistics, 
           vector<Position*> *myQueue, QueueSync* myQueueSync) 
      : AbstractConsumer(myConfiguration, myStatistics, myQueueSync), 
        queue(myQueue) {
      
   }
   
   Position* getQueueElement() {
      pthread_mutex_lock(&queueSync->queueMutex);
       
      // Queue empty
      if(queue -> size() == 0) {
         pthread_cond_wait(&queueSync->queueCondition, 
                           &queueSync->queueMutex);
      }
   
      bool wasFull = queue->size() >= QUEUE_ELEMENTS;
      Position *element = queue->front();
      queue -> erase(queue->begin());
      
      // Queue full
      if(wasFull) {
         pthread_cond_broadcast(&queueSync->queueCondition);
      }
      
      pthread_mutex_unlock(&queueSync->queueMutex);
      
      return element;
   }
   
   virtual void dataConsumer() {
      char dateBuffer[80];
      string buffer;
      stringstream ss;
      
      Position *element = getQueueElement();
   
      while(element != NULL) {
         if(ready) {
            ss.str("");
             
            strftime(dateBuffer,80,"%d-%m-%Y %H:%M:%S",&element->time);

            ss << dateBuffer << DELIMITER;            
            ss << element->moid << DELIMITER;
            ss << element->tripid << DELIMITER;
            ss << element->x << DELIMITER;
            ss << element->y << "\n";
            
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
   vector<Position*> *queue;
};

/*
4.0 Statistics class

*/
class StatisticsDisplay {

public:
   
   StatisticsDisplay(Configuration *myConfiguration,
                     Statistics *myStatistics, Timer *myTimer) :
                     configuration(myConfiguration), 
                     statistics(myStatistics), timer(myTimer),
                     outputfile(NULL) {
                        
      openStatistics();
   }
   
   virtual ~StatisticsDisplay() {
      closeStatistics();
   }
   
   void openStatistics() {
      if(outputfile == NULL) {
         outputfile = fopen((configuration->statisticsfile).c_str(), "w");
         
         if(outputfile == NULL) {
            cerr << "Unable to open: " << configuration->statisticsfile 
                 << " for writing, exiting" << endl;
            exit(EXIT_FAILURE);
         }
         
         fprintf(outputfile, "#Sec\tRead\tWrite\n");
      }
   }
   
   void closeStatistics() {
      if(outputfile != NULL) {
         fclose(outputfile);
         outputfile = NULL;
      }
   }
   
   size_t getElapsedSeconds() {
      return timer-> getDiff() / (1000 * 1000);
   }
   
   void printStatisticsData() {
      cout << "\r\033[2K" << "Sec: " << getElapsedSeconds();
      cout << " \033[1m Read:\033[0m " << statistics -> read;
      cout << " \033[1m Send:\033[0m " << statistics -> send;
      cout << " \033[1m Queuesize:\033[0m " << statistics -> queuesize;
      cout.flush();
   }
   
   void writeStatisticsData() {
      if(outputfile != NULL) {
         fprintf(outputfile, "%zu\t%lu\t%lu\n", getElapsedSeconds(),
                statistics -> read, statistics -> send);
         fflush(outputfile);
      }
   }
   
   void mainLoop() {
      
      gettimeofday(&lastrun, NULL);
      
      while(statistics->done == false) {
         printStatisticsData();
         writeStatisticsData();
         waitForNextSecond();
      }
   }
   
private:
      void waitForNextSecond() {
         struct timeval curtime;
         struct timeval result;
         
         do {
            usleep(100);
            gettimeofday(&curtime, NULL);
            timersub(&curtime, &lastrun, &result);
         } while(result.tv_sec < 1);
         
         lastrun.tv_sec++;
      }
   
      Configuration *configuration;
      Statistics *statistics;
      Timer *timer;
      FILE *outputfile;
      struct timeval lastrun;
};

/*
5.0.0 Helper function to create consumer threads

*/
void* startConsumerThreadInternal(void *ptr) {
  AbstractConsumer* consumer = (AbstractConsumer*) ptr;
  
  bool res = consumer->openSocket();
  
  if(! res) {
     cerr << "Unable to open socket!" << endl;
     exit(EXIT_FAILURE);
  }
  
  consumer -> dataConsumer();
  
  return NULL;
}

/*
5.0.1 Helper function to create producer threads

*/
void* startProducerThreadInternal(void *ptr) {
   AbstractProducer* producer = (AbstractProducer*) ptr;
   
   bool result = producer -> parseInputData();
   
   if(! result) {
      cerr << "Unable to parse input data" << endl;
      exit(EXIT_FAILURE);
   }
   
   return NULL;
}

/*
5.0.2 Helper function to create the statistic thread

*/
void* startStatisticsThreadInternal(void *ptr) {
   StatisticsDisplay* statistics = (StatisticsDisplay*) ptr;
   
   statistics -> mainLoop();
   
   return NULL;
}

/*
7.0 BerlinModPlayer main class

*/
class BModPlayer {

public:

/*
7.1 Default constructor, initializes variables and set the
    simulation timezone
   
*/   
   BModPlayer() {
      timeval curtime;
      
      // Set timezone to utc to allow conversions between
      // timestamp(UTC) <-> simulation time
      setenv("TZ", "UTC", 1);
      tzset();

      // Create and init configuration structure
      configuration = new Configuration();
      gettimeofday(&curtime, NULL);
      configuration->programstart = (time_t) curtime.tv_sec;
      configuration->beginoffset = 0;
      configuration->endoffset = 0;
   
      // Create and init statistics structure
      statistics = new Statistics();
      statistics->read=0;
      statistics->send=0;
      statistics->queuesize=0;
      statistics->done = false;
   
      // Create and init timer
      timer = new Timer();
      
      // Create and init new simulation
      simulation = new Simulation(configuration);

      // Init queuesync structure
      pthread_mutex_init(&queueSync.queueMutex, NULL);
      pthread_cond_init(&queueSync.queueCondition, NULL);
   }
 
/*
7.2 Create worker depending on commandline flags
   
*/
   void createWorker() {
      if(configuration->simulationmode == SIMULATION_MODE_ADAPTIVE) {
          vector<InputData*> *inputData = new vector<InputData*>();
   
          consumer = new AdaptiveConsumer(configuration, statistics, 
                                  simulation, inputData, &queueSync);
                  
          producer = new AdapiveProducer(configuration, statistics, 
                                  simulation, inputData, &queueSync);
                                  
      } else if(configuration->simulationmode == SIMULATION_MODE_FIXED) {
         vector<Position*> *inputData = new vector<Position*>();
      
         consumer = new FixedConsumer(configuration, statistics, 
                                 inputData, &queueSync);
      
         producer = new FixedProducer(configuration, statistics, 
                                 inputData, &queueSync);
      } else {
         cerr << "Unknown simulation mode" << endl;
         exit(EXIT_FAILURE);
      }
   }

/*
7.3 Main function of the BerlinMODPlayer.
    Start the worker and wait until they finish
   
*/
   void run(int argc, char *argv[]) {

      parseParameter(argc, argv, configuration);

      createWorker();

      StatisticsDisplay statisticsDisplay(configuration, 
                        statistics, timer);

      // Create worker threads
      pthread_create(&readerThread, NULL, 
                     &startProducerThreadInternal, producer);

      pthread_create(&writerThread, NULL, 
                     &startConsumerThreadInternal, consumer);

      pthread_create(&statisticsThread, NULL, 
                     &startStatisticsThreadInternal, 
                     &statisticsDisplay);
                  
      timer->start();
   
      // Wait for running threads
      pthread_join(readerThread, NULL);
      pthread_join(writerThread, NULL);
   
      statistics->done = true;
      pthread_join(statisticsThread, NULL);
   
      pthread_mutex_destroy(&queueSync.queueMutex);
      pthread_cond_destroy(&queueSync.queueCondition);
      
      cleanup();
   }
   
private:

/*
7.4 Print usage infomations about this software
   
*/   
   void printHelpAndExit(char *progName) {
      cerr << "Usage: " << progName << " -i <inputfile> -o <statisticsfile> ";
      cerr << "-h <hostname> -p <port> -s <adaptive|fixed> -b <beginoffset> ";
      cerr << "-e <endoffset>";
      cerr << endl;
      cerr << endl;
      cerr << "Required parameter:" << endl;
      cerr << "-i is the CVS file with the trips to simulate" << endl;
      cerr << "-o is the output file for statistics" << endl;
      cerr << "-h specifies the hostname to connect to" << endl;
      cerr << "-p specifies the port to connect to" << endl;
      cerr << "-s sets the simulation mode" << endl;
      cerr << endl;
      cerr << "Optional parameter:" << endl;
      cerr << "-b is the begin time offset for the simulation" << endl;
      cerr << "-e is the end time offset for the simulation" << endl;
      cerr << endl;
      cerr << "For example: " << progName << " -i trips.csv ";
      cerr << "-o statistics.txt -h localhost ";
      cerr << "-p 10000 -s adaptive -b '2007-05-28 06:00:14'" << endl;
      cerr << "-e '2007-05-28 08:22:31'" << endl;
      exit(-1);
   }

/*
7.5 Parse commandline parameter and create the corresponding
    configuration object
   
*/   
   void parseParameter(int argc, char *argv[], Configuration *configuration) {
   
      unsigned int flags = 0;
      int option = 0;
      struct tm tm; 
   
      while ((option = getopt(argc, argv,"i:o:h:p:s:b:e:")) != -1) {
          switch (option) {
             case 'i':
                flags |= CMDLINE_INPUTFILE;
                configuration->inputfile = string(optarg);
             break;
             
             case 'o':
                flags |= CMDLINE_STATISTICS;
                configuration->statisticsfile = string(optarg);
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
             
             case 'b':
                 flags |= CMDLINE_BEGINTIME;
                 
                 if (! strptime(optarg, "%Y-%m-%d %H:%M:%S", &tm)) {
                    cerr << "Unable to parse begin date: " << optarg 
                         << endl << endl;
                    printHelpAndExit(argv[0]);
                 }
                 
                 configuration->beginoffset = mktime(&tm);
             break;
             
             case 'e':
                 flags |= CMDLINE_ENDTIME;
                 
                 if (! strptime(optarg, "%Y-%m-%d %H:%M:%S", &tm)) {
                    cerr << "Unable to parse end date: " << optarg 
                         << endl << endl;
                    printHelpAndExit(argv[0]);
                 }
                 
                 configuration->endoffset = mktime(&tm);
             break;
          
             default:
               printHelpAndExit(argv[0]);
          }
      }
   
      unsigned int requiredFalgs = CMDLINE_INPUTFILE |
                                   CMDLINE_STATISTICS |
                                   CMDLINE_DESTHOST |
                                   CMDLINE_DESTPORT |
                                   CMDLINE_SIMULATION_MODE;
   
      if((flags & requiredFalgs) != requiredFalgs) {
         printHelpAndExit(argv[0]);
      }
   }

/*
7.6 Cleanup and delete all instantiated objects
   
*/    
   void cleanup() {
      if(consumer != NULL) {
         delete consumer;
         consumer = NULL;
      }
   
      if(producer != NULL) {
         delete producer;
         producer = NULL;
      }

      if(statistics != NULL) {
         delete statistics;
         statistics = NULL;
      }
   
      if(configuration != NULL) {
         delete configuration;
         configuration = NULL;
      }
   
      if(timer != NULL) {
         delete timer;
         timer = NULL;
      }
      
      if(simulation != NULL) {
         delete simulation;
         simulation = NULL;
      }
   }
   
   Configuration *configuration;
   Statistics *statistics;
   Timer *timer;
   Simulation *simulation;
   AbstractConsumer *consumer;
   AbstractProducer *producer;
   
   QueueSync queueSync;
   pthread_t readerThread;
   pthread_t writerThread;
   pthread_t statisticsThread;
};

/*
8.0 Main Function - launch the Berlin Mod Player

*/
int main(int argc, char *argv[]) {
   
   BModPlayer bModPlayer;
   bModPlayer.run(argc, argv);
   
   return EXIT_SUCCESS;
}
