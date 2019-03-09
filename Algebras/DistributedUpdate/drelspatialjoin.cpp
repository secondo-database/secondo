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
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/DRel/DRel.h"
#include "Algebras/DRel/DRelHelpers.h"

#include <string.h>
#include <iostream>

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace drel;
using namespace distributed2;

ListExpr realJoinMMRTreeTM(ListExpr args);
ListExpr RenameTypeMap(ListExpr args); 

namespace drelupdate{
    
ListExpr drelspatialjoinTM(ListExpr args){

    std::string err = "d[f]rel(X) x d[f]rel(Y) x attr x attr expected";
    
    if( !nl->HasLength( args, 4 ) ){
        return listutils::typeError(err + 
                        ": four arguments are expected");
    }
    if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
        return listutils::typeError( "internal Error" );
    }
    
    ListExpr drel1Type = nl->First( nl->First( args ) );
    ListExpr drel1Value = nl->Second( nl->First( args ) );
    ListExpr drel2Type = nl->First( nl->Second( args ) );
    ListExpr drel2Value = nl->Second( nl->Second( args ) );

    ListExpr attr1Name = nl->First( nl->Third( args ) );
    ListExpr attr2Name = nl->First( nl->Fourth( args ) ); 
    
    ListExpr darray1Type, darray2Type;
    distributionType dType1, dType2;
    int dAttr1, dKey1, dAttr2, dKey2;

    if( !DRelHelpers::drelCheck( 
        drel1Type, darray1Type, dType1, dAttr1, dKey1 ) ) {
        return listutils::typeError(
                err + ": first argument is not a d[f]rel" );
    }
    if( !DRelHelpers::drelCheck( 
        drel2Type, darray2Type, dType2, dAttr2, dKey2 ) ) {
        return listutils::typeError(
                err + ": second argument is not a d[f]rel" );
    }
    if( dType1 == replicated || dType2 == replicated ) {
        return listutils::typeError( 
                "join of replicated d[f]rels not supported" );
    }
    
    ListExpr attr1L, attr2L, attr1List, attr2List;
    attr1L = nl->Second( nl->Second( nl->Second( drel1Type ) ) );
    attr2L = nl->Second( nl->Second( nl->Second( drel1Type ) ) ); 

    attr1List = DRelHelpers::removePartitionAttributes(attr1L, dType1);
    attr2List = DRelHelpers::removePartitionAttributes(attr2L, dType2);
    
    ListExpr rename1Res = RenameTypeMap(nl->TwoElemList(
            nl->TwoElemList(
                listutils::basicSymbol<Stream<Tuple> >( ),
                nl->TwoElemList(
                    listutils::basicSymbol<Tuple>( ),
                    attr1List ) ),
            nl->SymbolAtom("a") ) );
    
    ListExpr rename2Res = RenameTypeMap(nl->TwoElemList(
            nl->TwoElemList(
                listutils::basicSymbol<Stream<Tuple> >( ),
                nl->TwoElemList(
                    listutils::basicSymbol<Tuple>( ),
                    attr2List ) ),
            nl->SymbolAtom("b") ) );
    
    cout << "rename1Res: " << nl->ToString(rename1Res) << endl;
    cout << "rename2Res: " << nl->ToString(rename2Res) << endl;
    
    string attr1 = nl->SymbolValue(attr1Name) + "_a";
    string attr2 = nl->SymbolValue(attr2Name) + "_b";
    
    ListExpr spJoinTMresult = realJoinMMRTreeTM( nl->FourElemList(
                                            rename1Res,
                                            rename2Res,
                                            nl->SymbolAtom( attr1 ),
                                            nl->SymbolAtom( attr2 ) ) ); 

    if( !nl->HasLength( spJoinTMresult, 3 ) ) {
        return spJoinTMresult;
    }

    ListExpr resAttrList = DRelHelpers::addPartitionAttributes(
                nl->Second( nl->Second( nl->Third( spJoinTMresult ) ) ) );

    ListExpr resultRel = nl->TwoElemList(
                                listutils::basicSymbol<Relation>( ),
                                nl->TwoElemList(
                                    listutils::basicSymbol<Tuple>( ),
                                    resAttrList) );

    ListExpr ftype1, ftype2;
    int pos1 = listutils::findAttribute( 
                attr1L, nl->SymbolValue( attr1Name ), ftype1 );
    int pos2 = listutils::findAttribute( 
                attr2L, nl->SymbolValue( attr2Name ), ftype2 );
    
    //check whether spatial repartition by join attributes is allowed
    bool allowedType2d_1, allowedType3d_1;
    bool allowedType2d_2, allowedType3d_2;
    
    allowedType2d_1 = DistTypeSpatial<temporalalgebra::CellGrid2D>::
                                            allowedAttrType2d( ftype1 );
    allowedType3d_1 = DistTypeSpatial<temporalalgebra::CellGrid<3> >::
                                            allowedAttrType3d( ftype1 );
    allowedType2d_2 = DistTypeSpatial<temporalalgebra::CellGrid2D>::
                                            allowedAttrType2d( ftype2 );
    allowedType3d_2 = DistTypeSpatial<temporalalgebra::CellGrid<3> >::
                                            allowedAttrType3d( ftype2 );
                                            
    if( !allowedType2d_1  &&  !allowedType3d_1 ){
        return listutils::typeError("spatial repartition of first d[f]rel"
        " by " + nl->SymbolValue(attr1Name) + " attribute not allowed");
    }
    
    if( !allowedType2d_2  &&  !allowedType3d_2 ){
        return listutils::typeError("spatial repartition of second d[f]rel"
        " by " + nl->SymbolValue(attr2Name) + " attribute not allowed");
    }
    
    if( (allowedType2d_1 && !allowedType2d_2) ||
        (allowedType3d_1 && !allowedType3d_2) ){
        return listutils::typeError("spatial repartition of d[f]rels"
        "in the same manner is not possible");    
    }
    
    //check whether spatial repartition by join attributes is necessary    
    bool rel1Flag = false;
    bool rel2Flag = false;
    bool compatFlag = false;    

    if( (dType1 == spatial2d || dType1 == spatial3d) &&
        (dAttr1 == pos1 - 1) ) {
        rel1Flag = true;
    }

    if( (dType2 == spatial2d || dType2 == spatial3d) && 
        (dAttr2 == pos2 - 1) ) {
        rel2Flag = true;
    }

    if( (rel1Flag && rel2Flag && dType1 == spatial2d && dType2 == spatial2d) || 
        (rel1Flag && rel2Flag && dType1 == spatial3d && dType2 == spatial3d) ){
        compatFlag = dKey1 == dKey2;
    }
    
    if( rel1Flag && rel2Flag && !compatFlag ) {
            
        if( !nl->IsAtom( drel1Value )
          || !nl->IsAtom( drel2Value )
          || nl->AtomType( drel1Value ) != SymbolType
          || nl->AtomType( drel2Value ) != SymbolType ) {
             return listutils::typeError( 
                    "cannot estimate drel size" );
        }
        //only for drel
        int size1 = DRelHelpers::countDRel( nl->SymbolValue( drel1Value ) );
        int size2 = DRelHelpers::countDRel( nl->SymbolValue( drel2Value ) );

        if( size1 <= size2 ) { //smaller drel will be repartitioned
            rel1Flag = false;
            cout << "repartition of drel 1 is necessary" << endl;
        } 
        else {
            rel2Flag = false;
            cout << "repartition of drel 2 is necessary" << endl;
        }
    } 

    //build result type
    ListExpr resultType;
    
    distributionType dType;
    ListExpr cellgridtype;
    if( allowedType2d_1 ) {
        cellgridtype = listutils::
                    basicSymbol<temporalalgebra::CellGrid2D>( );
        dType = spatial2d;
    } else {
        cellgridtype = listutils::
                    basicSymbol<temporalalgebra::CellGrid<3> >( );
        dType = spatial3d;
    }

    resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            resultRel,
            nl->FourElemList( 
                nl->IntAtom( dType ),
                nl->IntAtom( pos1 - 1 ),
                nl->IntAtom( rand() ),
                cellgridtype) );
    
    ListExpr appendList = nl->FourElemList(
                            nl->BoolAtom( rel1Flag ),
                            nl->BoolAtom( rel2Flag ),
                            nl->StringAtom( nl->ToString( attr1Name ) ), 
                            nl->StringAtom( nl->ToString( attr2Name ) ) );
    
    return nl->ThreeElemList( 
                nl->SymbolAtom( Symbols::APPEND( ) ),
                appendList,
                resultType );
}

bool spatialPartition(ListExpr drel1Type, void* ptr1, string attr, 
                      ListExpr drel2Type, void* ptr2, DFRel*& resDFRel,
                      ListExpr& dfrelType){
    
    ListExpr query;
    if(ptr2 != 0){
        query = nl->FourElemList(
                    nl->SymbolAtom("drelspatialpartition"),
                    DRelHelpers::createPointerList(drel1Type, ptr1),
                    nl->SymbolAtom( attr ),
                    DRelHelpers::createPointerList(drel2Type, ptr2) );
    } else {
        query = nl->ThreeElemList(
                    nl->SymbolAtom("drelspatialpartition"),
                    DRelHelpers::createPointerList(drel1Type, ptr1),
                    nl->SymbolAtom( attr ) );
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
        resDFRel = ( DFRel* )qRes.addr;
        if( !resDFRel->IsDefined() ){
            return false;
        }
        dfrelType = resultType;
    }
    
    return true;
}

//R1 first d[f]rel, R1 second d[f]rel, G gridType
template<class R1, class R2, class G>
int drelspatialjoinVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {
            
    R1* drel1 = ( R1* ) args[0].addr;
    R2* drel2 = ( R2* ) args[1].addr;
    bool rel1Flag = ( (CcBool*) args[4].addr )->GetBoolval();
    bool rel2Flag = ( (CcBool*) args[5].addr )->GetBoolval();
    string attr1 = ( (CcString*) args[6].addr )->GetValue();  
    string attr2 = ( (CcString*) args[7].addr )->GetValue(); 
    
    ListExpr drel1Type = qp->GetType( qp->GetSon(s,0) );
    ListExpr drel2Type = qp->GetType( qp->GetSon(s,1) );
    
    result = qp->ResultStorage( s );
    DFRel* resultDFRel = ( DFRel* )result.addr;
    
    distributionType dType;
    int attr, key;
    DFRel::checkDistType( nl->Third( qp->GetType( s ) ), dType, attr, key );
    
    ListExpr drel1ptr, drel2ptr;
    G* grid = 0;
    if(rel1Flag && rel2Flag){
        
        cout << "1 & 2" << endl;
        
        drel1ptr = DRelHelpers::createdrel2darray(drel1Type, drel1);
        drel2ptr = DRelHelpers::createdrel2darray(drel2Type, drel2);
        
        grid = ( (DistTypeSpatial<G>*) drel1->getDistType( ) )->getGrid( );
        
    } else if(rel1Flag && !rel2Flag){ 
        
        cout << "1 & !2" << endl;
        
        drel1ptr = DRelHelpers::createdrel2darray(drel1Type, drel1);
        
        DFRel* dfrel2 = 0;
        ListExpr dfrel2Type;
        if( !spatialPartition(drel2Type, drel2, attr2, drel1Type, 
                              drel1, dfrel2, dfrel2Type) ){
            cout << "spatial partition of second d[f]rel failed" << endl;
            resultDFRel->makeUndefined();
            return 0;
        }
        
        drel2ptr = DRelHelpers::createdrel2darray(dfrel2Type, dfrel2);
        
        grid = ( (DistTypeSpatial<G>*) drel1->getDistType( ) )->getGrid( );
        
    } else if(!rel1Flag && rel2Flag){ 
        
        cout << "!1 & 2" << endl;
        
        DFRel* dfrel1 = 0;
        ListExpr dfrel1Type;
        if( !spatialPartition(drel1Type, drel1, attr1, drel2Type, 
                              drel2, dfrel1, dfrel1Type) ){
            cout << "spatial partition of first d[f]rel failed" << endl;
            resultDFRel->makeUndefined();
            return 0;
        }
        
        drel1ptr = DRelHelpers::createdrel2darray(dfrel1Type, dfrel1);
        drel2ptr = DRelHelpers::createdrel2darray(drel2Type, drel2);
        
        grid = ( (DistTypeSpatial<G>*) drel2->getDistType( ) )->getGrid( );
        
    } else { 
        
        cout << "!1 & !2" << endl;
        
        DFRel* dfrel1 = 0;
        ListExpr dfrel1Type;
        if( !spatialPartition(drel1Type, drel1, attr1, drel2Type, 
                              0, dfrel1, dfrel1Type) ){
            cout << "spatial partition of first d[f]rel failed" << endl;
            resultDFRel->makeUndefined();
            return 0;
        }
        
        DFRel* dfrel2 = 0;
        ListExpr dfrel2Type;
        if( !spatialPartition(drel2Type, drel2, attr2, dfrel1Type, 
                              dfrel1, dfrel2, dfrel2Type) ){
            cout << "spatial partition of second d[f]rel failed" << endl;
            resultDFRel->makeUndefined();
            return 0;
        }
        
        drel1ptr = DRelHelpers::createdrel2darray(dfrel1Type, dfrel1);
        drel2ptr = DRelHelpers::createdrel2darray(dfrel2Type, dfrel2);
        
        grid = ( (DistTypeSpatial<G>*) dfrel1->getDistType( ) )->getGrid( ); 
    }
    
    //share grid
    string queryS = "(share2 \"grid\" " + nl->ToString(
                     DRelHelpers::createPointerList(
                        nl->Fourth( nl->Third( qp->GetType( s ) ) ), grid) ) +
                    " TRUE " + nl->ToString( drel1ptr ) +")";
    Word res;
    if( !QueryProcessor::ExecuteQuery( queryS, res ) ) {
        cout << "share grid to the workers failed!" << endl;
        resultDFRel->makeUndefined();
        return 0;
    } //not completed
                    
    string filter1fun, filter2fun, filter3fun;
    
    filter1fun = "(fun (s1 STREAMELEM) (= (attr s1 Cell_a) (attr s1 Cell_b)))";
    filter2fun = "(fun (s2 STREAMELEM) (gridintersects (getObject \"grid\") "
                 "(bbox (attr s2 " + attr1 + "_a)) " 
                 "(bbox (attr s2 " + attr2 + "_b)) (attr s2 Cell_a)))";
    
    //control attr for Cell and Original
    string query ="(dmap2 " + nl->ToString(drel1ptr) + nl->ToString(drel2ptr) +
           "\"\" (fun (elem1 ARRAYFUNARG1) (elem2 ARRAYFUNARG2) "
           "(remove (extend (filter (filter (itSpatialJoin "
           "(rename (feed elem1) a) (rename (feed elem2) b) " + attr1 + "_a " +
            attr2 +"_b) " + filter1fun +") " + filter2fun + ") ((Cell "
           "(fun (t1 TUPLE) (attr t1 Cell_a))) (Original (fun (t2 TUPLE) "
           "TRUE)))) (Cell_a Original_a Cell_b Original_b))) 1238)";
            
    ListExpr queryAsList;
    if( !nl->ReadFromString(query, queryAsList)){
        resultDFRel->makeUndefined();
        return 0;
    }
    
    cout << "query: " << nl->ToString(queryAsList) << endl;

    bool correct, evaluable, defined, isFunction;
    ListExpr resultType;
    
    OpTree tree = 0;
    qp->Construct(queryAsList, correct, evaluable, defined, 
                   isFunction, tree, resultType); 
    qp->SetEvaluable(tree, true); 
    
    if(!correct || !evaluable || !defined ){
        resultDFRel->makeUndefined();
        return 0; //for distributed update also
    } else {
        Word qRes;
        qp->EvalS(tree, qRes, OPEN);
        qp->Destroy(tree, false);
        resultDFRel = (DFRel*) qRes.addr;
        if( resultDFRel->IsDefined() ){
            DistTypeSpatial<G>* resultDType = new DistTypeSpatial<G>( 
                                                dType, attr, key, grid );
            resultDFRel->setDistType( resultDType ); //new distType
        }
        result.setAddr(resultDFRel);
    }
    
    return 0;
}

template<class R1, class R2>
int drelspatialjoinVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

    distributionType dType;
    DRelHelpers::drelCheck( qp->GetType( s ), dType );
    
    if( dType == spatial2d ) {
        drelspatialjoinVMT<R1, R2, temporalalgebra::CellGrid2D>
                ( args, result, message, local, s );
    } else {
        drelspatialjoinVMT<R1, R2, temporalalgebra::CellGrid<3> >
                ( args, result, message, local, s );
    }

    return 0;
}

ValueMapping drelspatialjoinVM[] = {
    drelspatialjoinVMT<DRel, DRel>,
    drelspatialjoinVMT<DRel, DFRel>,
    drelspatialjoinVMT<DFRel, DRel>,
    drelspatialjoinVMT<DFRel, DFRel>
};

int drelspatialjoinSelect( ListExpr args ) {
    
    int t1 = DRel::checkType( nl->First( args ) ) ? 0 : 2;
    int t2 = DRel::checkType( nl->Second( args ) ) ? 0 : 1;

    return t1 + t2;
}

/*
Specification of drelspatialjoin

*/

OperatorSpec drelspatialjoinSpec( 
    " d[f]rel(X) x d[f]rel(Y) x attr x attr "
    "-> dfrel(X@Y) ",
    " _ _ drelspatialjoin[_,_]",
    "Computes the spatial join of two d[f]rels. ", //extend!
    " query drel1 drel2 drelspatialjoin[GeoData, GeoData]"
);

/*
Operator instance of drelspatialjoin

*/

Operator drelspatialjoinOp(
    "drelspatialjoin",
    drelspatialjoinSpec.getStr( ),
    4,
    drelspatialjoinVM,
    drelspatialjoinSelect,
    drelspatialjoinTM
);
    
} // end namespace drelupdate
