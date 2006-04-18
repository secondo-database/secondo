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
  ( Version >=50407 
    -> ( Version >= 50600 % test for version of SWI-Prolog later than 5.6
          -> (  % using ISO stream predicates for recent versions
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
       )
     ; true).

 
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
   dynamic(optDebugLevel/1),
   dynamic(loadedModule/1).

/*
---- optimizerOptionInfo(+Option,-Meaning,-GoalOn,-GoalOff)
----
Provides information on optimizer options. ~Option~ is the option in question,
~Meaning~ is a description of that option. When ~Option~ get activated, ~GoalOn~
should be called, is it is deactivated, ~GoalOff~ is called.

*/

optimizerOptionInfo(entropy,       
                    '\tEstimate selectivities by maximizing the entropy.',
                    ( loadFiles(entropy), 
                      (   notIsDatabaseOpen
                        ; ( getSecondoList(ObjList),
                            checkForAddedIndices(ObjList), 
                            checkForRemovedIndices(ObjList),
                            checkIfSmallRelationsExist(ObjList),
                            retractall(storedSecondoList(_))
                          )
                      )
                    ), 
	            loadFiles(standard)).
%optimizerOptionInfo(uniformSpeed,     
%                    'Set machine speed factor to constant 1.0.',
%                    true, true).
%optimizerOptionInfo(costsConjuctive,  
%           'Apply costs only to operators directly considered by Dijkstra',
%           true, true).
optimizerOptionInfo(dynamicSample,    
                    'Use dynamic instead of static (saved) samples.',
                    true, true).
optimizerOptionInfo(rewriteMacros,    
                    'Allow for macros (with[<expr> as <macro>] in <query>).',
                    true, true).
optimizerOptionInfo(rewriteInference, 
                    'Automatically add inferred predicates to where clause.',
                    true, true).
optimizerOptionInfo(rewriteCSE,       
                    'Substitute common subexpressions by extended attributes.',
                    true, delOption(rewriteRemove)).
optimizerOptionInfo(rewriteRemove,       
'Remove attributes as early as possible.\n\t\t\t(NOTE: This auto-selects \'rewriteCSE\'!)',
                    setOption(rewriteCSE), true).
optimizerOptionInfo(debug,            
                    '\tActivate debugging code. Also use \'toggleDebug.\'.',
                    showDebugLevel,true).

/*
---- showOptions
     setOtion(+Option)
     delOption(+Option)
----
Show information of possible optimizer options and current option settings, select and unselect
optimizer options.

*/

showOptions :- 
  findall(X,optimizerOptionInfo(X,_,_,_),Options),
  write('\n\nOptimizer options:\n'),
  showOption(Options), 
  write('\nType \'setOption(X).\' to select option X.\n'), 
  write('Type \'delOption(X).\' to unselect option X.\n'),
  write('Type \'showOptions.\' to view this option list.\n\n').

showOption([]).
showOption([Option|Y]) :-
  optimizerOptionInfo(Option,Text,_,_),
  ( optimizerOption(Option) 
      -> write(' [x] ')
       ; write(' [ ] ')
  ),
  write(Option), write(':\t'), write(Text), nl,
  showOption(Y), !.

setOption(X) :-
  nonvar(X),
  optimizerOptionInfo(X,Text,GoalOn,_),
  retractall(optimizerOption(X)),
  assert(optimizerOption(X)), 
  call(GoalOn),
  write('Switched on option: \''), write(X), write('\' - '), 
  write(Text), write('\n'), !.

setOption(X) :-
  write('Unknown option \''), write(X), write('\'.\n'), fail, !.

delOption(X) :-
  nonvar(X),
  optimizerOptionInfo(X,Text,_,GoalOff),
  retractall(optimizerOption(X)), 
  call(GoalOff),
  write('Switched off option: \''), write(X), write('\' - '), 
  write(Text), write('\n'), !.

delOption(X) :-
  write('Unknown option \''), write(X), write('\'.\n'), 
  showOptions,
  fail, !.


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

% The files for the standard optimization procedure will be
% loaded by default!
loadFiles(standard) :-
  ( not(loadedModule(standard)),
    [optimizer], 
    [statistics],
    [database],
    [operators],
    [boundary],
    [searchtree],
    retractall(loadedModule(_)),
    assert(loadedModule(standard))
  )
  ; true.

% Optional files for the entropy optimization procedure
loadFiles(entropy) :-
  ( not(loadedModule(entropy)),
    ['./Entropy/entropy_opt'],
    retract(loadedModule(_)), 
    assert(loadedModule(entropy))
  )
  ; true.


/*
4 Some Auxiliary predicates for Debugging

*/

/*
Print debugging information

Predicate ~dm/1~ can be used as ~write~/1. Output is printed when optimizer option
debug is defined (see file operators.pl).

Predicate ~dm(mode,message)~ writes ~message~ if ~optDebugLevel(mode)~ 
or ~optDebugLevel(all)~ is defined.

*/

dm([]) :- !.

dm([X1 | XR]) :-
  optimizerOption(debug), !,
  write(X1),
  dm(XR).

dm(X) :-
  optimizerOption(debug), !,
  write(X).

dm(_) :- !.
  
dm(Level, X) :-
  ( optDebugLevel(Level) ; optDebugLevel(all) ), !, 
  dm(X).

dm(_,_) :- !.

/*
Execute debugging code

dc works like dm, but calls a goal instead of simply printing messages:

*/

dc(Command) :-
  optimizerOption(debug), !,
  call(Command).

dc(_) :- !.

dc(Level, Command) :-
  ( optDebugLevel(Level) ; optDebugLevel(all) ), !,
  dc(Command).

dc(_,_) :- !.


/*
5 Optimizer Startup Procedure

*/

/*
5.1 Loading and Initializing Modules

*/

% Startup procedure for the optimizer
:- [opsyntax].
:- loadFiles(standard).        % load the files for the standard optimizer 
:- assert(highNode(0)),
   assert(boundarySize(0)),
   assert(boundaryMaxSize(0)).
  
/*
5.2 Setting Startup Options


This are the optional standart settings for the optimizer when getting started. 
Feel free to change.
 
*/
% :- setOption(entropy).          % Using entropy extension?
% :- setOption(uniformSpeed).     % Using uniform machine speed factor?
% :- setOption(costsConjunctive). % Applying costs only to conjunctive sub query?
% :- setOption(dynamicSample).    % Using dynamic samples instead of static ones?
:- setOption(rewriteMacros).    % Using macro expansion features?
:- setOption(rewriteInference). % Using automatic inference of predicates?
:- setOption(rewriteCSE).       % Substitute common subexpressions in queries?
% :- setOption(rewriteRemove).    % Apply early removal of unused attributes?
% :- setOption(debug), assert(optDebugLevel(all)). % Activating debugging code?

/*
5.3 Print Additional Information

*/

:- showOptions.
:-   nl, write('NOTE: SWI-Prolog Version 5.4.7 shows '),
     write('no prompt in the MSYS console!'), nl,
     write('      A workaround is to type in the predicate "getprompt."'), nl, nl.
:- ( current_prolog_flag(windows,true), % query getprompt on windows systems
     getprompt
   ) ; true.



/*
6 Shortcuts and Aliases

*/

quit :- halt. % aliasing 'halt/0' in conformity to the Secondo system
