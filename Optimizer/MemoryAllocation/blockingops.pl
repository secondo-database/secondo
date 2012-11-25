/*

$Header$
@author Nikolai van Kempen

Provides predicate to create the constraints list to respect blocking operators.

*/

/*
Determines if an operator is a blocking operator.
isBlockingOperator(?OpName, ?Sig)
*/
isBlockingOperator(OpName, Sig) :-
	opSignature(OpName, _, Sig, _, Attributes),
	member(block, Attributes),
	!.

isBlockingOperator(OpName, Sig) :-
	% Note that this might return an incorrect result, even if this
	% is unlikely. It might occur for operators with the same name
	% but different blocking attributes.
	clause(opSignature(OpName, _, Sig, _, Attributes), _),
	member(block, Attributes),
	!.

:- dynamic
	useBlockingOpOptimization/0,
	maConstraint/2.

% Enabled by default
useBlockingOpOptimization.

setUseBlockingOpOptimization(on) :-
	retractall(useBlockingOpOptimization),
	assertz(useBlockingOpOptimization).

setUseBlockingOpOptimization(off) :-
	retractall(useBlockingOpOptimization).

/*
Creates the constraint [[GlobalMemory-n, 1, 1, ...]] (n times a 1)
This means that the globalmemory-n is distributed over all operators.
*/
createGlobalMemoryConstraints(MIDS, _Path, [[MaxMemory|Rest]]) :-
	(
		\+ useBlockingOpOptimization 
	; 
		% This is not possible because plan/2 may fail for partial paths.
		useModifiedDijkstra
	),
	length(MIDS, Len),
	listFix(Len, 1, Rest),
  secondoGlobalMemory(GlobalMemory),
	MaxMemory is GlobalMemory - Len,
	!.
	
createGlobalMemoryConstraints(MIDS, Path, CList) :-
	useBlockingOpOptimization,
	% with this plan, the structure is clearer than
	% in the cost edges. Could be improved, of course.
	once(plan(Path, Plan)),
	ensure(once(analyzePlanForConstraints(0, Plan, _Forward))),
	createMaxMemConstraints(MIDS, CList),
	!.

createGlobalMemoryConstraints(MIDS, Path, CList) :-
	throw(error_Internal(ma_createGlobalMemoryConstraints(MIDS, Path, CList))::
		failed).

/*
This is done just as the analyzeTerm predicates within the ma.pl.
The plan is recursively inspected to create the tree of blocking operators to
identify the shelves. A shelf is a set separated by two blocking operators(or the end or the beginning). As soon as the shelves are identified,
the entire available memory can be granted for every shelf.
*/	
analyzePlanForConstraints(CID, Plan, _ForwardVar) :-
	Plan=memory(Term, MID, _AList),
	!,
	assertz(maConstraint(CID, MID)),
 	
	% Special case: if only the next op is blocking
	% Forward the operator because they need to occur in both constraints.
	analyzePlanForConstraints(CID, Term, MID).
	
analyzePlanForConstraints(CID, Plan, ForwardMID) :-
	compound(Plan),
	Plan=..[Functor|OpArgs],
	isBlockingOperator(Functor, _),
	NextCID is CID+1,
	(ground(ForwardMID) ->
		assertz(maConstraint(NextCID, ForwardMID))
	;
		true
	),
	opMap(Functor, Map),
	mapArguments(Map, OpArgs, MappedOpArgs),
	% analyze sub streams
	analyzePlanForConstraintsList(NextCID, MappedOpArgs, _ForwardNOMID). 

analyzePlanForConstraints(CID, Plan, _ForwardMID) :-
	compound(Plan),
	Plan=..[Functor|OpArgs], 
	\+ isBlockingOperator(Functor, _),
	opMap(Functor, Map),
	mapArguments(Map, OpArgs, MappedOpArgs), 
	% analyze sub streams
	analyzePlanForConstraintsList(CID, MappedOpArgs, _ForwardNOMID).

analyzePlanForConstraints(CID, Plan, ForwardMID) :-
	compound(Plan),	
	throw(error_Internal(ma_analyzePlanForConstraints(CID, Plan, ForwardMID)::
		failed)). 
	
analyzePlanForConstraints(_CID, Plan, _VarList, _ForwardMID) :-
	\+ compound(Plan),
	!. 

analyzePlanForConstraintsList(_CID, [], _ForwardMID) :-
	!.
analyzePlanForConstraintsList(CID, [Plan|Rest], ForwardMID) :-
	analyzePlanForConstraints(CID, Plan, ForwardMID),
	analyzePlanForConstraintsList(CID, Rest, ForwardMID).

/*
Creates the constraint's lists from the maConstraint facts.
The idea is very simple. The size of every constraint list must have the same size as the number of memory operators and a 1 means that this operator's memory
is relevant for this memory constraint and 0 means that this value is not important for this constraint.

*/
createMaxMemConstraints(MIDS, CList) :-
	findall(ID, maConstraint(ID, _), IDS),
	max_list(IDS, Max),
	MaxP1 is Max + 1,
	!,
	createMaxMemConstraintsForID(0, MaxP1, MIDS, CList),
	dm(ma3, ['\nConstraints: ', CList]),
	!,
	checkConstraints(MIDS, CList).

createMaxMemConstraints(_MIDS, [[GlobalMemory]]) :-
	!,
	% No maConstraints present.
  secondoGlobalMemory(GlobalMemory).

% End of search	

/*
Some simple checks, just to be sure that nothing wrong is passed onto the nonlinear optimization.
*/
checkConstraints(MIDS, []) :-
	!,
	throw(error_Internal(ma_checkConstraints(MIDS, [])::
		'An empty constraint list is not allowed.')).

checkConstraints(MIDS, CList) :-
	member(C, CList),
	length(MIDS, Len),
	length(C, CLen),
	CLen2 is CLen-1,
	Len \= CLen2,
	!,
	throw(error_Internal(ma_checkConstraints(MIDS, CList)::
		'Constraintlist has the wrong size.')).

checkConstraints(_MIDS, _) :-
	% Then it is probably okay.
	!.

createMaxMemConstraintsForID(MaxID, MaxID, _MIDS, []) :-
	!.

% Shelf with no memory using operator
createMaxMemConstraintsForID(ID, MaxID, MIDS, CList) :-
	\+ maConstraint(ID, _),
	IDNext is ID + 1,
	createMaxMemConstraintsForID(IDNext, MaxID, MIDS, CList),
	!.

createMaxMemConstraintsForID(ID, MaxID, MIDS, CList) :-
	createMaxMemConstraintsForMIDS(ID, MIDS, CL1),
  secondoGlobalMemory(Memory),
	count(member(1, CL1), Count),
	MemoryAvailable is Memory - Count, % Leave "some" space for rounding. 
  % Note that an integer nonlinear optimization problem is much more 
	% expensive and the expected "integer error" shouldn't be that high.

  % It should be like that everytime. But, if there are so many operators 
	% involved this optimzation is not able to compute a solution.
	minimumOperatorMemory(MinOpMem),
  ensure((X is Count * MinOpMem, MemoryAvailable>=X)),
  % Hence, an exception will be raised here because the optimization routine
	% needs a starting point with MinOpMemory MiB memory for every operator. 
	% This can be changed, of course, but an optmization doesn't make much sense 
	% with these values.

	append([MemoryAvailable], CL1, CL), % The first entry is the amount
	% of memory that can be assigned in this shelf.
	IDNext is ID + 1,
	createMaxMemConstraintsForID(IDNext, MaxID, MIDS, CLRest),
	append(CLRest, [CL], CList),
	!.	

createMaxMemConstraintsForMIDS(_ID, [], []) :- 
	!.

createMaxMemConstraintsForMIDS(ID, [MID|MIDRest], [1|CListRest]) :-
	maConstraint(ID, MID),
	!,
	createMaxMemConstraintsForMIDS(ID, MIDRest, CListRest).

createMaxMemConstraintsForMIDS(ID, [MID|MIDRest], [0|CListRest]) :-
	maConstraint(ID, _),
	\+ maConstraint(ID, MID),
	!,
	createMaxMemConstraintsForMIDS(ID, MIDRest, CListRest).

% eof
