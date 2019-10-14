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
#include "Stream.h"
#include "Algebras/DRel/DRel.h"
#include "Algebras/DRel/DRelHelpers.h"
#include "Algebras/Distributed2/DArray.h"
#include "Algebras/FText/FTextAlgebra.h"

#include "Algebras/Relation-C++/OperatorFilter.h"

#include <string>
#include <iostream>

extern NestedList* nl;
extern QueryProcessor* qp;

template<int equalCheck>
ListExpr insertDeleteRelTypeMap(ListExpr args);
ListExpr insertTupleTypeMap(ListExpr args);

ListExpr updateSearchRelTypeMap(ListExpr args);
ListExpr updateDirectRelTypeMap(ListExpr args);

ListExpr appendIdentifierTypeMap (ListExpr args);

ListExpr allUpdatesRTreeTypeMap(ListExpr& args, std::string opName);
ListExpr allUpdatesBTreeTypeMap( const ListExpr& args, std::string opName);

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
1. Operator ~drelinsert~

1.1 Type Mapping ~drelinsert~

*/

ListExpr drelinsertTM( ListExpr args ){
    
    std::string err = "stream(tuple(X)) x drel(rel(tuple(X))) x int " 
                      "expected";
    
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
    distributionType type;
    int attr, key;
    if(!DRel::checkType( drelType, type, attr, key ) ){
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
    if( type == replicated ){
        attrList = ConcatLists( 
                            attrList,
                            nl->OneElemList(nl->TwoElemList(
                                nl->SymbolAtom("Original"),
                                listutils::basicSymbol<CcBool>() ) ) );
    } else if ( type == spatial2d || type == spatial3d ) {
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
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(), 
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(), 
                                nl->Second(result) ), 
                            nl->Third(drelType) ); //distType
    
    return resType;
}

/*
~distributeSimilarToDRel~

Distributes a relation in the same manner as a given drel. 

*/

bool distributeSimilarToDRel( Relation* rel, ListExpr relType, DRel* drel,
                              ListExpr drelType, DFRel*& dfrel_tmp ){
    
    std::string relPtr = nl->ToString( DRelHelpers::createPointerList(
                                                      relType, rel) );
    std::string drelPtr = nl->ToString( DRelHelpers::createdrel2darray(
                                                        drelType, drel) );
    
    distributionType type = drel->getDistType()->getDistType();
    std::string dtype = getName(type);

    std::string query;
    if(type == replicated){
        query = "(drelfdistribute " + relPtr + "\"TMP_DATA\" " + 
                dtype + " (consume (getWorkers " + drelPtr + ")))";
    } 
    else if(type == drel::random){
        query = "(drelfdistribute " + relPtr + "\"TMP_DATA\" " + 
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
        std::string attr = nl->ToString( nl->First(nl->First(attrList) ) );
            
        query = "(drelfdistribute " + relPtr + "\"TMP_DATA\" " +
                dtype + " " + attr + " (size " + drelPtr + ") " + 
                "(consume (getWorkers " + drelPtr + ")))";
    }
    
    Word qRes;
    if( !QueryProcessor::ExecuteQuery( query, qRes ) ) {
        cout << "distribution of a stream to d[f]rel failed" << endl;
        return false;
    } else {
        dfrel_tmp = (DFRel*) qRes.addr;
    }

    if( !dfrel_tmp || !dfrel_tmp->IsDefined() ){
        return false;
    }
    
    return true;
}

/*
~removeTempObjects~

Removes temporary objects from a given dfrel.

*/

bool removeTempObjects(DFRel* dfrel_tmp, ListExpr drelType){
    
    std::string dfrelPtr = nl->ToString(DRelHelpers::createPointerList(
                nl->TwoElemList(
                    listutils::basicSymbol<distributed2::DFArray>(),
                    nl->Second(drelType) ),
                dfrel_tmp ) ) ; 

    std::string query = "(deleteRemoteObjects " + dfrelPtr + ")";
    
    Word qRes;
    if( !QueryProcessor::ExecuteQuery( query, qRes ) ) {
        return false;
    } else {
        int deletedObjects = ((CcInt*) qRes.addr)->GetValue();
        return deletedObjects > 0 ? true : false;
    }
}

/*
1.2 Value Mapping ~drelinsert~

*/

int drelinsertVM(Word* args, Word& result, int message,
           Word& local, Supplier s ){
    
    Stream<Tuple> stream(args[0].addr);
    DRel* drel = (DRel*) args[1].addr; 
    CcInt* port = (CcInt*) args[2].addr;
    
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
    
    //distribute relation similar to drel
    ListExpr relType = nl->TwoElemList(
                        listutils::basicSymbol<Relation> (),
                        nl->Second( qp->GetType( qp->GetSon(s,0) ) ) );
    ListExpr drelType = qp->GetType( qp->GetSon(s,1) );
    
    DFRel* dfrel_tmp = 0;
    if( ! distributeSimilarToDRel(rel, relType, drel, 
                                      drelType, dfrel_tmp) ){
        resultDFRel->makeUndefined();
        delete dfrel_tmp;
        delete rel;
        return 0;
    }

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
    
    std::string query = "(dmap2 " + nl->ToString(arg1ptr) + 
                                    nl->ToString(arg2ptr) + "\"\" "
                         "(fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
                         "(insert (feed elem1) elem2)) " + 
                         boost::to_string( port->GetValue() ) + ")"; 
                    
    Word qRes;
    if( !QueryProcessor::ExecuteQuery( query, qRes ) ) {
        resultDFRel->makeUndefined();
    } else {
        DFArray* dfarray = (DFArray*) qRes.addr;
        if( !dfarray || !dfarray->IsDefined()){
            resultDFRel->makeUndefined();
        } else {
            resultDFRel->copyFrom( *dfarray );
            resultDFRel->setDistType( drel->getDistType()->copy() );
        }
        delete dfarray;
    }

    //delete temporary objects from remote server
    if( !removeTempObjects(dfrel_tmp, drelType) ){
        cout<<"temporary objects not deleted" << endl;
    }
        
    //check pointers
    delete dfrel_tmp;
    delete rel;
    return 0;
}

/*
1.3 Specification of ~drelinsert~

*/

OperatorSpec drelinsertSpec(
    " stream(tuple(X)) x drel(rel(tuple(X))) x int "
    "-> dfrel(rel(tuple(X@[TID:tid])))",
    " _ _ drelinsert [_]",
    "Inserts tuples of the stream into the distributed relation. "
    "The tuples to insert should not contain the partition " 
    "attributes (Cell, Original). The third argument specifies "
    "a port number.",
    "query rel1 feed drel1 drelinsert [1238]"
);

/*
1.4 Definition of ~drelinsert~

*/

Operator drelinsertOp (
    "drelinsert",                
    drelinsertSpec.getStr( ),         
    drelinsertVM,           
    Operator::SimpleSelect,     
    drelinsertTM           
);

/*
2. Operator ~drelinserttuple~

2.1 Type Mapping ~drelinserttuple~ 

*/

ListExpr drelinserttupleTM( ListExpr args ){
    
    if(!nl->HasLength(args, 3) ){
        return listutils::typeError("wrong number of arguments");
    }
    
    ListExpr drelType = nl->First( args );
    ListExpr attrList = nl->Second( args );
    
    // check for correct types
    distributionType type;
    int attr, key;
    if(!DRel::checkType( drelType, type, attr, key ) ){
        return listutils::typeError(
                "first argument is not a drel");
    }
    if(nl->AtomType( attrList ) != NoAtom){ 
        return listutils::typeError(
                "second argument must be a list of attributes");
    }
    if(!CcInt::checkType( nl->Third( args ) ) ){
        return listutils::typeError(
                "last argument not of type int");
    }
    
    // attrList != relType (spatial2d, 3d, replicated) 
    if( type == replicated ) {
        attrList = ConcatLists( attrList, nl->OneElemList(
                                 listutils::basicSymbol<CcBool>() ) );
    } else if ( type == spatial2d || type == spatial3d ) {
        attrList = ConcatLists( attrList, nl->TwoElemList(
                                listutils::basicSymbol<CcInt>(),
                                listutils::basicSymbol<CcBool>() ) );
    }
    
    ListExpr result = insertTupleTypeMap( nl->TwoElemList(
                                                nl->Second( drelType ),
                                                attrList ) ); 

    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second( result ) ),
                            nl->Third( drelType ) ); //distType

    return resType;
}

/*
2.2 Value Mapping ~drelinserttuple~ 

*/

int drelinserttupleVM(Word* args, Word& result, int message,
           Word& local, Supplier s ){  
    
    DRel* drel = (DRel*) args[0].addr;
    CcInt* port = (CcInt*) args[2].addr;
    
    result = qp->ResultStorage(s);
    DFRel* resultDFRel = (DFRel*)result.addr;
    
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
    
    while(nl->ListLength( rest ) > i){ 
        last = nl->Append(last, nl->First( rest ) );
        rest = nl->Rest( rest );
    }
    insertType = nl->TwoElemList(nl->First( resTupleType ),
                                 insertType);
    
    //create a new tuple from the attribute list
    Tuple* insertTuple = new Tuple( new TupleType( insertType ) );
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
    
    Relation* rel = new Relation( insertTuple->GetTupleType() );
    rel->AppendTuple( insertTuple );
    insertTuple->DeleteIfAllowed();

    //distribute relation similar to drel
    ListExpr drelType = qp->GetType( qp->GetSon(s,0) );
    ListExpr attrs = DRelHelpers::removePartitionAttributes(
                    nl->Second( nl->Second( nl->Second( drelType ) ) ),
                    type );
    ListExpr relType = nl->TwoElemList(
                            listutils::basicSymbol<Relation> (),
                            nl->TwoElemList(
                                listutils::basicSymbol<Tuple>(),
                                attrs ) );
    
    DFRel* dfrel_tmp = 0;
    if( ! distributeSimilarToDRel(rel, relType, drel, drelType, 
                                                      dfrel_tmp) ){
        resultDFRel->makeUndefined();
        delete dfrel_tmp;
        delete rel;
        return 0;
    }
    
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
    
    std::string query = "(dmap2 " + nl->ToString(arg1ptr) + 
                                    nl->ToString(arg2ptr) + "\"\" "
                         "(fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
                         "(insert (feed elem1) elem2)) " + 
                         boost::to_string( port->GetValue() ) + ")"; 
    
    Word qRes;
    if( !QueryProcessor::ExecuteQuery( query, qRes ) ) {
        resultDFRel->makeUndefined();
    } else {
        DFArray* dfarray = (DFArray*) qRes.addr;
        if( !dfarray || !dfarray->IsDefined()){
            resultDFRel->makeUndefined();
        } else {
            resultDFRel->copyFrom( *dfarray );
            resultDFRel->setDistType( drel->getDistType()->copy() );
        }
        delete dfarray;
    }

    //delete temporary objects from remote server
    if( !removeTempObjects(dfrel_tmp, drelType) ){
        cout<<"temporary objects not deleted" << endl;
    }                    
    
    //check pointers
    delete dfrel_tmp;
    delete rel;
    return 0;
}

/*
2.3 Specification of ~drelinserttuple~

*/

OperatorSpec drelinserttupleSpec(
    " drel(rel(tuple(X))) x [t1 ... tn] x int"
    "-> dfrel(rel(tuple(X@[TID:tid])))",
    " _ drelinserttuple [_; _]",
    "Inserts a new tuple with the values from "
    "the second argument list into the distributed relation. "
    "The third argument specifies a port number. ",
    "query drel1 drelinserttuple [\"Z\", 10 ; 1238]"
);

/*
2.4 Definition of ~drelinserttuple~

*/

Operator drelinserttupleOp (
    "drelinserttuple",                
    drelinserttupleSpec.getStr( ),         
    drelinserttupleVM,           
    Operator::SimpleSelect,     
    drelinserttupleTM           
);

/*
3. Operators ~dreldelete~, ~drelupdate~, ~dreldeletebyid~,
~drelupdatebyid~

3.1 Type Mapping functions

3.1.1 Type Mapping of ~dreldelete~ 

*/

ListExpr dreldeleteTM ( ListExpr args ){

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
    
    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    
    // create function type for dmap2 operator
    std::string fun = "(fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
                      "(deletesearch (feed elem1) elem2)) ";
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second( result ) ),
                            nl->Third( drelType2 ) ); 
    
    ListExpr append = nl->OneElemList( nl->TextAtom( fun ) );
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
3.1.2 Type Mapping of ~drelupdate~ 

*/

ListExpr drelupdateTM ( ListExpr args ){
    
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
    
    if( !nl->HasLength(result, 3) ){
        return result;
    }
    
    // create function for dmap2 operator
    std::string fun = "(fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
                      "(updatesearch (feed elem1) elem2 " + 
                       nl->ToString(funList_new) + ")) ";
                       
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second( nl->Third( result ) ) ),
                            nl->Third( drelType2 ) ); 
    
    ListExpr append = nl->OneElemList( nl->TextAtom( fun ) );
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
3.1.3 Type Mapping of ~dreldeletebyid~ 

*/

ListExpr dreldeletebyidTM ( ListExpr args ){
    
    if(!nl->HasLength(args, 4)){
        return listutils::typeError("wrong number of arguments");
    }
    // check for correct types
    ListExpr arg1Type = nl->First(args);
    if( !DRel::checkType( arg1Type ) && 
        !DFRel::checkType( arg1Type ) &&
        !DArray::checkType( arg1Type ) &&
        !DFArray::checkType( arg1Type ) ){
        return listutils::typeError(
            "first argument is not a d[f]rel or a d[f]array");
    }
    if( !Relation::checkType( nl->Second( arg1Type ) ) ){
        return listutils::typeError(
            "wrong subtype of the first argument");
    }
    ListExpr drelType2 = nl->Second(args);
    if( !DRel::checkType( drelType2 ) ){
        return listutils::typeError(
            "second argument is not a drel");
    }
    
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
                                nl->Second( nl->Second( arg1Type ) ) ),
                            nl->Second( drelType2 ),
                            attrName ) ); 
    
    if( !nl->HasLength(result, 3) ){
        return result;
    }
    
    // create function for dmap2 operator
    std::string attr = nl->ToString(attrName);
    std::string fun = "(fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
                      "(deletebyid4 (feed elem1) elem2 " + attr + ")) ";
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second( nl->Third( result) ) ),
                            nl->Third( drelType2 ) );
    
    ListExpr append = nl->OneElemList( nl->TextAtom( fun ) );
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
3.1.4 Type Mapping of ~drelupdatebyid~ 

*/

ListExpr drelupdatebyidTM ( ListExpr args ){
    
    if(!nl->HasLength(args, 5)){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args) ){
        return listutils::typeError("internal Error");
    }
    // check for correct types
    ListExpr arg1Type = nl->First( nl->First(args) );
    if( !DRel::checkType( arg1Type ) && 
        !DFRel::checkType( arg1Type ) &&
        !DArray::checkType( arg1Type ) &&
        !DFArray::checkType( arg1Type ) ){
        return listutils::typeError(
            "first argument is not a d[f]rel or a d[f]array");
    }
    if( !Relation::checkType( nl->Second( arg1Type ) ) ){
        return listutils::typeError(
            "wrong subtype of the first argument");
    }
    ListExpr drelType2 = nl->First( nl->Second(args) );
    if( !DRel::checkType( drelType2 ) ){
        return listutils::typeError(
            "second argument is not a drel");
    }
    
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
                                nl->Second( nl->Second( arg1Type ) ) ),
                            nl->Second( drelType2 ),
                            attrName,
                            mapList ) );
    
    if( !nl->HasLength(result, 3) ){
        return result;
    }
    
    // create function for dmap2 operator
    std::string attr = nl->ToString(attrName);
    std::string fun = "(fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
                      "(updatebyid2 (feed elem1) elem2 " + attr + " " +
                       nl->ToString(funList_new) + "))";
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second( nl->Third(result) ) ),
                            nl->Third( drelType2 ) );
    
    ListExpr append = nl->OneElemList( nl->TextAtom( fun ) );
    
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
3.2 Value Mapping function

used by ~dreldelete~, ~drelupdate~, ~dreldeletebyid~, ~drelupdatebyid~ 
operators

*/

template<class R>
int dreldmap2VMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){
    
    int argsNum = qp->GetNoSons( s );
    
    R* arg1 = (R*) args[0].addr;
    DRel* drel2 = (DRel*) args[1].addr;
    CcInt* port = (CcInt*) args[argsNum - 2].addr;
    FText* funText = (FText*) args[argsNum - 1].addr;
    
    result = qp->ResultStorage(s);
    DFRel* resultDFRel = (DFRel*)result.addr;
    
    ListExpr arg1ptr;
    if( DRel::checkType( qp->GetType( qp->GetSon(s,0) ) ) ||
       DFRel::checkType( qp->GetType( qp->GetSon(s,0) ) ) ) {
       arg1ptr = DRelHelpers::createdrel2darray(
                    qp->GetType( qp->GetSon(s,0) ), arg1 );   
    } else {
       arg1ptr = DRelHelpers::createPointerList(
                    qp->GetType( qp->GetSon(s,0) ), arg1 );
    }
    ListExpr arg2ptr = DRelHelpers::createdrel2darray(
                        qp->GetType( qp->GetSon(s,1) ), drel2 );
    
    std::string query = "(dmap2 " + nl->ToString(arg1ptr) + 
                                    nl->ToString(arg2ptr) +
                        "\"\" " + funText->GetValue() + 
                        boost::to_string( port->GetValue() ) + ")";
                    
    Word qRes;
    if( !QueryProcessor::ExecuteQuery( query, qRes ) ) {
        resultDFRel->makeUndefined();
        return 0;
    } else {
        DFArray* dfarray = (DFArray*) qRes.addr;
        if( !dfarray || !dfarray->IsDefined()){
            resultDFRel->makeUndefined();
            delete dfarray;
            return 0;
        }
        resultDFRel->copyFrom( *dfarray );
        resultDFRel->setDistType( drel2->getDistType()->copy() );
        delete dfarray;
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

/*
3.3 Specifications

3.3.1 Specification of ~dreldelete~ 

*/

OperatorSpec dreldeleteSpec( 
    " d[f]rel(rel(tuple(X))) x drel(rel(tuple(X))) x int"
    "-> dfrel(rel(tuple(X@[TID:tid])))",
    " _ _ dreldelete [_]",
    "Deletes tuples from the distributed relation given "
    "as a second argument. The first distributed relation "
    "contains tuples to delete. The third argument "
    "is a port number.",
    "query drel1 drelfilter[.No > 5] drel1 dreldelete [1238]"
);

/*
3.3.2 Specification of ~drelupdate~ 

*/

OperatorSpec drelupdateSpec( 
    " d[f]rel(rel(tuple(X))) x drel(rel(tuple(X))) x funlist x int"
    "-> dfrel(rel(tuple(X@[X1_old t1]@...[Xn_old tn]@[TID:tid])))",
    " _ _ drelupdate [_ ; _]",
    "Updates tuples in the distributed relation given " 
    "as a second argument by replacing attributes as "
    "specified in the funlist. The first distributed "
    "relation contains tuples to update. The fourth argument "
    "is a port number.", 
    "query drel1 drelfilter[.No > 5] drel1 drelupdate [No: .No + 1; 1238]" 
);

/*
3.3.3 Specification of ~dreldeletebyid~ 

*/

OperatorSpec dreldeletebyidSpec( 
    " d[f]rel(rel(tuple(X))) x drel(rel(tuple(Y))) x attr x int"
    "-> dfrel(rel(tuple(Y@[TID:tid]))),   "
    " d[f]array(rel(tuple(X))) x drel(rel(tuple(Y))) x attr x int"
    "-> dfrel(rel(tuple(Y@[TID:tid]))) ",
    " _ _ dreldeletebyid [_, _]",
    "Deletes tuples from the distributed relation given "
    "as a second argument. The first argument contains "
    "tuples to delete and specifies tupleids by the attr "
    "argument. It can be of types d[f]array or d[f]rel. "
    "The last argument is a port number.",
    "query drel1 drelfilteraddid[.No > 5] drel1 dreldeletebyid[TID, 1238]"
);

/*
3.3.4 Specification of ~drelupdatebyid~ 

*/

OperatorSpec drelupdatebyidSpec( 
    " d[f]rel(rel(tuple(X))) x drel(rel(tuple(Y))) x attr x funlist x int"
    "-> dfrel(rel(tuple(X))),   "
    " d[f]array(rel(tuple(X))) x drel(rel(tuple(Y))) x attr x funlist x int"
    "-> dfrel(rel(tuple(X)))",                            
    " _ _ drelupdatebyid [_ ; _; _]",
    "Updates tuples in the distributed relation given "
    "as a second argument. The first argument contains  " 
    "tuples to update and specifies tupleid by the attr "
    "argument. It can be of types d[f]array or d[f]rel. "
    "The functions may use parts from the first argument " 
    "(dot notation) or from the second drel (double dot notation).",
    "query drel1 dreladdid drel1 drelupdatebyid [TID; No: ..No + 1; 1238]"
);

/*
3.4 Definitions 

3.4.1 Definition of ~dreldelete~

*/

Operator dreldeleteOp (
    "dreldelete",                
    dreldeleteSpec.getStr( ),         
    2,
    dreldmap2VM,           
    dreldmap2UpdSelect,      
    dreldeleteTM           
);

/*
3.4.2 Definition of ~drelupdate~

*/

Operator drelupdateOp (
    "drelupdate",                
    drelupdateSpec.getStr( ),         
    2,
    dreldmap2VM,           
    dreldmap2UpdSelect,      
    drelupdateTM           
);

/*
3.4.3 Definition of ~dreldeletebyid~

*/

Operator dreldeletebyidOp (
    "dreldeletebyid",                
    dreldeletebyidSpec.getStr( ),         
    2,
    dreldmap2VM,           
    dreldmap2UpdSelect,     
    dreldeletebyidTM           
);

/*
3.4.4 Definition of ~drelupdatebyid~

*/

Operator drelupdatebyidOp (
    "drelupdatebyid",                
    drelupdatebyidSpec.getStr( ),         
    2,
    dreldmap2VM,           
    dreldmap2UpdSelect,     
    drelupdatebyidTM           
);

/*
4. Operators ~drelinsertrtree~, ~dreldeletertree~, ~drelupdatertree~,
~drelinsertbtree~, ~dreldeletebtree~, ~drelupdatebtree~

4.1 Type Mapping function for update distributed rtree

*/

ListExpr drelAllUpdatesRTreeTM (ListExpr& args, std::string opName ){
    
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
    
    if( !nl->HasLength(result, 3) ){
        return result;
    }

    //create function for dmap2 operator
    std::string fun = "(fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
                      "(" + opName + " (feed elem1) elem2 " + 
                      nl->ToString(attrName) + "))";

    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second( nl->Third( result ) ) ),
                            nl->Third( drelType ) );
    
    ListExpr append = nl->OneElemList( nl->TextAtom( fun ) );

    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
4.1.1 Type Mapping of ~drelinsertrtree~

*/

ListExpr drelinsertrtreeTM(ListExpr args){
    return drelAllUpdatesRTreeTM(args, "insertrtree");
}

/*
4.1.2 Type Mapping of ~dreldeletertree~

*/

ListExpr dreldeletertreeTM(ListExpr args){
    return drelAllUpdatesRTreeTM(args, "deletertree");
}

/*
4.1.3 Type Mapping of ~drelupdatertree~

*/

ListExpr drelupdatertreeTM(ListExpr args){
    return drelAllUpdatesRTreeTM(args, "updatertree");
}

/*
4.2 Type Mapping function for update distributed btree

*/

ListExpr drelAllUpdatesBTreeTM (ListExpr& args, std::string opName ){
    
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
    std::string fun = "(fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
                      "(" + opName + " (feed elem1) elem2 " + 
                      nl->ToString(attrName) + "))";

    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(), 
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(), 
                                nl->Second( nl->Third( result ) ) ), 
                            nl->Third( drelType ) );

    ListExpr append = nl->OneElemList( nl->TextAtom( fun ) );
        
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
4.2.1 Type Mapping of ~drelinsertbtree~

*/

ListExpr drelinsertbtreeTM(ListExpr args){
    return drelAllUpdatesBTreeTM(args, "insertbtree");
}

/*
4.2.2 Type Mapping of ~dreldeletebtree~

*/

ListExpr dreldeletebtreeTM(ListExpr args){
    return drelAllUpdatesBTreeTM(args, "deletebtree");
}

/*
4.2.3 Type Mapping of ~drelupdatebtree~

*/

ListExpr drelupdatebtreeTM(ListExpr args){
    return drelAllUpdatesBTreeTM(args, "updatebtree");
}

/*
4.3 Value Mapping function

used by ~drelinsertrtree~, ~dreldeletertree~, ~drelupdatertree~, 
~drelinsertbtree~, ~dreldeletebtree~, ~drelupdatebtree~ operators

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
    DFRel* resultDFRel = (DFRel*)result.addr;
    
    ListExpr arg1ptr = DRelHelpers::createdrel2darray(
                        qp->GetType( qp->GetSon(s,0) ), drel );
    ListExpr arg2ptr = DRelHelpers::createPointerList(
                        qp->GetType( qp->GetSon(s,1) ), da );
    
    std::string query = "(dmap2 " + nl->ToString(arg1ptr) + 
                                    nl->ToString(arg2ptr) +
                        "\"\" " + funText->GetValue() + 
                        boost::to_string( port->GetValue() ) + ")";
                    
    Word qRes;
    if( !QueryProcessor::ExecuteQuery( query, qRes ) ) {
        resultDFRel->makeUndefined();
        return 0;
    } else {
        DFArray* dfarray = (DFArray*) qRes.addr;
        if( !dfarray || !dfarray->IsDefined()){
            resultDFRel->makeUndefined();
            delete dfarray;
            return 0;
        }
        resultDFRel->copyFrom( *dfarray );
        resultDFRel->setDistType( drel->getDistType()->copy() );
        delete dfarray;
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

/*
4.4 Specifications

4.4.1 Specification of ~drelinsertrtree~

*/

OperatorSpec drelinsertrtreeSpec(
    " d[f]rel(rel(tuple(X@[TID:tid]))) x da(rtree(tuple(X))) x attr x int"
    "-> dfrel(rel(tuple(X@[TID:tid])))",
    " _ _ drelinsertrtree [_, _]",
    "Inserts references of tuples with tupleids into the "
    "distributed rtree. ",
    "query drel da_rtree drelinsertrtree [GeoData, 1238]"
);

/*
4.4.2 Specification of ~dreldeletertree~

*/

OperatorSpec dreldeletertreeSpec( 
    " d[f]rel(rel(tuple(X@[TID:tid]))) x da(rtree(tuple(X))) x attr x int"
    "-> dfrel(rel(tuple(X@[TID:tid])))",
    " _ _ dreldeletertree [_, _]",
    "Deletes references of tuples with tupleids from the "
    "distributed rtree. ",
    "query drel da_rtree dreldeletertree [GeoData, 1238]"
);

/*
4.4.3 Specification of ~drelupdatertree~

*/

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
4.4.4 Specification of ~drelinsertbtree~

*/

OperatorSpec drelinsertbtreeSpec(
    " d[f]rel(rel(tuple(X@[TID:tid]))) x da(btree(tuple(X))) x attr x int"
    "-> dfrel(rel(tuple(X@[TID:tid])))",
    " _ _ drelinsertbtree [_, _]",
    "Inserts references of tuples with tupleids into the "
    "distributed btree. ",
    "query drel da_btree drelinsertbtree [BevT, 1238]"
);

/*
4.4.5 Specification of ~dreldeletebtree~

*/

OperatorSpec dreldeletebtreeSpec(
    " d[f]rel(rel(tuple(X@[TID:tid]))) x da(btree(tuple(X))) x attr x int"
    "-> dfrel(rel(tuple(X@[TID:tid])))",
    " _ _ dreldeletebtree [_, _]",
    "Deletes references of tuples with tupleids from the "
    "distributed btree. ",
    "query drel da_btree dreldeletebtree [BevT, 1238]"
);

/*
4.4.6 Specification of ~drelupdatebtree~

*/

OperatorSpec drelupdatebtreeSpec( 
    " d[f]rel(rel(tuple(X@[X1_old t1]@...[Xn_old tn]@[TID:tid]))) x"
    " da(btree(tuple(X))) x attr x int "
    "-> dfrel(rel(tuple(X@[X1_old t1]@...[Xn_old tn]@[TID:tid])))",
    " _ _ drelupdatebtree [_, _]",
    "Updates references of tuples with tupleids in the " 
    "distributed btree.",
    "query drel da_btree drelupdatebtree [BevT, 1238]"
);

/*
4.5 Definitions

4.5.1 Definition of ~drelinsertrtree~

*/
Operator drelinsertrtreeOp (
    "drelinsertrtree",                
    drelinsertrtreeSpec.getStr( ),         
    2,
    dreldmap2IVM,           
    dreldmap2IUpdSelect,   
    drelinsertrtreeTM           
);

/*
4.5.2 Definition of ~dreldeletertree~

*/

Operator dreldeletertreeOp (
    "dreldeletertree",                
    dreldeletertreeSpec.getStr( ),         
    2,
    dreldmap2IVM,           
    dreldmap2IUpdSelect,    
    dreldeletertreeTM           
);

/*
4.5.3 Definition of ~drelupdatertree~

*/

Operator drelupdatertreeOp (
    "drelupdatertree",                
    drelupdatertreeSpec.getStr( ),         
    2,
    dreldmap2IVM,           
    dreldmap2IUpdSelect,      
    drelupdatertreeTM           
);

/*
4.5.4 Definition of ~drelinsertbtree~

*/

Operator drelinsertbtreeOp (
    "drelinsertbtree",                
    drelinsertbtreeSpec.getStr( ),         
    2,
    dreldmap2IVM,           
    dreldmap2IUpdSelect,    
    drelinsertbtreeTM           
);

/*
4.5.5 Definition of ~dreldeletebtree~

*/

Operator dreldeletebtreeOp (
    "dreldeletebtree",                
    dreldeletebtreeSpec.getStr( ),         
    2,
    dreldmap2IVM,           
    dreldmap2IUpdSelect,    
    dreldeletebtreeTM           
);

/*
4.5.6 Definition of ~drelupdatebtree~

*/

Operator drelupdatebtreeOp (
    "drelupdatebtree",                
    drelupdatebtreeSpec.getStr( ),
    2,
    dreldmap2IVM,           
    dreldmap2IUpdSelect,     
    drelupdatebtreeTM           
);

/*
5. Operators ~dreladdid~, ~drelfilteraddid~, ~drelfilterdelete~,
~drelfilterupdate~

5.1 Type Mapping functions

5.1.1 Type Mapping of ~dreladdid~

*/

ListExpr dreladdidTM ( ListExpr args ){
    
    if(!nl->HasLength(args, 1) ){
        return listutils::typeError("wrong number of arguments");
    }
    ListExpr drelType = nl->First(args); 
    if( !DRel::checkType( drelType ) && 
        !DFRel::checkType( drelType ) ){
        return listutils::typeError(
                "first argument is not a d[f]rel");
    }
    
    ListExpr result = appendIdentifierTypeMap(
                        nl->OneElemList(
                            nl->TwoElemList(
                                listutils::basicSymbol<Stream<Tuple> >(),
                                nl->Second( nl->Second( drelType ) ) ) ) );
    
    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    
    // create function for dmap operator
    std::string fun ="(fun (elem1 ARRAYFUNARG1) (addid (feed elem1)))";
    
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second( result ) ),
                            nl->Third( drelType ) ); //distType
    
    ListExpr append = nl->OneElemList( nl->TextAtom( fun ) );

    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
5.1.2 Type Mapping of ~drelfilteraddid~

*/

ListExpr drelfilteraddidTM( ListExpr args ){
    
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
                "first argument is not a d[f]rel"); 
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
                            nl->Second( relType ) ),
                        nl->TwoElemList(
                            nl->SymbolAtom("feed"), //dummy
                            drelValue ) ),
                    nl->TwoElemList( map, fun ) ) );
    
    if(!listutils::isTupleStream( filterRes ) ){
        return filterRes;
    }
    
    ListExpr result = appendIdentifierTypeMap( nl->OneElemList( filterRes ) );
    
    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    
    // create function for dmap operator
    std::string funText = "(fun (elem1 ARRAYFUNARG1) (addid (filter "
                          "(feed elem1) " + nl->ToString(fun) + ")))";

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

/*
5.1.3 Type Mapping of ~drelfilterdelete~

*/

ListExpr drelfilterdeleteTM( ListExpr args ){
    
    if(!nl->HasLength(args,2)){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args) ){
        return listutils::typeError("internal error");
    }
    // check for correct types
    ListExpr drelType = nl->First( nl->First( args ) );
    ListExpr drelValue = nl->Second( nl->First( args ) );
    if( !DRel::checkType( drelType ) ){
        return listutils::typeError(
                "first argument is not a drel"); // ONLY DREL
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
                            nl->Second( nl->Second( drelType ) ) ),
                        nl->TwoElemList(
                            nl->SymbolAtom("feed"), //dummy
                            drelValue ) ),
                    nl->TwoElemList( map, fun ) ) );
    
    if(!listutils::isTupleStream( filterRes ) ){
        return filterRes;
    }

    ListExpr result = insertDeleteRelTypeMap<0>( nl->TwoElemList(
                                            filterRes, 
                                            nl->Second( drelType ) ) );

    if(!listutils::isTupleStream( result ) ){
        return result;
    }
    
    // create function for dmap operator
    std::string funText = "(fun (elem1 ARRAYFUNARG1) (deletedirect "
           "(filter (feed elem1) " + nl->ToString(fun) + ") elem1))";
 
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(), 
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(), 
                                nl->Second( result ) ), 
                            nl->Third( drelType ) ); // distType

    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );
        
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
5.1.4 Type Mapping of ~drelfilterupdate~

*/

ListExpr drelfilterupdateTM( ListExpr args ){
    
    if(!nl->HasLength(args,3)){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists(args) ){
        return listutils::typeError("internal error");
    }
    // check for correct types
    ListExpr drelType = nl->First( nl->First( args ) );
    ListExpr drelValue = nl->Second( nl->First( args ) );
    if( !DRel::checkType( drelType ) ){
        return listutils::typeError(
                "first argument is not a drel"); // ONLY DREL
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
                            nl->Second( nl->Second( drelType ) ) ),
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

    ListExpr result = updateDirectRelTypeMap( 
                            nl->ThreeElemList(
                                filterRes,
                                nl->Second( drelType ), // relType
                                update_mapList ) );

    if( !nl->HasLength(result, 3) ){
        return result;
    }
    
    // create function for dmap operator
    std::string funText = "(fun (elem1 ARRAYFUNARG1) (updatedirect "
          "(filter (feed elem1) " + nl->ToString(filter_fun) + ") elem1 " + 
           nl->ToString(funList_new) + "))";
           
    ListExpr resType = nl->ThreeElemList(
                            listutils::basicSymbol<DFRel>(), 
                            nl->TwoElemList(
                                listutils::basicSymbol<Relation>(), 
                                nl->Second( nl->Third( result ) ) ), 
                            nl->Third( drelType ) ); //distType

    ListExpr append = nl->OneElemList( nl->TextAtom( funText ) );
        
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
5.2 Value Mapping function

used by ~dreladdid~, ~drelfilteraddid~, ~drelfilterdelete~, 
~drelfilterupdate~ operators

*/

template<class R>
int dreldmapVMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){
    
    int argsNum = qp->GetNoSons( s );

    R* drel = (R*) args[0].addr; 
    FText* funText = (FText*) args[argsNum - 1].addr;
    
    result = qp->ResultStorage(s);
    DFRel* resultDFRel = (DFRel*)result.addr;
    
    ListExpr drelptr = DRelHelpers::createdrel2darray
                            (qp->GetType( qp->GetSon(s, 0) ), drel);

    std::string query = "(dmap " + nl->ToString(drelptr) + "\"\" " + 
                          funText->GetValue() + ")";

    Word qRes;
    if( !QueryProcessor::ExecuteQuery( query, qRes ) ) {
        resultDFRel->makeUndefined();
        return 0;
    } else {
        DFArray* dfarray = (DFArray*) qRes.addr;
        if( !dfarray || !dfarray->IsDefined()){
            resultDFRel->makeUndefined();
            delete dfarray;
            return 0;
        }
        resultDFRel->copyFrom( *dfarray );
        resultDFRel->setDistType( drel->getDistType()->copy() );
        delete dfarray;
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

/*
5.3 Specifications

5.3.1 Specification of ~dreladdid~

*/

OperatorSpec dreladdidSpec( 
    " d[f]rel(rel(tuple(X))) -> dfrel(rel(tuple(X@[TID:tid])))", 
    " _ dreladdid",
    "Appends tupleid attribute to each tuple of the "
    "distributed relation. Must be applied directly "
    "to the distributed relation, because other " 
    "operators may corrupt tupleids.", 
    "query drel1 dreladdid head[5] drel1 dreldeletebyid[TID, 1238]"
);

/*
5.3.2 Specification of ~drelfilteraddid~

*/

OperatorSpec drelfilteraddidSpec( 
    " d[f]rel(rel(tuple(X))) x fun -> dfrel(rel(tuple(X@[TID:tid])))", 
    " _ drelfilteraddid [_]",
    "Only tuples, fulfilling a certain condition are selected, "
    "extended with the TID attribute and passed to the output "
    "distributed relation.", 
    "query drel1 drelfilteraddid [.No > 5] dreldeletebyid[TID, 1238]"
);

/*
5.3.3 Specification of ~drelfilterdelete~

*/

OperatorSpec drelfilterdeleteSpec( 
    "drel(rel(tuple(X))) x fun -> dfrel(rel(tuple(X@[TID:tid])))", 
    " _ drelfilterdelete [_]",
    "Only tuples, fulfilling a certain condition are selected "
    "and deleted from the distributed relation. The output "
    "distributed relation contains deleted tuples extended "
    "with the TID attribute. ", 
    "query drel1 drelfilterdelete[.No > 5]"
);

/*
5.3.4 Specification of ~drelfilterupdate~

*/

OperatorSpec drelfilterupdateSpec( 
    " drel(rel(tuple(X))) x fun x funlist -> "
    " dfrel(rel(tuple(X@[X1_old t1]@...[Xn_old tn]@[TID:tid])))", 
    " _ drelfilterupdate [filter_fun; update_funlist]",
    "Only tuples, fulfilling a certain condition (second argument) "
    "are selected and updated by replacing attributes as specified "
    "in the funlist (third argument). The output "
    "distributed relation contains updated tuples extended "
    "with old attribute values and TID attribute. ", 
    "query drel1 drelfilterupdate[.No > 5; No: .No + 1]"
);

/*
5.4 Definitions

5.4.1 Definition of ~dreladdid~

*/

Operator dreladdidOp (
    "dreladdid",                
    dreladdidSpec.getStr( ),
    2,
    dreldmapVM,           
    dreldmapUpdSelect,     
    dreladdidTM           
);

/*
5.4.2 Definition of ~drelfilteraddid~

*/

Operator drelfilteraddidOp (
    "drelfilteraddid",                
    drelfilteraddidSpec.getStr( ),
    2,
    dreldmapVM,           
    dreldmapUpdSelect,     
    drelfilteraddidTM           
);

/*
5.4.3 Definition of ~drelfilterdelete~

*/

Operator drelfilterdeleteOp (
    "drelfilterdelete",                
    drelfilterdeleteSpec.getStr( ),
    2,
    dreldmapVM,           
    dreldmapUpdSelect,     
    drelfilterdeleteTM           
);

/*
5.4.4 Definition of ~drelfilterupdate~

*/

Operator drelfilterupdateOp (
    "drelfilterupdate",                
    drelfilterupdateSpec.getStr( ),
    2,
    dreldmapVM,           
    dreldmapUpdSelect,     
    drelfilterupdateTM           
);

/*
6. Operators ~drelexactmatchS~, ~drelrangeS~, ~drelwindowintersectsS~

6.1 Type Mapping functions

6.1.1 Type Mapping of ~drelexactmatchS~, ~drelrangeS~
 
*/

//RANGE 2, EXACTMATCH 3
template<int operatorId>
ListExpr drelindexquerySTM (ListExpr args){

    //check number of arguments 2/3
    if( !nl->HasLength( args, 2 ) && !nl->HasLength( args, 3 ) ){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists( args ) ){
        return listutils::typeError("internal error");
    }
    ListExpr darrayBTreeType = nl->First( nl->First( args ) );
    if( !DArray::checkType( darrayBTreeType ) ){
        return listutils::typeError(
            "first argument is not a darray");
    }

    ListExpr args2;
    if( operatorId == 2 ){ 
        args2 = nl->ThreeElemList(
                    nl->Second( darrayBTreeType ),
                    nl->First( nl->Second( args ) ),
                    nl->First( nl->Third( args ) ) );
    } else {
        args2 = nl->TwoElemList(
                    nl->Second( darrayBTreeType ),
                    nl->First( nl->Second( args ) ) );
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

    std::string funText;
    if( operatorId == 2 ){ 
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

/*
6.1.2 Type Mapping of ~drelwindowintersectsS~
 
*/

ListExpr drelwindowintersectsSTM (ListExpr args){

    if( !nl->HasLength( args, 2 ) ){
        return listutils::typeError("wrong number of arguments");
    }
    if(!DRelHelpers::isListOfTwoElemLists( args ) ){
        return listutils::typeError("internal error");
    }
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
    
    std::string fun = "(fun (elem1 ARRAYFUNARG1) (windowintersectsS elem1 " +
                nl->ToString( nl->Second( nl->Second( args ) ) ) + ")) ";
                
    ListExpr append = nl->OneElemList( nl->TextAtom( fun ) );
        
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        append,
        resType);
}

/*
6.2 Value Mapping function

used by ~drelexactmatchS~, ~drelrangeS~, ~drelwindowintersectsS~ operators
 
*/

template<class A>
int dreldmapSVMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){
    
    int argsNum = qp->GetNoSons( s );

    A* darray = (A*) args[0].addr; 
    FText* funText = (FText*) args[argsNum - 1].addr;
    
    result = qp->ResultStorage(s);
    DFArray* resultDFArray = (DFArray*)result.addr;
    
    ListExpr arg1ptr = DRelHelpers::createPointerList
                            (qp->GetType( qp->GetSon(s, 0) ), darray);

    std::string query = "(dmap " + nl->ToString( arg1ptr ) + 
                        "\"\" " + funText->GetValue() + ")";

    Word qRes;
    if( !QueryProcessor::ExecuteQuery( query, qRes ) ) {
        resultDFArray->makeUndefined();
        return 0;
    } else {
        resultDFArray = (DFArray*) qRes.addr; 
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
6.3 Specifications

6.3.1 Specification of ~drelexactmatchS~

*/

OperatorSpec drelexactmatchSSpec( 
    " da(btree(tuple(X))) x ti -> dfarray(rel(tuple(Id tid)))",
    " _ drelexactmatchS [_]",
    "Uses the given distributed btree to find all tuple "
    "identifiers where the key matches argument value ti. ",
    "query da_btree drelexactmatchS [10]"
);

/*
6.3.2 Specification of ~drelrangeS~

*/

OperatorSpec drelrangeSSpec( 
    " da(btree(tuple(X))) x ti x ti-> dfarray(rel(tuple(Id tid)))",
    " _ drelrangeS [_, _]",
    "Uses the given distributed btree to find all tuple "
    "identifiers where the key is between argument values ti. ",
    "query da_btree drelrangeS [5, 10]"
);

/*
6.3.3 Specification of ~drelwindowintersectsS~

*/

OperatorSpec drelwindowintersectsSSpec( 
    " da(rtree(tuple(X))) x ti -> dfarray(rel(tuple(Id tid)))",
    " _ drelwindowintersectsS [_]",
    "Uses the given distributed rtree to find all tuple "
    "identifiers whose bounding box intersects with "
    "ti bounding box. The argument value ti can be in "
    "{rect<d>} or SPATIAL<d>D, where d in {2, 3, 4, 8}.",
    "query da_rtree drelwindowintersectsS [ bbox(thecenter) ]"
);

/*
6.4 Definitions

6.4.1 Definition of ~drelexactmatchS~

*/

Operator drelexactmatchSOp (
    "drelexactmatchS",                
    drelexactmatchSSpec.getStr( ),
    2,
    dreldmapSVM,           
    dreldmapSUpdSelect,     
    drelindexquerySTM<3>           
);

/*
6.4.2 Definition of ~drelrangeS~

*/

Operator drelrangeSOp (
    "drelrangeS",                
    drelrangeSSpec.getStr( ),
    2,
    dreldmapSVM,           
    dreldmapSUpdSelect,     
    drelindexquerySTM<2>             
);

/*
6.4.3 Definition of ~drelwindowintersectsS~

*/

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
