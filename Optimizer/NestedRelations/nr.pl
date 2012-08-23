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
  secondoCatalogInfo(DCRel, _, _, [[nrel, _]]).

/*
The list elements needs to be in external spelling.

getArelUnnestAtom(+ARelListInExternalSpelling, ?UnnestAtom)
*/
getArelUnnestAtom([], '').

getArelUnnestAtom([A|Rest], String) :-
  getArelUnnestAtom(Rest, RString),
  atomic_list_concat([' unnest[', A, ']', RString], '', String).

/*

*/
concatAttributes([], '').

concatAttributes([A], AttrDC) :-
  dcName2externalName(AttrDC, A).

concatAttributes([A|REST], OUT) :-
  dcName2externalName(AttrDC, A),
  concatAttributes(REST, ROUT),
  OUT=AttrDC:ROUT.

/*
Atts has to be append this way or the operator type has to be changed from xfy to yfx, but this wouldn't be good. Then it is no longer possible we separate the elements from left to right. You can do this: a:b or a:(b:c) but NOT: (a:b):c. Then we can't obtain the first element with A : _. Note that we consider a "x:y..."-builded term as a list from left to right and not as a tree!
---- 
appendAttribute(+A1, +A2, -T)
----
*/
appendAttribute(A, [], A) :- !.
appendAttribute([], A, A) :- !.
appendAttribute(A, B, T) :-
  attributeTermToList(A:B, LST),
  listToAttributeTerm(LST, T).

appendAttributeList(A, B) :- 
	listToAttributeTerm(A, B).

listToAttributeTerm([A], A) :- !.
listToAttributeTerm([A|REST], T) :-
  listToAttributeTerm(REST, T2),
  T = A:T2.

attributeTermToList(A, _) :-
  var(A), !, 
  ErrMsg='Variables are not allowed here.',
  throw(error_Internal(nestedrelations_attributeTermToList(A,_)::ErrMsg)).

attributeTermToList(A:B, LST) :-
  attributeTermToList(A, LST1),
  attributeTermToList(B, LST2),
  append(LST1, LST2, LST), !.
attributeTermToList(A, [A]) :-
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
removeLastAttribute(+A, -B)
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
findfirst(X, [X|_], X) :- !.
findfirst(X, [Y|Rest], R) :-
  X \= Y, 
	!,
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
  ErrMsg='Query not in expected format.',
  throw(error_Internal(nr_getRelsFromQuery(Query, Rels)::ErrMsg)).

/*
Determine the cardinality for arel attributes.
Need if a arel apears within the from clause of a subquery.
Because there is no exact cardinality for arel attributes, and
this needed cardinality depending on the selectivitiy of the 
outer query, i just made the very simple an inexact aproximation
that the cardinality is the number of all rows within the arel
relation divided by the rows within the outer relation.
*/
% open issue
cardNR(arel(_, DCFQN, _, _, _, _), Size) :-
	(var(DCFQN);DCFQN=fqn(no)),
	Size=10,
  dm(nr, ['\ncardinality for: ', DCFQN, ' is: ', Size]).

cardNR(arel(_, DCFQN, _, _, _, _), Size) :-
  databaseName(DB),
  cardByFQN(DB, DCFQN, Size),
  dm(nr, ['\ncardinality for: ', DCFQN, ' is: ', Size]).

cardByFQN(DB, DCFQN, Size) :-
	DCFQN=Outer:_,
	!,
  storedCard(DB, DCFQN, ARelSize),
	% TODO: correct this
  cardByFQN(DB, Outer, OuterSize),
  Size is ARelSize / OuterSize.

cardByFQN(_, DCFQN, Size) :-
	atomic(DCFQN), !, % So DCFQN is a dc-relation identifier.
	card(DCFQN, Size).

/*
Predicates to support nested optimizer calls as needed by the subquery extension.
*/

:- dynamic optimizerState/1.

% Fix that some predicates are not registered yet as dynamic.
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

nrOptInfo :-
	write('\nCurrent optimizer state:\n'), 
	optimizerFacts(FList),
	forAllIn(writefacts, FList),
	write('\nStored optimizer states:\n'), 
	writefacts(optimizerState).

/* 
Outputs most of the relevant facts inserted and deleted during the lookup.
*/
nrInfo :-
  write('*** FACTS ***\n'),
  writefacts(optimizerState),
  writefacts(queryAttrDesc),
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

:- dynamic 
	subqueryCurrent/1,
	subqueryMaxLabel/1,
	subqueryDFSLabel/3,
	queryAttrDesc/3,
	currentIsStarQuery/2,
	currentVariablesAll/2,
	currentQueryRels/2,
	currentUsedAttr/2,
	currentQueryAttr/2,
	arelTermToOuterRelTerm/2.

% Should be called before a totally new query will be processed.
% But NOT before subqueries are processed.
totalNewQuery :-
  retractall(currentLevel(_)), % see the comments within subqueries.pl
  asserta(currentLevel(0)),  
  retractall(subqueryCurrent(_)),
  retractall(subqueryMaxLabel(_)),
  retractall(subqueryDFSLabel(_, _, _)),
  retractall(currentVariablesAll(_, _)),
  retractall(currentIsStarQuery(_, _)),
  retractall(currentQueryRels(_, _)),
  retractall(currentUsedAttr(_, _)),
  retractall(currentQueryAttr(_, _)),
  retractall(queryAttrDesc(_, _, _)),
  retractall(optimizerState(_)),
	retractall(arelTermToOuterRelTerm(_, _, _)),
  newQuery.


/*
0 is the top level query and at this time, the optimizer is not in the lookup
of a subquery.

During the lookup phase, these are node numbers of the subquery ~tree~. These are also stored within the subquery(_, _, _) terms to generate a identifer for the subqueries. This is needed to be able to identify a subquery within the plan_to_atom phase.
*/
getSubqueryCurrent(N) :-
  subqueryCurrent(N),
  !.
getSubqueryCurrent(0) :-
  !.

setSubqueryCurrent(N) :-
  retractall(subqueryCurrent(_)),
  assertz(subqueryCurrent(N)).

/*
Stores the current enviroment and restores the enviroment given by the parameter. This is not needed during the lookup, but later within the plan generation by the subquery extension.
*/
switchToSQID(SQID) :-
	getSubqueryCurrent(CSQID),
	storeCurrentQueryInfos(CSQID),
  setSubqueryCurrent(SQID),
  restoreCurrentQueryInfos(SQID).

storeCurrentQueryInfos(SQID) :-
  % What variable are accessable for a query?
  retractall(currentIsStarQuery(SQID, _)),
  findall(isStarQuery, isStarQuery, L1),
  asserta(currentIsStarQuery(SQID, L1)),

  retractall(currentVariablesAll(SQID, _)),
  findall(variable(V1, V2), variable(V1, V2), L2),
  asserta(currentVariablesAll(SQID, L2)),

  retractall(currentQueryRels(SQID, _)),
  findall(queryRel(V1, V2), queryRel(V1, V2), L3),
  asserta(currentQueryRels(SQID, L3)),

  retractall(currentUsedAttr(SQID, _)),
  findall(usedAttr(V1, V2), usedAttr(V1, V2), L4),
  asserta(currentUsedAttr(SQID, L4)),

  retractall(currentQueryAttr(SQID, _)),
  findall(queryAttr(V1), queryAttr(V1), L5),
  asserta(currentQueryAttr(SQID, L5)).

restoreCurrentQueryInfos(SQID) :-
  retractall(isStarQuery),
  currentIsStarQuery(SQID, L1),
  assertzall(L1),

  retractall(variable(_, _)),
  currentVariablesAll(SQID, L2),
  assertzall(L2),

  retractall(queryRel(_, _)),
  currentQueryRels(SQID, L3),
  assertzall(L3),

  retractall(usedAttr(_, _)),
  currentUsedAttr(SQID, L4),
  assertzall(L4),

  retractall(queryAttr(_)),
  currentQueryAttr(SQID, L5),
  assertzall(L5).

/*
The stored facts about the subquery could be deleted now. But for errory tracking we kept them and let them removed if a totaly new query will looked up.
*/
leaveSubquery :-
  dm(nr, ['\nleaveSubquery']),
  subqueryCurrent(SQID),
	injectUsedAttrToPreviousSQID(SQID),
  storeCurrentQueryInfos(SQID),
  setSQIDToPrevious(SQID),
  !.

% don't fail silently coz if this happend there is really something wrong.
leaveSubquery :-
  throw(error_Internal(nestedrelations_leaveSubquery::invalidState)).

/*

*/
injectUsedAttrToPreviousSQID(SQID) :-
  subqueryDFSLabel(PreviousSQID, SQID, _),
  getRelsBySQID(PreviousSQID, OuterRelList),
  retract(currentUsedAttr(PreviousSQID, L1)),
  findall(A, (
			usedAttr(XRel, XAttr), 
			member(XRel, OuterRelList), 
			A=usedAttr(XRel, XAttr),
			\+ member(A, L1)
		), L2),
	
	getCurrentARels(CurrentARelsList),
	createUsedAttrFromARelList(SQID, CurrentARelsList, L3),
	
	appendLists([L1, L2, L3], L4),
  assertz(currentUsedAttr(PreviousSQID, L4)).

createUsedAttrFromARelList(_, [], []).
createUsedAttrFromARelList(SQID, [RelT|Rest], [A|ARel]) :-
	createUsedAttrFromARelList(SQID, Rest, ARel),	
	RelT=rel(Rel, _),
	% The following rebuilding the attribute label is based on how it is done
	% within the nrLookupRel predicates that handels the arel access.
	Rel=..[arel, _, _, Attr2, Case |_],
	((Attr2=Pre:RestAttr, ground(Pre));RestAttr=Attr2),
	ensure(ground(RestAttr)),
	arelTermToOuterRelTerm(SQID, RelT, ORelT),
	A=usedAttr(ORelT, attr(RestAttr, 0, Case)).

getCurrentARels(CurrentARelList) :-
  findall(A, (
      queryRel(_, RelT),
			RelT=rel(Rel, _),
			Rel=..[arel|_],
			A=RelT
    ), L1),
  findall(A, (
      variable(_, RelT),
			RelT=rel(Rel, _),
			Rel=..[arel|_],
			A=RelT
    ), L2),
	append(L1, L2, CurrentARelList).

/*
Returns all rels used in the query with the given SQID, works not
for the current query.
*/
getRelsBySQID(SQID, Rels) :-
  currentVariablesAll(SQID, L1),
  currentQueryRels(SQID, L2),
  findall(X, member(variable(_, X), L1), LR1),
  findall(X, member(queryRel(_, X), L2), LR2),
  append(LR1, LR2, Rels).

setSQIDToPrevious(SQID) :-
  retractall(subqueryCurrent(_)),
  subqueryDFSLabel(PreviousSQID, SQID, _), % find the prev node
  asserta(subqueryCurrent(PreviousSQID)),
  restoreCurrentQueryInfos(PreviousSQID).

enterSubquery(Type) :-
  dm(nr, ['\nenterSubquery']),
  % Just ennumerate the subquries coz we have currently to other way to 
  % identify a query. In particular i'm interested in the parent queries to 
  % get some knowledge about these query variable bindings.
  getSubqueryCurrent(SQID),
  (retract(subqueryMaxLabel(Max)) ->
    true
  ;
    Max=0),
  retractall(subqueryMaxLabel(_)),
  NewSQID is Max+1,
  assertz(subqueryMaxLabel(NewSQID)),
  setSubqueryCurrent(NewSQID),
  assertz(subqueryDFSLabel(SQID, NewSQID, Type)),

  storeCurrentQueryInfos(SQID), % To restore the current lookup process 
  % after a subquery was processed.
  newQuery,
  !.

enterSubquery(Type) :-
  throw(error_Internal(nestedrelations_enterSubquery(Type)::invalidState)).

/*
If not bounded to the current query, it is necessary to look into the variable bindings of the outer queries. Note that that some outer queries maybe skipped coz the variable isn't used there.
*/
findBinding(Var, Result, SQID) :-
  variable(Var, rel(Rel, Var)),
	!,
 	getSubqueryCurrent(SQID),
  Result=variable(Var, rel(Rel, Var)).

findBinding(Var, Result, SQIDFound) :-
 	getSubqueryCurrent(SQID),
  findBinding(SQID, Var, Result, SQIDFound).

findBinding(SQID, Var, Result, SQIDFound) :-
  ((currentVariablesAll(SQID, VarsList),
  	findfirst(variable(Var, rel(_, Var)), VarsList, Result)) ->
    SQIDFound=SQID
  ; 
		(
			subqueryDFSLabel(PrevSQID, SQID, _),
      findBinding(PrevSQID, Var, Result, SQIDFound)
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
reduceToARel(AttrList, ARelPath, AttrListOut) :-
  findall(ALast, (
      member(A, AttrList),
      lastAttribute(A, ALast),
      appendAttribute(ARelPath, ALast, Cmp),
      A=Cmp
    ), AttrListOut).

%reduceAttrDescListToSimpleAttrList(AttrListDesc, AttrListOut) :-
reduceAttrDescListToSimpleAttrList([], []).
reduceAttrDescListToSimpleAttrList([AD|AttrListDescRest], [A|AttrListOutRest]) :-
	AD=..[_, Attr|_],
	downcase_atom(Attr, AttrDC),
	A=AttrDC,
	reduceAttrDescListToSimpleAttrList(AttrListDescRest, AttrListOutRest).

/*
Not a very efficent version.
Example:
?- getRelDesc(orteh, X).
X = reldesc([[bevth, bevTH, u, orteh:bevth, int, noarel, sizeTerm(12, 5.0, 0)], [subrel, subRel, u, orteh:subrel, arel, areldesc([[kennzeichen, kennzeichen, u, orteh:subrel:kennzeichen, string, noarel, sizeTerm(60, 8.320158102766799, 0)], [ort, ort, u, orteh:subrel:ort, string, noarel, sizeTerm(60, 18.1600790513834, 0)], [vorwahl, vorwahl, u, orteh:subrel:vorwahl, string, noarel, sizeTerm(60, 10.725296442687746, 0)], [bevt, bevT, u, orteh:subrel:bevt, int, noarel, sizeTerm(12, 5.0, 0)]], sizeTerm(192, 42.20553359683794, 0)), sizeTerm(60, 90.9090909090909, 153.0909090909091)]], sizeTerm(72, 95.9090909090909, 153.0909090909091)).

It work'S like getRelAttrList/3, but here the spelling is stored, too.

The list that is describing a attribute has the following format:

Attr = [DCAttr, DCAttrMod, DCFQN, Type, ARelTerm, AST],
DCAttr: The attribute name as it is
DCAttrMod: the modified attribute name that is changing more or less often due to renaming.
DCFQN: The full qualified identifier, only is the source is a field from a relation. var(DCFQN) otherwise.
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

getRelDesc2(DB, DCRel, ARelPath, AllAtts, [DCAttr|AttrList1], 
		[ResAttr|ResAttrList1], TupleSize) :-
	!,
  appendAttribute(ARelPath, DCAttr, ADCFQN),
  appendAttribute(DCRel, ADCFQN, DCFQN),

  storedAttrSize(DB, DCRel, ADCFQN, Type, MemSize, Core, LOB),
  AST = sizeTerm(MemSize, Core, LOB),

	% Store the name of the attribute as to use within the stream.
	spelling(DCFQN, Spelling1),
	(Spelling1=lc(SP2) ->
		(Case=l)
	;
		(SP2=Spelling1, Case=u)
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
	% DCFQN is only set if it is a real attributes, e.g. read from a relation.
  ResAttr = [DCAttr, SP2, Case, DCFQN, Type, ARelTerm, AST],
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

nrARename(AD, Rename, ADR) :-
  AD = [DCAttr, DCAttrMod, Case, DCFQN, Type, ARelTerm, AST],
	(ARelTerm =.. [areldesc|_] ->
		(
			% A rename renames the attributes within all arel attributes, too.
			ARelTerm=areldesc(ARelAtts, ARelSZ),
			nrRenameAttrList(ARelAtts, Rename, RARelAtts),
			ARelTermNew=areldesc(RARelAtts, ARelSZ)
		)
	;
		ARelTermNew=ARelTerm
	),
	ensure(atomic(Rename)),
	appendAttribute(Rename, DCAttrMod, NewAttrMod),
  ADR = [DCAttr, NewAttrMod, Case, DCFQN, Type, ARelTermNew, AST].

/*
For a attribute desc list like returned by #getRelDesc/2
*/
recomputeSizeTerm([], sizeTerm(0, 0, 0)).

recomputeSizeTerm([AD|Rest], SizeTermResult) :-
  recomputeSizeTerm(Rest, SizeTermResult1),
  AD = [_, _, _, _, _, _, AST|_],
  addSizeTerms([SizeTermResult1, AST], SizeTermResult).

/*

*/
buildAttrList(LQuery, SQID, TOP, AttrList, ST) :-
  getRelsFromQuery(LQuery, Rels),
  makeList(Rels, LRels),
	buildAttrList2(LRels, SQID, TOP, AttrList, ST).

buildAttrList(LQuery, SQID, TOP, AttrList, ST) :-
  ErrMsg='failed to bulid the attribute list.',
  throw(error_Internal(nr_buildAttrList(LQuery, SQID, TOP, AttrList, ST)::
		ErrMsg)).

buildAttrList2(LRels, SQID, TOP, AttrList2, ST) :-
  isStarQuery(SQID),
  % This fails when terms like [var.*, var.intatt*100 as xy] are allowed.
  % Then queryAttr terms are to be evaluated, too.
  attsFromRels(SQID, TOP, LRels, AttrList1, ST),
  applyTOP(SQID, TOP, LRels, AttrList1, AttrList2),
  !.

buildAttrList2(LRels, SQID, TOP, AttrList2, ST) :-
  \+ isStarQuery(SQID),
  % This fails when terms like [var.*, var.intatt*100 as xy] are allowed.
  % Then queryAttr terms are to be evaluated, too.
  attsFromFacts(SQID, TOP, LRels, AttrList1, ST),
	% open issue: handle count/aggr queries
  applyTOP(SQID, TOP, LRels, AttrList1, AttrList2),
  !.

/*
Transforms the virtual relation description ~AttrList~ based on the given transformation operator in the way the secondo text-syntax operator would change the structure.

applyTOP(+SQID, +TOP, +LRels, +AttrList, -AttrListOut)
*/
applyTOP(_, notop, _, AttrList, AttrList) :-
	!.

applyTOP(_, unnest(arel, attrname(NAttr)), _, AttrList, AttrList2) :-
  NAttr=attr(Attr, _, _),
	nrSimpleFindAttribute(Attr, AttrList, AD, BeforeA, AfterA),
  AD=[_, _, _, _, _, areldesc(ARelAtts, _), _],
	%append(BeforeA, ARelAtts, TMP1),
	%append(TMP1, AfterA, AttrList2),
	appendLists([BeforeA, ARelAtts, AfterA], AttrList2),
  !.

applyTOP(_, unnest(mpoint, attrname(NAttr)), _, AttrList, AttrList2) :-
  NAttr=attr(Attr, _, _),
	nrSimpleFindAttribute(Attr, AttrList, AD, BeforeA, AfterA),
  AD   =[DCAttr, NewLabel, Case, FQN, Type, ARelD, _CSZ],
	ensure(Type=mpoint),
	% Can the upoint size and CSZ be used to adjust the card?!?
	%CSZNEW=CSZ, % open issue. But i think we can set this size to a fixed value.
	secDatatype(upoint, MemSize, _FlobSize, _, _, _),
	CSZNEW=sizeTerm(MemSize, MemSize, 0),
  ADNEW=[DCAttr, NewLabel, Case, FQN, upoint, ARelD, CSZNEW],
	appendLists([BeforeA, [ADNEW], AfterA], AttrList2),
  !.

applyTOP(_, nest(Attrs3, LA), _, AttrList, AttrList4) :-
	LA=attrname(attr(NewLabel, _, Case)),
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
  recomputeSizeTerm(AttrList3, CSZ), % No extra space for the arel attribute
	% himself is added.
	ARelD=areldesc(AttrList3, CSZ),
	FQN=fqn(no), % Just building a term that indicates that this attribute
	% is not comming from the database, i can't pass just no because it
	% might be confusing, so i return this compounded term. To pass a variable
	% is not possible because the subquery extension does some ground(_) checks,
	% and these would fail then.
  AD=[DCAttr, NewLabel, Case, FQN, arel, ARelD, CSZ],
	append(Removed, [AD], AttrList4),
  !.

/*

*/
applyTOP(SQID, TOP, LRels, AttrList, AttrList) :-
  throw(error_Internal(nr_applyTOP(SQID, TOP, LRels, AttrList, AttrList))
		::'failed to change to attribute list based on the transformation operator.').

nrRemoveAttrs([], AttrList, AttrList, []).
nrRemoveAttrs([Attr|Rest], AttrList, AttrList3, RemovedNew) :-
	Attr=attrname(attr(Attr2, _, _)),
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

attsFromRels(+SQID, +TOP, +RelList, -AttrList, -SizeTermResult)
*/
attsFromRels(_, _, [], [], sizeTerm(0, 0, 0)) :- !.

attsFromRels(SQID, TOP, [Rel|RelRest], AttrList, SizeTermResult) :-
  attsFromRels(SQID, TOP, RelRest, AttrListRest, SizeTerm1),
  Rel=rel(RelDCName, Var),
  atomic(RelDCName),
  !,
  %getRelAttrList(RelDCName, ResAttrList, SizeTerm),
  getRelDesc(RelDCName, reldesc(ResAttrList, SizeTerm)),
	nrRenameAttrList(ResAttrList, Var, ResAttrList2),
  append(ResAttrList2, AttrListRest, AttrList),
  addSizeTerms([SizeTerm1,SizeTerm], SizeTermResult).

attsFromRels(SQID, TOP, [Rel|RelRest], AttrList, SizeTermResult) :-
  attsFromRels(SQID, TOP, RelRest, AttrListRest, SizeTerm1),
  Rel=rel(T, Var),
  T=..[arel|_],
  !,
  T=arel(_, _, _, _, areldesc(ResAttrList, _), SizeTerm),
  %(var(FQN);FQN=fqn(no)), % create during query execution
  %FQN=fqn(no),
  nrRenameAttrList(ResAttrList, Var, ResAttrList2),
  append(ResAttrList2, AttrListRest, AttrList),
  addSizeTerms([SizeTerm1,SizeTerm], SizeTermResult).

attsFromRels(SQID, TOP, [Rel|RelRest], AttrList, SizeTermResult) :-
  attsFromRels(SQID, TOP, RelRest, AttrListRest, SizeTerm1),
  Rel=rel(T, _),
  T=relsubquery(_, _, _, PS), % This SQID value may differ from SQID.
  !,
  PS=sqInfo(_, _, _, SizeTerm, ResAttrList),
	% Note that here is no renaming needed, this was already done.
  append(ResAttrList, AttrListRest, AttrList),
  addSizeTerms([SizeTerm1,SizeTerm], SizeTermResult).

attsFromRels(SQID, TOP, Rels, AttrList, SizeTermResult) :-
  throw(error_Internal(nr_attsFromRels(SQID, TOP, Rels, AttrList, 
		SizeTermResult)::failed)).

/*
The nasty "trick" is here to exclude the unneeded attributes.
*/
attsFromFacts(SQID, TOP, RelList, AttrList3, SizeTermResult) :-
  % This is way is choosen because it is pretty hard to collect these
  % information based on usedAttr and queryAttr facts. In particular
  % if they are coming from subqueries.
  attsFromRels(SQID, TOP, RelList, AttrList1, _),
  currentUsedAttr(SQID, L1),
  currentQueryAttr(SQID, L2),
  %append(L1, L2, L),
  nrRemoveUnusedAttrs(L1, AttrList1, AttrList2),
	% Currently, queryAttr are only possible if this query is NOT a isStarQuery.
  addQueryAttr(SQID, L2, AttrList2, AttrList3),
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
  L1=usedAttr(RelT, attr(Attr, _, _)),
	RelT=rel(_, Var),
  %applyOnAttributeList(downcase_atom, Attr, DCAttr),
	Attr=DCAttr,
	addVar(DCAttr, Var, DCAttrVar),
  A1=[_,DCAttrVar|_]. % See getRelDesc for a structure description

nrContainsAttribute(A1, [_|Rest]) :-
  nrContainsAttribute(A1, Rest).

nrRemoveUnusedAttrs(_, [] , []).

nrRemoveUnusedAttrs(L, [A1|ARest], [A1|ARest2]) :-
  nrContainsAttribute(A1, L),
  !,
  nrRemoveUnusedAttrs(L, ARest, ARest2).

nrRemoveUnusedAttrs(L, [_|ARest], ARest2) :-
  !,
  nrRemoveUnusedAttrs(L, ARest, ARest2).

addQueryAttr(_, [], AttrList, AttrList).

addQueryAttr(SQID, [L|Rest], AttrList, AttrList2) :-
  L=queryAttr(attr(Attr, Index, Case)),
	\+ queryAttrDesc(SQID, attr(Attr, Index, Case), _),

  applyOnAttributeList(downcase_atom, Attr, DCAttr),
	% Open issue: compute the type and the sizeTerm
  AD=[DCAttr, Attr, Case, fqn(no), unknown, noarel, sizeTerm(0, 0, 0)],
  append(AttrList, [AD], AttrList1),
  addQueryAttr(SQID, Rest, AttrList1, AttrList2).

addQueryAttr(SQID, [L|Rest], AttrList, AttrList2) :-
  L=queryAttr(attr(Attr, Index, Case)),
	queryAttrDesc(SQID, attr(Attr, Index, Case), OutQuery),
  OutQuery = subquery(_, _, _, ARelAttrList),

  applyOnAttributeList(downcase_atom, Attr, DCAttr),
  % Open issue: compute the type and the sizeTerm
	recomputeSizeTerm(ARelAttrList, SizeTermResult),
	ARelDesc=areldesc(ARelAttrList, SizeTermResult),
  AD=[DCAttr, Attr, Case, fqn(no), arel, ARelDesc, SizeTermResult],
  append(AttrList, [AD], AttrList1),
  addQueryAttr(SQID, Rest, AttrList1, AttrList2).

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
  RelT=rel(relsubquery(_, _, _, PS), _),
  PS=sqInfo(_, _, Size, TupleSize, _).

getQueryResultSize(select _ from RelT, Size, TupleSize) :-
  RelT=rel(Rel, _),
  card(Rel, Size),
  tupleSizeSplit(Rel, TupleSize).

getQueryResultSize(select _ from RelT, Size, sizeTerm(0, 0, 0)) :-
  \+ optimizerOption(nawracosts),
  \+ optimizerOption(improvedcosts),
  \+ optimizerOption(memoryAllocation),
  RelT=rel(Rel, _),
  card(Rel, Size).

/*
In this case we can obtain the values from the pog optimization. (because a where condition is within the query)
Note that with is not available for the standard costs (but nested relations works only with these standard cost). But i added it anyway if later needed.
*/
getQueryResultSize(_, Size, TupleSize) :-
  highNode(N),
  resultSize(N, Size),
  nodeTupleSize(N, TupleSize).

getQueryResultSize(_, Size, sizeTerm(0, 0, 0)) :-
  \+ optimizerOption(nawracosts),
  \+ optimizerOption(improvedcosts),
  \+ optimizerOption(memoryAllocation),
  highNode(N),
  resultSize(N, Size).

getQueryResultSize(Query, Size, TupleSize) :-
  ErrMsg='Can\'t obtain query result size',
  throw(error_Internal(nr_getQueryResultSize(Query, Size,TupleSize)::ErrMsg)).

/*

*/
addUsedAttrIfNeeded(RelT, Attr) :-
	usedAttr(RelT, Attr), 
	!.
addUsedAttrIfNeeded(RelT, Attr) :-
  assertz(usedAttr(RelT, Attr)),
	!.

getAttributeDesc(Type, Var:Attr, Rel, AD) :-
  atomic(Var),
  atomic(Attr),
  downcase_atom(Attr, AttrDC),
  findAttrLists(Type, Var, Rel, AttrDesc),
  nrSimpleFindAttribute(AttrDC, AttrDesc, AD).

getAttributeDesc(Type, Attr, Rel, AD) :-
  atomic(Attr),
  downcase_atom(Attr, AttrDC),
  findAttrLists(Type, *, Rel, AttrDesc),
  nrSimpleFindAttribute(AttrDC, AttrDesc, AD).

nrLookupAttr(Var:Attr, attr(Attr2, 0, Case)) :-
  atomic(Var),
  atomic(Attr),
  downcase_atom(Attr, AttrDC),
  findAttrLists(attribute, Var, Rel2, AttrDesc),
  nrSimpleFindAttribute(AttrDC, AttrDesc, AD),
  AD=[_, Attr2, Case|_],
  !,
  Attr2=_:RestAttr2,
	addUsedAttrIfNeeded(Rel2, attr(RestAttr2, 0, Case)).

nrLookupAttr(Attr, attr(Attr2, 0, Case)) :-
  atomic(Attr),
  downcase_atom(Attr, AttrDC),
  findAttrLists(attribute, *, Rel2, AttrDesc),
  nrSimpleFindAttribute(AttrDC, AttrDesc, AD),
  AD=[_, Attr2, Case|_],
  !,
  Attr2=RestAttr2,
	addUsedAttrIfNeeded(Rel2, attr(RestAttr2, 0, Case)).

nrLookupAttr(Query as Name, OutQuery) :-
  simplifiedIsQuery(Query),
  !,
  nrLookupSubqueryAttr(Query as Name, OutQuery).

/*
When creating the plan for this query, this query need always consumed with the aconsume operator.

Delimitation:

This form von subqueries always creates a arel attribute, is should does not recognize queries that returns a atomic value. Hence, 

sql select (select count(*) from orte) as label from orte first 1

should return a arel attribute. But in fact it won't create a arel attribute because of some other limitations (closure property is not satiesfied for count/max etc queries).

Nevertheless, i had my focus only on subqueries that creates arel attributes and the subqueries extends does not support this either.

However, if later someone want to implement both, the problem is just how to decide when to create a arel attribute or when a atomic result should be returned. It is important that the user can see what type will be returned, he will need this information when he want to use this attribute in the where part of the query.

For example:

sql select (select count(*) from orte) as label from orte first 1

Case 1: Atomic result
The where part would like for example this way: where label=10
Case 2: Arel 
The where part would like for example this way: where all(select * from label where count=10).

*/
nrLookupSubqueryAttr(Query as Name, OutQuery as attr(Name, 0, u)) :-
  \+ queryAttr(attr(Name, 0, u)),
 	enterSubquery(attribute),
  lookupSubquery(Query, Query2),
  getSubqueryCurrent(SQID), % Remember the query node for later use.
  OutQuery = subquery(SQID, Query2, []),
  % gathering now the information that might needed later.
  % In fact, this arel attribute can only be used within sql predicates.
  % maybe later for sorting, but that is an open issue.
  % Even if all needed informations are still available, i simplifies the later
  % lookup calls.
  %leaveSubquery,
  %assertz(queryAttr(attr(Name, 0, u))),
  %dm(nr, ['\nlookupSubqueryAttr result: ', OutQuery]).


  % immediately create the plan.
  % Note that if a optimzation will performed, at every time only of optimize
  % process will be in progress.
  %nrSubqueryToStream(Query2, Stream, Costs),
  %getQueryResultSize(Query2, Size, _),

  leaveSubquery,
  TOP=notop,
  buildAttrList(Query2, SQID, TOP, AttrList1, _),
  %nrRenameAttrList(AttrList1, Name, AttrList2),
  AttrList1=AttrList2,
  %PS=sqInfo(Stream, Costs, Size, TupleSize, AttrList2),
	
  OutQuery2 = subquery(SQID, Query2, [], AttrList2),

  % Surrounding Rel2 with a subquery(_) term makes the a later check with 
  % simplifiedIsQuery expendable.
  %OutQuery=subquery(Query2, SQID, TOP, PS),
  getSubqueryCurrent(CSQID), % Remember the query node for later use.
  assertz(queryAttr(attr(Name, 0, u))),
  assertz(queryAttrDesc(CSQID, attr(Name, 0, u), OutQuery2)).

nrLookupSubqueryAttr(Query, OutQuery) :-
  throw(error_Internal(nr_lookupSubqueryAttr(Query, OutQuery)::failed)).

nrLookupPred1(Var:Attr, attr(Attr2, Index, Case), RelsBefore, RelsAfter) :-
  atomic(Var),
  atomic(Attr),
  downcase_atom(Attr, AttrDC),
  findAttrLists(predicate, Var, Rel2, AttrDesc),
  nrSimpleFindAttribute(AttrDC, AttrDesc, AD),
  AD=[_, Attr2, Case|_],
  !,
  (Attr2=_:RestAttr2;Attr2=RestAttr2),
  (member(Rel2, RelsBefore) ->
    RelsAfter = RelsBefore
  ;
    append(RelsBefore, [Rel2], RelsAfter)
  ),
  nth1(Index, RelsAfter, Rel2),
	addUsedAttrIfNeeded(Rel2, attr(RestAttr2, 0, Case)),
  !.

nrLookupPred1(Attr, attr(Attr2, Index, Case), RelsBefore, RelsAfter) :-
  atomic(Attr),
  downcase_atom(Attr, AttrDC),
  findAttrLists(predicate, *, Rel2, AttrDesc),
  nrSimpleFindAttribute(AttrDC, AttrDesc, AD),
  AD=[_, Attr2, Case|_],
  !,
  Attr2=RestAttr2,
  (member(Rel2, RelsBefore) ->
    RelsAfter = RelsBefore
  ;
    append(RelsBefore, [Rel2], RelsAfter)
  ),
  nth1(Index, RelsAfter, Rel2),
	addUsedAttrIfNeeded(Rel2, attr(RestAttr2, 0, Case)),
  !.

/*
Possible within attribute queries or within predicate queries.
Not within the from clause queries (direct, of course, within the from query, attributes or predicates may contain query with a valid call of this predicate).
*/
nrLookupRel(RelVar:ARel as Var, Y) :-
  atomicCheck([RelVar, ARel, Var]),
  downcase_atom(ARel, AttrDC),
	findAttrLists(relation, RelVar, RelT, AttrDescList),
  nrSimpleFindAttribute(AttrDC, AttrDescList, AD),
  AD=[_, Attr2, Case, DCFQN, Type, ARelTerm, ST],

	(Type \= arel ->
		(
			ErrMsg='Source attribute has not the type arel. This is not allowed',
			throw(error_Internal(nr_nrLookupRel(RelVar:ARel as Var, Y))::ErrMsg)
		)
	;
		true
	),
  !,
	ARelTerm=areldesc(AttrList, ST1),
	nrRenameAttrList(AttrList, Var, AttrListR),
	ARelTerm2=areldesc(AttrListR, ST1),

  Y=rel(arel(RelVar:AttrDC, DCFQN, Attr2, Case, ARelTerm2, ST), Var),
  subqueryCurrent(SQID),
	assertz(arelTermToOuterRelTerm(SQID, Y, RelT)),
	
  checkVarIsFree(Var),
  assertz(variable(Var, Y)).

nrLookupRel(ARel as Var, Y) :-
	atomic(ARel),
  atomicCheck([ARel, Var]),
  downcase_atom(ARel, AttrDC),
	findAttrLists(relation, *, RelT, AttrDescList),
  nrSimpleFindAttribute(AttrDC, AttrDescList, AD),
  AD=[_, Attr2, Case, DCFQN, Type, ARelTerm, ST],
	Type=arel, % Just fail silently...maybe ARel is
	% a regular relation and should be handeld by the other predicate.
/*
  (Type \= arel ->
    (
      ErrMsg='Source attribute has not the type arel. This is not allowed',
      throw(error_Internal(nr_nrLookupRel(ARel as Var, Y))::ErrMsg)
    )
  ;
    true
  ),
*/
  !,
	ARelTerm=areldesc(AttrList, ST1),
	nrRenameAttrList(AttrList, Var, AttrListR),
	ARelTerm2=areldesc(AttrListR, ST1),

  Y=rel(arel(AttrDC, DCFQN, Attr2, Case, ARelTerm2, ST), Var),
  subqueryCurrent(SQID),
	assertz(arelTermToOuterRelTerm(SQID, Y, RelT)),

  checkVarIsFree(Var),
  assertz(variable(Var, Y)).

nrLookupRel(RelVar:ARel, Y) :-
  atomicCheck([RelVar, ARel]),

  downcase_atom(ARel, AttrDC),
	findAttrLists(relation, RelVar, RelT, AttrList),
  nrSimpleFindAttribute(AttrDC, AttrList, AD),
  AD=[_, Attr2, Case, DCFQN, Type, ARelTerm, ST],

  (Type \= arel ->
    (
      ErrMsg='Source attribute has not the type arel. This is not allowed',
      throw(error_Internal(nr_nrLookupRel(RelVar:ARel, Y))::ErrMsg)
    )
  ;
    true
  ),
  !,
  ARelTerm=areldesc(_, _),

  Y=rel(arel(RelVar:AttrDC, DCFQN, Attr2, Case, ARelTerm, ST), *),
  subqueryCurrent(SQID),
	assertz(arelTermToOuterRelTerm(SQID, Y, RelT)),
	
	% XXX still to implement
  %\+ duplicateAttrs(Y),

  assertz(queryRel(RelVar:AttrDC, Y)).

nrLookupRel(Query, _) :-
  simplifiedIsQuery(Query),
	ErrMsg='Subqueries within the from clause needs to be specified with a label like \'Query as label\'.',
	throw(error_SQL(nr_nrLookupRel(Query, _)::ErrMsg)).

nrLookupRel(Query as Var, RRel) :-
  simplifiedIsQuery(Query),
  enterSubquery(relation),
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
  PS=sqInfo(Stream, Costs, Size, TupleSize, AttrList2),

  % Surrounding Rel2 with a subquery(_) term makes the a later check with 
  % simplifiedIsQuery expendable.
  RRel=rel(relsubquery(Query2, SQID, TOP, PS), Var),
	checkVarIsFree(Var),
  assertz(variable(Var, RRel)).

nrLookupRel(Query as Var, RRel) :-
  simplifiedIsQuery(Query),
  throw(error_Internal(nr_nrLookupRel(Query as Var, RRel)::failed)).

nrLookupRel(Query unnest(Attr) as Var, RRel) :-
  simplifiedIsQuery(Query),
  enterSubquery(relation),
  getSubqueryCurrent(SQID),
  lookupSubquery(Query, Query2),

  % immediately create the plan.
  % Note that if a optimzation will performed, at every time only of optimize
  % process will be in progress.
	nrSubqueryToStream(Query2, Stream, Costs),
  getQueryResultSize(Query2, Size, _),

  lookupAttr(Attr, NAttr),
	getAttributeDesc(attribute, Attr, _, AD),
  AD=[_, _, _, _, Type|_],
	(member(Type, [arel, mpoint]) -> 
		true
	;
		throw(error_SQL(nr_nrLookupRel(Query unnest(Attr) as Var, RRel)::
			'The unnest operator can only be applied on arel or mpoint attributes.'))
	),

  leaveSubquery,

  TOP=unnest(Type, attrname(NAttr)),
  buildAttrList(Query2, SQID, TOP, AttrList1, TupleSize),
  nrRenameAttrList(AttrList1, Var, AttrList2),
  PS=sqInfo(Stream, Costs, Size, TupleSize, AttrList2),

  % Surrounding Rel2 with a subquery(_) term makes the a later check with 
  % simplifiedIsQuery expendable.
  RRel=rel(relsubquery(Query2, SQID, TOP, PS), Var),
	checkVarIsFree(Var),
  assertz(variable(Var, RRel)).

% This syntax is a little bit strange, even if it makes sense.
nrLookupRel(Query nest(Attrs) as NewLabel as Var, RRel) :-
  lookupRel(Query nest(Attrs, NewLabel) as Var, RRel).

nrLookupRel(Query nest(Attrs, NewLabel) as Var, RRel) :-
  simplifiedIsQuery(Query),
  enterSubquery(relation),
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
  assertz(queryAttr(attr(NewLabel, 0, u))),

  leaveSubquery,
  TOP=nest(Attrs3, LA),
  buildAttrList(Query2, SQID, TOP, AttrList1, TupleSize),
  nrRenameAttrList(AttrList1, Var, AttrList2),
  PS=sqInfo(Stream, Costs, Size, TupleSize, AttrList2),

  % Surrounding Rel2 with a relsubquery(_) term makes the a later check with 
  % simplifiedIsQuery expendable.
  RRel=rel(relsubquery(Query2, SQID, TOP, PS), Var),
	checkVarIsFree(Var),
  assertz(variable(Var, RRel)).

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
	%Extend=[newattr(attrname(Attr), StreamIn)],
	%StreamOut=projectextend(transformstream(feed(1)), [], Extend).
	% Better:
	attrnames([Attr], AAL),
	StreamOut=namedtransformstream(feed(StreamIn), AAL).

/*
Special version for the lookupAttr phase.
In this case, of course, the attrlist can't be reduces based on the
usedAttr facts.
getCurrentAttrList(+SQID, ?Var, ?RelT, -AttrList) 
*/
getCurrentAttrList(_, Var, RelT, AttrList) :-
  Var \= *,
  variable(Var, RelT),
	extractAttrList(RelT, AttrList).

getCurrentAttrList(_, *, RelT, AttrList) :-
  queryRel(_, RelT),
	extractAttrList(RelT, AttrList).

getCurrentAttrList(SQID, Var, RelT, AttrList) :-
	Var \= *,
  variable(Var, RelT),
  attsFromRels(SQID, notop, [RelT], AttrList, _).

getCurrentAttrList(SQID, *, RelT, AttrList) :-
  queryRel(_, RelT),
  attsFromRels(SQID, notop, [RelT], AttrList, _).

/*
Like findBinding, but searches only in the direct outer query.
*/
findParentBinding(SQID, Var, Result, SQIDFound) :-
  subqueryDFSLabel(PrevSQID, SQID, _),
  findParentBinding(PrevSQID, Var, Result, SQIDFound).

findParentBinding(SQID, Var, Result, SQID) :-
  currentVariablesAll(SQID, List),
  findfirst(variable(Var, rel(_, Var)), List, Result).

getParentAttrList(SQID, Var, RelT, AttrList) :-
  Var \= *,
	findParentBinding(SQID, Var, Result, _),
  Result=variable(Var, RelT),
  extractAttrList(RelT, AttrList).

getParentAttrList(SQID, *, RelT, AttrList) :-
  subqueryDFSLabel(PrevSQID, SQID, _),
  currentQueryRels(PrevSQID, List),
	member(queryRel(_, RelT), List),
  extractAttrList(RelT, AttrList).

getParentAttrList(SQID, Var, RelT, AttrList) :-
  Var \= *,
	findParentBinding(SQID, Var, Result, SQIDFound),
  Result=variable(Var, RelT),
  attsFromRels(SQIDFound, notop, [RelT], AttrList, _).

getParentAttrList(SQID, *, RelT, AttrList) :-
  subqueryDFSLabel(PrevSQID, SQID, _),
  currentQueryRels(PrevSQID, List),
	member(queryRel(_, RelT), List),
  attsFromRels(PrevSQID, notop, [RelT], AttrList, _).


% With the lookup of attribute, the attribute may come from the attribute
% gather together within the from clause.
% Of course, lookups for outer query attribute may usefull and possible,
% but this is not implemented.
findAttrLists(Mode, Var, RelT, AttrList) :-
  getSubqueryCurrent(SQID),
	member(Mode, [attribute, predicate]),
	getCurrentAttrList(SQID, Var, RelT, AttrList).

/*
Case: relation
A arel attribute can only obtained from the sourrounding query.

Either with just the name, or by dereferencing a variable binding like var:arel.

RelVar has always to be a term hat isn't confusing prolog, so e.g. starts not with uppercase letters.

Case: predicate
For predicates, one more this must be allowed, it is to
reference a outer query attribute.
*/
findAttrLists(Mode, Var, RelT, AttrList) :-
  getSubqueryCurrent(SQID),
	member(Mode, [relation, predicate]),
	getParentAttrList(SQID, Var, RelT, AttrList).

% Special case: reference without var:_ can access other attributes that
% are not always attributes of relations.
findAttrLists(Mode, *, norel, AttrList) :-
  getSubqueryCurrent(SQID),
	member(Mode, [relation, predicate]),
  subqueryDFSLabel(PrevSQID, SQID, _),
  currentQueryAttr(PrevSQID, L),
	member(queryAttr(Attr), L),
	Attr=attr(Label, _, Case),
	queryAttrDesc(PrevSQID, Attr, SQT),
	SQT=subquery(_, _, _, ARelAttrList),
	downcase_atom(Label, LabelDC),
	recomputeSizeTerm(ARelAttrList, SizeTermResult),
	ARelDesc=areldesc(ARelAttrList, SizeTermResult),
  AD=[LabelDC, Label, Case, _, arel, ARelDesc, SizeTermResult],
	AttrList=[AD].

findAttrLists(Mode, Var, RelT, AttrList) :-
	\+ member(Mode, [relation, attribute, predicate]),
	throw(error_Internal(nr_findAttrLists(Mode, Var, RelT, AttrList)::
		'illegal call')).

/*
Extract the attribute list if the list is stored within the rel term.
*/
extractAttrList(rel(relsubquery(_, _, _, PS), _), AttrList) :-
	!,
  PS=sqInfo(_, _, _, _, AttrList).

extractAttrList(rel(arel(_, _, _, _, ARelTerm, _), _), AttrList) :-
	!,
	ARelTerm=areldesc(AttrList, _).

/*
A new variable may overwrite a variable in a outer query, so then the previous defined variable is not visible under this query anymore.
Two variables with the same name on the same deep within different subqueries is not a problem.
*/
checkVarIsFree(Var) :-
  variable(Var, _),
  atomic_list_concat(['The variable ', Var, ' is already used.'], '', ErrMsg),
  throw(error_SQL(nr_checkVarIsFree(Var)::malformedExpression::ErrMsg)).

checkVarIsFree(_).

/*
Applies the give case to the last attribute within the attribute list.
*/
nrApplyCase(A:B, Case, Atom) :-
	nrApplyCase(B, Case, ExtSpell),
	atomic_list_concat([ExtSpell, '_', A], Atom).
	
nrApplyCase(Attr, u, ExtSpell) :-
	atomic(Attr),
	upper(Attr, ExtSpell).
nrApplyCase(Attr, l, Attr) :-
	atomic(Attr).

nrApplyCase(Attr, Case, AttrOut) :-
	Case \= l,
	Case \= u,
	throw(error_Internal(nr_nrApplyCase(Attr, Case, AttrOut)::invalidCase)).

/*
StreamIn is a plan that creates a tuple stream, on this stream, the stream is transformed based on the transformation operation(TOP) to create the ~StreamOut~ plan.
addTransformationOperator(+StreamIn, +TOP, -StreamOut)
*/
addTransformationOperator(Stream, notop, Stream).


addTransformationOperator(Stream1, unnest(arel, Attr), Stream2) :-
  Stream2=unnest(Stream1, Attr).

% Example: 
% extendstream[UTrip: units(.Trip)] remove[Trip] renameattr[Trip: UTrip] consume
addTransformationOperator(Stream1, unnest(mpoint, Attr), Stream4) :-
	Attr=attrname(attr(A, Index, Case)),
	AttrN=attr(tempxxxxx, Index, Case),
	NewAttr=newattr(attrname(AttrN), units(attr(A, Index, Case))),
  Stream2=extendstream(Stream1, NewAttr),
  Stream3=remove(Stream2, Attr),
  Stream4=renameattr(Stream3, newattr(Attr, attrname(AttrN))).

addTransformationOperator(Stream1, nest(Attrs, NewLabel), Stream3) :-
  % The nest operation expect a sorted stream.
  Stream2=sortby(Stream1, Attrs),
  Stream3=nest(Stream2, Attrs, NewLabel).
  % Open issue: restoring the sort order after the nest operation, but this
  % not easy at all.

addTransformationOperator(StreamIn, TOP, StreamOut) :-
	throw(error_Internal(nr_addTransformationOperator(StreamIn, TOP, StreamOut)::failed)).

/*

*/
nrRel(Rel) :-
  Rel = rel(T, _),
  T=..[arel|_].
nrRel(Rel) :-
  isRelsubquery(Rel).
nrRel(pr(_, Rel1, _)) :-
  nrRel(Rel1).
nrRel(pr(_, _, Rel2)) :-
  nrRel(Rel2).
nrRel(pr(_, Rel)) :-
  nrRel(Rel).

isRelsubquery(Rel) :-
  Rel = rel(T, _),
  T =.. [relsubquery|_].

% eof
