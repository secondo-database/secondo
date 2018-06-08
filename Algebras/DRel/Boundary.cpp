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
#include "Boundary.h"
#include "Algebras/Distributed2/Dist2Helper.h"
#include "TypeConstructor.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "ConstructorTemplates.h"

extern NestedList* nl;

using namespace std;

namespace drel {

    /*
    1 Implementation of class ~Boundary~

    This class represents a list ob Attributes to define boundarys.

    1.1 Constructors

    */
    Boundary::Boundary( ) :
        refs( 1 ), defined( true ) {
    }

    Boundary::Boundary( const string _attrType ) :
        refs( 1 ), defined( true ), attrType( _attrType ) {
    }

    /*
    1.1 Copyconstructor

    */
    Boundary::Boundary( const Boundary& src ) :
        refs( 1 ), defined( src.defined ), attrType( src.attrType ), 
        boundaryArray( src.boundaryArray ) {
    }

    /*
    1.2 Assignment operator

    */
    Boundary& Boundary::operator=( const Boundary& src ) {
        if( this == &src ) {
            return *this;
        }
        refs = 1;
        defined = src.defined;
        attrType = src.attrType;
        boundaryArray = src.boundaryArray;
        return *this;
    }

    /*
    1.3 Destructor

    */
    Boundary::~Boundary( ) {
        for( vector<Attribute*>::iterator it = boundaryArray.begin( );
            it != boundaryArray.end( ); ++it ) {
            delete ( *it );
        }
        boundaryArray.clear( );
    }

    /*
    1.3 ~isDefined~

    Returns true if the boundary is defined, false otherwise.

    */
    bool Boundary::isDefined( ) {
        return defined;
    }

    /*
    1.4 ~DeleteIfAllowed~

    Calls the destructor if the reference counter is 0.

    */
    bool Boundary::DeleteIfAllowed( ) {
        assert( refs > 0 );
        refs--;
        if( refs == 0 ) {
            delete this;
            return true;
        } else {
            return false;
        }
    }

    /*
    1.5 ~IncReference~

    Increases the reference counter.

    */
    void Boundary::IncReference( ) {
        refs++;
    }

    /*
    1.6 ~getSize~

    Returns the number of attributes in the boundaryarray.

    */
    const size_t Boundary::getSize( ) {
        return boundaryArray.size( );
    }

    vector<Attribute*> Boundary::getArray( ) {
        return boundaryArray;
    }

    const std::string Boundary::getAttrType( ) {
        return attrType;
    }

    /*
    1.7 ~makeUndefined~

    Makes the boundary undefined.

    */
    void Boundary::makeUndefined( ) {
        defined = false;
    }

    /*
    1.8 ~insertBound~

    Insert an attribute as boundary to the array.

    */
    void Boundary::insertBound( Attribute* _attr ) {
        boundaryArray.push_back( _attr );
    }

    /*
    1.9 ~useSampleAsBound~

    Uses a given vector as boundaryarray. This is usefull, if the 
    sample is to small.

    */
    void Boundary::useSampleAsBound( vector<Attribute*> sample ) {
        boundaryArray = sample;
    }

    /*
    1.10 ~toListExpr~

    Returns the DRel as a NestedList.

    */
    ListExpr Boundary::toListExpr( ) {

        ListExpr l = nl->TheEmptyList( );
        ListExpr lastElem = l;
        ListExpr valueList;

        int algebraId;
        int typeId;
        SecondoCatalog* sc = SecondoSystem::GetCatalog( );
        sc->GetTypeId( attrType, algebraId, typeId );

        for( vector<Attribute*>::iterator it = boundaryArray.begin( ); 
            it != boundaryArray.end( ); ++it ) {
        
            valueList = ( am->OutObj( algebraId, typeId ) )
                ( ( nl->TwoElemList( nl->IntAtom( algebraId ), 
                    nl->IntAtom( typeId ) ) ), SetWord( *it ) );

            if( l == nl->TheEmptyList( ) ) {
                l = nl->Cons( valueList, nl->TheEmptyList( ) );
                lastElem = l;
            }
            else {
                lastElem = nl->Append( lastElem, valueList );
            }
        }
        
        return nl->OneElemList( l );
    }

    /*
    1.11 ~save~

    Saves the object. Boundary is a secondo object, so this is the secondo 
    save method.

    */
    bool Boundary::save( SmiRecord & valueRecord, size_t & offset ) {

        if( !distributed2::writeVar( isDefined( ), valueRecord, offset ) ) {
            return false;
        }
        if( !distributed2::writeVar( getSize( ), valueRecord, offset ) ) {
            return false;
        }
            
        return saveAttrList( valueRecord, offset );
    }

    /*
    1.12 ~saveAttrList~

    Saves all attributes of the boundaryarray.

    */
    bool Boundary::saveAttrList( SmiRecord& valueRecord, size_t& offset ) {

        int algebraId;
        int typeId;
        SecondoCatalog* sc = SecondoSystem::GetCatalog( );
        if( ! ( sc->GetTypeId( attrType, algebraId, typeId ) ) ) {
            return false;
        }

        return saveAttrList( valueRecord, offset, 
            nl->TwoElemList( nl->IntAtom( algebraId ), nl->IntAtom( typeId ) ),
            algebraId, typeId );
    }

    /*
    1.13 ~saveAttrList~

    Saves all attributes of the boundaryarray. Calls the algebra manager to 
    call save method of each attribute in the boundary array.

    */
    bool Boundary::saveAttrList( 
        SmiRecord& valueRecord, size_t& offset, 
        const ListExpr attrType, const int algebraId, const int typeId ) {

        for( vector<Attribute*>::iterator it = boundaryArray.begin( ); 
            it != boundaryArray.end( ); ++it ) {
            Word attr( *it );
            if( !( am->SaveObj( algebraId, typeId, 
                valueRecord, offset, attrType, attr ) ) ) {
                return false;
            }
        }
        return true;
    }

    /*
    1.14 ~readFrom~

    Reads a NestedList and returns a Boundary pointer. The pointer is 0 if the
    list has an error.

    */
    Boundary* Boundary::readFrom( ListExpr typeInfo, ListExpr value ) {

        if( listutils::isSymbolUndefined( value ) ) {
            Boundary* boundary = new Boundary( );
            boundary->makeUndefined( );
            return boundary;
        }

        if( ! nl->HasLength( typeInfo, 2 ) ) {
            return 0;
        }

        if( !nl->HasLength( nl->Second( typeInfo ), 2 ) ) {
            return 0;
        }

        ListExpr attrType = nl->First( nl->Second( typeInfo ) );
        int algebraId = nl->IntValue( nl->First( attrType ) );
        int typeId = nl->IntValue( nl->Second( attrType ) );

        if( nl->ListLength( value ) < 1 ) {
            return 0;
        }

        SecondoCatalog* sc = SecondoSystem::GetCatalog( );
        Boundary* boundary = 
            new Boundary( sc->GetTypeName( algebraId, typeId ) );

        int errorPos = 0;
        ListExpr errorInfo;
        bool correct = false;
        while( ! nl->IsEmpty( value ) ) {
            Word attr = ( am->InObj( algebraId, typeId ) )
                ( ( attrType ), value, errorPos, errorInfo, correct );
            if( !correct ) {
                boundary->makeUndefined( );
                return boundary;
            }
            boundary->insertBound( ( Attribute* )attr.addr );
            value = nl->Rest( value );
        }

        return boundary;
    }

    /*
    1.15 ~createBoundary~

    Creates a new Boundary. This is a factory function. The returned boundary 
    is a boundary for the relation rel and the attribute attr. Here the type 
    of the attribute is needed and the size of the array.

    */
    Boundary* Boundary::createBoundary( 
        Relation* _rel, int _attr, string _attrType, int _arraySize ) {

        vector<Attribute*> sample = createSample( _rel, _attr );
        Boundary* boundary = new Boundary( _attrType );

        // sample is smaller than the array
        if( ( unsigned )_arraySize >= sample.size( ) ) {
            boundary->useSampleAsBound( sample );
            return boundary;
        }

        int nth = everyNthTupleForArray( sample.size( ), _arraySize );
        int i = 1;
        for( vector<Attribute*>::iterator it = sample.begin( ) ; 
            it != sample.end( ); ++it ) {

            if( i == nth ) {
                i = 1;
                boundary->insertBound( ( *it )->Clone( ) );
            }
            else {
                i++;
            }
            ( *it )->DeleteIfAllowed( );
        }

        sample.clear( );

        return boundary;
    }


    /*
    1.16 ~createSample~

    Creates a sample for a given relation and a given attribute.

    */
    vector<Attribute*> Boundary::createSample( Relation* _rel, int _attr ) {

        assert( _attr >= 0 );

        vector<Attribute*> sample;

        int sampleSize = computeSampleSize( _rel->GetNoTuples( ) );
        int nth = everyNthTupleForSample( sampleSize, _rel->GetNoTuples( ) );

        GenericRelationIterator* it = _rel->MakeScan( );
        Tuple* tuple;
        while( ( tuple = it->GetNthTuple( nth, false ) ) ) {
            sample.push_back( tuple->GetAttribute( _attr )->Clone( ) );
            tuple->DeleteIfAllowed( );
        }
        delete it;

        sort( sample.begin( ), sample.end( ), compareAttributes );

        return sample;
    }

    /*
    1.17 ~getBoundaryIndexNumber~

    Returns the array index for an attribute. For example the boundary is 
    (0 5 9) and the attribute is an CcInt with the value 2. In this case 
    the array index is 1.

    */
    int Boundary::getBoundaryIndexNumber( Attribute* _attr ) {

        int i = 0;
        for( vector<Attribute*>::iterator it = boundaryArray.begin( ); 
            it != boundaryArray.end( ); ++it ) {

            if( _attr->Compare( ( *it ) ) != 1 ) {
                return i;
            }
            i++;
        }

        return i;
    }

    /*
    1.18 ~isEqual~

    Compares this boundary with another one. Returns true if the boundarys are 
    equal, false otherwise. Is one boundary undefined false is returned.

    */
    bool Boundary::isEqual( Boundary* _boundary ) {
        // for same pointer return true
        if( this == _boundary ) {
            return true;
        }
        if( !defined || !_boundary->isDefined( ) ) {
            return false;
        }
        if( attrType != _boundary->getAttrType( ) ) {
            return false;
        }
        if( boundaryArray.size( ) != _boundary->getSize( ) ) {
            return false;
        }

        // compare the boundary arrays
        vector<Attribute*> _boundaryArray = _boundary->getArray( );
        vector<Attribute*>::iterator it1 = boundaryArray.begin( );
        vector<Attribute*>::iterator it2 = _boundaryArray.begin( );

        while( it1 != boundaryArray.end( ) || it2 != _boundaryArray.end( ) ) {
            if( ( *it1 )->Compare( ( *it2 ) ) != 0 ) {
                return false;
            }
            ++it1;
            ++it2;
        }

        return true;
    }

    /*
    1.19 ~BasicType~

    Returns the BasicType of the secondo type.

    */
    const string Boundary::BasicType( ) {
        return "boundary";
    }

    /*
    1.20 ~checkType~

    Checks the type in the NestedList.

    */
    const bool Boundary::checkType( const ListExpr list ) {
        if( !nl->HasLength( list, 2 ) ) {
            return false;
        }

        return listutils::isSymbol( nl->First( list ), BasicType( ) );
    }

    /*
    1.21 ~checkType~

    Checks the type in the NestedList.

    */
    const bool Boundary::checkType( 
        const ListExpr list, const ListExpr attrType ) {

        if( ! checkType( list ) ) {
            return false;
        }
        if( ! nl->HasLength( nl->Second( list ), 1 ) ) {
            return false;
        }

        if( ! listutils::isSymbol( attrType) ) {
            return false;
        }

        return listutils::isSymbol( 
            nl->First( nl->Second( list ) ), nl->SymbolValue( attrType ) );
    }

    /*
    1.22 ~Property~

    Returns the secondo property informations.

    */
    ListExpr Boundary::Property( ) {
        return ( nl->TwoElemList(
            nl->FourElemList(
                nl->StringAtom( "Signature" ),
                nl->StringAtom( "Expample Type List" ),
                nl->StringAtom( "List Rep" ),
                nl->StringAtom( "Example List" ) ),
            nl->FourElemList(
                nl->StringAtom( "-> SIMPLE" ),
                nl->StringAtom( BasicType( ) ),
                nl->StringAtom( "( " + BasicType( ) + ") = (5, 10, 15, 20)" ),
                nl->TextAtom( " (('" + BasicType( ) +
                    "') ( 5 10 15 20)" )
            ) ) );
    }

    /*
    1.23 ~In~

    Secondo In function.

    */
    Word Boundary::In( const ListExpr typeInfo,
        const ListExpr value,
        const int errorPos,
        ListExpr & errorInfo,
        bool & correct ) {

        Word res( ( void* )0 );
        res.addr = Boundary::readFrom( typeInfo, value );
        correct = res.addr != 0;
        return res;
    }

    /*
    1.24 ~Out~

    Secondo Out function.

    */
    ListExpr Boundary::Out( const ListExpr typeInfo, Word value ) {

        Boundary* boundary = ( Boundary* )value.addr;
        if( ! boundary->isDefined( ) ) {
            return listutils::getUndefined( );
        }
        return boundary->toListExpr( );
    }

    /*
    1.25 ~Create~

    Secondo Create function.

    */
    Word Boundary::Create( const ListExpr typeInfo ) {
        Word w;
        w.addr = new Boundary( );
        return w;
    }

    /*
    1.26 ~Delete~

    Secondo Delete function.

    */
    void Boundary::Delete( const ListExpr typeInfo, Word & w ) {
        Boundary* b = ( Boundary* )w.addr;
        b->DeleteIfAllowed( );
        w.addr = 0;
    }

    /*
    1.27 ~Open~

    Secondo Open function.

    */
    bool Boundary::Open(
        SmiRecord& valueRecord, size_t& offset,
        const ListExpr typeInfo, Word& value ) {

        ListExpr attrType = nl->First( nl->Second( typeInfo ) );
        int algebraId = nl->IntValue( nl->First( attrType ) );
        int typeId = nl->IntValue( nl->Second( attrType ) );

        bool defined;
        int size;
        if( ! distributed2::readVar<bool>( defined, valueRecord, offset ) ) {
            return false;
        }
        if( !defined ) {
            Boundary* boundary = new Boundary( );
            value.addr = boundary;
            boundary->makeUndefined( );
            return true;
        }

        if( !distributed2::readVar<int>( size, valueRecord, offset ) ) {
            return false;
        }
        
        SecondoCatalog* sc = SecondoSystem::GetCatalog( );
        Boundary* boundary = 
            new Boundary( sc->GetTypeName( algebraId, typeId ) );
        value.addr = boundary;

        Word attr;
        for( int i = 0 ; i < size ; i++ ) {
            if( !( am->OpenObj( algebraId, typeId, valueRecord, 
                offset, attrType, attr ) ) ) {

                return false;
            }
            boundary->insertBound( ( Attribute* )attr.addr );
        }

        return true;
    }

    /*
    1.28 ~Save~

    Secondo Save function.

    */
    bool Boundary::Save(
        SmiRecord& valueRecord, size_t& offset,
        const ListExpr typeInfo, Word& value ) {

        Boundary* b = static_cast< Boundary* >( value.addr );
        return b->save( valueRecord, offset );
    }

    /*
    1.29 ~Close~

    Secondo Close function.

    */
    void Boundary::Close( const ListExpr typeInfo, Word & w ) {

        Boundary* b = ( Boundary* )w.addr;
        b->DeleteIfAllowed( );
        w.addr = 0;
    }

    /*
    1.30 ~Clone~

    Secondo Clone function.

    */
    Word Boundary::Clone( const ListExpr typeInfo, const Word & w ) {

        Boundary* b = ( Boundary* )w.addr;
        Word res;
        res.addr = new Boundary( *b );
        return res;
    }

    /*
    1.31 ~Cast~

    Secondo Cast function.

    */
    void* Boundary::Cast( void * addr ) {

        return ( new ( addr ) Boundary( "" ) );
    }

    /*
    1.32 ~TypeCheck~

    Secondo TypeCheck function.

    */
    bool Boundary::TypeCheck( ListExpr type, ListExpr & errorInfo ) {
        return Boundary::checkType( type );
    }

    /*
    1.33 ~SizeOf~

    Secondo SizeOf function.

    */
    int Boundary::SizeOf( ) {
        return 42; // a magic number
    }

    /*
    1.34 ~computeSampleSize~

    Computes the sample size for a total size of a relation.
    Attention: totalSize has to be bigger than 0.

    */
    int Boundary::computeSampleSize( const int totalSize ) {

        assert( totalSize > 0 );

        if( totalSize <= 5000 ) {
            return totalSize;
        }

        int sampleSize = totalSize / 1000;

        return sampleSize < 5000 ? 5000 : sampleSize;
    }

    /*
    1.35 ~everyNthTupleForSample~

    The minimum size of a sample is 5000 or 1/1000 of the ~totalSize~.
    The returned number said that every nth tuple has to be part of the
    sample.
    Attention: totalSize has to be bigger than 0.

    */
    int Boundary::everyNthTupleForSample( const int totalSize ) {

        assert( totalSize > 0 );

        return totalSize / computeSampleSize( totalSize );
    }
    
    /*
    1.36 ~everyNthTupleForSample~

    The minimum size of a sample is 5000 or 1/1000 of the ~totalSize~.
    The returned number said that every nth tuple has to be part of the
    sample.
    Attention: sampleSize and totalSize have to be bigger than 0.

    */
    int Boundary::everyNthTupleForSample( 
        const int sampleSize, const int totalSize ) {

        assert( sampleSize > 0 );
        assert( totalSize > 0 );

        return totalSize / sampleSize;
    }

    /*
    1.37 ~everyNthTupleForArray~

    The sample uses every nth tuple of a relation. To fill the array only
    every nth Tuple of the sample can be used, because normaly a sample is
    much bigger than the array.
    Attention: sampleSize and arraySize have to be bigger than 0.

    */
    int Boundary::everyNthTupleForArray( 
        const int sampleSize, const int arraySize ) {

        assert( sampleSize > 0 );
        assert( arraySize > 0 );

        if( arraySize >= sampleSize ) {
            return 1;
        }

        return sampleSize / arraySize;
    }

    bool Boundary::compareAttributes( 
        const Attribute* attr1, const Attribute* attr2 ) {

        return attr1->Compare( attr2 ) == -1;
    }

    /*
    1.38 ~BoundaryTC~

    TypeConstructor of the type Boundary

    */
    TypeConstructor BoundaryTC(
        Boundary::BasicType( ),
        Boundary::Property,
        Boundary::Out,
        Boundary::In,
        0, 0,
        Boundary::Create,
        Boundary::Delete,
        Boundary::Open,
        Boundary::Save,
        Boundary::Close,
        Boundary::Clone,
        Boundary::Cast,
        Boundary::SizeOf,
        Boundary::TypeCheck
    );

} // end of namespace drel