/*
//[Appendix] [\appendix]

[Appendix]

1 Initializing and Calling the Optimizer

[File ~calloptimizer.pl~]

The optimizer is started by loading this file.

*/

:-
  op(800, xfx, =>),
  op(800, xfx, <=),
  op(800, xfx, #),
  op(800, xfx, div),
  op(800, xfx, mod),
  op(800, xfx, starts),
  op(800, xfx, contains),
  op(200, xfx, :),
  assert(highNode(0)),
  assert(boundarySize(0)),
  assert(boundaryMaxSize(0)),
  [optimizer],
  [database],
  [operators],
  [boundary],
  [searchtree].
