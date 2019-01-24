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
ListExpr insertTupleTypeMap(ListExpr args);
ListExpr updateSearchRelTypeMap(ListExpr args);
ListExpr appendIdentifierTypeMap (ListExpr args);
ListExpr updatedirect2TM(ListExpr args);
ListExpr deletebyid2TM(ListExpr args);
ListExpr allUpdatesRTreeTypeMap(ListExpr& args, std::string opName);

using namespace drel;
using namespace distributed2;


namespace drelupdate{
/*
1.1 Type Mapping ~drelInsertDeleteTM~

*/
ListExpr drelinsertTM( ListExpr args ){
    
    //cout << "drelinsertTM" << endl;
    //cout << nl->ToString( args ) << endl;
    
    std::string err = "stream(tuple(X)) x drel(rel(X)) x int expected";
    
    if(!nl->HasLength(args,3)){
        return listutils::typeError("wrong number of arguments");
    }    
    // check for correct types
    ListExpr streamType = nl->First( args );
    if(!Stream<Tuple>::checkType(streamType)){
        return listutils::typeError(err + 
                ": first argument is not a stream");
    }
    ListExpr drelType = nl->Second( args );
    if(!DRel::checkType( drelType ) ){
        return listutils::typeError(err +
                ": second argument is not a drel");
    }
    ListExpr port = nl->Third( args );
    if(!CcInt::checkType( port ) ){
        return listutils::typeError(err + 
                ": last argument not of type int");
    }
    
    //streamType != relType (spatial2d, 3d, replicated)
    ListExpr attrList = nl->Second(nl->Second( streamType ) );
    ListExpr distType = nl->Third( drelType );
    
    distributionType type;
    if( !getTypeByNum(nl->IntValue(nl->First(distType) ),type) ){
        return listutils::typeError(
                "distribution type not supported");
    } else if (type == replicated){
        attrList = ConcatLists( 
                            attrList,
                            nl->OneElemList(nl->TwoElemList(
                                nl->SymbolAtom("Original"),
                                listutils::basicSymbol<CcBool>() ) ) ); 
    } else if (type == spatial2d || type == spatial3d) {
        attrList = DRelHelpers::addPartitionAttributes( attrList );
    }
    
    streamType = nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple> >(),
                    nl->TwoElemList(
                        listutils::basicSymbol<Tuple>(), attrList) ); 
        
    ListExpr result = insertDeleteRelTypeMap<0>( nl->TwoElemList(
                                    streamType, nl->Second(drelType) ) );
                                
    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    
    // create function type for dmap2 operator
    /*
    string funText = "(fun (dmapelem1 ARRAYFUNARG1) (dmapelem2 ARRAYFUNARG2) "
                     "(insert (feed dmapelem1) dmapelem2))";
    */
    ListExpr funText = nl->FourElemList(
            nl->SymbolAtom("fun"),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem1"),
                    nl->SymbolAtom("ARRAYFUNARG1")),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem2"),
                    nl->SymbolAtom("ARRAYFUNARG2")),
            nl->ThreeElemList(
                    nl->SymbolAtom( "insert" ),
                    nl->TwoElemList(
                            nl->SymbolAtom("feed"),
                            nl->SymbolAtom("dmapelem1")),
                    nl->SymbolAtom("dmapelem2") ) );
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(), 
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(), 
                                nl->Second(result) ), 
                            distType);

    ListExpr append = nl->OneElemList(
                            nl->TextAtom( nl->ToString(funText) ) );
        
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
~distributeSimilarToDRel~

Distributes a relation in the same manner as a given drel. 

*/
//DFRel* distributeSimilarToDRel(QueryProcessor* qps,
DFRel* distributeSimilarToDRel( Relation* rel, ListExpr relType,
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
                        nl->StringAtom("") );
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
        std::string attrName = nl->ToString( nl->First(nl->First(attrList) ) );
            
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
    qp->Construct(query, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qp->SetEvaluable(tree, true); 
    
    DFRel* dfrel_tmp = 0;
    if(!correct || !evaluable || !defined ){
        dfrel_tmp->makeUndefined();
    } else {
        Word qRes;
        qp->EvalS(tree, qRes, OPEN);
        qp->Destroy(tree, false);
        dfrel_tmp = (DFRel*) qRes.addr;
    }
    
    return dfrel_tmp;
}

/*
~removeTempObjects~

Removes temporary objects from a given dfrel.

*/
bool removeTempObjects(QueryProcessor* qps,
                        DRel* drel_tmp, ListExpr drelType){
    
    ListExpr query = nl->TwoElemList(
                        nl->SymbolAtom("deleteRemoteObjects"),
                        nl->TwoElemList(
                            nl->TwoElemList(
                               listutils::basicSymbol<distributed2::DArray>(),
                               nl->Second( drelType) ),
                            nl->TwoElemList(
                                        nl->SymbolAtom("ptr"),
                                        listutils::getPtrList(drel_tmp) ) ) );
    
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
    DRel* drel = (DRel*) args[1].addr; 
    CcInt* port = (CcInt*) args[2].addr;
    FText* funText = (FText*) args[3].addr;
    
    result = qp->ResultStorage(s);
    DFRel* resultDFRel = (DFRel*)result.addr;
    
    ListExpr function;
    if( !nl->ReadFromString(funText->GetValue(), function)){
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
    //QueryProcessor* qps = new QueryProcessor( nl, am );
    
    ListExpr relType = nl->TwoElemList(
                            listutils::basicSymbol<Relation> (),
                            nl->Second( qp->GetType( qp->GetSon(s,0) ) ) );
    ListExpr drelType = qp->GetType( qp->GetSon(s,1) );
    
    //DFRel* dfrel_tmp = distributeSimilarToDRel(qp, rel, relType, 
    DFRel* dfrel_tmp = distributeSimilarToDRel(rel, relType,
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
    qp->Construct(query, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qp->SetEvaluable(tree, true); 
    
    if(!correct || !evaluable || !defined ){
        resultDFRel->makeUndefined();
    } else {
        Word qRes;
        qp->EvalS(tree, qRes, OPEN);
        qp->Destroy(tree, false);
        resultDFRel = (DFRel*) qRes.addr;
        if(resultDFRel->IsDefined()){
            resultDFRel->setDistType( drel->getDistType()->copy() );
        }
        result.setAddr(resultDFRel);
    }
    
    /*
    //delete temporary objects from remote server
    if( !removeTempObjects(qps, drel_tmp, drelType) ){
        cout<<"temporary objects not deleted" << endl;
    }
    */
        
    //check pointers
    delete dfrel_tmp;
    delete rel;
    //delete qps;
    return 0;
}




ListExpr dreldeleteTM ( ListExpr args ){
    
    /*
    cout << "dreldeleteTM" << endl;
    cout << nl->ToString( args ) << endl;
    */

    if(!nl->HasLength(args, 3)){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args) ){
        return listutils::typeError("internal Error");
    }
    // check for correct types
    ListExpr drelType1, relType1, distType1, drelValue1, darrayType1;
    if(!DRelHelpers::isDRelDescr(nl->First(args), drelType1, relType1,
        distType1, drelValue1, darrayType1) ) {
        return listutils::typeError( 
                "first argument is not a d[f]rel");   
    }
    ListExpr drelType2, relType2, distType2, drelValue2, darrayType2;
    if(!DRelHelpers::isDRelDescr(nl->Second(args), drelType2, relType2,
        distType2, drelValue2, darrayType2) ) {
        return listutils::typeError( 
                "second argument is not a d[f]rel");  //only drel 
    }
    if( nl->SymbolValue( nl->First( drelType2 ) ) != DRel::BasicType( ) ){
        return listutils::typeError(
                "second argument must be a drel");
    } 

    //check for equality of distribution types
    
    ListExpr port = nl->Third(args);
    if(!CcInt::checkType(nl->First(port))){
        return listutils::typeError(
                "last argument not of type int");
    }
    
    ListExpr result = insertDeleteRelTypeMap<0>(
                            nl->TwoElemList(
                                nl->TwoElemList(
                                    listutils::basicSymbol<Stream<Tuple> >(),
                                    nl->Second(relType1) ),
                                relType2 ) );
    
    //cout << nl->ToString(result) << endl;
    
    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    
    // create function type for dmap2 operator
    ListExpr function= nl->FourElemList(
            nl->SymbolAtom("fun"),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem1"),
                    nl->SymbolAtom("ARRAYFUNARG1") ),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem2"),
                    nl->SymbolAtom("ARRAYFUNARG2") ),
            nl->ThreeElemList(
                    nl->SymbolAtom( "deletesearch" ),
                    nl->TwoElemList(
                            nl->SymbolAtom("feed"),
                            nl->SymbolAtom("dmapelem1") ),
                    nl->SymbolAtom("dmapelem2") ) );
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second(result) ),
                            distType1);     //disttype1 = distType2
    
    ListExpr append = nl->OneElemList(
                            nl->TextAtom(nl->ToString(function) ) );
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}


ListExpr drelupdateTM ( ListExpr args ){
    
    /*
    cout << "drelupdateTM" << endl;
    cout << nl->ToString( args ) << endl;
    */
    
    if(!nl->HasLength(args, 4)){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args) ){
        return listutils::typeError("internal Error");
    }
    // check for correct types
    ListExpr drelType1, relType1, distType1, drelValue1, darrayType1;
    if(!DRelHelpers::isDRelDescr(nl->First(args), drelType1, relType1,
        distType1, drelValue1, darrayType1) ) {
        return listutils::typeError( 
                "first argument is not a d[f]rel");   
    }
    ListExpr drelType2, relType2, distType2, drelValue2, darrayType2;
    if(!DRelHelpers::isDRelDescr(nl->Second(args), drelType2, relType2,
        distType2, drelValue2, darrayType2) ) {
        return listutils::typeError( 
                "second argument is not a d[f]rel");  //only drel 
    }
    if( nl->SymbolValue( nl->First( drelType2 ) ) != DRel::BasicType( ) ){
        return listutils::typeError(
                "second argument must be a drel");
    }
    
    //check for equality of distribution types
    
    ListExpr mapList = nl->First( nl->Third(args) ); 
    ListExpr funList = nl->Second( nl->Third(args) );

    ListExpr tempfun;
    ListExpr funList_new = nl->TheEmptyList( );
    while( !nl->IsEmpty( funList ) ){
        
        if( !nl->HasLength( nl->First( funList ), 2 ) ){
            return listutils::typeError( "internal error" );
        }
        
        if( !DRelHelpers::replaceDRELFUNARG(
                nl->Second (nl->First( funList ) ), "TUPLE", tempfun ) ) {
            return listutils::typeError( "error in the function format" );
        }
        
        if( nl->IsEmpty( funList_new ) ){
            funList_new = nl->OneElemList( nl->TwoElemList(
                nl->First(nl->First( funList ) ), tempfun ) );
        } else {
            funList_new = listutils::concat( funList_new,
                        nl->OneElemList( nl->TwoElemList(
                            nl->First(nl->First( funList ) ), tempfun ) ) );
        }
        
        funList = nl->Rest( funList );
    }

    /*
    cout << nl->ToString(mapList) << endl;
    cout << nl->ToString(funList_new) << endl;
    */
    
    ListExpr port = nl->Fourth(args);
    if(!CcInt::checkType(nl->First(port))){
        return listutils::typeError(
                "last argument not of type int");
    }
    
    ListExpr result = updateSearchRelTypeMap(
                            nl->ThreeElemList(
                                nl->TwoElemList(
                                    listutils::basicSymbol<Stream<Tuple> >(),
                                    nl->Second(relType1) ),
                                relType2,
                                mapList ) );
    
    //cout << nl->ToString(result) << endl;
    
    if( !nl->HasLength(result, 3) ){
        return result;
    }
    
    // create function type for dmap2 operator
    ListExpr function= nl->FourElemList(
            nl->SymbolAtom("fun"),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem1"),
                    nl->SymbolAtom("ARRAYFUNARG1") ),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem2"),
                    nl->SymbolAtom("ARRAYFUNARG2") ),
            nl->FourElemList(
                    nl->SymbolAtom( "updatesearch" ),
                    nl->TwoElemList(
                            nl->SymbolAtom("feed"),
                            nl->SymbolAtom("dmapelem1") ),
                    nl->SymbolAtom("dmapelem2"),
                    funList_new ) );
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second(nl->Third(result) ) ),
                            distType1);     //disttype1 = distType2
    
    ListExpr append = nl->OneElemList(
                            nl->TextAtom(nl->ToString(function) ) );
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

ListExpr drelupdatebyidTM ( ListExpr args ){
    
    /*
    cout << "drelupdate2TM" << endl;
    cout << nl->ToString( args ) << endl;
    */
    
    if(!nl->HasLength(args, 5)){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args) ){
        return listutils::typeError("internal Error");
    }
    // check for correct types
    ListExpr drelType1, relType1, distType1, drelValue1, darrayType1;
    if(!DRelHelpers::isDRelDescr(nl->First(args), drelType1, relType1,
        distType1, drelValue1, darrayType1) ) {
        return listutils::typeError( 
                "first argument is not a d[f]rel");   
    }
    ListExpr drelType2, relType2, distType2, drelValue2, darrayType2;
    if(!DRelHelpers::isDRelDescr(nl->Second(args), drelType2, relType2,
        distType2, drelValue2, darrayType2) ) {
        return listutils::typeError( 
                "second argument is not a d[f]rel");  //only drel 
    }
    if( nl->SymbolValue( nl->First( drelType2 ) ) != DRel::BasicType( ) ){
        return listutils::typeError(
                "second argument must be a drel");
    }
    ListExpr attrName = nl->Second ( nl->Third( args ) );
    if(!listutils::isSymbol( attrName ) ){
        return listutils::typeError(
                "third argument must be an attribute name" );
    }
    
    //check for equality of distribution types
    
    ListExpr mapList = nl->First( nl->Fourth(args) ); 
    ListExpr funList = nl->Second( nl->Fourth(args) );

    ListExpr tempfun;
    ListExpr fundef;
    ListExpr funList_new = nl->TheEmptyList( );
    while( !nl->IsEmpty( funList ) ){
        
        if( !nl->HasLength( nl->First( funList ), 2 ) ){
            return listutils::typeError( "internal error" );
        }
        
        fundef = nl->Second( nl->First( funList ) );
        if(    !nl->HasLength( fundef, 4 )
            || !nl->HasLength( nl->Second( fundef ), 2 )
            || !nl->HasLength( nl->Third( fundef ), 2 )
            || !listutils::isSymbol( nl->Second ( nl->Second( fundef ) ) )
            || !listutils::isSymbol( nl->Second ( nl->Third( fundef ) ) ) ) 
        {
            return listutils::typeError( "error in the function format" );
        }
        
        tempfun = nl->FourElemList(
                        nl->First( fundef ),
                        nl->TwoElemList(
                            nl->First ( nl->Second( fundef ) ), 
                            nl->SymbolAtom( "TUPLE" ) ),
                        nl->TwoElemList(
                            nl->First ( nl->Third( fundef ) ),
                            nl->SymbolAtom( "TUPLE2" ) ),
                        nl->Fourth( fundef ) );
        
        if( nl->IsEmpty( funList_new ) ){
            funList_new = nl->OneElemList( nl->TwoElemList(
                nl->First(nl->First( funList ) ), tempfun ) );
        } else {
            funList_new = listutils::concat( funList_new,
                        nl->OneElemList( nl->TwoElemList(
                            nl->First(nl->First( funList ) ), tempfun ) ) );
        }
        
        funList = nl->Rest( funList );
    }

    /*
    cout << nl->ToString(mapList) << endl;
    cout << nl->ToString(funList_new) << endl;
    */
    
    ListExpr port = nl->Fifth( args );
    if(!CcInt::checkType( nl->First(port) ) ){
        return listutils::typeError(
                "last argument not of type int");
    }
    
    ListExpr result = updatedirect2TM(
                            nl->FourElemList(
                                nl->TwoElemList(
                                    listutils::basicSymbol<Stream<Tuple> >(),
                                    nl->Second(relType1) ),
                                relType2,
                                attrName,
                                mapList ) );
    
    //cout << nl->ToString(result) << endl;
    
    if( !nl->HasLength(result, 3) ){
        return result;
    }
    
    // create function type for dmap2 operator
    ListExpr function= nl->FourElemList(
            nl->SymbolAtom("fun"),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem1"),
                    nl->SymbolAtom("ARRAYFUNARG1") ),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem2"),
                    nl->SymbolAtom("ARRAYFUNARG2") ),
            nl->FiveElemList(
                    nl->SymbolAtom( "updatedirect2" ),
                    nl->TwoElemList(
                            nl->SymbolAtom("feed"),
                            nl->SymbolAtom("dmapelem1") ),
                    nl->SymbolAtom("dmapelem2"),
                    attrName,
                    funList_new ) );
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second(nl->Third(result) ) ),
                            distType1);     //disttype1 = distType2
    
    ListExpr append = nl->OneElemList(
                            nl->TextAtom(nl->ToString(function) ) );
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

ListExpr dreldeletebyidTM ( ListExpr args ){
    
    /*
    cout << "dreldeletebyid2TM" << endl;
    cout << nl->ToString( args ) << endl;
    */
    
    if(!nl->HasLength(args, 4)){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args) ){
        return listutils::typeError("internal Error");
    }
    // check for correct types
    ListExpr drelType1, relType1, distType1, drelValue1, darrayType1;
    if(!DRelHelpers::isDRelDescr(nl->First(args), drelType1, relType1,
        distType1, drelValue1, darrayType1) ) {
        return listutils::typeError( 
                "first argument is not a d[f]rel");   
    }
    ListExpr drelType2, relType2, distType2, drelValue2, darrayType2;
    if(!DRelHelpers::isDRelDescr(nl->Second(args), drelType2, relType2,
        distType2, drelValue2, darrayType2) ) {
        return listutils::typeError( 
                "second argument is not a d[f]rel");  //only drel 
    }
    if( nl->SymbolValue( nl->First( drelType2 ) ) != DRel::BasicType( ) ){
        return listutils::typeError(
                "second argument must be a drel");
    }
    ListExpr attrName = nl->Second ( nl->Third( args ) );
    if(!listutils::isSymbol( attrName ) ){
        return listutils::typeError(
                "third argument must be an attribute name" );
    }
    
    //check for equality of distribution types
    
    ListExpr port = nl->Fourth( args );
    if(!CcInt::checkType( nl->First(port) ) ){
        return listutils::typeError(
                "last argument not of type int");
    }
    
    ListExpr attrList1 = nl->Second( nl->Second( relType1 ) );
    ListExpr attrType;
    std::string name = nl->SymbolValue(attrName);
    int index = listutils::findAttribute(attrList1, name, attrType);
    if(!index){
    return listutils::typeError("Attribute not exist");
    }
    
    for(int i = 1; i < index; i++){
        attrList1 = nl->Rest( attrList1 );
    }
    attrList1 = nl->First( attrList1 );
    
    
    ListExpr result = deletebyid2TM(
                            nl->ThreeElemList(
                                nl->TwoElemList(
                                    listutils::basicSymbol<Stream<Tuple> >(),
                                    nl->TwoElemList(
                                        listutils::basicSymbol<Tuple>(),
                                        nl->OneElemList( attrList1 ) ) ),
                                relType2,
                                attrName ) ); //check TID

    //cout << nl->ToString(result) << endl;
    
    if( !nl->HasLength(result, 3) ){
        return result;
    }
    
    // create function type for dmap2 operator
    ListExpr function= nl->FourElemList(
            nl->SymbolAtom("fun"),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem1"),
                    nl->SymbolAtom("ARRAYFUNARG1") ),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem2"),
                    nl->SymbolAtom("ARRAYFUNARG2") ),
            nl->FourElemList(
                    nl->SymbolAtom( "deletebyid2" ),
                    nl->ThreeElemList(
                        nl->SymbolAtom("project"),
                        nl->TwoElemList(
                            nl->SymbolAtom("feed"),
                            nl->SymbolAtom("dmapelem1") ),
                        nl->OneElemList( attrName ) ),
                    nl->SymbolAtom("dmapelem2"),
                    attrName ) ); // check TID
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second(nl->Third(result) ) ),
                            distType1);     //disttype1 = distType2
    
    ListExpr append = nl->OneElemList(
                            nl->TextAtom(nl->ToString(function) ) );
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

ListExpr drelAllUpdatesRTreeTM (ListExpr& args, std::string opName ){
    
    /*
    cout << "insertrtreeTM" << endl;
    cout << nl->ToString( args ) << endl;
    */
    
    if(!nl->HasLength(args, 4) ){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args) ){
        return listutils::typeError("internal error");
    }
    ListExpr drelType, relType, distType, drelValue, darrayType;
    if(!DRelHelpers::isDRelDescr(nl->First(args), drelType, relType,
        distType, drelValue, darrayType) ){
        return listutils::typeError(
                "first argument is not a d[f]rel"); // dfrel?
    }
    ListExpr darrayRTreeType = nl->First( nl->Second( args ) );
    if( !DArray::checkType( darrayRTreeType ) ){ //only DFArray
        return listutils::typeError(
                "second argument is not a darray");
    }
    
    //check for attrName
    ListExpr attrName = nl->Second ( nl->Third( args ) );
    if(!listutils::isSymbol( attrName ) ){
        return listutils::typeError(
                "third argument must be an attribute name" );
    }
    
    ListExpr port = nl->Fourth(args);
    if(!CcInt::checkType(nl->First(port) ) ){
        return listutils::typeError(
                "last argument not of type int");
    }
    
    ListExpr args2 = nl->ThreeElemList(
                            nl->TwoElemList(
                                listutils::basicSymbol<Stream<Tuple> >(),
                                nl->Second( relType ) ),
                            nl->Second( darrayRTreeType ),
                            attrName );
    
    
    ListExpr result = allUpdatesRTreeTypeMap(
                            args2, opName);
    /*
                        nl->ThreeElemList(
                            nl->TwoElemList(
                                listutils::basicSymbol<Stream<Tuple> >(),
                                nl->Second( relType ) ),
                            nl->Second( darrayRTreeType ),
                            attrName ), 
                            opName );
    */
    
    //cout << nl->ToString(result) << endl;
    
    if( !nl->HasLength(result, 3) ){
        return result;
    }
    
    if(opName == "updatebtree"){ //not a good idea
        opName = "updatertree";
    }
    
    // create function type for dmap2 operator
    ListExpr function= nl->FourElemList(
            nl->SymbolAtom("fun"),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem1"),
                    nl->SymbolAtom("ARRAYFUNARG1") ),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem2"),
                    nl->SymbolAtom("ARRAYFUNARG2") ),
            nl->FourElemList(
                    nl->SymbolAtom( opName ),
                    nl->TwoElemList(
                            nl->SymbolAtom("feed"),
                            nl->SymbolAtom("dmapelem1") ),
                    nl->SymbolAtom("dmapelem2"),
                    attrName ) ); 
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second(nl->Third(result) ) ),
                            distType);
    
    ListExpr append = nl->OneElemList(
                            nl->TextAtom(nl->ToString(function) ) );

    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

ListExpr drelinsertrtreeTM(ListExpr args){
    return drelAllUpdatesRTreeTM(args, "insertrtree");
}

ListExpr dreldeletertreeTM(ListExpr args){
    return drelAllUpdatesRTreeTM(args, "deletertree");
}

ListExpr drelupdatertreeTM(ListExpr args){
    return drelAllUpdatesRTreeTM(args, "updatebtree");
}

/*
dmap2VM

*/

int drelupdateVM(Word* args, Word& result, int message,
           Word& local, Supplier s ){
    
    int argsNum = qp->GetNoSons( s );
   
    //cout<< "argsNum: " << argsNum << endl;
   
    DFRel* dfrel = (DFRel*) args[0].addr; //template
    //DRel* drel = (DRel*) args[1].addr;
    DArray* da = (DArray*) args[1].addr;
    CcInt* port = (CcInt*) args[argsNum - 2].addr;
    FText* funText = (FText*) args[argsNum - 1].addr;
    
    result = qp->ResultStorage(s);
    DFRel* resultDFRel = (DFRel*)result.addr; //D[f]Rel? expected
    
    ListExpr function;
    if( !nl->ReadFromString(funText->GetValue(), function) ){
        resultDFRel->makeUndefined();
        return 0;
    }
    
    ListExpr drelType1 = qp->GetType( qp->GetSon(s,0) );
    ListExpr drelType2 = qp->GetType( qp->GetSon(s,1) );
    
    ListExpr query = nl->SixElemList(
                        nl->SymbolAtom("dmap2"),
                        nl->TwoElemList(
                            nl->TwoElemList( // check
                               listutils::basicSymbol<distributed2::DFArray>(),
                               nl->Second(drelType1) ),
                            nl->TwoElemList(
                                nl->SymbolAtom("ptr"),
                                listutils::getPtrList(dfrel) ) ),
                        nl->TwoElemList(
                            nl->TwoElemList( //darraytype
                                listutils::basicSymbol<distributed2::DArray>(),
                                nl->Second(drelType2) ),
                            nl->TwoElemList(
                                nl->SymbolAtom("ptr"),
                                listutils::getPtrList( da ) ) ), // drel_old
                        nl->TwoElemList(
                                listutils::basicSymbol<CcString>(),
                                nl->StringAtom("") ),
                        function,
                        nl->TwoElemList(
                                listutils::basicSymbol<CcInt>(), 
                                nl->IntAtom(port->GetValue() ) ) ); 
    
    bool correct, evaluable, defined, isFunction;
    ListExpr resultType;
    
    OpTree tree = 0;
    qp->Construct(query, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qp->SetEvaluable(tree, true); 
    
    if(!correct || !evaluable || !defined ){
        resultDFRel->makeUndefined();
    } else {
        Word qRes;
        qp->EvalS(tree, qRes, OPEN);
        qp->Destroy(tree, false);
        resultDFRel = (DFRel*) qRes.addr;
        if(resultDFRel->IsDefined()){ // not a good idea
            resultDFRel->setDistType( dfrel->getDistType()->copy() );
        }
        result.setAddr(resultDFRel);
    }
    
    return 0;
}

ListExpr dreladdidTM ( ListExpr args ){

    /*
    cout << "dreladdidTM" << endl;
    cout << nl->ToString( args ) << endl;
    */
    
    if(!nl->HasLength(args, 1) ){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args) ){
        return listutils::typeError("internal error");
    }
    // check for correct types
    ListExpr drelType, relType, distType, drelValue, darrayType;
    if(!DRelHelpers::isDRelDescr(nl->First(args), drelType, relType,
        distType, drelValue, darrayType) ){
        return listutils::typeError(
                "first argument is not a d[f]rel"); //only drel
    }
    
    ListExpr result = appendIdentifierTypeMap(
                        nl->OneElemList(
                            nl->TwoElemList(
                                listutils::basicSymbol<Stream<Tuple> >(),
                                nl->Second(relType) ) ) );
    
    //cout << nl->ToString(result) << endl;
    
    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    //create function type for dmap operator
    ListExpr function = nl->ThreeElemList(
                            nl->SymbolAtom("fun"),
                            nl->TwoElemList(
                                nl->SymbolAtom("dmapelem1"),
                                nl->SymbolAtom("ARRAYFUNARG1") ),
                            nl->TwoElemList(
                                nl->SymbolAtom("addid"),
                                nl->TwoElemList(
                                    nl->SymbolAtom("feed"),
                                    nl->SymbolAtom("dmapelem1") ) ) );
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second( result ) ),
                            distType);
    
    ListExpr append = nl->OneElemList(
                            nl->TextAtom(nl->ToString(function) ) );

    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
dmapVM

*/

int dreladdidVM(Word* args, Word& result, int message,
           Word& local, Supplier s ){
    
    DRel* drel = (DRel*) args[0].addr; //only drel or template?
    FText* funText = (FText*) args[1].addr;
    
    result = qp->ResultStorage(s);
    DFRel* resultDFRel = (DFRel*)result.addr;

    ListExpr function;
    if( !nl->ReadFromString(funText->GetValue(), function) ){
        resultDFRel->makeUndefined();
        return 0;
    }
    
    ListExpr drelType = qp->GetType( qp->GetSon(s, 0) );
    
    ListExpr query = nl->FourElemList(
                        nl->SymbolAtom("dmap"),
                        nl->TwoElemList(
                            nl->TwoElemList(
                                listutils::basicSymbol<distributed2::DArray>(),
                                nl->Second(drelType) ),
                            nl->TwoElemList(
                                nl->SymbolAtom("ptr"),
                                listutils::getPtrList(drel) ) ),
                        nl->TwoElemList(
                                listutils::basicSymbol<CcString>(),
                                nl->StringAtom("") ),
                        function );
    
    bool correct, evaluable, defined, isFunction;
    ListExpr resultType;
    
    OpTree tree = 0;
    qp->Construct(query, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qp->SetEvaluable(tree, true); 
    
    if(!correct || !evaluable || !defined ){
        resultDFRel->makeUndefined();
    } else {
        Word qRes;
        qp->EvalS(tree, qRes, OPEN);
        qp->Destroy(tree, false);
        resultDFRel = (DFRel*) qRes.addr;
        if(resultDFRel->IsDefined()){
            resultDFRel->setDistType( drel->getDistType()->copy() );
        }
        result.setAddr(resultDFRel);
    }
    
    return 0;
}



ListExpr drelinserttupleTM( ListExpr args ){
    
    /*
    cout << "drelinserttupleTM" << endl;
    cout << nl->ToString(args) << endl;
    */
    
    if(!nl->HasLength(args, 3) ){
        return listutils::typeError("wrong number of arguments");
    }
    //list of two elem - internal error
    
    ListExpr drelType = nl->First( args );
    ListExpr tupleType = nl->Second( args );
    
    // check for correct types
    if(!DRel::checkType( drelType ) ){  //only drel - must be checked
        return listutils::typeError(
                "first argument is not a drel");
    }
    if(nl->AtomType( tupleType ) != NoAtom){
        return listutils::typeError(
                "second argument must be a list of attributes");
    }
    if(!CcInt::checkType( nl->Third( args ) ) ){
        return listutils::typeError(
                "last argument not of type int");
    }
    
    //streamType != relType (spatial2d, 3d, replicated)
    ListExpr attrList = nl->Second(nl->Second(nl->Second( drelType ) ) );
    
    distributionType type;
    if(!getTypeByNum(nl->IntValue(nl->First( nl->Third( drelType ) ) ),type) ){
        return listutils::typeError(  
                "distribution type not supported"); 
    } else if (type == replicated || type == spatial2d || type == spatial3d ){
        attrList = DRelHelpers::removePartitionAttributes(attrList, type);
    }
    
    //cout << "attrList: " << nl->ToString(attrList) << endl;
    
    ListExpr result = insertTupleTypeMap( nl->TwoElemList(
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->TwoElemList(
                                    listutils::basicSymbol<Tuple>(),
                                    attrList ) ),
                            tupleType ) );

    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    
    // create function type for dmap2 operator, maybe to VM?
    ListExpr funList = nl->FourElemList(
            nl->SymbolAtom("fun"),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem1"),
                    nl->SymbolAtom("ARRAYFUNARG1")),
            nl->TwoElemList(
                    nl->SymbolAtom("dmapelem2"),
                    nl->SymbolAtom("ARRAYFUNARG2")),
            nl->ThreeElemList(
                    nl->SymbolAtom( "insert" ),
                    nl->TwoElemList(
                            nl->SymbolAtom("feed"),
                            nl->SymbolAtom("dmapelem1")),
                    nl->SymbolAtom("dmapelem2")));
    
    ListExpr resAttrList = listutils::concat(
                nl->Second(nl->Second(nl->Second( drelType ) ) ),
                nl->OneElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("TID"),
                        nl->SymbolAtom(TupleIdentifier::BasicType() ) ) ) );

    //cout << "resAttrList: " << nl->ToString(resAttrList) << endl;

    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->TwoElemList(
                                    listutils::basicSymbol<Tuple>(),
                                    resAttrList) ),
                            nl->Third( drelType ) ); //distType
    
    ListExpr append = nl->TwoElemList(
                            nl->TextAtom(nl->ToString(funList) ),
                            nl->TextAtom(nl->ToString(attrList) ) );
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

int drelinserttupleVM(Word* args, Word& result, int message,
           Word& local, Supplier s ){
   
    int argsNum = qp->GetNoSons( s );
    
    //cout<< "argsNum: " << argsNum << endl;

    DRel* drel = (DRel*) args[0].addr;
    CcInt* port = (CcInt*) args[argsNum - 3].addr;
    FText* funText = (FText*) args[argsNum - 2].addr;
    FText* attrList = (FText*) args[argsNum - 1].addr;
    
    result = qp->ResultStorage(s);
    DFRel* resultDFRel = (DFRel*)result.addr; //D[f]Rel? expected
    
    ListExpr function, attrs;
    if( !nl->ReadFromString(funText->GetValue(), function )
        || !nl->ReadFromString(attrList->GetValue(), attrs ) ){
        resultDFRel->makeUndefined();
        return 0;
    }
    
    //cout<< "attrs: " << nl->ToString( attrs ) << endl;
    
    ListExpr resTupleType = nl->Second(
                                nl->Second( GetTupleResultType( s ) ) );
    
    ListExpr rest, last, insertType;
    rest = nl->Second( resTupleType );
    insertType = nl->OneElemList(nl->First( rest ) );
    last = insertType;
    rest = nl->Rest( rest );
    
    int i = 1;
    distributionType type = drel->getDistType()->getDistType();
    if(type == replicated)
        i = 2;
    else if( type == spatial2d || type == spatial3d )
        i = 3;
    
    
    while(nl->ListLength( rest ) > i){ // not for s2d, s3d, re
        last = nl->Append(last, nl->First( rest ) );
        rest = nl->Rest( rest );
    }
    insertType = nl->TwoElemList(nl->First( resTupleType ),
                                 insertType);
        
    //cout << "insrtType: " << nl->ToString ( insertType ) << endl;
    
    TupleType* insertTupleType = new TupleType( insertType );
    Tuple* insertTuple = new Tuple( insertTupleType );
    
    Supplier supplier = args[1].addr;
    Supplier supplier_i;
    Word attrValue;
    Attribute* attr;
    for(int i = 0; i < insertTuple->GetNoAttributes(); i++){
        supplier_i = qp->GetSupplier(supplier, i);
        qp->Request(supplier_i, attrValue);
        attr = (Attribute*) attrValue.addr;
        insertTuple->PutAttribute(i, attr->Clone() );
    }
    
    Relation* rel = new Relation(insertTuple->GetTupleType() );
    rel->AppendTuple( insertTuple );
    
    //distribute relation in the same manner as a drel
    //QueryProcessor* qps = new QueryProcessor( nl, am );
    
    ListExpr relType = nl->TwoElemList(
                            listutils::basicSymbol<Relation> (),
                            nl->TwoElemList(
                                listutils::basicSymbol<Tuple>(),
                                attrs ) );
    
    ListExpr drelType = qp->GetType( qp->GetSon(s,0) );
    
    //DFRel* dfrel_tmp = distributeSimilarToDRel(qps, rel, relType,
    DFRel* dfrel_tmp = distributeSimilarToDRel(rel, relType,
                                             drel, drelType );
    
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
    
    bool correct, evaluable, defined, isFunction;
    ListExpr resultType;
    
    OpTree tree = 0;
    qp->Construct(query, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qp->SetEvaluable(tree, true); 
    
    if(!correct || !evaluable || !defined ){
        resultDFRel->makeUndefined();
    } else {
        Word qRes;
        qp->EvalS(tree, qRes, OPEN);
        qp->Destroy(tree, false);
        resultDFRel = (DFRel*) qRes.addr;
        if(resultDFRel->IsDefined()){
            resultDFRel->setDistType( drel->getDistType()->copy() );
        }
        result.setAddr(resultDFRel);
    }
    
    /*
    //delete temporary objects from remote server
    if( !removeTempObjects(qps, drel_tmp, drelType) ){
        cout<<"temporary objects not deleted" << endl;
    }
    */
    
        
    //check pointers
    delete dfrel_tmp;
    delete rel;
    //delete qps;
    return 0;
}
    
/*
1.3 Specification

*/
OperatorSpec drelinsertSpec(
    " stream(tuple(X)) x drel(rel(tuple(X))) x int"
    "-> dfrel(rel(tuple(X@[TID:tid])))",
    " _ _ drelinsert [_]",
    "Inserts tuples of the stream into the distributed relation. "
    "The tuples to insert should not contain the partition " 
    "attributes (Cell, Original). The third argument specifies "
    "a port number",
    "query rel1 feed drel1 drelinsert [1238]"
);

OperatorSpec drelinserttupleSpec(
    " drel(rel(tuple(X))) x [t1 ... tn] x int"
    "-> dfrel(rel(tuple(X@[TID:tid])))",
    " _ drelinserttuple [_; _]",
    "Inserts a new tuple with the values from "
    "the second argument-list into the distributed relation. "
    "The third argument specifies a port number. ",
    "query drel1 drelinserttuple [\"Z\", 10 ; 1238]"
);

OperatorSpec dreldeleteSpec( 
    " d[f]rel(rel(tuple(X))) x drel(rel(tuple(X))) x int"
    "-> dfrel(rel(tuple(X@[TID:tid])))",
    " _ _ dreldelete [_]",
    "Deletes tuples contained in the first distributed relation "
    "from the second distributed relation. The third argument "
    "is a port number",
    "query drel1 drelfilter[.No > 5] drel1 dreldelete [1238]"
);

OperatorSpec drelupdateSpec( 
    " d[f]rel(rel(tuple(X))) x drel(rel(tuple(X))) x funlist x int"
    "-> dfrel(rel(tuple(X@[X1_old t1]@...[Xn_old tn]@[TID:tid])))",
    " _ _ drelupdate [_ ; _]",
    "Updates tuples in the distributed relation given " 
    "as second argument by replacing attributes as specified "
    "in the funlist. The first distributed relation contains "
    "tuples to update. ",
    "query drel1 drelfilter[.No > 5] drel1 drelupdate [No: .No + 1; 1238]" 
);

OperatorSpec dreladdidSpec( 
    " d[f]rel(rel(tuple(X))) -> dfrel(rel(tuple(x@[TID:tid])))", //check
    " _ dreladdid",
    "Appends tupleid attribute to each tuple of the "
    "distributed relation. Must be applied directly "
    "to the distributed relation, because other " 
    "operators may corrupt tupleids.", 
    "query drel1 dreladdid head[5] drel1 dreldeletebyid[TID, 1238]"
);

OperatorSpec dreldeletebyidSpec( 
    " d[f]rel(rel(tuple(X))) x drel(rel(tuple(Y))) x attr x int"
    "-> dfrel(rel(tuple(Y@[TID:tid])))",
    " _ _ dreldeletebyid [_ ]",
    "Deletes tuples from the distributed relation given "
    "as a second argument. The first distributed relation " 
    "contains tuples to delete and specifies tupleid "
    "by the attr argument.",
    "query drel1 dreladdid head[5] drel1 dreldeletebyid[TID, 1238]"
);

OperatorSpec drelupdatebyidSpec( 
    " d[f]rel(rel(tuple(X))) x drel(rel(tuple(Y))) x attr x funlist x int"
    "-> dfrel(rel(tuple(X)))",
    " _ _ drelupdatebyid [_ ; _; _]",
    "Updates tuples in the distributed relation "
    "given as a second argument. The first distributed relation " 
    "contains tuples to update and specifies tupleid "
    "by the attr argument. The functions may use parts " 
    "from the first drel (dot notation) or " 
    "from the second drel (double dot notation).",
    "query drel1 dreladdid drel1 drelupdatebyid [TID; No: ..No + 1; 1238]"
);

OperatorSpec drelinsertrtreeSpec(
    " d[f]rel(rel(tuple(X@[TID:tid]))) x da(rtree(tuple(X))) x attr x int"
    "-> dfrel(rel(tuple(X@[TID:tid])))",
    " _ _ drelinsertrtree [_, _]",
    "Inserts references of tuples with tupleids into the "
    "distributed rtree. ",
    "query drel da_rtree drelinsertrtree [GeoData, 1238]"
);

OperatorSpec dreldeletertreeSpec( 
    " d[f]rel(rel(tuple(X@[TID:tid]))) x da(rtree(tuple(X))) x attr x int"
    "-> dfrel(rel(tuple(X@[TID:tid])))",
    " _ _ dreldeletertree [_, _]",
    "Deletes references of tuples with tupleids from the "
    "distributed rtree. ",
    "query drel da_rtree dreldeletertree [GeoData, 1238]"
);

OperatorSpec drelupdatertreeSpec( 
    " d[f]rel(rel(tuple(X@[X1_old t1]@...[Xn_old tn]@[TID:tid]))) x"
    " da(rtree(tuple(X))) x attr x int "
    "-> dfrel(rel(tuple(X@[X1_old t1]@...[Xn_old tn]@[TID:tid])))",
    " _ _ drelupdatertree [_, _]",
    "Updates references of tuples with tupleids in the " 
    "distributed rtree.",
    "query drel da_rtree drelupdatertree [GeoData, 1238]"
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

Operator drelinserttupleOp (
    "drelinserttuple",                
    drelinserttupleSpec.getStr( ),         
    drelinserttupleVM,           
    Operator::SimpleSelect,     
    drelinserttupleTM           
); 

Operator dreldeleteOp (
    "dreldelete",                
    dreldeleteSpec.getStr( ),         
    drelupdateVM,           
    Operator::SimpleSelect,     
    dreldeleteTM           
);

Operator drelupdateOp (
    "drelupdate",                
    drelupdateSpec.getStr( ),         
    drelupdateVM,           
    Operator::SimpleSelect,     
    drelupdateTM           
); 

Operator dreladdidOp (
    "dreladdid",                
    dreladdidSpec.getStr( ),         
    dreladdidVM,           
    Operator::SimpleSelect,     
    dreladdidTM           
);

Operator dreldeletebyidOp (
    "dreldeletebyid",                
    dreldeletebyidSpec.getStr( ),         
    drelupdateVM,           
    Operator::SimpleSelect,     
    dreldeletebyidTM           
);

Operator drelupdatebyidOp (
    "drelupdatebyid",                
    drelupdatebyidSpec.getStr( ),         
    drelupdateVM,           
    Operator::SimpleSelect,     
    drelupdatebyidTM           
);

Operator drelinsertrtreeOp (
    "drelinsertrtree",                
    drelinsertrtreeSpec.getStr( ),         
    drelupdateVM,           
    Operator::SimpleSelect,     
    drelinsertrtreeTM           
);

Operator dreldeletertreeOp (
    "dreldeletertree",                
    dreldeletertreeSpec.getStr( ),         
    drelupdateVM,           
    Operator::SimpleSelect,     
    dreldeletertreeTM           
);

Operator drelupdatertreeOp (
    "drelupdatertree",                
    drelupdatertreeSpec.getStr( ),         
    drelupdateVM,           
    Operator::SimpleSelect,     
    drelupdatertreeTM           
);


class DistributedUpdateAlgebra : public Algebra{
    public:
        DistributedUpdateAlgebra() : Algebra(){
            AddOperator(&drelinsertOp);
            //drelinsertOp.SetUsesArgsInTypeMapping( );
            AddOperator(&dreldeleteOp);
            dreldeleteOp.SetUsesArgsInTypeMapping( );
            AddOperator(&drelupdateOp);
            drelupdateOp.SetUsesArgsInTypeMapping( );
            AddOperator(&dreladdidOp);
            dreladdidOp.SetUsesArgsInTypeMapping( );
            AddOperator(&drelupdatebyidOp);
            drelupdatebyidOp.SetUsesArgsInTypeMapping( );
            AddOperator(&dreldeletebyidOp);
            dreldeletebyidOp.SetUsesArgsInTypeMapping( );
            AddOperator(&drelinsertrtreeOp);
            drelinsertrtreeOp.SetUsesArgsInTypeMapping( );
            AddOperator(&dreldeletertreeOp);
            dreldeletertreeOp.SetUsesArgsInTypeMapping( );
            AddOperator(&drelupdatertreeOp);
            drelupdatertreeOp.SetUsesArgsInTypeMapping( );
            
            AddOperator(&drelinserttupleOp);
        }
};

    
} //end of namespace drelupdate

extern "C"
Algebra*
   InitializeDistributedUpdateAlgebra( NestedList* nlRef,
                                       QueryProcessor* qpRef ) {
   return new drelupdate::DistributedUpdateAlgebra ();
}
