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

operator doubleexport alias DOUBLEEXPORT pattern _ _ op[_,_]
operator parahashjoin alias PARAHASHJOIN pattern _ _ _ op
operator parajoin alias PARAJOIN pattern _ _ _ op[fun] implicit parameters stream1, stream2 types TUPSTREAM2, TUPSTREAM3 
operator add0Tuple alias ADD0TUPLE pattern _ op
operator fconsume alias FCONSUME pattern _ op [list; list; list]
operator ffeed alias FFEED pattern _ op [list; list; list]
operator hadoopjoin alias HADOOPJOIN pattern _ _ op [_, _, _, _; list; fun] implicit parameters stream1, stream2 types ANY, ANY2
operator fdistribute alias FDISTRIBUTE pattern _ op [list; list; list; list]
operator spread alias SPREAD pattern _op [list;list;list]
operator collect alias COLLECT pattern _op[list]