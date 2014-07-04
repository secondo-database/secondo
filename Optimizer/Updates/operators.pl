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

1 Constants for Operators

[File ~operators.pl~]

1.1 Constants for Certain Operators

The constants for the operators have been determined by experiments. For
those experiments, time measurement code was added to some relational operators
(hashjoin, product, and sortmergejoin). This permitted determining how much
CPU time is needed for sorting tuples, how much CPU time is needed for hashing etc.
If one wants to experiment by oneself, the time meeasurement support in the
relational algebra can be easily switched on by uncommenting a line in the
relational algebra (and thus defining a symbol ~MEASURE\_OPERATORS~).

The experiments considered only a rather small set of queries. Although the
constants below are quite accurate e.g. for examples 14 to
21 in the optimizer, they may be inappropriate for other queries.

*/

feedTC(0.4).
consumeTC(1.0).
filterTC(1.68).
productTC(1.26, 0.4).
leftrangeTC(10).
loopjoinTC(1.0).
exactmatchTC(10.0).
hashjoinTC(1.5, 0.65).
sortmergejoinTC(0.3, 0.73). % first also used for sort(), second for mergejoin()
symmjoinTC(1.4, 0.7).
extendTC(1.5).
removeTC(0.6).
projectTC(0.1).
renameTC(0.1).
windowintersectsTC(0.1).
spatialjoinTC(20.0, 0.7).
distancescanTC(0.25).
ksmallestTC(0.05, 0.23).
createbtreeTC(0.12).

% sorttidTC and rdupTC are estimated without experiment
% They still need to be estimated
sorttidTC(0.1).  % used for sorting tuple ids after a windowIntersectsS
rdupTC(0.1).

/*
1.2 General Constants

The predicate

---- secOptConstant(+ConstantName, -Value)
----

is used to define general constants to be used by the optimizer.

All constants can be printed using predicate

---- showOptConstants/0
----

*/

:-assert(helpLine(showOptConstants,0,[],
         'Display settings of several constants.')).

showOptConstant :-
  secOptConstant(X, Y),
  write('  secOptConstant( '),write(X), write(',\t'), write(Y), write(')'), nl.

showOptConstants :-
  nl, write('Overview on all optimizer constants secOptConstant/2:\n'),
  findall(_, showOptConstant, _).

% Section:Start:secOptConstant_2_b
% Section:End:secOptConstant_2_b

/*
1.2.1 Constants for Buffer Size

The value of the buffer size in bytes. This is the amount of main memory, operators are allowed to use to keep their internal data (e.g. hashtables, tuple buffers etc.). This should be equal to the setting for ~MaxMemPerOperator~
in file ~SecondoConfig.ini~.

*/

secOptConstant(bufferSize, 16384000). % 15.625 MB ~ 16 MB

/*
1.2.2 Constants for Sampling

The maximum sample size in bytes. The cardinality of samples will be reduced, such that it hopefully does not get larger than this value.

Minimum and maximun cardinalities for selection and join relation samples

Standard scaling factor for samples.

*/

secOptConstant(sampleScalingFactor, 0.00001).  % scaling factor for samples

secOptConstant(sampleSelMaxDiskSize, 2048).    % maximum KB size for samples
secOptConstant(sampleSelMinCard, 100).         % minimum cardinality for samples
secOptConstant(sampleSelMaxCard, 2000).        % maximum cardinality for samples

secOptConstant(sampleJoinMaxDiskSize, 2048).   % maximum KB size for samples
secOptConstant(sampleJoinMinCard, 50).         % minimum cardinality for samples
secOptConstant(sampleJoinMaxCard, 500).        % maximum cardinality for samples

% Section:Start:secOptConstant_2_e
% Section:End:secOptConstant_2_e

/*

2 Operators

In this section, signatures and property descriptions for operators are defined
and made available.


2.1 Signatures and Properties of Operators

The result data type of an operator call may either be queried from the Secondo
kernel, or it may be infered from using one of the following rules, if the type
of the arguments is known.

Each implemented signature for an operator should have a rule here. This helps
much to avoid the overhead of sending data type queries to Secondo to infer the
type of expressions

---- opSignature(+Operator, ?Algebra, +ArgTypeList, -Resulttype, -Flags).
----

Basically, these clauses do the same as the ~type mapping fubctions~ within the
Secondo kernel: They are provided with a list of argument (~ArgTypeList~) types
and return the type ~Resulttype~ of the ~Operator~, when called with that argument
types. Addiditionally, we register information on which algebra defines the operator
and on whether the operator has certain properties, resp. belongs to certain classes
of operators.

While for some operators according information may be queried from the database kernel,
we strongly recommend to provide descriptions for each operator available!

~ArgTypeList~ is a PROLOG list of terms representing the valid Secondo type
expressions for all arguments. Simple types are noted by their DC-name.

Valid Type Expressions are given in Secondo nested list syntax, but using
Prolog-style lists with suqared brackets and commas separating elements.
To avoid confusion with Prolog-variables, everything is written in lower case
letters (DC-spelling).

Lists are noted

---- [E1,E2,...,En]
----

Tuples are noted

---- [tuple, [[A1,T1],[A2,T2],...,[An,Tn]]]
----

where ~Ai~ is the DC-name of an attribute and ~Ti~ its DC-type.

Streams are noted

---- [stream,T]
----

where ~T~ is the stream's element type.

Relations are noted

---- [rel, <tuple>]
----

Where ~Ei~ is the ith element.

Mappings are noted

---- [map, T1, T2, ..., Tn, T]
----
Where ~Ti~ is the DC-type of the mapping's i-th argument and ~T~ is the DC-type
of the mapping's result.

~Lists~ are noted

---- [E1, E2, E3]
----

where ~Ei~ are the elements of the parameter list
(which may be nested list types, again).

~Functionlist~ are noted

---- [(N1,M1),(N2,M2),...,(Nn,Mn)]
----

where ~Ni~ is the name for the i-th component and ~Mi~ is the signature of the
i-th mapping.

~Resulttype~ is a term representing a valid Secondo type expression.

~Flags~ is a list of flags indicating that the operator has specific properties:

   * comm: ~Op~ is binary and commutative. Used to normalize predicates

   * ass: ~Op~ is binary and associative. Could be used for algebraic simplification.

   * trans: ~Op~ is binary and transitive. Could be used for algebraic simplification.

   * idem: ~Op~ is unary and idempotent. Could be used for algebraic simplification.

   * bbox(D): ~Op~ implements a predicate (~Resulttype~ is ~bool~) using a filter and refinement approach with D-dimensional MBRs

   * block: ~Op~ is a blocking stream operator

   * aggr: ~Op~ somehow aggregates a stream to a scalar object. Used to identify aggregation expressions.

   * sidefx: ~Op~ may have side effects (modifies state of pseudo random number generators, creates output on terminal, creates/modifies the filesystem).

   * itract: ~Op~ is interactive, e.g. waiting for external events, like user interaction, network events, etc.

   * exp: ~Op~ is expensive. Only operations with a result from kind DATA may be marked as being expensive! Used during CSE rewriting.

   * join: ~Op~ is a join operator (joins two streams). Used during CSE rewriting

   * typemapop: ~Op~ is a type mapping operator.

   * (list may be extended as required)

% Section:Start:opSignature_5_s
% Section:End:opSignature_5_s

2.7.1 StandardAlgebra

*/
opSignature((+), standard, [int,int],int,[comm,ass]).
opSignature((+), standard, [int,real],real,[comm,ass]).
opSignature((+), standard, [real,int],real,[comm,ass]).
opSignature((+), standard, [real,real],real,[comm,ass]).

opSignature((-), standard, [int,int],int,[comm]).
opSignature((-), standard, [int,real],real,[comm]).
opSignature((-), standard, [real,int],real,[comm]).
opSignature((-), standard, [real,real],real,[comm]).

% opSignature((*), standard, [int,int],int,[comm,ass]).
% opSignature((*), standard, [int,real],real,[comm,ass]).
% opSignature((*), standard, [real,int],real,[comm,ass]).
% opSignature((*), standard, [real,real],real,[comm,ass]).

%% --- BEGIN alternate definitions to test the CSE extension
opSignature((*), standard, [int,int],int,[comm,ass,exp]).
opSignature((*), standard, [int,real],real,[comm,ass,exp]).
opSignature((*), standard, [real,int],real,[comm,ass,exp]).
opSignature((*), standard, [real,real],real,[comm,ass,exp]).
%% --- END alternate definitions to test the CSE extension

opSignature((/), standard, [int,int],real,[]).
opSignature((/), standard, [int,real],real,[]).
opSignature((/), standard, [real,int],real,[]).
opSignature((/), standard, [real,real],real,[]).

opSignature(mod, standard, [int,int],int,[]).
opSignature(div, standard, [int,int],int,[]).
opSignature(sqrt, standard, [real],real,[]).
opSignature(randint, standard, [int], int,[sidefx]).
opSignature(randseed, standard, [int],bool,[sidefx]).
opSignature(randmax, standard, [],int,[]).
opSignature(seqinit, standard, [int],bool,[sidefx]).
opSignature(seqnext, standard, [],int,[sidefx]).
opSignature(log, standard, [int],int,[]).

opSignature((<), standard, [int,int],bool,[]).
opSignature((<), standard, [int,real],bool,[]).
opSignature((<), standard, [real,int],bool,[]).
opSignature((<), standard, [real,real],bool,[]).
opSignature((<), standard, [bool,bool],bool,[]).
opSignature((<), standard, [string,string],bool,[]).

opSignature((>), standard, [int,int],bool,[]).
opSignature((>), standard, [int,real],bool,[]).
opSignature((>), standard, [real,int],bool,[]).
opSignature((>), standard, [real,real],bool,[]).
opSignature((>), standard, [bool,bool],bool,[]).
opSignature((>), standard, [string,string],bool,[]).

opSignature((<=), standard, [int,int],bool,[]).
opSignature((<=), standard, [int,real],bool,[]).
opSignature((<=), standard, [real,int],bool,[]).
opSignature((<=), standard, [real,real],bool,[]).
opSignature((<=), standard, [bool,bool],bool,[]).
opSignature((<=), standard, [string,string],bool,[]).

opSignature((>=), standard, [int,int],bool,[]).
opSignature((>=), standard, [int,real],bool,[]).
opSignature((>=), standard, [real,int],bool,[]).
opSignature((>=), standard, [real,real],bool,[]).
opSignature((>=), standard, [bool,bool],bool,[]).
opSignature((>=), standard, [string,string],bool,[]).

opSignature((=), standard, [int,int],bool,[comm]).
opSignature((=), standard, [int,real],bool,[comm]).
opSignature((=), standard, [real,int],bool,[comm]).
opSignature((=), standard, [real,real],bool,[comm]).
opSignature((=), standard, [bool,bool],bool,[comm]).
opSignature((=), standard, [string,string],bool,[comm]).

opSignature((#), standard, [int,int],bool,[comm]).
opSignature((#), standard, [int,real],bool,[comm]).
opSignature((#), standard, [real,int],bool,[comm]).
opSignature((#), standard, [real,real],bool,[comm]).
opSignature((#), standard, [bool,bool],bool,[comm]).
opSignature((#), standard, [string,string],bool,[comm]).

opSignature(starts,   standard, [string,string],bool,[]).
opSignature(contains, standard, [string,string],bool,[]).
opSignature(substr,   standard, [string,int,int],string,[]).
opSignature(not,      standard, [bool],bool,[idem]).
opSignature(and,      standard, [bool,bool],bool,[]).
opSignature(or,       standard, [bool,bool],bool,[]).
opSignature(sand,     standard, [bool,bool],bool,[comm,ass]).
opSignature(sor,      standard, [bool,bool],bool,[comm,ass]).

opSignature(isempty, standard, [int],bool,[]).
opSignature(isempty, standard, [real],bool,[]).
opSignature(isempty, standard, [bool],bool,[]).
opSignature(isempty, standard, [string],bool,[]).

opSignature(++, standard, [string],string,[]).

opSignature(intersection, standard, [int,int],int,[comm,ass]).
opSignature(intersection, standard, [real,real],real,[comm,ass]).
opSignature(intersection, standard, [bool,bool],bool,[comm,ass]).
opSignature(intersection, standard, [string,string],string,[comm,ass]).

opSignature((minus), standard, [int,int],int,[]).
opSignature((minus), standard, [real,real],real,[]).
opSignature((minus), standard, [bool,bool],bool,[]).
opSignature((minus), standard, [string,string],string,[]).

opSignature(relcount, standard, [string],int,[]).
opSignature(relcount2, standard, [string],int,[]).
opSignature(keywords, standard, [string],[stream,string],[]).
opSignature(ifthenelse, standard, [bool,T,T],T,[]) :- isData(T).
opSignature(ifthenelse2, standard, [bool,T,T],T,[]) :- isData(T).

opSignature(between, standard, [T,T,T],bool,[]) :- isData(T).
opSignature(ldistance, standard, [string,string],int,[]).
opSignature(hashvalue, standard, [T,int],int,[]) :- isData(T).
opSignature(round, standard, [real,int],real,[]).
opSignature(int2real, standard, [int],real,[]).
opSignature(real2int, standard, [real],int,[]).
opSignature(int2bool, standard, [int],bool,[]).
opSignature(bool2int, standard, [bool],int,[]).
opSignature(ceil, standard, [real],real,[]).
opSignature(floor, standard, [real],real,[]).
opSignature(char, standard, [int],string,[]).

opSignature(num2string, standard, [int],string,[]).
opSignature(num2string, standard, [real],string,[]).
opSignature(isdefined, standard, [T],data,[]) :- isData(T).
opSignature(compare, standard, [T,T],int,[]) :- isData(T).
opSignature(getMinVal, standard, TL,T,[]) :-
  isData(T), is_list(TL), my_list_to_set(TL, [T]).
opSignature(getMaxVal, standard, TL,T,[]) :-
  isData(T), is_list(TL), my_list_to_set(TL, [T]).
opSignature(setoption, standard, [string,int],bool,[sidefx]).

opSignature(abs, standard, [int],int,[]).
opSignature(abs, standard, [real],real,[]).

opSignature((,), standard, [T,T],[T,[set,T]],[]). % op provided by Poneleit

/*
2.7.2 FTextAlgebra

*/
opSignature((contains), ftext, [text,string],bool,[]).
opSignature((contains), ftext, [text,text],bool,[]).
opSignature(length, ftext, [text],int,[]).
opSignature(keywords, ftext, [text],[stream,string],[]).
opSignature(sentences, ftext, [text],[stream,text],[]).
opSignature(dice, ftext, [int,text,text],real,[xexp]).
opSignature(getcatalog, ftext, [],
      [stream,[tuple,[[objectname,string],[type,string],[typeexpr,text]]]],[]).
opSignature(substr, ftext, [text,int,int],string,[exp]).
opSignature(subtext, ftext, [text,int,int],text,[exp]).

opSignature(find, ftext, [text,text],[stream,int],[]).
opSignature(find, ftext, [text,string],[stream,int],[]).
opSignature(find, ftext, [string,text],[stream,int],[]).
opSignature(find, ftext, [string,string],[stream,int],[]).

opSignature(isempty, ftext, [text],bool,[]).
opSignature((+), ftext, [text,text],text,[comm,ass]).
opSignature((+), ftext, [text,string],text,[comm,ass]).
opSignature((+), ftext, [string,text],text,[comm,ass]).

opSignature((<), ftext, [text,text],bool,[]).
opSignature((<), ftext, [text,string],bool,[]).
opSignature((<), ftext, [string,text],bool,[]).

opSignature((<=), ftext, [text,text],bool,[]).
opSignature((<=), ftext, [text,string],bool,[]).
opSignature((<=), ftext, [string,text],bool,[]).

opSignature((=), ftext, [text,text],bool,[comm]).
opSignature((=), ftext, [text,string],bool,[comm]).
opSignature((=), ftext, [string,text],bool,[comm]).

opSignature((>=), ftext, [text,text],bool,[]).
opSignature((>=), ftext, [text,string],bool,[]).
opSignature((>=), ftext, [string,text],bool,[]).

opSignature((>), ftext, [text,text],bool,[]).
opSignature((>), ftext, [text,string],bool,[]).
opSignature((>), ftext, [string,text],bool,[]).

opSignature((#), ftext, [text,text],bool,[comm]).
opSignature((#), ftext, [text,string],bool,[comm]).
opSignature((#), ftext, [string,text],bool,[comm]).

opSignature(evaluate, ftext, [text],[stream,[tuple,[[cmdstr,text],
        [success,bool],[evaluable,bool],[defined,bool],[isfunction,bool],
        [resultyype,text],[result,text],[errormessage,text],
        [elapsedtimereal,real],[elapsedtimecpu,real]]]],[sidefx]).
opSignature(evaluate, ftext, [text,bool],[stream,[tuple,[[cmdstr,text],
        [success,bool],[evaluable,bool],[defined,bool],[isfunction,bool],
        [resulttype,text],[result,text],[errormessage,text],
        [elapsedtimereal,real],[elapsedtimecpu,real]]]],[sidefx]).

opSignature(replace, ftext, [text,text,text],text,[exp]).
opSignature(replace, ftext, [text,text,string],text,[exp]).
opSignature(replace, ftext, [text,string,text],text,[exp]).
opSignature(replace, ftext, [text,string,string],text,[exp]).
opSignature(replace, ftext, [string,text,text],text,[exp]).
opSignature(replace, ftext, [string,text,string],text,[exp]).
opSignature(replace, ftext, [string,string,text],text,[exp]).
opSignature(replace, ftext, [string,string,string],text,[exp]).

opSignature(replace, ftext, [text,int,int,text],text,[exp]).
opSignature(replace, ftext, [text,int,int,string],text,[exp]).
opSignature(replace, ftext, [string,int,int,text],text,[exp]).
opSignature(replace, ftext, [string,int,int,string],text,[exp]).

opSignature(toupper, ftext, [text],text,[]).
opSignature(tolower, ftext, [text],text,[]).
opSignature(tostring, ftext, [text],string,[]).
opSignature(totext, ftext, [string],text,[]).
opSignature(isDBObject, ftext, [string],bool,[]).
opSignature(getTypeNL, ftext, [_],text,[exp]).

opSignature(getValueNL, ftext, [[stream,T]],[stream,text],[exp]) :- isData(T).
opSignature(getValueNL, ftext, [[stream,[tuple,T]]],[stream,text],[exp]) :-
        isData(T).
opSignature(getValueNL, ftext, [_],text,[exp]).

opSignature(toobject, ftext, [text,T],T,[exp]) :- isData(T).
opSignature(toobject, ftext, [string,T],T,[exp]) :- isData(T).

opSignature(chartext, ftext, [int],text,[]).

opSignature(sendtextUDP, ftext, Args, text,[sidefx]) :-
  is_list(Args), length(Args) >= 3, length(Args) <= 5,
   my_list_to_set(Args,Types),
  subset(Types, [string,text]).

opSignature(receivetextUDP, ftext, [string,string,real],[stream,[tuple,[
        [ok,bool],[msg,text],[errmsg,string],[senderip, string],
        [senderport,string],[senderipversion,string]]]],[itract]).

opSignature(receivetextUDP, ftext, [string,text,real],[stream,[tuple,[
        [ok,bool],[msg,text],[errmsg,string],[senderip, string],
        [senderport,string],[senderipversion,string]]]],[itract]).

opSignature(receivetextUDP, ftext, [text,string,real],[stream,[tuple,[
        [ok,bool],[msg,text],[errmsg,string],[senderip, string],
        [senderport,string],[senderipversion,string]]]],[itract]).

opSignature(receivetextUDP, ftext, [text,text,real],[stream,[tuple,[
        [ok,bool],[msg,text],[errmsg,string],[senderip, string],
        [senderport,string],[senderipversion,string]]]],[itract]).

opSignature(receivetextstreamUDP, ftext, [string,string,real,real],
        [stream,[tuple,[
                 [ok,bool],[msg,text],[errmsg,string],[senderip, string],
                 [senderport,string],[senderipversion,string]]]],[itract]).

opSignature(receivetextstreamUDP, ftext,[string,text,real,real],[stream,[tuple,[
        [ok,bool],[msg,text],[errmsg,string],[senderip, string],
        [senderport,string],[senderipversion,string]]]],[itract]).

opSignature(receivetextstreamUDP, ftext, [text,string,real,real],
        [stream,[tuple,[
         [ok,bool],[msg,text],[errmsg,string],[senderip, string],
         [senderport,string],[senderipversion,string]]]],[itract]).

opSignature(receivetextstreamUDP, ftext, [text,text,real,real],[stream,[tuple,[
        [ok,bool],[msg,text],[errmsg,string],[senderip,string],
        [senderport,string],[senderipversion,string]]]],[itract]).

opSignature(svg2text, ftext, [svg], text,[exp]).
opSignature(text2svg, ftext, [text], svg,[exp]).

opSignature(crypt, ftext, [string], text,[]).
opSignature(crypt, ftext, [string,string], text,[]).
opSignature(crypt, ftext, [string,text], text,[]).
opSignature(crypt, ftext, [text], text,[]).
opSignature(crypt, ftext, [text,string], text,[]).
opSignature(crypt, ftext, [text,text], text,[]).

opSignature(checkpw, ftext, [text,text], bool,[]).
opSignature(checkpw, ftext, [text,string], bool,[]).
opSignature(checkpw, ftext, [string,text], bool,[]).
opSignature(checkpw, ftext, [string,string], bool,[]).

opSignature(md5, ftext, [string], string,[]).
opSignature(md5, ftext, [text], string,[]).

opSignature(blowfish_encode, ftext, [text,text], text,[]).
opSignature(blowfish_encode, ftext, [text,string], text,[]).
opSignature(blowfish_encode, ftext, [string,text], text,[]).
opSignature(blowfish_encode, ftext, [string,string], text,[]).

opSignature(blowfish_decode, ftext, [text,text], text,[]).
opSignature(blowfish_decode, ftext, [text,string], text,[]).
opSignature(blowfish_decode, ftext, [string,text], text,[]).
opSignature(blowfish_decode, ftext, [string,string], text,[]).



/*
2.7.3 DateTimeAlgebra

*/
opSignature((+), datetime, [instant,duration],instant,[]).
opSignature((+), datetime, [duration,duration],duration,[comm,ass]).
opSignature((-), datetime, [instant,instant],duration,[]).
opSignature((-), datetime, [instant,duration],instant,[]).
opSignature((-), datetime, [duration,duration],duration,[]).
opSignature((=), datetime, [duration,duration],bool,[comm]).
opSignature((=), datetime, [instant,instant],bool,[comm]).
opSignature((<), datetime, [duration,duration],bool,[]).
opSignature((<), datetime, [instant,instant],bool,[]).
opSignature((>), datetime, [duration,duration],bool,[]).
opSignature((>), datetime, [instant,instant],bool,[]).
opSignature((*), datetime, [duration,real],duration,[]).
opSignature(weekday_of, datetime, [instant],string,[]).
opSignature(leapyear, datetime, [int],bool,[]).
opSignature(year_of, datetime, [instant],int,[]).
opSignature(month_of, datetime, [instant],int,[]).
opSignature(day_of, datetime, [instant],int,[]).
opSignature(day_of, datetime, [duration],int,[]).
opSignature(hour_of, datetime, [instant],int,[]).
opSignature(minute_of, datetime, [instant],int,[]).
opSignature(second_of, datetime, [instant],int,[]).
opSignature(millisecond_of, datetime, [instant],int,[]).
opSignature(millisecond_of, datetime, [duration],int,[]).
opSignature(now, datetime, [],instant,[]).
opSignature(today, datetime, [],instant,[]).
opSignature(theInstant, datetime, IntList,instant,[]) :-
  is_list(IntList), length(IntList) >=1, length(IntList) <=7,
  my_list_to_set(IntList,[int]), !.
opSignature((/), datetime, [duration,duration],int,[]).
opSignature(minInstant, datetime, [],instant,[]).
opSignature(maxInstant, datetime, [],instant,[]).
opSignature(minDuration, datetime, [],duration,[]).
opSignature(maxDuration, datetime, [],duration,[]).
opSignature(create_duration, datetime, [real],duration,[]).
opSignature(create_duration, datetime, [int,int],duration,[]).
opSignature(create_instant, datetime, [real],instant,[]).
opSignature(create_instant, datetime, [int,int],instant,[]).
opSignature(duration2real, datetime, [duration],real,[]).
opSignature(instant2real, datetime, [instant],real,[]).
opSignature(tostring, datetime, [instant],string,[]).
opSignature(tostring, datetime, [duration],string,[]).


/*
2.7.4 SpatialAlgebra

*/
opSignature(isempty, spatial, [T],bool,[]) :-
  memberchk(T,[point,points,line,sline,region]),!.
opSignature((=), spatial, [T,T],bool,[comm,bbox(2)]) :-
  memberchk(T,[point,points,line,sline,region]),!.
opSignature((#), spatial, [T,T],bool,[comm,bbox(2)]) :-
  memberchk(T,[point,points,line,sline,region]),!.
opSignature(intersects, spatial, [T1,T2],bool,[comm,bbox(2)]) :-
  (   (memberchk(T1,[points,line,region]), memberchk(T2,[points,line,region]))
    ; (T1 = sline, T2 = sline)
  ),!.
opSignature(inside, spatial, [T1,T2],bool,[bbox(2)]) :-
  memberchk(T1,[point,points,line,region]),
  memberchk(T2,[points,line,region]),!.
opSignature(adjacent, spatial, [T1,T2],bool,[comm,bbox(2)]) :-
  memberchk(T1,[points,line,region]), memberchk(T2,[points,line,region]),!.
opSignature(overlaps, spatial, [region,region],bool,[comm,bbox(2)]).
opSignature(onborder, spatial, [point,region],bool,[bbox(2)]).
opSignature(ininterior, spatial, [point,region],bool,[bbox(2)]).
opSignature(intersection, spatial, [T,T],T,[comm]) :-
  memberchk(T,[point,points,line,region]),!.
opSignature(minus, spatial, [T,T],T,[]) :-
  memberchk(T,[point,points,line,region]),!.
opSignature(union, spatial, [point,points],points,[comm,ass]).
opSignature(union, spatial, [points,point],points,[comm,ass]).
opSignature(union, spatial, [points,points],points,[comm,ass]).
opSignature(crossings, spatial, [line,line],points,[comm]).
opSignature(crossings, spatial, [sline,sline],points,[comm]).
opSignature(touchpoints, spatial, [line,region],points,[]).
opSignature(touchpoints, spatial, [region,region],points,[comm]).
opSignature(commonborder, spatial, [region,region],line,[comm]).
opSignature(single, spatial, [points],point,[]).
opSignature(distance, spatial, [T1,T2],real,[comm]) :-
  memberchk(T1,[point,points,line,sline,region,rect]),
  memberchk(T2,[point,points,line,sline,region,rect]),!.
opSignature(direction, spatial, [point,point],real,[]).
opSignature(no_components, spatial, [T],int,[]) :-
  memberchk(T,[points,line,region,rect]),!.
opSignature(no_segments, spatial, [T],int,[]) :-
  memberchk(T,[line,sline,region]),!.
opSignature(size, spatial, [T],real,[]) :-
  memberchk(T,[line,sline,region]),!.
opSignature(size, spatial, [T],real,[]) :-
  memberchk(T,[point,points,line,sline,region]),!.
opSignature(translate, spatial, [T,real,real],T,[]) :-
  memberchk(T,[point,points,line,region]),!.
opSignature(rotate, spatial, [T,real,real,real],T,[]) :-
  memberchk(T,[point,points,line,region]),!.
opSignature(center, spatial, [points],point,[]).
opSignature(convexhull, spatial, [points],region,[]).
opSignature(windowclippingin, spatial, [line,rect],line,[exp]).
opSignature(windowclippingin, spatial, [region,rect],region,[exp]).
opSignature(windowclippingout, spatial, [line,rect],line,[exp]).
opSignature(windowclippingout, spatial, [region,rect],region,[exp]).
opSignature(components, spatial, [points],[stream,point],[]).
opSignature(components, spatial, [region],[stream,region],[]).
opSignature(components, spatial, [line],[stream,line],[]).
opSignature(vertices, spatial, [region],points,[]).
opSignature(vertices, spatial, [line],points,[]).
opSignature(boundary, spatial, [region],line,[]).
opSignature(boundary, spatial, [line],points,[]).
opSignature(scale, spatial, [T,real],T,[]) :-
  memberchk(T,[point,points,line,region]),!.
opSignature(atpoint, spatial, [sline,point,bool],real,[]).
opSignature(atposition, spatial, [sline,point,bool],point,[]).
opSignature(subline, spatial, [sline,real,real,bool],line,[]).
opSignature((+), spatial, [point,point],point,[comm,ass]).
opSignature(getx, spatial, [point],real,[]).
opSignature(gety, spatial, [point],real,[]).
opSignature(line2region, spatial, [line],region,[]).
opSignature(rect2region, spatial, [rect],region,[]).
opSignature(area, spatial, [region],real,[]).
opSignature(polylines, spatial, [line,bool],[stream,line],[]).
opSignature(polylines, spatial, [line,bool,points],[stream,line],[]).
opSignature(polylinesC, spatial, [line,bool],[stream,line],[]).
opSignature(polylinesC, spatial, [line,bool,points],[stream,line],[]).
opSignature(segments, spatial, [line],[stream,line],[]).
opSignature(get, spatial, [points,int],point,[]).
opSignature(simplify, spatial, [line,real],line,[]).
opSignature(simplify, spatial, [line,real,points],line,[]).
opSignature(realminize, spatial, [line],line,[]).
opSignature(makeline, spatial, [point,point],line,[]).
opSignature(makesline, spatial, [point,point],sline,[]).
opSignature(union2, spatial, [line,line],line,[comm,ass]).
opSignature(union2, spatial, [line,region],region,[comm,ass]).
opSignature(union2, spatial, [region,line],region,[comm,ass]).
opSignature(union2, spatial, [region,region],region,[comm,ass,exp]).
opSignature(intersection2, spatial, [line,line],line,[comm,ass]).
opSignature(intersection2, spatial, [line,region],line,[comm]).
opSignature(intersection2, spatial, [region,line],line,[comm]).
opSignature(intersection2, spatial, [region,region],region,[comm,ass,exp]).
opSignature(difference2, spatial, [line,line],line,[]).
opSignature(difference2, spatial, [line,region],line,[]).
opSignature(difference2, spatial, [region,line],region,[]).
opSignature(difference2, spatial, [region,region],region,[exp]).
opSignature(commonborder2, spatial, [region,region],line,[comm,ass]).
opSignature(toline, spatial, [sline],line,[]).
opSignature(fromline, spatial, [line],sline,[]).
opSignature(iscycle, spatial, [sline],bool,[]).
opSignature(bbox, spatial, [T], rect, []):-
  memberchk(T, [region, point, line, points, sline]),!.
opSignature(collect_line, spatial, [[stream,line]],line,[aggr,block,exp]).
opSignature(collect_line, spatial, [[stream,sline]],line,[aggr,block,exp]).
opSignature(collect_line, spatial, [[stream,point]],line,[aggr,block,exp]).
opSignature(collect_sline, spatial, [[stream,line]],sline,[aggr,block,exp]).
opSignature(collect_sline, spatial, [[stream,sline]],sline,[aggr,block,exp]).
opSignature(collect_sline, spatial, [[stream,point]],sline,[aggr,block,exp]).

/*
2.7.5 TemporalAlgebra

*/
opSignature(isempty, temporal, [T],bool,[]) :-
  memberchk(T,[instant,rbool,rint,rreal,periods,rstring,
                       ubool,uint,ureal,upoint,ustring,
                       ibool,iint,ireal,ipoint,istring]),!.
opSignature((=), temporal, [T,T],bool,[comm]) :-
  memberchk(T,[instant,rbool,rint,rreal,periods,rstring,
                       ibool,iint,ireal,ipoint,istring]),!.
opSignature((#), temporal, [T,T],bool,[comm]) :-
  memberchk(T,[instant,rbool,rint,rreal,periods,rstring,
                       ibool,iint,ireal,ipoint,istring]),!.
opSignature(nonequal, temporal, [T,T],bool,[comm]) :-
  memberchk(T,[mbool,mint,mreal,mpoint,mstring]),!.
opSignature(equal, temporal, [T,T],bool,[comm]) :-
  memberchk(T,[mbool,mint,mreal,mpoint,mstring]),!.
opSignature((<), temporal, [instant,instant],bool,[]).
opSignature((<=), temporal, [instant,instant],bool,[]).
opSignature((>), temporal, [instant,instant],bool,[]).
opSignature((>=), temporal, [instant,instant],bool,[]).
opSignature(intersects, temporal, [T,T],bool,[comm]) :-
  memberchk(T,[rbool,rint,rreal,periods,rstring]),!.
opSignature(inside, temporal, [instant,period],bool,[]).
opSignature(inside, temporal, [period,period],bool,[]).
opSignature(inside, temporal, [bool,rbool],bool,[]).
opSignature(inside, temporal, [rbool,rbool],bool,[]).
opSignature(inside, temporal, [int,rint],bool,[]).
opSignature(inside, temporal, [rint,rint],bool,[]).
opSignature(inside, temporal, [real,rreal],bool,[]).
opSignature(inside, temporal, [rreal,rreal],bool,[]).
opSignature(inside, temporal, [string,rstring],bool,[]).
opSignature(inside, temporal, [rstring,rstring],bool,[]).
opSignature(before, temporal, [bool,rbool],bool,[]).
opSignature(before, temporal, [rbool,bool],bool,[]).
opSignature(before, temporal, [rbool,rbool],bool,[]).
opSignature(before, temporal, [int,rint],bool,[]).
opSignature(before, temporal, [rint,int],bool,[]).
opSignature(before, temporal, [rint,rint],bool,[]).
opSignature(before, temporal, [real,rreal],bool,[]).
opSignature(before, temporal, [rreal,real],bool,[]).
opSignature(before, temporal, [rreal,rreal],bool,[]).
opSignature(before, temporal, [string,rstring],bool,[]).
opSignature(before, temporal, [rstring,string],bool,[]).
opSignature(before, temporal, [rstring,rstring],bool,[]).
opSignature(before, temporal, [instant,periods],bool,[]).
opSignature(before, temporal, [periods,instant],bool,[]).
opSignature(before, temporal, [periods,periods],bool,[]).
opSignature(intersection, temporal, [T,T],T,[comm,ass,exp]) :-
  memberchk(T,[rbool,rint,rreal,rstring,periods]), !.
opSignature(union, temporal, [T,T],T,[comm,ass,exp]) :-
  memberchk(T,[rbool,rint,rreal,rstring,periods]), !.
opSignature(minus, temporal, [T,T],T,[exp]) :-
  memberchk(T,[rbool,rint,rreal,rstring,periods]), !.
opSignature(minimum, temporal, [rbool],bool,[]).
opSignature(minimum, temporal, [rint],int,[]).
opSignature(minimum, temporal, [rreal],real,[]).
opSignature(minimum, temporal, [rstring],string,[]).
opSignature(minimum, temporal, [periods],instant,[]).
opSignature(maximum, temporal, [rbool],bool,[]).
opSignature(maximum, temporal, [rint],int,[]).
opSignature(maximum, temporal, [rreal],real,[]).
opSignature(maximum, temporal, [rstring],string,[]).
opSignature(maximum, temporal, [periods],instant,[]).
opSignature(no_components, temporal, [T],int,[]) :-
  memberchk(T,[rbool,rint,rreal,rstring,periods,
               mbool,mint,mreal,mstring,mpoint,movingregion]),!.
opSignature(inst, temporal, [T],instant,[]) :-
  memberchk(T,[ipoint,ibool,iint,ireal,istring,intimeregion]),!.
opSignature(val, temporal, [ibool],bool,[]).
opSignature(val, temporal, [iint],int,[]).
opSignature(val, temporal, [ireal],real,[]).
opSignature(val, temporal, [istring],string,[]).
opSignature(val, temporal, [ipoint],point,[]).
opSignature(val, temporal, [intimeregion],region,[]).
opSignature(atinstant, temporal, [T1,instant],T2,[]) :-
  memberchk((T1,T2),[(mbool,ibool),(mint,iint),(mreal,ireal),(mstring,istring),
                     (mpoint,ipoint),(movingregion,intimeregion)]),!.
opSignature(atperiods, temporal, [T,periods],T,[exp]) :-
  memberchk(T,[mbool,mint,mreal,mstring,mpoint,movingregion]),!.
opSignature(deftime, temporal, [T],periods,[exp]) :-
  memberchk(T,[mbool,mint,mreal,mstring,mpoint,movingregion]),!.
opSignature(trajectory, temporal, [mpoint],line,[exp]).
opSignature(present, temporal, [T1,T2],bool,[bbox(3)]) :-
  memberchk(T2,[instant,periods]),
  memberchk(T1,[mbool,mint,mreal,mstring,mpoint,movingregion]),!.
opSignature(passes, temporal, [T1,T2],bool,[bbox(2),exp]) :-
  memberchk((T1,T2),[(mpoint,point),(mpoint,region),(mpoint,rect)]),!.
opSignature(passes, temporal, [T1,T2],bool,[exp]) :-
  memberchk((T1,T2),[(mbool,bool),(mint,int),(mreal,real),(mstring,string)]),!.
opSignature(initial, temporal, [T1],T2,[]) :-
  memberchk((T1,T2),[(mpoint,ipoint),(movingregion,intimeregion),(mint,iint),
                     (mreal,ireal),(mbool,ibool),(mstring,istring)]),!.
opSignature(final, temporal, [T1],T2,[]) :-
  memberchk((T1,T2),[(mpoint,ipoint),(movingregion,intimeregion),(mint,iint),
                     (mreal,ireal),(mbool,ibool),(mstring,istring)]),!.
opSignature(units, temporal, [T],[stream,UT],[]) :-
  memberchk((T,UT),[(mpoint,upoint),(movingregion,uregion),(mreal,ureal),
                    (mint,uint),(mbool,ubool),(mstring,ustring)]),!.
opSignature(bbox, temporal, [T],rect3,[]) :-
  memberchk(T,[mpoint,movingregion,upoint,uregion,
               ipoint,intimeregion,instant,periods]),!.
opSignature(mbrange, temporal, [T],T,[exp]) :-
  memberchk(T,[periods,rreal,rint,rbool,rstring]).
opSignature(bbox2d, temporal, [T],rect,[]) :-
  memberchk(T,[mpoint,upoint,ipoint,movingregion,intimeregion,uregion]),!.
opSignature(bboxold, temporal, [T],rect3,[]) :-
  memberchk(T,[mpoint,upoint,ipoint,movingregion,intimeregion,uregion]),!.
opSignature(bboxold, temporal, [T],T,[]) :-
  memberchk(T,[periods,rreal,rint,rbool,rstring]).
opSignature(at, temporal, [T1,T2],T1,[exp]) :-
  memberchk((T1,T2),[(mpoint,point),(movingregion,region),(mint,int),
                     (mreal,real),(mbool,bool),(mstring,string)]),!.
opSignature(distance, temporal, [mpoint,point],mreal,[exp]).
opSignature(simplify, temporal, [mpoint,real],mpoint,[exp]).
opSignature(simplify, temporal, [mpoint,real,duration],mpoint,[exp]).
opSignature(simplify, temporal, [mreal,real],mreal,[exp]).
opSignature(integrate, temporal, [ureal],real,[]).
opSignature(integrate, temporal, [mreal],real,[]).
opSignature(linearize, temporal, [ureal],ureal,[]).
opSignature(linearize, temporal, [mreal],mreal,[]).
opSignature(linearize2, temporal, [mreal],mreal,[]).
opSignature(linearize2, temporal, [ureal],ureal,[]).
opSignature(approximate,temporal,[[stream,[tuple,AttrList]],TimeAttr,ValAttr],
        T,[exp]):-
  is_list(AttrList),
  member([TimeAttr,instant],AttrList),
  memberchk([ValAttr,T],AttrList),
  (T = real ; T = point ).
opSignature(minimum, temporal, [T1],T2,[exp]) :-
  memberchk((T1,T2),[(rint,int),(rbool,bool),(rreal,real),
                     (rstring,string),(periods,instant)]),!.
opSignature(maximum, temporal, [T1],T2,[exp]) :-
  memberchk((T1,T2),[(rint,int),(rbool,bool),(rreal,real),
                     (rstring,string),(periods,instant)]),!.
opSignature(breakpoints, temporal, [mpoint,duration],points,[exp]).
opSignature(translate, temporal, [mpoint,duration,real,real],mpoint,[]).
opSignature(theyear, temporal, [int],periods,[]).
opSignature(themonth, temporal, [int,int],periods,[]).
opSignature(theday, temporal, [int,int,int],periods,[]).
opSignature(thehour, temporal, [int,int,int,int],periods,[]).
opSignature(theminute, temporal, [int,int,int,int,int],periods,[]).
opSignature(thesecond, temporal, [int,int,int,int,int,int],periods,[]).
opSignature(theperiod, temporal, [periods,periods],periods,[]).
opSignature(theRange, temporal, [T1,T1,bool,bool],T2,[]) :-
  memberchk((T1,T2),[(bool,rbool),(int,rint),(real,rreal),(string,rstring)]),!.
opSignature(box3d, temporal, [T],rect3,[]) :-
  memberchk(T,[rect,instant,periods]), !.
opSignature(box3d, temporal, [rect,T],rect3,[]) :-
  memberchk(T,[instant,periods]), !.
opSignature(box2d, temporal, [T],rect,[]):-
  memberchk(T,[rect, rect3, rect4, rect8]), !.
opSignature(mbool2mint, temporal, [mbool],mint,[]).
opSignature(extdeftime, temporal, [T1,T2],T1,[]) :-
  memberchk((T1,T2),[(mbool,ubool),(mint,uint)]),!.
opSignature(translateappend, temporal, [mpoint,mpoint,duration],mpoint,[exp]).
opSignature(translateappendS, temporal,
        [[stream,[tuple,AttrList]],Attr,duration],mpoint,[exp]) :-
  is_list(AttrList), memberchk([Attr,mpoint]), !.
opSignature(reverse, temporal, [mpoint],mpoint,[idem,exp]).
opSignature(samplempoint, temporal, [mpoint,duration],mpoint,[exp]).
opSignature(samplempoint, temporal, [mpoint,duration,bool],mpoint,[exp]).
opSignature(samplempoint, temporal, [mpoint,duration,bool,bool],mpoint,[exp]).
%opSignature(gps, temporal, [mpoint,duration],
%    [stream,tuple([(Time,instant),(Position,point)]),[]].
opSignature(disturb, temporal, [mpoint,real,real],mpoint,[exp]).
opSignature(length, temporal, [mpoint],real,[exp]).
opSignature(hat, temporal, [mint],mint,[exp]).
opSignature(restrict, temporal, [mint],mint,[exp]).
opSignature(restrict, temporal, [mint,int],mint,[exp]).
opSignature(speedup, temporal, [mpoint,real],mpoint,[]).
opSignature(avespeed, temporal, [mpoint],real,[]).
opSignature(submove, temporal, [mpoint,real],mpoint,[exp]).
opSignature(uval, temporal, [uint], int,[]).
opSignature(distancetraversed, temporal, [mpoint], mreal,[exp]).
opSignature(delay, temporal, [mpoint, mpoint], mreal,[exp]).

/*
2.7.5 TemporalExtAlgebra

*/
opSignature(at, temporalext, [T1,T2],T1,[exp]) :-
  memberchk((T1,T2),[(mint,rint),(mbool,rbool),(mreal,rreal),
  (mstring,rstring)]), !.
opSignature(at, temporalext, [mpoint,points],mpoint,[bbox(2),exp]).
opSignature(at, temporalext, [mpoint,line],mpoint,[bbox(2),exp]).
opSignature(passes, temporalext, [mpoint,points],bool,[bbox(2),exp]).
opSignature(passes, temporalext, [mpoint,line],bool,[bbox(2),exp]).
opSignature(passes, temporalext, [movingregion,point],bool,[bbox(2),exp]).
opSignature(passes, temporalext, [movingregion,points],bool,[bbox(2),exp]).
opSignature(val, temporal, [istring],string,[]).
opSignature(derivative_new, temporalext, [mreal],mreal,[]).
opSignature(derivable_new, temporalext, [mreal],mbool,[]).
opSignature(speed_new, temporalext, [mpoint],mreal,[]).
opSignature(velocity_new, temporalext, [mpoint],mpoint,[]).
opSignature(mdirection, temporalext, [mpoint],mreal,[]).
opSignature(locations, temporalext, [mpoint],points,[exp]).
opSignature(atmin, temporalext, [T],T,[exp]) :-
  memberchk(T,[mint,mbool,mreal,mstring]),!.
opSignature(atmax, temporalext, [T],T,[exp]) :-
  memberchk(T,[mint,mbool,mreal,mstring]),!.
opSignature(rangevalues, temporalext, [T1],T2,[exp]) :-
  memberchk((T1,T2),[(mint,rint),(mreal,rreal),
                     (mbool,rbool),(mstring,rstring)]),!.
opSignature(sometimes, temporalext, [mbool],bool,[exp]).
opSignature(always, temporalext, [mbool],bool,[exp]).
opSignature(never, temporalext, [mbool],bool,[exp]).
opSignature(setunitoftime, temporalext, [real],real,[sidefx]).
opSignature(setunitofdistance, temporalext, [real],real,[sidefx]).
opSignature(concatS, temporalext, [[stream,mpoint]],mpoint,[aggr,exp]).
opSignature(concatS2, temporalext, [[stream,mpoint],int],mpoint,[aggr,exp]).
opSignature(everNearerThan, temporalext, [mpoint,mpoint,real],bool,[exp]).
opSignature(everNearerThan, temporalext, [mpoint,point,real],bool,[exp]).
opSignature(everNearerThan, temporalext, [point,mpoint,real],bool,[exp]).

/*
2.7.6 TemporalLiftedAlgebra

*/
opSignature((=), temporallifted, [T,T],mbool,[comm,exp]) :-
  memberchk(T,[mbool,mint,mstring,mreal,mpoint,movingregion]),!.
opSignature((=), temporallifted, [T1,T2],mbool,[comm,exp,liftedequality]) :-
  (   memberchk((T1,T2),[(mbool,bool),(mint,int),(mstring,string),
                         (mreal,real)])
    ; memberchk((T2,T1),[(mbool,bool),(mint,int),(mstring,string),
                         (mreal,real)])
  ),!.
opSignature((=), temporallifted, [T1,T2],mbool,[comm,exp,liftedspatialrange]) :-
  (   memberchk((T1,T2),[(mpoint,point),(movingregion,region)])
    ; memberchk((T2,T1),[(mpoint,point),(movingregion,region)])
  ),!.
opSignature((#), temporallifted, [T,T],mbool,[comm,exp]) :-
  memberchk(T,[mbool,mint,mstring,mreal,mpoint,movingregion]),!.
opSignature((#), temporallifted, [T1,T2],mbool,[comm,exp]) :-
  (   memberchk((T1,T2),[(mbool,bool),(mint,int),(mstring,string),
                         (mreal,real),(mpoint,point),(movingregion,region)])
    ; memberchk((T2,T1),[(mbool,bool),(mint,int),(mstring,string),
                         (mreal,real),(mpoint,point),(movingregion,region)])
  ),!.
opSignature((<), temporallifted, [T1,T2],mbool,[exp,liftedleftrange]) :-
  memberchk((T1,T2),[(mbool,bool),(mint,int),(mstring,string),
                         (mreal,real)])
  ,!.
opSignature((<), temporallifted, [T1,T2],mbool,[exp,liftedrightrange]) :-
  memberchk((T1,T2),[(bool,mbool),(int,mint),(string,mstring),
                         (real,mreal)])
  ,!.
opSignature((<=), temporallifted, [T1,T2],mbool,[exp,liftedleftrange]) :-
  memberchk((T1,T2),[(mbool,bool),(mint,int),(mstring,string),
                         (mreal,real)])
  ,!.
opSignature((<=), temporallifted, [T1,T2],mbool,[exp,liftedrightrange]) :-
  memberchk((T1,T2),[(bool,mbool),(int,mint),(string,mstring),
                         (real,mreal)])
  ,!.
opSignature((>), temporallifted, [T1,T2],mbool,[exp,liftedrightrange]) :-
  memberchk((T1,T2),[(mbool,bool),(mint,int),(mstring,string),
                         (mreal,real)])
  ,!.
opSignature((>), temporallifted, [T1,T2],mbool,[exp,liftedleftrange]) :-
  memberchk((T1,T2),[(bool,mbool),(int,mint),(string,mstring),
                         (real,mreal)])
  ,!.
opSignature((>=), temporallifted, [T1,T2],mbool,[exp,liftedrightrange]) :-
  memberchk((T1,T2),[(mbool,bool),(mint,int),(mstring,string),
                         (mreal,real)])
  ,!.
opSignature((>=), temporallifted, [T1,T2],mbool,[exp,liftedleftrange]) :-
  memberchk((T1,T2),[(bool,mbool),(int,mint),(string,mstring),
                         (real,mreal)])
  ,!.
opSignature(isempty, temporallifted, [T],mbool,[]) :-
  memberchk(T,[mbool, mint, mstring, mreal, mpoint, mregion]),!.
opSignature(inside, temporallifted, [mpoint,points],mbool,
                                    [exp,liftedspatialrange]).
opSignature(inside, temporallifted, [mpoint,line],mbool,
                                    [exp,liftedspatialrange]).
opSignature(inside, temporallifted, [movingregion,points],
                                     mbool,[exp,liftedspatialrange]).
opSignature(inside, temporallifted, [movingregion,line],mbool,
                                    [exp,liftedspatialrange]).
opSignature(intersection, temporallifted, [T,T],T,[comm,ass,exp]) :-
  memberchk(T,[mbool, mint, mstring, mreal]),!.
opSignature(intersection, temporallifted, [T1,T2],T1,[comm,exp]) :-
  memberchk((T1,T2),[(mbool,bool),(mint,int),(mstring,string),(mreal,real)]),!.
opSignature(intersection, temporallifted, [T1,T2],T2,[comm,exp]) :-
  memberchk((T1,T2),[(bool,mbool),(int,mint),(string,mstring),(real,mreal)]),!.
opSignature(intersection, temporallifted, [mpoint,mpoint],
            mpoint,[comm,ass,exp]).
opSignature(intersection, temporallifted, [T,mpoint],mpoint,[comm,exp]) :-
  memberchk(T,[line,points]),!.
opSignature(intersection, temporallifted, [mpoint,T],mpoint,[comm,exp]) :-
  memberchk(T,[line,points]),!.
opSignature(union, temporallifted, [mpoint,region],movingregion,[]).
opSignature(union, temporallifted, [mpoint,movingregion],movingregion,[]).
opSignature(union, temporallifted, [point,movingregion],movingregion,[]).

opSignature(minus, temporallifted, [T,T],T,[exp]) :-
  memberchk(T,[mbool,mint,mstring,mreal]),!.
opSignature(minus, temporallifted, [T1,T2],T1,[exp]) :-
  memberchk((T1,T2),[(mbool,bool),(mint,int),(mstring,string),(mreal,real)]),!.
opSignature(minus, temporallifted, [T1,T2],T2,[exp]) :-
  memberchk((T1,T2),[(bool,mbool),(int,mint),(string,mstring),(real,mreal)]),!.
opSignature(minus, temporallifted, [region,mpoint],movingregion,[]).
opSignature(minus, temporallifted, [movingregion,point],movingregion,[]).
opSignature(minus, temporallifted, [movingregion,mpoint],movingregion,[]).
opSignature(minus, temporallifted, [movingregion,points],movingregion,[]).
opSignature(minus, temporallifted, [movingregion,line],movingregion,[]).
opSignature(rough_center, temporallifted, [movingregion],mpoint,[exp]).
opSignature(no_components, temporallifted, [movingregion],mint,[exp]).
opSignature(perimeter, temporallifted, [movingregion],mreal,[exp]).
opSignature(area, temporallifted, [movingregion],mreal,[exp]).
opSignature(distance, temporallifted, [mpoint,mpoint],mreal,[comm,exp]).
opSignature(distance, temporallifted, [mreal,mreal],mreal,[comm,exp]).
opSignature(distance, temporallifted, [mreal,real],mreal,[comm,exp]).
opSignature(distance, temporallifted, [real,mreal],mreal,[comm,exp]).
opSignature(and, temporallifted, [bool,mbool],mbool,[comm,exp]).
opSignature(and, temporallifted, [mbool,mbool],mbool,[comm,ass,exp]).
opSignature(and, temporallifted, [mbool,bool],mbool,[comm,exp]).
opSignature(or, temporallifted, [bool,mbool],mbool,[comm,exp]).
opSignature(or, temporallifted, [mbool,mbool],mbool,[comm,ass,exp]).
opSignature(or, temporallifted, [mbool,bool],mbool,[comm,exp]).
opSignature(not, temporallifted, [mbool],mbool,[idem,exp]).
opSignature(zero, temporallifted, [],mint,[]).
opSignature(periods2mint, temporallifted, [periods],mint,[comm,exp]).
opSignature((+), temporallifted, [mint,mint],mint,[comm,ass,exp]).
opSignature(eplus, temporallifted, [mint,mint],mint,[comm,ass,exp]).
opSignature(concat, temporallifted, [mpoint,mpoint],mpoint,[exp]).
opSignature(abs, temporallifted, [mreal],mreal,[exp]).

/*
2.7.7 TemporalUnitAlgebra

*/
opSignature(makemvalue, temporalunit, [[stream,[tuple,AttrList]],Attr],T2,
        [block,aggr,exp]) :-
  ( not(optimizerOption(determinePredSig)) ; is_list(AttrList)),
  member([Attr,T1],AttrList),
  member((T1,T2),[(upoint,mpoint),(ubool,mbool),(uint,mint),(ureal,mreal),
                  (ustring,mstring),(uregion,movingregion)]),!.
opSignature(the_mvalue, temporalunit, [[stream,T1]],T2,[block,aggr,exp]) :-
  memberchk((T1,T2),[(upoint,mpoint),(ubool,mbool),(uint,mint),(ureal,mreal),
                     (ustring,mstring),(uregion,movingregion)]),!.
opSignature(queryrect2d, temporalunit, [instant],rect,[]).
opSignature(point2d, temporalunit, [periods],point,[]).
opSignature(circle, temporalunit, [point,real,int],region,[]).
opSignature(makepoint, temporalunit, [int,int],point,[]).
opSignature(makepoint, temporalunit, [real,real],point,[]).
opSignature(isempty, temporalunit, [T],bool,[]) :-
  memberchk(T,[upoint,uregion,ubool,uint,ureal,ustring]),!.
opSignature(deftime, temporalunit, [T],periods,[]) :-
  memberchk(T,[upoint,uregion,ubool,uint,ureal,ustring]),!.
opSignature(present, temporalunit, [T,instant],bool,[bbox(3)]) :-
  memberchk(T,[upoint,uregion,ubool,uint,ureal,ustring]),!.
opSignature(present, temporalunit, [T,periods],bool,[bbox(3)]) :-
  memberchk(T,[upoint,uregion,ubool,uint,ureal,ustring]),!.
opSignature(initial, temporalunit, [[stream,T1]],T2,[block,aggr]) :-
  memberchk((T1,T2),[(upoint,ipoint),(uregion,intimeregion),(ubool,ibool),
                     (uint,iint),(ureal,ireal),(ustring,istring)]),!.
opSignature(initial, temporalunit, [T1],T2,[]) :-
  memberchk((T1,T2),[(upoint,ipoint),(uregion,intimeregion),(ubool,ibool),
                     (uint,iint),(ureal,ireal),(ustring,istring)]),!.
opSignature(final, temporalunit, [[stream,T1]],T2,[block,aggr]) :-
  memberchk((T1,T2),[(upoint,ipoint),(uregion,intimeregion),(ubool,ibool),
                     (uint,iint),(ureal,ireal),(ustring,istring)]),!.
opSignature(final, temporalunit, [T1],T2,[]) :-
  memberchk((T1,T2),[(upoint,ipoint),(uregion,intimeregion),(ubool,ibool),
                     (uint,iint),(ureal,ireal),(ustring,istring)]),!.
opSignature(atinstant, temporalunit, [T1,instant],T2,[]) :-
  memberchk((T1,T2),[(ubool,ibool),(uint,iint),(ureal,ireal),(ustring,istring),
                     (upoint,ipoint),(uregion,intimeregion)]),!.
opSignature(atperiods, temporalunit, [T,periods],[stream,T],[]) :-
  memberchk(T,[ubool,uint,ureal,ustring,upoint,uregion]),!.
opSignature(at, temporalunit, [T1,T2],[stream,T1],[]) :-
  memberchk((T1,T2),[(ubool,bool),(uint,int),(ureal,real),(ustring,string),
                     (upoint,point),(upoint,intimeregion)]),!.
opSignature(at, temporalunit, [upoint,region],upoint,[]).
opSignature(at, temporalunit, [upoint,rect],upoint,[]).
opSignature(atmax, temporalunit, [ureal],[stream,ureal],[]).
opSignature(atmax, temporalunit, [T],T,[]) :-
  memberchk(T,[ubool,uint,ustring]),!.
opSignature(atmin, temporalunit, [ureal],[stream,ureal],[]).
opSignature(atmin, temporalunit, [T],T,[]) :-
  memberchk(T,[ubool,uint,ustring]),!.
opSignature(intersection, temporalunit, [T1,T2],[stream,TI1],[comm]) :-
  member((TI1,TI2),[(ubool,bool),(uint,int),(ureal,real),
                       (ustring,string),(upoint,point)]),
  ((T1=TI1,T2=TI2);(T1=TI2,T2=TI1);(T1=TI1,T2=TI1)), !.
opSignature(intersection, temporalunit,[line,upoint],[stream,upoint],[comm]).
opSignature(intersection, temporalunit,[upoint,line],[stream,upoint],[comm]).
opSignature(intersection, temporalunit,[upoint,uregion],[stream,upoint],[comm]).
opSignature(intersection, temporalunit,[uregion,upoint],[stream,upoint],[comm]).
opSignature(intersection, temporalunit,[upoint,region],[stream,upoint],[comm]).
opSignature(intersection, temporalunit,[region,upoint],[stream,upoint],[comm]).
opSignature(inside, temporalunit, [upoint,uregion],[stream,ubool],[comm]).
opSignature(inside, temporalunit, [upoint,line],[stream,ubool],[comm]).
opSignature(inside, temporalunit, [upoint,points],[stream,ubool],[]).
opSignature(inside, temporalunit, [uregion,points],[stream,ubool],[]).
opSignature(passes, temporalunit, [T1,T2],bool,[bbox(2)]) :-
  memberchk((T1,T2),[(upoint,point),(uregion,region)]), !.
opSignature(passes, temporalunit, [T1,T2],bool,[]) :-
  memberchk((T1,T2),[(ubool,bool),(uint,int),(ureal,real),(ustring,string)]), !.
opSignature(get_duration, temporalunit, [periods],duration,[]).
opSignature(trajectory, temporalunit, [upoint],line,[]).
opSignature(distance, temporalunit, [T1,T2],ureal,[comm]) :-
  memberchk((T1,T2),[(upoint,upoint),(upoint,point),(point,upoint),
                     (uint,uint),(uint,int),(int,uint),(ureal,ureal),
                     (ureal,real),(real,ureal)]), !.
opSignature(abs, temporalunit, [uint],uint,[]).
opSignature(abs, temporalunit, [ureal],[stream,ureal],[]).
opSignature(speed, temporalunit, [mpoint],mreal,[]).
opSignature(speed, temporalunit, [upoint],ureal,[]).
opSignature(velocity, temporalunit, [mpoint],mpoint,[]).
opSignature(velocity, temporalunit, [upoint],upoint,[]).
opSignature(derivable, temporalunit, [ureal],ubool,[]).
opSignature(derivative, temporalunit, [ureal],ureal,[]).
opSignature(no_components, temporalunit, [T],uint,[]) :-
  memberchk(T,[ubool,uint,ureal,upoint,uregion]), !.
opSignature(not, temporalunit, [ubool],ubool,[idem]).
opSignature(and, temporalunit, [ubool,ubool],ubool,[comm,ass]).
opSignature(and, temporalunit, [bool,ubool],ubool,[comm]).
opSignature(and, temporalunit, [ubool,bool],ubool,[comm]).
opSignature(or, temporalunit, [ubool,ubool],ubool,[comm,ass]).
opSignature(or, temporalunit, [bool,ubool],ubool,[comm]).
opSignature(or, temporalunit, [ubool,bool],ubool,[comm]).
opSignature(sometimes, temporalunit, [ubool],bool,[]).
opSignature(sometimes, temporalunit, [[stream,ubool]],bool,[block,aggr,exp]).
opSignature(never, temporalunit, [ubool],bool,[]).
opSignature(never, temporalunit, [[stream,ubool]],bool,[block,aggr,exp]).
opSignature(always, temporalunit, [ubool],bool,[]).
opSignature(always, temporalunit, [[stream,ubool]],bool,[block,aggr,exp]).
opSignature(==, temporalunit, [T,T],bool,[comm]) :-
  memberchk(T,[ubool,uint,ureal,upoint,ustring,uregion]), !.
opSignature((##), temporalunit, [T,T],bool,[comm]) :-
  memberchk(T,[ubool,uint,ureal,upoint,ustring,uregion]), !.
opSignature((<<), temporalunit, [T,T],bool,[]) :-
  memberchk(T,[ubool,uint,ureal,upoint,ustring,uregion]), !.
opSignature((>>), temporalunit, [T,T],bool,[]) :-
  memberchk(T,[ubool,uint,ureal,upoint,ustring,uregion]), !.
opSignature((<<==), temporalunit, [T,T],bool,[]) :-
  memberchk(T,[ubool,uint,ureal,upoint,ustring,uregion]), !.
opSignature((>>==), temporalunit, [T,T],bool,[]) :-
  memberchk(T,[ubool,uint,ureal,upoint,ustring,uregion]), !.
opSignature((=), temporalunit, [T1,T2],[stream,ubool],[comm]) :-
  member((TI1,TI2),[(ubool,bool),(uint,int),(ureal,real),(ustring,string),
                     (upoint,point),(uregion,region)]),
  ((T1=TI1,T2=TI2);(T1=TI2,T2=TI1);(T1=T2,T1=TI1)), !.
opSignature((#), temporalunit, [T1,T2],[stream,ubool],[comm]) :-
  member((TI1,TI2),[(ubool,bool),(uint,int),(ureal,real),(ustring,string),
                     (upoint,point),(uregion,region)]),
  ((T1=TI1,T2=TI2);(T1=TI2,T2=TI1);(T1=T2,T1=TI1)), !.
opSignature((<), temporalunit, [T1,T2],[stream,ubool],[]) :-
  member((TI1,TI2),[(ubool,bool),(uint,int),(ureal,real),(ustring,string)]),
  ((T1=TI1,T2=TI2);(T1=TI2,T2=TI1);(T1=T2,T1=TI1)), !.
opSignature((>), temporalunit, [T1,T2],[stream,ubool],[]) :-
  member((TI1,TI2),[(ubool,bool),(uint,int),(ureal,real),(ustring,string)]),
  ((T1=TI1,T2=TI2);(T1=TI2,T2=TI1);(T1=T2,T1=TI1)), !.
opSignature((<=), temporalunit, [T1,T2],[stream,ubool],[]) :-
  member((TI1,TI2),[(ubool,bool),(uint,int),(ureal,real),(ustring,string)]),
  ((T1=TI1,T2=TI2);(T1=TI2,T2=TI1);(T1=T2,T1=TI1)), !.
opSignature((>=), temporalunit, [T1,T2],[stream,ubool],[]) :-
  member((TI1,TI2),[(ubool,bool),(uint,int),(ureal,real),(ustring,string)]),
  ((T1=TI1,T2=TI2);(T1=TI2,T2=TI1);(T1=T2,T1=TI1)), !.
opSignature(uint2ureal, temporalunit, [uint],ureal,[]).
opSignature(the_unit, temporalunit, [point,point,instant,instant,bool,bool],
        upoint,[]).
opSignature(the_unit, temporalunit, [ipoint,ipoint,bool,bool],upoint,[]).
opSignature(the_unit, temporalunit, [real,real,real,bool,instant,instant,bool,
        bool],ureal,[]).
opSignature(the_unit, temporalunit, [T1,duration,bool,bool],T2,[]) :-
  memberchk((T1,T2),[(ibool,ubool),(iint,uint),(istring,ustring)]), !.
opSignature(the_unit, temporalunit, [T1,instant,instant,bool,bool],T2,[]) :-
  memberchk((T1,T2),[(bool,ubool),(int,uint),(string,ustring)]), !.
opSignature(the_ivalue, temporalunit, [instant,T1],T2,[]) :-
  memberchk((T1,T2),[(bool,ibool),(int,iint),(real,ireal),(string,istring),
                     (point,ipoint),(region,intimeregion)]), !.
opSignature(length, temporalunit, [upoint],real,[]).


/*
2.7.8 MovingRegionAlgebra

*/
opSignature(atinstant, movingregion, [movingregion,instant],intimeregion,[]).
opSignature(atinstant, movingregion, [uregion,instant],intimeregion,[]).
opSignature(initial, movingregion, [movingregion],intimeregion,[]).
opSignature(final, movingregion, [movingregion],intimeregion,[]).
opSignature(inst, movingregion, [intimeregion],instant,[]).
opSignature(val, movingregion, [intimeregion],region,[]).
opSignature(deftime, movingregion, [movingregion],periods,[]).
opSignature(present, movingregion, [movingregion,instant],bool,[bbox(3)]).
opSignature(present, movingregion, [movingregion,periods],bool,[bbox(3)]).
opSignature(intersection, movingregion, [mpoint,movingregion],mpoint,[exp]).
opSignature(inside, movingregion, [mpoint,movingregion],mbool,[exp]).
opSignature(at, movingregion, [mpoint,region],mpoint,[exp]).
opSignature(at, movingregion, [movingregion,point],mpoint,[exp]).
opSignature(bbox, movingregion, [uregion],rect3,[]).
opSignature(bbox, movingregion, [intimeregion],rect3,[]).
opSignature(bbox, movingregion, [movingregion],rect3,[]).
opSignature(bbox2d, uregion, [uregion],rect,[]).
opSignature(bbox2d, intimeregion, [uregion],rect,[]).
opSignature(bbox2d, movingregion, [uregion],rect,[]).
opSignature(move, movingregion, [mpoint,region],movingregion,[exp]).
opSignature(vertextrajectory, movingregion, [uregion],line,[]).
opSignature(vertextrajectory, movingregion, [movingregion],line,[exp]).
opSignature(units, movingregion, [movingregion],[stream,uregion],[]).


/*
2.7.9 RectangleAlgebra

*/
opSignature(isempty, rectangle, [T],bool,[]) :-
  memberchk(T,[rect,rect2,rect3,rect4,rect8]),!.
opSignature((=), rectangle, [T,T],bool,[comm]) :-
  memberchk(T,[rect,rect2,rect3,rect4,rect8]),!.
opSignature((#), rectangle, [T,T],bool,[comm]) :-
  memberchk(T,[rect,rect2,rect3,rect4,rect8]),!.
opSignature(intersects, rectangle, [T,T],bool,[comm]) :-
  memberchk(T,[rect,rect2,rect3,rect4,rect8]),!.
opSignature(inside, rectangle, [T,T],bool,[]) :-
  memberchk(T,[rect,rect2,rect3,rect4,rect8]),!.
opSignature(union, rectangle, [T,T],T,[comm,ass]) :-
  memberchk(T,[rect,rect2,rect3,rect4,rect8]),!.
opSignature(intersection, rectangle, [T,T],T,[comm,ass]) :-
  memberchk(T,[rect,rect2,rect3,rect4,rect8]),!.
opSignature(translate, rectangle, [rect,real,real],rect,[]).
opSignature(translate, rectangle, [rect3,real,real,real],rect3,[]).
opSignature(translate, rectangle, [rect4,real,real,real,real],rect4,[]).
opSignature(distance, rectangle, [T,T],real,[]) :-
  memberchk(T,[rect,rect2,rect3,rect4,rect8]),!.
opSignature(rectangle2, rectangle, [int, int, int, int],rect,[]).
opSignature(rectangle2, rectangle, [real, real, real, real],rect,[]).
opSignature(rectangle3, rectangle, [int, int, int, int, int, int],rect3,[]).
opSignature(rectangle3, rectangle, [real,real,real,real,real,real],rect3,[]).
opSignature(rectangle4, rectangle, [int,int,int,int,int,int,int,int],rect4,[]).
opSignature(rectangle4, rectangle, [real,real,real,real,real,real,real,real],
        rect4,[]).
opSignature(rectangle8, rectangle, [int,int,int,int,int,int,int,int,int,int,int,
        int,int,int,int,int],rect8,[]).
opSignature(rectangle8, rectangle, [real,real,real,real,real,real,real,real,
        real,real,real,real,real,real,real,real],rect8,[]).
opSignature(rectproject, rectangle, [T, int, int],rect,[]) :-
  memberchk(T,[rect,rect2,rect3,rect4,rect8]),!.
opSignature(minD, rectangle, [T,int],real,[]) :-
  memberchk(T,[rect,rect2,rect3,rect4,rect8]),!.
opSignature(maxD, rectangle, [T,int],real,[]) :-
  memberchk(T,[rect,rect2,rect3,rect4,rect8]),!.
opSignature(bbox, rectangle, [T],T,[idem]) :-
  memberchk(T,[rect,rect2,rect3,rect4,rect8]),!.
opSignature(enlargeRect, rectangle, [rect,real,real],rect,[]).
opSignature(enlargeRect, rectangle, [rect3,real,real,real],rect3,[]).
opSignature(enlargeRect, rectangle, [rect4,real,real,real,real],rect4,[]).
opSignature(enlargeRect, rectangle, [rect8,real,real,real,real,
                                     real,real,real,real],rect8,[]).
opSignature(size, rectangle, [T],real,[]) :-
  memberchk(T,[rect,rect3,rect4,rect8]),!.
opSignature(scalerect, rectangle, [T|FactorList],T,[]) :-
  memberchk((T,D),[(rect,2),(rect3,3),(rect4,4),(rect8,8)]),
  length(FactorList,D), ground([T|FactorList]),
  findall(X,member(X,FactorList),[real]),
  !.



/*
2.7.10 StreamAlgebra

*/
opSignature(count, stream, [[stream,T]],int,[block,exp,aggr]) :- isData(T), !.
opSignature(printstream, stream, [[stream,T]],[stream,T],[sidefx]) :-
  (isData(T) ; T = [tuple,_]), !.
opSignature(transformstream,stream,[[stream,T]],[stream,[tuple,[[elem,T]]]],
        [idem]) :- isData(T), !.
opSignature(transformstream, stream,[[stream,[tuple,[[_,T]]]]],[stream,T],
        [idem]) :- isData(T), !.
opSignature(projecttransformstream, stream, [[stream,[tuple,AttrList]],Attr],
  [stream,Type],[]) :- memberchk([Attr,Type],AttrList),!.
opSignature(namedtransformstream, stream, [[stream,T],Attr],
  [stream,[tuple,[[Attr,T]]]],[]) :- isData(T), !.
opSignature(feed, stream,[[rel,[tuple,AttrList]]],[stream,[tuple,AttrList]],[]).
opSignature(feed, stream,[T],[stream,T],[]) :- isData(T),!.
opSignature(use, stream, [[stream,T1],[map, T1, T2]],[stream,T2],[]) :-
  isData(T1),isData(T2),!.
opSignature(use, stream, [[stream,T1],[map, T1, [stream, T2]]],[stream,T2],[]):-
  isData(T1),isData(T2),!.
opSignature(use2,stream,[[stream,T1],[stream,T2],[map,T1,T2,T3]],[stream,T3],[])
        :- isData(T1),isData(T2),isData(T3),!.
opSignature(use2, stream, [[stream,T1],[stream,T2],[map,T1,T2,[stream,T3]]],
        [stream,T3],[]) :-
  isData(T1),isData(T2),isData(T3),!.
opSignature(use2,stream,[[stream,T1],[stream,T2],[map,T1,T2,T3]],[stream,T3],[])
        :-isData(T1),isData(T2),isData(T3),!.
opSignature(use2, stream, [[stream,T1],[stream,T2],[map,T1,T2,[stream,T3]]],
        [stream,T3],[]) :-
  isData(T1),isData(T2),isData(T3),!.
opSignature(use2, stream, [T1,[stream,T2],[map,T1,T2,T3]],[stream,T3],[]) :-
  isData(T1),isData(T2),isData(T3),!.
opSignature(use2,stream,[T1,[stream,T2],[map,T1,T2,[stream,T3]]],[stream,T3],[])
        :- isData(T1),isData(T2),isData(T3),!.
opSignature(use2, stream, [[stream,T1],T2,[map,T1,T2,T3]],[stream,T3],[]) :-
  isData(T1),isData(T2),isData(T3),!.
opSignature(use2,stream,[[stream,T1],T2,[map,T1,T2,[stream,T3]]],[stream,T3],[])
        :- isData(T1),isData(T2),isData(T3),!.
opSignature(aggregateS,stream,[[stream,T],[map,T,T,T],T],T,[block,aggr,exp]) :-
  isData(T),!.
opSignature(filter,stream,[[stream,T],[map,T,bool]],[stream,T],[]):-isData(T),!.
opSignature(ensure, stream, [[stream,T],int],
            bool,[block,aggr,exp]) :- isData(T).
opSignature(echo,stream,[[stream,T],string],[stream,T],[sidefx]):-isData(T),!.
opSignature(echo,stream,[[stream,T],bool,string],[stream,T],[sidefx])
          :-isData(T),!.
opSignature(realstream, stream, [real,real,real],[stream,real],[]).
opSignature(streamelem, stream, [[stream,T]|_],T,[typemapop]) :- !.
opSignature(streamelem, stream, [T|_],T,[typemapop]).
opSignature(streamelem2, stream, [_,[stream,T]|_],T,[typemapop]) :- !.
opSignature(streamelem2, stream, [_,T|_],T,[typemapop]).
opSignature(tail, stream, [[stream,T],int],[stream,T],[block]) :- isData(T).
opSignature(tail, stream,[[stream,[tuple,AttrList]],int],
        [stream,[tuple,AttrList]],[block]).
opSignature(tail, stream, [[stream,T],int,bool],[stream,T],[block]):-isData(T).
opSignature(tail, stream,[[stream,[tuple,AttrList]],int,bool],
        [stream,[tuple,AttrList]],[block]).
opSignature(kinds, stream, [string],[stream,string],[]).


/*
2.7.11 TupleIdentifierAlgebra

*/
opSignature(tupleid, tupleidentifier, [[tuple,_]],tid,[]).
opSignature(addtupleid, tupleidentifier, [[stream,[tuple,AttrList]]],
      [stream,[tuple,AttrList2]],[]) :-
  append(AttrList,[[id,tid]],AttrList2), !.
opSignature((=), tupleidentifier, [tid,tid],bool,[comm]).
opSignature((#), tupleidentifier, [tid,tid],bool,[comm]).
opSignature((<), tupleidentifier, [tid,tid],bool,[]).
opSignature((<=), tupleidentifier, [tid,tid],bool,[]).
opSignature((>), tupleidentifier, [tid,tid],bool,[]).
opSignature((>=), tupleidentifier, [tid,tid],bool,[]).
opSignature(tid2int, tupleidentifier, [tid],int,[]).
opSignature(int2tid, tupleidentifier, [int],tid,[]).


/*
2.7.12 RTreeAlgebra

*/
opSignature(creatertree, rtree, [[rel,[tuple,AttrList]],Key],Result,[block]) :-
  memberchk([Key,KeyType],AttrList),
  isKind(Kind,KeyType),
  memberchk((Kind,RTreeType),[(spatial2d,rtree),(temporal,rtree3),
                              (spatial2d,rtree3),(spatial4d,rtree4),
                              (spatial8d,rtree8)]),
  Result = [RTreeType,[tuple,AttrList],Key,false], !.
opSignature(creatertree, rtree, [[rel,[tuple,AttrList]],Key],Result,[block]) :-
  memberchk([Key,KeyType],AttrList),
  memberchk((KeyType,RTreeType),[(rect,rtree),(rect3,rtree3),(rect4,rtree4),
                                 (rect8,rtree8)]),
  Result = [RTreeType,[tuple,AttrList],Key,false], !.
opSignature(creatertree,rtree,[[stream,[tuple,AttrList]],Key],Result,[block]) :-
  memberchk([Key,KeyType],AttrList),
  isKind(Kind,KeyType),
  memberchk((Kind,RTreeType),[(spatial2d,rtree),(temporal,rtree3),
                              (spatial2d,rtree3),(spatial4d,rtree4),
                              (spatial8d,rtree8)]),
  memberchk([TIDATTR,tid],AttrList),
  delete(AttrList,[TIDATTR,tid],RTreeAttrList),
  Result = [RTreeType,[tuple,RTreeAttrList],Key,false], !.
opSignature(creatertree,rtree,[[stream,[tuple,AttrList]],Key],Result,[block]):-
  memberchk([Key,KeyType],AttrList),
  memberchk((KeyType,RTreeType),[(rect,rtree),(rect3,rtree3),(rect4,rtree4),
                                 (rect8,rtree8)]),
  select([_,tid],AttrList,RTreeAttrList),
  Result = [RTreeType,[tuple,RTreeAttrList],Key,false], !.
opSignature(creatertree,rtree,[[stream,[tuple,AttrList]],Key],Result,[block]):-
  select([low,int],AttrList,RTreeAttrList1),
  select([high,int],RTreeAttrList1,RTreeAttrList2),
  select([_,tid],RTreeAttrList2,RTreeAttrList),
  memberchk([Key,KeyType],RTreeAttrList),
  isKind(Kind,KeyType),
  memberchk((Kind,RTreeType),[(spatial2d,rtree),(temporal,rtree3),
                              (spatial2d,rtree3),(spatial4d,rtree4),
                              (spatial8d,rtree8)]),
  Result = [RTreeType,[tuple,RTreeAttrList],Key,true], !.
opSignature(creatertree,rtree,[[stream,[tuple,AttrList]],Key],Result,[block]) :-
  select([low,int],AttrList,RTreeAttrList1),
  select([high,int],RTreeAttrList1,RTreeAttrList2),
  select([_,tid],RTreeAttrList2,RTreeAttrList),
  memberchk([Key,KeyType],RTreeAttrList),
  memberchk((KeyType,RTreeType),[(rect,rtree),(rect3,rtree3),(rect4,rtree4),
                                 (rect8,rtree8)]),
  Result = [RTreeType,[tuple,RTreeAttrList],Key,true], !.

opSignature(bulkloadrtree,rtree,[[stream,[tuple,AttrList]],Key],Result,
        [block]):-
  memberchk([Key,KeyType],AttrList),
  isKind(Kind,KeyType),
  memberchk((Kind,RTreeType),[(spatial2d,rtree),(temporal,rtree3),
                              (spatial2d,rtree3),(spatial4d,rtree4),
                              (spatial8d,rtree8)]),
  select([_,tid],AttrList,RTreeAttrList),
  Result = [RTreeType,[tuple,RTreeAttrList],Key,false], !.
opSignature(bulkloadrtree, rtree, [[stream,[tuple,AttrList]],Key],Result,
        [block]) :-
  memberchk([Key,KeyType],AttrList),
  memberchk((KeyType,RTreeType),[(rect,rtree),(rect3,rtree3),(rect4,rtree4),
                                 (rect8,rtree8)]),
  select([_,tid],AttrList,RTreeAttrList),
  Result = [RTreeType,[tuple,RTreeAttrList],Key,false], !.
opSignature(bulkloadrtree, rtree, [[stream,[tuple,AttrList]],Key],Result,
        [block]) :-
  select([low,int],AttrList,RTreeAttrList1),
  select([high,int],RTreeAttrList1,RTreeAttrList2),
  select([_,tid],RTreeAttrList2,RTreeAttrList),
  memberchk([Key,KeyType],RTreeAttrList),
  isKind(Kind,KeyType),
  memberchk((Kind,RTreeType),[(spatial2d,rtree),(temporal,rtree3),
                              (spatial2d,rtree3),(spatial4d,rtree4),
                              (spatial8d,rtree8)]),
  Result = [RTreeType,[tuple,RTreeAttrList],Key,true], !.
opSignature(bulkloadrtree, rtree, [[stream,[tuple,AttrList]],Key],Result,
        [block]) :-
  select([low,int],AttrList,RTreeAttrList1),
  select([high,int],RTreeAttrList1,RTreeAttrList2),
  select([_,tid],RTreeAttrList2,RTreeAttrList),
  memberchk([Key,KeyType],RTreeAttrList),
  memberchk((KeyType,RTreeType),[(rect,rtree),(rect3,rtree3),(rect4,rtree4),
                                 (rect8,rtree8)]),
  Result = [RTreeType,[tuple,RTreeAttrList],Key,true], !.
opSignature(windowintersects, rtree, [RTree,
    [rel,[tuple,TupleType]],QueryType],[stream,[tuple,TupleType]],[]) :-
  RTree = [RTreeType,[tuple,TupleType],Key,false],
  memberchk([Key,KeyType],TupleType),
  memberchk((Kind,RTreeType),[(spatial2d,rtree),(temporal,rtree3),
                              (spatial2d,rtree3),(spatial4d,rtree4),
                              (spatial8d,rtree8)]),
  isKind(KeyType,Kind),isKind(QueryType,Kind),!.
opSignature(windowintersectsS, rtree, [RTree,QueryType],
        [stream,[tuple,[[id,tid]]]],[]) :-
  RTree = [RTreeType,[tuple,TupleType],Key,false],
  memberchk([Key,KeyType],TupleType),
  memberchk((Kind,RTreeType),[(spatial2d,rtree),(temporal,rtree3),
                              (spatial2d,rtree3),(spatial4d,rtree4),
                              (spatial8d,rtree8)]),
  isKind(KeyType,Kind),
  isKind(QueryType,Kind),
  !.
opSignature(gettuples, rtree, [[stream,[tuple,AttrList1]],
     [rel,[tuple,AttrList2]]],[stream,[tuple,ResAttrList]],[]) :-
  select([_,tid],AttrList1,AttrList3),
  append(AttrList3,AttrList2,ResAttrList), !.
opSignature(gettuplesdbl, rtree, [[stream,[tuple,AttrList1]],
     [rel,[tuple,AttrList2]],AttrRestr],[stream,[tuple,ResAttrList]],[]) :-
  select([id,tid],AttrList1,AttrList3),
  select([low,int],AttrList3,AttrList4),
  select([high,int],AttrList4,AttrList5),
  memberchk([AttrRestr,_],AttrList2),
  append(AttrList5,AttrList2,ResAttrList), !.
opSignature(gettuples2, rtree, [[stream,[tuple,AttrList1]],
       [rel,[tuple,AttrList2]],TIDAttrName],[stream,[tuple,ResAttrList]],[]) :-
  select([TIDAttrName,tid],AttrList1,AttrList3),
  append(AttrList3,AttrList2,ResAttrList), !.
opSignature(nodes, rtree, [RTree],[stream,[tuple,[[level,int],[nodeid,int],
                                                [mbr,MBRType],[fatherid,int],
                                                [isleaf,bool],[minentries,int],
                                                [maxentries,int],
                                                [countentries,int]]]],[]) :-
  RTree = [RTreeType,[tuple,AttrList],KeyAttr,_],
  memberchk((MBRType,RTreeType),[[rect,rtree],[rect3,rtree3],[rect4,rtree4],
                                 [rect8,rtree8]]),
  memberchk([KeyAttr,MBRType],AttrList), !.
opSignature(treeheight, rtree, [[RTreeType|_]],int,[]) :-
  memberchk(RTreeType,[rtree,rtree3,rtree4,rtree8]), !.
opSignature(no_nodes, rtree, [[RTreeType|_]],int,[]) :-
  memberchk(RTreeType,[rtree,rtree3,rtree4,rtree8]), !.
opSignature(no_entries, rtree, [[RTreeType|_]],int,[]) :-
  memberchk(RTreeType,[rtree,rtree3,rtree4,rtree8]), !.
opSignature(bbox, rtree, [[RTreeType|_]],MBRType,[]) :-
  memberchk((MBRType,RTreeType),[(rect,rtree),(rect3,rtree3),(rect4,rtree4),
                                 (rect8,rtree8)]), !.
opSignature(entries, rtree, [[RTreeType|_]],
        [stream,[tuple,[[nodeid,int],[mbr,MBRType],[tupleid,tid]]]],[]) :-
  memberchk((MBRType,RTreeType),[(rect,rtree),(rect3,rtree3),(rect4,rtree4),
                                 (rect8,rtree8)]),!.
opSignature(getFileInfo, rtree, [[RTreeType|_]],text,[exp]) :-
  memberchk(RTreeType,[rtree,rtree3,rtree4,rtree8]), !.


/*
2.7.13 FunctionAlgebra

*/
opSignature(any, function, [T|_],T,[typemapop]).
opSignature(any2, function, [_, T |_],T,[typemapop]).
opSignature(within, function, [T1,[map,T1,[stream,T2]]],[stream,T2],[]).
opSignature(within, function, [T1,[map,T1,T2]],T2,[]).
opSignature(within2, function, [T1,T2,[map,T1,T2,[stream,T3]]],[stream,T3],[]).
opSignature(within2, function, [T1,T2,[map,T1,T2,T3]],T3,[]).
opSignature(whiledo, function, [T,[map,T,bool],[map,T,T],bool],[stream,T],[]) :-
  isData(T),!.


/*
2.7.14 CollectionAlgebra

*/
opSignature(contains, collection, [[vector,T],T],bool,[exp]) :- isData(T), !.
opSignature(contains, collection, [[set,T],T],bool,[exp]) :- isData(T), !.
opSignature(contains, collection, [[multiset,T],T],int,[exp]) :- isData(T), !.
opSignature(in, collection, [T,[vector,T]],bool,[exp]) :- isData(T), !.
opSignature(in, collection, [T,[set,T]],bool,[exp]) :- isData(T), !.
opSignature(in, collection, [T,[multiset,T]],bool,[exp]) :- isData(T), !.
opSignature(insert, collection, [[set,T],T],[set,T],[exp]) :- isData(T), !.
opSignature(insert, collection, [[multiset,T],T],[set,T],[exp]) :- isData(T), !.
opSignature((+), collection, [[vector,T],T],[vector,T],[exp]) :- isData(T), !.
opSignature(collect_set, collection, [[stream,T]],[set,T],[block,aggr,exp]) :-
  isData(T), !.
opSignature(collect_multiset,collection,[[stream,T]],
            [multiset,T],[block,aggr,exp])
        :-isData(T),!.
opSignature(collect_vector,collection,[[stream,T]],[vector,T],[block,aggr,exp])
        :-isData(T), !.
opSignature(components, collection, [[vector,T]],[stream,T],[]) :- isData(T), !.
opSignature(components, collection, [[set,T]],[stream,T],[]) :- isData(T), !.
opSignature(components, collection, [[multiset,T]],[stream,T],[]):-isData(T), !.
opSignature(get, collection, [[vector,T],int],T,[exp]) :- isData(T), !.
opSignature(deleteelem, collection, [[set,T],T],[set,T],[exp]) :- isData(T), !.
opSignature(deleteelem,collection,[[multiset,T],T],[multiset,T],[exp])
        :- isData(T),!.
opSignature(concat,collection,[[vector,T],[vector,T]],[vector,T],[ass,exp])
        :- isData(T),!.
opSignature(union, collection, [[set,T],[set,T]],[set,T],[comm,ass,exp])
        :- isData(T),!.
opSignature(union, collection, [[multiset,T],[multiset,T]],[multiset,T],
        [comm,ass,exp]) :- isData(T),!.
opSignature(intersection,collection,[[set,T],[set,T]],[set,T],[comm,ass,exp])
        :- isData(T),!.
opSignature(intersection,collection,[[multiset,T],[multiset,T]],[multiset,T],
        [comm,ass,exp]) :- isData(T),!.
opSignature(difference, collection, [[set,T],[set,T]],
            [set,T],[exp]):- isData(T),!.
opSignature(difference, collection, [[multiset,T],
              [multiset,T]],[multiset,T],[exp])
        :- isData(T),!.
opSignature(size, collection, [[set,T]],int,[]) :- isData(T),!.
opSignature(size, collection, [[multiset,T]],int,[]) :- isData(T),!.
opSignature(size, collection, [[vector,T]],int,[]) :- isData(T),!.
opSignature((=), collection, [[set,T],[set,T]],bool,[comm]) :- isData(T),!.
opSignature((=), collection, [[multiset,T],[multiset,T]],bool,[comm])
        :- isData(T),!.
opSignature((>), collection, [[set,T],[set,T]],bool,[]) :- isData(T),!.
opSignature((>), collection, [[multiset,T],[multiset,T]],bool,[]):-isData(T),!.
opSignature((<), collection, [[set,T],[set,T]],bool,[]) :- isData(T),!.
opSignature((<), collection, [[multiset,T],[multiset,T]],bool,[]):-isData(T),!.
opSignature((>=), collection, [[set,T],[set,T]],bool,[]) :- isData(T),!.
opSignature((>=), collection, [[multiset,T],[multiset,T]],bool,[]):-isData(T),!.
opSignature((<=), collection, [[set,T],[set,T]],bool,[]) :- isData(T),!.
opSignature((<=), collection, [[multiset,T],[multiset,T]],bool,[]):-isData(T),!.
opSignature(is_defined, collection, [[set,T]],bool,[]) :- isData(T),!.
opSignature(is_defined, collection, [[multiset,T]],bool,[]) :- isData(T),!.
opSignature(is_defined, collection, [[vector,T]],bool,[]) :- isData(T),!.


/*
2.7.15 GSLAlgebra

*/
opSignature(rng_init, gsl, [inbt,int],bool,[sidefx]).
opSignature(rng_int, gsl, [],int,[sidefx]).
opSignature(rng_intN, gsl, [int],int,[sidefx]).
opSignature(rng_getMin, gsl, [],int,[]).
opSignature(rng_getMax, gsl, [],int,[]).
opSignature(rng_real, gsl, [],real,[sidefx]).
opSignature(rng_realpos, gsl, [],real,[sidefx]).
opSignature(rng_setSeed, gsl, [int],bool,[sidefx]).
opSignature(rng_getSeed, gsl, [],int,[]).
opSignature(rng_getType, gsl, [],int,[]).
opSignature(rng_flat, gsl, [real,real],real,[sidefx]).
opSignature(rng_gaussian, gsl, [real],real,[sidefx]).
opSignature(rng_binomial, gsl, [int,real],int,[sidefx]).
opSignature(rng_geometric, gsl, [real,real],int,[sidefx]).
opSignature(rng_exponential, gsl, [real],real,[sidefx]).
opSignature(rng_poisson, gsl, [real],int,[sidefx]).
opSignature(rng_NoGenerators, gsl, [],int,[]).
opSignature(rng_GeneratorName, gsl, [int],string,[]).
opSignature(rng_GeneratorMinRand, gsl, [int],int,[]).
opSignature(rng_GeneratorMaxRand, gsl, [int],int,[]).


/*
2.7.16 SimulationAlgebra

*/
opSignature(sim_set_rng, simulation, [int,int],bool,[sidefx]).
opSignature(sim_set_event_params, simulation, [real,real,real,real], bool,
        [sidefx]).
opSignature(sim_set_dest_params, simulation, [real,real,real,real,real,real,
        real,real,real,real,real,real,real,real],bool,[sidefx]).
opSignature(sim_create_trip, simulation, [[stream,[tuple,AttrList]],A1,A2,
      instant,point],mpoint,[block,sidefx]) :-
  memberchk([A1,line],AttrList),member([A2,real],AttrList),!.
opSignature(sim_create_trip, simulation, [[stream,[tuple,AttrList]],A1,A2,
      instant,point,real],mpoint,[block,sidefx]) :-
  memberchk([A1,line],AttrList),member([A2,real],AttrList),!.
opSignature(sim_print_params, simulation, [],bool,[sidefx]).
opSignature(sim_fillup_mpoint, simulation, [mpoint,instant,instant,bool,bool,
                                            bool],mpoint,[]).
opSignature(sim_trips, simulation, [mpoint,duration],[stream,mpoint],[]).
opSignature(sim_trips, simulation, [mpoint,duration,real],[stream,mpoint],[]).


/*
2.7.17 ArrayAlgebra

*/
opSignature(size, array, [[array,_]],int,[]).
opSignature(get, array, [[array,T]],T,[]).
opSignature(put, array, [[array,T],T,int],[array,T],[]).
opSignature(makearray, array, [TList],[array,T],[]) :-
  is_list(TList),my_list_to_set(TList,[T]), !.
opSignature(makearrayN, array, [T,int],[array,T],[]).
opSignature(sortarray, array, [[array,T],[map,T,int]],[array,T],[]).
opSignature(tie, array, [[array,T],[map,T,T,T]],T,[]).
opSignature(cumulate, array, [[array,T],[map,T,T,T]],[array,T],[]).
opSignature(distribute, array, [[stream,[tuple,AttrList]],Key],
            [array,[rel,[tuple,AttrList2]]],[block]) :-
  select([Key,int],AttrList,AttrList2), !.
opSignature(summarize, array, [[array,[rel,Tuple]]],[stream,Tuple],[]).
opSignature(loop, array, [[array,T],[map,T,R]],array(R),[]).
opSignature(loopa, array, [[array,T],[array,U],[map,T,U,R]],[array,R],[]).
opSignature(loopb, array, [[array,T],[array,U],[map,T,U,R]],[array,R],[]).
opSignature(loopswitch, array, [[array,T],FunList],[array,R],[]) :-
  is_list(FunList),
  setof(Sig,member([_,Sig],FunList),[[map,T,R]]), !.
opSignature(loopswitcha, array, [[array,T],[array,U],FunList],[array,R],[]) :-
  ground(FunList),is_list(FunList),
  setof(Sig,member([_,Sig],FunList),[[map,T,U,R]]), !.
opSignature(loopswitchb, array, [[array,T],[array,U],FunList],[array,R],[]) :-
  ground(FunList),is_list(FunList),
  setof(Sig,member([_,Sig],FunList),[[map,T,U,R]]), !.
opSignature(loopselect, array, [[array,T],FunList,int,real],[array,R],[]) :-
  ground(FunList),is_list(FunList),
  setof(Sig,member([_,Sig],FunList),[[map,T,R]]), !.
opSignature(loopselecta,array,[[array,T],[array,U],FunList,int,real],
        [array,R],[]):-
  ground(FunList),is_list(FunList),
  setof(Sig,member([_,Sig],FunList),[[map,T,U,R]]), !.
opSignature(loopselectb,array,[[array,T],[array,U],FunList,int,real],
        [array,R],[]):-
  ground(FunList),is_list(FunList),
  setof(Sig,member([_,Sig],FunList),[[map,T,U,R]]), !.
opSignature(partjoin, array, [[array,[rel,T]],[array,[rel,U]],
                              [map,[rel,T],[rel,U],R]],[array,R],[]).
opSignature(partjoinswitch, array,[[array,[rel,T]], [array,[rel,U]],
                                   FunList],[array,R],[]):-
  ground(FunList),is_list(FunList),
  setof(Sig,member([_,Sig],FunList),[[map,[rel,T],[rel,U],R]]), !.
opSignature(partjoinselect, array,[[array,[rel,T]], [array,[rel,U]],
                                   FunList,int,real],[array,R],[]):-
  ground(FunList),is_list(FunList),
  setof(Sig,member([_,Sig],FunList),[[map,[rel,T],[rel,U],R]]), !.
opSignature(element, array, [[array, T]|_],T,[typemapop]).
opSignature(element2, array, [[array, _],[array, T]|_],T,[typemapop]).


/*
2.7.18 BinaryFileAlgebra

*/
opSignature(saveto, binaryfile, [binfile,string],bool,[sidefx]).

/*
2.7.19 BTreeAlgebra

*/
opSignature(createbtree, btree, [[rel,[tuple,AttrList]],Key],
                           [btree,[tuple,AttrList],KeyType],[block,exp]) :-
  memberchk([Key,KeyType],AttrList),
  (memberchk(KeyType,[int,real,text]);isKind(KeyType,indexable)),!.
opSignature(createbtree, btree, [[stream,[tuple,AttrList]],Key],
                            [btree,[tuple,AttrList2],KeyType],[block,exp]) :-
  select([_,tid],AttrList,AttrList2),
  memberchk([Key,KeyType],AttrList2),
  (memberchk(KeyType,[int,real,text]);isKind(KeyType,indexable)),!.
opSignature(exactmatch, btree, [[btree,[tuple,AttrList],KeyType],
      [rel,[tuple,AttrList]],Key],[stream,[tuple,AttrList]],[]) :-
  memberchk([Key,KeyType],AttrList), !.
opSignature(leftrange, btree, [[btree,[tuple,AttrList],KeyType],
      [rel,[tuple,AttrList]],Key],[stream,[tuple,AttrList]],[]) :-
  memberchk([Key,KeyType],AttrList), !.
opSignature(rightrange, btree, [[btree,[tuple,AttrList],KeyType],
      [rel,[tuple,AttrList]],Key],[stream,[tuple,AttrList]],[]) :-
  memberchk([Key,KeyType],AttrList), !.
opSignature(range, btree, [[btree,[tuple,AttrList],KeyType],
      [rel,[tuple,AttrList]],Key1,Key2],[stream,[tuple,AttrList]],[]) :-
  memberchk([Key1,KeyType],AttrList), memberchk([Key2,KeyType],AttrList),!.
opSignature(exactmatchS, btree, [[btree,[tuple,AttrList],KeyType],Key],
        [stream,[tuple,[[id,tid]]]],[]) :-
  memberchk([Key,KeyType],AttrList), !.
opSignature(leftrangeS, btree, [[btree,[tuple,AttrList],KeyType],Key],
        [stream,[tuple,[[id,tid]]]],[]) :-
  memberchk([Key,KeyType],AttrList), !.
opSignature(rightrangeS, btree, [[btree,[tuple,AttrList],KeyType],Key],
        [stream,[tuple,[[id,tid]]]],[]) :-
  memberchk([Key,KeyType],AttrList), !.
opSignature(rangeS, btree, [[btree,[tuple,AttrList],KeyType],Key1,Key2],
        [stream,[tuple,[[id,tid]]]],[]) :-
  memberchk([Key1,KeyType],AttrList), memberchk([Key2,KeyType],AttrList), !.
opSignature(insertbtree, btree, [[stream,[tuple,AttrList1]],
                                 [btree,[tuple,AttrList2],KeyType],Key],
                                 [stream,[tuple,AttrList1]],[sidefx]) :-
  select([_,tid],AttrList1,AttrList2), memberchk([Key,KeyType],AttrList1), !.
opSignature(deletebtree, btree, [[stream,[tuple,AttrList1]],
                                 [btree,[tuple,AttrList2],KeyType],Key],
                                 [stream,[tuple,AttrList1]],[sidefx]) :-
  select([_,tid],AttrList1,AttrList2), memberchk([Key,KeyType],AttrList1), !.
opSignature(updatebtree, btree, [[stream,[tuple,AttrList1]],
                                 [btree,[tuple,AttrList2],KeyType],Key],
                                 [stream,[tuple,AttrList1]],[sidefx]) :-
  select([_,tid],AttrList1,AttrList2), memberchk([Key,KeyType],AttrList1), !.
opSignature(getFileInfo, btree, [[btree,_,_]],text,[]).

/*
2.7.20 RelationAlgebra

*/
opSignature(feed, relation, [[rel,[tuple,X]]],[stream,[tuple,X]],[]).
opSignature(consume, relation, [[stream,[tuple,X]]],[rel,[tuple,X]],[block]).
opSignature(tuple, relation, [[stream,[tuple,X]]|_],[tuple,X],[typemapop]).
opSignature(tuple, relation, [[rel,[tuple,X]]|_],[tuple,X],[typemapop]).
opSignature(tuple2, relation, [[stream,[tuple,_]],[stream,[tuple,X]]|_],
                                 [tuple,X],[typemapop]).
opSignature(tuple2, relation, [[rel,[tuple,_]],[rel,[tuple,X]]|_],
                                [tuple,X],[typemapop]).
opSignature(attr, relation, [[tuple,AttrList],Attr],T,[]) :-
  memberchk([Attr,T],AttrList),!.
opSignature(filter, relation, [[stream,[tuple,X]],[map,[tuple,X],bool]],
                    [stream,[tuple,X]],[]).
opSignature(project, relation, [[stream,[tuple,AttrList1]],ProjList],
          [stream,[tuple,AttrList2]],[]) :-
  is_list(ProjList), is_list(AttrList1),
  ground([[stream,[tuple,AttrList1]],ProjList]),
  findall([A,T],(member([A,T],AttrList1),member(A,ProjList)),AttrList2),!.
opSignature(remove, relation, [[stream,[tuple,AttrList1]],RemList],
          [stream,[tuple,AttrList2]],[]) :-
  is_list(RemList), is_list(AttrList1),
  ground([[stream,[tuple,AttrList1]],RemList]),
  findall([A,T],(member([A,T],AttrList1),not(member(A,RemList))),AttrList2),!.
opSignature(product, relation, [[stream,[tuple,S1]],[stream,[tuple,S2]]],
        [stream,[tuple,S3]],[join,block]) :-
  append(S1,S2,S3), !. % not checking for uniqueness of attrnames!
opSignature(count, relation, [[rel,[tuple,_]]],int,[]).
opSignature(count, relation, [[stream,[tuple,_]]],int,[block,aggr]).
opSignature(count2, relation, [[rel,[tuple,_]]],int,[]).
opSignature(count2, relation, [[stream,[tuple,_]]],int,[block,aggr]).
opSignature(roottuplesize, relation, [[rel,[tuple,_]]],int,[]).
opSignature(roottuplesize, relation, [[stream,[tuple,_]]],int,[block,aggr]).
opSignature(exttuplesize, relation, [[rel,[tuple,_]]],real,[]).
opSignature(exttuplesize, relation, [[stream,[tuple,_]]],real,[block,aggr]).
opSignature(tuplesize, relation, [[rel,[tuple,_]]],real,[]).
opSignature(tuplesize, relation, [[stream,[tuple,_]]],real,[block,aggr]).
opSignature(rootattrsize, relation, [[rel,[tuple,AttrList]],Attr],int,[]) :-
  memberchk([Attr,_],AttrList), !.
opSignature(rootattrsize, relation, [[stream,[tuple,AttrList]],Attr],int,
        [block,aggr]) :- memberchk([Attr,_],AttrList), !.
opSignature(extattrsize, relation, [[rel,[tuple,AttrList]],Attr],real,[]) :-
  memberchk([Attr,_],AttrList), !.
opSignature(extattrsize, relation, [[stream,[tuple,AttrList]],Attr],real,
        [block,aggr]) :- memberchk([Attr,_],AttrList), !.
opSignature(attrsize, relation, [[rel,[tuple,AttrList]],Attr],real,[]) :-
  memberchk([Attr,_],AttrList), !.
opSignature(attrsize, relation, [[stream,[tuple,AttrList]],Attr],real,
        [block,aggr]) :- memberchk([Attr,_],AttrList), !.
opSignature(rename, relation, [[stream,[tuple,A]],P],[stream,[tuple,R]],[]) :-
  ground([[stream,[tuple,A]],P]),
  findall([AR,T],(member([AO,T],A),my_concat_atom([AO,P],'_',AR)),R),!.
opSignature((!), relation, [[stream,[tuple,X]]],[stream,[tuple,X]],[]).
opSignature((!), relation, [[stream,T]],[stream,T],[]) :- isData(T),!.
opSignature(getFileInfo, relation, [[rel,[tuple,_]]],text,[]).
opSignature(getFileInfo, relation, [[trel,[tuple,_]]],text,[]).
opSignature(sizecounters, relation, [[stream,[tuple,X]],string],
        [stream,[tuple,X]],[]).
opSignature(dumpstream, relation, [[stream,[tuple,X]],string,string],
        [stream,[tuple,X]],[sidefx]).
opSignature(reduce, relation, [[stream,[tuple,X]],[map,[stream,[tuple,X]],bool],
        int],[stream,[tuple,X]],[]).
opSignature(tconsume, relation, [[stream,[tuple,X]]],[trel,[tuple,X]],[block]).
opSignature(countboth, relation, [[stream,[tuple,_]],[stream,[tuple,_]]],int,
        [block,exp]).


/*
2.7.21 ExtRelationAlgebra

*/
opSignature(sample, extrelation, [[rel,[tuple,X]],int,real],[stream,[tuple,X]],
        [sidefx]).
opSignature(sample, extrelation, [[rel,[tuple,X]],int,real,int],
        [stream,[tuple,X]],[sidefx]).
opSignature(group, extrelation, [[stream, X]],[rel,X],[typemapop]).
opSignature(cancel, extrelation, [[stream,X],[map,X,bool]],[stream,X],[]).
opSignature(extract, extrelation, 
           [[stream,[tuple,AL]],Attr],AType,[aggr,exp]) :-
  (not(optimizerOption(determinePredSig)); is_list(AL)),
  memberchk([Attr,AType],AL),!.

opSignature(extend,extrelation,[[stream,[tuple,AL]],ExtL],[stream,[tuple,RL]],
        []):-
  is_list(ExtL), ground([[stream,[tuple,AL]],ExtL]),
  findall([EN,ET],(member([EN,[map,[tuple,AL],ET]],ExtL)),EL),
  append(AL,EL,RL), !.
opSignature(concat, extrelation, [[stream,[tuple,X]],[stream,[tuple,X]]],
            [stream,[tuple,X]],[]).
opSignature(min, extrelation, [[stream,[tuple,AL]],Attr],
            Type,[block,aggr,exp]) :-
  memberchk([Attr,Type],AL),!.
opSignature(max, extrelation, [[stream,[tuple,AL]],Attr],
            Type,[block,aggr,exp]) :-
  memberchk([Attr,Type],AL),!.
opSignature(avg, extrelation, [[stream,[tuple,AL]],Attr],
            real,[block,aggr,exp]) :-
  memberchk([Attr,Type],AL),memberchk(Type,[int,real]),!.
opSignature(sum, extrelation, [[stream,[tuple,AL]],Attr],
            Type,[block,aggr,exp]) :-
  memberchk([Attr,Type],AL),memberchk(Type,[int,real]),!.
opSignature(var, extrelation, [[stream,[tuple,AL]],Attr],
           real,[block,aggr,exp]) :-
  memberchk([Attr,Type],AL),memberchk(Type,[int,real]),!.
opSignature(stats, extrelation, [[stream,[tuple,AL]],A1,A2],[stream,[tuple,
    [(countx,int),(minx,real),(maxx,real),(sumx,real),(avgx,real),(varx,real),
     (county,int),(miny,real),(maxy,real),(sumy,real),(avgy,real),(vary,real),
     (count,int),(countxy,int),(covxy,real),(corrxy,real)]]],[block]) :-
  memberchk([A1,T1],AL),memberchk(T1,[int,real]),
  memberchk([A2,T2],AL),memberchk(T2,[int,real]),!.
opSignature(head, extrelation, [[stream,[tuple,X]],int],[stream,[tuple,X]],[]).
opSignature(head, extrelation, [[stream,T],int],[stream,T],[]) :- isData(T),!.
opSignature(sortby, extrelation, [[stream,[tuple,X]],_],[stream,[tuple,X]],
        [block]).
opSignature(sort, extrelation, [[stream,[tuple,X]]],[stream,[tuple,X]],[block]).
opSignature(rdup, extrelation, [[stream,[tuple,X]]],[stream,[tuple,X]],[]).
opSignature(mergesec, extrelation, [[stream,[tuple,X]],[stream,[tuple,X]]],
        [stream,[tuple,X]],[]).
opSignature(mergediff, extrelation, [[stream,[tuple,X]],[stream,[tuple,X]]],
        [stream,[tuple,X]],[]).
opSignature(mergeunion, extrelation, [[stream,[tuple,X]],[stream,[tuple,X]]],
        [stream,[tuple,X]],[]).
opSignature(mergejoin,extrelation,[[stream,[tuple,X]],[stream,[tuple,Y]],XA,YA],
        [stream,[tuple,R]],[join]) :-
  memberchk([XA,T],X),memberchk([YA,T],X),append(X,Y,R), !.
opSignature(sortmergejoin,extrelation,[[stream,[tuple,X]],
        [stream,[tuple,Y]],XA,YA],[stream,[tuple,R]],[join,block]) :-
  memberchk([XA,T],X),memberchk([YA,T],X),append(X,Y,R), !.
opSignature(hashjoin,extrelation,[[stream,[tuple,X]],
        [stream,[tuple,Y]],XA,YA,int],[stream,[tuple,R]],[join,block]) :-
  memberchk([XA,T],X),memberchk([YA,T],X),append(X,Y,R), !.
opSignature(loopjoin, extrelation, [[stream,[tuple,X]],
        [map,[tuple,X],[stream,[tuple,Y]]]],[stream,[tuple,R]],[join]) :-
  append(X,Y,R), !.
opSignature(extendstream, extrelation, [[stream,[tuple,X]],
        [Name,[map,[tuple,X],[stream,Y]]]],[stream,[tuple,R]],[]) :-
  append(X,[[Name,Y]],R),!.
opSignature(extendstream,extrelation,[[stream,[tuple,A]],FL],
        [stream,[tuple,R]],[]):-
  is_list(FL), ground([[stream,[tuple,A]],FL]),
  findall([EA,ET],(member([EA,[map,[tuple,A],[stream,ET]]],FL)),EAL),
  append(A,EAL,R), !.
opSignature(projectextendstream, extrelation, [[stream,[tuple,A]],PL,F],
                                               [stream,[tuple,R]],[]):-
  is_list(PL),
  F = [Name,[map,[tuple,A],[stream,ET]]],
  ground([[stream,[tuple,A]],PL,F]),
  findall([PA,PT],(member([PA,PT],A),member(PA,PL)),R1),
  append(R1,[Name,ET],R), !.
opSignature(groupby,extrelation,[[stream,[tuple,A]],GL,FL],[stream,[tuple,R]],
        []):-
  is_list(GL),is_list(FL),
  ground([[stream,[tuple,A]],GL,FL]),
  findall([GA,GT],(member([GA,GT],A),member(GA,GL)),R1),
  findall([EA,ET],(member([EA,[map,[rel,[tuple,A]],ET]],FL)),R2),
  append(R1,R2,R), !.
opSignature(aggregate, extrelation,[[stream,[tuple,A]],Attr,[map,T,T,T],T],T,
        [block,aggr,exp]):-
  memberchk([Attr,T],A),!.
opSignature(aggregateB,extrelation,[[stream,[tuple,A]],Attr,[map,T,T,T],T],T,
        [block,aggr,exp]):-
  memberchk([Attr,T],A),!.
opSignature(symmjoin, extrelation, [[stream,[tuple,A1]],[stream,[tuple,A2]],
        [map,[tuple,A1],[tuple,A2],bool]],[stream,[tuple,R]],[join]) :-
  append(A1,A2,R), !.
opSignature(symmproductextend, extrelation, [[stream,[tuple,A1]],
        [stream,[tuple,A2]],ExtL],[stream,[tuple,R]],[join]) :-
  is_list(ExtL), ground([[stream,[tuple,A1]],[stream,[tuple,A2]],ExtL]),
  findall([EN,ET],(member([EN,[map,[tuple,A1],[tuple,A2],ET]],ExtL)),EL),
  append(A1,A2,R1),
  append(R1,EL,R), !.
opSignature(symmproduct, extrelation, [[stream,[tuple,A1]],
        [stream,[tuple,A2]]],[stream,[tuple,R]],[join]) :-
  append(A1,A2,R), !.
opSignature(projectextend, extrelation, [[stream,[tuple,A]],PL,FL],
                                               [stream,[tuple,R]],[]) :-
  is_list(PL),is_list(FL),
  ground([[stream,[tuple,A]],PL,FL]),
  findall([PA,PT],(member([PA,PT],A),member(PA,PL)),R1),
  findall([EA,ET],member([EA,[map,[tuple,A],ET]],FL),R2),
  append(R1,R2,R), !.
opSignature(krdup, extrelation,[[stream,[tuple,A]]|UList],[stream,[tuple,A]],
        []) :-
  not(UList = []), ground([[stream,[tuple,A]]|UList]),
  findall(RA,(member(RA,UList),not(memberchk([RA,_],A))),[]), !.
opSignature(addcounter,extrelation,[[stream,[tuple,A]],N,int],
        [stream,[tuple,R]],[]) :-
  not(memberchk([N,_],A)),
  append(A,[],R), !.
opSignature(ksmallest, extrelation, [[stream,[tuple,A]],int,Attr],
        [stream,[tuple,A]],[block]) :-
  memberchk([Attr,_],A), !.
opSignature(kbiggest, extrelation, [[stream,[tuple,A]],int,Attr],
        [stream,[tuple,A]],[block]) :-
  memberchk([Attr,_],A), !.


/*
2.7.22 PlugJoinAlgebra

*/
opSignature(spatialjoin, plugjoin, [[stream,[tuple,X]],
                                   [stream,[tuple,Y]],AX,AY],
        [stream,[tuple,R]],[join, block]) :-
  memberchk([AX,_],X), memberchk([AY,_],Y), append(X,Y,R), !. % no type check



/*
2.7.23 PlaneSweepAlgebra

*/
opSignature(realm, planesweep, [line,line],line,[exp]).
opSignature(realm, planesweep, [line,region],region,[]).
opSignature(realm, planesweep, [region,line],region,[]).
opSignature(realm, planesweep, [region,region],region,[exp]).
opSignature(intersection_new, planesweep, [line,line],line,[comm,ass,exp]).
opSignature(intersection_new, planesweep, [line,region],line,[comm,ass,exp]).
opSignature(intersection_new, planesweep, [region,line],line,[comm,ass,exp]).
opSignature(intersection_new, planesweep, [region,region],region,[exp]).
opSignature(union_new, planesweep, [line,line],line,[comm,ass,exp]).
opSignature(union_new, planesweep, [line,region],region,[comm,ass]).
opSignature(union_new, planesweep, [region,line],region,[comm,ass]).
opSignature(union_new, planesweep, [region,region],region,[comm,ass,exp]).
opSignature(minus_new, planesweep, [line,line],line,[exp]).
opSignature(minus_new, planesweep, [line,region],line,[]).
opSignature(minus_new, planesweep, [region,line],region,[]).
opSignature(minus_new, planesweep, [region,region],region,[exp]).
opSignature(p_intersects, planesweep, [line,line],bool,[comm,bbox(2),exp]).
opSignature(p_intersects, planesweep, [line,region],bool,[comm,bbox(2),exp]).
opSignature(p_intersects, planesweep, [region,line],bool,[comm,bbox(2),exp]).
opSignature(p_intersects, planesweep, [region,region],bool,[comm,bbox(2),exp]).
opSignature(intersects_new, planesweep, [line,line],bool,[comm,bbox(2),exp]).
opSignature(intersects_new, planesweep, [line,region],bool,[comm,bbox(2),exp]).
opSignature(intersects_new, planesweep, [region,line],bool,[comm,bbox(2),exp]).
opSignature(intersects_new, planesweep, [region,region],
            bool,[comm,bbox(2),exp]).


/*
2.7.24 GraphAlgebra

*/
opSignature(thevertex, graph, [graph,int],vertex,[]).
opSignature(maxdegree, graph, [graph,bool],int,[]).
opSignature(mindegree, graph, [graph,bool],int,[]).
opSignature(circle, graph, [graph,vertex,float],graph,[exp]).
opSignature(connectedcomponents, graph, [graph],
        [stream,[tuple,[[graph,graph]]]],[]).
opSignature(shortestpath, graph, [graph,vertex,vertex],path,[exp]).
opSignature(shortestpath, graph, [graph,vertex,int],path,[exp]).
opSignature(shortestpath, graph, [graph,int,vertex],path,[exp]).
opSignature(shortestpath, graph, [graph,int,int],path,[exp]).
opSignature(edges, graph, [path],[stream,[tuple,[[edge,edge]]]],[]).
opSignature(edges, graph, [graph],[stream,[tuple,[[edge,edge]]]],[]).
opSignature(vertices, graph, [path],[stream,[tuple,[[vertex,vertex]]]],[]).
opSignature(vertices, graph, [graph],[stream,[tuple,[[vertex,vertex]]]],[]).
opSignature(partof, graph, [graph,graph],bool,[exp]).
opSignature(key, graph, [vertex],int,[]).
opSignature(pos, graph, [vertex],point,[]).
opSignature(source, graph, [edge],int,[]).
opSignature(target, graph, [edge],int,[]).
opSignature(cost, graph, [edge],real,[]).
opSignature(placenodes, graph, [graph],graph,[exp]).
opSignature(merge, graph, [graph,graph],graph,[exp]).
opSignature(constgraph, graph, [[stream,[tuple,X]],A1,A2,
        [map,[tuple,X],real]],graph,[block,aggr,exp]) :-
  memberchk([A1,int],X),memberchk([A2,int],X),!.
opSignature(constgraphpoints, graph, [[stream,[tuple,X]],A1,A2,[map,[tuple,X],
        real],P1,P2],graph,[block,aggr,exp]) :-
  memberchk([A1,int],X),memberchk([A2,int],X),
  memberchk([P1,point],X),memberchk([P2,point],X),!.
opSignature((=), graph, [vertex,vertex],bool,[comm]).
opSignature((=), graph, [edge,edge],bool,[comm]).
opSignature((=), graph, [path,path],bool,[comm]).
opSignature((=), graph, [graph,graph],bool,[comm,exp]).
opSignature(equalway, graph, [path,path],bool,[comm,exp]).


/*
2.7.25 OptAuxAlgebra

*/
opSignature(predcounts, optaux, [[stream,[tuple,X]],FL],
        [stream,[tuple,[[atom,int],[counter,int]]]],[block]) :-
  is_list(FL), ground([[stream,[tuple,X]],FL]),
  findall(F,(member(F,FL),not(F = [_,[map,[tuple,X],bool]])),[]), !.


/*
2.7.26 TopOpsAlgebra

*/
opSignature(toprel, topops, [T1,T2],int9m,[]) :-
  memberchk(T1,[point, points, line, region]),
  memberchk(T2,[point, points, line, region]),!.
opSignature(toppred, topops, [T1,T2,cluster],bool,[]) :-
  memberchk(T1,[point, points, line, region]),
  memberchk(T2,[point, points, line, region]),!.
opSignature(tradjacent, topops, [T1,T2],bool,[comm,ass]) :-
  memberchk(T1,[point, points, line, region]),
  memberchk(T2,[point, points, line, region]),!.
opSignature(trinside, topops, [T1,T2],bool,[]) :-
  memberchk(T1,[point, points, line, region]),
  memberchk(T2,[point, points, line, region]),!.
opSignature(trcovers, topops, [T1,T2],bool,[]) :-
  memberchk(T1,[point, points, line, region]),
  memberchk(T2,[point, points, line, region]),!.
opSignature(trcoveredby, topops, [T1,T2],bool,[]) :-
  memberchk(T1,[point, points, line, region]),
  memberchk(T2,[point, points, line, region]),!.
opSignature(trequal, topops, [T1,T2],bool,[comm,ass]) :-
  memberchk(T1,[point, points, line, region]),
  memberchk(T2,[point, points, line, region]),!.
opSignature(trdisjoint, topops, [T1,T2],bool,[comm,ass]) :-
  memberchk(T1,[point, points, line, region]),
  memberchk(T2,[point, points, line, region]),!.
opSignature(troverlaps, topops, [T1,T2],bool,[comm,ass]) :-
  memberchk(T1,[point, points, line, region]),
  memberchk(T2,[point, points, line, region]),!.
opSignature(trcontains, topops, [T1,T2],bool,[]) :-
  memberchk(T1,[point, points, line, region]),
  memberchk(T2,[point, points, line, region]),!.

/*
2.7.27 TopRelAlgebra

*/
opSignature(invert, toprel, [int9m],int9m,[idem]).
opSignature(invert, toprel, [cluster],cluster,[idem]).
opSignature(union, toprel, [int9m,int9m],int9m,[comm,ass]).
opSignature(union, toprel, [cluster,cluster],cluster,[comm,ass]).
opSignature(intersection, toprel, [int9m,int9m],int9m,[comm,ass]).
opSignature(intersection, toprel, [cluster,cluster],cluster,[comm,ass]).
opSignature(multiintersection, toprel, [T,T|L],T,[]) :-
  (T = int9m ; T = cluster), my_list_to_set(L,[T]), !.
opSignature((+), toprel, [cluster,int9m],cluster,[]).
opSignature(number_of, toprel, [int9m],int,[]).
opSignature((-), toprel, [cluster,int9m],cluster,[]).
opSignature(renamecluster, toprel, [cluster,string],cluster,[]).
opSignature(name_of, toprel, [cluster],string,[]).
opSignature(contains, toprel, [cluster,int9m],bool,[]).
opSignature(disjoint, toprel, [cluster,cluster],bool,[comm,ass]).
opSignature((-), toprel, [cluster,cluster],cluster,[]).
opSignature(createpgroup, toprel, [cluster,cluster|L], predicategroup,[]) :-
  not(member(T,L),T \= cluster),!.
opSignature(createvalidpgroup, toprel, [cluster,cluster|L], predicategroup,[]):-
  not(member(T,L),T \= cluster),!.
opSignature(createprioritypgroup,toprel,[cluster,cluster|L],predicategroup,[]):-
  not(member(T,L),T \= cluster),!.
opSignature(clustername_of, toprel, [predicategroup,int9m],string,[]).
opSignature(clusterof, toprel, [predicategroup,int9m],cluster,[]).
opSignature(transpose, toprel, [int9m],int9m,[idem]).
opSignature(transpose, toprel, [cluster,string],cluster,[]).
opSignature(sizeof, toprel, [cluster],int,[]).
opSignature(sizeof, toprel, [predicategroup],int,[]).
opSignature(createcluster, toprel, [string,string],cluster,[]).
opSignature(createcluster, toprel, [string,text],cluster,[]).
opSignature(isComplete, toprel, [predicategroup],bool,[]).
opSignature(unspecified, toprel, [predicategroup],cluster,[]).
opSignature(pwdisjoint, toprel, [cluster,cluster|L],bool,[]) :-
  not((member(T,L), T \= cluster)),!.
opSignature(restrict, toprel, [cluster,string],cluster,[]).
opSignature(restrict, toprel, [cluster,text],cluster,[]).
opSignature(relax, toprel, [cluster,string],cluster,[]).
opSignature(relax, toprel, [cluster,text],cluster,[]).
opSignature(stdpgroup, toprel, [],predicategroup,[]).
opSignature((=), toprel, [int9m,int9m],bool,[comm]).
opSignature((=), toprel, [cluster,cluster],bool,[comm]).
opSignature((=), toprel, [predicategroup,predicategroup],bool,[comm]).
opSignature(getcluster, toprel, [predicategroup,string],cluster,[]).


/*
2.7.28 PictureAlgebra

*/
opSignature(height, picture, [picture],int,[]).
opSignature(width, picture, [picture],int,[]).
opSignature(isgrayscale, picture, [picture],bool,[]).
opSignature(filename, picture, [picture],string,[]).
opSignature(category, picture, [picture],string,[]).
opSignature(picturedate, picture, [picture],instant,[]).
opSignature(isportrait, picture, [picture],bool,[exp]).
opSignature(colordist, picture, [picture,int],histogram,[exp]).
opSignature(equals, picture, [picture,picture,int,int],real,[exp]).
opSignature(contains, picture, [picture,picture],bool,[exp]).
opSignature(simpleequals, picture, [picture,picture],bool,[exp]).
opSignature(like, picture, [picture,int,int,int,int],bool,[exp]).
opSignature(like, picture, [picture,real,real,int,int],bool,[exp]).
opSignature(scale, picture, [picture,int,int,int,int],picture,[exp]).
opSignature(cut, picture, [picture,int,int,int,int],picture,[exp]).
opSignature(flipleft, picture, [picture,int],picture,[exp]).
opSignature(mirror, picture, [picture,bool],picture,[exp]).
opSignature(display, picture, [picture],bool,[sidefx]).
opSignature(export, picture, [picture,text],bool,[sidefx]).


/*
2.7.29 ImExAlgebra

*/
opSignature(csvexport, imex, [[stream,[tuple,X]],string,bool,bool],
        [stream,[tuple,X]],[sidefx]).
opSignature(csvexport, imex, [[stream,[tuple,X]],string,bool,bool,string],
        [stream,[tuple,X]],[sidefx]).
opSignature(csvexport, imex,[[stream,X],string,bool],[stream,X],[sidefx]) :-
  isData(X).
opSignature(shpexport, imex, [[stream,[tuple,X]],string,bool,bool],
    [stream,[tuple,X]],[sidefx]).
opSignature(shpexport, imex, [[stream,X],string],
    [stream,X],[sidefx]) :- isData(X), !.
opSignature(db3export, imex, [[stream,[tuple,X]],text],[stream,[tuple,X]],
        [sidefx]).
opSignature(shptype, imex, [text],text,[]).
opSignature(shpimport, imex, [T,text],[stream,T],[]) :-
  memberchk(T,[point, points, line, region]),!.
opSignature(dbtype, imex, [text],text,[]).
opSignature(dbimport, imex, [[rel,[tuple,X]],text],[stream,[tuple,X]],[]).
opSignature(saveObject, imex, [_,string,text],bool,[sidefx]).
opSignature(csvimport, imex, [[rel,[tuple,X]],text,int,string],
        [stream,[tuple,X]],[]).
opSignature(csvimport, imex, [[rel,[tuple,X]],text,int,string,string],
        [stream,[tuple,X]],[]).
opSignature(isFile, imex, [text],bool,[]).
opSignature(isFile, imex, [string],bool,[]).
opSignature(removeFile, imex, [text],bool,[sidefx]).
opSignature(removeFile, imex, [string],bool,[sidefx]).
opSignature(createDirectory, imex, [text],bool,[sidefx]).
opSignature(createDirectory, imex, [string],bool,[sidefx]).
opSignature(fileSize, imex, [text],int,[]).
opSignature(fileSize, imex, [text,bool],int,[]).
opSignature(fileSize, imex, [string],int,[]).
opSignature(fileSize, imex, [string,bool],int,[]).
opSignature(isDirectory, imex, [text],bool,[]).
opSignature(isDirectory, imex, [string],bool,[]).
opSignature(writeFile, imex, [T1,T2],bool,[sidefx]) :-
  memberchk(T1,[text,string]), memberchk(T2,[text,string]),!.
opSignature(writeFile, imex, [T1,T2,bool],bool,[sidefx]) :-
  memberchk(T1,[text,string]), memberchk(T2,[text,string]),!.
opSignature(readFile, imex, [text],text,[]).
opSignature(readFile, imex, [string],text,[]).
opSignature(moveFile, imex, [text],bool,[sidefx]).
opSignature(moveFile, imex, [string],bool,[sidefx]).
opSignature(moveFile, imex, [text],[stream,text],[sidefx]).
opSignature(moveFile, imex, [text,int],[stream,text],[sidefx]).
opSignature(moveFile, imex, [string],[stream,text],[sidefx]).
opSignature(moveFile, imex, [string,int],[stream,text],[sidefx]).
opSignature(toCSVtext, imex, [T],text,[]) :- isKind(T,csvexportable),!.
opSignature(fromCSVtext, imex, [T,text],T,[]) :- isKind(T,csvimportable),!.
opSignature(fromCSVtext, imex, [T,string],T,[]) :- isKind(T,csvimportable),!.


/*
2.7.30 HashAlgebra

*/
opSignature(createhash, hash, [[rel,[tuple,AttrList]],Key],
                                 [hash,[tuple,AttrList],KeyType],[block]) :-
  memberchk([Key,KeyType],AttrList),
  (memberchk(KeyType,[int,real,text]);isKind(KeyType,indexable)),!.
opSignature(createhash, hash, [[stream,[tuple,AttrList]],Key],
                              [hash,[tuple,AttrList2],KeyType],[block]) :-
  select([_,tid],AttrList,AttrList2),
  memberchk([Key,KeyType],AttrList2),
  (memberchk(KeyType,[int,real,text]);isKind(KeyType,indexable)),!.
opSignature(exactmatch, hash, [[hash,[tuple,AttrList],KeyType],
      [rel,[tuple,AttrList]],Key],[stream,[tuple,AttrList]],[]) :-
  memberchk([Key,KeyType],AttrList), !.
opSignature(exactmatchS, hash, [[hash,[tuple,AttrList],KeyType],Key],
        [stream,[tuple,[[id,tid]]]],[]) :-
  memberchk([Key,KeyType],AttrList), !.
opSignature(inserthash, hash, [[stream,[tuple,AttrList1]],
                                [hash,[tuple,AttrList2],KeyType],Key],
                                [stream,[tuple,AttrList1]],[sidefx]) :-
  select([_,tid],AttrList1,AttrList2), memberchk([Key,KeyType],AttrList1), !.
opSignature(deletehash, hash, [[stream,[tuple,AttrList1]],
                                 [hash,[tuple,AttrList2],KeyType],Key],
                                 [stream,[tuple,AttrList1]],[sidefx]) :-
  select([_,tid],AttrList1,AttrList2), memberchk([Key,KeyType],AttrList1), !.
opSignature(updatehash, hash, [[stream,[tuple,AttrList1]],
                                 [hash,[tuple,AttrList2],KeyType],Key],
                                 [stream,[tuple,AttrList1]],[sidefx]) :-
  select([_,tid],AttrList1,AttrList2), memberchk([Key,KeyType],AttrList1), !.
opSignature(getFileInfo, hash, [[hash,_,_]],text,[exp]).


/*
2.7.31 TBTreeAlgebra

*/
opSignature(createtbtree, tbtree, [[rel,[tuple,X]],ID,UP],[tbtree,X, ID, UP],
        [block]) :- memberchk([ID,int],X), memberchk([UP,upoint],X), !.
opSignature(tbentries, tbtree, [[tbtree,_,_,_]],int,[]).
opSignature(tbnodes, tbtree, [[tbtree,_,_,_]],int,[]).
opSignature(tbleafnodes, tbtree, [[tbtree,_,_,_]],int,[]).
opSignature(tblevel, tbtree, [[tbtree,_,_,_]],int,[]).
opSignature(getnodes, tbtree, [[tbtree,_,_,_]], [stream,[tuple,[[id,int],
    [parentid,int], [level,int],[isleaf,bool],[entries,int],[box,rect3],
    [cov,int]]]],[]).
opSignature(getFileInfo, tbtree, [[tbtree,_,_,_]],text,[]).
opSignature(getentries, tbtree, [[tbtree,_,_,_]],[stream,[tuple,[[tupleid,int],
    [trjid,int],[box,rect3]]]],[]).
opSignature(windowintersectsS, tbtree, [[tbtree,_,_,_],QT],
    [stream,[tuple,[[id,tid]]]],[]) :- memberchk(QT,[rect,rect3]),!.
opSignature(getBox, tbtree, [[tbtree,_,_,_]],rect3,[]).
opSignature(windowintersects, tbtree, [[tbtree,X,_,_],[rel,[tuple,X]],QT],
        [stream,[tuple,X]],[]) :-
  memberchk(QT,[rect,rect3]),!.
opSignature(bulkloadtbtree, tbtree, [[stream,[tuple,X]],ID,UP,TID],
        [tbtree,X,ID,UP],[block]) :-
  memberchk((ID,int),X), memberchk((UP,upoint),X), memberchk((TID,tid),X), !.
opSignature(getallentries,tbtree,[[tbtree,_,_,_]],[stream,[tuple,[[tupleid,int],
        [trjid,int],[box,rect3],[nodeid,int]]]],[]).

/*
2.7.32 UpdateRelationAlgebra

*/
opSignature(createinsertrel, updaterelation, [[rel,[tuple,X]]],[rel,[tuple,R]],
        [sidefx]) :- append(X,[[tid,tid]],R),!.
opSignature(createdeleterel, updaterelation, [[rel,[tuple,X]]],[rel,[tuple,R]],
        [sidefx]) :- append(X,[[tid,tid]],R),!.
opSignature(createupdaterel, updaterelation, [[rel,[tuple,X]]],[rel,[tuple,R]],
        [sidefx]) :-
  ground([[rel,[tuple,X]]]),
  findall([OldName,Type],
          ( member([Name,Type],X),
            atom_concat(Name,'_old',OldName)
          ),
          O),
  append(X,O,R), !.
opSignature(insert, updaterelation, [[stream,[tuple,X]],[rel,[tuple,X]]],
        [stream,[tuple,R]],[sidefx]) :-
  append(X,[[tid,tid]],R),!.
opSignature(insertsave, updaterelation, [[stream,[tuple,X]],[rel,[tuple,X]],
        [rel,[tuple,R]]],[stream,[tuple,R]],[sidefx]) :-
  append(X,[[tid,tid]],R),!.
opSignature(inserttuple,updaterelation,[[rel,[tuple,X]]|VL],
        [stream,[tuple,R]],[sidefx]):-
  is_list(VL), %% Types in Value-List not checked!
  append(X,[[tid,tid]],R),!.
opSignature(inserttuplesave, updaterelation,[[rel,[tuple,X]],
            [rel,[tuple,R]]|VL],
        [stream,[tuple,R]],[sidefx]) :-
  is_list(VL), %% Types in Value-List not checked!
  append(X,[[tid,tid]],R),!.
opSignature(deletesearch, updaterelation, [[stream,[tuple,X]],[rel,[tuple,X]]],
        [stream,[tuple,R]],[sidefx]) :-
  append(X,[[tid,tid]],R),!.

opSignature(deletedirect, updaterelation, [[stream,[tuple,X]],[rel,[tuple,X]]],
        [stream,[tuple,R]],[sidefx]) :-
  append(X,[[tid,tid]],R),!.

opSignature(deletesearchsave,updaterelation,[[stream,[tuple,X]],
        [rel,[tuple,X]]],[stream,[tuple,R]],[sidefx]) :-
  append(X,[[tid,tid]],R),!.

opSignature(deletedirectsave,updaterelation,[[stream,[tuple,X]],[rel,[tuple,X]],
        [rel,[tuple,R]]],[stream,[tuple,R]],[sidefx]) :-
  append(X,[[tid,tid]],R),!.
opSignature(updatesearch, updaterelation, [[stream,[tuple,X]],[rel,[tuple,X]],
        FunList],[stream,[tuple,R]],[sidefx]) :-
  is_list(FunList), %% Types in FunList not checked!
  ground([[stream,[tuple,X]],[rel,[tuple,X]],FunList]),
  findall([OldName,Type],
          ( member([Name,Type],X),
            atom_concat(Name,'_old',OldName)
          ),
          O),
  append(X,O,R1), append(R1,[[tid,tid]],R),!.

opSignature(updatedirect, updaterelation, [[stream,[tuple,X]],[rel,[tuple,X]],
        FunList],[stream,[tuple,R]],[sidefx]) :-
  is_list(FunList), %% Types in FunList not checked!
  ground([[stream,[tuple,X]],[rel,[tuple,X]],FunList]),
  findall([OldName,Type],
          ( member([Name,Type],X),
            atom_concat(Name,'_old',OldName)
          ),
          O),
  append(X,O,R1), append(R1,[[tid,tid]],R),!.

opSignature(updatesearchsave,updaterelation,[[stream,[tuple,X]],[rel,[tuple,X]],
        [rel,[tuple,XI]],FunList],[stream,[tuple,R]],[sidefx]) :-
  is_list(FunList), %% Types in FunList not checked!
  ground([[stream,[tuple,X]],[rel,[tuple,X]],[rel,[tuple,XI]],FunList]),
  append(X,[[tid,tid]],XI),
  findall([OldName,Type],
          ( member([Name,Type],X),
            atom_concat(Name,'_old',OldName)
          ),
          O),
  append(XI,O,R), !.

opSignature(updatedirectsave,updaterelation,[[stream,[tuple,X]],
        [rel,[tuple,XI]],[rel,[tuple,XI]],FunList],[stream,[tuple,R]],
        [sidefx]) :-
  is_list(FunList), %% Types in FunList not checked!
  ground([[stream,[tuple,X]],[rel,[tuple,XI]],[rel,[tuple,XI]],FunList]),
  append(X,[[tid,tid]],XI),
  findall([OldName,Type],
          ( member([Name,Type],X),
            atom_concat(Name,'_old',OldName)
          ),
          O),
  append(XI,O,R), !.

opSignature(addid, updaterelation, [[stream,[tuple,X]]],[stream,[tuple,XI]],
        [sidefx]) :- append(X,[[tid,tid]],XI),!.

opSignature(deletebyid, updaterelation, [[rel,[tuple,X]],tid],
        [stream,[tuple,XI]],[sidefx]):- append(X,[[tid,tid]],XI),!.
opSignature(updatebyid, updaterelation, [[stream,[tuple,X]],[rel,[tuple,X]],tid,
        FunList],[stream,[tuple,R]],[sidefx]) :-
  is_list(FunList), %% Types in FunList not checked!
  ground([[stream,[tuple,X]],[rel,[tuple,X]],tid,FunList]),
  findall([OldName,Type],
          ( member([Name,Type],X),
            atom_concat(Name,'_old',OldName)
          ),
          O),
  append(X,[[tid,tid]],XI), append(XI,O,R), !.

opSignature(insertrtree, updaterelation, [[stream,[tuple,XI]],
        [rtree,[tuple,X], _, KeyType],KeyName],[stream,[tuple,XI]],[sidefx]) :-
  select([KeyName,KeyType],XI,X), !.

opSignature(deletertree, updaterelation, [[stream,[tuple,XI]],
        [rtree,[tuple,X], _, KeyType],KeyName],[stream,[tuple,XI]],[sidefx]) :-
  select([KeyName,KeyType],XI,X), !.

opSignature(updatertree, updaterelation, [[stream,[tuple,R]],
        [rtree,[tuple,X], _, KeyType],KeyName],[stream,[tuple,XI]],[sidefx]) :-
  ground([[stream,[tuple,R]],[rtree,[tuple,X], _, KeyType],KeyName]),
  select([KeyName,KeyType],R,X),
  findall([OldName,Type],
          ( member([Name,Type],X),
            atom_concat(Name,'_old',OldName)
          ),
          O),
  append(R,O,XI), !.


/*
2.7.32 NearestNeighborAlgebra

*/
opSignature(distancescan, nearestneighbor, [[rtree,[tuple,X], _, _],
	   [rel,[tuple,X]], T, int], [stream,[tuple,X]], []) :-
  memberchk(T,[point,points,line,sline,region,rect]), !.

opSignature(distancescan2, nearestneighbor, [[rtree,[tuple,X], _, _],
	   [rel,[tuple,X]], T, int], [stream,[tuple,X]], []) :-
  memberchk(T,[point,points,line,sline,region,rect]), !.

opSignature(distancescan3, nearestneighbor, [[rtree,[tuple,X], _, _],
	   [rel,[tuple,X]], T, int], [stream,[tuple,X]], []) :-
  memberchk(T,[point,points,line,sline,region,rect]), !.

opSignature(distancescan4, nearestneighbor, [[rtree3,[tuple,X], _, _],
	   [rel,[tuple,X]], point, instant, int], [stream,[tuple,X]], []) :-
  !.

opSignature(knearest, nearestneighbor, [[stream,[tuple,AttrList]], Key,
					mpoint, int],
	   [stream,[tuple,AttrList]], []) :-
  memberchk([Key, _],AttrList), !.

opSignature(knearestvector, nearestneighbor, [[stream,[tuple,AttrList]], Key,
					mpoint, int],
	   [stream,[tuple,AttrList]], []) :-
  memberchk([Key, _],AttrList), !.

opSignature(oldknearestfilter, nearestneighbor, [[rtree,[tuple,X], _, _],
						 [rel,[tuple,X]],
						 mpoint, int],
	   [stream,[tuple,X]], []) :-
  !.

opSignature(rect2periods, nearestneighbor, [rect3], periods, []) :-
  !.

opSignature(bboxes, nearestneighbor, [[stream, periods], mpoint],
	    [stream, rect], []) :-
  !.

opSignature(coverage, nearestneighbor, [[rtree3,_, _, _]],
	    [stream, [tuple, [[_, int], [_, uint]]]], []) :-
  !.

opSignature(coverage2, nearestneighbor, [[rtree3,_, _, _]],
	    [stream, [tuple, [[_, int], [_, int],
			      [_, mint]]]], []) :-
  !.

opSignature(knearestfilter, nearestneighbor,[[rtree,[tuple,X1], _, _],
	   [rel,[tuple,X1]], [btree,[tuple,X2], _, _], [rel,[tuple,X2]],
					    mpoint, int],
	    [stream, [tuple, _]], []) :-
  !.

/*
2.7.2 (Still) Missing Algebras

The typemappings for the operators of the following algebras still need to be declared:

   * GeneralTreeAlgebra

   * MTreeAlgebra

   * XTreeAlgebra

   * ChessAlgebra

   * ClusterAlgebra

   * ConstraintAlgebra

   * DateAlgebra

   * FuzzyAlgebra

   * GraphVizAlgebra

   * HierarchicalGeoAlgebra

   * HistogramAlgebra

   * JBBoxAlgebra

   * MidiAlgebra

   * MP3Algebra

   * MRegionOpsAlgebra

   * NetworkAlgebra

   * OldRelationAlgebra

   * PartitionedStreamAlgebra

   * PeriodicAlgebra

   * PointRectangleAlgebra

   * PointSequenceAlgebra

   * PolygonAlgebra

   * RasterAlgebra

   * RasterSpatialAlgebra

   * RegionInterpolationAlgebra

   * RemoteStreamAlgebra

   * RoseAlgebra

   * StreamExampleAlgebra

   * TemporalNetAlgebra

   * UGridAlgebra

   * WebAlgebra

Decriptions of the operators provided by these algebras should be added, when
they are indended to be used with the optimizer.

*/

/*
NearestNeighbor Algebra

*/

%Faked operator
opSignature(isknn, nearestneighbor, [IDType, int, mpoint, string, 
                              string, string, int], mbool, []):-
  memberchk(IDType,[int, real, string]), !.


% Section:Start:opSignature_5_e
/*
STPatternAlgebra

*/
opSignature(stconstraint, stpattern, [string, string, stvector],bool,[]).
opSignature(vec, stpattern, StrList, stvector,[]):-
  onlyContains(StrList,string).
opSignature(end, stpattern, [string], instant,[]).
opSignature(start, stpattern, [string], instant,[]).
opSignature(pattern, stpattern, [NamedPredList, ConList], bool,[]):-
  onlyContains(NamedPredList,namedPred),
  onlyContains(ConList,bool).
opSignature(patternex, stpattern, [NamedPredList, ConList, bool], bool,[]):-
  onlyContains(NamedPredList,namedPred),
  onlyContains(ConList,bool).
opSignature(as, stpattern, [mbool,X], namedPred,[]):-
  atom(X).
opSignature(randomdelay, stpattern, [mpoint, duration], mpoint,[]).
opSignature(passmbool, stpattern, [mbool], mbool,[]).
opSignature(randommbool, stpattern, [instant], mbool,[]).
% Section:End:opSignature_5_e

/*
2.2 Checking Operators for Certain Properties

The following subsections introduce predicates for checking on whether a
operator, signature, or outermost operator of a term has certain properties
(resp. belongs to certain classes of operators).

----
checkOpProperty( +Op, +ArgTypes, ?Flag)
checkOpProperty( +Term, ?Flag)
----

These first predicate succeeds, if operator ~Op~ with signature ~ArgTypes~ has property
flag ~Flag~ set. Otherwise it fails.

The second predicate does the same, but determines the argument types itself by
calling ~getTypeTree/2~.

*/

% checkOpProperty(+Op,+ArgsTypeList,?Flag) :-
checkOpProperty(Op,ArgsTypeList,Flag) :-
  ground(Op), ground(ArgsTypeList),
  opSignature(Op, _, ArgsTypeList,_,Flags),
  member(Flag,Flags).

checkOpProperty(Op,ArgsTypeList,Flag) :-
  (   not(ground(Op))
    ; not(ground(ArgsTypeList))
  ),
  throw(error_Internal(operators_checkOpProperty(Op,ArgsTypeList,Flag)
                    :wrongInstantiationPattern)),
  fail, !.

checkOpProperty(Op,ArgsTypeList,_Flag) :-
  ground(Op), ground(ArgsTypeList),
  fail, !.

% The following version takes a term and extracts the types by itself:
% checkOpProperty(+Term,?Flag) :-
checkOpProperty(Term,Flag) :-
  getTypeTree(Term,X),
  X = [Op,ArgsTrees,Type],
  findall(Type,member([_,_,Type],ArgsTrees),ArgTypes),
  checkOpProperty(Op, ArgTypes, Flag), !.

/*

2.2.1 Predicates which can use bboxes

Several operators, like geometric predicates who use bounding boxes, have
properties, that require them to be handled differently in some ways. These
operators are flagged with a property ~bbox(Dim)~, where ~Dim~ is the dimension
of the used minimum bounding rectangles: either ~d2~, ~d3~, ~d4~, or ~d8~,
in their ~opSignature/5~ description.

*/

% Replacement for old version: if optimizerOption(determinePredSig) is NOT used
isBBoxPredicate(Term) :-
  not(optimizerOption(determinePredSig)),
  compound(Term), not(is_list(Term)),
  Term =.. [Op|_],
  opSignature(Op, _, _ArgsTypeList,bool,Flags),
  memberchk(bbox(_Dim),Flags),
  dm(gettypetree,['INFO:\tOperator name matching used to determine ',
              'isBBoxPredicate(',Op,').\n']),
  !.

/*
----
isBBoxPredicate(intersects).  % but not: rT x rT
isBBoxPredicate(intersects_new).
isBBoxPredicate(p_intersects).
isBBoxPredicate(inside).      % but also: mT x mT -> movingbool
isBBoxPredicate(passes).
isBBoxPredicate(insideold).
isBBoxPredicate(adjacent).
isBBoxPredicate(attached).
isBBoxPredicate(overlaps).
isBBoxPredicate(onborder).
isBBoxPredicate(ininterior).
isBBoxPredicate(trcovers).
isBBoxPredicate(trequal).
isBBoxPredicate(tradjacent).
isBBoxPredicate(trinside).
isBBoxPredicate(trcovers).
isBBoxPredicate(trcoveredby).
isBBoxPredicate(troverlaps).
----

*/

% Section:Start:isBBoxPredicate_1_e
% Section:End:isBBoxPredicate_1_e

% more recent version: if optimizerOption(determinePredSig) is used
% --- isBBoxPredicate(+Op,+ArgTypeList,?Dimension)
isBBoxPredicate(Op,ArgsTypeList,Dim) :-
  opSignature(Op, _, ArgsTypeList,bool,Flags),
  memberchk(bbox(Dim),Flags),!.

% The following version takes a term and extracts the types by itself:
% --- isBBoxPredicate(+Term,?Dimension)
isBBoxPredicate(Term,Dim) :-
  getTypeTree(Term,X),
  X = [Op,ArgsTrees,Type],
  findall(Type,member([_,_,Type],ArgsTrees),ArgTypes),
  isBBoxPredicate(Op,ArgTypes,Dim), !.

% Section:Start:isBBoxLiftedPred_1_e
% Section:End:isBBoxLiftedPred_1_e

% other operators using bboxes:

% Replacement for old version: if optimizerOption(determinePredSig) is NOT used
isBBoxOperator(Term) :-
  not(optimizerOption(determinePredSig)),
  compound(Term), not(is_list(Term)),
  Term =.. [Op|_],
  opSignature(Op, _, _ArgsTypeList,_,Flags),
  memberchk(bbox(_Dim),Flags),
  dm(gettypetree,['INFO:\tOperator name matching used to determine ',
              'isBBoxOperator(',Op,').\n']),
  !.

/*
----
% old version: if optimizerOption(determinePredSig) is NOT used
isBBoxOperator(touchpoints).
isBBoxOperator(intersection).
isBBoxOperator(intersection_new).
isBBoxOperator(commonborder).
isBBoxOperator(commonborderscan).
isBBoxOperator(X) :- isBBoxPredicate(X).
----

*/

% Section:Start:isBBoxOperator_1_e
% Section:End:isBBoxOperator_1_e

% more recent version: if optimizerOption(determinePredSig) is used
% --- isBBoxOperator(+Op,+ArgTypeList,?Dimension)
isBBoxOperator(Op,ArgsTypeList,Dim) :-
  opSignature(Op, _, ArgsTypeList,_,Flags),
  memberchk(bbox(Dim),Flags),!.


/*
2.2.2 Commutative operators

These binary operators can be handled specially in some translation rules.
They should be marked with ~comm~ in their property flags within ~opSignature/5~.

*/

% current version: if optimizerOption(determinePredSig),
% the predicate can be replaced by
% --- checkOpProperty(Op,[A1,A2],comm)

isCommutativeOP(Op,ArgsTypeList) :-
  opSignature(Op, _, ArgsTypeList, _, Flags),
  memberchk(comm,Flags),!.

isCommutativeOP(Term) :-
  optimizerOption(determinePredSig),
  compound(Term), not(is_list(Term)),
  checkOpProperty(Term,comm), !.

isCommutativeOP(Op) :-
  atom(Op),
  opSignature(Op, _, _, _, Flags),
  dm(gettypetree,['INFO:\tOperator name matching used to determine ',
              'isCommutativeOP(',Op,').\n']),
  memberchk(comm,Flags),!.

/*
----
% old version: if optimizerOption(determinePredSig) is NOT used
isCommutativeOP((=)).
isCommutativeOP((#)).
isCommutativeOP(intersects).
isCommutativeOP(intersects_new).
isCommutativeOP(p_intersects).
isCommutativeOP(adjacent).
isCommutativeOP(attached).
isCommutativeOP(overlaps).
isCommutativeOP(everNearerThan).
isCommutativeOP(distance).
isCommutativeOP(trequal).
isCommutativeOP(trdisjoint).
isCommutativeOP(troverlaps).
----

*/

% Section:Start:isCommutativeOP_1_e
% Section:End:isCommutativeOP_1_e

/*
2.2.3 Aggregation operators

These use a common cost function. They can be recognized by predicate
~isAggregationOP(OP)~ if they have been marked with the ~aggr~ flag within
their operator description ~opSignature/5~.

*/
isAggregationOP(Op,ArgsTypeList) :-
  opSignature(Op, _, ArgsTypeList, _, Flags),
  memberchk(aggr,Flags),!.

isAggregationOP(Term) :-
  optimizerOption(determinePredSig),
  compound(Term), not(is_list(Term)),
  checkOpProperty(Term,aggr), !.

isAggregationOP(Op) :-
  atom(Op),
  opSignature(Op, _, _, _, Flags),
  dm(gettypetree,['INFO:\tOperator name matching used to determine ',
              'isAggregationOP(',Op,').\n']),
  memberchk(aggr,Flags),!.

/*
----
isAggregationOP(count).
isAggregationOP(min).
isAggregationOP(max).
isAggregationOP(sum).
isAggregationOP(avg).
isAggregationOP(extract).
isAggregationOP(var).

% For later extensions (though needing separate cost functions):
isAggregationOP(aggregate).  % the cost of the provided function should be applied, works lineary
isAggregationOP(aggregateB). % the cost of the provided function should be applied,
                             %   Additionally, the operator works balanced (in log(CX) steps).
----

*/

% Section:Start:isAggregationOP_1_e
% Section:End:isAggregationOP_1_e


/*
2.2.4 Join Operators

PlanRewriting needs to identify join operators to allow for a generalized handling.
For each join operator ~j~, a flag ~join~ must be defined within the operator
description's ~Flags~ field.

When option ~rewriteCSE~ is enabled, this information is used to handle
Join operators are expected to merge the attribute sets of their first two
arguments. All other operators are expected not to change the attribute set of
the manipulated stream.

Otherwise, a dedicated rule must be added to predicate ~insertExtend/4~ in file
~optimizer.pl~.

*/

isJoinOP(Op,ArgsTypeList) :-
  opSignature(Op, _, ArgsTypeList, _, Flags),
  memberchk(join,Flags),!.

isJoinOP(Term) :-
  optimizerOption(determinePredSig),
  compound(Term), not(is_list(Term)),
  checkOpProperty(Term,join), !.

isJoinOP(Op) :-
  atom(Op),
  opSignature(Op, _, _, _, Flags),
  memberchk(join,Flags),
  dm(gettypetree,['INFO:\tOperator name matching used to determine ',
              'isJoinOP(',Op,').\n']),
  !.

% only required because type mappings for PStreamAlgebra are not provided yet.
isJoinOP(pjoin) :-
  dm(gettypetree,['INFO:\tOperator name matching used to determine ',
              'isJoinOP(',pjoin,').\n']), !.

/*
----
isJoinOP(sortmergejoin).
isJoinOP(mergejoin).
%isJoinOP(symmjoin). % has a dedicated rule for insertExtend/4
isJoinOP(symmproductextend). % could get a dedicated rule for insertExtend/4
isJoinOP(hashjoin).
isJoinOP(spatialjoin).
isJoinOP(loopjoin).
isJoinOP(product).
isJoinOP(symmproduct).
isJoinOP(pjoin).
----

*/

% Section:Start:isJoinOP_1_e
% Section:End:isJoinOP_1_e

/*
2.2.5 Maitenance of Tuple Ordering

The interesting orders extension needs to recognize operators, that maintain/disturb existing
orderings. ~no~ means, the operator will destroy preexisting orderings. ~outer~ means, it will
keep the ordering of the outer input stream (the leftmost one). If no fact is stored about an
operator, it is assumed, that it will maintain any existing ordering.

*/

maintainsOrderOP(hashjoin,              no).
maintainsOrderOP(symmjoin,              no).
maintainsOrderOP(spatialjoin,           no).
maintainsOrderOP(loopjoin,           outer).
maintainsOrderOP(product,               no).
maintainsOrderOP(symmproduct,           no).
maintainsOrderOP(symmproductextend,     no).
maintainsOrderOP(sort,                  no).
maintainsOrderOP(sortby,                no).
maintainsOrderOP(sortmergejoin,    special).

% Section:Start:maintainsOrderOP_2_e
% Section:End:maintainsOrderOP_2_e


/*
3 Properties of Datatypes

This section defines properties for certain data types.

3.1 Null Values

---- nullValue(Type, NullValueType, NullValueValue)
----

Declares NULL-values, that can be used to initialize Attributes, aggregation
expressions, etc. for a given Secondo data type ~Type~.

~NullValueType~ should be one of ~undefined~, ~null~, ~one~, ~empty~, ~error~,
~default~.

  * undefined --- used as a standard value for attributes, if nothing else is
    specified. Also represents results of failed operations.

  * null --- neutral element for operations usually expressed by $T + T$

  * one --- neutral element fpr operations usually expressed by $T \times T$

  * empty --- represents an empty value, for set-based data types.

  * error --- usually the same as ~undefined~

  * default --- a defined value used as a standard value

~NullValueValue~ ist the value part of a nested list expression representing the
according NULL value, i.e.

---- [const <Type> value <NullValueValue>]
----

*/

nullValue(bool,undefined,'undefined').
nullValue(bool,null,'FALSE').
nullValue(bool,one,'TRUE').
nullValue(bool,error,'undefined').
nullValue(bool,default,'FALSE').

nullValue(int,undefined,'undefined').
nullValue(int,null,'0').
nullValue(int,one,'1').
nullValue(int,error,'undefined').
nullValue(int,default,'0').

nullValue(real,undefined,'undefined').
nullValue(real,null,'0.0').
nullValue(real,one,'1.0').
nullValue(real,error,'undefined').
nullValue(real,default,'0.0').

nullValue(string,undefined,'undefined').
nullValue(string,empty,'""').
nullValue(string,error,'undefined').
nullValue(string,default,'""').

nullValue(text,undefined,'undefined').
nullValue(text,empty,'\'\'').
nullValue(text,error,'undefined').
nullValue(text,default,'\'\'').

nullValue(point,undefined,'()').
nullValue(point,null,'(0.0 0.0)').
nullValue(point,error,'()').
nullValue(point,default,'(0.0 0.0)').

nullValue(points,undefined,'()').
nullValue(points,empty,'()').
nullValue(points,error,'()').
nullValue(points,default,'()').

nullValue(line,undefined,'undefined').
nullValue(line,empty,'()').
nullValue(line,error,'undefined').
nullValue(line,default,'()').

nullValue(sline,undefined,'undefined').
nullValue(sline,empty,'()').
nullValue(sline,error,'undefined').
nullValue(sline,default,'()').

nullValue(region,undefined,'()').
nullValue(region,empty,'()').
nullValue(region,error,'()').
nullValue(region,default,'()').

nullValue(instant,undefined,'undefined').
nullValue(instant,error,'undefined').
nullValue(instant,default,'currenttime').

nullValue(duration,undefined,'undefined').
nullValue(duration,null,'(0 0)').
nullValue(duration,error,'undefined').
nullValue(duration,default,'(0 0)').

nullValue(periods,undefined,'()').
nullValue(periods,empty,'()').
nullValue(periods,null,'()').
nullValue(periods,one,'(("begin of time" "end of time" TRUE TRUE))').
nullValue(periods,error,'()').
nullValue(periods,default,'()').

nullValue(mpoint,undefined,'()').
nullValue(mpoint,empty,'()').
nullValue(mpoint,error,'()').
nullValue(mpoint,default,'()').

nullValue(upoint,undefined,'undefined').
nullValue(upoint,error,'undefined').
nullValue(upoint,default,'((currenttime currenttime TRUE TRUE)(0.0 0.0))').

nullValue(ipoint,undefined,'undefined').
nullValue(ipoint,error,'undefined').
nullValue(ipoint,default,'(currenttime (0.0 0.0))').

nullValue(mreal,undefined,'()').
nullValue(mreal,empty,'()').
nullValue(mreal,null,
       '((("begin of time" "end of time" TRUE TRUE) (0.0 0.0 0.0 FALSE)))').
nullValue(mreal,one,
       '((("begin of time" "end of time" TRUE TRUE) (0.0 0.0 1.0 FALSE)))').
nullValue(mreal,error,'()').
nullValue(mreal,default,'()').

nullValue(ureal,undefined,'undefined').
nullValue(ureal,null,
          '(("begin of time" "end of time" TRUE TRUE) (0.0 0.0 0.0 FALSE))').
nullValue(ureal,one,
          '(("begin of time" "end of time" TRUE TRUE) (0.0 0.0 1.0 FALSE))').
nullValue(ureal,error,'()').
nullValue(ureal,default,'()').

nullValue(ireal,undefined,'undefined').
nullValue(ireal,error,'undefined').
nullValue(ireal,default,'(currenttime 0.0)').

nullValue(rreal,undefined,'()').
nullValue(rreal,empty,'()').
nullValue(rreal,error,'()').
nullValue(rreal,default,'()').

nullValue(mint,undefined,'()').
nullValue(mint,null,'((("begin of time" "end of time" TRUE TRUE) 0))').
nullValue(mint,one,'((("begin of time" "end of time" TRUE TRUE) 1))').
nullValue(mint,error,'()').
nullValue(mint,default,'()').

nullValue(uint,undefined,'undefined').
nullValue(uint,null,'(("begin of time" "end of time" TRUE TRUE) 0)').
nullValue(uint,one,'(("begin of time" "end of time" TRUE TRUE) 1)').
nullValue(uint,error,'()').
nullValue(uint,default,'()').

nullValue(iint,undefined,'undefined').
nullValue(iint,null,'undefined').
nullValue(iint,error,'undefined').
nullValue(iint,default,'(currenttime 0)').

nullValue(rint,undefined,'()').
nullValue(rint,empty,'()').
nullValue(rint,error,'()').
nullValue(rint,default,'()').

nullValue(mbool,undefined,'()').
nullValue(mbool,null,'((("begin of time" "end of time" TRUE TRUE) FALSE))').
nullValue(mbool,one,'((("begin of time" "end of time" TRUE TRUE) TRUE))').
nullValue(mbool,error,'()').
nullValue(mbool,default,'()').

nullValue(ubool,undefined,'undefined').
nullValue(ubool,null,'(("begin of time" "end of time" TRUE TRUE) FALSE)').
nullValue(ubool,one,'(("begin of time" "end of time" TRUE TRUE) TRUE)').
nullValue(ubool,error,'()').
nullValue(ubool,default,'()').

nullValue(ibool,undefined,'undefined').
nullValue(ibool,null,'undefined').
nullValue(ibool,error,'undefined').
nullValue(ibool,default,'(currenttime FALSE)').

nullValue(rbool,undefined,'()').
nullValue(rbool,empty,'()').
nullValue(rbool,error,'()').
nullValue(rbool,default,'()').

nullValue(mstring,undefined,'()').
nullValue(mstring,null,'((("begin of time" "end of time" TRUE TRUE) ""))').
nullValue(mstring,error,'()').
nullValue(mstring,default,'()').

nullValue(ustring,undefined,'undefined').
nullValue(ustring,null,'(("begin of time" "end of time" TRUE TRUE) "")').
nullValue(ustring,error,'()').
nullValue(ustring,default,'()').

nullValue(istring,undefined,'undefined').
nullValue(istring,null,'undefined').
nullValue(istring,error,'undefined').
nullValue(istring,default,'undefined').

nullValue(rstring,undefined,'()').
nullValue(rstring,empty,'()').
nullValue(rstring,error,'()').
nullValue(rstring,default,'()').

nullValue(movingregion,undefined,'()').
nullValue(movingregion,null,'()').
nullValue(movingregion,error,'()').
nullValue(movingregion,default,'()').

nullValue(uregion,undefined,'undefined').
nullValue(uregion,null,'()').
nullValue(uregion,error,'()').
nullValue(uregion,default,'()').

nullValue(intimeregion,undefined,'undefined').
nullValue(intimeregion,null,'undefined').
nullValue(intimeregion,error,'undefined').
nullValue(intimeregion,default,'(currenttime ())').

nullValue(rect,undefined,'undefined').
nullValue(rect,empty,'(0.0 0.0 0.0 0.0)').
nullValue(rect,error,'undefined').
nullValue(rect,default,'(0.0 0.0 0.0 0.0)').

nullValue(rect3,undefined,'undefined').
nullValue(rect3,empty,'(0.0 0.0 0.0 0.0 0.0 0.0)').
nullValue(rect3,error,'undefined').
nullValue(rect3,default,'(0.0 0.0 0.0 0.0 0.0 0.0)').

nullValue(rect4,undefined,'undefined').
nullValue(rect4,empty,'(0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0)').
nullValue(rect4,error,'undefined').
nullValue(rect4,default,'(0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0)').

nullValue(rect8,undefined,'undefined').
nullValue(rect8,empty,
         '(0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0)').
nullValue(rect8,error,'undefined').
nullValue(rect8,default,
         '(0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0)').

% Section:Start:nullValue_3_e
% Section:End:nullValue_3_e

/*
End of file ~operators.pl~

*/
