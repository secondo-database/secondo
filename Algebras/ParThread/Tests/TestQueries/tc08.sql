open database parthread_lie;

#testcase tc8_dataParSingleHashjoin
#yields (int 233976)
query Osm_Ways feed par[1,NodeRef] filter[.NodeCounter=3] delayS[2] {w} Osm_Nodes feed par[1,NodeId] {n} hashjoin[NodeRef_w, NodeId_n] par[8] count


close database;
