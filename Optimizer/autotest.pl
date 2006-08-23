/* 

28.04.2005 M. Spiekermann

These predictes defined below are used as a small regression test of the
optimizer. However, it will only test if optimization and execution of given
SQL queries is successful. There is no check if the result of the optimization
is correct. 

Currently there are three kinds of tests:

  * ~runExamples/0~ which runs the examples presented in the file optimizer.pl.
Therefore you will need database opt. 

  * ~runTPCExamples/0~ which runs some queries of the TPC-H benchmark. The
queries need the database ~tpc~.

  * ~runGenExamples/0~. This is a stress test for the optimizer since it generates
many variants of possible queries on data of database opt.

Some of the queries need more than the default global and local stack sizes of
SWI-Prolog, hence the optimizer must be started with the following options


---- SecondoPL -G6m -L8m
----

*/

:- ['tpcqueries'].


/*
Top level commands. These can be used to optimize example
queries or to optimize and run them. For the ~standard~ example
queries database ~opt~ and for the TPC-H queries the database
~tpc\_h~ is needed.

*/

runExamples :- runExamples3(_,0).

runTPCExamples :- runTPCExamples2(_,0).

runGenExamples :- runGenExamples2(_,0).

optimizeExamples :-
  sqlExampleList(L),
  optimizeQueries(L).

optimizeGenExamples :-
  genExampleList(L),
  optimizeQueries(L).

optimizeTPCExamples :-
  tpcExampleList(L),
  optimizeQueries(L).

runExamples2(NumOfErrors, N) :- 
  sqlExampleList(List),
  runQueries(List, NumOfErrors, N).

runExamples3(NumOfErrors, N) :- 
  sqlExampleList(List),
  addfirst(List, [], L2),
  runQueries(L2, NumOfErrors, N).

runTPCExamples2(NumOfErrors, N) :-
  tpcExampleList(List),
  runQueries(List, NumOfErrors, N).

runGenExamples2(NumOfErrors, N) :- 
  genExampleList(List),
  addfirst(List, [], L2),
  runQueries(L2, NumOfErrors, N).

tpcExampleList(List) :-
  findall(X, tpcQuery(_,X), List).

sqlExampleList(List) :-
  findall(X, sqlExample(_,X), List).

genExampleList(L) :-
  findall(X, genQuery(X), L).

/*
The predicate ~addfirst/3~ adds a first 3 clause if the
query does not have one. This limits the result set.

*/

addfirst([], A, A).

addfirst([H|T], A, L) :-
  addfirst(H, H1),
  append(A, [H1], R),
  addfirst(T, R, L). 

addfirst(H, H) :- hasFirstOp(H).
addfirst(H, H first 3).

hasFirstOp(_ first _).


showExamples :-
  findall([Nr, Q], sqlExample(Nr, Q), List),
  showQueries(List).

showTPCExamples :-
  findall(X, tpcQuery(_,X), List),
  showQueries(List).

showGenExamples :-
  findall(X, genQuery(X), List),
  showQueries(List).

showQueries(L) :-
  nl, 
  write('List of SQL queries:'), nl, 
  write('===================='),
  showQueries2(L).

showQueries2([]) :- nl .
showQueries2([[Nr, Q]|T]) :-
  nl, write(Nr), write(': '), write(Q), nl, write('--'), showQueries2(T).

/*
The predicated below are useful to open databases from a
bash shell script.

*/

openOpt :-
  open 'database opt'.
openOpt.

openTPC :-
  open 'database tpc_h'.
openTPC.

runQueries(List, NumOfErrors, N) :-
  optimizeQueries(List, NumOfErrors, N, Plans),
  executeQueries(Plans, NumOfErrors, N).

optimizeQueries(InputList) :- 
  optimizeQueries(InputList, _, 0, _).

optimizeQueries(InputList, NumOfErrors, N, PlanList) :- 
  optimizeQueries2(InputList,[], PlanList, [], OptErrList), 
  nl, nl, write('Error Report:'),
  showError('optimization', OptErrList, NumOfErrors, N).

executeQueries(PlanList, NumOfErrors, N) :- 
  runQuery(PlanList, [], ExecErrList),
  nl, nl, write('Error Report:'),
  showError('execution', ExecErrList, NumOfErrors, N).

optimizeQueries2([], A, A, B, B).
optimizeQueries2([H|T], A, N, B, M) :- 
  optimizeQuery(H, A1, A, B1, B),
  optimizeQueries2(T, A1, N, B1, M).

optimizeQuery(H, A1, A, B1, B) :-
  nl, write('SQL: '), write(H), nl,
  ( not(optimize(H))
      *-> nl, write('*** Error: Optimization of query ('),   
          write(H), write(') failed!'), nl, nl, A1 = A, append(B, [H], B1)
        ; append(A, [H], A1), B1 = B ).

runQuery([], A, A).
runQuery([H|T], A, N) :-
  ( not(sql(H))
    *-> nl, write('*** Error: Execution of query ('), 
        write(H), write(') failed!'), nl, nl, append(A, [H], A1)
      ; true ),
  runQuery(T, A1, N).


showError(_, L, N, N) :-
  length(L,0), 
  nl, write('  There were no errors!'), nl, !.

showError(Type, L, NumOfErrors, N) :-
  length(L,N1),
  nl, write('  '), write(Type), write('  There were '), write(N1), write(' errors!'), 
  NumOfErrors is N1 + N,
  showError2(Type, L). 

showError2(_, []).
showError2(Type, [H|T]) :- 
  nl, write('('), write(H), write(')  -- '), write(Type), write(' failed.'),
  nl, write('Query: '), 
  showError2(Type, T).


/*
Generate a number of queries from a given pattern

*/

genAttr(*).
genAttr(count(*)).

sAttr(p:plz).
sAttr(s:sname).

gAttr([p:ort, count(*) as nr]).

whereClause(p:plz < 45678).
whereClause(p:plz = 37263).
whereClause(p:plz = s:plz).

selAttr(A) :- genAttr(A).
selAttr(A) :- sAttr(A).

relations([plz as p, staedte as s]).

query1(select S from Rel) :-
  selAttr(S),
  relations(Rel).

query1(select S from Rel where W) :-
  selAttr(S),
  relations(Rel),
  whereClause(W).

query2(select S from Rel where W) :-
  gAttr(S),
  relations(Rel),
  whereClause(W).

genQuery(X first 3) :-
  query1(X).

genQuery(X orderby p:plz) :-
  query1(X).

genQuery(X orderby p:plz first 3) :-
  query1(X).

genQuery(X groupby p:ort first 3) :-
  query2(X).

genQuery(X) :-
  query1(X).
