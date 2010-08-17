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

//paragraph [1] title: [{\Large \bf ] [}]
//[->] [$\rightarrow$]



[1] RemoteStream Algebra

August 2005 GZS

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

May 2010 Jiamin Lu use binary form to transport the tuples,
replace the original method of using strings to transfer the nestedLists
of the tuples.

1 RemoteStream Algebra

This algebra allows distributed query processing by connecting streams 
sockets.

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SocketIO.h"
#include "RelationAlgebra.h"
#include "../FText/FTextAlgebra.h"
#include "ListUtils.h"
#include "Symbols.h"
#include "Serialize.h"
#include <string>
#include <iostream>   //for testing
#include <cmath>

#define TRACE_ON

extern NestedList* nl;
extern QueryProcessor *qp;

/*

2 Overview

This Algebra has two operators: ~send~ and ~receive~

The ~send~ operator expects a stream and a port number, and start to listen to
that port. It returns the number of tuples sent.

  * stream(tuple) port [->] int       send

The port number must be in the format pNNNN. For example, to send
data from rel ~ten~ through port ~1032~ we write

  query ten feed send\[p1032\]

The ~receive~ operator expetcs a hostname and a port, and produces a stream:

  * hostname port [->] stream(tuple)      receive

If the hostname start with "ip\_", means this is an IP address.
Besides, since the symbol type can't include "." character,
use "\_" to replace it.
For example, to receive data from ip: 192.168.0.1
through prot ~1032~ we write

  query receive(ip\_192\_168\_0\_1, p1032) consume

2.1 Type Mapping Function

Type mapping for ~send~ is

----	((stream tuple) int) -> (int)
----

Type mapping for ~receive~ is

----	(hostname port) -> (stream tuple)
----

*/

ListExpr
TSendTypeMap(ListExpr args)
{
  ListExpr first, second;
  string argstr;

  int len = nl->ListLength(args);
  if (len != 2 && len != 3)
  {
    ErrorReporter::ReportError(
        "Operator send expect a list of two or three arguments");
    return nl->TypeError();
  }

  first = nl->First(args);
  nl->WriteToString(argstr, first);
  if(! listutils::isTupleStream(first))
  {
    ErrorReporter::ReportError(
        "Operator send expects a list with structure "
        "((stream (tuple ((a1 t1)...(an tn)))) port)\n"
        "Operator count gets a list with structure '"
        + argstr + "'.");
    return nl->TypeError();
  }

  second = nl->Second(args);
  if (! listutils::isSymbol(second))
  {
    ErrorReporter::ReportError(
        "Operator send expects as second argument a "
        "symbol atom (port number p9999)." );
    return nl->TypeError();
  }

  int index = 0;
  string AppendAttrType = "";
  ListExpr attrType = nl->Empty();
  if (len == 3)
  {
    //Check the third argument should be an attribute name;
    ListExpr third;
    third = nl->Third(args);
    if (! listutils::isSymbol(third))
    {
      ErrorReporter::ReportError(
          "Operator send expects the key attribute name be a "
          "symbol atom." );
      return nl->TypeError();
    }

    string name = nl->SymbolValue(third);
    index = listutils::findAttribute(
        nl->Second(nl->Second(first)), name, attrType);
    if (index == 0)
    {
      ErrorReporter::ReportError(
          "Attrbute name " + name + " not found in attribute list." );
      return nl->TypeError();
    }

    if (! listutils::isSymbol(attrType))
    {
      ErrorReporter::ReportError(
          "Attrbute name " + name + " should be SymbolType" );
      return nl->TypeError();
    }

    stringstream ss;
    ss << "APPEND " << nl->ToString(attrType);
    AppendAttrType = ss.str();
  }

  string port = nl->SymbolValue(second).substr(1),
         streamType = nl->ToString(first),
         tupleType = nl->ToString(nl->Second(first));

  Socket *gate = Socket::CreateGlobal( "localhost", port ),
         *client;

  if (!gate || !gate->IsOk())
    {
      ErrorReporter::ReportError(
          "Unable to listen to port." );
      return nl->TypeError();
    }

  client = gate->Accept();

  if (!client || !client->IsOk())
  {
    ErrorReporter::ReportError(
        "Unable to connect with client." );
    return nl->TypeError();
  }

  client->GetSocketStream() << streamType << AppendAttrType << endl;
  string rtnInfo;
  getline(client->GetSocketStream(), rtnInfo);
  if (rtnInfo != "<GET TYPE/>")
  {
    ErrorReporter::ReportError(
        "Unable to get return type info from client");
    return nl->TypeError();
  }


  // Warning - the client pointer is not being deleted.

  gate->Close();
  delete gate;

  int sd = client->GetDescriptor();

  return nl->ThreeElemList(
           nl->SymbolAtom("APPEND"),
           nl->ThreeElemList(nl->IntAtom(index),
                             nl->StringAtom(nl->ToString(attrType)),
                             nl->IntAtom(sd)),
           nl->SymbolAtom("int"));
}

ListExpr
TReceiveTypeMap(ListExpr args)
{
  ListExpr first, second;

  CHECK_COND(nl->ListLength(args) == 2,
    "Operator receive expects a list of length two.");

  first = nl->First(args);
  CHECK_COND( nl->IsAtom(first) && nl->AtomType(first) == SymbolType,
    "Operator receive expects as first argument a symbol "
    "atom (host name, p.e. localhost)." );

  second = nl->Second(args);
  CHECK_COND( nl->IsAtom(second)
      && nl->AtomType(second) == SymbolType,
    "Operator send expects as second argument a symbol "
    "atom (port number, p.e. p9999)." );

  string host = nl->SymbolValue(first),
         port = nl->SymbolValue(second).substr(1),
         streamTypeStr;

  //Accept ip address
  string ipPrefix = "ip_";
  if (!host.compare(0, ipPrefix.size(), ipPrefix))
  {
    host = host.substr(ipPrefix.size());
    host = replaceAll(host, "_", ".");
  }

  cout << "Host: " << host << ":" << port << endl;
  Socket *client =
      Socket::Connect( host, port, Socket::SockGlobalDomain );


  CHECK_COND( client && client->IsOk(),
      "Unable to connect to server." );

  getline( client->GetSocketStream(), streamTypeStr );
  client->GetSocketStream() << "<GET TYPE/>" << endl;

  string keyTypeName = "";
  string::size_type loc = streamTypeStr.find("APPEND",0);
  if (loc != string::npos)
  {
    keyTypeName = streamTypeStr.substr(loc + 7);
    streamTypeStr = streamTypeStr.substr(0, loc);

    if ((keyTypeName != "")
        && (keyTypeName != "int")
        && (keyTypeName != "string")
        && (keyTypeName != "bool")
        && (keyTypeName != "real"))
    {
      ErrorReporter::ReportError("Error key attribute type");
      return nl->SymbolAtom("typeerror");
    }
  }

  ListExpr streamType;
  nl->ReadFromString(streamTypeStr, streamType);

  // Warning - the client pointer is not being deleted.

  return nl->ThreeElemList(
           nl->SymbolAtom("APPEND"),
           nl->TwoElemList(
               nl->StringAtom(keyTypeName),
               nl->IntAtom(client->GetDescriptor())),
           streamType);
}


/*
3 Value Mapping Function

Though the class Socket defined inside Secondo offers an iostream
object to transform data, and makes the communication very easy,
this method needs the sender to
invoke Tuple class's ~Out~ function to transfer all tuples
into nestedLists, then the receiver can use the ~In~ function
to rebuild all the tuples.
Since the transformation is inefficient and unnecessary,
we decided to improve it by transporting tuples' binary data directly.

In the sender side, we create a public memory buffer
called tuple block with 64KB size,
which is the maximum size of a tuple. Then all the tuples got from
the predecessor operator will be transform into binary form by
using ~WriteToBin~ method, and filled into the tuple block.
The buffer will be sent out when its left size is smaller than the
latest tuple. Since one socket package's size is only 1KB,
the tuple block need to be divided into several socket packages,
and send them continuously.

Each socket package contains two head data:

  * sock\_ID: The unique identifier of this socket.

  * sock\_Num: Shows successive socket amount.

The ~sock\_ID~ is used to synchronize the send and receive operation
between two sides. Its value starts from -1. Sender writes this
value at the head position of each socket package, then the receiver
will increase this value and send it back when it gets the package,
to indicate the operation is finished.

Since the sender's buffer could be divided up into 64 socket packages,
the ~sock\_Num~ is used to indicate the order of these packages.
~Sock\_Num~ follows reverse order, the first values means
how many packages the current buffer is divided into.
Then the ~sock\_Num~ will be decreased one by one.
The package with ~sock\_Num~ as 1 means this is
the last package of the tuple block.

In the receiver side, we receive the socket packages continuously,
and put only the tuples' binary data into a memory buffer
also with 64KB size. When a package with ~sock\_Num~ as 1 is got,
then transform these binary data back into tuples
and put them into a tuple buffer,
The next socket package will be received when the tuple buffer is empty.

At the same time, by connecting RemoteStreamAlgebra with Hadoop system,
we enhance the operators by putting an indicated key attribute
value before each tuple while sending them,
so the Java program in Hadoop can read
the key attribute values, and partition the tuples' binary form
without reading the tuple themselves.

*/

//static const u_int32_t MAX_TUPLESIZE = 65535; //Copy from class Tuple
static const u_int32_t MAX_TOTALTUPLESIZE = 655359;
//(10*2^16 - 1) including huge flobs

static const u_int32_t SOCKET_SIZE = 1024;  //Copy from StreamBuffer
static const u_int32_t SOCKET_METASIZE = 2 * sizeof(u_int32_t);
const u_int32_t SOCKTUP_SIZE = SOCKET_SIZE - SOCKET_METASIZE;

void sendSocket(Socket* client, char* buf,
      int32_t* sock_ID, int32_t sock_Num)
{
  TRACE_ENTER
  //Set sock_ID and sock_Num
  memcpy(buf, sock_ID, sizeof(int32_t));
  memcpy(buf + sizeof(int), &sock_Num, sizeof(int32_t));

  if(!client->Write(buf, SOCKET_SIZE))
  {
    string errMsg = client->GetErrorText();
    cerr << "Sender write socket failed: " << errMsg << endl;
    return;
  }
  else
  {
    int32_t replySockID = 0;
    do
    {
      if(!client->Read(buf, sizeof(int32_t)))
      {
        string errMsg = client->GetErrorText();
        cerr << "Sender read socket failed: " << errMsg << endl;
        return;
      }
      memcpy(&replySockID, buf, sizeof(int32_t));
    }while(replySockID != (*sock_ID+1));
    // return until the package is received
  }

  (*sock_ID)++;
  TRACE_LEAVE
}

int32_t receiveSocket(Socket* client, char* buf,
                  int32_t* sock_ID)
{
  //Read one socket, remove the socket info,
  //and put tuple value to allocated buffer
  char sockBuffer[SOCKET_SIZE];
  const int32_t expectSockID = (*sock_ID);
  int32_t get_SockID = expectSockID - 1;
  int32_t sockNum = -1;

  while(true)
  {
    //Make sure get\_SockID is read from the socket.
    get_SockID = expectSockID - 1;
    memcpy(sockBuffer, &get_SockID, sizeof(int32_t));

    if(!client->Read(sockBuffer, SOCKET_SIZE))
    {
      string errMsg = client->GetErrorText();
      cerr << "Receiver read socket failed: " << errMsg << endl;
      return -2;    //Indicate that read socket error
    }

    memcpy(&get_SockID, sockBuffer, sizeof(int32_t));
    if (get_SockID == expectSockID)
    {
      //Read the sock\_Num of this socket
      memcpy(&sockNum, sockBuffer + sizeof(int32_t), sizeof(int32_t));
      //Read tuple value to allocated buffer
      memcpy(buf, sockBuffer + SOCKET_METASIZE, SOCKTUP_SIZE);

      //Return the successive signal
      (*sock_ID)++;
      memcpy(sockBuffer, sock_ID, sizeof(int32_t));
      if(!client->Write(sockBuffer,sizeof(int32_t)))
      {
        string errMsg = client->GetErrorText();
        cerr << "Receiver write socket failed: " << errMsg << endl;
        return -3;  //Indicate that write back socket error
      }
      return sockNum;    
    }
  }
}

void sendTupleBlock(Socket* client, char* buf,
                    int32_t* sock_ID, u_int32_t curPos)
{
  u_int32_t offset = SOCKET_METASIZE;
  int sock_Num =
      ceil((float(curPos - SOCKET_METASIZE)) / SOCKTUP_SIZE);
  //The amount of the sockets need to be sent for this buffer

  while(offset < curPos)
  {
    sendSocket(client, buf + (offset - SOCKET_METASIZE),
        sock_ID, sock_Num);
    sock_Num--;
    offset += SOCKTUP_SIZE;
  }

  assert(sock_Num == 0);
}


int
TSendStream(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  Word elem;
  SocketDescriptor sd;
  bool haveKey = false;
  int keyIndex = 0;
  string keyTypeName;
  u_int32_t keySize = 0;
  if (qp->GetNoSons(s) == 6)
  {
    haveKey = true;
    keyIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;
    keyTypeName =
        (string)(char*)((CcString*)args[4].addr)->GetStringval();

    if ("int" == keyTypeName)
      keySize = sizeof(int32_t);
    else if ("string" == keyTypeName)
      keySize = 0; //The size depends on the value
    else if ("bool" == keyTypeName)
      keySize = sizeof(bool);
    else if ("real" == keyTypeName)
      keySize = sizeof(double);  //the size of double in 64 platform ??
    else
    {
      cerr << "ERROR: unknown storage type for attribute No "
          << keyIndex << endl;
      assert(false);
    }

/*
Since the RemoteStreamAlgebra need contact with Java program,
so the key attribute value type can only be one of the four
basic data types: int, real, bool and string.
If the communication happens between two Secondo system,
then the key attribute value type can be any kind.
The method of getting the binary size of different Secondo data type
is shown below:

*/
      //Get the storageType of the key attribute
//    Attribute::StorageType st;
//    ListExpr streamTupleTypeNL =
//        SecondoSystem::GetCatalog()->NumericType(
//        nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s, 0))));
//    TupleType* streamTupleType = new TupleType(streamTupleTypeNL);
//    int algId = streamTupleType->GetAttributeType(keyIndex).algId;
//    int typeId = streamTupleType->GetAttributeType(keyIndex).typeId;
//    st = static_cast<Attribute*>(
//            am->CreateObj(algId, typeId)(0).addr)->GetStorageType();
//
//    if (Attribute::Default == st)
//    {
//      keySize = sizeof(keySize)
//          + streamTupleType->GetAttributeType(keyIndex).size;
//    }
//    else if ((Attribute::Core != st) && (Attribute::Extension != st))
//    {
//      cerr << "ERROR: unknown storage type for attribute No "
//          << keyIndex << endl;
//      assert(false);
//    }

    sd = ((CcInt*) args[5].addr)->GetIntval();
  }
  else
  {
    sd = ((CcInt*) args[4].addr)->GetIntval();
  }
  Socket *client = Socket::CreateClient( sd );
  int count = 0;
  int wcount = 0;

  char tupleBlock[MAX_TOTALTUPLESIZE];
  memset(tupleBlock, 0, sizeof(tupleBlock));
  u_int32_t offset = SOCKET_METASIZE;
  u_int32_t tbSize = MAX_TOTALTUPLESIZE - SOCKET_SIZE;
  // left space in tupleBlock

  int32_t sock_ID = 0;    //used to synchronize the sockets,
  int32_t tup_Num = 0;     //amount of tuples inside buffer

  int counter = 0;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  bool haveNext = false;
  while ( ( haveNext = qp->Received(args[0].addr)) || (tup_Num > 0))
  {
    counter++;

    size_t tupleBlockSize;

    if ( haveNext )
    {
      Tuple *tuple = (Tuple*)elem.addr;

      size_t coreSize = 0;
      size_t extensionSize = 0;
      size_t flobSize = 0;
      tupleBlockSize = tuple->GetBlockSize(coreSize, extensionSize, flobSize);
      // tupleBlockSize =  sizeof(uint32_t) + sizeof(uint16_t)
      //                   + coreSize + extensionSize + flobSize
      assert(tupleBlockSize < MAX_TOTALTUPLESIZE);

      if(haveKey && ("string" == keyTypeName))
      {
        keySize =
            ((string)(char*)((CcString*)tuple
                ->GetAttribute(keyIndex))->GetStringval()).size() + 1;
      }

      if (tupleBlockSize + keySize + sizeof(keySize) >= tbSize)
      {
        //No more space for this tuple, send the buffer
        sendTupleBlock(client, tupleBlock, &sock_ID, offset);
        count += tup_Num;

        //reset buffer
        memset(tupleBlock, 0, sizeof(tupleBlock));
        offset = SOCKET_METASIZE;
        tbSize = MAX_TOTALTUPLESIZE - SOCKET_SIZE;
        tup_Num = 0;
      }

      //Write the key Value before the tuple value
      if (haveKey)
      {
        if ("int" == keyTypeName)
        {
          memcpy(tupleBlock + offset, &keySize, sizeof(keySize));
          offset += sizeof(keySize);
          int keyValue =
              ((CcInt*)tuple->GetAttribute(keyIndex))->GetIntval();
          memcpy(tupleBlock + offset, &keyValue, keySize);
        }
        else if ("string" == keyTypeName)
        {
          memcpy(tupleBlock + offset, &keySize, sizeof(keySize));
          offset += sizeof(keySize);
          memcpy(tupleBlock + offset,
              (char*)((CcString*)tuple->GetAttribute(keyIndex))
              ->GetStringval(), keySize);
        }
        else if ("bool" == keyTypeName)
        {
          memcpy(tupleBlock + offset, &keySize, sizeof(keySize));
          offset += sizeof(keySize);
          bool keyValue =
              ((CcBool*)tuple->GetAttribute(keyIndex))->GetBoolval();
          memcpy(tupleBlock + offset, &keyValue, keySize);
        }
        else if ("real" == keyTypeName)
        {
          memcpy(tupleBlock + offset, &keySize, sizeof(keySize));
          offset += sizeof(keySize);
          double keyValue =
              ((CcReal*)tuple->GetAttribute(keyIndex))->GetRealval();
          memcpy(tupleBlock + offset, &keyValue, keySize);
        }
        offset += keySize;
        tbSize -= (sizeof(keySize) + keySize);
      }

      // Write the tuple into the buffer
      tuple->WriteToBin(tupleBlock + offset,
          coreSize, extensionSize, flobSize);

      tup_Num++;
      offset += tupleBlockSize;
      tbSize -= tupleBlockSize;
      wcount++;

      tuple->DeleteIfAllowed();
      qp->Request(args[0].addr, elem);
    }
    else
    {
      sendTupleBlock(client, tupleBlock, &sock_ID, offset);
      count += tup_Num;
      tup_Num = 0;
    }
  }
  //tup_Num = -1;

  sendSocket(client, tupleBlock, &sock_ID, -1);

  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, count);
  qp->Close(args[0].addr);

  if (client)
    client->Close();
  delete client;
  client = 0;

  return 0;
}

int
TReceiveStream(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  struct RemoteStreamInfo
  {
    Socket *client;
    int32_t sock_ID;
    string keyTypeName;
    bool haveKey;
    TupleType* tupleType;
    TupleBuffer *tupleBuffer;
    GenericRelationIterator *iterator;
    bool sockValid; //Indicate whether the socket is available

    inline RemoteStreamInfo(
        Socket *client, string ktn, TupleType* tupleType ):
      client( client ), sock_ID(0), keyTypeName(ktn),
      tupleType( tupleType ), tupleBuffer(0), iterator(0)
      ,sockValid(true)
    {
      haveKey = keyTypeName.size() > 0 ? true : false;
    }

    inline ~RemoteStreamInfo()
    {
      if( client )
        client->Close();
      delete client;
      client = 0;

      if (tupleBuffer)
        delete tupleBuffer;
      tupleBuffer = 0;
    }

    Tuple* LoadTuples(char* buf, u_int32_t curPos)
    {

      tupleBuffer = new TupleBuffer(
          qp->MemoryAvailableForOperator());

      u_int32_t offset = 0;
      Tuple* tuple = 0;
      while(offset < curPos)
      {
        if (haveKey)
        {
          //Ignore the part contains the key value;
          u_int32_t keySize = 0;
          memcpy(&keySize, buf + offset, sizeof(u_int32_t));
          offset += sizeof(u_int32_t);  //skip keySize
          offset += keySize;            //skip keyValue
        }

        u_int32_t tupleBlockSize = 0;
        memcpy(&tupleBlockSize, buf + offset, sizeof(tupleBlockSize));
        if (tupleBlockSize > 0)
        {
          tuple = new Tuple(tupleType);
          tuple->ReadFromBin(buf + offset);
          offset += tupleBlockSize;

          tupleBuffer->AppendTuple(tuple);
          tuple->DeleteIfAllowed();
          tuple = 0;
        }
        else
          break;
      }
      if(tupleBuffer->GetNoTuples() > 0)
      {
        iterator = tupleBuffer->MakeScan();
        return iterator->GetNextTuple();
      }
      return 0;
    }

  } *remoteStreamInfo;

  switch( message )
  {
    case OPEN:
    {
      TRACE_ENTER
      TupleType* tupleType;

      ListExpr resultType = GetTupleResultType(s);
      tupleType = new TupleType(nl->Second(resultType));

      string keyTypeName =
          (string)(char*)((CcString*)args[2].addr)->GetStringval();

      SocketDescriptor sd = ((CcInt*) args[3].addr)->GetIntval();
      Socket *client = Socket::CreateClient( sd );

      remoteStreamInfo =
          new RemoteStreamInfo( client, keyTypeName, tupleType );

      local.addr = remoteStreamInfo;
      TRACE_LEAVE
      return 0;
    }

    case REQUEST:
    {
      TRACE_ENTER
      string line;
      remoteStreamInfo = ((RemoteStreamInfo*) local.addr);
      Tuple* tuple = 0;

      GenericRelationIterator* itr = remoteStreamInfo->iterator;
      if (itr != 0)
      {
        tuple = itr->GetNextTuple();
        if (tuple != 0)
        {
          result = SetWord(tuple);
          return YIELD;
        }
        else
        {
          delete itr;
          remoteStreamInfo->iterator = 0;
          delete remoteStreamInfo->tupleBuffer;
          remoteStreamInfo->tupleBuffer = 0;
        }
      }

      char tupleBlock[MAX_TOTALTUPLESIZE];
      memset(tupleBlock, 0, sizeof(tupleBlock));
      Socket* client = remoteStreamInfo->client;
      int* sock_ID = &(remoteStreamInfo->sock_ID);
      int sock_Num = -1;

      size_t offset = 0;
      if (remoteStreamInfo->sockValid)
      {
          while ((sock_Num = receiveSocket(
              client, tupleBlock + offset, sock_ID)) > 1)
          {
            offset += SOCKTUP_SIZE;
          }

          if (sock_Num > 0)
          {
            offset += SOCKTUP_SIZE;
            //Load the tuples from the buffer
            tuple = remoteStreamInfo->LoadTuples(tupleBlock, offset);
            if (tuple)
            {
              result.setAddr(tuple);
              TRACE_LEAVE
              return YIELD;
            }
          }
          else
            remoteStreamInfo->sockValid = false;
        }

      //No more results
      result.setAddr(0);

      TRACE_LEAVE
      return CANCEL;
    }

    case CLOSE:
    {
      remoteStreamInfo = ((RemoteStreamInfo*) local.addr);
      delete remoteStreamInfo;
      TRACE_LEAVE
      return 0;
    }
  }
  /* should not happen */
  return -1;
}

/*
4 Definition of Operators

*/

const string sendSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(stream (tuple x)) int -> int</text--->"
    "<text>send [ _ , _ ]</text--->"
    "<text>Listen to a port and send the tuples of "
    "a stream to a client.</text--->"
    "<text>query Rel feed send[p1032]</text--->"
    ") )";

const string receiveSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(stream (tuple x)) int -> int</text--->"
    "<text>receive [ _ ]</text--->"
    "<text>Read the tuples of "
    "a stream.</text--->"
    "<text>query receive(hostname,p1032) "
    "consume</text--->"
    ") )";


/*
Used to explain the signature and the meaning of operators.

*/

Operator TSend (
  "send",    //name
  sendSpec,         //specification
  TSendStream,   //value mapping
  Operator::SimpleSelect,   //trivial selection function
  TSendTypeMap   //type mapping
);

Operator TReceive (
  "receive",    //name
  receiveSpec,         //specification
  TReceiveStream,   //value mapping
  Operator::SimpleSelect,   //trivial selection function
  TReceiveTypeMap   //type mapping
);


/*
5 Creating the Algebra

*/

class RemoteStreamAlgebra : public Algebra
{
 public:
  RemoteStreamAlgebra() : Algebra()
  {
    AddOperator( &TSend );
    AddOperator( &TReceive );
  }
  ~RemoteStreamAlgebra() {};
};


/*
6 Initialization

*/

extern "C"
Algebra*
InitializeRemoteStreamAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new RemoteStreamAlgebra());
}

