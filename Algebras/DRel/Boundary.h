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
#ifndef _Boundary_h_
#define _Boundary_h_

#include "NestedList.h"
#include "AlgebraTypes.h"
#include "Attribute.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

class Tuple;

namespace drel {
    /*
    1 ~Boundary~

    A boundary is the information of a range partitioning. It is an intervall 
    of Attributes informations.
    
    */
    class Boundary {

    public:
        /*
        1.1 Methods

        */
        Boundary( );
        Boundary( const std::string _attrType );
        Boundary( const Boundary& src );

        Boundary& operator=( const Boundary& src );

        ~Boundary( );

        bool isDefined( );
        bool DeleteIfAllowed( );
        void IncReference( );
        const size_t getSize( );
        std::vector<Attribute*> getArray( );
        const std::string getAttrType( );
        void makeUndefined( );

        void insertBound( Attribute* _attr );
        void useSampleAsBound( std::vector<Attribute*> sample );

        static Boundary* createBoundary( Relation* _rel, int _attr, 
            std::string _attrType, int _arraySize );
        static std::vector<Attribute*> createSample(
            Relation* _rel, int _attr );
        int getBoundaryIndexNumber( Attribute* _attr );

        bool isEqual( Boundary* _boundary );

        ListExpr toListExpr( );
        bool save( SmiRecord& valueRecord, size_t& offset );
        static Boundary* readFrom( ListExpr typeInfo, ListExpr value );

        static const std::string BasicType( );
        static const bool checkType( const ListExpr list );
        static const bool checkType( 
            const ListExpr list, const ListExpr attrType );
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
        static int computeSampleSize( const int totalSize );
        static int everyNthTupleForSample( const int totalSize );
        static int everyNthTupleForSample( 
            const int sampleSize, const int totalSize );
        static int everyNthTupleForArray( 
            const int sampleSize, const int arraySize );
        static bool compareAttributes( 
            const Attribute* attr1, const Attribute* attr2 );

        bool saveAttrList( SmiRecord& valueRecord, size_t& offset );
        bool saveAttrList( SmiRecord& valueRecord, size_t& offset,
            const ListExpr attrType, const int algebraId, const int typeId);

        /*
        1.2 Methods

        1.2.1 ~refs~

        Reference counter.

        */
        size_t refs;
        /*
        1.2.2 ~defined~

        Is the object defined.

        */
        bool defined;
        /*
        1.2.3 ~attrType~

        Type of the boundary attributes.

        */
        std::string attrType;
        /*
        1.2.4 ~boundaryArray~

        Array with the boundaries.

        */
        std::vector<Attribute*> boundaryArray;
    };
    
} // end of namespace drel

#endif // _Boundary_h_