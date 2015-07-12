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


1 Player for BerlinMod GPS Data. This program generates a
stream of GPS coordinate updates and send them to a 
network socket. For more information, see the documentation
of this software.

1.1 Includes

*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <limits>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>

#include <curl/curl.h>

#include "timer.h"

/*
1.2 Defines

*/
#define CMDLINE_INPUTFILE        1<<0
#define CMDLINE_STATISTICS       1<<1
#define CMDLINE_DESTURL          1<<2
#define CMDLINE_SIMULATION_MODE  1<<3
#define CMDLINE_BEGINTIME        1<<4
#define CMDLINE_ENDTIME          1<<5
#define CMDLINE_UPDATE_RATE      1<<6
#define CMDLINE_SIMULATION_SPEED 1<<7

#define QUEUE_ELEMENTS 10000
#define DELIMITER ","

#define SIMULATION_MODE_ADAPTIVE 1
#define SIMULATION_MODE_FIXED    2

#define EOT             "\004"

#define VERSION         1.1

using namespace std;

// Forward declaration
class Simulation;

/*
1.3 Structs

*/
struct Configuration {
   string inputfile;
   string statisticsfile;
   string url;
   short simulationmode;
   time_t beginoffset;
   time_t endoffset;
   time_t programstart;
   size_t updaterate;
   size_t simulationspeed;
};

// Shared over multipe theads, so volatile is used to 
// prevent caching issues
struct Statistics {
   volatile unsigned long read;
   volatile unsigned long send;
   volatile unsigned long queuesize;
   volatile bool done;
   Simulation* simulation;
};

struct InputData {
    size_t moid;
    size_t tripid;
    time_t time_start;
    time_t time_end;
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
   time_t time;
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

   if(left->time <= right->time) {
      return true;
   }
   
   return false;
}


/*
2.0 Simulation class, provides informations, e.g., the current
time in the simulation.

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
      
      elapsedTime = elapsedTime * configuration -> simulationspeed;
      
      elapsedTime = elapsedTime 
           + (curtime.tv_usec / 1000000.0 * configuration -> simulationspeed);
                  
      return elapsedTime + configuration->beginoffset;
   }
   
private:
   Configuration *configuration;
};


/*
2.0 Abstract output class 

*/
class AbstractOutput {
public:
   
   AbstractOutput() : ready(false) {
      
   }
   
   virtual ~AbstractOutput() {
   }
   
   bool isReady() {
      return ready;
   }
   
   virtual bool open() = 0;
   virtual bool close() = 0;
   virtual bool sendData(Position* position) = 0;
   
protected:
   bool ready;
   
private:
};

/* 
2.1 CSV output class - convert data into
    csv and send it to a TCP socket

*/
class CSVOutput : public AbstractOutput {
public:
   
   CSVOutput(string &url) : socketfd(-1) {
      // tcp:// - 6 chars
      string hostnameport = url.substr(6);
     
      size_t pos = hostnameport.find("/");
      
      // Missing / or missing port
      if(pos == string::npos || pos+1 == hostnameport.length()) {
         cerr << "Unable to parse CSV URL: " << url << endl;
         cerr << "See help for more details" << endl;
         exit(EXIT_FAILURE);
      }

      hostname = hostnameport.substr(0, pos);
      string portString = hostnameport.substr(pos + 1, hostnameport.length());
      port = atoi(portString.c_str());
   }
   
   virtual bool sendData(Position* position) {
      stringstream ss;
      string buffer;
      char dateBuffer[80];
   
      strftime(dateBuffer,80,"%d-%m-%Y %H:%M:%S",
               gmtime(&(position->time)));

      ss << dateBuffer << DELIMITER;
      ss << position->moid << DELIMITER;
      ss << position->tripid << DELIMITER;
      ss << position->x << DELIMITER;
      ss << position ->y << "\n";
   
      buffer = ss.str();
      bool result = sendData(buffer);
   
      return result;
   }
   
   /*
   3.1 Open the network socket for writing

   */
   bool open() {

      struct hostent *server;
      struct sockaddr_in server_addr;

      socketfd = socket(AF_INET, SOCK_STREAM, 0); 

      if(socketfd < 0) {
         cerr << "Error opening socket" << endl;
         return false;
      }

      // Resolve hostname
      server = gethostbyname(hostname.c_str());
   
      if(server == NULL) {
         cerr << "Error resolving hostname: " << hostname << endl;
         return false; 
      }   
   
      // Connect
      memset(&server_addr, 0, sizeof(server_addr));
      server_addr.sin_family = AF_INET;
      server_addr.sin_port = htons(port);
   
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
   
   /*
   3.2 Close the tcp socket
   
   */
   bool close() {
      if(socketfd == -1) {
         return true;
      }
     
      // Send EOT (End of Transmission)
      int res = write(socketfd, EOT, sizeof(char));
 
      if(res < 0) {
         cerr << "Sending EOT failed" << endl;
      }

      shutdown(socketfd, 2);
      socketfd = -1;
      
      ready = false;
      
      return true;
   }
   
   /*
   3.3 Write the string on the tcp socket, ensured that
   the write class is retired, if a recoverable error occurs.
   
   */
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
   
private:
   int socketfd;
   string hostname;
   int port;
};

/* 
2.2 HTTP output class - convert data into
    json and generate a HTTP call

*/

static size_t curlWriteCallback(void *contents, size_t size, 
              size_t nmemb, void *userp) { 

    string curlReadBuffer;
    size_t realsize = size * nmemb;
    curlReadBuffer.append((char*) contents, realsize);
    
    //cout << "Curl Result: " << curlReadBuffer;
    
    return realsize;
}

class HTTPJSonOutput : public AbstractOutput {
public:
   HTTPJSonOutput(string myURL) : url(myURL), headers(NULL) {
      
   }
   
   /*
   2.2.1 Send position in JSON Format
   
   */
   virtual bool sendData(Position* position) {
      stringstream ss;
      char dateBuffer[80];
   
      //  "i1":"2015-03-06T23:20:01.000",
      strftime(dateBuffer,80,"%Y-%m-%dT%H:%M:%S.000",
               gmtime(&(position->time)));
   
      // Build JSON data
      ss << "{" << endl;
      ss << "\"Moid\":\"" << position -> moid << "\"," << endl;
      ss << "\"Position\":{" << endl;
      ss << "\"instant\":\"" << dateBuffer << "\"," << endl; 
      ss << "\"x\":" << position->x << "," << endl;
      ss << "\"y\":" << position->y << endl;
      ss << "}" << endl;
      ss << "}" << endl;
     
      // Make HTTP request
      CURLcode res;
      string postdata = ss.str();
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata.c_str());
      res = curl_easy_perform(curl);
      
      if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
      }
      
      return true;
   }
   
   /*
   2.2.2 Open output 
   
   */
   virtual bool open() {
      
      // Init cURL
      curl_global_init(CURL_GLOBAL_ALL);
      curl = curl_easy_init();
      if(curl) {
          curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
          curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
// Uncoment to debug http requests
//          curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
          curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
      }
          
      // Init headers
      curl_slist_append(headers, "Accept: application/json");  
      curl_slist_append(headers, "Content-Type: application/json");
      curl_slist_append(headers, "charsets: utf-8"); 
      
      ready = true;
      return true;
   }
   
   
   /*
   2.2.3 Close output
   
   */
   virtual bool close() {
      ready = false;
      
      if(curl != NULL) {
         curl_easy_cleanup(curl);
         curl = NULL;
      }
      
      if(headers != NULL) {
         curl_slist_free_all (headers);
         headers = NULL;
      }
      
      return true;
   }

private:
   string url;
   CURL *curl;
   struct curl_slist *headers;
};

class OutputFactory {
public:
   static AbstractOutput* getOutputInstance(string &url) {
      
      if(url.compare(0, 6, "tcp://") == 0) {
         return new CSVOutput(url);
      }
      
      if(url.compare(0,7, "http://") == 0) {
         return new HTTPJSonOutput(url);
      }
      
      return NULL;
   }
};


/*
2.0 Abstract Producer class - reads berlin mod csv data, jump to the
right offset, parses the lines and calles the abtract functions 
handleCSVLine and handleInputEnd

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

    time_t parseCSVDate(string date) {
       
      struct tm tm;
      memset(&tm, 0, sizeof(struct tm));
      
      if (strptime(date.c_str(), "%Y-%m-%d %H:%M:%S.", &tm)) {
         return mktime(&tm);
      }
   
      if (strptime(date.c_str(), "%Y-%m-%d %H:%M", &tm)) {
         return mktime(&tm);
      }
   
      if (strptime(date.c_str(), "%Y-%m-%d %H", &tm)) {
         return mktime(&tm);
      }
   
      if (strptime(date.c_str(), "%Y-%m-%d", &tm)) {
         return mktime(&tm);
      }
   
      return 0;
   }
   
   bool isBeforeBeginOffset(vector<std::string> lineData) {
            
      if(jumpToOffsetDone == false && 
         configuration->beginoffset > 0) {
            
         time_t date = parseCSVDate(lineData[2]);
   
         if (date == 0) {
            return true;
         }
   
         if(date < configuration->beginoffset) {
            return true;
         }
         
         jumpToOffsetDone = true;      
      }   
      
      return false;
   }
   
   bool isAfterEndOffset(vector<std::string> lineData) {
      
      if(configuration->endoffset > 0) {
         
         time_t date = parseCSVDate(lineData[2]);
      
         if (date == 0) {
            return false;
         }
      
         if(date > configuration->endoffset) {
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
2.1 Fixed producer class - produce a queue with points

*/
class FixedProducer : public AbstractProducer {

public:
   
   FixedProducer(Configuration *myConfiguration, Statistics *myStatistics, 
        vector<Position*> *myData, QueueSync *myQueueSync) : 
        AbstractProducer(myConfiguration, myStatistics, myQueueSync), 
        data(myData) {
      
        prepareQueue = new vector<Position*>();
   }
   
/*
2.2 Destructor remove all element from both queues
   
*/
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
      time_t time1;
      time_t time2;
      
      // Parse date
      time1 = parseCSVDate(lineData[2]);
      time2 = parseCSVDate(lineData[3]);
      
      if (time1 == 0) {
         cerr << "Unable to parse start date: " << lineData[2] << endl;
         return false;
      }
   
      if (time2 == 0) {
         cerr << "Unable to parse end date: " << lineData[3] << endl;
         return false;
      }
   
      Position *pos1 = new Position;
      Position *pos2 = new Position;
   
      pos1 -> moid = atoi(lineData[0].c_str());   
      pos2 -> moid = atoi(lineData[0].c_str());   
      
      pos1 -> tripid = atoi(lineData[1].c_str());
      pos2 -> tripid = atoi(lineData[1].c_str());
      
      pos1 -> time = time1;
      pos2 -> time = time2;
      
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
      strftime(dateBuffer,80,"%d-%m-%Y %H:%M:%S",gmtime(&(position->time)));
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
      pthread_mutex_lock(&queueSync->queueMutex);
      data->push_back(NULL);
      pthread_mutex_unlock(&queueSync->queueMutex);
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
           
           prepareQueue = new vector<InputData*>();
           activeTime = 0;
   }
   
   virtual ~AdapiveProducer() {
      
      if(prepareQueue != NULL) {
         deleteVectorContent(prepareQueue);
         delete prepareQueue;
         prepareQueue = NULL;
      }
      
      if(data != NULL) {
         deleteVectorContent(data);
         delete data;
         data = NULL;
      }
   }
   
   void deleteVectorContent(vector<InputData*> *myVector) {
      if(myVector != NULL) {
         while(! myVector -> empty()) {
            InputData *entry = myVector->back();
            myVector -> pop_back();
      
            if(entry != NULL) {
               delete entry;
               entry = NULL;
            }
         }
      }
   }
   
   void formatData(struct tm *tm, char *buffer, size_t bufferLength) {
     strftime(buffer, bufferLength, "%Y-%m-%d %H:%M:%S", tm);
   }
   
   void waitForLineRead(time_t lineDate) {
      
      time_t lineDiff = 0;
      
      do {
          time_t simulationTime = simulation -> getSimulationTime();
          lineDiff = lineDate - simulationTime;
          
          //char buffer[80];
          //formatData(&lineDate, buffer, sizeof(buffer));
          //cout << "Time of line: " << buffer 
          //     <<  " " << mktime(&lineDate) << endl;
        
          //time_t simulationTime = simulationTime;
          //formatData(gmtime(&simulationTime), buffer, sizeof(buffer));
          
          // cout << "Simulation time is: " 
          //      << buffer << " " << simulationTime << endl;
          
          if(lineDiff > 0) {
             usleep(1000);
          }

      } while(lineDiff > 0);
      
   }

   virtual bool handleCSVLine(vector<std::string> &lineData) {
   
      // Skip CSV header
      if(lineData[0] == "Moid") {
         return true;     
      }
      
      // 2007-06-08 08:32:26.781
      time_t time1;
      time_t time2;
      
      // Parse date
      time1 = parseCSVDate(lineData[2]);
      time2 = parseCSVDate(lineData[3]);
      
      if (time1 == 0) {
         cerr << "Unable to parse start date: " << lineData[2] << endl;
         return false;
      }
   
      if (time2 == 0) {
         cerr << "Unable to parse end date: " << lineData[3] << endl;
         return false;
      }
      
      // Set begin offset, if not specified via command line argument
      // This value it's required to determine the begin of the simulation
      if(configuration->beginoffset == 0) {
         configuration->beginoffset = time1;
      }
            
      // Wait with the processing of the line, until the simulation has 
      // reached this time
      if(activeTime < time1) {
         movePrepareQueueToRealQueue();
         waitForLineRead(time1);
         activeTime = time1;
      }
   
      InputData *inputdata = new InputData;
   
      inputdata -> moid = atoi(lineData[0].c_str());   
      inputdata -> tripid = atoi(lineData[1].c_str());
      inputdata -> time_start = time1;
      inputdata -> time_end = time2;
      inputdata -> x_start = atof(lineData[4].c_str());
      inputdata -> y_start = atof(lineData[5].c_str());
      inputdata -> x_end = atof(lineData[6].c_str());
      inputdata -> y_end = atof(lineData[7].c_str());
      
      inputdata -> x_diff = inputdata->x_end - inputdata->x_start;
      inputdata -> y_diff = inputdata->y_end - inputdata->y_end;
      inputdata -> time_diff = inputdata->time_end
                                - inputdata->time_start;

      putDataIntoQueue(inputdata);
      
      statistics->read++;

      return true;
   }
   
   void putDataIntoQueue(InputData *inputdata) {
      prepareQueue->push_back(inputdata);
   }
   
   void movePrepareQueueToRealQueue() {
      pthread_mutex_lock(&queueSync->queueMutex);
      
      // Move data from the prepare queue to the real queue
      for(vector<InputData*>::iterator it = prepareQueue->begin(); 
         it != prepareQueue->end(); it++) {
         
         data->push_back(*it);
      }
      
      prepareQueue->clear();
      
      // Update statistics 
      statistics->queuesize = data->size(); 
      
      pthread_mutex_unlock(&queueSync->queueMutex);
   }
   
   virtual void handleInputEnd() {
      // Move pending data to real queue
      movePrepareQueueToRealQueue();
      
      // Add terminal token
      data->push_back(NULL);
   }
   
private:
   Simulation *simulation;
   vector<InputData*> *data;
   vector<InputData*> *prepareQueue;
   time_t activeTime;
};


/*
3.0 Consumer class - consumes berlin mod data and write it to a 
tcp socket. This is an abstract class, the method dataConsumer()
needs to be implemented in child classes.

*/
class AbstractConsumer {  
   
public:
   
   AbstractConsumer(Configuration *myConfiguration, Statistics *myStatistics, 
           QueueSync* myQueueSync) : configuration(myConfiguration), 
           statistics(myStatistics), queueSync(myQueueSync) {
      
           output = OutputFactory::getOutputInstance(configuration -> url);
           
           if(output == NULL) {
              cerr << "Unable to find an output instance for URL: "
                   << configuration -> url << endl;
              exit(EXIT_FAILURE);
           }
           
           bool res = output -> open();
           
           if(! res) {
              cerr << "Unable to open output!" << endl;
              exit(EXIT_FAILURE);
           }
   }
   
   virtual ~AbstractConsumer() {
      if(output != NULL) {
         output -> close();
         delete output;
      }
   }
   
/*
3.4 Send Position to output handler
   
*/   
      bool formatAndSendData(Position *position) {
         bool result = output -> sendData(position);
         return result;
      }
   
/*
3.5 abstract method, need to be implemented in
subclasses
   
*/
   virtual void dataConsumer() = 0;
   

protected:
   Configuration *configuration;
   Statistics *statistics;
   QueueSync *queueSync;
   AbstractOutput *output;
   
private:
};


/*
4.1 FixedConsumer class

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
4.1.1 Remove all Elements from working queue that are out dated
   
*/
   void removeOldElements() {
      pthread_mutex_lock(&queueSync->queueMutex);
      
      time_t currentSimulationTime = simulation->getSimulationTime();
      
      for(vector<InputData*>::iterator it = queue -> begin(); 
          it != queue -> end(); ) {
         
         InputData *element = *it;

         if(element == NULL) {
           it++;
           continue;
         }

         if(element->time_end < currentSimulationTime) {
            it = queue -> erase(it);
            delete element;
         } else {
            it++;
         }
      } 
  
      pthread_mutex_unlock(&queueSync->queueMutex);
   }

/*
4.1.2 Create the current position for the Trip
      and send the data
   
*/
   bool formatAndSendElement(string &buffer, InputData *element, 
       time_t currentSimulationTimeRun) {
          
      // The time is behind this unit, don't send a position update
      // Element will be removed on next clanup call
      if(currentSimulationTimeRun >= element->time_end) {
         return false;
      }
          
      Position *position = new Position();

      float diff = 0;

      if(currentSimulationTimeRun != element->time_start) {
         diff = (float) (currentSimulationTimeRun - element->time_start) / 
	 (float) element->time_diff;
      }

      position->x = element->x_start + (element->x_diff * diff);
      position->y = element->y_start + (element->y_diff * diff);
      position->time = currentSimulationTimeRun;
      position->moid = element->moid;
      position->tripid = element->tripid;
            
      bool result = formatAndSendData(position);
      
      delete position;
      
      return result;
   }

/*
4.1.3 Fetch the produced elements and send them on the output
tcp socket
   
*/   
   virtual void dataConsumer() {
      string buffer;
      time_t currentSimulationTimeRun;
      size_t counter;
      InputData *element;
      
      currentSimulationTimeRun = 0;
      
      while(currentSimulationTimeRun < configuration->endoffset) {
         removeOldElements();

         if(! output -> isReady()) {
             cerr << "Output not ready, skipping simulation run" << endl;
             usleep(10000);
             continue;
         }
         
         // Wait for next simulation run
         while(currentSimulationTimeRun + (int) configuration->updaterate 
             > simulation->getSimulationTime()) {
                
            usleep(10000);
         }
         
         pthread_mutex_lock(&queueSync->queueMutex);
         counter = 0;
         currentSimulationTimeRun = simulation->getSimulationTime();
      
         for(vector<InputData*>::iterator it = queue -> begin(); 
             it != queue -> end(); it++) {
         
            element = *it;
            
            if(element == NULL) {
               continue;
            }

            bool res = formatAndSendElement(buffer, element, 
                 currentSimulationTimeRun);

            if(res == true) {
               statistics->send++;
            }
            
            // Check simulation time, break current run if the
            // next simulation second has begun and start a new
            // run.
            if(counter % 10 == 0) {
               time_t nextSimulationRun = currentSimulationTimeRun 
                      + (int) configuration->updaterate;
               
               if(nextSimulationRun <= simulation->getSimulationTime()) {
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
4.2 AdaptiveConsumer class

*/
class FixedConsumer : public AbstractConsumer {
public:
   FixedConsumer(Configuration *myConfiguration, Statistics *myStatistics, 
           vector<Position*> *myQueue, QueueSync* myQueueSync) 
      : AbstractConsumer(myConfiguration, myStatistics, myQueueSync), 
        queue(myQueue) {
      
   }

/*
4.2.1 Get the next element from the producer queue
   
*/
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


/*
4.2.3 Fetch the produced elements and send them on the output
tcp socket
   
*/    
   virtual void dataConsumer() {

      Position *element = getQueueElement();
   
      while(element != NULL) {
         if(output -> isReady()) {
            
            bool res = formatAndSendData(element);

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
5.0 Statistics class. Print informations about the simulation
on the console and into an output file.

*/
class StatisticsDisplay {

public:
   
   StatisticsDisplay(Configuration *myConfiguration,
                     Statistics *myStatistics, Timer *myTimer) :
                     configuration(myConfiguration), 
                     statistics(myStatistics), timer(myTimer),
                     outputfile(NULL), lastRead(0), lastSend(0) {
                        
      openStatistics();
   }
   
   virtual ~StatisticsDisplay() {
      closeStatistics();
   }
   
   /*
   5.1 Open the statistics output file
   
   */
   void openStatistics() {
      if(outputfile == NULL) {
         outputfile = fopen((configuration->statisticsfile).c_str(), "w");
         
         if(outputfile == NULL) {
            cerr << "Unable to open: " << configuration->statisticsfile 
                 << " for writing, exiting" << endl;
            exit(EXIT_FAILURE);
         }
         
         fprintf(outputfile, "#Sec\tRead\tWrite\tDiff read\tDiff send\n");
      }
   }
   
   /*
   5.2 Closes the statistics output file
   
   */
   void closeStatistics() {
      if(outputfile != NULL) {
         fclose(outputfile);
         outputfile = NULL;
      }
   }
   
   /*
   5.3 Get the number of elapsed seconds in the simulation
   
   */
   size_t getElapsedSeconds() {
      return timer-> getDiff() / (1000 * 1000);
   }
   
   /*
   5.4 Print statistical informations on the screen
   
   */
   void printStatisticsData() {
      cout << "\r\033[2K" << "Sec: " << getElapsedSeconds();
      cout << " \033[1m Read:\033[0m " << statistics -> read;
      cout << " \033[1m Send:\033[0m " << statistics -> send;
      cout << " \033[1m Queue size:\033[0m " << statistics -> queuesize;
      
      if(configuration->simulationmode == SIMULATION_MODE_ADAPTIVE) {
         if(statistics -> simulation != NULL) {
            time_t simulationTime = 
                 (statistics -> simulation) -> getSimulationTime();
            strftime(dateBuffer,80,"%d-%m-%Y %H:%M:%S",gmtime(&simulationTime));
            cout << " \033[1m Simulation time:\033[0m " << dateBuffer;
         }
      }
      
      cout.flush();
   }
   
   /*
   5.5 Print statistical informations into the output file
   
   */
   void writeStatisticsData() {
      if(outputfile != NULL) {

         unsigned long read = statistics -> read;
         unsigned long send = statistics -> send;

         fprintf(outputfile, "%zu\t%lu\t%lu\t%llu\t%llu\n", 
                getElapsedSeconds(), read, send, 
                (read - lastRead), (send - lastSend));
         fflush(outputfile);

         lastRead = read;
         lastSend = send;
      }
   }
   
   /*
   5.6 Write final statics data
   
   */
   void writeFinalData() {
      if(outputfile != NULL) {
         fprintf(outputfile, "\n\n");
         fprintf(outputfile, "### Total execution time (ms): %lld\n",
            timer-> getDiff() / 1000);
         fprintf(outputfile, "### Read: %lu\n", statistics -> read);
         fprintf(outputfile, "### Send: %lu\n", statistics -> send);
      }
   }
   
   /*
   5.7 Main statistics loop, update statistics information
   
   */
   void mainLoop() {
      
      gettimeofday(&lastrun, NULL);
      
      while(statistics->done == false) {
         printStatisticsData();
         writeStatisticsData();
         waitForNextSecond();
      }
      
      // Statistics are done, close output file
      writeFinalData();
      closeStatistics();
   }
   

protected:
   
   /*
   5.8 Wait until the next second of the simulation has
   begun
   
   */
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
  
private: 
      Configuration *configuration;
      Statistics *statistics;
      Timer *timer;
      FILE *outputfile;
      struct timeval lastrun;
      long long lastRead;
      long long lastSend;
      char dateBuffer[80];
};

/*
5.0.0 Helper function to create consumer threads

*/
void* startConsumerThreadInternal(void *ptr) {
  AbstractConsumer* consumer = (AbstractConsumer*) ptr;
  
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
      configuration->endoffset = numeric_limits<time_t>::max();
      configuration->updaterate = 1;
      configuration->simulationspeed = 1;
   
      // Create and init timer
      timer = new Timer();
      
      // Create and init new simulation
      simulation = new Simulation(configuration);
      
      // Create and init statistics structure
      statistics = new Statistics();
      statistics->read=0;
      statistics->send=0;
      statistics->queuesize=0;
      statistics->done = false;
      statistics->simulation = simulation;

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
      cerr << "Player for BerlinMod data, version " << VERSION << endl;
      cerr << endl;
      cerr << "Usage: " << progName << " -i <inputfile> -o <statisticsfile> ";
      cerr << "-u <connection url> -s <adaptive|fixed> {-b <beginoffset>} ";
      cerr << "{-e <endoffset>} {-r update rate} {-f simulation speed up}";
      cerr << endl;
      cerr << endl;
      cerr << "Required parameter:" << endl;
      cerr << "-i is the CVS file with the trips to simulate" << endl;
      cerr << "-o is the output file for statistics" << endl;
      cerr << "-u specifies the connection url" << endl;
      cerr << "-s sets the simulation mode" << endl;
      cerr << endl;
      cerr << "Optional parameter:" << endl;
      cerr << "-b is the begin time offset for the simulation" << endl;
      cerr << "-e is the end time offset for the simulation" << endl;
      cerr << endl;
      cerr << "Optional parameter for the adaptive simulation:" << endl;
      cerr << "-r is the update rate (default: 1 second)" << endl;
      cerr << "-f is the simulation speed up (default: 1)" << endl;
      cerr << endl;
      cerr << "Supported connection URLs:" << endl;
      cerr << "tcp://hostname/myport - Send csv lines to host ";
      cerr << "'hostname' on port 'myport'" << endl;
      cerr << "http://hostname/position - Send JSON requests to ";
      cerr << "the specified URL" << endl;
      cerr << endl;
      cerr << "For example: " << progName << " -i trips.csv ";
      cerr << "-o statistics.txt -u tcp://localhost/10000 ";
      cerr << "-s adaptive -b '2007-05-28 06:00:14' ";
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
      
      memset(&tm, 0, sizeof(struct tm));
   
      while ((option = getopt(argc, argv,"i:o:u:s:b:e:r:f:")) != -1) {
          switch (option) {
             case 'i':
                flags |= CMDLINE_INPUTFILE;
                configuration->inputfile = string(optarg);
             break;
             
             case 'o':
                flags |= CMDLINE_STATISTICS;
                configuration->statisticsfile = string(optarg);
             break;
          
             case 'u':
                flags |= CMDLINE_DESTURL;
                configuration->url = string(optarg);
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
             
             case 'r':
                 flags |= CMDLINE_UPDATE_RATE;
                 configuration->updaterate = atoi(optarg);
             break;
             
             case 'f':
                 flags |= CMDLINE_SIMULATION_SPEED;
                 configuration->simulationspeed = atoi(optarg);
             break;
          
             default:
               printHelpAndExit(argv[0]);
          }
      }
   
      unsigned int requiredFalgs = CMDLINE_INPUTFILE |
                                   CMDLINE_STATISTICS |
                                   CMDLINE_DESTURL |
                                   CMDLINE_SIMULATION_MODE;
   
      if((flags & requiredFalgs) != requiredFalgs) {
         printHelpAndExit(argv[0]);
      }
      
      // Check offsets
      checkOffsets(argv);
   }
   
/*
7.6 Check offset parameter, the offset end needs to be
after the begin offset
   
*/
   void checkOffsets(char *argv[]) {
      
      // Check if begin and end offset are set
      if(configuration->beginoffset > 0 && configuration->endoffset > 0) {
         
         // Is begin offset after end offset? => Error
         if(configuration->beginoffset >= configuration->endoffset) {   
            cerr << "End offset must be greater than begin offset" << endl;
            cerr << endl;
            printHelpAndExit(argv[0]);
         }
      }
   }

/*
7.7 Cleanup and delete all instantiated objects
   
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
