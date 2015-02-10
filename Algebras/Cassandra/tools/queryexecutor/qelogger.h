/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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


1 QueryExecutor for Distributed-SECONDO


1.1 Includes

*/

#ifndef __QUERYEXECUTOR_LOGGER__
#define __QUERYEXECUTOR_LOGGER__

#define LOGFILE "/tmp/queryexecutor.log"

#define LOG_DEBUG(M)   do { Logger::instance->lock(); \
       Logger::instance->debug() \
       << "[Debug] " << M << endl; Logger::instance->unlock(); } while (false)
          
#define LOG_INFO(M)    do { Logger::instance->lock(); \
       Logger::instance->info()  \
       << "[Info] "  << M << endl; Logger::instance->unlock(); } while (false)
          
#define LOG_WARN(M)    do { Logger::instance->lock(); \
       Logger::instance->warn()  \
       << "[Warn] "  << M << endl; Logger::instance->unlock(); } while (false)
          
#define LOG_ERROR(M)   do { Logger::instance->lock(); \
       Logger::instance->error() \
       << "[Error] " << M << endl; Logger::instance->unlock(); } while (false)

class Logger {
   
public:
   static void open() {
      if(Logger::instance == NULL) {
        Logger::instance = new Logger();
      }
   }
   
   static void close() {
      if(Logger::instance != NULL) {
         delete Logger::instance;
         Logger::instance = NULL;
      }
   }
   
   Logger() {
      logfile.open(LOGFILE);
   } 
   
   virtual ~Logger() {
      logfile.close();
   }
   
   void lock() {
      
   }
   
   void unlock() {
      
   }
   
   ostream& debug() {
      return logfile;
   }
   
   ostream& info() {
      return logfile;
   }
   
   ostream& warn() {
      return cout;
   }
   
   ostream& error() {
      return cout;
   }
   
   static Logger *instance;
      
private:
   ofstream logfile;
};

Logger* Logger::instance = NULL;

#endif