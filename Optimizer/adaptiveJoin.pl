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

This file contains clauses which translate a plan computed by the
standardoptimizer into a plan using operators of the
~PartitionedStream-Algebra~. 

1 Overview

If the optimizer option ~adaptiveJoin~ is switched on, a join will be translated
into an adaptive join using the translation rules below:

----

join(Arg1, arg(N), pr(X=Y, _, _)) => pjoin1( Stream, Rel, Fields ) :-
  optimizerOption(adaptiveJoin),   
  try_pjoin1(N, X, Y, Rel, Fields),
  Arg1 => Stream.

join(arg(N), Arg2, pr(X=Y, _, _)) => pjoin1( Stream, Rel, Fields ) :-
  optimizerOption(adaptiveJoin),   
  try_pjoin1(N, X, Y, Rel, Fields),
  Arg2 => Stream.

join00(Arg1S, Arg2S, pr(X = Y, _, _)) => pjoin2(Arg1S, Arg2S, Fields) :-
  optimizerOption(adaptiveJoin), 
  try_pjoin2(X, Y, Fields).

----

Note: Currently, there is only a rule for translating an equi join. The terms 
implicitArg(1) and implicitArg(2) will be converted by plan\_to\_atom into '.'
and '..' respectively. 

*/

try_pjoin1(N, X, Y, Rel, Fields) :-
  isOfSecond(AttrRel, X, Y),
  isNotOfSecond(AttrStream, X, Y),
  createpjoin1(AttrStream, N, AttrRel, Rel, Fields).


try_pjoin1(N, X, Y, Rel, Fields) :-
  isOfFirst(attr(Name1, _, Case1), X, Y),
  isNotOfFirst(attr(Name2, _, Case2), X, Y),
  % we need to swap arguments here since pjoin assumes 
  % the relation to be th second argument!
  AttrStream = attr(Name2, 1, Case1),
  AttrRel = attr(Name1, 2, Case2),
  createpjoin1(AttrStream, N, AttrRel, Rel, Fields).


createpjoin1(AttrStream, N, AttrRel, Rel, [F1, F2, F3, F4]) :-   
  %write('AttrStream: '), writeln(AttrStream),
  %write('AttrRel: '), writeln(AttrRel),
  argument(N, Rel),
  writeln(Rel),
  Rel = rel(_, Var, _),
  hasIndex(Rel, AttrRel, IndexName, btree),
  %writeln('pjoin1-hasindex'),
  RelArg = rename(feed(implicitArg(2)), Var),
  F1 = field(attr(symj, _, l), symmjoin(implicitArg(1), RelArg, AttrStream =
  AttrRel)),
  F2 = field(attr(hj, _, l), hashjoin(implicitArg(1), RelArg,
  attrname(AttrStream),
  attrname(AttrRel), 997)),
  F3 = field(attr(smj, _, l), sortmergejoin(implicitArg(1), RelArg,
  attrname(AttrStream), attrname(AttrRel))),
  F4 = field(attr(ilj, _, l), loopjoin(implicitArg(1), rename(exactmatch(IndexName,
  implicitArg(2), AttrStream), Var))).


try_pjoin2(X, Y, [F1, F2, F3]) :- 
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y),
  F1 = field( attr(symj, _, l), 
              symmjoin(implicitArg(1), implicitArg(2), X = Y)),
  F2 = field( attr(hj, _, l), 
              hashjoin( implicitArg(1), implicitArg(2), 
                        attrname(Attr1), attrname(Attr2), 997)),
  F3 = field( attr(smj, _, l), 
              sortmergejoin( implicitArg(1), implicitArg(2), 
                             attrname(Attr1), attrname(Attr2))).



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
      
----

Obviously the main problems of the plan reorganization are

(1)  to identify tuple sources

(2)  to apply puse for a sequence of operations like
$filter(rename(project(..))))$ and to replace the inner argument with a '.' 


2 Implementation

The predicate ~makePstream/2~ will be the interface for the standard optimizer.
If option adaptiveJoin is set this predicate will be called instead of 
~plan/2~

*/

makePStream(Path, Plan) :-
  deleteNodePlans,
  traversePath(Path),
  highestNode(Path, N), 
  nodePlan(N, TmpPlan),
  nl, write('TmpPlan: '), write(TmpPlan), nl, nl,
  makePStreamRec(pdelete(TmpPlan), Plan, _), 
  nl, write('Plan: '), write(Plan), nl, nl.

/*
The predicate ~makePStreamRec/3~ translates a plan starting with
$pdelete(...)$ into a plan using appropriate ~puse~ and ~pfeed~ operations.

*/


btreeOp(exactmatch).
btreeOp(leftrange).
btreeOp(rightrange).

makePStreamRec(Plan, pdelete(PStream), _) :-
  Plan =.. [ pdelete | Arg ],
  makePStreamRec(Arg, Term, Source),
  %nl, write('Source: '), write(Source), nl,
  Term = [ Op1 | _ ],
  %nl, write('Op1: '), write(Op1), nl,
  constructPStream(Op1, Source, PStream).

/*
Note: a ~pjoin2~ or ~pjoin1~ operator may also be a tuple source for subsequent puse
operations, refer to example (3). Hence its translation will be also 
unified with ~Source~.

*/
 
makePStreamRec(pjoin2(Arg1, Arg2, Fields), Source, Source) :-
  makePStreamRec(Arg1, Arg1S, Source1),
  makePStreamRec(Arg2, Arg2S, Source2),
  constructPStream(Arg1S, Source1, PStream1), 
  constructPStream(Arg2S, Source2, PStream2),
  Source = pjoin2(PStream1, PStream2, Fields). 

makePStreamRec(pjoin1(Arg1, Arg2, Fields), Source, Source) :-
  makePStreamRec(Arg1, Arg1S, Source1),
  constructPStream(Arg1S, Source1, PStream1),
  Source = pjoin1(PStream1, Arg2, Fields). 

 
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
 % nl, write('Args2:'), write(Args2), nl,
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

*/
 
constructPStream(Arg, _, Arg) :-
  Arg =.. [ pjoin2 | _ ], !.
 
constructPStream(Arg, _, Arg) :-
  Arg =.. [ pjoin1 | _ ], !.
 
constructPStream(Arg, _, Arg) :-
  Arg =.. [ puse | _ ], !.
 
constructPStream(Arg1, Source, puse(Source, Arg2)) :-
  nonvar(Source),
  %nl, write('Source: '), write(Source), nl,
  translateArgs(Arg1, Source, Arg2).
  %nl, write('Translated Arg: '), write(Arg2), nl.

% should never be reached
constructPStream(_, _, _) :-
  nl, write('*** constructPStream: Error *** '), nl,
  fail.
 
/*
The predicate ~translateArgs/3~ recurses into the argument term
and replaces ~Source~ with ~implicitArg(1)~. 

*/

translateArgs(ArgsIn, Source, ArgsOut) :- 
  ArgsIn =.. [Functor | Args ],
  handleArgs(Args, Args2, Source),
  ArgsOut =.. [Functor | Args2 ].

handleArgs([Source | Args], [implicitArg(1) | Args], Source) :-
  compound(Source), !.

handleArgs([Arg | Args], [Arg | Args2], Source) :-
  not(compound(Arg)),
  handleArgs(Args, Args2, Source).
 
handleArgs([Arg | Args], [Arg2 | Args2], Source) :-
  compound(Arg), !,
  translateArgs(Arg, Source, Arg2),
  handleArgs(Args, Args2, Source).

handleArgs([], [], _).

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

