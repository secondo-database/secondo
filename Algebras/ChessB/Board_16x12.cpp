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


#include "Board_16x12.hpp"

namespace {
    #define OUTSIDE     UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED

    #define UNDEFINED_ROW     UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED, \
        UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED, \
        UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED, \
        UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED

    #define EMPTY_ROW     UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED, \
        NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, \
        UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED
}


/*

2 constructs a chess board with agents on initial positions

*/
const Board_16x12::board_t INITIAL_BOARD = { {
    UNDEFINED_ROW, UNDEFINED_ROW, OUTSIDE,
    WHITE_ROOK, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN,
    WHITE_KING, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK,
    OUTSIDE, OUTSIDE,
    WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN,
    WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN,
    OUTSIDE, EMPTY_ROW, EMPTY_ROW, EMPTY_ROW, EMPTY_ROW, OUTSIDE,
    BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN,
    BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN,
    OUTSIDE, OUTSIDE,
    BLACK_ROOK, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN,
    BLACK_KING, BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK,
    OUTSIDE, UNDEFINED_ROW, UNDEFINED_ROW
} };


/*

3 constructs an empty chess board

*/

const Board_16x12::board_t EMPTY_BOARD = { {
    UNDEFINED_ROW, UNDEFINED_ROW,
    EMPTY_ROW, EMPTY_ROW, EMPTY_ROW, EMPTY_ROW,
    EMPTY_ROW, EMPTY_ROW, EMPTY_ROW, EMPTY_ROW,
    UNDEFINED_ROW, UNDEFINED_ROW
} };
