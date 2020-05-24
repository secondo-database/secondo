open database parthread_lie;

#testcase TC5par3CtxFilterDelay1
#yields (int 233976)
query Osm_Ways feed par[1] filter[.NodeCounter=3] delayS[1] par[16] count;

close database;
