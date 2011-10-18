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

operator intersects alias INTERSECTS pattern _ infixop _
operator inside alias INSIDE pattern _ infixop _
operator before alias BEFORE pattern _ infixop _
operator union alias UNION pattern _ infixop _
operator minimum alias MINIMUM pattern op ( _ )
operator maximum alias MAXIMUM pattern op ( _ )
operator inst alias INST pattern op ( _ )
operator val alias VAL pattern op ( _ )
operator atinstant alias ATINSTANT pattern _ infixop _
operator atperiods alias ATPERIODS pattern _ infixop _
operator deftime alias DEFTIME pattern op ( _ )
operator trajectory alias TRAJECTORY pattern op ( _ )
operator present alias PRESENT pattern _ infixop _
operator passes alias PASSES pattern _ infixop _
operator initial alias INITIAL pattern op ( _ )
operator final alias FINAL pattern op ( _ )
operator at alias AT pattern _ infixop _
operator units alias UNITS pattern op ( _ )
operator getunit alias GETUNIT pattern op ( _, _ )
operator theyear alias THEYEAR pattern op ( _ )
operator themonth alias THEMONTH pattern op ( _, _ )
operator theday alias THEDAY pattern op ( _, _, _ )
operator thehour alias THEHOUR pattern op ( _, _, _ ,_ )
operator theminute alias THEMINUTE pattern op ( _, _, _ ,_ , _ )
operator thesecond alias THESECOND pattern op ( _, _, _ ,_ , _, _ )
operator theperiod alias THEPERIOD pattern op ( _, _ )
operator theRange alias THERANGE pattern op( _ , _, _, _ )

operator equal alias EQUAL pattern _ infixop _
operator nonequal alias NONEQUAL pattern _ infixop _

operator bbox alias BBOX pattern op ( _, _ )
operator mbrange alias MBRANGE pattern op ( _ )
operator box2d alias BOX2D pattern op ( _ )
operator bbox2d alias BBOX2D pattern op ( _, _ )
operator bboxold alias BBOXOLD pattern op ( _, _ )
operator simplify alias SIMPLIFY pattern op (_, _)
operator breakpoints alias BREAKPOINTS pattern op(_,_)
operator integrate alias INTEGRATE pattern op(_)
operator minimum alias MINIMUM pattern op(_)
operator maximum alias MAXIMUM pattern op(_)

operator approximate alias APPROXIMATE pattern _ op [_, _ ]
operator translateappend alias TRANSLATEAPPEND pattern _ op [_, _ ]
operator translateappendS alias TRANSLATEAPPENDS pattern _ op [_, _ ]
operator reverse alias REVERSE pattern op ( _ )
operator samplempoint alias SAMPLEMPOINT pattern op ( _ , _ )
operator gps alias GPS pattern op(_, _)
operator disturb alias DISTURB pattern _ op [_, _]
operator length alias LENGTH pattern op ( _ )


operator equalizeU alias EQUALIZEU pattern _ op [_]
operator vertices alias VERTICES pattern op( _ )
operator hat alias HAT pattern op ( _ )
operator speedup alias SPEEDUP pattern _ op [_]
operator avg_speed alias AVG_SPEED pattern op(_)
operator submove alias SUBMOVE pattern _ op[_]
operator uval alias UVAL pattern op ( _ )
operator mp2onemp alias MP2ONEMP pattern op(_,_,_)
operator p2mp alias MP2ONEMP pattern op(_,_,_,_)
operator delay alias DELAY pattern op(_,_)
operator distancetraversed alias DISTANCETRAVERSED pattern op(_)
operator mint2mbool alias MINT2MBOOL pattern op(_)
operator mint2mreal alias MINT2MREAL pattern op(_)
operator turns alias TURNS pattern op( _, _, _, _, _, _ )
operator timeshift alias TIMESHIFT pattern _ op[ _ ]
operator gridcellevents alias GRIDCELLEVENTS pattern op(_)
operator squareddistance alias SQUAREDDISTANCE pattern op ( _ , _ )
operator getRefinementPartion alias GETREFINEMENTPARTION pattern op( _, _ )

operator atRect alias atRect pattern _ infixop _
operator moveTo alias moveTo pattern _ op [_]

operator fillGaps alias fillGaps pattern op(_,_)
operator removeShort alias REMOVESHORT pattern op(_,_)

