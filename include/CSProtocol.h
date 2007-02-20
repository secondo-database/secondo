/*
---- 
This file is part of SECONDO.

Copyright (C) 2007, University in Hagen, Faculty of Mathematics and Computer Science, 
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

Feb. 2007, M. Spiekermann: Documentation of the Client-Server-Protocol. New protocol tag
__<Message>__ introduced. 


1 Protocol Overview

The client and the server interchange messages as byte sequences which will be sent over
a TCP-IP socket. A message is a simple tag of the pattern __<tag/>__ or a pair of
start and end tags __<tag>...</tag>__. After each tag a newline symbol __\n__ must be
sent. Between a paired tag some message dependet data will be delivered.


2 Connecting

After the connection is established the server confirms it.

Server:

----
    <SecondoOK/>\n
----

Then the client needs to send an authorization, currently the user and password will
be ignored since there is no user management implemented yet.

Client:

----
    <Connnect>
    ((user)(passwd))\n
    </Connect>
----

Again, the server will confirm this.


Server:

----
    <SecondoIntro>\n
    You are connected with a Secondo server.\n
    </SecondoIntro>\n
----

or returns an error, which will be sent in a single line. In the future there
could be send more specific information about a server. The error message does
not contain the error of a specific secondo command instead it indicates a
protocol error.

Server:

----
    <SecondoError>\n
    message\n
    </SecondoError>\n
----

Afterwards ther server waits for client requests.

3 Running Commands

Afterwards the Client can send one of the requests explained below.
The most interesting one is to send a Secondo command to the server.

----
    <Secondo>\n 
    cmdLevel\n 
    line1\n  
    ...
    lineN\n
    </Secondo>\n
----

"cmdLevel" is an integer, it values could be 0 (list-syntax) or 1 (SOS-syntax).
The command itself can be wrapped into several lines. Every command which is known
by the SecondoInterface (see "SecondoInterface.h") can be used. Note: The command cannot
contain the line "</Secondo>".

The response will
be a list which may be returned in text or binary representation.

----
    <SecondoResponse>\n
    line1\n
    ....
    lineN\n  
    </SecondoResponse>\n
----

or

----
    <SecondoResponse>\n
    byte1 ... byteN
    </SecondoResponse>\n
----

Depending on the response mode (textual or binary) the client must convert the
result into a nested list data structure. The encoding of binary lists is
documented in the "Documents" directory. Binary lists are smaller and faster to
analyze (no parsing is needed).  The list itself is structured as explained in
"SecondoInterface.h".


After a "<Secondo>" request the server may send some information to the client.
These contain a list of any structure. The client (in particular the
  implementation of Class ~MessageHandler~) has to care about it.

----
    <Message>
    list\n
    </Message>
----



4 Catalog Information

Deprecated! Will be removed in future versions.

These kinds of client requests are very special. Sometimes it may be necessary to
get internal information about a specific type, if so the messages below are needed.

Please refer to the implementation for further details.  

----
<NumericType>\n 
  type (string)\n
</NumericType>\n

<NumericTypeResponse>\n 
  outlist (textual list)
</NumericTypeResponse>\n 
----

----
<GetTypeId>\n 
  type (string)\n
</GetTypeId>\n

<GetTypeIdResponse>\n 
  algebraId TypeId (two int values separated by a space)\n
</GetTypeIdResponse>\n 
----

----
<LookUpType>\n 
  typeExpression (textual list)\n
</LookUpType>\n

<LookUpTypeResponse>\n
  ((name) algebraId typeId) (textual list)\n
</LookUpTypeResponse>\n
----

5 Save and Restore

It is possible to interchange objects or databases between the client and the
server site. Hence you can use the client to restore objects or databases or to
save object or databases. Some special messages are needed since the usual
Secondo-commands for this purpose assume that the files are on the server's
site. All of the requests below will return a __<SecondoResponse>__ message.


The save requests will return a Secondo result list which is the list
representation of an object or database.

----
<ObjectSave>\n
  name (string)\n
</ObjectSave>\n

<DbSave/>
----

The restore requests will return the ~normal~ result lists of the corresponding
Secondo commands. These requests need to transmit a file to the server. 

----
    <DbRestore>
    dbName\n
    filename\n
    <FileData>\n
    N\n
    byte1 ... byteN
    </FileData>\n
    </DbRestore>\n
----

----
    <ObjectRestore>
    objName\n
    filename\n
    <FileData>\n
    N\n
    byte1 ... byteN
    </FileData>\n
    </ObjectRestore>\n
----

The values of "dbName", "objName" and "fileName" are strings. The value "N" indicates the
file size in bytes followed by the bytes of the file.

6 Disconnecting

The client can close the connection by sending

----
<Disconnect/>/n
----

*/


#ifndef SEC_CSProtocol_H
#define SEC_CSProtocol_H

#include <string>
#include <iostream>
#include <list>

#include "ErrorCodes.h"
#include "NestedList.h"
#include "Messages.h"
#include "NList.h"
#include "LogMsg.h"
#include "SecondoSystem.h"

using namespace std;

/*
Utility functions
   
*/   

namespace csp {

void
sendList(iostream& iosock, NList list);

} // end of namespace



class ServerMessage : public MessageHandler {

  iostream& iosock;
  const string startMessage;
  const string endMessage;
  bool ignore;
  
  public:
  virtual bool handleMsg(NList msg) {

   if (ignore) {
     cerr << "Warning: Last request was not <Secondo>! "
          << "Message will not be sent to the client." << endl
          << startMessage << endl 
          << msg.str() << endMessage << endl;
     return false;
   } 

   //cerr << "Sending message ..." << endl;
   iosock << startMessage << endl;
   csp::sendList(iosock, msg);
   iosock << endMessage << endl;
          
   return true;       
  } 
  
  ServerMessage(iostream& ios) : 
   iosock(ios),
   startMessage("<Message>"),
   endMessage("</Message>"),
   ignore(true)
  {}
   
  ~ServerMessage() {}
 
  void ignoreMsg(bool value) { ignore = value; }
  
}; 



/*
4 struct CSProtocol

*/

struct CSProtocol {

 private:
 iostream& iosock;
 const string err;  
 bool ignoreMsg;
 NestedList* nl;
 ServerMessage* msgHandler;
 MessageCenter* msg;
 
 
 public:
 const string startFileData;
 const string endFileData;
 const string startObjectRestore;
 const string endObjectRestore;
 const string startDbRestore;
 const string endDbRestore;
 const string startResponse;
 const string endResponse;
 const string startMessage;
 const string endMessage;
 const string startError;
 //const string errorTag = "<SendFileError/>"; 
 
 CSProtocol(NestedList* instance, iostream& ios, bool server = false) : 
   iosock(ios), 
   err("Protocol-Error: "),
   startFileData("<FileData>"),  
   endFileData("</FileData>"),
   startObjectRestore("<ObjectRestore>"),
   endObjectRestore("</ObjectRestore>"),
   startDbRestore("<DbRestore>"),
   endDbRestore("</DbRestore>"),
   startResponse("<SecondoResponse>"),
   endResponse("</SecondoResponse>"),
   startMessage("<Message>"),
   endMessage("</Message>"),
   startError("<Error>")
 {
   ignoreMsg = true;
   nl = instance;
   msg = MessageCenter::GetInstance();

   // The message handler will send Secondo runtime messages
   // to the server.
   msgHandler = new ServerMessage(ios);
   if (server)
     msg->AddHandler(msgHandler);
   msgHandler->ignoreMsg(true);
 }
 
   
 void skipRestOfLine()
 {
   iosock.ignore( INT_MAX, '\n' );
 }

 void IgnoreMsg(bool value) 
 { 
    ignoreMsg = value; 
    msgHandler->ignoreMsg(value);
    cerr << "IgnoreMsg: " << value << endl; 
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

 /*
 bool SendMessage(ListExpr list) {

   if (ignoreMsg) {
     cerr << "Warning: Last request was not <Secondo>! "
          << "Message will not be sent to the client." << endl
          << startMessage << endl 
          << list << endMessage << endl;
     return false;
   } 

   iosock << startMessage << endl;
   csp::sendList(nl,list);
   iosock << endMessage << endl;
          
   return true;       
 } 
 */
  
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



bool 
ReadList(const string& endTag, ListExpr& resultList, int& errorCode) {

  string line = "";
  string result = "";
  bool success = false;
  if ( !RTFlag::isActive("Server:BinaryTransfer") ) { 
    // textual data transfer
    do
    {
      getline( iosock, line );
      if ( line != endTag )
      {
        result += line + "\n";
      }
    }
    while (line != endTag && !iosock.fail());      
    nl->ReadFromString( result, resultList );
    success = true;
    
  } else { // binary data transfer
  
    nl->ReadBinaryFrom(iosock, resultList);
    ofstream outFile("TTYCS.bnl");
    nl->WriteBinaryTo(resultList, outFile);
    getline( iosock, line );
    
    if (line != endTag ) 
    {
      cerr << "Error: No " << endTag << " found after "
           << "receiving a binary list!" << endl;
      errorCode = ERR_IN_SECONDO_PROTOCOL;
      resultList = nl->TheEmptyList();
    } 
    else 
    {
      success = true;
    }  
  }
  return success;  
} 


int
ReadResponse( ListExpr& resultList,
              int& errorCode,
              int& errorPos,
              string& errorMessage )
{
  // read next line
  string line="";
  getline( iosock, line );
  
  bool badbit = iosock.bad();
  bool success = false;

  // read messages if present
  ListExpr messageList = nl->Empty();
  while ( !badbit && line == startMessage ) 
  {
    success = ReadList(endMessage, messageList, errorCode);
    if (success) {
      msg->Send(messageList);
      getline( iosock, line );
      badbit = iosock.bad();
    }  
  } 

  // network error 
  if (badbit) {
    errorCode = ERR_IN_SECONDO_PROTOCOL;
    return errorCode;
  }   
    
  
  if ( line == startResponse )
  {
    success = ReadList(endResponse, resultList, errorCode);
    
    if (success) 
    {
     errorCode = nl->IntValue( nl->First( resultList ) );
     errorPos  = nl->IntValue( nl->Second( resultList ) );
     TextScan ts = nl->CreateTextScan( nl->Third( resultList ) );
     nl->GetText( ts, nl->TextLength( nl->Third( resultList ) ), 
                  errorMessage );

     nl->DestroyTextScan( ts );
     resultList = nl->Fourth( resultList );
    }
  }
  else if ( line == startError )
  {
    getline( iosock, errorMessage );
    getline( iosock, line ); // eat up end tag
    errorCode = ERR_IN_SECONDO_PROTOCOL;
  }
  else
  {
    errorCode = ERR_IN_SECONDO_PROTOCOL;
  }
  return errorCode;
}

};

#endif

