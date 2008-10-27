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

*/
/*
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

*/
/*
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

*/
/*
2.4 The implementation of the predicates

2.4.1 findRelation/2

  The predicate is used to find a relation (arg 1) in an given list (arg 2)
of relations. If arg 1 could be found in arg 2, so the predicate finishs 
successfully otherwise it fails.

*/

% the last element of list is reached and it unifies with arg1
% (it means arg1 could be found in arg2) - predicate successful
findRelation(Relation, Relation ):- % Relation found
	debug_writeln('findRelation 0').

% the list is empty (arg1 should not be found in list before) 
% (it means arg1 could NOT be found in arg2) - predicate fails
findRelation(Relation, [] ):- % Relation not found in list
	debug_writeln('findRelation 1 - ', Relation, '#'),
	fail. 

% the element heading the list is be able to unify with argument one
% (it means arg1 could be found in arg2) - predicate successful
findRelation(Relation, [ Relation | _ ]):- % Relation found
	debug_writeln('findRelation 3').

% the element heading the list couldn't be unified with argument 1, 
% but the list is not empty
% try again with the next top element of list
findRelation(SearchFor, [ _ | Rest ]):-
	debug_writeln('findRelation 2'),
	findRelation(SearchFor, Rest).

% just for debugging ???
%findRelation(X,Y):-
%	% write some details
%	writeln('findRelation ? - unexpected call (internal error)'),
%	writeln(X),
%	writeln(Y),
%	fail. % the call failed - of course


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
	debug_writeln('add_relation 1 - ', 
		Relation, ' // ', ListRelations),
	%!, % if relation not in list - add_relation 1 have to fail
	findRelation(Relation , ListRelations),
	!. % if Relation could be found - do not search again

% append list of relations by the given relation
add_relation(Relation):-
	debug_writeln('add_relation 2'),
	listOfRelations(OldList),
	retract(listOfRelations(OldList)),
	NewList = [ Relation | OldList ],
	asserta(listOfRelations(NewList)),
	debug_writeln('add_relation 2 - ', Relation, ' added'),
	!.

/*
2.4.3 add\_predicate/1

  The predicate appends the list of sql predicates always found
by sql predicate (arg 1). Finally the list of sql predicates
is stored as argument of fact listOfPredicates/1.

*/

add_predicate(Predicate):-
	listOfPredicates(OldList),
	retract(listOfPredicates(OldList)),
	NewList = [ Predicate | OldList ],
	asserta(listOfPredicates(NewList)),
	debug_writeln('add_predicate 1 - ', Predicate, ' added'),
	!.


/*
2.4.4 build\_list\_of\_predicates/1

  The predicate is the entry point of part one. It initiates 
the construction of the list of relations and the list of predicates
finally stored as argument of facts listOfRelations/1 and
listOfPredicates/1.

*/

build_list_of_predicates:-
	% clean up environment
	retractall(listOfRelations(_)),
	retractall(listOfPredicates(_)),
	asserta(listOfPredicates([])),
	asserta(listOfRelations([])),
	!,
	% build list of relations and list of predicates
	% all edges starting at source of pog
	edge(0,PredIdent,Pr,_,_,_),
	debug_writeln(
		'build_list_of_predicates 1 - analyse edge ', Pr),
	% only select supports selects only currently
	Pr = select( _ , pr( PredExpr , Relation ) ),
	% memory structure to store a found predicate
	Predicate = [ PredIdent , Relation , PredExpr ],
	% PredIdent is used to ident the predicate in pog
	% Relation is used to group predicates of a relation
	% PredExpr is the predicate itself used for predcounts query
	add_relation(Relation), % add Relation to the listOfRelations
	% add Predicate to the listOfPredicates
	add_predicate(Predicate),
	fail. % loop for all edges matching edge(0,_,_,_,_,_)


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

*/
/*
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

*/
/*
3.2.2 POG identifiers

  Each sql predicate has gotten an identifier in POG during parsing 
the sql query.
To assign the collected statistics to the right edge of the POG,
it's necessary to translate each identifier of the results of predcount
to the corresponding identifier of the POG. This is done by predicate
translate\_ident\_bits/3. (see part three for details)
Part two supplies the translation by bundling the list of original POG 
identifiers to each constructed predcount execution plan.

*/
/*
3.3 The defined dynamic facts

none

*/
/*
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

build_queries(Queries):-
	% get list of relations created in step 1
	listOfRelations(RelationList),
	% build queries
	build_queries(RelationList, Queries).

/*
3.4.2 build\_queries/2

  The predicate implements a loop over all relation instances given
as first argument.

*/
build_queries([], []). % end of recursion
build_queries([ Relation | RRest], [ Query | QRest]):-
	debug_writeln('build_queries 2: ', Relation),
	% build tupel for relation Relation
	build_query(Relation, Query),
	% recursive call for other relations in list
	build_queries(RRest, QRest).

/*
3.4.3 build\_query/2

  The predicate constructs the tuple [ PredIdentList Query ] 
for the given relation (argument 1).

*/

build_query(Relation, [ PredIdentList | Query]):-
	debug_writeln('build_query 1'),
	% build predcounts expression
	build_predcount_exprs(Relation, PredcountExpr, 
		PredIdentList), 
	% build relation expression
	build_relation_expr(Relation, RelationExpr), 
	% concatinate string
	atom_concat(RelationExpr, PredcountExpr, ' consume;', 
	   '', '', Query).


/*
3.4.4 build\_relation\_expr/2

  The predicate builds relation expression " query relation feed \{alias\} " .
Setting the alias is necessary to support direct using of sql predicates
of the original query. In addition it prepares the future 
support of joins.

*/

build_relation_expr(Relation, RelationExpr):-
	debug_writeln('build_relation_expr - ', Relation),
	plan_to_atom(Relation, RelName), % determine relation
	% ??? add using of small relation / samples here
	% build alias if necessary
	build_alias_string(Relation, RelAliasString),
	atom_concat('query ', RelName, ' feed ', RelAliasString, 
		'', RelationExpr). % concatinate string

/*
3.4.5 build\_alias\_string/2

  The predicate builds the relation alias expression " \{alias\} " .

*/

% vjo 2008-10-23 veraltet
% build_alias_string(rel(_,*,_), ''). % no alias necessary
% build_alias_string(rel(_,RelAlias,_), AliasString):-
%         % create alias expression
% 	atom_concat(' {', RelAlias, '}', '', '', AliasString).

build_alias_string(rel(_,*), ''). % no alias necessary
build_alias_string(rel(_,RelAlias), AliasString):-
        % create alias expression
	atom_concat(' {', RelAlias, '}', '', '', AliasString).


/*
3.4.6 build\_predcount\_exprs/3

  The predicate builds the predcounts expression string 
 " predcounts [ ... ] " .

*/

build_predcount_exprs(Relation, PredcountString, PredIdentList):-
	debug_writeln('build_predcount_exprs - ', Relation),
	% get list of predicates created by part one
	listOfPredicates(ListOfPredicates), 
	% build list of predcounts expression elements and 
	% predicate translation list
	build_predcount_expr(Relation, ListOfPredicates, 
		PredcountExpr, PredIdentList),
	atom_concat(' predcounts [ ', PredcountExpr, ' ] ', '', '', 
		PredcountString).


/*
3.4.7 build\_predcount\_expr/4

  The predicate builds the expression for each sql predicate of
predcounts expression string and concates the expression to the list
of predcount expressions (argument 3). It adds the POG identifier 
of the sql predicate to the translation list (argument 4), too.

*/

build_predcount_expr(_, [], '', []):- % end of recursion
	debug_writeln('build_predcount_expr 0').
build_predcount_expr(Relation,[ Predicate | Rest ], PredcountExpr, 
	PredIdentList):- % used if Predicate use Relation
	Predicate = [ PredIdent, Relation, PredExpr ],
	debug_writeln('build_predcount_expr 1: ', Relation, 
		' // ', Predicate),
	% recursive call for other predicates in list
	build_predcount_expr(Relation, Rest, PERest, PIRest),
	%!, % if Predicate does not use Relation add_predcount fails
	build_predicate_expression(PredIdent, PredExpr, 
		PredPredcountExpr), 
	build_comma_separated_string(PredPredcountExpr, PERest, 
		PredcountExpr),
	% add predicate identifier of POG to list 
	PredIdentList = [ PredIdent | PIRest ].
build_predcount_expr(Relation,[ Predicate | Rest ], PredcountExpr, 
	PredIdentList):- % used if Predicate do not use Relation
	debug_writeln('build_predcount_expr 2: ', Relation, 
		' // ', Predicate),
	% recursive call for other predicates in list
	build_predcount_expr(Relation, Rest,
		PredcountExpr, PredIdentList).


% alt:	add_predcount(Relation, Predicate, PERest, PIRest, 
% PredcountExpr, PredIdentList). % build predicate expression for 
% precounts expression

/*
3.4.8 build\_predicate\_expression/3

  The predicate constructs one sql predicate expression for predcount
expression.

*/

build_predicate_expression(PredIdent, Expression, PredcountExpr):-
	debug_writeln('build_predicate_expression'),
	plan_to_atom(Expression, PredExpr),
	atom_concat('P', PredIdent, ': ', PredExpr,
		'', PredcountExpr).


/*
3.4.9 build\_comma\_separated\_string/3

  Arg3 = Arg1 + ',' + Arg2 

*/

build_comma_separated_string('', AtEnd, AtEnd).
build_comma_separated_string(AtBegin, '', AtBegin).
build_comma_separated_string(AtBegin, AtEnd, NewString):-
	atom_concat(AtBegin, ',', AtEnd, '', '', NewString).


/*
3.4.10 add\_predcount/6

  deprecated

*/

add_predcount(Relation, [ PredIdent , Relation , Expr ], PERest, 
	PIRest, PredcountExpr, PredIdentList):-
	debug_writeln('add_predcount 1'),
  build_predicate_expression(PredIdent, Expr, PredString),
	build_comma_separated_string(PredString,
		PERest, PredcountExpr),
	PredIdentList = [ PredIdent | PIRest ].
add_predcount(Relation, Pred, PERest, PIRest, PERest, PIRest):- 
	% if Pred does not use Relation
	debug_writeln('add_predcount 0: ', Relation, ' # ', Pred).

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

calculate_predcounts([], []). % Rekursionsabschluss
calculate_predcounts([[PredIDs | Query] | QRest],
	[PredCount | PRest]):-
	% rekursiv alle predcounts-Plaene betrachten
	calculate_predcounts(QRest,PRest),
	% predcounts-Plan ausfuehren
	secondo(Query, [_|[PredTupels]]),
	% Wert fuer atom in Ergebnistupel in den Wert 
	% fuer POG uebersetzen
	translate_ident_bits(PredTupels, PredIDs, PredCount).

/*
4.4.2 translate\_ident\_bits/3

  The predicate transforms the predicate identifiers of the
operator predcounts 
(argument one) to the node identifiers of POG (argument three) using
the translation tables given as a list (argument two).
Each atom of argument one is finally translated by using 
calculate\_pog\_ident/3.

*/

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
	OutTupel = [ OutAtom | Count].


/*
4.4.3 calculate\_pog\_ident/3

  The predicate translate each atom identifier of the operator
predcounts to the corresponding node identifier of POG.

*/

calculate_pog_ident(0, [], 0). % Rekursionsabschluss
calculate_pog_ident(InAtom, [ TranElem | TRest ], OutAtom):-
	% mod = Ganzzahl; restliche Bits
	IRest is InAtom // 2,
	IBit is InAtom - ( 2 * IRest), 
	calculate_pog_ident(IRest, TRest, ORest), 
	OBit is IBit * TranElem,
	OutAtom is ORest + OBit.
calculate_pog_ident(_,_,_):-
	writeln('Fehler bei translate_ident_bits').

/*
5 A simple example

5.1 Overview

  The following simple example explains the using of

  * build\_list\_of\_predicates/0

  * build\_queries/1

  * calculate\_predcounts/2

5.2 Code example

  The output that shows the content of variables is restructured
for easier understanding. Normally the variable's content is shown
within one line per variable. For easier reading and referencing
such a line is devided into more than one. Additionally these lines
are numerated.

----
    F=[8,[2,3]]
----

  For example such a original prolog output line would
devided as follows:

----
    000 F=
    001 [
    002   8,
    003   [
    004     2,
    005     3
    006   ]
    007 ]
----

  Now it's possible to reference each element just by the
line's number.

  In Prolog all structured data is represented as a list.
For simplifing the explanations below we decide between a list and
a tuple. Both structures are represented as a list in Prolog, but
a tuple is a list with a fixed len. In no case a list called 
tuple will be composed of more or less elements as described in
this example.

  For example the list of predicates is a typical list
because their len is determined by the sql query to be optimized.
The predicate description which the list of predicates is composed of
is a typical tuple. It ever contains of three element.

5.2.1 Preparation

----
    59 ?- setOption(entropy).
    Total runtime ... Times (elapsed / cpu): 0.080001sec / 0.07sec = 1.14287
    Switched ON option: 'entropy' - Use entropy to estimate selectivities.

    Yes
----

  Setting the option entropy is nessecary first to let the
system generate the dynamic facts edge/6. These facts are used
by later prolog praedicates to analyse the sql query to be
optimized.

  After toggling on the Option entropy the sql query to be
optimized has to be entered.

----
    60 ?- sql select count(*) from [ plz as a, plz as b ]
     where [a:plz > 40000, b:plz < 50000, a:plz <50000].
    
    Computing best Plan ...
    Destination node 7 reached at iteration 6
    Height of search tree for boundary is 1
    
    Computing best Plan ...
    Destination node 7 reached at iteration 6
    Height of search tree for boundary is 1
    
    Computing best Plan ...
    Destination node 7 reached at iteration 6
    Height of search tree for boundary is 1
    
    Computing best Plan ...
    Destination node 7 reached at iteration 6
    Height of search tree for boundary is 1
    
    Computing best Plan ...
    Destination node 7 reached at iteration 6
    Height of search tree for boundary is 1
    
    Computing best Plan ...
    Destination node 7 reached at iteration 6
    Height of search tree for boundary is 1
    
    Computing best Plan ...
    Destination node 7 reached at iteration 6
    Height of search tree for boundary is 1
    
    Computing best Plan ...
    Destination node 7 reached at iteration 6
    Height of search tree for boundary is 1
    
    Computing best Plan ...
    Destination node 7 reached at iteration 6
    Height of search tree for boundary is 1
    
    Computing best Plan ...
    Destination node 7 reached at iteration 6
    Height of search tree for boundary is 1
    
    Computing best Plan ...
    Destination node 7 reached at iteration 6
    Height of search tree for boundary is 1
    
    Computing best Plan ...
    Destination node 7 reached at iteration 6
    Height of search tree for boundary is 1
    
    No
----

  After entering the sql query the option correlation
has to be toggled on. By doing this the option entropy
will be toggled off automatically.

----
    61 ?- setOption(correlation).
----

5.2.2 Analyze the query to be opimized

  This analysis is done by the prolog predicate 
build\_list\_of\_predicates/0.

----
    62 ?- build_list_of_predicates.

    No
----

  After calling the predicate build\_list\_of\_predicates/0
the dynamic facts listOfPredicates/1 and listOfRelations/1 were
created. The content of their arguments are shown below.

----
    63 ?- listOfPredicates(X).
    010 X = 
    020 [
    030   [
    031     4, 
    032     rel(plz, a), 
    033     attr(a:pLZ, 1, u)<50000
    034   ], 
    040   [
    041     2, 
    042     rel(plz, b), 
    043     attr(b:pLZ, 1, u)<50000
    044   ], 
    050   [
    051     1, 
    052     rel(plz, a), 
    053     attr(a:pLZ, 1, u)>40000
    054   ]
    060 ]
    
    Yes
----

  The content of X is the list of sql predicates found by
predicate build\_list\_of\_predicates/0 before. Each sql predicate found 
is represented by one tuple (lines 30-34, 40-44 and 50-54).
Each tuple is composed of three elements: (sql joins are disallowed yet!)

  1 POG identifier of the sql predicate (lines 31,41 and 51)

  2 the relation instance of the sql predicate represented by fact rel/2
(lines 32, 42 and 52)

  3 the sql predicate itself (lines 33, 43 and 53)

----

    64 ?- listOfRelations(Y).
    
    110 Y = 
    120 [
    130   rel(plz, b, l), 
    140   rel(plz, a, l)
    150 ]
    
    Yes
----

  The content of Y is the distinctive list of relation instances
found by build\_list\_of\_predicates/0. Although two sql predicates use
the same relation PLZ the relation was added twice to the list
because the relation PLZ was used as relation instance a and relation
instance b within the sql query. On the other hand the relation instance
b was added just once, although the relation instance is used by
two predicates. For a more detailed explanation of relation instances
see the section instances of relations above.

5.2.3 Build predcounts queries and get the statistics

----
    65 ?- build_queries(Queries), calculate_predcounts(Queries,Result).
    0.init finished
    Total runtime ... Times (elapsed / cpu): 1.91003sec / 1.91sec = 1.00002
    0.init finished
    Total runtime ... Times (elapsed / cpu): 0.530008sec / 0.54sec = 0.981496
    210 Queries = 
    220 [
    230   [
    231     [2] |
    232     query plz feed {b} predcounts [ P2: (.PLZ_b < 50000) ]
    232     consume;
    233   ], 
    240   [
    241     [4, 1] |
    242     query plz  feed  {a} predcounts [ P4: (.PLZ_a < 50000),
    242     P1: (.PLZ_a > 40000) ]  consume;
    243   ]
    250 ],
    310 Result = 
    320 [
    330   [
    331     [2, 21470], 
    332     [0, 19797]
    333   ], 
    340   [
    341     [5, 3127], 
    342     [1, 19797], 
    343     [4, 18343], 
    344     [0, 0]
    345   ]
    350 ]

    Yes
----

  The variable Queries contains a list of tuples (lines
230-233 and 240-243). One tuple
corresponds with exactly one relation instance (lines 130
and 140)
found by build\_list\_of\_predicates/0. Each tuple is composed of two 
elements:

  1 the list of POG identifiers of the sql predicates used by
the predcounts execution plan (see element two);  the order of the
POG identifiers corresponds with the order of using the sql predicates
within the predcounts execution plan

  2 predcounts execution plan of one relation instance with all
sql predicates using this relation instance

  The variable Result is unified with the list of results of
predcounts execution plans created by the predicate build\_queries/1.
(lines 232 and 242) Each result corresponds with one relation instance.
The order of predcounts results stored in Result matches the order of
predcounts execution plans in the variable Queries.

  The predcounts result is a list of tuples. Each tuple is
composed of a element atom (first one) and an element count (second
one). An explanation of the tuple's meaning could be found in the 
documentation of operator predcounts.

  The predcounts's results stored in Result are already
transformed. What means transformed? This issue is discussed later in
this document detailed. In a short manner one could say, the identifiers
of the tuples (column atom) are transformed for using it directly to
identify nodes of the POG.

5.2.4 How are the predcounts results to be interpreted ?

  Each predcounts result is exclusively calculated for
exact one relation instance. The tuples attribute atom identifies which
predicates a tuple has to solve and not to solve to increase the count value
of this tuple. 

  To understand the meaning of predcounts result the predcounts
execution plan of relation instance a (line 242) will be executed. 
  
----
    67 ?- secondo(query plz  feed  {a} predcounts [ P4: (.PLZ_a < 50000),
    P1: (.PLZ_a > 40000) ]  consume;', A).
    0.init finished
    Total runtime ...   Times (elapsed / cpu): 0.940014sec / 0.89sec = 1.0562
    
    410 A =
    420 [
    430   [rel, 
    431     [tuple, 
    432       [
    433         [Atom, int], 
    434         [Count, int]
    435       ]
    436     ]
    437   ],
    440   [
    441     [3, 3127],
    442     [2, 19797],
    443     [1, 18343],
    444     [0, 0]
    445   ]
    450 ]
    
    Yes
----

  While looking at the listing above you will see the predcounts
result is a list of tuples and each tuple is composed of a value Atom
and a value Count.

  To understand the value atom one has to imagine the value in
binary mode. So the value 1 is 01. The Count 18343 means that 18343 tuples
of the relation instance b solve the the predicate P4 but not the
predicate P1. (Please note the tuples must not solve the predicate P1.
This is in contradiction to the meaning of POG's nodes identifier.)

  To determine how many tuples of relation instance a solves
the predicate P4 in independence of predicate P1, the values Count of 
the result's tuples identified by 2 (binary 10) and 3 (binary 11) 
has to be added. In the example you get a count of 18343 + 3127 = 21470.

5.2.5 Some Theory

5.2.5.1 Definitions

	.

  Be $Pi = \{ p_1, p_2, p_4, ..., p_{2^m} \}$ the set of predicates used
by the sql query to be optimized and $P_{N_i}$ the set of predicates
solved at POG's node $N_i$.

  Be $R = \{R_0, R_1, \ldots, R_n \}$ the set of relation instances used by
the sql query to be optimized and $R_{N_i}$ the set of relation instances
used by the predicates of set $P_{N_i}$.

  Be $C_{N_0} = R_0 \times R_1 \times \ldots \times R_n$ the
initial set of tupels at the start node $N_0$.

  Be $r: P \rightarrow 2^R$ the function which determines the
relation instances $R_{p_i} \subseteq R$ used by the predicate
$p_i \in P$.

  Be the cardinality $C_{N_i}$ the count of tuples which solves all
predicates $p \in P_{N_i}$.

  Be the selectivity $S_{N_i} = \frac{C_{N_i}}{C_{N_0}}$.

  Be $PC_{R_i} \subset \aleph \times \aleph$ the predcounts result. Each 
element $t \in PC_{R_i}$ is composed of an attribute atom and an attribute
count. The elements of the tuple $t \in PC_{R_i}$ can be obtained
by the functions $a_{atom}: PC_{R_i} \rightarrow \aleph$ and
$a_{count}: PC_{R_i} \rightarrow \aleph$. In addition there is a function
$solved: PC_{R_i} \rightarrow P$ which determines the set of predicates
solved by the tuples $e \in R_i$ that are represented by tuple $t \in PC_{R_i}$
of predcounts result.

  Be $card: R_{N_i} \times \aleph \rightarrow \aleph$ a function which
determines $\forall t \in R_{N_i}, n \le 2^{|P|}: card(t,n) = ???$

  Be $CM_{R_i}$ the set of tuples $c_{R_i,\aleph} \in \aleph \times
\aleph$ of the the predcounts result. The elements of the tuple $c_{R_i,\aleph}$
can be obtained by the functions $a_{atom}(c_{R_i,\aleph}) \mapsto \aleph$ and
$a_{count}(c_{R_i,\aleph}) \mapsto \aleph$

  Be $s: PC_{R_i} \rightarrow P$ the function which determines
the set of predicates solved by tuples represented by a tuple
of predcounts result PC. For example the $a_{atom}(R_i,3)=5$ returns
the set $\{p_4, p_1\}$.

5.2.5.2 Types of POG's nodes

	.

  While evaluating the predcounts results it's possible to devide
the set of POG's nodes into the following types of nodes:

  ** was ist mit Praedikaten, die keine Relation verwenden **

	1. the start node $N_0$ with $|P_{N_0}| = 0$

	2. nodes $N_i$ with $|P_{N_i}| = 1$ and $|R_{N_i}| = 1$

	3. nodes $N_i$ with $|R_{N_i}| = 1$

	4. nodes $N_i$ with $|R_{N_i}| > 1$ but 
$\forall p \in P_{N_i}: |r(p)| = 1$

	5. nodes $N_i$ with $\exists p \in P_{N_i}: |r(p)| \ge 2$

5.2.5.3 Determine the cardinalities

	** Das folgende muesste eigentlich bewiesen werden - oder ? **

5.2.5.3.1 Node of type 1

	Within a POG there exist only one node of type 1. This node is
the start node of the POG.

	To determine the cardinality of node $N_0$ based on predcount results
the cardinalities $C_{R_i}$ of all relation instances have to calculated first.
This can be done by: $$C_{R_i} = \sum_t^{PC_{R_i}} a_{count}(t)$$ 

	The cardinality $C_{N_0}$ can be calculated as follows:
$$C_{N_0} = \prod_{R_i}^R C_{R_i}$$

5.2.5.3.1 Nodes of type 2 and 3

	The nodes of type 2 are all nodes which follows directly of the
POG's start node. At all of these nodes just one predicate was evaluated.
The predicate evaluated uses exactly one relation instance $R_{p_i} \in
R_{N_i}$ with $p_i \in P_{N_i}$.


	The cardinality of these nodes $C_{N_i}$ can be obtained using
predcounts results $PC_{R_i}$:
$$C_{N_i} = C_{R_i}' * \prod_{R_j}^{R \setminus R_{N_i}} C_{R_j}$$
with
$$C_{R_i}' = \sum_t^{\{e \in PC_{R_i} { | } \forall p \in P_{N_i}:
p \in solved(e)\}}
a_{count}(t)$$

	The way described above is also usable to determine the
cardinality $C_{N_i}$ of nodes of type 3.

5.2.5.3.1 Nodes of type 4

	The nodes of type 4 includes all nodes except type 1,2,3 nodes and
nodes which includes join predicates. To obtain the cardinality of such
a node the following equation has to be solved:
$$C_{N_i} = \prod_{R_m}^{R_{N_i}} C_{R_m}' * 
\prod_{R_j}^{R \setminus R_{N_i}} C_{R_j}$$ with
$$C_{R_m}' = \sum_t^{\{e \in PC_{R_m} { | } \forall p \in P_{N_i}:
p \in solved(e)\}} a_{count}(t)$$.

	The both equations above are the general form of the equations
for type 1,2 and 3 nodes.

5.2.5.3.1 nodes of type 5

	The nodes of type 5 are nodes including join predicates. The
source is not still able to deal with these predicates.

5.2.6 The cardinalities of the example's nodes

  The sql query to be optimized contains three predicates (see lines 010 - 060):

  * $P1: attr(a:pLZ, 1, u)>40000$

  * $P2: attr(b:pLZ, 1, u)<50000$

  * $P4: attr(a:pLZ, 1, u)<50000$

  The three predicates uses two relation instances. Therefore two
predcounts execution plans were created (lines 232 and 242).

  * $query plz feed \{b\} predcounts [ P2: (.PLZ\_b < 50000) ] consume;$

  * $query plz  feed  \{a\} predcounts [ P4: (.PLZ\_a < 50000), 
P1: (.PLZ_a > 40000) ]  consume;$

  The queries's result was stored in the variable Result as follows:

----
    310 Result =
    320 [
    330   [
    331     [2, 21470],
    332     [0, 19797]
    333   ],
    340   [
    341     [5, 3127],
    342     [1, 19797],
    343     [4, 18343],
    344     [0, 0]
    345   ]
    350 ]
----

  Using the predcounts result's above we get:

  * $R = \{ R_a, R_b \}$

  * $PC_{R_a} = \{ (5,3127), (1,19797), (4,18343), (0,0) \}$

  * $PC_{R_b} = \{ (2,21470), (0,19797) \}$

  By adding some values we get additionally:
$$C_{R_a} = \sum_t^{PC_{R_a}} a_{count}(t) = 3127 + 19797 + 18343 + 0 = 41267$$
$$C_{R_b} = \sum_t^{PC_{R_b}} a_{count}(t) = 21470 + 19797 = 41267$$

  At first all nodes of the example's POG are of the type 1 to 4.
So the cardinality of each POG's node can be obtained by using the
equations for type 4 nodes.


5.2.6.1 node $N_0$ (type 1)

	$P_{N_0} = \emptyset$ and $R_{N_0} = \emptyset$

  $$C_{N_0} = \prod_{R_i}^R C_{R_i} $$
$$ C_{N_0} =  \prod_{R_i}^{\{ R_a, R_b\}} C_{R_i} $$
$$ C_{N_0} =  C_{R_a} * C_{R_b} $$
$$ C_{N_0} = 41267 * 41267 $$
$$ C_{N_0} = 1702965289 $$

5.2.6.2 node $N_1$ (type 2)

	$P_{N_1} = \{ p_1 \}$ and $R_{N_1} = \{ R_a \}$

  $$ C_{N_1} = C_{R_{N_1}}' * \prod_{R_i}^{R \setminus R_{N_1}} C_{R_i}$$
$$ C_{N_1} = C_{R_a}' * \prod_{R_i}^{R \setminus \{ R_a \}} C_{R_i} $$
$$ C_{N_1} = C_{R_a}' * \prod_{R_i}^{\{ R_b \}} C_{R_i} $$
$$ C_{N_1} = C_{R_a}' * C_{R_b} $$
$$ C_{N_1} = \left( \sum_t^{\{e \in PC_{R_a} { | } \forall p \in \{ p_1 \}: 
p \in solved(e)\}} a_{count}(t) \right) * C_{R_b} $$
$$ C_{N_1} = \left( \sum_t^{\{ (5,3127), (1,19797) \}} a_{count}(t)
\right) * C_{R_b} $$
$$ C_{N_1} = ( 3127 + 19797 ) * 41267$$
$$ C_{N_1} = 946004708 $$

5.2.6.3 node $N_2$ (type 2)

	$P_{N_2} = \{ p_2 \}$ and $R_{N_2} = \{ R_b \}$

$$ C_{N_2} = C_{R_b}' * C_{R_a} $$
$$ C_{N_2} = \left( \sum_t^{\{e \in PC_{R_b} { | } \forall p \in \{ p_2 \}: 
p \in solved(e)\}} a_{count}(t) \right) * C_{R_a} $$
$$ C_{N_2} = \left( \sum_t^{\{ (2,21470) \}} a_{count}(t)
\right) * C_{R_a} $$
$$ C_{N_2} = ( 21470 ) * 41267$$
$$ C_{N_2} = 886002490 $$

5.2.6.4 node $N_4$ (type 2)

	$P_{N_4} = \{ p_4 \}$ and $R_{N_4} = \{ R_a \}$

$$ C_{N_4} = C_{R_a}' * C_{R_b} $$
$$ C_{N_4} = \left( \sum_t^{\{e \in PC_{R_a} { | } \forall p \in \{ p_4 \}: 
p \in solved(e)\}} a_{count}(t) \right) * C_{R_b} $$
$$ C_{N_4} = \left( \sum_t^{\{ (5,3127), (4,18343) \}} a_{count}(t)
\right) * C_{R_b} $$
$$ C_{N_4} = ( 3127 + 18343 ) * 41267$$
$$ C_{N_4} = 886002490 $$

5.2.6.4 node $N_5$ (type 3)

	$P_{N_5} = \{ p_1, p_4 \}$ and $R_{N_5} = \{ R_a \}$

$$ C_{N_4} = C_{R_a}' * C_{R_b} $$
$$ C_{N_4} = \left( \sum_t^{\{e \in PC_{R_a} { | } \forall p \in \{ p_1, p_4 \}:
p \in solved(e)\}} a_{count}(t) \right) * C_{R_b} $$
$$ C_{N_4} = \left( \sum_t^{\{ (5,3127) \}} a_{count}(t)
\right) * C_{R_b} $$
$$ C_{N_4} = ( 3127 ) * 41267$$
$$ C_{N_4} = 129041909 $$


5.2.6.5 node $N_3$ (type 4)

	$P_{N_3} = \{ p_1, p_2 \}$ and $R_{N_3} = \{ R_a, R_b \}$

  $$ C_{N_3} = \prod_{R_m}^{R_{N_3}} C_{R_m}' * 
\prod_{R_j}^{R \setminus R_{N_3}} C_{R_j} $$
$$ C_{N_3} = \prod_{R_m}^{\{R_a, R_b\}} C_{R_m}' * 
\prod_{R_j}^{R \setminus \{ R_a, R_b \}} C_{R_j} $$
$$ C_{N_3} = \prod_{R_m}^{\{R_a, R_b\}} C_{R_m}' * 
\prod_{R_j}^{\emptyset} C_{R_j} $$
$$ C_{N_3} = \prod_{R_m}^{\{R_a, R_b\}} C_{R_m}'$$
$$ C_{N_3} = C_{R_a}' * C_{R_b}' $$

  ** aus nachfolgender Formel ergibt sich noch ein Fehler (p in Durchschnitt!) **

$$ C_{N_3} = \sum_t^{\{e \in PC_{R_a} { | } \forall p \in P_{N_3}:
p \in solved(e)\}} a_{count}(t) * 
\sum_t^{\{e \in PC_{R_b} { | } \forall p \in P_{N_3}:
p \in solved(e)\}} a_{count}(t)$$.

  ** denn eigentlich muss folgendes herauskommen {p1} statt {p1,p2} **

$$ C_{N_3} = \sum_t^{\{e \in PC_{R_a} { | } \forall p \in \{ p_1 \}: 
p \in solved(e)\}} a_{count}(t) * 
\sum_t^{\{e \in PC_{R_b} { | } \forall p \in \{ p_2 \}: 
p \in solved(e)\}} a_{count}(t)$$
$$ C_{N_3} = \sum_t^{\{ (5,3127), (1,19797) \}} a_{count}(t) * 
\sum_t^{\{ (2,21470) \}} a_{count}(t) $$
$$ C_{N_3} = ( 3127 + 19797 ) * 21470 $$
$$ C_{N_3} = 22924 * 21470 $$
$$ C_{N_3} = 492178280 $$


5.2.6.5 node $N_6$ (type 4)

	dto

5.2.6.5 node $N_7$ (type 4)

	dto


5.2.7 What means transformed in case of the result set?

  ** der Abschnitt muss woanders hin **

  To explain let us look at the native??? results of the 
predcounts execution plans.

----
66 ?- secondo('query plz  feed  {b} predcounts [ P2: (.PLZ_b < 50000) ] 
 consume;', Z).
0.init finished
Total runtime ...   Times (elapsed / cpu): 0.800012sec / 0.61sec = 1.3115

410 Z = 
420 [
430   [rel, [tuple, [[Atom, int], [Count, int]]]], 
440   [
441     [1, 21470], 
442     [0, 19797]
443   ]
450 ]

Yes
67 ?- secondo(, A).
0.init finished
Total runtime ...   Times (elapsed / cpu): 0.940014sec / 0.89sec = 1.0562

510 A = 
520 [
530   [rel, [tuple, [[Atom, int], [Count, int]]]], 
540   [
541     [3, 3127], 
542     [2, 19797], 
543     [1, 18343], 
544     [0, 0]
545   ]
550 ]

Yes
----

  As you can see??? the tuples of the result are identified by
0 and 1. The tuple identified by 0 shows the number of rows of the
relation plz doesn't solve??? the sql predicate " PLZ < 50000 " . The
other tuple shows the number of rows solve the sql predicate. But
where should these results are written at the POG?

  For each predcounts execution plan there was created a 
transformation list (lines 231 and 241). For the example we have to
look at the first one. The list contains just one element. So the only
one predicate of predcounts operator corresponds with this element.
This means 21470 rows of plz solve the predicate identified with 2 in
the POG and 19797 rows don't do so???. So one is able to set the
cardinalty of node 2 (binary: 010) to 21470. The value 19797 cann't
be used directly. To used this value one have to combine them with 
the result tuples of the other predcounts queries identified with
atom = 0.

  Let's have a look at the second predounts query (lines 242),
their transformation list (line 241) and their native result list (lines
540-545). The predcounts query builds statistics of two predicates P4
and P1. Using the transformation list one is able to determine the
identifiers of the predicates within the POG. Within the POG the
predicate P1 is identified by 1 (binary 001) and the predicate P4
by 4 (binary 100). Within the predcounts result set the predicate P4
is identified by 2 (binary 010) and P1 by 1 (binary 001). To transform
the identifiers (column atom) of the native result set the identifiers
have to be changed by the following way:

  1 Each identifier of the tuples (lines 541-544) is composed of
the predicate identifiers. By looking at the identifiers in binary mode
you are able to recognize this. The identifier is the sum??? of all
of the identifiers of solved predicates. In case of the example it means:

    * atom=3 - binary 11 - predicates 1 and 2 are solved

    * atom=2 - binary 10 - predicate 2 is solved AND predicate 1 is not

    * atom=1 - binary 01 - predicate 1 is solved AND predicate 2 is not

    * atom=0 - binary 00 - predicates 1 and 2 are not solved

  2 The transformation list (line 241) means:

    * predicate 1 is identified by 1 within the POG

    * predicate 2 is identified by 4 within the POG

  3 In case of predicate 1 there is nothing to do.

  4 In case of predicate 2 it's identifier has to be changed to 4.

  5 As you've seen above in the case of a solved predicate 2 the bit
2\^2 is set. (atom=3 and atom=2) To transform the tuples for using
for POG the bit 2i\^3 has to be set instead of bit 2i\^2. For the
result 11 has to be transformed to 101 and 10 to 100.



  ** hier weiter **


----
66 ?-
----

  
*/
/*
1 Helper predicates

  The following predicates are used for easier programming and
understanding of the main code.

1.1 debug\_write

----    debug_write(Message)
----

  The predicate debug\_write writes the given message to 
the screen
without a trailing newline. The message is only written
if the fact debug\_level(X) with X greater then 0 is set.

*/

debug_write(_):- % if X=0 then no debugging messages  
   % if fact debug_level is not set retract fails
   retract((debug_level(X))),
   % reassert the fact
   assert((debug_level(X))),
   % X=0 no debug output requested
   X=0,
   !.
% write debugging messages without concated newline
debug_write(Message):-
   % if fact debug_level is not set retract fails
   retract((debug_level(X))),
   % reassert the fact
   assert((debug_level(X))),
   % X>0 debug output requested
   X > 0,
   % write debug message
   write(Message),
   !.
debug_write(_).       % in any other case ignore call of debug_write

/*
1.2 debug\_writeln

----    debug_writeln(message)
        debug_writeln(Msg1, Msg2)
        debug_writeln(Msg1, Msg2, Msg3)
        debug_writeln(Msg1, Msg2, Msg3, Msg4)
----

  The predicates debug\_writeln write the given message(s) to the 
screen. The
given message(s) are concatinated without any delimiter but a newline
will be appended behind??? the last message. The message(s) are only 
written if the fact debug\_level(X) with X greater then 0 is set.

*/

% if X=0 then no debugging messages
debug_writeln(_):-
   % if fact debug_level is not set retract fails
   retract((debug_level(X))),
   % reassert the fact
   assert((debug_level(X))),
   % X=0 no debug output requested
   X=0,
   !.
% write debugging messages concated newline
debug_writeln(Message):-
   retract((debug_level(X))),
   assert((debug_level(X))),
   X > 0,
   debug_write(Message),
   writeln(''),
   !.
debug_writeln(_).

/* 
  The following three predicates write two up to four messages 
using the predicates debug\_write/1 and debug\_writeln/1

*/

% write two messages
debug_writeln(Msg1,Msg2):-
   debug_write(Msg1),
   debug_writeln(Msg2).

%write three messages
debug_writeln(Msg1,Msg2,Msg3):-
   debug_write(Msg1),
   debug_write(Msg2),
   debug_writeln(Msg3).

% write four messages
debug_writeln(Msg1,Msg2,Msg3,Msg4):-
   debug_write(Msg1),
   debug_write(Msg2),
   debug_write(Msg3),
   debug_writeln(Msg4).

/*
1.3 atom\_concat/6

----    atom_concat(Txt1, Txt2, Txt3, Txt4, Txt5, Rslt)
----

  The predicate atom\_concat/6 concatinates five strings to one. It uses
the already defined predicate atom\_concat/3.

*/

% concat more than two strings
atom_concat(Txt1, Txt2, Txt3, Txt4, Txt5, Rslt):-
   atom_concat(Txt1, Txt2, TMP1),
   atom_concat(TMP1, Txt3, TMP2),
   atom_concat(TMP2, Txt4, TMP3),
   atom_concat(TMP3, Txt5, Rslt).
