#include <iostream>

#include "LogMsg.h"

const int __endian_detect = 1;
 
inline bool __little_endian() { return *(char *)&__endian_detect == 1;}
 

int
main () {

 RTFlag::initByString("hallo,test,bla,blub");

 cout << RTFlag::isActive("hallo") << endl;
 cout << RTFlag::isActive("test") << endl;
 cout << RTFlag::isActive("bla") << endl;
 cout << RTFlag::isActive("blub") << endl;
 cout << RTFlag::isActive("xfindo") << endl;

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

 RTFlag::initByString("");
 RTFlag::initByString(",");
 RTFlag::initByString("xvc");
 cout << RTFlag::isActive("xvc") << endl;
 

}
