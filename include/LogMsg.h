/*
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

1 Header File LogMsg.h 

December 2003 M. Spiekermann 

1.1 Overview

This file declares a class ~RTFlag~ (Runtime Flag) and a preprocessor Macro
~LOGMSG~. It can be used to identify a bool value with a string constant. The
value for a given flag name is true, when it appears in the file
"SecondoConfig.ini"[1] in the list of the values for the key RTFlag. It can be
used for trace messages or runtime configuration. This is good for testing or
comparing different implementations without recompilation. The alternative of
defining and reading in new keys in "SecondoConfig.ini" is much more
complicated.  The macro  or the class are used as presented below. 

----LOGMSG("MyFlagName", cerr << "variable x:" << x << endl;)

    if ( RTFlag::isActive("MyFlagName") ) {
    
    ... code which should only be executed in this case

    } 
----

All flags should be documented in the configuration file. However, the
mechanism is quite simple, take care to use the same string constants in your
code and in the configuration file otherwise you will get in trouble. 

*/


#ifndef CLASS_RTFLAG_H
#define CLASS_RTFLAG_H

#include <map>
#include <string>

using namespace std;

#ifndef LOGMSG_OFF
#define LOGMSG(a, b) if ( RTFlag::isActive(a) ) { b }
#endif

class RTFlag {

public:

  RTFlag(){};
  ~RTFlag(){};

  static void initByString( const string& keyList );

  inline static bool isActive( const string& key ) { 
    
    if ( (it=flagMap.find( key )) != flagMap.end() ) { return it->second;  } else { return false; };
  };

private:

  static map<string,bool> flagMap;
  
  static map<string,bool>::iterator it;

};

#endif
