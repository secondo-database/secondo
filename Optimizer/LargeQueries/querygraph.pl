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
 



[10]  Query Graph  Utility Predicates 

By Gero Willmes, January 2012

Implementations for my master thesis

  


[newpage]

[toc]

[newpage]

*/


/*

1 Query Graph Utility Predicates

1.1 qgcreateNodeList2

*/

/*
----    qgcreateNodeList2(+RelList, -QGNodeList)
----

Description: 

Creates a list of nodes for the query graph recursively. qgcreateNodeList2 is called by qgcreateNodeList.

Input:

~RelList~: list of relations

Output:

~QGNodeList~: list of query graph nodes. The nodes are of type ~qgNode(Number,Rel, Card)~.

*/

qgcreateNodeList2([Rel],[qgNode(1,Rel,Card)]):-
  Rel=rel(RelName,_),
  card(RelName,Card),
  assert(qgNode(1,Rel,Card)),!.  

qgcreateNodeList2([Rel|RelList],QGNodeList) :-
  length(RelList,Length),
  N is Length+1,
  Rel=rel(RelName,_),
  card(RelName,Card),
  assert(qgNode(N,Rel,Card)),!, 
  qgcreateNodeList2(RelList,QGNodeListOld), 
  append(QGNodeListOld,[qgNode(N,Rel,Card)],QGNodeList).



/*

1.2 qgcreateNodeList

*/

/*

----    qgcreateNodeList(+RelList,-QGNodeList) 
----

Description: 

Deletes all global qgnodes and creates a list of nodes for the query graph. 

Input:

~RelList~: list of relations

Output:

~QGNodeList~: list of query graph nodes. The nodes are of type ~qgNode(Number,Rel, Card)~.

*/

qgcreateNodeList(RelList,QGNodeList) :-
  retractall(qgNode(_,_,_)), 
  length(RelList,N),
  N>0,
  !, 
  reverse(RelList, RelList2),
  qgcreateNodeList2(RelList2,QGNodeList).


/*

1.3 findQGNodeNo

*/

/*

----    findQGNodeNo(+Rel,-QGNodeNo)
----

Description:

Retrieves the query graph node number to a given relation

Input:

~Rel~: Relation

Output:

~QGNodeNo~: node number of the query graph

*/

findQGNodeNo(Rel,QGNodeNo):-
  qgNode(X,Rel,_), 
  QGNodeNo = X.  

/*

1.4 qgEdge

*/

/*

---- qgEdge(+Pred, -QGSourceNodeNo, 
    -QGTargetNodeNo, -Plan, -Size, -Cost, -Sel) 
----

Description:

Retrieves to a given predicate ~Pred~ 
the source node number ~QGSourceNodeNo~ and   
the target node number ~QGTargetNodeNo~ in the query graph

Input:

~Pred~:  predicate (selection or join)

Output:

~QGSourceNodeNo~: source node number in the query graph

~QGTargetNodeNo~: target node number in the query graph

Plan, Cost, Size and Sel are not calculated at this point

*/

qgEdge(pr(_,Rel),QGSourceNodeNo, QGTargetNodeNo, _, _, _,_):-
  findQGNodeNo(Rel, QGSourceNodeNo),
  QGTargetNodeNo = QGSourceNodeNo.

qgEdge(pr(_,Rel1,Rel2), QGSourceNodeNo, QGTargetNodeNo, _, _, _,_):-
  findQGNodeNo(Rel1,QGSourceNodeNo),!,
  findQGNodeNo(Rel2,QGTargetNodeNo).


/*

1.5 createQGEdgePlan

*/

/*

----    createQGEdgePlan(+Pred,-QGEdgePlan,-Size,-Cost, -Sel)
----

Description:

Calculates for the current predicate ~Pred~ the plan ~QGEdgePlan~, 
the ~Size~,  the ~Cost~ and the Selectivity ~Sel~

Input:

~Pred~: predicate (join or selection)

Output:

~QGEdgePlan~: edge plan 

~Size~: size of the result

~Cost~: cost of the plan

~Sel~: selectivity of the predicate

*/

createQGEdgePlan(_,QGEdgePlan,Size,Cost,Sel) :-
  edge(Source, Target, Term, Result, _, _),
  Term => QGEdgePlan,
  %calculate plan:
  planEdge(Source, Target, QGEdgePlan, Result),
  %calculate sizes:
  deleteSizes,
  assignSizes,
  %calculate edge cost:  
  deleteCostEdges,
  createCostEdges,
  %calculate edge selectivity:  
  edgeSelectivity(Source, Target, Sel),  
  costEdge(Source, Target, QGEdgePlan, Result, Size, Cost),
  !.

/*

1.6 createQGEdges

*/

/*

----    createQGEdges(+PredList,+RelList,-QGEdgeList)
----

Description:

Calculates to a given list of predicates ~Predlist~
and a list of relations ~RelList~
the edges of the query graph ~QGEdgeList~.

Input: 

~PredList~: list of predicates (join or selection)

~RelList~: list of relations

Output:

~QGEdgeList~: edge list of the query graph

*/

createQGEdges([],_,[]).

createQGEdges([Pred|PredList],RelList,QGEdgeList):-
  pog(RelList,[Pred],_,_),
  %calculate edge plan:
  createQGEdgePlan(Pred, QGEdgePlan, Size, Cost, Sel),
  createQGEdges(PredList, RelList, QGEdgeListOld),
  qgEdge(Pred, QGSourceNodeNo, QGTargetNodeNo, QGEdgePlan,
  Size, Cost, Sel),
  assert(qgEdgeGlobal(Pred, QGSourceNodeNo, QGTargetNodeNo, 
  QGEdgePlan, Size, Cost,Sel)),
  append(QGEdgeListOld,[qgEdge(Pred, QGSourceNodeNo, QGTargetNodeNo,
  QGEdgePlan, Size, Cost, Sel)], QGEdgeList).


/*

1.7 writeQGEdges

*/


/*
----    writeQGEdges
----

Description:

textual output of the query graph edges

*/

writeQGEdges:-
 findall([QGSourceNodeNo, QGTargetNodeNo, Size, Cost, Sel], 
 qgEdgeGlobal(_, QGSourceNodeNo, QGTargetNodeNo,
  _, Size, Cost, Sel), L),
  Format = [ ['Source Node', 'l'],
             ['Target Node', 'l'],
             ['Size', 'l'],
             ['Cost', 'l'],
             ['Sel', 'l'] 
                         ],
  showTuples(L, Format).

/*

1.8 writeQGNodes

*/

/*

----    writeQGNodes
----

Description:

Textual output of the query graph nodes

*/

writeQGNodes :-
  findall([Number,Rel,Card], qgNode(Number,Rel,Card), L),
  Format = [ ['Number', 'l'],
             ['Relation', 'l'], 
             ['Cardinality', 'l']
                           ],	
  showTuples(L, Format).


/*

1.9 writequerygraph

*/

/*
----    writequerygraph
----

Description:

Textual output of the query graph 

*/

writequerygraph:-
  nl,
  write('QueryGraph'),
  nl,
  write('=========='),
  nl,
  nl,
  write('Nodes:'),
  writeQGNodes,
  nl,
  nl,
  write('Edges:'),
  writeQGEdges,
  nl.


/*

1.10 writeSpanEdges

*/

/*
----    writeSpanEdges(+SpanTree)
----

Description:

textual output of the spanning tree edges generated from the query graph.

Input:

~SpanTree~: list of spanning tree edges

Output: textual output

*/

writeSpanEdges(SpanEdges):-
  write('Source Node'), tab(3), write('Target Node'), tab(8),
  write('Size'),tab(8), write('Cost'), tab(8), write('Sel'),nl,
  write('------------------------------------------------------------'),
  nl,
  writeSpanEdges2(SpanEdges).

writeSpanEdges2([]).

writeSpanEdges2([H|SpanEdges]):-
  H = qgEdge(_, QGSourceNodeNo, QGTargetNodeNo, QGEdgePlan, 
  Size, Cost, Sel),
  write(QGSourceNodeNo),tab(13),write(QGTargetNodeNo),tab(18), 
  write(Size),tab(10), write(Cost), tab(6), write(Sel),tab(6),
  nl,
  write('Edge Plan: '), write(QGEdgePlan),
  nl,
  nl,
  writeSpanEdges2(SpanEdges). 


/*

1.11 writespantree

*/

/*

----    writespantree(+SpanTree, +RemovedEdgeList)
----

Description:

Textual output of the spanning tree generated from the query graph 

Input:

~SpanTree~: list of spanning tree edges

~RemovedEdgeList~: list of removed edges (removed from query graph during kruskal algorithm)

Output: textual output

*/

writespantree(SpanTree, RemovedEdgeList):-  
  nl,
  write('Spanning Tree'),
  nl,
  write('============='),
  nl,
  nl,
  write('Spanning tree nodes:'),
  nl,
  writeQGNodes,
  nl,
  nl,
  length(SpanTree, NumberEdges),
  write(NumberEdges), 
  write(' Spanning tree edge(s):'),
  nl,
  nl,
  writeSpanEdges(SpanTree),
  nl,
  nl,
  length(RemovedEdgeList, NumberRemovedEdges),
  write(NumberRemovedEdges),
  write(' Edge(s) removed from query graph during 
  spanning tree algorithm:'),
  nl,
  nl,
  writeSpanEdges(RemovedEdgeList),
  nl,
  write('Number of spanning tree edges:            '),
  write(NumberEdges),
  nl,
  write('Number of edges removed from query graph: '), 
  write(NumberRemovedEdges),
  nl.


/*

1.12 countSelectionEdges

*/

/*

----    countSelectionEdges(+QGEdgeList, +0, -NumberSelections)
----

Description:

Counts the number of selection edges from a list of query graph edges

Input:

~QGEdgeList~: list of query graph edges

Output:

~NumberSelections~: Number of selection edges

*/

countSelectionEdges([],AkkNumber,AkkNumber).
countSelectionEdges([qgEdge(_,Node,Node,_,_,_,_)|QGEdgeList],
AkkNumber, Number):-
  N2 is AkkNumber+1,
  countSelectionEdges(QGEdgeList, N2, Number).

countSelectionEdges([qgEdge(_,A,B,_,_,_,_)|QGEdgeList],
AkkNumber, Number):-
  A =\= B,
  countSelectionEdges(QGEdgeList, AkkNumber, Number).


/*

1.13 analyzeQuery

*/

/*

----    analyzeQuery(+Query)

----

Description:

Creates the querygraph and the spanning tree of the query graph. 
Analyzes the number of selection and join predicates. 
Calcultates the number of connected components of the spanning tree. 

Input:

~Query~: query of Type 'select Args from Rels where Preds'

Output:

Textual output of the query graph and the spanning tree generated from the query graph 

*/
  
analyzeQuery(select Args from Rels where Preds):-
  querygraph(select Args from Rels where Preds, QGNodeList, QGEdgeList),!,
  writequerygraph,!, 
  nl,
  length(Rels, NumberRels),
  length(Preds, NumberPreds),
  write('The query contains '),write(NumberRels),
  write(' relations and '), write(NumberPreds), write(' predicates.'),
  countSelectionEdges(QGEdgeList,0,NumberSelections),
  nl,
  write('Number of selection edges: '), write(NumberSelections),
  nl,
  NumberJoinEdges is NumberPreds-NumberSelections,
  write('Number of join      edges: '), write(NumberJoinEdges),
  nl,
  sortQGEdgesByCost(QGEdgeList,QGSortedEdgeList),!,
  QGSortedEdgeList = [qgEdge(_, SourceNode1, TargetNode1,
  _,_, Cost1, _)|_],!,
  write('Cheapest       edge:  '), write('('),write(SourceNode1), 
  write(','),write(TargetNode1), write(') '), write(' Cost: '), write(Cost1),
  nl,!,
  reverse(QGSortedEdgeList, ReverseSortedEdges),!,
  ReverseSortedEdges = [qgEdge(_, SourceNode2, TargetNode2,_,_, Cost2, _)|_],!,
  write('Most expensive edge:  '), write('('), write(SourceNode2),
  write(','),write(TargetNode2), write(') '), write(' Cost: '), write(Cost2),
  nl,  
  nl,
  kruskal(QGEdgeList,QGNodeList, SpanTree, RemovedEdgeList,_),!, 
  writespantree(SpanTree, RemovedEdgeList),!,
  countComponents(N),!,
  ( N = 1 -> write('Number of connected components:           1');
   write('There is a minimum spanning forest. It contains '), 
  write(N), write(' connected components.')
  )
  ,nl,
  writeComponents.

analyzeQuery(_):-
  nl,
  write('analyzeQuery(Query) must be called with a Query argument
   of type <select Args from Rels where Preds> !'),
  nl,
  !,fail.


/*

1.14 writeComponents

*/

/*
----    writeComponents
----

Description:

Textual output of the spanning tree components

*/

writeComponents:-
  nl,  
  nl,
  findall(component(nodes(QGNodes), edges(QGEdges), Plan), 
  component(nodes(QGNodes), edges(QGEdges), Plan),List),
  length(List, L),
  write(L), write(' Spanning tree component(s):'),
  nl,
  write('---------------'),
  nl,
  nl,  
  writeComponents2(List),  
  
  findall(edge(Source - Target), 
  notoptimizededge(qgEdge(_,Source,Target,_,_,_,_)),NotOptimizedEdgeList),
  write('Not optimized edges: '),
  length(NotOptimizedEdgeList,NOEL), write(NOEL),
  nl,write(NotOptimizedEdgeList),nl,
  
  findall(component(nodes(QGNodes), edges(QGEdges), Plan),
  component(nodes(QGNodes), edges(QGEdges), 0),NotOptimizedComponentList),
  write('Not optimized Components: '),length(NotOptimizedComponentList,NOCL),
  write(NOCL),nl.

writeComponents2([]).
writeComponents2([H|Components]):-
  H = component(nodes(Nodes), edges(Edges),_),
  write('Component'),nl,
  write('Nodes: '), writeComponents3(Nodes),nl,
  write('Edges: '), length(Edges, LE),write(LE),nl,
  writeComponents4(Edges),nl,
  %write('Plan:'), nl,write(Plan),nl,  
  write('----------'),
  nl,
  nl,
  
  writeComponents2(Components),  
  nl.

writeComponents3([]).
writeComponents3([H|Nodes]):-
  H = qgNode(Number,_,_),
  write(Number), write(' '),
  writeComponents3(Nodes).

writeComponents4([]).
writeComponents4([H|Rest]):-
  H = qgEdge(_,N1,N2,_,_,_,_),
  write('('),write(N1), write('-'),write(N2), write(')'),write(' '),
  writeComponents4(Rest).

 


 :- use_module(library(sort)). 
  
compareSizes(Delta,qgEdge(_, _, _,_, Size1, _,_),
qgEdge(_,_,_,_, Size2, _,_)):-
        compare(Delta,Size1,Size2).

compareCosts(Delta,qgEdge(_, _, _,_, _,Cost1,_),
qgEdge(_,_,_,_, _, Cost2,_)):-
        compare(Delta,Cost1,Cost2).

/*

1.15 sortQGEdgesBySize

*/

/*
----    sortQGEdgesBySize(+QGEdgeList,-QGSortedEdgeList)
----

Description:

Sorts the edges of the query graph by edge size

Input:

~QGEdgeList~: list of qgedge/6

Output:

~QGSortedEdgeList~: sorted list of qgedge/6

*/

sortQGEdgesBySize(QGEdgeList,QGSortedEdgeList) :- 
  predsort(compareSizes,QGEdgeList,QGSortedEdgeList).
  
/*

1.16 sortQGEdgesByCost

*/

/*
----    sortQGEdgesByCost(+QGEdgeList,-QGSortedEdgeList)
----

Description:

Sorts the edges of the query graph by Cost

Input:

~QGEdgeList~: list of qgedge/6

Output:

~QGSortedEdgeList~: sorted list of qgedge/6

*/

sortQGEdgesByCost(QGEdgeList,QGSortedEdgeList) :- 
  predsort(compareCosts,QGEdgeList,QGSortedEdgeList).

/*

1.17 removeQGEdge

*/

/*
----    removeQGEdge(+QGEdge,+QGEdgeList,-QGRemainingEdgeList)
----

Description: 
Removes the query graph edge ~QGEdge~ from the list ~QGEdgeList~ 
if ~QGEdge~ is member of ~QGEdgeList~. Oherwise nothing is done.

Input:

~QGEdge~: query graph edge. Must be of type qgEdge/7

~QGEdgeList~: list of qgedges

Output:

~QGRemainingEdgeList~: Remaining edge list

*/ 

removeQGEdge(qgEdge(P, QGNodeNum1, QGNodeNum2, _, Size,Cost,Sel),
[qgEdge(P, QGNodeNum1, QGNodeNum2, _,Size, Cost, Sel)|QGEdgeListRest],
 QGEdgeListRest).

removeQGEdge(qgEdge(P, QGNodeNum1, QGNodeNum2, _, Size, Cost, Sel),
[QGEdge|QGEdgeListRest],[QGEdge|QGEdgeListRest2]):-
  removeQGEdge(qgEdge(P, QGNodeNum1, QGNodeNum2, _, Size, Cost, Sel), 
  QGEdgeListRest, QGEdgeListRest2).

/*

1.18 order

*/

/*
----    order(+A,+B):-
----

Description:
Succeeds if A and B are of type qgEdge([_], A,B,[_], [_],[_],[_]) and B is cheaper than A

Input:

~A~: query graph edge

~B~: query graph edge

*/   

order(A,B):-
  A = qgEdge(_, _,_ , _, _, Cost1,_),
  B = qgEdge(_, _,_ , _, _, Cost2,_),
  !,
  Cost1 > Cost2.  %reverse sort order is needed here!

/*

1.19 quicksort

*/

/*
----    quicksort(+QGEdgeList, -SortedQGEdgeList)
----

Description:

Sorts a list of query graph edges by using the order(A,B) predicate

Input:

~QGEdgeList~: list of query graph edges

Output:

~SortedQGEdgeList~: sorted list of query graph edges

*/   

quicksort([], []).

quicksort([Pivot|L], SL) :-
  divide(Pivot, L, L_Lower, L_Larger),
  quicksort(L_Lower, SL_Lower),
  quicksort(L_Larger, SL_Larger),
  append(SL_Lower, [Pivot|SL_Larger], SL).

% divide: divide the list into two lists 
divide(_, [], [], []).

divide(X, [Y|L], [Y|L_Lower], L_Larger) :-
  order(X,Y),
  %X > Y,
  !,
  divide(X, L, L_Lower, L_Larger).

divide(X, [Y|L], L_Lower, [Y|L_Larger]) :-
  divide(X, L, L_Lower, L_Larger). 

/*

1.20 existsPath

*/

/*
----    existsPath(+A,+B,+SpanTree)
----

Description:

Succeeds if there exists a path in the ~SpanTree~ from node ~A~ to node ~B~

Input:

~A~: Node number

~B~: Node number

~SpanTree~: list of span tree edges

*/   

%direct connection:
existsPath(A,B,SpanTree):-
 existsEdge(A,B,SpanTree,_),!.

%indirect connection:
existsPath(A,B,SpanTree):-
  existsEdge(A,X,SpanTree, Rest),
  existsPath(X,B,Rest),!.

/*

1.21 existsEdge

*/

/*
----    existsEdge(+A,+B,+SpanTree, -Rest)
----

Description:

Succeeds if there exists an edge (A,B) or (B,A) in the ~SpanTree~ 

Input:

~A~: Node number

~B~: Node number

~SpanTree~: list of span tree edges

Output:

~Rest~: If an edge was found, ~Rest~ is ~SpanTree~ without this edge

*/  

existsEdge(A,B,SpanTree, Rest):-
  SpanTree = [qgEdge(_, A,B,_, _,_,_)|Rest].

existsEdge(A,B,SpanTree, Rest):-
  SpanTree = [qgEdge(_, B,A,_, _,_,_)|Rest].

existsEdge(A,B, [H|SpanTree], [H|Rest]):-
  existsEdge(A,B, SpanTree, Rest).

/*

1.22 querygraph

*/

/*
----    querygraph(+select Args from Rels where Preds, 
	-QGNodeList, -QGEdgeList)
----

Description: 

Creates a query graph from a select query

Input: 

select statement of type 'select Args from Rels where Preds' 

Output: 

~QGNodeList~: node list of the query graph

~QGEdgeList~: edge list of the query graph



*/

querygraph(select _ from Rels where Preds, QGNodeList, QGEdgeList) :-
  newQuery,
  lookupRels(Rels,RelList),
  lookupPreds(Preds,PredList),
  !, %no backtracking
  qgcreateNodeList(RelList,QGNodeList),
  %delete all previously created edges:
  retractall(qgEdgeGlobal(_,_,_,_,_,_, _)),
  createQGEdges(PredList,RelList,QGEdgeList).

