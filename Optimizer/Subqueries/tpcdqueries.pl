/*

----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

August 2008, Burkart Poneleit. Initial Version

This file provides a framework to execute TPC-Benchmarks. All queries for the benchmark
should be asserted as facts tpc(Benchmark, QueryDefinition). Currently this file contains
the definition of all TPC-D queries.

Before executing a benchmark, the predicate initialize has to be called exactly once to
create the result relation. The result relation will contain the runtime of all intermediate
steps of the optimizer, total runtime for the query and flags, which indicate whether the
step of the optimizer succeeded or failed.

Executing a benchmark is as simple as calling the predicate

---- executeBenchmark(Benchmark)
----

where Benchmark is d, h or whatever TPC-Benchmark you want to run. On finishing the
benchmark run an additional entry is created in the result relation to give you the total
runtime for the benchmark run.

---- executeSingleQuery(Benchmark, QueryNumber)
----

To just run a single query and still get the corresponding results such as runtime for the
intermediate steps of the optimizer, you can use the predicate executeSingleQuery/2

*/
:- dynamic(queryValidation/0).

:- assert(queryValidation).

/*

To skip certain queries when executing a benchmark, a fact

---- skipQuery(Benchmark, QueryNumber)
----

can be asserted.

*/

:- dynamic(skipQuery/2).

:- assert(skipQuery(d, 1)),
   assert(skipQuery(d, 3)),
   assert(skipQuery(d, 5)),
   assert(skipQuery(d, 6)),
   assert(skipQuery(d, 8)),
   assert(skipQuery(d, 10)),
   assert(skipQuery(d, 11)),
   assert(skipQuery(d, 12)),
   assert(skipQuery(d, 13)),
   assert(skipQuery(d, 14)),
   assert(skipQuery(d, 15)),
   assert(skipQuery(d, 18)),
   assert(skipQuery(d, 19)).

/*

Results of the benchmark runs are kept in a relation in the database. Standard name for this
relation is benchmarkResult. This can be changed by retracting the fact resultRelation and
reasserting it with a different name. To create the relation the predicate initialize has to
be called. It creates the relation and inserts a "Dummy" entry, as correct handling of empty relation
in Secondo optimizer is still wonting.

*/
:- dynamic(resultRelation/1).

:- assert(resultRelation(benchmarkResult)).

:- dynamic(maxRunTime/1).

initialize :-
  resultRelation(Rel),
  my_concat_atom([Rel, ' = [const rel(tuple([Query: string,',
                                  'Setup: bool, ',
                  'Rewrite: bool, ',
                  'Lookup: bool, ',
                  'QueryToPlan: bool, ',
                  'planToAtom: bool, ',
                  'Execute: bool, ',
                  'Teardown: bool, ',
                  'runtime_Rewrite: duration, ',
                  'runtime_Lookup: duration, ',
                  'runtime_QueryToPlan: duration, ',
                  'runtime_planToAtom: duration, ',
                  'runtime_Execute: duration, ',
                  'runtime_Total: duration, ',
                  'Date: instant]))',
                  ' value (("Dummy" FALSE FALSE FALSE ',
                  'FALSE FALSE FALSE FALSE (0 0) (0 0) (0 0) ',
                  '(0 0) (0 0) (0 0) -1))]'],
                  Query),
  let(Query).

initialize :-
  nl, write('Please assert resultRelation(<RelName>)').

time(Begin, Duration) :-
  get_time(End),
  Time is End - Begin,
  convert_time(Time, _, _, _, Hour, Minute, Sec, MilliSec),
  convert_time(0, _, _, _, H, _, _, _),
  Hour1 is Hour - H,
  Duration is (Hour1 * 3600000) + (Minute * 60000) + (Sec * 1000) + MilliSec.

tpc(Benchmark, No) :-
  skipQuery(Benchmark, No),
  upcase_atom(Benchmark, BMOut),
  my_concat_atom(['\nTPC-', BMOut, ' ', No, '\n\tSkipped'], Result),
  my_concat_atom([Benchmark, No], Key),
  ( retractall(benchmarkResult(Key, _)) ; true ),
  assert(benchmarkResult(Key, Result)).

tpc(Benchmark, No) :-
  resultRelation(Rel),
  upcase_atom(Benchmark, BMOut),
  my_concat_atom(['"TPC-', BMOut, ' ', No, '"'], QueryName),
  my_string_to_atom(QueryString, QueryName),
  sql insert into Rel values [QueryString, false, false, false, false,
    false, false, false, create_duration(0, 0), create_duration(0, 0),
    create_duration(0, 0), create_duration(0, 0), create_duration(0, 0),
    create_duration(0, 0), instant(0)],
  get_time(BeginAll),
  catch( ((setupQuery(Benchmark, No) )
     -> ( SetupResult = '', sql update Rel set [setup = true]
          where [date = instant(0), (query) = QueryString] )
    ;  my_concat_atom(['\tSetup failed', '\n'], SetupResult)
   ),
   _, my_concat_atom(['\tSetup failed', '\n'], SetupResult)),
  tpc(Benchmark, No, Query),
  catch( (ground(Query), get_time(BeginRewrite), (rewriteQuery(Query, RQuery))
    -> ( RewriteResult = '', time(BeginRewrite, TRewrite),
        sql update Rel set [rewrite = true,
          runtime_Rewrite = create_duration(0, TRewrite)]
        where [date = instant(0), (query) = QueryString] )
    ;  my_concat_atom(['\tRewrite failed', '\n'], RewriteResult)
   ),
   _, my_concat_atom(['\tRewrite failed', '\n'], RewriteResult)),
  catch( ((ground(RQuery), get_time(BeginLookup), callLookup(RQuery, Query2))
  -> ( LookupResult = '', time(BeginLookup, TLookup),
        sql update Rel set [lookup = true,
          runtime_Lookup = create_duration(0, TLookup)]
        where [date = instant(0), (query) = QueryString] )
    ;  my_concat_atom(['\tLookup failed', '\n'], LookupResult)
   ),
   _, my_concat_atom(['\tLookup failed', '\n'], LookupResult)),
  !,
  catch( ((ground(Query2), get_time(BeginQTP), queryToPlan(Query2, Plan, _))
    -> ( QueryToPlanResult = '', time(BeginQTP, TQTP),
        sql update Rel set [queryToPlan = true,
          runtime_QueryToPlan = create_duration(0, TQTP)]
        where [date = instant(0), (query) = QueryString] )
    ;  my_concat_atom(['\tQueryToPlan failed', '\n'], QueryToPlanResult)
   ),
   _, my_concat_atom(['\tQueryToPlan failed', '\n'], QueryToPlanResult)),
  !,
  catch( ((ground(Plan), get_time(BeginPTA), plan_to_atom(Plan, QueryOut))
    -> ( PlanToAtomResult = '', time(BeginPTA, TPTA),
        sql update Rel set [planToAtom = true,
          runtime_PlanToAtom = create_duration(0, TPTA)]
        where [date = instant(0), (query) = QueryString] )
    ;  my_concat_atom(['\tPlanToAtom failed', '\n'], PlanToAtomResult)
   ),
   _, my_concat_atom(['\tPlanToAtom failed', '\n'], PlanToAtomResult)),
  (( ground(QueryOut), get_time(BeginExec),
    my_concat_atom(['query ', QueryOut], QueryText) ) ; true),
  catch( ((ground(QueryOut), secondo(QueryText))
    -> ( ExecuteResult = '', time(BeginExec, TExec),
        sql update Rel set [execute = true,
          runtime_Execute = create_duration(0, TExec)]
        where [date = instant(0), (query) = QueryString] )
    ;  my_concat_atom(['\tExecute failed', '\n'], ExecuteResult)
   ),
   _, my_concat_atom(['\tExecute failed', '\n'], ExecuteResult)),
  catch( (teardownQuery(Benchmark, No)
    -> ( TeardownResult = '',
        sql update Rel set [teardown = true]
          where [date = instant(0), (query) = QueryString] )
    ;  my_concat_atom(['\tTeardown failed', '\n'], TeardownResult)
   ),
   _, my_concat_atom(['\tTeardown failed', '\n'], TeardownResult)),
   my_concat_atom(['\nTPC-', BMOut, ' ', No, '\n',
                SetupResult,
                RewriteResult  ,
                LookupResult  ,
                QueryToPlanResult  ,
                PlanToAtomResult  ,
                ExecuteResult  ,
                TeardownResult],
                Result),
   time(BeginAll, TotalTime),
   sql update Rel set [runtime_Total = create_duration(0, TotalTime)]
    where [date = instant(0), (query) = QueryString],
   my_concat_atom([Benchmark, No], Key),
   ( retractall(benchmarkResult(Key, _)) ; true ),
   assert(benchmarkResult(Key, Result)).

tpc(d, No, Query) :- tpcd(No, Query).

executeSingleQuery(Benchmark, No) :-
  executeQuery(Benchmark, No),
  resultRelation(Rel),
  !,
  let('tempXXXXXXX = now()'),
  !,
  sql update Rel set [date = tempXXXXXXX] where [date = instant(0)],
  !,
  delete(tempXXXXXXX).

executeQuery(Benchmark, No) :-
  tpc(Benchmark, No).

executeQueries(_, []).

executeQueries(Benchmark, [[No, _] | Rest]) :-
  executeQuery(Benchmark, No),
  % dc(tpcd, (
  % N is No + 1,
  % write_list(['\nExecute next Query (', N, ')? y/n']),
  % get_single_char(Answer),
  % Answer = 121 )),
  executeQueries(Benchmark, Rest).

executeBenchmark(Benchmark) :-
    get_time(Begin),
    resultRelation(Rel),
    sql delete from Rel where date = instant(0),
    setupBenchmark(Benchmark),
    retractall(benchmarkResult(_, _)),
  findall([No, Query], tpc(Benchmark, No, Query), Queries),
  executeQueries(Benchmark, Queries),
  !,
  time(Begin, Total),
    upcase_atom(Benchmark, BMOut),
    my_concat_atom(['"TPC-', BMOut, '"'], BMName),
    my_string_to_atom(BMString, BMName),
  let('tempXXXXXXX = now()'),
    sql insert into Rel values [BMString, false, false, false, false,
      false, false, false, create_duration(0, 0), create_duration(0, 0),
      create_duration(0, 0), create_duration(0, 0), create_duration(0, 0),
      create_duration(0, Total), instant(0)],
  sql update Rel set [date = tempXXXXXXX] where [date = instant(0)],
  delete(tempXXXXXXX),
  findall(Result, benchmarkResult(_, Result), L),
  nl,
    write_list(L).

/*

Run Tpcd Queries and export Benchmark results to csv.

*/

runAnalysis(Database) :-
  ( cdb ; true ),
  odb(Database),
  ( initialize ; true ),
  sql(delete from benchmarkResult where (query) contains "T"),
  ( delete(tempXXXXXXX) ; true ),
  !,
  setOption(subqueryUnnesting),
  executeBenchmark(d),
  !,
  executeBenchmark(d),
  !,
  delOption(subqueryUnnesting),
  executeBenchmark(d),
  !,
  executeBenchmark(d),
  !,
  setOption(subqueryUnnesting),
  retractall(skipQuery(d, 15)),
  executeSingleQuery(d, 15),
  !,
  executeSingleQuery(d, 15),
  !,
  delOption(subqueryUnnesting),
  executeSingleQuery(d, 15),
  !,
  executeSingleQuery(d, 15),
  !,
  assert(skipQuery(d, 15)),
  setOption(subqueryUnnesting),
  get_time(A),
  convert_time(A, Y, M, D, _, _, _, _),
  my_concat_atom(['benchmarkResult feed csvexport[\'Analyse ',
    Database, '-', Y, '-', M, '-', D, '.csv\',
    FALSE, TRUE, ";"] count'], Query),
  write('ExportQuery: '), write(Query), nl,
  !,
  query(Query).

/*

Any operations which must happen before a query is run
should be coded as a predicate. The predicate is automatically
called when using executeSingleQuery/2 or runBenchmark/1.
Currently used to emulate views for Query 15 of TPC-D benchmark.

---- setupQuery(Benchmark, QueryNumber)
----

*/

setupQuery(d, 15) :-
    ( teardownQuery(d, 15) ; true ),
    let(revenue, select
    [lsuppkey as supplier_no,
    sum(lextendedprice * (1 - ldiscount)) as total_revenue]
    from
    lineitem
    where
    [lshipdate >= instant("1996-01-01"),
    lshipdate < theInstant(
      year_of(instant("1996-01-01")),
      month_of(instant("1996-01-01")) + 3,
      day_of(instant("1996-01-01")))]
    groupby
    [lsuppkey]).

setupQuery(_,_).

/*

Any operations to clean up after a query should be coded as a predicate.
Currently used to emulate views for Query 15 of TPC-D benchmark.

---- teardownQuery(Benchmark, QueryNumber)
----

*/

teardownQuery(d, 15) :-
  catch(drop_relation(revenue), _, true).

teardownQuery(_, _).

/*

The idea behind setupBenchmark/1 is to assert all facts for substitution parameters
and to determine the substitution parameters at runtime iff queryValidation is asserted.
If queryValidation is asserted the substitution parameters are the values for query
validation according to the benchmark.

*/

setupBenchmark(d) :-
  not(queryValidation),
  retractall(region(_,_)),
  assert(region(0, "AFRICA")),
  assert(region(1, "AMERICA")),
  assert(region(2, "ASIA")),
  assert(region(3, "EUROPE")),
  assert(region(4, "MIDDLE EAST")),
  retractall(typeSyllable1(_,_)),
  assert(typeSyllable1(0, "STANDARD")),
  assert(typeSyllable1(1, "SMALL")),
  assert(typeSyllable1(2, "MEDIUM")),
  assert(typeSyllable1(3, "LARGE")),
  assert(typeSyllable1(4, "ECONOMY")),
  assert(typeSyllable1(5, "PROMO")),
  retractall(typeSyllable2(_,_)),
  assert(typeSyllable2(0, "ANODIZED")),
  assert(typeSyllable2(1, "BURNISHED")),
  assert(typeSyllable2(2, "PLATED")),
  assert(typeSyllable2(3, "POLISHED")),
  assert(typeSyllable2(4, "BRUSHED")),
  retractall(typeSyllable3(_,_)),
  assert(typeSyllable3(0, "TIN")),
  assert(typeSyllable3(1, "NICKEL")),
  assert(typeSyllable3(2, "BRASS")),
  assert(typeSyllable3(3, "STEEL")),
  assert(typeSyllable3(4, "COPPER")).

setupBenchmark(_).

/*

The Queries defined for TPC-D Benchmark. Queries with syntax currently
not understood are commented out.

*/

tpcd(1, select
      [lreturnflag,
      llinestatus,
      sum(lquantity) as sum_qty,
      sum(lextendedprice) as sum_base_price,
      sum(lextendedprice*(1-ldiscount)) as sum_disc_price,
      sum(lextendedprice*(1-ldiscount)*(1+ltax)) as sum_charge,
      avg(lquantity) as avg_qty,
      avg(lextendedprice) as avg_price,
      avg(ldiscount) as avg_disc,
      count(*) as count_order]
      from
      lineitem
      where
% substitution parameter
      [lshipdate <= create_instant(instant2real(instant("1998-12-01")) - Delta)]
%      [lshipdate <= instant("1998-09-02")]
      groupby
      [lreturnflag,
      llinestatus]
      orderby
      [lreturnflag,
      llinestatus]) :-
% Delta is randomly selected within [60 - 120]
  queryValidation
  -> ( Delta is 90.0 )
  ; ( Delta is random(60.0) + 60.0 ).

tpcd(2, select
    [sacctbal,
    sname,
    nname,
    ppartkey,
    pmfgr,
    saddress,
    sphone,
    scomment]
    from
    [part,
    supplier,
    partsupp,
    nation,
    region]
    where
    [ppartkey = pspartkey,
    ssuppkey = pssuppkey,
% substitution parameter
%     psize = 15,
         psize = Size,
% substitution parameter
%        ptype like "%BRASS"
%     ptype contains "BRASS",
         ptype contains Type,
     snationkey = nnationkey,
     nregionkey = rregionkey,
% substitution parameter
%     rname = "EUROPE",
         rname = Region,
     pssupplycost = (
      select
      min(ps:pssupplycost)
      from
      [partsupp as ps, supplier as s,
      nation as n, region as r]
      where
      [ppartkey = ps:pspartkey,
       s:ssuppkey = ps:pssuppkey,
       s:snationkey = n:nnationkey,
       n:nregionkey = r:rregionkey,
  % substitution parameter
%       rname = "EUROPE"]
       r:rname = Region]
    )]

    orderby[
    sacctbal desc,
    nname,
    sname,
    ppartkey] first 100) :-
  queryValidation
  -> ( Size is 15,
       Type = "BRASS",
     Region = "EUROPE" )
  ; ( Size is random(49) + 1,
      T is random(5),
      typeSyllable3(T, Type),
      R is random(4),
      region(R, Region)).


tpcd(3, select
    [lorderkey,
    sum(lextendedprice*(1-ldiscount)) as revenue,
    oorderdate,
    oshippriority]
    from
    [customer,
    orders,
    lineitem]
    where
% substitution parameter
    [cmktsegment = "BUILDING",
    ccustkey = ocustkey,
    lorderkey = oorderkey,
% substitution parameter
    oorderdate < instant("1995-03-15"),
% substitution parameter
    lshipdate > instant("1995-03-15")]
    groupby
    [lorderkey,
    oorderdate,
    oshippriority]
    orderby
    [revenue desc,
    oorderdate] first 10).

tpcd(4, select
    [oorderpriority,
    count(*) as ordercount]
    from orders
    where
% substitution parameter
    [oorderdate >= instant("1993-07-01"),
% substitution parameter
    oorderdate < theInstant(
      year_of(instant("1993-07-01")),
      month_of(instant("1993-07-01")) + 3,
      day_of(instant("1993-07-01"))),
    exists(
    select
    *
    from
    lineitem
    where
    [lorderkey = oorderkey,
    lcommitdate < lreceiptdate])
    ]
    groupby
    [oorderpriority]
    orderby
    [oorderpriority]).

tpcd(5, select
    [nname,
    sum(lextendedprice * (1 - ldiscount)) as revenue]
    from
    [customer,
    orders,
    lineitem,
    supplier,
    nation,
    region]
    where
    [ccustkey = ocustkey,
    oorderkey = lorderkey,
    lsuppkey = ssuppkey,
    cnationkey = snationkey,
    snationkey = nnationkey,
    nregionkey = rregionkey,
% substitution parameter
    rname = "ASIA",
% substitution parameter
    oorderdate >= instant("1994-01-01"),
% substitution parameter
    oorderdate < theInstant(
      year_of(instant("1994-01-01")) + 1,
      month_of(instant("1994-01-01")) ,
      day_of(instant("1994-01-01")))]
    groupby
    [nname]
    orderby
    [revenue desc]).

tpcd(6, select
    [sum(lextendedprice*ldiscount) as revenue]
    from
    lineitem
    where
% substitution parameter
    [lshipdate >= instant("1994-01-01"),
% substitution parameter
    lshipdate < theInstant(
      year_of(instant("1994-01-01")) + 1,
      month_of(instant("1994-01-01")) ,
      day_of(instant("1994-01-01"))),
% substitution parameter
    between(ldiscount, 0.06 - 0.01, 0.06 + 0.01),
% substitution parameter
    lquantity < 24]
    groupby[]).

tpcd(7, select
    [supp_nation,
    cust_nation,
    lyear, sum(volume) as revenue]
    from (
      select
      [n1:nname as supp_nation,
      n2:nname as cust_nation,
      year_of(lshipdate) as lyear,
      lextendedprice * (1 - ldiscount) as volume]
      from
      [supplier,
      lineitem,
      orders,
      customer,
      nation as n1,
      nation as n2]
      where
      [ssuppkey = lsuppkey,
      oorderkey = lorderkey,
      ccustkey = ocustkey,
      snationkey = n1:nnationkey,
      cnationkey = n2:nnationkey,
% substitution parameter
      (n1:nname = "FRANCE" and n2:nname = "GERMANY")
% substitution parameter
      or (n1:nname = "GERMANY" and n2:nname = "FRANCE"),
      between(instant2real(lshipdate),
              instant2real(instant("1995-01-01")),
              instant2real(instant("1996-12-31")))]
    ) as shipping
    groupby
    [supp_nation,
    cust_nation,
    lyear]
    orderby
    [supp_nation,
    cust_nation,
    lyear]).

/*

Q8 does not work, arbitrary expressions over aggregations
cannot be translated at the moment.

*/

tpcd(8, select
    [oyear,
/*    sum(case
    when nation = "BRAZIL"
    then volume
    else 0
    end) / sum(volume) as mktshare */
    (aggregate((ifthenelse(nation = "BRAZIL", volume, 0.0)),
              (+),
              'real',
              const(real,0.0)) / sum(volume)) as mktshare
    ]
    from (
      select
      [year_of(oorderdate) as oyear,
      lextendedprice * (1-ldiscount) as volume,
      n2:nname as nation]
      from
      [part,
      supplier,
      lineitem,
      orders,
      customer,
      nation as  n1,
      nation as n2,
      region]
      where
      [ppartkey = lpartkey,
      ssuppkey = lsuppkey,
      lorderkey = oorderkey,
      ocustkey = ccustkey,
      cnationkey = n1:nnationkey,
      n1:nregionkey = rregionkey,
% substitution parameter
      rname = "AMERICA",
      snationkey = n2:nnationkey,
      between(instant2real(oorderdate),
              instant2real(instant("1995-01-01")),
              instant2real(instant("1996-12-31"))),
% substitution parameter
      ptype = "ECONOMY ANODIZED STEEL"]
    ) as allnations
    groupby
    [oyear]
    orderby
    [oyear]).

tpcd(9, select
    [nation,
    oyear,
    sum(amount) as sumprofit]
    from (
      select
      [nname as nation,
      year_of(oorderdate) as oyear,
      lextendedprice * (1 - ldiscount) - pssupplycost * lquantity as amount]
      from
      [part,
      supplier,
      lineitem,
      partsupp,
      orders,
      nation]
      where
      [ssuppkey = lsuppkey,
      pssuppkey = lsuppkey,
      pspartkey = lpartkey,
      ppartkey = lpartkey,
      oorderkey = lorderkey,
      snationkey = nnationkey,
% substitution parameter
      pname contains "green"]
    ) as profit
    groupby
    [nation,
    oyear]
    orderby
    [nation,
    oyear desc]).


tpcd(10, select
    [ccustkey,
    cname,
    sum(lextendedprice * (1 - ldiscount)) as revenue,
    cacctbal,
    nname,
    caddress,
    cphone,
    ccomment]
    from
    [customer,
    orders,
    lineitem,
    nation]
    where
    [ccustkey = ocustkey,
    lorderkey = oorderkey,
    oorderdate >= instant("1993-10-01"),
    oorderdate < theInstant(
      year_of(instant("1993-10-01")),
      month_of(instant("1993-10-01")) + 3,
      day_of(instant("1993-10-01"))),
    lreturnflag = "R",
    cnationkey = nnationkey]
    groupby
    [ccustkey,
    cname,
    cacctbal,
    cphone,
    nname,
    caddress,
    ccomment]
    orderby
    [revenue desc] first 20).

/*

Q11 is only partially working, implementation for the
having clause is still missing.

*/

tpcd(11, select
    [pspartkey,
    (sum(pssupplycost * psavailqty)) as value]
    from
    [partsupp,
    supplier,
    nation]
    where
    [pssuppkey = ssuppkey,
    snationkey = nnationkey,
% substitution parameter
    nname = "GERMANY"]
    groupby
    [pspartkey]
/*    having
    [sum(pssupplycost * psavailqty) > (
        select
  % substitution parameter
        [sum(pssupplycost * psavailqty * 0.0001)]
        from
        [partsupp,
        supplier,
        nation]
        where
        [pssuppkey = ssuppkey,
        snationkey = nnationkey,
        nname = "GERMANY"]
      )
    ]*/
    orderby
    [value desc]).

tpcd(12, select
/*  sum(case
    when oorderpriority ="1-URGENT"
    or oorderpriority ="2-HIGH"
    then 1
    else 0
    end) as highlinecount,
    sum(case
    when oorderpriority <> "1-URGENT"
    and oorderpriority <> "2-HIGH"
    then 1
    else 0
    end) as lowlinecount*/
    [lshipmode,
    aggregate((ifthenelse(oorderpriority = "1-URGENT" or
                          oorderpriority = "2-HIGH", 1, 0)),
              (+), 'int', [const,int,value,0]) as highlinecount,
    aggregate((ifthenelse(not(oorderpriority = "1-URGENT") and
                          not(oorderpriority = "2-HIGH"), 1, 0)),
              (+), 'int', [const,int,value,0]) as lowlinecount
    ]
    from
    [orders,
    lineitem]
    where
    [oorderkey = lorderkey,
% substitution parameter
    lshipmode in ("MAIL", "SHIP"),
%        lshipmode = "MAIL" or lshipmode = "SHIP",
    lcommitdate < lreceiptdate,
    lshipdate < lcommitdate,
% substitution parameter
    lreceiptdate >= instant("1994-01-01"),
% substitution parameter
    lreceiptdate < theInstant(
      year_of(instant("1994-01-01")) + 1,
        month_of(instant("1994-01-01")),
          day_of(instant("1994-01-01")))]
    groupby
    [lshipmode]
    orderby
    [lshipmode]).


/*

Q13 does not work, no implementation of left outerjoin

*/

tpcd(13, _).

/*

tpcd(13, select
    [ccount, count(*) as custdist]
    from (
      select
      [ccustkey,
      count(oorderkey)]
      from
      customer left outer join orders on
      [ccustkey = ocustkey
      and ocomment not like "%[WORD1]%[WORD2]%"]
      groupby
      [ccustkey]
    )as corders (ccustkey, ccount)
    groupby
    [ccount]
    orderby
    [custdist desc,
    ccount desc]).

Q14Does not work, arbitrary expressions over aggregation aren't translated

*/


tpcd(14, select

/*  [100.00 * sum(case
    when ptype like "PROMO%"
    then lextendedprice*(1-ldiscount)
    else 0
    end) / sum(lextendedprice * (1 - ldiscount)) as promo_revenue]*/
    [(aggregate(((ifthenelse(ptype starts "PROMO",
                    lextendedprice*(1-ldiscount), 0)) * 100.0),
      (+), 'real', [const,real,value,0.0])
      / sum(lextendedprice * (1 - ldiscount))) as promo_revenue]
    from
    [lineitem,
    part]
    where
    [lpartkey = ppartkey,
    lshipdate >= instant("1995-09-01"),
    lshipdate < theInstant(
      year_of(instant("1995-09-01")),
      month_of(instant("1995-09-01")) + 1,
      day_of(instant("1995-09-01")))]
    groupby[]).

/*

Implementation only partially correct. The view is emulated by a
temporary relation.

*/

tpcd(15, % Query) :-
/*  create view revenue[STREAM_ID] (supplier_no, total_revenue) as
select
l_suppkey,
sum(l_extendedprice * (1 - l_discount))
from
lineitem
where
l_shipdate >= date "[DATE]"
and l_shipdate < date "[DATE]" + interval "3" month
group by
l_suppkey;*/
select
    [ssuppkey,
    sname,
    saddress,
    sphone,
    total_revenue]
    from
    [supplier,
    revenue/*[STREAMID]*/]
    where
    [ssuppkey = supplier_no,
    total_revenue = (
      select
      max(r:total_revenue)
      from
      revenue as r/*[STREAMID]*/
    )]
    orderby
    [ssuppkey])
/* drop view revenue[STREAM_ID]; */.

tpcd(16, select
    [pbrand,
    ptype,
    psize,
    count(distinct pssuppkey) as suppliercnt]
    from
    [partsupp,
    part]
    where
    [ppartkey = pspartkey,
% substitution parameter
    not(pbrand = "Brand#45"),
% substitution parameter
    not(ptype starts "MEDIUM POLISHED"),
% substitution parameters
    psize in (49, 14, 23, 45, 19, 3, 36, 9),
    pssuppkey not in(
      select
      ssuppkey
      from
      supplier
      where
%      scomment like "%Customer%Complaints%"
            [scomment contains "Customer",
      scomment contains "Complaints"]
    )]
    groupby
    [pbrand,
    ptype,
    psize]
    orderby
    [suppliercnt desc,
    pbrand,
    ptype,
    psize]).

tpcd(17, select
    [sum(lextendedprice / 7.0) as avg_yearly]
    from
    [lineitem,
    part]
    where
    [ppartkey = lpartkey,
    pbrand = "Brand#23",
    pcontainer = "MED BOX",
    lquantity < (
      select
      avg(0.2 * l1:lquantity)
      from
      [lineitem as l1]
      where
      [l1:lpartkey = ppartkey]
    )]
    groupby
    []).

/*

Q18 is only partially working, implementation of the having clause is still
missing.

*/

tpcd(18, select
    [cname,
    ccustkey,
    oorderkey,
    oorderdate,
    ototalprice,
    sum(lquantity) as quant]
    from
    [customer,
    orders,
    lineitem]
    where
    [oorderkey in (
      select
      lorderkey
      from
      lineitem
      /* groupby
      [lorderkey having
% substitution parameter
      sum(lquantity) > 300] */
    ),
    ccustkey = ocustkey,
    oorderkey = lorderkey]
    groupby
    [cname,
    ccustkey,
    oorderkey,
    oorderdate,
    ototalprice]
    orderby
    [ototalprice desc,
    oorderdate]).

tpcd(19, select
    [(sum(lextendedprice * (1 - ldiscount) )) as revenue]
    from
    [lineitem,
    part]
    where
    [((
      (
        (
          (
            (
              (
                (
                  (
                    (ppartkey = lpartkey)
% substitution parameter
                    and (pbrand = "Brand#12")
                  )
                 and (pcontainer in ( "SM CASE", "SM BOX", "SM PACK", "SM PKG"))
                )
% substitution parameter
                and (lquantity >= 1)
              )
% substitution parameter
              and (lquantity <= 1 + 10)
            )
            and (between(psize, 1, 5))
          )
          and (lshipmode in ("AIR", "AIR REG"))
        )
      and (lshipinstruct = "DELIVER IN PERSON")
      )
    )
    or
    (
      (
        (
          (
            (
              (
                (
                  (
                    (ppartkey = lpartkey)
% substitution parameter
                    and (pbrand = "Brand#23")
                  )
               and (pcontainer in ("MED BAG", "MED BOX", "MED PKG", "MED PACK"))
                )
% substitution parameter
                and (lquantity >= 10)
              )
% substitution parameter
              and (lquantity <= 10 + 10)
            )
            and (between(psize, 1, 10))
          )
          and (lshipmode in ("AIR", "AIR REG"))
%          and (lshipmode = "AIR" or lshipmode = "AIR REG")
        )
      and (lshipinstruct = "DELIVER IN PERSON")
      )
    ))
    or
    (
      (
        (
          (
            (
              (
                (
                  (
                    (ppartkey = lpartkey)
% substitution parameter
                    and (pbrand = "Brand#34")
                  )
                 and (pcontainer in ( "LG CASE", "LG BOX", "LG PACK", "LG PKG"))
                )
% substitution parameter
                and (lquantity >= 20)
              )
% substitution parameter
              and (lquantity <= 20 + 10)
            )
            and (between(psize, 1, 15))
          )
          and (lshipmode in ("AIR", "AIR REG"))
        )
      and (lshipinstruct = "DELIVER IN PERSON")
      )
    )]
    groupby[]).

tpcd(20, select
    [sname,
    saddress]
    from
    [supplier, nation]
    where
    [ssuppkey in (
      select
      [pssuppkey]
      from
      [partsupp]
      where
      [pspartkey in (
        select
        ppartkey
        from
        part
        where
%        pname like '[COLOR]%'
% substitution parameter
        tostring(pname) starts "forest"
      ),
      psavailqty > (
        select
        sum(lquantity * 0.5)
        from
        lineitem
        where
        [lpartkey = pspartkey,
        lsuppkey = pssuppkey,
% substitution parameter
        lshipdate >= instant("1994-01-01"),
% substitution parameter
        lshipdate < theInstant(
          year_of(instant("1994-01-01")) + 1,
          month_of(instant("1994-01-01")),
          day_of(instant("1994-01-01")))]
      )]
    ),
    snationkey = nnationkey,
% substitution parameter
    nname = "CANADA"]
    orderby
    [sname]).


tpcd(21, select
    [sname,
    count(*) as numwait]
    from
    [supplier,
    lineitem as l1,
    orders,
    nation]
    where
    [ssuppkey = l1:lsuppkey,
    oorderkey = l1:lorderkey,
    oorderstatus = "F",
    l1:lreceiptdate > l1:lcommitdate,
    exists(
      select
      *
      from
      lineitem as l2
      where
      [l2:lorderkey = l1:lorderkey,
      not(l2:lsuppkey = l1:lsuppkey)]
    ),
    not(exists(
      select
      *
      from
      lineitem as l3
      where
      [l3:lorderkey = l1:lorderkey,
      not(l3:lsuppkey = l1:lsuppkey),
      l3:lreceiptdate > l3:lcommitdate]
    )),
    snationkey = nnationkey,
% substitution parameter
    nname = "SAUDI ARABIA"]
    groupby
    [sname]
    orderby
    [numwait desc,
    sname]
    first 100).

tpcd(22, select
    [cntrycode,
    count(*) as numcust,
    sum(cacctbal) as totacctbal]
    from (
      select
      [substr(cphone, 1, 2) as cntrycode,
      cacctbal]
      from
      customer
      where
      [substr(cphone, 1, 2) in
% substitution parameters
      ("13","35","31","23","29","30","18"),
      cacctbal > (
        select
        avg(c1:cacctbal)
        from
        customer as c1
        where
        [c1:cacctbal > 0.00,
        substr(c1:cphone, 1, 2) in
% substitution parameter
        ("13","35","31","23","29","30","18")]
      ),
      not( exists(
        select
        *
        from
        orders
        where
        ocustkey = ccustkey
      ))]
    ) as custsale
    groupby
    [cntrycode]
    orderby
    [cntrycode]).
