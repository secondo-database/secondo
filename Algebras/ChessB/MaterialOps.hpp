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

#ifndef SECONDO_ALGEBRAS_CHESS_MATERIALOPS_HPP
#define SECONDO_ALGEBRAS_CHESS_MATERIALOPS_HPP

#include <cmath>
#include <numeric>
#include <functional>
#include <algorithm>

#include "Material.hpp"

/*
2. operators on Material objects implementing piececount
    and approx functionality, see ChessAlgebra.examples for
    further information.

*/

struct piececount_material_op : std::binary_function< Material, Piece, int >
{
    int operator () ( const Material& m, const Piece& p ) const
    {
        return m.pieces[ p.get() - 2 ];
    }
};

struct piececount_smat_op : std::binary_function< Material, CcString, int >
{
    int operator () ( const Material& m, const CcString& piece ) const
    {
        PIECE_TYPE pt = PT_UNDEFINED;
        try {
            pt = Piece::from_agent_type( piece.GetValue() );
        }
        catch( const std::exception& )
        {
            PIECE p = UNDEFINED;
            try {
                p = Piece::from_agent( piece.GetValue() );
            }
            catch( const std::exception& ){
                throw std::runtime_error( "Unknown Piece" );
            }
            if ( p < BLACK_KNIGHT || p > WHITE_KING )
                throw std::runtime_error( "Unknown Piece" );
            return m.pieces[ p - 2 ];
        }
        if ( pt < PT_PAWN || pt > PT_KING )
            throw std::runtime_error( "Unknown Piece" );
        PIECE p1 = Piece( pt, WHITE ).get();
        PIECE p2 = Piece( pt, BLACK ).get();
        return m.pieces[ p1 - 2 ] + m.pieces[ p2 - 2 ];
    }
};

struct approx_material_op : std::binary_function< Material, Material, bool >
{
    bool operator () ( const Material& m1, const Material& m2 ) const
    {
        double sum = 0.0;
        for( int i = 0; i < 12; ++i )
            sum += Piece::value(i + 2) * m1.pieces[i]
                 - Piece::value(i + 2) * m2.pieces[i];
        return abs( sum ) < 0.2;
    }
};

#endif // SECONDO_ALGEBRAS_CHESS_MATERIALOPS_HPP
