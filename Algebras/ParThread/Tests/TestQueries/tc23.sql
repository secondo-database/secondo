open database parthread_lie;

#testcase TC23par6CtxDoubleHashjoinNoDelay
#yields (int 56395)
query Osm_Ways feed par[1, NodeRef] {w} Osm_Nodes feed par[1, NodeId] {n} hashjoin[NodeRef_w, NodeId_n] par[16, WayId_w] Osm_WayTags feed par[1, WayIdInTag] {t} hashjoin[WayId_w, WayIdInTag_t] par[16] count;

close database;
