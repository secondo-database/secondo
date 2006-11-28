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

Jan-April 2006, M. Spiekermann. New functions, especially ~wordWrap~ which
allows a pretty folding of words with a specified text length.

*/
#ifndef CHAR_TRANSFORM_H
#define CHAR_TRANSFORM_H

#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>

#include "WinUnix.h"

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

----
os << tab(5, 'x')} will create "xxxxx". 
----

Without parameters the default 

----
tab(4, ' '}
----
will be used.

*/

class tab {

  char c;
  int n;
  
public:
  tab(int no=4, char ch=' ') : c(ch), n(no) {}
  ~tab(){}

  inline ostream& operator() (ostream& os) const;
};

// The next operator is implemented in file "UtilFunctions.cpp"
ostream& operator << (ostream& os, const tab& f);


/*
The class below can be used as manipulator in streams. If the
runtime flag "CMSG:Color" is set $os << color(red)$ will return
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

  static void useColors(const bool value);
  inline ostream& operator() (ostream& os) const; 
};

// The next operator is implemented in file "UtilFunctions.cpp"
ostream& operator << (ostream& os, const color& c);


/*
The templates below can be used to extract numbers or the next
word out of a string or input stream by ignoring white spaces. 
For example: 

----
parse<double>("\t\n\n 0.005") => 0.005 
----  

*/

template<class T>
inline T parse(istream& is)
{
  T v;
  is >> v;
  return v;
}

template<class T>
inline T parse(const string& s) 
{
  istringstream is(s);
  return parse<T>(is);
}

/*
The function below checks if a string contains only white space
characters or if it is empty.

*/

bool isSpaceStr(const string& s);

size_t firstNonSpace(const string& s);

string removeNewLines(const string& s);

string expandTabs(const string& s, const int n);

string translate(const string& s,  const string& from, const string& to);

string hexStr(const string& s);


/*
A simple word wrapping algorithm which tries to fold a given
string after one of the allowed wrap chars (see below). Optionally
one could specify ~indent1~ for the first and ~indent2~ for the following lines.
Examples can be found in file "Tests/tcharutils.cpp".


*/

string
wordWrap( const int indent1, const int indent2, 
          const int textwidth, const string& s );

/*
Some more convenient signatures which call wordWrap with apropriate
parameters.

*/

string
wordWrap(const string& s1, const int textwidth, const string& s2);

string
wordWrap( const string& s1, const int indent2, 
          const int textwidth, const string& s2 );

string
wordWrap( const int textwidth, const string& s, 
          const int indent1=0, const int indent2=0 );

/*
The function trim removes all leading and trailing whitespaces.
As second parameter a string which defines more characters used
for trimming can be defined.

*/

string trim(const string& s, const string& ext="");

bool expandVarRef(string& s);

string expandVar(const string& s);

// append characters if necessary
string padStr(const string& s, size_t n, const char ch=' ');

bool removeSuffix(const string& suf, string& s);

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
