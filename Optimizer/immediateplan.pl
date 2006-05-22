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

[10] Modifications of the Query Optimizer for Secondo

Jan Engelkamp, August - September 2005

April 2006, Christian D[ue]ntgen. Adopted to work with recent optimizer.

[toc]

[newpage]

1 Introduction

The original query optimizer for the Secondo DBMS is contained
in a file named ~optimizer.pl~, which ca be found in the
~Optimizer~-directory of Secondo. Its idea and algorithm is described in detail
in R.H. G[ue]ting, T. Behr, V. T. de Almeida, Z. Ding, F. Hoffmann, and
M. Spiekermann, Secondo: An Extensible DBMS Architecture and Prototype.
Fernuniversit[ae]t Hagen, Informatik-Report 313, March 2004,
www.informatik.fernuni-hagen.de/import/pi4/papers/Secondo04.pdf, page 7 ff.

The present file named ~modifications.pl~ contains two modifications
of the query optimizer of Secondo, wich are:

  1 an algorithm, that immediately constructs the cheapest plan for a 
given query, instead of constructing a predicate order graph first and
then determining the cheapest plan by tracing the shortest path through
the graph.

  2 several alternative implementations for the set that contains 'the
already created but not yet ticked off nodes', which needs the
algorithm, that constructs the cheapest plan for a given query.

To use these modifications please copy the present file as ~modifications.pl~
into the ~Optimizer~-directory of Secondo, start ~SecondoPL~ as usual
and then enter the Prolog-command ~consult('modifications.pl')~ respectively
~['modifications.pl']~.

Each contained modification can be turned on and off using the following 
commands:

  1 ~setOption(immediatePlan)~ extends respectively changes the knowledge-base of
the optimizer in that way, that it immediately constructs the cheapest plan
for given queries. ~delOption(immediatePlan)~ turns this modification off again.

  2 The four alternative implementations for the set containing 'the
already created but not yet ticked off nodes' (REACHED-NODES)
are numbered 1, 2, 3 and 4. ~altRNSImplementation~ with one of the
numbers as its argument retracts the currently used implementation
from the knowledge-base and stores the chosen alternative
implementation instead. ~altRNSImplementation(off)~ reconsults the
file ~modifications.pl~ and consequently stores the original
implementation for the set REACHED-NODES again.

[newpage]

2 Immediate Construction of the Cheapest Plan for a given Query

2.1 The Idea and Algorithm

The original algorithm of the Secondo-optimizer first creates the
complete predicate order graph (POG) for a given query i. e. all its
nodes, edges, plan edges and cost edges and stores these elements as
facts into the knowledge-base. Only after the creation of the POG is
completed, the search of the shortest path through the POG starts.
(A POG for n predicates has "n!"[1] paths from the start-node 0 to
the destination-node "2^{n} - 1"[1].) The searches result i. e. the
shortest path through the graph is mapped onto the cheapest plan
for the given query.

The drawback of this algorithm is quite obvious: Always the complete graph with
all its "n * 2^{n-1}"[1] edges and "2^{n}"[1] nodes is created. (Not to mention
the plan and cost edges correponding to every edge of the graph.) So one can
assume, that it would cause a distinct improvement in efficiency of the above
described algorithm, if one would not create the complete POG first and then
search for the shortest path through it, but if the shortest path would
immediately be created.

An accordantly modified algorithm for the Secondo-optimizer is the following
one. In it two sets of nodes are used: REACHED-NODES for nodes that are already
created but not yet completely processed and TICKED-OFF-NODES for completely
processed nodes.


----
empty the set REACHED-NODES;
empty the set TICKED-OFF-NODES;
set the distance stored for the source-node to zero;
set the path stored for the source-node to [];
insert the source-node into the set REACHED-NODES;

while (REACHED-NODES is not empty) do
{ find the node cN in REACHED-NODES
       with minimal distance to the source-node ;
  construct/consider all successor-nodes of cN;

  for (every single successor-node sN of cN) do
  { if (sN is in TICKED-OFF-NODES) then
     { do nothing; }
    if (sN is not in TICKED-OFF-NODES but in REACHED-NODES) then
     { construct the edge from cN to sN;
       construct every plan edge from cN to sN;
       assign the estimated size of the interim result of sN to sN;
       construct the cost edges from cN to sN;
       find the cheapest of the cost edges from cN to sN;
       correct the distance and path stored for sN, if necessary;
     }
    if (sN is neither in TICKED-OFF-NODES nor in REACHED-NODES) then
     { construct the edge from cN to sN;
       construct every plan edge from cN to sN;
       assign the estimated size of the interim result of sN to sN;
       construct the cost edges from cN to sN;
       find the cheapest of the cost edges from cN to sN;
       calculate the distance and path of sN and store it;
       insert sN in REACHED-NODES;
     }
   }
   insert cN in TICKED-OFF-NODES;
   delete cN from REACHED-NODES;
}

read the distance and path stored for the destination-node,
that is the result;
----


2.2 Determining the Benefits for the Optimizers Efficiency

A predicate order graph (POG) for n predicates is a n-dimensional cube. It contains "2^{n}"[1] nodes. Every node has n neighbours. Hence in every loop of the above described algorithm at most n neighbours of the one current node are created respectively considered. At most, because not every neighbour is also a successor of the current node: Only the source-node has "n"[1] successors, its successors do have just "n-1"[1] successors, their successors do have just "n-2"[1] successors and so on. The destination-node has no successor. In every loop only those edges with the current node as its source have to be created, that lead to a successor, that is not yet completely processed (i. e. that is not yet a member of the set TICKED-OFF-NODES). Hence it will never be necessary to create every edge of a POG, if this graph consists of more than two nodes.

How many successors of a current node are already ticked off (processed) depends not only on the progress that the processing of the algorithm has made so far, but in what order the nodes of the POG become the current nodes of the algorithms loop. Hence a general conclusion for the benefit of efficiency, that causes the fact, that those edges, that would lead to nodes, which are already ticked off, cannot be given.

Additionally while processing the algorithm, by far not every node of the POG
needs to be regarded as the current node. A program can stop as soon as the
destination-node is reached. (But then the above described algorithm has to
be changed a bit; see chapter 2.5.1.) Hence often a large part of the POG can
be pruned away. In R.H. G[ue]ting et al., Secondo: An Extensible DBMS
Architecture and Prototype, page 16, one can find a small table which shows,
that for some examples instead of 256 only 18, instead of 512 only 104 and
instead of 1024 only 109 nodes are considered while searching the shortest path
through a POG. Thus using the above described algorithm (respectively the one
described in chapter 2.5.1) one can achieve that a lot less nodes have to be
created than are contained in the complete POG. For example if the clause
~test6~ (see chapter 2.6.2) is called, only 38 are created but 128 nodes are in
the complete POG, and if ~test23~ is called, only 224 instead of 2.048 nodes are
created.

This fact does probably have a much bigger effect on the optimizers efficiency
than the one, that it is not necessary to create edges to successors which are
already ticked off. But as already mentioned the order in which nodes, that have
to be considered, become the current nodes of the algorithms loop depends on the
given query. Hence a general conclusion of how big the improvement coming from
the fact, that it is by far not necessary to create every node of the POG, can't
be given.

To make reliable conlusions regarding the overall benefits for the optimizers
efficiency and to find out, if the modification of the optimizer is in real more
efficient than the original one, it is necessary to measure and compare times
needed for optimizing example-queries using first the original optimizer and the
other time the modified one.

This will be done in a following chapter named 'Measuring und Comparing 
Times needed for Optimization'.

2.3 Preparation

To extend respectively change the knowledge-base of the optimizer in that
way, that it immediately constructs the cheapest plan for given queries,
please enter ~setOption(immediatePlan)~ after starting ~SecondoPL~ as usual.
This will aso ensure, that this file (~immediateplan.pl~) is consulted.

~delOption(immediatePlan)~ turns this modification off, so one can use
the original implemented algorithm of the optimizer again.

The time needed for constructing respectively finding the cheapest plan
respectively shortest path will be given, if one uses ~setOption(immediatePlanTime)~.

There are goals in several clauses existing just for observing, how the
modified optimizer in detail works. They can be activated by switching on debugmode
~setOption(debug)~ and setting debug level to ~immPath~ (by ~debugLevel(immPath)~).

This will result in a lot of information about how the optimizer works being 
printed on your screen during construction of the cheapest plan. 

April 2006, Christian D[ue]ntgen. ~Observe~ was replaced by ~dm~ and/or ~dc~
commands. Set ~setOption(debug), debugLevel(immPath)~ to observe the algorithm.
Changed integration with optimizer.pl by removing dynamic code
modification (~immPathCreation/1~, ~immPathCreation/2~) with static predicate 
~immPlanTranslate/4~ and ~optimizerOption/1~ for the sake of a common interface.
Switching of optimizer options is now handled in file  ~calloptimizer.pl~.

Mai 2006, Christian D[ue]ntgen. Time measuring code moved to ``optimizer-pl'' as a
general optimizer option.

*/

:- dynamic immPathCreation/4.
:- dynamic createEmptyReachedNodesSet/1.
:- dynamic putIntoReachedNodesSet/3.
:- dynamic isInReachedNodesSet/2.
:- dynamic readAlreadyReachedNode/3.
:- dynamic getMinimalDistantNode/3.
:- dynamic deleteOutOfReachedNodesSet/3.
:- dynamic isEmpty/1.


% This is the toplevel predicate for this extension, called by optimizer.pl
% when ~optimizerOption(immediatePlan)~ is defined.
immPlanTranslate(Select from Rels where Preds, Stream, Select, Cost) :-
  optimizerOption(immediatePlan),
  immPathCreation(Rels, Preds, Stream, Cost), 
  !.

% Fall-back case, that should never be called
immPlanTranslate(_, _, _, _) :-
  write('\nWARNING: Fall-back clause of immPlanTranslate/4.'),
  throw(immPlanTranslate_fallback_case_reached), fail, !.

  
immPlanPrintWelcomeMod :-
     nl, write('*** Instead of creating the predicate order'),
     nl, write('*** graph and searching the shortest path'),
     nl, write('*** through it, from now on the shortest path'), 
     nl, write('*** will immediately be created.'),
     nl,
     nl, write('*** Please type delOption(immediatePlan) to use'),
     nl, write('*** the originally implemented algorithm of the'), 
     nl, write('*** Secondo-optimizer again.'), nl.

immPlanPrintWelcomePOG :-
     nl, write('*** From now on the predicate order graph first'),
     nl, write('*** will be created and after this the shortest'), 
     nl, write('*** path through it will be searched.'),
     nl,
     nl, write('*** Please type setOption(immediatePlan) to use the'),
     nl, write('*** modification of the optimizer again,'),
     nl, write('*** that immediately creates the shortest path.'), nl.
	

/*

[newpage]

2.4 Initial Activities

2.4.1 Sweeping the Knowledge-Base

Before the cheapest plan for a query can be constructed, one has to guarantee,
that no facts are part of the knowledge-base, that have been put into it, when
a former query has been evaluated.

*/

sweepKnowledgeBase(EmptyReachedNodesSet) :-
	deleteArguments,
	deleteNodes,
	deleteEdges,
	deletePlanEdges,
	deleteVariables,
	deleteSizes,
	deleteCostEdges,
	createEmptyReachedNodesSet(EmptyReachedNodesSet),
	emptyTickedOffSet,
	deleteMinDistNodes,
	optimizerOption(intOrders(_)) -> retractAllOrderInformation ; true.


/*

The last call is only needed, if one wants to count or to analyse the nodes
that become the nodes with the minimal distance to the source-node while
creating the shortest path through the POG (see chapter 2.5.16).

2.4.2 Storing ARP-Tripples

Given a list of relations, they all are packed into
argument-relation-predicate-tripples of the form arp(arg(i), [relation], []).
All these trippels are stored into the knowledge-base.

*/

assertArguments(Rels, Partition) :-
	length(Rels, Length),
	reverse(Rels, RevRels),
	partition(RevRels, Length, Partition).

/*

2.5 The Construction of the Cheapest Plan

2.5.1 Implementation of the Algorithm

*/

immPathCreation(Relations, Predicates, Plan, Cost) :-
	sweepKnowledgeBase(EmptyReachedNodesSet),
	assertArguments(Relations, Partition),
	length(Predicates, NoOfPreds),
	dc(immPath, ( write('Number of predicates in the query: '), 
	              write(NoOfPreds), nl,
                      write('All the predicates of the query: '), 
                      write(Predicates), nl, nl)),
	SourceNodeNo = 0,
	asserta(node(SourceNodeNo, [], Partition)),
	putIntoReachedNodesSet(EmptyReachedNodesSet, 
			       node(SourceNodeNo, 0, []),
			       ReachedNodesSet),
	immPathCreationLoop(ReachedNodesSet, Predicates, NoOfPreds),
	dc(immPath, (write('All nodes are processed.'), nl)),
	DestinationNodeNo is 2^NoOfPreds -1,
	getTickedOffNode(DestinationNodeNo,
			node(DestinationNodeNo, Cost, ShortestPath)),
	plan(ShortestPath, Plan).

immPathCreationLoop(ReachedNodesSet, Predicates, NoOfPreds) :-
	% if
	not(isEmpty(ReachedNodesSet)),
	!,
	% then
	processReachedNodesSet(ReachedNodesSet, NewReachedNodesSet, 
			       Predicates, NoOfPreds),
	immPathCreationLoop(NewReachedNodesSet, 
			    Predicates, NoOfPreds).

immPathCreationLoop(_, _, _) :-
	% if isEmpty(ReachedNodesSet) return true
	dc(immPath, (write('The set REACHED-NODES is empty.'), nl, nl)),
	true.

/*

For a correct implementation of the above described algorithm, the next
clause ~processReachedNodesSet~ would have to be the following one:

----
processReachedNodesSet(ReachedNodesSet, NewNewReachedNodesSet, 
		       Predicates, NoOfPreds) :-
	getMinimalDistantNode(ReachedNodesSet, NDPNode, 
			      NewReachedNodesSet),
	mapNPPNode_NDPNode(NPPNode, NDPNode),
	createSuccessors(NPPNode, NoOfPreds, 
			 Predicates, SuccessorList),
	processSuccessors(NewReachedNodesSet, NewNewReachedNodesSet,
			  NPPNode, NDPNode, SuccessorList),
	tickOff(NDPNode).
----

But then inevitably all the nodes of the POG would be created, because
the criteria causing the loop to stop would be the emptiness of the set
REACHED-NODES (as intended and corresponding to the algorithm).
However the aim of the immediate construction of the shortest path
respectively cheapest plan is to stop the construction of nodes and
edges as soon as the destination-node is reached.

Thus the above described algorithm must be changed as follows:

----
empty the set REACHED-NODES;
empty the set TICKED-OFF-NODES;
set the distance stored for the source-node to zero;
set the path stored for the source-node to [];
insert the source-node into the set REACHED-NODES;

while (REACHED-NODES is not empty
       and the destination-node is not yet reached) do
{ ... }

read the distance an path stored for the destination-node,
that is the result;
----

Now there are two criterias causing the loop to stop: first the emptiness
of the set REACHED-NODES and second the fact, that the destination-node
is reached.

This leads to the following implementation of ~processReachedNodesSet~:

*/

processReachedNodesSet(ReachedNodesSet, NewNewReachedNodesSet, 
                       Predicates, NoOfPreds) :-
	getMinimalDistantNode(ReachedNodesSet, NDPNode,
			      NewReachedNodesSet),
	dc(immPath, (write('Minimal distant node: '), write(NDPNode), nl, nl)),
	tickOff(NDPNode),
	mapNPPNode_NDPNode(NPPNode, NDPNode),
	dc(immPath, assertz(minDistNode(NPPNode))),
	createSuccessors(NPPNode, NoOfPreds,
			 Predicates, SuccessorList),
	dc(immPath, (write('Successors are created.'), nl, nl)),
	% if
	not(destinationNode(NDPNode, NoOfPreds)),
	!,
	% then
	processSuccessors(NewReachedNodesSet, NewNewReachedNodesSet, 
                          NPPNode, NDPNode, SuccessorList),
	dc(immPath, (write('*** Successors processed. ***'), nl, nl)),
	true.

processReachedNodesSet(_, EmptyReachedNodesSet, _, _) :-
	% else i. e. the destination-node is reached
	createEmptyReachedNodesSet(EmptyReachedNodesSet).

destinationNode(node(NodeNo, _, _), NoOfPreds) :-
	NodeNo >= 2^NoOfPreds -1.

/*

The second clause

----
processReachedNodesSet(_, EmptyReachedNodesSet, _, _) :-
	createEmptyReachedNodesSet(EmptyReachedNodesSet).
----

realizes some kind of trick. If the destination-node is reached, the
set REACHED-NODES ist emptied and thus the criteria causing the loop to stop
is fulfilled. It would be more neatly (and a more correct implementation of the
algorithm), if both of the two criterias 'the set REACHED-NODES is empty' and
'the destination-node is reached' would cause the loop to stop, but therefore
more complex clauses would be needed, which are by far less comprehensible.

For an exact implementation of the algorithm, there must also be a goal like
~deleteOutOfReachedNodesSet(SetIn, Node, SetOut)~ at the end, that deletes the
current node out of the set named REACHED-NODES. But a goal like that is not
needed, because ~getMinimalDistantNode(SetIn, Node, SetOut)~ not only delivers
the node with the current minimal distance to the source-node, but at the same
time deletes the node out of the given set of already reached nodes.

The call ~assertz(minDistNode(NPPNode))~ in the first clause is needed, if one wants
to count or to analyse the nodes that become the nodes with the minimal
distance to the source-node while creating the shortest path through the POG.
After the shortest path has been created, it is possible to call
~countMinDistNodes~ and ~writeMinDistNodes~. If these nodes are of no interest,
the above call should be commented out.

2.5.2 Creating the Successors of a Given Node

*/

createSuccessors(Node, NoOfPreds, Predicates, SuccessorList) :-
	successorNodes(Node, NoOfPreds, 
		       Predicates, 1, [], SuccessorList).

/*

The arguments of

----
successorNodes(node(NodeNo, NodePreds, Partition), NoOfPreds, Predicates, 
	       PredPos, Accumulator, SuccessorList)
----
have the following meaning:

  1 ~node(NodeNo, NodePreds, Partition)~ is the current node, of which
successors are created respectively considered. ~NodePreds~ is its list of
predicates.

  2 ~NoOfPreds~ is the total of all existing predicates in the given query.

  3 ~Predicates~ is a list containing all the queries predicates.

  4 ~PredPos~ is the position of the currently for the construction of the next
successor considered predicate. It must be initialized with 1 when the clause
~successorNodes~ is called first. (It would be easier if it would be initialized
with 0, but in ~optimizer.pl~ the first predicate has the position 1 instead of
0. Thus here the same is done.)

  5 ~Accumulator~ is an accumulator for the list of successors. Initially it 
must be the empty list.

  6 ~SuccessorList~ is the list of successors of the current node.

*/

successorNodes(_, NoOfPreds, _, PredPos, 
	       SuccessorList, SuccessorList) :-
	PredPos > NoOfPreds,
	!. % All successors are constructed.

successorNodes(node(NodeNo, NodePreds, Partition), 
	       NoOfPreds, [Predicate|RestAllPreds], PredPos, 
               Accumulator, SuccessorList) :-
	% if
	member(Predicate, NodePreds),
        !,
	% then
	NewPredPos is PredPos + 1,
	successorNodes(node(NodeNo, NodePreds, Partition), 
		       NoOfPreds, RestAllPreds, NewPredPos, 
                       Accumulator, SuccessorList).

successorNodes(node(NodeNo, NodePreds, Partition), 
	       NoOfPreds, [Predicate|RestAllPreds], PredPos, 
               Accumulator, SuccessorList) :-
	% else i.e. not(member(Predicate, NodePreds)),
	PredNo is 2^(PredPos-1),
	NewNodeNo is NodeNo + PredNo,
	createSuccessor(NewNodeNo, Predicate, PredNo, NodePreds, 
                        Partition, NewNode),
	NewPredPos is PredPos + 1,
	successorNodes(node(NodeNo, NodePreds, Partition), 
          NoOfPreds, RestAllPreds, NewPredPos,
	  [succ(NewNodeNo, NewNode, PredNo, Predicate)|Accumulator], 
          SuccessorList).

/*

2.5.3 Creating a New Node

*/

createSuccessor(NewNodeNo, _, _, _, _, NewNode) :-
	% if
	node(NewNodeNo, StoredNodePreds, Partition),
	!,
	% then
	NewNode = node(NewNodeNo, StoredNodePreds, Partition).

createSuccessor(NewNodeNo, Predicate, 
		PredNo, NodePreds, Partition, NewNode) :-
	% else
	NewNode = node(NewNodeNo, 
		       [Predicate|NodePreds], NewPartition),
	assertz(NewNode),
	copyPart(Predicate, PredNo, Partition, NewPartition),
	retract(node(NewNodeNo, _, _)),
	assertz(NewNode).

/*

To avoid unnecessary duplications of clauses and to use the clauses of the file
~optimizer.pl~ when possible, ~copyPart~ is used as a goal in ~createSuccessor~.

But ~copyPart~ can only be used if a node that contains ~NewPartition~
as its third element is already part of the knowledge-base. However 
~NewPartition~ is not yet instantiated when ~copyPart~ is called. That's the 
reason, why ~NewNode~ is first asserted and then, after ~NewPartition~ has been
instantiated, is rectracted and again (with the meanwhile instantiated 
~NewPartition~) asserted.

2.5.4 Different Representations for Nodes

*/

createNDPNode(node(NodeNo, _, _), node(NodeNo, _, _)).

createNDPSuccessor(succ(NodeNo, _, _, _), node(NodeNo, _, _)).

mapNPPNode_NDPNode(node(NodeNo, Predicates, Partition), 
		   node(NodeNo, _, _)) :-
	node(NodeNo, Predicates, Partition).

/*

There exist two structures in the knowledge-base named ~node~ containing three
elements each, that realize different representations for nodes.
They are called ~NPPNodes~ and ~NDPNodes~.
~NPPNode~ stands for number-predicates-partition-node.
Its form is ~node(NodeNo, Predicates, Partition)~.
~NDPNode~ stands for number-distance-path-node.
Its form is ~node(NodeNo, Distance, Path)~.

At first one might think that it is a bit awkward to use those different
~node~-structures in parallel. One might think that is would be better to use
only one structure like ~node(NodeNo, Predicates, Partition, Distance, Path)~.
But notice: Facts of the form ~node(NodeNo, Predicates, Partition)~ are part
of the knowledge-base, but ~node(NodeNo, Distance, Path)~-facts only exist
as part of the elements of the set that describes the alredy reached but not yet
ticked off nodes, called REACHED-NODES in the above described algorithm.

The first clause ~createNDPNode~ is currently not used but added for
comprehensibility.

2.5.5 Processing the Successors of a Node

*/


processSuccessors(ReachedNodesSet, ReachedNodesSet, _, _, []) :-
	!.

processSuccessors(ReachedNodesSet, NewReachedNodesSet, NPPNode, 
                  NDPNode, [Successor]) :-
	dc(immPath, (write('Check Successor: '), write(Successor), nl)),
	checkSuccessor(ReachedNodesSet, NewReachedNodesSet, NPPNode, 
                       NDPNode, Successor),
	dc(immPath, (write('Successor checked.'), nl, nl)),
	true.

processSuccessors(ReachedNodesSet, NewNewReachedNodesSet, NPPNode, 
                  NDPNode, [Successor|SuccessorList]) :-
	dc(immPath, (write('Check Successor: '), write(Successor), nl)),
	checkSuccessor(ReachedNodesSet, NewReachedNodesSet, NPPNode, 
                       NDPNode, Successor),

	dc(immPath, (write('Successor checked.'), nl, nl)),
	processSuccessors(NewReachedNodesSet, NewNewReachedNodesSet,
			  NPPNode, NDPNode, SuccessorList).

/*

The true in the second clause is only needed for the above described workaround
concerning the search and replace of ~o b s e r v e~ if one wants to watch in
detail how the algorithm works.

Note: When ~getMinimalDistantNode~ is called as a goal finally a structure
containing a ~NDPNode~ is retracted from the knowledge-base. This takes place
even if a clause that called ~getMinimalDistantNode~ fails after calling this
goal. For details please consider the file ~boundary.pl~.

However the description here causes one to presume that a call of
~getMinimalDistantNode~ leads to a deletion of a ~NDPNode~ out of a set (that
is usually realized as a list in Prolog). Such kind of deletion out of a set
would not take place, if the clause ~processReachedNodesSet~ fails. Hence the
set named ~ReachedNodesSet~ in the clause ~immPathCreationLoop~ (the first
argument there) would not be empty after ~processReachedNodesSet~ failed, if
a set respectively a list would be used.

Because finally the call of ~getMinimalDistantNode~ retracts a fact out of the
knowledge-base, this takes place even if ~processReachedNodesSet~ fails.
Thus the first of the above clauses is necessary to guarantee, that a clause
calling ~processSuccessors~ doesn't even fail, if the destination-node, which
has no successors, is reached. This clause is currently not needed, because
the construction of the shortest path immediately stops, when the
destination-node is reached, but it is absolutely necessary, if all the nodes
and edges of the POG should be created.

2.5.6 Creating Edges to Successors

*/

:- dynamic checkSuccessor/5.

checkSuccessor(ReachedNodesSet, ReachedNodesSet, _, _, 
		succ(SuccNodeNo, _, _, _)) :-
	isTickedOff(SuccNodeNo),
	dc(immPath, (write('Successor is already ticked off.'), nl)),
	!.

checkSuccessor(ReachedNodesSet, NewReachedNodesSet, 
               node(NodeNo, NodePreds, Partition), NDPNode, 
               succ(SuccNodeNo, _, PredNo, Predicate)) :-
	isInReachedNodesSet(ReachedNodesSet, node(SuccNodeNo, _, _)),
	!,
	dc(immPath, (write('Successor is not ticked off '), 
	             write('but already reached.'), nl)),
	createEdge(NodeNo, SuccNodeNo,
		   node(NodeNo, NodePreds, Partition),
                   PredNo, Predicate, Edge),
	createPlanEdges(Edge, PlanEdges),
	assignSize(Edge),
	cheapestCostEdge(PlanEdges, CostEdge),
	correctDistanceAndPath(NDPNode, SuccNodeNo, CostEdge,
                               ReachedNodesSet, NewReachedNodesSet).

checkSuccessor(ReachedNodesSet, NewReachedNodesSet, 
               node(NodeNo, NodePreds, Partition), NDPNode, 
               succ(SuccNodeNo, _, PredNo, Predicate)) :-
	dc(immPath, (write('Successor is not ticked off '),
	             write('and not yet reached.'), nl)),
	createEdge(NodeNo, SuccNodeNo, 
		   node(NodeNo, NodePreds, Partition), 
                   PredNo, Predicate, Edge),
	createPlanEdges(Edge, PlanEdges),
	assignSize(Edge),
	cheapestCostEdge(PlanEdges, CostEdge),
	dc(immPath, (write('Cheapest cost edge is determined: '), 
	             write(CostEdge), nl)),
	createNDPSuccessor(succ(SuccNodeNo, _, _, _), NDPSuccessor),
	setDistanceAndPath(NDPNode, NDPSuccessor, CostEdge),
	putIntoReachedNodesSet(ReachedNodesSet, NDPSuccessor,
                               NewReachedNodesSet).

/*

2.5.7 Creating a New Edge

*/

createEdge(NodeNo, NewNodeNo, Node, PredNo, _, NewEdge) :-
	% if
	edge(NodeNo, NewNodeNo, _, _, Node, PredNo),
	!,
	% then
	NewEdge = edge(NodeNo, NewNodeNo, _, _, Node, PredNo).

createEdge(NodeNo, NewNodeNo, Node, PredNo, Predicate, NewEdge) :-
	% else
	NewEdge = edge(NodeNo, NewNodeNo, _, _, Node, PredNo),
	assertz(NewEdge),
	newEdge(Predicate, PredNo, Node, NewEdge),
	retract(edge(NodeNo, NewNodeNo, _, _, _, _)),
	assertz(NewEdge).

/*

To avoid unnecessary duplications of clauses and to use the clauses of the file
~optimizer.pl~ when possible, ~newEdge~ is used as a goal in ~createEdge~.

But ~newEdge~ can only be used if a corresponding ~edge~-structure is already
part of the knowledge-base. However ~NewEdge~ is not yet instantiated when 
~newEdge~ is called. That's the reason, why ~NewEdge~ is first asserted and 
then, after ~newEdge~ has been called, is rectracted and again (with the 
meanwhile instantiated elements) asserted.

2.5.8 Creating Plan Edges

*/

createPlanEdges(Edge, PlanEdges) :-
	createPlanEdges(Edge),
	allCorrPlanEdges(Edge, PlanEdges).

createPlanEdges(Edge) :-
	not(createPlanEdge(Edge)).

createPlanEdge(edge(Source, Target, Term, Result, _, _)) :-
	Term => Plan,
	assert(planEdge(Source, Target, Plan, Result)),
	fail.

/*

According to ~createPlanEdges~/0 and ~createPlanEdge~/0, which are defined in 
~optimizer.pl~ the clauses ~createPlanEdges~/1 and ~createPlanEdge~/1 aim at 
the creation of plan edges corresponding to a given Edge.

2.5.9 Getting Plan Edges

*/

allCorrPlanEdges(edge(Source, Target, _, Result, _, _), PlanEdges) :-
	findall(planEdge(Source, Target, Plan, Result),
		planEdge(Source, Target, Plan, Result), PlanEdges).

getPlanEdges(EdgeList, PlanEdgeList) :-
	getPlanEdges(EdgeList, [], PlanEdgeList).

getPlanEdges([], PlanEdgeList, PlanEdgeList).

getPlanEdges([Edge|EdgeList], Accumulator, PlanEdgeList) :-
	allCorrPlanEdges(Edge, PlanEdges),
	append(PlanEdges, Accumulator, AccumulatorPlus),
	getPlanEdges(EdgeList, AccumulatorPlus, PlanEdgeList).

/*

The ~getPlanEdges~-clauses are currently not used, but have been useful for
testing. And they may be useful for further modifications of the optimizer.

2.5.10 Assigning Sizes and Selectivities

*/

assignSizestoSucc([]).

assignSizestoSucc([Edge|EdgeList]) :-
	assignSize(Edge),
	assignSizestoSucc(EdgeList).

assignSize(edge(Source, Target, Term, Result, _, _)) :-
	assignSize(Source, Target, Term, Result).

/*

The ~assignSizestoSucc~-clauses are currently not used, but have been useful for
testing. And they may be useful for further modifications of the optimizer.

2.5.11 Creating Cost Edges

*/

createCostEdges([]).

createCostEdges([planEdge(Source, Target, Plan, Result)|
                 PlanEdgeList]) :-
	edgeSelectivity(Source, Target, Sel),
	cost(Plan, Sel, Size, Cost),
	assert(costEdge(Source, Target, Plan, Result, Size, Cost)),
	createCostEdges(PlanEdgeList).

/*

2.5.12 Getting the Cheapest Cost Edge

*/

cheapestCostEdge([planEdge(Source, Target, Plan, Result)|
		  PlanEdgeList], CostEdge) :-
	createCostEdges([planEdge(Source, Target, Plan, Result)|
		         PlanEdgeList]),
	getCheapestCostEdge(Source, Target, CostEdge).

/*

Note: This clause could cause problems, if the given list of plan edges is
empty. But in the current context it is guaranteed, that the list is not empty.

*/

getCheapestCostEdge(Source, Target, CheapestCostEdge) :-
	findall(costEdge(Source, Target, Plan, Result, Size, Cost),
		costEdge(Source, Target, Plan, Result, Size, Cost),
		CostEdges),
	findCheapestCostEdge(CostEdges, CheapestCostEdge).

findCheapestCostEdge([CheapestCostEdge], CheapestCostEdge) :-
	!.

findCheapestCostEdge([costEdge(Source, Target, Plan, 
                               Result, Size, Cost1)|Remainder], 
		      costEdge(ResSource, ResTarget, ResPlan, 
                      ResResult, ResSize, ResCost)) :-
	findCheapestCostEdge(Remainder, 
			     costEdge(_, _, _, _, _, Cost2)),
	% if
	Cost1 =< Cost2,
	!,
	% then
	costEdge(ResSource, ResTarget, ResPlan, 
		 ResResult, ResSize, ResCost) 
	= costEdge(Source, Target, Plan, Result, Size, Cost1).

findCheapestCostEdge([_|Remainder], CheapestCostEdge) :-
	% else
	findCheapestCostEdge(Remainder, CheapestCostEdge).

/*

One might expect, that the following clauses can be processed faster
than the above.

----
findCheapestCostEdge([costEdge(_, _, _, _, _, Cost1)|Remainder],
		      costEdge(Source2, Target2, Plan2,
			       Result2, Size2, Cost2)) :-
	findCheapestCostEdge(Remainder,
			     costEdge(Source2, Target2, Plan2,
				Result2, Size2, Cost2)),
	% if
	Cost1 >= Cost2,
	!.

findCheapestCostEdge([CheapestCostEdge|_], CheapestCostEdge).
	% else
----

But that is not true for a relatively small amount of elements in the
list, as tests have shown. For just a few elements in the list of
cost edges, the used clauses are a bit more efficient. And currently
just a few plan edges respectively cost edges are created for every
edge of the POG, hence the list always contains just a few elements.

2.5.13 Determining and Correcting Dinstances and Paths

*/

setDistanceAndPath(node(NodeNo, Distance, Path), 
               node(SuccNo, SuccDistance, SuccPath), 
	       costEdge(NodeNo, SuccNo, Term, Result, Size, Cost)) :-
	SuccDistance is Distance + Cost,
	dc(immPath, (write('Distance of successor '), write(SuccNo),
                     write(' determined: '), write(SuccDistance), nl)),
	append(Path, 
               [costEdge(NodeNo, SuccNo, Term, Result, Size, Cost)], 
               SuccPath).

correctDistanceAndPath(node(NodeNo, Distance, Path), SuccNo,
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
	dc(immPath, (write('Distance of successor is corrected: '),
	             write(NewSuccDistance), nl)),
	putIntoReachedNodesSet(NewReachedNodesSet, 
                         node(SuccNo, NewSuccDistance, NewSuccPath),
                         NewNewReachedNodesSet).

correctDistanceAndPath(_, _, _, ReachedNodesSet, ReachedNodesSet).
	% else do nothing
	
/*

2.5.14 Ticking Off Nodes - The Interface

The following rules just exist for comprehensibility.
Actually they are not absolutely necessary.

*/


% enable modifications by intOrder:
:- dynamic emptyTickedOffSet/0. 
:- dynamic tickOff/1. 
:- dynamic isTickedOff/1.
:- dynamic getTickedOffNode/2.

emptyTickedOffSet :-
	emptyCenter.

tickOff(node(NodeNo, Distance, Path)) :-
 	assert(center(NodeNo, node(NodeNo, Distance, Path))).

isTickedOff(NodeNo) :-
 	center(NodeNo, _).

getTickedOffNode(NodeNo, Node) :-
	center(NodeNo, Node).

/*

2.5.15 The Reached-Nodes Set - The Interface

The following rules primarily exist for comprehensibility. They don't do more
than map their calls onto calls of the abstract data type ~boundary~, that is
realized in the file ~boundary.pl~. But they form some kind of interface, that
is useful for further modifications (like the one described in chapter 3,
that would be by far more complex without this interface).

*/

createEmptyReachedNodesSet(ReachedNodesSet) :-
	b_empty(ReachedNodesSet).

putIntoReachedNodesSet(ReachedNodesSet, Node, NewReachedNodesSet) :-
	b_insert(ReachedNodesSet, Node, NewReachedNodesSet).

isInReachedNodesSet(ReachedNodesSet, node(NodeNo, _, _)) :-
	b_memberByName(ReachedNodesSet, NodeNo, node(NodeNo,_ , _)).

readAlreadyReachedNode(ReachedNodesSet, NodeNo, Node) :-
	b_memberByName(ReachedNodesSet, NodeNo, Node).

getMinimalDistantNode(ReachedNodesSet, Node, NewReachedNodesSet) :-
	b_removemin(ReachedNodesSet, Node, NewReachedNodesSet).

deleteOutOfReachedNodesSet(ReachedNodesSet, node(NodeNo, _, _), 
                           NewReachedNodesSet) :-
	b_deleteByName(ReachedNodesSet, NodeNo, NewReachedNodesSet).

isEmpty(ReachedNodesSet) :-
	b_isEmpty(ReachedNodesSet),
	dc(immPath, (write('There are no more reached '),
	             write('but not yet ticked off nodes.'), nl)),
	true.

/*

The true in the last clause is only needed for the above described
workaround concerning the search and replace of ~o b s e r v e~ if
one wants to watch in detail how the algorithm works.

2.5.16 The Nodes with Minimal Distance to the Source-Node

In the clause ~processReachedNodesSet~ a call ~assertz(minDistNode(NPPNode))~
is included but normally commented out (see chapter 2.5.1). It is only needed,
if one wants to count or to analyse the nodes that become the nodes with the
minimal distance to the source-node while creating the shortest path through
the POG. After the shortest path has been created, it is possible to call
~countMinDistNodes~ and ~writeMinDistNodes~.

*/

:- dynamic minDistNode/1.

deleteMinDistNodes :-
	not(deleteMinDistNode).

deleteMinDistNode :-
	retract(minDistNode(_)),
	fail.

writeMinDistNodes :-
	not(writeMinDistNode).

writeMinDistNode :-
	minDistNode(node(NodeNo, _, _)),
	write('Node: '), write(NodeNo), nl,
	fail.

countMinDistNodes :-
	findall(minDistNode(Node), minDistNode(Node), Nodes),
	length(Nodes, Length),
	write(Length), write(' nodes were found '),
	write('(including the destination node).'), nl.

/*

2.6 Measuring und Comparing Times needed for Optimization

April 2006, Christian D[ue]ntgen: 

The text of this and the following sub-sections of section 2 have not been adopted to the current 
implementation (regarding integration into the optimizer and switching between options).

2.6.1 The Procedure

*/ 

checkCalcTime(Head):-
	get_time(Time1),
	call(Head),
	get_time(Time2),
	Time3 is 1000*(Time2-Time1),
	write('Time needed for calculating the overall result: '), 
	write(Time3), nl.

countNodes :-
	findall(node(NodeNo, Predicates, Partition),
		node(NodeNo, Predicates, Partition), Nodes),
	length(Nodes, Length),
	write(Length), write(' nodes were found.'), nl.

countPlanEdges :-
	findall(planEdge(Source, Target, Plan, Result),
		planEdge(Source, Target, Plan, Result), PlanEdges),
	length(PlanEdges, Length),
	write(Length), write(' plan edges were found.'), nl.

/*

~checkCalcTime~ with the following test-clauses as its argument has been called ten times for each test-clause. Five times after ~immPathCreation(on, time)~ was called and five times after ~immPathCreation(off, time)~ was called.

Additionally the actually created nodes and plan edges have been count.

It was ensured that all needed selectivities were already available, hence it
was not necessary, that the optimizer had to call the Secondo-kernel. For to
ensure, that the kernel was never called, all test-clauses were called at least
once in advance, ~SecondoPL~ then was quitted and afterwards newly started and
~secondo('open database opt', Res).~ was not called in the context of the
surveys.

Thus the file ~storedSels.pl~ contained the following entries:

----
storedSel(orte:ort=plz:ort, 0.000560748).
storedSel(plz:pLZ=44225, 0.00233645).
storedSel(staedte:bev>270000, 0.344828).
storedSel(staedte:sName starts "S", 0.103448).
storedSel(orte:ort contains "dorf", 0.04).
storedSel((plz:pLZ mod 13)=0, 0.0630841).
storedSel(staedte:sName=orte:ort, 0.00155172).
storedSel(staedte:pLZ=plz:pLZ, 4.02836e-05).
storedSel(staedte:vorwahl=orte:vorwahl, 0.00275862).
storedSel(staedte:kennzeichen=orte:kennzeichen, 0.00293103).
storedSel(staedte:bev*1000=orte:bevT, 0.000172414).
storedSel(staedte:kennzeichen starts "A", 0.0344828).
storedSel(staedte:pLZ<60000, 1.01724).
storedSel(staedte:pLZ>50000, 0.0172414).
storedSel(orte:kennzeichen starts "A", 0.04).
storedSel(staedte:sName contains "burg", 0.137931).
storedSel(staedte:sName starts "D", 0.0862069).
storedSel(staedte:kennzeichen starts "D", 0.0862069).
storedSel(staedte:sName contains "dorf", 0.0344828).
storedSel(staedte:pLZ>40000, 0.0172414).
storedSel(staedte:pLZ<50000, 1.01724).
----

The used computer for the tests was a Fujitsu Siemens 
Amilo L 1300 Notebook with an Intel Celeron M 370 1.5 GHz 
processor and 512 MB RAM.

The operating system was Linux, distribution 9.3 from the SUSE distributor with kernel 2.6.11.4.

Furthermore SWI-Prolog 5.0.10 was used.

With the results of these calls a table was built with the following columns:

  1 ~nop~ = number of predicates of the test-query

  2 ~functor~ = the functor of the test-clause that leads to the evaluation
of the test-query

  3 ~on/off~ = indicating, of ~immPathCreation(on)~ or ~immPathCreation(off)~
was called beforehand; 1 means ~on~, 0 means ~off~

  4 ~non~ = number of nodes created

  5 ~nope~ = number of plan edges created

  6 ~ctimes~ = times needed for either creating the POG and searching the
shortest path through it or creating the shortest path immediately; five tests
cause five times

  7 ~ttimes~ = times needed for calculating the overall (total) result; five
tests cause five times

  8 ~actime~ = the average of the five ~ctimes~ (Measurement results that are
obviously out of bound are not considered.)

  9 ~attime~ = the average of the five ~ttimes~ (Measurement results that are
obviously out of bound are not considered.)

A last column headed ~impactime~ gives the improvement of the average time, that
is needed for either creating the POG and searching the shortest path through it
or creating the shortest path immediately (~actime~), hence compares the calls
of similar test-clauses when once ~immPathCreation~ is set to ~on~ and once to
~off~. The given values have to be interpreted as follows: The average time that
needed the originally implemented algorithm for creating the POG and searching
the shortest path through it is set to 100 per cent. If the given
~impacttime~-value is n, then the implementation of the modified algorithm on
average only needs (100-n) per cent of it, hence n per cent of the originally
needed time is cut down.
In other words: An 'improvement' of n per cent means, that n per cent
of the time needed for creating the POG and searching the shortest path through
it is not needed, if one immediately creates the shortest path, hence one has
achieved a reduction of time of n per cent. (An improvement/reduction of 50 for
example means, that only half of the original time is still needed, an
improvement of 95, that only 5 per cent is still needed.)

In a first survey mainly typical every-day-life-queries were used, that had also
been used in other contexts for testing and explaining the Secondo-system. (The
results were already impressively.) In a subsequent survey more complex queries
with a lot more predicates have been tested.

2.6.2 The Test-Clauses

The following clauses are identical to the clauses ~example14~/2,
~example15~/2, ~example16~/2, ~example20~/2 and ~example21~/2 from the file
~optimizer.pl~. They are listed here just for a better clarity.

*/

test1(Query, Cost) :- optimize(
  select * from [staedte as s, plz as p] 
  where [p:ort = s:sname, p:plz > 40000,  (p:plz mod 5) = 0],
  Query, Cost
  ).

test2(Query, Cost) :- optimize(
  select * from staedte where bev > 500000,
  Query, Cost
  ).

test3(Query, Cost) :-  optimize(
  select * from [staedte as s, plz as p] 
  where [s:sname = p:ort, p:plz > 40000],
  Query, Cost
  ).

test4(Query, Cost) :- optimize(
  select *
  from [staedte as s, plz as p]
  where [
    p:ort = s:sname,
    p:plz > 40000,
    s:bev > 300000],
  Query, Cost
  ).

test5(Query, Cost) :- optimize(
  select *
  from [staedte, plz as p1, plz as p2, plz as p3]
  where [
    sname = p1:ort,
    p1:plz = p2:plz + 1,
    p2:plz = p3:plz * 5],
  Query, Cost
  ).

/*

The following clause is identical to ~example18~/2 without the
predicate 'bev is greater than 300.000'.

*/

test6(Query, Cost) :- optimize(
  select *
  from [staedte, plz as p1]
  where [
    sname = p1:ort,
    bev < 500000,
    p1:plz > 50000,
    p1:plz < 60000,
    kennzeichen starts "W",
    p1:ort contains "burg",
    p1:ort starts "M"],
  Query, Cost
  ).

/*

Further tests:

*/

test7(Query, Cost) :- optimize(
  select [sname, bev]
  from [staedte, plz as p1]
  where [
    p1:ort starts "M",
    kennzeichen starts "W",
    p1:ort contains "burg",
    bev < 500000,
    p1:plz > 50000,
    p1:plz < 60000,
    sname = p1:ort
    ]
  orderby [p1:ort asc, p1:plz desc],
  Query, Cost
  ).

test8(Query, Cost) :- optimize(
  select [sname, bev]
  from staedte 
  where [bev>270000, sname starts "S"],
  Query, Cost
  ).

test9(Query, Cost) :- optimize(
  select * 
  from [orte as o, plz as p] 
  where [o:ort = p:ort, o:ort contains "dorf", (p:plz mod 13) = 0],
  Query, Cost
  ).

test10(Query, Cost) :- optimize(
  select [sname, bev div 100 as bevintausend]
  from staedte,
  Query, Cost
  ).

test11(Query, Cost) :- optimize(
  select [o:ort, p1:plz, p2:plz] 
  from [orte as o, plz as p1, plz as p2] 
  where [
    o:ort = p1:ort, 
    p2:plz = (p1:plz +1), 
    o:ort contains "dorf"] 
  orderby [o:ort asc, p2:plz desc],
  Query, Cost
  ).

test12(Query, Cost) :- optimize(
  select * 
  from [orte as o, plz as p] 
  where [o:ort = p:ort, p:plz = 44225],
  Query, Cost
  ).

test13(Query, Cost) :- optimize(
  select * 
  from [orte as o, plz as p] 
  where [
    o:ort = p:ort, 
    p:plz > 50000,
    o:ort contains "dorf"],
  Query, Cost
  ).

test14(Query, Cost) :- optimize(
  select count(*) 
  from [orte as o, plz as p1, plz as p2] 
  where [
    o:ort = p1:ort, 
    p2:plz = p1:plz + 1, 
    (p2:plz mod 5) = 0,
    p1:plz > 50000,
    o:ort contains "dorf"],
  Query, Cost
  ).

test15(Query, Cost) :- optimize(
  select [ort, min(plz) as minplz, 
          max(plz) as maxplz, count(*) as cntplz] 
  from plz
  where plz > 40000
  groupby ort
  orderby cntplz desc
  first 10,
  Query, Cost
  ).

test16(Query, Cost) :- optimize(
  select *
  from [staedte, plz as p1, orte as o]
  where [
    sname = p1:ort,
    o:ort = p1:ort,
    bev < 500000,
    p1:plz > 50000,
    p1:plz < 60000,
    kennzeichen starts "W",
    p1:ort contains "burg",
    p1:ort starts "M"],
  Query, Cost
  ).

/*

The following three clauses are identical to the clauses ~example17~/2,
~example18~/2 and ~example19~/2 from the file ~optimizer.pl~. They are listed
here just for a better clarity.

~test17~, ~test21~, ~test22~, ~test23~ and ~test25~ cause an 'Out of local
stack'-error, if ~immPathCreation~ is set to ~off~ (i. e. the originally
implemented algorithm is used). Thus Prolog has to be initialized with a bigger
local stack. For ~test17~ and ~test22~ a local stack of 4 MB is fine enough, for
~test23~ a local stack of 8 MB is needed, but for ~test21~ and ~test25~ even a
local stack of 16 MB is still too small.

For to use bigger local stacks please don't use the command ~SecondoPL~, but
~SecondoPL -L4M~ when for example the stack should be 4 MB.
Alternatively it is possible to use the command ~pl -L4M~. In this case the
following calls are necessary afterwards:

----
['calloptimizer.pl'].
['modifications.pl'].
----

*/

test17(Query, Cost) :- optimize(
  select *
  from [staedte, plz as p1, plz as p2, plz as p3]
  where [
    sname = p1:ort,
    p1:plz = p2:plz + 1,
    p2:plz = p3:plz * 5,
    bev > 300000,
    bev < 500000,
    p2:plz > 50000,
    p2:plz < 60000,
    kennzeichen starts "W",
    p3:ort contains "burg",
    p3:ort starts "M"],
  Query, Cost
  ).

test18(Query, Cost) :- optimize(
  select *
  from [staedte, plz as p1]
  where [
    sname = p1:ort,
    bev > 300000,
    bev < 500000,
    p1:plz > 50000,
    p1:plz < 60000,
    kennzeichen starts "W",
    p1:ort contains "burg",
    p1:ort starts "M"],
  Query, Cost
  ).

test19(Query, Cost) :- optimize(
  select *
  from [staedte, plz as p1, plz as p2]
  where [
    sname = p1:ort,
    p1:plz = p2:plz + 1,
    bev > 300000,
    bev < 500000,
    p1:plz > 50000,
    p1:plz < 60000,
    kennzeichen starts "W",
    p1:ort contains "burg",
    p1:ort starts "M"],
  Query, Cost
  ).

test20(Query, Cost) :- optimize(
  select *
  from [staedte, orte as o, plz as p]
  where [
  sname = p:ort,
  p:ort = o:ort,
  sname = o:ort,
  plz = p:plz,
  vorwahl = o:vorwahl,
  kennzeichen = o:kennzeichen,
  bev * 1000 = o:bevt,
  kennzeichen starts "A"
  ],
  Query, Cost
  ).

test21(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  s:plz = p:plz,
  s:sname = o:ort,
  p:ort starts "M",
  p:ort contains "burg",
  s:sname contains "burg",
  s:bev > 300000,
  s:bev < 500000,
  s:bev * 1000 = o:bevt,
  s:kennzeichen starts "A",
  o:kennzeichen starts "A",
  p:plz > 50000,
  p:plz < 60000,
  s:plz > 50000,
  s:plz < 60000
  ],
  Query, Cost
  ).

test22(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  s:plz = p:plz,
  s:sname = o:ort,
  p:ort starts "M",
  p:ort contains "burg",
  s:bev > 300000,
  s:bev < 500000,
  s:bev * 1000 = o:bevt,
  s:kennzeichen starts "A",
  p:plz > 50000,
  p:plz < 60000
  ],
  Query, Cost
  ).

test23(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  s:plz = p:plz,
  s:sname = o:ort,
  s:bev * 1000 = o:bevt,
  s:vorwahl = o:vorwahl,
  s:kennzeichen = o:kennzeichen,
  o:ort = p:ort,
  p:ort starts "M",
  p:ort contains "burg",
  s:sname contains "burg",
  s:bev > 300000,
  s:bev < 500000
  ],
  Query, Cost
  ).

test24(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  s:vorwahl = o:vorwahl,
  s:kennzeichen = o:kennzeichen,
  o:ort = p:ort,
  p:ort starts "M",
  p:ort contains "burg",
  s:sname contains "burg",
  s:bev > 300000,
  s:bev < 500000,
  s:kennzeichen starts "A"
  ],
  Query, Cost
  ).

test25(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  s:plz = p:plz,
  s:sname = o:ort,
  s:bev * 1000 = o:bevt,
  s:vorwahl = o:vorwahl,
  s:kennzeichen = o:kennzeichen,
  o:ort = p:ort,
  p:ort starts "M",
  p:ort contains "burg",
  s:sname contains "burg",
  s:bev > 300000,
  s:bev < 500000,
  s:kennzeichen starts "A",
  o:kennzeichen starts "A",
  p:plz > 50000,
  p:plz < 60000,
  s:plz > 50000,
  s:plz < 60000
  ],
  Query, Cost
  ).

/*

2.6.3 The Results

In a first survey mainly typical every-day-life-queries were used, that had
also been used in other contexts for testing and explaining the Secondo-system.
The results in detail can be found in the file ~ResultFirstSurvey.html~,
attached to this file. They are really impressively.

The assumption is, that an improvement (reduction of time) is significant, if it
is at least five per cent. Thus in distinctly most of the test-cases the
improvement of this modification is significant. There is only one case
(~test8~) where the improvement is significantly negative. (This is tested
several times.) In ~test2~ with one predicate the improvement is also negative
(but not significantly, following the definition for significance made above).

One can easily see that the improvement is the better, the more predicates
are in a query - up to more than 70 per cent in ~test6~ and ~test7~ with seven
predicates each.

In a subsequent survey more complex queries with more predicates were
tested. The results can be found in the file ~ResultSecondSurvey.html~, also
attached to this file. Now the time-reduction is up to approximately 90 per cent
when there are eleven predicates in a query (~test23~) and often more than 75 per
cent. Unfortunately it is not possible to evaluate queries with up to fourteen
or seventeen predicates (~test21~ and ~test25~) - neither with the original
optimizer, nor with the modified one. (They ares just evaluated, if the
Prolog-interpreter is initialized with both a local and a global stack of 32
MB; see chapter 2.7.3.) The result is an

----
ERROR: Out of local stack
----

In nearly every case in the second survey the first of the five measurement
results is not considered because they are obviously out of bound. The reason
for this should be the clause ~sweepKnowledgeBase~. This clause deletes all
the nodes, edges, plan edges and so on, that are still stored in the knowledge-base. And there are just a few, if the formerly called test-clause represents
a query with just a few predicates, or a lot, if the formerly called test-clause
represents a query with many predicates. This hasnt't been noticeable in the first
survey, because there haven't been test-clauses with that much predicates.

2.7 Testing how many Predicates are evaluated at most

2.7.1 The Procedure

As shown above, the modified optimizer evaluates queries with up to
eleven predicates without any problems, but ~test21~ and ~test25~
containing more predicates cause errors. Thus it is an interesting
question, how many predicates the optimizer is able to process,
i. e. what is the maximum of predicates it is able to evaluate.

For this purpose the following test-clauses were built and the
results of their callings inserted in a table, which can be found
in the file ~MaxPredicatesTest.html~, attached to this file.

After first calling ~immPathCreation(off, time~) and second
~immPathCreation(on, time~) in both cases the following goals were
called, successively replacing the ~X~ with the numbers from 26 up
to 35.

----
sweepKnowledgeBase(_), checkCalcTime(testX(A,B)),
countNodes, countPlanEdges.
----

If an error-message occurred, first the local and if neccessary also
the global stack of Prolog was extended.

The set-up of the table of results is similar to the one of the
tables contained in ~ResultFirstSurvey.html~ and
~ResultSecondSurvey.html~. For details please consider chapter 2.6.1.

2.7.2 The Test-Clauses

The following five clauses contain the predicates of ~test25~, the
first one eleven, the second one twelve, the third one thirteen and
the fourth and fifth one fourteen each.

*/

test26(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  s:plz = p:plz,
  s:sname = o:ort,
  s:bev * 1000 = o:bevt,
  s:vorwahl = o:vorwahl,
  s:kennzeichen = o:kennzeichen,
  o:ort = p:ort,
  p:ort starts "M",
  p:ort contains "burg",
  s:sname contains "burg",
  s:bev > 300000,
  s:plz < 60000
  ],
  Query, Cost
  ).

test27(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  s:plz = p:plz,
  s:sname = o:ort,
  s:bev * 1000 = o:bevt,
  s:vorwahl = o:vorwahl,
  s:kennzeichen = o:kennzeichen,
  o:ort = p:ort,
  p:ort starts "M",
  p:ort contains "burg",
  s:sname contains "burg",
  s:bev > 300000,
  s:bev < 500000,
  s:plz < 60000
  ],
  Query, Cost
  ).

test28(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  s:plz = p:plz,
  s:sname = o:ort,
  s:bev * 1000 = o:bevt,
  s:vorwahl = o:vorwahl,
  s:kennzeichen = o:kennzeichen,
  o:ort = p:ort,
  p:ort starts "M",
  p:ort contains "burg",
  s:sname contains "burg",
  s:bev > 300000,
  s:bev < 500000,
  s:kennzeichen starts "A",
  s:plz < 60000
  ],
  Query, Cost
  ).

test29(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  s:plz = p:plz,
  s:sname = o:ort,
  s:bev * 1000 = o:bevt,
  s:vorwahl = o:vorwahl,
  s:kennzeichen = o:kennzeichen,
  o:ort = p:ort,
  p:ort starts "M",
  p:ort contains "burg",
  s:sname contains "burg",
  s:bev > 300000,
  s:bev < 500000,
  s:kennzeichen starts "A",
  o:kennzeichen starts "A",
  s:plz < 60000
  ],
  Query, Cost
  ).

/*

The following clause is nothing more than a modification of ~test29~
just containing the predicates in another order. It is only
evaluated, if Prolog was initialized with a local stack of 4 MB.

*/

test30(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  p:ort starts "M",
  p:ort contains "burg",
  s:sname contains "burg",
  s:bev > 300000,
  s:bev < 500000,
  s:kennzeichen starts "A",
  o:kennzeichen starts "A",
  s:plz = p:plz,
  s:sname = o:ort,
  s:bev * 1000 = o:bevt,
  s:vorwahl = o:vorwahl,
  s:kennzeichen = o:kennzeichen,
  o:ort = p:ort,
  s:plz < 60000
  ],
  Query, Cost
  ).

/*

The following clause is a modification of ~test21~ containing one
predicate less and the other in just another order. It causes an
'Out of local stack'-error even if the local stack of Prolog was set to 16 MB.

*/

test31(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  p:plz > 50000,
  p:plz < 60000,
  s:plz > 50000,
  s:plz < 60000,
  s:plz = p:plz,
  s:sname = o:ort,
  s:kennzeichen starts "A",
  o:kennzeichen starts "A",
  p:ort starts "M",
  p:ort contains "burg",
  s:sname contains "burg",
  s:bev > 300000,
  s:bev < 500000
  ],
  Query, Cost
  ).

/*

The following clause is a modification of ~test21~ containing two
predicates less. It is evaluated if both the local and the global
stack of Prolog is set to 8 MB.

*/

test32(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  s:plz = p:plz,
  s:sname = o:ort,
  s:sname contains "burg",
  s:bev > 300000,
  s:bev < 500000,
  s:bev * 1000 = o:bevt,
  s:kennzeichen starts "A",
  o:kennzeichen starts "A",
  p:plz > 50000,
  p:plz < 60000,
  s:plz > 50000,
  s:plz < 60000
  ],
  Query, Cost
  ).

test33(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  p:ort starts "M",
  p:ort contains "burg",
  s:sname contains "burg",
  s:sname contains "agde",
  s:bev > 300000,
  s:bev < 500000,
  s:kennzeichen starts "A",
  o:kennzeichen starts "A",
  s:plz = p:plz,
  s:sname = o:ort,
  s:bev * 1000 = o:bevt,
  s:vorwahl = o:vorwahl,
  s:kennzeichen = o:kennzeichen,
  o:ort = p:ort,
  s:plz < 60000
  ],
  Query, Cost
  ).

test34(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  p:ort starts "M",
  p:ort contains "burg",
  s:sname contains "burg",
  s:sname contains "agde",
  p:ort contains "agde",
  s:bev > 300000,
  s:bev < 500000,
  s:kennzeichen starts "A",
  o:kennzeichen starts "A",
  s:plz = p:plz,
  s:sname = o:ort,
  s:bev * 1000 = o:bevt,
  s:vorwahl = o:vorwahl,
  s:kennzeichen = o:kennzeichen,
  o:ort = p:ort,
  s:plz < 60000
  ],
  Query, Cost
  ).

/*

The following clause causes an 'Out of local stack'-error
even if the local stack of Prolog was set to 32 MB.

*/

test35(Query, Cost) :- optimize(
  select *
  from [staedte as s, orte as o, plz as p]
  where [
  p:ort starts "M",
  p:ort contains "burg",
  s:sname contains "burg",
  s:sname contains "ag",
  s:sname contains "de",
  p:ort contains "agde",
  s:bev > 300000,
  s:bev < 500000,
  s:kennzeichen starts "A",
  o:kennzeichen starts "A",
  s:plz = p:plz,
  s:sname = o:ort,
  s:bev * 1000 = o:bevt,
  s:vorwahl = o:vorwahl,
  s:kennzeichen = o:kennzeichen,
  o:ort = p:ort,
  s:plz < 60000
  ],
  Query, Cost
  ).

/*

2.7.3 The Results

The results can be found in the file ~MaxPredicatesTest.html~,
attached to this file. Again they show the benefits of the
modification.

But they also show, that it is not possible to give an exact maximum
of predicates, that the optimizer is able to evaluate. It is
somewhere around 15, if one doesn't want to extend the local and the
global stack of the Prolog-interpreter above 16 MB.
But ~test31~ only contains 13 predicates (and ~test21~ 14) and they
are not evaluated.

Just for interest both the local and the global stack were extended
to 32 MB, and in that case even ~test21~, ~test25~ and ~test31~
were evaluated. The results can be found in ~MaxPredicatesTest.html~.

Note that even ~test22~ with only 10 predicates needs
an extended local stack (4 MB), but ~test29~ representing a query
containig 14 predicates doesn't need that. ~test34~ representing a
query containing 16 predicates can be evaluated, if the interpreter
was initialized with both a local and a global stack of 16 MB.

An interesting fact is, that ~test30~, which is identical to
~test29~ just containing the predicates in another order, needs
an extended local stack (4 MB) but ~test29~ doesn't need it - though
both test-clauses cause the creation of 1.015 nodes and 2.551 plan edges.

The boundary of query-complexity, that the optimizer is able to
handle, is either only determined by the numbers of nodes and
plan edges, that have to be created while constructing the cheapest
plan. First ~test29~ and ~test30~ create the same number of nodes and plan
edges. And second one can easily see, that if the original optimizer is used, it
can cope with over 4.000 nodes and over 30.000 plan edges, if both
the local and the global stack of Prolog were set to 16 MB. But
while evaluating ~test31~ just about 3.300 nodes and 6.500 plan edges are
created and the optimizer already needs stacks of 32 MB.

[newpage]

3 Alternative Implementations for the Set REACHED-NODES

3.1 Introduction

Both optimizers, the original one and the modified one realized in
chapter two, have problems to evaluate queries containing a lot of
predicates. They cause an

----
ERROR: Out of local stack
----

Even if the local stack of the Prolog-interpreter is extended, the
optimizer is not able to cope with more than sixteen predicates in a
test-query.

Because the original implementation of the set REACHED-NODES is rather complex
(for details see the file ~boundary.pl~), it was worth trying, if more simple
implementations maybe need less memory-resources. Thus
four alternative and rather simple implementations are realized and
tested in the following chapters.

Please note, that the alternative implementations can only be used
together with the modified optimizer, realized in chapter two, not with
the original one.

  1 Instead of using facts stored in the knowledge-base and additionally a
binary search tree the first implementation uses facts and a sorted list. Please
call ~altRNSImplementation(1)~ to use this implementation for the set of
already reached but not yet ticked off nodes.

  2 In the second implementation the set REACHED-NODES is just a sorted
list. One has to call ~altRNSImplementation(2)~ to use this one.

  3 In the third alternative, which can be chosen by calling
~altRNSImplementation(3)~, just facts stored in the knowledge-base altogether
constitute the set REACHED-NODES.

  4 In the fourth implementation (~altRNSImplementation(4)~) also just facts
are stored in the knowledge-base. But the facts have another shape than those
used in implementation No. 3 and the clauses used for finding the node
with the currently minimal distance to the source-node have been changed.

For to use the original implementation again after using another, one has
to call ~altRNSImplementation(off)~.

3.2 Choosing one of the Implementations

There are four alternative implementations, numbered 1, 2, 3 and 4.
Please call ~altRNSImplementation~ with one of the numbers as its
argument to retract the currently used implementation from the
knowledge-base and to store the chosen alternative implementation
instead.

Call ~altRNSImplementation(off)~ to reconsult the file ~modifications.pl~ and
to consequently store the original implementation for the set REACHED-NODES
again.

*/


altRNSImplementation(off) :-
      !,
      consult('modifications.pl'),
      nl, write('*** From now on the original implementation for'),
      nl, write('*** the set REACHED-NODES is used again.'), nl.

altRNSImplementation(1) :-
      !,
      consult('modifications.pl'),
      printWelcomeRNS,
      retractCurrentRNSImplementation,
      altRNSImplementation1.

altRNSImplementation(2) :-
      !,
      consult('modifications.pl'),
      printWelcomeRNS,
      retractCurrentRNSImplementation,
      altRNSImplementation2.

altRNSImplementation(3) :-
      !,
      consult('modifications.pl'),
      printWelcomeRNS,
      retractCurrentRNSImplementation,
      altRNSImplementation3.

altRNSImplementation(4) :-
      !,
      consult('modifications.pl'),
      printWelcomeRNS,
      retractCurrentRNSImplementation,
      altRNSImplementation4.

altRNSImplementation(_) :-
      nl, write('*** Please choose one of the alternatives 1, 2,'),
      nl, write('*** 3 or 4 - or the original implementation.'), nl.

printWelcomeRNS :-
      nl, write('*** From now on (only) the modified optimizer'),
      nl, write('*** uses an alternative implementation for the'),
          write(' set REACHED-NODES.'),
      nl, write('*** Please call ''altRNSImplementation(off)'' to'),
      nl, write('*** use the original implementation again.'), nl.


retractCurrentRNSImplementation :-
     retract(
	     createEmptyReachedNodesSet(_) :- (_)
	    ),
     retract(
	     putIntoReachedNodesSet(_, _, _) :- (_)
	    ),
     retract(
             isInReachedNodesSet(_, _) :- (_)
	    ),
     retract(
             readAlreadyReachedNode(_, _, _) :- (_)
	    ),
     retract(
             getMinimalDistantNode(_, _, _) :- (_)
	    ),
     retract(
             deleteOutOfReachedNodesSet(_, _, _) :- (_)
	    ),
     retract(
             isEmpty(_) :- (_)
	    ).

/*

3.3 The First Alternative: Facts and a Sorted List

The already reached but not yet ticked off nodes are stored in the
knowledge-base as facts of the shape

----
reachedNode(NodeNo, Distance, Path)
----

Additionally a sorted list (sorted by ~distance~) is used to find
the node with the minimal distance to the source-node faster than
by scanning the facts in the knowledge-base.

*/

altRNSImplementation1 :-
      asserta(
	      createEmptyReachedNodesSet([]) :-
			( deleteAllReachedNodes )
             ),
      asserta(
              putIntoReachedNodesSet(ReachedNodesSet, 
		  node(NodeNo, Distance, Path),
                       NewReachedNodesSet) :-
		( assert(reachedNode(NodeNo, Distance, Path)),
		  putIntoDistanceList(ReachedNodesSet,
		  pair(Distance, NodeNo),
		  NewReachedNodesSet) )
             ),
      asserta(
	      isInReachedNodesSet(_, node(NodeNo, _, _)) :-
		( reachedNode(NodeNo, _, _) )
             ),
      asserta(
	      readAlreadyReachedNode(_, NodeNo, 
		  node(NodeNo, Distance, Path)) :-
		( reachedNode(NodeNo, Distance, Path) )
             ),
      asserta(
	      getMinimalDistantNode(ReachedNodesSet, 
		  node(NodeNo, Distance, Path), 
		  NewReachedNodesSet) :-
		( deleteFirstOfDistanceList(ReachedNodesSet,
		  pair(_, NodeNo),
		  NewReachedNodesSet),
		  retract(reachedNode(NodeNo, Distance, Path)) )
             ),
      asserta(
	      deleteOutOfReachedNodesSet(ReachedNodesSet, 
		  node(NodeNo, Distance, _), 
                       NewReachedNodesSet) :-
		( deleteOutOfDistanceList(ReachedNodesSet,
		  pair(Distance, NodeNo),
		  NewReachedNodesSet),
		  retract(reachedNode(NodeNo, _, _)) )
             ),
      asserta(
	      isEmpty([]) :-
		( dc(immPath, (write('There are no more reached '),
		               write('but not yet ticked off nodes.'), nl)),
		  true )
             ).

:- dynamic reachedNode/3.

deleteAllReachedNodes :-
	not(deleteNextReachedNode).

deleteNextReachedNode :-
	retract(reachedNode(_, _, _)),
	fail.

putIntoDistanceList([], Element, [Element]).

putIntoDistanceList(InList, pair(Distance, NodeNo), OutList) :-
	InList = [pair(FirstDistance, _)|_],
	% if
	Distance =< FirstDistance,
	!,
	% then
	OutList = [pair(Distance, NodeNo)|InList].
	
putIntoDistanceList([Element|InListRemainder], 
		pair(Distance, NodeNo),
		[Element|OutListRemainder]) :-
	% else
	putIntoDistanceList(InListRemainder, 
			    pair(Distance, NodeNo),
			    OutListRemainder).
	
deleteFirstOfDistanceList([pair(_, NodeNo)|ListRemainder],
			  pair(_, NodeNo),
			  ListRemainder).

deleteOutOfDistanceList([], _, []).

deleteOutOfDistanceList([Element|InListRemainder], 
			Element, InListRemainder) :-
	!.

deleteOutOfDistanceList([OtherElement|InListRemainder],
			Element, [OtherElement|OutListRemainder]) :-
	deleteOutOfDistanceList(InListRemainder, 
				Element, OutListRemainder).

/*

3.4 The Second Alternative: Just a Sorted List

The set REACHED-NODES is just a sorted list.

*/

altRNSImplementation2 :-
      asserta(
	      createEmptyReachedNodesSet([]) :-
			( true )
             ),
      asserta(
              putIntoReachedNodesSet(ReachedNodesSet, 
			  node(NodeNo, Distance, Path),
                          NewReachedNodesSet) :-
			( putIntoNodeList(ReachedNodesSet,
			  node(NodeNo, Distance, Path),
			  NewReachedNodesSet) )
             ),
      asserta(
	      isInReachedNodesSet(ReachedNodesSet, Node) :-
			( member(Node, ReachedNodesSet) )
             ),
      asserta(
	      readAlreadyReachedNode(ReachedNodesSet, NodeNo, 
			  node(NodeNo, Distance, Path)) :-
			( member(node(NodeNo, Distance, Path),
			  ReachedNodesSet) )
             ),
      asserta(
	      getMinimalDistantNode(ReachedNodesSet,
		      	  node(NodeNo, Distance, Path),
		      	  NewReachedNodesSet) :-
			( deleteFirstOfNodeList(ReachedNodesSet,
			  node(NodeNo, Distance, Path),
			  NewReachedNodesSet) )
             ),
      asserta(
	      deleteOutOfReachedNodesSet(ReachedNodesSet, 
			  node(NodeNo, _, _),
                          NewReachedNodesSet) :-
			( deleteOutOfNodeList(ReachedNodesSet,
			  node(NodeNo, _, _),
			  NewReachedNodesSet) )
             ),
      asserta(
	      isEmpty([]) :-
			( dc(immPath, (write('There are no more reached '),
			               write('but not yet ticked off nodes.'),
        		               nl)),
			  true )
             ).

putIntoNodeList([], Element, [Element]).

putIntoNodeList(InList, node(NodeNo, Distance, Path), OutList) :-
	InList = [node(_, FirstDistance, _)|_],
	% if
	Distance =< FirstDistance,
	!,
	% then
	OutList = [node(NodeNo, Distance, Path)|InList].
	
putIntoNodeList([Element|InListRemainder], 
		node(NodeNo, Distance, Path),
		[Element|OutListRemainder]) :-
	% else
	putIntoNodeList(InListRemainder,
			    node(NodeNo, Distance, Path), 
			    OutListRemainder).

deleteFirstOfNodeList([node(NodeNo, Distance, Path)|ListRemainder],
			  node(NodeNo, Distance, Path),
			  ListRemainder).

deleteOutOfNodeList([], _, []).

deleteOutOfNodeList([Element|InListRemainder], 
		     Element, InListRemainder) :-
	!.

deleteOutOfNodeList([OtherElement|InListRemainder],
			Element, [OtherElement|OutListRemainder]) :-
	deleteOutOfNodeList(InListRemainder, 
			    Element, OutListRemainder).

/*

3.5 The Third Alternative: Just Facts in the Knowledge-Base

The already reached but not yet ticked off nodes are stored in the
knowledge-base as facts of the shape

----
reachedNode(NodeNo, Distance, Path)
----

Hence it is rather costly to find the node with the minimal distance
to the source-node.

*/

altRNSImplementation3 :-
      asserta(
	      createEmptyReachedNodesSet(empty) :-
			( deleteAllReachedNodes )
             ),
      asserta(
              putIntoReachedNodesSet(_, 
		  node(NodeNo, Distance, Path),
                       full) :-
		( assert(reachedNode(NodeNo, Distance, Path)) )
             ),
      asserta(
	      isInReachedNodesSet(_, node(NodeNo, _, _)) :-
		( reachedNode(NodeNo, _, _) )
             ),
      asserta(
	      readAlreadyReachedNode(_, NodeNo, 
		  node(NodeNo, Distance, Path)) :-
		( reachedNode(NodeNo, Distance, Path) )
             ),
      asserta(
	      getMinimalDistantNode(_, Node, NewReachedNodesSet) :-
		( getMinimalDistantNode(Node, NewReachedNodesSet) )
             ),
      asserta(
	      deleteOutOfReachedNodesSet(_, node(NodeNo, _, _),
                  NewReachedNodesSet) :-
		( retract(reachedNode(NodeNo, _, _)),
		  checkReachedNodes(NewReachedNodesSet) )
             ),
      asserta(
	      isEmpty(empty) :-
		( dc(immPath, (write('There are no more reached '),
		               write('but not yet ticked off nodes.'),
        	               nl)),
		  true )
             ).

getMinimalDistantNode(node(ResNodeNo, ResDistance, ResPath),
			NewReachedNodesSet) :-
	dc(immPath, (write('Searching the node with the minimal distance '),
	             write('to the source-node.'), nl)),
	findall(reachedNode(NodeNo, Distance, Path),
		reachedNode(NodeNo, Distance, Path),
		ReachedNodes),
	findMinimalDistantNode(ReachedNodes,
		reachedNode(ResNodeNo, ResDistance, ResPath)),
	retract(reachedNode(ResNodeNo, ResDistance, ResPath)),
	dc(immPath, (write('Found the node with the minimal distance.'), nl)),
	checkReachedNodes(NewReachedNodesSet).

findMinimalDistantNode([Node], Node) :-
	!.

findMinimalDistantNode([reachedNode(_, Distance1,_)|Remainder],
		       MinDistNode) :-
	findMinimalDistantNode(Remainder, 
		       reachedNode(NodeNo2, Distance2, Path2)),
	% if
	Distance1 >= Distance2,
	!,
	% then
	MinDistNode = reachedNode(NodeNo2, Distance2, Path2).

findMinimalDistantNode([Node|_], Node).
	% else

checkReachedNodes(full) :-
	% if
	reachedNode(_, _, _),
	!.

checkReachedNodes(empty).
	% else

/*

3.6 The Fourth Alternative: Also just Facts in the Knowledge-Base

This alternative implementation is just some kind of modification of the
implementation No. 3.
Two changes have been made: First the already reached but not yet ticked off
nodes are stored in the knowledge-base as facts of the shape

----
reachedNode(Distance, NodeNo, Path)
----

~Distance~ becomes the first element of this structure, because the
Prolog-interpreter automatically builds an index regarding the first element.

Second instead of a list as a means for to find the nodes with the minimal
distance to the source-node, a direct search of the knowledge-base takes place.

*/

altRNSImplementation4 :-
      asserta(
	      createEmptyReachedNodesSet(empty) :-
			( deleteAllReachedNodes )
             ),
      asserta(
              putIntoReachedNodesSet(_,
		  node(NodeNo, Distance, Path),
                       full) :-
		( assert(reachedNode(Distance, NodeNo, Path)) )
             ),
      asserta(
	      isInReachedNodesSet(_, node(NodeNo, _, _)) :-
		( reachedNode(_, NodeNo, _) )
             ),
      asserta(
	      readAlreadyReachedNode(_, NodeNo,
		  node(NodeNo, Distance, Path)) :-
		( reachedNode(Distance, NodeNo, Path) )
             ),
      asserta(
	      getMinimalDistantNode(_, Node, NewReachedNodesSet) :-
		( getMinDistNode(Node, NewReachedNodesSet) )
             ),
      asserta(
	      deleteOutOfReachedNodesSet(_, node(NodeNo, _, _),
                  NewReachedNodesSet) :-
		( retract(reachedNode(_, NodeNo, _)),
		  checkReachedNodesSet(NewReachedNodesSet) )
             ),
      asserta(
	      isEmpty(empty) :-
		( dc(immPath, (write('There are no more reached '),
		               write('but not yet ticked off nodes.'),
                               nl)),
		  true )
             ).

getMinDistNode(node(ResNodeNo, ResDistance, ResPath),
	       NewReachedNodesSet) :-
	dc(immPath, (write('Searching the node with the minimal distance '),
	             write('to the source-node.'), nl)),
	reachedNode(Distance, NodeNo, Path),
	findMinDistNode(reachedNode(Distance, NodeNo, Path),
		     reachedNode(ResDistance, ResNodeNo, ResPath)),
	dc(immPath, (write('Found the node with the minimal distance.'), nl)),
	retract(reachedNode(ResDistance, ResNodeNo, ResPath)),
	checkReachedNodesSet(NewReachedNodesSet).

findMinDistNode(reachedNode(Distance1, _, _), MinDistNode) :-
	% if
	checkReachedNodesSet(Status),
	Status == full,
	reachedNode(Distance, NodeNo, Path),
	Distance < Distance1,
	% then
	findMinDistNode(reachedNode(Distance, NodeNo, Path),
			       MinDistNode).

findMinDistNode(Node, Node).
	% else

checkReachedNodesSet(full) :-
	% if
	reachedNode(_, _, _),
	!.

checkReachedNodesSet(empty).
	% else

/*

In theory the ~findMinDistNode~-clauses are not very efficient, because they
compare every fact with every other (and with itself), thus for n facts there
are "n^{2}"[1] camparisons. But the advantage of the clauses is, that they
avoid any kind of container for the facts (like a list in the 'findall and
search the cheapest element'-approach; see chapter 3.5). Thus their need for
stack-space is hopefully not that big as if a container were used.

3.7 Testing the Alternative Implementations

3.7.1 The Procedure

After switching over from one to another alternative implementation, the
following goals have been called

----
sweepKnowledgeBase(_), checkCalcTime(testX(A,B)),
checkCalcTime(testX(C,D)).
----

successively replacing the ~X~ with the numbers from 1 upwards.

3.7.2 The Results

All the results can be found in the file ~AltRNSImplemComparison.html~,
attached to this file.

The first thing that one may notice, if he has a look at the times, needed for
constructing the shortest path while the original implementation for
the set REACHED-NODES is used, is, that they are better (i. e. smaller) in many
cases, than they were in the surveys, carried out in chapter 2.6
(see ~ResultFirstSurvey.html~ and ~ResultSecondSurvey.html~). The reason for
this should be, that here ~sweepKnowledgeBase~ is called in advance. This
probably has a distinct effect on every ~ctime1~ because the call
~sweepKnowledgeBase~ within the clause ~immPathCreation~ can then be processed
without any effort.

A somehow surprising fact is, that the implementations No. 1 (facts in the
knowledge-base and additionally a sorted list) and No. 2 (just a sorted list)
are not really worse than the original implementation (facts in the
knowledge-base and additionally a binary search tree) as long as queries do
not contain more than ten predicates. For test-queries with
just a few predicates this was expected, but the results show, that the original
implementation is not significantly better until there are ten or more
predicates in a query. For many test-clauses representing queries with less
than ten predicates, often even the simplier implementations cause a better
(i. e. smaller) time needed for constructing the shortest path through the POG.
But the original implementation is clearly the better the more predicates
have to be evaluated and the simplier implementations lose any kind of
competition.

An interesting result is, that the alternative implementation No. 1 (facts in
the knowledge-base and additionally a sorted list) needs more time than No. 2
(just a sorted list) as long as there are less than ten predicates in a query,
but (in most cases) dinstinctly less time, if queries contain more than ten
predicates. Hence the complexity of elements contained in a list does only
have significant consequences, if the list is pretty long.

The simple alternative implementations No. 3 and No. 4
(just facts in the knowledge-base) are
not interesting in any sense. The time needed for evaluating a query is
always bigger than in the other cases. It is true, that the sizes of the
stacks, that needs the Prolog-interpreter are smaller for some test-clauses,
if one uses the alternative implementation No. 3,
but the optimizer always needs a lot more time. The only partly promissing
example is ~test33~, that doesn't need an enhancement of the local- and the
trail-stack, but can be evaluated in an acceptable time. (But the original
implementation of the set REACHED-NODES causes a very much better time
needed for creating the shortest path.)

The aim of the alternative implementations and the survey documented in
~AltRNSImplemComparison.html~ was to test, if maybe more simple structured sets
do not need that much stack-resources and hopefully cause 'Out of local
stack'-errors later than the original implementation. This could not be shown.
Only the clauses ~test30~ and ~test32~ can be evaluated with a smaller local
stack as it is needed while using the original implementation. But on the
other hand it is possible to evaluate ~test21~, ~test25~ and ~test34~ with a
local and a global stack of 32 MB each (~test34~ only needs 16 MB) with the
original implementation, but the others need more stack-resources.

[newpage]

4 Literature

The following list considers only those books and papers, that have
been very useful for learning Prolog and programming the above
described modifications of the original Secondo-optimizer.

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

R. Sedgewick, Algorithmen, 2. Auflage, M[ue]nchen, 2002.

L. Sterling, E. Shapiro, Prolog, Fortgeschrittene 
Programmiertechniken, Bonn, 1988.

W. Weisweber, Prolog, Logische Programmierung in der Praxis, 
Bonn, 1997.

*/

