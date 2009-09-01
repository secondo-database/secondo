#ifndef SECONDO_ALGEBRAS_CHESS_PLYOPS_HPP
#define SECONDO_ALGEBRAS_CHESS_PLYOPS_HPP

#include <functional>
#include "Ply.hpp"

struct startfield_op : std::unary_function< Ply, Field* >
{
    Field* operator () ( const Ply& ply )
    {
        return new Field( ply.from() );
    }
};

struct endfield_op : std::unary_function< Ply, Field* >
{
    Field* operator () ( const Ply& ply )
    {
        return new Field( ply.to() );
    }
};

struct agent_op : std::unary_function< Ply, Piece* >
{
    Piece* operator () ( const Ply& ply )
    {
        return new Piece( ply.agent() );
    }
};

struct captures_op : std::unary_function< Ply, bool >
{
    bool operator () ( const Ply& ply )
    {
        return ply.is_capture();
    }
};

struct captured_op : std::unary_function< Ply, Piece* >
{
    Piece* operator () ( const Ply& ply )
    {
        return new Piece( ply.captured() );
    }
};

struct check_op : std::unary_function< Ply, bool >
{
    bool operator () ( const Ply& ply )
    {
        return ply.is_check();
    }
};

struct is_mate_op : std::unary_function< Ply, bool >
{
    bool operator () ( const Ply& ply )
    {
        return ply.is_mate();
    }
};

struct is_stalemate_op : std::unary_function< Ply, bool >
{
    bool operator () ( const Ply& ply )
    {
        return ply.is_stalemate();
    }
};

struct is_castling_op : std::unary_function< Ply, bool >
{
    bool operator () ( const Ply& ply )
    {
        return ply.is_castling();
    }
};

struct is_enpassant_op : std::unary_function< Ply, bool >
{
    bool operator () ( const Ply& ply )
    {
        return ply.is_enpassant();
    }
};

struct enpassant_field_op : std::unary_function< Ply, Field* >
{
    Field* operator () ( const Ply& ply )
    {
        return new Field( ply.enpassant_field() );
    }
};

#endif // SECONDO_ALGEBRAS_CHESS_PLYOPS_HPP
