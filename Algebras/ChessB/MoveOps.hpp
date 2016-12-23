/*


*/

#ifndef SECONDO_ALGEBRAS_CHESS_MOVE_OPS_HPP
#define SECONDO_ALGEBRAS_CHESS_MOVE_OPS_HPP

#include <functional>
#include "Field.hpp"
#include "Piece.hpp"
#include "Ply.hpp"
#include "Position.hpp"

struct apply_ply_op : std::binary_function< Position, Ply, Position* >
{
    Position* operator () ( Position& pos, const Ply& ply )
    {
        pos.move( ply.agent(), ply.from(), ply.to() );
        if ( ply.is_promotion() )
            pos[ ply.to() ] = ply.promoted_to().get();
        if ( ply.is_enpassant() )
            pos[ ply.enpassant_field() ] = NONE;

        if ( ply.is_castling() )
        {
            if ( WHITE == ply.color() )
            {
                if ( 2 == ply.to().file )
                    pos.move( Piece(WHITE_ROOK), Field(0, 0), Field(3, 0) );
                else
                    pos.move( Piece(WHITE_ROOK), Field(7, 0), Field(5, 0) );
            }
            else
            {
                if ( 2 == ply.to().file )
                    pos.move( Piece(BLACK_ROOK), Field(0, 7), Field(3, 7) );
                else
                    pos.move( Piece(BLACK_ROOK), Field(7, 7), Field(5, 7) );
           }
           pos.disable_castling( true, ply.color() );
           pos.disable_castling( false, ply.color() );
        }

        if ( PT_KING == ply.agent().type() )
        {
           pos.disable_castling( true, ply.color() );
           pos.disable_castling( false, ply.color() );
        }

        if ( PT_PAWN == ply.agent().type()
            && 2 == abs( ply.to().row - ply.from().row ) )
        {
            pos.enable_enpassant( uint8_t( ply.from().file ) );
        }
        else
            pos.disable_enpassant();

        if ( pos.is_castling_possible( true, WHITE )
            && ( Field(0, 0) == ply.from() || Field(0, 0) == ply.to() ) )
        {
            pos.disable_castling( true, WHITE );
        }
        else if ( pos.is_castling_possible( false, WHITE )
            && ( Field(7, 0) == ply.from() || Field(7, 0) == ply.to() ) )
        {
            pos.disable_castling( false, WHITE );
        }
        else if ( pos.is_castling_possible( true, BLACK )
            && ( Field(0, 7) == ply.from() || Field(0, 7) == ply.to() ) )
        {
            pos.disable_castling( true, BLACK );
        }
        else if ( pos.is_castling_possible( false, BLACK )
            && ( Field(7, 7) == ply.from() || Field(7, 7) == ply.to() ) )
        {
            pos.disable_castling( false, BLACK );
        }

        pos.increment_turn();
        return new Position(pos);
    }
};

struct revert_ply_op : std::binary_function< Position, Ply, Position* >
{
    Position* operator () ( Position& pos, const Ply& ply )
    {
        pos.decrement_turn();
        pos.move( ply.agent(), ply.to(), ply.from() );
        if ( ply.is_capture() )
            pos[ ply.to() ] = ply.captured().get();
        if ( ply.is_promotion() )
            pos[ ply.from() ] =  Piece( PT_PAWN, pos.turn() ).get();
        else if ( ply.is_enpassant() )
            pos[ ply.enpassant_field() ] = Piece( PT_PAWN, !pos.turn() ).get();
        else if ( ply.is_castling() )
        {
            if ( WHITE == ply.color() )
            {
                if ( 2 == ply.to().file )
                    pos.move( Piece(WHITE_ROOK), Field(3, 0), Field(0, 0) );
                else
                    pos.move( Piece(WHITE_ROOK), Field(5, 0), Field(7, 0) );
            }
            else
            {
                if ( 2 == ply.to().file )
                    pos.move( Piece(BLACK_ROOK), Field(3, 7), Field(0, 7) );
                else
                    pos.move( Piece(BLACK_ROOK), Field(5, 7), Field(7, 7) );
            }
        }
        pos.state( ply.old_state() );
        return new Position(pos);
    }
};

#endif // SECONDO_ALGEBRAS_CHESS_MOVE_OPS_HPP
