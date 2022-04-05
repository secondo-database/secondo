/*
---- 
This file is part of SECONDO.

Copyright (C) 2007, University in Hagen, 
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


//characters  [1]  verbatim:  [\verb@] [@]


Feb. 2007, M. Spiekermann: Documentation of the Client-Server-Protocol. New
protocol tag __<Message>__ introduced. 


1 Protocol Overview

The client and the server interchange messages as byte sequences which will be
sent over a TCP-IP socket. A message is a simple tag of the pattern "<tag/>"[1]
or a pair of start and end tags "<tag>...</tag>". After each tag a newline
symbol "\n" must be sent. Between a paired tag some message dependent data
will be delivered.


2 Connecting

After the connection is established the server confirms it.

Server:

----
    <SecondoOK/>\n
----

Then the client needs to send an authorization. Currently username and
password are send unencrypted. If authorization is decativated 
(see SecondoConfig.ini) the received information is ignored.

Client:

----
    <Connnect>
    user\n
    password\n
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
The command itself can be wrapped into several lines. Every command which is
known by the SecondoInterface (see "SecondoInterface.h") can be used. Note: The
command cannot contain the line "</Secondo>".

The response will be a list which may be returned in text or binary
representation.

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


4 Retrieving operator information

It is possible to ask Secondo for some unique identificators of an 
operator value mapping. The input is the operator's name and a list 
containing the arguments.

----
   <startGetOperatorIndexes>
     opname\n
     list\n
   <endGetOperatorIndexes>
----

The respose to this query are three numbers identifying the value map. and a 
nested list represening the result type.

----
   <startGetOperatorIndexesResponse>
     int\n
     int\n
     int\n
     list\n
   <endGetOperatorIndexesResponse>
----


5 CostEstimations

5.1 GetCosts

The client may ask for estimating costs for an operator.

----
  <GETCOSTS>\n
    nostreams\n
    algId\n
    opId\n\
    funId\n
   ( noTuples\n
    sizeOfTuple\n ) ^ nostreams
    memory\n
  </GETCOSTS>\n
----

All values are integer values. The first integer (nostreams) determines
how many noTuples/sizeOfTuple pairs are sent to the kernel. 

The server will answer by:

----
  <COSTRESPONSE>\n
     success\n
     cost\n
  </COSTRESPONSE>\n
----

success and cost are returned as integer values. 


6 Get Linear Cost Function

Client's request

----   
  <GETLINEARCOSTFUN>\n
     nostreams\n
     algId\n
     opId\n
     funId\n
    ( notuples\n
      sizeofTuple\n ) ^ nostreams
  </GETLINEARCOSTFUN>\n
----

Server's answer:

----
   <LINEARCOSTFUNRESPONSE>\n
       success\n
       sufficientMemory\n
       timeAtSuffMemory\n
       timeAt16MB\n  
   </LINEARCOSTFUNRESPONSE>\n
----

All returned values are doubles.



7 Get detailed cost functions

Client's request

----   
  <GETCOSTFUN>\n
     nostreams\n
     algId\n
     opId\n
     funId\n
    ( notuples\n
      sizeofTuple\n ) ^ nostreams
  </GETCOSTFUN>\n
----

Server's answer:

----
   <COSTFUNRESPONSE>\n
       success\n
       funType\n
       sufficientMemory\n
       timeAtSuffMemory\n
       timeAt16MB\n  
       a\n
       b\n
       c\n
       d\n
   </COSTFUNRESPONSE>\n
----

The funType is an integer values, all other values are doubles.



4 Catalog Information

Deprecated! Will be removed in future versions.

These kinds of client requests are very special. Sometimes it may
be necessary to get internal information about a specific type, 
if so the messages below are needed.

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
site. All of the requests below will return a "<SecondoResponse>" message.


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

The values of "dbName", "objName" and "fileName" are strings. The value
 "N" indicates the file size in bytes followed by the bytes of the file.

6 File Transfer 

For transfer a file from client to the server (for example for importing it),
 the client sends

----
   <FileTransfer>\n
   filename\n
----

Depending wether overwriting of files is allowed or not, the next line 
sent to the server is.

----
   <ALLOW_OVERWRITING>\n
----

or
  
----
  <DISALLOW_OVERWRITING>\n
----

The answer of the server in case of an error is.

----
  <SecondoError>\n
   ErrorMessage \n
  </SecondoError>\n
----

If there are no problems up to now, the server answers:

----
  <SecondoOK>
----

In this case, the client sends the file to the server:

----
   <FileData>\n
   N\n
   byte1..byteN
   </FileData>\n
   </FileTransfer>
----

The filename is the name of the file created on server side. N is 
the size of the file.


The reverse way, i.e. requesting a file from the server, works as follows:

The client sends to the server:

----
   <RequestFile>\n
   filename\n
   </RequestFile>
----

The answer of the server is:

----
   <FileData>\n
   N\n
   byte1...byteN
   </FileData>
----

in case of successful access to the file or in case of an error

----
    <SendFileError>\n
----


7 Disconnecting

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

//#define TRACE_ON 1
#undef TRACE_ON
#include "LogMsg.h"
#include "ErrorCodes.h"
#include "NestedList.h"
#include "Messages.h"
#include "TraceMacros.h"
#include "limits.h"
#include "DebugWriter.h"


extern DebugWriter dwriter;


/*
Utility functions
   
*/   

namespace csp {

void
sendList(std::iostream& iosock, NestedList* nl, ListExpr list);

} // end of namespace



class ServerMessage : public MessageHandler {

  std::iostream& iosock;
  const std::string startMessage;
  const std::string endMessage;
  bool ignore;
  
  public:
  virtual bool handleMsg(NestedList* nl, ListExpr msg, 
                         int source __attribute__((unused))) {

   if (ignore) {
     std::cerr << "Warning: Last request was not <Secondo>! "
          << "Message will not be sent to the client." << std::endl
          << startMessage << std::endl 
          << nl->ToString(msg) 
          << endMessage << std::endl;
     return false;
   } 

   //std::cerr << "Sending message ..." << std::endl;
   iosock << startMessage << std::endl;
   csp::sendList(iosock, nl, msg);
   iosock << endMessage << std::endl;
          
   return true;       
  } 

  void Flush(){
    iosock.flush();
  }
  
  ServerMessage(std::iostream& ios) : 
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
 std::iostream& iosock;
 const std::string err;  
 bool ignoreMsg;
 NestedList* nl;
 ServerMessage* msgHandler;
 MessageCenter* msg;
 
 
 public:
 const std::string startFileData;
 const std::string endFileData;
 const std::string startObjectRestore;
 const std::string endObjectRestore;
 const std::string startDbRestore;
 const std::string endDbRestore;
 const std::string startResponse;
 const std::string endResponse;
 const std::string startMessage;
 const std::string endMessage;
 const std::string startError;
 const std::string sendFileError; 
 const std::string startRequestOperatorIndexes;
 const std::string endRequestOperatorIndexes;
 const std::string startOperatorIndexesResponse;
 const std::string endOperatorIndexesResponse;
 const std::string startFileTransfer;
 const std::string endFileTransfer;
 const std::string startRequestFile;
 const std::string endRequestFile;
 
 CSProtocol(NestedList* instance, std::iostream& ios, bool server = false) : 
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
   startError("<Error>"),
   sendFileError("<SendFileError/>"),
   startRequestOperatorIndexes("<REQUESTOPERATORINDEXES>"),
   endRequestOperatorIndexes("</REQUESTOPERATORINDEXES>"),
   startOperatorIndexesResponse("<OPERATORINDEXESRESPONSE>"),
   endOperatorIndexesResponse("</OPERATORINDEXESRESPONSE>"),
   startFileTransfer("<FileTransfer>"),
   endFileTransfer("</FileTransfer>"),
   startRequestFile("<RequestFile>"),
   endRequestFile("</RequestFile>")
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


 ~CSProtocol(){
     msg->RemoveHandler(msgHandler);
     delete msgHandler;
  }
 
   
 void skipRestOfLine()
 {
   iosock.ignore( INT_MAX, '\n' );
 }

 void IgnoreMsg(bool value) 
 { 
    ignoreMsg = value; 
    msgHandler->ignoreMsg(value);
    SHOW(ignoreMsg) 
 }
 
 bool nextLine(const std::string& exp, std::string& errMsg)
 { 
   std::string line="";
   getline( iosock, line );
  
   if ( line != exp ) {
     errMsg = err + exp + " expected! But got \"" + line + "\"\n";
     std::cerr << errMsg << std::endl;
     return false;
   }
   //std::cerr << "line: \"" << line << "\"" << std::endl; 
   return true;  
 }

  
 bool SendFile(const std::string& filename) {

  std::string line = "";  
  
  //cout << "Begin SendFile()" << std::endl;
  //cout << "Transmitting file: " << filename;

  std::ifstream restoreFile( filename.c_str(), std::ios::binary );
  if ( ! restoreFile ){
     iosock <<  sendFileError << std::endl;
     return false;
  }
  
  // send begin file sequence
  iosock << startFileData << std::endl;
  
  try {

    {
      const unsigned int bufSize =512;
      char buf[bufSize];       
      restoreFile.seekg (0, restoreFile.end);
      uint64_t length = restoreFile.tellg();
      restoreFile.seekg (0, restoreFile.beg);

      // send file size
      iosock << length << std::endl;         
      // cout << "SendFile: file size: " << length <<  " bytes." << std::endl;
      
      // send file data
      //uint64_t read2 = 0;
      while (!restoreFile.eof() && !iosock.fail())
      {
        restoreFile.read(buf, bufSize);
        unsigned int read = restoreFile.gcount();
        //read2 += read;
        iosock.write(buf, read);
      }
      //cout << "SendFile: transmitted " 
      //     << read2 <<  " bytes to the server." << std::endl;

      restoreFile.close();
      iosock.flush();
    }
    // send end sequence => empty file;
    iosock << endFileData << std::endl;
    iosock.flush();

  } catch (std::ios_base::failure&) {
     std::cerr << std::endl 
          << "Caught exception: I/O error on socket stream object!"
          << std::endl;
     return false;
  }   

  //cout << "End SendFile()" << std::endl;
  return true;
}       

bool ReceiveFile( const std::string& localFileName )
{
  
  std::string errMsg = "";
  std::string line="";
  getline(iosock, line);
  if(line == sendFileError ){ 
     return false;
  }
  if(line != startFileData){
    // protocol error
    return false;
  }

  // read file size
  uint64_t size = 0;
  iosock >> size;    
  skipRestOfLine();
  // cout << "Size: " << size << std::endl;
  
  std::ofstream localFile;
  localFile.open( localFileName.c_str(), std::ios::binary );
  
  unsigned int bufSize=512;
  char buf[bufSize];
  size_t calls=0;

  // get data and write them to local file

  while (!iosock.fail() && size)
  {
    if (size < bufSize)
      bufSize = size;       
    iosock.read(buf, bufSize);
    calls++;
    size_t read=iosock.gcount();
    localFile.write(buf, read);
    size -= read; 
  }
  //cout << "Average read bytes per iosock.read(): " 
  //     << (1.0*size2)/calls << std::endl;
  localFile.close();

  // check protool end sequence
  if ( !nextLine(endFileData, errMsg) ) {
    return false;    
  }
    
  return true;
}



bool 
ReadList(const std::string& endTag, ListExpr& resultList, 
         int& errorCode, bool debug, void* caller, 
         int callerID) {

  dwriter.write(debug, cout, caller, callerID, "start ReadList");
  std::string line = "";
  std::string result = "";
  bool success = false;
  if ( !RTFlag::isActive("Server:BinaryTransfer") ) { 
    dwriter.write(debug, cout, caller, callerID, "textual list transfer");
    // textual data transfer
    do {
      getline( iosock, line );
      if ( line != endTag )
      {
        result += line + "\n";
      }
    } while (line != endTag && !iosock.fail()); 
    dwriter.write(debug, cout, caller, callerID, "text received, parse it");
    nl->ReadFromString( result, resultList );
    dwriter.write(debug, cout, caller, callerID, "list parsing finished");
    success = true;
    
  } else { // binary data transfer
    dwriter.write(debug, cout, caller, callerID, "binary list transfer");
    nl->ReadBinaryFrom(iosock, resultList);
    dwriter.write(debug, cout, caller, callerID, "list transfer finished");
    
    //std::ofstream outFile("TTYCS.bnl");
    //nl->WriteBinaryTo(resultList, outFile);
    
    getline( iosock, line );
    dwriter.write(debug, cout, caller, callerID, "end tag read");
    
    if (line != endTag ) 
    {
      dwriter.write(true, std::cerr, caller, callerID, "end tag invalid");
      errorCode = ERR_IN_SECONDO_PROTOCOL;
      resultList = nl->TheEmptyList();
    } 
    else 
    {
      success = true;
    }  
  }
  dwriter.write(debug, cout, caller, callerID, "finished methjod ReadList");
  return success;  
} 


int
ReadResponse( ListExpr& resultList,
              int& errorCode,
              int& errorPos,
              std::string& errorMessage,
              MessageHandler* msgHandler = 0,
              int source  = -1,
              bool debug = false,
              void* caller = 0,
              int callerID = 1)
{

  dwriter.write(debug, cout, caller, callerID, "called ReadResponse");

  // read next line
  std::string line="";
  try{
      getline( iosock, line );
  } catch(std::ios_base::failure& ex){
    
    dwriter.write(debug, std::cerr, caller, callerID, "exception occured");
    std::cerr << "Exception occurred during reading response from server" 
              << std::endl; 
    std::cerr << "Exceptionm is " << ex.what() << std::endl;
    errorCode = ERR_IN_SECONDO_PROTOCOL;
    try{
       iosock.clear();
    } catch(...){
       std::cerr << "clear failed" << endl;
    }
    return errorCode;
  }
  dwriter.write(debug, cout, caller, callerID, "read first line");
  
  bool badbit = iosock.bad();
  bool success = false;

  // read messages if present
  ListExpr messageList = nl->Empty();
  while ( !badbit && line == startMessage ) 
  {
    dwriter.write(debug, cout, caller, callerID, "receive message");
    success = ReadList(endMessage, messageList, errorCode, debug, 
                       caller, callerID);
    dwriter.write(debug, cout, caller, callerID, "message received", success);
    if (success) {
      dwriter.write(debug, cout, caller, callerID, "send message to center");
      msg->Send(nl,messageList, source);
      if(msgHandler){
          msgHandler->handleMsg(nl, messageList, source);
      }
      getline( iosock, line );
      badbit = iosock.bad();
      dwriter.write(debug, cout, caller, callerID, "sending message finished");
    }  
  } 

  // network error 
  if (badbit) {
    errorCode = ERR_IN_SECONDO_PROTOCOL;
    dwriter.write(debug, cout, caller, callerID, "network error");
    return errorCode;
  }   
    
  
  if ( line == startResponse )
  {
    dwriter.write(debug, cout, caller, callerID, "read response list");
   
    success = ReadList(endResponse, resultList, errorCode, 
                       debug, caller, callerID);
    dwriter.write(debug, cout, caller, callerID, "reading list", success);
    
    if (success) 
    {
     errorCode = nl->IntValue( nl->First( resultList ) );
     errorPos  = nl->IntValue( nl->Second( resultList ) );
     dwriter.write(debug, cout, caller, callerID, "extract error");
     TextScan ts = nl->CreateTextScan( nl->Third( resultList ) );
     nl->GetText( ts, nl->TextLength( nl->Third( resultList ) ), 
                  errorMessage );
     nl->DestroyTextScan( ts );
     dwriter.write(debug, cout, caller, callerID, 
                   "error extracted, extract result");
     resultList = nl->Fourth( resultList );
     dwriter.write(debug, cout, caller, callerID, "result extracted");
    }
  }
  else if ( line == startError )
  {
    dwriter.write(debug, cout, caller, callerID, "receive error");
    getline( iosock, errorMessage );
    getline( iosock, line ); // eat up end tag
    errorCode = ERR_IN_SECONDO_PROTOCOL;
    dwriter.write(debug, cout, caller, callerID, "protocol error");
  }
  else
  {
    dwriter.write(debug, cout, caller, callerID, "protocol error");
    errorCode = ERR_IN_SECONDO_PROTOCOL;
  }
  dwriter.write(debug, cout, caller, callerID, "readResponse finsihed");
  return errorCode;
}

};

#endif

