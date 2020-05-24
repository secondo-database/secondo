open database parthread_lie;

#testcase TC4par3CtxFilterNoDelay
#yields (int 233976)
query Osm_Ways feed par[1] filter[.NodeCounter>3] par[16] count;

close database;
