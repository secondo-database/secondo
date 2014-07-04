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

October 2009, Mahmoud Sakr. Initial Version

//paragraph [10] title: [{\Large \bf ]  [}]
//characters [1] formula:       [$]     [$]
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[**] [$**$]
//[star] [$*$]
//[->] [$\rightarrow$]
//[toc] [\tableofcontents]
//[=>] [\verb+=>+]
//[<] [$<$]
//[>] [$>$]
//[%] [\%]

//[newpage] [\newpage]

[10] Defining Faked Operators

[File ~fekedoperators.pl~]

[toc]

[newpage]

*/

/*

1 Operator isknn

*/
validate_isknn_input(K, QueryObject, Rel, MPointAttr, IDAttr, RebuildIndexes):-
  ( (integer(K), K >0 )
    -> true
    ; ( write_list(['\nERROR:\tThe second parameter (k) in the'
                    ' iskNN predicate is expected to be an Integer'
                    ' > 0\n but got: ', K], ErrMsg),
        throw(error_Internal(optimizer_validate_isknn_input(K, QueryObject,
                           Rel, MPointAttr, IDAttr, 
                           RebuildIndexes)::malformedExpression::ErrMsg)),
        fail
      )
  ),
  ( QueryObject= dbobject(QueryObj)
    -> true
    ; ( write_list(['\nERROR:\tThe third parameter (QueryObject) in '
                    'the iskNN predicate is expected to be a database '
                    'object\n but got: ', QueryObject], ErrMsg),
        throw(error_Internal(optimizer_validate_isknn_input(K, QueryObject, 
                             Rel, MPointAttr, IDAttr, 
                            RebuildIndexes)::malformedExpression::ErrMsg)),
        fail
      )
  ),
  ( memberchk(RebuildIndexes, [0, 1])
    ; ( write_list(['\nERROR:\tThe last parameter (RebuildIndexes) '
                    'in the iskNN predicate is expected to be 0 '
                    'or 1\n but got: ', RebuildIndexes], ErrMsg),
        throw(error_Internal(optimizer_validate_isknn_input(K, QueryObject, 
                 Rel, MPointAttr, IDAttr, 
                 RebuildIndexes)::malformedExpression::ErrMsg)),
        fail
      )
  ),
  getknnDCNames(K, QueryObj, Rel, MPointAttr, IDAttr,
    _, DCRel, DCMPointAttr, DCIDAttr, _, _, _, _, _),

  ( (
      relation(DCRel, _),
      attrType(DCRel:DCMPointAttr, mpoint),
      attrType(DCRel:DCIDAttr, _)
    )
    ;
    (
      write_list(['\nERROR:\tThe 4th, 5th, and 6th parameters '
                  '(Relation, Attribute, and IDAttribute ) in the iskNN'
                  ' predicate are expected to refer to exisiting Relation '
                  'and Attribute names\n but got: ',
      Rel, ", ", MPointAttr, ", and ", IDAttr], ErrMsg),
      throw(error_Internal(optimizer_validate_isknn_input(K, 
                     QueryObject, Rel, MPointAttr, IDAttr, 
                     RebuildIndexes)::malformedExpression::ErrMsg)),
      fail
    )
  ).

knearest(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr, RebuildIndexes):-
  getknnDCNames(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr,
    _, _, _, _, DCUnitRel, _, _, _, _),
  (RebuildIndexes = 0 ; rebuildKnearestIndexes(K, DCQueryObj, DCRel, 
   DCMPointAttr, DCIDAttr)),
  (relation(DCUnitRel, _) %check the columns too
     -> runknearest(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr)
     ;  (  createUnitRelation(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr),
           runknearest(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr)
        )
  ).

runknearest(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr):-
  getknnExtNames(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr,
    ExtQueryObj, _, _, ExtIDAttr, ExtUnitRel, ExtUnitAttr, 
    ExtResultRel, ExtMBoolAttr, _),
  getknnDCNames(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr,
    _, _, _, DCIDAttr, _, _, DCResultRel, _, _),

  (relation(DCResultRel, _) %check the attributes
   ;(   my_concat_atom(['let ', ExtResultRel, ' = ', ExtUnitRel, 
      ' feed head[300] knearest[',
      ExtUnitAttr, ', ', ExtQueryObj, ', ', K, '] sortby[', ExtIDAttr,
			' asc] groupby[', ExtIDAttr, '; ', ExtMBoolAttr,
      ': mint2mbool(group feed ',
			'extend[Tmp: periods2mint(deftime(.', ExtUnitAttr,
      '))] aggregateB[Tmp; fun(x: mint, y: mint) x + y; zero()]) ] consume'],
      '', Query),
	nl, write('Creating the nearest neighbor relation '- Query ), nl,
	secondo(Query),
	createIndex(DCResultRel, DCIDAttr, btree)
    )).

createUnitRelation(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr):-
  getknnExtNames(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr,
    _, ExtRel, ExtMPointAttr, ExtIDAttr, ExtUnitRel, ExtUnitAttr, _, _, _),
  getknnDCNames(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr,
    _, _, _, _, DCUnitRel, _, _, _, _),

  not(relation(DCUnitRel, _)), %check the attributes
  my_concat_atom(['let ', ExtUnitRel, ' = ', ExtRel, 
              ' feed projectextendstream[', ExtIDAttr, ';',
              ExtUnitAttr, ': units( .', ExtMPointAttr, ')] sortby[', 
              ExtUnitAttr, '] consume'], '', Query),
  secondo(Query).


rebuildKnearestIndexes(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr):-
  getknnDCNames(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr,
    _, _, _, DCIDAttr, DCUnitRel, _, DCResultRel, _, _),
  databaseName(DB),
  (not(storedIndex(DB, DCResultRel, DCIDAttr, btree, _)); 
  dropIndex(DCResultRel, DCIDAttr, btree)),
  (not(storedRel(DB, DCUnitRel, _)); drop_relation(DCUnitRel)),
  (not(storedRel(DB, DCResultRel, _)); drop_relation(DCResultRel)).

/*
The predicates gives names for the temporary database object created
during the evaluation of the isknn operator. The names are evalauted
here and propagated as arguments in the whole cycle of evaluation. In
other words, if you need to change the way of composing the names, you
need only to change these functions.

getknnDCNames(+K, +QueryObj, +Rel, +MPointAttr, +IDAttr,
    -DCQueryObj, -DCRel, -DCMPointAttr, -DCIDAttr, -DCUnitRel, -DCUnitAttr, -DCResultRel, -DCMBoolAttr, -DCBtree).

getknnExtNames(K, QueryObj, Rel, MPointAttr, IDAttr,
    ExtQueryObj, ExtRel, ExtMPointAttr, ExtIDAttr, ExtUnitRel, ExtUnitAttr, ExtResultRel, ExtMBoolAttr, ExtBtree).

where the schema of the involved tables look as follows:
the query database object = dbobject(ExtQueryObj)
the moving points table = ExtRel[ExtIDAttr, ExtMPointAttr, ... ]
the units table= ExtUnitRel[ExtIDAttr, ExtUnitAttr]
the result table= ExtResultRel[ExtIDAttr, ExtMBoolAttr]
the Btree= ExtBtree[ExtIDAttr]

*/

getknnExtNames(K, QueryObj, Rel, MPointAttr, IDAttr,
    ExtQueryObj, ExtRel, ExtMPointAttr, ExtIDAttr, ExtUnitRel, 
    ExtUnitAttr, ExtResultRel, ExtMBoolAttr, ExtBtree):-
  downcase_atom(QueryObj, DCQueryObj),
  downcase_atom(Rel, DCRel),
  downcase_atom(MPointAttr, DCMPointAttr),
  downcase_atom(IDAttr, DCIDAttr),
  dcName2externalName(DCRel, ExtRel),
  dcName2externalName(DCRel:DCMPointAttr, ExtMPointAttr),
  dcName2externalName(DCRel:DCIDAttr, ExtIDAttr),
  dcName2externalName(DCQueryObj, ExtQueryObj),
  my_concat_atom(['Unit', ExtRel], '', ExtUnitRel),
  my_concat_atom(['U', ExtMPointAttr], '', ExtUnitAttr),
  my_concat_atom([ExtRel, ExtMPointAttr, ExtQueryObj, K, 'NN'], '', 
                 ExtResultRel),
  ExtMBoolAttr = 'MBoolRes',
  my_concat_atom([ExtResultRel, ExtIDAttr, 'btree'], '_', ExtBtree).

getknnDCNames(K, QueryObj, Rel, MPointAttr, IDAttr,
    DCQueryObj, DCRel, DCMPointAttr, DCIDAttr, DCUnitRel, DCUnitAttr, 
    DCResultRel, DCMBoolAttr, DCBtree):-
  getknnExtNames(K, QueryObj, Rel, MPointAttr, IDAttr,
    ExtQueryObj, ExtRel, ExtMPointAttr, ExtIDAttr, ExtUnitRel, ExtUnitAttr, 
    ExtResultRel, ExtMBoolAttr, ExtBtree),
  downcase_atom(ExtQueryObj, DCQueryObj ),
  downcase_atom(ExtRel, DCRel),
  downcase_atom(ExtMPointAttr, DCMPointAttr),
  downcase_atom(ExtIDAttr, DCIDAttr),
  downcase_atom(ExtUnitRel, DCUnitRel),
  downcase_atom(ExtUnitAttr, DCUnitAttr),
  downcase_atom(ExtResultRel, DCResultRel),
  downcase_atom(ExtMBoolAttr, DCMBoolAttr),
  downcase_atom(ExtBtree, DCBtree).
