$Id$

------------------------------------------------------------------------------
Using CVS

Unified diff output
  cvs -q diff -u

Checkout all directories, prune empty directories, remove sticky bit

  cvs -q update -APd

------------------------------------------------------------------------------
Database import

  let wjc3 = readpgn('wjc.pgn') transformstream extend[No: seqnext()] consume
  query wjc3 feed filter[.No = 99] consume
 
The file wjc.pgn must be available in the secondo/bin directory.

------------------------------------------------------------------------------
Testcases

See directory

  secondo/Tests/Testspecs/chessalgebra

and files

  chess.test
  wjc.pgn
  query*

To start the testrunner please do

  cd secondo/Tests/Testspecs/chessalgebra
  Testrunner -i chess.test

------------------------------------------------------------------------------

