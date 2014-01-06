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

operator spread alias SPREAD pattern _op [list; list; list]
operator spreadFiles alias SPREADFILES pattern op (_,_,_)
operator collect alias COLLECT pattern _op[list]
operator para alias PARA pattern op (_)
operator hadoopMap alias HADOOPMAP pattern _ op [list; fun] implicit parameter lobject1 type TPARA
operator hadoopReduce alias HADOOPREDUCE pattern _ op [list; fun] implicit parameter lobject1 type TPARA
operator hadoopReduce2 alias HADOOPREDUCE2 pattern _ _ op [list; fun] implicit parameters lobject1, lobject2 types TPARA, TPARA2 
operator createFList alias CREATEFLIST pattern _ op[_, _, _, _]
operator pffeed alias PFFEED pattern _ op [list;list]
operator pffeed2 alias PFFEED2 pattern _ op [list;list]
operator pffeed3 alias PFFEED3 pattern _ op [list;list]
operator hadoopMapAll alias HADOOPMAPALL pattern op (_)
