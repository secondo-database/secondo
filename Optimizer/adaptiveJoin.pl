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

May 23, 2006. M. Spiekermann. Inital version 

June 02, 2006. M. Spiekermann. Support for index selections added.

June 07, 2006. M. Spiekermann. Support for index loop-joins added.

June 09, 2006. M. Spiekermann. Corrections for correct handling of unrenamed relations.

July 2006. M. Spiekermann. Major changes for supporting ~pjoin1~ with new
semantics. It now combines join and selection predicates.

August 2006. M. Spiekermann. Integration of pjoin1 finished. Furthermore
non-equi joins like p1:plz = p2:plz+5 can now be rewritten in terms of the
partitioned stream algebra. 

This file contains clauses which translate a plan computed by the
standardoptimizer into a plan using operators of the
~PartitionedStream-Algebra~. 

1 Overview

If the optimizer option ~adaptiveJoin~ is switched on, a join will be translated
into an adaptive join using the translation rules below:

----

join00(Arg1S, Arg2S, pr(X = Y, _, _)) => pjoin2(Arg1S, Arg2S, Fields) :-
  optimizerOption(adaptiveJoin), 
  try_pjoin2(X, Y, Fields).

----

Note: Currently, there is only a rule for translating an equi join. The terms 
implicitArg(1) and implicitArg(2) will be converted by plan\_to\_atom into '.'
and '..' respectively. 

*/


matchExpr(Pred, Rel, Term, AttrRel, AttrStream) :-
  Pred = pr(X=Y, _, Rel),
  isOfSecond(AttrRel, X, Y),
  %showValue('AttrRel-1', AttrRel),
  isNotOfSecond(AttrStream, X, Y),
  %showValue('AttrStream-1', AttrStream),
  hasIndex(Rel, AttrRel, IndexName, btree),
  Term = exactmatch(IndexName, implicitArg(2), AttrStream).

matchExpr(Pred, Rel, Term, AttrRel2, AttrStream) :-
  Pred = pr(X=Y, Rel, _),
  isOfFirst(AttrRel, X, Y),
  AttrRel = attr(Name1, _, Case1),
  isNotOfFirst(attr(Name2, _, Case2), X, Y),
  %isNotOfFirst(AttrStream, X, Y),
  hasIndex(Rel, AttrRel, IndexName, btree),
  % we need to swap arguments here since pjoin assumes 
  % the relation to be the second argument!
  AttrStream = attr(Name2, 1, Case1),
  AttrRel2 = attr(Name1, 2, Case2),
  showValue('AttrRel-2', AttrRel2),
  showValue('AttrStream-2', AttrStream),
  Term = exactmatch(IndexName, implicitArg(2), AttrStream).

  
createpjoin1(AttrStream, AttrRel, FilterOps, ProbeOps, MatchOps, Ctr, [F1, F2, F3, F4]) :-   
 %showValue('AttrStream', AttrStream),
 %showValue('AttrRel', AttrRel),
 %showValue('FilterOps', FilterOps),
 %showValue('ProbeOps', ProbeOps),
 %showValue('MatchOps', ProbeOps),
  Ctr2 is Ctr + 1,
  Ctr3 is Ctr + 2,
  F1 = field( attr(symj, _, l), 
              symmjoin( implicitArg(1), 
                        counter(head(ProbeOps, 500),Ctr2),
                        AttrStream = AttrRel )),
  F2 = field( attr(hj, _, l), 
              hashjoin( implicitArg(1), 
                        counter(FilterOps,Ctr3),
                        attrname(AttrStream), 
                        attrname(AttrRel), 997) ),
  F3 = field( attr(smj, _, l), 
              sortmergejoin( implicitArg(1), 
                             counter(FilterOps,Ctr3),
                             attrname(AttrStream), attrname(AttrRel) )),
  F4 = field( attr(ilj, _, l), 
              loopjoin( implicitArg(1), 
                        counter(MatchOps,Ctr3) )).


try_pjoin2(X, Y, [F1, F2, F3]) :- 
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y),
  %not( possibleIndexJoin(Pred,_) ),
  %Pred = pr(X = Y, _, _),
  F1 = field( attr(symj, _, l), 
              symmjoin(implicitArg(1), implicitArg(2), X = Y)),
  F2 = field( attr(hj, _, l), 
              hashjoin( implicitArg(1), implicitArg(2), 
                        attrname(Attr1), attrname(Attr2), 997)),
  F3 = field( attr(smj, _, l), 
              sortmergejoin( implicitArg(1), implicitArg(2), 
                             attrname(Attr1), attrname(Attr2))).


try_pjoin2_hj(X, Y, [F1, F3]) :- 
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y),
  F1 = field( attr(symj, _, l), 
              symmjoin(implicitArg(1), implicitArg(2), X = Y)),
  F3 = field( attr(smj, _, l), 
              sortmergejoin( implicitArg(1), implicitArg(2), 
                             attrname(Attr1), attrname(Attr2))).


try_pjoin2_smj(X, Y, [F1, F2]) :- 
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y),
  F1 = field( attr(symj, _, l), 
              symmjoin(implicitArg(1), implicitArg(2), X = Y)),
  F2 = field( attr(hj, _, l), 
              hashjoin( implicitArg(1), implicitArg(2), 
                        attrname(Attr1), attrname(Attr2), 997)).



                            

/*

The costs for these operators are defined as $min(hashjoin, sortmergejoin) - 1$,
hence the optimization algorithm will always choose one of them. The
implementation of the ~pjoin~ operators does a probe join on the incoming tuples and
estimates the input cardinalities and the output cardinalities. Based on these
parameters local cost functions (implemented inside the algebra module) are used
to choose the best alternative. 

In order to suport cardinality estimation streams containing marker
tuples which contain information about the stream size are used. The
marker tuples can be created with the operator ~pfeed~. Furthermore the ordinary
operators of the relation algebra can be applied by using operator ~puse~ which
passes only normal tuples to its parameter function. For implementation details
refer to the algebra module ~PartitionedStream~.

Hence after the plan is computed we need to change the operations used for
creating and modifying tuple streams. Examples for some complete plans
are presented below:

----
    (1) sql select count(*) from [plz as a, plz as b] 
            where [a:plz = b:plz].

    query
      plz  pfeed[100]  puse[. project[PLZ] {a} ] 
      plz  pfeed[100]  puse[. project[PLZ] {b} ] 
      pjoin2[ symj: . .. symmjoin[(.PLZ_a = ..PLZ_b)], 
                hj: . .. hashjoin[PLZ_a, PLZ_b, 997], 
               smj: . .. sortmergejoin[PLZ_a, PLZ_b] 
      ]  
      pdelete  
      count

    (2) sql select count(*) from [plz as a, plz as b] 
            where [a:plz = b:plz, a:plz > 50000].

    query
      plz  pfeed[100]  puse[. project[PLZ] {a}  filter[(.PLZ_a > 50000)] ] 
      plz  pfeed[100]  puse[. project[PLZ] {b} ] 
      pjoin2[ symj: . .. symmjoin[(.PLZ_a = ..PLZ_b)], 
                hj: . .. hashjoin[PLZ_a, PLZ_b, 997], 
               smj: . .. sortmergejoin[PLZ_a, PLZ_b] 
      ]  
      pdelete  
      count

    (3) sql select count(*) from [plz as a, plz as b] 
            where [a:plz = b:plz, a:plz > 50000, a:ort = b:ort].

    query
      plz  pfeed[100]  puse[. project[Ort, PLZ] {a}  filter[(.PLZ_a > 50000)] ] 
      plz  pfeed[100]  puse[. project[Ort, PLZ] {b} ] 
      pjoin2[ symj: . .. symmjoin[(.PLZ_a = ..PLZ_b)], 
                hj: . .. hashjoin[PLZ_a, PLZ_b, 997], 
               smj: . .. sortmergejoin[PLZ_a, PLZ_b] 
      ]  
      puse[.  filter[(.Ort_a = .Ort_b)] ]  
      pdelete  
      count
 
   (4) sql select count(*) from [staedte as s, plz as p] 
           where [s:sname = p:ort]
    
    query 
      Staedte  pfeed[100]  puse[. project[SName] {s} ] 
      plz pjoin1[ symj: . ..  feed {p} symmjoin[(.SName_s = ..Ort_p)], 
                    hj: . ..  feed {p} hashjoin[SName_s, Ort_p, 997], 
                   smj: . ..  feed {p} sortmergejoin[SName_s, Ort_p], 
                   ilj: .  loopjoin[plz_Ort ..  exactmatch[.SName_s] {p} ] 
      ]  
      pdelete  count
     
    General term created for poin1: 
          
    query 
      Staedte  pfeed[100]  puse[. project[SName] {s} ] 
      plz pjoin1[ 
           symj: . .. feed head[1000] {c1} <exp1> <sel1> head[500] {c2} 
                      symmjoin[(.SName_s = ..Ort_p)], 
             hj: . .. feed <exp1> <sel1> {c3} hashjoin[SName_s, Ort_p, 997],  
             smj: . .. feed <exp1> <sel1> {c3} sortmergejoin[SName_s, Ort_p], 
            ilj: . loopjoin[plz_Ort .. exactmatch[.SName_s] <exp1> <sel1> {c3}]
      ]  
      pdelete  count

      with
           exp1 = project[attr1, ... attrM] {p}
           sel1 = filter[pr1] ...  filter[prN]


      
   (5) bug?: sql select s:sname from[plz as p, staedte as s ]w here p:plz=37263
    
       Such queries are not possible in the POG. We need to add a join predicate
       like join(TRUE, rel(plz), rel(staedte)) which will be translated into
       symmjoin[TRUE].
   
   (5a) sql select s:sname from[plz as p, staedte as s ]
                           where [p:plz=37263, p:ort=s:sname].
      
    (6) An example where an index join performs better than the 
        standard optimizer expects:
       
       sql select count(*) from [plz as p1, plz as p2, plz as p3] 
           where [p1:plz > 50000, p2:plz < 50200, p1:plz = p2:plz, p1:plz = p3:plz].


    (7) sql select count(*) from [staedte as s, plz as p] 
           where [s:sname = p:ort, p:plz < 50000]
    

A general enhancement of efficiency can be achieved if we project away no longer
required attributes. This is done by the term ~exp1~. Afterwards an aditonally
rename operator may be necessary. 

The expression ~sel1~ will do the selection on the right argument stream. During
the probe-join the tuples fulfilling pr1, ...  prN must be counted in order to
estimate the input cardinality. This will be done by the the function ~symj~
which reads in at most 1000 tuples from the input relation
and filters these tuples by the conditions specified in expression ~sel1~. The
remaining tuples will be counted by operator ~pcount~ which cancels processing
after 500 tuples have been passed over and stores the number of received tuples
in a global variable accessible for the pjoin1 implementation.  Hence it is
possible to estimate the input cardinalty and the join cardinality.
      
As a side effect this operator allows to apply selection predicates directly to
tuples retrieved by a btree-index. This may improve index-loop join applications
since plans generated by the standard optimizer can apply selections only after
the join.
    
----

Obviously the main problems of the plan reorganization are

(1)  to identify tuple sources

(2)  to apply operator ~puse~ for a sequence of operations like
$filter(rename(project(..))))$ and to replace the inner argument with a '.' 


2 Implementation

The predicate ~makePstream/2~ will be the interface for the standard optimizer.
If option adaptiveJoin is set this predicate will be called instead of 
~plan/2~

*/

makePStream(TmpPath, Plan) :-
  %showValue('TmpPath',TmpPath),
  nl, writeln('*** Rewrite Path ***'),
  rewritePath(TmpPath, Path),
  %showValue('Path', Path),
  deleteNodePlans,
  traversePath(Path),
  highestNode(Path, N), 
  nodePlan(N, TmpPlan),
  %showValue('TmpPlan', TmpPlan), nl, nl,
  nl, writeln('*** Rewrite Plan ***'),
  makePStreamRec(pdelete(TmpPlan), Plan, _). 
  %showValue('Plan', Plan), nl, nl.

rewritePath([H|T], [NewH|NewT]) :-
  %H = costEdge(Src, Tgt, Plan, Result, Size, Cost),
  rewriteCostEdge(H, NewH),
  rewritePath(T, NewT).

rewritePath([],[]).

rewriteCostEdge(E, E2) :-
  E = costEdge(Src, Tgt, Plan, Result, Size, Cost),
  Plan =.. [pjoin2 | Args],
  edge(Src, Tgt, join(_,_,Pred), _, _, _),
  possibleIndexJoin(Pred, _, LR ),
  Plan2 =.. [ pjoin1 | [LR | [Pred | Args]] ],
  E2 = costEdge(Src, Tgt, Plan2, Result, Size, Cost), !.

rewriteCostEdge(E, E).
  
 
/*
Store the join and selection predicates. For each join predicate we will check
if it is possible to use an index. This information will be used in the
translation rule for pjoin2 to avoid that a plan with pjoin2 will be computed
even if there are possible applications for pjoin1.

*/
 
:- dynamic storedPred/1.
:- dynamic possibleIndexJoin/3.
 
storePredicates(Preds) :-
  resetCounter(pjoin1, 1),
  retractall( storedPred(_) ),
  registerPreds(Preds),
  registerPossibleIndexJoins(_).

registerPreds([]).

registerPreds([H|T]) :-
  assert( storedPred(H) ),
  registerPreds(T).
 
getJoinPred(P) :-
  storedPred(P),
  P = pr(_,_,_).

checkForIndex(P) :-
  P = pr(X=Y, _, Rel),
  isOfSecond(Attr, X, Y),
  checkForIndex2(Rel, Attr, P, right).

checkForIndex(P) :-
  P = pr(X=Y, Rel, _),
  isOfFirst(Attr, X, Y),
  checkForIndex2(Rel, Attr, P, left).

checkForIndex(_). 

checkForIndex2(Rel, Attr, P, Arg) :-
  hasIndex(Rel, Attr, _, _),
  getCounter(pjoin1, Val),
  assert( possibleIndexJoin(P, Val, Arg) ),
  Val is Val + 3.

registerPossibleIndexJoins(L) :-
  nl, writeln('*** Register possible Index Joins ***'),
  retractall( possibleIndexJoin(_,_,_) ),
  findall(P, getJoinPred(P), L),
  checklist(checkForIndex, L),
  findall(P2, possibleIndexJoin(P2,_,_), L2),
  nl, showValue('Index Join Candidates',L2).
 
 
findJoinEdges2(Src1, Tgt1, Pred1) :-
  edge(Src1, Tgt1, join(_,_,Pred1), _, _, _).
  %edge(Src2, Tgt2, join(_,_,Pred2), _, _, _),
  %not([Src1, Tgt1] = [Src2, Tgt2]),
  %Pred1 = Pred2. 

findJoinEdges(L) :-
  findall([Src, Tgt, Pred], findJoinEdges2(Src, Tgt, Pred), L).
 
showJoinEdges :-
  findJoinEdges(L), 
  writeln(L).         

deleteRegularJoinEdges :-
  writeln('*** Modify Plan Edges ***'),
  %writePlanEdgesX,
  findJoinEdges(L),
  %showValue('POG join edges', L),
  removeJoinEdgesRec(L).

removeJoinEdgesRec([H|T]) :-
  H = [Src, Tgt, _],
  findall([Src, Tgt, Plan], planEdge(Src, Tgt, Plan, _), L ),
  handleEdgesRec(L),
  removeJoinEdgesRec(T).

removeJoinEdgesRec([]).


handleEdgesRec([H|T]) :-
  H = [Src, Tgt, Plan],
  handleEdge(Src, Tgt, Plan),
  handleEdgesRec(T).
 
handleEdgesRec([]). 

handleEdge(Src, Tgt, Plan) :-  
  not( allowedOp(Plan) ),
  delOp(Plan, F1, F2),
  write('deleting planEdge '), 
  write(Src), write('-'), write(Tgt), write(': '), 
  write(F1), write('('), write(F2), writeln(' ... )'), 
  retract( planEdge(Src, Tgt, Plan, _) ), !.

 
handleEdge(_, _, _).
  
allowedOp(Term) :-
  Term =.. [ remove | [ Arg1, _] ],
  Arg1 =.. [ F | _ ],
  myOp(F).
 
allowedOp(Term) :-
  Term =.. [ F | _ ],
  myOp(F).
 
myOp(pjoin2).
myOp(pjoin2_hj).
myOp(pjoin2_smj).


delOp(Term, F1, F2) :-
  Term =.. [ F1 | [ Arg1, _] ],
  Arg1 =.. [ F2 | _ ].

delOp(Term, F1, '') :-
  Term =.. [ F1 | _ ].
 
  
/*
The predicate ~makePStreamRec/3~ translates a plan starting with
$pdelete(...)$ into a plan using appropriate ~puse~ and ~pfeed~ operations.
This is done recursiveley for all arguments of each operator.

*/


btreeOp(exactmatch).
btreeOp(leftrange).
btreeOp(rightrange).

makePStreamRec(Plan, pdelete(PStream), _) :-
  Plan =.. [ pdelete | Arg ],
  %showValue('Arg: ', Arg),
  makePStreamRec(Arg, Term, Source),
  %showValue('Source: ', Source),
  Term = [ Op1 | _ ],
  %nl, write('Op1: '), write(Op1), nl,
  constructPStream(Op1, Source, implicitArg(1), PStream).

/*
Note: a ~pjoin2~ or ~pjoin1~ operator may also be a tuple source for subsequent
~puse~ operations, refer to example (3). Hence its translation will be also 
unified with ~Source~.

*/
 


makePStreamRec(pjoin2(Arg1, Arg2, Fields), Source, Source) :-
  makePStreamRec(Arg1, Arg1S, Source1),
  makePStreamRec(Arg2, Arg2S, Source2),
  constructPStream(Arg1S, Source1, implicitArg(1), PStream1), 
  constructPStream(Arg2S, Source2, implicitArg(1), PStream2),
  Source = pjoin2(PStream1, PStream2, Fields). 

makePStreamRec(pjoin2_hj(Arg1, Arg2, Fields), Source, Source) :-
  makePStreamRec(pjoin2(Arg1, Arg2, Fields), Source, Source).

makePStreamRec(pjoin2_smj(Arg1, Arg2, Fields), Source, Source) :-
  makePStreamRec(pjoin2(Arg1, Arg2, Fields), Source, Source).
 
makePStreamRec(pjoin1(right, Pred, Arg1, Arg2, _), Source, Source) :-
  makePStreamRec(Arg1, Arg1S, Source1),
  makePStreamRec(Arg2, Arg2S, Source2),
  constructPStream(Arg1S, Source1, implicitArg(1), PStream1),
  constructPStream(Arg2S, Source2, feed(implicitArg(2)), PStream2), 
  %writeln('Mark A'),
  possibleIndexJoin(Pred, Ctr, _),
  ProbeStream = counter(head(feed(implicitArg(2)),1000),Ctr),
  constructPStream(Arg2S, Source2, ProbeStream, PStream3),
  %writeln('Mark B'),
  %showValue('Pred: ', Pred),
  %showValue('MS: ', MatchStream),
  matchExpr(Pred, Rel, MatchStream, AttrRel, AttrStream),
  %writeln('Mark C'),
  %showValue('MatchStream', MatchStream),
  constructPStream(Arg2S, Source2, MatchStream, PStream4),
  %showValue('Arg2S', Arg2S),
  %showValue('PStream2', PStream2),
  %showValue('Source2', Source2),
  %showValue('Pred', Pred),
  PStream2 = puse(_, FilterOps),
  PStream3 = puse(_, ProbeOps),
  PStream4 = puse(_, MatchOps),
  createpjoin1(AttrStream, AttrRel, FilterOps, ProbeOps, MatchOps, Ctr, Fields),
  Source = pjoin1( PStream1, Rel, Ctr, Fields ).

makePStreamRec(pjoin1(left, Pred, Arg1, Arg2, _), Source, Source) :-
  Pred = pr(X=Y, RelA, RelB),
  makePStreamRec(pjoin1(right, pr(X=Y, RelA, RelB), Arg2, Arg1, _), Source, Source).

/*
Terminate the recursion when a tuple source is recognized

*/
makePStreamRec(feed(Rel), pfeed(Rel,100), pfeed(Rel,100)).


/*
Rewrite index selections into application of ~pcreate~

*/
makePStreamRec(Term, Result, Result) :-
  Term =.. [Functor | [_, rel(_, _, _), _] ],
  btreeOp(Functor),
  writeln('BTree operation on base relation detected!'),
  Result =.. [ pcreate | [Term, 100] ].
  
 
  
/*
Decompose a term into $[Functor | Args ]$ and make recursive calls for
the ~Args~.

*/

makePStreamRec(Term, Term2, Source) :-
 Term =.. [Functor | Args],
 %nl, write('Functor: '), write(Functor), nl,
 %nl, write('Args: '), write(Args), nl,
 makeArgs(Args, Args2, Source),
 %nl, write('Args2:'), write(Args2), nl,
 Term2 =.. [Functor | Args2].

% should never be reached
makePStreamRec(Term, Term, _) :-
  nl, write('*** makePStreamRec: Error ***'), nl,
  fail.

/*
Recursive calls for the functor's arguments.
If ~Arg~ is not a compound term we don't need to decompose 
it

*/

makeArgs([Arg | Args], [Arg | Args2], Source) :-
  not(compound(Arg)), !,
  makeArgs(Args, Args2, Source).
 
makeArgs([Arg | Args], [Arg2 | Args2], Source) :-
  compound(Arg), !,
  makePStreamRec(Arg, Arg2, Source),
  makeArgs(Args, Args2, Source).

makeArgs([], [], _).

 
/*
3 Application of Operator ~puse~

The clauses below will check if a given argument stream ~Arg~ is already
a valid $stream(ptuple(...))$. This is the case if the argument term has a functor 
pjoin or puse. If not, a ~puse~ operator needs to be applied. The predicate

----
    constructPStream(Arg, Source, Translation)
----

translates ~Arg~ into ~puse(Source, Arg2)~. The occurence of ~Source~ 
in Arg will be replaced by ~implicitArg(1)~. For example:

----
    rename(project(pfeed(rel(plz, a, l), 100), [attrname(attr(pLZ, 0, u))]), a)
---- 

will be replaced by

----
    puse( pfeed(rel(plz, a, l), 100), rename(project(implicitArg(1),
                                       [attrname(attr(pLZ, 0, u))]), a) )
----

If the argument is still an application of an operator which returns a stream
of ptuple nothing has to be done.

*/
 
constructPStream(Arg, _, _, Arg) :-
  Arg =.. [ pjoin2 | _ ], !.
 
constructPStream(Arg, _, _, Arg) :-
  Arg =.. [ pjoin1 | _ ], !.
 
constructPStream(Arg, _, _, Arg) :-
  Arg =.. [ puse | _ ], !.

constructPStream(Arg, _, _, Arg) :-
  Arg =.. [ pfeed | _ ], !.
 
constructPStream(Arg, _, _, Arg) :-
  Arg =.. [ pcreate | _ ], !.
 
constructPStream(Arg1, Source, NewSource, puse(Source, Arg2)) :-
  nonvar(Source),
  %nl, write('Source: '), write(Source), nl,
  translateArgs(Arg1, Source, NewSource, Arg2).
  %nl, write('Translated Arg: '), write(Arg2), nl.

% should never be reached
constructPStream(_, _, _, _) :-
  nl, write('*** constructPStream: Error *** '), nl,
  fail.
 
/*
The predicate ~translateArgs/3~ recurses into the argument term
and replaces ~Source~ with ~NewSource~. 

*/

translateArgs(ArgsIn, Source, NewSource, ArgsOut) :- 
  ArgsIn =.. [Functor | Args ],
  handleArgs(Args, Args2, Source, NewSource),
  ArgsOut =.. [Functor | Args2 ].

handleArgs([Source | Args], [NewSource | Args], Source, NewSource) :-
  compound(Source), !.

handleArgs([Arg | Args], [Arg | Args2], Source, NewSource) :-
  not(compound(Arg)),
  handleArgs(Args, Args2, Source, NewSource).
 
handleArgs([Arg | Args], [Arg2 | Args2], Source, NewSource) :-
  compound(Arg), !,
  translateArgs(Arg, Source, NewSource, Arg2),
  handleArgs(Args, Args2, NewSource, Source).

handleArgs([], [], _, _).

/*

4 Auxiliary Stuff

Observing cardinalities and estimations

*/

estimatedCards :-
 query 'SEC_COUNTERS 
          feed filter[(.CtrNr = SEC_COMMANDS count) 
                       and (.CtrStr contains "PSA::pjoin2") ] 
                       project[CtrStr, Value] consume;'.


/*
In order to organize probe joins on random samples we assume that base relations
of the database are not sorted according to one of their attribues. The predicate
~shuffleRels/0~ will guarantee this. All relations of the currently opened database
which have not the order status "shuffled" will be updated.

*/

checkRelName(DB, Rel) :-
  not(sub_atom(Rel, _, _, _, '_small')),
  not(sub_atom(Rel, _, _, _, '_sample_s')),
  not(sub_atom(Rel, _, _, _, '_sample_j')),
  not(sub_atom(Rel, _, _, _, 'SEC_')),
  not(hasStoredOrder(DB, Rel, shuffled)).
 
shuffleRels([]).

shuffleRels([Obj | ObjList]) :-
  Obj = ['OBJECT', Rel, _, [[rel, [_|_]]]],
  atom(Rel), 
  databaseName(DB),
  nonvar(DB),
  checkRelName(DB, Rel),
  atom_concat('update ', Rel, Plan1),
  atom_concat(Plan1, ' := ', Plan2 ),
  atom_concat(Plan2, Rel, Plan3 ),
  % MAX_INT (4 Byte) = 127 * 256 * 256 * 256 = 2130706432
  atom_concat(Plan3, ' feed extend[SortId: randint(2130706432)]', Plan4),
  atom_concat(Plan4, ' sortby[SortId asc] remove[SortId] consume', Plan5 ),
  nl, write('Plan:'), write(Plan5), nl,
  secondo(Plan5, _),
  changeStoredOrder(DB, Rel, shuffled),
  shuffleRels(ObjList),
  !.

shuffleRels([_ | ObjList]) :-
  shuffleRels(ObjList),
  !.

shuffleRels :-
  databaseName(DB),
  retractall(storedSecondoList(_)),
  getSecondoList(ObjList),
  write('\nRelations of database '), write(DB), write(' which need to be shuffled:\n'),
  shuffleRels(ObjList).



