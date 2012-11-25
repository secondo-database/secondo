/*
$Header$
@author Nikolai van Kempen

Predicates to compute the derivate of a given function.

Currently supported is only to derivate formulas with one variable because this is all that is needed for the memory optimization.
*/


/*
log is defined as ln in SWI-Prolog. 
*/
:- arithmetic_function(ln/1).
ln(A, B) :-
	B is log(A).

/*
Following the most important derivation rules sufficing our needs.
derivate(+F, ?VAR, ?DF)
F is the formula to derivate, VAR is the variable (a prolog variable or a term that is not a constant) and DF the derivative formular. 
*/

derivate(X1, _, 0) :- 
	constant(X1),
	!.

% In this case X is the variable itself.
% The direct usage of a prolog variable is allowed.
derivate(X1, X2, 1) :- 
	sameVar(X1, X2), 
	!.

derivate(F+G, X, DF+DG) :- 
	derivate(F, X, DF),
	derivate(G, X, DG).

derivate(F-G, X, DF-DG) :- 
	derivate(F, X, DF),
	derivate(G, X, DG).

derivate(C*F, X, C*DF) :- 
	constant(C),
	derivate(F, X, DF).

derivate(F*G, X, F*DG+G*DF) :- 
	derivate(F, X, DF), 
	derivate(G, X, DG).

derivate(F/G, X, (DF*G-F*DG)/G**2) :-
	derivate(F, X, DF), 
	derivate(G, X, DG).

derivate(F**C, X, (C*(F**C1)*DF)) :-
	constant(C),
	C1 is C-1,
	derivate(F, X, DF).

% e.g. ln(x)' := 1/x
derivate(ln(F), X, DF/F) :- 
	derivate(F, X, DF).

derivate(sin(F), X, DF * cos(F)) :-
	derivate(F, X, DF).

constant(X) :- 
	number(X).

isVar(X) :-
	var(X), !.

isVar(X) :-
	\+ constant(X),
	atomic(X), !.
	
/*
We consider a variable identical if 
a) two free variables are the	same variable
b) two atomic values are identical if they are not constants.
*/	
sameVar(X1, X2) :-
  X1==X2,
  isVar(X1),
  isVar(X2), 
	!.

sameVar(X1, X2) :-
  \+ constant(X1),
  \+ constant(X2),
  \+ var(X1),
  \+ var(X2),
  X1=X2, 
	!.

% eof
