/*
//[<] [$<$]
//[>] [$>$]

1 Binary Search Tree

This module offers a binary search tree with an additional operation to get
(and remove) the minimum, to support a simple implementation of a priority
queue. It supports duplicate entries for keys.

1.1 The Interface
 
----	tree_empty(-Tree).
----

~Tree~ is an empty search tree.

----	tree_insert(+Tree, +Key, +Value, -Tree2)
----

Insertion of the ~Key~, ~Value~ pair into ~Tree~ results in ~Tree2~. Keys must
support the predicates [<], [>], =.

----	tree_delete(+Tree, +Key, +Value, -Tree2)
----

Deletes the ~Key~, ~Value~ pair from ~Tree~, resulting in ~Tree2~. If the pair
is not found in the tree, an error message is given.

----	tree_min(+Tree, -Key, -Value, -Tree2)
----

Returns a ~Key~, ~Value~ pair with minimal key and removes it from ~Tree~,
resulting in ~Tree2~. Fails if the tree is empty.


1.2 Implementation

A tree is represented by a term of the form

----	tree(Key, ValueList, Left, Right)
----

or by the term

----	nil
----

denoting the empty tree.

1.2.1 ~tree\_empty~

*/

tree_empty(nil).

/*
1.2.2 ~tree\_insert~

*/

tree_insert(nil, Key, Value, tree(Key, [Value], nil, nil)) :- !.

tree_insert(tree(KeyT, ValuesT, Left, Right), Key, Value,
	tree(KeyT, ValuesT, Left2, Right)) :-
  Key < KeyT,
  tree_insert(Left, Key, Value, Left2).

tree_insert(tree(KeyT, ValuesT, Left, Right), Key, Value,
	tree(KeyT, ValuesT, Left, Right2)) :-
  Key > KeyT,
  tree_insert(Right, Key, Value, Right2).

tree_insert(tree(Key, ValuesT, Left, Right), Key, Value,
	tree(Key, [Value | ValuesT], Left, Right)).

/*
1.2.3 ~tree\_delete~

*/

tree_delete(nil, Key, Value, nil) :- !,
  write('Error in SearchTree: pair ('), write(Key), write(', '), write(Value),
  write(') not found.'), nl.

tree_delete(tree(KeyT, ValuesT, Left, Right), Key, Value,
	tree(KeyT, ValuesT, Left2, Right)) :-
  Key < KeyT, !,
  tree_delete(Left, Key, Value, Left2).

tree_delete(tree(KeyT, ValuesT, Left, Right), Key, Value,
	tree(KeyT, ValuesT, Left, Right2)) :-
  Key > KeyT, !,
  tree_delete(Right, Key, Value, Right2).

tree_delete(tree(Key, [Value], Left, nil), Key, Value, Left) :- !.

tree_delete(tree(Key, [Value], Left, Right), Key, Value,
	tree(KeyMin, ValuesMin, Left, Right2)) :-  !,
  tree_removeMinNode(Right, KeyMin, ValuesMin, Right2).

tree_delete(tree(Key, ValuesT, Left, Right), Key, Value,
	tree(Key, ValuesT2, Left, Right)) :-
  tree_deleteFromList(ValuesT, Value, ValuesT2).

/*
----	tree_deleteFromList(+List, +Value, -List2)
----

Removing ~Value~ from ~List~ yields ~List2~.

*/
tree_deleteFromList(List, Value, ListOut) :-
  append(List1, [Value | List2], List),
  append(List1, List2, ListOut), !.

tree_deleteFromList(_, Value, _) :-
  write('Error in SearchTree: key found, but value '), write(Value),
  write(' not found.'), nl.

/*
----	tree_removeMinNode(+Tree, -KeyMin, -ValuesMin, -Tree2)
----

Removing from ~Tree~ the node with minimal key ~KeyMin~ and associated
~ValuesMin~ yields ~Tree2~. Must only be called for non-empty trees.

*/

tree_removeMinNode(tree(Key, Values, nil, Right), Key, Values, Right) :- !.

tree_removeMinNode(tree(Key, Values, Left, Right), KeyMin, ValuesMin,
	tree(Key, Values, Left2, Right)) :-
  tree_removeMinNode(Left, KeyMin, ValuesMin, Left2).

tree_removeMinNode(nil, _, _, _) :-
  write('Error in SearchTree: removeMinNode called for empty tree.'), nl.

/*
1.2.4 ~tree\_min~

*/

tree_min(tree(Key, [Value], nil, Right), Key, Value, Right) :- !.

tree_min(tree(Key, [Value, Value2 | Values], nil, Right), Key, Value,
	tree(Key, [Value2 | Values], nil, Right)) :- !.

tree_min(tree(Key, Values, Left, Right), KeyMin, ValueMin,
	tree(Key, Values, Left2, Right)) :-
  tree_min(Left, KeyMin, ValueMin, Left2).

/*
1.3 Testing

*/
test(T3) :-
  tree_insert(nil, 0.7, 1, T), tree_insert(T, 0.44, 2, T1),
  tree_insert(T1, 16, 2, T2), tree_insert(T2, 0.44, 7, T3).

/*

Material for testing:

----	test(T), tree_delete(T, 0.44, 2, T1), tree_delete(T1, 0.44, 7, T2).
	test(T), tree_delete(T, 0.7, 1, T1).

	test(T), tree_min(T, Key, Value, T1), tree_min(T1, Key1, Value1, T2),
	tree_min(T2, Key2, Value2, T3).
----

----	tree_height(+Tree, -Height) :-
----

Returns the height of the search tree.

*/

tree_height(nil, 0).

tree_height(tree(_, _, Left, Right), H) :-
  tree_height(Left, L),
  tree_height(Right, R),
  H is max(L, R) + 1.
