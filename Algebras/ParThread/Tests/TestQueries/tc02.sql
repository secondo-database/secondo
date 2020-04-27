open database parthread_lie;

#testcase tc2_dataParThreeFilter
#yields (int 24979)
query Osm_Ways feed par[1] filter[.NodeCounter=3] delayS[3] par[3] count;

close database;
