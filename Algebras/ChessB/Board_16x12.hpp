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


1 Defines and includes

*/

#ifndef SECONDO_ALGEBRAS_CHESS_BOARD_16X12_HPP
#define SECONDO_ALGEBRAS_CHESS_BOARD_16X12_HPP

#include <tr1/array>
#include "PlyT.hpp"
#include "Piece.hpp"


/*
2 class Board_16x12

    represents a chess board in 16x12 format which is
    technically much easier to handle  than a natural 8x8
    representation. Query german wikipedia for "Schachprogramme"
    for further information.

    Also definies move vectors of all agents

*/

class Board_16x12
{
public:
    typedef std::tr1::array< PIECE, 192 > board_t;
    typedef board_t::difference_type dir_t;

    Board_16x12(){}
    Board_16x12( const board_t& board ) : board_(board) {}

    // iterate over board
    typedef board_t::const_iterator const_iterator;
    const_iterator begin() const { return board_.begin(); }
    const_iterator end() const { return board_.end(); }
    const_iterator iter( const Field& f ) const
    {
        return board_.begin() + index(f);
    }

    typedef board_t::iterator iterator;
    iterator begin() { return board_.begin(); }
    iterator end() { return board_.end(); }
    iterator iter( const Field& f )
    {
        return board_.begin() + index(f);
    }

    // select pieces on a board
    PIECE& operator[] ( const Field& f ) { return board_[ index(f) ]; }
    const PIECE& operator[] ( const Field& f ) const
        { return board_[ index(f) ]; }

    // compare two board objects
    bool operator== ( const Board_16x12& other ) const
        { return board_ == other.board_; }

    // definition of agent moves
    struct moves_t
    {
        dir_t moves[8];
        int count;
        int steps;
    };

    // move vectors
    static const moves_t& moves( PIECE_TYPE type )
    {
        static const moves_t defs[5] =
        {
            { { -33, -31, -18, -14, 14, 18, 31, 33 }, 8, 1 },
            { { -17, -15,  15,  17,  0,  0,  0,  0 }, 4, 7 },
            { { -16,  -1,   1,  16,  0,  0,  0,  0 }, 4, 7 },
            { { -17, -16, -15,  -1,  1, 15, 16, 17 }, 8, 7 },
            { { -17, -16, -15,  -1,  1, 15, 16, 17 }, 8, 1 }
        };
        return defs[ type - 2 ];
    }

    Field field( const const_iterator& it ) const
    {
        dir_t offset = it - board_.begin();
        return Field( offset % 16 - 4, offset / 16 - 2 );
    }
private:
    int index( const Field& f ) const { return f.row * 16 + 36 + f.file; }
    board_t board_;
};

extern const Board_16x12::board_t EMPTY_BOARD;
extern const Board_16x12::board_t INITIAL_BOARD;

#endif // SECONDO_ALGEBRAS_CHESS_BOARD_16X12_HPP
