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
#include "NList.h"

using namespace std;

/*
1 Implementation of Class StopWatch

*/


StopWatch::StopWatch() :
startCPU(0),
stopCPU(0)
{
  start();
}


void
StopWatch::start() {

#ifndef SECONDO_WIN32
  gettimeofday(&startReal, 0);
#else
  time(&startReal);
#endif 
  startCPU=clock();
}


const double
StopWatch::diffSecondsReal() {

#ifndef SECONDO_WIN32
  gettimeofday(&stopReal, 0);
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

  stopCPU = clock();
  return ((double) (stopCPU - startCPU)) / CLOCKS_PER_SEC;
}


const string
StopWatch::minutesAndSeconds(const double seconds) {
  
  static char sbuf[20+1];
  static double frac = 0, sec = 0, min = 0;
  
  frac = modf(seconds/60, &min);
  sec = seconds - (60 * min);
  sprintf(&sbuf[0], "%.0f:%02.0f", min, sec);
  
  return string((const char*) sbuf);
}


const string
StopWatch::timeStr(const time_t& inTime /* = 0*/) {

  static char sbuf[20+1];
  
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



/*
A simple word wrapping algorithm which tries to fold a given
string after one of the allowed wrap chars (see below). Optionally
one could specify ~indent1~ for the first and ~indent2~ for the following lines.
Examples can be found in file "Tests/tcharutils.cpp".


*/

string
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
      cout << "text[" << p1 << "]='" << text.substr(p1,20) << "...'" << endl;
    
    if (lines > 1) {
      len = textwidth - indent2;
      indentStr = indent2Str;
      // do a line break
      result += "\n";
    }  

    string substr="";
    // search a suitable wrap position
    bool found = false;
    bool newLine = true;
    size_t p2 = end;
    size_t endPos = min(p1+len, end);
   
    while ( !found /*&& (endPos >= (p1+(2*len/3)))*/ )
    { 
      // check for a predefined linebreak
      p2 = text.find_first_of('\n', p1);
      if ( p2 <= endPos ) 
      { 
        newLine = true;
        break;
      }  
      
      if ( (end - p1) <= len) // check if last line
      { 
        newLine = true;
        p2 = end;
        break;
      } 
       
      newLine = false;
      p2 = text.find_last_of(wrapChars, endPos);
      
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
      cout << "endPos: " << endPos << endl;
      cout << "p2: " << p2 << endl;
    }  
      
    if ( !newLine && 
         ( (p2 == string::npos) || ((p2-p1+1) < 2*len/3) || (p2 <= p1) ))
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
      assert( p2 >= p1 );    
      substr = text.substr(p1,p2-p1);
      lastbreak = p2;
    }
    if (trace) {
      cout << "text[" << lastbreak << "]='" 
           << text[lastbreak] << "'" << endl;
    }  
    lines++;

    // 
    if ( isspace(text[lastbreak]) ) {
      lastbreak++;
    }  
    p1 = lastbreak;
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


bool removeSuffix(const string& suf, string& s)
{
  size_t pos = s.rfind(suf);
  if (pos != string::npos) {
    s = s.substr(0,pos);
    return true;
  }
  return false;
}  


/*
6 Parts of class ~NList~

*/

NestedList* NList::nl = 0;

ostream& operator<<(ostream& os, const NList& n) { 
  os << n.convertToString(); 
  return os;
}


/*
Static members of class ~CMsg~.

*/

void CMsg::init()
{
  { 
    stdOutput = 1; 
    fp = new ofstream();
    logFileStr = "secondo.log";
    prefix = "tmp/";
    files[logFileStr] = fp;
    fp->open((prefix + logFileStr).c_str()); 
    buffer.str("");
    allErrors.str("");
    devnull.str("");
  }
}

