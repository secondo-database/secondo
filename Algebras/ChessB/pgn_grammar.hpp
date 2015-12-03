#ifndef SECONDO_ALGEBRAS_CHESS_PGN_GRAMMAR_HPP
#define SECONDO_ALGEBRAS_CHESS_PGN_GRAMMAR_HPP

#include <map>
#include <string>
#include <vector>

//#define BOOST_SPIRIT_DEBUG
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_utility.hpp>
#include <boost/spirit/include/classic_actor.hpp>
namespace bs = boost::spirit::classic;

struct ply_data
{
    string check, castling, capture, file,
        afile, rank, arank, piece, promoted, ply;
};

struct game_data
{
    typedef std::map< std::string, std::string > tags_t;
    typedef std::vector< ply_data > moves_t;

    // data
    tags_t tags;
    moves_t moves;
    std::string result;

    // temporaries
    std::string name;
    ply_data ply;
};


struct game_parser : public bs::grammar< game_parser >
{
    game_data& data;
    ply_data empty_ply;
    game_parser( game_data& data_ ) : data(data_){}

    template< typename ScannerT >
    struct definition
    {
        definition( game_parser const& self )
        {
            using namespace bs;
            game_data& g = self.data;
            ply_data&  p = g.ply;

            root
                =   game >> end_p
                ;
            game
                =   * tag
                    >> * move
                    >>   RESULT[ assign_a(g.result) ]
                ;
            tag
                =   ch_p('[')
                    >> tag_name [ assign_a(g.name) ]
                    >> confix_p( '"', (*print_p)[ insert_at_a(g.tags, g.name) ], '"' )
                                [ assign_a(g.name, "") ]
                    >> ch_p(']')
                ;
            tag_name
                =   *( alnum_p | ch_p( '_' ) )
                ;
            move
                =   NUMBER
                    >>   PLY [ assign_a(p.ply) ]
                             [ push_back_a(g.moves, p) ]
                             [ assign_a( p, self.empty_ply ) ]
                    >> ! NAG
                    >> ! COMMENT
                    >> ! PLY [ assign_a(p.ply) ]
                             [ push_back_a(g.moves, p) ]
                             [ assign_a( p, self.empty_ply ) ]
                    >> ! NAG
                    >> ! COMMENT
                ;
            NUMBER
                =   lexeme_d[ uint_p >> ch_p( '.' ) ]
                ;
            PLY
                =   lexeme_d
                    [
                        chset_p( "NBRQK" ) [ assign_a(p.piece) ]
                        >>  ! (
                                  ch_p('x') [ assign_a(p.capture) ]
                                | ch_p('-')
                              )
                        >>   range_p( 'a', 'h' ) [ assign_a(p.file) ]
                        >>   range_p( '1', '8' ) [ assign_a(p.rank) ]
                        >> ! ( str_p("++") | ch_p('+') | ch_p('#') )
                                                 [ assign_a(p.check) ]
                    ]
                  | lexeme_d
                    [
                        chset_p( "NBRQK" ) [ assign_a(p.piece) ]
                        >>   (
                                  range_p('a', 'h') [ assign_a(p.afile) ]
                                | range_p('1', '8') [ assign_a(p.arank) ]
                              )
                        >> ! (
                                  ch_p('x')[ assign_a(p.capture) ]
                                | ch_p('-')
                              )
                        >>   range_p( 'a', 'h' ) [ assign_a(p.file) ]
                        >>   range_p( '1', '8' ) [ assign_a(p.rank) ]
                        >> ! ( str_p("++") | ch_p('+') | ch_p('#') )
                                                 [ assign_a(p.check) ]
                    ]
                  | lexeme_d
                    [
                        range_p( 'a', 'h' ) [ assign_a(p.file) ]
                        >>   range_p( '1', '8' ) [ assign_a(p.rank) ]
                        >> ! (  ch_p('=')
                                >> chset_p( "NBRQ" ) [ assign_a(p.promoted) ]
                              )
                        >> ! ( str_p("++") | ch_p('+') | ch_p('#') )
                                                 [ assign_a(p.check) ]
                    ]
                  | lexeme_d
                    [
                        range_p( 'a', 'h' ) [ assign_a(p.afile) ]
                        >>   ch_p('x') [ assign_a(p.capture) ]
                        >>   range_p( 'a', 'h' ) [ assign_a(p.file) ]
                        >>   range_p( '1', '8' ) [ assign_a(p.rank) ]
                        >> ! (  ch_p('=')
                                >> chset_p( "NBRQ" ) [ assign_a(p.promoted) ]
                              )
                        >> ! ( str_p("++") | ch_p('+') | ch_p('#') )
                                                 [ assign_a(p.check) ]
                    ]
                  | (
                        str_p( "O-O-O" ) [ assign_a(p.castling) ]
                        >> ! ( str_p("++") | ch_p('+') | ch_p('#') )
                                                 [ assign_a(p.check) ]
                     )
                  |  (
                        str_p( "O-O" ) [ assign_a(p.castling) ]
                        >> ! ( str_p("++") | ch_p('+') | ch_p('#') )
                                                 [ assign_a(p.check) ]
                     )
                ;
            RESULT
                =   str_p( "1-0" )
                    | str_p( "0-1" )
                    | str_p( "1/2-1/2" )
                    | ch_p( '*' )
                ;
            COMMENT
                =   lexeme_d[ confix_p( '{', *anychar_p, '}' ) ]
                ;
            NAG
                =   lexeme_d[ ch_p( '$' ) >> uint_p ]
                ;
        }
        bs::rule<ScannerT> root, game, tag, tag_name, move,
                       NUMBER, PLY, NAG, COMMENT, RESULT, CASTLING;
        const bs::rule<ScannerT>& start() const { return root; }
    };
};

#endif // SECONDO_ALGEBRAS_CHESS_PGN_GRAMMAR_HPP
