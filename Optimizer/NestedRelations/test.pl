/*
$Header$
@author Nikolai van Kempen
*/

reset :-  
	reload, 
	closeDB, 
	updateDB(optext), 
	odb.

odb :-  
	open('database optext').

reload :-
	['NestedRelations/test'], 
	['NestedRelations/nr'], 
	['NestedRelations/init'], 
	['NestedRelations/util'], 
	['NestedRelations/tutil'], 
	['NestedRelations/nr_subqueries'], 
	['MemoryAllocation/ma.pl'], 
	['MemoryAllocation/ma_improvedcosts.pl'], 
	%['Subqueries/subqueries'], % reloaded already then by optimizer
	[database],
	[operators],
	[statistics],
	[optimizer].

% Simply switch the enviroment
nr :-
	setOption(nestedRelations),
	delOption(memoryAllocation).
		
ma :-
	delOption(nestedRelations),
	setOption(memoryAllocation).

nomanr :-
	delOption(nestedRelations),
	delOption(memoryAllocation).

manr :-
	setOption(nestedRelations),
	setOption(memoryAllocation).

/*
 sql select *
        from [staedte, plz as p1, plz as p2, plz as p3]
        where [sname = p1:ort,
          p1:plz = p2:plz + 1
         , p2:plz = p3:plz * 5
          , bev > 300000
          ,bev < 500000
          , p2:plz > 50000
          , p2:plz < 60000
          ,kennzeichen starts "W",
           p3:ort contains "burg"
         , p3:ort starts "M"].

Note: the test run only test if the goal is provable, not if the result is correct.
*/

% Just some simple queries that don't work with nested relations, just to test if i haven't damaged something because subquery-error-tracking is much more complex as for non-subquery queries.
testNRQuery(100, [database(opt)], select * from orte).
testNRQuery(101, [], select bevt from orte).
testNRQuery(102, [], select [bevt, kennzeichen] from orte).
testNRQuery(103, [], select * from orte as oXy).
testNRQuery(104, [], select oXy:bevt from orte as oXy).
testNRQuery(105, [], select [oXy:bevt, oXy:kennzeichen] from orte as oXy).
testNRQuery(106, [], select [oXy:bevt, oXy:kennzeichen] from orte as oXy where [oXy:bevt>10, oXy:bevt<1000]).
testNRQuery(107, [], select * from [orte as o, plz as p] where o:ort=p:ort).
testNRQuery(108, [],  select [ort, min(plz) as minplz, max(plz) as maxplz,  count(*) as cntplz] from plz where plz > 40000 groupby ort).
testNRQuery(109, [], select [ort, plz] from plz orderby [ort asc, plz desc]).
testNRQuery(110, [], select [ort, plz] from plz where ort="Berlin" orderby [ort asc, plz desc]).
% Error reported: var1 does not fit Secondo's names conventions
% But seems to be a problem not related to my work.
testNRQuery(111, [expectedResult(fail)], select aggregate((distinct b:no*1), (*), 'int', [const,int,value,0] ) as fac from [ten as a, ten as b] where [a:no < b:no] groupby a:no).
testNRQuery(112, [], select [ort, min(plz) as minplz, max(plz) as maxplz, count(distinct *) as cntplz] from plz where plz > 40000 groupby ort orderby cntplz desc first 2).
testNRQuery(113, [], select [min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz, count(distinct ort) as ortcnt] from plz groupby []).  
testNRQuery(114, [], select sum(no) from ten).
testNRQuery(115, [], select avg(distinct no) from ten where no > 5).



% Nested relation test queries

% The most simple queries with renaming test on a nrel-relation.
testNRQuery(0, [database(optext)], select * from orteh).
testNRQuery(1, [], select * from orteh as o).
testNRQuery(2, [], select * from orteh as oXXXXX).
testNRQuery(3, [], select * from orteh as oXyZ).
testNRQuery(4, [], select bevth from orteh).
testNRQuery(5, [], select o:bevth from orteh as o).
testNRQuery(6, [], select oXXXXX:bevth from orteh as oXXXXX).
testNRQuery(7, [], select oXyZ:bevth from orteh as oXyZ).

% This is currently not possible because the POG can't handle this.
testNRQuery(9, [expectedResult(fail)], select * from orteh as o where [exists(select * from o:subrel as p)]).

% With subqueries
testNRQuery(10, [], select * from orteh as o where [o:bevth>10, exists(select * from o:subrel as p where p:bevt>o:bevth)]).
testNRQuery(11, [], select * from orteh as oAB where [oAB:bevth>10, exists(select * from oAB:subrel as pZxY where pZxY:bevt>oAB:bevth)]).
testNRQuery(12, [], select * from orteh as o where [exists(select * from o:subrel where bevt>o:bevth)]).
testNRQuery(13, [], select * from orteh as o where [o:bevth>10, exists(select * from o:subrel where bevt>o:bevth)]).
testNRQuery(14, [], select * from orteh as o where [o:bevth>10, exists(select bevt from o:subrel where bevt>o:bevth)]).

% This case isn't that easy anymore, the thing is that subrel isn't know within the subquery. 
% Ein nachträgliches nochmalige project scheint schon zu erfolgen, das erste scheint dann unnötig zu sein,
% fraglich ist erst mal noch wodurch das erste rojekct erzeugt wird..
testNRQuery(15, [expectedResult(fail)], select o:bevth from orteh as o where [o:bevth>10, exists(select bevt from o:subrel where bevt>o:bevth)]).

testNRQuery(16, [], select [o:bevth,o:subrel] from orteh as o where [o:bevth>10, exists(select bevt from o:subrel where bevt>o:bevth)]).

testNRQuery(17, [], select * from orteh as o where [o:bevth>10, exists(select p:bevt from o:subrel as p where p:bevt>o:bevth)]).
testNRQuery(18, [], select * from orteh as o where [o:bevth>10, exists(select [p:bevt, p:kennzeichen] from o:subrel as p where p:bevt>o:bevth)]).
testNRQuery(19, [], select * from orteh as o where [o:bevth>10, not exists(select [p:bevt, p:kennzeichen] from o:subrel as p where p:bevt>o:bevth)]).
testNRQuery(20, [], select * from orteh as o where [o:bevth in (select p:bevt from o:subrel as p where p:bevt>o:bevth)]).
testNRQuery(21, [], select * from orteh as o where [o:bevth not in(select p:bevt from o:subrel as p where p:bevt>o:bevth)]).

% selections
testNRQuery(31, [], select * from orteh as o where [exists(select * from o:subrel as p where [p:bevt>o:bevth, p:bevt>100000])]).
testNRQuery(32, [], select * from orteh as o where [o:bevth < 99, exists(select * from o:subrel as p where [p:bevt>o:bevth, p:bevt<1000000])]).
testNRQuery(33, [], select * from orteh as o where [o:bevth < 99, exists(select * from o:subrel as p where [p:bevt>o:bevth, p:bevt<1000000])]).
testNRQuery(34, [], select * from orteh as o where [o:bevth < 99, exists(select * from o:subrel as p where [p:bevt>o:bevth, p:kennzeichen="H", p:bevt<1000000])]).
testNRQuery(35, [], select [o:bevth, o:subrel] from orteh as o where [o:bevth < 99, exists(select * from o:subrel as p where [p:bevt>o:bevth, p:kennzeichen="H", p:bevt<1000000])]).
testNRQuery(36, [], select * from orteh as o where [o:bevth<199, o:bevth in (select p:bevt from o:subrel as p where p:bevt>o:bevth)]).
testNRQuery(37, [], select * from orteh as o where [o:bevth<199, o:bevth in (select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1])]).
testNRQuery(38, [], select * from orteh as o where [o:bevth<199, o:bevth >( all (select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).
testNRQuery(39, [], select * from orteh as o where [o:bevth<199, o:bevth =(any(select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).
testNRQuery(40, [], select * from orteh as o where [o:bevth<199, 10 >( all (select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).
testNRQuery(41, [], select * from orteh as o where [o:bevth<199, 10 <=(all(select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).
% "= all" queries won't work, i think there a little error within the
% subqueries extension, but it's really difficult to track this error.
testNRQuery(42, [expectedResult(fail)], select * from orteh as o where [o:bevth<199, 10 =(all(select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).
testNRQuery(43, [expectedResult(fail)], select * from orteh as o where [o:bevth<199, o:bevth =(all(select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).

% Expected not to work, no preTransformNestedPredicate predicates are 
% defined for this.
%testNRQuery(39, select * from orteh as o where [o:bevth<199, o:bevth >(some(select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).
testNRQuery(44, [], select * from orteh as o where [o:bevth<199, o:bevth >(any(select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).


% Multiple subqueries
testNRQuery(51, [], select * from orteh as o 
	where [o:bevth>10, 
		exists(select p:bevt from o:subrel as p where p:bevt>o:bevth),
		exists(select p2:kennzeichen from o:subrel as p2 where [p2:bevt>o:bevth]),
		exists(select pABC3:vorwahl from o:subrel as pABC3 where pABC3:bevt>o:bevth)
	]).

testNRQuery(52, [], select * from orteh as o 
	where [o:bevth>10, 
		exists(select bevt from o:subrel where bevt>o:bevth),
		exists(select kennzeichen from o:subrel where [bevt>o:bevth, bevt=3410]),
		exists(select vorwahl from o:subrel where bevt>o:bevth)
	]).

% sub-sub-query
testNRQuery(70, [], select * from orte as o where [
	exists(select * from ten as t where [t:no<5,t:no=o:bevt,
		exists(select * from thousand as t2 where t2:no=t:no)])]).

testNRQuery(71, [], select * from orte as o where [
		exists(select * from ten as t where [t:no<5,t:no=o:bevt,
			exists(select * from thousand as t2 where t2:no=t:no)]),
		exists(select * from ten as t1 where [t1:no<5,t1:no=o:bevt,
			exists(select * from thousand as t2 where t2:no=t1:no)]),
		exists(select * from ten as t2 where [t2:no<5,t2:no=o:bevt,
			exists(select * from thousand as t3 where t3:no=t2:no)])
	]).

% joins

testNRQuery(80, [], select * from orteh as o 
	where [exists(select * from [o:subrel as p, plz as p2] where [p:bevt<o:bevth, p:ort=p2:ort])]).
testNRQuery(81, [], select * from orteh as o 
	where [exists(select * from [plz as p2, o:subrel as p] where [p:bevt<o:bevth, p2:ort=p:ort])]).
testNRQuery(82, [], select * from orteh as o where [exists(select * from [plz as p2, o:subrel as p] where [p:bevt<o:bevth+5, p2:ort=p:ort])]).
testNRQuery(83, [expectedResult(fail)], select * from orteh as o where [exists(select [p2:ort, o:bevth+5 as plus5, p:bevt, p:ort] from [plz as p2, o:subrel as p] where [p:bevt<plus5, p2:ort=p:ort])]).
% .. won't work because even this won't work:
testNRQuery(84, [expectedResult(fail)], select bevt+5 as plus5 from orte where plus5=10).

% Attribute-level
% no new name
testNRQuery(300, [expectedResult(fail)], select [bevth, (select * from subrel)]from orteh).
% no renmae for new attribute
testNRQuery(301, [expectedResult(fail)], select [o:bevth, (select * from o:subrel)]from orteh as o).
testNRQuery(302, [], select [o:bevth, (select * from o:subrel) as sub]from orteh as o).
% arel access without variable not (yet) possible.
testNRQuery(310, [expectedResult(fail)], select [bevth, (select * from subrel) as sr1]from orteh).

testNRQuery(320, [], select [o:bevth, (select s:kennzeichen from o:subrel as s) as sub]from orteh as o).
% with where condition
testNRQuery(321, [], select [o:bevth, (select s:kennzeichen from o:subrel as s where [s:kennzeichen = "B"]) as sub]from orteh as o).
testNRQuery(322, [], select [o:bevth, o:subrel, (select s:kennzeichen from o:subrel as s where [s:kennzeichen = "B"]) as sub]from orteh as o where [exists(select * from o:subrel as s where[s:kennzeichen = "B", s:bevt=o:bevth])]).
% sql select [*] from orte.
% sql select [*, bevt] from orte will fail, too.
testNRQuery(323, [expectedResult(fail)], select [*, (select s:kennzeichen from o:subrel as s where [s:kennzeichen = "B"]) as sub]from orteh as o where [exists(select * from o:subrel as s where[s:kennzeichen = "B", s:bevt=o:bevth])]).

testNRQuery(324, [], select [o:bevth, o:subrel, (select s:kennzeichen from o:subrel as s where [s:kennzeichen = "B"]) as sub]from orteh as o where [exists(select * from o:subrel as s where[s:kennzeichen = "B", s:bevt=o:bevth])]).

testNRQuery(325, [], select [o:bevth, o:subrel, (select sX:kennzeichen from o:subrel as sX where [sX:kennzeichen = "B"]) as sub]from orteh as o where [exists(select sY:kennzeichen from o:subrel as sY where[sY:kennzeichen = "B", sY:bevt=o:bevth])]).
testNRQuery(326, [], select [o:bevth, o:subrel, (select s:kennzeichen from o:subrel as s where [s:kennzeichen = "B"]) as sub]from orteh as o where [exists(select * from o:subrel as s where[s:kennzeichen = "B"])]).
testNRQuery(327, [], select [o:bevth, o:subrel, (select count(*) from o:subrel as sX where [sX:kennzeichen = "B"]) as sub]from orteh as o where [exists(select sY:kennzeichen from o:subrel as sY where[sY:kennzeichen = "B", sY:bevt=o:bevth])]).
testNRQuery(328, [], select [o:bevth, o:subrel, (select count(distinct *) from o:subrel as sX where [sX:kennzeichen = "B"]) as sub]from orteh as o where [exists(select sY:kennzeichen from o:subrel as sY where[sY:kennzeichen = "B", sY:bevt=o:bevth])]).
testNRQuery(329, [], select [o:bevth, o:subrel, (select sX:kennzeichen from o:subrel as sX where [sX:kennzeichen = "B"] groupby sX:kennzeichen) as sub]from orteh as o where [exists(select sY:kennzeichen from o:subrel as sY where[sY:kennzeichen = "B", sY:bevt=o:bevt])]).

testNRQuery(340, [], select [o:bevth, (select * from [o:subrel as s, plz as p] where [s:ort=p:ort]) as sub] from orteh as o).
testNRQuery(341, [], select [o:bevth, (select * from [o:subrel as s, plz as p] where [s:ort=p:ort]) as sub] from orteh as o where [exists(select s2:kennzeichen from o:sub as s2 where[s2:kennzeichen = "B", s2:bevt=o:bevth])]).
% Using "s" twice as a variable.
testNRQuery(342, [], select [o:bevth, (select * from [o:subrel as s, plz as p] where [s:ort=p:ort]) as sub] from orteh as o where [exists(select s:kennzeichen from o:subrel as s where[s:kennzeichen = "B", s:bevt=o:bevth])]).
testNRQuery(343, [], select [(select [(select * from ten as t3 first 1) as bl] from ten as t2 first 1) as x] from ten as t first 1).

/*
% unnest
% No renaming for unnest operators allowed (pointless)
testNRQuery(600, [expectedResult(fail)], select [o:bevth, unnest(o:subrel) as usubrel ]from orteh as o).

testNRQuery(601, [], select [o:bevth, unnest(o:subrel)]from orteh as o).
testNRQuery(602, [], select [o:bevth, unnest(o:subrel)]from orteh as o where [o:kennzeichen="B"]).
testNRQuery(602, [], select [bevth, unnest(subrel)]from orteh where [kennzeichen="B"]).
testNRQuery(603, [], select [o:bevth, unnest(o:subrel)]from orteh as o where [o:kennzeichen="B"] orderby[o:bevt desc]).
testNRQuery(604, [], select [o:bevth, unnest(o:subrel)]from orteh as o orderby[o:bevt desc]).
% Not possible, don't know if this makes much sense
% After applying the unnest operation, the arel attribut is not available 
% anymore. But it is still possible, the unnest should be performed on a query.
testNRQuery(605, [expectedResult(fail)], select [o:bevth, o:subrel, unnest(o:subrel)]from orteh as o).
% like here:
testNRQuery(606, [], select [o:bevth, o:subrel, unnest(select * from o:subrel)]from orteh as o).
testNRQuery(607, [], select [o:bevth, unnest((select * from o:subrel as s where s:kennzeichen="M"))]from orteh as o).
testNRQuery(608, [], select [o:bevth, unnest((select s:kennzeichen from o:subrel as s))]from orteh as o).
testNRQuery(609, [], select [o:bevth, unnest((select s:kennzeichen from o:subrel as s where s:kennzeichen="M"))]from orteh as o).
testNRQuery(610, [], select [unnest((select * from o:subrel as s))]from orteh as o).
% This is an example that a unnest operation can't be done after feeding
% the relation into a stream.
testNRQuery(620, [], select [unnest((select * from o:subrel as s where[s:kennzeichen=p:ort]))] from [orteh as o, plz as p]where[p:plz=o:bevth]).

testNRQuery(621, [], select [unnest(select * from ten as s)]from ten as t).
testNRQuery(622, [], select [unnest(select s:no from ten as s)]from ten as t).
*/

% queries as relations
% The user needs to specify a attribute name!
/*
testNRQuery(700, [expectedResult(fail)], select [o:bevt, nest(o:bevt)] from orte as o).
testNRQuery(701, [], select [o:bevt, nest(o:bevt) as subrel] from orte as o).
*/

% subqueries within the from clause
testNRQuery(700, [], select * from (select * from orteh) as o).
testNRQuery(701, [], select * from (select bevth from orteh) as o).
testNRQuery(702, [], select * from (select [bevth] from orteh) as o).
testNRQuery(703, [], select * from (select [bevth, subrel] from orteh) as o).
testNRQuery(704, [], select * from (select [xy:bevth, xy:subrel] from orteh as xy) as o).
testNRQuery(705, [], select [o:bevth] from (select [xy:bevth, xy:subrel] from orteh as xy) as o).
testNRQuery(706, [], select [o:bevth] from (select [xy:bevth, xy:subrel] from orteh as xy) unnest(xy:subrel) as o).
testNRQuery(707, [], select [o:kennzeichen] from (select [xy:subrel] from orteh as xy) unnest(xy:subrel) as o).
testNRQuery(730, [], select * from (select * from orte as x) nest(x:bevt, subrel) as o first 1).
testNRQuery(731, [], select [o:bevt] from (select * from orte as x) nest(x:bevt, subrel) as o).
testNRQuery(732, [], select [o:bevt, o:subrel] from (select * from orte as x) nest(x:bevt, subrel) as o).
testNRQuery(733, [], select [o:bevt, o:subrel] from (select [x:bevt, x:kennzeichen] from orte as x) nest(x:bevt, subrel) as o).
testNRQuery(734, [], select [o:bevt, o:subREL] from (select [x:bevt, x:kennzeichen] from orte as x) nest(x:bevt, subREL) as o).
testNRQuery(735, [], select [o:bevt, o:subREL] from (select [x:bevt, x:kennzeichen, x:vorwahl] from orte as x) nest(x:bevt, subREL) as o).

testNRQuery(750, [], select * from (select * from (select * from orte) as o1) as o2 first 1).
testNRQuery(751, [], select * from (select * from (select * from (select * from orte) as o0) as o1) as o2 first 1).
testNRQuery(752, [], select * from (select * from (select * from (select * from orteh) as o0) as o1) as o2 first 1).
testNRQuery(753, [], select * from (select * from (select * from (select * from orte as oh where [oh:kennzeichen="B"]) as o0) as o1) as o2).
testNRQuery(754, [], select * from (select * from (select * from (select * from (select * from orte) as oh where [oh:kennzeichen="B"]) as o0) as o1) as o2).
testNRQuery(755, [], select * from (select * from (select * from (select * from (select kennzeichen from orte) as oh where [oh:kennzeichen="B"]) as o0) as o1) as o2).
testNRQuery(756, [], select * from (select * from (select * from (select * from (select [bevt, vorwahl] from orte orderby vorwahl desc) as oh where [oh:bevt=10]) as o0) as o1) as o2).
testNRQuery(757, [], select * from (select o1:vorwahl from (select * from (select * from (select [bevt, vorwahl] from orte orderby vorwahl desc) as oh where [oh:bevt=10]) as o0) as o1) as o2).
testNRQuery(758, [], select o1:vorwahl from (select [bevt, vorwahl] from orte orderby vorwahl desc) as o1).
testNRQuery(759, [], select o1:vorwahl from (select [o0:bevt, o0:vorwahl] from orte as o0 orderby o0:vorwahl desc) as o1).

testNRQuery(770, [], select * from (select * from orte as o) nest(o:bevt, subRel) as o1).
testNRQuery(771, [], select * from (select [o:bevt, o:kennzeichen] from orte as o) nest(o:bevt, subRel) as o1).
testNRQuery(772, [], select o1:subrel from (select [o:bevt, o:kennzeichen] from orte as o) nest(o:bevt, subrel) as o1).
testNRQuery(773, [], select * from (select o1:subrel from (select [o:bevt, o:kennzeichen] from orte as o) nest(o:bevt, subrel) as o1) as o2).

/*
Works, but the result is not correct. This is not possible in this way because a count(*) does not return a tuple stream. So there is not natural straight forward implementation possible, more is to done manullay to reconvert the result into a tuple stream.
Idea: query intstream(Orte feed count, Orte feed count) namedtransformstream[Count] consume
=>
 query 1 feed transformstream projectextend[; Count: (Orte feed count)] consume

See the comments of the nrSubqueryToStream/3 predicate for more information.
Update: Now this works for the count operator only.
*/
testNRQuery(780, [expectedResult(fail)], select count(*) as summe from orte).
testNRQuery(781, [], select * from (select count(*) from orte as o0) as o1).
testNRQuery(782, [], select * from (select count(distinct kennzeichen) from orte) as o1).
/*
testNRQuery(783, [], select * from (select sum(o0:bevt) from orte as o0) as o1).
testNRQuery(784, [], select * from (select max(bevt) from orte) as o1).
testNRQuery(785, [], select * from (select min(bevt) from orte) as o1).
testNRQuery(786, [], select * from (select avg(bevt) from orte) as o1).
*/

testNRQuery(787, [], select [kennzeichen, sum(bevt) as x] from orte groupby kennzeichen).
testNRQuery(788, [], select * from (select [kennzeichen, sum(bevt) as x] from orte groupby kennzeichen) as oOOOo).
testNRQuery(789, [], select * from (select [kennzeichen, sum(bevt) as sumEinwohner] from orte groupby kennzeichen) as y where y:kennzeichen="B").

% subqueries within the from clause
% first only with the deep 1.
testNRQuery(800, [], select * from (select * from orte) as o).
% Subquery needs to be given a label.
testNRQuery(801, [expectedResult(fail)], select * from (select * from orte)).
testNRQuery(802, [], select * from (select * from orte) as o first 1).
testNRQuery(803, [], select * from (select * from orte) as o orderby o:bevt first 1).
testNRQuery(804, [], select o:bevt from (select * from orte) as o).
testNRQuery(805, [], select o:bevt from (select bevt from orte) as o).
testNRQuery(806, [], select o:bevt from (select x:bevt from orte as x) as o first 1).



% unnesting
testNRQuery(810, [], select o:bevt from (select subrel from orteh) unnest(subrel) as o first 1).
testNRQuery(811, [], select [o:bevth, o:bevt] from (select * from orteh) unnest(subrel) as o first 1).
testNRQuery(812, [], select [o:bevth, o:bevt, o:kennzeichen] from (select * from orteh) unnest(subrel) as o orderby [o:kennzeichen] first 1).
testNRQuery(813, [], select * from (select subrel from orteh) unnest(subrel) as o orderby [o:kennzeichen] first 1).
testNRQuery(814, [], select * from (select x:subrel from orteh as x) unnest(x:subrel) as o orderby [o:kennzeichen] first 1).
testNRQuery(815, [], select [o:bevt, o:bevth, o:kennzeichen] from (select [x:bevth,x:subrel] from orteh as x) unnest(x:subrel) as o orderby [o:kennzeichen] first 1).
testNRQuery(816, [], select [o:bevt, o:bevth, o:kennzeichen] from (select * from orteh as x) unnest(x:subrel) as o orderby [o:kennzeichen] first 1).

% nesting
testNRQuery(820, [], select * from (select * from orte as x) nest(x:bevt, subrel) as o first 1).
testNRQuery(821, [], select * from (select * from orte as x) nest(x:bevt) as subrel as o first 1).

% o:kennzeichen is moved into the arel attribute.
% so the query is written wrong.
testNRQuery(822, [expectedResult(fail)], select * from (select * from orte as x) nest(x:bevt, subrel) as o orderby [o:kennzeichen] first 1).
% sorting is only allowed over attributes. nest sorts always the query over its nesting attributes.
testNRQuery(823, [expectedResult(fail)], select * from (select * from orte as x orderby x:kennzeichen) nest(x:bevt, subrel) as o where exists(select * from o:subrel as s where [s:kennzeichen="B", s:vorwahl=o:bevt]) first 1).

testNRQuery(824, [], select * from (select [x:bevt div 100 as bevth, x:kennzeichen, x:ort, x:vorwahl, x:bevt] from orte as x) nest(x:bevt) as subrel as o first 1).
testNRQuery(825, [], select * from (select [x:bevt div 100 as bevth, x:kennzeichen, x:ort, x:vorwahl, x:bevt] from orte as x) nest(bevth) as subrel as o first 1).

testNRQuery(830, [], select [o:bevt, o:kennzeichen] from (select x:subrel from orteh as x) unnest(x:subrel) as o where o:kennzeichen="B" orderby [o:kennzeichen] first 1).
testNRQuery(831, [], select [o:bevt, o:bevth, o:kennzeichen] from (select [x:bevth,x:subrel] from orteh as x) unnest(x:subrel) as o where o:kennzeichen="B" orderby [o:kennzeichen] first 1).
testNRQuery(832, [], select * from (select [x:bevth,x:subrel] from orteh as x) unnest(x:subrel) as o where o:kennzeichen="B" orderby [o:kennzeichen] first 1).
testNRQuery(833, [], select * from (select * from orteh as x) unnest(x:subrel) as o where o:kennzeichen="B" orderby [o:kennzeichen] first 1).
testNRQuery(834, [], select * from [(select * from orteh as x) as o] where [o:bevth=10] first 1).

% joins
testNRQuery(840, [], select * from [(select * from orteh as x) as o1, (select * from orteh as x) as o2] where [o1:bevth=10,o1:bevth=o2:bevth] first 1).
testNRQuery(841, [], select * from [(select * from orteh as x) as o1, (select * from orteh as x) as o2] where [o1:bevth=10,o1:bevth=o2:bevth-1] first 1).
testNRQuery(842, [], select * from [(select * from orteh as x) unnest(x:subrel) as o1, (select * from orteh as x) as o2] where [o1:kennzeichen="B",o1:bevth=o2:bevth] orderby [o1:vorwahl] first 1).
testNRQuery(843, [expectedResult(fail)], select * from [(select * from orteh as x) unnest(x:subrel) as o1, (select [x:bevt div 100 as bevth, x:kennzeichen, x:ort, x:vorwahl, x:bevt] from orte as x) nest(bevth) as subrel as o2] where [o1:kennzeichen="B",o1:bevth=o2:bevth] orderby [o1:vorwahl] first 1).
% ... not possible due to this:
testNRQuery(844, [expectedResult(fail)], select[bevt div 100 as bevth]from orte where [bevth=10]).
% so we are not able to nest 
testNRQuery(845, [], select * from [(select * from orteh as x) unnest(x:subrel) as o1, (select [x:kennzeichen, x:ort, x:vorwahl, x:bevt] from orte as x) nest(x:bevt) as subrel as o2] where [o1:kennzeichen="B",o1:bevth=o2:bevt] orderby [o1:vorwahl] first 1).
testNRQuery(846, [], select * from [(select * from orteh as x) unnest(x:subrel) as o1, (select [x:kennzeichen, x:ort, x:vorwahl, x:bevt] from orte as x) nest([x:bevt, x:vorwahl]) as subrel as o2] where [o1:kennzeichen="B",o1:bevth=o2:bevt] orderby [o1:vorwahl] first 1).
testNRQuery(847, [expectedResult(fail)], select * from [(select * from orteh as x) unnest(x:subrel) as o1, (select [x:bevt div 100 as bevth, x:kennzeichen, x:ort, x:vorwahl, x:bevt] from orte as x) nest([bevth,x:bevt]) as subrel as o2] where [o1:bevth=o2:bevt] orderby [o1:vorwahl] first 1).

% Queries on database literature
% May reveal some more cases to take care about.
% Sometimes the secondo kernel crashes here during computing the 
% catalog informations and destroys the database...don't know why.
/*
testNRQuery(2000, [database(literature)], select * from authordoc first 1).
testNRQuery(2001, [], select * from authordoc as a where [a:name="A. Bar-Hen"]).

testNRQuery(2002, [], select * from authordoc as a where [a:name="A. Bar-Hen", exists(select * from a:details as d)]).
testNRQuery(2003, [], select * from authordoc as a where [a:name="A. Bar-Hen", exists(select * from a:details as d where [exists(select * from d:publications as p where d:year=p:docid)])]).
testNRQuery(2004, [], select * from authordoc as a where [a:name="A. Bar-Hen", exists(select * from a:details as d where [exists(select * from d:publications as p where a:name=p:title)])]).
*/

% Returns by backtracking the subqueries test examples 
testNRQuery(NoNew, [database(opt)], Query) :-
	testQuery(No, Query), 
	% Reduces to futility due too long running times.
	((No > 3, No < 8) ; member(No, [47])),
	NoNew is No + 10000.

% open database optext again because i mostly work there
testNRQuery(100000, [database(optext)], select * from orteh).

testNRQuery(No, Q) :-
	testNRQuery(No, _, Q).

traceNR(No) :- 
	tracegoal(testNR(No)).

testNR(No) :- 
	testNRQuery(No, _, Q), !, sql(Q).

testNR(No, Option) :- 
	member(Option, [trace, t]),!,
	reset,
	traceNR(No).

testNR(No, Option) :- 
	member(Option, [noreset, nor, n]),!,
	traceNR(No).
	
testNR(No, _) :- 
	testNR(No).

:- dynamic 
	testResult/5,
	testRunning/0.

testNR :-
	reload,
	retractall(testResult(_, _, _, _, _)),
	retractall(testRunning),
	asserta(testRunning),
	delOption(memoryAllocation), % incompatible
  setOption(nestedRelations),
	(
		testNRQuery(No, Properties, Query),
		openDatabaseP(Properties),
		% So we check if this is provable, not if the result is correct.
		% That that sql catches exceptions, otherwise more must be done here.
		(getTime(sql(Query), TimeMS) ->
			addResult(Properties, No, TimeMS, ok)
		;
			addResult(Properties, No, -1, failed)
		),
		fail
	).

testNR :- 
	!,
	retractall(testRunning),
	showTestResults.

addResult(Properties, No, TimeMs, ok) :-
	propertyValue(expectedResult, Properties, ER),
	\+ ER, 
	assertz(testResult(No, TimeMs, ok, nok, 'ok (but expected not to work!)')),
	!.
	
addResult(Properties, No, TimeMs, ok) :-
	assertz(testResult(No, TimeMs, ok, ok, 'ok')), 
	!.

addResult(Properties, No, _, failed) :-
  propertyValue(expectedResult, Properties, ER),
  \+ ER,
  assertz(testResult(No, -1, failed, ok, 'failed (expected not to work)')),
  !.

addResult(Properties, No, _, failed) :-
	assertz(testResult(No, -1, failed, nok, failed)),
  !.

addResult(Properties, No, Time, Stat) :-
	throw(addResultFailed::addResult(Properties, No, Time, Stat)).

showTestResults :- 
	!,
	(
		(
			testResult(TID, TimeMS, _, _, RC),
			((number(TimeMS), TimeMS >= 0) ->
  			format('~` t~d~6+ ~` t~d~6+ms ~w~n', [TID, TimeMS, RC])
			;
  			format('~` t~d~6+ ~` t~6+   ~w~n', [TID, RC])
			),
			fail
		) 
	;
		(
			count(testResult(_, _, ok, _, _), OKCount),
			count(testResult(_, _, failed, _, _), FailedCount),
			count(testResult(_, _, _, nok, _), NOKCount),
			write_list(['Summary: OK: ', OKCount, ' failed: ', FailedCount, '\n']),
			write_list([NOKCount, ' returned not with the expected result state.\n']),
			!, 
			NOKCount=0 % Return with true if all tests returns as expected.
		)
	).

openDatabaseP(Properties) :-
	member(database(DB), Properties),
	!,
	openDatabaseD(DB).

openDatabaseP(Properties) :-
	% not really recommended, but for tests runs ~in order~, this works.
	\+ member(database(DB), Properties),
	!.

openDatabaseD(DB) :-
	databaseName(DB), 
	!. % is already open

openDatabaseD(DB) :-
	closeDBIfOpen,
	updateDB(DB), 
	openDB(DB),	
 	!.

openDatabaseD(DB) :-
	throw(failedToOpenDatabase::DB).

closeDBIfOpen :-
	\+ databaseName(_), 
	!.

closeDBIfOpen :-
	databaseName(_),
	!,
	closeDB.

% eof
