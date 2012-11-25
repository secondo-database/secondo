/*
$Header$
@author Nikolai van Kempen

Following some stuff used from auxiliary.pl to display nrel types in the way
it is done within the SecondoBDB etc.

*/


/*
This is for nested relations. Like the algebra display function 4 blanks are added when the predicate runs into a arel relation.
*/
max_attr_length_nrel([], _, R, R).

max_attr_length_nrel([[Name, Type] | AttrDescription], Deep, IN, OUT) :-
  ( Type = [arel,[tuple,ArelDef]]
    -> (
      NewDeep is Deep + 1,
      append(IN, [0], NewIN),
      max_attr_length_nrel(ArelDef, NewDeep, NewIN, NewOut),
      OUT2=NewOut
    )
    ;
      OUT2=IN
  ),
  max_attr_length_nrel(AttrDescription, Deep, OUT2, M1OUT),
  % The arel attribute name is not indented.
  atom_length(Name, M2),
  % Unfortunately this is a little bit nasty, but every nested relation 
  % within the deep x should be layouted at the same position.
	nth0(Deep, M1OUT, INDEND),
  M is max(INDEND, M2),
  setx(M1OUT, Deep, M, OUT).

/*
Create a new list where the element at position ~Index~ in the first list will be ~Element~.
setx(+List, +Index, +Element, -NewList)
*/
setx([_], 0, T, [T]).
setx([_|REST], 0, T, [T|REST]).
setx([A|REST], Deep, T, [A|OUT]) :-
  NewDeep is Deep-1,
  setx(REST, NewDeep, T, OUT).

/* 
For nrel types the same thing is done as by the display predicate.
*/
nr_pretty_print([[nrel, [tuple, AttrDescription]], Tuples]) :-
  display([[nrel, [tuple, AttrDescription]], Tuples]).

/* 
Display predicate for nested relations.
*/
nr_display([Rel, [tuple, Attrs]], Tuples) :-
  Rel = nrel,
  !,
  nl,
  max_attr_length_nrel(Attrs, 0, [0], AttrLengths),
  % We should ensure this or otherwise the layout might be confusing
  % during shorter names in an inner relation.
  % max_attr_length_nrel calculates just for every deep needed lengths.
  makeIncreasingList(AttrLengths, AttrLengths2),
  displayTuplesNrel(Attrs, 0, Tuples, AttrLengths2).

/*
Means every element to the right is at least as high as all values to the left.
*/
makeIncreasingList([],[]) :- !.
makeIncreasingList([A],[A]) :- !.
makeIncreasingList([A,B|Rest],[A,D|Rest2]) :- 
	!,
  C is max(A, B),
  makeIncreasingList([C|Rest],[D|Rest2]).

/* 
Display function for nested relations, kept here separated, too. But if needed, this could handle the usual rel/trel case as well.
*/
displayTuplesNrel(_, _, [], _).

% just avoids too much nl calls. It would result into a nasty layout.
displayTuplesNrel(Attrs, Deep, [Tuple], AttrLength) :-
  displayTupleNrel(Attrs, Deep, Tuple, AttrLength).

displayTuplesNrel(Attrs, Deep, [Tuple | Rest], AttrLength) :-
  displayTupleNrel(Attrs, Deep, Tuple, AttrLength),
  nl,
  displayTuplesNrel(Attrs, Deep, Rest, AttrLength).

displayTupleNrel([], _, _, _).

displayTupleNrel([[Name, Type] | Attrs], Deep, [Value | Values], 
		[AttrNameLength|RestLength]) :-
  atom_length(Name, NLength),
  PadLength is (Deep * 4) + (AttrNameLength - NLength),
  write_spaces(PadLength),
  write(Name),
  write(' : '),
  ( Type = [arel,[tuple,ArelDef]]
    -> nl,
      NewDeep is Deep + 1,
      % Note: The first element is an internal reference used by the 
      % nested relation algebra.
      Value = [_|DValue],
      displayTuplesNrel(ArelDef, NewDeep, DValue, RestLength)
    ;
      (display(Type, Value),nl)
  ),
  displayTupleNrel(Attrs, Deep, Values, [AttrNameLength|RestLength]).

% eof
