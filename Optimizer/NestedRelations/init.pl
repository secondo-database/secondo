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

	% This must always be available, otherwiese loading the optimizer.pl will
	% display some errors(because, of course, prolog does not know what to do
	% this these 'operators').
	op(799, xfx, unnest),
	op(799, xfx, nest),

	% Note that this files must be loaded, regardless if the nestedRelations 
	% is enabled or not.
	['nr.pl'],
	['nr_auxiliary.pl'],

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
	%delOption(subqueries)
	% This is not implemented and not checked how nested relations affect
	% the subquery unnesting capabilities.
	delOption(subqueryUnnesting).

% eof
