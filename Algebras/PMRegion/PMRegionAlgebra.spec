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
operator perimeter alias PERIMETER pattern op ( _ )
operator area alias AREA pattern op ( _ )
operator traversedarea alias TRAVERSEDAREA pattern op ( _ )
operator translate alias TRANSLATE pattern _ op [list]
operator coverduration alias COVERDURATION pattern op ( _ )
operator inside alias INSIDE pattern _ infixop _
operator union alias UNION pattern _ infixop _
operator minus alias MINUS pattern _ infixop _
operator intersection alias INTERSECTION pattern op ( _ , _ )
operator intersects alias INTERSECTS pattern _ infixop _
operator pmreg2mreg alias PMREG2MREG pattern op ( _ )
operator mreg2pmreg alias MREG2PMREG pattern op ( _ )

