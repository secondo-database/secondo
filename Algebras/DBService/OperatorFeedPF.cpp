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
#include "OperatorFeedPF.hpp"
#include "Algebra.h"
#include "NList.h"
#include "Symbols.h"
//#include "NestedList.h"

namespace DBService
{

/*
1 Operator ~feedpf~

1.1 Constructor

*/

OperatorFeedPF::OperatorFeedPF()
{
    // TODO Auto-generated constructor stub

}

/*
1.2 Destructor

*/

OperatorFeedPF::~OperatorFeedPF()
{
    // TODO Auto-generated destructor stub
}

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

ListExpr OperatorFeedPF::checkFirstArgumentIsRelation(ListExpr nestedList)
{
    if (listutils::isRelDescription(nestedList, true)
            || listutils::isRelDescription(nestedList, false))
    {
        return nl->Cons(nl->SymbolAtom(Symbol::STREAM()), nl->Rest(nestedList));
    }
    if (listutils::isOrelDescription(nestedList))
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                               nl->Second(nestedList));
    ErrorReporter::ReportError("rel(tuple(...)), trel(tuple(...)) or "
                               "orel(tuple(...)) expected");
    return nl->TypeError();
}

ListExpr OperatorFeedPF::mapType(ListExpr nestedList)
{
    if (!nl->HasLength(nestedList, 3))
    {
        return listutils::typeError(
                "operator feedpf requires three arguments");
    }

    ListExpr first = nl->First(nestedList);

    ListExpr rel = checkFirstArgumentIsRelation(first);
    if(rel == nl->TypeError())
    {
        return rel;
    }

    //ListExpr second  = nl->Second(nestedList);
    // TODO check that second argument contains the attributes for projection

    //ListExpr third  = nl->Third(nestedList);
    // TODO check that third argument contains the filter arguments

    return 0;
}

SelectFunction OperatorFeedPF::selectFunction()
{
    return 0;
}

ValueMapping* OperatorFeedPF::mapValue()
{
    return 0;
}

} /* namespace DBService */
