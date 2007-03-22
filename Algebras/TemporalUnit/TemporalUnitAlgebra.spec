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


operator speed alias SPEED pattern op ( _ )
operator queryrect2d alias QUERYRECT2D pattern op ( _ )
operator point2d alias POINT2D pattern op ( _ )
operator get_duration alias GET_DURATION pattern op ( _ )
operator makemvalue alias MAKEMVALUE pattern _ op [ _ ]
operator the_mvalue alias THE_MVALUE pattern _ op
operator circle alias CIRCLE pattern op ( _ , _ , _ )
operator makepoint alias MAKEPOINT pattern op ( _ )
operator velocity alias VELOCITY pattern op ( _ )
operator derivable alias DERIVABLE pattern op ( _ )
operator derivative alias DERIVATIVE pattern op ( _ )
operator atperiods alias ATPERIODS pattern _ infixop _
operator at alias AT pattern _ infixop _
operator atmax alias ATMAX pattern op ( _ )
operator atmin alias ATMIN pattern op ( _ )
operator intersection alias INTERSECTION pattern op ( _ , _ )
operator inside alias INSIDE pattern _ infixop _
operator distance alias DISTANCE pattern op ( _ , _ )
operator abs alias ABS pattern op ( _ )
operator printstream alias PRINTSTREAM pattern _ op
operator uint2ureal alias UINT2UREAL pattern op ( _ )
operator the_unit alias THE_UNIT pattern op( _ , _ , _ , _ , _ , _ , _ )
operator the_ivalue alias THE_IVALUE pattern op( _ , _ )
operator == alias VEQ pattern _ infixop _
operator ## alias VNE pattern _ infixop _
operator << alias VLT pattern _ infixop _
operator >> alias VGT pattern _ infixop _
operator <<== alias VLE pattern _ infixop _
operator >>== alias VGE pattern _ infixop _

# Operator signatures already defined elsewhere:

#operator no_components alias NO_COMPONENTS pattern op ( _ )
#operator isempty alias ISEMPTY pattern op ( _ )
#operator not alias NOT pattern op ( _ )
#operator and alias AND pattern _ infixop _
#operator or alias OR pattern _ infixop _
#operator sometimes alias SOMETIMES pattern op ( _ )
#operator never alias NEVER pattern op ( _ )
#operator always alias ALWAYS pattern op ( _ )
#operator = alias EQ pattern _ infixop _
#operator # alias NE pattern _ infixop _
#operator < alias LT pattern _ infixop _
#operator > alias GT pattern _ infixop _
#operator <= alias LE pattern _ infixop _
#operator >= alias GE pattern _ infixop _

