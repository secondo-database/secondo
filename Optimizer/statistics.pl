/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [10] title: [{\Large \bf ]  [}]
//[toc] [\tableofcontents]


[10] Statistics

[File ~statistics.pl~]

While module [~database.pl~] handles basic information on database objects, like
types/ schemas, tuple sizes, cardinalities etc. this module deals with statistics
on databases, that rely to predicate evaluation.

Above all, it covers the estimation of predicate selectivities, predicate
evaluation times (PETs), and type inference for expressions.

[toc]

1 Information about Selectivity of Predicates

1.1 Rules about Commutativity of Predicates

~isCommutativeOP/1~ is defined in file ``operators.pl''.

*/

% general rules of commution
commute(Term,_,Commuted) :-
  member((Term,Commuted),[(X < Y,Y > X),(X <= Y, Y >= X),(X > Y, Y < X),
                          (X >= Y, Y <= X),(trcoveredby(X,Y), trcovers(Y,X))]),
  !.
% special rules inferred from operator characteristics
commute(Term,ResList,Commuted) :-
  optimizerOption(determinePredSig),
  Term =.. [Op, Arg1, Arg2],
  getTypeTree(Term,ResList,[_,Args,_]),
  findall(T,member([_,_,T],Args),ArgTypes),
  isCommutativeOP(Op,ArgTypes),
  Commuted =.. [Op, Arg2, Arg1], !.
% old rule - still using old ~isCommutativeOP/1~-facts
commute(Pred1, _, Pred2) :-
  not(optimizerOption(determinePredSig)),
  Pred1 =.. [OP, Arg1, Arg2],
  isCommutativeOP(OP),
  Pred2 =.. [OP, Arg2, Arg1], !.

% binary version - extracting rellist from predicate descriptor
commute(Pred1, Pred2) :-
  ( Pred1 = pr(P,A)
    -> RL = [(1,A)]
    ; ( Pred1 = pr(P,A,B)
        -> RL = [(1,A),(2,B)]
        ; fail
      )
  ),
  commute(Pred1,RL,Pred2),!.

/*

1.3 Determine the Simple Form of Predicates

Simple forms of predicates are stored in predicate ~sel~ or
in predicate ~storedSel~.


----	simple(+Term, +ArgRelList, -Simple) :-
----

The simple form of a term ~Term~ containing attributes of ~Rel1~ and/or ~Rel2~
is ~Simple~.

Commutative predicates are transformed to the minimum equivalent expression
regarding lexicographical order on the arguments, e.g.

--- op(b,a)
---

is transformed into

--- op(a,b)
---


*/

simple(attr(Var:Attr, 0, _), RelList, Rel2:Attr2) :-
  memberchk((1,rel(Rel, Var)),RelList),
  downcase_atom(Rel,Rel2), downcase_atom(Attr,Attr2), !.
simple(attr(Attr, 0, _), RelList, Rel2:Attr2) :-
  memberchk((1,rel(Rel, *)),RelList),
  downcase_atom(Rel,Rel2), downcase_atom(Attr,Attr2), !.
simple(attr(Var:Attr, ArgNo, _), RelList, Rel2:Attr2) :-
  memberchk((ArgNo,rel(Rel, Var)),RelList),
  downcase_atom(Rel,Rel2), downcase_atom(Attr,Attr2), !.
simple(attr(Attr, ArgNo, _), RelList, Rel2:Attr2) :-
  memberchk((ArgNo,rel(Rel, *)),RelList),
  downcase_atom(Rel,Rel2), downcase_atom(Attr,Attr2), !.
simple(dbobject(X),_,dbobject(X)) :- !.

simple([], _, []) :- !.
simple([A|Rest], RelList, [Asimple|RestSimple]) :-
  simple(A,RelList,Asimple),
  simple(Rest,RelList,RestSimple),
  !.

% Normalize simple predicates, if possible
simple(Term,RelList,Simple) :-
  compound(Term),
  Term =.. [_,Arg1,Arg2],
  compare(>,Arg1,Arg2),
  commute(Term,RelList,Term2),
  simple(Term2,RelList,Simple),
% dm(selectivity,['Commuting arguments: ',Term, ' changed to ',Simple,'.\n']),
  !.

simple(Term, RelList, Simple) :-
  compound(Term),
  Term =.. [Op|Args],
  simple(Args, RelList, ArgsSimple),
  Simple =..[Op|ArgsSimple],
  !.

simple(Term, _, Term).

% fallback-clause for calls to old clause
% ---  simple(+Term,+R1,+R2,-Simple)
simple(Term,R1,R2,Simple) :-
  dm(selectivity,['$$$$$$$ Using simple/4 fallback clause! $$$$$$$\n']),
  simple(Term,[(1,R1),(2,R2)],Simple),!.

/*

----	simplePred(Pred, Simple) :-
----

The simple form of predicate ~Pred~ is ~Simple~.

*/
% old clauses used, if option ~determinePredSig~ is not set:
simplePred(pr(P, A, B), Simple) :-
  not(optimizerOption(determinePredSig)),
  optimizerOption(subqueries),
  simpleSubqueryPred(pr(P, A, B), Simple),!.

simplePred(pr(P, A, B), Simple) :-
  not(optimizerOption(determinePredSig)),
  simple(P, A, B, Simple), !.

simplePred(pr(P, A), Simple) :-
  not(optimizerOption(determinePredSig)),
  simple(P, A, A, Simple), !.

simplePred(X, Y) :-
  not(optimizerOption(determinePredSig)),
  term_to_atom(X,Xt),
  concat_atom(['Malformed expression: \'', Xt, '\'.'],'',ErrorMsg),
  throw(error_SQL(statistics_simplePred(X, Y):malformedExpression#ErrorMsg)),!.


% with option ~determinePredSig~ a specialized version is called:
simplePred(Pred,Simple) :-
  optimizerOption(determinePredSig),
  ( Pred = pr(P,A,B)
     -> RelList = [(1,A),(2,B)]
     ;  ( Pred = pr(P,A)
          -> RelList = [(1,A)]
          ;  ( throw(error_Internal(statistics_simplePred(Pred, Simple)
                  :simplificationFailed)),
               fail %% error!
             )
        )
  ),
  simple(P,RelList,Simple), !.

% prompt an error, if simplification failed:
simplePred(Pred,Simple) :-
  throw(error_Internal(statistics_simplePred(Pred, Simple)
                  :simplificationFailed)),
  !, fail.

/*

1.4 Handling Selectivities, PETs and MBR Sizes

Selectivities, bounding box (MBR) intersection selectivities and average
bounding box sizes are estimated using a sample approach. The according
information is added to the knowledge base and saved to files when quitting
the optimizer.

Auxiliary predicates for ~selectivity~.

*/

sampleS(rel(Rel, Var), rel(Rel2, Var)) :-
  atom_concat(Rel, '_sample_s', Rel2).

sampleJ(rel(Rel, Var), rel(Rel2, Var)) :-
  atom_concat(Rel, '_sample_j', Rel2).

% create the name of a selection-sample for a relation
sampleNameS(lc(Name), lc(Sample)) :-
  atom_concat(Name, '_sample_s', Sample), !.
sampleNameS(Name, Sample) :-
  atom_concat(Name, '_sample_s', Sample), !.

% create the name of a join-sample for a relation
sampleNameJ(lc(Name), lc(Sample)) :-
  atom_concat(Name, '_sample_j', Sample), !.
sampleNameJ(Name, Sample) :-
  atom_concat(Name, '_sample_j', Sample), !.

sampleNameSmall(lc(Name), lc(Small)) :-
  atom_concat(Name, '_small', Small), !.
sampleNameSmall(Name, Small) :-
  atom_concat(Name, '_small', Small), !.

possiblyRename(Rel, Renamed) :-
  Rel = rel(_, *),
  !,
  Renamed = feed(Rel).

possiblyRename(Rel, Renamed) :-
  Rel = rel(_, Name),
  Renamed = rename(feed(Rel), Name).

dynamicPossiblyRenameJ(Rel, Renamed) :-
  Rel = rel(_, *),
  !,
  secOptConstant(sampleJoinMaxCard, JoinSize),
  secOptConstant(sampleScalingFactor, SF),
  Renamed = sample(Rel, JoinSize, SF).

dynamicPossiblyRenameJ(Rel, Renamed) :-
  Rel = rel(_, Name),
  secOptConstant(sampleJoinMaxCard, JoinSize),
  secOptConstant(sampleScalingFactor, SF),
  Renamed = rename(sample(Rel, JoinSize, SF), Name).

dynamicPossiblyRenameS(Rel, Renamed) :-
  Rel = rel(_, *),
  !,
  secOptConstant(sampleSelMaxCard, SelectionSize),
  secOptConstant(sampleScalingFactor, SF),
  Renamed = sample(Rel, SelectionSize, SF).

dynamicPossiblyRenameS(Rel, Renamed) :-
  Rel = rel(_, Name),
  secOptConstant(sampleSelMaxCard, SelectionSize),
  secOptConstant(sampleScalingFactor, SF),
  Renamed = rename(sample(Rel, SelectionSize, SF), Name).


/*
----
  selectivityQuerySelection(+Pred, +Rel,
                            -QueryTime, -BBoxResCard, -FilterResCard)

  selectivityQueryJoin(+Pred, +Rel1, +Rel2,
                       -QueryTime, -BBoxResCard, -FilterResCard)
----

The cardinality query for a selection predicate is performed on the selection sample. The cardinality query for a join predicate is performed on the first ~n~ tuples of the selection sample vs. the join sample, where ~n~ is the size of the join sample. It is done in this way in order to have two independent samples for the join, avoiding correlations, especially for equality conditions.

If ~optimizerOption(dynamicSample)~ is defined, dynamic samples are used instead of the \_sample\_j / \_sample\_s resp. \_small relations.

The predicates return the time ~QueryTime~ used for the query, and the cardinality ~FilterResCard~ of the result after applying the predicate.

If ~Pred~ has a predicate operator that performs checking of overlapping minimal bounding boxes, the selectivity query will additionally return the cardinality after the bbox-checking in ~BBoxResCard~, otherwise its return value is set to constant  ~noBBox~.

*/

/*
---- getBBoxIntersectionTerm(+Arg1,+Arg2,+Dimension,-PredTerm)
----

Return the bbox-selectivity-predicate ~PredTerm~ for for ~Dimension~ dimensions
for arguments ~Arg1~ and ~Arg2~.

*/
getBBoxIntersectionTerm(Arg1,Arg2,2,PredTerm) :-
  PredTerm   =.. [intersects, box2d(bbox(Arg1)), box2d(bbox(Arg2))], !.
getBBoxIntersectionTerm(Arg1,Arg2,_,PredTerm) :-
  PredTerm   =.. [intersects, bbox(Arg1), bbox(Arg2)], !.

/*

----
  bboxSizeQuerySel(+Term,+Rel,+TermTypeTree,-Size)
  bboxSizeQueryJoin(+Term,+Rel1,+Rel2,+TermTypeTree,-Size)
----

Query for the average size (volume) of ~Term~'s MBR (in a selection or join
predicate) and store it within the knowledge base.

Only do so, if ~optimizerOption(determinePredSig)~ is active.

Returns ~Size~ = ~none~, if the term has no bounding box, or
~optimizerOption(determinePredSig)~ is not active.

*/

% For terms within selection predicates:
%--- bboxSizeQuerySel(+Term,+Rel,+TermTypeTree,-Size)
bboxSizeQuerySel(_,_,_,none) :-
  not(optimizerOption(determinePredSig)),!.

bboxSizeQuerySel(Term,Rel,_,Size) :-
  simple(Term, [(1,Rel)], SimpleTerm),
  databaseName(DB),
  storedBBoxSize(DB,SimpleTerm,Size), !. % already known

bboxSizeQuerySel(Term,Rel,[_,_,T],Size) :-
  dm(selectivity,['\nbboxSizeQuerySel(',Term,', ',Rel,', [_,_,',
                  T,'], ',Size,') called!\n']),
  ( isKind(T,spatial2d) ; memberchk(T,[rect,rect3,rect4,rect8,
                                     mpoint,upoint,ipoint,
                                     instant,periods,
                                     movingregion,intimeregion,uregion]) ),
  simple(Term, [(1,Rel)], SimpleTerm),
  Rel = rel(DCrelName, _),
  ( optimizerOption(dynamicSample)
    -> dynamicPossiblyRenameS(Rel, RelQuery)
    ;  ( ensureSampleSexists(DCrelName),
         sampleS(Rel, RelS),
         possiblyRename(RelS, RelQuery)
       )
  ),
  newVariable(NewAttr),
  SizeQueryT = avg(projectextend(RelQuery,
                                 [],
                                 [field(attr(NewAttr,0,l),size(bbox(Term)))]),
                   NewAttr),
  plan_to_atom(SizeQueryT,SizeQueryA),
  concat_atom(['query ',SizeQueryA],'',SizeQuery),
  dm(selectivity,['\nThe Avg-Size query is: ', SizeQuery, '\n']),
  secondo(SizeQuery,[real,Size]),
  databaseName(DB),
  assert(storedBBoxSize(DB,SimpleTerm,Size)), !. % store it

bboxSizeQuerySel(Term,Rel,[_,_,T],none) :-
  dm(selectivity,['bboxSizeQuerySel/5: Term \'',Term,'\' for relation ',Rel,
            ' has Type ',T,', but no bbox!\n\n']),!. % no bbox available


% For terms within join predicates:
% --- bboxSizeQueryJoin(+Term,+Rel1,+Rel2,+TermTypeTree,-Size)
bboxSizeQueryJoin(_,_,_,_,none) :-
  not(optimizerOption(determinePredSig)),!.

bboxSizeQueryJoin(Term,Rel1,Rel2,_,Size) :-
  simple(Term, [(1,Rel1),(2,Rel2)], SimpleTerm),
  databaseName(DB),
  storedBBoxSize(DB,SimpleTerm,Size), !. % already known

% --- bboxSizeQueryJoin(+Term,+Rel1,+Rel2,+TermTypeTree,-Size)
bboxSizeQueryJoin(Term,Rel1,Rel2,[_,_,T],Size) :-
  dm(selectivity,['\nbboxSizeQueryJoin(',Term,', ',Rel1,', ',Rel2,', [_,_,',
                  T,'], ',Size,') called!\n']),
  ( isKind(T,spatial2d) ; memberchk(T,[rect,rect3,rect4,rect8,
                                     mpoint,upoint,ipoint,
                                     instant,periods,
                                     movingregion,intimeregion,uregion]) ),
  simple(Term, [(1,Rel1),(2,Rel2)], SimpleTerm),
  Rel1 = rel(DCrelName1, _),
  Rel2 = rel(DCrelName2, _),
  transformPred(Term, txx1, 1, Term2),
  newVariable(NewAttr),
  ( optimizerOption(dynamicSample)
    -> ( dynamicPossiblyRenameJ(Rel1, Rel1Query),
         dynamicPossiblyRenameJ(Rel2, Rel2Query),
         SizeQueryT = avg(projectextend(
            loopjoin(Rel1Query, fun([param(txx1, tuple)], Rel2Query)),
            [],
            [field(attr(NewAttr,0,l),size(bbox(Term2)))]),NewAttr)
       )
    ;  ( ensureSampleSexists(DCrelName1),
         ensureSampleJexists(DCrelName2),
         sampleS(Rel1, Rel1S),
         sampleJ(Rel2, Rel2S),
         possiblyRename(Rel1S, Rel1Query),
         possiblyRename(Rel2S, Rel2Query),
         Rel2S = rel(BaseName, _),
         card(BaseName, JoinSize),
         SizeQueryT = avg(projectextend(
            loopjoin(head(Rel1Query, JoinSize),
            fun([param(txx1, tuple)], Rel2Query)),
            [],
            [field(attr(NewAttr,0,l),size(bbox(Term2)))]),NewAttr)
       )
  ),
  plan_to_atom(SizeQueryT,SizeQueryA),
  concat_atom(['query ',SizeQueryA],'',SizeQuery),
  dm(selectivity,['\nThe Avg-Size query is: ', SizeQuery, '\n']),
  secondo(SizeQuery,[real,Size]),
  databaseName(DB),
  assert(storedBBoxSize(DB,SimpleTerm,Size)), !. % store it

bboxSizeQueryJoin(Term,Rel1,Rel2,[_,_,T],none) :- % no bbox available
  dm(selectivity,['bboxSizeQueryJoin/5: Term \'',Term,'\' for relations ',Rel1,
    ' and ',Rel2,' has Type ',T,', but no bbox!\n\n']),!.

/*
Now, we are prepared to formulate the actual predicates committing the
selectivity queries:

*/

% spatial predicate with bbox-checking
selectivityQuerySelection(Pred, Rel, QueryTime, BBoxResCard,
        FilterResCard) :-
  Pred =.. [OP, Arg1, Arg2],
  ( optimizerOption(determinePredSig)
    -> ( getTypeTree(Pred,[(1,Rel)],[OP,ArgsTrees,bool]),
         findall(T,member([_,_,T],ArgsTrees),ArgsTypeList),
         isBBoxOperator(OP,ArgsTypeList,Dim),
         getBBoxIntersectionTerm(Arg1,Arg2,Dim,BBoxPred),
         ArgsTrees = [Arg1Tree,Arg2Tree],
         bboxSizeQuerySel(Arg1,Rel,Arg1Tree,_),
         bboxSizeQuerySel(Arg2,Rel,Arg2Tree,_)
       )
    ;  ( isBBoxPredicate(OP),
         getBBoxIntersectionTerm(Arg1,Arg2,std,BBoxPred)
       )
  ),
  ( optimizerOption(dynamicSample)
    -> dynamicPossiblyRenameS(Rel, RelQuery)
    ;  ( Rel = rel(DCrelName, _),
         ensureSampleSexists(DCrelName),
         sampleS(Rel, RelS),
         possiblyRename(RelS, RelQuery)
       )
  ),
  dm(selectivity,['===== spatial unary predicate with bbox-checking =====\n']),
  Query = count(filter(counter(filter(RelQuery, BBoxPred),1), Pred)),
  transformQuery(Rel, Pred, Query, Query2),
  plan_to_atom(Query2, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  dm(selectivity,['\nSelectivity query : ', QueryAtom, '\n']),
  getTime(
    ( secondo(QueryAtom, ResultList)
      -> (true)
      ;  ( term_to_atom(Pred,PredA),
           concat_atom(['Selectivity query failed: Please check ',
                       'whether predicate \'', PredA, '\' is a boolean ',
                       'function!'],'',ErrorMsg),
           write_list(['\nERROR:\t',ErrorMsg,' ']), nl,
           throw(error_SQL(statistics_selectivityQuerySelection(Pred, Rel,
               QueryTime, BBoxResCard, FilterResCard)
               :selectivityQueryFailed#ErrorMsg)),
           fail
         )
    )
    ,QueryTime
  ),
  clearSelectivityQuery(Rel, Pred),
  ( ResultList = [int, FilterResCard]
    -> true
    ;  ( write_list(['\nERROR:\tUnexpected result list format during ',
                     'selectivity query:\n',
                     'Expected \'[int, <intvalue>]\' but got \'',
                     ResultList, '\'.']), nl,
         throw(error_Internal(statistics_selectivityQuerySelection(
                         Pred, Rel, QueryTime, BBoxResCard,FilterResCard)
                         :unexpectedListType)),
         fail
       )
  ),
  dm(selectivity,['Elapsed Time: ', QueryTime, ' ms\n']),
  secondo('list counters',  ResultList2),
  ( ResultList2 = [[1, BBoxResCard]|_]
    -> true
    ;  ( write_list(['\nERROR:\tUnexpected result list format during ',
                     'list counters query:\n',
                     'Expected \'[[1, BBoxResCard]|_]\' but got \'',
                     ResultList2, '\'.']), nl,
         throw(error_Internal(statistics_selectivityQuerySelection(
                         Pred, Rel, QueryTime, BBoxResCard,FilterResCard)
                         :unexpectedListType)),
         fail
       )
  ),
  !.

% normal predicate
selectivityQuerySelection(Pred, Rel, QueryTime, noBBox, ResCard) :-
  Pred =.. [OP|_],
  ( optimizerOption(determinePredSig)
    -> ( getTypeTree(Pred,[(1,Rel)],[OP,ArgsTrees,bool]),
         findall(T,member([_,_,T],ArgsTrees),ArgsTypeList),
         not(isBBoxOperator(OP,ArgsTypeList,_))
       )
    ;  not(isBBoxPredicate(OP))
  ),
  ( optimizerOption(dynamicSample)
    -> dynamicPossiblyRenameS(Rel, RelQuery)
    ;  ( Rel = rel(DCrelName, _),
         ensureSampleSexists(DCrelName),
         sampleS(Rel, RelS),
         possiblyRename(RelS, RelQuery)
       )
  ),
  dm(selectivity,['--------------  unary standard predicate --------------\n']),
  Query = count(filter(RelQuery, Pred)),
  transformQuery(Rel, Pred, Query, Query2),
  plan_to_atom(Query2, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  dm(selectivity,['\nSelectivity query : ', QueryAtom, '\n']),
  getTime(
    ( secondo(QueryAtom, ResultList)
      -> (true)
      ;  ( term_to_atom(Pred,PredA),
           concat_atom(['Selectivity query failed. Please check ',
                       'whether predicate \'', PredA, '\' is a boolean ',
                       'function! '],'',ErrMsg),
           write_list(['\nERROR:\t',ErrMsg]), nl,
           throw(error_SQL(statistics_selectivityQuerySelection(Pred, Rel,
                QueryTime, noBBox, ResCard):selectivityQueryFailed#ErrMsg)),
           fail
         )
    )
    ,QueryTime
  ),
  clearSelectivityQuery(Rel, Pred),
  ( ResultList = [int, ResCard]
    -> true
    ;  ( write_list(['\nERROR:\tUnexpected result list format during ',
                     'selectivity query:\n',
                     'Expected \'[int, <intvalue>]\' but got \'',
                     ResultList, '\'.']), nl,
         throw(error_Internal(statistics_selectivityQuerySelection(
                         Pred, Rel, QueryTime, noBBox, ResCard)
                         :unexpectedListType)),
         fail
       )
  ),
  dm(selectivity,['Elapsed Time: ', QueryTime, ' ms\n']), !.

selectivityQuerySelection(Pred, Rel, QueryTime, BBox, ResCard) :-
  write_list(['\nERROR:\tSelectivity query failed for unknown reason.']), nl,
  throw(error_Internal(statistics_selectivityQuerySelection(Pred, Rel,QueryTime,
        BBox, ResCard):selectivityQueryFailed)),  fail.

% spatial predicate with bbox-checking
selectivityQueryJoin(Pred, Rel1, Rel2, QueryTime, BBoxResCard, FilterResCard) :-
  Pred =.. [OP, Arg01, Arg02],
  ( optimizerOption(determinePredSig)
    -> ( getTypeTree(Pred,[(1,Rel1),(2,Rel2)],[OP,ArgsTrees,bool]),
         findall(T,member([ _, _, T],ArgsTrees),ArgsTypeList),
         isBBoxOperator(OP,ArgsTypeList,Dim),
         ArgsTrees = [Arg1Tree,Arg2Tree],
         bboxSizeQueryJoin(Arg01,Rel1,Rel2,Arg1Tree,_),
         bboxSizeQueryJoin(Arg02,Rel1,Rel2,Arg2Tree,_)
       )
    ;  ( isBBoxPredicate(OP),
         Dim = std
       )
  ),
  streamName(txx1),
  assert(selectivityQuery(Pred)),
  transformPred(Pred, txx1, 1, Pred2),
  dm(selectivity,['===== spatial binary predicate with bbox-checking =====\n']),
  Pred2 =.. [_, Arg1, Arg2],
  getBBoxIntersectionTerm(Arg1,Arg2,Dim,Pred3),
  ( optimizerOption(dynamicSample)
    -> ( dynamicPossiblyRenameJ(Rel1, Rel1Query),
         dynamicPossiblyRenameJ(Rel2, Rel2Query),
         Query = count(filter(counter(loopjoin(Rel1Query,
           fun([param(txx1, tuple)], filter(Rel2Query, Pred3))),1),Pred2) )
       )
    ;  ( Rel1 = rel(DCrelName1, _),
         Rel2 = rel(DCrelName2, _),
         ensureSampleSexists(DCrelName1),
         ensureSampleJexists(DCrelName2),
         sampleS(Rel1, Rel1S),
         sampleJ(Rel2, Rel2S),
         possiblyRename(Rel1S, Rel1Query),
         possiblyRename(Rel2S, Rel2Query),
         Rel2S = rel(BaseName, _),
         card(BaseName, JoinSize),
         Query = count(filter(counter(loopjoin(Rel1Query,
                       fun([param(txx1, tuple)],
                       filter(Rel2Query, Pred3))),1), Pred2) )
       )
  ),
  transformQuery(Rel1, Rel2, Pred, Query, JoinSize, Query2),
  plan_to_atom(Query2, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  dm(selectivity,['\nSelectivity query : ', QueryAtom, '\n']),
  getTime(
    ( secondo(QueryAtom, ResultList)
      -> (true)
      ;  ( write_list(['\nERROR:\tSelectivity query failed. Please check ',
                       'whether predicate \'', Pred, '\' is a boolean ',
                       'function! ']), nl,
           throw(error_Internal(statistics_selectivityQueryJoin(
                         Pred, Rel1, Rel2, QueryTime, BBoxResCard,FilterResCard)
                         :selectivityQueryFailed)),
           fail
         )
    )
    ,QueryTime
  ),
  clearSelectivityQuery(Rel1, Rel2, Pred),
  clearStreamName,
  ( ResultList = [int, FilterResCard]
    -> true
    ;  ( write_list(['\nERROR:\tUnexpected result list format during ',
                     'selectivity query:\n',
                     'Expected \'[int, <intvalue>]\' but got \'',
                     ResultList, '\'.']), nl,
         throw(error_Internal(statistics_selectivityQueryJoin(
                         Pred, Rel1, Rel2, QueryTime, BBoxResCard,FilterResCard)
                         :unexpectedListType)),
         fail
       )
  ),
  dm(selectivity,['Elapsed Time: ', QueryTime, ' ms\n']),
  secondo('list counters',  ResultList2),
  ( ResultList2 = [[1, BBoxResCard]|_]
    -> true
    ;  ( write_list(['\nERROR:\tUnexpected result list format during ',
                     'list counters query:\n',
                     'Expected \'[[1, BBoxResCard]|_]\' but got \'',
                     ResultList2, '\'.']), nl,
         throw(error_Internal(statistics_selectivityQueryJoin(
                         Pred, Rel1, Rel2, QueryTime, BBoxResCard,FilterResCard)
                         :unexpectedListType)),
         fail
       )
  ),
  !.

% normal predicate
selectivityQueryJoin(Pred, Rel1, Rel2, QueryTime, noBBox, ResCard) :-
  Pred =.. [OP|_],
  ( optimizerOption(determinePredSig)
    -> ( getTypeTree(Pred,[(1,Rel1),(2,Rel2)],[OP,ArgsTrees,bool]),
         findall(T,member([_,_,T],ArgsTrees),ArgsTypeList),
         not(isBBoxOperator(OP,ArgsTypeList,_))
       )
    ;  not(isBBoxPredicate(OP))
  ),
  streamName(txx1),
  assert(selectivityQuery(Pred)),
  transformPred(Pred, txx1, 1, Pred2),
  dm(selectivity,['-------------- binary standard predicate --------------\n']),
  ( optimizerOption(dynamicSample)
    -> ( dynamicPossiblyRenameJ(Rel1, Rel1Query),
         dynamicPossiblyRenameJ(Rel2, Rel2Query),
         Query = count(loopsel(Rel1Query, fun([param(txx1, tuple)],
                       filter(Rel2Query, Pred2))))
       )
    ;  ( Rel1 = rel(DCrelName1, _),
         Rel2 = rel(DCrelName2, _),
         ensureSampleSexists(DCrelName1),
         ensureSampleJexists(DCrelName1),
         ensureSampleJexists(DCrelName2),
         sampleS(Rel1, Rel1S),
         sampleJ(Rel1, Rel1J),
         sampleJ(Rel2, Rel2S),
         Rel1J = rel(BaseName, _),
         possiblyRename(Rel1S, Rel1Query),
         possiblyRename(Rel2S, Rel2Query),
         card(BaseName, JoinSize),
         Query = count(loopsel(Rel1Query,
           fun([param(txx1, tuple)], filter(Rel2Query, Pred2))))
       )
   ),  
  transformQuery(Rel1, Rel2, Pred, Query, JoinSize, Query2),
  plan_to_atom(Query2, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  dm(selectivity,['\nSelectivity query : ', QueryAtom, '\n']),
  getTime(
    ( secondo(QueryAtom, ResultList)
      -> (true)
      ;  ( write_list(['\nERROR:\tSelectivity query failed. Please check ',
                       'whether predicate \'', Pred, '\' is a boolean ',
                       'function! ']), nl,
           throw(error_Internal(statistics_selectivityQueryJoin(Pred, Rel1,Rel2,
                         QueryTime, noBBox, ResCard):selectivityQueryFailed)),
           fail
         )
    )
    ,QueryTime
  ),
  clearSelectivityQuery(Rel1, Rel2, Pred),
  clearStreamName,
  ( ResultList = [int, ResCard]
    -> true
    ;  ( write_list(['\nERROR:\tUnexpected result list format during ',
                     'selectivity query:\n',
                     'Expected \'[int, <intvalue>]\' but got \'',
                     ResultList, '\'.']), nl,
         throw(error_Internal(statistics_selectivityQueryJoin(
                         Pred, Rel1, Rel2, QueryTime, noBBox, ResCard)
                         :unexpectedListType)),
         fail
       )
  ),
  dm(selectivity,['Elapsed Time: ', QueryTime, ' ms\n']), !.

selectivityQueryJoin(Pred, Rel1, Rel2, QueryTime, BBox, ResCard) :-
  term_to_atom(Pred,PredT),
  concat_atom(['Selectivity query failed for: \'',PredT,
               '\'. Unknown reason.'],'',ErrMsg),
  write_list(['\nERROR:\t',ErrMsg]), nl,
  throw(error_Internal(statistics_selectivityQueryJoin(Pred, Rel1, Rel2,
        QueryTime, BBox, ResCard):selectivityQueryFailed)), fail.

/*

----	transformPred(Pred, Param, Arg, Pred2) :-
----

~Pred2~ is ~Pred~ transformed such that the attribute X of relation ~ArgNo~ is written
as ``attribute(Param, attrname(X))''

*/

% handle subqueries
transformPred(Pred, Param, Arg, Pred2) :-
  optimizerOption(subqueries),
  subqueryTransformPred(Pred, Param, Arg, Pred2).

transformPred(attr(Attr, Arg, Case), Param, Arg,
  attribute(Param, attrname(attr(Attr, Arg, Case)))) :- !.

transformPred(attr(Attr, Arg, Case), _, _, attr(Attr, Arg, Case)) :- !.

transformPred([], _, _, []).
transformPred([Arg1|Args1], Param, Arg, [Arg1T|Args1T]) :-
  transformPred(Arg1, Param, Arg, Arg1T),
  transformPred(Args1, Param, Arg, Args1T).

transformPred(Pred, Param, Arg, Pred2) :-
  compound(Pred),
  Pred =.. [T|Args],
  transformPred(Args, Param, Arg, Args2),
  Pred2 =.. [T|Args2].

transformPred(Pred, _, _, Pred).


% Selectivities must not be 0

nonzero(0, 1) :- !.

nonzero(N, N).

/*
---- getTime(:Goal, -Time)
----

Measures the time used to execute ~Goal~ in milliseconds (ms).

*/

getTime(Goal, TimeMS) :-
  get_time(Time1),
  call(Goal),
  get_time(Time2),
  Time3 is Time2 - Time1,
  convert_time(Time3, _, _, _, _, Minute, Sec, MilliSec),
  TimeMS is Minute *60000 + Sec*1000 + MilliSec, !.
  
/*

----
selectivity(+P, -Sel)
selectivity(+P, -Sel, -CalcPET, -ExpPET)
getPET(+P, -CalcPET, -ExpPET)
----

The selectivity of predicate ~P~ is ~Sel~. The analytic predicate cost function reports the evaluation of the predicate to take ~CalcPET~ milliseconds of time. During the actual query, the evaluation took ~ExpPET~ milliseconds of time for a single evaluation.

If ~selectivity~ is called, it first tries to look up
the selectivity via the predicate ~sels~. If no selectivity
is found, a Secondo query is issued, which determines the
selectivity. The retrieved selectitivity is then stored in
predicate ~storedSel~. This ensures that a selectivity has to
be retrieved only once.

Additionally, the time to evaluate a predicate is estimated by
dividing the query time by the number of predicate evaluations.
The result is stored in a table ~storedPET(DB, Pred, CalcPET, ExpPET)~, where
~PET~ means ~Predicate Evaluation Time~.

*/

sels(Pred, Sel, CalcPET, ExpPET) :-
  databaseName(DB),
  storedSel(DB, Pred, Sel),
  storedPET(DB, Pred, CalcPET, ExpPET), !.

sels(Pred, Sel, CalcPET, ExpPET) :-
  commute(Pred, Pred2),
  databaseName(DB),
  storedSel(DB, Pred2, Sel),
  storedPET(DB, Pred2, CalcPET, ExpPET), !.


% Wrapper selectivity/2 for standard optimizer
selectivity(Pred, Sel) :-
  selectivity(Pred, Sel, _, _).

selectivity(P, X) :-
  write('Error in optimizer: cannot find selectivity for '),
  simplePred(P, PSimple), write(PSimple), nl,
  write('Call: selectivity('), write(P), write(',Sel)\n'),
  throw(error_Internal(statistics_selectivity(P, X):selectivityQueryFailed)),
  fail, !.



% Wrapper selectivity/5 to get also bbox selectivity
selectivity(Pred, Sel, BBoxSel, CalcPET, ExpPET) :-
  selectivity(Pred, Sel, CalcPET, ExpPET),
  simplePred(Pred, PSimple),
  databaseName(DB),
  (   storedBBoxSel(DB, PSimple, BBoxSel)
    ; (commute(PSimple,PSimple2), storedBBoxSel(DB, PSimple2, BBoxSel) )
  ),
  !.

selectivity(Pred, Sel, noBBox, CalcPET, ExpPET) :-
  selectivity(Pred, Sel, CalcPET, ExpPET).

% handle 'pseudo-joins' (2 times the same argument) as selections
selectivity(pr(Pred, Rel, Rel), Sel, CalcPET, ExpPET) :-
  selectivity(pr(Pred, Rel), Sel, CalcPET, ExpPET), !.

% check if selectivity has already been stored
selectivity(P, Sel, CalcPET, ExpPET) :-
  simplePred(P, PSimple),
  sels(PSimple, Sel, CalcPET, ExpPET), !.

% query for join-selectivity (static samples case)
selectivity(pr(Pred, Rel1, Rel2), Sel, CalcPET, ExpPET) :-
  not(optimizerOption(dynamicSample)),
  Rel1 = rel(BaseName1, _),
  sampleNameJ(BaseName1, SampleName1),
  ensureSampleJexists(BaseName1),
  card(SampleName1, SampleCard1),
  Rel2 = rel(BaseName2, _),
  sampleNameJ(BaseName2, SampleName2),
  ensureSampleJexists(BaseName2),
  card(SampleName2, SampleCard2),
  selectivityQueryJoin(Pred, Rel1, Rel2, MSs, BBoxResCard, ResCard),
  nonzero(ResCard, NonzeroResCard),
  Sel is NonzeroResCard / (SampleCard1 * SampleCard2), % must not be 0
  tupleSizeSplit(BaseName1,TupleSize1),
  tupleSizeSplit(BaseName2,TupleSize2),
  calcExpPET(MSs, SampleCard1, TupleSize1,
                  SampleCard2, TupleSize2, NonzeroResCard, MSsRes),
                                         % correct PET
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  predCost(PSimple, CalcPET), % calculated PET
  ExpPET is MSsRes / max(SampleCard1 * SampleCard2, 1),
  dm(selectivity,['Predicate Cost  : (', CalcPET, '/', ExpPET,
                  ') ms\nSelectivity     : ', Sel,'\n']),
  databaseName(DB),
  assert(storedPET(DB, PSimple, CalcPET, ExpPET)),
  assert(storedSel(DB, PSimple, Sel)),
  ( ( BBoxResCard = noBBox ; storedBBoxSel(DB, PSimple, _) )
    -> true
    ;  ( nonzero(BBoxResCard, NonzeroBBoxResCard),
         BBoxSel is NonzeroBBoxResCard / (SampleCard1 * SampleCard2),
         dm(selectivity,['BBox-Selectivity: ',BBoxSel,'\n']),
         assert(storedBBoxSel(DB, PSimple, BBoxSel))
       )
  ),
  ( optimizerOption(determinePredSig)
    -> (
          getTypeTree(Pred, [(1,Rel1),(2,Rel2)], [Op,Args,ResultType]),
          findall(T,(member([_,_,T],Args)),ArgTypeList),
          Signature =.. [Op|ArgTypeList],
          assert(storedPredicateSignature(DB, PSimple, [Op,Args,ResultType])),
          dm(selectivity,['The topmost signature for ',Pred,' is:\t',
                          Signature,' -> ',ResultType,'\n'])
       )
    ; true
  ),
  !.

% query for selection-selectivity (static samples case)
selectivity(pr(Pred, Rel), Sel, CalcPET, ExpPET) :-
  not(optimizerOption(dynamicSample)),
  Rel = rel(BaseName, _),
  sampleNameS(BaseName, SampleName),
  ensureSampleSexists(BaseName),
  card(SampleName, SampleCard),
  selectivityQuerySelection(Pred, Rel, MSs, BBoxResCard, ResCard),
  nonzero(ResCard, NonzeroResCard),
  Sel is NonzeroResCard / SampleCard,   % must not be 0
  tupleSizeSplit(BaseName,TupleSize),
  calcExpPET(MSs, SampleCard, TupleSize, MSsRes), % correct PET
  simplePred(pr(Pred, Rel), PSimple),
  predCost(PSimple,CalcPET), % calculated PET
  ExpPET is MSsRes / max(SampleCard,1),
  dm(selectivity,['Predicate Cost  : (', CalcPET, '/', ExpPET,
                  ') ms\nSelectivity     : ', Sel,'\n']),
  databaseName(DB),
  assert(storedPET(DB, PSimple, CalcPET, ExpPET)),
  assert(storedSel(DB, PSimple, Sel)),
  ( ( BBoxResCard = noBBox ; storedBBoxSel(DB, PSimple, _) )
    -> true
    ;  ( nonzero(BBoxResCard, NonzeroBBoxResCard),
         BBoxSel is NonzeroBBoxResCard / SampleCard,
         dm(selectivity,['BBox-Selectivity: ',BBoxSel,'\n']),
         assert(storedBBoxSel(DB, PSimple, BBoxSel))
       )
  ),
  ( optimizerOption(determinePredSig)
      -> (
           getTypeTree(Pred, [(1,Rel)], [Op,Args,ResultType]),
           findall(T,(member([_,_,T],Args)),ArgTypeList),
           Signature =.. [Op|ArgTypeList],
           assert(storedPredicateSignature(DB, PSimple, [Op,Args,ResultType])),
           dm(selectivity,['The topmost signature for ',Pred,' is:\t',
                           Signature,' -> ',ResultType,'\n'])
         )
      ; true
  ),
  !.

% query for join-selectivity (dynamic sampling case)
selectivity(pr(Pred, Rel1, Rel2), Sel, CalcPET, ExpPET) :-
  optimizerOption(dynamicSample),
  Rel1 = rel(BaseName1, _),
  card(BaseName1, Card1),
  secOptConstant(sampleJoinMaxCard, JoinSize),
  secOptConstant(sampleScalingFactor, SF),
  SampleCard1 is max(1,min(Card1, max(JoinSize, Card1 * SF))),
  Rel2 = rel(BaseName2, _),
  card(BaseName2, Card2),
  SampleCard2 is max(1,min(Card2, max(JoinSize, Card2 * SF))),
  selectivityQueryJoin(Pred, Rel1, Rel2, MSs, BBoxResCard, ResCard),
  nonzero(ResCard, NonzeroResCard),
  Sel is NonzeroResCard / (SampleCard1 * SampleCard2),  % must not be 0
  tupleSizeSplit(BaseName1,TupleSize1),
  tupleSizeSplit(BaseName2,TupleSize2),
  calcExpPET(MSs, SampleCard1, TupleSize1,
                  SampleCard2, TupleSize2, NonzeroResCard, MSsRes),
                                     % correct PET
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  predCost(PSimple,CalcPET), % calculated PET
  ExpPET is MSsRes / max(SampleCard1 * SampleCard2,1),
  dm(selectivity,['Predicate Cost  : (', CalcPET, '/', ExpPET,
                  ') ms\nSelectivity     : ', Sel,'\n']),

  databaseName(DB),
  assert(storedPET(DB, PSimple, CalcPET, ExpPET)),
  assert(storedSel(DB, PSimple, Sel)),
  ( ( BBoxResCard = noBBox ; storedBBoxSel(DB, PSimple, _) )
    -> true
    ;  ( nonzero(BBoxResCard, NonzeroBBoxResCard),
         BBoxSel is NonzeroBBoxResCard / (SampleCard1 * SampleCard2),
         dm(selectivity,['BBox-Selectivity: ',BBoxSel,'\n']),
         assert(storedBBoxSel(DB, PSimple, BBoxSel))
       )
  ),
  ( optimizerOption(determinePredSig)
      -> (
           getTypeTree(Pred, [(1,Rel1),(2,Rel2)], [Op,Args,ResultType]),
           findall(T,(member([_,_,T],Args)),ArgTypeList),
           Signature =.. [Op|ArgTypeList],
           assert(storedPredicateSignature(DB, PSimple, [Op,Args,ResultType])),
           dm(selectivity,['The topmost signature for ',Pred,' is:\t',
                           Signature,' -> ',ResultType,'\n'])

         )
      ; true
  ),
  !.

% query for selection-selectivity (dynamic sampling case)
selectivity(pr(Pred, Rel), Sel, CalcPET, ExpPET) :-
  optimizerOption(dynamicSample),
  Rel = rel(BaseName, _),
  card(BaseName, Card),
  secOptConstant(sampleSelMaxCard, SelectionSize),
  SampleCard is max(1,min(Card, max(SelectionSize, Card * 0.00001))),
  selectivityQuerySelection(Pred, Rel, MSs, BBoxResCard, ResCard),
  nonzero(ResCard, NonzeroResCard),
  Sel is NonzeroResCard / SampleCard,		% must not be 0
  tupleSizeSplit(BaseName,TupleSize),
  calcExpPET(MSs, SampleCard, TupleSize, MSsRes), % correct PET
  simplePred(pr(Pred, Rel), PSimple),
  predCost(PSimple,CalcPET), % calculated PET
  ExpPET is MSsRes / max(SampleCard,1),
  dm(selectivity,['Predicate Cost  : (', CalcPET, '/', ExpPET,
                  ') ms\nSelectivity     : ', Sel,'\n']),
  databaseName(DB),
  assert(storedPET(DB, PSimple, CalcPET, ExpPET)),
  assert(storedSel(DB, PSimple, Sel)),
  ( ( BBoxResCard = noBBox ; storedBBoxSel(DB, PSimple, _) )
    -> true
    ;  ( nonzero(BBoxResCard, NonzeroBBoxResCard),
         BBoxSel is NonzeroBBoxResCard / SampleCard,
         dm(selectivity,['BBox-Selectivity: ',BBoxSel,'\n']),
         assert(storedBBoxSel(DB, PSimple, BBoxSel))
       )
  ),
  ( optimizerOption(determinePredSig)
      -> (
          getTypeTree(Pred, [(1,Rel)], [Op,Args,ResultType]),
          findall(T,(member([_,_,T],Args)),ArgTypeList),
          Signature =.. [Op|ArgTypeList],
          assert(storedPredicateSignature(DB, PSimple, [Op,Args,ResultType])),
          dm(selectivity,['The topmost signature for ',Pred,' is:\t',
                          Signature,' -> ',ResultType,'\n'])
         )
      ; true
  ),
  !.

% handle ERRORs
selectivity(P, X, Y, Z) :-
  simplePred(P, PSimple),
  term_to_atom(PSimple,PSimpleT),
  concat_atom(['Cannot find selectivity for \'', PSimpleT, '\'.'],'',ErrMsg),
  write('Error in optimizer: '), write(ErrMsg),
  write('Call: selectivity('), write(P), write(', _, _, _)\n'),
  throw(error_Internal(statistics_selectivity(P, X, Y, Z)
                                   :selectivityQueryFailed#ErrMsg)),
  fail, !.


% access stored PETs by simplified predicate term
getPET(P, CalcPET, ExpPET) :-
  databaseName(DB),
  simplePred(P,PSimple),
  (   storedPET(DB, PSimple, CalcPET, ExpPET)
    ; ( commute(PSimple, PSimple2), storedPET(DB, PSimple2, CalcPET, ExpPET) )
  ), !.

getPET(P, X, Y) :-
  simplePred(P, PSimple),
  term_to_atom(PSimple,PSimpleT),
  concat_atom(['Cannot find PETs for \'', PSimpleT, '\'.'],'',ErrMsg),
  write('Error in optimizer: '), write(ErrMsg), nl,
  write('Call: getPET('), write(P), write(', _, _)\n'),
  throw(error_Internal(statistics_getPET(P, X, Y):missingData#ErrMsg)),
  fail, !.

/*

The selectivities retrieved via Secondo queries can be loaded
(by calling ~readStoredSels~) and stored (by calling
~writeStoredSels~).

*/

readStoredSels :-
  retractall(storedSel(_, _, _)),
  retractall(storedBBoxSel(_, _, _)),
  retractall(storedPredicateSignature(_, _, _)),
  retractall(storedBBoxSize(_,_,_)),
  [storedSels].

/*

The following functions are auxiliary functions for ~writeStoredSels~. Their
purpose is to convert a list of character codes (e.g. [100, 99, 100]) to
an atom (e.g. "dcd"), which makes the stored selectitivities more
readable.

*/

isIntList([]).

isIntList([X | Rest]) :-
  integer(X),
  isIntList(Rest).

charListToAtom(CharList, Atom) :-
  atom_codes(A, CharList),
  concat_atom([' "', A, '"'], Atom).

replaceCharList(InTerm, OutTerm) :-
  isIntList(InTerm),
  !,
  charListToAtom(InTerm, OutTerm).

replaceCharList(InTerm, OutTerm) :-
  compound(InTerm),
  !,
  InTerm =.. TermAsList,
  maplist(replaceCharList, TermAsList, OutTermAsList),
  OutTerm =.. OutTermAsList.

replaceCharList(X, X).

writeStoredSels :-
  open('storedSels.pl', write, FD),
  write(FD,
    '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredSel(FD), _),
  close(FD).

writeStoredSel(Stream) :-
  storedSel(DB, X, Y),
  replaceCharList(X, XReplaced),
  write(Stream, storedSel(DB, XReplaced, Y)), write(Stream, '.\n').

writeStoredSel(Stream) :-
  storedBBoxSel(DB, X, Y),
  replaceCharList(X, XReplaced),
  write(Stream, storedBBoxSel(DB, XReplaced, Y)), write(Stream, '.\n').

writeStoredSel(Stream) :-
  storedPredicateSignature(DB, X, [Y1,Y2,Y3]),
  replaceCharList(X, XReplaced),
  write(Stream, storedPredicateSignature(DB, XReplaced, [Y1,Y2,Y3])),
  write(Stream, '.\n').

writeStoredSel(Stream) :-
  storedBBoxSize(DB,X,Y),
  replaceCharList(X, XReplaced),
  write(Stream, storedBBoxSize(DB, XReplaced, Y)),
  write(Stream, '.\n').

showSel :-
  storedSel(DB, X, Y),
  replaceCharList(X, XRepl),
  format('  ~p~16|~p.~p~n',[Y,DB,XRepl]).
%  write(Y), write('\t\t'), write(DB), write('.'), write(X), nl.

showBBoxSel :-
  storedBBoxSel(DB, X, Y),
  replaceCharList(X, XRepl),
  format('  ~p~16|~p.~p~n',[Y,DB,XRepl]).
%  write(Y), write('\t\t'), write(DB), write('.'), write(X), nl.

showBBoxSize :-
  storedBBoxSize(DB, X, Y),
  replaceCharList(X, XRepl),
  format('  ~p~16|~p.~p~n',[Y,DB,XRepl]).
%  write(Y), write('\t\t'), write(DB), write('.'), write(X), nl.

:-assert(helpLine(showSels,0,[],'Display known selectivities.')).
showSels :-
  write('\nStored selectivities:\n'),
  findall(_, showSel, _),
  write('\nStored bbox-selectivities:\n'),
  findall(_, showBBoxSel, _),
  write('\nStored average bbox sizes:\n'),
  findall(_, showBBoxSize, _).

:-
  dynamic(storedSel/3),
  dynamic(storedBBoxSel/3),
  dynamic(storedPredicateSignature/3),
  dynamic(storedBBoxSize/3),
  at_halt(writeStoredSels),
  readStoredSels.

readStoredPETs :-
  retractall(storedPET(_, _, _, _)),
  [storedPETs].

writeStoredPETs :-
  open('storedPETs.pl', write, FD),
  write(FD,
    '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredPET(FD), _),
  close(FD).

writeStoredPET(Stream) :-
  storedPET(DB, X, Y, Z),

  replaceCharList(X, XReplaced),
  write(Stream, storedPET(DB, XReplaced, Y, Z)), write(Stream, '.\n').

:-assert(helpLine(showPETs,0,[],'Display known predicate evaluation times.')).
showPETs :-
  write('\nStored predicate costs:\n'),
  format('  ~p~18|~p~34|~p~n',['Calculated [ms]', 'Measured [ms]','Predicate']),
  findall(_, showPET, _).

showPET :-
  storedPET(DB, P, Calc, Exp),
  replaceCharList(P, PRepl),
  format('  ~p~18|~p~34|~p.~p~n',[Calc, Exp, DB, PRepl]).

:-
  dynamic(storedPET/4),
  at_halt(writeStoredPETs),
  readStoredPETs.

writePETs :-
  findall(_, writePET, _).

writePET :-
  storedPET(DB, X, Y, Z),
  replaceCharList(X, XReplaced),
  write('DB: '), write(DB),
  write(', Predicate: '), write(XReplaced),
  write(', Cost: '), write(Y), write('/'), write(Z), write(' ms\n').


readStoredPredicateSignatures :-
  retractall(storedPredicateSignature(_, _, _)),
  [storedPredicateSignatures].

writeStoredPredicateSignatures :-
  open('storedPredicateSignatures.pl', write, FD),
  write(FD,
    '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredPredicateSignature(FD), _),
  close(FD).

writeStoredPredicateSignature(Stream) :-
  storedPredicateSignature(DB, X, [Y1,Y2,Y3]),
  replaceCharList(X, XReplaced),
  write(Stream, storedPredicateSignature(DB, XReplaced, [Y1,Y2,Y3])),
  write(Stream, '.\n').

:-assert(helpLine(showStoredPredicateSignatures,0,[],
                  'Display known predicate signatures.')).
showStoredPredicateSignatures :-
  write('\nStored predicate signatures:\n'),
  format('  ~p~12|~p~36|~p~n',['Database','Predicate','Signature']),
  findall(_, showStoredPredicateSignature, _).

showStoredPredicateSignature :-
  storedPredicateSignature(DB, P, S),
  replaceCharList(P, PRepl),
  format('  ~p~12|~p~36|~p~n',[DB, PRepl, S]).


/*
1.5 Determining System Speed and Calibration Factors

To achieve good cost estimations, the used cost factors for operators need to be calibrated.

*/

/*
The cost factors for the cost functions are read from a file:

*/

readStoredOperatorTF :-
  retractall(tempOperatorTF(_)),
  retractall(storedOperatorTF(_)),
  [sysDependentOperatorTF].


/*
The cost factors for the cost functions are stored into a file;

*/

writeStoredOperatorTF :-
  open('sysDependentOperatorTF.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredOperatorTF(FD), _),
  close(FD).

writeStoredOperatorTF(Stream) :-
  storedOperatorTF(Op, A, B),
  % replaceCharList(X, XReplaced),
  write(Stream, storedOperatorTF(Op, A, B)),
  write(Stream, '.\n').

/*
The Original cost factors for operators are:

*/
setOriginalOperatorTFs :-
  assert(storedOperatorTF(consume, 0.00000147, 0.0003)),
  assert(storedOperatorTF(loopjoin, 0.000000095, 0.000014)),
  assert(storedOperatorTF(filter, 0, 0)),
  assert(storedOperatorTF(string, notype, 0.00000053)),
  assert(storedOperatorTF(int, notype, 0.00000051)),
  assert(storedOperatorTF(real, notype, 0.00000052)),
  assert(storedOperatorTF(feed, 0.000000735, 0)),
  assert(storedOperatorTF(producta, 0.00000781, 0.00453249)),
  assert(storedOperatorTF(productb, 0.000049807, 0.008334)),
  assert(storedOperatorTF(productc, 0.434782, 0)),
  assert(storedOperatorTF(hashtemp, 0.000000789, 0.0002168)),
  assert(storedOperatorTF(hashcachecal, 0.1, 61.2)),
  assert(storedOperatorTF(hashtab, 0.000000766, 0)),
  assert(storedOperatorTF(extend, 0.000015, 0)),
  assert(storedOperatorTF(intsort, 0.000000083, 0.00001086)),
  assert(storedOperatorTF(extsort, 0.000000218, 0.000125193)),
  assert(storedOperatorTF(concatenationjoin, 0.000000095, 0.000014)),
  assert(storedOperatorTF(concatenationflob, 0.00000171, 0)),
  assert(storedOperatorTF(spatialjoinx, 0.000000216, 0.00000000004)),
  assert(storedOperatorTF(spatialjoiny, 0.00000024,  0.000000000045)),
  assert(storedOperatorTF(remove, 0.000015, 0)),
  assert(storedOperatorTF(rename, 0.00001, 0)),
  assert(storedOperatorTF(project, 0, 0)),
  assert(storedOperatorTF(count, 0, 0)),
  assert(storedOperatorTF(orderby, 0, 0)),
  assert(storedOperatorTF(groupby, 0, 0)),
  assert(storedOperatorTF(exactmatchfun, 0.000002265, 0.000298)),
  assert(storedOperatorTF(exactmatch, 0.000002265, 0.000298)),
  assert(storedOperatorTF(leftrange, 0.000002265, 0.000298)),
  assert(storedOperatorTF(windowintersects, 0.000000972, 0)),
  assert(storedOperatorTF(insidelr, 0.016712944, 0)).

:- dynamic(storedOperatorTF/3),
  at_halt(writeStoredOperatorTF),
  readStoredOperatorTF.

:- ( not(storedOperatorTF(_,_,_))
     -> setOriginalOperatorTFs
     ;  true
   ).


/*
Estimate the speed the stytem by posing a specific query, calculating the used time and comparing it with the value of the appropriate cost function. From this, calculate a new calibration factor.

*/

tfCPU(TestRel) :-
  concat_atom(['query ', TestRel, ' feed count'],'',Query),
  getTime(secondo(Query, [_, _]),Time),
  downcase_atom(TestRel, DCTestRel),
         write('\n>>>>>>>stat_012\n'), nl,
  card(DCTestRel, Card),
  tupleSizeSplit(DCTestRel, sizeTerm(Core, InFlob, _)),
  TLExt is Core + InFlob,
  testRelVolume(Card, TLExt),
  storedOperatorTF(feed, A, B),
  Cost is (TLExt * A + B) * Card,
  CFnew is Time / (Cost * 1000),
  vCostFactorCPU(V),
  nl, write(' The current CPU calibration factor: '), write(V),
  nl, write(' The suitable factor: '), write(CFnew),
  setCostFactor(tempCostFactor(vCostFactorCPU,CFnew)).

testRelVolume(Size, TupleSizeExt) :-
  Volume is Size * TupleSizeExt,
  Volume > 3500000,
  !.

testRelVolume(_, _) :-
  nl,
  write('Test relation should have a size of at least 3,5 MB (without Flobs).'),
  fail.


/*
Create a new clause for the temporary storage of the new calibration factor.

*/

setCostFactor(tempCostFactor(Op, CF)) :-
  retract(tempCostFactor(Op, _)),
  assert(tempCostFactor(Op, CF)), !.

setCostFactor(tempCostFactor(Op, CF)) :-
  assert(tempCostFactor(Op, CF)).

/*
Replaces the old calibration factor with a new one.

*/

updateCostFactorCPU :-
  tempCostFactor(vCostFactorCPU, CF),
  retract(vCostFactorCPU(_)),
  assert(vCostFactorCPU(CF)),
  write('vCostFactorCPU: '), write(CF),
  write(' update').

updateCostFactorCPU(CF) :-
  retract(vCostFactorCPU(_)),
  assert(vCostFactorCPU(CF)),
  write('vCostFactorCPU: '), write(CF),
  write(' update').

/*
The calibration factor and the conversion factor for the cost functions
are read from a file ``costFactors.pl:

*/

readStoredCostFactors :-
  retractall(wCostFactor(_)),
  retractall(vCostFactorCPU(_)),
  retractall(tempCostFactor(_,_)),
  [sysDependentCostFactors].

writeStoredCostFactors :-
  open('sysDependentCostFactors.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredCostFactors(FD), _),
  close(FD).

/*
The conversion factor for the cost functions is written back into the file.

*/

writeStoredCostFactors(Stream) :-
  wCostFactor(X),
  % replaceCharList(X, XReplaced),
  write(Stream, wCostFactor(X)),
  write(Stream, '.\n').

/*
The calibration factor of the cost functions is written back into the file.

*/

writeStoredCostFactors(Stream) :-
  vCostFactorCPU(X),
  % replaceCharList(X, XReplaced),
  write(Stream, vCostFactorCPU(X)),
  write(Stream, '.\n').

setOriginalCalibrationFactors :-
  assert(wCostFactor(1000000)),
  assert(vCostFactorCPU(1)).


:-
  dynamic(costFactors/1),
  at_halt(writeStoredCostFactors),
  readStoredCostFactors.

:- ( (not(wCostFactor(_)) ; not(vCostFactorCPU(_)))
     -> (   retractall(wCostFactor(_)),
            retractall(vCostFactorCPU(_)),
            retractall(tempCostFactor(_,_)),
            setOriginalCalibrationFactors
        )
     ;  true
   ).

/*
1.6 Estimating PETs using an Analytical Model

---- predCost(+Pred, -PredCost)
     predCost(+Predicate, -PredCost, -ArgTypeX, -ArgSize, +predArg(PA))
----

Calculation of the costs of a predicate. To get the total costs of a predicate,
the costs of its sub-terms are assessed first. The estimated costs are based
on the attribute sizes and the attribute types.

If the type of a term cannot be dertermined, a failure value (e.g. ~undefined~) should be returned. The clauses should be modified to handle the case where a recursive call yields an undefined result.

*/

predCost(Pred, PredCost) :-
  predCost(Pred, PredCost, _, _, predArg(1)).

predCost(Pred, PredCost, predArg(N)) :-
  predCost(Pred, PredCost, _, _, predArg(N)).

predCost(X = Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  equalTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
                                   % should somehow depend on ArgSize
  PredCost is PredCostX + PredCostY + OperatorTC.

predCost(X < Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  smallerTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + OperatorTC.

predCost(X > Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  biggerTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + OperatorTC.

predCost(X <= Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  equalsmallerTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + OperatorTC.

predCost(X >= Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  equalbiggerTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + OperatorTC.

predCost(X inside Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  insideTCnew(ArgTypeX, ArgTypeY, OperatorTC, S),
  sTTS(ArgSizeX,ArgSizeXT),
  sTTS(ArgSizeY,ArgSizeYT),
  PredCost is PredCostX + PredCostY
            + (ArgSizeXT ** S) * (ArgSizeYT * OperatorTC ) / 100000.

predCost(X adjacent Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  adjacentTCnew(ArgTypeX,  ArgTypeY, OperatorTC, S),
  sTTS(ArgSizeX,ArgSizeXT),
  sTTS(ArgSizeY,ArgSizeYT),
  PredCost is PredCostX + PredCostY
              + (ArgSizeXT * OperatorTC) * (ArgSizeYT ** S) / 100000.

predCost(X intersects Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  intersectsTCnew(ArgTypeX,  ArgTypeY, OperatorTC, S),
  sTTS(ArgSizeX,ArgSizeXT),
  sTTS(ArgSizeY,ArgSizeYT),
  PredCost is PredCostX + PredCostY
              + (ArgSizeXT * OperatorTC)*(ArgSizeYT ** S) / 100000.

predCost(X contains Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  containsTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + OperatorTC.

predCost(X + Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  plusTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + PA * OperatorTC.

predCost(X - Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX,  ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  minusTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + PA * OperatorTC.

predCost(X * Y, PredCost,  ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX,  ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY,  ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  timesTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + PA * OperatorTC.

predCost(X / Y, PredCost,  ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX,  ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY,  ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  overTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + PA * OperatorTC.

predCost(Rel:Attr, 0,  ArgType, ArgSize, _) :- !,
  downcase_atom(Attr, DCAttr),
  downcase_atom(Rel, DCRel),
  attrType(DCRel:DCAttr, ArgType),
  attrSize(DCRel:DCAttr, ArgSize), !.

% default value changed from 0 to 0.0000005
predCost(_, 0.0000005, notype, 1, _) :- !.

biggerArg(sizeTerm(X1,X2,X3), sizeTerm(Y1,Y2,Y3), sizeTerm(X1,X2,X3)) :-
  X1+X2+X3 >= Y1+Y2+Y3, !.

biggerArg(_, Arg2, Arg2).

/*
~sTTS~ meams sizeTerm to Total Size.

Maps a sizeTerm to a single value, equal to the sum of the term's components.

*/
sTTS(sizeTerm(X,Y,Z),T) :-
  T is X + Y + Z.

sTTS(T,T).


/*
Different operators can occur within a predicate. Cost factors for certain predicates and different combinations of argument types are stored in the following Prolog facts:

*/

plusTCnew(_, _, 0.0000005).
minusTCnew(_, _, 0.0000005).
smallerTCnew(_, _, 0.0000005).
biggerTCnew(_, _, 0.0000005).
timesTCnew(_, _, 0.0000005).
overTCnew(_, _, 0.0000005).
modTCnew(_, _, 0.0000005).
divTCnew(_, _, 0.0000005).
equalTCnew(_, _, 0.0000005).
equalbiggerTCnew(_, _, 0.0000005).
equalsmallerTCnew(_, _, 0.0000005).
insideTCnew(point, line, 0.05086, 0).
insideTCnew(point, region, 0.04457463, 0).
insideTCnew(line, region, 0.046230259, 0).
insideTCnew(region, region, 0.00004444, 1).
insideTCnew(_, _, 0.046230259, 0).
adjacentTCnew(line, region, 0.000004702, 1).
adjacentTCnew(region, line, 0.000004702, 1).
adjacentTCnew(region, region, 0.046612378, 0).
adjacentTCnew(line, line, 0.051482479, 0).
adjacentTCnew(_, _, 0.000040983, 0).
intersectsTCnew(line, line, 0.041509433, 0).
intersectsTCnew(line, region, 0.000046277, 0).
intersectsTCnew(region, line, 0.000046277, 0).
intersectsTCnew(region, region, 0.046612328, 0).
intersectsTCnew(_, _, 0.000046277, 0).
containsTCnew(_, _, 0.00002).



/*
1.7 Correcting Measured PETs

When ~optimizerOption(nawracosts))~ is defined, PETs will be corrected:

----
calcExpPET(+MSs, +Card, +TupleSize, -Time)
calcExpPET(+MSs, +Card1, +TupleSize1,
                 +Card2, +TupleSize2, +ResCard, -Time)
----
Calculation of experimental net-PETs (predicate evaluation times):

Reduce the measured time ~MSs~ when determinating the selectivity of selection-predicates
by the estimated costs of operator feed.

Reduce the measured time when determining the selectivity of join-predicates
by the estimated costs of the operators feed and loopjoin.

Other arguments: ~Card~, ~Card1~, ~Card2~ are the cardinality of the input streams; ~TupleSize~, ~TupleSize1~, ~TupleSize2~ are the tuple sizes of the input streams, ~ResCard~ is the cardinality of the join.

~Time~ is the sesulting net-PET in milliseconds.

*/


calcExpPET(MSs, Card, TupleSize, Time) :-
  optimizerOption(nawracosts),
  storedOperatorTF(feed, OpA, OpB),
  wCostFactor(W),
  TupleSize = sizeTerm(Core, InFlob, _),
  Cost is  (OpA * (Core + InFlob) + OpB) * Card,
  costtotal(Cost, CostTotal),
  TimeOp is MSs - (CostTotal / W) * 1000,
  notnegativ(TimeOp, Time), !.

calcExpPET(MSs, _, _, MSs) :-
  not(optimizerOption(nawracosts)), !.

calcExpPET(MSs, Card1, TupleSize1, Card2, TupleSize2, ResCard, Time) :-
  optimizerOption(nawracosts),
  storedOperatorTF(feed, OpA, OpB),
  storedOperatorTF(loopjoin, A, B),
  storedOperatorTF(concatenationflob, A2, _),
  TupleSize1 = sizeTerm(Core1, InFlob1, ExtFlob1),
  TupleSize2 = sizeTerm(Core2, InFlob2, ExtFlob2),
  FlobSize is ExtFlob1 + ExtFlob2,
  ExtSize1 is Core1 + InFlob1,
  ExtSize2 is Core2 + InFlob2,
  wCostFactor(W),
  Cost is  (OpA * ExtSize1 + OpB) * Card1 + (OpA * ExtSize2 + OpB) * Card2
          + (((ExtSize1 + ExtSize2) * A + B) + (FlobSize * A2)) * ResCard,
  costtotal(Cost, CostTotal),
  TimeOp is MSs - (CostTotal / W) * 1000,
  notnegativ(TimeOp, Time), !.

calcExpPET(MSs, _, _, _, _, _, MSs) :-
  not(optimizerOption(nawracosts)), !.


/*
1 Determining Expression Types and Operator Signatures

The following predicates are used to determine the Signature of operators
used within queries, especially within predicates.

The approach investigates a term bottom-up, i.e. it first tries to determine
the type of the arguments on the leaves of the operator tree by inspecting the
attribute table or by sending getTypeNL-Queries to Secondo.

Once all argument types are known for a operator node, we check whether the
signature is already known. If so, we already know the operator result type.
Otherwise, we need to query Secondo for it.


---- getTypeTree(+Expr,+RelList,-TypeTree)
----
Retrieves the complete type/operator tree ~TypeTree~ from an expression ~Expr~
for a given relation list ~RelList~.

~Expr~ is an expression in internal format using attribute and relations
descriptors etc.

~RelList~ has format [(Arg1,Rel1),...(ArgN,RelN)], where Arg1...ArgN are integers
corresponding to the ~Arg~ fields used in attribute descriptors within ~Expr~,
and Rel1,..., RelN are the according relation descriptors.

~TypeTree~ has format [~Op~, ~TypeTreeList~, ~ResType~], where
~Op~ is the operator symbol,
~TypeTreeList~ is a list of entries with format ~TypeTree~, and
~ResType~ is the type of the complete expression (root node).

Atomic leaves have special markers instead of an operator symbol:
integer, real, text and string atoms have ~atom~, attributes have ~attr~,
attribute names have ~attrname~, database object names have ~dbobject~,
type names have ~typename~, relations have ~relation~. In all these cases,
~TypeTreeList~ becomes the expression forming the according primitive.
Newly created (calculated) attributes are marked with ~newattr~.

Facts describing known operator signatures are defined in file ~operators.pl~.
The all have format

---- opSignature(+Operator, +Algebra, +ArgTypeList, -Resulttype, -Flags).
----

~Operator~ is the name of the operator (Prolog infix-operators are inclosed in
round parantheses)

~Algebra~ is the DC-nmae of the algebra defining the operator.

~ArgTypeList~ is a PROLOG list of terms representing the valid Secondo type
expressions for all arguments.

~Resulttype~ is a term representing a valid Secondo type expression.

~Flags~ is a list describing certain properties of the operators

If an operator/signature combination is unknown, the optimizer tries to create
and execute a query to determine the according result type. If a result is
achieved, it is temporarily stored to the optimizer's knowledge base using the
dynamic predicate

---- queriedOpSignature(Op,ArgTypes,TypeDC,Flags).
----


The type trees of all analysed expressions are kept within a temporal facts

---- tmpStoredExprTypeTree(+P,(-Op, -TypeTreeList, -ResType))
----

These facts are not made persistent and will be lost whenever a new query is
started.

----  getTypeTree(+PredExpr,-TypeTree)
----

This form of the predicate receives one argument ~PredExpr~, that represents
one of the internal predicate representations: pr(P,A) or pr(P,A,B).

It automatically extracts the predicate expression and creates the relation
argument list and calls ~getTypeTree/3~.

*/

:- dynamic(queriedOpSignature/4).% used for new operator signatures
:- dynamic(tmpStoredTypeTree/2). % used to buffer results of expression analysis

getTypeTree(PredExpr,Result) :-
  PredExpr =.. [pr,P|RelList],
  createRelArgListFromPredRelList(RelList,RelArgList),
  getTypeTree(P,RelArgList,Result), !.

% createRelArgListFromPredRelList(+RelList,-RelArgList)
% numbers all elemenets from ~RelList~, starting with 1.
createRelArgListFromPredRelList(X,XA) :-
  createRelArgListFromPredRelList2(X,1,XA),!.
createRelArgListFromPredRelList([],_,[]).
createRelArgListFromPredRelList([X|Xs],N1,[(N1,X)|X2]) :-
  N2 is N1 + 1,
  createRelArgListFromPredRelList(Xs,N2,X2).


% fail, if option not selected:
getTypeTree(_,_,_) :-
  not(optimizerOption(determinePredSig)),!,
  fail.

% getTypeTree(Expr,Rels,Result) :-
%   write_list(['$$$ Calling ',getTypeTree(Expr,Rels,Result),'\n']),
%   fail.

% Use temporarily saved results.
% Results of calls to getTypeTree/3 are saved as facts tmpStoredTypeTree/2
% for the time of the time until the optimizer is shut down or onother database
% is openend.
getTypeTree(Expr,_,Result) :-
  tmpStoredTypeTree(Expr,Result),!.

% Handle Lists of expressions
getTypeTree(arglist([]),_,[]).
getTypeTree(arglist([X|R]),Rels,[X1|R1]) :-
  getTypeTree(X,Rels,X1),
  getTypeTree(arglist(R),Rels,R1),
%   dm(selectivity,['$$$ getTypeTree: []: []']),nl,
  !.

% Primitive: int-atom
getTypeTree(IntAtom,_,[atom,IntAtom,int]) :-
  atomic(IntAtom), integer(IntAtom),
%   dm(selectivity,['$$$ getTypeTree: ',IntAtom,': ',int]),nl,
  assert(tmpStoredTypeTree(IntAtom,[atom,IntAtom,int])),
  !.

% Primitive: real-atom
getTypeTree(RealAtom,_,[atom,RealAtom,real]) :-
  atomic(RealAtom), float(RealAtom),
%   dm(selectivity,['$$$ getTypeTree: ',RealAtom,': ',real]),nl,
  assert(tmpStoredTypeTree(RealAtom,[atom,RealAtom,real])),
  !.

% Primitive: text-atom
getTypeTree(TextAtom,_,[atom,TextAtom,text]) :-
  atom(TextAtom),
  not(is_list(TextAtom)),
  not(opSignature(TextAtom,_,[],_,_)),
%   dm(selectivity,['$$$ getTypeTree: ',TextAtom,': ',text]),nl,
  assert(tmpStoredTypeTree(TextAtom,[atom,TextAtom,text])),
  !.

% Primitive: string-atom
getTypeTree(TextAtom,_,[atom,TextAtom,string]) :-
  is_list(TextAtom), TextAtom = [First | _], atomic(First), !,
  string_to_list(_,TextAtom),
%   dm(selectivity,['$$$ getTypeTree: ',TextAtom,': ',string]),nl,
  assert(tmpStoredTypeTree(TextAtom,[atom,TextAtom,string])),
  !.

% Primitive: relation-descriptor
getTypeTree(rel(DCrelName, X),_,[relation,rel(DCrelName, X),tuple(TupleList)]):-
  getRelAttrList(DCrelName, AttrList, _),
  findall((N,A),(member([N,A,_],AttrList)),TupleList),
% dm(selectivity,['$$$ getTypeTree: ',rel(DCrelName, X),': ',tuple(TupleList)]),
% nl,
  assert(tmpStoredTypeTree(rel(DCrelName, X),
                           [relation,rel(DCrelName, X),tuple(TupleList)])),
  !.

% Primitive: type-descriptor
getTypeTree(DCType,_,[typename,DCType,DCType]) :-
  secDatatype(DCType, _, _, _, _, _),
%   dm(selectivity,['$$$ getTypeTree: ',DCType,': ',DCType]),nl,
  assert(tmpStoredTypeTree(DCType,[typename,DCType,DCType])),
  !.

% Primitive: object-descriptor
getTypeTree(dbobject(NameDC),_,[dbobject,dbobject(NameDC),TypeDC]) :-
  secondoCatalogInfo(NameDC,_,_,[TypeExprList]),
  dcNList(TypeExprList,TypeDC),
%   dm(selectivity,['$$$ getTypeTree: ',dbobject(NameDC),': ',TypeDC]),nl,
  assert(tmpStoredTypeTree(dbobject(NameDC),
                           [dbobject,dbobject(NameDC),TypeDC])),
  !.

% Primitive: attribute-descriptor
getTypeTree(attr(RenRelName:Attr, Arg, Z),RelList,
        [attr,attr(RenRelName:Attr, Arg, Z),DCType]) :-
  memberchk((Arg,Rel),RelList),
  Rel = rel(DCrelName,RenRelName),
  downcase_atom(Attr,DCAttr),
  databaseName(DB),
  storedRel(DB,DCrelName,AttrList),
  memberchk(DCAttr,AttrList),
  storedAttrSize(DB,DCrelName,DCAttr,DCType,_,_,_),
%   dm(selectivity,['$$$ getTypeTree: ',attr(RenRelName:Attr, Arg, Z),
%   ': ',DCType]),nl,
  assert(tmpStoredTypeTree(attr(RenRelName:Attr, Arg, Z),
                          [attr,attr(RenRelName:Attr, Arg, Z),DCType])),
  !.
getTypeTree(attr(Attr, Arg, Y),Rels,[attr,attr(Attr, Arg, Y),DCType]) :-
  downcase_atom(Attr,DCAttr),
  memberchk((Arg,Rel),Rels),
  Rel = rel(DCrelName, _),
  databaseName(DB),
  storedRel(DB,DCrelName,AttrList),
  memberchk(DCAttr,AttrList),
  storedAttrSize(DB,DCrelName,DCAttr,DCType,_,_,_),
%   dm(selectivity,['$$$ getTypeTree: ',attr(Attr, Arg, Y),': ',DCType]),nl,
  assert(tmpStoredTypeTree(attr(Attr, Arg, Y),
                          [attr,attr(Attr, Arg, Y),DCType])),
  !.

% Primitive: attribute-name-descriptor
getTypeTree(attrname(attr(X:Name, Y, Z)),_,[attrname,
        attrname(attr(X:Name, Y, Z)),DCType]) :-
  downcase_atom(Name,DCType),
%   dm(selectivity,['$$$ getTypeTree: ',attrname(attr(X:Name, Y, Z)),
%   ': ',DCType]),nl,
  assert(tmpStoredTypeTree(attrname(attr(X:Name, Y, Z)),_,[attrname,
        attrname(attr(X:Name, Y, Z)),DCType])),
  !.
getTypeTree(attrname(attr(Name, Y, Z)),_,[attrname,
        attrname(attr(Name, Y, Z)),DCType]) :-
  downcase_atom(Name,DCType),
%   dm(selectivity,['$$$ getTypeTree: ',attrname(attr(Name, Y, Z)),
%   ': ',DCType]),nl,
  assert(tmpStoredTypeTree(attrname(attr(Name, Y, Z)),_,[attrname,
        attrname(attr(Name, Y, Z)),DCType])),
  !.

% Primitive: newly created attribute
getTypeTree(newattr(AttrExpr,ValExpr),Rels,[newattr,
        [AttrExprTree,ValExprTree],DCType]) :-
  getTypeTree(arglist([AttrExpr,ValExpr]),Rels,[AttrExprTree,ValExprTree]),
  ValExprTree = [_,_,DCType],
%   dm(selectivity,['$$$ getTypeTree: ',newattr(AttrExpr,ValExpr),
%   ': ',DCType]),nl,
  assert(tmpStoredTypeTree(newattr(AttrExpr,ValExpr),Rels,
        [newattr,[AttrExprTree,ValExprTree],DCType])),
  % also store the new attribue itself:
  ( AttrExpr = attrname(attr(Name, Arg, Case))
    -> assert(tmpStoredTypeTree(attr(Name, Arg, Case),
                                  [attr,attr(Name, Arg, Case),DCType]))
    ;  true
  ), !.

% Expression using defined operator signature
getTypeTree(Expr,Rels,[Op,ArgTree,TypeDC]) :-
  compound(Expr),
  Expr =.. [Op|Args],
  getTypeTree(arglist(Args),Rels,ArgTree),
  findall(T,member([_,_,T],ArgTree),ArgTypes), % extract types from ArgTree
  (   opSignature(Op,_,ArgTypes,TypeDC,_)
    ; queriedOpSignature(Op,ArgTypes,TypeDC,_)
  ),
%   dm(selectivity,['$$$ getTypeTree: ',Expr,': ',TypeDC]),nl,
  assert(tmpStoredTypeTree(Expr,[Op,ArgTree,TypeDC])),
  !.

% Expression using unknown operator signature
getTypeTree(Expr,Rels,[Op,ArgsTypes,TypeDC]) :-
  compound(Expr),
  Expr =.. [Op|Args],
  getTypeTree(Args,Rels,ArgTree),
     % extract types from ArgTree
  findall(T,member([_,_,T],ArgTree),ArgTypes),
     % create a valid expression using defined null values
  createNullValues(ArgsTypes,NullArgs),
     % send getTypeNL-query to Secondo to infer the signature
  NullQueryExpr =.. [Op|NullArgs],
  plan_to_atom(getTypeNL(NullQueryExpr),NullQueryAtom),
  concat_atom(['query ',NullQueryAtom],'',Query),
%   dm(selectivity,['getTypeNL-Query = ',Query]),nl,
  secondo(Query,[text,TypeDC]),
     % store signature in fact base
  assert(queriedOpSignature(Op,ArgTypes,TypeDC,[])),
%   dm(selectivity,['$$$ getTypeTree: ',Expr,': ',TypeDC]),nl,
  assert(tmpStoredTypeTree(Expr,[Op,ArgsTypes,TypeDC])),
  !.

getTypeTree(A,B,C) :-
    term_to_atom(A,A1),
    concat_atom(['Cannot resolve typetree for expression \'',A1,'\'.'],'',MSG),
    throw(error_Internal(statistics_getTypeTree(A,B,C):typeMapError#MSG)),
    !, fail.

/*
The following predicate changes a complete nested list to use DC-spelling
only.

---- dcNList(+NList,?DCNList)
----

*/
dcNList(X1,X2) :-
  atomic(X1),
  downcase_atom(X1,X2),!.
dcNList([],[]).
dcNList([X1|R1],[X2|R2]) :-
  dcNList(X1,X2),
  dcNList(R1,R2),!.

/*
The next predicate creates a list of Null Values for a given type list.
For stream and relation arguments, typed but empty instances are returned.
For mappings, typed dummy mappings are created.

---- createNullValues(+Type,-Null)
----

~Type~ is a single type or a list of types.

~Null~ is the according Null Value, resp. a list of such NVs.

*/
% special case: indexes
createNullValues([tuple,_],_) :- fail.
createNullValues([rtree|_],_) :- fail.
createNullValues([rtree3|_],_) :- fail.
createNullValues([rtree3|_],_) :- fail.

% special case: relation types
createNullValues([RelType,[tuple,X]],NullRelString) :-
  memberchk(RelType,[rel,trel]),
  createConstAttrList(X,X1),
  concat_atom(['[const ',RelType,'(tuple([',X1,'])) value ()]'],'',NullRelAtom),
  string_to_atom(NullRelString,NullRelAtom),!.

% special case: tuple streams
createNullValues([stream,[tuple,X]],feed(NullRelString)) :-
  createConstAttrList(X,X1),
  concat_atom(['[const ',rel,'(tuple([',X1,'])) value ()]'],'',NullRelAtom),
  string_to_atom(NullRelString,NullRelAtom),!.

% special case: data streams
createNullValues([stream,T],feed(TNV)) :-
  isData(T),
  createNullValues(T,TNV),!.

% special case: parameter functions/ mappings
createNullValues([map|Args],fun(Params, ResNV)) :-
  append(ArgTypeList,[ResType],Args),
  createNullValues(ResType,ResNV),
  createFunArgs(ArgTypeList,Params),!.

% handle lists
createNullValues([],[]).
createNullValues([X|R],[X1|R1]) :-
  createNullValues(X,X1),
  createNullValues(R,R1),!.

% handle single values
createNullValues(Type,NVstring) :-
  (  nullValue(Type,standard,NV) % try 'standard' value first
   ; nullValue(Type,empty,NV)    % 'empty' value comes second
   ; nullValue(Type,_,NV)        % any other NullValue as last resort
  ), !, % do not try again
  concat_atom(['[const ',Type,' value ',NV,']'],'',NVatom),
  string_to_atom(NVstring,NVatom),!.

% create an atom describing an attribute list
createConstAttrList([],'').
createConstAttrList([[Name,Type]],Expr) :-
  concat_atom([Name,Type],':',Expr),!.
createConstAttrList([[Name,Type]|Rest],Expr) :-
  Rest \= [],
  concat_atom([Name,Type],':',Expr1),
  createConstAttrList(Rest,Expr2),
  concat_atom([Expr1,Expr2],',',Expr),!.

% create an argument list for a dummy function with given signature
% List format: [param(Var1, Type1), ..., param(VarN, TypeN)]
createFunArgs([],[]).
createFunArgs([ArgType|R],[param(Var, ArgType)|Args]) :-
  newVariable(Var),
  createFunArgs(R,Args),!.


/*
1 Printing Metadata on Database

---- showDatabase
----

This predicate will inquire all collected statistical data on the
opened Secondo database and print it on the screen.

*/

:-assert(helpLine(showDatabase,0,[],
         'List available metadata on relations within current database.')).

showSingleOrdering(DB, Rel) :-
  findall( X,
           ( storedOrder(DB,Rel,X1),
             translateOrderingInfo(Rel, X1, X)
           ),
           OrderingAttrs
         ),
  write('\n\n\tOrdering:  '),
  write(OrderingAttrs), nl, !.

translateOrderingInfo(_, none,none) :- !.
translateOrderingInfo(_, shuffled,shuffled) :- !.
translateOrderingInfo(Rel, Attr, AttrS) :-
  dcName2externalName(Rel:Attr, AttrS).

showSingleRelationCard(DB, Rel) :-
  storedCard(DB, Rel, Card),
  write('\n\n\tCardinality:   '), write(Card), nl, !.

showSingleRelationCard(_, _) :-
  write('\n\n\tCardinality:   *'), nl, !.

showSingleRelationTuplesize(_, Rel) :-
  tuplesize(Rel, Size),
  tupleSizeSplit(Rel, Size2),
  write('\tAvg.TupleSize: '), write(Size), write(' = '),
    write(Size2), nl, !.

showSingleRelationTuplesize(_, _) :-
  write('\tAvg.TupleSize: *'), nl, !.

showSingleIndex(Rel) :-
  databaseName(DB),
  storedIndex(DB, Rel, Attr, IndexType, _),
  dcName2externalName(Rel:Attr, AttrS),
  write('\t('), write(AttrS), write(':'), write(IndexType), write(')').

showSingleRelation :- showRelation(_).
:- assert(helpLine(showRelation,1,
    [[+,'RelDC','The relation to get information about.']],
    'Show meta data on a given relation.')).

showRelation(Rel) :-
  databaseName(DB),
  storedRel(DB, Rel, _),
  dcName2externalName(Rel, RelS),
  ( ( sub_atom(Rel,_,_,1,'_sample_') ; sub_atom(Rel,_,_,0,'_small') )
    -> fail
    ;  true
  ),
  write('\nRelation '), write(RelS),
  ( systemTable(Rel,_)
    -> write('\t***SYSTEM TABLE***')
    ;  true
  ),
  getSampleSname(RelS, SampleS),
  getSampleJname(RelS, SampleJ),
  getSmallName(RelS, Small),
  write('\t(Auxiliary objects:'),
  ( secondoCatalogInfo(DCSampleS,SampleS,_,_)
    -> (card(DCSampleS,CardSS), write_list([' SelSample(',CardSS,') ']) )
    ; true ),
  ( secondoCatalogInfo(DCSampleJ,SampleJ,_,_)
    -> (card(DCSampleJ,CardSJ), write_list([' JoinSample(',CardSJ,') ']) )
    ; true ),
  ( secondoCatalogInfo(DCSmall,Small  ,_,_)
    -> (card(DCSmall,CardSM), write_list([' SmallObject(',CardSM,') ']) )
    ; true ),
  write(')'), nl,
  findall(_, showAllAttributes(Rel), _),
  findall(_, showAllIndices(Rel), _),
  showSingleOrdering(DB, Rel),
  showSingleRelationCard(DB, Rel),
  showSingleRelationTuplesize(DB, Rel).

showSingleAttribute(Rel,Attr) :-
  databaseName(DB),
  storedAttrSize(DB, Rel, Attr, Type, MemSize, CoreSize, LobSize),
  dcName2externalName(Rel:Attr, AttrS),
  format('\t~p~35|~p~49|~p~60|~p~69|~p~n',
  [AttrS, Type, MemSize, CoreSize, LobSize]).

showAllAttributes(Rel) :-
  format('\t~p~35|~p~49|~p~60|~p~69|~p~n',
  ['AttributeName','Type','MemoryFix','DiskCore','DiskLOB']),
  findall(_, showSingleAttribute(Rel, _), _).

showAllIndices(Rel) :-
  write('\n\tIndices: \n'),
  findall(_, showSingleIndex(Rel), _).

showDatabase :-
  databaseName(DB),
  write('\nCollected information for database \''), write(DB),
    write('\':\n'),
  findall(_, showSingleRelation, _),
  write('\n(Type \'showDatabaseSchema.\' to view the complete '),
  write('database schema.)\n').

showDatabase :-
  write('\nNo database open. Use open \'database <name>\' to'),
  write(' open an existing database.\n'),
  fail.

/*
1 Examples

The following examples can be used used to test the functionality of this module.

Example 22:

*/
example24 :- optimize(
  select *
  from [staedte as s, ten]
  where [
    s:plz < 1000 * no,
    s:plz < 4578]
  ).

/*
Example 23:

*/

example23 :- optimize(
  select *
  from [thousand as th, ten]
  where [
    (th:no mod 10) < 5,
    th:no * 100 < 50000,
    (th:no mod 7) = no]
  ).
