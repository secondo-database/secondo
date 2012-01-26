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
 



[10]  Predicates for Large Query Optimization Algorithm ~Ascending Cost Order (ACO)~


By Gero Willmes, January 2012

Implementations for my master thesis

  


[newpage]

[toc]

[newpage]

*/

/*

1  Predicates for Large Query Optimization Algorithm 'Ascending Cost Order (ACO)'

*/



/*

1.1 aco

*/
 
/*
----    aco(+Query, -Stream, -Select2, -Update, -Cost)

----

Description:

Main predicate which does the translation of a large predicate set query in ascending cost order (aco). 
The signature is identical to translate/5.

By Gero Willmes

Input:

~Query~: Query of Type 'select Args from Rels where Preds'

Output:

~Stream~: Plan

~Select2~: Arguments of the select clause

~Update~: Update clause

~Cost~: Cost of the Plan

*/   

aco(Select from Rels where Preds, Stream, Select2, Update, Cost) :-
  not( optimizerOption(immediatePlan) ),
  not( optimizerOption(intOrders(_))  ), 
  nl,write('Large Predicate Set Optimization
 using ascending cost order (aco)!'),nl,
     
  %create query graph:
  qgcreateNodeList(Rels,QGNodeList),
  retractall(qgEdgeGlobal(_,_,_,_,_,_, _)),
  createQGEdges(Preds,Rels,QGEdgeList),!,

  %create spanning tree:
  kruskal(QGEdgeList,QGNodeList,_,RemovedEdgeList, Plan),!,
  
  splitEdges(RemovedEdgeList, RemovedJoinEdges, RemovedSelectionEdges),

  %integrate selection edges into the plan:
  embedSelectionEdges(Plan, RemovedSelectionEdges, Plan2),!,
  
  %integrate join edges into the plan:
  embedJoinEdges(Plan2, RemovedJoinEdges, RawPlan),!, 
   
  Cost is 1.0, 
  Stream = RawPlan,  
 
  splitSelect(Select, Select2, Update),
  !,  
  nl.

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

%selection edges build a cycle in the query graph:
completesCycle(A,A,_).

%join edges build a cycle if there is already 
%an existing path from A to B:
completesCycle(A,B,SpanTree):-
 length(SpanTree,L),
 L > 0,
 A =\= B,
 existsPath(A,B,SpanTree).


%there is no cycle if the spanning tree is empty 
%and the node numbers A and B are different:
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

/* 

1.4 kruskal

*/

/*
----    kruskal(+QEdgeList,+QGNodeList,-SpanTree,-RemovedEdgeList, -Stream)
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
  retractall(component(nodes(_),_)),
  createComponents(QGNodeList,_),!,
  quicksort(QGEdgeList, QGEdgeListSorted),!,
  kruskal2(QGEdgeListSorted,[], SpanTree,RemovedEdgeList),!
  
  %if the query graph was connected 
  %there is a single resulting component:
  ,
  findall(Plan, component(nodes(_),Plan), ComponentList),
  length(ComponentList, L),
  write('kruskal: '), write(L), write('resulting component(s)'),nl,
  L=1,
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

/*

1.6 createComponents

*/
  
/*
----    createComponents(+QGNodeList,-ComponentList)
----

Description:

Creates a list of components ~ComponentList~ from a list of query graph nodes ~QGNodeList~. 
A single component is a structure of type "component(nodes(QGNodeList), Plan)". 
Initially each component only has a single QGNode in its QGNodeList.

Input:

~QGNodeList~: List of query graph nodes

Output:

~ComponentList~: List of components

*/
 
createComponents([],[]).

createComponents([QGNode|QGNodeList],
[component(nodes([QGNode]), Plan)|ComponentList]):-
  QGNode = qgNode(_,rel(X,Y),_),
  argument(N, rel(X,Y)),!,%neu
  arg(N) => Plan,%neu
  %Plan =feed(rel(X,Y)),
  assert(component(nodes([QGNode]), Plan)),
  createComponents(QGNodeList, ComponentList).

/*

1.7 reoptimize

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

1.8 joinComponents

*/
 
/*
----    joinComponents(+SpanEdge, +Component1, +Component2)
----

Description:

Joins the components ~Component1~ and ~Component2~ to a new Component. 
Also a new resulting plan is calculated from the old plans of the components and ~SpanEdge~

Input:

~SpanEdge~: Spanning tree edge

~Component1~: Joinable Component

~Component2~: Joinable Component

*/

joinComponents(SpanEdge, Component1, Component2):-
 Component1 = component(nodes(NodeList1), Plan1), 
 Component2 = component(nodes(NodeList2), Plan2),
 append(NodeList1, NodeList2, JoinedNodeList),
 reoptimizePlan(Plan1, Plan2, SpanEdge, OptimizedPlan), 
 %remove old components and save new component:
 retract(component(nodes(NodeList1), Plan1)),
 retract(component(nodes(NodeList2), Plan2)),
 assert(component(nodes(JoinedNodeList), OptimizedPlan)).

/*

1.9 isIncidentEdgeToA

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
  component(nodes(QGNodes), Plan), 
  Component = component(nodes(QGNodes), Plan).

/*

1.10 isIncidentEdgeToB

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
  component(nodes(QGNodes), Plan), 
  Component = component(nodes(QGNodes), Plan).

/*

1.11 joinableComponents

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

1.12 countComponents

*/

/*

----    countComponents(-N)
----

Description:

Counts the number of global facts of type "component(nodes(QGNodes), Plan)"

Output:

~N~: Number of global facts of type "component(nodes(QGNodes), Plan)"

*/

countComponents(N):-
  findall(component(nodes(QGNodes), Plan), 
  component(nodes(QGNodes), Plan), List),!,
  length(List, N).
  
%Start predicates to imupdateplancostsplement:

/*

1.13 reoptimizePlan

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
  assert(componentPlan(1, Plan1)),
  assert(componentPlan(2, Plan2)),
  join(res(1), res(2), Pred) => RawPlan,
  embedComponentPlans(RawPlan, OptimizedPlan),
  retract(componentPlan(1, Plan1)),
  retract(componentPlan(2, Plan2)).

reoptimizePlan(Plan1, Plan2, SpanEdge, OptimizedPlan):-
SpanEdge = qgEdge(Pred,_,_,_,_,_,_),
  join00(Plan1, Plan2, Pred) => OptimizedPlan.

reoptimizePlan(Plan1, Plan2, SpanEdge, OptimizedPlan):-
SpanEdge = qgEdge(Pred,_,_,_,_,_,_),
%use standard join:
OptimizedPlan = symmjoin(Plan1, Plan2, Pred).

/*

1.14 splitEdges

*/

/*
----       splitEdges(+Edges, -JoinEdges, -SelectionEdges)
----

Description:

Splits the list of query graph edges ~Edges~ into 2 lists: ~JoinEdges~ and ~SelecitonEdges~

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

1.15 embedComponentPlans

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

1.16 embedSelectionEdges

*/

/*

----    embedSelectionEdges(+Plan, +RemovedEdgeList, -ResultPlan)
----

Description:

Calculates a new plan ~ResultPlan~ from ~Plan~, and ~RemovedEdgeList~

Input:

~Plan~: Input plan

~RemovedEdgeList~: list of selection edges that were removed from query graph during kruskal algorithm

Output:

~ResultPlan~: Resulting plan

*/

embedSelectionEdges(Plan, [], Plan).

embedSelectionEdges(AkkPlan, RemovedEdgeList, ResultPlan):-
  RemovedEdgeList = [H|Rest],
  embedSelectionEdge(AkkPlan, NewPlan,H),
  embedSelectionEdges(NewPlan, Rest, ResultPlan).

/*

1.17 embedSelectionEdge

*/

/*

----    embedSelectionEdge(+PlanFragment, -NewPlanFragment, +Edge)
----

Description:

Calculates a new plan fragment ~NewPlanFragment~ from ~PlanFragment~, and ~Edge~

Input:

~PlanFragment~: Input plan fragment

~Edge~: selection edge that was removed from query graph during kruskal algorithm

Output:

~NewPlanfragment~: Resulting plan

*/

embedSelectionEdge(PlanFragment, NewPlanFragment, Edge) :-
  Edge = qgEdge(Pred, _, _, _,_, _, _),
  Pred = pr(P, A),
  argument(N, A),
  arg(N) => PlanFragment,
  NewPlanFragment = filter(PlanFragment,P),!.

embedSelectionEdge(Term, Term2, Edge) :-
  compound(Term), !,
  Term =.. [Functor | Args],
  embeddedSelectionEdge(Args, Args2, Edge),
  Term2 =.. [Functor | Args2].

embedSelectionEdge(Term, Term, _).

embeddedSelectionEdge([], [],_).

embeddedSelectionEdge([Arg | Args], [Arg2 | Args2], Edge) :-
  embedSelectionEdge(Arg, Arg2, Edge),
  embeddedSelectionEdge(Args, Args2, Edge).

incResultCost(Inc):-
 resultCost(Old),
 retractall(resultCost(_)),!,
 New is Old+Inc,
 asserta(resultCost(New)).

initResultCost:-
 retractall(resultCost(_)),!,
 asserta(resultCost(0)).

/*

1.18 embedJoinEdge

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

1.19 embedJoinEdges

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
