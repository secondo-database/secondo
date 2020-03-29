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

operator dmapS alias DMAPS pattern  _ op[_,fun] implicit parameter dmapelem type ARRAYORTASKSFUNARG1
operator dmapS2 alias DMAPS2 pattern _ _ op[_,fun] implicit parameters elem1, elem2 types ARRAYORTASKSFUNARG1, ARRAYORTASKSFUNARG2
operator dmapS3 alias DMAPS3 pattern _ _ _ op[_,fun] implicit parameters elem1, elem2, elem3 types ARRAYORTASKSFUNARG1, ARRAYORTASKSFUNARG2, ARRAYORTASKSFUNARG3
operator dmapS4 alias DMAPS4 pattern _ _ _ _ op[_,fun] implicit parameters elem1, elem2, elem3, elem4 types ARRAYORTASKSFUNARG1, ARRAYORTASKSFUNARG2, ARRAYORTASKSFUNARG3, ARRAYORTASKSFUNARG4
operator dmapS5 alias DMAPS5 pattern _ _ _ _ _ op[_,fun] implicit parameters elem1, elem2, elem3, elem4, elem5 types ARRAYORTASKSFUNARG1, ARRAYORTASKSFUNARG2, ARRAYORTASKSFUNARG3, ARRAYORTASKSFUNARG4, ARRAYORTASKSFUNARG5
operator dproductS alias DPRODUCTS pattern _ _ op[_,fun] implicit parameters elem1, elem2 types ARRAYORTASKSFUNARG1, ARRAYORTASKSFUNFSARG2
operator partitionFS alias PARTITIONFS pattern  _ op[_,fun,fun,_] implicit parameters arg1, arg2 types ARRAYORTASKSFUNARGPARTITIONF, ARRAYORTASKSFUNARGPARTITIONF
operator collectS alias COLLECTS pattern  _ op[_]
operator schedule alias SCHEDULE pattern  _ op[_]
operator tasks2tuples alias TASKS2TUPLES pattern  _ op
