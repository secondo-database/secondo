/*
*Specification of Operator Signatures*

RHG, 9.12.2013


\tableofcontents



1 Simple Signatures

Signatures

----	sig(Op, Args, ResultType)
----

are facts describing signatures where ~Op~ is the name of the operator, ~Args~ is a list of argument types, represented as nested lists, and ~ResultType~ is the result type. Argument and result types may contain variables, with a variable ~X\_n~ denoted as [var, X, N], and ~X~ is a type constructor.

The first three signatures are:

---- 	feed: rel(Tuple) -> stream(Tuple)
	filter: stream(Tuple) x (Tuple -> bool) -> stream(Tuple)
	consume: stream(Tuple) -> rel(Tuple)
----

*/

sig(feed,  
  [[rel, [var, tuple, 1]] ], 
  [stream, [var, tuple, 1]]  ).
sig(filter, 
  [ [stream, [var, tuple, 1]], [map, [var, tuple, 1], bool] ], 
  [stream, [var, tuple, 1]]).
sig(consume, 
  [ [stream, [var, tuple, 1]] ], 
  [rel, [var, tuple, 1]]).

sig(+, [int, int], int).
sig(+, [int, real], real).
sig(+, [real, int], real).
sig(+, [real, real], real).

/*
The expression

----	ten feed
----

is represented in Prolog as a list

----	[feed, ten]
----

and the list of argument types for feed determined by the query processor is

---- 	[rel, [tuple, [[no, int]]]]
----

2 Complex Signatures

2.1 General Idea

We now introduce so-called complex signatures:

---- 	csig(Op, Args, ResultType, Decls, Preds)
----

Declarations have the form

----	T in {int, real}

	[[var, t, 1], [int, real]]
----

where ~T~ is a type variable to range over set of types that are enumerated.

Predicates (~Preds~) can be used on the one hand to check argument types,  for example, to check whether an identifier provided is an attribute name, or whether two tuple types have disjoint attributes. They are written like Prolog predicates.

----	attr(Ident, AttrType, AttrNo, Tuple)
	disjoint(Tuple_1, Tuple_2)

	[attr, [var, ident, 1], [var, attrType, 1], 
	  [var, attrNo, 1], [var, tuple, 1]]
	[disjoint, [var, tuple, 1], [var, tuple, 2]]
----

On the other hand, they can be used to compute derived types as well as arguments. Derived types can be specified, for example to describe the result type of a join operation.

----	concat(Tuple_1, Tuple_2, Tuple_3)

	[concat, [var, tuple, 1], [var, tuple, 2], [var, tuple, 3]]
----

Derived arguments can be computed, for example, to determine attribute numbers. This was already shown in the attr example above.

----	attrNos(AttrList, Tuple_1, AttrNos)

	[attrNos, [var, attrList, 1], [var, tuple, 1], [var, attrNos, 1]]
----




2.2 Examples

The operators *attr*, *hashjoin*, and *project* and further operators can then be specified as follows:

----	attr: tuple(Attrs) x Ident -> (Number, Type); 
	  attr(Ident, Attrs, Type, Number);
	  
	hashjoin: stream(tuple(Attrs_1)) x stream(tuple(Attrs_2)) 
		x Ident_1 x Ident_2
		-> stream(tuple(Attrs_3));	  
	    attr(Ident_1, Attrs_1, Type_1, Number_1),
	    attr(Ident_2, Attrs_2, Type_1, Number_2),
	    concat(Attrs_1, Attrs_2, Attrs_3),
            distinctAttrs(Attrs3)

	project: stream(tuple(Attrs_1)) x (Ident_i)+
		-> (Numbers_i, stream(tuple(Attrs_2)));
	  attrs(Ident_i, Attrs_1, Types_i, Numbers_i),
	  combine(Ident_i, Types_i, Attrs_2),
	  distinctAttrs(Attrs_2)

	atinstant: moving(T) x instant -> intime(T) 
	where T in {int, real, string, bool, point, region}.

	deftime: moving(T) -> periods 
	where T in {int, real, string, bool, point, region}.
----

Not yet implemented:

----	extend: stream(Tuple) x (Ident_i x (Tuple -> Type_i))+	
		-> stream(tuple(Attrs_2));
	  Tuple_1 = tuple(Attrs_1),
          combine(Ident_i, Type_i, Attrs_2),
	  concat(Attrs_1, Attrs_2, Attrs_3),
	  distinctAttrs(Attrs_3)

	projectextend: 
	  stream(Tuple) x (Ident_i)+ x (Ident_j x (Tuple -> Type_j))+	
		-> stream(tuple(Attrs_4));
	  Tuple_1 = tuple(Attrs_1),
	  attrs(Ident_i, Attrs_1, Type_i, Number_i),
	  combine(Ident_i, Type_i, Attrs_2),
          combine(Ident_j, Type_j, Attrs_3),
	  concat(Attrs_2, Attrs_3, Attrs_4),
	  distinctAttrs(Attrs_4)

	projectextendstream: 
	  stream(Tuple) x (Ident_i)+ x (Ident x (Tuple -> stream(Type)))	
		-> stream(tuple(Attrs_4));
	  Tuple_1 = tuple(Attrs_1),
	  attrs(Ident_i, Attrs_1, Type_i, Number_i),
	  combine(Ident_i, Type_i, Attrs_2),
          combine(Ident, Type, Attrs_3),
	  concat(Attrs_2, Attrs_3, Attrs_4),
	  distinctAttrs(Attrs_4)

	groupby:
	  stream(Tuple) x (Ident_i)+ 
		x (Ident_j x (rel(Tuple) -> Type_j))+
		-> stream(tuple(Attrs_4));
	  Tuple_1 = tuple(Attrs_1),
	  attrs(Ident_i, Attrs_1, Type_i, Number_i),
	  combine(Ident_i, Type_i, Attrs_2),
          combine(Ident_j, Type_j, Attrs_3),
	  concat(Attrs_2, Attrs_3, Attrs_4),
	  distinctAttrs(Attrs_4)

	nest:
	  stream(tuple(Attrs_1)) x (Ident_i)+ x Ident_2
		-> stream(tuple(Attr_5));
	  minus(Attrs_1, Ident_i, Attrs_3),
	  createAttr(Ident_2, arel(tuple(Attrs3)), Attrs4),
	  concat(Attrs2, Attrs_4, Attrs_5),
	  distinctAttrs(A_5)
	  
	remove:
	  stream(tuple(Attrs_1)) x (Ident_i)+
		-> stream(tuple(Attrs_3));
	 minus(Attrs_1, Ident_i, Attrs_3).

	transformstream:
	  stream(Data) -> stream(tuple([elem: Data]))

	namedtransformstream:
	  stream(Data) x Ident -> stream(tuple([Ident: Data]))


----

2.3 Specifications

We list once more the general form of complex signatures:

----	csig(Op, Args, ResultType, Decls, Preds)
----

2.3.1 *attr*

----	attr: tuple(Attrs) x Ident -> (Number, Type); 
	  attr(Ident, Attrs, Type, Number);
----

*/

csig(attr, 						% Op
  [ [tuple, [any, attrs, 1]], [var, ident, 1] ],	% Args
  [ append, [var, attrNo, 1], [var, attrType, 1] ],	% ResultType
  [],							% Decls
  [ [attr, [var, ident, 1], [var, attrs, 1], 
    [var, attrType, 1], [var, attrNo, 1]] ]). 		% Preds

/*
2.3.2 *hashjoin*
  
----	hashjoin: stream(tuple(Attrs_1)) x stream(tuple(Attrs_2)) 
		x Ident_1 x Ident_2
		-> stream(tuple(Attrs_3));	  
	    attr(Ident_1, Attrs_1, Type_1, Number_1),
	    attr(Ident_2, Attrs_2, Type_1, Number_2),
	    concat(Attrs_1, Attrs_2, Attrs_3),
            distinctAttrs(Attrs3)
----

*/

csig(hashjoin, 						% Op
  [ [stream, [tuple, [any, attrs, 1]]], 
    [stream, [tuple, [any, attrs, 2]]],    
    [var, ident, 1], [var, ident, 2] ],			% Args
  [stream, [tuple, [var, attrs, 3]]],			% ResultType
  [],							% Decls
  [ [attr, [var, ident, 1], [var, attrs, 1], 
      [var, attrType, 1], [var, attrNo, 1]],
    [attr, [var, ident, 2], [var, attrs, 2], 
      [var, attrType, 2], [var, attrNo, 2]],			
    [concat, [var, attrs, 1], [var, attrs, 2], 
      [var, attrs, 3]],
    [distinctAttrs, [var, attrs, 3]] ]).		% Preds

/*
2.3.3 project

----	project: stream(tuple(Attrs_1)) x (Ident_i)+
		-> (Numbers_i, stream(tuple(Attrs_2)));
	  attrs(Ident_i, Attrs_1, Types_i, Numbers_i),
	  combine(Ident_i, Types_i, Attrs_2),
	  distinctAttrs(Attrs_2)
----

*/


csig(project,						% Op
  [ [stream, [tuple, [any, attrs, 1]]], 
    [+, [lvar, ident, i]] ],				% Args
  [ append, [var, numbers, i], 
    [stream,  [tuple, [var, attrs, 2]]] ],		% ResultType
  [],							% Decls
  [ [attrs, [var, ident, i], [var, attrs, 1], 
      [var, types, i], [var, numbers, i]],
    [combine, [var, ident, i], [var, types, i], [var, attrs, 2]],
    [distinctAttrs, [var, attrs, 2]]
  ]).							% Preds



/*
2.3.4 atinstant


----	atinstant: moving(T) x instant -> intime(T) 
	where T in {int, real, string, bool, point, region}.
----

*/

csig(atinstant,						% Op
  [ [moving, [var, t, 1]], instant ],			% Args
  [intime, [var, t, 1]],				% ResultType
  [ [[var, t, 1], [int, real, string, bool, point, 
    region]] ],						% Decls
  []).							% Preds

/*
2.3.4 deftime


----	deftime: moving(T) -> periods 
	where T in {int, real, string, bool, point, region}.
----

*/

csig(deftime,						% Op
  [ [moving, [var, t, 1]] ],				% Args
  periods,						% ResultType
  [ [[var, t, 1], [int, real, string, bool, point, 
    region]] ],						% Decls
  []).							% Preds


/*
3 Implementation

3.1 Typemap


----	typemap(Op, ArgTypes, ResType)
----

The type mapping for ~Op~ applied to the list of types ~ArgTypes~ yields result type ~ResType~.

*/

typemap(Op, ArgTypes, ResType) :-
  sig(Op, Args, Res),
  matches(ArgTypes, Args, Bindings),
  apply(Res, Bindings, ResType).

% version for complex signatures:

typemap(Op, ArgTypes, ResType) :-
  csig(Op, Args, Res, Decls, Preds),
  defineTypeSets(Decls),
  matches(ArgTypes, Args, Bindings),
  evalPreds(Preds, Bindings, Bindings2),
  apply(Res, Bindings2, ResType),
  \+ releaseTypeSets.


defineTypeSets(Decls) :-
  \+ releaseTypeSets,
  defineTypeSets2(Decls).

defineTypeSets2([]).

defineTypeSets2([ [[var, T, N], Types] | Decls]) :-
  assert( typeSet([var, T, N], Types) ),
  defineTypeSets2(Decls).

releaseTypeSets :-
  retract(typeSet(_, _)),
  fail.




/*
3.2 Matches

----	matches(ArgTypes, Args, Bindings)
----

The list of argument types ~ArgTypes~ matches the list of argument type specifications ~Args~ with the bindings ~Bindings~.

*/

% attribute types, e.g. int, bool

matches(Tc, [var, data, N], [[data, N, Tc]]) :-
  kind(data, Types),
  member(Tc, Types),
  !.


matches(Tc, [var, T, N], [[T, N, Tc]]) :-
  typeSet([var, T, N], Types),
  member(Tc, Types),
  !.


% identifiers, e.g. plz, ort

matches(Tc, [var, ident, N], [[ident, N, Tc]]) :-
  atom(Tc),
  !.

% type constructor applied to arguments matches variable

matches([Tc | List], [var, Tc, N], [[Tc, N, [Tc | List]]]) :-
  !.

% free variable matching anything

matches(X, [any, Var, N], [[Var, N, X]]).

% a list of equal types

matches(Args, [+, ArgType], Bindings) :-
  matches2(Args, [+, ArgType], Bindings, 1).


matches([Tc | List], [Tc | Rest], Bindings):-
  matches(List, Rest, Bindings),
  !.

matches([], [], []).

matches([ArgType | ArgTypes], [Arg | Args], Bindings) :-
  matches(ArgType, Arg, B),
  matches(ArgTypes, Args, Bindings1),
  consistent(B, Bindings1, Bindings).


% matching of list variables

matches2([], [+, _], [], _).

matches2([Arg | Args], [+, ArgType], Bindings, N) :-
  N2 is N + 1,
  element(N, ArgType, ArgTypeN),
  matches(Arg, ArgTypeN, B),
  matches2(Args, [+, ArgType], Bindings1, N2),
  consistent(B, Bindings1, Bindings).


% Atomic types belonging to kind DATA

kind(data, [int, real, string, bool]).



/*
----	element(I, Type, Type2)
----


Within nested list ~Type~ rewrite every occurrence of [lvar, Tc, N] into [var, Tc, [N, I]] and return this as ~Type2~.

*/

element(I, [lvar, Tc, N], [var, Tc, [N, I]]) :-
 !.

element(_, [], []).

element(I, [First | Rest], [First2 | Rest2]) :-
  element(I, First, First2),
  element(I, Rest, Rest2),
  !.

element(_, X, X).


% example: matches([klaus, peter, arne, ralf, udo], 
%	[+, [lvar, ident, i]], B).

/*
----	consistent(B1, B2, Bindings)
----

Two lists of bindings ~B1~ and ~B2~ are consistent, if their sets of variables are disjoint or for equal variables they have the same values. The joint bindings are returned in ~Bindings~.

*/

consistent([], B, B).

consistent([B1 | B1s], B2, Bindings) :-
  consistent2(B1, B2, BindingsB1),
  consistent(B1s, B2, BindingsB1s),
  append(BindingsB1, BindingsB1s, Bindings).

consistent2(B1, [], [B1]).

consistent2(B1, [B1 | _], []) :-
  !.

consistent2(B1, [B2 | B2s], BindingsB1) :-
  \+ conflict(B1, B2),
  consistent2(B1, B2s, BindingsB1).


/*
----	conflict(B1, B2)
----

Two bindings ~B1~ and ~B2~ are in conflict if they have the same variable but different values.

*/


conflict([Tc, N, X], [Tc, N, Y]):-
  X \= Y,
  write('Conflict between types '), 
  write(X), write(' and '), write(Y), 
  write(' bound to variable '), write(Tc), write('_'), write(N), nl.

/*
3.3 Evaluation of Predicates

----	evalPreds(Preds, Bindings, Bindings2)
----

Evaluate predicates ~Preds~ based on ~Bindings~, resulting in new ~Bindings2~.

*/

evalPreds([], Bindings, Bindings).

evalPreds([Pred | Preds], Bindings, Bindings3) :-
  evalPred(Pred, Bindings, Bindings2),
  evalPreds(Preds, Bindings2, Bindings3).

evalPred(
  [attr, [var, ident, No1], [var, attrs, No2], 
    [var, attrType, No3], [var, attrNo, No4]],
  Bindings, Bindings3) :-
  bound(Bindings, [var, ident, No1], Ident),
  bound(Bindings, [var, attrs, No2], Attrs),
  isAttr(Ident, Type, No, Attrs),
  addBinding(Bindings, [var, attrType, No3], Type, Bindings2),
  addBinding(Bindings2, [var, attrNo, No4], No, Bindings3).

evalPred(
  [disjoint, [var, tuple, No1], [var, tuple, No2]],
  Bindings, Bindings) :-
  bound(Bindings, [var, tuple, No1], Tuple1),
  bound(Bindings, [var, tuple, No2], Tuple2),
  disjoint(Tuple1, Tuple2).
  
evalPred(
  [concat, [var, attrs, No1], [var, attrs, No2], [var, attrs, No3]],
  Bindings, Bindings2) :-
  bound(Bindings, [var, attrs, No1], List1),
  bound(Bindings, [var, attrs, No2], List2),
  append(List1, List2, List3),
  addBinding(Bindings, [var, attrs, No3], List3, Bindings2).

evalPred(
  [distinctAttrs, [var, attrs, No]],
  Bindings, Bindings) :-
  bound(Bindings, [var, attrs, No], Attrs),
  distinctAttrs(Attrs).

evalPred(
  [attrs, [var, ident, No1], [var, attrs, No2], [var, types, No3], 
	[var, numbers, No4]],
  Bindings, Bindings3) :-
  bound(Bindings, [var, ident, No1], Ident),
  bound(Bindings, [var, attrs, No2], Attrs),
  attrs(Ident, Attrs, Types, Numbers),
  addBinding(Bindings, [var, types, No3], Types, Bindings2),
  addBinding(Bindings2, [var, numbers, No4], Numbers, Bindings3).

evalPred(
  [combine, [var, ident, No1], [var, types, No2], [var, attrs, No3]],
  Bindings, Bindings2) :-
  bound(Bindings, [var, ident, No1], Ident),
  bound(Bindings, [var, types, No2], Types),
  combine(Ident, Types, Attrs),
  addBinding(Bindings, [var, attrs, No3], Attrs, Bindings2).

/*
3.3.1 attr

*/

isAttr(Ident, Type, No, List) :-
  isAttr2(Ident, Type, List, 1, No),
  !.

isAttr(Ident, _, _, Attrs) :-
  write('Error:  attribute '),
  write(Ident), write(' does not occur in attribute list '), 
  write(Attrs), nl,
  fail.

isAttr2(Ident, Type, [ [Ident, Type] | _], N, N) :-
  !.

isAttr2(Ident, Type, [_ | Rest], M, N) :-
  M2 is M + 1, 
  isAttr2(Ident, Type, Rest, M2, N).


/*
3.3.2 attrs

----	attrs(+Ident, +Attrs, -Types, -Numbers)
----

*/

attrs([], _, [], []).

attrs([Ident | Idents], Attrs, [Type | Types], [Number | Numbers]) :-
  isAttr(Ident, Type, Number, Attrs),
  attrs(Idents, Attrs, Types, Numbers).


% [combine, [var, ident, i], [var, types, i], [var, attrs, 2]]

/*
3.3.3 combine

----	combine(+Ident, +Types, -Attrs).
----

*/

combine([], [], []).

combine([Ident | Idents], [Type | Types], 
	[ [Ident, Type] | Attrs]) :-
  combine(Idents, Types, Attrs).


/*
3.3.4 disjoint

*/



attrNames([], []).

attrNames([ [Ident, _] | Rest], [Ident | Names]) :-
  attrNames(Rest, Names).



disjoint(Tuple1, Tuple2) :-
  attrNames(Tuple1, Names1),
  attrNames(Tuple2, Names2),
  disjoint2(Names1, Names2). 

disjoint2([], _).

disjoint2([Name | Rest], Names2) :-
  \+ checkMember(Name, Names2),
  disjoint2(Rest, Names2).

checkMember(Name, Names) :-
  member(Name, Names),
  write('Error: attribute '),
  write(Name), write(' occurs among attributes '), write(Names), nl.

/*
3.3.5 distinctAttrs

*/

distinctList([]).

distinctList([Elem | Rest]):-
  \+ checkMember(Elem, Rest),
  distinctList(Rest).

distinctAttrs(Attrs):-
  attrNames(Attrs, Names),
  distinctList(Names).


/*
3.4 Handling Bindings

----	bound(Bindings, [var, Tc, No], Bound)
	bound(Bindings, [lvar, Tc, No], Bound)
----

The first version finds a binding for a given variable if it exists. The second version is used for list variables. For them, all bindings of the form [Tc, [No, i], X\_i] will be collected into a list [X\_1, ..., X\_n] and be returned in ~Bound~. 

*/

bound([[Tc, [No, X], Y] | Rest], [var, Tc, No], Bound) :-
  !,
  bound2([[Tc, [No, X], Y] | Rest], [var, Tc, No], Bound).

bound([ [Tc, No, X] | _], [var, Tc, No], X) :-
  !.

bound([ _ | Rest], [var, Tc, No], X) :-
  bound(Rest, [var, Tc, No], X).

bound([], [var, Tc, N], _) :-
  write('Error: no binding found for variable '),
  write(Tc), write('_'), write(N), nl.



% for list variables

bound2([], [var, _, _], []).

bound2([ [Tc, [No, _], X] | Rest], [var, Tc, No], [X | Rest2]) :-
  bound2(Rest, [var, Tc, No], Rest2),
  !.

bound2([ _ | Rest], [var, Tc, No], Rest2) :-
  bound2(Rest, [var, Tc, No], Rest2),
  !.

/*
----	addBinding(Bindings, [var, Tc, N], Type, Bindings2)
----

*/

addBinding(Bindings, [var, Tc, N], Type, Bindings2) :-
  consistent([[Tc, N, Type]], Bindings, Bindings2).


/*
3.5 Apply: Computing the Result Type

----	apply(Res, Bindings, ResType)
----

Applying the ~Bindings~ to the result type specification ~Res~ yields the result type ~ResType~.

*/

apply([var, Tc, N], [ [Tc, N, Type] | _], Type):-
  !.

apply([var, Tc, N], [ _ | Rest], Type) :-
  !,
  apply([var, Tc, N], Rest, Type).

apply([var, Tc, N], [], typeerror) :-
  !,
  write('Error: no binding for variable '), 
  write(Tc), write('_'), write(N), write(' found.'), nl.

apply([Tc , List], Bindings, [Tc , Type]) :-
  apply(List, Bindings, Type).

apply(ArgTypes, [], ArgTypes).

apply(ArgTypes, _, ArgTypes) :-
  noVariables(ArgTypes).

apply([append, Extra, Res], B, [append, ExtraArgs, ResultType]) :-
  apply(Extra, B, ExtraArgs),
  apply(Res, B, ResultType).


noVariables([]).

noVariables(X) :-
  atom(X).

noVariables([ [var, _, _] | _]) :-
  !,
  fail.

noVariables([Arg | ArgTypes]) :-
  noVariables(Arg),
  noVariables(ArgTypes).







