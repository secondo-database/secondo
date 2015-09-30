/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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

*/

#include "AlgebraOperators.h"

#include <chrono>

namespace KVS {

extern KeyValueStoreIPC* kvsIPC;

/* **********************
 * Operator: kvsRemoteStream(streamId)
 *
 */

ListExpr kvsRemoteStreamTM(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError("1 argument expected. [int = (streamId)].");
  }

  if (!CcInt::checkType(nl->First(nl->First(args)))) {
    return listutils::typeError(
        "1st argument should be int. [int = (streamId)].");
  }

  // get streamId and streamType

  ListExpr streamId = nl->First(args);
  Word streamIdRes;
  if (!QueryProcessor::ExecuteQuery(nl->ToString(streamId), streamIdRes)) {
    return listutils::typeError("could not evaluate the value of  " +
                                nl->ToString(streamId));
  }

  CcInt* streamIdVal = (CcInt*)streamIdRes.addr;
  if (!streamIdVal->IsDefined()) {
    return listutils::typeError("Undefined stream id.");
  }

  NetworkStreamIPC* nstreamIPC =
      kvsIPC->getNetworkStream(streamIdVal->GetIntval());

  if (nstreamIPC) {
    ListExpr streamType;
    string streamTypeStr = nstreamIPC->getStreamType();
    nl->ReadFromString(streamTypeStr, streamType);

    streamIdVal->DeleteIfAllowed();

    return streamType;
  } else {
    return listutils::typeError("Problem retrieving network stream.");
  }
}

int kvsRemoteStreamVM(Word* args, Word& result, int message, Word& local,
                      Supplier s) {
  NetworkStreamIPC* nstreamIPC = static_cast<NetworkStreamIPC*>(local.addr);

  switch (message) {
    case OPEN: {
      CcInt* streamId = static_cast<CcInt*>(args[0].addr);
      nstreamIPC = kvsIPC->getNetworkStream(streamId->GetIntval());

      if (nstreamIPC) {
        if (nstreamIPC->requestStream()) {
          local.addr = nstreamIPC;
          return 0;
        }
      }

      return CANCEL;
    }
    case REQUEST: {
      if (nstreamIPC) {
        unsigned int n = 0;
        char* tupleBuffer = nstreamIPC->nextTuple(&n);

        if (n > 0) {
          TupleType* tupleType;
          ListExpr resultType = GetTupleResultType(s);
          tupleType = new TupleType(nl->Second(resultType));

          Tuple* tempTuple = new Tuple(tupleType);
          tempTuple->ReadFromBin(tupleBuffer);

          delete[] tupleBuffer;

          result = SetWord(tempTuple);
          return YIELD;
        }
      }

      cout << "Canceling Request." << endl;
      result.setAddr(0);
      return CANCEL;
    }
    case CLOSE: {
      if (nstreamIPC) {
        kvsIPC->removeNetworkStream(nstreamIPC->streamid);
        local.setAddr(0);
      }

      result.setAddr(0);
      return 0;
    }
  }
  /* should not happen */
  return -1;
}

/* **********************
 * Operator: kvsRemoteStreamSCP(streamId)
 *
 */

ListExpr kvsRemoteStreamSCPTM(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError("1 argument expected. [int = (streamId)].");
  }

  if (!CcInt::checkType(nl->First(nl->First(args)))) {
    return listutils::typeError(
        "1st argument should be int. [int = (streamId)].");
  }

  // get streamId and streamType

  ListExpr streamId = nl->First(args);
  Word streamIdRes;
  if (!QueryProcessor::ExecuteQuery(nl->ToString(streamId), streamIdRes)) {
    return listutils::typeError("could not evaluate the value of  " +
                                nl->ToString(streamId));
  }

  CcInt* streamIdVal = (CcInt*)streamIdRes.addr;
  if (!streamIdVal->IsDefined()) {
    return listutils::typeError("Undefined stream id.");
  }

  // get type

  // busy wait for type file creation
  string typeFilePath = kvsIPC->getSCPTransferPath() + PATH_SLASH +
                        stringutils::int2str(streamIdVal->GetIntval()) +
                        "_type";
  while (!FileSystem::FileOrFolderExists(typeFilePath)) {
    boost::this_thread::sleep(boost::posix_time::milliseconds(250));
  }

  streamIdVal->DeleteIfAllowed();

  // read file
  ifstream typeFile(typeFilePath, ios::binary);

  size_t typeLen;
  typeFile.read((char*)&typeLen, sizeof(typeLen));

  char* typeBuffer = new char[typeLen];
  typeFile.read(typeBuffer, typeLen);

  typeFile.close();

  ListExpr streamType;
  nl->ReadFromString(typeBuffer, streamType);

  delete[] typeBuffer;

  return streamType;
}

int kvsRemoteStreamSCPVM(Word* args, Word& result, int message, Word& local,
                         Supplier s) {
  ifstream* dataFile = static_cast<ifstream*>(local.addr);

  switch (message) {
    case OPEN: {
      CcInt* streamId = static_cast<CcInt*>(args[0].addr);

      // busy wait for type file creation
      string dataFilePath = kvsIPC->getSCPTransferPath() + PATH_SLASH +
                            stringutils::int2str(streamId->GetIntval());
      while (!FileSystem::FileOrFolderExists(dataFilePath)) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(250));
      }

      delete dataFile;
      dataFile = new ifstream(dataFilePath, ios::binary);

      local.addr = dataFile;
      return 0;
    }
    case REQUEST: {
      size_t tupleBlockSize;
      dataFile->read((char*)&tupleBlockSize, sizeof(tupleBlockSize));

      if (tupleBlockSize != 0) {
        TupleType* tupleType;
        ListExpr resultType = GetTupleResultType(s);
        tupleType = new TupleType(nl->Second(resultType));

        Tuple* tempTuple = new Tuple(tupleType);

        char* tupleBuffer = new char[tupleBlockSize];
        dataFile->read(tupleBuffer, tupleBlockSize);

        tempTuple->ReadFromBin(tupleBuffer);

        delete[] tupleBuffer;

        result = SetWord(tempTuple);
        return YIELD;
      } else {
        dataFile->close();
        delete dataFile;

        result.setAddr(0);
        return CANCEL;
      }
    }
    case CLOSE: {
      if (dataFile != 0) {
        delete dataFile;
        local.setAddr(0);
        result.setAddr(0);
      }
      // cleanup?
      // remove from NetworkStreamBuffer?
      return 0;
    }
  }
  /* should not happen */
  return -1;
}

/* **********************
 * Operator: kvsDistribute(serverIdAttribute, distribution, clientCommand)
 *
 */

/* **********************
 * Operator: _ kvsDistribute(serverIdAttribute, {distribution,
 * distributionName}, targetRelationName, insertCommand [, deleteCommand] [,
 * restructure])
 *
 *
 *
 *
 */

string createBaseAttributeList(ListExpr attrList,
                               string serverIdAttributeName) {
  stringstream result;

  if (!nl->IsAtom(attrList)) {
    bool firstFound = true;
    ListExpr first;
    ListExpr rest = attrList;

    while (!nl->IsEmpty(rest)) {
      first = nl->First(rest);
      rest = nl->Rest(rest);

      if ((nl->ListLength(first) == 2) &&
          (nl->AtomType(nl->First(first)) == SymbolType)) {
        if (nl->SymbolValue(nl->First(first)).compare(serverIdAttributeName) !=
            0) {
          if (!firstFound) {
            result << ",";
          } else {
            firstFound = false;
          }

          result << nl->SymbolValue(nl->First(first));
        }
      }
    }
  }

  return result.str();
}

ListExpr kvsDistributeTM(ListExpr args) {
  int listLen = nl->ListLength(args);
  if (listLen < 5 || listLen > 7) {
    return listutils::typeError(
        "5 to 7 arguments expected. [stream (tuple (...)) x symbol x "
        "{distribution, text} x text x text [x text] [x bool]= _ "
        "(serverIdAttribute, {distribution, distributionName},  "
        "targetRelationName, insertCommand [, deleteCommand] [, "
        "restructure])]");
  }

  if (!Stream<Tuple>::checkType(nl->First(nl->First(args)))) {
    return listutils::typeError(
        "1st argument should be stream (tuple (...)). [stream (tuple (...)) x "
        "symbol x {distribution, text} x text x text [x text] [x bool]= _ "
        "(serverIdAttribute, {distribution, distributionName},  "
        "targetRelationName, insertCommand [, deleteCommand] [, "
        "restructure])]");
  }

  if (nl->AtomType(nl->First(nl->Second(args))) != SymbolType) {
    return listutils::typeError(
        "2nd argument should be symbol. [stream (tuple (...)) x symbol x "
        "{distribution, text} x text x text [x text] [x bool]= _ "
        "(serverIdAttribute, {distribution, distributionName},  "
        "targetRelationName, insertCommand [, deleteCommand] [, "
        "restructure])]");
  }

  if (!Distribution::checkType(nl->First(nl->Third(args))) &&
      !FText::checkType(nl->First(nl->Third(args)))) {
    return listutils::typeError(
        "3rd argument should be distribution OR text(distribution name). "
        "[stream (tuple (...)) x symbol x {distribution, text} x text x text "
        "[x text] [x bool]= _ (serverIdAttribute, {distribution, "
        "distributionName},  targetRelationName, insertCommand [, "
        "deleteCommand] [, restructure])]");
  }

  if (!FText::checkType(nl->First(nl->Fourth(args)))) {
    return listutils::typeError(
        "4th argument should be text. [stream (tuple (...)) x symbol x "
        "{distribution, text} x text x text [x text] [x bool]= _ "
        "(serverIdAttribute, {distribution, distributionName},  "
        "targetRelationName, insertCommand [, deleteCommand] [, "
        "restructure])]");
  }

  if (!FText::checkType(nl->First(nl->Fifth(args)))) {
    return listutils::typeError(
        "5th argument should be text. [stream (tuple (...)) x symbol x "
        "{distribution, text} x text x text [x text] [x bool]= _ "
        "(serverIdAttribute, {distribution, distributionName},  "
        "targetRelationName, insertCommand [, deleteCommand] [, "
        "restructure])]");
  }

  if (listLen > 5 && !FText::checkType(nl->First(nl->Sixth(args))) &&
      !CcBool::checkType(nl->First(nl->Sixth(args)))) {
    return listutils::typeError(
        "6th argument should be text or bool. [stream (tuple (...)) x symbol x "
        "{distribution, text} x text x text [x text] [x bool]= _ "
        "(serverIdAttribute, {distribution, distributionName},  "
        "targetRelationName, insertCommand [, deleteCommand] [, "
        "restructure])]");
  }

  if (listLen > 6 && !CcBool::checkType(nl->First(nl->Seventh(args)))) {
    return listutils::typeError(
        "7th argument should be bool. [stream (tuple (...)) x symbol x "
        "{distribution, text} x text x text [x text] [x bool]= _ "
        "(serverIdAttribute, {distribution, distributionName},  "
        "targetRelationName, insertCommand [, deleteCommand] [, "
        "restructure])]");
  }

  // append stream type as text
  NList streamTypeList(nl->First(nl->First(args)));
  string streamType = streamTypeList.convertToString();

  // append distribution name
  ListExpr dist_name;
  if (FText::checkType(nl->First(nl->Third(args)))) {
    dist_name = nl->Second(nl->Third(args));  // ((text 'distname')...)
  } else {
    string dist_name_str = nl->SymbolValue(nl->Second(nl->Third(args)));
    dist_name = nl->TextAtom(dist_name_str);  // ( (..) (distribution distname))
  }

  // append serverIdAttribute index
  ListExpr attr_desc = nl->First(nl->Second(args));
  string attr_name = nl->SymbolValue(attr_desc);

  ListExpr tuple_desc = nl->Second(nl->First(nl->First(args)));
  if (nl->IsEqual(nl->First(tuple_desc), Tuple::BasicType()) &&
      nl->ListLength(tuple_desc) == 2) {
    ListExpr attrL = nl->Second(tuple_desc);

    cout << "tuple_desc: " << nl->ToString(tuple_desc) << "\n";
    cout << "attrL:" << nl->ToString(attrL) << "\n";

    if (IsTupleDescription(attrL)) {
      int attrIndex;
      ListExpr attrType;

      attrIndex = FindAttribute(attrL, attr_name, attrType);

      if (attrIndex > 0) {
        return nl->ThreeElemList(
            nl->SymbolAtom(Symbol::APPEND()),
            nl->FourElemList(
                nl->IntAtom(attrIndex), nl->TextAtom(streamType),
                nl->TextAtom(createBaseAttributeList(attrL, attr_name)),
                dist_name),
            nl->First(nl->First(args)));  // return input stream

      } else {
        return listutils::typeError(
            "input not as expected: Attribute not found in tuple...");
      }
    } else {
      return listutils::typeError(
          "input not as expected: Tuple description not recognized...");
    }
  } else {
    return listutils::typeError(
        "input not as expected: TupleType not recognized...");
  }
}

int kvsDistributeVM(Word* args, Word& result, int message, Word& local,
                    Supplier s) {
  DistributeIPC* disIPC = static_cast<DistributeIPC*>(local.addr);

  switch (message) {
    case OPEN: {
      delete disIPC;

      int noSons = qp->GetNoSons(s);

      // Distribution:
      string distributionName =
          static_cast<FText*>(args[noSons - 1].addr)->GetValue();
      int distRef = kvsIPC->getDistributionRef(distributionName);

      if (distRef < 0) {
        ListExpr distributionType = qp->GetSupplierTypeExpr(qp->GetSon(s, 2));
        if (Distribution::checkType(distributionType)) {
          Distribution* dist = static_cast<Distribution*>(args[2].addr);

          distRef = kvsIPC->getDistributionRef(distributionName, dist->type,
                                               dist->toBin());
        } else {
          cout << "Error: No Distribution specified";
          return CANCEL;
        }
      }

      if (distRef >= 0) {
        int attrIndex = ((CcInt*)(args[noSons - 4].addr))->GetIntval() - 1;
        string streamType =
            static_cast<FText*>(args[noSons - 3].addr)->GetValue();
        string baseAttributeList =
            static_cast<FText*>(args[noSons - 2].addr)->GetValue();

        string targetRelation = static_cast<FText*>(args[3].addr)->GetValue();
        string insertCommand = static_cast<FText*>(args[4].addr)->GetValue();

        // Optional Arguments
        string deleteCommand("");
        bool restructure = true;
        if (noSons == 7 + 4) {
          deleteCommand = static_cast<FText*>(args[5].addr)->GetValue();
          restructure = static_cast<CcBool*>(args[6].addr)->GetValue();
        } else if (noSons == 6 + 4) {
          ListExpr sixthArgType = qp->GetSupplierTypeExpr(qp->GetSon(s, 5));
          if (FText::checkType(sixthArgType)) {
            deleteCommand = static_cast<FText*>(args[5].addr)->GetValue();
          } else {
            restructure = static_cast<CcBool*>(args[5].addr)->GetValue();
          }
        }

        cout << "streamType:" << streamType << endl;
        cout << "baseAttributeList:" << baseAttributeList << endl;
        cout << "targetRelation:" << targetRelation << endl;
        cout << "insertCommand:" << insertCommand << endl;
        cout << "deleteCommand:" << deleteCommand << endl;

        // open new local ipc connection to handle distribution
        IPCConnection* distributeConn = IPCConnection::connect(0);
        if (distributeConn) {
          cout << "Opened Connection Id (kvsDistribute):"
               << distributeConn->connectionId << endl;
        }

        disIPC = new DistributeIPC(distributeConn);

        cout << "StreamType:" << streamType << endl;

        if (disIPC->init(distRef, streamType, baseAttributeList, targetRelation,
                         insertCommand, deleteCommand, restructure)) {
          cout << "Distribution successfully initialized" << endl;

          Stream<Tuple> inTupleStream(args[0].addr);
          inTupleStream.open();
          Tuple* tuple;

          cout << "Reading Data ..." << endl;

          typedef std::chrono::high_resolution_clock Clock;
          auto overallTime = Clock::now();

          size_t tupleBufferSize = 0;
          char* tupleBuffer = 0;

          size_t tupleBlockSize;
          size_t coreSize = 0;
          size_t extensionSize = 0;
          size_t flobSize = 0;

          unsigned int requestTime = 0;
          auto before = Clock::now();

          while ((tuple = inTupleStream.request()) != 0 &&
                 !disIPC->checkResult()) {
            requestTime +=
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    Clock::now() - before)
                    .count();

            // get server id
            int serverId = static_cast<CcInt*>(tuple->GetAttribute(attrIndex))
                               ->GetIntval();

            tupleBlockSize =
                tuple->GetBlockSize(coreSize, extensionSize, flobSize);

            if (tupleBlockSize > tupleBufferSize) {
              delete[] tupleBuffer;
              tupleBuffer = new char[tupleBlockSize];
              tupleBufferSize = tupleBlockSize;
            }

            tuple->WriteToBin(tupleBuffer, coreSize, extensionSize, flobSize);

            disIPC->sendTuple(serverId, tupleBuffer, tupleBlockSize);
            before = Clock::now();
          }

          cout << "Finished Reading Data..." << endl;

          cout << "Overall Time:"
               << std::chrono::duration_cast<std::chrono::milliseconds>(
                      Clock::now() - overallTime)
                      .count()
               << endl;
          cout << "Request Time:" << requestTime << endl;

          disIPC->end();

          inTupleStream.close();

          local.addr = disIPC;
          return 0;
        }
      }

      return CANCEL;
    }
    case REQUEST: {
      if (disIPC != 0 && disIPC->getResult()) {
        int n = 0;
        char* tupleBuffer = disIPC->nextTuple(&n);

        if (n > 0) {
          TupleType* tupleType;
          ListExpr resultType = GetTupleResultType(s);

          tupleType = new TupleType(nl->Second(resultType));

          Tuple* tempTuple = new Tuple(tupleType);
          tempTuple->ReadFromBin(tupleBuffer);

          delete[] tupleBuffer;

          result = SetWord(tempTuple);
          return YIELD;
        } else {
          disIPC->close();
          disIPC->resultStreamSuccess = false;
          result.addr = 0;
          return CANCEL;
        }
      } else {
        cout << "No Result..." << endl;
        if (disIPC != 0) {
          disIPC->close();
          disIPC->resultStreamSuccess = false;
        }

        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      delete disIPC;
      local.addr = 0;
    }
      return CANCEL;
  }

  return CANCEL;
}

}  // namespace KVS
