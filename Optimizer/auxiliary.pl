/*

----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

----	display(Type, Value) :-
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




/*

1.2 Predicate ~secondo~


Predicate ~secondo~ expects its argument to be a string atom or
a nested list, representing a query to the SECONDO system. The query is
executed and the result pretty-printed. If the query fails, the error code
and error message are printed.

---- secondo(+SecondoExecutableExpression)
----

Send ~SecondoExecutableExpression~ to the Secondo kernel (using secondo/2) and
print the returned result onto the screen.

Keep the optimizer informed by tracking 'open', 'close', 'restore', 'let' and
'delete' commands within ~SecondoExecutableExpression~.

---- secondo_direct(+SecondoExecutableExpression)
----

Send ~SecondoExecutableExpression~ to the Secondo kernel (using secondo/2) and
print the returned result onto the screen.

Do NOT keep the optimizer informed by tracking 'open', 'close', 'rstore',
'let' or 'delete' commands within ~SecondoExecutableExpression~.

This variant is useful within automaically triggered queries to secondo, e.g.
deleting or creating objects used by the optimizer (like samples, indexes, small
objects). This command should never be used by the user!

*/


% Such facts are used to mark a file has been considered by some predicate:
:- dynamic(tempConsideredFile/1).


% atom_postfix(+Atom, +PrefixLength, ?Post)
% succeeds iff Post is a postfix of Atom starting after PrefixLength
atom_postfix(Atom, PrefixLength, Post) :-
  atom_length(Atom, Length),
  PostLength is Length - PrefixLength,
  sub_atom(Atom, PrefixLength, PostLength, 0, Post).


/*
Some facts which store important state information

---- databaseName(X)
----

When a database X gets openend, a fact databaseName(X) gets asserted.
If no such fact exists, no database is opened. The fact is used to get the name
of the currently opened database.

*/
:- assert(helpLine(secondo,1,
    [[+,'CommandString',
        'Command to send as a string (enclose in single quotes).']],
    'Send a command to the DBMS kernel.')).

:- dynamic databaseName/1.

promptSecondoResultSucceeded(Result) :-
  write('Command succeeded, result:'),
  nl, nl,
  show(Result), !.

promptSecondoResultFailed :-
  secondo_error_info(ErrorCode, ErrorString),
  write('Command failed with error code : '),
  write(ErrorCode), nl,
  write('and error message : '), nl,
  write(ErrorString), nl, !.

secondo(X) :-
  sub_atom(X,0,_,_,'open '),
  atom_postfix(X, 14, DB1),
  downcase_atom(DB1, DB), !,
  ( secondo(X, Y)
    *-> ( promptSecondoResultSucceeded(Y), !,
          retractall(databaseName(_)),
          assert(databaseName(DB)),
          readSystemIdentifiers,
          readSecondoTypeSizes,
          refreshSecondoCatalogInfo,
          checkObjectNamingConvention,
          updateCatalog,
          updateCatalog
%         ,ensureSmallObjectsExist
        )
    ;   ( promptSecondoResultFailed,
          fail
        )
  ), !.

secondo(X) :-
  sub_atom(X,0,_,_,'close'), !,
  ( secondo(X, Y)
    *-> ( promptSecondoResultSucceeded(Y),
          retractall(databaseName(_))
        )
    ;   ( promptSecondoResultFailed,
          fail
        )
  ), !.

secondo(X) :-
  sub_atom(X,0,6,_,'update '),
  isDatabaseOpen, !,
  ( secondo(X, Y)
    *-> promptSecondoResultSucceeded(Y)
    ;   ( promptSecondoResultFailed,
          fail
        )
  ), !.

secondo(X) :-
  member(Command,['let ', 'derive ', 'delete ']),
  sub_atom(X,0,CommandLength,_,Command),
  ( notIsDatabaseOpen
    -> ( write('\nCannot execute command, because no database is open.\n'),
         !, fail
       )
    ;  true
  ),
  ( member(Command,['let ', 'derive ']) % if this is a let/derive command,
    -> ( % we need to test, whether the (downcased) objectname is not
         % already used in the DB and the name is a valid itentifier
         sub_atom(X,PosEq,1,_,'='),
         Namelength is PosEq - CommandLength, !,
         sub_atom(X,CommandLength,Namelength,_,ExtObjNameA),
         removeWhitespace(ExtObjNameA,ExtObjName),
         dcName2externalName(DCobjName,ExtObjName),
         ( secondoCatalogInfo(DCobjName,ExtOther,_,_) % already used?
           -> ( write_list(['\nERROR:\tCannot create object \'',
                           ExtObjName,'\'.\n','--->\tThere already ',
                          'exits an object named \'',ExtOther,'\'!\n']),
                nl,
                !, fail
              )
           ;  ( validIdentifier(ExtObjName)        % identifier valid?
                -> true
                ;  ( write_list(['\nERROR:\tCannot create object \'',
                            ExtObjName,'\'.\n',
                            '--->\tInvalid identifier!\n']),
                     nl,
                     !, fail
                   )
              )
         )
       )
    ;  true       % if this is a delete command
  ),
  ( secondo(X, Y)
    *-> ( promptSecondoResultSucceeded(Y),
          updateCatalog,
          updateCatalog
%         ,ensureSmallObjectsExist
%         ,ensureSamplesExist
        )
    ;   ( promptSecondoResultFailed,
          fail
        )
  ),
  !.

secondo(X) :-
  sub_atom(X,0,_,_,'create '),
  not(my_concat_atom([create, database, _], ' ', X)),
  isDatabaseOpen,
  ( secondo(X, Y)
    *-> promptSecondoResultSucceeded(Y)
    ;   ( promptSecondoResultFailed,
          fail
        )
  ), !.

% restore database
% One should consider wiping out the knowledge base on the target database!
secondo(X) :-
  my_concat_atom([restore, database, DB1, from, _], ' ', X),
  ( notIsDatabaseOpen
    ->  ( downcase_atom(DB1, DB), !,
         ( secondo(X, Y)
           *-> ( promptSecondoResultSucceeded(Y),
                 retractall(databaseName(_)),
                 assert(databaseName(DB)),
                 readSystemIdentifiers
               )
           ;   ( promptSecondoResultFailed,
                 fail
               )
         )
        )
    ; write('\nERROR:\tCannot restore database, because a database is open.\n')
  ),
  updateCatalog,
  updateCatalog,
% ensureSmallObjectsExist,
  !.

% restore object
secondo(X) :-
  my_concat_atom([restore, _, from, _], ' ', X),
  ( isDatabaseOpen
    ->  write('\nERROR:\tCannot restore object, because no database is open.\n')
    ;   ( secondo(X, Y)
           *-> ( promptSecondoResultSucceeded(Y)
               )
           ;   ( promptSecondoResultFailed,
                 fail
               )
        )
  ),
  updateCatalog,
  updateCatalog,
%  ensureSmallObjectsExist,
  !.

secondo(X) :-
  my_concat_atom([list, objects],' ',X),
  isDatabaseOpen,
  secondo(X, Y),
  promptSecondoResultSucceeded(Y), !.

secondo(X) :-
  sub_atom(X,0,_,_,'query'),
  isDatabaseOpen, !,
  ( secondo(X, Y)
    *-> ( promptSecondoResultSucceeded(Y), !
        )
    ;   ( promptSecondoResultFailed,
          fail
        )
  ), !.

% fallback-case (all other commands)
secondo(X) :-
  ( secondo(X, Y)
    *-> ( promptSecondoResultSucceeded(Y), !)
    ;   ( promptSecondoResultFailed,
          fail
        )
  ), !.

% Variant of execulting a command on the Secondo kernel
% without updating the internal optimizer knowledge base.
% Not to be used by the user!
secondo_direct(X) :-
  ( secondo(X, Y)
    *-> ( promptSecondoResultSucceeded(Y), !)
    ;   ( promptSecondoResultFailed,
          fail
        )
  ), !.

/*

1.4 Operators ~query~, ~update~, ~let~, ~create~, ~open~, ~restore~ and ~delete~

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

:- assert(helpLine(let,1,
    [[+,'ExecQuery','Partial command in Secondo syntax.']],
    'Prepend \'query \' and send to DBMS-kernel.')).
:- assert(helpLine(query,1,
    [[+,'ExecQuery','Partial command in Secondo syntax.']],
    'Prepend \'query \' and send to DBMS-kernel.')).
:- assert(helpLine(derive,1,
    [[+,'ExecQuery','Partial command in Secondo syntax.']],
    'Prepend \'derive \' and send to DBMS-kernel.')).
:- assert(helpLine(create,1,
    [[+,'ExecQuery','Partial command in Secondo syntax.']],
    'Prepend \'create \' and send to DBMS-kernel.')).
:- assert(helpLine(update,1,
    [[+,'ExecQuery','Partial command in Secondo syntax.']],
    'Prepend \'update \' and send to DBMS-kernel.')).
:- assert(helpLine(delete,1,
    [[+,'ExecQuery','Partial command in Secondo syntax.']],
    'Prepend \'delete \' and send to DBMS-kernel.')).
:- assert(helpLine(open,1,
    [[+,'ExecQuery','Partial command in Secondo syntax.']],
    'Prepend \'open \' and send to DBMS-kernel.')).
:- assert(helpLine(restore,1,
    [[+,'ExecQuery','Partial command in Secondo syntax.']],
    'Prepend \'restore \' and send to DBMS-kernel.')).


:- op(993,  fx, open).
:- op(993,  fx, restore).
:- op(959,  fx, database).
%:- op(960, xfx, from).         % already defined in file optimizer.pl
:- op(993, fx, query).
:- op(993, fx, delete).
:- op(993, fx, let).
:- op(993, fx, create).
:- op(993, fx, derive).
:- op(993, fx, update).


isDatabaseOpen :-
  ( databaseName(_)
    -> true
    ;  ( write('No database open.'), nl, fail)
  ), !.

notIsDatabaseOpen :-
  not(databaseName(_)), !.

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
  my_concat_atom(['let', Query], ' ', QueryText), !,
  secondo(QueryText).

derive(Query) :-
  isDatabaseOpen,
  atom(Query),
  atom_concat('derive ', Query, QueryText), !,
  secondo(QueryText).

create(Query) :-
  atom(Query),
  atom_concat('create ', Query, QueryText), !,
  secondo(QueryText).

update(Query) :-
  isDatabaseOpen,
  atom(Query),
  atom_concat('update ', Query, QueryText), !,
  secondo(QueryText).

delete(Query) :-
  atom(Query),
  atom_concat('delete ', Query, QueryText), !,
  secondo(QueryText).

% more comfortable (without quoting)
open(Query) :-
  notIsDatabaseOpen,
  Query =.. [database, DB],
  atom(DB),
  atom_concat('open database ', DB, QueryText), !,
  secondo(QueryText),
	dropTempRels.

% the hard way...
open(Query) :-
  notIsDatabaseOpen,
  atom(Query),
  atom_concat('open ', Query, QueryText), !,
  secondo(QueryText),
	dropTempRels.

% the hard way...
restore(Query) :-
  notIsDatabaseOpen,
  atom(Query),
  atom_concat('restore ', Query, QueryText), !,
  secondo(QueryText).

% more comfortable (without quoting)
restore(Query) :-
  Query =.. [from,Obj,Source],
  ( ( atom(Source), Obj =.. [database, DB], atom(DB) ) % restore database
    -> ( notIsDatabaseOpen
         -> my_concat_atom(['restore database',DB,'from',Source],' ',QueryText)
         ; ( write_list(['\nERROR:\tCannot restore a database, \n',
                         '--->\twhile a database is open!']), nl,
             !, fail
           )
       )
    ;  ( ( atom(Obj), term_to_atom(Source, SourceA) ) % restore object
         -> ( notIsDatabaseOpen
              -> (write_list(['\nERROR:\tCannot restore object.\n',
                              '--->\tNo database open.']), nl, !, fail)
              ;  my_concat_atom(['restore ',Obj,'from',SourceA],' ',QueryText)
            )
         ;  ( write_list(['\nERROR:\tInvalid restore command.\n',
                   '\tThe correct syntax is \'restore <objname> from <file>.\'',
                   '\n\tor \'restore database <dbname> from <file>.\'.']), nl,
              !, fail
            )
       )
  ),
  secondo(QueryText).

/*
The keywords open and close are already assigned with operations
on streams. Hence we provide ~openDB~ and closeDB to close a database.

*/

:-assert(helpLine(cdb,0,[],'Close current database.')).
:-assert(helpLine(closeDB,0,[],'Close current database.')).

cdb :- closeDB.
closedb :- closeDB.
closeDB :-
  secondo('close database').

odb(Name) :- openDB(Name).
openDB(Name) :-
  atom_concat('open database ', Name, Cmd),
  secondo(Cmd),
	dropTempRels.

:-assert(helpLine(ldb,0,[],'List available databases.')).
:-assert(helpLine(listDB,0,[],'List available databases.')).

ldb :- listDB.
listDB :-
  secondo('list databases').

:-assert(helpLine(lo,0,[],'Send \'list objects\'.')).
:-assert(helpLine(listObj,0,[],'Send \'list objects\'.')).

lo :- listObj.
listObj :-
  secondo('list objects').

% search for a description of operator O

:- assert(helpLine(findop,1,
    [[+,'OperatorName','The name of the operator to query for.']],
    'Show help on a given Secondo operator.')).

findop(O) :-
  my_concat_atom([ 'query SEC2OPERATORINFO feed filter[.Name contains "',
                O, '"] consume' ], Q),
  runQuery(Q).

% dump the command history to a given file name

:- assert(helpLine(cmdHist2File,1,
    [[+,'FileName','The file used for storing the history.']],
    'Dump the command history to a file.')).
cmdHist2File(Name) :-
  my_concat_atom(['query SEC2COMMANDS feed '], Q),
  dumpQueryResult2File(Q, Name, Q2),
  runQuery(Q2).

% dump the result of a secondo query to a CSV file
dumpQueryResult2File(Q, File, Q2) :-
  my_concat_atom([Q, ' dumpstream["', File, '","|"] tconsume'], Q2).

% removing whitespace (tab, space) from an atomic
removeWhitespace(ExtObjNameA,ExtObjName) :-
  atom_chars(ExtObjNameA,CharListDirty),
  delete(CharListDirty,' ',CharListDirty1),
  delete(CharListDirty1,'\t',CharListCleaned),
  atom_chars(ExtObjName,CharListCleaned),
  !.

/*
Useful for debugging

*/

showValue(Name, Var) :-
  write(Name), write(': '), write(Var), nl, nl.


mark(X) :-
  write('(* Mark '), write(X), write(' *)'), nl.

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
  %format('~w~t~+~w~t~+~w~t~+~w~n',H),
  showTuplesRec(T, WriteSpec).

showTupleRec([], _).

showTupleRec([H|T], [Wh|Wt]) :-
  %showValue('Wh', Wh),
  writef(Wh, [H]),
  %format('~w~t ~+~w~t ~+~w~t~ +~w',[H]),
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
  my_concat_atom([' %', FieldLen, Adjust], WriteSpec),
  append(Tmp1, [Attr], L1),
  append(Tmp2, [WriteSpec], L2),
  showHeaderRec(T, L1, Res1, L2, Res2, TotalLen, Res3 ).


/*
Write a list of tuples to a file. The members of an inner
list are separated by a comma.

*/

writeElem(FD, []) :-
  write(FD, '\n').

writeElem(FD, [H | T]) :-
  write(FD, H),
  ( length(T, 0) -> write(FD, '  ')
                 ;  write(FD, ', ') ),
  writeElem(FD, T).

dumpTuples2File(Name, T, Format) :-
  project(1, Format, Fp),
  append([Fp], T, L),
  showValue('L:', L),
  open(Name, write, FD),
  checklist( writeElem(FD), L),
  close(FD).

/*
Extract the N-th element out of all tuples given in list
L and unify them with R.

*/

project(N, L, R) :-
  maplist(nth1(N), L, R).


/*
A predicate which translates the first letter of an atom to
lower case.

*/

downcase_first(Atom, Res) :-
  atom_chars(Atom, [H | T]),
  downcase_atom(H, Hdown),
  append([Hdown], T, L),
  name(Res, L).



/*
2.3 ~subList/3~

----
  subList(+L, +N, ?Res)
----

Unifies the first ~N~ elements of List ~L~ with ~Res~.

*/

subList(L, N, Res) :-
 subListRec(L, N, 1, Res).

subListRec([H|_], N, N, [H]).
subListRec([H|T], N, Pos, [H|T2]) :-
  NewPos is Pos + 1,
  subListRec(T, N, NewPos, T2).


/*
2.4 Runtime Flags

the dynamic predicate ~flag~ a tool for setting and querying options.
This is useful to set global options.

*/

:- dynamic flag/2,
   clearflags.

clearflags :-
  retractall( flag(_,_) ).

setflag(F) :- assert( flag(F, on) ).
clearflag(F) :- retractall( flag(F, _) ).

showflag( [F, Val], _ ) :-
  nl, write(F), write(' --> '), write(Val), nl.


showflags :-
  findall( [X, Y], flag(X, Y), L),
  maplist( showflag, L, _).

/*
2.5 Running Queries

The clause ~runQuery~ can be used to execute or just print queries
depending on the flag ~runMode~.

*/

showQuery(Q) :-
  nl, write(Q), nl.

runQuery(Q) :-
  flag(runMode, on),
  nl, write('Executing '), write(Q), write(' ...'), nl,
  secondo(Q), !.

runQuery(Q) :-
  showQuery(Q).


runQuery(Q, Res) :-
  flag(runMode, on),
  nl, write('Executing '), write(Q), write(' ...'), nl,
  secondo(Q, [_, Res]),
  nl, write('Result: '), write(Res), nl, !.

runQuery(Q, Res) :-
  showQuery(Q),
  Res = 999,
  nl, write('Dummy-Result: '), write(Res), nl.

/*
3.0 Stop Watch Predicate

---- time(+Clause)
----

This predicate stops the time required to answer query ~Clause~ and prints the
needed time on screen.

*/

time(Clause) :-
  get_time(Time1),
  (call(Clause) ; true),
  get_time(Time2),
  Time is Time2 - Time1,
  my_convert_time(Time, _, _, _, _, Minute, Sec, MilliSec),
  MSs is Minute *60000 + Sec*1000 + MilliSec,
  write('Elapsed Time: '),
  write(MSs),
  write(' ms'),nl.

/*
End of file ~auxiliary.pl~

*/
