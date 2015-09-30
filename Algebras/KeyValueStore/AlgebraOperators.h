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

#ifndef ALGEBRAOPERATORS_H_
#define ALGEBRAOPERATORS_H_

#include "StandardTypes.h"
#include "FTextAlgebra.h"
#include "Application.h"
#include "RectangleAlgebra.h"

#include "RelationAlgebra.h"
#include "SpatialAlgebra.h"
#include "FileSystem.h"
#include "Stream.h"

#include "KeyValueStoreIPC.h"
#include "DistributeIPC.h"

#include <algorithm>

#include "QuadTreeDistributionType.h"

#include "boost/thread.hpp"
#include "boost/date_time.hpp"

namespace KVS {

//
//
// SERVERS
//
//

struct kvsAddInfo : OperatorInfo {
  kvsAddInfo() {
    name = "kvsAdd";
    signature = FText::BasicType() + " x " + CcInt::BasicType() + " x " +
                CcInt::BasicType() + " -> " + CcBool::BasicType();
    syntax = "kvsAdd( host, interfacePort, kvsPort )";
    meaning = "adds server to server list";
  }
};
ListExpr kvsAddTM(ListExpr args);
int kvsAddVM(Word* args, Word& result, int message, Word& local, Supplier s);

struct kvsRemoveInfo : OperatorInfo {
  kvsRemoveInfo() {
    name = "kvsRemove";
    signature = CcInt::BasicType() + " -> " + CcBool::BasicType();
    syntax = "kvsRemove( idx )";
    meaning = "removes server from server list";
  }
};
ListExpr kvsRemoveTM(ListExpr args);
int kvsRemoveVM(Word* args, Word& result, int message, Word& local, Supplier s);

struct kvsUpdateServerListInfo : OperatorInfo {
  kvsUpdateServerListInfo() {
    name = "kvsUpdateServerList";
    signature = FText::BasicType() + " -> " + CcBool::BasicType();
    syntax = "kvsUpdateServerList( separatedList )";
    meaning =
        "updates entire server list (doesnt delete, just adds and changes "
        "information)";
  }
};
ListExpr kvsUpdateServerListTM(ListExpr args);
int kvsUpdateServerListVM(Word* args, Word& result, int message, Word& local,
                          Supplier s);

struct kvsSyncServerListInfo : OperatorInfo {
  kvsSyncServerListInfo() {
    name = "kvsSyncServerList";
    signature = " -> " + CcBool::BasicType();
    syntax = "kvsSyncServerList()";
    meaning =
        "syncs server list with master-server (needs to be set by kvsSetMaster "
        "Operator first)";
  }
};
ListExpr kvsSyncServerListTM(ListExpr args);
int kvsSyncServerListVM(Word* args, Word& result, int message, Word& local,
                        Supplier s);

struct kvsReconnectInfo : OperatorInfo {
  kvsReconnectInfo() {
    name = "kvsReconnect";
    signature = CcInt::BasicType() + " -> " + CcBool::BasicType();
    syntax = "kvsReconnect( idx )";
    meaning = "(not needed anymore)";
  }
};
ListExpr kvsReconnectTM(ListExpr args);
int kvsReconnectVM(Word* args, Word& result, int message, Word& local,
                   Supplier s);

struct kvsSetDatabaseInfo : OperatorInfo {
  kvsSetDatabaseInfo() {
    name = "kvsSetDatabase";
    signature = FText::BasicType() + " -> " + CcBool::BasicType();
    syntax = "kvsSetDatabase( databaseName )";
    meaning = "sets database name for all clients";
  }
};
ListExpr kvsSetDatabaseTM(ListExpr args);
int kvsSetDatabaseVM(Word* args, Word& result, int message, Word& local,
                     Supplier s);

struct kvsUseDatabaseInfo : OperatorInfo {
  kvsUseDatabaseInfo() {
    name = "kvsUseDatabase";
    signature = FText::BasicType() + " -> " + CcBool::BasicType();
    syntax = "kvsUseDatabase( databaseName )";
    meaning = "open database on all clients";
  }
};
ListExpr kvsUseDatabaseTM(ListExpr args);
int kvsUseDatabaseVM(Word* args, Word& result, int message, Word& local,
                     Supplier s);

struct kvsListInfo : OperatorInfo {
  kvsListInfo() {
    name = "kvsList";
    signature = " -> " + CcBool::BasicType();
    syntax = "kvsList( )";
    meaning = "lists servers";
  }
};
ListExpr kvsListTM(ListExpr args);
int kvsListVM(Word* args, Word& result, int message, Word& local, Supplier s);

//
//
//  QUAD TREE
//
//

struct qtcreatedistInfo : OperatorInfo {
  qtcreatedistInfo() {
    name = "qtcreatedist";
    signature = "(stream (tuple ((x1 t1) ... (xn tn)))) (xi " +
                Rectangle<2>::BasicType() + ") ( " +
                QuadTreeDistributionType::BasicType() + ")" + +" -> " +
                QuadTreeDistributionType::BasicType();
    syntax = "_ qtcreatedist [ _ , _ ]";
    meaning = "Assignes MBB to QuadTreeDistribution (not need anymore)";
    example =
        "Buildings feed extend[MBBox: bbox(.GeoData)] qtcreatedist[MBBox, "
        "distObj]";
    // Operator::SimpleSelect, ??
  }
};
ListExpr qtcreatedistTM(ListExpr inargs);
int qtcreatedistVM(Word* args, Word& result, int message, Word& local,
                   Supplier s);

struct qtserveridLocalInfo : OperatorInfo {
  qtserveridLocalInfo() : OperatorInfo() {
    name = "qtserveridLocal";
    signature = Rectangle<2>::BasicType() + " x " +
                QuadTreeDistributionType::BasicType() + " -> stream(int)";
    syntax = "qtserveridLocal( box, qtdistribution )";
    meaning =
        "Returns a stream of server ids of all quad tree nodes containing box  "
        "(not need anymore)";
    example =
        "Buildings feed extendstream[ServerId: qtserveridLocal(bbox(.GeoData), "
        "distribution)] ddistribute[ServerId, 12, Cluster]";
  }
};
ListExpr qtserveridLocalTM(ListExpr args);
int qtserveridLocalVM(Word* args, Word& result, int message, Word& local,
                      Supplier s);

struct qtserveridInfo : OperatorInfo {
  qtserveridInfo() : OperatorInfo() {
    name = "qtserverid";
    signature = Rectangle<2>::BasicType() + " x distribution -> stream(int)";
    syntax = "qtserverid( box, qtdistribution )";
    meaning =
        "Returns a stream of server ids of all quad tree nodes containing box  "
        "(not need anymore)";
    example =
        "Buildings feed extendstream[ServerId: qtserverid(bbox(.GeoData), "
        "distribution)] ddistribute[ServerId, 12, Cluster]";
  }
};
ListExpr qtserveridTM(ListExpr args);
int qtserveridVM(Word* args, Word& result, int message, Word& local,
                 Supplier s);

struct qtintersectsLocalInfo : OperatorInfo {
  qtintersectsLocalInfo() : OperatorInfo() {
    name = "qtintersectsLocal";
    signature = Rectangle<2>::BasicType() + " x " + Rectangle<2>::BasicType() +
                " x " + QuadTreeDistributionType::BasicType() + " -> " +
                CcBool::BasicType();
    syntax = "qtintersectsLocal( rectA, rectB, qtdistribution )";
    meaning =
        "Returns whether the current server contains the smallest common point "
        "of two overlapping rectangles  (not need anymore)";
    example =
        "... filter[qtintersectsLocal(bbox(.GeoData_A), bbox(.GeoData_B), "
        "qtdistribution)] ...";
  }
};
ListExpr qtintersectsLocalTM(ListExpr args);
int qtintersectsLocalVM(Word* args, Word& result, int message, Word& local,
                        Supplier s);

struct qtintersectsInfo : OperatorInfo {
  qtintersectsInfo() : OperatorInfo() {
    name = "qtintersects";
    signature = Rectangle<2>::BasicType() + " x " + Rectangle<2>::BasicType() +
                " x " + QuadTreeDistributionType::BasicType() + " -> " +
                CcBool::BasicType();
    syntax = "qtintersects( rectA, rectB, qtdistribution )";
    meaning =
        "Returns whether the current server contains the smallest common point "
        "of two overlapping rectangles  (not need anymore)";
    example =
        "... filter[qtintersects(bbox(.GeoData_A), bbox(.GeoData_B), "
        "qtdistribution)] ...";
  }
};
ListExpr qtintersectsTM(ListExpr args);
int qtintersectsVM(Word* args, Word& result, int message, Word& local,
                   Supplier s);

struct qtDistinctInfo : OperatorInfo {
  qtDistinctInfo() : OperatorInfo() {
    name = "qtDistinct";
    signature = QuadTreeDistributionType::BasicType() + " x " +
                Rectangle<2>::BasicType() + " -> " + CcBool::BasicType();
    syntax = "qtDistinct(qtdistribution, rect )";
    meaning =
        "checks if rectangle origin is currently assigned to server by quad "
        "tree";
    example = " ...";
    usesArgsInTypeMapping = true;
  }
};
ListExpr qtDistinctTM(ListExpr args);
int qtDistinctVM(Word* args, Word& result, int message, Word& local,
                 Supplier s);

//
//
// DISTRIBUTION
//
//
//

struct kvsServerIdInfo : OperatorInfo {
  kvsServerIdInfo() : OperatorInfo() {
    name = "kvsServerId";
    signature = "{distribution, text} x {" + Rectangle<2>::BasicType() + "," +
                CcInt::BasicType() + "} x bool -> stream(int)";
    syntax =
        "kvsServerId( {int, rect}, {distribution, text} [, requestOnly]  )";
    meaning =
        "Returns a stream of server ids that are assigned to the input "
        "parameter by the distribution. If it's not part of the distribution "
        "yet, it is added unless the optional parameter requestOnly is set to "
        "TRUE. ";
    example =
        "Buildings feed extendstream[ServerId: kvsServerId(bbox(.GeoData), "
        "distribution)] kvsDistribute[ServerId, distribution, 'Buildings', "
        "'count']";
    usesArgsInTypeMapping = true;
  }
};
ListExpr kvsServerIdTM(ListExpr args);
int kvsServerIdVM(Word* args, Word& result, int message, Word& local,
                  Supplier s);

struct kvsSaveDistInfo : OperatorInfo {
  kvsSaveDistInfo() : OperatorInfo() {
    name = "kvsSaveDist";
    signature = "distribution -> bool";
    syntax = "kvsSaveDist( distribution )";
    meaning =
        "retrieves distribution object information from application and saves "
        "it";
    example = "query kvsSaveDist(distribution)";
    usesArgsInTypeMapping = true;
  }
};
ListExpr kvsSaveDistTM(ListExpr args);
int kvsSaveDistVM(Word* args, Word& result, int message, Word& local,
                  Supplier s);

//
//
// KEY VALUE STORE
//
//

struct kvsStartAppInfo : OperatorInfo {
  kvsStartAppInfo() {
    name = "kvsStartApp";
    signature = " -> " + CcBool::BasicType();
    syntax = "kvsStartApp( )";
    meaning = "connects algebra to application";
  }
};
ListExpr kvsStartAppTM(ListExpr args);
int kvsStartAppVM(Word* args, Word& result, int message, Word& local,
                  Supplier s);

struct kvsTransferIdInfo : OperatorInfo {
  kvsTransferIdInfo() {
    name = "kvsTransferId";
    signature = " -> " + CcInt::BasicType();
    syntax = "kvsTransferId()";
    meaning = "creates transfer id for kvsRemoteStream (not used)";
  }
};
ListExpr kvsTransferIdTM(ListExpr args);
int kvsTransferIdVM(Word* args, Word& result, int message, Word& local,
                    Supplier s);

struct kvsGlobalIdInfo : OperatorInfo {
  kvsGlobalIdInfo() {
    name = "kvsGlobalId";
    signature = " -> " + CcInt::BasicType();
    syntax = "kvsGlobalId()";
    meaning = "creates global id (over multiple algebra instances)";
  }
};
ListExpr kvsGlobalIdTM(ListExpr args);
int kvsGlobalIdVM(Word* args, Word& result, int message, Word& local,
                  Supplier s);

struct kvsInitClientsInfo : OperatorInfo {
  kvsInitClientsInfo() {
    name = "kvsInitClients";
    signature = CcInt::BasicType() + " -> " + CcBool::BasicType();
    syntax = "kvsInitClients( port )";
    meaning = "trys to connect to all known clients";
  }
};
ListExpr kvsInitClientsTM(ListExpr args);
int kvsInitClientsVM(Word* args, Word& result, int message, Word& local,
                     Supplier s);

struct kvsStartClientInfo : OperatorInfo {
  kvsStartClientInfo() {
    name = "kvsStartClient";
    signature = CcInt::BasicType() + " -> " + CcBool::BasicType();
    syntax = "kvsStartClient( port )";
    meaning = "opens key-value-store port";
  }
};
ListExpr kvsStartClientTM(ListExpr args);
int kvsStartClientVM(Word* args, Word& result, int message, Word& local,
                     Supplier s);

struct kvsStopClientInfo : OperatorInfo {
  kvsStopClientInfo() {
    name = "kvsStopClient";
    signature = CcInt::BasicType() + " -> " + CcBool::BasicType();
    syntax = "kvsStopClient( port )";
    meaning = "closes key-value-store port";
  }
};
ListExpr kvsStopClientTM(ListExpr args);
int kvsStopClientVM(Word* args, Word& result, int message, Word& local,
                    Supplier s);

struct kvsSetIdInfo : OperatorInfo {
  kvsSetIdInfo() {
    name = "kvsSetId";
    signature = CcInt::BasicType() + " -> " + CcBool::BasicType();
    syntax = "kvsSetId( id )";
    meaning = "sets server id which is assigned by distribution";
  }
};
ListExpr kvsSetIdTM(ListExpr args);
int kvsSetIdVM(Word* args, Word& result, int message, Word& local, Supplier s);

struct kvsSetMasterInfo : OperatorInfo {
  kvsSetMasterInfo() {
    name = "kvsSetMaster";
    signature = FText::BasicType() + " x " + CcInt::BasicType() + " x " +
                CcInt::BasicType() + " -> " + CcBool::BasicType();
    syntax = "kvsSetMaster( host, interfacePort, kvsPort )";
    meaning = "sets master-server conneciton information";
  }
};
ListExpr kvsSetMasterTM(ListExpr args);
int kvsSetMasterVM(Word* args, Word& result, int message, Word& local,
                   Supplier s);

struct kvsRetrieveInfo : OperatorInfo {
  kvsRetrieveInfo() {
    name = "kvsRetrieve";
    signature = Stream<Tuple>::BasicType() + " x distribution x region-> " +
                Stream<Tuple>::BasicType() + "\n" + Stream<Tuple>::BasicType() +
                " x symbol -> " + Stream<Tuple>::BasicType() + "\n";
    syntax = "_ kvsRetrieve( {serverIdAttribute, distribution} [,region] )";
    meaning = "supposed to sync queries with master server (not tested)";
    usesArgsInTypeMapping = true;
  }
};
ListExpr kvsRetrieveTM(ListExpr args);
int kvsRetrieveSelect(ListExpr args);
int kvsRetrieveByIdVM(Word* args, Word& result, int message, Word& local,
                      Supplier s);
int kvsRetrieveByRegionVM(Word* args, Word& result, int message, Word& local,
                          Supplier s);

struct kvsExecInfo : OperatorInfo {
  kvsExecInfo() {
    name = "kvsExec";
    signature = FText::BasicType() + " -> " + CcBool::BasicType();
    syntax = "kvsExec( command )";
    meaning = "execute command on clients (not tested)";
  }
};
ListExpr kvsExecTM(ListExpr args);
int kvsExecVM(Word* args, Word& result, int message, Word& local, Supplier s);

struct kvsFilterInfo : OperatorInfo {
  kvsFilterInfo() {
    name = "kvsFilter";
    signature = Stream<Tuple>::BasicType() +
                " x distribution x region x int x bool -> " +
                Stream<Tuple>::BasicType();
    syntax =
        "_ kvsFilter( {distribution, distributionName} ,region ,globalId "
        "[,updateDistribution] )";
    meaning =
        "used to update data state or filter data thats known by target "
        "clients";
    usesArgsInTypeMapping = true;
  }
};
ListExpr kvsFilterTM(ListExpr args);
int kvsFilterVM(Word* args, Word& result, int message, Word& local, Supplier s);

//
//
// NETWORKSTREAMS & DISTRIBUTION
//
//

struct kvsRemoteStreamInfo : OperatorInfo {
  kvsRemoteStreamInfo() {
    name = "kvsRemoteStream";
    signature = CcInt::BasicType() + " -> " +
                Stream<Tuple>::BasicType();  // returns stream tupel
    syntax = "kvsRemoteStream ( streamId )";
    meaning = "used to receive data";
    usesArgsInTypeMapping = true;
  }
};

ListExpr kvsRemoteStreamTM(ListExpr args);
int kvsRemoteStreamVM(Word* args, Word& result, int message, Word& local,
                      Supplier s);

struct kvsRemoteStreamSCPInfo : OperatorInfo {
  kvsRemoteStreamSCPInfo() {
    name = "kvsRemoteStreamSCP";
    signature = CcInt::BasicType() + " -> " +
                Stream<Tuple>::BasicType();  // returns stream tupel
    syntax = "kvsRemoteStreamSCP ( streamId )";
    meaning = "(not tested)";
    usesArgsInTypeMapping = true;
  }
};
ListExpr kvsRemoteStreamSCPTM(ListExpr args);
int kvsRemoteStreamSCPVM(Word* args, Word& result, int message, Word& local,
                         Supplier s);

struct kvsDistributeInfo : OperatorInfo {
  kvsDistributeInfo() {
    name = "kvsDistribute";
    signature = "stream (tuple (...)) x " + CcInt::BasicType() +
                " x distribution x " + FText::BasicType() + " x " +
                FText::BasicType() +
                " -> stream (tuple (...))\n"
                "stream (tuple (...)) x " +
                CcInt::BasicType() + " x distribution x " + FText::BasicType() +
                " x " + FText::BasicType() + " x " + FText::BasicType() +
                " -> stream (tuple (...))\n"
                "stream (tuple (...)) x " +
                CcInt::BasicType() + " x distribution x " + FText::BasicType() +
                " x " + FText::BasicType() + " x " + CcBool::BasicType() +
                " -> stream (tuple (...))\n"
                "stream (tuple (...)) x " +
                CcInt::BasicType() + " x distribution x " + FText::BasicType() +
                " x " + FText::BasicType() + " x " + FText::BasicType() +
                " x " + CcBool::BasicType() +
                " -> stream (tuple (...))\n"
                "stream (tuple (...)) x " +
                CcInt::BasicType() + " x " + FText::BasicType() + " x " +
                FText::BasicType() + " x " + FText::BasicType() +
                " -> stream (tuple (...))\n"
                "stream (tuple (...)) x " +
                CcInt::BasicType() + " x " + FText::BasicType() + " x " +
                FText::BasicType() + " x " + FText::BasicType() + " x " +
                FText::BasicType() +
                " -> stream (tuple (...))\n"
                "stream (tuple (...)) x " +
                CcInt::BasicType() + " x " + FText::BasicType() + " x " +
                FText::BasicType() + " x " + FText::BasicType() + " x " +
                CcBool::BasicType() +
                " -> stream (tuple (...))\n"
                "stream (tuple (...)) x " +
                CcInt::BasicType() + " x " + FText::BasicType() + " x " +
                FText::BasicType() + " x " + FText::BasicType() + " x " +
                FText::BasicType() + " x " + CcBool::BasicType() +
                " -> stream (tuple (...))";

    // signature = "stream (tuple (...)) x " + CcInt::BasicType() + " x {" +
    // QuadTreeDistributionType::BasicType() + "," + FText::BasicType() + "} x "
    // + FText::BasicType() + " x " + FText::BasicType() +
    //      "[ x " + FText::BasicType() + "] [x " + FText::BasicType() +"] ->
    //      stream (tuple (...))";

    syntax =
        "_ kvsDistribute [ serverIdAttribute, {distribution, distributionName} "
        ", targetRelationName, clientCommand, [deleteCommand,] [restructure] ]";
    meaning = "distribute data stream";
    usesArgsInTypeMapping = true;
  }
};
ListExpr kvsDistributeTM(ListExpr args);
int kvsDistributeVM(Word* args, Word& result, int message, Word& local,
                    Supplier s);

/*

  DATA SOURCE

*/

struct kvsDataSourceSCPInfo : OperatorInfo {
  kvsDataSourceSCPInfo() {
    name = "kvsDataSourceSCP";
    signature = " -> " + Stream<Tuple>::BasicType();
    syntax = "kvsDataSourceSCP ( )";
    meaning = "(not tested)";
    usesArgsInTypeMapping = true;
  }
};

ListExpr kvsDataSourceSCPTM(ListExpr args);
int kvsDataSourceSCPVM(Word* args, Word& result, int message, Word& local,
                       Supplier s);
}

#endif /* ALGEBRAOPERATORS_H_ */
