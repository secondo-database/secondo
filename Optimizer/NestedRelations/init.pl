/*
$Header$
@author Nikolai van Kempen

Mainfile to load the other nested relations files if the option is enabled.
*/

initNR :-
	(optimizerOptionInfo(nestedRelations, _, _, _, _, _)	->
		true % avoid adding the fact more than once if we reload this file.
	;
		L='Support for nested relations.',
		% assert now non-multifile facts.
		assertz(optimizerOptionInfo(nestedRelations, none, no, L, loadNR, true)),
		assertz(optDebugLevel(nr))
	),

	% Note that this files must be loaded, regardless if the nestedRelations 
	% option is enabled or not.
	['nr.pl'],
	['nr_auxiliary.pl'],

	['nvkutil.pl'],
	['util.pl'],
	['tutil.pl'],
	['test.pl'].

:- initNR.

loadNR :-
	% This extension relies heaviliy onto the subqueries extension, so this won't 
	% work without the enabled subqueries extension.
	(optimizerOption(subqueries) ->
		true
	; 
		setOption(subqueries)
	),
 	delOption(determinePredSig), % incompatible
	% This is not implemented and not checked how nested relations affects
	% the subquery unnesting capabilities.
	delOption(subqueryUnnesting),
	nrSelfCheck.

nrSelfCheck :-
	nrOKOpts(List),
	optimizerOption(X),
	\+ member(X, List),
	write_list(['\nNestedRelations: Warning: Option ', X, 
		' may be incompatible.']), nl,
	sleep(5),
	fail.

nrSelfCheck :-
	!.

nrOKOpts([nestedRelations, subqueries, debug, autosave, autoSamples, standard,
	noHashjoin]).

knownAsIncompatible([determinePredSig, subqueryUnnesting, memoryAllocation]).

% eof
