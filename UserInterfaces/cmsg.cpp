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

April 2007, M. Spiekermann. Some code moved from LogMsg.h into this file

*/

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "CharTransform.h"
#include "FileSystem.h"
#include "LogMsg.h"
#include "Messages.h"
#include "StopWatch.h"


#ifdef THREAD_SAFE
#include <boost/thread.hpp>
#endif


using namespace std;





ostream* traceOS = &cerr;

/*
A global pointer to an ~ostream~ object.

*/

/*
Per-thread message state; see the class definition in include/LogMsg.h. Held in
function-local statics so that each thread's stream is constructed the first
time that thread asks for it -- the streams carry a locale, and one that was
never constructed fails on the first ~endl~ rather than at the point of use.

They are also deliberately never destroyed, which is why each is reached
through a pointer that is allocated once and never freed. Secondo logs while it
shuts down -- the algebras, the broker and the context all report as they are
reset -- and glibc destroys a thread's thread-local objects at the *start* of
~exit~, before any static destructor runs. A stream destroyed at that point but
written to a moment later corrupts the heap, which is what SecondoPL did on
Ubuntu 22.04 and 24.04. Living forever costs one small stream per thread that
ever logs, and the memory goes back when the process ends in any case.

The same reasoning covers the mutex: it must still be lockable at whatever
point during teardown the last message is sent.

*/
int& CMsg::stdOutput()
{
  static thread_local int value = 1;
  return value;
}

std::ofstream*& CMsg::fp()
{
  static thread_local std::ofstream* value = 0;
  return value;
}

std::stringstream& CMsg::buffer()
{
  static thread_local std::stringstream* stream = new std::stringstream();
  return *stream;
}

std::stringstream& CMsg::devnull()
{
  static thread_local std::stringstream* stream = new std::stringstream();
  return *stream;
}

// Guards allErrors and the open log files.
std::mutex& CMsg::msgMutex()
{
  static std::mutex* guarded = new std::mutex();
  return *guarded;
}

CMsg cmsg;

/*

Here we define a global instance ~cmsg~ which can be used in any other code
file.  Since it is not specified when the constructor will be called (we
observed problems on Mac OSX with gcc 4.0)the initialize function will be
called 

*/

CMsg::CMsg()
{
  init(); 
}
  

CMsg::~CMsg() // close open files
{
  lock_guard<mutex> guard(msgMutex());
  for ( map<string,ofstream*>::iterator it = files.begin();
  it != files.end();
  it++ )
  {
     it->second->close();
     delete it->second;
  }
}


void CMsg::init()
{
  // Runs once, while the global cmsg is constructed. The default log file goes
  // into the shared map rather than into the thread-local fp: fp names the file
  // the *calling* thread is currently writing to, and file() sets it.
  lock_guard<mutex> guard(msgMutex());
  logFileStr = "secondo.log";
  prefix = "tmp/";
  ofstream* defaultFile = new ofstream();
  files[logFileStr] = defaultFile;
  defaultFile->open((prefix + logFileStr).c_str());
  allErrors.str("");
}

ostream& CMsg::file()
{
  {
    lock_guard<mutex> guard(msgMutex());
    fp() = files[logFileStr];
  }
  stdOutput() = 3;
  return buffer();
}

ostream& CMsg::file(const string& fileName)
{
  lock_guard<mutex> guard(msgMutex());
  map<string,ofstream*>::iterator it = files.find(fileName);

  if  ( it != files.end() ) {

    fp() = it->second;

  } else {

    fp() = new ofstream();
    files[fileName] = fp();
    fp()->open((prefix + fileName).c_str());
  }
  //stdOutput() = 3;
  return *fp();
}

ostream& CMsg::info(const string& key) {

  if (RTFlag::isActive(key)) {
    stdOutput() = 1; return buffer();
  } else { 
    stdOutput() = 0; return devnull(); 
  }
}

void CMsg::send() {

  if ( isSpaceStr( buffer().str() ) ) {
    buffer().str("");
    buffer().clear();
    return;
  }

  // Held for the whole of the write, so that a message reaches the terminal or
  // the log in one piece instead of interleaved with another thread's, and so
  // that allErrors and the file streams are not touched concurrently.
  lock_guard<mutex> guard(msgMutex());

  switch (stdOutput()) {

  case 3:
  {
    // fp is only ever set by file(), which is also what selects channel 3, so
    // it is non-null here; checked rather than assumed because the two are now
    // separate pieces of per-thread state.
    if ( fp() ) {
      (*fp()) << buffer().str();
    }
     break;
  }
  case 2: 
  {
    cerr << color(red) << "Error: " << buffer().str() << color(normal);
    cerr.flush();
    allErrors << "Error: " << buffer().str();
    break;
  }
  case 1: 
  {
    cout << buffer().str();
    cout.flush();
    break;
  }
  case 0:
  {
    devnull().str("");
    devnull().clear();
    break;
  } 
  default :
  {
    allErrors << buffer().str();
  }
  }

  buffer().str("");
  buffer().clear();
}

string CMsg::getErrorMsg() {

  lock_guard<mutex> guard(msgMutex());
  string result = allErrors.str();
//  allErrors.str("");
//  allErrors.clear(); 

  if ( isSpaceStr(result) ) {
    result = "";
  }

  return result;
}

void CMsg::resetErrors(){
  lock_guard<mutex> guard(msgMutex());
  allErrors.str("");
  allErrors.clear();
}

/*
Implementation of Class ProgMesHandler 

*/

bool
ProgMesHandler::handleMsg(NestedList* nl, ListExpr list, int source)
{
  #ifdef THREAD_SAFE
  boost::lock_guard<boost::mutex> guard(mtx);
  #endif

  if(source>=0){
    return false;
  }

  if(!nl->HasMinLength(list,2)){
     return false;
  }
  if ( !nl->IsEqual(nl->First(list),"progress") ){
    return false;
  }
  ListExpr second = nl->Second(list);
  if(!nl->HasMinLength(second,2)){
    return false;
  }
  if(nl->AtomType(nl->First(second))!=IntType){
     return false;
  }
  if(nl->AtomType(nl->Second(second))!=IntType){
     return false;
  }


  int ActValue = nl->IntValue(nl->First(second)); 
  int TotalValue = nl->IntValue(nl->Second(second)); 

  double rt = 0;

  // initialize the size of the progress bar
  if (ActValue < 0) {
    for (int i = 1; i < TotalValue; i++){
      if ((i % 10) == 0) {
          cout << "|"; 
      } else {
          cout << "-";
      }
    }
    cout << "|" << endl;
    if(s){
      delete s;
    }
    s = new StopWatch;  
    total = TotalValue;
    highest = -1; 
    return true; 
  }


  if(!s) {
    s = new StopWatch(); 
  }

  // end of progress messages
  if(TotalValue <= 0){
      cout << endl << "feddisch (done)!" << endl << endl;
      delete s;
      s = 0;
      return true;
  }

  // normal progress messages
  
  rt = s->diffSecondsReal();
  if(ActValue > TotalValue){
     ActValue = TotalValue;
  }

  //if((ActValue <= highest)){
    // don't go back in progress
  //  return true;
 // }
  //highest = ActValue;
  
  double pr = (double)ActValue / (double)TotalValue;

  int dots = (int)  (((double)total) * pr);
  if(dots<0){
     dots = 0;
  }
  if(dots>total){
    dots = total;
  }

  string bar1(dots, '.');
  string bar2(total-dots, ' ');
  int p = ((ActValue*100) / TotalValue);
  int restTime = static_cast<int>( ceil( rt/p * (100-p) ) );
  int showMin = restTime / 60;
  int showSec = restTime - (showMin * 60);

  if(ActValue>0){
    cout << "\r" << bar1 << bar2 
    //     << " ( " << setw(3) << setfill(' ') <<  p << "\% )  "
         << " remaining: " << showMin << ":" 
         << setw(2) << setfill('0') << showSec << " min  "
         << flush;
  } else {
    cout << "\r" << bar1 << bar2 
    //   << " remaining: ?? :  min  "
         << " ( " << setw(3) << setfill(' ') <<  0 << "\% )  "
         << flush;
  }
  return true;
}

