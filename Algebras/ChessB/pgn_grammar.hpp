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
    std::string check, castling, capture, file,
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
            game_data& g = self.data;
            ply_data&  p = g.ply;

            root
                =   game >> bs::end_p
                ;
            game
                =   * tag
                    >> * move
                    >>   RESULT[ bs::assign_a(g.result) ]
                ;
            tag
                =   bs::ch_p('[')
                    >> tag_name [ bs::assign_a(g.name) ]
                    >> bs::confix_p( '"', (*bs::print_p)[ bs::insert_at_a(g.tags, g.name) ], '"' )
                                [ bs::assign_a(g.name, "") ]
                    >> bs::ch_p(']')
                ;
            tag_name
                =   *( bs::alnum_p | bs::ch_p( '_' ) )
                ;
            move
                =   NUMBER
                    >>   PLY [ bs::assign_a(p.ply) ]
                             [ bs::push_back_a(g.moves, p) ]
                             [ bs::assign_a( p, self.empty_ply ) ]
                    >> ! NAG
                    >> ! COMMENT
                    >> ! PLY [ bs::assign_a(p.ply) ]
                             [ bs::push_back_a(g.moves, p) ]
                             [ bs::assign_a( p, self.empty_ply ) ]
                    >> ! NAG
                    >> ! COMMENT
                ;
            NUMBER
                =   bs::lexeme_d[ bs::uint_p >> bs::ch_p( '.' ) ]
                ;
            PLY
                =   bs::lexeme_d
                    [
                        bs::chset_p( "NBRQK" ) [ bs::assign_a(p.piece) ]
                        >>  ! (
                                  bs::ch_p('x') [ bs::assign_a(p.capture) ]
                                | bs::ch_p('-')
                              )
                        >>   bs::range_p( 'a', 'h' ) [ bs::assign_a(p.file) ]
                        >>   bs::range_p( '1', '8' ) [ bs::assign_a(p.rank) ]
                        >> ! ( bs::str_p("++") | bs::ch_p('+') | bs::ch_p('#') )
                                                 [ bs::assign_a(p.check) ]
                    ]
                  | bs::lexeme_d
                    [
                        bs::chset_p( "NBRQK" ) [ bs::assign_a(p.piece) ]
                        >>   (
                                  bs::range_p('a', 'h') [ bs::assign_a(p.afile) ]
                                | bs::range_p('1', '8') [ bs::assign_a(p.arank) ]
                              )
                        >> ! (
                                  bs::ch_p('x')[ bs::assign_a(p.capture) ]
                                | bs::ch_p('-')
                              )
                        >>   bs::range_p( 'a', 'h' ) [ bs::assign_a(p.file) ]
                        >>   bs::range_p( '1', '8' ) [ bs::assign_a(p.rank) ]
                        >> ! ( bs::str_p("++") | bs::ch_p('+') | bs::ch_p('#') )
                                                 [ bs::assign_a(p.check) ]
                    ]
                  | bs::lexeme_d
                    [
                        bs::range_p( 'a', 'h' ) [ bs::assign_a(p.file) ]
                        >>   bs::range_p( '1', '8' ) [ bs::assign_a(p.rank) ]
                        >> ! (  bs::ch_p('=')
                                >> bs::chset_p( "NBRQ" ) [ bs::assign_a(p.promoted) ]
                              )
                        >> ! ( bs::str_p("++") | bs::ch_p('+') | bs::ch_p('#') )
                                                 [ bs::assign_a(p.check) ]
                    ]
                  | bs::lexeme_d
                    [
                        bs::range_p( 'a', 'h' ) [ bs::assign_a(p.afile) ]
                        >>   bs::ch_p('x') [ bs::assign_a(p.capture) ]
                        >>   bs::range_p( 'a', 'h' ) [ bs::assign_a(p.file) ]
                        >>   bs::range_p( '1', '8' ) [ bs::assign_a(p.rank) ]
                        >> ! (  bs::ch_p('=')
                                >> bs::chset_p( "NBRQ" ) [ bs::assign_a(p.promoted) ]
                              )
                        >> ! ( bs::str_p("++") | bs::ch_p('+') | bs::ch_p('#') )
                                                 [ bs::assign_a(p.check) ]
                    ]
                  | (
                        bs::str_p( "O-O-O" ) [ bs::assign_a(p.castling) ]
                        >> ! ( bs::str_p("++") | bs::ch_p('+') | bs::ch_p('#') )
                                                 [ bs::assign_a(p.check) ]
                     )
                  |  (
                        bs::str_p( "O-O" ) [ bs::assign_a(p.castling) ]
                        >> ! ( bs::str_p("++") | bs::ch_p('+') | bs::ch_p('#') )
                                                 [ bs::assign_a(p.check) ]
                     )
                ;
            RESULT
                =   bs::str_p( "1-0" )
                    | bs::str_p( "0-1" )
                    | bs::str_p( "1/2-1/2" )
                    | bs::ch_p( '*' )
                ;
            COMMENT
                =   bs::lexeme_d[ bs::confix_p( '{', *bs::anychar_p, '}' ) ]
                ;
            NAG
                =   bs::lexeme_d[ bs::ch_p( '$' ) >> bs::uint_p ]
                ;
        }
        bs::rule<ScannerT> root, game, tag, tag_name, move,
                       NUMBER, PLY, NAG, COMMENT, RESULT, CASTLING;
        const bs::rule<ScannerT>& start() const { return root; }
    };
};

#endif // SECONDO_ALGEBRAS_CHESS_PGN_GRAMMAR_HPP
