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


using std::ostream;
using std::string;
using std::stringstream;
using std::cerr;
using std::cout;
using std::endl;


class Trace {

  public:
  Trace(const string& s, ostream& os = cerr) : 
    trc(false), 
    prefix(s), 
    dest(os), 
    fn("???"), 
    lmax(1) 
  {}

  void enter(const string& function) { 
    if (trc) {  
      fn = function; 
      pre() << "Start" << endl;
    }  
  }
  void on() { trc = true;
    dest << "* trace on [" << prefix << "]" << endl;   
  }
  void off() { trc = false; 
    dest << "* trace off [" << prefix << "]" << endl;   
  }
  void level(int n) { lmax = n; }

  template <typename T>
  inline void out(const T& v, int level=1) { 
    if (trc && (level <= lmax))  
      pre() << v << endl; 
  }

  template <typename T>
  inline void show(const string& var, const T& value, int level=1) { 
    if (trc && (level <= lmax))  
      pre() << var << " = " << value << endl; 
  }


  inline void flush(int level=1) { 
    if (trc && (level <= lmax))  {
      pre() << ss.str() << endl; 
    }  
    ss.str("");
    ss.clear();
  }

  inline stringstream& add() { return ss; }

  private:

    inline ostream& pre() { 
       return dest << prefix << ":" << fn << ":"; 
    }

    bool trc;
    const string prefix;
    ostream& dest;
    string fn;
    stringstream ss;
    int lmax;
};

/*
Auxiliary functions for debugging

*/

string Array2HexStr(const char* data, size_t size, size_t offset = 0);
string Array2Str(const char* data, size_t size);

/*
The functions above converts the data behind a char pointer into a string value.

*/

template<typename T>
inline string Var2HexStr(const T& v)
{
  return Array2HexStr( (char*)&v , sizeof(T), 0 );
}


#endif
