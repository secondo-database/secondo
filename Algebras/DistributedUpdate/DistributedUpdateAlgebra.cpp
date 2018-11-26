/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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

*/


#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecParser.h" 
#include "Algebras/Stream/Stream.h"
#include "Algebras/DRel/DRel.h"
#include "Algebras/DRel/DRelHelpers.h"
#include "Algebras/Distributed2/DArray.h"
#include "Algebras/FText/FTextAlgebra.h"

#include <string.h>
#include <iostream>

extern NestedList* nl;
extern QueryProcessor* qp;

template<int equalCheck>
ListExpr insertDeleteRelTypeMap(ListExpr args);

using namespace drel;
using namespace distributed2;


namespace drelupdate{
/*
1.1 Type Mapping ~drelinsertTM~

*/
ListExpr drelinsertTM( ListExpr args ){
    
    cout << "drelinsertTM" << endl;
    cout << nl->ToString( args ) << endl;
    
    //drel or dfrel?
    std::string err = "stream(tuple(X)) x d[f]rel(rel(X)) x int expected";
    
    if(!nl->HasLength(args,3)){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args)){
        return listutils::typeError("internal Error");
    }    
    // check for correct types
    ListExpr streamType = nl->First(nl->First(args));
    if(!Stream<Tuple>::checkType(streamType)){
        return listutils::typeError(err + 
                ": first argument is not a stream");
    }
    ListExpr drelType, relType, distType, drelValue, darrayType;
    if(!DRelHelpers::isDRelDescr(nl->Second(args), drelType, relType,
        distType, drelValue, darrayType)) {
        return listutils::typeError(err + 
                ": second argument is not a d[f]rel");   
    }
    ListExpr port = nl->Third(args);
    if(!CcInt::checkType(nl->First(port))){
        return listutils::typeError(err + 
                ": last argument not of type int");
    }
    
    //streamType != relType (spatial2d, 3d, replicated)
    distributionType type;
    if( !getTypeByNum(nl->IntValue(nl->First(distType) ),type) ){
        return listutils::typeError( err + 
                ": distribution type not supported"); 
    } else if (type == replicated){
        streamType = nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                        nl->TwoElemList(listutils::basicSymbol<Tuple>(),
                        listutils::concat(
                            nl->Second(nl->Second(streamType) ),
                            nl->OneElemList(nl->TwoElemList(
                                    nl->SymbolAtom("Original"),
                                    listutils::basicSymbol<CcBool>() ) ) ) ) );
    } else if(type == spatial2d || type == spatial3d){
        streamType = nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                        nl->TwoElemList(listutils::basicSymbol<Tuple>(),
                        listutils::concat(
                            nl->Second(nl->Second(streamType) ),
                            nl->TwoElemList(
                                nl->TwoElemList(nl->SymbolAtom("Cell"),
                                    listutils::basicSymbol<CcInt>() ),
                                nl->TwoElemList(nl->SymbolAtom("Original"),
                                    listutils::basicSymbol<CcBool>() ) ) ) ) );
    }
        
    ListExpr result = insertDeleteRelTypeMap<0>( 
                                nl->TwoElemList(streamType, relType) );
                                
    if(!listutils::isTupleStream(result)){
        return result;
    }

    // create function type for dmap2 operator
    ListExpr funList = nl->FourElemList(
            nl->SymbolAtom("fun"),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem1"),
                    nl->SymbolAtom("ARRAYFUNARG1")),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem2"),
                    nl->SymbolAtom("ARRAYFUNARG2")),
            nl->ThreeElemList(
                    nl->SymbolAtom("insert"),
                    nl->TwoElemList(
                            nl->SymbolAtom("feed"),
                            nl->SymbolAtom("dmapelem1")),
                    nl->SymbolAtom("dmapelem2")));
     
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(), 
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(), 
                                nl->Second(result) ), 
                            distType);

    ListExpr append = nl->OneElemList(
                            nl->TextAtom(nl->ToString(funList) ) );
        
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
~distributeSimilarToDRel~

Distributes a relation in the same manner as a given drel. 

*/
DFRel* distributeSimilarToDRel(QueryProcessor* qps,
                                Relation* rel, ListExpr relType,
                                DRel* drel, ListExpr drelType){

    distributionType type = drel->getDistType()->getDistType();
    
    ListExpr relPtr = nl->TwoElemList(
                            relType, 
                            nl->TwoElemList(
                                nl->SymbolAtom("ptr"),
                                listutils::getPtrList(rel) ) );
    ListExpr drelPtr = nl->TwoElemList(
                            nl->TwoElemList(
                                listutils::basicSymbol<distributed2::DArray>(),
                                nl->Second(drelType) ),         
                            nl->TwoElemList(
                                nl->SymbolAtom("ptr"),
                                listutils::getPtrList(drel) ) );
    ListExpr size = nl->TwoElemList(
                        nl->SymbolAtom("size"),
                        drelPtr );
    ListExpr workers = nl->TwoElemList(
                            nl->SymbolAtom("consume"),
                            nl->TwoElemList(
                                nl->SymbolAtom("getWorkers"),
                                drelPtr ) );
    ListExpr name = nl->TwoElemList(
                        listutils::basicSymbol<CcString>(),
                        nl->StringAtom("TMP_DATA") );
    ListExpr dtype = nl->SymbolAtom(getName(type) );
    
    ListExpr query;
    if(type == replicated){
        query = nl->FiveElemList(
                        nl->SymbolAtom("drelfdistribute"),
                        relPtr, name, dtype, workers );
    } 
    else if(type == drel::random){
        query = nl->SixElemList(
                        nl->SymbolAtom("drelfdistribute"),
                        relPtr, name, dtype, size, workers );
    } 
    else if(  type == drel::hash || type == range 
            || type == spatial2d || type == spatial3d){
        
        //get attribute name for distribution
        ListExpr attrList = nl->Second(
                                nl->Second(nl->Second(drelType) ) );
        int attrPos = nl->IntValue(nl->Second(nl->Third(drelType) ) );

        for(int i = 0; i < attrPos; i++){
            attrList = nl->Rest(attrList); 
        }
        string attrName = nl->ToString( nl->First(nl->First(attrList) ) );
            
        query = listutils::concat( 
                        nl->OneElemList(
                            nl->SymbolAtom("drelfdistribute") ),
                        nl->SixElemList(relPtr, name, dtype,
                            nl->SymbolAtom(attrName), size, workers) );
    }
    
    //cout<< nl->ToString(query) << endl;
    
    bool correct, evaluable, defined, isFunction;
    ListExpr resultType;
    
    OpTree tree = 0;
    qps->Construct(query, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qps->SetEvaluable(tree, true); 
    
    DFRel* dfrel_tmp = 0;
    if(!correct || !evaluable || !defined ){
        dfrel_tmp->makeUndefined();
    } else {
        Word qRes;
        qps->EvalS(tree, qRes, OPEN);
        qps->Destroy(tree, false);
        dfrel_tmp = (DFRel*) qRes.addr;
    }
    
    return dfrel_tmp;
}

/*
~removeTempObjects~

Removes temporary objects from a given dfrel.

*/
bool removeTempObjects(QueryProcessor* qps,
                        DFRel* dfrel_tmp, ListExpr drelType){
    
    ListExpr query = nl->TwoElemList(
                        nl->SymbolAtom("deleteRemoteObjects"),
                        nl->TwoElemList(
                            nl->TwoElemList(
                               listutils::basicSymbol<distributed2::DFArray>(),
                               nl->Second( drelType) ),
                            nl->TwoElemList(
                                        nl->SymbolAtom("ptr"),
                                        listutils::getPtrList(dfrel_tmp) ) ) );
    
    bool correct, evaluable, defined, isFunction;
    ListExpr resultType;
    
    OpTree tree = 0;
    qps->Construct(query, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qps->SetEvaluable(tree, true); 
    
    if(!correct || !evaluable || !defined ){
        return false;
    } else {
        Word qRes;
        qps->EvalS(tree, qRes, OPEN);
        qps->Destroy(tree, false);
        int deletedObjects = ((CcInt*) qRes.addr)->GetValue();
        bool success = deletedObjects > 0 ? true : false;
        return success;
    }
}

/*
1.2 Value Mapping ~drelinsertVM~

*/
int drelinsertVM(Word* args, Word& result, int message,
           Word& local, Supplier s ){
    
    Stream<Tuple> stream(args[0].addr);
    DRel* drel = (DRel*) args[1].addr; //D[f]Rel?
    CcInt* port = (CcInt*) args[2].addr;
    FText* funText = (FText*) args[3].addr;
    
    result = qp->ResultStorage(s);
    DFRel* resultDFRel = (DFRel*)result.addr; //D[f]Rel? expected
    
    ListExpr function;
    if( !nl->ReadFromString(funText->GetValue(), function) ){
        resultDFRel->makeUndefined();
        return 0;
    }
    
    //collect all tuples from stream into relation 
    stream.open();
    Tuple* tup = stream.request(); 
    Relation* rel = new Relation(tup->GetTupleType()); 
    rel->AppendTuple(tup);
    tup->DeleteIfAllowed();
    while((tup = stream.request()) != 0){
        rel->AppendTuple(tup);
        tup->DeleteIfAllowed();
    }
    stream.close();
    
    //distribute relation in the same manner as a drel
    QueryProcessor* qps = new QueryProcessor( nl, am );
    
    ListExpr relType = nl->TwoElemList(
                            listutils::basicSymbol<Relation> (),
                            nl->Second( qp->GetType( qp->GetSon(s,0) ) ) );
    ListExpr drelType = qp->GetType( qp->GetSon(s,1) );
    
    DFRel* dfrel_tmp = distributeSimilarToDRel(qps, rel, relType, 
                                               drel, drelType);
    
    ListExpr query = nl->SixElemList(
                        nl->SymbolAtom("dmap2"),
                        nl->TwoElemList(
                            nl->TwoElemList(
                               listutils::basicSymbol<distributed2::DFArray>(),
                               nl->Second(drelType) ),
                            nl->TwoElemList(
                                nl->SymbolAtom("ptr"),
                                listutils::getPtrList(dfrel_tmp) ) ),
                        nl->TwoElemList(
                            nl->TwoElemList( //darraytype
                                listutils::basicSymbol<distributed2::DArray>(),
                                nl->Second(drelType) ),
                            nl->TwoElemList(
                                nl->SymbolAtom("ptr"),
                                listutils::getPtrList(drel) ) ),
                        nl->TwoElemList(
                                listutils::basicSymbol<CcString>(),
                                nl->StringAtom("") ),
                        function,
                        nl->TwoElemList(
                                listutils::basicSymbol<CcInt>(), 
                                nl->IntAtom(port->GetValue() ) ) );            
    
    //cout<< nl->ToString(query) << endl;
    
    bool correct, evaluable, defined, isFunction;
    ListExpr resultType;
    
    OpTree tree = 0;
    qps->Construct(query, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qps->SetEvaluable(tree, true); 
    
    if(!correct || !evaluable || !defined ){
        resultDFRel->makeUndefined();
    } else {
        Word qRes;
        qps->EvalS(tree, qRes, OPEN);
        qps->Destroy(tree, false);
        resultDFRel = (DFRel*) qRes.addr;
        if(resultDFRel->IsDefined()){
            resultDFRel->setDistType( drel->getDistType()->copy() );
        }
        result.setAddr(resultDFRel);
    }
    
    //delete temporary objects from remote server
    if( !removeTempObjects(qps, dfrel_tmp, drelType) ){
        cout<<"temporary objects not deleted" << endl;
    }
        
    //check pointers
    delete dfrel_tmp;
    delete rel;
    delete qps;
    return 0;
}
    
/*
1.3 Specification

*/
OperatorSpec drelinsertSpec(
    " stream(tuple(X)) x drel(rel(tuple(X))) x int"
    "-> drel(rel(tuple(x@[TID:tid])))",
    " _ _ drelinsert [_]",
    "Inserts tuples of the stream into the distributed relation.",
    "query rel feed drel drelinsert [1238]"
);

/*
1.4 Definition

*/
Operator drelinsertOp (
    "drelinsert",                
    drelinsertSpec.getStr( ),         
    drelinsertVM,           
    Operator::SimpleSelect,     
    drelinsertTM           
);  

class DistributedUpdateAlgebra : public Algebra{
    public:
        DistributedUpdateAlgebra() : Algebra(){
            AddOperator(&drelinsertOp);
            drelinsertOp.SetUsesArgsInTypeMapping( );
        }
};
    
    
} //end of namespace drelupdate

extern "C"
Algebra*
   InitializeDistributedUpdateAlgebra( NestedList* nlRef,
                                       QueryProcessor* qpRef ) {
   return new drelupdate::DistributedUpdateAlgebra ();
}
