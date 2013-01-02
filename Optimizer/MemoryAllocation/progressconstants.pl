/*

$Header$
@author Nikolai van Kempen

Provides access to the ProgressConstants.csv values.

Note: Won't distinguishing between the standalone and the client/server mode.

*/

:- dynamic pcInit/1.

pcInit(true).

pcInit :-
	File='../bin/ProgressConstants.csv',
	csv_read_file(File, Rows, []),
	assertProgressConstants(Rows),
	retractall(pcInit(_)).

assertProgressConstants([]) :-
	!.

assertProgressConstants([R|Rest]) :-
	assertProgressConstants(Rest),
	R=..[row|Args],
	Args=[Algebra, Operator, Constant, Value, Unit|_],
	asserta(progressConstant(Algebra, Operator, Constant, Value, Unit)),
	!.

getProgressConstant(_, _) :-
	pcInit(true),
	pcInit,
	retractall(pcInit(true)),
	fail.

getProgressConstant(Constant, Value) :-
	progressConstant(_, _, Constant, Value, _),
	!.

% eof
