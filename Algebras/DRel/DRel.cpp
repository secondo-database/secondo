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
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "DRel.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "ConstructorTemplates.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace distributed2;

namespace drel {

    /*
    1 Class ~DRel~

    Implementation.

    1.1 Constructors

    */
    template<arrayType T>
    DRelT<T>::DRelT( const std::vector<uint32_t>&v, const std::string& name ) :
        DArrayT<T>( v, name ), distType( 0 ) {
    }

    template<arrayType T>
    DRelT<T>::DRelT( const int dummy ) :
        DArrayT<T>( dummy ), distType( 0 ) {
    }

    /*
    1.1 Copyconstructor

    */
    template<arrayType T>
    DRelT<T>::DRelT( const DRelT& src ) :
        DArrayT<T>( src ), distType( new DistTypeBasic( *src.distType ) ) {
    }

    template<arrayType T>
    DRelT<T>::DRelT( const DArrayBase& src ) :
        DArrayT<T>( src ), distType( 0 ) {
    }

    template<arrayType T>
    DRelT<T>::DRelT( const DArrayT<T>& src ) :
        DArrayT<T>( src ), distType( 0 ) {
    }

    /*
    1.2 Assignment operator

    */
    template<arrayType T>
    DRelT<T>& DRelT<T>::operator=( const DRelT& src ) {
        if( this == &src ) {
            return *this;
        }
        DArrayT<T>::operator=( src );
        distType = new DistTypeBasic( *src.distType );
        return *this;
    }

    template<arrayType T>
    DRelT<T>& DRelT<T>::operator=( const DArrayBase& src ) {
        DArrayBase::operator=( src );
        distType = 0;
        return *this;
    }

    template<arrayType T>
    DRelT<T>& DRelT<T>::operator=( const DArrayT<T>& src ) {
        DArrayT<T>::operator=( src );
        distType = 0;
        return *this;
    }

    /*
    1.3 Destructor

    */
    template<arrayType T>
    DRelT<T>::~DRelT( ) {
        if( distType != 0 ) {
            delete distType;
        }
    }

    /*
    1.4 ~setDistType~

    Set the distType.

    */
    template<arrayType T>
    void DRelT<T>::setDistType( DistTypeBasic* _distType ) {
        distType = _distType;
    }

    template<distributed2::arrayType T>
    DistTypeBasic* DRelT<T>::getDistType( ) {
        return distType;
    }

    /*
    1.5 ~getTypeName~

    Returns the type name of the darray. (DARRAY or DFARRAY)

    */
    template<arrayType T>
    std::string DRelT<T>::getTypeName( ) const {
        return getName( T );
    }

    /*
    1.6 ~saveDistType~

    Saves the distType.

    */
    template<arrayType T>
    bool DRelT<T>::saveDistType(
        SmiRecord& valueRecord, size_t& offset, ListExpr typeInfo ) {

        if( distType == 0 ) {
            return true;
        }
        return distType->save( valueRecord, offset, typeInfo );
    }

    /*
    1.7 ~toListExpr~

    Returns the DRel as a NestedList.

    */
    template<arrayType T>
    ListExpr DRelT<T>::toListExpr( ListExpr typeInfo ) const {
        ListExpr listArray = DArrayBase::toListExpr( );
        if( listutils::isSymbolUndefined( listArray ) ) {
            return listArray;
        }
        return nl->FourElemList( nl->First( listArray ),
            nl->Second( listArray ),
            nl->Third( listArray ),
            distType->toListExpr( 
                nl->First( nl->Rest( nl->Rest( typeInfo ) ) ) ) );
    }

    /*
    1.8 ~readFrom~

    Reads a NestedList and returns a DRel pointer. The pointer is 0 if the 
    list has an error.

    */
    template<arrayType T>
    DRelT<T>* DRelT<T>::readFrom( ListExpr typeInfo, ListExpr list ) {
        if( listutils::isSymbolUndefined( list ) ) {
            DArrayT<T>* darray = DArrayT<T>::readFrom( list );
            DRelT<T>* rel = new DRelT<T>( *darray );
            delete darray;
            return rel;
        }
        if( !nl->HasLength( nl->First( list ), 4 ) ) {
            return 0;
        }

        DArrayT<T>* darray = DArrayBase::readFrom<DArrayT<T>>(
            nl->ThreeElemList( 
                nl->First( list ), 
                nl->Second( list ), 
                nl->Third( list ) ) );
        if( darray == 0 ) {
            return 0;
        }

        DRelT<T>* rel = new DRelT<T>( *darray );
        delete darray;

        ListExpr distTypeList = nl->Fourth( list );
        if( !nl->HasMinLength( distTypeList, 1 ) ) {
            delete rel;
            return 0;
        }
        ListExpr typeList = nl->First( distTypeList );
        distributionType type;
        if( !( DistTypeBasic::readType( typeList, type ) ) ) {
            delete rel;
            return 0;
        }

        DistTypeBasic* dType;

        switch( type ) {
        case replicated:
            dType = DistTypeBasic::readFrom( distTypeList );
            break;
        case random: 
            dType = DistTypeBasic::readFrom( distTypeList );
            break;
        case hash:
            dType = DistTypeHash::readFrom( distTypeList ); // noch typeInfo
            break;
        case range:
            dType = DistTypeRange::readFrom( distTypeList );
            break;
        case spatial2d:
            dType = DistTypeSpatial<temporalalgebra::CellGrid2D>::
                readFrom( distTypeList );
            break;
        case spatial3d:
            dType = DistTypeRange::readFrom( distTypeList ); // noch falsch
            break;
        default:
            delete rel;
            return 0;
            break;
        }

        if( dType == 0 ) {
            delete rel;
            return 0;
        }

        rel->setDistType( dType );
        return rel;
    }

    /*
    1.9 ~equalDistType~

    Compares the disttype of this drel with the disttype of another one.

    */

    template<arrayType T>
    template<class R>
    const bool DRelT<T>::equalDistType( R* drel ) {
        return distType->isEqual( drel->getDistType( ) );
    }

    template const bool DRel::equalDistType<DRel>( DRel* );
    template const bool DRel::equalDistType<DFRel>( DFRel* );
    template const bool DFRel::equalDistType<DRel>( DRel* );
    template const bool DFRel::equalDistType<DFRel>( DFRel* );

    /*
    1.10 ~BasicType~

    Returns the BasicType of the secondo type.

    */
    template<arrayType T>
    const std::string DRelT<T>::BasicType( ) {
        if( T == DFARRAY ) {
            return "dfrel";
        } else if( T == DARRAY ) {
            return "drel";
        }
        return "unknown";
    }

    /*
    1.11 ~checkType~

    Checks the type in the NestedList. 

    */
    template<arrayType T>
    const bool DRelT<T>::checkType( const ListExpr list ) {
        if( T != DARRAY && T != DFARRAY ) {
            return false;
        }
        if( !nl->HasLength( list, 3 ) ) {
            return false;
        }
        if( !listutils::isSymbol( nl->First( list ), BasicType( ) ) ) {
            return false;
        }
        ListExpr distTypeExpr = nl->Third( list );
        return ( DistTypeBasic::checkType( distTypeExpr )
            || DistTypeHash::checkType( distTypeExpr )
            || DistTypeRange::checkType( distTypeExpr ) );
    }

    /*
    1.12 ~Property~

    Returns the secondo property informations.

    */
    template<arrayType T>
    ListExpr DRelT<T>::Property( ) {
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
    template<arrayType T>
    Word DRelT<T>::In( const ListExpr typeInfo,
        const ListExpr value,
        const int errorPos,
        ListExpr& errorInfo,
        bool& correct ) {

        Word res( ( void* )0 );
        res.addr = DRelT<T>::readFrom( typeInfo, value );
        correct = res.addr != 0;
        return res;
    }

    /*
    1.14 ~Out~

    Secondo Out function.

    */
    template<arrayType T>
    ListExpr DRelT<T>::Out( const ListExpr typeInfo, Word value ) {
        DRelT<T>* rel = ( DRelT<T>* )value.addr;
        return rel->toListExpr( typeInfo );
    }

    /*
    1.15 ~Create~

    Secondo Create function.

    */
    template<arrayType T>
    Word DRelT<T>::Create( const ListExpr typeInfo ) {
        Word w;
        std::vector<uint32_t> m;
        w.addr = new DRelT<T>( m, "" );
        return w;
    }

    /*
    1.16 ~Delete~

    Secondo Delete function.

    */
    template<arrayType T>
    void DRelT<T>::Delete( const ListExpr typeInfo, Word & w ) {
        DRelT<T>* rel = ( DRelT<T>* )w.addr;
        delete rel;
        w.addr = 0;
    }

    /*
    1.17 ~Open~

    Secondo Open function.

    */
    template<arrayType T>
    bool DRelT<T>::Open( 
        SmiRecord& valueRecord, size_t& offset, 
        const ListExpr typeInfo, Word& value ) {

        if( !DArrayBase::open<DArrayT<T>>( 
                valueRecord, offset, typeInfo, value ) ) {
            return false;
        }
        DArrayT<T>* darray = ( DArrayT<T>* )value.addr;
        DRelT<T>* rel = new DRelT<T>( *darray );
        delete darray;
        value.addr = rel;

        if( !rel->IsDefined( ) ) {
            return true;
        }

        std::string typeString;
        if( !readVar<std::string>( typeString, valueRecord, offset ) ) {
            delete rel;
            return false;
        }
        distributionType type;
        if( !supportedType( typeString, type ) ) {
            delete rel;
            return false;
        }

        if( type == replicated || type == random ) {

            rel->setDistType( new DistTypeBasic( type ) );
            return true;
        }

        int attr;
        if( !readVar<int>( attr, valueRecord, offset ) ) {

            delete rel;
            return false;
        }

        if( type == hash) {

            rel->setDistType( new DistTypeHash( type, attr ) );
            return true;
        }

        int key;
        if( !readVar<int>( key, valueRecord, offset ) ) {
            return 0;
        }

        if( type == range ) {

            ListExpr distTypeExpr = nl->Third( typeInfo );
            ListExpr boundaryTypeExpr = nl->Fourth( distTypeExpr );
            Word value;
            if( !Boundary::Open( 
                valueRecord, offset, boundaryTypeExpr, value ) ) {

                delete rel;
                return false;
            }
            
            rel->setDistType( new DistTypeRange( 
                type, attr, key, ( Boundary* ) value.addr ) );
        }
        else if( type == spatial2d ) {

            temporalalgebra::CellGrid2D* grid = 
                static_cast< temporalalgebra::CellGrid2D* >( 
                    Attribute::Open( valueRecord, offset, 
                        nl->Fourth( nl->Third( typeInfo ) ) ) );

            rel->setDistType( 
                new DistTypeSpatial<temporalalgebra::CellGrid2D>( 
                    type, attr, key, grid ) );
        }
        //else if( type == spatial3d ) {
        //    ListExpr distTypeExpr = nl->Third( typeInfo );
        //    ListExpr gridTypeExpr = nl->Fourth( distTypeExpr );
        //    temporalalgebra::CellGrid<3>* grid = 
        //        static_cast< temporalalgebra::CellGrid<3>* >( 
        //    Attribute::Open( 
        //      valueRecord, offset, nl->Fourth( nl->Third( typeInfo ) ) ) );

        //    rel->setDistType( 
        //    new DistTypeSpatial<temporalalgebra::CellGrid<3>( 
        //      type, attr, key, grid ) );
        //}

        return true;
    }

    /*
    1.18 ~Save~

    Secondo Save function.

    */
    template<arrayType T>
    bool DRelT<T>::Save( 
        SmiRecord& valueRecord, size_t& offset, 
        const ListExpr typeInfo, Word& value ) {

        if( !DArrayBase::save( valueRecord, offset, typeInfo, value ) ) {
            return false;
        }
        DRelT<T>* drel = ( DRelT<T>* ) value.addr;
        return drel->saveDistType( 
            valueRecord, offset, 
            nl->First( nl->Rest( nl->Rest( typeInfo ) ) ) );
    }

    /*
    1.19 ~Close~

    Secondo Close function.

    */
    template<arrayType T>
    void DRelT<T>::Close( const ListExpr typeInfo, Word & w ) {
        DRelT<T>* rel = ( DRelT<T>* )w.addr;
        delete rel;
        w.addr = 0;
    }

    /*
    1.20 ~Clone~

    Secondo Clone function.

    */
    template<arrayType T>
    Word DRelT<T>::Clone( const ListExpr typeInfo, const Word & w ) {
        DRelT<T>* rel = ( DRelT<T>* )w.addr;
        Word res;
        res.addr = new DRelT<T>( *rel );
        return res;
    }

    /*
    1.21 ~Cast~

    Secondo Cast function.

    */
    template<arrayType T>
    void* DRelT<T>::Cast( void * addr ) {
        return ( new ( addr ) DRelT<T>( 0 ) );
    }

    /*
    1.22 ~TypeCheck~

    Secondo TypeCheck function.

    */
    template<arrayType T>
    bool DRelT<T>::TypeCheck( ListExpr type, ListExpr & errorInfo ) {
        return DRelT<T>::checkType( type );
    }

    /*
    1.23 ~SizeOf~

    Secondo SizeOf function.

    */
    template<arrayType T>
    int DRelT<T>::SizeOf( ) {
        return 42; // a magic number
    }

    /*
    1.24 ~DFRelTC~

    TypeConstructor of the type DFRel

    */
    TypeConstructor DFRelTC(
        DFRel::BasicType( ),
        DFRel::Property,
        DFRel::Out,
        DFRel::In,
        0, 0,
        DFRel::Create,
        DFRel::Delete,
        DFRel::Open,
        DFRel::Save,
        DFRel::Close,
        DFRel::Clone,
        DFRel::Cast,
        DFRel::SizeOf,
        DFRel::TypeCheck
    );

    /*
    1.25 ~DRelTC~

    TypeConstructor of the type DRel

    */
    TypeConstructor DRelTC(
        DRel::BasicType( ),
        DRel::Property,
        DRel::Out,
        DRel::In,
        0, 0,
        DRel::Create,
        DRel::Delete,
        DRel::Open,
        DRel::Save,
        DRel::Close,
        DRel::Clone,
        DRel::Cast,
        DRel::SizeOf,
        DRel::TypeCheck
    );

} // end of namespace drel