/*

----
This file is part of SECONDO.

Copyright (C) 2014, University in Hagen, Department of Computer Science,
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


*/



% Section:Start:opSyntaxPreference_3_e
% Section:End:opSyntaxPreference_3_e

/*

2 Executable Language Operator Syntax


----	secondoOp(?Op, ?Syntax, ?NoArgs) :-
----

~Op~ is a Secondo operator written in ~Syntax~, with ~NoArgs~ arguments.
Currently implemented:

  * postfix, N arguments: corresponds to \_ \# and \_ \_ \#, ...

  * postfixbrackets, N:  $N \geq 1$ arments before the functor, the rest following it
      within squared brackets, separated by commas:
        \_ \# [ \_ ], \_ \# [ \_ , \_ ], \_ \# [ \_ , \_ , \_ ] ...

  * prefix, N: $N \geq 0$ arguments following the functor in round parantheses,
      separated by commas:: \# (\_, \_)

  * infix, 2 arguments: \_ \# \_

  * special, N: all operators requiring a special ~plan\_to\_atom~ rule.

There are deprecated standard translations for operators, which should not be used anymore.
Define an explicit ~secondoOP~ fact instead!

  * binary operators: \_ \# \_  infix

  * N-ary operators, $N \neq 2$:  \# ( \_, \_, \_, ... ) prefix

----
Quick Reference:

 OperatorSyntax         |    Use: OperatorType, NoArgs
 -----------------------+------------------------------------------------
 # ()                   |    prefix, 0
 # ( _ )                |    prefix, 1
 # ( _, _ )             |    prefix, 2
 # (  _, _, _, ...)     |    prefix, n ( n>=1 )
 _ #                    |    postfix, 1
 _ # _                  |    infix, 2
 _ # [ _ ]              |    postfixbrackets, 1
 _ # [ _, _ ]           |    postfixbrackets, 1
 _ # [ _, _, ... ]      |    postfixbrackets, 1
 _ _ #                  |    postfix, 2
 _ _ # [ _ ]            |    postfixbrackets, 2
 _ _ # [ _, _ ]         |    postfixbrackets, 2
 _ _ # [ _, _, ... ]    |    postfixbrackets, 2
 _ _ _ #                |    postfix, 3
 _ _ _ # [ _ ]          |    postfixbrackets, 3
 _ _ _ # [ _, _ ]       |    postfixbrackets, 3
 _ _ _ # [ _, _, ... ]  |    postfixbrackets, 3
any using list          |    special, _
any using funlist       |    special, _
all others,             |    special, _

----

*/

:- dynamic(secondoOp/3).


