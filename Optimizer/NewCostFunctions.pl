/*
8 Computing Edge Costs for Plan Edges

8.1 The Costs of Terms

----
    cost(+Term, +Sel, +Pred,
        -ResAttrList, -ResTupleSize, -ResCard, -Cost)
----

Calculates the expected ~Cost~ of an executable ~Term~ representing a
predicate
 ~Pred~ with selectivity ~Sel~.
Also returns the list of available attributes ~ResAttrList~ with all
available
information on attribute names and sizes. ~ResAttrList~ has format
----
[[AttrName, AttrType, sizeTerm(CoreSize,IntFLOBSize,ExtFLOBSize)], [...]]

----

with one inner list for each attribute.

This is evaluated recursively descending into the term. When the operator
realizing the predicate (e.g. ~filter~) is encountered, the selectivity
~Sel~ is
used to determine the size of the result.

If more than a single operator with a selectivity occurs within ~Term~, the
topmost call receives the total selectivity as an argument.

Information on attributes and tuple sizes for each node ~N~ of the POG
can be
retrieved by calling

---- getNodeInfo(+N, -ResAttrList, -ResTupleSize).
----

Operator-related constants used within the cost functions should be
stored in
facts

---- costConst(+OpName, +ConstName, -Value)
----


8.1.1 Arguments

*/

% the if-then-else-part  is just for error-detection --- FIXME!
cost(rel(Rel, X1_), Sel, PETCalc, PETExp,
        ResAttrList, ResTupleSize, ResCard, 0) :-
  dcName2internalName(RelDC,Rel),
  ( Rel = RelDC
    -> true
    ;  (
         write('ERROR:\tcost/8 failed due to non-dc relation name.'), nl,
         write('---> THIS SHOULD BE CORRECTED!'), nl,
         throw(error_Internal(optimizer_cost(rel(Rel, X1_), Sel, PETCalc,
            PETExp, ResAttrList, ResTupleSize, ResCard, 0)
              :malformedExpression))),
         fail
       )
  ),
  tupleSizeSplit(RelDC,ResTupleSize),
  getRelAttrList(RelDC, ResAttrList, ResTupleSize)
  card(Rel, ResCard).


cost(res(N), _, _, _, ResAttrList, ResTupleSize, ResCard, 0) :-
  resultSize(N, ResCard),
  getNodeInfo(N, ResAttrList, ResTupleSize).

/*
8.1.2 Operators

*/

cost(feed(X), Sel, S, C) :-
  cost(X, Sel, S, C1),
  feedTC(A),
  C is C1 + A * S.


/*
Here ~feedTC~ means ``feed tuple cost'', i.e., the cost per tuple, a constant to
be determined in experiments. These constants are kept in file ``operators.pl''.

*/


cost(feedproject(X, _), Sel, S, C) :-
  cost(X, Sel, S, C1),
  feedTC(A),
  C is C1 + A * S.



cost(consume(X), Sel, S, C) :-
  cost(X, Sel, S, C1),
  consumeTC(A),
  C is C1 + A * S.

/*
For ~filter~, there are several special cases to distinguish:

  1 ~filter(spatialjoin(...), P)~

  2 ~filter(gettuples(...), P)~

  3 ~filter(windowintersects(...), P)~

  4 ``normal'' ~filter(...)~

For the first three cases, the edge is the translation of a spatial predicate, that
makes use of bounding box checks. The first argument of filter will already reduce
the set of possible candidates, so that the cardinality of tuples processed by filter
will be smaller than the cardinality passed down in the 3rd argument of ~cost~. Also, the selectivity passed with the second argument of ~cost~ is the ~total~ selectivity. To
get the selectivity of the preselection, one can analyse the predicate and lookup
the table ~storedBBoxSel/3~ for that selectivity, which should be passed to the recursive call of ~cost~.

PROBLEM: What happens with the entropy-optimizer? As for cases 2 + 3, there is no
problem, as the index is used to create a fresh tuple stream. But, as for case 1, we
might get into problems, as the selectivity of the bbox-check depends on earlier
predicates - so we should consider both selectivities in the minimization of the entropy.

*/

cost(filter(X, _), Sel, S, C) :-
  isPrefilter(X),     % holds for spatialjoin or loopjoin
          % isPrefilter defindad after cost clauses.

%  X = spatialjoin(_, _, attrname(attr(Attr1, ArgNr1, Case1)),
%                        attrname(attr(Attr2, ArgNr2, Case2))),
%  getSimplePred(P, PSimple),
%  databaseName(DB),
%  storedBBoxSel(DB, PSimple, BBoxSel),
%  cost(X, BBoxSel, SizeX, CostX),

  cost(X, Sel, SizeX, CostX),
  filterTC(A),
  S is SizeX,
  C is CostX + A * SizeX, !.

cost(filter(gettuples(rdup(sort(
      windowintersectsS(dbobject(IndexName), BBox))), rel(RelName, *)),
      FilterPred), Sel, Size, Cost):-
  dm(costFunctions,['cost(filter(gettuples(rdup(sort(windowintersectsS(...): ',
                    'IndexName= ',IndexName,', BBox=',BBox,
                    ', FilterPred=',FilterPred]),
  Cost is 0,
  card(RelName, RelCard),
  Size is RelCard * Sel,!.
%   write('...Inside cost estimation '),nl,
%   card(RelName, RelCard),
%   write('...Inside cost estimation1 '),nl,
%   concat_atom(['query no_entries(', IndexName, ') '], '', Command),
%   write('...Inside cost estimation2 '- Command),nl,
%   secondo(Command, [_,IndexCard]),
%   write('...IndexCard' - IndexCard),nl,
%   windowintersectsTC(WITC),
%   write('...Inside cost estimation3 '),nl,
%   CostWI is Sel * 1.2 * IndexCard * WITC * 0.25,   % including 20% false positives
%   write('...Inside cost estimation4 '),nl,
%   sorttidTC(STC),
%   write('...Inside cost estimation5 '),nl,
%   CostSort is Sel * 1.2 * IndexCard * STC,
%   write('...Inside cost estimation6 '),nl,
%   rdupTC(RDTC),
%   write('...Inside cost estimation7 '),nl,
%   CostRD is Sel * 1.2 * IndexCard * RDTC,
%   write('...Inside cost estimation8 '),nl,
%   CostGT is Sel * 1.2 * WITC * 0.75,
%   Cost is CostWI+ CostSort + CostRD + CostGT,
%   write('...Total cost is ' - Cost),nl,
%   Size is Sel * RelCard,
%   write('...Final size is ' - Size),nl.




cost(filter(X, _), Sel, S, C) :-  % 'normal' filter
  cost(X, 1, SizeX, CostX),
  filterTC(A),
  S is SizeX * Sel,
  C is CostX + A * SizeX.
  %C is CostX.

cost(product(X, Y), _, S, C) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, 1, SizeY, CostY),
  productTC(A, B),
  S is SizeX * SizeY,
  C is CostX + CostY + SizeY * B + S * A.

cost(leftrange(_, Rel, _), Sel, Size, Cost) :-
  cost(Rel, 1, RelSize, _),
  leftrangeTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

cost(rightrange(_, Rel, _), Sel, Size, Cost) :-
  cost(Rel, 1, RelSize, _),
  leftrangeTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

/*

Simplistic cost estimation for loop joins.

If attribute values are assumed independent, then the selectivity
of a subquery appearing in an index join equals the overall
join selectivity. Therefore it is possible to estimate
the result size and cost of a subquery
(i.e. ~exactmatch~ and ~exactmatchfun~). As a subquery in an
index join is executed as often as a tuple from the left
input stream arrives, it is also possible to estimate the
overall index join cost.

*/
cost(exactmatchfun(_, Rel, _), Sel, Size, Cost) :-
  cost(Rel, 1, RelSize, _),
  exactmatchTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

cost(exactmatch(_, Rel, _), Sel, Size, Cost) :-
  cost(Rel, 1, RelSize, _),
  exactmatchTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

cost(loopjoin(X, Y), Sel, S, Cost) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, Sel, SizeY, CostY),
  S is SizeX * SizeY,
  loopjoinTC(C),
  Cost is C * SizeX + CostX + SizeX * CostY.

cost(fun(_, X), Sel, Size, Cost) :-
  cost(X, Sel, Size, Cost).



/*

Previously the cost function for ~hashjoin~ contained a term

----    A * SizeX + A * SizeY
----

which should account for the cost of distributing tuples
into the buckets. However in experiments the cost of
hashing was always ten or more times smaller than the cost
of computing products of buckets. Therefore that term
was considered unnecessary.

*/
cost(hashjoin(X, Y, _, _, NBuckets), Sel, S, C) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, 1, SizeY, CostY),
  hashjoinTC(A, B),
  S is SizeX * SizeY * Sel,
  %showValue('SizeX', SizeX),
  %showValue('SizeY', SizeY),
  %showValue('CostX', CostX),
  %showValue('CostY', CostY),
  H is                                          % producing the arguments
    A * NBuckets * (SizeX/NBuckets + 1) *       % computing the product for each
      (SizeY/NBuckets +1) +                     % pair of buckets
    B * S,
  %showValue('Hashcost', H),
  C is CostX + CostY + H.                       % producing the result tuples


cost(sort(X), Sel, S, C) :-
  cost(X, Sel, SizeX, CostX),
  sortmergejoinTC(A, _),
  S is SizeX,
  C is CostX +                                  % producing the argument
    A * SizeX * log(SizeX + 1).                 % sorting the arguments
             %   individual cost of ordering predicate still not applied!


% Sortby with empty sorting list is ignored:
cost(sortby(X, []), Sel, S, C) :-
  cost(X, Sel, S, C).

cost(sortby(X, Y), Sel, S, C) :-
  Y \= [],
  cost(sort(X), Sel, S, C).

cost(mergejoin(X, Y, _, _), Sel, S, C) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, 1, SizeY, CostY),
  sortmergejoinTC(_, B),
  S is SizeX * SizeY * Sel,
  C is CostX + CostY +                        % producing the arguments
    B * S.                                    % parallel scan of sorted relations

cost(sortmergejoin(X, Y, AX, AY), Sel, S, C) :-
  cost(mergejoin(sortby(X, [AX]),sortby(Y, [AY]), AX, AY), Sel, S, C).


% two rules used by the 'interesting orders extension':
cost(sortLeftThenMergejoin(X, Y, AX, AY), Sel, S, C) :-
  cost(mergejoin(sortby(X, [AX]), Y, AX, AY), Sel, S, C).

cost(sortRightThenMergejoin(X, Y, AX, AY), Sel, S, C) :-
  cost(mergejoin(X, sortby(Y, [AY]), AX, AY), Sel, S, C).



/*
   Simple cost estimation for ~symmjoin~

*/
cost(symmjoin(X, Y, _), Sel, S, C) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, 1, SizeY, CostY),
  symmjoinTC(A, B),                     % fetch relative costs
  S is SizeX * SizeY * Sel,             % calculate size of result
  C is CostX + CostY +                  % cost to produce the arguments
    A * (SizeX * SizeY) +               % cost to handle buffers and collision
    B * S.                              % cost to produce result tuples

cost(spatialjoin(X, Y, _, _), Sel, S, C) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, 1, SizeY, CostY),
  spatialjoinTC(A, B),
  S is SizeX * SizeY * Sel,
  C is CostX + CostY +
  A * (SizeX + SizeY) +                 % effort is essentially proportional to the
          % sizes of argument streams
  B * S.                                % cost to produce result tuples


/*
costs for pjoin2 will only be used if option ~adpativeJoin~ is enabled.

*/

cost(pjoin2(X, Y, [ _ | _ ]), Sel, Size, C) :-
  cost(X, 1, SizeX, _),
  cost(Y, 1, SizeY, _),
  Size is Sel * SizeX * SizeY,
  cost(sortmergejoin(X, Y, _, _), Sel, S1, C1),
  cost(hashjoin(X, Y, _, _, 99997), Sel, S1, C2),
  C is min(C1, C2).

cost(pjoin2_hj(X, Y, [ _ | _ ]), Sel, Size, C) :-
  cost(hashjoin(X, Y, _, _, 99997), Sel, Size, C).

cost(pjoin2_smj(X, Y, [ _ | _ ]), Sel, Size, C) :-
  cost(hashjoin(X, Y, _, _, 99997), Sel, Size, C).

cost(extend(X, _), Sel, S, C) :-
  cost(X, Sel, S, C1),
  extendTC(A),
  C is C1 + A * S.

cost(remove(X, _), Sel, S, C) :-
  cost(X, Sel, S, C1),
  removeTC(A),
  C is C1 + A * S.

cost(project(X, _), Sel, S, C) :-
  cost(X, Sel, S, C1),
  projectTC(A),
  C is C1 + A * S.

cost(rename(X, _), Sel, S, C) :-
  cost(X, Sel, S, C1),
  renameTC(A),
  C is C1 + A * S.

% Xris: Added, costfactors not verified
cost(rdup(X), Sel, S, C) :-
  cost(X, Sel, S, C1),
  sortmergejoinTC(A, _),
  C is C1 + A * S.



%fapra1590
cost(windowintersects(_, Rel, _), Sel, Size, Cost) :-
  cost(Rel, 1, RelSize, _),
  windowintersectsTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

% Cost function copied from windowintersects
% May be wrong, but as it is usually used together
% with 'gettuples', the total cost should be OK
cost(windowintersectsS(dbobject(IndexName), _), Sel, Size, Cost) :-
  % get relationName Rel from Index
  concat_atom([RelName|_],'_',IndexName),
  dcName2internalName(RelDC,RelName),
  Rel = rel(RelDC, *),
  cost(Rel, 1, RelSize, _),
  windowintersectsTC(C),
  Size is Sel * RelSize,  % bad estimation, may contain additional dublicates
  Cost is Sel * RelSize * C * 0.25. % other 0.75 applied in 'gettuples'

cost(gettuples(X, _), Sel, Size, Cost) :-
  cost(X, Sel, Size, CostX),
  windowintersectsTC(C),
  Cost is   CostX            % expected to include cost of 'windowintersectsS'
          + Size * C * 0.75. % other 0.25 applied in 'windowintersectsS'



isPrefilter(X) :-
  X = spatialjoin(_, _, _, _).

isPrefilter(X) :-
  X = loopjoin(_, _).




/*
The following code fragment may be needed, when also the non-conjunctive
part of the query will be assigned with costs. At the moment, it is obsolete
and therefore commented out:

----

% Dummy-Costs for simple aggregation queries
cost(simpleAggrNoGroupby(_, Stream, _), Sel, Size, Cost) :-
  cost(Stream, Sel, Size, Cost).

cost(simpleUserAggrNoGroupby(Stream, _, _, _),
  Sel, Size, Cost) :- cost(Stream, Sel, Size, Cost).

----


*/
