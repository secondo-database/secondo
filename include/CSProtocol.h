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

*/


#ifndef SEC_CSProtocol_H
#define SEC_CSProtocol_H

#include <string>
#include <iostream>

#include "ErrorCodes.h"
#include "NestedList.h"
#include "LogMsg.h"

using namespace std;

struct CSProtocol {

 private:
 iostream& iosock;
 const string err;  

 public:
 const string startFileData;
 const string endFileData;
 const string startObjectRestore;
 const string endObjectRestore;
 const string startDbRestore;
 const string endDbRestore;
 //const string errorTag = "<SendFileError/>"; 
 
 CSProtocol(iostream& ios) : 
   iosock(ios), 
   err("Protocol-Error: "),
   startFileData("<FileData>"),  
   endFileData("</FileData>"),
   startObjectRestore("<ObjectRestore>"),
   endObjectRestore("</ObjectRestore>"),
   startDbRestore("<DbRestore>"),
   endDbRestore("</DbRestore>")
 { }
 
 void skipRestOfLine()
 {
   iosock.ignore( INT_MAX, '\n' );
 }

 bool nextLine(const string& exp, string& errMsg)
 { 
   string line="";
   getline( iosock, line );
  
   if ( line != exp ) {
     errMsg = err + exp + " expected! But got \"" + line + "\"\n";
     cerr << errMsg << endl;
     return false;
   }
   cerr << "line: \"" << line << "\"" << endl; 
   return true;  
 }

 bool SendFile(const string& filename) {

  string line = "";  
  
  //cout << "Begin SendFile()" << endl;
  cout << "Transmitting file: " << filename;
  
  // send begin file sequence
  iosock << startFileData << endl;
  
  try {

    ifstream restoreFile( filename.c_str() );
    if ( restoreFile )
    {
      const int bufSize =512;
      static char buf[bufSize];       
      int read = 0;

      // compute file size
      do { 
        restoreFile.read(buf, bufSize);
        read += restoreFile.gcount();

      } while ( restoreFile.good() );

      // send file size
      iosock << read << endl;         
      cout << " (" << read <<  " bytes)" << endl;
      
      ifstream restoreFile2( filename.c_str() );
      
      // send file data
      while (!restoreFile2.eof() && !iosock.fail())
      {
        restoreFile2.read(buf, bufSize);
        int read = restoreFile2.gcount();
        iosock.write(buf, read);
      }
      restoreFile2.close();
      
      cout.flush();
      iosock.flush();
    }
    // send end sequence => empty file;
    iosock << endFileData << endl;

  } catch (ios_base::failure) {
     cerr << endl 
          << "Caught exception: I/O error on socket stream object!"
          << endl;
     return false;
  }   

  //cout << "End SendFile()" << endl;
  return true;
}       

bool ReceiveFile( const string& serverFileName )
{
  //cout << "Begin ReceiveFile()" << endl;
  
  string errMsg = "";
  if ( !nextLine(startFileData, errMsg) )
    return false;
  
  // read file size
  int size = 0;
  iosock >> size;    
  skipRestOfLine();
  cout << "Size: " << size << endl;
  
  ofstream serverFile( serverFileName.c_str() );
  
  static int bufSize=512;
  char buf[bufSize];
  int calls=0;
  int size2=size;
  while (!iosock.fail() && size)
  {
    if (size < bufSize)
      bufSize = size;       
    iosock.read(buf, bufSize);
    calls++;
    int read=iosock.gcount();
    serverFile.write(buf, read);
    size -= read; 
  }
  cout << "Average read bytes per iosock.read(): " << (1.0*size2)/calls << endl;
  serverFile.close();

  // check protool end sequence
  if ( !nextLine(endFileData, errMsg) )
    return false;    
    
  //cout << "End ReceiveFile()" << endl;
  
  return true;
}

int
ReadResponse( NestedList* nl, 
              ListExpr& resultList,
              int& errorCode,
              int& errorPos,
              string& errorMessage )
{
    string result = "";
    string line="";
    
    getline( iosock, line );
    if ( line == "<SecondoResponse>" )
    {
      bool success = false;
      
      if ( !RTFlag::isActive("Server:BinaryTransfer") ) { 
        // textual data transfer
        do
        {
          getline( iosock, line );
          if ( line != "</SecondoResponse>" )
          {
            result += line + "\n";
          }
        }
        while (line != "</SecondoResponse>" && !iosock.fail());      
        nl->ReadFromString( result, resultList );
        success = true;
        
      } else { // binary data transfer
      
        nl->ReadBinaryFrom(iosock, resultList);
        ofstream outFile("TTYCS.bnl");
        nl->WriteBinaryTo(resultList, outFile);
        getline( iosock, line );
        if (line != "</SecondoResponse>" ) {
          cerr << "Error: No </SecondoResponse> found after "
               << "receiving a binary list!" << endl;
          errorCode = ERR_IN_SECONDO_PROTOCOL;
          resultList = nl->TheEmptyList();
        } else {
          success = true;
        }  
      }  
        if (success) {
          errorCode = nl->IntValue( nl->First( resultList ) );
          errorPos  = nl->IntValue( nl->Second( resultList ) );
          TextScan ts = nl->CreateTextScan( nl->Third( resultList ) );
          nl->GetText( ts, nl->TextLength( nl->Third( resultList ) ), 
                       errorMessage );

          nl->DestroyTextScan( ts );
          resultList = nl->Fourth( resultList );
        }
    }
    else if ( line == "<SecondoError>" )
    {
      errorCode = ERR_IN_SECONDO_PROTOCOL;
      getline( iosock, errorMessage );
      getline( iosock, line );
    }
    else
    {
      errorCode = ERR_IN_SECONDO_PROTOCOL;
    }
    return errorCode;
}


 
};

#endif

