#include <iostream>

#include "LogMsg.h"

const int __endian_detect = 1;
 
inline bool __little_endian() { return *(char *)&__endian_detect == 1;}
 

int
main () {

 LogMsg::initByString("hallo,test,bla,blub");

 cout << LogMsg::isActive("hallo") << endl;
 cout << LogMsg::isActive("test") << endl;
 cout << LogMsg::isActive("bla") << endl;
 cout << LogMsg::isActive("blub") << endl;
 cout << LogMsg::isActive("xfindo") << endl;

 LOGMSG( "bla", 
 
   cout << "bla is active" << endl;

 )


 LOGMSG( "sdflkj",

   cout << "sdlfkj si not active " << endl;

 )
 
 cout << "machine has ";
 if ( __little_endian() ) {
   cout << "little";
 } else {
   cout << "big";
 }
 cout << " endian byte order!" << endl;

}
