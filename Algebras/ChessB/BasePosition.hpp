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


[1] BasePosition

1 Defines and includes

*/


#ifndef SECONDO_ALGEBRAS_CHESS_BASE_POSITION_HPP
#define SECONDO_ALGEBRAS_CHESS_BASE_POSITION_HPP

#include <boost/cstdint.hpp>
#include "Ply.hpp"
#include "Board_16x12.hpp"

/*
2 class basic_position

    provides methods for creating and manipulating position
    objects. A position object represents a chess position.

    Basic functionality is:

    - iterators for iterating over the underlying board representation
    - move: to make a single chess move
    - turn: to increment/decrement players turn
    - setting and getting flags for special match actions, e.g.
      en_passant and castling.

*/

template< class BoardT >
class basic_position
{
public:
    typedef BoardT board_t;
    typedef typename board_t::moves_t moves_t;

    // Don't initialize any members here, since Secondo uses placement new
    // on already initialized memory for serialization.
    basic_position() {}

    // Constructs new position. Every BoardT-type (board representation class)
    // provides constant values EMPTY_BOARD and INITIAL_BOARD to begin with
    // position construction. Set turn to WHITE or BLACK. Set state to 15 for
    // initial position and to 0 for an empty position.
    basic_position( const typename BoardT::board_t& board,
                    COLOR turn, boost::uint8_t state, boost::uint8_t m )
        : board_(board), turn_(turn), state_(state), move_number_(m) {}

    // Provides sequentially access to underlying board representation.
    typedef typename BoardT::const_iterator const_iterator;
    const_iterator begin() const { return board_.begin(); }
    const_iterator end() const { return board_.end(); }
    const_iterator iter( const Field& f ) const { return board_.iter(f); }

    typedef typename BoardT::iterator iterator;
    iterator begin() { return board_.begin(); }
    iterator end() { return board_.end(); }
    iterator iter( const Field& f ) { return board_.iter(f); }

    int move_number() const { return move_number_; }

    // Provides random access to to underlying board representation.
    PIECE& operator[] ( const Field& f ) { return board_[ f ]; }
    const PIECE& operator[] ( const Field& f ) const { return board_[ f ]; }

    void move( const Piece& piece, const Field& from, const Field& to )
    {
        board_[ from ] = NONE;
        board_[ to ] = piece.get();
    }

    bool operator == ( const basic_position< BoardT >& other ) const
    {
        return board_ == other.board_ && turn_ == other.turn_
            && state_ == other.state_ && move_number_ == other.move_number_;
    }

    bool is_enpassant_possible() const
    {
        return 1 == enpassant_();
    }
    int enpassant_file() const
    {
        return enpassant_file_();
    }

    void enable_enpassant( boost::uint8_t col )
    {
        state_ = ( state_ & 0x0F ) | ( col << 5 ) | 0x10;
    }
    void disable_enpassant()
    {
        state_ &= 0x0F;
    }

    bool is_castling_possible( bool is_long, COLOR color ) const
    {
        return castling_( is_long, color ) > 0;
    }
    void disable_castling( bool is_long, COLOR color )
    {
        state_ &= 0xFF ^ castling_bit_( is_long, color );
    }

    boost::uint8_t state() const { return state_; }
    void state( boost::uint8_t value )
    {
        state_ = value;
    }
    COLOR turn() const { return turn_; }

    void increment_turn()
    {
        turn_ = ! turn_;
        ++move_number_;
    }

    void decrement_turn()
    {
        turn_ = ! turn_;
        --move_number_;
    }

    static const moves_t& moves( PIECE_TYPE t ) { return board_t::moves( t ); }
    Field field( const const_iterator& it ) const { return board_.field( it ); }
protected:
    BoardT  board_;
    COLOR   turn_;
    boost::uint8_t state_;
    boost::uint8_t move_number_;

    boost::uint8_t enpassant_() const { return state_ >>  4 & 1; }
    boost::uint8_t enpassant_file_() const { return state_ >>  5 & 7; }
    boost::uint8_t castling_( bool is_long, COLOR color ) const
    {
        return state_ & castling_bit_( is_long, color );
    }
    boost::uint8_t castling_bit_( bool is_long, COLOR color ) const
    {
        return 1 << ( color + 2 * ( is_long ? 1 : 0 ) );
    }
};

typedef basic_position< Board_16x12 > position_t;
const position_t INITIAL_POSITION = position_t(INITIAL_BOARD, WHITE, 0x0F, 0);
const position_t EMPTY_POSITION = position_t(EMPTY_BOARD, WHITE, 0x00, 0);

#endif // SECONDO_ALGEBRAS_CHESS_BASE_POSITION_HPP
