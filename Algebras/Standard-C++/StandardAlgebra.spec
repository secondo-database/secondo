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

operator div alias DIV pattern _ infixop _
operator mod alias MOD pattern _ infixop _
operator randint alias RANDINT pattern op ( _ )
operator log alias LOG pattern op ( _ )

operator > alias GT pattern _ infixop _
operator < alias LT pattern _ infixop _
operator <= alias LE pattern _ infixop _
operator >= alias GE pattern _ infixop _
operator # alias NE pattern _ infixop _

operator = alias EQ pattern _ infixop _  

operator not alias NOT pattern op ( _ )
operator and alias AND pattern  _ infixop _
operator or alias OR pattern  _ infixop _

operator starts alias STARTS pattern _ infixop _
operator contains alias CONTAINS pattern _ infixop _
operator substr alias SUBSTR pattern op ( _, _ , _ )

operator isempty alias ISEMPTY pattern op ( _ )

operator intersection alias INTERSECTION pattern op (_, _)
operator minus alias SET_MINUS pattern _ infixop _

operator setoption alias SETOPTION pattern op (_, _)

operator relcount alias RELCOUNT pattern _ op
operator relcount2 alias RELCOUNT2 pattern _ op

operator keywords alias KEYWORD pattern _ op
operator elapsedtime alias ELAPSEDTIME pattern _ op
operator between alias BETWEEN pattern _ op [ _, _]

operator ldistance alias LDISTANCE pattern op (_ , _)

operator hashvalue alias HASHVALUE pattern op ( _ , _)



