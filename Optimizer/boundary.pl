/*

1 Interface ~Boundary~

[File ~boundary.pl~]

1.1 The Interface
 
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
1.2 Implementation of Boundary 
 
The boundary is represented by two data structures, namely (i) a predicate

----	boundary(Name, node(Name, Distance, Path))
----

and (ii) a binary search tree with (key, value) pairs of the form

----    (Distance, NodeName)
----

The first serves to get efficient access by node name, as needed in the
functions ~memberByName~ and ~deleteByName~. The binary search tree supports to
find efficiently the minimum, and can also be efficiently updated.


*/

b_empty(nil) :-
  not(deleteBoundary),
	retract(boundarySize(_)),
	retract(boundaryMaxSize(_)),
	assert(boundarySize(0)),
  	assert(boundaryMaxSize(0)).

deleteBoundary :-
  retract(boundary(_, _)), fail.


b_isEmpty(nil).


b_removemin(Tree, Node, Tree2) :-
  tree_min(Tree, _, Name, Tree2),
  boundary(Name, Node),
  retract(boundary(Name, Node)),
  	retract(boundarySize(N)),
  	N1 is N - 1,
  	assert(boundarySize(N1)).


b_insert(Tree, node(Name, Distance, Path), Tree2) :-
  tree_insert(Tree, Distance, Name, Tree2),
  assert(boundary(Name, node(Name, Distance, Path))),
	retract(boundarySize(N)),
	N1 is N + 1,
	assert(boundarySize(N1)),
	adjustMaxSize.

adjustMaxSize :-
  boundarySize(N),
  boundaryMaxSize(M),
  N =< M.

adjustMaxSize :-
  boundarySize(N),
  boundaryMaxSize(M),
  N > M,
  retract(boundaryMaxSize(_)),
  assert(boundaryMaxSize(N)).


b_memberByName(_, Name, Node) :-
  boundary(Name, Node).


b_deleteByName(Tree, Name, Tree2) :-
  boundary(Name, node(Name, Distance, _)),
  tree_delete(Tree, Distance, Name, Tree2),
  retract(boundary(Name, _)),
  	retract(boundarySize(N)),
  	N1 is N - 1,
  	assert(boundarySize(N1)).

/*
1.3 Testing

*/

b_test(T3) :- b_empty(X),
  b_insert(X, node(0, 0, []), T),
  b_insert(T, node(1, 161, [p]), T2),
  b_insert(T2, node(2, 60, [q]), T3).

/*
Material for testing:

----	b_test(T), b_removemin(T, Node, T2), b_removemin(T2, Node2, T3).
	b_test(T), b_memberByName(_, 1, Node).
	b_test(T), b_deleteByName(T, 0, T1).
----


*/











 
 

















 


















