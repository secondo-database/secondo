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

ListExpr insertRelTypeMap(ListExpr args);

namespace distributed2 {    
    template<int x>
    ListExpr dmapXTMT ( ListExpr args );
    
    int dmapXVM(Word* args, Word& result, int message,
                Word& local, Supplier s );
    
    template<class A>
    int cleanUpVMT(Word* args, Word& result, int message,
                Word& local, Supplier s );
}

using namespace drel;
using namespace distributed2;


namespace drelupdate{
/*
1.1. Type Mapping ~drelinsertTM~

*/

ListExpr drelinsertTM( ListExpr args ){
    
    cout << "drelinsertTM" << endl;
    cout << nl->ToString( args ) << endl;
    
    //drel or dfrel?
    std::string err = "stream(tuple(X)) x d[f]rel(rel(X)) expected";
    
    if(!nl->HasLength(args,2)){
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
                ": second argument is not a d[f]rel"); //d[f]rel?   
    }
    
    //streamType != relType (spatial2d, 3d, replicated)
    distributionType type;
    if( !getTypeByNum(nl->IntValue(nl->First(distType)),type)){
        return listutils::typeError( err + 
                ": distribution type not supported"); 
    } else if (type == replicated){
        streamType = nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                        nl->TwoElemList(listutils::basicSymbol<Tuple>(),
                        listutils::concat(
                            nl->Second(nl->Second(streamType)),
                            nl->OneElemList(nl->TwoElemList(
                                    nl->SymbolAtom("Original"),
                                    listutils::basicSymbol<CcBool>())))));
    } else if(type == spatial2d || type == spatial3d){
        streamType = nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                        nl->TwoElemList(listutils::basicSymbol<Tuple>(),
                        listutils::concat(
                            nl->Second(nl->Second(streamType)),
                            nl->TwoElemList(
                                nl->TwoElemList(nl->SymbolAtom("Cell"),
                                    listutils::basicSymbol<CcInt>()),
                                nl->TwoElemList(nl->SymbolAtom("Original"),
                                    listutils::basicSymbol<CcBool>())))));
    }

    cout<<"streamType new: " << nl->ToString(streamType) << endl;
        
    ListExpr result = insertRelTypeMap(
                            nl->TwoElemList(streamType,relType));
    
    cout << nl->ToString( result ) << endl;
    if(!listutils::isTupleStream(result)){
        return result;
    }
    
    // create function type to call dmapXTMT<2>
    ListExpr funList = nl->TwoElemList(
        nl->FourElemList(
            nl->SymbolAtom("map"),
            relType,           
            relType,
            result),
        nl->FourElemList(
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
                    nl->SymbolAtom("dmapelem2"))));
    
    //cout << "funList: " << nl->ToString(funList) << endl;
    
    ListExpr dmapResult = distributed2::dmapXTMT<2>(
        nl->FiveElemList(
            nl->TwoElemList(darrayType, drelValue), //type of stream d[f]rel?
            nl->TwoElemList(darrayType, drelValue), 
            nl->TwoElemList(
                listutils::basicSymbol<CcString>(),
                nl->StringAtom("")),
            funList,
            nl->TwoElemList(
                listutils::basicSymbol<CcInt>(), nl->IntAtom(1238)))); //dummy
    
    //cout << "dmapResult: " << nl->ToString(dmapResult) << endl;
   
    if(!nl->HasLength(dmapResult, 3)){
        return dmapResult;
    }
    
    ListExpr drelResultType; //stream res -> dfarray (D2A)
    if(distributed2::DArray::checkType(nl->Third(dmapResult))){
        drelResultType = listutils::basicSymbol<DRel>();
    } else if(distributed2::DFArray::checkType(nl->Third(dmapResult))){
        drelResultType = listutils::basicSymbol<DFRel>();
    } else {
        return dmapResult;
    }
    
    ListExpr resType = nl->ThreeElemList(
                                drelResultType,
                                nl->Second(nl->Third(dmapResult)),
                                distType);
    
    //cout << "resType: " << nl->ToString(resType) << endl;
    
    ListExpr streamRelType = nl->TwoElemList(
                                    listutils::basicSymbol<Relation>(), 
                                    nl->Second(nl->First(nl->First(args))));
    
    ListExpr append = nl->FiveElemList(
                            nl->First(nl->Second(dmapResult)),
                            nl->Second(nl->Second(dmapResult)),
                            nl->TextAtom(nl->ToString(streamRelType)), 
                            nl->TextAtom(nl->ToString(drelType)),
                            nl->TextAtom(nl->ToString(drelType))); //not used
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

DRel* createDRelOpTree(QueryProcessor* qps,
                        Relation* rel, ListExpr relType,
                        DRel* drel, ListExpr drelType){
    
    distributionType type = drel->getDistType()->getDistType(); 

    ListExpr relPtr = nl->TwoElemList(
                            relType, 
                            nl->TwoElemList(
                                nl->SymbolAtom("ptr"),
                                listutils::getPtrList(rel)));
    ListExpr drelPtr = nl->TwoElemList(
                            drelType, 
                            nl->TwoElemList(
                                nl->SymbolAtom("ptr"),
                                listutils::getPtrList(drel)));
    ListExpr size = nl->TwoElemList(
                        nl->SymbolAtom("size"),
                        nl->TwoElemList(
                            nl->SymbolAtom("drelconvert"), 
                            drelPtr));
    ListExpr workers = nl->TwoElemList(
                            nl->SymbolAtom("consume"),
                            nl->TwoElemList(
                                nl->SymbolAtom("getWorkers"),
                                nl->TwoElemList(
                                    nl->SymbolAtom("drelconvert"),
                                    drelPtr)));
    ListExpr name = nl->TwoElemList(
                        listutils::basicSymbol<CcString>(),
                        nl->StringAtom(""));
    ListExpr dtype = nl->SymbolAtom(getName(type));
    ListExpr query;
    if(type == replicated){
        query = nl->FiveElemList(
                    nl->SymbolAtom("dreldistribute"),
                    relPtr, name, dtype, workers );
    } else if(type == drel::random){
        query = nl->SixElemList(
                    nl->SymbolAtom("dreldistribute"),
                    relPtr, name, dtype, size, workers );
    } else if(type == drel::hash || type == range 
            || type == spatial2d || type == spatial3d){
        
        ListExpr attrList = nl->Second(nl->Second(nl->Second(drelType)));
        int attrPos = nl->IntValue(nl->Second(nl->Third(drelType)));
    
        for(int i = 0; i < attrPos; i++){
            attrList = nl->Rest(attrList); 
        }
        string attrName = nl->ToString(nl->First(nl->First(attrList)));
            
        query = listutils::concat( 
                            nl->OneElemList(
                                nl->SymbolAtom("dreldistribute")),
                            nl->SixElemList(relPtr, name, dtype,
                                nl->SymbolAtom(attrName), 
                                size, workers));
    }
    
    cout<< nl->ToString(query) << endl;
    
    bool correct, evaluable, defined, isFunction;
    ListExpr resultType;
    
    OpTree tree = 0;
    qps->Construct(query, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qps->SetEvaluable(tree, true); 
    
    DRel* drel_tmp = 0;
    if(!correct || !evaluable || !defined){
        drel_tmp->makeUndefined();
    } else {
        Word qRes;
        qps->EvalS(tree, qRes, OPEN);
        qps->Destroy(tree, false);
        drel_tmp = (DRel*) qRes.addr;
    }
    
    return drel_tmp;
}

/*
1.2. Value Mapping ~drelinsertVM~

*/

int drelinsertVM(Word* args, Word& result, int message,
           Word& local, Supplier s ){
    
    Stream<Tuple> stream(args[0].addr);
    DRel* drel = (DRel*) args[1].addr; //D[f]Rel?
    FText* relTypeText = (FText*) args[4].addr;
    FText* drelTypeText = (FText*) args[5].addr;
        
    ListExpr relType, drelType;
    if(    !nl->ReadFromString(relTypeText->GetValue(), relType)
        || !nl->ReadFromString(drelTypeText->GetValue(), drelType)){
        result = qp->ResultStorage(s);
        DRel* resultDRel = (DRel*)result.addr;
        resultDRel->makeUndefined();
        return 0;
    }
    
    cout<< "relType:" << nl->ToString(relType) << endl;
    cout<< "drelType: " << nl->ToString(drelType) << endl;
        
    //collect all tuples from stream into relation 
    stream.open();
    Tuple* tup = stream.request(); //DeleteIfAllowed
    Relation* rel = new Relation(tup->GetTupleType()); 
    rel->AppendTuple(tup); 
    while((tup = stream.request()) != 0){
        rel->AppendTuple(tup);
        tup->DeleteIfAllowed();
    }
    stream.close();
    
    cout << "relation from stream created" << endl;
    
    //distribute relation in the same manner as drel
    QueryProcessor* qps = new QueryProcessor( nl, am );
    DRel* drel_tmp = createDRelOpTree(qps,
                                    rel, relType, drel, drelType);
    //check for defined
    cout << "opTree created" << endl;
    
    //create argument vector for dmapXVM
    ArgVector argVec = {drel_tmp, drel, new CcString(true, ""), 
                        new CcBool(false, false), //dummy, ignored by dmapXVM
                        new CcInt(true, 1238), // port
                        args[2].addr, args[3].addr};
                        
    dmapXVM(argVec, result, message, local, s);
    
    DRel* resultDRel = (DRel*)result.addr;
    
    if(!resultDRel->IsDefined()){
        return 0;
    }
    
    resultDRel->setDistType( drel->getDistType()->copy());
    
    //delete temporary objects from remote server
    
    //check pointers
    
    delete rel;
    delete qps;
    
    return 0;
}

    
/*
1.3 Specification
    
*/
OperatorSpec drelinsertSpec(
    " stream(tuple(X)) x drel(rel(tuple(X))) "
    "-> drel(rel(tuple(x@[TID:tid])))",
    " _ _ drelinsert",
    "Inserts tuples of the stream into the distributed relation.",
    "query rel feed drel drelinsert"
);


/*
1.4 Definition
    
*/
Operator drelinsertOp (
    "drelinsert",                 // name
    drelinsertSpec.getStr( ),          // specification
    drelinsertVM,            // value mapping
    Operator::SimpleSelect,       // trivial selection function
    drelinsertTM            // type mapping
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

