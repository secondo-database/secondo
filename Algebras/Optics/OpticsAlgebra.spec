
# file Standard-C++/StandardAlgebra.spec 

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

operator opticsR alias OPTICSR pattern _ op [list]
operator opticsM alias OPTICSM pattern _ op [list]
operator opticsF alias OPTICSF pattern _ op [_,_,_,fun] implicit parameters argument1, argument2 types AGGRTYPE, AGGRTYPE
operator opticsTF alias OPTICSTF pattern _ op [_,_,fun] implicit parameters argument1, argument2 types STREAMELEM,STREAMELEM 
operator extractDbScan alias EXTRACTDBSCAN pattern _ op[_]
