/*

$Header$
@author Nikolai van Kempen

Provides access to the ../bin/ProgressConstants.csv values.

Note: Won't distinguishing between the standalone and the client/server mode.

*/


/*
Reads the csv file when
- getProgressConstant is called the first time
- after reloading this file and the first getProgressConsant call.
*/
:- dynamic pcInit/1.
pcInit(true).

pcInit :-
	File='../bin/ProgressConstants.csv',
	% With the options functor(progressConsant), arity(5), match_arity(false)
	% it is possible to assert the facts directly, but then you need to 
	% retract the header value...
	csv_read_file(File, Rows, [strip(true)]),
	retractall(progressConstant(_, _, _, _, _)),
	Rows=[_HeaderRow|Rest],
	assertProgressConstants(Rest), 
	retractall(pcInit(_)).

assertProgressConstants([]) :-
	!.

assertProgressConstants([R|Rest]) :-
	assertProgressConstants(Rest),
	R=..[_|Args],
	Args=[Algebra, Operator, Constant, Value, Unit|_],
	asserta(progressConstant(Algebra, Operator, Constant, Value, Unit)),
	!.

getProgressConstant(_, _, _, _) :-
	pcInit(true),
	ensure(pcInit, 'failed to load the progressConstants'),
	retractall(pcInit(true)),
	fail.

getProgressConstant(Algebra, Operator, Constant, Value) :-
	progressConstant(Algebra, Operator, Constant, Value, _),
	!.

% eof
