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

2 Part one - Determine relations and sql predicates using by the sql query

2.1 Goal of part one

  Goal of part one is to determine any relation and any sql predicate
used by sql query to be optimized. This information will be used by
Part two to construct execution plans for secondo database to
estimate the selectivity of joined predicates.

2.2 The algorithm in few words

  1 Find any used sql predicate by using predicate pattern edge(0,A,B,\_,\_,\_).

    The zero as argument 1 means only edges should be used
    starting at start node. ???For the result B is an identifier
    of a marginal predicate ever and all edges using only one predicate
    match the pattern.??? A unifies with POG identifier
    of sql predicate unified with B.

  2 Based on unifiers of??? B the relations of sql query can be identified.
    The directly computed??? list could contain a relation twice:

    1 two instances of one relation (different aliases)

    2 one instance of one relation twice (used by different sql predicates)

    In the first case nothing is to be done. In the second case the
    second occurence of the relation shouldn't be added to the list of
    relations.

  3 A list is built containing the relations found. This list is stored as
    parameter of dynamic fact listOfRelations/1.

  4 Also based on unifiers of B the sql predicates of sql query are searched.

  5 The list of sql predicates generated before is stored as parameter of
    the dynamic fact listOfPredicates/1.

2.3 The defined dynamic facts

2.3.1 listOfPredicates/1

  The dynamic fact listOfPredicates/1 exists only once. The
argument is
a list containing the sql predicates of the sql query. The predicates
are represented as a list of tuples. These tuples consist three elements:

  * the POG identifier of the sql predicate,

  * the relation of the sql predicate as predicate rel/3 and

  * the sql predicate itself.

2.3.2 listOfRelations/1

  The dynamic fact listOfRelations/1 exists only once, too. The
argument
is also a list, but it contains the instances of the relations used
by the sql predicates of the sql query. Each relation instance is
represented by
the predicate rel/3.The list is distinctive. Nevertheless it could contain
one relation twice, if the relation is used with different aliases.

2.3.2.1 instances of relations

  One relation can be used in a sql query more than one time
by defining distinctive aliases.
An instance of a relation is identified uniquely??? by an alias or just by
the name of the relation if no alias was defined. Each sql predicate
is exactly bundled with one relation instance.

2.4 The implementation of the predicates

2.4.1 findRelation/2

  The predicate is used to find a relation (arg 1) in an given list (arg 2)
of relations. If arg 1 could be found in arg 2, so the predicate finishs
successfully otherwise it fails.

*/

% just for debugging issues
findRelation(Relation, [] ):- % Relation not found in list
  debug_writeln(findRelation/2, 70, [ Relation, 
   ' could not be found in list' ]),
  !, % do not call unexpected call
  fail.

% the element heading the list is be able to unify with argument one
% (it means arg1 could be found in arg2) - predicate successful
findRelation(SearchFor, [ Relation | _ ]):- % Relation found
  set_equal(SearchFor, Relation),
  debug_writeln(findRelation/2, 71, [ 'found/equals ',
      SearchFor, ' in/and ', Relation ]).

% the element heading the list couldn't be unified with argument 1,
% but the list is not empty
% try again with the next top element of list
findRelation(SearchFor, [ _ | Rest ]):-
  debug_writeln(findRelation/2, 75, [ ' look for ', 
     SearchFor, ' in the rest ', Rest ]),
  !, % do not call unexpected call
  findRelation(SearchFor, Rest).

% unexpected call
findRelation(A, B):-
  not(B=[]),
  error_exit(findRelation/2, [ 'unexpected call with arguments: ', [A, B] ]).


%
set_equal(Set, Set):-
  debug_writeln(set_equal/2, 81, [ 'lists ', Set, ' and ', Set, ' are equal']).
% this definition is just for performance issues
set_equal([Elem1,Elem2], [Elem2,Elem1]):-
  debug_writeln(set_equal/2, 81, [ 'sets ', [Elem1,Elem2],
      ' and ', [Elem2,Elem1], ' are equal']).
set_equal(Set1, Set2):-
  subset(Set1,Set2),
  subset(Set2,Set1),
  debug_writeln(set_equal/2, 81, [ 'sets ', Set1, ' and ', Set2, ' are equal']).
% just for debugging issues
set_equal(Set1, Set2):-
  debug_writeln(set_equal/2, 80, [ 'sets ', Set1, ' and ',
    Set2, ' are not equal']),
  fail.


/*
2.4.2 add\_relation/1

  The predicate appends the list of relations by the given
relation (arg 1),
if arg 1 is not still a part of the list. The list of relations is stored
in fact listOfRelations/1.

*/


% the given relation is still a part of the list of relations -
% do not add the relation
add_relation(Relation):-
  listOfRelations(ListRelations),
  %!, % if relation not in list - add_relation 1 have to fail
  findRelation(Relation , ListRelations),
  debug_writeln(add_relation/1, 62, [ Relation,
      ' is still a part of listOfRelations ', ListRelations, 
     ' (do not apped)' ]),
  !. % if Relation could be found - do not search again

% append list of relations by the given relation
add_relation(Relation):-
%Q%  listOfRelations(OldList),
  retract(listOfRelations(OldList)),
  NewList = [ Relation | OldList ],
  asserta(listOfRelations(NewList)),
  debug_writeln(add_relation/1, 61, [ ' append listOfRelations ', 
      OldList, ' by ', Relation ]),
  !.
% unexpected call
add_relation(A):-
  error_exit(add_relation/1, [ 'unexpected call with argument: ', A ]).

/*
2.4.3 add\_predicate/1

  The predicate appends the list of sql predicates always found
by sql predicate (arg 1). Finally the list of sql predicates
is stored as argument of fact listOfPredicates/1.

*/

add_predicate(Predicate):-
%Q%  listOfPredicates(OldList),
  retract(listOfPredicates(OldList)),
  NewList = [ Predicate | OldList ],
  asserta(listOfPredicates(NewList)),
  debug_writeln(add_predicate/1, 61, [ ' append listOfPredicates ', 
       OldList, ' by ', Predicate ]),
  !.
% unexpected call
add_predicate(A):-
  error_exit(add_predicate/1, [ 'unexpected call with argument: ', A ]).


/*
2.4.4 build\_list\_of\_predicates/1

  The predicate is the entry point of part one. It initiates
the construction of the list of relations and the list of predicates
finally stored as argument of facts listOfRelations/1 and
listOfPredicates/1.

**something to do ?**

*/
build_list_of_predicates:-
  % clean up environment
  retractall(listOfRelations(_)),
  retractall(listOfPredicates(_)),
  asserta(listOfPredicates([])),
  asserta(listOfRelations([])),
%?%  !, kann dann "unexpected call" noch erreicht werden?
  % get all Predicates
  findall([PredIdent,Pr], edge(0,PredIdent,Pr,_,_,_), PredList),
  debug_writeln(build_list_of_predicates/0, 31, [ 'predicates found ',
      PredList]),
  % build list of relations and list of predicates
  build_list_of_predicates2(PredList).

% unexpected call
build_list_of_predicates:-
  error_exit(build_list_of_predicates/0, [ 'unexpected call' ]).

build_list_of_predicates2([]):-
  debug_writeln(build_list_of_predicates2/1, 42, [ 'all predicates evaluated']).

build_list_of_predicates2([ [ PredIdent,Pr ] | Rest ]):-
  get_predicate_contents(Pr, PredExpr, Relation),
  build_list_of_predicates3(PredIdent, PredExpr, Relation),
  debug_writeln(build_list_of_predicates2/1, 41, [ 'predicate tuple ',
      [PredIdent,Pr], ' evaluated' ]),
  build_list_of_predicates2(Rest).

build_list_of_predicates2([ [ PredIdent,Pr ] | Rest ]):-
  debug_writeln(build_list_of_predicates2/1, 40, 
     [ 'predicate tuple ', [PredIdent,Pr],
     ' not supported - ignore it']),
  build_list_of_predicates2(Rest).

% unexpected call
build_list_of_predicates2(A):-
  error_exit(build_list_of_predicates2/1, [ 'unexpected call with argument ',
     A ]).

build_list_of_predicates3(PredIdent, PredExpr, Relation):-
  % memory structure to store a found predicate
  PredIdent2 is round(PredIdent),
  Predicate = [ PredIdent2 , Relation , PredExpr ],
  % PredIdent is used to ident the predicate in pog
  % Relation is used to group predicates of a relation
  % PredExpr is the predicate itself used for predcounts query
  debug_writeln(build_list_of_predicates3/3, 59, [ 'predicate id ', PredIdent, 
    ' is transformed to ', PredIdent2]),
  (
    not(optimizerOption(joinCorrelations2)),
    not(optimizerOption(joinCorrelations)),
    is_list(Relation)
    ->
    info_continue(
      build_list_of_predicates3/2,
      [ 'join predicates not activated - ignore predicate ',
      Predicate ])
    ;
    % add Relation to the listOfRelations
    add_relation(Relation),
    % add Predicate to the listOfPredicates
    add_predicate(Predicate),
    debug_writeln(build_list_of_predicates3/2, 51, [ 'relation ', Relation, 
      ' and predicate ', Predicate, ' was added'])
  ),
  !.

% unexpected call
build_list_of_predicates3(A, B):-
  error_exit(build_list_of_predicates3/2, [ 'unexpected call with arguments ',
     [ A, B ] ]).



/*
2.4.5 get\_predicate\_contents/3

  The predicate get\_predicate\_contents/3 returns the content
describing the predicate.

*/

% predicates using just one relation
get_predicate_contents(Predicate, PredExpr, Relation):-
  % only select supports selects only currently
  Predicate = select( _ , PredExpr ),
  PredExpr = pr( _ , Relation ),
  debug_writeln(get_predicate_contents/3, 61, [ 'select predicate found ', 
      Predicate, ' => ', [PredExpr, Relation]]).

% join predicates
get_predicate_contents(Predicate, PredExpr, Relation):-
  Predicate = join( _, _, PredExpr ),
  PredExpr = pr( _ , Relation1, Relation2 ),
  Relation = [ Relation1, Relation2 ],
  debug_writeln(get_predicate_contents/3, 62, [ 'join predicate found ', 
        Predicate, ' => ', [PredExpr, Relation]]).

% sortedjoin predicates
get_predicate_contents(Predicate, PredExpr, Relation):-
  Predicate = sortedjoin( _, _, PredExpr, _, _),
  PredExpr = pr( _ , Relation1, Relation2 ),
  Relation = [ Relation1, Relation2 ],
  debug_writeln(get_predicate_contents/3, 62, [ 'join predicate evaluated ',
             Predicate, ' => ', [PredExpr, Relation]]).

% sortedjoin predicates
get_predicate_contents(Predicate, PredExpr, Relation):-
  Predicate = sortedjoin( _, _, PredExpr, _, _),
  PredExpr = pr( _ , Relation1, Relation2 ),
  Relation = [ Relation1, Relation2 ],
  debug_writeln(get_predicate_contents/3, 62, [ 'join predicate evaluated ',
      Predicate, ' => ', [PredExpr, Relation]]).

% other unsupported type of predicate
get_predicate_contents(Predicate, _, _):-
  info_continue(get_predicate_contents/3, [ 'unsupported type of predicate ',
      Predicate, ' - ignore it']),
  fail.



/*

  //characters [1] Type: [\underline{\it ]  [}]

*/


/*
3 Part two - construct all predcount execution plans

3.1 The goal of Part two

  The goal of part two is the construction of execution plans of
predcounts queries, ???which are able to run against
secondo database. They are constructed using
predicates "listOfPredicates/1"[1] and "listOfRelations/1" created by part one.
Finally the part three will using the execution plans to collect statistics
to bring??? the POG more detailed.

3.2 The algorithm in few words

  1 *???Punkt sinnlos???*
    The used sql predicates and relations are stored as argument 1 of
    predicates listOfPredicates/1 and listOfRelations/1. (see part one)

  2 for each relation instance: creation of predcount execution plan

    1 collect all sql predicates using one relation instance

    2 construct predcount execution plan (one per relation instance)

    3 generate the list of POG identifiers of sql predicates used
      by the constructed predcount execution plan

3.2.1 predcount execution plans

  The stream consuming sql operator predcount called with a list of
sql predicates determine for each element of the stream which
combination of given sql predicates are solved.
Each sql predicate i given as parameter is identified by an identifier
2\^i. This way of identifing allows to identify each combination of sql
predicates just by sum the identifiers of each solved sql predicate.
The result of the predcount operator is a stream of tuples. The count
of tuples is 2\^(count of sql predicates). Each tuple consists of an
identifier identifing a unique comination of solved sql predicates and
number of tuples of the original stream solving this combination of
sql predicates.

  The identifier does not correspond with the POG identifier
of the sql predicate. (see section below)

3.2.2 POG identifiers

  Each sql predicate has gotten an identifier in POG during parsing
the sql query.
To assign the collected statistics to the right edge of the POG,
it's necessary to translate each identifier of the results of predcount
to the corresponding identifier of the POG. This is done by predicate
translate\_ident\_bits/3. (see part three for details)
Part two supplies the translation by bundling the list of original POG
identifiers to each constructed predcount execution plan.

3.3 The defined dynamic facts

none

3.4 The implementation of the predicates

3.4.1 build\_queries/1

  The predicate is the entry point of part two. The result is returned
as the argument. The result is a list of tuples. One tupel per
relation instance is created. Each tuple contains a query (second
element) which represents the predcount execution plan for that
relation instance. The first element of the tupel is the list of
predicate identifiers called translation list. (see section 3.2.2
above) The elements of the translation list are used in step 3 to
translate the tupels responsed by predcount for the POG.

*/

build_queries(Queries4):-
  % get list of relations created in step 1
  listOfRelations(RelationList),
  % build queries
  build_queries(RelationList, Queries),
  debug_writeln(build_queries/1, 39,
     [ 'first list of queries built (contain redundant evaluations)', 
     Queries ]),
  % remove obsolete queries
  % joining relations by product is default and can also happen if 
  %  Option joinCorrelation2 is activated
  % (if no set of seful join predicates could be found)
  remove_obsolete_queries(Queries, [], [], QueriesRev),
  remove_obsolete_queries(QueriesRev, [], [], Queries4),
  debug_writeln(build_queries/1, 31,
    [ 'final list of queries built (without redundant evaluations)',
    Queries4 ]).

% unexpected call
build_queries(A):-
  error_exit(build_queries/1, [ 'unexpected call with argument ', A ]).

/*
3.4.2 remove\_obsolete\_queries/4

  % VJO doku

  While using Option correlationsProduct the optimizer calculates
the predcount results for joined relations. In contrast to the
Option correlationsJoin there is no need to explicitly calculate
the predcount results for singleton relations. That's why the
queries for singleton relations can be removed if the singleton
relation is a member of any set of joined relations of another
predcount query.

  Which relation will be removed depends on the order of relations
stored as fact listOfRelations/1.

*/

% end of recursion
remove_obsolete_queries([], _, QueryList, QueryList):-
  debug_writeln(remove_obsolete_queries/4, 41, [ 'end of recursion ', [[],
      'unknown', QueryList, QueryList] ]).

% obsolete query -> remove query
remove_obsolete_queries([ QueryDesc | RQuery ], Keep, RQueryRev, QueryList):-
  QueryDesc = [ QueryPreds, _, _],
  subset(QueryPreds, Keep),
  debug_writeln(remove_obsolete_queries/4, 42, [ 'query ',
          QueryDesc, ' removed' ]),
  remove_obsolete_queries(RQuery, Keep, RQueryRev, QueryList).

% query still neseccary -> keep query
remove_obsolete_queries([ QueryDesc | RQuery ], Keep, RQueryRev, QueryList):-
  QueryDesc = [ QueryPreds, _, _],
  (
    not(is_join_list(QueryPreds))
    ->
    merge_set(QueryPreds, Keep, KeepNew) ;
    KeepNew = Keep
  ),
  debug_writeln(remove_obsolete_queries/4, 43, [ 'query ', QueryDesc,
      ' kept' ]),
  remove_obsolete_queries(RQuery, KeepNew, [ QueryDesc | RQueryRev ], 
      QueryList).

% unexpected call
remove_obsolete_queries(A, B, C, D):-
  error_exit(remove_obsolete_queries/4, ['unexpected call with arguments ',
    [A, B, C, D]]).

/*
3.4.3 is\_join\_list/1

  The identifier of join predicates are enclosed by []. The
predicate looks for such elements within a list. If such one
is detected the list is identified as a lists containing
identifiers of join predicates.

*/

% join predicate is found -> end of recursion
is_join_list([ [ J ] | _ ]):-
  debug_writeln(is_join_list/1, 51, [ 'join predicate id ', [J], 'detected' ]).
% current element isn't a identifier of a join predicate -> continue with 
% next one
is_join_list([ NJ | IdentList]):-
  debug_writeln(is_join_list/1, 59, [ 'predicate id ', NJ, ' is no join']),
  is_join_list(IdentList).


/*
3.4.2 build\_queries/2

  The predicate implements a loop over all relation instances given
as first argument.

*/

% end of recursion
build_queries([], []):-
  debug_writeln(build_queries/2, 42, [ 'all queries built' ]).

build_queries([ Relation | RRest], [ Query | QRest]):-
  debug_outln('build_queries 2: ', Relation),
  % store all Predicates as facts corrPredicate/3
  deleteCorrPredicates,
  listOfPredicates(Predicates),
  storeCorrPredicates(Predicates),
  % build tupel for relation Relation
  build_query(Relation, Query),
  debug_writeln(build_queries/2, 41, [ 'query', Query, 
     ' built for relation ', Relation ]),
  % recursive call for other relations in list
  build_queries(RRest, QRest),
  % remove temporary facts corrPredicate/3
  deleteCorrPredicates.

% unexpected call
build_queries(A, B):-
  error_exit(build_queries/2, ['unexpected call with arguments ', [A, B]]).

% delete temporary facts
deleteCorrPredicates:-
  retractall((corrPredicates(_, _, _))),
  debug_writeln(deleteCorrPredicates/0, 51, ['facts corrPredicate/3 removed']).

% unexpected call
deleteCorrPredicates:-
  error_exit(deleteCorrPredicates/0, ['unexpected call']).

% creates temporary facts
storeCorrPredicates([]):-
  debug_writeln(storeCorrPredicates/1, 52,
       [ 'all facts corrPredicates/3 stored' ]).

storeCorrPredicates([ [ PredID, PredRel, PredExpr ] | ListOfPredicates ]):-
  assert((corrPredicates(PredID, PredRel, PredExpr))),
  debug_writeln(storeCorrPredicates/1, 51, [ 'fact ', 
        corrPredicates(PredID, PredRel, PredExpr), 'stored']),
  storeCorrPredicates(ListOfPredicates).

% unexpected call
storeCorrPredicates(A):-
  error_exit(storeCorrPredicates/1, ['unexpected call with argument ', A]).



/*
3.4.3 build\_query/2


  The predicate constructs the tuple [ PredIdentList Query ]
for the given relation (argument 1).

*/

build_query(Relation, [ PredIdentList , Query , Relation ]):-
  % build sel and join lists
  get_join_predicates(Relation, JoinPredicateIDs, JoinPredicates),
  debug_writeln(build_query/2, 59, [ 'got join predicates ', 
       JoinPredicateIDs, ' for relation ', Relation]),
  get_sel_predicates(Relation, SelPredicateIDs, _),
  debug_writeln(build_query/2, 59, [ 'got select predicates ', 
       SelPredicateIDs, ' for relation ', Relation]),
  length(SelPredicateIDs, CountSelPreds),
  !,
  % set samples to be used instead of relations
  removeSamplesToBeUsed,
  setSamplesToBeUsed(Relation),
  !,
  % build relation expression
  build_relation_expr(Relation, JoinPredicates, RelationExpr, 
        UsedJoinPredicateIDs, CountSelPreds),
  debug_writeln(build_query/2, 59, [ 'relation expression ', RelationExpr, 
      ' built for relation ', Relation, ' using join predicates ',
       UsedJoinPredicateIDs]),
  !,
  % build list of predicate identifiers identifying the predicates
  % evaluated by predcount operator
  flatten(UsedJoinPredicateIDs, UsedJoinPredicateIDsTmp),
  flatten(JoinPredicateIDs, JoinPredicateIDsTmp),
  subtract(JoinPredicateIDsTmp, UsedJoinPredicateIDsTmp, 
      UnUsedJoinPredicateIDs),
  append(SelPredicateIDs, UnUsedJoinPredicateIDs, PCPredicates),
  debug_writeln(build_query/2, 59,
      ['predcount expression to be built for predicates ', PCPredicates]),
  !,
  % build predcount expression
  build_predcount_exprs(PCPredicates, PredcountExpr, PredIdentListTmp),
  debug_writeln(build_query/2, 59, ['predcount expression built ', 
       PredcountExpr, ' with translation list ', PredIdentListTmp]),
  % the identifiers of used join predicates are added to
  % the list of predicate identifiers to support calculate_pog_ident/3
  % (the idents of used join preds are enclosed by [])
  append(PredIdentListTmp, UsedJoinPredicateIDs, PredIdentList),
  debug_writeln(build_query/2, 59, ['final tanslation list is ', 
      PredIdentList]),
  !,
  % concatinate string
  atom_concat('query ', RelationExpr, PredcountExpr,
    ' consume;', '', Query),
%?%  debug_writeln(build_query/2, 51, [ 'query tuple ', [ PredIdentList ,
%  Query , RelPred ], ' built for relation ', Relation]).
  debug_writeln(build_query/2, 51, [ 'query tuple ', 
      [ PredIdentList , Query , Relation ], ' built for relation ', Relation]),
  % remove samples to be used instead of relations
  removeSamplesToBeUsed.

% unexpected call
build_query(A, B):-
  error_exit(build_query/2, ['unexpected call with arguments ', [A,B] ]).



/*
3.4.4 build\_relation\_expr/2

  The predicate builds relation expression " query relation feed \{alias\} " .
Setting the alias is necessary to support direct using of sql predicates
of the original query.

  If Option correlationsProduct is activated the string " relation feed \{
alias \} " is generated for each relation of a set of relations. The
generated strings are concatinated and joined with the key words query and
product.

  If Option correlationsJoin is activated a more complicated
algorithm is used to produce a derivated relation used to
calculate the resulats of operator predcount. The algorithm
is described in capture.

*/

% if Option correlationsJoin is set - applicate operator 
% predcount on a derivated relation
build_relation_expr(Relation, JoinPredicates, RelationExpr, 
     SmallestJoinSet, CountSelPreds):-
        optimizerOption(joinCorrelations), % nicht Option Kreuzprodukt
  % if no join predicate exists use product by default
  not(JoinPredicates=[]),
  % at least one selection predicate
  CountSelPreds > 0,
%?%  is_list(Relation), wie sollte ein join pred definiert werden 
% ueber eine relationsinstanz
  %!, no cut here - it is possible, that no set of join predicates
% joins all relations
  % in that case get_all_join_sets/3 fails (that's correct) and a producti
%  of relations should be used
  (
    get_all_join_sets(Relation, JoinPredicates, JoinSets),
    not(JoinSets=[])
    ->
    debug_writeln(build_relation_expr/4, 69, 
     [ 'set of possible sets of join predicates for relation ', Relation, 
    ' is ', JoinSets]);
    warn_continue(build_relation_expr/4, 
        [ 'no set sufficient join predicates for relation ', Relation, 
        ' found - use product']),
    fail
  ),
  %!,  if its not possible to construct a sample query
  % repeat until a joinset and path is found which is translatable to a
  %  sample query
  get_smallest_join(JoinSets, SmallestJoinSet, _),
  debug_writeln(build_relation_expr/4, 69, 
     [ 'smallest set of join predicates for relation ', Relation, 
     ' is ', SmallestJoinSet]),
  %!, if its not possible to construct a sample query
  get_cheapest_path(SmallestJoinSet, CheapestJoinPath),
  debug_writeln(build_relation_expr/4, 69, [ 
        'cheapest path of smallest join ', CheapestJoinPath]),
  %!, if its not possible to construct a sample query
  construct_sample_plan(CheapestJoinPath, SamplePlan),
  !,
  plan_to_atom(SamplePlan, RelationExpr),
  %plan_to_atom(CheapestJoinPlan, RelationExpr),
  debug_writeln(build_relation_expr/4, 61, 
     [ '(Option joinCorrelations set) relation expression ',
      RelationExpr, ' built for relation ', Relation, 
      ' using set of join predicates ', SmallestJoinSet]),
  !.


build_relation_expr(Relation, JoinPredicates, 
                    RelationExpr, SmallestJoinSet, _):-
  build_relation_expr2(Relation, JoinPredicates, RelationExpr, SmallestJoinSet).

% unexpected call
build_relation_expr(A, B, C, D, E):-
  error_exit(build_relation_expr/4, 
      ['unexpected call with arguments ',[A, B, C, D, E]]).

% if Option correlationsProduct is set or
% if Option correlationsJoin is set and no sufficient 
% set of join predicates were found to build the derivated relation
%             - applicate operator predcount on a product of relations
% end of recursion
build_relation_expr2([ LastRelation | [] ], _, RelationExpr, []):-
  (
    optimizerOption(joinCorrelations);
    optimizerOption(joinCorrelations2)
  ),
  LastRelation = rel(_, RelAlias),
  % use Sample instead of relation
  sampleToBeUsed(LastRelation, NameSample),
  debug_writeln(build_relation_expr2/4, 69, 
      ['use sample ',NameSample,' instead of relation ',LastRelation]),
  Sample = rel(NameSample, RelAlias),
%?%  !, % keine Wirkung von unexpected call
  plan_to_atom(Sample, RelName), % determine relation
  % build alias if necessary
  build_alias_string(Sample, RelAliasString),
  atom_concat(' ', RelName, ' feed ', RelAliasString,
    ' ', RelationExpr), % concatinate string
  debug_writeln(build_relation_expr2/4, 62, 
     [ '(Option joinCorrelations* set) last relation ', LastRelation,
       ' added as ', RelName, ' with alias ', RelAliasString,
       ' to product']).

% recursion
build_relation_expr2([ Relation | OtherRelations ], JoinPredicates, 
                       RelationExpr, JoinPredicates2):-
  (
    optimizerOption(joinCorrelations);
    optimizerOption(joinCorrelations2)
  ),
  Relation = rel(_, RelAlias),
  % use Sample instead of relation
  sampleToBeUsed(Relation, NameSample),
  debug_writeln(build_relation_expr2/4, 69, ['use sample ',
                NameSample,' instead of relation ',Relation]),
  Sample = rel(NameSample, RelAlias),
%?%  !, % keine Wirkung von unexpected call
  plan_to_atom(Sample, RelName), % determine relation
  % build alias if necessary
  build_alias_string(Sample, RelAliasString),
  build_relation_expr2(OtherRelations, JoinPredicates, RelationExprTmp,
                       JoinPredicates2),
  atom_concat(RelationExprTmp, RelName, ' feed ', RelAliasString,
    ' product ', RelationExpr), % concatinate string
  debug_writeln(build_relation_expr2/4, 62, [ 
          '(Option joinCorrelations* set) relation ', Relation, 
          ' added as ', RelName, ' with alias ', RelAliasString, 
          ' to product']).

% for singleton relations
build_relation_expr2(Relation, [], RelationExpr, []):-
  Relation = rel(_, RelAlias),
  % use Sample instead of relation
  sampleToBeUsed(Relation, NameSample),
  debug_writeln(build_relation_expr/4, 69, ['use sample ',
             NameSample,' instead of relation ',Relation]),
  Sample = rel(NameSample, RelAlias),
%?%  !, % keine Wirkung von unexpected call
  plan_to_atom(Sample, RelName), % determine relation
  % build alias if necessary
  build_alias_string(Sample, RelAliasString),
  atom_concat(' ', RelName, ' feed ', RelAliasString,
    '', RelationExpr), % concatinate string
  debug_writeln(build_relation_expr2/4, 63, [ 'singleton relation ', 
               Relation, ' used as ', RelName, ' with alias ', 
               RelAliasString]).

% unexpected call
build_relation_expr2(A, B, C, D):-
  error_exit(build_relation_expr2/4, ['unexpected call with arguments ',
                [A, B, C, D]]).




/*
3.4.5 build\_alias\_string/2

  The predicate builds the relation alias expression " \{alias\} " .

*/

build_alias_string(rel(_,*), ''):-
  Relation = rel(_,*),
  debug_writeln(build_alias_string/2, 71, [ 'no alias given for relation ',
     Relation]). % no alias necessary

build_alias_string(rel(_,RelAlias), AliasString):-
  Relation = rel(_,RelAlias),
        % create alias expression
  atom_concat(' {', RelAlias, '}', '', '', AliasString),
  debug_writeln(build_alias_string/2, 72, [ ' alias given for relation ', 
                Relation, ' is ', RelAlias]).

% unexpected call
build_alias_string(A, B):-
  error_exit(build_alias_string/2, ['unexpected call with arguments ',[A, B]]).



/*
3.4.6 build\_predcount\_exprs/3

  The predicate builds the predcounts expression string
 " predcounts [ ... ] " .

*/

build_predcount_exprs(Predicates, PredcountString, PredIdentList):-
  % get list of predicates created by part one
  findall(
    [PredID, Predicate],
    (
      corrPredicates(PredID, _, Predicate),
      member(PredID, Predicates)
    ),
    ListOfPredicates
  ),
  debug_writeln(build_predcount_exprs/3, 69, [ 'list of predicates ', 
               ListOfPredicates, ' for predcount expression ']),
%?%  !, wegen unexpected call raus
  % build list of predcounts expression elements and
  % predicate translation list
  build_predcount_expr(ListOfPredicates, PredcountExpr, PredIdentList),
  atom_concat(' predcounts [ ', PredcountExpr, ' ] ', '', '',
    PredcountString),
  debug_writeln(build_predcount_exprs/3, 61, [ 'predcount expression ', 
             PredcountString, ' and translation list ', PredIdentList, 
             ' for list of predicates ',ListOfPredicates]).

% unexpected call
build_predcount_exprs(A, B, C):-
  error_exit(build_predcount_exprs/3, [ 'unexpected call with arguments ',
            [A, B, C]]).


/*
3.4.7 build\_predcount\_expr/4

  The predicate builds the expression for each sql predicate of
predcounts expression string and concates the expression to the list
of predcount expressions (argument 3). It adds the POG identifier
of the sql predicate to the translation list (argument 4), too.


*/

build_predcount_expr([], '', []):- % end of recursion
  debug_writeln(build_predcount_expr/3, 72, ['end of recursion']).
build_predcount_expr([ Predicate | Rest ], PredcountExpr,
  PredIdentList):- % used if Predicate use Relation
  Predicate = [ PredIdentNew, PredExprNew ],
  get_predicate_expression(PredIdentNew, PredExprNew, PredExpr, PredIdent),
%nur chaos  debug_writeln(build_predcount_expr/3, 79, [ 'using ',
%  [PredIdent,PredExpr], ' for ', [PredIdentNew, PredExprNew]]),
%?%  !, no longer necessary, because of using of get_predicate_expression/4
  % recursive call for other predicates in list
  build_predcount_expr(Rest, PERest, PIRest),
  %!, % if Predicate does not use Relation add_predcount fails
  build_predicate_expression(PredIdent, PredExpr,
    PredPredcountExpr),
  debug_writeln(build_predcount_expr/3, 79, [ 'predcount expression ', 
                PredPredcountExpr, ' for predicate ', Predicate]),
  build_comma_separated_string(PredPredcountExpr, PERest,
    PredcountExpr),
  debug_writeln(build_predcount_expr/3, 71, [
        'new predcount list expression ', PredcountExpr]),
  % add predicate identifier of POG to list
  PredIdentList = [ PredIdent | PIRest ],
  debug_writeln(build_predcount_expr/3, 71, [ 
          'new translation list ', PredIdentList]).

% unexpected call
build_predcount_expr(A, B, C):-
  error_exit(build_predcount_expr/3, ['unexpected call',[A, B, C]]).


/*
3.4.8 get\_predicate\_expression/2

  The predicate get\_predicate\_expression/2 returns the
predicates expression.

*/

% predicates using one relation
get_predicate_expression(Ident, Predicate, PredExpr, Ident):-
  Predicate = pr( PredExpr, _),
  debug_writeln(get_predicate_expression/4, 81, [ 
          'expression for selection predicate ', PredExpr]).
% predicates using two relations (join)
get_predicate_expression(Ident, Predicate, PredExpr, Ident):-
  Predicate = pr( PredExpr, _, _),
  debug_writeln(get_predicate_expression/4, 82, [ 
          'expression for join predicate ', PredExpr]).
%X01get_predicate_expression(Ident, Predicate, '', [Ident]):-
%X01  Predicate = pr( _, _, _).
% unexpected call
get_predicate_expression(A, B, C, D):-
  error_exit(get_predicate_expression/4,
    ['unexpected call (only predicates of form pr/2 and pr/3 supported)',
    [A, B, C, D]]).

/*
3.4.8 build\_predicate\_expression/3

  The predicate constructs one sql predicate expression for predcount
expression.

*/

% used for predicates which should not be evaluated by predcount operator
%X01build_predicate_expression(_, '', ''):-
%X01  debug_outln('build_predicate_expression (no expression)').

build_predicate_expression(PredIdent, Expression, PredcountExpr):-
  plan_to_atom(Expression, PredExpr),
  atom_concat('P', PredIdent, ': ', PredExpr,
    '', PredcountExpr),
  debug_writeln(build_predicate_expression/3, 81, [ 
          'built predcount expression ', PredcountExpr, 
          ' for predicate ', [PredIdent, Expression]]).

% unexpected call
build_predicate_expression(A, B, C):-
  error_exit(build_predicate_expression/3, [ 
            'unexpected call with arguments ',[A, B, C]]).



/*
3.4.9 build\_comma\_separated\_string/3

  Arg3 = Arg1 + ',' + Arg2

*/

build_comma_separated_string('', AtEnd, AtEnd):-
  debug_writeln(build_comma_separated_string/3, 81, [ 
         'arguments ', ['', AtEnd], ' result ', AtEnd]).
build_comma_separated_string(AtBegin, '', AtBegin):-
  debug_writeln(build_comma_separated_string/3, 81, [ 
           'arguments ', [AtBegin, ''], ' result ', AtBegin]).
build_comma_separated_string(AtBegin, AtEnd, NewString):-
  atom_concat(AtBegin, ',', AtEnd, '', '', NewString),
  debug_writeln(build_comma_separated_string/3, 81, [ 
           'arguments ', [AtBegin, AtEnd], ' result ', NewString]).
build_comma_separated_string(A, B, C):-
  error_exit(build_comma_separated_string/3, [ 
          'unexpected call with arguments ',[A, B, C]]).


/*
6.5.4.5 get\_join\_predicates/3

  returns all join predicates applicable to the relations

*/

% alle JoinPreds fuer Relation ermitteln
get_join_predicates(Relation, JoinPredicateIDs, JoinPredicates):-
  listOfPredicates(Predicates),
  get_join_predicates2(Relation, Predicates, JoinPredicateIDs, JoinPredicates).

get_join_predicates2(_, [], [], []).
get_join_predicates2(Relation, [ Predicate | Predicates ], 
             [ [PredID] | JoinPredicateIDs ], 
             [ Predicate | JoinPredicates ]):-
  Predicate = [ PredID, PredRelation, pr(_, _, _) ],
  subset(PredRelation, Relation),
  !,
  get_join_predicates2(Relation, Predicates, JoinPredicateIDs, JoinPredicates).
get_join_predicates2(Relation, [ _ | Predicates ], JoinPredicateIDs,
             JoinPredicates):-
  get_join_predicates2(Relation, Predicates, JoinPredicateIDs, JoinPredicates).

/*
6.5.4.5 get\_sel\_predicates/3

  returns all selection predicates applicable to the relations

*/

% alle SelPreds ermitteln
get_sel_predicates(Relation, SelPredicateIDs, SelPredicates):-
  listOfPredicates(Predicates),
  get_sel_predicates2(Relation, Predicates, SelPredicateIDs, SelPredicates).

get_sel_predicates2(_, [], [], []).
get_sel_predicates2(Relation, [ Predicate | Predicates ], 
        [ PredID | SelPredicateIDs ], [ Predicate | SelPredicates ]):-
  Predicate = [ PredID, PredRelation, pr(_, _) ],
  (
    is_list(Relation)
    ->
    member(PredRelation, Relation);
    Relation = PredRelation
  ),
  !,
  get_sel_predicates2(Relation, Predicates, SelPredicateIDs, SelPredicates).
get_sel_predicates2(Relation, [ _ | Predicates ], SelPredicateIDs, 
    SelPredicates):-
  get_sel_predicates2(Relation, Predicates, SelPredicateIDs, SelPredicates).

/*
6.5.4.5 get\_all\_join\_sets/3

  find all sets of join predicates to join the relations

*/

%alle Kombis von JoinPreds ermitteln, die reichen alle Relationen zu verbinden
get_all_join_sets(Relation, JoinPredicates, JoinSets):-
  debug_outln('get_all_join_sets: ',[Relation, JoinPredicates, JoinSets]),
  flatten([Relation], RelationList),
  findall(JoinSet, get_join_set0(RelationList, JoinPredicates, [], 
          JoinSet), JoinSets).

get_join_set0(Relation, JoinPredicates, [], JoinSet):-
  debug_outln('get_join_set0: ',[Relation, JoinPredicates, [], JoinSet]),
  get_join_set(Relation, JoinPredicates, [], JoinSet).

get_join_set(Relations, _, JointRelations, []):-
  set_equal(Relations, JointRelations),
  !.
get_join_set(Relations, [ JoinPredicate | JoinPredicates ], 
           JointRelations, [ JoinPredicateID | JoinSet ]):-
  JoinPredicate = [ JoinPredicateID, PredRelations, _ ],
  not(subset(PredRelations, JointRelations)),
  append(PredRelations, JointRelations, JointRelationsTmp),
  my_list_to_set(JointRelationsTmp, JointRelationsNew),
  get_join_set(Relations, JoinPredicates, JointRelationsNew, JoinSet).
get_join_set(Relations, [ JoinPredicate | JoinPredicates ], 
             JointRelations, JoinSet):-
  JoinPredicate = [ _, _, _ ],
  %ist Alternativpfad fuer findall, deshalb nicht: 
  % subset(PredRelations, JointRelations),
  get_join_set(Relations, JoinPredicates, JointRelations, JoinSet).

/*
6.5.4.5 get\_smallest\_join/3

  looks for the join set with the smallest result set

*/

% Kosten aller Kombis ermitteln
% geht von Unabhaengigkeit der Predikate aus
get_smallest_join([ JoinSet | [] ], [JoinSet], Selectivity):-
  get_join_selectivity(JoinSet, Selectivity).
get_smallest_join([ JoinSet | JoinSets ], JoinSetOld, SelectivityOld):-
  not(JoinSets=[]),
  get_smallest_join(JoinSets, JoinSetOld, SelectivityOld),
  get_join_selectivity(JoinSet, Selectivity),
  Selectivity >= SelectivityOld.
get_smallest_join([ JoinSet | JoinSets ], JoinSet, Selectivity):-
  not(JoinSets=[]),
  get_join_selectivity(JoinSet, Selectivity).


/*
6.5.4.5 get\_join\_selectivity/2

  calculate the selectivity of a join set

*/

get_join_selectivity([], 1):-
  !.
get_join_selectivity([ JoinPredID | JoinSet ], SelectivityNew):-
  get_join_selectivity(JoinSet, SelectivityOld),
  get_joinpred_selectivity(JoinPredID, Selectivity),
  SelectivityNew is SelectivityOld * Selectivity,
  !.
get_join_selectivity(Unknown, _):-
  error_exit(get_join_selectivity/2,'unexpected call',[Unknown]).

/*
6.5.4.5 get\_joinpred\_selectivity/2

  get the marginal selectivity of a join predicate

*/

get_joinpred_selectivity(JoinPredID, Selectivity):-
  get_real_nodeid(JoinPredID, JoinPredIDReal),
  edgeSelectivity(0, JoinPredIDReal, Selectivity),
  !.
get_joinpred_selectivity(JoinPredID, 1):-
  get_real_nodeid(JoinPredID, JoinPredIDReal),
  not(edgeSelectivity(0, JoinPredIDReal, _)),
  writeln(
     'WARN: get_joinpred_selectivity/2: no edgeSelectivity/3 found for node ',
     JoinPredID,' - use selectivity of 1').

/*
6.5.4.5 get\_cheapest\_path/2

 finds the cheapest path

 The implementation using get\_first\_path/2 supports searching 
 of the cheapest path which is translateable to a sample query.

*/

get_cheapest_path(JoinSet, CheapestJoinPath):-
  flatten(JoinSet, JoinSetTmp),
  sumlist(JoinSetTmp, TargetNodeID),
  findall([Cost, PredIDs, Path], get_path_cost(0, TargetNodeID, Path, 
          PredIDs, Cost), Paths),
  msort(Paths, OrderedPaths),
  !,
  % doesn't support: try different paths
  %OrderedPaths = [ [ _, _, CheapestJoinPath ] | _ ],
  % get first path
  get_first_path(OrderedPaths, CheapestJoinPath).

% use the first path
get_first_path([[ _, _, CheapestJoinPath] | _], CheapestJoinPath):-
  true.
% if the last returned path is not translatable try next one
get_first_path([ _ | Rest], CheapestJoinPath):-
  not(Rest=[]),
  get_first_path(Rest, CheapestJoinPath).




/*
6.5.4.5 get\_path\_cost/5

 calculate cost of a path

*/

get_path_cost(NodeID2, NodeID, [], [], 0):-
  NodeID2 =:= NodeID.
get_path_cost(LastNodeID, TargetNodeID, [ CostEdge | CostEdges ], 
             [ PredID | PredIDs ], CostNew):-
  planEdge(LastNodeID, NewNodeID, PredPath, _),
  PredID is NewNodeID - LastNodeID,
  equals_and(PredID,PredID,TargetNodeID),
  edgeSelectivity(LastNodeID, NewNodeID, Selectivity),
  cost(PredPath, Selectivity, ResultSize, Cost),
  CostEdge = costEdge(LastNodeID, NewNodeID, PredPath, NewNodeID,
                      ResultSize, Cost),
  get_path_cost(NewNodeID, TargetNodeID, CostEdges, PredIDs, CostOld),
  CostNew is CostOld + Cost.

/*
6.5.4.5 setSamplesToBased/2

 manages the samples used by predcounts approach

*/

% there are 2 named samples which could be used
setSamplesToBeUsed(Relations):-
  % if relation instances have to use the same sample for any usage
  % the facts corrUsageCounter/2 and sampleToBeUsed/2 have to be
  % global at level build_queries/0
  retractall(corrUsageCounter(_,_)),
  setSamplesToBeUsed2(Relations).

% predcounts query uses just one relation
setSamplesToBeUsed2(Relation):-
  Relation = rel(_,_),
  increaseRelationCounter(Relation, CurrentCounter),
  setSampleToBeUsed(Relation, CurrentCounter).
% predcounts query uses more than one relation
setSamplesToBeUsed2([]):-
  debug_writeln(storeSamplesToBeUsed/2, 9000, ['end of recusision']),
  !.
setSamplesToBeUsed2([ Relation | Rest ]):-
  not(Relation=[]),
  increaseRelationCounter(Relation, CurrentCounter),
  setSampleToBeUsed(Relation, CurrentCounter),
  !,
  setSamplesToBeUsed2(Rest).

increaseRelationCounter(Relation, CurrentCounter):-
  retract(corrUsageCounter(Relation, CurrentCounter)),
  NextCounter is CurrentCounter + 1,
  assert(corrUsageCounter(Relation, NextCounter)).
increaseRelationCounter(Relation, 1):-
  assert(corrUsageCounter(Relation, 1)).

setSampleToBeUsed(Relation, Counter):-
  debug_writeln(setSampleToBeUsed/2, 999, ['call: ', [Relation, Counter]]),
  fail.
% just for debugging issues - use a sample predefined by fact 
% correlationsPredefinedSample/2
setSampleToBeUsed(Relation, _):-
  retract(correlationsPredefinedSample(Relation, PredefinedSample)),
  assert(correlationsPredefinedSample(Relation, PredefinedSample)),
  correlationsPredefinedSample(Relation, PredefinedSample),
  assert(sampleToBeUsed(Relation, PredefinedSample)),
  warn_continue(setSampleToBeUsed/2, ['use predefined sample ',
                PredefinedSample,' for relation ',Relation]).
% second usage of relation - use sample S
setSampleToBeUsed(Relation, 2):-
  Relation = rel(Name,_),
  % costruct sample name
  SampleSuffix = '_sample_s',
  atom_concat(Name, SampleSuffix, NameSample),
  downcase_atom(NameSample,DCNameSample),
  % if neccesary create sample (the name of created sample have to be 
  % equal to NameSample !!!)
  ensureSampleSexists(Name),
  % register sample
  assert(sampleToBeUsed(Relation, DCNameSample)),
  debug_writeln(setSampleToBeUsed/2, 9000, ['register sample ',
        DCNameSample,' for relation ',Relation,
        ' (stored as sampleToBeUsed/2)']).
% first usage of relation - use sample J
setSampleToBeUsed(Relation, 1):-
  Relation = rel(Name,_),
  % costruct sample name
  SampleSuffix = '_sample_j',
  atom_concat(Name, SampleSuffix, NameSample),
  downcase_atom(NameSample,DCNameSample),
  % if neccesary create sample (the name of created sample have to 
  % be equal to NameSample !!!)
  ensureSampleJexists(Name),
  % register sample
  assert(sampleToBeUsed(Relation, DCNameSample)),
  debug_writeln(setSampleToBeUsed/2, 9000, ['register sample ',
        DCNameSample,' for relation ',Relation,
        ' (stored as sampleToBeUsed/2)']).
% other usages of relation - use relation itself (another 
% implementation: create new samples)
setSampleToBeUsed(Relation, Id):-
  Id >= 3,
  Relation = rel(_,_),
  assert(sampleToBeUsed(Relation, Relation)),
  warn_continue(setSampleToBeUsed/2, [
       'no more sample avalible for relation ',Relation]).
% unexpected call
setSampleToBeUsed(A,B):-
  error_exit(setSampleToBeUsed/2,'unexpected call',[A,B]).

removeSamplesToBeUsed:-
  retractall(sampleToBeUsed(_,_)),
  debug_writeln(removeSampleSuffixes/0, 9000,
       ['facts sampleToBeUsed/2 retracted']).

/*
6.5.4.5 construct\_sample\_plan/2

 prepares a plan based of a path and replaces all usages of relations
 by usages of their samples

*/

construct_sample_plan(Path, Plan):-
  plan(Path, TmpPlan),
  !, % if prepare_sample_query/2 fails because of untranslatable 
     % TmpPlan do not retry plan/2
  prepare_sample_query(TmpPlan, Plan).

prepare_sample_query(In, Out):-
  query_sample(In, Out),
  !.
prepare_sample_query(In, Out):-
  warn_continue(prepare_sample_query/2, ['unable to build sample query', 
     [In, Out]]),
  !,
  fail.

query_sample([],[]) :-
  !.

query_sample([First|Next], [FirstResult|NextResult]) :-
  query_sample(First, FirstResult),
  query_sample(Next, NextResult), !.

query_sample(IndexName, _) :-
  atomic(IndexName),
  dcName2externalName(DCindexName,IndexName),
  databaseName(DB),
  storedIndex(DB,_,_,_,DCindexName),
  % dann ist der Ausdruck fuer sample nicht anwendbar
  % Suche nach sample query ist fehlgeschlagen
  warn_continue(query_sample/2,[
      'using of indexes not supported for sample queries - try another path',
      IndexName]),
  !,
  fail.

query_sample(rel(Name, V), rel(NameSample, V)) :-
  sampleToBeUsed(rel(Name, V), NameSample),
  debug_writeln(query_sample/2, 9000, ['use sample ',NameSample,
     ' instead of relation ',rel(Name, V)]),
  !.

query_sample( rel(Name, V), rel(Name, V) ) :-
  atomic(rel(Name, V)),
  warn_continue(query_sample/2, ['no sample of relation ',rel(Name, V),
   ' usable - use relation itself']),
  !.

query_sample( Term, Term ) :-
  atomic(Term), !.

query_sample( Term, Result ) :-
  not(Term=rel(_, _)),
  compound(Term),
  not(is_list(Term)),
  Term =.. [Op|Args],
  query_sample( Args, ArgsResult ),
  Result =.. [Op|ArgsResult], !.

/*
4 Part three - execution of predcounts plans and
tranforming of the results for the POG

4.1 Goal of part three

  After the construction of predcounts execution plans
in part two the plans
have to be executed now. The predcounts results have to be transformed
because
the identifiers used by operator predcounts don't generally match with the
identifiers used by POG. Finally the results are added to the POG.

4.2 Algorithm in few words

  1 execute predcounts execution plan

  2 transform the predicate identifiers (atom) of the operator predcounts
    to the correspondig node identifiers used by the POG

4.3 Defined dynamic facts

  none

4.4 Implementation of the predicates

4.4.1 calculate\_predcounts/2

  The predicate is the entry point of part three. It uses the result of
part two as first argument and returns the result as second arguent.
It executes any given predcounts execution plan and transforms each predicate
identifiers of operator predcounts to POG's one using
translate\_ident\_bits.

*/

% end of recursion
calculate_predcounts([], []):-
  retractall(predcounts_result(_,_)),
  debug_writeln(calculate_predcounts/2, 32, [ 
      'all facts predcounts_result/2 removed' ]).
calculate_predcounts([[PredIDs , Query , Relation ] | QRest],
  [PredCount2 | PRest]):-
  % rekursiv alle predcounts-Plaene betrachten
  calculate_predcounts(QRest,PRest),
  !,
  % predcounts-Plan ausfuehren
  secondo(Query, [_|[PredTupels]]),
  debug_writeln(calculate_predcounts/2, 39, [ 
      'predcounts result for relations ', Relation, ' is ', PredTupels]),
  !,
  % Wert fuer atom in Ergebnistupel in den Wert
  % fuer POG uebersetzen
  translate_ident_bits(PredTupels, PredIDs, PredCount),
  debug_writeln(calculate_predcounts/2, 39, [ 
       'translated predcounts results ', PredCount ]),
  % bei Nutzung von nested-loop statt product ist noch ein Tupel aufzunehmen
  (
    optimizerOption(joinCorrelations), % nicht Kreuzprodukt
    %containsJoinPred(PredIDs)
    % Relation is a list of Relations
    is_list(Relation)
    ->
    debug_writeln(calculate_predcounts/2, 39, [ 
        'add number of tuples of product of ', Relation, 
        ', which do not meet the join predicate']),
    get_card_product(Relation, CardProduct),
    debug_writeln(calculate_predcounts/2, 39, [ 
        'cardinality of product of all relations ', Relation, 
        ' is ', CardProduct]),
    get_sum_tuples(PredCount, SumTuples),
    debug_writeln(calculate_predcounts/2, 39, [
        'sum of all predcount tupels ', PredCount, ' is ', SumTuples]),
    PCJoin is CardProduct - SumTuples,
    debug_writeln(calculate_predcounts/2, 39, [ 
          'number of tuples of product of ', Relation, 
          ', which do not meet the join predicate is ', PCJoin]),
    PredCount2 = [ [ 0 , PCJoin ] | PredCount ];
    PredCount2 = PredCount
  ),
  % speichere predcounts Ergebnisse als Fakt
  assert(predcounts_result(Relation, PredCount2)),
  debug_writeln(calculate_predcounts/2, 31, [ 'store relations ',
            Relation, ' predcounts result ', PredCount2, ' as fact']).
% unexpected call
calculate_predcounts(A, B):-
  error_exit(calculate_predcounts/2, ['unexpected call with arguments ',
           [A, B]]).

/*
4.4.2 Helper get\_sum\_tuples/2

*/

% end of recursion
get_sum_tuples([], 0):-
  debug_writeln(get_sum_tuples/2, 42, ['end of recursion']).
get_sum_tuples([ [ _ , Count] | Rest ], SumTuplesNew):-
  get_sum_tuples(Rest, SumTuplesOld),
  SumTuplesNew is SumTuplesOld + Count,
  debug_writeln(get_sum_tuples/2, 41, [Count, ' added to ', SumTuplesOld,
    ' is ', SumTuplesNew]).
% unexpected call
get_sum_tuples(A, B):-
  error_exit(get_sum_tuples/2, ['unexpected call',[A, B]]).

/*
4.4.2 Helper get\_card\_product/2

*/

get_card_product([], 0):-
  debug_writeln(get_card_product/2, 42, [ 'end of recursion']).
get_card_product(RelationList, CP):-
  get_card_product2(RelationList, CP),
  debug_writeln(get_card_product/2, 41, ['product of cardinalities of ',
            RelationList, ' is ', CP]).
get_card_product(Relation, Card):-
  get_card_relation(Relation, Card),
  debug_writeln(get_card_product/2, 43, ['cardinality of ', Relation, 
          ' is ', Card]).
% unexpected call
get_card_product(A, B):-
  error_exit(get_card_product/2, ['unexpected call',[A, B]]).

% end of recursion
get_card_product2([], 1):-
  debug_writeln(get_card_product2/2, 52, [ 'end of recursion']).
get_card_product2([ Relation | Rest ], CPNew):-
  get_card_relation(Relation, Card),
  get_card_product2(Rest, CPOld),
  CPNew is CPOld * Card,
  debug_writeln(get_card_product2/2, 51, [
            'multi old product of cardinalities ', CPOld, ' with ', 
            Card, ' to ', CPNew]).
% unexpected call
get_card_product2(A, B):-
  error_exit(get_card_product2/2, ['unexpected call',[A, B]]).
/*
4.4.2 Helper get\_card\_relation/2

*/

get_card_relation(Relation, Card):-
  get_relationid(Relation, RelationID),
  resSize(RelationID, Card),
  debug_writeln(get_card_relation/2, 61, ['cardinality of ', 
            Relation, ' with id ', RelationID, ' is ', Card]).

get_relationid(Relation, RelationID):-
  node(0, _, RelationList),
  get_relationid2(Relation, RelationList, RelationID),
  debug_writeln(get_relationid/2, 71, ['relation id of relation ', 
         Relation, ' is ', RelationID]).

% end of recursion
get_relationid2(Relation, [ RelDesc | _], RelationID):-
  RelDesc = arp( RelationID, [ Relation ], _),
  debug_writeln(get_relationid2/3, 82, ['relation id of relation ', 
        Relation, ' is ', RelationID]).
get_relationid2(Relation, [ _ | Rest], RelationID):-
  debug_writeln(get_relationid2/3, 81, ['search again in list ', Rest]),
  get_relationid2(Relation, Rest, RelationID).
% unexpected call
get_relationid2(A, B, C):-
  error_exit(get_relationid2/2, ['unexpected call',[A, B, C]]).

/*
4.4.2 translate\_ident\_bits/3

  The predicate transforms the predicate identifiers of the
operator predcounts
(argument one) to the node identifiers of POG (argument three) using
the translation tables given as a list (argument two).
Each atom of argument one is finally translated by using
calculate\_pog\_ident/3.

*/

% end of recursion
translate_ident_bits([], _, []). % Rekursionsabschluss
translate_ident_bits([ InTupel | IRest], TranTab,
  [ OutTupel | ORest]):-
  % rekursiv alle alle Tupel durchgehen
  translate_ident_bits(IRest, TranTab, ORest),
  % Tupel zerlegen
  InTupel = [ InAtom | Count ],
  % Wert fuer Atom in korrespondierenden POG-Wert uebersetzen
  calculate_pog_ident(InAtom, TranTab, OutAtom),
  % Zieltupel zusammensetzen
  OutTupel = [ OutAtom | Count],
  debug_writeln(translate_ident_bits/3, 41, [ 'real id of ', InAtom,
           ' is ', OutAtom, ' using translation list ', TranTab]).
% unexpected call
translate_ident_bits(A, B, C):-
  error_exit(translate_ident_bits/2, ['unexpected call',[A, B, C]]).


/*
4.4.3 calculate\_pog\_ident/3

  The predicate translate each atom identifier of the operator
predcounts to the corresponding node identifier of POG.

*/

% end of recursion
calculate_pog_ident(_, [], 0):- % Rekursionsabschluss
  debug_writeln(calculate_pog_ident/3, 52, ['end of recursion']).
calculate_pog_ident(InAtom, [ [ JoinId ] | TRest ], OutAtom):-
  debug_outln('calculate_pog_ident: add join ID ', JoinId),
  calculate_pog_ident(InAtom, TRest, OutAtom2),
  OutAtom is OutAtom2 + JoinId,
  debug_writeln(calculate_pog_ident/3, 53, ['add join id ', JoinId, 
            ' to id => new id ', OutAtom]).
calculate_pog_ident(InAtom, [ TranElem | TRest ], OutAtom):-
  % mod = Ganzzahl; restliche Bits
  IRest is InAtom // 2,
  IBit is InAtom - ( 2 * IRest),
  calculate_pog_ident(IRest, TRest, ORest),
  OBit is IBit * TranElem,
  OutAtom is ORest + OBit,
  debug_writeln(calculate_pog_ident/3, 51, ['']).
% unexpected call
calculate_pog_ident(A, B, C):-
  error_exit(calculate_pog_ident/2, ['unexpected call',[A, B, C]]).



/*
5 Helper predicates

  The following predicates are used for easier programming and
understanding of the main code.

5.1 debug\_write

----    debug_writeln(Message)
----

  The predicate debug\_write writes the given message to
the screen
without a trailing newline. The message is only written
if the fact debug\_level(X) with X greater then 0 is set.

*/

debug_out(_):- % if X=0 then no debugging messages
   % if fact debug_level is not set retract fails
   retract((debug_level(X))),
   % reassert the fact
   assert((debug_level(X))),
   % X=0 no debug output requested
   X=0,
   !.
% write debugging messages without concated newline
debug_out(Message):-
   % if fact debug_level is not set retract fails
   retract((debug_level(X))),
   % reassert the fact
   assert((debug_level(X))),
   % X>0 debug output requested
   X > 0,
   % write debug message
   write(Message),
   !.
debug_out(_).       % in any other case ignore call of debug_write

/*
5.2 debug\_outln

----    debug_outln(message)
        debug_outln(Msg1, Msg2)
        debug_outln(Msg1, Msg2, Msg3)
        debug_outln(Msg1, Msg2, Msg3, Msg4)
        debug_outln(Msg1, Msg2, Msg3, Msg4, Msg5)
        debug_outln(Msg1, Msg2, Msg3, Msg4, Msg5, Msg6)
----

  The predicates debug\_outln write the given message(s) to the
screen. The
given message(s) are concatinated without any delimiter but a newline
will be appended behind??? the last message. The message(s) are only
written if the fact debug\_level(X) with X greater then 0 is set.

*/

% if X=0 then no debugging messages
debug_outln(_):-
   % if fact debug_level is not set retract fails
   retract((debug_level(X))),
   % reassert the fact
   assert((debug_level(X))),
   % X=0 no debug output requested
   X=0,
   !.
% write debugging messages concated newline
debug_outln(Message):-
   retract((debug_level(X))),
   assert((debug_level(X))),
   X > 0,
   debug_out(Message),
   writeln(''),
   !.
debug_outln(_).

check_debug_level(LevelPred):-
   % if fact debug_level is not set retract fails
   retract((debug_level(LevelSet))),
   % reassert the fact
   assert((debug_level(LevelSet))),
   % check level
   LevelPred =< LevelSet,
   !.

check_debug_list(Pred):-
   % if fact debug_level is not set retract fails
   retract((debug_list(DebugPredList))),
   % reassert the fact
   assert((debug_list(DebugPredList))),
   % check set predicates
   (
     member(all, DebugPredList),
  not(member(-Pred, DebugPredList));
  member(+Pred, DebugPredList)
   ),
   !.

debug_writeln(Pred, Level, MessageList):-
  % check whether the message has to be written or not
  (
    check_debug_level(Level);
    check_debug_list(Pred)
  ),
  write_list(['DEBUG(', Level, '): ', Pred, ': ']),
  write_list(MessageList),
  nl,
  !.

debug_writeln(Pred, Level, _):-
  not(
    check_debug_level(Level);
    check_debug_list(Pred)
  ).

debug_writelnFacts(Pred, Level, MessageList, Arg1, Arg2):-
  % check whether the message has to be written or not
  (
    check_debug_level(Level);
    check_debug_list(Pred)
  ),
  findall(Arg1, Arg2, List),
  write_list(['DEBUG(', Level, '): ', Pred, ': ']),
  write_list(MessageList),
  write_list(List),
  nl,
  !.

debug_writelnFacts(Pred, Level, _, _, _):-
  not(
    check_debug_level(Level);
    check_debug_list(Pred)
  ).

%debug_writeln(_, _, _).

write_list([]).
write_list([ nl | Rest ]):-
  nl,
  write_list(Rest).
write_list([ Atom | Rest ]):-
  write(Atom),
  write_list(Rest).
write_list(Atom):-
  write(Atom).

write_sub_list(Head, [ Atom | [] ]):-
  writeln(Head, Atom).
write_sub_list(Head, [ Atom | Rest ]):-
  writeln(Head, Atom),
  write_sub_list(Head, Rest).


/*
  The following three predicates write two up to four messages
using the predicates debug\_write/1 and debug\_writeln/1

*/

% write two messages
debug_outln(Msg1,Msg2):-
   debug_out(Msg1),
   debug_outln(Msg2).

%write three messages
debug_outln(Msg1,Msg2,Msg3):-
   debug_out(Msg1),
   debug_out(Msg2),
   debug_outln(Msg3).

% write four messages
debug_outln(Msg1,Msg2,Msg3,Msg4):-
   debug_out(Msg1),
   debug_out(Msg2),
   debug_out(Msg3),
   debug_outln(Msg4).

% write five messages
debug_outln(Msg1,Msg2,Msg3,Msg4,Msg5):-
   debug_out(Msg1),
   debug_out(Msg2),
   debug_out(Msg3),
   debug_out(Msg4),
   debug_outln(Msg5).

% write six messages
debug_outln(Msg1,Msg2,Msg3,Msg4,Msg5,Msg6):-
   debug_out(Msg1),
   debug_out(Msg2),
   debug_out(Msg3),
   debug_out(Msg4),
   debug_out(Msg5),
   debug_outln(Msg6).

/*
5.3 atom\_concat/6

----    atom_concat(Txt1, Txt2, Txt3, Txt4, Txt5, Rslt)
----

  The predicate atom\_concat/6 concatinates five strings to one. It uses
the already defined predicate atom\_concat/3.

*/

% concat more than two strings
atom_concat(Txt1, Txt2, Txt3, Rslt):-
   atom_concat(Txt1, Txt2, TMP1),
   atom_concat(TMP1, Txt3, Rslt).

atom_concat(Txt1, Txt2, Txt3, Txt4, Rslt):-
   atom_concat(Txt1, Txt2, TMP1),
   atom_concat(TMP1, Txt3, TMP2),
   atom_concat(TMP2, Txt4, Rslt).

atom_concat(Txt1, Txt2, Txt3, Txt4, Txt5, Rslt):-
   atom_concat(Txt1, Txt2, TMP1),
   atom_concat(TMP1, Txt3, TMP2),
   atom_concat(TMP2, Txt4, TMP3),
   atom_concat(TMP3, Txt5, Rslt).

/*
5.4 writeln/2 - writeln/6

----    writeln(Txt1, Txt2, Txt3, Txt4, Txt5, Rslt)
----


*/

% concat more than two strings
writeln(Msg1, Msg2):-
   write(Msg1), writeln(Msg2).
writeln(Msg1, Msg2, Msg3):-
   write(Msg1), writeln(Msg2, Msg3).
writeln(Msg1, Msg2, Msg3, Msg4):-
   write(Msg1), writeln(Msg2, Msg3, Msg4).
writeln(Msg1, Msg2, Msg3, Msg4):-
   write(Msg1), writeln(Msg2, Msg3, Msg4).
writeln(Msg1, Msg2, Msg3, Msg4, Msg5):-
   write(Msg1), writeln(Msg2, Msg3, Msg4, Msg5).
writeln(Msg1, Msg2, Msg3, Msg4, Msg5, Msg6):-
   write(Msg1), writeln(Msg2, Msg3, Msg4, Msg5, Msg6).


/*
5.5 xxx


*/

info_continue(Pred, MessageList):-
  write_list([ 'INFO: ', Pred, ': ']),
  write_list(MessageList),
  nl.
info_continue(Pred, Message, List):-
  info_continue(Pred,[Message,' args:',List]).
warn_continue(Pred, MessageList):-
  write_list([ 'WARN: ', Pred, ': ']),
  write_list(MessageList),
  nl.
warn_continue(Pred, Message, List):-
  warn_continue(Pred,[Message,' args:',List]).
error_exit(Pred, MessageList):-
  write_list([ 'ERROR: ', Pred, ': ']),
  write_list(MessageList),
  nl,
  abort.
error_exit(Pred, Message, List):-
  error_exit(Pred,[Message,' args:',List]).


equals_or_contains(Relation, Relation2):-
  set_equal(Relation, Relation2),
  debug_outln(
       'equals_or_contains: Relation of Predicate equals Relation of \c
 query to build').

equals_or_contains([ Relation2 | _ ], Relation2):-
  debug_outln('equals_or_contains: Relation ',Relation2,
    ' of Predicate in Relations of query to build').

equals_or_contains([], Relation2):-
  debug_outln('equals_or_contains: Relation ',Relation2,
    ' of Predicate not in Relations of query to build'),
  fail.

equals_or_contains([ _ | Relations ], Relation2):-
  equals_or_contains(Relations,Relation2).



set_debug_list(List):-
  retractall((debug_list(_))),
  assert((debug_list(List))).

set_debug_level(Level):-
  retractall((debug_level(_))),
  assert((debug_level(Level))).

max_list([ E | [] ], E).
max_list([ E | R ], Max):-
  max_list(R, MaxOld),
  Max is max(E, MaxOld),
  !.
max_list(L,_):-
  error_exit(max_list/2, 'unexpected call', [L]).

min_list([ E | [] ], E).
min_list([ E | R ], Min):-
  min_list(R, MinOld),
  Min is min(E, MinOld),
  !.
min_list(L,_):-
  error_exit(min_list/2, 'unexpected call', [L]).


and(X,Y):-
  X,
  Y.

contains(E1, [ E2 ]):-
  E1 =:= E2.
contains(E1, [ E2 | _ ]):-
  E1 =:= E2.
contains(E,[ _ | S ]):-
  not(S=[]),
  contains(E,S).


equals_and(A, B, C):-
  A2 is round(A),
  B2 is round(B),
  C2 is round(C),
  A2 =:= B2 /\ C2.


  %
get_real_nodeid(NodeID, NodeIDReal):-
  node(NodeIDReal, _, _),
  NodeID =:= NodeIDReal,
  !.


/*
6 The interface for the optimizer

6.1 addCorrelationSizes/0

  The predicate calls all predicates to be used to interact with
the correlations approach.

*/

addCorrelationSizes:-
  info_continue(addCorrelationSizes,['start correlations code']),
  % remove temporary facts
  %retractall(corrCummSel(_,_)),
  %!,
  % get all predicates and all relations of the query
  build_list_of_predicates,
  listOfRelations(ListOfRelations),
        debug_writeln(addCorrelationSizes/0, 9, [
          'list of relations built ', ListOfRelations]),
  listOfPredicates(ListOfPredicates),
        debug_writeln(addCorrelationSizes/0, 9, [
            'list of predicates built ', ListOfPredicates]),
  !,
  % for testing purposes
  %writeln('add relation set of 3 relations'),
  %retract((listOfRelations(R))),
  %append(R,[[rel(ten, a), rel(ten, *), rel(ten, b)]],Rnew),
  %assert((listOfRelations(Rnew))),
  !,
  % construct the predcounts execution plans
  build_queries(Queries),
  debug_writeln(addCorrelationSizes/0, 9, ['predcount queries built ', 
            Queries]),
  !,
  % run the executions plans
  calculate_predcounts(Queries, PCResults),
  debug_writeln(addCorrelationSizes/0, 9, [
            'predcount results calculated ', PCResults]),
  !,
  % set float nodeids
  correct_nodeids(PCResults, PCResultsNew),
  debug_writeln(addCorrelationSizes/0, 9, ['node ids corrected ',
            PCResultsNew]),
  !,
  % store results of predcounts queries into facts corrCummSel/2
  retractall((corrCummSel(_, _))),
  storeCorrCummSel(PCResultsNew),
  findall([Node, RCard], corrCummSel(Node, RCard), DEBUG),
  debug_writeln(addCorrelationSizes/0, 9, ['facts corrCummSel/2 created: ', 
       DEBUG]),
  !,
  % use selectivities of marginal predicates estimated by default optimizer
  % the selectivities will be overwritten by values estimated by 
  % correlations optimizer within corrFeasible/4
  findall([MarginalPred,Sel],edgeSelectivity(0,MarginalPred,Sel),MP),
  !,
  % prepare a consistent set of relative node cardinalities (MP2 and JP2)
  % use no additional joint selectivities (2nd arg = []), use facts 
  % corrCummSel/2 only
  corrFeasible(MP, [], MP2, JP2),
  debug_writeln(addCorrelationSizes/0, 9, [
       'corrFeasible/4 finished with: ',[MP2,JP2]]),
  !,
  % the predicate maximize_entropy/3 seems to be 6 times faster if JP2 is sorted
  msort(JP2, SortedJP2),
  % estimate unknown cardinalities by maximum entropy approach
  getTime( maximize_entropy(MP2, SortedJP2, Result) , MaxEntropyTime),
  writeln('maximize_entropy needed: ',MaxEntropyTime,' ms'),
  !,
  % store relative node cardinalities of all nodes as facts corrRelNodeCard/2
  retractall((corrRelNodeCard(_,_))),
  storeRelativeNodeCardinalities(Result),
  debug_writeln(addCorrelationSizes/0, 9, ['corrRelNodeCard/2']),
  !,
  % reset facts resultSize/2 and edgeSelectivity/3 by using facts 
  % corrRelNodeCard/2
  resetPOGSizes,
  debug_writeln(addCorrelationSizes/0, 9, [
      'resultSize/2 and edgeSelectivity/3 of POG updated']),
  %!,
  % remove temporary facts
  %retractall(corrCummSel(_,_)),
  info_continue(addCorrelationSizes,['finish correlations code']),
  !.


/*
6.2 storeRelativeNodeCardinalities/1

  The predicate stores the result sets of all predcounts queries
as facts corrRelNodeCard/2.

*/

storeRelativeNodeCardinalities([]):-
  debug_writeln(storeRelativeNodeCardinalities/1, 32, ['end of recursion']).
storeRelativeNodeCardinalities([ [ NodeId, Sel ] | Rest ]):-
  node(Node,_,_),
  Node =:= NodeId,
  !,
  retractall((corrRelNodeCard(Node,_))),
  assert((corrRelNodeCard(Node, Sel))),
  storeRelativeNodeCardinalities(Rest).


storeCorrCummSel([]):-
  removePCResults,
  debug_writeln(storeCorrCummSel/1, 32, ['end of recursion']).
storeCorrCummSel([ TupleList | RList ]):-
  debug_writeln(storeCorrCummSel/1, 39, ['evaluate predcounts results ',
      TupleList]),
  % predcount Tupel in Fakten storedPCTuple/2 mit korrigierten 
  % Count-Werten umsetzen
  removePCResults,
  debug_writeln(storeCorrCummSel/1, 39, ['predcount results facts removed']),
  storePCResults(TupleList, TupleList),
  debug_writeln(storeCorrCummSel/1, 39, ['predcount results ', 
      TupleList, ' stored as facts']),
  !,
  % aus PCResults fuer Option entropy die kummulierten Sels berrechnen 
  % und als corrCummSel/2 speichern
  storeCummSel,
  debug_writeln(storeCorrCummSel/1, 39, [
      'edgeSelectivities based on predcounts results calculated']),
  !,
  %storeCorrCummSel(TupleList, TupleList), % resSize, edgeSel nur ohne entropy
  % Ergebnis der naechsten query verarbeiten
  storeCorrCummSel(RList).


storeCummSel:-
  storedPCTuple(0, BaseCard),
  % fuer alle Nodes die Kummulierte Sel speichern
  findall([NodeID, NodeCard], storedPCTuple(NodeID, NodeCard), NodeCards),
  storeCummSel2(BaseCard, NodeCards),
  debug_writeln(storeCummSel/0, 41, [
      'joint selectivity of all nodes tuples ', NodeCards, ' stored']).

storeCummSel2(_, []):-
  debug_writeln(storeCummSel2/2, 52, ['end of recursion']).
storeCummSel2(BaseCard, [ [ NodeID, NodeCard ] | NodeCards ]):-
  storeCummSel3(NodeID, NodeCard, BaseCard),
  debug_writeln(storeCummSel2/2, 51, ['joint selectivity of node ', 
      NodeID, ' calculated using cardinalities ', [NodeCard, BaseCard]]),
  storeCummSel2(BaseCard, NodeCards).

storeCummSel3(NodeID, _, 0.0):-
  storeCummSel3(NodeID, _, 0),
  debug_writeln(storeCummSel3/3, 52, ['use storeCummSel3(', 
      [NodeID, unset, 0], ')']).
storeCummSel3(NodeID, _, 0):-
  retractall((corrCummSel(NodeID, _))),
  assert((corrCummSel(NodeID, 1))),
  debug_writeln(storeCummSel3/3, 53, 
     ['cardinality of base node is 0 - corrCummSel(', [NodeID, 1],
      ') asserted']).
storeCummSel3(NodeID, NodeCard, BaseCard):-
  CummSel is NodeCard / BaseCard,
  storeCummSel(NodeID, CummSel),
  debug_writeln(storeCummSel3/3, 52, ['corrCummSel(', [NodeID, CummSel], 
        ') asserted']).
  %format(atom(DEBUG),'~20f',CummSel),
  %debug_writeln(storeCummSel3/3, 52, ['corrCummSel(', [NodeID, DEBUG], 
  %') asserted']).

storeCummSel(NodeID, CummSel):-
  debug_writeln(storeCummSel/2,9000,['reset corrCummSel: ',[NodeID, CummSel]]),
  retractall((corrCummSel(NodeID, _))),
  assert((corrCummSel(NodeID, CummSel))).

correctCorrCummSels([]).
correctCorrCummSels([ [Node,Sel] | Rest ]):-
  storeCummSel(Node, Sel),
  !,
  correctCorrCummSels(Rest).

appendCummSel(NodeID, _):-
  corrCummSel(NodeID, _).
appendCummSel(NodeID, CummSel):-
  assert((corrCummSel(NodeID, CummSel))).

removePCResults:-
  retractall((storedPCTuple(_, _))),
  debug_writeln(removePCResults/0, 41, ['facts storedPCTuple/2 removed']).

storePCResults([], _):-
  debug_writeln(storePCResults/2, 42, ['end of recursion']).
storePCResults([ [ NodeID, _ ] | RList ], PCList):-
  correctNodeCard(NodeID, PCList, CorrectCard),
  !,
  assert((storedPCTuple(NodeID, CorrectCard))),
  debug_writeln(storePCResults/2, 41,['corrected node cardinality ', 
       CorrectCard, ' asserted for node ', NodeID, 
       ' as fact storedPCTuple/2']),
  !,
  storePCResults(RList, PCList).

correctNodeCard(_, [], 0):-
  debug_writeln(correctNodeCard/3, 52, ['end of recursion']).
correctNodeCard(NodeID, [ [ NID , NC ] | RList ], NewCard):-
  correctNodeCard(NodeID, RList, OldCard),
  getAdjustment(NodeID, NID, NC, Adjustment),
  NewCard is OldCard + Adjustment,
  debug_writeln(correctNodeCard/3, 51, ['cardinality of node ', 
      NodeID, ' adjusted by ', Adjustment]).

getAdjustment(NodeID, NID, NC, Adjustment):-
  equals_and(NodeID, NID, NodeID),
  Adjustment = NC,
  debug_writeln(getAdjustment/4, 61, ['adjustment ', Adjustment, 
      ' of node ', NID, ' for node ', NodeID]).
getAdjustment(NodeID, NID, _, 0):-
  debug_writeln(getAdjustment/4, 61, ['no adjustment of node ', 
      NID, ' for node ', NodeID]).



/*
6.3 resetPOGSizes/0

  The predicate uses the facts corrRelNodeCard/2 to reset the facts
resultSize/2 and edgeSelectivity/3 of POG. By doing so, the results
of correlations aproach are returned to the optimizer.

*/

resetPOGSizes :-
  findall([Source, Target, Term, Result], edge(Source, Target, 
      Term, Result, _, _), Edges),
  debug_writeln(resetPOGSizes1/0, 49, ['edges to be refreshed: ', Edges]),
  resetPOGSizes1(Edges),
  debug_writeln(resetPOGSizes1/0, 49, ['all edges refreshed', Edges]).

resetPOGSizes1([]):-
  debug_writeln(resetPOGSizes1/1, 52, ['end of recursion']).
resetPOGSizes1([ [Source, Target, Term, Result] | Rest ]):-
  resetPOGSize(Source, Target, Term, Result),
  resetPOGSizes1(Rest).
resetPOGSizes1(A):-
  error_exit(resetPOGSizes1/1,'unexpected call',[A]).

% derivated from resetNodeSize of entropy_opt.pl
resetPOGSize(Source, Target, select(Arg, _), Result):-
  resSize(Arg, Card),
  correlationsSel(Source, Target, Sel),
  Size is Card * Sel,
  resetNodeSize(Result, Size),
  debug_writeln(resetPOGSize/4, 69, ['fact reseted to: resultSize(',
      [Result, Size],')']),
  retractall((edgeSelectivity(Source, Target, _))),
  assert(edgeSelectivity(Source, Target, Sel)),
  debug_writeln(resetPOGSize/4, 69, ['fact reseted to: edgeSelectivity(',
      [Source, Target, Sel],')']),
  !.

% derivated from resetNodeSize of entropy_opt.pl
resetPOGSize(Source, Target, join(Arg1, Arg2, _), Result) :-
  resSize(Arg1, Card1),
  resSize(Arg2, Card2),
  correlationsSel(Source, Target, Sel),
  Size is Card1 * Card2 * Sel,
  resetNodeSize(Result, Size),
  debug_writeln(resetPOGSize/4, 69, ['fact reseted to: resultSize(',
     [Result, Size],')']),
  retractall((edgeSelectivity(Source, Target, _))),
  assert(edgeSelectivity(Source, Target, Sel)),
  debug_writeln(resetPOGSize/4, 69, ['fact reseted to: edgeSelectivity(',
    [Source, Target, Sel],')']),
  !.

% derivated from resetNodeSize of entropy_opt.pl
resetPOGSize(Source, Target, sortedjoin(Arg1, Arg2, _, _, _), Result) :-
  resSize(Arg1, Card1),
  resSize(Arg2, Card2),
  correlationsSel(Source, Target, Sel),
  Size is Card1 * Card2 * Sel,
  resetNodeSize(Result, Size),
  debug_writeln(resetPOGSize/4, 69, ['fact reseted to: resultSize(',
     [Result, Size],')']),
  retractall((edgeSelectivity(Source, Target, _))),
  assert(edgeSelectivity(Source, Target, Sel)),
  debug_writeln(resetPOGSize/4, 69, ['fact reseted to: edgeSelectivity(',
     [Source, Target, Sel],')']),
  !.

resetPOGSize(A, B, C, D):-
  error_exit(resetNodeSize/4,'unexpected call',[A,B,C,D]).

correlationsSel(Source, _, 0):-
  corrRelNodeCard(Source, SSel),
  SSel =:= 0.

correlationsSel(Source, Target, Sel):-
  corrRelNodeCard(Source, SSel),
  corrRelNodeCard(Target, TSel),
  Sel is TSel / SSel.

correlationsSel(Source, _, _):-
  not(corrRelNodeCard(Source, _)),
  error_exit(correlationsSel/3, 'no corrRelNodeCard found for node',[Source]).

correlationsSel(_, Target, _):-
  not(corrRelNodeCard(Target, _)),
  error_exit(correlationsSel/3, 'no corrRelNodeCard found for node',[Target]).

correlationsSel(A, B, C):-
  error_exit(correlationsSel/3,'unexpected call',[A,B,C]).


resetNodeSize(0, _):- % is still the correct size
  debug_writeln(resetNodeSize/2, 52, ['node size of node 0 is still correct']).
resetNodeSize(0.0, _):- % is still the correct size
  debug_writeln(resetNodeSize/2, 52, 
     ['node size of node 0.0 is still correct']).
resetNodeSize(NodeID, Size) :-
  resultSize(Node,_),
  Node=:=NodeID, % wegen int und real-Mischung!
  retract((resultSize(Node,_))),
  setNodeSize(Node, Size),
  debug_writeln(resetNodeSize/2, 51, ['size of node ', Node, 
     ' updated to ', Size]).
resetNodeSize(NodeID, Size) :-
  Node is NodeID + 0.0, % NodeID has to be real
  setNodeSize(Node, Size),
  debug_writeln(resetNodeSize/2, 53, ['size ', Size, ' of node ', 
      Node, ' inserted']).
resetNodeSize(NodeID, Size) :-
  error_exit(resetNodeSize/2,'unexpected call',[NodeID, Size]).

/*
6.4 correct\_nodeids/2

  The predicates resets all node identifiers in the predcounts queries
result sets to the identifier of POG. (transfomrs int to real numbers)

*/

correct_nodeids([], []).
correct_nodeids([ PCResult | PCResults ], [ PCResultNew | PCResultsNew ]):-
  correct_nodeids2(PCResult, PCResultNew),
  correct_nodeids(PCResults, PCResultsNew).

correct_nodeids2([], []).
correct_nodeids2([ [ NodeID | Rest ] | PCTuples ],
       [ [ NodeIDNew | Rest ] | PCTuplesNew ]):-
  correct_nodeid(NodeID, NodeIDNew),
  !,
  correct_nodeids2(PCTuples, PCTuplesNew).

correct_nodeid(NodeID, NodeIDNew):-
  node(NodeIDNew, _, _),
  NodeIDNew =:= NodeID.
correct_nodeid(NodeID, _):-
  writeln('Fehler: kann Fakt node/3 zu Node ',NodeID,' nicht finden').




/*
6.5 corrFeasible/4

  The predicate is a wrapper for corrFeasible/0. It was created to provide
an interface equal to feasible/4 of entropy approach. The predicate has
to be used to join selectivities of different approaches for iterative scaling
algorithm.

*/

corrFeasible(MP, JP, MP2, JP2):-
  % facts corrCummSel/2 of correlations approach still exists
  optimizerOption(correlations),
  % store given selectivities as additional facts corrCummSel/2
  storeMarginalSels(MP),
  storeAdditionalJointSelectivities(JP),
  % now all known marginal and joint selectivities stored as fact corrCummSel/2
  debug_writeln(corrFeasible/4, 39, ['all known node selectivities stored']),
  debug_writelnFacts(corrFeasible/4, 39,
        ['known node selectivities [NodeId, Selectivity]: '], 
        [N,S], corrCummSel(N,S)),
  !,
  % just for debugging issues - set predefined relative node cardinalities
  setPredefinedNodeCards,
  debug_writeln(corrFeasible/4, 39, ['all predefined node selectivities set']),
  debug_writelnFacts(corrFeasible/4, 39, 
      ['known node selectivities [NodeId, Selectivity]: '],
      [N,S], corrCummSel(N,S)),
  % produce a consistent set of known selectivities
  corrFeasible,
  debug_writeln(corrFeasible/4, 39, 
       ['all inconsistent selectivities corrected']),
  !,
  debug_writelnFacts(corrFeasible/4, 39, 
        ['known node selectivities [NodeId, Selectivity]: '],
        [N,S], corrCummSel(N,S)),
  !,
  % while running corrFeasible/0 the node 0 could be a selectivity 
  % greater than 1
  % now correct that by dividing all selectivities by selectivity of node 0
  renormAllCummSels,
  !,
  % build list of selectivities of marginal and joint predicates
  loadMarginalSels(MP2),
  findall([NodeID, NodeSel], corrCummSel(NodeID, NodeSel), JP2),
  debug_writeln(corrFeasible/4, 39, ['finished']),
  !.

/*
6.5.1 renormAllCummSels/0

  While using corrFeasible/0 to produce a consistent set of cardinalities
the relative node cardinality of node 0 could become greater than 1.
If the cardinality greater than 1 so divide all relative node cardinalities
by the relative node cardinalities of node 0.

*/

renormAllCummSels:-
  corrCummSel(NodeID, BaseSel),
  NodeID =:= 0,
  BaseSel =:= 1, % nothing to be done
  debug_writeln(renormAllCummSels/0, 9000, 
      ['relative node cardinality of node ',NodeID,' is ',
      BaseSel,' - nothing to do']).
renormAllCummSels:-
  highNode(HN),
  %mit hoechster Genauigkeit, dann aber Rundungsfehler:
  corrCummSel(NodeID, BaseSel),
  %corrCummSel(NodeID, BaseSelTmp),
  %getEpsilon(Epsilon),
  %BaseSel is ceil(BaseSelTmp/Epsilon) * Epsilon,
  NodeID =:= 0,
  debug_writeln(renormAllCummSels/0, 9000, 
      ['relative node cardinality of node ',NodeID,' is ',
       BaseSel,' - correct all nodes']),
  renormAllCummSels2(BaseSel, HN).

renormAllCummSels2(1, _).
renormAllCummSels2(_, NodeID):-
  NodeID < 0,
  debug_writeln(renormAllCummSels2/2, 9000, ['end of recursision']).
renormAllCummSels2(BaseSel, NodeID):-
  renormAllCummSel(BaseSel, NodeID),
  debug_writeln(renormAllCummSels2/2, 9000,
      ['relative node cardinality of node ',NodeID,' corrected']),
  NodeIDNew is NodeID - 1,
  renormAllCummSels2(BaseSel, NodeIDNew).

renormAllCummSel(BaseSel, NodeID):-
  corrCummSel(NodeID2, CummSel),
  NodeID2 =:= NodeID,
  retract((corrCummSel(NodeID2, CummSel))),
  CummSelNew is CummSel / BaseSel,
  assert((corrCummSel(NodeID2, CummSelNew))),
  debug_writeln(renormAllCummSel/2, 9000, 
      ['relative node cardinality of node ',NodeID,
       ' corrected from ',CummSel,' to ',CummSelNew]).
renormAllCummSel(_, NodeID):-
  node(NodeID2,_,_),
  NodeID2 =:= NodeID,
  not(corrCummSel(NodeID2, _)),
  debug_writeln(renormAllCummSel/2, 9000, 
      ['no relative node cardinality of node ',NodeID,' known']).

/*
6.5.2 loadMarginalSels/1

  The predicates constructs the new list of selectivities of
marginal predicates. If once was corrected by corrFeasible/0
the corrected value is used.

*/

loadMarginalSels(CorrectedMarginalPreds):-
  findall(PredID, corrMarginalSel(PredID, _), MarginalPredIDs),
  loadMarginalSels2(MarginalPredIDs, CorrectedMarginalPreds).

loadMarginalSels2([], []).
% use fact corrCummSel/2 to get selectivity of marginal predicate
loadMarginalSels2([ PredID | PredIDs ], [ [ PredID, CummSel ] | MP ]):-
  corrCummSel(PredID, CummSel),
  %nodeCardRange(PredID, [CummSel|_]),
  debug_writeln(loadMarginalSels2/2,9000,
       ['got marginal Sel from corrCummSel: ',[PredID, CummSel]]),
  !,
  loadMarginalSels2(PredIDs, MP).
% if no fact corrCummSel/2 exists, use corrMarginalSel/2
loadMarginalSels2([ PredID | PredIDs ], [ [ PredID, MargSel ] | MP ]):-
  corrMarginalSel(PredID, MargSel),
  debug_writeln(loadMarginalSels2/2,9000,
     ['got marginal Sel from corrMarginalSel: ',[PredID, MargSel]]),
  !,
  loadMarginalSels2(PredIDs, MP).
% unexpected call
loadMarginalSels2(A, B):-
  error_exit(loadMarginalSels2/2,'unexpected call',[A,B]).

/*
6.5.3 storeMarginalSels/1

  stores known selectivities as facts corrMarginalSel/2 or corrCummSel/2

*/

storeMarginalSels(MP):-
  retractall((corrMarginalSel(_, _))),
  storeMarginalSels2(MP).

storeMarginalSels2([]).
storeMarginalSels2([ [ PredID, MargSel ] | MP ]):-
  resetMarginalSel(PredID, MargSel),
  % vjo hier nicht gut, besser weiter oben
  %set MargSel only, if no Sel set for PredID
  appendCummSel(PredID, MargSel),
  storeMarginalSels2(MP).

resetMarginalSel(PredID, MargSel):-
  retractall((corrMarginalSel(PredID, _))),
  assert((corrMarginalSel(PredID, MargSel))).

storeAdditionalJointSelectivities([]).
storeAdditionalJointSelectivities([ [ _, NodeID, CummEdgeSel ] | Rest]):-
  storeCummSel(NodeID, CummEdgeSel),
  storeAdditionalJointSelectivities(Rest).

/*
6.5.4

  The predicate checks the known relative node cardinalities stored 
  as facts corrCummSel/2.
  If the set of cardinalities not consistent the predicate generates 
  a consistent set.

*/

corrFeasible:-
  findall([A,B],corrCummSel(A,B),KnownSelectivities),
  debug_writeln(corrCummSel/2,9,['known selectivities: ',KnownSelectivities]),
  highNode(HN),
  retractall((nodeCardRange(_,_))),
  retractall((nodeCardList(_,_))),
  % block 1 (lines 003 - 015)
  initNodeCardLimits(0, HN),
    findall([N,L,U],nodeCardRange(N,[L,U]),X),
    debug_writeln(corrFeasible/0, 49, [
       'initial cardinality ranges of nodes [node, low, up]: ',X]),
  % other blocks (lines 020 - 106)
  correctNodeCards(HN),
    findall([N,L,U],nodeCardRange(N,[L,U]),X2),
  debug_writeln(corrFeasible/0, 49, [
      'final cardinality ranges of nodes [node, low, up]: ',X2]),
  % reset facts corrCummSel/2
    %alle corrCummSels anpassen:
  %findall([Node, Sel],nodeCardRange(Node,[Sel,Sel]),NodesToCorrect),
    findall([Node, Sel],
     (corrCummSel(Node,_), nodeCardRange(Node,[Sel,_])),
     NodesToCorrect),
  writeln(['to correct: ',NodesToCorrect]),
  correctCorrCummSels(NodesToCorrect).

/*
6.5.4.1 initNodeCardLimits/0

  block 1

*/

initNodeCardLimits(CurrentNode, HighestNode):-
  CurrentNode > HighestNode,
  debug_writeln(initNodeCardLimits/2, 52, ['end of recursion']).
initNodeCardLimits(CurrentNode, HighestNode):-
  corrCummSel(CurrentNodeId, RelCard),
  CurrentNode =:= CurrentNodeId, % wegen Durchmischung von int und real
  format(atom(DEBUG),'~20f',RelCard),
  debug_writeln(initNodeCardLimits/2, 59, [
      'set nodeCardRange of known node [node, low, up]: ',
       CurrentNodeId, [DEBUG, DEBUG]]),
  %debug_writeln(initNodeCardLimits/2, 59, ['set nodeCardRange of 
  % known node [node, low, up]: ',CurrentNodeId, [RelCard, RelCard]]),
  assert((nodeCardRange(CurrentNodeId, [RelCard, RelCard]))),
  !,
  NextNode is CurrentNode + 1.0,
  initNodeCardLimits(NextNode, HighestNode).
initNodeCardLimits(CurrentNode, HighestNode):-
  % fact corrCummSel/2 does not exist for current node
  % get real nodeID
  node(Curr,_,_),
  Curr=:=CurrentNode,
  !,
  findall(ULim, (edge(Prev, Curr, _,_,_,_), 
          nodeCardRange(Prev,[_,ULim])), ULimPrevs),
  % select the minimum of upper limit of all previous nodes
  min_list([1 | ULimPrevs], UpperLimit),
  format(atom(DEBUG),'~20f',UpperLimit),
  debug_writeln(initNodeCardLimits/2, 59, [
        'set nodeCardRange of UNKNOWN node [node, low, up]: ',
        Curr, [0, DEBUG]]),
  %debug_writeln(initNodeCardLimits/2, 59, ['set nodeCardRange of 
  % UNKNOWN node [node, low, up]: ',Curr, [0, UpperLimit]]),
  assert((nodeCardRange(Curr, [ 0, UpperLimit]))),
  !,
  NextNode is CurrentNode + 1.0,
  initNodeCardLimits(NextNode, HighestNode).

/*
6.5.4.2 correctNodeCards/1

  blocks 2 to last

*/

correctNodeCards(CurrentNode):-
  CurrentNode<0,
  debug_writeln(correctNodeCards/1, 52, ['end of recursion']).
correctNodeCards(CurrentNode):-
  % get real nodeID
  node(Curr,_,_),
  Curr=:=CurrentNode,
  !,
  % block 3 (lines 030 - 035)
  findall(SuccCardList, (edge(Curr,Succ,_,_,_,_), 
        nodeCardList(Succ, SuccCardList)), SuccCardLists),
  debug_writeln(correctNodeCards/1, 59, ['node ',Curr,
       ': card lists of successors: ',SuccCardLists]),
  !,
  % block 4 (lines 040 - 060)
  mergeCardLists(SuccCardLists, MergedList),
  debug_writeln(correctNodeCards/1, 59, ['node ',Curr,
      ': merged card lists: ',MergedList]),
  !,
  % block 5 (lines 070 - 079)
  calculateLowerLimit(MergedList, LowerLimitTmp),
  getEpsilon(Epsilon),
  %besser mal runden:
  LowerLimit is LowerLimitTmp + Epsilon,
  %LowerLimit is ceil(LowerLimitTmp/Epsilon)*Epsilon + Epsilon,
  %format(atom(DEBUG2),'~20f',LowerLimit),
  %writeln(['XXX: ',LowerLimitTmp,LowerLimit,DEBUG2]),
  format(atom(DEBUG),'~20f',LowerLimit),
  debug_writeln(correctNodeCards/1, 59, ['node ',Curr,
      ': calculated lower limit of card is: ',DEBUG]),
  !,
  % block 6 (lines 080 - 095)
  % resets nodeCardRange/2
  checkConsistencyAndCorrectNodeCard(Curr, LowerLimit),
  !,
  % block 7 (lines 100 - 106)
  nodeCardRange(Curr, Tuple),
  assert(nodeCardList( Curr, [ Tuple | MergedList] )),
  debug_writeln(correctNodeCards/1, 59, ['node ',Curr,
      ': set card list: ',[ Tuple | MergedList]]),
  !,
  NextNode is CurrentNode - 1.0,
  correctNodeCards(NextNode).
/*
6.5.4.3 mergeCardLists/2

 block 4

*/

mergeCardLists([], []):-
  debug_writeln(mergeCardLists/2, 62, ['end of recursion']).
mergeCardLists(SuccCardLists, MergedList):-
  nth1(1,SuccCardLists, FirstList),
  length(FirstList, CardListLen),
  debug_writeln(mergeCardLists/2, 69, ['merge list ', SuccCardLists,
       ' with ', CardListLen,' levels']),
  mergeCardLists(SuccCardLists, MergedList, 1, CardListLen).

mergeCardLists(_, [], Level, HighestLevel):-
  Level > HighestLevel,
  debug_writeln(mergeCardLists/4, 72, ['end of recursion']).
mergeCardLists(SuccCardLists, [ MergedTuples | MergedList ], Level, 
     HighestLevel):-
  NextLevel is Level + 1,
  mergeCardLists(SuccCardLists, MergedList, NextLevel, HighestLevel),
  !,
  sumLimits(SuccCardLists, Level, SumLowerLimit, SumUpperLimit),
  LowerLimit is SumLowerLimit / Level,
  UpperLimit is SumUpperLimit / Level,
  debug_writeln(mergeCardLists/4, 79, ['tuples at level ',Level,
      ' merged to ',[LowerLimit,UpperLimit]]),
  MergedTuples = [ LowerLimit, UpperLimit ].

sumLimits([], _, 0, 0):-
  debug_writeln(sumLimits/4, 82, ['end of recursion']).
sumLimits([ SuccCardList | SuccCardLists ], Level, SumLowerLimit, 
          SumUpperLimit):-
  sumLimits(SuccCardLists, Level, SumLowerLimitOld, SumUpperLimitOld),
  !,
  nth1(Level, SuccCardList, Tuple),
  Tuple = [ LowerLimit, UpperLimit ],
  debug_writeln(sumLimits/4, 89, ['tuple ',Tuple,
     ' to be merged at level ',Level]),
  SumLowerLimit is SumLowerLimitOld + LowerLimit,
  SumUpperLimit is SumUpperLimitOld + UpperLimit.
sumLimits(A,B,C,D):-
  error_exit(sumLimits/4,'unexpected call',[A,B,C,D]).

/*
6.5.4.4 calculateLowerLimit/2

 block 5

*/

calculateLowerLimit([], 0):-
  debug_writeln(calculateLowerLimit/2, 62, ['end of recursion']).
calculateLowerLimit([ [ LowLim, _ ] | MergedList ], LowerLimit):-
  calculateLowerLimit(MergedList, LowerLimitOld),
  !,
  % the expression -LowerLimitOld produces the alternating signs 
  % for the odd/even levels
  LowerLimit is LowLim - LowerLimitOld.


/*
6.5.4.5 checkConsistencyAndCorrectNodeCard/2

 block 6

*/

checkConsistencyAndCorrectNodeCard(Curr, LowerLimit):-
  nodeCardRange(Curr, [ StoredLowLim, StoredUpLim]),
  !,
  debug_writeln(checkConsistencyAndCorrectNodeCard/2, 69, ['node ',
       Curr,' check consistency of card against max of set ',
    [StoredUpLim, [StoredLowLim, LowerLimit]]]),
  max_list([StoredLowLim, LowerLimit], NewLowerLimit),
    %T is StoredUpLim - LowerLimit,
   %writeln(['ACard of ',Curr,' is ',T]),
  resetNodeCardRange(Curr, NewLowerLimit, StoredUpLim),
  true.
checkConsistencyAndCorrectNodeCard(Curr, _):-
  error_exit(checkConsistencyAndCorrectNodeCard/2, 
      'unexpected call (nodeCardRange/2 not set for node)',[Curr]).

resetNodeCardRange(Curr, NewLowerLimit, StoredUpLim):-
  NewLowerLimit =< StoredUpLim,
   debug_writeln(resetNodeCardRange/3, 79,['node ',Curr,
       ': cardinality range consistent: ', 
       [Curr, NewLowerLimit, StoredUpLim]]).
resetNodeCardRange(Curr, NewLowerLimit, StoredUpLim):-
  NewLowerLimit > StoredUpLim,
  %format(atom(DEBUG1),'~20f',NewLowerLimit),
  %format(atom(DEBUG2),'~20f',StoredUpLim),
  %writeln('###',DEBUG1,'###',DEBUG2),
  DEBUG_delta is NewLowerLimit - StoredUpLim,
  info_continue(resetNodeCardRange/3, ['cardinality range ', 
    [NewLowerLimit, StoredUpLim],' of node ',Curr,
    ' inconsistent reset upper limit (Delta: ',DEBUG_delta,')']),
  retract((nodeCardRange(Curr,_))),
  assert((nodeCardRange(Curr,[ NewLowerLimit, NewLowerLimit]))).

/*
6.6.6.6 getEpsilon/2

 returns the current Epsilon, which is used to avoid
 atom cardinalities of 0

*/

getEpsilon(Epsilon):-
  retract(correlationsEpsilon(Epsilon)),
  assert(correlationsEpsilon(Epsilon)),
  !.
getEpsilon(1e-05):-
  !.

/*
6.6.6.6 setPredefinedNodeCards/2

 support usage of predefined node cardinalities

*/

setPredefinedNodeCards:-
  retract(correlationsPredefinedNodeCard(A,B)),
  assert(correlationsPredefinedNodeCard(A,B)),
  findall([Node,Sel], correlationsPredefinedNodeCard(Node, Sel), PredefCards),
  setPredefinedNodeCards(PredefCards).
setPredefinedNodeCards.

setPredefinedNodeCards([]).
setPredefinedNodeCards([ [ Node, Card ] | Rest]):-
  not(Node=[]),
  retractall(corrCummSel(Node,_)),
  assert(corrCummSel(Node,Card)),
  warn_continue(setPredefinedNodeCards/1, [
      'predefined relative node cardinality ',Card,' set for node ',Node]),
  !,
  setPredefinedNodeCards(Rest).



