/*
Test implementation of N-tree (one node at level 1 only)

*/
% Steps for starting:
% open database newyorknew
% [ntree2]
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
  assert(distance(Center, Dist)).
 
 
% init :- initialize the sets of centers, trips, and distances. 

:- dynamic nCenters/1.

init :-
  let 'Centers = Centers15F feed moconsume[N]',
  let 'Distances = Distances15 feed mconsume',
  let 'TestTrips = TestTrips2389 feed moconsume[Ind]',
  retractall(nCenters(_)),
  secondo('query Centers count', [_, C]),
  assert(nCenters(C)),
  centers,
  distances.
  

  
% countCands :- count the remaining candidates.

countCands(N) :-
  findall(U, candidate(U), L),
  length(L, N).

countDistEvals(N) :-
  findall([C, U], distance(C, U), L),
  length(L, N).
  
countResults(N) :-
  findall([C, U], returned(C, U, _), L),
  length(L, N1),
  findall([X, Y], searched(X, Y, _), L2),
  length(L2, N2),
  N is N1 + N2.
  

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
   
   
:- dynamic dmin/2.
   
updateDmin(N, D) :-
  dmin(_, Dmin),
  D < Dmin,
  retractall(dmin(_, _)),
  assert(dmin(N, D)),
  !.
  
updateDmin(_, _).
   
  

% rangeSearch2(Q, R) :- perform a range query with query object Q 
% (a trip number) and Radius R. 

% to be called with an empty list Res
:- dynamic returned/3.
:- dynamic searched/3.
:- dynamic distance/2.


rangeSearch2(Q, R) :- 
  assert(dmin(10000, 1000000.0)),
  \+ rangeSearch2a(Q, R),
  countDistEvals(N),
  \+ pruneSearched(R),
  countResults(M),
  write_list(['There have been ', N, ' distance evaluations and ', 
    M, ' results.']).

rangeSearch2a(Q, R) :-
  nCenters(N), N1 is N - 1,
  candidates(0, N1),
  retractall(returned(_, _, _)),
  retractall(searched(_, _, _)),
  retractall(distance(_, _)),
  candidate(CenterI),
  retract(candidate(CenterI)),
  evalDistance(CenterI, Q, Dist),
  updateDmin(CenterI, Dist),
  dmin(_, Dmin),
    write('Dmin = '), write(Dmin), nl,
  \+ prune2(CenterI, Dist, R),
    countCands(M),
    write_list(['Remaining candidates: ', M]), nl, nl,
  center(CenterI, _, _, MaxDist, Size),
      write_list(['CenterI = ', CenterI, '; Distance = ', Dist, 
        '; MaxDist = ', MaxDist]), nl,
      Sum is Dist + MaxDist,
      Diff is Dist - MaxDist,
      write_list(['Sum = ', Sum, 
        '; Difference = ', Diff]), nl,
  ( R > Dist + MaxDist
      -> ( write('returned'), nl, nl, assert(returned(CenterI, Dist, Size)));
    R > Dist - MaxDist 
      -> ( write('searched'), nl, nl, assert(searched(CenterI, Dist, Size)));
      write('ignored'), nl, nl, nl ),
  fail.
  
  
  
prune2(CenterI, Dist, R) :-
  candidate(CenterJ),
  distM(CenterI, CenterJ, DistIJ),
  center(CenterJ, _, _, MaxDist, Size),
      write_list(['CenterJ = ', CenterJ]), nl, 
  ( R > Dist + DistIJ + MaxDist
      -> ( write('reported'), nl, nl, assert(returned(CenterJ, Dist, Size)), 
           retract(candidate(CenterJ))) ;
    ( abs(Dist - DistIJ, Diff),                        
      R < Diff - MaxDist ) 
      -> ( write('pruned'), nl, nl, retract(candidate(CenterJ))) ;
      true ),
  fail.
      
pruneSearched(Radius) :-
  dmin(_, Dmin),
  searched(Center, Dist, Size),
  Dist > Dmin + 2 * Radius,
  retract(searched(Center, Dist, Size)),
    write('Pruned center '), write(Center), nl,
  fail.
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  




