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

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include "SocketIO.h"
#include "RectangleAlgebra.h"
#include "RelationAlgebra.h"

#include "FTextAlgebra.h"
#include "Stream.h"

#include "TypeMapUtils.h"
#include "Symbols.h"

#include <string>
#include <vector>
#include <functional>
#include <fstream>

#include "boost/thread.hpp"
#include "boost/date_time.hpp"

#include "AlgebraOperators.h"

#include "KeyValueStoreIPC.h"

namespace KVS {

qtdistributionInfo qtdi;
qtdistributionFunctions qtdf;
TypeConstructor qtdistributionTC(qtdi, qtdf);

qtnodeInfo qtni;
qtnodeFunctions qtnf;
TypeConstructor qtnodeTC(qtni, qtnf);

KeyValueStoreIPC* kvsIPC;

/*


5 Implementation of the Algebra Class

*/

class KeyValueStoreAlgebra : public Algebra {
 public:
  KeyValueStoreAlgebra() : Algebra() {
    /*

    Registration of Types

    */

    AddTypeConstructor(&qtdistributionTC);
    AddTypeConstructor(&qtnodeTC);

    //! is this a good idea? seems wrong
    qtdistributionTC.AssociateKind(Kind::SIMPLE());
    qtnodeTC.AssociateKind(Kind::SIMPLE());

    /*

     Registration of Operators

    */
    /*
     * ServerOperators
     */
    // AddOperator(kvsAddInfo(), kvsAddVM, kvsAddTM);
    // AddOperator(kvsRemoveInfo(), kvsRemoveVM, kvsRemoveTM);
    // AddOperator(kvsReconnectInfo(), kvsReconnectVM, kvsReconnectTM);
    AddOperator(kvsUpdateServerListInfo(), kvsUpdateServerListVM,
                kvsUpdateServerListTM);
    AddOperator(kvsSyncServerListInfo(), kvsSyncServerListVM,
                kvsSyncServerListTM);
    AddOperator(kvsSetDatabaseInfo(), kvsSetDatabaseVM, kvsSetDatabaseTM);
    AddOperator(kvsUseDatabaseInfo(), kvsUseDatabaseVM, kvsUseDatabaseTM);
    AddOperator(kvsListInfo(), kvsListVM, kvsListTM);

    /*
     * Quad Tree
     */
    // AddOperator(qtcreatedistInfo(), qtcreatedistVM, qtcreatedistTM);
    // AddOperator(qtserveridLocalInfo(), qtserveridLocalVM, qtserveridLocalTM);
    // AddOperator(qtserveridInfo(), qtserveridVM, qtserveridTM);
    // AddOperator(qtintersectsLocalInfo(), qtintersectsLocalVM,
    //            qtintersectsLocalTM);
    // AddOperator(qtintersectsInfo(), qtintersectsVM, qtintersectsTM);
    AddOperator(qtDistinctInfo(), qtDistinctVM, qtDistinctTM);

    /*
     * Distribution
     */
    AddOperator(kvsServerIdInfo(), kvsServerIdVM, kvsServerIdTM);
    AddOperator(kvsSaveDistInfo(), kvsSaveDistVM, kvsSaveDistTM);
    AddOperator(kvsDistStreamInfo(), kvsDistStreamVM, kvsDistStreamTM);
    AddOperator(kvsFilterInfo(), kvsFilterVM, kvsFilterTM);

    /*
     * Key Value Store
     */
    AddOperator(kvsStartAppInfo(), kvsStartAppVM, kvsStartAppTM);
    AddOperator(kvsTransferIdInfo(), kvsTransferIdVM, kvsTransferIdTM);
    AddOperator(kvsGlobalIdInfo(), kvsGlobalIdVM, kvsGlobalIdTM);
    AddOperator(kvsInitClientsInfo(), kvsInitClientsVM, kvsInitClientsTM);
    AddOperator(kvsStartClientInfo(), kvsStartClientVM, kvsStartClientTM);
    AddOperator(kvsStopClientInfo(), kvsStopClientVM, kvsStopClientTM);
    AddOperator(kvsSetIdInfo(), kvsSetIdVM, kvsSetIdTM);
    AddOperator(kvsSetMasterInfo(), kvsSetMasterVM, kvsSetMasterTM);

    ValueMapping retrieveFuns[] = {kvsRetrieveByIdVM, kvsRetrieveByRegionVM, 0};
    AddOperator(kvsRetrieveInfo(), retrieveFuns, kvsRetrieveSelect,
                kvsRetrieveTM);

    AddOperator(kvsExecInfo(), kvsExecVM, kvsExecTM);

    /*
     * Networkstreams & Distribution
     */
    AddOperator(kvsRemoteStreamInfo(), kvsRemoteStreamVM, kvsRemoteStreamTM);
    // AddOperator(kvsRemoteStreamSCPInfo(), kvsRemoteStreamSCPVM,
    //            kvsRemoteStreamSCPTM);
    AddOperator(kvsDistributeInfo(), kvsDistributeVM, kvsDistributeTM);

    /*
     * Data Source
     */
    // AddOperator(kvsDataSourceSCPInfo(), kvsDataSourceSCPVM,
    // kvsDataSourceSCPTM);
  }
  ~KeyValueStoreAlgebra() {
    if (kvsIPC) {
      delete kvsIPC;
      kvsIPC = 0;
    }
  };
};

KeyValueStoreAlgebra* algInstance;

}  // end of namespace ~KVS~

extern "C" Algebra* InitializeKeyValueStoreAlgebra(NestedList* nlRef,
                                                   QueryProcessor* qpRef) {
  KVS::algInstance = new KVS::KeyValueStoreAlgebra;
  KVS::kvsIPC = new KVS::KeyValueStoreIPC(0);

  RTFlag::setFlag("Server:BinaryTransfer", true);

  return KVS::algInstance;
}
