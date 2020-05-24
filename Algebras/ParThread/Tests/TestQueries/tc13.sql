open database parthread_lie;

#testcase TC13par4CtxGeoDistanceNoDelay
#yields (int 267587)
query Osm_Nodes feed par[1] extend[Geo: makepoint(.Lon, .Lat)] extend[DistInKm: distanceOrthodrome(.Geo, [const point value(10.00 50.00)])/1000] par[16] count;

close database;
