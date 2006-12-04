testExamplesV :- 
  setAlgebras,
  secondo('query SEC2OPERATORINFO feed 
  filter[testAlgebras contains .Algebra]
  sortby[Algebra asc, Name asc] project[Name, Example, Result] consume', 
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
  setAlgebras,
  secondo('query SEC2OPERATORINFO feed filter[testAlgebras contains .Algebra]   sortby[Algebra asc, Name asc] project[Name, Example, Result] consume',   
  [_ | [ List | _ ]]), member([Name, Query, Result], List), 
  secondo(Query, [_, ResultQuery]),
  check(Name, Query, Result, ResultQuery), 
  fail.


setAlgebras :-
  secondo('update testAlgebras := [const text value 
	<text>StandardAlgebra, FunctionAlgebra</text--->]'), 
  !.


setAlgebras :-
  secondo('let testAlgebras = [const text value 
	<text>StandardAlgebra, FunctionAlgebra</text--->]').









check(_, _, Result, ResultQuery) :- 
  checkEqual(Result, ResultQuery),
  !.


check(Name, Query, Result, ResultQuery) :-
  nl, write('============================================================='), 
  nl, write('Operator:        '), write(Name), 
  nl, write('Example:         '), write(Query),
  nl, write('ResultExample:   '), write(Result), 
  nl, write('ResultQuery:     '), write(ResultQuery). 




checkEqual(Res1, Res1) :- nl, write('Results are equal'), !.

checkEqual('TRUE', true) :- nl, write('Results are equal'), !.

checkEqual('FALSE', false) :- nl, write('Results are equal'), !.

checkEqual(Res1, Res2) :- term_to_atom(Res2, Res1), nl, write('Results are equal'), !.

checkEqual(Res1, Res2) :- term_to_atom(Res1, Res2), nl, write('Results are equal'), !.

checkEqual(Res1, Res2) :- term_to_atom(Res, Res1), term_to_atom(Res, Res2), nl, write('Results are equal'), !.





