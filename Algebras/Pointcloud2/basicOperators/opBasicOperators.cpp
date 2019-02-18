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



1 Implementation of the Basic Operators

These Operators are:

  * bbox

  * bbox2d

  * maxz

  * minz

  * size

*/
#include "opBasicOperators.h"

#include "QueryProcessor.h"

using namespace pointcloud2;

extern NestedList *nl;
extern QueryProcessor *qp;

/*
1.1 The Pointcloud2 bbox Operator

*/
ListExpr op_bbox::bboxTM(ListExpr args) {

    const std::string err("pointcloud2 expected");

    // although bbox needs to be specified as "pattern op(_,_)"
    // to avoid contradictory definitions, only one parameter is accepted
    // (see detailed comment in Pointcloud2Algebra.spec):
    if (!nl->HasLength(args,1)) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    if (Pointcloud2::TypeCheck(nl->First(args))) {
        return nl->SymbolAtom(Rectangle<DIMENSIONS>::BasicType());
    }

    return listutils::typeError(err);
}

int op_bbox::bboxVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {

    Pointcloud2* pc2 = static_cast<Pointcloud2*>(args[0].addr);
    result = qp->ResultStorage(s);
    Rectangle<DIMENSIONS>* res =
            static_cast<Rectangle<DIMENSIONS>*>(result.addr);
    *res = pc2->getBoundingBox();
    return 0;
}

std::string op_bbox::getOperatorSpec(){
    return OperatorSpec(
        " pointcloud2 -> rectangle<" + std::to_string(DIMENSIONS) + ">",
        " bbox( _ ) ",
        " Returns bounding box of the pointcloud2 ",
        " query bbox(pc2)"
    ).getStr();
}

std::shared_ptr<Operator> op_bbox::getOperator(){
    return std::make_shared<Operator>("bbox",
                                    getOperatorSpec(),
                                    &op_bbox::bboxVM,
                                    Operator::SimpleSelect,
                                    &op_bbox::bboxTM);
}

/*
1.2 The Pointcloud2 bbox2d Operator

*/
ListExpr op_bbox2d::bbox2dTM(ListExpr args) {

    const std::string err("pointcloud2 expected");

    // although bbox2d needs to be specified as "pattern op(_,_)"
    // to avoid contradictory definitions, only one parameter is accepted
    // (see detailed comment in Pointcloud2Algebra.spec):
    if (!nl->HasLength(args,1)) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    if (Pointcloud2::TypeCheck(nl->First(args))) {
        return nl->SymbolAtom(Rect::BasicType());
    }

    return listutils::typeError(err);
}


int op_bbox2d::bbox2dVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {

    Pointcloud2* pc2 = static_cast<Pointcloud2*>(args[0].addr);
    result = qp->ResultStorage(s);
    Rect* res = static_cast<Rect*>(result.addr);
    if (pc2 != nullptr) {
        *res = pc2->getBoundingBox().Project2D(0,1);
    }
    return 0;
}

std::string op_bbox2d::getOperatorSpec(){
    return OperatorSpec(
            " pointcloud2 -> rect",
            " bbox2d( _ ) ",
            " Returns XY-projection of the bounding box of the pointcloud2 ",
            " query bbox2d(pc2)"
    ).getStr();
}

std::shared_ptr<Operator> op_bbox2d::getOperator(){
    return std::make_shared<Operator>("bbox2d",
                                    getOperatorSpec(),
                                    &op_bbox2d::bbox2dVM,
                                    Operator::SimpleSelect,
                                    &op_bbox2d::bbox2dTM);
}

/*
1.3 The Pointcloud2 maxZ Operator

*/
ListExpr op_maxz ::maxzTM(ListExpr args) {

    const std::string err("pointcloud2 expected");

    if (!nl->HasLength(args,1)) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    if (Pointcloud2::TypeCheck(nl->First(args))) {
        return listutils::basicSymbol<CcReal>();
    }

    return listutils::typeError(err);
}


int op_maxz ::maxzVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {

    Pointcloud2* pc2 = static_cast<Pointcloud2*>(args[0].addr);
    result = qp->ResultStorage(s);
    CcReal* res = static_cast<CcReal*>(result.addr);
    if (pc2 == nullptr || !pc2->isDefined()) {
        res->SetDefined(false);
    } else {
        Rect3 bbox = pc2->getBoundingBox();
        if (bbox.IsDefined()) {
            res->Set(true, bbox.MaxD(2));
        } else {
            res->SetDefined(false);
        }
    }
    return 0;
}

std::string op_maxz ::getOperatorSpec(){
    return OperatorSpec(
            " pointcloud2 -> real",
            " maxZ( _ ) ",
            " Returns the maximum height of points ",
            " query maxZ(pc2)"
    ).getStr();
}

std::shared_ptr<Operator> op_maxz ::getOperator(){
    return std::make_shared<Operator>("maxZ",
                                    getOperatorSpec(),
                                    &op_maxz ::maxzVM,
                                    Operator::SimpleSelect,
                                    &op_maxz ::maxzTM);
}

/*
1.4 The Pointcloud2 minZ Operator

*/
ListExpr op_minz::minzTM(ListExpr args) {

    const std::string err("pointcloud2 expected");

    if (!nl->HasLength(args,1)) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    if (Pointcloud2::TypeCheck(nl->First(args))) {
        return listutils::basicSymbol<CcReal>();
    }

    return listutils::typeError(err);
}


int op_minz::minzVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {

    Pointcloud2* pc2 = static_cast<Pointcloud2*>(args[0].addr);
    result = qp->ResultStorage(s);
    CcReal* res = static_cast<CcReal*>(result.addr);
    if (pc2 == nullptr || !pc2->isDefined()) {
        res->SetDefined(false);
    } else {
        Rect3 bbox = pc2->getBoundingBox();
        if (bbox.IsDefined()) {
            res->Set(true, bbox.MinD(2));
        } else {
            res->SetDefined(false);
        }
    }
    return 0;
}

std::string op_minz::getOperatorSpec(){
    return OperatorSpec(
            " pointcloud2 -> real",
            " minZ( _ ) ",
            " Returns the minimal height of points ",
            " query minZ(pc2)"
    ).getStr();
}

std::shared_ptr<Operator> op_minz::getOperator(){
    return std::make_shared<Operator>("minZ",
                                    getOperatorSpec(),
                                    &op_minz::minzVM,
                                    Operator::SimpleSelect,
                                    &op_minz::minzTM);
}

/*
1.5 The Pointcloud2 size Operator

*/
ListExpr op_size::sizeTM(ListExpr args) {

    const std::string err("pointcloud2 expected");

    if (!nl->HasLength(args,1)) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    if (Pointcloud2::TypeCheck(nl->First(args))) {
        return listutils::basicSymbol<CcInt>();
    }

    return listutils::typeError(err);
}


int op_size::sizeVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {

    Pointcloud2* pc2 = static_cast<Pointcloud2*>(args[0].addr);
    result = qp->ResultStorage(s);
    CcInt* res = static_cast<CcInt*>(result.addr);
    if (pc2 == nullptr) {
        res->SetDefined(false);
    } else if (pc2->isDefined()) {
        res->Set(true, pc2->getPointCount());
    } else {
        res->SetDefined(false);
    }
    return 0;
}


std::string op_size::getOperatorSpec(){
    return OperatorSpec(
            " pointcloud2 -> int",
            " size( _ ) ",
            " Returns the number of points ",
            " query size(pc2)"
    ).getStr();
}

std::shared_ptr<Operator> op_size::getOperator(){
    return std::make_shared<Operator>("size",
                                    getOperatorSpec(),
                                    &op_size::sizeVM,
                                    Operator::SimpleSelect,
                                    &op_size::sizeTM);
}


