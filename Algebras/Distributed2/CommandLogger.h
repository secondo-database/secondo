/*
----
This file is part of SECONDO.

Copyright (C) 2017,
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

*/
#ifndef COMMAND_LOGGER_H
#define COMMAND_LOGGER_H 

#include <vector>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace distributed2{

class ConnectionInfo;


class CommandLogger{
  public:
     CommandLogger() {}
     ~CommandLogger() {}

     void reset(){
        log.clear();
     }
     
     void insert(ConnectionInfo* ci,
                 std::string command){
       boost::lock_guard<boost::mutex> lock(mtx);
       log.push_back(std::make_pair(ci,command));
     }

     const std::pair<ConnectionInfo*, std::string>& 
     operator[](const int i) const{
        boost::lock_guard<boost::mutex> lock(mtx);
        return log[i];
     }

     size_t size()const{
       return log.size();
     }

  private:
     std::vector<std::pair<ConnectionInfo*, std::string> > log;
     mutable boost::mutex mtx;
};

}

#endif

