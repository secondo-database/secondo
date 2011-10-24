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

Nov 2010 Jiamin Lu change the type mapping function by using
arguments' values in typemapping function, to avoid using symbol type
argument values to express the IP address and port.

1 RemoteStream Algebra

This algebra allows distributed query processing by connecting streams
sockets.

*/

using namespace std;

#include "RemoteStreamAlgebra.h"
#include "Symbols.h"
//#include "../HadoopParallel/HadoopParallelAlgebra.h"

#define TRACE_ON

extern NestedList* nl;
extern QueryProcessor *qp;

const int min_PortNum = 1024;
const int max_PorNum = 65535;

/*

2 Overview

This Algebra has two operators: ~send~ and ~receive~

The ~send~ operator expects a stream and a port number, and start to listen to
that port. It returns the number of tuples sent.

  * stream(tuple) port [->] int       send

As we register these two operators with ~SetUsesArgsInTypeMapping~,
the port number is an integer number, E.g.

  query plz feed send[1032]

The ~receive~ operator expetcs a hostname and a port, and produces a stream:

  * hostname port [->] stream(tuple)      receive

The hostname is a string type, could be an IP address, or a hostname.

  query receive("192.168.0.1", 1032) consume

2.1 Type Mapping Function

Type mapping for ~send~ is

----	((stream tuple((a1 t1)(a2 t2) ... (an tn)))
       x int x [ai]) -> (int)
----

Type mapping for ~receive~ is

----	(string int) -> (stream tuple)
----

Operator send also accept an optional argument keyAttribute.
If the keyAttribute is specified, then for each tuple,
it's keyAttribute value would be copied, and put at the head of
the tuple's binary value.

*/

ListExpr
TSendTypeMap(ListExpr args)
{
  NList l(args);

  string lenErr = "Operator send expect a list of "
      "two or three arguments";
  string typeErr = "Operator send expects "
      "(stream(tuple(a1, a2, ... an))) x int x [ai]";
  string portErr = "Error! Port number should within [ " +
      int2string(min_PortNum) + "," + int2string(max_PorNum) + " ]";
  string keyNameErr = "Error! NOT find attribute name : ";
  string keyTypeErr = "Error! Expect {int,string,bool,real} type "
      "of key attribute : ";
  string connErr = "Error! Connection error of ";
  string evaErr = "Error! Infeasible evaluation in TM for attribute ";

  int len = l.length();
  if (len != 2 && len != 3)
    return l.typeError(lenErr);

  NList attr;
  if (!l.first().first().checkStreamTuple(attr))
    return l.typeError(typeErr);
  string streamType = l.first().first().convertToString();
  string tupleType = l.first().first().second().convertToString();

  if (!l.second().first().isSymbol(Symbols::INT()))
    return l.typeError(typeErr);
  NList pList;
  if (!QueryProcessor::GetNLArgValueInTM(l.second().second(), pList))
    return l.typeError(evaErr + "port number");
  int port = pList.intval();
  if (port < min_PortNum || port > max_PorNum)
  {
    cerr << "The present port number is: " << port << endl;
    return l.typeError(portErr);
  }

  int keyIndex = 0;    //The index of the key attribute
  ListExpr keyTypeNL = nl->Empty();
  string keyTypeStr = "";
  if (3 == len)
  {
    if (!l.third().first().isAtom())
      return l.typeError(typeErr);
    string keyName = l.third().second().str();

    keyIndex = listutils::findAttribute(
        attr.listExpr(), keyName, keyTypeNL);
    if (0 == keyIndex)
      return l.typeError(keyNameErr + keyName);

    NList keyType(keyTypeNL);
    if (!( keyType.isSymbol(Symbols::INT())
        || keyType.isSymbol(Symbols::STRING())
        || keyType.isSymbol(Symbols::REAL())
        || keyType.isSymbol(Symbols::BOOL())))
      return l.typeError(keyTypeErr + keyName);
    keyTypeStr = keyType.convertToString();
  }

  Socket *gate = Socket::CreateGlobal("localhost", int2string(port)),
         *client;

  if (!gate || !gate->IsOk())
    return l.typeError(connErr +
          "unable listening to port: " + int2string(port));

  client = gate->Accept();
  if (!client || !client->IsOk())
  {
    gate->Close();
    return l.typeError(connErr +
        "unable connecting with client." );
  }

  client->GetSocketStream()
      << streamType
      << ((0 == keyTypeStr.length()) ? ""
          : ("APPEND " + keyTypeStr))<< endl;
  string rtnInfo;
  getline(client->GetSocketStream(), rtnInfo);
  //The client should send a handshake message to server as a response
  if (rtnInfo != "<GET TYPE/>")
  {
    gate->Close();
    return l.typeError(connErr +
        "unable getting handshake message from the client.");
    return nl->TypeError();
  }

  // Warning - the client pointer is not being deleted.
  gate->Close();
  delete gate;

  int sd = client->GetDescriptor();
  return NList(NList(Symbol::APPEND()),
               NList(NList(keyIndex),
                     NList(keyTypeStr, true),
                     NList(sd)),
               NList(Symbols::INT())).listExpr();
}

ListExpr
TReceiveTypeMap(ListExpr args)
{
//  ListExpr first, second;
  NList l(args);

  string lenErr = "Operator receive expects a list of length two.";
  string typeErr = "Operator receive expects (string , int)";
  string portErr = "Error! Port number should within [ " +
    int2string(min_PortNum) + "," + int2string(max_PorNum) + " ]";
  string connErr = "Error! Connection error of ";
  string evaErr = "Error! Infeasible evaluation for attribute ";

  if (l.length() != 2)
    return l.typeError(lenErr);

  if (!l.first().first().isSymbol(Symbols::STRING()))
    return l.typeError(typeErr);

  if (!l.second().first().isSymbol(Symbols::INT()))
    return l.typeError(typeErr);

  string host, streamTypeStr;
  NList hList;
  if (!QueryProcessor::GetNLArgValueInTM(l.first().second(), hList))
    return l.typeError(evaErr + "host name");
  host = hList.str();
  NList pList;
  if (!QueryProcessor::GetNLArgValueInTM(l.second().second(), pList))
    return l.typeError(evaErr + "port number");
  int port = pList.intval();
  if (port < min_PortNum || port > max_PorNum)
    return l.typeError(portErr);

  cout << "Host: " << host << ":" << port << endl;
  Socket *client =
      Socket::Connect( host, int2string(port),
          Socket::SockGlobalDomain );

  if (!client || !client->IsOk())
    return l.typeError(connErr +
      "unable connecting with server." );

  getline( client->GetSocketStream(), streamTypeStr );
  client->GetSocketStream() << "<GET TYPE/>" << endl;

  string keyTypeName = "";
  string::size_type loc = streamTypeStr.find(Symbol::APPEND(),0);
  if (loc != string::npos)
  {
    keyTypeName = streamTypeStr.substr(loc + 7);
    streamTypeStr = streamTypeStr.substr(0, loc);
  }

  ListExpr streamType;
  nl->ReadFromString(streamTypeStr, streamType);

  // Warning - the client pointer is not being deleted.
  return nl->ThreeElemList(
           nl->SymbolAtom(Symbol::APPEND()),
           nl->TwoElemList(
               nl->StringAtom(keyTypeName),
               nl->IntAtom(client->GetDescriptor())),
           streamType);
}


/*
3 Value Mapping Function

Though the class Socket defined inside Secondo offers an iostream
object to transform data, and makes the communication very easy,
this method needs the sender to invoke tuple's ~Out~ function
to transform tuples into nestedLists,
then the receiver can use the ~In~ function to rebuild the tuples.
Obviously the transformation is inefficient and unnecessary,
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

//static const u_int32_t MAX_TUPLESIZE = 65535;
static const u_int32_t MAX_TOTALTUPLESIZE = 655359;
//(10*2^16 - 1) including huge flobs

static const u_int32_t SOCKET_SIZE = 1024; //Copy from StreamBuffer
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
      memcpy(&sockNum, sockBuffer + sizeof(int32_t),
          sizeof(int32_t));
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
  int sock_Num = static_cast<int>(
      ceil((float(curPos - SOCKET_METASIZE)) / SOCKTUP_SIZE));
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

    if (CcInt::BasicType() == keyTypeName)
      keySize = sizeof(int32_t);
    else if (CcString::BasicType() == keyTypeName)
      keySize = 0; //The size depends on the value
    else if (CcBool::BasicType() == keyTypeName)
      keySize = sizeof(bool);
    else if (CcReal::BasicType() == keyTypeName)
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

      if(haveKey && (CcString::BasicType() == keyTypeName))
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
        if (CcInt::BasicType() == keyTypeName)
        {
          memcpy(tupleBlock + offset, &keySize, sizeof(keySize));
          offset += sizeof(keySize);
          int keyValue =
              ((CcInt*)tuple->GetAttribute(keyIndex))->GetIntval();
          memcpy(tupleBlock + offset, &keyValue, keySize);
        }
        else if (CcString::BasicType() == keyTypeName)
        {
          memcpy(tupleBlock + offset, &keySize, sizeof(keySize));
          offset += sizeof(keySize);
          memcpy(tupleBlock + offset,
              (char*)((CcString*)tuple->GetAttribute(keyIndex))
              ->GetStringval(), keySize);
        }
        else if (CcBool::BasicType() == keyTypeName)
        {
          memcpy(tupleBlock + offset, &keySize, sizeof(keySize));
          offset += sizeof(keySize);
          bool keyValue =
              ((CcBool*)tuple->GetAttribute(keyIndex))->GetBoolval();
          memcpy(tupleBlock + offset, &keyValue, keySize);
        }
        else if (CcReal::BasicType() == keyTypeName)
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

    Tuple* LoadTuples(char* buf, u_int32_t curPos, Supplier s)
    {

      tupleBuffer = new TupleBuffer(
          (qp->GetMemorySize(s) * 1024 * 1024));

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
            tuple = remoteStreamInfo->LoadTuples(tupleBlock, offset, s);
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
    "( <text>(stream (tuple ((x1 t1) ... (xn tn)))) "
    "int [xi] -> int</text--->"
    "<text>_ send [ port , keyAttr ]</text--->"
    "<text>Send the tuples of a stream to a client through a port."
    "The optional third parameter select a key attribute, "
    "and copy its value as the tuple's head value, "
    "since in parallel join procedure, we need to extract the "
    "key attribute value in an external Java program.</text--->"
    "<text></text--->"
    ") )";

const string receiveSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>string int -> (stream (tuple x))</text--->"
    "<text>receive ( nodeName , port)</text--->"
    "<text>Get a stream of tuples from a remote machine "
    "through a port. No matter the key attribute in send operator "
    "is specified or not, tuples' head values are always ignored."
    "</text--->"
    "<text></text--->"
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
    TSend.SetUsesArgsInTypeMapping();
    AddOperator( &TReceive );
    TReceive.SetUsesArgsInTypeMapping();
    TReceive.SetUsesMemory();
  }
  ~RemoteStreamAlgebra() {};
};


/*
6 Initialization

*/

extern "C"
Algebra*
InitializeRemoteStreamAlgebra( NestedList* nlRef,
    QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new RemoteStreamAlgebra());
}

