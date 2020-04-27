open database parthread_lie;

#testcase tc9_dataParDoubleHashjoin
#yields (int 465556)
query Osm_Ways feed par[1, NodeRef] filter[.NodeCounter=3] delayS[2] {w} Osm_Nodes feed par[1, NodeId] {n} hashjoin[NodeRef_w, NodeId_n] par[8, WayId_w] Osm_WayTags feed par[1, WayIdInTag] {t} hashjoin[WayId_w, WayIdInTag_t] par[8] count

close database;
