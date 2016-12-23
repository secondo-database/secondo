#ifndef SECONDO_ALGEBRAS_CHESS_PLYT_HPP
#define SECONDO_ALGEBRAS_CHESS_PLYT_HPP

#include <iostream>
#include <sstream>
#include <boost/cstdint.hpp>

#include "Field.hpp"
#include "Piece.hpp"


enum PLY_TYPE { PLY_ORDINARY, PLY_PROMOTION, PLY_ENPASSANT, PLY_CASTLING };
enum PLY_STATE { PLY_NONE, PLY_CHECK, PLY_MATE, PLY_STALEMATE };

class PlyT
{
public:
    PlyT() {}
    explicit PlyT( boost::uint32_t value ) : value_(value) {}
    PlyT( Field from,
          Field to,
          PIECE agent,
          boost::uint8_t old_state,
          PLY_TYPE type = PLY_ORDINARY,
          PIECE_TYPE captured_type = PT_NONE,
          PLY_STATE state = PLY_NONE ) : value_(0)
    {
        value_ |= from.file;
        value_ |= from.row      <<  3;
        value_ |= to.file       <<  6;
        value_ |= to.row        <<  9;
        value_ |= agent         << 12;
        value_ |= captured_type << 16;
        value_ |= type          << 19;
        value_ |= state         << 21;
        value_ |= old_state     << 23;
    }

    boost::uint32_t value() const { return value_; }
    PLY_TYPE type() const { return PLY_TYPE( type_() ); }

    boost::uint8_t old_state() const { return boost::uint8_t( old_state_() ); }
    void old_state( PLY_STATE state )
        { value_ = (value_ & 0xFF9FFFFF) | state << 21; }

    Field from() const { return Field( from_file_(), from_row_() ); }
    Field to() const  { return Field( to_file_(), to_row_() ); }

    Piece agent() const
    {
        return is_promotion() ?
            Piece( PT_PAWN, COLOR( color_() ) ) :
            Piece( PIECE( agent_() ) );
    }

    Piece captured() const
    {
        return Piece( PIECE_TYPE( captured_type_() ), ! COLOR( color_() ) );
    }

    COLOR color() const { return COLOR( color_() ); }
    COLOR turn() const { return COLOR( color_() ); }

    bool is_capture() const { return captured().type() != PT_NONE; }

    bool is_enpassant() const
        { return PLY_ENPASSANT == static_cast<PLY_TYPE>( type_() ); }
    Field enpassant_field() const
    {
        return is_enpassant() ?
            Field( to_file_(), to_row_() + 1 - 2 * color_() )
            : Field( UNDEF );
    }

    bool is_promotion() const { return PLY_PROMOTION == type_(); }
    Piece promoted_to() const
        { return is_promotion() ? Piece( PIECE(agent_()) ) : Piece( NONE ); }

    bool is_castling() const { return PLY_CASTLING == type_(); }
    bool is_short_castling() const { return 6 == to_file_(); }

    void state( PLY_STATE new_state ) { old_state( new_state ); }
    PLY_STATE state() const { return PLY_STATE( state_() ); }
    bool is_check() const { return PLY_CHECK == state_(); }
    bool is_mate() const { return PLY_MATE == state_(); }
    bool is_stalemate() const { return PLY_STALEMATE == state_(); }

protected:
    boost::uint32_t value_;

    boost::uint32_t from_file_() const     { return value_       &  7; } //  0 -  2
    boost::uint32_t from_row_() const      { return value_ >>  3 &  7; } //  3 -  5
    boost::uint32_t to_file_() const       { return value_ >>  6 &  7; } //  6 -  8
    boost::uint32_t to_row_() const        { return value_ >>  9 &  7; } //  9 - 11
    boost::uint32_t agent_() const         { return value_ >> 12 & 15; } // 12 - 15
    boost::uint32_t captured_type_() const { return value_ >> 16 &  7; } // 16 - 18
    boost::uint32_t type_() const          { return value_ >> 19 &  3; } // 19 - 20
    boost::uint32_t state_() const         { return value_ >> 21 &  3; } // 21 - 22
    boost::uint32_t old_state_() const     { return value_ >> 23 & 63; } // 23 - 31

    boost::uint32_t color_() const         { return value_ >> 12 &  1; }
};

#endif // SECONDO_ALGEBRAS_CHESS_PLYT_HPP
