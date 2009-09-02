#This file is part of SECONDO.

#Copyright (C) 2004, University in Hagen, Department of Computer Science, 
#Database Systems for New Applications.

#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

operator = alias EQUAL pattern _ infixop _
operator < alias LESS pattern _ infixop _
operator ~ alias APPROXEQUAL pattern  _ infixop _
operator is alias IS pattern _ infixop _
operator samecolor alias SAMECOLOR pattern _ infixop _
operator is_neighbor alias IS_NEIGHBOR pattern _ infixop _
operator left alias LEFT pattern _ infixop _
operator right alias RIGHT pattern _ infixop _
operator above alias ABOVE pattern _ infixop _
operator below alias BELOW pattern _ infixop _

operator iswhite alias ISWHITE pattern op ( _ )
operator piecevalue alias PIECEVALUE pattern op ( _ )
operator column alias COLUMN pattern op ( _ )
operator row alias ROW pattern op ( _ )
operator north alias NORTH pattern op ( _ )
operator south alias SOUTH pattern op ( _ )
operator west alias WEST pattern op ( _ )
operator east alias EAST pattern op ( _ )
operator northwest alias NORTHWEST pattern op ( _ )
operator southeast alias SOUTHEAST pattern op ( _ )
operator northeast alias NORTHEAST pattern op ( _ )
operator southwest alias SOUTHWEST pattern op ( _ )

operator startfield alias from pattern op ( _ )
operator endfield alias to pattern op ( _ )
operator agent alias AGENTB pattern _ op
operator captures alias capturesb pattern _ op 
operator captured alias CAPTUREDB pattern _ op
operator check alias is_checkb pattern _ op
operator checkmate alias CHECKMATE pattern op ( _ )
operator is_mate alias ISMATE pattern op ( _ )
operator stalemate alias is_stalemate pattern op ( _ )
operator is_castling alias IS_CASTLING pattern op ( _ )
operator is_enpassant alias IS_ENPASSANT pattern op ( _ )
operator enpassant_field alias ENPASSANT_FIELD pattern op ( _ )
operator readpgn alias READPGN pattern  op ( _ )
operator lastmove alias LASTMOVE pattern  op ( _ )
operator is_even alias IS_EVEN pattern  op ( _ )
operator is_odd alias IS_ODD pattern  op ( _ )

operator neighbors alias NEIGHBORS pattern _ op
operator pieces alias PIECES pattern _ op
operator moveNo alias MOVENO pattern _ op
operator pos_fields alias POS_FIELDS pattern _ op
operator pos_moves alias POS_MOVES pattern _ op
operator pos_moves_blocked alias POS_MOVES_BLOCKED pattern _ op
operator positions alias POSITIONS pattern _ op
operator moves alias MOVES pattern _ op
operator history alias HISTORY pattern _ op
operator twotuples alias TWOTUPLES pattern _ op

operator includes alias INCLUDES pattern _ op [ _ ]
operator piececount alias PIECECOUNT pattern _ op [ _ ]
operator attackcount alias ATTACKCOUNT pattern _ op [ _ ]
operator protectcount alias PROTECTCOUNT pattern _ op [ _ ]
operator apply_ply alias APPLY_PLY pattern _ op [ _ ]
operator piece_moves alias PIECE_MOVES pattern _ op [ _ ]
operator piece_moves_blocked alias PIECE_MOVES_BLOCKED pattern _ op [ _ ]
operator getkey alias GETKEY pattern _ op [ _ ]
operator getposition alias GETPOSITION pattern _ op [ _ ]
operator getmove alias GETMOVE pattern _ op [ _ ]
operator exists alias EXISTS pattern _ op [ fun ] implicit parameter elem type STREAMELEM
operator forall alias FORALL pattern _ op [ fun ] implicit parameter elem type STREAMELEM
operator ntuples alias NTUPLES pattern _ op [ _ ]

operator apply_move alias APPLY_MOVE pattern _ op [ _, _ ]
operator posrange_b alias POSRANGEB pattern _ op [ _, _ ]

operator attackedby alias ATTACKEDBY pattern _ _ op [ _ ]
operator attackedfrom alias ATTACKEDFROM pattern _ _ op [ _ ]
operator protectedby alias PROTECTEDBY pattern _ _ op [ _ ]
operator protectedfrom alias PROTECTEDFROM pattern _ _ op [ _ ]

# just to test a stream operator implementation inside the
# chess algebra's class framework.
operator stddev alias STDDEV pattern _ op
#operator movingpoints alias MOVINGPOINTS pattern _ op
