/*

$Header$
@author Nikolai van Kempen

Provides a extension to the secondo optimizer to optimize the memory allocation of operators within a query.

*/

% Not declared as dynamic within the optimizer.
:- dynamic costEdge/6. 

/*
This predicate returns the GlobalMemory property from the SecondoConfig.ini file. 
Unit: MiB
secondoGlobalMemory(?Memory)
*/
:- dynamic cacheGlobalMemory/1. % Fix value during runtime
secondoGlobalMemory(Memory) :- 
	cacheGlobalMemory(Memory),
	!.

secondoGlobalMemory(Memory) :- 
	predicate_property(secondo_global_memory(_), foreign), 
	secondo_global_memory(Memory),
	assertz(cacheGlobalMemory(Memory)),
	!.

% Fallback to the current default value.
secondoGlobalMemory(512) :-
	!.

/*
Every operator gains at least this amount of memory. But this extension is able to compute solutions for other values. But the lowest value is one MiB. Refering to other comments why this lower limit can't be less then one megabyte.
Unit: MiB
*/
minimumOperatorMemory(16).

/*
Some strategies are implemented to get a memory-optimized plan.

Strategy 'staticEqual':
Emulate the default strategy when this extension is disabled, but with computed costs by the ~CostEstimation~ implementations.

Strategy 'static':
The most simple one is to give every operator a fixed amount of memory and after the computes ssp within the pog the execution plan ist optimized for the memory distribution.

Strategy 'staticDiv':
Distriubutes the entire memory equally to all memory consuming operators. Note that this amount of memory consuming operators is calculated very expensive.

Strategy 'enumerate':
Enumerates all available paths within the POG and optimizes every path.

Strategy 'modifiedDijkstra':
Based on the POG dijkstra algorithm, it avoids to optimize many paths. This is more a heurisk as this algorithm does not guarantee to output the best result.

Open issue: a more suitable algorithm to find a shortest path within a graph with nonlineare cost functions, constraints and variables which represent a consume.

*/
setMAStrategies(List) :-
	retractall(maStrategiesToEval(_)),
	asserta(maStrategiesToEval(List)).

createExtendedEvalList(List) :-
	secondoGlobalMemory(Max), % This is secondo's default value
	minimumOperatorMemory(MinMem),
	List=[
 		% No optimation, assign maxMemory/memory operators within the plan.
		staticEqual(16),

		% 2-phase approach
		static(MinMem), 
		static(Max), 
		% This strategy is totally unsuitable for more then 5 predicates and a high
		% number of paths. Just implemented to evaluate results.
		staticMaxDivOpCount,

		% Divides the global memory by the pog height (=number of predicates).
		% The differences to the staticMaxDivOpCount method is, that this
		% values can be computed very fast.
		staticMaxDivPOG,

		% 1-phase approach
		modifiedDijkstra,
		% This strategy is totally unsuitable for more then 5 predicates and a high
		% number of paths. Just implemented to evaluate results.
 		enumeratePaths
	], 
	!.

% Evaluation mode: Test all strategies
maEval :-
	createExtendedEvalList(List),
	setMAStrategies(List).

% Default mode
maNoEval :-
	setMAStrategies([modifiedDijkstra]).

:- 
	dynamic(maStrategiesToEval/1),
	dynamic(maCurrentBestPlan/3),
	dynamic(maNLOPTResult/1),
	maNoEval.

/*
Returns the amount of memory that is used to compute the costs for memory
using operators during cost edge creation.
Unit: MiB
*/
:- dynamic staticMemory/1.
staticMemory(16). 

getStaticMemory(MiB) :-
	staticMemory(MiB), 
	!.

getStaticMemory(MiB) :-
	!, 
	Msg='The staticMemory fact is missing.',
	throw(error_Internal(ma_getStaticMemory(MiB)::Msg)).

setStaticMemory(MiB) :-
	ensure(integer(MiB)),
	retractall(staticMemory(_)),
	asserta(staticMemory(MiB)).

/*
Chooses an appropriate amount of memory that is assigned to every operator. The idea is to find, with this memory assignments in MList, a path that is equal to the result of the one-phase approach.
*/
getMemoryTestList(MList) :-
  maStrategiesToEval(List),
  findall(MiB, (member(A, List), A=static(MiB)), MList).

/*
Provable if strategy S should be evaluated.
*/
testStrategy(S) :-
  maStrategiesToEval(List),
	member(S, List).

/*
This predicates tries to find the best plan regarding the optimal memory allocation for the memory using operators. To find this plan, some algorithms are evaluated and the cheapest is returned to the calling predicate.

*/
maBestPlan(Path, Costs) :-
	retractall(maResult(_, _, _, _, _)),
	retractall(maCurrentBestPlan(_, _, _)),
	retractall(maNLOPTResult(_)),
	retractall(useModifiedDijkstra), % Just to be sure

	getMemoryTestList(MemoryList),
	(MemoryList \= [] ->
		maBestPlan(MemoryList, _, _)	
	;
		true	
	),
  (testStrategy(staticMaxDivOpCount) ->
		evalBestPathByStaticMaxDivOpCount(_, _)
	;
		true
	),
  (testStrategy(staticMaxDivPOG) ->
		evalBestPathByStaticMaxDivPOG(_, _)
	;
		true
	),
	(testStrategy(modifiedDijkstra) ->
		evalBestPathByModifiedDijkstra(_, _)
	;
		true
	),
  (testStrategy(staticEqual(SEMiB)) ->
    evalBestPathByStaticEqual(SEMiB, _, _)
  ;
    true
  ),
	% Note that we don't need to check whether the upper paths are better. The
	% best plan is always within the enumerated paths.
	(testStrategy(enumeratePaths) ->
		ensure(evalBestPathByEnumeration(_, _))
	;
		true
	),
	ensure((
		% obtain & set the best path
	 	maCurrentBestPlan(Path, Costs, MemoryValues),
		clearMemoryValues,
		assertzall(MemoryValues),
		ground(Path),
		writeMaResults
	)).

/*
Precodition: A already created POG.
*/
evalBestPathByStaticMaxDivOpCount(RPath, NewCosts) :-
  assignCosts, % The costEdges are needed right now.
  write('Computes the max amount of memory consuming operators, '),
  write('this may take a while...\n'),
  getTime(getMaxMemoryConsumingOperatorsFromPOG(OPCount), Time1),
  secondoGlobalMemory(GMemory),
  MiB is floor(GMemory / OPCount),
  write_list(['\nMemory per Operator: ', MiB, '\n']),
	!,
	getTime(maBestPlanStatic(MiB, RPath, NewCosts), Time2),
	Time is Time1+Time2,
	setBestPlanIfCheaper(RPath, NewCosts),
	term_list_concat([staticMaxDivOpCount(OPCount), ' ', MiB, ' MiB'], '', Label),
  maLogResult(Label, RPath, NewCosts, Time).

/*
Precodition: A already created POG.
*/
evalBestPathByStaticMaxDivPOG(RPath, NewCosts) :-
	highNode(H), 
	X is round(log(H+1)/log(2)), % refer to the pog creation predicates
  write_list(['Path length within the pog: ', X, '\n']),
  secondoGlobalMemory(GMemory),
  MiB is round(GMemory / X),
  write_list(['Memory/Operator: ', MiB]), nl,
  !,
  getTime(maBestPlanStatic(MiB, RPath, NewCosts), Time),
  setBestPlanIfCheaper(RPath, NewCosts),
  term_list_concat([staticMaxDivPOG(X), ' ', MiB, ' MiB'], '', Label),
  maLogResult(Label, RPath, NewCosts, Time).

evalBestPathByModifiedDijkstra(Path, Costs) :-
  getTime(bestPathByModifiedDijkstra(Path, Costs), Time),
  maLogResult(modifiedDijkstra, Path, Costs, Time),
	setBestPlanIfCheaper(Path, Costs),
	!.

evalBestPathByStaticEqual(MiB, Path, Costs) :-
  getTime(maBestPathByStaticEqual(MiB, Path, Costs), Time),
  maLogResult(staticEqual, Path, Costs, Time),
	setBestPlanIfCheaper(Path, Costs),
	!.

evalBestPathByEnumeration(SPath, SCosts) :-
  getTime((once(bestPathByEnumeration) ; true), OptTime),
  currentShortestPath(SPath, SCosts, MemoryValues, Formula), % Obtain path

	% Restore for debug processing within test.pl
	retractall(formulaList(_)), 
	assertz(formulaList(Formula)),

  clearMemoryValues,
  assertzall(MemoryValues),
  maLogResult(enumerate, SPath, SCosts, OptTime),
  getCounter(maPathCounter, PC),
  getCounter(maOptPathCounter, OPC),
  dm(ma, ['\n', PC, ' paths enumerated and ', OPC, ' optimized.']),
	setBestPlanIfCheaper(SPath, SCosts),
	!.

/*
Save the path if this path is cheaper as the stored one.
*/
setBestPlanIfCheaper(Path, Costs) :-
	\+ maCurrentBestPlan(_, _, _),
	!,
	getMemoryValues(MemoryValues),
	assertz(maCurrentBestPlan(Path, Costs, MemoryValues)).

setBestPlanIfCheaper(Path, Costs) :-
	maCurrentBestPlan(_CPath, CCosts, _CMemoryValues),
	(Costs < CCosts ->
		(
			getMemoryValues(MemoryValues),
			retractall(maCurrentBestPlan(_, _, _)),
			assertz(maCurrentBestPlan(Path, Costs, MemoryValues))
		)
	;
		true
	),
	!.

/*
Returns a list with all current memoryValue facts.
*/	
getMemoryValues(MemoryValues) :-
	findall(A, (
			memoryValue(B,C,D),
			A=memoryValue(B,C,D)
		), MemoryValues).

/*
Runs the regular dijkstra algorithm for different initial memory values and chooses the shortest path.
*/
maBestPlan([MiB], RPath, NewCosts) :-
	!,
	getTime(maBestPlanStatic(MiB, RPath, NewCosts), Time),
	setBestPlanIfCheaper(RPath, NewCosts),
	atomic_list_concat([static, ' ', MiB, ' MiB'], '', Label),
  maLogResult(Label, RPath, NewCosts, Time).

maBestPlan([MiB|Rest], Path, Costs) :-
	!,
	maBestPlan([MiB], _Path1, _Cost1),
	maBestPlan(Rest, _Path2, _Cost2),
	% Return now the shorter path, this works here only because there
	% are no previous evaluations.
	maCurrentBestPlan(Path, Costs, _MemoryValues).

maBestPlanStatic(MiB, RPath, NewCosts) :-
  % SSP=single shortest path
  dm(ma, ['\nCompute SSP with ', MiB, ' MiB Memory per Operator...']),
  setStaticMemory(MiB),
  clearMemoryValues,
  assignCosts, 
	!,
  highNode(N),
  dijkstra(0, N, Path, Cost),
	ensure((
  	dm(ma, ['\n\tCosts before memory optimization: ', Cost]),
  	% Note: At this point, we might have more memory granted as we have.
  	% The following memory optimization process can corrects it.
  	pathMemoryOptimization(Path, RPath),
		% Not implemented here is to stop the search for an optimum path if 
		% there is a path that has enough memory. But this can only be done
		% if the assigned memory is never smaller within the path as the
		% sufficient memory value.
  	getCostsFromPath(RPath, NewCosts),
  	dm(ma, ['\n\tCosts after memory optimization: ', NewCosts]),
  	dm(ma, ['\n\tPath: ', RPath])
	)).

bestPathByModifiedDijkstra(RPath, NewCosts) :-
  % The MiB Value is not really needed. It is just used
  % during the costEdge creating phase to supply a value.
	MiB=16, 
  asserta(useModifiedDijkstra),
  dm(ma, ['\nCompute SSP:Modified Dijkstra with ', MiB, 
		' MiB Memory per Operator...']),
  setStaticMemory(MiB),
  clearMemoryValues,
  assignCosts,
  !,
  highNode(N),
  dijkstra(0, N, Path, Cost),
	% Reoptimises the path to include the blocking operators
  retractall(useModifiedDijkstra),
  ensure((
    dm(ma, ['\n\tCosts before memory optimisation: ', Cost]),
    % Note: At this point, we might have more memory granted as we have.
    % The following memory optimization process corrects it.
    pathMemoryOptimization(Path, RPath),
    % Not implement here is to stop the search for an optimum path if 
    % there is a path that has enough memory. But this can only be done
    % if the assigned memory is never smaller within the path as the
    % sufficient memory value.
    getCostsFromPath(RPath, NewCosts),
    dm(ma, ['\n\tCosts after memory optimisation: ', NewCosts]),
    dm(ma, ['\n\tPath: ', RPath])
  )).

/*
Strategy to "compare" some results.
Tries to adopt the strategy that the entire memory is distributed equally
over all memory using operators. 
This is needed because within the standard costs and improved costs
are the costs for some operators not available (itHashJoin etc.).
*/
maBestPathByStaticEqual(MiB, PathNew, CostsNew) :-
  % SSP=single shortest path
  dm(ma, ['\nCompute SSP with ', MiB, ' MiB Memory per Operator...']),
  setStaticMemory(MiB),
  clearMemoryValues,
  assignCosts,
  !,
  highNode(N),
  dijkstra(0, N, Path, Costs),
  ensure((
  	dm(ma, ['\nCompute Memory distribution...']),
		newOpt,
		analyzePath(Path), % Just a dirty solution to get knowledge about	
		% memory using operators.
		formulaList(FList),
		length(FList, Ops),
		(Ops > 0 ->
			(
				secondoGlobalMemory(GMemory),
 				minimumOperatorMemory(MinOpMem),
				OPMem is round(max(MinOpMem, GMemory/Ops)),
				listFix(Ops, OPMem, AMem),
				assignMemory(AMem),
 				% recompute and get the new costs
				recomputePathCosts(Path, PathNew),
    		getCostsFromPath(PathNew, CostsNew),
				dm(ma, ['\n\tMemory values: ', AMem, ' Costs: ', CostsNew])
			)
		;
			(
				PathNew=Path,
				CostsNew=Costs
			)
		)
  )).

/*
Path enumeration strategy
Guarantees to find the best path based on the given cost functions and apart from the different rounding issues.
*/
bestPathByEnumeration :-
  dm(ma, ['\nEnumerate all possible paths. This might take a while...']),
	resetCounter(maPathCounter),
	resetCounter(maOptPathCounter),
	secondoGlobalMemory(GMemory),
/* 
Just set a value, no matter which value as the later optimization tries every possibilities. 
*/
  setStaticMemory(GMemory),
  clearMemoryValues,
  assignCosts, 
  highNode(N), 
	retractall(currentShortestPath(_, _, _, _)),
	!,
	enumeratePaths(Path, N, Cost),
	% The other ensure predicates are more or less for error tracking
	% but this ensure is much more important as in the other places where I used 
	% it. As we are working with backtracking, an error in the below
	% sequence would most likely occur for every loop. So error tracking
	% would be very difficult if no exception is thrown here in case of 
	% an error.
	ensure(once(processResult(Path, Cost))),

	% If a path is found here which has enough memory, we still need to 
	% continue the search. This is different as to use the
	% shortest path algorithm. We still might find a path
	% that has not enough memory, but nevertheless less costs.
	fail. % force backtracking to obtain the next path. 

processResult(Path, Costs) :-
	nextCounter(maPathCounter, _),
  dm(ma3, ['\n\tCosts before memory optimization: ', Costs]),
	% Note: At this point, we might have more memory granted as we have.
 	% The following memory optimisation process will correct this. 
	nextCounter(maOptPathCounter, _),
 	pathMemoryOptimization(Path, RPath),
 	getCostsFromPath(RPath, NewCosts),
 	dm(ma3, ['\n\tCosts after memory optimization: ', NewCosts]),
 	dm(ma3, ['\n\tPath: ', RPath]),
 	storePathIfNeeded(RPath, NewCosts).

/*
This predicates are enumeration related, it is not the same
as setBestPlanIfCheaper/2.
*/
isShorter(Costs) :-
	currentShortestPath(_, SCosts, _, _),
	Costs < SCosts.
isShorter(_) :-
	\+ currentShortestPath(_, _, _, _).

/*
Used to maintain the results after backtracking.
*/
storePathIfNeeded(Path, Costs) :-
	(isShorter(Costs) ->
		(
			% The new path is shorter as the current stored path or there
			% is no stored path yet.
			retractall(currentShortestPath(_, _, _, _)),
			getMemoryValues(MemoryValues),
			formulaList(Formula), % Store for debugging
			asserta(currentShortestPath(Path, Costs, MemoryValues, Formula))
		)
	;
		true % the given path is not shorter, try the next one.
	).

% Contains the so far shortest path.
:- dynamic currentShortestPath/4.


/*
Enumerates all costEdge paths within the POG trough backtracking.

You can inspect the enumerated paths, normally, by executing a sql statemant and then call:
highNode(N), enumeratePaths(Path, N, Costs), (plan(Path, Plan)-> plan_to_atom(Plan, A);true), write_list(['\n: ', Costs, ' -> ', A]), fail.
If you remove the ->, you need to use once/1 to avoid nasty backtracking.
*/
enumeratePaths(Path, N, Costs) :-
	enumeratePaths([], 0, N, Path, Costs).

enumeratePaths(Path, N, N, Path, 0).

enumeratePaths(PartPath, Node, N, RPath, RCosts) :-
	Node\=N,
	costEdge(Node, Target1, A, B, C, ECosts),
	E=costEdge(Node, Target1, A, B, C, ECosts),
	append(PartPath, [E], Path),
	enumeratePaths(Path, Target1, N, RPath, Costs),
	RCosts is ECosts + Costs.

/*
This is done to distribute the entire memory equally over all memory consuming operators. Due to different paths, with differnt numbers of memory consuming operators, within the POG, this is not that simple. So in absent of a good strategy to get a good gussed value, all paths are enumerated. Note that this value is just a maximum. Not every path uses this number of memory consuming operators. Due to the enumeration of every path, which is to evaluate the various possible strategies.
*/
getMaxMemoryConsumingOperatorsFromPOG(OPCount) :-
	resetCounter(maxMemoryUsingOperators),
	(getMaxMemoryConsumingOperatorsFromPOG2;true), 
	% Note that this memoryUsingOperator counter is just added because
	% it is a littlemore complicated to compute the number of memory using
	% operators.
	getCounter(maxMemoryUsingOperators, OPCount1),
	OPCount is max(OPCount1, 1), % 1 means the entire memory can be assigned
	% to one operator.
	dm(ma, ['\nMax memory consuming operators within a path: ', OPCount]).

getMaxMemoryConsumingOperatorsFromPOG2 :-
	highNode(N),
	!, 
	enumeratePaths(Path, N, _),
	ensure((
 		resetCounter(memoryUsingOperators),
		recomputePathCosts(Path, _), 
  	getCounter(memoryUsingOperators, OpCount),
		getCounter(maxMemoryUsingOperators, CurrentMaxOpCount),
		Max is max(OpCount, CurrentMaxOpCount),
		resetCounter(maxMemoryUsingOperators, Max)
	)),
	fail.

/*

ceCosts = CostEstimation Costs

Gets the costs from the CostEstimation implementation.

This method with the alist is not very efficient, but it can be easily extended without taken care of every predicates using it.
	
ceCosts(+OpName, +CardX, +SizeX, +MiB, -CostsInMS, -AList)
*/
ceCosts(OpName, CardX, SizeX, MiB, CostsInMS, AList) :-
	ensure((number(CardX), number(SizeX))),
	% The getCosts API expects integer values.
	ICardX is round(CardX),
	ISizeX is round(SizeX),
  % Very nasty signature handling
	% An operator is non-memory sensitiv if the maOpSig fact
	% doesn't exist or getOpIndexes fails.
  maOpSig(OpName, Sig),
  getOpIndexes(OpName, Sig, _ResultType, AlgID, OpID, FunID),
  % Whatever amount of memory we choose here...we still need the values for
  % the plan generation.
  %getCosts(AlgID, OpID, FunID, ICardX, ISizeX, MiB, OpCostsInMS),

  getCostFun(AlgID, OpID, FunID, ICardX, ISizeX, FT, DList),
	checkDList(DList),
	AList=[functionType(FT), dlist(DList)],
	% Store for debugging
	PARAMS=params(cardX(CardX), sizeX(SizeX)),
	AList=[functionType(FT), dlist(DList), PARAMS],

 	minimumOperatorMemory(MinOpMem),
	DList=[SufficientMemoryInMiB|_],
	buildFormulaByFT(FT, DList, CMiB, CostF),
  CMiB is max(MinOpMem, min(MiB, SufficientMemoryInMiB)),
	CostsInMS is CostF,
	checkOpCosts(CostsInMS).

%ceCosts(+OpName, +CardX, +SizeX, +CardY, +SizeY, +MiB, -CostsInMS, -AList)
ceCosts(OpName, CardX, SizeX, CardY, SizeY, MiB, CostsInMS, AList) :- 
	ensure((number(CardX), number(SizeX), number(CardY), number(SizeY))),
	% The getCosts API expects integer values.
	ICardX is round(CardX),
	ISizeX is round(SizeX),
	ICardY is round(CardY),
	ISizeY is round(SizeY),
  % Nasty signature handling
  maOpSig(OpName, Sig),
  getOpIndexes(OpName, Sig, _ResultType, AlgID, OpID, FunID),
  % Whatever amount of memory we choose here...we still need the values for
  % the plan generation.
  getCostFun(AlgID, OpID, FunID, ICardX, ISizeX, ICardY, ISizeY, FT, DList),
	checkDList(DList),
	% Store for debugging
	PARAMS=params(cardX(CardX), sizeX(SizeX), cardY(CardY), sizeY(SizeY)),
	AList=[functionType(FT), dlist(DList), PARAMS],

 	minimumOperatorMemory(MinOpMem),
	DList=[SufficientMemoryInMiB|_],
	buildFormulaByFT(FT, DList, CMiB, CostF),
  CMiB is max(MinOpMem, min(MiB, SufficientMemoryInMiB)),
	CostsInMS is CostF,
	checkOpCosts(CostsInMS).

/*

The opCosts predicate supports either to compute the costs on static
memory values which are assigned to every memory consuming operator or
based on an explicit assigned memory amount. 

*/
% Non-join operator
opCosts(MT, Term, [CardX, SizeX], OpCostsInMS, NewTerm) :-
  var(MT),
  Term=..[Op|_],
  getStaticMemory(MiB),
  ceCosts(Op, CardX, SizeX, MiB, OpCostsInMS, AList),
  toMemoryTerm(Term, MiB, AList, NewTerm),
  nextCounter(memoryUsingOperators, _).

% Join operator
opCosts(MT, Term, [CardX, SizeX, CardY, SizeY], OpCostsInMS, NewTerm) :-
  var(MT),
  Term=..[Op|_],
  getStaticMemory(MiB),
  ceCosts(Op, CardX, SizeX, CardY, SizeY, MiB, OpCostsInMS, AList),
  toMemoryTerm(Term, MiB, AList, NewTerm),
  nextCounter(memoryUsingOperators, _).

% Second cost call with stored memory term.
opCosts(MT, Term, _, OpCostsInMS, Term) :-
  nonvar(MT),
  % In this case, the costs were calculated at least one time earlier.
  % The new memory value is obtained from the memoryValue term 
  % and costs are recomputed.
  % Of course, the other variables like row size and
  % row cards, are non-changing as assumed.
  ensure((
    MT=memory(_, MID, AList), % If not found, there is something wrong.
    memoryValue(MID, MiB, _) % Calculate on the new given memory value.
  ), 'Error: memoryValue missing'),
  propertyValue(functionType, AList, FT),
  propertyValue(dlist, AList, DList),
  DList=[SuffMiB|_], % Don't use more memory as needed for the cost calculation.
  minimumOperatorMemory(MinOpMem), % And not less than this value.
  buildFormulaByFT(FT, DList, CMiB, CostF),
  CMiB is max(MinOpMem, min(MiB, SuffMiB)),
  OpCostsInMS is CostF,
	checkOpCosts(OpCostsInMS),
  dm(ma6, ['\nComputed costs: ', OpCostsInMS]),
  nextCounter(memoryUsingOperators, _).

opCosts(MT, Term, _, OpCostsInMS, NewTerm) :-
  throw(error_Internal(ma_opCosts(MT, Term, _, OpCostsInMS, NewTerm)
    ::'failed to compute the costs')).

/*
Due to frequent errors, some values are explicit checked here.
*/
checkOpCosts(OpCostsInMS) :-
	(OpCostsInMS>=0 ->
		true
	;
		(
			atomic_list_concat(['Negative costs are not allowed, ', 
				'but a function computed negative costs. ',
				'There is something wrong.'], Msg),
  		throw(error_Internal(ma_checkOpCosts(OpCostsInMS)::Msg)) 
		)
	).

checkDList([]) :- 
	!.

checkDList([A|Rest]) :- 
	checkDList(Rest),
  (maIsNumber(A) ->
    true
  ;
    (
      atomic_list_concat(['A value within the dlist-list is not a number: ',
				A, '. This is not allowed, there is something wrong.'], Msg),
      throw(error_Internal(ma_checkDList([A|Rest])::Msg))
    )
  ).

/*
Checking a value for '$NaN' is a problem.
number('$NaN') succeeds.
Hence, the check is done with catch(_ is N+N, _, fail).

refer to:
http://comments.gmane.org/gmane.comp.ai.prolog.swi/7558
*/
maIsNumber(N) :-
	number(N), 
	catch(_ is N+N, _, fail).

/*
The idea is to change the assigned memory value dynamically because later changes within the path are much more expensive to perform.
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

/*
The nonlinear optimization for a given path.
*/
pathMemoryOptimization(Path, RPath) :-
	ensure((
		newOpt,
		analyzePath(Path),
		buildFormula(MIDS, Vars, F, SufficientMemoryInMiB),
		createGlobalMemoryConstraints(MIDS, Path, CList),
		setFormula(Vars, F),
		dm(ma3, ['\nFormula: ', F]),
  	length(Vars, Len), % Len is the number of memory consuming operators.
  	secondoGlobalMemory(GlobalMemory),
  	doMemoryOptimization(Path, GlobalMemory, CList, Len, 
			SufficientMemoryInMiB, _Result, RPath)
	)).

/*
doMemoryOptimization(+Path, +Memory, +MaxMemoryConstraints, +Len, 
	+SufficientMemoryInMiB, -Result, -RPath)
*/
doMemoryOptimization(Path, _, _, 0, _, _, Path) :-
	% Hence, there is nothing more to do
	dm(ma, ['\nNo memory optimisation needed: no memory using operators.']),
	!.

doMemoryOptimization(Path, Memory, CList, _Len, SufficientMemoryInMiB, 0, 
		RPath) :-
	minimumOperatorMemory(MinOpMem),
	listMax(MinOpMem, SufficientMemoryInMiB, MemoryValues),
	listSum(MemoryValues, Sum),
	Sum =< Memory,
	dm(ma, ['\nNo memory optimization needed: enough memory available.']),
	dm(ma, ['\n\tMemory values: ', MemoryValues]),
	restDistribution(CList, MemoryValues, ARResult),
	assignMemory(ARResult),
	recomputePathCosts(Path, RPath),
	!.

% Not implemented here is the case to avoid the nonlinear optimization
% If the sufficient memory values sum is higher as the global memory but
% with the blocking operators constraints there is enaugh memory. In this
% case it would not needed to execute the optimization.

doMemoryOptimization(Path, Memory, CList, Len, SufficientMemoryInMiB, ARResult, 
		RPath) :-
	Len > 0,
	dm(ma, ['\nStart optimization with max Memory bound: ', Memory, ' MiB...']),
	dm(ma, ['\nSufficient memory values in MiB: ', SufficientMemoryInMiB]),
	dm(ma, ['\nMax memory constraints: ', CList]),
	minimumOperatorMemory(MinOpMem),
	dm(ma, ['\nMinimum operator memory: ', MinOpMem]),
  getTime(
		memoryOptimization(MinOpMem, Memory, CList, Len, SufficientMemoryInMiB, 
			Result), % C predicate, see MemoryOptimization.cpp
		TimeMS),
	dm(ma, ['\nOptimization result: ', Result]),
	dm(ma, ['\nComputed in: ', TimeMS, 'ms']),
	
	% Just store this result, but it is not needed for this program.	
	retractall(maNLOPTResult(_)),
	assertz(maNLOPTResult(Result)),

	optimizeResultToAssignableResult(Result, AResult),
	restDistribution(CList, AResult, ARResult), % This might happen when constant
	% functions are involved.
	% Assign the new memory values
	assignMemory(ARResult),
	% and the costs are changed...
	% So we need to update the costs within the costEdge facts of the given path.
	% If you need the costs within the POG you need to call createCostEdges again.
	recomputePathCosts(Path, RPath),
	!.

doMemoryOptimization(Path, Memory, CList, Len, SufficientMemoryInMiB, Result, 
		Path) :-
	Msg='Memory optimization failed.',
	throw(error_Internal(ma_doMemoryOptimization(Path, Memory, CList, Len, 
		SufficientMemoryInMiB, Result, Path)::Msg)).

/*
Rounding the Results because the secondo kernel expects integer values.
The result in ~Result~ may assign a little more memory than allowed
due to precision and the choosen algorithm. So either
this is allowed or it is needed to apply an algorithm to 
correct it. For example it might be an option to subtract the too much
assigned memory from the highest value. But currently it is
no problem to grant a little more memory, as the Secondo kernel
won't forbid it.
*/
optimizeResultToAssignableResult(OResult, AResult) :-
	forceCeilingListToInts(OResult, AResult),
	%ceilingListToInts(OResult, AResult),
	%roundListToInts(Result, CResult), % If it should be more accurate.
	dm(ma, ['\nRounded result: ', AResult]).

/*
Because earlier all sufficient memory values are floored, the  
values are here ceil-ed. But note the special case: 
if the optimization result is an integer, and because the value 
was floored earlier, 1 need to be added, otherwise it might happen 
that the operator gets not enough memory. 
*/
forceCeilingListToInts([], []).
forceCeilingListToInts([F|FRest], [I|IRest]) :-
	(integer(F) ->
		I is F+1
	;
  	I is ceiling(F)
	),
  forceCeilingListToInts(FRest, IRest).

/*
Distributes the unused memory equally over all operators.
*/
:- dynamic(restDistribution/1).
restDistribution(equal).

setRestDistribution(Mode) :-
	member(Mode, [equal, none]),
	retractall(restDistribution(_)),
	assertz(restDistribution(Mode)).

restDistribution(_CList, AResult, AResult) :-
	restDistribution(none),
	!.

restDistribution(CList, AResult, ARResult) :-
	restDistribution(equal),
	!,
	(CList = [_] ->
		true
	;
		% This is that easy, because a operator at a edge may occur
		% within other shelfs. Experience shows that the only blocking
		% operator sortmergejoin is nearly never used. Hence, i did
		% not implemented this. Nevertheless...rest distribution is just
		% for error fixing all the previous assumptions and approximations.
		% But if ever needed, this should be integrated into the nonlinear
		% optimization problem.
		dm(ma, ['\nMulit-shelf rest distribution currently not implemented.'])
	),
  formulaList(List),
	listSum(AResult, Sum),
	secondoGlobalMemory(GMemory),
	length(AResult, OpCount),
	Rest is round((GMemory - Sum) / OpCount), % equal distribution
	(Rest > 0 ->
		restDistribution(Rest, List, AResult, ARResult)
	;
		AResult=ARResult
	),
	dm(ma, ['\nRest distribution result: ', ARResult]).

restDistribution(_, [], [], []).
restDistribution(Memory, [_E|Rest], [MiB1|MRest1], [MiB|MRest]) :-
	restDistribution(Memory, Rest, MRest1, MRest),
	MiB is MiB1 + Memory.

/*
Determines the costs from a given path.
*/
getCostsFromPath([], 0).
getCostsFromPath([Edge|Rest], Costs) :-
	Edge=costEdge(_, _, _, _, _, Cost),
	getCostsFromPath(Rest, RestCosts),
	Costs is Cost + RestCosts, 
	!.
getCostsFromPath(Path, Costs) :-
	Msg='Can\'t get costs from path.',
	throw(error_Internal(ma_getCostsFromPath(Path, Costs))::Msg).

/*
If the assigned memory values are changed, the costs are no longer valid so they need to be recomputed. 
*/	
recomputePathCosts([], []).
recomputePathCosts([CEdge|RPath], [NewCEdge|NewPath]) :-
  CEdge=costEdge(Source, Target, Term, Result, _, _),
  edge(Source, Target, EdgeTerm, _, _, _),
  extractPredFromEdgeTerm(EdgeTerm, Pred),
  edgeSelectivity(Source, Target, Sel),
	% this will now respect the new assigned memory values.
  costterm(Term, Source, Target, Result, Sel, Pred, Size, Cost, NewTerm),
  NewCEdge=costEdge(Source, Target, NewTerm, Result, Size, Cost),
	recomputePathCosts(RPath, NewPath).

recomputePathCosts(A, B) :-
	Msg='Can\'t recompute costs from path.',
	throw(error_Internal(ma_recomputePathCosts(A, B))::Msg).

/*
Assign the memory
Note the lose connection with the result list. The order needs to be the same as in the formula list.
*/
assignMemory(CResult) :-
  formulaList(List),
  assignMemory(List, CResult).

assignMemory([], []).
assignMemory([E|Rest], [MiB|MRest]) :-
  propertyValue(mid, E, MID), 
	retract(memoryValue(MID, _, AList)),
	assertz(memoryValue(MID, MiB, AList)),
	dm(ma3, ['\nAssign: ', MiB, ' to ', MID]),
	assignMemory(Rest, MRest).

assignMemory(A, B) :-
	Msg='Failed: assign memory.',
	throw(error_Internal(ma_assignMemory(A, B)::Msg)).

/*
Remove the different stored facts.
*/	
newOpt :-
	retractall(maConstraint(_, _)),
	retractall(maFormula(_, _)),
	retractall(maDerivativeFormula(_, _, _)),
	retractall(formulaList(_)),
	retractall(maNLOPTResult(_)),
	asserta(formulaList([])).

/*
This method inspects the path recursivly. For all operators
with a CostEstimation implementaion, the objective function etc. facts will be
added.
*/
analyzePath([Edge]) :- 
	!,
	Edge=costEdge(_, _, Term, _, _, _),
	% terms are in need to be viewed in reverse order.
	analyzeTerm(Edge, Term).

analyzePath([Edge|Rest]) :- 
	!,
	% Edges must be analyzed in order.
	analyzePath([Edge]),
	analyzePath(Rest).

analyzePath(P) :-
	Msg='Can\'t analyze the path.',
	throw(error_Internal(ma_analyzePath(P)::Msg)).

analyzeListTerm(_Edge, []) :- 
	!.
analyzeListTerm(Edge, [Term|Rest]) :- 
	!, 
	analyzeTerm(Edge, Term),
	analyzeListTerm(Edge, Rest).

/*
analyzeTerm(+Edge, +Term)
*/
analyzeTerm(Edge, Term) :-
	compound(Term),
	Term=..[Functor|Args],
	Functor=memory, % These are added during the assignCosts phase.
	Args=[SubTerm, MID, AList],
	% 
	SubTerm=..[OpFunctor|OpArgs],
	% This asumption is based on how the terms are constructed and plans 
	% are generated.
	opMap(OpFunctor, Map),
	addFormulaByAList(_MIDVAR, MID, AList),
	% Just find the streams for further inspection.
	mapArguments(Map, OpArgs, MappedOpArgs),
	analyzeListTerm(Edge, MappedOpArgs).
	
analyzeTerm(Edge, Term) :-
	compound(Term),
	Term=..[Functor|OpArgs],
	Functor\=memory,
	% We have to inspect the subterms for further memory consuming operators, 
	% but only if the given term contains substreams.
	opMap(Functor, Map),
	mapArguments(Map, OpArgs, MappedOpArgs),
	analyzeListTerm(Edge, MappedOpArgs).
	
/*
For non-compound terms, there is no stream in it and there is no further analysation necessary.
*/
analyzeTerm(_, Term) :- 
	\+ compound(Term).

% This might be okay, but more likely something is wrong, so at least 
% we have to get aware of the problem.
analyzeTerm(Edge, Term) :-
	Msg='Can\'t optimize the term, maybe a opSigMap fact is missing.',
	throw(error_Internal(ma_analyzeTerm(Edge, Term)::Msg)).

:- dynamic formulaList/1.
formulaList([]).

/*
Create a var MIDVAR that represents the amount of memory for this operator.
After the optimisation we can change the memory amount to this
variable value.  
*/
addFormulaByAList(MIDVAR, MID, AList) :-
	dm(ma3, ['\nAdd formula: ', AList]),
	formulaList(List),
	retractall(formulaList(_)),
	append([midVar(MIDVAR), mid(MID)], AList, NewAList),
  append(List, [NewAList], NewList), % The [...] are needed here.
	asserta(formulaList(NewList)).

/*
Based on the formulas for every memory consuming operator, the cost formula for the entire path is built here. Note that only the memory using operators are optmized here, because the total costs are added. This is so far correct, but the computed value is the "optimum", but not the values of our total costs.

Note that all cost functions of the nodes are added.
when we compute the derivatives, we can do this seperatly for every operator.
It is not needed to compute the derivative of the entire total cost function.
buildFormula(-MIDS, -AllVars, -Formula, -SufficientMemory) :-
*/
buildFormula(MIDS, AllVars, F, SufficientMemory) :-
	formulaList(List),
	getAllVars(List, MIDS, AllVars),
	buildFormula(List, AllVars, 0, F, SufficientMemory).
	
buildFormula([], _, _, 0, []) :- !.

buildFormula([E], AllVars, Dimension, F, [SufficientMemoryInMiB]) :-
	!,
	propertyValue(functionType, E, FT),
	propertyValue(dlist, E, AList),
	propertyValue(midVar, E, MVar),
	buildFormulaByFT(FT, AList, MVar, F),
	AList=[DSufficientMemoryInMiB|_],
	% AList contains double values. It is possible to continue with these values,
	% but in the end, only integer values can be assigend to a plan.
	% Hence, the value is already restricted here to the next integer value.
	%SufficientMemoryInMiB is ceiling(DSufficientMemoryInMiB),
	% Update: Now the floor-value is used. Later the result is always
	% rounded up.
	SufficientMemoryInMiB is floor(DSufficientMemoryInMiB),
	% Note: See the differential_calculus.pl for limitations.
	ensure((
		derivate(F, MVar, DX),
		setDerivative(Dimension, AllVars, DX),
		dm(ma3, ['\nMVar: ', MVar, ' D: ', Dimension, ' DX: ', DX, ' F: ', F]) 
	), derviateError).

buildFormula([E|Rest], AllVars, Dimension, F, SufficientMemoryInMiB) :-
	!,
	buildFormula([E], AllVars, Dimension, F1, SM),
	NextDimension is Dimension + 1,
	buildFormula(Rest, AllVars, NextDimension, F2, SMR),
	F = F1 + F2,
	append(SM, SMR, SufficientMemoryInMiB).

/*
Returns a list of all variables that are used by the optimization.
getAllVars(+, -)
*/
getAllVars([], [], []).
getAllVars([E|ERest], [MID|MIDRest], [MVar|MRest]) :-
	propertyValue(midVar, E, MVar),
	propertyValue(mid, E, MID),
	getAllVars(ERest, MIDRest, MRest).

/*
Following, the differnt function types based on the returned values of the getCostFun predicate. Note that, it is done this way because returning arbitrary formula's from the C++ environment isn't that easy as within prolog. At least it was the first idea to support more complex cost functions, even as only simple function will be delivered from the C++ CostEstimation class. But with the below predicates, much more complex functions are supported as long as these are strict and decreasing, more exact, all functions which the choosen optimize algorihm can handle.
buildFormulaByFT(+FunctionType, +DList, +MVar, ?Out)

Parameter MVar:
Allowed values are all values between the minimum operator memory(>=1) and the sufficient memory value. Don't call this predicate with other memory values.
But don't pass MVar to this predicate! It is paramter, but not one with 
a value.
*/

buildFormulaByFT(1, DList, MVar, Out2) :-
  !,
	ensure(var(MVar), 'Error: Parameter MVar is a bounded variable.'),
  % Vars names as within the CostEstimation.h.
  DList=[SufficientMemory, TimeAtSuffMemory, TimeAt16MB|_],
  % The memory can only be the free variable.
  ensure((number(SufficientMemory), number(TimeAtSuffMemory), 
		number(TimeAt16MB))),
  % This approximation is dangerous if the calculation is enabled for 
  % memory assigments below 16 MiB. Because, when we approach 1 MiB the 
  % difference to the real costs might be getting fairly large (if the costs 
	% are not really linear) and this is not respected within the CostEstimation 
	% implementations that return type 1 functions.
	(SufficientMemory =\= 16 ->
		Gradient is (TimeAt16MB-TimeAtSuffMemory)/(SufficientMemory-16)
	;
		Gradient = 0 % Actually in this case it would be better to remove
								 % this operator from the optimization process, but it
								 % would lead to a more complex code, I wanted to 
								 % avoid this.
	),
  TimeAt0MB is TimeAt16MB+(16*Gradient),
  Out=TimeAt0MB-(MVar*Gradient),

	% Some checks...
	minimumOperatorMemory(MinM),
  ((Gradient =:= 0, SufficientMemory > MinM, maWarn(constant)) ->
		% this might be okay, but in general this is not correct.
		write_list(['\n(ma.pl) WARNING: constant memory dependent cost ',
      'function detected: ', Out,
      '. Results may wrong. Please check the cost functions.'])
  ;
    true
  ),
	(Gradient < 0 ->
		(
			(maWarn(increasing) ->
				(
					write_list(['\n(ma.pl) WARNING: increasing memory dependent cost ',
						'function detected: ', Out, '. Fallback to constant fuction. ',
						'Results may wrong. Please check the cost function.']),
					maWarnSleep(1)
				)
			;
				true
			),
			% Fallback to this constant function to allow still a correct
			% optimization.
      Out2=TimeAt16MB
    )
  ;
    Out2=Out
	),
	checkNegative(Out, MVar, SufficientMemory).

buildFormulaByFT(2, DList, MVar, Out2) :- 
	!, 
	ensure(var(MVar), 'Error: Parameter MVar is a bounded variable.'),
	% Vars names as within the CostEstimation.h.
	DList=[SufficientMemory, _TimeAtSuffMemory, TimeAt16MB, A, B|_],
	% Note: _TimeAtSuffMemory = (A/_SufficientMemory)+B is not
	% always true due some complications within the cost estimation
	% implementation. But it should always be "near" the value that is
	% indicated by the cost function. The value is ignored, because
	% nonlinear optimization for non static function are not solvable in general,
	% as far as I know.
	ensure((number(A), number(B))), % The memory can be the only free variable.
	Out=(A/MVar)+B, % MVar will be bound to a value greater or equal MinOpMem.

	% Some checks...
	minimumOperatorMemory(MinM),
  ((A =:= 0, SufficientMemory > MinM, maWarn(constant)) ->
    % this might be okay, but in general this is not correct.
    write_list(['\n(ma.pl) WARNING: constant memory dependent cost ',
      'function detected: ', Out,
      '. Results may wrong. Please check the cost functions.'])
  ;
    true
  ),
  (A < 0 ->
    (
      (maWarn(increasing) ->
        (
          write_list(['\n(ma.pl) WARNING: increasing memory dependent cost ',
            'function detected: ', Out, '. Fallback to constant fuction. ',
            'Results may wrong. Please check the cost function.']),
          maWarnSleep(1)
        )
      ;
        true
      ),
      % Fallback to this constant function to allow still a correct
      % optimization.
      Out2=TimeAt16MB
    )
  ;
    Out2=Out
  ),
	checkNegative(Out, MVar, SufficientMemory).

buildFormulaByFT(FT, DList, MVar, Out) :-
	Msg='Can\'t build formula.',
	throw(error_Internal(ma_buildFormulaByFT(FT, DList, MVar, Out)::Msg)).

/*
Validates if the function may return a negativ cost value because this
is not valid.
*/
checkNegative(Out, MVar, SufficientMemory) :-
  secondoGlobalMemory(GMemory),
	TestMem is min(GMemory, SufficientMemory),
	% This is very important because it is not allowed to bind MVar to
	% a value. I think there is a better way to this...
	replaceVar(Out, MVar, X, OutR),
	X=TestMem,  % MVar=TestMem is not allowed.
	CostsAtMax is OutR,
	((CostsAtMax < 0, maWarn(negative)) ->
		(
			write_list(['\n(ma.pl) WARNING: memory dependent cost function ',
				'with potentially negative costs detected: ', Out, 
				'. Results may wrong. Please check the cost functions.']),
			maWarnSleep(1)
		)
	;
		true
	).

/*
Return which plan arguments are streams.
Note that therefore the plan term needs to match the operator signatures, at least for the first arguments. So the assumption is that the operator's signature is equal to the arguments of the plan terms. If a plan functor does not match an operator name, you have to add manually opSigMap facts.

The reason why this is done is because it is neccessary to analyse the terms within the cost edges. So, either we have to define for every operator which argumentes are substreams to be analyzed by adding operators specific analyze-predicates or by knowing which arguments are substreams. Hence, the information what arguments are substreams, is delivered by this predicate.

Examples:
?- opMap(hashjoin, Map).
Map = [t, t, f, f, f].
?- opMap(symmjoin, Map).
Map = [t, t, f].
?- opMap(count, Map).
Map = [t].
?- opMap(sortLeftThenMergejoin, Map).
Map = [t, t, f, f].
*/
opMap(Op, Map) :-
  opSigMap(Op, Map),
	!.

/*
This won't work for some operators because often PSig can't be a variable.
That's the reason why for these operators the opSigMap facts must be added manually.
*/
opMap(Op, Map) :-
	opSignature(Op, _, PSig, _, _),
	analyzeOpSig(PSig, Map),
	!.

opMap(Op, Map) :-
	% try harder...
	clause(opSignature(Op, _, PSig, _, _), _),
	analyzeOpSig(PSig, Map),
	!.

/*
An error is thrown if there is nothing found for the operator, it might sometimes be appropriate to assume false, but I assume the error rate is higher if there is no exception thrown out.
*/
opMap(Op, Map) :-
	Msg='Signature mapping not found for the given operator.',
	throw(error_Internal(ma_opMap(Op, Map)::Msg)).

analyzeOpSig([P|PRest], [t|Map]) :-
	nonvar(P), % Because P may be a variable, see the hashjoin signature.
	P=[stream|_], 
	!,
	analyzeOpSig(PRest, Map).
analyzeOpSig([_|PRest], [f|Map]) :- !,
	analyzeOpSig(PRest, Map).
analyzeOpSig([], []).

/*
Extract only the relevant arguments depending on the given map. A 't' means true and the argument is taken into the result map. A 'f' means false and, of course, then the argument is not taken into the result. t/f is just used because it is shorter than true or fail/false.

Examples:
?- mapArguments([t], [count(xy)], ML).
ML = [count(xy)].
?- mapArguments([t,t], [feed(x),feed(y)], ML).
ML = [feed(x), feed(y)].
?- mapArguments([t,f], [feed(x),feed(y)], ML).
ML = [feed(x)].
?- mapArguments([f,t], [feed(x),feed(y)], ML).
ML = [feed(y)].
?- mapArguments([f,f], [feed(x),feed(y)], ML).
ML = [].
*/
mapArguments(Map, [], []) :-
  length(Map, MA),
  ensure(MA is 0), % simple check if there is nothing wrong
  !.

mapArguments([Map|MapRest], [Arg|Arguments], MappedList) :-
  Map=t,
  !,
  mapArguments(MapRest, Arguments, Mapped),
  append([Arg], Mapped, MappedList).

mapArguments([Map|MapRest], [_|Arguments], Mapped) :-
  Map=f,
  !,
  mapArguments(MapRest, Arguments, Mapped).

% Additional parameter are treated as 'f'.
mapArguments([], [_|Arguments], Mapped) :-
  !,
  mapArguments([], Arguments, Mapped).

/*
See MemoryAllocation.cpp for more information about the properties the passed formula must fulfil.
*/
:- dynamic 	maFormula/2,
						maDerivativeFormula/3.
setFormula(VarList, Formula) :-
	ensure(is_list(VarList)),
	retractall(maFormula(_, _)),
 	assert(maFormula(VarList, Formula)).

setDerivative(Dimension, VarList, Formula) :-
	ensure((
		integer(Dimension),
		Dimension >= 0
	)),
	retractall(maDerivativeFormula(Dimension, _, _)),
 	assert(maDerivativeFormula(Dimension, VarList, Formula)).

/*
Callback predicate from MemoryOptimization.cpp to compute the costs for given vector of memory assignments.
	
Note that the objective function is stored here as a fact, so there is no need to loopthrough it into the C++ runtime, even if it might not be very efficient. The idea behind this is mainly that we don't want to pass the formula as a string to the C++ environment to evaluate it there, because this is not that simple and it might need another library to do this. On the other hand, it was not very clear in the beginning how complex the cost functions will be. However, with these predicates, even more complex cost functions could be optimized.
*/
objectiveFunction(Vars, Result) :-
	dm(ma6, ['\nCompute objectiveFunction: Vars: ', Vars]),
	maFormula(Vars, Formula),
	Result is Formula, % Evaluate the expression with the bindings from Vars.
	dm(ma6, ['\nCompute objectiveFunction: Result:', Result]).

/*
This is the predicate to compute the gradient. It is called out of the MemoryAllocation.cpp. But note that this depends on the choosen algorithm, some of the alogithm doesn't need a derivation.
*/
derivativeFunction(Dimension, Vars, Result) :-
	dm(ma6, ['\nCompute derivativeFunction: Dimension: ', Dimension, 
		' Vars: ', Vars]),
	maDerivativeFormula(Dimension, Vars, Formula),
  Result is Formula, % Evaluate the expression with the bindings from Vars.
	dm(ma6, ['\nCompute derivativeFunction: Result: ', Result]).

/*

*/
extractPredFromEdgeTerm(EdgeTerm, Pred) :-
  EdgeTerm = select(_, Pred).
extractPredFromEdgeTerm(EdgeTerm, Pred) :-
  EdgeTerm = join(_, _, Pred).
extractPredFromEdgeTerm(EdgeTerm, Pred) :-
  EdgeTerm = sortedjoin(_, _, Pred, _, _).

/*

*/
maSelfCheck :-
  write('\nSelf-Check...'),
  (maOpInfo(quit) ->
    write('passed\n')
  ;
    (
      write('\nFAILED! Please enter \'maOpInfo.\' to track the error\n'),
      sleep(5),
    	delOption(memoryAllocation)
    )
  ).

maWrite(gossipy, List) :-
	!,
	write_list(List).
maWrite(Mode, _List) :- 
	Mode \= gossipy,
	!.

/*
The results of the different strategies are collected within the maResult facts.
*/
:- dynamic maResult/5.

maLogResult(Type, Path, Costs, Time) :-
  plan(Path, Plan),
 	plan_to_atom(consume(Plan), PlanAtom),
  assertz(maResult(Type, Path, PlanAtom, Costs, Time)).

writeMaResults :-
	write('\nMemory allocation results:\n'),
	FH='~w~30+ ~` t~w~20+ ~` t~w~10+ ~n',
	format(FH, ['Algorithm', 'Costs', 'Time']),
	findall([A, B], (
			maResult(A, _, _, B, Time), 
			F='~w~30+ ~` t~f~20+ ~` t~d~10+ms ~n',
			format(F, [A, B, Time])
		), _).

/*
Just write some information about the optimizing process.
*/
maInfo :-
  write('*** FACTS ***\n'),
	writefacts(maConstraint),
	writefacts(memoryValue),
	writefacts(formulaList),
	writefacts(maStrategiesToEval),
	writefacts(staticMemory),
	writefacts(maFormula),
	writefacts(maDerivativeFormula).

maDebug :-
	debugLevel(ma),
	debugLevel(ma3).
	
/*
Writes some operators to stdout that have a CostEstimation implementation.
*/
maOpInfo :-
	maOpInfo(gossipy).

maOpInfo(Mode) :-
	maOpSig(OpName, Sig),
	ensure(opMap(OpName, Map)),
	maWrite(Mode, ['\n\nOperator: ', OpName, '\n\tSignature: ', Sig, 
		'\n\tMap: ', Map]),
	(isBlockingOperator(OpName, Sig) ->
		maWrite(Mode, ['\nBlocking op: yes'])
	;
		maWrite(Mode, ['\nBlocking op: no'])
	),
 	(getOpIndexes(OpName, Sig, _ResultType, AlgID, OpID, FunID) ->
		(getCostFun(AlgID, OpID, FunID, 1000000, 100, 2000000, 250, FT, DList) ->
			(
				maWrite(Mode, ['\nFunction type: ', FT, ' DList: ', DList])
			)
		;
			(
				maWrite(Mode, ['\nWARNING: failed to receive cost function.']),
				nextCounter(maFailedOps, _)
			)
		)
	;
		(
			maWrite(Mode, ['\nWARNING: failed to receive OpIndexes.']),
			nextCounter(maFailedOps, _)
		)
	),
	fail.

maOpInfo(_Mode) :-
	getCounter(maFailedOps, FailedOps),
	resetCounter(maFailedOps),
	FailedOps=0.

/*
Constructs a list that show the sufficient memory value and the assigned
memory values.
The result list is in the same order as the constructed formula.
*/
maMemoryInfo(InfoTerm) :-
	formulaList(F),
	% warning: this might come from another optimization.
	% it is needed to check here manually if this belongs to "this" 
	% optimization.
	(maNLOPTResult(OPT) -> true ; (length(F, FLen), length(OPT, FLen))),
	maMemoryInfo(F, OPT, InfoTerm),
	!.
	
maMemoryInfo([], [], []) :-
	!.

maMemoryInfo([F|FRest], [O|OPTRest], [I|IRest]) :-
	maMemoryInfo(FRest, OPTRest, IRest),
	member(mid(MID), F),
	member(dlist(DList), F),
	DList=[SuffMemory|_],
  memoryValue(MID, Memory, _),
	I=[SuffMemory, O, Memory],
	!.

/*
unused
*/
cListSum([], [], 0) :-
	!.
cListSum([_MiB|Rest], [0|CRest], Sum) :-
	!,
	cListSum(Rest, CRest, Sum).
cListSum([MiB|Rest], [1|CRest], Sum2) :-
	!,
	cListSum(Rest, CRest, Sum),
	Sum2 is MiB+Sum.

/*
Disables or enables the warnings because for big pog's the amount of
displayed warnings can be huge. The same thing is the case for the sleep
thing below.
*/
maWarnMode(true).

maWarn(_Type) :-
	maWarnMode(R),
	!,
	R.

/*

*/
warnSleep(no).

maWarnSleep(_S) :-
	warnSleep(no), 
	!.

maWarnSleep(S) :-
	sleep(S),
	!.

% eof
