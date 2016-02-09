/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/


upper(Lower, Upper) :-
  atom_codes(Lower, [First | Rest]),
  to_upper(First, First2),
  UpperList = [First2 | Rest],
  atom_codes(Upper, UpperList).


wp(Plan) :-
  plan_to_atom(Plan, PlanAtom),
  write(PlanAtom).


plan_to_atom(rel(Name, _, l), Result) :-
  atom_concat(Name, ' ', Result),
  !.

plan_to_atom(rel(Name, _, u), Result) :-
  upper(Name, Name2),
  atom_concat(Name2, ' ', Result),
  !.

plan_to_atom(res(N), Result) :-
  atom_concat('res(', N, Res1),
  atom_concat(Res1, ') ', Result),
  !.


plan_to_atom(Term, Result) :-
    is_list(Term), Term = [First | _], atomic(First), !,
    atom_codes(TermRes, Term),
    concat_atom(['"', TermRes, '"'], '', Result).

plan_to_atom([X], AtomX) :-
  plan_to_atom(X, AtomX),
  !.

plan_to_atom([X | Xs], Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Xs, XsAtom),
  concat_atom([XAtom, ', ', XsAtom], '', Result),
  !.

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
if L is output by the ~secondo~ predicate. If L is a relation,
a special output format is used which makes reading the
output more comfortable. That output format closely resembles
the output format used by SecondoTTY.

1.1.1 Predicates Auxiliary to Predicate ~pretty\_print~

*/

my_concat_atom([A,B],C) :- !,  
       current_predicate(atomic_list_concat/2), 
       atom_concat(A,B,C).
my_concat_atom(X,Y) :- 
      current_predicate(atomic_list_concat/2), 
      atomic_list_concat(X,Y),!.
my_concat_atom(X,Y) :- 
      current_predicate(concat_atom/2),
      concat_atom(X,Y).

my_concat_atom(X,Y,Z) :- 
      current_predicate(atomic_list_concat/3),
      atomic_list_concat(X,Y,Z), !.

my_concat_atom(X,Y,Z) :- 
      current_predicate(concat_atom/3),
      concat_atom(X,Y,Z).

my_string_to_atom(X,Y) :- 
      current_predicate(atom_string/2), 
      atom_string(Y,X),!.

my_string_to_atom(X,Y) :- 
     current_predicate(string_to_atom/2),
     string_to_atom(X,Y).

my_string_to_list(L,C) :-
      current_predicate(string_codes/2),
      string_codes(L,C),!.

my_string_to_list(L,C) :-
      current_predicate(string_to_list/2),
      string_to_list(L,C).

my_convert_time(Stamp,Y,Mon,Day,Hour,Min,Sec, MilliSec) :-
     current_predicate(stamp_date_time/3),
     stamp_date_time(Stamp, date(Y, Mon, Day, Hour, Min, FSec,_,_,_), local),
     Sec is integer(float_integer_part(FSec)),
     MilliSec is integer(float_fractional_part(FSec)*1000), !.

my_convert_time(Stamp,Y,Mon,Day,Hour,Min,Sec, MilliSec) :-
  current_predicate(convert_time/8),
  convert_time(Stamp,Y,Mon,Day,Hour,Min,Sec, MilliSec).  

my_list_to_set(L,S) :-
  is_list(L),
  list_to_set(L,S).

sformat(X,Y,Z) :-
  current_predicate(format/3),!,
  format(string(X),Y,Z).





is_atomic_list([]).
is_atomic_list([Head | Tail]) :-
  atomic(Head),
  is_atomic_list(Tail).

write_spaces(0).

write_spaces(N) :-
  N > 0,
  write(' '),
  N1 is N - 1,
  write_spaces(N1).

write_tabs(N) :-
  N1 is 2 * N ,
  write_spaces(N1).

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

max_attr_length([], 0).

max_attr_length([[Name, _] | AttrDescription], M) :-
  max_attr_length(AttrDescription, M1),
  atom_length(Name, M2),
  M is max(M1, M2).

write_tuple([], [], _).

write_tuple([[Name, _] | RestOfAttr], [AttrValue | RestOfValues], M) :-
  write(Name),
  atom_length(Name, NLength),
  PadLength is M - NLength,
  write_spaces(PadLength),
  write(' : '),
  write(AttrValue),
  nl,
  write_tuple(RestOfAttr, RestOfValues, M).

write_tuples(_, [], _).

write_tuples(AttrDescription, [Tuple], M) :-
  !,
  write_tuple(AttrDescription, Tuple, M).

write_tuples(AttrDescription, [Tuple | TupleList], M) :-
  write_tuple(AttrDescription, Tuple, M),
  nl,
  write_tuples(AttrDescription, TupleList, M).

/*

1.1.2 Predicate ~pretty\_print~

*/

pretty_print([[RelType, [tuple, AttrDescription]], Tuples]) :-
  (RelType = rel ; RelType = trel),
  !,
  nl,
  max_attr_length(AttrDescription, AttrLength),
  write_tuples(AttrDescription, Tuples, AttrLength).

% NVK ADDED NR: Support for nested relations.
pretty_print([[nrel, [tuple, AttrDescription]], Tuples]) :-
  nr_pretty_print([[nrel, [tuple, AttrDescription]], Tuples]),
  !.
% NVK ADDED NR END

pretty_print(L) :-
  write_element(L, 0).

/*

1.1.2 Predicate ~show~

*/


show([Type, Value]) :-
  !,
  display(Type, Value).

show(Y) :-
  write(Y),
  pretty_print(Y),
  nl.

/*

1.1.3 Predicate ~display~

----    display(Type, Value) :-
----

Display the value according to its type description. To be extended when new
type constructors are added to Secondo.

*/


% Section:Start:display_2_b
% Section:End:display_2_b

display(int, N) :-
  !,
  write(N).

display(real, N) :-
  !,
  write(N).

display(bool, N) :-
  !,
  write(N).

display(string, N) :-
  !,
  term_to_atom(String, N),
  displayString(String).

display(date, N) :-
  !,
  term_to_atom(String, N),
  displayString(String).

display(instant, N) :-
  !,
  term_to_atom(String, N),
  displayString(String).

display(text, N) :-
  !,
  write_elements([N], 0).

display(rect, [L, R, B, T]) :-
  !,
  write('rectangle xl = '), write(L),
  write(', xr = '), write(R),
  write(', yb = '), write(B),
  write(', yt = '), write(T).

display([Rel, [tuple, Attrs]], Tuples) :-
  (Rel = rel ; Rel = trel),
  !,
  nl,
  max_attr_length(Attrs, AttrLength),
  displayTuples(Attrs, Tuples, AttrLength).

% NVK ADDED NR: Support for nested relations.
display([Rel, [tuple, Attrs]], Tuples) :-
  nr_display([Rel, [tuple, Attrs]], Tuples),
  !.

% Just write the terms to stdout to avoid the 'There is no specific display'
% message.
display(upoint, UPoint) :-
  write_term(UPoint, []),
  !.
display(mpoint, MPoint) :-
  write_element(MPoint, 0),
  !.
% NVK ADDED END

display(duration, [0, MSec]) :-
  MSec > 3600000,
  !,
  Hour is round(float_integer_part(MSec / 3600000.0)),
  write(Hour), write('h '),
  Rest is MSec - (Hour * 3600000),
  display(duration, [0, Rest]).

display(duration, [0, MSec]) :-
  MSec > 60000,
  !,
  Min is round(float_integer_part(MSec / 60000.0)),
  write(Min), write('min '),
  Rest is MSec - (Min * 60000),
  display(duration, [0, Rest]).

display(duration, [0, MSec]) :-
  MSec > 1000,
  !,
  Sec is round(float_integer_part(MSec / 1000.0)),
  write(Sec), write('s '),
    Rest is MSec - (Sec * 1000),
    display(duration, [0, Rest]).

display(duration, [0, MSec]) :-
  !,
    MS is round(MSec),
  write(MS), write('ms').

display(duration, [Days, MSec]) :-
  !,
  write(Days), write('d '),
  display(duration, [0, MSec]).

display(Type, Value) :-
  write('There is no specific display function for type '), write(Type),
  write('. '),
  nl,
  write('Generic display used. '),
  nl,
  pretty_print(Value),
  nl.


displayString([]).

displayString([Char | Rest]) :-
  put(Char),
  displayString(Rest).

displayTuples(_, [], _).

displayTuples(Attrs, [Tuple | Rest], AttrLength) :-
  displayTuple(Attrs, Tuple, AttrLength),
  nl,
  displayTuples(Attrs, Rest, AttrLength).


displayTuple([], _, _).

displayTuple([[Name, Type] | Attrs], [Value | Values], AttrNameLength) :-
  atom_length(Name, NLength),
  PadLength is AttrNameLength - NLength,
  write_spaces(PadLength),
  write(Name),
  write(' : '),
  display(Type, Value),
  nl,
  displayTuple(Attrs, Values, AttrNameLength).


