/*
----
This file is part of SECONDO.

Copyright (C) 2018,
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

//[$][\$]

@author
c. Behrndt

@note
Checked - 2020

@history
Version 1.0 - Created - C.Behrndt - 2020

*/
#ifndef _BasicEngineHelper_CPP_
#define _BasicEngineHelper_CPP_

#include <boost/algorithm/string.hpp>
#include <fstream>

using namespace std;

namespace BasicEngine {

/*
7 Helper Functions ~BasicEngineHelper~

Implementation.

7.1 ~replaceStringAll~

Replacing a string with an other string.

Returns the new string.

*/
string replaceStringAll(string str
                    , const string& replace, const string& with) {
  if(!replace.empty()) {
    size_t pos = 0;
    while ((pos = str.find(replace, pos)) != string::npos) {
      str.replace(pos, replace.length(), with);
      pos += with.length();
    }
  }
return str;
}

/*
7.2 ~readFile~

Reads a file and returns the input as a string text.

*/
string readFile(string *path){
string line = "";
ifstream myfile (*path);
string res;

  if (myfile.is_open()){
    while ( getline (myfile,line)){
      res.append(line);
    }
    myfile.close();
  }
return res;
}

}/* namespace BasicEngine */
#endif //_BasicEngineHelper_CPP_
