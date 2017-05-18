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

operator apply alias APPLYPREDICATE pattern _ op [fun] implicit parameter blockentry type BLOCKENTRY
operator attr alias ATTR pattern op (_, _)
operator blockcount alias BLOCKCOUNT pattern _ op
operator count alias COUNT pattern _ op
operator consume alias CONSUME pattern _ op
operator cconsume alias CCONSUME pattern _ op [ _ ]
operator distance alias DISTANCE pattern op ( _ , _ )
operator feed alias FEED pattern _ op
operator feedproject alias FEEDPROJECT pattern _ op [list]
operator filter alias FILTER pattern _ op [ fun ] implicit parameter streamelem type STREAMELEM !!
operator cfilter alias CFILTER pattern _ op [ fun, _, _, _ ] implicit parameter streamelem type STREAMELEM !!
operator and alias AND pattern _ infixop _
operator or alias OR pattern _ infixop _
operator not alias NOT pattern op ( _ )
operator # alias NE pattern _ infixop _
operator < alias LT pattern _ infixop _
operator <= alias LE pattern _ infixop _
operator = alias EQ pattern _ infixop _
operator >= alias GE pattern _ infixop _
operator > alias GT pattern _ infixop _
operator rename alias RENAME pattern _ op [ _ ]
operator itHashJoin alias ITHASHJOIN pattern _ _ op [_,_]
operator itSpatialjoin alias SPATIALJOIN pattern _ _ op [_, _, _, _, _]

operator cand alias CAND pattern _ op [fun, fun] implicit parameter block type ANY
operator cor alias COR pattern _ op [fun, fun] implicit parameter block type ANY

operator repeat alias REPEAT pattern op(_,_)
operator totuples alias TOTUPLES pattern _ op
operator toblocks alias TOBLOCKS pattern _ op [ _ ]
operator test alias TEST pattern op ( _ )