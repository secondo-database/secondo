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

operator randommbool alias RandomMBool pattern op(_)
operator passmbool alias PassMBool pattern op(_)
operator stpqexpr1 alias STPQexpr1 pattern op(_,_,_)
operator runstpqexpr1 alias runSTPQexpr1 pattern op(_)
operator ndefunit alias NDefUnit pattern op(_, _)
operator randomshiftdelay alias RANDOMSHIFTDELAY pattern op(_,_,_,_)
operator mytest alias MYTEST pattern op()