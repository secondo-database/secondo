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

//paragraph	[23]	table3columns:	[\begin{quote}\begin{tabular}{lll}]	[\end{tabular}\end{quote}]
//[--------]	[\hline]
//characters	[1]	verbatim:	[\verb|]	[|]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [$\leq$]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File Counter.h 

December 2003 M. Spiekermann 

August 2004 M. Spiekermann, Function ~resetAll~ introduced.

Nove 2004 M. Spiekermann. The counter values will additonally be
saved in CSV format in a file called "cmd-counters.csv".

1.1 Overview

The class ~Counter~ provides a mechanism for registering counters with an
identifier. It was implemented as dynamic data structure using "std::map"[1].
Counters are created with the first call of "Counter::getRef" which returns a
reference to a variable of type "long" initialized to 0.  A counter can be
incremented or modified with 

----Counter::getRef("MyCounterName")++;
    Counter::getRef("MyCounterName")+=500;
----

All counters used are reported when the ~Application~ object is destructed or
after every query command if the RTFlag "SI:PrintCounters"[1] was set up in
"SecondoConfig.ini". Please choose long unique identifier strings to avoid that
two different programmers use the same identifier for a counter. For reasons of
simplicity there is no check for uniqueness otherwise a ~create~ function must
be introduced and called at a suitable position. 

*/


#ifndef CLASS_COUNTER_H
#define CLASS_COUNTER_H

#include <map>
//#include <pair>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

#include "LogMsg.h"
#include "CharTransform.h"

using namespace std;


class Counter 
{

public:

  typedef map<string, long> Str2ValueMap;
  typedef pair<long,bool> CounterInfo;
  typedef map<string, CounterInfo> Str2InfoMap;
  typedef Str2InfoMap::iterator iterator;

  Counter(){};
  ~Counter(){};
/*
Simple constructor and destructor.

*/

  static long& getRef( const string& identifier, const bool reportVal = true ) 
  { 
    if ( (it=CounterMap.find( identifier )) != CounterMap.end() ) 
    { 
      it->second.second = reportVal;
      return it->second.first; 
    } 
    else 
    { 
      // initialize new Counter;
      CounterMap[identifier] = make_pair(0, true);
      long& CounterRef = CounterMap[identifier].first;
      CounterMap[identifier].second = reportVal;
      return CounterRef;
    }
  };
/*
Returns a reference for the counter given its identifier. If a counter is not
found, then it is created. The flag ~displayVal~ is used to tell whether this
counter will be reported (function ~reportValues~).

*/
	
  static void resetAll() 
  {
    for ( it = CounterMap.begin(); it != CounterMap.end(); it++ ) 
      it->second.first = 0;
  }
/*
Resets all values to 0.

*/

  static void reportValue( const string& identifier, const bool reportVal) 
  {
    if ( (it=CounterMap.find( identifier )) != CounterMap.end() ) 
    {
      it->second.second = reportVal;
    }
  }
  
  static Counter::Str2ValueMap usedCounters()
  {
    Str2ValueMap resMap; 
    it=CounterMap.begin();
    while ( it != CounterMap.end() ) 
    {
      if ( it->second.second )
        resMap[it->first] = it->second.first;
      it++;
    }
    return resMap;
  }  

  
/*
Sets the report flag with the value of ~reportVal~.

*/
  static void reportValues(const int CmdNr = -1 )
  {
    int colSepWidth = 2; 
    int windowWidth = 80;

    const string colSepStr(colSepWidth,' ');
    const string sep(windowWidth,'_');

    cout << endl << sep << endl << "Counter Values:" << endl;

    // calculate the longest entry
    typedef vector<string> CounterStrings; 
    CounterStrings entryTable;
    Str2ValueMap usedCtrs = usedCounters();
    Str2ValueMap::iterator sit = usedCtrs.begin();
    
    int maxEntryLen = 0;
    while ( sit != usedCtrs.end()) 
    {
      stringstream counter;
      counter << sit->first << " = " << sit->second;
      
      int len = counter.str().length();
      if ( len > maxEntryLen )
        maxEntryLen = len;
      
      entryTable.push_back( counter.str() );
      sit++;
    }
    int colMax = windowWidth / (maxEntryLen + colSepWidth);
 
    // pretty print counter values
    // sort by string names
    sort( entryTable.begin(), entryTable.end() );
    CounterStrings::const_iterator it2=entryTable.begin();
    int colNr = 0;
    while ( it2 != entryTable.end() ) {
     
     string xspace(maxEntryLen - it2->length(), ' '); 
     cout << colSepStr << *it2 << xspace;
     it2++; 
     colNr++;
     if ( colNr == colMax ) {
       cout << endl;
       colNr = 0;
     } 
    }
    cout << endl << sep << endl << endl;
    cmsg.send();

  };
/*
Reports the values of all counters diplaying on the screen and also to a 
~csv~ file.

*/


private:

  static Str2InfoMap CounterMap;
  static iterator it;
/*
The map (and iterator) structure that holds the counters.

*/
};

#endif
