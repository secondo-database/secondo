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
---- optimizerOptionInfo(+Option,+SUperOption,-Meaning,-GoalOn,-GoalOff)
----
Provides information on optimizer options. ~Option~ is the option in question,
~Meaning~ is a description of that option. When ~Option~ get activated, ~GoalOn~
should be called, is it is deactivated, ~GoalOff~ is called.

~SuperOption~ is the name of the option that ~Option~ is a suboption of. If ~none~,
~Option~ is a top-level option.

*/

optimizerOptionInfo( adaptiveJoin, none,   
                     '\tAllow usage of adaptive join operators.',
                     ( delOption(entropy), 
                       loadFiles(adaptiveJoin) ), 
                     true ).


optimizerOptionInfo(entropy, none,
                    '\tUse entropy to estimate selectivities.',
                    ( delOption(intOrders(on)),
                      delOption(intOrders(quick)),
                      delOption(intOrders(path)),
                      delOption(intOrders(test)),
                      delOption(immediatePlan),
                      loadFiles(entropy), 
                      (   notIsDatabaseOpen
                        ; ( retractall(storedSecondoList(_)),
                            getSecondoList(ObjList),
                            checkForAddedIndices(ObjList), 
                            checkForRemovedIndices(ObjList),
                            checkIfSmallRelationsExist(ObjList),
                            retractall(storedSecondoList(_))
                          )
                      )
                    ), 
	            true ).

/*
----
optimizerOptionInfo(uniformSpeed,     
                    'Set machine speed factor to constant 1.0.',
                    true, true).
optimizerOptionInfo(costsConjuctive,  
           'Apply costs only to operators directly considered by Dijkstra',
           true, true).
----

*/

optimizerOptionInfo(immediatePlan, none,   
                    '\tImmediately create a path rather than the POG.',
                    ( delOption(entropy), 
                      loadFiles(immediatePlan)
                    ),
                    ( delOption(intOrders(on)),
                      delOption(intOrders(quick)),
                      delOption(intOrders(path)),
                      delOption(intOrders(test)),
                      true
                    ) ).

optimizerOptionInfo(intOrders(on), immediatePlan,   
                    '\tConsider interesting orders (on-variant).',
                    ( delOption(entropy),
                      delOption(intOrders(quick)),
                      delOption(intOrders(path)),
                      delOption(intOrders(test)),
                      loadFiles(intOrders),
                      intOrdersPrintWelcomeIO,
                      changeOriginalOptimizer,
                      changeModificationsPL2,
                      correctStoredNodesShape,
                      createMergeJoinPlanEdges,
                      intOrderonImplementation
                    ), 
                    ( doNotCreateMergeJoinPlanEdges,
                      loadFiles(standard),
                      loadFiles(immediatePlan)
                    )).
optimizerOptionInfo(intOrders(path), immediatePlan,   
                    'Consider interesting orders (path-variant).',
                    ( delOption(entropy),
                      delOption(intOrders(on)),
                      delOption(intOrders(quick)),
                      delOption(intOrders(test)),
                      loadFiles(intOrders),
                      intOrdersPrintWelcomeIO,
                      changeOriginalOptimizer,
                      changeModificationsPL2,
                      correctStoredNodesShape,
                      createMergeJoinPlanEdges
                    ), 
                    ( doNotCreateMergeJoinPlanEdges,
                      loadFiles(standard),
                      loadFiles(immediatePlan)
                    )).
optimizerOptionInfo(intOrders(quick), immediatePlan,   
                    'Consider interesting orders (quick-variant).',
                    ( delOption(entropy),
                      delOption(intOrders(on)),
                      delOption(intOrders(path)),
                      delOption(intOrders(test)),
                      loadFiles(intOrders),
                      intOrdersPrintWelcomeIO,
                      changeOriginalOptimizer,
                      changeModificationsPL0,
                      createMergeJoinPlanEdges                      
                    ), 
                    ( doNotCreateMergeJoinPlanEdges,
                      loadFiles(standard),
                      loadFiles(immediatePlan)
                    )).
optimizerOptionInfo(intOrders(test), immediatePlan,   
                    'Consider interesting orders (test-variant).',
                    ( delOption(entropy),
                      delOption(intOrders(on)),
                      delOption(intOrders(quick)),
                      delOption(intOrders(path)),
                      loadFiles(intOrders),
                      intOrdersPrintWelcomeIO,
                      changeOriginalOptimizer,
                      changeModificationsPL1,
                      correctStoredNodesShape,
                      createMergeJoinPlanEdges
                    ), 
                    ( doNotCreateMergeJoinPlanEdges,
                      loadFiles(standard),
                      loadFiles(immediatePlan)
                    )).

optimizerOptionInfo(pathTiming, none,   
                    '\tPrompt time used to find a best path.',
                    true, true).
optimizerOptionInfo(dynamicSample, none,   
                    '\tUse dynamic instead of static (saved) samples.',
                    true, true).
optimizerOptionInfo(rewriteMacros, none,   
                    '\tAllow for macros in queries.',
                    true, true).
optimizerOptionInfo(rewriteInference, none,
                    'Add inferred predicates to where clause.',
                    true, true).
optimizerOptionInfo(rewriteNonempty, rewriteInference,
                    'Handle \'nonempty\' in select statements.',
                    true, true).
optimizerOptionInfo(rtreeIndexRules, rewriteInference,  
                    'Infer predicates to exploit R-tree indices.',
                      (   setOption(rewriteInference), 
                          (
                            not(optimizerOption(entropy))
                          ; notIsDatabaseOpen
                          ; ( retractall(storedSecondoList(_)),
                              getSecondoList(ObjList),
                              checkForAddedIndices(ObjList), 
                              checkForRemovedIndices(ObjList),
                              checkIfSmallRelationsExist(ObjList),
                              retractall(storedSecondoList(_))
                            )
                          )
                      ), 
                      true).
optimizerOptionInfo(rewriteCSE, none,      
                    '\tExtended with attributes for CSE values.',
                    true, delOption(rewriteRemove)).
optimizerOptionInfo(rewriteCSEall, rewriteCSE,     
                    '\tExtend with attributes for _ALL_ CSEs.',
                    true, true).
optimizerOptionInfo(rewriteRemove, rewriteCSE,     
                    '\tRemove attributes as early as possible.',
                    setOption(rewriteCSE), true).
optimizerOptionInfo(debug,none,            
                    '\t\tExecute debugging code. Also use \'toggleDebug.\'.',
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
  findall(X,optimizerOptionInfo(X,_,_,_,_),Options),
  write('\n\nOptimizer options (and sub-options):\n'),
  showOption(Options), 
  write('\nType \'setOption(X).\' to select option X.\n'), 
  write('Type \'delOption(X).\' to unselect option X.\n'),
  write('Type \'showOptions.\' to view this option list.\n\n').

showOption([]).
showOption([Option|Y]) :-
  optimizerOptionInfo(Option,Super,Text,_,_),
  ( Super = none -> write(' [') ; write('    (') ),
  ( optimizerOption(Option) 
      -> write('x')
       ; write(' ')
  ),
  ( Super = none -> write(']    ') ; write(') ') ),
  write(Option), write(':\t'), write(Text), nl,
  showOption(Y), !.

setOption(X) :-
  nonvar(X),
  optimizerOptionInfo(X,_,Text,GoalOn,_),
  retractall(optimizerOption(X)),
  assert(optimizerOption(X)), 
  call(GoalOn),
  write('Switched on option: \''), write(X), write('\' - '), 
  write(Text), write('\n'), !.

setOption(X) :-
  write('Unknown option \''), write(X), write('\'.\n'), fail, !.

delOption(X) :-
  nonvar(X),
  optimizerOptionInfo(X,_,Text,_,GoalOff),
  ( optimizerOption(X)
    -> ( retractall(optimizerOption(X)), 
         call(GoalOff),
         write('Switched off option: \''), write(X), write('\' - '), 
         write(Text), write('\n')
       )
    ;  true
  ), !.

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

/*
3.1 Setting Debugging Options

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
3.2 Switching between optimization module options

*/

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
    retract(loadedModule(standard)), 
    retract(loadedModule(entropy)), 
    assert(loadedModule(entropy))
  )
  ; true.

% Optional files for the immediate plan extension
loadFiles(immediatePlan) :-
  ( not(loadedModule(immediatePlan)),
    [immediateplan],
    retractall(loadedModule(immediatePlan)),
    retractall(loadedModule(completePOG)),
    assert(loadedModule(immediatePlan)),
    immPlanPrintWelcomeMod
  ) 
  ; true.
    
% Optional files for the interesting orders extension
loadFiles(intOrders) :-
  ( not(loadedModule(intOrders)),
    setOption(immediatePlan),
    [intOrders],
    retractall(loadedModule(intOrders)),
    assert(loadedModule(intOrders))
  )
  ; true.

% Optional files for usage of adapative join operators 
loadFiles(adaptiveJoin) :-
  ( not(loadedModule(adaptiveJoin)),
    [adaptiveJoin],
    retractall(loadedModule(adaptiveJoin)),
    assert(loadedModule(adaptiveJoin))
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
:- setOption(entropy).          % Using entropy extension?
% :- setOption(immediatePlan).    % Don't create complete POG?
% :- setOption(intOrders(on)).    % Consider interesting orders (on-variant)?
% :- setOption(intOrders(quick)). % Consider interesting orders (path-variant)?
% :- setOption(intOrders(path)).  % Consider interesting orders (path-variant)?
% :- setOption(intOrders(test)).  % Consider interesting orders (test-variant)?
:- setOption(pathTiming).       % Prompt time used to create immediate plan?
% :- setOption(uniformSpeed).     % Using uniform machine speed factor?
% :- setOption(costsConjunctive). % Applying costs only to conjunctive sub query?
% :- setOption(dynamicSample).    % Using dynamic samples instead of static ones?
:- setOption(rewriteMacros).    % Using macro expansion features?
:- setOption(rewriteInference). % Using automatic inference of predicates?
:- setOption(rtreeIndexRules).  % Use additional rules to exploit R-tree indices?
:- setOption(rewriteNonempty).  % Handle 'nonempty' in select statements?
:- setOption(rewriteCSE).       % Substitute common subexpressions in queries?
% :- setOption(rewriteCSEall).    % Extend attributes for ALL CSEs?
% :- setOption(rewriteRemove).    % Apply early removal of unused attributes?
:- assert(optDebugLevel(selectivity)), setOption(debug). % Activating selectivity debugging code?
% :- assert(optDebugLevel(all)), setOption(debug). % Activating all debugging code?

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
