/*
---- 
This file is part of SECONDO.

Copyright (C) 2007, University in Hagen, Faculty of Mathematics and Computer Science, 
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

March. 2007, M. Spiekermann: A class for extracting environment variables 


   
*/   

#ifndef SEC_ENVIRONMENT_H
#define SEC_ENVIRONMENT_H

#include <string>
#include <map>


/*
1 Class ~Environment~

This class maintains values of environment variables. If you want
to add new variables to extract you need to add them in the implementation
file Tools/Utilities/Environment.cpp

*/
class Environment {

 typedef enum {Int, Float, Bool, String} Type; 	
 map<string, Type>   keyMap;
 map<string, int>    intMap;
 map<string, double> floatMap;
 map<string, bool>   boolMap;
 map<string, string> stringMap;

 // There will be only one instance. Hence the constructor
 // is private. ~getInstance~ will call it if necessary.
 Environment();
 static Environment* instance;
 
 template<class R>
 R getValue(string key, map<string, R>& m, R d) 
 {
   if ( m.find(key) == m.end() )
     return d;
   return m.find(key)->second;	   
 }

 void init();

 public:
 
  static Environment& getInstance() { 
   if (!instance)
     instance = new Environment();
   return *instance;
  }
 
  ~Environment() 
  { 
    if (instance)	  
     delete instance;	  
  } 
  
  int getInt(string key, int d=0) 
  {
    return getValue(key, intMap, d);  
  } 	  

  double getFloat(string key, double d=0.0) 
  {
    return getValue(key, floatMap, d);  
  } 	  

  bool getBool(string key, bool d=false) 
  {
    return getValue(key, boolMap, d);  
  } 	  

  string getString(string key, string d="") 
  {
    return getValue(key, stringMap, d);  
  } 	  

};

#endif
