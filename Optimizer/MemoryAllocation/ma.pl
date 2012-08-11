/*

$Header$
@author Nikolai van Kempen

Provides a extension to the secondo optimizer to optimize the memory allocation of operators within a query.

*/

% Not declared as dynamic within the optimizer.
:- dynamic costEdge/6. 

/*
This predicate makes the GlobalMemory property from the SecondoConfig.ini available within prolog.
Unit: MiB
secondoGlobalMemory(?Memory)
*/
secondoGlobalMemory(Memory) :- 
	predicate_property(secondo_global_memory(_), foreign), 
	!, 
	secondo_global_memory(Memory).

% Fallback to the current default value.
secondoGlobalMemory(512).

/*
Every operator gains at least this amount of memory. But this extension
is able to compute solutions for other values. But the absolute lowest values is one MiB, refer to other comments why this lower limit can't be less then one megabyte.
Unit: MiB
*/
minimumOperatorMemory(16).

/*
We can find a good memory-optmized plan with different stategies.

Strategy 'static':
The simplest one is to give every operator a fixed amount of memory.
This no real optimisation strategy, but allows comparsions to the normal behaviour. Note that there is still a difference between the usauall behaviour and the current version because the cost will be calulated by the CostEstimation class.

Strategy 'staticDiv':
A little smarter is now to distriubute the entire memory over all memory consuming operators. Note that this number of memory consuming operators is only a estimated value because we don't now that the path the shorted will be, one path may have more memory consuming opertors as another path.

Choose a path on the upper strategies and then try optimize it with there memory-depending cost functions.

*/
:- 
	dynamic(maStrategiesToEval/1),
	secondoGlobalMemory(Max), % This is secondo default value
	List=[
		static(16), 
		static(Max), 
		modifiedDijkstra,
		% These are totally unsuitable for more then 5 predicates and a high
		% number of paths. They are just implemented to evaluate results.
		staticMaxDivOpCount,
 		enumeratePaths
	],
	retractall(maStrategiesToEval(_)), % support reloading.
	asserta(maStrategiesToEval(List)).

/*
Returns the amount of memory that is used to compute the costs for memory
using operators during cost edge creation.
Unit: MiB
*/
:- dynamic staticMemory/1.
staticMemory(16).

getStaticMemory(MiB) :-
	staticMemory(MiB), !.

getStaticMemory(MiB) :-
	!, 
	Msg='The staticMemory fact is missing',
	throw(error_Internal(ma_getStaticMemory(MiB)::Msg)).

setStaticMemory(MiB) :-
	ensure(integer(MiB)),
	retractall(staticMemory(_)),
	asserta(staticMemory(MiB)).

/*
Choose an appropriate amount of memory that is assigned to every operator. The idea behind this is to find, with this memory assignment, a path that is after the memory optimization the shortes path.
Precodition: A already created POG.
*/
getMemoryTestList(MList) :-
  maStrategiesToEval(List),
	
  findall(MiB, (member(A, List), A=static(MiB)), MList1),

	(testStrategy(staticMaxDivOpCount) ->
		(
  		assignCosts, % Even if we recall this later, we need this now.
			% could improved, but it doens't seems worth it.
			write('Compute max memory consuming operators, this may take a while...'),
			nl,
			getMaxMemoryConsumingOperatorsFromPOG(OPCount),
			write_list(['getMaxMemoryConsumingOperatorsFromPOG: ', OPCount]), nl,
			secondoGlobalMemory(GMemory),
		 	Memory is integer(GMemory / OPCount),
			append(MList1, [Memory], MList)
		)
	; 
		MList=MList1
	).

/*
Provable if strategy S should be evaluated.
*/
testStrategy(S) :-
  maStrategiesToEval(List),
	member(S, List).

:- dynamic useModifiedDijkstra/0.

/*
Here we try to find the best plan regarding the optimal memory allocation for the memory using operators. This is not a simple task at all because there sees to be no such algorithm that computes the shortest path for edges with nonlinear cost functions.

*/
maBestPlan(Path, Costs) :-
	retractall(maResult(_, _, _, _)),
	retractall(useModifiedDijkstra),

	getMemoryTestList(MemoryList),
	maBestPlan(MemoryList, Path1, Costs1),
	getMemoryValues(MemoryValues1),

	(testStrategy(modifiedDijkstra) ->
		(
			asserta(useModifiedDijkstra),
			maBestPlan(MemoryList, Path2, Costs2),
			retractall(useModifiedDijkstra),
			chooseShorterResult(Costs2, Costs1, Path2, Path1, MemoryValues1, 
				Costs3, Path3)
		)
	;
		(Costs3=Costs1, Path3=Path1)
	),

	(testStrategy(enumeratePaths) ->
		(
			ensure((
				getTime((bestPathByEnumeration ; true), OptTime),
				currentShortestPath(SPath, SCosts, MemoryValues),
				clearMemoryValues, 
				assertzall(MemoryValues),
				assertz(maResult(enumerate, SPath, SCosts, OptTime)),
				Path=SPath,
				Costs=SCosts, 
				getCounter(maPathCounter, PC),
				getCounter(maOptPathCounter, OPC),
  			dm(ma, ['\n', PC, ' paths enumerated and ', OPC, ' optimized.'])
			))
			% Note that we don't need to check if the upper paths are better, the
			% best plan is always under the enumerated paths.
		)
	;
		(
			Path=Path3,
			Costs=Costs3
		)
	),
	ensure((
		ground(Path),
		writeMaResults
	)).

/*
C1/C2: The Costs
P1/P2: The Paths
Chooses the shorter path, C2/P2/MemoryValues2 need always to be the previoues stored path because only for the P2 the memory values are restored.
chooseShorterResult(+C1, +C2, +P1, +P2, +MemoryValues2, ?RC, ?RP)
*/
chooseShorterResult(C1, C2, P1, _ /*P2*/, MemoryValues2, RC, RP) :-
	C1 < C2,
	RC=C1,
	RP=P1,
	clearMemoryValues, 
	assertzall(MemoryValues2).

chooseShorterResult(C1, C2, _, P2, _, RC, RP) :-
	C1 >= C2,
	RC=C2,
	RP=P2.

chooseShorterResult(C1, C2, P1, P2, MemoryValues2, RC, RP) :-
  Msg='Failed to choose the shorter path',
  throw(error_Internal(ma_chooseShorterResult(C1, C2, P1, P2, MemoryValues2, 
		RC, RP)::Msg)).

/*
The results of the different strategies are collected within the maResult facts.
*/
:- dynamic maResult/4.
writeMaResults :-
	write('\nMemory allocation results:\n'),
	findall([A, B], (
			maResult(A, _, B, Time), 
			write_list(['\n', A, '\t', B, '\t', Time, 'ms'])
		), _).

/*
Just write some information about the optimizing process.
*/
maInfo :-
	writefacts(memoryValue),
	writefacts(maStrategiesToEval),
	writefacts(staticMemory),
	writefacts(maFormula),
	writefacts(maDerivativeFormula).

getMemoryValues(MemoryValues) :-
	findall(A, (
			memoryValue(B,C,D),
			A=memoryValue(B,C,D)
		), MemoryValues).

/*

*/
maBestPlan([MiB], RPath, NewCosts) :-
	getTime(maBestPlan2([MiB], RPath, NewCosts), Time),
	atomic_list_concat([static, ' ', MiB, ' MiB'], '', Label),
	assertz(maResult(Label, RPath, NewCosts, Time)).

maBestPlan([MiB|Rest], Path, Cost) :-
	!,
	maBestPlan([MiB], Path1, Cost1),
	getMemoryValues(MemoryValues1),
	maBestPlan(Rest, Path2, Cost2),
	% Choose now the shorter path
	chooseShorterResult(Cost2, Cost1, Path2, Path1, MemoryValues1, Cost, Path).

maBestPlan2([MiB], RPath, NewCosts) :-
  % SSP=single shortest path
  dm(ma, ['\nCompute SSP with ', MiB, ' MiB Memory per Operator...']),
  setStaticMemory(MiB),
  clearMemoryValues,
  assignCosts, 
	!,
  highNode(N),
  dijkstra(0, N, Path, Cost),
	ensure((
  	dm(ma, ['\n\tCosts before memory optimisation: ', Cost]),
  	% Note: At this point, we might have more memory granted as we have.
  	% The following memory optimisation process will correct this. 
  	optimizeMemoryInPath(Path, RPath),
		% Not implement here is to stop the search for a optimum path if 
		% there is a path that has enough memory. But this can only be done
		% if the assigned memory is nenver small within the path as the
		% sufficient memory value.
  	getCostsFromPath(RPath, NewCosts),
  	dm(ma, ['\n\tCosts after memory optimisation: ', NewCosts]),
  	dm(ma, ['\n\tPath: ', RPath])
	)).

/*
Path enumeration strategy
Guarantees finding the best path based on the given cost functions and apart from the different rouding issues.
*/
bestPathByEnumeration :-
  dm(ma, ['\nEnumerate all possible paths, this might take a while...']),
	resetCounter(maPathCounter),
	resetCounter(maOptPathCounter),
	secondoGlobalMemory(GMemory),
/* 
Just set a value, dosn't matter what value because the later optimization if we try all possibilities. But we try a little smarter strategy to avoid optimizing every path. If the costs are calulated with giving every operator the maximum amount of memory, we can't find a better plan thourgh optimizing if the best costs are already higher than costs of the so far found best path. 
*/
  setStaticMemory(GMemory),
  clearMemoryValues,
  assignCosts, 
  highNode(N), 
	retractall(currentShortestPath(_, _, _)),
	!,
	enumeratePaths(Path, N, Cost),
	% The other ensure predicates are more or less for error tracking
	% but this ensure is much more important as the other places where i used 
	% it. Because we work here with backtracking, an error in the below
	% sequence would most likely occur for every loop. So error tacking
	% would be very difficult if no exception is thrown here in case of 
	% an error.
	ensure(processResult(Path, Cost)),

	% If here a path is found that has enough memory, we still need to 
	% continue the search. This is differently as when we use the
	% shortest path algorithm. The thing is that we still might find a path
	% that has not enough memory, but nevertheless fewer costs.
	fail. % force backtracking to obtain the next path. 

processResult(Path, Costs) :-
	nextCounter(maPathCounter, _),
  dm(ma3, ['\n\tCosts before memory optimization: ', Costs]),

	(isShorter(Costs) -> 
		(
  		% Note: At this point, we might have more memory granted as we have.
  		% The following memory optimisation process will correct this. 
			nextCounter(maOptPathCounter, _),
  		optimizeMemoryInPath(Path, RPath),
  		getCostsFromPath(RPath, NewCosts),
  		dm(ma3, ['\n\tCosts after memory optimization: ', NewCosts]),
  		dm(ma3, ['\n\tPath: ', RPath]),
  		storePathIfNeeded(RPath, NewCosts)
		)
	;
		true
	).

isShorter(Costs) :-
	currentShortestPath(_, SCosts, _),
	Costs < SCosts.
isShorter(_) :-
	\+ currentShortestPath(_, _, _).

/*
Used to maintain the results after backtracking.
*/
storePathIfNeeded(Path, Costs) :-
	(isShorter(Costs) ->
		(
			% The new path is shorter as the current stored path or there
			% is no stored path yet.
			retractall(currentShortestPath(_, _, _)),
			findall(A, (memoryValue(B,C,D), A=memoryValue(B,C,D)), MemoryValues),
			asserta(currentShortestPath(Path, Costs, MemoryValues))
		)
	;
		true % this path is not shorter to try the next one.
	).

% Contains the so far shortest path.
:- dynamic currentShortestPath/3.


/*
Enumerates all costEdge paths within the POG trough backtracking.
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
This is done distribute the entire memory over all memory consuming operators.  Due to different paths, with differnt numbers of memory consuming operators, within the POG this not that simple. So in absent of a good strategy to get a good gussed value, all paths are enumerated. Note that this value is just a maximum, not every path will use this number of memory consuming operators. Due to the enumeration of every path, this is just more to evaluate the different possible strategy, if this strategy is ever considerd to be a good one, this really needs to be optimized.

NOTE: If you are confused why this return for example x and you don't see a optimization with x variables, this is very likely okay. The path with x memory variables is just so bad that it is never considerd to be worth it to be optimized.
*/
getMaxMemoryConsumingOperatorsFromPOG(OPCount) :-
	resetCounter(maxMemoryOp),
	(getMaxMemoryConsumingOperatorsFromPOG2;true), 
	% Note that this memoryUsingOperator couter is just added because
	% it is a little complicate to compute the number of memory using
	% operators.
	getCounter(maxMemoryUsingOperators, OPCount1),
	OPCount is max(OPCount1, 1), % The 1 means the entire memory can be assigned
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

*/
operatorMemSpec(sort,		[t], 				blocking, [[stream,[tuple,[[a,int]]]]]).
operatorMemSpec(sortby,	[t,f], 			blocking, [[stream,[tuple,[[a,int]]]]]).
operatorMemSpec(sortmergejoin, 	[t,t,f,f], 	blocking, 
	[[stream, [tuple, [[a, int]]]], [stream, [tuple, [[b, int]]]], a, b]).
operatorMemSpec(hashjoin, 	[t,t,f,f], 	nonblocking, 
	[[stream, [tuple, [[a, int]]]], [stream, [tuple, [[b, int]]]], a, b]).
operatorMemSpec(mergejoin, 	[t,t,f,f], 	blocking, 
	[[stream, [tuple, [[a, int]]]], [stream, [tuple, [[b, int]]]], a, b]).
operatorMemSpec(loopjoin, 	[t,t], 	nonblocking, 
	[[stream,[tuple, [[a, int]]]], [map,[tuple,[[a, int]]],
		[stream,[tuple,[[b, int]]]]]]).
operatorMemSpec(symmjoin, 	[t,t,f], 	nonblocking, 
	[[stream,[tuple,[[a,int]] ]], [stream,[tuple,[[b,int]]]], 
		[map,[tuple,[[a,int]]],[tuple,[[b,int]]],bool]]).
% Add more operators here if needed.

/*
This method with the alist is not very efficient, but it can easily extended without taken care about every predicates using this.
	
AList is a annotation list with some information we can use later, for example
within the later optimization, then we don't need to requery this information, even if we store now more AList as we need...for the optimisation of the a given path is this to much, but for other alorotihm this might be better. 

opCosts(+OpName, +CardX, +SizeX, +MiB, -CostsInMS, -AList)
*/
opCosts(OpName, CardX, SizeX, MiB, CostsInMS, AList) :-
	ensure((number(CardX), number(SizeX))),
	% The getCosts API expects integer values.
	ICardX is integer(CardX),
	ISizeX is integer(SizeX),
  % Very nasty signature handling
	% A operatoren is non-memory sensitiv if the operatorMemSpec fact 
	% doesn't exisits or getOpIndexes fails.
  operatorMemSpec(OpName, _, _, Sig),
  getOpIndexes(OpName, Sig, _ /*ResultType*/, AlgID, OpID, FunID),
  % Whatever amount of memory we choose here...we still need the values for
  % the plan generation.
  %getCosts(AlgID, OpID, FunID, ICardX, ISizeX, MiB, OpCostsInMS),

  getCostFun(AlgID, OpID, FunID, ICardX, ISizeX, FT, DList),
	AList=[functionType(FT), dlist(DList)],

 	minimumOperatorMemory(MinOpMem),
	DList=[SufficientMemoryInMiB|_],
  CMiB is max(MinOpMem, min(MiB, SufficientMemoryInMiB)),
	buildFormulaByFT(FT, AList, CMiB, CostF),
	CostsInMS is CostF.

%opCosts(+OpName, +CardX, +SizeX, +CardY, +SizeY, +MiB, -CostsInMS, -AList)
opCosts(OpName, CardX, SizeX, CardY, SizeY, MiB, CostsInMS, AList) :- 
	ensure((number(CardX), number(SizeX), number(CardY), number(SizeY))),
	% The getCosts API expects integer values.
	ICardX is integer(CardX),
	ISizeX is integer(SizeX),
	ICardY is integer(CardY),
	ISizeY is integer(SizeY),
  % Very nasty signature handling
  operatorMemSpec(OpName, _, _ /*BType*/, Sig),
  getOpIndexes(OpName, Sig, _ /*ResultType*/, AlgID, OpID, FunID),
  % Whatever amount of memory we choose here...we still need the values for
  % the plan generation.
  %getCosts(AlgID, OpID, FunID, ICardX, ISizeX, ICardY, ISizeY, MiB,OpCostsInMS),

  getCostFun(AlgID, OpID, FunID, ICardX, ISizeX, ICardY, ISizeY, FT, DList),
	AList=[functionType(FT), dlist(DList)],

 	minimumOperatorMemory(MinOpMem),
	DList=[SufficientMemoryInMiB|_],
  CMiB is max(MinOpMem, min(MiB, SufficientMemoryInMiB)),
	buildFormulaByFT(FT, AList, CMiB, CostF),
	CostsInMS is CostF.

% Just for simulating some results because currently there are no cost 
% functions implemented.
fakedCosts(true).
opCosts(OpName, CardX, SizeX, MiB, CostsInMS, AList) :-
	fakedCosts(true),
	opCosts(OpName, CardX, SizeX, 0, 0, MiB, CostsInMS, AList).

opCosts(OpName, CardX, SizeX, CardY, SizeY, MiB, CostsInMS, AList) :-
	fakedCosts(true),
	dm(ma6, ['\ngetCosts failed for op: ', OpName, ' returning simulated costs: ']), 
	%FIX:
	%CostsInMS is CardX*SizeX + CardY*SizeY,
	A is CardX*SizeX + CardY*SizeY,
	%B is 0.1,
	SufficientMemoryInMiB is ceiling(A*100 / (2**20)),
 	minimumOperatorMemory(MinOpMem),
  CMiB is max(MinOpMem, min(MiB, SufficientMemoryInMiB)),
	%CostsInMS 	 is A / (CMiB*B),
	CostsInMS 	 is A / CMiB,
  CMiB2 is max(16, SufficientMemoryInMiB),
	CostsAt16MiB is A / CMiB2,
	dm(ma6, [CostsInMS,'\n']), 
	AList=[
		functionType(2), 
		dlist([SufficientMemoryInMiB, -1, CostsAt16MiB, A, b, c, d])
	].

/*
The nonlinear optimization.
*/
optimizeMemoryInPath(Path, RPath) :-
	ensure((
		newOpt,
		analyzePath(Path),

		buildFormula(Vars, F, SufficientMemoryInMiB),
		setFormula(Vars, F),
		dm(ma3, ['\nFormula: ', F]),
  	length(Vars, Len), % Len is the number of memory consuming operators.
  	secondoGlobalMemory(Memory),
		MemoryAvailable is Memory - Len, % Leave "some" space for rounding. 
		% Note that an integer nonlinear problem is more complex and the 
		% expected "integer error" shouldn't be that high.
  	ensure(MemoryAvailable>=Len), % Should always be the case, but if not we 
		% will
		% raise a exception here because the optimization routine needs a start
		% point with one MiB memory for every operator. This can be changed, of
		% course, but a optmization makes not much sense with these values
		% when we can at least assign one MiB memory to an operator.
  	doMemoryOptimization(Path, MemoryAvailable, Len, SufficientMemoryInMiB, 
			_ /*Result*/, RPath)
	)).

/*
doMemoryOptimization(+Path, +Memory, +Len, +SufficientMemoryInMiB, -Result, -RPath)
*/
doMemoryOptimization(Path, _, 0, _, _, Path) :-
	% Hence, there is nothing more to do
	dm(ma3, ['\nNo memory optimisation needed.']).

doMemoryOptimization(Path, Memory, Len, SufficientMemoryInMiB, Result, RPath) :-
	Len > 0,
	dm(ma, ['\nStart optimisation with Memory: ', Memory, ' MiB...']),
	dm(ma, ['\nSufficient memory values in MiB: ', SufficientMemoryInMiB]),
	minimumOperatorMemory(MinOpMem),
	dm(ma, ['\nMinimum operator memory: ', MinOpMem]),
  getTime(
		memoryOptimization(MinOpMem, Memory, Len, SufficientMemoryInMiB, Result), 
		TimeMS),
	dm(ma, ['\nOptimization result: ', Result]),
	dm(ma, ['\nComputed in: ', TimeMS, 'ms']),

	optimizeResultToAssignableResult(Result, AResult),
	% Assign the new memory values
	assignMemory(AResult),
	% and the costs are changed...
	% So we need to update the costs within the costEdge facts of the given path.
	% If you need the costs within the POG you need to call createCostEdges again.
	recomputePathCosts(Path, RPath).

doMemoryOptimization(Path, Memory, Len, SufficientMemoryInMiB, Result, Path) :-
	Msg='Memory optimization failed.',
	throw(error_Internal(ma_doMemoryOptimization(Path, Memory, Len, 
		SufficientMemoryInMiB, Result, Path)::Msg)).

/*
Rounding the Results because the secondo kernel expected integer values.
The result in ~Result~ may assign a little more memory than allowed
due to precision and the choosen algorithm. So either
we allow this or we need to apply a algorithm to 
correct this. For example we could subtract the to much
assigned memory from the highest value. But currently it 
no problem to grant a little bit more memory, the Secondo kernel
won't forbid this.
*/
optimizeResultToAssignableResult(OResult, AResult) :-
	ceilingListToInts(OResult, AResult),
	%roundListToInts(Result, CResult), % If it should be more accurate.
	dm(ma, ['\nRounded result: ', AResult]).
	% If needed, here can now the rest unused memory distributed.

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
	retractall(maFormula(_, _)),
	retractall(maDerivativeFormula(_, _, _)),
	retractall(formulaList(_)),
	asserta(formulaList([])).

/*

*/
analyzePath([Edge]) :- 
	!,
	%Edge=costEdge(Source, Target, Term, Result, Size, Cost),
	Edge=costEdge(_, _, Term, _, _, _),
	% terms are in need to be views in reverse order.
	analyzeTerm(Edge, Term).

analyzePath([Edge|Rest]) :- 
	!,
	% Edges must be analyzed in order!
	analyzePath([Edge]),
	analyzePath(Rest).

analyzePath(P) :-
	Msg='Can\'t analyze the path.',
	throw(error_Internal(ma_analyzePath(P)::Msg)).

analyzeListTerm(_ /*Edge*/, []) :- 
	!.
analyzeListTerm(Edge, [Term|Rest]) :- 
	!, 
	analyzeTerm(Edge, Term),
	analyzeListTerm(Edge, Rest).

analyzeTerm(Edge, Term) :-
	compound(Term),
	Term=..[Functor|Args],
	Functor=memory, % These are added during the assignCosts phase.
	Args=[SubTerm, MID, AList],
	% 
	SubTerm=..[OpFunctor|OpArgs],
	% This ansumption is based on how the terms are constructed and plans 
	% are generated.
	operatorMemSpec(OpFunctor, Map, BlockingType, _),

	addFormulaByAList(MIDVAR, MID, AList),

	(BlockingType = blocking ->
		% how to create the shelf constraints? first, we need to bring the
		% terms in order.
		true
	;
		true
	),
	% Just finding the SubTerms to inspect.
	mapArguments(Map, OpArgs, MappedOpArgs),
	analyzeListTerm(Edge, MappedOpArgs).
	%analyzeTerm(Edge, SubTerm).
	
analyzeTerm(Edge, Term) :-
	compound(Term),
	Term=..[Functor|OpArgs],
	Functor\=memory,
	% So we have to inspect the subterms for further memory consuming operators, 
	% but only if the given term contains substreams.
	opMap(Functor, Map),
	mapArguments(Map, OpArgs, MappedOpArgs),
	analyzeListTerm(Edge, MappedOpArgs).
	
/*
For non-compound terms, there is no stream in it and there is no further analyze necessary.
analyzeTerm(+Edge, +Term)
*/
analyzeTerm(_, Term) :- 
	\+ compound(Term).

% This might be okay, but more likly is something wrong, so at least 
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
Based on the formulas for every memory consuming operator, we build the cost formula for the entire path. Note that we optimize just the memory using operators, because the total costs are additive, this is so far correct, but the computed value that is the "optimum", is not the values of our total costs.

Note that all cost functions of the nodes are additive, 
when we compute the derivatives, we can do this seperatly for every operator.
It is not needed to compute the derivative of the entire total cost function.
buildFormula(-AllVars, -Formula, -SufficientMemory) :-
*/
buildFormula(AllVars, F, SufficientMemory) :-
	formulaList(List),
	getAllVars(List, AllVars),
	buildFormula(List, AllVars, 0, F, SufficientMemory).
	
buildFormula([], _, _, 0, []) :- !.

buildFormula([E], AllVars, Dimension, F, [SufficientMemoryInMiB]) :-
	!,
	propertyValue(functionType, E, FT),
	propertyValue(dlist, E, AList),
	propertyValue(midVar, E, MVar),
	buildFormulaByFT(FT, AList, MVar, F),
	AList=[DSufficientMemoryInMiB|_],
	% AList conts double values, we could compute the result 
	% with double values, but later we can only assign integer values.
	% so we need to retrict the allowed value to the next integer value.
	SufficientMemoryInMiB is ceiling(DSufficientMemoryInMiB),
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
Returns a list of all variables we are using for the optimization.
getAllVars(+, -)
*/
getAllVars([], []).
getAllVars([E|ERest], [MVar|MRest]) :-
	propertyValue(midVar, E, MVar),
	getAllVars(ERest, MRest).

/*
Following, the differnt function types based on the returned values of the getCostFun predicate. Note that this done this way because returning arbitrary formula's from the C++ enviroment isn't that easy as within prolog. At least it was the first idea to support more complex cost functions, even if now from the C++ CostEstimation class just simple function will be deliverd. But with the below predicates, much more complex functions are supported as long these are strict and decreasing, more exact, all function that the choosen optimize algorihm can handle.
buildFormulaByFT(+FunctionType, +DList, +MVar, ?Out)

Parameter MVar:
Allowed values are all values between the minimum operator memory(>=1) and the sufficient memory value. Don't call this with other memory values.
*/
buildFormulaByFT(1, DList, MVar, Out) :- 
	!,
	DList=[_, _, TimeAt16MB, A|_], % Vars names as within the CostEstimation.h.
	ensure((number(TimeAt16MB), number(A))), % The memory can be the only free variable.
	TimeAt0MB is TimeAt16MB+(16*A), % This approximation is	dangerous if the calculation is enabled for memory assigments below 16 MiB.
	Out=TimeAt0MB-(MVar*A).

buildFormulaByFT(2, DList, MVar, Out) :- 
	!,
	DList=[_, _, _, A|_], % Vars names as within the CostEstimation.h.
	ensure(number(A)), % The memory can be the only free variable.
	Out=A/MVar. % MVar will bound to a values below 1.

buildFormulaByFT(3, DList, MVar, Out) :- 
	!,
	DList=[_, _, _, A, B|_], % Vars names as within the CostEstimation.h.
	ensure((number(A), number(B))), % The memory can be the only free variable.
	Out=A-(MVar*B).

buildFormulaByFT(FT, DList, MVar, Out) :-
	Msg='Can\'t build formula.',
	throw(error_Internal(ma_buildFormulaByFT(FT, DList, MVar, Out)::Msg)).



/*
Return which plan arguments are streams.
Note that therefore the plan term needs to match the operator signatures, at least for the first arguments. So the assumption is made that the operator signatures equals the arguments meaning of the plan terms. If a plan functor does not match a operator name, manually added opSigMap facts must be added. 

The reason why this is done is because it is neccessary to analyse the terms within the cost edges. So, either we have to define for every operator which argumentes are substreams to be analyzed by adding opeprators specific analyze-predicates or by knowing which argumente are substreams. Hence, the information what arguments are substreams, is delivered by this predicate.

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
  opSigMap(Op, Map).

/*
This won't work for some operators because often PSig can't be a variable.
That the reason why for these operators the opSigMap facts must be added manually.
*/
opMap(Op, Map) :-
	opSignature(Op, _, PSig, _, _),
	analyzeOpSig(PSig, Map).

opMap(Op, Map) :-
	% try harder...
	clause(opSignature(Op, _, PSig, _, _), _),
	analyzeOpSig(PSig, Map).

/*
An error is thrown if there is nothing found for the operator, it might sometimes appropriate to assume false, but i assume the error rate is higher if there is no exception thrown.
*/
opMap(Op, Map) :-
	Msg='Signature mapping not found for the given operator.',
	throw(error_Internal(ma_opMap(Op, Map)::Msg)).

analyzeOpSig([P|PRest], [t|Map]) :-
	\+ var(P), % Because P may be a variable, see the hashjoin signature.
	P=[stream|_], !,
	analyzeOpSig(PRest, Map).
analyzeOpSig([_|PRest], [f|Map]) :- !,
	analyzeOpSig(PRest, Map).
analyzeOpSig([], []).

:- dynamic opSigMap/2.
% Refer to the cost functions about these operators.
opSigMap(sortLeftThenMergejoin,  [t,t,f,f]).
opSigMap(sortRightThenMergejoin, [t,t,f,f]).
% Overwritten because the opSignature result would't recognise the secondo argument as a stream:
opSigMap(res, 					[f]).   % not known as a plan term.
opSigMap(loopjoin, 			[t,t]). % differs from the plan terms.

/*
opSigMap(rename, 				[t]).
opSigMap(remove, 				[t]).
opSigMap(filter, 				[t,f]).
opSigMap(project,				[t,f]).
opSigMap(feedproject,		[f,f]).
opSigMap(afeed,					[f]).
opSigMap(extend,				[t,f]).
*/

/*
Extract only the relevant arguments depending on the given map. A t means true and the argument is taken into the result map. A f means false and, of course, then the argument is not taken into the result. t/f is just used because it is shorter as true and fail/false.

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

% Additional parameter are treated as f.
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
	
Note that the objective function is stored here as a fact, so we don't need to loopthrough it into the c++ runtime, even if it might not be very efficient. This idea behind this is mainly that we don't want to pass the formula as a string to C++ enviroment to evalute it there coz this not that simple and we might need another library to do this. On the onther hand, it was at the beginning not very clear how complex the cost functions will be. However, with these predicates, even more complex cost functions could be optimized.
*/
objectiveFunction(Vars, Result) :-
	dm(ma6, ['\nCompute objectiveFunction: Vars: ', Vars]),
	maFormula(Vars, Formula),
	Result is Formula, % Evaluate the expression with the bindings from Vars.
	dm(ma6, ['\nCompute objectiveFunction: Result:', Result]).

/*
This is the predicate to compute the gradient, it will be called from the MemoryAllocation.cpp. But note that this depends on the choosen algorithm, some of the alogithm doesn't need a derivation.
*/
derivativeFunction(Dimension, Vars, Result) :-
	dm(ma6, ['\nCompute derivativeFunction: Dimension: ', Dimension, 
		' Vars: ', Vars]),
	maDerivativeFormula(Dimension, Vars, Formula),
  Result is Formula, % Evaluate the expression with the bindings from Vars.
	dm(ma6, ['\nCompute derivativeFunction: Result: ', Result]).

% eof

