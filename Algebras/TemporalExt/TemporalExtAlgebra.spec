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

operator atinstant alias ATINSTANT pattern _ infixop _
operator atperiods alias ATPERIODS pattern _ infixop _
operator present alias PRESENT pattern _ infixop _
operator passes alias PASSES pattern _ infixop _
operator initial alias INITIAL pattern op ( _ )
operator final alias FINAL pattern op ( _ )
operator at alias AT pattern _ infixop _
operator deftime alias DEFTIME pattern op ( _ )
operator inst alias INST pattern op ( _ )
operator val alias VAL pattern op ( _ )
operator derivative_new alias DERIVATIVE_NEW pattern op ( _ )
operator derivable_new alias DERIVABLE_NEW pattern op ( _ )
operator speed_new alias SPEED_NEW pattern op ( _ )
operator rangevalues alias RANGEVALUES pattern op ( _ )
operator sometimes alias SOMETIMES pattern op ( _ )
operator always alias ALWAYS pattern op ( _ )
operator never alias NEVER pattern op ( _ )
operator setunitoftime alias SETUNITOFTIME pattern op ( _ )
operator setunitofdistance alias SETUNITOFDISTANCE pattern op ( _ )
operator velocity_new alias VELOCITY_NEW pattern op ( _ )
operator direction alias DIRECTION pattern op ( _, _, _ )
operator heading alias HEADING pattern op ( _, _, _ )
operator locations alias LOCATIONS pattern op ( _ )
operator atmin alias ATMIN pattern op ( _ )
operator atmax alias ATMAX pattern op ( _ )
operator concatS alias CONCATS pattern  _ op
operator concatS2 alias CONCATS2 pattern  _ op [ _ ]
operator everNearerThan alias EVERNEARERTHAN pattern op( _, _, _ )
operator inside alias INSIDE pattern _ infixop _
operator splitAtGaps alias SPLITATGAPS pattern op ( _ , _ )
operator splitAtSpeed alias SPLITATSPEED pattern op ( _ , _ , _ )
operator splitAtLength alias SPLITATLENGTH pattern op ( _ , _ , _ )
operator cyclicbulkload alias CYCLICBULKLOAD pattern _  op [ _, _, _, _ ]

