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


[1] FieldOps.hpp

1 Defines and includes

*/

#ifndef SECONDO_ALGEBRAS_CHESS_FIELDOPS_HPP
#define SECONDO_ALGEBRAS_CHESS_FIELDOPS_HPP

#include <string>
#include <functional>
#include "Field.hpp"


/*
2. operator definitions - for operating on field objects

    Basic functionality is to check for a field's color, a field's string
    representation and a fields position either in the grid or
    related to a neighbor field.

    See ChessAlgebra.examples for further information

*/

struct field_ctor_op : std::unary_function< CcString, Field* >
{
    Field* operator () ( const CcString& f )
    {
        std::string s = f.GetValue();
        if ( 2 == s.length() )
            return new Field(s[0] - 'a', s[1] - '1');
        return new Field(UNDEF);
    }
};

struct iswhite_field_op : std::unary_function< Field, bool >
{
    bool operator () ( const Field& field ) const { return field.is_white(); }
};

struct file_op : std::unary_function< Field, std::string >
{
    std::string operator () ( const Field& field )
    {
        return std::string( 1, static_cast< char >( field.file + 'a' ) );
    }
};

struct row_op : std::unary_function< Field, int >
{
    int operator () ( const Field& field )
    {
        return field.row + 1;
    }
};

struct north_op : std::unary_function< Field, Field* >
{
    Field* operator () ( const Field& field )
    {
         if ( 7 == field.row )
            return new Field( UNDEF );
         return new Field( field.file, field.row + 1 );
    }
};

struct east_op : std::unary_function< Field, Field* >
{
    Field* operator () ( const Field& field )
    {
        if ( 7 == field.file )
            return new Field( UNDEF );
        return new Field( field.file + 1, field.row );
    }
};

struct south_op : std::unary_function< Field, Field* >
{
    Field* operator () ( const Field& field )
    {
        if ( 0 == field.row )
            return new Field( UNDEF );
        return new Field( field.file, field.row - 1 );
    }
};

struct west_op : std::unary_function< Field, Field* >
{
    Field* operator () ( const Field& field )
    {
        if ( 0 == field.file )
            return new Field( UNDEF );
        return new Field( field.file - 1, field.row );
    }
};

struct northwest_op : std::unary_function< Field, Field* >
{
    Field* operator () ( const Field& field )
    {
        if ( 0 == field.file || 7 == field.row )
            return new Field( UNDEF );
        return new Field( field.file - 1, field.row + 1 );
    }
};

struct northeast_op : std::unary_function< Field, Field* >
{
    Field* operator () ( const Field& field )
    {
        if ( 7 == field.file || 7 == field.row )
            return new Field( UNDEF );
        return new Field( field.file + 1, field.row + 1 );
    }
};

struct southwest_op : std::unary_function< Field, Field* >
{
    Field* operator () ( const Field& field )
    {
        if ( 0 == field.file || 0 == field.row )
            return new Field( UNDEF );
        return new Field( field.file - 1, field.row - 1 );
    }
};

struct southeast_op : std::unary_function< Field, Field* >
{
    Field* operator () ( const Field& field )
    {
        if ( 7 == field.file || 0 == field.row )
            return new Field( UNDEF );
        return new Field( field.file + 1, field.row - 1 );
    }
};

struct is_neighbor_op : std::binary_function< Field, Field, bool >
{
    bool operator () ( const Field& f1, const Field& f2 )
    {
        int x = abs( f1.file - f2.file );
        int y = abs( f1.row - f2.row );
        return ( 1 == x || 0 == x ) && ( 1 == y || 0 == y ) && f1 != f2;
    }
};

struct left_op : std::binary_function< Field, Field, bool >
{
    bool operator () ( const Field& f1, const Field& f2 )
    {
        return f1.file + 1 == f2.file && f1.row == f2.row;
    }
};

struct right_op : std::binary_function< Field, Field, bool >
{
    bool operator () ( const Field& f1, const Field& f2 )
    {
        return f1.file - 1 == f2.file && f1.row == f2.row;
    }
};

struct above_op : std::binary_function< Field, Field, bool >
{
    bool operator () ( const Field& f1, const Field& f2 )
    {
        return f1.file == f2.file && f1.row - 1 == f2.row;
    }
};

struct below_op : std::binary_function< Field, Field, bool >
{
    bool operator () ( const Field& f1, const Field& f2 )
    {
        return f1.file == f2.file && f1.row + 1 == f2.row;
    }
};

class neighbors_op : public std::unary_function< Field, std::pair<bool, Field*> >
{
    Field self_;
    int dir_;

public:
    neighbors_op( const Field& f, ListExpr ) : self_(f), dir_(0){}

    std::pair<bool, Field*> operator()( const Field& )
    {
        static const int dirs[8][2] =
            { {0,1}, {1,1}, {1,0}, {1,-1}, {0,-1}, {-1,-1}, {-1,0}, {-1,1} };

        if ( dir_ >= 8 )
            return std::make_pair( false, new Field(UNDEF) );

        for( ; dir_ < 8; ++dir_ )
        {
            int file = self_.file + dirs[dir_][0];
            int row = self_.row + dirs[dir_][1];
            if ( file < 0 || file > 7 || row < 0 || row > 7 )
                continue;
            ++dir_;
            return std::make_pair( true, new Field(file, row) );
        }
        return std::make_pair( false, new Field(UNDEF) );
    }
};

#endif // SECONDO_ALGEBRAS_CHESS_FIELDOPS_HPP
