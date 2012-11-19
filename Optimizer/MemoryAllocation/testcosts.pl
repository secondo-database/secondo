/*

$Header$
@author Nikolai van Kempen

The returned function can have only one free variable, because over
this function we want to optimize the memory allocation.
*/
costFunction(Op, Type, [X1], F) :-
  getOpIndexes(Op, Type, ResultType, AlgID, OpID, FunID),
  %F=100*1/(0.1*X1).
  F=100000/(0.01*X1).

costFunction(Op, Type, Tuples, TupleSize, [X1], F) :-
  getOpIndexes(Op, Type, ResultType, AlgID, OpID, FunID),
  getCostFun(AlgID, OpID, FunID, Tuples, TupleSize, FunType, DList),
  write_list(['FunType: ', FunType, '\n']),
  write_list(['DList: ', DList, '\n']),
  %F=100*1/(0.1*X1).
  F=100000/(0.01*X1).


optCost :-
  getOpIndexes(count, [[stream,[tuple,[[a,int]]]]],ResultType, AlgID, OpID, FunID),
  write_list(['Result type: ', ResultType, '\n']),
  write_list(['Algebra ID: ', AlgID, '\n']),
  write_list(['Operator ID : ', OpID, '\n']),
  write_list(['Function ID: ', FunID, '\n']),

  getCosts(AlgID, OpID, FunID, 10000, 100, 16, Costs),
  write_list(['Costs: ', Costs, '\n']).

getOpInfos(OpName, Tuples, TupleSize, FunType, DList) :-
  operatorMemSpec(OpName, _, Blocking, PType),
  getOpIndexes(OpName, PType, ResultType, AlgID, OpID, FunID),
  getCostFun(AlgID, OpID, FunID, Tuples, TupleSize, FunType, DList).

getOpInfos(OpName, Tuples, TupleSize, Tuples2, TupleSize2, FunType, DList) :-
  operatorMemSpec(OpName, Blocking, PType),
  getOpIndexes(OpName, PType, ResultType, AlgID, OpID, FunID),
  getCostFun(AlgID, OpID, FunID, Tuples, TupleSize, Tuples2, TupleSize2,
    FunType, DList).

costFunctionExample :-
  MiB=16,
  costFunctionExample(MiB).

costFunctionExample(MiB) :-
  costFunction(count, [[stream,[tuple,[[a,int]]]]], [MemoryInMiB], F),
  write('Function: '), write_term(F, []), nl,
  MemoryInMiB=MiB,
  Result is F,
  write('Costs: '), write(Result), write('ms'), nl,
  ResultS is Result / 1000,
  write('Costs: '), write(ResultS), write('s'), nl.

% eof
