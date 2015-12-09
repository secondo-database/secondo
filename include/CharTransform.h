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

August 2005, M. Spiekermann, new class ~color~ and new function 
~isSpaceStr~ added. 

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

  inline std::ostream& operator() (std::ostream& os) const;
};

// The next operator is implemented in file "UtilFunctions.cpp"
std::ostream& operator << (std::ostream& os, const tab& f);


/*
The class below can be used as manipulator in streams. If the
runtime flag "CMSG:Color" is set $os << color(red)$ will return
a escape sequence which switches to color red on "some" terminals.
Without the flag an empty string will be returned. 

*/

typedef enum {normal, red, green, blue} ColorCode;

class color {

  ColorCode c;
  static std::vector<std::string> col;
  static int n;

public:
  color(const ColorCode C = normal) :c(C) { col.resize(n); }

  static void useColors(const bool value);
  inline std::ostream& operator() (std::ostream& os) const; 
};

// The next operator is implemented in file "UtilFunctions.cpp"
std::ostream& operator << (std::ostream& os, const color& c);


/*
The templates below can be used to extract numbers or the next
word out of a string or input stream by ignoring white spaces. 
For example: 

----
parse<double>("\t\n\n 0.005") => 0.005 
----  

*/

template<class T>
inline T parse(std::istream& is)
{
  T v;
  is >> v;
  return v;
}

template<class T>
inline T parse(const std::string& s) 
{
  std::istringstream is(s);
  return parse<T>(is);
}

/*
The function below checks if a string contains only white space
characters or if it is empty.

*/

bool isSpaceStr(const std::string& s);

bool contains(const std::string& s);

size_t firstNonSpace(const std::string& s);

std::string removeNewLines(const std::string& s);

std::string expandTabs(const std::string& s, const int n);

std::string translate(const std::string& s,  const std::string& from,
                      const std::string& to);

std::string hexStr(const std::string& s);

static inline std::string int2Str(const int i) 
{
   std::stringstream s; 
   s << i;
   return s.str();
}   


/*
A simple word wrapping algorithm which tries to fold a given
string after one of the allowed wrap chars (see below). Optionally
one could specify ~indent1~ for the first and ~indent2~ for the following lines.
Examples can be found in file "Tests/tcharutils.cpp".


*/

std::string
wordWrap( const size_t indent1, const size_t indent2, 
          const size_t textwidth, const std::string& s );

/*
Some more convenient signatures which call wordWrap with apropriate
parameters.

*/

std::string
wordWrap(const std::string& s1, const int textwidth, const std::string& s2);

std::string
wordWrap( const std::string& s1, const int indent2, 
          const int textwidth, const std::string& s2 );

std::string
wordWrap( const int textwidth, const std::string& s, 
          const int indent1=0, const int indent2=0 );

/*
The function trim removes all leading and trailing whitespaces.
As second parameter a string which defines more characters used
for trimming can be defined.

*/

std::string trim(const std::string& s, const std::string& ext="");

bool expandVarRef(std::string& s);

std::string expandVar(const std::string& s);

// append characters if necessary
std::string padStr(const std::string& s, size_t n, const char ch=' ');

bool hasPrefix(const std::string& pre, const std::string& s);
bool hasSuffix(const std::string& suf, const std::string& s);

bool removeSuffix(const std::string& suf, std::string& s);
bool removePrefix(const std::string& pre, std::string& s);

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
