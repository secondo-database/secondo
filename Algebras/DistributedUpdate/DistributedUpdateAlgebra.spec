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

operator drelinsert alias DRELINSERT pattern _ _ op [_]
operator drelinserttuple alias DRELINSERTTUPLE pattern _ op [list; _]
operator dreldelete alias DRELDELETE pattern _ _ op [_]
operator drelupdate alias DRELUPDATE pattern _ _ op [funlist; _] implicit parameters elem1 types DRELFUNARG1

operator dreldeletebyid alias DRELDELETEBYID pattern _ _ op [_, _]
operator drelupdatebyid alias DRELUPDATEBYID pattern _ _ op [_; funlist; _] implicit parameters elem1, elem2 types SUBSUBTYPE1, DRELFUNARG2

operator drelinsertrtree alias DRELINSERTRTREE pattern _ _ op [_, _]
operator dreldeletertree alias DRELDELETERTREE pattern _ _ op [_, _]
operator drelupdatertree alias DRELUPDATERTREE pattern _ _ op [_, _]

operator drelinsertbtree alias DRELINSERTBTREE pattern _ _ op [_, _]
operator dreldeletebtree alias DRELDELETEBTREE pattern _ _ op [_, _]
operator drelupdatebtree alias DRELUPDATEBTREE pattern _ _ op [_, _]

operator dreladdid alias DRELADDID pattern _ op
operator drelfilteraddid alias DRELFILTERADDID pattern _ op [fun] implicit parameters elem1 types DRELFUNARG1

operator drelfilterdelete alias DRELFILTERDELETE pattern _ op [fun] implicit parameters elem1 types DRELFUNARG1
operator drelfilterupdate alias DRELFILTERUPDATE pattern _ op [fun; funlist] implicit parameters elem1 types DRELFUNARG1

operator drelexactmatchS alias DRELEXACTMATCHS pattern _ op [_]
operator drelrangeS alias DRELRANGES pattern _ op [_, _]
operator drelwindowintersectsS alias DRELWINDOWINTERSECTSS pattern _ op [_]

operator drelspatialjoin alias DRELSPATIALJOIN pattern _ _ op [_, _]
