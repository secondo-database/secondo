/*
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


//[$][\$]
//[_][\_]

*/
#include "Algebras/DBService/DBServiceAlgebra.hpp"
#include "Algebras/DBService/OperatorAddNode.hpp"
#include "Algebras/DBService/OperatorCheckDBServiceStatus.hpp"
#include "Algebras/DBService/OperatorFeedPF.hpp"
#include "Algebras/DBService/OperatorGetConfigParam.hpp"
#include "Algebras/DBService/OperatorInitDBServiceWorker.hpp"
#include "Algebras/DBService/OperatorLetDConsume.hpp"
#include "Algebras/DBService/OperatorStartDBService.hpp"

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
    AddOperator(FeedPFInfo(),
                OperatorFeedPF::mapValue,
                OperatorFeedPF::mapType);
    AddOperator(LetDConsumeInfo(),
                OperatorLetDConsume::letdconsumeVM,
                OperatorLetDConsume::selectFunction,
                OperatorLetDConsume::mapType);
    AddOperator(GetConfigParamInfo(),
                OperatorGetConfigParam::mapValue,
                OperatorGetConfigParam::mapType);
    AddOperator(InitDBServiceWorkerInfo(),
                OperatorInitDBServiceWorker::mapValue,
                OperatorInitDBServiceWorker::mapType);
    AddOperator(StartDBServiceInfo(),
                OperatorStartDBService::mapValue,
                OperatorStartDBService::mapType);
}

} /* namespace DBService */
