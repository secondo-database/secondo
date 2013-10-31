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

operator crossings alias CROSSINGS pattern op ( _, _ )

operator intersection alias INTERSECTION pattern op ( _, _ )

operator intersects alias INTERSECTS pattern _ infixop _

operator intersects2 alias INTERSECTS pattern _ infixop _

operator overlaps2 alias OVERLAPS pattern _ infixop _

operator lineToLine2 alias LINETOLINE2 pattern op( _ )

operator minus alias MINUS pattern _ infixop _

operator union alias UNION pattern _ infixop _

operator inside2 alias INSIDE2 pattern _ infixop _

operator crossingsWithScaling alias CROSSINGSWITHSCALING pattern op ( _, _ )

operator intersectionWithScaling alias INTERSECTIONWITHSCALING pattern op ( _, _ )

operator intersectsWithScaling alias INTERSECTSWITHSCALING pattern _ infixop _

operator overlapsWithScaling alias OVERLAPSWITHSCALING pattern _ infixop _

operator minusWithScaling alias MINUSWITHSCALING pattern _ infixop _

operator unionWithScaling alias UNIONWITHSCALING pattern _ infixop _

operator insideWithScaling alias INSIDEWITHSCALING pattern _ infixop _

operator coarse alias COARSE pattern op( _ )

operator coarse2 alias COARSE2 pattern op( _ )

operator testIntersection alias TESTINTERSECTION pattern op ( _, _ )

operator testIntersects alias TESTINTERSECTS pattern _ infixop _

operator testMinus alias TESTMINUS pattern _ infixop _

operator testUnion alias TESTUNION pattern _ infixop _

operator testIntersectionWithScaling alias TESTINTERSECTIONWITHSCALING pattern op ( _, _ )

operator testIntersectsWithScaling alias TESTINTERSECTSWITHSCALING pattern _ infixop _

operator testMinusWithScaling alias TESTMINUSWITHSCALING pattern _ infixop _

operator testUnionWithScaling alias TESTUNIONWITHSCALING pattern _ infixop _

operator bbox alias BBOX pattern op(_,_)
