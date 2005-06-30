:- dynamic newResultSize/2, resultCounter/4, sample_sel/2, entropy_node/2,sample_cond_sel/4.

optimize_entropy( Query ) :-
  deleteResultCounter,
  deleteNewResultSize,
  deleteSampleSelectivity,
  callLookup(Query, Query2),
  queryToPlan(Query2, Plan, Cost),
  plan_to_atom(Plan, FirstQuery),
  createMarginalProbabilities( MP ), !,
  write( 'First Plan:' ), nl, write( FirstQuery ), nl, nl,
  query_sample(Plan, Plan2),
  plan_to_atom(Plan2, SampleQuery),
  write('The plan in sample is: '), nl,
  write(SampleQuery), nl, nl,
  write('Executing the query in the sample...'),
  query(SampleQuery), !, nl, nl,
  createNewResultSizes,
  createSampleSelectivity,
  createJointProbabilities( JP ),
  assignEntropyCost( MP, JP ),!, nl, write( MP ), nl, write( JP ), nl,
  write( 'First Plan:' ), nl, write( FirstQuery ), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  entropyBestPlan.




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
  sample_cond_sel( N0, N1, _, Sel ),
  CP1 is Sel * AccSel,
  createJointProbability( N1, CP1, T ).

createJointProbability( _, _, [] ).

deleteSampleSelectivity :-
  retractall(sample_sel(_,_)),
  retractall(sample_cond_sel(_,_,_,_)).

createSampleSelectivity2 :-
  resultCounter(_, Source, Target, Result),
  newResultSize(Result, Value),
  edge(Source, Target, Term, Result, _, _),
  assignSampleSelectivity(Source, Target, Result, Term, Value),
  fail.

assignSampleSelectivity(Source, Target, Result, select(Arg, Pr), Value) :-
  newResSize(Arg, Card),
  Sel is Value / Card,
  simplePred(Pr, SPr ),!,
  assert(sample_cond_sel(Source, Target, Result, Sel)),
  assert(sample_sel(SPr,Sel)).

assignSampleSelectivity(Source, Target, Result, join(Arg1, Arg2, Pr), Value) :-
  newResSize(Arg1, Card1),
  newResSize(Arg2, Card2),
  Sel is Value / (Card1 * Card2),
  simplePred(Pr, SPr ), !,
  assert(sample_cond_sel(Source, Target, Result, Sel)),
  assert(sample_sel(SPr,Sel)).

entropyBestPlan :-
  deleteCounters,
  highNode(N),
  dijkstra(0, N, Path, Cost),
  plan(Path, Plan),
  write('The best plan is:'), nl, nl,
  wp(Plan),
  nl, nl,
  write('The cost is: '), write(Cost), nl.

assignEntropyCost(Marginalprob, JointProb) :-
  deleteEntropyNodes,
  deleteSizes,
  deleteCostEdges, !,
  maximize_entropy(Marginalprob, JointProb, Result),
  createEntropyNode(Result),
  assignEntropySizes,
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

entropySel( 0, Target, Sel ) :-
  entropy_node( Target, Sel ).

entropySel( Source, Target, Sel ) :-
  entropy_node( Source, P1 ),
  entropy_node( Target, P2 ),
  P1 > 0,
  Sel is P2 / P1.

argList( 1, [_] ).
argList( N, [_|L] ) :-
  N1 is N-1,
  argList( N1, L ).

showValues( Pred, Arity ) :-
  not(showValues2( Pred, Arity )).

showValues2( Pred, Arity ) :-
  argList( Arity, L ),
  P=..[Pred|L], !, P, nl, write( P ), fail.

small(rel(Rel, Var, Case), rel(Rel2, Var, Case)) :-
  atom_concat(Rel, '_small', Rel2).

newResSize(arg(N), Size) :- argument(N, R ), small( R, rel(SRel, _, _)), card(SRel, Size), !.
newResSize(res(N), Size) :- newResultSize(N, Size), !.

/*
query_sample prepares the query to be executed in the small database.
Assumes that the small database has the same indexes that are in the full database,
but with the sufix '_small'
*/

query_sample(rel(Name, V, C), Result) :-
  atom_concat( Name, '_small', NameSample ),
  Result = rel(NameSample, V, C),
  !.

query_sample(exactmatch(IndexName, R, V), Result) :-
  atom_concat(IndexName,'_small', IndexNameSmall),
  query_sample( R, R2 ),
  Result = exactmatch(IndexNameSmall, R2, V),
  !.

/*
To be modified - it should handle functors with any number of arguments. Currently it handles only from 1 to 5 arguments. It should handle lists, too.
*/

query_sample( Term, Result ) :-
  functor(Term, Fun, 1 ),
  arg(1, Term, Arg1),
  query_sample(Arg1, Res1),
  Result =.. [Fun | [Res1]],
  !.

query_sample( Term, Result ) :-
  functor(Term, Fun, 2 ),
  arg(1, Term, Arg1),
  arg(2, Term, Arg2),
  query_sample(Arg1, Res1),
  query_sample(Arg2, Res2),
  Result =.. [Fun | [Res1, Res2]],
  !.

query_sample( Term, Result ) :-
  functor(Term, Fun, 3 ),
  arg(1, Term, Arg1),
  arg(2, Term, Arg2),
  arg(3, Term, Arg3),
  query_sample(Arg1, Res1),
  query_sample(Arg2, Res2),
  query_sample(Arg3, Res3),
  Result =.. [Fun | [Res1, Res2, Res3]],
  !.

query_sample( Term, Result ) :-
  functor(Term, Fun, 4 ),
  arg(1, Term, Arg1),
  arg(2, Term, Arg2),
  arg(3, Term, Arg3),
  arg(4, Term, Arg4),
  query_sample(Arg1, Res1),
  query_sample(Arg2, Res2),
  query_sample(Arg3, Res3),
  query_sample(Arg4, Res4),
  Result =.. [Fun | [Res1, Res2, Res3, Res4]],
  !.

query_sample( Term, Result ) :-
  functor(Term, Fun, 5 ),
  arg(1, Term, Arg1),
  arg(2, Term, Arg2),
  arg(3, Term, Arg3),
  arg(4, Term, Arg4),
  arg(5, Term, Arg5),
  query_sample(Arg1, Res1),
  query_sample(Arg2, Res2),
  query_sample(Arg3, Res3),
  query_sample(Arg4, Res4),
  query_sample(Arg5, Res5),
  Result =.. [Fun | [Res1, Res2, Res3, Res4, Res5]],
  !.

query_sample( Term, Result ) :-
  Result = Term,
  !.

optimize_sample( Query ) :-
  deleteNewResultSize,
  deleteSampleSelectivity,
  callLookup(Query, Query2),
  queryToPlan(Query2, Plan, Cost),
  plan_to_atom(Plan, FirstQuery),
  write( 'First Plan:' ), nl, write( FirstQuery ), nl, nl,
  query_sample(Plan, Plan2),
  plan_to_atom(Plan2, SampleQuery),
  write('The plan in sample is: '), nl,
  write(SampleQuery), nl, nl,
  write('Executing the query in the sample...'),
  query(SampleQuery), nl, nl,
  createNewResultSizes,
  createSampleSelectivity,
  write( 'First Plan:' ), nl, write( FirstQuery ), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  optimize(Query).

optimize_sample( Query, FirstQuery, SecondQuery ) :-
  deleteNewResultSize,
  deleteSampleSelectivity,
  callLookup(Query, Query2),
  queryToPlan(Query2, Plan, Cost),
  plan_to_atom(Plan, FirstQuery),
  write( 'First Plan:' ), nl, write( FirstQuery ), nl, nl,
  query_sample(Plan, Plan2),
  plan_to_atom(Plan2, SampleQuery),
  write('The plan in sample is: '), nl,
  write(SampleQuery), nl, nl,
  write('Executing the query in the sample...'),
  query(SampleQuery), nl, nl,
  createNewResultSizes,
  createSampleSelectivity,
  write( '*** First Plan:' ), nl, write( FirstQuery ), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  optimize(Query,SecondQuery,_),
  nl, write( '*** Second Plan:' ), nl, write( SecondQuery ), nl, nl.

compare_plan( Query ) :-
  optimize_sample( Query, FirstQuery, SecondQuery ), nl,
  query( FirstQuery ), nl,
  query( SecondQuery ).

compare_sel :-
  storedSel( Pr, Sel1 ), sample_sel( Pr, Sel2 ),
  Change is 100*((Sel2 / Sel1)-1),
  write( Pr ), nl, write( Sel1 ), write( ' -> ' ), write( Sel2 ),
  write( ' (' ), write( Change ), write( '%)' ), nl,nl,
  fail.


