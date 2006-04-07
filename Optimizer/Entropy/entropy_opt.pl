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

September 2005, G. Zimbrao. Initial version. 

April 2006, M. Spiekermann. Isolating the changes needed for the entropy optimizer.
Redundant code (code which is also defined in the standard optimizer) removed.
Some parts were merged with the standard optimizer. Moreover a mechanism for
switching between ~standard~ and ~entropy~ optimiztation was implemented in
~calloptimizer.pl~. Finally, the ~entropy~ code was documented.

*/


/*
1 Data Structures

*/

:- dynamic
   % nCounter/1, defined in optimizer.pl
   % nCounter tracks the number of counters in queries which are used
   % to compute the size of intermediate results along a path through the POG.

   smallResultSize/2,
   smallResultCounter/4,
   
   % smallResultCounter(Nc, Source, Target, Result))
   % 
   
   entropy_node/2,
   small_cond_sel/4,
   useEntropy/0,
   firstResultSize/2,
   firstEdgeSelectivity/3.

/*
Some clauses which print the estimated sizes of the standard optimizer
and compares them with the sizes calculated by the entropy approach.

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

/*
Maintain and increment the number of used counters. The dynamic fact ~nCounter~
tracks the number of counters in queries which are used to compute the size of
intermediate results along a path through the POG.

*/

%:- dynamic
%     nCounter/1, defined in optimizer.pl

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


/* 
The clauses below are used to store a list of counters returned by SECONDO.
The command ~list counters~ will return a list of pairs (ctrNum, value) which
is stored in ~smallResultSize/2~

*/
 
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

/*
Based on the counter values we can compute the conditional selectivities
along the path chosen by the ~standard~ optimizer. The values are stored
in ~small\_cond\_sel/4~.

When both counter values are 0 or equal, we have a selectivity of 1. In this case we assign 0.99 instead, to ``leave some space'' for avoiding zero atoms.

*/
 
compute_sel(0, 0, 0.99).

compute_sel(X, X, 0.99).

compute_sel(Num, Den, Sel) :-
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

createSmallSelectivity :-
  deleteSmallSelectivity, !,
  not(createSmallSelectivity2).
 
createSmallSelectivity2 :-
  smallResultCounter(_, Source, Target, Result),
  smallResultSize(Result, Value),
  edge(Source,Target,Term,Result,_,_),
  assignSmallSelectivity(Source, Target, Result, Term, Value),
  fail.

/*
2 Some clauses operationg the ~small~ database.

prepare\_query\_small prepares the query to be executed in the small database.
Assumes that the small database has the same indexes that are in the full database,
but with the sufix '\_small'

*/
 
small(rel(Rel, Var, Case), rel(Rel2, Var, Case)) :-
  atom_concat(Rel, '_small', Rel2).

newResSize(arg(N), Size) :- argument(N, R ), small( R, rel(SRel, _, _)), card(SRel, Size), !.
newResSize(res(N), Size) :- smallResultSize(N, Size), !.


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
3 Interaction with the ~standard~ optimizer

At some places in optimizer.pl it will be checked
whether ~usingVersion(entropy)~ holds. If this is true, ~clause2~ will be 
called instead of ~clause~.

*/


/*
The rule ~traversePath~ is used inside predicate ~plan/2~ of the
standard optimizer, but for the entropy approach the version below will
be called. In this case the values of the counters will be stored in 

----  nodePlan/2
      smallResultCounter/4
----      

*/

traversePath2([]).

traversePath2([costEdge(Source, Target, Term, Result, _, _) | Path]) :-
  %nl, write('### traversePath2'), nl,
  embedSubPlans(Term, Term2),
  nextCounter(Nc),
  assert(nodePlan(Result, counter(Nc,Term2))),
  assert(smallResultCounter(Nc, Source, Target, Result)),
  traversePath2(Path).

/*
The clause ~translate2~ runs a plan given as ~STREAM1~ with ~Cost1~, runs
it on the small database. Conditional selectivities are computed along
the path of the first plan. Then the iterative scaling algorithm computes the
remaining conditional selectivites. The new selectivites are assigned to the
POG and a second plan is computed and returned as ~Stream2~ with costs ~Cost2~.

*/

translate2(Stream1, Stream2, Cost1, Cost2) :-
  % deleteSmallResults,   
  %  retractall(highNode(_)),assert(highNode(0)),
  %  translate(Query, Stream1, Select, Cost1), !,
  try_entropy(Stream1, Stream2, Cost1, Cost2), !,
  warn_plan_changed(Stream1, Stream2).

try_entropy(Stream1, Stream2, Cost1, Cost2) :-
 
  useEntropy, highNode(HN), HN > 1, HN < 256, !,
  
  nl, 
  write('*** Using Entropy-approach ************' ), 
  nl, !,
  
  plan_to_atom(Stream1, FirstQuery),
  prepare_query_small(Stream1, PlanSmall),
  plan_to_atom(PlanSmall, SmallQuery),
  
  write('The plan in small database is: '), nl, 
  write(SmallQuery), nl, nl,
  write('Executing the query in the small database...'),
  
  deleteEntropyNodes, !,
  query(SmallQuery), !, nl,
  
  write('First Plan:'), nl, 
  write( FirstQuery ), nl, nl,
  write('Estimated Cost: '), 
  write(Cost1), nl, nl,
 
  % compute a new plan. The assignCost clause will use assignEntropyCost. 
  bestPlan(Stream2, Cost2).

try_entropy(Stream1, Stream1, Cost1, Cost1).

warn_plan_changed(Plan1, Plan2) :-
  not(Plan1 = Plan2),
  nl,
  write( '************************************' ), nl,
  write( '* * *  INITIAL PLAN CHANGED !  * * *' ), nl,
  write( '************************************' ), nl.

warn_plan_changed(_,_).


/*

We introduce ~sql2~ as a prolog operator.

*/

:- op(990, fx, sql2).


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
Now it is assuming an implicit order. Should be altered to work in the same way as conditional probabilities.

*/

createMarginalProbabilities( MP ) :-
  createMarginalProbability( 0, MP ).

createMarginalProbability( N, [[Pred, Sel]|T] ) :-
  edgeSelectivity(N, M, Sel),
  Pred is M - N,
  createMarginalProbability( M, T ).

createMarginalProbability( _, [] ).

createJointProbabilities( JP ) :-
  createJointProbability( 0, 1, JP ).

createJointProbability( N0, AccSel, [[N0, N1, CP1]|T] ) :-
  small_cond_sel( N0, N1, _, Sel ),
  CP1 is Sel * AccSel,
  createJointProbability( N1, CP1, T ).

createJointProbability( _, _, [] ).









:- dynamic

  marginal/2.



saveMarginal([]).

saveMarginal([[Pred, Sel]|L]) :-
  assert(marginal(Pred, Sel)),
  saveMarginal(L).


loadMarginal(MP) :-
  findall([Pred, Sel], marginal(Pred, Sel), MP).


deleteMarginal :- 
  retractall(marginal(_, _)).





















assignEntropyCost :-
  createSmallResultSizes, !,
  createSmallSelectivity, !,
  createMarginalProbabilities( MP ),!,
  createJointProbabilities( JP ),!,
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


createEntropyNode( [] ).
createEntropyNode( [[N,E]|L] ) :-
  assert(entropy_node(N,E)),
  createEntropyNode( L ).


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

 
/*
Delete facts stored during the ~entropy~ optimization
procedure.

*/
 
deleteEntropyNodes :-
  retractall(entropy_node(_,_)).
 
deleteFirstSizes :-
  retractall(firstResultSize(_,_)),
  retractall(firstEdgeSelectivity(_,_,_)).
  
deleteSmallSelectivity :-
  retractall(small_cond_sel(_,_,_,_)).
 
deleteSmallResults :-
  deleteSmallResultCounter,
  deleteSmallResultSize,
  deleteSmallSelectivity,
  deleteFirstSizes.
 
 
/*
Auxiliary stuff

*/
 
useEntropy.

use_entropy :-
  assert(useEntropy).

dont_use_entropy :-
  retractall(useEntropy).
  
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


