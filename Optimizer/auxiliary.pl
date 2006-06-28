/*

---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

pretty_print([[rel, [tuple, AttrDescription]], Tuples]) :-
  !,
  nl,
  max_attr_length(AttrDescription, AttrLength),
  write_tuples(AttrDescription, Tuples, AttrLength).

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

----	display(Type, Value) :-
----

Display the value according to its type description. To be extended when new
type constructors are added to Secondo.

*/



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

display([rel, [tuple, Attrs]], Tuples) :-
  !,
  nl,
  max_attr_length(Attrs, AttrLength),
  displayTuples(Attrs, Tuples, AttrLength).

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

/*

1.2 Predicate ~secondo~


Predicate ~secondo~ expects its argument to be a string atom or
a nested list, representing a query to the SECONDO system. The query is
executed and the result pretty-printed. If the query fails, the error code
and error message are printed.

*/

% Such facts are used to mark a file has been considered by some predicate:
:- dynamic(tempConsideredFile/1). 


% atom_postfix(+Atom, +PrefixLength, ?Post)
% succeeds iff Post is a postfix of Atom starting after PrefixLength
atom_postfix(Atom, PrefixLength, Post) :- 
  atom_length(Atom, Length),
  PostLength is Length - PrefixLength,
  sub_atom(Atom, PrefixLength, PostLength, 0, Post).

% The following facts define Secondo object types, that are used for indices:
indexType(btree).
indexType(rtree).
indexType(rtree3).

/*
---- getSmallIndexCreateQuery(+Granularity, +BBoxType, +Type, +Rel, +Attr, 
                              +IndexName, -QueryAtom)
----
Create a ~QueryAtom~ that is a executable Secondo command string, that will create the
appropriate specialized R-Tree index from relation ~Rel~ for key attribute ~Attr~, with 
bounding boxes of type ~BBoxType~ and granularity of the bounding boxes set to ~Granularity~,
where the index is named ~IndexName~.

If both, ~Granularity~ and ~BBoxType~ are ~none~, a standard index will be created

*/

getSmallIndexCreateQuery(Granularity,BBoxType,Type,Rel,Attr,IndexName,QueryAtom)
  :-
  indexCreateQuery(Granularity,BBoxType,Type,Rel,Attr,IndexName,QueryList), !,
  concat_atom(QueryList, QueryAtom), !.

% Rules to create index queries

% rules to build specialized R-tree indices:
indexCreateQuery(object, time, _, Rel, Attr, IndexName, 
  ['let ', IndexName, '_small = ', Rel, 
   '_small feed addid extend[ p: point2d( deftime( .', Attr,
   ' ) ) ] creatertree[ p ]']) :- !.
indexCreateQuery(object, space, _, Rel, Attr, IndexName,
  ['let ', IndexName, '_small = ', Rel, 
   '_small feed addid extend[ t: trajectory( .', Attr,
   ' ) ] creatertree[ t ]']) :- !.
indexCreateQuery(object, d3, _, Rel, Attr, IndexName, 
  ['let ', IndexName, '_small = ', Rel, 
   '_small feed addid extend[ b: box3d( bbox( trajectory( .', Attr,
   ' ) ), deftime( .', Attr, ' ) ) ]',
   ' creatertree[ b ]']) :- !.

indexCreateQuery(unit, time, _, Rel, Attr, IndexName,
  ['let ', IndexName, '_small = ', Rel, 
   '_small feed addid extendstream[ Unit: units( .', Attr, 
   ' ) ] extend[ p: point2d( deftime( .Unit ) ) ]',
   ' creatertree[ p ]']) :- !.
indexCreateQuery(unit, space, _, Rel, Attr, IndexName,
  ['let ', IndexName, '_small = ', Rel, 
   '_small feed addid extendstream[ Unit: units( .', Attr, 
   ' ) ] extend[ t: trajectory( .Unit ) ]',
   ' creatertree[ t ]']) :- !.
indexCreateQuery(unit, d3, _, Rel, Attr, IndexName, 
  ['let ', IndexName, '_small = ', Rel, 
   '_small feed addid extendstream[ Unit: units( .', Attr, 
   ' ) ] creatertree[ Unit ]']) :- !.

% for later extensions:
indexCreateQuery(group10, time,  _, _, _, _, _) :- fail, !.
indexCreateQuery(group10, space, _, _, _, _, _) :- fail, !.
indexCreateQuery(group10, d3,    _, _, _, _, _) :- fail, !.

% the standard indices must go last:

% all types 'rtree<n>' are created with 'creatertree':
indexCreateQuery(none, none, Type, Rel, Attr, IndexName, 
  ['let ', IndexName, '_small = ', Rel, 
   '_small creatertree[ ', Attr, ' ]']) :-
  sub_atom(Type, 0, _, _, rtree), !.

% all other standard indices are created as follows:
indexCreateQuery(none, none, Type, Rel, Attr, IndexName, 
  ['let ', IndexName, '_small = ', Rel, 
   '_small create', Type, '[ ', Attr, ' ]']) :- !.

/*
---- createIndexSmall(+Rel, +ObjList, +IndexName, 
                      +LogicalIndexType, +Attr, +Granularity, +BBoxType)
----

Test, if a \_small index has to be created (is not a member of ~ObjList~) and 
create it if necessary. The index is specified by the relation ~Rel~ and key 
attribute ~Attr~, the index' name ~IndexName~, its type ~LogicalIndexType~, 
the index' ~Granularity~ and type of bounding box ~BBoxType~.

Also, add a \_small relation, if it is still not available.

*/

createIndexSmall(_, _, _, _, _, _, _) :- 
  not(optimizerOption(entropy)),!.
  
createIndexSmall(Rel, ObjList, IndexName, LogicalIndexType, Attr, 
                                            Granularity, BBoxType) :-
  optimizerOption(entropy),
  member(['OBJECT', Rel, _ , [[rel | _]]], ObjList),
  concat_atom([Rel, 'small'], '_', RelSmallName),
  concat_atom([IndexName, 'small'], '_', IndexSmallName),
  % create _small relation if not present (needed to create _small index)
  ( not(member(['OBJECT', RelSmallName, _ , [[rel | _]]], ObjList))
    -> tryCreateSmallRelation(Rel, ObjList) 
    ;  true
  ),
  % create _small index if not present
  (   ( indexType(LogicalIndexType),
        LogicalIndexType = PhysicalIndexType
      )
    ; ( optimizerOption(rtreeIndexRules), 
        member([LogicalIndexType, PhysicalIndexType],
               [[object_time,rtree], [object_space,rtree], [object_d3,rtree3],
%               [group10_time,rtree],[group10_space,rtree],[group10_d3,rtree3], 
%               for later extensions
                [unit_time,rtree],   [unit_space,rtree],   [unit_d3,rtree3]
               ])
      )
  ),
  ( not(member(['OBJECT',IndexSmallName,_ ,[[PhysicalIndexType | _]]],ObjList))
    *-> ( dm(index,['\nIn createIndexSmall: getSmallIndexCreateQuery(',
                    Granularity, ',', BBoxType, ',', PhysicalIndexType, ',', 
                    Rel, ',', Attr, ',', IndexName, ',', QueryAtom, ')\n']),
          getSmallIndexCreateQuery(Granularity, BBoxType, PhysicalIndexType, 
                                   Rel, Attr, IndexName, QueryAtom),
          tryCreate(QueryAtom)
        )
    ; true
  ), !.

createIndexSmall(Rel, ObjList, _, _, _, _, _) :-
  optimizerOption(entropy),
  not(member(['OBJECT', Rel, _ , [[rel | _]]], ObjList)),
  write('ERROR: missing relation '),
  write(Rel),
  write(' cannot create small relation and an index on small relation!'), !, 
  fail.

% Test, if there is a _small-relation for relation Rel, otherwise create it
checkIfSmallRelationExists(Rel, ObjList) :-
  member(['OBJECT', Rel, _ , [[rel | _]]], ObjList),
  concat_atom([Rel, 'small'], '_', RelSmallName),
  not(member(['OBJECT', RelSmallName, _ , [[rel | _]]], ObjList)),
  downcase_atom(Rel, RelD),
  tryCreateSmallRelation(RelD, ObjList), !.

% Test if for each relation in ObjList there also is a _small-relation, 
% otherwise create it
checkIfSmallRelationsExist(ObjList) :-
  optimizerOption(entropy),
  findall(X,
          ( member(['OBJECT', X, _ , [[rel | _]]], ObjList),
            not(sub_atom(X, _, _, 0, '_small')),
            not(sub_atom(X, _, _, 1, '_sample_')),
            not(sub_atom(X, 0, _, 0, 'SEC_DERIVED_OBJ')),
            checkIfSmallRelationExists(X, ObjList)
          ),
          _), !.

checkIfSmallRelationsExist(_) :-
  not(optimizerOption(entropy)), !.

  
% checkIfIndexIsStored(_, _, LFRel, LFAttr, Granularity, BBoxType, 
%                                            IndexType, IndexName, _) :-
%  (   ( % standard index
%        Granularity = none,
%        BBoxType = none,
%        LogicalIndexType = IndexType
%      )
%    ; ( % specialized R-Tree index
%        concat_atom([Granularity, BBoxType], '_', LogicalIndexType)
%      )
%  ),
%  databaseName(DB),
%  storedIndex(DB, LFRel, LFAttr, LogicalIndexType, IndexName), !.

checkIfIndexIsStored(Rel, Attr, LFRel, LFAttr, 
                     Granularity, BBoxType, IndexType, 
                     IndexName, ObjList) :-
  databaseName(DB),
  storedNoIndex(DB, LFRel, LFAttr),
  ( (Granularity = none, BBoxType = none) % standard index
    -> LogicalIndexType = IndexType
    ; ( % specializes R-Tree index
        concat_atom([Granularity, BBoxType], '_', LogicalIndexType)
      )
  ),
  retractall(storedNoIndex(DB, LFRel, LFAttr)),
  retractall(storedIndex(DB, LFRel, LFAttr, LogicalIndexType, IndexName)),
  assert(storedIndex(DB, LFRel, LFAttr, LogicalIndexType, IndexName)),
  createIndexSmall(Rel,ObjList,IndexName,LogicalIndexType,Attr, 
                                              Granularity,BBoxType), !.

checkIfIndexIsStored(Rel, Attr, LFRel, LFAttr, 
                     Granularity, BBoxType, IndexType, 
                     IndexName, ObjList) :-
  ( (Granularity = none, BBoxType = none) 
    -> LogicalIndexType = IndexType % standard index
       % specialized R-Tree index
    ;  concat_atom([Granularity, BBoxType], '_', LogicalIndexType) 
  ),
  databaseName(DB),
  retractall(storedNoIndex(DB, LFRel, LFAttr)),
  retractall(storedIndex(DB, LFRel, LFAttr, LogicalIndexType, IndexName)),
  assert(storedIndex(DB, LFRel, LFAttr, LogicalIndexType, IndexName)),
  createIndexSmall(Rel,ObjList,IndexName,LogicalIndexType,Attr, 
                                              Granularity,BBoxType), !.

checkForAddedIndex(ObjList) :-
  member(['OBJECT', IndexName, _ , [[IndexType | _]]], ObjList),
  indexType(IndexType),
  concat_atom(L, '_', IndexName),
  (  ( L = [LFRel, Attr], atomic(LFRel), atomic(Attr) )  % standard index
     *-> ( Granularity = none, BBoxType = none )
     ; ( % specialized R-tree index
        L = [LFRel, Attr, Granularity, BBoxType],
        member(Granularity, [object, unit, group10]),
        member(BBoxType, [time, space, d3])
       ) 
  ),
  not(Attr = small),
  not(Attr = sample),
  not(tempConsideredFile(IndexName)),
  relname(LFRel, Rel),
  lowerfl(Attr, LFAttr),
  checkIfIndexIsStored(Rel, Attr, LFRel, LFAttr, Granularity, 
                              BBoxType, IndexType, IndexName, ObjList),
  assert(tempConsideredFile(IndexName)).

relname(LFRel, LFRel) :-
  spelled(LFRel, _, l), 
  !.

relname(LFRel, Rel) :-
  spelled(LFRel, _, u),
  upper(LFRel, Rel).
  
checkForAddedIndices(ObjList) :-
  retractall(tempConsideredFile(_)),
  findall(_, checkForAddedIndex(ObjList), _), !.

checkForRemovedIndex(ObjList) :-
  databaseName(DB),
  storedIndex(DB, Rel, Attr, LogicalIndexType, IndexName),
  (   ( indexType(LogicalIndexType),
        LogicalIndexType = PhysicalIndexType
      )
    ; ( optimizerOption(rtreeIndexRules), 
        member([LogicalIndexType, PhysicalIndexType],
              [[object_time,rtree], [object_space,rtree], [object_d3,rtree3],
%              [group10_time,rtree],[group10_space,rtree],[group10_d3,rtree3], 
%              for later extensions
               [unit_time,rtree],   [unit_space,rtree],   [unit_d3,rtree3]
              ])
      )
  ),
  not(member(['OBJECT', IndexName, _ , [[PhysicalIndexType | _]]], ObjList)),
  retract(storedIndex(DB, Rel, Attr, LogicalIndexType, IndexName)),
  assert(storedNoIndex(DB, Rel, Attr)),
  concat_atom([IndexName, 'small'], '_', IndexNameSmall),
  member(['OBJECT', IndexNameSmall, _ , [[PhysicalIndexType | _]]], ObjList),
  concat_atom(['delete ',IndexNameSmall], '', QueryAtom),
  secondo(QueryAtom).

checkForRemovedIndices(ObjList) :-
  findall(_, checkForRemovedIndex(ObjList), _).

checkIfIndex(X, ObjList) :-
  sub_atom(X, _, _, _, IndexName),
  member(['OBJECT', IndexName, _ , [[IndexType | _]]], ObjList),
  indexType(IndexType),
  checkForAddedIndices(ObjList),!.

checkIfIndex(_, _) :-
  true.	

checkIsInList(X, ObjList, Type) :-
  sub_atom(X, _, _, _, Name),
  member(['OBJECT', Name, _ , [[Type | _]]], ObjList),!.

checkIsInList(_, _, _) :-
  fail.


/* 
Some facts which store important state information

*/
 
:- dynamic storeupdateRel/1.
:- dynamic storeupdateIndex/1.
:- dynamic storedDatabaseOpen/1.

% the facts above get asserted with value 0 at the startup
% of the optimizer. Refer to calloptimizer.pl

:- dynamic databaseName/1.

% databaseName/1 gets asserted, when a database is opened


secondoResultSucceeded(Result) :-
  write('Command succeeded, result:'),
  nl, nl,
  show(Result), !.

secondoResultFailed :- 
  secondo_error_info(ErrorCode, ErrorString),
  write('Command failed with error code : '),
  write(ErrorCode), nl,
  write('and error message : '), nl,
  write(ErrorString), nl, !.



secondo(X) :-
  sub_atom(X,0,4,_,S),
  atom_prefix(S,'open'),	
  atom_postfix(X, 14, DB1),
  downcase_atom(DB1, DB), !,
  ( secondo(X, Y) 
    *-> ( secondoResultSucceeded(Y), !,
          retractall(storedDatabaseOpen(_)),
          assert(storedDatabaseOpen(1)),
          retractall(databaseName(_)),
          assert(databaseName(DB)),
          retractall(storedSecondoList(_)),
          getSecondoList(ObjList),
          checkIfSmallRelationsExist(ObjList),
          getSecondoList(ObjList2),
          checkForAddedIndices(ObjList2),
          checkForRemovedIndices(ObjList2)
        )
    ;   ( secondoResultFailed,
          fail
        )
  ), !.

secondo(X) :-
  sub_atom(X,0,5,_,S),
  atom_prefix(S,'close'), !,	
  ( secondo(X, Y)
    *-> ( secondoResultSucceeded(Y),
          retract(storedDatabaseOpen(_)),
          assert(storedDatabaseOpen(0)),
          retractall(storedSecondoList(_)),
          retractall(databaseName(_))
        )
    ;   ( secondoResultFailed,
          fail
        )
  ), !.
  
secondo(X) :-
  sub_atom(X,0,6,_,S),
  atom_prefix(S,'update'),
  isDatabaseOpen, !,
  ( secondo(X, Y)
    *-> ( secondoResultSucceeded(Y),
          retractall(storedSecondoList(_))
        )
    ;   ( secondoResultFailed,
          fail
        )
  ), !.
    
secondo(X) :-
  sub_atom(X,0,6,_,S),
  atom_prefix(S,'derive'),
  isDatabaseOpen, !,
  ( secondo(X, Y)
    *-> ( secondoResultSucceeded(Y),
          retractall(storedSecondoList(_))
        )
    ;   ( secondoResultFailed,
          fail
        )
  ), !.

secondo(X) :-
  sub_atom(X,0,3,_,S),
  atom_prefix(S,'let'),
  isDatabaseOpen, !, 
  ( secondo(X, Y)
    *-> ( secondoResultSucceeded(Y),
          retractall(storedSecondoList(_)),
          getSecondoList(ObjList),
          checkIfIndex(X, ObjList)
        )
    ;   ( secondoResultFailed,
          fail
        )
  ), !.
  
secondo(X) :-
  sub_atom(X,0,6,_,S),
  atom_prefix(S,'create'),
  sub_atom(X,0,15,_,S),
  not(atom_prefix(S,'create database')),
  isDatabaseOpen,  
  ( secondo(X, Y)
    *-> ( retractall(storedSecondoList(_)),
          secondoResultSucceeded(Y)
        )
    ;   ( secondoResultFailed,
          fail
        )
  ), !.

secondo(X) :-
  concat_atom([restore, database, DB1, from, _], ' ', X),
  notIsDatabaseOpen,
  downcase_atom(DB1, DB), !,
  ( secondo(X, Y)
    *-> ( secondoResultSucceeded(Y), 
          retractall(storedDatabaseOpen(_)),
          assert(storedDatabaseOpen(1)),
          retractall(databaseName(_)),
          assert(databaseName(DB)),
          retractall(storedSecondoList(_)),
          getSecondoList(ObjList1),
          checkIfSmallRelationsExist(ObjList1),
          getSecondoList(ObjList2),
          checkForAddedIndices(ObjList2),
          getSecondoList(ObjList3),
          checkForRemovedIndices(ObjList3)
        )
    ;   ( secondoResultFailed,
          fail
        )
  ), !.

secondo(X) :-
  concat_atom(['restore database', _, 'from', _], ' ', X),
  isDatabaseOpen,
  write('\nCannot restore database, because a database is open.\n'), !.

secondo(X) :- % variant to use during updateRel is processed
  concat_atom([Command, _],' ',X),
  Command = 'delete',
  storeupdateRel(1),  
  isDatabaseOpen,
  getSecondoList(ObjList),
  checkIsInList(X, ObjList, rel),
  secondo(X, _),
  retractall(storedSecondoList(_)), !.

secondo(X) :- % normal variant to delete a relation
  concat_atom([Command, Name],' ',X),
  Command = 'delete',
  storeupdateRel(0),
  isDatabaseOpen,
  getSecondoList(ObjList),
  checkIsInList(X, ObjList, rel),
  databaseName(DB),
  storedRel(DB, Name, _), 
  secondo(X, Y),
  retractall(storedSecondoList(_)),
  downcase_atom(Name, DCName),
  updateRel(DCName),  
  secondoResultSucceeded(Y), !.

secondo(X) :- % variant used when updating indexes
  concat_atom([Command, _],' ',X),
  Command = 'delete',
  storeupdateIndex(1), 
  isDatabaseOpen,
  getSecondoList(ObjList),
  indexType(Type),
  checkIsInList(X, ObjList, Type),
  secondo(X, _),
  retractall(storedSecondoList(_)),
  !.

secondo(X) :- % normal variant to delete an index
  concat_atom([Command, Name],' ',X),
  Command = 'delete',
  storeupdateIndex(0),
  isDatabaseOpen,
  databaseName(DB),
  getSecondoList(ObjList),
  indexType(Type),
  checkIsInList(X, ObjList, Type),
  storedIndex(DB, _, _, Type, Name),  
  secondo(X, Y),
  retractall(storedSecondoList(_)),
  updateIndex,  
  secondoResultSucceeded(Y), !.

secondo(X) :-
  concat_atom([Command, Name],' ',X),
  Command = 'list',
  Name = 'objects',
  isDatabaseOpen,
  secondo(X, Y),
  secondoResultSucceeded(Y), !.

secondo(X) :-
  sub_atom(X,0,5,_,S),
  atom_prefix(S,'query'),
  isDatabaseOpen, !,
  ( secondo(X, Y) 
    *-> ( secondoResultSucceeded(Y), !
        )
    ;   ( secondoResultFailed,
          fail
        )
  ), !.
  
secondo(X) :-
  ( secondo(X, Y) 
    *-> ( secondoResultSucceeded(Y), !,
          retractall(storedSecondoList(_))
        )
    ;   ( secondoResultFailed,
          fail
        )
  ), !.


/*

1.3 Operators ~query~, ~update~, ~let~, ~create~, ~open~, ~restore~ and ~delete~

The purpose of these operators is to make using the PROLOG interface
similar to using SecondoTTY. A SecondoTTY query

----    query ten
----

can be issued as

----    query 'ten'.
----

in the PROLOG interface via the ~query~ operator. The operators
~delete~, ~let~, ~create~, ~open~, and ~update~ work the same way.

*/
isDatabaseOpen :-
  storedDatabaseOpen(1), !.
  
isDatabaseOpen :-
  storedDatabaseOpen(0),
  write('No database open.'), nl,
  !, fail.

notIsDatabaseOpen :-
  storedDatabaseOpen(Status),
  Status = 0.


query(Query) :-
  query(Query, _).
 
query(Query, Time) :-
  isDatabaseOpen,
  atom(Query),
  atom_concat('query ', Query, QueryText), !,
  getTime( secondo(QueryText), Time).

let(Query) :-
  isDatabaseOpen,
  atom(Query),
  atom_concat('let ', Query, QueryText), !,
  secondo(QueryText).

derive(Query) :-
  isDatabaseOpen,
  atom(Query),
  atom_concat('derive ', Query, QueryText), !,
  secondo(QueryText).

create(Query) :-
  notIsDatabaseOpen,
  atom(Query),
  atom_concat('create ', Query, QueryText), !,
  secondo(QueryText).

create(Query) :-
  isDatabaseOpen,
  atom(Query),
  atom_concat('create ', Query, QueryText), !,
  secondo(QueryText).

update(Query) :-
  isDatabaseOpen,
  atom(Query),
  atom_concat('update ', Query, QueryText), !,
  secondo(QueryText).
    
delete(Query) :-
  notIsDatabaseOpen,
  atom(Query),
  atom_concat('delete ', Query, QueryText), !,
  secondo(QueryText).

delete(Query) :-
  isDatabaseOpen,
  atom(Query),
  atom_concat('delete ', Query, QueryText), !,
  secondo(QueryText).

open(Query) :-
  notIsDatabaseOpen,
  atom(Query),
  atom_concat('open ', Query, QueryText), !,
  secondo(QueryText).

restore(Query) :-
  notIsDatabaseOpen,
  atom(Query),
  atom_concat('restore ', Query, QueryText), !,
  secondo(QueryText).

:-
  op(800, fx, query),
  op(800, fx, delete),
  op(800, fx, let),
  op(800, fx, create),
  op(800, fx, open),
  op(800, fx, derive),
  op(800, fx, update),
  op(800, fx, restore).
 

/*
2.1 Generic display for printing formatted tables 

Sometimes we want to collect some information and to print it 
as a table, e.g. predicate ~writeSizes/0~. Below there are some
predicates which display lists of well defined tuples as specified
by a header list. If the data is presented in a prolog list
as [t1, t2, ... tn] and ti are itself prolog lists of fixed length
each value will be printed in a separate column. Example application:

----
    findall(X, createMyTuples(_, X), L ),
    TupFormat = [ ['Size', 'c'], 
                  ['Time', 'l'], 
                  ['Error', 'l'] ],
    showTuples(L, TupFormat). 
----

*/
 
showTuples(L, TupFormat) :-
  nl, nl, 
  showHeader(TupFormat, WriteSpec),  
  showTuplesRec(L, WriteSpec).
 
showTuplesRec([], _).

showTuplesRec([H|T], WriteSpec) :-
  showTupleRec(H, WriteSpec), nl,
  showTuplesRec(T, WriteSpec).

showTupleRec([], _).

showTupleRec([H|T], [Wh|Wt]) :- 
  writef(Wh, [H]),
  showTupleRec(T, Wt).

showHeader(L, WriteSpec) :-
  showHeaderRec(L, [], HeadList, [], WriteSpec, 0, Len),
  showTuplesRec([HeadList], WriteSpec),
  Len2 is Len + 2,
  writef('%r', ['-', Len2]), nl.
 
showHeaderRec([], L1, L1, L2, L2, N, N).
 
showHeaderRec([H|T], Tmp1, Res1, Tmp2, Res2, Tmp3, Res3 ) :-
  H = [Attr, Adjust],
  atom_length(Attr, Len),
  FieldLen is Len + 4,
  TotalLen is Tmp3 + FieldLen,
  concat_atom([' %', FieldLen, Adjust], WriteSpec),
  append(Tmp1, [Attr], L1),
  append(Tmp2, [WriteSpec], L2),
  showHeaderRec(T, L1, Res1, L2, Res2, TotalLen, Res3 ).

/*
2.2 The built in predicate ~portray~

Pretty printing of user defined operators. The built in dynamic predicate
~portray/1~ is used by the ~print~ predicate. It can be used to define how
Terms should be typed. We use this to add spaces for prefix infix and postfix
operators.

*/
 
portray(Term) :-
  Term =.. [F | [Arg1, Arg2] ],
  current_op(_, xfx, F),
  print(Arg1), write(' '), write(F), write(' '), print(Arg2).
  
portray(Term) :-
  Term =.. [F | Arg1],
  current_op(_, fx, Term),
  write(F), write(' '), print(Arg1).

portray(Term) :-
  Term =.. [F | Arg1],
  current_op(_, xf, Term),
  print(Arg1), write(' '), write(F).

portray(Term) :-
  write(Term).
