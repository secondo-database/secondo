/*
----
This file is part of SECONDO.

Copyright (C) 2002-2007, University in Hagen, Faculty of Mathematics
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

Implementention of some useful helper funtions.

November 2002 M. Spiekermann, Implementation of TimeTest.

September 2003 M. Spiekermann, Implementation of LogMsg/RTFlags.

December 2003 M. Spiekermann, Implementation of class Counter.

July 2004 M. Spiekermann, Implementation of showActiveFlags.

*/

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include <map>
#include <sstream>
#include <vector>
#include <string>

#include "StopWatch.h"
#include "LogMsg.h"
#include "Counter.h"
#include "CharTransform.h"

using namespace std;

/*
1 Implementation of Class StopWatch

*/


StopWatch::StopWatch()
{
  start();
}


void
StopWatch::start() {

#ifndef SECONDO_WIN32
  gettimeofday(&startReal, 0);
  //cout << startReal.tv_sec << endl;
#else
  time(&startReal);
#endif

#ifdef SECONDO_WIN32
  cstartCPU=clock();
  cstopCPU=cstartCPU;
#else
  times(&startCPU);
  stopCPU=startCPU;
#endif

}


const double
StopWatch::diffSecondsReal() {

#ifndef SECONDO_WIN32
  gettimeofday(&stopReal, 0);
  //cout << stopReal.tv_sec << endl;
  double diffSec = (stopReal.tv_sec - startReal.tv_sec)*1.0;
  diffSec += (stopReal.tv_usec - startReal.tv_usec)*1.0 / 1000000;
  return diffSec;
#else
  time(&stopReal);
  return difftime(stopReal, startReal);
#endif
}


const double
StopWatch::diffSecondsCPU() {
#ifdef SECONDO_WIN32
      cstopCPU = clock();
      return ((double) (cstopCPU - cstartCPU)) / CLOCKS_PER_SEC;
#else
     times(&stopCPU);
    double click = (double)sysconf(_SC_CLK_TCK);
    return (double)((stopCPU.tms_utime-startCPU.tms_utime)/click +
                    (stopCPU.tms_stime-startCPU.tms_stime)/click);
#endif
}


const string
StopWatch::minutesAndSeconds(const double seconds) {

  char sbuf[20+1];
  double frac = 0, sec = 0, min = 0;

  frac = modf(seconds/60, &min);
  sec = seconds - (60 * min);
  sprintf(&sbuf[0], "%.0f:%02.0f", min, sec);

  return string((const char*) sbuf);
}


const string
StopWatch::timeStr(const time_t& inTime /* = 0*/) {

  char sbuf[20+1];

  time_t usedTime = 0;
  if ( inTime == 0 ) {
    time(&usedTime);
  } else {
    usedTime = inTime;
  }

  const tm *ltime = localtime(&usedTime);
  strftime(&sbuf[0], 20, "%H:%M:%S", ltime);

  return string((const char*) sbuf);
}


const string
StopWatch::diffReal() {

  ostringstream buffer;

  buffer << "Elapsed time: " << diffSecondsReal() << " seconds.";

  return buffer.str();
}


const string
StopWatch::diffCPU() {

   ostringstream buffer;

   buffer << "Used CPU time: " << diffSecondsCPU() << " seconds.";

   return buffer.str();
}


const string
StopWatch::diffTimes() {

   ostringstream buffer;
   double sReal = diffSecondsReal();
   double sCPU = diffSecondsCPU();

   buffer << "Times (elapsed / cpu): ";

   if (sReal > 60.0)
   {
     buffer << minutesAndSeconds(sReal) << "min ";
     buffer << "(" << sReal << "sec) /";
   }
   else
   {
     buffer << sReal << "sec / ";
   }

   buffer << sCPU << "sec = " << sReal/sCPU;
   //buffer << " / ticks = " << (stopCPU - startCPU)
   //<< "[" << stopCPU << " - " << startCPU << "]";

   return buffer.str();
}


/*
2 Implementation of Class RTFlag

*/

map<string,bool>::iterator RTFlag::it;

map<string,bool> RTFlag::flagMap;


void
RTFlag::showActiveFlags(ostream& os) {

  os << "Active runtime flags:" << endl;
  if ( flagMap.size() == 0 ) {
    os << "  -none- " << endl;
  }
  for ( it = flagMap.begin(); it != flagMap.end(); it++ ) {
    os << "  -" << it->first << "-" << endl;
  }

}

void
RTFlag::initByString( const string &keyList ) {

   //  The string contains a comma separated list of
   //  keywords which is inserted into a map.

   const char* sep = ",";
   int n = keyList.length();

   if (n == 0) {
     return;
   }

   char* pbuf = new char[n+1];
   keyList.copy(pbuf,n);
   pbuf[n] = 0;

   char* pkey = 0;
   pkey=strtok(pbuf,sep);

   string key = string(pkey);
   flagMap[key] = true;

   while ( (pkey=strtok(0,sep)) != NULL ) {

     key = string(pkey);
     flagMap[key] = true;
   }


   // actions based on some activated flags
   if ( RTFlag::isActive("CMSG:Color") ) {
     color::useColors(true);
   }

   delete [] pbuf;
}

bool
RTFlag::isActive( const string& key ) {

  if ( (it=flagMap.find( key )) != flagMap.end() ) {

    return it->second;
  }
  else {

    return false;
  }
}

void
RTFlag::setFlag( const string& key, const bool value ) {

  if ( (it=flagMap.find( key )) != flagMap.end() ) {

    it->second = value;
  }
  else {

    flagMap[key] = value;
    cerr << "New Flag added!" << endl;
    showActiveFlags(cout);
  }
}

/*
3 Implementation of Class Counter

*/

map<string, Counter::CounterInfo>::iterator Counter::it;

map<string, Counter::CounterInfo> Counter::CounterMap;

/*
4 Implementation of ~operator<<~ for class ~tab~ see CharTransform.h

*/

ostream& operator << (ostream& os, const tab& f) {
  return f(os);
}


/*
5 Parts of class ~color~

*/



ostream&
tab::operator() (ostream& os) const
{
  for (int i=n; i!=0; i--) os.put(c);
  return os;
}

// The next operator is implemented in file "UtilFunctions.cpp"
ostream& operator << (ostream& os, const tab& f);


/*
The class below can be used as manipulator in streams. If the
runtime flag "CMSG:Color" is set $os << color(red)$ will return
a escape sequence which switches to color red on "some" terminals.
Without the flag an empty string will be returned.

*/

void
color::useColors(const bool value) {

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

ostream& color::operator() (ostream& os) const
{
    os << col[c % n];
    return os;
}

vector<string> color::col;
int color::n = 1;

ostream& operator << (ostream& os, const color& c) {
  return c(os);
}


/*
The function below checks if a string contains only white space
characters or if it is empty.

*/

bool
isSpaceStr(const string& s)
{
  return s.find_first_not_of(" \t\v\n\r") == string::npos;
}

size_t
firstNonSpace(const string& s)
{
  return s.find_first_not_of(" \t\v\n\r");
}

string removeNewLines(const string& s)
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


string expandTabs(const string& s, const int n)
{
  string result=s;

  size_t p1 = 0;
  size_t p2 = 0;

  while (p1 != string::npos) {
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

string translate(const string& s,  const string& from, const string& to)
{
  string result=s;

  size_t end = s.size();
  size_t p1 = 0;
  size_t p2 = 0;

  while (p1 < end)
  {
    p2 = result.find_first_of(from,p1);

    if ( p2 != string::npos )
    {
      result.replace(p2, 1, to);
      p1 = p2+1;
    }
    else
    {
      p1 = end;
    }
    //cout << p1 << endl;
  }
  return result;
}

string hexStr(const string& s)
{
  stringstream res;

  size_t end = s.size();
  size_t p = 0;

  while (p < end)
  {
    res << hex << static_cast<int>( s[p] ) << " ";
    p++;
    //cout << p << endl;
  }
  return res.str();
}



class StringTokenizer{
  public:
     StringTokenizer(const string& _src, 
                     const string& _delim) : 
        src(_src), delim(_delim),
        pos(0) {}

     bool hasMoreTokens(){
       return pos < src.length();
     }

     string nextToken(char& delimiter){
        size_t nextpos = src.find_first_of(delim,pos);
        if(nextpos==string::npos){
           nextpos = src.length();
        }
        string res = src.substr(pos,(nextpos-pos)+1);
        assert(res.size()>0);
        delimiter = res.at(res.size()-1);
        pos = nextpos + 1;
        return res;
     }
  private:
     string src;
     string delim;
     size_t pos;  
};


class StringSplitter{
 public:
     StringSplitter(const string& _src, const size_t _length):
         src(_src),length(_length), length2(_length), pos(0) {
       assert(_length>0);
     }
     
     StringSplitter(const string& _src, const size_t _length1, 
                    const size_t _length2):
         src(_src), length(_length1), length2(_length2), pos(0){
         if(length==0){
            length=_length2;
         }
         assert(length>0);
         assert(_length2>0);
     }

     bool hasMoreTokens(){
       return pos < src.length();
     }

     string nextToken(){
        string res = src.substr(pos,length);
        pos += (length + 1);
        length = length2;
        return res;
     }

  private:
     string src;
     size_t length;
     size_t length2;
     size_t pos;  

};



/*
A simple word wrapping algorithm which tries to fold a given
string after one of the allowed wrap chars (see below). Optionally
one could specify ~indent1~ for the first and ~indent2~ for the following lines.
Examples can be found in file "Tests/tcharutils.cpp".


*/
string
wordWrap( const size_t indent1, const size_t indent2,
          const size_t textwidth, const string& s ){

  string indentstr = "";
  size_t indent = indent1;
  string indent2str(indent2,' ');

  static const string delim(" ,.:!?;)]}-+*><=\t\n");

  string res = "";


  char curDelim;
  string line(indent1, ' ');

  StringTokenizer st(s, delim); 

  while(st.hasMoreTokens()){ // process all tokens in s

    string token = st.nextToken(curDelim);
    token = expandTabs(token,4); 

    if(line.size() + token.size() <= textwidth){
      // token fits in current line
      line += token;
      if(curDelim == '\n' || curDelim == '\r'){
         res += line;
         line = indent2str;
         indent = indent2; 
      }
    } else {
      // remainder of line too short for token
      if(token.size() <= textwidth-indent2){
        // token fits into a single line
        // insert a line break
        res += line;
        res += "\n";
        indent = indent2;
       
        line = indent2str + token;
        if(curDelim == '\n' || curDelim == '\r'){
           res += line;
           line = indent2str;
           indent = indent2; 
        }
      } else { // token longer as a single line -> split it
        StringSplitter ss(token, textwidth-indent, textwidth-indent2);
        string tokenpart;
        while(ss.hasMoreTokens()){
          tokenpart = ss.nextToken();
          res += line;
          res += "\n";
          line = indent2str + tokenpart;
        }
        indent = indent2;
        if(curDelim == '\n' || curDelim == '\r'){
           res += line;
           line = indent2str;
        }
      }   
    }
  } // for all tokens
  if(line.length() > indent){
     res += line; // append last line to res
  }
 
  return res;
}



/*
Some more convenient signatures which call wordWrap with apropriate
parameters.

*/

string
wordWrap(const string& s1, const int textwidth, const string& s2)
{
  return s1 + wordWrap( s1.size(), s1.size(), textwidth, s2);
}

string
wordWrap( const string& s1, const int indent2,
          const int textwidth, const string& s2 )
{
  return s1 + wordWrap( s1.size(), indent2, textwidth, s2);
}

string
wordWrap( const int textwidth, const string& s,
          const int indent1/*=0*/, const int indent2/*=0*/ )
{
  return wordWrap(indent1, indent2, textwidth, s);
}

/*
The function trim removes all leading and trailing whitespaces.
As second parameter a string which defines more characters used
for trimming can be defined.

*/

string trim(const string& s, const string& ext/*=""*/)
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

bool expandVarRef(string& s)
{
  //cout << "Input: \"" << s << "\"" << endl;
  size_t pos1 = s.find('$');
  size_t pos2 = s.find(')', pos1);
  if ( (s.size() > pos1+1) && (s[pos1+1] == '(') && (pos2 != string::npos) )
  {
    // evaluate environment variable
    string var = s.substr(pos1+2, (pos2-pos1-1)-1);
    const char* val = getenv( var.c_str() );
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

string expandVar(const string& s)
{
  string t = s;
  expandVarRef(t);
  if ( WinUnix::WindowsHost() )
    t = translate(t, "/", "\\");
  else
    t = translate(t, "\\", "/");
  return t;
}

// append characters if necessary
string padStr(const string& s, size_t n, const char ch/*=' '*/)
{
  if ( n > s.length() )
    return s + string( n - s.length(), ch );
  else
    return s;
}


bool hasSuffix(const string& suf, const string& s)
{
  return (s.rfind(suf) != string::npos);
}


bool removeSuffix(const string& suf, string& s)
{
  size_t pos = s.rfind(suf);
  if (pos != string::npos) {
    s = s.substr(0,pos);
    return true;
  }
  return false;
}


bool hasPrefix(const string& pre, const string& s)
{
  return ( s.find(pre) == 0);
}


bool removePrefix(const string& pre, string& s)
{
  size_t pos = s.find(pre);
  if (pos == 0) {
    s = s.substr(pre.length());
    return true;
  }
  return false;
}

