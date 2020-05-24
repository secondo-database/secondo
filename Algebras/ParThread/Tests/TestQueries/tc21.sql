open database parthread_lie;

#testcase TC21par4CtxSingleHashjoinNoDelay
#yields (int 233976)
query Osm_Ways feed par[1, NodeRef] {w} Osm_Nodes feed par[1, NodeId] {n} hashjoin[NodeRef_w, NodeId_n] par[16] count;


close database;
