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
#include "Algebras/DBService/DBServiceAlgebra.hpp"
#include "Algebras/DBService/OperatorAddNode.hpp"
#include "Algebras/DBService/OperatorCheckDBServiceStatus.hpp"
#include "Algebras/DBService/OperatorDBSARG.hpp"
#include "Algebras/DBService/OperatorDBRARG.hpp"
#include "Algebras/DBService/OperatorRELARG.hpp"
#include "Algebras/DBService/OperatorDDelete.hpp"
#include "Algebras/DBService/OperatorGetConfigParam.hpp"
#include "Algebras/DBService/OperatorInitDBServiceWorker.hpp"
#include "Algebras/DBService/OperatorPingDBService.hpp"
#include "Algebras/DBService/OperatorRead.hpp"
#include "Algebras/DBService/OperatorRead2.hpp"
#include "Algebras/DBService/OperatorRead3.hpp"
#include "Algebras/DBService/OperatorSetTraceLevel.hpp"
#include "Algebras/DBService/OperatorStartDBService.hpp"
#include "Algebras/DBService/OperatorWrite.hpp"
#include "Algebras/DBService/OperatorRderive.hpp"
#include "Algebras/DBService/OperatorUseIncrementalMetadataUpdate.hpp"

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
    AddOperator(SetTraceLevelInfo(),
                OperatorSetTraceLevel::mapValue,
                OperatorSetTraceLevel::mapType);
    AddOperator(PingDBServiceInfo(),
                OperatorPingDBService::mapValue,
                OperatorPingDBService::mapType);
    AddOperator(Read2Info(),
                OperatorRead2::mapValue,
                OperatorRead2::mapType)->SetUsesArgsInTypeMapping();
    AddOperator(Read3Info(),
                OperatorRead3::mapValue,
                OperatorRead3::mapType)->SetUsesArgsInTypeMapping();
    AddOperator(StartDBServiceInfo(),
                OperatorStartDBService::mapValue,
                OperatorStartDBService::mapType);
    AddOperator(UseIncrementalMetadataUpdateInfo(),
                OperatorUseIncrementalMetadataUpdate::mapValue,
                OperatorUseIncrementalMetadataUpdate::mapType);

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
}

} /* namespace DBService */
