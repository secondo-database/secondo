/*
$Header$
@author Nikolai van Kempen

Testing file for derivation examples.

*/
testDC(1,    1, _).
testDC(2,  100, _).
testDC(3, -200, _).
testDC(4,    X, X).
testDC(5,    _, _).  % Means testDC(5,    Y, X). 
testDC(6,  1/X, X).
testDC(7,  ln(X), X).

% More complex functions
testDC(10,  ln(1/X), X).
testDC(11,  ln(1/(X**10)), X).
testDC(12,  ln((X**2+X**4+X**9)/(X**10)), X).

% derivation with more than one variable is currently not implemented
testDC(21, 20000-X+10000-(_/X), X).

% Test the functions of the test.pl file.
testDC(X, F, DV) :-
	testMA(Y, VarList, F),
	X is 1000+Y,
	VarList=[DV|_].
	
testDC(No) :-
	testDC(No, F, DVar) ,
	derivate(F, DVar, DF),
	nl,
	write_list(['No: ', No, ' D: ', DVar, ' F: ', F, '\n']),
	write_list(['Derivative: ', DF, '\n']).

testDC :-
	testDC(_),
	fail.

% eof
