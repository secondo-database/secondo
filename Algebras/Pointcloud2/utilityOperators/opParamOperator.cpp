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



1 Implementation of the Param Operators

These Operators are:

  * pc2SetParam

  * pc2GetParams

*/
#include "opParamOperator.h"

#include "Algebras/FText/FTextAlgebra.h"


using namespace pointcloud2;

extern NestedList *nl;
extern QueryProcessor *qp;

/*
1.1 The Pointcloud2 pc2SetParam Operator

*/
ListExpr op_setParam::setParamTM(ListExpr args) {
    if (!nl->HasLength(args, 2)) {
        return listutils::typeError("wrong number of arguments");
    }

    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    // has ->SetUsesArgsInTypeMapping() in Algebra, therefore:
    // ( (? ?) (? ?) )
    if(!nl->HasLength(arg1, 2) || !nl->HasLength(arg2, 2))
        return listutils::typeError("internal error");

    if (!CcString::checkType(nl->First(arg1)))
        return listutils::typeError("first argument must be a string");

    ListExpr pm = nl->Second(arg1);
    if(nl->AtomType(pm) != StringType)
        return listutils::typeError("first argument must be constant");

    std::string parameter = nl->StringValue(pm);
    if (!Pointcloud2::params->contains(parameter))
        return listutils::typeError("parameter name unknown");

    const Param& param = Pointcloud2::params->get(parameter);

    switch(param.getType()) {
    case Param::TYPE::BOOL:
        if (!listutils::isSymbol(nl->First(arg2), CcBool::BasicType())) {
            return listutils::typeError("second parameter "
                    "must be a bool value");
        }
        break;
    case Param::TYPE::INT:
        if (!listutils::isSymbol(nl->First(arg2), CcInt::BasicType())) {
            return listutils::typeError("second parameter "
                    "must be an int value");
        }
        break;
    case Param::TYPE::REAL:
        if (!listutils::isSymbol(nl->First(arg2), CcReal::BasicType())) {
            return listutils::typeError("second parameter "
                    "must be a real value");
        }
        break;
    default:
        assert(false); // unexpected case
        break;
    }

    return nl->SymbolAtom(CcBool::BasicType());
}


int op_setParam::setParamVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {

    result = qp->ResultStorage(s);
    CcBool* res = static_cast<CcBool*>(result.addr);

    std::string paramName = (static_cast<CcString*>(args[0].addr))->GetValue();
    
    const Param& param = Pointcloud2::params->get(paramName);
    bool success;
    switch(param.getType()) {
    case Param::TYPE::BOOL: {
        bool value = (static_cast<CcBool*>(args[1].addr))->GetValue();
        param.setValueBool(value);
        success = true;
        break;
    }
    case Param::TYPE::INT: {
        int value = (static_cast<CcInt*>(args[1].addr))->GetValue();
        success = param.trySetValueInt(value);
        break;
    }
    case Param::TYPE::REAL: {
        double value = (static_cast<CcReal*>(args[1].addr))->GetValue();
        success = param.trySetValueReal(value);
        break;
    }
    default:
        assert(false); // unexpected case
        success = false;
        break;
    }
    res->Set(true, success);

    return 0;
}

std::string op_setParam::getOperatorSpec(){
    return OperatorSpec(
        " string x T -> bool",
        " pc2SetParam( _, _ ) ",
        " Set params in pc2 algebra ",
        " query pc2SetParam(\"CELL_SIZE_IN_M\", 20)"
    ).getStr();
}

std::shared_ptr<Operator> op_setParam::getOperator(){
    return std::make_shared<Operator>("pc2SetParam",
            op_setParam::getOperatorSpec(),
            &op_setParam::setParamVM,
            Operator::SimpleSelect,
            &op_setParam::setParamTM);
}

/*
2.1 The Pointcloud2 pc2GetParams Operator

*/
ListExpr op_getParams::getParamsTM(ListExpr args) {
    if (!nl->IsEmpty(args)) {
        return listutils::typeError("no arguments expected");
    }

    //build result
    ListExpr attrList = nl->FiveElemList (
            nl->TwoElemList(
                    nl->SymbolAtom("Name"),
                    listutils::basicSymbol<CcString>()),
            nl->TwoElemList(
                    nl->SymbolAtom("Type"),
                    listutils::basicSymbol<FText>()),
            nl->TwoElemList(
                    nl->SymbolAtom("Value"),
                    listutils::basicSymbol<FText>()),
            nl->TwoElemList(
                    nl->SymbolAtom("Default"),
                    listutils::basicSymbol<FText>()),
            nl->TwoElemList(
                    nl->SymbolAtom("Meaning"),
                    listutils::basicSymbol<FText>()));

    return nl->TwoElemList(
                nl->SymbolAtom(Symbol::STREAM()),
                nl->TwoElemList(
                        nl->SymbolAtom(Tuple::BasicType()),
                        attrList));
}

int op_getParams::getParamsVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {
    ParamsLocalInfo *li = static_cast<ParamsLocalInfo *>(local.addr);
    switch (message) {
    case OPEN: {
        if (li) delete li;
        const ListExpr resultType = GetTupleResultType( s );
        local.addr = new ParamsLocalInfo(nl->Second(resultType));
        return 0;
    }
    case REQUEST:
        result.addr = li ? li->getNext() : nullptr;
        return result.addr ? YIELD : CANCEL;

    case CLOSE:
        if (li) {
            delete li;
            local.addr = nullptr;
        }
        return 0;
    }
    return 0;
}


std::string op_getParams::getOperatorSpec(){
    return OperatorSpec(
        " -> stream(Name,Type,Value,Default,Meaning)",
        " pc2GetParams",
        " Get parameters in pc2 algebra ",
        " query pc2GetParams() consume"
    ).getStr();
}

std::shared_ptr<Operator> op_getParams::getOperator(){
    return std::make_shared<Operator>("pc2GetParams",
            op_getParams::getOperatorSpec(),
            &op_getParams::getParamsVM,
            Operator::SimpleSelect,
            &op_getParams::getParamsTM);
}

/*
1.1 Implementation of FeedLocalInfo Functions

*/
Tuple* ParamsLocalInfo::getNext()
{
    if(_pos >= Pointcloud2::params->size())
        return nullptr;

    const Param& param = Pointcloud2::params->get(_pos);
    Tuple* result = new Tuple(_resultType);
    result->PutAttribute(0, new CcString(param.getName()));
    result->PutAttribute(1, new FText(true, param.getTypeAsString()));
    result->PutAttribute(2, new FText(true, param.getValueAsString()));
    result->PutAttribute(3, new FText(true,
            param.getDefaultValueAsString()));
    result->PutAttribute(4, new FText(true, param.getMeaning()));

    ++_pos;
    return result;
}
