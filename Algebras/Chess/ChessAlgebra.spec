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

operator movingpoints alias MOVINGPOINTS pattern op ( _ )
operator getkey alias GETKEY pattern _ infixop _
operator getmove alias GETMOVE pattern _ infixop _
operator getpos alias GETPOS pattern op ( _, _ )
operator moves alias MOVES pattern op ( _ )
operator positions alias POSITIONS pattern op ( _ )
operator moveno alias MOVENO pattern op ( _ )
operator pieces alias PIECES pattern op ( _ )
operator agent alias AGENT pattern op ( _ )
operator captured alias CAPTURED pattern op ( _ )
operator startrow alias STARTROW pattern op ( _ )
operator endrow alias ENDROW pattern op ( _ )
operator startfile alias STARTFILE pattern op ( _ )
operator endfile alias ENDFILE pattern op ( _ )
operator check alias CHECK pattern op ( _ )
operator captures alias CAPTURES pattern op ( _ )
operator chessrange alias CHESSRANGE pattern op (_, _, _, _, _)
operator includes alias INCLUDES pattern _ infixop _
operator chesscount alias CHESSCOUNT pattern _ infixop _
operator chesscountall alias CHESSCOUNTALL pattern _ op 
operator chessreadgames alias CHESSREADGAMES pattern op (_)
operator chessreadonegame alias CHESSREADONEGAME pattern op (_)