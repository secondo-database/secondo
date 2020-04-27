open database parthread_lie;

#testcase tc5_parSingleSymmjoin
#yields (int 688)
query Osm_Ways feed head[1000] filter[.NodeCounter>3] {w} delayS[2] par[1] Osm_Nodes feed {n} par[1] symmjoin[.NodeRef_w = ..NodeId_n] par[1] count

close database;
