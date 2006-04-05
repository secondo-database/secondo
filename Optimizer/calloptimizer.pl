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



% the version predicate can be used to query the current operating
% mode of the query optimizer. Currently one of the modes ~standard~ 
% and ~entropy~ may be chosen.

:-  [opsyntax].

:- dynamic
  version/2,
  loaded/1.

% The files for the standard optimiztation procedure will be
% loaded by default!

mode(standard,'Shortest path search in POG.').
mode(entropy, 'Run a plan computed by the standard optimization on a 
             small database afterwards compute a new plan based on 
             observed and computed selectivities.').


loadFiles(mode(standard,_)) :-
%  Mode = 'standard',
%  not loaded(standard), 
%  ['./Entropy/optimizer'], 
  [optimizer], 
  [statistics],
  [database],
  [operators],
  [boundary],
  [searchtree],
%  retractall(loaded(_)),
  assert(loaded(standard)).

% Optional files for the entropy optimization procedure
loadFiles(mode(entropy,_)) :-
 %  Mode = 'entropy',
  %  not loaded(entropy),
  ['./Entropy/entropy_opt'],
  %  retract(loaded(_)), 
  assert(loaded(entropy)).
  
usingVersion(V) :-
  version(V, on).
 
useVersion(V) :-
  version(V, on),
  printVersion(V).
 
useVersion(V) :-
  version(Last, on),
  loadFiles(mode(V,_)),
  assert( version(V, on) ),
  retract( version(Last, on) ),
  printVersion(V).

 %showVersions :- not showVersions2. 

showVersions :-
  nl, 
  write('Optimization procedure variants:'), nl,
  write('--------------------------------'), nl,
  not showModInfo,
  write('Use the predicate "useVersion/1" to switch between these variants.').
  
showModInfo :-  
  mode(M,Desc),
  write('* "'), write(M), write('": '), write(Desc), nl, nl, fail.
 
 
printVersion(V) :- 
  nl, nl, 
  write('** Optimizer version set to "'), write(V), write('" **'), 
  nl, nl.

 
% Startup procedure for the optimizer
:-
  assert(highNode(0)),
  assert(boundarySize(0)),
  assert(boundaryMaxSize(0)),
  
% always load the files for the standard optimizes  
  assert(version(standard, on)),
  loadFiles(mode(standard,_)),

% A info message for a special prolog version 
  nl, write('Note: Version 5.4.7 shows in the MSYS console no prompt!'), nl,
  write('A workaround is to type in the predicate "getprompt."'), nl,
 
% show info about avaliable versions
  showVersions,
  
% use the standard optimization 
  useVersion(standard).
  
