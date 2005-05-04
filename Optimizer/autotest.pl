/* 

28.04.2005 M. Spiekermann

These predictes defined below are used as a small regression test of the
optimizer. However, it will only test if optimization and execution of given
SQL queries is successful. There is no check if the result of the optimization
is correct. 

The test is invoked by the goal ~optimizeTest~. You will need the databases
~opt~ and ~tpc~.

*/


optimizeExamples :- optimizeExamples2(_,0).

optimizeExamples2(NumOfErrors, N) :- 
  secondo('open database opt'),
  findall(X, sqlExample(_,X), List),
  nl, write('Testing '), write(List), nl,
  optimizeAndExecute(List, NumOfErrors, N).

optimizeTPC :- optimizeTPC(_,0).

optimizeTPC2(NumOfErrors, N) :-
  secondo('open database tpc_h'),
  findall(X, tpcQuery(_,X), List),
  nl, write('Testing '), write(List), nl,
  optimizeAndExecute(List, NumOfErrors, N).

optimizeAndExecute(InputList, NumOfErrors, N) :- 
  optimizeQueries2(InputList,[], ResultList, [], OptErrList), 
  runQuery(ResultList, [], ExecErrList),
  nl, nl, write('Error Report:'),
  showError('optimization', OptErrList, NumOfErrors, N),
  nl, nl, 
  showError('execution', ExecErrList, NumOfErrors, N).

optimizeQueries2([], A, A, B, B).
optimizeQueries2([H|T], A, N, B, M) :- 
  optimizeQuery(H, A1, A, B1, B),
  optimizeQueries2(T, A1, N, B1, M).

optimizeQuery(H, A1, A, B1, B) :-
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
  nl, write('There were no errors!'), nl, !.

showError(Type, L, NumOfErrors, N) :-
  length(L,N1),
  nl, write('  '), write(Type), write(' - There were '), write(N1), write(' errors!'), 
  NumOfErrors is N1 + N,
  showError2(Type, L). 

showError2(_, []).
showError2(Type, [H|T]) :- 
  nl, write('('), write(H), write(')  -- '), write(Type), write(' failed.'),
  nl, write('Query: '), 
  showError2(Type, T).


