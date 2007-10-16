/*
---- 
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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
using namespace std;

#include <stdlib.h>
#include <assert.h>

#include <iostream>
#include <string>
#include <map>

#include "Environment.h"

Environment*
Environment::instance = 0;

Environment::Environment()
{
  keyMap["SEC_pScale"] = Float; 
  keyMap["SEC_pMinRead"] = Int;
  keyMap["SEC_pMaxRead"] = Int;
  keyMap["SECONDO_PLATFORM"] = String; 

  init(); 
}

void
Environment::init() {

  map<string, Type>::iterator it = keyMap.begin();
  for(; it != keyMap.end(); it++) {

    string key = it->first;	  
    char* value = getenv( key.c_str() );

    if ( value != 0 ) {    
    switch (it->second) {

      case Int:   { int v = atoi(value); 
		    intMap[key] = v;
		    break; } 

      case Float: { float v = atof(value); 
		    floatMap[key] = v;
		    break; }

      case Bool:  { bool v = (value == "true" || value == "TRUE") ? true 
			                                          : false;
		    boolMap[key] = v;
		    break; }

      case String: { stringMap[key] = value; break; }

      default: assert(false);		   
    }	      
    }
  }	  

}	

