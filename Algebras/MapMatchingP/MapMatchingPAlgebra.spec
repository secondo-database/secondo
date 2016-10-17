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


# postfix operator having a parameter in square brackets
operator countNumberMMP alias COUNTNUMBERMMP pattern _ op[_]
# operator testMMP alias TESTMMP pattern _ op
operator coordMMP alias TESTMMP pattern _ op
operator mmprelcount2 alias MMPRELCOUNT2 pattern _ op
operator nodeRelToRegion alias NODERELTOREGION pattern _ op
operator pointToRegion alias POINTTOREGION pattern _ op[_]
operator nodesToRegionNodes alias NODESTOREGIONNODES pattern _ op[ _ , _ ]
operator edgesToRegionNodes alias EDGESTOREGIONNODES pattern _ op[ _ ]
operator sLineToRegion alias SLINETOREGION pattern _ op[_]
operator sLineRelToRegion alias SLINERELTOREGION pattern _ op[_]
operator createEdgesForRegionNodes alias CREATEEDGESFORREGIONNODES pattern _ op[ _ , _ , _ , _ ]
operator mapMatchWalks alias MAPMATCHWALKS pattern op( _ , _ , _ , _ , _ )
operator removeOverlapping alias REMOVEOVERLAPPING pattern op( _ , _ , _ , _ )




