#ifndef SECONDO_ALGEBRAS_CHESS_TYPES_PIECE_HPP
#define SECONDO_ALGEBRAS_CHESS_TYPES_PIECE_HPP

#include <map>
#include <string>
#include <iostream>
#include <stdexcept>
#include <tr1/array>

#include "Attribute.h"
#include "ListStream.hpp"
#include "Type.hpp"

template< typename T, typename P, int N >
class piece_mapper
{
public:
    typedef std::map< T, P > MAPPER_T;
    typedef std::tr1::array< T, N > AGENTS_ST;
    typedef typename AGENTS_ST::const_iterator iterator;

    piece_mapper( const AGENTS_ST& agents )
    {
        int counter = agents.size() - 1;
        typedef typename AGENTS_ST::const_reverse_iterator rit;
        for( rit i = agents.rbegin(); i != agents.rend(); ++i )
            mapper_[ *i ] = static_cast< P >( counter-- );
    }

    P operator() ( const T& t ) const
    {
        typename MAPPER_T::const_iterator i = mapper_.find( t );
        if ( mapper_.end() == i )
            throw std::runtime_error( "PIECE_MAPPING_ERROR" );
        return i->second;
    }

private:
    MAPPER_T mapper_;
};

enum COLOR { BLACK, WHITE };

enum PIECE_TYPE { PT_NONE, PT_PAWN, PT_KNIGHT, PT_BISHOP,
    PT_ROOK, PT_QUEEN, PT_KING, PT_UNDEFINED };

enum PIECE { NONE,
             BLACK_PAWN = 2, WHITE_PAWN,
             BLACK_KNIGHT, WHITE_KNIGHT,
             BLACK_BISHOP, WHITE_BISHOP,
             BLACK_ROOK, WHITE_ROOK,
             BLACK_QUEEN, WHITE_QUEEN,
             BLACK_KING, WHITE_KING,
             UNDEFINED };

class Piece : public Attribute
{
public:
    static const std::string& name()
    {
        static const std::string name( "piece" );
        return name;
    }

    Piece() {}
    explicit Piece( PIECE piece ) 
        : Attribute(true),value_( piece ), defined_(true) {}
    Piece( undef_t undef ) : Attribute(false),value_(NONE), defined_(false) {}
    Piece( PIECE_TYPE type, COLOR color )
        : Attribute(true),value_( type == PT_NONE || type == PT_UNDEFINED ?
          PIECE(2 * type) : PIECE( 2 * type + color ) ), defined_(true) {}

    PIECE_TYPE type() const { return static_cast<PIECE_TYPE>( value_ >> 1 ); }
    COLOR color() const { return static_cast<COLOR>( value_ & 1 ); }
    bool is_white() const { return WHITE == color(); }

    bool operator == ( const Piece& other ) const
        { return value_ == other.value_; }
    bool operator != ( const Piece& other ) const
        { return value_ != other.value_; }
    bool operator < ( const Piece& other ) const
        { return value_ < other.value_; }
    bool operator > ( const Piece& other ) const
        { return value_ > other.value_; }
    PIECE get() const { return value_; }

    static double value( int piece )
    {
        static double values[15] =
            { 0, 0, -1, 1, -2.75, 2.75, -3.25, 3.25, -5, 5, -9, 9, 0, 0, 0 };
        return values[piece];
    }

    static const std::tr1::array<std::string, 16>& AGENTS()
    {
        static const std::tr1::array<std::string, 16> values = { {
            "none", "none", "pawn", "Pawn", "knight", "Knight", "bishop",
            "Bishop", "rook", "Rook", "queen", "Queen", "king", "King",
            "undefined", "undefined" } };
        return values;
    }

    static const std::tr1::array<std::string, 8>& AGENT_TYPES()
    {
        static const std::tr1::array< std::string, 8 > values = { {
            "NONE", "PAWN", "KNIGHT", "BISHOP",
            "ROOK", "QUEEN", "KING", "UNDEFINED" } };
        return values;
    }

    static const std::tr1::array<char, 16>& AGENTS_S()
    {
        static const std::tr1::array<char, 16> values = { {
            ' ', ' ', 'p', 'P', 'n', 'N', 'b', 'B',
            'r', 'R', 'q', 'Q', 'k', 'K', 'u', 'u' } };
        return values;
    }

    static const std::tr1::array<char, 8>& AGENT_TYPES_S()
    {
        static const std::tr1::array<char, 8> values =
            { { ' ', 'P', 'N', 'B', 'R', 'Q', 'K', 'U' } };
        return values;
    }

    std::string agent() const { return AGENTS()[ value_ ]; }
    std::string agent_type() const { return AGENT_TYPES()[ type() ]; }

    static PIECE from_agent( const std::string& agent )
    {
        static piece_mapper<std::string, PIECE, 16> mapper( AGENTS() );
        return mapper( agent );
    }

    static PIECE_TYPE from_agent_type( const std::string& agent )
    {
        static piece_mapper<std::string, PIECE_TYPE, 8> mapper( AGENT_TYPES() );
        return mapper( agent );
    }

    char agent_s() const { return AGENTS_S()[ value_ ]; }
    char agent_type_s() const { return AGENT_TYPES_S()[ type() ]; }

    static PIECE from_agent_s( char agent )
    {
        static piece_mapper<char, PIECE, 16> mapper( AGENTS_S() );
        return mapper( agent );
    }

    static PIECE_TYPE from_agent_type_s( char agent )
    {
        static piece_mapper<char, PIECE_TYPE, 8> mapper( AGENT_TYPES_S() );
        return mapper( agent );
    }

    // pure virtual functions of class Attribute
    virtual bool Adjacent( const Attribute* other ) const
    {
        const Piece& p = *static_cast< const Piece* >( other );
        return abs( value_ - p.value_ ) == 1;
    }

    virtual int Compare( const Attribute* other ) const
    {
        const Piece& p = *static_cast< const Piece* >( other );
        return value_ - p.value_;
    }

    virtual std::ostream& Print( std::ostream& os ) const
    {
        return os << agent();
    }

    virtual void CopyFrom( const Attribute* other )
    {
        *this = Piece( *static_cast< const Piece* >( other ) );
    }

    virtual Attribute* Clone() const { return new Piece( *this ); }
    virtual bool IsDefined() const { return defined_; }
    virtual void SetDefined( bool def ) { defined_ = def; }
    virtual size_t Sizeof() const { return sizeof( *this ); }
    virtual size_t HashValue() const { return value_; }

    static Piece In( ListExpr i )
    {
        return Piece( Piece::from_agent( from_atom<std::string>(i) ) );
    }

    static ListExpr Out( const Piece& p )
    {
        return nl->StringAtom( p.agent() );
    }

    static bool KindCheck( ListExpr type, ListExpr& )
    {
        return nl->IsEqual( type, "piece" );
    }

    friend std::ostream& operator << ( std::ostream& os, const Piece& piece )
    {
        return os << piece.agent();
    }

    friend std::istream& operator >> ( std::istream& is, Piece& piece )
    {
        std::string agent;
        is >> agent;
        piece = Piece( Piece::from_agent( agent ) );
        return is;
    }

    friend std::ostream& operator << ( std::ostream& os, const PIECE_TYPE& pt )
    {
        return os << Piece::AGENT_TYPES()[ pt ];
    }

    friend std::istream& operator >> ( std::istream& is, PIECE_TYPE& pt )
    {
        std::string agent_type;
        is >> agent_type;
        pt = Piece::from_agent_type( agent_type );
        return is;
    }

    friend COLOR operator ! ( COLOR color )
    {
        return static_cast<COLOR>( 1 ^ color );
    }

protected:
    PIECE value_;
    bool defined_;
};

COLOR operator ! ( COLOR color );

std::ostream& operator << ( std::ostream& os, const Piece& piece );
std::istream& operator >> ( std::istream& is, Piece& piece );

std::ostream& operator << ( std::ostream& os, const PIECE_TYPE& pt );
std::istream& operator >> ( std::istream& is, PIECE_TYPE& pt );

#endif // SECONDO_ALGEBRAS_CHESS_TYPES_PIECE_HPP
