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

operator touches alias TOUCHES pattern _ infixop _
operator attached alias ATTACHED pattern _ infixop _
operator overlaps alias OVERLAPS pattern _ infixop _
operator onborder alias ONBORDER pattern _ infixop _
operator ininterior alias ININTERIOR pattern _ infixop _
operator crossings alias CROSSINGS pattern op ( _, _ )
operator single alias SINGLE pattern  op ( _ )
operator distance alias DISTANCE pattern  op ( _, _ )
operator direction alias DIRECTION pattern  op ( _, _ )
operator no_components alias NO_COMPONENTS pattern  op ( _ )
operator no_segments alias NO_SEGMENTS pattern  op ( _ )
operator size alias SIZE pattern  op ( _ )
operator touchpoints alias TOUCHPOINTS pattern  op ( _, _ )
operator commonborder alias COMMONBORDER pattern  op ( _, _ )
operator bbox alias BBOX pattern  op ( _ )
operator insidepathlength alias INSIDEPATHLENGTH pattern  _ infixop _
operator insidescanned alias INSIDESCANNED pattern  _ infixop _
operator insideold alias INSIDEOLD pattern  _ infixop _
operator translate alias TRANSLATE pattern  op ( _, _ , _)

