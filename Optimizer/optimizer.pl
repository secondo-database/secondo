/*
//paragraph [10] title:	[{\Large \bf ]	[}]
//characters [1] formula: 	[$] 	[$]
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[**] [$**$]
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

----	select *
	from Staedte, Laender, Regiert
	where Land = LName and PName = 'CDU' and LName = PLand
----

for relation schemas

----	Staedte(SName, Bev, Land)
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

----	pr(P, A)
	pr(P, B, C)
----

A selection or join predicate, e.g. pr(p, a), pr(q, b, c). Means a
selection predicate p on relation a, and a join predicate q on relations
b and c.

----	arp(Arg, Rels, Preds)
----

An argument, relations, predicate triple. It describes a set of relations
~Rels~ on which the predicates ~Preds~ have been evaluated. To access the
result of this evaluation one needs to refer to ~Arg~.

Arg is either arg(N) or res(N), N an integer. Examples: arg(5), res(1)

Rels is a list of relation names, e.g. [a, b, c]

Preds is a list of predicate names, e.g. [p, q, r]


----	node(No, Preds, Partition)
----

A node.

~No~ is the number of the node into which the evaluated predicates
are encoded (each bit corresponds to a predicate number, e.g. node number
5 = 101 (binary) says that the first predicate (no 1) and the third
predicate (no 4) have been evaluated in this node. For predicate i,
its predicate number is "2^{i-1}"[1].

~Preds~ is the list of names of evaluated predicates, e.g. [p, q].

~Partition~ is a list of arp elements, see above.


----	edge(Source, Target, Term, Result, Node, PredNo)
----

An edge, representing a predicate.

~Source~ and ~Target~ are the numbers of source and target nodes in the
predicate order graph, e.g. 0 and 1.

~Term~ is either a selection or a join, for example,
select(arg(0), pr(p, a) or join(res(4), res(1), pr(q, a, b))

~Result~ is the number of the node into which the result of this predicate
application should be written. Normally it is the same as Target,
but for an edge leading to a node combining several independent results,
it the number of the ``real'' node to obtain this result. An example of this can
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

----	pog(Rels, Preds, Nodes, Edges) :-
----

For a given list of relations ~Rels~ and predicates ~Preds~, ~Nodes~ and
~Edges~ are the predicate order graph where edges are annotated with selection
and join operations applied to the correct arguments.

Example call:

----	pog([staedte, laender], [pr(p, staedte), pr(q, laender), pr(r, staedte,
	laender)], N, E).
----

*/

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
  deleteSizes,
  deleteCostEdges.
/*

3.2 partition

----	partition(Rels, N, Partition0) :-
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

----	pog2(Partition0, NoOfPreds, Preds, Nodes, Edges) :-
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

----	newNodes(Pred, PredNo, NodesOld, NodesNew) :-
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

----	copyPart(Pred, PNo, Part, Part2) :-
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

----	newEdges(Pred, PredNo, NodesOld, EdgesNew) :-
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

----	findRel(Rel, Node, Source, Arg):-
----

find the relation ~Rel~ within a node description ~Node~ and return the
node number ~No~ and the description ~Arg~ of the argument (e.g. res(3)) found
within the arp containing Rel.

----	findRels(Rel1, Rel2, Node, Source, Arg1, Arg2):-
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

----	copyEdges(Pred, PredNo, EdgesOld, EdgesCopy):-
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

----	writeEdgeList(List):-
----

Write the list of edges ~List~.

*/

writeEdgeList([edge(Source, Target, Term, _, _, _) | Edges]) :-
  write(Source), write('-'), write(Target), write(':'), write(Term), nl,
  writeEdgeList(Edges).

/*
4 Managing the Graph in Memory

4.1 Storing and Deleting Nodes and Edges

----	storeNodes(NodeList).
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

----	writeNodes.
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

---- 	plz(PLZ:string, Ort:string)
	Staedte(SName:string, Bev:int, PLZ:int, Vorwahl:string, Kennzeichen:string)
----

Then we should be able to enter queries:

---- 	select SName, Bev
	from Staedte
	where Bev > 500000
----

In the next example we need to avoid the name conflict for PLZ

----	select *
	from Staedte as s, plz as p
	where s.SName = p.Ort and p.PLZ > 40000
----

In the PROLOG version, we will use the following notations:

----	rel(Name, Var, Case)
----

For example

----	rel(staedte, *, u)
----

is a term denoting the ~Staedte~ relation; ~u~ says that it is actually to be 
written in upper case whereas

----	rel(plz, *, l)
----

denotes the ~plz~ relation to be written in lower case. The second argument
~Var~ contains an explicit variable if it has been assigned, otherwise the 
symbol [*]. If an explicit variable has been used in the query, we need to 
perfom renaming in the plan. For example, in the second query above, the 
relations would be denoted as 

----	rel(staedte, s, u)
	rel(plz, p, l)
----

Within predicates, attributes are annotated as follows:

----	attr(Name, Arg, Case)

	attr(ort, 2, u)
----

This says that  ~ort~ is an attribute of the second argument within a join 
condition, to be written in upper case. For a selection condition, the second 
argument is ignored; it can be set to 0 or 1.

Hence for the two queries above, the translation would be

----	fromwhere(
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

----	feed:		rel(Tuple) -> stream(Tuple)
	consume:	stream(Tuple) -> rel(Tuple)

	filter:		stream(Tuple) x (Tuple -> bool) -> stream(Tuple)
	product:	stream(Tuple1) x stream(Tuple2) -> stream(Tuple3)

				where Tuple3 = Tuple1 o Tuple2

	hashjoin:	stream(Tuple1) x stream(Tuple2) x attrname1 x attrname2
				x nbuckets -> stream(Tuple3)

				where 	Tuple3 = Tuple1 o Tuple2
					attrname1 occurs in Tuple1
					attrname2 occurs in Tuple2
					nbuckets is the number of hash buckets
						to be used

	sortmergejoin:	stream(Tuple1) x stream(Tuple2) x attrname1 x attrname2
				-> stream(Tuple3)

				where 	Tuple3 = Tuple1 o Tuple2
					attrname1 occurs in Tuple1
					attrname2 occurs in Tuple2
					
	loopjoin:	stream(Tuple1) x (Tuple1 -> stream(Tuple2)
				-> stream(Tuple3)

				where 	Tuple3 = Tuple1 o Tuple2
				
	exactmatch:	btree(Tuple, AttrType) x rel(Tuple) x AttrType
				-> stream(Tuple)

	extend:		stream(Tuple1) x (Newname x (Tuple -> Attrtype))+
				-> stream(Tuple2)

				where 	Tuple2 is Tuple1 to which pairs
					(Newname, Attrtype) have been appended

	remove:		stream(Tuple1) x Attrname+ -> stream(Tuple2)

				where	Tuple2 is Tuple1 from which the mentioned
					attributes have been removed.

	project:	stream(Tuple1) x Attrname+ -> stream(Tuple2)

				where	Tuple2 is Tuple1 projected on the
					mentioned attributes.

	rename		stream(Tuple1) x NewName -> stream(Tuple2)

				where 	Tuple2 is Tuple1 modified by appending
					"_newname" to each attribute name

	count		stream(Tuple) -> int

				count the number of tuples in a stream

	sortby		stream(Tuple) x (Attrname, asc/desc)+	-> stream(Tuple)

				sort stream lexicographically by the given
				attribute names
----

In PROLOG, all expressions involving such operators are written in prefix
notation.

Parameter functions are written as

----	fun([param(Var1, Type1), ..., paran(VarN, TypeN)], Expr)
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
  atom_codes(Lower, [First | Rest]),
  to_upper(First, First2),
  UpperList = [First2 | Rest],
  atom_codes(Upper, UpperList).

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

:-
  dynamic(varDefined/1).

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


plan_to_atom(fun(Params, Expr), Result) :-
  params_to_atom(Params, ParamAtom),
  plan_to_atom(Expr, ExprAtom),
  concat_atom(['fun ', ParamAtom, ExprAtom], '', Result),
  !.


plan_to_atom(attribute(X, Y), Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  concat_atom(['attr(', XAtom, ', ', YAtom, ')'], '', Result),
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
  atom_concat('.', ResA, Result).

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


/*
Translation of operators driven by predicate ~secondoOp~ in 
file ~opSyntax~. There are rules for

  * postfix, 1 or 2 arguments

  * postfix followed by one argument in square brackets, in total 2 
or 3 arguments

  * prefix, 2 arguments

Other syntax, if not default (see below) needs to be coded explicitly.

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

params_to_atom([param(Var, Type) | Params], Result) :-
  type_to_atom(Type, TypeAtom),
  params_to_atom(Params, ParamsAtom),
  concat_atom(['(', Var, ': ', TypeAtom, ') ', ParamsAtom], '', Result),
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
  argument(N, rel(Name, *, Case)), !.

arg(N) => rename(feed(rel(Name, Var, Case)), Var) :-
  argument(N, rel(Name, Var, Case)).

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
  hasIndex(rel(Name, *, Case), attr(AttrName, Arg, AttrCase), IndexName).

indexselect(arg(N), pr(Y = attr(AttrName, Arg, AttrCase), _)) =>
  rename(exactmatch(IndexName, rel(Name, Var, Case), Y), Var)
  :-
  argument(N, rel(Name, Var, Case)),
  !,
  hasIndex(rel(Name, Var, Case), attr(AttrName, Arg, AttrCase), IndexName).

indexselect(arg(N), pr(attr(AttrName, Arg, Case) <= Y, Rel)) => X :-
  indexselect(arg(N), pr(Y >= attr(AttrName, Arg, Case), Rel)) => X.

indexselect(arg(N), pr(Y >= attr(AttrName, Arg, AttrCase), _)) =>
  leftrange(IndexName, rel(Name, *, Case), Y)
  :-
  argument(N, rel(Name, *, Case)),
  !,
  hasIndex(rel(Name, *, Case), attr(AttrName, Arg, AttrCase), IndexName).

indexselect(arg(N), pr(Y >= attr(AttrName, Arg, AttrCase), _)) =>
  rename(leftrange(IndexName, rel(Name, Var, Case), Y), Var)
  :-
  argument(N, rel(Name, Var, Case)),
  !,
  hasIndex(rel(Name, Var, Case), attr(AttrName, Arg, AttrCase), IndexName).

indexselect(arg(N), pr(attr(AttrName, Arg, Case) >= Y, Rel)) => X :-
  indexselect(arg(N), pr(Y <= attr(AttrName, Arg, Case), Rel)) => X.

indexselect(arg(N), pr(Y <= attr(AttrName, Arg, AttrCase), _)) =>
  rightrange(IndexName, rel(Name, *, Case), Y)
  :-
  argument(N, rel(Name, *, Case)),
  !,
  hasIndex(rel(Name, *, Case), attr(AttrName, Arg, AttrCase), IndexName).

indexselect(arg(N), pr(Y <= attr(AttrName, Arg, AttrCase), _)) =>
  rename(rightrange(IndexName, rel(Name, Var, Case), Y), Var)
  :-
  argument(N, rel(Name, Var, Case)),
  !,
  hasIndex(rel(Name, Var, Case), attr(AttrName, Arg, AttrCase), IndexName).


/*
Here ~ArgS~ is meant to indicate ``argument stream''.

5.2.3 Translation of Joins

A join can always be translated to filtering the Cartesian product.

*/

join(Arg1, Arg2, pr(Pred, _, _)) => filter(product(Arg1S, Arg2S), Pred) :-
  Arg1 => Arg1S,
  Arg2 => Arg2S.


/*

Index joins:

*/


join(Arg1, arg(N), pr(X=Y, _, _)) => loopjoin(Arg1S, MatchExpr) :-
  isOfSecond(Attr2, X, Y),
  isNotOfSecond(Expr1, X, Y),
  argument(N, RelDescription),
  hasIndex(RelDescription, Attr2, IndexName),
  Arg1 => Arg1S,
  exactmatch(IndexName, arg(N), Expr1) => MatchExpr.

join(arg(N), Arg2, pr(X=Y, _, _)) => loopjoin(Arg2S, MatchExpr) :-
  isOfFirst(Attr1, X, Y),
  isNotOfFirst(Expr2, X, Y),
  argument(N, RelDescription),
  hasIndex(RelDescription, Attr1, IndexName),
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

----	select *
	from plz as p1, plz as p2
	where p1.PLZ = p2.PLZ + 1
----

can be translated to

----	plz feed {p1} plz feed {p2} extend[newPLZ: PLZ_p2 + 1]
	hashjoin[PLZ_p1, newPLZ, 997]
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
	attrname(Attr1), attrname(Attr2), 997)   :-
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).

/*

----	isOfFirst(Attr, X, Y)
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

writePlanEdge :-
  planEdge(Source, Target, Plan, Result),
  write('Source: '), write(Source), nl,
  write('Target: '), write(Target), nl,
  write('Plan: '), wp(Plan), nl,
  % write(Plan), nl,
  write('Result: '), write(Result), nl, nl,
  fail.

writePlanEdges :- not(writePlanEdge).




/*
7 Assigning Sizes and Selectivities to the Nodes and Edges of the POG

----	assignSizes.
	deleteSizes.
----

Assign sizes (numbers of tuples) to all nodes in the pog, based on the
cardinalities of the argument relations and the selectivities of the
predicates. Store sizes as facts of the form resultSize(Result, Size). Store
selectivities as facts of the form edgeSelectivity(Source, Target, Sel).

Delete sizes from memory.

7.1 Assigning Sizes and Selectivities

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
  selectivity(Pred, Sel),
  Size is Card * Sel,
  setNodeSize(Result, Size),
  assert(edgeSelectivity(Source, Target, Sel)).

assignSize(Source, Target, join(Arg1, Arg2, Pred), Result) :-
  resSize(Arg1, Card1),
  resSize(Arg2, Card2),
  selectivity(Pred, Sel),
  Size is Card1 * Card2 * Sel,
  setNodeSize(Result, Size),
  assert(edgeSelectivity(Source, Target, Sel)).

/*
----	setNodeSize(Node, Size) :-
----

Set the size of node ~Node~ to ~Size~ if no size has been assigned before.

*/

setNodeSize(Node, _) :- resultSize(Node, _), !.
setNodeSize(Node, Size) :- assert(resultSize(Node, Size)).

/*
----	resSize(Arg, Size) :-
----

Argument ~Arg~ has size ~Size~.

*/

resSize(arg(N), Size) :- argument(N, rel(Rel, _, _)), card(Rel, Size), !.
resSize(arg(N), _) :- write('Error in optimizer: cannot find cardinality for '),
  argument(N, Rel), wp(Rel), nl, fail.
resSize(res(N), Size) :- resultSize(N, Size), !.

/*
----	writeSizes :-
----

Write sizes and selectivities.

*/

writeSize :-
  resultSize(Node, Size),
  write('Node: '), write(Node), nl,
  write('Size: '), write(Size), nl, nl,
  fail.
writeSize :-
  edgeSelectivity(Source, Target, Sel),
  write('Source: '), write(Source), nl,
  write('Target: '), write(Target), nl,
  write('Selectivity: '), write(Sel), nl, nl,
  fail.
writeSizes :- not(writeSize).

/*
----	deleteSizes :-
----

Delete node sizes and selectivities of edges.

*/

deleteSize :- retract(resultSize(_, _)), fail.
deleteSize :- retract(edgeSelectivity(_, _, _)), fail.
deleteSizes :- not(deleteSize).


/*
8 Computing Edge Costs for Plan Edges

8.1 The Costs of Terms

----	cost(Term, Sel, Size, Cost) :-
----

The cost of an executable ~Term~ representing a predicate with selectivity ~Sel~
is ~Cost~ and the size of the result is ~Size~.

This is evaluated recursively descending into the term. When the operator
realizing the predicate (e.g. ~filter~) is encountered, the selectivity ~Sel~ is
used to determine the size of the result. It is assumed that only a single
operator of this kind occurs within the term.

8.1.1 Arguments

*/

cost(rel(Rel, _, _), _, Size, 0) :-
  card(Rel, Size).

cost(res(N), _, Size, 0) :-
  resultSize(N, Size).

/*
8.1.2 Operators

*/

cost(feed(X), Sel, S, C) :-
  cost(X, Sel, S, C1),
  feedTC(A),
  C is C1 + A * S.

/*
Here ~feedTC~ means ``feed tuple cost'', i.e., the cost per tuple, a constant to
be determined in experiments. These constants are kept in file ``Operators.pl''.

*/

cost(consume(X), Sel, S, C) :-
  cost(X, Sel, S, C1),
  consumeTC(A),
  C is C1 + A * S.

cost(filter(X, _), Sel, S, C) :-
  cost(X, 1, SizeX, CostX),
  filterTC(A),
  S is SizeX * Sel,
  C is CostX + A * SizeX.

/*
For the moment we assume a cost of 1 for evaluating a predicate; this should be
changed shortly.

*/

cost(product(X, Y), _, S, C) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, 1, SizeY, CostY),
  productTC(A, B),
  S is SizeX * SizeY,
  C is CostX + CostY + SizeY * B + S * A.


cost(leftrange(_, Rel, _), Sel, Size, Cost) :-
  cost(Rel, 1, RelSize, _),
  leftrangeTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

cost(rightrange(_, Rel, _), Sel, Size, Cost) :-
  cost(Rel, 1, RelSize, _),
  leftrangeTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

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

*/
cost(exactmatchfun(_, Rel, _), Sel, Size, Cost) :-
  cost(Rel, 1, RelSize, _),
  exactmatchTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

cost(exactmatch(_, Rel, _), Sel, Size, Cost) :-
  cost(Rel, 1, RelSize, _),
  exactmatchTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

cost(loopjoin(X, Y), Sel, S, Cost) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, Sel, SizeY, CostY),
  S is SizeX * SizeY,
  loopjoinTC(C),
  Cost is C * SizeX + CostX + SizeX * CostY.

cost(fun(_, X), Sel, Size, Cost) :-
  cost(X, Sel, Size, Cost).



/*

Previously the cost function for ~hashjoin~ contained a term

----    A * SizeX + A * SizeY
----

which should account for the cost of distributing tuples
into the buckets. However in experiments the cost of
hashing was always ten or more times smaller than the cost
of computing products of buckets. Therefore that term
was considered unnecessary.

*/
cost(hashjoin(X, Y, _, _, NBuckets), Sel, S, C) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, 1, SizeY, CostY),
  hashjoinTC(A, B),
  S is SizeX * SizeY * Sel,
  C is CostX + CostY +        			% producing the arguments
    A * NBuckets * (SizeX/NBuckets + 1) *       % computing the product for each
      (SizeY/NBuckets +1) +                     % pair of buckets
    B * S.                                      % producing the result tuples


cost(sortmergejoin(X, Y, _, _), Sel, S, C) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, 1, SizeY, CostY),
  sortmergejoinTC(A, B),
  S is SizeX * SizeY * Sel,
  C is CostX + CostY +				% producing the arguments
    A * SizeX * log(SizeX + 1) +
    A * SizeY * log(SizeY + 1) +		% sorting the arguments
    B * S.               			% parallel scan of sorted relations


cost(extend(X, _), Sel, S, C) :-
  cost(X, Sel, S, C1),
  extendTC(A),
  C is C1 + A * S.

cost(remove(X, _), Sel, S, C) :-
  cost(X, Sel, S, C1),
  removeTC(A),
  C is C1 + A * S.

cost(project(X, _), Sel, S, C) :-
  cost(X, Sel, S, C1),
  projectTC(A),
  C is C1 + A * S.

cost(rename(X, _), Sel, S, C) :-
  cost(X, Sel, S, C1),
  renameTC(A),
  C is C1 + A * S.

/*
8.2 Creating Cost Edges

These are plan edges extended by a cost measure.

*/

createCostEdge :-
  planEdge(Source, Target, Term, Result),
  edgeSelectivity(Source, Target, Sel),
  cost(Term, Sel, Size, Cost),
  assert(costEdge(Source, Target, Term, Result, Size, Cost)),
  fail.

createCostEdges :- not(createCostEdge).

deleteCostEdge :-
  retract(costEdge(_, _, _, _, _, _)), fail.

deleteCostEdges :- not(deleteCostEdge).

writeCostEdge :-
  costEdge(Source, Target, Plan, Result, Size, Cost),
  write('Source: '), write(Source), nl,
  write('Target: '), write(Target), nl,
  write('Plan: '), wp(Plan), nl,
  write('Result: '), write(Result), nl, 
  write('Size: '), write(Size), nl, 
  write('Cost: '), write(Cost), nl, 
  nl,
  fail.

writeCostEdges :- not(writeCostEdge).

/*
----	assignCosts
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

----	node(Name, Distance, Path)
----

where ~Name~ is the node number, ~Distance~ the distance along the shortest
path to this node, and ~Path~ is the list of edges forming the shortest path.
   
The graph is represented by the set of ~costEdges~. 

The center is represented as a set of facts of the form

----	center(NodeNumber, node(Name, Distance, Path))
----

Since predicates are generally indexed by their first argument, finding a node
in the center via the node number should be very efficient. We assume it is
possible in constant time.

The boundary is represented by an abstract data type as described in the
interface below. Essentially it is a priority queue implementation.

 
----	successor(Node, Succ) :- 
----

~Succ~ is a successor of node ~Node~ via some edge. This includes computation
of the distance and path of the successor.

*/ 

successor(node(Source, Distance, Path), node(Target, Distance2, Path2)) :-
  costEdge(Source, Target, Term, Result, Size, Cost),
  Distance2 is Distance + Cost,
  append(Path, [costEdge(Source, Target, Term, Result, Size, Cost)], Path2).

/*

----	dijkstra(Source, Dest, Path, Length) :- 
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
----	dijkstra1(Boundary, Dest, NoOfCalls) :-
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
writePath([costEdge(Source, Target, Term, Result, Size, Cost) | Path]) :-
  write(costEdge(Source, Target, Result, Size, Cost)), nl,
  write('    '), wp(Term), nl,
  writePath(Path).

/*
----	putsuccessors(Boundary, Node, BoundaryNew) :-
----

Insert into ~Boundary~  all successors of node ~Node~ not yet present in
the center, updating their distance if they are already present, to obtain
~BoundaryNew~. 

*/   
putsuccessors(Boundary, Node, BoundaryNew) :-  
  findall(Succ, successor(Node, Succ), Successors),

	% write('successors of '), write(Node), nl,
	% writeList(Successors), nl, nl,

  putsucc1(Boundary, Successors, BoundaryNew). 
 
/* 
----	putsucc1(Boundary, Successors, BoundaryNew) :- 
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

----	b_empty(-Boundary) :-
----

Creates an empty boundary and returns it.

----	b_isEmpty(+Boundary) :-
----

Checks whether the boundary is empty.


----	b_removemin(+Boundary, -Node, -BoundaryOut) :-
---- 

Returns the node ~Node~ with minimal distance from the set ~Boundary~ and
returns also ~BoundaryOut~ where this node is removed.
   
----	b_insert(+Boundary, +Node, -BoundaryOut) :-
----
 
Inserts a node that must not yet be present (i.e., no other node of that
name).
   
----	b_memberByName(+Boundary, +Name, -Node) :-
---- 
 
If a node ~Node~ with name ~Name~ is present, it is returned. 
 
---- 	b_deleteByName(+Boundary, +Name, -BoundaryOut) :-
---- 
 
Returns the boundary, where the node with name ~Name~ is deleted. 
 
*/

/*
9.3 Constructing the Plan from the Shortest Path

----	plan(Path, Plan)
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

traversePath([costEdge(_, _, Term, Result, _, _) | Path]) :-
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
  Path2 = [costEdge(_, N, _, _, _, _) | _].


/*
9.4 Computing the Best Plan for a Given Predicate Order Graph

*/

bestPlan :-
  assignCosts,
  highNode(N),
  dijkstra(0, N, Path, Cost),
  plan(Path, Plan),
  write('The best plan is:'), nl, nl,
  wp(Plan),
  nl, nl,
  write('The cost is: '), write(Cost), nl.

bestPlan(Plan, Cost) :-
  assignCosts,
  highNode(N),
  dijkstra(0, N, Path, Cost),
  plan(Path, Plan).

/*
10 A Larger Example

It is now time to test efficiency with a larger example. We consider the query:

----	select *
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
    pr(attr(sName, 1, u) = attr(p1:ort, 2, u), rel(staedte, *, u), rel(plz, p1, l)),
    pr(attr(p1:pLZ, 1, u) = (attr(p2:pLZ, 2, u) + 1), rel(plz, p1, l), rel(plz, p2, l)),
    pr(attr(p2:pLZ, 1, u) = (attr(p3:pLZ, 2, u) * 5), rel(plz, p2, l), rel(plz, p3, l)),
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
    pr(attr(sName, 1, u) = attr(p1:ort, 2, u), rel(staedte, *, u), rel(plz, p1, l)),
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
    pr(attr(sName, 1, u) = attr(p1:ort, 2, u), rel(staedte, *, u), rel(plz, p1, l)),
    pr(attr(p1:pLZ, 1, u) = (attr(p2:pLZ, 2, u) + 1), rel(plz, p1, l), rel(plz, p2, l)),
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
    pr(attr(sName, 1, u) = attr(p1:ort, 2, u), rel(staedte, *, u), rel(plz, p1, l)),
    pr(attr(p1:pLZ, 1, u) = (attr(p2:pLZ, 2, u) + 1), rel(plz, p1, l), rel(plz, p2, l)),
    pr(attr(p2:pLZ, 1, u) = (attr(p3:pLZ, 2, u) * 5), rel(plz, p2, l), rel(plz, p3, l))
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

----	select <attr-list>
	from <rel-list>
	where <pred-list>
----

The first example query from [Section 4.1.1] can then be written as:

----	select [sname, bev]
	from [staedte]
	where [bev > 500000]
----

Instead of lists consisting of a single element we will also support writing
just the element, hence the query can also be written:

----	select [sname, bev]
	from staedte
	where bev > 500000
----

The second query can be written as:

----	select *
	from [staedte as s, plz as p]
	where [sname = p:ort, p:plz > 40000]
----

Note that all relation names and attribute names are written just in lower
case; the system will lookup the spelling in a table.

Furthermore, it will be possible to add a groupby- and an orderby-clause:

  * groupby

----	select <aggr-list>
	from <rel-list>
	where <pred-list>
	groupby <group-attr-list>
----

Example:

----	select [ort, min(plz) as minplz, max(plz) as maxplz,  count(*) as cntplz]
	from plz
	where plz > 40000
	groupby ort
----

  * orderby

----	select <attr-list>
	from <rel-list>
	where <pred-list>
	orderby <order-attr-list>
----

Example:

----	select [ort, plz]
	from plz
	orderby [ort#asc, plz#desc]
----

This example also shows that the where-clause may be omitted. It is also
possible to combine grouping and ordering:

----	select [ort, min(plz) as minplz, max(plz) as maxplz,  count(*) as cntplz]
	from plz
	where plz > 40000
	groupby ort
	orderby cntplz#desc
----

Currently only a basic part of this language has been implemented.


11.2 Structure

We introduce ~select~, ~from~, ~where~, and ~as~ as PROLOG operators:

*/

:- op(990, fx, sql).
:- op(985, xfx, >>).
:- op(950, fx, select).
:- op(960, xfx, from).
:- op(950, xfx, where).
:- op(930, xfx, as).
:- op(970, xfx, groupby).
:- op(980, xfx, orderby).
:- op(930, xf, asc).
:- op(930, xf, desc).

/*
This ensures that the select-from-where statement is viewed as a term with the
structure:

----	from(select(AttrList(), where(RelList, PredList))
----

That this works, can be tested with:

----	P = (select s:sname from staedte as s where s:bev > 500000),
	P = (X from Y), X = (select AttrList), Y = (RelList where PredList),
	RelList = (Rel as Var).
----

The result is:

----	P = select s:sname from staedte as s where s:bev>500000
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

----	relation(staedte, [sname, bev, plz, vorwahl, kennzeichen]).
	relation(plz, [plz, ort]).
----

The spelling of relation or attribute names is given in a table

----	spelling(staedte:plz, pLZ).
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

newQuery :- not(clearVariables), not(clearQueryRelations).

clearVariables :- retract(variable(_, _)), fail.

clearQueryRelations :- retract(queryRel(_, _)), fail.

/*

----	lookup(Query, Query2) :-
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















makeList(L, L) :- is_list(L).

makeList(L, [L]) :- not(is_list(L)).

/*

11.3.3 Modification of the From-Clause

----	lookupRels(Rels, Rels2)
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
----	lookupRel(Rel, Rel2) :-
----

Translate and store a single relation definition.

*/

:- dynamic
  variable/2,
  queryRel/2.

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
----	duplicateAttrs(Rel) :-
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
  spelled(Rel:Attr, attr(Attr2, _, Case)).

lookupAttr(Attr asc, Attr2 asc) :- !,
  lookupAttr(Attr, Attr2).

lookupAttr(Attr desc, Attr2 desc) :- !,
  lookupAttr(Attr, Attr2).

lookupAttr(Attr, Attr2) :-
  isAttribute(Attr, Rel), !,
  spelled(Rel:Attr, Attr2).

lookupAttr(*, *).

lookupAttr(count(*), count(*)).

lookupAttr(Name, Name) :-
  write('Error in attribute list: could not recognize '), write(Name), nl, fail.

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
----	lookupPred1(+Pred, Pred2, +N, +RelsBefore, -M, -RelsAfter) :-
----

~Pred2~ is the transformed version of ~Pred~; before this is called, ~N~
attributes in list ~RelsBefore~ have been found; after the transformation in
total ~M~ attributes referring to the relations in list ~RelsAfter~ have been
found.

*/

lookupPred1(Var:Attr, attr(Var:Attr2, N1, Case), N, RelsBefore, N1, RelsAfter)
  :-
  variable(Var, Rel2), !,   Rel2 = rel(Rel, _, _),
  spelled(Rel:Attr, attr(Attr2, _, Case)),
  N1 is N + 1,
  append(RelsBefore, [Rel2], RelsAfter).

lookupPred1(Attr, attr(Attr2, N1, Case), N, RelsBefore, N1, RelsAfter) :-
  isAttribute(Attr, Rel), !,
  spelled(Rel:Attr, attr(Attr2, _, Case)),
  queryRel(Rel, Rel2),
  N1 is N + 1,
  append(RelsBefore, [Rel2], RelsAfter).

lookupPred1(Term, Term2, N, RelsBefore, M, RelsAfter) :-
  compound(Term),
  functor(Term, F, 1), !,
  arg(1, Term, Arg1),
  lookupPred1(Arg1, Arg1Out, N, RelsBefore, M, RelsAfter),
  functor(Term2, F, 1),
  arg(1, Term2, Arg1Out).

lookupPred1(Term, Term2, N, RelsBefore, M, RelsAfter) :-
  compound(Term),
  functor(Term, F, 2), !,
  arg(1, Term, Arg1),
  arg(2, Term, Arg2),
  lookupPred1(Arg1, Arg1Out, N, RelsBefore, M1, RelsAfter1),
  lookupPred1(Arg2, Arg2Out, M1, RelsAfter1, M, RelsAfter),
  functor(Term2, F, 2),
  arg(1, Term2, Arg1Out),
  arg(2, Term2, Arg2Out).

% may need to be extended to operators with more than two arguments.

lookupPred1(Term, Term, N, Rels, N, Rels) :-
  atom(Term),
  not(is_list(Term)),
  write('Symbol '), write(Term), 
  write(' not recognized, supposed to be a Secondo object.'), nl, !.

lookupPred1(Term, Term, N, Rels, N, Rels).
 

/*
11.3.6 Check the Spelling of Relation and Attribute Names

*/

spelled(Rel:Attr, attr(Attr2, 0, l)) :-
  %spelling(Rel:Attr, lc(Attr2)), !.
  spelling(Rel:Attr, Attr3),
  Attr3 = lc(Attr2),
  !.

spelled(Rel:Attr, attr(Attr2, 0, u)) :-
  spelling(Rel:Attr, Attr2), !.

spelled(_:Attr, attr(Attr, 0, u)) :- !.	% no attr entry in spelling table

spelled(Rel, Rel2, l) :-
  %spelling(Rel, lc(Rel2)), !.
  spelling(Rel, Rel3),
  Rel3 = lc(Rel2),
  !.

spelled(Rel, Rel2, u) :-
  spelling(Rel, Rel2), !.

spelled(Rel, Rel, u).		% no rel entry in spelling table.

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

----	translate(Query, Stream, SelectClause, Cost) :-
----

~Query~ is translated into a ~Stream~ to which still the translation of the
~SelectClause~ needs to be applied. A ~Cost~ is returned which currently is
only the cost for evaluating the essential part, the conjunctive query.

*/

translate(Select from Rels where Preds, Stream, Select, Cost) :- !,
  pog(Rels, Preds, _, _),
  bestPlan(Stream, Cost).

translate(Select from Rel, feed(Rel), Select, 0) :-
  not(is_list(Rel)),
  !.

translate(Select from [Rel], feed(Rel), Select, 0).

translate(Select from [Rel | Rels], product(feed(Rel), Stream), Select, 0) :-
  translate(Select from Rels, Stream, Select, _).

translate(Query orderby Attrs, sortby(Stream, AttrNames), Select, 0) :-
  translate(Query, Stream, Select, _),
  attrnamesSort(Attrs, AttrNames).

/*

----	queryToPlan(Query, Plan, Cost) :-
----

Translate the ~Query~ into a ~Plan~. The ~Cost~ for evaluating the conjunctive
query is also returned. The ~Query~ must be such that relation and attribute
names have been looked up already.

*/

queryToPlan(Query, consume(Stream), Cost) :-
  translate(Query, Stream, select *, Cost), !.

queryToPlan(Query, count(Stream), Cost) :-
  translate(Query, Stream, select count(*), Cost), !.

queryToPlan(Query, consume(project(Stream, AttrNames)), Cost) :-
  translate(Query, Stream, select Attrs, Cost), !,
  makeList(Attrs, Attrs2),
  attrnames(Attrs2, AttrNames).



/*

----	queryToStream(Query, Plan, Cost) :-
----

Same as ~queryToPlan~, but returns a stream plan, if possible. To be used for 
``mixed queries'' that add Secondo operators to the plan built by the optimizer.

*/

queryToStream(Query,  Stream, Cost) :-
  translate(Query, Stream, select *, Cost), !.

queryToStream(Query, count(Stream), Cost) :-
  translate(Query, Stream, select count(*), Cost), !.

queryToStream(Query,  project(Stream, AttrNames), Cost) :-
  translate(Query, Stream, select Attrs, Cost), !,
  makeList(Attrs, Attrs2),
  attrnames(Attrs2, AttrNames).



/*

----	attrnames(Attrs, AttrNames) :-
----

Transform each attribute X into attrname(X).

*/

attrnames([], []).

attrnames([Attr | Attrs], [attrname(Attr) | AttrNames]) :-
  attrnames(Attrs, AttrNames).

/*

----	attrnamesSort(Attrs, AttrNames) :-
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

----	optimize(Query).
----

Optimize ~Query~ and print the best ~Plan~.

*/

optimize(Query) :-
  callLookup(Query, Query2),
  queryToPlan(Query2, Plan, Cost),
  plan_to_atom(Plan, SecondoQuery),
  write('The plan is: '), nl, nl,
  write(SecondoQuery), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl.


optimize(Query, QueryOut, CostOut) :-
  callLookup(Query, Query2),
  queryToPlan(Query2, Plan, CostOut),
  plan_to_atom(Plan, QueryOut).

/*
----	sqlToPlan(QueryText, Plan)
----

Transform an SQL ~QueryText~ into a ~Plan~. The query is given as a text atom.

*/
sqlToPlan(QueryText, Plan) :-
  term_to_atom(sql Query, QueryText),
  optimize(Query, Plan, _).


/*
----	sqlToPlan(QueryText, Plan)
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


Example3:

*/

example14 :- optimize(
  select * from [staedte as s, plz as p] where [p:ort = s:sname, p:plz > 40000, (p:plz mod 5) = 0]
  ).

example14(Query, Cost) :- optimize(
  select * from [staedte as s, plz as p] where [p:ort = s:sname, p:plz > 40000, (p:plz mod 5) = 0],
  Query, Cost
  ).


/*
Example4:

*/
example15 :- optimize(
  select * from staedte where bev > 500000
  ).

example15(Query, Cost) :- optimize(
  select * from staedte where bev > 500000,
  Query, Cost
  ).

/*
Example5:

*/
example16 :-  optimize(
  select * from [staedte as s, plz as p] where [s:sname = p:ort, p:plz > 40000]
  ).

example16(Query, Cost) :-  optimize(
  select * from [staedte as s, plz as p] where [s:sname = p:ort, p:plz > 40000],
  Query, Cost
  ).


/*
Example6. This may need a larger local stack size. Start Prolog as

----	pl -L4M
----

which initializes the local stack to 4 MB.

*/
example17 :- optimize(
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

example17(Query, Cost) :- optimize(
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


/*
Example 18:

*/
example18 :- optimize(
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

example18(Query, Cost) :- optimize(
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

/*
Example 19:

*/
example19 :- optimize(
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

example19(Query, Cost) :- optimize(
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


/*
Example 20:

*/
example20 :- optimize(
  select *
  from [staedte as s, plz as p]
  where [
    p:ort = s:sname,
    p:plz > 40000,
    s:bev > 300000]
  ).

example20(Query, Cost) :- optimize(
  select *
  from [staedte as s, plz as p]
  where [
    p:ort = s:sname,
    p:plz > 40000,
    s:bev > 300000],
  Query, Cost
  ).

/*
Example 21:

*/
example21 :- optimize(
  select *
  from [staedte, plz as p1, plz as p2, plz as p3]
  where [
    sname = p1:ort,
    p1:plz = p2:plz + 1,
    p2:plz = p3:plz * 5]
  ).

example21(Query, Cost) :- optimize(
  select *
  from [staedte, plz as p1, plz as p2, plz as p3]
  where [
    sname = p1:ort,
    p1:plz = p2:plz + 1,
    p2:plz = p3:plz * 5],
  Query, Cost
  ).

/*

12 Optimizing and Calling Secondo

----	sql Term
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
  mOptimize(Term, Query, Cost),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  query(Query).

sql(Term, SecondoQueryRest) :-
  mStreamOptimize(Term, SecondoQuery, Cost),
  concat_atom([SecondoQuery, ' ', SecondoQueryRest], '', Query),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  query(Query).

let(X, Term) :-
  mOptimize(Term, Query, Cost),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  concat_atom(['let ', X, ' = ', Query], '', Command),
  secondo(Command).

let(X, Term, SecondoQueryRest) :-
  mStreamOptimize(Term, SecondoQuery, Cost),
  concat_atom([SecondoQuery, ' ', SecondoQueryRest], '', Query),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  concat_atom(['let ', X, ' = ', Query], '', Command),
  secondo(Command).


/*
----	streamOptimize(Term, Query, Cost) :-
----

Optimize the ~Term~ producing an incomplete Secondo query plan ~Query~ 
returning a stream.

*/
streamOptimize(Term, Query, Cost) :-
  callLookup(Term, Term2),
  queryToStream(Term2, Plan, Cost),
  plan_to_atom(Plan,  Query).

/*
----	mOptimize(Term, Query, Cost) :-
	mStreamOptimize(union [Term], Query, Cost) :-
----

Means ``multi-optimize''. Optimize a ~Term~ possibly consisting of several subexpressions to be independently optimized, as in union and intersection queries. ~mStreamOptimize~ is a variant returning a stream.

*/

:-op(800, fx, union).
:-op(800, fx, intersection).

mOptimize(union Terms, Query, Cost) :-
  mStreamOptimize(union Terms, Plan, Cost),
  concat_atom([Plan, 'consume'], '', Query).

mOptimize(intersection Terms, Query, Cost) :-
  mStreamOptimize(intersection Terms, Plan, Cost),
  concat_atom([Plan, 'consume'], '', Query).

mOptimize(Term, Query, Cost) :-
  optimize(Term, Query, Cost).


mStreamOptimize(union [Term], Query, Cost) :-
  streamOptimize(Term, QueryPart, Cost),
  concat_atom([QueryPart, 'sort rdup '], '', Query).

mStreamOptimize(union [Term | Terms], Query, Cost) :-
  streamOptimize(Term, Plan1, Cost1),
  mStreamOptimize(union Terms, Plan2, Cost2),
  concat_atom([Plan1, 'sort rdup ', Plan2, 'mergeunion '], '', Query),
  Cost is Cost1 + Cost2.

mStreamOptimize(intersection [Term], Query, Cost) :-
  streamOptimize(Term, QueryPart, Cost),
  concat_atom([QueryPart, 'sort rdup '], '', Query).

mStreamOptimize(intersection [Term | Terms], Query, Cost) :-
  streamOptimize(Term, Plan1, Cost1),
  mStreamOptimize(intersection Terms, Plan2, Cost2),
  concat_atom([Plan1, 'sort rdup ', Plan2, 'mergesec '], '', Query),
  Cost is Cost1 + Cost2.

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







