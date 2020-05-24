open database parthread_lie;

#testcase TC10parAttr4CtxFilterProjectNoDelay
#yields (int 24979)
query Osm_Ways feed par[1, WayId] filter[.NodeCounter>3] par[16] project[WayId, NodeCounter] par[16] count;

close database;
