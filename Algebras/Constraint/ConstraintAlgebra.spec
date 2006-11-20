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

operator cunion alias CUNION pattern _ infixop _
operator coverlap alias COVERLAP pattern _ infixop _
operator cjoin alias CJOIN pattern _ infixop _
operator cintersection alias CINTERSECTION pattern _ infixop _
operator cprojection alias CPROJECTION pattern  op ( _, _ )
operator cselection alias CSELECTION pattern  op ( _, _, _, _, _ )
operator csatisfy alias CSATISFY pattern  op ( _ )
operator no_tuples alias NO_TUPLES pattern  op ( _ )
operator no_constraints alias NO_TUPLES pattern  op ( _ )
operator point2constraint alias POINT2CONSTRAINT pattern  op ( _ )
operator points2constraint alias POINTS2CONSTRAINT pattern  op ( _ )
operator line2constraint alias LINE2CONSTRAINT pattern  op ( _ )
operator region2constraint alias REGION2CONSTRAINT pattern  op ( _ , _ )
operator constraint2point alias CONSTRAINT2POINT pattern  op ( _ )
operator constraint2points alias CONSTRAINT2POINTS pattern  op ( _ )
operator constraint2line alias CONSTRAINT2LINE pattern  op ( _ )
operator constraint2region alias CONSTRAINT2REGION pattern  _ op
operator triangulate alias TRIANGULATE pattern _ op


