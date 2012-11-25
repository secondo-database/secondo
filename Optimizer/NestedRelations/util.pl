/*
$Header$
@author Nikolai van Kempen

Utility predicates that are NEEDED for the regular program execution.
*/

/*
This works like the assertion predicate, it has no effect if Goal could been proven, if not, a exception is thrown.  I would prefer assertion, but it runs under SWI-Prolog 5.10.4 under some circumstances into an infinite loop.
ensure(+Goal)
*/
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

%ceilingListToInts(+FloatList, -IntegerList)
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

/*
Summarize all list elements.
*/
listSum([], 0).
listSum([Value|Rest], Sum) :-
	listSum(Rest, RSum),
	Sum is Value+RSum.

/*

listMax(+MinValue, +ValueList, -ResultList)
*/
listMax(_Min, [], []) :-
	!.
listMax(Min, [Value|Rest], ResultList) :-
	listMax(Min, Rest, RestResult),
	Result is max(Min, Value),
	append([Result], RestResult, ResultList),
	!.


/*
Creates a list with all the same elements
*/
listFix(0, _Value, []) :-
	!.
listFix(Len, Value, [Value|Rest]) :-
	!,
	NextLen is Len - 1,
	listFix(NextLen, Value, Rest).

/*
Like makeList, but this behavior is better when errors results into backtracking because the error can be better identified.
*/
btMakeList(L, L) :- 
	is_list(L),
	!.

btMakeList(L, [L]) :- 
	\+ is_list(L),
	!.

/*
Calls the goal ~Goal~ ~Count~ Times and returns all exec times within 
a list.
xcall(+Goal, +Count, -Times)

Example:
?- xcall(sleep(3), 0, 10, Times).
Times = [3000, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 3000].

Note that the goal needs to stable in way thata a nonvar(X) term X in Goal produces every time the same results. Hence, 
xcall((A is random(10)), 0, 10, Times)
will fail.
*/
xcall(_Goal, _Sleep, 0, []) :- 
  !.

xcall(Goal, Sleep, Count, Result) :-
  Count>0,
	(Sleep =\= 0 -> sleep(Sleep) ; true),
	garbage_collect,
  getTime(once(Goal), Time),
  CountNew is Count - 1,
  xcall(Goal, Sleep, CountNew, List), 
  append(List, [Time], Result).

/*
Special version if the predicate delivers the time ifself.
The time must be the last attribut that is not within ~Goal~
because it will later be added by the call predicate.
*/
xcallAddTime(_Goal, _Sleep, 0, []) :-
  !.

xcallAddTime(Goal, Sleep, Count, Result) :-
  Count>0,
  (Sleep =\= 0 -> sleep(Sleep) ; true),
  garbage_collect,
	call(Goal, Time),
  CountNew is Count - 1,
  xcallAddTime(Goal, Sleep, CountNew, List),
  append(List, [Time], Result).

/*
like atomic_list_concat/3 but here terms a converted to atoms, if needed.
*/
term_list_concat(TList, Sep, A) :-
	!,
  termlist_to_atomlist(TList, AList),
  atomic_list_concat(AList, Sep, A).

termlist_to_atomlist([], []) :-
	!.

termlist_to_atomlist([T|TRest], [T|ARest]) :-
	atomic(T),
	!,
	termlist_to_atomlist(TRest, ARest).

termlist_to_atomlist([T|TRest], [A|ARest]) :-
	\+ atomic(T),
	!,
	termlist_to_atomlist(TRest, ARest),
	term_to_atom(T, A).

/*
Determines the index of the smallest or highest number within the list.
find_extreme_value0(+Type, +List, ?Index)
*/
find_extreme_value0(Type, List, Index) :- 
	!,
	find_extreme_value0(Type, List, 0, Index, _Value).

find_extreme_value0(_Type, [V], Index, Index, V) :- 
	!.

find_extreme_value0(min, [A,B|Rest], CIx, Ix, CVal) :-
  CIx2 is CIx + 1,
  find_extreme_value0(min, [B|Rest], CIx2, IxR, CValR),
  (A<CValR ->
    (Ix=CIx, CVal=A)
  ;
    (Ix=IxR, CVal=CValR)
  ).

find_extreme_value0(max, [A,B|Rest], CIx, Ix, CVal) :-
  CIx2 is CIx + 1,
  find_extreme_value0(max, [B|Rest], CIx2, IxR, CValR),
  (A>CValR ->
    (Ix=CIx, CVal=A)
  ;
    (Ix=IxR, CVal=CValR)
  ).
	
/*
Removes the smallest or highest number within the list.
remove_extreme_value(Type, +List, -NewList)
*/
remove_extreme_value(_Type, [], []) :-
	!.

remove_extreme_value(Type, List, RList) :- 
	List \= [],
	!,
	find_extreme_value0(Type, List, Index),
	once(split_at_index(List, Index, HList, [_|TList])),
	append(HList, TList, RList).
	
/*
Splits the list at a given index starting at 0.

split_at_index(+List, +Index, ?HeadList, ?TailList) 
*/
split_at_index(List, 0, [], List) :-
	!.

split_at_index([Head|Tail], Index, [Head|HeadList], TailList) :-
	Index > 0,
	!,
  NewIndex is Index - 1,
  split_at_index(Tail, NewIndex, HeadList, TailList).

/*
Replaces a variable within an arbitrary term.
replaceVar(+TermIn, +VarOld, +VarNew, -TermOut)
*/
replaceVar(Term, _VarOld, _VarNew, Term) :-
	\+ compound(Term),
	nonvar(Term).

replaceVar(Term, VarOld, VarNew, VarNew) :-
	var(Term),
	Term == VarOld.
replaceVar(Term, VarOld, _VarNew, Term) :-
	var(Term),
	Term \== VarOld.

replaceVar([], _VarOld, _VarNew, []) :-
	!.
replaceVar([A|Rest], VarOld, VarNew, [AT|RestT]) :-
	replaceVar(A, VarOld, VarNew, AT),
	replaceVar(Rest, VarOld, VarNew, RestT),
	!.
replaceVar(TermIn, VarOld, VarNew, TermOut) :-
	compound(TermIn),
	TermIn =..[F|List],
	replaceVar(List, VarOld, VarNew, ListNew),
	TermOut =..[F|ListNew],
	!.

% eof
