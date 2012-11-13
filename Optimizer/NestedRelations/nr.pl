/*
$Header$
@author Nikolai van Kempen

This files provides different functions to work with nested relations within the optimizer.

Open issues: optimization of such queries, selectivities, cards 
*/

/* 
Checks if DCRel is a rel of the type nrel. Can be used to enumerate the nested relation objects within the current database.

is_nrel(?DCRel)
*/
is_nrel(DCRel) :-
  secondoCatalogInfo(DCRel, _, _, [[nrel, _]]).

/*
Builds a atom that applies a sequence of unnest operations.
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
appendAttribute(+A1, +A2, ?T)
----
*/
appendAttribute(A, [], A) :- !.
appendAttribute([], A, A) :- !.
appendAttribute(A, B, T) :-
  attributeTermToList(A:B, LST),
  listToAttributeTerm(LST, T).

appendAttributeList(A, B) :- 
	listToAttributeTerm(A, B).

listToAttributeTerm([A], A) :- 
	!.
listToAttributeTerm([A|REST], T) :-
  listToAttributeTerm(REST, T2),
  T = A:T2.

attributeTermToList(A, _) :-
  var(A), 
	!, 
  ErrMsg='Variables are not allowed here.',
  throw(error_Internal(nestedrelations_attributeTermToList(A,_)::ErrMsg)).

attributeTermToList(A:B, LST) :-
  attributeTermToList(A, LST1),
  attributeTermToList(B, LST2),
  append(LST1, LST2, LST), 
	!.
attributeTermToList(A, [A]) :-
  A \= _:_, 
	!.


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
Determine the cardinality for arel attributes.
Needed if a arel relation apears within the from clause of a subquery.
Because there is no exact cardinality for arel attributes, and
this needed cardinality depending on the selectivitiy of the 
outer query, implemented ist just the very simple approximation
that the cardinality is the number of all rows within the arel
relation divided by the rows of all outer arels/nrels.
*/
/*
Just return the stored size.
*/
nrCard(irrel(_, _, _, Card, _, _, _), Card) :- 
	!.

% open issue
% how to estimate the card size for arels that are created on the fly?
nrCard(fqn(no), 10) :-
	!. 

nrCard(FQN, Card) :-
	FQN=_:_,
  databaseName(DB),
	cardByFQN(DB, FQN, Card),
	!.

/*
The the average numer of rows within the arel relation. 
Note that the variation to the real cardinality may be huge,
what is totally different from the cardinalty for the top level relation.
*/
cardByFQN(DB, DCFQN, Size) :-
	DCFQN=Outer:_,
	!,
  storedCard(DB, DCFQN, ARelSize),
  cardByFQN(DB, Outer, OuterSize),
  Size is ARelSize / OuterSize.

cardByFQN(_, DCFQN, Size) :-
	atomic(DCFQN), 
	!, % So DCFQN is a dc-relation identifier.
	card(DCFQN, Size).

/*
Just return here pre-stored tuple sizes.
*/
nrTupleSizeSplit(irrel(_, _, _, _, TupleSize, _, _), TupleSize).

% Fix that some predicates are not registered yet as dynamic.
:- dynamic 
	node/3,
	edge/6,
	argument/2,
	planEdge/4.

% New dynamic facts used by this extension.
:- dynamic 
	subqueryCurrent/1,
	subqueryMaxLabel/1,
	subqueryDFSLabel/3,
	queryAttrDesc/3,
	aliasBySQID/2,
	isStarQueryBySQID/2,
	variablesBySQID/2,
	queryRelsBySQID/2,
	usedAttrsBySQID/2,
	queryAttrsBySQID/2.

/* 
Outputs most of the relevant facts inserted and deleted during the lookup.
*/
nrInfo :-
  write('*** FACTS ***\n'),
	FList=[queryAttrDesc, currentAttrs, currentRels,
		isStarQueryBySQID, variablesBySQID, 
		queryRelsBySQID, usedAttrsBySQID, queryAttrsBySQID,
		subqueryDFSLabel, subqueryCurrent, subqueryMaxLabel,
		queryRel, variable, usedAttr, queryAttr, aliasBySQID],
	forAllIn(writefacts, FList).

% Should be called before a totally new query will be processed.
% But NOT before subqueries are processed.
totalNewQuery :-
  retractall(currentLevel(_)), % see the comments within subqueries.pl
  asserta(currentLevel(0)),  
  retractall(subqueryCurrent(_)),
  retractall(subqueryMaxLabel(_)),
  retractall(subqueryDFSLabel(_, _, _)),
  retractall(variablesBySQID(_, _)),
  retractall(isStarQueryBySQID(_, _)),
  retractall(queryRelsBySQID(_, _)),
  retractall(usedAttrsBySQID(_, _)),
  retractall(queryAttrsBySQID(_, _)),
  retractall(aliasBySQID(_, _)),
  retractall(queryAttrDesc(_, _, _)),
  newQuery.


/*
0 is the top level query and at this time, the optimizer is not in the lookup
of a subquery.

During the lookup phase, these are node numbers of the subquery ~tree~. These are also stored within the subquery(_, _, _) terms to generate a identifer for the subqueries. This is needed to be able to identify a subquery within the plan_to_atom phase.
*/
getSubqueryCurrent(N1) :-
  subqueryCurrent(N2),
  !, 			% This N1/N2 behavior with the cut is important. Otherwise you might
					% encouter backtracking problems.
	N1=N2.
getSubqueryCurrent(0) :-
  !.

setSubqueryCurrent(N) :-
  retractall(subqueryCurrent(_)),
  assertz(subqueryCurrent(N)).

/*
Stores the current enviroment and restores the enviroment given by the parameter. This is not needed during the lookup, but later within the plan generation by the subquery extension.
*/
restoreSQID(_SQID) :-
  \+ optimizerOption(nestedRelations). 

restoreSQID(SQID) :-
  optimizerOption(nestedRelations), 
	getSubqueryCurrent(CSQID),
	storeCurrentQueryInfos(CSQID),
  setSubqueryCurrent(SQID),
  restoreCurrentQueryInfos(SQID).

storeCurrentQueryInfos(SQID) :-
  % What variable are accessable for a query?
  retractall(isStarQueryBySQID(SQID, _)),
  findall(isStarQuery, isStarQuery, L1),
  asserta(isStarQueryBySQID(SQID, L1)),

  retractall(variablesBySQID(SQID, _)),
  findall(variable(V1, V2), variable(V1, V2), L2),
  asserta(variablesBySQID(SQID, L2)),

  retractall(queryRelsBySQID(SQID, _)),
  findall(queryRel(V1, V2), queryRel(V1, V2), L3),
  asserta(queryRelsBySQID(SQID, L3)),

  retractall(usedAttrsBySQID(SQID, _)),
  findall(usedAttr(V1, V2), usedAttr(V1, V2), L4),
  asserta(usedAttrsBySQID(SQID, L4)),

  retractall(queryAttrsBySQID(SQID, _)),
  findall(queryAttr(V1), queryAttr(V1), L5),
  asserta(queryAttrsBySQID(SQID, L5)).

restoreCurrentQueryInfos(SQID) :-
  retractall(isStarQuery),
  isStarQueryBySQID(SQID, L1),
  assertzall(L1),

  retractall(variable(_, _)),
  variablesBySQID(SQID, L2),
  assertzall(L2),

  retractall(queryRel(_, _)),
  queryRelsBySQID(SQID, L3),
  assertzall(L3),

  retractall(usedAttr(_, _)),
  usedAttrsBySQID(SQID, L4),
  assertzall(L4),

  retractall(queryAttr(_)),
  queryAttrsBySQID(SQID, L5),
  assertzall(L5).

/*
The stored facts about the subquery could be deleted now. But for error tracking the are kept and only removed if a totaly new query will be looked up.
*/
leaveSubquery :-
  \+ optimizerOption(nestedRelations), 
	!.

leaveSubquery :-
  optimizerOption(nestedRelations), 
  dm(nr, ['\nleaveSubquery']),
  subqueryCurrent(SQID),
	injectUsedAttrToPreviousSQID(SQID),
  storeCurrentQueryInfos(SQID),
  setSQIDToPrevious(SQID),
  !.

% don't fail silently because if this happens there is really something wrong.
leaveSubquery :-
  throw(error_Internal(nestedrelations_leaveSubquery::invalidState)).

/*
Assertz's usedAttr facts to the surrounding query if attributes are
used of the outer query. 
*/
injectUsedAttrToPreviousSQID(SQID) :-
  subqueryDFSLabel(PreviousSQID, SQID, _),
  getRelsBySQID(PreviousSQID, OuterRelList),
  retract(usedAttrsBySQID(PreviousSQID, L1)),
  findall(A, (
			usedAttr(XRel, XAttr), 
			member(XRel, OuterRelList), 
			A=usedAttr(XRel, XAttr),
			\+ member(A, L1)
		), L2),
	
	getCurrentARels(CurrentARelsList),
	createUsedAttrFromARelList(SQID, CurrentARelsList, L3),
	
	appendLists([L1, L2, L3], L4),
  assertz(usedAttrsBySQID(PreviousSQID, L4)).

createUsedAttrFromARelList(_, [], []).
createUsedAttrFromARelList(SQID, [RelT|Rest], [A|ARel]) :-
	createUsedAttrFromARelList(SQID, Rest, ARel),	
	RelT=rel(Rel, _),
	% The following task of rebuilding the attribute label is based on how it is
	% done within the nrLookupRel predicates that handels the arel access.
	Rel=irrel(arel, _, _, _, _, _, arel(_, _, Attr2, Case, SourceRelT)),
	((Attr2=Pre:RestAttr, ground(Pre));RestAttr=Attr2),
	ensure(ground(RestAttr)),
	A=usedAttr(SourceRelT, attr(RestAttr, 0, Case)).

getCurrentARels(CurrentARelList) :-
  findall(A, (
      queryRel(_, RelT),
			RelT=rel(Rel, _),
			Rel=..[irrel,arel|_],
			A=RelT
    ), L1),
  findall(A, (
      variable(_, RelT),
			RelT=rel(Rel, _),
			Rel=..[irrel,arel|_],
			A=RelT
    ), L2),
	append(L1, L2, CurrentARelList).

/*
Returns all rels used in the query with the given SQID, works not
for the current query.
*/
getRelsBySQID(SQID, Rels) :-
  variablesBySQID(SQID, L1),
  queryRelsBySQID(SQID, L2),
  findall(X, member(variable(_, X), L1), LR1),
  findall(X, member(queryRel(_, X), L2), LR2),
  append(LR1, LR2, Rels).

setSQIDToPrevious(_SQID) :-
  \+ optimizerOption(nestedRelations),
	!. 

setSQIDToPrevious(SQID) :-
  optimizerOption(nestedRelations), 
  retractall(subqueryCurrent(_)),
  subqueryDFSLabel(PreviousSQID, SQID, _), % find the prev node
  asserta(subqueryCurrent(PreviousSQID)),
  restoreCurrentQueryInfos(PreviousSQID),
	!.

enterSubquery(_Type) :-
  \+ optimizerOption(nestedRelations), 
	!.

enterSubquery(Type) :-
  optimizerOption(nestedRelations),
  dm(nr, ['\nenterSubquery']),
  % Just ennumerate the subquries coz we have currently to other way to 
  % identify a query. In particular i'm interested in the parent queries to 
  % get some knowledge about these query variable bindings.
  getSubqueryCurrent(SQID),
  (retract(subqueryMaxLabel(Max)) ->
    true
  ;
    Max=0
	),
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
  ((variablesBySQID(SQID, VarsList),
  	findfirst(variable(Var, rel(_, Var)), VarsList, Result)) ->
    SQIDFound=SQID
  ; 
		(
			subqueryDFSLabel(PrevSQID, SQID, _),
      findBinding(PrevSQID, Var, Result, SQIDFound)
    )
  ).

/*
Like findBinding, but searches only in the direct outer query.
*/
findParentBinding(SQID, Var, Result, SQIDFound) :-
  subqueryDFSLabel(PrevSQID, SQID, _),
  findParentBinding(PrevSQID, Var, Result, SQIDFound).

findParentBinding(SQID, Var, Result, SQID) :-
  variablesBySQID(SQID, List),
  findfirst(variable(Var, rel(_, Var)), List, Result).

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
Extracts the select clause from a query.
getSelectFromQuery(+Query, ?Select)
~Select~ might be a list or direct the rel term.
*/
getSelectFromQuery(Query first _, Select) :-
  !, getSelectFromQuery(Query, Select).
getSelectFromQuery(Query last _, Select) :-
  !, getSelectFromQuery(Query, Select).
getSelectFromQuery(Query orderby _, Select) :-
  !, getSelectFromQuery(Query, Select).
getSelectFromQuery(Query groupby _, Select) :-
  !, getSelectFromQuery(Query, Select).

getSelectFromQuery(select Select from _, Select) :-
	!.

getSelectFromQuery(Query, Select) :-
  ErrMsg='Query not in expected format.',
  throw(error_Internal(nr_getSelectFromQuery(Query, Select)::ErrMsg)).

/*
Recognize terms as queries (even if the query themselves is not valid)
*/
simplifiedIsQuery(Query orderby _) :-
  simplifiedIsQuery(Query).
simplifiedIsQuery(Query groupby _) :-
  simplifiedIsQuery(Query).
simplifiedIsQuery(Query first _) :-
  simplifiedIsQuery(Query).
simplifiedIsQuery(Query last _) :-
  simplifiedIsQuery(Query).
simplifiedIsQuery(select _ from _ where _).
simplifiedIsQuery(select _ from _).

/*
Provable if the query SQID is a ~isStarQuery~.
*/
isStarQuery(SQID) :-
	isStarQueryBySQID(SQID, L),
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

/*
Not a very efficent version.
Example:
?- getRelDesc(orteh, X).
X = reldesc([[bevth, bevTH, u, orteh:bevth, int, noarel, sizeTerm(12, 5.0, 0)], [subrel, subRel, u, orteh:subrel, arel, areldesc([[kennzeichen, kennzeichen, u, orteh:subrel:kennzeichen, string, noarel, sizeTerm(60, 8.320158102766799, 0)], [ort, ort, u, orteh:subrel:ort, string, noarel, sizeTerm(60, 18.1600790513834, 0)], [vorwahl, vorwahl, u, orteh:subrel:vorwahl, string, noarel, sizeTerm(60, 10.725296442687746, 0)], [bevt, bevT, u, orteh:subrel:bevt, int, noarel, sizeTerm(12, 5.0, 0)]], sizeTerm(192, 42.20553359683794, 0)), sizeTerm(60, 90.9090909090909, 153.0909090909091)]], sizeTerm(72, 95.9090909090909, 153.0909090909091)).

It work's like getRelAttrList/3, but here the spelling is stored, too.

The list that is describing a attribute has the following format:

Attr = [DCAttr, AttrMod, DCFQN, Type, ARelTerm, AST],
DCAttr: The attribute name as it is, this will not change by the renaming.
AttrMod: the modified attribute name that is changing more or less often due to renaming. Except the case, this is how the attribute name will apear within the stream.
Case: u/l for AttrMode as usual
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
createAttrListFromQuery(LQuery, SQID, TOP, AttrList, ST) :-
  getRelsFromQuery(LQuery, Rels),
  makeList(Rels, LRels),
	createAttrListFromQuery2(LQuery, LRels, SQID, TOP, AttrList, ST).

createAttrListFromQuery(LQuery, SQID, TOP, AttrList, ST) :-
  ErrMsg='failed to create the attribute list for a given query.',
  throw(error_Internal(nr_createAttrListFromQuery(LQuery, SQID, TOP, AttrList,
		ST)::ErrMsg)).

createAttrListFromQuery2(_LQuery, LRels, SQID, TOP, AttrList2, ST) :-
  isStarQuery(SQID),
  % This fails when terms like [var.*, var.intatt*100 as xy] are allowed.
  % Then queryAttr terms are to be evaluated, too.
  attsFromRels(SQID, TOP, LRels, AttrList1, ST),
  applyTOP(TOP, LRels, AttrList1, AttrList2),
  !.

createAttrListFromQuery2(LQuery, LRels, SQID, TOP, AttrList2, ST) :-
  \+ isStarQuery(SQID),
  % This fails when terms like [var.*, var.intatt*100 as xy] are allowed.
  % Then queryAttr terms are to be evaluated, too.
  attsFromFacts(SQID, LQuery, TOP, LRels, AttrList1, ST),
  applyTOP(TOP, LRels, AttrList1, AttrList2),
  !.

/*
Transforms the irrel relation description ~AttrList~ based on the given transformation operator in the way the secondo text-syntax operator would change the structure.

applyTOP(+TOP, +LRels, +AttrList, -AttrListOut)
*/
applyTOP(notop, _, AttrList, AttrList) :-
	!.

applyTOP(unnest(arel, attrname(NAttr)), _, AttrList, AttrList2) :-
  NAttr=attr(Attr, _, _),
	nrSimpleFindAttribute(Attr, AttrList, AD, BeforeA, AfterA),
  AD=[_, _, _, _, _, areldesc(ARelAtts, _), _],
	appendLists([BeforeA, ARelAtts, AfterA], AttrList2),
  !.

applyTOP(unnest(mpoint, attrname(NAttr)), _, AttrList, AttrList2) :-
  NAttr=attr(Attr, _, _),
	nrSimpleFindAttribute(Attr, AttrList, AD, BeforeA, AfterA),
  AD   =[DCAttr, NewLabel, Case, FQN, Type, ARelD, _CSZ],
	ensure(Type=mpoint),
	% Can the upoint size and CSZ be used to adjust the card?!?
	secDatatype(upoint, MemSize, _FlobSize, _, _, _),
	CSZNEW=sizeTerm(MemSize, MemSize, 0),
  ADNEW=[DCAttr, NewLabel, Case, FQN, upoint, ARelD, CSZNEW],
	appendLists([BeforeA, [ADNEW], AfterA], AttrList2),
  !.

applyTOP(nest(Attrs3, LA, _), _, AttrList, AttrList4) :-
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
applyTOP(TOP, LRels, AttrList, AttrList) :-
  throw(error_Internal(nr_applyTOP(TOP, LRels, AttrList, AttrList))
		::'failed to change to attribute list based on the transformation operator.').

nrRemoveAttrs([], AttrList, AttrList, []).
nrRemoveAttrs([Attr|Rest], AttrList, AttrList3, RemovedNew) :-
	Attr=attrname(attr(Attr2, _, _)),
	nrSimpleFindAttribute(Attr2, AttrList, A, BeforeA, AfterA),
	append(BeforeA, AfterA, AttrList2),
	nrRemoveAttrs(Rest, AttrList2, AttrList3, Removed),
	append([A], Removed, RemovedNew).

/*
The three cases that a relation can be occur within the from list:
- a relation
- a arel attribut
- a query
So for a isStartQuery, all attributes needs to be collected.

attsFromRels(+SQID, +TOP, +RelList, -AttrList, -SizeTermResult)
*/
attsFromRels(_, _, [], [], sizeTerm(0, 0, 0)) :- 
	!.

attsFromRels(SQID, TOP, [Rel|RelRest], AttrList, SizeTermResult) :-
  attsFromRels(SQID, TOP, RelRest, AttrListRest, SizeTerm1),
  Rel=rel(RelDCName, Var),
  atomic(RelDCName),
  !,
  getRelDesc(RelDCName, reldesc(ResAttrList, SizeTerm)),
	nrRenameAttrList(ResAttrList, Var, ResAttrList2),
  append(ResAttrList2, AttrListRest, AttrList),
  addSizeTerms([SizeTerm1,SizeTerm], SizeTermResult).

attsFromRels(SQID, TOP, [Rel|RelRest], AttrList, SizeTermResult) :-
  attsFromRels(SQID, TOP, RelRest, AttrListRest, SizeTerm1),
  Rel=rel(T, _),
	T=irrel(_, _, _, _, SizeTerm, ResAttrList, _),
 	!,
	% Note that here is no renaming needed, this was already done.
  append(ResAttrList, AttrListRest, AttrList),
  addSizeTerms([SizeTerm1,SizeTerm], SizeTermResult).

attsFromRels(SQID, TOP, Rels, AttrList, SizeTermResult) :-
  throw(error_Internal(nr_attsFromRels(SQID, TOP, Rels, AttrList, 
		SizeTermResult)::failed)).

attsFromFacts(_SQID, LQuery, _TOP, _RelList, AttrList3, SizeTermResult) :-
	countQuery(LQuery), % This works also for already looked-up queries.
	AttrList3=[[count, count, u, fqn(no), int, noarel, sizeTerm(16, 5, 0)]],
  recomputeSizeTerm(AttrList3, SizeTermResult),
  !.

% Currently this is not allowed for aggregation queries.
% The problem here is to obtain the appropriate sizeTerm for an
% expression.	
attsFromFacts(_SQID, LQuery, _TOP, _RelList, AttrList3, SizeTermResult) :-
	isAggrQuery(LQuery), % This works also for already looked-up queries.
	% In this case, the size Term is may not correct.
	AttrList3=[[value, value, u, fqn(no), unknown, noarel, sizeTerm(16, 5, 0)]],
  recomputeSizeTerm(AttrList3, SizeTermResult),
  !.

/*
The nasty "trick" is here to exclude the unneeded attributes.
*/
attsFromFacts(SQID, LQuery, TOP, RelList, AttrList3, SizeTermResult) :-
  % This is way is choosen because it is pretty hard to collect these
  % information based on usedAttr and queryAttr facts. In particular
  % if they are coming from subqueries.
	getSelectFromQuery(LQuery, Select),
	btMakeList(Select, SelectL),
  attsFromRels(SQID, TOP, RelList, AttrList1, _),

  %usedAttrsBySQID(SQID, L1),
	onlyAttr(SelectL, L1),
  nrRemoveUnusedAttrs(L1, AttrList1, AttrList2),


	% Currently, queryAttr are only possible if this query is NOT a isStarQuery.
  queryAttrsBySQID(SQID, L2),
  addQueryAttr(SQID, L2, AttrList2, AttrList3),
  recomputeSizeTerm(AttrList3, SizeTermResult),
  !.

/*
All other entries within the select clause are queryAttr terms used in combination with the as xy term.
Note too: when a groupby is used, every column with a aggregate function need to get a name and then they are handley with the queryAttr case.
*/
onlyAttr([], []).
onlyAttr([attr(Name, Index, Case)|Rest], [attr(Name, Index, Case)|Rest2]) :-
	!,
	onlyAttr(Rest, Rest2).
onlyAttr([A|Rest], Rest2) :-
	A\=attr(_Name, _Index, _Case),
	!,
	onlyAttr(Rest, Rest2).
	
/*
Find a attribute by it's simple name that was never renamed.
This supports to write queries without manually add all renaming labels.

Note that this is only syntatic sugar and might return a wrong result.
But still it is much better as to manually adopt the renaming process during writing a query.
Another option is to extend the duplicate check method to even avoid this
name conflicts.
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
	Attr=DCAttr,
	addVar(DCAttr, Var, DCAttrVar),
  A1=[_,DCAttrVar|_]. % See getRelDesc for a structure description

nrContainsAttribute(A1, [L1|_]) :-
  L1=attr(Attr, _Index, Case),
  A1=[_,Attr,Case|_]. % See getRelDesc for a structure description

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

getQueryResultSize(select _ from RelT, Card, TupleSize) :-
  RelT=rel(irrel(_, _, _, Card, TupleSize, _, _), _).

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

% standard costs
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

/*
If the attriubte origin is a outer query, then it is needed to extract
the attribute from the tuple alias that is generated by the subqueries
extension.
Note: This can only be used with the "as Alias" term, otherwise it will handeld
wrong withint he selectClause predicate within the optimizer.pl file.
*/
adjustAttrTerm(AttrTerm, SQID, AttrTerm) :-
  getSubqueryCurrent(SQID),
  !.
adjustAttrTerm(AttrTerm, SRCSQID, attr(Alias, attrname(AttrTerm))) :-
  getSubqueryCurrent(SQID),
  SQID\=SRCSQID,
  getAlias(SRCSQID, Alias),
  !.

getAlias(SRCSQID, Alias) :-
  getSubqueryCurrent(SQID),
  getAlias(SQID, SRCSQID, Alias).

getAlias(SQID, SRCSQID, Alias) :-
  subqueryDFSLabel(PrevSQID, SQID, _),
  PrevSQID=SRCSQID,
  aliasBySQID(SQID, Alias).

getAlias(SQID, SRCSQID, Alias) :-
  subqueryDFSLabel(PrevSQID, SQID, _),
  PrevSQID\=SRCSQID,
  getAlias(PrevSQID, SRCSQID, Alias).

getAttributeDesc(Type, Var:Attr, Rel, SRCSQID, AD) :-
  atomic(Var),
  atomic(Attr),
  downcase_atom(Attr, AttrDC),
  findAttrLists(Type, SRCSQID, Var, Rel, AttrDesc),
  nrSimpleFindAttribute(AttrDC, AttrDesc, AD).

getAttributeDesc(Type, Attr, Rel, SRCSQID, AD) :-
  atomic(Attr),
  downcase_atom(Attr, AttrDC),
  findAttrLists(Type, SRCSQID, *, Rel, AttrDesc),
  nrSimpleFindAttribute(AttrDC, AttrDesc, AD).

nrLookupAttr(Var:Attr, AttrTermA) :-
  atomic(Var),
  atomic(Attr),
  downcase_atom(Attr, AttrDC),
  findAttrLists(attribute, SRCSQID, Var, Rel2, AttrDesc),
  nrSimpleFindAttribute(AttrDC, AttrDesc, AD),
  AD=[_, Attr2, Case|_],
  !,
  Attr2=_:RestAttr2,
	AttrTerm=attr(Attr2, 0, Case),
	adjustAttrTerm(AttrTerm, SRCSQID, AttrTermA),
	addUsedAttrIfNeeded(Rel2, attr(RestAttr2, 0, Case)).

nrLookupAttr(Attr, AttrTermA) :-
  atomic(Attr),
  downcase_atom(Attr, AttrDC),
  findAttrLists(attribute, SRCSQID, *, Rel2, AttrDesc),
  nrSimpleFindAttribute(AttrDC, AttrDesc, AD),
  AD=[_, Attr2, Case|_],
  !,
  Attr2=RestAttr2,
	AttrTerm=attr(Attr2, 0, Case),
	adjustAttrTerm(AttrTerm, SRCSQID, AttrTermA),
	addUsedAttrIfNeeded(Rel2, attr(RestAttr2, 0, Case)).

nrLookupAttr(Query as Name, OutQuery) :-
  simplifiedIsQuery(Query),
  !,
  nrLookupSubqueryAttr(Query as Name, OutQuery).

/*
A query within the attribute list may return an atomic value or a arel relation.
Simple aggregation queries delivers a atomic value because they are handely by the rewiring rules of the subqueries extension.
*/
nrLookupSubqueryAttr(Query as Name, OutQuery as attr(Name, 0, u)) :-
  \+ queryAttr(attr(Name, 0, u)), % Not already used
 	enterSubquery(attribute),
  lookupSubquery(Query, Query2),
  getSubqueryCurrent(SQID), % Remember the query node for later use.
  OutQuery = subquery(SQID, Query2, []),
  leaveSubquery,

  TOP=notop,
  createAttrListFromQuery(Query2, SQID, TOP, AttrList1, _),
  % Surrounding Rel2 with a subquery(_) term makes the a later check with 
  % simplifiedIsQuery expendable.
  OutQuery2 = subquery(SQID, Query2, [], AttrList1),
  getSubqueryCurrent(CSQID), % Remember the query node for later use.

	% Open issue: These facts needs only to be added if the query 
	% is not followed by a unnest or nest term.
  assertz(queryAttr(attr(Name, 0, u))),
	% Add additional information, because in the later process it is not 
	% very easy to obtain this information that belongs to this attribute.
  assertz(queryAttrDesc(CSQID, attr(Name, 0, u), OutQuery2)).

nrLookupSubqueryAttr(Query, OutQuery) :-
  throw(error_Internal(nr_lookupSubqueryAttr(Query, OutQuery)::failed)).

nrLookupPred1(Var:Attr, attr(Attr2, Index, Case), RelsBefore, RelsAfter) :-
  atomic(Var),
  atomic(Attr),
  downcase_atom(Attr, AttrDC),
  findAttrLists(predicate, _SRCSQID, Var, Rel2, AttrDesc),
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
  findAttrLists(predicate, _SRCSQID, *, Rel2, AttrDesc),
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
intermediate result relation-lookup

Note that here only the queryRel and variable facts are added.
*/
nrLookupRel(Term, RelT) :-
	nrLookupIRRel(Term, RelT),
	RelT=rel(_, Var),
	nrAssertRelTerm(Var, RelT),
	nrDuplicateAttrCheck. % A variable may not avoid in every case a duplicate 
	% attribute name. Even if within attributes the user avoided to use _ within
	% attirubtes names, because of nested relations and subqueries, this
	% might happen nevertheless. 
	% Example: 
	% select * 
	% from [(select * from orte as x first 1),
	%				 (select * from (select * from plz as x first 1)) ] first 1.

nrAssertRelTerm(*, RelT) :-
  assertz(queryRel(_, RelT)),
	!.

nrAssertRelTerm(Var, RelT) :-
	Var \= *,
  checkVarIsFree(Var),
  assertz(variable(Var, RelT)),
	!.

nrAssertRelTerm(Var, RRel) :-
	throw(error_Internal(nr_nrAssertRelTerm(Var, RRel)::failed)).

/*
Checks if a attribute name apears more than one time within the stream.
*/
nrDuplicateAttrCheck :-
  getSubqueryCurrent(SQID),
	%member(_Type1, [relation, attribute, predicate]),
	% Note that findAttrLists may generate more entries than needed because
	% of the type, but that dosen't affect the correctness.
	Type=attribute, % to check for conflicts, this is enaugh.
  findAttrLists(Type, SQID, _RelVar1, RelT1, AttrDescList1),
  findAttrLists(Type, SQID, _RelVar2, RelT2, AttrDescList2),
	RelT1 \= RelT2,
	member(AD1, AttrDescList1),
	member(AD2, AttrDescList2),
	AD1=[_, AS1, Case1|_],
	AD2=[_, AS2, Case2|_],
	AS1=AS2,
	Case1=Case2,
	nrApplyCase(AS1, Case1, Attr),
	atomic_list_concat(['\n\nERROR: The attribute name \'', Attr, 
		'\' appears more than once within the stream. Use the renaming option to ',
		'prevent this.'], ErrMsg),
	throw(error_SQL(nr_nrDuplicateAttrCheck::failed::ErrMsg)).
	
nrDuplicateAttrCheck :-
	!.

/*
Simple irrel lookup for a regular relation, used for later post processing.
for examle within unnest terms.
*/
nrLookupIRRel(Rel as Var, IRRel) :-
  atomic(Rel),
	nrLookupIRRel2(Rel as Var, IRRel).
nrLookupIRRel(Rel, IRRel) :-
  atomic(Rel),
	nrLookupIRRel2(Rel as *, IRRel).

/*
Possible within attribute queries or within predicate queries.
Not within the from clause queries (direct, of course, within the from query, attributes or predicates may contain queries with a valid call of this predicate).

Handels the four from <arel> cases:
var:arel 
var:arel as label
arel
arel as label

arel -> irrel
*/
nrLookupIRRel(RelVar:ARel as Var, RelT) :-
  nrLookupIRRel2(RelVar:ARel as Var, RelT).
nrLookupIRRel(RelVar:ARel, RelT) :-
  atomic(RelVar),
  atomic(ARel),
  nrLookupIRRel2(RelVar:ARel as *, RelT).
nrLookupIRRel(ARel, RelT) :-
  atomic(ARel),
  nrLookupIRRel2((*):ARel as *, RelT).
nrLookupIRRel(ARel as Var, RelT) :-
  atomic(ARel),
  atomic(Var),
  nrLookupIRRel2((*):ARel as Var, RelT).

% Nest
% irrel -> nest ireel
nrLookupIRRel(SrcRel nest(Attrs) as NewLabel as Var, RelT) :-
  nrLookupIRRel2(SrcRel nest(Attrs, NewLabel) as Var, RelT),
  !.
nrLookupIRRel(SrcRel nest(Attrs, NewLabel) as Var, RelT) :-
  nrLookupIRRel2(SrcRel nest(Attrs, NewLabel) as Var, RelT),
  !.
nrLookupIRRel(SrcRel nest(Attrs, NewLabel), RelT) :-
  nrLookupIRRel2(SrcRel nest(Attrs, NewLabel) as *, RelT),
  !.
nrLookupIRRel(SrcRel nest(Attrs) as NewLabel, RelT) :-
  % This is the reason for the nrLookupRel2 predicates,
  % otherwise the as * term will, of course, produce a infinite loop.
  nrLookupIRRel2(SrcRel nest(Attrs, NewLabel) as *, RelT),
  !.

% irrel -> unnest ireel
nrLookupIRRel(SrcRel unnest(Attr) as Var, RelT) :-
	nrLookupIRRel2(SrcRel unnest(Attr) as Var, RelT),
	!.
nrLookupIRRel(SrcRel unnest(Attr), RelT) :-
	% This is the reason for the nrLookupRel2 predicates,
	% otherwise the as * term will, of course, produce a infinite loop.
	nrLookupIRRel2(SrcRel unnest(Attr) as *, RelT),
	!.

% Query lookup
% query -> irrel
nrLookupIRRel(Query, RelT) :-
	nrLookupIRRel2(Query as *, RelT),	
	!.
nrLookupIRRel(Query as Var, RelT) :-
	nrLookupIRRel2(Query as Var, RelT),	
	!.

% Top-level Rel lookup
% rel -> irrel
% nrel -> irrel
% Note that this case happens only iff after this a nest or unnest will follow.
nrLookupIRRel2(Rel as Var, IRRel) :-
  atomic(Rel),
  downcase_atom(Rel, RelDCName),
  relation(RelDCName, _), 
	!,
	Stream=feed(rel(RelDCName, Var)), % A project can't be done here, it is later
	% done within the optimizer.
  getRelDesc(RelDCName, reldesc(AttrList1, SizeTerm)),
  nrRenameAttrList(AttrList1, Var, AttrList3),
	card(RelDCName, Card),
  ExSpec=relation(rel(RelDCName, Var)),
  IRRel=rel(irrel(relation, Stream, notop, Card, SizeTerm, AttrList3, 
		ExSpec), Var).

/*
arel -> irrel
*/
nrLookupIRRel2(RelVar:ARel as Var, RelT) :-
  atomicCheck([RelVar, ARel, Var]),
  downcase_atom(ARel, AttrDC),
  findAttrLists(relation, SRCSQID, RelVar, AttrRelT, AttrDescList),
  nrSimpleFindAttribute(AttrDC, AttrDescList, AD),
  AD=[_, Attr2, Case, DCFQN, Type, ARelTerm, _],

  ((RelVar \= *, Type \= arel) ->
    (
      ErrMsg='Source attribute has not the type arel. This is not allowed',
      throw(error_Internal(nr_nrLookupRel(RelVar:ARel as Var, RelT))::ErrMsg)
    )
  ;
    Type=arel % Just fail silently if RelVar=*...maybe ARel is
    % a regular relation and should be handeld by the other predicate.
    % if not, the user will see a default error message, wich maybe
    % hard to track it is a error within the program. Then it should
    % tried again with label.
  ),
  !,
  ARelTerm=areldesc(AttrList, ST1),
  nrRenameAttrList(AttrList, Var, AttrListR),
	
	% This is the lookup phase, but nevertheless is the plan generated here
	% Otherwise nested pog optimizatios can occur when the plan is created.
	% (this is the case for subqueries, but both cases are treated equally).
	getAlias(SRCSQID, Alias),
	Stream = afeed(attribute(Alias, a(Attr2, 0, Case))),
	card(DCFQN, Card),
	TOP=notop,
	ExSpec=arel(RelVar:ARel, DCFQN, Attr2, Case, AttrRelT), 
	RelT=rel(irrel(arel, Stream, TOP, Card, ST1, AttrListR, ExSpec), Var).

nrLookupIRRel2(SrcRel unnest(Attr) as Var, RelT) :-
	nrLookupIRRel(SrcRel, SrcRelT),
  !,
  SrcRelT=rel(irrel(_, SrcStream, SrcTOP, SrcCard, _, SrcAttrList, _), SrcVar),
	downcaseAttributeList(Attr, AttrDC),
	lastAttribute(AttrDC, AttrLastDC),
  (nrSimpleFindAttribute(AttrLastDC, SrcAttrList, AD) ->
		true
	;
    throw(error_SQL(nr_nrLookupIRRel2(SrcRel unnest(Attr) as Var, RelT)::
      'Attribute not found.'))
	),
  AD=[_, Attr2, Case, _, Type|_],
  (member(Type, [arel, mpoint]) ->
    true
  ;
    throw(error_SQL(nr_nrLookupIRRel2(SrcRel unnest(Attr) as Var, RelT)::
      'The unnest operator can only be applied on arel or mpoint attributes.'))
  ),

	addTransformationOperator(SrcStream, SrcTOP, Stream1),
	addRenameOperator(Stream1, SrcVar, Stream2),
	% Open issue
	Card is SrcCard*2, % no clue at the moment how to handle this.
	
	NAttr=attr(Attr2, 0, Case),
  TOP=unnest(Type, attrname(NAttr)),
  applyTOP(TOP, _, SrcAttrList, AttrList2),
  nrRenameAttrList(AttrList2, Var, AttrList3),
  recomputeSizeTerm(AttrList3, SizeTermResult),
  ExSpec=unnest(SrcRelT, NAttr),
  RelT=rel(irrel(unnest, Stream2, TOP, Card, SizeTermResult, AttrList3, 
		ExSpec), Var).

nrLookupIRRel2(SrcRel nest(Attrs, NewLabel) as Var, RelT) :-
	nrLookupIRRel(SrcRel, SrcRelT),
	!,
  SrcRelT=rel(irrel(_, SrcStream, SrcTOP, SrcCard, _, SrcAttrList, _), SrcVar),
  makeList(Attrs, AttrsL),
	lookupAD(AttrsL, SrcAttrList, Attrs2L),
  attrnames(Attrs2L, Attrs3),
  LA=attrname(attr(NewLabel, 0, u)),
  assertz(queryAttr(attr(NewLabel, 0, u))), % XXX

	addTransformationOperator(SrcStream, SrcTOP, Stream1),
	addRenameOperator(Stream1, SrcVar, Stream2),
	% open issue
	Card is SrcCard/2, % no clue at the moment how to handle this.

  TOP=nest(Attrs3, LA, []),
  applyTOP(TOP, _, SrcAttrList, AttrList2),
  nrRenameAttrList(AttrList2, Var, AttrList3),
  recomputeSizeTerm(AttrList3, SizeTermResult),
  ExSpec=nest(SrcRelT, Attrs, NewLabel),
  RelT=rel(irrel(nest, Stream2, TOP, Card, SizeTermResult, AttrList3, ExSpec),
		Var).


nrLookupIRRel2(Query as Var, RelT) :-
  simplifiedIsQuery(Query),
  enterSubquery(relation),
  getSubqueryCurrent(SQID),
  lookupSubquery(Query, Query2),

  % immediately create the plan.
  % Note that if a optimzation will performed, at every time only one optimize
  % process will be in progress.
	nrSubqueryToStream(Query2, Stream, _Costs),
  getQueryResultSize(Query2, Card, _),

  leaveSubquery,
  TOP=notop,
  createAttrListFromQuery(Query2, SQID, TOP, AttrList1, TupleSize),
  nrRenameAttrList(AttrList1, Var, AttrList2),

  % Surrounding Rel2 with a subquery(_) term makes the a later check with 
  % simplifiedIsQuery expendable.
  ExSpec=query(Query2, SQID),
 	RelT=rel(irrel(query, Stream, TOP, Card, TupleSize, AttrList2, ExSpec), Var),	
	!.

nrLookupIRRel2(Query as Var, RelT) :-
  simplifiedIsQuery(Query),
  throw(error_Internal(nr_nrLookupIRRel2(Query as Var, RelT)::failed)).

/*

*/
lookupAD([], _, []).
lookupAD([Attr|ARest], AL, [A2|ARest2]) :-
	lookupAD(ARest, AL, ARest2),
  downcaseAttributeList(Attr, AttrDC),
  lastAttribute(AttrDC, AttrLastDC),
  (nrSimpleFindAttribute(AttrLastDC, AL, AD) ->
    true
  ;
    throw(error_SQL(nr_lookupAD([Attr|ARest], AL, [A2|ARest2])::failed::
			'Attribute not found.'))
  ),
  AD=[_, Attr2, Case|_],
	A2=attr(Attr2, 0, Case).

/*
For count queries, here is fixed that within the secondo the sql dialect fulfills not the closure property. Within secondo, a count query returns a single value and not a relation. Here the value is transformed into a tuple stream.
This is just a work around, i think it would be better to implement a new count operator that returns a tuple stream (with one tuple) as a result that can be consumed.
The same is the case for max/min/avg etc.

Another extension would be nice, too. If count produces a tule stream, labeling a the attribute should be possible:
select count(*) as anzahl from orte.

nrSubqueryToStream(+Query, ?Stream, ?Costs) 

The stream returned is always a consumeable tuple stream.
*/
/*
Note that still both forms have the problem to ignore the addTmpVariables(+Stream, -Stream2) extensions. But it seems distance-related, so it is not supported anyway.
nrSubqueryToStream(Query, Stream, Costs) :-
  queryToPlan(Query, Stream1, Costs),
	nrExtractStream(Stream1, Stream).

nrExtractStream(consume(Stream), Stream).
nrExtractStream(aconsume(Stream), Stream).
nrExtractStream(count(StreamIn), StreamOut) :-
	Attr=attr(count, 0, u),
  attrnames([Attr], AAL),
  StreamOut=namedtransformstream(feed(StreamIn), AAL).
*/

nrSubqueryToStream(Query, Stream, Costs) :-
	countQuery(Query), 
  queryToStream(Query, Stream1, Costs),
	Attr=attr(count, 0, u),
	atomicValueToTupleStream(count(Stream1), Attr, Stream).

nrSubqueryToStream(Query, Stream, Costs) :-
	isAggrQuery(Query), 
	% if addTmpVariables(Stream2, StreamOut), is used there, it won't work 
	% this way.
  queryToPlan(Query, Stream1, Costs), 
	Attr=attr(value, 0, u),
	atomicValueToTupleStream(Stream1, Attr, Stream).

% implement more rules for sum, min etc. 
% Note that then the createAttrListFromQuery predicates needs to be modified, 
% too.
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
  \+ nrIsStar(Var),
  variable(Var, RelT),
	extractAttrList(RelT, AttrList).

getCurrentAttrList(_, *, RelT, AttrList) :-
  queryRel(_, RelT),
	extractAttrList(RelT, AttrList).

getCurrentAttrList(SQID, Var, RelT, AttrList) :-
  \+ nrIsStar(Var),
  variable(Var, RelT),
  attsFromRels(SQID, notop, [RelT], AttrList, _).

getCurrentAttrList(SQID, *, RelT, AttrList) :-
  queryRel(_, RelT),
  attsFromRels(SQID, notop, [RelT], AttrList, _).

getParentAttrList(SQID, Var, RelT, AttrList, FoundSQID) :-
  %Var \= *,
  \+ nrIsStar(Var),
	%findParentBinding(SQID, Var, Result, _), % 1 level search
	findBinding(SQID, Var, Result, FoundSQID), % Multilevel
  Result=variable(Var, RelT),
  extractAttrList(RelT, AttrList).

getParentAttrList(SQID, Var, RelT, AttrList, FoundSQID) :-
  %Var \= *,
  \+ nrIsStar(Var),
	%findParentBinding(SQID, Var, Result, FoundSQID), % 1 level search
	findBinding(SQID, Var, Result, FoundSQID), % Multilevel
  Result=variable(Var, RelT),
  attsFromRels(FoundSQID, notop, [RelT], AttrList, _).

getParentAttrList(SQID, *, RelT, AttrList, PrevSQID) :-
  %subqueryDFSLabel(PrevSQID, SQID, _), % 1 level search
	reverseTraversePath(SQID, PrevSQID), % Multilevel
  queryRelsBySQID(PrevSQID, List),
	member(queryRel(_, RelT), List),
  extractAttrList(RelT, AttrList).

getParentAttrList(SQID, *, RelT, AttrList, PrevSQID) :-
  %subqueryDFSLabel(PrevSQID, SQID, _), % 1 level search
	reverseTraversePath(SQID, PrevSQID), % Multilevel
  queryRelsBySQID(PrevSQID, List),
	member(queryRel(_, RelT), List),
  attsFromRels(PrevSQID, notop, [RelT], AttrList, _).

/*
Traverses the path backwards by backtracking from SQID to the root.
*/
reverseTraversePath(SQID, PrevSQID) :-
  subqueryDFSLabel(PrevSQID, SQID, _).

reverseTraversePath(SQID, PrevPrevSQID) :-
  subqueryDFSLabel(PrevSQID, SQID, _),
	reverseTraversePath(PrevSQID, PrevPrevSQID).

/*
With the lookup of attribute, the attribute may come from the attribute
gather together within the from clause.
Of course, lookups for outer query attribute may usefull and possible,
but this is not implemented.
findAttrLists(+Mode, ?SQID, ?Var, -RelT, -AttrList)
*/
findAttrLists(Mode, SQID, Var, RelT, AttrList) :-
  getSubqueryCurrent(SQID),
	member(Mode, [attribute, predicate]),
	getCurrentAttrList(SQID, Var, RelT, AttrList).

/*
Case: relation
A arel attribute can only obtained from the sourrounding query.

Either with just the name, or by dereferencing a variable like var:arel.

RelVar has always to be a term hat isn't confusing prolog, so e.g. starts not with uppercase letters.

Case: predicate
For predicates, one more this must be allowed, it is to
reference a outer query attribute.

*/
findAttrLists(Mode, FoundSQID, Var, RelT, AttrList) :-
  getSubqueryCurrent(SQID),
	member(Mode, [attribute, relation, predicate]),
	getParentAttrList(SQID, Var, RelT, AttrList, FoundSQID).

% Special case: reference without var:_ can access other attributes that
% are not always attributes of relations.
findAttrLists(Mode, PrevSQID, *, norel, AttrList) :-
  getSubqueryCurrent(SQID),
	member(Mode, [relation, predicate]),
  subqueryDFSLabel(PrevSQID, SQID, _),
  queryAttrsBySQID(PrevSQID, L),
	member(queryAttr(Attr), L),
	Attr=attr(Label, _, Case),
	queryAttrDesc(PrevSQID, Attr, SQT),
	SQT=subquery(_, _, _, ARelAttrList),
	downcase_atom(Label, LabelDC),
	recomputeSizeTerm(ARelAttrList, SizeTermResult),
	ARelDesc=areldesc(ARelAttrList, SizeTermResult),
  AD=[LabelDC, Label, Case, _, arel, ARelDesc, SizeTermResult],
	AttrList=[AD].

findAttrLists(Mode, SRCSQID, Var, RelT, AttrList) :-
	\+ member(Mode, [relation, attribute, predicate]),
	throw(error_Internal(nr_findAttrLists(Mode, SRCSQID, Var, RelT, AttrList)::
		'illegal call')).

/*
Extract the attribute list if the list is stored within the rel term.
*/
extractAttrList(rel(irrel(_, _, _, _, _, AttrList, _), _), AttrList).

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
	atomic_list_concat([ExtSpell, '_', A], Atom),
	!.
	
nrApplyCase(Attr, u, ExtSpell) :-
	atomic(Attr),
	upper(Attr, ExtSpell),	
	!.
nrApplyCase(Attr, l, Attr) :-
	atomic(Attr),
	!.

nrApplyCase(Attr, Case, AttrOut) :-
	Case \= l,
	Case \= u,
	throw(error_Internal(nr_nrApplyCase(Attr, Case, AttrOut)::invalidCase)).

/*
StreamIn is a plan that creates a tuple stream, on this stream, the stream is transformed based on the transformation operation(TOP) to create the ~StreamOut~ plan.
addTransformationOperator(+StreamIn, +TOP, -StreamOut)
*/
addTransformationOperator(Stream, notop, Stream) :- 
	!.

addTransformationOperator(Stream1, unnest(arel, Attr), Stream2) :-
  Stream2=unnest(Stream1, Attr), 
	!.

% Example: 
% extendstream[UTrip: units(.Trip)] remove[Trip] renameattr[Trip: UTrip] consume
addTransformationOperator(Stream1, unnest(mpoint, Attr), Stream4) :-
	Attr=attrname(attr(A, Index, Case)),
  newUniqueVar('tempattr', TempAttr),
	AttrN=attr(TempAttr, Index, Case),
	NewAttr=newattr(attrname(AttrN), units(attr(A, Index, Case))),
  Stream2=extendstream(Stream1, NewAttr),
  Stream3=remove(Stream2, Attr),
  Stream4=renameattr(Stream3, newattr(Attr, attrname(AttrN))),
	!.

addTransformationOperator(Stream1, nest(Attrs, NewLabel, SortAttrs), Stream4) :-
  % The nest operation expect a sorted stream.
  Stream2=sortby(Stream1, Attrs),
  Stream3=nest(Stream2, Attrs, NewLabel), 
  % Open issue: restoring the sort order after the nest operation, but this
  % not easy at all.
	nrRestoreSort(Stream3, SortAttrs, Stream4),
	!.

addTransformationOperator(StreamIn, TOP, StreamOut) :-
	throw(error_Internal(nr_addTransformationOperator(StreamIn, TOP, StreamOut)
		::failed)).

/*
Add the rename operator only if neede to the stream.
*/
addRenameOperator(Stream, *, Stream) :-
	!.

addRenameOperator(StreamIn, Rename, StreamOut) :-
  Rename \= *,
  StreamOut=rename(StreamIn, Rename),
	!.


/*
Special version to restore a given sort order.
*/
nrRestoreSort(Stream, [], Stream) :-
	!. 

nrRestoreSort(StreamIn, SortAttrs, StreamOut) :-
	!, 
	StreamOut=sortby(StreamIn, SortAttrs).

/*
This is used within the statistics.pl to identfy relation term that
are not working currently with statistics.
*/
nrRel(Rel) :-
  Rel = rel(T, _),
  T =.. [irrel|_].
nrRel(pr(_, Rel1, _)) :-
  nrRel(Rel1).
nrRel(pr(_, _, Rel2)) :-
  nrRel(Rel2).
nrRel(pr(_, Rel)) :-
  nrRel(Rel).

/*
Open issue
Currently for nested relation related queries, selectivity estimation and 
PET's are faked.
*/
nrSelectivity(pr(Pred, Rel1, Rel2), Sel, CalcPET, ExpPET) :-
  optimizerOption(nestedRelations),
  % Recognize if it is the nested relations case
  nrRel(pr(Pred, Rel1, Rel2)),
  dm(nr, ['\nFake nestedRelation selectivity, pred: ', Pred]),
  ensure(simplePred(pr(Pred, Rel1, Rel2), PSimple)),
  Sel=0.1,
  CalcPET=0.1,
  ExpPET=0.1,
  databaseName(DB),
  % unfortunately it is still necessary to store this faked results.
  assert(storedPET(DB, PSimple, CalcPET, ExpPET)),
  assert(storedSel(DB, PSimple, Sel)),
  !.

nrSelectivity(pr(Pred, Rel), Sel, CalcPET, ExpPET) :-
  optimizerOption(nestedRelations),
  % Recognize if it is the nested relations case
  nrRel(pr(Pred, Rel)),
  dm(nr, ['\nFake nestedRelation selectivity, pred: ', Pred]),
  ensure(simplePred(pr(Pred, Rel), PSimple)),
  Sel=0.1,
  CalcPET=0.1,
  ExpPET=0.1,
  databaseName(DB),
  % unfortunately it is still necessary to store this faked results.
  assert(storedPET(DB, PSimple, CalcPET, ExpPET)),
  assert(storedSel(DB, PSimple, Sel)),
  !.

/*
The assignment of an alias is to late for this extension within the subquery extension, so this is behaviour was changed. The new strategy is to assgin an alias
name if the lookup of a query is done. When to plan is generated, the newAlias predicate of the subquery extension returns this stored value and not a new alias.
*/
nrAssignAlias :-
  \+ optimizerOption(nestedRelations).

nrAssignAlias :-
  optimizerOption(nestedRelations),
	newUniqueVar('alias', Alias),
  getSubqueryCurrent(SQID),
  assertz(aliasBySQID(SQID, Alias)),
	!.

nrAssignAlias :-
	throw(error_Internal(nr_nrAssignAlias::failed)).

/*
nrIsStar is used to support backtracking without var is bounded to a ground term. This is done here because Var \= * returns false if var(Var) is true and (var(X) ; Var \= *) is not a good alternative(backtracking).
*/
nrIsStar(X) :-
	var(X), 
	!, 
	fail.

nrIsStar(*) :- 
	!.

/*
Recognize simple aggr queries
Works even if Query is a alreday looked-up query.
*/
isAggrQuery(Query) :-
	catch(aggrQuery(Query, _, _, _), error_SQL(optimizer_aggrQuery(_, undefined, 
		undefined)::malformedExpression::_), fail).

% eof
