#ifndef SECONDO_ALGEBRAS_CHESS_POSITIONOPS_HPP
#define SECONDO_ALGEBRAS_CHESS_POSITIONOPS_HPP

#include <map>
#include <functional>
#include <algorithm>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include "ternary_function.hpp"
#include "Position.hpp"
#include "Material.hpp"
#include "Piece.hpp"
#include "MoveOps.hpp"

using boost::lambda::_1;

struct attackcount_op : binary_function< Position, Field, int >
{
    int operator () ( const Position& pos, const Field& f ) const
    {
        int count = 0;
        // Count pawns attacks
        int row = f.row + ( pos.turn() == WHITE ? -1 : 1 );
        if (pos[ Field(f.file - 1, row) ] == Piece(PT_PAWN, pos.turn()).get())
            ++count;
        if (pos[ Field(f.file + 1, row) ] == Piece(PT_PAWN, pos.turn()).get())
            ++count;

        // Count pieces attacks
        for( int pt = PT_KNIGHT; pt <= PT_KING; ++pt )
        {
            position_t::board_t::moves_t dirs = pos.moves( PIECE_TYPE(pt) );
            position_t::const_iterator it1 = pos.iter( f );
            for( int d = 0; d < dirs.count; ++d )
            {
                position_t::const_iterator it = it1;
                for( int i = 0; i < dirs.steps; ++i )
                {
                    it += dirs.moves[d];
                    if ( *it == NONE )
                        continue;
                    if ( *it == Piece( PIECE_TYPE(pt), pos.turn() ).get() )
                        ++count;
                    break;
                }
            }
        }
        return count;
    }
};

struct protectcount_op : binary_function< Position, Field, int >
{
    int operator () ( const Position& pos, const Field& f ) const
    {
        int count = 0;
        // Count pawns
        PIECE protecting = Piece( PT_PAWN, !pos.turn() ).get();
        int row = f.row + ( pos.turn() == WHITE ? 1 : -1 );
        if ( pos[ Field(f.file - 1, row) ] == protecting )
            ++count;
        if ( pos[ Field(f.file + 1, row) ] == protecting )
            ++count;

        // Count pieces
        for( int pt = PT_KNIGHT; pt <= PT_KING; ++pt )
        {
            position_t::board_t::moves_t dirs = pos.moves( PIECE_TYPE(pt) );
            position_t::const_iterator it1 = pos.iter( f );
            for( int d = 0; d < dirs.count; ++d )
            {
                position_t::const_iterator it = it1;
                for( int i = 0; i < dirs.steps; ++i )
                {
                    it += dirs.moves[d];
                    if ( *it == NONE )
                        continue;
                    if ( *it == Piece( PIECE_TYPE(pt), !pos.turn() ).get() )
                        ++count;
                    break;
                }
            }
        }
        return count;
    }
};

struct move_generator
{
    typedef vector< PlyT > moves_t;
    typedef moves_t::const_iterator iterator;

    move_generator( const Position& pos ) : pos_(pos)
    {
        generate_castlings( pos_ );

        //typedef Position::const_iterator pos_iter;
        for( int row = 0; row < 8; ++row )
        {
            for( int file = 0; file < 8; ++file )
            {
                Field from( file, row );
                Piece agent( pos_[from] );
                if ( agent.get() < BLACK_PAWN || agent.get() > WHITE_KING || agent.color() != pos_.turn() )
                    continue;
                if ( agent.type() == PT_PAWN )
                    generate_pawn_moves( file, row, agent );
                else if ( PT_KNIGHT <= agent.type() && agent.type() <= PT_KING )
                    generate_piece_moves( pos_, from, agent );
            }
        }
    }

    Field find_king( COLOR color ) const
    {
        PIECE king = Piece( PT_KING, color ).get();
        for( int i = 0; i < 64; ++i )
            if ( pos_[ Field(i%8, i/8) ] == king )
                return Field(i%8, i/8);
        throw runtime_error( "King not found" );
    }

    iterator begin() const { return moves_.begin(); }
    iterator end() const { return moves_.begin(); }
    const PlyT& operator[]( int n ) const { return moves_[n]; }
    int size() const { return moves_.size(); }

private:
    moves_t moves_;
    Position pos_;


    void add_move( const PlyT& p )
    {
        Ply ply(p);
        delete apply_ply_op()( pos_, ply );
        Field king_pos = find_king( !pos_.turn() );
        if ( attackcount_op()( pos_, king_pos ) == 0 )
            moves_.push_back( p );
        delete revert_ply_op()( pos_, ply );
    }

    void generate_piece_moves( const Position& pos_,
        const Field& from, const Piece& agent )
    {
        position_t::board_t::moves_t dirs = pos_.moves( agent.type() );
        for( int d = 0; d < dirs.count; ++d )
        {
            position_t::const_iterator it = pos_.iter( from );
            for( int i = 0; i < dirs.steps; ++i )
            {
                it += dirs.moves[d];
                if ( *it == UNDEFINED )
                    break;
                if ( *it != NONE && Piece(*it).color() == pos_.turn() )
                    break;
                Field to( pos_.field( it ) );
                Piece captured( pos_[to] );
                add_move( PlyT( from, to, agent.get(), pos_.state(), PLY_ORDINARY, captured.type() ) );
                if ( *it != NONE )
                    break;
            }
        }
    }

    void generate_castlings( const Position& pos_ )
    {
        int row = pos_.turn() == WHITE ? 0 : 7;
        if ( pos_.is_castling_possible( true, pos_.turn() ) &&
            pos_[Field(3, row)] == NONE && pos_[Field(2, row)] == NONE &&
            pos_[Field(1, row)] == NONE )
        {
            PIECE agent = pos_[Field(4, row)];
            add_move( PlyT(Field(4, row), Field(2, row), agent, pos_.state(), PLY_CASTLING ) );
        }
        if ( pos_.is_castling_possible( false, pos_.turn() ) &&
            pos_[Field(5, row)] == NONE && pos_[Field(6, row)] == NONE )
        {
            Field from( 4, row );
            Field to( 6, row );
            PIECE agent = pos_[from];
            add_move( PlyT( from, to, agent, pos_.state(), PLY_CASTLING ) );
        }
    }

    void generate_pawn_moves( int file, int row, const Piece& agent )
    {
        Field from( file, row );
        int dest_row = row + ( pos_.turn() == WHITE ? 1 : -1 );
        Field to( file, dest_row );
        if ( pos_[ to ] == NONE )
        {
            add_move( PlyT( from, to, agent.get(), pos_.state() ) );

            if ( row == ( pos_.turn() == WHITE ? 1 : 6 ) )
            {
                dest_row += pos_.turn() == WHITE ? 1 : -1;
                to = Field( file, dest_row );
                if ( pos_[ to ] == NONE )
                    add_move( PlyT( from, to, agent.get(), pos_.state() ) );
            }
        }
        dest_row = row + ( pos_.turn() == WHITE ? 1 : -1 );
        generate_pawn_capture( from, Field( file - 1, dest_row ), agent );
        generate_pawn_capture( from, Field( file + 1, dest_row ), agent );
    }

    void generate_pawn_capture( const Field& from, const Field& to, const Piece& agent )
    {
        Piece captured( pos_[to] );
        if ( captured.get() == UNDEFINED )
            return;
        if ( captured.get() == NONE )
        {
            int to_row = pos_.turn() == WHITE ? 5 : 2;
            Field capture_f = Field( to.file, pos_.turn() == WHITE ? 4 : 3 );
            if ( to.row == to_row && pos_.is_enpassant_possible()
                 && Piece( pos_[capture_f] ) == Piece( PT_PAWN, !pos_.turn() )
                 && pos_.enpassant_file() == to.file )
            {
                add_move( PlyT( from, to, agent.get(), pos_.state(), PLY_ENPASSANT, PT_PAWN ) );
            }
        }
        else if ( captured.color() != pos_.turn() )
            add_move( PlyT( from, to, agent.get(), pos_.state(), PLY_ORDINARY, captured.type() ) );
    }
};

struct pieces_op : unary_function< Position, Material* >
{
    Material* operator () ( const Position& pos ) const
    {
        Material* m = new Material( DEFINED );
        for( int i = 0; i < 64; ++i )
        {
            Field field( i % 8, i / 8 );
            PIECE piece = pos[field];
            if ( piece >= BLACK_PAWN && piece <= WHITE_KING )
                ++m->pieces[ piece - 2 ];
        }
        return m;
    }
};

struct moveNo_op : unary_function< Position, int >
{
    int operator () ( const Position& p ) const { return p.move_number(); }
};

struct checkmate_op : unary_function< Position, bool >
{
    bool operator () ( const Position& pos ) const
    {
        move_generator gen( pos );
        Field king_pos = gen.find_king( pos.turn() );
        return protectcount_op()( pos, king_pos ) > 0 && gen.size() == 0;
    }
};

struct stalemate_op : unary_function< Position, bool >
{
    bool operator () ( const Position& pos ) const
    {
        move_generator gen( pos );
        Field king_pos = gen.find_king( pos.turn() );
        return protectcount_op()( pos, king_pos ) == 0 && gen.size() == 0;
    }
};

struct includes_op : binary_function< Position, Position, bool >
{
    bool operator () ( const Position& p1, const Position& p2 ) const
    {
        typedef Position::const_iterator Iter;
        for( Iter i1 = p1.begin(), i2 = p2.begin();
             i1 != p1.end() && i2 != p2.end(); ++i1, ++i2 )
        {
            if ( *i1 != *i2 && *i2 != NONE )
                return false;
        }
        return true;
    }
};

struct piececount_position_op : binary_function< Position, Piece, int >
{
    int operator() ( const Position& pos, const Piece& piece ) const
    {
        PIECE p = piece.get();
        return count_if( pos.begin(), pos.end(), p == boost::lambda::_1 );
    }
};

struct piececount_spos_op : binary_function< Position, CcString, int >
{
    int operator() ( const Position& pos, const CcString& piece ) const
    {
        PIECE_TYPE pt = PT_UNDEFINED;
        try {
            pt = Piece::from_agent_type( piece.GetValue() );
        }
        catch( const exception& )
        {
            PIECE p = UNDEFINED;
            try {
                p = Piece::from_agent( piece.GetValue() );
            }
            catch( const exception& ){
                throw runtime_error( "Unknown Piece 1" );
            }
            if ( p < NONE || p > WHITE_KING )
                throw runtime_error( "Unknown Piece 2" );
            return count_if( pos.begin(), pos.end(), p == boost::lambda::_1 );
        }
        if ( pt < PT_NONE || pt > PT_KING )
            throw runtime_error( "Unknown Piece 3" );
        int sum = 0;
        for( int i = 0; i < 64; ++i )
            if ( Piece( pos[Field(i%8, i/8)] ).type() == pt )
                ++sum;
        return sum;
    }
};

struct approx_position_op : binary_function< Position, Position, bool >
{
    bool operator() ( const Position& p1, const Position& p2 ) const
    {
        return abs( p1.value_sum() - p2.value_sum() ) < 0.2;
    }
};

struct posrange_op : ternary_function< Position, Field, Field, Position* >
{
    Position* operator()( const Position& p, const Field& f1, const Field& f2 )
    {
        Position* pos = new Position( EMPTY_POSITION );
        for( int y = f1.row; y <= f2.row; ++y )
            for( int x = f1.file; x <= f2.file; ++x )
                (*pos)[ Field( x, y ) ] = p[ Field( x, y ) ];
        return pos;
    }
};

struct attacked_by_p_op : ternary_function< Position, Piece, Piece, bool >
{
    bool operator()( const Position& pos, const Piece& p1, const Piece& p2 )
    {
        Piece attacking = Piece( p2.type(), pos.turn() );
        PIECE attacked = Piece( p1.type(), !pos.turn() ).get();

        for( int i = 0; i < 64; ++i )
        {
            Field f( i % 8, i / 8 );
            if ( Piece( pos[f] ) == attacking )
            {
                if ( p2.type() == PT_PAWN )
                {
                    int row = f.row + ( pos.turn() == WHITE ? 1 : -1 );
                    if ( pos[ Field(f.file - 1, row) ] == attacked )
                        return true;
                    if ( pos[ Field(f.file + 1, row) ] == attacked )
                        return true;
                }
                else
                {

                 /*
                  Warning, sometimes the following code produces a memory
                  error, i.e. the it will point after the valid data.

                 */

                    position_t::board_t::moves_t dirs = pos.moves( p2.type() );
                    position_t::const_iterator it1 = pos.iter( f );
                    for( int d = 0; d < dirs.count; ++d )
                    {
                        position_t::const_iterator it = it1;
                        for( int i = 0; i < dirs.steps; ++i )
                        {
                            it += dirs.moves[d];
                            if ( *it == NONE )
                                continue;
                            if ( *it == attacked )
                                return true;
                        }
                    }
                }
            }
        }
        return false;
    }
};

struct attacked_by_f_op : ternary_function< Position, Field, Piece, bool >
{
    bool operator()( const Position& pos, const Field& f, const Piece& p )
    {
        PIECE attacking = Piece( p.type(), pos.turn() ).get();
        if ( p.type() == PT_PAWN )
        {
            int row = f.row + ( pos.turn() == WHITE ? -1 : 1 );
            if ( pos[ Field(f.file - 1, row) ] == attacking )
                return true;
            if ( pos[ Field(f.file + 1, row) ] == attacking )
                return true;
            return false;
        }

        position_t::board_t::moves_t dirs = pos.moves( p.type() );
        position_t::const_iterator it1 = pos.iter( f );
        for( int d = 0; d < dirs.count; ++d )
        {
            position_t::const_iterator it = it1;
            for( int i = 0; i < dirs.steps; ++i )
            {
                it += dirs.moves[d];
                if ( *it == NONE )
                    continue;
                if ( *it == attacking )
                    return true;
                break;
            }
        }
        return false;
    }
};

struct attacked_from_p_op : ternary_function< Position, Piece, Field, bool >
{
    bool operator()( const Position& pos, const Piece& p, const Field& f )
    {
        PIECE_TYPE attacking = Piece( pos[f] ).type();
        if ( attacking < PT_PAWN || attacking > PT_KING )
            return false;

        PIECE attacked = Piece( p.type(), !pos.turn() ).get();
        // pawns attacks
        if ( attacking == PT_PAWN )
        {
            int row = f.row + ( pos.turn() == WHITE ? -1 : 1 );
            if ( pos[ Field(f.file - 1, row) ] == attacked )
                return true;
            if ( pos[ Field(f.file + 1, row) ] == attacked )
                return true;
            return false;
        }

        // pieces attacks
        position_t::board_t::moves_t dirs = pos.moves( attacking );
        position_t::const_iterator it1 = pos.iter( f );
        for( int d = 0; d < dirs.count; ++d )
        {
            position_t::const_iterator it = it1;
            for( int i = 0; i < dirs.steps; ++i )
            {
                it += dirs.moves[d];
                if ( *it == NONE )
                    continue;
                if ( *it == attacked )
                    return true;
                break;
            }
        }
        return false;
    }
};

struct attacked_from_f_op : ternary_function< Position, Field, Field, bool >
{
    bool operator()( const Position& pos, const Field& f1, const Field& f2 )
    {
        PIECE_TYPE attacking = Piece( pos[f2] ).type();
        if ( attacking < PT_PAWN || attacking > PT_KING )
            return false;

        // pawns attacks
        if ( attacking == PT_PAWN )
        {
            int row = f2.row + ( pos.turn() == WHITE ? -1 : 1 );
            if ( Field(f2.file - 1, row) == f1 )
                return true;
            if ( Field(f2.file + 1, row) == f1 )
                return true;
            return false;
        }

        // pieces attacks
        position_t::board_t::moves_t dirs = pos.moves( attacking );
        position_t::const_iterator it1 = pos.iter( f2 );
        for( int d = 0; d < dirs.count; ++d )
        {
            position_t::const_iterator it = it1;
            for( int i = 0; i < dirs.steps; ++i )
            {
                it += dirs.moves[d];
                if ( pos.field( it ) == f1 )
                    return true;
                if ( *it == NONE )
                    continue;
                break;
            }
        }
        return false;
    }
};

struct protected_by_p_op : ternary_function< Position, Piece, Piece, bool >
{
    bool operator()( Position& pos, const Piece& p1, const Piece& p2 )
    {
        pos.increment_turn();
        bool result = attacked_by_p_op()( pos, p1, p2 );
        pos.decrement_turn();
        return result;
    }
};

struct protected_by_f_op : ternary_function< Position, Field, Piece, bool >
{
    bool operator()( Position& pos, const Field& f, const Piece& p )
    {
        pos.increment_turn();
        bool result = attacked_by_f_op()( pos, f, p );
        pos.decrement_turn();
        return result;
    }
};

struct protected_from_p_op : ternary_function< Position, Piece, Field, bool >
{
    bool operator()( Position& pos, const Piece& p, const Field& f )
    {
        pos.increment_turn();
        bool result = attacked_from_p_op()( pos, p, f );
        pos.decrement_turn();
        return result;
    }
};

struct protected_from_f_op : ternary_function< Position, Field, Field, bool >
{
    bool operator()( Position& pos, const Field& f1, const Field& f2 )
    {
        pos.increment_turn();
        bool result = attacked_from_f_op()( pos, f1, f2 );
        pos.decrement_turn();
        return result;
    }
};

struct apply_move_op : ternary_function< Position, Field, Field, Position* >
{
    Position* operator()( Position& pos, const Field& f1, const Field& f2 )
    {
        // TODO Plausibilitaetspruefung, argument checking
        if ( pos[f1] != NONE )
        {
            pos[f2] = pos[f1];
            pos[f1] = NONE;
        }
        return new Position(pos); // TODO
    }
};

struct pos_fields_op : unary_function< Position, pair<bool, Tuple*> >
{
    pos_fields_op( const Position&, ListExpr type ) : index_(-1), type_(type){}

    pair<bool, Tuple*> operator()( const Position& pos )
    {
        TupleType type( type_ );
        if ( ++index_ < 64 )
        {
            Tuple* result = new Tuple( new TupleType(type_) );
            Field* field = new Field( index_ % 8, index_ / 8 );
            result->PutAttribute( 0, field );
            result->PutAttribute( 1, new Piece( pos[*field] ) );
            return make_pair( true, result );
        }
        return make_pair( false, new Tuple( new TupleType(type_) ) );
    }

    static list_ostream type( ListExpr )
    {
        return list_ostream()
               << ( list_ostream() << ChessBSymbol("Field")
                                   << ChessBSymbol("field") )
               << ( list_ostream() << ChessBSymbol("Piece")
                                   << ChessBSymbol("piece") );
    }
private:
    int index_;
    ListExpr type_;
};

struct tuple_t
{
    tuple_t() : bp(NONE){}
    tuple_t( Piece sp_, Field sf_, Piece bp_, Field bf_, Piece ep_, Field ef_ )
        : sp(sp_), bp(bp_), ep(ep_), sf(sf_), bf(bf_), ef(ef_){}
    Piece sp, bp, ep;
    Field sf, bf, ef;
};

struct pos_moves_op : unary_function< Position, pair<bool, Tuple*> >
{
    pos_moves_op( const Position& pos, ListExpr type )
        : type_(type), gen_(pos), current_(-1){}

    pair<bool, Tuple*> operator()( const Position& )
    {
        TupleType type( type_ );
        if ( ++current_ < gen_.size() )
        {
            const PlyT& ply = gen_[current_];
            Tuple* result = new Tuple( new TupleType(type_) );
            result->PutAttribute( 0, new Piece( ply.agent() ) );
            result->PutAttribute( 1, new Field( ply.from() ) );
            result->PutAttribute( 2, new Piece( ply.captured() ) );
            result->PutAttribute( 3, new Field( ply.to() ) );
            return make_pair( true, result );
        }
        return make_pair( false, new Tuple( new TupleType(type_) ) );
    }

    static list_ostream type( ListExpr )
    {
        return list_ostream()
           << ( list_ostream() << ChessBSymbol("SPiece") 
                               << ChessBSymbol("piece") )
           << ( list_ostream() << ChessBSymbol("SField")
                               << ChessBSymbol("field") )
           << ( list_ostream() << ChessBSymbol("EPiece")
                               << ChessBSymbol("piece") )
           << ( list_ostream() << ChessBSymbol("EField")
                               << ChessBSymbol("field") );
    }
private:
    ListExpr type_;
    move_generator gen_;
    int current_;
};

class p_moves_blocked_op;

struct pos_moves_blocked_op : unary_function< Position, pair<bool, Tuple*> >
{
    pos_moves_blocked_op( const Position& pos, ListExpr t )
        : type_(t), current_(0)
    {
        int srow = pos.turn() == WHITE ? 1 : 6;
        int brow = pos.turn() == WHITE ? 2 : 5;
        int erow = pos.turn() == WHITE ? 3 : 4;
        Piece pawn = Piece( PT_PAWN, pos.turn() );
        for( int file = 0; file < 8; ++file )
        {
            Piece spiece = Piece( pos[ Field( file, srow ) ] );
            Piece bpiece = Piece( pos[ Field( file, brow ) ] );
            Piece epiece = Piece( pos[ Field( file, erow ) ] );
            if ( spiece == pawn && bpiece.get() != NONE
                 && epiece.color() != pos.turn() )
            {
                tuples.push_back( tuple_t( spiece, Field( file, srow ), bpiece,
                    Field( file, brow ), epiece, Field( file, erow ) ) );
            }
        }

        for( int i = 0; i < 64; ++i )
        {
            tuple_t t;
            t.sf = Field( i % 8, i / 8 );
            t.sp = Piece( pos[t.sf] );
            if ( t.sp.type() > PT_KNIGHT && t.sp.type() < PT_KING
                 && t.sp.color() == pos.turn() )
            {
                position_t::board_t::moves_t dirs = pos.moves( t.sp.type() );
                position_t::const_iterator it1 = pos.iter( t.sf );
                for( int d = 0; d < dirs.count; ++d )
                {
                    position_t::const_iterator it = it1;
                    for( int i = 0; i < dirs.steps; ++i )
                    {
                        it += dirs.moves[d];
                        if ( *it == UNDEFINED )
                            break;
                        if ( t.bp == Piece( NONE ) )
                        {
                            if ( *it != NONE )
                            {
                                t.bp = Piece(*it);
                                t.bf = pos.field(it);
                            }
                            continue;
                        }
                        if ( *it == NONE )
                        {
                            t.ep = Piece(*it);
                            t.ef = pos.field(it);
                            tuples.push_back( t );
                            continue;
                        }
                        else if ( Piece(*it).color() != pos.turn()
                             && Piece(*it).type() != PT_KING )
                        {
                            t.ep = Piece(*it);
                            t.ef = pos.field(it);
                            tuples.push_back( t );
                        }
                        t.bp = Piece(*it);
                        t.bf = pos.field(it);
                    }
                }
            }
        }
    }

    pair<bool, Tuple*> operator()( const Position& )
    {
        TupleType type( type_ );
        if ( current_ < tuples.size() )
        {
            const tuple_t& t = tuples[ current_++ ];
            Tuple* result = new Tuple( new TupleType(type_) );
            result->PutAttribute( 0, new Piece(t.sp) );
            result->PutAttribute( 1, new Field(t.sf) );
            result->PutAttribute( 2, new Piece(t.bp) );
            result->PutAttribute( 3, new Field(t.bf) );
            result->PutAttribute( 4, new Piece(t.ep) );
            result->PutAttribute( 5, new Field(t.ef) );
            return make_pair( true, result );
        }
        return make_pair( false, new Tuple( new TupleType(type_) ) );
    }

    static list_ostream type( ListExpr )
    {
        return list_ostream()
           << ( list_ostream() << ChessBSymbol("SPiece") 
                               << ChessBSymbol("piece") )
           << ( list_ostream() << ChessBSymbol("SField")
                               << ChessBSymbol("field") )
           << ( list_ostream() << ChessBSymbol("BPiece")
                               << ChessBSymbol("piece") )
           << ( list_ostream() << ChessBSymbol("BField")
                               << ChessBSymbol("field") )
           << ( list_ostream() << ChessBSymbol("EPiece")
                               << ChessBSymbol("piece") )
           << ( list_ostream() << ChessBSymbol("EField")
                               << ChessBSymbol("field") );
    }

private:
    ListExpr type_;
    vector< tuple_t > tuples;
    vector< tuple_t >::size_type current_;
    friend class p_moves_blocked_op;
};

struct piece_moves_op : binary_function< Position, Piece, pair<bool, Tuple*> >
{
    piece_moves_op( const Position& pos, const Piece& agent, ListExpr type )
        : type_(type), gen_(pos), current_(-1){}

    pair<bool, Tuple*> operator()( const Position&, const Piece& agent )
    {
        TupleType type( type_ );
        while( ++current_ < gen_.size() )
        {
            const PlyT& ply = gen_[current_];
            if ( ply.agent() != agent )
                continue;

            Tuple* result = new Tuple( new TupleType(type_) );
            result->PutAttribute( 0, new Piece( ply.agent() ) );
            result->PutAttribute( 1, new Field( ply.from() ) );
            result->PutAttribute( 2, new Piece( ply.captured() ) );
            result->PutAttribute( 3, new Field( ply.to() ) );
            return make_pair( true, result );
        }
        return make_pair( false, new Tuple( new TupleType(type_) ) );
    }

    static list_ostream type( ListExpr )
    {
        return list_ostream()
               << ( list_ostream() << ChessBSymbol("SPiece")
                                   << ChessBSymbol("piece") )
               << ( list_ostream() << ChessBSymbol("SField")
                                   << ChessBSymbol("field") )
               << ( list_ostream() << ChessBSymbol("EPiece")
                                   << ChessBSymbol("piece") )
               << ( list_ostream() << ChessBSymbol("EField")
                                   << ChessBSymbol("field") );
    }
private:
    ListExpr type_;
    move_generator gen_;
    int current_;
};

struct p_moves_blocked_op : binary_function< Position, Piece, pair<bool, Tuple*> >
{
    p_moves_blocked_op( const Position& pos, const Piece& p, ListExpr t )
        : type_(t), op_(pos, t){}

    pair<bool, Tuple*> operator()( const Position& pos, const Piece& piece )
    {
        // TODO piece is undefined for some reason in
        // query wjc feed head[1] extract[elem] getposition[0]
        // piece_moves_blocked[Queen] consume;

        TupleType type( type_ );
        while( op_.current_ < op_.tuples.size() )
        {
            const tuple_t& t = op_.tuples[ op_.current_++ ];
            if ( t.sp != piece )
                continue;

            Tuple* result = new Tuple( new TupleType(type_) );
            result->PutAttribute( 0, new Piece(t.sp) );
            result->PutAttribute( 1, new Field(t.sf) );
            result->PutAttribute( 2, new Piece(t.bp) );
            result->PutAttribute( 3, new Field(t.bf) );
            result->PutAttribute( 4, new Piece(t.ep) );
            result->PutAttribute( 5, new Field(t.ef) );
            return make_pair( true, result );
        }
        return make_pair( false, new Tuple( new TupleType(type_) ) );
    }

    static list_ostream type( ListExpr e )
    {
        return pos_moves_blocked_op::type( e );
    }
private:
    ListExpr type_;
    pos_moves_blocked_op op_;
};

#endif // SECONDO_ALGEBRAS_CHESS_POSITIONOPS_HPP
