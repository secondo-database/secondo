/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//characters [1] verbatim:  [\verb@]         [@]
//paragraph  [1] title:     [{\Large \bf ]   [}]


[1] Material.hpp

1 Defines and includes

*/

#ifndef SECONDO_ALGEBRAS_CHESS_MATERIAL_HPP
#define SECONDO_ALGEBRAS_CHESS_MATERIAL_HPP

#include <tr1/array>
#include "Attribute.h"
#include "ListStream.hpp"
#include "Type.hpp"
#include "Piece.hpp"

/*
2. class Material represents material type

*/


struct Material : Attribute
{
    static const std::string& name()
    {
        static const std::string name( "material" );
        return name;
    }

    std::tr1::array< int, 12 > pieces;

    Material(){}
    Material( undef_t def ) : Attribute(def==DEFINED),
                              defined_(def == DEFINED)
    {
        std::tr1::array< int, 12 > tmp = {{ 0 }};
        pieces = tmp;
    }

    // pure virtual functions of class dAttribute
    virtual bool Adjacent( const Attribute* ) const { return 0; }

    virtual int Compare( const Attribute* other ) const
    {
        const Material& m = *static_cast< const Material* >( other );
        for( int i = 0; i < 12; ++i )
        {
            if ( pieces[i] < m.pieces[i] )
                return -1;
            else if ( pieces[i] > m.pieces[i] )
                return 1;
        }
        return 0;
    }

    virtual std::ostream& Print( std::ostream& os ) const
    {
        os << "Material: [ ";
        copy( pieces.begin(), pieces.end() - 1,
            std::ostream_iterator< int >( os, ", " ) );
        os << pieces[11] << " ]\n";
        return os;
    }

    virtual void CopyFrom( const Attribute* other )
    {
        *this = Material( *static_cast< const Material* >( other ) );
    }

    virtual Attribute* Clone() const { return new Material( *this ); }
    virtual bool IsDefined() const { return defined_; }
    virtual void SetDefined( bool def ) { defined_ = def; }
    virtual size_t Sizeof() const { return sizeof( *this ); }
    virtual size_t HashValue() const
    {
        size_t hash(0);
        for( int i = 0; i < 12; ++i )
        {
            hash += ( i + 1 ) * pieces[i];
            hash *= 2;
        }
        return hash;
    }

    static Material In( ListExpr instance )
    {
        Material m(DEFINED);
        list_istream lis( instance );
        for( int i = 0; i < 12; ++i )
            lis >> m.pieces[i];
        return m;
    }

    static ListExpr Out( const Material& m )
    {
        list_ostream los;
        for( int i = 0; i < 12; ++i )
            los << m.pieces[i];
        return los;
    }

    static bool KindCheck( ListExpr type, ListExpr& )
    {
        return nl->IsEqual( type, "material" );
    }

    bool operator == ( const Material& other ) const
    {
        return pieces == other.pieces;
    }

    double value_sum() const
    {
        double sum = 0.0;
        for( int i = 0; i < 12; ++i )
            sum += Piece::value(i+2) * pieces[i];
        return sum;
    }

    bool operator < ( const Material& other ) const
    {
        return value_sum() < other.value_sum();
    }

    bool operator > ( const Material& other ) const
    {
        return value_sum() > other.value_sum();
    }

    friend std::ostream& operator << ( std::ostream& os, const Material& m )
    {
        return m.Print( os );
    }
protected:
    bool defined_;
};

#endif // SECONDO_ALGEBRAS_CHESS_MATERIAL_HPP
