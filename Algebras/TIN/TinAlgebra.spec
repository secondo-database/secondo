#This file is part of SECONDO.

#Copyright (C) 2004, University in Hagen, Department of Computer Science,
#Database Systems for New Applications.

#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published bycd

#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

operator atlocation alias ATLOCATION pattern _ op [_,_]
operator tin2stlfile alias TIN2STLFILE pattern _ op [_]
operator tinmin alias TINMIN pattern _ op
operator tinmax alias TINMAX pattern _ op
operator createTin alias CREATETIN pattern _ op [_]
operator unaryOp alias UNARYOP pattern _ op [ _ ]
operator raster2tin alias RASTER2TIN pattern _ op [_]
operator tin2tuplestream alias TIN2TUPLESTREAM pattern _ op
operator tin2tinattribute alias TIN2TINATTRIBUTE pattern _ op
operator tinattribute2tin alias TINATTRIBUTE2TIN pattern _ op



