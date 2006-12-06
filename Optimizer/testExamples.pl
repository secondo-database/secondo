selftest :-
  prepare,
  testExamples.

selftestV :-
  prepare,
  testExamplesV.

testExamplesV :-
  secondo('query SEC2OPERATORINFO feed TestAlgebras feed 
  hashjoin[Algebra, Alg, 9997] sortby[AlgNo asc, Name asc] 
  project[Name, Example, Result] consume', 
  [_ | [ List | _ ]]), member([Name, Query, Result], List), 
  nl, write('============================================================='), 
  nl, write('Operator:        '), write(Name), 
  nl, write('Example:         '), write(Query),
  nl, write('ResultExample:   '), write(Result), 
	secondo(Query, [_, ResultQuery]),
  nl, write('ResultQuery:     '), write(ResultQuery), 
  checkEqual(Result, ResultQuery), 
  fail.




testExamples :-
  secondo('query SEC2OPERATORINFO feed TestAlgebras feed 
  hashjoin[Algebra, Alg, 9997] sortby[AlgNo asc, Name asc] 
  project[Name, Example, Result] consume',   
  [_ | [ List | _ ]]), 
  member([Name, Query, Result], List),
  secondo(Query, [_, ResultQuery]),
  checkResults(Name, Query, Result, ResultQuery), 
  fail.



prepare :- 
  databaseName('berlintest'),
  !.

prepare :-
  notIsDatabaseOpen,
  secondo('open database berlintest'),
  !.

prepare :-
  isDatabaseOpen,
  secondo('close database'),
  secondo('open database berlintest').



checkResults(_, _, Result, ResultQuery) :- 
  checkEqual(Result, ResultQuery),
  !.


checkResults(Name, Query, Result, ResultQuery) :-
  nl, write('============================================================='), 
  nl, write('Operator:        '), write(Name), 
  nl, write('Example:         '), write(Query),
  nl, write('ResultExample:   '), write(Result), 
  nl, write('ResultQuery:     '), write(ResultQuery). 




checkEqual(Res1, Res1) :- nl, write('Results are equal'), !.

checkEqual('TRUE', true) :- nl, write('Results are equal'), !.

checkEqual('FALSE', false) :- nl, write('Results are equal'), !.

checkEqual(Res1, Res2) :- term_to_atom(Res, Res1), term_to_atom(Res, Res2), nl, write('Results are equal').






