/*

----
This file is part of SECONDO.

Copyright (C) 2012, University Hagen, Faculty of Mathematics and
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
//[newpage] [\newpage]
//[_] [\_]
 



[10] Predicates for Large Query Optimization Algorithm ~Query Graph Decomposition and Materialization (QGDM)~


By Gero Willmes, January 2012

Implementations for my master thesis

  


[newpage]

[toc]

[newpage]

*/


/*

1 Predicates for Large Query Optimization Algorithm ~Query Graph Decomposition and Materialization (QGDM)~

*/

/*

1.1 qgdm

*/

/*
----    qgdm(+Query, -Stream2, -Select2, -Update, -Cost)
----

Description:

Main predicate which does the translation of a large predicate set query 
using query graph decomposition and  materializing (qgdm).
The signature is identical to translate1/5.

By Gero Willmes

Input:

~Query~: Query of Type 'select Args from Rels where Preds'

Output:

~Stream2~: Plan

~Select2~: Arguments of the select clause

~Update~: Update clause

~Cost~: Cost of the Plan

*/

qgdm(Select from Rels where Preds, Stream2, Select2, Update, Cost) :-
  not( optimizerOption(immediatePlan) ),
  not( optimizerOption(intOrders(_))  ), 
  set_prolog_flag(unknown, warning),
  nl,nl,
  write('Large predicate set optimization using query graph decomposition '), 
  write('and  materializing (qgdm)'),nl,  
  
  %create query graph:
  qgcreateNodeList(Rels,QGNodeList),
  retractall(qgEdgeGlobal(_,_,_,_,_,_, _)),
  createQGEdges(Preds,Rels,QGEdgeList),!,

  initResultCost,
  setRels(Rels),
  setSelect(Select),  
  setMaxEdgesPerComponent(7), 
  
  %create spanning tree:
  nl,write('Start Kruskal...'),
  kruskal(QGEdgeList,QGNodeList,_, _ /*RemovedEdgeList*/, Stream),!,
  nl,write('Kruskal finished'),
  writeComponents,
  resultCost(Cost),
  
  splitSelect(Select, Select02, Update),
  % Hook for CSE substitution
  rewritePlanforCSE(Stream, Stream2, Select02, Select2), 
  !.

/*

1.2 completesCycle

*/
 
/*
----    completesCycle(+A,+B,+SpanTree)
----

Description:

Succeeds if the edge (A, B) completes a cycle in the ~SpanTree~ 

Input:

~A~: Node number

~B~: Node number

~SpanTree~: list of span tree edges

*/   

%selection edges build a cycle in the query graph :
completesCycle(A,A,_).

%join edges build a cycle if there is already 
%an existing path from A to B:
completesCycle(A,B,SpanTree):-
 length(SpanTree,L),
 L > 0,
 A =\= B,
 existsPath(A,B,SpanTree).


%there is no cycle if the spanning tree is empty 
completesCycle(A,B,[]):-
 A =\= B,
 !,fail.

/*

1.3 completesCycle

*/

/*
----    completesCycle(+X, +SpanTree)
----

Description:

Succeeds if the edge ~X~ completes a cycle in the ~SpanTree~ 
~X~ must be of type qgEdge([_], A,B,[_], [_],[_],[_])

Input:

~X~: query graph edge of type qgEdge([_], A,B,[_], [_],[_],[_])

~SpanTree~: list of span tree edges

*/   

completesCycle(X,SpanTree):-
 X = qgEdge(_, A,B,_, _,_,_),!,
 completesCycle(A,B,SpanTree).


:- dynamic selectionpredicate/1.

/*

1.4 kruskal

*/

/*
----    kruskal(+QGEdgeList,+QGNodeList,-SpanTree,-RemovedEdgeList, -Stream)
----

Description:

Extended kruskal algorithm 

Input:

~QGEdgeList~: Edges of a query graph

~QGNodeList~: Nodes of a query graph


Output:

~SpanTree~: Edges of the minimum spanning tree calculated from the edges of the query graph

~RemovedEdgeList~: Edges which are member of the query graph but not member of the minimum spanning tree

~Stream~: Plan

*/   

kruskal(QGEdgeList,QGNodeList,SpanTree,RemovedEdgeList, Stream):-
  retractall(component(nodes(_),edges(_),_)),
  retractall(selectionpredicate(_)),
  retractall(notoptimizededge(_)),
  retractall(mcomponent(_,_)),
  retractall(map(_,_)),
  
  splitEdges(QGEdgeList, JoinEdges, SelectionEdges),
  globalizeSelectionPredicates(SelectionEdges),
  
  nl,write('Create components...'),
  createComponents(QGNodeList,_),!,
  nl,write('Components created.'),
  
  quicksort(JoinEdges, QGEdgeListSorted),!,
  
  nl,write('Start Kruskal2...'),
  kruskal2(QGEdgeListSorted,[], SpanTree,RemovedEdgeList),!,
  nl,write('Kruskal2 finished.'),
  
  writeComponents,

  nl,write('Optimize all remaining components...'),
  optimizeAllRemainingComponents,
  nl,write('All remaining components are optimized.'),
  
  nl,write('Materialize all components...'),
  getTime((materializeAllComponents), Time),
  nl,write('All components are materialized.'),
  
  nl,write('Optimize all remaining edges...'),
  optimizeAllRemainingEdges(RemovedEdgeList),
  nl,write('All remaining edges optimized.'),
 
  findall(Plan, component(nodes(_),edges(_),Plan), 
  ComponentList),
  length(ComponentList, L),
  nl,write('kruskal: '), write(L), 
  write(' resulting component(s)'),nl,
  nl,write('Time for materializing all Components: '), 
  TimeInSeconds is Time/1000, write(TimeInSeconds), write(' seconds.'),
  
  ComponentList = [Head|_],
  Stream = Head.

/*

1.5 kruskal2

*/

/*
----    kruskal2(+QGEdgeList,+AkkSpanTree,-SpanTree,-RemovedEdgeList)
----

Description:

kruskal algorithm 

Input:

~QGEdgeList~: Edges of a query graph

~AkkSpanTree~: Akkumulator for the spanning tree edges

Output:

~SpanTree~: Edges of the minimum spanning tree calculated from the edges of the query graph

~RemovedEdgeList~: Edges which are member of the query graph but not member of the minimum spanning tree

~Stream~: Plan

*/   

kruskal2([],Akk, Akk,[]).

kruskal2([MaxSpanEdge|EdgeList], AkkSpanTree, SpanTree, 
RemovedEdgeList):-
  \+completesCycle(MaxSpanEdge,AkkSpanTree),!,
  kruskal2(EdgeList,[MaxSpanEdge|AkkSpanTree], SpanTree, 
  RemovedEdgeList),!,    
  reoptimize(MaxSpanEdge).

kruskal2([MaxEdge|EdgeList],AkkSpanTree, SpanTree, 
[MaxEdge|RemovedEdgeList]):-
  completesCycle(MaxEdge,AkkSpanTree),!, 
  kruskal2(EdgeList,AkkSpanTree, SpanTree, RemovedEdgeList).

/*

1.6 extractSelectionPrecicates

*/  

/* 
----    extractSelectionPrecicates(+QGNode, -SelectionPredicates)
----

Description:
Retrieves all selection predicates for the specified query graph node ~QGNode~ 

*/

extractSelectionPredicates(QGNode, SelectionPredicates):-
  QGNode = qgNode(NodeNumber,_,_),  
  findall(qgEdge(Pred, NodeNumber, NodeNumber, Plan, Size, Cost, Sel),
  selectionpredicate(qgEdge(Pred, NodeNumber, NodeNumber,
  Plan, Size, Cost, Sel)), SelectionPredicates),
  findall(selectionpredicate(qgEdge(Pred, NodeNumber, NodeNumber, 
  Plan, Size, Cost, Sel)),
  selectionpredicate(qgEdge(Pred, NodeNumber, NodeNumber, 
  Plan, Size, Cost, Sel)), SelectionPredicates2),
  removeSelectionPredicates(SelectionPredicates2).

/*

1.7 firstNElements

*/

/*
----    firstNElements(+N, +InList, -OutList)
----

Description:

returns the first n elements of the list ~InList~  as ~OutList~

*/

firstNElements(N, InList, OutList):-
  length(OutList, N),
  append(OutList, _, InList).

/*

1.8 removeSelectionPredicates

*/

/* 
----    removeSelectionPredicates(+List)
----

Description:

All global facts which correlate with the terms in ~List~ are removed.
~List~ elements must be of type 'selectionpredicate(qgEdge(Pred, NodeNo, NodeNo, Plan, Size, Cost, Sel))'

*/
removeSelectionPredicates([]).
removeSelectionPredicates([H|Rest]):-
  H = selectionpredicate(qgEdge(_, NodeNo, NodeNo, _, _, _, _)),
  retract(H),
  removeSelectionPredicates(Rest).

/*

1.9 globalizeSelectionPredicates

*/   
 
/*
----    globalizeSelectionPredicates(+QGEdgeList) 
----

Description:

Creates global facts of type 'selectiopredicate(QGEdge)' from a list of query graph edges ~QGEdgeList~
~QGEdgeList~ elements must be of type 'qgEdge([_], NodeNumber, NodeNumber, [_], [_], [_], [_])'.

*/

globalizeSelectionPredicates([]).
globalizeSelectionPredicates([H|Rest]):-
  H = qgEdge(_, NodeNumber, NodeNumber, _, _, _, _),
  assert(selectionpredicate(H)),
  globalizeSelectionPredicates(Rest).

qgedgeList2predList([], []).
qgedgeList2predList([qgEdge(Pred,_,_,_,_,_,_)|RestEdges], [Pred|RestPreds]):-
  qgedgeList2predList(RestEdges, RestPreds).

/*

1.10 createComponents

*/ 

/*
----    createComponents(+QGNodeList,-ComponentList)
----

Description:

Creates a list of components ~ComponentList~ from a list of query graph nodes ~QGNodeList~. 
A single component is a structure of type "component(nodes(QGNodeList), edges(QGEdgeList), Plan)". 
Initially each component only has a single QGNode in its QGNodeList.

Input:

~QGNodeList~: List of query graph nodes

Output:

~ComponentList~: List of components

*/
 
createComponents([],[]).

createComponents([QGNode|QGNodeList],
[component(nodes([QGNode]), edges([]), Plan)|ComponentList]):-
  QGNode = qgNode(_,rel(_,_),_),
  Plan = 0,
  extractSelectionPredicates(QGNode, SelectionPredicates),
  assert(component(nodes([QGNode]), 
  edges(SelectionPredicates), Plan)),
  createComponents(QGNodeList, ComponentList).

/*

1.11 reoptimize

*/

/*
----    reoptimize(+SpanEdge)
----

Description:

Retrieves the joinable Components and joins them. Two Components are joinable if they contain either the Source Node or the Target Node of the ~SpanEdge~

Input:

~SpanEdge~: Spanning tree edge

*/

reoptimize(SpanEdge):-
  joinableComponents(SpanEdge, Component1, Component2),!,
  joinComponents(SpanEdge, Component1, Component2).

/*

1.12 joinComponents

*/ 

/*
----    joinComponents(+SpanEdge, +Component1, +Component2)
----

Description:

Joins the components ~Component1~ and ~Component2~ to a new Component. 

Input:

~SpanEdge~: Spanning tree edge

~Component1~: Joinable Component

~Component2~: Joinable Component

*/

joinComponents(SpanEdge, Component1, Component2):-
  Component1 = component(nodes(NodeList1),edges(EdgeList1), Plan1), 
  Component2 = component(nodes(NodeList2),edges(EdgeList2),Plan2),
 
  length(EdgeList1,EL1),
  length(EdgeList2,EL2),
  EL3 is EL1 + EL2,  
  getMaxEdgesPerComponent(Max),
  EL3 +1 < Max,     
     
  append(EdgeList1, EdgeList2, JoinedEdgeList),
  append(JoinedEdgeList, [SpanEdge], NewJoinedEdgeList),

  %Add nodes:
  append(NodeList1, NodeList2, JoinedNodeList),  
  
 %remove old components and save new component:
 retract(component(nodes(NodeList1),edges(EdgeList1), Plan1)),
 retract(component(nodes(NodeList2),edges(EdgeList2), Plan2)),
 assert(component(nodes(JoinedNodeList), edges(NewJoinedEdgeList), 0)).


joinComponents(SpanEdge, Component1, Component2):-
  Component1 = component(nodes(NodeList1),edges(EdgeList1), Plan1), 
  Component2 = component(nodes(NodeList2),edges(EdgeList2),Plan2),
  
  length(EdgeList1,EL1),
  length(EdgeList2,EL2),
  EL3 is EL1 + EL2,  
  getMaxEdgesPerComponent(Max),
  EL3 +1 =:= Max,    
   
  nl,write('Build resulting component of maximal size.'),   
    
  append(EdgeList1, EdgeList2, JoinedEdgeList),
  append(JoinedEdgeList, [SpanEdge], NewJoinedEdgeList),

  append(NodeList1, NodeList2, JoinedNodeList),
  
  qgedgeList2predList(EdgeList1, PredList1),
  qgedgeList2predList(EdgeList2, PredList2),
  SpanEdge = qgEdge(Pred, _, _, _, _,_,_),
  
  append(PredList1, PredList2, PredList3),
  append(PredList3, [Pred], Preds), 
  
  getSelect(Select), 
  getRels(Rels),
  
  Query = (Select from Rels where Preds), 
  translate(Query, Stream, _, _, Cost),    
  removePredinfos(Stream, NewJoinedEdgeList, ResultPlan, _, Stream),
  incResultCost(Cost),  

  %remove old components and save new component:
  retract(component(nodes(NodeList1),edges(EdgeList1), Plan1)),
  retract(component(nodes(NodeList2),edges(EdgeList2), Plan2)),
  assert(component(nodes(JoinedNodeList), edges(NewJoinedEdgeList), 
  ResultPlan)).



joinComponents(SpanEdge, Component1, Component2):-
  Component1 = component(nodes(_),edges(EdgeList1),_), 
  Component2 = component(nodes(_),edges(EdgeList2),_),
 
  length(EdgeList1,EL1),
  length(EdgeList2,EL2),
  EL3 is EL1 + EL2,  
  getMaxEdgesPerComponent(Max),
  EL3 +1 > Max,   
  nl,write('Merging both components would cause a too large '), 
  write('resulting component. Optimize the bigger one ...'), 

  compareComponents(Component1, Component2, LargerComponent,_), 
  LargerComponent = component(nodes(NodeListLarger),
  edges(EdgeListLarger),PlanLarger), 
      
  PlanLarger = 0,
  
  largerList(EdgeList1, EdgeList2, LargerEdgeList),
  qgedgeList2predList(LargerEdgeList, Preds),
    
  getSelect(Select), 
  getRels(Rels),
 
  Query = (Select from Rels where Preds),
  translate(Query, Stream, _, _, Cost),    
  removePredinfos(Stream, LargerEdgeList, ResultPlan, _, Stream),    
  incResultCost(Cost),  
 
  %remove old components and save new component:
  retract(LargerComponent),
  assert(component(nodes(NodeListLarger),edges(EdgeListLarger),ResultPlan)),

  %the SpanEdge was not optimized here, i.e. must be saved:
  assert(notoptimizededge(SpanEdge)),
  nl,write('Finished optimizing the bigger component.').


joinComponents(SpanEdge, Component1, Component2):-
  Component1 = component(nodes(_),edges(EdgeList1),_), 
  Component2 = component(nodes(_),edges(EdgeList2),_),
 
  length(EdgeList1,EL1),
  length(EdgeList2,EL2),
  EL3 is EL1 + EL2,  
  getMaxEdgesPerComponent(Max),
  EL3 +1 > Max,   
  
  compareComponents(Component1, Component2, LargerComponent,_), 
  LargerComponent = component(nodes(_),edges(_),PlanLarger), 
  PlanLarger \= 0,
  %the SpanEdge was not optimized here, i.e. must be saved:
  assert(notoptimizededge(SpanEdge)).

/*

1.13 optimizeAllRemainingEdges

*/

/*
----       optimizeAllRemainingEdges(+NonSpanningEdgeList)
----

Description:

Optimizes all non spanning edges and all query graph edges which were not optimized yet. 
Therefore the materialized components are used and a query over them is generated.

*/
optimizeAllRemainingEdges(NonSpanningEdges):-
  findall(qgEdge(Pred,_,_,_,_,_,_), 
  notoptimizededge(qgEdge(Pred,_,_,_,_,_,_)), NotOptimizedEdges),
  length(NotOptimizedEdges, NumberEdges),
  length(NonSpanningEdges, NumberNonSpanningEdges), 
  getAllTemporaryRelNames(TemporaryRelNames),
  length(TemporaryRelNames, NumberTempRels), 
  nl, write('Number of temporary relations: '),
  write(NumberTempRels),
  NumberOfRemainingEdges is NumberEdges + NumberNonSpanningEdges,
  nl, write('Number of remaining edges to be optimized: '),
  write(NumberOfRemainingEdges),
    
  NumberOfRemainingEdges =< 10,
  
  qgedgeList2predList(NotOptimizedEdges, NotOptimizedPreds1),
  qgedgeList2predList(NonSpanningEdges, NotOptimizedPreds2),
  append(NotOptimizedPreds1, NotOptimizedPreds2, NotOptimizedPreds),
  getExternalPredNames(NotOptimizedPreds, PredsExternal),

  getQuery(Query),
  Query    = (select Args from _ where _),
  NewQuery = (select Args from TemporaryRelNames where PredsExternal),
  
  %this is done in predicate 'optimize/3'
  retractall(removefilter(_)),
  rewriteQuery(NewQuery, NewQueryR),
  callLookup(NewQueryR, NewQuery2), !,
  translate(NewQuery2, Stream, _,_,Cost), 
  append(NotOptimizedEdges, NonSpanningEdges, AllNotOptimizedEdges),
  removePredinfos(Stream, AllNotOptimizedEdges, ResultPlan, _, Stream),
  %substituteTempRels(StreamWithoutPredInfo, TemporaryRelNames, ResultPlan),
  incResultCost(Cost),
  retractall(component(_,_,_)),
  assert(component(nodes([]), edges([]), ResultPlan)),
  retractall(notoptimizededge(_)),  
  !.

optimizeAllRemainingEdges(NonSpanningEdges):-
  findall(qgEdge(Pred,_,_,_,_,_,_), 
  notoptimizededge(qgEdge(Pred,_,_,_,_,_,_)), NotOptimizedEdges),
  length(NotOptimizedEdges, NumberEdges),
  length(NonSpanningEdges, NumberNonSpanningEdges), 
  getAllTemporaryRelNames(TemporaryRelNames),
  length(TemporaryRelNames, NumberTempRels), 
  nl, write('Number of temporary relations: '),write(NumberTempRels),
  NumberOfRemainingEdges is NumberEdges + NumberNonSpanningEdges,
  nl, write('Number of remaining edges to be optimized: '),
  write(NumberOfRemainingEdges),
    
  NumberOfRemainingEdges > 10,
    
  qgedgeList2predList(NotOptimizedEdges, NotOptimizedPreds1),
  qgedgeList2predList(NonSpanningEdges, NotOptimizedPreds2),
  append(NotOptimizedPreds1, NotOptimizedPreds2, NotOptimizedPreds),
  getExternalPredNames(NotOptimizedPreds, PredsExternal),

  getQuery(Query),
  Query    = (select Args from _ where _),
  NewQuery = (select Args from TemporaryRelNames where PredsExternal),
  
  %this is done in predicate 'optimize/3'
  retractall(removefilter(_)),
  rewriteQuery(NewQuery, NewQueryR),
  callLookup(NewQueryR, NewQuery2), !,
  qgdm(NewQuery2, _, _,_,_), 
  !.

/*

1.14 optimizeAllRemainingComponents

*/
 
/*
----       optimizeAllRemainingComponents
----

Description:

Generates sub plans for all components which are not optimized yet. 
A component of type 'component(nodes(QGNodes), edges(QGEdges), Plan)'
is considered as optimized if Plan is different from 0.
A component 'component(nodes(QGNodes), edges(QGEdges), 0)' is not considered as optimized.

*/
optimizeAllRemainingComponents:-
  findall(component(nodes(QGNodes), edges(QGEdges), _), 
  component(nodes(QGNodes), edges(QGEdges), 0),
  NotOptimizedComponentList),
  length(NotOptimizedComponentList,Number),
  nl,write(Number),
  write(' remaining components have to be optimized...'),
  optimizeRemainingComponents(NotOptimizedComponentList).

/*

1.15 optimizeRemainingComponents

*/ 

/*
----       optimizeRemainingComponents(+ComponentList)
----

Description:

Optimizes all components passed in the list ~ComponentList~

*/ 
optimizeRemainingComponents([]).
optimizeRemainingComponents([H|Rest]):-
  nl,write('Optimize remaining component...'),
  optimizeRemainingComponent(H),
  nl,write('Finished optimizing remaining component.'),
  optimizeRemainingComponents(Rest).

/*

1.16 optimizeRemainingComponent

*/  

/*
----       optimizeRemainingComponent(+Component)
----

Description:

Optimizes the ~Component~ of tye 'component(nodes(QGNodes), edges(QGEdges), 0)'. 
QGNodes is a list of query graph nodes. QGEdges is a list of query graph edges.
Optimizing means generating a sub plan for QGEdges. Finally the component is asserted as a global fact. 

*/ 

optimizeRemainingComponent(Component):-
  Component = component(nodes(NodeList),edges(EdgeList),_),
  length(EdgeList,NumberEdges),
  NumberEdges > 0,
  qgedgeList2predList(EdgeList, Preds),
    
  getSelect(Select), 
  getRels(Rels),
 
  Query = (Select from Rels where Preds),
  length(Preds, PL),
  nl, write(PL), write(' Predicates in the query.'),
  translate(Query, Stream, _, _, Cost),
  removePredinfos(Stream, EdgeList, ResultPlan, _, Stream),   
  incResultCost(Cost),   

  %remove old components and save new component:
  retract(Component),
  assert(component(nodes(NodeList),edges(EdgeList),ResultPlan)).

optimizeRemainingComponent(Component):-
  Component = component(nodes(NodeList),edges(EdgeList),0),
  length(EdgeList,NumberEdges),
  NumberEdges = 0,
  NodeList = [QGNode],  
  QGNode = qgNode(_,rel(X,Y),_),
  argument(N, rel(X,Y)),
  arg(N) => Plan,
  %remove old components and save new component:
  retract(Component),
  assert(component(nodes(NodeList),edges(EdgeList),Plan)).

/*

1.17 materializeAllComponents

*/ 

/*
----       materializeAllComponents
----

Description:
Materializes all components

*/ 
materializeAllComponents:-
  findall(component(nodes(QGNodes), edges(QGEdges), Plan), 
  component(nodes(QGNodes), edges(QGEdges), Plan),ComponentList),
  length(ComponentList,Number),
  nl,write(Number),write(' components have to be materialized...'),
  materializeComponents(ComponentList).

/*

1.18 materializeComponents

*/

/*
----       materializeComponents(+ComponentList)
----

Description:

Materializes all components passed in the list ~ComponentList~

*/ 
materializeComponents([]).
materializeComponents([H|Rest]):-
  nl,write('Materialize one component...'),
  materializeComponent(H),
  nl,write('Finished materializing one component.'),
  materializeComponents(Rest).
 
/* Special case: Component with 1 node and no edges:*/
materializeComponent(Component):-
  Component = component(nodes(NodeList),edges(Edges),_),
  length(NodeList, NumberNodes),
  NumberNodes =:= 1,
  length(Edges, NumberEdges),
  NumberEdges =:=  0,  
  NodeList = [qgNode(_, rel(RelName,_),_)|_],      
  createRelationNameMap(NodeList,RelName),
  assert(mcomponent(Component, relname(RelName))).

materializeComponent(Component):-
  Component = component(nodes(NodeList),edges(_),Plan),
  plan_to_atom(Plan, Atom),
  newTempRel(X),
  concat_atom(['let ', X, ' = ', Atom, ' consume '],'', Command),
  nl,write('Command: '),write(Command),  
  secondo(Command),
   
  card(X, CardX),  
  (CardX < 50 ->
  createSamples(X, CardX, CardX);true),
        
  createRelationNameMap(NodeList,X),
  assert(mcomponent(Component, relname(X))).

createRelationNameMap([],_).
createRelationNameMap([QGNode|RestQGNodes],NewRelName):-
  QGNode = qgNode(_,rel(A,_),_),
  assert(map(A,NewRelName)),
  createRelationNameMap(RestQGNodes, NewRelName).
 

getAllTemporaryRelNames(TempRelNames):-
  findall(X, mcomponent(_, relname(X)), TempRelNames).
  

deleteAllTemporaryRelations:-
  getAllTemporaryRelNames(TempRelNames),
  nl,write('These temporary relations have to be deleted: '),
  write(TempRelNames),
  deleteTemporaryRelations(TempRelNames),
  dropTempRels,
  deleteTempRels.
  

deleteTemporaryRelations([]).
deleteTemporaryRelations([H|Rest]):-
  drop_relation(H),
  retractall(mcomponent(_,relname(H))),
  deleteTemporaryRelations(Rest).

/*

1.19 largerList

*/

/*

---- largerList(+L1, +L2, -LLarger)
----

Description:

Returns the larger list ~LLarger~ of the two input lists
~L1~ and ~L2~.

*/
largerList(L1, L2, LLarger):-
  length(L1, Length1),
  length(L2, Length2),
  Length1 > Length2,
  LLarger = L1.

largerList(L1, L2, LLarger):-
  length(L1, Length1),
  length(L2, Length2),
  Length1 =< Length2,
  LLarger = L2.

/*

1.20 compareComponents

*/

/*
---- compareComponents(+C1, +C2, -CLarger, -CSmaller)
----

Description:

Returns the larger component ~CLarger~ and the smaller ~CSmaller~ of the two input components
~C1~ and ~C2~. Larger means that there are more query graph edges in the component

*/
compareComponents(C1, C2, CLarger, CSmaller):-
  C1 = component(nodes(_),edges(EdgeList1), _), 
  C2 = component(nodes(_),edges(EdgeList2),_),
  largerList(EdgeList1, EdgeList2, LargerEdgeList),
  LargerEdgeList = EdgeList1,
  CLarger = C1,
  CSmaller = C2.

compareComponents(C1, C2, CLarger, CSmaller):-
  C1 = component(nodes(_),edges(EdgeList1), _), 
  C2 = component(nodes(_),edges(EdgeList2),_),
  largerList(EdgeList1, EdgeList2, LargerEdgeList),
  LargerEdgeList = EdgeList2,
  CLarger = C2,
  CSmaller = C1.

/*

1.21 isIncidentEdgeToA

*/
   
/*

----    isIncidentEdgeToA(+SpanEdge, -Component)
----

Description:

A spanning tree edge is always incident to two components. 
This predicate determines the first (or left) component to which an edge is incident.

Input:

~SpanEdge~: Spanning tree edge

Output:

~Component~: First (or left) incident Component

*/

isIncidentEdgeToA(SpanEdge, Component):-
  SpanEdge = qgEdge(_, A,_,_, _,_,_),
  member(qgNode(A,_,_), QGNodes),
  component(nodes(QGNodes), edges(QGEdges),Plan), 
  Component = component(nodes(QGNodes), edges(QGEdges),Plan).

/*

1.22 isIncidentEdgeToB

*/

/*

----    isIncidentEdgeToB(+SpanEdge, -Component)
----

Description:

A spanning tree edge is always incident to two components. 
This predicate determines the second (or right) component to which an edge is incident.

Input:

~SpanEdge~: Spanning tree edge

Output:

~Component~: Second (or right) incident Component

*/

isIncidentEdgeToB(SpanEdge, Component):-
  SpanEdge = qgEdge(_, _,B,_, _,_,_),
  member(qgNode(B,_,_), QGNodes),
  component(nodes(QGNodes), edges(QGEdges), Plan), 
  Component = component(nodes(QGNodes), edges(QGEdges),Plan).

/*

1.23 joinableComponents

*/

/*

----    joinableComponents(+SpanEdge, -Component1, -Component2)
----

Description:

A spanning tree edge is always incident to two components. 
This predicate determines the two components to which an edge is incident.

Input:

~SpanEdge~: Spanning tree edge

Output:

~Component1~: First (or left) incident Component

~Component2~: Second (or right) incident Component

*/

joinableComponents(SpanEdge, Component1, Component2):-
 isIncidentEdgeToA(SpanEdge, Component1),
 isIncidentEdgeToB(SpanEdge, Component2).

/*

1.24 countComponents

*/

/*

----    countComponents(-N)
----

Description:

Counts the number of global facts of type "component(nodes(QGNodes), Plan)"

Output:

~N~: Number of global facts of type "component(nodes(QGNodes),edges(QGEdges), Plan)"

*/

countComponents(N):-
  findall(component(nodes(QGNodes), Plan), 
  component(nodes(QGNodes), edges(_),Plan), List),!,
  length(List, N).

/*

1.25 reoptimizePlan

*/
 
/*

----    reoptimizePlan(+Plan1, +Plan2, +SpanEdge, -OptimizedPlan)
----

Description:

Calculates a new plan ~OptimizedPlan~ from ~Plan1~, ~Plan2~ and ~SpanEdge~

Input:

~Plan1~: First input plan

~Plan2~: Second input plan

~SpanEdge~: Spanning tree edge

Output:

~OptimizedPlan~: Resulting plan

*/


reoptimizePlan(Plan1, Plan2, SpanEdge, OptimizedPlan):-
SpanEdge = qgEdge(Pred,_,_,_,_,_,_),
  join(Plan1, Plan2, Pred) => OptimizedPlan.


reoptimizePlan(Plan1, Plan2, SpanEdge, OptimizedPlan):-
SpanEdge = qgEdge(Pred,_,_,_,_,_,_),
%use standard join:
OptimizedPlan = symmjoin(Plan1, Plan2, Pred).

/*

1.26 splitEdges

*/

/*
----    splitEdges(+Edges, -JoinEdges, -SelectionEdges)
----

Description:

Splits a list of query graph edges ~Edges~ into 2 lists: 
join edges ~JoinEdges~ and seleciton edges ~SelectionEdges~

*/
splitEdges(Edges, JoinEdges, SelectionEdges):-
 splitEdges2(Edges, JoinEdges, SelectionEdges).

splitEdges2([], [], []).

splitEdges2([H|RestEdges],Joins, [H|RestSelections]):-
  H = qgEdge(_,Source, Source,_,_,_,_), 
  splitEdges2(RestEdges, Joins,RestSelections).

splitEdges2([H|RestEdges],[H|RestJoins], Selections):-
  H = qgEdge(_,Source, Target,_,_,_,_),
  Source =\= Target,
  splitEdges2(RestEdges, RestJoins, Selections).

/*

1.27 mapRelationNames

*/

/*

----    mapRelationNames(+QGEdgeList, -QGEdgeListTemp)
----

Description:

Maps all relation names of the input list of query graph edges ~QGEdge~ to  temporary relation names

Input:

~QGedgeList~: List of query graph edges

Output:

~QGEdgeListTemp~: list of query graph edges with temporary relation names

*/

mapRelationNames([],[]).

mapRelationNames([QGEdge|RestEdges], [QGEdgeTemp|RestTempEdges]):-
  mapRelationName(QGEdge, QGEdgeTemp),
  mapRelationNames(RestEdges, RestTempEdges).

/*

1.28 mapRelationName

*/

/*

----    mapRelationName(+QGEdge, -QGEdgeTemp)
----

Description:

Replaces the relationnames in a query graph edge
qgEdge(pr(P, rel(A,[_]), rel(B,[_])), [_], [_], [_],[_], [_],[_]),
by the temporary relation name

Input:

~QGEdge~: Input query graph edge with relation names

Output:

~QGEdgeTemp~: query graph edge with temporary relation names

*/

mapRelationName(TermFragment, NewTermFragment) :-
  TermFragment = pr(P, rel(A,_), rel(B,_)),
  map(A, TempA),
  map(B, TempB),  
  NewTermFragment = pr(P, rel(TempA,*), rel(TempB,*)),!.

mapRelationName(Term, Term2) :-
  compound(Term), !,
  Term =.. [Functor | Args],
  embeddedmapRelationName(Args, Args2),
  Term2 =.. [Functor | Args2].

mapRelationName(Term, Term).

embeddedmapRelationName([], []).

embeddedmapRelationName([Arg | Args], [Arg2 | Args2]) :-
  mapRelationName(Arg, Arg2),
  embeddedmapRelationName(Args, Args2).


:-dynamic resultCost/1.
incResultCost(Inc):-
 resultCost(Old),
 retractall(resultCost(_)),!,
 New is Old+Inc,
 asserta(resultCost(New)).

initResultCost:-
 retractall(resultCost(_)),!,
 asserta(resultCost(0)).

:-dynamic getSelect/1.
setSelect(Select):-
 retractall(getSelect(_)),!,
 asserta(getSelect(Select)).

:-dynamic getRels/1.
setRels(Rels):-
 retractall(getRels(_)),!,
 asserta(getRels(Rels)).

:-dynamic getMaxEdgesPerComponent/1.
setMaxEdgesPerComponent(Max):-
  retractall(getMaxEdgesPerComponent(_)),!,
  asserta(getMaxEdgesPerComponent(Max)).

:-dynamic getQuery/1.
setQuery(Query):-
 retractall(getQuery(_)),!,
 asserta(getQuery(Query)).

createPredicateMap(PredsExternal, PredsInternal):-
  retractall(predmap(_,_)),
  makeList(PredsExternal, PEList),
  makeList(PredsInternal, PIList),
  createPredicateMap2(PEList, PIList).

createPredicateMap2([],[]).
createPredicateMap2([PE|RestE],[PI|RestI]):-
  assert(predmap(PE,PI)),
  createPredicateMap2(RestE,RestI).

getExternalPredName(PredInternal, PredExternal):-
  predmap(PredExternal, PredInternal).

getExternalPredNames([],[]).
getExternalPredNames([PI|RestI],[PE|RestE]):-
  getExternalPredName(PI, PE),
  getExternalPredNames(RestI, RestE).

/*

1.29 removePredinfos

*/

/*

----    removePredinfos(+Plan, +EdgeList, -ResultPlan, -ResultCost, +RawPlan)
----

Description:

Removes all predinfo(...) terms from ~Plan~ and returns a ~ResultPlan~ without predinfo(...) terms.

Input:

~Plan~: Input plan

~EdgeList~: join edges which are member of the spanning tree

~RawPlan~: Plan without any cost information (required by the cost/5 predicate to calculate the predicate cost within the term)

Output:

~ResultPlan~: Resulting plan

~ResultCost~: Resulting cost

*/

removePredinfos(Plan, EdgeList, ResultPlan, ResultCost, RawPlan):-
  removePredinfos2(Plan, EdgeList, ResultPlan, RawPlan),
  resultCost(ResultCost).

removePredinfos2(Plan, [], Plan,_).  

removePredinfos2(AkkPlan, EdgeList, ResultPlan, RawPlan):-
  EdgeList = [H|Rest],
  removePredinfo(AkkPlan, NewPlan, RawPlan, H, _),
  removePredinfos2(NewPlan, Rest, ResultPlan, RawPlan).

/*

1.30 removePredinfo

*/ 

/*

----    removePredinfo(+PlanFragment, +NewPlanFragment, +RawPlan, +Edge, -Cost)
----
Description:

Removes a predinfo(...) term from ~PlanFragment~ and returns a new new plan fragment ~NewPlanFragment~ without predinfo(...) term.

Input:

~Plan~: Input plan

~Edge~: join edge which is member of the spanning tree

~RawPlan~: Plan without any cost information (required by the cost/5 predicate to calculate the predicate cost within the term)

Output:

~NewPlanFragment~: Resulting plan

~Cost~: Resulting cost

*/   

removePredinfo(PlanFragment, NewPlanFragment, _, _, _) :-  
  PlanFragment = predinfo(Plan, _, _),
  NewPlanFragment = Plan,
  !.

removePredinfo(Term, Term2,RawPlan,Edge, _) :-
  compound(Term), !,
  Term =.. [Functor | Args],
  embeddedPredinfo(Args, Args2, RawPlan, Edge,_),
  Term2 =.. [Functor | Args2].
 

removePredinfo(Term, Term,_, _,_).

embeddedPredinfo([], [],_,_,_).

embeddedPredinfo([Arg | Args], [Arg2 | Args2], RawPlan,Edge,_) :-
  removePredinfo(Arg, Arg2,RawPlan,Edge, _),
  embeddedPredinfo(Args, Args2, RawPlan,Edge, _).

/*

1.31 substituteTempRels

*/

/*

----    substituteTempRels(+Plan, +TempRelList, -ResultPlan)
----

Description:

Substitutes all temporary relations by the plan from mcomponent(component([_],[_],Plan), relname(X))

Input:

~Plan~: Input plan which contains temporary relation names.

~TempRelList~: list of temporary relnames

Output:

~ResultPlan~: Resulting plan tat contains no temporary rel names but plans

*/


substituteTempRels(Plan, TempRelList, ResultPlan):-
  substituteTempRels2(Plan, TempRelList, ResultPlan).

substituteTempRels2(Plan, [], Plan).  

substituteTempRels2(AkkPlan, TempRelList, ResultPlan):-
  TempRelList = [H|Rest],
  substituteTempRel(AkkPlan, NewPlan,H),
  substituteTempRels2(NewPlan, Rest, ResultPlan).

/*

1.32 substituteTempRel

*/  

/*

----    substituteTempRel(+PlanFragment, -NewPlanFragment,  +TempRelName)
----

Description:

Substitutes a temporary relation name by the plan from mcomponent(component([_],[_],Plan), relname(X))

Input:

~PlanFragment~: Input plan

~TempRelName~: temporary relation name to be replaced by the plan

Output:

~NewPlanFragment~: Resulting plan fragement that contains no temporary rel name but plan.

*/   
 
substituteTempRel(PlanFragment, NewPlanFragment, TempRelName) :-    
  PlanFragment = feedproject(rel(TempRelName,*),_),
  mcomponent(component(_,_,Plan), relname(TempRelName)),  
  NewPlanFragment = Plan.  
  
 
substituteTempRel(Term, Term2,TempRelName) :-
  compound(Term), !,
  Term =.. [Functor | Args],
  embeddedsubstituteTempRel(Args, Args2, TempRelName),
  Term2 =.. [Functor | Args2].
 

substituteTempRel(Term, Term,_).

embeddedsubstituteTempRel([], [],_).

embeddedsubstituteTempRel([Arg | Args], [Arg2 | Args2], TempRelName) :-
  substituteTempRel(Arg, Arg2,TempRelName),
  embeddedsubstituteTempRel(Args, Args2,TempRelName).

 
