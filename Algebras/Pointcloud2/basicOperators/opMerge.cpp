/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 Implementation of the Merge Operator

*/
#include <set>
#include "opMerge.h"


using namespace pointcloud2;

extern NestedList *nl;
extern QueryProcessor *qp;

/*
1.7 The Pointcloud2 Merge Operator

*/
ListExpr op_merge::mergeTM(ListExpr args) {

    const std::string err("pointcloud2 x pointcloud2 expected");

    if (!nl->HasLength(args,2)) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    ListExpr errorInfo = nl->TheEmptyList();
    const ListExpr pc1 = nl->First(args);
    if (!Pointcloud2::TypeCheck(pc1, errorInfo)) {
        return listutils::typeError(err
                + " (first argument is not a pointcloud2)");
    }

    const ListExpr pc2 = nl->Second(args);
    if (!Pointcloud2::TypeCheck(pc2, errorInfo)) {
        return listutils::typeError(err
                    + " (second argument is not a pointcloud2) ");
    }

    //Do both pointcloud2s have the same reference system
    Referencesystem::Type pc1ref, pc2ref;
    try {
        pc1ref = nl->IsAtom(nl->Second(pc1))
                ? Referencesystem::toEnum(nl->SymbolValue(nl->Second(pc1)))
                : Referencesystem::toEnum(
                        nl->SymbolValue(nl->First(nl->Second(pc1))));
        pc2ref = nl->IsAtom(nl->Second(pc2))
                ? Referencesystem::toEnum(nl->SymbolValue(nl->Second(pc2)))
                : Referencesystem::toEnum(
                        nl->SymbolValue(nl->First(nl->Second(pc2))));
        if (pc1ref!=pc2ref) {
            return listutils::typeError(err
                    + " (2 different reference systems)");
        }
    } catch(std::invalid_argument&) {
        return listutils::typeError(err
                    + " (impossible reference system error)");
    }

    ListExpr appendList = nl->OneElemList(nl->IntAtom(0));

    //Do both pointcloud2s have attributes
    if ((!nl->HasLength(nl->Second(pc1), 2)) ||
            (!nl->HasLength(nl->Second(pc2), 2))){
        return nl->ThreeElemList(
                        nl->SymbolAtom(Symbols::APPEND()),
                        appendList,
                        nl->TwoElemList(
                            listutils::basicSymbol<Pointcloud2>(),
                            nl->SymbolAtom(Referencesystem::toString(pc1ref))
                        )
                );
    }

    const ListExpr pc1TupletypeInfo = nl->Second(nl->Second(pc1));
    if (!listutils::isTupleDescription(pc1TupletypeInfo)){
        return listutils::typeError(err + " (error in pc1 tuple description)");
    }
    const ListExpr pc2TupletypeInfo = nl->Second(nl->Second(pc2));
    if (!listutils::isTupleDescription(pc2TupletypeInfo)){
        return listutils::typeError(err + " (error in pc2 tuple description)");
    }

    ListExpr pc1Attrs = nl->Second(pc1TupletypeInfo);
    const ListExpr pc2Attrs = nl->Second(pc2TupletypeInfo);

    //Are attributes not empty
    if ((nl->IsEmpty(pc1Attrs)) || (nl->IsEmpty(pc2Attrs))) {
        return nl->ThreeElemList(
                        nl->SymbolAtom(Symbols::APPEND()),
                        appendList,
                        nl->TwoElemList(
                            listutils::basicSymbol<Pointcloud2>(),
                            nl->SymbolAtom(Referencesystem::toString(pc1ref))
                        )
                );
    }

//    //Differs the attribute structure (aka tuple schema) of the two pc2s
//    TupleType* tt1 = new TupleType(pc1TupletypeInfo);
//    TupleType* tt2 = new TupleType(pc2TupletypeInfo);
//    if (tt1->equalSchema(*tt2)) {
//        delete tt1, tt2;
//        appendList = nl->OneElemList(nl->IntAtom(-1));
//        const ListExpr type = nl->TwoElemList(
//                                    listutils::basicSymbol<Pointcloud2>(),
//                                    nl->TwoElemList(
//                                          nl->SymbolAtom(
//                                          Referencesystem::toString(pc1ref)),
//                                          pc1TupletypeInfo));
//        return nl->ThreeElemList(
//                                nl->SymbolAtom(Symbols::APPEND()),
//                                appendList,
//                                type);
//    }
//    delete tt1, tt2;

    //What attributes do both pc2s have.
    //Creating 3 lists. The attribute list for tuple type of result pc2,
    //and the index lists for append mechanism.
    ListExpr attrList = nl->TheEmptyList();
    ListExpr indexListpc1 = nl->TheEmptyList();
    ListExpr indexListpc2 = nl->TheEmptyList();
    ListExpr lastAttr,lastIndex1, lastIndex2;
    int pc1AttrIndex = 0;
    while (!nl->IsEmpty(pc1Attrs)) {
        const ListExpr current = nl->First(pc1Attrs);
        pc1AttrIndex++;
        if ((!nl->HasLength(current,2))
                || !listutils::isSymbol(nl->First(current))
                || !listutils::isSymbol(nl->Second(current))){
            return listutils::typeError(err
                + " (error in reading attribute types of first pointcloud)");
        }
        const ListExpr attrName = nl->First(current);
        const ListExpr attrType = nl->Second(current);
        ListExpr pc2AttrType;
        const int pc2AttrIndex = listutils::findAttribute(
                                                pc2Attrs,
                                                nl->SymbolValue(attrName),
                                                pc2AttrType);
        if ((pc2AttrIndex != 0)
                && (nl->IsEqual(attrType,nl->SymbolValue(pc2AttrType)))) {
            const ListExpr attr = nl->TwoElemList(attrName,attrType);
            if (nl->IsEmpty(attrList)) {
                attrList = nl->OneElemList(attr);
                lastAttr = attrList;
                indexListpc1 = nl->OneElemList(nl->IntAtom(pc1AttrIndex));
                lastIndex1 = indexListpc1;
                indexListpc2 = nl->OneElemList(nl->IntAtom(pc2AttrIndex));
                lastIndex2 = indexListpc2;
            } else {
                lastAttr = nl->Append(lastAttr, attr);
                lastIndex1 = nl->Append(lastIndex1, nl->IntAtom(pc1AttrIndex));
                lastIndex2 = nl->Append(lastIndex2, nl->IntAtom(pc2AttrIndex));
            }
        }
        pc1Attrs = nl->Rest(pc1Attrs);
    } // end while

    //Is there any attribute both pc2s have
    if (nl->IsEmpty(attrList)) {
        return nl->ThreeElemList(
                       nl->SymbolAtom(Symbols::APPEND()),
                       appendList,
                       nl->TwoElemList(
                           listutils::basicSymbol<Pointcloud2>(),
                           nl->SymbolAtom(Referencesystem::toString(pc1ref))
                       ));
    }

    const ListExpr tupletypeinfo = nl->TwoElemList(
                                            listutils::basicSymbol<Tuple>(),
                                            attrList);
    if (!listutils::isTupleDescription(tupletypeinfo)) {
        return listutils::typeError(err
                + " (error in creating tupletypeinfo)");
    }

    appendList = nl->OneElemList(nl->IntAtom(nl->ListLength(indexListpc1)));
    ListExpr lastAppend = appendList;
    while (!nl->IsEmpty(indexListpc1)){
        lastAppend = nl->Append(lastAppend, nl->First(indexListpc1));
        indexListpc1 = nl->Rest(indexListpc1);
    }
    while (!nl->IsEmpty(indexListpc2)){
            lastAppend = nl->Append(lastAppend, nl->First(indexListpc2));
            indexListpc2 = nl->Rest(indexListpc2);
    }

    const ListExpr type = nl->TwoElemList(
                            listutils::basicSymbol<Pointcloud2>(),
                            nl->TwoElemList(
                                    nl->SymbolAtom(
                                            Referencesystem::toString(pc1ref)),
                                    tupletypeinfo
                            )
                      );
    const ListExpr result = nl->ThreeElemList(
                                    nl->SymbolAtom(Symbols::APPEND()),
                                    appendList,
                                    type);

    return result;
}


int op_merge::mergeVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {

    Pointcloud2* pc1 = static_cast<Pointcloud2*>(args[0].addr);
    Pointcloud2* pc2 = static_cast<Pointcloud2*>(args[1].addr);
    result = qp->ResultStorage(s);
    Pointcloud2* res = static_cast<Pointcloud2*>(result.addr);

    if (!pc1 || !pc2 || (!pc1->isDefined() && !pc2->isDefined())) {
        res->setDefined(false);
        return 0;
    }

    const int attrCount = static_cast<CcInt*>(args[2].addr)->GetIntval();
    const bool hasTuple = attrCount > 0;
    const ListExpr tupleTypeInfo = hasTuple
                               ? nl->Second(nl->Second(GetTupleResultType(s)))
                               : nl->TheEmptyList();
    std::vector<int> attrIndexPc1(attrCount);
    std::vector<int> attrIndexPc2(attrCount);
    if (hasTuple) {
        attrIndexPc1.clear();
        for (int i = 3; i < attrCount+3; i++){//appended pc1IndexList
            CcInt* x = static_cast<CcInt*>(args[i].addr);
            attrIndexPc1.emplace_back(x->GetIntval());
        }
        attrIndexPc2.clear();
        for (int i = attrCount+3; i < (2*attrCount)+3; i++){//appended pc2Index
            CcInt* x = static_cast<CcInt*>(args[i].addr);
            attrIndexPc2.emplace_back(x->GetIntval());
        }
    }

    // a timer may be used for testing:
    // Timer timer { Timer::UNIT::millis };
    // stringstream st;
    // size_t pointCount1 = pc1->getPointCount();
    // size_t pointCount2 = pc2->getPointCount();
    // st << "merge clouds with " << pointCount1 << " + " << pointCount2
    //         << " = " << (pointCount1 + pointCount2) << " points "
    //         "and " << attrCount << " attributes";
    // timer.startTask(st.str());

    res->startInsert();
    res->merge(pc1, hasTuple, attrIndexPc1, tupleTypeInfo);
    res->merge(pc2, hasTuple, attrIndexPc2, tupleTypeInfo);
    res->finalizeInsert();

    res->setDefined(true);

    // timer.stopTask();
    // cout << timer.getReportForAllTasks() << endl;

    return 0;
}


std::string op_merge::getOperatorSpec(){
    return OperatorSpec(
            " pointcloud2 x pointcloud2 -> pointcloud2",
            " merge( _ , _ ) ",
            " merges two pointcloud2s ",
            " query merge(pc1,pc2) "
    ).getStr();
}

std::shared_ptr<Operator> op_merge::getOperator(){
    return std::make_shared<Operator>("merge",
                                    getOperatorSpec(),
                                    &op_merge::mergeVM,
                                    Operator::SimpleSelect,
                                    &op_merge::mergeTM);
}



