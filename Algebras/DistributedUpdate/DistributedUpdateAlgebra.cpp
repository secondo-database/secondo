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

#include "Algebras/Relation-C++/OperatorFilter.h"

#include <string.h>
#include <iostream>

extern NestedList* nl;
extern QueryProcessor* qp;

template<int equalCheck>
ListExpr insertDeleteRelTypeMap(ListExpr args);
ListExpr insertTupleTypeMap(ListExpr args);

ListExpr updateSearchRelTypeMap(ListExpr args);
ListExpr updateDirectRelTypeMap(ListExpr args);

ListExpr appendIdentifierTypeMap (ListExpr args);

ListExpr allUpdatesRTreeTypeMap(ListExpr& args, string opName);
ListExpr allUpdatesBTreeTypeMap( const ListExpr& args, string opName);

template<int operatorId>
ListExpr IndexQuerySTypeMap(ListExpr args);
ListExpr WindowIntersectsSTypeMap(ListExpr args);

ListExpr deletebyid4TM(ListExpr args);
ListExpr updatebyid2TM(ListExpr args);

using namespace drel;
using namespace distributed2;


namespace drelupdate{
    
extern Operator drelspatialjoinOp;
    
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
    
    // create function for dmap2 operator
    string funText = "(fun (dmapelem1 ARRAYFUNARG1) (dmapelem2 ARRAYFUNARG2) "
                     "(insert (feed dmapelem1) dmapelem2)) ";
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(), 
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(), 
                                nl->Second(result) ), 
                            distType);

    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );
        
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
~distributeSimilarToDRel~

Distributes a relation in the same manner as a given drel. 

*/

DFRel* distributeSimilarToDRel( Relation* rel, ListExpr relType,
                                DRel* drel, ListExpr drelType){

    distributionType type = drel->getDistType()->getDistType();
    
    
    string relPtr = nl->ToString( DRelHelpers::createPointerList(
                                                        relType, rel) );
    string drelPtr = nl->ToString( DRelHelpers::createPointerList(
                nl->TwoElemList(
                    listutils::basicSymbol<distributed2::DArray>(),
                    nl->Second(drelType) ),
                drel) );

    string dtype = getName(type);
    
    string query2;
    if(type == replicated){
        query2 = "(drelfdistribute " + relPtr + "\"TMP_DATA\" " + 
                dtype + " (consume (getWorkers " + drelPtr + ")))";
    } 
    else if(type == drel::random){
        query2 = "(drelfdistribute " + relPtr + "\"TMP_DATA\" " + 
                dtype + " (size " + drelPtr + ") " +
                "(consume (getWorkers " + drelPtr + ")))";
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
            
        query2 = "(drelfdistribute " + relPtr + "\"TMP_DATA\" " +
                dtype + " " + attrName + " (size " + drelPtr + ") " + 
                "(consume (getWorkers " + drelPtr + ")))";
    }

    cout<< "query2: " << query2 << endl;
    
    DFRel* dfrel_tmp = 0;
    
    ListExpr query;
    if( !nl->ReadFromString(query2, query) ){
        dfrel_tmp->makeUndefined();
        return dfrel_tmp;
    }
    
    cout<< "query: " << nl->ToString(query) << endl;
    
    
    bool correct, evaluable, defined, isFunction;
    ListExpr resultType;
    
    OpTree tree = 0;
    qp->Construct(query, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qp->SetEvaluable(tree, true); 
    
    //DFRel* dfrel_tmp = 0;
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

bool removeTempObjects(DFRel* dfrel_tmp, ListExpr drelType){
    
    string dfrelPtr = nl->ToString(DRelHelpers::createPointerList(
                nl->TwoElemList(
                    listutils::basicSymbol<distributed2::DFArray>(),
                    nl->Second(drelType) ),
                dfrel_tmp ) ) ; 

    string query2 = "(deleteRemoteObjects " + dfrelPtr + ")";
    
    ListExpr query;
    if( !nl->ReadFromString(query2, query) ){
        return false;
    }

    bool correct, evaluable, defined, isFunction;
    ListExpr resultType;
    
    OpTree tree = 0;
    qp->Construct(query, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qp->SetEvaluable(tree, true); 
    
    if(!correct || !evaluable || !defined ){
        return false;
    } else {
        Word qRes;
        qp->EvalS(tree, qRes, OPEN);
        qp->Destroy(tree, false);
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
    
    DFRel* dfrel_tmp = distributeSimilarToDRel(rel, relType,
                                                drel, drelType);
    
    ListExpr arg1ptr = DRelHelpers::createPointerList(
            nl->TwoElemList(
                listutils::basicSymbol<distributed2::DFArray>(),
                nl->Second(drelType) ),
            dfrel_tmp );
    
    ListExpr arg2ptr = DRelHelpers::createPointerList(
            nl->TwoElemList(
                listutils::basicSymbol<distributed2::DArray>(),
                nl->Second(drelType) ),
            drel );
    
    string query2 = "(dmap2 " + nl->ToString(arg1ptr) + nl->ToString(arg2ptr) +
                    "\"\" " + funText->GetValue() + 
                    boost::to_string( port->GetValue() ) + ")"; 
                    
    ListExpr query;
    if( !nl->ReadFromString(query2, query)){
        resultDFRel->makeUndefined();
        return 0;
    }
    
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
    
    //delete temporary objects from remote server
    if( !removeTempObjects(dfrel_tmp, drelType) ){
        cout<<"temporary objects not deleted" << endl;
    }
        
    //check pointers
    delete dfrel_tmp;
    delete rel;
    //delete qps;
    return 0;
}

ListExpr drelinserttupleTM( ListExpr args ){
    
    //cout << "drelinserttupleTM" << endl;
    //cout << nl->ToString(args) << endl;
    
    if(!nl->HasLength(args, 3) ){
        return listutils::typeError("wrong number of arguments");
    }
    
    ListExpr drelType = nl->First( args );
    ListExpr tupleType = nl->Second( args );
    
    // check for correct types
    if(!DRel::checkType( drelType ) ){
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
    
    // create function for dmap2 operator, maybe to VM?
    string funText = "(fun (dmapelem1 ARRAYFUNARG1) (dmapelem2 ARRAYFUNARG2) "
                     "(insert (feed dmapelem1) dmapelem2)) ";
    
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
                            nl->TextAtom( funText ),
                            nl->TextAtom( nl->ToString(attrList) ) );
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

int drelinserttupleVM(Word* args, Word& result, int message,
           Word& local, Supplier s ){
   
    int argsNum = qp->GetNoSons( s );
    
    cout<< "argsNum: " << argsNum << endl;

    DRel* drel = (DRel*) args[0].addr;
    CcInt* port = (CcInt*) args[argsNum - 3].addr;
    FText* funText = (FText*) args[argsNum - 2].addr;
    FText* attrList = (FText*) args[argsNum - 1].addr;
    
    result = qp->ResultStorage(s);
    DFRel* resultDFRel = (DFRel*)result.addr;
    
    ListExpr attrs;
    if( !nl->ReadFromString(attrList->GetValue(), attrs ) ){
        resultDFRel->makeUndefined();
        return 0;
    }
    
    cout<< "attrs: " << nl->ToString( attrs ) << endl;
    
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
    
    
    while(nl->ListLength( rest ) > i){ // for s2d, s3d, re
        last = nl->Append(last, nl->First( rest ) );
        rest = nl->Rest( rest );
    }
    insertType = nl->TwoElemList(nl->First( resTupleType ),
                                 insertType);
        
    cout << "insrtType: " << nl->ToString ( insertType ) << endl;
    
    TupleType* insertTupleType = new TupleType( insertType );
    Tuple* insertTuple = new Tuple( insertTupleType );
    
    Supplier supplier = args[1].addr;
    Supplier supplier_i;
    Word attrValue;
    Attribute* attr; //check ptr
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
    
    ListExpr arg1ptr = DRelHelpers::createPointerList(
            nl->TwoElemList(
                listutils::basicSymbol<distributed2::DFArray>(),
                nl->Second(drelType) ),
            dfrel_tmp );
    
    ListExpr arg2ptr = DRelHelpers::createPointerList(
            nl->TwoElemList(
                listutils::basicSymbol<distributed2::DArray>(),
                nl->Second(drelType) ),
            drel );
    
    string query2 = "(dmap2 " + nl->ToString(arg1ptr) + nl->ToString(arg2ptr) +
                    "\"\" " + funText->GetValue() + 
                    boost::to_string( port->GetValue() ) + ")"; 
                    
    ListExpr query;
    if( !nl->ReadFromString(query2, query)){
        resultDFRel->makeUndefined();
        return 0;
    }
    
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
    
    //delete temporary objects from remote server 
    if( !removeTempObjects(dfrel_tmp, drelType) ){
        cout<<"temporary objects not deleted" << endl;
    }
    
    //check pointers
    delete dfrel_tmp;
    delete rel;
    //delete qps;
    return 0;
}

ListExpr dreldeleteTM ( ListExpr args ){
    
    cout << "dreldeleteTM" << endl;
    cout << nl->ToString( args ) << endl;

    if(!nl->HasLength(args, 3)){
        return listutils::typeError("wrong number of arguments");
    }
    //check for correct types
    ListExpr drelType1 = nl->First(args);
    if( !DRel::checkType( drelType1 ) && 
        !DFRel::checkType( drelType1 ) ){
        return listutils::typeError(
            "first argument is not a d[f]rel");
    }
    ListExpr drelType2 = nl->Second(args);
    if( !DRel::checkType( drelType2 ) ){
        return listutils::typeError(
            "second argument is not a drel");
    }
    
    //check for equality of distribution types
    
    ListExpr port = nl->Third(args);
    if( !CcInt::checkType( port ) ){
        return listutils::typeError(
            "last argument not of type int");
    }
    
    ListExpr result = insertDeleteRelTypeMap<0>(
                        nl->TwoElemList(
                            nl->TwoElemList(
                                listutils::basicSymbol<Stream<Tuple> >(),
                                nl->Second(nl->Second( drelType1 ) ) ),
                            nl->Second( drelType2 ) ) );
    
    //cout << nl->ToString(result) << endl;
    
    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    
    // create function type for dmap2 operator
    string funText = "(fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
                     "(deletesearch (feed elem1) elem2)) ";
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second(result) ),
                            nl->Third( drelType1 ) );  //disttype1 = distType2
    
    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}


ListExpr drelupdateTM ( ListExpr args ){
    
    cout << "drelupdateTM" << endl;
    cout << nl->ToString( args ) << endl;
    
    if(!nl->HasLength(args, 4)){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args) ){
        return listutils::typeError("internal Error");
    }
    // check for correct types
    ListExpr drelType1 = nl->First( nl->First(args) );
    if( !DRel::checkType( drelType1 ) && 
        !DFRel::checkType( drelType1 ) ){
        return listutils::typeError(
            "first argument is not a d[f]rel");
    }
    ListExpr drelType2 = nl->First( nl->Second(args) );
    if( !DRel::checkType( drelType2 ) ){
        return listutils::typeError(
            "second argument is not a drel");
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
    if( !CcInt::checkType(nl->First( port ) ) ){
        return listutils::typeError(
                "last argument not of type int");
    }
    
    ListExpr result = updateSearchRelTypeMap(
                            nl->ThreeElemList(
                                nl->TwoElemList(
                                    listutils::basicSymbol<Stream<Tuple> >(),
                                    nl->Second( nl->Second( drelType1 ) ) ),
                                nl->Second( drelType2 ),
                                mapList ) );
    
    //cout << nl->ToString(result) << endl;
    
    if( !nl->HasLength(result, 3) ){
        return result;
    }
    
    // create function for dmap2 operator
    string funText = "(fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
                     "(updatesearch (feed elem1) elem2 " + 
                       nl->ToString(funList_new) + ")) ";
                       
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second( nl->Third(result) ) ),
                            nl->Third( drelType1 ) ); //disttype1 = distType2
    
    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

ListExpr dreldeletebyidTM ( ListExpr args ){
    
    cout << "dreldeletebyid2TM" << endl;
    cout << nl->ToString( args ) << endl;
    
    if(!nl->HasLength(args, 4)){
        return listutils::typeError("wrong number of arguments");
    }
    // check for correct types
    ListExpr argType1 = nl->First(args);
    if( !DRel::checkType( argType1 ) && 
        !DFRel::checkType( argType1 ) &&
        !DArray::checkType( argType1 ) &&
        !DFArray::checkType( argType1 ) ){
        return listutils::typeError(
            "first argument is not a d[f]rel or a d[f]array");
    }
    ListExpr drelType2 = nl->Second(args);
    if( !DRel::checkType( drelType2 ) ){
        return listutils::typeError(
            "second argument is not a drel");
    }
    
    //check for equality of distribution types
    
    ListExpr attrName = nl->Third(args);
    if(!listutils::isSymbol( attrName ) ){
        return listutils::typeError(
            "third argument must be an attribute name" );
    }
    ListExpr port = nl->Fourth( args );
    if(!CcInt::checkType( port ) ){
        return listutils::typeError(
            "last argument not of type int");
    }

    ListExpr result = deletebyid4TM(
                        nl->ThreeElemList(
                            nl->TwoElemList(
                                listutils::basicSymbol<Stream<Tuple> >(),
                                nl->Second( nl->Second( argType1 ) ) ),
                            nl->Second( drelType2 ),
                            attrName ) ); //check TID

    //cout << nl->ToString(result) << endl;
    
    if( !nl->HasLength(result, 3) ){
        return result;
    }
    
    // create function for dmap2 operator
    string attr = nl->ToString(attrName);
    string funText = "(fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
                     "(deletebyid4 (feed elem1) elem2 " + attr + ")) ";
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second(nl->Third(result) ) ),
                            nl->Third( drelType2 ) );  //disttype1 = distType2
    
    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

ListExpr drelupdatebyidTM ( ListExpr args ){
    
    //cout << "drelupdate2TM" << endl;
    //cout << nl->ToString( args ) << endl;
    
    if(!nl->HasLength(args, 5)){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args) ){
        return listutils::typeError("internal Error");
    }
    // check for correct types
    ListExpr argType1 = nl->First( nl->First(args) );
    if( !DRel::checkType( argType1 ) && 
        !DFRel::checkType( argType1 ) &&
        !DArray::checkType( argType1 ) &&
        !DFArray::checkType( argType1 ) ){
        return listutils::typeError(
            "first argument is not a d[f]rel or a d[f]array");
    }
    ListExpr drelType2 = nl->First( nl->Second(args) );
    if( !DRel::checkType( drelType2 ) ){
        return listutils::typeError(
            "second argument is not a drel");
    }
    
    //check for equality of distribution types
    
    ListExpr attrName = nl->Second ( nl->Third( args ) );
    if(!listutils::isSymbol( attrName ) ){
        return listutils::typeError(
                "third argument must be an attribute name" );
    }
    
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
    
    ListExpr port = nl->Fifth( args );
    if(!CcInt::checkType( nl->First(port) ) ){
        return listutils::typeError(
                "last argument not of type int");
    }
    
    ListExpr result = updatebyid2TM(
                        nl->FourElemList(
                            nl->TwoElemList(
                                listutils::basicSymbol<Stream<Tuple> >(),
                                nl->Second( nl->Second(argType1) ) ),
                            nl->Second(drelType2),
                            attrName,
                            mapList ) );
    
    //cout << nl->ToString(result) << endl;
    
    if( !nl->HasLength(result, 3) ){
        return result;
    }
    
    // create function for dmap2 operator
    string attr = nl->ToString(attrName);
    string funText = "(fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
                     "(updatebyid2 (feed elem1) elem2 " + attr + " " +
                       nl->ToString(funList_new) + "))";
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second(nl->Third(result) ) ),
                            nl->Third( drelType2 ) ); //disttype1 = distType2
    
    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
dmap2VM

*/

template<class R>
int dreldmap2VMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){
    
    int argsNum = qp->GetNoSons( s );
    
    R* drel1 = (R*) args[0].addr;
    DRel* drel2 = (DRel*) args[1].addr;
    CcInt* port = (CcInt*) args[argsNum - 2].addr;
    FText* funText = (FText*) args[argsNum - 1].addr;
    
    result = qp->ResultStorage(s);
    DFRel* resultDFRel = (DFRel*)result.addr; //D[f]Rel? expected
    
    ListExpr arg1ptr;
    if( DRel::checkType( qp->GetType( qp->GetSon(s,0) ) ) ||
       DFRel::checkType( qp->GetType( qp->GetSon(s,0) ) ) ) {
       arg1ptr = DRelHelpers::createdrel2darray(
                    qp->GetType( qp->GetSon(s,0) ), drel1 );   
    } else {
       arg1ptr = DRelHelpers::createPointerList(
                    qp->GetType( qp->GetSon(s,0) ), drel1 );
    }
    ListExpr arg2ptr = DRelHelpers::createdrel2darray(
                        qp->GetType( qp->GetSon(s,1) ), drel2 );
    
    string query = "(dmap2 " + nl->ToString(arg1ptr) + nl->ToString(arg2ptr) +
                   "\"\" " + funText->GetValue() + 
                    boost::to_string( port->GetValue() ) + ")";
                    
    ListExpr queryAsList;
    if( !nl->ReadFromString(query, queryAsList)){
        resultDFRel->makeUndefined();
        return 0;
    }
    
    //cout<< nl->ToString(queryAsList) << endl;
    
    bool correct, evaluable, defined, isFunction;
    ListExpr resultType;
    
    OpTree tree = 0;
    qp->Construct(queryAsList, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qp->SetEvaluable(tree, true); 
    
    if(!correct || !evaluable || !defined ){
        resultDFRel->makeUndefined();
        return 0; //check
    } else {
        Word qRes;
        qp->EvalS(tree, qRes, OPEN);
        qp->Destroy(tree, false);
        resultDFRel = (DFRel*) qRes.addr;
        if(resultDFRel->IsDefined()){ // not a good idea
            resultDFRel->setDistType( drel2->getDistType()->copy() );
        }
        result.setAddr(resultDFRel);
    }
    
    return 0;
}

ValueMapping dreldmap2VM[] = { 
    dreldmap2VMT<DArray>,
    dreldmap2VMT<DFArray>
};

int dreldmap2UpdSelect( ListExpr args ){ 
    return DRel::checkType( nl->First( args ) ) ||
           DArray::checkType( nl->First( args ) ) ? 0 : 1;
}


ListExpr drelAllUpdatesRTreeTM (ListExpr& args, string opName ){
    
    //cout << "insertrtreeTM" << endl;
    //cout << nl->ToString( args ) << endl;
    
    if(!nl->HasLength(args, 4) ){
        return listutils::typeError("wrong number of arguments");
    }
    ListExpr drelType = nl->First(args);
    if( !DRel::checkType( drelType ) && 
        !DFRel::checkType( drelType ) ){
        return listutils::typeError(
                "first argument is not a d[f]rel");
    }
    ListExpr darrayRTreeType = nl->Second(args);
    if( !DArray::checkType( darrayRTreeType ) ){
        return listutils::typeError(
                "second argument is not a darray");
    }
    ListExpr attrName =  nl->Third(args);
    if(!listutils::isSymbol( attrName ) ){
        return listutils::typeError(
                "third argument must be an attribute name" );
    }
    ListExpr port = nl->Fourth(args);
    if(!CcInt::checkType( port ) ){
        return listutils::typeError(
                "last argument not of type int");
    }
    
    ListExpr args2 = nl->ThreeElemList(
                            nl->TwoElemList(
                                listutils::basicSymbol<Stream<Tuple> >(),
                                nl->Second(nl->Second( drelType ) ) ),
                            nl->Second( darrayRTreeType ),
                            attrName );
    
    ListExpr result = allUpdatesRTreeTypeMap(args2, opName);
        
    //cout << nl->ToString(result) << endl;
    
    if( !nl->HasLength(result, 3) ){
        return result;
    }

    //create function for dmap2 operator
    string funText = "(fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
                     "(" + opName + " (feed elem1) elem2 " + 
                      nl->ToString(attrName) + "))";

    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second(nl->Third(result) ) ),
                            nl->Third(drelType));
    
    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );

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
    return drelAllUpdatesRTreeTM(args, "updatertree");
}

ListExpr drelAllUpdatesBTreeTM (ListExpr& args, string opName ){
    
    cout << "btreeTM" << endl;
    cout << nl->ToString( args ) << endl;
    
    if(!nl->HasLength(args, 4) ){
        return listutils::typeError("wrong number of arguments");
    }
    // check for correct types
    ListExpr drelType = nl->First(args);
    if( !DRel::checkType( drelType ) && 
        !DFRel::checkType( drelType ) ){
        return listutils::typeError(
            "first argument is not a d[f]rel");
    }
    ListExpr darrayBTreeType = nl->Second( args );
    if( !DArray::checkType( darrayBTreeType ) ){
        return listutils::typeError(
            "second argument is not a darray");
    }
    ListExpr attrName = nl->Third( args );
    if( !listutils::isSymbol( attrName ) ){
        return listutils::typeError(
            "third argument must be an attribute name" );
    }
    ListExpr port = nl->Fourth(args);
    if(!CcInt::checkType( port ) ){
        return listutils::typeError(
                "last argument not of type int");
    }

    ListExpr args2 = nl->ThreeElemList(
                            nl->TwoElemList(
                                listutils::basicSymbol<Stream<Tuple> >(),
                                nl->Second(nl->Second( drelType ) ) ),
                            nl->Second( darrayBTreeType ),
                            attrName );
    
    
    ListExpr result = allUpdatesBTreeTypeMap(args2, opName);
    
    if( !nl->HasLength(result, 3) ){
        return result;
    }
    
    //create function for dmap2 operator
    string funText = "(fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
                     "(" + opName + " (feed elem1) elem2 " + 
                      nl->ToString(attrName) + "))";

    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(), 
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(), 
                                nl->Second(nl->Third(result) ) ), 
                            nl->Third( drelType ) );

    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );
        
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

ListExpr drelinsertbtreeTM(ListExpr args){
    return drelAllUpdatesBTreeTM(args, "insertbtree");
}

ListExpr dreldeletebtreeTM(ListExpr args){
    return drelAllUpdatesBTreeTM(args, "deletebtree");
}

ListExpr drelupdatebtreeTM(ListExpr args){
    return drelAllUpdatesBTreeTM(args, "updatebtree");
}

/*
dmap2VM for Index

*/

template<class R>
int dreldmap2IVMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){
    
    int argsNum = qp->GetNoSons( s );
    
    R* drel = (R*) args[0].addr;
    DArray* da = (DArray*) args[1].addr;
    CcInt* port = (CcInt*) args[argsNum - 2].addr;
    FText* funText = (FText*) args[argsNum - 1].addr;
    
    result = qp->ResultStorage(s);
    DFRel* resultDFRel = (DFRel*)result.addr; //D[f]Rel? expected
    
    ListExpr arg1ptr = DRelHelpers::createdrel2darray(
                        qp->GetType( qp->GetSon(s,0) ), drel );
    ListExpr arg2ptr = DRelHelpers::createPointerList(
                        qp->GetType( qp->GetSon(s,1) ), da );
    
    string query = "(dmap2 " + nl->ToString(arg1ptr) + nl->ToString(arg2ptr) +
                   "\"\" " + funText->GetValue() + 
                    boost::to_string( port->GetValue() ) + ")";
                    
    ListExpr queryAsList;
    if( !nl->ReadFromString(query, queryAsList)){
        resultDFRel->makeUndefined();
        return 0;
    }
    
    bool correct, evaluable, defined, isFunction;
    ListExpr resultType;
    
    OpTree tree = 0;
    qp->Construct(queryAsList, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qp->SetEvaluable(tree, true); 
    
    if(!correct || !evaluable || !defined ){
        resultDFRel->makeUndefined();
        return 0; //check
    } else {
        Word qRes;
        qp->EvalS(tree, qRes, OPEN);
        qp->Destroy(tree, false);
        resultDFRel = (DFRel*) qRes.addr;
        if(resultDFRel->IsDefined()){ // not a good idea
            resultDFRel->setDistType( drel->getDistType()->copy() );
        }
        result.setAddr(resultDFRel);
    }
    
    return 0;
}

ValueMapping dreldmap2IVM[] = {
    dreldmap2IVMT<DRel>,
    dreldmap2IVMT<DFRel>
};

int dreldmap2IUpdSelect( ListExpr args ){
    return DRel::checkType( nl->First( args ) ) ? 0 : 1;
}

ListExpr dreladdidTM ( ListExpr args ){

    //cout << "dreladdidTM" << endl;
    //cout << nl->ToString( args ) << endl;
    
    if(!nl->HasLength(args, 1) ){
        return listutils::typeError("wrong number of arguments");
    }
    ListExpr drelType = nl->First(args); //check
    if( !DRel::checkType( drelType ) && 
        !DFRel::checkType( drelType ) ){
        return listutils::typeError(
                "first argument is not a d[f]rel");
    }
    
    ListExpr result = appendIdentifierTypeMap(
                        nl->OneElemList(
                            nl->TwoElemList(
                                listutils::basicSymbol<Stream<Tuple> >(),
                                nl->Second( nl->Second(drelType) ) ) ) );
    
    //cout << nl->ToString(result) << endl;
    
    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    
    // create function for dmap operator
    string funText = "(fun (dmapelem1 ARRAYFUNARG1) (addid (feed dmapelem1)))";
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second( result ) ),
                            nl->Third( drelType ) ); //distType
    
    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );

    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

ListExpr drelfilteraddidTM( ListExpr args ){
    
    //cout << "drelfltrIDTM" << endl;
    //cout << nl->ToString( args ) << endl;
    
    //"drel(rel(X)) x fun"; //dfrel
    
    if(!nl->HasLength(args,2)){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args) ){
        return listutils::typeError("internal error");
    }
    // check for correct types
    ListExpr drelType, relType, distType, drelValue;
    if(!DRelHelpers::isDRelDescr(nl->First(args), drelType, relType,
        distType, drelValue) ){
        return listutils::typeError(
                "first argument is not a d[f]rel"); //d[f]rel
    }
    ListExpr fun, map;
    if( !DRelHelpers::replaceDRELFUNARG( nl->Second( args ),
        "STREAMELEM", fun, map) ){
        return listutils::typeError(
                "error in the function format");
    }

    ListExpr result = OperatorFilter::FilterTypeMap(
                nl->TwoElemList(
                    nl->TwoElemList(
                        nl->TwoElemList(
                            listutils::basicSymbol<Stream<Tuple> >(),
                            nl->Second(relType) ),
                        nl->TwoElemList(
                            nl->SymbolAtom("feed"), //dummy
                            drelValue ) ),
                    nl->TwoElemList( map, fun ) ) );
    
    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    
    ListExpr result2 = appendIdentifierTypeMap(nl->OneElemList( result ) );
    
    if(!listutils::isTupleStream( result2 ) ){
        return result2;
    }
    
    // create function for dmap operator
    string funText = "(fun (dmapelem1 ARRAYFUNARG1) "
            "(addid (filter (feed dmapelem1) " + nl->ToString(fun) + ")))";

    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(), 
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(), 
                                nl->Second(result2) ), 
                            distType);

    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );
        
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

ListExpr drelfilterdeleteTM( ListExpr args ){
    
    //cout << "drelfltrdelTM" << endl;
    //cout << nl->ToString( args ) << endl;
    
    //"drel(rel(X)) x fun"; //dfrel
    
    if(!nl->HasLength(args,2)){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args) ){
        return listutils::typeError("internal error");
    }
    // check for correct types
    ListExpr drelType, relType, distType, drelValue;
    if(!DRelHelpers::isDRelDescr(nl->First(args), drelType, relType,
        distType, drelValue) ){
        return listutils::typeError(
                "first argument is not a d[f]rel"); // dfrel!?
    }
    ListExpr fun, map;
    if( !DRelHelpers::replaceDRELFUNARG( nl->Second( args ),
        "STREAMELEM", fun, map) ){
        return listutils::typeError(
                "error in the function format");
    }

    ListExpr filterRes = OperatorFilter::FilterTypeMap(
                nl->TwoElemList(
                    nl->TwoElemList(
                        nl->TwoElemList(
                            listutils::basicSymbol<Stream<Tuple> >(),
                            nl->Second(relType) ),
                        nl->TwoElemList(
                            nl->SymbolAtom("feed"), //dummy
                            drelValue ) ),
                    nl->TwoElemList( map, fun ) ) );
    
    if(!listutils::isTupleStream( filterRes ) ){
        return filterRes;
    }

    ListExpr result = insertDeleteRelTypeMap<0>( nl->TwoElemList(
                                            filterRes, relType ) );

    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    
    // create function for dmap operator
    string funText = "(fun (elem1 ARRAYFUNARG1) (deletedirect "
        "(filter (feed elem1) " + nl->ToString(fun) + ") elem1))";
 
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(), 
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(), 
                                nl->Second( result ) ), 
                            distType);

    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );
        
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

ListExpr drelfilterupdateTM( ListExpr args ){
    
    //cout << "drelfltrupdTM" << endl;
    //cout << nl->ToString( args ) << endl;
    
    //"drel(rel(X)) x filter fun x update fun "; //dfrel
    
    if(!nl->HasLength(args,3)){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args) ){
        return listutils::typeError("internal error");
    }
    // check for correct types
    ListExpr drelType, relType, distType, drelValue;
    if(!DRelHelpers::isDRelDescr(nl->First(args), drelType, relType,
        distType, drelValue) ){
        return listutils::typeError(
                "first argument is not a d[f]rel"); // dfrel!?
    }
    ListExpr filter_fun, filter_map;
    if( !DRelHelpers::replaceDRELFUNARG( nl->Second( args ),
        "STREAMELEM", filter_fun, filter_map) ){
        return listutils::typeError(
                "error in the function format");
    }

    ListExpr filterRes = OperatorFilter::FilterTypeMap(
                nl->TwoElemList(
                    nl->TwoElemList(
                        nl->TwoElemList(
                            listutils::basicSymbol<Stream<Tuple> >(),
                            nl->Second(relType) ),
                        nl->TwoElemList(
                            nl->SymbolAtom("feed"), //dummy
                            drelValue ) ),
                    nl->TwoElemList( filter_map, filter_fun ) ) );
    
    if(!listutils::isTupleStream( filterRes ) ){
        return filterRes;
    }

    ListExpr update_mapList = nl->First( nl->Third(args) );
    ListExpr update_funList = nl->Second( nl->Third(args) );

    ListExpr tempfun;
    ListExpr funList_new = nl->TheEmptyList( );
    while( !nl->IsEmpty( update_funList ) ){
        
        if( !nl->HasLength( nl->First( update_funList ), 2 ) ){
            return listutils::typeError( "internal error" );
        }
        
        if( !DRelHelpers::replaceDRELFUNARG(
             nl->Second (nl->First( update_funList ) ), "TUPLE", tempfun ) ) {
            return listutils::typeError( "error in the function format" );
        }
        
        if( nl->IsEmpty( funList_new ) ){
            funList_new = nl->OneElemList( nl->TwoElemList(
                nl->First(nl->First( update_funList ) ), tempfun ) );
        } else {
            funList_new = listutils::concat( funList_new,
                        nl->OneElemList( nl->TwoElemList(
                        nl->First(nl->First( update_funList ) ), tempfun ) ) );
        }
        
        update_funList = nl->Rest( update_funList );
    }

    ListExpr result = updateDirectRelTypeMap( //updateSearchTM also possible
                            nl->ThreeElemList(
                                filterRes,
                                relType,
                                update_mapList ) );

    if( !nl->HasLength(result, 3) ){
        return result;
    }
    
    // create function for dmap operator
    string funText = "(fun (elem1 ARRAYFUNARG1) (updatedirect "
          "(filter (feed elem1) " + nl->ToString(filter_fun) + ") elem1 " + 
           nl->ToString(funList_new) + "))";
           
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(), 
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(), 
                                nl->Second( nl->Third(result) ) ), 
                            distType);

    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );
        
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
dmapVM

*/

template<class R>
int dreldmapVMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){
    
    int argsNum = qp->GetNoSons( s );

    R* drel = (R*) args[0].addr; 
    FText* funText = (FText*) args[argsNum - 1].addr;
    
    result = qp->ResultStorage(s);
    DFRel* resultDFRel = (DFRel*)result.addr;
    
    string drelptr = nl->ToString(DRelHelpers::createdrel2darray
                                (qp->GetType( qp->GetSon(s, 0) ), drel) );

    string query = "(dmap " + drelptr + "\"\" " + funText->GetValue() + ")";

    ListExpr queryAsList;
    if( !nl->ReadFromString(query, queryAsList)){
        resultDFRel->makeUndefined();
        return 0;
    }
    
    //cout<< nl->ToString(queryAsList) << endl;

    bool correct, evaluable, defined, isFunction;
    ListExpr resultType;
    
    OpTree tree = 0;
    qp->Construct(queryAsList, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qp->SetEvaluable(tree, true); 
    
    if(!correct || !evaluable || !defined ){
        resultDFRel->makeUndefined(); //setAddr
        return 0;
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

ValueMapping dreldmapVM[] = {
    dreldmapVMT<DRel>,
    dreldmapVMT<DFRel>
};

int dreldmapUpdSelect( ListExpr args ){
    return DRel::checkType( nl->First( args ) ) ? 0 : 1;
}

//RANGE 2, EXACTMATCH 3
template<int operatorId>
ListExpr drelindexquerySTM (ListExpr args){

    //nArgs 2/3
    if( !nl->HasLength( args, 2 ) && !nl->HasLength( args, 3 ) ){
        return listutils::typeError("wrong number of arguments");
    }
    //list of two elements
    ListExpr darrayBTreeType = nl->First( nl->First( args ) );
    if( !DArray::checkType( darrayBTreeType ) ){
        return listutils::typeError(
            "first argument is not a darray");
    }

    ListExpr args2;
    if( nl->HasLength( args, 2 ) ){
        args2 = nl->TwoElemList(
                    nl->Second( darrayBTreeType ),
                    nl->First( nl->Second( args ) ) );
    } else {
        args2 = nl->ThreeElemList(
                    nl->Second( darrayBTreeType ),
                    nl->First( nl->Second( args ) ),
                    nl->First( nl->Third( args ) ) );
    }
    
    ListExpr result = IndexQuerySTypeMap<operatorId>( args2 );
    
    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    
    ListExpr resType = nl->TwoElemList(
                            listutils::basicSymbol<DFArray>( ),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second( result ) ) );

    string funText;
    if(operatorId == 2){ 
        funText = "(fun (elem1 ARRAYFUNARG1) (rangeS elem1 " +
                nl->ToString( nl->Second( nl->Second( args ) ) ) + " " +
                nl->ToString( nl->Second( nl->Third( args ) ) ) + ")) ";
    } else {
        funText = "(fun (elem1 ARRAYFUNARG1) (exactmatchS elem1 " +
                nl->ToString( nl->Second( nl->Second( args ) ) ) + ")) ";
    }

    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );
        
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

ListExpr drelwindowintersectsSTM (ListExpr args){

    if( !nl->HasLength( args, 2 ) ){
        return listutils::typeError("wrong number of arguments");
    }
    //list of two elements
    ListExpr darrayRTreeType = nl->First( nl->First( args ) );
    if( !DArray::checkType( darrayRTreeType ) ){
        return listutils::typeError(
            "first argument is not a darray");
    }
    
    ListExpr result = WindowIntersectsSTypeMap( 
                            nl->TwoElemList(
                                nl->Second( darrayRTreeType ),
                                nl->First( nl->Second( args ) ) ) );

    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    
    ListExpr resType = nl->TwoElemList(
                            listutils::basicSymbol<DFArray>( ),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second( result ) ) );
    
    string funText = "(fun (elem1 ARRAYFUNARG1) (windowintersectsS elem1 " +
                nl->ToString( nl->Second( nl->Second( args ) ) ) + ")) ";
                
    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );
        
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

template<class A>
int dreldmapSVMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){
    
    int argsNum = qp->GetNoSons( s );

    A* darray = (A*) args[0].addr; 
    FText* funText = (FText*) args[argsNum - 1].addr;
    
    result = qp->ResultStorage(s);
    DFArray* resultDFArray = (DFArray*)result.addr;
    
    string arg1ptr = nl->ToString(DRelHelpers::createPointerList
                                (qp->GetType( qp->GetSon(s, 0) ), darray) );

    string query = "(dmap " + arg1ptr + "\"\" " + funText->GetValue() + ")";

    ListExpr queryAsList;
    if( !nl->ReadFromString(query, queryAsList)){
        resultDFArray->makeUndefined();
        return 0;
    }
    
    bool correct, evaluable, defined, isFunction;
    ListExpr resultType;
    
    OpTree tree = 0;
    qp->Construct(queryAsList, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qp->SetEvaluable(tree, true); 
    
    if(!correct || !evaluable || !defined ){
        resultDFArray->makeUndefined(); //setAddr
        return 0;
    } else {
        Word qRes;
        qp->EvalS(tree, qRes, OPEN);
        qp->Destroy(tree, false);
        resultDFArray = (DFArray*) qRes.addr; //check for defined
        result.setAddr(resultDFArray);
    }
    
    return 0;
}

ValueMapping dreldmapSVM[] = {
    dreldmapSVMT<DArray>,
    dreldmapSVMT<DFArray>
};

int dreldmapSUpdSelect( ListExpr args ){
    return DArray::checkType( nl->First( args ) ) ? 0 : 1;
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

OperatorSpec drelinsertbtreeSpec( //btree struct
    " d[f]rel(rel(tuple(X@[TID:tid]))) x da(btree(tuple(X))) x attr x int"
    "-> dfrel(rel(tuple(X@[TID:tid])))",
    " _ _ drelinsertbtree [_, _]",
    "Inserts references of tuples with tupleids into the "
    "distributed btree. ",
    "query drel da_btree drelinsertbtree [BevT, 1238]"
);

OperatorSpec dreldeletebtreeSpec( //btree struct
    " d[f]rel(rel(tuple(X@[TID:tid]))) x da(btree(tuple(X))) x attr x int"
    "-> dfrel(rel(tuple(X@[TID:tid])))",
    " _ _ dreldeletebtree [_, _]",
    "Deletes references of tuples with tupleids from the "
    "distributed btree. ",
    "query drel da_btree dreldeletebtree [BevT, 1238]"
);

OperatorSpec drelupdatebtreeSpec( //btree struct
    " d[f]rel(rel(tuple(X@[X1_old t1]@...[Xn_old tn]@[TID:tid]))) x"
    " da(btree(tuple(X))) x attr x int "
    "-> dfrel(rel(tuple(X@[X1_old t1]@...[Xn_old tn]@[TID:tid])))",
    " _ _ drelupdatebtree [_, _]",
    "Updates references of tuples with tupleids in the " 
    "distributed btree.",
    "query drel da_btree drelupdatebtree [BevT, 1238]"
);

OperatorSpec dreladdidSpec( // check
    " d[f]rel(rel(tuple(X))) -> dfrel(rel(tuple(x@[TID:tid])))", 
    " _ dreladdid",
    "Appends tupleid attribute to each tuple of the "
    "distributed relation. Must be applied directly "
    "to the distributed relation, because other " 
    "operators may corrupt tupleids.", 
    "query drel1 dreladdid head[5] drel1 dreldeletebyid[TID, 1238]"
);

OperatorSpec drelfilteraddidSpec( //check
    " d[f]rel(rel(tuple(X))) x fun -> dfrel(rel(tuple(x@[TID:tid])))", 
    " _ drelfilteraddid [_]",
    "Only tuples, fulfilling a certain condition are selected, "
    "extended with the TID attribute and passed to the output "
    "distributed relation.", 
    "query drel1 drelfilteraddid [.No > 5] dreldeletebyid[TID, 1238]"
);

OperatorSpec drelfilterdeleteSpec( 
    "d[f]rel(rel(tuple(X))) x fun -> dfrel(rel(tuple(x@[TID:tid])))", 
    " _ drelfilterdelete [_]",
    "Only tuples, fulfilling a certain condition are selected, "
    "extended with the TID attribute and deleted from the "
    "distributed relation.", 
    "query drel1 drelfilterdelete[.No > 5]"
);

OperatorSpec drelfilterupdateSpec( 
    " d[f]rel(rel(tuple(X))) x fun x funlist -> "
    " dfrel(rel(tuple(X@[X1_old t1]@...[Xn_old tn]@[TID:tid])))", 
    " _ drelfilterupdate [filter_fun; update_funlist]",
    "Only tuples, fulfilling a certain condition (second argument) "
    "are selected and updated by replacing attributes as specified "
    "in the funlist (third argument)", 
    "query drel1 drelfilterupdate[.No > 5; No: .No + 1]"
);

OperatorSpec drelexactmatchSSpec( //btree struct
    " da(btree(tuple(X))) x ti -> dfarray(rel(tuple(Id tid)))",
    " _ drelexactmatchS [_]",
    "Uses the given distributed btree to find all tuple "
    "identifiers where the key matches argument value ti. ",
    "query da_btree drelexactmatchS [10]"
);

OperatorSpec drelrangeSSpec( //btree struct
    " da(btree(tuple(X))) x ti x ti-> dfarray(rel(tuple(Id tid)))",
    " _ drelrangeS [_]",
    "Uses the given distributed btree to find all tuple "
    "identifiers where the key is between argument values ti. ",
    "query da_btree drelrangeS [5, 10]"
);

OperatorSpec drelwindowintersectsSSpec( //rtree struct
    " da(rtree(tuple(X))) x ti -> dfarray(rel(tuple(Id tid)))",
    " _ drelwindowintersectsS [_]",
    "Uses the given distributed rtree to find all tuple "
    "identifiers whose bounding box intersects with "
    "ti bounding box. The argument value ti can be in "
    "{rect<d>} or SPATIAL<d>D, where d in {2, 3, 4, 8}.",
    "query da_rtree drelwindowintersectsS [ bbox(thecenter) ]"
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
    2,
    dreldmap2VM,           
    dreldmap2UpdSelect,      
    dreldeleteTM           
);

Operator drelupdateOp (
    "drelupdate",                
    drelupdateSpec.getStr( ),         
    2,
    dreldmap2VM,           
    dreldmap2UpdSelect,      
    drelupdateTM           
); 

Operator dreldeletebyidOp (
    "dreldeletebyid",                
    dreldeletebyidSpec.getStr( ),         
    2,
    dreldmap2VM,           
    dreldmap2UpdSelect,     
    dreldeletebyidTM           
);

Operator drelupdatebyidOp (
    "drelupdatebyid",                
    drelupdatebyidSpec.getStr( ),         
    2,
    dreldmap2VM,           
    dreldmap2UpdSelect,     
    drelupdatebyidTM           
);

Operator drelinsertrtreeOp (
    "drelinsertrtree",                
    drelinsertrtreeSpec.getStr( ),         
    2,
    dreldmap2IVM,           
    dreldmap2IUpdSelect,   
    drelinsertrtreeTM           
);

Operator dreldeletertreeOp (
    "dreldeletertree",                
    dreldeletertreeSpec.getStr( ),         
    2,
    dreldmap2IVM,           
    dreldmap2IUpdSelect,    
    dreldeletertreeTM           
);

Operator drelupdatertreeOp (
    "drelupdatertree",                
    drelupdatertreeSpec.getStr( ),         
    2,
    dreldmap2IVM,           
    dreldmap2IUpdSelect,      
    drelupdatertreeTM           
);

Operator drelinsertbtreeOp (
    "drelinsertbtree",                
    drelinsertbtreeSpec.getStr( ),         
    2,
    dreldmap2IVM,           
    dreldmap2IUpdSelect,    
    drelinsertbtreeTM           
);

Operator dreldeletebtreeOp (
    "dreldeletebtree",                
    dreldeletebtreeSpec.getStr( ),         
    2,
    dreldmap2IVM,           
    dreldmap2IUpdSelect,    
    dreldeletebtreeTM           
);

Operator drelupdatebtreeOp (
    "drelupdatebtree",                
    drelupdatebtreeSpec.getStr( ),
    2,
    dreldmap2IVM,           
    dreldmap2IUpdSelect,     
    drelupdatebtreeTM           
);

Operator dreladdidOp (
    "dreladdid",                
    dreladdidSpec.getStr( ),
    2,
    dreldmapVM,           
    dreldmapUpdSelect,     
    dreladdidTM           
);

Operator drelfilteraddidOp (
    "drelfilteraddid",                
    drelfilteraddidSpec.getStr( ),
    2,
    dreldmapVM,           
    dreldmapUpdSelect,     
    drelfilteraddidTM           
);

Operator drelfilterdeleteOp (
    "drelfilterdelete",                
    drelfilterdeleteSpec.getStr( ),
    2,
    dreldmapVM,           
    dreldmapUpdSelect,     
    drelfilterdeleteTM           
);

Operator drelfilterupdateOp (
    "drelfilterupdate",                
    drelfilterupdateSpec.getStr( ),
    2,
    dreldmapVM,           
    dreldmapUpdSelect,     
    drelfilterupdateTM           
);

Operator drelexactmatchSOp (
    "drelexactmatchS",                
    drelexactmatchSSpec.getStr( ),
    2,
    dreldmapSVM,           
    dreldmapSUpdSelect,     
    drelindexquerySTM<3>           
);

Operator drelrangeSOp (
    "drelrangeS",                
    drelrangeSSpec.getStr( ),
    2,
    dreldmapSVM,           
    dreldmapSUpdSelect,     
    drelindexquerySTM<2>             
);

Operator drelwindowintersectsSOp (
    "drelwindowintersectsS",                
    drelwindowintersectsSSpec.getStr( ),
    2,
    dreldmapSVM,           
    dreldmapSUpdSelect,     
    drelwindowintersectsSTM           
);

class DistributedUpdateAlgebra : public Algebra{
    public:
        DistributedUpdateAlgebra() : Algebra(){
            AddOperator(&drelinsertOp);
            AddOperator(&drelinserttupleOp);

            AddOperator(&dreldeleteOp);
            AddOperator(&drelupdateOp);
            drelupdateOp.SetUsesArgsInTypeMapping( );

            AddOperator(&dreldeletebyidOp);
            AddOperator(&drelupdatebyidOp);
            drelupdatebyidOp.SetUsesArgsInTypeMapping( );

            AddOperator(&drelinsertrtreeOp);
            AddOperator(&dreldeletertreeOp);
            AddOperator(&drelupdatertreeOp);

            AddOperator(&drelinsertbtreeOp);
            AddOperator(&dreldeletebtreeOp);
            AddOperator(&drelupdatebtreeOp);

            AddOperator(&dreladdidOp);
            AddOperator(&drelfilteraddidOp);
            drelfilteraddidOp.SetUsesArgsInTypeMapping( );
            AddOperator(&drelfilterdeleteOp);
            drelfilterdeleteOp.SetUsesArgsInTypeMapping( );
            AddOperator(&drelfilterupdateOp);
            drelfilterupdateOp.SetUsesArgsInTypeMapping( );
            
            AddOperator(&drelexactmatchSOp);
            drelexactmatchSOp.SetUsesArgsInTypeMapping( );
            AddOperator(&drelrangeSOp);
            drelrangeSOp.SetUsesArgsInTypeMapping( );
            AddOperator(&drelwindowintersectsSOp);
            drelwindowintersectsSOp.SetUsesArgsInTypeMapping( );

            AddOperator(&drelspatialjoinOp);
            drelspatialjoinOp.SetUsesArgsInTypeMapping( );
        }
};

    
} //end of namespace drelupdate

extern "C"
Algebra*
   InitializeDistributedUpdateAlgebra( NestedList* nlRef,
                                       QueryProcessor* qpRef ) {
   return new drelupdate::DistributedUpdateAlgebra ();
}
