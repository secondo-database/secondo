#ifndef SECONDO_ALGEBRAS_CHESS_PIECEOPS_HPP
#define SECONDO_ALGEBRAS_CHESS_PIECEOPS_HPP

#include <string>
#include <functional>
#include "Piece.hpp"

struct piece_ctor_op : std::unary_function< CcString, Piece* >
{
    Piece* operator () ( const CcString& s )
    {
        return new Piece( Piece::from_agent( s.GetValue() ) );
    }
};

struct iswhite_piece_op : std::unary_function< Piece, bool >
{
    bool operator () ( const Piece& p ) { return p.is_white(); }
};

struct is_op : std::binary_function< Piece, Piece, bool >
{
    bool operator () ( const Piece& p1, const Piece& p2 )
        { return p1.type() == p2.type(); }
};

struct samecolor_op : std::binary_function< Piece, Piece, bool >
{
    bool operator () ( const Piece& p1, const Piece& p2 )
        { return p1.color() == p2.color(); }
};

struct piecevalue_op : std::unary_function< Piece, double >
{
    double operator () ( const Piece& p )
    {
        return Piece::value( p.get() );
    }
};

#endif // SECONDO_ALGEBRAS_CHESS_PIECEOPS_HPP
