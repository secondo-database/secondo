open database parthread_lie;

#testcase tc4_parThreeWayIdFilterProject
#yields *compareWayRel
query Osm_Ways feed par[1, WayId] filter[.NodeCounter=3] delayS[2] filter[.NodeCounter=3] par[8] project[WayId, NodeCounter] delayS[2] par[8] count;


close database;
