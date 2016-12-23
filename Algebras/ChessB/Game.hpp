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


[1] Game.hpp

1 Defines and includes

*/

#ifndef SECONDO_ALGEBRAS_CHESS_GAME_HPP
#define SECONDO_ALGEBRAS_CHESS_GAME_HPP

#include <sstream>
#include <utility>

#include "Attribute.h"
#include "StandardTypes.h"
#include "ListStream.hpp"
#include "Type.hpp"
#include "../../Tools/Flob/DbArray.h"
#include "Position.hpp"
#include "Piece.hpp"
#include "Ply.hpp"

/*
2. class Game represents game type

*/


struct Game : public Attribute
{
    static TypeConstructor tc;
    static const std::string& name()
    {
        static const std::string name( "chessgame" );
        return name;
    }

    typedef std::pair< CcString, CcString > tag_t;
    typedef DbArray< tag_t > tags_t;
    typedef DbArray< Ply > moves_t;

    Game() {}
    Game( size_t tag_count, size_t move_count )
        :Attribute(true),  tags(tag_count), moves(move_count),defined_(true) {}
    Game( undef_t def )
        : Attribute(true),tags(0), moves(0), defined_( def == DEFINED ) {}

    tags_t tags;
    const tag_t get_tag( int index ) const
    {
        tag_t s;
        tags.Get( index, s );
        return s;
    }
    void add_tag( const std::string& key, const std::string& value )
    {
        tags.Append( std::make_pair( CcString(true, key), CcString(true, value) ) );
    }

    moves_t moves;
    const Ply get_move( int index ) const
    {
        Ply s;
        moves.Get( index, s );
        return s;
    }

    // pure virtual functions of class Attribute
    virtual int NumOfFLOBs() const { return 2; }
    virtual Flob* GetFLOB( const int i )
    {
        return 0 == i ? static_cast<Flob*>(&tags) : static_cast<Flob*>(&moves);
    }

    virtual bool IsDefined() const { return defined_; }
    virtual void SetDefined( bool def ) { defined_ = def; }

    virtual bool Adjacent( const Attribute* ) const { return 0; }
    virtual int Compare( const Attribute* other ) const { return 0; }
    virtual size_t Sizeof() const { return sizeof( *this ); }

    virtual Attribute* Clone() const
    {
        Game *g = new Game( tags.Size(), moves.Size() );
        for( int i = 0; i < tags.Size(); ++i )
            g->tags.Put( i, get_tag( i ) );
        for( int j = 0; j < moves.Size(); ++j )
            g->moves.Put( j, get_move( j ) );
        g->defined_ = defined_;
        return g;
    }

    virtual void CopyFrom( const Attribute* other )
    {
        const Game* g = dynamic_cast<const Game*>( other );
        for( int i = 0; i < g->tags.Size(); ++i )
            tags.Append( g->get_tag( i ) );
        for( int j = 0; j < g->moves.Size(); ++j )
            moves.Append( g->get_move( j ) );
        defined_ = g->defined_;
    }

    virtual size_t HashValue() const
    {
        size_t hash = 0;
        for( int i = 0; i < tags.Size(); ++i )
        {
            const tag_t& tag = get_tag( i );
            hash += tag.first.HashValue() + tag.second.HashValue();
            hash *= 2;
        }

        for( int j = 0; j < moves.Size(); ++j )
        {
            const Ply& ply = get_move( j );
            hash += ply.HashValue();
            hash *= 2;
        }
        return hash;
    }

    virtual std::ostream& Print( std::ostream& os ) const
    {
        for( int i = 0; i < tags.Size(); ++i )
        {
            const tag_t& tag = get_tag( i );
            os << "[" << tag.first.GetValue()
               << " " << tag.second.GetValue() << "]\n";
        }
        os << "\n";

        for( int j = 0; j < moves.Size(); ++j )
        {
            os << j/2 + 1 << "." << get_move( j ) << " ";
            if ( ++j < moves.Size() )
                os << get_move( j ) << " ";
        }
        return os << "\n\n";
    }

    static Word In(  const ListExpr, const ListExpr i,
        const int, ListExpr&, bool& correct )
    {
        Game* g = new Game( DEFINED );
        try {
            list_istream is(i), info_l, moves_l;
            is >> info_l >> moves_l;

            while( ! info_l.end() )
            {
                list_istream entry_l;
                info_l >> entry_l;
                std::string key, value;
                entry_l >> key >> value;
                g->tags.Append( std::make_pair( CcString( true, key ),
                                          CcString( true,value  ) ) );
            }

            while( ! moves_l.end() )
            {
                list_istream ply_l;
                moves_l >> ply_l;
                Ply ply;
                ply_l >> ply;
                g->moves.Append( ply );
            }
            correct = true;
            return SetWord( g );
        }
        catch( const std::exception& e ) {
            cmsg.inFunError( e.what() );
            delete g;
        }
        correct = false;
        return SetWord( new Game( UNDEF ) );
    }

    static ListExpr Out( ListExpr, Word value )
    {
        Game* g = static_cast<Game*>( value.addr );
        list_ostream tags_l;
        for( int i = 0; i < g->tags.Size(); ++i )
        {
            const tag_t& tag = g->get_tag( i );
            tags_l << ( list_ostream() << tag.first.GetValue()
                                       << tag.second.GetValue() );
        }

        list_ostream moves_l;
        for( int j = 0; j < g->moves.Size(); ++j )
            moves_l << g->get_move( j );

        return list_ostream() << tags_l << moves_l;
    }

    static bool KindCheck( ListExpr type, ListExpr& )
        { return nl->IsEqual( type, "chessgame" ); }
    static Word Create( const ListExpr )
        { return SetWord( new Game( DEFINED ) ); }
    static void Delete( const ListExpr typeInfo, Word& w )
        { delete static_cast< Game* >(w.addr); }
    static void Close( const ListExpr typeInfo, Word& w )
        { delete static_cast< Game* >(w.addr); }
    static Word Clone( const ListExpr typeInfo, const Word& w )
        { return SetWord( static_cast< Game* >(w.addr)->Clone() ); }
    static int SizeOfObj()
        { return sizeof( Game ); }
    static void* cast( void* addr )
        { return new (addr) Game; }
    friend std::ostream& operator<< ( std::ostream& os, const Game& g )
        { return g.Print( os ); }

    static ListExpr Property()
    {
        return ( list_ostream()
            << ( list_ostream() << "Signature" << "Example Type List" )
            << ( list_ostream() << "-> DATA" << "chessgame" ) );
    }

protected:
    bool defined_;

private:
    Game( const Game& );
    Game& operator= ( const Game& );
};

#endif // SECONDO_ALGEBRAS_CHESS_GAME_HPP
