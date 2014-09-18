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
operator sentences alias SENTENCES pattern _ op
operator dice alias DICE pattern op (_, _, _)
operator contains alias CONTAINS pattern _ infixop _
operator keywords alias KEYWORDS pattern _ op
operator sentences alias SENTENCES pattern _ op
operator getcatalog alias GETCATALOG pattern op( _ )
operator subtext alias SUBTEXT pattern op(_, _, _)
operator find alias FIND pattern op(_, _)
operator replace alias REPLACE pattern op(_, _, _)
operator chartext alias CHARTEXT pattern op(_)
operator toupper alias TOUPPER pattern op(_)
operator tolower alias TOLOWER pattern op(_)
operator totext alias TOTEXT pattern op(_)
operator substr alias SUBSTR pattern op(_, _, _)
operator isempty alias ISEMPTY pattern op( _ )
operator trim alias TRIM pattern op( _ )
operator + alias PLUS pattern _ infixop  _
operator = alias EQ pattern _ infixop _
operator # alias NE pattern _ infixop _
operator < alias LT pattern _ infixop _
operator > alias GT pattern _ infixop _
operator <= alias LE pattern _ infixop _
operator >= alias GE pattern _ infixop _
operator tostring alias TOSTRING pattern op(_)
operator sendtextUDP alias SENDTEXTUDP pattern op(_, _, _, _)
operator receivetextUDP alias RECEIVETEXTUDP pattern op(_, _, _)
operator receivetextstreamUDP alias RECEIVETEXTSTREAMUDP pattern op(_, _, _)
operator sendtextstreamTCP alias SENDTEXTSTREAMTCP pattern _ op [ _, _, _, _ ]

operator isDBObject alias ISDBOBJECT pattern op( _ )
operator evaluate alias EVALUATE pattern op( _ )
operator getTypeNL alias GETTYPENL pattern _ op
operator getValueNL alias GETVALUENL pattern _ op
operator letObject alias LETOBJECT pattern op ( _, _, _ )
operator deleteObject alias DELETEOBJECT pattern op ( _ )
operator createObject alias CREATEOBJECT pattern op ( _, _, _)
operator updateObject alias UPDATEOBJECT pattern op ( _, _, _)
operator deriveObject alias DERIVEOBJECT pattern op ( _, _, _)
operator getObjectTypeNL alias GETOBJECTTYPENL pattern op ( _ )
operator getObjectValueNL alias GETOBJECTVALUENL pattern op ( _ )
operator toObject alias TOOBJECT pattern op(_, _)
operator getDatabaseName alias GETDATABASENAME pattern op( _ )
operator matchingOperatorNames alias MATCHINGOPERATORNAMES  pattern op( _,_ )
operator matchingOperators alias MATCHINGOPERATORS  pattern op( _,_ )
operator sys_getMatchingOperators alias SYS_GETMATCHINGOPERATORS pattern op( _,_ )
operator sys_getAlgebraName alias SYS_GETALGEBRANAME pattern op( _ )
operator sys_getAlgebraId alias SYS_GETALGEBRAID pattern op( _ )

operator sys_getOperatorInfo alias SYS_GETOPERATORINFO pattern op( _,_ )
operator sys_getTypeConstructorInfo alias SYS_GETTYPECONSTRUCTORINFO pattern op( _,_ )

operator md5 alias MD5 pattern op(_,_)
operator checkpw alias CHECKPW pattern op(_,_)
operator crypt alias Crypt pattern op(_)
operator blowfish_encode alias BLOWFISH_ENCODE pattern op(_,_)
operator blowfish_decode alias BLOWFISH_DECODE pattern op(_,_)
operator charToText alias CHARTOTEXT pattern op(_)

operator checkOperatorTypeMap alias CHECKOPERATORTYPEMAP pattern op( _, _ )
operator checkOperatorTypeMap2 alias CHECKOPERATORTYPEMAP2 pattern op( _, _ )
operator strequal alias STREQUAL pattern op( _, _, _ )
operator svg2text alias SVG2TEXT pattern op( _ )
operator text2svg alias TEXT2SVG pattern op( _ )
operator tokenize alias TOKENIZE pattern op( _, _ )
operator attr2text alias ATTR22TEXT pattern op(_)
operator trimAll alias trimAll pattern _ op
operator str2real alias STRTOREAL pattern  op(_)
operator str2int alias STRTOINT pattern  op(_)
operator recode alias RECODE pattern _ op [_, _]
operator endsWith alias ENDSWITH pattern _ infixop _
operator startsWith alias STARTSWITH pattern _ infixop _

operator markText alias markText pattern _ op[_,_,_,_,_] 
operator bashModifier alias BASHMODIFIER pattern op(_)
operator getBashModifiers alias GETBASHMODIFIERS pattern op()

operator getQueryNL alias GETQUERYNL pattern op()
operator getOpTreeNL alias GETOPTREENL pattern op()
operator getOpName alias GETOPNAME pattern op()

operator regexmatches alias REGEXMATCHES pattern op(_,_)
operator startsReg alias STARTSREG pattern op(_,_)
operator findPattern alias FINDPATTERN pattern op(_,_)
operator createRegEx alias CREATEREGEX pattern op(_)
operator createRegEx2 alias CREATEREGEX2 pattern op(_)

operator tmcheck alias TMCHECK pattern _ op [_]
operator fileExtension alias fileEXTENSION pattern op(_)


