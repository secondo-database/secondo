/*
$Header$
@author Nikolai van Kempen

Temporary utility predicates that are NOT NEEDED for the regular program execution.
*/

:- op(999, fx, tracegoal).

% Note that tracegoal is provable iff Goal was provable.
tracegoal(Goal) :-
	traceGoalsOn(Leash), % store...
  (Goal -> R=true ; R=fail),
	traceGoalsOff(Leash), % ... and restore leashes,
	R.

traceGoalsOn(Leash) :-
	(var(Leash) ->
  	leash(Leash), leash(-all)
	;
  	leash(Leash)
	),
  trace.

traceGoalsOff(Leash) :-
	notrace,
	nodebug,
	(var(Leash) ->
  	leash(Leash), leash(+all)
	;
  	leash(Leash)
	).

traceGoalsOn :-
  leash(-all),
  trace.

traceGoalsOff :-
	notrace,
	nodebug,
 	leash(+all).	

/*
Write all facts by its given name to stdout. 
Only useful for facts, not for predicates.
*/
writefacts(P) :-
	write('Facts '), 
	write(P), 
	write(': '), 
	nl,
	(
		current_functor(P, Arity), 
		LST1=[P], 
		length(LST2, Arity), % Creates a list with $Arity variables.
		append(LST1, LST2, LST), 
		A=..LST, 
		clause(A, true),
		write('    '), 
		write_term(A, []), 
		nl, 
		fail % try to find next one
	) 
	;
	 	true, 
	nl.

/*
Predicates to force writing the entire output without "..." within terms.
Use 0 for unlimited depth.
*/
debugMaxDepth(N) :-
	current_prolog_flag(debugger_print_options, Y),
	delete(Y, max_depth(_), Y2),
	append(Y2, [max_depth(N)], Y3),
	set_prolog_flag(debugger_print_options, Y3).

toplevelMaxDepth(N) :-
	current_prolog_flag(toplevel_print_options, Y),
	delete(Y, max_depth(_), Y2),
	append(Y2, [max_depth(N)], Y3),
	set_prolog_flag(toplevel_print_options, Y3).

maxOutput :-
	debugMaxDepth(0),
	toplevelMaxDepth(0).

:- maxOutput.

/*
Debug utility predicates that only takes affect for non test runs.
*/
sl :-
  testRunning, !.
sl :-
  sleep(3), !.

rd :-
  testRunning, !.
rd :-
  write('\nenter a term to continue: '),
  read(_), !.

% eof
