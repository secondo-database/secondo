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
//[toc] [\tableofcontents]
//[=>] [\verb+=>+]
//[:Section Translation] [\label{sec:translation}]
//[Section Translation] [Section~\ref{sec:translation}]
//[:Section 4.1.1] [\label{sec:4.1.1}]
//[Section 4.1.1] [Section~\ref{sec:4.1.1}]
//[Figure pog1] [Figure~\ref{fig:pog1.eps}]
//[Figure pog2] [Figure~\ref{fig:pog2.eps}]
//[newpage] [\newpage]

[10] A Query Optimizer for Secondo

Ralf Hartmut G[ue]ting, November - December 2002

[toc]

[newpage]

1 Introduction

1.1 Overview

This document not only describes, but ~is~ an optimizer for Secondo database
systems.  It contains the current source code for the optimizer, written in
PROLOG. It can be compiled by a PROLOG system (SWI-Prolog 5.0 or higher)
directly.

The current version of the optimizer is capable of handling conjunctive queries,
formulated in a relational environment. That is, it takes a set of
relations together with a set of selection or join predicates over these
relations and produces a query plan that can be executed by (the current
relational system implemented in) Secondo.

The selection of the query plan is based on cost estimates which in turn are
based on given selectivities of predicates. Selectivities of predicates are
maintained in a table (a set of PROLOG facts). If the selectivity of a predicate
is not available from that table, then an interaction with the Secondo system
should take place to determine the selectivity. There are various strategies
conceivable for doing this which will be described elsewhere. However, the
current version of the optimizer just emits a message that the selectivity is
missing and quits.

The optimizer also implements a simple SQL-like language for entering queries.
The notation is pretty much like SQL except that the lists occurring (lists of
attributes, relations, predicates) are written in PROLOG notation. Also note
that the where-clause is a list of predicates rather than an arbitrary boolean
expression and hence allows one to formulate conjunctive queries only.


1.2 Optimization Algorithm

The optimizer employs an as far as we know novel optimization algorithm which is
based on ~shortest path search in a predicated order graph~. This technique is
remarkably simple to implement, yet efficient.

A predicate order graph (POG) is the graph whose nodes represent sets of
evaluated predicates and whose edges represent predicates, containing all
possible orders of predicates. Such a graph for three predicates ~p~, ~q~, and
~r~ is shown in [Figure pog1].

                Figure 1: A predicate order graph for three predicates ~p~, ~q~
and ~r~  [pog1.eps]

Here the bottom node has no predicate evaluated and the top node has all
predicates evaluated. The example illustrates, more precisely, possible
sequences of selections on an argument relation of size 1000. If selectivities
of predicates are given (for ~p~ its is 1/2, for ~q~ 1/10, and for ~r~ 1/5),
then we can annotate the POG with sizes of intermediate results as shown,
assuming that all predicates are independent (not ~correlated~). This means that
the selectivity of a predicate is the same regardless of the order of
evaluation, which of course does not need to be true.

If we can further compute for each edge of the POG possible evaluation
methods, adding a new ``executable'' edge for each method, and mark the
edge with estimated costs for this method, then finding a shortest path through
the POG corresponds to finding the cheapest query plan. [Figure pog2] shows an
example of a POG annotated with evaluation methods.

                Figure 2: A POG annotated with evaluation methods [pog2.eps]

In this example, there is only a single method associated with each edge. In
general, however, there will be several methods. The example represents the
query:

----    select *
        from Staedte, Laender, Regiert
        where Land = LName and PName = 'CDU' and LName = PLand
----

for relation schemas

----    Staedte(SName, Bev, Land)
        Laender(LName, LBev)
        Regiert(PName, PLand)
----

Hence the optimization algorithm described and implemented in the following
sections proceeds in the following steps:

  1 For given relations and predicates, construct the predicate order graph and
store it as a set of facts in memory (Sections 2 through 4).

  2 For each edge, construct corresponding executable edges (called ~plan edges~
below). This is controlled by optimization rules describing how selections or
joins can be translated (Sections 5 and 6).

  3 Based on sizes of arguments and selectivities (stored in the file
~database.pl~) compute the sizes of all intermediate results. Also annotate
edges of the POG with selectivities (Section 7).

  4 For each plan edge, compute its cost and store it in memory (as a set of
facts). This is based on sizes of arguments and the selectivity associated with
the edge and on a cost function (predicate) written for each operator that may
occur in a query plan (Section 8).

  5 The algorithm for finding shortest paths by Dijkstra is employed to find a
shortest path through the graph of plan edges annotated with costs (called ~cost
edges~). This path is transformed into a Secondo query plan and returned
(Section 9).

  6 Finally, a simple subset of SQL in a PROLOG notation is implemented. So it
is possible to enter queries in this language. The optimizer determines from it
the lists of relations and predicates in the form needed for constructing the
POG, and then invokes step 1 (Section 11).



2 Data Structures

In the construction of the predicate order graph, the following data structures
are used.

----    pr(P, A)
        pr(P, B, C)
----

A selection or join predicate, e.g. pr(p, a), pr(q, b, c). Means a
selection predicate p on relation a, and a join predicate q on relations
b and c.

----    arp(Arg, Rels, Preds)
----

An argument, relations, predicate triple. It describes a set of relations
~Rels~ on which the predicates ~Preds~ have been evaluated. To access the
result of this evaluation one needs to refer to ~Arg~.

Arg is either arg(N) or res(N), N an integer. Examples: arg(5), res(1)

Rels is a list of relation names, e.g. [a, b, c]

Preds is a list of predicate names, e.g. [p, q, r]


----    node(No, Preds, Partition)
----

A node.

~No~ is the number of the node into which the evaluated predicates
are encoded (each bit corresponds to a predicate number, e.g. node number
5 = 101 (binary) says that the first predicate (no 1) and the third
predicate (no 4) have been evaluated in this node. For predicate i,
its predicate number is "2^{i-1}"[1].

~Preds~ is the list of names of evaluated predicates, e.g. [p, q].

~Partition~ is a list of arp elements, see above.


----    edge(Source, Target, Term, Result, Node, PredNo)
----

An edge, representing a predicate.

~Source~ and ~Target~ are the numbers of source and target nodes in the
predicate order graph, e.g. 0 and 1.

~Term~ is either a selection or a join, for example,
select(arg(0), pr(p, a) or join(res(4), res(1), pr(q, a, b))

~Result~ is the number of the node into which the result of this predicate
application should be written. Normally it is the same as Target,
but for an edge leading to a node combining several independent results,
it is the number of the ``real'' node to obtain this result. An example of this can
be found in [Figure pog2] where the join edge leading from node 3 to node 7 does
not use the result of node 3 (there is none) but rather the two independent
results from nodes 1 and 2 (this pair is conceptually the result available in
node 3).

~Node~ is the source node for this edge, in the form node(...) as
described above.

~PredNo~ is the predicate number for the predicate represented by this
edge. Predicate numbers are of the form "2^i" as explained
for nodes.

3 Construction of the Predicate Order Graph

3.1 pog

----    pog(Rels, Preds, Nodes, Edges) :-
----

For a given list of relations ~Rels~ and predicates ~Preds~, ~Nodes~ and
~Edges~ are the predicate order graph where edges are annotated with selection
and join operations applied to the correct arguments. Moreover, for each edge possible
translations into terms of the executable algebra are computed by the rule ~createPlanEdges~.

Example call:

----    pog([staedte, laender], [pr(p, staedte), pr(q, laender), pr(r, staedte,
        laender)], N, E).
----

*/

:- dynamic(highNode/1),
   dynamic(resultSize/2),
   dynamic(resultTupleSize/2).

usingVersion(standard).

pog(Rels, Preds, Nodes, Edges) :-
  length(Rels, N), reverse(Rels, Rels2), deleteArguments,
  partition(Rels2, N, Partition0),
  length(Preds, M), reverse(Preds, Preds2),
  pog2(Partition0, M, Preds2, Nodes, Edges),
  deleteNodes, storeNodes(Nodes),
  deleteEdges, storeEdges(Edges),
  deletePlanEdges, deleteVariables, createPlanEdges,
  HighNode is 2**M -1,
  retract(highNode(_)), assert(highNode(HighNode)),
  % uncomment next line for debugging
  % showpog(Rels, Preds),
  deleteSizes,
  deleteCostEdges.

showpog(Rels, Preds) :-
  nl, write('Rels: '), write(Rels),
  nl, write('Preds: '), write(Preds), 
  nl, nl, write('Nodes: '), nl, writeNodes,
  nl, nl, write('Edges: '), nl, writeEdges,
  nl, nl, write('Plan Edges: '), nl, writePlanEdges.

/*
3.2 partition

----    partition(Rels, N, Partition0) :-
----

Given a list of ~N~ relations ~Rel~, return an initial partition such that
each relation r is packed into the form arp(arg(i), [r], []).

*/

partition([], _, []).

partition([Rel | Rels], N, [Arp | Arps]) :-
  N1 is N-1,
  Arp = arp(arg(N), [Rel], []),
  assert(argument(N, Rel)),
  partition(Rels, N1, Arps).


/*
3.3 pog2

----    pog2(Partition0, NoOfPreds, Preds, Nodes, Edges) :-
----

For the given start partition ~Partition0~, a list of predicates ~Preds~
containing ~NoOfPred~ predicates, return the ~Nodes~ and ~Edges~ of the 
predicate order graph.

*/

pog2(Part0, _, [], [node(0, [], Part0)], []).

pog2(Part0, NoOfPreds, [Pred | Preds], Nodes, Edges) :-
  N1 is NoOfPreds-1,
  PredNo is 2**N1,
  pog2(Part0, N1, Preds, NodesOld, EdgesOld),
  newNodes(Pred, PredNo, NodesOld, NodesNew),
  newEdges(Pred, PredNo, NodesOld, EdgesNew),
  copyEdges(Pred, PredNo, EdgesOld, EdgesCopy),
  append(NodesOld, NodesNew, Nodes),
  append(EdgesOld, EdgesNew, Edges2),
  append(Edges2, EdgesCopy, Edges).

/*
3.4 newNodes

----    newNodes(Pred, PredNo, NodesOld, NodesNew) :-
----

Given a predicate ~Pred~ with number ~PredNo~ and a list of nodes ~NodesOld~
resulting from evaluating all predicates with lower numbers, construct
a list of nodes which result from applying to each of the existing nodes
the predicate ~Pred~.

*/

newNodes(_, _, [], []).

newNodes(Pred, PNo, [Node | Nodes], [NodeNew | NodesNew]) :-
  newNode(Pred, PNo, Node, NodeNew),
  newNodes(Pred, PNo, Nodes, NodesNew).

newNode(Pred, PNo, node(No, Preds, Part), node(No2, [Pred | Preds], Part2)) :-
  No2 is No + PNo,
  copyPart(Pred, PNo, Part, Part2).

/*
3.5 copyPart

----    copyPart(Pred, PNo, Part, Part2) :-
----

copy the partition ~Part~ of a node so that the new partition ~Part2~
after applying the predicate ~Pred~ with number ~PNo~ results.

This means that for a selection predicate we have to find the arp
containing its relation and modify it accordingly, the other arps
in the partition are copied unchanged.

For a join predicate we have to find the two arps containing its 
two relations and to merge them into a single arp; the remaining 
arps are copied unchanged.

Or a join predicate may find its two relations in the same arp which means
another join on the same two relations has already been performed.

*/

copyPart(_, _, [], []).

copyPart(pr(P, Rel), PNo, Arps, [Arp2 | Arps2]) :-
  select(X, Arps, Arps2),
  X = arp(Arg, Rels, Preds),
  member(Rel, Rels), !,
  nodeNo(Arg, No),
  ResNo is No + PNo,
  Arp2 = arp(res(ResNo), Rels, [P | Preds]).

copyPart(pr(P, R1, R2), PNo, Arps, [Arp2 | Arps2]) :-
  select(X, Arps, Arps2),
  X = arp(Arg, Rels, Preds),
  member(R1, Rels),
  member(R2, Rels), !,
  nodeNo(Arg, No),
  ResNo is No + PNo,
  Arp2 = arp(res(ResNo), Rels, [P | Preds]).

copyPart(pr(P, R1, R2), PNo, Arps, [Arp2 | Arps2]) :-
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
  Arp2 = arp(res(ResNo), Rels, [P | Preds]).

nodeNo(arg(_), 0).
nodeNo(res(N), N).

/*
3.6 newEdges

----    newEdges(Pred, PredNo, NodesOld, EdgesNew) :-
----

for each of the nodes in ~NodesOld~ return a new edge in ~EdgesNew~
built by applying the predicate ~Pred~ with number ~PNo~.

*/

newEdges(_, _, [], []).

newEdges(Pred, PNo, [Node | Nodes], [Edge | Edges]) :-
  newEdge(Pred, PNo, Node, Edge),
  newEdges(Pred, PNo, Nodes, Edges).

newEdge(pr(P, Rel), PNo, Node, Edge) :-
  findRel(Rel, Node, Source, Arg),
  Target is Source + PNo,
  nodeNo(Arg, ArgNo),
  Result is ArgNo + PNo,
  Edge = edge(Source, Target, select(Arg, pr(P, Rel)), Result, Node, PNo).

newEdge(pr(P, R1, R2), PNo, Node, Edge) :-
  findRels(R1, R2, Node, Source, Arg),
  Target is Source + PNo,
  nodeNo(Arg, ArgNo),
  Result is ArgNo + PNo,
  Edge = edge(Source, Target, select(Arg, pr(P, R1, R2)), Result, Node, PNo).

newEdge(pr(P, R1, R2), PNo, Node, Edge) :-
  findRels(R1, R2, Node, Source, Arg1, Arg2),
  Target is Source + PNo,
  nodeNo(Arg1, Arg1No),
  nodeNo(Arg2, Arg2No),
  Result is Arg1No + Arg2No + PNo,
  Edge = edge(Source, Target, join(Arg1, Arg2, pr(P, R1, R2)), Result,
    Node, PNo).


/*
3.7 findRel

----    findRel(Rel, Node, Source, Arg):-
----

find the relation ~Rel~ within a node description ~Node~ and return the
node number ~No~ and the description ~Arg~ of the argument (e.g. res(3)) found
within the arp containing Rel.

----    findRels(Rel1, Rel2, Node, Source, Arg1, Arg2):-
----

similar for two relations.

*/

findRel(Rel, node(No, _, Arps), No, ArgX) :-
  select(X, Arps, _),
  X = arp(ArgX, RelsX, _),
  member(Rel, RelsX).


findRels(Rel1, Rel2, node(No, _, Arps), No, ArgX) :-
  select(X, Arps, _),
  X = arp(ArgX, RelsX, _),
  member(Rel1, RelsX),
  member(Rel2, RelsX).

findRels(Rel1, Rel2, node(No, _, Arps), No, ArgX, ArgY) :-
  select(X, Arps, Rest),
  X = arp(ArgX, RelsX, _),
  member(Rel1, RelsX), !,
  select(Y, Rest, _),
  Y = arp(ArgY, RelsY, _),
  member(Rel2, RelsY).



/*
3.8 copyEdges

----    copyEdges(Pred, PredNo, EdgesOld, EdgesCopy):-
----

Given a set of edges ~EdgesOld~ and a predicate ~Pred~ with number ~PredNo~,
return a copy of each edge in ~EdgesOld~ in ~EdgesNew~ such that the
copied version reflects a previous application of predicate ~Pred~.

This is implemented by retrieving from each old edge its start node,
constructing for this start node and predicate ~Pred~ a target node to
which then the predicate associated with the old edge is applied. 

*/

copyEdges(_, _, [], []).

copyEdges(Pred, PNo, [Edge | Edges], [Edge2 | Edges2]) :-
  Edge = edge(_, _, Term, _, Node, PNo2),
  pred(Term, Pred2),
  newNode(Pred, PNo, Node, NodeNew),
  newEdge(Pred2, PNo2, NodeNew, Edge2),
  copyEdges(Pred, PNo, Edges, Edges2).

pred(select(_, P), P).
pred(join(_, _, P), P).

/*
3.9 writeEdgeList

----    writeEdgeList(List):-
----

Write the list of edges ~List~.

*/

writeEdgeList([edge(Source, Target, Term, _, _, _) | Edges]) :-
  write(Source), write('-'), write(Target), write(':'), write(Term), nl,
  writeEdgeList(Edges).

/*
4 Managing the Graph in Memory

4.1 Storing and Deleting Nodes and Edges

----    storeNodes(NodeList).
        storeEdges(EdgeList).
        deleteNodes.
        deleteEdges.
----

Just as the names say. Store a list of nodes or edges, repectively, as facts; 
and delete them from memory again.

*/

storeNodes([Node | Nodes]) :- assert(Node), storeNodes(Nodes).
storeNodes([]).

storeEdges([Edge | Edges]) :- assert(Edge), storeEdges(Edges).
storeEdges([]).

deleteNode :- retract(node(_, _, _)), fail.
deleteNodes :- not(deleteNode).

deleteEdge :- retract(edge(_, _, _, _, _, _)), fail.
deleteEdges :- not(deleteEdge).

deleteArgument :- retract(argument(_, _)), fail.
deleteArguments :- not(deleteArgument).


/*
4.2 Writing Nodes and Edges

----    writeNodes.
        writeEdges.
----

Write the currently stored nodes and edges, respectively.

*/
writeNode :-
  node(No, Preds, Partition), 
  write('Node: '), write(No), nl,
  write('Preds: '), write(Preds), nl,
  write('Partition: '), write(Partition), nl, nl,
  fail.
writeNodes :- not(writeNode).

writeEdge :-
  edge(Source, Target, Term, Result, _, _),
  write('Source: '), write(Source), nl,
  write('Target: '), write(Target), nl,
  write('Term: '), write(Term), nl,
  write('Result: '), write(Result), nl, nl,
  fail.

writeEdges :- not(writeEdge).

/*
5 Rule-Based Translation of Selections and Joins
[:Section Translation]

5.1 Precise Notation for Input

Since now we have to look into the structure of predicates, and need to be
able to generate Secondo executable expressions in their precise format, we 
need to define the input notation precisely.

5.1.1 The Source Language
[:Section 4.1.1]

We assume the queries can be entered basically as select-from-where 
structures, as follows. Let schemas be given as:

----    plz(PLZ:string, Ort:string)
        Staedte(SName:string, Bev:int, PLZ:int, Vorwahl:string, Kennzeichen:string)
----

Then we should be able to enter queries:

----    select SName, Bev
        from Staedte
        where Bev > 500000
----

In the next example we need to avoid the name conflict for PLZ

----    select *
        from Staedte as s, plz as p
        where s.SName = p.Ort and p.PLZ > 40000
----

In the PROLOG version, we will use the following notations:

----    rel(Name, Var, Case)
----

For example

----    rel(staedte, *, u)
----

is a term denoting the ~Staedte~ relation; ~u~ says that it is actually to be 
written in upper case whereas

----    rel(plz, *, l)
----

denotes the ~plz~ relation to be written in lower case. The second argument
~Var~ contains an explicit variable if it has been assigned, otherwise the 
symbol [*]. If an explicit variable has been used in the query, we need to 
perfom renaming in the plan. For example, in the second query above, the 
relations would be denoted as 

----    rel(staedte, s, u)
        rel(plz, p, l)
----

Within predicates, attributes are annotated as follows:

----    attr(Name, Arg, Case)

        attr(ort, 2, u)
----

This says that  ~ort~ is an attribute of the second argument within a join 
condition, to be written in upper case. For a selection condition, the second 
argument is ignored; it can be set to 0 or 1.

Hence for the two queries above, the translation would be

----    fromwhere(
          [rel(staedte, *, u)], 
          [pr(attr(bev, 0, u) > 500000, rel(staedte, *, u))]
        )

        fromwhere(
          [rel(staedte, s, u), rel(plz, p, l)],
          [pr(attr(s:sName, 1, u) = attr(p:ort, 2, u), 
                rel(staedte, s, u), rel(plz, p, l)),
           pr(attr(p:pLZ, 0, u) > 40000, rel(plz, p, l))]
        )
----

Note that the upper or lower case distinction refers only to the first letter
of a relation or attribute name. Other letters are written on the PROLOG side 
in the same way as in Secondo.

Note further that if explicit variables are used, the attribute name will
include them, e.g. s:sName.

The projection occurring in the select-from-where statement is for the moment 
not passed to the optimizer; it is treated outside.

So example 2 is rewritten as:

*/

example3 :- pog([rel(staedte, s, u), rel(plz, p, l)],
  [pr(attr(p:ort, 2, u) = attr(s:sName, 1, u),
        rel(staedte, s, u), rel(plz, p, l) ),
   pr(attr(p:pLZ, 1, u) > 40000, rel(plz, p, l)),
   pr((attr(p:pLZ, 1, u) mod 5) = 0, rel(plz, p, l))], _, _).

/*
The two queries mentioned above are:

*/

example4 :- pog(
  [rel(staedte, *, u)], 
  [pr(attr(bev, 1, u) > 500000, rel(staedte, *, u))],
  _, _).

example5 :- pog(
  [rel(staedte, s, u), rel(plz, p, l)],
  [pr(attr(s:sName, 1, u) = attr(p:ort, 2, u), rel(staedte, s, u), rel(plz, p,
l)),
   pr(attr(p:pLZ, 1, u) > 40000, rel(plz, p, l))],
  _, _).

/*

5.1.2 The Target Language

In the target language, we use the following operators:

----    feed:           rel(Tuple) -> stream(Tuple)
        consume:        stream(Tuple) -> rel(Tuple)

        filter:         stream(Tuple) x (Tuple -> bool) -> stream(Tuple)
        product:        stream(Tuple1) x stream(Tuple2) -> stream(Tuple3)

                                where Tuple3 = Tuple1 o Tuple2

        symmjoin:       stream(Tuple1) x stream(Tuple2) x 
                                (Tuple 1 x Tuple 2 -> bool) -> stream(Tuple3)

                                where Tuple3 = Tuple1 o Tuple2

        hashjoin:       stream(Tuple1) x stream(Tuple2) x attrname1 x attrname2
                                x nbuckets -> stream(Tuple3)

                                where   Tuple3 = Tuple1 o Tuple2
                                        attrname1 occurs in Tuple1
                                        attrname2 occurs in Tuple2
                                        nbuckets is the number of hash buckets
                                                to be used

        sortmergejoin:  stream(Tuple1) x stream(Tuple2) x attrname1 x attrname2
                                -> stream(Tuple3)

                                where   Tuple3 = Tuple1 o Tuple2
                                        attrname1 occurs in Tuple1
                                        attrname2 occurs in Tuple2
                                        
        spatialjoin:    stream(Tupel1) x stream(Tuple2) x attrname1 x attrname2
                                -> stream(Tuple3)

                                where   Tuple3 = Tuple1 o Tuple2
                                        attrname<n> occurs in Tuple<n>
                                        attribute<n> are MBB Types of
                                           the same dimension

        loopjoin:       stream(Tuple1) x (Tuple1 -> stream(Tuple2)
                                -> stream(Tuple3)

                                where   Tuple3 = Tuple1 o Tuple2
                                
        exactmatch:     btree(Tuple, AttrType) x rel(Tuple) x AttrType
                                -> stream(Tuple)

        extend:         stream(Tuple1) x (Newname x (Tuple -> Attrtype))+
                                -> stream(Tuple2)

                                where   Tuple2 is Tuple1 to which pairs
                                        (Newname, Attrtype) have been appended

        remove:         stream(Tuple1) x Attrname+ -> stream(Tuple2)

                                where   Tuple2 is Tuple1 from which the mentioned
                                        attributes have been removed.

        project:        stream(Tuple1) x Attrname+ -> stream(Tuple2)

                                where   Tuple2 is Tuple1 projected on the
                                        mentioned attributes.

        rename          stream(Tuple1) x NewName -> stream(Tuple2)

                                where   Tuple2 is Tuple1 modified by appending
                                        "_newname" to each attribute name

        count           stream(Tuple) -> int

                                count the number of tuples in a stream

        sortby          stream(Tuple) x (Attrname, asc/desc)+   -> stream(Tuple)

                                sort stream lexicographically by the given
                                attribute names

        groupby stream(Tuple) x GroupAttrs x NewFields -> stream(Tuple2)

                                group stream by the grouping attributes; for each group
                                compute new fields each of which is specified in the
                                form Attrname : Expr. The argument stream must already
                                be sorted by the grouping attributes.
----

In PROLOG, all expressions involving such operators are written in prefix
notation.

Parameter functions are written as

----    fun([param(Var1, Type1), ..., param(VarN, TypeN)], Expr)
----
        

5.1.3 Converting Plans to Atoms and Writing them.

Predicate ~plan\_to\_atom~ converts a plan to a string atom, which represents
the plan as a SECONDO query in text syntax. For attributes we have to
distinguish whether a leading ``.'' needs to be written (if the attribute occurs
within a parameter function) or whether just the attribute name is needed as in
the arguments for hashjoin, for example. Predicate ~wp~ (``write plan'') uses
predicate ~plan\_to\_atom~ to convert its argument to an atom and then writes that
atom to standard output.

*/

upper(Lower, Upper) :-
  atom_chars(Lower, [First | Rest]),
  char_type(First2, to_upper(First)),
  append([First2], Rest, UpperList),
  atom_chars(Upper, UpperList).
  /*atom_codes(Lower, [First | Rest]),
  to_upper(First, First2),
  UpperList = [First2 | Rest],
  atom_codes(Upper, UpperList).*/

wp(Plan) :-
  plan_to_atom(Plan, PlanAtom),
  write(PlanAtom).

/*
Function ~newVariable~ outputs a new unique variable name.
The variable name is unique in the sense that ~newVariable~ never
outputs the same name twice (in a PROLOG session). 
It should be emphasized that the output
is not a PROLOG variable but a variable name to be used for defining
abstractions in the Secondo system.

*/

:- dynamic(varDefined/1).

newVariable(Var) :-
  varDefined(N),
  !,
  N1 is N + 1,
  retract(varDefined(N)),
  assert(varDefined(N1)),
  atom_concat('var', N1, Var).

newVariable(Var) :- 
  assert(varDefined(1)),
  Var = 'var1'.

deleteVariable :- retract(varDefined(_)), fail.

deleteVariables :- not(deleteVariable).


/*
The ~plan\_to\_atom(attr(Name, Arg, Case), Result)~ predicate is not able to distinguish whether 
to return ~.Name~ or ~..Name~ for ~Arg~ = 2. Now, we use a predicate ~consider\_Arg2(T1, T2)~ to
 return a term ~T2~, that is constructed from term ~T1~ by replacing all occurrences of 
~attr(\_, 2, \_)~ in it by ~attr2(\_, 2, \_)~.
 
*/

consider_Arg2(Pred, Pred) :- 
  atomic(Pred).

consider_Arg2(attr(Name, 2, Case), attr2(Name, 2, Case)):- !.                 

consider_Arg2(Term,Term2) :-
  compound(Term),
  Term =.. [Op|Args],
  consider_Arg2_2(Args,Args2),
  Term2 =.. [Op|Args2].


consider_Arg2_2([],[]).

consider_Arg2_2([Me|Others],[Me2|Others2]) :-
  consider_Arg2(Me,Me2),
  consider_Arg2_2(Others,Others2).

/*
Arguments:

*/

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

/*
Lists:

*/


plan_to_atom([X], AtomX) :-
  plan_to_atom(X, AtomX),
  !.

plan_to_atom([X | Xs], Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Xs, XsAtom),
  concat_atom([XAtom, ', ', XsAtom], '', Result),
  !.


/*
Operators: only special syntax. General rules for standard syntax
see below.

*/


plan_to_atom(sample(Rel, S, T), Result) :-
  plan_to_atom(Rel, ResRel),
  concat_atom([ResRel, 'sample[', S, ', ', T, '] '], '', Result),
  !.

plan_to_atom(symmjoin(X, Y, M), Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  consider_Arg2(M, M2),          % transform second arg/3 to arg2/3
  plan_to_atom(M2, MAtom),
  concat_atom([XAtom, YAtom, 'symmjoin[',
    MAtom, '] '], '', Result),
  !.

plan_to_atom(hashjoin(X, Y, A, B, C), Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  plan_to_atom(A, AAtom),
  plan_to_atom(B, BAtom),
  concat_atom([XAtom, YAtom, 'hashjoin[',
    AAtom, ', ', BAtom, ', ', C, '] '], '', Result),
  !.

plan_to_atom(sortmergejoin(X, Y, A, B), Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  plan_to_atom(A, AAtom),
  plan_to_atom(B, BAtom),
  concat_atom([XAtom, YAtom, 'sortmergejoin[',
    AAtom, ', ', BAtom, '] '], '', Result),
  !.

plan_to_atom(spatialjoin(X, Y, A, B, _), Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  plan_to_atom(A, AAtom),
  plan_to_atom(B, BAtom),
  concat_atom([XAtom, YAtom, 'spatialjoin[',
    AAtom, ', ', BAtom, '] '], '', Result),
  !.

plan_to_atom(groupby(Stream, GroupAttrs, Fields), Result) :-
  plan_to_atom(Stream, SAtom),
  plan_to_atom(GroupAttrs, GAtom),
  plan_to_atom(Fields, FAtom),
  concat_atom([SAtom, 'groupby[', GAtom, '; ', FAtom, ']'], '', Result),
  !.

plan_to_atom(extend(Stream, Fields), Result) :-
  plan_to_atom(Stream, SAtom),
  plan_to_atom(Fields, FAtom),
  concat_atom([SAtom, 'extend[', FAtom, '] '], '', Result),
  !.

plan_to_atom(field(NewAttr, Expr), Result) :-
  plan_to_atom(attrname(NewAttr), NAtom),
  plan_to_atom(Expr, EAtom),
  concat_atom([NAtom, ': ', EAtom], '', Result).

/*
Special Operators for Algebras

Picture Algebra:

*/
plan_to_atom(equals(X, Y, A, B), Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  plan_to_atom(A, AAtom),
  plan_to_atom(B, BAtom),
  concat_atom([XAtom, ' ', YAtom, ' equals[', AAtom, ', ', BAtom, ']'], '', Result),
  !.

plan_to_atom(like(X, Y, Z, A, B), Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  plan_to_atom(Z, ZAtom),
  plan_to_atom(A, AAtom),
  plan_to_atom(B, BAtom),
  concat_atom([XAtom, ' like[', YAtom, ', ', ZAtom, ', ', AAtom, ', ', BAtom, ']'], 
  '', Result),
  !.

/*
End of Picture Algebra

*/


plan_to_atom(exactmatchfun(IndexName, Rel, attr(Name, R, Case)), Result) :-
  plan_to_atom(Rel, RelAtom),
  plan_to_atom(a(Name, R, Case), AttrAtom),
  newVariable(T),
  concat_atom(['fun(', T, ' : TUPLE) ', IndexName,
    ' ', RelAtom, 'exactmatch[attr(', T, ', ', AttrAtom, ')] '], Result),
  !.


plan_to_atom(newattr(Attr, Expr), Result) :-
  plan_to_atom(Attr, AttrAtom),
  plan_to_atom(Expr, ExprAtom),
  concat_atom([AttrAtom, ': ', ExprAtom], '', Result),
  !.


plan_to_atom(rename(X, Y), Result) :-
  plan_to_atom(X, XAtom),
  concat_atom([XAtom, '{', Y, '} '], '', Result),
  !.


plan_to_atom(counter(X, Y), Result) :-
  integer(Y), Y > 0, Y < 16,
  plan_to_atom(X, XAtom),
  concat_atom([XAtom, '{', Y, '} '], '', Result),
  !.


plan_to_atom(fun(Params, Expr), Result) :-
  params_to_atom(Params, ParamAtom),
  plan_to_atom(Expr, ExprAtom),
  concat_atom(['fun (', ParamAtom, ') ', ExprAtom], '', Result),
  !.


plan_to_atom(attribute(X, Y), Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  concat_atom(['attr(', XAtom, ', ', YAtom, ')'], '', Result),
  !.
 

plan_to_atom(date(X), Result) :-
  plan_to_atom(X, XAtom),
  concat_atom(['[const instant value ', XAtom, ']'], '', Result),
  !.
  
plan_to_atom(interval(X, Y), Result) :-
  concat_atom(['[const duration value (', X, ' ', Y, ')]'], '', Result),
  !.


/*
Sort orders and attribute names.

*/

plan_to_atom(asc(Attr), Result) :-
  plan_to_atom(Attr, AttrAtom), 
  atom_concat(AttrAtom, ' asc', Result).

plan_to_atom(desc(Attr), Result) :-
  plan_to_atom(Attr, AttrAtom), 
  atom_concat(AttrAtom, ' desc', Result).

plan_to_atom(attr(Name, Arg, Case), Result) :-
  plan_to_atom(a(Name, Arg, Case), ResA),
  atom_concat('.', ResA, Result),
  !.

plan_to_atom(attr2(Name, Arg, Case), Result) :-
  Arg = 2,
  plan_to_atom(a(Name, Arg, Case), ResA),
  atom_concat('..', ResA, Result),
  !.

plan_to_atom(attr2(Name, Arg, Case), Result) :-
  plan_to_atom(a(Name, Arg, Case), ResA),
  atom_concat('.', ResA, Result),
  !.

plan_to_atom(attrname(attr(Name, Arg, Case)), Result) :-
  plan_to_atom(a(Name, Arg, Case), Result).

plan_to_atom(a(A:B, _, l), Result) :-
  concat_atom([B, '_', A], '', Result),
  !.

plan_to_atom(a(A:B, _, u), Result) :-
  upper(B, B2),
  concat_atom([B2, '_', A], Result),
  !.

plan_to_atom(a(X, _, l), X) :-
  !.

plan_to_atom(a(X, _, u), X2) :-
  upper(X, X2),
  !.


plan_to_atom(true, Result) :-
  concat_atom(['TRUE'], '', Result),
  !.

/*
Translation of operators driven by predicate ~secondoOp~ in 
file ~opSyntax~. There are rules for

  * postfix, 1 or 2 arguments

  * postfix followed by one argument in square brackets, in total 2 or 3 arguments

  * prefix, 2 arguments

Other syntax, if not default (see below) needs to be coded explicitly.

This is the only place to modify when introducing new syntactical
formats for operators.

*/

plan_to_atom(Term, Result) :-
  functor(Term, Op, 1),
  secondoOp(Op, postfix, 1),
  arg(1, Term, Arg1),
  plan_to_atom(Arg1, Res1),
  concat_atom([Res1, ' ', Op, ' '], '', Result),
  !.

plan_to_atom(Term, Result) :-
  functor(Term, Op, 2),
  secondoOp(Op, postfix, 2),
  arg(1, Term, Arg1),
  plan_to_atom(Arg1, Res1),
  arg(2, Term, Arg2),
  plan_to_atom(Arg2, Res2),
  concat_atom([Res1, ' ', Res2, ' ', Op, ' '], '', Result),
  !.

plan_to_atom(Term, Result) :-
  functor(Term, Op, 2),
  secondoOp(Op, postfixbrackets, 2),
  arg(1, Term, Arg1),
  plan_to_atom(Arg1, Res1),
  arg(2, Term, Arg2),
  plan_to_atom(Arg2, Res2),
  concat_atom([Res1, ' ', Op, '[', Res2, '] '], '', Result),
  !.

plan_to_atom(Term, Result) :-
  functor(Term, Op, 3),
  secondoOp(Op, postfixbrackets, 3),
  arg(1, Term, Arg1),
  plan_to_atom(Arg1, Res1),
  arg(2, Term, Arg2),
  plan_to_atom(Arg2, Res2),
  arg(3, Term, Arg3),
  plan_to_atom(Arg3, Res3),
  concat_atom([Res1, ' ', Res2, ' ', Op, '[', Res3, '] '], '', Result),
  !.

plan_to_atom(Term, Result) :-
  functor(Term, Op, 2),
  secondoOp(Op, prefix, 2),
  arg(1, Term, Arg1),
  plan_to_atom(Arg1, Res1),
  arg(2, Term, Arg2),
  plan_to_atom(Arg2, Res2),
  concat_atom([Op, '(', Res1, ',', Res2, ') '], '', Result),
  !.


/*
Generic rules. Operators that are not 
recognized are assumed to be:

  * 1 argument: prefix

  * 2 arguments: infix 

  * 3 arguments: prefix

*/

plan_to_atom(Term, Result) :-
  functor(Term, Op, 1),
  arg(1, Term, Arg1),
  plan_to_atom(Arg1, Res1),
  concat_atom([Op, '(', Res1, ')'], '', Result).

plan_to_atom(Term, Result) :-
  functor(Term, Op, 2),
  arg(1, Term, Arg1),
  arg(2, Term, Arg2),
  plan_to_atom(Arg1, Res1),
  plan_to_atom(Arg2, Res2),
  concat_atom(['(', Res1, ' ', Op, ' ', Res2, ')'], '', Result).

plan_to_atom(Term, Result) :-
  functor(Term, Op, 3),
  arg(1, Term, Arg1),
  arg(2, Term, Arg2),
  arg(3, Term, Arg3),
  plan_to_atom(Arg1, Res1),
  plan_to_atom(Arg2, Res2),
  plan_to_atom(Arg3, Res3),
  concat_atom([Op, '(', Res1, ', ', Res2, ', ', Res3, ')'], '', Result).

plan_to_atom(X, Result) :-
  atomic(X),
  term_to_atom(X, Result),
  !.

plan_to_atom(X, _) :-
  write('Error while converting term: '),
  write(X),
  nl.


params_to_atom([], ' ').

params_to_atom([param(Var, Type)], Result) :-
  type_to_atom(Type, TypeAtom),
  concat_atom([Var, ': ', TypeAtom], '', Result),
  !.

params_to_atom([param(Var, Type) | Params], Result) :-
  type_to_atom(Type, TypeAtom),
  params_to_atom(Params, ParamsAtom),
  concat_atom([Var, ': ', TypeAtom, ', ', ParamsAtom], '', Result),
  !.




type_to_atom(tuple, 'TUPLE').           
type_to_atom(tuple2, 'TUPLE2').
type_to_atom(group, 'GROUP').


/*

5.2 Optimization Rules

We introduce a predicate [=>] which can be read as ``translates into''.

5.2.1 Translation of the Arguments of an Edge of the POG

If the argument is of the form res(N), then it is a stream already and can be
used unchanged. If it is of the form arg(N), then it is a base relation; a
~feed~ must be applied and possibly a ~rename~.

*/

res(N) => res(N).

arg(N) => feed(rel(Name, *, Case)) :-
  isStarQuery,
  argument(N, rel(Name, *, Case)), 
  !.

arg(N) => rename(feed(rel(Name, Var, Case)), Var) :-
  isStarQuery,
  argument(N, rel(Name, Var, Case)),
  !.

arg(N) => project(feed(rel(Name, *, Case)), AttrNames) :-
  argument(N, rel(Name, *, Case)), !, 
  usedAttrList(rel(Name, *, Case), AttrNames).

arg(N) => rename(project(feed(rel(Name, Var, Case)), AttrNames), Var) :-
  argument(N, rel(Name, Var, Case)), !,
  usedAttrList(rel(Name, Var, Case), AttrNames).

/*
5.2.2 Translation of Selections

*/

select(Arg, pr(Pred, _)) => filter(ArgS, Pred) :-
  Arg => ArgS.

select(Arg, pr(Pred, _, _)) => filter(ArgS, Pred) :-
  Arg => ArgS.


/*

Translation of selections using indices.

*/
select(arg(N), Y) => X :-
  indexselect(arg(N), Y) => X.

indexselect(arg(N), pr(attr(AttrName, Arg, Case) = Y, Rel)) => X :-
  indexselect(arg(N), pr(Y = attr(AttrName, Arg, Case), Rel)) => X.

indexselect(arg(N), pr(Y = attr(AttrName, Arg, AttrCase), _)) =>
  exactmatch(IndexName, rel(Name, *, Case), Y)
  :-
  argument(N, rel(Name, *, Case)),
  !,
  hasIndex(rel(Name, *, Case), attr(AttrName, Arg, AttrCase), IndexName, btree).

indexselect(arg(N), pr(Y = attr(AttrName, Arg, AttrCase), _)) =>
  rename(exactmatch(IndexName, rel(Name, Var, Case), Y), Var)
  :-
  argument(N, rel(Name, Var, Case)),
  !,
  hasIndex(rel(Name, Var, Case), attr(AttrName, Arg, AttrCase), IndexName, btree).

indexselect(arg(N), pr(attr(AttrName, Arg, Case) <= Y, Rel)) => X :-
  indexselect(arg(N), pr(Y >= attr(AttrName, Arg, Case), Rel)) => X.

indexselect(arg(N), pr(Y >= attr(AttrName, Arg, AttrCase), _)) =>
  leftrange(IndexName, rel(Name, *, Case), Y)
  :-
  argument(N, rel(Name, *, Case)),
  !,
  hasIndex(rel(Name, *, Case), attr(AttrName, Arg, AttrCase), IndexName, btree).

indexselect(arg(N), pr(Y >= attr(AttrName, Arg, AttrCase), _)) =>
  rename(leftrange(IndexName, rel(Name, Var, Case), Y), Var)
  :-
  argument(N, rel(Name, Var, Case)),
  !,
  hasIndex(rel(Name, Var, Case), attr(AttrName, Arg, AttrCase), IndexName, btree).

indexselect(arg(N), pr(attr(AttrName, Arg, Case) >= Y, Rel)) => X :-
  indexselect(arg(N), pr(Y <= attr(AttrName, Arg, Case), Rel)) => X.

indexselect(arg(N), pr(Y <= attr(AttrName, Arg, AttrCase), _)) =>
  rightrange(IndexName, rel(Name, *, Case), Y)
  :-
  argument(N, rel(Name, *, Case)),
  !,
  hasIndex(rel(Name, *, Case), attr(AttrName, Arg, AttrCase), IndexName, btree).

indexselect(arg(N), pr(Y <= attr(AttrName, Arg, AttrCase), _)) =>
  rename(rightrange(IndexName, rel(Name, Var, Case), Y), Var)
  :-
  argument(N, rel(Name, Var, Case)),
  !,
  hasIndex(rel(Name, Var, Case), attr(AttrName, Arg, AttrCase), IndexName, btree).

%fapra1590
indexselect(arg(N), pr(Y touches attr(AttrName, Arg, Case), Rel)) => X :-
  indexselect(arg(N), pr(attr(AttrName, Arg, Case) touches Y, Rel)) => X.

indexselect(arg(N), pr(attr(AttrName, Arg, AttrCase) touches Y, _)) =>
  filter(windowintersects(IndexName, rel(Name, *, Case), bbox(Y)), attr(AttrName, Arg, AttrCase) touches Y)
  :-
  argument(N, rel(Name, *, Case)),
  !,
  hasIndex(rel(Name, *, Case), attr(AttrName, Arg, AttrCase), IndexName, rtree).

/*
Here ~ArgS~ is meant to indicate ``argument stream''.

5.2.3 Translation of Joins

A join can always be translated to a ~symmjoin~.

*/

join(Arg1, Arg2, pr(Pred, _, _)) => symmjoin(Arg1S, Arg2S, Pred) :-
  Arg1 => Arg1S,
  Arg2 => Arg2S.


/*
product-filter join - for testing only! ~product~ is an asymmetric opertor,
so the arguments must be ordered in both possible ways!

*/

/*
join(Arg1, Arg2, pr(Pred, A1, A2)) => filter(product(Arg1S, Arg2S), pr(Pred, A1, A2)) :-
  Arg1 => Arg1S,
  Arg2 => Arg2S.

join(Arg1, Arg2, pr(Pred, A1, A2)) => filter(product(Arg2S, Arg1S), pr(Pred, A1, A2)) :-
  Arg1 => Arg1S,
  Arg2 => Arg2S.

*/

/*
Spatial Joins:

Joins with geometric predicates using bbox-operators like ~intersects~ or 
~touches~ can make use of the operator ~spatialjoin~ to filter the argument 
streams prior to applying the actual predicate using ~filter~. BBox operators 
are recognizable by using predicate ~isBBoxOperator\1~ as defined in file  
~operators.pl~.

As ~spatialjoin~ is an asymmetric operator, both possible argument orderings
have to be considered.

XRIS: For cost functions, the spatialjoin is considered to work on 2D-MBBs 
only. Rules to detect 3/4D-MBBs and pass the correct dimension-argument to 
the spatialjoin-term should be added later.

For the cost functions, the selectivity is distributed among the spatialjoin 
and the filter operators. Otherwise, cost/8 will over-estimate. This feature 
is implemented by a separate case of the cost function for 
~cost(filter(spatialjoin(...),...),...)~.

*/

join(Arg1, Arg2, pr(Pred, _, _)) => filter(spatialjoin(Arg1S, 
  Arg2S, attrname(Attr1), attrname(Attr2), 2), Pred2) :-
  compound(Pred),
  functor(Pred, Op, 2),
  isBBoxOperator(Op),
  Arg1 => Arg1S, 
  Arg2 => Arg2S, 
  arg(1, Pred, X),
  arg(2, Pred, Y),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y),
  functor(Pred2, Op, 2),       
  arg(1, Pred2, Attr1),
  arg(2, Pred2, Attr2).

join(Arg1, Arg2, pr(Pred, _, _)) => filter(spatialjoin(Arg2S, 
  Arg1S, attrname(Attr2), attrname(Attr1), 2), Pred2) :-
  compound(Pred),
  functor(Pred, Op, 2),
  isBBoxOperator(Op),
  Arg1 => Arg1S, 
  Arg2 => Arg2S, 
  arg(1, Pred, X),
  arg(2, Pred, Y),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y),
  functor(Pred2, Op, 2),       
  arg(1, Pred2, Attr1),
  arg(2, Pred2, Attr2).


/*
Index joins:

*/


join(Arg1, arg(N), pr(X=Y, _, _)) => loopjoin(Arg1S, MatchExpr) :-
  isOfSecond(Attr2, X, Y),
  isNotOfSecond(Expr1, X, Y),
  argument(N, RelDescription),
  hasIndex(RelDescription, Attr2, IndexName, btree),
  Arg1 => Arg1S,
  exactmatch(IndexName, arg(N), Expr1) => MatchExpr.

join(arg(N), Arg2, pr(X=Y, _, _)) => loopjoin(Arg2S, MatchExpr) :-
  isOfFirst(Attr1, X, Y),
  isNotOfFirst(Expr2, X, Y),
  argument(N, RelDescription),
  hasIndex(RelDescription, Attr1, IndexName, btree),
  Arg2 => Arg2S,
  exactmatch(IndexName, arg(N), Expr2) => MatchExpr.


exactmatch(IndexName, arg(N), Expr) =>
  exactmatch(IndexName, rel(Name, *, Case), Expr) :-
  argument(N, rel(Name, *, Case)),
  !.

exactmatch(IndexName, arg(N), Expr) =>
  rename(exactmatch(IndexName, rel(Name, Var, Case), Expr), Var) :-
  argument(N, rel(Name, Var, Case)),
  !.



/*

For a join with a predicate of the form X = Y we can distinguish four cases
depending on whether X and Y are attributes or more complex expressions. For
example, a query condition might be ``PLZA = PLZB'' in which case we have just
attribute names on both sides of the predicate operator, or it could be ``PLZA =
PLZB + 1''. In the latter case we have an expression on the right hand side.
This can still be translated to a hashjoin, for example, by first extending the
second argument by a new attribute containing the value of the expression. For
example, the query

----    select *
        from plz as p1, plz as p2
        where p1.PLZ = p2.PLZ + 1
----

can be translated to

----    plz feed {p1} plz feed {p2} extend[newPLZ: PLZ_p2 + 1]
        hashjoin[PLZ_p1, newPLZ, 999997]
        remove[newPLZ]
        consume
----

This technique is built into the optimizer as follows. We first define the four
cases (at the moment for equijoin only; this may later be extended) which also
translate the arguments into streams. Then the rules translating to join
methods can be formulated independently from this general technique. They
translate terms of the form join00(Arg1Stream, Arg2Stream, Pred).

*/

join(Arg1, Arg2, pr(X=Y, R1, R2)) => JoinPlan :-
  X = attr(_, _, _),
  Y = attr(_, _, _), !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  join00(Arg1S, Arg2S, pr(X=Y, R1, R2)) => JoinPlan.

join(Arg1, Arg2, pr(X=Y, R1, R2)) =>
        remove(JoinPlan, [attrname(attr(r_expr, 2, l))]) :-
  X = attr(_, _, _),
  not(Y = attr(_, _, _)), !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  Arg2Extend = extend(Arg2S, [newattr(attrname(attr(r_expr, 2, l)), Y)]),
  join00(Arg1S, Arg2Extend, pr(X=attr(r_expr, 2, l), R1, R2)) => JoinPlan.

join(Arg1, Arg2, pr(X=Y, R1, R2)) =>
        remove(JoinPlan, [attrname(attr(l_expr, 2, l))]) :-
  not(X = attr(_, _, _)),
  Y = attr(_, _, _), !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  Arg1Extend = extend(Arg1S, [newattr(attrname(attr(l_expr, 1, l)), X)]),
  join00(Arg1Extend, Arg2S, pr(attr(l_expr, 1, l)=Y, R1, R2)) => JoinPlan.

join(Arg1, Arg2, pr(X=Y, R1, R2)) =>
        remove(JoinPlan, [attrname(attr(l_expr, 1, l)),
                attrname(attr(r_expr, 2, l))]) :-
  not(X = attr(_, _, _)),
  not(Y = attr(_, _, _)), !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  Arg1Extend = extend(Arg1S, [newattr(attrname(attr(l_expr, 1, l)), X)]),
  Arg2Extend = extend(Arg2S, [newattr(attrname(attr(r_expr, 2, l)), Y)]),
  join00(Arg1Extend, Arg2Extend,
        pr(attr(l_expr, 1, l)=attr(r_expr, 2, l), R1, R2)) => JoinPlan.


join00(Arg1S, Arg2S, pr(X = Y, _, _)) => sortmergejoin(Arg1S, Arg2S,
        attrname(Attr1), attrname(Attr2))   :-
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).

join00(Arg1S, Arg2S, pr(X = Y, _, _)) => hashjoin(Arg1S, Arg2S,
        attrname(Attr1), attrname(Attr2), 999997)   :-
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).

join00(Arg1S, Arg2S, pr(Y = X, _, _)) => hashjoin(Arg2S, Arg1S,
        attrname(Attr2), attrname(Attr1), 999997)   :-
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).


/*

----    isOfFirst(Attr, X, Y)
        isOfSecond(Attr, X, Y)
----

~Attr~ equal to either ~X~ or ~Y~ is an attribute of the first(second) relation.

*/


isOfFirst(X, X, _) :- X = attr(_, 1, _).
isOfFirst(Y, _, Y) :- Y = attr(_, 1, _).
isOfSecond(X, X, _) :- X = attr(_, 2, _).
isOfSecond(Y, _, Y) :- Y = attr(_, 2, _).

isNotOfFirst(Y, X, Y) :- X = attr(_, 1, _).
isNotOfFirst(X, X, Y) :- Y = attr(_, 1, _).
isNotOfSecond(Y, X, Y) :- X = attr(_, 2, _).
isNotOfSecond(X, X, Y) :- Y = attr(_, 2, _).


/*
6 Creating Query Plan Edges

*/

createPlanEdge :-
  edge(Source, Target, Term, Result, _, _),
  Term => Plan,
  assert(planEdge(Source, Target, Plan, Result)),
  fail.

createPlanEdges :- not(createPlanEdge).

deletePlanEdge :-
  retract(planEdge(_, _, _, _)), fail.

deletePlanEdges :- not(deletePlanEdge).

planEdgeInfo(Source, Target, Plan, Result) :-
  write('Source: '), write(Source), nl,
  write('Target: '), write(Target), nl,
  write('Plan  : '), wp(Plan), nl,
  % write(Plan), nl,
  write('Result: '), write(Result), nl, nl.

writePlanEdges2:-
  planEdge(Source, Target, Plan, Result), 
  planEdgeInfo(Source, Target, Plan, Result), 
  fail.

writePlanEdges :- not(writePlanEdges2).




/*
7 Assigning Sizes, TupleSizes, Selectivities and Predicate Costs to the Nodes and Edges of the POG

----    assignSizes.
        deleteSizes.
----

Assign sizes (numbers of tuples) to all nodes in the pog, based on the
cardinalities of the argument relations and the selectivities of the
predicates. Store sizes as facts of the form resultSize(Result, Size). Stored
result tuple sizes as facts of the form resultTupleSize(Result, Size), stored
selectivities as facts of the form edgeSelectivity(Source, Target, Sel, PredCost, TupleGenerationCost).

Delete sizes from memory.

7.1 Assigning Sizes, Selectivities, Predicate Costs od Tuple Generation Costs

It is important that edges are processed in the order in which they have been
created. This will ensure that for an edge the size of its argument nodes are
available.

*/

assignSizes :- not(assignSizes1).

assignSizes1 :-
  edge(Source, Target, Term, Result, _, _),
  assignSize(Source, Target, Term, Result),
  fail.

assignSize(Source, Target, select(Arg, Pred), Result) :-
  resSize(Arg, Card),
  resTupleSize(Arg, ArgTupleSize),
  selectivity(Pred, Sel),
  Size is Card * Sel,
  setNodeSize(Result, Size),
  setNodeTupleSize(Result, ArgTupleSize),
  predicateCost(Pred, PredCost),
  assert(edgeSelectivity(Source, Target, Sel, PredCost, 0.0035)), 
  % tuple generation cost was set to 0.0035, 
  % as '*' caused problems within query
  % sql select[kname] from[kreis] where[gebiet touches magdeburg].
  !.
  

assignSize(Source, Target, join(Arg1, Arg2, Pred), Result) :-
  resSize(Arg1, Card1),
  resSize(Arg2, Card2),
  resTupleSize(Arg1, TSize1),
  resTupleSize(Arg2, TSize2),
  selectivity(Pred, Sel),
  addTupleSizes(TSize1,TSize2,TSize),
  Size is Card1 * Card2 * Sel,
  setNodeSize(Result, Size),
  setNodeTupleSize(Result, TSize),
  predicateCost(Pred, PredCost),
  Pred = pr( _, R1, R2),
  sampleRuntimesJ(R1,R2, _, _,TupleGenerationCost),  % CAVE: no info present for ad-hoc joins!
  assert(edgeSelectivity(Source, Target, Sel, PredCost, TupleGenerationCost)),
  !.


/*
----    setNodeSize(Node, Size) :-
----

Set the size of node ~Node~ to ~Size~ if no size has been assigned before.

*/

setNodeSize(Node, _) :- resultSize(Node, _), !.
setNodeSize(Node, Size) :- assert(resultSize(Node, Size)).

/*
----    setNodeTupleSize(Node, Size) :-
----

Set the tuple size of node ~Node~ to ~Size~ if no size has been assigned before.

*/

setNodeTupleSize(Node, _) :- resultTupleSize(Node, _), !.
setNodeTupleSize(Node, Size) :- assert(resultTupleSize(Node, Size)).

/*
----    resSize(Arg, Size) :-
----

Argument ~Arg~ has size ~Size~.

*/

resSize(arg(N), Size) :- argument(N, rel(Rel, _, _)), card(Rel, Size), !.
resSize(arg(N), _) :- write('Error in optimizer: cannot find cardinality for '),
  argument(N, Rel), wp(Rel), nl, fail.
resSize(res(N), Size) :- resultSize(N, Size), !.

/*
----    resTupleSize(Arg, Size) :-
----

Argument ~Arg~ has tuple size ~Size~.

*/

resTupleSize(arg(N), tupleSizeData(CoreTupleSize,InFlobSize,ExtFlobSize)) :- 
  argument(N, Rel), 
  Rel = rel(_, _, _),
  calculateQueryTupleSize(Rel, CoreTupleSize, InFlobSize, ExtFlobSize), 
  !.
resTupleSize(arg(N), _) :- write('Error in optimizer: cannot find tuple size for '),
  argument(N, Rel), wp(Rel), nl, fail.
resTupleSize(res(N), Size) :- resultTupleSize(N, Size), !.


/*
----    writeSizes :-
----

Write sizes, selectivities and predicate costs.

*/

writeSize :-
  resultSize(Node, Size),
  resultTupleSize(Node, tupleSizeData(Core,Inl,Ext)),
  write('Node:       '), write(Node), nl,
  write('Size:       '), write(Size), nl, 
  TotalTupleSize is Core+Inl+Ext,
  write('Tuple Size: '), write(TotalTupleSize), write(' = ('), 
  write(Core), write('+'), write(Inl), write('+'), write(Ext), write(')\n\n'),
  fail.
writeSize :-
  edgeSelectivity(Source, Target, Sel, PredCost, TupleGenerationCost),
  write('Source:        '), write(Source), nl,
  write('Target:        '), write(Target), nl,
  write('Selectivity:           '), write(Sel), nl, 
  write('Predicate cost:        '), write(PredCost), nl, 
  write('Tuple generation cost: '), write(TupleGenerationCost), nl, nl,
  fail.
writeSizes :- not(writeSize).

/*
----    deleteSizes :-
----

Delete node sizes and selectivities of edges.

*/

deleteSize :- retract(resultSize(_, _)), fail.
deleteSize :- retract(resultTupleSize(_, _)), fail.
deleteSize :- retract(edgeSelectivity(_, _, _, _, _)), fail.
deleteSizes :- not(deleteSize).


/*
8 Computing Edge Costs for Plan Edges

8.1 The Costs of Terms

----    cost(Term+, Pred+, Sel+, Size-, TupleSize-, PredCost+, TC+, Cost-)
----

The cost of an executable ~Term~ representing predicate ~Pred~ with selectivity 
~Sel~, predicate cost ~PredCost~, tuple generation cost ~TC~ is ~Cost~ and the 
size of the result is ~Size~, while its tuplesize is ~TupleSize~.

This is evaluated recursively descending into the term. When the operator
realizing the predicate (e.g. ~filter~) is encountered, the selectivity ~Sel~ is
used to determine the size of the result. The cost for a single evaluation 
of that predicate ~PedCost~ and possibly the cost to generate a result tuple ~TC~
is taken into account when estimating the total cost. It is assumed that only a 
single operator of this kind occurs within the term.

The base cost factors are defined in file ``operators.pl''. 

A standard naming scheme for cost factors is used to enhance the readability of 
the cost functions:

----   RTM:  ReadTupleMem 
       WTM:  WriteTupleMem 
       RTD:  ReadTupleDisk 
       WTD:  WriteTupleDisk 
       RPD:  ReadPageDisk 
       WPD:  WritePageDisk 
       FND:  FileNewDisk 
       FDD:  FileDeleteDisk 
       FOD:  FileOpenDisk 
       FCD:  FileClose 
    MaxMem:  maximum memory size per operator
        TC:  TupleCreate
         B:  PredicateCost/cost of evaluating of an ordering predicate
       Sel:  Selectivity
        CX:  Cardinality of first argument
        CX:  Cardinality of second argument
        Tx:  (necessary) TupleSize of first argument
        Ty:  (necessary) TupleSize of second argument
       TSC:  TupleSizeCoretuple (always handled)
       TSI:  TupleSizeInlineFlob (always handled)
       TSE:  TupleSizeExternalFlob (only handled when used in predicate)
----

8.1.0 Auxiliary Predicates to cost

----    gsf(M, N, S)
        log(B, V, R)
----

These predicates calculate $S=\sum_{i=1}^{N-M}{i+M} = {{(N-M)(N+M+1)}\over{2}}$
and $R = log_B V$.

*/

:- arithmetic_function(gsf/2).
gsf(M, N, S) :-
  S is (N-M)*(N+M+1)*0.5.

:- arithmetic_function(log/2).
log(Base, Value, Result) :-
  Result is log(Value) / log(Base).

% add two tupleSizeData(A,B,C) terms component wise:
addTupleSizes(tupleSizeData(Core1, InlFlob1, ExtFlob1), 
              tupleSizeData(Core2, InlFlob2, ExtFlob2), 
              tupleSizeData(Core, InlFlob, ExtFlob)) :- 
  Core    is Core1    + Core2,
  InlFlob is InlFlob1 + InlFlob2,
  ExtFlob is ExtFlob1 + ExtFlob2.

/*
---- setCurrentResultSizes
     setCurrentResultSizes(ResCard, ResTupleSize)
     getCurrentResultSizes(Term)
----
Get cardinality and tuplesize of the current (intermediate) result, formatted as 
a 'fixed'-term ~Term~, suitable as an input for the predicate cost/8.

The predicates are used to calculate costs arising from Secondo operators 
inserted into the executable plan by the pre-processing steps of the optimizer's 
~sql/1~ predicate, e.g. the final ~consume~, ~count~, ~project~, or commands 
inserted between multiple optimized streams, like ~mergeunion~ or ~mergesec~.

~setCurrentResultSizes/0~ should be called after a stream has been optimized in 
~streamOptimize~.

~setCurrentResultSizes(ResCard, ResTupleSize)~ will update 
~storedCurrentResultSizes/1~ with its arguments.

*/

:- dynamic(storedCurrentResultSizes/1),
   assert(storedCurrentResultSizes(fixed(0, tupleSizeData(0,0,0)))).

getCurrentResultSizes(Term) :-
  storedCurrentResultSizes(Term).

setCurrentResultSizes :- 
  highNode(0),
  retract(storedCurrentResultSizes(_)),
  assert(storedCurrentResultSizes(fixed(0, tupleSizeData(0,0,0)))),
  !.

setCurrentResultSizes :-
  highNode(N),
  resultSize(N, ResCard),
  resultTupleSize(N, ResTS),
  retract(storedCurrentResultSizes(_)),
  assert(storedCurrentResultSizes(fixed(ResCard, ResTS))),
  !.

setCurrentResultSizes :-
  retract(storedCurrentResultSizes(_)),
  assert(storedCurrentResultSizes(fixed(0, tupleSizeData(0,0,0)))),
  !.

setCurrentResultSizes(ResCard, ResTupleSize) :-
  retract(storedCurrentResultSizes(_)),
  assert(storedCurrentResultSizes(fixed(ResCard, ResTupleSize))),
  !.

/*
8.1.1 Arguments

*/


cost(rel(Rel, X, Y), _, _, Size, TupleSize, _, _, 0) :-
  calculateQueryTupleSize(rel(Rel, X, Y), CoreTupleSize, InFlobSize, ExtFlobSize), 
  TupleSize = tupleSizeData(CoreTupleSize,InFlobSize,ExtFlobSize),
  card(Rel, Size).

cost(res(N), _, _, Size, TupleSize, _, _, 0) :-
  resultTupleSize(N, TupleSize), 
  resultSize(N, Size).


/*
Argument with explicitly defined cardinality ~ResCard~ and tuple size 
~TupleSize~.  Used to calculate cost of operators inserted by the pre-processor, 
eg. in predicate ~mStreamOptimize~, and for the final count/consume.

*/

cost(fixed(ResCard, TupleSize), _, _, ResCard, TupleSize, _, _, 0).

/*
8.1.2 Operators

*/

cost(feed(X), P, Sel, S, TupleSize, PredCost, TC, C) :-
  cost(X, P, Sel, S, TupleSize, PredCost, TC, C1),
  cost_factors(_, _, _, _, RPD, _, _, _, FOD, _, _, BDBPS, _),
  feedTC(A,B),
  TupleSize = tupleSizeData(TSC, TSI, _),
  NumPages is ceiling((S * (TSC+TSI))/BDBPS),
  C is C1                        % create Relation, 
     + FOD                       % open relation
     + RPD * NumPages            % read tuples pagewise
     + A * S                     % consider overhead tuplewise
     + B * S * (TSC+TSI).        % consider overhead bytewise

/*
Here ~feedTC~ means ``feed tuple cost'', i.e., the cost per tuple, a constant to
be determined in experiments. These constants are kept in file ``Operators.pl''.

*/

cost(consume(X), P, Sel, S, TupleSize, PredCost, TC, C) :-
  cost(X, P, Sel, S, TupleSize, PredCost, TC, C1),
  consumeTC(A,B),
  TupleSize = tupleSizeData(TSC, TSI, TSE),
  C is C1                          % create input stream
     + A * S                       % consider overhead tuplewise
     + B * S * (TSC+TSI+TSE).      % consider overhead bytewise

/*
Generic cost function for aggregation operators like count, min, max, sum, avg.
For these operators, the fact ~isAggregationOP(name)~ should be defined in file
``operators.pl''.

*/

cost(Term, P, Sel, S, TupleSize, PredCost, TC, C) :-
  compound(Term),
  functor(Term, Op, 1),
  isAggregationOP(Op),               % recognize OP as an aggregation operator
  arg(1, Term, X),
  cost(X, P, Sel, S, TupleSize, PredCost, TC, C1),
  aggregateTC(A),
  C is C1                            % create input stream
     + A*S.                          % consider overhead tuplewise

     
/*
When filter is used on the output of a spatialjoin, the edge-selectivity is 
distributed among both operators. You can change the behaviour by changing 
~spatialjoinSDR/1~ in file ~operators.pl~.

*/

cost(filter(X, _), P, Sel, S, TupleSize, PredCost, TC, C) :-
%cost(filter(X, Pred), P, Sel, S, TupleSize, PredCost, TC, C) :- 
% A cost function for the ordering predicate built from Pred could be calculated
  X \= spatialjoin(_, _, _, _, _), % this is the normal case
  cost_factors(_, _, _, _, RPD, _, _, _, _, _, _, BDBPS, FCMR),
  filterTC(A),
  cost(X, P, Sel, SizeX, TupleSize, PredCost, TC, CostX),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  S is SizeX * Sel,
  C is CostX                     % create input stream
     + SizeX * (ExtFlobSize1 + ExtFlobSize2)/BDBPS * FCMR * RPD % read flobs from disk
     + (A + PredCost) * SizeX.   % consider predicate evaluation and overhead tuplewise

cost(filter(X, _), P, Sel, S, TupleSize, PredCost, TC, C) :-
%cost(filter(X, Pred), P, Sel, S, TupleSize, PredCost, TC, C) :- 
% A cost function for the ordering predicate built from Pred could be calculated
  X = spatialjoin(_, _, _, _, _),  % this is the special case where filter is used 
                                   % on the output of a spatialjoin.
  cost_factors(_, _, _, _, RPD, _, _, _, _, _, _, BDBPS, FCMR),
  filterTC(A),
  spatialjoinSDR(Z),           % (spatial join selectivity distribution ratio)
  SelJ is Z + Sel * (1-Z/100), % distribute overall selectivity between
  SelF is Sel/SelJ,            % the spatialjoin and filter operator
  cost(X, P, SelJ, SizeX, TupleSize, PredCost, TC, CostX),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  S is SizeX * SelF,
  C is CostX                     % create input stream
     + SizeX * (ExtFlobSize1 + ExtFlobSize2)/BDBPS * FCMR * RPD % read flobs from disk
     + (A + PredCost) * SizeX.   % consider predicate evaluation and overhead tuplewise


cost(product(X, Y), P, _, ResultCard, ResultTupleSize, B, TC, Cost) :-
  % the product operator is not symmetric. Y is buffered and X will get consumed.
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  TY = tupleSizeData(TSCy, TSIy, _),
  Ty is TSCy + TSIy,
  cost_factors(RTM, WTM, _, _, _, _, _, _, _, _, MaxMem, _, _),
  OY is (MaxMem / Ty),
  CY =< OY,
  % write('\nProduct: Case 1\n'),
  ResultCard is CX * CY,
  addTupleSizes(TX,TY, ResultTupleSize),
  Cost is CostX + CostY          % produce input streams
        + WTM * CY               % write Y-tuples to in-memory buffer
        + RTM * CX * CY          % read Y-tuples from in-memory buffer
        + TC * ResultCard.       % generate result tuples

cost(product(X, Y), P, _, ResultCard, ResultTupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  TY = tupleSizeData(TSCy, TSIy, _),
  Ty is TSCy + TSIy,
  cost_factors( _, WTM, RTD, WTD, _, _, FND, FDD, _, _, MaxMem, _, _),
  OY is (MaxMem / Ty),
  CY > OY,
  % write('\nProduct: Case 2\n'),
  ResultCard is CX * CY,
  addTupleSizes(TX,TY, ResultTupleSize),
  Cost is CostX + CostY          % produce input streams
        + FND + FDD              % create and remove temp file
        + WTM * OY               % write Y-tuples to in-memory buffer
        + WTD * CY               % write Y-tuples to disk
        + RTD * CX * CY          % read Y-tuples from disk
        + TC * ResultCard.       % generate result tuple

/*
  Uses a btree-index to select tuples from a relation.
  The index must be opened. The predicate is always ${X}\geq v$
  for ~rightrange~, ${}\leq v$ for ~leftrange~, and ${}= v$ for 
  ~exactmatch~.
  
  To estimate the number of disk accesses, we query the 
  height and the keys-per-leafe size $K$ of the BTree.
  then we expect to access 
  $\mbox{height}+{\mbox{Sel} \cdot \mbox{RelSize}}\over K$ disk pages 
  and evalute ${\mbox{Height}+K}\over 2$ times the comparisons.

  XRIS: By now, the fan-out for Btrees is assumed to be 15.
  
*/

cost(leftrange(_, Rel, _), P, Sel, Size, TupleSize, B, TC, Cost) :-
  cost(Rel, P, 1, RelSize, TupleSize, B, TC, CostR),
  cost_factors(_, _, _, _, RPD, _, _, _, FOD, FCD, _, BDBPS, FCMR),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Size is Sel * RelSize,
  Cost is CostR                           % produce input stream
        + Size*(ExtFlobSize1 + ExtFlobSize2)/BDBPS * FCMR * RPD % read flobs from disk
        + FOD + FCD                       % open/close btree file        
        + B * log(15,RelSize)             % later: RPD * (Height + K/2)
        + RPD * log(15,RelSize)           % later: RPD * (Height + Sel*RelSize/K)
        + TC * Sel * RelSize.             % produce tuple stream

cost(rightrange(_, Rel, _), P, Sel, Size, TupleSize, B, TC, Cost) :-
  cost(Rel, P, 1, RelSize, TupleSize, B, TC, CostR),
  cost_factors(_, _, _, _, RPD, _, _, _, FOD, FCD, _, BDBPS, FCMR),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Size is Sel * RelSize,
  Cost is CostR                           % produce input stream
        + Size*(ExtFlobSize1 + ExtFlobSize2)/BDBPS * FCMR * RPD % read flobs from disk
        + FOD + FCD                       % open/close btree file        
        + B * log(15,RelSize)             % find all matches,           later: B * (Height + K/2)
        + RPD * log(15,RelSize)           % read index pages from disk, later: D * (Height + Sel*RelSize/K)
        + TC * Sel * RelSize.             % produce tuple stream

/*
Simplistic cost estimation for loop joins.

If attribute values are assumed independent, then the selectivity
of a subquery appearing in an index join equals the overall
join selectivity. Therefore it is possible to estimate
the result size and cost of a subquery
(i.e. ~exactmatch~ and ~exactmatchfun~). As a subquery in an
index join is executed as often as a tuple from the left
input stream arrives, it is also possible to estimate the
overall index join cost.

See: ~cost(leftrange(...),...)~ for key on tuple constants.

*/

cost(exactmatchfun(_, Rel, _), P, Sel, Size, TupleSize, B, TC, Cost) :-
  cost(Rel, P, 1, RelSize, TupleSize, B, TC, CostR),
  cost_factors(_, _, _, _, RPD, _, _, _, FOD, FCD, _, BDBPS, FCMR),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Size is Sel * RelSize,
  Cost is CostR                           % produce input stream
        + Size*(ExtFlobSize1 + ExtFlobSize2)/BDBPS * FCMR * RPD % read flobs from disk
        + FOD + FCD                       % open/close btree file        
        + B * (log(15,RelSize)+Size)      % find all matches,           later: B * (Height + K/2)
        + RPD * log(15,RelSize)           % read index pages from disk, later: D * (Height + Sel*RelSize/K)
        + TC * Sel * RelSize.             % produce tuple stream

cost(exactmatch(_, Rel, _), P, Sel, Size, TupleSize, B, TC, Cost) :-
  cost(Rel, P, 1, RelSize, TupleSize, B, TC, CostR),
  cost_factors(_, _, _, _, RPD, _, _, _, FOD, FCD, _, BDBPS, FCMR),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Size is Sel * RelSize,
  Cost is CostR                           % produce input stream
        + Size*(ExtFlobSize1 + ExtFlobSize2)/BDBPS * FCMR * RPD % read flobs from disk
        + FOD + FCD                       % open/close btree file        
        + B * (log(15,RelSize)+Size)      % find all matches,           later: B * (Height + K/2)
        + RPD * log(15,RelSize)           % read index pages from disk, later: D * (Height + Sel*RelSize/K)
        + TC * Sel * RelSize.             % produce tuple stream


/*
  Algorithm loopjoin: see file ``ExtRelationAlgebra.cpp''.

*/

cost(loopjoin(X, Y), P, Sel, ResCard, TupleSize, PredCost, TC, Cost) :-
  cost(X, P, 1, CX, TX, PredCost, TC, CostX),
  cost(Y, P, Sel, CY, _, PredCost, TC, CostY),
  addTupleSizes(TX, TX, TupleSize),
  ResCard is CX * CY,
  cost_factors(_, _, _, _, RPD, _, _, _, _, _, _, BDBPS, _),
  loopjoinTC(A),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CX * CostY        % produce input streams, stream Y is produced once per X-tuple
        + RPD * CX*(ExtFlobSize1)/BDBPS % read flobs from disk
        + RPD * CY*(ExtFlobSize2)/BDBPS % read flobs from disk
        + A * CX                    % overhead for opening stream Y
        + TC * ResCard.             % create result tuples

cost(fun(_, X), P, Sel, Size, TupleSize, PredCost, TC, Cost) :- % no overhead considered, by now
  cost(X, P, Sel, Size, TupleSize, PredCost, TC, Cost).


/*
Cost function for ~hashjoin~:

  * NBuckets: Number of buckets in hash table

  * MemSizeX = 0.25[*]MaxMem; this one is used to hold X-tuples hashed against the hash table

  * MemSizeY = 0.75[*]MaxMem; this one is used for the hash table of Y-tuples

  For the evaluation of the join predicate, we know, that only 'is equal' is allowed, as
  hashjoin is an equijoin. For all 'is equal' predicates, it is assumed, that only in the
  case of $A=B$, the evaluation is really expensive, for in the contrary case, some cheap 
  test will usually discover, that $A\neqB$.

  Hence, we can estimate the number of expensive predicate evaluations by $\mbox{Sel} \cdot CX \cdot CY$.
  All other predicate evaluations are expected to be 'cheap' (with cost ~MinPET~).

  For the calculation of a hash value, a cost of ~MinPET~ is assigned.

*/

cost(hashjoin(X, Y, _, _, NBuckets), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostArgX),
  cost(Y, P, 1, CY, TY, B, TC, CostArgY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, MaxMem, BDBPS, FCMR),
  MemSizeX is 0.25*MaxMem,
  MemSizeY is 0.75*MaxMem,
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  % test for case1: Both argument relations X,Y fit into memory  
  (CX * (Tx+12)) =< MemSizeX, 
  (CY * (Ty+12)) =< MemSizeY, 
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  minPET(MinPET),
  ResSize is CX * CY * Sel,
  addTupleSizes(TX,TY,TupleSize),
  Cost is CostArgX + CostArgY                  % create input streams
       + RPD * (CX * ExtFlobSize1)/BDBPS       % read flobs from disk
       + RPD * (CX * max((CY/NBuckets),1) * ExtFlobSize2)/BDBPS/(1-FCMR) % read flobs from disk
       + MinPET * (CX + CY)                    % calculate hash value (use MinPET as an approximation to cost)
       + WTM * CY                              % inserting Y-tuples into hash table
                                               % collide each X-tuple with Y-tuples from hashed bucket
       % +(RTM + B) * CX * max((CY/NBuckets),1)%  (this is the original line)
       + (RTM + B) * ResSize                   %  - expensive cases
       + (RTM + MinPET)                        %  - cheap cases
               * (CX * max((CY/NBuckets),1) - ResSize) 
       + TC * ResSize,                         % create result tuples
  (optDebug *-> (
    RTMn is CX * max((CY/NBuckets),1),
    WTMn is CY,
    RTDn is 0,
    WTDn is 0,
    RPDn is (CX * ExtFlobSize1)/BDBPS + (CX * max((CY/NBuckets),1) * ExtFlobSize2)/BDBPS/(1-FCMR),
    WPDn is 0,
    FNDn is 0,
    FDDn is 0,
    FODn is 0,
    FCDn is 0,
    Bn   is ResSize,
    MinPETn is CX * max((CY/NBuckets),1) - ResSize + (CX + CY),
    TCn  is ResSize,
    write('\n\n\nhashjoin('), write(X), write(','), write(Y), write(', _, _, '), write(NBuckets), write('), '),
    write(P), write(','), write(Sel), write(','), write(ResSize), write(','), write(TupleSize), write(','), 
    write(B), write(','), write(TC), write(','), write(Cost), write(') Case1'),
    write('\n\tRTM: '), write(RTMn),
    write('\n\tWTM: '), write(WTMn),
    write('\n\tRTD: '), write(RTDn),
    write('\n\tWTD: '), write(WTDn),
    write('\n\tRPD: '), write(RPDn),
    write('\n\tWPD: '), write(WPDn),
    write('\n\tFND: '), write(FNDn),
    write('\n\tFDD:  '), write(FDDn),
    write('\n\tFOD:  '), write(FODn),
    write('\n\tFCD:  '), write(FCDn),
    write('\n\tB  :  '), write(Bn),
    write('\n\tTC :  '), write(TCn),
    write('\n\tMinPET:  '), write(MinPETn), 
    write('\n\tNeeded FlobSizes: '), write(ExtFlobSize1), write('/'), write(ExtFlobSize2), nl
  ); true).

cost(hashjoin(X, Y, _, _, NBuckets), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostArgX),
  cost(Y, P, 1, CY, TY, B, TC, CostArgY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  MemSizeX is 0.25*MaxMem,
  MemSizeY is 0.75*MaxMem,
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  % test for case4: neither X, nor Y fit in memory                                       
  (CX * (Tx+12)) > MemSizeX,
  (CY * (Ty+12)) > MemSizeY,
  MemSizeTY is max(1,(MemSizeY / Ty)),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  minPET(MinPET),
  ResSize is CX * CY * Sel,
  addTupleSizes(TX,TY,TupleSize),
  Cost is CostArgX + CostArgY          % create input streams
       + WTM * CY                      % insert pointers to Y in hash tables
       + RPD * (CY/MemSizeTY) * (CX * ExtFlobSize1)/BDBPS % read flobs from disk
       + RPD * CX * max(1,(MemSizeTY/NBuckets)) 
                  * (CY/MemSizeTY) * ExtFlobSize2/BDBPS/(1-FCMR) % read flobs from disk
                                       % compare X- and Y-tuples
       % +(B + RTM)  * CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) % (original line)
       + (B + RTM)  * ResSize          %  - cheap comparisons
       + (MinPET + RTM) 
           * (  CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) 
              - ResSize )              %  - expensive comparisons
       + 2 * (FND + FDD)               % create and delete temp files
       + WTD * CX                      % write X-tuples to disk
       + RTD * CX * CY/MemSizeTY       % X-tuples to be read from disk once per hash table
       + MinPET * (CY + CX * max(1,CY/MemSizeTY))  % calculate hash value (use MinPET as an approximation to cost)
       + TC * ResSize,                 % create result tuples
  (optDebug *-> (
    RTMn is CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY),
    WTMn is CY,
    RTDn is CX * CY/MemSizeTY,
    WTDn is CX,
    RPDn is (CY/MemSizeTY) * (CX * ExtFlobSize1)/BDBPS + CX 
            * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) 
            * ExtFlobSize2/BDBPS/(1-FCMR),
    WPDn is 0,
    FNDn is 2,
    FDDn is 2,
    FODn is 0,
    FCDn is 0,
    Bn   is ResSize,
    MinPETn is CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) 
                         - ResSize + (CY + CX * max(1,CY/MemSizeTY)),
    TCn  is Sel * CX * CY,
    write('\n\nhashjoin('), write(X), write(','), write(Y), 
    write(', _, _, '), write(NBuckets), write('), '),
    write(P), write(','), write(Sel), write(','), write(ResSize), 
    write(','), write(TupleSize), write(','), 
    write(B), write(','), write(TC), write(','), write(Cost), write(') Case4'),
    write('\n\tRTM: '), write(RTMn),
    write('\n\tWTM: '), write(WTMn),
    write('\n\tRTD: '), write(RTDn),
    write('\n\tWTD: '), write(WTDn),
    write('\n\tRPD: '), write(RPDn),
    write('\n\tWPD: '), write(WPDn),
    write('\n\tFND: '), write(FNDn),
    write('\n\tFDD:  '), write(FDDn),
    write('\n\tFOD:  '), write(FODn),
    write('\n\tFCD:  '), write(FCDn),
    write('\n\tB  :  '), write(Bn),
    write('\n\tTC :  '), write(TCn), 
    write('\n\tMinPET:  '), write(MinPETn), 
    write('\n\tNeeded FlobSizes: '), write(ExtFlobSize1), write('/'), 
    write(ExtFlobSize2), nl
  ); true).

cost(hashjoin(X, Y, _, _, NBuckets), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostArgX),
  cost(Y, P, 1, CY, TY, B, TC, CostArgY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, MaxMem, BDBPS, FCMR),
  MemSizeX is 0.25*MaxMem,
  MemSizeY is 0.75*MaxMem,
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  % case2: X fits in memory, but Y does not 
  (CX * (Tx+12)) =< MemSizeX, 
  (CY * (Ty+12)) >  MemSizeY,            
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  minPET(MinPET),
  ResSize is CX * CY * Sel,
  addTupleSizes(TX,TY,TupleSize),
  MemSizeTY is max(1,(MemSizeY / Ty)), % amount of Y-tuples fitting into MemY
  Cost is CostArgX + CostArgY          % create input streams
       + WTM * CY                      % insert Y-tuples into hash table
       + RPD * (CY/MemSizeTY) * (CX*ExtFlobSize1)/BDBPS % read flobs from disk
       + RPD * CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) 
                     * ExtFlobSize2/BDBPS/(1-FCMR) % read flobs from disk
                                       % compare each X-tuple with
                                       % the hashed Y-tuples from several hashtables
       %+ (B + RTM) * CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) % (original line)
       + (B + RTM) * ResSize           %  - expensive comparisons
       + (MinPET+RTM)*(  CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) 
                       - ResSize)      %  - cheap comparisons

       + RTM * CX * (CY/MemSizeTY)     % read X-tuples from memory once per hashtable
       + MinPET * (CY + CX * max(1,CY/MemSizeTY)) % calculate hash values (use MinPET as an approximation to cost)
       + TC * ResSize,                 % create result tuples
  (optDebug *-> (
    RTMn is CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) + CX * (CY/MemSizeTY),
    WTMn is CY,
    RTDn is 0,
    WTDn is 0,
    RPDn is (CY/MemSizeTY) * (CX*ExtFlobSize1)/BDBPS 
             +  CX * max(1,(MemSizeTY/NBuckets)) 
                   * (CY/MemSizeTY) 
                   * ExtFlobSize2/BDBPS/(1-FCMR),
    WPDn is 0,
    FNDn is 0,
    FDDn is 0,
    FODn is 0,
    FCDn is 0,
    Bn   is ResSize,
    MinPETn is CX * max(1,(MemSizeTY/NBuckets)) 
                  * (CY/MemSizeTY) 
                  - ResSize + (CY + CX * max(1,CY/MemSizeTY)),
    TCn  is Sel * CX * CY,
    write('\n\nhashjoin('), write(X), write(','), write(Y), 
    write(', _, _, '), write(NBuckets), write('), '),
    write(P), write(','), write(Sel), write(','), write(ResSize), 
    write(','), write(TupleSize), write(','), 
    write(B), write(','), write(TC), write(','), write(Cost), write(') Case2'),
    write('\n\tRTM: '), write(RTMn),
    write('\n\tWTM: '), write(WTMn),
    write('\n\tRTD: '), write(RTDn),
    write('\n\tWTD: '), write(WTDn),
    write('\n\tRPD: '), write(RPDn),
    write('\n\tWPD: '), write(WPDn),
    write('\n\tFND: '), write(FNDn),
    write('\n\tFDD:  '), write(FDDn),
    write('\n\tFOD:  '), write(FODn),
    write('\n\tFCD:  '), write(FCDn),
    write('\n\tB  :  '), write(Bn),
    write('\n\tTC :  '), write(TCn),
    write('\n\tMinPET:  '), write(MinPETn), 
    write('\n\tNeeded FlobSizes: '), write(ExtFlobSize1), write('/'), 
    write(ExtFlobSize2), nl
  ); true).

cost(hashjoin(X, Y, _, _, NBuckets), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostArgX),
  cost(Y, P, 1, CY, TY, B, TC, CostArgY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, MaxMem, BDBPS, FCMR),
  MemSizeX is 0.25*MaxMem,
  MemSizeY is 0.75*MaxMem,
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  % case3: X does not fit in memory, but Y does  (Nearly the same cost function as in case2!)
  (CX * (Tx+12)) >  MemSizeX, 
  (CY * (Ty+12)) =< MemSizeY,  
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  minPET(MinPET),
  ResSize is CX * CY * Sel,
  addTupleSizes(TX,TY,TupleSize),
  MemSizeTY is max(1,(MemSizeY / Ty)), % amount of Y-tuples fitting into MemY
  Cost is CostArgX + CostArgY          % create input streams
       + WTM * CY                      % insert Y-tuplesinto hash table
       + RPD * (CX * ExtFlobSize1)/BDBPS % read flobs from disk
       + RPD * CX * max(1,(MemSizeTY/NBuckets)) 
                  * (CY/MemSizeTY) 
                  * ExtFlobSize2/BDBPS/(1-FCMR) % read flobs from disk
                                       % compare each X-tuple with
                                       % the hashed Y-tuples from several hashtables
       %+ (B + RTM) * CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) % (original line)
       + (B + RTM) * ResSize           %  - expensive comparisons
       + (MinPET+RTM)*(  CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) 
                       - ResSize)      %  - cheap comparisons
       + RTM * CX * (CY/MemSizeTY)     % read X-tuples from memory once per hashtable
       + MinPET * (CX + CY)            % calculate hash values (use MinPET as an approximation to cost)
       + TC * ResSize,                 % create result tuples
  (optDebug *-> (
    RTMn is CX * max(1,(MemSizeTY/NBuckets)) 
               * (CY/MemSizeTY) 
               + CX * (CY/MemSizeTY),
    WTMn is CY,
    RTDn is 0,
    WTDn is 0,
    RPDn is (CX * ExtFlobSize1)/BDBPS + CX * max(1,(MemSizeTY/NBuckets)) 
                * (CY/MemSizeTY) * ExtFlobSize2/BDBPS/(1-FCMR),
    WPDn is 0,
    FNDn is 0,
    FDDn is 0,
    FODn is 0,
    FCDn is 0,
    Bn   is ResSize,
    MinPETn is CX * max(1,(MemSizeTY/NBuckets)) 
                  * (CY/MemSizeTY) - ResSize + (CX + CY),
    TCn  is ResSize,
    write('\n\nhashjoin('), write(X), write(','), write(Y), write(', _, _, '), 
    write(NBuckets), write('), '),
    write(P), write(','), write(Sel), write(','), write(ResSize), write(','), 
    write(TupleSize), write(','), 
    write(B), write(','), write(TC), write(','), write(Cost), write(') Case3'),
    write('\n\tRTM: '), write(RTMn),
    write('\n\tWTM: '), write(WTMn),
    write('\n\tRTD: '), write(RTDn),
    write('\n\tWTD: '), write(WTDn),
    write('\n\tRPD: '), write(RPDn),
    write('\n\tWPD: '), write(WPDn),
    write('\n\tFND: '), write(FNDn),
    write('\n\tFDD:  '), write(FDDn),
    write('\n\tFOD:  '), write(FODn),
    write('\n\tFCD:  '), write(FCDn),
    write('\n\tB  :  '), write(Bn),
    write('\n\tTC :  '), write(TCn), 
    write('\n\tMinPET:  '), write(MinPETn), 
    write('\n\tNeeded FlobSizes: '), write(ExtFlobSize1), write('/'), 
    write(ExtFlobSize2), nl
  );true).


/*
  This cost function for ~sortby~ depends on the operator's  implementation
  in ExtRelAlgPersistent, a variant of mergesort.
 
  For more accurate estimations, information on the distribution and order of 
  values of sort attributes would be needed.

  Operator ~sort~ applies ~sortby~ using lexicographical ordering on all attributes.

  Argument ~Order~ is either a float/integer representing the cost to evaluate the ordering predicate once,
  or a list of (Attrname SortOrder) elements.

*/

cost(sortby(X,Order), P, Sel, Card, TX, PredCost, TC, Cost) :- % XRIS: flob loading not yet considered here!
% A cost function for the ordering predicate built from Pred could be calculated
  cost(X, P, Sel, Card, TX, PredCost, TC, CostX),
  cost_factors(RTM, WTM, _, _, _, _, _, _, _, _, MaxMem, _, _),
  ((float(Order); integer(Order)) *-> B is Order; sortTC(B)),  % get cost for ordering predicate
  TX = tupleSizeData(TSCx, TSIx, _),
  Tx is TSCx + TSIx,
  HeapSize is MaxMem / (Tx + 12),      % determine heapsize in tuples
  % case1: complete relation fits into memory - no merging needed
  Card =< HeapSize,
  % write('\nCase1\n'),
  MCard is max(1,Card),                % ensure MCard >0
  Cost is CostX                        % produce argument stream
        + WTM * Card *0.5 *log(2,MCard)% inserts into heap 
        + RTM * Card                   % reads from heap 
        + B * Card * log(2,MCard),     % compare a pair of tuples (building the heap)
  (optDebug *-> (
    RTMn is Card,
    WTMn is Card *0.5 *log(2,MCard),
    RTDn is 0,
    WTDn is 0,
    RPDn is 0,
    WPDn is 0,
    FNDn is 0,
    FDDn is 0,
    FODn is 0,
    FCDn is 0,
    Bn   is Card * log(2,MCard),
    TCn  is 0,
    write('\n\nsortby('), write(X), write(','), write(Order), write(') Case1'),
    write('\n\tCostX: '), write(CostX),
    write('\n\tRTM: '), write(RTMn),
    write('\n\tWTM: '), write(WTMn),
    write('\n\tRTD: '), write(RTDn),
    write('\n\tWTD: '), write(WTDn),
    write('\n\tRPD: '), write(RPDn),
    write('\n\tWPD: '), write(WPDn),
    write('\n\tFND: '), write(FNDn),
    write('\n\tFDD:  '), write(FDDn),
    write('\n\tFOD:  '), write(FODn),
    write('\n\tFCD:  '), write(FCDn),
    write('\n\tB  :  '), write(Bn),
    write('\n\tTC :  '), write(TCn), nl
  ); true).


cost(sortby(X,Order), P, Sel, Card, TX, PredCost, TC, Cost) :-  % XRIS: flob loading not yet considered here!
% A cost function for the ordering predicate built from Pred could be calculated
  cost(X, P, Sel, Card, TX, PredCost, TC, CostX),             
  cost_factors(RTM, WTM, RTD, WTD, _, _, FND, FDD, FOD, FCD, MaxMem, _, _),
  ((float(Order); integer(Order)) *-> B is Order; sortTC(B)),   % get cost for ordering predicate
  TX = tupleSizeData(TSCx, TSIx, _),
  Tx is TSCx + TSIx,
  HeapSize is max(1,MaxMem / (Tx + 12)),           % determine heapsize in tuples
  % case2: relation does not fit into memory
  Card > HeapSize,
  PartCard is 2 * HeapSize,                        % calculate average cardinality of partion files generated
  PartNum is ceiling(max(0,(Card-HeapSize)) / max(1,PartCard)),  % calculate average number of partion files, 
  Cost is CostX                                    % produce argument stream
        + WTM * max(0,(Card-HeapSize))*0.5*log(2,HeapSize)  % inserts into heap =(Card-HeapSize)*0.5 * O(log HeapSize)
        + RTM * Card                               % reads from heap =Card * O(1)
        + WTD * max(0,(Card-HeapSize))             % write tuple to partition file (=Card-HeapSize)
        + RTD * max(0,(Card-HeapSize))             % read tuple from partition file (=Card-HeapSize)
        + (FND + FCD + FOD + FDD) * PartNum        % create, close, open and remove a partition file (=PartNum)
        + B * Card * log(2,HeapSize)               % compare a pair of tuples (partition step)
        + B * Card * log(2,PartNum+2),             % compare a pair of tuples (merge step)
  (optDebug *-> (
    RTMn is Card,
    WTMn is max(0,(Card-HeapSize))*0.5*log(2,HeapSize),
    RTDn is max(0,(Card-HeapSize)),
    WTDn is max(0,(Card-HeapSize)),
    RPDn is 0,
    WPDn is 0,
    FNDn is PartNum,
    FDDn is PartNum,
    FODn is PartNum,
    FCDn is PartNum,
    Bn   is Card * log(2,HeapSize) + Card * log(2,PartNum+2),
    TCn  is 0,
    write('\n\nsortby('), write(X), write(','), write(Order), write(') Case2'),
    write('\n\tCostX: '), write(CostX),
    write('\n\tRTM: '), write(RTMn),
    write('\n\tWTM: '), write(WTMn),
    write('\n\tRTD: '), write(RTDn),
    write('\n\tWTD: '), write(WTDn),
    write('\n\tRPD: '), write(RPDn),
    write('\n\tWPD: '), write(WPDn),
    write('\n\tFND: '), write(FNDn),
    write('\n\tFDD:  '), write(FDDn),
    write('\n\tFOD:  '), write(FODn),
    write('\n\tFCD:  '), write(FCDn),
    write('\n\tB  :  '), write(Bn),
    write('\n\tTC :  '), write(TCn), nl
  ); true).

cost(sort(X), P, Sel, CardX, TX, PredCost, TC, Cost) :- % 
% A cost function for the ordering predicate built from Pred could be calculated
  cost(sortby(X,lexall), P, Sel, CardX, TX, PredCost, TC, Cost).


/*
  Cost function for operator groupby, as defined in file ``ExtRelationAlgebra.cpp''

  Expects argument to be sorted on GroupAttr.
  Returns one extended tuple per group.

*/
cost(groupby(X, _, _), P, Sel, Card, TS, PredCost, TC, Cost) :- % XRIS: cost function missing!
%cost(groupby(X, GroupAttr, NewAttr-Fun-List), P, Sel, Card, TX, PredCost, TC, Cost) :-
  % for each tuple: check if GroupAttr-values matches predecessor, then store to buffer
  % for each group: for each NewAttr: calculate value by evaluating functions - similar to k-times ``extend''
  cost(X, P, Sel, Card, TS, PredCost, TC, Cost).
  

/*
  Cost function for operator ~rdup~. Similar to ~sort~, using comparison to test 
  on equality of tuples. Only the effords for forwarding and comparison of tuples
  is considered here.

  Expects a totally ordered input.

*/

cost(rdup(X), P, Sel, Card, TS, PredCost, TC, Cost) :-
  cost(X, P, Sel, CardX, TS, PredCost, TC, CostX),
  sortTC(B),
  Cost is CostX                  % produce argument stream
        + B * max(0,(CardX-1)),  % compare subsequent tuples for equality
  Card is CardX * Sel.


/*
  Detailed cost estimation for sortmergejoin

  A realistic cost estimation would need information, like histograms, 
  on the values of the join attributes. An option would be to recognize only
  those cases, that perform very bad, because the buffers overflow to disk.

  Problem: We do not know, how large the equivalence classes of $A \times B$ 
  with respect to ``A=B'' are. So, we even don't know how much of the buffers
  is used and how many tuples have to be temporarily stored to disk relations.

  For the sake of simplicity we assume, that all tuples with a common join 
  value fit into memory.

*/

cost(sortmergejoin(X, Y, _, _), P, Sel, ResultCard, 
     ResultTupleSize, B, TC, Cost) :-
  cost(sortby(X, B), P, 1, CX, TX, PredCost, TC, CostX),
  cost(sortby(Y, B), P, 1, CY, TY, PredCost, TC, CostY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, _, BDBPS, FCMR),
  addTupleSizes(TX,TY,ResultTupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  ResultCard is CX * CY * Sel,
  Cost is CostX + CostY                % producing and sorting the arguments
        + (CX * ExtFlobSize1)/BDBPS * RPD/FCMR % load flobs
        + (CY * ExtFlobSize2)/BDBPS * RPD/FCMR % load flobs
        + WTM * (CX+CY) * sqrt(Sel)    % write tuples to buffer/memory; A BAD ESTIMATION!
        + B   * (CX+CY)                % compare tuples 
        + RTM * (CX+CY) * sqrt(Sel)    % read tuples from buffer/memory; A BAD ESTIMATION!
        + TC  * ResultCard,            % create result tuple
  (optDebug *-> (
    RTMn is (CX+CY) * sqrt(Sel),
    WTMn is (CX+CY) * sqrt(Sel),
    RTDn is 0,
    WTDn is 0,
    RPDn is (CX * ExtFlobSize1)/BDBPS/FCMR + (CY * ExtFlobSize2)/BDBPS/FCMR,
    WPDn is 0,
    FNDn is 0,
    FDDn is 0,
    FODn is 0,
    FCDn is 0,
    Bn   is 0,
    TCn  is Sel * CX * CY,
    write('\n\nSortmergejoin('), write(X), write(','), write(Y), write(',_,_)'),
    write('\n\tCostX: '), write(CostX),
    write('\n\tCostY: '), write(CostY),
    write('\n\tRTM: '), write(RTMn),
    write('\n\tWTM: '), write(WTMn),
    write('\n\tRTD: '), write(RTDn),
    write('\n\tWTD: '), write(WTDn),
    write('\n\tRPD: '), write(RPDn),
    write('\n\tWPD: '), write(WPDn),
    write('\n\tFND: '), write(FNDn),
    write('\n\tFDD:  '), write(FDDn),
    write('\n\tFOD:  '), write(FODn),
    write('\n\tFCD:  '), write(FCDn),
    write('\n\tB  :  '), write(Bn),
    write('\n\tTC :  '), write(TCn),
    write('\nNeeded FlobSizes: '), write(ExtFlobSize1), write(ExtFlobSize2), nl
  ); true).



/*
  Cost function for mergejoin

  Same function as for ~sortmergejoin~, just without sorting the argument streams.

*/

cost(mergejoin(X, Y, _, _), P, Sel, ResultCard, ResultTupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, PredCost, TC, CostX),
  cost(Y, P, 1, CY, TY, PredCost, TC, CostY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, _, BDBPS, FCMR),
  addTupleSizes(TX,TY,ResultTupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  ResultCard is CX * CY * Sel,
  Cost is CostX + CostY                % producing and sorting the arguments
                                       % XRIS: Individual cost of ordering still not applied!
        + (CX * ExtFlobSize1)/BDBPS * RPD/FCMR % load flobs
        + (CY * ExtFlobSize2)/BDBPS * RPD/FCMR % load flobs
        + WTM * (CX+CY) * sqrt(Sel)    % write tuples to buffer/memory; A BAD ESTIMATION!
        + B   * (CX+CY)                % compare tuples (B could also be fixed, e.g. 0.001)
        + RTM * (CX+CY) * sqrt(Sel)    % read tuples from buffer/memory; A BAD ESTIMATION!
        + TC  * ResultCard.            % create result tuple

/*
   Quick & dirty cost function for Operator ~mergeunion~

*/

cost(mergeunion(X,Y), P, _, ResultCard, ResultTupleSize, PredCost, TC, Cost) :-
  cost(X, P, 1, CX, ResultTupleSize, PredCost, TC, CostX),
  cost(Y, P, 1, CY, ResultTupleSize, PredCost, TC, CostY),
  sortTC(A),
  ResultCard is CX + CY,               % neglect cardinality of intersection (similar to concat-operator)
  Cost is CostX + CostY                % producing and sorting the arguments
        + A * (CX + CY).               % compare tuples

/*
  Quick & dirty Cost function for Operator ~mergesec~

*/

cost(mergesec(X,Y), P, _, ResultCard, ResultTupleSize, PredCost, TC, Cost) :-
  cost(X, P, 1, CX, ResultTupleSize, PredCost, TC, CostX),
  cost(Y, P, 1, CY, ResultTupleSize, PredCost, TC, CostY),
  sortTC(A),
  ResultCard is min(CX,CY),            % not a very good estimation
  Cost is CostX + CostY                % producing and sorting the arguments
        + A * (CX + CY).               % compare tuples


/*
   Detailed cost function for ~symmjoin~

*/

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  % case1: X and Y fit into memory; OK
  (CX * (Tx+12)) =< (0.5 * MaxMem), (CY * (Ty+12)) =<  (0.5 * MaxMem),
  % write('\nSymmjoin Case1\n'),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  (CX =< CY -> (Min is CX, Max is CY, 
                MinFlob is ExtFlobSize1/BDBPS, 
                MaxFlob is ExtFlobSize2/BDBPS)
             ; (Min is CY, Max is CX, 
                MinFlob is ExtFlobSize2/BDBPS, 
                MaxFlob is ExtFlobSize1/BDBPS)),
  addTupleSizes(TX,TY,TupleSize),
  Cost is CostX + CostY                    % produce arguments
       + RPD * FCMR * (Min * (Min+1)/2 +       (Max-Min)) * MaxFlob % load flobs
       + RPD * FCMR * (Min * (Min+1)/2 + Min * (Max-Min)) * MinFlob % load flobs
       + WTM * 2 * Min                     % write X/Y-tuples to buffer in memory.
       + B   * Min * Max                   % check join condition
       + TC  * Min * Max * Sel             % create result tuples  
       + RTM * (Min*Min + Min*(Max-Min)),  % read X/Y-tuples from memory
  ResSize is CX * CY * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OY is min(CY,MaxMem/(2 * Ty)), % from this tuple+1 on, Y will be entirely buffered on disk
  % case 2.1: X fits in memory, but Y does not; CX =< CY; OY =< CX
  (CX * (Tx+12)) =< (0.5 * MaxMem), 
  (CY * (Ty+12)) > (0.5 * MaxMem), CX =< CY, OY =< CX,
  % write('\nSymmjoin Case2.1\n'),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  addTupleSizes(TX,TY,TupleSize), 
  Cost is CostX + CostY              % produce arguments
       + RPD * FCMR * (CX * (CX+1)/2 +      (CY-CX)) * ExtFlobSize2/BDBPS % load flobs
       + RPD * FCMR * (CX * (CX+1)/2 + CX * (CY-CX)) * ExtFlobSize1/BDBPS % load flobs
       + WTM * (CX + OY)             % write elements to buffer in memory
       + B * CX * CY                 % check join condition
       + TC * CX * CY * Sel          % create result tuples  
       + RTM * ( OY*OY               
                +gsf(OY,CX)
                +(CY-CX)*CX)         % read X/Y-tuples from memory
       + FND + FDD                   % create and delete tmp file
       + WTD * CX                    % write Y-tuples to disk
       + RTD * gsf(OY,CX),           % read Y-tuples from disk
  ResSize is CX * CY * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OY is min(CY,MaxMem/(2 * Ty)),    % from this tuple+1 on, Y will be entirely buffered on disk
  % case 2.2: X fits in memory, but Y does not; CX =< CY; OY > CX
  % this case has the same cost function as case1!
  (CX * (Tx+12)) =< (0.5 * MaxMem), 
  (CY * (Ty+12)) > (0.5 * MaxMem), CX =< CY, OY > CX,
  % write('\nSymmjoin Case2.2\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY              % produce arguments
       + RPD * FCMR * (CX * (CX+1)/2 +      (CY-CX)) * ExtFlobSize2/BDBPS % load flobs
       + RPD * FCMR * (CX * (CX+1)/2 + CX * (CY-CX)) * ExtFlobSize1/BDBPS % load flobs
       + WTM * (CX + CX)             % write X/Y-tuples to buffer in memory
       + B * CX * CY                 % check join condition
       + TC * CX * CY * Sel          % create result tuples  
       + RTM * ( CX*CX
               +(CY-CX)*CX),         % read X/Y-tuples from memory
  ResSize is CX * CY * Sel.
 
cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, _, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OY is min(CY,MaxMem/(2 * Ty)),    % from this tuple+1 on, Y will be entirely buffered on disk
  % case 2.3: X fits in memory, but Y does not; CX > CY
  (CX * (Tx+12)) =< (0.5 * MaxMem), 
  (CY * (Ty+12)) > (0.5 * MaxMem), CX > CY,
  % write('\nSymmjoin Case2.3\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY              % produce arguments
       + RPD * FCMR * (CY * (CY+1)/2 +      (CX-CY)) * ExtFlobSize1/BDBPS % load flobs
       + RPD * FCMR * (CY * (CY+1)/2 + CY * (CX-CY)) * ExtFlobSize2/BDBPS % load flobs
       + WTM * (CY + OY)             % write X/Y-tuples to buffer in memory
       + B * CX * CY                 % check join condition
       + TC * CX * CY * Sel          % create result tuples  
       + RTM * ( OY*OY
              +gsf(OY,CY))           % read X/Y-tuples from memory
       + FND + FDD                   % create and delete tmp file
       + WTD * CY                    % write Y-tuples to disk
       + RTD * ( gsf(OY,CY)
              +(CX-CY)*CY),          % read Y-tuples from disk
  ResSize is CX * CY * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OX is min(CX,MaxMem/(2 * Tx)), % from this tuple+1 on, X will be entirely buffered on disk
  % case 3.1: Y fits in memory, but X does not; CY =< CX; OX =< CY
  (CY * (Ty+12)) =< (0.5 * MaxMem), 
  (CX * (Tx+12)) > (0.5 * MaxMem), CY =< CX, OX =< CY, 
  % write('\nSymmjoin Case3.1\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY              % produce arguments
       + RPD * FCMR * (CY * (CY+1)/2 +      (CX-CY)) * ExtFlobSize1/BDBPS % load flobs
       + RPD * FCMR * (CY * (CY+1)/2 + CY * (CX-CY)) * ExtFlobSize2/BDBPS % load flobs
       + WTM * (CY + OX)             % write elements to buffer in memory
       + B * CY * CX                 % check join condition
       + TC * CY * CX * Sel          % create result tuples  
       + RTM * ( OX*OX               
              +gsf(OX,CY)
              +(CX-CY)*CY)           % read X/Y-tuples from memory
       + FND + FDD                   % create and delete tmp file
       + WTD * CY                    % write X-tuples to disk
       + RTD * gsf(OX,CY),           % read X-tuples from disk
  ResSize is CY * CX * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OX is min(CX,MaxMem/(2 * Tx)),    % from this tuple+1 on, X will be entirely buffered on disk
  % case 3.2: Y fits in memory, but X does not; CY =< CX; OX > CY
  % this case has the same cost function as case1!
  (CY * (Ty+12)) =< (0.5 * MaxMem), 
  (CX * (Tx+12)) > (0.5 * MaxMem), CY =< CX, OX > CY,
  % write('\nSymmjoin Case3.2\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY              % produce arguments
       + RPD * FCMR * (CY * (CY+1)/2 +      (CX-CY)) * ExtFlobSize1/BDBPS % load flobs
       + RPD * FCMR * (CY * (CY+1)/2 + CY * (CX-CY)) * ExtFlobSize2/BDBPS % load flobs
       + WTM * (CY + CY)             % write X/Y-tuples to buffer in memory
       + B * CY * CX                 % check join condition
       + TC * CY * CX * Sel          % create result tuples  
       + RTM * ( CY*CY
              +(CX-CY)*CY),          % read X/Y-tuples from memory
  ResSize is CY * CX * Sel.
 
cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OX is min(CX,MaxMem/(2 * Tx)),    % from this tuple+1 on, X will be entirely buffered on disk
  % case 3.3: Y fits in memory, but X does not; CY > CX
  (CY * (Ty+12)) =< (0.5 * MaxMem), (CX * (Tx+12)) > (0.5 * MaxMem), CY > CX,
  % write('\nSymmjoin Case3.3\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY              % produce arguments
       + RPD * FCMR * (CX * (CX+1)/2 +      (CY-CX)) * ExtFlobSize2/BDBPS % load flobs
       + RPD * FCMR * (CX * (CX+1)/2 + CX * (CY-CX)) * ExtFlobSize1/BDBPS % load flobs
       + WTM * (CX + OX)             % write X/Y-tuples to buffer in memory
       + B * CY * CX                 % check join condition
       + TC * CY * CX * Sel          % create result tuples  
       + RTM * ( OX*OX
              +gsf(OX,CX))           % read X/Y-tuples from memory
       + FND + FDD                   % create and delete tmp file
       + WTD * CX                    % write X-tuples to disk
       + RTD * ( gsf(OX,CX)
                +(CY-CX)*CX),        % read X/Y-tuples from disk
  ResSize is CY * CX * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OX is min(CX,MaxMem/(2 * Tx)), % from this tuple+1 on, X will be entirely buffered on disk
  OY is min(CY,MaxMem/(2 * Ty)), % from this tuple+1 on, Y will be entirely buffered on disk
  % case 4.1: neither X nor Y fit into memory; CX =< CY, OX =< OY
  (CX * (Tx+12)) > (0.5 * MaxMem), 
  (CY * (Ty+12)) > (0.5 * MaxMem), CX =< CY, OX =< OY,
  % write('\nSymmjoin Case4.1\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY
       + RPD * FCMR * (CX * (CX+1)/2 +      (CY-CX)) * ExtFlobSize2/BDBPS % load flobs
       + RPD * FCMR * (CX * (CX+1)/2 + CX * (CY-CX)) * ExtFlobSize1/BDBPS % load flobs
       + WTM * (OX + OY)             % write elements to buffer in memory
       + B * CX * CY                 % check join condition
       + TC * CX * CY * Sel          % create result tuples  
       + RTM * ( OX*OX                 
              +gsf(OX,OY)            
              +(CY-CX)*CX)           % read X/Y-tuples from memory
       + 2 * (FND + FDD)             % create and delete tmp files
       + WTD * (CX*CY)               % write X/Y-tuples to disk
       + RTD * ( gsf(OX,OY)            
              +gsf(OY,CX)            
              +(CY-CX)*CX),          % read X/Y-tuples from disk
  ResSize is CX * CY * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OX is min(CX,MaxMem/(2 * Tx)), % from this tuple+1 on, X will be entirely buffered on disk
  OY is min(CY,MaxMem/(2 * Ty)), % from this tuple+1 on, Y will be entirely buffered on disk
  % case 4.2: neither X nor Y fit into memory; CX =< CY, OX > OY
  (CX * (Tx+12)) > (0.5 * MaxMem), 
  (CY * (Ty+12)) > (0.5 * MaxMem), CX =< CY, OX > OY,
  % write('\nSymmjoin Case4.2\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY
       + RPD * FCMR * (CX * (CX+1)/2 +      (CY-CX)) * ExtFlobSize2/BDBPS % load flobs
       + RPD * FCMR * (CX * (CX+1)/2 + CX * (CY-CX)) * ExtFlobSize1/BDBPS % load flobs
       + WTM * (OX + OY)             % write elements to buffer in memory.
       + B * CX * CY                 % check join condition
       + TC * CX * CY * Sel          % create result tuples  
       + RTM * ( OY*OY
              +gsf(OY,CX)
              +(CY-CX)*CX)           % read X/Y-tuples from memory
       + 2 * (FND + FDD)             % create and delete tmp files
       + WTD * (CX*CY)               % write X/Y-tuples to disk
       + RTD * ( gsf(OY,OX)
                +gsf(OX,CX)
                +(CY-CX)*CX),        % read X/Y-tuples from disk
  ResSize is CX * CY * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OX is min(CX,MaxMem/(2 * Tx)), % from this tuple+1 on, X will be entirely buffered on disk
  OY is min(CY,MaxMem/(2 * Ty)), % from this tuple+1 on, Y will be entirely buffered on disk
  % case 4.3: neither X nor Y fit into memory; CY =< CX, OY =< OX
  (CX * (Tx+12)) > (0.5 * MaxMem), 
  (CY * (Ty+12)) > (0.5 * MaxMem), CY =< CX, OY =< OX,
  % write('\nSymmjoin Case4.3\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY
       + RPD * FCMR * (CY * (CY+1)/2 +      (CX-CY)) * ExtFlobSize1/BDBPS % load flobs
       + RPD * FCMR * (CY * (CY+1)/2 + CY * (CX-CY)) * ExtFlobSize2/BDBPS % load flobs
       + WTM * (OY + OX)             % write elements to buffer in memory.
       + B * CY * CX                 % check join condition
       + TC * CY * CX * Sel          % create result tuples  
       + RTM * ( OY*OY
              +gsf(OY,OX)
              +(CX-CY)*CY)           % read X/Y-tuples from memory
       + 2 * (FND + FDD)             % create and delete tmp files
       + WTD * (CY*CX)               % write X/Y-tuples to disk
       + RTD * ( gsf(OY,OX)
                +gsf(OX,CY)
                +(CX-CY)*CY),        % read X/Y-tuples from disk
  ResSize is CY * CX * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OX is min(CX,MaxMem/(2 * Tx)), % from this tuple+1 on, X will be entirely buffered on disk
  OY is min(CY,MaxMem/(2 * Ty)), % from this tuple+1 on, Y will be entirely buffered on disk
  % case 4.4: neither X nor Y fit into memory; CY =< CX, OY > OX
  (CX * (Tx+12)) > (0.5 * MaxMem), 
  (CY * (Ty+12)) > (0.5 * MaxMem), CY =< CX, OY > OX,
  % write('\nSymmjoin Case4.4\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY
       + RPD * FCMR * (CY * (CY+1)/2 +      (CX-CY)) * ExtFlobSize1/BDBPS % load flobs
       + RPD * FCMR * (CY * (CY+1)/2 + CY * (CX-CY)) * ExtFlobSize2/BDBPS % load flobs
       + WTM * (OY + OX)             % write elements to buffer in memory.
       + B * CY * CX                 % check join condition
       + TC * CY * CX * Sel          % create result tuples  
       + RTM * ( OX*OX
              +gsf(OX,CY)
              +(CX-CY)*CY)           % read X/Y-tuples from memory
       + 2 * (FND + FDD)             % create and delete tmp files
       + WTD * (CY*CX)               % write X/Y-tuples to disk
       + RTD * ( gsf(OX,OY)
                +gsf(OY,CY)
                +(CX-CY)*CY),        % read X/Y-tuples from disk
  ResSize is CX * CY * Sel.

cost(extend(X, _), P, Sel, S, TupleSize, PredCost, TC, C) :-
  cost(X, P, Sel, S, TupleSize1, PredCost, TC, C1),
  addTupleSizes(TupleSize1, tupleSizeData(12,0,0), TupleSize), % XRIS: TupleSize should be increased by the real extension size
  extendTC(A),
  C is C1 + A * S.                   % XRIS: costs for evaluation of mapping to generate value 
                                     %       for new attributes not yet considered!

cost(remove(X, _), P, Sel, S, TupleSize, PredCost, TC, C) :-
  cost(X, P, Sel, S, TupleSize1, PredCost, TC, C1),
  addTupleSizes(TupleSize1,tupleSizeData(-12,0,0), TupleSize), % XRIS: TupleSize should be decreased by the real removal size
  removeTC(A),
  C is C1 + A * S.

cost(project(X, _), P, Sel, S, TupleSize, PredCost, TC, C) :-
  cost(X, P, Sel, S, TupleSize, PredCost, TC, C1), % XRIS: TupleSize should be reduced according to the projection!
  projectTC(A,B),
  TupleSize = tupleSizeData(TSC, TSI, _),
  C is C1 + max(A*S, B*S*(TSC+TSI)).

cost(rename(X, _), P, Sel, S, TupleSize, PredCost, TC, C) :-
  cost(X, P, Sel, S, TupleSize, PredCost, TC, C1),
  renameTC(A),
  C is C1 + A * S.

cost(head(X, N), P, Sel, Card, TupleSize, PredCost, TC, Cost) :-
  cost(X, P, Sel, CX, TupleSize, PredCost, TC, C1),
  aggregateTC(B),
  Card is min(N, CX),
  Cost is C1                              % create argument
        + B * Card.                       % overhead for counting each tuple

cost(counter(X, _), P, Sel, S, TupleSize, PredCost, TC, C) :-
  cost(X, P, Sel, S, TupleSize, PredCost, TC, C).


%fapra1590
% See cost(leftrange(...),...) for comments on future refinements.
cost(windowintersects(_, Rel, _), P, Sel, Size, TupleSize, B, TC, Cost) :-
  cost(Rel, P, 1, RelSize, TupleSize, TC, B, _),
  cost_factors(_,_,_,_,RPD,_,_,_,FOD,FCD,_, _, _),
  Size is Sel * RelSize,
  Cost is FOD + FCD                       % open/close RTree file        
        + B * (log(15,RelSize)+Size)      % find all matches,           later: B * (Height + K/2)
        + RPD * log(15,RelSize)           % read index pages from disk, later: D * (Height + Sel*RelSize/K)
        + TC * Sel * RelSize.             % produce tuple stream


/*

  Cost function for operator spatialjoin as defined in file PlugJoinAlgebra.cpp.
  The smaller relation should be passed as the inner relation (Y), as it is used
  to build an index (R[*]Tree), while tuples from the outer relation are queried 
  against the index. The join performs a test on intersection of BBoxes.

  1 All records of Y fit into memory. getMaxLeaves() returns infinite, number of 
    leaves remains unlimited. 

  2 Y does not completely fit into memory. X and Y will be partitioned pairwise. 
    Each pair of partitions corresponds to one leaf of the index, getMaxLeaves() 
    determines how many pairs of partitions are created.
    As long as getMaxLeaves() is not reached, tuples from Y are inserted into
    the Rtree. After the treshold getMaxLeaves() is passed, any overflow in a 
    leaf does not result in a structural change of the tree, but the tuple is 
    simply written to into the according Y-partition file. Partition files are 
    organized in buckets of size MaxMem/#leaves.
    
    When all Y-tuples have been processed, each leaf in the RTree is examined. 
    If its Y-partition is not empty, all tuples from that leaf are flushed to 
    that Y-partition.

    Now, all tuples from X are processed: Each tuple is queried against the 
    Rtree and directed to all leaves and Y-partitions that might contain join
    partners (replication of x may occur). If a leaf is empty, x is spooled to
    the S-partition of that leaf, otherwise the join results are immeadeatly 
    reported.

    When all queries are processed, the Rtree is deleted and the operator is 
    restarted recursively for each partition with non-empty S- and R-part on 
    disk.

    Splitting an R[star]-tree costs $O(d \cdot M \log_2 M)$, for node capacity 
    $M$, number of dimensions $d$. 

    $\mbox{height}(Y) = \log_M (Y)$, $\mbox{nodes}(Y) = M^{\mbox{height}(Y)-1}$ 
    
    Creating an R[star]-tree ideally causes $\mbox{nodes}(Y) = M^{\mbox{height}(Y)-1}-1$ 
    splits.

*/


cost(spatialjoin(X,Y,_,_,D), P, Sel, Size, TupleSize, B, TC, Cost) :- % XRIS: Flobs not yet considered
  % D is the dimension of the joinattributes' bounding boxes
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM,WTM,_,_,RPD,WPD,FND,FDD,FOD,FCD,MaxMem,BDBPS,FCMR),
  TX = tupleSizeData(TSCx, TSIx, TSEx),
  TY = tupleSizeData(TSCy, TSIy, TSEy),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2), % Flobs should not be needed...
  Size is Sel * CX * CY,
  MaxLeaves  is max(0,(MaxMem/(Ty+24))),        % determine number of leaves fitting into in-memory Rtree
  PartNumber is max(0,ceiling(CY/MaxLeaves)-1), % determine how many partitions will be created
  ( PartNumber > 1 -> 
               (
                  PartSizeY  is (CY-MaxLeaves)/PartNumber, % determine average size of spooled partitions
                  PartSizeX  is (CX/CY)*PartSizeY, 
                  % estimate cost for one recursive call:
                  cost(spatialjoin(fixed(PartSizeX,TX),
                       fixed(PartSizeY,TY),_,_,D), P, Sel, _, _, B, TC, CostR)
               )
	      ;(
                  CostR is 0
               )
  ),
         
/*
  MaxLeavesOfRtree      is 10000, % parameter found in PlugJoinAlgebra.h
  MinLeaveOfRtree       is    40, % parameter found in PlugJoinAlgebra.h
  DefaultEntriesPerNode is    15, % parameter found in PlugJoinAlgebra.h
  ScalingFactor         is    17, % parameter found in PlugJoinAlgebra.h
  NodesToUse is max(min((ScalingFactor*(CX+CY)/(DefaultEntriesPerNode*MaxLeavesOfRtree)+1.0),
                         MaxLeavesOfRtree),
                    MinLeaveOfRtree), % found in PlugAndJoinAlgebra.cpp

*/
  Height = max(1,log(15,min(MaxLeaves,CY))),% fanout/node capacity set to 15 
  M = min(MaxLeaves, CY),
  INSERT_CONST is (RTM + WTM),  
  QUERY_CONST  is RTM * Height,
  SPLIT_CONST  is (RTM + WTM) * D * M * Height,

  Cost is CostX + CostY                     % produce arguments
        + INSERT_CONST * max(0,(CY-M))      % insert Y-tuples into Rtree
        + SPLIT_CONST * (15 ** (Height-1))  % resolve splits on insertions
        + QUERY_CONST * CX                  % query X-tuple against Rtree  
                                            % spool Y-tuples to partition file
        + (WPD+RPD) * ceiling((CY-M)*(Ty+TSEy)/BDBPS)
                                            % spool X-tuples to partition file 
        + (WPD+RPD) * ceiling(CX * (1 - M/CY)*(Tx+TSEx)/BDBPS)
        + (FND+FOD+FCD+FDD) * PartNumber    % handle partitions on disk
        + CostR * PartNumber                % cost for recursive calls
        + TC * Sel * M * CX * M/CY.         % create result tuples
  

/*
8.2 Creating Cost Edges

These are plan edges extended by a cost measure.

*/

createCostEdge :-
  planEdge(Source, Target, Term, Result),
  edge(Source, Target, ETerm, _, _, _),
  (ETerm = join(_,_,Predicate); ETerm = select(_,Predicate)),
  edgeSelectivity(Source, Target, Sel, PredCost, TupleGenerationCost),
  cost(Term,Predicate,Sel,Size,TupleSize,PredCost,TupleGenerationCost,Cost),
  assert(costEdge(Source, Target, Term, Result, Size, TupleSize, Cost)),
  fail.

createCostEdges :- not(createCostEdge).

deleteCostEdge :-
  retract(costEdge(_, _, _, _, _, _, _)), fail.

deleteCostEdges :- not(deleteCostEdge).

costEdgeInfo(Edge) :-
  Edge = costEdge(Source, Target, Plan, Result, Size, TupleSize, Cost),
  costEdgeInfo(Source, Target, Plan, Result, Size, TupleSize, Cost).

costEdgeInfo(Source, Target, Plan, Result, Size, TupleSize, Cost) :-
  nl, write('Source:    '), write(Source),
  nl, write('Target:    '), write(Target),
  nl, write('Plan:      '), wp(Plan),
  nl, write('           '), write(Plan),  % for testing
  nl, write('Result:    '), write(Result), 
  nl, write('Size:      '), write(Size),
  nl, write('TupleSize: '), write(TupleSize),
  nl, write('Cost:      '), write(Cost), nl. 

writeCostEdges2 :-
  costEdge(Source, Target, Plan, Result, Size, TupleSize, Cost),
  costEdgeInfo(Source, Target, Plan, Result, Size, TupleSize, Cost), nl,
  fail.

writeCostEdges :- not(writeCostEdges2).

/*
----    assignCosts
----

This just puts together creation of sizes and cost edges.

*/

assignCosts :-
  assignSizes,
  createCostEdges.


/*
9 Finding Shortest Paths = Cheapest Plans

The cheapest plan corresponds to the shortest path through the predicate order
graph.

9.1 Shortest Path Algorithm by Dijkstra

We implement the shortest path algorithm by Dijkstra. There are two  
relevant sets of nodes: 
 
  * center: the nodes for which shortest paths have already been
computed 

  * boundary: the nodes that have been seen, but that have not yet been  
expanded. These need to be kept in a priority queue. 
 
A node, as used during shortest path computation, is represented as a term 

----    node(Name, Distance, Path)
----

where ~Name~ is the node number, ~Distance~ the distance along the shortest
path to this node, and ~Path~ is the list of edges forming the shortest path.
   
The graph is represented by the set of ~costEdges~. 

The center is represented as a set of facts of the form

----    center(NodeNumber, node(Name, Distance, Path))
----

Since predicates are generally indexed by their first argument, finding a node
in the center via the node number should be very efficient. We assume it is
possible in constant time.

The boundary is represented by an abstract data type as described in the
interface below. Essentially it is a priority queue implementation.

 
----    successor(Node, Succ) :- 
----

~Succ~ is a successor of node ~Node~ via some edge. This includes computation
of the distance and path of the successor.

*/ 

successor(node(Source, Distance, Path), node(Target, Distance2, Path2)) :-
  costEdge(Source, Target, Term, Result, Size, TupleSize, Cost),
  Distance2 is Distance + Cost,
  append(Path,[costEdge(Source,Target,Term,Result,Size,TupleSize,Cost)],Path2).

/*

----    dijkstra(Source, Dest, Path, Length) :- 
----

The shortest path from ~Source~ to ~Dest~ is ~Path~ of length ~Length~.

*/
 
dijkstra(Source, Dest, Path, Length) :- 
  emptyCenter,
  b_empty(Boundary),
  b_insert(Boundary, node(Source, 0, []), Boundary1),
  dijkstra1(Boundary1, Dest, 0, notfound),
  center(Dest, node(Dest, Length, Path)).

emptyCenter :- not(emptyCenter1).

emptyCenter1 :- retract(center(_, _)), fail.

 
/* 
----    dijkstra1(Boundary, Dest, NoOfCalls) :-
----

Compute the shortest paths to all nodes and store them in a predicate
~center~. Initially to be called with no fact ~center~ asserted, and ~Boundary~
just containing the start node.

For testing we check at which iteration the destination ~Dest~ is reached.

*/
 
dijkstra1(Boundary, _, _, found) :- !,
        tree_height(Boundary, H),
        write('Height of search tree for boundary is '), write(H), nl.

dijkstra1(Boundary, _, _, _) :- b_isEmpty(Boundary).
dijkstra1(Boundary, Dest, N, _) :-
        % write('Boundary = '), writeList(Boundary), nl, write('====='), nl, 
  b_removemin(Boundary, Node, Bound2),
  Node = node(Name, _, _),
  assert(center(Name, Node)),
  checkDest(Name, Dest, N, Found),
  putsuccessors(Bound2, Node, Bound3),
  N1 is N+1,
  dijkstra1(Bound3, Dest, N1, Found).

checkDest(Name, Name, N, found) :- write('Destination node '), write(Name),
  write(' reached at iteration '), write(N), nl.

checkDest(_, _, _, notfound).


/*
Some auxiliary functions for testing:

*/

writeList([]).
writeList([X | Rest]) :- nl, nl, write('-----'), nl, write(X), writeList(Rest).

writeCenter :- not(writeCenter1).
writeCenter1 :-
  center(_, node(Name, Distance, Path)),
  write('Node: '), write(Name), nl,
  write('Cost: '), write(Distance), nl,
  write('Path: '), nl, writePath(Path), nl, fail.

writePath([]).
writePath([costEdge(Source,Target,Term,Result,Size,TupleSize,Cost) | Path]) :-
  write(costEdge(Source, Target, Result, Size, TupleSize, Cost)), nl,
  write('    '), wp(Term), nl,
  writePath(Path).

/*
----    putsuccessors(Boundary, Node, BoundaryNew) :-
----

Insert into ~Boundary~  all successors of node ~Node~ not yet present in
the center, updating their distance if they are already present, to obtain
~BoundaryNew~. 

*/   
putsuccessors(Boundary, Node, BoundaryNew) :-

  % comment out next line for debugging infos
  % succInfo(Node),

  findall(Succ, successor(Node, Succ), Successors), !,
  putsucc1(Boundary, Successors, BoundaryNew). 


/* 
Some predicates printing debugging output.

*/  

nodeInfo(Node) :-
  node(Nr, D, L) = Node,
  nl, write('Node      : '), write(Nr), 
  nl, write('Distance  : '), write(D),
  nl, write('Cost edges: '), nl, 
  showCostEdges(L).

showCostEdges([]) :- nl.
showCostEdges([H|T]) :- 
  costEdgeInfo(H), showCostEdges(T).

showSuccList(Node) :- successor(Node, Succ), nodeInfo(Succ), fail.

succInfo(Node) :-
  nl, write('===> computing successors of'), nl,
  nodeInfo(Node),
  nl, nl, write('List of Successors:'), nl,
  not(showSuccList(Node)), nl, nl.



/* 
----    putsucc1(Boundary, Successors, BoundaryNew) :- 
----

put all successors not yet in the center from the list ~Successors~ into the
~Boundary~ to get ~BoundaryNew~. The four cases to be distinguished are:

  * The list of successors is empty.

  * The first successor is already in the center, hence the shortest path to it
is already known and it does not need to be inserted into the boundary.

  * The first successor is already present in the boundary, at a smaller or
equal distance than the one via the curent edge. It can also be ignored.

  * The first succesor exists in the boundary, but at a higher distance. In
this case it replaces the previous node entry in the boundary.

  * The first successor does not exist in the boundary. It is inserted.

*/   

putsucc1(Boundary, [], Boundary).
 
putsucc1(Boundary, [node(N, _, _) | Successors], BNew) :- 
  center(N, _), !, 
  putsucc1(Boundary, Successors, BNew). 

putsucc1(Boundary, [node(N, D, _) | Successors], BNew) :-  
  b_memberByName(Boundary, N, node(N, DistOld, _)), 
  DistOld =< D, !, 
  putsucc1(Boundary, Successors, BNew). 

putsucc1(Boundary, [node(N, D, P) | Successors], BNew) :-
  b_memberByName(Boundary, N, node(N, DistOld, _)), 
  D < DistOld, !, 
  b_deleteByName(Boundary, N, Bound2), 
  b_insert(Bound2, node(N, D, P), Bound3), 
  putsucc1(Bound3, Successors, BNew). 

putsucc1(Boundary, [node(N, D, P) | Successors], BNew) :-  
  b_insert(Boundary, node(N, D, P), Bound2), 
  putsucc1(Bound2, Successors, BNew). 

  
/* 

9.2 Interface ~Boundary~
 
The boundary is represented in a data structure with the following 
operations:

----    b_empty(-Boundary) :-
----

Creates an empty boundary and returns it.

----    b_isEmpty(+Boundary) :-
----

Checks whether the boundary is empty.


----    b_removemin(+Boundary, -Node, -BoundaryOut) :-
---- 

Returns the node ~Node~ with minimal distance from the set ~Boundary~ and
returns also ~BoundaryOut~ where this node is removed.
   
----    b_insert(+Boundary, +Node, -BoundaryOut) :-
----
 
Inserts a node that must not yet be present (i.e., no other node of that
name).
   
----    b_memberByName(+Boundary, +Name, -Node) :-
---- 
 
If a node ~Node~ with name ~Name~ is present, it is returned. 
 
----    b_deleteByName(+Boundary, +Name, -BoundaryOut) :-
---- 
 
Returns the boundary, where the node with name ~Name~ is deleted. 
 
*/

/*
9.3 Constructing the Plan from the Shortest Path

----    plan(Path, Plan)
----

The plan corresponding to ~Path~ is ~Plan~.

*/
plan(Path, Plan) :-
  deleteNodePlans,
  traversePath(Path),
  highestNode(Path, N), 
  nodePlan(N, Plan).


deleteNodePlans :- not(deleteNodePlan).

deleteNodePlan :- retract(nodePlan(_, _)), fail.


traversePath([]).

traversePath([costEdge(_, _, Term, Result, _, _, _) | Path]) :-
  embedSubPlans(Term, Term2),
  assert(nodePlan(Result, Term2)),
  traversePath(Path).


embedSubPlans(res(N), Term) :-
  nodePlan(N, Term), !.

embedSubPlans(Term, Term2) :- 
  compound(Term), !,
  Term =.. [Functor | Args],
  embedded(Args, Args2),
  Term2 =.. [Functor | Args2].

embedSubPlans(Term, Term).


embedded([], []).

embedded([Arg | Args], [Arg2 | Args2]) :-
  embedSubPlans(Arg, Arg2),
  embedded(Args, Args2).


highestNode(Path, N) :-
  reverse(Path, Path2),
  Path2 = [costEdge(_, N, _, _, _, _, _) | _].


/*
9.4 Computing the Best Plan for a Given Predicate Order Graph

*/

bestPlan :-
  assignCosts,
  highNode(N), 
  dijkstra(0, N, Path, Cost),
  setCurrentResultSizes,
  writePath(Path),
  plan(Path, Plan),
  write('The best plan is:'), nl, nl,
  wp(Plan),
  nl, nl,
  write('The cost is: '), write(Cost), nl.

bestPlan(Plan, Cost) :-
  nl, write('Computing best Plan ...'), nl,
  assignCosts,
  highNode(N), 
  dijkstra(0, N, Path, Cost),
  setCurrentResultSizes,
  plan(Path, Plan).

/*
10 A Larger Example

It is now time to test efficiency with a larger example. We consider the query:

----    select *
        from Staedte, plz as p1, plz as p2, plz as p3,
        where SName = p1.Ort
          and p1.PLZ = p2.PLZ + 1
          and p2.PLZ = p3.PLZ * 5
          and Bev > 300000
          and Bev < 500000
          and p2.PLZ > 50000
          and p2.PLZ < 60000
          and Kennzeichen starts "W"
          and p3.Ort contains "burg"
          and p3.Ort starts "M"
----

This translates to:

*/

example6 :- pog(
  [rel(staedte, *, u), rel(plz, p1, l), rel(plz, p2, l), rel(plz, p3, l)],
  [
    pr(attr(sName,1,u) = attr(p1:ort,2,u), rel(staedte,*,u), rel(plz,p1,l)),
    pr(attr(p1:pLZ,1,u) = (attr(p2:pLZ,2,u) + 1), rel(plz,p1,l), rel(plz,p2,l)),
    pr(attr(p2:pLZ,1,u) = (attr(p3:pLZ,2,u) * 5), rel(plz,p2,l), rel(plz,p3,l)),
    pr(attr(bev, 1, u) > 300000,  rel(staedte, *, u)),
    pr(attr(bev, 1, u) < 500000,  rel(staedte, *, u)),
    pr(attr(p2:pLZ, 1, u) > 50000,  rel(plz, p2, l)),
    pr(attr(p2:pLZ, 1, u) < 60000,  rel(plz, p2, l)),
    pr(attr(kennzeichen, 1, u) starts "W",  rel(staedte, *, u)),
    pr(attr(p3:ort, 1, u) contains "burg",  rel(plz, p3, l)),
    pr(attr(p3:ort, 1, u) starts "M",  rel(plz, p3, l))
  ],
  _, _).

/*
This doesn't work (initially, now it works). Let's keep the numbers a bit
smaller and avoid too many big joins first.

*/
example7 :- pog(
  [rel(staedte, *, u), rel(plz, p1, l)],
  [
    pr(attr(sName,1,u) = attr(p1:ort,2,u), rel(staedte,*,u), rel(plz,p1,l)),
    pr(attr(bev, 0, u) > 300000,  rel(staedte, *, u)),
    pr(attr(bev, 0, u) < 500000,  rel(staedte, *, u)),
    pr(attr(p1:pLZ, 0, u) > 50000,  rel(plz, p1, l)),
    pr(attr(p1:pLZ, 0, u) < 60000,  rel(plz, p1, l)),
    pr(attr(kennzeichen, 0, u) starts "F",  rel(staedte, *, u)),
    pr(attr(p1:ort, 0, u) contains "burg",  rel(plz, p1, l)),
    pr(attr(p1:ort, 0, u) starts "M",  rel(plz, p1, l))
  ],
  _, _).

example8 :- pog(
  [rel(staedte, *, u), rel(plz, p1, l), rel(plz, p2, l)],
  [
    pr(attr(sName,1,u) = attr(p1:ort,2,u), rel(staedte,*,u), rel(plz,p1,l)),
    pr(attr(p1:pLZ,1,u) = (attr(p2:pLZ,2,u) + 1), rel(plz,p1,l), rel(plz,p2,l)),
    pr(attr(bev, 0, u) > 300000,  rel(staedte, *, u)),
    pr(attr(bev, 0, u) < 500000,  rel(staedte, *, u)),
    pr(attr(p1:pLZ, 0, u) > 50000,  rel(plz, p1, l)),
    pr(attr(p1:pLZ, 0, u) < 60000,  rel(plz, p1, l)),
    pr(attr(kennzeichen, 0, u) starts "F",  rel(staedte, *, u)),
    pr(attr(p1:ort, 0, u) contains "burg",  rel(plz, p1, l)),
    pr(attr(p1:ort, 0, u) starts "M",  rel(plz, p1, l))
  ],
  _, _).

/*
Let's study a small example again with two independent conditions.

*/

example9 :- pog([rel(staedte, s, u), rel(plz, p, l)],
  [pr(attr(p:ort, 2, u) = attr(s:sName, 1, u),
        rel(staedte, s, u), rel(plz, p, l) ),
   pr(attr(p:pLZ, 0, u) > 40000, rel(plz, p, l)),
   pr(attr(s:bev, 0, u) > 300000, rel(staedte, s, u))], _, _).

example10 :- pog(
  [rel(staedte, *, u), rel(plz, p1, l), rel(plz, p2, l), rel(plz, p3, l)],
  [
    pr(attr(sName, 1,u) = attr(p1:ort, 2,u), rel(staedte,*,u), rel(plz,p1,l)),
    pr(attr(p1:pLZ,1,u) = (attr(p2:pLZ,2,u) + 1), rel(plz,p1,l), rel(plz,p2,l)),
    pr(attr(p2:pLZ,1,u) = (attr(p3:pLZ,2,u) * 5), rel(plz,p2,l), rel(plz,p3,l))
  ],
  _, _).

/*
11 A User Level Language

We have started to construct the optimizer by building the predicate order
graph, using a notation for relations and predicates as useful for that
purpose. Later, in [Section Translation], we have adapted the notation to be
able to translate and construct query plans as needed in Secondo. In this
section we will introduce a more user friendly notation for queries, pretty
similar to SQL, but suitable for being written directly in PROLOG.

11.1 The Language

The basic select-from-where statement will be written as

----    select <attr-list>
        from <rel-list>
        where <pred-list>
----

The first example query from [Section 4.1.1] can then be written as:

----    select [sname, bev]
        from [staedte]
        where [bev > 500000]
----

Instead of lists consisting of a single element we will also support writing
just the element, hence the query can also be written:

----    select [sname, bev]
        from staedte
        where bev > 500000
----

The second query can be written as:

----    select *
        from [staedte as s, plz as p]
        where [sname = p:ort, p:plz > 40000]
----

Note that all relation names and attribute names are written just in lower
case; the system will lookup the spelling in a table.

Furthermore, it will be possible to add a groupby- and an orderby-clause:

  * groupby

----    select <aggr-list>
        from <rel-list>
        where <pred-list>
        groupby <group-attr-list>
----

Example:

----    select [ort, min(plz) as minplz, max(plz) as maxplz, count(*) as cntplz]
        from plz
        where plz > 40000
        groupby ort
----

  * orderby

----    select <attr-list>
        from <rel-list>
        where <pred-list>
        orderby <order-attr-list>
----

Example:

----    select [ort, plz]
        from plz
        orderby [ort asc, plz desc]
----

This example also shows that the where-clause may be omitted. It is also
possible to combine grouping and ordering:

----    select [ort, min(plz) as minplz, max(plz) as maxplz,  count(*) as cntplz]
        from plz
        where plz > 40000
        groupby ort
        orderby cntplz desc
----

Currently only a basic part of this language has been implemented.


11.2 Structure

We introduce ~select~, ~from~, ~where~, and ~as~ as PROLOG operators:

*/

:- op(990,  fx, sql).
:- op(986, xfx, first).
%:- op(985, xfx, >>).         % XRIS: what's that operator for?
:- op(980, xfx, orderby).
:- op(970, xfx, groupby).
:- op(960, xfx, from).
:- op(950,  fx, select).
:- op(950, xfx, where).
:- op(940,  fx, distinct).
:- op(935,  fx, nonempty). 
:- op(930, xfx, as).
:- op(930, xf , asc).
:- op(930, xf , desc).


/*
This ensures that the select-from-where statement is viewed as a term with the
structure:

----    from(select(AttrList), where(RelList, PredList))
----

That this works, can be tested with:

----    P = (select s:sname from staedte as s where s:bev > 500000),
        P = (X from Y), X = (select AttrList), Y = (RelList where PredList),
        RelList = (Rel as Var).
----

The result is:

----    P = select s:sname from staedte as s where s:bev>500000
        X = select s:sname
        Y = staedte as s where s:bev>500000
        AttrList = s:sname
        RelList = staedte as s
        PredList = s:bev>500000
        Rel = staedte
        Var = s
----

11.3 Schema Lookup

The second task is to lookup attribute names in order to build the input
notation for the construction of the predicate order graph.

11.3.1 Tables

In the file ~database~ we maintain the following tables.

Relation schemas are written as:

----    relation(staedte, [sname, bev, plz, vorwahl, kennzeichen]).
        relation(plz, [plz, ort]).
----

The spelling of relation or attribute names is given in a table

----    spelling(staedte:plz, pLZ).
        spelling(staedte:sname, sName).
        spelling(plz, lc(plz)).
        spelling(plz:plz, pLZ).
----

The default assumption is that the first letter of a name is upper case and all
others are lower case. If this is true, then no entry in the table ~spelling~
is needed. If a name starts with a lower case letter, then this is expressed by
the functor ~lc~.

11.3.2 Looking up Relation and Attribute Names

*/

callLookup(Query, Query2) :-
  newQuery,
  lookup(Query, Query2), !.

newQuery :- 
  not(clearVariables), 
  not(clearQueryRelations), 
  not(clearQueryAttributes), 
  not(clearUsedAttributes), 
  not(clearIsStarQuery).


clearVariables :- retract(variable(_, _)), fail.

clearQueryRelations :- retract(queryRel(_, _)), fail.

clearQueryAttributes :- retract(queryAttr(_)), fail.

clearUsedAttributes :- retract(usedAttr(_, _)), fail.

clearIsStarQuery :- retract(isStarQuery), fail.

/*

----    lookup(Query, Query2) :-
----

~Query2~ is a modified version of ~Query~ where all relation names and
attribute names have the form as required in [Section Translation].

*/

lookup(select Attrs from Rels where Preds,
        select Attrs2 from Rels2List where Preds2List) :-
  lookupRels(Rels, Rels2),
  lookupAttrs(Attrs, Attrs2),
  lookupPreds(Preds, Preds2),
  makeList(Rels2, Rels2List),
  makeList(Preds2, Preds2List).

lookup(select Attrs from Rels,
        select Attrs2 from Rels2) :-
  lookupRels(Rels, Rels2),
  lookupAttrs(Attrs, Attrs2).

lookup(Query orderby Attrs, Query2 orderby Attrs3) :-
  lookup(Query, Query2),
  makeList(Attrs, Attrs2),
  lookupAttrs(Attrs2, Attrs3).

lookup(Query groupby Attrs, Query2 groupby Attrs3) :-
  lookup(Query, Query2),
  makeList(Attrs, Attrs2),
  lookupAttrs(Attrs2, Attrs3).
  
lookup(Query first N, Query2 first N) :-
  lookup(Query, Query2).


makeList(L, L) :- is_list(L).

makeList(L, [L]) :- not(is_list(L)).

/*

11.3.3 Modification of the From-Clause

----    lookupRels(Rels, Rels2)
----

Modify the list of relation names. If there are relations without variables,
store them in a table ~queryRel~. Any two such relations must have distinct
sets of attribute names. Also, any two variables must be distinct.

*/

lookupRels([], []).

lookupRels([R | Rs], [R2 | R2s]) :-
  lookupRel(R, R2),
  lookupRels(Rs, R2s).

lookupRels(Rel, Rel2) :-
  not(is_list(Rel)),
  lookupRel(Rel, Rel2).

/*
----    lookupRel(Rel, Rel2) :-
----

Translate and store a single relation definition.

*/

:- dynamic
  variable/2,
  queryRel/2,
  queryAttr/1,
  isStarQuery/0,
  usedAttr/2.


lookupRel(Rel as Var, rel(Rel2, Var, Case)) :-
  relation(Rel, _), !,
  spelled(Rel, Rel2, Case),
  not(defined(Var)),
  assert(variable(Var, rel(Rel2, Var, Case))).

lookupRel(Rel, rel(Rel2, *, Case)) :-
  relation(Rel, _), !,
  spelled(Rel, Rel2, Case),
  not(duplicateAttrs(Rel)),
  assert(queryRel(Rel, rel(Rel2, *, Case))).

lookupRel(Term, Term) :-
  write('Error in query: relation '), write(Term), write(' not known'),
  nl, fail.

defined(Var) :-
  variable(Var, _),
  write('Error in query: doubly defined variable '), write(Var), write('.'), nl.

/*
----    duplicateAttrs(Rel) :-
----

There is a relation stored in ~queryRel~ that has attribute names also
occurring in ~Rel~.

*/

duplicateAttrs(Rel) :-
  queryRel(Rel2, _),
  relation(Rel2, Attrs2),
  member(Attr, Attrs2),
  relation(Rel, Attrs),
  member(Attr, Attrs),
  write('Error in query: duplicate attribute names in relations '),
  write(Rel2), write(' and '), write(Rel), write('.'), nl.

/*
11.3.4 Modification of the Select-Clause

*/

lookupAttrs(distinct X, distinct Y) :-
  lookupAttrs(X, Y).

lookupAttrs([], []).

lookupAttrs([A | As], [A2 | A2s]) :-
  lookupAttr(A, A2),
  lookupAttrs(As, A2s).

lookupAttrs(Attr, Attr2) :-
  not(is_list(Attr)),
  lookupAttr(Attr, Attr2).

lookupAttr(Var:Attr, attr(Var:Attr2, 0, Case)) :- !,
  variable(Var, Rel2),
  Rel2 = rel(Rel, _, _),
  spelled(Rel:Attr, attr(Attr2, VA, Case)),
  assert(usedAttr(Rel2, attr(Attr2, VA, Case))).

lookupAttr(Attr asc, Attr2 asc) :- !,
  lookupAttr(Attr, Attr2).

lookupAttr(Attr desc, Attr2 desc) :- !,
  lookupAttr(Attr, Attr2).

lookupAttr(Attr, Attr2) :-
  isAttribute(Attr, Rel),!,
  spelled(Rel:Attr, Attr2),
  queryRel(Rel, Rel2),
  assert(usedAttr(Rel2, Attr2)).

lookupAttr(*, *) :- assert(isStarQuery), !.

lookupAttr(count(*), count(*)) :- !.


lookupAttr(Expr as Name, Expr2 as attr(Name, 0, u)) :-
  lookupAttr(Expr, Expr2),
  not(queryAttr(attr(Name, 0, u))),
  !,
  assert(queryAttr(attr(Name, 0, u))).

lookupAttr(Expr as Name, Expr2 as attr(Name, 0, u)) :-
  lookupAttr(Expr, Expr2),
  queryAttr(attr(Name, 0, u)),
  !,
  write('***** Error: attribute name '), write(Name), 
  write(' doubly defined in query.'),
  nl.

/*

Generic lookup functionality for lookupAttr/2 on functors of arbitrary arity 
using Univ (=../2) rather than (functor/3 and arg/3) was introduced by 
C. Duentgen.

When extending operator syntax, only ~lookupPreds~ musst be extended.

*/

lookupAttr(Name, attr(Name, 0, u)) :-
  queryAttr(attr(Name, 0, u)),
  !.

lookupAttr(Term, Term2) :-
  compound(Term),
  Term =.. [Op|Args],
  lookupAttr1(Args, Args2),
  Term2 =.. [Op|Args2],
  !.

lookupAttr(Term, Term) :-
  atom(Term),
  write('Symbol \''),
  write(Term),
  write('\' in attribute list not recognized. Supposed to be a Secondo object\n').

lookupAttr(Term, Term).

lookupAttr1([],[]).

lookupAttr1([Me|Others],[Me2|Others2]) :-
  lookupAttr(Me,Me2),
  lookupAttr1(Others,Others2),
  !.

isAttribute(Name, Rel) :-
  queryRel(Rel, _),
  relation(Rel, List),
  member(Name, List).


/*
11.3.5 Modification of the Where-Clause

*/

lookupPreds([], []).

lookupPreds([P | Ps], [P2 | P2s]) :- !,
  lookupPred(P, P2),
  lookupPreds(Ps, P2s).

lookupPreds(Pred, Pred2) :-
  not(is_list(Pred)),
  lookupPred(Pred, Pred2).


lookupPred(Pred, pr(Pred2, Rel)) :-
  lookupPred1(Pred, Pred2, 0, [], 1, [Rel]), !.

lookupPred(Pred, pr(Pred2, Rel1, Rel2)) :-
  lookupPred1(Pred, Pred2, 0, [], 2, [Rel1, Rel2]), !.

lookupPred(Pred, _) :-
  lookupPred1(Pred, _, 0, [], 0, []),
  write('Error in query: constant predicate is not allowed.'), nl, fail, !.

lookupPred(Pred, _) :-
  lookupPred1(Pred, _, 0, [], N, _),
  N > 2,
  write('Error in query: predicate involving more than two relations '),
  write('is not allowed.'), nl, fail.

/*
----    lookupPred1(+Pred, -Pred2, +N, +RelsBefore, -M, -RelsAfter) :-
----

~Pred2~ is the transformed version of ~Pred~; before this is called, ~N~
attributes in list ~RelsBefore~ have been found; after the transformation in
total ~M~ attributes referring to the relations in list ~RelsAfter~ have been
found.

*/

lookupPred1(Var:Attr, attr(Var:Attr2, N1, Case), N, RelsBefore, N1, RelsAfter)
  :-
  variable(Var, Rel2), !,   Rel2 = rel(Rel, _, _),
  spelled(Rel:Attr, attr(Attr2, X, Case)),
  assert(usedAttr(Rel2, attr(Attr2, X, Case))),
  N1 is N + 1,
  append(RelsBefore, [Rel2], RelsAfter),
  !.

lookupPred1(Attr, attr(Attr2, N1, Case), N, RelsBefore, N1, RelsAfter) :-
  isAttribute(Attr, Rel), !,
  spelled(Rel:Attr, attr(Attr2, X, Case)),
  queryRel(Rel, Rel2),
  assert(usedAttr(Rel2, attr(Attr2, X, Case))),
  N1 is N + 1,
  append(RelsBefore, [Rel2], RelsAfter),
  !.

/*

Generic lookup for operators of arbitrary arity using (=../2) implemented
on Feb/16/2006 by C. Duentgen. See also  ~lookupAttrs~.

*/

lookupPred1(Term, Term2, N, RelsBefore, M, RelsAfter) :-
  compound(Term),
  Term =.. [Op|Args],
  lookupPred2(Args, Args2, N, RelsBefore, M, RelsAfter),
  Term2 =.. [Op|Args2],
  !.  

lookupPred1(Term, Term, N, Rels, N, Rels) :-
  atom(Term),
  not(is_list(Term)),
  write('Symbol \''), write(Term), 
  write('\' not recognized, supposed to be a Secondo object.'), nl, !.

lookupPred1(Term, Term, N, Rels, N, Rels).
 
lookupPred2([], [], N, RelsBefore, N, RelsBefore).

lookupPred2([Me|Others], [Me2|Others2], N, RelsBefore, M, RelsAfter) :-
  lookupPred1(Me,     Me2,     N, RelsBefore,  Q, RelsAfterMe),
  lookupPred2(Others, Others2, Q, RelsAfterMe, M, RelsAfter),
  !.

/*
11.3.6 Check the Spelling of Relation and Attribute Names
 
*/
 
spelled(Rel:Attr, attr(Attr2, 0, l)) :-
  downcase_atom(Rel, DCRel),
  downcase_atom(Attr, DCAttr),
  spelling(DCRel:DCAttr, Attr3),
  Attr3 = lc(Attr2),
  !.
 
spelled(Rel:Attr, attr(Attr2, 0, u)) :-
  downcase_atom(Rel, DCRel),
  downcase_atom(Attr, DCAttr),
  spelling(DCRel:DCAttr, Attr2),
  !.
 
spelled(_:_, attr(_, 0, _)) :- !, fail. % no attr entry in spelling table
 
spelled(Rel, Rel2, l) :-
  downcase_atom(Rel, DCRel),
  spelling(DCRel, Rel3),
  Rel3 = lc(Rel2),
  !.
 
spelled(Rel, Rel2, u) :-
  downcase_atom(Rel, DCRel),
  spelling(DCRel, Rel2), !.
 
spelled(_, _, _) :- !, fail.  % no rel entry in spelling table.



/*
10.3.7 Examples

We can now formulate several of the previous queries at the user level.

*/

example11 :- showTranslate(select [sname, bev] from staedte where bev > 500000).

showTranslate(Query) :-
  callLookup(Query, Query2),
  write(Query), nl,
  write(Query2), nl.

example12 :- showTranslate(
  select * from [staedte as s, plz as p] where [s:sname = p:ort, p:plz > 40000]
  ).

example13 :- showTranslate(
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
    p3:ort starts "M"]
  ).

/*
11.4 Translating a Query to a Plan

----    translate(Query, Stream, SelectClause, Cost) :-
----

~Query~ is translated into a ~Stream~ to which still the translation of the
~SelectClause~ needs to be applied. A ~Cost~ is returned which currently is
only the cost for evaluating the essential part, the conjunctive query.

*/

translate(Query groupby Attrs,
        groupby(sortby(Stream, AttrNamesSort), AttrNamesGroup, Fields), 
        select Select2, Cost) :-
  translate(Query, Stream, SelectClause, Cost1),
  makeList(Attrs, Attrs2),
  attrnames(Attrs2, AttrNamesGroup),
  attrnamesSort(Attrs2, AttrNamesSort),
  SelectClause = (select Select),
  makeList(Select, SelAttrs),
  translateFields(SelAttrs, Attrs2, Fields, Select2),
  getCurrentResultSizes(Arg),
  % determine extra costs for 'groupby(sort(...),...)'
  cost(groupby(sortby(Arg,AttrNamesSort),
               AttrNamesGroup,Fields),*,1,ResCard,ResTupleSize,0,0,Cost2),
  dm(['External cost (6) for groupby(sortby(...,',AttrNamesSort,')',
      AttrNamesGroup,',',Fields,'): ',Cost2,'\n']),
  setCurrentResultSizes(ResCard,ResTupleSize),
  ppCostFactor(PPCF),
  Cost is Cost1 + Cost2 * PPCF,
  !.

translate(Select from Rels where Preds, Stream, Select, Cost) :- 
  pog(Rels, Preds, _, _),
  bestPlan(Stream, Cost),
  !.

/*
Below we handle the case of queries without where-clause. This results
in simple Cartesian products of the relations in the from-clause. 
This case is not very important and we don't want to make the code complicated by
applying projections for removing unnecessary attributes which might be a performance
benefit if a groupby- or orderby- clause is present. The product will be computed
by the symmjoin operator with a constant filter function which returns always true.
This operator works well and has symmetric costs, whereas product has antisymmetric
costs. 

C. Duentgen, Feb/17/2006: changed tuple variable names for the sake of uniqueness
                          (otherwise, a triple-product will crash). 

*/


translate(Select from Rel, Stream, Select, 0) :-
  not(is_list(Rel)),
  makeStream(Rel, Stream), !.

translate(Select from [Rel], Stream, Select, 0) :-
  makeStream(Rel, Stream),
  deleteVariables.

translate(Select from [Rel | Rels], 
        symmjoin(S1, S2, fun([param(T1, tuple), param(T2, tuple2)], true)), 
        Select, 0) :-
  makeStream(Rel, S1),
  translate(Select from Rels, S2, Select, _),
  newVariable(T1),
  newVariable(T2),
  !.

makeStream(Rel, feed(Rel)) :- Rel = rel(_, *, _), !.

makeStream(Rel, rename(feed(Rel), Var)) :- Rel = rel(_, Var, _).



/*
The next predicate finds all attributes of a given relation ~Rel~
which are needed in this query. The result can be used to create
project(feed(..)) streams instead of simply feeding all attributes
from a relation into a stream. The system predicate ~setof~ is used
to find all goal for query ~usedAttr(Rel,X)~.

*/

usedAttrList(Rel, ResList) :- 
  setof(X, usedAttr(Rel, X), R1),
  %nl, write('AttrList: '), write(R1), nl,
  attrnames(R1, ResList). 



/*
----    translateFields(Select, GroupAttrs, Fields, Select2) :-
----

Translate the ~Select~ clause of a query containing ~groupby~. Grouping
was done by the attributes ~GroupAttrs~. Return a list ~Fields~ of terms
of the form ~field(Name, Expr)~; such a list can be used as an argument to the
groupby operator. Also, return a modified select clause ~Select2~,
which will translate to a corresponding projection operation.

*/

translateFields([], _, [], []).

translateFields([count(*) as NewAttr | Select], GroupAttrs, 
        [field(NewAttr , count(feed(group))) | Fields], [NewAttr | Select2]) :-
  translateFields(Select, GroupAttrs, Fields, Select2),
  !.

translateFields([sum(attr(Name, Var, Case)) as NewAttr | Select], GroupAttrs,
        [field(NewAttr,sum(feed(group),attrname(attr(Name,Var,Case))))|Fields],
        [NewAttr| Select2]) :-
  translateFields(Select, GroupAttrs, Fields, Select2),
  !.
  
translateFields([sum(Expr) as NewAttr | Select], GroupAttrs,
        [field(NewAttr, 
          sum(
            extend(feed(group), field(attr(xxxExprField, 0, l), Expr)),
            attrname(attr(xxxExprField, 0, l))
            ))
        | Fields],
        [NewAttr| Select2]) :-
  translateFields(Select, GroupAttrs, Fields, Select2),
  !.

translateFields([Attr | Select], GroupAttrs, Fields, [Attr | Select2]) :-
  member(Attr, GroupAttrs),
  !,
  translateFields(Select, GroupAttrs, Fields, Select2).


translateFields([Attr as Name|Select],GroupAttrs,Fields,[Attr as Name|Select2]) 
  :-
  member(Attr, GroupAttrs),
  !,
  translateFields(Select, GroupAttrs, Fields, Select2).


/*
Generic rule for aggregate functions, similar to count, sum, max, min, avg.

*/

translateFields([Term as NewAttr | Select], GroupAttrs,
        [field(NewAttr, Term2) | Fields],
        [NewAttr| Select2]) :-
  compound(Term),
  functor(Term, AggrOp, 1),
  arg(1, Term, attr(Name, Var, Case)),
  isAggregationOP(AggrOp),
  functor(Term2, AggrOp, 2),
  arg(1, Term2, feed(group)),
  arg(2, Term2, attrname(attr(Name, Var, Case))),
  translateFields(Select, GroupAttrs, Fields, Select2),
  !.

translateFields([Term as NewAttr | Select], GroupAttrs,
        [field(NewAttr, Term2) | Fields],
        [NewAttr| Select2]) :-
  compound(Term),
  functor(Term, AggrOp, 1),
  arg(1, Term, Expr),
  isAggregationOP(AggrOp),
  functor(Term2, AggrOp, 2),
  arg(1, Term2, extend(feed(group), field(attr(xxxExprField, 0, l), Expr))),
  arg(2, Term2, attrname(attr(xxxExprField, 0, l))),
  translateFields(Select, GroupAttrs, Fields, Select2),
  !.


translateFields([Term | Select], GroupAttrs,
        Fields, 
        Select2) :-
  compound(Term),
  functor(Term, AggrOp, 1),
  arg(1, Term, Attr),
  isAggregationOP(AggrOp),
  functor(Term2, AggrOp, 2),
  arg(1, Term2, feed(group)),
  arg(2, Term2, attrname(Attr)),
  translateFields(Select, GroupAttrs, Fields, Select2),
  write('*****'), nl,
  write('***** Error in groupby: missing name for new attribute'), nl,
  write('*****'), nl,
  !.


translateFields([Attr | Select], GroupAttrs, Fields, Select2) :-
  not(member(Attr, GroupAttrs)),
  !,
  translateFields(Select, GroupAttrs, Fields, Select2),
  write('*****'), nl,
  write('***** Error in groupby: '), 
  write(Attr), 
  write(' is neither a grouping attribute'), nl,
  write('      nor an aggregate expression.'), nl,
  write('*****'), nl.


/*

----    queryToPlan(Query, Plan, Cost) :-
----

Translate the ~Query~ into a ~Plan~. The ~Cost~ for evaluating the conjunctive
query is also returned. The ~Query~ must be such that relation and attribute
names have been looked up already.

*/

queryToPlan(Query, count(Stream), Cost) :-
  countQuery(Query),
  queryToStream(Query, Stream, Cost1),
  getCurrentResultSizes(Arg),
  cost(count(Arg), *, 1, _, _, 0, 0, Cost2), % extra costs for count(...)
  dm(['External cost for count(...): ', Cost2, '\n']),
  ppCostFactor(PPCF),
  Cost is Cost1 + Cost2 * PPCF,
  !.

queryToPlan(Query, consume(Stream), Cost) :-
  queryToStream(Query, Stream, Cost1),
  getCurrentResultSizes(Arg),
  cost(consume(Arg), *, 1, _, _, 0, 0, Cost2), % extra costs for consume(...)
  dm(['External cost for consume(...): ', Cost2, '\n']),
  ppCostFactor(PPCF),
  Cost is Cost1 + Cost2 * PPCF,
  !.


/*
Check whether ~Query~ is a counting query.

*/


countQuery(select count(*) from _) :- !.

countQuery(select distinct count(*) from _) :- !.

countQuery(Query groupby _) :-
  countQuery(Query).

countQuery(Query orderby _) :-
  countQuery(Query).

countQuery(Query first _) :-
  countQuery(Query).

/*

----    queryToStream(Query, Plan, Cost) :-
----

Same as ~queryToPlan~, but returns a stream plan, if possible.

*/

queryToStream(Query first N, head(Stream, N), Cost) :-
  queryToStream(Query, Stream, Cost1),
  getCurrentResultSizes(Arg),
  term_to_atom(Arg, ATerm),
  ATerm = fixed(_,TupleSize),
  % determine extra costs for head(...)
  cost(head(Arg, N), *, 1, ResCard, _, 0, 0, Cost2),
  dm(['External cost (1) for head(...,', N, '): ', Cost2, '\n']),
  setCurrentResultSizes(ResCard,TupleSize), % XRIS: reduce result cardinality at least for final ``consume''
  ppCostFactor(PPCF),
  Cost is Cost1 + Cost2 * PPCF,  % XRIS: optimizer could be extended to handle early aborts by ``head'' in non-blocking sub-OP-trees
  !.

queryToStream(Query orderby SortAttrs, Stream2, Cost) :- % OK
  translate(Query, Stream, Select, Cost1),
  finish(Stream, Select, SortAttrs, Stream2),
  getCurrentResultSizes(Arg),
  term_to_atom(Arg, ArgA),
  term_to_atom(Stream, StreamA),
  term_to_atom(Stream2, Stream2A),
  sub_atom(Stream2A, PrefixLen, _, SuffixLen, StreamA),     % get positions of Stream in Stream2
  sub_atom(Stream2A, 0, PrefixLen, _, PrefixA),             % get Prefix of Stream in Stream2
  sub_atom(Stream2A, _, SuffixLen, 0, SuffixA),             % get Suffix of Stream in Stream2
  concat_atom([PrefixA, ArgA, SuffixA], CostTermA),         
  term_to_atom(CostTerm, CostTermA),
  cost(CostTerm, *, 1, ResCard, ResTupleSize, 0, 0, Cost2), % determine extra costs for sort and project
  dm(['External cost (2) for ', PrefixA, '...', SuffixA, ': ', Cost2, '\n']),
  setCurrentResultSizes(ResCard,ResTupleSize),
  ppCostFactor(PPCF),
  Cost is Cost1 + Cost2 * PPCF,
  !.

queryToStream(Select from Rels where Preds, Stream2, Cost) :- % OK
  translate(Select from Rels where Preds, Stream, Select1, Cost1),
  finish(Stream, Select1, [], Stream2),
  getCurrentResultSizes(Arg),
  term_to_atom(Arg, ArgA),
  term_to_atom(Stream, StreamA),
  term_to_atom(Stream2, Stream2A),
  sub_atom(Stream2A, PrefixLen, _, SuffixLen, StreamA),     % get positions of Stream in Stream2
  sub_atom(Stream2A, 0, PrefixLen, _, PrefixA),             % get Prefix of Stream in Stream2
  sub_atom(Stream2A, _, SuffixLen, 0, SuffixA),             % get Suffix of Stream in Stream2
  concat_atom([PrefixA, ArgA, SuffixA], CostTermA),         
  term_to_atom(CostTerm, CostTermA),
  cost(CostTerm, *, 1, ResCard, ResTupleSize, 0, 0, Cost2), % determine extra costs for project etc
  dm(['External cost (3) for ', PrefixA, '...', SuffixA, ': ', Cost2, '\n']),
  setCurrentResultSizes(ResCard,ResTupleSize),
  ppCostFactor(PPCF),
  Cost is Cost1 + Cost2 * PPCF,
  !.

queryToStream(Select from Rels groupby Attrs, Stream2, Cost) :- % OK
  translate(Select from Rels groupby Attrs, Stream, Select1, Cost1), % costs for groupby(sort(...),...) already considered
  finish(Stream, Select1, [], Stream2),
  getCurrentResultSizes(Arg),
  term_to_atom(Arg, ArgA),
  term_to_atom(Stream, StreamA),
  term_to_atom(Stream2, Stream2A),
  sub_atom(Stream2A, PrefixLen, _, SuffixLen, StreamA),     % get positions of Stream in Stream2
  sub_atom(Stream2A, 0, PrefixLen, _, PrefixA),             % get Prefix of Stream in Stream2
  sub_atom(Stream2A, _, SuffixLen, 0, SuffixA),             % get Suffix of Stream in Stream2
  concat_atom([PrefixA, ArgA, SuffixA], CostTermA),
  term_to_atom(CostTerm, CostTermA),
  cost(CostTerm, *, 1, ResCard, ResTupleSize, 0, 0, Cost2), % determine extra costs for project etc
  dm(['External cost (4) for ', PrefixA, '...', SuffixA, ': ', Cost2, '\n']),
  setCurrentResultSizes(ResCard,ResTupleSize),
  ppCostFactor(PPCF),
  Cost is Cost1 + Cost2 * PPCF,
  !.

queryToStream(Query, Stream2, Cost) :-
  translate(Query, Stream, Select, Cost1),
  finish(Stream, Select, [], Stream2),
  getCurrentResultSizes(Arg),
  term_to_atom(Arg, ArgA),
  term_to_atom(Stream, StreamA),
  term_to_atom(Stream2, Stream2A),
  sub_atom(Stream2A, PrefixLen, _, SuffixLen, StreamA),   % get positions of Stream in Stream2
  sub_atom(Stream2A, 0, PrefixLen, _, PrefixA),             % get Prefix of Stream in Stream2
  sub_atom(Stream2A, _, SuffixLen, 0, SuffixA),             % get Suffix of Stream in Stream2
  concat_atom([PrefixA, ArgA, SuffixA], CostTermA),
  term_to_atom(CostTerm, CostTermA),
  cost(CostTerm, *, 1, ResCard, ResTupleSize, 0, 0, Cost2), % determine extra costs for project etc
  dm(['External cost (5) for ', PrefixA, '...', SuffixA, ': ', Cost2, '\nl']),
  setCurrentResultSizes(ResCard,ResTupleSize),
  ppCostFactor(PPCF),
  Cost is Cost1 + Cost2 * PPCF,
  !.

/*
----    finish(Stream, Select, Sort, Stream2) :-
----

Given a ~Stream~, a ~Select~ clause, and a set of attributes for sorting,
apply the final tranformations (extend, project, duplicate removal, sorting) to obtain ~Stream2~.

*/

finish(Stream, Select, Sort, Stream2) :-
  selectClause(Select, Extend, Project, Rdup),
  finish2(Stream, Extend, Project, Rdup, Sort, Stream2).

/*
----	selectClause(Select, Extend, Project, Rdup) :-
----

The ~Select~ clause contains attribute lists ~Extend~ for extension, ~Project~ for projection, and possibly a ~distinct~ keyword indicating duplicate removal returned in ~Rdup~.

*/


selectClause(select *, [], *, duplicates).

selectClause(select count(*), [], count(*), duplicates).

selectClause(select Attrs, Extend, Project, duplicates) :-
  makeList(Attrs, Attrs2),
  extendProject(Attrs2, Extend, Project).

selectClause(select distinct *, [], *, distinct).

selectClause(select distinct count(*), [], count(*), distinct).

selectClause(select distinct Attrs, Extend, Project, distinct) :-
  makeList(Attrs, Attrs2),
  extendProject(Attrs2, Extend, Project).

/*
----	finish2(Stream, Extend, Project, Rdup, Sort, Stream5) :-
----

Apply extension, projection, duplicate removal and sorting as specified to ~Stream~ to obtain ~Stream5~.

*/


finish2(Stream, Extend, Project, Rdup, Sort, Stream5) :-
  fExtend(Stream, Extend, Stream2),
  fProject(Stream2, Project, Stream3),
  fRdup(Stream3, Rdup, Stream4),
  fSort(Stream4, Sort, Stream5).



fExtend(Stream, [], Stream) :- !.

fExtend(Stream, Extend, extend(Stream, Extend)).



fProject(Stream, *, Stream) :- !.

fProject(Stream, count(*), Stream) :- !.

fProject(Stream, Project, project(Stream, AttrNames)) :-
  attrnames(Project, AttrNames).




fRdup(Stream, duplicates, Stream) :- !.

fRdup(Stream, distinct, rdup(sort(Stream))).



fSort(Stream, [], Stream) :- !.

fSort(Stream, SortAttrs, sortby(Stream, AttrNames)) :-
  attrnamesSort(SortAttrs, AttrNames).



/*
----	extendProject(Attrs, ExtendAttrs, ProjectAttrs) :-
----

Construct the extension and projection attribute lists from ~Attrs~.

*/


extendProject([], [], []).

extendProject([Expr as Name | Attrs], [field(Name, Expr) | Extend],
        [Name | Project]) :-
  !,
  extendProject(Attrs, Extend, Project).

extendProject([attr(Name, Var, Case) | Attrs], Extend,
        [attr(Name, Var, Case) | Project]) :-
  extendProject(Attrs, Extend, Project).


/*

----    attrnames(Attrs, AttrNames) :-
----

Transform each attribute X into attrname(X).

*/

attrnames([], []).

attrnames([Attr | Attrs], [attrname(Attr) | AttrNames]) :-
  attrnames(Attrs, AttrNames).

/*

----    attrnamesSort(Attrs, AttrNames) :-
----

Transform attribute names of orderby clause.

*/

attrnamesSort([], []).

attrnamesSort([Attr | Attrs], [Attr2 | Attrs2]) :-
  attrnameSort(Attr, Attr2),
  attrnamesSort(Attrs, Attrs2).

attrnameSort(Attr asc, attrname(Attr) asc) :- !.

attrnameSort(Attr desc, attrname(Attr) desc) :- !.

attrnameSort(Attr, attrname(Attr) asc).


/*


11.3.8 Integration with Optimizer

----    optimize(Query).
----

Optimize ~Query~ and print the best ~Plan~.

*/

optimize(Query) :-
  rewriteQuery(Query, RQuery),
  callLookup(RQuery, Query2),
  queryToPlan(Query2, Plan, Cost),
  plan_to_atom(Plan, SecondoQuery),
  write('The plan is: '), nl, nl,
  write(SecondoQuery), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  invalidateRelationCache.


optimize(Query, QueryOut, CostOut) :-
  rewriteQuery(Query, RQuery),
  callLookup(RQuery, Query2),
  queryToPlan(Query2, Plan, CostOut),
  plan_to_atom(Plan, QueryOut),
  invalidateRelationCache.

/*
----    sqlToPlan(QueryText, Plan)
----

Transform an SQL ~QueryText~ into a ~Plan~. The query is given as a text atom.

*/
sqlToPlan(QueryText, Plan) :-
  term_to_atom(sql Query, QueryText),
  optimize(Query, Plan, _).


/*
----    sqlToPlan(QueryText, Plan)
----

Transform an SQL ~QueryText~ into a ~Plan~. The query is given as a text atom.
~QueryText~ starts not with sql in this version.

*/
sqlToPlan(QueryText, Plan) :-
  term_to_atom(Query, QueryText),
  optimize(Query, Plan, _).




/*
11.3.8 Examples

We can now formulate the previous example queries in the user level language.
They are stored as prolog facts sqlExample/2. Examples can be called by
testing the predicates example/1 or example/2. Moreover, they are also
present as facts with name ~example<No>~.

Below we present some generic rules for evaluating SQL examples.

*/

showExample(Nr, Query) :- 
  sqlExample(Nr, Query), 
  nl, write('SQL: '), write(Query), nl, nl.

example(Nr) :- showExample(Nr, Query), optimize(Query).
example(Nr, Query, Cost) :- showExample(Nr, Example), optimize(Example, Query, Cost).


/*
Examples 14 - 22:

*/


sqlExample( 14,

  select * from [staedte as s, plz as p] where [p:ort = s:sname, p:plz > 40000, (p:plz mod 5) = 0]
  ).

sqlExample( 15,

  select * from staedte where bev > 500000
  ).


sqlExample( 16,

  select * from [staedte as s, plz as p] where [s:sname = p:ort, p:plz > 40000]
  ).

/*
Example 17. This may need a larger local stack size. Start Prolog as

----    pl -L4M
----

which initializes the local stack to 4 MB.

*/

sqlExample( 17,
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
    p3:ort starts "M"]
  ).



sqlExample( 18,
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
    p1:ort starts "M"]
  ).


sqlExample( 19, 
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
    p1:ort starts "M"]
  ).


sqlExample( 20,
  select *
  from [staedte as s, plz as p]
  where [
    p:ort = s:sname,
    p:plz > 40000,
    s:bev > 300000]
  ).


sqlExample( 21, 
  select *
  from [staedte, plz as p1, plz as p2, plz as p3]
  where [
    sname = p1:ort,
    p1:plz = p2:plz + 1,
    p2:plz = p3:plz * 5]
  ).

example14 :- example(14).
example15 :- example(15).
example16 :- example(16).
example17 :- example(17).
example18 :- example(18).
example19 :- example(19).
example20 :- example(20).
example21 :- example(21).



/*

12 Optimizing and Calling Secondo

----    sql Term
        sql(Term, SecondoQueryRest)
        let(X, Term)
        let(X, Term, SecondoQueryRest)
----    

~Term~ must be one of the available select-from-where statements.
It is optimized and Secondo is called to execute it. ~SecondoQueryRest~
is a character string (atom) containing a sequence of Secondo 
operators that can be appended to a given
plan found by the optimizer; in this case the optimizer returns a
plan producing a stream.

The two versions of ~let~ allow one to assign the result of a query
to a new object ~X~, using the optimizer.

*/
sql Term :-
  isDatabaseOpen,
  mOptimize(Term, Query, Cost),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  query(Query).

sql(Term, SecondoQueryRest) :-
  isDatabaseOpen,
  mStreamOptimize(Term, SecondoQuery, Cost),
  concat_atom([SecondoQuery, ' ', SecondoQueryRest], '', Query),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  query(Query).

let(X, Term) :-
  isDatabaseOpen,
  mOptimize(Term, Query, Cost),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  concat_atom(['let ', X, ' = ', Query], '', Command),
  secondo(Command).

let(X, Term, SecondoQueryRest) :-
  isDatabaseOpen,
  mStreamOptimize(Term, SecondoQuery, Cost),
  concat_atom([SecondoQuery, ' ', SecondoQueryRest], '', Query),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  concat_atom(['let ', X, ' = ', Query], '', Command),
  secondo(Command).


/*
----    streamOptimize(Term, Query, Cost) :-
----

Optimize the ~Term~ producing an incomplete Secondo query plan ~Query~ 
returning a stream.

*/
streamOptimize(Term, Query, Cost) :-
  callLookup(Term, Term2),
  queryToStream(Term2, Plan, Cost),
  setCurrentResultSizes,
  plan_to_atom(Plan, Query),
  invalidateRelationCache.

/*
----    mOptimize(Term, Query, Cost) :-
        mStreamOptimize(union [Term], Query, Cost) :-
----

Means ``multi-optimize''. Optimize a ~Term~ possibly consisting of several 
subexpressions to be independently optimized, as in union and intersection 
queries. ~mStreamOptimize~ is a variant returning a stream.

*/

:-op(800, fx, union).
:-op(800, fx, intersection).

mOptimize(union Terms, Query, Cost) :-
  mStreamOptimize(union Terms, Plan, ArgCost),
  getCurrentResultSizes(Arg),  
  concat_atom([Plan, 'consume'], '', Query), 
  cost(consume(Arg,*), *, 1, CCard, CTupleSize, 0, 0, CCost),
  setCurrentResultSizes(CCard, CTupleSize),
  Cost is ArgCost + CCost.

mOptimize(intersection Terms, Query, Cost) :-
  mStreamOptimize(intersection Terms, Plan, ArgCost),
  getCurrentResultSizes(Arg),  
  concat_atom([Plan, 'consume'], '', Query),
  cost(consume(Arg,*), *, 1, CCard, CTupleSize, 0, 0, CCost),
  setCurrentResultSizes(CCard, CTupleSize),
  Cost is ArgCost + CCost.

mOptimize(Term, Query, Cost) :-
  optimize(Term, Query, Cost).


mStreamOptimize(union [Term], Query, Cost) :-
  streamOptimize(Term, QueryPart, ArgCost),
  getCurrentResultSizes(Arg),
  concat_atom([QueryPart, 'sort rdup '], '', Query),
  cost(rdup(sort(Arg)), *, 1, SRCard, SRTupleSize, 0, 0, SRCost),
  setCurrentResultSizes(SRCard, SRTupleSize),
  Cost is ArgCost + SRCost.

mStreamOptimize(union [Term | Terms], Query, Cost) :-
  streamOptimize(Term, Plan1, Cost1),
  getCurrentResultSizes(Arg1),
  mStreamOptimize(union Terms, Plan2, Cost2),
  getCurrentResultSizes(Arg2),
  concat_atom([Plan1, 'sort rdup ', Plan2, 'mergeunion '], '', Query), 
  cost(mergeunion(Arg1, rdup(sort(Arg2))), *, 1, ResCard, ResTupleSize, 0, 0, Cost3),
  setCurrentResultSizes(ResCard, ResTupleSize),
  Cost is Cost1 + Cost2 + Cost3.

mStreamOptimize(intersection [Term], Query, Cost) :-
  streamOptimize(Term, QueryPart, ArgCost),
  getCurrentResultSizes(Arg),
  concat_atom([QueryPart, 'sort rdup '], '', Query),
  cost(rdup(sort(Arg)), *, 1, SRCard, SRTupleSize, 0, 0, SRCost),
  setCurrentResultSizes(SRCard, SRTupleSize),
  Cost is ArgCost + SRCost.

mStreamOptimize(intersection [Term | Terms], Query, Cost) :-
  streamOptimize(Term, Plan1, Cost1),              
  getCurrentResultSizes(Arg1),
  mStreamOptimize(intersection Terms, Plan2, Cost2), 
  getCurrentResultSizes(Arg2),
  concat_atom([Plan1, 'sort rdup ', Plan2, 'mergesec '], '', Query),
  cost(mergesec(Arg1, rdup(sort(Arg2))), *, 1, ResCard, ResTupleSize, 0, 0, Cost3),
  setCurrentResultSizes(ResCard, ResTupleSize),
  Cost is Cost1 + Cost2 + Cost3.

mStreamOptimize(Term, Query, Cost) :-
  streamOptimize(Term, Query, Cost).



/*
Some auxiliary stuff.

*/

bestPlanCount :-
  bestPlan(P, _),
  plan_to_atom(P, S),
  atom_concat(S, ' count', Q), 
  nl, write(Q), nl,
  query(Q).

bestPlanConsume :-
  bestPlan(P, _),
  plan_to_atom(P, S),
  atom_concat(S, ' consume', Q),
  nl, write(Q), nl,
  query(Q).

/*
Print debugging information

Predicate ~dm/1~ can be used as ~write~/1. Output is printed when option
optDebug is defined (see file operators.pl).

*/

dm([]) :- !.

dm([X1 | XR]) :-
  optDebug,
  write(X1),
  dm(XR), 
  !.

dm(X) :-
  optDebug,
  write(X),
  !.

dm(_) :- !.

  
/*

13 Auxiliary Predicates for Tuple Size Calculation

Tuple size is an important factor within cost functions, as the number of
tuples that fit into memory strongly depend on this value. During the stream
processing, the sizes of the tuples vary. The optimizer tries to reduce 
tuplesizes within the streams by initial and terminal projections on the 
set of used/required attributes.

*/

/*
  ~usedAttrListAtom(Rel, AttrList)~ unifies ~AttrList~ with the list of all
  downcased names of attributs from relation ~Rel~, that are used in the
  query.

*/
usedAttrListAtom(Rel, Result) :-
  usedAttrList(Rel, LT), 
  plan_to_atom(LT, LA),
  downcase_atom(LA, LD),
  concat_atom(['[',LD,']'], LDL),
  term_to_atom(Result, LDL).


/*
  ~atomicAttrName(AttrTermList, AttrNameList)~ unifies ~AttrNameList~
  with the list of atomic attribute names from ~AttrTermList~.

*/


% test: calculateProjectedTupleSize(rel(plz, p, l), [attr(plz, 1, u), 
% attr(ort, 1, u)],X,Y).

atomicAttrName([attr(_:Name, _, _)], [Result]) :-
  downcase_atom(Name, NameD),
  term_to_atom(Result, NameD),
  !.

atomicAttrName([attr(Name, _, _)], [Result]) :-
  downcase_atom(Name, NameD),
  term_to_atom(Result, NameD),
  !.

atomicAttrName([attr(_:Name, _, _) | Rest], [Result | MoreResults]) :-
  downcase_atom(Name, NameD),
  term_to_atom(Result, NameD),
  atomicAttrName(Rest, MoreResults),
  !.

atomicAttrName([attr(Name, _, _) | Rest], [Result | MoreResults]) :-
  downcase_atom(Name, NameD),
  term_to_atom(Result, NameD),
  atomicAttrName(Rest, MoreResults),
  !.

/* 
~calculateProjectedTupleSizeA(RelName, AttrNameList, CoreTupleSize, AvgInFlobSize,
AvgExtFlobSize)~ calculates ~CoreTupleSize~, ~AvgInFlobSize~ and ~AvgExtFlobSize~
for the projection of relation ~RelName~ on a subset ~AttrNameList~ of its 
attributes. The attribute names and relation name must be atomic terms.

~calculateProjectedProjectedTupleSize(Rel, AttrList, CoreTupleSize, InFlobSize, 
ExtFlobSize)~ does the same for relation ~Rel~ and the attribute list  ~AttrList~ 
given as complex terms.

*/
calculateProjectedTupleSizeA(RelName, [AttrName], CoreTupleSize, InFlobSize, ExtFlobSize) :-
  databaseName(DB),
  storedAttrSize(DB, RelName, AttrName, _, CoreTupleSize, InFlobSize, ExtFlobSize).

calculateProjectedTupleSizeA(RelName, [AttrName | Rest], CoreTupleSize, InFlobSize, ExtFlobSize) :-
  databaseName(DB),
  storedAttrSize(DB, RelName, AttrName, _, Core1, Inl1, Ext1),
  calculateProjectedTupleSizeA(RelName, Rest, Core2, Inl2, Ext2),
  CoreTupleSize is Core1+Core2,
  InFlobSize is Inl1+Inl2,
  ExtFlobSize is Ext1+Ext2.
  
calculateProjectedTupleSize(Rel, AttrList, CoreTupleSize, InFlobSize, ExtFlobSize) :-
  Rel = rel(R, _, _),
  downcase_atom(R, RelName),
  atomicAttrName(AttrList, AttrNameList),
  calculateProjectedTupleSizeA(RelName, AttrNameList, CoreTupleSize, InFlobSize, ExtFlobSize).

/*
~calculateQueryTupleSize(Rel, CoreTupleSize, InFlobSize)~ unifies 
~CoreTupleSize~ and ~InFlobSize~  with the tuple sizes relation ~Rel~ will have 
in this query after it has been projected according to the SQL select clause.

  The result is used to annotate the actual tuple sizes to each node of the pog.

*/
calculateQueryTupleSize(Rel, CoreTupleSize, InFlobSize, ExtFlobSize) :-
  usedAttrListAtom(Rel, UsedAttr),
  Rel = rel(R, _, _),
  downcase_atom(R, RelName),
  calculateProjectedTupleSizeA(RelName, UsedAttr, CoreTupleSize, InFlobSize, ExtFlobSize).  
  
/*

14 Query Rewriting

Rewriting is used as a preprocessing step during the optimization of a query. 
In the rewriting phase, a query posed by the user will be analysed and rewritten
on the level of the query language rather than on the executable language level.
The optimization on the executable langage level is done in the subsequent steps 
of the optimization process, and work on the rewritten query.

During the rewriting process, regarding the user`s original conditions, 
inferred conditions are added, some may be removed and others be modified.


---- rewriteQuery(OriginalQuery, -Rewrittenquery)
----

rewrites the ~OriginalQuery~ performing several steps of rewriting and returns
the finally ~RewrittenQuery~.

*/

rewriteQuery(Query, RewrittenQuery) :-
  rewriteQueryForInferenceOfPredicates(Query, RQuery1),
  rewriteQueryForRedundancy(RQuery1, RQuery2),
  rewriteQueryForCSE(RQuery2, RewrittenQuery),
  dm(['\nOriginal Query: ', Query, 
      '\n\nAfter Rewriting Stage 1: Infer Predicates:\n', RQuery1,
      '\n\nAfter Rewriting Stage 2: Remove redundancies:\n', RQuery2, 
      '\n\nAfter Rewriting Stage 3: Handle CSEs:\n', RewrittenQuery, '\n']).

/*

14.1 Inference of Predicates

*/

rewriteQueryForInferenceOfPredicates(Query, RewrittenQuery) :-
  rewriteQueryForNonempty(Query, RQuery1),
  rewriteQueryForInferredSubstitutions(RQuery1, RewrittenQuery),
  dm(['\nAfter Step 1: Infer nonempty-predicates:\n', RQuery1,
      '\nAfter Step 2: Infer substitutions:\n', RewrittenQuery, '\n']).


/*

14.1.1 Nonempty-Queries

Spatial data may contain `undefined' data and empty sets. Often, a user wants to
suppress empty results. To this end, the kewword `nonempty' within the select 
clause is introduced. A nonempty-query will automatically infer conditions
from the statements within the select clause, that guarantee, that only 
nonempty data will be reported, and rewrite the query: The additional 
predicates are added to the where clause of the query.

Rules to infer predicates. The code should be moved to file operators.pl

*/

rewritingNonemptyRule(val(X atinstant Y), [X present Y]).
rewritingNonemptyRule(X atperiods Y,      [X present Y]).
rewritingNonemptyRule(X at Y,             [X passes Y]).
rewritingNonemptyRule(X when P,           [X satisfies P]).
rewritingNonemptyRule(intersection(X, Y), [X intersects Y]).
rewritingNonemptyRule(minus(X, Y),        [Y inside X]).
rewritingNonemptyRule(X,                  [not(isempty(X))]) :- 
  X =.. [OP|_],
  isBBoxOperator(OP).


/*
---- inferNonemptyPredicates(-InferredPreds)
----
uses rules defined by ~inferNonemptyPredicate/2~ to detect expressions
within the term indexed by calling ~markNode/3~  that allow to infer 
nonempty-predicates. The latter are collected within list ~InferredPred~
and returned.

~inferSingleNonemptyPredicate(-Pred)~ searches the dynamic predicate
~storedExpressionLabel/3~ and the table ~inferNonemptyPredicate/2~ for 
any matches and returns the predictate inferred ~Pred~.

*/

inferNonemptyPredicates(NewPreds) :-
  findall(Pred, inferSingleNonemptyPredicate(Pred), NewPredsListList),
  flatten(NewPredsListList,NewPreds).

inferSingleNonemptyPredicate(Pred) :-
  storedExpressionLabel(Trigger, _, _),  
  rewritingNonemptyRule(Trigger, Pred).
   
/*
---- rewriteQueryForNonempty(+Query,-RewrittenQuery)
----

rewrites ~Query~ to contain conditions that force Secondo
to suppress empty data within the result of the query.

*/


% Normal query (without nonempty)
rewriteQueryForNonempty(Query, Query) :-
  Query \= from(select(nonempty(_)),_),
  Query \= from(select(distinct(nonempty(_))),_),
  retractExpressionLabels,
  write('1\n'),
  !.

% Ordinary nonempty-queries with distinct
rewriteQueryForNonempty(Query, RewrittenQuery) :-
  Query = from(select(distinct(nonempty(SelectClause))),where(Rels,WhereClause)),
  retractExpressionLabels,
  markNode(select(SelectClause),_,0),          % decompose select clause into terms
  inferNonemptyPredicates(NonEmptyConditions), % analyse SelectClause collection for inferred predicates
  ( is_list(WhereClause) -> 
      append_list(WhereClause, NonEmptyConditions, RewrittenWhereClause)
    ; RewrittenWhereClause = [WhereClause|NonEmptyConditions]
  ),
  sort(RewrittenWhereClause,RewrittenWhereClause1),
  % remove `nonempty' from query & append inferred predicates to where-clauses
  RewrittenQuery = from(select(distinct(SelectClause)),where(Rels,RewrittenWhereClause1)),
  write('2\n'),
  !.
  
% Ordinary nonempty-queries w/o distinct
rewriteQueryForNonempty(Query, RewrittenQuery) :-
  Query = from(select(nonempty(SelectClause)),where(Rels,WhereClause)),
  retractExpressionLabels,
  markNode(select(SelectClause),_,0),           % decompose select clause into terms
  inferNonemptyPredicates(NonEmptyConditions),  % analyse SelectClause collection for inferred predicates
  ( is_list(WhereClause) -> 
      append_list(WhereClause, NonEmptyConditions, RewrittenWhereClause)
    ; RewrittenWhereClause = [WhereClause|NonEmptyConditions]
  ),
  sort(RewrittenWhereClause,RewrittenWhereClause1),
  % remove `nonempty' from query & append inferred predicates to where-clauses
  RewrittenQuery = from(select(SelectClause),where(Rels,RewrittenWhereClause1)),
  write('3\n'),
  !.

% Special cases: nonempty with empty where clause and distinct
rewriteQueryForNonempty(Query, RewrittenQuery) :-
  Query \= from(select(distinct(nonempty(_))),where(_,_)),
  Query  = from(select(distinct(nonempty(SelectClause))),Rels),
  retractExpressionLabels,
  markNode(select(SelectClause),_,0),           % decompose select clause into terms
  inferNonemptyPredicates(NonEmptyConditions),  % analyse SelectClause collection for inferred predicates
  sort(NonEmptyConditions,NonEmptyConditions1),
  ( NonEmptyConditions1 = [] -> 
      RewrittenQuery = from(select(distinct(SelectClause)),Rels)
    ; RewrittenQuery = from(select(distinct(SelectClause)),where(Rels,NonEmptyConditions1))
  ),
  write('4\n'),
  !.
  
% Special cases: nonempty with empty where clause but w/o distinct
rewriteQueryForNonempty(Query, RewrittenQuery) :-
  Query \= from(select(nonempty(_)),where(_,_)),
  Query  = from(select(nonempty(SelectClause)),Rels),
  retractExpressionLabels,
  markNode(select(SelectClause),_,0),           % decompose select clause into terms
  inferNonemptyPredicates(NonEmptyConditions),  % analyse SelectClause collection for inferred predicates
  sort(NonEmptyConditions,NonEmptyConditions1),
  ( NonEmptyConditions1 = [] -> 
      RewrittenQuery = from(select(SelectClause),Rels)
    ; RewrittenQuery = from(select(SelectClause),where(Rels,NonEmptyConditions1))
  ),
  write('5\n'),
  !.

% Special cases: ordering and grouping clauses can be ignored for nonempty-rewriting.
rewriteQueryForNonempty(Query, RewrittenQuery) :-
  Query = first(Query2, X),
  rewriteQueryForNonempty(Query2, RewrittenQuery2),
  RewrittenQuery = first(RewrittenQuery2,X),
  write('6\n'),
  !.

rewriteQueryForNonempty(Query, RewrittenQuery) :-
  Query = orderby(Query2, X),
  rewriteQueryForNonempty(Query2, RewrittenQuery2),
  RewrittenQuery = orderby(RewrittenQuery2,X),
  write('7\n'),
  !.

rewriteQueryForNonempty(Query, RewrittenQuery) :-
  Query = groupby(Query2, X),
  rewriteQueryForNonempty(Query2, RewrittenQuery2),
  RewrittenQuery = groupby(RewrittenQuery2,X),
  write('8\n'),
  !.

% fallback case
%rewriteQueryForNonempty(Query,Query).

/*

14.1.2 Substitution of Conditions

If the where clause contains certain sets of conditions, these conditions may be
replaced by equivalent, but cheaper ones.

*/

% simple redundancy removement
simplifyCondition([starts(X,Y), contains(X,Y)], [contains(X,Y)]).

simplifyCondition([X <= Y, X = Y], [X <= Y]).
simplifyCondition([X <= Y, Y = X], [X <= Y]).

simplifyCondition([X >= Y, X = Y], [X >= Y]).
simplifyCondition([X >= Y, Y = X], [X >= Y]).

simplifyCondition([X < Y, X # Y], [X # Y]).
simplifyCondition([X < Y, Y # X], [X # Y]).

simplifyCondition([X > Y, X # Y], [X # Y]). 
simplifyCondition([X > Y, Y # X], [X # Y]). 

simplifyCondition([X = X], [X = X]).

simplifyCondition([X = Y, Y = X], [Y = X]).


% replacements with inferred conditions
% may need further processing 
expandCondition([X >= Y, X <= Y], [X = Y]).
expandCondition([X <= Y, X # Y], [X < Y]).
expandCondition([X >= Y, X # Y], [X > Y]).
expandCondition([not(isempty(atinstant(val(X),Y)))], [present(X,Y)]).

% Conditions resulting in false 
isFalseCondition(CondSet) :- 
  (
    member(false, CondSet)
  ; subset([X, not(X)], CondSet)
  ; subset([X > Y, X = Y], CondSet)
  ; subset([X < Y, X = Y], CondSet)
  ; subset([X < Y, X > Y], CondSet)
  ; subset([X <= Y, X > Y], CondSet)
  ; subset([X < Y, X >= Y], CondSet)
  ; subset([X # Y, X = Y], CondSet)
  ; subset([X # X], CondSet)
  ),
  write('\nYour where clause is syntactically equivalent to \'FALSE\'. \n'),
  write('This makes no sense as the query will return an empty result.\n'),
  write('Therefore, the query is canceled\n\n'),
  !.


simplifyOneCondition(In, Remove) :-
  simplifyCondition(X,Remove),
  subset(X,In).

simplifyAllConditions(CondIn, SimpleConds) :-
  findall(Out, simplifyOneCondition(CondIn,Out), RemoveList),
  flatten(RemoveList,FlatRemoveList),
  list_to_set(FlatRemoveList,RemoveSet),
  subtract(CondIn, RemoveSet, SimpleConds).

analyseConditions(ConditionsIn,WhereOut):-
%  doAllExpansions(ConditionsIn,ConditionsOut),
  not(isFalseCondition(ConditionsIn)),
  simplifyAllConditions(ConditionsIn,WhereOut).

%%%%%%

rewriteQueryForInferredSubstitutions(QIn, QIn) :-
  % case: no where clause
  QIn = from(select(_),WhereClause),
  WhereClause \= where(_,_).

rewriteQueryForInferredSubstitutions(QIn, QOut) :-
  % case: non-empty where clause
  QIn = from(select(SelClause),where(Rels,WhereIn)),
  analyseConditions(WhereIn,WhereOut),
  QOut = from(select(SelClause),where(Rels,WhereOut)).

% Special cases: ordering and grouping clauses can be ignored
rewriteQueryForInferredSubstitutions(Query, RewrittenQuery) :-
  Query = first(Query2, X),
  rewriteQueryForInferredSubstitutions(Query2, RewrittenQuery2),
  RewrittenQuery = first(RewrittenQuery2,X),
  !.

rewriteQueryForInferredSubstitutions(Query, RewrittenQuery) :-
  Query = orderby(Query2, X),
  rewriteQueryForInferredSubstitutions(Query2, RewrittenQuery2),
  RewrittenQuery = orderby(RewrittenQuery2,X),
  !.

rewriteQueryForInferredSubstitutions(Query, RewrittenQuery) :-
  Query = groupby(Query2, X),
  rewriteQueryForInferredSubstitutions(Query2, RewrittenQuery2),
  RewrittenQuery = groupby(RewrittenQuery2,X),
  !.

/*

14.2 Redundancy Elimination

Redundant clauses should be removed.

*/

/*
---- rewriteQueryForRedundancy(+Query,-RewrittenQuery).
----

rewrites ~Query~ by dropping redundant where clauses and optimizing
equivalent expressions. At the moment, only doublets within the list of where
conditions are dropped.

*/

rewriteQueryForRedundancy(Query, RewrittenQuery) :-
  Query = first(X),
  rewriteQueryForRedundancy(X, RX),
  RewrittenQuery = first(RX),
  !.

rewriteQueryForRedundancy(Query, RewrittenQuery) :-
  Query = orderby(X,Y),
  rewriteQueryForRedundancy(X, RX),
  RewrittenQuery = orderby(RX,Y),
  !.

rewriteQueryForRedundancy(Query, RewrittenQuery) :-
  Query = groupby(X,Y),
  rewriteQueryForRedundancy(X, RX),
  RewrittenQuery = groupby(RX,Y),
  !.

rewriteQueryForRedundancy(Query, RewrittenQuery) :-
  Query = from(select(SelClause), where(Rels,WhereClause)),  
  list_to_set(WhereClause,RewrittenWhereClause),  % eliminate condition doublets  
  (RewrittenWhereClause = [] -> 
     (RewrittenQuery = from(select(SelClause), Rels))
   ; (RewrittenQuery = from(select(SelClause), where(Rels,RewrittenWhereClause)))
  ),
  !.

rewriteQueryForRedundancy(Query,Query).


/*

14.3 Handle Common SubExpressions (CSEs)

The repeated evaluation of common subexpressions (CSEs) should be avoided, 
especially if complex funtions are used or big objects are created. Intermediary
results can be stored by extending the tuples by an addiditonal attribute using
the CSE as a function to calculate the attribute values. The extend should be 
called immeadiatly before the CSE occurs for the first time; and it should be 
removed immeadetely after its last occurence (in sequence of evaluation). 
If the CSE is named using the keyword ``as'' in the select-clause, no remove 
is needed and the final rename can be omitted. The same holds for queries, where 
the remove would be inserted directly in front of a final consume(project[...]).

When replacing occurences of CSEs, one starts at the bottom of the operator tree
and inserts the extend beneath the first occurence. Then, the path to the root
is followed upward, replacing all further occurences of the CSE with the 
extended attribute.

*/

/*
---- rewriteQueryForCSE(+Query,-RewrittenQuery)
----

Rewrites ~Query~ such that ~RewrittenQuery~ does not contain ~expensive~ common
subexpressions (CSEs) anymore. A CSE is expensive, if it contains at least one
operator labeled by ~rewritingCSEExpensiveOP/1~. The definitions of 
~rewritingCSEExpensiveOP/1~ should be moved to file operators.pl later.

*/

rewriteQueryForCSE(Query, Query). % XRIS: Extend this


/*

14.3.1 Auxiliary Predicates to ~rewriteQueryForCSE/2~

---- isSubTerm(+Term,-SubTerm)
----

succeeds, iff ~SubTerm~ is a real (not equal) subterm of ~Term~.

*/

isSubTerm(Term,SubTerm) :-
  term_to_atom(Term, TermA),
  term_to_atom(SubTerm, SubTermA),
  sub_atom(TermA, Before, _, After, SubTermA),
  After + Before > 0.

/*
---- replace_term(+Term, +SubExpr, +Replacement, -Result)
----

Replace any occurences of ground term ~SubExpr~ in expression ~Term~ with 
ground term ~Replacement~ and return the ~Result~.

*/

replace_termL([], _, _, []).
replace_termL([Me|Others], SubExpr, Repl, [MeRepl|OthersRepl]) :-
  replace_term(Me, SubExpr, Repl, MeRepl),
  replace_termL(Others, SubExpr, Repl, OthersRepl).

replace_term(Term, Term, Replacement, Replacement).

replace_term(Term, SubExpr, Replacement, Result) :-
  compound(Term),
  Term \= SubExpr,
  Term =.. [Me|Args],
  replace_termL(Args, SubExpr, Replacement, ArgsResult),
  Result =.. [Me|ArgsResult].

replace_term(Term, SubExpr, _, Term) :-
  atomic(Term),
  Term \= SubExpr.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


/*
---- markNode(+Node, -NodeMarked, +Level)
----

Creates a dynamic table ~storedExpressionLabel(Expr,Label,LevelList)~ by
parsing the operator tree for term ~node~. Returns the same term in ~NodeMarked~.
~Level~ gives the level of the operator tree`S ROOT (usually 0).

Each sub-expression ~Expr~ is labeled with an unique identifier ~label~ and a 
list ~LevelList~ of all levels within the operator tree, where ~Expr~ occurs 
as a sub-expression.

*/

:- dynamic(storedExpressionLabel/3),
   reset_gensym. % reset unique identifier generator

% delete table of label-term associations
retractExpressionLabels :-
  retractall(storedExpressionLabel(_, _, _)),
  reset_gensym. % reset unique identifier generator

% print a table of all stored term-label associations
showExpressionLabel :-
  storedExpressionLabel(Expr,Label,LevelList),
  write(' '), write(Label), write('   '), write(LevelList), 
  write('\t'), write(Expr), nl,
  fail.

showExpressionLabels :-
  write('\nLabeled Expressions:\n'),
  findall(_,showExpressionLabel,_),
  nl.
  
% succeeds, if Expr1 = Expr2 or (Expr1 = attr(X,_,Y) and Expr2 = attr(X,_Y))
compareExpression(Expr1,Expr1) :- !.
compareExpression(attr(X,_,Y),attr(X,_,Y)) :- !.
compareExpression(Expr1,Expr2) :- 
  Expr1 =.. [Op|Args1],
  Expr2 =.. [Op|Args2],
  compareExpression2(Args1,Args2),
  !.
compareExpression2([],[]).
compareExpression2([M1|O1],[M2|O2]) :- 
  compareExpression(M1,M2),
  compareExpression2(O1,O2),
  !.

% return the label associated with a known Node, or create a new association 
% and return that new label 
getExpressionLabel(Node1, Label, Level) :-
  storedExpressionLabel(Node2, Label, LevelList), 
  compareExpression(Node1,Node2),                  % this is a CSE
  retractall(storedExpressionLabel(Node, _, _)),
  !,
  assert(storedExpressionLabel(Node, Label, [Level|LevelList])).

getExpressionLabel(Node, Label, Level) :-
  not(storedExpressionLabel(Node, _, _)),
  gensym(cse_, Label),
  !,
  assert(storedExpressionLabel(Node, Label, [Level])).

% mark a list of arguments
markNode1([],[], _).            % nothing to do
markNode1([Me|Others],[MeMarked|OthersMarked],Level) :-
    markNode1(Others, OthersMarked,Level),
    markNode(Me,MeMarked,Level),
    !.

markNode(Node, Node, _) :-     % Node is a leaf. No label is assigned to leaves.
  atomic(Node);
  Node = :(_,_),
  !.

markNode(NodeList, MarkedNodeList, Level) :-
  is_list(NodeList),
  markArgs(NodeList, MarkedNodeList, Level),
  !.

markNode(Node, NodeMarked, Level) :-
  compound(Node),             % Node is an inner node.
  not(is_list(Node)),
  Node =.. [Me|MyArgs],       % decompose node
  NextLevel is Level+1,
  markNode1(MyArgs, MyArgsMarked, NextLevel),
  getExpressionLabel(Node,MyLabel,Level), % get label and update ExprList
%  MeMarked =.. [Me|MyArgsMarked],                 % label the node
%  NodeMarked =.. [xxx, MeMarked, MyLabel, Level], % label the node
  NodeMarked =.. [Me|MyArgsMarked],                % w/o labeling
  !.


/*
---- baseAttributes(+Term,-BaseAttributeList).
----

Collects the set ~BaseAttributeList~ of all attributes used within ~Term~.

*/

baseAttributes1([],[]) :- !.
  % no more arguments

baseAttributes1([Me|Others],BaseAttributes) :- 
  % process argument list
  baseAttributes(Me,MyBAList),
  baseAttributes(Others,OthersBAList),
  merge_set(MyBAList,OthersBAList,BaseAttributes),
  !.

baseAttributes(Label,RecList) :- 
  % term is a CSE attribute - recurse into CSE
  atom(Label),
  sub_atom(Label, 0, _, _, cse_),
  virtual_attribute(Label, Expr, _),
  baseAttributes(Expr,RecList),
  !.

baseAttributes(Term,[Term]) :- 
    atom(Term)     % attribute or secondo object
  ; Term = :(_,_), % renamed attribute
  !.

baseAttributes(Term,[]) :- 
  % numbers and strings are boring
  atomic(Term),
  !.

baseAttributes(as(Term,_),BaseAttributeList) :- 
  % we are not interested in aliases here
  baseAttributes(Term,BaseAttributeList),
  !.

baseAttributes(Term,BaseAttributeList) :- 
  % the base case
  compound(Term),
  Term =.. [_|Args],
  baseAttributes1(Args,BaseAttributeList),
  !.

isCSE(Expr) :-
  storedExpressionLabel(Expr,_,LevelList),
  not(length(LevelList,1)), !.

isNoCSE(Expr) :-
  storedExpressionLabel(Expr,_,LevelList),
  length(LevelList,1), !.

isNoCSE(Expr) :-
  \+(storedExpressionLabel(Expr,_,_)), !.

hasNoStoredCSE(Expr) :- % succeeds, iff Expr does not contain any CSE
  storedExpressionLabel(Expr,_,_),         % if Expr is a term
  \+ ((
       storedExpressionLabel(Expr1,_,_),   % Expr1 is a term
       isSubTerm(Expr,Expr1),            % Expr1 is a subterm of Expr
       isCSE(Expr1)                        % Expr1 is a CSE
     )), !.
  
replaceAllCSEs :-
  storedExpressionLabel(Expr1,Label1,_), % choose a term Expr1
  isCSE(Expr1),                     % Expr1 is a CSE
  hasNoStoredCSE(Expr1), % Expr1 contains no subterm, that is a CSE
  baseAttributes(Expr1,BaseAttrs1), % get base attributes for Expr1
  assert(virtual_attribute(Label1, Expr1,BaseAttrs1)), 
        % add (Label1,Expr1) to the the table of 'virtual attributes'
  retractall(storedExpressionLabel(Expr1,_,_)),        
        % retract Expr1 from term table to reduce search space
  storedExpressionLabel(Expr2,Label2,LevelList2),      
        % choose a second term Expr2
  isSubTerm(Expr2,Expr1),
        % Expr2 contains CSE Expr1 as a real subterm
  % replace Expr1 in all other stored terms Expr2 with Label1:
  replace_term(Expr2, Expr1, Label1, Expr2Replaced),
  retractall(storedExpressionLabel(Expr2,_,_)),
  retractall(storedExpressionLabel(Expr2Replaced,_,_)),
  assert(storedExpressionLabel(Expr2Replaced,Label2,LevelList2)),
  fail.
  

% examples for expensive operators
rewritingCSEExpensiveOP(intersection).
rewritingCSEExpensiveOP(minus).
rewritingCSEExpensiveOP(atinstant).
rewritingCSEExpensiveOP(atperiod). % ?
rewritingCSEExpensiveOP(at).       % ?
rewritingCSEExpensiveOP(crossings).
rewritingCSEExpensiveOP(center).
rewritingCSEExpensiveOP(touchpoints).
rewritingCSEExpensiveOP(commonborder).
rewritingCSEExpensiveOP(trajectory).
rewritingCSEExpensiveOP(distance).


% succeeds, iff Expr contains an expensive operator
rewritingCSEisExpensiveExpression(Expr) :-
  rewritingCSEExpensiveOP(Expr).

rewritingCSEisExpensiveExpression(Expr) :-
  is_list(Expr),
  Expr = [],
  fail.

rewritingCSEisExpensiveExpression(Expr) :-
  is_list(Expr),
  Expr = [Me|Others],
  (   rewritingCSEisExpensiveExpression(Me)
    ; rewritingCSEisExpensiveExpression(Others)
  ),
  !.

rewritingCSEisExpensiveExpression(Expr) :-
  compound(Expr),
  Expr =.. [OP|Args],
  (
      rewritingCSEExpensiveOP(OP)
    ; rewritingCSEisExpensiveExpression(Args)
  ),
  !.

