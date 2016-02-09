/* 
Fakten zum Tranformieren von Queries 
zuerst Typen zum konvertieren
dann die Ergebnisstruktur eines parallelen Queries

*/

secondoPrologType(type, 'Type').
secondoPrologType(element, 'ELEMENT').
secondoPrologType(darray2elem, 'DARRAYELEM').
secondoPrologType(darray2elem2, 'DARRAYELEM2').
secondoPrologType(streamelem, 'STREAMELEM').

/* 
Parallel ohne tie[] Operator - bei operator head[Int] 
wird aktuell noch die Anzahl mal worker zurueckgeliefert

*/

secondoPrologParallelQuery([RELATION | [] ], FUNCTIONS, [query, [consume,
                            [dsummarize, [dloop, RELATION,'\'""""\'',
                            [fun, [elem11,'DARRAYELEM'], FUNCTIONS ]]]]]).

/*
Parallel zusaetzlich mit Join Moeglichkeit ohne tie[] 
Operator - bei operator head[Int] wird aktuell noch die 
Anzahl * worker zurueckgeliefert

*/
secondoPrologParallelQuery(
          [RELATION1 | [RELATION2]], FUNCTIONS, [query, [consume, [dsummarize, 
          [dloop2, RELATION1, RELATION2,'\'""""\'',
          [fun, [elem11,'DARRAYELEM'], [elem22,'DARRAYELEM2'], 
          FUNCTIONS ]]]]]).

/*
Parallel inkl. tie[] Operator speziell fuer min-operator
 (wegen sortarray geht es aktuell nur mit int und reals) 

*/
secondoPrologParallelQueryMin(
       [RELATION | [] ], FUNCTIONS, [query, [get, [sortarray,
       [getValue, [dloop, RELATION,'\'""""\'', [fun, [elem11,'DARRAYELEM'],
       FUNCTIONS ]]], [fun, [element2, 'ELEMENT'], ['-', element2,
       ['*', element2, element2]]]], 0]]).

/*
Parallel ohne. tie[] Operator speziell fuer max-operator (wegen 
sortarray geht es aktuell nur mit int und reals) 

*/
secondoPrologParallelQueryMax(
      [RELATION | [] ], FUNCTIONS, [query,
      [get, [sortarray, [getValue, [dloop, RELATION,'\'""""\'',
      [fun, [elem11,'DARRAYELEM'], FUNCTIONS ]]], [fun, [element2, 'ELEMENT'],
      ['-', element2, ['*', element2, element2]]]], [size, RELATION]]]).

/* 
Parallel inkl. tie[] Operator speziell fuer avg-operator 

*/
secondoPrologParallelQueryAvg(
          [RELATION | [] ], FUNCTIONS, [query, ['/', [tie,
          [getValue, [dloop, RELATION,'\'""""\'', [fun, [elem11,'DARRAYELEM'], 
          FUNCTIONS ]]],
          [fun,[first3,'ELEMENT'],[second4,'ELEMENT'],['+',first3,second4]]],
          [size, RELATION]]]).

/*
 Parallel inkl. tie[] Operator, funktioniert aktuell nur mit 'count' und 'sum' 

*/
secondoPrologParallelQueryTie(
          [RELATION | [] ], FUNCTIONS, [query, [tie, 
          [getValue, [dloop, RELATION,'\'""""\'', [fun, [elem11,'DARRAYELEM'],
          FUNCTIONS ]]], [fun,[first3,'ELEMENT'],
          [second4,'ELEMENT'],['+',first3,second4]]]]).

/* 

Parallel zusaetzlich mit Join Moeglichkeit inkl. tie[] Operator

*/
secondoPrologParallelQueryTie(
        [RELATION1 | [RELATION2]], FUNCTIONS, [query,
        [tie,[getValue,[dloop2, RELATION1, RELATION2,'\'""""\'',[fun,
        [elem11,'DARRAYELEM'],[elem22,'DARRAYELEM2'], FUNCTIONS ]]],
       [fun,[first3,'ELEMENT'],[second4,'ELEMENT'],['+',first3,second4]]]]).

