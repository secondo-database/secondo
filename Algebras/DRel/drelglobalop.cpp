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


1 Implementation of the secondo operators sort, sortby, 
drelgroupby and rdup

*/
//#define DRELDEBUG

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecParser.h"

#include "Algebras/FText/FTextAlgebra.h"

#include "DRelHelpers.h"
#include "DRel.h"
#include "Partitioner.hpp"
#include "BoundaryCalculator.hpp"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace distributed2;

namespace distributed2 {

    extern Distributed2Algebra* algInstance;

    template<class A>
    int dmapVMT( Word* args, Word& result, int message,
        Word& local, Supplier s );

    template<class R>
    int areduceVMT(Word* args, Word& result, int message,
        Word& local, Supplier s );
};

namespace drel {

    ListExpr lsortTM( ListExpr args );
    ListExpr lsortbyTM( ListExpr args );
    ListExpr lrdupTM( ListExpr args );
    template<bool global>
    ListExpr drellgroupbyTM( ListExpr args );

    template<class R, bool setDType>
    int dreldmapNewVMT( 
        Word* args, Word& result, int message, Word& local, Supplier s );

/*
1.1 Type Mappings for all global operators

1.1.1 Type Mapping ~sortTM~

*/
    ListExpr sortTM( ListExpr args ) {

        string err = "d[f]rel(X) expected";

        ListExpr local = lsortTM( args );

        if( !nl->HasLength( local, 3 ) ) {
            return local;
        }

        ListExpr darrayType;
        distributionType dType;
        int attr, key;
        if( !DRelHelpers::drelCheck( 
            nl->First( nl->First( args ) ), darrayType, dType, attr, key ) ) {
            return listutils::typeError(
                err + ": first argument is not a d[f]rel" );
        }

        ListExpr relType = nl->Second( darrayType );
        ListExpr firstAttr = nl->First( nl->Second( nl->Second( relType ) ) );
        ListExpr attrName = nl->First( firstAttr );
        ListExpr attrType = nl->Second( firstAttr );

        // Compare the first attribute with the current partion of the drel
        // If the attribute match with the distributtion attribute only lsortby
        // will be necessary.
        string funText1, funText2;
        bool repartition;
        bool rangeRepartition = true;

        string boundName = "";

        if( dType == replicated || ( dType == range && attr == 0 ) ) {

            repartition = false;
            funText1 = nl->TextValue( nl->First( nl->Second( local ) ) );
            funText2 = nl->TextValue( nl->Second( nl->Second( local ) ) );

        }
        else {

            repartition = true;
            funText1 = "(areduce ";
            funText2 = " \"\" (fun (elem_1 AREDUCEARG1) "
                    "(sort (feed elem_1))) 1238)";
            
            if( dType == spatial2d || dType == spatial3d ) {

                ListExpr attrList = nl->Second( nl->Second( relType ) );
                attrList = DRelHelpers::removeAttrFromAttrList( 
                    attrList, "Original" );
                attrList = DRelHelpers::removeAttrFromAttrList( 
                    attrList, "Cell" );
                relType = nl->TwoElemList(
                    listutils::basicSymbol<Relation>( ),
                    nl->TwoElemList(
                        listutils::basicSymbol<Tuple>( ),
                        attrList ) );
            }
        }

        ListExpr resultType = 
            repartition ? 
            nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                relType,
                nl->FourElemList(
                        nl->IntAtom( range ),
                        nl->IntAtom( 0 ),
                        nl->IntAtom( rand( ) ),
                        nl->TwoElemList(
                            nl->SymbolAtom( Vector::BasicType( ) ),
                            attrType ) ) ) : 
            nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                relType, 
                nl->Third( nl->First( nl->First( args ) ) ) );
                    

        ListExpr appendList = nl->FiveElemList(
            nl->BoolAtom( repartition ),
            nl->BoolAtom( rangeRepartition ),
            nl->StringAtom( nl->SymbolValue( attrName ) ),
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            resultType );
    }

/*
1.1.2 Type Mapping ~sortbyTM~

*/
    ListExpr sortbyTM( ListExpr args ) {

        string err = "d[f]rel(X) x attrlist expected";

        ListExpr local = lsortbyTM( args );

        if( !nl->HasLength( local, 3 ) ) {
            return local;
        }

        ListExpr darrayType;
        distributionType dType;
        int attr, key;
        if( !DRelHelpers::drelCheck( 
            nl->First( nl->First( args ) ), darrayType, dType, attr, key ) ) {
            return listutils::typeError(
                err + ": first argument is not a d[f]rel" );
        }

        ListExpr relType = nl->Second( darrayType );
        string attrName = nl->SymbolValue( 
            nl->First( nl->Second( nl->Second( args ) ) ) );
        ListExpr attrType;

        int pos = listutils::findAttribute( 
            nl->Second( nl->Second( relType ) ), 
            attrName, attrType ) - 1;

        // Compare the first attribute with the current partion of the drel
        // If the attribute match with the distributtion attribute only lsortby
        // will be necessary.
        string funText1, funText2;
        string repartitionText1, repartitionText2;
        bool repartition;
        bool rangeRepartition = true;

        if( dType == replicated || ( dType == range && attr == pos ) ) {

            repartition = false;
            funText1 = nl->TextValue( nl->First( nl->Second( local ) ) );
            funText2 = nl->TextValue( nl->Second( nl->Second( local ) ) );

        }
        else {

            repartition = true;
            string attrList = nl->ToString( nl->Second( nl->Second( args ) ) );

            funText1 = "(areduce ";
            funText2 = " \"\" (fun (elem_1 AREDUCEARG1) "
                "(sortby (feed elem_1)" + attrList + ")) 1238)";

            if( dType == spatial2d || dType == spatial3d ) {
                ListExpr attrList = nl->Second( nl->Second( relType ) );
                attrList = DRelHelpers::removeAttrFromAttrList( 
                    attrList, "Original" );
                attrList = DRelHelpers::removeAttrFromAttrList( 
                    attrList, "Cell" );
                relType = nl->TwoElemList(
                    listutils::basicSymbol<Relation>( ),
                    nl->TwoElemList(
                        listutils::basicSymbol<Tuple>( ),
                        attrList ) );
            }
        }

        ListExpr resultType = 
            repartition ? 
            nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                relType,
                nl->FourElemList(
                        nl->IntAtom( range ),
                        nl->IntAtom( 0 ),
                        nl->IntAtom( rand( ) ),
                        nl->TwoElemList(
                            nl->SymbolAtom( Vector::BasicType( ) ),
                            attrType ) ) ) : 
            nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                relType, 
                nl->Third( nl->First( nl->First( args ) ) ) );

        ListExpr appendList = nl->FiveElemList(
            nl->BoolAtom( repartition ),
            nl->BoolAtom( rangeRepartition ),
            nl->StringAtom( attrName ),
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            resultType );
    }

/*
1.1.3 Type Mapping ~drelgroupbyTM~

*/
    ListExpr drelgroupbyTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "drelgroupbyTM" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        string err = "d[f]rel(X) x attrlist expected";

        ListExpr local = drellgroupbyTM<true>( args );

        if( !nl->HasLength( local, 3 ) ) {
            return local;
        }

        ListExpr darrayType;
        distributionType dType;
        int attr, key;
        if( !DRelHelpers::drelCheck( 
            nl->First( nl->First( args ) ), darrayType, dType, attr, key ) ) {
            return listutils::typeError(
                err + ": first argument is not a d[f]rel" );
        }

        ListExpr relType = nl->Second( darrayType );
        string attrName = nl->SymbolValue( 
            nl->First( nl->Second( nl->Second( args ) ) ) );
        ListExpr attrType;

        int pos = listutils::findAttribute( 
            nl->Second( nl->Second( relType ) ), 
            attrName, attrType ) - 1;

        // Compare the first attribute with the current partion of the drel
        // If the attribute match with the distributtion attribute only lsortby
        // will be necessary.
        string funText1, funText2;
        string repartitionText1, repartitionText2;
        bool repartition;
        bool rangeRepartition = false;

        if( dType == replicated || ( dType == range && attr == pos ) ) {

            repartition = false;
            funText1 = nl->TextValue( nl->First( nl->Second( local ) ) );
            funText2 = nl->TextValue( nl->Second( nl->Second( local ) ) );

        }
        else {

            ListExpr tempfun;
            ListExpr groupfun = nl->TheEmptyList( );
            ListExpr temp = nl->Second( nl->Third( args ) );
            while( !nl->IsEmpty( temp ) ) {

                if( !nl->HasLength( nl->First( temp ), 2 ) ) {
                    return listutils::typeError( "internal Error" );
                }

                if( !DRelHelpers::replaceDRELFUNARG(
                    nl->Second( nl->First( temp ) ),
                    "GROUP", tempfun ) ) {
                    return listutils::typeError( err +
                        ": error in the function format" );
                }

                if( nl->IsEmpty( groupfun ) ) {
                    groupfun = nl->OneElemList( nl->TwoElemList(
                        nl->First( nl->First( temp ) ), tempfun ) );
                } else {
                    groupfun = listutils::concat( groupfun,
                        nl->OneElemList( nl->TwoElemList(
                            nl->First( nl->First( temp ) ), tempfun ) ) );
                }

                temp = nl->Rest( temp );
            }

            ListExpr attrList = nl->Second( nl->Second( args ) );
            repartition = true;
            cout << "repartition true" << endl;
            if( dType == spatial2d || dType == spatial3d ) {
                funText1 = "(areduce (partitionF ";
                funText2 = " \"\" (fun (elem1_2 FFR) (elem2_2 FFR) "
                    "(remove (filter (feed elem1_2) "
                    "(fun (streamelem_3 STREAMELEM) "
                    "(= (attr streamelem_3 Original) TRUE))) "
                    "(Original Cell))) (fun (elem1_4 FFR) (elem2_5 FFR) "
                    "(hashvalue (attr elem2_5 Name) 9999)) "
                    "0) \"\" (fun (elem_6 AREDUCEARG1) (groupby "
                    "(feed elem_6) " + nl->ToString( attrList ) + " "
                    + nl->ToString( groupfun ) + " )) 1238)";

                ListExpr attrList = nl->Second( nl->Second( relType ) );
                attrList = DRelHelpers::removeAttrFromAttrList( 
                    attrList, "Original" );
                attrList = DRelHelpers::removeAttrFromAttrList( 
                    attrList, "Cell" );
                relType = nl->TwoElemList(
                    listutils::basicSymbol<Relation>( ),
                    nl->TwoElemList(
                        listutils::basicSymbol<Tuple>( ),
                        attrList ) );
            }
            else {
                funText1 = "(areduce (partition ";
                funText2 = " \"\" (fun (elem_1 SUBSUBTYPE1) "
                    "(hashvalue (attr elem_1 PLZ) 9999)) 0) "
                    "\"\" (fun (elem_2 AREDUCEARG1) (groupby "
                    "(feed elem_2)" + nl->ToString( attrList ) + " " + 
                    nl->ToString( groupfun ) + ")) 1238)";
            }
        }

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr resultType = 
            repartition ? 
            nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                relType,
                nl->TwoElemList(
                        nl->IntAtom( hash ),
                        nl->IntAtom( 0 ) ) ) : 
            nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                relType, 
                nl->Third( nl->First( nl->First( args ) ) ) );

        ListExpr appendList = nl->FiveElemList(
            nl->BoolAtom( repartition ),
            nl->BoolAtom( rangeRepartition ),
            nl->StringAtom( attrName ),
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            resultType );
    }

/*
1.1.4 Type Mapping ~rdupTM~

*/
    ListExpr rdupTM( ListExpr args ) {

        string err = "d[f]rel(X) expected";

        ListExpr local = lrdupTM( args );

        if( !nl->HasLength( local, 3 ) ) {
            return local;
        }

        ListExpr darrayType;
        distributionType dType;
        int attr, key;
        if( !DRelHelpers::drelCheck( 
            nl->First( nl->First( args ) ), darrayType, dType, attr, key ) ) {
            return listutils::typeError(
                err + ": first argument is not a d[f]rel" );
        }

        ListExpr relType = nl->Second( darrayType );
        ListExpr firstAttr = nl->First( nl->Second( nl->Second( relType ) ) );
        ListExpr attrName = nl->First( firstAttr );

        // Compare the first attribute with the current partion of the drel
        // If the attribute match with the distributtion attribute only lsortby
        // will be necessary.
        string funText1, funText2;
        bool repartition;
        bool rangeRepartition = false;

        string boundName = "";

        if( dType == replicated || dType == hash 
         || ( dType == range && attr == 0 ) ) {

            repartition = false;
            funText1 = nl->TextValue( nl->First( nl->Second( local ) ) );
            funText2 = nl->TextValue( nl->Second( nl->Second( local ) ) );

        }
        else {

            repartition = true;
            if( dType == spatial2d || dType == spatial3d ) {
                funText1 = "(areduce (partitionF ";
                funText2 = " \"\" (fun (elem1_1 FFR) (elem2_2 FFR) "
                    "(remove (filter (feed elem1_1) "
                    "(fun (streamelem_3 STREAMELEM) "
                    "(= (attr streamelem_3 Original) TRUE))) "
                    "(Original Cell))) (fun (elem1_4 FFR) (elem2_5 FFR) "
                    "(hashvalue (attr elem2_5 Name) 9999)) "
                    "0) \"\" (fun (elem_6 AREDUCEARG1) (rdup "
                    "(feed elem_6))) 1238)";

                ListExpr attrList = nl->Second( nl->Second( relType ) );
                attrList = DRelHelpers::removeAttrFromAttrList( 
                    attrList, "Original" );
                attrList = DRelHelpers::removeAttrFromAttrList( 
                    attrList, "Cell" );
                relType = nl->TwoElemList(
                    listutils::basicSymbol<Relation>( ),
                    nl->TwoElemList(
                        listutils::basicSymbol<Tuple>( ),
                        attrList ) );
            }
            else {
                funText1 = "(areduce (partition ";
                funText2 = " \"\" (fun (elem_1 SUBSUBTYPE1) "
                    "(hashvalue (attr elem_1 PLZ) 9999)) 0) "
                    "\"\" (fun (elem_2 AREDUCEARG1) "
                    "(rdup (feed elem_2))) 1238)";
            }
        }

        ListExpr resultType = 
            repartition ? 
            nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                relType,
                nl->TwoElemList(
                        nl->IntAtom( hash ),
                        nl->IntAtom( 0 ) ) ) :
            nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                relType, 
                nl->Third( nl->First( nl->First( args ) ) ) );

        ListExpr appendList = nl->FiveElemList(
            nl->BoolAtom( repartition ),
            nl->BoolAtom( rangeRepartition ),
            nl->StringAtom( nl->SymbolValue( attrName ) ),
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            resultType );
    }

/*
1.2 Value Mapping ~dreldmapVMT~

Uses a d[f]rel and creates a new drel. The d[f]rel is created by 
repartitioning the d[f]rel and execute a function on the d[f]rel.

*/
    template<class R, class T>
    int drelglobaldmapVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        int x = qp->GetNoSons( s );

        distributionType dType;
        int attr, key;
        DFRel::checkDistType( 
            nl->Third( qp->GetType( s ) ), dType, attr, key );

        R* drel = ( R* )args[ 0 ].addr;
        bool repartition = ( ( CcBool* )args[ x - 5 ].addr )->GetValue( );
        bool repartitionRange = ( ( CcBool* )args[ x - 4 ].addr )->GetValue( );

        // call other valuemapping to execute the query, if no boundaries 
        // are necessary
        if( !repartition || !repartitionRange ) {
            dreldmapNewVMT<R, false>( args, result, message, local, s );

            // if repartitioning is used set the disttype
            if( repartition ) {
                ( ( DFRel* )result.addr )->setDistType( 
                    new DistTypeHash( dType, attr ) );
            }
            else {
                // no repartitioning, use the distype of the source drel
                ( ( DFRel* )result.addr )->setDistType( 
                    drel->getDistType( )->copy( ) );
            }
            return 0;
        }

        ListExpr drelType = qp->GetType( qp->GetSon( s, 0 ) );
        ListExpr boundaryType = nl->Fourth( nl->Third( qp->GetType( s ) ) );

        string attrName = ( ( CcString* )args[ x - 3 ].addr )->GetValue( );
        FText* fun1 = ( FText* )args[ x - 2 ].addr;
        FText* fun2 = ( FText* )args[ x - 1 ].addr;

        result = qp->ResultStorage( s );
        DFRel* resultDFRel = ( DFRel* )result.addr;

        if( !drel->IsDefined( ) ) {
            resultDFRel->makeUndefined( );
            return 0;
        }
        
        // compute the boundary
        BoundaryCalculator<R>* calc = new BoundaryCalculator<R>( 
                attrName, boundaryType, new R( *drel ), drelType, 1238 );

        if( !calc->computeBoundary( ) ){
            resultDFRel->makeUndefined( );
            return 0;
        
        }

        collection::Collection* boundary = calc->getBoundary( );
        delete calc;

        // create a dfmatrix of the d[f]rel
        Partitioner<R, T>* parti = new Partitioner<R, T>( attrName, 
            boundaryType, drel, drelType, boundary, 1238 );

        if( !parti->repartition2DFMatrix( ) ) {
            cout << "repartition failed!!" << endl;
            result = qp->ResultStorage( s );
            ( ( DFRel* )result.addr )->makeUndefined( );
            return 0;
        }

        DFMatrix* matrix = parti->getDFMatrix( );
        ListExpr matrixType = parti->getMatrixType( );

        // create dmap call with drel pointer
        string matrixptr = nl->ToString( DRelHelpers::createPointerList( 
            matrixType, matrix ) );

        string funText = fun1->GetValue( ) + matrixptr + fun2->GetValue( );

        #ifdef DRELDEBUG
        cout << "funText" << endl;
        cout << funText << endl;
        #endif

        ListExpr funList;
        if( !nl->ReadFromString( funText, funList ) ) {
            resultDFRel->makeUndefined( );
        }

        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        string typeString, errorString;
        Word dmapResult;
        if( !QueryProcessor::ExecuteQuery( funList, dmapResult, 
                typeString, errorString,
                correct, evaluable, defined, isFunction ) ) {
            resultDFRel->makeUndefined( );
            return 0;
        }
        
        if( !correct || !evaluable || !defined ) {
            resultDFRel->makeUndefined( );
            return 0;
        }

        DFArray* dfarray = ( DFArray* )dmapResult.addr;
        if( !dfarray->IsDefined( ) ) {
            resultDFRel->makeUndefined( );
            delete dfarray;
            return 0;
        }

        resultDFRel->copyFrom( *dfarray );

        delete dfarray;
        delete parti;

        resultDFRel->setDistType( 
            new DistTypeRange( dType, attr, key, boundary ) );

        return 0;
    }


/*
1.3 ValueMapping Array for dmap
    
Used by the operators with only a drel input.

*/
    ValueMapping drelglobaldmapVM[ ] = {
        drelglobaldmapVMT<DRel, DArray>,
        drelglobaldmapVMT<DFRel, DFArray>
    };

/*
1.4 Selection function for dreldmap

Used to select the right position of the parameters. It is necessary, 
because the dmap-Operator ignores the second parameter. So so parameters 
must be moved to the right position for the dmap value mapping.

*/
    int drelglobaldmapSelect( ListExpr args ) {

        return DRel::checkType( nl->First( args ) ) ? 0 : 1 ;
    }

/*
1.5 Specification for all operators using dmapVM

1.5.7 Specification of rdup

Operator specification of the rdup operator.

*/
    OperatorSpec rdupSpec(
        " d[f]rel(X) "
        "-> d[f]rel(X) ",
        " _ rdup",
        "Removes duplicates in a d[f]rel. "
        "NOTE: Duplicates are only removed from the global d[f]rel and a "
        "repartition may be done.",
        " query drel1 rdup"
    );

/*
1.5.8 Specification of sort

Operator specification of the sort operator.

*/
    OperatorSpec sortSpec(
        " d[f]rel(X) "
        "-> d[f]rel(X) ",
        " _ sort",
        "Sorts a d[f]rel. "
        "NOTE: The operator only sorts the global d[f]rel and a "
        "repartition may be done.",
        " query drel1 sort"
    );
    
/*
1.5.9 Specification of drelgroupby

Operator specification of the drelgroupby operator.

*/
    OperatorSpec drelgroupbySpec(
        " d[f]rel(X) x attrlist x funlist "
        "-> d[f]rel(X) ",
        " _ drelgroupby[attrlist, funlist]",
        "Groups a d[f]rel according to attributes "
        "ai1, ..., aik and feeds the groups to other "
        "functions. The results of those functions are "
        "appended to the grouping attributes. The empty "
        "list is allowed for the grouping attributes (this "
        "results in a single group with all input tuples)."
        "NOTE: The operator groups the global d[f]rel and a "
        "repartition may be done.",
        " query drel1 drelgroupby[PLZ; Anz: group feed count]"
    );

/*
1.5.10 Specification of sortby

Operator specification of the sortby operator.

*/
    OperatorSpec sortbySpec(
        " d[f]rel(X) "
        "-> d[f]rel(X) ",
        " _ sortby[attrlist]",
        "Sorts a d[f]rel by a specific attribute list. "
        "NOTE: The operator sorts the global d[f]rel and a "
        "repartition may be done.",
        " query drel1 sortby[PLZ]"
    );

/*
1.6 Operator instance of operators using dmapVM

1.6.7 Operator instance of rdup operator

*/
    Operator rdupOp(
        "rdup",
        rdupSpec.getStr( ),
        2,
        drelglobaldmapVM,
        drelglobaldmapSelect,
        rdupTM
    );
    
/*
1.6.8 Operator instance of sort operator

*/
    Operator sortOp(
        "sort",
        sortSpec.getStr( ),
        2,
        drelglobaldmapVM,
        drelglobaldmapSelect,
        sortTM
    );
    
/*
1.6.9 Operator instance of drelgroupby operator

*/
    Operator drelgroupbyOp(
        "drelgroupby",
        drelgroupbySpec.getStr( ),
        2,
        drelglobaldmapVM,
        drelglobaldmapSelect,
        drelgroupbyTM
    );

/*
1.6.10 Operator instance of sortby operator

*/
    Operator sortbyOp(
        "sortby",
        sortbySpec.getStr( ),
        2,
        drelglobaldmapVM,
        drelglobaldmapSelect,
        sortbyTM
    );

} // end of namespace drel
