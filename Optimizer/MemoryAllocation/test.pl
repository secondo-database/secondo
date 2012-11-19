/*
$Header$
@author Nikolai van Kempen

- Provides predicates to test the nonlinear optimization.

- Provides predicates with test queries for the memory distribution.

*/

reloadMA :-
  ['MemoryAllocation/ma.pl'],
  ['MemoryAllocation/madata.pl'],
  ['MemoryAllocation/ma_improvedcosts.pl'],
  ['MemoryAllocation/test.pl'].


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
%testMAQuery(4, [], select (*)from[staedte, plz as p1, plz as p2, plz as p3, plz as p4, plz as p5]where[sname=p1:ort, p1:plz=p2:plz, p2:plz=p3:plz, p3:plz=p4:plz, p4:plz=p5:plz] orderby[sname] first 1).
% writes to much on the filesystem.
%testMAQuery(5, [], select (*)from[staedte, plz as p1, plz as p2, plz as p3, plz as p4, plz as p5, plz as p6]where[sname=p1:ort, sname="xy", p1:plz=p2:plz, p2:plz=p3:plz, p3:plz=p4:plz, p4:plz=p5:plz, p5:plz=p6:plz] orderby[sname] first 1).

testMAQuery(1000, [database(nrw2)], select * from [buildings as b, roads as r] where[b:osm_id=r:osm_id]).
testMAQuery(1010, [database(nrw2)], select * from [points as b, waterways as r] where[b:osm_id=r:osm_id]).
testMAQuery(1001, [], select * from [roads as r, buildings as b] where[r:osm_id=b:osm_id]).
testMAQuery(1002, [], select * from [buildings as b, roads as r, buildings as bu] where[b:osm_id=r:osm_id, r:osm_id=bu:osm_id] first 1).
testMAQuery(1003, [], select * from [buildings as b, roads as r, buildings as bu, points as p] where[b:osm_id=r:osm_id, r:osm_id=bu:osm_id, bu:osm_id=p:osm_id]).
testMAQuery(1004, [],  select count(*) from [buildings as b, roads as r] where[div(b:osm_id, 100000)=div(r:osm_id, 100000)]).

% The nrw3 database is created by the nrwtest/testnrw3.sh script.
% The difference is that the counter are kept to produce specific join
% results.
% case 1
testMAQuery(2001, [database(nrw3)], select * from [points as p, railways as r] where[p:no=r:no]).

% case 2
testMAQuery(2000, [database(nrw3)], select * from [buildings as b, roads as r] where[b:no=r:no]).

% case 3
testMAQuery(2002, [database(nrw3)], select * from [buildings as b, roads as r, points as p] where[b:no=r:no,r:no=p:no]).

testMAQuery(8000, [database(nrw3)], select * from [points as p1, points as p2, points as p3, points as p4, points as p5] where[p1:no=p2:no,p2:no=p3:no,p3:no=p4:no,p4:no=p5:no]).
testMAQuery(8001, [database(nrw3)], select * from [points as r1, buildings200k as r2, roads200k as r3, roads200k as r4, roads300k as r5] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no]).
testMAQuery(8002, [database(nrw3)], select * from [points as r1, buildings300k as r2, roads300k as r3, roads300k as r4, buildings200k as r5] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no]).
testMAQuery(8003, [database(nrw3)], select * from [points as r1, buildings300k as r2, roads300k as r3, roads400k as r4, buildings400k as r5] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no]).
testMAQuery(8004, [database(nrw3)], select * from [roads300k as r1, roads400k as r2, roads500k as r3, roads600k as r4, roads700k as r5] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no]).

testMAQuery(8010, [database(nrw3)], select * from [points as r1, roads500k as r2, buildings300k as r3, roads300k as r4, roads400k as r5, roads300k as r6] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no,r5:no=r6:no]).

testMAQuery(8011, [database(nrw3)], select * from [points as r1, roads as r2, roads100k as r3, roads200k as r4, roads300k as r5, roads400k as r6] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no, r5:no=r6:no]).

testMAQuery(5005, [database(nrw3)], select * from [points as p1, points as p2, points as p3, points as p4, points as p5, roads as r] where[p1:no=p2:no,p2:no=p3:no,p3:no=p4:no,p4:no=p5:no, p5:no=r:no]).

testMAQuery(2021, [database(nrw3)], select * from [buildings as b, roads as r, places as p] where[b:no=r:no,r:no=p:no]).

testMAQuery(2013, [database(nrw3)], select * from [buildings as b, roads as r, natural as n] where[b:no=r:no,r:no=p:no]).

% 162 cost edge paths for 2 pog paths:
testMAQuery(2014, [database(nrw3)], select * from [buildings as b, roads as r, roads as r2] where[b:no=r:no,b:no=r2:no]).

% 4374 cost edge paths for 6 pog paths:
% 2003
testMAQuery(2003, [database(nrw3)], select * from [buildings as b, roads as r, points as p, natural as n] where[b:no=r:no,r:no=p:no,p:no=n:no]).
testMAQuery(2033, [database(nrw3)], select * from [buildings as b1, buildings as b2, buildings as b3, buildings as b4] where[b1:no=b2:no,b2:no=b3:no,b3:no=b4:no]).


testMAQuery(2004, [database(nrw3)], select * from [buildings as b, roads as r, points as p, natural as n, buildings as b2] where[b:no=r:no,r:no=p:no,p:no=n:no, b2:no=n:no]).
% 157464  cost edge paths for 24 pog paths:
testMAQuery(2005, [database(nrw3)], select * from [buildings as b, roads as r, points as p, natural as n, buildings as b2] where[b:no=b2:no,b:no=r:no,r:no=p:no,p:no=n:no]).

% 7085880 cost edge paths for 120 pog paths:
% without new translation rules: 29160
testMAQuery(2006, [database(nrw3)], select * from [buildings as b, roads as r, points as p, roads as r2, natural as n, buildings as b2] where[b:no=b2:no,b:no=r:no,r:no=p:no,r2:no=p:no,p:no=n:no]).
%testMAQuery(2007, [database(nrw3)], select * from [buildings as b, roads as r, points as p, roads as r2, natural as n, buildings as b2, waterways as w] where[b:no=b2:no,b:no=r:no,r:no=p:no,r2:no=p:no,p:no=n:no, n:no=w:no]).

testMAQuery(2008, [database(nrw3)], select * from [buildings as b, roads as r, points as p, natural as n, waterways as w, roads as r2, buildings as b2] where[b:osm_id=r:osm_id,r:osm_id=p:osm_id,p:osm_id=n:osm_id,n:osm_id=w:osm_id,w:osm_id=b2:osm_id,b2:osm_id=r2:osm_id]).
testMAQuery(2018, [database(nrw3)], select * from [buildings as b, roads as r, points as p, natural as n, waterways as w, railways as rw, places as pl] where[b:osm_id=r:osm_id,r:osm_id=p:osm_id,p:osm_id=n:osm_id,n:osm_id=w:osm_id,w:osm_id=rw:osm_id,rw:osm_id=pl:osm_id]).

testMAQuery(3000, [database(nrw3)], select * from [points as p, railways as r] where[p:osm_id=r:osm_id]).

% -
testMAQuery(3001, [database(nrw3)], select * from [buildings as b1, buildings as b2] where[b1:no=b2:no]).
testMAQuery(3002, [database(nrw3)], select * from [buildings as b1, buildings as b2, buildings as b3] where[b1:no=b2:no, b2:no=b3:no]).


testMAQuery(4000, [database(nrw3)], select * from [buildings as b1, roads as r1, points as p, waterways as w, buildings as b2, roads as r2] where[b1:no=r1:no, b2:no=r2:no,w:osm_id=p:osm_id, w:osm_id=b2:osm_id,b1:no=w:osm_id]).

testMAQuery(5000, [database(nrw3)], select count(*) from [roads as r2, roads as w, buildings as p, buildings as b2] where [div(w:osm_id,1000000)=div(p:osm_id,100000), b2:no=p:no, b2:no=r2:no]).


%testMAQuery(6005, [database(nrw3)], select * from [buildings as b, roads as r, buildings as b2, roads as r2, buildings as b3] where[b:no=r:no, r:no=b2:no,b2:no=r2:no,r2:no=b3:no]).
%testMAQuery(6006, [database(nrw3)], select * from [buildings as b, roads as r, buildings as b2, roads as r2, buildings as b3, roads as r3] where[b:no=r:no, r:no=b2:no,b2:no=r2:no,r2:no=b3:no,b3:no=r3:no]).

testMAQuery(7005, [database(nrw3)], select * from [roads as b, roads as r, roads as b2, roads as r2, roads as b3] where[b:no=r:no, r:no=b2:no,b2:no=r2:no,r2:no=b3:no]).
testMAQuery(7006, [database(nrw3)], select * from [roads as b, roads as r, roads as b2, roads as r2, roads as b3, roads as r3] where[b:no=r:no, r:no=b2:no,b2:no=r2:no,r2:no=b3:no,b3:no=r3:no]).

testMAQuery(7007, [database(nrw3)], select * from [roads as b1, roads as b2, roads as b3] where[b1:no=b2:no, b2:no=b3:no]).
testMAQuery(7008, [database(nrw3)], select * from [roads as r1, roads as r2, roads as r3, roads as r4] where[r1:no=r2:no, r2:no=r3:no,r3:no=r4:no]).

createTestFilesX :-
	testMAByID(2001, 4, 4, 0), % Case 1
	testMAByID(2000, 4, 4, 0), % Case 2
	testMAByID(2002, 4, 4, 0), % Case 3
	testMAByID(2003, 4, 4, 0), % Case 4
	testMAByID(7007, 4, 4, 0),
	testMAByID(7008, 4, 4, 0),
	testMAByID(2005, 4, 4, 0),
	testMAByID(2006, 1, 4, 0),
	testMAByID(8000, 4, 4, 0),
	true.

createTestFiles :-
	testMAByID(8001, 4, 4, 0),
	testMAByID(8002, 4, 4, 0),
	%testMAByID(8003, 4, 4, 0),
	%testMAByID(2000, 4, 4, 0), %OK
	%testMAByID(2003, 4, 8, 0), %OK
	%testMAByID(2001, 4, 8, 0), %OK
	%testMAByID(2002, 4, 8, 0), %OK
	%testMAByID(2021, 4, 8, 0), %OK
	%testMAByID(7007, 4, 4, 0),
	%testMAByID(7008, 4, 4, 0),
	%testMAByID(2005, 4, 4, 0),
	%testMAByID(2006, 1, 4, 0),
	%testMAByID(2007, 1, 4, 0), NOK
	%testMAByID(2033, 4, 4, 0),
	%testMAByID(6005, 1, 3, 0),
	%testMAByID(6006, 1, 3, 0),
	true.
/*
	testMAByID(2013, 4, 4, 0),
	testMAByID(2014, 4, 4, 0),

	testMAByID(4000, 4, 4, 0),
	testMAByID(3000, 4, 4, 0),
	testMAByID(3001, 4, 4, 0),
	testMAByID(3002, 4, 4, 0), 
	testMAByID(2001, 4, 4, 0),
	%testMAByID(2002, 4, 4, 0),%OK
	testMAByID(2003, 4, 4, 0),
	%testMAByID(2004, 4, 4, 0),%OK
	%testMAByID(2005, 4, 4, 0),%OK
	testMAByID(2006, 4, 4, 0),
	%testMAByID(3001, 4, 4, 0),%OK
	testMAByID(2007, 4, 4, 0),
	testMAByID(2008, 4, 4, 10).
*/

/*
Creates the exact selectivities for the test queries.
*/
maCreateTestSels :-
	maCreateTestSels2.
maCreateTestSels :-
	!.

maCreateTestSels2 :-
	databaseName(DB),
	%Rels=['Buildings', 'Natural', 'Places', 'Points', 'Railways', 'Roads', 'Waterways'],
	findall(X, (
			storedRel(DB, R, _), 
			\+ sub_atom(R, _, _, _,sample),
			storedSpell(DB, R, S), 
			atomic(S),
			upper(S, X)
		), Rels),
	!,
	member(R1, Rels),
	member(R2, Rels),
	showRowCounts(R1, 'count', C1),
	showRowCounts(R2, 'count', C2),
	downcase_atom(R1, R1DC),
	downcase_atom(R2, R2DC),
	Min is min(C1, C2), % for the first Min rows, ~no~ exists once in 
											% both relations. Because of this special case, 
											% selectivities can be easiliy computed.
	Sel is Min / (C1*C2),
	retractall(storedSel(DB,R1DC:no=R2DC:no, _)),
	assertz(storedSel(DB,R1DC:no=R2DC:no, Sel)),
	fail.

testMAByID(No) :-
	testMAByID(No, 50, 10, 30).

testMAByID(No, CO, CQ, Sleep) :-
  testMAQuery(No, Properties, Query),
  processProperties(Properties),
	runMATest(No, tmode(opt(CO), query(CQ), sleep(Sleep)), Query).

runMATest(No, TMode, Query) :-
	maCreateTestSels,
	!,
	createExtendedEvalList(TList),
	setMAStrategies(TList),
	testStrategies(No, TMode, Query, TList, L),

	delOption(memoryAllocation),
	setOption(improvedcosts),
	%delOption(noHashjoin),
	sqltest(No, impcosts, TMode, Query, Atom, Costs, OptTime, ExecTime, OTimes1, 
		QTimes1, MInfo1),
	S=[improvedcosts, OptTime, ExecTime, Costs, Atom, OTimes1, QTimes1, MInfo1],
	delOption(improvedcosts),
	sqltest(No, stdcosts, TMode, Query, AtomSC, CostsSC, OptTimeSC, ExecTimeSC, 
		OTimes2, QTimes2, MInfo2),
	SC=[standardcosts, OptTimeSC, ExecTimeSC, CostsSC, AtomSC, OTimes2, 
    QTimes2, MInfo2],
	setOption(memoryAllocation),
	%setOption(noHashjoin),
	append(L, [S, SC], LNew),

  with_output_to(atom(A), processResults3(Query, LNew)),
  write(A),
 	get_time(TS), 
	convert_time(TS, Year, Month, Day, Hour, Minute, Sec, _MilliSec),
	atomic_list_concat([Year, '-', Month, '-', Day, ' ', Hour, ':', Minute, 
		':', Sec], DateTime),
	atomic_list_concat(['MemoryAllocation/test/matest',No,'.log'], FileName),
  open(FileName, write, FD),
	write(FD, '************ NEW ************\n'),
	write(FD, DateTime),
	write(FD, '\n'),
	write(FD, Query),
	write(FD, '\n'),
  write(FD, A),
  close(FD),
	atomic_list_concat(['MemoryAllocation/test/matest',No,'.term'], FileNameT),
  open(FileNameT, write, FDT),
	writeq(FDT, LNew),
	write(FDT, '.'),
  close(FDT).

deleteTestFile(File) :-
  atomic_list_concat(['MemoryAllocation/test/', File], FileName),
	(exists_file(FileName) ->
		delete_file(FileName)
	;
		true
	).

ensureDirExists(No) :-
  atomic_list_concat(['MemoryAllocation/test/', No], FileName),
	(exists_directory(FileName) ->
		true
	;
		make_directory(FileName)
	).

appendToTestFile(File, Data) :-
  atomic_list_concat(['MemoryAllocation/test/', File], FileName),
  open(FileName, append, FD),
  write(FD, Data),
  close(FD).

processResults(Query, LNew) :-
	% just store if later needed (manually)
	assertz(matestTestResultList(Query, LNew)), 

	write_list(['\nQuery: ', Query]), nl,
  FH='~w~30+ &~` t~w~9+ &~` t~w~11+   &~` t~w~12+   &~w &~w &~w\n \\\\',
  format(FH, ['Type', 'Costs', 'OptTime ms', 'ExecTime ms', 'Query', 'Query times', 'Memory']),
  findall(X, (
			member(X, LNew),
			X=[T, OT, ET, C, A, _OTimes, QTimes, MInfo|_],
    	F='~w~30+ &~` t~d~9+ &~` t~d~10+ &~` t~d~12+ &~w &~w &~w\\\\~n',
			ICosts is round(C),
    	format(F, [T, ICosts, OT, ET, A, QTimes, MInfo])
  	), _),
	nl.

processResults3(ID) :-
	atomic_list_concat(['MemoryAllocation/test/matest',ID,'.term'], FileName),
	open(FileName, read, FD),
	read(FD, Term),
	close(FD),
	processResults3(_, Term).

processResults3(Query, LNew) :-
  write_list(['\nQuery: ', Query]), nl,
  FH='~w~30+ &~` t~w~9+ &~` t~w~11+   &~` t~w~12+   &~w \\\\ ~n',
  format(FH, ['Type', 'Costs', 'OptTime ms', 'ExecTime ms', 'Memory']),
  %, 'Query', 'Query times', 'Memory']),
  findall(X, (
      member(X, LNew),
      X=[T, OT, ET, C, A, _OTimes, QTimes, MInfo|_],
      %F='~w~30+ &~` t~d~9+ &~` t~d~10+ &~` t~d~12+ &~w \\\\~n',
      ICosts is round(C),
      (MInfo = [] ->
        MI=''
      ;
        MInfo=[MI|_]
      ),
      %DATA=[T, ICosts, OT, ET, MI],
      %format(F, DATA),
    	F='~w~30+ &~` t~d~9+ &~` t~d~10+ &~` t~d~12+ &~w &~w \\\\~n',
    	format(F, [T, ICosts, OT, ET, A, MInfo])
    ), _),
  nl,
  findall(X, (
      member(X, LNew),
      X=[T, OT, ET, C, A, _OTimes, QTimes, MInfo|_],
      F='~w~30+ &~` t~d~9+ &~` t~d~10+ &~` t~d~12+ &~w \\\\~n',
      ICosts is round(C),
      (MInfo = [] ->
        MI=''
      ;
        MInfo=[MI|_]
      ),
      DATA=[T, ICosts, OT, ET, MI],
      length(DATA, LEN),
      latexListing(A, AL),
      %write_list(['\multicolumn{', LEN ,'}{|l|}{', AL, '}\\\\\n'])
      %write_list(['\\multicolumn{', LEN ,'}{l}{\\lstinline!', AL, '!}\\\\\n'])
      write_list(['# ', T, ':\n']),
      write_list([A, '\n'])
      %format(F, [T, ICosts, OT, ET, A, QTimes, MInfo])
    ), _),
  nl.

latexListing(A, AL) :-
	atom_chars(A, CharList),
	latexTranslate(CharList, CharListL),
	atom_chars(AL, CharListL).
	
latexTranslate([], []) :-
	!.

latexTranslateA('{', ['\\', '{']) :-
	!.
latexTranslateA('}', ['\\', '}']) :-
	!.
latexTranslateA(A, [A]) :-
	!.

latexTranslate([A|Rest], Out) :-
	latexTranslateA(A, ACharList),
	latexTranslate(Rest, RestCharList),
	append(ACharList, RestCharList, Out).
	
testStrategies(_No, _TMode, _Query, [], []) :- 
	!.
testStrategies(No, TMode, Query, [T|Rest], [S|SRest]) :-
	setMAStrategies([T]),
	sqltest(No, T, TMode, Query, Atom, Costs, OptTime, ExecTime, OTimes, QTimes, 
		MInfo),
  maResult(Label, _, _, _, _),
	S=[Label, OptTime, ExecTime, Costs, Atom, OTimes, QTimes, MInfo],
  ensure(count(maResult(_, _, _, _, _), 1)), % ensure that no more 
	% than one strategies was tested.
	testStrategies(No, TMode, Query, Rest, SRest).

sqltest(No, Name, TMode, Query, Atom, Costs, OptTime, AvgExecTime, OTimes,
		QTimes, MInfo) :- 
	TMode=tmode(opt(OCount), query(QCount), sleep(Sleep)),
	defaultExceptionHandler((
		% init warm state
		(onlyWriteTestFiles ->
			true
		;
		(QCount > 1 -> 
			(
				write_list(['Init warm state...\n']), 
				mOptimize(Query, AtomW, _),
				silentquery(Name, AtomW, _ETime),
				write_list(['done.\n'])
			)
		;
			true
		)),

		% Test run
		xcallstat(getTime(mOptimize(Query, Atom, Costs)), 0, OCount, 
			[OptTime, _OMin, _OMax], OTimes),
		(optimizerOption(memoryAllocation) ->
			(
				formulaList(F),
				maMemoryInfo(MInfo1),
				MInfo=[MInfo1, F]
			)
		;
			MInfo=[]
		),
		(onlyWriteTestFiles ->
			(
				writeTestFile(No, Name, Atom, QCount),
				AvgExecTime=0, 
				QTimes=[0]
			)
		;
			xcallstat(silentquery(Name, Atom), Sleep, QCount, 
				[AvgExecTime, _QMin, _QMax], QTimes)	
		)
	)).

% Allowes to test the queries with the SecondoBDB command.
% currently it is not possible to call rmlogs during during query 
% execution (and it would falsify the results).
onlyWriteTestFiles.

% If not interessent in any output.
writeTestFile(No, Name, Query, QCount) :-
	onlyWriteTestFiles,
	%atom_concat('query ', Query, QueryText), 
	atomic_list_concat(['query ', Query, ' feed head[0] consume'], QueryText),
	write_list(['Exec: ', QueryText, '...\n']),
	
	term_to_atom(Name, NameA),
	ensureDirExists(No),
	atomic_list_concat([No, '/', testrun_, NameA, '.sec'], F),
	QCountP1 is QCount+1,
	deleteTestFile(F),

	databaseName(DB),
	atomic_list_concat(['open database ', DB, '\n\n'], ODB),
	appendToTestFile(F, ODB),

	xcall((
			appendToTestFile(F, QueryText),
			appendToTestFile(F, '\n\n')
		), 0, QCountP1, _),

	% The result is not streamed, hence it can be thrown away without
	% falsifing the result.
	atomic_list_concat(['query SEC2COMMANDS feed sortby[CmdNr desc] head[',
		QCount, '] project[ElapsedTime]consume \n\n'], TQ),
	appendToTestFile(F, TQ),

	atomic_list_concat(['query SEC2COMMANDS feed sortby[CmdNr desc] head[',
		QCountP1, '] tail[', QCount,
		']project[ElapsedTime]avg[ElapsedTime]\n\n'], TQ2),
	appendToTestFile(F, TQ2),
	!.
	
silentquery(_Name, Query, ETimeMS) :-
	%atom_concat('query ', Query, QueryText), 
	atomic_list_concat(['query ', Query, ' feed head[0] consume'], QueryText),
	write_list(['Exec: ', QueryText, '...\n']),
	!,
	% The result is not streamed, hence it can be thrown away without
	% falsifing the result.
  (once(getTime(secondo(QueryText, _Result), PTime)) ->
		(
			write_list(['Exec: done.\n']),
			secondo('query SEC2COMMANDS feed sortby[CmdNr desc] head[1] project[ElapsedTime]consume', TResult),
			TResult=[[rel, [tuple, [['ElapsedTime', real]]]], [[ETime]]],
			ETimeMS is round(ETime * 1000),
			write_list(['ETime in ms: ', ETimeMS, ' Prolog eval time: ', PTime, '\n'])
		)
	;
		(
			write_list(['failed: ', Query, '\n']),
			!,
			fail
		)
	).

xcallstat(_Goal, _Sleep, 0, [-1, -1, -1], []) :-
	!.

xcallstat(Goal, Sleep, Count, [AvgExecTime, MinTime, MaxTime], Times) :-
	Count>0,
	!,
	xcallAddTime(Goal, Sleep, Count, AllTimes),
	write_list(['All times: ', AllTimes, '\n']), 
	/*
	(Count > 4 ->
		(Count > 9 ->
			(
				remove_extreme_value(min, AllTimes, TMP1),
				remove_extreme_value(min, TMP1, TMP2),
				remove_extreme_value(max, TMP2, TMP3),
				remove_extreme_value(max, TMP3, Times)
			)
		;
			(
				remove_extreme_value(min, AllTimes, TMP1),
				remove_extreme_value(max, TMP1, Times)
			)
		)
	;
		Times=AllTimes
	),
	*/
	Times=AllTimes,
	write_list(['NTimes: ', Times, '\n']),
	listSum(Times, TotalTime),
	min_list(Times, MinTime),
	max_list(Times, MaxTime),
	length(Times, Len),
	AvgExecTime is round(TotalTime / Len).

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

madebug :-
	debugLevel(ma),
	debugLevel(ma3).


/*
May fail with an exception for vanished memoryValue terms.
*/
maTestEnum :-
	maTestEnum(select * from [orte, plz as p] where ort=p:ort).

maTestEnum(Query) :-
	setStaticMemory(16),
	sql(Query),
	!,
	highNode(N), 
	enumeratePaths(Path, N, Costs), 
	(plan(Path, Plan)-> plan_to_atom(Plan, A);true), 
	Costs1 is floor(Costs),
	write_list(['\n', Costs1, ' -> ', A]), 
	fail.

poginfo :-
	writeNodes,
	writefacts(costEdge).

/*
Prints some information about some relation like row count and the tuplesize.
	sql(select count(*) from buildings),
	sql(select count(*) from natural),
	sql(select count(*) from places),
	sql(select count(*) from points),
	sql(select count(*) from railways),
	sql(select count(*) from roads),
*/
showRowCounts :-
	% non standard prolog, but now (new secondo checkout) there is more 
	% secondo output between the different calls. Hence, the output invoked
	% by whis is collected and at last written to stdout.
	with_output_to(atom(A), showRowCounts2),
	write(A).

showRowCounts2 :-
	databaseName(DB),
  FH='~w~17+  ~` t~w~15+  ~` t~w~15+  ~` t~w~15+  ~` t~w~15+  ~` t~w~15+ ~n',
  format(FH, ['Rel', 'Rows', 'Tuplesize', 'ExtTupleSize', 'RootTupleSize',
		'Data MiB']),
  findall(X, (
      storedRel(DB, R, _),
      \+ sub_atom(R, _, _, _,sample),
      storedSpell(DB, R, S),
			atomic(S),
      upper(S, X)
    ), Rels),
	findall(_, (
			%member(R, ['Buildings', 'Natural', 'Places', 'Points', 
			%	'Railways', 'Roads', 'Waterways']),
			member(R, Rels),
			showRowCounts(R)
		), _).

showRowCounts(Rel) :-
	showRowCounts(Rel, 'count', C),
	showRowCounts(Rel, 'tuplesize', T),
	showRowCounts(Rel, 'exttuplesize', ET),
	showRowCounts(Rel, 'roottuplesize', RT),
	DATA is round((C*ET)/(1024*1024)),
  F='~w~17+ &~` t~d~15+ &~` t~d~15+ &~` t~d~15+ &~` t~d~15+ &~` t~d~15+ \\\\~n',
  format(F, [Rel, C, T, ET, RT, DATA]),
	!.

showRowCounts(Rel, Op, R2) :-
	atomic_list_concat(['query ', Rel, ' ', Op], Atom),
	secondo(Atom, [_Type, R]),
	R2 is round(R),
	!.

/*

*/
countCostEdgePaths :-
  once(resetCounter(cecount)),
  highNode(N),
  enumeratePaths(Path, N, Cost),
  nextCounter(cecount, C),
  (0 is mod(C, 10000) -> % show progress
    (write(C), nl)
  ;
    true
  ),
  false.

countCostEdgePaths :-
  getCounter(cecount, C),	
	write_list(['Path count: ', C, '\n']).

/*
The result should be N!
*/
enumeratePOGPaths(Path, N, Costs) :-
  enumeratePOGPaths([], 0, N, Path, Costs).

enumeratePOGPaths(Path, N, N, Path, 0).

enumeratePOGPaths(PartPath, Node, N, RPath, RCosts) :-
  Node\=N,
  edge(Node, Target1, A, B, C, ECosts),
  E=edge(Node, Target1, A, B, C, ECosts),
  append(PartPath, [E], Path),
  enumeratePOGPaths(Path, Target1, N, RPath, Costs),
  RCosts is ECosts + Costs.

countPOGEdgePaths :-
  once(resetCounter(cecount)),
  highNode(N),
  enumeratePOGPaths(Path, N, Cost),
  nextCounter(cecount, C),
  (0 is mod(C, 10000) -> % show progress
    (write(C), nl)
  ;
    true
  ),
  false.

countPOGEdgePaths :-
  getCounter(cecount, C),
  write_list(['Path count: ', C, '\n']).


:- arithmetic_function(mod/2).
mod(A, B, M) :-
 M is A - (A div B) * B.

% eof
