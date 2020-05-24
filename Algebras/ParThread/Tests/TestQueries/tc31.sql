open database parthread_lie;

#testcase TC31par4CtxSingleProductNoDelay
#yields (int 233976)
query Osm_Ways feedNth[1000, TRUE] par[1] Osm_Nodes feed par[1] product par[1] count

close database;



