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
  
  typedef map<string,bool>::iterator FlagMapIter;
  static FlagMapIter it;

};

#endif
