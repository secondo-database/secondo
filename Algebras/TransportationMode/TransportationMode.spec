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
operator getpavenode1 alias GETPAVENODE1 pattern op(_,_,_,_,_)

operator getpavenode2 alias GETPAVENODE2 pattern op(_,_,_,_)

operator triangulation  alias TRIANGULATION pattern op(_)
operator triangulation2  alias TRIANGULATION2 pattern op(_)
operator convex alias CONVEX pattern op(_)
operator geospath alias GEOSPATH pattern op(_,_,_)
operator createdualgraph alias CREATEDUALGRAPH pattern op(_,_,_)

operator walk_sp_old alias WALK_SP_OLD pattern op(_,_,_,_,_)
operator walk_sp alias WALK_SP pattern op(_,_,_,_)
operator walk_sp_debug alias WALK_SP_DEBUG pattern op(_,_,_,_)
operator test_walk_sp alias TEST_WALK_SP pattern op(_,_,_,_,_)
operator setpave_rid alias SETPAVE_RID pattern op(_,_,_)
operator pave_loc_togp alias PAVE_LOC_TOGP pattern op(_,_,_,_)
operator generate_wp1 alias GENERATE_WP1 pattern op(_,_)
operator generate_wp2 alias GENERATE_WP2 pattern op(_,_)
operator generate_wp3 alias GENERATE_WP3 pattern op(_,_)
operator zval alias ZVAL pattern op(_)
operator zcurve alias ZCURVE pattern op(_,_)
operator regvertex alias REGVERTEX pattern op(_)
operator triangulation_new  alias TRIANGULATION_NEW pattern op(_)
operator triangulation_ext  alias TRIANGULATION_EXT pattern op(_)

operator triangulation_new2  alias TRIANGULATION_NEW2 pattern op(_)
operator triangulation_ext2  alias TRIANGULATION_EXT2 pattern op(_)
operator smcdgte alias SMCDGTE pattern op(_,_)
operator get_dg_edge alias GET_DG_EDGE pattern op(_,_)

operator getvnode alias GETVNODE pattern op(_,_,_,_,_,_)
operator getvgedge alias GETVGEDGE pattern op(_,_,_,_,_)
operator myinside alias MYINSIDE pattern _ infixop _
operator at_point alias AT_POINT pattern op(_,_,_)

operator decomposetri alias DECOMPOSETRI pattern op(_)
operator createvgraph alias CREATEVGRAPH pattern op(_,_,_)
operator getcontour alias GETCONTOUR pattern op(_)
operator getpolygon alias GETPOLYGON pattern op(_,_)

operator getallpoints alias GETALLPOINTS pattern op(_)
operator rotationsweep alias ROTATIONSWEEP pattern op(_,_,_,_,_)
operator gethole alias GETHOLE pattern op(_)


operator thepavement alias THEPAVEMENT pattern op(_,_)
operator cellbox alias CELLBOX pattern op(_,_)
operator create_bus_route1 alias CREATE_BUS_ROUTE1 pattern op(_,_,_,_,_,_,_)
operator create_bus_route2 alias CREATE_BUS_ROUTE2 pattern op(_,_,_,_,_,_,_,_)
operator refine_bus_route alias REFINE_BUS_ROUTE pattern op(_,_,_,_,_,_,_,_)

operator create_bus_route3 alias CREATE_BUS_ROUTE3 pattern op(_,_,_,_,_)
operator create_bus_route4 alias CREATE_BUS_ROUTE3 pattern op(_,_,_,_,_,_,_,_)
operator create_bus_stop1 alias CREATE_BUS_STOP1 pattern op(_,_,_,_,_,_,_,_,_)
operator create_bus_stop2 alias CREATE_BUS_STOP2 pattern op(_,_,_,_,_)
operator create_bus_stop3 alias CREATE_BUS_STOP3 pattern op(_,_,_,_,_,_,_,_)
operator create_bus_stop4 alias CREATE_BUS_STOP5 pattern op(_,_,_,_,_,_,_,_)
operator create_bus_stop5 alias CREATE_BUS_STOP5 pattern op(_,_,_,_,_,_,_,_)
operator getbusstops alias GETBUSSTOPS pattern op(_,_,_)
operator getstopid alias GETSTOPID pattern op(_)
operator getbusroutes alias GETBUSROUTES pattern op(_,_,_)
operator brgeodata alias BRGEODATA pattern op(_)
operator bsgeodata alias BSGEODATA pattern op(_,_)
operator up_down alias UP_DOWN pattern op(_)
operator thebusnetwork alias THEBUSNETWORK pattern op(_,_,_,_)
operator bn_busstops alias BN_BUSSTOPS pattern op(_)
operator bn_busroutes alias BN_BUSROUTES pattern op(_)
operator brsegments alias BRSEGMENTS pattern op(_,_)
operator mapbstopave alias MAPBSTOPAVE pattern op(_,_,_,_)
operator bs_neighbors1 alias BS_NEIGHBORS1 pattern op(_,_,_,_,_,_)
operator bs_neighbors2 alias BS_NEIGHBORS2 pattern op(_)
operator bs_neighbors3 alias BS_NEIGHBORS3 pattern op(_,_,_)
operator createbgraph alias CREATEBGRAPH pattern op(_,_,_,_,_)
operator getadjnode_bg alias GETADJNODE_BG pattern op(_,_)
operator bnnavigation alias BNNAVIGATION pattern op(_,_,_,_,_)
operator test_bnnavigation alias TEST_BNNAVIGATION pattern op(_,_,_,_,_)
operator get_route_density1 alias GET_ROUTE_DENSITY1 pattern op(_,_,_,_,_,_,_,_,_,_)
operator set_ts_nightbus alias SET_TS_NIGHTBUS pattern op(_,_,_,_,_,_)
operator set_ts_daybus alias SET_TS_DAYBUS pattern op(_,_,_,_,_,_)
operator set_br_speed alias SET_BR_SPEED pattern op(_,_,_,_,_,_,_,_)
operator create_bus_segment_speed alias CREATE_BUS_SEGMENT_SPEED pattern op(_,_,_,_,_,_,_,_,_,_,_)
operator create_night_bus_mo alias CREATE_NIGHT_BUS_MO pattern op(_,_,_)
operator create_daytime_bus_mo alias CREATE_DAYTIME_BUS_MO pattern op(_,_,_)

operator create_time_table1 alias CREATE_TIME_TABLE1 pattern op(_,_,_,_,_)


operator create_time_table2 alias CREATE_TIME_TABLE2 pattern op(_,_,_)
operator refmo2genmo alias REFMO2GENMO pattern op(_,_,_)
operator themetronetwork alias THEMETRONETWORK pattern op(_,_,_,_)
operator ms_neighbors1 alias MS_NEIGHBORS1 pattern op(_)
operator ms_neighbors2 alias MS_NEIGHBORS2 pattern op(_,_,_,_,_)
operator createmgraph alias CREATEMGRAPH pattern op(_,_,_,_)
operator createmetroroute alias CREATEMETROROUTE pattern op(_,_)
operator createmetrostop alias CREATEMETROSTOP pattern op(_)
operator createmetromo alias CREATEMETROMO pattern op(_,_)
operator mapmstopave alias MAPMSTOPAVE pattern op(_,_,_)
operator mnnavigation alias MNNAVIGATION pattern op(_,_,_,_)

operator thefloor alias THEFLOOR pattern op(_,_)
operator getheight alias GETHEIGHT pattern op(_)
operator getregion alias GETREGION pattern op(_)
operator thedoor alias THEDOOR pattern op(_,_,_,_,_,_)

operator type_of_door alias TYPE_OF_DOOR pattern op(_)
operator loc_of_door alias LOC_OF_DOOR pattern op(_,_)
operator state_of_door alias STATE_OF_DOOR pattern op(_)
operator get_floor alias GET_FLOOR pattern op(_,_)
operator add_height_groom alias ADD_HEIGHT_GROOM pattern op(_,_)
operator translate_groom alias TRANSLATE_GROOM pattern _ op [list]
operator createdoor3d alias CREATEDOOR3D pattern op(_)
operator createdoorbox alias CREATEDOORBOX pattern op(_)
operator createdoor1 alias CREATEDOOR1 pattern op(_,_,_,_,_,_)
operator createdoor2 alias CREATEDOOR2 pattern op(_)
operator createadjdoor1 alias CREATEADJDOOR1 pattern op(_,_,_,_,_,_,_)
operator createadjdoor2 alias CREATEADJDOOR2 pattern op(_,_)
operator path_in_region alias PATH_IN_REGION pattern op(_,_,_)
operator size alias SIZE pattern op(_)
operator bbox3d alias BBOX_D pattern op(_)
operator thebuilding alias THEBUILDING pattern op(_,_,_,_)
operator theindoor alias THEINDOOR pattern op(_,_,_)
operator createigraph alias CREATEIGRAPH pattern op(_,_,_,_)

operator generate_ip1 alias GENERATE_IP1 pattern op(_,_,_)
operator generate_mo1 alias GENERATE_MO1 pattern op(_,_,_,_,_,_)
operator getindoorpath alias GETINDOORPATH pattern op(_,_)
operator indoornavigation alias INDOORNAVIGATION pattern op(_,_,_,_,_,_)


operator maxrect alias MAXRECT pattern op(_)
operator remove_dirty alias REMOVE_DIRTY pattern op(_,_,_)
operator getrect1 alias GETRECT1 pattern op(_,_,_)
operator path_to_building alias PATH_TO_BUILDING pattern op(_,_,_,_,_)
operator set_building_type alias SET_BUILDING_TYPE pattern op(_,_,_)
operator ref_id alias REF_ID pattern op(_)
operator tm_at alias TM_AT pattern op(_,_)
operator tm_at2 alias TM_AT2 pattern op(_,_,_)
operator tm_at3 alias TM_AT3 pattern op(_,_,_,_)
operator val alias VAL pattern op(_)
operator inst alias INST pattern op(_)
operator contains alias CONTAINS pattern _ infixop _ 
operator tmcontains alias TMCONTAINS pattern op(_,_,_,_)
operator tm_duration alias TM_DURATION pattern op(_,_)
operator initial alias INITIAL pattern op(_)
operator final alias FINAL pattern op(_)
operator tm_build_id alias TM_BUILD_ID pattern op(_,_)
operator bcontains alias BCONTAINS pattern _ infixop _
operator bcontains2 alias BCONTAINS2 pattern op(_,_,_)

operator tm_room_id alias TM_ROOM_ID pattern op(_,_)
operator tm_plus_id alias TM_PLUS_ID pattern op(_,_)
operator tm_passes alias TM_PASSES pattern op(_,_,_)
operator tm_distance alias TM_DISTANCE pattern op(_,_,_)
operator tm_genloc alias TM_GENLOC pattern op(_,_,_)
operator modeval alias MODEVAL pattern op(_)
operator genmoindex alias GENMOINDEX pattern op(_)


operator setref_id alias SETREF_ID pattern op(_)
operator deftime alias DEFTIME pattern op(_)
operator tm_translate alias TM_TRANSLATE pattern  _ infixop _
operator tm_translate2 alias TM_TRNASLATE2 pattern _ infixop _
operator no_components alias NO_COMPONENTS pattern op (_)
operator lowres alias LOWRES pattern op(_)
operator trajectory alias TRAJECTORY pattern op(_)
operator gentrajectory alias GENTRAJECTORY pattern op(_,_)
operator genrangevisible alias GENRANGEVISIBLE pattern op(_,_)
operator getmode alias GETMODE pattern op(_)
operator getref alias GETREF pattern op(_,_)
operator atinstant alias ATINSTANT pattern _ infixop _
operator atperiods alias ATPERIODS pattern _ infixop _
operator mapgenmo alias MAPGENMO pattern op(_,_)
operator tm_units alias TM_UNITS pattern op(_)
operator getloc alias GETLOC pattern op(_,_)
operator tm_traffic alias TM_TRAFFIC pattern op(_,_,_,_)

operator thespace alias THESPACE pattern op(_)
operator putinfra alias PUTINFRA pattern op(_,_)
operator putrel alias PUTREL pattern op(_,_)
operator getinfra alias GETINFRA pattern op(_,_)
operator addinfragraph alias ADDINFRAGRAPH pattern op(_,_)
operator genmo_tm_list alias GENMO_TM_LIST pattern op(_)

operator generate_genmo alias GENERATE_GENMO pattern op(_,_,_,_)

operator generate_bench_1 alias GENERATE_BENCH_1 pattern op(_,_,_,_,_,_)
operator generate_bench_2 alias GENERATE_BENCH_2 pattern op(_,_,_,_,_)
operator generate_bench_3 alias GENERATE_BENCH_3 pattern op(_,_,_,_,_)
operator generate_bench_4 alias GENERATE_BENCH_4 pattern op(_,_,_,_,_,_)
operator generate_bench_5 alias GENERATE_BENCH_5 pattern op(_,_,_,_,_)

operator generate_car alias GENERATE_CAR pattern op(_,_,_,_)

operator get_rg_nodes alias GET_RG_NODES pattern op(_)
operator get_rg_edges1 alias GET_RG_EDGES1 pattern op(_)
operator get_rg_edges2 alias GET_RG_EDGES2 pattern op(_,_)
operator get_p_edges3 alias GET_P_EDGE3 pattern op(_,_,_,_)
operator get_p_edges4 alias GET_P_EDGE4 pattern op(_,_)
operator theosmpave alias THEOSMPAVE pattern op(_,_,_)
operator createosmgraph alias CREATEOSMGRAPTH pattern op(_,_,_)
operator osmlocmap alias OSMLOCMAP pattern op(_,_)
operator osm_path alias OSM_PATH pattern op(_,_,_)
operator creatergraph alias CREATERGRAPH pattern op(_,_,_,_)
operator shortestpath_tm alias SHORTESTPATH_TM pattern op(_,_,_,_)


operator navigation1 alias NAVIGATION1 pattern op(_,_,_,_,_,_,_)

operator modifyline alias MODIFYLINE pattern op(_)
operator refinedata alias REFINEDATA pattern op(_)
operator filterdisjoint alias FILTERDISJOINT pattern op(_,_,_)
operator refinebr alias REFINEBR pattern op(_,_,_)
operator bs_stops alias BS_STOPS pattern op(_,_,_)
operator set_bs_speed alias SET_BS_SPEED pattern op(_,_,_,_)
operator set_stop_loc alias SET_STOP_LOC pattern op(_)
operator getmetrodata alias GETMETRODATA pattern op(_,_,_)
operator sl2reg alias SL2REG pattern op(_)
operator tm_segs alias TM_SEGS pattern op(_)

operator tm_join1 alias TM_JOIN1 pattern op(_,_,_)

operator nearstops_building alias NEARSTOPS_PAVE pattern op(_,_)

operator decomposegenmo alias DECOMPOSEGENMO pattern op(_,_,_)
operator bulkloadtmrtree alias BULKLOADTMRTREE pattern _ op [_,_,_]
operator tm_nodes alias TM_NODES pattern op(_,_,_)


