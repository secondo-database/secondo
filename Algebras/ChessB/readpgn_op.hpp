#ifndef SECONDO_ALGEBRAS_CHESS_READPGN_OP_HPP
#define SECONDO_ALGEBRAS_CHESS_READPGN_OP_HPP

#include <functional>
#include <string>
#include <utility>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Algebras/FText/FTextAlgebra.h"
#include "pgn_grammar.hpp"
#include "BasePosition.hpp"
#include "Game.hpp"
#include "MoveOps.hpp"

class validate_move
    : public std::binary_function< position_t, ply_data, std::pair<bool, PlyT> >
{
    Field to_;
    Field from_;
    Piece agent_;
    Piece promoted_;
    PLY_STATE state_;
    bool is_capture_;

    void validate_state( position_t& pos, const ply_data& p )
    {
        if ( p.file.empty() && p.castling.empty() )
            throw std::runtime_error( "Destination file not defined" );
        if ( p.castling.empty() )
            to_.file = p.file[0] - 'a';

        if ( p.rank.empty() && p.castling.empty() )
            throw std::runtime_error( "Destination row not defined" );
        if ( p.castling.empty() )
            to_.row = p.rank[0] - '1';

        if ( p.piece.empty() && p.castling.empty() )
            agent_ = Piece( PT_PAWN, pos.turn() );
        else if ( p.castling.empty() )
            agent_ = Piece( Piece::from_agent_type_s(p.piece[0]), pos.turn() );

        if ( p.promoted.empty() )
            promoted_ = Piece( NONE );
        else
            promoted_ = Piece( Piece::from_agent_type_s( p.promoted[0] ),
                pos.turn() );

        from_.file = p.afile.empty() ? -1 : p.afile[0] - 'a';
        from_.row = p.arank.empty() ? -1 : p.arank[0] - '1';

        is_capture_ = p.capture == "x";
        if ( p.check == "+" || p.check == "++" )
            state_ = PLY_CHECK;
        else if ( p.check == "#" )
            state_ = PLY_MATE;
        else
            state_ = PLY_NONE;
    }

    PlyT validate_castling( position_t& pos, const ply_data& p )
    {
        bool l_castling = p.castling == "O-O-O";
        int to_file = l_castling ? 2 : 6;
        int row = pos.turn() == WHITE ? 0 : 7;
        PIECE piece = pos.turn() == WHITE ? WHITE_KING : BLACK_KING;

        if ( ! pos.is_castling_possible( l_castling, pos.turn() ) )
            throw std::runtime_error( "Castling isn't possible" );
        return PlyT( Field(4, row), Field(to_file, row), piece, pos.state(),
            PLY_CASTLING, PT_NONE, state_ );
    }

    PlyT validate_pawn_move( position_t& pos, const ply_data& )
    {
        if ( is_capture_ )
        {
            if ( from_.file == -1 )
                throw std::runtime_error( "File not defined in pawn capture" );
            from_.row = to_.row + ( pos.turn() == WHITE ? -1 : 1 );

            if ( pos[ from_ ] != agent_.get() )
                throw std::runtime_error( "Wrong move - piece isn't pawn" );

            if ( pos[to_] == NONE )
            {
                PIECE enp_piece = pos[ Field( to_.file, from_.row ) ];
                if ( enp_piece != Piece( PT_PAWN, !pos.turn() ).get()
                    || ! pos.is_enpassant_possible()
                    || pos.enpassant_file() != to_.file )
                    throw std::runtime_error( "Enpassant capture not possible" );
                return PlyT( from_, to_, agent_.get(), pos.state(),
                    PLY_ENPASSANT, PT_PAWN, state_ );
            }

            if ( Piece( pos[to_] ).color() == pos.turn() )
                throw std::runtime_error( "Captured piece is of same color" );
            if ( Piece( pos[to_] ).type() == PT_KING )
                throw std::runtime_error( "Captured piece is a king" );

            if ( promoted_.get() != NONE )
            {
                if ( to_.row != ( pos.turn() == WHITE ? 7 : 0 ) )
                    throw std::runtime_error( "Wrong promotion dest. field" );
                return PlyT( from_, to_, promoted_.get(), pos.state(),
                    PLY_PROMOTION, Piece( pos[to_] ).type(), state_ );
            }
            return PlyT( from_, to_, agent_.get(),pos.state(),
                PLY_ORDINARY, Piece( pos[to_] ).type(), state_ );
        } // capture

        if ( pos[to_] != NONE )
            throw std::runtime_error( "Dest. field isn't empty" );
        from_.row = to_.row + ( pos.turn() == WHITE ? -1 : 1 );
        from_.file = to_.file;
        if ( pos[from_] == agent_.get() )
        {
            if ( promoted_.get() != NONE )
            {
                if ( to_.row != ( pos.turn() == WHITE ? 7 : 0 ) )
                    throw std::runtime_error( "Wrong promotion dest. field" );
                return PlyT( from_, to_, promoted_.get(), pos.state(),
                    PLY_PROMOTION, PT_NONE, state_ );
            }
            return PlyT( from_, to_, agent_.get(), pos.state(),
                PLY_ORDINARY, PT_NONE, state_ );
        }
        if ( pos[from_] == NONE )
        {
            from_.row += pos.turn() == WHITE ? -1 : 1;
            if ( from_.row == ( pos.turn() == WHITE ? 1 : 6 )
                && pos[from_] == agent_.get() )
                return PlyT( from_, to_, agent_.get(), pos.state(),
                    PLY_ORDINARY, PT_NONE, state_ );
        }
        throw std::runtime_error( "Invalid pawn move" );
    }

    PlyT validate_piece_move( const position_t& pos, const ply_data& )
    {
        position_t::board_t::moves_t dirs = pos.moves( agent_.type() );

        Piece dest = Piece( pos[ to_ ] );
        if ( is_capture_ && ( dest.get() == NONE
            || dest.color() == pos.turn() || dest.type() == PT_KING ) )
            throw std::runtime_error( "Capture isn't possible" );

        position_t::const_iterator it1 = pos.iter( to_ );
        for( int d = 0; d < dirs.count; ++d )
        {
            position_t::const_iterator it = it1;
            for( int i = 0; i < dirs.steps; ++i )
            {
                it += dirs.moves[d];
                if ( *it == NONE )
                    continue;
                if ( *it != agent_.get() )
                    break;
                Field from = pos.field( it );
                if ( ( ( from_.file == -1 && from_.row == -1 )
                    || ( from_.file != -1 && from.file == from_.file )
                    || ( from_.row != -1 && from.row == from_.row ) ) )
                    return PlyT( from, to_, agent_.get(), pos.state(),
                        PLY_ORDINARY, dest.type(), state_ );
                else
                    break;
            }
        }
        throw std::runtime_error( "Invalid move" );
    }
public:
    PlyT operator()( position_t& pos, const ply_data& p )
    {
        validate_state( pos, p );
        if ( p.castling == "O-O-O" || p.castling == "O-O" )
            return validate_castling( pos, p );
        if ( agent_.type() == PT_PAWN )
            return validate_pawn_move( pos, p );
        return validate_piece_move( pos, p );
    }
};

class readpgn_op : public std::unary_function< FText, std::pair<bool, Game*> >
{
public:
    readpgn_op( const FText& filename, ListExpr type )
        : number_(0), content_(""), next_(content_.end()), end_(content_.end())
    {
        std::ifstream pgn( filename.GetValue().c_str(), std::ios::binary | std::ios::in );
        if ( !pgn )
            throw std::runtime_error( "PGN-File not found!" );

        std::ostringstream s;
        s << pgn.rdbuf();
        content_ = s.str();
        next_ = content_.begin();
        end_  = content_.end();
    }

    std::pair<bool, Game*> operator()( const FText& )
    {
        if ( content_.empty() )
            return std::make_pair( false, new Game(UNDEF) );

        while( next_ < end_ )
        {
            game_data data;
            bs::parse_info< std::string::const_iterator > info
                = bs::parse( next_, end_, game_parser(data), bs::space_p );
            next_ = info.stop;

            if ( data.result.empty() && next_ < end_ )
            {
                std::string sample = std::string( info.stop - 30, info.stop + 30 );
                for( std::string::size_type p = sample.find_first_of("\n\r");
                     p != std::string::npos; p = sample.find_first_of("\n\r", p) )
                     sample[p] = ' ';
                std::cerr << "PARSING ERROR" << "\n"
                     << sample << "\n" << std::string( 30, ' ' ) << "^\n\n";
                std::string::size_type continuation =
                    content_.find_first_of("[", info.stop - content_.begin() );
                if ( std::string::npos == continuation )
                    return std::make_pair( false, new Game(UNDEF) );
                next_ = content_.begin() + continuation;
            }
            else
            {
                ++number_;
                Game* g = new Game( data.tags.size(), data.moves.size() );
                for( game_data::tags_t::const_iterator it = data.tags.begin();
                     it != data.tags.end(); ++it )
                {
                    g->add_tag( it->first, it->second );
                }
                if ( validate_moves( *g, data.moves ) )
                    return std::make_pair( true, g );
                std::cerr << "Skip game Nr." << number_
                     << " due to invalid move.\n";
                delete g;
            }
        }
        return std::make_pair( false, new Game(UNDEF) );
    }

    bool validate_moves( Game& g, const game_data::moves_t& moves )
    {
        Position pos( INITIAL_POSITION );
        for( size_t ply_number = 0; ply_number < moves.size(); ++ply_number )
        {
            try
            {
                PlyT result = validate_move()( pos, moves[ply_number] );
                delete apply_ply_op()( pos, result );
                g.moves.Append( result );
            }
            catch ( const std::exception& e )
            {
                std::cerr << e.what() << " '" << moves[ply_number].ply << "' Nr."
                     << ply_number/2 + 1 << " in game Nr." << number_ << endl;
                return false;
            }
        }
        return true;
    }

private:
    int number_;
    std::string content_;
    std::string::const_iterator next_, end_;
};

#endif // SECONDO_ALGEBRAS_CHESS_READPGN_OP_HPP
