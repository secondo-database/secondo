/*
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

*/


#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"

#include "Row/SpatialJoinTouchRow.h"
#include "Memory/SpatialJoinTouchM.h"
#include "Column/SpatialJoinTouchColumn.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "Symbols.h"
#include "ListUtils.h"
#include <iostream>
#include <math.h>
#include "Algebras/CRel/TypeConstructors/CRelTC.h"
#include "Algebras/CRel/Operators/OperatorUtils.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Stream/Stream.h"
#include "Algebras/CRel/TBlock.h"
#include "Algebras/CRel/TypeConstructors/TBlockTC.h"

#include "Memory/SpatialJoinMLocalInfo.h"
#include "Row/SpatialJoinRowLocalInfo.h"
#include "Column/SpatialJoinColumnLocalInfo.h"

#include "../../include/MPointer.h"
#include "Algebras/MainMemory2/MainMemoryExt.h"


#include <time.h>


extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;
using namespace mm2algebra;

namespace sjt {

    class spatialJoinTouchRow::spatialJoinRowInfo: public OperatorInfo {
    public:
        spatialJoinRowInfo()
        {
            name        = "spatialJoinTouch";
            signature   = "stream(tuple(A)) x stream(tuple(B)) x a_i x b_j "
                          "           -> stream(tuple(A o B)) ";
            syntax      = "_ _ spatialJoinTouch [_,_]";
            meaning     = "Executes the Touch algorithm, which is a spatial "
                          "join, between two streams of datasets. "
                          "Further a_i and b_j must be of type rect or rect3 "
                          "in A and B. The current query is performed on "
                          "tuple streams.";
            example = "query Roads feed {a} Buildings feed {b} "
                      "spatialJoinTouch[GeoData_a, GeoData_b] count";
        }
    };

    class spatialJoinTouchM::spatialJoinMInfo: public OperatorInfo {
    public:
        spatialJoinMInfo()
        {
            name        = "spatialJoinTouch";
            signature   = "mpointer(mem(rel(tuple(A)))) x "
                          "mpointer(mem(rel(tuple(B)))) x a_i x b_j"
                          " -> stream(tuple(A o B)) ";
            syntax      = "_ _ spatialJoinTouch [_,_]";
            meaning     =  "Executes the Touch algorithm, which is a spatial "
                           "join, between two memory relations. "
                           "Further a_i and b_j must be of type rect or rect3 "
                           "in A and B. The current query is performed on "
                           "memory relations.";
            example = "query Roads feed {a} mconsume Buildings feed {b} "
                      "mconsume  spatialJoinTouch [ GeoData_a, GeoData_b] "
                      "count";
        }
    };

    class spatialJoinTouchColumn::spatialJoinColumnInfo: public OperatorInfo {
    public:
        spatialJoinColumnInfo()
        {
            name        = "spatialJoinTouch";
            signature   = "stream(tblock(ma, ((na0, ca0) ... (nai, cai)))) x "
                          "stream(tblock(ma, ((nb0, cb0) ... (nbi, cbi)))) x "
                          "a_i x b_j "
                          "-> stream(tblock(ma, ((nd0, cd0) ... (ndi, cdi))))";
            syntax      = "_ _ spatialJoinTouch [_,_]";
            meaning     = "Executes the Touch algorithm, which is a spatial "
                          "join, between two streams of datasets. "
                          "Further a_i and b_j must be of type rect or rect3 "
                          "in A and B. The current query is performed on "
                          " tuple-block streams.";
            example = "query Roads feed toblocks[1] {a} Buildings feed "
                      "toblocks[1] {b} spatialJoinTouch[GeoData_a, GeoData_b] "
                      "count";
        }
    };

    spatialJoinTouchRow::spatialJoinTouchRow() : Operator(
            spatialJoinRowInfo(),
            valueMappings,
            SelectValueMapping,
            spatialJoinRowType
            )
    {
        SetUsesArgsInTypeMapping();
        SetUsesMemory();
    }

    spatialJoinTouchRow::~spatialJoinTouchRow() {
    }

    spatialJoinTouchM::spatialJoinTouchM() : Operator(
            spatialJoinMInfo(),
            valueMappings,
            SelectValueMapping,
            spatialJoinMType
            )
    {
        SetUsesArgsInTypeMapping();
        SetUsesMemory();
    }

    spatialJoinTouchM::~spatialJoinTouchM() {
    }

    spatialJoinTouchColumn::spatialJoinTouchColumn() : Operator(
            spatialJoinColumnInfo(),
            valueMappings,
            SelectValueMapping,
            spatialJoinColumnType
            )
    {
        SetUsesArgsInTypeMapping();
        SetUsesMemory();
    }

    spatialJoinTouchColumn::~spatialJoinTouchColumn() {
    }

    int spatialJoinRowFun (Word* args, Word& result,
                    int message, Word& local, Supplier s)
    {
        SpatialJoinRowLocalInfo* localInfo
                        = static_cast<SpatialJoinRowLocalInfo*>(local.addr);

        switch( message )
        {
            case OPEN: {

                if (localInfo) {
                    delete localInfo;
                }

                if(local.addr) {
                    local.setAddr(0);
                }

                ListExpr ttl = nl->Second(GetTupleResultType(s));

                Word leftStreamWord;
                Word rightStreamWord;
                Word leftStreamWordIndexW;
                Word rightStreamWordIndexW;

                leftStreamWord = args[0];
                rightStreamWord = args[1];
                leftStreamWordIndexW = args[4];
                rightStreamWordIndexW = args[5];

                int fanout = 32;
                int numOfItemsInBucket = 12800;
                int cellFactor = 2;

                int leftStreamWordIndex =
                        ((CcInt*)leftStreamWordIndexW.addr)->GetIntval()-1;
                int rightStreamWordIndex =
                        ((CcInt*)rightStreamWordIndexW.addr)->GetIntval()-1;

                localInfo = new SpatialJoinRowLocalInfo(
                        leftStreamWord,
                        rightStreamWord,
                        leftStreamWordIndex,
                        rightStreamWordIndex,
                        ttl,
                        fanout,
                        numOfItemsInBucket,
                        cellFactor
                        );

                local.setAddr(localInfo);

                qp->Open (rightStreamWord.addr);

                return 0;
            }
            case REQUEST: {

                result.setAddr(localInfo->NextResultTuple());

                return result.addr != 0 ? YIELD : CANCEL;
            }
            case CLOSE: {

                qp->Close (args[1].addr);

                if(localInfo)
                {
                    delete localInfo;
                    local.setAddr(0);
                }

                return 0;
            }
            default: {
                /* should not happen */
                return -1;
            }
        }
    }

    int spatialJoinMFun (Word* args, Word& result,
                                       int message, Word& local, Supplier s)
    {
        int fanout = 32;
        int numOfItemsInBucket = 12800;
        int cellFactor = 2;

        MPointer* mPointerA = (MPointer*) args[0].addr;
        MPointer* mPointerB = (MPointer*) args[1].addr;
        MemoryRelObject* mrelA = (MemoryRelObject*) mPointerA->GetValue();
        MemoryRelObject* mrelB = (MemoryRelObject*) mPointerB->GetValue();

        Word vAIndexWord = args[4];
        Word vBIndexWord = args[5];


        int vAIndex = ((CcInt*)vAIndexWord.addr)->GetIntval()-1;
        int vBIndex = ((CcInt*)vBIndexWord.addr)->GetIntval()-1;

        vector<Tuple*>* vA = mrelA->getmmrel();
        vector<Tuple*>* vB = mrelB->getmmrel();

        result = qp->ResultStorage(s);
        MPointer* res = (MPointer*) result.addr;

        ListExpr ttl =
                nl->Second(nl->Second(nl->Second(GetTupleResultType(s))));

        TupleType* tt = new TupleType(ttl);

        SpatialJoinMLocalInfo * localInfo = new SpatialJoinMLocalInfo(
                *vA,
                *vB,
                vAIndex,
                vBIndex,
                tt,
                fanout,
                numOfItemsInBucket,
                cellFactor
        );

        MemoryRelObject* resRel = localInfo->getMatchings(*vB);

        delete localInfo;
        res->setPointer(resRel);
        resRel->deleteIfAllowed();

        tt->DeleteIfAllowed();

        return 0;
    }


    int spatialJoinColumnFun(Word* args, Word& result, int message,
                       Word& local, Supplier s) {

        SpatialJoinColumnLocalInfo* localInfo =
                    static_cast<SpatialJoinColumnLocalInfo*>(local.addr);

        //uint64_t memLimit = qp->GetMemorySize(s)*1024*1024;

        switch (message) {

            case OPEN: {
                if (localInfo) {
                    delete localInfo;
                }

                Word leftStreamWord = args[0];
                Word rightStreamWord = args[1];
                Word vAIndexWord = args[4];
                Word vBIndexWord = args[5];

                int firstStreamIndex = ((CcInt*)vAIndexWord.addr)->GetIntval();
                int secondStreamIndex = ((CcInt*)vBIndexWord.addr)->GetIntval();

                int fanout = 32;
                int numOfItemsInBucket = 12800;
                int cellFactor = 2;

                ListExpr ttl = nl->Second(nl->Second(GetTupleResultType(s)));

                local.addr = new SpatialJoinColumnLocalInfo(
                        leftStreamWord,
                        rightStreamWord,
                        fanout,
                        numOfItemsInBucket,
                        cellFactor,
                        firstStreamIndex,
                        secondStreamIndex,
                        ttl,
                        s
                        );

                return 0;
            }

            case REQUEST: {

                result.setAddr(localInfo->NextResultTBlock());

                return result.addr ? YIELD : CANCEL;
            }

            case CLOSE: {

                if (localInfo) {
                    delete localInfo;
                    local.addr = 0;
                }

                return 0;
            }
        } // End switch

        return 0;
    }

    ListExpr
    spatialJoinTouchRow::spatialJoinRowType( ListExpr args )
    {
               
        const int argNum = nl->ListLength(args);
        if (argNum != 4) {
            return listutils::typeError("Expected four arguments.");
        }
        if(!listutils:: checkUsesArgsInTypeMapping(args)){
           return listutils::typeError("internal error");
        }

        ListExpr fStream      = nl->First(nl->First(args));
        ListExpr sStream      = nl->First(nl->Second(args));
        ListExpr attrName1    = nl->First(nl->Third(args));
        ListExpr attrName2    = nl->First(nl->Fourth(args));


        string err = "relation x relation x attribute name x attribute name ";



        // first argument must be a stream of tuple-blocks
        if(!listutils::isTupleStream(fStream)) {
            return listutils::typeError("Error in  first argument: "
                                        "Tuple Stream expected.");
        }

        // second argument must be a stream of tuple-blocks
        if(!listutils::isTupleStream(sStream)) {
            return listutils::typeError("Error in  second argument.: "
                                        "Tuple Stream expected.");
        }

        ListExpr al1 = nl->Second(nl->Second(fStream));
        ListExpr al2 = nl->Second(nl->Second(sStream));

        // third argument must be an attribute name
        if(!listutils::isSymbol(attrName1)) {
            return listutils::typeError("Error in third argument: "
                                        "Attribute name expected.");
        }

        // fourth argument must be an attribute name
        if(!listutils::isSymbol(attrName2)) {
            return listutils::typeError("Error in fourth argument: "
                                        "Attribute name expected.");
        }

        if(!listutils::disjointAttrNames(al1, al2)){
            return listutils::typeError("conflicting type names");
        }

        ListExpr type1;
        string name1 = nl->SymbolValue(attrName1);
        int index1 = listutils::findAttribute(al1,name1,type1);
        if(index1==0){
            return listutils::typeError("attribute " + name1 + "not found");
        }

        ListExpr type2;
        string name2 = nl->SymbolValue(attrName2);
        int index2 = listutils::findAttribute(al2,name2,type2);
        if(index2==0){
            return listutils::typeError("attribute " + name2 + "not found");
        }

        if(!listutils::isSymbol(type1) ||
           !listutils::isSymbol(type2)){
            return listutils::typeError("composite types not supported");
        }

        ListExpr attrlist = listutils::concat(al1, al2);

        return nl->ThreeElemList(
                nl->SymbolAtom(Symbol::APPEND()),
                nl->TwoElemList(
                        nl->IntAtom(index1),
                        nl->IntAtom(index2)),
                nl->TwoElemList(
                        nl->SymbolAtom(Symbol::STREAM()),
                        nl->TwoElemList(
                                nl->SymbolAtom(Tuple::BasicType()),
                                attrlist)));
    }

    ListExpr
    spatialJoinTouchM::spatialJoinMType( ListExpr args )
    {
        string err = "relation x relation x attribute name x attribute name ";

        if(nl->ListLength(args)!=4){
            return listutils::typeError(err);
        }
        if(!listutils::checkUsesArgsInTypeMapping(args)){
            return listutils::typeError("internal error");
        }


        ListExpr a1;
        ListExpr a2;
        ListExpr mpointer1    = nl->First(nl->First(args));
        ListExpr mpointer2    = nl->First(nl->Second(args));
        ListExpr nameL1       = nl->First(nl->Third(args));
        ListExpr nameL2       = nl->First(nl->Fourth(args));



        if (!MPointer::checkType(mpointer1)){
            return listutils::typeError("a mpointer to a relation or "
                                        "a stream of tuples is expected");
        }

        a1 = nl->Second(mpointer1);

        if (!MPointer::checkType(mpointer2)){
            return listutils::typeError("a mpointer to a relation or "
                                        "a stream of tuples is expected");
        }

        a2 = nl->Second(mpointer2);

        if (!Mem::checkType(a1)){
            return listutils::typeError("first argument "
                                        "is not a memory object");
        }

        if (!Mem::checkType(a2)){
            return listutils::typeError("second argument "
                                        "is not a memory object");
        }

        ListExpr relation1 = nl->Second(a1);
        ListExpr relation2 = nl->Second(a2);

        if (!Relation::checkType(relation1)){
            return listutils::typeError("(first arg is not a memory relation)");
        }

        if (!Relation::checkType(relation2)){
            return listutils::typeError("(second arg is not"
                                        " a memory relation)");
        }

        ListExpr tuples1 = nl->Second(relation1);
        ListExpr tuples2 = nl->Second(relation2);

        ListExpr t1attr = nl->Second(tuples1);
        ListExpr t2attr = nl->Second(tuples2);

        ListExpr attrlist = listutils::concat(t1attr, t2attr);

        ListExpr type1;
        string name1 = nl->SymbolValue(nameL1);
        int index1 = listutils::findAttribute(t1attr,name1,type1);
        if(index1==0){
            return listutils::typeError("attribute " + name1 + "not found");
        }

        ListExpr type2;
        string name2 = nl->SymbolValue(nameL2);
        int index2 = listutils::findAttribute(t2attr,name2,type2);
        if(index2==0){
            return listutils::typeError("attribute " + name2 + "not found");
        }

        ListExpr relType = nl->TwoElemList(
                nl->SymbolAtom(Relation::BasicType()),
                nl->TwoElemList(
                        nl->SymbolAtom(Tuple::BasicType()),
                        attrlist
                ));

        return nl->ThreeElemList(
                nl->SymbolAtom(Symbol::APPEND()),
                nl->TwoElemList(
                        nl->IntAtom(index1),
                        nl->IntAtom(index2)
                ),
                MPointer::wrapType(Mem::wrapType(relType)));

    }

    ListExpr spatialJoinTouchColumn::spatialJoinColumnType(ListExpr args) {

        const int argNum = nl->ListLength(args);

        string err = "relation x relation x attribute name x attribute name ";

        if (argNum != 4)
        {
            return listutils::typeError("Expected four arguments.");
        }
        if(!listutils::checkUsesArgsInTypeMapping(args)){
          return listutils::typeError("internal error");
        }

        ListExpr fStream      = nl->First(nl->First(args));
        ListExpr sStream      = nl->First(nl->Second(args));
        ListExpr attrName1    = nl->First(nl->Third(args));
        ListExpr attrName2    = nl->First(nl->Fourth(args));



        // first argument must be a stream of tuple-blocks
        if(!listutils::isStream(fStream)) {
            return listutils::typeError("Error in  first argument: "
                                        "Stream expected.");
        }

        if(!CRelAlgebra::TBlockTI::Check(nl->Second(fStream))) {
            return listutils::typeError("Error in  first argument: "
                                        "Stream of tuple-blocks expected.");
        }

        // second argument must be a stream of tuple-blocks
        if(!listutils::isStream(sStream)) {
            return listutils::typeError("Error in  second argument.: "
                                        "Stream expected.");
        }

        if(!CRelAlgebra::TBlockTI::Check(nl->Second(sStream))) {
            return listutils::typeError("Error in  second argument: "
                                        "Stream of tuple-blocks expected.");
        }

        // third argument must be an attribute name
        if(!listutils::isSymbol(attrName1)) {
            return listutils::typeError("Error in third argument: "
                                        "Attribute name expected.");
        }

        // fourth argument must be an attribute name
        if(!listutils::isSymbol(attrName2)) {
            return listutils::typeError("Error in fourth argument: "
                                        "Attribute name expected.");
        }

        // extract information about tuple block from args[]
        CRelAlgebra::TBlockTI fTBlockInfo =
                CRelAlgebra::TBlockTI(nl->Second(fStream), false);
        CRelAlgebra::TBlockTI sTBlockInfo =
                CRelAlgebra::TBlockTI(nl->Second(sStream), false);

        // extract names of column of attribute from args[]
        std::string fAttrName = nl->SymbolValue(attrName1);
        std::string sAttrName = nl->SymbolValue(attrName2);

        // search for column index in the first relation
        uint64_t fNameIndex;
        if(!GetIndexOfColumn(fTBlockInfo, fAttrName, fNameIndex)) {
            return listutils::typeError("Error in third argument: "
                                        "Invalid column name.");
        }

        // search for column index in the second relation
        uint64_t sNameIndex;
        if(!GetIndexOfColumn(sTBlockInfo, sAttrName, sNameIndex)) {
            return listutils::typeError("Error in fourth argument: "
                                        "Invalid column name.");
        }

        // Initialize the type and size of result tuple block
        // and check for duplicates column names
        CRelAlgebra::TBlockTI rTBlockInfo = CRelAlgebra::TBlockTI(false);

        // structure helps to eliminate the duplicates
        std::set<std::string> columnNames;

        if(fTBlockInfo.GetDesiredBlockSize() >
           sTBlockInfo.GetDesiredBlockSize()) {

            rTBlockInfo.SetDesiredBlockSize(fTBlockInfo.GetDesiredBlockSize());
        }
        else {
            rTBlockInfo.SetDesiredBlockSize(sTBlockInfo.GetDesiredBlockSize());
        }

        for(size_t i=0; i < fTBlockInfo.columnInfos.size(); i++) {
            columnNames.insert(fTBlockInfo.columnInfos[i].name);
            rTBlockInfo.columnInfos.push_back(fTBlockInfo.columnInfos[i]);
        }

        for(size_t i=0; i < sTBlockInfo.columnInfos.size(); i++) {
            if(!columnNames.insert(sTBlockInfo.columnInfos[i].name).second){
                return listutils::typeError("Column name "
                                            + sTBlockInfo.columnInfos[i].name
                                            + " exists in both relations");
            }
            rTBlockInfo.columnInfos.push_back(sTBlockInfo.columnInfos[i]);
        }


        ListExpr commonNames = nl->TwoElemList(nl->IntAtom(fNameIndex),
                                               nl->IntAtom(sNameIndex));

        // Return result type
        return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                                 commonNames,
                                 rTBlockInfo.GetTypeExpr(true));
    }

    ValueMapping spatialJoinTouchRow::valueMappings[] = {
            spatialJoinRowFun,
            spatialJoinColumnFun,
            spatialJoinMFun
    };

    int spatialJoinTouchRow::SelectValueMapping(ListExpr args) {

        ListExpr stream1 = nl->First(args);
        if(listutils::isStream(stream1)) {

            if(!CRelAlgebra::TBlockTI::Check(nl->Second(stream1))) {
                return 0;
            }

            return 1;
        }

        return 2;
    }

    ValueMapping spatialJoinTouchM::valueMappings[] = {
            spatialJoinRowFun,
            spatialJoinColumnFun,
            spatialJoinMFun
    };

    int spatialJoinTouchM::SelectValueMapping(ListExpr args) {

        ListExpr stream1 = nl->First(args);
        if(listutils::isStream(stream1)) {

            if(!CRelAlgebra::TBlockTI::Check(nl->Second(stream1))) {
                return 0;
            }

            return 1;
        }

        return 2;
    }

    ValueMapping spatialJoinTouchColumn::valueMappings[] = {
            spatialJoinRowFun,
            spatialJoinColumnFun,
            spatialJoinMFun
    };

    int spatialJoinTouchColumn::SelectValueMapping(ListExpr args) {

        ListExpr stream1 = nl->First(args);
        if(listutils::isStream(stream1)) {

            if(!CRelAlgebra::TBlockTI::Check(nl->Second(stream1))) {
                return 0;
            }

            return 1;
        }

        return 2;
    }

    class SpatialJoinTOUCHAlgebra : public Algebra
    {
    public:
        SpatialJoinTOUCHAlgebra() : Algebra()
        {
            AddOperator(new spatialJoinTouchRow(), true);
            AddOperator(new spatialJoinTouchM(), true);
            AddOperator(new spatialJoinTouchColumn(), true);
        }
        ~SpatialJoinTOUCHAlgebra() {};
    };


} // end of namespace sjt

extern "C"
Algebra*
InitializeSpatialJoinTOUCHAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
    return (new sjt::SpatialJoinTOUCHAlgebra);
}

