/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

Dec. 2002, H. Bals
July 2005, M. Spiekermann, class ~tab~ added. 
August 2005, M. Spiekermann, new class ~color~ and new function ~isSpaceStr~ added. 


*/
#ifndef CHAR_TRANSFORM_H
#define CHAR_TRANSFORM_H

#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

/*

The functions in this file are necessary because in GCC 3.2
~toupper~ and ~tolower~ are templates. This makes it impossible
to use ~toupper~ or ~tolower~ in calls to the STL function
~transform~. Instead, the functions defined below can be used.

*/

inline char
ToUpperProperFunction(char c)
{
  return toupper(c);
};

inline char
ToLowerProperFunction(char c)
{
  return tolower(c);
}

/*
The class below can be used as a manipulator in streams:
os << tab(5, 'x') will create "xxxxx". Without parameters
the default tab(4, ' ') will be used.

*/

class tab {

  char c;
  int n;
  
public:
  tab(int no=4, char ch=' ') : c(ch), n(no) {}
  ~tab(){}

  inline ostream& operator() (ostream& os) const 
  {
    for (int i=n; i!=0; i--) os.put(c);
    return os; 
  }
};

// The next operator is implemented in file "UtilFunctions.cpp"
ostream& operator << (ostream& os, const tab& f);


/*
The class below can be used as manipulator in streams. If the
runtime flag "CMSG:Color" is set os << color(red) will return
a escape sequence which switches to color red on "some" terminals.
Without the flag an empty string will be returned. 

*/

typedef enum {normal, red, green, blue} ColorCode;

class color {

  ColorCode c;
  static vector<string> col;
  static int n;

public:
  color(const ColorCode C = normal) :c(C) { col.resize(n); }

  static void useColors(const bool value) {

    if ( value ) 
    {
      n = 4; // must be changed when new colors are added
      col.resize(n);
      col[normal % n] = "\033[0m"; 
      col[red % n] = "\033[31m";
      col[green % n] = "\033[32m";
      col[blue % n] = "\033[34m"; 
    }
    else
    {
      vector<string>::iterator it = col.begin();
      while ( it != col.end() )
      {
        *it = "";
        it++;
      }
    } 
  }

  inline ostream& operator() (ostream& os) const 
  { 
    os << col[c % n]; 
    return os; 
  }

};

// The next operator is implemented in file "UtilFunctions.cpp"
ostream& operator << (ostream& os, const color& c);


/*
The function below checks if a string contains only white space
characters or if it is empty.

*/

inline bool 
isSpaceStr(const string& s)
{
  return s.find_first_not_of(" \t\v\n") == string::npos; 
}

/*
The function trim removes all leading and trailing whitespaces.
As second parameter a string which defines more characters used
for trimming can be defined.

*/

inline string trim(const string& s, const string& ext="")
{	
  static const string ignoreChars=" \n\r\a\b\f\t\v"+ext;
  size_t start = s.find_first_not_of(ignoreChars);
  size_t end = s.find_last_not_of(ignoreChars);
  return s.substr(start,end-start+1);
}

inline bool expandVarRef(string& s)
{
  //cout << "Input: \"" << s << "\"" << endl;
  size_t pos1 = s.find('$');
  size_t pos2 = s.find(')', pos1);
  if ( (s.size() > pos1+1) && (s[pos1+1] == '(') && (pos2 != string::npos) )
  {
    // evaluate environment variable
    string var = s.substr(pos1+2, (pos2-pos1-1)-1);
    char* val = getenv( var.c_str() );
    if ( !val )
      val = "";
   
    s.replace(pos1, pos2-pos1+1, val);
    //cout << "Variable: " << var << endl;
    //cout << "Value   : " <<  val << endl;
    //cout << "Result  : " <<  s << endl;
    return expandVarRef(s);
  }
  return true;
}

inline string expandVar(const string& s)
{
  string t = s;
  expandVarRef(t);
  return t; 
}

class SpecialChars {

public:
  const char sp; // space
  const char dq; // double quote
  const char sq; // single quote
  const char lb; // left round brace
  const char rb; // right round brace

  SpecialChars() : 
    sp(' '),
    dq('"'),
    sq('\''),
    lb('('),
    rb(')')
  {}

};


#endif /* CHAR_TRANSFORM_H */
