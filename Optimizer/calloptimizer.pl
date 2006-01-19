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

//[Appendix] [\appendix]

[Appendix]

1 Initializing and Calling the Optimizer

[File ~calloptimizer.pl~]

The optimizer is started by loading this file.

*/

% Prolog 5.4.7. does not show a prompt when started in rxvt on
% windows using SecondoPL. The predicate below will solve this.
% The solution was found in the SWI-Prolog mailing list.
% http://gollem.science.uva.nl/SWI-Prolog/mailinglist/archive/2005/q3/0349.html

getprompt :- 
  current_stream(0, read, ConIn), set_stream(ConIn, tty(true)),
  current_stream(1, write, ConOut), set_stream(ConOut, tty(true)),
  current_stream(2, write, ConErr), set_stream(ConErr, tty(true)). 

:- [opsyntax].

:-
  assert(highNode(0)),
  assert(boundarySize(0)),
  assert(boundaryMaxSize(0)),

% Change comments to activate the entropy approach!
%
% Uncomment the next line to activate the entropy approach
% ['./Entropy/optimizer'],

% Load files for standard optimizer
  [optimizer],
  [statistics],
  [operators],
  [database],
  [boundary],
  [searchtree],
  [experiments],
  nl, write('Note: Version 5.4.7 shows in the MSYS console no prompt!'), nl,
  write('A workaround is to type in the predicate "getprompt."'), nl, nl.
  
