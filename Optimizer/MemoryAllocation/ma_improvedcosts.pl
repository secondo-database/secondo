/*
$Header$
@author Nikolai van Kempen (just of the modifications)

This is a modified version of the improvedcosts.pl with difference that for
some operators the costs are obtained from the CostEstimation class.

This cost model was choosen because only this model uses milliseconds as costs, the standard costs and the nawra costs do not.

Note that this cost model is currently not working together with the nested relations.
*/


/*
8 Computing Edge Costs for Plan Edges

This extension requires, that ~optimizerOption(determinePredSig)~ is set.

8.1 The Costs of Terms

----

Implemented:
    cost(+Term,           +Sel, +Pred,
        ?ResAttrList, -ResTupleSize, -ResCard, -Cost)

Planned:
    cost(+Term, +POGnode, +Sel, +Pred,
        ?ResAttrList, -ResTupleSize, -ResCard, -Cost)
----

Calculates the expected ~Cost~ of an executable ~Term~ representing a
predicate ~Pred~ with selectivity ~Sel~.

The costedge starts from node ~POGnode~ within the predicate order graph.
This information can be used to retrieve and propagate information on attribute
types and sizes throughout the recursive evaluation of the cost-predicate.

While the result's tuple size is always propagated up, ~ResAttrList~ can be set to
~ignore~. In this case, the attribute list is not considered by the recursive calls.
Otherwise, ~cost/8~ returns the list of available attributes ~ResAttrList~ with all
available information on attribute names and sizes. ~ResAttrList~ has format

----
[[AttrName, AttrType, sizeTerm(MemSize, CoreSize, LOBSize)], [...]]

----

with one inner list for each attribute.

This is evaluated recursively descending into the term. When the operator
realizing a predicate (e.g. ~filter~) is encountered, the selectivity
~Sel~ is used to determine the size of the result.

If more than a single operator with a selectivity occurs within ~Term~, the
topmost call receives the total selectivity as an argument.

Information on attributes and tuple sizes for each node ~N~ of the POG
can be retrieved by calling

----
    getResTupleSize(+Node, -ResCardinality)
    getResAttrList(+Node, -ResAttributeList)
----

Operator-related constants used within the cost functions should be
stored in facts

---- costConst(+OpName, +ConstName, -Value)
----

Within the term,

---- optimizerAnnotation(+Plan, +Note)
----

can be used, to annotate a plan ~Plan~ with internal information ~Note~, during
plan creation and/or cost estimation.

When found within special operators, it may encode additional information.
So, it is used to pass down information into the evaluation of the sub plans.

If encountered alone, it is just ignored.

*/


/*
8.1.0 Plan Annotations

*/

% Section:Start:cost_7_b
% Section:End:cost_7_b

% ignore plan annotations:
% NVK NOTE: Why the costs here are not forwarded?!?
% I changed this to forward the costs.
cost(optimizerAnnotation(X,Y), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
  cost(X, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTermX),
	NewTerm=optimizerAnnotation(NewTermX, Y),
	!.

% Section:Start:cost_7_m
% Section:End:cost_7_m

/*
NVK ADDED MA
If the memory is already assigned in previous step (not implemented at the moment). Needed for path costs recomputation, there were serveral ways to do this, all with some disadvatages, i choosed this way because it is still flexible if have one time non-additive costs and it results in a solution were the fewest number of codelines are to copied.
	
The memory terms are under total control of the memory optimization extension, don't ever create memory terms within other parts because it might result in confusing the optimization process.
*/
cost(memory(X, MID, AList), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, 
		Cost, NewTerm) :-
	% Forward the memory term.
	MT=memory(_, MID, AList),
  cost(X, Sel, Pred, MT, ResAttrList, ResTupleSize, ResCard, Cost, NewTermX),
	NewTerm = memory(NewTermX, MID, AList),
	!.
%	NVK ADDED END

/*
8.1.1 Arguments

Arguments are either basic relations, or intermediate results.

*/


% the if-then-else-part  is just for error-detection --- FIXME!
cost(rel(Rel, X), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, 0, rel(Rel, X)) :-
  dcName2internalName(RelDC,Rel),
  ( Rel = RelDC
    -> true
    ;  (
         write('ERROR:\tcost/8 failed due to non-dc relation name.'), nl,
         write('---> THIS SHOULD BE CORRECTED!'), nl,
         throw(error_Internal(optimizer_cost(rel(Rel, X1_), Sel, Pred,
                               ResAttrList, ResTupleSize, ResCard, 0)
              ::malformedExpression)),
         fail
       )
  ),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true ; getRelAttrList(RelDC, ResAttrList, _/*ResTupleSize*/)
  ),
  tupleSizeSplit(RelDC,ResTupleSize),
  card(Rel, ResCard),
	!.

/*
NVK ADDED NR 
Added because the dcName2internalName evaluation of the below predicate fails for non atomic values. Note that if other cost models should support the nested relations, this cost predicates must be added first.
*/
cost(rel(RT, X), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, 0, rel(RT, X)) :-
  optimizerOption(nestedRelations),
	RT=arel(RelVar:ARelDC, FQN, ModAttr, RName),
	!,
	% XXX still to eval if this works
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true ; getRelAttrList(FQN, ResAttrList, _/*ResTupleSize*/)
  ),
  tupleSizeSplitByFQN(FQN, ResTupleSize),
  card(RT, ResCard).
% NVK ADDED NR END

cost(res(N), _, _, _, ResAttrList, ResTupleSize, ResCard, 0, res(N)) :-
  resultSize(N, ResCard),
  getResTupleSize(N,ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true ;  getResAttrList(N, ResAttrList)
  ),!.

/*
8.1.2 Operators

*/

/*
NVK ADDED NR
I won't analyze the costs of the afeed operator, but to make this improved costs version work with nested relations, i add here the same cost estimation as for the feed operator.
*/
cost(afeed(X), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
  cost(X, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost1, NewTermX),
  costConst(feed, pertuple, U),
  costConst(feed, perbyte, V),
  ResTupleSize = sizeTerm(_ /*MemSize*/, Core, _/*LOB*/),
  Cost is Cost1 + ResCard * ( U + Core * V ),
	NewTerm=afeed(NewTermX),
	!.
% NVK ADDED END

% can handle rel and tconsume(...)
cost(feed(X), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, feed(NewTermX)) :-
  cost(X, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost1, NewTermX),
  costConst(feed, pertuple, U),
  costConst(feed, perbyte, V),
  ResTupleSize = sizeTerm(_ /*MemSize*/, Core, _/*LOB*/),
  Cost is Cost1 + ResCard * ( U + Core * V ),!.

cost(feedproject(X, ProjAttrFields), Sel, Pred, _, ResAttrList, ResTupleSize,
		ResCard, Cost, NewTerm) :-
  cost(X, Sel, Pred, _, ResAttrList1, _/*ResTupleSize1*/, ResCard, Cost1, NewTermX),
  costConst(feedproject, msPerTuple, U),
  costConst(feedproject, msPerByte, V),
  costConst(feedproject, msPerAttr, W),
  findall(AttrName,
         memberchk(attr(AttrName,_,_),ProjAttrFields),
         ProjAttrNames),
  projectAttrList(ResAttrList1, ProjAttrNames, ResAttrList2, ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  ResAttrList = ResAttrList2
  ),
  ResTupleSize = sizeTerm(_/*MemSize*/, Core, _/*LOB*/),
  length(ProjAttrNames,NoAttrs),
  Cost is   Cost1
          + ResCard * (   U
                        + Core * V
                        + NoAttrs * W),
	NewTerm=feedproject(NewTermX, ProjAttrFields),
	!.

cost(consume(X), Sel, Pred, _, ResAttrList,
                ResTupleSize, ResCard, Cost, consume(NewTermX)) :-
  cost(X, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost1, NewTermX),
  costConst(consume, msPerTuple, U),
  costConst(consume, msPerByteCore, V),
  costConst(consume, msPerByteLOB, W),
  ResTupleSize = sizeTerm(MemSize, Core, LOB),
  Cost is   Cost1
          + ResCard * (   U
                        + V * max(MemSize, Core)
                        + W * LOB),!.

cost(tconsume(X), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
  cost(X, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost1, NewTermX),
  ResCard = sizeTerm(Mem,Core,Lob),
  TRelSize is (max(Mem,Core) + Lob) * ResCard,
  costConst(general, maxMemBytePerOperator, MaxMem),
  ( TRelSize > MaxMem  % does not fit into  in-memory-tuple-buffer
    -> cost(consume(X), Sel, Pred, ResAttrList, ResTupleSize, ResCard, Cost)
    ;  Cost = Cost1
  ),
	NewTerm=tconsume(NewTermX),
	!.

/*
For ~filter~, there are several special cases to distinguish:

  1 ~filter(spatialjoin(...), P)~

  2 ~filter(gettuples(...), P)~

  3 ~filter(windowintersects(...), P)~

  4 ``normal'' ~filter(...)~

For the first three cases, the edge is the translation of a spatial predicate,
that makes use of bounding box checks. The first argument of filter will already
reduce the set of possible candidates, so that the cardinality of tuples
processed by filter will be smaller than the cardinality passed down in the 3rd
argument of ~cost~. Also, the selectivity passed with the second argument of
~cost~ is the ~total~ selectivity. To get the selectivity of the preselection,
one can analyse the predicate and lookup the BBox-Selectivity calling
~getBBoxSel/2~ for that predicate, which should be passed to the recursive call
of ~cost~.

PROBLEM: What happens with the entropy-optimizer? As for cases 2 + 3, there is
no problem, as the index is used to create a fresh tuple stream. But, as for
case 1, we might get into problems, as the selectivity of the bbox-check depends
on earlier predicates - so we should consider both selectivities in the
minimization of the entropy.

*/

% rule for filter with pre-filtering
cost(filter(X, Y), Sel, Pred, _, ResAttrList,
                ResTupleSize, ResCard, Cost, filter(NewTermX, Y)) :-
  isPrefilter(X),     % holds for spatialjoin or loopjoin
  % the prefilter has already reduced the cardinality of candidates
  selectivity(Pred, _, BBoxSel, CalcPET, ExpPET),
  ( BBoxSel > 0
    -> RefinementSel is Sel/BBoxSel
    ;  RefinementSel is 1              % if BBoxSel = 0 then ResCard1 is also 0
  ),
  cost(X, BBoxSel, Pred, _, ResAttrList, ResTupleSize, ResCard1, Cost1, NewTermX),
  costConst(filter, msPerTuple, U),    % ms per tuple
  ResCard is ResCard1 * RefinementSel,
  Cost is   Cost1
          + ResCard1 * (U + max(CalcPET,ExpPET)),!.

% rule for filter with spatio-temporal pattern
cost(Term, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, Term) :-
	Term=filter(gettuples(rdup(sort(windowintersectsS(dbobject(IndexName), BBox))),
  NewTerm=tconsume(NewTermX), rel(RelName, A1)), FilterPred),
  dm(costFunctions,['cost(filter(gettuples(rdup(sort(windowintersectsS(...): ',
                    'IndexName= ',IndexName,', BBox=',BBox,
                    ', FilterPred=',FilterPred]),
  dcName2internalName(RelDC,RelName),
  ( RelName = RelDC
    -> true
    ;  (
         write('ERROR:\tcost/8 failed due to non-dc relation name.'), nl,
         write('---> THIS SHOULD BE CORRECTED!'), nl,
         throw(error_Internal(optimizer_cost(filter(gettuples(rdup(sort(
                      windowintersectsS(dbobject(IndexName), BBox))),
                      rel(RelName, _)), FilterPred), Sel, Pred,
                               ResAttrList, ResTupleSize, ResCard, Cost)
              ::malformedExpression)),
         fail
       )
  ),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  getRelAttrList(RelDC, ResAttrList, _/*ResTupleSize*/)
  ),
  tupleSizeSplit(RelDC,ResTupleSize),
  Cost is 0,
  card(RelName, RelCard),
  ResCard is RelCard * Sel, !.
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

% rule for standard filter
cost(filter(X, Y), Sel, Pred, _, ResAttrList,
                ResTupleSize, ResCard, Cost, filter(NewTermX, Y)) :- % 'normal' filter
  cost(X, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard1, Cost1, NewTermX),
  costConst(filter, msPerTuple, U),    % ms per tuple
  getPET(Pred,CalcPET,ExpPET),
  ResCard is ResCard1 * Sel,
  Cost is   Cost1
          + ResCard1 * (U + max(CalcPET,ExpPET)),!.


cost(product(X, Y), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, product(NewTermX, NewTermX)) :-
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> (ResAttrListX = ignore, ResAttrListY = ignore) ; true
  ),
  cost(X, Sel, Pred, _, ResAttrListX, ResTupleSizeX, ResCardX, CostX, NewTermX),
  cost(Y, Sel, Pred, _, ResAttrListY, ResTupleSizeY, ResCardY, CostY, NewTermY),
  costConst(general, maxMemBytePerOperator, MaxMem),
  costConst(product, msPerByteRightOnBufferOverflow, U),
  costConst(product, msPerByteTotalOutput, V),
  ResCard is ResCardX * ResCardY,
  addSizeTerms([ResTupleSizeX,ResTupleSizeY],ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  append(ResAttrListX,ResAttrListY,ResAttrList)
  ),
  ResTupleSizeY = sizeTerm(MemSizeY,_,_),
  getTotalDiskSize(ResTupleSize,TupleSizeRes),
  ( ResCardY * MemSizeY > MaxMem            % ToDo: does not consider LOB sizes
    -> ( getTotalDiskSize(ResTupleSizeY,TupleSizeY),
         BufferedDataY is ResCardY * TupleSizeY
       )
    ;  BufferedDataY is 0
  ),
  Cost is   CostX + CostY
          + BufferedDataY * U
          + ResCard  * TupleSizeRes * V,!.



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

cost(exactmatchfun(A1, Rel, A3), Sel, Pred, _, 
                               ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
  cost(Rel, 1, Pred, _, ResAttrList, ResTupleSize, ResCard1, _, _),
  costConst(btreelookup, msPerSearch, U),
  costConst(btreelookup, msPerResultTuple, V),
  ResCard is ResCard1 * Sel,
  Cost is   U
          + V * ResCard,
	NewTerm=exactmatchfun(A1, Rel, A3),
	!.

cost(exactmatch(A1, Rel, A3), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTerm)
  :-
  cost(Rel, 1, Pred, _, ResAttrList, ResTupleSize, ResCard1, _, _),
  costConst(btreelookup, msPerSearch, U),
  costConst(btreelookup, msPerResultTuple, V),
  ResCard is ResCard1 * Sel,
  Cost is   U
          + V * ResCard,
	NewTerm=exactmatch(A1, Rel, A3),
	!.

cost(exactmatchS(dbObject(Index), _KeyValue), Sel, _Pred, _, ResAttrList,
     ResTupleSize, ResCard, Cost, exactmatchS(dbObject(Index), _KeyValue))
  :-
  dcName2externalName(DcIndexName,Index),
  getIndexStatistics(DcIndexName, noentries, _DCrelName, ResCard1),
  ResAttrList = [[tid, tid, sizeTerm(MemSize, MemSize, 0)]],
  secDatatype(tid, MemSize, _, _, _, _),
  ResTupleSize = sizeTerm(MemSize,MemSize,0),
  costConst(btreelookup, msPerSearch, U),
  costConst(btreelookup, msPerResultTuple, V),
  ResCard is ResCard1 * Sel,
  Cost is   U
          + V * ResCard * 0.25 ,!. % balance is 75% cost for gettuple

cost(leftrange(A1, Rel, A3), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, leftrange(A1, Rel, A3))
  :-
  cost(Rel, 1, Pred, _, ResAttrList, ResTupleSize, ResCard1, _),
  costConst(btreelookup, msPerSearch, U),
  costConst(btreelookup, msPerResultTuple, V),
  ResCard is ResCard1 * Sel,
  Cost is   U
          + V * ResCard,!.

cost(leftrangeS(dbObject(Index), _KeyValue), Sel, _Pred, _, ResAttrList,
     ResTupleSize, ResCard, Cost, leftrangeS(dbObject(Index), _KeyValue))
  :-
  dcName2externalName(DcIndexName,Index),
  getIndexStatistics(DcIndexName, noentries, _DCrelName, ResCard1),
  ResAttrList = [[tid, tid, sizeTerm(MemSize, MemSize, 0)]],
  secDatatype(tid, MemSize, _, _, _, _),
  ResTupleSize = sizeTerm(MemSize,MemSize,0),
  costConst(btreelookup, msPerSearch, U),
  costConst(btreelookup, msPerResultTuple, V),
  ResCard is ResCard1 * Sel,
  Cost is   U
          + V * ResCard * 0.25 ,!. % balance is 75% cost for gettuple

cost(rightrange(A1, Rel, A3), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, rightrange(A1, Rel, A3))
  :-
  cost(Rel, 1, Pred, _, ResAttrList, ResTupleSize, ResCard1, _, _),
  costConst(btreelookup, msPerSearch, U),
  costConst(btreelookup, msPerResultTuple, V),
  ResCard is ResCard1 * Sel,
  Cost is   U
          + V * ResCard,!.

cost(rightrangeS(dbObject(Index), _KeyValue), Sel, _Pred, _, ResAttrList,
     ResTupleSize, ResCard, Cost, rightrangeS(dbObject(Index), _KeyValue))
  :-
  dcName2externalName(DcIndexName,Index),
  getIndexStatistics(DcIndexName, noentries, _DCrelName, ResCard1),
  ResAttrList = [[tid, tid, sizeTerm(MemSize, MemSize, 0)]],
  secDatatype(tid, MemSize, _, _, _, _),
  ResTupleSize = sizeTerm(MemSize,MemSize,0),
  costConst(btreelookup, msPerSearch, U),
  costConst(btreelookup, msPerResultTuple, V),
  ResCard is ResCard1 * Sel,
  Cost is   U
          + V * ResCard * 0.25 ,!. % balance is 75% cost for gettuple

cost(range(A1, Rel, A3), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, range(A1, Rel, A3)) :-
  cost(Rel, 1, Pred, _, ResAttrList, ResTupleSize, ResCard1, _, _),
  costConst(btreelookup, msPerSearch, U),
  costConst(btreelookup, msPerResultTuple, V),
  ResCard is ResCard1 * Sel,
  Cost is   U
          + V * ResCard,!.

cost(rangeS(dbObject(Index), _KeyValue), Sel, _Pred, _, ResAttrList,
     ResTupleSize, ResCard, Cost, rangeS(dbObject(Index), _KeyValue))
  :-
  dcName2externalName(DcIndexName,Index),
  getIndexStatistics(DcIndexName, noentries, _DCrelName, ResCard1),
  ResAttrList = [[tid, tid, sizeTerm(MemSize, MemSize, 0)]],
  secDatatype(tid, MemSize, _, _, _, _),
  ResTupleSize = sizeTerm(MemSize,MemSize,0),
  costConst(btreelookup, msPerSearch, U),
  costConst(btreelookup, msPerResultTuple, V),
  ResCard is ResCard1 * Sel,
  Cost is   U
          + V * ResCard * 0.25 ,!. % balance is 75% cost for gettuple

cost(loopjoin(X, Y), Sel, Pred, MT, ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> (ResAttrListX = ignore, ResAttrListY = ignore) ; true
  ),
  cost(X, 1, Pred, _, ResAttrListX, ResTupleSizeX, ResCardX, CostX, NewTermX),
  cost(Y, Sel, Pred, _, ResAttrListY, ResTupleSizeY, ResCardY, CostY, NewTermY),
  addSizeTerms([ResTupleSizeX,ResTupleSizeY],ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  append(ResAttrListX,ResAttrListY,ResAttrList)
  ),
  ResCard is ResCardX * ResCardY,

  % Whatever amount of memory we choose here...we still need the values for
  % the plan generation.
  ResTupleSizeX = sizeTerm(MemSizeX,_,_),
  ResTupleSizeY = sizeTerm(MemSizeY,_,_),
	Sizes=[ResCardX, MemSizeX, ResCardY, MemSizeY],
	genericCosts(MT, loopjoin(NewTermX, NewTermY), Sizes, OpCostsInMS, NewTerm),
  Cost is CostX + CostY + OpCostsInMS,
  !.

/*

The genericCosts predicate supports either to compute the costs on static
memory values that is assigned to every memory consuming operator or
based on a explict assigned memory amount. The idea behind this is 
that isn't needed to duplicate all costs predicates.

*/
% Non-join operator
genericCosts(MT, Term, [CardX, SizeX], OpCostsInMS, NewTerm) :-
	var(MT),
	Term=..[Op|_],
  getStaticMemory(MiB),
  opCosts(Op, CardX, SizeX, MiB, OpCostsInMS, AList),
  toMemoryTerm(Term, MiB, AList, NewTerm),
	nextCounter(memoryUsingOperators, _).

% Join operator
genericCosts(MT, Term, [CardX, SizeX, CardY, SizeY], OpCostsInMS, NewTerm) :-
	var(MT),
	Term=..[Op|_],
  getStaticMemory(MiB),
  opCosts(Op, CardX, SizeX, CardY, SizeY, MiB, OpCostsInMS, AList),
  toMemoryTerm(Term, MiB, AList, NewTerm),
	nextCounter(memoryUsingOperators, _).

% Second cost call with stored memory term.
genericCosts(MT, Term, _, OpCostsInMS, NewTerm) :-
	\+ var(MT),
  % In this case, the cost were calculated at least on time before.
  % We obtain now the new memory value from the memoryValue term 
  % and recompute the costs based on the new memory value and the
  % stored AList.
  % Of course, that the other variables like row size and
  % row cards are non-chaging is assumed.
  ensure((
		MT=memory(_, MID, AList), % If not found, there is something wrong.
		memoryValue(MID, MiB, _) % Calculate on the new given memory value.
	), 'Error: memoryValue missing'), 
  propertyValue(functionType, AList, FT),
  propertyValue(dlist, AList, DList),
	DList=[SuffMiB|_], % Don't use more memory as needed for the cost calculation.
	minimumOperatorMemory(MinOpMem), % And not less than this value.
	CMiB is max(MinOpMem, min(MiB, SuffMiB)),
  buildFormulaByFT(FT, DList, CMiB, CostF),
  OpCostsInMS is CostF,
	dm(ma6, ['\nComputed costs: ', OpCostsInMS]),
  NewTerm=Term,
	nextCounter(memoryUsingOperators, _).
  
genericCosts(MT, Term, _, OpCostsInMS, NewTerm) :-
	Msg='failed to compute costs',
  throw(error_Internal(ma_genericCosts(MT, Term, _, OpCostsInMS, NewTerm)::Msg)).


/*
The idea is to change the assigned memory value dynamically because later changes withn the path are more cost expensive to perform.
*/
toMemoryTerm(Term, MiB, AList, NewTerm) :-
	newMID(MID),
	assertz(memoryValue(MID, MiB, AList)),
	NewTerm=memory(Term, MID, AList).

clearMemoryValues :-
	retractall(memoryValue(_, _, _)).

:- 
	dynamic maxMID/1,
					memoryValue/3.
maxMID(0).

newMID(SMID) :-
	retract(maxMID(CurrentMID)),
	MID is CurrentMID+1,
	asserta(maxMID(MID)),
	atomic_concat('mid', MID, SMID).
% END NVK ADDED MA

cost(loopjoin(X, Y), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
	write('\nloopoin fallback!'), sleep(5),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> (ResAttrListX = ignore, ResAttrListY = ignore) ; true
  ),
  cost(X, 1, Pred, _, ResAttrListX, ResTupleSizeX, ResCardX, CostX, NewTermX),
  cost(Y, Sel, Pred, _, ResAttrListY, ResTupleSizeY, ResCardY, CostY, NewTermY),
  addSizeTerms([ResTupleSizeX,ResTupleSizeY],ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  append(ResAttrListX,ResAttrListY,ResAttrList)
  ),
  ResCard is ResCardX * ResCardY,
  Cost is CostX + ResCardX * CostY,
	NewTerm=loopjoin(NewTermX, NewTermY),
	!.

cost(loopsel(X, Y), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, loopsel(NewTermX, NewTermY)) :-
  cost(X, 1, Pred, _, ignore, _, ResCardX, CostX, NewTermX),
  cost(Y, Sel, Pred, _, ResAttrList, ResTupleSize, ResCardY, CostY, NewTermX),
  ResCard is ResCardX * ResCardY,
  Cost is CostX + ResCardX * CostY,!.

cost(fun(A1, X), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
  cost(X, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTermX),
	NewTerm=fun(A1, NewTermX), 
	!.

% NVK ADDED MA
cost(Term, Sel, Pred, MT, ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
	Term=hashjoin(X, Y, P1, P2, NBuckets),
	Term=..[Functor|_],
	
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> (ResAttrListX = ignore, ResAttrListY = ignore) ; true
  ),
  cost(X, Sel, Pred, _, ResAttrListX, ResTupleSizeX, ResCardX, CostX, NewTerm1),
  cost(Y, Sel, Pred, _, ResAttrListY, ResTupleSizeY, ResCardY, CostY, NewTerm2),
  addSizeTerms([ResTupleSizeX,ResTupleSizeY],ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  append(ResAttrListX,ResAttrListY,ResAttrList)
  ),
  ResCard is ResCardX * ResCardY * Sel,

  ResTupleSizeX = sizeTerm(MemSizeX,_,_),
  ResTupleSizeY = sizeTerm(MemSizeY,_,_),
  Sizes=[ResCardX, MemSizeX, ResCardY, MemSizeY],
  genericCosts(MT, Term, Sizes, OpCostsInMS, NewTerm),
  Cost is CostX + CostY + OpCostsInMS,
	!.
% NVK ADDED

% NVK: Fallback to default.
cost(hashjoin(X, Y, A1, A2, NBuckets), Sel, Pred, _,
                                   ResAttrList, ResTupleSize, ResCard, Cost, 
				hashjoin(NewTermX, NewTermY, A1, A2, NBuckets)) :-
	write('\nhashjoin fallback'), sleep(2),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> (ResAttrListX = ignore, ResAttrListY = ignore) ; true
  ),
  cost(X, Sel, Pred, _, ResAttrListX, ResTupleSizeX, ResCardX, CostX, NewTermX),
  cost(Y, Sel, Pred, _, ResAttrListY, ResTupleSizeY, ResCardY, CostY, NewTermY),
  addSizeTerms([ResTupleSizeX,ResTupleSizeY],ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  append(ResAttrListX,ResAttrListY,ResAttrList)
  ),
  ResCard is ResCardX * ResCardY * Sel,
  costConst(general, maxMemBytePerOperator, MaxMem),
  costConst(hashjoin, msPerProbeTuple, U),
  costConst(hashjoin, msPerRightTuple, V),
  costConst(hashjoin, msPerResultTuple, W),
  costConst(hashjoin, byteBufferSizeRatioY, R),
  ResTupleSizeY = sizeTerm(MemSizeY,_,_),
  MemorySecond is  MaxMem * R,
  NoPasses is 1 + ((ResCardY * MemSizeY) / MemorySecond),
  Cost is   CostX + CostY
          + ResCardY * V             % reading into hashtable
          + ResCardX * NoPasses * U  % probing
          + ResCard * W,!.           % output tuples

% NVK ADDED MA
/*
As far as i know, this will never be called because for ~order by~ predicates,
 no plan edges are created.:w
*/
cost(Term, Sel, Pred, MT, ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
	Term=sort(X),
  cost(X, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, CostX, NewTermX),
  ResTupleSize = sizeTerm(MemSizeX,_,_),
  Sizes=[ResCard, MemSizeX],
  genericCosts(MT, sort(NewTermX), Sizes, OpCostsInMS, NewTerm),
  Cost is CostX + OpCostsInMS,
  !.

cost(Term, Sel, Pred, MT, ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
	Term=sortby(X, Atts),
  cost(X, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, CostX, NewTermX),
  ResTupleSize = sizeTerm(MemSizeX,_,_),
  Sizes=[ResCard, MemSizeX],
  genericCosts(MT, sortby(NewTermX, Atts), Sizes, OpCostsInMS, NewTerm),
  Cost is CostX + OpCostsInMS,
  !.
% NVK ADDED END

cost(sort(X), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
	write('\nsort fallback'), sleep(2),
  cost(X, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, CostX, NewTermX),
  costConst(general, maxMemBytePerOperator, MaxMem),
  costConst(sortby, msPerByteInputSorted, U),
  costConst(sortby, msPerByteOutput, V),
  costConst(sortby, msPerByteWrittenToDisk, O),
  ResTupleSize = sizeTerm(Mem,Core,Lob),
  Size is max(Core + Lob, Mem),
  ( (MaxMem / Mem) < ResCard
    -> State is 0
    ;  State is 1
  ),
  Cost is   CostX
          + ResCard * Size * (U + O * State)
          + ResCard * Size * V,
	NewTerm=sort(NewTermX),
	!.


% Sortby with empty sorting list is ignored:
cost(sortby(X, []), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, sortby(NewTermX, [])) :-
  cost(X, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTermX),!.

cost(sortby(X, Y), Sel, Pred, MT, ResAttrList, ResTupleSize, ResCard, Cost, sortby(NewTermX, Y)) :-
  Y \= [],
  cost(sort(X), Sel, Pred, MT, ResAttrList, ResTupleSize, ResCard, Cost, NewTermX),!.


/*
NVK ADDED NR
I don't have any costs for these operations.
TODO: ResAttrList modifications.
*/
cost(unnest(X, Attr), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, unnest(NewTermX, Attr)) :-
  cost(X, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTermX),!.

cost(nest(X, Attr, NewLabel), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, nest(NewTermX, Attr, NewLabel)) :-
  cost(X, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTermX),!.
% NVK ADDED END NR

cost(mergejoin(X, Y, A3, A4), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> (ResAttrListX = ignore, ResAttrListY = ignore) ; true
  ),
  cost(X, 1, Pred, _, ResAttrListX, ResTupleSizeX, ResCardX, CostX),
  cost(Y, 1, Pred, _, ResAttrListY, ResTupleSizeY, ResCardY, CostY),
  addSizeTerms([ResTupleSizeX,ResTupleSizeY],ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  append(ResAttrListX,ResAttrListY,ResAttrList)
  ),
  ResCard is ResCardX * ResCardY * Sel,
  costConst(mergejoin, msPerTupleRead, U),
  costConst(mergejoin, msPerResultTuple, FX),
  costConst(mergejoin, msPerResultAttribute, FY),
  length(ResAttrList,NoAttrs),
  Cost is   CostX + CostY
          + (ResCardX + ResCardY) * (ResCardX + ResCardY) * U
          + ResCard * (FX + NoAttrs * FY),
	NewTerm=mergejoin(X, Y, A3, A4),
	!.


cost(sortmergejoin(X, Y, AX, AY), Sel, Pred, MT,
    ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> (ResAttrListX = ignore, ResAttrListY = ignore) ; true
  ),
  cost(X, 1, Pred, _, ResAttrListX, ResTupleSizeX, ResCardX, CostX, NewTermX),
  cost(Y, 1, Pred, _, ResAttrListY, ResTupleSizeY, ResCardY, CostY, NewTermY),
  addSizeTerms([ResTupleSizeX,ResTupleSizeY],ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  append(ResAttrListX,ResAttrListY,ResAttrList)
  ),
  ResCard is ResCardX * ResCardY * Sel,

  ResTupleSizeX = sizeTerm(MemX,CoreX,LobX),
  ResTupleSizeY = sizeTerm(MemY,CoreY,LobY),
  SizeX is max(CoreX + LobX,MemX),
  SizeY is max(CoreY + LobY,MemY),
  Sizes=[ResCardX, SizeX, ResCardY, SizeY],
  genericCosts(MT, sortmergejoin(NewTermX, NewTermY, AX, AY), Sizes, OpCostsInMS, NewTerm),
  Cost is CostX + CostY + OpCostsInMS,
  !.

cost(sortmergejoin(X, Y, AX, AY), Sel, Pred, _,
		ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
	write('\nsortmergejoin fallback'), sleep(2),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> (ResAttrListX = ignore, ResAttrListY = ignore) ; true
  ),
  cost(X, 1, Pred, _, ResAttrListX, ResTupleSizeX, ResCardX, CostX, NewTermX),
  cost(Y, 1, Pred, _, ResAttrListY, ResTupleSizeY, ResCardY, CostY, NewTermY),
  addSizeTerms([ResTupleSizeX,ResTupleSizeY],ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  append(ResAttrListX,ResAttrListY,ResAttrList)
  ),
  ResCard is ResCardX * ResCardY * Sel,
  costConst(sortmergejoin, msPerByteReadSort, USortBy),
  costConst(mergejoin, msPerResultTuple, XMergeJoin),
  costConst(mergejoin, msPerResultAttribute, YMergeJoin),
  costConst(sortmergejoin, msPerByteReadMerge, WMergeJoin),
  length(ResAttrList,NoAttrs),
  ResTupleSizeX = sizeTerm(MemX,CoreX,LobX),
  ResTupleSizeY = sizeTerm(MemY,CoreY,LobY),
  SizeX is max(CoreX + LobX,MemX),
  SizeY is max(CoreY + LobY,MemY),
  Cost is   CostX + CostY
          + ResCardX * SizeX * USortBy
          + ResCardY * SizeY * USortBy
          + (ResCardX * SizeX + ResCardY * SizeY) * WMergeJoin
          + ResCard * (XMergeJoin + NoAttrs * YMergeJoin),
	NewTerm=sortmergejoin(NewTermX, NewTermY, AX, AY),
	!.


% two rules used by the 'interesting orders extension':
cost(sortLeftThenMergejoin(X, Y, AX, AY), Sel, Pred, _,
    ResAttrList, ResTupleSize, ResCard, Cost, 
		mergejoin(sortby(NewTermX, [AX]), Y, AX, AY)) :-
  cost(mergejoin(sortby(X, [AX]), Y, AX, AY), Sel, Pred, _, 
    ResAttrList, ResTupleSize, ResCard, Cost, NewTermX),!.

cost(sortRightThenMergejoin(X, Y, AX, AY), Sel, Pred, _,
    ResAttrList, ResTupleSize, ResCard, Cost, 
		mergejoin(NewTermX, sortby(Y, [AY]), AX, AY)) :-
  cost(mergejoin(X, sortby(Y, [AY]), AX, AY), Sel, Pred, _, 
    ResAttrList, ResTupleSize, ResCard, Cost, NewTermX),!.


% NVK ADDED MA
cost(symmjoin(X, Y, A1), Sel, Pred, MT, ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> (ResAttrListX = ignore, ResAttrListY = ignore) ; true
  ),
  cost(X, 1, Pred, _, ResAttrListX, ResTupleSizeX, ResCardX, CostX, NewTermX),
  cost(Y, 1, Pred, _, ResAttrListY, ResTupleSizeY, ResCardY, CostY, NewTermY),
  addSizeTerms([ResTupleSizeX,ResTupleSizeY],ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  append(ResAttrListX,ResAttrListY,ResAttrList)
  ),
  ResCard is ResCardX * ResCardY * Sel,

  % Whatever amount of memory we choose here...we still need the values for
  % the plan generation.
  ResTupleSizeX = sizeTerm(MemSizeX,_,_),
  ResTupleSizeY = sizeTerm(MemSizeY,_,_),

  Sizes=[ResCardX, MemSizeX, ResCardY, MemSizeY],
  genericCosts(MT, symmjoin(NewTermX, NewTermY, A1), Sizes, OpCostsInMS, NewTerm),
  Cost is CostX + CostY + OpCostsInMS,
  !.


% NVK ADDED

% NVK: Fallback to default.
cost(symmjoin(X, Y, A1), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
	write('\nsymmjoin fallback'), sleep(2),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> (ResAttrListX = ignore, ResAttrListY = ignore) ; true
  ),
  cost(X, 1, Pred, _, ResAttrListX, ResTupleSizeX, ResCardX, CostX, NewTermX),
  cost(Y, 1, Pred, _, ResAttrListY, ResTupleSizeY, ResCardY, CostY, NewTermY),
  costConst(symmjoin, msPerTuplePair, U),
  addSizeTerms([ResTupleSizeX,ResTupleSizeY],ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  append(ResAttrListX,ResAttrListY,ResAttrList)
  ),
  ResCard is ResCardX * ResCardY * Sel,
  getPET(Pred, _, ExpPET),             % only uses experimental PET
  Cost is   CostX + CostY
          + ResCardX * ResCardY * (ExpPET + U),
	NewTerm=symmjoin(NewTermX, NewTermY, A1), 
	!.


cost(spatialjoin(X, Y, A3, A4), Sel, Pred, _,
                              ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> (ResAttrListX = ignore, ResAttrListY = ignore) ; true
  ),
  cost(X, 1, Pred, _, ResAttrListX, ResTupleSizeX, ResCardX, CostX, NewTermX),
  cost(Y, 1, Pred, _, ResAttrListY, ResTupleSizeY, ResCardY, CostY, NewTermY),
  spatialjoinTC(A, B),                 % TODO: constants needed!
  addSizeTerms([ResTupleSizeX,ResTupleSizeY],ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  append(ResAttrListX,ResAttrListY,ResAttrList)
  ),
  ResCard is ResCardX * ResCardY * Sel,
  Cost is   CostX + CostY              % effort is essentially proportional to
          + A * (ResCardX + ResCardY)  % the sizes of argument streams
          + B * ResCard,             % cost to produce result tuples
	NewTerm=spatialjoin(NewTermX, NewTermY, A3, A4),
	!.


/*
costs for pjoin2 will only be used if option ~adpativeJoin~ is enabled.

*/

% NVK Currently not supported
cost(pjoin2(X, Y, [ A1 | A2 ]), Sel, Pred, _,
    ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
  cost(sortmergejoin(X, Y, _, _), Sel, Pred, _,
    ResAttrList, ResTupleSize, ResCard, Cost1, NewTermX),
  cost(hashjoin(X, Y, _, _, 99997), Sel, Pred, _,
    ResAttrList, ResTupleSize, ResCard, Cost2, NewTermY),
  Cost is min(Cost1, Cost2),
	NewTerm=pjoin2(X, Y, [ A1 | A2 ]),
	!.

cost(pjoin2_hj(X, Y, [ _ | _ ]), Sel, Pred,
                              ResAttrList, ResTupleSize, ResCard, Cost) :-
  cost(hashjoin(X, Y, _, _, 99997), Sel, Pred,
                              ResAttrList, ResTupleSize, ResCard, Cost),!.

cost(_, pjoin2_smj(X, Y, [ _ | _ ]), Sel, Pred,
                              ResAttrList, ResTupleSize, ResCard, Cost) :-
  cost(_, sortmergejoin(X, Y, _, _), Sel, Pred,
                              ResAttrList, ResTupleSize, ResCard, Cost),!.


cost(extend(X, ExtendFields), Sel, Pred, _,
    ResAttrList, ResTupleSize, ResCard, Cost, extend(NewTermX, ExtendFields)) :-
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> ResAttrList1 = ignore ; true
  ),
  cost(X, Sel, Pred, _, ResAttrList1, ResTupleSize1, ResCard, Cost1, NewTermX),
  costConst(extend, msPerTuple, U),
  costConst(extend, msPerTupleAndAttribute, V),
  ( Pred = pr(_,RelArg1)
    -> RelInfoList = [(1,RelArg1)]
    ; (Pred = pr(_,RelArg1,RelArg2), RelInfoList = [(1,RelArg1),(2,RelArg2)])
  ),
  createExtendAttrList(ExtendFields,RelInfoList,ExtendAttrs,ExtendAttrSize),
  addSizeTerms([ResTupleSize1, ExtendAttrSize],ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  append(ResAttrList1,ExtendAttrs,ResAttrList)
  ),
  length(ExtendFields,NoNewAttrs),
  Cost is   Cost1
          + ResCard * (U + NoNewAttrs * V),
  dm(costFunctions,['*****************************************************\n']),
  dm(costFunctions,['Extended Attributes: ',ExtendAttrs,'\n']),
  dm(costFunctions,['*****************************************************\n']),
  !.


cost(remove(X, DropListFields), Sel, Pred, _, ResAttrList, ResTupleSize, 
		ResCard, Cost, remove(NewTermX, DropListFields)) :-
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> ResAttrList1 = ignore ; true
  ),
  cost(X, Sel, Pred, _, ResAttrList1, ResTupleSize1, ResCard, Cost1, NewTermX),
  costConst(project, msPerTuple, U),
  costConst(project, msPerTupleAndAttr, V),
  findall(AttrName,
         ( member([AttrName, _, _],ResAttrList1),
           not(memberchk(attr(AttrName,_,_),DropListFields))
         ),
         ProjAttrNames),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> ( ResTupleSize = ResTupleSize1,             %% ToDo: Fixme
         NoAttrs is 3                              %% ToDo: Fixme
       )
    ;  ( projectAttrList(ResAttrList1, ProjAttrNames, ResAttrList, ResTupleSize),
         length(ResAttrList,NoAttrs)
       )
  ),
  length(ResAttrList,NoAttrs),
  Cost is   Cost1
          + ResCard * ( U + NoAttrs * V ),!.

cost(project(X,ProjAttrFields), Sel, Pred, _, ResAttrList, ResTupleSize, 
		ResCard, Cost, project(NewTermX,ProjAttrFields)) :-
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> ResAttrList1 = ignore ; true
  ),
  cost(X, Sel, Pred, _, ResAttrList1, ResTupleSize1, ResCard, Cost1, NewTermX),
  costConst(project, msPerTuple, U),
  costConst(project, msPerTupleAndAttr, V),
  findall(AttrName,
         memberchk(attr(AttrName,_,_),ProjAttrFields),
         ProjAttrNames),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> ResTupleSize = ResTupleSize1             %% ToDo: Fixme
    ;  projectAttrList(ResAttrList1, ProjAttrNames, ResAttrList, ResTupleSize)
  ),
  length(ProjAttrNames,NoAttrs),
  Cost is   Cost1
          + ResCard * ( U + NoAttrs * V ),!.

cost(projectextend(X,ProjAttrFields,ExtendFields), Sel, Pred, _, ResAttrList,
		ResTupleSize, ResCard, Cost, 
		projectextend(NewTermX,ProjAttrFields,ExtendFields)) :-
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> ResAttrList1 = ignore ; true
  ),
  cost(X, Sel, Pred, _, ResAttrList1, ResTupleSize1, ResCard, Cost1, NewTermX),
  costConst(project, msPerTuple, U),
  costConst(project, msPerTupleAndAttr, VP),
  costConst(extend,  msPerTupleAndAttribute, VE),
  findall(AttrName,
         memberchk(attr(AttrName,_,_),ProjAttrFields),
         ProjAttrNames),
  projectAttrList(ResAttrList1, ProjAttrNames, ResAttrList2, ResTupleSize2),
  ( Pred = pr(_,RelArg1)
    -> RelInfoList = [(1,RelArg1)]
    ; (Pred = pr(_,RelArg1,RelArg2), RelInfoList = [(1,RelArg1),(2,RelArg2)])
  ),
  createExtendAttrList(ExtendFields,RelInfoList,ExtendAttrs,ExtendAttrSize),
  addSizeTerms([ResTupleSize2, ExtendAttrSize],ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> ResTupleSize = ResTupleSize1             %% ToDo: Fixme
    ;  append(ResAttrList2,ExtendAttrs,ResAttrList)
  ),
  length(ProjAttrFields,NoPAttrs),
  length(ExtendFields,NoEAttrs),
  Cost is   Cost1
          + ResCard * (  U                    % per tuple
                       + NoEAttrs * VE        % extended attrs
                       + NoPAttrs * VP ),     % projected attrs
  dm(costFunctions,['*****************************************************\n']),
  dm(costFunctions,['Extended Attributes: ',ExtendAttrs,'\n']),
  dm(costFunctions,['*****************************************************\n']),
  !.


%% Missing: extendstream
%% Missing: projectextendstream
%% Missing: groupby

cost(rename(X, Suffix), Sel, Pred, _,
                        ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> ResAttrList1 = ignore ; true
  ),
  cost(X, Sel, Pred, _, ResAttrList1, ResTupleSize, ResCard, Cost, NewTermX),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  renameAttrs(ResAttrList1, Suffix, ResAttrList)
  ),
	NewTerm=rename(NewTermX, Suffix),
	!.

cost(rdup(X), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, rdup(NewTermX)) :-
  costConst(rdup, msPerTuple, U),
  costConst(rdup, msPerComparison, V),
  costConst(rdup, defaultSelectivity, W),
  Sel1 is Sel/W,  %% claim a fraction of the overall selectivity for rdup
                  %% rdup filters out an relative amount of (1-W) duplicats
  cost(X, Sel1, Pred, _, ResAttrList, ResTupleSize, ResCard1, Cost1, NewTermX),
  ResCard is ResCard1 * W,
  Cost is   Cost1
          + ResCard1 * (U + V),!. %% ToDo: Cost function looks strange...

cost(krdup(X,_AttrList), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, krdup(NewTermX,_AttrList)) :-
  cost(rdup(X), Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, Cost, NewTermX), !.

cost(windowintersects(dbobject(Index), Rel, QueryObj), Sel, Pred, _,
	 	ResAttrList, ResTupleSize, ResCard, Cost, 
		windowintersects(dbobject(Index), Rel, QueryObj)) :-
  cost(Rel, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard1, Cost1, _),
  costConst(windowintersects, msPerTuple, U),
  costConst(windowintersects, msPerByte, V),
  ResTupleSize = sizeTerm(Mem,Core,Lob),
  SizeE is max(Mem+Lob,Core+Lob), %% assuming Rel has only one FLOB
  ResCard is ResCard1 * Sel, %% ToDo: Estimate number of results using
                             %%       statistics on Index, Rel, and QueryObj
                             %%       eg. Sel = Keys(Index) * area(bbox(Index))
                             %%                 / bboxArea(QueryObj) / ResCard1
  Cost is Cost1 + max(ResCard,1) * (U + SizeE * V) /* * PET */,!.


% Cost function copied from windowintersects
% May be wrong, but as it is usually used together
% with 'gettuples', the total cost should be OK
cost(windowintersectsS(dbobject(IndexName), QueryObj), Sel, Pred, _,
                  ResAttrList, ResTupleSize, ResCard, Cost, windowintersectsS(dbobject(IndexName), QueryObj)) :- 
	% get relationName Rel from Index (it is not included in the arguments)
  concat_atom([RelName|_],'_',IndexName),
  dcName2internalName(RelDC,RelName),
  Rel = rel(RelDC, *),
  cost(Rel, Sel, Pred, _, ignore, _, ResCard1, Cost1, _),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true
    ;  ( secDatatype(tid, TMem, _, _, _, _), % Xris: Error
         ResAttrList = [[id, tid, sizeTerm(TMem, 0, 0)]]
       )
  ),
  costConst(windowintersects, msPerTuple, U),
  costConst(windowintersects, msPerByte, V),
  ResTupleSize = sizeTerm(TMem,0,0),
  SizeE is TMem,             % assuming Rel has only one FLOB
  ResCard is ResCard1 * Sel, %% ToDo: Estimate number of results using
                             %%       statistics on Index, Rel, and QueryObj
                             %%       eg. Sel = Keys(Index) * area(bbox(Index))
                             %%                 / bboxArea(QueryObj) / ResCard1
  Cost is   Cost1            % expected to include cost of 'windowintersectsS'
          + max(ResCard,1) * (U + SizeE * V)
          * 0.25,!.          % other 0.75 applied in 'gettuples'

cost(gettuples(X, Rel), Sel, Pred, _, ResAttrList, ResTupleSize, 
		ResCard, Cost, gettuples(NewTermX, Rel)) :-
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> (ResAttrList1 = ignore, ResAttrList2 = ignore) ; true
  ),
  cost(X, Sel, Pred, _, ResAttrList1, ResTupleSize1, ResCard, Cost1, NewTermX),
  cost(Rel, Sel, Pred, _, ResAttrList2, ResTupleSize2, _, _, _),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> (  secDatatype(tid, TMem, _, _, _, _),
          negateSizeTerm(sizeTerm(TMem, 0, 0),NegTidSize),
          addSizeTerms([NegTidSize,ResTupleSize1,ResTupleSize2],ResTupleSize)
       )
    ;  ( delete(ResAttrList1,[_,tid,TidSize],ResAttrList1WOtid), % drop tid-attr
         negateSizeTerm(TidSize,NegTidSize),                     % adjust size
         append(ResAttrList1WOtid,ResAttrList2,ResAttrList),     % concat tuples
         addSizeTerms([NegTidSize,ResTupleSize1,ResTupleSize2],ResTupleSize)
       )
  ),
  ResTupleSize2 = sizeTerm(Mem,Core,Lob),
  SizeE is max(0,max(Mem+Lob,Core+Lob)), % assuming Rel has only one FLOB
  costConst(windowintersects, msPerTuple, U),
  costConst(windowintersects, msPerByte, V),
  Cost is   Cost1           % expected to include cost of 'windowintersectsS'
          + ResCard * (U + SizeE * V) * 0.75,!. % other 0.25 applied
                                                % in 'windowintersectsS'

cost(gettuples2(X, Rel, attrname(TidAttr)), Sel, Pred, _, ResAttrList, 
		ResTupleSize, ResCard, Cost, 
		gettuples2(NewTermX, Rel, attrname(TidAttr))) :-
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> (ResAttrList1 = ignore, ResAttrList2 = ignore) ; true
  ),
  cost(X, Sel, Pred, _, ResAttrList1, ResTupleSize1, ResCard, Cost1, NewTermX),
  cost(Rel, Sel, Pred, _, ResAttrList2, ResTupleSize2, _, _, _),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> ( secDatatype(tid, TMem, _, _, _, _),
         negateSizeTerm(sizeTerm(TMem, 0, 0),NegTidSize),
         addSizeTerms([NegTidSize,ResTupleSize1,ResTupleSize2],ResTupleSize)
       )
    ;  ( delete(ResAttrList1,[TidAttr,tid,TidSize],ResAttrList1WOtid), % drop tid-attr
         negateSizeTerm(TidSize,NegTidSize),                          % adjust size
         append(ResAttrList1WOtid,ResAttrList2,ResAttrList),           % concat tuples
         addSizeTerms([NegTidSize,ResTupleSize1,ResTupleSize2],ResTupleSize)
       )
  ),
  ResTupleSize2 = sizeTerm(Mem,Core,Lob),
  SizeE is max(Mem+Lob,Core+Lob), % assuming Rel has only one FLOB
  costConst(windowintersects, msPerTuple, U),
  costConst(windowintersects, msPerByte, V),
  Cost is   Cost1           % expected to include cost of 'windowintersectsS'
          + ResCard * (U + SizeE * V) * 0.75,!. % other 0.25 applied
                                                % in 'windowintersectsS'

/*
cost functions for distancescan queries

*/

% get the Result properties from the POG's high node
cost(pogstream, _, _, _, ResAttrList, ResTupleSize, ResCard, 0, pogstream) :-
  highNode(Node),
  resultSize(Node, ResCard),
  getResTupleSize(Node, ResTupleSize),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true ;  getResAttrList(Node, ResAttrList)
  ).

cost(distancescan(A1, Rel, A3, A4), Sel, Pred, _,
     ResAttrList, ResTupleSize, ResCard, Cost, NewTerm) :-
  cost(Rel, Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, C1, NewTermX),
  distancescanTC(C),
  Cost is C1 + C * ResCard * log(ResCard + 1),
	NewTerm=distancescan(A1, Rel, A3, A4).

cost(ksmallest(X, K), Sel, Pred, _,
     ResAttrList, ResTupleSize, ResCard, C, NewTerm) :-
  cost(X, Sel,Pred, _, ResAttrList, ResTupleSize, ResCard, CostX, NewTermX),
  ksmallestTC(A, B),
  S is min(ResCard, K),
  C is CostX +
    A * ResCard +
    B * S * log(S + 1),
	NewTerm=ksmallest(NewTermX, K).

cost(createtmpbtree(rel(Rel, A2), A3), _, _, _, ResAttrList, ResTupleSize, 
		ResCard, Cost, NewTerm) :-
  dcName2internalName(RelDC,Rel),
  ( Rel = RelDC
    -> true
    ;  (
         write('ERROR:\tcost/8 failed due to non-dc relation name.'), nl,
         write('---> THIS SHOULD BE CORRECTED!'), nl,
         throw(error_Internal(optimizer_cost(createtmpbtree(Rel, _), _, _,
                               ResAttrList, ResTupleSize, ResCard, 0)
              ::malformedExpression)),
         fail
       )
  ),
  ( (ground(ResAttrList), ResAttrList = ignore)
    -> true ; getRelAttrList(RelDC, ResAttrList, _/*ResTupleSize*/)
  ),
  tupleSizeSplit(RelDC,ResTupleSize),
  card(Rel, ResCard),
  createbtreeTC(C),
  Cost is C * ResCard * log(ResCard),
	NewTerm=createtmpbtree(rel(Rel, A2), A3).

% predinfo inflicts no cost, it only annotates estimated selectivity and PET
% for progress estimation by the Secondo kernel
cost(predinfo(X, _piSel, _piPET), Sel, Pred, _,
     ResAttrList, ResTupleSize, ResCard, C, predinfo(NewTermX, _piSel, _piPET)) :-
  cost(X,Sel, Pred, _, ResAttrList, ResTupleSize, ResCard, C, NewTermX),
	!.

% Failed to compute the cost -- throw an exception!
cost(T, S, P, MT, A, TS, RC, Cost, NewTerm) :-
  concat_atom(['Calculation of cost failed.'],'',ErrMsg),
  write(ErrMsg), nl,
  throw(error_Internal(improvedcosts_cost(T, S, P, MT, A, TS, RC, Cost, NewTerm)
                   ::unknownError::ErrMsg)),
  !, fail.


/*
The following code fragment may be needed, when also the non-conjunctive
part of the query will be assigned with costs. At the moment, it is obsolete
and therefore commented out:

----

% Dummy-Costs for simple aggregation queries
cost(simpleAggrNoGroupby(_, Stream, _), S, P, A, TS, RC, Cost) :-
  cost(Stream, S, P, A, TS, RC, Cost).

cost(simpleUserAggrNoGroupby(Stream, _, _, _), S, P, A, TS, RC, Cost) :-
  cost(Stream, S, P, A, TS, RC, Cost).

----

*/

% Section:Start:cost_7_e
% Section:End:cost_7_e


/*
8.2 Predicates Auxiliar to cost/8

*/

% Succeeds, if ~X~ is a term that could act as a prefilter
% --- isPrefilter(+X)

% Section:Start:isPrefilter_1_b
% Section:End:isPrefilter_1_b

isPrefilter(X) :-
  X = spatialjoin(_, _, _, _),!.

isPrefilter(X) :-
  X = loopjoin(_, _),!.

% Section:Start:isPrefilter_1_e
% Section:End:isPrefilter_1_e

% Reflect modifications done by the ~rename~ operator to an attribute list
%     (i.e. a common ~Suffix~ is appended to every attribute name
% --- renameAttrs(+AttrList, +Suffix, -RenamedAttrList)
renameAttrs([],_,[]).
renameAttrs([[Attr, T, S]|More], Suffix, [[AttrRenamed, T, S]|MoreRes]) :-
  concat_atom([Attr, Suffix],'_',AttrRenamed),
  renameAttrs(More,Suffix,MoreRes), !.


:- dynamic(nodeAttributeList/2).

/*
---- setNodeResAttrList(+Node, +AttrList)
----

Annotate a node of the POG with a list of available attribute list ~AttrList~.

*/
% setNodeResAttrList(+Node, +AttrList)
setNodeResAttrList(Node, _) :- nodeAttributeList(Node, _), !.
setNodeResAttrList(Node, AttrList) :-
  ground(Node), ground(AttrList),
  assert(nodeAttributeList(Node, AttrList)),!.
setNodeResAttrList(N, A) :-
  concat_atom(['Error in setNodeResAttrList: Unbound variable.'],'',ErrMsg),
  write(ErrMsg), nl,
  throw(error_Internal(improvedcosts_setNodeResAttrList(N,A)
                   ::malformedExpression::ErrMsg)),
  !, fail.

/*
----
getResAttrList(+Node, -AttrList)
getResTupleSize(+Node, -TupleSize)
----
Retrieve tuple sizes and attributes at given nodes.

*/
% getResAttrList(+Node, -AttrList)
getResAttrList(0,[]) :- !.
getResAttrList(Node,AttrList) :-  
	nodeAttributeList(Node, AttrList), !.

% getResTupleSize(+Node, -TupleSize)
getResTupleSize(0,sizeTerm(0,0,0)) :- !.
getResTupleSize(Node, TupleSize) :- nodeTupleSize(Node, TupleSize), !.

/*

---- getSignature([+Op,+Args,+ResultType], -Op, -ArgTypeList) :-
----

Extract the operator name and the list of argument types of a ~TypeTree~
expression. If the signature cannot be retrieved, ~typeerror~ is returned
instead of the argument type list.

The input parameters [~Op~, ~Args~, ~ResultType~] can be computed for an
expression ~Expr~ and a list of relation descriptors ~RelList~ by calling

---- getSignature(+Expr,+RelList,-TypeTree)
----

~TypeTree~ then has the required format [~Op~, ~Args~, ~ResultType~].

*/
getSignature([Op,Args,_], Op, ArgTypeList) :-
  findall(T,(member([_,_,T],Args)),ArgTypeList),!.
getSignature([Op,_,_],Op,typeerror).
getSignature(A,B,C) :-
  throw(error_Internal(improvedcosts_getSignature(A,B,C)
                   ::wrongInstantiationPattern)),
  !, fail.


/*
8.3 Applying Cost Estination and Annotation of Intermediate Results

---- costterm(+Term, +Source, +Target, +Result, +Sel, +Pred, -Card, -Cost)
----

Determine the assessed costs of an input term using rules ~cost/8~.

~Term~ is the term, whose cost is to estimate,
~Source~ and ~Target~ are the pog node numbers incident to the edge represented by ~Term~,
~Result~ is the result number used for the result of the edge,
~Sel~ is the overall edge selectivity,
~Pred~ is the predicate represented by the edge,
~Card~ is the expected cardinality at the ~Target~ node,
~Cost~ is the expected cost in milliseconds.
~NewTerm~ allows term modifications to store the assigned amount of memory. (NVK ADDED)

*/
% costterm(+Term, +Source, +Target, +Result, +Sel, +Pred, -Card, -Cost, -NewTerm)
costterm(Term, Source, Target, Result, Sel, Pred, Card, Cost, NewTerm) :-
  ( optimizerOption(memoryAllocation) % NVK MODIFIED
   -> ( 
				cost(Term, Sel, Pred, _, ResAttrList, TupleSize, Card, Cost, NewTerm),
        setNodeResAttrList(Result, ResAttrList),
        setNodeTupleSize(Result, TupleSize)
      )
   ;  throw(error_Internal(ma_improvedcosts_costterm(Term, Source, Target, 
				Result, Sel, Pred, Card, Cost, NewTerm)::
				should_not_be_called_without_option_assignMemory))
  ),
  !.

/*
8.4 Operator-related Constants

Operator-related constants used within the cost functions should be
stored in facts

---- costConst(+OpName, +ConstName, -Value)
----

*/

% Section:Start:costConst_3_b
% Section:End:costConst_3_b

costConst(general, maxMemBytePerOperator, 4194304). % 4 MB buffers per operator
costConst(feed, pertuple, 0.00194).
costConst(feed, perbyte, 0.0000196).
costConst(feedproject, msPerTuple, 0.002).
costConst(feedproject, msPerByte, 0.000036).
costConst(feedproject, msPerAttr, 0.0018).
costConst(consume, msPerTuple, 0.024).
costConst(consume, msPerByteCore, 0.0003).
costConst(consume, msPerByteLOB, 0.001338).
costConst(project, msPerTuple, 0.00073).
costConst(project, msPerTupleAndAttr, 0.0004).
costConst(filter, msPerTuple, 0.01).
costConst(product, msPerByteRightOnBufferOverflow, 0.0003).
costConst(product, msPerByteTotalOutput, 0.000042).
costConst(btreelookup, msPerSearch, 0.15).
costConst(btreelookup, msPerResultTuple, 0.018).
costConst(hashjoin, msPerProbeTuple, 0.023).
costConst(hashjoin, msPerRightTuple, 0.0067).
costConst(hashjoin, msPerResultTuple, 0.0025).
costConst(hashjoin, byteBufferSizeRatioY, 0.75).
costConst(sortby, msPerByteInputSorted, 0.000396).
costConst(sortby, msPerByteOutput, 0.000194).
costConst(sortby, msPerByteWrittenToDisk, 0.00004).
costConst(mergejoin, msPerTupleRead, 0.0008077).
costConst(mergejoin, msPerResultTuple, 0.0012058).
costConst(mergejoin, msPerResultAttribute, 0.0001072).
costConst(sortmergejoin, msPerByteReadMerge, 0.0001738).
costConst(sortmergejoin, msPerByteReadSort, 0.00043).
costConst(symmjoin, msPerTuplePair, 0.2).
costConst(extend, msPerTuple, 0.0012).
costConst(extend, msPerTupleAndAttribute, 0.00085).
costConst(rdup, msPerTuple, 0.01).
costConst(rdup, msPerComparison, 0.1).
costConst(rdup, defaultSelectivity, 0.9).
costConst(windowintersects, msPerTuple, 0.00194).
costConst(windowintersects, msPerByte, 0.0000106).

% Section:Start:costConst_3_e
% Section:End:costConst_3_e

/*
End of file ~improvedcosts.pl~

*/
