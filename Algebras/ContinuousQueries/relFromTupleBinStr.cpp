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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of operation relFromTupleBinStr.

[toc]

1 Operation relFromTupleBinStr implementation

*/


// #include "Algebras/Relation-C++/RelationAlgebra.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include <boost/algorithm/string.hpp>

// #include "Stream.h"
// #include "DistributeStreamServer.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


namespace continuousqueries {

/*
1.1 Operation relFromTupleBinStr TypeMapping

*/

ListExpr relFromTupleBinStr_TM(ListExpr args) {
    // the list is coded as ( (<type> <query part>) (<type> <query part>) ...)

    std::cout << "relFromTupleBinStr: TypeMapping" << endl;
    std::cout << "Argument: " << nl->ToString(args) << endl;

    // Check for text x text -> rel
    if (!nl->HasLength(args, 2))
        return listutils::typeError(" two arguments are expected");
    
    ListExpr arg1Type = nl->First(args);
    ListExpr arg2Type = nl->Second(args);

    // Check first argument (tupledescription/text)
    if (!nl->HasLength(arg1Type, 2))
        return listutils::typeError(" internal error (attrlist) ");

    if (!FText::checkType(nl->First(arg1Type)))
        return listutils::typeError(" attrlist as text expected");

    ListExpr tupledescr;
    nl->ReadFromString(
        listutils::stringValue(nl->Second(arg1Type)), 
        tupledescr
    );

    if (!listutils::isAttrList(tupledescr))
        return listutils::typeError(" not a valid tuple description");

    // Check second argument (binstr/text)
    if (!nl->HasLength(arg2Type, 2))
        return listutils::typeError(" internal error (binstr) ");

    if (!FText::checkType(nl->First(arg2Type)))
        return listutils::typeError(" binary string as text expected");

    return  nl->TwoElemList(
        nl->SymbolAtom(TempRelation::BasicType()),
        nl->TwoElemList( 
            listutils::basicSymbol<Tuple>(),
            tupledescr
        )
    );
}

/*
1.2 Operation relFromTupleBinStr ValueMappings

*/

int relFromTupleBinStr_VM(Word* args, Word& result, int message,
              Word& local, Supplier s) {

    std::cout << "relFromTupleBinStr: ValueMapping" << endl;

    FText* fttupledescr  = static_cast<FText*>(args[0].addr);  
    FText* fttuplebinstr = static_cast<FText*>(args[1].addr);  
    std::string tuplebinstr = fttuplebinstr->GetValue();

    ListExpr resulttype;
    nl->ReadFromString(fttupledescr->GetValue(), resulttype);

    ListExpr resultTupleType = nl->TwoElemList(
        nl->SymbolAtom(Tuple::BasicType()),
        resulttype
    );

    ListExpr numResultTupleType = SecondoSystem::GetCatalog()
        ->NumericType(resultTupleType);

    TupleType* tt = new TupleType(numResultTupleType);
    Tuple* t = new Tuple(tt);

    t->ReadFromBinStr(0, tuplebinstr);
    
    Relation* rel = new Relation(tt);
    
    rel->Clear();
    rel->AppendTuple(t);
    
    t->DeleteIfAllowed();
    tt->DeleteIfAllowed();

    result.setAddr(rel);

    return 0;
}


/*
1.3 Operation relFromTupleBinStr operator selection array

*/
            
ValueMapping relFromTupleBinStr[] = {
    relFromTupleBinStr_VM
};

/*
1.4 Operation relFromTupleBinStr operator selection

*/

int relFromTupleBinStr_Select(ListExpr args) {
    return 0;
}

/*
1.5 Operation relFromTupleBinStr operator specification

*/

const std::string relFromTupleBinStrOpSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>text x text -> rel(tuple())</text--->"
"<text>relFromTupleBinStr (  _, _ )</text--->"
"<text>Converts a binary string representation of a tuple to a "
"relatation filled with this single tuple. </text--->"
"<text>query relFromTupleBinStr('((No int))', 'CwAAAAUAAQAAADE=')"
"</text--->"
") )";


/*
1.6 Operation relFromTupleBinStr

*/

Operator relFromTupleBinStr_Op(
    "relFromTupleBinStr",
    relFromTupleBinStrOpSpec,
    1,
    relFromTupleBinStr,
    relFromTupleBinStr_Select,
    relFromTupleBinStr_TM
);

} /* end of namespace */
