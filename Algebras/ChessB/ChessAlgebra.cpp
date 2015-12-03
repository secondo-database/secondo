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


[1] ChessAlgebra.cpp

1 Defines and includes

*/

#include <stdexcept>
#include "Algebra.h"
#include "Symbols.h"

#include "OperatorsInfo.hpp"
#include "TypeMapping.hpp"
#include "ValueMapping.hpp"

#include "Field.hpp"
#include "Piece.hpp"
#include "Ply.hpp"
#include "Material.hpp"
#include "Position.hpp"
#include "Game.hpp"

#include "FieldOps.hpp"
#include "PieceOps.hpp"
#include "PlyOps.hpp"
#include "MaterialOps.hpp"
#include "PositionOps.hpp"
#include "GameOps.hpp"
#include "readpgn_op.hpp"
#include "GenericOps.hpp"

// type information  for secondo user

//-----------------------------------------------------------------------------
static ConstructorInfo field_info( "field", "-> DATA", "field",
    "<stringvalue>", "\"a4\"", "Field of a chess board." );
static ConstructorInfo piece_info( "piece", "-> DATA", "piece",
    "<stringvalue>", "King", "Chess piece." );
static ConstructorInfo ply_info( "chessmove", "-> DATA", "chessmove",
    "(string x 3 int string int string int x 3)",
    "(\"King\" \"queen\" \"g\" 1 \"g\" 2 \"Kg1xg2\" 0 0 0)", "Chess move." );
static ConstructorInfo material_info( "material", "-> DATA", "material",
    "(int ... )", "(8 8 2 2 2 2 2 2 1 1 1 1)", "Count of pieces" );
static ConstructorInfo position_info( "position", "-> DATA", "position",
    "(int (string...)(...)...) int int", "...", "..." );
static ConstructorInfo game_info( "chessgame", "-> DATA", "chessgame",
    "(((string string) ...)(chessmove ...))", "()", "Chess game." );

TypeConstructor Game::tc( "chessgame", Game::Property, Game::Out,
    Game::In, 0, 0, Game::Create, Game::Delete, 0, 0, Game::Close,
    Game::Clone, Game::cast, Game::SizeOfObj, Game::KindCheck );

/*

3 Constructs new algebra.object.  Adds operators and types

*/

static Type<Field> t1(field_info);
static Type<Piece> t2(piece_info);
static Type<Ply>   t3(ply_info);
static Type<Material> t4( material_info );
static Type<Position> t5(position_info);

struct ChessAlgebra : Algebra
{
    ChessAlgebra()
    {

        AddTypeConstructor( &t1 );

        AddUnaryOperator< field_ctor_op >( field_ctor_info );
        AddUnaryOperator< iswhite_field_op >( iswhite_field_info );
        AddBinaryOperator< equal_to<Field> >( equals_field_info );
        AddBinaryOperator< less<Field> >( less_field_info );
        AddBinaryOperator< greater<Field> >( greater_field_info );
        AddUnaryOperator< file_op >( file_info );
        //DisplayTTY& d = DisplayTTY::GetInstance();

        AddUnaryOperator< row_op >( row_info );
        AddUnaryOperator< north_op >( north_info );
        AddUnaryOperator< northeast_op >( northeast_info );
        AddUnaryOperator< east_op >( east_info );
        AddUnaryOperator< southeast_op >( southeast_info );
        AddUnaryOperator< south_op >( south_info );
        AddUnaryOperator< southwest_op >( southwest_info );
        AddUnaryOperator< west_op >( west_info );
        AddUnaryOperator< northwest_op >( northwest_info );
        AddBinaryOperator< left_op >( left_info );
        AddBinaryOperator< right_op >( right_info );
        AddBinaryOperator< above_op >( above_info );
        AddBinaryOperator< below_op >( below_info );
        AddBinaryOperator< is_neighbor_op >( is_neighbor_info );
        AddUnaryStreamOperator< neighbors_op >( neighbors_info );

        AddTypeConstructor(&t2);
        AddUnaryOperator< piece_ctor_op >( piece_ctor_info );
        AddUnaryOperator< iswhite_piece_op >( iswhite_piece_info );
        AddBinaryOperator< is_op >( is_info );
        AddBinaryOperator< equal_to< Piece > >( equals_piece_info );
        AddBinaryOperator< samecolor_op >( samecolor_info );
        AddUnaryOperator< piecevalue_op >( piecevalue_info );

        AddTypeConstructor( &t3 );
        AddBinaryOperator< equal_to<Ply> >( equals_ply_info );
        AddUnaryOperator< startfield_op >( startfield_info );
        AddUnaryOperator< endfield_op >( endfield_info );
        AddUnaryOperator< agent_op >( agent_info );
        AddUnaryOperator< captures_op >( captures_info );
        AddUnaryOperator< captured_op >( captured_info );
        AddUnaryOperator< check_op >( check_info );
        AddUnaryOperator< is_mate_op >( is_mate_info );
        AddUnaryOperator< is_stalemate_op >( is_stalemate_info );
        AddUnaryOperator< is_castling_op >( is_castling_info );
        AddUnaryOperator< is_enpassant_op >( is_enpassant_info );
        AddUnaryOperator< enpassant_field_op >( enpassant_field_info );

        AddTypeConstructor( &t4 );
        AddBinaryOperator< piececount_material_op >(piececount_material_info);
        AddBinaryOperator< equal_to<Material> >( equals_material_info );
        AddBinaryOperator< less<Material> >( less_material_info );
        AddBinaryOperator< greater<Material> >( greater_material_info );
        AddBinaryOperator< approx_material_op >( approx_material_info );

        AddTypeConstructor( &t5 );
        // unary position operators
        AddUnaryOperator< pieces_op >( pieces_info );
        AddUnaryOperator< moveNo_op >( moveNo_info );
        AddUnaryOperator< checkmate_op >( checkmate_info );
        AddUnaryOperator< stalemate_op >( stalemate_info );

        // binary position operators
        AddBinaryOperator< includes_op >( includes_info );
        AddBinaryOperator< piececount_position_op >( piececount_position_info);
        AddBinaryOperator< piececount_spos_op >( piececount_spos_info );
        AddBinaryOperator< equal_to<Position> >( equals_position_info );
        AddBinaryOperator< less<Position> >( less_position_info );
        AddBinaryOperator< greater<Position> >( greater_position_info );
        AddBinaryOperator< approx_position_op >( approx_position_info );
        AddBinaryOperator< attackcount_op >( attackcount_info );
        AddBinaryOperator< protectcount_op >( protectcount_info );
        AddBinaryOperator< apply_ply_op >( apply_ply_info );

        // ternary position operators
        AddTernaryOperator< posrange_op >( posrange_info );
        AddTernaryOperator< attacked_by_p_op >( attacked_by_p_info );
        AddTernaryOperator< attacked_by_f_op >( attacked_by_f_info );
        AddTernaryOperator< attacked_from_p_op >( attacked_from_p_info );
        AddTernaryOperator< attacked_from_f_op >( attacked_from_f_info );
        AddTernaryOperator< protected_by_p_op >( protected_by_p_info );
        AddTernaryOperator< protected_by_f_op >( protected_by_f_info );
        AddTernaryOperator< protected_from_p_op >( protected_from_p_info );
        AddTernaryOperator< protected_from_f_op >( protected_from_f_info );

        // stream producing operators
        AddUnaryStreamOperator< pos_fields_op >( pos_fields_info );
        AddUnaryStreamOperator< pos_moves_op >( pos_moves_info );
        AddUnaryStreamOperator< pos_moves_blocked_op >(pos_moves_blocked_info);
        AddBinaryStreamOperator< piece_moves_op >( piece_moves_info );
        AddBinaryStreamOperator< p_moves_blocked_op >( p_moves_blocked_info );

        AddTypeConstructor( &Game::tc );
        Game::tc.AssociateKind( Kind::DATA() );
        AddBinaryOperator< getkey_op >( getkey_info );
        AddBinaryOperator< getposition_op >( getposition_info );
        AddBinaryOperator< getmove_op >( getmove_info );
        AddUnaryOperator< lastmove_op >( lastmove_info );

        AddUnaryStreamOperator< readpgn_op >( readpgn_info );
        AddUnaryStreamOperator< positions_op >( positions_info );
        AddUnaryStreamOperator< moves_op >( moves_info );
        AddUnaryStreamOperator<history_op>( history_info );

        // helpers
        AddUnaryOperator< even_op >( even_info );
        AddUnaryOperator< odd_op >( odd_info );
        AddUnaryStreamOperator< Ntuples_op<2> >( twotuples_info );
        //AddUnaryStreamOperator< ntuples_op >( ntuples_info );
        //AddBinaryFAggregateOperator< exists_op, void >( exists_info );
        //AddBinaryFAggregateOperator< forall_op, void >( forall_info );

        // demonstration of new aggregate functions
        const OperatorInfo stddev_info( "stddev", "stream(double) -> double",
            "_ stddev", "Compute standard deviation", "query r stddev" );
        AddUnaryAggregateOperator< stddev_op, CcReal >( stddev_info );

    }

    // templates for adding various operator types
    template< typename O, typename A >
    void AddUnaryAggregateOperator( const OperatorInfo& oi )
    {
        AddOperator( oi, unary_value_map< unary_aggregate< A, O > >,
                         unary_type_map< O > );
    }

    template< typename O, typename A >
    void AddBinaryFAggregateOperator( const OperatorInfo& oi )
    {
        AddOperator( oi, binary_value_map< binary_aggregate< A, O > >,
            binary_aggregate_type_map<A, typename O::result_type> );
    }

    template< typename O >
    void AddUnaryOperator( const OperatorInfo& oi )
    {
        AddOperator( oi, unary_value_map<O>, unary_type_map<O> );
    }

    template< typename O >
    void AddBinaryOperator( const OperatorInfo& oi )
    {
        AddOperator( oi, binary_value_map<O>, binary_type_map<O> );
    }

    template< typename O >
    void AddTernaryOperator( const OperatorInfo& oi )
    {
        AddOperator( oi, ternary_value_map<O>, ternary_type_map<O> );
    }

    template< typename O >
    void AddUnaryStreamOperator( const OperatorInfo& oi )
    {
        AddOperator( oi, unary_stream_value_map<O>, unary_stream_type_map<O> );
    }

    template< typename O >
    void AddBinaryStreamOperator( const OperatorInfo& oi )
    {
        AddOperator(oi, binary_stream_value_map<O>, binary_stream_type_map<O>);
    }
};

extern "C" Algebra* InitializeChessBAlgebra( NestedList*, QueryProcessor* )
{
    return new ChessAlgebra();
}
