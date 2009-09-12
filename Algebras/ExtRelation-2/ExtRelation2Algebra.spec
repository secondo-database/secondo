#This file is part of SECONDO. 

#Copyright (C) 2009, University in Hagen, Department of Computer Science, 
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


operator sort2 alias SORT2 pattern _ op
operator sortby2 alias SORTBY2 pattern _ op [list]
operator sortmergejoin2 alias SORTMERGEJOIN2 pattern _ _ op [_, _] 
operator hybridhashjoin alias HYBRIDHASHJOIN pattern _ _ op [_, _, _]

operator hybridhashjoinP alias HYBRIDHASHJOINP pattern _ _ op [_, _, _, _, _, _]
operator sort2with alias SORT2WITH pattern _ op[_, _]
operator sortby2with alias SORTBY2WITH pattern _ op[list; _, _]
operator heapstl alias HEAPSTL pattern _ op
operator heapstd alias HEAPSTD pattern _ op
operator heapbup alias HEAPBUP pattern _ op
operator heapbup2 alias HEAPBUP2 pattern _ op
operator heapmdr alias HEAPMDR pattern _ op
operator tuplecomp alias TUPLECOMP pattern _ op
operator tuplefile alias TUPLEFILE pattern _ op
operator tuplebuffer alias TUPLEBUFFER pattern _ op[_]

