/*

----
This file is part of SECONDO.

Copyright (C) 2012, University Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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
//[newpage] [\newpage]
//[_] [\_]
 



[10]  Predicates for Automatic Query Generation and Benchmark


By Gero Willmes, January 2012

Implementations for my master thesis

  


[newpage]

[toc]

[newpage]

*/



/*

1 Predicates for Automatic Query Generation and Benchmark

*/


:- use_module(library(random)).
:- dynamic queryComponent/1.
:- dynamic queryPred/1.

/*

1.1 generateQueryTerm

*/

/*
----     generateQueryTerm(+NumberRels, +Factor, -QueryTerm)
----

Description:

Generates a query term ~QueryTerm~ of type 'select count([star]) from RelList where PredList.'
over ~NumberRels~ relations. 
The relations 1..NumberRels must exist in the database and must be named r1...r(NumberRels). 
Each relation must have a single attribute of type integer which is named (Relationname)i. 
E.g. If ~NumberRels~ = 2, then there must exists two relations 'r1' and 'r2' where  
'r1' has the integer attribute 'r1i' and 'r2' has the integer attribute 'r2i'.
The generated predicates of ~PredList~ build a spanning tree over the relations in ~RelList~.
~Factor is not used~

*/

generateQueryTerm(NumberRels, QueryTerm):-
 retractall(queryComponent(_)),
 retractall(queryPred(_)),
 createQueryComponents(NumberRels,_),
 joinAllQueryComponents(NumberRels, _),
 findall(Pred, queryPred(Pred), PredList),
 generateRelNames(NumberRels, RelNames),
 reverse(RelNames, ReverseRelNames),
 QueryTerm = (select count(*) from ReverseRelNames where PredList).

/*

1.2 generate

*/

/*
----     generate(+DatabaseName, +NumberRelations)
----

Description:
Generates a query over ~NumberRelations~ relations in database ~DatabaseName~ and prints it on the screen

*/

generate(_, X):-
 isDatabaseOpen,
 generateQueryTerm(X, Query),
 nl,nl,write('Query: '),nl, nl, write(Query),nl,nl.

generate(DB, X):-
 open database DB,
 generateQueryTerm(X, Query),
 nl,nl,write('Query: '),nl, nl, write(Query),nl,nl.

/*

1.3 generateRelNames

*/
 
/*
----     generateRelNames(+Number, -RelNameList)
----

Description:

Generates ~Number~ relation names and returns them in the list ~RelNameList~ 

*/

generateRelNames(1, [RelName]):-
  concatrelname(1, RelName).

generateRelNames(N, [H|Rest]):-
  concatrelname(N, H),
  Nminus1 is N-1,
  generateRelNames(Nminus1, Rest).

/*

1.4 joinAllQueryComponents

*/

/*
----     joinAllQueryComponents(+N, +Factor)
----

Description:

Joins in a loop two randomly chosen query components of type 'queryComponent(relList(RelList))'. 
Finally there is a single resulting query component.
~Factor is not used~

*/
joinAllQueryComponents(1, _).
joinAllQueryComponents(N, _):-
  findall(queryComponent(relList(X)),queryComponent(relList(X)), 
  QueryComponentList),
  selectRandomListElement(QueryComponentList, QueryComponent1),
  select(QueryComponent1, QueryComponentList, 
  ReducedQueryComponentList),
  selectRandomListElement(ReducedQueryComponentList, QueryComponent2),
  joinQueryComponents(QueryComponent1, QueryComponent2, _),    
  Nminus1 is N-1,
  joinAllQueryComponents(Nminus1, _).

/*

1.5 joinQueryComponents

*/

/*
----    joinQueryComponents(+QueryComponent1, +QueryComponent2, +Factor)
----

Description:

Joins ~QueryComponent1~ and ~QueryComponent2~ which are of type 
'queryComponent(relList(RelList))'. 
Joining means generating 
a predicate and merging the relation lists of the two components into a single resulting component.
The selectivity of the generated predicate is adjusted in a way that the result size of 
this predicate multiplies the cardinality of the larger relation by a random factor between 0,1...10.
Finally the two input components are retracted and the resulting component and the generated predicate 
are asserted as global facts.
~Factor is not used~

*/
joinQueryComponents(QueryComponent1, QueryComponent2, _):-
  QueryComponent1 = queryComponent(relList(RelList1)),
  QueryComponent2 = queryComponent(relList(RelList2)),
  append(RelList1, RelList2, RelListNew),
  selectRandomListElement(RelList1, R1),
  selectRandomListElement(RelList2, R2),
  generatePredicate(R1, R2, _, Pred),
  assert(queryPred(Pred)),
  assert(queryComponent(relList(RelListNew))),
  retract(QueryComponent1),
  retract(QueryComponent2).  

/*

1.6 selectRandomListElement

*/

/*
----    selectRandomListElement(+List, -Element)
----

Description:
Returns a random element ~Element~ of the input ~List~

*/
selectRandomListElement(List, Element) :-
  length(List, Length),
  %random(+L:int, +U:int, -R:int) is det
  %Binds R to a random number in [L,U). 
  %If L and U are both integers, R is an integer, 
  %Otherwise, R is a float. Note that U will never be generated. 
  random(0, Length, Index),  
  nth0(Index, List, Element).

/*

1.7 concatrelname

*/

/*
----    concatrelname(+N, -RelName)
----

Description:
Concats the name of the relation ~RelName~ from the input number ~N~.

*/
concatrelname(N, RelName):-
  number(N),
  atom_concat('r', N, RelName).

/*

1.8 getAttrName

*/

/*
----    getAttrName(+RelName, -AttrName)
----

Description:

Concats the attribute name ~AttrName~ from the name of the relation ~RelName~

*/

getAttrName(RelName, AttrName):-
 atom_concat(RelName,'i',AttrName).

/*

1.9 generatePredicate

*/ 

/*
----    generatePredicate(+N1, +N2, +Factor, -Pred)
----

Description:

Generates a join ~Predicate~ for a relation number ~N1~ and ~N2~ . 

Input:
~N1~: Number of the first input relation
~N2~: Number of the second input relation
~Factor~: not used

Output:
~Pred~: generated predicate

*/
generatePredicate(N1, N2, _, Pred):-
  concatrelname(N1, R1),
  concatrelname(N2, R2),
  compareRelations(R1, R2, RelSmaller, RelLarger, _, _),  
  getAttrName(RelLarger, AttrLarger),
  getAttrName(RelSmaller, AttrSmaller),  
  genPred(AttrSmaller, AttrLarger,Pred).   

/*

1.10 genPred

*/
  
/*
----    genPred(+AttrSmaller, +AttrLarger, -Pred)
----

Description:

Generates a join predicate ~Pred~ for the two participating relations represented by the 
attribute names ~AttrSmaller~ and ~AttrLarger~.

Input:

~AttrSmaller~: Attribute name of the smaller relation
~AttrLarger~: Attribute name of the larger relation

Output:
~Pred~: generated predicate

*/  

genPred(AttrSmaller, AttrLarger,Pred):-
  %concat_atom([AttrSmaller,'=',AttrLarger], Pred).
  Pred = (AttrSmaller=AttrLarger).

 

list2codes([], "").

list2codes([Atom], Codes) :- atom_codes(Atom, Codes).

list2codes([Atom|ListTail], Codes) :-
    atom_codes(Atom, AtomCodes),
    append(AtomCodes, ",", AtomCodesWithComma),
    append(AtomCodesWithComma, ListTailCodes, Codes),
    list2codes(ListTail, ListTailCodes).

/*

1.11 list2string

*/

/*
----    list2string(+List, -String)
----

Description:
Converts an input ~List~ to an output ~String~

*/
list2string(List, String) :-
    ground(List),
    list2codes(List, Codes),
    atom_codes(String1, Codes),
    concat_atom(['[',String1,']'],String).

/*

1.12 sqlbenchmark

*/

/*
----    sqlbenchmark(+Term, -PlanBuild, -PlanExec)
----

Description:

Executes an sql term ~Term~ and returns the optimization time ~PlanBuild~ in ms
and the execution time ~PlanExec~ in ms.

Input:
~Term~: sql term (e.g. select count([star]) from ... where...)

Output:
~PlanBuild~: Optimization time in ms
~PlanExec~:  Execution time in ms

*/

sqlbenchmark(Term, PlanBuild, PlanExec) :- defaultExceptionHandler((
  isDatabaseOpen,
  getTime( mOptimize(Term, Query, Cost), PlanBuild ),
  nl, write('The best plan is: '), nl, nl, write(Query), nl, nl,
  write('Estimated Cost: '), write(Cost), nl, nl,
  query(Query, PlanExec),
  appendToRel('SqlHistory', Term, Query, Cost, PlanBuild, PlanExec)
 )).

/*

1.13 testloop

*/

/*
----    testloop(+NumberLoops, +AkkOpt, +AkkExec, -ResultOpt, 
        -ResultExec, +NumberRels)

----

Description:

Generates Queries and executes them in a loop. Akkkumulates optimization time and execution time.

Input:

~NumberLoops~: Number of test loops

~AkkOpt~:  Akkumulator for optimization time (should be initialized wit 0)

~AkkExec~: Akkumulator for execution time (should be initialized wit 0)

~NumberRels~: Number of Relations (for query generation) 

Output:

~ResultOpt~:  Resulting optimization time in ms 

~ResultExec~: Resulting execution time in ms

*/

testloop(0, ResultOpt, ResultExec, ResultOpt, ResultExec, _).

testloop(N, AkkOpt, AkkExec, ResultOpt, ResultExec, NumberRels):-
  generateQueryTerm(NumberRels, Query),           
  nl,write('Query:'),nl,
  nl,write(Query),
  
  sqlbenchmark(Query, OptTime, ExecTime),
  NewAkkOpt is AkkOpt + OptTime,
  NewAkkExec is AkkExec + ExecTime, 
  
  /*
  sqlbenchmark(Query, OptTime1, ExecTime1),
  sqlbenchmark(Query, OptTime2, ExecTime2),
  sqlbenchmark(Query, OptTime3, ExecTime3),
  sqlbenchmark(Query, OptTime4, ExecTime4),  
  NewAkkOpt is AkkOpt + 
  ((OptTime1 + OptTime2 + OptTime3 + OptTime4)/4), 
  NewAkkExec is AkkExec + 
  ((ExecTime1 + ExecTime2 + ExecTime3 + ExecTime4)/4),
  */
  
  NewN is N-1,
  dropTempRels,
  testloop(NewN,NewAkkOpt, NewAkkExec, ResultOpt, 
  ResultExec, NumberRels). 

/* 

1.14 benchmark

*/

/*
----    benchmark(+DBName, +NumberLoops, +NumberRels, 
        -AverageResultOpt, -AverageResultExec)
----

Description:

Generates in a loop ~NumberLoops~ random queries over ~NumberRels~ relations and returns the average optimization time, 
average execution time (and average query processing time as textual output).

Input:


~DBName~: Name of the database

~NumberLoops~: Number of test loops

~NumberRels~: Number of predicates in a single query

Output:

~AverageResultOpt~: Average optimization time, 

~AverageResultExec~: Average execution time 

Average query processing time
as textual output.

call e.g. benchmark(testdb50, 30, 50).

*/  

benchmark(DBName, NumberLoops, NumberRels, 
AverageResultOpt, AverageResultExec):-
  open database DBName,                  
  deleteAllSamples,
  closedb,
  open database DBName,                  
  testloop(NumberLoops,0,0,ResultOpt,ResultExec, NumberRels),  %!!
  AverageResultOpt is ResultOpt/NumberLoops,      
  AverageResultExec is ResultExec/NumberLoops,     
  nl,write('DBName: '), write(DBName),
  nl,write('Number of Predicates: '), write(NumberRels),
  nl,write('Testloops: '), write(NumberLoops),

  nl,write('Average Optimization Time: '), write(AverageResultOpt),
  nl,write('AVerage Execution Time: '), write(AverageResultExec),
  nl,write('AVerage Query Processing Time: '), 
  AverageQueryProcessingTime is AverageResultOpt + AverageResultExec,
  write(AverageQueryProcessingTime),
  closedb.

/*

1.15 benchmarkComponentSize

*/

/*
----    benchmarkComponentSize
----

Description:

Returns average optimization time, average execution time and average processing time
of a given query, depending on the component size (maximum number of edges per component)

*/

benchmarkComponentSize:-
  setOption(largeQueries(qgd)),
  setMaxEdgesPerComponent(10),
  benchmark(testdb120A, 1, 120, AvgOpt10, AvgExec10),
  setMaxEdgesPerComponent(9),
  benchmark(testdb120A, 1, 120, AvgOpt9, AvgExec9),
  setMaxEdgesPerComponent(8),
  benchmark(testdb120A, 1, 120, AvgOpt8, AvgExec8),
  setMaxEdgesPerComponent(7),
  benchmark(testdb120A, 1, 120, AvgOpt7, AvgExec7),
  setMaxEdgesPerComponent(6),
  benchmark(testdb120A, 1, 120, AvgOpt6, AvgExec6),
  setMaxEdgesPerComponent(5),
  benchmark(testdb120A, 1, 120, AvgOpt5, AvgExec5),
  nl,write('10'),
  nl,write('Average Opt:  '),write(AvgOpt10),
  nl,write('Average Exec: '),write(AvgExec10),
  Avg10 is AvgOpt10+AvgExec10,
  nl,write('Average Proc: '),write(Avg10),nl,
  nl,write('9'),
  nl,write('Average Opt:  '),write(AvgOpt9),
  nl,write('Average Exec: '),write(AvgExec9),
  Avg9 is AvgOpt9+AvgExec9,
  nl,write('Average Proc: '),write(Avg9),nl,
  nl,write('8'),
  nl,write('Average Opt:  '),write(AvgOpt8),
  nl,write('Average Exec: '),write(AvgExec8),
  Avg8 is AvgOpt8+AvgExec8,
  nl,write('Average Proc: '),write(Avg8),nl,
  nl,write('7'),
  nl,write('Average Opt:  '),write(AvgOpt7),
  nl,write('Average Exec: '),write(AvgExec7),
  Avg7 is AvgOpt7+AvgExec7,
  nl,write('Average Proc: '),write(Avg7),nl,
  nl,write('6'),
  nl,write('Average Opt:  '),write(AvgOpt6),
  nl,write('Average Exec: '),write(AvgExec6),
  Avg6 is AvgOpt6+AvgExec6,
  nl,write('Average Proc: '),write(Avg6),nl,
  nl,write('5'),
  nl,write('Average Opt:  '),write(AvgOpt5),
  nl,write('Average Exec: '),write(AvgExec5),
  Avg5 is AvgOpt5+AvgExec5,
  nl,write('Average Proc: '),write(Avg5),nl.


 

benchmarkComponentSizeSmall :-
  setOption(largeQueries(qgdm)),
  open database testdb40A,
  deleteAllSamples,
  generateQueryTerm(40, Query),           
  nl,write('Query:'),nl,
  nl,write(Query),
  setMaxEdgesPerComponent(2),
  sqlbenchmark(Query, AvgOpt2, AvgExec2),
  deleteAllSamples,
  setMaxEdgesPerComponent(3),
  sqlbenchmark(Query, AvgOpt3, AvgExec3),
  deleteAllSamples,
  setMaxEdgesPerComponent(4),
  sqlbenchmark(Query, AvgOpt4, AvgExec4),
  nl,write('2'),
  nl,write('Average Opt:  '),write(AvgOpt2),
  nl,write('Average Exec: '),write(AvgExec2),
  Avg2 is AvgOpt2 + AvgExec2,
  nl,write('Average Proc: '),write(Avg2),nl,
  nl,write('3'),
  nl,write('Average Opt:  '),write(AvgOpt3),
  nl,write('Average Exec: '),write(AvgExec3),
  Avg3 is AvgOpt3 + AvgExec3,
  nl,write('Average Proc: '),write(Avg3),nl,
  nl,write('4'),
  nl,write('Average Opt:  '),write(AvgOpt4),
  nl,write('Average Exec: '),write(AvgExec4),
  Avg4 is AvgOpt4 + AvgExec4,
  nl,write('Average Proc: '),write(Avg4),nl,
  closedb.

/*

1.16 benchmarkStandardOptimizer

*/

/*
----    benchmarkStandardOptimizer
----

Description:

Returns average optimization time
of queries with n predicates (n is varied in the range [1..11]),
Time is average optimization time from 30 random generated queries with simple equi-join predicates.

*/

benchmarkStandardOptimizer:-  
  delOption(largeQueries(qgd)),
  delOption(largeQueries(qgdm)),
  delOption(largeQueries(aco)),  
  open database testdb12A,                  
  deleteAllSamples,
  closedb,
  benchmark(testdb12A, 30, 2, AvgOpt1, _),
  benchmark(testdb12A, 30, 3, AvgOpt2, _),
  benchmark(testdb12A, 30, 4, AvgOpt3, _),
  benchmark(testdb12A, 30, 5, AvgOpt4, _),
  benchmark(testdb12A, 30, 6, AvgOpt5, _),
  benchmark(testdb12A, 30, 7, AvgOpt6, _),
  benchmark(testdb12A, 30, 8, AvgOpt7, _),
  benchmark(testdb12A, 30, 9, AvgOpt8, _),
  benchmark(testdb12A, 30, 10, AvgOpt9, _),
  benchmark(testdb12A, 30, 11, AvgOpt10, _),
  benchmark(testdb12A, 30, 12, AvgOpt11, _),
  nl,write('Average Opt:  '),nl,
  nl,write('1: '), write(AvgOpt1),
  nl,write('2: '), write(AvgOpt2),
  nl,write('3: '), write(AvgOpt3),
  nl,write('4: '), write(AvgOpt4),
  nl,write('5: '), write(AvgOpt5),
  nl,write('6: '), write(AvgOpt6),
  nl,write('7: '), write(AvgOpt7),
  nl,write('8: '), write(AvgOpt8),
  nl,write('9: '), write(AvgOpt9),
  nl,write('10: '), write(AvgOpt10),
  nl,write('11: '), write(AvgOpt11),nl.

/*

1.17 deleteAllSamples

*/

/*
----    deleteAllSamples
----

Description:

Deletes all samples from the current opened database

*/

deleteAllSamples :-
  retractall(rewriteCache(_, _)),
  findall(Rel, ( databaseName(DB),
                 storedRel(DB, Rel, _),        
         (atom_concat(_, '_sample_s', Rel);
         atom_concat(_, '_sample_j', Rel))
        ), RelList),
  write(RelList),
  write_list(['\nINFO:\tRemoving samples...']), nl,
  catch(deleteTempRels(RelList), _, true).


dropRelList([]).

dropRelList([H|Tail]):-
  drop_Relation(H),
  dropRelList(Tail).

  
 
compareRelations(RelName1, RelName2, SmallerRel, LargerRel, 
CardSmallerRel, CardLargerRel):-
 card(RelName1, CardR1),
 card(RelName2, CardR2),
 CardR1 >= CardR2,
 CardSmallerRel is CardR2,
 CardLargerRel is CardR1,
 SmallerRel = RelName2,
 LargerRel = RelName1.

compareRelations(RelName1, RelName2, SmallerRel, LargerRel, 
CardSmallerRel, CardLargerRel):-
 card(RelName1, CardR1),
 card(RelName2, CardR2),
 CardR1 < CardR2,
 CardSmallerRel is CardR1,
 CardLargerRel is CardR2,
 SmallerRel = RelName1,
 LargerRel = RelName2.
 


createQueryComponents(1,[queryComponent(relList([1]))]):-
  assert(queryComponent(relList([1]))).

createQueryComponents(N,[queryComponent(relList([N]))|RestList]) :-
  assert(queryComponent(relList([N]))),
  Nminus1 is N-1,
  createQueryComponents(Nminus1, RestList).    
 

/*
----    optimalitybenchmark
----

Description:

Compares in a loop of 100 Queries with 10 predicates each
the average optimzation time, execution time and 
processing time of the algorithms 
STD (standard optimizer), ACO, QGD and QGDM.

*/

optimalitybenchmark:-
  delOption(largeQueries(qgd)),
  delOption(largeQueries(qgdm)),
  delOption(largeQueries(aco)),  
  benchmark(testdb11,100, 11, OptTimeSTD, ExecTimeSTD), 
  ProcTimeSTD is OptTimeSTD + ExecTimeSTD, 
  setOption(largeQueries(aco)),
  benchmark(testdb11,100, 11, OptTimeACO, ExecTimeACO),  
  ProcTimeACO is OptTimeACO + ExecTimeACO, 
  setOption(largeQueries(qgd)),
  benchmark(testdb11,100, 11, OptTimeQGD, ExecTimeQGD),
  ProcTimeQGD is OptTimeQGD + ExecTimeQGD, 
  setOption(largeQueries(qgdm)),
  benchmark(testdb11,100, 11, OptTimeQGDM, ExecTimeQGDM), 
  ProcTimeQGDM is OptTimeQGDM + ExecTimeQGDM,  
  nl,write('STD:OptTime :  '),write(OptTimeSTD),
  nl,write('STD:ExecTime:  '),write(ExecTimeSTD),
  nl,write('STD:ProcTime:  '),write(ProcTimeSTD),
  nl,write('ACO:OptTime :  '),write(OptTimeACO),
  nl,write('ACO:ExecTime:  '),write(ExecTimeACO),
  nl,write('ACO:ProcTime:  '),write(ProcTimeACO),
  nl,write('QGD:OptTime :  '),write(OptTimeQGD),
  nl,write('QGD:ExecTime:  '),write(ExecTimeQGD),
  nl,write('QGD:ProcTime:  '),write(ProcTimeQGD),
  nl,write('QGDM:OptTime :  '),write(OptTimeQGDM),
  nl,write('QGDM:ExecTime:  '),write(ExecTimeQGDM),
  nl,write('QGDM:ProcTime:  '),write(ProcTimeQGDM).


/*
----    largeRelationsbenchmark
----

Description:

Compares Queries over 16 relations (15 predicates) 
where 50% of the relations are of class XXL (1000000 tuples)
the average optimzation time, execution time and 
processing time of the algorithms 
 ACO, QGD and QGDM.

*/

largeRelationsbenchmark:-
  setOption(largeQueries(aco)),
  benchmark(testdb16,5, 16, OptTimeACO, ExecTimeACO),  
  ProcTimeACO is OptTimeACO + ExecTimeACO, 
  setOption(largeQueries(qgd)),
  benchmark(testdb16,5, 16, OptTimeQGD, ExecTimeQGD),
  ProcTimeQGD is OptTimeQGD + ExecTimeQGD, 
  setOption(largeQueries(qgdm)),
  benchmark(testdb16,5, 16, OptTimeQGDM, ExecTimeQGDM), 
  ProcTimeQGDM is OptTimeQGDM + ExecTimeQGDM,  
  nl,write('ACO:OptTime :  '),write(OptTimeACO),
  nl,write('ACO:ExecTime:  '),write(ExecTimeACO),
  nl,write('ACO:ProcTime:  '),write(ProcTimeACO),
  nl,write('QGD:OptTime :  '),write(OptTimeQGD),
  nl,write('QGD:ExecTime:  '),write(ExecTimeQGD),
  nl,write('QGD:ProcTime:  '),write(ProcTimeQGD),
  nl,write('QGDM:OptTime :  '),write(OptTimeQGDM),
  nl,write('QGDM:ExecTime:  '),write(ExecTimeQGDM),
  nl,write('QGDM:ProcTime:  '),write(ProcTimeQGDM).




   
