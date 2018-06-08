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
        bool saveDistType( SmiRecord& valueRecord, size_t& offset, 
            const ListExpr typeInfo );
        ListExpr toListExpr( ListExpr typeInfo ) const;
        static DRelT* readFrom( ListExpr typeInfo, ListExpr list );

        template<class R>
        const bool equalDistType( R* drel );

        static const std::string BasicType( );
        static const bool checkType( const ListExpr list );
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

    typedef DRelT<distributed2::DFARRAY> DFRel;
    typedef DRelT<distributed2::DARRAY> DRel;
    
} // end of namespace drel

#endif // _DRel_h_