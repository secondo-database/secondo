open database parthread_lie;

#testcase tc1_parOneCount
query Osm_Ways feed delayS[1] par[1] count;

close database;
