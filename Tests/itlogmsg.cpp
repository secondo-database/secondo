#include <iostream>

#include "LogMsg.h"


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

}
