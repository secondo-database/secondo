/*
1 Testing translation of queries

[File ~testcases.pl~]

As described in Section 5 the optimizer supports a sql- like language
to call queries. A primary task is to expand this language with new
operators and types introduced by additional algebras.

This runs the risk of possible side effects on existing translation 
rules. Regression tests can help to detect such errors during 
implementation.

1.1 Testcases

Testcases are simple prolog- facts consisting of a unique testcase-
number, the input- statement and the translated query- string.

---- testcase(testcaseNumber, inputStatement, expectedQuery).
----

To add a new testcase, just create a new fact, set a new ~TestcaseNumber~
 followed by the query in user-level language and the expected result
 as string. 

1.1.1 Regular queries on local relations

prerequisite: 

  * simple star select with a single relation

*/

%fapra 2015/16
%testcase(1,select * from roads,'query Roads  feed  consume ').  B.Huber
testcase(1,select * from roads,'query Roads  feed consume').    %B.Huber

/*
  * select with projection and a single relation

*/
testcase(2,select [osm_id, name] from roads,
%B.Huber
%         'query Roads  feed  project[Osm_id, Name]  consume '). 
         'query Roads  feed project[Osm_id, Name] consume'). 
%B.Huber end

/*
  * select with projection, single relation and predicate 
    (expectedQuery needs to be validated)

*/
testcase(3,select [osm_id, name] from roads where name 
           starts "Fahrenbecke",
%B.Huber
%        'query Roads  feed  filter[(.Name starts "Fahrenbecke")]  \c
%        project[Osm_id, Name]  consume ').
        'query Roads  feedproject[Name, Osm_id] \c
        filter[(.Name starts "Fahrenbecke")] project[Osm_id, Name] consume').
%B.Huber end

/*
1.1.2 Distributed queries using Distributed2Algebra 

prerequisite: Roads is distributed by RoadsB1 without index 

  * simple star select with a single relation (no use of dmap-operator)

*/
testcase(4,select * from roads_d,
%B.Huber
%         'query RoadsDfRandom  dsummarize  consume ').
         'query RoadsDfRandom dmap["", .  feed] dsummarize consume').  
%B.Huber end

/*
  * select with a projection of one attribute and single relation 
  
*/
testcase(5,select name from roads_d,
%B.Huber
%        'query RoadsDfRandom  dmap["", .  feed  project[Name] ]  \c
%        dsummarize  consume ').
        'query RoadsDfRandom dmap["", .  feed project[Name]] \c
        dsummarize consume').
%B.Huber end

/*
  * select with a projection of multiple attributes and single relation

*/
testcase(6,select [name, osm_id] from roads_d,
%B.Huber
%         'query RoadsDfRandom  dmap["", .  feed  project[Name, \c
%         Osm_id] ]  dsummarize  consume ').
         'query RoadsDfRandom dmap["", .  feed \c
         project[Name, Osm_id]] dsummarize consume').
%B.Huber end

/*
  * simple star select with a single relation and single predicate 

*/
testcase(7,select * from roads_d where name starts "Fahrenbecke",
%B.Huber
%         'query RoadsDfRandom  dmap["", .  feed  filter[(.Name starts \c
%         "Fahrenbecke")] ]  dsummarize  consume ').
         'query RoadsDfSpatial dmap["", .  feed filter[(.Name starts \c
         "Fahrenbecke")]] dsummarize consume').
%B.Huber end


/*
  * count tupel of distributed relation supported by an index
  
*/
testcase(8,select count(*) from natural_d where type 
  starts "forest",
%B.Huber
%   'query NaturalDFunctionIndex_Type  NaturalDFunctionIndex  dmap2\c
%   ["", .  ..  range["forest", "forest"++], 1238]  \c
%   dsummarize  count ').
   'query NaturalDFunctionIndex dmap["", .  feed \c
   filter[(.Type starts "forest")] count] getValue tie[(.  + .. )]').
%B.Huber end


% optional: distributed evaluation of aggregations
% testcase(8,select count(*) from roads_d where name starts 
%  "Fahrenbecke",
%   'query RoadsB2_Name RoadsB2
%dloop2["", . .. range["Fahrenbecke", "Fahrenbecke"++]
%count]
%getValue tie[. + ..]).

/*
  * spatial query using rtree- index
  
*/
testcase(9, select * from buildings_d where geodata intersects eichlinghofen,
%B.Huber
%  'query share("eichlinghofen",TRUE); query BuildingsDSpatialIndex_GeoData  \c
%   BuildingsDSpatialIndex  dmap2["", .  ..  \c
%   windowintersects[eichlinghofen ]  \c
%   filter[(.GeoData intersects eichlinghofen )]  filter[.Original] , 1238]  \c
%   dsummarize  consume ').
  'query BuildingsDSpatialIndex_GeoData BuildingsDSpatialIndex \c
   dmap2["", .  ..  windowintersects[eichlinghofen] \c
   filter[(.GeoData intersects eichlinghofen)], 1238] \c
   dsummarize consume').
%B.Huber end


/*
  * equijoin test for both relations are partitioned by join attribute
  
*/
testcase(10, select * from [waterways_d as w1, waterways_d as w2] 
        where w1:type=w2:type,
%B.Huber
%  'query WaterwaysDFunction  WaterwaysDFunction  dmap2["", .  \c
%   feed {w1} ..  feed {w2} hashjoin[Type_w1, Type_w2, 999997] , 1238]  \c
%   dsummarize  consume ').
  'query WaterwaysDFunction WaterwaysDFunction dmap2["", .  \c
   feed{w1} ..  feed{w2} hashjoin[Type_w1, Type_w2, 999997], 1238] \c
   dsummarize consume').
%B.Huber end


/*
  * Equijoin test for both relations are partitioned, but not by join attribute
  
*/
testcase(11, select * from [roads_d as r1, roads_d as r2]
         where r1:name=r2:name,
%B.Huber
%  'query RoadsDfRandom  partitionF["LeftPartOfJoin", .  feed , \c
%  hashvalue(..Name,999997) , 0]  collect2["L", 1238]  RoadsDfRandom  \c
%  partitionF["RightPartOfJoin", .  feed , hashvalue(..Name,999997) , 0]  \c
%  collect2["R", 1238]  dmap2["", .  feed {r1} ..  feed {r2} \c
%  hashjoin[Name_r1, Name_r2, 999997] , 1238]  dsummarize  consume ').
  'query RoadsDfRandom partitionF["", .  feed, hashvalue(..Name, 999997), 0] \c
  collect2["", 1238] \c
  RoadsDfRandom partitionF["", .  feed, hashvalue(..Name, 999997), 0] \c
  collect2["", 1238] \c
  dmap2["", .  feed{r1} ..  feed{r2} \c
  hashjoin[Name_r1, Name_r2, 999997], 1238] \c
  dsummarize consume').
%B.Huber end

/*
  * Equijoin test for both are partitoned using modulo
  
*/
testcase(12, select * from [places_d as p1, places_d as p2] 
        where p1:population=p2:population,
%B.Huber
%  'query PlacesDfModuloPop  PlacesDfModuloPop  dmap2["", .  \c
%   feed {p1} ..  feed {p2} hashjoin[Population_p1, Population_p2, \c
%   999997] , 1238]  dsummarize  consume '). 
  'query PlacesDfModuloPop PlacesDfModuloPop dmap2["", .  \c
   feed{p1} ..  feed{p2} hashjoin[Population_p1, Population_p2, \c
   999997], 1238] dsummarize consume').
%B.Huber end
 
/*
  * Equijoin test for one relation is partitioned and the other is replicated

*/
testcase(13, select * from [places_d as p1, railways_d as r2]
where p1:osm_id=r2:osm_id, 
%B.Huber
%    'query PlacesDfModuloPop  dmap["", \c
%    Railways  feed {p1} .  feed {r2} hashjoin[Osm_id_p1, Osm_id_r2, \c
%    999997] ]  dsummarize  consume ').
    'query PlacesDfModuloPop \c
    partitionF["", .  feed, hashvalue(..Osm_id, 999997), 0] \c
    collect2["", 1238] \c
    Railways partitionF["", .  feed, hashvalue(..Osm_id, 999997), 0] \c
    collect2["", 1238] \c
    dmap2["", .  feed{p1} ..  feed{r2} \c
    hashjoin[Osm_id_p1, Osm_id_r2, 999997], 1238] dsummarize consume').
%B.Huber end
  
/*
  * Equijoin test for both relations are replicated (Error case)

*/
testcase(14, select * from [railways_d as b1, railways_d as b2] 
where b1:osm_id=b2:osm_id,
%B.Huber
%    'failed').
    'query Railways \c
     partitionF["", .  feed, hashvalue(..Osm_id, 999997), 0] \c
     collect2["", 1238] \c
     Railways partitionF["", .  feed, hashvalue(..Osm_id, 999997), 0] \c
     collect2["", 1238] \c
     dmap2["", .  feed{b1} ..  feed{b2} \c
     hashjoin[Osm_id_b1, Osm_id_b2, 999997], 1238] dsummarize consume').
%B.Huber end

/*
  * Standard Join for both are partitioned (Error case)

*/
testcase(15, select * from [waterways_d as w1, waterways_d as w2] 
where w1:type#w2:type,
%B.Huber
%    'failed.'). 
    'query WaterwaysDFunction WaterwaysDFunction \c
     dproduct["", .  feed{w1} ..  feed{w2} \c
     symmjoin[(.Type_w1 # ..Type_w2)], 1238] dsummarize consume').
%B.Huber end


/*
  * Standard Join for one relation is partitioned and the other is replicated

*/
testcase(17, select * from [places_d as p1, railways_d as b2] 
where substr(p1:osm_id,1,3)=substr(b2:osm_id,1,3),
%B.Huber
%'query PlacesDfModuloPop  dmap["", .  \c
%feed {b2}  Railways  feed {p1}  product  \c
%filter[(substr(.Osm_id_p1, 1, 3) = \c
%  substr(.Osm_id_b2, 1, 3))] ]  dsummarize  consume ').
'query PlacesDfModuloPop Railways \c
 dproduct["", .  feed{p1} ..  feed{b2} \c
 symmjoin[(substr(.Osm_id_p1, 1, 3) = \c
 substr(..Osm_id_b2, 1, 3))], 1238] dsummarize consume').
%B.Huber end
   
/*
  * Standard Join test for both relations are replicated (Error case)
  
*/
testcase(18, select * from [railways_d as b1, railways_d as b2]
        where substr(b1:osm_id,1,3)=substr(b2:osm_id,1,3),
%B.Huber
%  'failed.').
  'query Railways Railways dproduct["", .  feed{b1} ..  feed{b2} \c
   symmjoin[(substr(.Osm_id_b1, 1, 3) = \c
   substr(..Osm_id_b2, 1, 3))], 1238] dsummarize consume').
%B.Huber end


/*
  * distributed spatial query using a database object in predicate

*/
testcase(19,select * from roads_d where geodata intersects eichlinghofen,
%B.Huber
%         'query share("eichlinghofen",TRUE); query RoadsDfRandom  \c
%         dmap["", .  feed  filter[(.GeoData intersects eichlinghofen \c
%         )] ]  dsummarize  consume ').
         'query RoadsDfSpatial \c
          dmap["", .  feed filter[(.GeoData intersects eichlinghofen)]] \c
          dsummarize consume').
%B.Huber end

/*
  * local spatial query using a database object in predicate

*/
testcase(20,select * from roads where geodata intersects eichlinghofen,
%B.Huber
%         'query Roads  feed  filter[(.GeoData intersects eichlinghofen )\c
%         ]  consume ').
         'query Roads  feed filter[(.GeoData intersects eichlinghofen)] \c
          consume').
%B.Huber end

/*
  *  star select with projection and renamed attributes
     (doesn't work for non-distributed queries in basic- optimizer)

*/
testcase(21,select r1:name from roads_d as r1,
%B.Huber
%         'query RoadsDfRandom  dmap["", .  feed {r1}  \c
%         project[Name_r1] ]  dsummarize  consume ').
         'query RoadsDfRandom dmap["", .  feed{r1} \c
          project[Name_r1]] dsummarize consume').
%B.Huber end


/*
  *  simple select with count function

*/
testcase(22,select count(*) from roads_d,
%B.Huber
%         'query RoadsDfRandom  dsummarize  count ').
         'query RoadsDfRandom dmap["", .  feed count] \c
          getValue tie[(.  + .. )]').
%B.Huber end

/*
  *  merge filtrations with rename

*/
testcase(23,select * from buildings_d as x where [x:name="ho",x:name="hi"],
%B.Huber
%  'query BuildingsDSpatialIndex  dmap["", .  feed  \c
%  filter[.Original] ]  dmap["", .  feed {x}  \c
%  filter[(.Name_x = "hi")]  filter[(.Name_x = "ho")] ]  \c
%  dsummarize  consume ').
  'query BuildingsDSpatialIndex dmap["", .  feed{x} \c
   filter[(.Name_x = "hi")] filter[(.Name_x = "ho")]] \c
   dsummarize consume').
%B.Huber end

/*
  *  merge filtrations without rename

*/

testcase(24,select * from buildings_d where [name="ho",name="hi"],
%B.Huber
%  'query BuildingsDSpatialIndex  dmap["", .  feed  filter[.Original] ]  \c
%  dmap["", .  feed  filter[(.Name = "hi")]  \c
%  filter[(.Name = "ho")] ]  dsummarize  consume ').
  'query BuildingsDSpatialIndex dmap["", .  feed filter[(.Name = "hi")] \c
   filter[(.Name = "ho")]] dsummarize consume').
%B.Huber end

/*
  *  spatial join, both arguments distributed by join attribute

*/

testcase(25,select * from [buildings_d as x, buildings_d as y] 
  where x:geodata intersects y:geodata,
%B.Huber
%  'query BuildingsDSpatialIndex  BuildingsDSpatialIndex  \c
%  dmap2["", .  feed {x}  ..  feed {y}  \c
%  itSpatialJoin[GeoData_x, GeoData_y] filter[(.Cell_x = .Cell_y)]  \c
%  filter[gridintersects(grid,bbox(.GeoData_x) , \c
%    bbox(.GeoData_y) , .Cell_x) ]  \c
%  filter[(.GeoData_x intersects .GeoData_y)] , \c
%  1238]  dsummarize  consume ').
  'query BuildingsDSpatialIndex BuildingsDSpatialIndex \c
   dmap2["", .  feed{x} ..  feed{y} itSpatialJoin[GeoData_x, GeoData_y] \c
   filter[(((.Cell_x = .Cell_y) and (.GeoData_x intersects .GeoData_y)) \c
   and gridintersects(grid, bbox(.GeoData_x), \c
   bbox(.GeoData_y), .Cell_x))], 1238] dsummarize consume').
%B.Huber end

/*
  *  spatial join, one argument replicated, one argument distributed
     by join attribute

*/

testcase(26, select * from [buildings_d as x, railways_d as y] 
  where x:geodata intersects y:geodata,
%B.Huber
%  'query BuildingsDSpatialIndex  dmap["", .  feed {x}  \c
%  Railways  feed {y}  itSpatialJoin[GeoData_x, GeoData_y] \c
%  filter[(.Cell_x = .Cell_y)]  \c
%  filter[gridintersects(. ,bbox(.GeoData_x) , \c
%    bbox(.GeoData_y) , .Cell_x) ]  \c
%  filter[(.GeoData_x intersects .GeoData_y)] ]  \c
%  dsummarize  consume ').
  'query BuildingsDSpatialIndex Railways \c
   partitionF["", .  feed \c
   extendstream[Cell: cellnumber(bbox(.GeoData), grid)], ..Cell, 0] \c
   collect2["", 1238] \c
   dmap2["", .  feed{x} ..  feed{y} itSpatialJoin[GeoData_x, GeoData_y] \c
   filter[(((.Cell_x = .Cell_y) and (.GeoData_x intersects .GeoData_y)) and \c
   gridintersects(grid, bbox(.GeoData_x), bbox(.GeoData_y), .Cell_x))], 1238] \c
   dsummarize consume').
%B.Huber end

/*
  *  spatial join, one argument distributed by join attribute,
     one argument distributed by different attribute

*/

testcase(27, select * from [roads_d as x, waterways_d as y] 
  where x:geodata intersects y:geodata,
%B.Huber
%  'query RoadsDfSpatial  WaterwaysDFunction  \c
%  partitionF["", .  feed  \c
%  extendstream(Cell: cellnumber(bbox(.GeoData) ,grid) ), ..Cell, \c
%  0]  collect2["", 1238]  dmap2["", .  \c
%  feed {x}  ..  feed {y}  \c
%  itSpatialJoin[GeoData_x, GeoData_y] \c
%  filter[(.Cell_x = .Cell_y)]  \c
%  filter[gridintersects(grid,bbox(.GeoData_x) , \c
%  bbox(.GeoData_y) , .Cell_x) ]  filter[(.GeoData_x intersects \c
%  .GeoData_y)] , 1238]  dsummarize  consume ').
  'query RoadsDfSpatial WaterwaysDFunction \c
   partitionF["", .  feed \c
   extendstream[Cell: cellnumber(bbox(.GeoData), grid)], ..Cell, 0] \c
   collect2["", 1238] \c
   dmap2["", .  feed{x} ..  feed{y} itSpatialJoin[GeoData_x, GeoData_y] \c
   filter[(((.Cell_x = .Cell_y) and (.GeoData_x intersects .GeoData_y)) and \c
   gridintersects(grid, bbox(.GeoData_x), bbox(.GeoData_y), .Cell_x))], 1238] \c
   dsummarize consume').
%B.Huber end

/*
  *  spatial join, both arguments distributed by non-join attributes

*/

testcase(28, select * from [natural_d as x, waterways_d as y] where 
  x:geodata intersects y:geodata,
%B.Huber
%  'query NaturalDFunctionIndex  partitionF["", .  feed  \c
%  extendstream(Cell: cellnumber(bbox(.GeoData) ,grid) ), ..Cell, 0]  \c
%  WaterwaysDFunction  partitionF["", .  feed  \c
%  extendstream(Cell: cellnumber(bbox(.GeoData) ,grid) ), ..Cell, 0]  \c
%  areduce2[[], .  feed {x}  ..  feed {y}  itSpatialJoin[GeoData_x, GeoData_y]\c
%  filter[(.Cell_x = .Cell_y)]  filter[gridintersects(grid,bbox(.GeoData_x) , \c
%  bbox(.GeoData_y) , .Cell_x) ]  \c
%  filter[(.GeoData_x intersects .GeoData_y)] , 1238]  \c
%  dsummarize  consume ').
  'query NaturalDFunctionIndex partitionF["", .  feed \c
   extendstream[Cell: cellnumber(bbox(.GeoData), grid)], .Cell, 0] \c
   collect2["", 1238] WaterwaysDFunction \c
   partitionF["", .  feed extendstream[Cell: \c
   cellnumber(bbox(.GeoData), grid)], ..Cell, 0] collect2["", 1238] \c
   dmap2["", .  feed{x} ..  feed{y} itSpatialJoin[GeoData_x, GeoData_y] \c
   filter[(((.Cell_x = .Cell_y) and (.GeoData_x intersects .GeoData_y)) and \c
   gridintersects(grid, bbox(.GeoData_x), bbox(.GeoData_y), .Cell_x))], 1238] \c
   dsummarize consume').
%B.Huber end

/*
  *  spatial join, one argument replicated, one argument distributed by
     non-join attribute

*/

testcase(29, select * from [natural_d as x, railways_d as y] 
  where x:geodata intersects y:geodata,
%B.Huber
%  'query NaturalDFunctionIndex  dmap["", .  feed {x}  Railways  feed {y}  \c
%  itSpatialJoin[GeoData_x, GeoData_y] filter[(.Cell_x = .Cell_y)]  \c
%  filter[gridintersects(. ,bbox(.GeoData_x) , \c
%    bbox(.GeoData_y) , .Cell_x) ]  \c
%  filter[(.GeoData_x intersects .GeoData_y)] ]  dsummarize  consume '). 
  'query NaturalDFunctionIndex \c
   partitionF["", .  feed extendstream[Cell: \c
   cellnumber(bbox(.GeoData), grid)], .Cell, 0] collect2["", 1238] \c
   Railways partitionF["", .  feed \c
   extendstream[Cell: cellnumber(bbox(.GeoData), grid)], ..Cell, 0] \c
   collect2["", 1238] \c
   dmap2["", .  feed{x} ..  feed{y} itSpatialJoin[GeoData_x, GeoData_y] \c
   filter[(((.Cell_x = .Cell_y) and (.GeoData_x intersects .GeoData_y)) and \c
   gridintersects(grid, bbox(.GeoData_x), bbox(.GeoData_y), .Cell_x))], 1238] \c
   dsummarize consume').
%B.Huber end


/*
  * spatial join, two replicated arguments - expected fail

*/

testcase(30, select * from [railways_d as x, railways_d as y] 
  where x:geodata intersects y:geodata,
%B.Huber
%  'failed.').
  'query Railways partitionF["", .  feed \c
   extendstream[Cell: cellnumber(bbox(.GeoData), grid)], .Cell, 0] \c
   collect2["", 1238] \c
   Railways partitionF["", .  feed \c
   extendstream[Cell: cellnumber(bbox(.GeoData), grid)], ..Cell, 0] \c
   collect2["", 1238] \c
   dmap2["", .  feed{x} ..  feed{y} itSpatialJoin[GeoData_x, GeoData_y] \c
   filter[(((.Cell_x = .Cell_y) and (.GeoData_x intersects .GeoData_y)) and \c
   gridintersects(grid, bbox(.GeoData_x), bbox(.GeoData_y), .Cell_x))], \c
   1238] dsummarize consume').
%B.Huber end

/*
  * spatial join combined with simple selection

*/

testcase(31, select * from [roads_d as x, waterways_d as y] 
  where [x:geodata intersects y:geodata, x:name = "Secondo <3"],
%B.Huber
%  'query RoadsDfSpatial  WaterwaysDFunction  \c
%  partitionF["", .  feed  extendstream(Cell: \c
%  cellnumber(bbox(.GeoData) ,grid) ), ..Cell, 0]  \c
%  collect2["", 1238]  dmap2["", .  feed {x}  ..  feed {y}  \c
%  itSpatialJoin[GeoData_x, GeoData_y] \c
%  filter[(.Cell_x = .Cell_y)]  \c
%  filter[gridintersects(grid,bbox(.GeoData_x) , bbox(.GeoData_y) \c
%  , .Cell_x) ]  filter[(.GeoData_x intersects \c
%  .GeoData_y)] , 1238]  dmap["", .  \c
%  feed {x}  filter[(.Name_x = "Secondo <3")] ]  \c
%  dsummarize  consume ').
  'query RoadsDfSpatial WaterwaysDFunction \c
   partitionF["", .  feed \c
   extendstream[Cell: cellnumber(bbox(.GeoData), grid)], ..Cell, 0] \c
   collect2["", 1238] \c
   dmap2["", .  feed{x} filter[(.Name_x = "Secondo <3")] ..  feed{y} \c
   itSpatialJoin[GeoData_x, GeoData_y] filter[(((.Cell_x = .Cell_y) and \c
   (.GeoData_x intersects .GeoData_y)) and \c
   gridintersects(grid, bbox(.GeoData_x), \c
   bbox(.GeoData_y), .Cell_x))], 1238] dsummarize consume').
%B.Huber end


%B.Huber

/*
  * count with groupby for an attribute without where condition 

*/

testcase(32,select [type, count(*) as cnt]
            from roads_d
            groupby type,
         'query RoadsDfRandom dmap["", .  feed sortby[Type asc] \c
                groupby[Type; Cnt: group feed count]] \c
                dsummarize sortby[Type asc] \c
                groupby[Type; Cnt: group feed sum[Cnt]] consume').


/*
  * avg function with groupby for an attribute without where condition

*/

testcase(33,select [type, avg(width) as awidth]
            from waterways_d
            groupby [type],
         'query WaterwaysDFunction \c 
                dmap["", .  feed sortby[Type asc] \c 
                            groupby[Type; Var1: group feed sum[Width], \c
                                          Var2: group feed count]] \c
                dsummarize sortby[Type asc] \c
                groupby[Type; Awidth: group feed sum[Var1] / \c
                                      group feed sum[Var2]] consume').


/*
  * sum function with groupby for an attribute with where condition

*/

testcase(34,select [type, sum(maxspeed) as summax]
            from roads_d where name starts "F" 
            groupby type,
         'query RoadsDfSpatial \c
                dmap["", .  feed filter[(.Name starts "F")] sortby[Type asc] \c
                            groupby[Type; Summax: group feed sum[Maxspeed]]] \c
                     dsummarize sortby[Type asc] \c
                     groupby[Type; Summax: group feed sum[Summax]] consume').


/*
  * max function with groupby for an attribute without where condition

*/

testcase(35,select [name, max(maxspeed) as maximalspeed]
            from roads_d 
            groupby name,
  'query RoadsDfRandom \c
         dmap["", .  feed sortby[Name asc] \c
                     groupby[Name; Maximalspeed: group feed max[Maxspeed]]] \c
         dsummarize sortby[Name asc] \c
         groupby[Name; Maximalspeed: group feed sum[Maximalspeed]] consume').


/*
  * min function with groupby for an attribute and with where condition

*/

testcase(36, select [type, min(maxspeed) as minspeed]
             from roads_d  
             where name starts "Fahren" 
             groupby type,
         'query RoadsDfSpatial \c
                dmap["", .  feed filter[(.Name starts "Fahren")] \c
                     sortby[Type asc] \c
                     groupby[Type; Minspeed: group feed min[Maxspeed]]] \c
                dsummarize sortby[Type asc] \c
                groupby[Type; Minspeed: group feed sum[Minspeed]] consume').

/*
  * count-, avg-, max-, min-function in combination
  * with groupby for an attribute and with where condition

*/

testcase(37, select [name, count(*) as cnt, 
                           avg(maxspeed) as avgspeed, 
                           max(maxspeed) as maxspeed, 
                           min(maxspeed) as minspeed] 
              from roads_d where name starts "F" groupby name,
    'query RoadsDfSpatial \c
           dmap["", .  feed filter[(.Name starts "F")] sortby[Name asc] \c
                groupby[Name; \c
                  Cnt: group feed count, Var1: group feed sum[Maxspeed], \c
                                         Var2: group feed count, \c
                       Maxspeed: group feed max[Maxspeed], \c
                       Minspeed: group feed min[Maxspeed]]] \c
           dsummarize sortby[Name asc] \c
             groupby[Name; \c
                   Cnt: group feed sum[Cnt], \c
                   Avgspeed: group feed sum[Var1] / group feed sum[Var2], \c
                   Maxspeed: group feed sum[Maxspeed], \c
                   Minspeed: group feed sum[Minspeed]] consume').           

%B.Huber end


/*
1.2 Run Tests

Once the testcases are defined, it is possible to evaluate it
 automatically by calling optimize on the ~inputStatement~ and
 comparing the output with the ~expectedQuery~. 
 
To run all available testcases simple call the rule 
 checkAllTestCases. 

*/
checkAllTestCases :-
    findall(TestCaseNo,testcase(TestCaseNo,_,_),ListOfTestCaseNo),
    checkTestCase(ListOfTestCaseNo),!.

checkTestCase([H|T]) :-
    ansi_format([], 'Testing case ~w-------------\n', [H]),
    checkTestCase(H),
    write('\n\n'),
    checkTestCase(T).

checkTestCase([]).

/*
The rule checkTestCase(~TestcaseNumber~) will only execute the
 testcase assigned to the ~testcaseNumber~.
  
*/

checkTestCase(NumTestCase) :-
    testcase(NumTestCase,SqlQuery,ExpectedQueryStr),
    optimize(SqlQuery,TranslatedQuery, _),!,
    my_string_to_atom(ExpectedQueryStr,ExpectedQuery),
    compareTestCase(ExpectedQuery,TranslatedQuery,NumTestCase),
    !.

checkTestCase(NumTestCase) :-
    testcase(NumTestCase,_,ExpectedQueryStr),
    ansi_format([fg(red)], 'testcase ~w failed.\nExpected:\n', [NumTestCase]),
    ansi_format([], '~w' , [ExpectedQueryStr]),
    !.

checkTestCase(NumTestCase) :-
    ansi_format([fg(red)], 'testcase with number ~w not found', 
    [NumTestCase]),
    !.

compareTestCase(ExpectedQuery,TranslatedQuery,NumTestCase) :-
    is_list(TranslatedQuery),
    atomic_list_concat(TranslatedQuery,'; query ',TransQueryAtom),
    compareTestCase(ExpectedQuery,TransQueryAtom,NumTestCase),
    !.
    
compareTestCase(ExpectedQuery,TranslatedQuery,NumTestCase) :- 
    atom_concat('query ', TranslatedQuery, TranslatedQueryStr),
    (TranslatedQueryStr == ExpectedQuery
     -> ansi_format([fg(green)], 'testcase ~w succeeded \n', [NumTestCase]);
     ansi_format([fg(red)], 'testcase ~w failed.\nExpected:\n', [NumTestCase]),
     ansi_format([], '>>~w<<' , [ExpectedQuery]),
     ansi_format([fg(red)], '\n but got:\n', []), 
     ansi_format([], '>>query ~w<<', [TranslatedQuery])),
    !. 
%end fapra 2015/16
