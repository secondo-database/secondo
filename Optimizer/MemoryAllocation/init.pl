/*
$Header$
@author Nikolai van Kempen

Mainfile to load the other memory allocation optimization files if the option is enabled.
*/ 

initMA :-
	L='Assign memory to operators.',
  % assert now non-multifile facts.
	assertz(optimizerOptionInfo(memoryAllocation, none, no, L, loadMA, true)),

	assertz(optDebugLevel(ma)),
	% This is a more detailed log level.
	assertz(optDebugLevel(ma3)),
	% This is a very detailed log level for the callback predicates.
	assertz(optDebugLevel(ma6)). 

% Needs to be declared here (used within optimizer.pl)
:- dynamic useModifiedDijkstra/0.

:- initMA.

loadMA :-

	(current_functor(memoryOptimization, 6) ->
		true
	;	
		(
			write_list([
				'\nError: memoryOptimization/5 predicate unknown.',
				'\nMost likely secondo was compiled without memory optimization ',
				'support. Enable this within ',
				'$SECONDO_BUILD_DIR/UserInterface/makefile and recompile secondo.\n'
			]),
 	
			sleep(5),
			delOption(memoryAllocation),
			!, 
			fail	% unfortunately currently "Unknown option 'memoryAllocation'" is
						% reported and the option will still be activated. Hence delOption
						% is called above.
		)
	),

	% See ma_improvedcosts.pl for more information.
	delOption(improvedcosts),
	delOption(nawracosts),
	delOption(nestedRelations),
	
	% dependent files...
	['NestedRelations/util.pl'],

	% Load new cost functions
	['MemoryAllocation/ma_improvedcosts.pl'],

	['MemoryAllocation/blockingops.pl'],
	['MemoryAllocation/madata.pl'],
	['MemoryAllocation/ma.pl'],
	['MemoryAllocation/differential_calculus.pl'],
	['MemoryAllocation/progressconstants.pl'],

	% loading the testing files, can safly removed if not needed.
	['MemoryAllocation/test.pl'], 
	['MemoryAllocation/testdc.pl'],
	%['MemoryAllocation/eval.pl'],
	
	maSelfCheck.

% eof
