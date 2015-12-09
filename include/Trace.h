/*
---- 
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics 
and Computer Science, Database Systems for New Applications.

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

Sept. 2009, M. Spiekermann, a class for managing trace messages

*/


#ifndef SEC_TRACE_H
#define SEC_TRACE_H

#include "TraceMacros.h"

#define VAL(arg) #arg, arg

#include <iostream>
#include <string>
#include <sstream>




class Trace {

  public:
  Trace(const std::string& s, std::ostream& os = std::cerr) : 
    trc(false), 
    prefix(s), 
    dest(os), 
    fn("???"), 
    lmax(1) 
  {}

  void enter(const std::string& function) { 
    if (trc) {  
      fn = function; 
      pre() << "Start" << std::endl;
    }  
  }
  void on() { trc = true;
    dest << "* trace on [" << prefix << "]" << std::endl;   
  }
  void off() { trc = false; 
    dest << "* trace off [" << prefix << "]" << std::endl;   
  }
  void level(int n) { lmax = n; }

  template <typename T>
  inline void out(const T& v, int level=1) { 
    if (trc && (level <= lmax))  
      pre() << v << std::endl; 
  }

  template <typename T>
  inline void show(const std::string& var, const T& value, int level=1) { 
    if (trc && (level <= lmax))  
      pre() << var << " = " << value << std::endl; 
  }


  inline void flush(int level=1) { 
    if (trc && (level <= lmax))  {
      pre() << ss.str() << std::endl; 
    }  
    ss.str("");
    ss.clear();
  }

  inline std::stringstream& add() { return ss; }

  private:

    inline std::ostream& pre() { 
       return dest << prefix << ":" << fn << ":"; 
    }

    bool trc;
    const std::string prefix;
    std::ostream& dest;
    std::string fn;
    std::stringstream ss;
    int lmax;
};

/*
Auxiliary functions for debugging

*/

std::string Array2HexStr(const char* data, size_t size, size_t offset = 0);
std::string Array2Str(const char* data, size_t size);

/*
The functions above converts the data behind a char pointer into a string value.

*/

template<typename T>
inline std::string Var2HexStr(const T& v)
{
  return Array2HexStr( (char*)&v , sizeof(T), 0 );
}


#endif
