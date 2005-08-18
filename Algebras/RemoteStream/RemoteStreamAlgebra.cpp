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
#include <string>
#include <iostream>   //for testing

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


2.1 Type Mapping Function

Type mapping for ~send~ is

----	((stream tuple) int) -> (int)


Type mapping for ~receive~ is

----	(hostname port) -> (stream tuple)


*/

ListExpr
TSendTypeMap(ListExpr args)
{
  ListExpr first, second;
  string argstr;

  CHECK_COND(nl->ListLength(args) == 2,
    "Operator send expects a list of length two.");

  first = nl->First(args);

  nl->WriteToString(argstr, first);
  CHECK_COND( nl->ListLength(first) == 2 &&
              nl->ListLength(nl->Second(first)) == 2 &&
              TypeOfRelAlgSymbol(nl->First(first)) == stream  &&
              TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple ,
    "Operator send expects a list with structure "
    "((stream (tuple ((a1 t1)...(an tn)))) port)\n"
    "Operator count gets a list with structure '" + argstr + "'.");

  second = nl->Second(args);

  CHECK_COND( nl->IsAtom(second) && nl->AtomType(second) == SymbolType,
    "Operator send expects as second argument a symbol atom (port number p9999)." );

  string port = nl->SymbolValue(second).substr(1, 4),
         streamType = nl->ToString(first),
         tupleType = nl->ToString(nl->Second(first));

  Socket *gate = Socket::CreateGlobal( "localhost", port ),
         *client;

  CHECK_COND( gate && gate->IsOk(), "Unable to listen to port" );

  client = gate->Accept();

  CHECK_COND( client && client->IsOk(), "Unable to connect with client" );

  client->GetSocketStream() << streamType << endl;

  // Warning - the client pointer is not being deleted.

  gate->Close();
  delete gate;

  return nl->ThreeElemList(
           nl->SymbolAtom("APPEND"),
           nl->TwoElemList(nl->TextAtom(tupleType),
                           nl->IntAtom(client->GetDescriptor())),
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
    "Operator receive expects as first argument a symbol atom (host name, p.e. localhost)." );

  // by now accepts only hostnames without dots in the name.

  second = nl->Second(args);
  CHECK_COND( nl->IsAtom(second) && nl->AtomType(second) == SymbolType,
    "Operator send expects as second argument a symbol atom (port number, p.e. p9999)." );

  string host = nl->SymbolValue(first),
         port = nl->SymbolValue(second).substr(1, 4),
         streamTypeStr;

  cout << "Host: " << host << ":" << port << endl;

  Socket *client = Socket::Connect( host, port, Socket::SockGlobalDomain );

  CHECK_COND( client && client->IsOk(), "Unable to connect to server." );

  getline( client->GetSocketStream(), streamTypeStr );

  ListExpr streamType;
  nl->ReadFromString(streamTypeStr, streamType);

  // Warning - the client pointer is not being deleted.

  return nl->ThreeElemList(
           nl->SymbolAtom("APPEND"),
           nl->TwoElemList(nl->TextAtom(nl->ToString(nl->Second(streamType))),
                           nl->IntAtom(client->GetDescriptor())),
           streamType);
}


/*
3 Value Mapping Function

*/

int
TSendStream(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word elem;
  int count = 0;
  string tupleTypeStr = ((FText*) args[2].addr)->Get();
  SocketDescriptor sd = ((CcInt*) args[3].addr)->GetIntval();
  Socket *client = Socket::CreateClient( sd );
  iostream& ss = client->GetSocketStream();
  ListExpr argList, tupleType;

  nl->ReadFromString(tupleTypeStr, argList);

  tupleType = SecondoSystem::GetCatalog( ExecutableLevel )->NumericType( nl->TwoElemList(argList,
                nl->IntAtom(nl->ListLength(nl->Second(argList)))) );

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  while ( qp->Received(args[0].addr) )
  {
    ListExpr tuple = ((Tuple*)elem.addr)->Out(tupleType);

    ss << nl->ToString(tuple) << endl;

    ((Tuple*)elem.addr)->DeleteIfAllowed();
    qp->Request(args[0].addr, elem);
    count++;
  }

  ss << "<Disconnect/>" << endl;

  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, count);
  qp->Close(args[0].addr);

  client->Close();
  delete client;

  return 0;
}

int
TReceiveStream(Word* args, Word& result, int message, Word& local, Supplier s)
{
  struct RemoteStreamInfo
  {
    Socket *client;
    iostream& ss;
    ListExpr tupleType;

    inline RemoteStreamInfo( Socket *client, iostream& ss, ListExpr tupleType ):
      client( client ), ss( ss ), tupleType( tupleType ) {}

    inline ~RemoteStreamInfo()
    {
      if( client ) client->Close();
      delete client;
    }
  } *remoteStreamInfo;

  switch( message )
  {
    case OPEN:
    {
      Word arg2, arg3;

      // Getting tuple type
      qp->Request(args[2].addr, arg2);
      string tupleTypeStr = ((FText *) arg2.addr)->Get();
      ListExpr tupleTypeList, tupleType;

      nl->ReadFromString(tupleTypeStr, tupleTypeList);

      tupleType = SecondoSystem::GetCatalog( ExecutableLevel )->
                    NumericType( nl->TwoElemList(tupleTypeList,
                      nl->IntAtom(nl->ListLength(nl->Second(tupleTypeList)))) );

      // Getting socket descriptor
      qp->Request(args[3].addr, arg3);
      SocketDescriptor sd = ((CcInt*) arg3.addr)->GetIntval();
      Socket *client = Socket::CreateClient( sd );
      iostream& ss = client->GetSocketStream();

      remoteStreamInfo = new RemoteStreamInfo( client, ss, tupleType );

      local.addr = remoteStreamInfo;

      return 0;
    }

    case REQUEST:
    {
      string line;

      remoteStreamInfo = ((RemoteStreamInfo*) local.addr);

      getline(remoteStreamInfo->ss, line );
      if ( line != "<Disconnect/>" )
      {
        ListExpr tupleList, errorInfo;
        int errorPos;
        bool correct;

        nl->ReadFromString(line, tupleList);

        Tuple *tuple = Tuple::In( remoteStreamInfo->tupleType, tupleList,
                                  errorPos, errorInfo, correct );

	result = SetWord( tuple );
        return YIELD;
      }
      else
        return CANCEL;
    }

    case CLOSE:
    {
      remoteStreamInfo = ((RemoteStreamInfo*) local.addr);
      delete remoteStreamInfo;
      return 0;
    }
  }
  /* should not happen */
  return -1;
}

/*
4 Definition of Operators

*/

const string sendSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>(stream (tuple x)) int -> int</text--->"
                         "<text>send [ _ ]</text--->"
                         "<text>Listen to a port and send the tuples of "
                         "a stream to a client.</text--->"
                         "<text>query Rel feed send[p1032]</text--->"
                         ") )";

const string receiveSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>(stream (tuple x)) int -> int</text--->"
                            "<text>receive [ _ ]</text--->"
                            "<text>Read the tuples of "
                            "a stream.</text--->"
                            "<text>query receive(hostname,p1032) consume</text--->"
                            ") )";


/*
Used to explain the signature and the meaning of operators.

*/

Operator TSend (
  "send",    //name
  sendSpec,         //specification
  TSendStream,   //value mapping
  Operator::DummyModel, //dummy model mapping, defined in Algebra.h
  Operator::SimpleSelect,   //trivial selection function
  TSendTypeMap   //type mapping
);

Operator TReceive (
  "receive",    //name
  receiveSpec,         //specification
  TReceiveStream,   //value mapping
  Operator::DummyModel, //dummy model mapping, defined in Algebra.h
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

RemoteStreamAlgebra remoteStreamAlgebra;

/*
6 Initialization

*/

extern "C"
Algebra*
InitializeRemoteStreamAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&remoteStreamAlgebra);
}

