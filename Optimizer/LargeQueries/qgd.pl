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
 



[10] Predicates for Large Query Optimization Algorithm ~Query Graph Decomposition (QGD)~


By Gero Willmes, January 2012

Implementations for my master thesis

  


[newpage]

[toc]

[newpage]

*/




/*

1 Predicates for Large Query Optimization Algorithm ~Query Graph Decomposition (QGD)~

*/

/* 

1.1 qgd

*/
 
/*
----    qgd(+Select from Rels where Preds, -Stream2, -Select2, -Update, -Cost)
----

Description:

Main predicate which does the translation of a large predicate set query
using query graph decomposition (qgd). The signature is identical to translate1/5.

By Gero Willmes


Input:

~Query~: Query of Type 'Select from Rels where Preds'


Output:

~Stream2~: Plan

~Select2~: Arguments of the select clause

~Update~: Update clause

~Cost~: Cost of the Plan

*/   

qgd(Select from Rels where Preds, Stream2, Select2, Update, Cost) :-  
  not( optimizerOption(immediatePlan) ),
  not( optimizerOption(intOrders(_))  ),  % standard behaviour
  nl,nl,
  write('Large predicate set optimization using query graph decomposition (qgd).')
  ,nl,
  set_prolog_flag(unknown, warning),
  
  %create query graph:
  qgcreateNodeList(Rels,QGNodeList),
  retractall(qgEdgeGlobal(_,_,_,_,_,_, _)),
  createQGEdges(Preds,Rels,QGEdgeList),!,

  initResultCost,
  setRels(Rels),
  setSelect(Select),  

  setMaxEdgesPerComponent(4), 
  
  %create spanning tree:
  nl,write('Start Kruskal...'),
  kruskal(QGEdgeList,QGNodeList,_,RemovedEdgeList, Stream),!,
  nl,write('Kruskal finished'),
  writeComponents,
  embedJoinEdges(Stream, RemovedEdgeList, Plan),
  resultCost(Cost),
  
  splitSelect(Select, Select02, Update),  
  % Hook for CSE substitution
  rewritePlanforCSE(Plan, Stream2, Select02, Select2), 
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

%join edges build a cycle if there is already an 
%existing path from A to B:
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
~X~ must be of type qgEdge([_], A,B,[_],[_],[_],[_])

Input:

~X~: query graph edge of type qgEdge([_], A,B,[_],[_],[_],[_])

~SpanTree~: list of span tree edges

*/   

completesCycle(X,SpanTree):-
 X = qgEdge(_, A,B,_, _,_,_),!,
 completesCycle(A,B,SpanTree).

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
  
  nl,write('Optimize all remaining edges...'),
  optimizeAllRemainingEdges,
  nl,write('All remaining edges optimized.'),
  
  findall(Plan, component(nodes(_),edges(_),Plan), ComponentList),
  length(ComponentList, L),
  nl,write('kruskal: '), write(L), write(' resulting component(s)')
  ,nl,
  
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
  kruskal2(EdgeList,[MaxSpanEdge|AkkSpanTree], SpanTree, RemovedEdgeList),!,    
  reoptimize(MaxSpanEdge).

kruskal2([MaxEdge|EdgeList],AkkSpanTree, SpanTree, 
[MaxEdge|RemovedEdgeList]):-
  completesCycle(MaxEdge,AkkSpanTree),!, 
  kruskal2(EdgeList,AkkSpanTree, SpanTree, RemovedEdgeList).
  


:-dynamic selectionpredicate/1.

/*

1.6 extractSelectionPrecicates

*/

/* 
----    extractSelectionPrecicates(+QGNode, -SelectionPredicates)
----

Description:

Retrieves all selection predicates for the specified query graph node ~QGNode~ and removes them from the global list

*/

extractSelectionPredicates(QGNode, SelectionPredicates):-
  QGNode = qgNode(NodeNumber,_,_),
  findall(qgEdge(Pred, NodeNumber, NodeNumber, Plan, Size, Cost, Sel),
  selectionpredicate(qgEdge(Pred, NodeNumber, NodeNumber, 
  Plan, Size, Cost, Sel)), 
  SelectionPredicates),
  findall(selectionpredicate(qgEdge(Pred, NodeNumber, NodeNumber,
  Plan, Size, Cost, Sel)),
  selectionpredicate(qgEdge(Pred, NodeNumber, NodeNumber,
  Plan, Size, Cost, Sel)), SelectionPredicates2),
  removeSelectionPredicates(SelectionPredicates2).

/*

1.7 removeSelectionPredicates

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

1.8 globalizeSelectionPredicates

*/
   
/*
----    globalizeSelectionPredicates(+QGEdgeList) 
----

Description:

Creates global facts of type 'selectiopredicate(QGEdge)' from a list of query graph edges ~QGEdgeList~
~QGEdgeList~ elements must be of type ~qgEdge([_], NodeNumber, NodeNumber, [_], [_],[_], [_])~.

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

1.9 createComponents

*/

/*
----    createComponents(+QGNodeList,-ComponentList)
----

Description:

Creates a list of components ~ComponentList~ from a list of query graph nodes ~QGNodeList~. 
A single component is a structure of type "component(nodes(QGNodeList), edges(QGEdgeList), Plan)". 
Initially each component only has a single QGNode in its QGNodeList and an empty QGEDgeList.

Input:

~QGNodeList~: List of query graph nodes

Output:

~ComponentList~: List of components

*/
 
createComponents([],[]).

createComponents([QGNode|QGNodeList],[component(nodes([QGNode]), 
edges([]), Plan)|ComponentList]):-
  QGNode = qgNode(_,rel(_,_),_),
  Plan = 0,
  extractSelectionPredicates(QGNode, SelectionPredicates),
  assert(component(nodes([QGNode]), edges(SelectionPredicates), Plan)),
  createComponents(QGNodeList, ComponentList).

/*

1.10 reoptimize

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

1.11 joinComponents

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
  Component1 = component(nodes(_),edges(EdgeList1), _), 
  Component2 = component(nodes(_),edges(EdgeList2),_),
 
  length(EdgeList1,EL1),
  length(EdgeList2,EL2),
  EL3 is EL1 + EL2,  
  getMaxEdgesPerComponent(Max),
  EL3 +1 > Max,   
  nl,write('Merging both components would cause'), 
  write(' a too large resulting component. Optimize the bigger one ...'), 

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
  assert(component(nodes(NodeListLarger),edges(EdgeListLarger),
  ResultPlan)),

  %the SpanEdge was not optimized here, i.e. must be saved:
  assert(notoptimizededge(SpanEdge)),
  nl,write('Finished optimizing the bigger component.').


joinComponents(SpanEdge, Component1, Component2):-
  Component1 = component(nodes(_),edges(EdgeList1), _), 
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

1.12 optimizeAllRemainingEdges

*/

/*
----      optimizeAllRemainingEdges
----

Description:

Optimizes all remaining edges in ascending cost order (cost of the individually optimized edges) 
by successively merging all remaining components to one resulting component

*/

optimizeAllRemainingEdges:-
  findall(SpanEdge, notoptimizededge(SpanEdge), NotOptimizedEdgeList),
  quicksort(NotOptimizedEdgeList, SortedEdgeList),
  optimizeRemainingEdges(SortedEdgeList).

optimizeRemainingEdges([]).
optimizeRemainingEdges([H|Rest]):-
  optimizeRemainingEdge(H),
  optimizeRemainingEdges(Rest).

/*

1.13 optimizeRemainingEdge

*/

/*
----       optimizeRemainingEdge(+SpanEdge)
----

Description:

Optimizes a remaining edge ~SpanEdge~ by merging the two incident components and joining ther sub plans to a new plan

*/

optimizeRemainingEdge(SpanEdge):-
  SpanEdge = qgEdge(Pred,_,_,_,_,_,_),
  joinableComponents(SpanEdge, Component1, Component2),!,
  Component1 = component(nodes(QGNodes1), edges(QGEdges1), Plan1),
  Component2 = component(nodes(QGNodes2), edges(QGEdges2), Plan2),
  joinSubPlans(Plan1, Plan2, Pred, NewPlan),
  append(QGEdges1, QGEdges2, JoinedEdgeList),
  append(JoinedEdgeList, [SpanEdge], NewJoinedEdgeList),
  append(QGNodes1, QGNodes2, JoinedNodeList),
  
  retract(Component1),
  retract(Component2),
  retract(notoptimizededge(SpanEdge)),
 
  assert(component(nodes(JoinedNodeList), edges(NewJoinedEdgeList), 
  NewPlan)). 

/*

1.14 embedComponentPlans

*/

/*
----       embedComponentPlans(-TermIn, +TermOut)
----

Description:

Replaces all terms of typ res(N) in ~TermIn~ by "Term" from the fact componentPlan(N, "Term") and returns the result ~TermOut~

*/

embedComponentPlans(res(N), Term) :-
  componentPlan(N, Term), !.

embedComponentPlans(Term, Term2) :-
  compound(Term), !,
  Term =.. [Functor | Args],
  embeddedComponentPlan(Args, Args2),
  Term2 =.. [Functor | Args2].


embedComponentPlans(Term, Term).


embeddedComponentPlan([], []).

embeddedComponentPlan([Arg | Args], [Arg2 | Args2]) :-
  embedComponentPlans(Arg, Arg2),
  embeddedComponentPlan(Args, Args2).

/*

1.15 joinSubPlans

*/

/*
----       joinSubPlans(Plan1, Plan2, Pred, NewPlan)
----

Description:

Joins two sub plans ~Plan1~ and ~Plan2~  by the predicate ~Pred~ to a new plan ~NewPlan~

*/

joinSubPlans(Plan1, Plan2, Pred, NewPlan):-
  assert(componentPlan(1, Plan1)),
  assert(componentPlan(2, Plan2)),
  join(res(1), res(2), Pred) => RawPlan,
  embedComponentPlans(RawPlan, NewPlan),
  retract(componentPlan(1, Plan1)),
  retract(componentPlan(2, Plan2)).

/*

1.16 optimizeAllRemainingComponents

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

1.17 optimizeRemainingComponents

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

1.18 optimizeRemainingComponent

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



incResultCost(Inc):-
 resultCost(Old),
 retractall(resultCost(_)),!,
 New is Old+Inc,
 asserta(resultCost(New)).

initResultCost:-
 retractall(resultCost(_)),!,
 asserta(resultCost(0)).

setSelect(Select):-
 retractall(getSelect(_)),!,
 asserta(getSelect(Select)).

setRels(Rels):-
 retractall(getRels(_)),!,
 asserta(getRels(Rels)).

:-dynamic getMaxEdgesPerComponent/1.
setMaxEdgesPerComponent(Max):-
  retractall(getMaxEdgesPerComponent(_)),!,
  asserta(getMaxEdgesPerComponent(Max)).

/*

1.27 removePredinfos

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

1.28 removePredinfo

*/ 
  
/*

----    (+PlanFragment, +NewPlanFragment, +RawPlan, +Edge, -Cost)
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

1.29 embedJoinEdge

*/

/*

----    embedJoinEdge(+Plan, -ResultPlan, +Edge)
----

Description:

Replaces an expensive join that is member of the query graph but not member of the spanning tree by a selection edge and 
embeds it into the ~Plan~

Input:

~Plan~: Input plan

~Edge~: join edge that is member of the query graph but not member of the spanning tree

Output:

~ResultPlan~: Resulting plan

*/

embedJoinEdge(Plan, NewPlan,Edge):-
  Edge = qgEdge(Pred, _,_,_,_ ,_,_),
  Pred = pr(P, _, _), 
  NewPlan = filter(Plan,P).

/*

1.30 embedJoinEdges

*/

/*

----    embedJoinEdges(+Plan, +JoinEdges, -ResultPlan)
----

Description:

Replaces expensive join edges which are member of the query graph but not member of the spanning tree 
by selection edges and embeds them into the ~Plan~

Input:

~Plan~: Input plan

~JoinEdges~: join edges which aremember of the query graph but not member of the spanning tree

Output:

~ResultPlan~: Resulting plan

*/

embedJoinEdges(Plan,[] ,Plan).
embedJoinEdges(Plan, JoinEdges, NewPlan):-
 embedJoinEdges2(Plan,JoinEdges, NewPlan).

embedJoinEdges2(Resultplan, [], Resultplan).

embedJoinEdges2(Plan, [H|Tail], ResultPlan):-
 embedJoinEdge(Plan, NewPlan, H),
 embedJoinEdges2(NewPlan, Tail, ResultPlan). 
