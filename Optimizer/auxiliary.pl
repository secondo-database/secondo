/*

1 Auxiliary Predicates

This file contains the pretty-printing predicate
~pretty\_print~ and various auxiliary predicates for
~pretty\_print~ and a ~secondo~ predicate which uses just
one argument (the command) and pretty-prints the result.

1.1 Predicate ~pretty\_print~

Predicate ~pretty\_print~ prints a list L which is assumed to
be a PROLOG representation of a
Secondo nested list. That is the case e.g.
if L is output by the ~secondo~ predicate.

1.1.1 Predicates Auxiliary to Predicate ~pretty\_print~

*/
is_atomic_list([]).
is_atomic_list([Head | Tail]) :-
  atomic(Head),
  is_atomic_list(Tail).

write_tabs(0) :- !.

write_tabs(N) :-
  write('  '),
  N1 is N - 1,
  write_tabs(N1).

write_atoms([X]) :-
  !,
  write(X).

write_atoms([X | Rest]) :-
  write(X),
  write(', '),
  write_atoms(Rest).

write_element(X, N) :-
  atomic(X),
  write_tabs(N),
  write(X).

write_element(X, N) :-
  is_atomic_list(X),
  !,
  write_tabs(N),
  write('['),
  write_atoms(X),
  write(']').

write_element(X, N) :-
  is_list(X),
  N1 is N + 1,
  write_tabs(N),
  write('['),
  nl,
  write_elements(X, N1),
  write(']').

write_elements([], _).

write_elements([X], N) :-
  !,
  write_element(X, N).

write_elements([X | L], N) :-
  write_element(X, N),
  write(','),
  nl,
  write_elements(L, N).
  
/*

1.1.2 Predicate ~pretty\_print~

*/
pretty_print(L) :-
  write_element(L, 0).

/*

1.2 Predicate ~secondo~


Predicate ~secondo~ expects its argument to be a string atom or
a nested list, representing a query to the SECONDO system. The query is
executed and the result pretty-printed. If the query fails, the error code
and error message are printed.

*/
secondo(X) :-
  (
    secondo(X, Y),
    write('Command succeeded, result:'),
    nl,
    pretty_print(Y)
  );(
    secondo_error_info(ErrorCode, ErrorString),
    write('Command failed with error code : '),
    write(ErrorCode),
    nl,
    write('and error message : '),
    nl,
    write(ErrorString),
    nl,
    fail
  ).
