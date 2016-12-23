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


[1] Field.hpp

1 Defines and includes

*/

#ifndef SECONDO_ALGEBRAS_CHESS_FIELD_HPP
#define SECONDO_ALGEBRAS_CHESS_FIELD_HPP

#include <string>
#include <iostream>
#include "Attribute.h"
#include "ListStream.hpp"
#include "Type.hpp"

/*
2. class Field

    field objects represent a single field on the chess board

    Basic functionality is:

    - operators for comparing fields <,>,!=,==
    - dump a field object to a string, e.g. a1, a2, ..,h8
    - checking for fields color
    - technical functions to support secondo integration
      (e.g.  overidden virtual methods of Attribute)

*/

struct Field : public Attribute
{
    static const std::string& name()
    {
        static const std::string name( "field" );
        return name;
    }

    int file, row;

    Field(){}
    Field( int f, int r ) : Attribute(true),file(f), row(r)
    {
        /*
        if ( file < 0 || file > 7 )
            throw std::runtime_error( "File must be in range 'a'-'h'." );
        if ( row < 0 || row > 7 )
            throw std::runtime_error( "Row must be in range '1'-'8'." );
        */
       del.refs=1;
       del.SetDelete();
       SetDefined(true);
    }
    Field( undef_t undef ) : Attribute(true),file(0), row(0)  {
       del.refs=1;
       del.SetDelete();
       SetDefined(false);
        
    }
    Field( const std::string& s ) : Attribute(true) 
    {
        if ( s.length() != 2 )
            throw std::runtime_error( "Expecting string with length 2!" );
        file = s[0] - 'a';
        row = s[1] - '1';
        if ( file < 0 || file > 7 )
            throw std::runtime_error( "File must be in range 'a'-'h'." );
        if ( row < 0 || row > 7 )
            throw std::runtime_error( "Row must be in range '1'-'8'." );

        del.refs=1;
        del.SetDelete();
        SetDefined(true);
    }

    std::string to_string() const
    {
        std::string s( "a1" );
        s[0] += static_cast<char>( file );
        s[1] += static_cast<char>( row );
        return s;
    }

    bool is_white() const { return row % 2 != file % 2; }
    int index() const { return row * 8 + file; }

    // pure virtual functions of class Attribute
    virtual bool Adjacent( const Attribute* other ) const
    {
        const Field& f = *static_cast< const Field* >( other );
        return abs( index() - f.index() ) == 1;
    }

    virtual int Compare( const Attribute* other ) const
    {
        const Field& f = *static_cast< const Field* >( other );
        return index() - f.index();
    }

    virtual std::ostream& Print( std::ostream& os ) const
    {
        return os
            << static_cast<char>( file + 'a' )
            << static_cast<char>( row + '1' );
    }

    virtual void CopyFrom( const Attribute* other )
    {
        *this = Field( *static_cast< const Field* >( other ) );
    }

    virtual Attribute* Clone() const { return new Field( *this ); }
    virtual size_t Sizeof() const { return sizeof( *this ); }
    virtual size_t HashValue() const { return index(); }

    static Field In( ListExpr instance )
    {
        return Field( from_atom< std::string >( instance ) );
    }

    static ListExpr Out( const Field& field )
    {
        return nl->StringAtom( field.to_string() );
    }

    static bool KindCheck( ListExpr type, ListExpr& )
    {
        return nl->IsEqual( type, "field" );
    }

    friend bool operator == ( const Field& lhs, const Field& rhs )
    {
        return lhs.file == rhs.file && lhs.row == rhs.row;
    }

    friend bool operator != ( const Field& lhs, const Field& rhs )
    {
        return !( lhs == rhs );
    }

    friend bool operator < ( const Field& lhs, const Field& rhs )
    {
        return lhs.index() < rhs.index();
    }

    friend bool operator > ( const Field& lhs, const Field& rhs )
    {
        return lhs.index() > rhs.index();
    }

    friend std::ostream& operator << ( std::ostream& os, const Field& field )
    {
        return field.Print( os );
    }

    friend std::istream& operator >> ( std::istream& is, Field& field )
    {
        char file, row;
        is >> file >> row;
        field = Field( file - 'a', row - '1' );
        return is;
    }
};

#endif // SECONDO_ALGEBRAS_CHESS_FIELD_HPP
