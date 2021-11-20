/*
Test implementation of N-tree (one node only)

*/
% Steps for starting:
% open database newyorknew
% [ntree]
% reset
% init



% candidates(N, M) :- set up centers N to M as candidates.

:- dynamic candidate/1.

candidates(M, M):- 
  assert(candidate(M)),
  !.
  
candidates(N, M) :-
  N < M,
  assert(candidate(N)),
  N1 is N + 1, 
  candidates(N1, M).
  
  
  
  
candidates2([]).

candidates2([ [N, _] | L]) :-
  assert(candidate(N)),
  candidates2(L).
  
euclidean2(Trip, L) :-
  plan_to_atom(Trip, T),
  my_concat_atom(['update current := TestTrips morange[', T, '; ', T, '] 
    extract[Trip]'], '', Query),
  secondo(Query),
  secondo('update pos := makepoint(distanceAvg(p1, current), 
    distanceAvg(p2, current))'),
  secondo('query V mfeed replaceAttr[DEuc: distance(.P, pos)] 
    sortby[DEuc] project[N, DEuc] consume', [_, L]),
  retractall(candidate(_)),
  candidates2(L).
  
  
  
euclidean3(Trip, L) :-
  plan_to_atom(Trip, T),
  my_concat_atom(['update current := TestTrips morange[', T, '; ', T, '] 
    extract[Trip]'], '', Query),
  secondo(Query),
  secondo('{update d1 := distanceAvg(p1, current) | 
    update d2 := distanceAvg(p2, current) |
    update d3 := distanceAvg(p3, current)}'),
  secondo('query V3 mfeed replaceAttr[DEuc: 
    sqrt(pow(.D1 - d1, 2)+ pow(.D2 - d2, 2) + pow(.D3 - d3, 2))]
    sortby[DEuc] project[N, DEuc] consume', [_, L]),
  retractall(candidate(_)),
  candidates2(L).
  
  
% centers :- load centers as Prolog facts
:- dynamic center/5.

centers :-
  secondo('query Centers', [_, Values]),
  retractall(center(_, _, _, _, _)),
  centers2(Values).
  
centers2([]).

centers2([ [N, TripId, Trip, MaxDist, Size] | Values]) :-
  assert(center(N, TripId, Trip, MaxDist, Size)),
  centers2(Values).
  

  

  
% distances :- establish all distances between centers as prolog facts.
:- dynamic distM/3.
  
distances :-
  secondo('query Distances', [_, Values]),
  retractall(distM(_, _, _)),
  distances2(Values).
  
distances2([]).

distances2([ [N1, N2, D] | Values]) :-
  assert(distM(N1, N2, D)),
  distances2(Values).
  
% evalDistance(Center, Trip, Distance) :-  
%  
% Evaluate in Secondo the distance between a center and a trip. 
% Centers and trips must be stored in main memory relations as follows.
  
% let 'Centers = Centers1 feed moconsume[N]'.
% let 'TestTrips = TestTrips10CM500 feed moconsume[Ind]'

evalDistance(Center, Trip, Dist) :-
  plan_to_atom(Center, C),
  plan_to_atom(Trip, T),
  my_concat_atom(['query distanceAvg(Centers morange[', C, '; ', C, 
    '] extract[Trip], TestTrips morange[', T, '; ', T, '] extract[Trip])'], 
    '', Query),
  secondo(Query, [_, Dist]),
  assert(distCT(Center, Trip, Dist)).
 
 
% init :- initialize the sets of centers, trips, and distances. Pivot elements 
% p1 and p2 later to be replaced by the selected pivots, vector V will be 
% updated as well.

:- dynamic nCenters/1.

reset :- delete 'p1', delete 'p2', delete 'p3', delete 'current', delete 'pos', 
delete 'd1', delete 'd2', delete 'd3'.
 
init :-
  let 'Centers = Centers85F feed moconsume[N]',
  let 'Distances = Distances85 feed mconsume',
  let 'TestTrips = TestTrips2389 feed moconsume[Ind]',
  let 'p1 = Centers mfeed filter[.N = 1] extract[Trip]',
  let 'p2 = Centers mfeed filter[.N = 2] extract[Trip]',
  let 'p3 = Centers mfeed filter[.N = 3] extract[Trip]',
  let 'd1 = 0.0',
  let 'd2 = 0.0',
  let 'd3 = 0.0',
  let 'V = Centers mfeed projectextend[N; Dx: distanceAvg(.Trip, p1), 
    Dy: distanceAvg(.Trip, p2)] 
    extend[P: makepoint(.Dx, .Dy)] extend[DEuc: 0.0]
    mconsume',
  let 'V3 = Centers mfeed projectextend[N; D1: distanceAvg(.Trip, p1), 
    D2: distanceAvg(.Trip, p2), D3: distanceAvg(.Trip, p3)] 
    extend[DEuc: 0.0]
    mconsume',
  let 'current = p1',
  let 'pos = makepoint(distanceAvg(p1, current), distanceAvg(p2, current))',
  retractall(nCenters(_)),
  secondo('query Centers count', [_, C]),
  assert(nCenters(C)),
  centers,
  distances.
  
init2 :-
  retractall(dmin(_, _)),
  assert(dmin(0, 1000000.0)),
  nCenters(N),
  retractall(candidate(_)),
  candidates(1, N).
  
  
% countCands :- count the remaining candidates.

countCands(N) :-
  findall(U, candidate(U), L),
  length(L, N).
  
% oneCenter(Trip, PruningMethod) :- evaluate the distance of a trip to a 
% candidate center and prune candidate centers. Maintain also the minimal 
% distance of this trip to a center in predicate dmin. Possible pruning methods 
% are simple and mindist. Simple uses condition A > 2 D; mindist uses condition
% A > D + Dmin or A < D - Dmin.

:- dynamic dmin/2.
  
oneCenter(Trip, PruningMethod) :-
  candidate(N),
  evalDistance(N, Trip, D),
  retract(candidate(N)),
  updateDmin(N, D),
  !,
  write_list(['N = ', N, '; D = ', D]), nl,
  \+ prune(N, D, PruningMethod),
  countCands(M),
  write_list(['Remaining candidates: ', M]), nl, nl
  .
  
updateDmin(N, D) :-
  dmin(_, Dmin),
  D < Dmin,
  retractall(dmin(_, _)),
  assert(dmin(N, D)),
  !.
  
updateDmin(_, _).


prune(N, D, simple) :-
  distM(N, X, A),
  A > 2 * D,
  retract(candidate(X)),
  fail.
  
prune(N, D, mindist) :-
  candidate(X),
  distM(N, X, A),
  dmin(_, Dmin),
  Dplus is D + Dmin,
  Dminus is D - Dmin,
  ( A > Dplus ; A < Dminus ),
  % write_list(['X = ', X, '; A = ', A, '; Dplus = ', Dplus, '; Dminus = ', 
  % Dminus]), nl,
  retract(candidate(X)),
  fail.
  
  
% oneTrip(Trip, Count, Method, PruningMethod) :- determine the closest center 
% for one trip and the number 
% of rounds (distance evaluations) needed for that. Methods are basic, pivot2, 
% and pivot3.

oneTrip(Trip, Count, Method, PruningMethod) :-
  retractall(candidate(_)),
  assert(dmin(0, 1000000.0)),
  ( Method = basic 
    ->  (nCenters(C), candidates(1, C));
    Method = pivot2 -> euclidean2(Trip, _);
    euclidean3(Trip, _)
  ),
  oneTrip2(Trip, Count, PruningMethod),
  write_list(['Trip ', Trip, ' used ', Count, ' rounds.']), nl,
  dmin(Center, Dmin),
  write_list(['Closest center is ', Center, ' with distance ', Dmin, '.']), 
  nl, nl, nl.
  
  
 
  
  
oneTrip2(_, 0, _) :- 
  countCands(0),
  !.

oneTrip2(Trip, Count, PruningMethod) :- 
  oneCenter(Trip, PruningMethod),
  oneTrip2(Trip, Count2, PruningMethod),
  Count is Count2 + 1.

% allTrips(Start, End, Rounds, Method, PruningMethod) :- assign trips 
% numbered from Start to 
% End to centers, use Method basic or pivot 
% and count the number of rounds needed. Pruning methods are simple and mindist.
  
allTrips(Start, End, 0, _, _) :-
  Start > End,
  !.

allTrips(Start, End, Rounds, Method, PruningMethod) :-
  oneTrip(Start, Rounds1, Method, PruningMethod), 
  Start2 is Start + 1,
  allTrips(Start2, End, Rounds2, Method, PruningMethod),
  Rounds is Rounds1 + Rounds2.
  
 
 
% Range Queries

% range(Trip, Radius, NCenters, Centers, Count, Method, 
%   PruningMethod) :-
% find the centers that need to be considered in a range query with Radius. 
% Return the number of such centers in NCenters and the center identifiers 
% in the list Centers.

range(Trip, Radius, CountClosest, CountTotal, NCenters, Centers, Method, 
    PruningMethod) :-
  retractall(distCT(_, _, _)),
  oneTrip(Trip, CountClosest, Method, PruningMethod),
  addCenters(Trip, Radius, Count2, NCenters, Centers),
  CountTotal is CountClosest + Count2.
  
addCenters(Trip, Radius, Count, NCenters, Centers) :-
  retractall(rangeCenter(_, _, _)),
  retractall(evalCnt(_)),
  \+ addCenter(Trip, Radius),
  findall([X, A, Kind], rangeCenter(X, A, Kind), Centers),
  length(Centers, NCenters),
  findall(U, evalCnt(U), L2),
  length(L2, Count).

:- dynamic rangeCenter/3.
:- dynamic evalCnt/1.

addCenter(Trip, Radius) :-
  dmin(Nnq, Dmin),
  distM(Nnq, X, A),
  center(X, _, _, _, _),
  A < 2 * (Dmin + Radius),
  ( (A < 2 * Radius
  	% , A < Radius + Dmin + MaxDist
  	)	% new: maxDist pruning
      -> assert(rangeCenter(X, A, dist_nnq)) ;
    (dist(X, Trip, D), D < Dmin + 2 * Radius
    	% , D < Radius + MaxDist
    	) 		% new: maxDist pruning
      -> assert(rangeCenter(X, D, dist_q));
    true
  ),
  fail.

dist(Center, Trip, D) :-
  distCT(Center, Trip, D),
  !.
  
dist(Center, Trip, D) :-
  evalDistance(Center, Trip, D), 
  assert(evalCnt(1)),
  write_list(['N = ', Center, '; D = ', D]), nl.


  
checkDistances(Trip, Bound) :-
  plan_to_atom(Trip, T),
  plan_to_atom(Bound, B),
  my_concat_atom(['query TestTrips morange[', T, '; ', T, 
  '] extract[Trip] within[fun (x: ANY)', 
  ' Centers mfeed projectextend[N; D: distanceAvg(.Trip, x)] filter[.D < ', B, 
  '] consume]'], '', 
   Query),
   secondo(Query).
   
  
allRange(Start, End, _, 0, 0, 0, _, _) :-
  Start > End,
  !.

allRange(Start, End, Radius, CountClosest, CountTotal, NCenters, Method, 
    PruningMethod) :-
  range(Start, Radius, CountClosest1, CountTotal1, NCenters1, _, Method, 
    PruningMethod),
  Start2 is Start + 1,
  allRange(Start2, End, Radius, CountClosest2, CountTotal2, NCenters2, 
     Method, PruningMethod),
  CountClosest is CountClosest1 + CountClosest2,
  CountTotal is CountTotal1 + CountTotal2,
  NCenters is NCenters1 + NCenters2.
  


 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
%%% Using pivots

% twoPivots(Strategy) :- determine two pivot centers according to 
% a given strategy. Possible strategies are maxdist, ... The trips of these 
% centers are assigned to Secondo objects p1 and p2, respectively. The
% vector V is updated
  
twoPivots(maxdist) :-
  secondo('query Distances mfeed kbiggest[1; D] consume', 
    [_, [[Pivot1, Pivot2, _]]]),
  plan_to_atom(Pivot1, P1),
  plan_to_atom(Pivot2, P2),
  my_concat_atom(['update p1 := Centers mfeed filter[.N = ', P1, 
    '] extract[Trip]'], '', Query1),
  secondo(Query1),
  my_concat_atom(['update p2 := Centers mfeed filter[.N = ', P2, 
    '] extract[Trip]'], '', Query2),
  secondo(Query2),
  update 'V := Centers mfeed projectextend[N; Dx: distanceAvg(.Trip, p1), 
  Dy: distanceAvg(.Trip, p2)] 
  extend[P: makepoint(.Dx, .Dy)] extend[DEuc: 0.0]
  mconsume'.
 



