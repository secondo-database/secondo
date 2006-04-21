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

Jan-April 2006, M. Spiekermann. New functions, especially ~wordWrap~ which allows a pretty
folding of words with a specified text length.

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

inline bool 
isSpaceStr(const string& s)
{
  return s.find_first_not_of(" \t\v\n\r") == string::npos; 
}

inline size_t
firstNonSpace(const string& s)
{
  return s.find_first_not_of(" \t\v\n\r"); 
}

inline string removeNewLines(const string& s)
{
  string result=s;

  size_t end = s.size();
  size_t p1 = 0;
  size_t p2 = 0;

  while (p1 < end)
  {
    p2 = result.find_first_of('\n',p1);

    if (p2 != string::npos)
      result.erase(p2,1);    

    p1 = p2;
  }  
  return result;  
}


inline string expandTabs(const string& s, const int n)
{
  string result=s;

  size_t end = s.size();
  size_t p1 = 0;
  size_t p2 = 0;

  while (p1 < end)
  {
    p2 = result.find_first_of('\t',p1);

    if ( p2 != string::npos ) 
    {
      result.erase(p2, 1);
      result.insert(p2, string(n,' '));
    }  

    p1 = p2;
  }  
  return result;  
} 

/*
A simple word wrapping algorithm which tries to fold a given
string after one of the allowed wrap chars (see below). Optionally
one could specify ~indent1~ for the first and ~indent2~ for the following lines.
Examples can be found in file "Tests/tcharutils.cpp".


*/

inline string
wordWrap( const int indent1, const int indent2, 
          const int textwidth, const string& s )
{
  const bool trace = false;
  static const string wrapChars1(",.:!?;)]}-+*><=\t\n");
  static const string wrapChars = wrapChars1 + " ";
  string indent1Str = "";
  string indent2Str = string(indent2,' ');
  string& indentStr = indent1Str;

  //string text = removeNewLines(s);
  string text = expandTabs(s,4);

  // usable width for the first line
  size_t len = textwidth - indent1;
  int lines = 1;
  
  string result="";
  size_t end = text.size();
  size_t p1 = 0;
  size_t lastbreak = 0;
  
  while ( p1 < end )
  {
    if (trace)
      cout << "text[" << p1 << "]='" << text[p1] << "'" << endl;
    
    if (lines > 1) {
      len = textwidth - indent2;
      indentStr = indent2Str;
      // do a line break
      result += "\n";
    }  

    string substr="";
    if ( (end - p1) <= len) // check if rest fits
    {    
      substr = text.substr(p1,len);
      p1 = end; 
    }
    else
    {
      // search a suitable wrap position
      bool found = false;
      size_t p2 = 0;
      size_t endPos = p1+len;
      
      while ( !found && (endPos >= (p1+(2*len/3))) )
      { 
        p2 = text.find_last_of(wrapChars, endPos);
        
        // found a predefined linebreak
        if (text[p2] == '\n')
          break;
        
        bool lastOk = (end-p2) >= len/3;
        // dont wrap if the next char is also a wrap char
        if (lastOk && wrapChars.find(text[p2+1]) == string::npos) 
        {
          found = true;
        }  
        else
        { 
          endPos = p2-1;
        }  
      }

      if (trace) {
        cout << "p1: " << p1 << endl;
        cout << "p2: " << p2 << endl;
      }  
        
      if ( (p2 == string::npos) || ((p2-p1+1) < 2*len/3) || (p2 <= p1) )      
      {
        if (trace) 
          cout << "force break" << endl;
          
        // There is no suitable wrap char in the next len 
        // chars, hence we need to force a wrap. We split after
        // the first non-wrapChar below 4/5 of the textwidth
        
        size_t cutLen = 4*len/5; 
        endPos = p1+len;
        p2=endPos;
        while ( (p2 >= p1+cutLen) )
        { 
          p2 = text.find_last_not_of(wrapChars, endPos);
          endPos = p2-1;
        }  
        if (p2 == string::npos)
          p2 = p1+cutLen;
        substr = text.substr(p1,p2-p1);
        lastbreak = p2;
      }  
      else
      {
        if (trace) 
          cout << "soft break" << endl;
        
        assert( p2 <= (p1+len) );    
        assert( p2 > p1 );    
        substr = text.substr(p1,p2-p1);
        lastbreak = p2;
      }
      if (trace)
        cout << "text[" << lastbreak << "]='" << text[lastbreak] << "'" << endl;
      lines++;

      // 
      if ( isspace(text[lastbreak]) ) {
        lastbreak++;
      }  
      p1 = lastbreak;
    }
    result += indentStr + substr;
    if (trace) {
      cout << "substr: " << substr << endl;    
      cout << "lastbreak: " << lastbreak << endl;    
    }  
  }
  return result;
}

/*
Some more convenient signatures which call wordWrap with apropriate
parameters.

*/

inline string
wordWrap(const string& s1, const int textwidth, const string& s2)
{
  return s1 + wordWrap( s1.size(), s1.size(), textwidth, s2);
}

inline string
wordWrap( const string& s1, const int indent2, 
          const int textwidth, const string& s2 )
{
  return s1 + wordWrap( s1.size(), indent2, textwidth, s2);
}

inline string
wordWrap( const int textwidth, const string& s, 
          const int indent1=0, const int indent2=0 )
{
  return wordWrap(indent1, indent2, textwidth, s);
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
  
  if (start != string::npos && end != string::npos)
    return s.substr(start,end-start+1);
  if (start == string::npos && end != string::npos)
    return s.substr(0, end+1);  
  if (start != string::npos && end == string::npos)
    return s.substr(start);  

  return s;  
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
