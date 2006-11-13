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
operator makemvalue alias MAKEMVALUE pattern _ op [ _ ]
operator circle alias CIRCLE pattern op ( _ )
operator makepoint alias MAKEPOINT pattern op ( _ )
operator velocity alias VELOCITY pattern op ( _ )
operator derivable alias DERIVABLE pattern op ( _ )
operator derivative alias DERIVATIVE pattern op ( _ )
operator atperiods alias ATPERIODS pattern _ infixop _
operator sfeed alias SFEED pattern op ( _ )
operator suse alias SUSE pattern _ op [ _ ] implicit parameter streamelem type STREAMELEM
operator suse2 alias SUSE2 pattern _ _ op [ _ ] implicit parameters streamelem1, streamelem2 types STREAMELEM, STREAMELEM2
operator saggregate alias SAGGREGATE pattern _ op [ _ ; _ ] implicit parameter estreamelem type STREAMELEM
operator at alias AT pattern _ infixop _ 
operator atmax alias ATMAX pattern op ( _ )
operator atmin alias ATMIN pattern op ( _ )
operator intersection alias INTERSECTION pattern op ( _ , _ )
operator distance alias DISTANCE pattern op ( _ , _ )
operator transformstream alias TRANSFORMSTREAM pattern _ op
operator printstream alias PRINTSTREAM pattern _ op
operator uint2ureal alias UINT2UREAL pattern op ( _ )

# Operator signatures already defined elsewhere:

#operator filter alias FILTER pattern _ op [ fun ] implicit parameter streamelem type STREAMELEM
#operator count alias COUNT pattern _ op
#operator no_components alias NO_COMPONENTS pattern op ( _ )
#operator isempty alias ISEMPTY pattern op ( _ )
#operator not alias NOT pattern op ( _ )
#operator and alias AND pattern _ infixop _ 
#operator or alias OR pattern _ infixop _ 
#operator = alias EQ pattern _ infixop _ 
#operator # alias NE pattern _ infixop _ 
#operator < alias LT pattern _ infixop _ 
#operator > alias GT pattern _ infixop _ 
#operator <= alias LE pattern _ infixop _ 
#operator >= alias GE pattern _ infixop _ 

