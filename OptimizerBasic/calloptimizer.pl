/*
//[Appendix] [\appendix]

[Appendix]

1 Initializing and Calling the Optimizer

[File ~calloptimizer.pl~]

The optimizer is started by loading this file.

*/

:- [opsyntax].

:-
  assert(highNode(0)),
  assert(boundarySize(0)),
  assert(boundaryMaxSize(0)),
  [optimizer],
  [database],
  [statistics],
  [operators],
  [boundary],
  [searchtree].
