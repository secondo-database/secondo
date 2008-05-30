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

operator length alias LENGTH pattern op(_)
operator ifthenelse alias IFTHENELSE pattern op (_, _, _)
operator sentences alias SENTENCES pattern _ op
operator dice alias DICE pattern op (_, _, _)
operator contains alias CONTAINS pattern _ infixop _
operator keywords alias KEYWORDS pattern _ op
operator sentences alias SENTENCES pattern _ op
operator getcatalog alias GETCATALOG pattern op( _ )
operator subtext alias SUBTEXT pattern op(_, _, _)
operator find alias FIND pattern op(_, _)
operator evaluate alias EVALUATE pattern op( _ )
operator replace alias REPLACE pattern op(_, _, _)
operator getTypeNL alias GETTYPENL pattern _ op
operator getValueNL alias GETVALUENL pattern _ op
operator toobject alias TOOBJECT pattern op(_, _)
operator chartext alias CHARTEXT pattern op(_)
operator toupper alias TOUPPER pattern op(_)
operator tolower alias TOLOWER pattern op(_)
operator totext alias TOTEXT pattern op(_)
operator sendtext alias SENDTEXT pattern op(_, _, _, _)
operator receivetext alias RECEIVETEXT pattern op(_, _, _)
operator isDBObject alias ISDBOBJECT pattern op( _ )


operator substr alias SUBSTR pattern op(_, _, _)
operator isempty alias ISEMPTY pattern op( _ )
operator + alias PLUS pattern _ infixop  _
operator = alias EQ pattern _ infixop _
operator # alias NE pattern _ infixop _
operator < alias LT pattern _ infixop _
operator > alias GT pattern _ infixop _
operator <= alias LE pattern _ infixop _
operator >= alias GE pattern _ infixop _
operator tostring alias TOSTRING pattern op(_)
