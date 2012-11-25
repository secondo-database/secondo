/*
$Header$
@author Nikolai van Kempen

- Utility predicates of my evaluation...probably not  useful for anybody else.

*/

reloadMA :-
  ['MemoryAllocation/ma.pl'],
  ['MemoryAllocation/madata.pl'],
  ['MemoryAllocation/ma_improvedcosts.pl'],
  ['MemoryAllocation/test.pl'],
  ['MemoryAllocation/eval.pl'].


% The nrw3 database is created by the nrwtest/testnrw3.sh script.
% The difference is that the counter are kept to produce specific join
% results.
% case 1
evalQuery(2001, [database(nrw3)], select * from [points as p, railways as r] where[p:no=r:no]).

% case 2
evalQuery(2000, [database(nrw3)], select * from [buildings as b, roads as r] where[b:no=r:no]).

% case 3
evalQuery(2002, [database(nrw3)], select * from [buildings as b, roads as r, points as p] where[b:no=r:no,r:no=p:no]).

evalQuery(8000, [database(nrw3)], select * from [points as p1, points as p2, points as p3, points as p4, points as p5] where[p1:no=p2:no,p2:no=p3:no,p3:no=p4:no,p4:no=p5:no]).
evalQuery(8001, [database(nrw3)], select * from [points as r1, buildings200k as r2, roads200k as r3, roads200k as r4, roads300k as r5] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no]).
evalQuery(8002, [database(nrw3)], select * from [points as r1, buildings300k as r2, roads300k as r3, roads300k as r4, buildings200k as r5] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no]).
evalQuery(8003, [database(nrw3)], select * from [points as r1, buildings300k as r2, roads300k as r3, roads400k as r4, buildings400k as r5] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no]).

evalQuery(8013, [database(nrw3), maEval(staticEqual(16),static(16),static(256),staticMaxDivOpCount,staticMaxDivPOG,modifiedDijkstra)], select * from [roads500k as r1, roads400k as r2, buildings400k as r3, buildings500k as r4] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r1:no>0,r2:no>0,r3:no>0]).

evalQuery(8004, [database(nrw3)], select * from [roads300k as r1, roads400k as r2, roads500k as r3, roads600k as r4, roads700k as r5] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no]).

% 7085880 cost edge paths for 120 pog paths:
% standard costs path count: 29160
evalQuery(8005, [database(nrw3)], select * from [roads300k as r1, roads400k as r2, roads500k as r3, buildings300k as r4, buildings400k as r5, buildings500k as r6] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no, r5:no=r6:no]).

evalQuery(8006, [database(nrw3)], select * from [roads as r1, roads700k as r2, roads600k as r3, buildings as r4, buildings600k as r5, buildings500k as r6] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no, r5:no=r6:no]).


evalQuery(8010, [database(nrw3)], select * from [points as r1, roads500k as r2, buildings300k as r3, roads300k as r4, roads400k as r5, roads300k as r6] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no,r5:no=r6:no]).

evalQuery(8011, [database(nrw3)], select * from [points as r1, roads as r2, roads100k as r3, roads200k as r4, roads300k as r5, roads400k as r6] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no, r5:no=r6:no]).

evalQuery(2013, [database(nrw3)], select * from [buildings as b, roads as r, natural as n] where[b:no=r:no,r:no=p:no]).

% 162 cost edge paths for 2 pog paths:
evalQuery(2014, [database(nrw3)], select * from [buildings as b, roads as r, roads as r2] where[b:no=r:no,b:no=r2:no]).

% 4374 cost edge paths for 6 pog paths:
% 2003
evalQuery(2003, [database(nrw3)], select * from [buildings as b, roads as r, points as p, natural as n] where[b:no=r:no,r:no=p:no,p:no=n:no]).

evalQuery(2004, [database(nrw3)], select * from [buildings as b, roads as r, points as p, natural as n, buildings as b2] where[b:no=r:no,r:no=p:no,p:no=n:no, b2:no=n:no]).
% 157464  cost edge paths for 24 pog paths:
evalQuery(2005, [database(nrw3)], select * from [buildings as b, roads as r, points as p, natural as n, buildings as b2] where[b:no=b2:no,b:no=r:no,r:no=p:no,p:no=n:no]).

% 7085880 cost edge paths for 120 pog paths:
% without new translation rules: 29160
evalQuery(2006, [database(nrw3)], select * from [buildings as b, roads as r, points as p, roads as r2, natural as n, buildings as b2] where[b:no=b2:no,b:no=r:no,r:no=p:no,r2:no=p:no,p:no=n:no]).

createTestFiles :-
  testMAByID(8005, 1, 4, 2), % Case 7
	testMAByID(2001, 4, 4, 2), % Case 1
	testMAByID(2000, 4, 4, 2), % Case 2
	testMAByID(2002, 4, 4, 2), % Case 3
	testMAByID(2003, 4, 4, 2), % Case 4
  testMAByID(8003, 4, 4, 2), % Case 5
  testMAByID(8002, 4, 4, 2), % Case 6
  testMAByID(8004, 4, 4, 2),
  testMAByID(8006, 1, 1, 2),
  testMAByID(8013, 1, 1, 2),
	true.

/*
Creates the exact selectivities for the test queries.
*/
maCreateTestSels :-
	maCreateTestSels2.
maCreateTestSels :-
	!.

maCreateTestSels2 :-
	getAllNonSampleRels(Rels),
	databaseName(DB),
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
  evalQuery(No, Properties, Query),
  processProperties(Properties),
	runMATest(Properties, No, tmode(opt(CO), query(CQ), sleep(Sleep)), Query).

runMATest(Properties, No, TMode, Query) :-
	maCreateTestSels,
	!,
	(member(maEval(Strats), Properties) ->
		% Test only selected strategies...
		setMAStrategies(Strats)
	;
		% Test all strategies...
		(
			createExtendedEvalList(TList),
			setMAStrategies(TList)
		)
	),
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
	
	% Store results
 	get_time(TS), 
	convert_time(TS, Year, Month, Day, Hour, Minute, Sec, _MilliSec),
	atomic_list_concat([Year, '-', Month, '-', Day, ' ', Hour, ':', Minute, 
		':', Sec], DateTime),
	atomic_list_concat(['MemoryAllocation/test/matest',No,'.log'], FileName),
  open(FileName, write, FD),
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

reprocessResults3(ID) :-
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
      X=[T, OT, ET, C, A, _OTimes, _QTimes, MInfo|_],
      %F='~w~30+ &~` t~d~9+ &~` t~d~10+ &~` t~d~12+ &~w \\\\~n',
      ICosts is round(C),
      (MInfo = [] ->
        MI=''
      ;
        MInfo=[MI|_]
      ),
      %DATA=[T, ICosts, OT, ET, MI],
      %format(F, DATA),
    	F='~w~30+ &~` t~w~9+ &~` t~w~10+ &~` t~d~12+ &~w &~w \\\\~n',
			atomic_list_concat(['\\num{', ICosts, '}'], X2),
			atomic_list_concat(['\\num{', OT, '}'], X3),
    	%format(F, [T, ICosts, OT, ET, A, MInfo])
    	format(F, [T, X2, X3, ET, A, MInfo])
    ), _),
  nl,
  findall(X, (
      member(X, LNew),
      X=[T, _OT, _ET, _C, A|_],
      write_list(['# ', T, ':\n']),
      write_list([A, '\n'])
    ), _),
  nl.

latexListing(A, AL) :-
	atom_chars(A, CharList),
	latexTranslate(CharList, CharListL),
	atom_chars(AL, CharListL).
	
latexTranslate([], []) :-
	!.

latexTranslate([A|Rest], Out) :-
	latexTranslateA(A, ACharList),
	latexTranslate(Rest, RestCharList),
	append(ACharList, RestCharList, Out).

latexTranslateA('{', ['\\', '{']) :-
	!.
latexTranslateA('}', ['\\', '}']) :-
	!.
latexTranslateA(A, [A]) :-
	!.
	
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
Prints some information about some relation like the row count, tuplesize etc.
*/
showRowCounts :-
	% non standard prolog, but now (new secondo checkout) there is more 
	% secondo output between the different calls. Hence, the output invoked
	% by whis is collected and at last written to stdout.
	with_output_to(atom(A), showRowCounts2),
	write(A).

showRowCounts2 :-
  FH='~w~17+  ~` t~w~15+  ~` t~w~15+  ~` t~w~15+  ~` t~w~15+  ~` t~w~15+ ~n',
  format(FH, ['Rel', 'Rows', 'Tuplesize', 'ExtTupleSize', 'RootTupleSize',
		'Data MiB']),
	getAllNonSampleRels(Rels),
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
	DATAMiB is round((C*ET)/(1024**2)),
  F='~w~17+ &~` t~d~15+ &~` t~d~15+ &~` t~d~15+ &~` t~d~15+ &~` t~d~15+ \\\\~n',
  format(F, [Rel, C, T, ET, RT, DATAMiB]),
	!.

showRowCounts(Rel, Op, R2) :-
	atomic_list_concat(['query ', Rel, ' ', Op], Atom),
	secondo(Atom, [_Type, R]),
	R2 is round(R),
	!.

getAllNonSampleRels(Rels) :-
	databaseName(DB),
  findall(X, (
      storedRel(DB, R, _),
      \+ sub_atom(R, _, _, _,sample),
      storedSpell(DB, R, S),
			atomic(S), % don't care here about the lc(_) case.
      upper(S, X)
    ), Rels).

/*

*/
countCostEdgePaths :-
  once(resetCounter(cecount)),
  highNode(N),
  enumeratePaths(_Path, N, _Cost),
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
The result should be the factorial of N (= !N)
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
  enumeratePOGPaths(_Path, N, _Cost),
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

% There is a mod function within swi-prolog 5.x.y but somehow it can't be
% used within is/2 terms.
:- arithmetic_function(mod/2).
mod(A, B, M) :-
 M is A - (A div B) * B.

/*
Some facts and predicates to process my test data.
*/
maTestResults([
[2000,
['testrun_staticEqual(16).sec', 1108.06306775],
['testrun_static(16).sec', 1094.781208],
['testrun_static(256).sec', 1088.821897],
['testrun_staticMaxDivOpCount.sec', 1080.308661],
['testrun_staticMaxDivPOG.sec', 1084.16777075],
['testrun_modifiedDijkstra.sec', 1084.79048475],
['testrun_enumeratePaths.sec', 1068.23215975],
['testrun_impcosts.sec', 993.6681055],
['testrun_stdcosts.sec', 1007.610972]],

[2001,
['testrun_staticEqual(16).sec', 13.04901475],
['testrun_static(16).sec', 12.68681375],
['testrun_static(256).sec', 12.984243125],
['testrun_staticMaxDivOpCount.sec', 13.2904455],
['testrun_staticMaxDivPOG.sec', 12.820770875],
['testrun_modifiedDijkstra.sec', 12.853974125],
['testrun_enumeratePaths.sec', 13.2094465],
['testrun_impcosts.sec', 13.377928375],
['testrun_stdcosts.sec', 13.134874875]],

[2002,
['testrun_staticEqual(16).sec', 343.1777],
['testrun_static(16).sec', 326.04821625],
['testrun_static(256).sec', 284.706570875],
['testrun_staticMaxDivOpCount.sec', 287.180825],
['testrun_staticMaxDivPOG.sec', 286.37312725],
['testrun_modifiedDijkstra.sec', 288.9751645],
['testrun_enumeratePaths.sec', 289.381192625],
['testrun_impcosts.sec', 317.90451375],
['testrun_stdcosts.sec', 373.27153]],

[2003,
['testrun_staticEqual(16).sec', 307.29982825],
['testrun_static(16).sec', 323.7392885],
['testrun_static(256).sec', 219.162557],
['testrun_staticMaxDivOpCount.sec', 210.65918125],
['testrun_staticMaxDivPOG.sec', 221.08666225],
['testrun_modifiedDijkstra.sec', 216.165868],
['testrun_enumeratePaths.sec', 213.490941125],
['testrun_impcosts.sec', 234.717237875],
['testrun_stdcosts.sec', 213.272788125]],

[8001,
['testrun_staticEqual(16).sec', 354.8144545],
['testrun_static(16).sec', 337.43853675],
['testrun_static(256).sec', 321.000091],
['testrun_staticMaxDivOpCount.sec', 306.511835],
['testrun_staticMaxDivPOG.sec', 289.7197525],
['testrun_modifiedDijkstra.sec', 285.903809],
['testrun_enumeratePaths.sec', 289.52861475],
['testrun_impcosts.sec', 331.21191075],
['testrun_stdcosts.sec', 375.47679]],

[8002,
['testrun_staticEqual(16).sec', 361.54869125],
['testrun_static(16).sec', 376.486375],
['testrun_static(256).sec', 359.250565],
['testrun_staticMaxDivOpCount.sec', 343.6269805],
['testrun_staticMaxDivPOG.sec', 348.521256],
['testrun_modifiedDijkstra.sec', 344.39644675],
['testrun_enumeratePaths.sec', 355.46775575],
['testrun_impcosts.sec', 347.81456025],
['testrun_stdcosts.sec', 120.74938925]],

[8005,
['testrun_staticEqual(16).sec', 1001.965809],
['testrun_static(16).sec', 996.258335],
['testrun_static(256).sec', 942.573502],
['testrun_staticMaxDivOpCount.sec', 982.240451],
['testrun_staticMaxDivPOG.sec', 987.906534],
['testrun_modifiedDijkstra.sec', 963.548287],
['testrun_enumeratePaths.sec', 942.843598],
['testrun_impcosts.sec', 983.514721],
['testrun_stdcosts.sec', 967.498633]]]).

maPTestResults :-
	maTestResults(T),
	maPTestResults2(T, V),
	write(V),
	V=[[_,R]|_],
	inpercent(R, V).
	
inpercent(_R, []) :-
	!.
inpercent(R, [[_,V]|X]) :-
	T is ((R-V)/R)*100,
	write_list(['\nT: ',T]),
	inpercent(R, X).

maPTestResults2([[_NO|Vals]], Vals) :-
	!.

maPTestResults2([[_NO|Vals]|R], ValsO) :-
	maPTestResults2(R, ValsR),
	addVals(Vals, ValsR, ValsO).
	
addVals([], [], []).
addVals([[L,V1]|VR1], [[_,V2]|VR2], [[L,R]|RR]) :-
	R is V1+V2,
  addVals(VR1, VR2, RR).
	

% eof
