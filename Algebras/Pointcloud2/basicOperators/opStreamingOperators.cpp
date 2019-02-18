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



0 Pointcloud2 Streaming Operators

These operators are "feed" and "collect2pc".
They allow to feed a pc2-object into a tuple-stream
and to collect a pc2-object from a tuple-stream.
In this way one can leverage all the existing operators working on tuple streams.

*/
#include "opStreamingOperators.h"

#include <set>
#include "Algebras/Stream/Stream.h"

using namespace pointcloud2;

extern NestedList *nl;

/*
1 Implementation feed Operator

*/
ListExpr op_feed::feedTM(ListExpr args) {
    const std::string err("pointcloud2 expected");
    // input like: ((pointcloud2 (EUCLID (tuple ((Name string)(Val real)) ))))
    // checking it:
    // (?)
    if (!nl->HasLength(args,1)) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    // ( (? ?) )
    ListExpr typeInfo = nl->First(args);
    if (!nl->HasLength(typeInfo,2)) {
        return listutils::typeError(err + " (wrong format - 1)");
    }
    // ( (pointcloud2 ?) )
    if (!listutils::isSymbol(nl->First(typeInfo),Pointcloud2::BasicType())) {
        return listutils::typeError(err);
    }

    bool hasTupleInformation = false;
    ListExpr typeInfo2 = nl->Second(typeInfo);
    // ( (pointcloud (RS)) ) || ( (pointcloud2 (RS TT)) )
    if (nl->HasLength(typeInfo2,2)) {
        hasTupleInformation = true;
    } else if (nl->ListLength(typeInfo2) != -1) {
        return listutils::typeError(err + " (wrong format - 2)");
    }

    // construction of return value:
    // point type for Tuple - (P point)
    ListExpr pointType = nl->TwoElemList(
        nl->SymbolAtom("P"),
        listutils::basicSymbol<Point>()
    );

    // Z coordinate - (Alt real)
    ListExpr pointHeight = nl->TwoElemList(
        nl->SymbolAtom("Alt"),
        listutils::basicSymbol<CcReal>()
    );

    // streamTuple: ( (P point) (Alt real) )
    ListExpr streamTuple = nl->OneElemList(pointType);
    ListExpr last = nl->Append(streamTuple, pointHeight);

    if(hasTupleInformation) {
        ListExpr attributes = nl->Second(nl->Second(typeInfo2));

        std::set<std::string> attributeNames = {"P", "Alt"};
        while(!nl->IsEmpty(attributes))
        {
            ListExpr first = nl->First(attributes);
            attributes = nl->Rest(attributes);

            //TODO: Fix variable names....
            ListExpr attributeName = nl->First(first);
            std::string arttibuteNameAsString = nl->ToString(attributeName);
            int counter = 1;
            while(attributeNames.find(arttibuteNameAsString)
                    != attributeNames.end()){
                arttibuteNameAsString = arttibuteNameAsString +
                                        "_" + std::to_string(counter++);
            }
            attributeNames.insert(arttibuteNameAsString);
            // streamTuple: ((P point)(Alt real)(attrNameStr attrType){n})
            last = nl->Append(last,
                                    nl->TwoElemList(
                                        nl->SymbolAtom(
                                            arttibuteNameAsString),
                                        nl->Second(first)));
        }
    }
    // ( tuple
    //        ((P point)(Alt real)(attrNameStr attrType){n}) )
    streamTuple = nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()), streamTuple);

    //    ( stream
    //        ( tuple
    //            ((P point)(Alt real)(attrNameStr attrType){n}) ))
    const ListExpr streamExpr = nl->TwoElemList(
                nl->SymbolAtom(Symbol::STREAM()), streamTuple);

    return streamExpr;
}


int op_feed::feedVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {
    
    FeedLocalInfo *li = static_cast<FeedLocalInfo *>(local.addr);
    switch (message)
    {
    case OPEN:
    {
        if (li) delete li;
        ListExpr resultType = GetTupleResultType( s );
        local.addr=new FeedLocalInfo(static_cast<Pointcloud2*>(args[0].addr), 
                                    nl->Second(resultType));
        return 0;
    }
    case REQUEST:
        result.addr = li ? li->getNext() : nullptr;
        return result.addr ? YIELD : CANCEL;

    case CLOSE:
        if (li)
        {
            delete li;
            local.addr = nullptr;
        }
        return 0;
    }
    return 0;
}


std::string op_feed::getOperatorSpec(){
    return OperatorSpec(
            " pointcloud2 -> stream(...)",
            " _ feed ",
            " Returns the points as a tuple stream ",
            " query pc2 feed"
    ).getStr();
}

std::shared_ptr<Operator> op_feed::getOperator(){
    return std::make_shared<Operator>("feed",
                                    getOperatorSpec(),
                                    &op_feed::feedVM,
                                    Operator::SimpleSelect,
                                    &op_feed::feedTM);
}

/*
1.1 Implementation of FeedLocalInfo Functions

*/
Tuple* FeedLocalInfo::getNext()
{
    ++_pos;
    if(_pos <= _countOfPoints)
    {
        Tuple* result = new Tuple(_resultType);
        
        _pc2->getPoint(_pos, _currentPoint.get());

        result->PutAttribute(0, new Point(true, _currentPoint->_x, 
                                        _currentPoint->_y));
        result->PutAttribute(1, new CcReal(_currentPoint->_z));

        if(_currentPoint->_tupleId){
            Tuple* tuple = _pc2->getTuple(_currentPoint->_tupleId);
            for(int i = 2, j = 0; j < tuple->GetNoAttributes(); ++i, ++j) {
                // we must clone the attribute from the source tuple
                // in order to be able to delete the source tuple
                result->PutAttribute(i, tuple->GetAttribute(j)->Clone());
            }
            tuple->DeleteIfAllowed();
        }

        return result;
    }
    return nullptr;
}

/*
2 The Pointcloud2 CollectPc2 Operator
collectPc2: stream(tuple(...) xP xA xREF xZ1...Zn
-> pointcloud2(REF, (tuple(|Z1:t1,...,Zn:tn|)))

2.1 Type Mapping
Arg: Stream(tuple), AttrName point, alt(int,real), REF, Z1..Z2

*/

ListExpr OPCollectPc2::collectPc2TM(ListExpr args){
    std::string err = "Stream(tuple), AttrName point, alt, REF, Z1..Z2";
    if(!(nl->HasLength(args,5))){
        //5th argument is list that could be empty
        return listutils::typeError("expected 4 or more arguments");
    }
    const ListExpr arg1 = nl->First(args); //tuple-stream
    const ListExpr arg2 = nl->Second(args); //point
    const ListExpr arg3 = nl->Third(args); //alt
    const ListExpr arg4 = nl->Fourth(args); //REF
    ListExpr tupleAttr = nl->Fifth(args); //Attr-List

    if(!Stream<Tuple>::checkType(arg1)){
      return listutils::typeError("first arg is not a tuple stream");
    }
    if(nl->AtomType(arg2) != SymbolType){
       return listutils::typeError("second arg is not a valid attribute name");
    }
    if(nl->AtomType(arg3) != SymbolType){
       return listutils::typeError("third arg is not a valid attribute name");
    }

    // check for REF
    if(!listutils::isSymbol(arg4)){
        return listutils::typeError(err + " (REF)");
    } else {
        try {
            Referencesystem::toEnum(nl->SymbolValue(arg4));
        } catch(std::invalid_argument&) {
            return listutils::typeError(err + " (REF)");
        }
    }

    // extract the attribute list
    ListExpr attrList = nl->Second(nl->Second(arg1));
    ListExpr type = nl->TheEmptyList();

    ListExpr result = nl->OneElemList(nl->TheEmptyList());
    ListExpr lastRes = result;

    //Point
    const std::string p = nl->SymbolValue(arg2);

    //Attr.position
    int j = listutils::findAttribute(attrList, p, type);
    if (!Point::checkType(type)){
        return listutils::typeError(err + " ( Attr. no Point)");
    }

    //Pos 1 is point, pos 2 alt and the rest tuple-attr
    ListExpr indexes = nl->OneElemList(nl->IntAtom(j));
    ListExpr lastIndex = indexes;

    //Alt
    const std::string alt = nl->SymbolValue(arg3);
    j = listutils::findAttribute(attrList, alt, type);
    if (!(CcInt::checkType(type) || CcReal::checkType(type))){
        return listutils::typeError(err + " ( Alt no Int/Real)");
    }
    lastIndex = nl->Append(lastIndex, nl->IntAtom(j));
    lastIndex = nl->Append(lastIndex, nl->IntAtom(CcInt::checkType(type)));
    //attr. for tuple of Pc2
    std::string attrName;

    while (!nl->IsEmpty(tupleAttr)){
        attrName = nl->SymbolValue(nl->First(tupleAttr));
        j = listutils::findAttribute(attrList, attrName, type);
        if (j > 0){
            lastIndex = nl->Append(lastIndex, nl->IntAtom(j));
            lastRes   = nl->Append(lastRes,
                    nl->TwoElemList(nl->First(tupleAttr),type));
        }
        tupleAttr = nl->Rest(tupleAttr);
    }

    result = nl->Rest(result);

    //DEBUG
    std::cout << nl->ToString(result) << endl;

    if (nl->IsEmpty(result)){
        result = nl->TwoElemList(listutils::basicSymbol<Pointcloud2>(),
                                arg4);
    } else {
        result = nl->TwoElemList(listutils::basicSymbol<Tuple>(),result);
        result = nl->TwoElemList(arg4, result);
        result = nl->TwoElemList(listutils::basicSymbol<Pointcloud2>(),result);
    }

    ListExpr append = nl->OneElemList(nl->IntAtom(nl->ListLength(indexes)));
    ListExpr lastAppend = append;
    while (!nl->IsEmpty(indexes)){
        lastAppend = nl->Append(lastAppend, nl->First(indexes));
        indexes = nl->Rest(indexes);
    }

    //DEBUG
    std::cout<<"return: " << nl->ToString(result) << endl;
    std::cout<<"return: " << nl->ToString(append) << endl;

    return nl->ThreeElemList(
             nl->SymbolAtom(Symbols::APPEND()),
             append,
             result);
}

int OPCollectPc2::collectPc2VMT( Word* args, Word& result, int message,
        Word& local, Supplier s ){

    result = qp->ResultStorage(s);
    Pointcloud2* pc2 = static_cast<pointcloud2::Pointcloud2*>(result.addr);

    //read append structure
    //(index Point, index Alt, type Alt (0/1 real/int), index Attr)
    const int arg5 = static_cast<CcInt*>(args[5].addr)->GetIntval();
    const int countIndex = 6 + arg5;

    //only point or attributes?
    bool tupleExist = false;
    if (countIndex > 9){
        tupleExist = true;
    }

    std::vector<int> index(arg5);
    index.clear(); //TODO: Warum ist das notwendig??
    for (int i = 6; i < countIndex; i++){//all appended index
        CcInt* x = static_cast<CcInt*>(args[i].addr);
        index.emplace_back(x->GetIntval());
    }

    std::unique_ptr<TupleType> tt;
    if (tupleExist){
        const ListExpr resultType = GetTupleResultType(s);
        tt.reset(new TupleType(nl->Second(nl->Second(resultType))));
    }

    Stream<Tuple> stream(args[0]);
    stream.open();
    Tuple* elem;
    pc2->startInsert();
    while( (elem = stream.request()) != 0 ){
        //x, y, z;
        Point* p = static_cast<Point*>(elem->GetAttribute(index[0]-1));
        if (!p->IsDefined()){
            elem->DeleteIfAllowed();
            continue;
        }
        const double x = p->GetX();
        const double y = p->GetY();
        double z = 0.0;

        constexpr int altInt = 1;
        if (index[2] == altInt){ //Int/Real
            CcInt* zTup = static_cast<CcInt*>(elem->GetAttribute(index[1]-1));
            if (!zTup->IsDefined()){
                elem->DeleteIfAllowed();
                continue;
            }
            z = zTup->GetIntval();
        }else{
            CcReal* zTup = static_cast<CcReal*>(elem->GetAttribute(index[1]-1));
            if (!zTup->IsDefined()){
                elem->DeleteIfAllowed();
                continue;
            }
            z = zTup->GetRealval();
        }
        // build tuple for pointcloud2
        if (tupleExist){
            Tuple* attrTuple = new Tuple(tt.get());

            // number of attributes in result tuple
            for (int i = 3; i < arg5; i++){ //all attributes
                attrTuple->CopyAttribute(index[i]-1, elem, i-3);
            }

            PcPoint pcPoint {x, y, z};
            pc2->insert(pcPoint, attrTuple);
        } else{
            pc2->insert({x,y,z});
        }
        elem->DeleteIfAllowed();
    }
    stream.close();
    pc2->finalizeInsert();

    if (local.addr) {
        local.addr = nullptr;
    }

    return 0;
}



std::string OPCollectPc2::getOperatorSpec(){
    return OperatorSpec(
            "stream(...) -> pointcloud2(REF, tuple)",
            "_ collectPc2[_,_,_]",
            "collect stream P,A,REF,Attr in PC2",
            "query xx collectPc2 [P,Alt,ECULID]"
    ).getStr();
}


std::shared_ptr<Operator> OPCollectPc2::getOperator(){
    return std::make_shared<Operator>("collectPc2",
                                       getOperatorSpec(),
                                       OPCollectPc2::collectPc2VMT,
                                       Operator::SimpleSelect,
                                       &OPCollectPc2::collectPc2TM);
}

