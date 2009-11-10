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

operator thebusnetwork alias THEBUSNETWORK pattern op(_)
operator busnode alias BUSNODE pattern op(_)

operator busedge alias BUSEDGE pattern op(_)

operator busmove alias BUSMOVE pattern op(_)
operator find_path_t_1 alias FIND_PATH_T_1 pattern op(_,_,_,_)
operator find_path_t_2 alias FIND_PATH_T_2 pattern op(_,_,_,_)
operator find_path_t_3 alias FIND_PATH_T_3 pattern _ _ op[_,_,_,_,_]
operator find_path_t_4 alias FIND_PATH_T_4 pattern _ op[_,_,_,_]
operator find_path_t_5 alias FIND_PATH_T_5 pattern _ op[_,_,_,_]
operator find_path_t_6 alias FIND_PATH_T_6 pattern _ op[_,_,_,_]

operator find_path_bus_tree1 alias FIND_PATH_BUS_TREE1 pattern _ op[_,_,_,_]