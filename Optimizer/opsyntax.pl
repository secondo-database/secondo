/*

----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

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
//[<] [$<$]
//[>] [$>$]
//[%] [\%]

//[newpage] [\newpage]

[10] Defining Operator Syntax

[File ~opsyntax.pl~]

[toc]

[newpage]


1 Query Language Operator Syntax


*/

:-
  op(800, xfx, =>),
  op(800, xfx, <=),
  op(800, xfx, #),
  op(800, xfx, div),
  op(800, xfx, mod),
  op(800, xfx, and),
  op(800, xfx, starts),
  op(800, xfx, contains),
  op(200, xfx, :).

:- op(800, xfx, inside).
:- op(800, xfx, insideold).
:- op(800, xfx, intersects).
:- op(800, xfx, adjacent).
:- op(800, xfx, attached).
:- op(800, xfx, overlaps).
:- op(800, xfx, onborder).
:- op(800, xfx, ininterior).
:- op(800, xfx, touchpoints).
:- op(800, xfx, intersection).
:- op(800, xfx, commonborder).
:- op(800, xfx, commonborderscan).

:- op(800, xfx, or).
:- op(800,  fx, not).

:- op(800, xfx, present).
:- op(800, xfx, passes).
:- op(800, xfx, atinstant).
:- op(800, xfx, atperiods).
:- op(800, xfx, at).

:- op(800, xfx, satisfies).
:- op(800, xfx, when).

:- op(800, xfx, simpleequals).

:- op(800, xfx, intersects_new).
:- op(800, xfx, p_intersects).


/*

2 Executable Language Operator Syntax


----	secondoOp(?Op, ?Syntax, ?NoArgs) :-
----

~Op~ is a Secondo operator written in ~Syntax~, with ~NoArgs~ arguments.
Currently implemented:

  * postfix, 1 or 2 arguments: corresponds to \_ \# and \_ \_ \#

  * postfixbrackets1, 2 or more arguments. All but the first one are put into
      the brackets: \_ \# [ \_ ], \_ \# [ \_ , \_ ], \_ \# [ \_ , \_ , \_ ] ...

  * postfixbrackets2: 3 or more arguments. All but the first two are put into
      the brackets: \_ \_ \# [\_ ], \_ \_ \# [\_ , \_ ] ...

  * prefix, 2 arguments: \# (\_, \_)

  * prefix, either 1 or 3 arguments, does not need a rule here, is
translated by default.

  * infix, 2 arguments: does not need a rule, translated by default.

For all other forms, a plan\_to\_atom rule has to be programmed explicitly.
Otherwise, the standard schema is applied:

  * unary operators:   \# ( \_ )           prefix 1

  * binary operators: \_ \# \_              infix

  * N-ary operators:  \# ( \_, \_, \_, ... ) prefix, N [>] 2

----
Quick Reference:

 OperatorSyntax         |    Use: OperatorType, Arity ( '_' = don't care)
 -----------------------+------------------------------------------------
 # ( _ )                |    (no explicit specification)
 # ( _, _ )             |    prefix, 2
 # (  _, _, _, ...)     |    (no explicit specification)
 _ #                    |    postfix, 1
 _ # _                  |    (no explicit specification)
 _ # [ _ ]              |    postfixbrackets, _
 _ # [ _, _ ]           |    postfixbrackets, _
 _ # [ _, _, ... ]      |    postfixbrackets, _
 _ _ #                  |    postfix, 2
 _ _ # [ _ ]            |    postfixbrackets, _
 _ _ # [ _, _ ]         |    postfixbrackets2, _
 _ _ # [ _, _, ... ]    |    postfixbrackets2, _

----

*/

:- dynamic(secondoOp/3).

secondoOp( attrsize,          postfixbrackets1, 2).
secondoOp( avg,               postfixbrackets1, 2).
secondoOp( bbox,              prefix, 1).
secondoOp( between,           postfixbrackets1, 3).
secondoOp( box3d,             prefix, 2).
secondoOp( consume,           postfix, 1).
secondoOp( count,             postfix, 1).
secondoOp( distance,          prefix, 2).
secondoOp( equals,            postfixbarckets2, 4).
secondoOp( everNearerThan,    prefix, 3).
secondoOp( exactmatch,        postfixbrackets2, 3).
secondoOp( exactmatchS,       postfixbrackets1, 2).
secondoOp( extattrsize,       postfixbrackets1, 2).
secondoOp( extend,            postfixbrackets1, 2).
secondoOp( extract,           postfixbrackets1, 2).
secondoOp( exttuplesize,      postfix, 1).
secondoOp( feed,              postfix, 1).
secondoOp( filter,            postfixbrackets1, 2).
secondoOp( gettuples,         postfix, 2).
secondoOp( gettuples2,        postfixbrackets2, 3).
secondoOp( gettuplesdbl,      postfixbrackets2, 3).
secondoOp( getTypeNL,         postfix, 1).
secondoOp( getValueNL,        postfix, 1).
secondoOp( head,              postfixbrackets1, 2).
secondoOp( hashjoin,          postfixbrackets2, 5).
secondoOp( intersection_new,  prefix, 2).
secondoOp( intersection,      prefix, 2).
secondoOp( leftrange,         postfixbrackets2, 3).
secondoOp( leftrangeS,        postfixbrackets1, 2).
secondoOp( like,              postfixbrackets1, 5).
secondoOp( loopjoin,          postfixbrackets1, 2).
secondoOp( loopsel,           postfixbrackets1, 2).
secondoOp( max,               postfixbrackets1, 2).
secondoOp( memshuffle,        postfix, 1).
secondoOp( mergejoin,         postfixbrackets2, 4).
secondoOp( min,               postfixbrackets1, 2).
secondoOp( minus_new,         prefix, 2).
secondoOp( move,              prefix, 2).
secondoOp( nnscan,            postfixbrackets2, 3).
secondoOp( nnsearch,          postfixbrackets2, 4).
secondoOp( pcreate,           postfixbrackets1, 2).
secondoOp( pdelete,           postfix, 1).
secondoOp( pfeed,             postfixbrackets1, 2).
secondoOp( pjoin2,            postfixbrackets2, 4).
secondoOp( product,           postfix, 2).
secondoOp( puse,              postfixbrackets1, 2).
secondoOp( range,             postfixbrackets2, 4).
secondoOp( rangeS,            postfixbrackets1, 3).
secondoOp( rangesearch,       postfixbrackets2, 4).
secondoOp( rdup,              postfix, 1).
secondoOp( remove,            postfixbrackets1, 2).
secondoOp( rightrange,        postfixbrackets2, 3).
secondoOp( rightrangeS,       postfixbrackets1, 2).
secondoOp( rootattrsize,      postfixbrackets1, 2).
secondoOp( roottuplesize,     postfix, 1).
secondoOp( shuffle3,          postfix, 1).
secondoOp( sort,              postfix, 1).
secondoOp( sortby,            postfixbrackets1, 2).
secondoOp( sortmergejoin,     postfixbrackets2, 4).
secondoOp( sortmergejoin_r2,  postfixbrackets2, 4).
secondoOp( spatialjoin,       postfixbrackets2, 4).
secondoOp( sum,               postfixbrackets1, 2).
secondoOp( symmproduct,       postfix, 2).
secondoOp( tail,              postfixbrackets1, 2).
secondoOp( theperiod,         prefix, 2).
secondoOp( tuplesize,         postfix, 1).
secondoOp( union_new,         prefix, 2).
secondoOp( var,               postfixbrackets1, 2).
secondoOp( windowintersects,  postfixbrackets2, 3).
secondoOp( windowintersectsS, postfixbrackets1, 2).



