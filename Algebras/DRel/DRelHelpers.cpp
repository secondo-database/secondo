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
#define DRELDEBUG

#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "ListUtils.h"
#include "StandardTypes.h"
#include "QueryProcessor.h"

#include "DRelHelpers.h"
#include "DRel.h"

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
        while( index < count ) {
            if( attr->Compare( vector->GetComponent( index ) ) <= 0 ) {
                return index;
            }
            index++;
        }

        return index;
    }

} // end of namespace drel