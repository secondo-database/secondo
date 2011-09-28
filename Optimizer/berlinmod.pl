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
%:- setOption(rewriteRemove).     % remove unused attributes as soon as possible
:- setOption(earlyproject).      % remove unused attributes as soon as possible
:- setOption(subqueryUnnesting). % fully activate subquery processing

/*
Open the databasae

*/
:- catch(open database berlinmod,_,fail).


/*
Create samples with defined cardinalities to avoid long selectivity queries

*/
:- createSamples(datamctrip, 100, 50).
:- createSamples(datamtrip, 100, 50).
:- createSamples(datasccar, 20, 5).
:- createSamples(datascar, 20, 5).

/*
Create some materialized views:

*/

% define the list of global auxiliary objects as pairs [Name,SQL_create_query]
globalBMODauxObjects([
  [querylicences1, select * from querylicences where id <= 10 orderby id first 10],
  [querylicences2, select * from querylicences where id > 10  orderby id first 10],
  [queryinstants1, select * from queryinstants where id <= 10 orderby id first 10],
  [queryinstants2, select * from queryinstants where id > 10  orderby id first 10],
  [querypoints1,   select * from querypoints   where id <= 10 orderby id first 10],
  [querypoints2,   select * from querypoints   where id > 10  orderby id first 10],
  [queryperiods1,  select * from queryperiods  where id <= 10 orderby id first 10],
  [queryperiods2,  select * from queryperiods  where id > 10  orderby id first 10],
  [queryregions1,  select * from queryregions  where id <= 10 orderby id first 10],
  [queryregions2,  select * from queryregions  where id > 10  orderby id first 10]
  ]).

% create an auxiliary object ~Name~ using the optimized ~Query~. Append the
% command to stream ~S~ (unless ~S~ != none) and execute it, if necessary.
createBMODauxObject(Name, Query, S) :-
  optimize(Query, Plan, Cost),!,
  ( S = none
    -> ( write('## Auxiliary object: '), write(Name), write('\n'),
         write('## SQL: '), write(Query),
         write('\n## Expected Cost: '), write(Cost),
         write('\n## Optimized Plan: let '), write(Name), write(' ='), write(Plan), write(';\n\n')
      )
    ; ( write(S, '## Auxiliary object: '), write(S, Name), write(S, '\n'),
        write(S, '## SQL: '), write(S, Query),
        write(S, '\n## Expected Cost: '), write(S, Cost),
        write(S, '\ndelete '), write(S, Name), write(S, ';\n'),
        write(S, 'let '), write(S, Name), write(S, ' = '),
        write(S, Plan), write(S, ';\n\n')
      )
  ),
  ( secondoCatalogInfo(Name, _, _, _)
    -> write_list(['Object \'', Name, '\' already exists.\n\n'])
    ;  ( write_list(['Creating Object \'', Name, '\' to continue with optimization...']),
         concat_atom([Name, '=', Plan], ' ', Plan2),
         let(Plan2),
         write_list(['finished.\n\n'])
       )
  ),
  !.

% create all global auxiliary objects
createBMODglobalObjects(S) :-
  globalBMODauxObjects(OL),
  member([Name,SQL],OL),
  createBMODauxObject(Name, SQL, S).

:- findall(_,createBMODglobalObjects(none),_).

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

% Query 7 omitted (contains a subquery - does not work yet).
sqlBerlinMOD_R_query(7,
  select [pp:pos as pos, v1:licence as licence]
  from [datascar as v1, querypoints as pp]
  where [ v1:trip passes pp:pos,
          v1:type = "passenger",
          inst(initial(v1:trip at pp:pos)) <= (
            all(
                  select inst(initial(v2:trip at pp:pos))
                  from [datascar as v2]
                  where [v2:trip passes pp:pos, v2:type = "passenger"]
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
          distance(v1:journey,v2:journey)< 3.0) at true)) as pos]
  from [datasccar as v1, datasccar as v2, querylicences1 as ll]
  where [v1:licence = ll:licence,
         not(v2:licence = v1:licence),
         sometimes(distance(v1:journey, v2:journey) < 3.0)]
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
sqlBerlinMOD_R_query(16, nonimplemented) :-
  write_list(['Query 16 is omitted, since its predicate depends on 4 relations',
              ' at once, where only 2 are allowed with the Secondo Optimizer.',
              '\n']).

% Query 17:
sqlBerlinMOD_R_query(17,
  select n:pos
  from poscount as n
  where n:hits = poscounttemp
).


/*
Preparating Steps for Certain Benchmark Queries

*/

% Query 11
prepareSqlBerlinMOD_R_query(11,S) :-
  createBMODauxObject(pointsinstants,
      select [p:pos as pos, i:instant as instant]
      from [querypoints1 as p, queryinstants1 as i]
    ,S),
  !.

% Query 12
prepareSqlBerlinMOD_R_query(12,S) :-
  createBMODauxObject(pointsinstants,
      select [p:pos as pos, i:instant as instant]
      from [querypoints1 as p, queryinstants1 as i]
    ,S),
  !.

% Query 13
prepareSqlBerlinMOD_R_query(13,S) :-
  createBMODauxObject(regionsperiods,
    select [r:region as region, p:period as period]
    from [queryregions1 as r, queryperiods1 as p],
    S),
  !.

% Query 14
prepareSqlBerlinMOD_R_query(14,S) :-
   createBMODauxObject(regionsinstants,
    select [r:region as region, i:instant as instant]
    from [queryregions1 as r, queryinstants1 as i],
    S),
  !.

% Query 15
prepareSqlBerlinMOD_R_query(15,S) :-
   createBMODauxObject(pointsperiods,
    select [p:pos as pos, pp:period as period]
    from [querypoints1 as p, queryperiods1 as pp],
    S),
  !.

% Query 17
prepareSqlBerlinMOD_R_query(17,S) :-
  createBMODauxObject(poscount,
    select [pp:pos as pos, count(c:licence) as hits]
    from [querypoints as pp, datasccar as c]
    where c:journey passes pp:pos
    groupby pp:pos,
    S),
  createBMODauxObject(poscounttemp,
    select max(hits)
    from poscount,
    S),
  !.

% Rule to handle Queries without auxiliary objects.
prepareSqlBerlinMOD_R_query(_,_) :- !, true.


/*
Optimizing and Running the Queries, Creating a script

*/

:- assert(helpLine(optAllBMOD,0, [],
    'Optimize (but not execute) all BerlinMOD queries.')).
:- assert(helpLine(optBMOD,1,
    [[+,'QueryNumber','The number of the BerlinMOD query to optimize.']],
    'Optimize (but not execute) a single BerlinMOD query.')).
% optimize a given BerlinMod Query
optBMOD(Nr) :- optBMOD(Nr, none).

% optimize a given BerlinMod Query and additionally write all executable queries
% to a stream/ file ~S~
optBMOD(Nr, S) :-
  sqlBerlinMOD_R_query(Nr, SqlQuery),
  ( S = none
    -> ( write('# ======================================================='),
         write('======\n'),
         write_list(['\nQuery-Nr:          ', Nr, '\n'])
       )
    ;  ( write(S, '# ===================================================='),
         write(S, '=========\n'),
         write(S, '# BerlinMOD Query '), write(S, Nr), write(S, '\n'),
         write(S, '#    Deleting old result:\n'),
         write(S, '     delete BMODres'), write(S, Nr), write(S, ';\n')
       )
  ),
  prepareSqlBerlinMOD_R_query(Nr, S),
  catch(
    optimize(SqlQuery, ExecutableQuery, ExpectedCost),
    Exception,
    ( ( S = none
        -> ( write('\n\nError during optimization of this query:\n\t'),
            write(Exception),
            write('\n\n')
          )
        ;  ( write(S, '# Error during optimization of this query: '),
            write(S, Exception),
            write(S, '\n\n')
          )
      ),
      fail
    )
  ),
  ( S = none
    -> ( write_list(['Secondo-SQL-Query: ', SqlQuery, '\n']),
         write_list(['Executable Query:  ', ExecutableQuery, '\n']),
         write_list(['Estimated Cost:    ', ExpectedCost, '\n'])
       )
    ;  (
         write(S, '# SQL: '),write(S, SqlQuery), write(S, '\n'),
         write(S, '# Expected Cost: '), write(S, ExpectedCost), write(S, '\n'),
         write(S, 'let BMODres'), write(S, Nr), write(S, ' = '),
         write(S, ExecutableQuery), write(S, ';\n\n')
       )
  ).

:- assert(helpLine(runBMOD,1,
    [[+,'Queries','Number (or a list of such) of the BerlinMOD query to run.']],
    'Optimize and execute a single BerlinMOD query.')).
:- assert(helpLine(runALLBMOD,0, [],
    'Optimize and execute all BerlinMOD queries.')).

optAllBMOD :-
  findall(Nr,catch((sqlBerlinMOD_R_query(Nr,_), optBMOD(Nr)),_,true),L),
  write_list(['Optimized queries: ', L, '.\n']).

% Run a given BerlinMOD Query
runBMOD(Nr) :-
  sqlBerlinMOD_R_query(Nr, SqlQuery),
  prepareSqlBerlinMOD_R_query(Nr, none),
  writeln('=================================================================='),
  write('Query No: '), write(Nr), nl,
  write(SqlQuery), nl, nl, nl,
  sql SqlQuery.

runBMOD(QL) :-
  is_list(QL),
  findall( Nr, catch(( member(Nr, QL), runBMOD(Nr)), _, fail ), PosQL),
  write_list(['\n\nSuccessful queries: ', PosQL,'\n\n']).

runAllBMOD :-
  findall(Nr,catch((sqlBerlinMOD_R_query(Nr,_), runBMOD(Nr)),_,true),L),
  write_list(['Executed queries: ', L, '.\n']).






:- assert(helpLine(createBMODqueries,0,[],
    'Create a script of optimized executable BerlinMOD queries.')).
:- assert(helpLine(createBMODqueries,1,
    [[+,'QueryNumberList','A list of Query Numbers to optimize.']],
    'Create a script of optimized executable BerlinMOD queries.')).
% Create a script to run the optimized BerlinMOD queries
createBMODqueries(QueryNumberList) :-
  open('optimized_BerlinMOD_queries.SEC', write, FD),
  write(FD, '# Automatically generated file, do not edit by hand.\n'),
  write(FD, '# ============================================================\n'),
  write(FD, 'open database berlinmod;\n\n'),
  write(FD, '# ============================================================\n'),
  write(FD, '# Creating global auxiliary objects: \n\n'),
  findall(_,createBMODglobalObjects(FD),_),!,
  write(FD, '# ============================================================\n'),
  write(FD, '# START OF OPTIMIZED BERLINMOD QUERIES\n'),
  findall( Nr, ( member(Nr,QueryNumberList),
                sqlBerlinMOD_R_query(Nr,_),
                optBMOD(Nr, FD) ), QL ),!,
  write(FD, '# ============================================================\n'),
  write(FD, '# END OF OPTIMIZED BERLINMOD QUERIES\n'),
  write(FD, '# Successfully optimized queries: '),write(FD,QL),write(FD,'.\n'),
  write(FD, '# ============================================================\n'),
  write(FD, 'close database;\n\n'),
  write('\n\n>>> Optimized queries have been written to file '),
  close(FD),
  writeln('\'optimized_BerlinMOD_queries.SEC\'. <<<\n').

% Create a script with all optimizable queries:
createBMODqueries :-
  findall(Nr, sqlBerlinMOD_R_query(Nr,_), QL),
  createBMODqueries(QL).