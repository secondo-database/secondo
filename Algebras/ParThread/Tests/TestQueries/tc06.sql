open database parthread_lie;

#testcase tc6_parDoubleSymmjoin
#yields (int 3410)
query Osm_Ways feed head[1000] filter[.NodeCounter>3] {w} delayS[2] par[1] Osm_Nodes feed {n} par[1] symmjoin[.NodeRef_w = ..NodeId_n] par[1] Osm_WayTags feed {t} symmjoin[.WayId_w = ..WayIdInTag_t] par[1] count

close database;
