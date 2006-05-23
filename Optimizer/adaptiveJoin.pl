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


This file contains clauses which translate a plan computed by the
standardoptimizer into a plan using operators of the
~PartitionedStream-Algebra~. 

1 Overview

If the optimizer option ~adaptiveJoin~ is switched on, a join will be translated
into an adaptive join using the translation rules below:

----
    join00(S1,S2, pred(X = Y, _, _))
    
    =>
   
    pjoin2(S1, S2, [ field(attr(symj,_l), 
                     symmjoin(implicitArg(1), implicitArg(2), X = Y)),
                     field(attr(hj,_,l), 
                     hashjoin(implicitArg(1), implicitArg(2), 
                              attrname(Attr1), attrname(Attr2), 997)),
                     field(attr(smj,_,l), 
                     sortmergejoin(implicitArg(1), implicitArg(2), 
                                   attrname(Attr1), attrname(Attr2))) ])
----

Note: Currently, there is only a rule for translating an equi join. The terms 
implicitArg(1) and implicitArg(2) will be converted by plan\_to\_atom into '.'
and '..' respectively. 

The costs for this operator are defined as $min(hashjoin, sortmergejoin) - 1$,
hence the optimization algorithm will always choose this alternative. The
implementation of ~pjoin2~ does a probe join on the incoming streams and
estimates the input cardinalities and the output cardinalities. Based on these
parameters local cost functions (implemented inside the algebra module) are used
to choose the best alternative. In order to do this streams containing marker
tuples which contain information about the stream size need to be used. The
marker tuples can be created with the operator ~pfeed~. Furthermore the ordinary
operators of the relation algebra can be applied by using operator ~puse~ which
passes only normal tuples to its parameter function. 

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
  makePStreamRec(pdelete(TmpPlan), Plan, _). 
  %nl, write('Plan: '), write(Plan), nl, nl.

/*
The predicate ~makePStreamRec/3~ translates a plan starting with
$pdelete(...)$ into a plan using appropriate ~puse~ and ~pfeed~ operations.

*/

makePStreamRec(Plan, pdelete(PStream), _) :-
  Plan =.. [ pdelete | Arg ],
  makePStreamRec(Arg, Term, Source),
  %nl, write('Source: '), write(Source), nl,
  Term = [ Op1 | _ ],
  %nl, write('Op1: '), write(Op1), nl,
  constructPStream(Op1, Source, PStream).

/*
Note: a ~pjoin2~ operator may also be a tuple source for subsequent puse
operations, refer to example (3). Hence its translation will be also 
unified with ~Source~.

*/
 
makePStreamRec(pjoin2(Arg1, Arg2, Fields), Source, Source) :-
  makePStreamRec(Arg1, Arg1S, Source1),
  makePStreamRec(Arg2, Arg2S, Source2),
  constructPStream(Arg1S, Source1, PStream1), 
  constructPStream(Arg2S, Source2, PStream2),
  Source = pjoin2(PStream1, PStream2, Fields). 

/*
Terminate the recursion when a tuple source is recognized

*/
makePStreamRec(feed(Rel), pfeed(Rel,100), pfeed(Rel,100)).

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



