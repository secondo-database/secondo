#ifndef CLASS_LOGMSG_H
#define CLASS_LOGMSG_H

#include <map>
#include <string>

using namespace std;

#ifndef LOGMSG_OFF
#define LOGMSG(a, b) if ( LogMsg::isActive(a) ) { b }
#endif

class LogMsg {

public:

  LogMsg(){};
  ~LogMsg(){};

  static void initByString( const string& keyList );

  static bool isActive( const string& key ); 

private:

  static map<string,bool> logMap;
  
  typedef map<string,bool>::iterator logMapIter;

};

#endif
