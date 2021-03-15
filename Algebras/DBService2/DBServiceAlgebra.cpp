/*

1.1.1 Class Implementation

----
This file is part of SECONDO.

Copyright (C) 2017,
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
#include "Algebras/DBService2/DBServiceAlgebra.hpp"
#include "Algebras/DBService2/OperatorAddNode.hpp"
#include "Algebras/DBService2/OperatorCheckDBServiceStatus.hpp"
#include "Algebras/DBService2/OperatorDBSARG.hpp"
#include "Algebras/DBService2/OperatorDBRARG.hpp"
#include "Algebras/DBService2/OperatorRELARG.hpp"
#include "Algebras/DBService2/OperatorDDelete.hpp"
#include "Algebras/DBService2/OperatorDDeleteDB.hpp"
#include "Algebras/DBService2/OperatorGetConfigParam.hpp"
#include "Algebras/DBService2/OperatorInitDBServiceWorker.hpp"
#include "Algebras/DBService2/OperatorPingDBService.hpp"
#include "Algebras/DBService2/OperatorRead.hpp"
#include "Algebras/DBService2/OperatorRead2.hpp"
#include "Algebras/DBService2/OperatorSetTraceLevel.hpp"
#include "Algebras/DBService2/OperatorStartDBService.hpp"
#include "Algebras/DBService2/OperatorWrite.hpp"
#include "Algebras/DBService2/OperatorRderive.hpp"
#include "Algebras/DBService2/OperatorDBIARG.hpp"
#include "Algebras/DBService2/OperatorRead3_X.hpp"
#include "Algebras/DBService2/OperatorUseIncrementalMetadataUpdate.hpp"
#include "Algebras/DBService2/OperatorTestDBService.hpp"

#include <loguru.hpp>
#include <loguru.cc> // linker error if this isn't included at least once.

namespace DBService
{

DBServiceAlgebra::DBServiceAlgebra() :
        Algebra()
{
    AddOperator(AddNodeInfo(),
                OperatorAddNode::mapValue,
                OperatorAddNode::mapType);
    AddOperator(CheckDBServiceStatusInfo(),
                OperatorCheckDBServiceStatus::mapValue,
                OperatorCheckDBServiceStatus::mapType);
    AddOperator(WriteInfo(),
                OperatorWrite::mapValue,
                OperatorWrite::mapType);
    AddOperator(GetConfigParamInfo(),
                OperatorGetConfigParam::mapValue,
                OperatorGetConfigParam::mapType);
    AddOperator(InitDBServiceWorkerInfo(),
                OperatorInitDBServiceWorker::mapValue,
                OperatorInitDBServiceWorker::mapType);
    AddOperator(ReadInfo(),
                OperatorRead::mapValue,
                OperatorRead::mapType);
    AddOperator(DDeleteInfo(),
                OperatorDDelete::mapValue,
                OperatorDDelete::mapType);

    AddOperator(DDeleteDBInfo(),
                OperatorDDeleteDB::mapValue,
                OperatorDDeleteDB::mapType);
    AddOperator(SetTraceLevelInfo(),
                OperatorSetTraceLevel::mapValue,
                OperatorSetTraceLevel::mapType);
    AddOperator(PingDBServiceInfo(),
                OperatorPingDBService::mapValue,
                OperatorPingDBService::mapType);
    AddOperator(Read2Info(),
                OperatorRead2::mapValue,
                OperatorRead2::mapType)->SetUsesArgsInTypeMapping();
    AddOperator(StartDBServiceInfo(),
                OperatorStartDBService::mapValue,
                OperatorStartDBService::mapType);
    AddOperator(UseIncrementalMetadataUpdateInfo(),
                OperatorUseIncrementalMetadataUpdate::mapValue,
                OperatorUseIncrementalMetadataUpdate::mapType);

    AddOperator(TestDBServiceInfo(),
                OperatorTestDBService::mapValue,
                OperatorTestDBService::mapType);

    AddOperator(DBSARGInfo(),
                0,
                OperatorDBSARG::mapType);
    AddOperator(DBRARGInfo(),
                0,
                OperatorDBRARG::mapType);

    AddOperator(RELARGInfo(),
                0,
                OperatorRELARG::mapType);
    AddOperator(RderiveInfo(),
                OperatorRderive::mapValue,
                OperatorRderive::mapType);
    

    AddOperator(Read3_XInfo<0>(),
                OperatorRead3_X<0>::mapValue,
                OperatorRead3_X<0>::mapType);

    AddOperator(DBIARGInfo<1>(),
                0,
                OperatorDBIARG<1>::mapType);

    AddOperator(Read3_XInfo<1>(),
                OperatorRead3_X<1>::mapValue,
                OperatorRead3_X<1>::mapType);

    AddOperator(DBIARGInfo<2>(),
                0,
                OperatorDBIARG<2>::mapType);

    AddOperator(Read3_XInfo<2>(),
                OperatorRead3_X<2>::mapValue,
                OperatorRead3_X<2>::mapType);

    //TODO move to initLogger function
     //TODOMove to function "initializeLogger"
    char* argv[] = { (char*)"DBService", NULL };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    loguru::init(argc, argv);

    loguru::add_file("dbservice.log", loguru::Append,
            loguru::Verbosity_MAX);

}

} /* namespace DBService */
