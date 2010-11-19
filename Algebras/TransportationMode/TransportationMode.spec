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
operator generate_wp3 alias GENERATE_WP3 pattern op(_,_)
operator zval alias ZVAL pattern op(_)
operator zcurve alias ZCURVE pattern op(_,_)
operator regvertex alias REGVERTEX pattern op(_)
operator triangulation_new  alias TRIANGULATION_NEW pattern op(_)
operator get_dg_edge alias GET_DG_EDGE pattern op(_,_)
operator smcdgte alias SMCDGTE pattern op(_,_)
operator getvnode alias GETVNODE pattern op(_,_,_,_,_,_)
operator getvgedge alias GETVGEDGE pattern op(_,_,_,_,_)
operator myinside alias MYINSIDE pattern _ infixop _
operator getadjnode_dg alias GETADJNODE_DG pattern op(_,_)
operator getadjnode_vg alias GETADJNODE_VG pattern op(_,_)
operator decomposetri alias DECOMPOSETRI pattern op(_)
operator createvgraph alias CREATEVGRAPH pattern op(_,_,_)
operator getcontour alias GETCONTOUR pattern op(_)
operator getpolygon alias GETPOLYGON pattern op(_,_)
operator getallpoints alias GETALLPOINTS pattern op(_)
operator rotationsweep alias ROTATIONSWEEP pattern op(_,_,_,_,_)
operator gethole alias GETHOLE pattern op(_)
operator getsections alias GETSECTIONS pattern op(_,_,_,_,_,_)
operator geninterestp1 alias GENINTERESTP1 pattern op(_,_,_,_,_,_,_)
operator geninterestp2 alias GENINTERESTP2 pattern op(_,_,_,_,_,_)
operator cellbox alias CELLBOX pattern op(_,_)
operator create_bus_route1 alias CREATE_BUS_ROUTE1 pattern op(_,_,_,_,_,_,_)
operator create_bus_route2 alias CREATE_BUS_ROUTE2 pattern op(_,_,_,_,_,_,_,_)
operator refine_bus_route alias REFINE_BUS_ROUTE pattern op(_,_,_,_,_,_,_,_)
operator create_bus_route3 alias CREATE_BUS_ROUTE3 pattern op(_,_,_,_,_)
operator create_bus_route4 alias CREATE_BUS_ROUTE3 pattern op(_,_,_,_,_,_,_,_)
operator create_bus_stop1 alias CREATE_BUS_STOP1 pattern op(_,_,_,_,_,_)
operator create_bus_stop2 alias CREATE_BUS_STOP2 pattern op(_,_,_,_,_)
operator create_bus_stop3 alias CREATE_BUS_STOP3 pattern op(_,_,_,_,_,_,_,_)
operator create_bus_stop4 alias CREATE_BUS_STOP5 pattern op(_,_,_,_,_,_,_,_)
operator create_bus_stop5 alias CREATE_BUS_STOP5 pattern op(_,_,_,_,_,_,_,_)
operator maptoint alias MAPTOINT pattern op(_)
operator maptoreal alias MAPTOREAL pattern op(_)
operator get_route_density1 alias GET_ROUTE_DENSITY1 pattern op(_,_,_,_,_,_,_,_,_,_)
operator set_ts_nightbus alias SET_TS_NIGHTBUS pattern op(_,_,_,_,_,_)
operator set_ts_daybus alias SET_TS_DAYBUS pattern op(_,_,_,_,_,_)
operator set_br_speed alias SET_BR_SPEED pattern op(_,_,_,_,_,_,_,_)
operator create_bus_segment_speed alias CREATE_BUS_SEGMENT_SPEED pattern op(_,_,_,_,_,_,_,_,_,_,_)
operator create_night_bus_mo alias CREATE_NIGHT_BUS_MO pattern op(_,_,_)
operator create_daytime_bus_mo alias CREATE_DAYTIME_BUS_MO pattern op(_,_,_)
operator create_time_table alias CREATE_TIME_TABLE pattern op(_,_,_)
