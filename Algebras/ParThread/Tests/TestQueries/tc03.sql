open database parthread_lie;

#testcase tc3_parThreeWayIdFilter
#yields (int 24979)
query Osm_Ways feed par[1, WayId] filter[.NodeCounter=3] delayS[3] par[3] count;

close database;
