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

operator epsilon alias EPSILON pattern op ( _ )
operator trajectory alias TRAJECTORY pattern op ( _ )
operator deftime alias DEFTIME pattern op ( _ )
operator present alias PRESENT pattern _ infixop _
operator atinstant alias ATINSTANT pattern _ infixop _
operator atperiods alias ATPERIODS pattern _ infixop _
operator units alias UNITS pattern op ( _ )
operator d_passes alias D_PASSES pattern _ infixop _
operator p_passes alias P_PASSES pattern _ infixop _
operator d_at alias D_AT pattern _ infixop _
operator p_at alias P_AT pattern _ infixop _
operator touncertain alias TOUNCERTAIN pattern op( _, _ )

# During the implementation of the datatypes 'cupoint' and 'cmpoint' a re-
# implementation of the uncertain-template became necessary.  So the following
# two operators won't work anymore.

#operator val alias VAL pattern op( _ )
#operator tocpoint alias TOCPOINT pattern op( _, _ )





