quit :- 
  halt.

argList( 1, [_] ).
argList( N, [_|L] ) :-
  N1 is N-1,
  argList( N1, L ).

showValues( Pred, Arity ) :-
  not(showValues2( Pred, Arity )).

showValues2( Pred, Arity ) :-
  argList( Arity, L ),
  P=..[Pred|L], !, P, nl, write( P ), fail.


