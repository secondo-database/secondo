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

operator + alias PLUS pattern _ infixop _
operator - alias MINUS pattern _ infixop _
operator * alias TIMES pattern _ infixop _
operator / alias DIVIDEDBY pattern _ infixop _

operator translate alias TRANSLATE pattern _ op [list]
operator scale alias TRANSLATE pattern _ op [_,_]

operator toPrecise alias TOPRECISE pattern op(_)

operator makePrecPoint alias MAKEPRECPOINT pattern op(_,_)

operator contains alias CONTAINS pattern _ infixop _

operator intersects alias INTERSECTS pattern _ infixop _

operator union alias UNION pattern _ infixop _

operator intersection alias INTERSECTION pattern op(_,_) 

operator difference alias DIFFERENCE pattern op(_,_)

operator str2precise alias STR2PRECISE pattern op(_)






