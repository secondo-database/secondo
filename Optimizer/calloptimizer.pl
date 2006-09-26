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

/*
1.1 Coping with Version-dependant Problems 

1.1.2 SWI-Prolog 5.4.7 for Windows

*/

% Prolog 5.4.7. does not show a prompt when started in rxvt on
% windows using SecondoPL. The predicate below will solve this.
% The solution was found in the SWI-Prolog mailing list.
% http://gollem.science.uva.nl/SWI-Prolog/mailinglist/archive/2005/q3/0349.html

% C. Duentgen, Feb/2006: The solution used deprecated predicates, which now
% have been replaced by newer ones for more recent polog versions.                          
getprompt :-
  current_prolog_flag(version,Version),
  ( (Version >=50407) 
    -> ( (Version >= 50600) % test for version of SWI-Prolog later than 5.6
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
   dynamic(loadedModule/1),
   dynamic(optimizerOptionInfo/5).

/*
---- optimizerOptionInfo(+Option,+SUperOption,-Meaning,-GoalOn,-GoalOff)
----
Provides information on optimizer options. ~Option~ is the option in question,
~Meaning~ is a description of that option. When ~Option~ get activated, ~GoalOn~
should be called, is it is deactivated, ~GoalOff~ is called.

~SuperOption~ is the name of the option that ~Option~ is a suboption of. If ~none~,
~Option~ is a top-level option.

*/

optimizerOptionInfo(standard, none,
                    'Turn off all options. Use standard optimizer.',
                    delAllOptions,
                    true
                   ).

optimizerOptionInfo(nawracosts, none,
                    'Use cost functions as implemented by A. Nawra.',
                    true,
                    true 
                   ).


/*
----
optimizerOptionInfo(uniformSpeed, none,    
                    'Set machine speed factor to constant 1.0.',
                    true, true).
optimizerOptionInfo(costsConjuctive, none,  
           'Apply costs only to operators directly considered by Dijkstra',
           true, true).
----

*/

optimizerOptionInfo(immediatePlan, none,   
                    'Immediately create a path rather than the POG.',
                    ( delOption(entropy), 
                      loadFiles(immediatePlan)
                    ),
                    ( delOption(intOrders(on)),
                      delOption(intOrders(quick)),
                      delOption(intOrders(path)),
                      delOption(intOrders(test))
                    ) 
                   ).

optimizerOptionInfo(intOrders(on), immediatePlan,   
                    'Consider interesting orders (on-variant).',
                    ( delOption(entropy),
                      delOption(intOrders(quick)),
                      delOption(intOrders(path)),
                      delOption(intOrders(test)),
                      setOption(immediatePlan),
                      loadFiles(intOrders),
                      intOrdersPrintWelcomeIO,
                      changeOriginalOptimizer,
                      changeModificationsPL2,
                      correctStoredNodesShape,
                      createMergeJoinPlanEdges,
                      intOrderonImplementation
                    ), 
                    turnOffIntOrders
                   ).
optimizerOptionInfo(intOrders(path), immediatePlan,   
                    'Consider interesting orders (path-variant).',
                    ( delOption(entropy),
                      delOption(intOrders(on)),
                      delOption(intOrders(quick)),
                      delOption(intOrders(test)),
                      setOption(immediatePlan),
                      loadFiles(intOrders),
                      intOrdersPrintWelcomeIO,
                      changeOriginalOptimizer,
                      changeModificationsPL2,
                      correctStoredNodesShape,
                      createMergeJoinPlanEdges
                    ), 
                    turnOffIntOrders
                   ).
optimizerOptionInfo(intOrders(quick), immediatePlan,   
                    'Consider interesting orders (quick-variant).',
                    ( delOption(entropy),
                      delOption(intOrders(on)),
                      delOption(intOrders(path)),
                      delOption(intOrders(test)),
                      setOption(immediatePlan),
                      loadFiles(intOrders),
                      intOrdersPrintWelcomeIO,
                      changeOriginalOptimizer,
                      changeModificationsPL0,
                      createMergeJoinPlanEdges                      
                    ), 
                    turnOffIntOrders
                   ).
optimizerOptionInfo(intOrders(test), immediatePlan,   
                    'Consider interesting orders (test-variant).',
                    ( delOption(entropy),
                      delOption(intOrders(on)),
                      delOption(intOrders(quick)),
                      delOption(intOrders(path)),
                      setOption(immediatePlan),
                      loadFiles(intOrders),
                      intOrdersPrintWelcomeIO,
                      changeOriginalOptimizer,
                      changeModificationsPL1,
                      correctStoredNodesShape,
                      createMergeJoinPlanEdges
                    ),
                    turnOffIntOrders
                   ).

optimizerOptionInfo(pathTiming, none,   
                    'Prompt time used to find a best path.',
                    true, true).
optimizerOptionInfo(dynamicSample, none,   
                    'Use dynamic instead of static (saved) samples.',
                    true, true).
optimizerOptionInfo(rewriteMacros, none,   
                    'Allow for macros in queries.',
                    true, true).
optimizerOptionInfo(rewriteInference, none,
                    'Add inferred predicates to where clause.',
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
                    'Extend with attributes for CSE values.',
                    true, delOption(rewriteRemove)).
optimizerOptionInfo(rewriteCSEall, rewriteCSE,     
                    'Extend with attributes for _ALL_ CSEs.',
                    true, true).
optimizerOptionInfo(rewriteRemove, rewriteCSE,     
                    'Remove attributes as early as possible.',
                    setOption(rewriteCSE), true).

optimizerOptionInfo(debug,none,            
                    'Execute debugging code. Also use \'toggleDebug.\'.',
                    showDebugLevel,true).
optimizerOptionInfo(autosave,none,            
                    'Autosave option settings on \'halt.\'.',
                    true, true).

:- [calloptimizer_sec]. % include more options

/*
---- showOptions
     setOtion(+Option)
     delOption(+Option)
----
Show information of possible optimizer options and current option settings, select and unselect
optimizer options.

*/

showOptions :- 
  findall(X,optimizerOptionInfo(X,none,_,_,_),Options),
  write('\n\nOptimizer options (and sub-options):\n'),
  showOption(Options), 
  write('\nType \'loadOptions.\' to load the saved option configuration.\n'),
  write('Type \'saveOptions.\' to save current option configuration to disk.\n'),
  write('Type \'defaultOptions.\' to restore the default options.\n'),
  write('Type \'setOption(X).\' to select option X.\n'), 
  write('Type \'delOption(X).\' to unselect option X.\n'),
  write('Type \'showOptions.\' to view this option list.\n\n').

showOption([]).
showOption([Option|Y]) :-
  optimizerOptionInfo(Option,none,Text,_,_),
  write(' ['), 
  ( optimizerOption(Option) 
      -> write('x')
       ; write(' ')
  ),
  write(']    '),
  format('~p:~30|~p~n',[Option, Text]),
%  write(Option), write(':\t'), write(Text), nl,
  showSubOptions(Option),
  showOption(Y), !.

showSubOption(_,[]).
showSubOption(Super,[Option|Y]) :-
  optimizerOptionInfo(Option,Super,Text,_,_),
  write('    ('),
  ( optimizerOption(Option) 
      -> write('x')
       ; write(' ')
  ),
  write(') '),
  format('~p:~30|~p~n',[Option, Text]),
%  write(Option), write(':\t'), write(Text), nl,
  showSubOption(Super,Y), !.	

showSubOptions(Super) :-
  findall(X,optimizerOptionInfo(X,Super,_,_,_),SubOptions),
  showSubOption(Super, SubOptions), !.

setOption(X) :-
  nonvar(X),
  optimizerOptionInfo(X,_,Text,GoalOn,_),
  retractall(optimizerOption(X)),
  assert(optimizerOption(X)), 
  call(GoalOn),
  write('Switched ON option: \''), write(X), write('\' - '), 
  write(Text), write('\n'), 
  ( X \= standard 
    -> retractall(optimizerOption(standard))
    ; true
  ), !.

setOption(X) :-
  write('Unknown option \''), write(X), write('\'.\n'), fail, !.

delOption(X) :-
  nonvar(X),
  optimizerOptionInfo(X,_,Text,_,GoalOff),
  ( optimizerOption(X)
    -> ( retractall(optimizerOption(X)), 
         call(GoalOff),
         write('Switched OFF option: \''), write(X), write('\' - '), 
         write(Text), write('\n')
       )
    ;  true
  ), 
  ( not(optimizerOption(_))
    -> assert(optimizerOption(standard))
    ; true
  ), !.

delOneOption(X) :-
  write('Unknown option \''), write(X), write('\'.\n'), 
  showOptions,
  fail, !.

delOneOption :-
  optimizerOptionInfo(X,_,_,_,_), 
  X \= standard, 
  delOption(X),
  fail.

delAllOptions :-
  not(delOneOption),
  write('\nSwitched off all options. Optimizer now works in standard mode.\n').
  

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
    assert(loadedModule(entropy))
  )
  ; true.

% Optional files for the immediate plan extension
loadFiles(immediatePlan) :-
  ( not(loadedModule(immediatePlan)),
    [immediateplan],
    assert(loadedModule(immediatePlan)),
    immPlanPrintWelcomeMod
  ) 
  ; true.
    
% Optional files for the interesting orders extension
loadFiles(intOrders) :-
  ( not(loadedModule(intOrders)),
    [intOrders],
    assert(loadedModule(intOrders))
  )
  ; true.

% Optional files for usage of adapative join operators 
loadFiles(adaptiveJoin) :-
  ( not(loadedModule(adaptiveJoin)),
    [adaptiveJoin],
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

:- assert( storeupdateRel(0) ),
   assert( storeupdateIndex(0) ),
   assert( storedDatabaseOpen(0) ),
   assert( highNode(0) ),
   assert( boundarySize(0) ),
   assert( boundaryMaxSize(0) ).
  
/*
5.2 Setting Startup Options
 
The option configuration is stored in a file ~config_optimizer.pl~ between two sessions. It will be loaded and restored on subsequent sessions. If this file does not exist, default options are used.
 
~defaultOptions~ restores the built-in default option settings.

~loadOptions~ restores the option config from disk.

~saveOptions~ saves the current options to disk.

If ~optimizerOption(autosave)~ is defined, current option settings will be saved
to disk automatically on system halt.

*/

defaultOptions :-
  setOption(standard),
  setOption(debug),
  debugLevel(selectivity),
  setOption(autosave).


loadOptions :- consult('config_optimizer.pl').

saveSingleOption(Stream) :-
  optimizerOption(X),
  write(Stream, ':- '),
  write(Stream, setOption(X)), 
  write(Stream, '.\n').

saveOptions :- 
  open('config_optimizer.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, saveSingleOption(FD), _),
  close(FD).

initializeOptions :-
  catch(
	 (loadOptions, 
	  write('Loaded options\n')
         ), 
         _, % catch all exceptions
         (defaultOptions,
	  saveOptions, % save a blanc 'config_optimizer.pl'
          write('Saved options\n')
         )
       ).

:- at_halt((optimizerOption(autosave), saveOptions)). % automatically safe option configuration on exit
:- initializeOptions.

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


/*
7 Testing

*/


/*
Testing in database opt
----

:- [autotest], open 'database opt'.

tt :- runExamples, showOptions.

q1 :- sql select count(*) from[plz as a, plz as b] where[a:ort = b:ort].
q2 :- sql select count(*) from[plz as p1, plz as p2, staedte as s] where[s:sname = p1:ort, p2:ort = s:sname].

s1 :- setOption(intOrders(quick)), tt, q1.
s2 :- setOption(intOrders(on)), q1.
s3 :- setOption(intOrders(path)), q1.
s4 :- setOption(intOrders(test)), q1.

----

*/

