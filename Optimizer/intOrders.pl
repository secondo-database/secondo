/*
----
This file is part of SECONDO.

Copyright (C) 2005, University in Hagen, Department of Computer Science, 
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
//[toc] [\tableofcontents]
//[newpage] [\newpage]

[10] An Optimizer-Extension for to Consider Interesting Orders

Jan Engelkamp, December 2005 - February 2006

[toc]

[newpage]

1 Introduction

The original optimizer of the SECONDO DBMS (which can be found in the file ~optimizer.pl~ in the ~Optimizer~-directory of SECONDO) first constructs a so-called predicate order graph (POG) for a given query and then uses the algorithm of dijkstra to find the shortest path through this graph. This shortest path is the base of the (estimated) cheapest plan for the evaluation of the query. The optimizer is described in detail in in R.H. G[ue]ting, T. Behr, V. T. de Almeida, Z. Ding, F. Hoffmann, and M. Spiekermann, Secondo: An Extensible DBMS Architecture and Prototype. Fernuniversit[ae]t Hagen, Informatik-Report 313, March 2004, www.informatik.fernuni-hagen.de/import/pi4/papers/ Secondo04.pdf, page 7 ff.

A modification of that optimizer, that can be found in the file ~modifications.pl~, does not first construct the complete POG and then searches the shortest path through it, but immediately constructs the shortest path and hence prunes away large parts of the POG. (Thus it is more efficient than the original optimizer.) (The modifications are now located in a file called ``immediateplan.pl'', C. D[ue]ntgen.)

In the present file called ~intOrders.pl~ this modification is extended in that way, that the optimizer takes so-called 'interesting orders' into consideration while constructing the shortest path through the (imagined) predicate order graph. The term interesting orders means the orders of base-relations and intermediate results, that possibly allow to choose cheaper (join-) operators to process tuple-streams (and hence to create cheaper cost-edges of the POG), if they are considered, in comparison to the case, where the orders are ignored.

Taking interesting orders into consideration is the prerequisite to notice, if a certain join-implementation can be used, that is very efficient in comparison to other join-implementations: the mergejoin. Hence it can be said, that the aim of the following extension of the SECONDO-optimizer is to translate joins into a mergejoin-operator whenever it is convenient, to consequently construct cheaper plans than the original optimizer in some cases.

To extend respectively change the knowledge-base of the optimizer in that way, that it considers interesting orders, please enter ~intOrders(on)~ after starting ~SecondoPL~ as usual and entering the Prolog-command ~consult('intOrders.pl')~ respectively ~['intOrders.pl']~. Please ensure, that the file ~modifications.pl~ can be found in the ~Optimizer~-directory of the SECONDO-System as well.

~intOrders(off)~ turns all modifications off, so that one can use the original implemented algorithm of the optimizer again.

Further program-modes exist, that can be used to research into the different aspects and effects of considering interesting orders:

  1 After entering ~intOrders(quick)~ for all following queries to evaluate, the interesting orders of base relations and intermediate results are considered in that way, that always the cheapest cost-edge from one node of the POG to its successor is chosen - considering the orders of the incoming tuple-streams, so that the mergejoin can be used if possible. Every time a cost-edge from one node to its successor is chosen as the cheapest one, implicitly a decision is made about the order of its result. In the end it could be, that not the cheapest of all possible paths through the POG is created, because a slight possibility always exist, that a very cheap cost-edge from one node M to its successor N could be created, if the intermediate result belonging to M would be of a certain order, that is different from the order, that it actually has, if on the path from the source-node of the POG to M always the cheapest of all cost-edges has been chosen. The advantage of this program-mode is, that it works very fast and efficient.

  2 After entering ~intOrders(path)~ for all following queries to evaluate, the interesting orders of base relations and intermediate results are considered in that way, that the shortest path to construct does not only base on the nodes of the POG, but on a larger number of nodes. For every single node of the POG multiple 'versions' (twins) are stored and considered, dependent on the plan- respectively cost-edges, that actually lead to them. That is done, because the certain path, that leads to the respective node, determines the order of its result.

  3 After entering ~intOrders(on)~ the interesting orders of base relations and intermediate results are considered in that way, that the shortest path to construct does neither base on the nodes of the POG nor on nodes that represent certain paths, but on the nodes of the POG plus their actual order (to be correct: not the nodes are ordered, but the accompanying intermediate results). That means, that for every single node of the POG multiple 'versions' (twins) are considered, dependent on the plan- respectively cost-edges, that lead to the respective node (and that determine the order of its accompanying intermediate result). But only those node-versions are stored into the set of already created but not yet ticked off nodes, whose intermediate results do have orders, that are new in the sense, that there is not yet a node inside that set, that does have the same POG-number and the same order. If there is such a node already inside that set, the new version is only stored, if the total cost, that is caused on the path from the source-node to the respective node, is lower than the cost stored for the former created node-version.

  4 After entering ~intOrders(test)~ for all following queries to evaluate, the interesting orders of base relations and intermediate results are considered in the same way as after entering ~intOrders(quick)~. But the nodes, that are created and stored, do have the shape of the nodes, that are created after entering ~intOrders(on)~ or ~intOrders(path)~. This program-mode is only interesting for to test this extension of the SECONDO-optimizer.


[newpage]

2 Interesting Orders

2.1 The Idea

The original SECONDO-optimizer does not consider, if a relation is in a certain order or not - neither if a base relation, taken from the database, is ordered nor an intermediate result. But orders can be really interesting for query optimization, because the so-called mergejoin is a very efficient implementation for equi-joins. It causes costs of only "O(n+m)"[1], if one of the two given relations respectively tuple-streams consists of n tupels and the other of m tuples. But it can only be used for relations/tuple-streams, that are ordered (strictly speaken: that are ordered in exactly the same way). Hence for to decide, if a mergejoin instead of some other join-implementation can be used, one has to know, if the involved relations are ordered.

Consequently it is necessary:

  1 to determine, what the orders of the base relations are, that are part of the database (strictly speaken: Which attribute is the one, that determines the primary order of the respective relation. The type of the order - numerically or alphabetically, ascending or decending - is not interesting for the SECONDO-system, because for every data type it is exactly defined, what it means, if a relation is ordered. If a relations order is for example determined by a numerical attribute, it is defined, that the order is numerically ascending. If it would be decending, then for the SECONDO-System the relation is unordered.)

  2 to trace, how the orders are passed on to intermediate results. For intermediate relations, that are the result of a selection or a projection on say a relation A it is easy, to determine its order: its simply the order of the given relation A. But for joins the results order is dependent on the respective join-implementation, that is chosen to process the join.

The mergejoin passes on the order of the two original/given relations respectively tuple-streams, because in principle it is just a step-by-step scan of the two relations and a concatenation of those tupels, that match the join-condition. The loopjoin passes on the order of that one of the two original relations, that is used as the so-called 'outer relation' by the loopjoin-operator. The hashjoin does not pass on any of the orders of the original relations. The sortmergejoin sorts both of the original relations accordant to the join-predicate. Hence the order of its result does not depent on the orders of the given tuple-streams but on the join-predicate.

The sortmergejoin sorts both of the original relations accordant to the join-predicate. But there are cases, where it is not necessary to actually sort both relations, but only one and then use the mergejoin instead of the sortmergejoin. Hence instead of choosing one of the appropriate implementations for a join to process, in such a case two operators are concatenated: a sort-operator on one of the relations/tuple-streams and a mergejoin. In other words: A sort-operator is additionally inserted into the plan and the mergejoin is chosen as the appropriate join-implementation.

This principle (or mechanism) is described in detail in Guy M. Lohman, Grammar-like Functional Rules for Representing Query Optimization Alternatives, Proceedings of ACM-SIGMOD, 1988, p. 18-27. But it is described more general and therefore the term 'glue-operator' is used for operators, that are additionally inserted to ascertain a characteristic of the given relations/tuple-streams - like the sort-operator ascertains a respective order.

2.2 Different Ways of Taking Interesting Orders into Consideration

The algorithm of the modified SECONDO-optimizer is described in ~modifications.pl~, chapter 2.1 'The Idea and Algorithm'. By means of calling ~intOrders(quick)~  this algorithm (to be correct: the program, that realizes this algorithm) is extended with the following phases:

  1 Determine the orders of the base-relations and store them into the PROLOG-interpreters knowledge-base, when the ~argument~-elements are stored.

  2 When a new node is created as the successor of an already existing node, determine the orders of all relevant intermediate results and store this information as the order-information for that new node.

  3 When a new egde has been created, check the possibility of using a mergejoin as the appropriate operator for realizing that edge (and the possibility of using the glue-operator 'sort' plus a mergejoin).

  4 If a mergejoin can be used, determine the order of its result and store it as the order-information for the edges target-node.

  5 Together with all the other possible plan- and cost-edges create a mergejoin-plan- respectively cost-edge as well, if possible (or a plan-/cost-edge, that realizes a glue- plus the mergejoin-operator).

  6 After the cheapest of all cost-edges from one node to its successor has been determined, check, what operator it realizes. If it is a join-operator but no mergejoin (with or without a glue-operator in advance), correct the order-information stored for the successors result. If it is a mergejoin-operator, then the results order has already been determined.

By means of calling ~intOrders(path)~ the algorithm of the modified SECONDO-optimizer (to be correct: the program, that realizes this algorithm) is extended with the following phases:

  1 Determine the orders of the base-relations and store them into the PROLOG-interpreters knowledge-base, when the ~argument~-elements are stored.

  2 When a new node is created as the successor of an already existing node, determine the orders of all relevant intermediate results and store this information as the order-information for that new node.

  3 When a new egde has been created, check the possibility of using a mergejoin as the appropriate operator for realizing that edge (and the possibility of using the glue-operator 'sort' plus a mergejoin).

  4 If a mergejoin can be used, determine the order of its result and store it as the order-information for the edges target-node.

  5 Together with all the other possible plan- and cost-edges create a mergejoin-plan- respectively cost-edge as well, if possible (or a plan-/cost-edge, that realizes a glue- plus the mergejoin-operator).

  6 For every single cost-edge from one node to all its successors insert a separate target-node into the set of already created, but not yet ticked off nodes. Apart from all the information, that constitute a node of the POG, store the order information of the cost-edges result as well.

  7 When selecting the node with the minimal distance to the source-node from among all the nodes, contained in the set of already created, but not yet ticked off nodes, correct all relevant interesting orders of intermediate results, that are currently stored in the PROLOG-interpreters knowledge-base, so that no order, that has been determined after the respective node was put into that set, influences the orders of results, that will be determined in future.

By means of calling ~intOrders(on)~ the algorithm of the modified SECONDO-optimizer (to be correct: the program, that realizes this algorithm) is extended with the following phases:

  1 Determine the orders of the base-relations and store them into the PROLOG-interpreters knowledge-base, when the ~argument~-elements are stored.

  2 When a new node is created as the successor of an already existing node, determine the orders of all relevant intermediate results and store this information as the order-information for that new node.

  3 When a new egde has been created, check the possibility of using a mergejoin as the appropriate operator for realizing that edge (and the possibility of using the glue-operator 'sort' plus a mergejoin).

  4 If a mergejoin can be used, determine the order of its result and store it as the order-information for the edges target-node.

  5 Together with all the other possible plan- and cost-edges create a mergejoin-plan- respectively cost-edge as well, if possible (or a plan-/cost-edge, that realizes a glue- plus the mergejoin-operator).

  6 For every single cost-edge from one node to all its successors insert a separate target-node (together with the order information of the cost-edges result) into the set of already created, but not yet ticked off nodes, if (a) no variant of that target-node is already in that set, thats cost-edges result is of the same order like the node-variant, that shall be inserted, or if (b) a variant of that target-node is already in that set, thats cost-edges result is of the same order like the node-variant, but thats distance- respectively cost-value is higher, than the cost-value of the new node-variant, that shall be inserted. (Please note, in the formulation 'a target-nodes cost-edges result' that cost-edge is meant, that leads from its predecessor to that target-node. 'A target-nodes result' would not be the correct formulation, because the optimizer differs between an edges target-node and an edges result-node. For details please see ~optimizer.pl~, chapter 2 'Data Structures'.)

  7 When selecting the node with the minimal distance to the source-node from among all the nodes, contained in the set of already created, but not yet ticked off nodes, correct all relevant interesting orders of intermediate results, that are currently stored in the PROLOG-interpreters knowledge-base, so that no order, that has been determined after the respective node was put into that set, influences the orders of results, that will be determined in future.

[newpage]

3 Preparation

3.1 Changing Clauses of the Original Optimizer

For to consider interesting orders, it is necessary to modify clauses, that are part of the original optimizer. To avoid, that the original optimizers code (i. e. the file ~optimizer.pl~) must manually be edited, the clauses are modified inside the PROLOG-interpreters knowlege-base.

First the ~partition~-clauses, defined in chapter 3.2 of ~optimizer.pl~, have to be modified. How this is done, is described in detail in the following chapter 4.2 'Interesting Orders of Base-Relations'.

Second the ~copyPart~-clauses, defined in chapter 3.5 of ~optimizer.pl~, are changed. How this is done, is described in detail in the following chapter 4.3.1 'Determining Order Information while Creating Nodes'.

Aditionally new clauses have to be added - clauses, that are needed for the translation of joins (see chapter 5.1 'Translating Joins') and for determining the costs of the respective join-operators (see chapter 5.2 'Costs'). In particular these clauses are:

----
join00(...) => mergejoin(...) :- ...
join00(...) => sortLeftThenMergejoin(...) :- ...
join00(...) => sortRightThenMergejoin(...) :- ...
plan_to_atom(mergejoin(X, Y, A, B), Result) :- ...
plan_to_atom(sortLeftThenMergejoin(X, Y, A, B), Result) :- ...
plan_to_atom(sortRightThenMergejoin(X, Y, A, B), Result) :- ...
cost(mergejoin(X, Y, _, _), Sel, S, C) :- ...
cost(sortLeftThenMergejoin(X, Y, _, _), Sel, S, C) :- ...
cost(sortRightThenMergejoin(X, Y, _, _), Sel, S, C) :- ...
----

One clause is defined, that controls all necessary changes.

*/

changeOriginalOptimizer :-
      retractPartitionClauses,
      assertPartitionClauses,
      retractCopyPartClauses,
      assertCopyPartClauses.

restoreOriginalOptimizer :-
  restorePartitionClauses,
  restoreCopyPartClauses.
/*

3.2 Changing Clauses of ~modifications.pl~

The clauses of ~modifications.pl~, that have to be changed, when intersting orders shall be considered, are the clauses of ~checkSuccessor~ (see ~modifications.pl~, chapter 2.5.6 'Creating Edges to Successors') and ~sweepKnowledgeBase~ (see ~modifications.pl~, chapter 2.4.1 'Sweeping the Knowledge-Base' and chapter 4.1 of this file 'Retracting Formerly Stored Information')

For to consider interesting orders in different ways, different ~checkSuccessor~-clauses are defined. What that means, is described in the following chapter.

*/

changeModificationsPL0 :-
      assertCheckSuccessorClauses0.

changeModificationsPL1 :-
      assertCheckSuccessorClauses1.

changeModificationsPL2 :-
      assertCheckSuccessorClauses2.

restoreModifications :-
  restoreCheckSuccessorClauses.

/*

3.3 Turning On and Off the Different Program-Modes

To extend respectively change the knowledge-base of the SECONDO-optimizer in that way, that it considers interesting orders, please enter ~intOrders(quick)~, ~intOrders(on)~, ~intOrders(path)~ or ~intOrders(test)~ after starting ~SecondoPL~ as usual and entering the Prolog-command ~consult('intOrders.pl')~ respectively ~['intOrders.pl']~. Please ensure, that the file ~modifications.pl~ can be found in the ~Optimizer~-directory of the SECONDO-System as well.

~intOrders(off)~ turns all modifications off, so one can use the original implemented algorithm of the optimizer again.

After entering ~intOrders(quick)~ for all following queries to evaluate, the interesting orders of base relations and intermediate results are considered in that way, that always the cheapest cost-edge from one node of the POG to its successor is chosen - considering the orders of the incoming tuple-streams, so that the mergejoin can be used if possible. Every time a cost-edge from one node to its successor is chosen as the cheapest one, implicitly a decision is made about the order of its result. In the end it could be, that not the cheapest of all possible paths through the POG is created, because a slight possibility always exist, that a very cheap cost-edge from one node M to its successor N could be created, if the intermediate result belonging to M would be of a certain order, that is different from the order, that it actually has, if on the path from the source-node of the POG to M always the cheapest of all cost-edges has been chosen.

The advantage of this program-mode is, that it is very much faster than the ~intOrders(path)~- and the ~intOrders(on)~-mode.


~intOrders(quick)~ replaced by ~setOption(intOrders(quick))~ in file ``calloptimizer.pl''.

*/




/*

After entering ~intOrders(path)~ for all following queries to evaluate, the interesting orders of base relations and intermediate results are considered in that way, that the shortest path to construct does not only base on the nodes of the POG, but on a larger number of nodes. For every single node of the POG multiple 'versions' (twins) are stored and considered, dependent on the plan- respectively cost-edges, that actually lead to them. That is done, because the certain path, that leads to the respective node, determines the order of its result.

~intOrders(path)~ replaced by ~setOption(intOrders(path))~ in file ``calloptimizer.pl''.

*/


/*

After entering ~intOrders(path)~ the target and result of every single plan- respectively cost-edge cause a new node to be stored into the set of already created but not yet ticked off nodes. Hence this set becomes really big, if a query is rather complex.

After entering ~intOrders(on)~ the interesting orders of base relations and intermediate results are considered in that way, that the shortest path to construct does neither base on the nodes of the POG nor on nodes, that represent certain paths, but on the nodes of the POG plus their actual order (to be correct: not the nodes are ordered, but the accompanying intermediate results). That means, that for every single node of the POG multiple 'versions' (twins) are considered, dependent on the plan- respectively cost-edges, that lead to the respective node (and that determine the order of its accompanying intermediate result). But only those node-versions are stored into the set of already created but not yet ticked off nodes, whose intermediate results do have orders, that are new in the sense, that there is not yet a node inside that set, that does have the same POG-number and the same order. If there is such a node already inside that set, the new version is only stored, if the total cost, that is caused on the path from the source-node to the respective node, is lower than the cost stored for the former created node-version.

~intOrders(on)~ replaced by ~setOption(intOrders(on))~ in file ``calloptimizer.pl''.

*/


/*

After entering ~intOrders(test)~ for all following queries to evaluate, the interesting orders of base relations and intermediate results are considered in the same way as after entering ~intOrders(quick)~. But the nodes, that are created and stored, do have the shape of the nodes, that are created after entering ~intOrders(on)~ or ~intOrders(path)~.

~intOrders(test)~ replaced by ~setOption(intOrders(test))~ in file ``calloptimizer.pl''.

*/

/*

~intOrders(off)~ turns all modifications off, so that one can use the original implemented algorithm of the optimizer again.

~intOrders(off)~ replaced by ~delOption(intOrders(X))~ in file ``calloptimizer.pl''.

*/

/*

Please note, that the call of ~intOrders(A)~, in which any other fact than ~on~, ~path~ or ~test~ unifies with A, implies, that the implementation for the set containing the already created but not yet ticked off nodes, that is realized in chapter 6.2 'A Reached-Nodes-Set-Implementation, that Stores Interesting Orders', will be deactivated as well as the implementation of the set for ticked off nodes, that is realized in chapter 6.3 'A Ticked-Off-Nodes-Set-Implementation, that Stores Interesting Orders'.

*/

intOrdersPrintWelcomeIO :-
     nl, write('*** Interesting orders are taken'),
     nl, write('*** into consideration from now on.'), nl.

intOrdersPrintWelcomePOGIO :-
     nl, write('*** From now on the original SECONDO-optimizer'),
     nl, write('*** will be used.'), nl.
	

/*
Turn off the the interesting orders extensions of the optimizer. Restore original clauses of standard optimizer and immediate plan algorithms.

*/

turnOffIntOrders :-
  doNotCreateMergeJoinPlanEdges,
  restoreOriginalOptimizer,
  restoreModifications,
  restoreStoredNodesShape,
  restorePutIntoReachedNodesSetIOClauses.

/*

3.4 Observing, How the Optimizer in Detail Works

There are goals in several clauses of this file existing just for observing, how the modified optimizer in detail works. They are currently commented out. Search und replace the per-cent-symbol directly followed by the grid-symbol with the goal ~o b s e r v e~ and a lot of detailed information about how the optimizer works is printed on your screen while queries are evaluated. Search und replace ~o b s e r v e~ with the per-cent-symbol directly followed by the grid-symbol later to restore the original state of this program.

May 2006, Christian D[ue]ntgen: ~observe~ was replaced by ~dm/m~ and ~dc/2~. If you want to switch on the verbose mode, just query for 
---- setOption(debug), debugLevel(intOrders). 
----

at runtime. ~delOption(debug)~ or ~noDebug(intOrders)~ should stop the verbose mode.

Because it is not very helpful, when a lot of output races over your screen, you can change the existing file, so that the run of the optimizer stops after every interesting step - interesting in relation to the consideration of interesting orders. Just search and replace the per-cent-symbol directly followed by the exclamation-mark with the goal ~p a u s e~ and you must press the enter-key of your keyboard a few times while the optimizer evaluates a query. Search und replace ~p a u s e~ with the per-cent-symbol directly followed by the exclamation-mark later to restore the original state of this program.

*/

%!.

/*

[newpage]

4 Considering Interesting Orders

4.1 Retracting Formerly Stored Information

When the optimizer evaluates a query, first of all formerly stored information about interesting orders must be retractet from the knowledge-base, if existing.

---- enhanceSweepKnowledgeBaseClause/0 
----

Extension added as conditional code to predicate ~seepKnowledgeBaseClause~ in 
file ``immediateplan.pl''.

*/

retractAllOrderInformation :-
	retractIntOrders,
	retractTempOrderNotes,
	retractEqualOrders,
	retractOrderBackups,
	retractMinDistNodeOrders.

retractIntOrders :-
	not(retractIntOrder).

retractIntOrder :-
	retract(intOrder(_, _)),
	fail.

retractOrderBackups :-
	not(retractOrderBackup).

retractOrderBackup :-
	retract(orderBackup(_, _, _, _, _)),
	fail.

retractEqualOrders :-
	not(retractEqualOrder).

retractEqualOrder :-
	retract(equalOrder(_, _)),
	fail.

retractTempOrderNotes :-
	not(retractTempOrderNote).

retractTempOrderNote :-
	retract(tempOrderNote(_)),
	fail.

retractMinDistNodeOrders :-
	not(retractMinDistNodeOrder).

retractMinDistNodeOrder :-
	retract(minDistNodeOrder(_, _, _)),
	fail.

/*

4.2 Interesting Orders of Base-Relations

At the moment the SECONDO-DBMS does not provide information about the orders of the relations contained in the respective database. Hence the following facts are arbitrary defined and maybe do not represent the actual orders of an existing database.

Later some file ~storedOrders.pl~ shall automatically be generated by the SECONDO-DBMS (equivalent to the files ~storedCards.pl~, ~storedIndexes.pl~ and ~storedRels.pl~ for example), that contains automatically generated facts of the following shape:

----
storedOrder(Relation, Attribute)
----

At the moment, corresponding to every of the following ~storedRel~-facts a ~storedOrder~-fact has manually be defined.

----
storedRel(staedte, [sname, bev, plz, vorwahl, kennzeichen]).
storedRel(orte, [kennzeichen, ort, vorwahl, bevt]).
storedRel(plz, [plz, ort]).
----

For a better clarity the ~storedOrder~-facts are part of this file. They must be commented out later, if the above mentioned file ~storedOrders.pl~ exists.

*/

storedOrder(staedte, sName).
storedOrder(orte, kennzeichen).
storedOrder(plz, ort).

/*

Suggested alternatives for testing the optimizer:

----
storedOrder(staedte, pLZ).
storedOrder(orte, kennzeichen).
storedOrder(plz, pLZ).

storedOrder(staedte, sname).
storedOrder(orte, ort).
storedOrder(plz, ort).
----

The stored orders are used to determine the orders of the arguments, that represent the base relations inside the optimizers knowledge-base. Therefore the ~partition~-clauses, defined in chapter 3.2 of ~optimizer.pl~, must be modified. But instead of editing the file ~optimizer.pl~ manually, the original static procedure ~partition~ is defined as dynamic, the existing clauses are retracted from the knowledge-base and new clauses are asserted.

*/

% The following line was moved into file 'optimizer.pl':
%:- dynamic partition/3.

retractPartitionClauses :-
	not(retractPartitionClause).

retractPartitionClause :-
	retract(partition(_, _, _) :- (_) ),
	fail.

assertPartitionClauses :-
	assertz(partition([], _, [])
	),
	assertz(partition([Rel | Rels], N, [Arp | Arps]) :-
	(Rel = rel(RelName, _, _),
	 % if
	 storedOrder(RelName, Attribute),
	 !,
	 % then
  	 N1 is N-1,
  	 Arp = arp(arg(N), [Rel], []),
  	 assert(argument(N, Rel)),
  	 partition(Rels, N1, Arps),
	 dm(intOrders, ['relation ', Rel, ' is named arg(', N, ').\n']),
	 asserta(intOrder(arg(N), Attribute)),
	 dm(intOrders, [ 'It is ordered by: ', Attribute, '\n']),
	 true
	 )
	),
	assertz(partition([Rel | Rels], N, [Arp | Arps]) :-
	(% else
  	 N1 is N-1,
  	 Arp = arp(arg(N), [Rel], []),
  	 assert(argument(N, Rel)),
  	 partition(Rels, N1, Arps),
	 dm(intOrders, ['relation ', Rel, ' is named arg(', N, ').\n']),
	 asserta(intOrder(arg(N), noOrder)),
	 dm(intOrders, ['There is no order stored.\n']),
	 true
	 )
	).

/*

4.3 Interesting Orders of Intermediate Results

4.3.1 Determining Order Information while Creating Nodes

The ~copyPart~-clauses, defined in chapter 3.5 of ~optimizer.pl~, must be changed. But instead of changing the file ~optimizer.pl~ manually, the original static procedure ~copyPart~ is defined as dynamic, the existing clauses are retracted from the knowledge-base and new clauses are asserted.

The new goals inside the following ~copyPart~-clauses just determine the orders of the arguments of the relevant ~arp~-elements (see chapter 2 of ~optimizer.pl~) and call ~assertintOrderResNo~ to store all information about the arguments, that is later needed to determine the order of the respective result.

*/

:- dynamic copyPart/4.

retractCopyPartClauses :-
	not(retractCopyPartClause).

retractCopyPartClause :-
	retract(copyPart(_, _, _, _) :- (_) ),
	fail.

assertCopyPartClauses :-
	assertz(copyPart(_, _, [], [])
	),
	assertz(copyPart(pr(P, Rel), PNo, Arps, [Arp2 | Arps2]) :-
  	 (select(X, Arps, Arps2),
  	 X = arp(Arg, Rels, Preds),
  	 member(Rel, Rels), !,
  	 nodeNo(Arg, No),
  	 ResNo is No + PNo,
  	 Arp2 = arp(res(ResNo), Rels, [P | Preds]),
	 dm(intOrders, ['res(', ResNo, ') is generated with predicate ', 
                        P, ' from ', Arg, '\n']),
	 intOrder(Arg, ArgAttribute),
	 assertintOrderResNo(ResNo, ArgAttribute)
	 )
	),
	assertz(copyPart(pr(P, R1, R2), PNo, Arps, [Arp2 | Arps2]) :-
  	(select(X, Arps, Arps2),
  	 X = arp(Arg, Rels, Preds),
  	 member(R1, Rels),
  	 member(R2, Rels), !,
  	 nodeNo(Arg, No),
  	 ResNo is No + PNo,
  	 Arp2 = arp(res(ResNo), Rels, [P | Preds]),
	 dm(intOrders, ['res(', ResNo, ') is generated with predicate ', 
                        P, ' from ', Arg, '\n']),
	 intOrder(Arg, ArgAttribute),
	 assertintOrderResNo(ResNo, ArgAttribute)
	 )
	),
	assertz(copyPart(pr(P, R1, R2), PNo, Arps, [Arp2 | Arps2]) :-
  	(% if P is an equi-join
  	 P = (Pleft = Pright),
  	 !,
	 % then
  	 select(X, Arps, Rest),
  	 X = arp(ArgX, RelsX, PredsX),
  	 member(R1, RelsX),
  	 select(Y, Rest, Arps2),
  	 Y = arp(ArgY, RelsY, PredsY),
  	 member(R2, RelsY), !,
  	 nodeNo(ArgX, NoX),
  	 nodeNo(ArgY, NoY),
  	 ResNo is NoX + NoY + PNo,
  	 append(RelsX, RelsY, Rels),
  	 append(PredsX, PredsY, Preds),
  	 Arp2 = arp(res(ResNo), Rels, [P | Preds]),
	 dm(intOrders, ['res(', ResNo, ') is generated with predicate ',
                        P, ' from ', ArgX, ' and ', ArgY, '\n']),
	 intOrder(ArgX, ArgAttributeX),
	 intOrder(ArgY, ArgAttributeY),
	 assertintOrderResNo(ResNo, ArgAttributeX, ArgAttributeY,
			     Pleft, Pright)
	 )
	),
	assertz(copyPart(pr(P, R1, R2), PNo, Arps, [Arp2 | Arps2]) :-
	(% else (P is not an equi-join.)
  	 select(X, Arps, Rest),
  	 X = arp(ArgX, RelsX, PredsX),
  	 member(R1, RelsX),
  	 select(Y, Rest, Arps2),
  	 Y = arp(ArgY, RelsY, PredsY),
  	 member(R2, RelsY), !,
  	 nodeNo(ArgX, NoX),
  	 nodeNo(ArgY, NoY),
  	 ResNo is NoX + NoY + PNo,
  	 append(RelsX, RelsY, Rels),
  	 append(PredsX, PredsY, Preds),
  	 Arp2 = arp(res(ResNo), Rels, [P | Preds]),
	 dm(intOrders, ['res(', ResNo, ') is generated with \npredicate ', 
                        P, ' from ', ArgX, ' and ', ArgY, '\n']),
	 intOrder(ArgX, ArgAttributeX),
	 intOrder(ArgY, ArgAttributeY),
	 getPleftPright(P, Pleft, Pright),
	 assertintOrderResNo(ResNo, ArgAttributeX, ArgAttributeY,
			     Pleft, Pright)
	 )
	).

/*

If a join on two given streams is not an equi-join, nevertheless the both predicate-parts are needed individually. Unfortunately a special clause is needed for every comparison operator, that is possibly used in a query.

But note, that at the moment the original SECONDO-optimizer is not able to translate other than equi-joins (for details see chapter 5.2.3 'Translation of Joins' of ~optimizer.pl~).

*/

getPleftPright(Pleft > Pright, Pleft, Pright) :-
	!.

getPleftPright(Pleft < Pright, Pleft, Pright) :-
	!.

getPleftPright(Pleft >= Pright, Pleft, Pright) :-
	!.

getPleftPright(Pleft =< Pright, Pleft, Pright) :-
	!.

getPleftPright(P, attr(error, _, _), attr(error, _, _)) :-
	write('-- ERROR -- '), nl,
	write('The operator used in the predicate '), write(P),
	write('is not known.'),
	nl, nl,
	skip(10).

/*

In the above ~copyPart~-clauses ~assertintOrderResNo~ is called to store the order of the respective result into the PROLOG-interpreters knowledge-base.

*/

assertintOrderResNo(ResNo, noOrder) :-
	% if the given attribute / tuple-stream is not ordered
	% but already stored
	intOrder(res(ResNo), noOrder),
	!,
	% then
	dm(intOrders, ['It is not ordered. (This was already stored.)\n\n']),
	true.

assertintOrderResNo(ResNo, noOrder) :-
	% if the given attribute / tuple-stream is not ordered
	!,
	% then
	asserta(intOrder(res(ResNo), noOrder)),
	dm(intOrders, ['It is not ordered.\n\n']),
	true.

assertintOrderResNo(ResNo, ArgAttribute) :-
	% if an order-fact is already stored
	dc(intOrders, intOrder(res(ResNo), ArgAttributeOld)),
	retract(intOrder(res(ResNo), _)),
	!,
	% then
	assertintOrderResNo(ResNo, ArgAttribute),
	dm(intOrders, ['(A previously for res(', ResNo, 
                       ') stored order was retracted: ', 
                       ArgAttributeOld, '.)\n\n']),
	true.

assertintOrderResNo(ResNo, ArgAttribute) :-
	% else
	asserta(intOrder(res(ResNo), ArgAttribute)),
	dm(intOrders, ['It is ordered by: ', ArgAttribute, '\n\n']),
	true.

assertintOrderResNo(ResNo, ArgAttributeIn1, ArgAttributeIn2, Pleft, Pright) :-
	% if an order-fact is already stored
	dc(intOrders, intOrder(res(ResNo), ArgAttributeOld)),
	retract(intOrder(res(ResNo), _)),
	!,
	% then
	assertintOrderResNo(ResNo, ArgAttributeIn1, ArgAttributeIn2,
			    Pleft, Pright),
	dm(intOrders, ['(A previously for res(', ResNo,
	               ') stored order was retracted: ',
	               ArgAttributeOld, '.)\n\n']),
	true.

assertintOrderResNo(ResNo, noOrder, noOrder, _, _) :-
	% if the attributes / tuple-streams are not ordered
	!,
	% then
	asserta(intOrder(res(ResNo), noOrder)),
	dm(intOrders, ['$$ It is not ordered, because the tuple-streams are ',
                       ' not ordered.\n\n']),
	%!, skip(10),
	true.

assertintOrderResNo(ResNo, ArgAttributeX, ArgAttributeY, Pleft, Pright) :-
	% if the join-predicates can be analysed
	checkAttrAndPredPart(ArgAttributeX, Pleft, LeftEquals),
	checkAttrAndPredPart(ArgAttributeY, Pright, RightEquals),
	!,
	% then
	asserta(intOrder(res(ResNo),
			twoArgs(ArgAttributeX, ArgAttributeY,
				LeftEquals, RightEquals))),
	writeAttrAndPredPartMessage(ArgAttributeX, ArgAttributeY,
				    LeftEquals, RightEquals),
	asserta(orderBackup(ResNo, ArgAttributeX, ArgAttributeY,
			    Pleft, Pright)).

/*

The ~orderBackup~-fact is needed, if ~correctResultOrder~ corrects the order of a certain result. In such a case the original result-order has to be determined by using the ~orderBackup~-facts.

*/

assertintOrderResNo(ResNo, _, _, Pleft, Pright) :-
	% else
	asserta(intOrder(res(ResNo),
		twoArgs(noOrder, noOrder, notKnown, notKnown))),
	dm(intOrders, ['## It is not ordered, because at least one of the ',
                       'predicate-parts\n', Pleft, ' or ', Pright,
                       ' can''t sufficiently be analysed', '. \n\n']),
	%!, skip(10),
	doNothingWith(Pleft, Pright).

doNothingWith(_, _).

/*

The last clause ~doNothingWith~ is only needed to avoid a warning for singleton variables if all the write-goals are commented out, when ~intOrders.pl~ is consulted.

For to decide what order the result of a mergejoin respectively a sortLeftThenMergejoin or a sortRightThenMergejoin has, it is necessary to compare the orders of the tuple-streams, that shall be joined, (their order-attributes) with the join-attributes. The join-attributes are just attribute-names, if no variable for the relations is used in the query, but they are of the form ~variable:attribute~, if a variable is used. Hence the following clauses catch the four possible cases that have the shape of the following examples: ~sName = p:ort~, ~s:sName = ort~, ~s:sName = p:ort~ and ~sName = ort~.

*/

checkAttrAndPredPart(ArgAttribute,
		attr(ArgAttribute, _, _), joinAttr) :-
	% if equals
	!.

checkAttrAndPredPart(ArgAttribute,
		attr((_ : ArgAttribute), _, _), joinAttr) :-
	% if equals
	!.

/*

If two tuple-stream are joined by using the mergejoin-operator, for the result the order of the left tuple-stream is stored. If the join is for example ~sName = p:ort~, the order ~sName~ is stored into the knowledge-base. If this result shall be joined with another tuple-stream and the join-predicate is say ~p:ort=q:ort~, then the information is important, that ~sName~ and ~ort~ are equivalent in the sense, that ~ort~ could have been stored as the result-order of the first join as well. Therefore ~equalOrder~-facts exist.

It is enough to check for just one ~equalOrder~-fact for every given attribute. 'Fact-series' like ~equalOrder(a, b)~, ~equalOrder(b, c)~ and ~equalOrder(c, d)~ are not possible, because always the order-attribute of the left tuple-stream is stored as the result order of a mergejoin. And if a given query contains something like ~s1:sName=p:ort~ as well as ~p:ort=s2:sName~, then inevitably both facts ~equalOrder(sName, ort)~ and ~equalOrder(ort, sName)~ are stored into the knowledge-base.

*/

checkAttrAndPredPart(ArgAttribute,
		attr(Ppart, _, _), joinAttr) :-
	% if equals
	equalOrder(ArgAttribute, Ppart),
	!.

checkAttrAndPredPart(ArgAttribute,
		attr((_ : Ppart), _, _), joinAttr) :-
	% if equals
	equalOrder(ArgAttribute, Ppart),
	!.

/*

The following clauses are needed for to find out, if only one of the two tuple-streams, that are to be joined, is in the needed order and the other is not. In such a case a ~sortLeftThenMergejoin~ or a ~sortRightThenMergejoin~ can be the appropriate join-implementation.

*/

checkAttrAndPredPart(_, attr(_, _, _), otherAttr).

checkAttrAndPredPart(_, attr((_ : _), _, _), otherAttr).

/*

Join-predicates can contain arithmetic operators. Some examples are:

----
p1:plz = p2:plz + 1
p2:plz = p3:plz * 5
(p1:plz +1) = p2:plz
----

Those are caught by the following clauses.

*/

checkAttrAndPredPart(ArgAttribute, (Ppart + _), Result) :-
	checkAttrAndPredPart(ArgAttribute, Ppart, Result).

checkAttrAndPredPart(ArgAttribute, (Ppart - _), Result) :-
	checkAttrAndPredPart(ArgAttribute, Ppart, Result).

checkAttrAndPredPart(ArgAttribute, (Ppart * _), Result) :-
	checkAttrAndPredPart(ArgAttribute, Ppart, Result).

checkAttrAndPredPart(ArgAttribute, (Ppart / _), Result) :-
	checkAttrAndPredPart(ArgAttribute, Ppart, Result).

checkAttrAndPredPart(ArgAttribute, (Ppart // _), Result) :-
	checkAttrAndPredPart(ArgAttribute, Ppart, Result).

checkAttrAndPredPart(ArgAttribute, (Ppart mod _), Result) :-
	checkAttrAndPredPart(ArgAttribute, Ppart, Result).

/*

Unfortunately a similar clause is needed for every possibly used arithmetic operator. Other implementations for the operator-check were tried. The following implementation for example causes a 'Syntax error: Operator expected'

----
checkAttrAndPredPart(ArgAttribute,
	(attr(ArgAttribute, _, _) Operator _), joinAttr) :-
	isMathOperator(Operator).
----

And that one causes a 'Type error: atom expected, found attr(p2:pLZ, 2, u)+1'

----

checkAttrAndPredPart(ArgAttribute, Expression, joinAttr) :-
	parseExpression(Expression, ArgAttribute).

parseExpression(Expression, ArgAttribute) :-
	atom_chars(Expression, ExprList),
	filterExprAttribute(ExprList, ExprAttrList),
	atom_chars(ExprAttrList, ArgAttribute).
----

The following clauses are primarily needed, if one want's to know, how the optimizer in detail works.

*/

writeAttrAndPredPartMessage(ArgAttributeX, ArgAttributeY,
			    joinAttr, joinAttr) :-
	% if
	!,
	dm(intOrders, ['++ It is ordered, because the join is done on ',
                       ArgAttributeX, ' and ', ArgAttributeY, '.\n\n']),
	%!, skip(10),
	doNothingWith(ArgAttributeX, ArgAttributeY).

writeAttrAndPredPartMessage(ArgAttributeX, _, joinAttr, _) :-
	% if
	!,
	dm(intOrders, ['== It could be ordered as ', 
                       ArgAttributeX, ' if a sort-operator is used on ',
                       'the right tuple-stream.\n\n']),
	%!, skip(10),
	doNothingWith(ArgAttributeX, _).

writeAttrAndPredPartMessage(_, ArgAttributeY, _, joinAttr) :-
	% if
	!,
	dc(intOrders, ['oo It could be ordered as ',
                       ArgAttributeY, ' if a sort-operator is used on ',
                       'the left tuple-stream.\n\n']),
	%!, skip(10),
	doNothingWith(_, ArgAttributeY).

writeAttrAndPredPartMessage(noOrder, ArgAttributeY, _, _) :-
	% if
	!,
	dm(intOrders, ['** It is not ordered, because the left tuple-stream is \n', 
                       '   not ordered and the right join-attribute is not ',
                       ArgAttributeY]),
        writeAttrAndPredPartMessage2(ArgAttributeY),
	dm(intOrders, ['.\n\n']),
	%!, skip(10),
	true.

writeAttrAndPredPartMessage(ArgAttributeX, noOrder, _, _) :-
	% if
	!,
        dm(intOrders,['&& It is not ordered, because the right tuple-stream is',
                      '\n   not ordered and the left join-attribute is not ',
                      ArgAttributeX]),
	writeAttrAndPredPartMessage2(ArgAttributeX),
	dm(intOrders, ['.\n\n']),
	%!, skip(10),
	true.

writeAttrAndPredPartMessage(ArgAttributeX, ArgAttributeY,
			    otherAttr, otherAttr) :-
	% if
	!,
	dm(intOrders, ['~~ It is not ordered, because the join is not done on ',
                       ArgAttributeX]),
	writeAttrAndPredPartMessage2(ArgAttributeX),
	dm(intOrders, [' and ', ArgAttributeY]),
	writeAttrAndPredPartMessage2(ArgAttributeY),
	dm(intOrders, ['.\n\n']),
	%!, skip(10),
	true.

writeAttrAndPredPartMessage(_, _, _, _) :-
	write('-- ERROR -- '),
	nl, nl,
	skip(10).

/*

The goal ~doNothingWith~ is only needed to avoid a warning for singleton variables if all the write-goals are commented out, when ~intOrders.pl~ is consulted.

If the attributes of an equi-join are for example ~staedte:sName~ and ~plz:ort~, as the order of the result ~sName~ is stored into the knowledge-base (for the case, that a mergejoin will later be chosen as the appropriate join-implementation). That's the order-attribute of the left tuple-stream. Additionally it is stored, that ~sName~ and ~ort~ are equivalent in the sense, that ~ort~ could have been stored as the result-order as well. Therefore ~equalOrder~-facts are used.

But this kind of information is senseless, if the join-predicate is for example ~p1:ort=p2:ort~. Hence the ~equalOrder~-fact, that says that ~ort~ and ~ort~ are equivalent, is retracted at the first opportunity (which is the next call of ~writeAttrAndPredPartMessage2~).

*/

writeAttrAndPredPartMessage2(ArgAttribute) :-
	% if
	retract(equalOrder(ArgAttribute, ArgAttribute)),
	!.

writeAttrAndPredPartMessage2(ArgAttribute) :-
	% if
	equalOrder(ArgAttribute, EqualOrder),
	!,
	% then
	dm(intOrders, ['\n   (or an to ', ArgAttribute, ' equal order like ',
	               EqualOrder, ')']),
	doNothingWith(ArgAttribute, EqualOrder).

writeAttrAndPredPartMessage2(_).
	% else no equal ordered attribute is in the knowledge-base

/*

The goal ~doNothingWith~ is only needed to avoid a warning for singleton variables if all the write-goals are commented out, when ~intOrders.pl~ is consulted.

4.3.2 Determining the Orders of Intermediate Results

The following clauses ~setResultOrder~ and ~correctResultOrder~ are called in ~checkSuccessor~ (if the program-mode ~intOrders(quick)~ or ~intOrders(test)~ are used, in the other modes these clauses are used in a slightly other way) to first determine, if it is acutally possible to create mergejoin-plan-edges (or sortLeftThenMergejoin- or sortRightThenMergejoin-plan-edges) from a node to its successor (dependent on the arguments orders and the join-attributes), second, what the order of the respective result will be, if the mergejoin-operator (or the sortLeftThenMergejoin- or the sortRightThenMergejoin-operator) will later be chosen as the appropriate join-implementation, and third, what the order of the respective result actually is, after the cheapest cost-edge from the node to its successor is chosen. (The latter again only goes for the program-modes ~intOrders(quick)~ respectively ~intOrders(test)~. In the other program-modes no cheapest cost-edge is determined.)

Actually the order of the result is at first set to that order, that the result will have, if a mergejoin-plan-edge respectively -cost-edge (or a sortLeftThenMergejoin- or sortRightThenMergejoin-plan-edge) will be the cheapest one from the relevant node to its successor. That is done with the call of ~setResultOrder~. Later with the call of ~correctResultOrder~ the results order is corrected, if necessary.

Please note, that ~setResultOrder~ and ~correctResultOrder~ are even called, if a plan-edge realizing the mergejoin-operator is completely irrelevant - because (for example) the respective egde doesn't even represent a join or it does, but the join is no equi-join. In that case the results order has already been ascertained in advance and the calls are caught by clauses, that just return a 'true'.

Because the original version of ~checkSuccessor~ in chapter 2.5.6 'Creating Edges to Successors' of ~modifications.pl~ does not contain the goals ~setResultOrder~ and ~correctResultOrder~, it must be modified. But instead of editing the file ~modifications.pl~ manually, the original static procedure ~checkSuccessor~ is defined as dynamic, the existing clauses are retracted from the knowledge-base and new clauses are asserted.

*/



assertCheckSuccessorClauses0 :-
        retractCheckSuccessorClauses,
	assertz(checkSuccessor(ReachedNodesSet, ReachedNodesSet, _, _, 
		succ(SuccNodeNo, _, _, _)) :-
	(isTickedOff(SuccNodeNo),
	 !)
	),
	assertz(checkSuccessor(ReachedNodesSet, NewReachedNodesSet, 
               node(NodeNo, NodePreds, Partition), NDPNode, 
               succ(SuccNodeNo, _, PredNo, Predicate)) :-
	(isInReachedNodesSet(ReachedNodesSet, node(SuccNodeNo, _, _)),
	 !,
	 createEdge(NodeNo, SuccNodeNo,
	 	   node(NodeNo, NodePreds, Partition),
                   PredNo, Predicate, Edge),
	 setResultOrder(Edge),
	 createPlanEdges(Edge, PlanEdges),
	 assignSize(Edge),
	 cheapestCostEdge(PlanEdges, CostEdge),
	 correctResultOrder(CostEdge),
	 correctDistanceAndPath(NDPNode, SuccNodeNo, CostEdge,
                               ReachedNodesSet, NewReachedNodesSet)
	 )
	),
	assertz(checkSuccessor(ReachedNodesSet, NewReachedNodesSet, 
               node(NodeNo, NodePreds, Partition), NDPNode, 
               succ(SuccNodeNo, _, PredNo, Predicate)) :-
	(createEdge(NodeNo, SuccNodeNo,
		   node(NodeNo, NodePreds, Partition), 
                   PredNo, Predicate, Edge),
	setResultOrder(Edge),
	createPlanEdges(Edge, PlanEdges),
	assignSize(Edge),
	cheapestCostEdge(PlanEdges, CostEdge),
	correctResultOrder(CostEdge),
	createNDPSuccessor(succ(SuccNodeNo, _, _, _), NDPSuccessor),
	setDistanceAndPath(NDPNode, NDPSuccessor, CostEdge),
	putIntoReachedNodesSet(ReachedNodesSet, NDPSuccessor,
                               NewReachedNodesSet)
	 )
	).

setResultOrder(edge(_, _, _, Result, _, _)) :-
	% if both arguments do not equal join-attributes
	retract(intOrder(res(Result), twoArgs(_, _, otherAttr, otherAttr))),
	!,
	% then
	asserta(intOrder(res(Result), noOrder)).

setResultOrder(edge(_, _, _, Result, _, _)) :-
	% if both arguments equal join-attributes
	retract(intOrder(res(Result), twoArgs(A, B, joinAttr, joinAttr))),
	!,
	% then
	assert(tempOrderNote(mergejoinPossible)),
	dm(intOrders, ['For to generate res(', Result, ') ',
                       'a mergejoin-operator can be used.\n']),
	asserta(intOrder(res(Result), A)),
	assert(equalOrder(A, B)),
	dm(intOrders, ['Then it would be ordered by: ', A, '\n\n']),
	true.

setResultOrder(edge(_, _, _, Result, _, _)) :-
	% if left argument equals join-attribute
	retract(intOrder(res(Result), twoArgs(A, _, joinAttr, otherAttr))),
	!,
	% then
	assert(tempOrderNote(sortRightThenMergejoin)),
	dm(intOrders,['For to generate res(', Result, ') a sort-operator right ',
                      'and then a mergejoin-operator can be used.\n']),
	asserta(intOrder(res(Result), A)),
	dm(intOrders,['Then it would be ordered by: ', A, '\n\n']),
	true.

setResultOrder(edge(_, _, _, Result, _, _)) :-
	% if right argument equals join-attribute
	retract(intOrder(res(Result), twoArgs(_, B, otherAttr, joinAttr))),
	!,
	% then
	assert(tempOrderNote(sortLeftThenMergejoin)),
	dm(intOrders,['For to generate res(', Result, ') a sort-operator left ',
	              'and then a mergejoin-operator can be used.\n']),
	asserta(intOrder(res(Result), B)),
	dm(intOrders,['Then it would be ordered by: ', B, '\n\n']),
	true.

/*

The next two clauses are relevant, if (at least) one tuple-stream is not ordered and the others order-attribute does not equal the respective join-attribute.

*/

setResultOrder(edge(_, _, _, Result, _, _)) :-
	% if at least the left stream is not ordered
	% and the right is not the join-Attribute
	retract(intOrder(res(Result), twoArgs(noOrder, _, _, _))),
	!,
	% then
	asserta(intOrder(res(Result), noOrder)).
	
setResultOrder(edge(_, _, _, Result, _, _)) :-
	% if the right stream is not ordered
	% and the left is not the join-Attribute
	retract(intOrder(res(Result), twoArgs(_, noOrder, _, _))),
	!,
	% then
	asserta(intOrder(res(Result), noOrder)).

setResultOrder(_) :-
	true.
	% if the second element of intOrder/2 is no twoArgs/4-element

/*

The loopjoin passes on the order of that one of the two given relations, that is used as the so-called 'outer relation' by the loopjoin-operator. In SECONDO the order of the result of a loopjoin is the order of the left tuple-stream. Hence if the order of an intermediate result is corrected into the result-order of the loopjoin-operator, the order of the left tuple-stream is stored as the result-order.

The order of the result of a sortmergejoin is determined by the join-predicate, but it is not relevant, if the left or the right join-attribute ist chosen for to determine the results order. Hence if the order of an intermediate result is corrected into the result-order of the sortmergejoin-operator, the left join-attribute is stored as the result-order. (The right join-attribute could be stored as well.) To determine the left join-attribute the join-predicate has to be parsed equivalent to the parsing taking place in ~checkAttrAndPredPart~ (see chapter 4.3.1).

The result of a hashjoin-operator and of the carthesian product has no intersting order.

*/

correctResultOrder(costEdge(_, _, remove(Join, _), Result, _, _)) :-
	% if
	!,
	% then
	correctResultOrder(costEdge(_, _, Join, Result, _, _)).

correctResultOrder(costEdge(_, _, loopjoin(_, _),
		   Result, _, _)) :-
	% if
	!,
	% then
	retract(intOrder(res(Result), _)),
	orderBackup(Result, LeftStreamAttr, _, _, _),
	asserta(intOrder(res(Result), LeftStreamAttr)),
	dm(intOrders,['Order of res(', Result, ') is corrected.\n',
	              'The respective cost-edge uses the loopjoin.\n\n']),
	true.

correctResultOrder(costEdge(_, _, sortmergejoin(_, _, _, _),
		   Result, _, _)) :-
	% if
	!,
	% then
	retract(intOrder(res(Result), _)),
	orderBackup(Result, _, _, Pleft, _),
	getSortmergeOrder(Pleft, Order),
	asserta(intOrder(res(Result), Order)),
	dm(intOrders,['Order of res(', Result, ') is corrected.\n', 
	              'The respective cost-edge uses the sortmergejoin.\n\n']),
	true.

correctResultOrder(costEdge(_, _, hashjoin(_, _, _, _, _),
		   Result, _, _)) :-
	% if
	!,
	% then
	retract(intOrder(res(Result), _)),
	asserta(intOrder(res(Result), noOrder)),
	dm(intOrders,['Order of res(', Result, ') is corrected.\n',
	              'The respective cost-edge uses the hashjoin.\n\n']),
	true.

correctResultOrder(costEdge(_, _, filter(product(_, _), _),
		   Result, _, _)) :-
	% if
	!,
	% then
	retract(intOrder(res(Result), _)),
	asserta(intOrder(res(Result), noOrder)),
	dm(intOrders,['Order of res(', Result, ') is corrected.\n',
	              'The respective cost-edge uses the carthesian product.\n\n']),
	true.

correctResultOrder(_) :-
	% else
	true.

getSortmergeOrder(attr((_ : Order), _, _), Order) :-
	!.

getSortmergeOrder(attr(Order, _, _), Order) :-
	!.

getSortmergeOrder((Ppart + _), Order) :-
	getSortmergeOrder(Ppart, Order).

getSortmergeOrder((Ppart - _), Order) :-
	getSortmergeOrder(Ppart, Order).

getSortmergeOrder((Ppart * _), Order) :-
	getSortmergeOrder(Ppart, Order).

getSortmergeOrder((Ppart / _), Order) :-
	getSortmergeOrder(Ppart, Order).

getSortmergeOrder((Ppart // _), Order) :-
	getSortmergeOrder(Ppart, Order).

getSortmergeOrder((Ppart mod _), Order) :-
	getSortmergeOrder(Ppart, Order).

getSortmergeOrder(_, noOrder).

/*

4.4 Listing All Argument- and Result-Orders

Please note, that in the program-modes ~intOrders(path)~ and ~intOrders(on)~ more than just one order for every intermediate result can be given out, because more than just one variant of the POG-nodes are considered during the construction of the shortest path. (More information contain the following chapters.)

*/

writeOrders :-
	write('The resulting relation is ordered by: '),
	intOrder(res(_), Order),
	write(Order), nl,
	nl, write('The orders of the base arguments are:'), nl,
	nl, not(writeArgOrder),
	nl, write('The orders of intermediate results are:'), nl,
	nl, not(writeResultOrder).

writeArgOrder :-
	intOrder(arg(N), Attr),
	write('argument arg('), write(N),
	write(') : '), write(Attr), nl,
	fail.

writeResultOrder :-
	intOrder(res(N), Attr),
	write('intermediate result res('), write(N),
	write(') : '), write(Attr), nl,
	fail.

/*

[newpage]

5 The Merge-Join and its Variants

5.1 Translating Joins

Some clauses of the original optimizers code contained in the file ~optimizer.pl~ have to be modified, if interesting orders shall be considered. This has already been explained. Further some clauses must be added. One can manually modify that file. In that case it is necessary to first add such clauses into chapter 5.2.3 'Translation of Joins' of ~optimizer.pl~, that translate joins using the mergejoin-operator (respectively the sortLeftThenMergejoin- or sortRightThenMergejoin-operator), and second into chapter 5.1.3 'Converting Plans to Atoms and Writing them', that convert these operators into antoms.

Please note that those changes of the original optimizers code won't affect its processing. The clauses only become relevant, if ~intOrders(X)~ respectively ~intOrders(X, time)~ will be called beforehand and X unifies with ~quick~, ~path~, ~on~ or ~test~.

Another way to add clauses to the PROLOG-interpreters knowledge-base, is to define the relevant hitherto static procedures as dynamic and then assert the clauses into the knowledge-base. This is done in the following.

Please note, that both ways to enhance the optimizers set of clauses are equivalent.

May 2006, Christian D[ue]ntgen: the extensions to plan_to_atom/2 have been integrated into file ``optimizer.pl''.

*/

/*

Please note, that the SECONDO-kernel in fact knows the ~sortby~-operator, but this operator is not expected to be found in the middle of some plan - at least at the moment. Hence a plan, that contains the above translation of a sortLeftThenMergejoin- respectively sortRightThenMergejoin-plan-edge cannot be processed by the SECONDO-kernel.

Outputs of the above clauses for the following example-calls:

----
plan_to_atom(sortLeftThenMergejoin(rename(feed(rel(plz, p, l)), p),
rename(feed(rel(staedte, s, u)), s), attrname(attr(p:ort, 1, u)),
attrname(attr(s:sName, 2, u))), Result).

plan_to_atom(sortRightThenMergejoin(rename(feed(rel(plz, p, l)), p),
rename(feed(rel(staedte, s, u)), s), attrname(attr(p:ort, 1, u)),
attrname(attr(s:sName, 2, u))), Result).
----

are:

----
Result = 'plz  feed {p} sortby[Ort_p asc] Staedte
feed {s} mergejoin[Ort_p, SName_s]'

Result = 'plz  feed {p} Staedte  feed {s}
sortby[SName_s asc] mergejoin[Ort_p, SName_s]'
----

And the commands:

----
secondo('query plz feed {p} sortby[Ort_p asc]
	 Staedte feed {s} mergejoin[Ort_p, SName_s]')

secondo('query plz feed {p} Staedte  feed {s}
	 sortby[SName_s asc] mergejoin[Ort_p, SName_s]').
----

cause error-messages as expected.

Dependent on the future changes of the SECONDO-kernel, that enhance it to process translations of sortLeftThenMergejoin- respectively sortRightThenMergejoin-plan-edges, may require other translations than the above. Maybe instead of the above translations something like:

----
concat_atom([XAtom, YAtom, 'sortleftmergejoin[',
    AAtom, ', ', BAtom, '] '], '', Result),

concat_atom([XAtom, YAtom, 'sortrightmergejoin[',
    AAtom, ', ', BAtom, '] '], '', Result),
----

will be appropriate.

5.2 Costs

To determine all relevant costs, again some clauses must be added to ~optimizer.pl~. 

(May 2006, Christian D[ue]ntgen: The cost functions for mergejoin, sortLeftThenMergejoin and sortRightThenMergejoin have been moved to the file 
``optimizer.pl''.)

*/


/*

5.3 Deciding, what Join-Implementation can be Used

For to decide, if a join-implementation, that needs ordered tuple-streams, can be used or not, the clause ~orderTest~ is called with the argument, what implementation shall be tested. ~orderTest~ just checks, if an appropriate fact has been stored into the PROLOG-interpreters knowledge-base, and retracts it or fails.

*/

:- dynamic(orderTest/1). % allow modifications by intOrders extension

orderTest(_) :- fail.

createMergeJoinPlanEdges :-
	retract(orderTest(_) :- (_)),
	asserta(orderTest(A) :- (doOrderTest(A))).

doNotCreateMergeJoinPlanEdges :-
	retract(orderTest(_) :- (_)),
	asserta(orderTest(_) :- (fail)).

doOrderTest(mergejoinPossible) :-
	% if mergejoin is possible
	retract(tempOrderNote(mergejoinPossible)),
	!,
	dm(intOrders,['Plan-edge with mergejoin-operator created.\n\n']),
	%!, skip(10),
	true.

doOrderTest(sortLeftThenMergejoin) :-
	% if sort-operator right plus mergejoin is possible
	retract(tempOrderNote(sortLeftThenMergejoin)),
	!,
	dm(intOrders,['Plan-edge with first order left stream ',
	              'and then mergejoin-operator created.\n\n']),
	%!, skip(10),
	true.

doOrderTest(sortRightThenMergejoin) :-
	% if sort-operator right plus mergejoin is possible
	retract(tempOrderNote(sortRightThenMergejoin)),
	!,
	dm(intOrders,['Plan-edge with first order right stream ',
	              'and then mergejoin-operator created.\n\n']),
	%!, skip(10),
	true.

/*

[newpage]

6 Using Other than the POG-Nodes for the Shortest-Path-Construction

Please note, that the program-mode ~intOrders(quick)~ only calls clauses defined in the above chapters and works without those of the following chapters.

6.1 Using new Node-Sets-Implementations

The original SECONDO-optimizer uses the algorithm of dijkstra to search the shortest path through a formerly constructed so-called predicate order graph (POG). Its modification realized in ~modifications.pl~ also uses a variant of this algorithm for the immediate construction of that shortest path. (For details see chapter 2.1 of ~modifications.pl~.)

The following clause changes the implementation of the set for the already reached respectively created but not yet ticked off nodes, that is used by the algorithm of dijkstra, as well as the implementation of the set of already ticked off nodes.

*/

correctStoredNodesShape :-
  intOrdersRNSImplementation,
  retractCurrentTOSImplementation,
  intOrdersTOSImplementation.

restoreStoredNodesShape :-
  restoreRNSImplementation.
%  restoreTOSImplementation

/*

6.2 A Reached-Nodes-Set-Implementation, that Stores Interesting Orders

A decisive element of the algorithm of dijkstra is a set containing the already created but not yet ticked off nodes (called REACHED-NODES in ~modifications.pl~). Originally the nodes of the predicate order graph (POG) are more or less directly put into that set, thus the shortest path-search respectively -construction bases on those nodes.

In the current context (to be correct: in the program-modes ~intOrders(path)~ and ~intOrders(on)~) more than just the nodes of the POG have to be considered. Hence the implementation of the set REACHED-NODES is changed. It cannot contain the POG-nodes any more, but must contain 'enhanced' nodes. One interesting point should be the enumeration of these nodes: They are just numbered in the order, in which they are created; for every new node, a numerator is incremented. It is not absolutely necessary, to store an unambiguous number for every node, but it was useful in early program-tests. The other enhancement beside the enumeration of the stored nodes is of course the order-information for every result, that belongs to a stored node.

It must be said, that the following implementation of the set REACHED-NODES is not the most efficient of all possible implementations. An implementation equivalent to the one contained in the file ~boundary.pl~ in the ~Optimizer~-directory of the SECONDO-system would be more efficient. But at the moment it is more important to show the principles and ideas of the optimizers modifications and extensions and to describe these in clarity, than to realize the fastest of all thinkable programs.

*/

intOrdersRNSImplementation :-
      retractCurrentRNSImplementation,
      asserta(createEmptyReachedNodesSet([]) :-
	      (createEmptyReachedNodesSetIO([]))
             ),
      asserta(putIntoReachedNodesSet(ReachedNodesSet,
			  node(NodeNo, Distance, Path),
                          NewReachedNodesSet) :-
	      (putIntoReachedNodesSetIO(ReachedNodesSet,
			  node(NodeNo, Distance, Path),
			  intOrder(res(0), noOrder),
                          NewReachedNodesSet))
	       % This clause is only called once - when the source-
	       % node is inserted (see modifications.pl).
             ),
      asserta(isInReachedNodesSet(ReachedNodesSet, Node) :-
              (isInReachedNodesSetIO(ReachedNodesSet, Node))
             ),
      asserta(readAlreadyReachedNode(ReachedNodesSet, NodeNo,
			  node(NodeNo, Distance, Path)) :-
	      (readAlreadyReachedNodeIO(ReachedNodesSet, NodeNo,
			  node(NodeNo, Distance, Path)))
             ),
      asserta(getMinimalDistantNode(ReachedNodesSet,
		      	  node(NodeNo, Distance, Path),
		      	  NewReachedNodesSet) :-
	      (getMinimalDistantNodeIO(ReachedNodesSet,
		      	  node(NodeNo, Distance, Path),
		      	  NewReachedNodesSet))
             ),
      asserta(deleteOutOfReachedNodesSet(ReachedNodesSet,
			  node(NodeNo, _, _),
                          NewReachedNodesSet) :-
	      (deleteOutOfReachedNodesSetIO(ReachedNodesSet, 
			  node(NodeNo, _, _),
                          NewReachedNodesSet))
             ),
      asserta(isEmpty([]) :-
	      (isEmptyIO([]))
             ).

createEmptyReachedNodesSetIO([]) :-
	deleteLastNodeNos,
	assert(lastNodeNo(-1)),
	true.

:- dynamic putIntoReachedNodesSetIO/4. % enable modifications by intOrder

restorePutIntoReachedNodesSetIOClauses :-
  retractall(putIntoReachedNodesSetIO(_, _, _, _) :- (_)),
  assert( putIntoReachedNodesSetIO(ReachedNodesSet,
                                   node(POGNodeNo, Distance, Path),
                                   Order, NewReachedNodesSet) :-
          ( retract(lastNodeNo(LastNodeNo)),
            NewNodeNo is LastNodeNo + 1,
            assert(lastNodeNo(NewNodeNo)),
            putIntoNodeListIO(ReachedNodesSet,
                              node(NewNodeNo, POGNodeNo, Distance, Path, Order),
                              NewReachedNodesSet)
          )
        ).

:- restorePutIntoReachedNodesSetIOClauses.

isInReachedNodesSetIO(ReachedNodesSet,
                      node(POGNodeNo, Distance, Path)) :-
	member(node(_, POGNodeNo, Distance, Path, _), ReachedNodesSet).

readAlreadyReachedNodeIO(ReachedNodesSet, POGNodeNo,
			  node(POGNodeNo, Distance, Path)) :-
	member(node(_, POGNodeNo, Distance, Path, _), ReachedNodesSet),
	true.

getMinimalDistantNodeIO(ReachedNodesSet, node(POGNodeNo, Distance, Path),
		      NewReachedNodesSet) :-
	deleteFirstOfNodeListIO(ReachedNodesSet,
			      node(NodeNo, POGNodeNo, Distance, Path, Order),
			      NewReachedNodesSet),
	assertz(minDistNodeOrder(NodeNo, POGNodeNo, Order)),
	correctResultIntOrders.

correctResultIntOrders :-
	not(correctResultIntOrder).

correctResultIntOrder :-
	% if a minDistNodeOrder-fact exists
	minDistNodeOrder(_, _, intOrder(res(Result), Order)),
	% then the current intOrder-fact is corrected
	retractAllIntOrders(Result),
	assertz(intOrder(res(Result), Order)),
	fail.
	% else the current intOrder-fact is not changed

retractAllIntOrders(Result) :-
	not(retractAllIntOrder(Result)).

retractAllIntOrder(Result) :-
	retract(intOrder(res(Result), _)),
	fail.

deleteOutOfReachedNodesSetIO(ReachedNodesSet, node(POGNodeNo, _, _),
                             NewReachedNodesSet) :-
	deleteOutOfNodeListIO(ReachedNodesSet, node(_, POGNodeNo, _, _, _),
			  NewReachedNodesSet).

isEmptyIO([]) :-
	true.

/*

The suffix 'IO' is an abbreviation of 'interesting orders'.

The most interesting of the above clauses is ~getMinimalDistantNodeIO~ that contains ~correctResultIntOrders~ as its last goal. The aim of that goal is, that every time, when a new node with the minimal distance to the source-node of the POG (in comparison to all the other nodes contained in the set of all hitherto created but no yet ticked off nodes) is determined, all relevant interesting orders, that are currently stored in the PROLOG-interpreters knowledge-base, are corrected, so that no order, that was determined after the respective node was stored into that set, can influence the determination of the orders of the results, that belong to the mininal distance nodes successors.

Please note, that the clauses ~isInReachedNodesSetIO~, ~readAlreadyReachedNodeIO~ and ~deleteOutOfReachedNodesSetIO~ are only used in the program-mode ~intOrders(test)~ respectively ~intOrders(test, time)~. Thereore the order-information can be ignored within these clauses.

*/

putIntoNodeListIO([], node(NodeNo, POGNodeNo, Distance, Path, Order),
		  [node(NodeNo, POGNodeNo, Distance, Path, Order)]) :-
	true.

putIntoNodeListIO(InList, node(NodeNo, POGNodeNo, Distance, Path, Order),
		  OutList) :-
	InList = [node(_, _, FirstDistance, _, _)|_],
	% if
	Distance < FirstDistance,
	!,
	% then
	OutList = [node(NodeNo, POGNodeNo, Distance, Path, Order)|InList],
	true.
	
putIntoNodeListIO([Element|InListRemainder], 
		node(NodeNo, POGNodeNo, Distance, Path, Order),
		[Element|OutListRemainder]) :-
	% else
	putIntoNodeListIO(InListRemainder,
			  node(NodeNo, POGNodeNo, Distance, Path, Order),
			  OutListRemainder).

deleteFirstOfNodeListIO([node(NodeNo, POGNodeNo, Distance, Path, Order)
			|ListRemainder],
			node(NodeNo, POGNodeNo, Distance, Path, Order),
			ListRemainder) :-
	true.

deleteOutOfNodeListIO([], _, []).

deleteOutOfNodeListIO([node(NodeNo, POGNodeNo, Distance, Path, Order)
		      |InListRemainder],
		      node(NodeNo, POGNodeNo, Distance, Path, Order),
		      InListRemainder) :-
	!,
	true.

deleteOutOfNodeListIO([OtherElement|InListRemainder],
			Element, [OtherElement|OutListRemainder]) :-
	deleteOutOfNodeListIO(InListRemainder,
			    Element, OutListRemainder).

deleteLastNodeNos :-
	not(deleteLastNodeNumbers).

deleteLastNodeNumbers :-
	retract(lastNodeNo(_)),
	fail.

/*

6.3 A Ticked-Off-Nodes-Set-Implementation, that Stores Interesting Orders

*/

intOrdersTOSImplementation :-
  asserta(emptyTickedOffSet :- ( retractall(tickOffNode(_, _, _, _)) ) ),
  asserta( tickOff(node(POGNodeNo, Distance, Path)) :-
           ( minDistNodeOrder(NodeNo, POGNodeNo, Order),
             assert(tickOffNode(NodeNo, POGNodeNo,
			    node(POGNodeNo, Distance, Path), Order)),
             dm(intOrders,['Node ', POGNodeNo, ' with its result ordered by ']),
             dc(intOrders,(Order = intOrder(_, ActualOrder), write(ActualOrder),
	                   write(' is ticked off.'), nl, nl )),
             true
           )
	),
  asserta( isTickedOff(pair(POGNodeNo, Order)) :- 
           ( tickOffNode(_, POGNodeNo, _, Order) )
	 ),
  asserta( getTickedOffNode(POGNodeNo, Node) :-
           ( tickOffNode(_, POGNodeNo, Node, _) )
         ).


/*

~isTickedOff~ has only one argument of the shape ~pair~/2 instead of two single arguments to ascertain interface-conformity with the formerly defined and used clause ~isTickedOff~ (see ~modification.pl~, chapter 2.5.14 'Ticking Off Nodes - The Interface').

~getTickedOffNode~ is only used once to get information about the destination-node at the end of the shortest-path-construction. Hence the stored order-information can be ignored, because the here used modification of the algorithm of dijkstra stops as soon as the destination-node is determined for the first time as the node with the minimal distance to the source-node in comparison to all other nodes currently stored in the set of all hitherto created but not yet ticked off nodes - without considering its results order.

If the optimizer shall be enhanced some day in a way, that a user can express preferences regarding the resulting relations order, then ~getTickedOffNode~ has to be changed, so that it considers the destination-node results order. Additionally the ~destinationNode~-clause from chapter 2.5.1 'Implementation of the Algorithm' of ~modifications.pl~ has to be changed too.

6.4 Considering Nodes with Different Result-Oders

6.4.1 Realizing ~intOrders(test)~

*/

assertCheckSuccessorClauses1 :-
        retractCheckSuccessorClauses,
	assertz(checkSuccessor(ReachedNodesSet, ReachedNodesSet, _, _, 
		succ(SuccNodeNo, _, _, _)) :-
	(isTickedOff(pair(SuccNodeNo, _)),
	 !)
	),
	assertz(checkSuccessor(ReachedNodesSet, NewReachedNodesSet, 
               node(NodeNo, NodePreds, Partition), NDPNode, 
               succ(SuccNodeNo, _, PredNo, Predicate)) :-
	(isInReachedNodesSet(ReachedNodesSet, node(SuccNodeNo, _, _)),
	 !,
	 createEdge(NodeNo, SuccNodeNo,
	 	   node(NodeNo, NodePreds, Partition),
                   PredNo, Predicate, Edge),
	 setResultOrder(Edge),
	 createPlanEdges(Edge, PlanEdges),
	 assignSize(Edge),
	 cheapestCostEdge(PlanEdges, CostEdge),
	 correctDistancePathAndOrder(NDPNode, SuccNodeNo, CostEdge,
                               ReachedNodesSet, NewReachedNodesSet)
	 )
	),
	assertz(checkSuccessor(ReachedNodesSet, NewReachedNodesSet, 
               node(NodeNo, NodePreds, Partition), NDPNode, 
               succ(SuccNodeNo, _, PredNo, Predicate)) :-
	(createEdge(NodeNo, SuccNodeNo,
		   node(NodeNo, NodePreds, Partition), 
                   PredNo, Predicate, Edge),
	setResultOrder(Edge),
	createPlanEdges(Edge, PlanEdges),
	assignSize(Edge),
	cheapestCostEdge(PlanEdges, CostEdge),
	correctResultOrder(CostEdge),
	createNDPSuccessor(succ(SuccNodeNo, _, _, _), NDPSuccessor),
	setDistanceAndPath(NDPNode, NDPSuccessor, CostEdge),
	getIntOrder(Edge, Result, Order),
	putIntoReachedNodesSetIO(ReachedNodesSet, NDPSuccessor,
				 intOrder(res(Result), Order),
                                 NewReachedNodesSet)
	 )
	).

/*

Please note, that ~putIntoReachedNodesSetIO~ is called immediately instead of the interface ~putIntoReachedNodesSet~ as it is done in chapter 4.3.2 'Determining the Orders of Intermediate Results', where the ~checkSuccessor~-clause is definded, that is used in the program-mode ~intOrders(quick)~.

*/

getIntOrder(edge(_, _, _, Result, _, _), Result, Order) :-
	intOrder(res(Result), Order).

correctDistancePathAndOrder(node(NodeNo, Distance, Path), SuccNo,
		 costEdge(NodeNo, SuccNo, Term, Result, Size, Cost), 
		 ReachedNodesSet, NewNewReachedNodesSet) :-
	readAlreadyReachedNode(ReachedNodesSet, SuccNo,
                               node(SuccNo, SuccDistance, SuccPath)),
	NewSuccDistance is Distance + Cost,
	% if
	SuccDistance > NewSuccDistance,
	!,
	% then
	deleteOutOfReachedNodesSet(ReachedNodesSet, 
                               node(SuccNo, SuccDistance, SuccPath), 
                               NewReachedNodesSet),
	append(Path, 
	       [costEdge(NodeNo, SuccNo, Term, Result, Size, Cost)],
               NewSuccPath),
	correctResultOrder(costEdge(NodeNo, SuccNo, Term, Result, Size, Cost)),
	intOrder(res(Result), Order),
	putIntoReachedNodesSetIO(NewReachedNodesSet,
				 node(SuccNo, NewSuccDistance, NewSuccPath),
				 intOrder(res(Result), Order),
                                 NewNewReachedNodesSet).

correctDistancePathAndOrder(_, _, _, ReachedNodesSet, ReachedNodesSet).
	% else do nothing

/*

6.4.2 Realizing ~intOrders(path)~

*/

assertCheckSuccessorClauses2 :-
        retractCheckSuccessorClauses,
	assertz(checkSuccessor(ReachedNodesSet, ReachedNodesSet, _, _,
		succ(SuccNodeNo, _, _, _)) :-
	(% if node with the respective order is already ticked off
	 isTickedOff(pair(SuccNodeNo, intOrder(res(Result), Order))),
	 intOrder(res(Result), Order),
	 !)
	),
	assertz(checkSuccessor(ReachedNodesSet, NewReachedNodesSet,
               node(NodeNo, NodePreds, Partition), NDPNode, 
               succ(SuccNodeNo, _, PredNo, Predicate)) :-
	(% if edge already exists
	 edge(NodeNo, SuccNodeNo, _, Result, _, _),
	 !,
	 % then
	 backupAndSetOrder(edge(_, _, _, Result, _, _)),
	 insertNewNodesIntoRNS(NodeNo, NDPNode, SuccNodeNo,
                               ReachedNodesSet, NewReachedNodesSet)
	 )
	),
	assertz(checkSuccessor(ReachedNodesSet, NewReachedNodesSet,
               node(NodeNo, NodePreds, Partition), NDPNode, 
               succ(SuccNodeNo, _, PredNo, Predicate)) :-
	(% else
	 createEdge(NodeNo, SuccNodeNo,
	 	   node(NodeNo, NodePreds, Partition),
                   PredNo, Predicate, Edge),
	 backupAndSetOrder(Edge),
	 createPlanEdges(Edge, PlanEdges),
	 assignSize(Edge),
	 createCostEdges(PlanEdges),
	 insertNewNodesIntoRNS(NodeNo, NDPNode, SuccNodeNo,
                               ReachedNodesSet, NewReachedNodesSet)
	 )
	).

/*

The maybe most interesting goal of the above clause is ~backupAndSetOrder~. It is necessary to backup the orders of the tuple-streams, that are joined, because the order of the joins result depends on the chosen join-implementation. And in the program-modes ~intOrders(path)~ and ~intOrders(on)~ for every single edge several plan- respectively cost-edges are created one after the other using different join-implementations. Thus it is necessary to reset the arguments orders before the creation of every new plan-edge, so that the formerly created plan-edge does have no influence on the creation of the following ones.

*/

backupAndSetOrder(edge(_, _, _, Result, _, _)) :-
	intOrder(res(Result), OriginalOrder),
	setResultOrder(edge(_, _, _, Result, _, _)),
	intOrder(res(Result), SetOrder),
	retractOrderBackups2,
	asserta(orderBackup2(res(Result), OriginalOrder, SetOrder)).
	% The OriginalOrder-argument is not actually needed.
	% Only the SetOrder-argument is relevant.

retractOrderBackups2 :-
	not(retractOrderBackup2).

retractOrderBackup2 :-
	retract(oderBackup2(_, _, _)),
	fail.

/*

The following clauses aim is to insert the target-nodes of every single plan-edge (together with the accompanying result-orders) into the set of already created but not yet ticked off nodes. This strategy ascertains, that for every possible order of an intermediate result at least one accompanying node will be considered during the shortest-path-construction.

Please note, that now for a given query with say n predicates not just "2^{n}"[1] nodes are maximum created, but a lot more. The total number depends on the number of plan-edges, that are created from all the nodes of the constructed graph to their successors.

*/

insertNewNodesIntoRNS(NodeNo, NDPNode, SuccNodeNo,
                               ReachedNodesSet, NewReachedNodesSet) :-
	findall(costEdge(NodeNo, SuccNodeNo, Plan, Result, Size, Cost),
		costEdge(NodeNo, SuccNodeNo, Plan, Result, Size, Cost),
		CostEdges),
	insertIntoRNS(NDPNode, SuccNodeNo, CostEdges,
	      ReachedNodesSet, NewReachedNodesSet).

insertIntoRNS(_, _, [], ReachedNodesSet, ReachedNodesSet) :-
	!.

insertIntoRNS(NDPNode, SuccNodeNo,
	      [costEdge(Source, Target, Plan, Result, Size, Cost)],
	      ReachedNodesSet, NewReachedNodesSet) :-
	!,
	correctResultOrder2(Plan, Result),
	createNDPSuccessor(succ(SuccNodeNo, _, _, _), NDPSuccessor),
	setDistanceAndPath(NDPNode, NDPSuccessor,
		costEdge(Source, Target, Plan, Result, Size, Cost)),
	intOrder(res(Result), Order),
	putIntoReachedNodesSetIO(ReachedNodesSet, NDPSuccessor,
				 intOrder(res(Result), Order),
                                 NewReachedNodesSet).

insertIntoRNS(NDPNode, SuccNodeNo,
	      [costEdge(Source, Target, Plan, Result, Size, Cost)|
	      CostEdges],
	      ReachedNodesSet, NewNewReachedNodesSet) :-
	correctResultOrder2(Plan, Result),
	createNDPSuccessor(succ(SuccNodeNo, _, _, _), NDPSuccessor),
	setDistanceAndPath(NDPNode, NDPSuccessor,
		costEdge(Source, Target, Plan, Result, Size, Cost)),
	intOrder(res(Result), Order),
	putIntoReachedNodesSetIO(ReachedNodesSet, NDPSuccessor,
			         intOrder(res(Result), Order),
                                 NewReachedNodesSet),
	insertIntoRNS(NDPNode, SuccNodeNo, CostEdges,
	              NewReachedNodesSet, NewNewReachedNodesSet).

correctResultOrder2(Plan, Result) :-
	orderBackup2(res(Result), _, SetOrder),
	asserta(intOrder(res(Result), SetOrder)),
	correctResultOrder(costEdge(_, _, Plan, Result, _, _)).

/*

6.4.3 Realizing ~intOrders(on)~

For to realize the program-mode ~intOrders(on)~ it is just necessary to modify ~putIntoReachedNodesSetIO~ in that way, that not the target-nodes of every single plan-edge (together with the accompanying result-orders) are inserted into the set of already created but not yet ticked off nodes. Now target nodes are only inserted if (a) the set does not yet contain a variant of the respective node, thats result has the same order, or (b) the set does contain a variant of the respective node, thats result has the same order, but the new determined path leading to that node-variant cause lower costs than the already stored one.

*/

:- dynamic putIntoReachedNodesSetIO/4.

intOrderonImplementation :-
	 retractall(putIntoReachedNodesSetIO(_, _, _, _) :- (_)),
	 assertz(putIntoReachedNodesSetIO(ReachedNodesSet,
                node(POGNodeNo, DistanceNew, _),
		Order, ReachedNodesSet) :-
	(% if
	 alreadyInRNS(node(_, POGNodeNo, _, _, Order),
		      ReachedNodesSet, DistanceOld),
	 DistanceOld < DistanceNew,
	 !,
	 % then do nothing
	 dm(intOrders,[POGNodeNo, ' ordered by ']),
	 dc(intOrders, ( Order = intOrder(_, ActualOrder) )),
	 dm(intOrders,[ActualOrder, ' was already reached', 
                       ' and the cost were lower.\n\n']),
	 true
	 )
	),
	assertz(putIntoReachedNodesSetIO(ReachedNodesSet,
                node(POGNodeNo, Distance, Path),
		Order, NewReachedNodesSet) :-
	(% else
	 retract(lastNodeNo(LastNodeNo)),
	 NewNodeNo is LastNodeNo + 1,
	 assert(lastNodeNo(NewNodeNo)),
	 dm(intOrders,[POGNodeNo, ' ordered by ']),
	 dc(intOrders, ( Order = intOrder(_, ActualOrder) )),
	 dm(intOrders,[ActualOrder, ' was not yet reached',
	               ' or is now reached with lower cost.\n\n']),
	 putIntoNodeListIO(ReachedNodesSet,
              node(NewNodeNo, POGNodeNo, Distance, Path, Order),
	      NewReachedNodesSet)
	 )
	).

alreadyInRNS(node(_, POGNodeNo, _, _, intOrder(_, Order)),
		  ReachedNodesSet, Distance) :-
	member(node(_, POGNodeNo, Distance, _, intOrder(_, Order)),
	       ReachedNodesSet).

/*

[newpage]

7 Testing the Different Program-Modes

7.1 Additional Test-Clauses

*/

otest1(Query, Cost) :- optimize(
  select *
  from [staedte as s, plz as p]
  where [p:ort = s:sname, p:plz > 40000,
	 (p:plz mod 13) = 0]
  orderby p:plz asc,
  Query, Cost
  ).

otest2(Query, Cost) :- optimize(
  select *
  from [staedte as s, plz as p]
  where p:ort = s:sname
  orderby p:plz asc,
  Query, Cost
  ).

/*

Note, that at the moment the original SECONDO-optimizer is not able to translate other than equi-joins (for details see chapter 5.2.3 'Translation of Joins' of ~optimizer.pl~).

*/

otest3(Query, Cost) :- optimize(
  select *
  from [plz as p1, plz as p2]
  where p1:plz > p2:plz,
  Query, Cost
  ).

otest4(Query, Cost) :- optimize(
  select *
  from [plz as p1, plz as p2]
  where p1:plz < p2:plz,
  Query, Cost
  ).

otest5(Query, Cost) :- optimize(
  select *
  from [plz as p1, plz as p2]
  where p1:plz >= p2:plz,
  Query, Cost
  ).

otest6(Query, Cost) :- optimize(
  select *
  from [plz as p1, plz as p2]
  where p1:plz =< p2:plz,
  Query, Cost
  ).

otest7(Query, Cost) :- optimize(
  select *
  from [plz as p1, plz as p2]
  where p1:plz =\= p2:plz,
  Query, Cost
  ).

otest8(Query, Cost) :- optimize(
  select *
  from [staedte, plz as p1, plz as p2]
  where [
    sname = p1:ort,
    p1:plz = p2:plz + 1],
  Query, Cost
  ).

otest9(Query, Cost) :- optimize(
  select *
  from [staedte as s, plz as p1, plz as p2,
	plz as p3, plz as p4, plz as p5]
  where [
    % p1:plz = p3:plz * 5,
    % p1:plz = p2:plz + 1,
    s:plz = p4:plz,
    s:plz = p3:plz,
    p4:plz = p5:plz + 1,
    p1:plz = p2:plz,
    p5:plz = p1:plz + 1
],
  Query, Cost
  ).

otest10(Query, Cost) :- optimize(
  select *
  from [staedte as s, staedte as cities,
	plz as p, plz as zip,
	orte as o, orte as places]
  where [
    o:ort=p:ort,
    places:ort=zip:ort,
    o:ort=zip:ort,
    places:ort=p:ort,
    s:sname=places:ort,
    cities:sname=o:ort,
    cities:sname=places:ort,
    s:sname=o:ort,
    s:sname contains "dorf"
],
  Query, Cost
  ).

/*

7.2 The Procedure

For to test the enhanced optimizer, that considers interesting orders, the test-clauses of chapter 2.6.2 'The Test-Clauses' from ~modifications.pl~ are used plus those of the above defined ones, that do not cause errors. (For every of the clauses ~example14~ to ~example21~ from ~optimizer.pl~ exists an equivalent in ~modifications.pl~.)

Intentionally in majority such clauses are used that were not specially designed for to test the consideration of interesting orders, because it is understood as an important aspect to get an idea, for how many out of a total number of arbitrary chosen queries cheaper plans can be created, if interesting orders are considered. But please note, that the proportion of cases, where cheaper plans can be created, and cases, where it is not possible to find cheaper plans, very likely vary very much dependent on the actual used databases and their actual use, i. e. the respective queries. Additionally it is understood, that it is more likely to find existing errors, if not specially designed test-queries are evaluated but arbitrary chosen ones.

As for the tests described in ~modifications.pl~ it was ensured that all needed selectivities were already available, so it was not necessary, that the optimizer had to call the SECONDO-kernel during the evaluation of a query. (For details see chapter 2.6.1 'The Procedure' of ~modifications.pl~.)

The used computer was a Fujitsu Siemens Amilo L 1300 Notebook with an Intel Celeron M 370 1.5 GHz processor and 512 MB RAM. The operating system was Linux, distribution 9.3 from the SUSE distributor with kernel 2.6.11.4. Furthermore SWI-Prolog 5.0.10 was used.

For to ensure, that every test-clause (and even ~test21~ and ~test25~) can be evaluated, the PROLOG-interpreter respectively the optimizer was started using the command 'SecondoPL -L64M, -G64M, -T64M' instead of just 'SecondoPL'. (What that means is described in detail in ~modifications.pl~.)

All test-clauses are called immediately after the call of ~sweepKnowledgeBase~ to ensure, that the time needed for to evaluate a test-query was not influenced by the complexity of a formerly evaluated query.

The test-clauses are called several times, because (a) different variants of the used database were defined, where the base-relations have different orders, and (b) every program-mode was tested (without ~intOrders(test)~). For to create different variants of the used database, the ~storedOrder~-facts, described in chapter 4.2 'Interesting Orders of Base-Relations', are manipulated as follows. And for to switch between the different program-modes, the commands ~intOrders(quick, time)~, ~intOrders(on, time)~, ~intOrders(path, time)~, and ~intOrders(off, time)~ are called beforehand.

Orders of the base-relations in the first test-phase:

----
storedOrder(staedte, sName).
storedOrder(orte, kennzeichen).
storedOrder(plz, ort).
----

Orders of the base-relations in the second test-phase:

----
storedOrder(staedte, pLZ).
storedOrder(orte, kennzeichen).
storedOrder(plz, pLZ).
----

Orders of the base-relations in the third test-phase:

----
storedOrder(staedte, sname).
storedOrder(orte, ort).
storedOrder(plz, ort).
----

Please note, that the database-variants are defined such, that in the first two test-phases two of the three base-relations are ordered by attributes that correspond in the sense, that they are convenient join-attributes, and in the latter all three base-relations are ordered by attributes that correspond in that sense.

With the results of the test-calls several tables (one for every database-variant) are built, that contain the following columns:

  1 ~nop~ = number of predicates of the test-query

  2 ~functor~ = the functor of the test-clause that leads to the evaluation
of the test-query

  3 ~mode~ = indicating, if ~intOrders(quick)~, ~intOrders(test)~, ~intOrders(path)~, ~intOrders(on)~ or ~intOrders(off)~ was called beforehand

  4 ~ctimes~ = times needed for either constructing the POG and searching the
shortest path through it or constructing the shortest path immediately including the consideration of interesting orders

  5 ~cost~ = the estimated cost of the following plan

  6 ~plan~ = the cheapest plan from the source- to the destination-node, that the respective program-mode found respectively constructed

7.3 The Results

All the results can be found in the file ~intOrdersSurvey.html~, attached to this file.

The most important result is, that the optimizer pretty often actually creates cheaper plans for given queries, if it considers interesting orders in the ways described above, than it does, if it ignores the orders of base-relations and intermediate results. In the first test-phase for 12 out of totally 30 queries cheaper plans are created. In the second test-phase for 5 out of totally 30 queries cheaper plans are created, and in the third again for 12 out of totally 30 queries cheaper plans are created. Often, when the estimated costs differ, they distinctly differ. Two examples are: The original optimizer finds a plan for query No. 20 (~test20~), that causes (estimated) costs of 632,54. But if interesting orders are considered, the cheapest plan, that can be created for the same query, causes (estimated) costs of only 381,449 in the first and 378,084 in the third test-phase. For query No. 30 (~otest10~) the original optimizer finds a plan, that causes (estimated) costs of 2093,11. If interesting orders are considered, the cheapest plan, that can be created for the same query, causes (estimated) costs of only 734,619 in the third test-phase. (For to interprete these numbers, please see chapter 6.2.5 of the SECONDO Programmer's Guide - page 60 if version 2 from october 5, 2004 is used - and chapter 8.1 'The Costs of Terms' of ~optimizer.pl~.)

It is not very surprising, that the program-mode ~intOrders(quick)~ does virtually always create plans faster than the program-modes ~intOrders(on)~ and ~intOrders(path)~, and that ~intOrders(path)~ needs a lot of time for the evaluation of complex queries, because the set of already created but not yet ticked off nodes contains by far a lot more nodes in that program-mode than in the other modes.

For some queries with just a few predicates the program-mode ~intOrders(path)~ works faster than the program-mode ~intOrders(on)~. The reason for this should be, that the number of nodes, that are inserted into the set of already created but not yet ticked off nodes, do not distinctly differ in both program-modes, because there are just a few predicates in the query, but in the program-mode ~intOrders(on)~ it is necessary to call the goal ~alreadyInRNS~ in ~putIntoReachedNodesSetIO~ for every created node.

The reason why ~intOrders(off)~ works faster for some queries with just a few predicates even than ~intOrders(quick)~ should be obvious: The original optimizer avoids the additional work, that has to be done for to consider interesting orders. Additionally as described in chapter 6.2 'A Reached-Nodes-Set-Implementation, that Stores Interesting Orders' ~intOrders(path)~ and ~intOrders(on)~ do not realize the most efficient of all possible implementations for the set of already created, but not yet ticked off nodes.

In theory, there must be cases, where ~intOrders(quick)~ does not find the cheapest of all thinkable plans (see chapter 1 'Introduction'). Only ~intOrders(path)~ and ~intOrders(on)~ guarantee to always find the cheapest of all plans. But somehow surprising is, that for only one of the test-queries (~otest10~) ~intOrders(quick)~ creates another plan than ~intOrders(path)~ and ~intOrders(on)~. Obviously the cases, where ~intOrders(quick)~ does actually not find the cheapest of all thinkable plans, are pretty seldom. (It must be said, that really some work was necessary to find an appropriate test-query, where the resulting plans differ.)

Thus it is probably more interesting for practical uses to always use the faster working program-mode ~intOrders(quick)~, that has the disadvantage, that for some particular queries not the cheapest of all possible plans will be created, than the slower working program-mode ~intOrders(on)~, that guarantees to always create the cheapest of all plans.

The fact, that for the test-query No. 30 (~otest10~) the different program-modes come to different resulting plans should not be over-interpreted: First it must be said, that this query was specially designed for to test the consideration of interesting orders and thus it is somehow odd, because it several times contains virtually identical predicates. Second the plans differ just slightly and cause exactly the same (estimated) costs. Third similar differences between the resulting plans for that query can be caused, if in the clause ~putIntoNodeListIO~ the goal ~Distance "<"[1] FirstDistance~ is replaced by ~Distance "=<"[1] FirstDistance~. In that case, in the first test-phase the call of ~intOrders(on, time), otest10(A,B)~ respectively ~intOrders(path, time), otest10(A,B)~ cause the result:

----
A = 'Staedte  feed {cities} Staedte  feed {s} Orte  feed {places}
sortby[Ort_places asc] mergejoin[SName_s, Ort_places] sortby[Ort_places asc] mergejoin[SName_cities, Ort_places]  filter[(.SName_s contains "dorf")]
Orte  feed {o}  product  filter[(.SName_s = .Ort_o)]
filter[(.SName_cities = .Ort_o)]  loopjoin[plz_Ort plz  exactmatch[.Ort_places]
{p} ]  filter[(.Ort_o = .Ort_p)]  loopjoin[plz_Ort plz  exactmatch[.Ort_o]
{zip} ]  filter[(.Ort_places = .Ort_zip)]  consume '
B = 2017.96
----

instead of:

----
A = 'Staedte  feed {cities} Staedte  feed {s} Orte  feed {o}
sortby[Ort_o asc] mergejoin[SName_s, Ort_o] sortby[Ort_o asc]
mergejoin[SName_cities, Ort_o]  filter[(.SName_s contains "dorf")]
Orte  feed {places}  product  filter[(.SName_cities = .Ort_places)]
filter[(.SName_s = .Ort_places)]  loopjoin[plz_Ort plz  exactmatch[.Ort_places]
{p} ]  filter[(.Ort_o = .Ort_p)]  loopjoin[plz_Ort plz  exactmatch[.Ort_o]
{zip} ]  filter[(.Ort_places = .Ort_zip)]  consume '
B = 2017.96
----

Please note, that the use of the variables ~places~ and ~o~ is exchanged similar to the the use of the variables ~cities~ and ~s~ in the documented program-test.

Last but not least it must be said, that even the modified optimizer, realized in ~modifications.pl~, and the original SECONDO-optimizer, that does not consider interesting orders, cause plans for that test-query, that differ in a similar way. Again the use of the variables ~places~ and ~o~ is exchanged similar to the the use of the variables ~cities~ and ~s~ in the documented program-test. (See the following small output-protocol.)

----
?- immPathCreation(off), otest10(A, B).
...
A = 'Staedte  feed {cities} Staedte  feed {s}  filter[(.SName_s contains "dorf")]
Orte  feed {o} sortmergejoin[SName_s, Ort_o] sortmergejoin[SName_cities, Ort_o]
Orte  feed {places}  product  filter[(.SName_cities = .Ort_places)]
filter[(.SName_s = .Ort_places)]  loopjoin[plz_Ort plz  exactmatch[.Ort_places]
{p} ]  filter[(.Ort_o = .Ort_p)]  loopjoin[plz_Ort plz  exactmatch[.Ort_o]
{zip} ]  filter[(.Ort_places = .Ort_zip)]  consume '
B = 2093.11

?- immPathCreation(on), otest10(A, B).
...
A = 'Staedte  feed {cities} Staedte  feed {s}  filter[(.SName_s contains "dorf")]
Orte  feed {places} sortmergejoin[SName_s, Ort_places]
sortmergejoin[SName_cities, Ort_places]  Orte  feed {o}
product  filter[(.SName_cities = .Ort_o)]
filter[(.SName_s = .Ort_o)] loopjoin[plz_Ort plz  exactmatch[.Ort_o]
{p} ]  filter[(.Ort_places = .Ort_p)] loopjoin[plz_Ort plz  exactmatch[.Ort_places]
{zip} ]  filter[(.Ort_o = .Ort_zip)] consume '
B = 2093.11

----

[newpage]

8 Literature

The following list considers only those books and papers, that have
been very useful for learning Prolog and programming the above
described extension of the original Secondo-optimizer.

W.F. Clocksin, C.S. Mellish, Programming in Prolog: using the ISO 
standard, 5th Edition, Berlin, Heidelberg, New York, 2003.

R.H. G[ue]ting, Datenstrukturen, Fernuniversit[ae]t Hagen, 1999.

R.H. G[ue]ting, T. Behr, V. T. de Almeida, Z. Ding, F. Hoffmann, 
and M. Spiekermann, Secondo: An Extensible DBMS Architecture and 
Prototype. Fernuniversit[ae]t Hagen, Informatik-Report 313, March 
2004, www.informatik.fernuni-hagen.de/import/pi4/papers/
Secondo04.pdf

R. H. G[ue]ting, D. Ansorge, T. Behr, M. Spiekermann, Secondo User 
Manual, Version 3, September 21, 2004. Fernuniversit[ae]t Hagen, 
www.informatik.fernuni-hagen.de/import/pi4/Secondo.html/
files/SecondoManual.pdf

R. H. G[ue]ting, V. T. de Almeida, D. Ansorge, T. Behr, 
M. Spiekermann. Secondo Programmers Guide, Version 2, 
October 5, 2004, Fernuniversit[ae]t Hagen, 
www.informatik.fernuni-hagen.de/import/pi4/
Secondo.html/files/ProgrammersGuide.pdf

G. R[oe]hner, Informatik mit Prolog, Wiesbaden, 2002.

M. Schneider, Implementierungskonzepte f[ue]r Datenbanksysteme, Fernuniversit[ae]t Hagen, 1999.

R. Sedgewick, Algorithmen, 2. Auflage, M[ue]nchen, 2002.

L. Sterling, E. Shapiro, Prolog, Fortgeschrittene 
Programmiertechniken, Bonn, 1988.

W. Weisweber, Prolog, Logische Programmierung in der Praxis, 
Bonn, 1997.

*/
