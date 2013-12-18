/*

----
This file is part of SECONDO.

Copyright (C) 2004-2008, University Hagen, Faculty of Mathematics and
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
//[:Section Translation] [\label{sec:translation}]
//[Section Translation] [Section~\ref{sec:translation}]
//[:Section 4.1.1] [\label{sec:4.1.1}]
//[Section 4.1.1] [Section~\ref{sec:4.1.1}]
//[Figure pog1] [Figure~\ref{fig:pog1.eps}]
//[Figure pog2] [Figure~\ref{fig:pog2.eps}]
//[newpage] [\newpage]

*/

:- op(994, xfy,  ::).    % acts as separator within exceptions

/*
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

1.1 Naming Conventions

As the optimizer is implemented as a separated program, apart from the Secondo
kernel, it has to make some effords to keep itself informed about objects
saved within Secondo's databases. It does so by querying and inspecting
Secondo's catalog.

1.1.1 Naming Relations and Attributes

Due to some restrictions of the Prolog system, we restrict the use of
identifiers in databases when working with the optimizer:

  1 You may not use the underscore '\_' within attribute or relation
identifiers. The underscore is reserved for identifiers created by the
optimizer to encode information into the name of database objects containing
metadata.

  2 Valid identifiers always start with a letter (a-z, A-Z).

  3 You may use any spelling within object names. But you may only use
any object identifier once, regardless of spelling variants. This means,
that you may name a relation either 'Plz' or 'plz' or 'PLZ'. But you many NOT
use two relations, one named 'Plz', and another one named 'PLZ' within the
same database.

  4 You may use any spelling within attribute names. But you may only use
any attribute identifier once per relation, regardless of spelling variants.
This means, that within a relation 'Plz', you may name an attribute either 'No'
or 'NO' or 'no', but you many NOT use two attributes within 'Plz', one named
'No', and another one named 'no'. However, you MAY use 'No' or 'NO' as attribute
identifiers within any other relation (but, again, at most once per relation).

Ignoring this convention may confuse the optimizer. This is, for Prolog does
not allow certain atoms to start with upper case characters (because these are
reserved for variable identifiers). We could circumvent this restriction, but
that would make the code and output messages less readable.

1.1.2 Naming Indexes

Several information, e.g. on specialized indexes cannot be derived from the
catalog's type descriptions, so we are required to assume a naming convention to
infer the presence of (specialized) indexes for relations stored in a database.
When using the Secondo optimizer...

  1 ~Index~ identifiers need to respect following naming convention:\newline
$<$RelName$>$\_$<$KeyattrName$>$[ \_$<$LogicalIndexTypeCode$>$ [ \_$<$DisambiguationTag$>$ ] ]

$<$RelName$>$ is the name of the base relation, the index is built for.

$<$KeyattrName$>$ is the name (itentifier) of the attribute, that is used to
generate the keys stored within the index.

$<$LogicalIndexTypeCode$>$ is a alphanumerical code used to describe the logical
index type. While the base type of any index (rtee, rtree3, btree, xtree, hash
etc.) can be looked up in the system catalog, the meaning of the index is coded
using this tag. E.g. a rtree index can be described as a temporal index built on
single units or objects here. This informatioon is used within optimization
rules and required to construct small sample objects used to determine
selectivities and other parameters used during optimization.

$<$DisambiguationTag$>$ is a alphanumerical tag used to distinguish between
different indexes of the same relation and attribute. E.g. you could create
both, a xtree and an rtree3 index for indexing spatial data. You just
need to specify this tag, if you maintain more than a single index on the same
data. We recommend to use mnemonic tags coding the physical index type, e.g.
``\_r3'' for a 3d-rtree, or ``\_x'' for a xtree index, or just emumerate them
(``\_1'', ``\_2'' etc.).

Logical index types are registered in file ~database.pl~ using facts
~logicalIndexType/8~.

The syntax allows to leave out the $<$LogicalIndexTypeCode$>$.
In that case, the optimizer will assume a ``standard'' behaviour, e.g. will use
a simple 'rtree'-index, when the physical index type is 'rtree'.

By now, we have the following logical index types (and ~LogicalIndexTypeCode~s):

  * btree: (none)

  * hash: (none)

  * mtree: (none)

  * xtree: (none)

  * rtree, rtree3, rtree4, rtree8: (none)

  * temporal(rtree,object): tmpobj, temporal(rtree,unit): tmpuni

  * spatial(rtree,object): sptobj, spatial(rtree,unit): sptuni

  * spatiotemporal(rtree3,object): sptmpobj, spatiotemporal(rtree3,unit): sptmpuni

  * keyword(btree): keywdb, keyword(hash): keywdh,

  * btree(constunit): constuni

Logical index type are registered in file ~database.pl~ using facts
~logicalIndexType/8~.

By now, we have the following logical index types:

btree, rtree, rtree3, rtree4, rtree8, hash, mtree, xtree --- standard index types;
spatial(rtree,object) --- spatial index on total spatial MBR of 2D-moving objects;
spatial(rtree,unit) --- spatial index on unit-wise spatial MBRs of 2D-moving objects;
temporal(rtree,object) --- temporal index on total deftime of 2D-moving objects;
temporal(rtree,unit) --- temporal index on unit-wise deftimes of 2D-moving objects;
spatiotemporal(rtree3,object) --- spatiotemporal index on total MBR of 2D-moving objects;
spatiotemporal(rtree3,unit) --- spatiotemporal index on unit-wise MBRs of 2D-moving objects;

1.1.3 Sample Objects

The optimizer uses sampling to calculate basic costs. Samples can be created
manually or automatically. We use a specific namin schema for such sample
objects:

  1 ~relationname~\_~sample~\_j and  ~relationname~\_~sample~\_s are used
for sample relation objects.

  2 ~indexname~\_\_~small~ is used for indexes on small objects.

The ~sample~ and ~small~ objects are usually used to determine selectivities.

1.1.3 Using Null-ary Operators (with implicit arguments)

The Secondo kernel supports some prefix operators with arity 0. This kind of
operators uses some internal parameters, as system time, catalog data, etc.

In the Secondo kernel, these operators are used like this: ~opname()~. Prolog
forbids empty parameter lists. Therefor, this functions are used without
the empty round pranatheses within the optimizer. Thus, it is possible to query

---- sql select [no, now as currenttime] from ten
----

In this query ~now~ is the secondo function

---- now: --> instant
----

that is used like in

---- query ten feed extend[currenttime: now()] project[no, currenttime] consume
----

Other examples for null-ary prefix operators are ~seqnext~, ~randmax~,
~minInstant~, ~maxInstant~, ~today~, or ~rng\_int~.

1.2 Spelling of Identifiers within the Optimizer

As Prolog recognizes any atoms starting with an uppercase character as a
variable, identifiers starting with a upper case letter cannot be typed
when using the optimizer. (Also operator names may not start upper-cased.)

Instead, relations, indexes, attributes and any other database objects are
always referenced using totally down-cased versions of their original
identifiers when communicating with the optimizer. The optimizer will translate
this so-called ~DC-Spelling~ into its own ~InternalSpelling~ and convert it to
the full-spelled ~ExternalSpelling~, when it creates executable plans, which
are meant to be passed to the Secondo kernel.

Therefore, we have three kinds of spelling within the optimizer:

  * ~ExternalSpelling~: The original spelling of identifiers, as they are known within the Secondo Catalog.

  * ~InternalSpelling~: A representation used to store the ExternalSpelling within Prolog facts without confusing the Prolog system. It is ~only~ used to translate between ~DownCasedSpelling~ and ~ExternalSpelling~.

  * ~DownCasedSpelling~ or ~DC-Spelling~: The fully down-cased version of identifiers in ~ExternalSpelling~. Each upper-case character is substituted by its lower-case counterpart. This spelling is used within the optimizer, e.g. to store and query meta using facts and predicates. The user has to formulate all queries using this spelling for attributes, relations, indexes, etc.

As the optimizer only allows the use of identifiers in DC-Spelling, we need to
translate between that notation and the ~ExternalSpelling~ (the real names known to the Secondo kernel).

We use dynamic predicate storedSpell(Internal, External) to store the
translation on disk. Due to this, we must avoid identifiers starting with an
underscore or a uppercase character. As we proscribe the use of underscores
within relation and attribute identifiers, we just need to keep care of
identifiers starting with an uppercase letter.

We do so by introducing an ~InternalSpelling~ schema, which is illustrated
by the following table:

----
          External | Internal  | DCspelled
          ---------+-----------+---------
           Mouse   | mouse     |  mouse
           mouse   | lc(mouse) |  mouse
           mOUSe   | lc(mOUSe) |  mouse
           MOUSe   | mOUSe     |  mouse
----

Database object names are maintained by facts

---- storedSpell(+DB, +DCspelledObj, +InternalObj)
----

where ~DB~ is the DCspelled database name, ~DCspelledObj~ is the object name in
DownCasedSpelling, and ~InternalObj~ is the object's name in InternalSpelling
spelling.

Attribute Identifiers are stored using facts

---- storedSpell(+DB, +DCspelledRel : +DCspelledAttr, +InternalAttr)
----

~DB~ is the name of the database, which is always in DC-spelling (the Secondo
kernel does not consider spelling for database names at all).

Three predicates are used to translate between the different spelling schemas:

----
          dcName2internalName(?DC,?Intern)
          dcName2externalName(?DC,?External)
          internalName2externalName(?Intern,?Extern)
----

When using these predicates, at least one of the two arguments must be
instantiated with a ~ground term~, i.e. a term containing no unbound variable.

Otherwise, or if the translation fails due to some other circumstances, the
translation predicate will throw an exception. This is to prevent the propagation
of errors through the optimization process, which easily leads to unpredictable
behaviour of the optimizer.

So, if you get an exception from these predicates, carefully inspect the error
message. It is likely, that you just miss-typed an identifier.


1.3 Optimization Algorithm

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



1.4 Notion of Constants

In queries, string, text, int and real constants can be noted as used in Secondo, e.g.
"Hello World!" for a string constant, 'Hello Text' for a text constant, -557 or 56
for an int constant, and -1.565E-12, -1.5, 5688.45 for real constants.

~Bool constants~ must be noted ~true~ and ~false~ instead of ~TRUE~ and ~FALSE~,
because Prolog would interpret the correct symbols as variables.

~Int constants~ and ~real constants~ can be written as prolog integer constants.

~String constants~ are inclosed in double quotes (as in Secondo).

~Text constants~ can be noted in two different ways:

----
  [const, text, value, <char-list>]
  const(text,"TEXT")
----

where TEXT is a sequence of characters except the double quote,
<char-list> is the Prolog character code list (i.e. a Prolog string) representing
the text value.

All other constants need to be noted similar to the way this is done in Secondo:
as a nested list. Again, we use square brackets to delimit the list and commas to
separate its elements. Also a shorter alternative is available:

----
  [const, TYPE, value, VALUE]
  const(TYPE,VALUE)
----

where ~TYPE~ is a type descriptor (a Prolog term, like 'mpoint', 'region',
'vector(int)', or 'set(vector(real))'; and ~VALUE~ is a nested list using round
parentheses and commas to separate its elements.

This is also the only way to create undefined value constants.

Internally, ALL constants (also int, real, etc.) are noted as terms

----    value_expr(Type,Value)
----

where ~Type~ is a Prolog term for the type descriptor and ~Value~ is the nested list
representation of the constant value, both using round parantheses and commas internally.

For the standard type constants, special ~plan\_to\_atom/2~ rules exists, for
all other types, that should not be necessary, they are all handled by a generic
rule.

PROBLEMS: Using bool-atoms, string-atoms, and text-atoms within nested lists.




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
select(arg(0), pr(p, a)) or join(res(4), res(1), pr(q, a, b))

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


createPredicateFacts(Preds) :-
  optimizerOption(adaptiveJoin),
  storePredicates(Preds),
  !.

createPredicateFacts(_).


pog(Rels, Preds, Nodes, Edges) :-
  length(Rels, N), reverse(Rels, Rels2), deleteArguments,
  partition(Rels2, N, Partition0),
  length(Preds, M), reverse(Preds, Preds2),
  pog2(Partition0, M, Preds2, Nodes, Edges),
  deleteNodes, storeNodes(Nodes),
  deleteEdges, storeEdges(Edges),
  deleteVariables, deleteCounters,
  createPredicateFacts(Preds),
  deletePlanEdges, createPlanEdges,
  HighNode is 2**M -1,
  retract(highNode(_)), assert(highNode(HighNode)),
  % uncomment next line for debugging
  dc(pog, showpog(Rels, Preds)).

/*

3.2 Maintaining Counters

The dynamic fact ~nCounter~ stores the values of counters as a pair
(name, value).

*/

:- dynamic
     nCounter/2.

nextCounter(Name, Result) :-
  nCounter(Name, Value), !,
  Result is Value + 1,
  retract(nCounter(Name, Value)),
  assert(nCounter(Name, Result)).

% create a new counter if necessary
nextCounter(Name, 1) :-
  assert(nCounter(Name, 1)).


getCounter(Name, Value) :-
  nCounter(Name, Value), !.

getCounter(Name, 0) :-
  resetCounter(Name, 0).

resetCounter(Name) :-
  retractall(nCounter(Name, _)).

resetCounter(Name, Value) :-
  retractall(nCounter(Name, _)),
  assert(nCounter(Name, Value)).

deleteCounters :-
  resetCounter(nodeCtr).

showCounters :-
  findall([Name, Value], nCounter(Name, Value), List),
  showCounters(List).

showCounters([]).

showCounters([H|T]) :-
  H = [Name, Value],
  write(Name), write(' = '), write(Value), nl,
  showCounters(T).


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

:- dynamic partition/3. % to allow intOrders extension to modify it


restorePartitionClauses :-
  retractall( partition(_, _, _) :- (_) ),
  assert( partition([], _, []) :- (true) ),
  assert( partition([Rel | Rels], N, [Arp | Arps]) :-
          ( N1 is N-1,
            Arp = arp(arg(N), [Rel], []),
            assert(argument(N, Rel)),
            partition(Rels, N1, Arps)
          )
        ).

:- restorePartitionClauses.

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

:- dynamic copyPart/4. % to allow intOrders to modify it

copyPart(_, _, [], []).

restoreCopyPartClauses :-
  retractall( copyPart(_, _, _, _) :- (_) ),
  assert( copyPart(pr(P, Rel), PNo, Arps, [Arp2 | Arps2]) :-
          ( select(X, Arps, Arps2),
            X = arp(Arg, Rels, Preds),
            member(Rel, Rels), !,
            nodeNo(Arg, No),
            ResNo is No + PNo,
            Arp2 = arp(res(ResNo), Rels, [P | Preds])
          )
        ),
  assert( copyPart(pr(P, R1, R2), PNo, Arps, [Arp2 | Arps2]) :-
          ( select(X, Arps, Arps2),
            X = arp(Arg, Rels, Preds),
            member(R1, Rels),
            member(R2, Rels), !,
            nodeNo(Arg, No),
            ResNo is No + PNo,
            Arp2 = arp(res(ResNo), Rels, [P | Preds])
          )
        ),
  assert( copyPart(pr(P, R1, R2), PNo, Arps, [Arp2 | Arps2]) :-
          ( select(X, Arps, Rest),
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
            Arp2 = arp(res(ResNo), Rels, [P | Preds])
          )
        ).

:- restoreCopyPartClauses.

nodeNo(arg(_), 0).
nodeNo(res(N), N).

/*
3.6 newEdges

----    newEdges(+Pred, +PredNo, +NodesOld, -EdgesNew)
----

for each of the nodes in ~NodesOld~ return a new edge in ~EdgesNew~
built by applying the predicate ~Pred~ with number ~PNo~.

*/

newEdges(_, _, [], []).

newEdges(Pred, PNo, [Node | Nodes], [Edge | Edges]) :-
  newEdge(Pred, PNo, Node, Edge),
  newEdges(Pred, PNo, Nodes, Edges).

% newEdge(+Pred, +PredNo, +NodeOld, -EdgeNew)
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

/*
Special handling for distance-queries:
If a distancescan query is used, only joins keeping the order are
allowed, so ~sortedjoin~ is used instead of ~join~. The additional
parameters for ~sortedjoin~ are the sorted argument and the maximum
number of tuples, the other argument can contain

*/

newEdge(pr(P, R1, R2), PNo, Node, Edge) :-
  distanceRel(R3, _, _, _),
  findRels(R1, R2, Node, Source, Arg1, Arg2),
  findRel(R3, Node, Source, Arg1), !,
  Target is Source + PNo,
  nodeNo(Arg1, Arg1No),
  nodeNo(Arg2, Arg2No),
  Result is Arg1No + Arg2No + PNo,
  upperBound(R2, Node, UpperBound),
  Edge = edge(Source, Target, sortedjoin(Arg1, Arg2, pr(P, R1, R2), Arg1,
      UpperBound),
       Result, Node, PNo).

newEdge(pr(P, R1, R2), PNo, Node, Edge) :-
  distanceRel(R3, _, _, _),
  findRels(R1, R2, Node, Source, Arg1, Arg2),
  findRel(R3, Node, Source, Arg2), !,
  Target is Source + PNo,
  nodeNo(Arg1, Arg1No),
  nodeNo(Arg2, Arg2No),
  Result is Arg1No + Arg2No + PNo,
  upperBound(R1, Node, UpperBound),
  Edge = edge(Source, Target, sortedjoin(Arg1, Arg2, pr(P, R1, R2), Arg2,
      UpperBound),
       Result, Node, PNo).

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
pred(sortedjoin(_, _, P, _, _), P).

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

deleteNodes     :- retractall(node(_, _, _)).
deleteEdges     :- retractall(edge(_, _, _, _, _, _)).
deleteArguments :- retractall(argument(_, _)).


/*
4.2 Writing Nodes and Edges

----    writeNodes.
        writeEdges.
----

Write the currently stored nodes and edges, respectively.

*/
:-assert(helpLine(writeNodes,0,[],
                  'List nodes of the current POG.')).
:-assert(helpLine(writeEdges,0,[],
                  'List edges of the current POG.')).


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

----    rel(Name, Var)
----

For example

----    rel(staedte, *)
----

is a term denoting the un-renamed ~Staedte~ relation.

In a term

----    rel(plz, *)
----

the second argument ~Var~ contains an explicit variable if it has been
assigned, otherwise the symbol [*]. If an explicit variable has been used in
the query, we need to perfom renaming in the plan. For example, in the second
query above, the
relations would be denoted as

----    rel(staedte, s)
        rel(plz, p)
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
          [rel(staedte, *)],
          [pr(attr(bev, 0, u) > 500000, rel(staedte, *))]
        )

        fromwhere(
          [rel(staedte, s), rel(plz, p)],
          [pr(attr(s:sName, 1, u) = attr(p:ort, 2, u),
                rel(staedte, s), rel(plz, p)),
           pr(attr(p:pLZ, 0, u) > 40000, rel(plz, p))]
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

example3 :- pog([rel(staedte, s), rel(plz, p)],
  [pr(attr(p:ort, 2, u) = attr(s:sName, 1, u),
        rel(staedte, s), rel(plz, p) ),
   pr(attr(p:pLZ, 1, u) > 40000, rel(plz, p)),
   pr((attr(p:pLZ, 1, u) mod 5) = 0, rel(plz, p))], _, _).

/*

The two queries mentioned above are:

*/

example4 :- pog(
  [rel(staedte, *)],
  [pr(attr(bev, 1, u) > 500000, rel(staedte, *))],
  _, _).

example5 :- pog(
  [rel(staedte, s), rel(plz, p)],
  [pr(attr(s:sName, 1, u) = attr(p:ort, 2, u), rel(staedte, s),
    rel(plz, p)),
   pr(attr(p:pLZ, 1, u) > 40000, rel(plz, p))],
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

        loopjoin:       stream(Tuple1) x (Tuple1 -> stream(Tuple2)
                                -> stream(Tuple3)

                                where   Tuple3 = Tuple1 o Tuple2

        exactmatch:     btree(Tuple, AttrType) x rel(Tuple) x AttrType
                                -> stream(Tuple)

        exactmatch:     hash(Tuple, AttrType) x rel(Tuple) x AttrType
                                -> stream(Tuple)

        extend:         stream(Tuple1) x (Newname x (Tuple -> Attrtype))+
                                -> stream(Tuple2)

                                where   Tuple2 is Tuple1 to which pairs
                                        (Newname, Attrtype) have been appended

        remove:         stream(Tuple1) x Attrname+ -> stream(Tuple2)

                                where   Tuple2 is Tuple1 from which the
                                        mentioned attributes have been removed.

        project:        stream(Tuple1) x Attrname+ -> stream(Tuple2)

                                where   Tuple2 is Tuple1 projected on the
                                        mentioned attributes.

        projectextend:  stream(tuple1) x Attrname x (Newname
                          x (Tuple -> Attrtype))+  --> stream(tuple2)

        rename:         stream(Tuple1) x NewName -> stream(Tuple2)

                                where   Tuple2 is Tuple1 modified by appending
                                        "_newname" to each attribute name

        count:          stream(Tuple) -> int

                                count the number of tuples in a stream

        sortby:         stream(Tuple) x (Attrname, asc/desc)+   -> stream(Tuple)

                                sort stream lexicographically by the given
                                attribute names

        groupby:       stream(Tuple) x GroupAttrs x NewFields -> stream(Tuple2)

                                group stream by the grouping attributes; for
                                each group compute new fields each of which is
                                specified in the form Attrname : Expr. The
                                argument stream must already be sorted by the
                                grouping attributes.

        windowintersects:   rtree(Tuple) x rel(Tuple) x rect -> stream(Tuple)

                                Creates a stream of tuples containing all
                                tuples stored within the rtree and the relation,
                                whose bbox intersects the second argument.

        windowintersectsS:  rtree(Tuple) x rect -> stream(tid)

                                Creates a stream of tuple identifiers for all
                                tuples stored within the rtree, whose bbox
                                intersects the second argument.

        gettuples:          stream(tip) x rel(Tuple) -> stream(Tuple)

                                Create a stream of tuples, by reading all those
                                tuples from the relation, whose tuple identifier
                                are passed in the input stream

        sort:               stream(Tuple) -> stream(Tuple)

                                Sorts the input stream lexicographically

        rdup:               stream(Tuple) -> stream(Tuple)

                                Removes duplicates from a lexicographically
                                ordered input stream
----

In PROLOG, all expressions involving such operators are written in prefix
notation. Otherwise, the sequence of parameters of the internal plan
representation terms should be the same as in the target language.

Some predicates of the optimizer can handle stream operators automatically, if
operators working on a single stream (like filter, sort, rdup) have the stream
as their first argument. Join operators should habe the stream arguments at
positions 1 and 2.

Otherwise, you need to provide specialized rules to handle the operator!

Parameter functions are written as

----    fun([param(Var1, Type1), ..., param(VarN, TypeN)], Expr)
----


5.1.3 Converting Plans to Atoms and Writing them.

Predicate ~plan\_to\_atom~ converts a plan to a single atom, which represents
the plan as a SECONDO query in text syntax. For attributes we have to
distinguish whether a leading ``.'' needs to be written (if the attribute occurs
within a parameter function) or whether just the attribute name is needed as in
the arguments for hashjoin, for example. Predicate ~wp~ (``write plan'') uses
predicate ~plan\_to\_atom~ to convert its argument to an atom and then writes that
atom to standard output.

*/

/*
----  split_list(+InList,+N,?FirstN,?Rest)
----

Auxiliary predicate splitting a list ~InList~ at position ~N~. The first ~N~
list arguments are returned as ~FirstN~, the remainder as ~Rest~.
If the list has less than ~N~ elements, the predicate fails.

*/

split_list(InList,0,[],InList).    % case: finished all N leading elems
split_list([],N,_,_) :- N>0, fail. % case: error, not enough elems
split_list(_,N,_,_) :- N<0, fail.  % case: error, negative N
split_list([F|R],N,[F|R2],Rest) :- N2 is N-1, split_list(R,N2,R2,Rest), !.

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
It should be emphasized that the output is not a PROLOG variable but a
variable name (identifier) to be used for defining abstractions in the
Secondo system.

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

deleteVariables :- retractall(varDefined(_)).


/*
The ~plan\_to\_atom(attr(Name, Arg, Case), Result)~ predicate is not able to distinguish whether
to return ~.Name~ or ~..Name~ for ~Arg~ = 2. Now, we use a predicate ~consider\_Arg2(T1, T2)~ to return a term ~T2~, that is constructed from term ~T1~ by replacing all occurrences of ~attr(\_, 2, \_)~ in it by ~attr2(\_, 2, \_)~.

*/

consider_Arg2(Pred, Pred) :-
  atomic(Pred).

consider_Arg2(attr(Name, 2, Case), attr2(Name, 2, Case)):- !.

consider_Arg2(Term, Term) :-
  optimizerOption(subqueries),
  compound(Term),
  Term =.. [from | _].

consider_Arg2(Term, Term) :-
  optimizerOption(subqueries),
  compound(Term),
  isSubqueryPred1(Term).

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

rel_to_atom(rel(DCname, _), ExtName) :-
  dcName2externalName(DCname,ExtName).

% Section:Start:plan_to_atom_2_b
% Section:End:plan_to_atom_2_b

plan_to_atom( stconstraint(LiftedPred1, LiftedPred2, TempConstraint) , Result):-
 plan_to_atom( LiftedPred1, LP1 ),
 plan_to_atom( LiftedPred2, LP2 ),
 plan_to_atom( TempConstraint, TC ),
 concat_atom(['stconstraint(', LP1, ', ', LP2, ', ', TC, ')'],'', Result),
 !.


/*
The stpattern predicate

*/
plan_to_atom( pattern(NPredList, ConstraintList) , Result):-
 namedPredList_to_atom(NPredList, NPredList2),
 ((is_list(ConstraintList), list_to_atom(ConstraintList, ConstraintList2));
        (plan_to_atom(ConstraintList, ConstraintList2))),
 concat_atom(['. stpattern[', NPredList2, '; ',ConstraintList2,']'],'', Result),
 !.


plan_to_atom( stpattern(NPredList, ConstraintList) , Result):-
 namedPredList_to_atom(NPredList, NPredList2),
 ((is_list(ConstraintList), list_to_atom(ConstraintList, ConstraintList2));
        (plan_to_atom(ConstraintList, ConstraintList2))),
 concat_atom(['. stpattern[', NPredList2, '; ',ConstraintList2,']'],'', Result),
 !.

/*
The stpatternex predicate

*/
plan_to_atom( patternex(NPredList, ConstraintList, Filter) , Result):-
 namedPredList_to_atom(NPredList, NPredList2),
 ((is_list(ConstraintList), list_to_atom(ConstraintList, ConstraintList2));
        (plan_to_atom(ConstraintList, ConstraintList2))),
        plan_to_atom(Filter, Filter2),
 concat_atom(['. stpatternex[', NPredList2, '; ', ConstraintList2,
              '; ', Filter2, ']'],'',  Result),
 !.

/*
The iskNN faked operator

*/

plan_to_atom(isknn(TID, K, dbobject(QueryObj), Rel1, MPointAttr1, IDAttr1,
                                    RebuildIndexes), Result):- !,
  plan_to_atom(TID, ExtTID),
  atom_chars(Rel, Rel1),
  atom_chars(MPointAttr, MPointAttr1),
  atom_chars(IDAttr, IDAttr1),
  validate_isknn_input(K, dbobject(QueryObj), Rel, MPointAttr, IDAttr,
                       RebuildIndexes),

  getknnDCNames(K, QueryObj, Rel, MPointAttr, IDAttr,
    DCQueryObj, DCRel, DCMPointAttr, DCIDAttr, _, _, _, _, _),
  knearest(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr, RebuildIndexes),

  getknnExtNames(K, DCQueryObj, DCRel, DCMPointAttr, DCIDAttr,
    _, _, _, ExtIDAttr, _, _, ExtResultRel, ExtMBoolAttr, ExtBtree),

  concat_atom(['isknn(', ExtTID, ',"', ExtResultRel, '" , "', ExtBtree, '" , "',
               ExtMBoolAttr, '" , "', ExtIDAttr, '")'] , '', Result)
  .


% special rule to handle special attribute ~rowid~
plan_to_atom(rowid,' tupleid(.)' ) :- !.

plan_to_atom(A,A) :-
  string(A),
  write_list(['\nINFO: ',plan_to_atom(A,A),' found a string!\n']),
  !.

plan_to_atom(dbobject(Name),ExtName) :-
  dcName2externalName(DCname, Name),       % convert to DC-spelling
  ( dcName2externalName(DCname,ExtName)    % if Name is known
    -> true
    ; ( write_list(['\nERROR:\tCannot translate \'',dbobject(DCname),'\'.']),
        throw(error_Internal(optimizer_plan_to_atom(dbobject(DCname),
                                                  ExtName)::missingData)),
        fail
      )
  ),
  !.

plan_to_atom(Rel, Result) :-
  rel_to_atom(Rel, Name),
  atom_concat(Name, ' ', Result),
  !.

plan_to_atom(res(N), Result) :-
  atom_concat('res(', N, Res1),
  atom_concat(Res1, ')', Result),
  !.

/*
NVK NOTE: changed varname from SubqueryPred to Subquery because it is not in every case a predicate.

*/
plan_to_atom(Subquery, Result) :-
  optimizerOption(subqueries),
  subquery_plan_to_atom(Subquery, Result),
  !.
/*
NVK MODIFIED NR
Removed, don't know why this is called twice and i can't see a reason for this.
plan\_to\_atom(SubqueryPred, Result) :-
  optimizerOption(subqueries),
  subquery\_plan\_to\_atom(SubqueryPred, Result),
  !.
NVK MODIFIED NR END

*/

plan_to_atom(pr(P,_), Result) :-
   plan_to_atom(P, Result).

plan_to_atom(pr(P,_,_), Result) :-
   plan_to_atom(P, Result).

plan_to_atom([], '').

% Handle atomic value expressions (constants)
% string atom
plan_to_atom(value_expr(string,undefined), X) :-
  nullValue(string,undefined,X), !.
plan_to_atom(value_expr(string,Term), Result) :-
    catch((atom_codes(TermRes, Term), Test = ok),_,Test = failed), Test = ok,
    concat_atom(['"', TermRes, '"'], '', Result).
plan_to_atom(value_expr(string,Term),Result) :-
  concat_atom(['Invalid string constant: ',Term],ErrMsg),
  throw(error_Internal(optimizer_plan_to_atom(value_expr(string,Term),Result)
        ::malformedExpression:ErrMsg)),
  !, fail.

% text atom
plan_to_atom(value_expr(text,undefined), X) :-
  nullValue(text,undefined,X), !.
plan_to_atom(value_expr(text,Term), Result) :-
    is_list(Term),
    catch((atom_codes(TermRes, Term), Test = ok),_,Test = failed), Test = ok,
    concat_atom(['\'', TermRes, '\''], '', Result).
plan_to_atom(value_expr(text,Term), Result) :-
    atomic(Term),
    concat_atom(['\'', Term, '\''], '', Result).
plan_to_atom(value_expr(text,X),Result) :-
  concat_atom(['Invalid text constant: ',X],ErrMsg),
  throw(error_Internal(optimizer_plan_to_atom(value_expr(text,X),Result)
        ::malformedExpression:ErrMsg)),
  !, fail.

% int atom
plan_to_atom(value_expr(int,Result), Result) :-
  integer(Result).
plan_to_atom(value_expr(int,undefined), X) :-
  nullValue(int,undefined,X), !.
plan_to_atom(value_expr(int,X),Result) :-
  concat_atom(['Invalid int constant: ',X],ErrMsg),
  throw(error_Internal(optimizer_plan_to_atom(value_expr(int,X),Result)
        ::malformedExpression:ErrMsg)),
  !, fail.

% real atom
plan_to_atom(value_expr(real,Result), Result) :-
  float(Result).
plan_to_atom(value_expr(real,undefined), X) :-
  nullValue(real,undefined,X), !.
plan_to_atom(value_expr(real,X),Result) :-
  concat_atom(['Invalid real constant: ',X],ErrMsg),
  throw(error_Internal(optimizer_plan_to_atom(value_expr(real,X),Result)
        ::malformedExpression:ErrMsg)),
  !, fail.


% bool atom
plan_to_atom(value_expr(bool,X), Result) :-
  ( X = true
    -> Result = ' TRUE '
    ;  ( X = false
         -> Result = ' FALSE '
         ;  ( X = undefined
              -> nullValue(bool,undefined,Result)
              ; (concat_atom(['Invalid bool constant: ',X],ErrMsg),
                 throw(error_Internal(optimizer_plan_to_atom(value_expr(bool,X),
                    Result)::malformedExpression:ErrMsg)),
                 !, fail
                )
            )
       )
  ).

% constant value expression
plan_to_atom(value_expr(Type,Value), Result) :-
  \+ member(Type,[int,real,text,string,bool]), % special rules for these
  term_to_atom(Type,TypeA),
  nl_to_atom(Value,ValueA),
  ( nullValue(Type,Value,X)  % registered value (null, empty, error, default)
    -> Result = X
    ; concat_atom(['[const',TypeA,'value',ValueA,']'],' ',Result)
  ).

% type expression
plan_to_atom(type_expr(Term), Result) :-
  term_to_atom(Term, Result).

/*
Handle Implicit arguments in parameter functions:

*/

plan_to_atom(implicitArg(1), Result) :-
  atom_concat('.', ' ', Result), !.

plan_to_atom(implicitArg(2), Result) :-
  atom_concat('..', ' ', Result), !.


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
Operators: You only need to specify translation rules explicitly

  * for operators with a non-standard syntax (e.g. using a semicolon within a parameter list),

  * for operators that perform translations to more complex terms, or nave another name than is used by the kernel,

  * if you want to respect some optimizerOptions,

  * if you use implicit argumets,

  * if you need to change argument order.


For general/standart syntax rules, see below.
Non-standard syntax can be declared in file ~opsyntax.pl~ using facts ~secondoOp/3~.

*/

/*

The term

---- optimizerAnnotation(+Plan, +Note)
----

can be used, to annotate a plan ~Plan~ with internal information ~Note~, during
plan creation and/or cost estimation.

*/

plan_to_atom(optimizerAnnotation(X,_), Result) :-
  plan_to_atom(X, Result),
  !.

plan_to_atom(sample(Rel, S, T), Result) :-
  plan_to_atom(Rel, ResRel),
  concat_atom([ResRel, ' sample[', S, ', ', T, '] '], '', Result),
  !.

plan_to_atom(sample(Rel, S, T, Seed), Result) :-
  plan_to_atom(Rel, ResRel),
  concat_atom([ResRel, ' sample[', S, ', ', T, ', ', Seed, '] '], '', Result),
  !.

plan_to_atom(symmjoin(X, Y, M), Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  consider_Arg2(M, M2),          % transform second arg/3 to arg2/3
  plan_to_atom(M2, MAtom),
  concat_atom([XAtom, ' ', YAtom, ' symmjoin[',
    MAtom, ']'], '', Result),
  !.

plan_to_atom(symmproductextend(X, Y, Fields), Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  consider_Arg2(Fields, Fields2),          % transform second arg/3 to arg2/3
  plan_to_atom(Fields2, FieldsAtom),
  concat_atom([XAtom, ' ', YAtom, ' symmproductextend[',
    FieldsAtom, ']'], '', Result),
  !.

% two pseudo-operators used by the 'interesting orders extension':
plan_to_atom(sortLeftThenMergejoin(X, Y, A, B), Result) :-
  % first glue-operator sort on left tuple, then mergejoin
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  plan_to_atom(A, AAtom),
  plan_to_atom(B, BAtom),
  concat_atom([XAtom, ' sortby[', AAtom, ' asc] ',
               YAtom, ' mergejoin[', AAtom, ', ', BAtom, ']'], '', Result),
  !.

plan_to_atom(sortRightThenMergejoin(X, Y, A, B), Result) :-
  % first glue-operator sort on right tuple, then mergejoin
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  plan_to_atom(A, AAtom),
  plan_to_atom(B, BAtom),
  concat_atom([XAtom, ' ', YAtom, ' sortby[', BAtom, ' asc] ',
               'mergejoin[', AAtom, ', ', BAtom, ']'], '', Result),
  !.


plan_to_atom(sortmergejoin(X, Y, A, B), Result) :-
  optimizerOption(useRandomSMJ), % maps sortmergejoin to sortmergejoin_r2
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  plan_to_atom(A, AAtom),
  plan_to_atom(B, BAtom),
  concat_atom([XAtom, ' ', YAtom, ' sortmergejoin_r2[',
               AAtom, ', ', BAtom, ']'], '', Result),
  !.


plan_to_atom(sortmergejoin(X, Y, A, B), Result) :-
  optimizerOption(useRandomSMJ3),
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  plan_to_atom(A, AAtom),
  plan_to_atom(B, BAtom),
  concat_atom([XAtom, ' ', YAtom, ' sortmergejoin_r3[',
               AAtom, ', ', BAtom, ']'], '', Result),
  !.

plan_to_atom(pjoin1(X, Y, Ctr, Fields), Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Y, YAtom),
  plan_to_atom(Ctr, CtrAtom),
  plan_to_atom(Fields, FAtom),
  concat_atom([XAtom, ' ',YAtom,
               ' pjoin1[', CtrAtom, '; ', FAtom, ']'], '', Result),
  !.

plan_to_atom(groupby(Stream, GroupAttrs, Fields), Result) :-
  plan_to_atom(Stream, SAtom),
  plan_to_atom(GroupAttrs, GAtom),
  plan_to_atom(Fields, FAtom),
  concat_atom([SAtom, ' groupby[', GAtom, '; ', FAtom, ']'], '', Result),
  !.

plan_to_atom(field(NewAttr, Expr), Result) :-
  plan_to_atom(attrname(NewAttr), NAtom),
  plan_to_atom(Expr, EAtom),
  concat_atom([NAtom, ': ', EAtom], '', Result),
  !.


/*
Using the switch ``removeHiddenAttributes'', ~reduce~ can either be left in the plan or removed from it.

*/
plan_to_atom(reduce(Stream, Pred, Factor), Result) :-
  not(removeHiddenAttributes), !,  % is used in plan
  plan_to_atom(Stream, StreamAtom),
  plan_to_atom(Pred, PredAtom),
  plan_to_atom(Factor, FactorAtom),
  concat_atom([StreamAtom, ' reduce[', PredAtom, ', ', FactorAtom, ']'], '',
    Result),
  !.

plan_to_atom(reduce(Stream, _, _), StreamAtom) :-
  removeHiddenAttributes,   % is removed from plan
  plan_to_atom(Stream, StreamAtom),
  !.


/*
Attributes can be designated as hidden by setting their argument number to 100,
e.g. in a term ~attrname(attr(xxxIDplz, 100, l))~. If the flag ~removeHiddenAttributes~ is set,
they are removed from a projection list. Currently used for the entropy optimizer.

*/

:- dynamic(removeHiddenAttributes/0).

plan_to_atom(project(Stream, Fields), Result) :-
  not(removeHiddenAttributes), !,  % standard behaviour
  plan_to_atom(Stream, SAtom),
  plan_to_atom(Fields, FAtom),
  concat_atom([SAtom, ' project[', FAtom, ']'], '', Result),
  !.

plan_to_atom(project(Stream, Fields), Result) :-
  removeHiddenAttributes,
  plan_to_atom(Stream, SAtom),
  removeHidden(Fields, Fields2), % defined after plan_to_atom
  plan_to_atom(Fields2, FAtom),
  concat_atom([SAtom, ' project[', FAtom, ']'], '', Result),
  !.

plan_to_atom(feedproject(Stream, Fields), Result) :-
  not(removeHiddenAttributes), !,  % standard behaviour
  plan_to_atom(Stream, SAtom),
  plan_to_atom(Fields, FAtom),
  concat_atom([SAtom, ' feedproject[', FAtom, ']'], '', Result),
  !.

plan_to_atom(feedproject(Stream, Fields), Result) :-
  removeHiddenAttributes,
  plan_to_atom(Stream, SAtom),
  removeHidden(Fields, Fields2), % defined after plan_to_atom
  plan_to_atom(Fields2, FAtom),
  concat_atom([SAtom, ' feedproject[', FAtom, ']'], '', Result),
  !.


% Ignore sortby with empty sort list
plan_to_atom(sortby(Stream, []), Result) :-
  plan_to_atom(Stream, Result),
  !.


plan_to_atom(exactmatchfun(Index, Rel, attr(Name, R, Case)), Result) :-
  plan_to_atom(Rel, RelAtom),
  plan_to_atom(a(Name, R, Case), AttrAtom),
  plan_to_atom(dbobject(Index),IndexAtom),
  newVariable(T),
  concat_atom(['fun(', T, ' : TUPLE) ', IndexAtom,
    ' ', RelAtom, ' exactmatch[attr(', T, ', ', AttrAtom, ')]'], Result),
  !.


plan_to_atom(newattr(Attr, Expr), Result) :-
  plan_to_atom(Attr, AttrAtom),
  plan_to_atom(Expr, ExprAtom),
  concat_atom([AttrAtom, ': ', ExprAtom], '', Result),
  !.


plan_to_atom(rename(X, Y), Result) :-
  plan_to_atom(X, XAtom),
  concat_atom([XAtom, '{', Y, '}'], '', Result),
  !.

% NVK ADDED MA
plan_to_atom(memory(X, MID, _), Result) :-
  plan_to_atom(X, XAtom),
  ensure(memoryValue(MID, MiB, _)), % If not found, there is something wrong.
  atomic_list_concat([XAtom, '{memory ', MiB, '}'], '', Result),
  !.
% NVK ADDED MA END

/*
NVK ADDED NR
unnest is handled by the postfixbrackets method. But nest has a special syntax that is handled here.

*/
plan_to_atom(nest(X, Atts, NewAttLabel), Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Atts, AttsAtom),
  plan_to_atom(NewAttLabel, NewAttLabelAtom),
  atomic_list_concat([XAtom, ' nest[', AttsAtom, '; ', NewAttLabelAtom, ']'],
    '', Result),
  !.

/*
Could be handeld by the postfixbrackets case as well.

*/
plan_to_atom(unnest(X, Att), Result) :-
  plan_to_atom(X, XAtom),
  plan_to_atom(Att, AttAtom),
  atomic_list_concat([XAtom, ' unnest[', AttAtom, ']'], '', Result),
  !.
% NVK ADDED NR END


plan_to_atom(predinfo(X, Sel, Cost), Result) :-
  plan_to_atom(X, XAtom),
  concat_atom([XAtom, '{', Sel, ', ', Cost, '}'], '', Result),
  !.


plan_to_atom(fun(Params, Expr), Result) :-
  params_to_atom(Params, ParamAtom),
  plan_to_atom(Expr, ExprAtom),
  concat_atom(['fun(', ParamAtom, ') ', ExprAtom], '', Result),
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

plan_to_atom(projectextend(Stream,ProjectFields,ExtendFields), Result) :-
  plan_to_atom(Stream,SAtom),
  plan_to_atom(ProjectFields,PAtom),
  plan_to_atom(ExtendFields,EAtom),
  concat_atom([SAtom,' projectextend[',PAtom,' ; ',EAtom,']'], '', Result),
  !.

/*
Sort orders and attribute names.

*/

plan_to_atom(asc(Attr), Result) :-
  plan_to_atom(Attr, AttrAtom),
  atom_concat(AttrAtom, ' asc', Result),
  !.

plan_to_atom(desc(Attr), Result) :-
  plan_to_atom(Attr, AttrAtom),
  atom_concat(AttrAtom, ' desc', Result),
  !.

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
  plan_to_atom(a(Name, Arg, Case), Result),
  !.

/*
NVK ADDED NR
The idea behind this is to allow terms like 

----
attr(o:p:bevt, _, _) 
----

to reflect the renaming process when nested relations are used.

Example:

----
plan_to_atom(a(p:o:subrel, 0, u), R).
R = Subrel_o_p.
----

Note that the case is only related to the last element (in the example this is subrel).

*/

/*
----
plan_to_atom(a(A:B:C, _, l), Result) :-
  optimizerOption(nestedRelations),
  attributeTermToList(A:B:C, LST),
  reverse(LST, RLST),
  atomic_list_concat(RLST, '_', Result),
  !.
----

*/

plan_to_atom(a(A:B:C, _, _), Result) :-
  optimizerOption(nestedRelations),
  attributeTermToList(A:B:C, LST),
  reverse(LST, [F|RList]),
  upper(F, FUpper),
  atomic_list_concat([FUpper|RList], '_', Result),
  !.
% NVK ADDED NR END

% plan_to_atom(a(A:B, _, l), Result) :-
%   concat_atom([B, '_', A], '', Result),
%   !.

plan_to_atom(a(A:B, _, _), Result) :-
  upper(B, B2),
  concat_atom([B2, '_', A], Result),
  !.

% plan_to_atom(a(X, _, l), X) :-
%   !.

plan_to_atom(a(X, _, _), X2) :-
  upper(X, X2),
  !.


plan_to_atom(true, Result) :-
  concat_atom(['TRUE'], '', Result),
  !.

plan_to_atom(false, Result) :-
  concat_atom(['FALSE'], '', Result),
  !.

% rule to handle null-ary operators like now(), today(), seqnext()
plan_to_atom(Op, Result) :-
  atom(Op),
  secondoOp(Op, prefix, 0),
  systemIdentifier(Op, _), !,
  concat_atom([Op, '() '], '', Result),
  !.

/*
Integrating counters into query plans

*/

plan_to_atom(counter(Term,N), Result) :-
  plan_to_atom( Term, TermRes ),
  concat_atom( [ TermRes, ' {', N,'} '], Result ),
  !.


/*
User defined Aggregation operators

*/

plan_to_atom(aggregate(Term, AttrName, AggrFunction, DefaultVal), Result) :-
  plan_to_atom( Term, TermRes ),
  plan_to_atom( AttrName, AttrNameRes ),
  plan_to_atom( AggrFunction, AggrFunRes ),
  plan_to_atom( DefaultVal, DefaultValRes ),
  concat_atom( [ TermRes, ' aggregateB[', AttrNameRes, ' ; ', AggrFunRes,
                 ' ; ', DefaultValRes, ']' ], Result ),
  !.

/*
Simple, predefined aggregation functions, when used without a groupby

*/
plan_to_atom(simpleAggrNoGroupby(AggrOp, Stream, Expr), Result) :-
  isAggregationOP(AggrOp),
  Expr \= attr(_, _, _),
  newVariable(ExprAttrName),
  ExprAttr = attr(ExprAttrName, *, l),
  Term1 =.. [AggrOp,extend(Stream,[field(ExprAttr, Expr)]),attrname(ExprAttr)],
  plan_to_atom(Term1, Result),
  !.

plan_to_atom(simpleAggrNoGroupby(AggrOp, Stream, Expr), Result) :-
  isAggregationOP(AggrOp),
  Expr = attr(_, _, _),
  Term1 =.. [AggrOp, Stream, attrname(Expr)],
  plan_to_atom(Term1, Result),
  !.


plan_to_atom(simpleUserAggrNoGroupby(Stream, Expr, Fun, Default),
  Result) :-
  Expr \= attr(_, _, _),
  newVariable(ExprAttrName),
  ExprAttr = attr(ExprAttrName, *, l),
  Stream1 = extend(Stream, [field(ExprAttr, Expr)]),
  plan_to_atom(aggregate(Stream1, attrname(ExprAttr), Fun, Default), Result),
  !.

plan_to_atom(simpleUserAggrNoGroupby(Stream, Expr, Fun, Default),
  Result) :-
  Expr = attr(_, _, _),
  plan_to_atom(aggregate(Stream, attrname(Expr), Fun, Default), Result),
  !.

/*
Extensions for SQL ~update~ and ~insert~ commands

*/
plan_to_atom(inserttuple(Rel, Values), Result) :-
  plan_to_atom(Rel, Rel2),
  list_to_atom(Values, Values2),
  concat_atom([Rel2, ' inserttuple[', Values2, ']'], Result),
  !.

plan_to_atom(insert(Rel, InsertQuery),Result) :-
  plan_to_atom(InsertQuery, InsertQuery2),
  plan_to_atom(Rel, Rel2),
  concat_atom([InsertQuery2, ' ', Rel2, ' insert'], Result),
  !.

plan_to_atom(deletedirect(Rel, DeleteQuery),Result) :-
  plan_to_atom(DeleteQuery, DeleteQuery2),
  plan_to_atom(Rel, Rel2),
  concat_atom([DeleteQuery2, ' ', Rel2, ' deletedirect'], Result),
  !.

plan_to_atom(updatedirect(Rel, Transformations, UpdateQuery),Result) :-
  plan_to_atom(UpdateQuery, UpdateQuery2),
  list_to_atom(Transformations, Transformations2),
  plan_to_atom(Rel, Rel2),
  concat_atom([UpdateQuery2, ' ', Rel2, ' updatedirect[',
        Transformations2, ']'], Result),
  !.

plan_to_atom(insertbtree(InsertQuery, IndexName, Column),Result) :-
  plan_to_atom(InsertQuery, InsertQuery2),
  plan_to_atom(attrname(Column), Column2),
  concat_atom([InsertQuery2, ' ', IndexName, ' insertbtree[', Column2, ']'],
       Result),
  !.

plan_to_atom(deletebtree(DeleteQuery, IndexName, Column),Result) :-
  plan_to_atom(DeleteQuery, DeleteQuery2),
  plan_to_atom(attrname(Column), Column2),
  concat_atom([DeleteQuery2, ' ', IndexName, ' deletebtree[', Column2, ']'],
       Result),
  !.

plan_to_atom(updatebtree(UpdateQuery, IndexName, Column),Result) :-
  plan_to_atom(UpdateQuery, UpdateQuery2),
  plan_to_atom(attrname(Column), Column2),
  concat_atom([UpdateQuery2, ' ', IndexName, ' updatebtree[', Column2, ']'],
       Result),
  !.

plan_to_atom(insertrtree(InsertQuery, IndexName, Column),Result) :-
  plan_to_atom(InsertQuery, InsertQuery2),
  plan_to_atom(attrname(Column), Column2),
  concat_atom([InsertQuery2, ' ', IndexName, ' insertrtree[', Column2, ']'],
       Result),
  !.

plan_to_atom(deletertree(DeleteQuery, IndexName, Column),Result) :-
  plan_to_atom(DeleteQuery, DeleteQuery2),
  plan_to_atom(attrname(Column), Column2),
  concat_atom([DeleteQuery2, ' ', IndexName, ' deletertree[', Column2, ']'],
       Result),
  !.

plan_to_atom(updatertree(UpdateQuery, IndexName, Column),Result) :-
  plan_to_atom(UpdateQuery, UpdateQuery2),
  plan_to_atom(attrname(Column), Column2),
  concat_atom([UpdateQuery2, ' ', IndexName, ' updatertree[', Column2, ']'],
       Result),
  !.

plan_to_atom(inserthash(InsertQuery, IndexName, Column),Result) :-
  plan_to_atom(InsertQuery, InsertQuery2),
  plan_to_atom(attrname(Column), Column2),
  concat_atom([InsertQuery2, ' ', IndexName, ' inserthash[', Column2, ']'],
       Result),
  !.

plan_to_atom(deletehash(DeleteQuery, IndexName, Column),Result) :-
  plan_to_atom(DeleteQuery, DeleteQuery2),
  plan_to_atom(attrname(Column), Column2),
  concat_atom([DeleteQuery2, ' ', IndexName, ' deletehash[', Column2, ']'],
       Result),
  !.

plan_to_atom(updatehash(UpdateQuery, IndexName, Column),Result) :-
  plan_to_atom(UpdateQuery, UpdateQuery2),
  plan_to_atom(attrname(Column), Column2),
  concat_atom([UpdateQuery2, ' ', IndexName, ' updatehash[', Column2, ']'],
       Result),
  !.

/*
Extensions for SQL ~create~ and ~drop~ commands

*/

plan_to_atom(let(VarName, Type),Result) :-
  plan_to_atom(a(VarName, *, u), VarName2),
  plan_to_atom(Type, Type2),
  concat_atom(['let ', VarName2, ' = [const ', Type2, ' value ()]'],
       Result),
  !.

plan_to_atom(delete(Identifier),Result) :-
  atomic(Identifier), !,
  concat_atom(['delete ', Identifier], Result),
  !.

plan_to_atom(delete(Identifier),Result) :-
  plan_to_atom(Identifier, Identifier2),
  concat_atom(['delete ', Identifier2], Result),
  !.

plan_to_atom(deleteIndex(Identifier, QueryRest),[Result|QueryRest2]) :-
  atomic(Identifier), !,
  plan_to_atom(QueryRest, QueryRest2),
  concat_atom(['delete ', Identifier], Result),
  !.

plan_to_atom(deleteIndex(Identifier, QueryRest),[Result|QueryRest2]) :-
  plan_to_atom(QueryRest, QueryRest2),
  plan_to_atom(Identifier, Identifier2),
  concat_atom(['delete ', Identifier2], Result),
  !.

plan_to_atom(rel(Columns),Result) :-
  list_to_atom(Columns, Columns2),
  concat_atom(['rel(tuple([', Columns2, ']))'], Result),
  !.

% NVK ADDED NR
plan_to_atom(column(Name, Type),Result) :-
	Type=..[arel, Cols],
  list_to_atom(Cols, TypeT),
  plan_to_atom(a(Name, *, u), Name2),
  concat_atom([Name2, ': arel(tuple([', TypeT, ']))'], Result),
  !.
% NVK ADDED NR END

plan_to_atom(column(Name, Type),Result) :-
  plan_to_atom(a(Name, *, u), Name2),
  concat_atom([Name2, ': ', Type], Result),
  !.

plan_to_atom(createIndex(Rel, Columns, LogIndexType), Result) :-
  rel_to_atom(Rel, Rel2),
  plan_to_atom(attrname(Columns), Columns2),
  getCreateIndexExpression(Rel2, Columns2, LogIndexType, no, IndexName,
      Query),
  concat_atom(['derive ', IndexName, ' = ', Query], Result), !.

/*
Extensions for distance-queries

*/

plan_to_atom(ksmallest(Stream, Count, Attr), Result) :-
  plan_to_atom(Stream, Stream2),
  plan_to_atom(Attr, Attr2),
  concat_atom([Stream2, ' ksmallest[', Count, '; ', Attr2, ']'], Result), !.

plan_to_atom(distancescan(DCIndex, Rel, X, HeadCount), Result) :-
  plan_to_atom(X, X2),
  rel_to_atom(Rel, Rel2),
  concat_atom([DCIndex, ' ', Rel2, ' distancescan[', X2, ', ', HeadCount,']' ],
       Result), !.

plan_to_atom(varfuncquery(Stream, Var, Expr), Result) :-
  plan_to_atom(Stream, Stream2),
  plan_to_atom(Expr, Expr2),
  concat_atom([Expr2, ' within[fun(', Var, ':ANY) ', Stream2, ']'],
       Result).

plan_to_atom(createtmpbtree(Rel, Attr), Result) :-
  rel_to_atom(Rel, Rel2),
  plan_to_atom(attrname(Attr), Attr2),
  concat_atom([Rel2, ' createbtree[', Attr2, ']'], Result).

% Section:Start:plan_to_atom_2_m
% Section:End:plan_to_atom_2_m

/*
Translation of operators driven by predicate ~secondoOp~ in
file ~opsyntax~. There are rules for

  * infix: exactly 2 arguments separated by the functor

  * prefix: the functor  followed by at least 0 arguments in round parantheses

  * postfix: at least 1 argument followed by the functor

  * postfixbrackets: at least N argument before the functor, plus an arbitrary
    number of comma-separated arguments in squared bracket following the functor

*/


/*
Generic translation for postfix operators with at least n>=1 arguments:
To use this rule for an operator ~op~, insert a fact
~secondoOp(op, postfix, N)~ into file ~opsyntax.pl~

----
  _ #
  _ ... _ #

----

*/
plan_to_atom(InTerm,OutTerm) :-
  compound(InTerm),
  InTerm =.. [Op|ArgsIn],
  secondoOp(Op, postfix, N),
  N>=1,
  length(ArgsIn,NoArgs),
  NoArgs >= N,
  plan_to_atom_2(ArgsIn,ArgsOut),
  concat_atom(ArgsOut, ' ', ArgsOutAtom),
  concat_atom([ArgsOutAtom, Op], ' ', OutTerm), !.

/*
Generic translation for prefix operators with at least n>=0 arguments:
To use this rule for an operator ~op~, insert a fact
~secondoOp(op, prefix, N)~ into file ~opsyntax.pl~

----
  #( )
  #( _ )
  #( _, ..., _ )

----

*/
% prefix with 0 arguments must be defined above. We repeat the code here for
% explenatory reasons only:
% plan_to_atom(Op, Result) :-
%   atom(Op),
%   secondoOp(Op, prefix, 0),
%   systemIdentifier(Op, _), !,
%   concat_atom([Op, '() '], '', Result),
%   !.

plan_to_atom(InTerm,OutTerm) :-
  compound(InTerm),
  InTerm =.. [Op|ArgsIn],
  secondoOp(Op, prefix, N),
  N >= 0,
  length(ArgsIn,NoArgs),
  NoArgs >= N,
  plan_to_atom_2(ArgsIn,ArgsOut),
  concat_atom(ArgsOut, ', ', ArgsOutAtom),
  concat_atom([Op, '(', ArgsOutAtom, ')'], '', OutTerm), !.

/*
Generic translation for (binary) infix operators
To use this rule for an operator ~op~, insert a fact
~secondoOp(op, infix, 2)~ into file ~opsyntax.pl~

----
  _ # _

----

*/
plan_to_atom(InTerm, Result) :-
  compound(InTerm),
  InTerm =.. [Op,Arg1,Arg2],
  secondoOp(Op, infix, 2),
  plan_to_atom(Arg1, Res1),
  plan_to_atom(Arg2, Res2),
  concat_atom(['(', Res1, ' ', Op, ' ', Res2, ')'], '', Result), !.

/*
Generic translation for postfixbrackets operators with n arguments followed
by the functor and arbitrary number of arguments following it in square brackets:
To use this rule for an operator ~op~, insert a fact
~secondoOp(op, postfixbrackets, N)~ into file ~opsyntax.pl~

----

  _ #[ _ ]
  _ #[ _, ..., _ ]
  _ ... _ #[ _ ]
  _ ... _ #[ _, ..., _ ]

----

*/
plan_to_atom(InTerm,OutTerm) :-
  compound(InTerm),
  InTerm =.. [Op|ArgsIn],
  secondoOp(Op, postfixbrackets, N),
  N >= 0,
  length(ArgsIn,NoArgs),
  NoArgs >= N,
  split_list(ArgsIn,N,ArgsBefore,ArgsAfter),
  plan_to_atom_2(ArgsBefore,ArgsBeforeOut),
  concat_atom(ArgsBeforeOut, ' ', ArgsBeforeOutAtom),
  plan_to_atom_2(ArgsAfter,ArgsAfterOut),
  concat_atom(ArgsAfterOut, ', ', ArgsAfterOutAtom),
  concat_atom([ArgsBeforeOutAtom,' ',Op,'[', ArgsAfterOutAtom, ']'],'',OutTerm),
  !.

/*
Error handler for operator with 'special' syntax, that do not have a matching
~plan\_to\_atom~ rule:

To safeguard an operator ~op~ with special translation agains using standarad
translation., add a fact secondoOp(op, special, N) to file opsyntax.pl.

*/
plan_to_atom(InTerm,OutTerm) :-
  compound(InTerm),
  InTerm =.. [Op|_],
  secondoOp(Op, Syntax, N),
  term_to_atom(Syntax,SyntaxA),
  term_to_atom(N,NA), !,
  concat_atom(['ERROR: special plan_to_atom/2 rule for operator \'', Op,
               '\'is missing.\n',
               '\tDefined Syntax is: secondoOp(',Op,',',SyntaxA,',',NA,')\n',
               '\tPlease provide an according rule!\.'],'',ErrMsg),
  write(ErrMsg), nl,
  throw(error_Internal(optimizer_plan_to_atom(InTerm,OutTerm)
        ::malformedExpression:ErrMsg)),
  !, fail.


/*
Depricated generic rules. Operators that are not recognized are assumed to be:

  * 1 argument: prefix

  * 2 arguments: infix

  * 3+ arguments: prefix

*/

/* 1 argument: prefix */
plan_to_atom(Term, Result) :-
  functor(Term, Op, 1),
  \+(secondoOp(Op, _, _)), !,
  arg(1, Term, Arg1),
  plan_to_atom(Arg1, Res1),
  concat_atom([Op, '(', Res1, ')'], '', Result), !,
  write_list(
    ['WARNING: Applied deprecated default plan_to_atom/2 rule for unary',
     'prefix operator ',Op, '/1. Please add the following fact to file ',
     '\'opsyntax.pl\':\n','\tsecondoOp( ',Op,', prefix, 1).\n']), !.

/* 2 arguments: infix */
plan_to_atom(Term, Result) :-
  functor(Term, Op, 2),
  \+(secondoOp(Op, _, _)), !,
  arg(1, Term, Arg1),
  arg(2, Term, Arg2),
  plan_to_atom(Arg1, Res1),
  plan_to_atom(Arg2, Res2),
  concat_atom(['(', Res1, ' ', Op, ' ', Res2, ')'], '', Result), !,
  write_list(['WARNING: Applied deprecated default plan_to_atom/2 rule for ',
    'infix operator ', Op, '/2. Please add the following fact to file ',
    '\'opsyntax.pl\':\n','\tsecondoOp( ',Op,', infix, 2).\n']), !.

/* 3+ arguments: prefix */
plan_to_atom(InTerm,OutTerm) :-
  compound(InTerm),
  \+(secondoOp(Op, _, _)), !,
  InTerm =.. [Op|ArgsIn],
  length(ArgsIn,N),
  plan_to_atom_2(ArgsIn,ArgsOut),
  concat_atom(ArgsOut, ', ', ArgsOutAtom),
  concat_atom([Op, '(', ArgsOutAtom, ')'], '', OutTerm), !,
  write_list(['WARNING: Applied deprecated default plan_to_atom/2 rule for ',
              N,'-ary prefix operator ', Op, '/',N,
              '. Please add the following fact to file ',
              '\'opsyntax.pl\':\n','\tsecondoOp( ',Op,', infix, ',N,').\n']), !.

/* Standard translation of atomic terms */
plan_to_atom(X, Result) :-
  atomic(X),
  term_to_atom(X, Result),
  !.

% Section:Start:plan_to_atom_2_e
% Section:End:plan_to_atom_2_e

/* Error case */
plan_to_atom(X, _) :-
  term_to_atom(X,XA),
  concat_atom(['Error in plan_to_atom: No rule for handling term ',XA],
               '',ErrMsg),
  write(ErrMsg), nl,
  throw(error_Internal(optimizer_plan_to_atom(X,undefined)
        ::malformedExpression:ErrMsg)),
  !, fail.

/* auxiliary predicates */
plan_to_atom_2([],[]).

plan_to_atom_2([InHead|InRest],[OutHead|OutRest]) :-
  plan_to_atom(InHead,OutHead), !,
  plan_to_atom_2(InRest,OutRest).


% used within insert and update:
% % process constants using nested list value
list_to_atom([const, T, value, V], Atom) :- 
  plan_to_atom(value_expr(T,V),Atom),
  !.

% used within insert and update:
list_to_atom([X], AtomX) :-
  listelement_to_atom(X, AtomX),
  !.

% used within insert and update:
list_to_atom([X | Xs], Result) :-
  listelement_to_atom(X, XAtom),
  list_to_atom(Xs, XsAtom),
  concat_atom([XAtom, ', ', XsAtom], '', Result),
  !.

% used within insert and update:
% % process attr:value 
listelement_to_atom(set(Attr, Term), Result) :-
  plan_to_atom(attrname(Attr), Attr2),
  listelement_to_atom(Term, Term2),
  concat_atom([Attr2, ': ', Term2], Result),
  !.

% used within insert and update:
% % process constants using nested list value
listelement_to_atom([const, T, value, V], Result) :-
  plan_to_atom(value_expr(T,V),Result),
  !.

% used within insert and update:
% % process string atom
listelement_to_atom(Term, Result) :-
    is_list(Term), Term = [First | _], atomic(First), !,
    atom_codes(TermRes, Term),
    concat_atom(['"', TermRes, '"'], '', Result).

% used within insert and update:
% % process other kinds of elements
listelement_to_atom(Term, Result) :-
    plan_to_atom(Term, Result).

% used to translate into proper nested lists

% a nested list
nl_to_atom(L,A) :-
  is_list(L),
  findall(MemberA, ( member(Member,L), 
                     ( is_list(Member) 
                       -> nl_to_atom(Member,MemberA)
                       ;  plan_to_atom(Member,MemberA)
                     )
                   ),
          AList), !,
  concat_atom(AList, ' ', A1),!,
  concat_atom(['(',A1,')'],' ', A),
  !.

%  nested list atom
nl_to_atom(T,A) :-
 T \= [],
 plan_to_atom(T,A), !.



/*
Hidden fields have an argument number 100 and can be removed from a projection list by activating the flag ``removeHidenAttributes''. See ~plan\_to\_atom~ for ~project~.

*/

removeHidden([], []).

removeHidden([attrname(attr(_, 100, _)) | Fields], Fields2) :-
  removeHidden(Fields, Fields2),
  !.

removeHidden([Field | Fields], [Field | Fields2]) :-
  removeHidden(Fields, Fields2).



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

type_to_atom(tuple, 'TUPLE')   :- !.
type_to_atom(tuple2, 'TUPLE2') :- !.
type_to_atom(group, 'GROUP')   :- !.

% Section:Start:type_to_atom_2_m
% Section:End:type_to_atom_2_m

% Needed for aggregate
type_to_atom(X, Y) :-
  concat_atom([X], Y),
  !.
  

/*

5.2 Optimization Rules

We introduce a predicate [=>] which can be read as ``translates into''.

5.2.1 Translation of the Arguments of an Edge of the POG

If the argument is of the form res(N), then it is a stream already and can be
used unchanged. If it is of the form arg(N), then it is a base relation; a
~feed~ must be applied and possibly a ~rename~.

For ~res(N)~ and ~arg(N)~, there should be only one possible translation, so
respect the correct ordering of clauses and use cuts to enforce the uniqueness
of translation results.

*/

% Section:Start:translationRule_2_b
% Section:End:translationRule_2_b


res(N) => res(N).

% special handling for distancescan queries
arg(N) => distancescan(IndexName, rel(Name, *), Attr, 0) :-
  isStarQuery,
  argument(N, rel(Name, *)),
  distanceRel(rel(Name, *), IndexName, Attr, _), !.

% special handling for distancescan queries
arg(N) => rename(distancescan(IndexName, rel(Name, Var), Attr, 0), Var) :-
  isStarQuery,
  argument(N, rel(Name, Var)),
  distanceRel(rel(Name, Var), IndexName, Attr, _), !.

% special handling for distancescan queries
arg(N) => project(distancescan(IndexName, rel(Name, *), Attr, 0), AttrNames) :-
  argument(N, rel(Name, *)),
  distanceRel(rel(Name, *), IndexName, Attr, _), !,
  usedAttrList(rel(Name, *), AttrNames).

% special handling for distancescan queries
arg(N) => rename(project(distancescan(IndexName, rel(Name, Var), Attr, 0),
    AttrNames), Var) :-
  argument(N, rel(Name, Var)),
  distanceRel(rel(Name, Var), IndexName, Attr, _), !,
  usedAttrList(rel(Name, Var), AttrNames).
/*
NVK ADDED NR
*/

/*
There is even no afeedproject operator as for arel's (see below). Note that these rules are used for optimization of subquries, there are usually subqueries create like 0 COMPARE\_OP (select count(*) ...) and count(*) is NOT a isStarQuery, so the above rules are not used.

*/

/*
This translation rule handles all the cases:

  * arel access

  * subquery access

  * post added nest or unnest.

*/
arg(N) => Stream4 :-
  argument(N, RelT),
	RelT=rel(irrel(_, Stream1, TOP, _, _, _, _), Var),
  addTransformationOperator(Stream1, TOP, Stream2),
  (isStarQuery ->
		Stream3=Stream2
	;
		(
  		usedAttrList(RelT, AttrNames),
			Stream3=project(Stream2, AttrNames)
		)
	),
	addRenameOperator(Stream3, Var, Stream4).

arg(N) => feed(rel(Name, *)) :-
  optimizerOption(nestedRelations), 
  isStarQuery,
  argument(N, rel(Name, *)), 
  % NVK ADDED to distinguish between this rule and the irrel rule:
  atomic(Name),
  !.

arg(N) => rename(feed(rel(Name, Var)), Var) :-
  optimizerOption(nestedRelations),
  isStarQuery,
  argument(N, rel(Name, Var)),
  % NVK ADDED to distinguish between this rule and the irrel rule:
  atomic(Name),
  !.

arg(N) => feedproject(rel(Name, *), AttrNames) :-
  optimizerOption(nestedRelations),
  argument(N, rel(Name, *)), %!, NVK MODIFIED
  % NVK ADDED to distinguish between this rule and the irrel rule:
  atomic(Name),
  % There is no feedproject operator for nrel relations.
  %\+ secondoCatalogInfo(Name,_,_,[[nrel, _]]), 
  \+ is_nrel(Name),
  !,
  usedAttrList(rel(Name, *), AttrNames).

arg(N) => rename(feedproject(rel(Name, Var), AttrNames), Var) :-
  optimizerOption(nestedRelations),
  argument(N, rel(Name, Var)),
  % NVK ADDED to distinguish between this rule and the irrel rule:
  atomic(Name),
  % There is no feedproject operator for nrel relations.
  %\+ secondoCatalogInfo(Name,_,_,[[nrel, _]]), 
  \+ is_nrel(Name),
  !,
  usedAttrList(rel(Name, Var), AttrNames).

/*
There is no feedproject operator for nrel relations, hence this is translated into a project after the regular feed.

*/
arg(N) => project(feed(rel(Name, *)), AttrNames) :-
  optimizerOption(nestedRelations),
  argument(N, rel(Name, *)),
  is_nrel(Name),
  !,
  usedAttrList(rel(Name, *), AttrNames).

arg(N) => rename(project(feed(rel(Name, Var)), AttrNames), Var) :-
  optimizerOption(nestedRelations),
  argument(N, rel(Name, Var)),
  is_nrel(Name),
  !,
  usedAttrList(rel(Name, Var), AttrNames).

% NVK ADDED NR END

/*
NVK ADDED MA
There is not much to translate. Used to integrate the sortby operator into the memory allocation optimization.

*/

/*
----
sortby(N, AttrNames) => sortby(N, AttrNames) :-
  !.
----



*/
% NVK ADDED MA END

arg(N) => feed(rel(Name, *)) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED NR
  isStarQuery,
  argument(N, rel(Name, *)), !.

arg(N) => rename(feed(rel(Name, Var)), Var) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED NR
  isStarQuery,
  argument(N, rel(Name, Var)), !.

arg(N) => feedproject(rel(Name, *), AttrNames) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED NR
  argument(N, rel(Name, *)), !,
  usedAttrList(rel(Name, *), AttrNames).

arg(N) => rename(feedproject(rel(Name, Var), AttrNames), Var) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED NR
  argument(N, rel(Name, Var)), !,
  usedAttrList(rel(Name, Var), AttrNames).


/*
5.2.2 Translation of Selections

Be careful with cuts here, as backtracking is used to find all possible
translations of each edge!

*/

select(Arg, pr(Pred, _)) => filter(ArgS, Pred) :-
  Arg => ArgS.

select(Arg, pr(Pred, _, _)) => filter(ArgS, Pred) :-
  Arg => ArgS.


/*

Translation of selections using indices.

July 2008, Christian D[ue]ntgen. Added rules covering mtree indexes.

July 2008, Christian D[ue]ntgen. Problems with indexselect() => , when using
      A = B, where A ad B are attributes of the same relation.

April 2006, Christian D[ue]ntgen. Added project-translation for index selections.

May 2006, Translating a selection-predicate into a combination of ~windowintersects~/
~windowintersectsS~ and ~filter~, the optimizer will add an initial project ~after~ the
filter. For non-star-queries with one single selection this may even result in plans
with two consecutive projections. This problem is still unsolved.

*/
select(arg(N), Y) => X :-
  isStarQuery, % no projection needed
  indexselect(arg(N), Y) => X.

select(arg(N), Y) => project(X, AttrNames) :-
  not(isStarQuery),
  argument(N, rel(Name, *)),
  indexselect(arg(N), Y) => X,
  % no renaming, so just use native projection attr list
  usedAttrList(rel(Name, *), AttrNames).

select(arg(N), Y) => project(X, RenamedAttrNames) :-
  not(isStarQuery),
  argument(N, rel(Name, Var)), Var \= * ,
  indexselect(arg(N), Y) => X,
  usedAttrList(rel(Name, Var), AttrNames),
  % with renaming, so modify the projection attr list
  renameAttributes(Var, AttrNames, RenamedAttrNames).


% replace (Attr = Term) by (Term = Attr)
indexselect(arg(N), pr(attr(AttrName, Arg, Case) = Y, Rel)) => X :-
  not(isSubquery(Y)),
  indexselect(arg(N), pr(Y = attr(AttrName, Arg, Case), Rel)) => X.

% generic rule for (Term = Attr): exactmatch using btree or hashtable
% without rename
indexselect(arg(N), pr(Y = attr(AttrName, Arg, AttrCase), _)) =>
  exactmatch(dbobject(IndexName), rel(Name, *), Y)
  :-
  argument(N, rel(Name, *)),
  hasIndex(rel(Name,*),attr(AttrName,Arg,AttrCase),DCindex,IndexType),
  dcName2externalName(DCindex,IndexName),
  (IndexType = btree; IndexType = hash).

% generic rule for (Term = Attr): exactmatch using btree or hashtable
% with rename
indexselect(arg(N), pr(Y = attr(AttrName, Arg, AttrCase), _)) =>
  rename(exactmatch(dbobject(IndexName), rel(Name, Var), Y), Var)
  :-
  argument(N, rel(Name, Var)), Var \= * ,
  hasIndex(rel(Name,Var),attr(AttrName,Arg,AttrCase),DCindex,IndexType),
  dcName2externalName(DCindex,IndexName),
  (IndexType = btree; IndexType = hash).


% generic rule for (Term = Attr): rangesearch using mtree
% without rename
indexselect(arg(N), pr(Y = attr(AttrName, Arg, AttrCase), _)) =>
  rangesearch(dbobject(IndexName), rel(Name, *), Y, Y)
  :-
  argument(N, rel(Name, *)),
  hasIndex(rel(Name,*),attr(AttrName,Arg,AttrCase),DCindex,IndexType),
  dcName2externalName(DCindex,IndexName),
  IndexType = mtree.

% generic rule for (Term = Attr): rangesearch using mtree
% with rename
indexselect(arg(N), pr(Y = attr(AttrName, Arg, AttrCase), _)) =>
  rename(rangesearch(dbobject(IndexName), rel(Name, Var), Y), Var, Y)
  :-
  argument(N, rel(Name, Var)), Var \= * ,
  hasIndex(rel(Name,Var),attr(AttrName,Arg,AttrCase),DCindex,IndexType),
  dcName2externalName(DCindex,IndexName),
  IndexType = mtree.

% replace (Attr <= Term) with (Term >= Attr)
indexselect(arg(N), pr(attr(AttrName, Arg, Case) <= Y, Rel)) => X :-
  indexselect(arg(N), pr(Y >= attr(AttrName, Arg, Case), Rel)) => X.

% generic rule for (Term >= Attr): leftrange using btree
% without rename
indexselect(arg(N), pr(Y >= attr(AttrName, Arg, AttrCase), _)) =>
  leftrange(dbobject(IndexName), rel(Name, *), Y)
  :-
  argument(N, rel(Name, *)),
  hasIndex(rel(Name,*), attr(AttrName, Arg, AttrCase), DCindex, btree),
  dcName2externalName(DCindex,IndexName).

% generic rule for (Term >= Attr): leftrange using btree
% with rename
indexselect(arg(N), pr(Y >= attr(AttrName, Arg, AttrCase), _)) =>
  rename(leftrange(dbobject(IndexName), rel(Name, Var), Y), Var)
  :-
  argument(N, rel(Name, Var)), Var \= * ,
  hasIndex(rel(Name,Var), attr(AttrName, Arg, AttrCase), DCindex, btree),
  dcName2externalName(DCindex,IndexName).


% replace (Attr >= Term) with (Term <= Attr)
indexselect(arg(N), pr(attr(AttrName, Arg, Case) >= Y, Rel)) => X :-
  indexselect(arg(N), pr(Y <= attr(AttrName, Arg, Case), Rel)) => X.

% generic rule for (Term <= Attr): rightrange using btree
% without rename
indexselect(arg(N), pr(Y <= attr(AttrName, Arg, AttrCase), _)) =>
  rightrange(dbobject(IndexName), rel(Name, *), Y)
  :-
  argument(N, rel(Name, *)),
  hasIndex(rel(Name, *), attr(AttrName, Arg, AttrCase), DCindex, btree),
  dcName2externalName(DCindex,IndexName).

% generic rule for (Term <= Attr): rightrange using btree
% with rename
indexselect(arg(N), pr(Y <= attr(AttrName, Arg, AttrCase), _)) =>
  rename(rightrange(dbobject(IndexName), rel(Name, Var), Y), Var)
  :-
  argument(N, rel(Name, Var)), Var \= * ,
  hasIndex(rel(Name,Var), attr(AttrName, Arg, AttrCase), DCindex, btree),
  dcName2externalName(DCindex,IndexName).


/*

C. D[ue]ntgen, Feb 2006: When using renaming, the indexselect rule using
``windowintersects'' failed, because the rule accesses the original (not renamed)
relation. The correct approach is to do a rename on the result ~before~ the
filter is used.

*/


% Generic indexselect translation for predicates checking on mbbs
indexselect(arg(N), pr(Pred, _/*Rel*/)) =>
  filter(windowintersects(dbobject(IndexName), rel(Name, *), Y), Pred)
  :-
  (  Pred =.. [OP, attr(AttrName, Arg, AttrCase), Y]
   ; Pred =.. [OP, Y, attr(AttrName, Arg, AttrCase)] ),
  isBBoxPredicate(OP),
%   getTypeTree(Pred,[(1,Rel)],[OP,Args,ResType]),
%   findall(T,(member([_,_,T],Args)),ArgTypes),
  argument(N, rel(Name, *)),
  (     hasIndex(rel(Name,_),attr(AttrName,Arg,AttrCase),DCindex,rtree)
      ; hasIndex(rel(Name,_),attr(AttrName,Arg,AttrCase),DCindex,rtree3)
      ; hasIndex(rel(Name,_),attr(AttrName,Arg,AttrCase),DCindex,rtree4)
      ; hasIndex(rel(Name,_),attr(AttrName,Arg,AttrCase),DCindex,rtree8)
  ),
  dcName2externalName(DCindex,IndexName).

indexselect(arg(N), pr(Pred, _)) =>
  filter(rename(windowintersects(dbobject(IndexName), rel(Name,*),bbox(Y)),
                RelAlias), Pred)
  :-
  (  Pred =.. [OP, attr(AttrName, Arg, AttrCase), Y]
   ; Pred =.. [OP, Y, attr(AttrName, Arg, AttrCase)]),
  isBBoxPredicate(OP),
  argument(N, rel(Name, RelAlias)), RelAlias \= *,
  (   hasIndex(rel(Name,_),attr(AttrName,Arg,AttrCase),DCindex,rtree)
    ; hasIndex(rel(Name,_),attr(AttrName,Arg,AttrCase),DCindex,rtree3)
    ; hasIndex(rel(Name,_),attr(AttrName,Arg,AttrCase),DCindex,rtree4)
    ; hasIndex(rel(Name,_),attr(AttrName,Arg,AttrCase),DCindex,rtree8)
  ),
  dcName2externalName(DCindex,IndexName).

% exploit commutativity of operators (additional cases)
indexselect(arg(N), pr(Pred, Rel)) => X :-
  Pred =.. [OP, Y, attr(AttrName, Arg, Case)],
  not(isSubquery(Y)),
  ( optimizerOption(determinePredSig)
    -> ( checkOpProperty(Pred,comm),
         isBBoxPredicate(Pred,_Dim)
       )
    ;  ( isBBoxPredicate(OP),
         isCommutativeOP(OP)
       )
  ),
  Pred2 =.. [OP, attr(AttrName, Arg, Case), Y],
  indexselect(arg(N), pr(Pred2, Rel)) => X.


/*
C. D[ue]ntgen, Apr 2006: Added rules for specialized spatio-temporal R-Tree indices.
These indices are recognized by their index type.

Again. a possible ~rename~ must be done before ~filter~ can be applied.

*/

indexselect(arg(N), Pred) => X :-
  optimizerOption(rtreeIndexRules),
  indexselectRT(arg(N), Pred) => X.


% 'present' with temporal(rtree,object) index
indexselectRT(arg(N), pr(attr(AttrName, Arg, AttrCase) present Y, _)) =>
  filter(gettuples(windowintersectsS(dbobject(IndexName), queryrect2d(Y)),
         rel(Name, *)), attr(AttrName, Arg, AttrCase) present Y)
  :-
  argument(N, rel(Name, *)),
  hasIndex(rel(Name, _), attr(AttrName, Arg, AttrCase),
           DCindex, temporal(rtree,object)),
  dcName2externalName(DCindex,IndexName).

indexselectRT(arg(N), pr(attr(AttrName, Arg, AttrCase) present Y, _)) =>
  filter(rename(gettuples(windowintersectsS(dbobject(IndexName),queryrect2d(Y)),
        rel(Name, *)), RelAlias), attr(AttrName, Arg, AttrCase) present Y)
  :-
  argument(N, rel(Name, RelAlias)), RelAlias \= * ,
  hasIndex(rel(Name, _), attr(AttrName, Arg, AttrCase),
           DCindex, temporal(rtree,object)),
  dcName2externalName(DCindex,IndexName).

% 'present' with temporal(rtree,unit) index
indexselectRT(arg(N), pr(attr(AttrName, Arg, AttrCase) present Y, _)) =>
  filter(gettuples(rdup(sort(windowintersectsS(dbobject(IndexName),
         queryrect2d(Y)))), rel(Name, *)),
         attr(AttrName, Arg, AttrCase) present Y)
  :-
  argument(N, rel(Name, *)),
  hasIndex(rel(Name,_), attr(AttrName,Arg,AttrCase),
           DCindex, temporal(rtree,unit)),
  dcName2externalName(DCindex,IndexName).

indexselectRT(arg(N), pr(attr(AttrName, Arg, AttrCase) present Y, _)) =>
  filter(rename(gettuples(rdup(sort(
         windowintersectsS(dbobject(IndexName), queryrect2d(Y)))),
         rel(Name, *)), RelAlias), attr(AttrName,Arg,AttrCase) present Y)
  :-
  argument(N, rel(Name, RelAlias)), RelAlias \= * ,
  hasIndex(rel(Name,_), attr(AttrName,Arg,AttrCase),
           DCindex, temporal(rtree,unit)),
  dcName2externalName(DCindex,IndexName).


% 'passes' with spatial(rtree,object) index
indexselectRT(arg(N), pr(attr(AttrName, Arg, AttrCase) passes Y, _)) =>
  filter(gettuples(windowintersectsS(dbobject(IndexName), bbox(Y)),
                                     rel(Name, *)),
         attr(AttrName, Arg, AttrCase) passes Y)
  :-
  argument(N, rel(Name, *)),
  hasIndex(rel(Name, _), attr(AttrName, Arg, AttrCase),
           DCindex, spatial(rtree,object)),
  dcName2externalName(DCindex,IndexName).

indexselectRT(arg(N), pr(attr(AttrName, Arg, AttrCase) passes Y, _)) =>
  filter(rename(gettuples(windowintersectsS(dbobject(IndexName), bbox(Y)),
         rel(Name, *)), RelAlias), attr(AttrName, Arg, AttrCase) passes Y)
  :-
  argument(N, rel(Name, RelAlias)), RelAlias \= * ,
  hasIndex(rel(Name, _), attr(AttrName, Arg, AttrCase),
           DCindex, spatial(rtree,object)),
  dcName2externalName(DCindex,IndexName).


% 'passes' with spatial(rtree,unit) index
indexselectRT(arg(N), pr(attr(AttrName, Arg, AttrCase) passes Y, _)) =>
  filter(gettuples(rdup(sort(windowintersectsS(dbobject(IndexName), bbox(Y)))),
         rel(Name, *)), attr(AttrName, Arg, AttrCase) passes Y)
  :-
  argument(N, rel(Name, *)),
  hasIndex(rel(Name,_), attr(AttrName,Arg,AttrCase),
           DCindex, spatial(rtree,unit)),
  dcName2externalName(DCindex,IndexName).

indexselectRT(arg(N), pr(attr(AttrName, Arg, AttrCase) passes Y, _)) =>
  filter(rename(gettuples(rdup(sort(windowintersectsS(dbobject(IndexName),
                                                      bbox(Y)))),
         rel(Name, *)), RelAlias), attr(AttrName, Arg, AttrCase) passes Y)
  :-
  argument(N, rel(Name, RelAlias)), RelAlias \= * ,
  hasIndex(rel(Name,_), attr(AttrName,Arg,AttrCase),
           DCindex, spatial(rtree,unit)),
  dcName2externalName(DCindex,IndexName).

% special rules for range queries in the form distance(m(x), y) < d
% 'distance <' with spatial(rtree,object) index
indexselectRT(arg(N), pr(distance(attr(AttrName, Arg, AttrCase), Y)< D , _)) =>
      filter(gettuples(windowintersectsS(dbobject(IndexName),
        enlargeRect(bbox(Y),D,D)),  rel(Name, *)),
        distance(attr(AttrName, Arg, AttrCase), Y)< D)
  :-
%the translation will work only if Y is of spatial type
%otherwise it will crash
%We still need to develop a predicate that will check the type of a param
  argument(N, rel(Name, *)),
  hasIndex(rel(Name, _), attr(AttrName, Arg, AttrCase),
           DCindex, spatial(rtree,object)),
  dcName2externalName(DCindex,IndexName).

indexselectRT(arg(N), pr(distance(attr(AttrName, Arg, AttrCase), Y)< D , _)) =>
      filter(rename(gettuples(windowintersectsS(dbobject(IndexName),
        enlargeRect(bbox(Y),D,D)),  rel(Name, *)), RelAlias),
        distance(attr(AttrName, Arg, AttrCase), Y)< D)
  :-
%the translation will work only if Y is of spatial type
%otherwise it will crash
%We still need to develop a predicate that will check the type of a param
  argument(N, rel(Name, RelAlias)), RelAlias \= *,
  hasIndex(rel(Name, _), attr(AttrName, Arg, AttrCase),
           DCindex, spatial(rtree,object)),
  dcName2externalName(DCindex,IndexName).

% 'distance <' with spatial(rtree,unit) index
indexselectRT(arg(N), pr(distance(attr(AttrName, Arg, AttrCase), Y)< D , _)) =>
      filter(gettuples(rdup(sort(windowintersectsS(dbobject(IndexName),
        enlargeRect(bbox(Y),D,D)))),  rel(Name, *)),
        distance(attr(AttrName, Arg, AttrCase), Y)< D)
  :-
%the translation will work only if Y is of spatial type
%otherwise it will crash
%We still need to develop a predicate that will check the type of a param
  argument(N, rel(Name, *)),
  hasIndex(rel(Name,_), attr(AttrName,Arg,AttrCase),
           DCindex, spatial(rtree,unit)),
  dcName2externalName(DCindex,IndexName).

indexselectRT(arg(N), pr(distance(attr(AttrName, Arg, AttrCase), Y)< D , _)) =>
      filter(rename(gettuples(rdup(sort(windowintersectsS(dbobject(IndexName),
        enlargeRect(bbox(Y),D,D)))),rel(Name, *)), RelAlias),
        distance(attr(AttrName, Arg, AttrCase), Y)< D)
  :-
%the translation will work only if Y is of spatial type
%otherwise it will crash
%We still need to develop a predicate that will check the type of a param
  argument(N, rel(Name, RelAlias)), RelAlias \= *,
  hasIndex(rel(Name, _), attr(AttrName, Arg, AttrCase),
           DCindex, spatial(rtree,unit)),
  dcName2externalName(DCindex,IndexName).

% 'bbox(x) intersects box3d(bbox(Z),Y)' with unit_3d index
indexselectRT(arg(N), pr(bbox(attr(AttrName, Arg, AttrCase)) intersects
                       box3d(bbox(Z),Y), _)) =>
  gettuples(rdup(sort(windowintersectsS(dbobject(IndexName),
            box3d(bbox(Z),Y)))), rel(Name, *))
  :-
  argument(N, rel(Name, *)),
  hasIndex(rel(Name,_), attr(AttrName,Arg,AttrCase), DCindex, unit_3d),
  dcName2externalName(DCindex,IndexName).

indexselectRT(arg(N), pr(bbox(attr(AttrName, Arg, AttrCase)) intersects
                       box3d(bbox(Z),Y), _)) =>
  rename(gettuples(rdup(sort(windowintersectsS(dbobject(IndexName),
         box3d(bbox(Z),Y)))), rel(Name, *)), RelAlias)
  :-
  argument(N, rel(Name, RelAlias)), RelAlias \= * ,
  hasIndex(rel(Name,_), attr(AttrName,Arg,AttrCase),
           DCindex, spatiotemporal(rtree3,unit)),
  dcName2externalName(DCindex,IndexName).


% 'bbox(x) intersects box3d(bbox(Z),Y)' with object_3d index
indexselectRT(arg(N), pr(bbox(attr(AttrName, Arg, AttrCase)) intersects
                       box3d(bbox(Z),Y), _)) =>
  gettuples(windowintersectsS(dbobject(IndexName), box3d(bbox(Z),Y)),
            rel(Name, *))
  :-
  argument(N, rel(Name, *)),
  hasIndex(rel(Name,_), attr(AttrName,Arg,AttrCase),
           DCindex, spatiotemporal(rtree3,object)),
  dcName2externalName(DCindex,IndexName).

indexselectRT(arg(N), pr(bbox(attr(AttrName, Arg, AttrCase)) intersects
                       box3d(bbox(Z),Y), _)) =>
  rename(gettuples(windowintersectsS(dbobject(IndexName), box3d(bbox(Z),Y)),
         rel(Name, *)), RelAlias)
  :-
  argument(N, rel(Name, RelAlias)), RelAlias \= * ,
  hasIndex(rel(Name,_), attr(AttrName,Arg,AttrCase),
           DCindex, spatiotemporal(rtree3,object)),
  dcName2externalName(DCindex,IndexName).

% 'intersects' with object_4d
% does not consider 4d-boxes created 'on the fly'
indexselectRT(arg(N), pr(attr(AttrName, Arg, AttrCase) intersects Y, _)) =>
  gettuples(windowintersectsS(dbobject(IndexName), Y), rel(Name, *))
  :-
  argument(N, rel(Name, *)),
  hasIndex(rel(Name,_), attr(AttrName,Arg,AttrCase), DCindex, rtree4),
  dcName2externalName(DCindex,IndexName).

indexselectRT(arg(N), pr(bbox(attr(AttrName, Arg, AttrCase)) intersects Y, _))
  => rename(gettuples(windowintersectsS(dbobject(IndexName), Y),
     rel(Name, *)), RelAlias)
  :-
  argument(N, rel(Name, RelAlias)), RelAlias \= * ,
  hasIndex(rel(Name,_), attr(AttrName,Arg,AttrCase), DCindex, rtree4),
  dcName2externalName(DCindex,IndexName).

% 'intersects' with object_8d index
% does not consider 8d-boxes created 'on the fly'
indexselectRT(arg(N), pr(attr(AttrName, Arg, AttrCase) intersects Y, _)) =>
  gettuples(windowintersectsS(dbobject(IndexName), Y), rel(Name, *))
  :-
  argument(N, rel(Name, *)),
  hasIndex(rel(Name,_), attr(AttrName,Arg,AttrCase), DCindex, rtree8),
  dcName2externalName(DCindex,IndexName).

indexselectRT(arg(N), pr(bbox(attr(AttrName, Arg, AttrCase)) intersects Y, _))
  => rename(gettuples(windowintersectsS(dbobject(IndexName), Y),
     rel(Name, *)), RelAlias)
  :-
  argument(N, rel(Name, RelAlias)), RelAlias \= * ,
  hasIndex(rel(Name,_), attr(AttrName,Arg,AttrCase), DCindex, rtree8),
  dcName2externalName(DCindex,IndexName).

/*
Here ~ArgS~ is meant to indicate ``argument stream''.

*/


/*

5.2.3 Translation of Joins

A join can always be translated to a ~symmjoin~.

Moved to a later position.

*/





/*

Index joins:

*/


join(Arg1, arg(N), pr(X=Y, _, _)) => loopjoin(Arg1S, MatchExpr) :-
  isOfSecond(Attr2, X, Y),    % get the attrib from the 2nd relation in Attr2
  isNotOfSecond(Expr1, X, Y), % get the other argument in Expr1
  argument(N, RelDescription),
  hasIndex(RelDescription, Attr2, DCindex, btree),
  dcName2externalName(DCindex,IndexName),
  Arg1 => Arg1S,
  exactmatch(IndexName, arg(N), Expr1) => MatchExpr.

join(arg(N), Arg2, pr(X=Y, _, _)) => loopjoin(Arg2S, MatchExpr) :-
  isOfFirst(Attr1, X, Y),
  isNotOfFirst(Expr2, X, Y),
  argument(N, RelDescription),
  hasIndex(RelDescription, Attr1, DCindex, btree),
  dcName2externalName(DCindex,IndexName),
  Arg2 => Arg2S,
  exactmatch(IndexName, arg(N), Expr2) => MatchExpr.

/*
A loopjoin keeps the order of the outer relation, so it can also be
used for sortedjoin

*/
sortedjoin(Arg1, arg(N), pr(X=Y, _, _), Arg1, _) => loopjoin(Arg1S,
            MatchExpr) :-
  isOfSecond(Attr2, X, Y),    % get the attrib from the 2nd relation in Attr2
  isNotOfSecond(Expr1, X, Y), % get the other argument in Expr1
  argument(N, RelDescription),
  hasIndex(RelDescription, Attr2, DCindex, btree),
  dcName2externalName(DCindex,IndexName),
  Arg1 => Arg1S,
  exactmatch(IndexName, arg(N), Expr1) => MatchExpr.

sortedjoin(arg(N), Arg2, pr(X=Y, _, _), Arg2, _) => loopjoin(Arg2S,
            MatchExpr) :-
  isOfFirst(Attr1, X, Y),
  isNotOfFirst(Expr2, X, Y),
  argument(N, RelDescription),
  hasIndex(RelDescription, Attr1, DCindex, btree),
  dcName2externalName(DCindex,IndexName),
  Arg2 => Arg2S,
  exactmatch(IndexName, arg(N), Expr2) => MatchExpr.

/*
If the Inner Relation doesn't have an index on the join attribute,
try to create a temporary one

*/

sortedjoin(Arg1, arg(N), pr(X=Y, _, _), Arg1, UpperBound) =>
 loopjoin(Arg1S, MatchExpr) :-
  isOfSecond(Attr2, X, Y),    % get the attrib from the 2nd relation in Attr2
  isNotOfSecond(Expr1, X, Y), % get the other argument in Expr1
  argument(N, RelDescription),
  not(hasIndex(RelDescription, Attr2, _, btree)),
  UpperBound >= 99997,
  convertToLfName(Attr2, LfAttr),
  convertToLfName(RelDescription, LfRel),
  keyAttributeTypeMatchesIndexType(LfRel, LfAttr, btree),
  Arg1 => Arg1S,
  exactmatch(tmpindex(RelDescription, Attr2), arg(N), Expr1) => MatchExpr.

sortedjoin(arg(N), Arg2, pr(X=Y, _, _), Arg2, UpperBound) =>
 loopjoin(Arg2S, MatchExpr) :-
  isOfFirst(Attr1, X, Y),
  isNotOfFirst(Expr2, X, Y),
  argument(N, RelDescription),
  not(hasIndex(RelDescription, Attr1, _, btree)),
  UpperBound >= 99997,
  convertToLfName(Attr1, LfAttr),
  convertToLfName(RelDescription, LfRel),
  keyAttributeTypeMatchesIndexType(LfRel, LfAttr, btree),
  Arg2 => Arg2S,
  exactmatch(tmpindex(RelDescription, Attr1), arg(N), Expr2) => MatchExpr.


/*
Try to apply projections as early as possible!

*/
exactmatch(IndexName, arg(N), Expr) =>
  exactmatch(dbobject(IndexName), Rel, Expr) :-
  isStarQuery,
  argument(N, Rel),
  Rel = rel(_, *), % no renaming needed
  !.

exactmatch(IndexName, arg(N), Expr) =>
  rename(exactmatch(dbobject(IndexName), Rel, Expr), Var) :-
  isStarQuery, % no need for project
  argument(N, Rel),
  Rel = rel(_, Var),
  !.

exactmatch(IndexName, arg(N), Expr) =>
  project(exactmatch(dbobject(IndexName), Rel, Expr), AttrNames) :-
  not(isStarQuery),
  argument(N, Rel ),
  Rel = rel(_, *), % no renaming needed
  usedAttrList(Rel, AttrNames),
  !.


exactmatch(IndexName, arg(N), Expr) =>
  rename(project(exactmatch(dbobject(IndexName), Rel, Expr), AttrNames), Var) :-
  not(isStarQuery),
  argument(N, Rel),
  Rel = rel(_, Var), % with renaming
  usedAttrList(Rel, AttrNames),
  !.

/*
Jan09 Ingmar Goehr

Loopindexjoin with bbox predicate using specialized spatial R-Tree indices

*/

/*
The first case deals with predicates having at most two attributes as direct
arguments (no expressions).

*/

join(arg(N), Arg2, pr(Pred, _, _)) => filter(loopjoin(Arg2S, RTSpExpr),Pred) :-
  dm(translation,['Call is:',join(arg(N), Arg2, pr(Pred, _, _))
    => filter(loopjoin(Arg2S, RTSpExpr),Pred),'\n']),
  Pred =.. [Op, X, Y],
  isBBoxPredicate(Op),
  isOfFirst(Attr1, X, Y),     % determine attribute from the first relation
  isNotOfFirst(Expr2, X, Y),  % determine attribute not from the first relation
  argument(N, RelDescription),  % get first relation
  hasIndex(RelDescription, Attr1, DCindex, spatial(rtree, unit)),
  dcName2externalName(DCindex,IndexName),
  Arg2 => Arg2S,
  rtSpExpr(IndexName, arg(N), Expr2) => RTSpExpr.


/*
The second case deals with predicates having trhee attributes as direct
arguments (no expressions).

*/

rtSpExpr(IndexName, arg(N), Expr) =>
  rename(gettuples(rdup(sort(windowintersectsS(dbobject(IndexName),
                                              bbox(Expr)))),Rel),Var)
          :-
  argument(N, Rel),
  Rel = rel(_, Var),    % with renaming
  !.

rtSpExpr(IndexName, arg(N), Expr) =>
  gettuples(rdup(sort(windowintersectsS(dbobject(IndexName),bbox(Expr)))),Rel)
          :-
  argument(N, Rel),
  Rel = rel(_, *),    % without renaming
  !.

join(arg(N), Arg2, pr(Pred, _, _))
  => filter(loopjoin(Arg2S, RTSpTmpExpr),Pred) :-
  dm(translation,['Call is:',join(arg(N), Arg2, pr(Pred, _, _)) 
    => filter(loopjoin(Arg2S, RTSpTmpExpr),Pred),'\n']),
  fetchAttributeList(Pred,L),
  dm(translation,['L = ', L , '\n']),
  L = [A,B,C],
  isOfFirst(Attr1,A,B,C),
  areNotOfFirst(Attr2,Attr3,A,B,C),
  argument(N, RelDescription),
  hasIndex(RelDescription, Attr1, DCindex, spatiotemporal(rtree3, unit)),
  dcName2externalName(DCindex,IndexName),
  Arg2 => Arg2S,
  rtSpTmpExpr(IndexName, arg(N), Attr2, Attr3) => RTSpTmpExpr.

rtSpTmpExpr(IndexName, arg(N), Expr1, Expr2) =>
 rename(gettuples(rdup(sort(windowintersectsS(dbobject(IndexName),
                                          box3d(bbox(Expr2),Expr1)))),Rel),Var)
  :-
 argument(N,Rel),
 Rel = rel(_,Var), % with renaming
 !.

rtSpTmpExpr(IndexName, arg(N), Expr1, Expr2) =>
 gettuples(rdup(sort(windowintersectsS(dbobject(IndexName),
                                          box3d(bbox(Expr2),Expr1)))),Rel)
  :-
 argument(N,Rel),
 Rel = rel(_, *), % without renaming
 !.

/*
One could easily add rules for ~loopjoin~ with ~rightrange~ and ~leftrange~
operators for joins on "<=", ">=", "<" and ">" here.

*/


/*

Loopindexjoin with bbox predicate using an rtree

Some joins can be performed as a combination of a loopjoin using a
windowintersects and a consecutive filter.
This requires one argument to be a ~arg(N)~.

*/


join(Arg1, arg(N), pr(Pred, _, _))
  => filter(loopjoin(Arg1S, RTreeLookupExpr), Pred) :-
  Pred =.. [Op, X, Y],        % join condition is
  isBBoxPredicate(Op),        % a bbox-predicate
  isOfSecond(Attr2, X, Y),    % get the attrib from the 2nd relation in Attr2
  isNotOfSecond(Expr1, X, Y), % get the other argument in Expr1
  argument(N, RelDescription),% get info on 2nd relation
  (                           % the relation has an index on Attr2:
      hasIndex(RelDescription, Attr2, DCindex, rtree)
    ;   hasIndex(RelDescription, Attr2, DCindex, rtree3)
    ;   hasIndex(RelDescription, Attr2, DCindex, rtree4)
    ;   hasIndex(RelDescription, Attr2, DCindex, rtree8)
  ),
  dcName2externalName(DCindex,IndexName),
  Arg1 => Arg1S,
  rtreeindexlookupexpr(IndexName, arg(N), Expr1) => RTreeLookupExpr.

join(arg(N), Arg2, pr(Pred, _, _))
  => filter(loopjoin(Arg2S, RTreeLookupExpr), Pred) :-
  Pred =.. [Op, X, Y],
  isBBoxPredicate(Op),
  isOfFirst(Attr1, X, Y),
  isNotOfFirst(Expr2, X, Y),
  argument(N, RelDescription),
  (   hasIndex(RelDescription, Attr1, DCindex, rtree)
    ; hasIndex(RelDescription, Attr1, DCindex, rtree3)
    ; hasIndex(RelDescription, Attr1, DCindex, rtree4)
    ; hasIndex(RelDescription, Attr1, DCindex, rtree8)
  ),
  dcName2externalName(DCindex,IndexName),
  Arg2 => Arg2S,
  rtreeindexlookupexpr(IndexName, arg(N), Expr2) => RTreeLookupExpr.

/*
A loopjoin keeps the order of the outer relation, so it can also be
used for sortedjoin

*/
sortedjoin(Arg1, arg(N), pr(Pred, _, _), Arg1, _)
  => filter(loopjoin(Arg1S, RTreeLookupExpr), Pred) :-
  Pred =.. [Op, X, Y],        % join condition is
  isBBoxPredicate(Op),        % a bbox-predicate
  isOfSecond(Attr2, X, Y),    % get the attrib from the 2nd relation in Attr2
  isNotOfSecond(Expr1, X, Y), % get the other argument in Expr1
  argument(N, RelDescription),% get info on 2nd relation
  (                           % the relation has an index on Attr2:
      hasIndex(RelDescription, Attr2, DCindex, rtree)
    ;   hasIndex(RelDescription, Attr2, DCindex, rtree3)
    ;   hasIndex(RelDescription, Attr2, DCindex, rtree4)
    ;   hasIndex(RelDescription, Attr2, DCindex, rtree8)
  ),
  dcName2externalName(DCindex,IndexName),
  Arg1 => Arg1S,
  rtreeindexlookupexpr(IndexName, arg(N), Expr1) => RTreeLookupExpr.

sortedjoin(arg(N), Arg2, pr(Pred, _, _), Arg2, _)
  => filter(loopjoin(Arg2S, RTreeLookupExpr), Pred) :-
  Pred =.. [Op, X, Y],
  isBBoxPredicate(Op),
  isOfFirst(Attr1, X, Y),
  isNotOfFirst(Expr2, X, Y),
  argument(N, RelDescription),
  (   hasIndex(RelDescription, Attr1, DCindex, rtree)
    ; hasIndex(RelDescription, Attr1, DCindex, rtree3)
    ; hasIndex(RelDescription, Attr1, DCindex, rtree4)
    ; hasIndex(RelDescription, Attr1, DCindex, rtree8)
  ),
  dcName2externalName(DCindex,IndexName),
  Arg2 => Arg2S,
  rtreeindexlookupexpr(IndexName, arg(N), Expr2) => RTreeLookupExpr.

/*
Try to apply projections as early as possible!

*/


rtreeindexlookupexpr(IndexName, arg(N), Expr) =>
  windowintersects(dbobject(IndexName), Rel, Expr) :-
  isStarQuery,
  argument(N, Rel),
  Rel = rel(_, *), % no renaming needed
  !.

rtreeindexlookupexpr(IndexName, arg(N), Expr) =>
  rename(windowintersects(dbobject(IndexName), Rel, Expr), Var) :-
  isStarQuery, % no need for project
  argument(N, Rel),
  Rel = rel(_, Var),
  !.

rtreeindexlookupexpr(IndexName, arg(N), Expr) =>
  project(windowintersects(dbobject(IndexName), Rel, Expr), AttrNames) :-
  not(isStarQuery),
  argument(N, Rel ),
  Rel = rel(_, *), % no renaming needed
  usedAttrList(Rel, AttrNames),
  !.


rtreeindexlookupexpr(IndexName, arg(N), Expr) =>
  rename(project(windowintersects(dbobject(IndexName), Rel, Expr), AttrNames),
         Var) :-
  not(isStarQuery),
  argument(N, Rel),
  Rel = rel(_, Var), % with renaming
  usedAttrList(Rel, AttrNames),
  !.

/*
Joins with generic predicates using bbox checks, that can exploit
the spatialjoin to reduce the set of candidates. Predicates are chosen by
their properties as defiend by predicates ~isBBoxPredicate/1~ and
~isCommutativeOP/1~ in file ``operators.pl''.

CD: There seems to be a problem in this rules. When I added the rules
for 'Loopindexjoin with bbox predicate using an rtree', I first did this
behind this section. Then, they were never used. I inserted them in front of
this section, and it worked. There seems to exist a critical cut somewhere...

*/

join(Arg1, Arg2, pr(Pred, R1, R2)) => JoinPlan :-
  Pred =.. [Op, X, Y],
  ( optimizerOption(determinePredSig)
    -> ( checkOpProperty(Pred,comm),
         isBBoxPredicate(Pred,_Dim)
       )
    ; (isBBoxPredicate(Op),
       isCommutativeOP(Op)
      )
  ),
  X = attr(_, _, _),
  Y = attr(_, _, _), %!, % perhaps, this cut is the reason to the ^^^problem
                     % indeed, the cut prevented use of symmjoin for  
                     % bbox operators. RHG 5.2.2013
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  join00(Arg1S, Arg2S, pr(Pred, R1, R2)) => JoinPlan.


% spatialjoin with generic BBoxPredicate:
join00(Arg1S, Arg2S, pr(Pred, _, _)) => filter(spatialjoin(Arg1S,
  Arg2S, attrname(Attr1), attrname(Attr2)), Pred2) :-
  Pred =.. [Op, X, Y],
  ( optimizerOption(determinePredSig)
    -> ( checkOpProperty(Pred,comm),
         isBBoxPredicate(Pred,_Dim)
       )
    ; (isBBoxPredicate(OP),
       isCommutativeOP(OP)
      )
  ),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y),
  Pred2 =.. [Op, Attr1, Attr2].

% spatialjoin with generic commutative BBoxPredicate (additional):
join00(Arg1S, Arg2S, pr(Pred, _, _)) => filter(spatialjoin(Arg2S,
  Arg1S, attrname(Attr2), attrname(Attr1)), Pred2) :-
  Pred =.. [Op, Y, X],
  ( optimizerOption(determinePredSig)
    -> ( checkOpProperty(Pred,comm),
         isBBoxPredicate(Pred,_Dim)
       )
    ;  ( isBBoxPredicate(OP),
         isCommutativeOP(OP)
       )
  ),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, Y, X),
  Pred2 =.. [Op, Attr1, Attr2].



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
        hashjoin[PLZ_p1, newPLZ, 99997]
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
        remove(JoinPlan, [attrname(attr(r_expr, 2, u))]) :-
  X = attr(_, _, _),
  not(Y = attr(_, _, _)),
  not(isSubquery(Y)),
  !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  Arg2Extend = extend(Arg2S, [newattr(attrname(attr(r_expr, 2, u)), Y)]),
  join00(Arg1S, Arg2Extend, pr(X=attr(r_expr, 2, u), R1, R2)) => JoinPlan.

join(Arg1, Arg2, pr(X=Y, R1, R2)) =>
        remove(JoinPlan, [attrname(attr(l_expr, 2, u))]) :-
  not(X = attr(_, _, _)),
  not(isSubquery(X)),
  Y = attr(_, _, _), !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  Arg1Extend = extend(Arg1S, [newattr(attrname(attr(l_expr, 1, u)), X)]),
  join00(Arg1Extend, Arg2S, pr(attr(l_expr, 1, u)=Y, R1, R2)) => JoinPlan.

join(Arg1, Arg2, pr(X=Y, R1, R2)) =>
        remove(JoinPlan, [attrname(attr(l_expr, 1, u)),
                attrname(attr(r_expr, 2, u))]) :-
  not(X = attr(_, _, _)),
  not(Y = attr(_, _, _)),
  not(isSubquery(Y)),
  not(isSubquery(X)),
  !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  Arg1Extend = extend(Arg1S, [newattr(attrname(attr(l_expr, 1, u)), X)]),
  Arg2Extend = extend(Arg2S, [newattr(attrname(attr(r_expr, 2, u)), Y)]),
  join00(Arg1Extend, Arg2Extend,
        pr(attr(l_expr, 1, u)=attr(r_expr, 2, u), R1, R2)) => JoinPlan.


/*
Finally, a join can always be translated to a ~symmjoin~.

*/

join(Arg1, Arg2, pr(Pred, _, _)) => symmjoin(Arg1S, Arg2S, Pred) :-
  not(optimizerOption(noSymmjoin)),
  Arg1 => Arg1S,
  Arg2 => Arg2S.


/*
As join00 could be an orderkeeping sort, we also need a transformation
for sortedjoin

*/

sortedjoin(Arg1, Arg2, pr(X=Y, R1, R2), Arg1, UpperBound) => JoinPlan :-
  X = attr(_, _, _),
  Y = attr(_, _, _), !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  sortedjoin00(Arg1S, Arg2S, pr(X=Y, R1, R2), Arg1S, UpperBound) => JoinPlan.

sortedjoin(Arg1, Arg2, pr(X=Y, R1, R2), Arg2, UpperBound) => JoinPlan :-
  X = attr(_, _, _),
  Y = attr(_, _, _), !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  sortedjoin00(Arg1S, Arg2S, pr(X=Y, R1, R2), Arg2S, UpperBound) => JoinPlan.

sortedjoin(Arg1, Arg2, pr(X=Y, R1, R2), Arg1, UpperBound) =>
        remove(JoinPlan, [attrname(attr(r_expr, 2, l))]) :-
  X = attr(_, _, _),
  not(Y = attr(_, _, _)),
  not(isSubquery(Y)),
  !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  Arg2Extend = extend(Arg2S, [newattr(attrname(attr(r_expr, 2, l)), Y)]),
  sortedjoin00(Arg1S, Arg2Extend, pr(X=attr(r_expr, 2, l), R1, R2),
         Arg1S, UpperBound) => JoinPlan.

sortedjoin(Arg1, Arg2, pr(X=Y, R1, R2), Arg2, UpperBound) =>
        remove(JoinPlan, [attrname(attr(r_expr, 2, l))]) :-
  X = attr(_, _, _),
  not(Y = attr(_, _, _)),
  not(isSubquery(Y)),
  !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  Arg2Extend = extend(Arg2S, [newattr(attrname(attr(r_expr, 2, l)), Y)]),
  sortedjoin00(Arg1S, Arg2Extend, pr(X=attr(r_expr, 2, l), R1, R2),
         Arg2Extend, UpperBound) => JoinPlan.

sortedjoin(Arg1, Arg2, pr(X=Y, R1, R2), Arg1, UpperBound) =>
        remove(JoinPlan, [attrname(attr(l_expr, 2, l))]) :-
  not(X = attr(_, _, _)),
  not(isSubquery(X)),
  Y = attr(_, _, _), !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  Arg1Extend = extend(Arg1S, [newattr(attrname(attr(l_expr, 1, l)), X)]),
  sortedjoin00(Arg1Extend, Arg2S, pr(attr(l_expr, 1, l)=Y, R1, R2),
         Arg1Extend, UpperBound) => JoinPlan.

sortedjoin(Arg1, Arg2, pr(X=Y, R1, R2), Arg2, UpperBound) =>
        remove(JoinPlan, [attrname(attr(l_expr, 2, l))]) :-
  not(X = attr(_, _, _)),
  not(isSubquery(X)),
  Y = attr(_, _, _), !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  Arg1Extend = extend(Arg1S, [newattr(attrname(attr(l_expr, 1, l)), X)]),
  sortedjoin00(Arg1Extend, Arg2S, pr(attr(l_expr, 1, l)=Y, R1, R2),
         Arg2S, UpperBound) => JoinPlan.

sortedjoin(Arg1, Arg2, pr(X=Y, R1, R2), Arg1, UpperBound) =>
        remove(JoinPlan, [attrname(attr(l_expr, 1, l)),
                attrname(attr(r_expr, 2, l))]) :-
  not(X = attr(_, _, _)),
  not(Y = attr(_, _, _)),
  not(isSubquery(Y)),
  not(isSubquery(X)),
  !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  Arg1Extend = extend(Arg1S, [newattr(attrname(attr(l_expr, 1, l)), X)]),
  Arg2Extend = extend(Arg2S, [newattr(attrname(attr(r_expr, 2, l)), Y)]),
  sortedjoin00(Arg1Extend, Arg2Extend,
        pr(attr(l_expr, 1, l)=attr(r_expr, 2, l), R1, R2),
         Arg1Extend, UpperBound) => JoinPlan.

sortedjoin(Arg1, Arg2, pr(X=Y, R1, R2), Arg2, UpperBound) =>
        remove(JoinPlan, [attrname(attr(l_expr, 1, l)),
                attrname(attr(r_expr, 2, l))]) :-
  not(X = attr(_, _, _)),
  not(Y = attr(_, _, _)),
  not(isSubquery(Y)),
  not(isSubquery(X)),
  !,
  Arg1 => Arg1S,
  Arg2 => Arg2S,
  Arg1Extend = extend(Arg1S, [newattr(attrname(attr(l_expr, 1, l)), X)]),
  Arg2Extend = extend(Arg2S, [newattr(attrname(attr(r_expr, 2, l)), Y)]),
  sortedjoin00(Arg1Extend, Arg2Extend,
        pr(attr(l_expr, 1, l)=attr(r_expr, 2, l), R1, R2),
         Arg2Extend, UpperBound) => JoinPlan.

join00(Arg1S, Arg2S, pr(X = Y, _, _)) => sortmergejoin(Arg1S, Arg2S,
        attrname(Attr1), attrname(Attr2))   :-
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).


join00(Arg1S, Arg2S, pr(X = Y, _, _)) => hashjoin(Arg1S, Arg2S,
        attrname(Attr1), attrname(Attr2), 99997)   :-
  not(optimizerOption(noHashjoin)),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).

join00(Arg1S, Arg2S, pr(X = Y, _, _)) => hashjoin(Arg2S, Arg1S,
        attrname(Attr2), attrname(Attr1), 99997)   :-
  not(optimizerOption(noHashjoin)),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).

% NVK ADDED MA
:- dynamic(maUseNewTranslationRules/1).

join00(Arg1S, Arg2S, pr(X = Y, _, _)) => gracehashjoin(Arg1S, Arg2S,
		attrname(Attr1), attrname(Attr2), 99997) :-
  optimizerOption(memoryAllocation),
	maUseNewTranslationRules(true),
  \+ optimizerOption(noHashjoin),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).

join00(Arg1S, Arg2S, pr(X = Y, _, _)) => gracehashjoin(Arg2S, Arg1S,
		attrname(Attr2), attrname(Attr1), 99997) :-
  optimizerOption(memoryAllocation),
	maUseNewTranslationRules(true),
  \+ optimizerOption(noHashjoin),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).

join00(Arg1S, Arg2S, pr(X = Y, _, _)) => hybridhashjoin(Arg1S, Arg2S,
    attrname(Attr1), attrname(Attr2), 99997) :-
  optimizerOption(memoryAllocation),
	maUseNewTranslationRules(true),
  \+ optimizerOption(noHashjoin),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).

join00(Arg1S, Arg2S, pr(X = Y, _, _)) => hybridhashjoin(Arg2S, Arg1S,
    attrname(Attr2), attrname(Attr1), 99997) :-
  optimizerOption(memoryAllocation),
	maUseNewTranslationRules(true),
  \+ optimizerOption(noHashjoin),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).

join00(Arg1S, Arg2S, pr(X = Y, _, _)) => itHashJoin(Arg1S, Arg2S,
    attrname(Attr1), attrname(Attr2)) :-
  optimizerOption(memoryAllocation),
	maUseNewTranslationRules(true),
  \+ optimizerOption(noHashjoin),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).

join00(Arg1S, Arg2S, pr(X = Y, _, _)) => itHashJoin(Arg2S, Arg1S,
    attrname(Attr2), attrname(Attr1)) :-
  optimizerOption(memoryAllocation),
	maUseNewTranslationRules(true),
  \+ optimizerOption(noHashjoin),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).

% NVK ADDED MA END

/*
hashjoin keeps the order, if the size of the inner relation is
smaller than the size of the hash

*/

sortedjoin00(Arg1S, Arg2S, pr(X = Y, _, _), Arg1S, UpperBound) =>
        hashjoin(Arg1S, Arg2S, attrname(Attr1), attrname(Attr2), 99997)   :-
  not(optimizerOption(noHashjoin)),
  UpperBound < 99997,
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).

sortedjoin00(Arg1S, Arg2S, pr(X = Y, _, _), Arg2S, UpperBound) =>
        hashjoin(Arg2S, Arg1S, attrname(Attr2), attrname(Attr1), 99997)   :-
  not(optimizerOption(noHashjoin)),
  UpperBound < 99997,
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y).

/*
The following two rules are used for the ~adaptiveJoin~ extension. If used, the
previous two rules must be deactivated, inserting fail, as shown below.

*/

join00(Arg1S, Arg2S, pr(X = Y, Rel1, Rel2)) => pjoin2(Arg1S, Arg2S, Fields) :-
  optimizerOption(adaptiveJoin),
  try_pjoin2(X, Y, Rel1, Rel2, Fields).


  /*
  join00(Arg1S, Arg2S, pr(X = Y, _, _)) => pjoin2_smj(Arg1S, Arg2S, Fields) :-
  fail,
  try_pjoin2_smj(X, Y, Fields).


  join00(Arg1S, Arg2S, pr(X = Y, _, _)) => pjoin2_hj(Arg1S, Arg2S, Fields) :-
  fail,
  try_pjoin2_hj(X, Y, Fields).

  */



/*
Rules to create mergejoins with interesting orders extension

*/

join00(Arg1S, Arg2S, pr(X = Y, _, _))
  => mergejoin(Arg1S, Arg2S, attrname(Attr1), attrname(Attr2)) :-
  optimizerOption(intOrders(_)),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y),
  orderTest(mergejoinPossible).

join00(Arg1S, Arg2S, pr(X = Y, _, _))
  => sortLeftThenMergejoin(Arg1S, Arg2S, attrname(Attr1),
                                         attrname(Attr2)) :-
  optimizerOption(intOrders(_)),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y),
  orderTest(sortLeftThenMergejoin).

join00(Arg1S, Arg2S, pr(X = Y, _, _))
  => sortRightThenMergejoin(Arg1S, Arg2S, attrname(Attr1),
                                          attrname(Attr2)) :-
  optimizerOption(intOrders(_)),
  isOfFirst(Attr1, X, Y),
  isOfSecond(Attr2, X, Y),
  orderTest(sortRightThenMergejoin).

/*

----    isOfFirst(Attr?, X+, Y+)
        isOfSecond(Attr?, X+, Y+)
        isNotOfFirst(Attr?, X+, Y+)
        isNotOfSecond(Attr?, X+, Y+)
----

~Attr~ equal to either ~X~ or ~Y~ is an attribute of the first(second) relation.

*/

% Section:Start:translationRule_2_e
% Section:End:translationRule_2_e

% translation rule for sometimes(Pred). It is necessary for STPattern
indexselect(arg(N), pr(sometimes(InnerPred), _)) 
  => filter(ISL, sometimes(InnerPred)) :-
  indexselectLifted(arg(N), InnerPred)=> ISL.

% special rules for range queries in the form distance(m(x), y) < d
% 'distance <' with spatial(rtree,object) index

indexselectLifted(arg(N), distance(Y, attr(AttrName, Arg, AttrCase)) < D ) 
    => Res :-
  indexselectLifted(arg(N), distance(attr(AttrName, Arg, AttrCase), Y) < D ) 
    => Res.

indexselectLifted(arg(N), distance(attr(AttrName, Arg, AttrCase), Y) < D ) =>
  gettuples(windowintersectsS(dbobject(IndexName), 
    enlargeRect(bbox(Y),D,D)),  rel(Name, *))
  :-
  argument(N, rel(Name, *)),
  getTypeTree(Y, rel(Name, *), [_, _, T]),
  memberchk(T, [point, region]),
  hasIndex(rel(Name, _), 
    attr(AttrName, Arg, AttrCase), DCindex, spatial(rtree,object)),
  dcName2externalName(DCindex,IndexName).

indexselectLifted(arg(N), distance(attr(AttrName, Arg, AttrCase), Y) < D ) =>
  rename(gettuples(windowintersectsS(dbobject(IndexName),
  enlargeRect(bbox(Y),D,D)),  rel(Name, *)), RelAlias)
  :-
  argument(N, rel(Name, RelAlias)), RelAlias \= *,
  getTypeTree(Y, rel(Name, RelAlias), [_, _, T]),
  memberchk(T, [point, region]),
  hasIndex(rel(Name, _), 
    attr(AttrName, Arg, AttrCase), DCindex, spatial(rtree,object)),
  dcName2externalName(DCindex,IndexName).

% 'distance <' with spatial(rtree,unit) index
indexselectLifted(arg(N), distance(attr(AttrName, Arg, AttrCase), Y) < D ) =>
  gettuples(rdup(sort(windowintersectsS(dbobject(IndexName),
  enlargeRect(bbox(Y),D,D)))),  rel(Name, *))
  :-
  argument(N, rel(Name, *)),
  getTypeTree(Y, rel(Name, *), [_, _, T]),
  memberchk(T, [point, region]),
  hasIndex(rel(Name,_), 
    attr(AttrName,Arg,AttrCase), DCindex, spatial(rtree,unit)),
  dcName2externalName(DCindex,IndexName).

indexselectLifted(arg(N), distance(attr(AttrName, Arg, AttrCase), Y) < D ) =>
  rename(gettuples(rdup(sort(windowintersectsS(dbobject(IndexName),
  enlargeRect(bbox(Y),D,D)))),rel(Name, *)), RelAlias)
  :-
  argument(N, rel(Name, RelAlias)), RelAlias \= *,
  getTypeTree(Y, rel(Name, RelAlias), [_, _, T]),
  memberchk(T, [point, region]),
  hasIndex(rel(Name, _), 
    attr(AttrName, Arg, AttrCase), DCindex, spatial(rtree,unit)),
  dcName2externalName(DCindex,IndexName).


% general rules for liftedSpatialRangeQueries
% spatial(rtree,object) index, no rename

indexselectLifted(arg(N), Pred ) =>
  gettuples(windowintersectsS(dbobject(IndexName), BBox),  rel(Name, *))
  :-
  Pred =..[Op, Arg1, Arg2],
  ((Arg1 = attr(_, _, _), Attr= Arg1) ; (Arg2 = attr(_, _, _), Attr= Arg2)),
  argument(N, rel(Name, *)),
  getTypeTree(Arg1, _, [_, _, T1]),
  getTypeTree(Arg2, _, [_, _, T2]),
  isLiftedSpatialRangePred(Op, [T1,T2]),
  ((memberchk(T1, [rect, rect2, region, point, line, points, sline]), 
      BBox= bbox(Arg1));
   (memberchk(T2, [rect, rect2, region, point, line, points, sline]), 
      BBox= bbox(Arg2))),
  hasIndex(rel(Name, _), Attr, DCindex, spatial(rtree,object)),
  dcName2externalName(DCindex,IndexName).

% spatial(rtree,unit) index, no rename
indexselectLifted(arg(N), Pred ) =>
  gettuples(rdup(sort(windowintersectsS(dbobject(IndexName), BBox))),  
    rel(Name, *))
  :-
  Pred =..[Op, Arg1, Arg2],
  ((Arg1 = attr(_, _, _), Attr= Arg1) ; (Arg2 = attr(_, _, _), Attr= Arg2)),
  argument(N, rel(Name, *)),
  getTypeTree(Arg1, _, [_, _, T1]),
  getTypeTree(Arg2, _, [_, _, T2]),
  isLiftedSpatialRangePred(Op, [T1,T2]),
  ((memberchk(T1, [rect, rect2, region, point, line, points, sline]), 
     BBox= bbox(Arg1));
   (memberchk(T2, [rect, rect2, region, point, line, points, sline]), 
     BBox= bbox(Arg2))),
  hasIndex(rel(Name, _), Attr, DCindex, spatial(rtree,unit)),
  dcName2externalName(DCindex,IndexName).

% spatial(rtree,object) index, rename
indexselectLifted(arg(N), Pred ) =>
  rename(gettuples(windowintersectsS(dbobject(IndexName),
  BBox),  rel(Name, *)), RelAlias)
  :-
  Pred =..[Op, Arg1, Arg2],
  ((Arg1 = attr(_, _, _), Attr= Arg1) ; (Arg2 = attr(_, _, _), Attr= Arg2)),
  argument(N, rel(Name, RelAlias)), RelAlias \= *,
  getTypeTree(Arg1, _, [_, _, T1]),
  getTypeTree(Arg2, _, [_, _, T2]),
  isLiftedSpatialRangePred(Op, [T1,T2]),
  ((memberchk(T1, [rect, rect2, region, point, line, points, sline]), 
    BBox= bbox(Arg1));
   (memberchk(T2, [rect, rect2, region, point, line, points, sline]), 
    BBox= bbox(Arg2))),
  hasIndex(rel(Name, _), Attr, DCindex, spatial(rtree,object)),
  dcName2externalName(DCindex,IndexName).

% spatial(rtree,unit) index, rename
indexselectLifted(arg(N), Pred ) =>
  rename(gettuples(rdup(sort(windowintersectsS(dbobject(IndexName),
  BBox))),  rel(Name, *)), RelAlias)
  :-
  Pred =..[Op, Arg1, Arg2],
  ((Arg1 = attr(_, _, _), Attr= Arg1) ; (Arg2 = attr(_, _, _), Attr= Arg2)),
  argument(N, rel(Name, RelAlias)), RelAlias \= *,
  getTypeTree(Arg1, _, [_, _, T1]),
  getTypeTree(Arg2, _, [_, _, T2]),
  isLiftedSpatialRangePred(Op, [T1,T2]),
  ((memberchk(T1, [rect, rect2, region, point, line, points, sline]), 
     BBox= bbox(Arg1));
   (memberchk(T2, [rect, rect2, region, point, line, points, sline]), 
     BBox= bbox(Arg2))),
  hasIndex(rel(Name, _), Attr, DCindex, spatial(rtree,unit)),
  dcName2externalName(DCindex,IndexName).


% general rules for liftedEqualityQueries
% constuni(btree) index, no rename
indexselectLifted(arg(N), Pred ) =>
  gettuples(rdup(sort(exactmatchS(dbobject(IndexName), rel(Name, *), Y))), 
    rel(Name, *))
  :-
  Pred =..[Op, Arg1, Arg2],
  ((Arg1 = attr(_, _, _), Attr= Arg1) ; (Arg2 = attr(_, _, _), Attr= Arg2)),
  argument(N, rel(Name, *)),
  getTypeTree(Arg1, _, [_, _, T1]),
  getTypeTree(Arg2, _, [_, _, T2]),
  isLiftedEqualityPred(Op, [T1,T2]),
  ((memberchk(T1, [int, string, bool]), Y= Arg1);
   (memberchk(T2, [int, string, bool]), Y= Arg2)),
  hasIndex(rel(Name, _), Attr, DCindex, constuni(btree)),
  dcName2externalName(DCindex,IndexName).

% constuni(btree) index, rename
indexselectLifted(arg(N), Pred ) =>
  rename( gettuples(rdup(sort(exactmatchS(dbobject(IndexName), 
    rel(Name, Var), Y))), rel(Name, Var)), Var)
  :-
  Pred =..[Op, Arg1, Arg2],
  ((Arg1 = attr(_, _, _), Attr= Arg1) ; (Arg2 = attr(_, _, _), Attr= Arg2)),
  argument(N, rel(Name, Var)), Var \= * ,
  getTypeTree(Arg1, _, [_, _, T1]),
  getTypeTree(Arg2, _, [_, _, T2]),
  isLiftedEqualityPred(Op, [T1,T2]),
  ((memberchk(T1, [int, string, bool]), Y= Arg1);
   (memberchk(T2, [int, string, bool]), Y= Arg2)),
  hasIndex(rel(Name, _), Attr, DCindex, constuni(btree)),
  dcName2externalName(DCindex,IndexName).

% general rules for liftedRangeQueries
% constuni(btree) index, no rename
indexselectLifted(arg(N), Pred ) =>
  gettuples(rdup(sort(rangeS(dbobject(IndexName), rel(Name, *), Arg2 , Arg3))), 
    rel(Name, *))
  :-
  Pred =..[Op, Arg1, Arg2, Arg3],
  Arg1 = attr(_, _, _),
  argument(N, rel(Name, *)),
  getTypeTree(Arg1, _, [_, _, T1]),
  getTypeTree(Arg2, _, [_, _, T2]),
  getTypeTree(Arg3, _, [_, _, T3]),
  isLiftedRangePred(Op, [T1,T2,T3]),
  hasIndex(rel(Name, _), Arg1, DCindex, constuni(btree)),
  dcName2externalName(DCindex,IndexName).

% constuni(btree) index, rename
indexselectLifted(arg(N), Pred ) =>
  rename(gettuples(rdup(sort(rangeS(dbobject(IndexName), 
    rel(Name, Var), Arg2 , Arg3))), rel(Name, Var)), Var)
  :-
  Pred =..[Op, Arg1, Arg2, Arg3],
  Arg1 = attr(_, _, _),
  argument(N, rel(Name, Var)), Var \= * ,
  getTypeTree(Arg1, _, [_, _, T1]),
  getTypeTree(Arg2, _, [_, _, T2]),
  getTypeTree(Arg3, _, [_, _, T3]),
  isLiftedRangePred(Op, [T1,T2,T3]),
  hasIndex(rel(Name, _), Arg1, DCindex, constuni(btree)),
  dcName2externalName(DCindex,IndexName).

% general rules for liftedLeftRangeQueries
% constuni(btree) index, no rename
indexselectLifted(arg(N), Pred ) =>
  gettuples(rdup(sort(leftrangeS(dbobject(IndexName), rel(Name, *), Y))), 
    rel(Name, *))
  :-
  Pred =..[Op, Arg1, Arg2],
  ((Arg1 = attr(_, _, _), Attr= Arg1) ; (Arg2 = attr(_, _, _), Attr= Arg2)),
  argument(N, rel(Name, *)),
  getTypeTree(Arg1, _, [_, _, T1]),
  getTypeTree(Arg2, _, [_, _, T2]),
  isLiftedLeftRangePred(Op, [T1,T2]),
  ((memberchk(T1, [int, string, bool]), Y= Arg1);
   (memberchk(T2, [int, string, bool]), Y= Arg2)),
  hasIndex(rel(Name, _), Attr, DCindex, constuni(btree)),
  dcName2externalName(DCindex,IndexName).

% constuni(btree) index, rename
indexselectLifted(arg(N), Pred ) =>
  rename(gettuples(rdup(sort(leftrangeS(dbobject(IndexName), 
    rel(Name, Var), Y))), rel(Name, Var)), Var)
  :-
  Pred =..[Op, Arg1, Arg2],
  ((Arg1 = attr(_, _, _), Attr= Arg1) ; (Arg2 = attr(_, _, _), Attr= Arg2)),
  argument(N, rel(Name, Var)), Var \= * ,
  getTypeTree(Arg1, _, [_, _, T1]),
  getTypeTree(Arg2, _, [_, _, T2]),
  isLiftedLeftRangePred(Op, [T1,T2]),
  ((memberchk(T1, [int, string, bool]), Y= Arg1);
   (memberchk(T2, [int, string, bool]), Y= Arg2)),
  hasIndex(rel(Name, _), Attr, DCindex, constuni(btree)),
  dcName2externalName(DCindex,IndexName).

% general rules for liftedRightRangeQueries
% constuni(btree) index, no rename
indexselectLifted(arg(N), Pred ) =>
  gettuples(rdup(sort(rightrangeS(dbobject(IndexName), rel(Name, *), Y))), 
    rel(Name, *))
  :-
  Pred =..[Op, Arg1, Arg2],
  ((Arg1 = attr(_, _, _), Attr= Arg1) ; (Arg2 = attr(_, _, _), Attr= Arg2)),
  argument(N, rel(Name, *)),
  getTypeTree(Arg1, _, [_, _, T1]),
  getTypeTree(Arg2, _, [_, _, T2]),
  isLiftedRightRangePred(Op, [T1,T2]),
  ((memberchk(T1, [int, string, bool]), Y= Arg1);
   (memberchk(T2, [int, string, bool]), Y= Arg2)),
  hasIndex(rel(Name, _), Attr, DCindex, constuni(btree)),
  dcName2externalName(DCindex,IndexName).

% constuni(btree) index, rename
indexselectLifted(arg(N), Pred ) =>
  rename(gettuples(rdup(sort(rightrangeS(dbobject(IndexName), 
    rel(Name, Var), Y))), rel(Name, Var)), Var)
  :-
  Pred =..[Op, Arg1, Arg2],
  ((Arg1 = attr(_, _, _), Attr= Arg1) ; (Arg2 = attr(_, _, _), Attr= Arg2)),
  argument(N, rel(Name, Var)), Var \= * ,
  getTypeTree(Arg1, _, [_, _, T1]),
  getTypeTree(Arg2, _, [_, _, T2]),
  isLiftedRightRangePred(Op, [T1,T2]),
  ((memberchk(T1, [int, string, bool]), Y= Arg1);
   (memberchk(T2, [int, string, bool]), Y= Arg2)),
  hasIndex(rel(Name, _), Attr, DCindex, constuni(btree)),
  dcName2externalName(DCindex,IndexName).
  

%     isOfFirst(Res, X, Y)
% Returns X or Y, depending on which of them comes from the first stream
% argument (if both do, X comes first).
% Fails, iff none of X and Y are attributes from the first (left/ outer) stream.
isOfFirst(X, X, _) :- X = attr(_, 1, _).
isOfFirst(Y, _, Y) :- Y = attr(_, 1, _).

%     isOfSecond(Res, X, Y)
% Return X or Y, depending on which of them comes from the second stream
% argument  (if both do, X comes first).
% Fails, iff none of X and Y are attributes from the 2nd (right/ inner) stream.
isOfSecond(X, X, _) :- X = attr(_, 2, _).
isOfSecond(Y, _, Y) :- Y = attr(_, 2, _).

%    isNotOfFirst(Res, X, Y)
% Return the attribute (X or Y), that does not come from the first stream
% (X comes first)
isNotOfFirst(Y, X, Y) :- X = attr(_, 1, _).
isNotOfFirst(X, X, Y) :- Y = attr(_, 1, _).

%    isNotOfSecond(Res, X, Y)
% Return the attribute (X or Y), that does not come from the second stream
% (X comes first)
isNotOfSecond(Y, X, Y) :- X = attr(_, 2, _).
isNotOfSecond(X, X, Y) :- Y = attr(_, 2, _).

/*
Begin of Goehr's extension

The same as above, but for three attributes (return order: X, Y, Z).

*/

isOfFirst(X, X, _, _) :- X = attr(_, 1, _).
isOfFirst(Y, _, Y, _) :- Y = attr(_, 1, _).
isOfFirst(Z, _, _, Z) :- Z = attr(_, 1, _).


areNotOfFirst(X, Y, X, Y, _) :-
  X \= attr(_, 1, _),
  Y \= attr(_, 1, _).

areNotOfFirst(X, Z, X, _, Z) :-
  X \= attr(_, 1, _),
  Z \= attr(_, 1, _).

areNotOfFirst(Y, Z, _, Y, Z) :-
  Y \= attr(_, 1, _),
  Z \= attr(_, 1, _).

/*
----
fetchAttributeList(+Expr, ?AttrList)
----

Extracts a list of all ~attr/3~ terms occuring in ~Expr~.
The result list keeps the order of appearance of attr/3 terms
in ~Expr~.

*/
fetchAttributeList([],[]) :- !.

fetchAttributeList([T|L],R) :-
  fetchAttributeList(T,R1),
  fetchAttributeList(L,R2),
  append(R1, R2, R), !.

fetchAttributeList(attr(A,B,C),[attr(A,B,C)]) :- !.

fetchAttributeList(T,R) :-
  compound(T),
  \+ is_list(T),
  T =.. L,
  fetchAttributeList(L,R), !.

fetchAttributeList(A,[]) :-
  atom(A), !.

/*
End of Goehr's extension

*/


/*
6 Creating Query Plan Edges

*/

createPlanEdge :-
  edge(Source, Target, Term, Result, _, _),
  Term => Plan,
  assert(planEdge(Source, Target, Plan, Result)),
  fail.

createPlanEdges :-
  not(createPlanEdge),
  modifyPlanEdges.

deletePlanEdges :-
  retractall(planEdge(_, _, _, _)).

modifyPlanEdges :-
  optimizerOption(adaptiveJoin),
  deleteRegularJoinEdges.

modifyPlanEdges.

planEdgeInfoAll(Source, Target) :-
  write('Source: '), write(Source), nl,
  write('Target: '), write(Target), nl.

planEdgeInfo2(Plan, Result) :-
  write('Plan  : '), wp(Plan), nl,
  %  write('\t'), write(Plan), nl,
  write('Result: '), write(Result), nl, nl.


planEdgeInfo3(Source, Target, Plan, Result) :-
  planEdgeInfoAll(Source, Target),
  write('Term  : '), write(Plan), nl,
  write('----------------'), nl,
  planEdgeInfo2(Plan, Result).

writePlanEdges2:-
  planEdge(Source, Target, Plan, Result),
  planEdgeInfoAll(Source, Target),
  planEdgeInfo2(Plan, Result),
  nl,
  fail.

writePlanEdges3:-
  planEdge(Source, Target, Plan, Result),
  planEdgeInfo3(Source, Target, Plan, Result),
  nl,
  fail.

:-assert(helpLine(writePlanEdges,0,[],
                  'List executable sub plans for all POG edges.')).
:-assert(helpLine(writePlanEdgesX,0,[],
                  'List executable and internal sub plans for all POG edges.')).

writePlanEdges :- not(writePlanEdges2).

writePlanEdgesX :- not(writePlanEdges3).




/*
7 Assigning Sizes and Selectivities to the Nodes and Edges of the POG

----    assignSizes
----

Assign sizes (numbers of tuples) to all nodes in the pog, based on the
cardinalities of the argument relations and the selectivities of the
predicates. Store sizes as facts of the form resultSize(Result, Size). Store
selectivities as facts of the form edgeSelectivity(Source, Target, Sel).

Delete sizes from memory.

7.1 Assigning Sizes, Tuple Sizes, Selectivities and Predicate Costs

It is important that edges are processed in the order in which they have been
created. This will ensure that for an edge the size of its argument nodes are
available.

*/

assignSizes :-
  optimizerOption(correlations),
  not(assignSizes1),
  addCorrelationSizes,
  !.

assignSizes :-
  not(optimizerOption(correlations)),
  not(assignSizes1),
  !.


assignSizes1 :-
  edge(Source, Target, Term, Result, _, _),
  assignSize(Source, Target, Term, Result),
  fail.

% Versions for A. Nawra's and improved cost functions:
% Annotates tuple sizes to nodes and predicates to edges
%  - assignSize(+Source, +Target, +Term, -Result)
assignSize(Source, Target, select(Arg, Pred), Result) :-
  ( optimizerOption(nawracosts) ; optimizerOption(improvedcosts) ;
  	optimizerOption(memoryAllocation) ), % NVK MODIFIED MA
  resSize(Arg, Card),
  resTupleSize(Arg, TupleSize),
  selectivity(Pred, Sel, BBoxSel, CalcPET, ExpPET),
  Size is Card * Sel,
  !,
  setNodeSize(Result, Size),
  setNodeTupleSize(Result, TupleSize),
  setPredNoPET(Source, Target, CalcPET, ExpPET),
  assert(edgeSelectivity(Source, Target, Sel)),
  assert(edgeInputCards(Source, Target, Arg, undefined)),
  assert(edgeInfoProgress(Source, Target, BBoxSel, ExpPET)),
  !.

assignSize(Source, Target, join(Arg1, Arg2, Pred), Result) :-
  ( optimizerOption(nawracosts) ; optimizerOption(improvedcosts) ;
  	optimizerOption(memoryAllocation) ), % NVK MODIFIED MA
  resSize(Arg1, Card1),
  resSize(Arg2, Card2),
  resTupleSize(Arg1, TupleSize1),
  resTupleSize(Arg2, TupleSize2),
  selectivity(Pred, Sel, BBoxSel, CalcPET, ExpPET),
  Size is Card1 * Card2 * Sel,
  addSizeTerms([TupleSize1,TupleSize2],TupleSize),
  !,
  setNodeSize(Result, Size),
  setNodeTupleSize(Result, TupleSize),
  setPredNoPET(Source, Target, CalcPET, ExpPET),
  assert(edgeSelectivity(Source, Target, Sel)),
  assert(edgeInputCards(Source, Target, Arg1, Arg2)),
  assert(edgeInfoProgress(Source, Target, BBoxSel, ExpPET)),
  !.

/*
NVK ADDED MA
Currently not used, it was just a hack to integrate the sort operation
into the pog optimization. But because there are no CostEstimation 
implementations for the sort operation available, this is not needed.

*/
assignSize(Source, Target, sortby(Arg1, _), Result) :-
  optimizerOption(memoryAllocation),
  resSize(Arg1, Card1),
  resTupleSize(Arg1, TupleSize1),
  Sel = 1, % Here we have to predicate, so there is no selection, every
  % row is processed.
  Size is Card1 * Sel,
  !,
  setNodeSize(Result, Size),
  setNodeTupleSize(Result, TupleSize1),
  %setPredNoPET(Source, Target, 1, 1),
  assert(edgeSelectivity(Source, Target, Sel)),
  assert(edgeInputCards(Source, Target, Card1, Card1)),
  % Just insert this with some values, there is no predicate here
  % so there are no selctivity queries to determine the times.
  assert(edgeInfoProgress(Source, Target, 1, 1)),
  !.
% NVK ADDED MA END


% Versions for standard cost functions:

assignSize(Source, Target, select(Arg, Pred), Result) :-
  not( optimizerOption(nawracosts) ),
  not( optimizerOption(improvedcosts) ), % standard cost functions
  \+ optimizerOption(memoryAllocation), % NVK ADDED MA
  resSize(Arg, Card),
  selectivity(Pred, Sel, BBoxSel, _, ExpPET),
  Size is Card * Sel,
  setNodeSize(Result, Size),
  assert(edgeSelectivity(Source, Target, Sel)),
  assert(edgeInputCards(Source, Target, Arg, undefined)),
  assert(edgeInfoProgress(Source, Target, BBoxSel, ExpPET)),
  !.

assignSize(Source, Target, join(Arg1, Arg2, Pred), Result) :-
  not(  ( optimizerOption(nawracosts) ; optimizerOption(improvedcosts) ) ),
  \+ optimizerOption(memoryAllocation), % NVK ADDED MA
  resSize(Arg1, Card1),
  resSize(Arg2, Card2),
  selectivity(Pred, Sel, BBoxSel, _, ExpPET),
  Size is Card1 * Card2 * Sel,
  setNodeSize(Result, Size),
  assert(edgeSelectivity(Source, Target, Sel)),
  assert(edgeInputCards(Source, Target, Arg1, Arg2)),
  assert(edgeInfoProgress(Source, Target, BBoxSel, ExpPET)),
  !.

assignSize(Source, Target, sortedjoin(Arg1, Arg2, Pred, _, _), Result) :-
  assignSize(Source, Target, join(Arg1, Arg2, Pred), Result).

/*
----    setNodeSize(Node, Size) :-
----

Set the size of node ~Node~ to ~Size~ if no size has been assigned before.

*/

:- dynamic resultSize/2.

setNodeSize(Node, _) :- resultSize(Node, _), !.
setNodeSize(Node, Size) :- assert(resultSize(Node, Size)).

/*
----    resSize(Arg, Size) :-
----

Argument ~Arg~ has size ~Size~.

*/

resSize(arg(N), Size) :-
  argument(N, rel(Rel, _)),
  card(Rel, Size), !.
resSize(arg(N), _) :-
  write('Error in optimizer: cannot find cardinality for '),
  argument(N, Rel), wp(Rel), nl, fail.
resSize(res(N), Size) :- resultSize(N, Size), !.


/*
Assign tuple sizes to a node. Tuple sizes are saved as facts of the form
~nodeTupleSize(Node, sizeTerm(MemSize, CoreSize, LobSize))~.

*/

:- dynamic nodeTupleSize/2.
:- dynamic storedPredNoPET/3.

setNodeTupleSize(Node, _) :-
  nodeTupleSize(Node, _), !.

setNodeTupleSize(Node, TupleSize) :-
  assert(nodeTupleSize(Node, TupleSize)).

/*
Get the size of one single tuple from argument number ~N~.

*/

/*
NVK ADDED NR
Return saved sizes.

*/
resTupleSize(arg(N), SizeTerm) :-
  optimizerOption(nestedRelations),
  argument(N, rel(irrel(_, _, _, _, SizeTerm, _, _), _)),
  !.
% NVK ADDED NR END

resTupleSize(arg(N), TupleSize) :-
 argument(N, rel(Rel, _)),
 tupleSizeSplit(Rel, TupleSize), % should also reflect initial projections
 !.

resTupleSize(res(N), TupleSize) :-
  nodeTupleSize(N, TupleSize), !.

resTupleSize(X, Y) :-
  term_to_atom(X, XA),
  concat_atom(['Cannot find tuplesize for \'',XA,'\'.'],
              '',ErrMsg),
  write_list(['ERROR in optimizer: ',ErrMsg]), nl,
  throw(error_Internal(optimizer_resTupleSize(X,Y)::missingData::ErrMsg)),
  fail, !.


/*
Save and query Predicate Evalation Times (PETs) indexed by predicate number or
by Source and Target node numbers.

*/

setPredNoPET(Source, Target, CalcPET, ExpPET) :-
  Index is Target - Source,
  setPredNoPET(Index, CalcPET, ExpPET), !.

setPredNoPET(Index, _, _) :-
  storedPredNoPET(Index, _, _), !.

setPredNoPET(Index, CalcPET, ExpPET) :-
  assert(storedPredNoPET(Index, CalcPET, ExpPET)), !.

getPredNoPET(Source, Target, CalcPET, ExpPET) :-
  Index is Target - Source,
  getPredNoPET(Index, CalcPET, ExpPET), !.

getPredNoPET(Index, CalcPET, ExpPET) :-
  storedPredNoPET(Index, CalcPET, ExpPET), !.

getPredNoPET(Index, X, Y) :-
  concat_atom(['Cannot find annotated PET.'],'',ErrMsg),
  throw(error_Internal(optimizer_getPredNoPET(Index, X, Y)
        ::missingData::ErrMsg)),
  fail, !.

/*
----    writeSizes/0
        writeNodeSizes/0
        writeEdgeSels/0
----

Clauses for writing sizes and selectivities for nodes and edges.

*/

:-assert(helpLine(writeSizes,0,[],
      'List estimated cardinalities and selectivities for the current POG.')).
:-assert(helpLine(writeNodeSizes,0,[],
      'List estimated cardinalities for the current POG.')).
:-assert(helpLine(writeEdgeSels,0,[],
      'List estimated selectivities for the current POG.')).

writeSizes :-
  writeNodeSizes,
  writeEdgeSels.

writeNodeSizes :-
  findall([Node, Size], resultSize(Node, Size), L),
  Format = [ ['Node', 'l'],
             ['Size', 'l'] ],
  showTuples(L, Format).

edgeSelInfo(Source, Target, Sel, Pred) :-
 edgeSelectivity(Source, Target, Sel),
 edge(Source, Target, join(_, _, pr(Pred, _, _)), _, _, _).
 %plan_to_atom(P, Pred).

edgeSelInfo(Source, Target, Sel, Pred) :-
 edgeSelectivity(Source, Target, Sel),
 edge(Source, Target, sortedjoin(_, _, pr(Pred, _, _), _, _), _, _, _).
 %plan_to_atom(P, Pred).

edgeSelInfo(Source, Target, Sel, Pred) :-
 edgeSelectivity(Source, Target, Sel),
 edge(Source, Target, select(_, pr(Pred, _)), _, _, _).
 %plan_to_atom(P, Pred).


writeEdgeSels :-
 findall([Source-Target, Sel, Pred], edgeSelInfo(Source, Target, Sel, Pred), L),
  Format = [ ['Edge', 'l'],
             ['Selectivity', 'l'],
             ['Predicate', 'l'] ],
  showTuples(L, Format).


/*
The dynamic predicates below are used to display
the edge selectivites and estimated sizes of the
last computed best plan which is stored in ~path/1~.

Moreover if option ~useCounters~ is switched on, the real
sizes for the POG-nodes traversed by the path are computed.

After a query was processed one can investigate the plan by using
the predicates ~explainPlan/0~, ~checkSizes/0~ or ~showPredOrder/0~.

*/

:- dynamic pathInfo/10,
           observedNodeSizes/2,
           nodeSizeCounter/4,
           nodeSizeCounter/2,
           path/1.


sizeAndSel(Src, Tgt, Sel, Size) :-
  costEdge(Src, Tgt, _, _, Size, _),
  edgeSelectivity(Src,Tgt, Sel).

getSize(undefined, 1).
getSize(Arg, Card) :- resSize(Arg, Card).

getRealSize(undefined, 1).
getRealSize(res(N), S) :- getRealSize(N, S).
getRealSize(arg(N), S) :- resSize(arg(N), S).

getRealSize(ResNode, SizeReal) :-
  nodeSizeCounter(ResNode, SizeReal).

getRealSize(_, -1).


createPathInfo([H|T]) :-
  H = costEdge(Src, Tgt, _, ResNode, SizeEst, _),
  edgeSelectivity(Src, Tgt, SelEst),
  edgeInputCards(Src, Tgt, A1, A2),
  getSize(A1, C1),
  getSize(A2, C2),
  getRealSize(ResNode, SizeReal),
  SizeEstInt is ceil(SizeEst),
  C1c is ceil(C1),
  C2c is ceil(C2),
  getRealSize(A1, C1Real),
  getRealSize(A2, C2Real),
  SelReal is SizeReal / max((C1Real * C2Real),1),
  %showValue('SelEst: ', SelEst),
  %showValue('SelReal: ', SelReal),
  relativeError(SelEst, SelReal, SelErr),
  %showValue('SizeEstInt: ', SizeEstInt),
  %showValue('SelReal: ', SizeReal),
  relativeError(SizeEstInt, SizeReal, SizeErr),
  assert(pathInfo(Src, Tgt, C1c, C2c, SelEst, SelReal, SelErr, SizeEstInt,
                                                          SizeReal, SizeErr)),
  createPathInfo(T).

createPathInfo([]).

relativeError(A, B, Err) :-
  B > 0, A >= B,
  Err is round((A / B) * 100 - 100),
  %showValue('err: ', Err),
  !.

relativeError(A, B, Err) :-
  A > 0, B >= A,
  Err is round((B / A) * 100 - 100),
  %showValue('err: ', Err),
  !.

relativeError(_, 0, inf).
relativeError(0, _, inf).



getSizes(L, Format) :-
  optimizerOption(useCounters),
  computeNodeSizes, !,
  findall([Src-Tgt, C1, C2, Sel, SelErr, SelReal, SzEst, SzReal, SizeErr],
  pathInfo(Src, Tgt, C1, C2, Sel, SelErr, SelReal, SzEst, SzReal, SizeErr), L),
  Format = [ ['Edge', 'l'],
             ['Arg1', 'l'],
      ['Arg2', 'l'],
      ['Sel-Est', 'l'],
      ['Sel-Real', 'l'],
      ['E1', 'l'],
             ['Sz-Est', 'l'],
             ['Sz-Real', 'l'],
             ['E2', 'l']
             ].


checkSizes :-
  getSizes(L, Format),
  current_prolog_flag(float_format, FF),
  set_prolog_flag(float_format, '%.7f'),
  showTuples(L, Format),
  set_prolog_flag(float_format, FF).



computeNodeSizes :-
 pathInfo(_,_,_,_,_,_,_,_,_,_).

computeNodeSizes :-
  optimizerOption(useCounters),
  computeObservedNodeSizes, !,
  path(P), createPathInfo(P).

computeNodeSizes :-
  path(P), createPathInfo(P).

createObservedNodeSizes([]).
createObservedNodeSizes([ [Nc,Value] | T ]) :-
  nodeSizeCounter(Nc, _, _, Result ),
  assert(nodeSizeCounter(Result, Value)),
  createObservedNodeSizes( T ).

computeObservedNodeSizes :-
  retractall(observedNodeSizes(_,_)),
  secondo('list counters', C ), !,
  getCounter(nodeSizeCtr, Nc),
  %Last is Nc - 1,
  %showValue('C', C),
  subList(C, Nc, L),
  %showValue('Nc', Nc),
  %showValue('L', L),
  createObservedNodeSizes( L ).


getPredOrder(L, Format) :-
  findall([Src-Tgt, Op, Pred], edgePredicate(Src, Tgt, Pred, Op), L),
  Format = [ ['Edge',    'l'],
      ['Operator   ', 'l'],
             ['Predicate',   'l']
             ].

showPredOrder :-
  getPredOrder(L, Format),
  showTuples(L, Format).

edgePredicate(Source, Target, PlanFragment, Op) :-
  pathInfo(Source, Target, _, _, _, _, _, _, _, _),
  path(X),
  %showValue('X:', X),
  member(costEdge(Source,Target,Term,_,_,_),X),
  firstOp(Term, Op),
  %write('F1:'),write(F1), nl, write(F2), nl,
  edgeSelInfo(Source, Target, _, PlanFragment).


firstOp(Term, F1) :-
  Term =.. [ F1 | [ Arg1, _] ],
  Arg1 =.. [ _ | _ ], !.

firstOp(Term, F1) :-
  Term =.. [ F1 | _ ].


explainPlan :-
  checkSizes, showPredOrder.


/*
----    deleteSizes :-
----

Delete node sizes and selectivities of edges.

*/

deleteSizes :-
  retractall(resultSize(_, _)),
  retractall(edgeSelectivity(_, _, _)),
  retractall(edgeInputCards(_, _, _, _)),
  retractall(edgeInfoProgress(_, _, _, _)),
  retractall(nodeTupleSize(_, _)),
  retractall(nodeAttributeList(_, _)),
  retractall(nodeSizeCounter(_, _, _, _)),
  retractall(nodeSizeCounter(_, _)),
  retractall(pathInfo(_, _, _, _, _, _, _, _, _, _)),
  retractall(path(_)),
  retractall(tmpStoredTypeTree(_,_)).

deleteSizesNawra :-
  retractall(storedPredNoPET(_, _, _)).


/*
8 Computing Edge Costs for Plan Edges

8.1 The Costs of Terms

----    cost(+Term, +Sel, +Pred, -Size, -Cost)
----

The cost of an executable ~Term~ representing a predicate ~Pred~ with
selectivity ~Sel~ is ~Cost~ and the size of the result is ~Size~.

This is evaluated recursively descending into the term. When the operator
realizing the predicate (e.g. ~filter~) is encountered, the selectivity ~Sel~ is
used to determine the size of the result.
It is assumed that only a single operator of this kind occurs within the term.


8.1.1 Arguments

*/

/*
NVK ADDED NR
Added because the dcName2internalName evaluation of the below predicate fails for non atomic values. Note that if other cost models should support the nested relations, this cost predicates must be added first.

*/
cost(rel(T, _), _, _, Card, 0) :-
  optimizerOption(nestedRelations),
  T=irrel(_, _, _, Card, _, _, _).

% NVK ADDED NR END

% the if-then-else-part  is just for error-detection --- FIXME!
cost(rel(Rel, X1_), X2_, Pred_, Size, 0) :-
  dcName2internalName(RelDC,Rel),
  ( Rel = RelDC
    -> true
    ;  (
         write('ERROR:\tcost/4 failed due to non-dc relation name.'), nl,
         write('---> THIS SHOULD BE CORRECTED!'), nl,
         throw(error_Internal(optimizer_cost(rel(Rel, X1_, X2_, Pred_, Size, 0)
              ::malformedExpression))),
         fail
       )
  ),
  card(Rel, Size).

cost(res(N), _, _, Size, 0) :-
  resultSize(N, Size).

/*
8.1.2 Operators

*/

cost(feed(X), Sel, P, S, C) :-
  cost(X, Sel, P, S, C1),
  feedTC(A),
  C is C1 + A * S.

/*
NVK ADDED NR
I didn't analyzed the cost of the nested relations operators, hence, the same costs as for the regular feed operator are assumed. Note that the aspect of cost estimation wasn't looked at all under the topic "nested relations".

*/
cost(afeed(X), Sel, P, S, C) :-
  cost(X, Sel, P, S, C1),
  feedTC(A),
  C is C1 + A * S.

cost(aconsume(X), Sel, P, S, C) :-
  cost(X, Sel, P, S, C1),
  consumeTC(A),
  C is C1 + A * S.

/*
This is now needed, but i don't have any costs models for this operations so i just make this work with no costs.

*/
cost(unnest(X, _), Sel, P, S, C) :-
  cost(X, Sel, P, S, C).
cost(nest(X, _, _), Sel, P, S, C) :-
  cost(X, Sel, P, S, C).
cost(groupby(X, _, _), Sel, P, S, C) :-
  cost(X, Sel, P, S, C).
cost(transformstream(X), Sel, P, S, C) :-
  cost(X, Sel, P, S, C).
cost(namedtransformstream(X, _), Sel, P, S, C) :-
  cost(X, Sel, P, S, C).
cost(predinfo(X, _, _), Sel, P, S, C) :-
  cost(X, Sel, P, S, C).
cost(renameattr(X, _), Sel, P, S, C) :-
  cost(X, Sel, P, S, C).
cost(extendstream(X, _), Sel, P, S, C) :-
  cost(X, Sel, P, S, C).
cost(count(X), Sel, P, S, C) :-
  cost(X, Sel, P, S, C).

% See nr.pl:nrLookupRel
cost(attribute(_, _), _Sel, _P, 0, 0).
cost(a(_, _, _), _Sel, _P, 0, 0).

% NVK ADDED NR END


/*
Here ~feedTC~ means ``feed tuple cost'', i.e., the cost per tuple, a constant to
be determined in experiments. These constants are kept in file ``operators.pl''.

*/


cost(feedproject(X, _), Sel, P, S, C) :-
  cost(X, Sel, P, S, C1),
  feedTC(A),
  C is C1 + A * S.



cost(consume(X), Sel, P, S, C) :-
  cost(X, Sel, P, S, C1),
  consumeTC(A),
  C is C1 + A * S.

/*
For ~filter~, there are several special cases to distinguish:

  1 ~filter(spatialjoin(...), P)~

  2 ~filter(gettuples(...), P)~

  3 ~filter(windowintersects(...), P)~

  4 ``normal'' ~filter(...)~

For the first three cases, the edge is the translation of a spatial predicate,
that makes use of bounding box checks. The first argument of filter will already
reduce the set of possible candidates, so that the cardinality of tuples
processed by filter will be smaller than the cardinality passed down in the 3rd
argument of ~cost~. Also, the selectivity passed with the second argument of
~cost~ is the ~total~ selectivity. To get the selectivity of the preselection,
one can analyse the predicate and lookup the table ~storedBBoxSel/3~ for that
selectivity, which should be passed to the recursive call of ~cost~.

PROBLEM: What happens with the entropy-optimizer? As for cases 2 + 3, there is
no problem, as the index is used to create a fresh tuple stream. But, as for
case 1, we might get into problems, as the selectivity of the bbox-check depends
on earlier predicates - so we should consider both selectivities in the
minimization of the entropy.

*/

cost(filter(X, _), Sel, P, S, C) :-
  isPrefilter(X),  % holds for spatialjoin or loopjoin
                   % isPrefilter defindad after cost clauses.

%  X = spatialjoin(_, _, attrname(attr(Attr1, ArgNr1, Case1)),
%                        attrname(attr(Attr2, ArgNr2, Case2))),
%  getSimplePred(P, PSimple),
%  databaseName(DB),
%  storedBBoxSel(DB, PSimple, BBoxSel),
%  cost(X, BBoxSel, _, SizeX, CostX),

  cost(X, Sel, P, SizeX, CostX),
  filterTC(A),
  S is SizeX,
  C is CostX + A * SizeX, !.

% Section:Start:cost_5_m
%cost(filter(gettuples(rdup(sort(
%      windowintersectsS(dbobject(IndexName), BBox))), rel(RelName, *)),
%      FilterPred), Sel, _, Size, Cost):-
%  dm(costFunctions,['cost(filter(gettuples(rdup(sort(windowintersectsS(...): ',
%                    'IndexName= ',IndexName,', BBox=',BBox,
%                    ', FilterPred=',FilterPred]),
%  Cost is 0,
%  card(RelName, RelCard),
%  Size is RelCard * Sel,!.
% Section:End:cost_5_m

cost(filter(X, _), Sel, P, S, C) :-  % 'normal' filter
  cost(X, 1, P, SizeX, CostX),
  getPET(P, _, ExpPET),            % fetch stored predicate evaluation time
  filterTC(A),
  S is SizeX * Sel,
  C is CostX + SizeX * (A + ExpPET).
  %C is CostX.

cost(product(X, Y), _, P, S, C) :-
  cost(X, 1, P, SizeX, CostX),
  cost(Y, 1, P, SizeY, CostY),
  productTC(A, B),
  S is SizeX * SizeY,
  C is CostX + CostY + SizeY * B + S * A.

cost(leftrange(_, Rel, _), Sel, P, Size, Cost) :-
  cost(Rel, 1, P, RelSize, _),
  leftrangeTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

cost(leftrangeS(dbobject(Index), _KeyValue), Sel, _P, Size, Cost) :-
  dcName2externalName(DcIndexName,Index),
  getIndexStatistics(DcIndexName, noentries, _DCrelName, RelSize),
  leftrangeTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C * 0.25 . % balance of 75% is for gettuples

cost(rightrange(_, Rel, _), Sel, P, Size, Cost) :-
  cost(Rel, 1, P, RelSize, _),
  leftrangeTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

cost(rightrangeS(dbobject(Index), _KeyValue), Sel, _P, Size, Cost) :-
  dcName2externalName(DcIndexName,Index),
  getIndexStatistics(DcIndexName, noentries, _DCrelName, RelSize),
  leftrangeTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C * 0.25 . % balance of 75% is for gettuples

cost(range(_, Rel, _), Sel, P, Size, Cost) :-
  cost(Rel, 1, P, RelSize, _),
  exactmatchTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

cost(rangeS(dbobject(Index), _KeyValue), Sel, _P, Size, Cost) :-
  dcName2externalName(DcIndexName,Index),
  getIndexStatistics(DcIndexName, noentries, _DCrelName, RelSize),
  exactmatchTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C * 0.25 . % balance of 75% is for gettuples

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
cost(exactmatchfun(_, Rel, _), Sel, P, Size, Cost) :-
  cost(Rel, 1, P, RelSize, _),
  exactmatchTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

cost(exactmatch(_, Rel, _), Sel, P, Size, Cost) :-
  cost(Rel, 1, P, RelSize, _),
  exactmatchTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

cost(exactmatchS(dbobject(Index), _KeyValue), Sel, _P, Size, Cost) :-
  dcName2externalName(DcIndexName,Index),
  getIndexStatistics(DcIndexName, noentries, _DCrelName, RelSize),
  exactmatchTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C * 0.25 . % balance of 75% is for gettuples

cost(loopjoin(X, Y), Sel, P, S, Cost) :-
  cost(X, 1, P, SizeX, CostX),
  cost(Y, Sel, P, SizeY, CostY),
  S is SizeX * SizeY,
  loopjoinTC(C),
  Cost is C * SizeX + CostX + SizeX * CostY.

cost(fun(_, X), Sel, P, Size, Cost) :-
  cost(X, Sel, P, Size, Cost).



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
cost(hashjoin(X, Y, _, _, NBuckets), Sel, P, S, C) :-
  cost(X, 1, P, SizeX, CostX),
  cost(Y, 1, P, SizeY, CostY),
  hashjoinTC(A, B),
  S is SizeX * SizeY * Sel,
  %showValue('SizeX', SizeX),
  %showValue('SizeY', SizeY),
  %showValue('CostX', CostX),
  %showValue('CostY', CostY),
  H is                                          % producing the arguments
    A * NBuckets * (SizeX/NBuckets + 1) *       % computing the product for each
      (SizeY/NBuckets +1) +                     % pair of buckets
    B * S,
  %showValue('Hashcost', H),
  C is CostX + CostY + H.                       % producing the result tuples


cost(sort(X), Sel, P, S, C) :-
  cost(X, Sel, P, SizeX, CostX),
  sortmergejoinTC(A, _),
  S is SizeX,
  C is CostX +                                  % producing the argument
    A * SizeX * log(SizeX + 1).                 % sorting the arguments
             %   individual cost of ordering predicate still not applied!


% Sortby with empty sorting list is ignored:
cost(sortby(X, []), Sel, P, S, C) :-
  cost(X, Sel, P, S, C).

cost(sortby(X, Y), Sel, P, S, C) :-
  Y \= [],
  cost(sort(X), Sel, P, S, C).

cost(mergejoin(X, Y, _, _), Sel, P, S, C) :-
  cost(X, 1, P, SizeX, CostX),
  cost(Y, 1, P, SizeY, CostY),
  sortmergejoinTC(_, B),
  S is SizeX * SizeY * Sel,
  C is CostX + CostY +                     % producing the arguments
    B * S.                                 % parallel scan of sorted relations

cost(sortmergejoin(X, Y, AX, AY), Sel, P, S, C) :-
  cost(mergejoin(sortby(X, [AX]),sortby(Y, [AY]), AX, AY), Sel, P, S, C).


% two rules used by the 'interesting orders extension':
cost(sortLeftThenMergejoin(X, Y, AX, AY), Sel, P, S, C) :-
  cost(mergejoin(sortby(X, [AX]), Y, AX, AY), Sel, P, S, C).

cost(sortRightThenMergejoin(X, Y, AX, AY), Sel, P, S, C) :-
  cost(mergejoin(X, sortby(Y, [AY]), AX, AY), Sel, P, S, C).



/*
Simple cost estimation for ~symmjoin~

*/
cost(symmjoin(X, Y, _), Sel, P, S, C) :-
  cost(X, 1, P, SizeX, CostX),
  cost(Y, 1, P, SizeY, CostY),
  getPET(P, _, ExpPET),                 % fetch stored predicate evaluation time
  symmjoinTC(A, B),                     % fetch relative costs
  S is SizeX * SizeY * Sel,             % calculate size of result
  C is CostX + CostY +                  % cost to produce the arguments
    (A + ExpPET) * (SizeX * SizeY) +    % cost to handle buffers, collisions
                                        %         and evaluate the predicate
    B * S.                              % cost to produce result tuples

cost(spatialjoin(X, Y, _, _), Sel, P, S, C) :-
  cost(X, 1, P, SizeX, CostX),
  cost(Y, 1, P, SizeY, CostY),
  spatialjoinTC(A, B),
  S is SizeX * SizeY * Sel,
  C is CostX + CostY +
  A * (SizeX + SizeY) +           % effort is essentially proportional to the
                                  % sizes of argument streams
  B * S.                          % cost to produce result tuples


/*
costs for pjoin2 will only be used if option ~adpativeJoin~ is enabled.

*/

cost(pjoin2(X, Y, [ _ | _ ]), Sel, P, Size, C) :-
  cost(X, 1, P, SizeX, _),
  cost(Y, 1, P, SizeY, _),
  Size is Sel * SizeX * SizeY,
  cost(sortmergejoin(X, Y, _, _), Sel, P, S1, C1),
  cost(hashjoin(X, Y, _, _, 99997), Sel, P, S1, C2),
  C is min(C1, C2).

cost(pjoin2_hj(X, Y, [ _ | _ ]), Sel, P, Size, C) :-
  cost(hashjoin(X, Y, _, _, 99997), Sel, P, Size, C).

cost(pjoin2_smj(X, Y, [ _ | _ ]), Sel, P, Size, C) :-
  cost(hashjoin(X, Y, _, _, 99997), Sel, P, Size, C).

cost(extend(X, _), Sel, P, S, C) :-
  cost(X, Sel, P, S, C1),
  extendTC(A),
  C is C1 + A * S.

cost(remove(X, _), Sel, P, S, C) :-
  cost(X, Sel, P, S, C1),
  removeTC(A),
  C is C1 + A * S.

cost(project(X, _), Sel, P, S, C) :-
  cost(X, Sel, P, S, C1),
  projectTC(A),
  C is C1 + A * S.

cost(projectextend(X, ProjectionList, ExtensionList), Sel, P, S, C) :-
  cost(X, Sel, P, S, C1),
  length(ProjectionList, PL),
  length(ExtensionList, EL),
  extendTC(EC),
  projectTC(PC),
  C is C1 + (PC * PL + EC * EL) * S.

cost(rename(X, _), Sel, P, S, C) :-
  cost(X, Sel, P, S, C1),
  renameTC(A),
  C is C1 + A * S.

% Xris: Added, costfactors not verified
cost(rdup(X), Sel, P, S, C) :-
  cost(X, Sel, P, S, C1),
  sortmergejoinTC(A, _),
  C is C1 + A * S.

% Xris: Added, costfactors not verified
cost(krdup(X,_AttrList), Sel, P, S, C) :-
  cost(rdup(X), Sel, P, S, C).

%fapra1590
cost(windowintersects(_, Rel, _), Sel, P, Size, Cost) :-
  cost(Rel, 1, P, RelSize, _),
  windowintersectsTC(C),
  Size is Sel * RelSize,
  Cost is Sel * RelSize * C.

% Cost function copied from windowintersects
% May be wrong, but as it is usually used together
% with 'gettuples', the total cost should be OK
cost(windowintersectsS(dbobject(IndexName), _), Sel, P, Size, Cost) :-
  % get relationName Rel from Index
  concat_atom([RelName|_],'_',IndexName),
  dcName2internalName(RelDC,RelName),
  Rel = rel(RelDC, *),
  cost(Rel, 1, P, RelSize, _),
  windowintersectsTC(C),
  Size is Sel * RelSize,  % bad estimation, may contain additional dublicates
  Cost is Sel * RelSize * C * 0.25. % other 0.75 applied in 'gettuples'

cost(gettuples(X, _), Sel, P, Size, Cost) :-
  cost(X, Sel, P, Size, CostX),
  windowintersectsTC(C),
  Cost is   CostX            % expected to include cost of 'windowintersectsS'
          + Size * C * 0.75. % other 0.25 applied in 'windowintersectsS'

cost(gettuples2(X, Rel, _IdAttrName), Sel, P, Size, Cost) :-
  cost(gettuples(X, Rel), Sel, P, Size, Cost).

/*
For distance-queries

*/

% get the size from the POG's high node
cost(pogstream, _, _, Size, 0) :-
  highNode(Node),
  resultSize(Node, Size).

cost(distancescan(_, Rel, _, _), Sel, P, Size, Cost) :-
  cost(Rel, Sel, P, Size, C1),
  distancescanTC(C),
  Cost is C1 + C * Size * log(Size + 1).

cost(ksmallest(X, K), Sel, P, S, C) :-
  cost(X, Sel, P, SizeX, CostX),
  ksmallestTC(A, B),
  S is min(SizeX, K),
  C is CostX +
    A * SizeX +
    B * S * log(S + 1).

/*
special handling for distance-queries which have to create an
temporary index

*/
cost(createtmpbtree(rel(Rel, _), _), _, _, RelSize,
     Cost) :-
  createbtreeTC(C),
  card(Rel, RelSize),
  Cost is C * RelSize * log(RelSize + 1).

/*
For matching a collection of symbolic trajectories (mlabel or mstring)

*/
cost(matches(rel(Rel, _), _, _), Sel, _, Size, Cost) :-
  card(Rel, RelSize),
  write('matches: size of relation is '),
  writeln(RelSize),
  Size is RelSize * Sel,
  Cost is 0.5 * Size.

cost(filtermatches(rel(Rel, _), _, _), Sel, _, Size, Cost) :-
  card(Rel, RelSize),
  write('filtermatches: size of relation is '),
  writeln(RelSize),
  Size is RelSize * Sel,
  Cost is 0.5 * Size.

cost(indexmatches(rel(Rel, _), _, _, _), Sel, _, Size, Cost) :-
  card(Rel, RelSize),
  write('indexmatches: size of relation is '),
  writeln(RelSize),
  Size is RelSize * Sel,
  Cost is 0.5 * Size.

% Section:Start:cost_5_e
% Section:End:cost_5_e

isPrefilter(X) :-
  X = spatialjoin(_, _, _, _).

isPrefilter(X) :-
  X = loopjoin(_, _).

% Section:Start:isPrefilter_1_e
% Section:End:isPrefilter_1_e



/*
The following code fragment may be needed, when also the non-conjunctive
part of the query will be assigned with costs. At the moment, it is obsolete
and therefore commented out:

----

% Dummy-Costs for simple aggregation queries
cost(simpleAggrNoGroupby(_, Stream, _), Sel, P, Size, Cost) :-
  cost(Stream, Sel, P, Size, Cost).

cost(simpleUserAggrNoGroupby(Stream, _, _, _),
  Sel, P, Size, Cost) :- cost(Stream, Sel, P, Size, Cost).

----


*/

/*

8.2 Creating Cost Edges

These are plan edges extended by a cost measure.

*/

% for debugging only:
createCostEdge(Source,Target,costEdge(Source, Target, Term, Result, Size, Cost))
  :- % use improved cost functions
  optimizerOption(improvedcosts),
  planEdge(Source, Target, Term, Result),
  edge(Source, Target, EdgeTerm, _, _, _),
  (   EdgeTerm = select(_, Pred)
    ; EdgeTerm = join(_, _, Pred)
    ; EdgeTerm = sortedjoin(_, _, Pred, _, _)
  ),
  edgeSelectivity(Source, Target, Sel),
  costterm(Term, Source, Target, Result, Sel, Pred, Size, Cost),
  assert(costEdge(Source, Target, Term, Result, Size, Cost)).


createCostEdge :- % use improved cost functions
  optimizerOption(improvedcosts),
  planEdge(Source, Target, Term, Result),
  edge(Source, Target, EdgeTerm, _, _, _),
  (   EdgeTerm = select(_, Pred)
    ; EdgeTerm = join(_, _, Pred)
    ; EdgeTerm = sortedjoin(_, _, Pred, _, _)
  ),
  edgeSelectivity(Source, Target, Sel),
  costterm(Term, Source, Target, Result, Sel, Pred, Size, Cost),
  assert(costEdge(Source, Target, Term, Result, Size, Cost)),
  fail.

createCostEdge :- % use Nawra's cost functions
  optimizerOption(nawracosts),
  planEdge(Source, Target, Term, Result),
  edgeSelectivity(Source, Target, Sel),
  costterm(Term, Sel, Source, Target, Size, Cost),
  assert(costEdge(Source, Target, Term, Result, Size, Cost)),
  fail.

/*
NVK ADDED MA
The costs are now based on a given operator function to estimate the costs. This is based on the ma\_improvedcosts and the CostEstimation class.

*/
createCostEdge :-
  optimizerOption(memoryAllocation),
  planEdge(Source, Target, Term, Result),
  edge(Source, Target, EdgeTerm, _, _, _),
  % Here it is ensured that no predicate will fail because the error 
  % will occur later and then there is no reference what edge had a problem.
  ensure((
    extractPredFromEdgeTerm(EdgeTerm, Pred),
    edgeSelectivity(Source, Target, Sel),
    costterm(Term, Source, Target, Result, Sel, Pred, Size, Cost, NewTerm),
    % Changed to remember assigned memory, if ever needed.
    % Currently the plan is generated on costEdge's, to there is not need
    % to change the planEdge.
    %retract(planEdge(Source, Target, Term, Result)),
    %asserta(planEdge(Source, Target, NewTerm, Result)),

    assert(costEdge(Source, Target, NewTerm, Result, Size, Cost))
  )),
  fail.
% NVK ADDED MA END


createCostEdge :- % use standard cost functions
  not(optimizerOption(nawracosts)),
  not(optimizerOption(improvedcosts)),
  \+ optimizerOption(memoryAllocation), % NVK ADDED MA
  planEdge(Source, Target, Term, Result),
  edge(Source, Target, EdgeTerm, _, _, _),
  (   EdgeTerm = select(_, Pred)
    ; EdgeTerm = join(_, _, Pred)
    ; EdgeTerm = sortedjoin(_, _, Pred, _, _)
  ),
  edgeSelectivity(Source, Target, Sel),
  cost(Term, Sel, Pred, Size, Cost), % Code changed by Goehr
  assert(costEdge(Source, Target, Term, Result, Size, Cost)),
  fail.


createCostEdges :- not(createCostEdge).

deleteCostEdges :-
  retractall(costEdge(_, _, _, _, _, _)),
  retractall(storedExtendSTermCost(_, _)),
  retractall(storedExtendAttrSize(_, _)),
  retractall(storedWindowIntersectsS(_)).

costEdgeInfo(Edge) :-
  Edge = costEdge(Source, Target, Plan, Result, Size, Cost),
  costEdgeInfo(Source, Target, Plan, Result, Size, Cost).

costEdgeInfo(Source, Target, Plan, Result, Size, Cost) :-
  nl, write('Source: '), write(Source),
  nl, write('Target: '), write(Target),
  nl, write('Plan  : '), wp(Plan),
%  nl, write('        '), write(Plan),  % for testing
  nl, write('Result: '), write(Result),
  nl, write('Size  : '), write(Size),
  nl, write('Cost  : '), write(Cost), nl.

writeCostEdges2 :-
  costEdge(Source, Target, Plan, Result, Size, Cost),
  costEdgeInfo(Source, Target, Plan, Result, Size, Cost), nl,
  fail.

writeCostEdges :- not(writeCostEdges2).

writeCostEdges2(Src) :-
  costEdge(Src, Target, Plan, Result, Size, Cost),
  costEdgeInfo(Src, Target, Plan, Result, Size, Cost), nl,
  fail.

writeCostEdges(Src) :- not(writeCostEdges2(Src)).


/*
----    assignCosts
----

This just puts together creation of sizes and cost edges. It must be applied
before calling ~bestPlan/2~. Hence a clause sequence ~pog, assignCosts,
bestPlan~ will unify a best plan according to the assigned costs.

*/

assignCosts :-
  deleteSizes,
  ( ( optimizerOption(nawracosts) ; optimizerOption(improvedcosts) )
    -> deleteSizesNawra; true),
  deleteCostEdges,
% Section:Start:assignCosts_0_i1
% Section:End:assignCosts_0_i1
  assignSizes,
% Section:Start:assignCosts_0_i2
% Section:End:assignCosts_0_i2
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
	% NVK ADDED MA
 	(\+ optimizerOption(memoryAllocation) ; \+ useModifiedDijkstra),
	% NVK ADDED MA END
  costEdge(Source, Target, Term, Result, Size, Cost),
  Distance2 is Distance + Cost,
  append(Path, [costEdge(Source, Target, Term, Result, Size, Cost)], Path2).

/*
NVK ADDED MA

*/
successor(node(Source, Distance, Path), node(Target, Distance2, RPath)) :-
  optimizerOption(memoryAllocation),
  useModifiedDijkstra,
  costEdge(Source, Target, Term, Result, Size, Cost),
  append(Path, [costEdge(Source, Target, Term, Result, Size, Cost)], Path2),

  ensure((
    dm(ma, ['\n\tCosts before memory optimisation: ', Cost]),
    % Note: At this point, we might have more memory granted as we have.
    % The following memory optimisation process will correct this. 
    pathMemoryOptimization(Path2, RPath),
    % Not implemented here is to stop the search for a optimum path if 
    % there is a path that has enough memory. But this can only be done
    % if the assigned memory is nenver smaller within the path as the
    % sufficient memory value.
    getCostsFromPath(RPath, NewCosts),
    dm(ma, ['\n\tCosts after memory optimisation: ', NewCosts]),
    dm(ma3, ['\n\tPath: ', RPath])
  )),
  Distance2 is NewCosts,
	% if this is not the case, there is something wrong and this
	% case can not be computed correctly with the dijkstra algorithm.
	ensure(Distance < Distance2).
% NVK ADDED MA END

/*

----    dijkstra(Source, Dest, Path, Length) :-
----

The shortest path from ~Source~ to ~Dest~ is ~Path~ of length ~Length~.

*/

:- dynamic center/2.

dijkstra(Source, Dest, Path, Length) :-
  emptyCenter,
  b_empty(Boundary),
  b_insert(Boundary, node(Source, 0, []), Boundary1),
  dijkstra1(Boundary1, Dest, 0, notfound),
  center(Dest, node(Dest, Length, Path)),
  !. % Cut inserted to avoid doubled solutions

emptyCenter :- retractall(center(_, _)).


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
writePath([costEdge(Source, Target, Term, Result, Size, Cost) | Path]) :-
  write(costEdge(Source, Target, Result, Size, Cost)), nl,
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
  optimizerOption(adaptiveJoin),
  makePStream(Path, Plan).


plan(Path, Plan) :-
  deleteNodePlans,
  traversePath(Path),
  highestNode(Path, N),
  nodePlan(N, Plan),
	!. % NVK ADDED MA

plan(Path, Plan) :-
  concat_atom(['Cannot create plan.'],'',ErrMsg),
  write(ErrMsg), nl,
  throw(error_Internal(optimizer_plan(Path, Plan)
                   ::unspecifiedError:ErrMsg)),
  !, fail.


deleteNodePlans :-  retractall(nodePlan(_, _)).

% switch for entropy optimizer
traversePath(Path) :-
  optimizerOption(entropy),
  traversePath2(Path), !.

% insert counters
traversePath(Path) :-
  optimizerOption(useCounters), !,
  resetCounter(nodeSizeCtr),
  traversePathX(Path).

% default
traversePath([]).

traversePath([costEdge(Source, Target, Term, Result, _, _) | Path]) :-
  edgeSelectivity(Source, Target, Sel),
  edgeInfoProgress(Source, Target, BBoxSel, ExpPET),
  markupProgress(Term, Sel, BBoxSel, ExpPET, Term2),
  embedSubPlans(Term2, Term3),
  assert(nodePlan(Result, Term3)),
  traversePath(Path),
	!. % NVK ADDED

traversePath(Path) :-
  concat_atom(['Cannot traverse Path.'],'',ErrMsg),
  write(ErrMsg), nl,
  throw(error_Internal(optimizer_traversePath(Path)
                   ::unspecifiedError::ErrMsg)),
  !, fail.

/*
---- markupProgress(Term+, Sel+, BBoxSel+, ExpPET+, Term2-) :-
----

Attach progress information to the right operators in a term.

*/

markupProgress(Term, _, _, _, Term) :- optimizerOption(noprogress).

markupProgress(project(Stream, Attrs), Sel, BBoxSel, ExpPET,
  project(Stream2, Attrs)) :-
  markupProgress(Stream, Sel, BBoxSel, ExpPET, Stream2),
  !.

markupProgress(remove(Stream, Attrs), Sel, BBoxSel, ExpPET,
  remove(Stream2, Attrs)) :-
  markupProgress(Stream, Sel, BBoxSel, ExpPET, Stream2),
  !.

markupProgress(rename(Stream, Var), Sel, BBoxSel, ExpPET,
  rename(Stream2, Var)) :-
  markupProgress(Stream, Sel, BBoxSel, ExpPET, Stream2),
  !.



% filter - windowintersects combinations

markupProgress(filter(Stream, Pred), Sel, BBoxSel, ExpPET,
    predinfo(filter(Stream2, Pred), Sel2, ExpPET)) :-
  BBoxSel \= noBBox,
  Sel2 is (Sel / BBoxSel) * 0.999,   %must be real
  markupProgressBBoxIndex(Stream, BBoxSel, Stream2),
  !.


% loopjoin

markupProgress(loopjoin(Stream, Expr), Sel, BBoxSel, ExpPET,
    loopjoin(Stream, Expr2)) :-
  markupProgress(Expr, Sel, BBoxSel, ExpPET, Expr2),
  !.

% loopjoin with filter - windowintersects

markupProgress(filter(loopjoin(Stream, Expr), Pred), Sel, BBoxSel, ExpPET,
    predinfo(filter(loopjoin(Stream, Expr2), Pred), Sel2, ExpPET)) :-
  BBoxSel \= noBBox,
  Sel2 is (Sel / BBoxSel) * 0.999,
  markupProgressBBoxIndex(Expr, BBoxSel, Expr2),
  !.


markupProgress(Stream, Sel, _, ExpPET, predinfo(Stream, Sel2, ExpPET)) :-
  Sel2 is Sel * 0.999.              %must be real




/*
---- markupProgressBBoxIndex(Stream+, Sel+, Stream2-)
----

Marks up a bbox index access operator (like windowintersects) with ~Sel~. Succeeds if such an operator exists in the term ~Stream~.

*/

markupProgressBBoxIndex(rename(Stream, Var), Sel, rename(Stream2, Var)) :-
  markupProgressBBoxIndex(Stream, Sel, Stream2),
  !.

markupProgressBBoxIndex(windowintersects(Index, Rel, Arg), Sel,
  predinfo(windowintersects(Index, Rel, Arg), Sel, 0.01) ).
















traversePathX([]).

traversePathX([costEdge(Source, Target, Term, Result, _, _) | Path]) :-
  embedSubPlans(Term, Term2),
  nextCounter(nodeSizeCtr, Nc),
  assert(nodePlan(Result, counter(Term2,Nc))),
  assert(nodeSizeCounter(Nc, Source, Target, Result)),
  traversePathX(Path).


embedSubPlans(res(N), Term) :-
  nodePlan(N, Term), !.

embedSubPlans(Term, Term2) :-
  compound(Term), !,
  Term =.. [Functor | Args],
  embedded(Args, Args2),
  Term2 =.. [Functor | Args2].
 %  nl, write('Term2: '), write(Term2).

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
  bestPlan(Plan, Cost), !,
  write('The best plan is:'), nl, nl,
  wp(Plan),
  nl, nl,
  write('The cost is: '), write(Cost), nl.

/*
NVK ADDED MA
See MemoryAllocation/ma.pl for more information.

*/
bestPlan(Plan, Cost) :-
  optimizerOption(memoryAllocation),
  nl, write('Computing best Plan ...'), nl,
  maBestPlan(Path, Cost),
  assert(path(Path)),
  dc(bestPlan, writePath(Path)),
  plan(Path, Plan),
	!.
% NVK ADDED MA END

bestPlan(Plan, Cost) :-
  \+ optimizerOption(memoryAllocation), % NVK ADDED MA
  nl, write('Computing best Plan ...'), nl,
  dc(bestPlan, writeCostEdges),
  highNode(N),
  dijkstra(0, N, Path, Cost),
  assert(path(Path)),
  dc(bestPlan, writePath(Path)),
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
  [rel(staedte, *), rel(plz, p1), rel(plz, p2), rel(plz, p3)],
  [
    pr(attr(sName, 1, u) = attr(p1:ort, 2, u),
                           rel(staedte, *), rel(plz, p1)),
    pr(attr(p1:pLZ, 1, u) = (attr(p2:pLZ, 2, u) + 1),
                             rel(plz, p1), rel(plz, p2)),
    pr(attr(p2:pLZ, 1, u) = (attr(p3:pLZ, 2, u) * 5),
                             rel(plz, p2), rel(plz, p3)),
    pr(attr(bev, 1, u) > 300000,  rel(staedte, *)),
    pr(attr(bev, 1, u) < 500000,  rel(staedte, *)),
    pr(attr(p2:pLZ, 1, u) > 50000,  rel(plz, p2)),
    pr(attr(p2:pLZ, 1, u) < 60000,  rel(plz, p2)),
    pr(attr(kennzeichen, 1, u) starts "W",  rel(staedte, *)),
    pr(attr(p3:ort, 1, u) contains "burg",  rel(plz, p3)),
    pr(attr(p3:ort, 1, u) starts "M",  rel(plz, p3))
  ],
  _, _).

/*
This doesn't work (initially, now it works). Let's keep the numbers a bit
smaller and avoid too many big joins first.

*/
example7 :- pog(
  [rel(staedte, *), rel(plz, p1)],
  [
    pr(attr(sName, 1, u) = attr(p1:ort, 2, u),
            rel(staedte, *), rel(plz, p1)),
    pr(attr(bev, 0, u) > 300000,  rel(staedte, *)),
    pr(attr(bev, 0, u) < 500000,  rel(staedte, *)),
    pr(attr(p1:pLZ, 0, u) > 50000,  rel(plz, p1)),
    pr(attr(p1:pLZ, 0, u) < 60000,  rel(plz, p1)),
    pr(attr(kennzeichen, 0, u) starts "F",  rel(staedte, *)),
    pr(attr(p1:ort, 0, u) contains "burg",  rel(plz, p1)),
    pr(attr(p1:ort, 0, u) starts "M",  rel(plz, p1))
  ],
  _, _).

example8 :- pog(
  [rel(staedte, *), rel(plz, p1), rel(plz, p2)],
  [
    pr(attr(sName, 1, u) = attr(p1:ort, 2, u),
            rel(staedte, *), rel(plz, p1)),
    pr(attr(p1:pLZ, 1, u) = (attr(p2:pLZ, 2, u) + 1),
            rel(plz, p1), rel(plz, p2)),
    pr(attr(bev, 0, u) > 300000,  rel(staedte, *)),
    pr(attr(bev, 0, u) < 500000,  rel(staedte, *)),
    pr(attr(p1:pLZ, 0, u) > 50000,  rel(plz, p1)),
    pr(attr(p1:pLZ, 0, u) < 60000,  rel(plz, p1)),
    pr(attr(kennzeichen, 0, u) starts "F",  rel(staedte, *)),
    pr(attr(p1:ort, 0, u) contains "burg",  rel(plz, p1)),
    pr(attr(p1:ort, 0, u) starts "M",  rel(plz, p1))
  ],
  _, _).

/*
Let's study a small example again with two independent conditions.

*/

example9 :- pog([rel(staedte, s), rel(plz, p)],
  [pr(attr(p:ort, 2, u) = attr(s:sName, 1, u),
        rel(staedte, s), rel(plz, p) ),
   pr(attr(p:pLZ, 0, u) > 40000, rel(plz, p)),
   pr(attr(s:bev, 0, u) > 300000, rel(staedte, s))], _, _).

example10 :- pog(
  [rel(staedte, *), rel(plz, p1), rel(plz, p2), rel(plz, p3)],
  [
    pr(attr(sName, 1, u) = attr(p1:ort, 2, u),
                           rel(staedte, *), rel(plz, p1)),
    pr(attr(p1:pLZ, 1, u) = (attr(p2:pLZ, 2, u) + 1),
                            rel(plz, p1), rel(plz, p2)),
    pr(attr(p2:pLZ, 1, u) = (attr(p3:pLZ, 2, u) * 5),
                            rel(plz, p2), rel(plz, p3))
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
        where [s:sname = p:ort, p:plz > 40000]
----

Note that all relation names and attribute names are written just in lower
case; the system will lookup the spelling in a table.

Furthermore, it possible possible to add a groupby- and an orderby-clause:

  * groupby

----    select <aggr-list>
        from <rel-list>
        where <pred-list>
        groupby <group-attr-list>
----

Example:

----    select [ort, min(plz) as minplz, max(plz) as maxplz,  count(*) as cntplz]
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

When using ~groupby~, the ~select list~ may contain

  * attributes from the ~$<$groupby-attr-list$>$~

  * aggregation functions like ~count([*])~, ~count(distinct [*])~,
    ~count($<$attr$>$)~, ~count(distinct $<$attr$>$)~, ~count($<$expr$>$)~,
    ~count(all $<$expr$>$)~, ~count(distinct $<$expr$>$)~, ~max($<$attr$>$)~,
    ~aggrop(distinct $<$attr$>$)~ (where ~aggrop~ is one of ~max~, ~min~, ~avg~,
    ~sum~). Also, user defined aggregation functions can be applied using the
    ~aggregate~ functor (explained below).

Example using a user defined aggregation function:

----  select
        aggregate((distinct b:no*1), (*), 'int', [const,int,value,0] ) as fac
      from [ten as a, ten as b]
      where [a:no < b:no]
      groupby a:no.
----

The next example also shows that the where-clause may be omitted. It is also
possible to combine grouping and ordering. You can further restrict the result
to the first or last ~N~ tuples by using ~first N~ or ~last N~:

----    select [ort,
                min(plz) as minplz,
                max(plz) as maxplz,
                count(distinct *) as cntplz]
        from plz
        where plz > 40000
        groupby ort
        orderby cntplz desc
        first 2.
----

Finally, it is possible to use an empty attribute list with groupby. In this
case, only a single group is created, e.g. all tuples belong to the same group:

----    select [min(plz) as minplz,
                max(plz) as maxplz,
                avg(plz) as avgplz,
                count(distinct ort) as ortcnt]
        from plz
        groupby [].
----

Simple aggregations:

If only a single value is created using an aggregation function in the select
list of a non-groupby query, it is not allowed to name it using the ~as~ directive.
Such queries may have the following syntax:

  * select $<$AggrOp$>$($<$Attr$>$) from ...

  * select $<$AggrOp$>$(all $<$Attr$>$) from ...

  * select $<$AggrOp$>$(distinct $<$Attr$>$) from ...

  * select aggregate($<$Attr$>$, $<$AggrOp$>$, $<$Type$>$, $<$Defaultvalue$>$ )

where $<$AggrOp$>$ is one of ~max~, ~min~, ~avg~, ~sum~, ~extract~. It is also
allowed to use expressions instead of attributes here.
The last syntax is for user defined aggregation functions, where ~Attr~ is the
attribute to aggregate over (possibly with preceding ~all~ or ~distinct~), ~AggrOp~
is a associative and commutative bijection operator name (infix operator must be passed in
round paranthesis), ~Type~ is the datatype of  ~Attr~, and ~DefaultValue~ is the value that
will be returned, if the query yields no value for ~Attr~.
Note: If using an expression instead of an attribute, ensure that $<$Type$>$ and
$<$Defaultvalue$>$ match the evaluated expression's type!

Examples:

----    select sum(no)
        from ten.

        select avg(distinct no)
        from ten
        where no > 5.

        select aggregate(distinct no+1.1, (*), 'real', [const,real,value,0.0] )
        from ten
        where no > 5.
----

Currently only this basic part of SQL has been implemented.


11.2 Structure

We introduce ~select~, ~from~, ~where~, ~as~, etc. as PROLOG operators:

(Here, only SQL keywords should be defined.
 Operator syntax is defined in file ~opsyntax.pl~!)

*/


:- op(993,  fx,  sql).
:- op(992,  fx,  create).
:- op(992,  fx,  drop).
:- op(991,  xfx, by).
:- op(988,  fx,  defmacro).
:- op(987, xfx,  usemacro).
:- op(986, xfx,  first).
:- op(986, xfx,  last).
:- op(980, xfx,  orderby).
:- op(970, xfx,  groupby).
:- op(960, xfx,  from).
:- op(960, xfx,  set).   % for update, insert
:- op(950,  fx,  select).
:- op(950, xfx,  where).
:- op(950, xfx,  select).% for update, insert
:- op(950,  fx,  delete).% for update, insert
:- op(950,  fx,  update).% for update, insert
:- op(950, xfx,  values).% for update, insert
:- op(950,  fx,  index).
:- op(950,  fx,  table).
:- op(945,  fx,  on).
:- op(940, xfx,  into).  % for update, insert
:- op(940,  fx,  distinct).
:- op(940,  fx,  all).
:- op(940, xfx,  columns).
:- op(935,  fx,  nonempty).
%:- op(930, xfx,  as).
:- op(930, xfy,  as). % NVK MODIFIED NR, compare testquery 571 (test.pl)
:- op(930, xf ,  asc).
:- op(930, xf ,  desc).
:- op(930,  fx,  insert).% for update, insert
:- op(930, xfx,  indextype).
:- op(800,  fx,  union).
:- op(800,  xfx, union).
:- op(800,  fx,  intersection).

% Section:Start:opPrologSyntax_3_e
% Section:End:opPrologSyntax_3_e


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
  \+ optimizerOption(nestedRelations),!, % NVK ADDED
  newQuery,
  lookup(Query, Query2), !.

% NVK ADDED NR
callLookup(Query, Query2) :-
  optimizerOption(nestedRelations),
  !,
  totalNewQuery,  % This is more like a totally new query, hence we
                  % have to invalidate the old query facts as well.
  lookup(Query, Query2), !.

/*
  Intended for subquery lookup's.

*/
callSubqueryLookup(Query, Query2) :-
  optimizerOption(nestedRelations),!,
  newQuery,
  lookup(Query, Query2), !.
% NVK ADDED NR END


newQuery :-
% Section:Start:newQuery_0_i
% Section:End:newQuery_0_i
  clearVariables,
  clearQueryRelations,
  clearQueryAttributes,
  clearUsedAttributes,
  clearIsStarQuery,
  clearSelectivityQuery.

clearVariables       :- retractall(variable(_, _)).
clearQueryRelations  :- retractall(queryRel(_, _)).
clearQueryAttributes :- retractall(queryAttr(_)).
clearUsedAttributes  :- retractall(usedAttr(_, _)).
clearIsStarQuery     :- retractall(isStarQuery).
clearSelectivityQuery :- retractall(selectivityQuery(_)).

/*

----    lookup(+Query, -Query2) :-
----

~Query2~ is a modified version of ~Query~ where all relation names and
attribute names have the form as required in [Section Translation].

*/

% Section:Start:lookup_2_b
% Section:End:lookup_2_b

/*
Version of lookup for large queries.

*/

%LargeQueries start:
lookup(select Attrs from Rels where Preds,
        select Attrs2 from Rels2List where Preds2List) :-  
  %largeQueries start:
  optimizerOption(largeQueries(qgdm)),  
  %largeQueries end
  lookupRels(Rels, Rels2),
  lookupAttrs(Attrs, Attrs2),
  lookupPreds(Preds, Preds2),
  makeList(Rels2, Rels2List),
  makeList(Preds2, Preds2List),
  %largeQueries start:
  createPredicateMap(Preds, Preds2List),
  %largeQueries end
  (optimizerOption(entropy)
    -> registerSelfJoins(Preds2List, 1); true). % needed for entropy optimizer
%LargeQueries end

/*
Standard version of lookup.

*/


lookup(select Attrs from Rels where Preds,
        select Attrs2 from Rels2List where Preds2List) :-
  lookupRels(Rels, Rels2),
  lookupAttrs(Attrs, Attrs2),
  lookupPreds(Preds, Preds2),
  makeList(Rels2, Rels2List),
  makeList(Preds2, Preds2List),
  (optimizerOption(entropy)
    -> registerSelfJoins(Preds2List, 1); true). % needed for entropy optimizer

lookup(select Attrs from Rels,
        select Attrs2 from Rels2) :-
  lookupRels(Rels, Rels2), !,
  lookupAttrs(Attrs, Attrs2).

%%%% Begin: for update and insert
lookup(insert into Rel values Values,
        insert into Rel2List values Values) :-
  lookupRel(Rel, Rel2),
  makeList(Rel2, Rel2List),
  !.

lookup(insert into Rel select Attrs from QueryRest,
        insert into Rel2List select Attrs2 from QueryRest2) :-
  lookup(select Attrs from QueryRest, select Attrs2 from QueryRest2),
  lookupRelNoDblCheck(Rel, Rel2),
  makeList(Rel2, Rel2List), !.

lookup(delete from Rel where Condition,
        delete from Rel2List where Condition2) :-
  lookup(select * from Rel where Condition,
  _ from Rel2List where Condition2),
  !.

lookup(delete from Rel,
        delete from Rel2List) :-
  lookupRel(Rel, Rel2),
  makeList(Rel2, Rel2List), !.

lookup(update Rel set Transformations where Condition,
       update Rel2List set Transformations2List where Condition2) :-
  lookup(select * from Rel where Condition, _ from Rel2List where Condition2),
  lookupTransformations(Transformations, Transformations2),
  makeList(Transformations2, Transformations2List),
  !.

lookup(update Rel set Transformations,
       update Rel2List set Transformations2List) :-
  lookupRel(Rel, Rel2),
  makeList(Rel2, Rel2List),
  lookupTransformations(Transformations, Transformations2),
  makeList(Transformations2, Transformations2List),
  !.
%%%% End: for update and insert

lookup(create table Tablename columns Columns,
       create table Tablename columns Columns2List) :-
  makeList(Columns, Columns2List), !.

lookup(create index on Rel columns Attrs indextype IndexType,
       create index on Rel2List columns Attrs2List indextype IndexType) :-
  lookup(create index on Rel columns Attrs,
  create index on Rel2List columns Attrs2List), !.


lookup(create index on Rel columns Attrs,
       create index on Rel2List columns Attrs2List) :-
  lookupRel(Rel, Rel2),
  makeList(Rel2, Rel2List),
  lookupAttrs(Attrs, Attrs2),
  makeList(Attrs2, Attrs2List), !.

lookup(drop table Rel,
       drop table Rel2List) :-
  lookupRel(Rel, Rel2),
  makeList(Rel2, Rel2List),
  !.

lookup(drop index on Rel columns Attrs indextype IndexType,
       drop index on Rel2List columns Attrs2List indextype IndexType) :-
  lookup(drop index on Rel columns Attrs,
  drop index on Rel2List columns Attrs2List), !.

lookup(drop index on Rel columns Attrs,
       drop index on Rel2List columns Attrs2List) :-
  lookupRel(Rel, Rel2),
  makeList(Rel2, Rel2List),
  lookupAttrs(Attrs, Attrs2),
  makeList(Attrs2, Attrs2List), !.

lookup(drop index Rel,
       drop index Rel2List) :-
  lookupIndex(Rel, Rel2),
  makeList(Rel2, Rel2List),
  !.

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

lookup(Query last N, Query2 last N) :-
  lookup(Query, Query2).

% Section:Start:lookup_2_e
% Section:End:lookup_2_e


makeList(L, L) :- is_list(L).

makeList(L, [L]) :- not(is_list(L)).

/*

----    dissolvelist(+ListIn, -ElementOut)
----

Removes the brackets of ListIn, if ~ListIn~ contains only one element

*/

dissolveList([], []) :- !.
dissolveList([L], L) :- !.
dissolveList([E|R], [E|R]) :- !.

/*

11.3.3 Modification of the From-Clause

----    lookupRels(+Rels, -Rels2)
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
----    lookupRel(+Rel, -Rel2) :-
----

Translate and store a single relation definition.

*/

:- dynamic
  variable/2,
  queryRel/2,
  queryAttr/1,
  isStarQuery/0,
  usedAttr/2,
  distanceRel/4,
  planvariable/2.


lookupRel(Rel as Var, Y) :-
  atomic(Rel),       %% changed code FIXME
  atomic(Var),       %% changed code FIXME
  dcName2externalName(RelDC,Rel), % get downcase spelling
  relation(RelDC, _), !,          %% changed code FIXME
  ( variable(Var, _)
    -> ( concat_atom(['Doubly defined variable \'',Var,'\'.'],'',ErrMsg),
         write_list(['\nERROR:\t',ErrMsg]), nl,
         throw(error_SQL(optimizer_lookupRel(Rel as Var,Y)
                                       ::malformedExpression:ErrMsg)),
         fail
       )
    ;  Y = rel(RelDC, Var)
  ),
  assert(variable(Var, rel(RelDC, Var))).

lookupRel(Rel, rel(RelDC, *)) :-
  atomic(Rel),       %% changed code FIXME
  dcName2externalName(RelDC,Rel),
  relation(RelDC, _), !,
  not(duplicateAttrs(RelDC)),
  assert(queryRel(RelDC, rel(RelDC, *))).

% NVK ADDED NR
lookupRel(Term, ResultTerm) :-
  optimizerOption(nestedRelations),
  nrLookupRel(Term, ResultTerm).
% NVK ADDED NR END

lookupRel(Rel, Rel2) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED
  optimizerOption(subqueries),
  lookupSubquery(Rel, Rel2).

lookupRel((Rel) as Var, (Rel2) as Var) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED
  optimizerOption(subqueries),
  lookupSubquery(Rel, Rel2).

lookupRel(Rel, Rel2) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED
  optimizerOption(subqueries),
  lookupSubquery(Rel, Rel2).

lookupRel((Rel) as Var, (Rel2) as Var) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED
  optimizerOption(subqueries),
  lookupSubquery(Rel, Rel2).

lookupRel(X,Y) :- !,
  (isDatabaseOpen
   -> ( term_to_atom(X,XA),
        concat_atom(['Unknown relation: \'',XA,'\'.'],'',ErrMsg),
        write_list(['\nERROR:\t',ErrMsg]), nl,
        throw(error_SQL(optimizer_lookupRel(X,Y)::unknownRelation::ErrMsg)),
        fail
      )
   ;  ( concat_atom(['No database open'], '', ErrMsg),
        write_list(['\nERROR:\t', ErrMsg]), nl,
        throw(error_SQL(optimizer_lookupRel(X,Y)::noDatabaseOpen::ErrMsg)),
        fail
      )
  ).

/*
----    lookupRelNoDblCheck(+Rel, -Rel2) :-
----

Translate and store a single relation definition without looking for
duplicate attributes.

*/

%%%% Begin: for update and insert
lookupRelNoDblCheck(Rel, rel(RelDC, *)) :-
  atomic(Rel),       %% changed code FIXME
  dcName2externalName(RelDC,Rel),
  relation(RelDC, _), !,
  assert(queryRel(RelDC, rel(RelDC, *))).
%%%% End: for update and insert

/*
----    lookupIndex(+Rel, -Rel2) :-
----

Translate and store a single index definition.

*/

lookupIndex(Rel, RelDC) :-
  atomic(Rel),
  dcName2externalName(RelDC, Rel).

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
  not(Rel = Rel2), !,
  term_to_atom(Rel,RelA),
  term_to_atom(Rel2,Rel2A),
  concat_atom(['Duplicate attribute alias names in relations ',
              Rel2A, ' and ',RelA,'.'],'',ErrMsg),
  write_list(['\nERROR:\t',ErrMsg]),
  throw(error_SQL(optimizer_duplicateAttrs(Rel)::malformedExpression::ErrMsg)),
  nl.

/*
11.3.4 Modification of the Select-Clause

*/

lookupAttrs(distinct X, distinct Y) :-
  lookupAttrs(X, Y).

lookupAttrs(all X, Y) :-
  lookupAttrs(X, Y).

lookupAttrs([], []).

lookupAttrs([A | As], [A2 | A2s]) :-
  lookupAttr(A, A2),
  lookupAttrs(As, A2s).

lookupAttrs(Attr, Attr2) :-
  not(is_list(Attr)),
  lookupAttr(Attr, Attr2).

% complex constant value expression
lookupAttr([const, Type, value, Value], value_expr(Type,Value)) :-
  ground(Type), ground(Value),
  ( atom(Type)
    -> Op = Type
    ;  ( (compound(Type), \+ is_list(Type))
         -> Type =.. [Op|_]
         ;  fail % Type is a list, which means it is not a valid type expression
       )
  ),
  downcase_atom(Op,OpDC),
  secDatatype(OpDC, _, _, _, _, _),
  !.

lookupAttr([const, Type, value, Value], Y) :-
  concat_atom(['ERROR:\tConstant value expression \'',
              [const, Type, value, Value],'\' could not be parsed!\n'],
              '',ErrMsg),
  write_list(['\nERROR: ',ErrMsg]), nl,
  throw(error_SQL(optimizer_lookupAttr([const, Type, value, Value],Y)
                                ::malformedExpression:ErrMsg)),
  !, fail.

% text constant
lookupAttr(const(text,Value), value_expr(text,Value2)) :-
  ( atomic(Value)
    -> atom_codes(Value2,Value)
    ;  ( (is_list(Value), atom_codes(_,Value))
          -> Value = Value2
          ; ( concat_atom(['ERROR:\tConstant value expression \'',
              const(text,Value),'\' could not be parsed!\n'],
                      '',ErrMsg),
              write_list(['\nERROR: ',ErrMsg]), nl,
              throw(error_SQL(optimizer_lookupAttr(const(text,Value),
                       value_expr(text,Value2))::malformedExpression:ErrMsg)),
              !, fail
            )
       )
  ),
  !.

% complex constant value expression (alternative)
lookupAttr(const(Type,Value), value_expr(Type,Value)) :-
  ground(Type), ground(Value),
  ( atom(Type)
    -> Op = Type
    ;  ( (compound(Type), \+ is_list(Type))
         -> Type =.. [Op|_]
         ;  ( % Type is a list, it is not a valid type expression
              concat_atom(['ERROR:\tConstant value expression \'',
                      [const, Type, value, Value],'\' could not be parsed!\n'],
                      '',ErrMsg),
              write_list(['\nERROR: ',ErrMsg]), nl,
              throw(error_SQL(optimizer_lookupAttr(const(Type,Value),
                      value_expr(Type,Value))::malformedExpression:ErrMsg)),
              !, fail
            )
       )
  ),
  downcase_atom(Op,OpDC),
  secDatatype(OpDC, _, _, _, _, _),
  !.

lookupAttr(const(Type,Value), Y) :-
  concat_atom(['ERROR:\tConstant value expression \'',
              const(Type,Value),'\' could not be parsed!\n'],
              '',ErrMsg),
  write_list(['\nERROR: ',ErrMsg]), nl,
  throw(error_SQL(optimizer_lookupAttr(const(Type,Value),Y)
                                ::malformedExpression:ErrMsg)),
  !, fail.

% renamed attribute
lookupAttr(Var:Attr, attr(Var:Attr2, 0, Case)) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED
	!,
  atomic(Var),  %% changed code FIXME
  atomic(Attr), %% changed code FIXME
  variable(Var, Rel2),
  Rel2 = rel(Rel, _),
  spelled(Rel:Attr, attr(Attr2, VA, Case)),
  (   usedAttr(Rel2, attr(Attr2, VA, Case))
    ; assert(usedAttr(Rel2, attr(Attr2, VA, Case)))
  ).

% sorting orders
lookupAttr(Attr asc, Attr2 asc) :- !,
  lookupAttr(Attr, Attr2).

lookupAttr(Attr desc, Attr2 desc) :- !,
  lookupAttr(Attr, Attr2).

% attribute
lookupAttr(Attr, Attr2) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED
  atomic(Attr), %% changed code FIXME
  downcase_atom(Attr,AttrDC),
  isAttribute(AttrDC, Rel),!,
  spelled(Rel:Attr, Attr2),
  queryRel(Rel, Rel2),
  (   usedAttr(Rel2, Attr2)
    ; assert(usedAttr(Rel2, Attr2))
  ).

% count(*)
lookupAttr(count(*), count(*)) :- !.

% *
lookupAttr(*, *) :- assert(isStarQuery), !.

% count
lookupAttr(count(T), count(T2)) :-
  lookupAttr(T, T2), !.

% special clause for rowid
lookupAttr(rowid, rowid) :- !.

/*
NVK ADDED NR
lookup subqueries within the attribute list.
This will create a new arel attribute.

*/
lookupAttr(InTerm, OutTerm) :-
  optimizerOption(nestedRelations),
  nrLookupAttr(InTerm, OutTerm),
  !.
% NVK ADDED NR END

/*
Special clause for ~aggregate~

*/

lookupAttr(Term, Result) :-
  compound(Term),
  Term =.. [AggrOp, Term2, Op, Type, Default],
  member(AggrOp,[aggregate]),
  lookupAttr(Term2, Term2Res),
  lookupAttr(Default,DefaultRes),
  Result =.. [AggrOp, Term2Res, Op, Type, DefaultRes],
  !.

lookupAttr(T, T2) :-
  compound(T),
  T =.. [Op, Expr],
  lookupAttr(Expr, Expr2),
  T2 =.. [Op, Expr2],
  isAggregationOP(T2),
  !.

lookupAttr(Expr as Name, Expr2 as attr(Name, 0, u)) :-
  lookupAttr(Expr, Expr2),
  not(queryAttr(attr(Name, 0, u))),
  !,
  assert(queryAttr(attr(Name, 0, u))).

lookupAttr(Expr as Name, Y) :-
  lookupAttr(Expr, _),
  queryAttr(attr(Name, 0, u)),
  !,
  term_to_atom(Name,NameA),
  concat_atom(['Doubly defined attribute names \'',NameA,'\'',
              ' within query.'],'',ErrMsg),
  write_list(['\nERROR: ',ErrMsg]), nl,
  throw(error_SQL(optimizer_lookupAttr(Expr as Name,Y)
                                ::malformedExpression:ErrMsg)),
  fail.

/*
Generic lookupAttr/2-rule for functors of arbitrary arity using Univ (=../2):

*/
lookupAttr(Name, attr(Name, 0, u)) :-
  queryAttr(attr(Name, 0, u)),
  !.

% string constant
lookupAttr(Term, Term) :-
  is_list(Term),
  catch(string_to_list(_, Term), _, fail),
  !.

lookupAttr(Term, Term2) :-
  compound(Term),
  Term =.. [Op|Args],
  lookupAttr1(Args, Args2),
  Term2 =.. [Op|Args2],
  !.

% bool constant
lookupAttr(true, value_expr(bool,true)) :- !.
lookupAttr(false, value_expr(bool,false)) :- !.


% null-ary operator
lookupAttr(Op, Op) :-
  atom(Op),
  secondoOp(Op, prefix, 0),
  systemIdentifier(Op, _),
  !.

% special clause for string atoms (they regularly cause problems since they
% are marked up in double quotes, which Prolog handles as strings, that are
% represented as charactercode lists...
lookupAttr(Term, value_expr(string,Term)) :-
  is_list(Term), % list represents a string (list of characters)
  catch((string_to_list(_,Term), Test = ok),_,Test = failed), Test = ok,
  !.


% database object
lookupAttr(Term, dbobject(TermDC)) :-
  atomic(Term),
  \+ is_list(Term),
  dcName2externalName(TermDC,Term),
  secondoCatalogInfo(TermDC,_,_,_),
  !.

% Primitive: int-atom
lookupAttr(IntAtom, value_expr(int,IntAtom)) :-
  atomic(IntAtom), integer(IntAtom),
  !.

% Primitive: real-atom
lookupAttr(RealAtom, value_expr(real,RealAtom)) :-
  atomic(RealAtom), float(RealAtom),
  !.

%% Primitive: text-atom
%lookupAttr(Term, value_expr(text,Term)) :-
%  atom(Term), !.

lookupAttr(Term, Term) :-
  atom(Term),
  concat_atom(['Unknown symbol: \'',Term,'\' is not recognized!'],'',ErrMsg),
  write_list(['\nERROR:\t',ErrMsg]),
  throw(error_SQL(optimizer_lookupAttr(Term, Term)::unknownIdentifier::ErrMsg)),
  fail.

% Fallback clause
lookupAttr(Term, Term) :- !.

lookupAttr1([],[]) :- !.

lookupAttr1([Me|Others],[Me2|Others2]) :-
  lookupAttr(Me,Me2),
  lookupAttr1(Others,Others2),
  !.

isAttribute(Name, Rel) :-
  queryRel(Rel, _),
  relation(Rel, List),
  member(Name, List), !.


/*
11.3.5 Modification of the Where-Clause


03/10/2006: (C. D[ue]ntgen) Enabled optimizer to process even predicates with more
            than two occurences of attributes, as long as at most 2 relations
            are concerned, e.g. something like $a+a>b$, a and b being attributes
            of relations A resp. B, is now accepted.

*/

lookupPreds([], []) :- !.

lookupPreds([P | Ps], [P2 | P2s]) :-
  lookupPred(P, P2),
  lookupPreds(Ps, P2s), !.

lookupPreds(Pred, Pred2) :-
  not(is_list(Pred)),
  lookupPred(Pred, Pred2), !.

/*
Used within the spatiotemporal pattern predicate.
If Pred is among the additional predicates list, it
is looked up and the list is updated with the new
syntax not to loose track of the additonal predicates.
This lookupPred should always be placed as the first
called lookupPred.

*/

lookupPred(Pred, pr(Pred2, Rel)) :-
  removefilter(Pred),
  nextCounter(selectionPred,_),
  lookupPred1(Pred, Pred2, [], [Rel]), !,
  retract(removefilter(Pred)),
  assert(removefilter(Pred2)).

% Section:Start:lookupPred_2_b
% Section:End:lookupPred_2_b



lookupPred(Pred, pr(Pred2, Rel)) :-
  nextCounter(selectionPred,_),
  lookupPred1(Pred, Pred2, [], [Rel]), !.

lookupPred(Pred, pr(Pred2, Rel1, Rel2)) :-
  nextCounter(joinPred,_),
  lookupPred1(Pred, Pred2, [], [Rel1, Rel2]), !.

lookupPred(Pred, X) :-
  lookupPred1(Pred, _, [], Rels),
  length(Rels, N),
  term_to_atom(Pred,PredA),
  term_to_atom(Rels,RelsA),
  ( (N = 0)
    -> ( concat_atom(['Malformed predicate: \'',PredA,
                     '\' is a constant. This is not allowed.'],'',ErrMsg)
       )
    ; ( (N > 2)
        -> concat_atom(['Malformed predicate: \'',PredA,
                       '\' involves more than two relations: ',RelsA,
                       '. This is not allowed.'],'',ErrMsg)
        ; concat_atom(['Malformed predicate: \'',PredA,
                       '\' unspecified reason.'],'',ErrMsg)
      )
  ),
  write_list(['\nERROR:\t',ErrMsg]),nl,
  throw(error_SQL(optimizer_lookupPred(Pred, X)::malformedExpression::ErrMsg)),
  fail, !.

/*
----    lookupPred1(+Pred, -Pred2, +RelsBefore, -RelsAfter)
----

~Pred2~ is the transformed version of ~Pred~; before this is called,
attributes so far considered have already used relations from list ~RelsBefore~.
The relation list is updated and returned in ~RelsAfter~.

*/

% Section:Start:lookupPred1_2_b
% Section:End:lookupPred1_2_b

/*
Used within the spatiotemporal pattern predicates stpattern.
The only component of the STPP that need lookup is the
list of lifted predicates. This special lookupPred1
predicate separates the lifted predicates and passes them
to the normal lookupPred1. The constraints are passed as is
since there is no need to lookup them. The aliases are
composed back with the looked up lifted predicates by the
lookupPattern predicate.

*/

lookupPred1(pattern(Preds,C), pattern(Res,C1), RelsBefore, RelsAfter) :-
  lookupPattern(Preds, Res, RelsBefore, RelsAfterMe),
  ((is_list(C), lookupPred2(C, C1, RelsAfterMe, RelsAfter));
  lookupPred1(C, C1, RelsAfterMe, RelsAfter)),
  !.

/*
Used within the extended spatiotemporal pattern predicates stpatternex.
The components that need lookup are the list of lifted predicates and
the boolean condition (part2 of the pattern). This special lookupPred1
predicate separates the lifted predicates and passes them
to the normal lookupPred1. The boolean condidtion is also
passed to the normal lookupPred1. The constraints are passed as is
since there is no need to lookup them. The aliases are
composed back with the looked up lifted predicates by the
lookupPattern predicate.

*/
lookupPred1(patternex(Preds,C, F),patternex(Res,C1, F1),RelsBefore,RelsAfter) :-
  lookupPattern(Preds, Res, RelsBefore, RelsAfterMe),
  lookupPred1(F, F1, RelsAfterMe, RelsAfterMee),
  ((is_list(C), lookupPred2(C, C1, RelsAfterMee, RelsAfter));
  lookupPred1(C, C1, RelsAfterMee, RelsAfter)),
  !.


lookupPred1(Pred, Pred2, RelsBefore, RelsAfter) :-
  optimizerOption(subqueries),
  lookupSubqueryPred(Pred, Pred2, RelsBefore, RelsAfter), !.

% NVK ADDED NR
lookupPred1(InTerm, OutTerm, RelsBefore, RelsAfter) :-
  optimizerOption(nestedRelations),
  nrLookupPred1(InTerm, OutTerm, RelsBefore, RelsAfter),
  !.
% NVK ADDED NR END

lookupPred1(Var:Attr, attr(Var:Attr2, Index, Case), RelsBefore, RelsAfter)
  :-
  \+ optimizerOption(nestedRelations), % NVK ADDED
  variable(Var, Rel2), !, Rel2 = rel(Rel, Var),
  downcase_atom(Rel,RelDC),
  downcase_atom(Attr,AttrDC),
  spelled(RelDC:AttrDC, attr(Attr2, X, Case)),
  ( member(Rel2, RelsBefore)
      -> RelsAfter = RelsBefore
       ; append(RelsBefore, [Rel2], RelsAfter)
  ),
  nth1(Index,RelsAfter,Rel2),
  (   usedAttr(Rel2, attr(Attr2, X, Case))
    ; assert(usedAttr(Rel2, attr(Attr2, X, Case)))
  ), !.

lookupPred1(Attr, attr(Attr2, Index, Case), RelsBefore, RelsAfter) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED
  isAttribute(Attr, Rel), !,
  downcase_atom(Rel,RelDC),
  downcase_atom(Attr,AttrDC),
  spelled(RelDC:AttrDC, attr(Attr2, X, Case)),
  queryRel(Rel, Rel2),
  ( member(Rel2, RelsBefore)
      -> RelsAfter = RelsBefore
       ; append(RelsBefore, [Rel2], RelsAfter)
  ),
  nth1(Index,RelsAfter,Rel2),
  (   usedAttr(Rel2, attr(Attr2, X, Case))
    ; assert(usedAttr(Rel2, attr(Attr2, X, Case)))
  ), !.

% Section:Start:lookupPred1_2_m
% Section:End:lookupPred1_2_m

% Primitive: int-atom
lookupPred1(IntAtom, value_expr(int,IntAtom), RelsBefore, RelsBefore) :-
  atomic(IntAtom), integer(IntAtom),
  !.

% Primitive: real-atom
lookupPred1(RealAtom, value_expr(real,RealAtom), RelsBefore, RelsBefore) :-
  atomic(RealAtom), float(RealAtom),
  !.

% Primitive: string-atom (they regularly cause problems since they
% are marked up in double quotes, which Prolog handles as strings, that are
% represented as charactercode lists...)
lookupPred1(Term, value_expr(string,Term), RelsBefore, RelsBefore) :-
  is_list(Term), % list represents a string (list of characters)
  catch((string_to_list(_,Term), Test = ok),_,Test = failed), Test = ok,
  !.

%% Primitive: generic atom (constant expression)
%lookupPred1(Term, value_expr(text,Term), RelsBefore, RelsBefore) :-
%  atom(Term), !.
lookupPred1(const(Type,Value), value_expr(Type,Value), RelsBefore, RelsBefore):-
  ground(Type), ground(Value),
  ( atom(Type)
    -> Op = Type
    ;  ( (compound(Type), \+ is_list(Type))
         -> Type =.. [Op|_]
         ;  fail % Type is a list, which means it is not a valid type expression
       )
  ),
  downcase_atom(Op,OpDC),
  secDatatype(OpDC, _, _, _, _, _),
  !.

% constant value expression
lookupPred1([const, Type, value, Value], value_expr(Type,Value),
            RelsBefore, RelsBefore) :-
  ground(Type), ground(Value),
  ( atom(Type)
    -> Op = Type
    ;  ( (compound(Type), \+ is_list(T))
         -> T =.. [Op|_]
         ;  fail
       )
  ),
  downcase_atom(Op,OpDC),
  secDatatype(OpDC, _, _, _, _, _),
  !.

lookupPred1([const, Type, value, Value], value_expr(Type,Value),
            RelsBefore, RelsBefore) :-
  write_list(['ERROR:\tConstant value expression \'',
              [const, Type, value, Value],'\' could not be parsed!\n']),
  !, fail.

% special case for rowid
lookupPred1(rowid, rowid, RelsBefore, RelsBefore) :- !.

lookupPred1(Term, Term2, RelsBefore, RelsAfter) :-
%if placed before lookupPred1(pattern*), pattern query crash
  compound(Term),
  \+ is_list(Term),
  Term =.. [Op|Args],
  \+ isSubqueryPred1(Term),
  lookupPred2(Args, Args2, RelsBefore, RelsAfter),
  Term2 =.. [Op|Args2],
  !.

% null-ary operator
lookupPred1(Op, Op, Rels, Rels) :-
  atom(Op),
  secondoOp(Op, prefix, 0),
  systemIdentifier(Op, _),
  !.

% database object
lookupPred1(Term, dbobject(TermDC), Rels, Rels) :-
  atomic(Term),
  \+ is_list(Term),
  dcName2externalName(TermDC,Term),
  secondoCatalogInfo(TermDC,_,_,_),
  !.

% Primitive: text-atom.
lookupPred1(Term, value_expr(text,Term), RelsBefore, RelsBefore) :-
  atom(Term),
  not(is_list(Term)), !.

lookupPred1(Term, Term, Rels, Rels) :-
 atom(Term),
 \+ is_list(Term),
 concat_atom(['Symbol \'', Term,
            '\' not recognized. It is neither an attribute, nor a Secondo ',
            'object.\n'],'',ErrMsg),
 write_list(['\nERROR:\t',ErrMsg]),
 throw(error_SQL(optimizer_lookupPred1(Term, Term)::unknownIdentifier::ErrMsg)).

% fallback clause for non-atoms
lookupPred1(Term, Term, RelsBefore, RelsBefore).

lookupPred2([], [], RelsBefore, RelsBefore).

lookupPred2([Me|Others], [Me2|Others2], RelsBefore, RelsAfter) :-
  lookupPred1(Me,     Me2,     RelsBefore,  RelsAfterMe),
  lookupPred2(Others, Others2, RelsAfterMe, RelsAfter),
  !.

/*
----    lookupTransformations(+Transf, -Transf2)
----

Handles the transformations in an update command

*/

%%%% Begin: for update and insert
lookupTransformations([], []) :- !.

lookupTransformations([T | Ts], [T2 | T2s]) :-
  lookupTransformation(T, T2),
  lookupTransformations(Ts, T2s), !.

lookupTransformations(Trans, Trans2) :-
  \+ is_list(Trans),
  lookupTransformation(Trans, Trans2), !.

/*
----    lookupTransformation(+Transf, -Transf2)
----

Handles one transformation in an update command

*/

lookupTransformation(Attr = Expr, Attr2 = Expr2) :-
  lookupAttr(Attr,Attr2),
  lookupSetExpr(Expr, Expr2),
  !.

/*
----    lookupSetExpr(+Expr, -Expr2)
----

Handles the expressions in the set-clause of an update command. Scans
the term for attributes and looks for these attributes in the database

*/

lookupSetExpr([], []) :- !.

lookupSetExpr(Attr, Attr2) :-
  isAttribute(Attr, _), !,
  lookupAttr(Attr, Attr2).

lookupSetExpr([Term|Terms], [Term2|Terms2]) :-
  lookupSetExpr(Term, Term2),
  lookupSetExpr(Terms, Terms2), !.

lookupSetExpr(Term, Term2) :-
  compound(Term),
  Term =.. [Op|Args],
  lookupSetExpr(Args, Args2),
  Term2 =.. [Op|Args2],
  !.

lookupSetExpr(Expr, Expr).
%%%% End: for update and insert

/*
11.3.6 Check the Spelling of Relation and Attribute Names

---- spelled(+In,-Out)
----

*/

% NVK ADDED
/*
Examples:

----
?- spelled(staedte:sName, X).
X = attr(sName, 0, u).
?- spelled(orteh:subrel:kennzeichen, X).
X = attr(kennzeichen, 0, u).
----


Here there is no atomic check anymore, downcase\_atom will throw an exception 
if there is a non-atomic value between the colons. This predicate could 
handle the next two predicate cases as well.

*/
spelled(Rel:Atts, attr(Attr2, 0, Case)) :-
  optimizerOption(nestedRelations),
  applyOnAttributeList(downcase_atom, Rel:Atts, DCRel:DCAtts),
  spelling(DCRel:DCAtts, DCLastAtt),
  (DCLastAtt = lc(Attr2), Case=l ; Attr2=DCLastAtt, Case=u),
  !.
% NVK ADDED END

spelled(Rel:Attr, attr(Attr2, 0, l)) :-
  atomic(Rel),  %% changed code FIXME
  atomic(Attr), %% changed code FIXME
  downcase_atom(Rel, DCRel),
  downcase_atom(Attr, DCAttr),
  spelling(DCRel:DCAttr, Attr3),
  Attr3 = lc(Attr2),
  !.

spelled(Rel:Attr, attr(Attr2, 0, u)) :-
  atomic(Rel),  %% changed code FIXME
  atomic(Attr), %% changed code FIXME
  downcase_atom(Rel, DCRel),
  downcase_atom(Attr, DCAttr),
  spelling(DCRel:DCAttr, Attr2),
  not( Attr2 = lc(_) ),
  !.

spelled(_:_, attr(_, 0, _)) :- !, fail. % no attr entry in spelling table

spelled(Rel, Rel2, l) :-
  atomic(Rel),  %% changed code FIXME
  downcase_atom(Rel, DCRel),
  spelling(DCRel, Rel3),
  Rel3 = lc(Rel2),
  !.

spelled(Rel, Rel2, u) :-
  atomic(Rel),  %% changed code FIXME
  downcase_atom(Rel, DCRel),
  spelling(DCRel, Rel2),
  not( Rel2 = lc(_) ),
  !.

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

----    translate1(+Query, -Stream, -SelectClause, -UpdateClause, -Cost) :-
----

~Query~ is translated into a ~Stream~ to which still the translation of the
~SelectClause~ and ~UpdateClause~ need to be applied. A ~Cost~ is
returned which currently is only the cost for evaluating the essential
part, the conjunctive query.

*/


% special handling for the entropy optimizer
translate1(Query groupby Attrs,
        groupby(sortby(Stream, AttrNamesSort), AttrNamesGroup, Fields),
        select Select2, Update, Cost) :-
  optimizerOption(entropy),
  translate1(Query, Stream, SelectClause, Update, Cost),
  makeList(Attrs, Attrs2),
  attrnames(Attrs2, AttrNamesGroup),
  attrnamesSort(Attrs2, AttrNamesSort),
  SelectClause = (select Select),
  makeList(Select, SelAttrs),
  translateFields(SelAttrs, Attrs2, Fields, Select2,_,_),
  !.

translate1(Query, Stream3, Select2, Update, Cost2) :-
  optimizerOption(entropy),
  deleteSmallResults,
  retractall(highNode(_)), assert(highNode(0)),
  assert(buildingSmallPlan),
  retractall(removeHiddenAttributes),
  translate(Query, Stream1, Select, Update, Cost1),
  translateEntropy(Stream1, Stream2, Cost1, Cost2),
  % Hook for CSE substitution:
  rewritePlanforCSE(Stream2, Stream3, Select, Select2),
  !.


%LargeQueries start:
translate1(Select from Rels where Preds, Stream2, Select2, Update, Cost) :-
  optimizerOption(largeQueries(qgd)),
  length(Preds,NumberOfPreds),
  NumberOfPreds > 10,
  qgd(Select from Rels where Preds, Stream2, Select2, Update, Cost).

translate1(Select from Rels where Preds, Stream2, Select2, Update, Cost) :-
  optimizerOption(largeQueries(qgdm)),
  length(Preds,NumberOfPreds),
  NumberOfPreds > 10,
  qgdm(Select from Rels where Preds, Stream2, Select2, Update, Cost).

%LargeQueries end




% default handling
translate1(Query, Stream2, Select2, Update, Cost) :-
  translate(Query, Stream, Select, Update, Cost),
  rewritePlanforCSE(Stream, Stream2, Select, Select2), 
    % Hook for CSE substitution
  !.

%    the main predicate which does the translation of a query
%    translate(+Query, -Stream, -SelectClause, -UpdateClause, -Cost).
%  This version of the predicate is only used while the optimizer option
%  earlyproject is active.
% (Clause added by Goehr)
translate(Query groupby Attrs,
        groupby(sortby(projectextend(Stream,AExtend1,[FExtend|ExtendAttrs]),
                                     AttrNamesSort), AttrNamesGroup, Fields),
        select Select3, Update, Cost) :-
  optimizerOption(earlyproject),
  translate(Query, Stream, SelectClause, Update, Cost),
  makeList(Attrs, Attrs2),
  Attrs2 \= [],
  SelectClause = (select Select),
  makeList(Select, SelAttrs),
  translateFields(SelAttrs, Attrs2, Fields, Select2, ExtendAttrs,ProjectAttrs),
  getProjectAttrs(Select,Attrs2,FExtend,Project),
  getProjectAttrs1(Select,Attrs2,ProjectAttrs,AExtend),
  attrnames(Project, AttrNamesGroup),
  attrnames(AExtend,AExtend1),
  attrnamesSort(Project, AttrNamesSort),
  delExtends(Select2,Select3),
  !.

% the main predicate which does the translation of a query
%   translate(+Query, -Stream, -SelectClause, -Cost)
translate(Query groupby Attrs,
        groupby(sortby(Stream, AttrNamesSort), AttrNamesGroup, Fields),
        select Select2, Update, Cost) :-
  translate(Query, Stream, SelectClause, Update, Cost), % Added by Goehr
  makeList(Attrs, Attrs2),
  attrnames(Attrs2, AttrNamesGroup),
  attrnamesSort(Attrs2, AttrNamesSort),
  SelectClause = (select Select),
  makeList(Select, SelAttrs),
  % translateFields(SelAttrs, Attrs2, Fields, Select2), % Original Code
  translateFields(SelAttrs, Attrs2, Fields, Select2,_,_), % change by Goehr
  !.

% insert query
translate(insert into Rel values Val,
        Stream, select*, [], 0) :-
  updateIndex(insert, Rel, inserttuple(Rel, Val), Stream),
  !.

% delete query
translate(delete from Rel where Condition,
        Stream, Select, delete from Rel, Cost) :-
  translate(select * from Rel where Condition, Stream, Select, _, Cost),
  !.

% delete query
translate(delete from Rel,
        Stream, Select, delete from Rel, Cost) :-
  translate(select * from Rel, Stream, Select, _, Cost),
  !.

% update query
translate(update Rel set Transformations where Condition,
        Stream, Select, update Rel set Transformations2, Cost) :-
  translate(select * from Rel where Condition, Stream, Select, _, Cost),
  translateTransformations(Transformations, Transformations2),
  !.

% update query
translate(update Rel set Transformations,
        Stream, Select, update Rel set Transformations2, Cost) :-
  translate(select * from Rel, Stream, Select, _, Cost),
  translateTransformations(Transformations, Transformations2),
  !.

%LargeQueries start:

translate(Select from Rels where Preds, Stream, Select2, Update, Cost) :-
  optimizerOption(largeQueries(aco)),
  length(Preds, NumberOfPreds),
  NumberOfPreds > 10,
  aco(Select from Rels where Preds, Stream, Select2, Update, Cost).

%LargeQueries end

% NVK ADDED MA
/*
----
translate(Select from Rels where Preds orderby OrderAtts, Stream, Select2, Update, Cost) :-
  not( optimizerOption(immediatePlan) ),
  not( optimizerOption(intOrders(_))  ),  % standard behaviour
  optimizerOption(memoryAllocation), % NVK ADDED MA
  getTime(( pog(Rels, Preds, _, _),
            %assignCosts, !, % this is now done within maBestPlan.
            addNewHighNode(N, sortby(res(N), OrderAtts)),
            bestPlan(Stream, Cost),
            !
          ), Time),
  ( optimizerOption(pathTiming)
    -> ( write('\nTIMING for path creation: '), write(Time), write(' ms.\n') )
     ; true
  ),
  splitSelect(Select, Select2, Update),
  !.
----

*/

translate(Select from Rels where Preds, Stream, Select2, Update, Cost) :-
  not( optimizerOption(immediatePlan) ),
  not( optimizerOption(intOrders(_))  ),  % standard behaviour
  optimizerOption(memoryAllocation), % NVK ADDED MA

  getTime(( pog(Rels, Preds, _, _),
            %assignCosts, !, % this is now done within maBestPlan.
            bestPlan(Stream, Cost),
            !
          ), Time),
  ( optimizerOption(pathTiming)
    -> ( write('\nTIMING for path creation: '), write(Time), write(' ms.\n') )
     ; true
  ),
  splitSelect(Select, Select2, Update),
  !.
% NVK ADDED MA END

translate(Select from Rels where Preds, Stream, Select2, Update, Cost) :-
  not( optimizerOption(immediatePlan) ),
  not( optimizerOption(intOrders(_))  ),  % standard behaviour
  \+ optimizerOption(memoryAllocation), % NVK ADDED MA
  getTime(( pog(Rels, Preds, _, _),
            assignCosts, !,
            bestPlan(Stream, Cost),
            !
          ), Time),
  ( optimizerOption(pathTiming)
    -> ( write('\nTIMING for path creation: '), write(Time), write(' ms.\n') )
     ; true
  ),
  splitSelect(Select, Select2, Update),
  !.

/*
Modified version to integrate the optional generation of immediate plans.
The option is activated, when ~optimizerOption(immediatePlan)~ is
defined. See file ``immediateplan.pl'' for further information.

The rule is also used when using the ``interesting orders extension''.
To activate this option use ``setOption(intOrders(V))'' for a variant ~V~.

*/

translate(Select from Rels where Preds, Stream, Select2, Update, Cost) :-
  ( optimizerOption(immediatePlan) ; optimizerOption(intOrders(_)) ),
  getTime(( immPlanTranslate(Select from Rels where Preds, Stream,Select,Cost),
            !
          ), Time),
  ( optimizerOption(pathTiming)
    -> ( write('\nTIMING for path creation: '), write(Time), write(' ms.\n') )
     ; true
  ),
  splitSelect(Select, Select2, Update), !.

/*
Below we handle the case of queries without where-clause. This results
in simple Cartesian products of the relations in the from-clause.
This case is not very important and we don't want to make the code complicated by
applying projections for removing unnecessary attributes which might be a performance
benefit if a groupby- or orderby- clause is present. The product will be computed
by the symmjoin operator with a constant filter function which always returns true.
This operator works well and has symmetric costs, whereas product has antisymmetric
costs.

C. Duentgen, Feb/17/2006: changed tuple variable names for the sake of uniqueness
                          (otherwise, a triple-product will crash).

*/

translate(Select from Rel, Stream, Select2, Update, 0) :-
  not(is_list(Rel)),
  makeStream(Rel, Stream),
  (optimizerOption(pathTiming)
    -> write('\nTIMING for path creation: No optimization.\n') ; true
  ), splitSelect(Select, Select2, Update), !.

translate(Select from [Rel], Stream, Select2, Update, 0) :-
  makeStream(Rel, Stream),
  deleteVariables,
  (optimizerOption(pathTiming)
    -> write('\nTIMING for path creation: No optimization.\n') ; true
  ), splitSelect(Select, Select2, Update).


translate(Select from [Rel | Rels],
        symmjoin(S1, S2, fun([param(T1, tuple), param(T2, tuple2)], true)),
        Select2, Update, 0) :-
  makeStream(Rel, S1),
  translate1(Select from Rels, S2, Select2, Update, _),
  newVariable(T1),
  newVariable(T2),
  !.

% special handling for distance-queries
makeStream(Rel, distancescan(IndexName, Rel, Attr, HeadCount)) :-
  Rel = rel(_, *),
  distanceRel(Rel, IndexName, Attr, HeadCount), !.

% special handling for distance-queries
makeStream(Rel, rename(distancescan(IndexName, Rel, Attr, HeadCount), Var)) :-
  Rel = rel(_, Var),
  distanceRel(Rel, IndexName, Attr, HeadCount), !.

makeStream(Rel, feed(Rel)) :- 
  \+ optimizerOption(nestedRelations), % NVK ADDED
	Rel = rel(_, *), !.

makeStream(Rel, rename(feed(Rel), Var)) :- 
  \+ optimizerOption(nestedRelations), % NVK ADDED
	Rel = rel(_, Var).

/*
NVK ADDED NR
arel attributes of the NestedRelation Algebra needs a afeed instead of a feed.

*/
makeStream(Rel, Plan) :-
  optimizerOption(nestedRelations),
  Rel = rel(Name, *),
  atomic(Name),
  Plan = feed(Rel),
  !.

makeStream(Rel, Plan) :-
  optimizerOption(nestedRelations),
  Rel = rel(Name, Var),
  atomic(Name),
  Plan=rename(feed(Rel), Var).

makeStream(RelT, Stream3) :-
  RelT=rel(irrel(_, Stream1, TOP, _, _, _, _), Var),
  addTransformationOperator(Stream1, TOP, Stream2),
	addRenameOperator(Stream2, Var, Stream3).
% NVK ADDED NR/MA

% Catch this delayed error coz it is very nasty to track.
makeStream(RelTerm, _) :-
  RelTerm=..[where, _, _],
  throw(error_Internal(optimizer_makeStream(RelTerm, _)::malformedExpression:
   'where at beginning')).

makeStream(RelTerm, _) :-
  optimizerOption(nestedRelations),
  throw(error_Internal(optimizer_makeStream(RelTerm, _)::failed)).
	
% NVK ADDED NR/MA END
% NVK ADDED NR END

/*
Begin Code added by Goehr

*/

% Delete all attributes from ~SelectClause~ which
% otherwise would be extended in the finish part.
delExtends([],[]).

delExtends([Name|Select],[Name|NewAttr]) :-
  attr(_,_,_) = Name,
  delExtends(Select,NewAttr),
  !.

delExtends([_ as Name|Select],[Name|NewAttr]) :-
  delExtends(Select,NewAttr),
  !.

/*
---- getProjectAttrs(+SelectClause,+GroupAttrs,-FExtend,-ProjectAttrs)
----

getProjectAttrs is part of the earlyproject optimizerOption
and gathers all attribute which must be included in
the projectextend before sortby.

*/
getProjectAttrs(_,[],[],[]).


getProjectAttrs(Select,GroupAttrs,[field(SName,SExpr)|ExtendAttrs],
                                  [SName|ProjectAttrs]) :-
  Select = [SExpr as SName |RestSelect],
  GroupAttrs = [GExpr|RestGroup],
  SExpr = GExpr,
  getProjectAttrs(RestSelect,RestGroup,ExtendAttrs,ProjectAttrs),
  !.

getProjectAttrs(Select,GroupAttrs,ExtendAttrs,[SName|ProjectAttrs]) :-
  Select = [SExpr as SName |_],
  GroupAttrs = [GExpr|RestGroup],
  SExpr \= GExpr,
  getProjectAttrs(Select,RestGroup,ExtendAttrs,ProjectAttrs),
  !.

% ---- getProjectAttrs1(+Select,+GroupAttrs,+ProjAttrs,-IntersAttrs)
% ----
getProjectAttrs1(Select,GroupAttrs,GroupAttrs2,AExtend) :-
  GroupAttrs2 = [attr(_,_,_)|_],
  list_to_set(Select,SelectSet),
  list_to_set(GroupAttrs,GroupAttrsSet),
  intersection(SelectSet,GroupAttrsSet,Intersect),
  append(Intersect,GroupAttrs2,AExtend),
  !.

getProjectAttrs1(_,_,[],[]).


/*
End Code added by Goehr

*/

/*
The next predicate finds all attributes of a given relation ~Rel~
which are needed in this query. The result can be used to create
project(feed(...)) streams instead of simply feeding all attributes
from a relation into a stream. The system predicate ~setof~ is used
to find all goal for query ~usedAttr(Rel,X)~.

NVK NOTE 
To handle nested relations this needs to work for terms like rel(irrel(...), Var). This is done by adding the facts in the appropriate way.
NVK NOTE END

*/
%   usedAttrList(+Rel, -ResList)
usedAttrList(Rel, ResList) :-
  setof(X, usedAttr(Rel, X), R1),
  %nl, write('AttrList: '), write(R1), nl,
  attrnames(R1, ResList).

renameAttributes(_, [], []).
renameAttributes(Var, [attrname(attr(Attr,Arg,Case))|AttrNames],
                      [attrname(attr(Var:Attr,Arg,Case))|RenamedAttrNames]
                ) :-
  renameAttributes(Var, AttrNames, RenamedAttrNames), !.

/* Begin Code modified by Goehr */

/*
----  translateFields(+Select, +GroupAttrs,
                      -Fields, -Select2, -ExtendAttrs, -ProjectAttrs)
----

Translate the ~Select~ clause of a query containing ~groupby~. Grouping
was done by the attributes ~GroupAttrs~. Return a list ~Fields~ of terms
of the form ~field(Name, Expr)~; such a list can be used as an argument to the
groupby operator. Also, return a modified select clause ~Select2~,
which will translate to a corresponding projection operation.

*/

translateFields([], _, [], [],_,_).

% case: attribute without rename
translateFields([Attr | Select], GroupAttrs, Fields, [Attr | Select2],
                ExtendAttrs, ProjectAttrs) :-
  member(Attr, GroupAttrs),
  !,
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs).

% case: attribute with rename
translateFields([Attr as Name | Select], GroupAttrs, Fields,
                [Attr as Name | Select2], ExtendAttrs, ProjectAttrs) :-
  member(Attr, GroupAttrs),
  !,
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs).

/*
Aggregations using ~count~ need a translation different from other
simple aggregation operators:

*/

% case: count(*) / count(all *) with rename
translateFields([count(*) as NewAttr | Select], GroupAttrs,
        [field(NewAttr , count(feed(group))) | Fields], [NewAttr | Select2],
        ExtendAttrs, ProjectAttrs) :-
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  !.
translateFields([count(all *) as NewAttr | Select], GroupAttrs,
        [field(NewAttr , count(feed(group))) | Fields], [NewAttr | Select2],
        ExtendAttrs, ProjectAttrs) :-
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  !.

% case: count(distinct *) with rename
translateFields([count(distinct *) as NewAttr | Select], GroupAttrs,
        [field(NewAttr , count(rdup(sort(feed(group))))) | Fields],
        [NewAttr | Select2],ExtendAttrs, ProjectAttrs) :-
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  !.

% case: count(attr) / count(all attr) with rename
% (does not count rows, where the attr is undefined)
translateFields([count(Term) as NewAttr | Select], GroupAttrs,
  [field(NewAttr,count(filter(feed(group),not(isempty(attr(A,B,C))))))|Fields],
  [NewAttr | Select2],[],[Term|Term2]) :-
  optimizerOption(earlyproject),
  ( Term = attr(A,B,C) ; Term = (all attr(A,B,C)) ),
  translateFields(Select, GroupAttrs, Fields, Select2,[],Term2),
  !.

translateFields([count(Term) as NewAttr | Select], GroupAttrs,
  [field(NewAttr,count(filter(feed(group),not(isempty(attr(A,B,C))))))|Fields],
  [NewAttr | Select2],ExtendAttrs, ProjectAttrs) :-
  ( Term = attr(A,B,C) ; Term = (all attr(A,B,C)) ),
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  !.

% case: count(distinct attr) with rename
% (does only count rows with defined and distinct attr values)
translateFields([count(distinct attr(A,B,C)) as NewAttr | Select], GroupAttrs,
  [field(NewAttr,count(rdup(sort(project(filter(feed(group),
  not(isempty(attr(A,B,C)))),AttrName)))))|Fields],
  [NewAttr | Select2],ExtendAttrs, ProjectAttrs) :-
  attrnames([attr(A,B,C)], AttrName),
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  !.

% case: count(distinct expr) with rename
translateFields([count(distinct Expr) as NewAttr | Select], GroupAttrs,
          [field(NewAttr,count(CountStream))|Fields], [NewAttr | Select2],
          ExtendAttrs, ProjectAttrs) :-
  compound(Expr),
  newVariable(ExpAttrName),
  Expr \= attr(_,_,_),
  AttrExtStream = extend(feed(group),
                         [newattr(attrname(attr(ExpAttrName, 1, l)), Expr)]),
  CountStream = rdup(sort(filter(AttrExtStream,
                                 not(isempty(attr(ExpAttrName, 1, l)))))),
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  !.

% case: count(expr) / count(all expr) with rename
translateFields([count(Term) as NewAttr | Select], GroupAttrs,
          [field(NewAttr,count(CountStream))|Fields], [NewAttr | Select2],
          ExtendAttrs, ProjectAttrs) :-
  (Term = (all Expr) ; Term = Expr),
  compound(Expr),
  Expr \= attr(_,_,_),
  newVariable(ExpAttrName),
  AttrExtStream = extend(feed(group),
                         [newattr(attrname(attr(ExpAttrName, 1, l)), Expr)]),
  CountStream = filter(AttrExtStream ,not(isempty(attr(ExpAttrName, 1, l)))),
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  !.

/*
Generic rules for simple predifined aggregation functions, similar to sum,
min, max, avg.
For these operators, the property ~isAggregationOP(Op)~ is declared in file
``operators.pl''. For ``count'', there are special cases (see above).
These aggregation functions are expected to ignore undefined values and
therefore we can do without filter(X,not(isempty(Y))).

*/

% case: simple predefined aggregation functions (always with rename!)
%       aggrop(distinct attr) with rename
translateFields([Term as NewAttr | Select], GroupAttrs,
          [field(NewAttr, Term2) | Fields], [NewAttr | Select2],
          ExtendAttrs, ProjectAttrs) :-
  compound(Term),
  Term =.. [AggrOp, (distinct attr(Name, Var, Case))],
  isAggregationOP(AggrOp),
  attrnames([attr(Name, Var, Case)], AttrName),
  AggStream = rdup(sort(project(feed(group),AttrName))),
  Term2 =.. [AggrOp, AggStream, attrname(attr(Name, Var, Case))],
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  !.

% case: simple predefined aggregation functions (always with rename!)
%       aggrop(attr) / aggrop(all attr) with rename
translateFields([Term as NewAttr | Select], GroupAttrs,
        [field(NewAttr, Term2) | Fields], [NewAttr| Select2],
        ExtendAttrs, ProjectAttrs) :-
  compound(Term),
  (   Term =.. [AggrOp, (all attr(Name, Var, Case))]
    ; Term =.. [AggrOp, attr(Name, Var, Case)]
  ),
  isAggregationOP(AggrOp),
  Term2 =.. [AggrOp, feed(group), attrname(attr(Name, Var, Case))],
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  !.

% case: simple predefined aggregation functions
%       over expression (always with rename!)
%       aggrop(distinct expr) with rename
translateFields([Term as NewAttr | Select], GroupAttrs,
          [field(NewAttr, Term2) | Fields], [NewAttr | Select2],
          ExtendAttrs, ProjectAttrs) :-
  compound(Term),
  Term =.. [AggrOp, (distinct Expr)],
  isAggregationOP(AggrOp),
  Expr \= attr(_,_,_),
  newVariable(ExprAttr),
  attrnames([attr(ExprAttr, 0, l)], ExprAttrName),
  AttrExtStream = extend(feed(group),
                         [newattr(attrname(attr(ExprAttr, 0, l)), Expr)]),
  AggStream = rdup(sort(project(AttrExtStream, ExprAttrName))),
  Term2 =.. [AggrOp, AggStream, attrname(attr(ExprAttr, 0, l))],
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  !.

% case: simple predefined aggregation functions
%       over expression (always with rename!)
%       aggrop(expr) / aggrop(all expr) with rename
translateFields([Term as NewAttr | Select], GroupAttrs,
          [field(NewAttr, Term2) | Fields], [NewAttr | Select2],
          [ExtendAttrs|ExtendAttrs2],[]) :-
  optimizerOption(earlyproject),
  compound(Term),
  ( Term =.. [AggrOp, (all Expr)]; Term =.. [AggrOp, Expr] ),
  isAggregationOP(AggrOp),
  Expr \= attr(_,_,_),
  newVariable(ExprAttr),
  AttrExtStream = feed(group),
  Term2 =.. [AggrOp, AttrExtStream, attrname(attr(ExprAttr, 0, u))],
  ExtendAttrs = field(attr(ExprAttr, 0, u),Expr),
  translateFields(Select, GroupAttrs, Fields, Select2, ExtendAttrs2,[]),
  !.

translateFields([Term as NewAttr | Select], GroupAttrs,
  [field(NewAttr, Term2) | Fields], [NewAttr | Select2],_,_) :-
  compound(Term),
  ( Term =.. [AggrOp, (all Expr)]; Term =.. [AggrOp, Expr] ),
  isAggregationOP(AggrOp),
  Expr \= attr(_,_,_),
  newVariable(ExprAttr),
  AttrExtStream = extend(feed(group),
                         [newattr(attrname(attr(ExprAttr, 0, u)), Expr)]),
  Term2 =.. [AggrOp, AttrExtStream, attrname(attr(ExprAttr, 0, u))],
  translateFields(Select, GroupAttrs, Fields, Select2,_,_),
  !.

% case: ERROR simple predefined aggregation functions (missing new name)
translateFields([Term | Select], GroupAttrs, Fields, Select2,
                ExtendAttrs, ProjectAttrs) :-
  compound(Term),
  Term =.. [AggrOp, _],
  isAggregationOP(AggrOp),
%  Term2 =.. [AggrOp, feed(group), attrname(Attr)],
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  concat_atom(['Malformed expression: Missing name for aggregation ',
               'expression.'],ErrMsg), nl,
  write_list(['ERROR: ',ErrMsg]),
  throw(error_SQL(optimizer_translateFields([Term | Select], GroupAttrs,
                                            Fields, Select2, ExtendAttrs,
                                            ProjectAttrs)
                                           ::malformedExpression::ErrMsg)),
  !.

/*
Generic rules for user defined aggregation functions, using
~aggregate~.

The SQL-syntax is as follows:

  aggregate(ARGUMENT, FUNCTION, TYPE, DEFAULTVALUE)

  ARGUMENT is either a valid attribute, or an expression. Additional, you may
  use preceeding ~all~ or ~distinct~ to aggregate over all resp. only distinct
  attribute resp. expression values.

  FUNCTION is the operator name (only the name!) used as the aggregation
  function. The operator must have signature TYPE x TYPE [->] TYPE. It must
  be a commutative and associative function. If you want to use
  an infix operator, you must enclose it in round parentheses, eg. ([star]) for
  the multiplication [star]: TYPE x TYPE [->] TYPE.

  TYPE is the datatype processed by the aggregation function.

  DEFAULTVALUE is a value of type TYPE.

  Otherwise, you can use user defined aggregation in the select clause of
  a query, just like ordinary aggregation operator, like ~sum~, ~avg~, ~var~,
  ~min~, or ~max~.

  No filtering for definedness of attribute values is done. If required, you
  have to insert an additional consition into the where-clause
  (e.g. ~not(isempty(ARGUMENT))~).

  As usual for SQL, aggregation is only allowed in the context of grouping!

*/

% case: complex/user defined arbitrary aggregation functions
%       over distinct attribute (without defined-filtering)
translateFields([Term as NewAttr | Select], GroupAttrs,
        [field(NewAttr, Term2) | Fields],
        [NewAttr| Select2],ExtendAttrs, ProjectAttrs) :-
  compound(Term),
  Term =.. [AggrOp, (distinct attr(Name, Var, Case)), FunOp, Type, Default],
  member(AggrOp, [aggregate]),
  newVariable(Var1),
  newVariable(Var2),
  AggrFun =.. [FunOp, Var1, Var2],
  attrnames([attr(Name, Var, Case)], AttrName),
  Term2 =..[AggrOp,
            rdup(sort(project(feed(group), AttrName))),
            attrname(attr(Name, Var, Case)),
            fun([ param(Var1, Type), param(Var2, Type)], AggrFun),
            Default],
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  !.

% case: complex/user defined arbitrary aggregation functions over
%       all attribute (without defined-filtering)
translateFields([Term as NewAttr | Select], GroupAttrs,
        [field(NewAttr, Term2) | Fields],
        [NewAttr| Select2],ExtendAttrs, ProjectAttrs) :-
  compound(Term),
  (   Term =.. [AggrOp, (all attr(Name, Var, Case)), FunOp, Type, Default]
    ; Term =.. [AggrOp,      attr(Name, Var, Case) , FunOp, Type, Default]
  ),
  member(AggrOp, [aggregate]),
  newVariable(Var1),
  newVariable(Var2),
  AggrFun =.. [FunOp, Var1, Var2],
  Term2 =..[AggrOp,
            feed(group),
            attrname(attr(Name, Var, Case)),
            fun([ param(Var1, Type), param(Var2, Type)], AggrFun),
            Default],
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  !.

% case: complex/arbitrary aggregation functions over distinct expression
%       over distinct attribute (without defined-filtering)
translateFields([Term as NewAttr | Select], GroupAttrs,
        [field(NewAttr, Term2) | Fields],
        [NewAttr| Select2],ExtendAttrs, ProjectAttrs) :-
  compound(Term),
  Term =.. [AggrOp, (distinct Expr), FunOp, Type, Default],
  member(AggrOp, [aggregate]),
  Expr \= attr(_,_,_),
  newVariable(ExprAttr),
  newVariable(Var1),
  newVariable(Var2),
  AggrFun =.. [FunOp, Var1, Var2],
  attrnames([attr(ExprAttr, 0, l)], ExprAttrName),
  ExtStream  = extend(feed(group), field(attr(ExprAttr, 0, l), Expr)),
  AggrStream = rdup(sort(project(ExtStream, ExprAttrName) ) ),
  Term2 =.. [AggrOp,
             AggrStream,
             attrname(attr(ExprAttr, 0, l)),
             fun([param(Var1, Type),param(Var2, Type)],AggrFun),
             Default],
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  !.

% case: complex/arbitrary aggregation functions over all expression
%       over distinct attribute (without defined-filtering)
translateFields([Term as NewAttr | Select], GroupAttrs,
        [field(NewAttr, Term2) | Fields],
        [NewAttr| Select2],ExtendAttrs, ProjectAttrs) :-
  compound(Term),
  (   Term =.. [AggrOp, (all Expr), FunOp, Type, Default]
    ; Term =.. [AggrOp, Expr, FunOp, Type, Default]
  ),
  Expr \= attr(_,_,_),
  member(AggrOp, [aggregate]),
  newVariable(ExpAttrName),
  newVariable(Var1),
  newVariable(Var2),
  AggrFun =.. [FunOp, Var1, Var2],
  Term2 =.. [AggrOp,
             extend(feed(group), field(attr(ExpAttrName, 0, l), Expr)),
             attrname(attr(ExpAttrName, 0, l)),
             fun([param(Var1, Type),param(Var2, Type)],AggrFun),
             Default],
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttrs),
  !.

% case: ERROR complex/user defined arbitrary aggregation functions
%       missing new name
translateFields([Term | Select], GroupAttrs, Fields, Select2,
                ExtendAttrs, ProjectAttr) :-
  compound(Term),
  Term =.. [AggrOp, _, _, _],
  member(AggrOp, [aggregate]),
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttr),
  term_to_atom(Term,XA),
  concat_atom(['Malformed expression: Missing name for aggregation ',
               'expression in \'',XA,'\'.'],'',ErrMsg), nl,
  write_list(['ERROR: ',ErrMsg]),
  throw(error_SQL(optimizer_translateFields([Term | Select], GroupAttrs,
                                            Fields, Select2,
                                            ExtendAttrs, ProjectAttr)
                                           ::malformedExpression::ErrMsg)),
  !.

% case: ERROR (general error case)
translateFields([Attr | Select], GroupAttrs, Fields, Select2,
                ExtendAttrs, ProjectAttr) :-
  not(member(Attr, GroupAttrs)),
  !,
  translateFields(Select, GroupAttrs, Fields, Select2,
                  ExtendAttrs, ProjectAttr),
  term_to_atom(Attr,XA),
  concat_atom(['Malformed expression: ',XA,' is neither a grouping attribute ',
               'nor an aggregate expression.'],'',ErrMsg), nl,
  write_list(['ERROR: ',ErrMsg]),
  throw(error_SQL(optimizer_translateFields([Attr | Select], GroupAttrs,
                                            Fields, Select2,
                                            ExtendAttrs, ProjectAttr)
                                           ::malformedExpression::ErrMsg)),
  !.

% case: ERROR (fallback error case)
translateFields(X, GroupAttrs, Fields, Select2, ExtendAttrs, ProjectAttr) :-
  term_to_atom(X,XA),
  concat_atom(['Malformed expression in fields: \'', XA, '\'.'],'',ErrMsg), nl,
  write_list(['ERROR: ',ErrMsg]),
  throw(error_SQL(optimizer_translateFields(X, GroupAttrs, Fields, Select2,
                                            ExtendAttrs, ProjectAttr)
                                           ::malformedExpression::ErrMsg)),
  !.

/* End Code modified by Goehr */

/*
----    translateTransformations(+Transf, -Transf2)
----

Translates the transformations in an update-command

*/

translateTransformations([], []) :-
  !.

translateTransformations([Transf|Rest], [Transf2|Rest2]) :-
  translateTransformation(Transf, Transf2),
  translateTransformations(Rest, Rest2).

/*
----  translateTransformation(+Transf, -Transf2)
----

Translates one transformation in an update-command

*/

translateTransformation(Attr = Term, set(Attr, Term)).


/*
----    translateColumns(+Col, -Col2)
----

Translates the columns in a create table command

*/

translateColumns([], []) :-
  !.

translateColumns([Column|Rest], [Column2|Rest2]) :-
  translateColumn(Column, Column2),
  translateColumns(Rest, Rest2),
  !.

/*
----    translateColumn(+Col, -Col2)
----

Translates one column in a create table command

*/

/*
NVK ADDED NR

*/

translateColumn(Name:Type, column(Name, arel(LColsT))) :-
  optimizerOption(nestedRelations),
  translateType(Type, Type2), 
	Type2=..[arel, Cols],
	makeList(Cols, LCols),
	translateColumns(LCols, LColsT),
	!.

translateColumn(Name: Type, column(Name, Type2)) :-
  optimizerOption(nestedRelations),
  translateType(Type, Type2), 
	\+ Type2 =..[arel|_],
	!.
% NVK ADDED NR END

translateColumn(Name: Type, column(Name, Type2)) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED
  translateType(Type, Type2), !.


/*
----    translateType(+Type, -Type2)
----

Translates an SQL-Type to a SECONDO-Type

*/

translateType(integer, int) :- !.
translateType(boolean, bool) :- !.

translateType(Type, Type) :- !.

/*
----    translateDistanceQuery(+Query, +DistAttr1, +DistAttr2,
          +HeadCount, -Stream, -Select, -Update, -Cost)
----

Translates a query which is ordered by the distance between ~DistAttr1~
and ~DistAttr2~. ~HeadCount~ is the number of object to look for in the
distance query. ~assertDistanceRel~ is used to check for an
R-Tree-Index.

*/

translateDistanceQuery(Select from Rel, X, Y, HeadCount,
         StreamOut, SelectOut, Update, Cost) :-
  Rel = rel(_, _),
  assertDistanceRel([Rel], X, Y, HeadCount), !,
  translate1(Select from [Rel], StreamOut, SelectOut, Update, Cost),
  retractall(distanceRel(Rel, _, _, HeadCount)).

/*

If the query has a where-clause and an R-Tree-Index can be used, the POG
is calculated twice: One version using the index and one version not
using the index. The costs of these two solutions are compared in
~chooseFasterSolution~ and the faster one is returned


*/

translateDistanceQuery(Select from Rels where Condition, X, Y,
         HeadCount, StreamOut, SelectOut, Update, Cost) :-
  assertDistanceRel(Rels, X, Y, 0),
  translate1(Select from Rels where Condition, StreamRTree, SelectOut1,
      Update1, CostRTree),
  finishDistanceSortRTree(StreamRTree, CostRTree, HeadCount, StreamOut1,
     Cost1), !,
  retractall(distanceRel(_, _, _, _)),
  translate1(Select from Rels where Condition, StreamTmp, SelectOut2,
      Update2, CostTmp),
  finishDistanceSort(StreamTmp, CostTmp, X, Y, HeadCount, StreamOut2, Cost2),
  write('Best Solution using distancescan: '), nl,
  write(StreamOut1),nl,
  write('cost: '), write(Cost1), nl, nl,
  write('Best Solution without distancescan: '), nl,
  write(StreamOut2),nl,
  write('cost: '), write(Cost2), nl, nl,
  chooseFasterSolution(StreamOut1, SelectOut1, Update1, Cost1, StreamOut2,
         SelectOut2, Update2, Cost2, StreamOut, SelectOut,
         Update, Cost),
  (StreamOut1 = StreamOut; retractall(planvariable(_, _))).

/*

If the query can't use an R-Tree-Index, use ~sortby~ or ~ksmallest~
(dependent on the value of ~HeadCount~)

*/

translateDistanceQuery(Query, X, Y, 0, StreamOut,
         Select, Update, Cost) :-
  retractall(distanceRel(_, _, _, _)),
  translate1(Query, Stream, Select, Update, Cost),
  newVariable(ExprAttrName),
  ExprAttr = attr(ExprAttrName, *, l),
  StreamOut = remove(sortby(extend(Stream, [field(ExprAttr, distance(X, Y))]),
       attrname(ExprAttr)),
       attrname(ExprAttr)).

translateDistanceQuery(Query, X, Y, HeadCount, StreamOut,
         Select, Update, Cost) :-
  retractall(distanceRel(_, _, _, _)),
  translate1(Query, Stream, Select, Update, Cost),
  newVariable(ExprAttrName),
  ExprAttr = attr(ExprAttrName, *, l),
  StreamOut = remove(ksmallest(extend(Stream,
          [field(ExprAttr, distance(X, Y))]),
          HeadCount, attrname(ExprAttr)),
       attrname(ExprAttr)).


/*

----    queryToPlan(+Query, -Plan, -Cost) :-
----

Translate the ~Query~ into a ~Plan~. The ~Cost~ for evaluating the conjunctive
query is also returned. The ~Query~ must be such that relation and attribute
names have been looked up already.

The predicate differentiates between normal consume-queries, simple count
queries and predefined/user-defined aggregation queries (at most one unnamed
aggregation, without an expression, but respecting all and distinct) that come
without a groupby clause.

*/

% case: create table query
queryToPlan(create table TableName columns Columns,
     let(TableName, rel(Columns2)), 0) :-
  translateColumns(Columns, Columns2),
  !.

% case: drop table query
queryToPlan(drop table [TableName], Result, 0) :-
  finishUpdate(drop table [TableName], delete(TableName), Result), !.

% case: create index query
queryToPlan(create index on [Rel] columns ColumnList indextype IndexType,
     createIndex(Rel, ColumnList2, IndexType), 0) :-
  dissolveList(ColumnList, ColumnList2), !.

% case: create index query
queryToPlan(create index on [Rel] columns ColumnList,
     Result, Cost) :-
  dissolveList(ColumnList, ColumnList2),
  convertToLfName(ColumnList2, LfColumnList),
  convertToLfName(Rel, LfRel),
  keyAttributeTypeMatchesIndexType(LfRel, LfColumnList, LogIndexType),
  queryToPlan(create index on [Rel] columns ColumnList indextype LogIndexType,
       Result, Cost), !.

% case: drop index query
queryToPlan(drop index on [Rel] columns ColumnList indextype IndexType,
     Result, Cost) :-
  dissolveList(ColumnList, ColumnList2),
  findall(DCIndex, hasIndex(Rel, ColumnList2, DCIndex, IndexType), IndexList),
  queryToPlan(drop index IndexList, Result, Cost), !.

% case: drop index query
queryToPlan(drop index on [Rel] columns ColumnList, Result, Cost) :-
  queryToPlan(drop index on [Rel] columns ColumnList indextype _, Result,
       Cost), !.

% case: drop index query
queryToPlan(drop index [], [], 0) :-
  write('\nError:\tNo index found'), nl,
  throw(error_SQL(optimizer_queryToPlan(drop index [], [], 0)::noIndex)),
  fail.

% case: drop index query
queryToPlan(drop index [DCTableName], delete(TableName), 0) :-
  dcName2externalName(DCTableName, TableName), !.

queryToPlan(drop index IndexList, [], 0) :-
  writeList(['\nError:\tToo many indexes found: ', IndexList]),
  nl,
  throw(error_SQL(optimizer_queryToPlan(drop index IndexList, [], 0)
    ::multipleIndexes)),
  fail.

% case: count query
% special case: just inquring the cardinality of a relation
queryToPlan(select count(*) from Rel, count(Rel), 0):-
  Rel = rel(R, _),
  not(is_nrel(R)),
  !.

% general counting query
queryToPlan(Query, StreamOut, Cost) :-
  countQuery(Query),
  queryToStream(Query, Stream, Cost), !,
  addTmpVariables(count(Stream), StreamOut),
  write('Query = '), write(Query), nl,
  write('Stream = '), write(Stream), nl,
  write('StreamOut = '), write(StreamOut), nl.
 

% case: predefined aggregation query (New Method)
queryToPlan(Query, StreamOut, Cost) :-
  aggrQuery(Query, Op, Query1, AggrExpr),
  queryToStream(Query1, Stream, Cost),
  Stream2 =.. [simpleAggrNoGroupby, Op, Stream, AggrExpr], !,
  addTmpVariables(Stream2, StreamOut).

% case: userdefined aggregation query (New Method)
queryToPlan(Query, StreamOut, Cost) :-
  userDefAggrQuery(Query, Query1, AggrExpr, Fun, Default),
  queryToStream(Query1, Stream, Cost),
  Stream2 =.. [simpleUserAggrNoGroupby, Stream, AggrExpr, Fun, Default], !,
  addTmpVariables(Stream2, StreamOut).

% case: update query
queryToPlan(Query, StreamOut, Cost) :-
  updateQuery(Query),
  queryToStream(Query, Stream, Cost), !,
  addTmpVariables(count(Stream), StreamOut).

% case: ordinary consume query
queryToPlan(Query, StreamOut, Cost) :-
  \+ optimizerOption(nestedRelations),
  queryToStream(Query, Stream, Cost), !,
  addTmpVariables(consume(Stream), StreamOut).

/*
NVK ADDED NR
All plan for subqueries needs to be consumed with the aconsume operator. Even if the stream wasn't feed with the afeed operator.

*/
queryToPlan(Query, StreamOut, Cost) :-
  optimizerOption(nestedRelations),
  queryToStream(Query, Stream, Cost), !,
  ensure((consumeStream(Stream, SConsumed),
  addTmpVariables(SConsumed, StreamOut))).

consumeStream(Stream, aconsume(Stream)) :-
  getSubqueryCurrent(N),
  N>0,
  !.
consumeStream(Stream, consume(Stream)) :-
  !.
% NVK ADDED NR END

/*
Check whether ~Query~ is a counting query.

*/


countQuery(select count(_) from _) :- !.
countQuery(select all count(_) from _) :- !.      % This is deprecated!
countQuery(select distinct count(_) from _) :- !. % This is deprecated!

countQuery(Query groupby _) :- countQuery(Query).
countQuery(Query orderby _) :- countQuery(Query).
countQuery(Query first _)   :- countQuery(Query).
countQuery(Query last _)   :- countQuery(Query).

/*
----    aggrQuery(+Query, -Op, -Query1, -AggrAttr)
----

If ~Query~ is a simple predefined aggrgation query, it returns the aggregation
functor in ~Op~, the aggregation expression in ~AggrAttr~ and the modified query
in ~Query1~.

*/
aggrQuery(select all T from F, AggrOp, select T1 from F, AggrExpr) :-
  aggrQuery(select T from F, AggrOp, select T1 from F, AggrExpr), !.

aggrQuery(select distinct T from F, AggrOp,
          select distinct T1 from F, AggrExpr) :-
  aggrQuery(select T from F, AggrOp, select T1 from F, AggrExpr), !.

aggrQuery(select T from F, AggrOp, select * from F, AggrExpr) :-
  compound(T),
  T =.. [AggrOp, all AggrExpr],
  isAggregationOP(AggrOp), !.

aggrQuery(select T from F, AggrOp, select distinct * from F, AggrExpr) :-
  compound(T),
  T =.. [AggrOp, distinct AggrExpr],
  isAggregationOP(AggrOp), !.

aggrQuery(select T from F, AggrOp, select * from F, AggrExpr) :-
  compound(T),
  T =.. [AggrOp, AggrExpr],
  isAggregationOP(AggrOp), !.

aggrQuery(Query groupby G, _, _, _) :-
  aggrQuery(Query, _, _, _), !,
  % This is not allowed, simple aggregations have no groupby!
  % Send an error!
  concat_atom(['Expected a simple aggregation, but found a \'groupby\'!\n'],
              '',ErrMsg),
  write_list(['ERROR: ',ErrMsg]),
  throw(error_SQL(optimizer_aggrQuery(Query groupby G, undefined, undefined)
                                           ::malformedExpression::ErrMsg)),
  fail.
aggrQuery(Query orderby Order, AggrOp, Query1 orderby Order, AggrExpr) :-
  aggrQuery(Query, AggrOp, Query1, AggrExpr), !.
aggrQuery(Query first N, AggrOp, Query1 first N, AggrExpr)   :-
  aggrQuery(Query, AggrOp, Query1, AggrExpr), !.
aggrQuery(Query last N, AggrOp, Query1 last N, AggrExpr)   :-
  aggrQuery(Query, AggrOp, Query1, AggrExpr), !.


/*
----  userDefAggrQuery(Query, Query1, AggrExpr, Fun, Default)
----

Check, whether ~Query~ is a simple user defined aggregation query.
If so, return the arguments for the aggregate operator and the modified
~Query1~, agreegation is performed over ~AggrExpr~ using the function with
operator name ~Fun~, and ~Default~ as result for the empty aggregation.

*/

userDefAggrQuery(select all T from F, select T1 from F, A, B, C)
  :- userDefAggrQuery(select T from F, select T1 from F, A, B, C), !.

userDefAggrQuery(select distinct T from F, select distinct T1 from F, A, B, C)
  :- userDefAggrQuery(select T from F, select T1 from F, A, B, C), !.

userDefAggrQuery(select T from F, select * from F, AggrExpr, Fun, Default) :-
  compound(T),
  T =.. [AggrOp, all AggrExpr, FunOp, Type, Default],
  member(AggrOp,[aggregate]),
  newVariable(Var1),
  newVariable(Var2),
  AggrFun =.. [FunOp, Var1, Var2],
  Fun = fun([ param(Var1, Type), param(Var2, Type)], AggrFun), !.

userDefAggrQuery(select T from F, select distinct * from F,
                                                  AggrExpr, Fun, Default) :-
  compound(T),
  T =.. [AggrOp, distinct AggrExpr, FunOp, Type, Default],
  member(AggrOp,[aggregate]),
  newVariable(Var1),
  newVariable(Var2),
  AggrFun =.. [FunOp, Var1, Var2],
  Fun = fun([ param(Var1, Type), param(Var2, Type)], AggrFun), !.

userDefAggrQuery(select T from F, select * from F, AggrExpr, Fun, Default) :-
  compound(T),
  T =.. [AggrOp, AggrExpr, FunOp, Type, Default],
  member(AggrOp,[aggregate]),
  newVariable(Var1),
  newVariable(Var2),
  AggrFun =.. [FunOp, Var1, Var2],
  Fun = fun([ param(Var1, Type), param(Var2, Type)], AggrFun), !.

userDefAggrQuery(Query groupby G, _, _, _, _) :-
  userDefAggrQuery(Query, _, _, _, _), !,
  % This is not allowed, simple aggregations have no groupby!
  % Send an error!
  concat_atom(['Expected a simple aggregation, but found a \'groupby\'!\n'],
              '',ErrMsg),
  write_list(['ERROR: ',ErrMsg]),
  throw(error_SQL(optimizer_userDefAggrQuery(Query groupby G,
                             undefined, undefined, undefined, undefined)
                                    ::malformedExpression::ErrMsg)),
  fail.
userDefAggrQuery(Query orderby Order,
                 Query1 orderby Order, AggrExpr, Fun, Default) :-
  userDefAggrQuery(Query, Query1, AggrExpr, Fun, Default), !.
userDefAggrQuery(Query first N, Query1 first N, AggrExpr, Fun, Default) :-
  userDefAggrQuery(Query, Query1, AggrExpr, Fun, Default), !.
userDefAggrQuery(Query last N, Query1 last N, AggrExpr, Fun, Default) :-
  userDefAggrQuery(Query, Query1, AggrExpr, Fun, Default), !.

/*
----    updateQuery(+Query)
----

Check whether ~Query~ is an update query.

*/

updateQuery(insert into _ values _) :- !.
updateQuery(insert into _ select _ from _) :- !.
updateQuery(delete from _) :- !.
updateQuery(update _ set _) :- !.

updateQuery(Query groupby _) :- updateQuery(Query).
updateQuery(Query orderby _) :- updateQuery(Query).
updateQuery(Query first _)   :- updateQuery(Query).
updateQuery(Query last _)   :- updateQuery(Query).

/*

----    queryToStream(+Query, -Plan, -Cost) :-
----

Same as ~queryToPlan~, but returns a stream plan, if possible.

*/

% special handling for distance-queries
queryToStream(Query orderby [distance(X, Y)] first N, StreamOut, Cost) :-
  !, translateDistanceQuery(Query, X, Y, N, Stream, Select, Update, Cost),
  finish(Stream, Select, [], Stream2),
  finishUpdate(Update, Stream2, StreamOut).

% special handling for distance-queries
queryToStream(Query orderby [distance(X, Y)], StreamOut, Cost) :-
  !, translateDistanceQuery(Query, X, Y, 0, Stream, Select, Update, Cost),
  finish(Stream, Select, [], Stream2),
  finishUpdate(Update, Stream2, StreamOut).

queryToStream(Query first N, head(Stream, N), Cost) :-
  queryToStream(Query, Stream, Cost),
  !.

queryToStream(Query last N, tail(Stream, N), Cost) :-
  queryToStream(Query, Stream, Cost),
  !.

% NVK ADDED MA
/*
The sort operation is an important part of a query, hence it should be respected within the memory optimization.
Queries just with a sort without ~Preds~ are still handled by the below default predicate.

*/

/*
----
queryToStream(Query from Rels where Preds orderby SortAttrs, Stream3, Cost) :-
  optimizerOption(memoryAllocation),
  % Note that this edge is not really important for the dijkstra algorithm because
  % we has to perform this operation, regardless what path we took before.
  % But we have to include this edge for memory optimization, another path with
  % more free memory may better as nother path with less free memory.
  attrnamesSort(SortAttrs, AttrNames),
  translate1(Query from Rels where Preds orderby AttrNames, Stream, Select, Update, Cost),
  %finish(Stream, Select, SortAttrs, Stream2),
  finish(Stream, Select, [], Stream2),
  finishUpdate(Update, Stream2, Stream3),
  !.
----

*/
% NVK ADDED MA END

queryToStream(Query orderby SortAttrs, Stream3, Cost) :-
  translate1(Query, Stream, Select, Update, Cost),
  finish(Stream, Select, SortAttrs, Stream2),
  finishUpdate(Update, Stream2, Stream3),
  !.

queryToStream(Select from Rels where Preds, Stream3, Cost) :-
  translate1(Select from Rels where Preds, Stream, Select1, Update, Cost),
  finish(Stream, Select1, [], Stream2),
  finishUpdate(Update, Stream2, Stream3),
  !.

queryToStream(Select from Rels groupby Attrs, Stream3, Cost) :-
  translate1(Select from Rels groupby Attrs, Stream, Select1, Update, Cost),
  finish(Stream, Select1, [], Stream2),
  finishUpdate(Update, Stream2, Stream3),
  !.

queryToStream(Query, Stream3, Cost) :-
  translate1(Query, Stream, Select, Update, Cost),
  finish(Stream, Select, [], Stream2),
  finishUpdate(Update, Stream2, Stream3),
  !.

queryToStream(Query, Stream, Cost) :-
  optimizerOption(subqueries),
  subqueryToStream(Query, Stream, Cost),
  !.

/*
----    finish(+Stream, +Select, +Sort, -Stream2) :-
----

Given a ~Stream~, a ~Select~ clause, and a set of attributes for sorting,
apply the final tranformations (extend, project, duplicate removal, sorting) to obtain ~Stream2~.

*/

finish(Stream, Select, Sort, Stream2) :-
  selectClause(Select, Extend, Project, Rdup),
  finish2(Stream, Extend, Project, Rdup, Sort, Stream2).

/*
---- selectClause(+Select, -Extend, -Project, -Rdup) :-
----

The ~Select~ clause contains attribute lists ~Extend~ for extension, ~Project~ for projection, and possibly a ~distinct~ keyword indicating duplicate removal returned in ~Rdup~.

*/

% count queries
selectClause(select count(all Attr), Ext, Pro, duplicates ) :-
  selectClause(select count(Attr), Ext, Pro, _ ), !.
selectClause(select count(distinct Attr), Ext, Pro, distinct ) :-
  selectClause(select count(Attr), Ext, Pro, _ ), !.

selectClause(select count(*), [], count(*), duplicates).
selectClause(select count(Attr), Extend, Project, duplicates) :-
  makeList(Attr, Attrs2),
  extendProject(Attrs2, Extend, Project), !.

% single predefined aggregation function query
selectClause(select T, Extend, Project, distinct) :-
  compound(T),
  T =.. [Op, (distinct Attr)],
  isAggregationOP(Op),
  makeList(Attr, Attrs2),
  extendProject(Attrs2, Extend, Project), !.

selectClause(select T, Extend, Project, duplicates) :-
  compound(T),
  ( T =.. [Op, (all Attr)] ; T =.. [Op, Attr] ),
  isAggregationOP(Op),
  makeList(Attr, Attrs2),
  extendProject(Attrs2, Extend, Project), !.

% single userdefined aggregation function query
selectClause(select T, Extend, Project, distinct) :-
  compound(T),
  T =.. [Op, (distinct Attr), _Fun, _Type, _Default],
  member(Op,[aggregate]),
  makeList(Attr, Attrs2),
  extendProject(Attrs2, Extend, Project), !.

selectClause(select T, Extend, Project, duplicates) :-
  compound(T),
  (   T =.. [Op, (all Attr), _Fun1, _Type1, _Default1]
    ; T =.. [Op, Attr, _Fun2, _Type2, _Default2]
  ),
  Attr = attr(_,_,_),
  member(Op,[aggregate]),
  makeList(Attr, Attrs2),
  extendProject(Attrs2, Extend, Project), !.

% generic cases - using recursion
selectClause(select distinct T, Ext, Pro, distinct) :-
  selectClause(select T, Ext, Pro, _), !.
selectClause(select all T, Ext, Pro, Rdup) :-
  selectClause(select T, Ext, Pro, Rdup), !.

selectClause(select *, [], *, duplicates).
selectClause(select Attrs, Extend, Project, duplicates) :-
  makeList(Attrs, Attrs2),
  extendProject(Attrs2, Extend, Project), !.

/*
---- finish2(+Stream, +Extend, +Project, +Rdup, +Sort, -Stream5) :-
----

Apply extension, projection, duplicate removal and sorting as specified to ~Stream~ to obtain ~Stream5~.
If option ~rewriteCSE~ is active, also remove auxiliary attributes extended to the results.

*/


finish2(Stream, Extend, Project, Rdup, Sort, Stream6) :-
  fExtend(Stream, Extend, Stream2),
  fRemoveExtendedVirtualAttributes(Stream2,Stream3),
  fProject(Stream3, Project, Stream4),
  fRdup(Stream4, Rdup, Stream5),
  fSort(Stream5, Sort, Stream6).



fExtend(Stream, [], Stream) :- !.

fExtend(Stream, Extend, extend(Stream, Extend)).



fProject(Stream, *, Stream) :- !.

fProject(Stream, count(*), Stream) :- !.

fProject(Stream, Project, project(Stream, AttrNames)) :-
  attrnames(Project, AttrNames).



fRemoveExtendedVirtualAttributes(StreamIn,StreamOut) :-
  optimizerOption(rewriteCSE),
  rewritePlanRemoveInsertedAttributes(StreamIn, StreamOut), !.

fRemoveExtendedVirtualAttributes(Stream,Stream) :- !.



fRdup(Stream, duplicates, Stream) :- !.

fRdup(Stream, distinct, rdup(sort(Stream))).



fSort(Stream, [], Stream) :- !.

fSort(Stream, SortAttrs, StreamOut) :-
  rewriteForDistanceSort(Stream, SortAttrs, Stream2, SortAttrs2, NewAttrs),
  attrnamesSort(SortAttrs2, AttrNames),
  removeAttrs(sortby(Stream2, AttrNames), NewAttrs, StreamOut).


/*
---- extendProject(+Attrs, -ExtendAttrs, -ProjectAttrs) :-
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

:- assert(helpLine(optimize,1,
    [[+,'SQLquery','The SQL-style query to optimize.']],
    'Optimize (but do not execute) a query in SQL-style syntax.')).
:- assert(helpLine(optimize,3,
    [[+,'SQLquery','The SQL-style query to optimize.'],
     [-,'SecondoQuery','The optimized executable Secondo plan.'],
     [-,'CostOut','The estimated cost for the optimized plan.']],
    'Optimize (but do not execute) a query in SQL-style syntax.')).

optimize(Query) :-
  optimize(Query, SecondoQuery, Cost),
  write('The plan is: '), nl, nl,
  write(SecondoQuery), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl.


%LargeQueries start:
optimize(Query, QueryOut, CostOut) :-  
  %largeQueries start
  optimizerOption(largeQueries(qgdm)),
  setQuery(Query),  
  %largeQueries end
  retractall(removefilter(_)),
  rewriteQuery(Query, RQuery),
  callLookup(RQuery, Query2), !,
  queryToPlan(Query2, Plan, CostOut), !,
  plan_to_atom(Plan, QueryOut).
%LargeQueries end



optimize(Query, QueryOut, CostOut) :-
  retractall(removefilter(_)),
  rewriteQuery(Query, RQuery),
  callLookup(RQuery, Query2), !,
  queryToPlan(Query2, Plan, CostOut), !,
  plan_to_atom(Plan, QueryOut).

/*
----    sqlToPlan(QueryText, Plan)
----

Transform an SQL ~QueryText~ into a ~Plan~. The query is given as a text atom.
This predicate is called by the Secondo OptimizerServer.

It will also catch exceptions and transform it into a nested list encoding a
message that can be handled by the Secondo Javagui.

If the exception has Format

---- <prolog-file>_<Predicate>(<Arguments>)::<error-code>.
----

or

---- <prolog-file>_<Predicate>(<Arguments>)::<error-code>::<error-message>.
----

a error message is created and unified with ~Plan~. The Error message contains
the ErrorCode and/or the problem description.
All other exceptions are caught and `Exception during optimization'' is
returned as the message content.

*/

sqlToPlan2(QueryText, Plan) :-
  string(QueryText),
  string_to_atom(QueryText,AtomText),
  term_to_atom(Query, AtomText),
  optimize(Query, Plan, _).

% special treatment for create and drop queries
sqlToPlan2(QueryText, 'done') :-
  term_to_atom(sql Query, QueryText),
  ( Query =.. [create| _]; Query =.. [drop| _]), !,
  sql Query.

sqlToPlan2(QueryText, Plan) :-
  term_to_atom(sql Query, QueryText),
  optimize(Query, Plan, _).

/*
----    sqlToPlan2(QueryText, Plan)
----

Transform an SQL ~QueryText~ into a ~Plan~. The query is given as a text atom.
~QueryText~ starts not with sql in this version.

*/

% special treatment for create and drop queries
sqlToPlan2(QueryText, 'done') :-
  term_to_atom(Query, QueryText),
  ( Query =.. [create| _]; Query =.. [drop| _]), !,
  sql Query.

sqlToPlan2(QueryText, Plan) :-
  term_to_atom(Query, QueryText),
  optimize(Query, Plan, _).

sqlToPlan(QueryText, ResultText) :-
  asserta(errorHandlingRethrow), % force rethrowing of exceptions to handle them
                                 % here finally
  catch( sqlToPlan2(QueryText, ResultText),
         Exc, % catch all exceptions!
         ( write('\nsqlToPlan: Exception \''),write(Exc),write('\' caught.'),nl,
           ( ( Exc = error_SQL(ErrorTerm),
               ( ErrorTerm=(_::ErrorCode::Message) ; ErrorTerm=(_::Message) )
             ) %% Problems with the SQL query itself:
             -> concat_atom(['SQL ERROR (usually a user error):\n',Message],'',
                             MessageToSend)
             ;  ( ( Exc = error_Internal(ErrorTerm),
                    (   ErrorTerm = (_::ErrorCode::Message)
                      ; ErrorTerm = (_::Message)
                    )
                  )
                  -> concat_atom(['Internal ERROR (usually a problem with the ',
                                  'knowledge base):\n',Message],'',
                                  MessageToSend)
                  %% all other exceptions:
                  ;  concat_atom(['Unclassified ERROR (usually a bug):\n',Exc],
                                  '',MessageToSend)
                )
           ),
           term_to_atom(MessageToSend,ResultTMP),
           atom_concat('::ERROR::',ResultTMP,ResultText)
         )
       ),
  ( var(ResultText)
    -> ResultText = '::ERROR::Something\'s wrong!' %'
    ;  true
  ),
  retract(errorHandlingRethrow), % re-establish the original state for the
                                 % general exception handler
  true.




/*
----    sqlToPlanStr(QueryText, Plan)
----

The same as the predicate before, but using a string-atom instead a simple atom.
Thus, we can avoid problems when coding single quotes in queries, e.g. within
the ~aggregate~ command.

*/



/*
11.3.8 Examples

We can now formulate the previous example queries in the user level language.
They are stored as prolog facts sqlExample/2. Examples can be called by
testing the predicates example/1 or example/2. Moreover, they are also
present as facts with name ~example$<$No$>$~.

Below we present some generic rules for evaluating SQL examples.

*/

showExample(Nr, Query) :-
  sqlExample(Nr, Query),
  nl, write('SQL: '), write(Query), nl, nl.

example(Nr) :- showExample(Nr, Query), optimize(Query).
example(Nr, Query, Cost) :- showExample(Nr, Example),
                            optimize(Example, Query, Cost).

% The following predicate will minic the behaviour of the OptimizerServer.
% It will catch exceptions and create error messages to be sent to the JavaGUI:
exampleToPlan(Nr) :-
  sqlExample(Nr, QueryTerm),
  write_list(['\n\nExampleNr: ',Nr,'\nSQL: ',QueryTerm,'\n']),
  term_to_atom(sql QueryTerm,QueryAtom),
  sqlToPlan(QueryAtom,Plan),
  write_list(['Plan: ', Plan, '\n']).

/*
Examples 14 - 22:

*/

% Example: test standard-optimization
sqlExample( 14,

  select *
  from [staedte as s, plz as p]
  where [p:ort = s:sname, p:plz > 40000, (p:plz mod 5) = 0]
  ).

sqlExample( 15,

  select * from staedte where bev > 500000
  ).


sqlExample( 16,

  select *
  from [staedte as s, plz as p]
  where [s:sname = p:ort, p:plz > 40000]
  ).

/*
Example 17. This may need a larger local stack size. Start Prolog as

----    pl -L4M
----

which initializes the local stack to 4 MB.

Example 17 is too complex for the interesting Orders extension (even with 64M stacks):

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


% Example: test reserved pseudo-attribute 'rowid', which is translated to
%          'tupleid(.)'
sqlExample( 51,
  select [no, rowid as ii]
  from ten
  ).

sqlExample( 52,
  select [no, tid2int(rowid) as id]
  from ten
  where (tid2int(rowid) < 6) and (no = no)
  ).

% Example: test null-ary function with implicit parameter, as 'today,
%          randmax, now'
sqlExample( 53,
  select [no, now as time, today as date]
  from ten
  where (no > 5)
  ).

% Example: user defined aggregation with grouping
sqlExample( 100,
  select aggregate((distinct b:no*1), (*), 'int', [const,int,value,0] ) as fac
  from [ten as a, ten as b]
  where [a:no < b:no]
  groupby a:no
  ).

% Example: predefined aggregations with groupby, omitting the where-clause, but
%          with ordering and output restriczion:
sqlExample( 101,
  select [ort,
          min(plz) as minplz,
          max(plz) as maxplz,
          count(distinct *) as cntplz]
  from plz
  where plz > 40000
  groupby ort
  orderby cntplz desc
  first 2
  ).

% Example: Aggregations with empty groupby.
sqlExample( 102,
  select [min(plz) as minplz,
          max(plz) as maxplz,
          avg(plz) as avgplz,
          count(distinct ort) as ortcnt]
  from plz
  where plz > 40000
  groupby []
  ).

% Example: Simple aggregation over attribute without groupby
sqlExample( 103,
  select sum(no)
  from ten
  ).

% Example: Simple distinct aggregation over attribute without groupby
sqlExample( 104,
  select avg(distinct no)
  from ten
  where no > 5
  ).

% Example: Simple distinct aggregation over expression without groupby
sqlExample( 105,
  select aggregate(distinct no+1.1, (*), 'real', [const,real,value,0.0] )
  from ten
  where no > 5
  ).

% Example: Mixed Aggregations with empty groupby.
sqlExample( 106,
  select [min(plz) as minplz,
          max(plz) as maxplz,
          avg(plz) as avgplz,
          count(distinct ort) as diffOrtCnt,
          count(all ort) as allOrtCnt,
          aggregate(plz*2.0,(+),'real',[const,real,value,0.0]) as mydoublesum]
  from plz
  where [plz >= 40000, plz <50000]
  groupby []
  ).

% Example: spatial predicate intersects (database berlintest)
sqlExample( 200,
  select [s:name as sname, w:name as wname]
  from [strassen as s, wstrassen as w]
  where [s:geodata intersects w:geodata]
  ).

% Example: spatio temporal predicate (database berlintest)
sqlExample( 301,
  select [s:name as sname]
  from [strassen as s]
  where [train1 passes s:geodata]
  ).

% Example: temporal predicate (database berlintest)
sqlExample( 302,
  select [t:id as id]
  from [trains as t]
  where [t:trip present six30]
  ).

% Example: spatio temporal predicate (database berlintest)
sqlExample( 303,
  select [s:name as sname]
  from [strassen as s]
  where [(train1 atperiods deftime(train5)) passes s:geodata]
  ).

% Example: spatio temporal predicate (database berlintest)
sqlExample( 304,
  select [s:name as sname]
  from [trains as t, strassen as s]
  where [(t:trip atperiods deftime(train5)) passes s:geodata]
  ).

% Example: distance query, no predicate (database berlintest)
sqlExample( 400,
  select *
  from kinos
  orderby distance(geodata, mehringdamm) first 5
  ).

% Example: distance query, selection predicate (database berlintest)
sqlExample( 401,
  select *
  from ubahnhof
  where typ = "Zone A"
  orderby distance(geodata, alexanderplatz) first 5
  ).

% Example: distance query, rtree, no predicate (database berlintest)
sqlExample( 402,
  select *
  from strassen
  orderby distance(geodata, mehringdamm) first 5
  ).

% Example: distance query, rtree, selection predicate (database berlintest)
sqlExample( 403,
  select *
  from strassen
  where typ starts "Hauptstra"
  orderby distance(geodata, alexanderplatz) first 5
  ).

% Example: distance query, rtree, join predicate (database berlintest)
sqlExample( 404,
  select *
  from [strassen as s, sbahnhof as b]
  where s:name = b:name
  orderby distance(s:geodata, mehringdamm) first 5
  ).

% Section:Start:sqlExample_1_e
% Section:End:sqlExample_1_e

% Example: spatio temporal pattern query (database berlintest)
sqlExample( 500,
  select count(*)
  from trains
  where pattern([trip inside msnow as preda,
                 distance(trip, mehringdamm)<10.0 as predb],
                [stconstraint("preda","predb",vec("aabb"))])).

sqlExample( 501,
  select count(*)
  from trains
  where pattern([trip inside sehenswuerdaspoints as preda,
                 distance(trip, mehringdamm)<10.0 as predb,
                 trip = mehringdamm as predc
                ],
                [stconstraint("preda","predb",vec("aabb")),
                 stconstraint("predb","predc",vec("aabb"))
                ])).


example14 :- example(14).
example15 :- example(15).
example16 :- example(16).
example17 :- example(17).
example18 :- example(18).
example19 :- example(19).
example20 :- example(20).
example21 :- example(21).
example100 :- example(100).
example101 :- example(101).
example102 :- example(102).
example103 :- example(103).
example104 :- example(104).
example105 :- example(105).
example106 :- example(106).



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

/*

Exception Handling

If an error is encountered during the optimization process, an exception should be
thrown using the built-in Prolog predicate

----
        throw(error_SQL(X)),      for errors in the SQL-query
        throw(error_Internal(X)), for errors due to optimizer failure
----

~error\_SQL(X)~ is an exception class to report errors related to the query
itself (e.g. the user's SQL command is malformed, the user does not have
sufficient rights to perform the action, etc.).

~error\_Internal(X))~ is an exception class to report errors that do not directly
concern the SQL query, but have been occured dur to internal optimizer problems
(like missing meta data).

In all cases, ~X~ is a term that represents the error-message respecting format

----    <prolog-file>_<Predicate>(<Arguments>)::<error-code>
----

or

----    <prolog-file>_<Predicate>(<Arguments>)::<error-code>::<error-message>.
----

~error-message~ should be a informative, free-form text message that can be
presented to the user and helping him with understanding and possibly fixing
the problem.

~error-code~ should be a tag to be used within internal exception handling.

Error codes used by now are:

  * missingData - some expected meta data cannot be retrieved

  * selectivityQueryFailed - a selectivity query failed

  * malformedExpression - an expression does not obey the demanded syntax

  * unexpectedListType - a nested list xpression does not obey the demanded form

  * prohibitedAction - a action has been demanded, that my not be performed

  * missingParameter - a required parameter has not been bound

  * schemaError - an identifier or object violates the naming conventions

  * noDatabase - no database is currently opened

  * requestUserInteraction - user interaction is required to recover from this error

  * unknownIdentifier - the specified identifier is not recognized

  * unknownAttribute - the specified attribute is unknown

  * unknownRelation - the specified relation is unknown

  * unknownIndex - the specified index is not found

  * objectAlreadyExists - an according object already exists or the name is already in use

  * wrongType - a passed argument has a wrong type

  * unspecifiedError - the nature of an error is unknown or irrelevant


A standard exception handler is implemented by
the predicate ~defaultExceptionHandler(G)~ that will catch any exception respecting the
exception-format described above, that is thrown within goal ~G~.

*/

:- assert(helpLine(sql,1,
    [[+,'SQLquery','The SQL-style query to run optimized.']],
    'Optimize and run an SQL-style query .')).
:- assert(helpLine(sql,2,
    [[+,'SQLquery','The SQL-style query to run optimized.'],
     [+,'SECrest','The executable Secondo-style end of the query.']],
    'Form and run a query from the first (optimized) and second argument .')).
:- assert(helpLine(let,2,
    [[+,'ObjName','The name for the object to create.'],
     [+,'SQLquery','The SQL-style query to run optimized.']],
    'Create a DB object using an SQL-style value.')).
:- assert(helpLine(let,3,
    [[+,'ObjName','The name for the object to create.'],
     [+,'SQLquery','The SQL-style query to run optimized.'],
     [+,'SECrest','The executable Secondo-style end of the query.']],
    'Create a DB object, using a combined SQL and Secondo executable query.')).


:- dynamic(errorHandlingRethrow/0),   % standard behaviour is to handle
   retractall(errorHandlingRethrow).  % errors quitely.

defaultExceptionHandler(G) :-
  catch( G,
         Exception,
         (  % write('\ndefaultExceptionHandler/1: Exception \''),
            % write(Exception),write('\' caught.'),nl,
            ( (Exception = error_SQL(X) ; Exception = error_Internal(X))
            % only handle these kinds of exceptions
            -> ( write_list(['\n\nThe following SQL Error was caught: \'', X,
                     '\'.\n','This is usually a problem within the query.\n\n'])
                )
            ;  ( Exception = error_Internal(X)
                  -> ( write_list(['\n\nThe following Internal Error was ',
                          'caught: \'', X, '\'.\n','This is usually due to a ',
                          'problem with optimizer\'s knowledge base, the ',%'
                          'meta data within the database, or a problem within ',
                          'the internal optimization routines.\n\n'])
                    )
                  ;  ( Exception = error(_Formal, _Context)
                      -> ( write_list(['\n\nThe following Runtime Error was ',
                            'caught: \'', X, '\'.\n','This is usually a ',
                            'problem with the optimizer knowledge base or a ',
                            'program bug within the optimizer.\n\n'])
                          )
                      ;  ( write_list(['\n\nThe following Unclassified Error ',
                            'was caught: \'', X, '\'.\n','The reason is ',
                            'unknown. Please carefully check the error message',
                            ' to trace the problem.\n\n'])
                          )
                    )
                )
          ),
          ( errorHandlingRethrow  % retract errorHandlingRethrow to quit quitely
            -> throw(Exception)   % assert errorHandlingRethrow to re-throw
            ;  ( print_message(error,Exception), % all exceptions!
                  fail             % With not(errorHandlingRethrow), the error
               )                   % message is printed and the predicate fails.
          )
         )
       ).


:-assert(helpLine(history,0,[],
  'List the query history (since last \'storeHistory\').')).
history :-
  showRel('SqlHistory').

:-assert(helpLine(deleteHistory,0,[],
  'Delete the query history relation \'SqlHistory\' from the current DB.')).
deleteHistory :-
  clearRel('SqlHistory').

:-assert(helpLine(storeHistory,0,[],
'Store the query history into relation \'SqlHistory\' within the current DB.')).
storeHistory :-
  saveRel('SqlHistory').


sql create X by Term :- let(X, Term).

% special handling for create query
sql create Term :- !, sqlNoQuery(create Term).

% special handling for drop query
sql drop Term :- !, sqlNoQuery(drop Term).

% Default handling
sql Term :- defaultExceptionHandler((
  isDatabaseOpen,
  getTime( mOptimize(Term, Query, Cost), PlanBuild ),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  query(Query, PlanExec),
  appendToRel('SqlHistory', Term, Query, Cost, PlanBuild, PlanExec)
 )).



sql(Term, SecondoQueryRest) :- defaultExceptionHandler((
  isDatabaseOpen,
  mStreamOptimize(Term, SecondoQuery, Cost),
  concat_atom([SecondoQuery, ' ', SecondoQueryRest], '', Query),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  query(Query, _)
 )).

let(X, Term) :- defaultExceptionHandler((
  isDatabaseOpen,
  mOptimize(Term, Query, Cost),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  concat_atom(['let ', X, ' = ', Query], '', Command),
  secondo(Command)
 )).

let(X, Term, SecondoQueryRest) :- defaultExceptionHandler((
  isDatabaseOpen,
  mStreamOptimize(Term, SecondoQuery, Cost),
  concat_atom([SecondoQuery, ' ', SecondoQueryRest], '', Query),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  concat_atom(['let ', X, ' = ', Query], '', Command),
  secondo(Command)
 )).

/*
----    sqlNoQuery(+Term)
----

special handling for create and drop-queries, which do not use the
query command in SECONDO

*/

sqlNoQuery(Term) :-  defaultExceptionHandler((
  isDatabaseOpen,
  mOptimize(Term, Query, Cost),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  multiquery(Query)
 )).

/*
----    multiquery(+Querylist)
----

Executes all queries in ~Querylist~.
This is used for the drop command, which can result in multiple queries
in SECONDO to delete the relation and all indices.

*/

multiquery([]) :- !.

multiquery([Query|Rest]) :-
  multiquery(Query),
  multiquery(Rest), !.

multiquery(Query) :-
  write('Execute: '), write(Query), nl,
  secondo(Query), !.

/*
----    streamOptimize(Term, Query, Cost) :-
----

Optimize the ~Term~ producing an incomplete Secondo query plan ~Query~
returning a stream.

*/
streamOptimize(Term, Query, Cost) :-
  callLookup(Term, Term2),
  queryToStream(Term2, Plan, Cost),
  plan_to_atom(Plan,  Query).

/*
----    mOptimize(Term, Query, Cost) :-
        mStreamOptimize(union [Term], Query, Cost) :-
----

Means ``multi-optimize''. Optimize a ~Term~ possibly consisting of several subexpressions to be independently optimized, as in union and intersection queries. ~mStreamOptimize~ is a variant returning a stream.

*/

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
  %concat_atom([QueryPart, 'sort rdup '], '', Query).
  % NVK MODIFIED NR: missing blank before sort.
  concat_atom([QueryPart, ' sort rdup '], '', Query).

mStreamOptimize(union [Term | Terms], Query, Cost) :-
  streamOptimize(Term, Plan1, Cost1),
  mStreamOptimize(union Terms, Plan2, Cost2),
  %concat_atom([Plan1, 'sort rdup ', Plan2, 'mergeunion '], '', Query),
  % NVK MODIFIED NR: missing blank before sort.
  concat_atom([Plan1, ' sort rdup ', Plan2, 'mergeunion '], '', Query),
  Cost is Cost1 + Cost2.

mStreamOptimize(intersection [Term], Query, Cost) :-
  streamOptimize(Term, QueryPart, Cost),
  %concat_atom([QueryPart, 'sort rdup '], '', Query).
  % NVK MODIFIED NR: missing blank before sort.
  concat_atom([QueryPart, ' sort rdup '], '', Query).

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
  query(Q, _).

bestPlanConsume :-
  bestPlan(P, _),
  plan_to_atom(P, S),
  atom_concat(S, ' consume', Q),
  nl, write(Q), nl,
  query(Q, _).


% Section:Start:auxiliaryPredicates
% Section:End:auxiliaryPredicates

/*
Used within the spatiotemporal pattern predicates.
Looks up the aliased lifted predicate list within the
spatiotemporal pattern predicate.

*/
composeNPredList( [P | PredListRest], [A | AliasListRest],
                                                  [P as A| NPredListRest]):-
 composeNPredList(PredListRest, AliasListRest, NPredListRest).
composeNPredList( [], [], []).


lookupPatternPreds([Pred| PRest], [Pred2| PRest2], RelsBefore, RelsAfterPreds):-
 lookupPred1(Pred, Pred2, RelsBefore, RelsAfterMe),
 lookupPatternPreds(PRest, PRest2, RelsAfterMe, RelsAfterPreds).
lookupPatternPreds([], [], RelsBefore, RelsBefore).

lookupPattern( NPredList , NPredList2, RelsBefore, RelsAfter) :-
  removeAliases(NPredList, PredList, AliasList),
  lookupPatternPreds(PredList, PredList2, RelsBefore, RelsAfter),
  composeNPredList(PredList2, AliasList, NPredList2).

/*
Used within spatiotemporal pattern predicates

*/

namedPred_to_atom(NP,NP1):-
 NP=..[as,P,A],
 flatten([A],A1),
 flatten([P],P1),
 A1=[A3], P1=[P2],
 plan_to_atom(P2,P3),
 concat_atom([A3, ' : ', P3],'', NP1),!.

namedPredList_to_atom([NP], NP1):-
 namedPred_to_atom(NP,NP1),!.
namedPredList_to_atom([ NP | NPListRest], Result):-
 namedPred_to_atom(NP, NP1),
 namedPredList_to_atom( NPListRest, SubResult),
 concat_atom([NP1, ',', SubResult], ' ', Result),!.

/*
---- onlyContains(+List,+Pattern)
----

Test whether either ~List~ unifies with ~Pattern~ all elements in ~List~
unify with ~Pattern~.

*/
onlyContains(List,Pattern):- (List=Pattern ; delete(List,Pattern,[]) ),!.

%evalIskNN(K, QueryObj, RelName, AttrName, TupleIDs).
isLiftedSpatialRangePred(Op, [T1,T2]):-
  opSignature(Op, _, [T1, T2], _, Flags),
  memberchk(liftedspatialrange,Flags),!.

isLiftedSpatialRangePred(Term) :-
  optimizerOption(determinePredSig),
  compound(Term), not(is_list(Term)),
  checkOpProperty(Term,liftedspatialrange), !.


isLiftedEqualityPred(Op, [T1,T2]):-
  opSignature(Op, _, [T1, T2], _, Flags),
  memberchk(liftedequality,Flags),!.

isLiftedEqualityPred(Term) :-
  optimizerOption(determinePredSig),
  compound(Term), not(is_list(Term)),
  checkOpProperty(Term,liftedequality), !.


isLiftedRangePred(Op, [T1,T2,T3]):-
  opSignature(Op, _, [T1, T2, T3], _, Flags),
  memberchk(liftedrange,Flags),!.

isLiftedRangePred(Term) :-
  optimizerOption(determinePredSig),
  compound(Term), not(is_list(Term)),
  checkOpProperty(Term,liftedrange), !.

isLiftedLeftRangePred(Op, [T1,T2]):-
  opSignature(Op, _, [T1, T2], _, Flags),
  memberchk(liftedleftrange,Flags),!.

isLiftedLeftRangePred(Term) :-
  optimizerOption(determinePredSig),
  compound(Term), not(is_list(Term)),
  checkOpProperty(Term,liftedleftrange), !.

isLiftedRightRangePred(Op, [T1,T2]):-
  opSignature(Op, _, [T1, T2], _, Flags),
  memberchk(liftedrightrange,Flags),!.

isLiftedRightRangePred(Term) :-
  optimizerOption(determinePredSig),
  compound(Term), not(is_list(Term)),
  checkOpProperty(Term,liftedrightrange), !.



/*
14 Query Rewriting

See file ``rewriting.pl''

*/

/*
15 Plan Rewriting

See file ``rewriting.pl''

*/

:- [rewriting]. % include query and plan rewriting

/*
16 Real Node Sizes

For testing the quality of estimations it is necessary to compute the real node
sizes of the POG. Therefore we have to compute a path to each node and to
generate a counting query for the plan.

The predicate ~allCards/0~ prints out the real cardinality for each POG node.

*/

:- dynamic realResult/2.

card(N) :- dijkstra(0, N, Path, _),
  deleteCounters,
  N > 0,
  highNode(High),
  N =< High,
  plan(Path, Plan),
  plan_to_atom(Plan, Query),
  atom_concat(Query, ' count', CountQuery),
  atom_concat('query ', CountQuery, FullQuery),
  secondo(FullQuery, [_, Result]),
  write('The result for node '), write(N), write(' is: '),write(Result), nl, nl,
  assert(realResult(N, Result)).


cards(N) :-
  highNode(High),
  N > High.

cards(N) :-
  resultSize(N, _),
  card(N),
  N1 is N + 1,
  cards(N1),
  !.

cards(N) :-
  N1 is N + 1,
  cards(N1).

resultSizes(Node, SizeEst, SizeReal) :-
  resultSize(Node, SizeEst),
  realResult(Node, SizeReal).

writeRealSizes :-
  findall([Node, SizeEst, SizeReal], resultSizes(Node, SizeEst, SizeReal), L),
  Format = [ ['Node', 'l'],
             ['Size-Estimated', 'l'],
             ['Size-Real', 'l'] ],
  showTuples(L, Format).

computeCards :-
  retractall(realResult(_, _)),
  cards(1).

:-assert(helpLine(allCards,0,[],
         'Compare estimated and actual cardinalities for the current POG.')).

allCards :-
  computeCards,
  writeRealSizes.

/*
17 Loading Extensions for Nawra Cost Functions

*/

:- [nawra].


/*
18 Loading Extensions for Subquery Processing

*/

:- ['./Subqueries/subqueries'].

/*
19 Update operations

*/


/*
19.1 Update the Index after an Insert, Update or Delete-Operation

----    updateIndex(+Operation, +Rel, +Stream, -Stream2)
----

Extends ~Stream~ by the commands which are necessary to update the indices
of ~Rel~

*/

updateIndex(Operation, [rel(Rel, Var)], StreamIn, StreamOut) :-
  relation(Rel, Attrs),
  updateIndex(Operation, rel(Rel, Var), Attrs, StreamIn, StreamOut),
  !.

/*
----    updateIndex(+Operation, +Rel, +Attrs, +Stream, -Stream2)
----

Extends ~Stream~ by the commands which are necessary to update the indices
of ~Rel~ on the Attributes ~Attrs~

*/

updateIndex(_, _, [], StreamIn, StreamIn) :-
  !.

/*
NVK ADDED NR
No indices handling for attributes within nested relations.

*/
updateIndex(Operation, Rel, [Attr|Attrs], StreamIn, StreamOut) :-
	Attr=_:_,
  updateIndex(Operation, Rel, Attrs, StreamIn, StreamOut).
% NVK ADDED NR END

updateIndex(Operation, Rel, [Attr|Attrs], StreamIn, StreamOut) :-
	Attr\=_:_, % NVK ADDED NR
  lookupAttr(Attr, Attr2),
  updateAttrIndex(Operation, Rel, Attr2, StreamIn, Stream),
  updateIndex(Operation, Rel, Attrs, Stream, StreamOut).


/*
----    updateAttrIndex(+Operation, +Rel, +Attr, +Stream, -Stream2)
----

Extends ~Stream~ by the commands which are necessary to update the indices
of ~Rel~ on the Attribute ~Attr~

*/

updateAttrIndex(Operation, Rel, Attr, StreamIn, StreamOut) :-
  findall((DCindex, LogicalIndexType),
   hasIndex(Rel, Attr, DCindex, LogicalIndexType), List),
  updateAttrIndex(Operation, Rel, Attr, List, StreamIn, StreamOut),
  !.

/*
----    updateAttrIndex(+Operation, +Rel, +Attr, +Indices, +Stream, -Stream2)
----

Extends ~Stream~ by the commands which are necessary to update the indices
of ~Rel~ on the Attribute ~Attr~

*/

updateAttrIndex(_, _, _, [], StreamIn, StreamIn) :- !.

updateAttrIndex(Operation, Rel, Attr, [(DCindex, LogicalIndexType)|Rest],
  StreamIn, StreamOut) :-
  dcName2externalName(DCindex,IndexName),
  logicalIndexType(_, LogicalIndexType, PhysicalIndexType, _, _, _, _, _),
  updateTypedIndex(Operation, Attr, IndexName, PhysicalIndexType,
     StreamIn, Stream2),
  updateAttrIndex(Operation, Rel, Attr, Rest, Stream2, StreamOut),
  !.

/*
----    updateTypedIndex(+Operation, +Attr, +IndexName, +IndexType, +Stream,
                         -Stream2)
----

Extends ~Stream~ by the commands which are necessary to update the index
~IndexName~ on the Attribute ~Attr~

*/

updateTypedIndex(insert, Attr, IndexName, btree, StreamIn,
   insertbtree(StreamIn, IndexName, Attr)) :-
  !.

updateTypedIndex(insert, Attr, IndexName, IndexType, StreamIn,
   insertrtree(StreamIn, IndexName, Attr)) :-
  memberchk(IndexType,[rtree,rtree3,rtree4,rtree8]),!.

updateTypedIndex(insert, Attr, IndexName, hash, StreamIn,
   inserthash(StreamIn, IndexName, Attr)) :-
  !.


updateTypedIndex(delete, Attr, IndexName, btree, StreamIn,
   deletebtree(StreamIn, IndexName, Attr)) :-
  !.

updateTypedIndex(delete, Attr, IndexName, IndexType, StreamIn,
   deletertree(StreamIn, IndexName, Attr)) :-
  memberchk(IndexType,[rtree,rtree3,rtree4,rtree8]),!.

updateTypedIndex(delete, Attr, IndexName, hash, StreamIn,
   deletehash(StreamIn, IndexName, Attr)) :-
  !.


updateTypedIndex(update, Attr, IndexName, btree, StreamIn,
   updatebtree(StreamIn, IndexName, Attr)) :-
  !.

updateTypedIndex(update, Attr, IndexName, IndexType, StreamIn,
   updatertree(StreamIn, IndexName, Attr)) :-
  memberchk(IndexType,[rtree,rtree3,rtree4,rtree8]),!.

updateTypedIndex(update, Attr, IndexName, hash, StreamIn,
   updatehash(StreamIn, IndexName, Attr)) :-
  !.

updateTypedIndex(drop, _, IndexName, _, StreamIn,
   deleteIndex(IndexName, StreamIn)) :-
  !.


/*
19.2 Split the select and update clause

----    splitSelect(+Clause, -SelectClause, -UpdateClause)
----

Splits ~Clause~ into ~SelectClause~ and ~UpdateClause~

*/

splitSelect(select SelectClause, select SelectClause, []) :-
  !.

splitSelect(UpdateClause select SelectClause, select SelectClause,
     UpdateClause) :-
  !.


/*
19.3 Insert the update commands in the end of the operation

----    finishUpdate(+UpdateClause, +Stream, -Stream2)
----

Extends ~Stream~ by the commands for an insert, delete or update-
operation.

*/

finishUpdate([], Stream2, Stream2) :-
  !.

finishUpdate(insert into Rel, Stream, Stream3) :-
  updateAttrNamesForInsert(Stream, Rel, Stream2),
  updateIndex(insert, Rel, insert(Rel, Stream2), Stream3),
  !.

finishUpdate(delete from Rel, Stream2, Stream3) :-
  updateIndex(delete, Rel, deletedirect(Rel, Stream2), Stream3),
  !.

finishUpdate(update Rel set Transformations, Stream2, Stream3) :-
  updateIndex(update, Rel, updatedirect(Rel, Transformations, Stream2),
       Stream3),
  !.

finishUpdate(drop table Rel, Stream2, Stream3) :-
  updateIndex(drop, Rel, Stream2, Stream3),
  !.


/*
19.4 Rename the attributes of the select operation for the insert
operation

----    updateAttrNamesForInsert(+Stream, +RelList, -Stream2)
----

Renames the attributes of the select operation to make them fit to
the structure of the relation in ~RelList~

*/

updateAttrNamesForInsert(Stream, [rel(Rel, Var)], Stream2) :-
  collectQueryAttrs(rel(Rel, Var), AttrsSrc),
  relation(Rel, AttrsDest),
  lookupAttrs(AttrsDest, AttrsDest2),
  calcExtendList(AttrsSrc, AttrsDest2, AttrsExtend),
  extendStream(Stream, AttrsExtend, AttrsDest2, Stream2).


/*
----    collectQueryAttrs(+InsertRel, -Attrs)
----

Selects the attributes of the query-relation execpt for ~InsertRel~ as
this relation is used for the insert-command

*/

collectQueryAttrs(InsertRel, Attrs) :-
  findall(Rel, queryRel(_, Rel), Rels),
  delete(Rels, InsertRel, Rels2),
  collectQueryAttrs2(Rels2, Attrs).

/*
----    collectQueryAttrs2(+RelList, -Attrs)
----

Selects the attributes from ~RelList~ which are used in the query

*/


collectQueryAttrs2([], []).

collectQueryAttrs2([rel(Rel, _)|Rels], AttrsOut) :-
  isStarQuery, !,
  relation(Rel, Attrs),
  lookupAttrs(Attrs, Attrs2),
  collectQueryAttrs2(Rels, Attrs3),
  concatNonEmpty(Attrs2, Attrs3, AttrsOut).

collectQueryAttrs2([Rel|Rels], AttrsOut) :-
  setof(X, usedAttr(Rel, X),Attrs),
  collectQueryAttrs2(Rels, Attrs2),
  concatNonEmpty(Attrs, Attrs2, AttrsOut).

/*
----    calcExtendList(+Attrs1, +Attrs2, -AttrsOut)
----

Compares if the elements of ~Attrs1~ and ~Attrs2~ are equal and returns
a list with the differences for the extend command

*/

calcExtendList([], _, []).

calcExtendList(_, [], []).

calcExtendList([Attr|Rest1], [Attr|Rest2], AttrsOut) :-
   calcExtendList(Rest1, Rest2, AttrsOut).

calcExtendList([Attr1|Rest1], [Attr2|Rest2], AttrsOut) :-
   calcExtendList(Rest1, Rest2, AttrsRest),
   concatNonEmpty(newattr(attrname(Attr2), Attr1), AttrsRest, AttrsOut).


/*
---- extendStream(+Stream, +ExtendList, +ProjectList, -StreamOut)
----

Extends ~Stream~ by the extend and project command using ~ExtendList~
and ~ProjectList~

*/

extendStream(Stream, [], _, Stream).

extendStream(Stream, ExtendList, ProjectList,
                          project(extend(Stream, ExtendList), ProjectList2)) :-
  attrnames(ProjectList, ProjectList2).



/*
20 Auxiliary functions for create index

*/

/*
----    convertToLfName(+Name, -LFName)
----

Converts ~Name~ to the form which is needed by keyAttributeTypeMatchesIndexType

*/

convertToLfName([], []) :- !.

convertToLfName(attr(DcName, _, _), LfName) :-
  downcase_atom(DcName, LfName), !.

convertToLfName(rel(DcName, _), LfName) :-
  downcase_atom(DcName, LfName), !.

convertToLfName([FullName, Rest], [LfName, Rest2]) :-
  convertToLfName(FullName, LfName),
  convertToLfName(Rest, Rest2), !.


/*
21 Distance queries

*/

/*
----    upperBound(+Rel, +Node, -UpperBound)
----

Calculates the maximum number of elements, the arp of ~Node~ which contains
~Rel~ can have

*/

upperBound(Rel, node(_, _, Arps), UpperBound) :-
  select(X, Arps, _),
  X = arp(_, RelsX, _),
  member(Rel, RelsX),
  upperBound(RelsX, UpperBound).

/*
----    upperBound(+Rels, -UpperBound)
----

Calculates the maximum number of elements, a join of Rels can have

*/

upperBound([], 1) :- !.

upperBound([rel(Rel, _)|Rels], UpperBound) :-
  upperBound(Rels, UpperBound1),
  card(Rel, Card),
  UpperBound is UpperBound1 * Card.


/*
----    addPlanVariables(+Plan, +Variables, -Plan2)
----

Adds ~Variables~ to the beginning of ~Plan~
This is used for distance-queries using temporary indices

*/

addPlanVariables(Result, [], Result) :- !.

addPlanVariables(ResultIn, [[Var, Expr]|Rest],
   varfuncquery(ResultTmp, Var, Expr)) :-
  addPlanVariables(ResultIn, Rest, ResultTmp).


/*
----    assertDistanceRel(+Rels, +DistAttr1, +DistAttr2, +HeadCount)
----

Checks whether an rtree-index on ~DistAttr1~ or ~DistAttr2~ exists in ~Rels~

*/

assertDistanceRel([Rel|_], X, Y, HeadCount) :-
  Rel = rel(_,_),
  X = attr(_, _, _),
  Y = dbobject(_),
  hasIndex(Rel, X ,DCindex, rtree),
  dcName2externalName(DCindex,IndexName), !,
  assert(distanceRel(Rel, IndexName, Y, HeadCount)).

assertDistanceRel([Rel|_], X, Y, HeadCount) :-
  Rel = rel(_,_),
  Y = attr(_, _, _),
  X = dbobject(_),
  hasIndex(Rel, Y ,DCindex, rtree),
  dcName2externalName(DCindex,IndexName), !,
  assert(distanceRel(Rel, IndexName, X, HeadCount)).

assertDistanceRel([_|Rels], X, Y, HeadCount) :-
  assertDistanceRel(Rels, X, Y, HeadCount).


/*
----    finishDistanceSort(+Stream, +Cost, +DistAttr1, +DistAttr2,
                           +HeadCount, -Stream2, -Cost2)
----

Adds the commands to sort ~Stream~ by the distance between ~DistAttr1~ and
~DistAttr2~ and calculates the additional costs

*/

finishDistanceSort(Stream, Cost, X, Y, 0, StreamOut, Cost2) :-
  !, newVariable(ExprAttrName),
  ExprAttr = attr(ExprAttrName, *, l),
  StreamOut = remove(sortby(extend(Stream, [field(ExprAttr, distance(X, Y))]),
       attrname(ExprAttr)),
       attrname(ExprAttr)),
  ( (optimizerOption(nawracost) ; optimizerOption(improvedcosts) )
    -> (  highNode(Source),
          Target is Source + 1,
          Result = res(Target),
          Pred = pr(fakePred(1,1,0,0)),
          costterm(remove(sort(extend(pogstream, none)), none), Source, Target,
                   Result, 1, Pred, _Card, CostTmp)
       )
    ;  cost(remove(sort(extend(pogstream, _)), _), 1, _, _, CostTmp)
  ),
  Cost2 is Cost + CostTmp.

finishDistanceSort(Stream, Cost, X, Y, HeadCount, StreamOut, Cost2) :-
  !, newVariable(ExprAttrName),
  ExprAttr = attr(ExprAttrName, *, l),
  StreamOut = remove(ksmallest(extend(Stream,
          [field(ExprAttr, distance(X, Y))]),
          HeadCount, attrname(ExprAttr)),
       attrname(ExprAttr)),
  ( (optimizerOption(nawracost) ; optimizerOption(improvedcosts) )
    -> (  highNode(Source),
          Target is Source + 1,
          Result = res(Target),
          Pred = pr(fakePred(1,1,0,0)),
          costterm(remove(sort(extend(pogstream, none)), none), Source, Target,
                   Result, 1, Pred, _Card, CostTmp)
       )
    ; cost(remove(ksmallest(extend(pogstream, _), HeadCount), _), 1, _, _,
       CostTmp)
  ),
  Cost2 is Cost + CostTmp.

/*
----    finishDistanceSortRTree(+Stream, +Cost, +HeadCount, -Stream2, -Cost2)
----

Adds the commands for a distancescanquery which are not set by the POG and
corrects the cost estimation according to ~HeadCount~ and the costs for
temporary indices

*/

finishDistanceSortRTree(StreamIn, CostIn, 0,
   StreamOut, CostOut) :-
  findTmpIndex(StreamIn, StreamOut, CostTmpIndex),
  CostOut is CostIn + CostTmpIndex.

finishDistanceSortRTree(StreamIn, CostIn, HeadCount,
   StreamOut, CostOut) :-
  findTmpIndex(head(StreamIn, HeadCount), StreamOut, CostTmpIndex),
  highNode(Node),
  resultSize(Node, Size),
  CostOut is CostIn * min(HeadCount, Size) / Size + CostTmpIndex.

/*
----    findTmpIndex(+Stream, -Stream2, - Cost)
----

Looks for temporary indexes in ~Stream~, replaces them by a variable and
stores them in planvariable

*/

findTmpIndex([], [], 0).

findTmpIndex(dbobject(tmpindex(Rel,Attr)), a(IndexName, *, l), Cost) :-
  newVariable(IndexName),
  Expr = createtmpbtree(Rel, Attr),
  assert(planvariable(IndexName, Expr)),
  cost(Expr, _, _, _, Cost).

findTmpIndex([E|R], [E2|R2], Cost) :-
  findTmpIndex(E, E2, Cost1),
  findTmpIndex(R, R2, Cost2),
  Cost is Cost1 + Cost2.

findTmpIndex(P, P2, Cost) :-
  P =.. [Op| Params],
  findTmpIndex(Params, Params2, Cost),
  P2 =.. [Op| Params2].

/*
----    addTmpVariables(+Stream, -Stream2)
----

Adds the planvariables for the temporary indices to the beginning of
the stream

*/

addTmpVariables(Stream, StreamOut) :-
  findall([Variable, Expression], planvariable(Variable, Expression), L),
  addPlanVariables(Stream, L, StreamOut),
  retractall(planvariable(_, _)).


/*
----    chooseFasterSolution(+Stream1, +Select1, +Update1, +Cost1,
                             +Stream2, +Select2, +Update2, +Cost2,
                             -Stream3, -Select3, -Update3, -Cost3)
----

Compares the costs of the two possible solutions and returns the faster
one

*/

chooseFasterSolution(StreamOut1, SelectOut1, Update1, Cost1, _, _, _, Cost2,
       StreamOut1, SelectOut1,Update1, Cost1) :-
  Cost1 < Cost2, !.

chooseFasterSolution(_, _, _, _, StreamOut2, SelectOut2, Update2, Cost2,
       StreamOut2, SelectOut2, Update2, Cost2).

/*

---- rewriteForDistanceSort(+StreamIn, +AttrsIn, -StreamOut, -AttrsOut,
     -NewAttrs) :-
----

Transform the distance operator in the orderby-clause. ~NewAttrs~
returns the temporary attributes, which were added to contain the
distance. These attributes have to be removed later.

*/

rewriteForDistanceSort(Stream, [], Stream, [], []).

rewriteForDistanceSort(Stream, distance(X, Y), StreamOut, ExprAttr,
         attrname(ExprAttr)) :-
  !, newVariable(ExprAttrName),
  ExprAttr = attr(ExprAttrName, *, l),
  StreamOut = extend(Stream, [field(ExprAttr, distance(X, Y))]).

rewriteForDistanceSort(Stream, [Attr | Attrs], StreamOut, [Attr2 | Attrs2],
        NewAttrsOut) :-
  rewriteForDistanceSort(Stream, Attr, Stream2, Attr2, NewAttr),
  rewriteForDistanceSort(Stream2, Attrs, StreamOut, Attrs2, NewAttrs),
  concatNonEmpty(NewAttr, NewAttrs, NewAttrsOut).

rewriteForDistanceSort(Stream, Attr asc, StreamOut, Attr2 asc, NewAttrs) :-
  rewriteForDistanceSort(Stream, Attr, StreamOut, Attr2, NewAttrs).

rewriteForDistanceSort(Stream, Attr desc, StreamOut, Attr2 desc, NewAttrs) :-
  rewriteForDistanceSort(Stream, Attr, StreamOut, Attr2, NewAttrs).

rewriteForDistanceSort(Stream, Attr, Stream, Attr, []).

/*

---- concatNonEmpty(+Elem, +List, -ListOut) :-
----

Adds ~Elem~ to the front of ~List~ if ~Elem~ is not empty

*/


concatNonEmpty([], List, List).

concatNonEmpty([Attr|Rest], List, ListOut) :-
  concatNonEmpty(Rest, List, List2),
  concatNonEmpty(Attr, List2, ListOut).

concatNonEmpty(Attr, List, [Attr | List]).


/*

---- removeAttrs(+StreamIn, +NewAttrs, -StreamOut).
----

Removes the Attributes ~NewAttrs~ from the query

*/


removeAttrs(StreamIn, [], StreamIn).

removeAttrs(StreamIn, NewAttrs, remove(StreamIn, NewAttrs)).


/*
Load Faked operators extension

*/
:- [fakedoperators].

/*
End of file ~optimizer.pl~

*/

