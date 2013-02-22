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

operator atlocation alias ATLOCATION pattern _ infixop _
operator atinstant alias ATINSTANT pattern _ infixop _
operator inst alias INST pattern op(_)
operator val alias VAL pattern op(_)
operator atperiods alias ATPERIODS pattern _ infixop _
operator atrange alias ATRANGE pattern _ op [_,_,_]
operator deftime alias DEFTIME pattern op(_)
operator bbox alias BBOX pattern op(_,_)
operator minimum alias MINIMUM pattern op(_)
operator maximum alias MAXIMUM pattern op(_)
operator map alias MAP pattern _ op [fun ]
         implicit parameter element type CELL1
operator map2 alias MAP2 pattern _ _ op [fun]
         implicit parameters elem1, elem2 types CELL1, CELL2
operator fromregion alias FROMREGION pattern op(_,_)
operator toregion alias TOREGION pattern _ op
operator s2ms alias STOMS pattern  op(_,_,_,_)
operator compose  alias COMPOSE pattern _ infixop _
operator matchgrid alias MATCHGRID pattern _ op [_,fun,_]
         implicit parameter element type CELLS
operator getgrid alias GETGRID pattern op(_)
operator importHgt alias IMPORTHGT pattern _ op
operator importEsriGrid alias IMPORTESRIGRID pattern op(_)
operator importEsriRaster alias IMPORTESRIRASTER pattern _ op
