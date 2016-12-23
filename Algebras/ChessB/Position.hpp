#ifndef SECONDO_ALGEBRAS_CHESS_POSITION_HPP
#define SECONDO_ALGEBRAS_CHESS_POSITION_HPP

#include <numeric>
#include <functional>
#include <algorithm>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
namespace bl = boost::lambda;

#include "Attribute.h"
#include "ListStream.hpp"
#include "Type.hpp"

#include "BasePosition.hpp"
#include "Board_16x12.hpp"
#include "Type.hpp"

class Position : public position_t, public Attribute
{
public:
    static const std::string& name()
    {
        static const std::string name( "position" );
        return name;
    }

    Position() {}
    Position( undef_t def )
        : position_t(EMPTY_POSITION), Attribute(def) { }
    Position( const position_t& pos ) : position_t(pos), Attribute(true){}

    // pure virtual functions of class Attribute
    virtual bool Adjacent( const Attribute* ) const { return 0; }
    virtual int Compare( const Attribute* other ) const { return 0; }
    virtual Attribute* Clone() const { return new Position( *this ); }
    virtual size_t Sizeof() const { return sizeof( *this ); }

    virtual size_t HashValue() const
    {
        size_t hash = std::accumulate( begin(), end(), 0 );
        return hash + move_number_ + state_ + turn_;
    }

    virtual std::ostream& Print( std::ostream& os ) const
    {
        os << "\nMove number: " << move_number() << ", ";
        os << "Turn: " << ( turn() == WHITE ? "white" : "black" ) << ", ";
        os << "Enpassant file: ";
        if ( is_enpassant_possible() )
            os << char(enpassant_file() + 'a') << "\n";
        else
            os << "None\n";
        os << "White castling[ long: "
           << ( is_castling_possible(true, WHITE) ? "yes" : "no" )
           << ",  short: "
           << ( is_castling_possible(false, WHITE) ? "yes" : "no" )
           << " ]\n";
        os << "Black castling[ long: "
           << ( is_castling_possible(true, BLACK) ? "yes" : "no" )
           << ",  short: "
           << ( is_castling_possible(false, BLACK) ? "yes" : "no" )
           << " ]\n\n";

        os << "    _________________\n";
        for ( int row = 7; row >= 0; --row )
        {
            os << " " << row << " | ";
            for ( int file = 0; file < 8; ++file )
                os  << Piece( (*this)[ Field(file, row) ] ).agent_s() << " ";
            os << "|\n";
        }
        os << "    -----------------\n     a b c d e f g h\n\n";
        return os;
    }

    friend std::ostream& operator<<( std::ostream& os, const Position& pos )
    {
        return pos.Print(os);
    }

    virtual void CopyFrom( const Attribute* other )
    {
        *this =  *static_cast< const Position* >( other );
    }

    virtual Position& operator=(const Position& src){
       position_t::operator=(src);
       Attribute::operator=(src);
       return *this;
    }


    static Position In( ListExpr instance )
    {
        int move, turn, state;
        list_istream is( instance ), fields;
        is >> move >> fields;
        if ( is.size() == 4 )
            is >> turn >> state;
        else
        {
            turn = move % 2 == 0;
            state = 0;
        }

        Position pos( position_t( EMPTY_BOARD, COLOR(turn), state, move ) );
        for( int y = 0; y < 8; ++y )
        {
            list_istream row;
            fields >> row;
            for( int x = 0; x < 8; ++x )
            {
                std::string agent;
                row >> agent;
                pos[ Field( x, y ) ] = Piece::from_agent( agent );
            }
        }
        return pos;
    }

    static ListExpr Out( const Position& pos )
    {
        if(!pos.IsDefined()){
           return nl->SymbolAtom("undef");
        }
        list_ostream ls;
        ls << pos.move_number();
        list_ostream fields;
        for( int y = 0; y < 8; ++y )
        {
            list_ostream row;
            for( int x = 0; x < 8; ++x )
                row << Piece( pos[ Field( x, y ) ] ).agent();
            fields << row;
        }
        return ls << fields << int(pos.turn()) << int(pos.state());
    }

    static bool KindCheck( ListExpr type, ListExpr& )
    {
        return nl->IsEqual( type, "position" );
    }

    bool operator < ( const Position& other ) const
    {
        return value_sum() < other.value_sum();
    }

    bool operator > ( const Position& other ) const
    {
        return value_sum() > other.value_sum();
    }

    bool operator == ( const Position& other ) const
    {
        return board_ == other.board_;
    }

    double value_sum() const
    {
        double sum = .0;
        std::for_each( begin(), end(), sum += bl::bind( Piece::value, bl::_1 ) );
        return sum;
    }

};

#endif // SECONDO_ALGEBRAS_CHESS_POSITION_HPP
