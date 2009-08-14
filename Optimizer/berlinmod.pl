/*
BerlinMOD/R Queries for Secondo SQL

(OBA Queries)

The following definitions present the BerlinMOD/R queries in the Secondo-SQL
syntax.

*/

/*
Choose Option Settings

*/

:- setOption(autoSamples).       % determin sample sizes autonomously
:- setOption(determinePredSig).  % use type inference
:- setOption(rtreeIndexRules).   % utilize rtree indexes
:- setOption(rewriteCSE).        % hand common subexpressions smartly
:- setOption(rewriteRemove).     % remove unused attributes as soon as possible
:- setOption(earlyproject).      % remove unused attributes as soon as possible
:- setOption(subqueryUnnesting). % fully activate subquery processing


/*
Create some materialized views:

*/


:- catch(open database berlinmod,_,fail),
   catch(
    (
      let(querylicences1, select * from querylicences where id <= 10 orderby id first 10),
      let(querylicences2, select * from querylicences where id > 10  orderby id first 10),
      let(queryinstants1, select * from queryinstants where id <= 10 orderby id first 10),
      let(queryinstants2, select * from queryinstants where id > 10  orderby id first 10),
      let(querypoints1,   select * from querypoints   where id <= 10 orderby id first 10),
      let(querypoints2,   select * from querypoints   where id > 10  orderby id first 10),
      let(queryperiods1,  select * from queryperiods  where id <= 10 orderby id first 10),
      let(queryperiods2,  select * from queryperiods  where id > 10  orderby id first 10),
      let(queryregions1,  select * from queryregions  where id <= 10 orderby id first 10),
      let(queryregions2,  select * from queryregions  where id > 10  orderby id first 10)
    ), _, true
   ).



/*
Secondo-SQL Query Definitions

*/

% Query 1:
sqlBerlinMOD_R_query(1,
  select distinct [ll:licence as licence, c:model as model]
  from [datasccar as c, querylicences as ll]
  where c:licence = ll:licence
).

% Query 2 :
sqlBerlinMOD_R_query(2,
  select count(licence) from datasccar where type = "passenger"
).

% Query 3:
sqlBerlinMOD_R_query(3,
  select [ll:licence as licence, ii:instant as instant,
          val(c:journey atinstant ii:instant) as pos]
  from [datasccar as c, querylicences1 as ll, queryinstants1 as ii]
  where [c:licence = ll:licence, c:journey present ii:instant]
).

% Query  4:
sqlBerlinMOD_R_query(4,
  select [pp:pos as pos, c:licence as licence]
  from [datasccar as c,querypoints as pp]
  where c:journey passes pp:pos
).

% Query 5:
sqlBerlinMOD_R_query(5,
  select [ll1:licence as licence1, ll2:licence as licence2,
          distance(trajectory(v1:journey),trajectory(v2:journey))as dist]
  from [datasccar as v1, datasccar as v2, querylicences1 as ll1,
        querylicences2 as ll2]
  where [v1:licence = ll1:licence,
         v2:licence = ll2:licence,
         not(v1:licence = v2:licence)]
).

% Query 6:
sqlBerlinMOD_R_query(6,
  select [v1:licence as licence1, v2:licence as licence2]
  from [datasccar as v1, datasccar as v2]
  where [v1:licence < v2:licence,
         v1:type = "truck",
         v2:type = "truck",
         sometimes(distance(v1:journey,v2:journey) <= 10.0)]
).

% Query 7 omitted (contains a subquery).
:- setOption(subqueryUnnesting). % fully activate subquery processing

sqlBerlinMOD_R_query(7,
  select [pp:pos as pos, v1:licence as licence]
  from [datascar as v1, querypoints as pp]
  where [v1:trip passes pp:pos,
         v1:type = "passenger",
         inst(initial(v1:trip at pp:pos)) <= (
          all(
            select [inst(initial(v2:trip at pp:pos)) as firsttime]
            from [datascar as v2]
            where [v2:trip passes pp:pos,
                  v2:type = "passenger"
                  ]
            )
          )
        ]
).

% Query 8:
sqlBerlinMOD_R_query(8,
  select [v1:licence as licence, pp:period as period,
          length(v1:journey atperiods pp:period) as dist]
  from [datasccar as v1, queryperiods1 as pp, querylicences1 as ll]
  where [v1:licence = ll:licence,v1:journey present pp:period]
).

% Query 9:
sqlBerlinMOD_R_query(9,
  select [pp:period as period, max(length(v1:journey
          atperiods pp:period)) as dist]
  from [datasccar as v1, queryperiods as pp]
  where v1:journey present pp:period
  groupby pp:period
).

% Query 10:
sqlBerlinMOD_R_query(10,
  select [v1:licence as querylicence, v2:licence as
          otherlicence,v1:journey atperiods(deftime((
          distance(v1:journey,v2:journey)<= 3.0) at true)) as pos]
  from [datasccar as v1, datasccar as v2, querylicences1 as ll]
  where [v1:licence = ll:licence,
         not(v2:licence = v1:licence),
         sometimes(distance(v1:journey, v2:journey) <= 3.0)]
).

% Query 11:
sqlBerlinMOD_R_query(11,
  select [c:licence as licence, p:pos as pos, p:instant as instant]
  from [datasccar as c, pointsinstants as p]
  where val(c:journey atinstant p:instant) = p:pos
).

% Query 12:
sqlBerlinMOD_R_query(12,
  select [pi:pos as pos, pi:instant as instant,
          c1:licence as licence1, c2:licence as licence2]
  from [datasccar as c1, datasccar as c2, pointsinstants as pi]
  where [val(c1:journey atinstant pi:instant) = pi:pos,
         val(c2:journey atinstant pi:instant) = pi:pos]
).

% Query 13:
sqlBerlinMOD_R_query(13,
  select [rp:region as region, rp:period as period,
          c:licence as licence]
  from [datasccar as c, regionsperiods as rp]
  where (c:journey atperiods rp:period) passes rp:region
).

% Query 14:
sqlBerlinMOD_R_query(14,
  select [ri:region as region, ri:instant as instant,
          c:licence as licence]
  from [datasccar as c, regionsinstants as ri]
  where val(c:journey atinstant ri:instant) inside ri:region
).

% Query 15:
sqlBerlinMOD_R_query(15,
  select [pp:pos as pos, pp:period as period, c:licence as licence]
  from [datasccar as c, pointsperiods as pp]
  where (c:journey atperiods pp:period) passes pp:pos
).

% Query 16 omitted (predicate depends on 4 relations).


% Query 17:

sqlBerlinMOD_R_query(17,
  select n:pos
  from poscount as n
  where n:hits = poscounttemp
).

/*
Preparating Steps for Certain Benchmark Queries

*/
prepareSqlBerlinMOD_R_query(11) :-
  (secondoCatalogInfo(pointsinstants, _, _, _) ;
  let(pointsinstants,
    select [p:pos as pos, i:instant as instant]
    from [querypoints1 as p, queryinstants1 as i]
  )),
  !.

prepareSqlBerlinMOD_R_query(12) :-
  (secondoCatalogInfo(pointsinstants, _, _, _) ;
  let(pointsinstants,
    select [p:pos as pos, i:instant as instant]
    from [querypoints1 as p, queryinstants1 as i]
  )),
  !.

prepareSqlBerlinMOD_R_query(13) :-
  (secondoCatalogInfo(regionsperiods, _, _, _) ;
  let(regionsperiods,
    select [r:region as region, p:period as period]
    from [queryregions1 as r, queryperiods1 as p]
  )),
  !.

prepareSqlBerlinMOD_R_query(14) :-
  (secondoCatalogInfo(regionsinstants, _, _, _) ;
  let(regionsinstants,
    select [r:region as region, i:instant as instant]
    from [queryregions1 as r, queryinstants1 as i]
  )),
  !.

prepareSqlBerlinMOD_R_query(15) :-
  (secondoCatalogInfo(pointsperiods, _, _, _) ;
  let(pointsperiods,
    select [p:pos as pos, pp:period as period]
    from [querypoints1 as p, queryperiods1 as pp]
  )),
  !.

prepareSqlBerlinMOD_R_query(17) :-
  (secondoCatalogInfo(poscount, _, _, _) ;
  let(poscount,
    select [pp:pos as pos, count(c:licence) as hits]
    from [querypoints as pp, datasccar as c]
    where c:journey passes pp:pos
    groupby pp:pos
  )),
  (secondoCatalogInfo(poscounttemp, _, _, _) ;
  let(poscounttemp,
    select max(hits)
    from poscount
  )),
  !.

prepareSqlBerlinMOD_R_query(_) :- !, true.


/*
Run a Benchmark Query

*/

optBMOD(Nr) :-
  prepareSqlBerlinMOD_R_query(Nr),
  sqlBerlinMOD_R_query(Nr, SqlQuery),
  optimize(SqlQuery, ExecutableQuery, Cost),
  write_list(['\nQuery-Nr:          ', Nr, '\n']),
  write_list(['Secondo-SQL-Query: ', SqlQuery, '\n']),
  write_list(['Executable Query:  ', ExecutableQuery, '\n']),
  write_list(['Estimated Cost:    ', Cost, '\n']).

runBMOD(Nr) :-
  prepareSqlBerlinMOD_R_query(Nr),
  sqlBerlinMOD_R_query(Nr, SqlQuery),
  optimize(SqlQuery, ExecutableQuery, EstimCost),

  write_list(['\nQuery-Nr:          ', Nr, '\n']),
  write_list(['Secondo-SQL-Query: ', SqlQuery, '\n']),
  write_list(['Executable Query:  ', ExecutableQuery, '\n']),
  write_list(['Estimated Cost:    ', EstimCost, '\n']),
  write_list(['Execution Cost:    ', ExecCost, '\n']).

