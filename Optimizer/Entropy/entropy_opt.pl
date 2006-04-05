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

April 2006, M. Spiekermann. Isolating the changes neede for the entropy optimizer.
Redundant code (code which is also defined in the standard optimizer) removed.

*/



/*
----    writeSizes :-
----

Write first sizes and compare sizes.

*/

writeFirstSize :-
  firstResultSize(Node, Size),
  write('Node: '), write(Node), nl,
  write('Size: '), write(Size), nl, nl,
  fail.
writeFirstSize :-
  firstEdgeSelectivity(Source, Target, Sel),
  write('Source: '), write(Source), nl,
  write('Target: '), write(Target), nl,
  write('Selectivity: '), write(Sel), nl, nl,
  fail.
writeFirstSizes :- not(writeFirstSize).

compareSize :-
  firstResultSize(Node, Size1),
  resultSize(Node, Size2),
  write('Node: '), write(Node),
  write(', Size: '), write(Size1),
  write(' ==> '), write(Size2), nl, nl,
  fail.
compareSize :-
  firstEdgeSelectivity(Source, Target, Sel1),
  edgeSelectivity(Source, Target, Sel2),
  write('Source: '), write(Source),
  write(', Target: '), write(Target),
  write(', Selectivity: '), write(Sel1),
  write(' ==> '), write(Sel2), nl, nl,
  fail.
compareSizes :- not(compareSize).


% nCounter tracks the number of counters in queries to get the size of intermediate results.
%:-
%  dynamic(nCounter/1).

nextCounter(C) :-
  nCounter(N),
  !,
  N1 is N + 1,
  retract(nCounter(N)),
  assert(nCounter(N1)),
  C = N1.

nextCounter(C) :-
  assert(nCounter(1)),
  C = 1.

% deleteCounter :- retract(nCounter(_)), fail.
%
% deleteCounters :- not(deleteCounter).

traversePath2([]).

traversePath2([costEdge(Source, Target, Term, Result, _, _) | Path]) :-
  embedSubPlans(Term, Term2),
  nextCounter(Nc),
  assert(nodePlan(Result, counter(Nc,Term2))),
  assert(smallResultCounter(Nc, Source, Target, Result)),
  traversePath2(Path).

deleteSmallResultCounter :-
  retractall(smallResultCounter(_,_,_,_)).

createSmallResultSize([]).
createSmallResultSize([ [Nc,Value] | T ]) :-
  smallResultCounter(Nc, _, _, Result ),
  assert(smallResultSize(Result, Value)),
  createSmallResultSize( T ).

createSmallResultSizes2 :-
  deleteSmallResultSize,
  secondo('list counters', C ), !,
  createSmallResultSize( C ).

createSmallResultSizes :-
  deleteSmallResultSize, !,
  not(createSmallResultSizes2).

deleteSmallResultSize :-
  retractall(smallResultSize(_,_)).

createSmallSelectivity :-
  deleteSmallSelectivity, !,
  not(createSmallSelectivity2).

deleteSmallSelectivity :-
  retractall(small_cond_sel(_,_,_,_)).

compute_sel( 0, 0, Sel ) :-
  Sel is 1.

compute_sel( Num, Den, Sel ) :-
  Sel is Num / Den.

assignSmallSelectivity(Source, Target, Result, select(Arg, _), Value) :-
  newResSize(Arg, Card),
  compute_sel( Value, Card, Sel ),!,
  assert(small_cond_sel(Source, Target, Result, Sel)).

assignSmallSelectivity(Source, Target, Result, join(Arg1, Arg2, _), Value) :-
  newResSize(Arg1, Card1),
  newResSize(Arg2, Card2),
  Card is Card1 * Card2,
  compute_sel( Value, Card, Sel ),!,
  assert(small_cond_sel(Source, Target, Result, Sel)).

createSmallSelectivity2 :-
  smallResultCounter(_, Source, Target, Result),
  smallResultSize(Result, Value),
  edge(Source,Target,Term,Result,_,_),
  assignSmallSelectivity(Source, Target, Result, Term, Value),
  fail.

small(rel(Rel, Var, Case), rel(Rel2, Var, Case)) :-
  atom_concat(Rel, '_small', Rel2).

newResSize(arg(N), Size) :- argument(N, R ), small( R, rel(SRel, _, _)), card(SRel, Size), !.
newResSize(res(N), Size) :- smallResultSize(N, Size), !.

/*
prepare\_query\_small prepares the query to be executed in the small database.
Assumes that the small database has the same indexes that are in the full database,
but with the sufix '\_small'
*/

prepare_query_small( count(Term), count(Result) ) :-
  query_small(Term, Result).

prepare_query_small( Term, count(Result) ) :-
  query_small(Term, Result).

query_small(rel(Name, V, C), Result) :-
  atom_concat( Name, '_small', NameSmall ),
  Result = rel(NameSmall, V, C),
  !.

query_small(exactmatch(IndexName, R, V), Result) :-
  atom_concat(IndexName,'_small', IndexNameSmall),
  query_small( R, R2 ),
  Result = exactmatch(IndexNameSmall, R2, V),
  !.

% To be modified - it should handle functors with any number of arguments. Currently it
% handles only from 1 to 5 arguments. It should handle lists, too.

query_small( Term, Result ) :-
  functor(Term, Fun, 1 ),
  arg(1, Term, Arg1),
  query_small(Arg1, Res1),
  Result =.. [Fun | [Res1]],
  !.

query_small( Term, Result ) :-
  functor(Term, Fun, 2 ),
  arg(1, Term, Arg1),
  arg(2, Term, Arg2),
  query_small(Arg1, Res1),
  query_small(Arg2, Res2),
  Result =.. [Fun | [Res1, Res2]],
  !.

query_small( Term, Result ) :-
  functor(Term, Fun, 3 ),
  arg(1, Term, Arg1),
  arg(2, Term, Arg2),
  arg(3, Term, Arg3),
  query_small(Arg1, Res1),
  query_small(Arg2, Res2),
  query_small(Arg3, Res3),
  Result =.. [Fun | [Res1, Res2, Res3]],
  !.

query_small( Term, Result ) :-
  functor(Term, Fun, 4 ),
  arg(1, Term, Arg1),
  arg(2, Term, Arg2),
  arg(3, Term, Arg3),
  arg(4, Term, Arg4),
  query_small(Arg1, Res1),
  query_small(Arg2, Res2),
  query_small(Arg3, Res3),
  query_small(Arg4, Res4),
  Result =.. [Fun | [Res1, Res2, Res3, Res4]],
  !.

query_small( Term, Result ) :-
  functor(Term, Fun, 5 ),
  arg(1, Term, Arg1),
  arg(2, Term, Arg2),
  arg(3, Term, Arg3),
  arg(4, Term, Arg4),
  arg(5, Term, Arg5),
  query_small(Arg1, Res1),
  query_small(Arg2, Res2),
  query_small(Arg3, Res3),
  query_small(Arg4, Res4),
  query_small(Arg5, Res5),
  Result =.. [Fun | [Res1, Res2, Res3, Res4, Res5]],
  !.

query_small( Term, Result ) :-
  Result = Term,
  !.

/*
We introduce ~sql2~ as PROLOG operator:

*/

:- op(990, fx, sql2).


/*
Modifications in queryToStream(Query, Plan, Cost) :-


queryToStream(Query first N, head(Stream, N), Cost) :-
  queryToStream(Query, Stream, Cost),
  !.

queryToStream(Query orderby SortAttrs, Stream2, Cost) :-
  translate(Query, Stream, Select, Cost),
  finish(Stream, Select, SortAttrs, Stream2),
  !.

queryToStream(Query, Stream2, Cost) :-
  translate(Query, Stream, Select, Cost),
  finish(Stream, Select, [], Stream2).

*/

/*
  Entropy stuff.

*/

translate2(Stream1, Stream2, Cost1, Cost2) :-
  deleteSmallResults,   
  %  retractall(highNode(_)),assert(highNode(0)),
  %  translate(Query, Stream1, Select, Cost1), !,
  try_entropy(Stream1, Stream2, Cost1, Cost2), !,
  warn_plan_changed(Stream1, Stream2).

try_entropy(Stream1, Stream2, Cost1, Cost2) :-
  useEntropy, highNode(HN), HN > 1, HN < 256, !,
  nl, write('*** Trying to use the Entropy-approach ******************************' ), nl, !,
  plan_to_atom(Stream1, FirstQuery),
  prepare_query_small(Stream1, PlanSmall),
  plan_to_atom(PlanSmall, SmallQuery),
  write('The plan in small database is: '), nl, write(SmallQuery), nl, nl,
  write('Executing the query in the small database...'),
  deleteEntropyNodes, !,
  query(SmallQuery), !, nl,
  assignEntropyCost, !,
  write('First Plan:'), nl, write( FirstQuery ), nl, nl,
  write('Estimated Cost: '), write(Cost1), nl, nl,
  entropyBestPlan(Stream2, Cost2).

try_entropy(Stream1, Stream1, Cost1, Cost1).

entropyBestPlan(Plan,Cost) :-
  deleteCounters,
  highNode(N),
  dijkstra(0, N, Path, Cost),
  plan(Path, Plan).

warn_plan_changed(Plan1, Plan2) :-
  not(Plan1 = Plan2),
  nl,
  write( '*******************************************************' ), nl,
  write( '* * *  INITIAL PLAN CHANGED BY ENTROPY APPROACH!  * * *' ), nl,
  write( '*******************************************************' ), nl.

warn_plan_changed(_,_).


/*

12 Optimizing and Calling Secondo.

*/

sql2 Term :-
  isDatabaseOpen,
  use_entropy,
  mOptimize(Term, Query, Cost),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  query(Query),
  dont_use_entropy.

sql2(Term, SecondoQueryRest) :-
  isDatabaseOpen,
  use_entropy,
  mStreamOptimize(Term, SecondoQuery, Cost),
  concat_atom([SecondoQuery, ' ', SecondoQueryRest], '', Query),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  query(Query),
  dont_use_entropy.


/*
Some auxiliary stuff.

*/

entropySel( 0, Target, Sel ) :-
  entropy_node( Target, Sel ).

entropySel( Source, Target, Sel ) :-
  entropy_node( Source, P1 ),
  entropy_node( Target, P2 ),
  P1 > 0,
  Sel is P2 / P1.
/*
Now it is assuming an implicit order. Should be altered to work in the same way as conditional probabilities
*/

createMarginalProbabilities( MP ) :-
  createMarginalProbability( 0, MP ).

createMarginalProbability( N, [Sel|T] ) :-
  edgeSelectivity(N, M, Sel),
  createMarginalProbability( M, T ).

createMarginalProbability( _, [] ).

createJointProbabilities( JP ) :-
  createJointProbability( 0, 1, [_|JP] ).

createJointProbability( N0, AccSel, [[N1,CP1]|T] ) :-
  nl, write('X4'), nl,
  small_cond_sel( N0, N1, _, Sel ),
  nl, write('X5'), nl,
  CP1 is Sel * AccSel,
  createJointProbability( N1, CP1, T ).

createJointProbability( _, _, [] ).


assignEntropyCost :-
  nl, write('X1'), nl,
  createSmallResultSizes, !,
  createSmallSelectivity, !,
  nl, write('X2'), nl,
  createMarginalProbabilities( MP ),!,
  nl, write('X3'), nl,
  createJointProbabilities( JP ),!,
  nl, write('SUCCESS'), nl,
  saveFirstSizes,
  deleteSizes,
  deleteCostEdges,
  maximize_entropy(MP, JP, Result), !,
  createEntropyNode(Result),
  assignEntropySizes,
  write( MP ), write(', ') , write( JP ), nl, nl,
  createCostEdges.

assignEntropySizes :- not(assignEntropySizes1).

assignEntropySizes1 :-
  edge(Source, Target, Term, Result, _, _),
  assignEntropySize(Source, Target, Term, Result),
  fail.

assignEntropySize(Source, Target, select(Arg, _), Result) :-
  resSize(Arg, Card),
  entropySel(Source, Target, Sel),
  Size is Card * Sel,
  setNodeSize(Result, Size),
  assert(edgeSelectivity(Source, Target, Sel)).

assignEntropySize(Source, Target, join(Arg1, Arg2, _), Result) :-
  resSize(Arg1, Card1),
  resSize(Arg2, Card2),
  entropySel(Source, Target, Sel),
  Size is Card1 * Card2 * Sel,
  setNodeSize(Result, Size),
  assert(edgeSelectivity(Source, Target, Sel)).

deleteEntropyNodes :-
  retractall(entropy_node(_,_)).

createEntropyNode( [] ).
createEntropyNode( [[N,E]|L] ) :-
  assert(entropy_node(N,E)),
  createEntropyNode( L ).

:- dynamic
   smallResultSize/2,
   smallResultCounter/4,
   entropy_node/2,
   small_cond_sel/4,
   useEntropy/0,
   firstResultSize/2,
   firstEdgeSelectivity/3.

useEntropy.

use_entropy :-
  assert(useEntropy).

dont_use_entropy :-
  retractall(useEntropy).
  
deleteSmallResults :-
  deleteSmallResultCounter,
  deleteSmallResultSize,
  deleteSmallSelectivity,
  deleteFirstSizes.

saveFirstSizes :-
  not(copyFirstResultSize), !,
  not(copyFirstEdgeSelectivity).

copyFirstResultSize :-
  resultSize(Result, Size),
  assert(firstResultSize(Result, Size)),
  fail.

copyFirstEdgeSelectivity :-
  edgeSelectivity(Source, Target, Sel),
  assert(firstEdgeSelectivity(Source, Target, Sel)),
  fail.

deleteFirstSizes :-
  retractall(firstResultSize(_,_)),
  retractall(firstEdgeSelectivity(_,_,_)).
  
quit :- 
  halt.

argList( 1, [_] ).
argList( N, [_|L] ) :-
  N1 is N-1,
  argList( N1, L ).

showValues( Pred, Arity ) :-
  not(showValues2( Pred, Arity )).

showValues2( Pred, Arity ) :-
  argList( Arity, L ),
  P=..[Pred|L], !, P, nl, write( P ), fail.
  


% sql select count(*) from plz as p where [p:plz > 40000, p:plz < 50000].
% sql select count(*) from plz as p where [(p:plz mod 20) = 0, (p:plz mod 30) = 0].


