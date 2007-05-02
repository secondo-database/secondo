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

operator readpgn alias READPGN pattern  op ( _ )
operator agent alias AGENT pattern _ op
operator captured alias CAPTURED pattern _ op
operator startrow alias STARTROW pattern _ op
operator endrow alias ENDROW pattern _ op
operator startfile alias STARTFILE pattern _ op
operator endfile alias ENDFILE pattern _ op
operator check alias CHECK pattern _ op
operator captures alias CAPTURES pattern _ op
operator cnt alias CNT pattern _ op [ _ ]
operator pieces alias PIECES pattern _ op
operator moveNo alias MOVENO pattern _ op
operator posrange alias POSRANGE pattern _ op [_, _, _, _]
operator includes alias INCLUDES pattern _ op [_]
operator getposition alias GETPOSITION pattern _ op [ _ ]
operator getmove alias GETMOVE pattern _ op [ _ ]
operator getkey alias GETKEY pattern _ op [ _ ]
operator moves alias MOVES pattern _ op
operator positions alias POSITIONS pattern _ op
operator movingpoints alias MOVINGPOINTS pattern _ op