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
#include "Algebra.h"
#include "NestedList.h"
#include "Symbols.h"

#include "Algebras/Relation-C++/OperatorFeed.h"
#include "Algebras/Relation-C++/OperatorFilter.h"
#include "Algebras/Relation-C++/OperatorProject.h"

#include "Algebras/DBService/DBServiceConnector.hpp"
#include "Algebras/DBService/OperatorFeedPF.hpp"

namespace DBService
{

/*
5.8 Operator ~feedpf~

Applies project and filter conditions to a relation in order
to provide a tuple stream.
Retrieves replica location from DBService if primary node storing
the relation is not available, and reads them from the respective file.

5.8.1 Type mapping function of operator ~feedpf~

Result type of project filter operation.

----    ((relation (tuple x)) (map (tuple x) bool)) (( <project> ))
               -> (stream (tuple x))
----

*/

ListExpr OperatorFeedPF::mapType(ListExpr nestedList)
{
    cout << listutils::stringValue(nestedList) << endl;

    if (!nl->HasLength(nestedList, 3))
    {
        ErrorReporter::ReportError(
                "expected signature: <TODO> x <TODO> x <TODO>");
                return nl->TypeError();
    }

    ListExpr feedInput = nl->First(nestedList);
    ListExpr filterConditions = nl->Third(nestedList);
    ListExpr projectAttributes = nl->Second(nestedList);

    DBServiceConnector::getInstance();

    ListExpr feedResult = OperatorFeed::FeedTypeMap(feedInput);
    if (feedResult == nl->TypeError())
    {
        // TODO -> contact DBServiceConnector and retrieve stream from there
    }
    ListExpr filterInput = nl->TwoElemList(
            feedResult,
            filterConditions);
    ListExpr filterResult = OperatorFilter::FilterTypeMap(filterInput);
    if(filterResult == nl->TypeError())
    {
        // TODO
    }
    ListExpr projectInput = nl->TwoElemList(filterResult, projectAttributes);
    ListExpr projectResult = OperatorProject::ProjectTypeMap(projectInput);
    if(projectResult == nl->TypeError())
    {
        // TODO
    }
    //TODO expand nested list with some argument for finding the replica
    //TODO clarify whether this is necessary also in case of success
    return projectResult;
}

int OperatorFeedPF::mapValue(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s)
{
    // TODO check input (type mapping result) -> contact DBService if necessary

    Word feedArgs; // TODO
    Word feedResult;
    int rc = OperatorFeed::Feed(&feedArgs, feedResult, message, local, s);
    if(rc != 0)
    {
        // TODO or check DBService?
        return rc;
    }

    Word filterArgs;// TODO some combination of args & feedResult
    Word filterResult;
    rc = OperatorFilter::Filter(&filterArgs,
                                filterResult,
                                message,
                                local,
                                s);
    if(rc != 0)
    {
        return rc;
    }

    Word projectArgs;// TODO some combination of args & filterResult
    Word projectResult;
    rc = OperatorProject::Project(&projectArgs,
                                  projectResult,
                                  message,
                                  local,
                                  s);
    if(rc != 0)
    {
        return rc;
    }
    result = projectResult;// TODO assign result variable correctly
    return 0;
}

} /* namespace DBService */
