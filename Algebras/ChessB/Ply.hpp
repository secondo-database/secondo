#ifndef SECONDO_ALGEBRAS_CHESS_PLY_HPP
#define SECONDO_ALGEBRAS_CHESS_PLY_HPP

#include "StandardAttribute.h"
#include "ListStream.hpp"
#include "Type.hpp"
#include "PlyT.hpp"

class Ply;
list_ostream& operator << ( list_ostream&, const Ply& );
list_istream& operator >> ( list_istream&, Ply& );

class Ply : public PlyT, public StandardAttribute
{
public:
    static const std::string& name()
    {
        static const string name( "chessmove" );
        return name;
    }

    Ply() {}
    Ply( const PlyT& p ) : PlyT(p), defined_(true) {}
    Ply( undef_t undef ) : PlyT(0), defined_(false) {}

    // pure virtual functions of class StandardAttribute
    virtual bool Adjacent( const Attribute* ) const { return 0; }

    virtual int Compare( const Attribute* other ) const
    {
        const Ply& p = *static_cast< const Ply* >( other );
        return value_ - p.value_;
    }

    virtual ostream& Print( ostream& os ) const
    {
        if ( is_castling() )
            os << ( to().file == 6 ? "0-0" : "0-0-0" );
        else
        {
            if ( agent().type() != PT_PAWN )
                os << agent().agent_type_s();
            os << from() << ( is_capture() ? 'x' : '-' ) << to();
            if ( is_promotion() )
                os << "=" << promoted_to();
            if ( state() != PLY_NONE )
                os << ( is_check() ? '+' : ( is_mate() ? '#' : ' ' ) );
        }
        return os;
    }

    friend ostream& operator << ( ostream& os, const Ply& p )
    {
        return p.Print(os);
    }

    virtual void CopyFrom( const StandardAttribute* other )
    {
        *this = Ply( *static_cast< const Ply* >( other ) );
    }

    virtual Attribute* Clone() const { return new Ply( *this ); }
    virtual bool IsDefined() const { return defined_; }
    virtual void SetDefined( bool def ) { defined_ = def; }
    virtual size_t Sizeof() const { return sizeof( *this ); }
    virtual size_t HashValue() const { return value_; }

    static Ply In( ListExpr instance )
    {
        Ply p;
        int from_row, to_row, state, type, old_state;
        string agent, captured, from_file, to_file, pgn;
        bool check;

        list_istream is( instance );
        is >> agent >> captured >> from_file >> from_row
            >> to_file >> to_row >> check >> pgn;
        if ( is.size() == 11 )
            is >> state >> type >> old_state;
        else
        {
            state = 0;
            type = 0;
            old_state = 0;
        }

        return PlyT( Field( from_file[0] - 'a', from_row - 1 ),
                  Field( to_file[0] - 'a', to_row - 1),
                  Piece::from_agent(agent),
                  uint8_t(old_state), PLY_TYPE(type),
                  Piece(Piece::from_agent(captured)).type(),
                  PLY_STATE(state) );
    }

    static ListExpr Out( const Ply& p )
    {
        stringstream buf;
        p.Print(buf);
        list_ostream los;
        return los << p.agent().agent()
               << p.captured().agent()
               << string( 1, static_cast<char>( p.from().file + 'a' ) )
               << p.from().row + 1
               << string( 1, static_cast<char>( p.to().file + 'a' ) )
               << p.to().row + 1
               << p.is_check()
               << buf.str()
               << int( p.state() )
               << int( p.type() )
               << int( p.old_state() );
    }

    friend list_ostream& operator << ( list_ostream& os, const Ply& p )
    {
        stringstream buf;
        p.Print(buf);

        list_ostream ply_l;
        ply_l  << p.agent().agent()
               << p.captured().agent()
               << string( 1, static_cast<char>( p.from().file + 'a' ) )
               << p.from().row + 1
               << string( 1, static_cast<char>( p.to().file + 'a' ) )
               << p.to().row + 1
               << p.is_check()
               << buf.str()
               << int( p.state() )
               << int( p.type() )
               << int( p.old_state() );
        return os << ply_l;
    }

    friend list_istream& operator >> ( list_istream& is, Ply& p )
    {
        int from_row, to_row, state, type, old_state;
        string agent, captured, from_file, to_file, pgn;
        bool check;
        is >> agent >> captured >> from_file >> from_row
            >> to_file >> to_row >> check >> pgn;
        if ( is.size() == 11 )
            is >> state >> type >> old_state;
        else
        {
            state = 0;
            type = 0;
            old_state = 0;
        }

        p = PlyT( Field( from_file[0] - 'a', from_row - 1 ),
                  Field( to_file[0] - 'a', to_row - 1),
                  Piece::from_agent(agent),
                  uint8_t(old_state), PLY_TYPE(type),
                  Piece(Piece::from_agent(captured)).type(),
                  PLY_STATE(state) );
        return is;
    }

    static bool KindCheck( ListExpr type, ListExpr& )
    {
        return nl->IsEqual( type, "chessmove" );
    }

    // ---- operators -----------
    bool operator == ( const Ply& other ) const
        { return value_ == other.value_; }
    bool operator < ( const Ply& other ) const
        { return value_ < other.value_; }
    bool operator > ( const Ply& other ) const
        { return value_ > other.value_; }
protected:
    bool defined_;
};

#endif // SECONDO_ALGEBRAS_CHESS_PLY_HPP
