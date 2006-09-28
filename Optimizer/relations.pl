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

August 2006, M. Spiekermann. Initial version. 

1 Overview

This file provides clauses which are helpful to store collected
meta data like size estimations costs into relations located in a
secondo database.

The interface is presented below
----
     relAttrs
     relValue
     clearRel     
----     

Afterwards you can use
----
    saveRel
    showRel
----

The first one is used ro store a collection of tuples which into a
relation object in SECONDO. The second just displays the tuples on the
screen.

*/


% store all queries of a session in the dynamic predicate below      
:- dynamic
     queryText/8.
 
relAttrs('SqlHistory',  [ ['QueryId', 'int'], 
                          ['SqlText', 'text'], 
                          ['QueryPlan', 'text'], 
                          ['JoinPreds', 'int'], 
                          ['SelPreds', 'int'],
                          ['Costs', 'real'], 
                          ['PlanBuild', 'int'],
                          ['PlanExec', 'int'] ] ).
 

relAttrs('NodeSizes', [ ['Qid', 'int'],
                        ['Node', 'int'], 
                        ['SizeStd', 'real'], 
                        ['SizeEntropy', 'real'], 
                        ['SizeReal', 'real']     ] ).


appendToRel('SqlHistory', Term, Plan, Cost, PlanBuild, PlanExec) :-
  nextCounter(qid, Qid),
  getCounter(joinPred, Joins),
  getCounter(selectionPred, Sels),
  resetCounter(joinPred),
  resetCounter(selectionPred),
  swritef(S, '%t', [Term]),
  string_to_atom(S, Sql),
  quoteText(Sql, Sql2),
  quoteText(Plan, Plan2),
  assert( queryText(Qid, Sql2, Plan2, Joins, Sels, Cost, PlanBuild, PlanExec) ).

                         
relValue('SqlHistory', Tuples) :-
  findall( [Nr, Sql, Plan, Joins, Sels, Costs, T1, T2], 
           queryText(Nr, Sql, Plan, Joins, Sels, Costs, T1, T2),
           Tuples ).


relValue('NodeSizes', Tuples) :-
  getCounter(qid, Qid),
  findall([Qid, N, S1, S2, S3], createNodeSize(N, S1, S2, S3), Tuples).

 

relValue(Name, []) :-
  nl, write('Error: Meta data relation '), 
  write(Name), writeln(' unknown!'),
  fail.


clearRel('SqlHistory') :-
  retractall( queryText(_,_,_,_,_,_,_,_) ).
 
clearRel('NodeSizes').
% tuples are computed and not stored in a dynamic clauses


saveRel(Name) :-
  saveRel(Name, Name).
 
saveRel(Name, Name2) :-
  relAttrs(Name, Attrs),
  makeRelType2(Attrs, Schema), !,
  relValue(Name, Tuples), !,
  Tuples \= [],
  convertList(Tuples, '', SecTuples),
  %retractall(storedSecondoList(_)),
  getSecondoList(ObjList),
  runCmd(ObjList, Name2, Schema, SecTuples), !,
  clearRel(Name).

 
 
/*
Convert the list of attributes into a nested list of [rel [ tuple ...]].

*/
 
mkRelType(Attrs, [rel, [tuple, Attrs]]).


showRel(Name) :-
  relAttrs(Name, Attrs),
  mkRelType(Attrs, RelType),
  relValue(Name, Tuples),
  display(RelType, Tuples).

showRel(Name, tab) :-
  relAttrs(Name, Attrs),
  mkTupFormat(Attrs, TupFormat),
  relValue(Name, Tuples),
  showTuples(Tuples, TupFormat).


mkTupFormat([H|T], [NewH|NewT]) :-
 H = [ A, _ ],
 NewH = [ A, 'l' ],
 mkTupFormat(T, NewT).
 
mkTupFormat([], []).

 
runCmd(ObjList, Obj, S, V) :-
  not(findRelObj(ObjList, Obj)), 
  nl, write('Object not present! Running let comamnd.'),
  makeRelCmd(Obj, S, V, Cmd),
  nl, writeln(Cmd),
  secondo(Cmd).
 
runCmd(_, Obj, S, V) :-
  nl, write('Object present! Running update comamnd.'),
  appendToRelCmd(Obj, S, V, Cmd),
  nl, writeln(Cmd),
  secondo(Cmd).
 
/*
Search a relation with name ~Rel~ in the object list ~L~
instantiated by getSecondoList(L).
 
*/

findRelObj([], _) :-
  fail, !.

findRelObj([H|_], Rel) :-
  H = ['OBJECT', Rel, _, [[rel, [_|_]]]],
  atom(Rel), !.
  
findRelObj([_|T], Obj) :-
  findRelObj(T, Obj).


/*
Predicate ~convertList/3~ converts a nested prolog list into an atom which
contains a nested list in SECONDO format.

*/
 

convertList(L, Tmp, ResAtom) :-
  convertListRec(L, Tmp, TmpRes),
  parenthesize(TmpRes, ResAtom).

convertListRec([], Tmp, Tmp).

convertListRec([H|T], Tmp, ResAtom) :-
  is_list(H),
  convertListRec(H, '', Res1),
  parenthesize(Res1, Presult),
  atom_concat(Tmp, Presult, TmpAtom),
  convertListRec(T, TmpAtom, ResAtom).
  
convertListRec([H|T], Tmp, ResAtom) :- 
  concatWithSpace(Tmp, H, Tmp2),
  convertListRec(T, Tmp2, ResAtom).

/*
Some helper predicates for atom costruction.

*/
 
concatWithSpace(Atom1, Atom2, Result) :-
  atom_concat(Atom1, ' ', Tmp),
  atom_concat(Tmp, Atom2, Result).
 
appendWithTab(Atom1, Atom2, Result) :-
  atom_concat(Atom1, Atom2, Tmp),
  atom_concat(Tmp, '\t', Result).

 
parenthesize(Atom, Result) :-
  atom_concat('(', Atom, Tmp),
  atom_concat(Tmp, ')',  Result).

parenthesize2(Atom, Result) :-
  atom_concat('[', Atom, Tmp),
  atom_concat(Tmp, ']',  Result).
 
quoteText(Atom, Result) :-
 atom_concat('<text>', Atom, Tmp),
 atom_concat(Tmp, '</text--->',  Result).


 

/*

Auxiliary clauses for creating SECONDO ~let~ and ~update~ 
comamnds. Therefore the tuple type must be written as 

----
 rel(tuple([attr1: type1, ..., attrN, typeN])
---- 

*/
 
makeRelType([],Tmp, Tmp).
 
makeRelType([H|T], Tmp, TypeAtom) :-
  H = [Attr, Type],
  atom_concat(Tmp, Attr, A0),
  atom_concat(A0, ':', A1), 
  atom_concat(A1, ' ', A2), 
  atom_concat(A2, Type, A3),
  concatComma(T, A3, A4),
  makeRelType(T, A4, TypeAtom). 
  
concatComma([_|_], Atom, Result) :-
  atom_concat(Atom, ', ', Result).
 
concatComma(_, Atom, Atom).
 
makeRelType2(List, TypeAtom) :-
  makeRelType(List, '', Tmp1),
  atom_concat('rel(tuple([', Tmp1, Tmp2), 
  atom_concat(Tmp2, ']))', TypeAtom). 
 

makeRelCmd(ObjName, RelSchema, Values, ResultCmd) :-
  makeRelConstant(RelSchema, Values, RelConstant),
  atom_concat('let ', ObjName, Q1), 
  atom_concat(Q1 , ' = ', Q2 ),
  atom_concat(Q2 , RelConstant, ResultCmd ). 

makeRelConstant(RelSchema, Values, Result) :-
  atom_concat('[const ', RelSchema, Q1 ),
  atom_concat(Q1 , ' value ', Q2 ), 
  atom_concat(Q2 , Values, Q3),
  atom_concat(Q3 , ']', Result).


/*
We will update an existing relation by concatenating it with a 
relation constant.

*/
 
appendToRelCmd(ObjName, RelSchema, Values, ResultCmd) :- 
  makeRelConstant(RelSchema, Values, RelConstant),
  atom_concat('update ', ObjName, Q1), 
  atom_concat(Q1 , ' := ', Q2 ),
  atom_concat(Q2 , ObjName, Q3 ),
  atom_concat(Q3 , ' feed ', Q4 ),
  atom_concat(Q4 , RelConstant, Q5),
  atom_concat(Q5 , ' feed concat consume', ResultCmd ). 

