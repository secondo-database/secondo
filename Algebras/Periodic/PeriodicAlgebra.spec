#This file is part of SECONDO.

# Copyright (C) 2004-2007,
# University in Hagen,
# Faculty of Mathematics and  Computer Science,
# Database Systems for New Applications.

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

operator = alias EQUALS pattern _ infixop _
operator contains alias CONTAINS pattern _ infixop _
operator atinstant alias ATINSTANT pattern _ infixop _ 
operator breakpoints alias BREAKPOINTS pattern op(_,_)
operator createpmpoint alias CREATEPMPOINT pattern op(_)
operator distance alias DISTANCE pattern op(_ , _ )
operator end alias END pattern op(_)
operator expand alias EXPAND pattern op(_)
operator final alias FINAL pattern op(_)
operator initial alias INITIAL pattern op(_)
operator intersection alias INTERSECTION pattern op(_,_)
operator intersects alias INTERSECTS pattern _ infixop _
operator length alias LENGTH pattern op(_)
operator numberOfCNodes alias NUMBEROFCNODES pattern op(_)
operator numberOfFlatUnits alias NUMBEROFFLATUNITS pattern op(_)
operator numberOfNodes alias NUMBEROFNODES pattern op(_)
operator numberOfPNodes alias NUMBEROFPNODES pattern op(_)
operator numberOfUnits alias NUMBEROFUNITS pattern op(_)
operator ptranslate alias PTRANSLATE pattern _ op [_]
operator start alias START pattern op(_)
operator toprel alias TOPREL pattern op(_,_)
operator trajectory alias TRAJECTORY pattern op(_)
operator union alias UNION pattern _ infixop _
