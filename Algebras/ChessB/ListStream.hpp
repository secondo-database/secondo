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


[1] ListStream.hpp

1 Defines and includes

*/

#ifndef SECONDO_LISTSTREAM_HPP
#define SECONDO_LISTSTREAM_HPP

#include <string>
#include <stdexcept>
#include "NestedList.h"
#include "QueryProcessor.h"
#include <boost/cstdint.hpp>


using namespace std;
using boost::uint8_t;

template< typename T > T from_atom( ListExpr instance );
template<> int from_atom<int>( ListExpr instance );
template<> double from_atom<double>( ListExpr instance );
template<> bool from_atom<bool>( ListExpr instance );
template<> string from_atom<string>( ListExpr instance );
template<> uint8_t from_atom<uint8_t>( ListExpr instance );

/*

2. stream classes provide technical stream operators <<,  >>
    on various secondo ListExpr
*/


class list_istream
{
    ListExpr list_;
    int position_;

public:
    list_istream( ListExpr list = 0 ) : list_(list), position_(1) {}

    bool end() const
    {
        return position_ > nl->ListLength( list_ ) ;
    }

    size_t size() const { return nl->ListLength( list_ ); }

    list_istream& operator >> ( list_istream& ls )
    {
        ListExpr current = nl->Nth( position_++, list_ );
        if ( nl->IsAtom( current ) )
            throw std::runtime_error( "List expected - got atom." );

        ls.list_ = current;
        return *this;
    }

    list_istream& operator >> ( int& t )
    {
        t = from_atom<int>( nl->Nth( position_++, list_ ) );
        return *this;
    }

    list_istream& operator >> ( double& t )
    {
        t = from_atom<double>( nl->Nth( position_++, list_ ) );
        return *this;
    }

    list_istream& operator >> ( bool& t )
    {
        t = from_atom<bool>( nl->Nth( position_++, list_ ) );
        return *this;
    }

    list_istream& operator >> ( string& t )
    {
        t = from_atom<string>( nl->Nth( position_++, list_ ) );
        return *this;
    }
};

class ChessBSymbol
{
    string s_;
public:
    ChessBSymbol( const string& s ) : s_(s){}
    const string& get() const { return s_; }
};

class list_ostream
{
    ListExpr list_;

public:
    list_ostream() : list_(0) {}

    operator ListExpr() const { return list_; }

    list_ostream& operator << ( const list_ostream& ls )
    {
        if ( ! list_  )
            list_ = nl->Cons( ls, 0 );
        else
            nl->Append( nl->End(list_), ls );
        return *this;
    }

    list_ostream& operator << ( int n )
    {
        if ( ! list_  )
            list_ = nl->Cons( nl->IntAtom( n ), 0 );
        else
            nl->Append( nl->End(list_), nl->IntAtom( n ) );
        return *this;
    }

    list_ostream& operator << ( const char* s )
    {
        return *this << string(s);
    }

    list_ostream& operator << ( const std::string& s )
    {
        ListExpr se = s.length() < 3 * STRINGSIZE ?
            nl->StringAtom( s ) : nl->TextAtom( s );

        if ( ! list_  )
            list_ = nl->Cons( se, 0 );
        else
            nl->Append( nl->End(list_), se );

        return *this;
    }

    list_ostream& operator << ( double d )
    {
        if ( ! list_  )
            list_ = nl->Cons( nl->RealAtom( d ), 0 );
        else
            nl->Append( nl->End(list_), nl->RealAtom( d ) );
        return *this;
    }

    list_ostream& operator << ( bool b )
    {
        if ( ! list_  )
            list_ = nl->Cons( nl->BoolAtom( b ), 0 );
        else
            nl->Append( nl->End(list_), nl->BoolAtom( b ) );
        return *this;
    }

    list_ostream& operator << ( const ChessBSymbol& s )
    {
        if ( ! list_  )
            list_ = nl->Cons( nl->SymbolAtom( s.get() ), 0 );
        else
            nl->Append( nl->End(list_), nl->SymbolAtom( s.get() ) );
        return *this;
    }

    friend ostream& operator << ( ostream& os, const list_ostream& le )
    {
        return os << nl->ToString( le );
    }
};

#endif // SECONDO_LISTSTREAM_HPP
