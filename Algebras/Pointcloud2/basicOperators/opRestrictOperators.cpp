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



1 Implementation of the restrict Operators

These Operators are:

  * restrictPc2

  * restrictXY

  * restrictZ

  * restrictAttr

  * restrictRnd

*/
#include "opRestrictOperators.h"

using namespace pointcloud2;

extern NestedList *nl;
extern QueryProcessor *qp;

/*
1.1 The Pointcloud2 restrict Operator

*/
ListExpr op_restrict::restrictTM(ListExpr args) {
    const std::string err("pointcloud2 x rect" + std::to_string(DIMENSIONS)
            + " expected");

    if (!nl->HasLength(args, 2)) {
        return listutils::typeError(err + " (two arguments expected)");
    }

    ListExpr pc2Type = nl->First(args);
    if (!Pointcloud2::TypeCheck(pc2Type)) {
        return listutils::typeError(err + " (error in first argument)");
    }

    if (!PcBox::checkType(nl->Second(args))) {
        return listutils::typeError(err + " (error in second argument)");
    }

    // create the result type (Pointcloud2)
    return pc2Type;
}

int op_restrict::restrictVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {

    Pointcloud2* pc2Source = static_cast<Pointcloud2*>(args[0].addr);
    PcBox* bbox = static_cast<PcBox*>(args[1].addr);

    result = qp->ResultStorage(s);
    Pointcloud2* pc2Dest = static_cast<Pointcloud2*>(result.addr);

    pc2Dest->copySelectionFrom(pc2Source, bbox);

    return 0;
}

std::string op_restrict::getOperatorSpec(){
    return OperatorSpec(
        " pointcloud2(X) x rect<" + std::to_string(DIMENSIONS) +">"
        + " -> pointcloud2(X)",
        " _ restrictPc2[ _ ] ",
        " Creates a new pointcloud2 with the points in the given rect3 ",
        " query pc2 restrictPc2[ [const rect3 value (0 10 0 20 0 50)] ]"
    ).getStr();
}

std::shared_ptr<Operator> op_restrict::getOperator(){
    return std::make_shared<Operator>("restrictPc2",
                                    getOperatorSpec(),
                                    &op_restrict::restrictVM,
                                    Operator::SimpleSelect,
                                    &op_restrict::restrictTM);
}



/*
1.2 The Pointcloud2 restrictXY Operator

*/
ListExpr op_restrictXY::restrictXYTM(ListExpr args) {
    const std::string err("pointcloud2 x rect expected");

    if (!nl->HasLength(args, 2)) {
        return listutils::typeError(err + " (two arguments expected)");
    }

    ListExpr pc2Type = nl->First(args);
    if (!Pointcloud2::TypeCheck(pc2Type)) {
        return listutils::typeError(err + " (error in first argument)");
    }

    if (!Rect::checkType(nl->Second(args))) {
        return listutils::typeError(err + " (error in second argument)");
    }

    // create the result type (Pointcloud2)
    return pc2Type;
}

int op_restrictXY::restrictXYVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {

    Pointcloud2* pc2Source = static_cast<Pointcloud2*>(args[0].addr);
    PcBox sourceBbox = pc2Source->getBoundingBox();

    Rect* rect = static_cast<Rect*>(args[1].addr);

    // to express unlimited Z range, we use the sourceBbox values rather
    // than a minimum/maximum double value (which would cause an overflow in
    // Rectangle<dim>::Area()). An alternative maximum value would be
    // pow(std::numeric_limits<double>::max(), 1.0 / DIMENSIONS) / 2.0 - FACTOR
    // as this avoids an overflow in Area.
    // do not use rect->getMinX() etc. which always returns 0.0!
    double minMax[] = { rect->MinD(0), rect->MaxD(0),
                        rect->MinD(1), rect->MaxD(1),
                        sourceBbox.MinD(2), sourceBbox.MaxD(2) };
    PcBox bbox({rect->IsDefined(), minMax});

    result = qp->ResultStorage(s);
    Pointcloud2* pc2Dest = static_cast<Pointcloud2*>(result.addr);

    pc2Dest->copySelectionFrom(pc2Source, &bbox);

    return 0;
}

std::string op_restrictXY::getOperatorSpec(){
    return OperatorSpec(
        " pointcloud2(X) x rect -> pointcloud2(X)",
        " _ restrictXY( _ ) ",
        " Creates a new pointcloud2 with the points in the given XY-rect ",
        " query pc2 restrictXY[ [const rect value (0 10 0 20)] ]"
    ).getStr();
}

std::shared_ptr<Operator> op_restrictXY::getOperator(){
    return std::make_shared<Operator>("restrictXY",
                                    getOperatorSpec(),
                                    &op_restrictXY::restrictXYVM,
                                    Operator::SimpleSelect,
                                    &op_restrictXY::restrictXYTM);
}


/*
1.3 The Pointcloud2 restrictZ Operator

*/
ListExpr op_restrictZ::restrictZTM(ListExpr args) {
    const std::string err("pointcloud2 x real x real expected");

    if (!nl->HasLength(args, 3)) {
        return listutils::typeError(err + " (three arguments expected)");
    }

    ListExpr pc2Type = nl->First(args);
    if (!Pointcloud2::TypeCheck(pc2Type)) {
        return listutils::typeError(err + " (error in first argument)");
    }
    if (!CcReal::checkType(nl->Second(args))) {
        return listutils::typeError(err + " (error in second argument)");
    }

    if (!CcReal::checkType(nl->Third(args))) {
        return listutils::typeError(err + " (error in third argument)");
    }

    // create the result type (Pointcloud2)
    return pc2Type;
}

int op_restrictZ::restrictZVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {

    Pointcloud2* pc2Source = static_cast<Pointcloud2*>(args[0].addr);
    PcBox sourceBbox = pc2Source->getBoundingBox();

    CcReal* minZ = static_cast<CcReal*>(args[1].addr);
    CcReal* maxZ = static_cast<CcReal*>(args[2].addr);
    const bool isDefined = minZ->IsDefined() && maxZ->IsDefined();

    // to express unlimited XY range, we use the sourceBbox values rather
    // than a minimum/maximum double value (which would cause an overflow in
    // Rectangle<dim>::Area()). See remarks in restrictXYVM
    double minMax[] = { sourceBbox.MinD(0), sourceBbox.MaxD(0),
                        sourceBbox.MinD(1), sourceBbox.MaxD(1),
                        minZ->GetRealval(), maxZ->GetRealval() };
    PcBox bbox({isDefined, minMax});

    result = qp->ResultStorage(s);
    Pointcloud2* pc2Dest = static_cast<Pointcloud2*>(result.addr);

    pc2Dest->copySelectionFrom(pc2Source, &bbox);

    return 0;
}

std::string op_restrictZ::getOperatorSpec(){
    return OperatorSpec(
        " pointcloud2(X) x real x real -> pointcloud2(X)",
        " _ restrictZ( _, _ ) ",
        " Creates a new pointcloud2 with the points in the given Z-interval ",
        " query pc2 restrictZ[ -5.0, 5.0 ]"
    ).getStr();
}

std::shared_ptr<Operator> op_restrictZ::getOperator(){
    return std::make_shared<Operator>("restrictZ",
                                    getOperatorSpec(),
                                    &op_restrictZ::restrictZVM,
                                    Operator::SimpleSelect,
                                    &op_restrictZ::restrictZTM);
}

/*
1.4 restrictAttr Operator

*/
ListExpr op_restrictAttr::restrictAttrTM( ListExpr args ) {
    // work in progress
    // (? ?)
    if( !nl->HasLength(args, 2) ) {
        return listutils::typeError("wrong number of arguments");
    }
    // (pointcloud2<> ?)
    if( !Pointcloud2::TypeCheck(nl->First(args)) ) {
        return listutils::typeError(R"(first argument "
                            "must be a pointcloud2)");
    }

    if( !Pointcloud2::isTupleCloud( nl->First(args)) ) {
        return listutils::typeError(R"(pointcloud must have attributes "
                                    "for this operator to have a point)");
    }
    // (pointcloud2<> (map ? ?))
    if( !listutils::isMap<1>( nl->Second(args)) ) {
        return listutils::typeError(R"(second argument "
                                    "has to be a map with one argument)");
    }
    // (pointcloud2<> (map tuple ?))
    if( !Tuple::checkType( nl->Second(nl->Second(args))) ) {
        return listutils::typeError(R"(argument to the map "
                                    "in restrictAttr must be a Tuple)");
    }
    // (pointcloud2<> (map tuple bool))
    if( !CcBool::checkType( nl->Third(nl->Second(args))) ) {
        return listutils::typeError(R"(result of the map "
                                    "in restrictAttr must be Boolean)");
    }

    return nl->First(args);
}

int op_restrictAttr::restrictAttrVM(Word* args, Word& result, int message,
                Word& local, Supplier s) {
    // work in progress
    result = qp->ResultStorage(s);
    Pointcloud2* pc2source = static_cast<Pointcloud2*>(args[0].addr);
    Pointcloud2* pc2dest = static_cast<Pointcloud2*>(result.addr);

    // case undefined
    pc2dest->setDefined(pc2source->isDefined());
    if( !pc2dest->isDefined() ) {
            return 0;
    }

    Word f = args[1];
    ArgVectorPointer funargs = qp->Argument(f.addr);
    Word funres;

    PcPoint pcPoint;
    SmiRecordFileIterator *it = pc2source->getFileIterator();
    SmiRecord record;
    pc2dest->startInsert();
    while( it->Next(record) ) {
        pc2source->getPoint(record.GetId(), &pcPoint);
        Tuple *t = pc2source->getTuple(pcPoint._tupleId);

        // function application
        *funargs[0] = t;

        qp->Request(f.addr, funres);

        CcBool *res = static_cast<CcBool*>(funres.addr);
        if (res->GetBoolval()) {
                pc2dest->insert(pcPoint, t);
        } else {
                t->DeleteIfAllowed();
        }
    }
    delete it;
    pc2dest->finalizeInsert();
    return 0;
}

std::string op_restrictAttr::getOperatorSpec() {
    return OperatorSpec(
        "pointcloud2(X) x (fun: T -> bool) -> pointcloud2(X)",
        "_ restrictAttr[_] ",
        "restrict to points fulfilling the given predicate",
        "query pc2 restrictAttr[]"
    ).getStr();
}

std::shared_ptr<Operator> op_restrictAttr::getOperator() {
    return std::make_shared<Operator>("restrictAttr",
                                    getOperatorSpec(),
                                    &op_restrictAttr::restrictAttrVM,
                                    Operator::SimpleSelect,
                                    &op_restrictAttr::restrictAttrTM);
}

/*
1.5 The Pointcloud2 restrictRnd Operator

*/
ListExpr op_restrictRnd::restrictRndTM(ListExpr args) {
    const std::string err("pointcloud2 x int expected");

    if (!nl->HasLength(args, 2)) {
        return listutils::typeError(err + " (two arguments expected)");
    }

    ListExpr pc2Type = nl->First(args);
    if (!Pointcloud2::TypeCheck(pc2Type)) {
        return listutils::typeError(err + " (error in first argument)");
    }
    if (!CcInt::checkType(nl->Second(args))) {
        return listutils::typeError(err + " (error in second argument)");
    }

    // create the result type (Pointcloud2)
    return pc2Type;
}

int op_restrictRnd::restrictRndVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {

    Pointcloud2* pc2Source = static_cast<Pointcloud2*>(args[0].addr);
    PcBox sourceBbox = pc2Source->getBoundingBox();

    CcInt* count = static_cast<CcInt*>(args[1].addr);
    const bool isDefined = count->IsDefined();
    size_t countValue = count->GetValue() < 0 ? 0 : size_t(count->GetValue());

    result = qp->ResultStorage(s);
    Pointcloud2* pc2Dest = static_cast<Pointcloud2*>(result.addr);

    if (!isDefined || countValue <= 0) {
        // do nothing
    } else if (countValue >= pc2Source->getPointCount()) {
        pc2Dest->copyAllFrom(pc2Source);
    } else {
        BitArray bitMap { pc2Source->getPointCount(), true };
        bitMap.initializeRandom(countValue);
        pc2Dest->copySelectionFrom(pc2Source, bitMap);
    }
    return 0;
}

std::string op_restrictRnd::getOperatorSpec(){
    return OperatorSpec(
        " pointcloud2(X) x int -> pointcloud2(X)",
        " _ restrictRnd[_] ",
        " Creates a new pointcloud2 with a random selection of n points ",
        " query pc2 restrictRnd[ 1000 ]"
    ).getStr();
}

std::shared_ptr<Operator> op_restrictRnd::getOperator(){
    return std::make_shared<Operator>("restrictRnd",
                                    getOperatorSpec(),
                                    &op_restrictRnd::restrictRndVM,
                                    Operator::SimpleSelect,
                                    &op_restrictRnd::restrictRndTM);
}

