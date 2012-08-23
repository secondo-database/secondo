/*
  $Header$
  @author Nikolai van Kempen

	Utility predicates that are NEEDED for the regular program
	execution.
*/

/*
	This works like the assertion predicate, it has no effect if 
	Goal could been proven, if not, a exception is thrown.
	I would prefer assertion, but it runs under SWI-Prolog 5.10.4
	under some circumstances into an infinite loop.
*/
%ensure(+Goal)
ensure(Goal) :-
  Goal, 
	!.

ensure(Goal) :- 
	!,
	Msg='The Goal could not been proven, but it needs to be provable.',
  throw(error_Internal(putil_ensure(Goal))::Msg).

%ensure(+Goal, +Exception)
ensure(Goal, _) :-
  Goal,
  !.

ensure(_, Exception) :-
  !,
  throw(Exception).


/*
Returns a property value within the list, could be done with the memory predicate, too(but then with backtracking).
Note that properties are terms with one argument.

Example:
?- propertyValue(prop, [a(b), c, prop(test), xy,xy2(b)], V).
V = test .

*/
propertyValue(Property, [E], Value) :-
  !,
  E =.. [Functor, Arg1],
  Property=Functor,
  Value=Arg1.

propertyValue(Property, [E|_], Value) :-
  propertyValue(Property, [E], Value), !.

propertyValue(Property, [_|Rest], Value) :-
  propertyValue(Property, Rest, Value).

/*
	Rouding/ceiling of float lists.
*/
%roundListToInts(+FloatList, -IntegerList)
roundListToInts([], []).
roundListToInts([F|FRest], [I|IRest]) :-
  I is round(F),
  roundListToInts(FRest, IRest).

%roundListToInts(+FloatList, -IntegerList)
ceilingListToInts([], []).
ceilingListToInts([F|FRest], [I|IRest]) :-
  I is ceiling(F),
  ceilingListToInts(FRest, IRest).

/*
Counts how often the goal ~Goal~ is provable.
A simpler alternative is 
  findall(x, Goal, L),
  length(L, Count).
but this might create huge lists.
*/
count(Goal, Count) :-
  Counter = counter(0),
  (
    Goal,
    arg(1, Counter, CC),
    NewC is CC+1,
    nb_setarg(1, Counter, NewC),
    fail
  ;         
    Counter=counter(Count)
  ).

/* 
Calls assertz for every element within the list.
*/
assertzall([]) :- 
	!.

assertzall([F|Rest]) :-
  assertz(F),
  assertzall(Rest).

% A list containing a list of facts.
assertzalllist([]) :- 
	!.

assertzalllist([List|Rest]) :- 
	!,
  assertzall(List),
  assertzalllist(Rest).

/*
Calls the predicate P/1 for every element within the list.
*/
forAllIn(_, []) :- !.
forAllIn(P, [E|Rest]) :-
  call(P, E),
  forAllIn(P, Rest).

/*
Like append/3 all lists within the first parameter list are added
to the result list.
*/
appendLists([], []).
appendLists([List|RestLists], ResultList) :-
  appendLists(RestLists, RestResultList),
  append(List, RestResultList, ResultList).

% eof
