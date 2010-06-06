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

operator checksline alias CHECKSLINE pattern op(_,_)
operator modifyboundary alias MODIFYBOUNDARY pattern op(_,_)
operator segment2region alias SEGMENT2REGION pattern op(_,_)
operator paveregion alias PAVEREGION pattern op(_,_,_,_,_,_,_)
operator junregion alias JUNREGION pattern op(_,_,_,_,_,_,_)
operator decomposeregion alias DECOMPOSEREGION pattern op(_)
operator fillpavement alias FILLPAVEMENT pattern op(_,_,_,_,_)
operator getpavenode1 alias GETPAVENODE1 pattern op(_,_,_,_,_)
operator getpaveedge1 alias GETPAVEEDGE1 pattern op(_,_,_,_,_,_)
operator getpavenode2 alias GETPAVENODE2 pattern op(_,_,_,_)
operator getpaveedge2 alias GETPAVEEDGE2 pattern op(_,_,_,_,_,_)
operator triangulation  alias TRIANGULATION pattern op(_)
operator convex alias CONVEX pattern op(_)
operator geospath alias GEOSPATH pattern op(_,_,_)
operator createdualgraph alias CREATEDUALGRAPH pattern op(_,_,_)
operator nodedualgraph alias NODEDUALGRAPH pattern op(_)
operator walk_sp alias WALK_SP pattern op(_,_,_,_,_,_,_)
operator generate_wp1 alias GENERATE_WP1 pattern op(_,_)
operator generate_wp2 alias GENERATE_WP2 pattern op(_,_)
operator zval alias ZVAL pattern op(_)
operator zcurve alias ZCURVE pattern op(_,_)
operator regvertex alias REGVERTEX pattern op(_)
operator triangulation_new  alias TRIANGULATION_NEW pattern op(_)
operator get_dg_edge alias GET_DG_EDGE pattern op(_,_)
operator getvnode alias GETVNODE pattern op(_,_,_,_,_,_)
operator getvgedge alias GETVGEDGE pattern op(_,_,_,_,_,_)
operator myinside alias MYINSIDE pattern _ infixop _
operator getadjnode_dg alias GETADJNODE_DG pattern op(_,_)
operator getadjnode_vg alias GETADJNODE_VG pattern op(_,_)
operator decomposetri alias DECOMPOSETRI pattern op(_)
operator createvgraph alias CREATEVGRAPH pattern op(_,_,_)