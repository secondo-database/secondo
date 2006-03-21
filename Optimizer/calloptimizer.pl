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

% C. Duentgen, Feb/2006: The solution used deprecated predicates, which now
% have been replaced by newer ones for more recent polog versions.                          

getprompt :-
  current_prolog_flag(version,Version),
  ( Version >= 50600 -> % test for version of SWI-Prolog later than 5.6
      (  % using ISO stream predicates for recent versions
         stream_property(ConIn,  file_no(0)), 
         stream_property(ConIn,  mode(read)),  
         set_stream(ConIn,  tty(true)),

         stream_property(ConOut, file_no(1)), 
         stream_property(ConOut, mode(write)), 
         set_stream(ConOut, tty(true)),

         stream_property(ConErr, file_no(2)), 
         stream_property(ConErr, mode(write)), 
         set_stream(ConErr, tty(true))
      )
    ; (  % using deprecated predicates for old versions
         current_stream(0, read, ConIn), set_stream(ConIn, tty(true)),
         current_stream(1, write, ConOut), set_stream(ConOut, tty(true)), 
         current_stream(2, write, ConErr), set_stream(ConErr, tty(true))
      )
  ).

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
  [searchtree].

%:- [experiments].

:-  nl, write('NOTE: Version 5.4.7 shows in the MSYS console no prompt!'), nl,
    write('A workaround is to type in the predicate "getprompt."'), nl, nl.

:-  current_prolog_flag(windows,true), % query getprompt on windows systems
    getprompt.

/*

2 Properties of the optimizer

Some features of the optimizer may be switched on or off.
Optional features can use the predicate ~optimizerOption/1~ to check
if options are (un)selected and can register with predicate 
~optimizerOptionInfo/2~ to inform the user on their existence and
meaning when typing ~showOptions/0~. Predicates ~setOption/1~ and
~delOption/1~ should be used to select/unselect options.

*/

:- dynamic(optimizerOption/1),
   dynamic(optDebugLevel/1).

/*
---- optimizerOptionInfo(+Option,-Meaning)
----
Provides information on optimizer options. ~Option~ is the option in question,
~Meaning~ is a description of that option.

*/

optimizerOptionInfo(uniformSpeed,     'Instructs the optimizer to use a uniform machine speed factor (1.0)').
optimizerOptionInfo(costsConjuctive,  'Apply costs only to operators directly considered by Dijkstra').
optimizerOptionInfo(dynamicSample,    'Use dynamic samples instead of static ones').
optimizerOptionInfo(rewriteMacros,    'Allow use of macros (with[<expr> as <macro>] in ...)').
optimizerOptionInfo(rewriteInference, 'Automatically add inferred predicates to where clause').
optimizerOptionInfo(rewriteCSE,       'Substitute common subexpressions').
optimizerOptionInfo(debug,            '\tActivate debugging code and messages. Also use \'toggleDebug.\'').

/*
---- showOptions
     setOtion(+Option)
     delOption(+Option)
----
Show information of possible optimizer options and current option settings, select and unselect
optimizer options.

*/

showOptions :- 
  findall(X,optimizerOptionInfo(X,_),Options),
  write('\n\nOptimizer options:\n'),
  showOption(Options), 
  write('\nType \'setOption(X).\' to select option X.\n'), 
  write('Type \'delOption(X).\' to unselect option X.\n'),
  write('Type \'showOptions.\' to view this option list.\n\n').

showOption([]).
showOption([Option|Y]) :-
  optimizerOptionInfo(Option,Text),
  ( optimizerOption(Option) 
      -> write(' [x] ')
       ; write(' [ ] ')
  ),
  write(Option), write(':\t'), write(Text), nl,
  showOption(Y), !.

setOption(X) :-
  nonvar(X),
  optimizerOptionInfo(X,Text),
  retractall(optimizerOption(X)),
  assert(optimizerOption(X)), 
  write('Switched on: \''), write(X), write('\': '), 
  write(Text), write('.\n'), !.

setOption(X) :-
  write('Unknown option \''), write(X), write('\'.\n'), fail, !.

delOption(X) :-
  nonvar(X),
  optimizerOptionInfo(X,Text),
  retractall(optimizerOption(X)), 
  write('Switched off: \''), write(X), write('\': '), 
  write(Text), write('.\n'), !.

delOption(X) :-
  write('Unknown option \''), write(X), write('\'.\n'), fail, !.


/*
3 Code for certain optimizer options

Normally, such code should be placed with the module providing the features,
but here, you will find some things of general interest, like settings for
integrated debugging features.

*/


ppCostFactor(0) :-
  optimizerOption(costConjunctive), !.

ppCostFactor(1) :- !.
 

showDebugLevel :-
  findall(X,optDebugLevel(X),List),
  write('\tDebug levels: '),
  write(List), nl.

toggleDebug :-
  optimizerOption(debug),
  retract(optimizerOption(debug)),
  write('\nNow surpressing all debugging output.'),
  !.

toggleDebug :-
  not(optimizerOption(debug)),
  assert(optimizerOption(debug)),
  write('\nNow displaying debugging output.\n'),
  showDebugLevel,
  write('\tYou can add debug levels by \'debugLevel(level).\'\n'),
  write('\tor drop them by \'nodebugLevel(level).\'.\n'),
  write('\tLevel \'all\' will output messages from all levels.\n'),
  !.

debugLevel(Mode) :- 
  \+(optDebugLevel(Mode)) -> assert(optDebugLevel(Mode)),
  showDebugLevel,
  nl.

nodebugLevel(Mode) :-
  optDebugLevel(Mode) -> retractall(optDebugLevel(Mode)),
  showDebugLevel,
  nl.


/*
4 Optimizer Startup Settings

This are the optional standart settings for the optimizer when getting started. 
Feel free to change.
 
*/

% :- setOption(uniformSpeed).     % Uncomment to use uniform machine speed factor (1.0)
% :- setOption(costsConjunctive). % Uncomment to apply costs only to operators considered by dijkstra
% :- setOption(dynamicSample).    % Uncomment to use dynamic samples instead of static ones
:- setOption(rewriteMacros).    % Comment out to switch off macro expansion features
:- setOption(rewriteInference). % Comment out to switch off automatic inference of predicates
:- setOption(rewriteCSE).       % Comment out to switch off substitution of common subexpressions
:- setOption(debug), assert(optDebugLevel(all)). % Uncomment to see all debugging output

:- showOptions.


