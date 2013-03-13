/*
$Header$
@author Nikolai van Kempen

Note: the test run only tests if the goal is provable and not wheather the result is correct. 
	
Before you are able to run the test, you have to create and prepare the test databases. Please make sure you created the test databases with testdbs/recreate.sh.
*/

% Just some simple queries that don't work with nested relations, just to test if I haven't damaged something because subquery-error-tracking is much more complex as for non-subquery queries.
testNRQuery(1, [database(opt)], select * from orte).
testNRQuery(2, [], select bevt from orte).
testNRQuery(3, [], select [bevt, kennzeichen] from orte).
testNRQuery(4, [], select * from orte as oXy).
testNRQuery(5, [], select oXy:bevt from orte as oXy).
testNRQuery(6, [], select [oXy:bevt, oXy:kennzeichen] from orte as oXy).
testNRQuery(7, [], select [oXy:bevt, oXy:kennzeichen] from orte as oXy where [oXy:bevt>10, oXy:bevt<1000]).
testNRQuery(8, [], select * from [orte as o, plz as p] where o:ort=p:ort).
testNRQuery(9, [],  select [ort, min(plz) as minplz, max(plz) as maxplz,  count(*) as cntplz] from plz where plz > 40000 groupby ort).
testNRQuery(10, [], select [ort, plz] from plz orderby [ort asc, plz desc]).
testNRQuery(11, [], select [ort, plz] from plz where ort="Berlin" orderby [ort asc, plz desc]).
testNRQuery(12, [], select aggregate((distinct b:no*1), (*), 'int', [const,int,value,0] ) as fac from [ten as a, ten as b] where [a:no < b:no] groupby a:no).
testNRQuery(13, [], select [ort, min(plz) as minplz, max(plz) as maxplz, count(distinct *) as cntplz] from plz where plz > 40000 groupby ort orderby cntplz desc first 2).
testNRQuery(14, [], select [min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz, count(distinct ort) as ortcnt] from plz groupby []).  
testNRQuery(15, [], select sum(no) from ten).
testNRQuery(16, [], select avg(distinct no) from ten where no > 5).
testNRQuery(17, [], union[select * from orte, select * from orte]).

%return true but the result is crap:
%testNRQuery(131, [], sql union[select count(*) from orte, select count(*) from orte]).
testNRQuery(30, [expectedResult(fail)], select (select count(*) from orte) as label from orte where label=10 first 1).
% Works, but look at the comments of the lookupAttr(Query as Var, ...) for more
% infomation.
testNRQuery(31, [], select (select count(*) from orte) as label from orte first 1).

% Nested relation test queries

% The most simple queries with renaming test on a nrel-relation.
testNRQuery(100, [database(optext)], select * from orteh).
testNRQuery(101, [], select * from orteh as o).
testNRQuery(102, [], select * from orteh as oXXXXX).
testNRQuery(103, [], select * from orteh as oXyZ).
testNRQuery(104, [], select bevth from orteh).
testNRQuery(105, [], select o:bevth from orteh as o).
testNRQuery(106, [], select oXXXXX:bevth from orteh as oXXXXX).
testNRQuery(107, [], select oXyZ:bevth from orteh as oXyZ).
% predication no a attribute that is not within the select clause
testNRQuery(108, [], select subrel from orteh where bevth=34).
testNRQuery(109, [], select o:subrel from orteh as o where o:bevth=34).

% This is currently not possible because the POG can't handle this.
% This works now, look at the comments within the subqueries.pl file.
% predicate: correlationsRels/2.
testNRQuery(110, [], select * from orteh as o where [exists(select * from o:subrel as p)]).

% With subqueries
testNRQuery(111, [], select * from orteh as o where [exists(select * from o:subrel as p where p:bevt>o:bevth)]).
testNRQuery(112, [], select * from orteh as o where [o:bevth>10, exists(select * from o:subrel as p where p:bevt>o:bevth)]).
testNRQuery(113, [], select * from orteh as oAB where [oAB:bevth>10, exists(select * from oAB:subrel as pZxY where pZxY:bevt>oAB:bevth)]).
testNRQuery(114, [], select * from orteh as o where [exists(select * from o:subrel where bevt>o:bevth)]).
testNRQuery(115, [], select * from orteh as o where [o:bevth>10, exists(select * from o:subrel where bevt>o:bevth)]).
testNRQuery(116, [], select * from orteh as o where [o:bevth>10, exists(select bevt from o:subrel where bevt>o:bevth)]).

% works after adding the injectUsedAttrToPreviousSQID/1 predicate.
testNRQuery(117, [], select o:bevth from orteh as o where [o:bevth>10, exists(select bevt from o:subrel where bevt>o:bevth)]).

testNRQuery(118, [], select [o:bevth,o:subrel] from orteh as o where [o:bevth>10, exists(select bevt from o:subrel where bevt>o:bevth)]).

testNRQuery(119, [], select * from orteh as o where [o:bevth>10, exists(select p:bevt from o:subrel as p where p:bevt>o:bevth)]).
testNRQuery(120, [], select * from orteh as o where [o:bevth>10, exists(select [p:bevt, p:kennzeichen] from o:subrel as p where p:bevt>o:bevth)]).
testNRQuery(121, [], select * from orteh as o where [o:bevth>10, not exists(select [p:bevt, p:kennzeichen] from o:subrel as p where p:bevt>o:bevth)]).
testNRQuery(122, [], select * from orteh as o where [o:bevth in (select p:bevt from o:subrel as p where p:bevt>o:bevth)]).
testNRQuery(123, [], select * from orteh as o where [o:bevth not in(select p:bevt from o:subrel as p where p:bevt>o:bevth)]).

% selections
testNRQuery(140, [], select * from orteh as o where [exists(select * from o:subrel as p where [p:bevt>o:bevth, p:bevt>100000])]).
testNRQuery(141, [], select * from orteh as o where [o:bevth < 99, exists(select * from o:subrel as p where [p:bevt>o:bevth, p:bevt<1000000])]).
testNRQuery(141, [], select * from orteh as o where [exists(select * from o:subrel as p where [p:bevt>1000])]).
testNRQuery(142, [], select * from orteh as o where [o:bevth < 99, exists(select * from o:subrel as p where [p:bevt>o:bevth, p:bevt<1000000])]).
testNRQuery(143, [], select * from orteh as o where [o:bevth < 99, exists(select * from o:subrel as p where [p:bevt>o:bevth, p:kennzeichen="H", p:bevt<1000000])]).
testNRQuery(144, [], select * from orteh as o where [o:bevth < 99, not(exists(select * from o:subrel as p where [p:bevt>o:bevth, p:kennzeichen="H", p:bevt<1000000]))]).
testNRQuery(145, [], select [o:bevth, o:subrel] from orteh as o where [o:bevth < 99, exists(select * from o:subrel as p where [p:bevt>o:bevth, p:kennzeichen="H", p:bevt<1000000])]).
testNRQuery(146, [], select * from orteh as o where [o:bevth<199, o:bevth in (select p:bevt from o:subrel as p where p:bevt>o:bevth)]).
testNRQuery(147, [], select * from orteh as o where [o:bevth<199, o:bevth in (select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1])]).
testNRQuery(148, [], select * from orteh as o where [o:bevth<199, o:bevth >( all (select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).
testNRQuery(149, [], select * from orteh as o where [o:bevth<199, o:bevth =(any(select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).
testNRQuery(150, [], select * from orteh as o where [o:bevth<199, 10 >( all (select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).
testNRQuery(151, [], select * from orteh as o where [o:bevth<199, 10 <=(all(select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).
% "= all" queries won't work, i think there a little error within the
% subqueries extension, but it's really difficult to track this error.
testNRQuery(152, [expectedResult(fail)], select * from orteh as o where [o:bevth<199, 10 =(all(select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).
testNRQuery(153, [expectedResult(fail)], select * from orteh as o where [o:bevth<199, o:bevth =(all(select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).

% Is not expected to work, no preTransformNestedPredicate predicates are 
% defined for this.
%testNRQuery(39, select * from orteh as o where [o:bevth<199, o:bevth >(some(select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).
testNRQuery(154, [], select * from orteh as o where [o:bevth<199, o:bevth >(any(select p:bevt from o:subrel as p where [p:bevt>o:bevth,p:bevt>1]))]).

% Multiple subqueries
testNRQuery(200, [], select * from orteh as o 
	where [o:bevth>10, 
		exists(select p:bevt from o:subrel as p where p:bevt>o:bevth),
		exists(select p2:kennzeichen from o:subrel as p2 where [p2:bevt>o:bevth]),
		exists(select pABC3:vorwahl from o:subrel as pABC3 where pABC3:bevt>o:bevth)
	]).

testNRQuery(201, [], select * from orteh as o 
	where [o:bevth>10, 
		exists(select bevt from o:subrel where bevt>o:bevth),
		exists(select kennzeichen from o:subrel where [bevt>o:bevth, bevt=3410]),
		exists(select vorwahl from o:subrel where bevt>o:bevth)
	]).

% sub-sub-query
testNRQuery(202, [], select * from orte as o where [
	exists(select * from ten as t where [t:no<5,t:no=o:bevt,
		exists(select * from thousand as t2 where t2:no=t:no)])]).

testNRQuery(203, [], select * from orte as o where [
		exists(select * from ten as t where [t:no<5,t:no=o:bevt,
			exists(select * from thousand as t2 where t2:no=t:no)]),
		exists(select * from ten as t1 where [t1:no<5,t1:no=o:bevt,
			exists(select * from thousand as t2 where t2:no=t1:no)]),
		exists(select * from ten as t2 where [t2:no<5,t2:no=o:bevt,
			exists(select * from thousand as t3 where t3:no=t2:no)])
	]).

% Reference to a outer-outer query
testNRQuery(204, [expectedResult(fail)], select * from orte as o where [
	exists(select * from ten as t where [t:no<5,t:no=o:bevt,
		exists(select * from thousand as t2 where [t2:no=t:no, o:bevt=t2:no])])]).
% ...within the attribute list
testNRQuery(205, [expectedResult(fail)], select * from orte as o where [
	exists(select * from ten as t where [t:no<5,t:no=o:bevt,
		exists(select [t2:no, o:bevt] from thousand as t2 where [t2:no=t:no])])]).

% joins

testNRQuery(250, [], select * from orteh as o 
	where [exists(select * from [o:subrel as p, plz as p2] where [p:bevt<o:bevth, p:ort=p2:ort])]).
testNRQuery(251, [], select * from orteh as o 
	where [exists(select * from [plz as p2, o:subrel as p] where [p:bevt<o:bevth, p2:ort=p:ort])]).
testNRQuery(252, [], select * from orteh as o where [exists(select * from [plz as p2, o:subrel as p] where [p:bevt<o:bevth+5, p2:ort=p:ort])]).
testNRQuery(253, [expectedResult(fail)], select * from orteh as o where [exists(select [p2:ort, o:bevth+5 as plus5, p:bevt, p:ort] from [plz as p2, o:subrel as p] where [p:bevt<plus5, p2:ort=p:ort])]).
% .. won't work because even this won't work:
testNRQuery(254, [expectedResult(fail)], select bevt+5 as plus5 from orte where plus5=10).

% Attribute-level
% no new name
testNRQuery(300, [expectedResult(fail)], select [bevth, (select * from subrel)]from orteh).
% no renmae for new attribute
testNRQuery(301, [expectedResult(fail)], select [o:bevth, (select * from o:subrel)]from orteh as o).
testNRQuery(302, [], select [o:bevth, (select * from o:subrel) as sub]from orteh as o).
testNRQuery(310, [], select [bevth, (select * from subrel) as sr1]from orteh).

testNRQuery(320, [], select [o:bevth, (select s:kennzeichen from o:subrel as s) as sub]from orteh as o).
testNRQuery(321, [], select [o:bevth, o:subrel, (select s:kennzeichen from o:subrel as s) as sub]from orteh as o).
% with where condition
testNRQuery(322, [], select [o:bevth, (select s:kennzeichen from o:subrel as s where [s:kennzeichen = "B"]) as sub]from orteh as o).
testNRQuery(323, [], select [o:bevth, o:subrel, (select s:kennzeichen from o:subrel as s where [s:kennzeichen = "B"]) as sub]from orteh as o where [exists(select * from o:subrel as s where[s:kennzeichen = "B", s:bevt=o:bevth])]).
% sql select [*] from orte.
% sql select [*, bevt] from orte will fail, too.
testNRQuery(324, [expectedResult(fail)], select [*, (select s:kennzeichen from o:subrel as s where [s:kennzeichen = "B"]) as sub]from orteh as o where [exists(select * from o:subrel as s where[s:kennzeichen = "B", s:bevt=o:bevth])]).

testNRQuery(325, [], select [o:bevth, o:subrel, (select s:kennzeichen from o:subrel as s where [s:kennzeichen = "B"]) as sub]from orteh as o where [exists(select * from o:subrel as s where[s:kennzeichen = "B", s:bevt=o:bevth])]).

testNRQuery(326, [], select [o:bevth, o:subrel, (select sX:kennzeichen from o:subrel as sX where [sX:kennzeichen = "B"]) as sub]from orteh as o where [exists(select sY:kennzeichen from o:subrel as sY where[sY:kennzeichen = "B", sY:bevt=o:bevth])]).
testNRQuery(326, [], select [o:bevth, (select * from o:subrel where [kennzeichen = "B"]) as newSubRel] from orteh as o where [exists(select kennzeichen from o:subrel where[kennzeichen = "B"])]).
testNRQuery(327, [], select [o:bevth, o:subrel, (select s:kennzeichen from o:subrel as s where [s:kennzeichen = "B"]) as sub]from orteh as o where [exists(select * from o:subrel as s where[s:kennzeichen = "B"])]).
testNRQuery(328, [], select [o:bevth, o:subrel, (select count(*) from o:subrel as sX where [sX:kennzeichen = "B"]) as sub]from orteh as o where [exists(select sY:kennzeichen from o:subrel as sY where[sY:kennzeichen = "B", sY:bevt=o:bevth])]).
testNRQuery(329, [], select [o:bevth, o:subrel, (select count(distinct *) from o:subrel as sX where [sX:kennzeichen = "B"]) as sub]from orteh as o where [exists(select sY:kennzeichen from o:subrel as sY where[sY:kennzeichen = "B", sY:bevt=o:bevth])]).
testNRQuery(330, [], select [o:bevth, o:subrel, (select [sX:kennzeichen, count(*) as countlabel] from o:subrel as sX where [sX:kennzeichen = "B"] groupby sX:kennzeichen) as sub]from orteh as o where [exists(select sY:kennzeichen from o:subrel as sY where[sY:kennzeichen = "B", sY:bevt=o:bevth])]).
testNRQuery(331, [], select ort from (select [ort, min(plz) as minplz, max(plz) as maxplz, count(*) as cntplz] from plz where plz > 40000 groupby ort)).
testNRQuery(332, [], select [minplz, ort] from (select [ort, min(plz) as minplz, max(plz) as maxplz, count(*) as cntplz] from plz where plz > 40000 groupby ort)).
testNRQuery(333, [], select [minplz, ox] from (select [ort as ox, min(plz) as minplz, max(plz) as maxplz, count(*) as cntplz] from plz where plz > 40000 groupby ort)).
testNRQuery(334, [], select [y:minplz, y:ox as os] from (select [ort as ox, min(plz) as minplz, max(plz) as maxplz, count(*) as cntplz] from plz where plz > 40000 groupby ort) as y first 1).

testNRQuery(340, [], select [o:bevth, (select * from [o:subrel as s, plz as p] where [s:ort=p:ort]) as sub] from orteh as o).

% malformed: "from o:sub" => sub is not a attribute of orteh as o.
testNRQuery(341, [expectedResult(fail)], select [o:bevth, (select * from [o:subrel as s, plz as p] where [s:ort=p:ort]) as sub] from orteh as o where [exists(select s2:kennzeichen from o:sub as s2 where[s2:kennzeichen = "B", s2:bevt=o:bevth])]).
testNRQuery(342, [expectedResult(fail)], select [o:bevth, (select * from [o:subrel as s, plz as p] where [s:ort=p:ort]) as sub] from orteh as o where [exists(select s2:kennzeichen from sub as s2 where[s2:kennzeichen = "B", s2:bevt=o:bevth])]).
% sub can be accessed without a label.
testNRQuery(343, [expectedResult(fail)], select [o:bevth, o:subrel, (select * from [o:subrel as s, plz as p] where [s:ort=p:ort]) as sub] from (select * from orteh) as o where [exists(select s2:kennzeichen from sub as s2 where[s2:kennzeichen = "B", s2:bevt=o:bevth])]).
% but it is possible to fool the optimizer by rewriting the query to the
% following form (no promise that this is always possible):
testNRQuery(344, [], select [o:bevth, o:sub] from (select [o1:bevth, (select * from [o1:subrel]) as sub] from orteh as o1) as o where [exists(select s2:kennzeichen from o:sub as s2 where[s2:kennzeichen = "B", s2:bevt=o:bevth])]).
testNRQuery(345, [], select [o:bevth, o:sub] from (select [o1:bevth, (select * from [o1:subrel]) as sub] from orteh as o1) as o where [exists(select * from o:sub as s2)]).
testNRQuery(346, [], select [o:bevth, o:sub] from (select [o1:bevth, (select bevt from [o1:subrel]) as sub] from orteh as o1) as o where [exists(select * from o:sub)]).

% Using "s" twice as a variable.
testNRQuery(345, [], select [o:bevth, (select * from [o:subrel as s, plz as p] where [s:ort=p:ort]) as sub] from orteh as o where [exists(select s:kennzeichen from o:subrel as s where[s:kennzeichen = "B", s:bevt=o:bevth])]).
testNRQuery(346, [], select [(select [(select * from ten as t3 first 1) as bl] from ten as t2 first 1) as x] from ten as t first 1).

% subqueries within the from clause
testNRQuery(400, [], select * from (select * from orteh) as o).
testNRQuery(401, [], select * from (select bevth from orteh) as o).
testNRQuery(402, [], select * from (select [bevth] from orteh) as o).
testNRQuery(403, [], select * from (select [bevth, subrel] from orteh) as o).
testNRQuery(404, [], select * from (select [xy:bevth, xy:subrel] from orteh as xy) as o).
testNRQuery(405, [], select [o:bevth] from (select [xy:bevth, xy:subrel] from orteh as xy) as o).
testNRQuery(406, [], select [o:bevth] from (select [xy:bevth, xy:subrel] from orteh as xy) unnest(xy:subrel) as o).
% fail if we apply the unnest operator on a integer attribute.
testNRQuery(407, [expectedResult(fail)], select * from (select [xy:bevth, xy:subrel] from orteh as xy) unnest(xy:bevth) as o).
testNRQuery(408, [], select [o:kennzeichen] from (select [xy:subrel] from orteh as xy) unnest(xy:subrel) as o).
testNRQuery(408, [], select * from (select * from orteh) unnest(subrel) as o).
testNRQuery(430, [], select * from (select * from orte as x) nest(x:bevt, subrel) as o first 1).
testNRQuery(431, [], select [o:bevt] from (select * from orte as x) nest(x:bevt, subrel) as o).
testNRQuery(432, [], select [o:bevt, o:subrel] from (select * from orte as x) nest(x:bevt, subrel) as o).
testNRQuery(433, [], select [o:bevt, o:subrel] from (select [x:bevt, x:kennzeichen] from orte as x) nest(x:bevt, subrel) as o).
testNRQuery(434, [], select [o:bevt, o:subREL] from (select [x:bevt, x:kennzeichen] from orte as x) nest(x:bevt, subREL) as o).
testNRQuery(435, [], select [o:bevt, o:subREL] from (select [x:bevt, x:kennzeichen, x:vorwahl] from orte as x) nest(x:bevt, subREL) as o).

testNRQuery(450, [], select * from (select * from (select * from orte) as o1) as o2 first 1).
testNRQuery(451, [], select * from (select * from (select * from (select * from orte) as o0) as o1) as o2 first 1).
testNRQuery(452, [], select * from (select * from (select * from (select * from orteh) as o0) as o1) as o2 first 1).
testNRQuery(453, [], select * from (select * from (select * from (select * from orte as oh where [oh:kennzeichen="B"]) as o0) as o1) as o2).
testNRQuery(454, [], select * from (select * from (select * from (select * from (select * from orte) as oh where [oh:kennzeichen="B"]) as o0) as o1) as o2).
testNRQuery(455, [], select * from (select * from (select * from (select * from (select kennzeichen from orte) as oh where [oh:kennzeichen="B"]) as o0) as o1) as o2).
testNRQuery(456, [], select * from (select * from (select * from (select * from (select [bevt, vorwahl] from orte orderby vorwahl desc) as oh where [oh:bevt=10]) as o0) as o1) as o2).
testNRQuery(457, [], select * from (select o1:vorwahl from (select * from (select * from (select [bevt, vorwahl] from orte orderby vorwahl desc) as oh where [oh:bevt=10]) as o0) as o1) as o2).
testNRQuery(458, [], select o1:vorwahl from (select [bevt, vorwahl] from orte orderby vorwahl desc) as o1).
testNRQuery(459, [], select o1:vorwahl from (select [o0:bevt, o0:vorwahl] from orte as o0 orderby o0:vorwahl desc) as o1).

testNRQuery(470, [], select * from (select * from orte as o) nest(o:bevt, subRel) as o1).
testNRQuery(470, [], select * from (select * from orte) nest(bevt, subRel) as o1).
testNRQuery(471, [], select * from (select [o:bevt, o:kennzeichen] from orte as o) nest(o:bevt, subRel) as o1).
testNRQuery(471, [], select * from (select [bevt div 100 as bevth, ort, kennzeichen, vorwahl] from orte) nest(bevth, subRel) as o1).
testNRQuery(472, [], select o1:subrel from (select [o:bevt, o:kennzeichen] from orte as o) nest(o:bevt, subrel) as o1).
testNRQuery(473, [], select * from (select o1:subrel from (select [o:bevt, o:kennzeichen] from orte as o) nest(o:bevt, subrel) as o1) as o2).
testNRQuery(474, [], select * from (select * from orte as o) nest([o:bevt, o:vorwahl, o:kennzeichen], subRel) as o1).
% Nest on a already nested relation
testNRQuery(475, [], select * from (select * from staedtenested as s1) nest([s1:sname], sub2) as s2).
% Aggregate subqueries within the from clause
testNRQuery(480, [], select * from (select [min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz, count(distinct ort) as ortcnt] from plz groupby []) as subrel).  
testNRQuery(481, [], select s:minplz from (select [min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz, count(distinct ort) as ortcnt] from plz groupby []) as s).  
testNRQuery(482, [], select * from(select [ort, min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz] from plz groupby[ort]) as x).
testNRQuery(483, [], select x:ort from(select [ort, min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz] from plz groupby[ort]) as x).
testNRQuery(484, [], select [count(*) as countattr] from(select [ort, min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz] from plz groupby[ort]) as x groupby[]).
testNRQuery(485, [], select [x:ort, count(*) as countattr] from(select [ort, min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz] from plz groupby[ort]) as x groupby[x:ort]).
testNRQuery(486, [], select [s:ort, count(*) as count] from (select [ort, min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz] from plz groupby[ort]) as s groupby[s:ort]).
testNRQuery(487, [], select [s:ort, count(*) as count] from (select [ort, min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz] from plz where ort="Berlin" groupby[ort]) as s where s:ort="Berlin" groupby[s:ort]).
testNRQuery(488, [], select [s:ort, s:plz, count(*) as count] from (select [ort, plz, min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz] from plz where ort="Berlin" groupby[ort, plz]) as s where s:ort="Berlin" groupby[s:ort, s:plz]).
testNRQuery(489, [], select [s:ort, s:plz, count(*) as count] from [(select [ort, plz, min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz] from plz where ort="Berlin" groupby[ort, plz]) as s, (select * from orteh) unnest(subrel) as oh] where [s:ort="Berlin", s:ort=oh:ort] groupby[s:ort, s:plz]).
testNRQuery(490, [], select [s:ort, s:plz, oh:kennzeichen, count(*) as count] from [(select [ort, plz, min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz] from plz where ort="Berlin" groupby[ort, plz]) as s, (select * from orteh) unnest(subrel) as oh] where [s:ort="Berlin", s:ort=oh:ort] groupby[s:ort, s:plz, oh:kennzeichen]).
testNRQuery(491, [], select [count(*) as count] from [(select [ort, plz, min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz] from plz where ort="Berlin" groupby[ort, plz]) as s, (select * from orteh) unnest(subrel) as oh] where [s:ort="Berlin", s:ort=oh:ort] groupby[]).
testNRQuery(492, [], select distinct(s:ort) from [(select [ort, plz, min(plz) as minplz, max(plz) as maxplz, avg(plz) as avgplz] from plz where ort="Berlin" groupby[ort, plz]) as s, (select * from orteh) unnest(subrel) as oh] where [s:ort="Berlin", s:ort=oh:ort]).

/*
Works, but the result is not correct. It can not be possible in this way because a count(*) does not return a tuple stream.
See the comments of the nrSubqueryToStream/3 predicate for more information.
Update: Now this works
*/
testNRQuery(500, [expectedResult(fail)], select count(*) as summe from orte).
testNRQuery(501, [], select * from (select count(*) from orte as o0) as o1).
testNRQuery(502, [], select * from (select count(distinct kennzeichen) from orte) as o1).
/*
testNRQuery(503, [], select * from (select sum(o0:bevt) from orte as o0) as o1).
testNRQuery(504, [], select * from (select max(bevt) from orte) as o1).
testNRQuery(505, [], select * from (select min(bevt) from orte) as o1).
testNRQuery(506, [], select * from (select avg(bevt) from orte) as o1).
*/

testNRQuery(507, [], select [kennzeichen, sum(bevt) as x] from orte groupby kennzeichen).
testNRQuery(508, [], select * from (select [kennzeichen, sum(bevt) as x] from orte groupby kennzeichen) as oOOOo).
testNRQuery(509, [], select * from (select [kennzeichen, sum(bevt) as sumEinwohner] from orte groupby kennzeichen) as y where y:kennzeichen="B").
% There is no "collect" operator for this
testNRQuery(510, [expectedResult(fail)], select [x:bevt, (select o:kennzeichen from orte as o where o:bevt=x:bevt) as subrel] from orte as x groupby [x:bevt]).
testNRQuery(511, [],  select [x:bevt, (select o:kennzeichen from orte as o where o:bevt=x:bevt) as subrel] from orte as x).

% subqueries within the from clause
% first only with the deep 1.
testNRQuery(530, [], select * from (select * from orte) as o).
testNRQuery(531, [], select * from (select * from orte)).
testNRQuery(532, [], select * from (select * from orte) as o first 1).
testNRQuery(533, [], select * from (select * from orte) as o orderby o:bevt first 1).
testNRQuery(534, [], select o:bevt from (select * from orte) as o).
testNRQuery(535, [], select o:bevt from (select bevt from orte) as o).
testNRQuery(536, [], select o:bevt from (select x:bevt from orte as x) as o first 1).

% unnesting
testNRQuery(550, [], select o:bevt from (select subrel from orteh) unnest(subrel) as o first 1).
testNRQuery(551, [], select [o:bevth, o:bevt] from (select * from orteh) unnest(subrel) as o first 1).
testNRQuery(552, [], select [o:bevth, o:bevt, o:kennzeichen] from (select * from orteh) unnest(subrel) as o orderby [o:kennzeichen] first 1).
testNRQuery(553, [], select * from (select subrel from orteh) unnest(subrel) as o orderby [o:kennzeichen] first 1).
testNRQuery(554, [], select * from (select x:subrel from orteh as x) unnest(x:subrel) as o orderby [o:kennzeichen] first 1).
testNRQuery(555, [], select [o:bevt, o:bevth, o:kennzeichen] from (select [x:bevth,x:subrel] from orteh as x) unnest(x:subrel) as o orderby [o:kennzeichen] first 1).
testNRQuery(556, [], select [o:bevt, o:bevth, o:kennzeichen] from (select * from orteh as x) unnest(x:subrel) as o orderby [o:kennzeichen] first 1).

% nesting
testNRQuery(570, [], select * from (select * from orte as x) nest(x:bevt, subrel) as o first 1).
testNRQuery(571, [], select * from (select * from orte as x) nest(x:bevt) as subrel as o first 1).

% o:kennzeichen is moved into the arel attribute.
% so the query is written wrong.
testNRQuery(572, [expectedResult(fail)], select * from (select * from orte as x) nest(x:bevt, subrel) as o orderby [o:kennzeichen] first 1).
% sorting is only allowed over attributes. nest sorts always the query over its nesting attributes.
testNRQuery(573, [expectedResult(fail)], select * from (select * from orte as x orderby x:kennzeichen) nest(x:bevt, subrel) as o where exists(select * from o:subrel as s where [s:kennzeichen="B", s:vorwahl=o:bevt]) first 1).

testNRQuery(574, [], select * from (select [x:bevt div 100 as bevth, x:kennzeichen, x:ort, x:vorwahl, x:bevt] from orte as x) nest(x:bevt) as subrel as o first 1).
testNRQuery(575, [], select * from (select [x:bevt div 100 as bevth, x:kennzeichen, x:ort, x:vorwahl, x:bevt] from orte as x) nest(bevth) as subrel as o first 1).

testNRQuery(576, [], select [o:bevt, o:kennzeichen] from (select x:subrel from orteh as x) unnest(x:subrel) as o where o:kennzeichen="B" orderby [o:kennzeichen] first 1).
testNRQuery(577, [], select [o:bevt, o:bevth, o:kennzeichen] from (select [x:bevth,x:subrel] from orteh as x) unnest(x:subrel) as o where o:kennzeichen="B" orderby [o:kennzeichen] first 1).
testNRQuery(578, [], select * from (select [x:bevth,x:subrel] from orteh as x) unnest(x:subrel) as o where o:kennzeichen="B" orderby [o:kennzeichen] first 1).
testNRQuery(579, [], select * from (select * from orteh as x) unnest(x:subrel) as o where o:kennzeichen="B" orderby [o:kennzeichen] first 1).
testNRQuery(580, [], select * from [(select * from orteh as x) as o] where [o:bevth=10] first 1).
testNRQuery(581, [], select * from [(select [bevt div 100 as bevth, bevt, ort, kennzeichen, vorwahl] from   orte) nest(bevth, subRel) as x1, (select [bevt div 100 as bevth,bevt, ort, kennzeichen, vorwahl]  from   orte) nest(bevth, subRel) as x2] where x1:bevth=x2:bevth).

% joins
testNRQuery(590, [], select * from [(select * from orteh as x) as o1, (select * from orteh as x) as o2] where [o1:bevth=10,o1:bevth=o2:bevth] first 1).
testNRQuery(591, [], select * from [(select * from orteh as x) as o1, (select * from orteh as x) as o2] where [o1:bevth=10,o1:bevth=o2:bevth-1] first 1).
testNRQuery(592, [], select * from [(select * from orteh as x) unnest(x:subrel) as o1, (select * from orteh as x) as o2] where [o1:kennzeichen="B",o1:bevth=o2:bevth] orderby [o1:vorwahl] first 1).
% this works now...
testNRQuery(593, [], select * from [(select * from orteh as x) unnest(x:subrel) as o1, (select [x:bevt div 100 as bevth, x:kennzeichen, x:ort, x:vorwahl, x:bevt] from orte as x) nest(bevth) as subrel as o2] where [o1:kennzeichen="B",o1:bevth=o2:bevth] orderby [o1:vorwahl] first 1).
% but this not... (because in the first case the selectivities are faked)
testNRQuery(594, [expectedResult(fail)], select[bevt div 100 as bevth]from orte where [bevth=10]).

testNRQuery(595, [], select * from [(select * from orteh as x) unnest(x:subrel) as o1, (select [x:kennzeichen, x:ort, x:vorwahl, x:bevt] from orte as x) nest(x:bevt) as subrel as o2] where [o1:kennzeichen="B",o1:bevth=o2:bevt] orderby [o1:vorwahl] first 1).
testNRQuery(596, [], select * from [(select * from orteh as x) unnest(x:subrel) as o1, (select [x:kennzeichen, x:ort, x:vorwahl, x:bevt] from orte as x) nest([x:bevt, x:vorwahl]) as subrel as o2] where [o1:kennzeichen="B",o1:bevth=o2:bevt] orderby [o1:vorwahl] first 1).
testNRQuery(597, [], select * from [(select * from orteh as x) unnest(x:subrel) as o1, (select [x:bevt div 100 as bevth, x:kennzeichen, x:ort, x:vorwahl, x:bevt] from orte as x) nest([bevth,x:bevt]) as subrel as o2] where [o1:bevth=o2:bevt] orderby [o1:vorwahl] first 1).

% increase complexity...
testNRQuery(600, [], select (select * from ortem2 where bevm=3) as ortem from ten first 1).
testNRQuery(601, [], select (select * from (select * from ortem2) as xy) as m from ten first 1).
testNRQuery(602, [], select (select * from (select * from ortem2 first 1) unnest(subm) as xy) as m from ten first 1).
testNRQuery(603, [], select (select * from (select [t:bevm, t:subm] from ortem2 as t where t:bevm=3 first 1) unnest(t:subm) as xy) as m from ten first 1).
testNRQuery(604, [],  select [(select * from (select [t:bevm, t:subm] from ortem2 as t where t:bevm=3 first 1) unnest(t:subm) as xy) as m, (select * from orteh) as as2, (select * from (select * from orteh) unnest(subrel) as r3) as as3] from ten first 1).
testNRQuery(605, [], select [(select * from (select [t:bevm, t:subm] from ortem2 as t where t:bevm=3 first 1) unnest(t:subm) as xy) as m, (select * from orteh) as as2, (select * from (select * from orte) nest(bevt, subrel3) as r3) as as3] from ten first 1).
testNRQuery(606, [], select [(select * from (select [t:bevm, t:subm] from ortem2 as t where t:bevm=3 first 1) unnest(t:subm) as xy) as m, (select * from orteh) as as2, (select * from (select * from orte) nest(bevt, subrel3) as r3) as as3] from ortem2 as om2 where om2:bevm=3 first 1).
testNRQuery(607, [], select [(select * from (select [t:bevm, t:subm] from ortem2 as t where t:bevm=3 first 1) unnest(t:subm) as xy) as m, (select * from orteh) as as2, (select * from (select * from orte) nest(bevt, subrel3) as r3) as as3, (select * from (select * from (select * from orteh) unnest(subrel) as r5) nest(r5:bevt, subrel4) as r4) as as4] from ortem2 as om2 where om2:bevm=3 first 1).
testNRQuery(608, [], select [(select * from (select [t:bevm, t:subm] from ortem2 as t where t:bevm=3 first 1) unnest(t:subm) as xy) as m, (select * from orteh) as as2, (select * from (select * from orte) nest(bevt, subrel3) as r3) as as3, (select * from (select * from (select * from orteh) unnest(subrel) as r5) nest(r5:bevt, subrel4) as r4) as as4, (select plz from plz where ort="Berlin") as aberlin] from ortem2 as om2 where om2:bevm=3 first 1).
testNRQuery(609, [], select [(select * from (select [t:bevm, t:subm] from ortem2 as t where t:bevm=3 first 1) unnest(t:subm) as xy) as m, (select * from orteh) as as2, (select * from (select * from orte) nest(bevt, subrel3) as r3) as as3, (select * from (select * from (select * from orteh) unnest(subrel) as r5) nest(r5:bevt, subrel4) as r4) as as4, (select plz from plz where ort="Berlin") as aberlin, (select * from orteh as oh where exists(select * from oh:subrel where kennzeichen="B")) as abkennzeichen] from ortem2 as om2 where om2:bevm=3 first 1).
testNRQuery(610, [], select [(select * from (select [t:bevm, t:subm] from ortem2 as t where t:bevm=3 first 1) unnest(t:subm) as xy) as m, (select * from orteh) as as2, (select * from (select * from orte) nest(bevt, subrel3) as r3) as as3, (select * from (select * from (select * from orteh) unnest(subrel) as r5) nest(r5:bevt, subrel4) as r4) as as4, (select plz from plz where ort="Berlin") as aberlin, (select * from orteh as oh where exists(select [vorwahl] from oh:subrel where kennzeichen="B")) as abkennzeichen] from [ortem2 as om2, ten as tentable] where [om2:bevm=3, om2:bevm=tentable:no] first 1).
testNRQuery(611, [], select [om2:subm, (select * from (select [t:bevm, t:subm] from ortem2 as t where t:bevm=3 first 1) unnest(t:subm) as xy) as m, (select * from orteh) as as2, (select * from (select * from orte) nest(bevt, subrel3) as r3) as as3, (select * from (select * from (select * from orteh) unnest(subrel) as r5) nest(r5:bevt, subrel4) as r4) as as4, (select plz from plz where ort="Berlin") as aberlin, (select * from orteh as oh where exists(select [vorwahl] from oh:subrel where kennzeichen="B")) as abkennzeichen] from [ortem2 as om2, ten as tentable] where [om2:bevm=3, om2:bevm=tentable:no, exists(select * from om2:subm), exists(select * from [om2:subm, orte as o8] where [o8:bevt=bevth]), om2:bevm in(select no from ten)] first 1).
testNRQuery(612, [], select [om2:subm, (select * from (select [t:bevm, t:subm] from ortem2 as t where t:bevm=3 first 1) unnest(t:subm) as xy) as m, (select * from orteh) as as2, (select * from (select * from orte) nest(bevt, subrel3) as r3) as as3, (select * from (select * from (select * from orteh) unnest(subrel) as r5) nest(r5:bevt, subrel4) as r4) as as4, (select plz from plz where ort="Berlin") as aberlin, (select * from orteh as oh where exists(select [vorwahl] from oh:subrel where kennzeichen="B")) as abkennzeichen] from [ortem2 as om2, ten as tentable] where [om2:bevm=3, om2:bevm=tentable:no, exists(select * from om2:subm), exists(select * from [om2:subm, orte as o8] where [o8:bevt=bevth]), om2:bevm in(select no from ten), exists(select (select * from r10:subh as r20 where r20:ort="Berlin") as attr11 from om2:subm as r10)] first 1).
testNRQuery(613, [], select [om2:subm, (select * from (select [t:bevm, t:subm] from ortem2 as t where t:bevm=3 first 1) unnest(t:subm) as xy) as m, (select * from orteh) as as2, (select * from (select * from orte) nest(bevt, subrel3) as r3) as as3, (select * from (select * from (select * from orteh) unnest(subrel) as r5) nest(r5:bevt, subrel4) as r4) as as4, (select plz from plz where ort="Berlin") as aberlin, (select * from orteh as oh where exists(select [vorwahl] from oh:subrel where kennzeichen="B")) as abkennzeichen] from [ortem2 as om2, ten as tentable] where [om2:bevm=3, om2:bevm=tentable:no, exists(select * from om2:subm), exists(select * from [om2:subm, orte as o8] where [o8:bevt=bevth]), om2:bevm in(select no from ten), exists(select * from om2:subm as r10 where exists(select vorwahl from r10:subh))] first 1).
testNRQuery(614, [], select [om2:subm, (select * from (select [t:bevm, t:subm] from ortem2 as t where t:bevm=3 first 1) unnest(t:subm) as xy) as m, (select * from orteh) as as2, (select * from (select * from orte) nest(bevt, subrel3) as r3) as as3, (select * from (select * from (select * from orteh) unnest(subrel) as r5) nest(r5:bevt, subrel4) as r4) as as4, (select plz from plz where ort="Berlin") as aberlin, (select * from orteh as oh where exists(select [vorwahl] from oh:subrel where kennzeichen="B")) as abkennzeichen] from (select * from [ortem2, ten] where[bevm=no]) as om2 where [om2:bevm=3, exists(select * from om2:subm), exists(select * from [om2:subm, orte as o8] where [o8:bevt=bevth]), om2:bevm in(select tenvar:no from ten as tenvar), exists(select * from om2:subm as r10 where exists(select vorwahl from r10:subh))] first 1).


% Following some queries that are reduced to these parts of the above queries
% that uncoverd some problems.
testNRQuery(620, [], select * from ortem2 as o where [exists(select * from o:subm as r where exists(select * from r:subh))]).
testNRQuery(621, [], select * from ortem2 as o where [exists(select [r:bevth, r:subh] from o:subm as r where exists(select * from r:subh))]).

testNRQuery(630, [], select [o:bevm,o:subm] from ortem2 as o where [exists(select * from o:subm as r)]).
testNRQuery(631, [], select o:bevm from ortem2 as o where [exists(select * from o:subm as r)]).
% usedAttr are not return to the outer query by default. 
% the predicate injectUsedAttrToPreviousSQID ensures now that the usedAttr
% facts are now inserted.
testNRQuery(632, [], select o:bevt from orte as o where [o:bevt=105, exists(select * from orte as o2 where o:vorwahl=o2:vorwahl)]).
testNRQuery(633, [], select [o:bevt,o:vorwahl] from orte as o where [o:bevt=105, exists(select * from orte as o2 where o:vorwahl=o2:vorwahl)]).
testNRQuery(634, [], select * from (select * from [ortem2, ten] where[bevm=no]) as om2 where[om2:bevm=3] first 1).
testNRQuery(635, [], select [om2:bevm] from (select * from [ortem2, ten] where[bevm=no]) as om2 where [om2:bevm in(select no from ten)] first 1).
testNRQuery(636, [], select * from orte where [bevt in(select no from ten)]).
testNRQuery(637, [], select * from orte where [bevt in(select no from ten where no=bevt)]).
testNRQuery(638, [], select * from orte as o where [o:bevt in(select no from ten where no=o:bevt)]).

testNRQuery(1000, [], select [o:bevth, (select * from o:subrel where kennzeichen="KLE") as sub] from orteh as o where [exists(select * from o:subrel where kennzeichen="KLE")] first 3).
% whatever this means...
testNRQuery(1001, [], select * from orteh orderby subrel last 2).

% create table for nested relations.
testNRQuery(1002, [preexecgoal(sql drop table crtest1;true)], create table crtest1 columns [att1: int]).
testNRQuery(1003, [], insert into crtest1 select no from thousand).
testNRQuery(1004, [], drop table crtest1).

testNRQuery(1010, [preexecgoal(sql drop table crtest2);true], create table crtest2 columns [bevTH:int, ort:string]).
testNRQuery(1011, [], drop table crtest2).

testNRQuery(1020, [preexecgoal(sql drop table crtest3;true)], create table crtest3 columns [bevTH:int, ort:string, subrel:arel([kennzeichen:int])]).
testNRQuery(1021, [], drop table crtest3).


testNRQuery(1030, [preexecgoal(sql drop table crorteh;true)], create table crorteh columns [ bevTH: int, subRel:arel([kennzeichen: string, ort: string, vorwahl: string, bevT:int]) ]).
testNRQuery(1031, [expectedResult(fail)], insert into crorteh select * from orteh).
testNRQuery(1032, [], drop table crorteh).

testNRQuery(1100, [], select [o:bevth, (select ort from o:subrel) as newsubrel] from orteh as o).
testNRQuery(1101, [], select [o:bevth, (select ort from o:subrel where ort starts "B") as newsubrel] from orteh as o).
testNRQuery(1102, [],  select * from orteh as o where [exists(select ort from o:subrel where ort starts "B")]).
testNRQuery(1103, [], select [o:ort, (select * from plz where ort=o:ort) as newsubrel] from orte as o).

testNRQuery(1107, [], select (select * from subrel as s) as sr from orteh).

testNRQuery(1200, [], select * from orteh unnest subrel).
testNRQuery(1201, [], select * from orteh unnest(subrel) as o).
% Subrel is no longer avaiable
testNRQuery(1202, [expectedResult(fail)], select [o:subrel] from orteh unnest (subrel) as o).
testNRQuery(1203, [], select [o:bevth] from orteh unnest(subrel) as o).
testNRQuery(1204, [], select [o:bevth, o:kennzeichen] from orteh unnest (subrel) as o).
testNRQuery(1205, [], select * from (select * from orteh unnest subrel as o) as x).
testNRQuery(1206, [], select x:kennzeichen from (select [o:bevt,o:kennzeichen] from orteh unnest subrel as o) as x).
testNRQuery(1207, [], select * from orteh unnest(subrel) as o).
testNRQuery(1208, [], select o:kennzeichen from orteh unnest subrel as o).
testNRQuery(1209, [], select kennzeichen from orteh unnest subrel where kennzeichen="B").
testNRQuery(1210, [], select ort from orteh unnest subrel where kennzeichen="B").
testNRQuery(1211, [], select * from plz nest(ort, postleitzahlen) as o).
testNRQuery(1212, [], select (select * from m:subm unnest(subh) as x) as r from ortem as m).
testNRQuery(1213, [], select * from orte nest(o:bevt, subrel)).
testNRQuery(1214, [], select * from orte nest(o:bevt, subrel) as t).
testNRQuery(1215, [], select * from (orte as t) nest(o:bevt, subrel)).
testNRQuery(1216, [], select * from (orte as t) nest(o:bevt, subrel) as x).
testNRQuery(1217, [], select * from (select * from orte) nest(o:bevt, subrel)).
testNRQuery(1218, [], select * from (select * from orte as o) nest(o:bevt, subrel)).
testNRQuery(1219, [], select * from orte nest(bevt, subrel) where bevt=3410).
testNRQuery(1220, [], select * from (ortem unnest(subm)) unnest(subh)).
testNRQuery(1221, [], select * from (ortem unnest(subm)) unnest(subh) where bevt>3000).
testNRQuery(1222, [], select ort from (ortem unnest(subm)) unnest(subh) where bevt>3000).
testNRQuery(1223, [], select p:plz from [(ortem unnest(subm)) unnest(subh), plz as p] where [bevt>3000, p:ort=ort]).
testNRQuery(1224, [], select * from ortem unnest(subm) unnest(subh) nest(bevth, newsubr)).
testNRQuery(1225, [], select * from (select * from ortem unnest(subm) unnest(subh)) nest(bevth, subr)).
testNRQuery(1226, [], select * from (select [bevt div 100 as bevth, ort, kennzeichen, vorwahl] from orte) nest(bevth, subRel)).
testNRQuery(1227, [], select * from orteh unnest subrel).
testNRQuery(1228, [], select * from (select [bevt div 100 as bevth, bevt, ort, kennzeichen, vorwahl] from orte) nest(bevth, subRel)).
testNRQuery(1229, [], select * from (select [bevt div 100 as bevth, bevt, ort, kennzeichen, vorwahl] from orte) nest(bevth, subRel) where [bevth>30]).
testNRQuery(1230, [], select [ort, (select count(*) from plz as p where p:ort=ort) as sr] from orte).
testNRQuery(1231, [], select [o:bevth, (select ort from o:subrel where ort starts "B") as newsubrel]from (select * from (select [bevt div 100 as bevth, bevt, ort, kennzeichen, vorwahl] from orte) nest(bevth, subRel)) as o where [exists(select ort from o:subrel where[ort starts "B"])]).
testNRQuery(1232, [], select count(*) from ortem2 unnest(subm)).
% Special alias resolve (skips one level)
testNRQuery(1233, [], select [(select [(select * from o:subrel first 1) as rr] from ten) as abc] from orteh as o first 1).
% Same case, but without using an alias.
testNRQuery(1234, [], select [(select [(select * from subrel first 1) as rr] from ten) as abc] from orteh  first 1).
testNRQuery(1235, [], select [(select o:bevth as ttt from o:subrel as s) as xy] from orteh as o).
testNRQuery(1236, [], select * from orteh unnest(subrel) nest(bevth, subrel2) unnest(subrel2)).
% fix columnname count!
testNRQuery(1237, [], select count from (select count(*) from orteh) first 1).
testNRQuery(1238, [], select x:count from (select count(*) from orteh where bevth=3) as x first 1).
testNRQuery(1239, [], select x:count from [(select count(*) from orteh where bevth=3) as x, ten]).
testNRQuery(1240, [], select x:count from [(select count(*) from orteh where bevth=3) as x, ten] where x:count=no).
testNRQuery(1241, [], select value*100 as result from (select sum(bevth) from orteh)).




testNRQuery(1300, [], insert into plz values [99999, "fake city"]).
testNRQuery(1301, [], delete from plz where [plz=99999, ort="fake city"]).

% Queries on database literature
% May reveal some more cases to take care of.
/*
testNRQuery(2000, [database(literature)], select * from authordoc first 1).
testNRQuery(2001, [], select * from authordoc as a where [a:name="A. Bar-Hen"]).

testNRQuery(2002, [], select * from authordoc as a where [a:name="A. Bar-Hen", exists(select * from a:details as d)]).
testNRQuery(2003, [], select * from authordoc as a where [a:name="A. Bar-Hen", exists(select * from a:details as d where [exists(select * from d:publications as p where d:year=p:docid)])]).
testNRQuery(2004, [], select * from authordoc as a where [a:name="A. Bar-Hen", exists(select * from a:details as d where [exists(select * from d:publications as p where a:name=p:title)])]).
*/

% Test cases for unnesting a mpoint attributes into it's units.
testNRQuery(3000, [database(berlintest)], select * from trains first 1).
testNRQuery(3001, [], select * from trains where id=531).
testNRQuery(3002, [], select [t:id, t:trip] from trains as t where t:id=531).
testNRQuery(3003, [], select * from (select [t:id, t:trip] from trains as t where t:id=531) as t2).
testNRQuery(3004, [], select * from (select [t:id, t:trip] from trains as t where t:id=531) unnest(t:trip) as t2).
testNRQuery(3005, [], select * from (select [t:id, t:trip] from trains as t where t:id=531) unnest(t:trip) as t2 first 1).
testNRQuery(3006, [], select * from (select [id, trip] from trains where id=531)  unnest(trip) as t2 first 1).
% this query unnests the mpoint and in the next steps the upoint are collected
% into a nested relation.
testNRQuery(3007, [], select * from (select * from (select * from trains where id=531)  unnest(trip) as t2) nest([t2:id,t2:line,t2:up], trips) as x).
% and now get only the id...
testNRQuery(3008, [], select x2:id from (select * from (select * from (select * from trains where id=531)  unnest(trip) as t2) nest([t2:id,t2:line,t2:up], trips) as x) as x2).
% join it agin with the trains table.
testNRQuery(3009, [], select * from [(select * from (select * from (select * from trains where id=531)  unnest(trip) as t2) nest([t2:id,t2:line,t2:up], trips) as x) as x2, trains as t4] where t4:id=x2:id).
testNRQuery(3010, [], select count(*) from trains unnest(trip)).
testNRQuery(3011, [], select * from (select * from trains unnest(trip) where id=531) nest([id,line,up], trips) as x).

% some cases to test some other stuff, but for relations with moving or spatial
% objects. It's uncertain how far everthing works.
testNRQuery(3100, [], select count(*) from trains where trip passes mehringdamm).

testNRQuery(3101, [preexecgoal((secondo('let seven05 = theInstant(2003,11,20,7,5);true')))], select [id, line, up, val(trip atinstant seven05) as pos] from trains where [trip passes mehringdamm, trip present seven05]).
testNRQuery(3102, [], select * from trains where [not(isempty(deftime(intersection(trip, msnow))))]).
testNRQuery(3103, [], select [id, intersection(trip, msnow) as insnow] from trains where [not(isempty(deftime(intersection(trip, msnow))))]).



% Returns by backtracking the subqueries test examples 
testNRQuery(NoNew, [database(opt)], Query) :-
	testQuery(No, Query), 
	% Reduces to futility due too long running times.
	((No > 3, No < 8) ; member(No, [47])),
	NoNew is No + 10000.

% open database optext again because I mostly work there
testNRQuery(100000, [database(optext)], select * from orteh first 1).

testNRQuery(No, Q) :-
	testNRQuery(No, _, Q).

traceNR(No) :- 
	tracegoal(testNR(No)).

testNR(No) :- 
	testNRQuery(No, _, Q), !, write(Q), sql(Q).

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

% Supports nasty source code reloading in my test environment.
:- multifile reload/0.
reload.

testNR :-
	reload, % this is to simplify my testings, but when using this, the 
	% catalogs needs be reloaded.
	retractall(testResult(_, _, _, _, _)),
	retractall(testRunning),
	asserta(testRunning),
	delOption(memoryAllocation), % incompatible
  setOption(nestedRelations),
	(
		testNRQuery(No, Properties, Query), 
		processProperties(Properties),
		% We check whether this is provable and not if the result is correct or not.
		% The sql predicates catches exceptions. Otherwise more must be done at
		% this point.
		write_list(['\n\nRun query: ', Query, '\n']),
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


% Test queries within a given range of query numbers. 3rd argument not needed.


testNR(Nmin, Nmax, _) :-
	reload, % this is to simplify my testings, but when using this, the 
	% catalogs needs be reloaded.
	retractall(testResult(_, _, _, _, _)),
	retractall(testRunning),
	asserta(testRunning),
	delOption(memoryAllocation), % incompatible
  setOption(nestedRelations),
	(
		testNRQuery(No, Properties, Query), No >= Nmin, No =< Nmax,
		processProperties(Properties),
		% We check whether this is provable and not if the result is correct or not.
		% The sql predicates catches exceptions. Otherwise more must be done at
		% this point.
		write_list(['\n\nRun query: ', Query, '\n']),
		(getTime(sql(Query), TimeMS) ->
			addResult(Properties, No, TimeMS, ok)
		;
			addResult(Properties, No, -1, failed)
		),
		fail
	).

testNR(_, _, _) :- 
	!,
	retractall(testRunning),
	showTestResults.

writeTestNR :-
  testNRQuery(_, [expectedResult(fail)], _),
  fail.

writeTestNR :-
  testNRQuery(No, Properties, Query),
  No < 10000,
  write('# Query No '), write(No), write(':'), nl, nl, 
  ( Properties = [database(DB)] -> 
      write('{close database | open database '), write(DB), write('}'), nl, nl
  ;
      write('')
  ),
  ( Properties = [expectedResult(fail)] -> 
      write('# known to fail'), nl, nl
  ;
      ( write('query '), write(No), nl, nl,
        write(Query), nl, nl
      )
  ),
  fail.

writeTestNR.
  





processProperties(Properties) :-
	openDatabaseP(Properties),
	(member(preexecgoal(Goal), Properties) ->
		Goal
	;
		true
	).

addResult(Properties, No, TimeMs, ok) :-
	propertyValue(expectedResult, Properties, ER),
	\+ ER, 
	assertz(testResult(No, TimeMs, ok, nok, 'ok (but expected not to work!)')),
	!.
	
addResult(_, No, TimeMs, ok) :-
	assertz(testResult(No, TimeMs, ok, ok, 'ok')), 
	!.

addResult(Properties, No, _, failed) :-
  propertyValue(expectedResult, Properties, ER),
  \+ ER,
  assertz(testResult(No, -1, failed, ok, 'failed (expected not to work)')),
  !.

addResult(_, No, _, failed) :-
	assertz(testResult(No, -1, failed, nok, failed)),
  !.

addResult(Properties, No, Time, Stat) :-
	throw(error_Internal(test_addResult(Properties, No, Time, Stat)::failed)).

showTestResults :- 
	!,
	(
		(
			testResult(TID, TimeMS, _, _, RC),
			((number(TimeMS), TimeMS >= 0) ->
  			format('~` t~d~6+ ~` t~d~8+ms ~w~n', [TID, TimeMS, RC])
			;
  			format('~` t~d~6+ ~` t~8+   ~w~n', [TID, RC])
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
	% not really recommended, but for test runs ~in order~, this works.
	\+ member(database(_), Properties),
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
	throw(error_Internal(test_openDatabaseD(DB)::'Failed to open database.')).

closeDBIfOpen :-
	\+ databaseName(_), 
	!.

closeDBIfOpen :-
	databaseName(_),
	!,
	closeDB.

% eof
