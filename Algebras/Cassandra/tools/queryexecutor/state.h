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

#ifndef __QUERYEXECUTOR_STATE__
#define __QUERYEXECUTOR_STATE__


class QueryexecutorState {

public:
   QueryexecutorState() {
      pthread_mutex_init(&mutex, NULL);
   }
   
   virtual ~QueryexecutorState() {
      pthread_mutex_destroy(&mutex);
   }
   
   void setState(size_t workerid, string state) {
      pthread_mutex_lock(&mutex);
      states[workerid] = state;
      pthread_mutex_unlock(&mutex);
      print();
   }
   
   void setQuery(string myQuery) {
      pthread_mutex_lock(&mutex);
      query = myQuery;
      pthread_mutex_unlock(&mutex);
   }
   
   void print() {
      pthread_mutex_lock(&mutex);
      size_t lines = 4 + states.size();
      cout << "\033[" << lines << "F";
      cout << "\033[0J";
      cout << "\r";
      cout << "================" << endl;
      cout << "Execution state for query: " << query << endl;
      cout << "================" << endl;
      
      for (std::map<size_t,string>::iterator it=states.begin(); 
          it!=states.end(); ++it) {
             
          std::cout << it->first << " => " << it->second << '\n';
      }

      pthread_mutex_unlock(&mutex);
   }
   
private:
   string query;
   map< size_t, string > states;
   pthread_mutex_t mutex;
};

#endif