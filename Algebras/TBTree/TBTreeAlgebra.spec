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

operator createtbtree alias CREATETBTREE pattern _ op [ _, _ ]
operator bulkloadtbtree alias BULKLOADTBTREE pattern _ op [ _, _ , _]
operator tbentries alias TBENTRIES pattern op(_)
operator tbnodes alias TBNODES pattern op(_)
operator tbleafnodes alias TBLEAFNODES pattern op(_)
operator tblevel alias TBLEVEL pattern op(_)
operator getnodes alias GETNODES pattern op(_)
operator getFileInfo alias GETFILEINFO pattern op(_)
operator getentries alias GETENTRIES pattern op(_)
operator windowintersectsS alias WINDOWINTERSECTSS pattern _ op [_]
operator getBox alias GETBOX pattern op(_)
operator windowintersects alias WINDOWINTERSECTS pattern _ _ op[_]
operator bulkloadtbtree alias BULKLOADTBTREE pattern _ op[_, _, _]
operator getentries alias GETENTRIES pattern op(_)

