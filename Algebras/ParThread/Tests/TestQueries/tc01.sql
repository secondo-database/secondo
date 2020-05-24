open database parthread_lie;

#testcase TC1par2CtxOnlyPipelineNoDelay
#yields (int 340107)
query Osm_Ways feed par[1] count;

close database;
