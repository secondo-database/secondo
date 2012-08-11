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

*/

/*

Predicate ~load_storefiles~: Safely consult files.

----
  load_storefiles(:Files)
  load_storefiles(:Files, +Options)
----

Failsave loading of stored meta data. Files are created if necessary before
trying to read them.

*/

% open the file for append to create is prior to read access (which would fail
% with an error if the file does not exist yet).
open_files([F|More]) :-
  open_files(F),
  open_files(More), !.

open_files(F) :-
  atom_concat(F,'.pl',F2),
  open(F2, append, Bla, [type(text),close_on_abort(true)]),
  close(Bla).

% more specific - set user-defined options
load_storefiles(Files, Options) :-
  open_files(Files),
  load_files(Files,Options).

% default version with standard options
load_storefiles(Files) :-
  load_storefiles(Files,[autoload(true), must_be_module(false),
                                    silent(true), format(source)]).

/*
0 Help on User Level Predicates

There are a lot of predicates within the optimizer, that are intended to be
(or may be) directly called by the user.

To allow for a help functionality, for each such predicate, a fact describing
the predicate, its syntax, parameters and meaning should be asserted.

These facts have format

---- helpLine(Name,Arity,ParamList,Meaning)
----

~Name~ is the predicate's name, ~Arity~ its arity (number of Parameters),
~Meaning~ is the meaning of the predicate, and

~ParamList~ is a list of list with format

---- [ParamType,ParamName,ParamMeaning]
----

~ParamType~ is one of + (input type, - (output type), ? (input/output type).
~ParamName~ is the parameter's name, and ~ParamMeaning~ is a textual description
of the parameter's meaning.

The user level predicates

----
 helpMe
 helpMe(+PredicateName)
----

will print the according (and hopefully helpful) information to the screen.

*/
:- dynamic(helpLine/4).

:- assert(helpLine(helpMe,2,
    [[+,'PredicateName','The predicate to get information about.'],
     [+,'Arity','Chooses amongh predicates with more than one arity.']],
    'Show help on a user level predicate with a given arity.')).
:- assert(helpLine(helpMe,1,
    [[+,'PredicateName','The predicate get get information about.']],
    'Show help on a given user level predicate.')).
:-assert(helpLine(helpMe,0,[],'List available user level predicates.')).

helpMe(Pred, Arity) :-
  helpLine(Pred, Arity, Params, Meaning),
  nl, write('Help on predicate \''), write(Pred), write('/'), write(Arity),
  write('\':  '), write(Meaning), nl,                                       %'
  write('  '), write(Pred),
  ( Arity > 0
    -> ( write('('), nl,
        helpMeWriteParamList(Params),
         write('  )')
       )
    ; true
  ),
  nl, !.

helpMe(Pred, Arity) :-
  nl, write('No help available for predicate \''), write(Pred), write('/'), %'
  write(Arity), write('\'.'), nl,                                           %'
  fail.

helpMe(Pred) :-
  findall( [Pred, Arity, Meaning], helpLine(Pred, Arity, _, Meaning), PredList),
  sort(PredList,PredList2),
  length(PredList2, L),
  ( L > 1
    -> ( nl,
         write('There are several arities for predicate \''), write(Pred), %'
         write('\'\n'),                                                    %'
         write('Help is available on the following arities:'), nl,
         format('  ~w~20|/~w~28|~w~n',['Predicate','Arity','Meaning']),
         write('---------------------------------------------------------'), nl,
         helpMePrintLine(PredList2),
         nl, write('Use \'helpMe(Pred,Arity).\''),
         write(' for help on a certain arity of that predicate.'), nl
       )
    ;  ( L = 1
         -> ( PredList2 = [[Pred, Arity, _]],
              helpMe(Pred, Arity)
            )
         ;  ( % L = 0
              write('There is no help on predicate \''), write(Pred),    %'
              write('\'.'), nl                                           %'
            )
       )
  ),
  !.

helpMe :-
  nl, write('Help is available on the following user level predicates:'), nl,
  format('  ~w~20|/~w~28|~w~n',['Predicate','Arity','Meaning']),
  write('---------------------------------------------------------'), nl,
  findall( [Pred, Arity, Meaning],
          ( helpLine(Pred , Arity, _, Meaning)
          ),
          PredList),
  sort(PredList,PredList2),
  helpMePrintLine(PredList2),
  nl, write('Use \'helpMe(Pred).\' or \'helpMe(Pred,Arity).\''),
  write(' for help on a certain predicate'), nl,
  !.

helpMePrintLine([]).
helpMePrintLine([[Pred, Arity, Meaning]|X]) :-
  format('  ~w~24|/~w~28|~w~n',[Pred,Arity,Meaning]),
%  write('  '), write(Pred), write('/'), write(Arity), write(': '),
%  write(Meaning), nl,
  helpMePrintLine(X),
  !.

helpMeWriteParamList([[Type,Name,Meaning]]) :-
  write('    '), write(Type), write(Name), write(':\t'), write(Meaning), nl, !.
helpMeWriteParamList([[Type,Name,Meaning]|X]) :-
  write('    '), write(Type), write(Name), write(':\t'),
  write(Meaning), write(','), nl,
  helpMeWriteParamList(X), !.
helpMeWriteParamList(_) :-
  write('ERROR: Wrong parameter format in helpLine/4!'), nl, !.

/*
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
   dynamic(optimizerOptionInfo/6).

/*
---- optimizerOptionInfo(+Option,+SuperOption,-NonStandard,-Meaning,-GoalOn,-GoalOff)
----
Provides information on optimizer options. ~Option~ is the option in question,
~Meaning~ is a description of that option. When ~Option~ get activated, ~GoalOn~
should be called, is it is deactivated, ~GoalOff~ is called.

~SuperOption~ is the name of the option that ~Option~ is a suboption of. If ~none~,
~Option~ is a top-level option.

~NonStandard = yes~ means this option somehow alters the optimization process.
~NonStandard = no~ means, the option does not modify the optimization process.

*/

optimizerOptionInfo(standard, none, no,
                    'Adopt options for standard optimization process.',
                    delAllNonstandardOptions,
                    true
                   ).

optimizerOptionInfo(useCounters, none, no,
                    'Insert counters into the computed plan.',
                    true,
                    true
                   ).

optimizerOptionInfo(noHashjoin, none, yes,
                    'Disables hashjoin.',
                    true,
                    true
                   ).

optimizerOptionInfo(noSymmjoin, none, yes,
                    'Disables symmjoin.',
                    true,
                    true
                   ).

optimizerOptionInfo(noIndex, none, yes,
                    'Disables the utilization of indexes.',
                    true,
                    true
                   ).

optimizerOptionInfo(useRandomSMJ, none, yes,
                    'Uses sortmergejoin_r2 instead of sortmergejoin.',
                    true,
                    true
                   ).


optimizerOptionInfo(allOff, none, no,
                    'Turn off really ALL options.',
                    delAllOptions,
                    true
                   ).

optimizerOptionInfo(nawracosts, none, yes,
                    'Use cost functions as implemented by A. Nawra.',
                    ( delOption(improvedcosts)
                    ),
                    true
                   ).

optimizerOptionInfo(earlyproject, none, yes,
                    'Project before sort and groupby',
                    true,
                    true
                   ).

optimizerOptionInfo(improvedcosts, none, yes,
                    'Use improved cost functions.',
                    ( delOption(nawracosts),
                      setOption(determinePredSig),
                      loadFiles(improvedcosts)
                    ),
                    true
                   ).


/*
----
optimizerOptionInfo(uniformSpeed, none, yes,
                    'Set machine speed factor to constant 1.0.',
                    true, true).
optimizerOptionInfo(costsConjuctive, none, yes,
           'Apply costs only to operators directly considered by Dijkstra',
           true, true).
----

*/
optimizerOptionInfo(determinePredSig, none, yes,
                    'Send queries to investigate predicate argument types.',
                    true, (delOption(improvedcosts))).

optimizerOptionInfo(use_matchingOperators, determinePredSig, yes,
                    'Use \'matchingOperators\' instead of \'getTypeNL\' to determine types.',
                    (setOption(determinePredSig)), true).

optimizerOptionInfo(immediatePlan, none, yes,
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

optimizerOptionInfo(intOrders(on), immediatePlan, yes,
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
optimizerOptionInfo(intOrders(path), immediatePlan, yes,
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
optimizerOptionInfo(intOrders(quick), immediatePlan, yes,
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
optimizerOptionInfo(intOrders(test), immediatePlan, yes,
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


%LargeQueries start
optimizerOptionInfo(largeQueries(qgd), none, yes,
                    'Large predicate set optimization (query graph decomposition)',
                    (delOption(largeQueries(aco)), 
                     delOption(largeQueries(qgdm)), 
                     loadFiles(largequeries),
                     initLargeQueries), true).

optimizerOptionInfo(largeQueries(qgdm), none, yes,
                    'Large predicate set optimization (query graph decomposition and materialization)',
                    ( delOption(largeQueries(aco)), 
                      delOption(largeQueries(qgd)), 
                      loadFiles(largequeries),
                      initLargeQueries), true).

optimizerOptionInfo(largeQueries(aco), none, yes,
                    'Large predicate set optimization (ascending cost order)',
                    (delOption(largeQueries(qgd)), 
                     delOption(largeQueries(qgdm)),
                     loadFiles(largequeries),
                     initLargeQueries), true).


%LargeQueries end


optimizerOptionInfo(pathTiming, none, no,
                    'Prompt time used to find a best path.',
                    true, true).
optimizerOptionInfo(dynamicSample, none, yes,
                    'Use dynamic instead of static (saved) samples.', true,
                    ( notIsDatabaseOpen ; ensureSamplesExist ) ).
optimizerOptionInfo(autoSamples, none, no,
                    'Automatically determine sample sizes.',
                    true, true).
optimizerOptionInfo(eagerObjectCreation, none, yes,
                'Create all samples and small objects at \'open databases\'.',
                    true, true).
optimizerOptionInfo(rewriteMacros, none, no,
                    'Allow for macros in queries.',
                    true, true).
optimizerOptionInfo(rewriteInference, none, yes,
                    'Add inferred predicates to where clause.',
                    true, true).
optimizerOptionInfo(rtreeIndexRules, rewriteInference, yes,
                    'Infer predicates to exploit R-tree indices.',
                      (   setOption(rewriteInference),
                          (
                            not(optimizerOption(entropy))
                          ; notIsDatabaseOpen
%                          ; ensureSmallObjectsExist
                          )
                      ),
                      true).
optimizerOptionInfo(rewriteCSE, none, yes,
                    'Extend with attributes for CSE values.',
                    true, delOption(rewriteRemove)).
optimizerOptionInfo(rewriteCSEall, rewriteCSE, no,
                    'Extend with attributes for _ALL_ CSEs.',
                    true, true).
optimizerOptionInfo(rewriteRemove, rewriteCSE, no,
                    'Remove attributes as early as possible.',
                    setOption(rewriteCSE), true).

optimizerOptionInfo(debug, none, no,
                    'Execute debugging code. Also use \'toggleDebug.\'.',
                    showDebugLevel,true).
optimizerOptionInfo(autosave, none, no,
                    'Autosave option settings on \'halt.\'.',
                    true, true).
optimizerOptionInfo(noprogress, none, no,
                    'Do not send predicate data for progress estimation.',
                    true, true).
optimizerOptionInfo(subqueries, none, yes,
                   'Process subqueries.',
                   true,
                   (delOption(subqueryUnnesting),
                   delOption(nestedRelations)) % NVK ADDED
                  ).
optimizerOptionInfo(subqueryUnnesting, subqueries, yes,
                   'Apply unnesting algorithms to subqueries.',
                   setOption(subqueries),
                   true
                  ).

% Section:Start:optimizerOptionInfo_6_e
% Section:End:optimizerOptionInfo_6_e

:- load_storefiles(calloptimizer_sec). % include more options

/*
---- showOptions
     setOtion(+Option)
     delOption(+Option)
----
Show information of possible optimizer options and current option settings, select and unselect
optimizer options.

*/

:- assert(helpLine(showOptions,0,[],'List available options.')).
:- assert(helpLine(setOption, 1, [
        [+,'OptionName','Name of the option to select.']],
        'Set a given option.')).
:- assert(helpLine(delOption, 1, [
        [+,'OptionName','Name of the option to deselect.']],
        'Unset a given option.')).

showOptions :-
  findall(X,optimizerOptionInfo(X,none,_,_,_,_),Options),
  write('\n\nOptimizer options (and sub-options):\n'),
  showOption(Options),
  write('\nType \'loadOptions.\' to load the saved option configuration.\n'),
  write('Type \'saveOptions.\' to save current option configuration to disk.\n'),
  write('Type \'defaultOptions.\' to restore the default options.\n'),
  write('Type \'setOption(X).\' to select option X.\n'),
  write('Type \'delOption(X).\' to unselect option X.\n'),
  write('Type \'showOptions.\' to view this option list.\n\n'),
  write('Type \'helpMe.\' to get an overview on user level predicates.\n\n').

showOption([]).
showOption([Option|Y]) :-
  optimizerOptionInfo(Option,none,_,Text,_,_),
  write(' ['),
  ( optimizerOption(Option)
      -> write('x')
       ; write(' ')
  ),
  write(']    '),
  format('~w:~30|~w~n',[Option, Text]),
  showSubOptions(Option),
  showOption(Y), !.

showSubOption(_,[]).
showSubOption(Super,[Option|Y]) :-
  optimizerOptionInfo(Option,Super,_,Text,_,_),
  write('    ('),
  ( optimizerOption(Option)
      -> write('x')
       ; write(' ')
  ),
  write(') '),
  format('~w:~30|~w~n',[Option, Text]),
  showSubOption(Super,Y), !.

showSubOptions(Super) :-
  findall(X,optimizerOptionInfo(X,Super,_,_,_,_),SubOptions),
  showSubOption(Super, SubOptions), !.

setOption(X) :-
  nonvar(X),
  optimizerOptionInfo(X,_,NonStandard,Text,GoalOn,_),
  retractall(optimizerOption(X)),
  assert(optimizerOption(X)),
  call(GoalOn),
  write('Switched ON option: \''), write(X), write('\' - '),
  write(Text), write('\n'),
  ( X \= allOff
       -> retractall(optimizerOption(allOff))
        ; true
  ),
  ( (X \= standard, NonStandard = yes)
       -> retractall(optimizerOption(standard))
        ; true
  ),
  !.

setOption(X) :-
  write('Unknown option \''), write(X), write('\'.\n'), fail, !.

delOption(X) :-
  nonvar(X),
  optimizerOptionInfo(X,_,_,Text,_,GoalOff),
  ( optimizerOption(X)
    -> ( retractall(optimizerOption(X)),
         call(GoalOff),
         write('Switched OFF option: \''), write(X), write('\' - '),
         write(Text), write('\n')
       )
    ;  true
  ),
  ( not( ( optimizerOption(Z),
           optimizerOptionInfo(Z,_,yes,_,_,_)
         ))
    -> ( retractall(optimizerOption(standard)),
         assert(optimizerOption(standard))
       )
     ; true
  ),
  ( not((optimizerOption(Z), Z \= standard))
    -> ( retractall(optimizerOption(allOff)),
         assert(optimizerOption(allOff))
       )
     ; true
  ), !.

delOneOption(X) :-
  write('Unknown option \''), write(X), write('\'.\n'),
  showOptions,
  fail, !.

delOneOption :-
  optimizerOptionInfo(X,_,_,_,_,_),
  X \= none,
  delOption(X),
  fail.

delAllOptions :-
  not(delOneOption),
  write('\nSwitched off all options.\n').

delOneNonstandardOption :-
  optimizerOptionInfo(X,_,yes,_,_,_),
  X \= standard,
  delOption(X),
  fail.

delAllNonstandardOptions :-
  not(delOneNonstandardOption),
  write('\nSwitched off all nonstandard-options. '),
  write('Optimizer now works in standard mode.\n').


/*
3 Code for certain optimizer options

Normally, such code should be placed with the module providing the features,
but here, you will find some things of general interest, like settings for
integrated debugging features.

*/

/*
3.1 Setting Debugging Options

*/

:- assert(helpLine(showDebugLevel,0,[],'Show active debug levels.')).
:- assert(helpLine(toggleDebug,0,[],'Switch debugging mode on/off.')).
:- assert(helpLine(debugLevel,1,
  [[+,'Level','The name of the debug level to add to the debug list']],
  'Add a given level to the debug list.')).
:- assert(helpLine(nodebugLevel,1,
  [[+,'Level','The name of the debug level to remove from the debug list']],
  'Remove a given level from the debug list.')).

ppCostFactor(0) :-
  optimizerOption(costConjunctive), !.

ppCostFactor(1) :- !.


showDebugLevel :-
  findall(X,optActiveDebugLevel(X),AList),
  findall(X,optDebugLevel(X),DList),
  write('\tAvailable Debug levels: '),
  write(DList), nl,
  write('\tActive Debug levels: '),
  write(AList), nl.

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
  write('\tLevel \'none\' will deactivate all qualified debugging.\n'),
  !.

% handle more than a single mode
debugLevel([]).
debugLevel([Mode|Modes]) :-
  ( debugLevel(Mode) ; true ),
  debugLevel(Modes).

debugLevel(none) :-
  retractall(optActiveDebugLevel(_)),
  write_list(['Deactivated all qualified debug levels.\n\n']), !.

debugLevel(Mode) :-
  ( Mode = all ; optDebugLevel(Mode) ),
  ( optActiveDebugLevel(Mode)
      -> write_list(['Debug Level \'',Mode,'\' is already active.\n'])
      ;  assert(optActiveDebugLevel(Mode))
  ),
  showDebugLevel,
  nl, !.


debugLevel(Mode) :-
  Mode \= all ,
  not(optDebugLevel(Mode)),
  findall(X,optDebugLevel(X), L),
  findall(X,optActiveDebugLevel(X),A),
  write_list(['DebugLevel \'',Mode,'\' is unknown.\n\n',
              'Available debugLevels are:\n',L,'.\n\n',
              'Active debugLevels are:\n',A,'.\n',
              'DebugLevel \'none\' deactivates all qualified debugging.\n\n']),
  !.

% handle more than a single mode
nodebugLevel([]).
nodebugLevel([Mode|Modes]) :-
  ( nodebugLevel(Mode) ; true ),
  nodebugLevel(Modes).

nodebugLevel(Mode) :-
  ( Mode = all ; optDebugLevel(Mode) ),
  ( optActiveDebugLevel(Mode)
      -> retractall(optActiveDebugLevel(Mode))
      ;  write_list(['Debug Level \'',Mode,'\' is not active.\n'])
  ),
  showDebugLevel,
  nl, !.

nodebugLevel(Mode) :-
  Mode \= all,
  not(optDebugLevel(Mode)),
  findall(X,optDebugLevel(X), L),
  findall(X,optActiveDebugLevel(X),A),
  write_list(['DebugLevel \'',Mode,'\' is unknown.\n\n',
              'Available debugLevels are:\n',L,'.\n\n',
              'Active debugLevels are:\n',A,'.\n']),
  !.

/*
3.2 Registering Debug Levels

Each debug level is required to be registered by a fact ~optDebugLevel/1~ here:

*/

optDebugLevel(bestPlan).         % Prompting best plan generated
optDebugLevel(costFunctions).    % Tracing some cost function calls
optDebugLevel(dbhandling).       % Processing database object metadata
optDebugLevel(gettypetree).      % Tracing calls to getTypeTree
optDebugLevel(immPath).          % Monitoring immediate path generation
optDebugLevel(insertExtend).     % Tracing insertExtend during plan rewriting
optDebugLevel(intOrders).        % Monitoring interesting orders extension
optDebugLevel(pog).              % Printing info on pog
optDebugLevel(rewrite).          % General info on rewriting
optDebugLevel(rewriteCSE).       % Tracing CSE rewriting
optDebugLevel(rewriteMacros).    % Macro expansion during query rewriting
optDebugLevel(rewritePlan).      % Plan rewriting
optDebugLevel(subqueryUnnesting).
optDebugLevel(subqueryDebug).
optDebugLevel(temprels).
optDebugLevel(selectivity).      % Details on selectivity queries
optDebugLevel(translation).      % Details on translation rule matching

/*
3.3 Switching between optimization module options

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
    [relations],
    [testExamples],
% Section:Start:loadFiles_1_i
% Section:End:loadFiles_1_i
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


% Optional files for the correlations options
loadFiles(correlations) :-
  ( not(loadedModule(correlations)),
    ['./Correlations/correlations'],
    assert(loadedModule(correlations))
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

%LargeQueries start:
% Optional files for the largeQueries options
loadFiles(largequeries) :-
  ( not(loadedModule(largequeries)),
    ['./LargeQueries/largequeries'],
    assert(loadedModule(largequeries))
  )
  ; true.
%LargeQueries end


% Optional files for usage of Xris' cost functions
loadFiles(improvedcosts) :-
  ( not(loadedModule(improvedcosts)),
    [improvedcosts],
    assert(loadedModule(improvedcosts))
  )
  ; true.

% Section:Start:loadFiles_1_e
% Section:End:loadFiles_1_e

/*
4 Some Auxiliary predicates for Debugging

*/

/*
Print debugging information

Predicate ~dm/1~ can be used as ~write~/1. Output is printed when optimizer option
debug is defined (see file operators.pl).

Predicate ~dm(mode,message)~ writes ~message~ if ~optActiveDebugLevel(mode)~
or ~optActiveDebugLevel(all)~ is defined.

*/

write_list([]).
write_list([X|Rest]) :-
  write(X),
  write_list(Rest).

dm(X) :-
  ( optimizerOption(debug)
    -> write_list(X)
    ;  true
  ), !.

dm(_) :- !.

dm(Level, X) :-
  optimizerOption(debug), !,
  ( ( optActiveDebugLevel(Level) ; optActiveDebugLevel(all) )
    -> write_list(X)
    ;  ( optDebugLevel(Level)
           -> true
           ;  ( write_list(['\nERROR:\tdebugLevel \'',Level,'\' is unknown!\n',
                    '\tplease register it with a fact optDebugLevel/1 in file',
                    '\n\tcalloptimizer.pl.\n\n']),
                write_list(X)
              )
       )
  ), !.

dm(_,_) :- !.

/*
Execute debugging code

dc works like dm, but calls a goal instead of simply printing messages:

*/

dc(Command) :-
  optimizerOption(debug),
  call(Command),
  !.
dc(_) :- !.

dc(Level, Command) :-
  ( optimizerOption(debug)
    -> ( ( optActiveDebugLevel(Level) ; optActiveDebugLevel(all) )
           -> call(Command)
           ;  true
       )
    ;  ( optDebugLevel(Level)
         -> true
         ;  write_list(['\nERROR:\tdebugLevel \'',Level,'\' is unknown!\n',
                    '\tplease register it with a fact optDebugLevel/1 in file',
                    '\tcalloptimizer.pl.\n\n'])
       )
  ), !.
dc(_, _) :- !.


/*
5 Optimizer Startup Procedure

*/

/*
5.1 Loading and Initializing Modules

*/

% Startup procedure for the optimizer
:- [opsyntax].
:- loadFiles(standard).        % load the files for the standard optimizer

:- assert( highNode(0) ),
   assert( boundarySize(0) ),
   assert( boundaryMaxSize(0) ).

/*
5.2 Setting Startup Options

The option configuration is stored in a file ~config\_optimizer.pl~ between two sessions. It will be loaded and restored on subsequent sessions. If this file does not exist, default options are used.

~defaultOptions~ restores the built-in default option settings.

~loadOptions~ restores the option config from disk.

~saveOptions~ saves the current options to disk.

If ~optimizerOption(autosave)~ is defined, current option settings will be saved
to disk automatically on system halt.

*/

:- assert(helpLine(defaultOptions,0,[],'Choose the default option setting.')).
:- assert(helpLine(loadOptions,0,[],'Restore option settings to saved ones.')).
:- assert(helpLine(saveOptions,0,[],'Save option settings to disk.')).

defaultOptions :-
  setOption(standard),
  delOption(useCounters),
  delOption(noHashjoin),
  delOption(noSymmjoin),
  delOption(noIndex),
  delOption(useRandomSMJ),
  setOption(debug),
  debugLevel(selectivity),
  setOption(autoSamples),
% Section:Start:defaultOptions_0_i
% Section:End:defaultOptions_0_i
  setOption(autosave).


loadOptions :-
  delAllOptions,
  retractall(optActiveDebugLevel(_)),
  open('config_optimizer.pl',read,S), %force an exception if 
  close(S),                           %file does not exist
  consult('config_optimizer.pl').

loadOptions :-
  defaultOptions,
  write('Use default options\n').

saveSingleOption(Stream) :-
  optimizerOption(X),
  write(Stream, ':- '),
  write(Stream, setOption(X)),
  write(Stream, '.\n').

saveSingleDebugLevel(Stream) :-
  optActiveDebugLevel(X),
  write(Stream, ':- '),
  write(Stream, debugLevel(X)),
  write(Stream, '.\n').

saveOptions :-
  open('config_optimizer.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, saveSingleOption(FD), _),
  findall(_, saveSingleDebugLevel(FD), _),
  close(FD).

initializeOptions :-
  catch(
    ( loadOptions,!,
      write('Loaded options\n')
    ),
    _, % catch all exceptions
    ( defaultOptions,
      saveOptions, % save a blanc 'config_optimizer.pl'
      write('Saved options\n')
    )
  ).

% NVK ADDED: Initialize the extensions.
:- ['./NestedRelations/init.pl']. 
:- ['./MemoryAllocation/init.pl']. 
% NVK ADDED END


:- at_halt((optimizerOption(autosave), saveOptions)). % automatically safe option configuration on exit
:- initializeOptions.


/*
5.3 Print Additional Information

*/

:- showOptions.
%:-   nl, write('NOTE: SWI-Prolog Version 5.4.7 shows '),
%     write('no prompt in the MSYS console!'), nl,
%     write('      A workaround is to type in the predicate "getprompt."'), nl, nl.
:- ( current_prolog_flag(windows,true), % query getprompt on windows systems
     getprompt
   ) ; true.



/*
6 Shortcuts and Aliases

*/

:- assert(helpLine(quit,0,[],'Quit the optimizer.')).

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

/*
End of file ~calloptimizer.pl~

*/
