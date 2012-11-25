/*
$Header$
@author Nikolai van Kempen

- Provides predicates to test the nonlinear optimization.

- Provides predicates and queries to test the memory distribution.

Hint:
For more complex queries with many join operatores like the hybridhashjoin, you might 
need to increase the open files limit with ulimit or by adding the line (depending on your current os):

* - nofile 16384

to the /etc/securitiy/limits.conf file.

*/

% The exact result should assign the maximum memory to X1. Then 
% rest can be assigned to X2.
testNLOPT(1, [X1, X2], [2000-(2*X1), 1000-X2], [[512, 1, 1]], [40,50]).

testNLOPT(2, [X, Y], [100000/X, 100000/Y], [[512, 1, 1]], [400,400]).
% multi-constraints
testNLOPT(3, [X, Y], [10000-X, 20000-Y], [[512, 1, 0],[512,0,1]], [400, 600]).

% A degenerated cost function, this is not allowed(non decreasing), but 
% nevertheless, it works here, of course.
% Exact result is [MinOpMemory, MinOpMemory]
testNLOPT(100, [X, Y], [X, Y], [[512, 1, 1]], [40,50]).

% Another non allowed formula because the result may be below zero.
testNLOPT(101, [X, Y], [100-X, 200-Y], [[512, 1, 1]], [40, 60]).

% Example used in my bachelor thesis
testNLOPT(102, [X, Y], [800-1.5*X, (10000/Y)+100], [[512, 1, 1]], [512, 512]).

testNLOPT(103, [X, Y], [327059.2481927711-X*325.45301204819276, 14982.5-Y*0.0], 
	[[256, 1, 1]], [421,11]).

% constant funtions...useless contraints
testNLOPT(104, [X, Y, Z], [17354.521447872-X*0.0, 2348898.504871901-Y*0.0,
	0.0/Z+794284.2725344], [[255,1,0,0],[254,1,1,0],[254,0,1,1]], [54,7232,8493]).

testNLOPT(105, [X, Y, Z], [17354.521447872-X*1.0, 2348898.504871901-Y*1.0,
	1.0/Z+794284.2725344], [[255,1,0,0],[254,1,1,0],[254,0,1,1]], [54,7232,8493]).

testNLOPT(444, [A,B,C], 
	[10000-(10*A),10000-(10*B),10000-(10*C)],
	[[512,1,1,1]], 
	[400, 600, 17]).

testNLOPT(1000, [A,B,C,D, E,F,G,H], 
	[10000-(10*A),10000-(10*B),10000-(10*C),(100000/D)+1000,10000-(10*E),10000-(10*F),10000-(10*G),10000-(10*H)],
	[[512,1,1,1,1,1,1,1,1]], 
	[400, 600, 17, 512, 1024, 1000, 233, 30]).

testNLOPT(2000, [X1,X2,X3,X4,X5,X6,X7,X8,X9,X10,X11,X12,X13,X14,X15,X16,X17
,X18,X19,X20], 
	[
	(59515200.0/X1)+48152.6,
	(59515200.0/X2)+48152.6,
	(59515200.0/X3)+48152.6,
	(59515200.0/X4)+48152.6,
	(59515200.0/X5)+48152.6,
	(59515200.0/X6)+48152.6,
	(59515200.0/X7)+48152.6,
	(59515200.0/X8)+48152.6,
	(59515200.0/X9)+48152.6,
	(59515200.0/X10)+48152.6,
	(59515200.0/X11)+48152.6,
	(59515200.0/X12)+48152.6,
	(59515200.0/X13)+48152.6,
	(1259515200.0/X14)+0,
	59515200.0-(X15*1000000),
	(59515200.0/X16)+8152.6,
	(59515200.0/X17)+152.6,
	(59515200.0/X18)+52.6,
	(59515200.0/X19)+48152.6,
	(59515200.0/X20)+48152.6
	],
	[[512,1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1]], 
	[480, 480, 900, 17, 100, 999, 11234, 19, 44, 345,
	480, 480, 900, 170, 50, 999, 11234, 19, 44, 21
	]).

testNLOPT(2001, [X1,X2,X3,_X4,X5,X6,X7,X8,X9,X10,X11,X12,X13,X14,X15,X16,X17
,X18,X19,X20], 
  [
  (59515200.0/X1)+48152.6,
  (59515200.0/X2)+48152.6,
  (59515200.0/X3)+48152.6,
  48152.6,
  (59515200.0/X5)+48152.6,
  (59515200.0/X6)+48152.6,
  (59515200.0/X7)+48152.6,
  (59515200.0/X8)+48152.6,
  (59515200.0/X9)+48152.6,
  (59515200.0/X10)+48152.6,
  (59515200.0/X11)+48152.6,
  (59515200.0/X12)+48152.6,
  (59515200.0/X13)+48152.6,
  (1259515200.0/X14)+0,
  59515200.0-(X15*1000000),
  (59515200.0/X16)+8152.6,
  (59515200.0/X17)+152.6,
  (59515200.0/X18)+52.6,
  (59515200.0/X19)+48152.6,
  (59515200.0/X20)+48152.6
  ],
  [
		[512,1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0],
		[512,0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0],
		[512,0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1]
	], 
  [480, 480, 900, 17, 100, 999, 11234, 19, 44, 345,
  480, 480, 900, 170, 50, 999, 11234, 19, 44, 21
  ]).

testNLOPT(No) :-
	testNLOPT(No, V, F, C, SL),
	testNLOPT(V, F, C, SL).
	
testNLOPT :-
	ensure(testNLOPT(_)),
	fail.
testNLOPT.

/*
Testing predicate for a given formular.

Assumes a minimum amount of memory for every operator of 16 MiB.
The maximum is fix 512 MiB.

testNLOPT(+VarList, +Formulas, +ConstraintsV, +SufficientMemoryList)
*/
testNLOPT(VarList, Formulas, ConstraintsV, SufficientMemoryList) :-
	nl,
	(member(16, SufficientMemoryList) ->
		% Just make it 17 to make the optimizaion work, see comments
		% within the other files for more information.
		% in short: MMA does not support equal-constraints.
		throw('do not use 16 MiB within the sufficient memory values list.')
	;
		true
	),
	newOpt,
	buildTestFormula(VarList, VarList, 0, Formulas, Formula),
	!,
 	setFormula(VarList, Formula),
  maFormula(VarList, _),
  length(VarList, Len),
  write_list(['\nVariables       : ', VarList, '\n']),
  write_list(['Formula         : ', Formula, '\n']),
  write_list(['Constraints     : ', ConstraintsV, '\n']),
  write_list(['SufficientMemory: ', SufficientMemoryList, '\n']),
  getTime(memoryOptimization(16, 512, ConstraintsV, Len, 
		SufficientMemoryList, X), TimeMS),
	nl,
  write_list(['NLOPT-Result: ', X, '\n']),
  write_list(['Time        : ', TimeMS, 'ms\n']),
  maFormula(X, F2),
	Costs is F2,
  write_list(['Total costs : ', Costs, '\n']).

buildTestFormula([Var], AllVars, Dimension, [F], F) :-
	!,
  derivate(F, Var, DX),
  setDerivative(Dimension, AllVars, DX),
  dm(ma3, ['\nMVar: ', Var, ' D: ', Dimension, ' DX: ', DX, ' F: ', F]).

buildTestFormula([Var|VarRest], AllVars, Dimension, [F|Rest], Formula) :-
	!,
	buildTestFormula([Var], AllVars, Dimension, [F], OutF),
	DNew is Dimension + 1,
	buildTestFormula(VarRest, AllVars, DNew, Rest, RestFormula),
	Formula = OutF + RestFormula.


	

testMAQuery(1, [database(opt)], select * from orte).
testMAQuery(2, [], select bevt from orte).
testMAQuery(3, [], select [bevt, kennzeichen] from orte).
testMAQuery(4, [], select * from orte as oXy).
testMAQuery(5, [], select oXy:bevt from orte as oXy).
testMAQuery(6, [], select [oXy:bevt, oXy:kennzeichen] from orte as oXy).
testMAQuery(7, [], select [oXy:bevt, oXy:kennzeichen] from orte as oXy where [oXy:bevt>10, oXy:bevt<1000]).
testMAQuery(8, [], select * from [orte as o, plz as p] where o:ort=p:ort).
testMAQuery(9, [],  select [ort, min(plz) as minplz, max(plz) as maxplz,  count(*) as cntplz] from plz where plz > 40000 groupby ort).
testMAQuery(10, [], select [ort, plz] from plz orderby [ort asc, plz desc]).
testMAQuery(11, [], select [ort, plz] from plz where ort="Berlin" orderby [ort asc, plz desc]).


testMAQuery(100, [], select (*)from[staedte, plz as p1, plz as p2, plz as p3]where[sname=p1:ort, p1:plz=p2:plz+1, p2:plz=p3:plz*5] first 1).
testMAQuery(101, [], select (*)from[staedte, plz as p1, plz as p2, plz as p3]where[sname=p1:ort, p1:plz=p2:plz+1, p2:plz=p3:plz*5] orderby[sname] first 1).
testMAQuery(102, [], select (*)from[staedte, plz as p1, plz as p2, plz as p3, plz as p4]where[sname=p1:ort, p1:plz=p2:plz, p2:plz=p3:plz, p3:plz=p4:plz] orderby[sname] first 1).
testMAQuery(103, [], select (*)from[staedte, plz as p1, plz as p2, plz as p3, plz as p4]where[sname=p1:ort, p1:plz=p2:plz*3, p2:plz=p3:plz*4, p3:plz=p4:plz*5] orderby[sname] first 1).

testMAQuery(1000, [database(nrw2)], select * from [buildings as b, roads as r] where[b:osm_id=r:osm_id]).
testMAQuery(1010, [database(nrw2)], select * from [points as b, waterways as r] where[b:osm_id=r:osm_id]).
testMAQuery(1001, [], select * from [roads as r, buildings as b] where[r:osm_id=b:osm_id]).
testMAQuery(1002, [], select * from [buildings as b, roads as r, buildings as bu] where[b:osm_id=r:osm_id, r:osm_id=bu:osm_id] first 1).
testMAQuery(1003, [], select * from [buildings as b, roads as r, buildings as bu, points as p] where[b:osm_id=r:osm_id, r:osm_id=bu:osm_id, bu:osm_id=p:osm_id]).
testMAQuery(1004, [],  select count(*) from [buildings as b, roads as r] where[div(b:osm_id, 100000)=div(r:osm_id, 100000)]).

% Just run the queries
testMA :-
	testMA((currentTestCase(Q), sql(Q))).

% compute statistics
testMA(TestGoal) :-
  reload, % this is to simplify my testings, but when using this, the 
  % catalogs should be reloaded.
  retractall(testResult(_, _, _, _, _)),
  retractall(testRunning),
  asserta(testRunning),
	delOption(nestedRelations),
	setOption(memoryAllocation),
  (
    testMAQuery(No, Properties, Query),
		% write_list(['\nTest No: ', No]),
  	processProperties(Properties),
		retractall(currentTestCase(_)),
		asserta(currentTestCase(Query)),
    % So we check if this is provable, not if the result is correct.
    % That that sql catches exceptions, otherwise more must be done here.
    (getTime(call(TestGoal), TimeMS) ->
      addResult(Properties, No, TimeMS, ok)
    ;
      addResult(Properties, No, -1, failed)
    ),
    fail
  ).

testMA(_TestGoal) :-
  !,
  retractall(testRunning),
  showTestResults.

% eof
