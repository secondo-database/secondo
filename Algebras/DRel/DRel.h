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
#ifndef _DFRel_h_
#define _DFRel_h_

#include "Algebras/Distributed2/DArray.h"
#include "Algebras/Collection/CollectionAlgebra.h"

#include "DRelHelpers.h"
#include "DistTypeBasic.h"
#include "DistTypeHash.h"
#include "DistTypeRange.h"
#include "DistTypeSpatial.h"

namespace drel {
/*
1 ~DRel~

Secondo type to distribute relations to workers and makes it possible 
to query about the distributed data.

*/
    template<distributed2::arrayType T>
    class DRelT : public distributed2::DArrayT<T> {

    public:
/*
1.1 Methods

*/
        DRelT( const std::vector<uint32_t>&v, const std::string& name );
        DRelT( const int dummy );
        DRelT( const DRelT& src );
        DRelT( const distributed2::DArrayBase& src );
        DRelT( const distributed2::DArrayT<T>& src );

        DRelT& operator=( const DRelT& src );
        DRelT& operator=( const distributed2::DArrayBase& src );
        DRelT& operator=( const distributed2::DArrayT<T>& src );

        virtual ~DRelT( );

        void setDistType( DistTypeBasic* _distType );
        DistTypeBasic* getDistType( );
        std::string getTypeName( ) const;
        ListExpr toListExpr( ListExpr typeInfo ) const;
        static DRelT* readFrom( ListExpr typeInfo, ListExpr list );
        static DistTypeBasic* readDistTypeFrom( 
            ListExpr typeInfo, ListExpr list );

        template<class R>
        const bool equalDistType( R* drel );

        static const std::string BasicType( );
        static const bool checkType( const ListExpr list );
        static const bool checkDistType( const ListExpr list );
        static const bool checkType( const ListExpr list, 
            distributionType& type, int& attr, int& key );
        static const bool checkDistType( const ListExpr list, 
            distributionType& dtype, int& attr, int& key );
        static ListExpr Property( );

        static Word In( const ListExpr typeInfo,
            const ListExpr value,
            const int errorPos,
            ListExpr& errorInfo,
            bool& correct );

        static ListExpr Out( const ListExpr typeInfo, Word value );

        static Word Create( const ListExpr typeInfo );

        static void Delete( const ListExpr typeInfo, Word& w );

        static bool Open( SmiRecord& valueRecord,
            size_t& offset,
            const ListExpr typeInfo,
            Word& value );

        static bool Save( SmiRecord& valueRecord,
            size_t& offset,
            const ListExpr typeInfo,
            Word& value );

        static void Close( const ListExpr typeInfo, Word& w );

        static Word Clone( const ListExpr typeInfo, const Word& w );

        static void* Cast( void* addr );

        static bool TypeCheck( ListExpr type, ListExpr& errorInfo );

        static int SizeOf( );

    private:
/*
1.2 Members

1.2.1 ~distType~

Informations about the distribution of the relation.

*/
        DistTypeBasic* distType;
    };


/*
1 Class ~DRel~ Implementation.

1.1 Constructors

*/
    template<distributed2::arrayType T>
    DRelT<T>::DRelT( const std::vector<uint32_t>&v, const std::string& name ) :
        distributed2::DArrayT<T>( v, name ), distType( 0 ) {

        #ifdef DRELDEBUG
        cout << "DRelT constructor" << endl;
        #endif
    }

    template<distributed2::arrayType T>
    DRelT<T>::DRelT( const int dummy ) :
        distributed2::DArrayT<T>( dummy ), distType( 0 ) {

        #ifdef DRELDEBUG
        cout << "DRelT constructor" << endl;
        #endif
    }

/*
1.1 Copyconstructor

*/
    template<distributed2::arrayType T>
    DRelT<T>::DRelT( const DRelT& src ) :
        distributed2::DArrayT<T>( src ), distType( src.distType ) {

        #ifdef DRELDEBUG
        cout << "DRelT copy constructor" << endl;
        #endif
    }

    template<distributed2::arrayType T>
    DRelT<T>::DRelT( const distributed2::DArrayBase& src ) :
        distributed2::DArrayT<T>( src ), distType( 0 ) {

        #ifdef DRELDEBUG
        cout << "DRelT copy constructor" << endl;
        #endif
    }

    template<distributed2::arrayType T>
    DRelT<T>::DRelT( const distributed2::DArrayT<T>& src ) :
        distributed2::DArrayT<T>( src ), distType( 0 ) {

        #ifdef DRELDEBUG
        cout << "DRelT copy constructor" << endl;
        #endif
    }

/*
1.2 Assignment operator

*/
    template<distributed2::arrayType T>
    DRelT<T>& DRelT<T>::operator=( const DRelT& src ) {

        #ifdef DRELDEBUG
        cout << "DRelT assignment operator" << endl;
        #endif

        if( this == &src ) {
            return *this;
        }
  distributed2::DArrayT<T>::operator=( src );
        distType = src.distType;
        return *this;
    }

    template<distributed2::arrayType T>
    DRelT<T>& DRelT<T>::operator=( const distributed2::DArrayBase& src ) {

        #ifdef DRELDEBUG
        cout << "DRelT assignment operator" << endl;
        #endif

  distributed2::DArrayBase::operator=( src );
        distType = 0;
        return *this;
    }

    template<distributed2::arrayType T>
    DRelT<T>& DRelT<T>::operator=( const distributed2::DArrayT<T>& src ) {

        #ifdef DRELDEBUG
        cout << "DRelT assignment operator" << endl;
        #endif

  distributed2::DArrayT<T>::operator=( src );
        distType = 0;
        return *this;
    }

/*
1.3 Destructor

*/
    template<distributed2::arrayType T>
    DRelT<T>::~DRelT( ) {

        #ifdef DRELDEBUG
        cout << "DRelT destructor" << endl;
        #endif

        if( distType != 0 ) {
            delete distType;
        }
    }

/*
1.4 ~setDistType~

Set the distType.

*/
    template<distributed2::arrayType T>
    void DRelT<T>::setDistType( DistTypeBasic* _distType ) {

        #ifdef DRELDEBUG
        cout << "DRelT::setDistType" << endl;
        cout << "distType" << endl;
        _distType->print( );
        #endif

        distType = _distType;
    }

/*
1.5 ~getDistType~

Get the distType.

*/
    template<distributed2::arrayType T>
    DistTypeBasic* DRelT<T>::getDistType( ) {

        #ifdef DRELDEBUG
        cout << "DRelT::getDistType" << endl;
        cout << "distType" << endl;
        distType->print( );
        #endif

        return distType;
    }

/*
1.6 ~getTypeName~

Returns the type name of the darray. (DARRAY or DFARRAY)

*/
    template<distributed2::arrayType T>
    std::string DRelT<T>::getTypeName( ) const {

        #ifdef DRELDEBUG
        cout << "DRelT::getTypeName" << endl;
        cout << "TypeName" << endl;
        cout << getName( T ) << endl;
        #endif

        return getName( T );
    }

/*
1.7 ~toListExpr~

Returns the DRel as a NestedList.

*/
    template<distributed2::arrayType T>
    ListExpr DRelT<T>::toListExpr( ListExpr typeInfo ) const {

        #ifdef DRELDEBUG
        cout << "DRelT::toListExpr" << endl;
        cout << "typeInfo" << endl;
        cout << nl->ToString( typeInfo ) << endl;
        #endif

        ListExpr listArray = distributed2::DArrayBase::toListExpr( );
        if( listutils::isSymbolUndefined( listArray ) ) {
            return listArray;
        }

        #ifdef DRELDEBUG
        cout << "darray" << endl;
        cout << nl->ToString( listArray ) << endl;
        cout << "distTypeList" << endl;
        distType->print( );
        #endif

        ListExpr drelList = nl->TwoElemList(
            listArray,
            distType->toListExpr( nl->Third( typeInfo ) ) );

        #ifdef DRELDEBUG
        cout << "drelList" << endl;
        cout << nl->ToString( drelList ) << endl;
        #endif

        return drelList;
    }

/*
1.8 ~readFrom~

Reads a NestedList and returns a DRel pointer. The pointer is 0 if the 
list has an error.

*/
    template<distributed2::arrayType T>
    DRelT<T>* DRelT<T>::readFrom( ListExpr typeInfo, ListExpr list ) {

        #ifdef DRELDEBUG
        cout << "DRelT::readFrom" << endl;
        cout << "typeInfo" << endl;
        cout << nl->ToString( typeInfo ) << endl;
        cout << "list" << endl;
        cout << nl->ToString( list ) << endl;
        #endif

        if( listutils::isSymbolUndefined( list ) ) {
        distributed2::DArrayT<T>* darray = 
                           distributed2::DArrayT<T>::readFrom( list );
            DRelT<T>* rel = new DRelT<T>( *darray );
            delete darray;
            return rel;
        }

        if( !nl->HasLength( list, 3 ) ) {
            DRelT<T>* rel = new DRelT<T>( 0 );
            rel->makeUndefined( );
            return rel;
        }

        if( !nl->HasLength( typeInfo, 3 ) ) {
            DRelT<T>* rel = new DRelT<T>( 0 );
            rel->makeUndefined( );
            return rel;
        }

  distributed2::DArrayT<T>* darray = 
           distributed2::DArrayT<T>::readFrom( nl->Second( list ) );
        DRelT<T>* rel = new DRelT<T>( *darray );
        delete darray;

        DistTypeBasic* distrType = readDistTypeFrom(
            nl->Third( typeInfo ), nl->Third( list ) );

        if( !distrType ) {
            rel->makeUndefined( );
        }
        else {
            rel->setDistType( distrType );
        }

        return rel;
    }

    template<distributed2::arrayType T>
    DistTypeBasic* DRelT<T>::readDistTypeFrom( 
        ListExpr typeInfo, ListExpr list ) {

        if( !nl->HasMinLength( list, 1 ) ) {
            return 0;
        }

        ListExpr typeDef = nl->First( list );

        if( !nl->IsAtom( typeDef )) {
            return 0;
        }

        distributionType type;
        if( nl->AtomType( typeDef ) == SymbolType ) {  // type by symbol
            if( !supportedType( nl->SymbolValue( typeDef ), type ) ) {
                return 0;
            }
        }
        else if( nl->AtomType( typeDef ) == IntType ) { // type by int
            if( !getTypeByNum( ( int )nl->IntValue( typeDef ), type ) ) {
                return 0;
            }
        }
        else {
            return 0;
        }

        if( nl->HasLength( list, 1 )
         && ( type == random || type == replicated ) ) {
            return new DistTypeBasic( type );
        }

        if( !nl->HasMinLength( list, 2 ) ) {
            return 0;
        }

        if( !nl->IsAtom( nl->Second( list ) )
         || nl->AtomType( nl->Second( list ) ) != IntType ) {
            return 0;
        }

        int attrPos = nl->IntValue( nl->Second( list ) );
        if( attrPos < 0 ) {
            return 0;
        }

        if( nl->HasLength( list, 2 ) && type == hash ) {
            return new DistTypeHash( type, attrPos );
        }

        if( !nl->HasLength( list, 4 ) ) {
            return 0;
        }

        if( !nl->IsAtom( nl->Third( list ) )
         || nl->AtomType( nl->Third( list ) ) != IntType ) {
            return 0;
        }

        int key = nl->IntValue( nl->Third( list ) );

        if( type == range ) {
            bool correct;
            ListExpr errorInfo;
            Word value = collection::Collection::In(
                nl->Fourth( typeInfo ),
                nl->Fourth( list ), 0, errorInfo,
                correct );

            if( !correct ) {
                return 0;
                
            }

            return new DistTypeRange(
                type, attrPos, key, ( collection::Collection* ) value.addr );
        }

        if( type == spatial2d ) {

            temporalalgebra::CellGrid2D* grid =
                DistTypeSpatial<temporalalgebra::CellGrid2D>::ReadFrom(
                    nl->Fourth( typeInfo ),
                    nl->Fourth( list ) );

            if( !grid ) {
                return 0;
            }

            return new DistTypeSpatial<temporalalgebra::CellGrid2D>(
                type, attrPos, key, grid );
        }
        
        if( type == spatial3d ) {

            temporalalgebra::CellGrid<3>* grid =
                DistTypeSpatial<temporalalgebra::CellGrid<3>>::ReadFrom(
                    nl->Fourth( typeInfo ),
                    nl->Fourth( list ) );

            if( !grid ) {
                return 0;
            }

            return new DistTypeSpatial<temporalalgebra::CellGrid<3>>(
                type, attrPos, key, grid );
        }

        return 0;
    }

/*
1.9 ~equalDistType~

Compares the disttype of this drel with the disttype of another one.

*/
    template<distributed2::arrayType T>
    template<class R>
    const bool DRelT<T>::equalDistType( R* drel ) {

        #ifdef DRELDEBUG
        cout << "DRelT::equalDistType" << endl;
        #endif

        if( distType == 0 ) {
            return false;
        }
        return distType->isEqual( drel->getDistType( ) );
    }

    typedef DRelT<distributed2::DFARRAY> DFRel;
    typedef DRelT<distributed2::DARRAY> DRel;
    
    template const bool DRel::equalDistType<DRel>( DRel* );
    template const bool DRel::equalDistType<DFRel>( DFRel* );
    template const bool DFRel::equalDistType<DRel>( DRel* );
    template const bool DFRel::equalDistType<DFRel>( DFRel* );

/*
1.10 ~BasicType~

Returns the BasicType of the secondo type.

*/
    template<distributed2::arrayType T>
    const std::string DRelT<T>::BasicType( ) {

        #ifdef DRELDEBUG
        cout << "DRelT::BasicType" << endl;
        #endif

        if( T == distributed2::DFARRAY ) {
            return "dfrel";
        } else if( T == distributed2::DARRAY ) {
            return "drel";
        }
        return "unknown";
    }

/*
1.11 ~checkType~

Checks the type in the NestedList. 

*/
    template<distributed2::arrayType T>
    const bool DRelT<T>::checkType( const ListExpr list ) {

        distributionType type;
        int attr, key;
        return DRelT<T>::checkType( list, type, attr, key );
    }

    template<distributed2::arrayType T>
    const bool DRelT<T>::checkType( 
        const ListExpr list, distributionType& type, int& attr, int& key ) {

        #ifdef DRELDEBUG
        cout << "DRelT::checkType" << endl;
        cout << "list" << endl;
        cout << nl->ToString( list ) << endl;
        #endif

        if( T != distributed2::DARRAY && T != distributed2::DFARRAY ) {
            return false;
        }
        if( !nl->HasLength( list, 3 ) ) {
            return false;
        }
        if( !listutils::isSymbol( nl->First( list ), BasicType( ) ) ) {
            return false;
        }

        if( !listutils::isRelDescription( nl->Second( list ) ) ) {
            return false;
        }

        #ifdef DRELDEBUG
        ListExpr distTypeExpr = nl->Third( list );
        cout << "distTypeExpr" << endl;
        cout << nl->ToString( distTypeExpr ) << endl;
        #endif

        if( !DRelT<T>::checkDistType( nl->Third( list ), type, attr, key ) ) {
            return false;
        }

        return true;
    }

    template<distributed2::arrayType T>
    const bool DRelT<T>::checkDistType( 
        const ListExpr list, distributionType& type, int& attr, int& key ) {
    
        attr = key = -1;

        if( !nl->HasMinLength( list, 1 ) ) {
            return false;
        }

        ListExpr typeDef = nl->First( list );
        if( !nl->IsAtom( typeDef )) {
            return false;
        }

        if( nl->AtomType( typeDef ) == IntType ) {
            if( !getTypeByNum( ( int )nl->IntValue( typeDef ), type ) ) {
                return false;
            }
        }
        else {
            return false;
        }

        if( nl->HasLength( list, 1 )
         && ( type == random || type == replicated ) ) {
            return true;
        }

        if( !nl->HasMinLength( list, 2 ) ) {
            return false;
        }

        if( !nl->IsAtom( nl->Second( list ) )
         || nl->AtomType( nl->Second( list ) ) != IntType ) {
            return false;
        }

        attr = nl->IntValue( nl->Second( list ) );
        if( attr < 0 ) {
            return false;
        }

        if( nl->HasLength( list, 2 ) && type == hash ) {
            return true;
        }

        if( !nl->HasLength( list, 4 ) ) {
            return false;
        }

        if( !nl->IsAtom( nl->Third( list ) )
         || nl->AtomType( nl->Third( list ) ) != IntType ) {
            return false;
        }

        key = nl->IntValue( nl->Third( list ) );

        if( type == range ) {
            return Vector::checkType( nl->Fourth( list ) );
        }

        if( type == spatial2d ) {
            return temporalalgebra::CellGrid2D::checkType( 
                nl->Fourth( list ) );
        }
        
        if( type == spatial3d ) {
            return temporalalgebra::CellGrid<3>::checkType( 
                nl->Fourth( list ) );
        }

        return false;
    }

/*
1.12 ~Property~

Returns the secondo property informations.

*/
    template<distributed2::arrayType T>
    ListExpr DRelT<T>::Property( ) {

        #ifdef DRELDEBUG
        cout << "DRelT::Property" << endl;
        #endif

        return ( nl->TwoElemList(
            nl->FourElemList(
                nl->StringAtom( "Signature" ),
                nl->StringAtom( "Expample Type List" ),
                nl->StringAtom( "List Rep" ),
                nl->StringAtom( "Example List" ) ),
            nl->FourElemList(
                nl->StringAtom( "-> SIMPLE" ),
                nl->StringAtom( BasicType( ) ),
                nl->StringAtom( "(string " + BasicType( ) + ") = (x, y)" ),
                nl->TextAtom( " (('" + BasicType( ) + 
                    "') ( ( mydarray (0 1 0 1) ('localhost' 1234 'config.ini')"
                    " ('localhost'  1235 'config.ini')))( 'random' ))" )
            ) ) );
    }

/*
1.13 ~In~

Secondo In function.

*/
    template<distributed2::arrayType T>
    Word DRelT<T>::In( const ListExpr typeInfo,
        const ListExpr value,
        const int errorPos,
        ListExpr& errorInfo,
        bool& correct ) {

        #ifdef DRELDEBUG
        cout << "DRelT::In" << endl;
        cout << nl->ToString( typeInfo ) << endl;
        #endif

        Word res( ( void* )0 );
        res.addr = DRelT<T>::readFrom( typeInfo, value );
        correct = res.addr != 0;
        return res;
    }

/*
1.14 ~Out~

Secondo Out function.

*/
    template<distributed2::arrayType T>
    ListExpr DRelT<T>::Out( const ListExpr typeInfo, Word value ) {

        DRelT<T>* drel = ( DRelT<T>* )value.addr;

        #ifdef DRELDEBUG
        cout << "DRelT::Out" << endl;
        cout << "typeInfo" << endl;
        cout << nl->ToString( typeInfo ) << endl;
        cout << "drel" << endl;
        cout << nl->ToString( drel->toListExpr( typeInfo ) ) << endl;
        #endif

        return drel->toListExpr( typeInfo );
    }

/*
1.15 ~Create~

Secondo Create function.

*/
    template<distributed2::arrayType T>
    Word DRelT<T>::Create( const ListExpr typeInfo ) {

        #ifdef DRELDEBUG
        cout << "DRelT::Create" << endl;
        cout << "typeInfo" << endl;
        cout << nl->ToString( typeInfo ) << endl;
        #endif

        Word w;
        std::vector<uint32_t> m;
        w.addr = new DRelT<T>( m, "" );
        return w;
    }

/*
1.16 ~Delete~

Secondo Delete function.

*/
    template<distributed2::arrayType T>
    void DRelT<T>::Delete( const ListExpr typeInfo, Word & w ) {

        #ifdef DRELDEBUG
        cout << "DRelT::Delete" << endl;
        cout << "typeInfo" << endl;
        cout << nl->ToString( typeInfo ) << endl;
        #endif

        DRelT<T>* rel = ( DRelT<T>* )w.addr;
        delete rel;
        w.addr = 0;
    }

/*
1.17 ~Open~

Secondo Open function.

*/
    template<distributed2::arrayType T>
    bool DRelT<T>::Open( 
        SmiRecord& valueRecord, size_t& offset, 
        const ListExpr typeInfo, Word& value ) {

        #ifdef DRELDEBUG
        cout << "DRelT::Open" << endl;
        cout << "typeInfo" << endl;
        cout << nl->ToString( typeInfo ) << endl;
        #endif

        if( !distributed2::DArrayBase::open<distributed2::DArrayT<T>>( 
                valueRecord, offset, typeInfo, value ) ) {
            cout << "darray open fail" << endl;
            return false;
        }
        distributed2::DArrayT<T>* darray 
                  = ( distributed2::DArrayT<T>* )value.addr;
        DRelT<T>* rel = new DRelT<T>( *darray );
        delete darray;
        value.addr = rel;

        if( !rel->IsDefined( ) ) {
            #ifdef DRELDEBUG
            cout << "DRel not defined" << endl;
            #endif
            return true;
        }

        if( !nl->HasLength( typeInfo, 3 ) ) {
            #ifdef DRELDEBUG
            cout << "DRel type info error" << endl;
            #endif
            return false;
        }
        
        ListExpr distTypeInfo = nl->Third( typeInfo );

        if( !nl->IsAtom( nl->First( distTypeInfo ) )
         || nl->AtomType( nl->First( distTypeInfo ) ) != IntType ) {
            #ifdef DRELDEBUG
            cout << "DRel dist type info error" << endl;
            #endif
            return false;
        }
        
        distributionType type;
        if( !getTypeByNum( ( int )nl->IntValue( nl->First( distTypeInfo ) ), 
            type ) ) {
            #ifdef DRELDEBUG
            cout << "Distribution type information error" << endl;
            #endif
            return false;
        }

        #ifdef DRELDEBUG
        cout << "dist type" << endl;
        cout << type << endl;
        #endif

        if( type == replicated || type == random ) {

            rel->setDistType( new DistTypeBasic( type ) );
            return true;
        }

        // get attribute number
        if( !nl->HasMinLength( distTypeInfo, 2 )
            || !nl->IsAtom( nl->Second( distTypeInfo ) )
            || nl->AtomType( nl->Second( distTypeInfo ) ) != IntType ) {
            #ifdef DRELDEBUG
            cout << "Attribute type information error" << endl;
            #endif
            return false;
        }


        int attr = ( int )nl->IntValue( nl->Second( distTypeInfo ) );
        if( attr < 0 ) {
            return false;
        }

        #ifdef DRELDEBUG
        cout << "attr open" << endl;
        cout << attr << endl;
        #endif

        if( type == hash) {

            rel->setDistType( new DistTypeHash( type, attr ) );
            return true;
        }

        // disttribution type key
        if( !nl->HasMinLength( distTypeInfo, 3 )
            || !nl->IsAtom( nl->Third( distTypeInfo ) )
            || nl->AtomType( nl->Third( distTypeInfo ) ) != IntType ) {
            cout << "Distribution type key information error" << endl;
            return false;
        }

        int key = ( int )nl->IntValue( nl->Third( distTypeInfo ) );

        if( type == range ) {

            #ifdef DRELDEBUG
            cout << "type of vector" << endl;
            cout << nl->ToString( nl->Fourth( distTypeInfo ) ) << endl;
            #endif
            
            Word value;
            if( !collection::Collection::Open( 
                valueRecord, offset, nl->Fourth( distTypeInfo ), value ) ) {

                delete rel;
                return false;
            }

            #ifdef DRELDEBUG
            cout << "open vector" << endl;
            ( ( collection::Collection* ) value.addr )->Print( cout );
            cout << endl;
            #endif
            
            rel->setDistType( new DistTypeRange( 
                type, attr, key, ( collection::Collection* ) value.addr ) );

            return true;
        }
        
        if( type == spatial2d ) {

            temporalalgebra::CellGrid2D* grid = 
                static_cast< temporalalgebra::CellGrid2D* >( 
                    Attribute::Open( valueRecord, offset, 
                        nl->Fourth( distTypeInfo ) ) );

            #ifdef DRELDEBUG
            cout << "open cellgrid2d" << endl;
            grid->Print( cout );
            cout << endl;
            #endif

            rel->setDistType( 
                new DistTypeSpatial<temporalalgebra::CellGrid2D>( 
                    type, attr, key, grid ) );

            return true;
        }
        
        if( type == spatial3d ) {

            temporalalgebra::CellGrid<3>* grid =
                static_cast< temporalalgebra::CellGrid<3>* >(
                    Attribute::Open( valueRecord, offset,
                        nl->Fourth( distTypeInfo ) ) );

            #ifdef DRELDEBUG
            cout << "open cellgrid3d" << endl;
            grid->Print( cout );
            cout << endl;
            #endif

            rel->setDistType( 
                new DistTypeSpatial<temporalalgebra::CellGrid<3>>( 
                    type, attr, key, grid ) );

            return true;
        }

        #ifdef DRELDEBUG
        cout << "open error" << endl;
        #endif

        return false;
    }

/*
1.18 ~Save~

Secondo Save function.

*/
    template<distributed2::arrayType T>
    bool DRelT<T>::Save( 
        SmiRecord& valueRecord, size_t& offset, 
        const ListExpr typeInfo, Word& value ) {

        #ifdef DRELDEBUG
        cout << "DRelT::Save" << endl;
        cout << "typeInfo" << endl;
        cout << nl->ToString( typeInfo ) << endl;
        #endif

        if( !distributed2::DArrayBase::save( valueRecord, offset, 
                                             typeInfo, value ) ) {
            return false;
        }

        DRelT<T>* drel = ( DRelT<T>* ) value.addr;
        return drel->getDistType( )->save( 
            valueRecord, offset, nl->Third( typeInfo ) );
    }

/*
1.19 ~Close~

Secondo Close function.

*/
    template<distributed2::arrayType T>
    void DRelT<T>::Close( const ListExpr typeInfo, Word & w ) {

        #ifdef DRELDEBUG
        cout << "DRelT::Close" << endl;
        cout << "typeInfo" << endl;
        cout << nl->ToString( typeInfo ) << endl;
        #endif

        DRelT<T>* rel = ( DRelT<T>* )w.addr;
        delete rel;
        w.addr = 0;
    }

/*
1.20 ~Clone~

Secondo Clone function.

*/
    template<distributed2::arrayType T>
    Word DRelT<T>::Clone( const ListExpr typeInfo, const Word & w ) {

        #ifdef DRELDEBUG
        cout << "DRelT::Clone" << endl;
        cout << "typeInfo" << endl;
        cout << nl->ToString( typeInfo ) << endl;
        #endif

        DRelT<T>* rel = ( DRelT<T>* )w.addr;
        Word res;
        res.addr = new DRelT<T>( *rel );
        return res;
    }

/*
1.21 ~Cast~

Secondo Cast function.

*/
    template<distributed2::arrayType T>
    void* DRelT<T>::Cast( void * addr ) {

        #ifdef DRELDEBUG
        cout << "DRelT::Cast" << endl;
        #endif

        return ( new ( addr ) DRelT<T>( 0 ) );
    }

/*
1.22 ~TypeCheck~

Secondo TypeCheck function.

*/
    template<distributed2::arrayType T>
    bool DRelT<T>::TypeCheck( ListExpr type, ListExpr & errorInfo ) {

        #ifdef DRELDEBUG
        cout << "DRelT::TypeCheck" << endl;
        cout << "typeInfo" << endl;
        cout << nl->ToString( type ) << endl;
        #endif

        return DRelT<T>::checkType( type );
    }

/*
1.23 ~SizeOf~

Secondo SizeOf function.

*/
    template<distributed2::arrayType T>
    int DRelT<T>::SizeOf( ) {
        return 42; // a magic number
    }



} // end of namespace drel

#endif // _DRel_h_
