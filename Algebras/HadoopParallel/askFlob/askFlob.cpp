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


1 Ask a remote Secondo Server to process a genFlobResult query


Using the C++ interface provided for connecting an Secondo server



1.1 Includes

*/

#include <string>
#include <iostream>
#include <stdlib.h>
#include "SecondoInterface.h"
#include "NestedList.h"
#include "NList.h"


using namespace std;


int main(int argc, char** argv){
  
  if (argc != 8){
    cerr << "Usage: " << argv[0] << " needs the following parameters" << endl;
    cerr << "  host:\tstring" << endl;
    cerr << "  port:\tint" << endl;
    cerr << "  localConfig\tstring" << endl;
    cerr << "  dbName:\tstring" << endl;
    cerr << "  source:\tint" << endl;
    cerr << "  sheetFile:\tstring" << endl;
    cerr << "  resultFile:\tstring" << endl << endl; 

    return -1;
  }
    
  string host = argv[1];
  string port = argv[2];
  string localConfig = argv[3];
  string dbName = argv[4];
  string source = argv[5];
  string sheetFile = argv[6];
  string resultFile = argv[7];

  // create an interface to the secondo server
  // the paramneter must be true to communicate as client
  SecondoInterface* si = new SecondoInterface(true);

  // read in runtime flags from the config file
  si->InitRTFlags(localConfig);



  // set some variables needed to connect with the server
  string user="";
  string passwd="";
  string errMsg;          // return param
  bool   multiUser=true;

  // try to connect
  if(!si->Initialize(user,passwd,host,port,localConfig,errMsg,multiUser)){
    // connection failed, handle error
    cerr << "Cannot initialize secondo system" << endl;
    cerr << "Error message = " << errMsg << endl;
    return  -1;
  }
  
  // connected
  cout << "SecondoInterface successfull initialized" << endl;
 
  string command = "";
  
  NestedList* nl = si->GetNestedList();
  NList::setNLRef(nl);
  ListExpr res = nl->TheEmptyList();  // will contain the result
  SecErrInfo err;                     // will contain error information
  //process several constant queries to get the flob file 
  
  //open the database
  command = "open database " + dbName;
  si->Secondo(command, res, err);
  if(err.code != 0){
    //In case the database does not exist
    command = "create database " + dbName;
    si->Secondo(command, res, err);
    if (err.code != 0){
      cerr << "Create the database " << dbName << " fails!" << endl;
      return -1;
    }
    command = "open database " + dbName;
    si->Secondo(command, res, err);
    if (err.code != 0){
      cerr << "Cannot open the database " << dbName << " fails!" << endl;
      return -1;
    }
  }

  //execute the genFlobResult
  command = "query genFlobResult( " + source + ", \""
    + sheetFile + "\", \"" + resultFile + "\")";
  si->Secondo(command, res, err);
  if (err.code != 0){
    cerr << "Process the genFlobResult query fails " << endl;
    cerr << "Error during command. Error code :" << err.code << endl;
    cerr << "Error message = " << err.msg << endl;
  } else {
    cerr << "Result is: " << nl->ToString(res) << endl;
  }

  command = "close database";
  si->Secondo(command, res, err);
  
  // close connection
  si->Terminate();
  // destroy the interface
  delete si;
  return 0;


}



