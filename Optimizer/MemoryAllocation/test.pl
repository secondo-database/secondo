/*
$Header$
@author Nikolai van Kempen

Just some simple test predicates to illustrate how the form looks like.
The assumed global memory for these examples are 512 MiB.
*/

exampleFunction1([X1], 100000/(0.01*X1)).
exampleFunction2([X1], 123456/(0.01*X1)).
exampleFunction3([X1], 200000/(0.011*X1)).

% The exact result should assign the maximum memory to X1.
% Currently it is 1 MiB. Therfore, the exact result is [511, 1]
testMA(1, [X1, X2], 2000-(2*X1) + 1000-X2).

% Exact result is [256, 256]
testMA(2, [X, Y], 1/X+1/Y).

% A degenerated cost function, this is not allowed(non decreasing), but 
% nevertheless, it works.
% Exact result is [1, 1]
testMA(100, [X, Y], X+Y).

% Another non allowed formula coz the result may be below zero.
testMA(101, [X, Y], 100-X+200-Y).

testMASimple(No) :-
	testMA(No, V, F),
	matest(V, F).
	
testMASimple :-
	testMASimple(_),
	fail.

/*
Testing predicate for a given formular.

Only for non-gradient based algorithm.

motest(+VarList, +Formula)
*/
matest(VarList, Formula) :-
	nl,
 	setFormula(VarList, Formula),
  maFormula(VarList, _),
  length(VarList, Len),
  write_list(['Variables    : ', VarList, '\n']),
  write_list(['Formuala     : ', Formula, '\n']),
  getTime(memoryOptimization(16, 512, Len, [50, 100], X), TimeMS),
  write_list(['Result of opt: ', X, '\n']),
  write_list(['Time         : ', TimeMS, 'ms\n']),
  maFormula(X, F2),
	Costs is F2,
  write_list(['Total costs  : ', Costs, '\n']).
	

testMAQuery(1, [], select (*)from[staedte, plz as p1, plz as p2, plz as p3]where[sname=p1:ort, p1:plz=p2:plz+1, p2:plz=p3:plz*5] first 1).
testMAQuery(2, [], select (*)from[staedte, plz as p1, plz as p2, plz as p3]where[sname=p1:ort, p1:plz=p2:plz+1, p2:plz=p3:plz*5] orderby[sname] first 1).
testMAQuery(3, [], select (*)from[staedte, plz as p1, plz as p2, plz as p3, plz as p4]where[sname=p1:ort, p1:plz=p2:plz, p2:plz=p3:plz, p3:plz=p4:plz] orderby[sname] first 1).
testMAQuery(4, [], select (*)from[staedte, plz as p1, plz as p2, plz as p3, plz as p4, plz as p5]where[sname=p1:ort, p1:plz=p2:plz, p2:plz=p3:plz, p3:plz=p4:plz, p4:plz=p5:plz] orderby[sname] first 1).
testMAQuery(5, [], select (*)from[staedte, plz as p1, plz as p2, plz as p3, plz as p4, plz as p5, plz as p6]where[sname=p1:ort, sname="xy", p1:plz=p2:plz, p2:plz=p3:plz, p3:plz=p4:plz, p4:plz=p5:plz, p5:plz=p6:plz] orderby[sname] first 1).


testMA :-
  reset,
	delOption(nestedRelations),
	setOption(memoryAllocation),
  retractall(testResult(_, _, _, _, _)),
  (
    testMAQuery(No, Properties, Query),
    % So we check if this is provable, not if the result is correct.
    (getTime(sql(Query), TimeMS) ->
      addResult(Properties, No, TimeMS, ok)
    ;
      addResult(Properties, No, -1, failed)
    ),
    fail
  ).

testMA :-
  !,
  showTestResults.

% eof
