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

operator importSTL alias IMPORTSTL pattern op ( _ )
operator exportSTL alias EXPORTSTL pattern _ op [ _ , _ , _ ]

operator translate alias TRANSLATE pattern  _ op [list]
operator rotate alias ROTATE pattern  _ op [ _,  _, _ ]
operator scale alias SCALE pattern _ op [ _, _ ]
operator scaleDir alias SCALEDIR pattern _ op [ _, _ ]
operator mirror alias MIRROR pattern _ op [ _ ]
operator size alias SIZE pattern  op ( _ )
operator bbox3d alias BBOX3D pattern  op ( _ )
operator test alias TEST pattern op ( _ )
operator createCube alias CREATECUBE pattern op(_, _ )
operator createCylinder alias CREATECYLINDER pattern op(_, _ , _, _)
operator createCone alias CREATECONE pattern op(_, _ , _, _)
operator createSphere alias CREATESPHERE pattern op(_, _ , _ )

operator union alias UNION pattern _ infixop _
operator intersection alias INTERSECTION pattern op ( _, _ )
operator minus alias MINUS pattern _ infixop _
operator components alias COMPONENTS pattern op(_)

operator region2surface alias REGION2SURFAE pattern op( _ )
operator region2volume alias REGION2VOLUME pattern op( _, _ )
operator mregion2volume alias MREGION2VOLUME pattern op( _, _ )
