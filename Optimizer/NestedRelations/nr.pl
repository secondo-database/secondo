/*
$Header$
@author Nikolai van Kempen

This files provides different functions to work with nested relations within the optimizer.
*/

/* 
Checks if DCRel is a rel of the type nrel. Can be used to enumerate the nested relation objects within the current database.
is_nrel(?DCRel)
*/
is_nrel(DCRel) :-
  secondoCatalogInfo(DCRel,_,_,[[nrel, _]]).

/*
The list elements needs to be in external spelling.
*/
getArelUnnestAtom([], '').
getArelUnnestAtom([A|REST], STRING) :-
  getArelUnnestAtom(REST, RSTRING),
  atomic_list_concat([' unnest[', A, ']', RSTRING], '', STRING).

/*

*/
concatAttributes([], '').
% Note: this to avoid a underscore if we don't want it.
concatAttributes([A], AttrDC) :-
  dcName2externalName(AttrDC, A).
concatAttributes([A|REST], OUT) :-
  dcName2externalName(AttrDC, A),
  concatAttributes(REST, ROUT),
  OUT=AttrDC:ROUT. % : is a op of type xfy, so we can test for subrelation with OUT = _ : _ : _

/*
Atts has to be append this way or the operator type has to be changed from xfy to yfx, but this wouldn't be good. Then it is no longer possible we separate the elements from left to right. You can do this: a:b or a:(b:c) but NOT: (a:b):c. Then we can't obtain the first element with A : _. Note that we consider a "x:y..."-builded term as a list from left to right and not as a tree!
---- 
appendAttribute(+A, -T)
----
*/
appendAttribute(A, T) :-
	 attAppend(A, T).
appendAttribute(A:B, T) :-
	 attAppend(A:B, T).

appendAttribute(A, [], A) :- !.
appendAttribute([], A, A) :- !.
appendAttribute(A, B, T) :-
	 attAppend(A:B, T).

attAppend(A, T) :-
  attsToList(A, LST),
  attAppendLst(LST, T).

appendAttributeList(A, B) :- 
	attAppendLst(A, B).

attAppendLst([A], A) :- !.
attAppendLst([A|REST], T) :-
  attAppendLst(REST, T2),
  T = A:T2.

attsToList(A, _) :-
  var(A), !, 
  ErrMsg='Variables are not allowed here.',
  throw(error_Internal(nestedrelations_attsToList(A,_)::ErrMsg)).

attsToList(A:B, LST) :-
  attsToList(A, LST1),
  attsToList(B, LST2),
  append(LST1, LST2, LST), !.
attsToList(A, [A]) :-
  A \== _:_, !.


/*
lastAttribute(+, ?)
*/
lastAttribute(A, A) :-
	atomic(A).

lastAttribute(A:B, Result) :-
	% If this failed, A is something like x:y, note that (x:y):b is not allowed.
	assertion(atomic(A)),
	lastAttribute(B, Result).

/*
removeLastAttribute(A, B)
*/

% NVK NOTE: Fails on length(A, 1) coz there is no such thing as [] for our
% : mechanism.
/*removeLastAttribute(A, T) :-
  attsToList(A, LST),
	removeLast(LST, LST2),
  attAppendLst(LST2, T).
*/

removeLastAttribute([], _) :- !, fail.
removeLastAttribute(A, []) :-
	atomic(A).
removeLastAttribute(A, B) :-
	A=A1:A2,
	removeLastAttribute(A2, B1),
	appendAttribute(A1, B1, B).

% Removes the last element from the list.
removeLast([_], []) :- !.
removeLast([X|A], [X|B]) :-
	removeLast(A, B).

/*
downcaseAttributeList(A, B) :-
	atomic(A),!, downcase_atom(A, B),!.
downcaseAttributeList(A1:B1, A2:B2) :-
	downcase_atom(A1, A2),
	downcaseAttributeList(B1, B2).
*/
downcaseAttributeList(A, B) :-
	applyOnAttributeList(downcase_atom, A, B).

% Eval's a predicate P/2 on every list element.
applyOnAttributeList(P, A1:B1, A2:B2) :-
	call(P, A1, A2),!,
	applyOnAttributeList(P, B1, B2).
applyOnAttributeList(P, A, B) :-
	call(P, A, B),!.

downcaseList([], []) :- !.
downcaseList([A], [B]) :-
  downcase_atom(A, B),!.
downcaseList([A|RA], [B|RB]) :-
  downcase_atom(A, B),
  downcaseList(RA, RB).

/*
Returns in $Result the first element in $List that unifies with $S.
findfirst(+S,+List,-Result)
*/
findfirst(_, [], _) :- !, fail.
findfirst(X, [X|_], X) :- !.
findfirst(X, [Y|Rest], R) :-
  X \= Y, !,
  findfirst(X, Rest, R).

/*
Checks if every element within the list is atomic.
*/
atomicCheck([]).
atomicCheck([A|Rest]) :-
  assertion(atomic(A)),
  atomicCheck(Rest).

/*
Extracts the relations from a query.
getRelsFromQuery(+Query, ?Rels)
Rels might be a list or direct the rel term.
*/
getRelsFromQuery(Query first _, Rels) :-
	!, getRelsFromQuery(Query, Rels).
getRelsFromQuery(Query last _, Rels) :-
	!, getRelsFromQuery(Query, Rels).
getRelsFromQuery(Query orderby _, Rels) :-
	!, getRelsFromQuery(Query, Rels).
getRelsFromQuery(Query groupby _, Rels) :-
	!, getRelsFromQuery(Query, Rels).

getRelsFromQuery(Query, Rels) :-
  Query =.. [from, _, Where],
  Where =.. [where, Rels, _], !.

getRelsFromQuery(select _ from Rels, Rels) :- !.

getRelsFromQuery(Query, Rels) :-
	!, 
  ErrMsg='Query not in expected format.',
  throw(error_Internal(nestedrelations_getRelsFromQuery(Query,_)::ErrMsg)).

/*
Extracts the attributes from a query.
getAttsFromQuery(+Query, ?Atts)
*/
getAttsFromQuery(Query, Atts) :-
  Query =.. [from, Select, _],
	Select =.. [select, Atts],
	!.

getAttsFromQuery(Query, Atts) :-
  !,
  ErrMsg='Query not in expected format.',
  throw(error_Internal(nestedrelations_getAttsFromQuery(Query, Atts)::ErrMsg)).

/* 
Calls assertz for every element within the list.
*/
assertzall([]) :- !.
assertzall([F|Rest]) :- 
	assertz(F),
	assertzall(Rest).

% A list containing a list of facts.
assertzalllist([]) :- !.
assertzalllist([List|Rest]) :- !,
	assertzall(List),
	assertzalllist(Rest).

/*
Determine the cardinality for arel attributes.
Need if a arel apears within the from clause of a subquery.
Because there is no exact cardinality for arel attributes, and
this needed cardinality depending on the selectivitiy of the 
outer query, i just made the very simple an inexact aproximation
that the cardinality is the number of all rows within the arel
relation divided by the rows within the outer relation.
*/
cardNR(Var:NRel, Size) :-
  %This is only possible during the lookup and not later.
  %Inspect the findBinding predicate for further information.
  attributeToFQN(Var:NRel, DCFQN, _),
	% Note that for DCFQN card/2 can't be called again. 
  databaseName(DB),
	cardByFQN(DB, DCFQN, Size),
	dm(nr, ['\ncardinality for: ', Var:NRel, ' is: ', Size]).

cardNR(arel(_, DCFQN, _, _), Size) :-
  databaseName(DB),
  cardByFQN(DB, DCFQN, Size),
  dm(nr, ['\ncardinality for: ', DCFQN, ' is: ', Size]).

% Open issue: card's for arels that are created during the query.
cardByFQN(_, DCFQN, 50) :-
	DCFQN=queryAttr(_).
cardByFQN(_, DCFQN, 50) :-
	DCFQN=queryAttr(_):_.

cardByFQN(DB, DCFQN, Size) :-
	DCFQN=Outer:_,!,
	%writefacts(storedCard), sleep(8),
  storedCard(DB, DCFQN, ARelSize),
  cardByFQN(DB, Outer, OuterSize),
  Size is ARelSize / OuterSize.

cardByFQN(_, DCFQN, Size) :-
	atomic(DCFQN), !, % So DCFQN is a dc-relation identifier.
	card(DCFQN, Size).

/*
Predicates to support nested optimizer calls as needed by the subquery extension.

	Errors like

The following Unclassified Error was caught: '_G565'.
The reason is unknown. Please carefully check the error message to trace the problem.

ERROR: Unknown message: nostream::[rel(o:subrel,p)]where[pr(attr(p:o:kennzeichen,1,u)=value_expr(string,[72]),rel(o:subrel,p)),pr(attr(p:o:bevT,1,u)<value_expr(int,1000000),rel(o:subrel,p))]

	indicates often a optimizer call whithin the optimize process without saving and restoring	
	the optimizer state.
*/

:- dynamic(optimizerState/1).

% Fix that node may not registered yet as dynamic.
:- 
	dynamic 
		node/3,
		edge/6,
		argument/2,
		planEdge/4.

/* 
Mainly we have to take care about the dynamic facts, handeld within the pog predicate.
*/
putOptimizerState :-
	dm(nr, ['\nStore current optimizer state...']),
	%sleep(5),writeOptimizerState,
 	findall(argument(V1, V2), argument(V1, V2), L4),
 	findall(node(V1, V2, V3), node(V1, V2, V3), L2),
 	findall(edge(V1, V2, V3, V4, V5, V6), edge(V1, V2, V3, V4, V5, V6), L3),
 	findall(varDefined(V1), varDefined(V1), L5),
 	findall(nCounter(nodeCtr, V1), nCounter(nodeCtr, V1), L1),
 	findall(highNode(V1), highNode(V1), L6),
 	findall(planEdge(V1, V2, V3, V4), planEdge(V1, V2, V3, V4), L7),
	
	asserta(optimizerState([L1, L2, L3, L4, L5, L6, L7])).
	
restoreOptimizerState :-
	dm(nr, ['\nRestore last optimizer state...']),
	% Get the last stored state
	retract(optimizerState(List)),
	% Get rid of the current facts from the last call.
	clearOptimizerState,
	% Insert now all facts...
	assertzalllist(List).

clearOptimizerState :-
	deleteArguments,
  deleteNodes,
  deleteEdges,
  deleteVariables,
 	deleteCounters,
  deletePlanEdges,
  retractall(highNode(_)).

optimizerFacts([nCounter, node, edge, argument, 
	varDefined, highNode, planEdge]).

writeOptimizerState :-
	write('\nCurrent optimizer state:\n'), 
	optimizerFacts(FList),
	forAllIn(writefacts, FList),
	write('\nStored optimizer states:\n'), 
	writefacts(optimizerState).

forAllIn(P, []) :- !.
forAllIn(P, [E|Rest]) :- 
	call(P, E),	
	forAllIn(P, Rest).

/* 
Outputs most of the relevant facts inserted and deleted during the lookup task.
*/
writeQueryFacts :-
  write('***************** FACTS *************************'),nl,
  writefacts(currentAttrs),
  writefacts(currentRels),
  writefacts(currentIsStarQuery),
  writefacts(currentVariables),
  writefacts(currentVariablesAll),
  writefacts(currentQueryRels),
  writefacts(currentUsedAttr),
  writefacts(currentQueryAttr),
  writefacts(subqueryDFSLabel),
  writefacts(subqueryCurrent),
  writefacts(subqueryMaxLabel),
  writefacts(queryRel),
  writefacts(variable),
  writefacts(usedAttr),
  writefacts(queryAttr),
  true.


% Should be called before a totally new query will be processed.
% But NOT before subqueries are processed.
totalNewQuery :-
  retractall(currentLevel(_)), % see the comments within subqueries.pl
  asserta(currentLevel(0)),  
  retractall(subqueryCurrent(_)),
  retractall(subqueryMaxLabel(_)),
  retractall(subqueryDFSLabel(_,_)),
  retractall(currentVariablesAll(_, _)),
  retractall(currentIsStarQuery(_, _)),
  retractall(currentQueryRels(_, _)),
  retractall(currentUsedAttr(_, _)),
  retractall(currentQueryAttr(_, _)),
  retractall(optimizerState(_)),
  retractall(relationTransformation(_, _, _, _, _)),
  retractall(finalTransformation(_, _, _, _, _)),
  newQuery.

/*
Transform a attribute identifier to it's full qualified name (FQN).
Means: 
  o:bevth is orteh:bevth
  p:o:bevth is orteh:subrel:bevth

  bevt for from o:subrel -> orteh:subrel:bevt
In this case, we have to define first the meaning of operations like this.

ModAttr tracks back the renaming process, that we know the attribute name in current subquery.
*/
% allowed for unnesting, meaning: only the first x:_ is a variable.
aattributeToFQN(A:B:C, X, ModAttr) :-
  !,
  ErrMsg='attribute access of this kind is not implemented',
  throw(error_SQL(optimizer_attributeToFQN(A:B:C,X,ModAttr)::unknownIdentifier::ErrMsg)).

% Already known
attributeToFQN(arel(_, FQN, ModAttr, _), FQN, ModAttr) :-
	!.
%attributeToFQN(arel(_, FQN, ModAttr, _), orte, ModAttr).

attributeToFQN(A, A, []) :-
  atomic(A),
	!.

attributeToFQN(A, FQN, ModAttr) :-
  A=Var:Attr,
	!,
  (findBinding(Var, variable(Var, rel(DCRel, Var))) -> 
		true
  ;
  	% Please take a look into the subqueries.pl file, predicate isQuery
  	% for information.
    throw(error_SQL(optimizer_lookupPred1(XY, XY)::unknownIdentifier::ignore_if_called_from_subquries_is_query1))
	),
  %write('DCRel is: '), write(DCRel),nl,
	(DCRel = relsubquery(_, SQID, TOP, _) ->
		% what todo now?
		(
			% So the attributes can be found within the result relation of the subquery. But still to compute what the attributes orign is.
			getRel(Attr, SQID, TOP, Rel, Mod),
			%Rel=rel(DCRel2, Var2),
			DCRel2=Rel,
			(Rel=queryAttr(_) ->
				(FQN=Rel, ModAttr=Var)
			;
			(
			%attributeToFQN(DCRel2, DCRelFQN, SubModAttr),
  		%appendAttribute(DCRelFQN, Attr, FQN),
  		appendAttribute(DCRel2, Attr, FQN),
  		appendAttribute(Var, Mod, Var2),
  		%appendAttribute(Var2, SubModAttr, ModAttr)
			ModAttr=Var2
			))
		)
	;
  	(
			% Var refers to the current query or a surrounding query
			attributeToFQN(DCRel, DCRelFQN, SubModAttr),
  		appendAttribute(DCRelFQN, Attr, FQN),
  		appendAttribute(Var, SubModAttr, ModAttr)
		)
	).

/*
Attr can be found within 
- the queryRel facts of the SQID.
- the variable facts of the subquery.
- another from-subquery.
Not implemented is the attribute access for computed attributes or 
constants.
*/
getRel(Attr, SQID, TOP, Rel, []) :-
	%var(TOP), % No transformation operator used
	atomic(Attr),
	currentQueryRels(SQID, L),
	member(queryRel(Rel, _), L),
	relation(Rel, AttList),
	member(Attr, AttList).

getRel(Attr, SQID, TOP, Rel, Var) :-
	%var(TOP), % No transformation operator used
	atomic(Attr),
	currentVariablesAll(SQID, L),
	member(variable(Var, RelT), L),
	RelT=rel(Rel, _),
	relation(Rel, AttList),
	member(Attr, AttList).

% for unesting in this case the attribute found this way
% more complex var bindings are not allowed here in difference
% to the subquieres within attributes oder predictes.
getRel(Var:Attr, SQID, TOP, Rel, Var) :-
  %var(TOP), % No transformation operator used
  atomic(Attr),
  currentVariablesAll(SQID, L),
  member(variable(Var, RelT), L),
  RelT=rel(Rel, _),
  relation(Rel, AttList),
  member(Attr, AttList).

/*
Special case that might happen now, the attribute to lookup might be now a new created attribute basedn on a expression within a subquery.
See for an example test query no. 843.
*/
getRel(Attr, SQID, TOP, Rel, []) :-
  atomic(Attr),
  currentQueryAttr(SQID, L),
  member(queryAttr(attr(Attr, Index, Case)), L),
	Rel=queryAttr(attr(Attr, Index, Case)).

/*
getRel(Attr, SQID, TOP, Rel, Mod) :-
	ground(TOP),
	getRel(Attr, SQID, _, Rel, Mod).
*/
getRel(Attr, SQID, unnest(UAttr), Rel, []) :-
  atomic(Attr),
	ground(UAttr),
	UAttr=attrname(attr(UAttrInternal, _, _)),
  atomic(UAttrInternal),
	dcName2internalName(DC, UAttrInternal),
	!,
	getRel(DC, SQID, _, URel, UVar),
  appendAttribute(URel, DC, XY), 
	Rel=XY.

getRel(_:Attr, SQID, unnest(UAttr), Rel, []) :-
  atomic(Attr),
  ground(UAttr),
  UAttr=attrname(attr(UAttrInternal, _, _)),
  atomic(UAttrInternal),
  dcName2internalName(DC, UAttrInternal),
  !,
  getRel(DC, SQID, _, URel, UVar),
  appendAttribute(URel, DC, XY),
  Rel=XY.

getRel(Attr, SQID, unnest(UAttr), Rel, V) :-
  atomic(Attr),
  ground(UAttr),
  UAttr=attrname(attr(UAttrInternal, _, _)),
	UAttrInternal=_:_,
  dcName2internalName(DC, UAttrInternal),
  !,
  getRel(DC, SQID, _, URel, UVar),
	DC=V:DCRest,
  appendAttribute(URel, DCRest, XY),
  Rel=XY.

getRel(_:Attr, SQID, unnest(UAttr), Rel, V) :-
  atomic(Attr),
  ground(UAttr),
  UAttr=attrname(attr(UAttrInternal, _, _)),
	UAttrInternal=_:_,
  dcName2internalName(DC, UAttrInternal),
  !,
  getRel(DC, SQID, _, URel, UVar),
	DC=V:DCRest,
  appendAttribute(URel, DCRest, XY),
  Rel=XY.
/*
in case of nest(...) we can lookup the attributes without doing any special
getRel(_:Attr, SQID, nest(UAttr), Rel, V) :-
*/

/*
  currentVariablesAll(SQID, L),
  member(variable(Var, RelT), L),
  RelT=rel(Rel, _),
  relation(Rel, AttList),
  member(Attr, AttList).
*/
	
/*
Currently support for explicit bindings only like var:xy
*/
getRelation(Identifer, Rel) :-
	attributeToFQN(Identifer, FQN, _),
	relNameFromFQN(FQN, Rel),
	atomic(Rel).

getRelation(Identifer, Rel) :-
	Msg='Failed to compute the relation.',
  throw(error_Internal(nr_getRelation(Identifer, Rel)::Msg)).

/*
If not bounded to the current query, we have to take a look into the variable bindings of the outer queries. Note that that some outer queries maybe skipped coz the variable isn't used there.
*/
findBinding(Var, Result) :-
  % These are the bindings for the current query.
  variable(Var, rel(Rel, Var)),!,
  Result=variable(Var, rel(Rel, Var)).

findBinding(Var, Result) :-
  % These are the bindings for the outer queries.
  subqueryCurrent(CurrentDeep),
  findBinding(CurrentDeep, Var, Result).

findBinding(-1, Var, _) :- !,
%   throw(error_SQL(optimizer_findBinding(-1, Var, _)::unknownBinding)).
  fail.

findBinding(SQID, Var, Result) :-
  ((currentVariablesAll(SQID, VarsList),
  	findfirst(variable(Var, rel(FREEVAR, Var)), VarsList, Result)) ->
    true %Result=variable(Var, rel(DCRel, Var))
  ; 
		(
			subqueryDFSLabel(PrevSQID, SQID),
      findBinding(PrevSQID, Var, Result)
    )
  ).

/*
Extract the relation name from a full qualified identifer.
Example:
?- relNameFromFQN(orteh, REL).
REL = orteh.
?- relNameFromFQN(orteh:subrel, REL).
REL = orteh.
relNameFromFQN(+FQN, -Rel)
*/
relNameFromFQN(FQN, Rel) :-
	FQN=Rel:_, !.

relNameFromFQN(FQN, Rel) :-
	FQN=Rel, !. % so there is no : in FQN.

/*
Recognize terms as queries (even if the query themselves is not valid)
*/
simplifiedIsQuery(select _ from _ where _).
simplifiedIsQuery(select _ from _).
simplifiedIsQuery(Query orderby _) :-
  simplifiedIsQuery(Query).
simplifiedIsQuery(Query groupby _) :-
  simplifiedIsQuery(Query).
simplifiedIsQuery(Query first _) :-
  simplifiedIsQuery(Query).
simplifiedIsQuery(Query last _) :-
  simplifiedIsQuery(Query).

/*
Provable if the query SQID is a ~isStarQuery~.
*/
isStarQuery(SQID) :-
	currentIsStarQuery(SQID, L),
	member(isStarQuery, L).

/*

*/
reduceToARel(AttrList, ARelPath, AttrList2) :-
  findall(ALast, (
      member(A, AttrList),
      lastAttribute(A, ALast),
      appendAttribute(ARelPath, ALast, Cmp),
      A=Cmp
    ), L),
  L=AttrList2.

/*
Not a very efficent version.
Example:
getRelDesc(orteh, X).
X = reldesc([[bevth, bevth, orteh:bevth, 'BevTH', int, noarel, sizeTerm(12, 5.0, 0)], [subrel, subrel, orteh:subrel, 'SubRel', arel, areldesc([[kennzeichen, kennzeichen, orteh:subrel:kennzeichen, 'Kennzeichen', string, noarel, sizeTerm(60, 8.320158102766799, 0)], [ort, ort, orteh:subrel:ort, 'Ort', string, noarel, sizeTerm(60, 18.1600790513834, 0)], [vorwahl, vorwahl, orteh:subrel:vorwahl, 'Vorwahl', string, noarel, sizeTerm(60, 10.725296442687746, 0)], [bevt, bevt, orteh:subrel:bevt, 'BevT', int, noarel, sizeTerm(12, 5.0, 0)]], sizeTerm(192, 42.20553359683794, 0)), sizeTerm(60, 90.9090909090909, 153.0909090909091)]], sizeTerm(72, 95.9090909090909, 153.0909090909091)).

It work'S like getRelAttrList/3, but here the spelling is stored, too.

The list that is describing a attribute has the following format:

Attr = [DCAttr, DCAttrMod, DCFQN, SP3, Type, ARelTerm, AST],
DCAttr: The attribute name as it is
DCAttrMod: the modified attribute name that is changing more less often due to renaming.
DCFQN: The full qualified identifier, only is the source is a field from a relation. var(DCFQN) otherwise.
SP3: The external spelling as the name will occur within the stream.
TYPE: the attributes data type
ARelTerm: recursive decription of arel relations.
AST=The sizeTerm.

*/
getRelDesc(DCRel, reldesc(AttrList3, TotalSizeTerm)) :-
  databaseName(DB),
	atomic(DCRel),
  relation(DCRel, AttrList1),
  reduceToARel(AttrList1, [], AttrList2),
  getRelDesc2(DB, DCRel, [], AttrList1, AttrList2, AttrList3, TotalSizeTerm), 
	!.

getRelDesc(DCFQN, reldesc(AttrList3, TotalSizeTerm)) :-
  databaseName(DB),
	DCFQN=DCRel:ARelPath,
  relation(DCRel, AttrList1),
  reduceToARel(AttrList1, ARelPath, AttrList2),
  getRelDesc2(DB, DCRel, ARelPath, AttrList1, AttrList2, AttrList3, TotalSizeTerm), 
	!.

getRelDesc(A, B) :-
  throw(error_Internal(nr_getRelDesc(A, B)::failed)).

getRelDesc2(_, _, _, _, [], [], sizeTerm(0, 0, 0)) :- 
	!.

getRelDesc2(DB, DCRel, ARelPath, AllAtts, [DCAttr|AttrList1], [ResAttr|ResAttrList1], TupleSize) :-
	!,
  appendAttribute(ARelPath, DCAttr, ADCFQN),
  appendAttribute(DCRel, ADCFQN, DCFQN),

  storedAttrSize(DB, DCRel, ADCFQN, Type, MemSize, Core, LOB),
  AST = sizeTerm(MemSize, Core, LOB),

	% Store the name of the attribute as to use within the stream.
	spelling(DCFQN, Spelling1),
	(Spelling1=lc(SP2) ->
		(SP3=SP2, Case=l)
	;
		(SP2=Spelling1, upper(Spelling1, SP3), Case=u)
	),

  getRelDesc2(DB, DCRel, ARelPath, AllAtts, AttrList1, ResAttrList1, TupleSize1),
	(Type = arel -> 
		(
  		appendAttribute(ARelPath, DCAttr, AP2),
  		reduceToARel(AllAtts, AP2, AttrListARel),
  		getRelDesc2(DB, DCRel, AP2, AllAtts, AttrListARel, ResAttrList2, ARelSZ),
			ARelTerm=areldesc(ResAttrList2, ARelSZ)
		)
	;
		ARelTerm=noarel % not available
	),
	ensure(atomic(DCAttr)), % Can be removed...but to be clear
	% DCFQN is only set if it is a real attributes, e.h. read from a relation.
  %ResAttr = [DCAttr, DCAttr, Case, DCFQN, SP3, Type, ARelTerm, AST],
  ResAttr = [DCAttr, SP2, Case, DCFQN, SP3, Type, ARelTerm, AST],
  addSizeTerms([TupleSize1, AST], TupleSize),
  !.

/*
Applies the rename operation on the virtual relation description to reflect the attribute renaming.

nrRenameAttrList(+AttrList, +Rename, -RAttrList)
*/
nrRenameAttrList(AL, *, AL) :- !.

nrRenameAttrList([], _, []).

nrRenameAttrList([A|AttrList], Rename, [AR|RAttrList]) :-
	nrRenameAttrList(AttrList, Rename, RAttrList),
	nrARename(A, Rename, AR).

nrARename(A, Rename, AR) :-
  A = [DCAttr, DCAttrMod, Case, DCFQN, SP, Type, ARelTerm, AST],
	(ARelTerm =.. [areldesc|_] ->
		(
			% A rename renames the attributes within the arel, too.
			ARelTerm=areldesc(ARelAtts, ARelSZ),
			nrRenameAttrList(ARelAtts, Rename, RARelAtts),
			ARelTermNew=areldesc(RARelAtts, ARelSZ)
		)
	;
		ARelTermNew=ARelTerm
	),
	ensure(atomic(Rename)),
	downcase_atom(Rename, DCRename),
	atomic_list_concat([SP, '_', Rename], SPR),
	appendAttribute(DCRename, DCAttrMod, NewAttrMod),
  AR = [DCAttr, NewAttrMod, Case, DCFQN, SPR, Type, ARelTermNew, AST].

/*
For a attribute desc list like returned by #getRelDesc/2
*/
recomputeSizeTerm([], sizeTerm(0, 0, 0)).

recomputeSizeTerm([A|Rest], SizeTermResult) :-
  recomputeSizeTerm(Rest, SizeTermResult1),
  A = [_, _, _, _, _, _, _, AST],
  addSizeTerms([SizeTermResult1, AST], SizeTermResult).

/*

*/
buildAttrList(LQuery, SQID, TOP, AttrList2, ST) :-
  isStarQuery(SQID),
  getRelsFromQuery(LQuery, Rels),
  makeList(Rels, LRels),
  % This fails when terms like [var.*, var.intatt*100 as xy] are allowed.
  % Then queryAttr terms are to be evaluated, too.
  attsFromRels(LQuery, SQID, TOP, LRels, AttrList1, ST),
  applyTOP(SQID, TOP, LRels, AttrList1, AttrList2),
  !.

buildAttrList(LQuery, SQID, TOP, AttrList2, ST) :-
  \+ isStarQuery(SQID),
  getRelsFromQuery(LQuery, Rels),
  makeList(Rels, LRels),
  % This fails when terms like [var.*, var.intatt*100 as xy] are allowed.
  % Then queryAttr terms are to be evaluated, too.
  attsFromFacts(LQuery, SQID, TOP, LRels, AttrList1, ST),
  applyTOP(SQID, TOP, LRels, AttrList1, AttrList2),
  !.

buildAttrList(LQuery, SQID, TOP, AttrList, ST) :-
  ErrMsg='failed to bulid the attribute list.',
  throw(error_Internal(nr_buildAttrList(LQuery, SQID, TOP, AttrList, ST)::ErrMsg)).

/*
Transforms the virtual relation description ~AttrList~ based on the given transformation operator in the way the secondo text-syntax operator would change the structure.
*/
applyTOP(SQID, notop, LRels, AttrList, AttrList) :-
	!.

applyTOP(SQID, unnest(attrname(NAttr)), LRels, AttrList, AttrList2) :-
  NAttr=attr(Attr, Index, Case),
	nrSimpleFindAttribute(Attr, AttrList, A, BeforeA, AfterA),
  A=[_, _, _, _, _, _, areldesc(ARelAtts, _), _],
	append(BeforeA, ARelAtts, TMP1),
	append(TMP1, AfterA, AttrList2),
  !.

applyTOP(SQID, nest(Attrs3, LA), LRels, AttrList, AttrList4) :-
	LA=attrname(attr(NewLabel, Index, Case)),
	(
		% only the case for non isStarQuery.
		% it's a little bit nasty, but the queryAttr facts need to be added
		% before we leave the subquery and compute this attribute description.
		nrRemoveAttrs([LA], AttrList, AttrList2, _)
	;
		AttrList=AttrList2
	),
	nrRemoveAttrs(Attrs3, AttrList2, AttrList3, Removed),
	lastAttribute(NewLabel, Attr),
	downcase_atom(Attr, DCAttr),
  recomputeSizeTerm(AttrList3, CSZ),
	AD=areldesc(AttrList3, CSZ),
	nrApplyCase(NewLabel, Case, SP3),
  NEW=[DCAttr, NewLabel, Case, _, SP3, arel, AD, CSZ],
	append(Removed, [NEW], AttrList4),
  !.

/*

*/
nrApplyCase(Attr, u, ExtSpell) :-
	atomic(Attr),
	upper(Attr, ExtSpell).
nrApplyCase(Attr, Case, Attr) :-
	\+ Case=u,
	atomic(Attr).

nrApplyCase(A:B, Case, S) :-
	nrApplyCase(B, Case, ExtSpell1),
	atomic_list_concat([ExtSpell1, '_', A], S).
	

applyTOP(SQID, TOP, LRels, AttrList, AttrList) :-
  throw(error_Internal(nr_applyTOP(SQID, TOP, LRels, AttrList, AttrList))
		::'failed to change to attribute list based on the transformation operator.').

nrRemoveAttrs([], AttrList, AttrList, []).
nrRemoveAttrs([Attr|Rest], AttrList, AttrList3, RemovedNew) :-
	Attr=attrname(attr(Attr2, Index, Case)),
	nrSimpleFindAttribute(Attr2, AttrList, A, BeforeA, AfterA),
	append(BeforeA, AfterA, AttrList2),
	nrRemoveAttrs(Rest, AttrList2, AttrList3, Removed),
	append([A], Removed, RemovedNew).

/*
The three cases that relation can be occur within the from list:
- a relation
- a arel attribut
- a query
So for a isStartQuery, all attributes are needed to be collected.
*/
attsFromRels(LQuery, SQID, TOP, [], [], sizeTerm(0, 0, 0)).

attsFromRels(LQuery, SQID, TOP, [Rel|RelRest], AttrList, SizeTermResult) :-
  attsFromRels(LQuery, SQID, TOP, RelRest, AttrListRest, SizeTerm1),
  Rel=rel(RelDCName, Var),
  atomic(RelDCName),
  !,
  %getRelAttrList(RelDCName, ResAttrList, SizeTerm),
  getRelDesc(RelDCName, reldesc(ResAttrList, SizeTerm)),
	nrRenameAttrList(ResAttrList, Var, ResAttrList2),
  append(ResAttrList2, AttrListRest, AttrList),
  addSizeTerms([SizeTerm1,SizeTerm], SizeTermResult).

attsFromRels(LQuery, SQID, TOP, [Rel|RelRest], AttrList, SizeTermResult) :-
  attsFromRels(LQuery, SQID, TOP, RelRest, AttrListRest, SizeTerm1),
  Rel=rel(T, Var),
  T=..[arel|_],
  !,
  T=arel(_, FQN, ModAttr, RName),
  FQN=_:_,
  %getRelAttrList(FQN, ResAttrList, SizeTerm),
  getRelDesc(FQN, reldesc(ResAttrList, SizeTerm)),
	nrRenameAttrList(ResAttrList, Var, ResAttrList2),
  append(ResAttrList2, AttrListRest, AttrList),
  addSizeTerms([SizeTerm1,SizeTerm], SizeTermResult).

attsFromRels(LQuery, SQID, TOP, [Rel|RelRest], AttrList, SizeTermResult) :-
  attsFromRels(LQuery, SQID, TOP, RelRest, AttrListRest, SizeTerm1),
  Rel=rel(T, Var),
  T=relsubquery(Query2, _, _, PS), % This SQID value may differ from SQID.
  !,
  PS=sqInfo(_, _, _, SizeTerm, ResAttrList),
	% Note that here is no renaming needed, this was already done.
  append(ResAttrList, AttrListRest, AttrList),
  addSizeTerms([SizeTerm1,SizeTerm], SizeTermResult).

attsFromRels(LQuery, SQID, TOP, [Rel|RelRest], AttrList, SizeTermResult) :-
  throw(error_Internal(nr_attsFromRels(LQuery, SQID, TOP, [Rel|RelRest], AttrList, SizeTermResult)::failed)).

/*
The nasty "trick" is here to exclude the unneeded attributes.
*/
attsFromFacts(LQuery, SQID, TOP, RelList, AttrList3, SizeTermResult) :-
  % This is way is choosen because it is pretty hard to collect these
  % information based on usedAttr and queryAttr facts. In particular
  % if they are coming from subqueries.
  attsFromRels(LQuery, SQID, TOP, RelList, AttrList1, _),
  currentUsedAttr(SQID, L1),
  currentQueryAttr(SQID, L2),
  %append(L1, L2, L),
  nrRemoveUnusedAttrs(L1, AttrList1, AttrList2),
  addQueryAttr(L2, AttrList2, AttrList3),
  recomputeSizeTerm(AttrList3, SizeTermResult),
  !.

/*
Find a attribute by it's simple name that was never renamed.
This supports to write queries without manually add all renaming labels.
*/
nrSimpleFindAttribute(AttrDC, [AL|_], AL) :-
  AL=[AttrDC|_]. % See getRelDesc for a structure description
nrSimpleFindAttribute(AttrDC, [_|Rest], AL) :-
  nrSimpleFindAttribute(AttrDC, Rest, AL).

/*
Return the index within the attribute list, starting at index 0.
*/
nrSimpleFindAttribute(Attr, AL, A, Index) :-
	nrSimpleFindAttributeTmp(Attr, AL, A, 0, Index).
nrSimpleFindAttributeTmp(Attr, [AL|_], AL, Index, Index) :-
  AL=[_,Attr|_]. % See getRelDesc for a structure description
nrSimpleFindAttributeTmp(Attr, [_|Rest], AL, CIndex, Index) :-
	CIndexNext is CIndex + 1,
  nrSimpleFindAttributeTmp(Attr, Rest, AL, CIndexNext, Index).

nrSimpleFindAttribute(Attr, AL, A, Firsts, Rest) :-
  nrSimpleFindAttributeTmp2(Attr, AL, A, [], Firsts, Rest).
nrSimpleFindAttributeTmp2(Attr, [AL|Rest], AL, Firsts, Firsts, Rest) :-
  AL=[_,Attr|_]. % See getRelDesc for a structure description
nrSimpleFindAttributeTmp2(Attr, [A|Rest], AL, Firsts, F2, Rest2) :-
	append(Firsts, [A], Firsts2),
  nrSimpleFindAttributeTmp2(Attr, Rest, AL, Firsts2, F2, Rest2).


/*

*/
addVar(A, *, A).
addVar(A, Var, AV) :-
	Var \= *,
	appendAttribute(Var, A, AV).

/*
Provable if the attribute A1 is in the list,
*/
nrContainsAttribute(A1, [L1|_]) :-
  L1=usedAttr(RelT, attr(Attr, Index, Case)),
	RelT=rel(_, Var),
  %applyOnAttributeList(downcase_atom, Attr, DCAttr),
	Attr=DCAttr,
	addVar(DCAttr, Var, DCAttrVar),
  A1=[_,DCAttrVar|_]. % See getRelDesc for a structure description
/*
nrContainsAttribute(A1, [L1|_]) :-
  L1=queryAttr(attr(Attr, Index, Case)),
  applyOnAttributeList(downcase_atom, Attr, DCAttr),
  A1=..[DCAttr|_]. % See getRelDesc for a structure description
*/
nrContainsAttribute(A1, [_|Rest]) :-
  nrContainsAttribute(A1, Rest).

nrRemoveUnusedAttrs(_, [] , []).

nrRemoveUnusedAttrs(L, [A1|ARest], [A1|ARest2]) :-
  nrContainsAttribute(A1, L),
  !,
  nrRemoveUnusedAttrs(L, ARest, ARest2).

nrRemoveUnusedAttrs(L, [A1|ARest], ARest2) :-
  !,
  nrRemoveUnusedAttrs(L, ARest, ARest2).

addQueryAttr([], AttrList, AttrList).

addQueryAttr([L|Rest], AttrList, AttrList2) :-
  L=queryAttr(attr(Attr, Index, Case)),
  applyOnAttributeList(downcase_atom, Attr, DCAttr),
  (Case=u ->
    upper(Attr, SP3)
  ;
    SP3=Attr
  ),
	% Open issue: compute the type and the sizeTerm
  NEW=[DCAttr, Attr, Case, _, SP3, unknown, noarel, sizeTerm(0, 0, 0)],
  append(AttrList, [NEW], AttrList1),
  addQueryAttr(Rest, AttrList1, AttrList2).

/*
Returns the result size of a query to use this in a later optimization process if needed.
*/
getQueryResultSize(Query last _, Size, TupleSize) :-
  getQueryResultSize(Query, Size, TupleSize).
getQueryResultSize(Query first _, Size, TupleSize) :-
  getQueryResultSize(Query, Size, TupleSize).
getQueryResultSize(Query orderby _, Size, TupleSize) :-
  getQueryResultSize(Query, Size, TupleSize).
% I found no clues that for this case there is any estimation implemended,
% so i make only a worst case estimation. It might be possible to calculate
% a appropriate values based on the stored selectivities.
getQueryResultSize(Query groupby _, Size, TupleSize) :-
  getQueryResultSize(Query, Size, TupleSize).

% note that as soon as more than one relation is involved, a where 
% condition must be within the query.
getQueryResultSize(select S from [RelT], Size, TupleSize) :-
  getQueryResultSize(select S from RelT, Size, TupleSize).

getQueryResultSize(select _ from RelT, Size, TupleSize) :-
  RelT=rel(relsubquery(Q, SQID, TOP, PS), Var),
  PS=sqInfo(Stream, Costs, Size, TupleSize, _).

getQueryResultSize(select _ from RelT, Size, TupleSize) :-
  RelT=rel(Rel, _),
  card(Rel, Size),
  tupleSizeSplit(Rel, TupleSize).

/*
In this case we can obtain the values from the pog optimization. (because a where condition is within the query)
Note that with is not available for the standard costs (but nested relations works only with these standard cost). But i added it anyway if later needed.
*/
getQueryResultSize(Query, Size, TupleSize) :-
  highNode(N),
  resultSize(N, Size),
  nodeTupleSize(N, TupleSize).

getQueryResultSize(Query, Size, sizeTerm(0, 0, 0)) :-
  \+ optimizerOption(nawracosts),
  \+ optimizerOption(improvedcosts),
  \+ optimizerOption(memoryAllocation).

getQueryResultSize(Query, Size, TupleSize) :-
  ErrMsg='Can\' obtain query result size',
  throw(error_Internal(nr_getQueryResultSize(Query, Size,TupleSize)::ErrMsg)).

/*

*/
nrLookupRel((Query) as Var, RRel) :-
  simplifiedIsQuery(Query),
  enterSubquery,
  getSubqueryCurrent(SQID),
  lookupSubquery(Query, Query2),

  % immediately create the plan.
  % Note that if a optimzation will performed, at every time only of optimize
  % process will be in progress.
	nrSubqueryToStream(Query2, Stream, Costs),
  getQueryResultSize(Query2, Size, _),

  leaveSubquery,
  TOP=notop,
  buildAttrList(Query2, SQID, TOP, AttrList1, TupleSize),
  nrRenameAttrList(AttrList1, Var, AttrList2),
  write('buildAttrList: '), write_term(AttrList2, []), nl, rd,
  PS=sqInfo(Stream, Costs, Size, TupleSize, AttrList2),

  % Surrounding Rel2 with a subquery(_) term makes the a later check with 
  % simplifiedIsQuery expendable.
  RRel=rel(relsubquery(Query2, SQID, TOP, PS), Var),
  assert(variable(Var, RRel)).

nrLookupRel((Query) as Var, RRel) :-
  simplifiedIsQuery(Query),
  throw(error_Internal(nr_nrLookupRel((Query) as Var, RRel)::failed)).

nrLookupRel((Query) unnest(Attr) as Var, RRel) :-
  simplifiedIsQuery(Query),
  enterSubquery,
  getSubqueryCurrent(SQID),
  lookupSubquery(Query, Query2),

  % immediately create the plan.
  % Note that if a optimzation will performed, at every time only of optimize
  % process will be in progress.
	nrSubqueryToStream(Query2, Stream, Costs),
  getQueryResultSize(Query2, Size, _),

  lookupAttr(Attr, NAttr),

  leaveSubquery,

  TOP=unnest(attrname(NAttr)),
  buildAttrList(Query2, SQID, TOP, AttrList1, TupleSize),
  nrRenameAttrList(AttrList1, Var, AttrList2),
  write('buildAttrList: '), write_term(AttrList2, []), nl,
  write('TOP: '), write_term(TOP, []), nl, rd,
  PS=sqInfo(Stream, Costs, Size, TupleSize, AttrList2),

  % Surrounding Rel2 with a subquery(_) term makes the a later check with 
  % simplifiedIsQuery expendable.
  RRel=rel(relsubquery(Query2, SQID, TOP, PS), Var),
  assert(variable(Var, RRel)).

% This syntax is a little bit strange, even if it makes sense.
nrLookupRel((Query) nest(Attrs) as NewLabel as Var, RRel) :-
  lookupRel((Query) nest(Attrs, NewLabel) as Var, RRel).

nrLookupRel((Query) nest(Attrs, NewLabel) as Var, RRel) :-
  simplifiedIsQuery(Query),
  enterSubquery,
writeQueryFacts,rd,
  getSubqueryCurrent(SQID),
  lookupSubquery(Query, Query2),

  % immediately create the plan.
  % Note that if a optimzation will performed, at every time only of optimize
  % process will be in progress.
	nrSubqueryToStream(Query2, Stream, Costs),
  getQueryResultSize(Query2, Size, _),

  lookupAttrs(Attrs, Attrs2), % note that a usedAttr fact might be added.
	makeList(Attrs2, Attrs2L),
	attrnames(Attrs2L, Attrs3),
  LA=attrname(attr(NewLabel, 0, u)),
  assert(queryAttr(attr(NewLabel, 0, u))),

  leaveSubquery,
  TOP=nest(Attrs3, LA),
  buildAttrList(Query2, SQID, TOP, AttrList1, TupleSize),
  nrRenameAttrList(AttrList1, Var, AttrList2),
  write('buildAttrList: '), write_term(AttrList2, []), nl,
  write('Attrs3: '), write_term(Attrs3, []), nl,
  write('LA: '), write_term(LA, []), nl, rd,
  PS=sqInfo(Stream, Costs, Size, TupleSize, AttrList2),

  % Surrounding Rel2 with a relsubquery(_) term makes the a later check with 
  % simplifiedIsQuery expendable.
  RRel=rel(relsubquery(Query2, SQID, TOP, PS), Var),
  assert(variable(Var, RRel)).

/*
nrLookupRel(Rel, RRel) :-
  optimizerOption(nestedRelations),
  simplifiedIsQuery(Rel), 
  ...
  assert(queryRel(_, RRel)). % Because of this, to makes this work, much more needs to be done whereever queryRel is used. 
*/


/*
For count queries, here is fixed that within the secondo the sql dialect fulfills not the closure property. Within secondo, a count query returns a single value and not a relation. Here the value is transformed into a tuple stream.
This is just a work around, i think it would be better to implement a new count operator that returns a tuple stream (with one tuple) as a result that can be consumed.
The same is the case for max/min/avg etc.

Another extension would be nice, too. If count produces a tule stream, labeling a the attribute should be possible:
select count(*) as anzahl from orte.

nrSubqueryToStream(+Query, ?Stream, ?Costs) 

The stream returned is always a consumeable query.
*/
nrSubqueryToStream(Query, Stream, Costs) :-
	countQuery(Query), 
  queryToStream(Query, Stream1, Costs),
	Attr=attr(count, 0, u),
	atomicValueToTupleStream(count(Stream1), Attr, Stream).

% implement more rules for sum, min etc. 
% Note that then the buildAttrList predicates needs to be modified, too.

nrSubqueryToStream(Query, Stream, Costs) :-
  queryToStream(Query, Stream, Costs).

atomicValueToTupleStream(StreamIn, Attr, StreamOut) :-
	Extend=[newattr(attrname(Attr), StreamIn)],
	StreamOut=projectextend(transformstream(feed(1)), [], Extend).

% eof
