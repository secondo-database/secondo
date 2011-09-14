
/*
----
This file is part of SECONDO.

Copyright (C) 2007,
Faculty of Mathematics and Computer Science,
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


1 How to write your own Secondo Application in C++


This is a very small example describing how, an C++ programm can
use the secondo interface to execute queries without client server 
communication..



1.1 Includes

*/

#include <string>
#include <iostream>
#include <stdlib.h>
#include "SecondoInterface.h"
#include "NestedList.h"
#include "NList.h"


using namespace std;


/* 
1.2 The Secondo Interface

For a stand alone application it's required to define the SecondoInterface globally 
and to name it si. This is because some code has an __extern__ definition of a
SecondoInterface.

*/

SecondoInterface* si =0;


int main(int argc, char** argv){


   // create an interface to the secondo functionality
   // the paramneter must be false to communicate with secondo directly
   si = new SecondoInterface(false);

   // define the name of the configuration file
   string config = "Config.ini";

   // read in runtime flags from the config file
   si->InitRTFlags(config);



   // set some variables needed to connect with secondo
   // the parameters host and port are ignored when
   // using secondo directly 

  string user="";
  string passwd="";
  string host="";
  string port ="";
  string errMsg;          // return param
  bool   multiUser=false;

  // try to connect
  if(!si->Initialize(user,passwd,host,port,config,errMsg,multiUser)){
     // connection failed, handle error
     cerr << "Cannot initialize secondo system" << endl;
     cerr << "Error message = " << errMsg << endl;
     return  1;
  }
  
  // connected
  cout << "SecondoInterface successfull initialized" << endl;


  
  string command = "";
  
  NestedList* nl = si->GetNestedList();
  NList::setNLRef(nl);
  
   // let the user enter secondo commands until he/she eters quit
  
  while(command!="quit") {
     cout << "Enter Command >";
     getline(cin,command);
     if((command!="quit") && (command.length()>0)){
        cout << "send command " << command << endl;
        ListExpr res = nl->TheEmptyList();  // will contain the result
        SecErrInfo err;                     // will contain error information
       // this is the command to communicate with secondo
        si->Secondo(command, res, err); 

        // check whether command was successful
        if(err.code!=0){ 
          // if the error code is different to zero, an error is occurred
          cout << "Error during command. Error code :" << err.code << endl;
          cout << "Error message = " << err.msg << endl;
        } else {
          // command was successful
          // do what ever you want to de with the result list
          // in this little example, the result is just printed out
          cout << "Command successful processed" << endl;
          cout << "Result is:" << endl;
          cout << nl->ToString(res) << endl << endl;
        }
     }
  }

  // close connection to secondo
  si->Terminate();
  // destroy the interface
  delete si;
  return 0;


}



