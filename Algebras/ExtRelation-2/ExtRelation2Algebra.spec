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


operator sort alias SORT pattern _ op
operator sortby alias SORTBY pattern _ op [list]
operator sortmergejoin alias SORTMERGEJOIN pattern _ _ op [_, _] 
operator gracehashjoin alias GRACEHASHJOIN pattern _ _ op [_, _, _]
operator hybridhashjoin alias HYBRIDHASHJOIN pattern _ _ op [_, _, _]

operator sortmergejoinParam alias SORTMERGEJOINPARAM pattern _ _ op [_, _, _] 
operator gracehashjoinParam alias GRACEHASHJOINPARAM pattern _ _ op [_, _, _, _, _, _]
operator hybridhashjoinParam alias HYBRIDHASHJOINPARAM pattern _ _ op [_, _, _, _, _, _]
operator sortParam alias SORTPARAM pattern _ op[_, _, _]
operator sortbyParam alias SORTBYPARAM pattern _ op[list; _, _, _]
operator heapstl alias HEAPSTL pattern _ op
operator heapstd alias HEAPSTD pattern _ op
operator heapbup alias HEAPBUP pattern _ op
operator heapbup2 alias HEAPBUP2 pattern _ op
operator heapmdr alias HEAPMDR pattern _ op
operator tuplecomp alias TUPLECOMP pattern _ op
operator tuplefile alias TUPLEFILE pattern _ op[_]
operator tuplebuffer alias TUPLEBUFFER pattern _ op[_]
operator tuplebuffer2 alias TUPLEBUFFER2 pattern _ op[_]

operator itHashJoin alias ITHASHJOIN pattern _ _ op [_,_]

