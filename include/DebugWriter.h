/*
----
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, 
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


#ifndef DEBUGWRITER_H
#define DEBUGWRITER_H

#include <iostream>
#include <string>
#ifdef THREAD_SAFE
#include <boost/thread.hpp>
#endif


class DebugWriter{

  public:

  void write(bool enable, std::ostream& out, 
        void* caller, int callerID, const std::string& message){
     if(!enable) { return; }
     #ifdef THREAD_SAFE
     boost::lock_guard<boost::mutex> guard(mtx);
     #endif
     out << callerID << "::" << caller << ":: " << message << std::endl;
     out.flush(); 
  }

  void write(bool enable, std::ostream& out,
        void* caller, int callerID, const std::string& message, bool state){
     if(!enable) { return; }
     #ifdef THREAD_SAFE
     boost::lock_guard<boost::mutex> guard(mtx);
     #endif
     out << callerID << "::" << caller << ":: " << message 
         << " -> " << state << std::endl;
     out.flush(); 
  }
  

  private:
  
  #ifdef THREAD_SAFE
     boost::mutex mtx;
  #endif

};


#endif

