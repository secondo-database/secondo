/*
//characters [1] formula: [$] [$]
//characters [2] program: [{\tt ] [}]
//[<=] [$\leq$]
//[>=] [$\geq$]

" "[1]

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
~calloptimizer.pl~. Finally, the code documentation was revised.

May 2006. M. Spiekermann. Creation and update mechanism for relation objects.
This may be useful for a deeper analysis of the quality of cardinality
estimations.

June 2006. Generic predicates for displaying lists of tuples as formatted tables
added. This helps to get an quick overview about the different estimation values.

*/


/*
1 Data Structures

*/

:- dynamic
   % to compute the size of intermediate results along a path through the POG.

   smallResultSize/2,
   smallResultCounter/4,
   
   % smallResultCounter(Nc, Source, Target, Result))
   
   entropy_node/2,
   small_cond_sel/4,
   firstResultSize/2,
   firstEdgeSelectivity/3,

   selfJoin/3,
   selfJoinToDo/2,
   buildingSmallPlan/0.
   

/*
2 Tools for Evaluation of the Entropy Approach

Below there are some clauses which print the estimated sizes of the standard optimizer
and compares them with the sizes calculated by the entropy approach. Moreover,
it is possible to compute the ~real~ nod sizes and to store the queries and
the estimations in SECONDO relation objects.


2.1 Construction of Single Tuples

*/


createCompareSizeTuple(Node,[ Node, FirstSize, EntropySize ]) :-
  firstResultSize(Node, FirstSize),
  resultSize(Node, EntropySize).

createCompareSelTuple(Source, Target, [ Source, Target, SelStd, SelEntropy ]) :-
  firstEdgeSelectivity(Source, Target, SelStd),
  edgeSelectivity(Source, Target, SelEntropy).

createNodeSize(Node, SizeStd, SizeEntropy, SizeReal) :-
  firstResultSize(Node, SizeStd),
  resultSize(Node, SizeEntropy),
  realResult(Node, SizeReal).


/*
2.3 User Interface for displaying Estimation Values 

The predicate ~compareEstimations/0~ displays values for node sizes
and selectivity estimations of the standard and entropy approach.

The predicate ~allSizes/0~ computes the real cardinlities of the
POG and creates or updates two relation objects (Nodesizes and Queries).
The tuples can be joined by attribute ~Qid~ and contain information about
the estimated and real sizes and some facts about the query. It uses the
predicate ~computeCards~ of the standard optimizer.

Predicate ~showAllSizes/0~ displays already computed values.

*/
 
compareSizes :-
 compareEstimations.

compareEstimations :-
 compareNodeSizes,
 compareEdgeSels. 

allSizes :-
  computeCards,
  showRel('NodeSizes').
 

/*
Collect all values stored in dynamic predicate into lists and pretty print
them as tables.

*/
 
compareNodeSizes :-
 findall(X, createCompareSizeTuple(_, X), L ),
 TupFormat = [ ['Node', 'c'], 
               ['SizeStd', 'l'], 
               ['SizeEntropy', 'l'] ],
 showTuples(L, TupFormat). 


compareEdgeSels :-
 findall(X, createCompareSelTuple(_, _, X), L ),
 Format = [ ['Source', 'c'], 
            ['Target', 'c'], 
            ['SelStd', 'l'], 
            ['SelEntropy', 'l'] ],
 showTuples(L, Format). 


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



----	compute_sel(ResSize, ArgSize, Sel) :-
----

Observed selectivities are determined according to the following rules.

  1 If the argument size is 0, then a selectivity 0 is assigned. The chain of dependent probablities constructed in ~ComputeJointProbabilities~ is terminated when a selectivity 0 is encountered. This means that the entropy optimizer will not get assumptions about the probabilities of nodes after this point.

  2 If the argument size is big enough, but the result size is zero, then we assume that one element has been found instead (similar to the other selectivity queries on samples).

  3 If the result size is equal to the argument size, we have a selectivity of one. Instead we assume a selectivity of 0.99, to avoid zero atoms.

  4 Otherwise the selectivity is ~ResSize~ / ~ArgSize~, as expected.

*/

compute_sel(_, N, 0) :-
  N = 0,
  !.

compute_sel(0, N, N1) :-
  N1 is 1 / N,
  !.

compute_sel(N, N, 0.99) :-
  !.

compute_sel(ResSize, ArgSize, Sel) :-
  Sel is ResSize / ArgSize.




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
3 Some clauses operating the ~small~ database.

prepare\_query\_small prepares the query to be executed in the small database.
Assumes that the small database has the same indexes that are in the full database,
but with the suffix '\_small'

*/
 
small(rel(Rel, Var, Case), rel(Rel2, Var, Case)) :-
  atom_concat(Rel, '_small', Rel2).



newResSize(arg(N), Size) :- 
  argument(N, R ), 
  small( R, rel(SRel, _, _)),   card(SRel, Size), !.

newResSize(res(N), Size) :- 
  smallResultSize(N, Size), !.



prepare_query_small( count(Term), count(Result) ) :-
  query_small(Term, Result), !.

prepare_query_small( Term, count(Result) ) :-
  query_small(Term, Result), !.



query_small([],[]) :- !.

query_small([First|Next], [FirstResult|NextResult]) :-
  query_small(First, FirstResult),
  query_small(Next, NextResult), !.

query_small(IndexName, NameSmall) :-
  storedIndex(_,_,_,_,IndexName),
  atom_concat( IndexName, '_small', NameSmall ), !.

query_small(rel(Name, V, C), rel(NameSmall, V, C)) :-
  atom_concat( Name, '_small', NameSmall ), !.

query_small( Term, Term ) :-
  atomic(Term), !.

query_small( Term, Result ) :-
  compound(Term),
  not(is_list(Term)),
  Term =.. [Op|Args],
  query_small( Args, ArgsResult ),
  Result =.. [Op|ArgsResult], !.



/*
3 Interaction with the ~standard~ Optimizer

At some places in optimizer.pl it will be checked
whether ~optimizerOption(entropy)~ holds. If this is true, ~clause2~ will be 
called instead of ~clause~.

*/


/*
The rule ~traversePath~ is used inside predicate ~plan/2~ of the
standard optimizer, but for the entropy approach the version below will
be called. In this case the values of the counters will be stored in 

----  nodePlan/2
      smallResultCounter/4
----      



----	traversePath2(+Path) :-
----

This is a variant of ~traversePath~ that embeds counters. Furthermore, it looks for self joins and for them inserts a self join correction operator ~reduce~.

*/

traversePath2([]).

traversePath2([costEdge(Source, Target, Term, Result, _, _) | Path]) :-
  embedSubPlans(Term, Term2),
  possiblyCorrectSelfJoins(Source, Target, Term2, Term3),
  nextCounter(nodeCtr, Nc),
  assert(nodePlan(Result, counter(Term3,Nc))),
  assert(smallResultCounter(Nc, Source, Target, Result)),
  traversePath2(Path).


/*
3.1 Dealing with Self Joins

This is done in the following steps:

  1 Self joins are discovered via ~registerSelfJoins~ hooked into the ~lookup~ predicate in optimizer.pl.

(docu to be completed).

*/

registerSelfJoins([], _).

registerSelfJoins([Pred | Preds], N) :-
  registerSelfJoin(Pred, N),
  N2 is N * 2,
  registerSelfJoins(Preds, N2).



registerSelfJoin(pr(_, Rel1, Rel2), N) :-
  Rel1 = rel(X, _, _),
  Rel2 = rel(X, _, _),
  assert(selfJoin(N, Rel1, Rel2)),
  assert(selfJoinToDo(Rel1, Rel2)),
  atom_concat('xxxID', X, IDAttrName),
  IDAttr = attr(IDAttrName, 100, l),	% 100 indicates hidden attribute,
					% to be used by projection translation
  assert(usedAttr(Rel1, IDAttr)),
  assert(usedAttr(Rel2, IDAttr)),
  nl, nl,
  write('*************'), nl,
  write('Predicate '), write(N), write(' has a self join on relation '), 
  write(X), nl, nl.

registerSelfJoin(_, _).




possiblyCorrectSelfJoins(Source, Target, TermIn, TermOut) :-
  buildingSmallPlan,
  PredNo is Target - Source,
  selfJoin(PredNo, Rel1, Rel2),
  selfJoinToDo(Rel1, Rel2),
  retractall(selfJoinToDo(Rel1, Rel2)),
  retractall(selfJoinToDo(Rel2, Rel1)),
  idAttr(Rel1, 1, Attr1),
  idAttr(Rel2, 2, Attr2),
  Rel1 = rel(RelName, _, _),
  atom_concat(RelName, '_small', SmallName),
  card(RelName, RelCard),
  card(SmallName, SmallCard),
  TermOut = reduce(TermIn, Attr1 = Attr2, RelCard div SmallCard).

possiblyCorrectSelfJoins(_, _, Term, Term).


idAttr(rel(RelName, *, _), ArgNo, attr(IdAttr, ArgNo, l)) :- !,
  atom_concat('xxxID', RelName, IdAttr).

idAttr(rel(RelName, Var, _), ArgNo, attr(Var:IdAttr, ArgNo, l)) :-
  atom_concat('xxxID', RelName, IdAttr).





/*
The clause ~translateEntropy~ runs a plan given as ~Stream1~ with ~Cost1~ on 
the small database. Conditional selectivities are computed along
the path of the first plan. Then the iterative scaling algorithm computes the
remaining conditional selectivites. The new selectivites are assigned to the
POG and a second plan is computed and returned as ~Stream2~ with costs ~Cost2~.

*/

translateEntropy(Stream1, Stream2, Cost1, Cost2) :-
  % limit application to queries with a maximum of 8 predicates:
  highNode(HN), HN > 1, HN < 256, !, 
  
  nl, 
  write('*** Using Entropy-approach ************' ), 
  nl, !,

  assert(removeHiddenAttributes),		% show first plan without
						% self join modifications
  plan_to_atom(Stream1, FirstQuery),
  retractall(removeHiddenAttributes),


  prepare_query_small(Stream1, PlanSmall),
  plan_to_atom(PlanSmall, SmallQuery),
  
  write('The plan on the small database is: '), nl, 
  write(SmallQuery), nl, nl,
  write('Executing the query in the small database...'),
  
  deleteEntropyNodes, !,
  query(SmallQuery, _), !, nl,
  
  write('First Plan:'), nl, 
  write( FirstQuery ), nl, nl,
  write('Estimated Cost: '), 
  write(Cost1), nl, nl, !,
 
  % compute a new plan based on the cost annotations of
  % the new selectivities computed by maximize_entropy 
  deleteCounters,
  retract(buildingSmallPlan),
  retractall(selfJoin(_, _, _)),
  retractall(usedAttr(_, _)),
  assert(removeHiddenAttributes),
  assignEntropyCost,
  bestPlan(Stream2, Cost2), !,

  warn_plan_changed(Stream1, Stream2),
  !.

translateEntropy(Stream1, Stream1, Cost1, Cost1) :-
  % Just clean up stored data and return the first plan with its estimated cost:
  deleteCounters,
  retract(buildingSmallPlan),
  retractall(selfJoin(_, _, _)),
  retractall(usedAttr(_, _)),
  retractall(removeHiddenAttributes),
  assert(removeHiddenAttributes).


assignEntropyCost :-
 
  createSmallResultSizes, !,
  createSmallSelectivity, !,
  
  createMarginalProbabilities( MP ),!,
  createJointProbabilities( JP ),!,
 
  % store selectivities of the first query and  delete the
  % cost annotations in the POG 
  saveFirstSizes,
  deleteSizes,
  deleteCostEdges,
  
  nl, 
  write('Marginal Sel.: MP ='), write( MP ), nl,
  write('Conditional Sel.: JP =') , write( JP ), nl, nl,
  
  feasible(MP, JP, MP2, JP2), !,

  % call the predicate implemented in C++ which computes the 
  % remaining conditional probabilities. 
  write('calling maximize_entropy with:'), nl,
  write( MP2 ), write(', ') , write( JP2 ), nl, nl,
  maximize_entropy(MP2, JP2, Result), !,
  
  createEntropyNode(Result),
  assignEntropySizes,
  createCostEdges.



/*

4 Predicate ~pl\_maximize\_entropy/3~ 

This predicate computes conditional probabilities using the
Maximum Entropy approach. It is done by means of the so called iterative
scaling algorithm which is implemented in C++. 
The implementation is in the file "iterative\_scaling.cpp".

Usage: 

----
    maximize_entropy( [p1 p2 p3 ... pn], [[1, cp1], [2, cp2] ...], R )
----  

The parameters are a list of marginal probabilites (e.g. the selectivites on the
sample relations) and a list of conditional probabilities (computed after running
an initial query plan on a small database). It assumed the same coding of
predicates using bits, as done in POG construction - that is, to the predicate
n, if the ith-bit is set to 1 then the ith-predicate is already evaluated.  

Each pi is the probability for predicate $2^i$ Each pair [n, cp] is the given
probability cp of joint predicates n using the ith-bit convention above.  The result
is in the form of a list of pairs [n, cp] also.

Example applications:

----
    maximize_entropy([[1, 0.5], [2, 0.5]], [], R).
    R = [[0, 1.0], [1, 0.5], [2, 0.5], [3, 0.25]]

    maximize_entropy([[1, 0.5], [2, 0.5], [4, 0.5]], [[3, 0.4], [6, 0.4]], R).
    R = [ [0, 1.0], [1, 0.5], [2, 0.5], [3, 0.4], 
          [4, 0.5], [5, 0.34], [6, 0.4], [7, 0.32] ]
----

In the first case nothing is known about conditional probabilities and the
result is just the product (the independence assumption). In the second case the
remaining unknown conditional selectivites are computed.

*/

warn_plan_changed(Plan1, Plan2) :-
  not(Plan1 = Plan2),
  nl,
  write( '************************************' ), nl,
  write( '* * *  INITIAL PLAN CHANGED !  * * *' ), nl,
  write( '************************************' ), nl.

warn_plan_changed(_,_).


/*
4 Clauses working with Selectivities

*/

entropySel( 0, Target, Sel ) :-
  entropy_node( Target, Sel ).

entropySel( Source, Target, Sel ) :-
  entropy_node( Source, P1 ),
  entropy_node( Target, P2 ),
  P1 > 0,
  Sel is P2 / P1.
 
/*
Construction of the input parameters (a lists of marginal probabilites and
conditional probabilities) for the ~maximize\_entropy/3~ predicate.

*/

createMarginalProbabilities( MP ) :-
  createMarginalProbability( 0, MP ).

createMarginalProbability( N, [[Pred, Sel]|T] ) :-
  edgeSelectivity(N, M, Sel),
  Pred is M - N,
  createMarginalProbability( M, T ).

createMarginalProbability( _, [] ).

/*
The chain of probabilities constructed in ~createJointProbability~ is terminated when a selectivity 0 is encountered. Selectivity 0 is put into small\_cond\_sel if the sample (intermediate result on the small database) had a size of less than 10).

*/

createJointProbabilities( JP ) :-
  createJointProbability( 0, 1, JP ).

createJointProbability( N0, AccSel, [[N0, N1, CP1]|T] ) :-
  small_cond_sel( N0, N1, _, Sel ),
  Sel > 0,
  CP1 is Sel * AccSel,
  createJointProbability( N1, CP1, T ).

createJointProbability( _, _, [] ).





/*
----	feasible(Marginal, Joint, Marginal2, Joint2) :-
----

~Marginal2~ and ~Joint2~ are adjusted versions of the marginal selectivities ~Marginal~ and the joint selectivities ~Joint~. Adjustments are done to avoid inconsistent settings and zero atoms, to ensure that iterative scaling finds a numeric solution. The basic conditions that need to hold for a conjunction of two predicates "p" and "q" with selectivities "S_p" and "S_q" and the joint selectivity "S_{pq}" are the following:

  1 "S_{pq}" [<=] "S_p"

  2 "S_{pq}" [<=] "S_q"

  3 "S_{pq}" [>=] "S_p + S_q - 1"

If any "S_{pq}" lies outside these bounds it will be set to the corresponding boundary value. To avoid zero atoms, additionally the value of "S_{pq}" is reduced or increased by 1 \%, respectively.

Marginals of value 1 can arise if all tuples in a sample qualify. In this case we also lower the selectivity slightly (to 0.99) to avoid zero atoms. 


*/

:- dynamic marginal/2.

saveMarginal([]).

saveMarginal([[Pred, 1]|L]) :-			% marginals must not be 1
  assert(marginal(Pred, 0.99)),
  saveMarginal(L).

saveMarginal([[Pred, Sel]|L]) :-
  assert(marginal(Pred, Sel)),
  saveMarginal(L).

loadMarginal(MP) :-
  setof([Pred, Sel], marginal(Pred, Sel), MP).

deleteMarginal :- 
  retractall(marginal(_, _)).


feasible(Marginal, Joint, Marginal2, Joint2) :-
  deleteMarginal,
  saveMarginal(Marginal),
  feasible2(Joint, Joint2),
  loadMarginal(Marginal2).

/*
----	feasible2(Joint, Joint2) :-
----

~Joint2~ is the adjusted version of the joint selectivities given in ~Joint~ in the form "[[Source, Target, TargetSel] | List]"[2]. The first ~Source~ is 0, hence the first entry in the list is that marginal selectivity measured on the small database. Since this value is likely to be more precise than the marginal value obtained from sampling, we use it to replace that marginal value. This has also the advantage that the remaining selectivities observed on the small database are consistent with this one.

*/

feasible2([], []).

feasible2([[0, Target, TargetSel] | Joint], Joint2) :-
  retract(marginal(Target, _)),
  assert(marginal(Target, TargetSel)),
  feasible3(TargetSel, Joint, Joint2).


feasible3(_, [], []).

feasible3(PrevJoint, [[Source, Target, TargetSel] | Joint], 
	[[Target, TargetSel2] | Joint2]) :-
  LastPred is Target - Source,
  marginal(LastPred, LastPredSel),
  %nl, write('call adjusted with '), 
  %write('PrevJoint = '), write(PrevJoint), write(', '), 
  %write('LastPredSel = '), write(LastPredSel), write(', '), 
  %write('TargetSel'), write(TargetSel), nl,
  adjusted(PrevJoint, LastPredSel, TargetSel, TargetSel2),
  feasible3(TargetSel2, Joint, Joint2).


adjusted(PSel, QSel, JointSel, JointSel) :-
  JointSel < 0.99 * PSel,
  JointSel < 0.99 * QSel,
  JointSel > 1.01 * (PSel + QSel - 1).

adjusted(PSel, QSel, JointSel, JointSel2) :-
  MinSel is min(PSel, QSel),
  JointSel >= 0.99 * MinSel,
  JointSel2 is 0.99 * MinSel.

adjusted(PSel, QSel, JointSel, JointSel2) :-
  JointSel =< 1.01 * (PSel + QSel - 1),
  JointSel2 is 1.01 * (PSel + QSel - 1).

  
simpleadjust(MP, [_ |JP] , MP2, JP2):-
  margadjust(MP, MP2),
  jointadjust(JP, JP2).

margadjust([], []).

margadjust([[_, Sel] | L], [Sel | L2]) :-
  margadjust(L, L2).

jointadjust([], []).

jointadjust([[_, Target, TargetSel] | L], [[Target, TargetSel] | L2]) :-
  jointadjust(L, L2).



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
6 Example Queries

----
   sql select count(*) from plz as p where [p:plz > 40000, p:plz < 50000].
   sql select count(*) from plz as p where [(p:plz mod 20) = 0, (p:plz mod 30) = 0].
----

*/
 


