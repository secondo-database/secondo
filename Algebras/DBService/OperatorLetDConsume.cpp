/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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
#include "OperatorLetDConsume.hpp"

#include "OperatorConsume.h"

#include "DBServiceManager.hpp"

using namespace std;

namespace DBService
{

ListExpr OperatorLetDConsume::mapType(ListExpr nestedList)
{
    cout << listutils::stringValue(nestedList) << endl;

    if (nl->ListLength(nestedList) != 2)
    {
        ErrorReporter::ReportError(
                "expected signature: stream(tuple(...)) x string");
        return nl->TypeError();
    }

    if (!Stream<Tuple>::checkType(nl->First(nestedList)))
    {
        ErrorReporter::ReportError(
                "first argument must be: stream(tuple(...))");
        return nl->TypeError();
    }

    if(!CcString::checkType(nl->Second(nestedList)))
    {
        ErrorReporter::ReportError(
                "second argument must be: string");
        return nl->TypeError();
    }

    DBServiceManager::getInstance();

    // TODO append string (relation name) to resulting NestedList
    return OperatorConsume::ConsumeTypeMap<false>(nl->First(nestedList));
}

int OperatorLetDConsume::mapValue(Word* args,
                                  Word& result,
                                  int message,
                                  Word& local,
                                  Supplier s)
{
    // TODO create files
    return OperatorConsume::Consume(args, result, message, local, s);
}

} /* namespace DBService */
