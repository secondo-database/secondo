/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//characters [1] verbatim:  [\verb@]         [@]
//paragraph  [1] title:     [{\Large \bf ]   [}]


[1] OperatorsInfo.hpp

1. defines information on operators for secondo user

*/

#ifndef SECONDO_ALGEBRAS_CHESS_OPERATORS_INFO_HPP
#define SECONDO_ALGEBRAS_CHESS_OPERATORS_INFO_HPP

const OperatorInfo field_ctor_info( "field", "string -> field",
    "field ( _ )", "Constructs field", "query field( \"a1\" )" );

const OperatorInfo equals_field_info( "=", "field x field -> bool",
    "_ = _", "Checks for equality", "query a1 = a1" );

const OperatorInfo less_field_info( "<", "field x field -> bool",
    "_ < _", "Field comparision", "query a1 < a2" );

const OperatorInfo greater_field_info( ">", "field x field -> bool",
    "_ > _", "Field comparision", "query a1 > d6" );

const OperatorInfo iswhite_field_info( "iswhite", "field -> bool",
    "iswhite ( _ )", "Return color of the field",
    "query iswhite( a1 )" );

const OperatorInfo file_info( "column", "field -> string", "column ( _ )",
    "Return column of the field", "query column( a1 )" );

const OperatorInfo row_info( "row", "field -> int", "row ( _ )",
    "Return row of the field", "query row( a1 )" );

const OperatorInfo north_info( "north", "field -> field", "north ( _ )",
    "Return field to the north", "query north( a1 )" );

const OperatorInfo east_info( "east", "field -> field", "east ( _ )",
    "Return field to the east", "query east( a1 )" );

const OperatorInfo south_info( "south", "field -> field", "south ( _ )",
    "Return field to the south", "query south( a1 )" );

const OperatorInfo west_info( "west", "field -> field", "west ( _ )",
    "Return field to the west", "query west( a1 )" );

const OperatorInfo northwest_info( "northwest", "field -> field",
    "northwest ( _ )", "Field to the northwest",
    "query northwest( a1 )" );

const OperatorInfo northeast_info( "northeast", "field -> field",
    "northeast ( _ )", "Field to the northeast",
    "query northeast( a1 )" );

const OperatorInfo southwest_info( "southwest", "field -> field",
    "southwest ( _ )", "Field to the southwest",
    "query southwest( a1 )" );

const OperatorInfo southeast_info( "southeast", "field -> field",
    "southeast ( _ )", "Field to the southeast",
    "query southeast( a1 )" );

const OperatorInfo is_neighbor_info( "is_neighbor", "field x field -> bool",
    "_ is_neighbor _", "Are two fields neighbors",
    "query a1 is_neighbor a2" );

const OperatorInfo left_info( "left", "field x field -> bool", "_ left _",
    "Is first field to the left of the second",
    "query a1 left b1" );

const OperatorInfo right_info( "right", "field x field -> bool", "_ right _",
    "Is first field to the right of the second",
    "query d1 right d2" );

const OperatorInfo above_info( "above", "field x field -> bool",
    "_ above _", "Is first field above of the second",
    "query a1 left a1" );

const OperatorInfo below_info( "below", "field x field -> bool",
    "_ below _", "Is first field below of the second",
    "query a1 below a1" );

const OperatorInfo neighbors_info( "neighbors", "field -> stream( field )",
    "_ neighbors", "Produce stream of neighbors",
    "query a1 neighbors printstream count" );

//-----------------------------------------------------------------------------
const OperatorInfo piece_ctor_info( "piece", "string -> piece",
    "piece ( _ )", "Constructs piece", "query piece( \"Pawn\" )" );

const OperatorInfo iswhite_piece_info( "iswhite", "piece -> bool",
    "iswhite ( _ )", "Return piece color.",
    "query iswhite( Pawn )" );

const OperatorInfo equals_piece_info( "=", "piece x piece -> bool",
    "_ = _", "Checks for equality",
    "query Pawn = king" );

const OperatorInfo is_info( "is", "piece x piece -> bool", "_ is _",
    "Checks for the equal piece type",
    "query Pawn is pawn" );

const OperatorInfo samecolor_info( "samecolor", "piece x piece -> bool",
    "_ samecolor _", "Checks for equal piece color",
    "query Pawn samecolor Queen" );

const OperatorInfo piecevalue_info( "piecevalue", "piece -> real",
    "piecevalue ( _ )", "Return value of the piece.",
    "query piecevalue( Pawn ) = 1" );

//-----------------------------------------------------------------------------
const OperatorInfo equals_ply_info( "=", "chessmove x chessmove -> bool",
    "_ = _", "Checks for equality",
    "query query wjc feed filter[.No = 5] extract[elem] getmove[7] = "
    "query wjc feed filter[.No = 5] extract[elem] getmove[6]" );

const OperatorInfo startfield_info( "startfield", "chessmove -> field",
    "startfield ( _ )", "Returns the start field.",
    "query startfield(wjc feed filter[.No = 5] extract[elem] getmove[7])" );

const OperatorInfo endfield_info( "endfield", "chessmove -> field",
    "endfield ( _ )", "Returns destination field.",
    "query endfield(wjc feed filter[.No = 5] extract[elem] getmove[7])" );

const OperatorInfo agent_info( "agent", "chessmove -> piece", "_ agent",
    "Returns the moving piece.",
    "query wjc feed filter[.No = 5] extract[elem] getmove[7] agent" );

const OperatorInfo captures_info( "captures", "chessmove -> bool",
    "_ captures", "Check for a capture.",
    "query wjc feed filter[.No = 5] extract[elem] getmove[7] captures" );

const OperatorInfo captured_info( "captured", "chessmove -> piece",
    "_ captured", "Returns captured piece.",
    "query wjc feed filter[.No = 5] extract[elem] getmove[7] captured" );

const OperatorInfo check_info( "check", "chessmove -> bool", "_ check",
    "Checks for the check.",
    "query wjc feed filter[.No = 5] extract[elem] getmove[7] check" );

const OperatorInfo is_mate_info( "is_mate", "chessmove -> bool",
    "is_mate ( _ )", "Checks for the mate.",
    "query is_mate( wjc feed filter[.No = 5] extract[elem] getmove[7] )" );

const OperatorInfo is_stalemate_info( "is_stalemate", "chessmove -> bool",
    "is_stalemate ( _ )", "Checks for stalemate.",
    "query is_stalemate(wjc feed filter[.No = 5] extract[elem] getmove[7])" );

const OperatorInfo is_castling_info( "is_castling", "chessmove -> bool",
    "is_castling ( _ )", "Check for a castling.",
    "query is_castling( wjc feed filter[.No = 5] extract[elem] getmove[7] )" );

const OperatorInfo is_enpassant_info( "is_enpassant", "chessmove -> bool",
  "is_enpassant ( _ )", "Checks for an enpassant.",
  "query is_enpassant(wjc feed filter[.No = 5] extract[elem] getmove[7])" );

const OperatorInfo enpassant_field_info( "enpassant_field",
  "chessmove -> field", "enpassant_field ( _ )",
  "Returns the enpassant field.",
  "query enpassant_field(wjc feed filter[.No = 5] extract[elem] getmove[7])" );

//-----------------------------------------------------------------------------
const OperatorInfo piececount_material_info( "piececount",
    "material x piece -> int", " _ piececount [ _ ]", "Count of pieces",
    "query wjc feed filter[.No = 2] extract[elem] getposition[1] pieces"
    " piececount[Queen]" );

const OperatorInfo equals_material_info( "=", "material x material -> bool",
    "_ = _", "Checks for equality",
    "query wjc feed filter[.No = 2] extract[elem] getposition[6] pieces = "
    "query wjc feed filter[.No = 2] extract[elem] getposition[7] pieces" );

const OperatorInfo less_material_info( "<", "material x material -> bool",
    "_ < _", "material comparision",
    "query wjc feed filter[.No = 2] extract[elem] getposition[6] pieces < "
    "query wjc feed filter[.No = 2] extract[elem] getposition[7] pieces" );

const OperatorInfo greater_material_info( ">", "material x material -> bool",
    "_ > _", "material comparision",
    "query wjc feed filter[.No = 2] extract[elem] getposition[6] pieces > "
    "query wjc feed filter[.No = 2] extract[elem] getposition[7] pieces" );

const OperatorInfo approx_material_info("~","material x material -> bool",
    "_ ~ _", "Approx. equality",
    "query wjc feed filter[.No = 2] extract[elem] getposition[6] pieces ~ "
    "wjc feed filter[.No = 2] extract[elem] getposition[7] pieces" );

//-----------------------------------------------------------------------------
const OperatorInfo pieces_info( "pieces", "position -> material",
    "pieces _ ", "Material",
    "query wjc feed filter[.No = 2] extract[elem] getposition[6] pieces" );

const OperatorInfo moveNo_info( "moveNo", "position -> int",
    "_ moveNo", "Move number",
    "query wjc feed filter[.No = 2] extract[elem] getposition[5] moveNo" );

const OperatorInfo checkmate_info( "checkmate", "position -> bool",
    "checkmate ( _ )", "Mate check",
    "query checkmate(wjc feed filter[.No = 2] extract[elem] getposition[6])" );

const OperatorInfo stalemate_info( "stalemate", "position -> bool",
    "stalemate ( _ )", "Stalemate check",
    "query stalemate(wjc feed filter[.No = 2] extract[elem] getposition[6])" );

//-----------------------------------------------------------------------------
const OperatorInfo includes_info( "includes", "position x position -> bool",
    "_ includes [ _ ]", "...",
    "query wjc feed filter[.No = 2] extract[elem] getposition[34] includes"
    " [ wjc feed filter[.No = 2] extract[elem] getposition[34] ]" );

const OperatorInfo piececount_spos_info( "piececount",
    "position x string -> int", "_ piececount [ _ ]", "...",
    "query wjc feed filter[.No = 2] extract[elem] getposition[34] "
    "piececount[\"KING\"]" );

const OperatorInfo piececount_position_info( "piececount",
    "position x piece -> int", "_ piececount [ _ ]", "...",
    "query wjc feed filter[.No = 2] extract[elem] getposition[34] "
    "piececount[Pawn]" );

const OperatorInfo equals_position_info( "=", "position x position -> bool",
    "_ = _", "Position equality",
    "query wjc feed filter[.No = 2] extract[elem] getposition[7] = "
    "wjc feed filter[.No = 2] extract[elem] getposition[6]" );

const OperatorInfo less_position_info( "<", "position x position -> bool",
    "_ < _", "position comparision",
    "query wjc feed filter[.No = 2] extract[elem] getposition[7] < "
    "wjc feed filter[.No = 2] extract[elem] getposition[7]" );

const OperatorInfo greater_position_info( ">", "position x position -> bool",
    "_ > _", "position comparision",
    "query wjc feed filter[.No = 2] extract[elem] getposition[7] > "
    "wjc feed filter[.No = 2] extract[elem] getposition[6]" );

const OperatorInfo approx_position_info( "~", "position x position -> bool",
    "_ ~ _", "Position material comparision", "query wjc feed filter[.No = 2]"
    " extract[elem] getposition[6] ~ wjc feed filter[.No = 2]"
    " extract[elem] getposition[6]" );

const OperatorInfo attackcount_info("attackcount", "position x field -> int",
    "_ attackcount [ _ ]", "...",
    "query wjc feed filter[.No = 2] extract[elem]"
    " getposition[6] attackcount [e4]" );

const OperatorInfo protectcount_info("protectcount","position x field -> int",
    "_ protectcount [ _ ]", "...",
    "query wjc feed filter[.No = 2] extract[elem]"
    " getposition[6] protectcount [e4]" );

const OperatorInfo apply_ply_info( "apply_ply",
    "position x chessmove -> position", "_ apply_ply[ _ ]", "...",
    "query wjc feed filter[.No = 2] extract[elem] getposition[6] "
    "apply_ply[wjc feed filter[.No = 2] extract[elem] getmove[7] ]" );

//-----------------------------------------------------------------------------
const OperatorInfo posrange_info( "posrange_b",
    "position x field x field -> position", "_ posrange [ _, _ ]",
    "...", "query wjc feed filter[.No = 2] extract[elem] getposition[6] "
    "posrange[a1, c3]" );

const OperatorInfo attacked_by_p_info( "attackedby",
    "position x piece x piece -> bool", "_ _ attackedby [ _ ]",
    "...", "query wjc feed filter[.No = 2] extract[elem] getposition[6]"
    " queen attackedby[Queen]" );

const OperatorInfo attacked_by_f_info( "attackedby",
    "position x field x piece -> bool", "_ _ attackedby [ _ ]",
    "...", "query wjc feed filter[.No = 2] extract[elem] getposition[6]"
    " d4 attackedby [Queen]" );

const OperatorInfo attacked_from_p_info( "attackedfrom",
    "position x piece x field -> bool", "_ _ attackedfrom [ _ ]",
    "...", "query wjc feed filter[.No = 2] extract[elem] getposition[6]"
    " Queen attackedfrom [d4]" );

const OperatorInfo attacked_from_f_info( "attackedfrom",
    "position x field x field -> bool", "_ _ attackedfrom [ _ ]",
    "...", "query wjc feed filter[.No = 2] extract[elem] getposition[6]"
    " c4 attackedfrom [d4]" );

const OperatorInfo protected_by_p_info( "protectedby",
    "position x piece x piece -> bool", "_ _ protectedby [ _ ]",
    "...", "query wjc feed filter[.No = 2] extract[elem] getposition[6]"
    " queen protectedby [Queen]" );

const OperatorInfo protected_by_f_info( "protectedby",
    "position x field x piece -> bool", "_ _ protectedby [ _ ]",
    "...", "query wjc feed filter[.No = 2] extract[elem] getposition[6]"
    " c4 protectedby [Queen]" );

const OperatorInfo protected_from_p_info( "protectedfrom",
    "position x piece x field -> bool", "_ _ protectedfrom [ _ ]",
    "...", "query wjc feed filter[.No = 2] extract[elem] getposition[6]"
    " Queen protectedfrom [d4]" );

const OperatorInfo protected_from_f_info( "protectedfrom",
    "position x field x field -> bool", "_ _ protectedfrom [ _ ]",
    "...", "query wjc feed filter[.No = 2] extract[elem] getposition[6]"
    " c4 protectedfrom [d4]" );

const OperatorInfo apply_move_info( "apply_move",
    "position x field x field -> position", "_ apply_move[_, _]",
    "...", "query wjc feed filter[.No = 2] extract[elem] getposition[6]"
    " apply_move[c4, d4]" );

//-----------------------------------------------------------------------------
const OperatorInfo pos_fields_info( "pos_fields",
    "position -> stream( tuple( [Field: field, Piece: piece] ) )",
    "_ pos_fields", "...", "query wjc feed filter[.No = 2] extract[elem]"
    " getposition[6] pos_fields consume" );

const OperatorInfo pos_moves_info( "pos_moves",
    "position x piece -> stream(tuple([SPiece: piece, SField: field, EPiece: "
    "piece, EField: field]))", "_ pos_moves", "...", "query wjc feed "
    "filter[.No = 2] extract[elem] getposition[6] pos_moves consume" );

const OperatorInfo pos_moves_blocked_info( "pos_moves_blocked",
    "position -> stream(tuple([SPiece: piece, SField: field, BPiece: piece, "
    "BField: field, EPiece: piece, EField: field]))", "_ pos_moves_blocked",
    "...", "query wjc feed filter[.No = 2] extract[elem] getposition[6] "
    "pos_moves_blocked consume" );

const OperatorInfo piece_moves_info( "piece_moves",
    "position x piece -> stream(tuple([SPiece: piece, SField: field,"
    "EPiece: piece, EField: field]))", "_ piece_moves [ _ ]", "...",
    "query wjc feed filter[.No = 2] extract[elem] getposition[6]"
    " piece_moves [Queen] consume" );

const OperatorInfo p_moves_blocked_info( "piece_moves_blocked",
    "position x piece -> stream(tuple([SPiece: piece, SField: field, "
    "BPiece: piece, BField: field, EPiece: piece, EField: field]))",
    "_ piece_moves_blocked[ _ ]", "..", "query wjc feed filter[.No = 2] "
    "extract[elem] getposition[6] piece_moves_blocked[Pawn]" );
//-----------------------------------------------------------------------------
const OperatorInfo getkey_info( "getkey", "chessgame x string -> string",
    "_ getkey [ _ ]", "...",
    "query wjc feed filter[.No = 2] extract[elem] getkey [\"Event\"]" );

const OperatorInfo getposition_info( "getposition",
    "chessgame x int -> position", "_ getposition [ _ ]",
    "...", "query wjc feed filter[.No = 2] extract[elem] getposition[6]" );

const OperatorInfo getmove_info( "getmove", "chessgame x int -> chessmove",
    "_ getmove [ _ ]", "...",
    "query wjc feed filter[.No = 2] extract[elem] getmove [3]" );

const OperatorInfo lastmove_info( "lastmove", "chessgame -> int",
    "lastmove ( _ )", "...",
    "query lastmove( wjc feed filter[.No = 2] extract[elem] )" );

const OperatorInfo readpgn_info( "readpgn", "text -> stream(chessgame)",
    "readpgn ( _ )", "Read chessgames from PGN-file",
    "query readpgn('wjc.pgn') printstream count" );

const OperatorInfo positions_info( "positions", "chessgame->stream(position)",
    "_ positions", "...", "query wjc feed filter[.No = 2] extract[elem] "
    "positions transformstream head[3] consume" );

const OperatorInfo moves_info( "moves", "chessgame -> stream(chessmove)",
    "_ moves", "...", "query wjc feed filter[.No = 2] extract[elem] moves"
    " transformstream head[3] consume" );

const OperatorInfo history_info( "history",
    "chessgame -> stream(tuple([No: int, Pos: position, Move: chessmove]))",
    "_ history", "...",
    "query wjc feed filter[.No = 2] extract[elem] history head[3] consume" );

//-----------------------------------------------------------------------------
const OperatorInfo even_info( "is_even", "int -> bool",
    "is_even ( _ )", "...", "query is_even(2)" );

const OperatorInfo odd_info( "is_odd", "int -> bool",
    "is_odd ( _ )", "...", "query is_odd(2)" );

const OperatorInfo exists_info( "exists",
    "stream(ANY) x (ANY -> bool) -> bool",
    "_ exists [ funlist ]", "...", "query r feed exists[p]" );

const OperatorInfo forall_info( "forall",
    "stream(ANY) x (ANY -> bool) -> bool",
    "_ forall [ funlist ]", "...", "query r feed forall[p]" );

const OperatorInfo twotuples_info( "twotuples",
    "stream(tuple(X)) -> stream(tuple([X1, X2]))",
    "_ twotuples", "...", "query g1 twotuples consume" );

const OperatorInfo ntuples_info( "ntuples",
    "stream(tuple(X)) -> stream(tuple([X1, X2, ...]))",
    "_ ntuples [ _ ]", "...", "query g1 ntuples[n_is_4] consume" );

#endif // SECONDO_ALGEBRAS_CHESS_OPERATORS_INFO_HPP
