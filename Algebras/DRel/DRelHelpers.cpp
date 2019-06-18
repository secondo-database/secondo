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


1 ~DRelHelpers~

Class with usefull helper functions for the DRelAlgebra

*/
//#define DRELDEBUG

#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "ListUtils.h"
#include "StandardTypes.h"
#include "QueryProcessor.h"

#include "DRelHelpers.h"
#include "DRel.h"
#include "Partitioner.hpp"

using namespace std;

namespace drel {

/*
1.1 ~isListOfTwoElemLists~

Returns true if the list is a list of lists with two elements.

*/
    bool DRelHelpers::isListOfTwoElemLists( ListExpr list ) {
        while( !nl->IsEmpty( list ) ) {
            if( !nl->HasLength( nl->First( list ), 2 ) ) {
                return false;
            }
            list = nl->Rest( list );
        }
        return true;
    }

/*
1.2 ~isDRelDescr~

Returns true if the argument is a correct d[f]rel description. The caller 
get the corresponding drelType (drel or dfrel), the relation type 
(for example ( rel( tuple( ( PLZ int ) ) ) )  ), the distribution type 
as a list and the value (for example the name of the drel.

*/
    bool DRelHelpers::isDRelDescr( ListExpr arg, ListExpr& drelType, 
        ListExpr& relType, ListExpr& distType, ListExpr& drelValue ) {

        #ifdef DRELDEBUG
        cout << "isDRelDescr" << endl;
        cout << nl->ToString( arg ) << endl;
        #endif  

        ListExpr type;
        if( nl->HasLength( arg, 1 ) ) {
            type = arg;
            drelValue = nl->TheEmptyList( );
        }
        else if( nl->HasLength( arg, 2 ) ) {
            type = nl->First( arg );
            drelValue = nl->Second( arg );
        }
        else {
            return false;
        }

        if( !DRel::checkType( type )
         && !DFRel::checkType( type ) ) {
            #ifdef DRELDEBUG
            cout << "keine drel" << endl;
            #endif

            return false;
        }
        drelType = type;

        if( !Relation::checkType( nl->Second( drelType ) ) ) {
            #ifdef DRELDEBUG
            cout << "keine relation" << endl;
            #endif
            return false;
        }

        relType = nl->Second( drelType );
        distType = nl->Third( drelType );

        return true;
    }

/*
1.3 ~isDRelDescr~

Returns true if the argument is a correct d[f]rel description. The caller
get the corresponding drelType (drel or dfrel), the relation type
(for example ( rel( tuple( ( PLZ int ) ) ) )  ) and the distribution type
as a list.

*/
    bool DRelHelpers::isDRelDescr( ListExpr arg, ListExpr& drelType, 
        ListExpr & relType, ListExpr& distType, ListExpr & drelValue,
        ListExpr & darrayType ) {

        if( !isDRelDescr( arg, drelType, relType, distType, drelValue ) ) {
            return false;
        }

        if( nl->SymbolValue( nl->First( drelType ) ) == DRel::BasicType( ) ) {
            darrayType = nl->TwoElemList(
                listutils::basicSymbol<distributed2::DArray>( ),
                relType );
        }
        else {
            darrayType = nl->TwoElemList(
                listutils::basicSymbol<distributed2::DFArray>( ),
                relType );
        }

        return true;
    }
    
/*
1.4 ~isDRelDescr~

To call the type mappings of the Distributed2Algebra it is nessesarry to 
replace the type operator DRELFUNARG of the DRelAlgebra with the type 
operator of the Distributed2Algebra. This is what this function does.
The type operator is given as a string. You get a new function as a 
nested list if the argument is correct.

*/
    bool DRelHelpers::replaceDRELFUNARG( 
        ListExpr arg, string type, ListExpr& fun ) {

        if( !nl->HasLength( arg, 3 ) ) {
            return false;
        }
        if( !nl->HasLength( nl->Second( arg ), 2 ) ) {
            return false;
        }
        if( !nl->IsAtom( nl->Second( nl->Second( arg ) ) ) ) {
            return false;
        }
        if( nl->AtomType( nl->Second( nl->Second( arg ) ) ) != SymbolType ) {
            return false;
        }

        fun = nl->ThreeElemList(
            nl->First( arg ),
            nl->TwoElemList(
                nl->First( nl->Second( arg ) ),
                nl->SymbolAtom( type ) ),
            nl->Third( arg ) );

        return true;
    }

/*
1.5 ~isDRelDescr~

To call the type mappings of the Distributed2Algebra it is nessesarry to
replace the type operator DRELFUNARG of the DRelAlgebra with the type
operator of the Distributed2Algebra. This is what this function does.
The type operator is given as a string. You get a new function as a
nested list if the argument is correct.
It is the same like the function above, but now you get the new function 
devided by function and mapping for the function.

*/
    bool DRelHelpers::replaceDRELFUNARG( 
        ListExpr arg, string type, ListExpr& fun, ListExpr& map ) {

        if( !nl->HasLength( arg, 2 ) ) {
            return false;
        }
        if( !replaceDRELFUNARG( nl->Second( arg ), type, fun ) ) {
            return false;
        }

        map = nl->First( arg );
        return true;
    }

/*
1.6 ~computeSampleSize~

Computes the sample size for a total size of a relation.
Attention: totalSize has to be bigger than 0.

*/
    int DRelHelpers::computeSampleSize( const int totalSize ) {

        assert( totalSize > 0 );

        if( totalSize <= 5000 ) {
            return totalSize;
        }

        int sampleSize = totalSize / 1000;

        return sampleSize < 5000 ? 5000 : sampleSize;
    }

/*
1.7 ~everyNthTupleForSample~

The minimum size of a sample is 5000 or 1/1000 of the ~totalSize~.
The returned number said that every nth tuple has to be part of the
sample.
Attention: totalSize has to be bigger than 0.

*/
    int DRelHelpers::everyNthTupleForSample( const int totalSize ) {

        assert( totalSize > 0 );

        return totalSize / computeSampleSize( totalSize );
    }

/*
1.8 ~everyNthTupleForSample~

The minimum size of a sample is 5000 or 1/1000 of the ~totalSize~.
The returned number said that every nth tuple has to be part of the
sample.
Attention: sampleSize and totalSize have to be bigger than 0.

*/
    int DRelHelpers::everyNthTupleForSample(
        const int sampleSize, const int totalSize ) {

        assert( sampleSize > 0 );
        assert( totalSize > 0 );

        return totalSize / sampleSize;
    }

/*
1.9 ~everyNthTupleForArray~

The sample uses every nth tuple of a relation. To fill the array only
every nth Tuple of the sample can be used, because normaly a sample is
much bigger than the array.
Attention: sampleSize and arraySize have to be bigger than 0.

*/
    int DRelHelpers::everyNthTupleForArray(
        const int sampleSize, const int arraySize ) {

        assert( sampleSize > 0 );
        assert( arraySize > 0 );

        if( arraySize >= sampleSize ) {
            return 1;
        }

        return sampleSize / arraySize;
    }

/*
1.10 ~compareAttributes~

Compares two attributes by calling the compare function of the attributes.
returns true, if the first argument is smaller than the second argument.
The defintion of the compare attribute can be found in the attribute 
implementation.

*/
    bool DRelHelpers::compareAttributes(
        const Attribute* attr1, const Attribute* attr2 ) {

        return attr1->Compare( attr2 ) == -1;
    }

/*
1.11 ~listOfIntAtoms~

Checks a list to be a list of int atoms.

*/
    bool DRelHelpers::listOfIntAtoms( ListExpr list ) {

        if( nl->IsAtom( list ) ) {
            return false;
        }

        while( nl->IsEmpty( list ) ) {

            if( !nl->IsAtom( nl->First( list ) )
             || !nl->AtomType( nl->First( list ) ) ) {
                return false;
            }

        }

        return true;
    }

/*
1.12 ~getIndex~

Search the right index in a vector for a given attribute. This is a seqential 
search.

*/
    int DRelHelpers::getIndex( 
        collection::Collection* vector, Attribute* attr ) {

        int count = vector->GetNoComponents( );

        int index = 0;
        while( index < count - 1 ) {
            Attribute* vi = vector->GetComponent( index );
            if( attr->Compare( vi ) <= 0 ) {
                delete vi;
                return index;
            }
            delete vi;
            index++;
        }

        return index;
    }

/*
1.13 ~countDRel~

Computes the number of tuple in the given drel.

*/
    int DRelHelpers::countDRel( string drel ) {

        cout << endl;
        cout << "Start: Compute the size of the drel ..." << endl;

        int count = -1;

        string query =
        "(tie (getValue (dmap (drel2darray " + drel + ")"
        " \"\" (fun (dmapelem1 ARRAYFUNARG1) (count dmapelem1)))) "
        "(fun (first2 ELEMENT) (second3 ELEMENT) (+first2 second3)))";

        Word result;
        if( !QueryProcessor::ExecuteQuery( query, result ) ) {
            cout << "ERROR: Computation of the drel size failed!" << endl;
            return count;
        }

        CcInt* res = ( CcInt* )result.addr;

        if( !res ) {
            cout << "ERROR: Computation of the drel size failed!" << endl;
            return count;
        }

        if( !res->IsDefined( ) ) {
            cout << "ERROR: Computation of the drel size failed!" << endl;
            delete res;
            return count;
        }

        count = res->GetValue( );
        delete res;

        cout << "Done. DRel size: " + to_string( count ) << endl;

        return count;
    }

/*
1.14 ~drelCheck~

Checks if the list is a drel or dfrel expression.

*/
    bool DRelHelpers::drelCheck( ListExpr list ) {

        distributionType type;
        int attr, key;
        ListExpr darray;

        return drelCheck( list, darray, type, attr, key );
    }

/*
1.15 ~drelCheck~

Checks if the list is a drel or dfrel expression. Second argument will be 
the corresponding darray or dfarray expression.

*/
    bool DRelHelpers::drelCheck( ListExpr list, ListExpr& darray ) {

        distributionType type;
        int attr, key;

        return drelCheck( list, darray, type, attr, key );
    }

/*
1.16 ~drelCheck~

Checks if the list is a drel or dfrel expression. Second argument will be 
the used distributionType of the d[f]rel.

*/
    bool DRelHelpers::drelCheck( ListExpr list, distributionType& type ) {

        int attr, key;
        ListExpr darray;

        return drelCheck( list, darray, type, attr, key );
    }

/*
1.17 ~drelCheck~

Checks if the list is a drel or dfrel expression. The other arguments are the 
corresponding d[f]array expression, the used distributionType, the attribute 
number and the key. If the attribute nummber or the key are not used in the 
d[f]rel, they are -1.

*/
    bool DRelHelpers::drelCheck( ListExpr list, ListExpr& darray, 
        distributionType& type, int& attr, int& key ) {

        if( DRel::checkType( list, type, attr, key ) ) {
            darray = nl->TwoElemList(
                listutils::basicSymbol<distributed2::DArray>( ),
                nl->Second( list ) );
        }
        else if( DFRel::checkType( list, type, attr, key ) ) {
            darray = nl->TwoElemList(
                listutils::basicSymbol<distributed2::DFArray>( ),
                nl->Second( list ) );
        }
        else {
            return false;
        }

        return true;
    }

/*
1.17 ~repartition4JoinRequired~

Tests if for a join of two d[f]rels a repatition is requiered.

*/
    ListExpr DRelHelpers::repartition4JoinRequired( 
        ListExpr drel1Value, ListExpr drel2Value,
        ListExpr attr1Name, ListExpr attr2Name,
        distributionType dType1, distributionType dType2, 
        ListExpr attr1List, ListExpr attr2List, 
        const int dAttr1, const int dAttr2, 
        const int dKey1, const int dKey2,
        bool& drel1reparti, bool& drel2reparti ) {

        assert( listutils::isAttrList( attr1List ) );
        assert( listutils::isAttrList( attr2List ) );
        assert( dType1 != replicated || dType2 != replicated );

        cout << "repartition4JoinRequired" << endl;

        ListExpr attrType;
        int attrPos1 = listutils::findAttribute( attr1List, 
            nl->SymbolValue( attr1Name ), attrType );
        int attrPos2 = listutils::findAttribute( attr2List, 
            nl->SymbolValue( attr2Name ), attrType );

        ListExpr resultDistType;

        if( dType1 == hash && attrPos1 - 1 == dAttr1 ) {

            if( dType2 == hash && attrPos2 - 1 == dAttr2 ) {
                drel1reparti = false;
                drel2reparti = false;
                resultDistType = nl->TwoElemList(
                    nl->IntAtom( hash ),
                    nl->IntAtom( attrPos1 - 1 ) );
            }
            else if( dType2 == range && attrPos2 - 1 == dAttr2 ) {

                int count1 = DRelHelpers::countDRel( 
                    nl->ToString( drel1Value ) );
                int count2 = DRelHelpers::countDRel( 
                    nl->ToString( drel2Value ) );

                if( count1 <= count2 ) {
                    drel1reparti = false;
                    drel2reparti = true;
                    resultDistType = nl->TwoElemList(
                        nl->IntAtom( hash ),
                        nl->IntAtom( attrPos1 - 1 ) );

                    cout << "count1 <= count2 " << endl;
                }
                else {
                    cout << "dType1 == hash dType2 == range" << endl;
                    drel1reparti = true;
                    drel2reparti = false;
                    resultDistType = nl->FourElemList(
                        nl->IntAtom( range ),
                        nl->IntAtom( nl->ListLength( attr1List ) 
                                        + attrPos2 - 1 ),
                        nl->IntAtom( dKey2 ),
                        nl->TwoElemList(
                            nl->SymbolAtom( Vector::BasicType( ) ),
                            attrType ) );
                }
            }
            else {
                drel1reparti = false;
                drel2reparti = true;
                resultDistType = nl->TwoElemList(
                    nl->IntAtom( hash ),
                    nl->IntAtom( attrPos1 - 1 ) );
            }
        }
        else if( dType1 == range && attrPos1 - 1 == dAttr1 ) {

            if( dType2 == hash && attrPos2 - 1 == dAttr2 ) {

                int count1 = DRelHelpers::countDRel( 
                    nl->ToString( drel1Value ) );
                int count2 = DRelHelpers::countDRel( 
                    nl->ToString( drel2Value ) );

                if( count1 <= count2 ) {
                    drel1reparti = false;
                    drel2reparti = true;
                    resultDistType = nl->FourElemList(
                        nl->IntAtom( range ),
                        nl->IntAtom( nl->ListLength( attr1List ) 
                                        + attrPos1 - 1 ),
                        nl->IntAtom( dKey1 ),
                        nl->TwoElemList(
                            nl->SymbolAtom( Vector::BasicType( ) ),
                            attrType ) );
                }
                else {
                    drel1reparti = true;
                    drel2reparti = false;
                    resultDistType = nl->TwoElemList(
                        nl->IntAtom( hash ),
                        nl->IntAtom( nl->ListLength( attr1List ) 
                                        + attrPos2 - 1 ) );
                }
            }
            else if( dType2 == range && attrPos2 - 1 == dAttr2 ) {

                if( dKey1 == dKey2 ) {
                    drel1reparti = false;
                    drel2reparti = false;
                    resultDistType = nl->FourElemList(
                        nl->IntAtom( range ),
                        nl->IntAtom( nl->ListLength( attr1List ) 
                                        + attrPos1 - 1 ),
                        nl->IntAtom( dKey1 ),
                        nl->TwoElemList(
                            nl->SymbolAtom( Vector::BasicType( ) ),
                            attrType ) );
                }
                else {

                    int count1 = DRelHelpers::countDRel( 
                        nl->ToString( drel1Value ) );
                    int count2 = DRelHelpers::countDRel( 
                        nl->ToString( drel2Value ) );

                    if( count1 <= count2 ) {
                        drel1reparti = false;
                        drel2reparti = true;
                        resultDistType = nl->FourElemList(
                            nl->IntAtom( range ),
                            nl->IntAtom( nl->ListLength( attr1List ) 
                                            + attrPos1 - 1 ),
                            nl->IntAtom( dKey1 ),
                            nl->TwoElemList(
                                nl->SymbolAtom( Vector::BasicType( ) ),
                                attrType ) );
                    }
                    else {
                        drel1reparti = true;
                        drel2reparti = false;
                        resultDistType = nl->FourElemList(
                            nl->IntAtom( range ),
                            nl->IntAtom( nl->ListLength( attr1List ) 
                                            + attrPos2 - 1 ),
                            nl->IntAtom( dKey1 ),
                            nl->TwoElemList(
                                nl->SymbolAtom( Vector::BasicType( ) ),
                                attrType ) );
                    }
                }
            }
            else {
                drel1reparti = false;
                drel2reparti = true;
                resultDistType = nl->FourElemList(
                    nl->IntAtom( range ),
                    nl->IntAtom( nl->ListLength( attr1List ) 
                                    + attrPos2 - 1 ),
                    nl->IntAtom( dKey1 ),
                    nl->TwoElemList(
                        nl->SymbolAtom( Vector::BasicType( ) ),
                        attrType ) );
            }
        }
        else if( dType2 == range && attrPos2 - 1 == dAttr2 ) {
            drel1reparti = true;
            drel2reparti = false;
            resultDistType = nl->FourElemList(
                nl->IntAtom( range ),
                nl->IntAtom( nl->ListLength( attr1List ) 
                                + attrPos2 - 1 ),
                nl->IntAtom( dKey2 ),
                nl->TwoElemList(
                    nl->SymbolAtom( Vector::BasicType( ) ),
                    attrType ) );
        }
        else if( dType2 == hash && attrPos2 - 1 == dAttr2 ) {
            drel1reparti = true;
            drel2reparti = false;
            resultDistType = nl->TwoElemList(
                nl->IntAtom( hash ),
                nl->IntAtom( nl->ListLength( attr1List ) 
                                + attrPos2 - 1 ) );
        }
        else {
            drel1reparti = true;
            drel2reparti = true;
            resultDistType = nl->TwoElemList(
                nl->IntAtom( hash ),
                nl->IntAtom( attrPos1 - 1 ) );
        }

        drel1reparti = drel1reparti ? dType1 != replicated : drel1reparti;
        drel2reparti = drel2reparti ? dType2 != replicated : drel2reparti;

        return resultDistType;
    }

/*
1.17 ~removeAttrFromAttrList~

Removes a attribute given as a symbol from an attribute list.

*/
    ListExpr DRelHelpers::removeAttrFromAttrList( 
        ListExpr attrList, ListExpr attr ) {

        assert( nl->IsAtom( attr ) && nl->AtomType( attr ) == SymbolType );

        return removeAttrFromAttrList( attrList, nl->SymbolValue( attr ) );
    }

/*
1.18 ~removeAttrFromAttrList~

Removes a attribute given as a string from an attribute list.

*/
    ListExpr DRelHelpers::removeAttrFromAttrList( 
        ListExpr attrList, string attr ) {

        set<string> names = { attr };

        ListExpr head, last;
        listutils::removeAttributes( attrList, names, head, last );

        return head;
    }

/*
1.19 ~removeAttrFromAttrList~

Removes a attribute list given as a string set from an attribute list.

*/
    ListExpr DRelHelpers::removeAttrFromAttrList( 
        ListExpr attrList, set<string>& names ) {

        ListExpr head, last;
        listutils::removeAttributes( attrList, names, head, last );

        return head;
    }

/*
1.20 ~removePartitionAttributes~

Removes all partition attributes from the attrList. The partition attributes 
may be Original and Cell.

*/
    ListExpr DRelHelpers::removePartitionAttributes( ListExpr attrList, 
        distributionType type ) {

        if( type == replicated ) {
            return removeAttrFromAttrList( attrList, "Original" );
        }
        else if( type == spatial2d || type == spatial3d ) {
            set<string> names = { "Original", "Cell" };
            return removeAttrFromAttrList( attrList, names );
        }

        return attrList;
    }

/*
1.20 ~removePartitionAttributes~

Removes all partition attributes from the attrList. The partition attributes 
may be Original and Cell.

*/
    ListExpr DRelHelpers::addPartitionAttributes( ListExpr attrList ) {

        ListExpr attrType;
        int pos = listutils::findAttribute( 
            attrList, "Cell", attrType );

        if( pos != 0 ) {
            return attrList;
        }

        attrList = ConcatLists( attrList, nl->OneElemList(
            nl->TwoElemList(
                nl->SymbolAtom( "Cell" ),
                listutils::basicSymbol<CcInt>( ) ) ) );

        pos = listutils::findAttribute( 
            attrList, "Original", attrType );

        if( pos != 0 ) {
            return attrList;
        }

        return ConcatLists( attrList, nl->OneElemList(
            nl->TwoElemList(
                nl->SymbolAtom( "Original" ),
                listutils::basicSymbol<CcBool>( ) ) ) );
    }

/*
1.21 ~getRemovePartitonAttr~

Returns the partition attributes for a distributionType.

*/
    ListExpr DRelHelpers::getRemovePartitonAttr( distributionType type ) {

        if( type == replicated ) {
            return nl->OneElemList( 
                nl->SymbolAtom( "Original" ) );
        }

        if( type == spatial2d || type == spatial3d ) {
            return nl->TwoElemList( 
                nl->SymbolAtom( "Original" ), 
                nl->SymbolAtom( "Cell" ) );
        }

        return nl->TheEmptyList( );
    }

/*
1.22 Value Mapping ~createRepartitionQuery~

Uses a d[f]rel and creates a repartition query. For the query the argument 
query is used. The query is a nested list. If the query is used as subquery 
the "elem" in the query has to be unique. Therefor the arument elem, elem2 
and streamelem are used, default is "1". The drelType has to be the nestedlist
type of the drel1. The resultDistType is the requiered distType. If range 
partition is choosen drel2 has to be partitioned by range too. attrName is the
partion attribute. The argument port is used if range partitioning is choosen 
to bring the boundary object to the workers and create a dfmatrix.

*/
    template<class R, class Q, class T>
    bool DRelHelpers::createRepartitionQuery( 
        ListExpr drelType,
        R* drel1, T* drel2,
        ListExpr resultDistType,
        string attrName,
        int port,
        ListExpr& query,
        int elem1, int elem2,
        int streamelem ) {

        distributionType sourceDistType, targetDistType;
        getTypeByNum( nl->IntValue( nl->First( resultDistType ) ),
            targetDistType );

        getTypeByNum( nl->IntValue( nl->First( nl->Third (drelType ) ) ),
            sourceDistType );

        if( targetDistType == range ) {

            Partitioner<R, Q>* parti = new Partitioner<R, Q>( attrName, 
                nl->Fourth( resultDistType ), drel1, drelType, 
                ( ( DistTypeRange* )drel2->getDistType( ) )
                    ->getBoundary( )->Clone( ), 1238 );

            if( !parti->repartition2DFMatrix( ) ) {
                cout << "repartition failed!!" << endl;
                return false;
            }

            distributed2::DFMatrix* matrix = parti->getDFMatrix( );

            delete parti;

            if( !matrix || !matrix->IsDefined( ) ) {
                cout << "repartition failed!!" << endl;
                return false;
            }

            query = nl->FourElemList(
                nl->SymbolAtom( "collect2" ),
                    nl->TwoElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom( distributed2::DFMatrix::BasicType( ) ),
                        nl->TwoElemList( 
                            listutils::basicSymbol<Relation>( ),
                            nl->TwoElemList(
                                listutils::basicSymbol<Tuple>( ),
                                DRelHelpers::removePartitionAttributes(
                                            nl->Second( 
                                                nl->Second( 
                                                    nl->Second( drelType ) ) ),
                                            targetDistType ) ) ),
                        nl->Second( drelType ) ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "ptr" ),
                        listutils::getPtrList( matrix ) ) ),
                nl->StringAtom( "" ),
                nl->IntAtom( 1238 ) );
        }
        else {

            ListExpr drel2darray = nl->TwoElemList(
                nl->SymbolAtom( "drel2darray" ),
                nl->TwoElemList(
                    drelType,
                    nl->TwoElemList(
                        nl->SymbolAtom( "ptr" ),
                        listutils::getPtrList( new R( *drel1 ) ) ) ) );

            if( sourceDistType == replicated 
             || sourceDistType == spatial2d 
             || sourceDistType == spatial3d ) {

                ListExpr removeAttr = DRelHelpers::getRemovePartitonAttr( 
                        sourceDistType );

                query = nl->FourElemList(
                    nl->SymbolAtom( "collect2" ),
                    nl->SixElemList(
                        nl->SymbolAtom( "partitionF" ),
                        drel2darray,
                        nl->StringAtom( "" ),
                        nl->FourElemList(
                            nl->SymbolAtom( "fun" ),
                            nl->TwoElemList( 
                                nl->SymbolAtom( "elem"
                                    + to_string( elem1 ) + "_1" ),
                                nl->SymbolAtom( "FFR" ) ),
                            nl->TwoElemList( 
                                nl->SymbolAtom( "elem"
                                    + to_string( elem2 ) + "_2" ),
                                nl->SymbolAtom( "FFR" ) ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "remove" ),
                                nl->ThreeElemList(
                                    nl->SymbolAtom( "filter" ),
                                    nl->TwoElemList( 
                                        nl->SymbolAtom( "feed" ),
                                        nl->SymbolAtom( "elem"
                                            + to_string( elem1 ) + "_1" ) ),
                                    nl->ThreeElemList(
                                        nl->SymbolAtom( "fun" ),
                                        nl->TwoElemList( 
                                            nl->SymbolAtom( "streamelem_"
                                                + to_string( streamelem ) ),
                                            nl->SymbolAtom( "STREAMELEM" ) ),
                                        nl->ThreeElemList(
                                            nl->SymbolAtom( "=" ),
                                            nl->ThreeElemList(
                                                nl->SymbolAtom( "attr" ),
                                                nl->SymbolAtom( "streamelem_"
                                                    + to_string( streamelem ) ),
                                                nl->SymbolAtom( "Original" ) ),
                                            nl->BoolAtom( true ) ) ) ),
                                removeAttr ) ),
                        nl->FourElemList(
                            nl->SymbolAtom( "fun" ),
                            nl->TwoElemList(
                                nl->SymbolAtom( "elem"
                                    + to_string( elem1 ) + "_4" ),
                                nl->SymbolAtom( "FFR" ) ),
                            nl->TwoElemList(
                                nl->SymbolAtom( "elem"
                                    + to_string( elem2 ) + "_5" ),
                                nl->SymbolAtom( "FFR" ) ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "hashvalue" ),
                                nl->ThreeElemList(
                                    nl->SymbolAtom( "attr" ),
                                    nl->SymbolAtom( "elem" 
                                        + to_string( elem2 ) + "_5" ),
                                    nl->SymbolAtom( attrName ) ),
                                nl->IntAtom( 99999 ) ) ),
                            nl->IntAtom( 0 ) ),
                        nl->StringAtom( "" ),
                        nl->IntAtom( 1238 ) );

            }
            else {
                query = nl->FourElemList(
                    nl->SymbolAtom( "collect2" ),
                    nl->FiveElemList(
                        nl->SymbolAtom( "partition" ),
                        drel2darray,
                        nl->StringAtom( "" ),
                        nl->ThreeElemList(
                            nl->SymbolAtom( "fun" ),
                            nl->TwoElemList(
                                nl->SymbolAtom( "elem_"
                                    + to_string( elem1 ) ),
                                nl->SymbolAtom( "SUBSUBTYPE1" ) ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "hashvalue" ),
                                nl->ThreeElemList(
                                    nl->SymbolAtom( "attr" ),
                                    nl->SymbolAtom( "elem_" 
                                        + to_string( elem1 ) ),
                                    nl->SymbolAtom( attrName ) ),
                                nl->IntAtom( 99999 ) ) ),
                            nl->IntAtom( 0 ) ),
                        nl->StringAtom( "" ),
                        nl->IntAtom( 1238 ) );
            }
        }

        return true;
    }

/*
1.23 ~createPointerList~

Creates a ListExpr for a pointer to a given type.

*/
    ListExpr DRelHelpers::createPointerList( ListExpr type, void* ptr ) {

        return nl->TwoElemList(
            type,
            nl->TwoElemList(
                nl->SymbolAtom( "ptr" ),
                listutils::getPtrList( ptr ) ) );
    }

    ListExpr DRelHelpers::createdrel2darray( ListExpr type, void* ptr ) {

        return nl->TwoElemList(
            nl->SymbolAtom( "drel2darray" ),
            createPointerList( type, ptr ) );
    }

    // function instantiations
    template bool DRelHelpers::createRepartitionQuery<
            DRel, distributed2::DArray, DRel>(
        ListExpr, DRel*, DRel*, ListExpr, string, int, ListExpr&, 
        int, int, int );
    template bool DRelHelpers::createRepartitionQuery<
            DRel, distributed2::DArray, DFRel>(
        ListExpr, DRel*, DFRel*, ListExpr, string, int, ListExpr&, 
        int, int, int );
    template bool DRelHelpers::createRepartitionQuery<
            DFRel, distributed2::DFArray, DRel>(
        ListExpr, DFRel*, DRel*, ListExpr, string, int, ListExpr&, 
        int, int, int );
    template bool DRelHelpers::createRepartitionQuery<
            DFRel, distributed2::DFArray, DFRel>(
        ListExpr, DFRel*, DFRel*, ListExpr, string, int, ListExpr&, 
        int, int, int );

} // end of namespace drel
