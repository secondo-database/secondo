/*
//paragraph [10] title:    [{\Large \bf ]    [}]
//characters [1] formula:     [$]     [$]
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[**] [$**$]
//[toc] [\tableofcontents]
//[=>] [\verb+=>+]
//[:Section Translation] [\label{sec:translation}]
//[Section Translation] [Section~\ref{sec:translation}]
//[:Section 4.1.1] [\label{sec:4.1.1}]
//[Section 4.1.1] [Section~\ref{sec:4.1.1}]
//[Figure pog1] [Figure~\ref{fig:pog1.eps}]
//[Figure pog2] [Figure~\ref{fig:pog2.eps}]
//[newpage] [\newpage]

[10] Query Optimization for Distributed Query Processing

Fapra group 2015/2016: 

  * Thomas Fischer

  * Korinna Kurrer

  * Andreas Obergrusberger

  * Thomas Peredery

  * Luciana Plocki

[toc]

[newpage]

1 Introduction

This file contains additions to ~optimizerNewProperties~ to support query optimization for distributed processing, using the ~Distributed2Algebra~. 


2 The Target Language

In the target language, we use the following additional operators:

----
     dloop     darray(X) x string x  (X->Y) -> darray(Y)
           
                Performs a function on each element of a darray instance.The 
                string argument specifies the name of the result. If the 
                name is undefined or an empty string, a name is generated 
                automatically.

   dloop2     darray(X) x darray(Y) x string x (fun : X x Y -> Z) -> darray(Z)
           
               Performs a function on the elements of two darray instances. 
               The string argument specifies the name of the resulting 
               darray. If the string is undefined or empty, a name is 
               generated automatically.
         
    dmap    d[f]array x string x fun -> d[f]array
           
              Performs a function on a distributed file array. If the 
              string argument is empty or undefined, a name for the result 
              is chosen automatically. If not, the string specifies the 
              name. The result is of type dfarray if the function produces 
              a tuple stream or a relationi; otherwise the result is a 
              darray.

  dmap2    d[f]array x d[f]array x string x fun -> d[f]array
           
             Joins the slots of two distributed arrays.

 partition  d[f]array(rel(tuple)) x string x (tuple->int) x int-> dfmatrix
           
             Redistributes the contents of a dfarray value. The new slot 
             contents are kept on the worker where the values were stored 
             before redistributing them. The last argument (int) 
             determines the number of slots of the redistribution. If 
             this value is smaller or equal to zero, the number of slots 
             is overtaken from the array argument.

partitionF  d[f]array(rel(X)) x string x ([fs]rel(X)->stream(Y)) x (Y ->
            int) x int -> dfmatrix(rel(Y))
           
              Repartitions a distributed [file] array. Before repartition,
              a function is applied to the slots.

 collect2  dfmatrix x string x int -> dfarray
            
            Collects the slots of a matrix into a  dfarray. The string 
            is the name of the resulting array, the int value specified 
            a port for file transfer. The port value can be any port 
            usable on all workers. A corresponding file transfer server 
            is started automatically.
          
  areduce  dfmatrix(rel(t)) x string x (fsrel(t)->Y) x int -> d[f]array(Y)
            
            Performs a function on the distributed slots of an array. 
            The task distribution is dynamically, meaning that a fast 
            worker will handle more slots than a slower one. The result 
            type depends on the result of the function. For a relation 
            or a tuple stream, a dfarray will be created. For other non-
            stream results, a darray is the resulting type.
 
dsummarize    darray(DATA) -> stream(DATA) , d[f]array(rel(X)) -> stream(X)
           
               Produces a stream of the darray elements.
          
  getValue   {darray(T),dfarray(T)} -> array(T)
           
               Converts a distributed array into a normal one.

     tie     ((array t) (map t t t)) -> t
          
                Calculates the "value" of an array evaluating the elements
                of the array with a given function from left to right.
----


3 Replication

To consider distributed queries with predicates containing non-relation
 objects, it is necessary to replicate the objects to the 
 involved workers. 

For now we assume that every found object is contained in the distributed
 part of the query (function of dmap or dmap2).

A possible later extension is to examine the distributed relations and 
 to share the objects only to workers containing parts of those relations.

*/

:-
  dynamic(replicatedObject/1).

%distributed query without objects
replicateObjects(QueryPart, QueryPart) :-
  findall(X,replicatedObject(X), ObjectList),
  length(ObjectList,0),!.

%distributed query using objects in predicate
replicateObjects(QueryPart, Result) :-
  findall(X,replicatedObject(X), ObjectList),
  length(ObjectList,Length),
  Length >0,
  maplist(createSharedClause,ObjectList,CommandList),
  append(CommandList,[QueryPart], Result).

createSharedClause(Obj, SharedCommand) :-
  atom_concat('share("',Obj,StrObj),
  atom_concat(StrObj,'",TRUE)',SharedCommand).


/*
4 Translation of Plans  

*/

plan_to_atom_string(X, Result) :-
  isDistributedQuery,
  retractall(replicatedObject(_)),
  plan_to_atom(X,QueryPart),
  replicateObjects(QueryPart, Result),
  !.
  
plan_to_atom_string(X, Result) :-
  not(isDistributedQuery),
  plan_to_atom(X,Result),
  !.

plan_to_atomD(obj(Object,_,u), Result) :-
  isDistributedQuery,  
  upper(Object, UpperObject),
  atom_concat(UpperObject, ' ', Result),
  assertOnce(replicatedObject(UpperObject)),
  !.

plan_to_atomD(obj(Object,_,l), Result) :-
  isDistributedQuery,
  atom_concat(Object, ' ', Result),
  assertOnce(replicatedObject(Object)),
  !.

  
plan_to_atomD(obj(Object,_,u), Result) :-
  upper(Object, UpperObject),
  atom_concat(UpperObject, ' ', Result),
  !.

plan_to_atomD(obj(Object,_,l), Result) :-
  atom_concat(Object, ' ', Result),
  !.
  
plan_to_atomD(dot, Result) :-
  atom_concat('.', ' ', Result),
  !.





plan_to_atomD(dloop2(PreArg1, PreArg2, PostArg1, PostArg2), Result) :-
plan_to_atom(PreArg1, PreArg1Atom),
plan_to_atom(PreArg2, PreArg2Atom),
plan_to_atom(PostArg1, PostArg1Atom),
plan_to_atom(PostArg2, PostArg2Atom),
concat_atom(
  [PreArg1Atom,
  PreArg2Atom,
  'dloop2[',
  PostArg1Atom, ', ',
  PostArg2Atom, ']'], '', Result),
!.



plan_to_atomD(our_attrname(attr(Name, Arg, Case)), Result) :-
  plan_to_atomD(our_a(Name, Arg, Case), Result).

plan_to_atomD(our_a(_:B, _, _), Result) :-
  upper(B, B2),
  concat_atom(['..', B2], Result),
  !.

plan_to_atomD(our_a(X, _, _), Result) :-
  upper(X, X2),
  concat_atom(['..', X2], Result),
  !.

plan_to_atomD(simple_attrname(attr(Name, Arg, Case)), Result) :-
  plan_to_atomD(simple_a(Name, Arg, Case), Result), !.

plan_to_atomD(simple_a(_:B, _, _), B2) :-
  upper(B, B2),
  !.

plan_to_atomD(simple_a(X, _, _), X2) :-
  upper(X, X2),
  !.

plan_to_atomD(extendstream(A, B, C), Plan) :-
  plan_to_atom(A, PlanA),
  plan_to_atom(B, PlanB),
  plan_to_atom(C, PlanC),
  concat_atom([PlanA, ' ', 'extendstream(',
    PlanB, ': ', PlanC, ')'], Plan).




/*
5 Translation Rules

5.1 Translation of Arguments

Treat translation into distributed arguments. The properties we use are...
  
  *  ~distribution~ (~DistributionType~, ~DistributionAttribute~, ~DistributionParameter~):

    ~DistributionType~ is ~share~, ~spatial~, ~modulo~, ~function~ or ~random~.

    ~DistributionAttribute~ is the attribute of the relation used to determine
  on which partition(s) to put a given tuple (in theory this could also be a list).

    ~DistributionParameter~ is the parameter used for the distribution (like grid or
  function object / operator).

  * ~distributedobjecttype~(~Type~) (~Type~ is ~darray~, ~dfarray~ or ~dfmatrix~).

  * ~disjointpartitioning~ signals that, if we treat a partition as the multi set
  of the tuples it contains, the union of all partitions is the original relation
  (put differently, in as far as duplicates exist, they have been present in the 
  original relation). 
  
    Since some Secondo plans eliminate duplicates anyways, they can do without their
  arguments having this property (e.g. spatial join).
  
*/

% Translate into object found in SEC2DISTRIBUTED.
distributedarg(N) translatesD [ObjName, X] :-
  X =[distribution(DistType, DCDistAttr, DistParam),
  distributedobjecttype(DistObjType),disjointpartitioning],
  argument(N, Rel),
  Rel = rel(Name, _, _),
  distributedRels(rel(Name, _, _), ObjName, DistObjType, 
    DistType, DistAttr, DistParam),
  not(DistType = spatial),
  downcase_atom(DistAttr, DCDistAttr).

% Spatial partitioning with filtering on original attribute
% does not in general yield disjoint partitions
distributedarg(N) translatesD [ObjName, 
  [distribution(DistType, DCDistAttr, DistParam),
  distributedobjecttype(DistObjType)]] :-
  argument(N, Rel),
  Rel = rel(Name, _, _),
  distributedRels(rel(Name, _, _), ObjName, DistObjType, 
    DistType, DistAttr, DistParam),
  DistType = spatial,
  downcase_atom(DistAttr, DCDistAttr).

% Filter spatially distributed argument on attribute original.
distributedarg(N) translatesD [Plan, 
  [distribution(spatial, DCDistAttr, DistParam),
  distributedobjecttype(DistObjType), disjointpartitioning]] :-
  argument(N, Rel),
  Rel = rel(Name, _, _),
  distributedRels(rel(Name, _, _), ObjName, DistObjType, 
    spatial, DistAttr, DistParam),
  downcase_atom(DistAttr, DCDistAttr),
  Plan = dmap(ObjName, " ", filter(feed(rel(., *, u)), attr(original, l, u))).

/*
Redistributed argument relation to be spatially distributed using the provided attribute. The distribution type must be spatial and the attribute must be provided as a ground term. The grid may be provided to be used for the distribution. If it is not provided we fall back to using the grid object called grid. You need to have this in your database. Yields a dfarray or a dfmatrix.

*/

distributedarg(N) translatesD [Plan, [distribution(DistType, DistAttr, Grid),
  distributedobjecttype(DistObjType)]] :-
  % only use this in one direction. Might be generalized in the future.
  ground(DistAttr),
  ground(DistType),
  % if we do not have a grid specified, use the grid-object
  (ground(Grid) -> true; Grid = grid),
  DistType = spatial,
  argument(N, Rel),
  Rel = rel(Name, _, _),
  distributedRels(rel(Name, _, _), ObjName, _, OriginalDistType, _, _),
  % cannot redistribute replicated relations
  not(OriginalDistType = share),
  spelled(Name:DistAttr, AttrTerm),
  InnerPlan = partitionF(ObjName, " ", extendstream(feed(rel('.', *, u)), 
    attrname(attr(cell, *, u)), cellnumber(bbox(AttrTerm), Grid)),
    attr('.Cell', *, u), 0), %there should be another option to add the 2nd dot
  % collect into dfarray or simply be content with the dfmatrix
  (DistObjType = dfarray,
    Plan = collect2(InnerPlan, " ", 1238);
    DistObjType = dfmatrix,
    Plan = InnerPlan).

/* 

5.2 Translation of Selections that Concern Distributed Relations

*/

% Commutativity of intersects.
distributedselect(ObjName, 
  pr(Val intersects attr(Attr, Arg, Case), Rel)) translatesD X :-
  distributedselect(ObjName, pr(attr(Attr, Arg, Case) intersects Val, Rel)) 
    translatesD X.

/*
5.2.1 Using a Spatial Index

*/

% Use spatial index for an intersection predicate.
distributedselect(arg(N), Pred)
  translatesD [dmap2(IndexObj, RelObj, " ", 
    filter(filter(Intersection, InnerPred), attr(original, l, u)), 1238),
    [distributedobjecttype(dfarray), disjointpartitioning]] :-
    argument(N, Rel),
    Pred = pr(Attr intersects Val, rel(_, Var, _)),
    Pred = pr(InnerPred, _),
    % We need a materialized argument relation to use the index
    distributedRels(Rel, RelObj, _, _, _),
    RelObj = rel(RelObjName, _, _),
    % Lookup an rtree index for the relation + attribute
    downcase_atom(RelObjName, DCRelObjName),
    attrnameDCAtom(Attr, DCAttr),
    distributedIndex(DCRelObjName, DCAttr, rtree, DCIndexObjName),
    % Check the database object for the correct spelling
    spelledObj(DCIndexObjName, IndexObjName,_, Case),
    IndexObj = rel(IndexObjName, *, Case),
    IndParam = rel('.', *, u),
    RelParam = rel('..', *, u),
    renameStream(windowintersects(IndParam, RelParam, Val),
      Var, Intersection).

/*
5.2.2 Using a Standard Index (B-Tree)

*/

% Use btree index for a starts predicate.
distributedselect(arg(N), pr(Attr starts Val, rel(_, Var, _)))
  translatesD [dmap2(IndexObj, RelObj, " ", 
    Range, 1238), [distributedobjecttype(dfarray), disjointpartitioning]] :-
    argument(N, Rel),
    distributedRels(Rel, RelObj, _, _, _),
    RelObj = rel(RelObjName, _, _),
    downcase_atom(RelObjName, DCRelObjName),
    attrnameDCAtom(Attr, DCAttr),
    % Lookup a btree index for the relation + attribute
    distributedIndex(DCRelObjName, DCAttr, btree, DCIndexObjName),
    spelledObj(DCIndexObjName, IndexObjName,_, Case),
    IndexObj = rel(IndexObjName, *, Case),
    IndParam = rel('.', *, u),
    RelParam = rel('..', *, u),
    renameStream(range(IndParam, RelParam, Val, increment(Val)),
      Var, Range).

/*
5.2.3 Selection Without Index

*/

% Generic case.
distributedselect(Arg, pr(Cond, rel(_,Var,_))) translatesD
  [dmap(ArgS," ", filter(Param,Cond)), P] :-
  Arg  => [ArgS, P],
  % we accept darrays and dfarrays
  (member(distributedobjecttype(dfarray), P) ;
    member(distributedobjecttype(darray), P)),
  % partitions of the argument relations need to disjoint
  member(disjointpartitioning, P),
  % rename if needed
  feedRenameRelation(rel('.',*, u), Var, Param).


/*
5.3 Distributed Join

5.3.1 Distributed Spatial Join

*/

% Translate a distributed spatial join with an intersection predicate.
distributedjoin(Arg1, Arg2, Pred)
translatesD [SecondoPlan, [DistAttr1, distributedobjecttype(dfarray), 
disjointpartitioning]]:-
  Pred = pr(Attr1 intersects Attr2, rel(_, Rel1Var, _), rel(_, Rel2Var, _)),
  isOfFirst(Attr1, Rel1, Rel2),
  isOfSecond(Attr2, Rel1, Rel2),
  attrnameDCAtom(Attr1, Attr1Name),
  attrnameDCAtom(Attr2, Attr2Name),
  % allow using replicated + any distribution or both distributed by
  % join predicate
  ((DistAttr1 = distribution(_, _, _),
    DistAttr2 = distribution(share, _, _));
  (DistAttr1 = distribution(spatial, Attr2Name, GridObj),
    DistAttr2 = distribution(spatial, Attr1Name, GridObj))),
  Arg1 => [ObjName1, [DistAttr1| Props1]],
  Arg2 => [ObjName2, [DistAttr2| Props2]],
  % rename the parameter relations if needed
  feedRenameRelation(param1, Rel1Var, Param1Plan),
  feedRenameRelation(param2, Rel2Var, Param2Plan),
  % rename the cell attribute if needed
  renamedRelAttr(attr(cell, 1, u), Rel1Var, CellAttr1),
  renamedRelAttr(attr(cell, 2, u), Rel2Var, CellAttr2),
  Scheme =
    filter(
        filter(
            filter( 
                itSpatialJoin(
                    Param1Plan,
                    Param2Plan, 
                    attrname(Attr1),
                    attrname(Attr2)
                    ),
                CellAttr1 = CellAttr2
                ),
            gridintersects(
                GridObj, 
                bbox(Attr1), 
                bbox(Attr2), 
                CellAttr1
                )
            ),
        Attr1 intersects Attr2
        ),
  % We have the actual query now. Distribute it to the workers.
  distributedquery([ObjName1, [DistAttr1| Props1]], 
    [ObjName2, [DistAttr2| Props2]], Scheme)
    translatesD SecondoPlan.

/*
5.3.2 Handling Distribution Types (Replicated vs. Partitioned) and Distributed Object Types (~darray~ vs. ~dfarray~)

----    distributedquery(Arg1, Arg2, QueryScheme)
----

Distribute the query given by ~QueryScheme~ to the workers. The scheme has
  the place holders ~param1~ and ~param2~ for its argument. The actual arguments
  are given in ~Arg1~ and ~Arg2~ as a pair of a plan and a property list. 
  Several cases might arise depening on ~Arg1~'s and ~Arg2~'s distribution type (replicated vs partitioned) and their distributed object type (d(f)array vs dfmatrix).

*/

% Arg1 replicated, Arg2 partitioned, Arg2 is a d(f)array
distributedquery([Arg1S, P1], [Arg2S, P2], QueryScheme) translatesD Query :-
  not(isPartitioned([Arg1S, P1])),
  isPartitioned([Arg2S, P2]),
  not(isDfmatrix([Arg2S, P2])),
  substituteSubterm(param2, rel('.', *, u), QueryScheme, QueryScheme1),
  substituteSubterm(param1, Arg2S, QueryScheme1, QueryScheme2),
  Query = dmap(Arg2S, " ", QueryScheme2), !.

% Arg2 replicated, Arg1 partitioned, Arg1 is a d(f)array
distributedquery([Arg1S, P1], [Arg2S, P2], QueryScheme) translatesD Query :-
  isPartitioned([Arg1S, P1]),
  not(isPartitioned([Arg2S, P2])),
  not(isDfmatrix([Arg1S, P1])),
  substituteSubterm(param1, rel('.', *, u), QueryScheme, QueryScheme1),
  substituteSubterm(param2, Arg2S, QueryScheme1, QueryScheme2),
  Query = dmap(Arg1S, " ", QueryScheme2), !.

% Arg1 partitioned, Arg2 partitioned, both are d(f)arrays
distributedquery([Arg1S, P1], [Arg2S, P2], QueryScheme) translatesD Query :-
  isPartitioned([Arg1S, P1]),
  isPartitioned([Arg2S, P2]),
  not(isDfmatrix([Arg2S, P2])),
  not(isDfmatrix([Arg1S, P1])),
  substituteSubterm(param1, rel('.', *, u), QueryScheme, QueryScheme1),
  substituteSubterm(param2, rel('..', *, u), QueryScheme1, QueryScheme2),
  Query = dmap2(Arg1S, Arg2S, " ", QueryScheme2, 1238), !.

% Arg1 partitioned, Arg2 partitioned, both dfmatrices
distributedquery([Arg1S, P1], [Arg2S, P2], QueryScheme) translatesD Query :-
  isPartitioned([Arg1S, P1]),
  isPartitioned([Arg2S, P2]),
  isDfmatrix([Arg2S, P2]),
  isDfmatrix([Arg1S, P1]),
  substituteSubterm(param1, rel('.', *, u), QueryScheme, QueryScheme1),
  substituteSubterm(param2, rel('..', *, u), QueryScheme1, QueryScheme2),
  Query = areduce2(Arg1S, Arg2S, "", QueryScheme2, 1238), !.

% Arg1 replicated, Arg2 replicated
distributedquery([Arg1S, P1], [Arg2S, P2], _) translatesD _ :-
  not(isPartitioned([Arg1S, P1])),
  not(isPartitioned([Arg2S, P2])),
  write('A potential plan edge could not be generated because '),
  write('queries with two replicated arguments '),
  write('cannot be formulated using DistributedAlgebra as of now.\n'),
  fail.

/*
5.3.3 Equijoin

*/

%Equijoin    
distributedjoin(ObjName1, ObjName2, pr(attr(X1,X2,X3)=attr(Y1,Y2,Y3),
                Rel1, Rel2)) 
translatesD [SecondoPlan, [none]] :-
 X=attr(X1,X2,X3),
 Y=attr(Y1,Y2,Y3),
 Rel1 = rel(_, _, _),
 Rel2 = rel(_, _, _),
 isOfFirst(_, X, Y), 
 isOfSecond(_, X, Y),
 buildSecondoPlan(ObjName1, ObjName2, pr(X=Y, Rel1, Rel2), 
		  SecondoPlan, false). 
           
%Standard Join           
distributedjoin(ObjName1, ObjName2, pr(Pred,Rel1, Rel2)) 
translatesD [SecondoPlan, [none]] :-
 Rel1 = rel(_, _, _),
 Rel2 = rel(_, _, _),
 buildStdSecondoPlan(ObjName1, ObjName2, pr(Pred, Rel1, Rel2),
		  SecondoPlan, false). 

/*
It is assumed that if "function" is specified in
the system relation "SEC2DISTRIBUTED", then a deterministic
function using the specified attribute was used.
The functions used for partitioning both used relations are assumed
to result in the same values if given the same attribute value. E.g.
both used the same hashvalue.

Equijoin Secondo Plan for both are partitioned by join attribute
 using modulo. 
 Modulo is the most efficient compared to the other options, 
 because we do not need to repartition and also there is no
 need to calculate the worker, on which a tuple is located, 
 the worker number is already the modulo value. Thus it is 
 slightly more efficient than any other function (i.e. hash).
 In case it is possible in the future to deploy different secondo plans
 to different workers (i.e. tell each worker which part of the shared
 relation it should use), having 2 replicated relations
 is the most efficient solution. 

*/

buildSecondoPlan(ObjName1, ObjName2, pr(X=Y, Rel1, Rel2), 
		 SecondoPlan, _):-		 
 plan_to_atom(simple_attrname(X), X2),
 plan_to_atom(simple_attrname(Y), Y2),
 distributedRels(_, ObjName1, _, 'modulo', X2),
 distributedRels(_, ObjName2, _, 'modulo', Y2),
 Rel1 = rel(_, Rel1Var, _),
 Rel2 = rel(_, Rel2Var, _),
 % rename the parameter relations of the dmapped plan if needed
 feedRenameRelation(rel('.', *, u), Rel1Var, Feed1),
 feedRenameRelation(rel('..', *, u), Rel2Var, Feed2),
 !,
 SecondoPlan = dmap2(ObjName1, ObjName2, " ",
               hashjoin(Feed1, Feed2,attrname(X), 
               attrname(Y), 999997), 1238).
               
%Equijoin Secondo Plan for both are partitioned by join attribute
%using a function
buildSecondoPlan(ObjName1, ObjName2, pr(X=Y, Rel1, Rel2), 
		 SecondoPlan, _):-		 
 plan_to_atom(simple_attrname(X), X2),
 plan_to_atom(simple_attrname(Y), Y2),
 distributedRels(_, ObjName1, _, 'function', X2),
 distributedRels(_, ObjName2, _, 'function', Y2),
 Rel1 = rel(_, Rel1Var, _),
 Rel2 = rel(_, Rel2Var, _),
 % rename the parameter relations of the dmapped plan if needed
 feedRenameRelation(rel('.', *, u), Rel1Var, Feed1),
 feedRenameRelation(rel('..', *, u), Rel2Var, Feed2),
 !,
 SecondoPlan = dmap2(ObjName1, ObjName2, " ",
               hashjoin(Feed1, Feed2,attrname(X), 
               attrname(Y), 999997), 1238). 

%Equijoin Secondo Plan for one replicated (relation) and
%one partitioned (darray/dfarray)
buildSecondoPlan(ObjName1, ObjName2, pr(X=Y, Rel1, Rel2), 
		 SecondoPlan, _):-
 distributedRels(_ ,ObjName1,_ ,'share',_ ),
 isPartitioned(ObjName2),
 Rel1 = rel(_, Rel1Var, _),
 Rel2 = rel(_, Rel2Var, _),
 % rename the parameter relations of the dmapped plan if needed
 feedRenameRelation(ObjName1, Rel1Var, Feed1),
 feedRenameRelation(rel('.', *, u), Rel2Var, Feed2),
  !,
 SecondoPlan = dmap(ObjName2, " ", 
 hashjoin(Feed1,
	  Feed2, 
          attrname(X), attrname(Y), 999997)).

%Commutativity for Equijoin & Standard Join
buildSecondoPlan(ObjName1, ObjName2, pr(Pred, Rel1, Rel2), 
	         SecondoPlan, false):-  
 buildSecondoPlan(ObjName2, ObjName1, pr(Pred, Rel1, Rel2),
		  SecondoPlan, true).
	  		  
	         	         
%Equijoin Secondo Plan for repartitioning 2 "wrongly"
%partitioned relations (darray/dfarray)
buildSecondoPlan(ObjName1, ObjName2, pr(X=Y, Rel1, Rel2), 
	         SecondoPlan, _):-	   
  isPartitioned(ObjName1),
  isPartitioned(ObjName2),
  Rel1 = rel(_, Rel1Var, _),
  Rel2 = rel(_, Rel2Var, _),
  % rename the parameter relations of the dmapped plan if needed
  feedRenameRelation(rel('.', *, u), Rel1Var, Feed1),
  feedRenameRelation(rel('..', *, u), Rel2Var, Feed2),
  !,	         
  SecondoPlan = dmap2(
	collect2( 
	   partitionF(ObjName1, "LeftPartOfJoin", feed(rel('.',*,u)), 
	   hashvalue(our_attrname(X), 999997), 0),
	   "L", 1238),
	collect2(
	   partitionF(ObjName2, "RightPartOfJoin", feed(rel('.',*,u)),
	   hashvalue(our_attrname(Y), 999997), 0),
	   "R", 1238),
	" ", 
	hashjoin(Feed1,
	      Feed2, 
	      attrname(X), attrname(Y), 999997), 
	      1238).	
	    
%Equijoin Secondo Plan for repartitioning 2 replicated rels
buildSecondoPlan(ObjName1, ObjName2, pr(attr(_,_,_)=attr(_,_,_), _, _), 
	         _, true):-	         	         
  distributedRels(_ ,ObjName1,_ ,'share',_ ),
  distributedRels(_, ObjName2, _,'share', _),
  !,
  write('Both relations are replicated, the query cannot be executed!'),
  false.

% Plan yields a dfmatrix
isDfmatrix([_, P]) :-
  member(distributedobjecttype(dfmatrix), P).

% Plan yields a partitioned distribution.
isPartitioned([_, P]):-
 is_list(P), !,(
 member(distribution('function', _, _), P);
 member(distribution('modulo', _, _), P);
 member(distribution('random', _, _), P);
 member(distribution('spatial', _, _), P)).                     

% Secondo object represents a partitioned distribution.
isPartitioned(ObjName):-
 distributedRels(_, ObjName,_ ,'function', _);
 distributedRels(_, ObjName,_ ,'modulo', _);
 distributedRels(_, ObjName,_ ,'random', _);
 distributedRels(_, ObjName,_ ,'spatial', _).

%Standard Join Secondo Plan (one replicated, one partitioned)	
buildStdSecondoPlan(ObjName1, ObjName2, pr(Pred, Rel1, Rel2), 
	         SecondoPlan, _):-
  (DistArgrel = ObjName2, ReplArgrel = ObjName1;
    DistArgrel = ObjName1, ReplArgrel = ObjName2),
  distributedRels(_, ReplArgrel, _ , 'share', _),
  isPartitioned(DistArgrel),
  Rel1 = rel(_, Rel1Var, _),
  Rel2 = rel(_, Rel2Var, _),
  % rename the parameter relations of the dmapped plan if needed
  feedRenameRelation(rel('.', *, u), Rel2Var, Feed2),
  feedRenameRelation(ReplArgrel, Rel1Var, Feed1),
  !,
  SecondoPlan = dmap(DistArgrel, " ", 
  filter(product(Feed2,Feed1), Pred)).
  
%Standard Join Secondo Plan, both are partitioned 
buildStdSecondoPlan(ObjName1, ObjName2, pr(_, _, _), 
	         _, true):-
  isPartitioned(ObjName1),
  isPartitioned(ObjName2),
  !,
  write('The joined relations are both partitioned and thus'), 
  write(' not distributed correctly for standard join.'),
  false.
  
%Standard Join Secondo Plan, if repartitioning is needed 
buildStdSecondoPlan(_, _, pr(_, _, _), _, true):-
  !,
  write('The joined relations are not distributed correctly '),
  write('for standard join.'),
  false.

  
/*
6 Cost Functions

*/

% Taken from standard optimizer.
costD(itSpatialJoin(X, Y, _, _), Sel, S, C) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, 1, SizeY, CostY),
  itSpatialJoinTC(A, B),
  S is SizeX * SizeY * Sel,
  C is CostX + CostY +
  A * (SizeX + SizeY) +
  B * S.

costD(windowintersects(_, Rel, _), Sel, Size, Cost) :-
  cost(Rel, 1, RelSize, _),
  windowintersectsTC(A),
  Size is Sel * RelSize,
  Cost is Size * A.
  
costD(hashvalue(_,_), _, 1, 0).

costD(dmap(Obj, _, InnerPlan), Sel, S, C) :-
  distributedRels(LocalMasterRel, Obj, _, _, _),
  substituteSubterm(rel('.', *, u), LocalMasterRel, InnerPlan, LocalInnerPlan),
  cost(LocalInnerPlan, Sel, S, InnerC),
  !,
  C is InnerC * S.

costD(dmap(Obj, _, InnerPlan), Sel, S, C) :-
  substituteSubterm(rel('.', *, u), Obj, InnerPlan, LocalInnerPlan),
  cost(LocalInnerPlan, Sel, S, InnerC),
  !,
  C is InnerC * S.

% if we cannot determine cost of first dmap-argument
costD(dmap(_, _, X), Sel, S, C) :-
  cost(X, 1, SizeX, CostX),
  dmapTC(A),
  S is SizeX * Sel,
  C is CostX + A * SizeX.

 costD(dmap2(_, RelObj, _, InnerPlan, _), Sel, S, C) :-
  distributedRels(LocalMasterRel, RelObj, _, _, _),
  substituteSubterm(rel('..', *, u), LocalMasterRel, 
    InnerPlan, LocalInnerPlan),
  dmap2TC(A),
  cost(LocalMasterRel, 1, Card, _),
  cost(LocalInnerPlan, Sel, _, InnerCost),
  !,
  S is Sel * Card,
  C is InnerCost + A * S.  

% we have two d/farray-objects as arguments
costD(dmap2(RelObj1, RelObj2, _, InnerPlan, _), Sel, _, C) :-
  distributedRels(LocalMasterRel1, RelObj1, _, _, _),
  distributedRels(LocalMasterRel2, RelObj2, _, _, _),
  substituteSubterm(rel('.', *, u), LocalMasterRel1, 
    InnerPlan, LocalInnerPlan1),
  substituteSubterm(rel('..', *, u), LocalMasterRel2, 
    LocalInnerPlan1, LocalInnerPlan),
  dmap2TC(A),
  cost(LocalMasterRel1, 1, Card1, _),
  cost(LocalMasterRel2, 1, Card2, _),
  cost(LocalInnerPlan, Sel, _, InnerCost),
  !,
  S1 is Sel * Card1,
  S2 is Sel * Card2,
  C is InnerCost + A * S1 + A * S2.

% we have two d/farray-values as arguments 
costD(dmap2(Arg1, Arg2, _, InnerPlan, _), Sel, _, C) :-
  cost(Arg1, _, _, C1),
  cost(Arg2, _, _, C2),
  substituteSubterm(rel('.', *, u), Arg1, 
    InnerPlan, LocalInnerPlan1),
  substituteSubterm(rel('..', *, u), Arg2, 
    LocalInnerPlan1, LocalInnerPlan),
  cost(LocalInnerPlan, Sel, _, InnerCost),
  dmap2TC(A),
  !,
  ArgS1 is Sel * C1,
  ArgS2 is Sel * C2,
  C is InnerCost + A * ArgS1 + A * ArgS2.
  
 costD(dmap2(RelObj1, RelObj2, _, InnerPlan, _), Sel, _, C) :-
 substituteSubterm(rel('.', *, u), "#!SUBST1!#", RelObj1, RelObj_Mod1),
  substituteSubterm(rel('.', *, u), "#!SUBST2!#", RelObj2, RelObj_Mod2),
  substituteSubterm(rel('.', *, u), RelObj_Mod1, InnerPlan, TempPlan1),
  substituteSubterm(rel('..', *, u), RelObj_Mod2, TempPlan1, TempPlan2),
  substituteSubterm( "#!SUBST1!#", rel('.',*,u),TempPlan2, TempPlan3),
  substituteSubterm( "#!SUBST2!#", rel('.',*,u),TempPlan3, FinallyGoodPlan),
  dmap2TC(A),
  cost(RelObj1, 1, Card1, _),
  cost(RelObj2, 1, Card2, _),
  cost(FinallyGoodPlan, Sel, _, InnerCost),
  !,
  S1 is Sel * Card1,
  S2 is Sel * Card2,
  C is InnerCost + A * S1 + A * S2.

% we have two d/fmatrix-values as arguments 
costD(areduce2(Arg1, Arg2, _, InnerPlan, _), Sel, _, C) :-
  cost(Arg1, _, _, C1),
  cost(Arg2, _, _, C2),
  substituteSubterm(rel('.', *, u), Arg1, 
    InnerPlan, LocalInnerPlan1),
  substituteSubterm(rel('..', *, u), Arg2, 
    LocalInnerPlan1, LocalInnerPlan),
  cost(LocalInnerPlan, Sel, _, InnerCost),
  areduce2TC(A),
  !,
  ArgS1 is Sel * C1,
  ArgS2 is Sel * C2,
  C is InnerCost + A * ArgS1 + A * ArgS2.

 costD(collect2(InnerPlan, _ , _), Sel, S, C) :-
  cost(InnerPlan, Sel, S, InnerCost),
  collect2TC(A),
  C is InnerCost + A * S.  
  
 costD(partitionF(RelObj, _, InnerPlan, _, _), Sel, S, C) :-
   distributedRels(LocalMasterRel, RelObj, _, _, _),
  substituteSubterm(rel('.', *, u), LocalMasterRel, 
    InnerPlan, LocalInnerPlan),
  partitionFTC(A),
  cost(LocalMasterRel, 1, S, _),
  cost(LocalInnerPlan, Sel, _, InnerCost),
  !,
  C is (InnerCost + A) * S.

  % generic case
 costD(partitionF(RelObj, _, _, _), _, S, C) :-
  cost(RelObj, 1, RS, RC),
  partitionFTC(A),
  S is RS,
  C is RC + S * A.

 costD(extendstream(Stream, _, cellnumber(bbox(_), _)), _, S, C) :-
  cost(Stream, 1, S, StreamC),
  extendstreamTC(ETC),
  bboxTC(BTC),
  cellnumberTC(CTC),
  TC is  ETC + BTC + CTC,
  C is S * TC + StreamC.

costD(range(_, Rel, _, _), Sel, S, C) :-
  cost(Rel, 1, Card, _),
  S is Sel * Card,
  leftrangeTC(A),
  C is A * S.

costD(dloop2(_, RelObj, _, InnerPlan), Sel, S, C) :-
  distributedRels(LocalMasterRel, RelObj, _, _, _),
  substituteSubterm(rel('..', *, u), LocalMasterRel, 
    InnerPlan, LocalInnerPlan),
  dloopTC(A),
  cost(LocalMasterRel, 1, Card, _),
  cost(LocalInnerPlan, Sel, _, InnerCost),
  !,
  S is Sel * Card,
  C is InnerCost + A * S.


/* dummy for dsummarize */
costD(dsummarize(_), _, _, 0).

costD(dsummarize(X), Sel, S, C) :-
  cost(X, Sel, S, C1),
  dsummarizeTC(A),
  C is C1 + A * S.


/*
7 Merging Plan Edges

  ----    mergePlanEdges(PlanEdgeList, MergedEdgesList)
  ----

  Merge the distribution of a query on a distributed query result
  to the distribution of the query on a query result. Example:
  dmap(... filter(.,bla1)) dmap filter(., bla2) 
  ...becomes:  dmap(... filter(filter(., bla1), bla2)) 

*/

mergePlanEdges([], []).
mergePlanEdges([X], [X]).

/*
  Merge rule for two successive dmaps with filtrations as there parameters
  should be the most common case.

*/

mergePlanEdges([Edge1, Edge2|Edges], MergedEdges) :-
  Edge1 = costEdge(Source, _, Plan1, Res1, _, C1),
  Edge2 = costEdge(_, Target, Plan2, Res2, S2, C2),
  Plan1 = dmap(Arg, _, filter(FilterArg, Pred1)),
  successiveFilterOnParam(FilterArg, ArgTerm),
  Plan2 = dmap(res(Res1), ResName, filter(ArgTerm, Pred2)),
  MergedPlan = dmap(Arg, ResName, 
    filter(filter(FilterArg, Pred1), Pred2)),
  % the plan is already chosen at this point, so costs will have no influence
  MergedCosts is C1 + C2,
  MergedHead = costEdge(Source, Target, MergedPlan, Res2, S2, MergedCosts),
  mergePlanEdges([MergedHead|Edges], MergedEdges).

% First two edges cannot be merges according to the above rules.
mergePlanEdges([X|Tail], [X|MergedTail]) :-
  mergePlanEdges(Tail, MergedTail).

% Term is a dot or a nested filtration on a dot.
successiveFilterOnParam(Term, ArgTerm) :-
  functor(Term, filter, 2),
  arg(1, Term, FirstArg),
  successiveFilterOnParam(FirstArg, ArgTerm).

successiveFilterOnParam(Term, Term) :-
  Term = feed(rel('.', _, _)).

successiveFilterOnParam(Term, Term) :-
  Term = rename(feed(rel('.', _, _)), _).

/*
8 Check for Distributed Queries

Checks if all relations are distributed. Currently the 
optimizer can only handle queries including relations, that 
are all local or distributed. Situations with mixed 
relation types will be discarded.

*/

%handle not distributed queries
checkDistributedQuery :-
   not(isDistributedQuery),
   isLocalQuery,
   !.
   
checkDistributedQuery :-
   isDistributedQuery,
   not(isLocalQuery),
   !.
   
checkDistributedQuery :- 
  write('Error in query: not all relations distributed '),
  fail,
  !.


/*
8 Check the Spelling of Non-Relation Objects 
 
*/

spelledObj(Term, Obj, Type, l) :-
  downcase_atom(Term, DcObj),
  objectCatalog(DcObj, LcObj, Type),
  LcObj = lc(Obj),
  !.

spelledObj(Term, Obj, Type, u) :-    
  downcase_atom(Term, DcObj),
  objectCatalog(DcObj, Obj, Type), 
  !.
  
spelledObj(_, _, _, _) :- !, fail.  % no entry, avoid backtracking.


/*
9 Auxiliary Predicates

*/

%fapra 15/16

% Extract parts from a query
destructureQuery(Select from Rel where Pred, Select, Rel, Pred).

% Pred is a predicate about the value of an attribute being equal to given value
attrValueEqualityPredicate(Pred, Value, Attr, Rel) :-
  Pred = pr(Value = Attr, Rel),
  Attr = attr(_, _, _).

attrValueEqualityPredicate(Pred, Value, Attr, Rel) :-
  Pred = pr(Attr = Value, Rel),
  Attr = attr(_, _, _).

/*

----   substituteSubterm(Substituted, Substitute, OriginalTerm, TermWithSubstitution)
----

Substituting ~Substituted~ for ~Substitute~ on ~OriginalTerm~ yields ~TermWithSubstitution~. We have a cut in every clause to remove unnecessary choice points
during the search for planedges, which ois driven by meta predicates.

*/

% The whole term is to be substituted:
substituteSubterm(Substituted, Substitute, Substituted, Substitute):- !.

% The whole term doesn't match and it's not compound:
substituteSubterm(Substituted, _, OriginalTerm, OriginalTerm) :-
  functor(OriginalTerm, _, 0),
  OriginalTerm \= Substituted, !.

% The whole term doesn't match and it's compount - dive into its subterms:
substituteSubterm(Substituted, Substitute, OriginalTerm, 
  TermWithSubstitution) :-
  functor(OriginalTerm, Functor, Arity),
  functor(TermWithSubstitution, Functor, Arity),
  substituteSubtermInNthSubterm(Arity, Substituted, 
    Substitute, OriginalTerm, TermWithSubstitution), !.

% Terminal case. All subterms have been processed.
substituteSubtermInNthSubterm(0, _, _, _, _):- !.

% Generic case. Process nth subterm.
substituteSubtermInNthSubterm(N, Substituted, Substitute, 
  OriginalTerm, TermWithSubstitution) :-
  not(N = 0),
  arg(N, OriginalTerm, OriginalNthTerm),
  substituteSubterm(Substituted, Substitute, 
    OriginalNthTerm, NthTermWithSubstitution),
  arg(N, TermWithSubstitution, NthTermWithSubstitution),
  Next is N - 1,
  substituteSubtermInNthSubterm(Next, Substituted, 
    Substitute, OriginalTerm, TermWithSubstitution), !.


/*
Rename an attribute to match the renaming of its relation.

*/

% No renaming needed.
renamedRelAttr(RelAttr, Var, RelAttr) :-
  Var = *, !.

renamedRelAttr(attr(Name, N, C), Var, attr(Var:Name, N, C)).


% Extract the down case name from an attr term.
attrnameDCAtom(Attr, DCAttrName) :-
  Attr = attr(_:Name, _, _),
  !,
  atom_string(AName, Name),
  downcase_atom(AName, DCAttrName).

attrnameDCAtom(Attr, DCAttrName) :-
  Attr = attr(Name, _, _),
  atom_string(AName, Name),
  downcase_atom(AName, DCAttrName).


/*
Rename a tuple a stream.

*/

% No renaming needed.
renameStream(Stream, Var, Plan) :-
  Var = *,
  !,
  Plan = Stream.

renameStream(Stream, Var, rename(Stream, Var)).

/*
Transform a relation to a tuple stream and rename it.

*/

% No renaming needed.
feedRenameRelation(Rel, Var, Plan) :-
  Var = *,
  !,
  Plan = feed(Rel).

feedRenameRelation(Rel, Var, Plan) :-
  Plan = rename(feed(Rel), Var).

feedRenameRelation(rel(Rel, Var,_), Plan) :-
  feedRenameRelation(Rel, Var, Plan),!.


/*
11 Extensions to File ~database.pl~

11.1 Auxiliary Predicates

*/

[library(apply)].

:-
  dynamic(isDistributedQuery/0),
  dynamic(isLocalQuery/0).

/* 
Strip a string off its opening and closing quote. 

*/

stringWithoutQuotes(Str, StrQuoteless) :-
  string_to_atom(Str, StrAtom),
  string_concat(X, '\"', StrAtom),
  string_to_atom(X, XAtom),
  string_concat('\"', StrQuoteless , XAtom).

stringWithoutQuotes(Str, Str) :-
  not(string(Str)),!.

/*
Removes the suffix '\_d' from ~DRel~ indicating a distributed relation. If the 
relation is not listed in SEC2DISTRIBUTED the unchanged name is returned in
Variable ~ORel~

*/  
removeDistributedSuffix(DRel as _,ORel) :-
    removeDistributedSuffix(DRel,ORel),!.

removeDistributedSuffix(DRel,ORel) :-
    atom(DRel),
    string_concat(X,'_d', DRel),
    string_to_atom(X, ORel),
    isDistributedRelation(rel(ORel,_,_)),!,
    assertOnce(isDistributedQuery).

removeDistributedSuffix(ORel,DRel) :-
    ORel = DRel,
    !,
    assertOnce(isLocalQuery).

/* 
Ensure to assert a fact only once.

*/

assertOnce(Fact) :-
    not(Fact),!,
    assert(Fact).

assertOnce(_).


/*
11.2 Creating a list of database objects. 

We assume that the object name starts with an capital
 letter. If not an lc()- functor indicates that the 
initial letter is written in lower case. The rest of the
identifier is written mixed case.

*/

:- 
  dynamic(storedObject/3).
  
objectCatalog(DcObj, Obj, Type) :-
  storedObject(DcObj, Obj, Type),
  !.

objectCatalog(DcObj, LcObj, Type) :-
  getSecondoList(ObjList),
  member(['OBJECT',Obj,_,[Type|_]], ObjList),
  downcase_atom(Obj, DcObj),
  is_lowerfl(Obj),
  LcObj = lc(Obj),
  assert(storedObject(DcObj, LcObj, Type)),
  !.

objectCatalog(DcObj, FlObj, Type) :-
  getSecondoList(ObjList),
  member(['OBJECT',Obj,_,[Type|_]], ObjList),
  downcase_atom(Obj, DcObj),
  not(is_lowerfl(Obj)),
  lowerfl(Obj,FlObj),
  assert(storedObject(DcObj, FlObj, Type)),
  !.


/*
11.3 Reading the catalogue of distributed relations

Get metainformation about the distributed relations in this db.
Use distributedRels/5 predicate in conjuction with isDistributedQuery
to cover special cases for distributed queries. 

*/

:-
  dynamic(storedDistributedRelation/6),
  dynamic(onlineWorker/3).

distributedRels(Rel, Obj, DistType, PartType, DistAttr) :-
  distributedRels(Rel, Obj, DistType, PartType, DistAttr, _).

distributedRels(rel(Rel,Var,Case), ObjName, DistType, 
  PartType, DistAttr, DistParam) :-
    storedDistributedRelation(_,_,_,_,_,_), 
    ground(Var), !,% first argument instantiated - but do not match against Var
    storedDistributedRelation(rel(Rel,_,Case), ObjName, 
    DistType, PartType, DistAttr, DistParam).

distributedRels(rel(Rel,Var,Case), ObjName, DistType, 
  PartType, DistAttr, DistParam) :-
    storedDistributedRelation(_,_,_,_,_,_), !,
    storedDistributedRelation(rel(Rel,Var,Case), ObjName, 
    DistType, PartType, DistAttr, DistParam).

distributedRels(rel(Rel,Var,Case), ObjName, DistType, 
  PartType, DistAttr, DistParam) :-
    not(storedDistributedRelation(_,_,_,_,_,_)),
    ground(Var), !,% first argument instantiated - but do not match against Var
    queryDistributedRels,!,
    storedDistributedRelation(rel(Rel,_,Case), ObjName, 
    DistType, PartType, DistAttr, DistParam).

distributedRels(rel(Rel,Var,Case), ObjName, DistType, 
  PartType, DistAttr, DistParam) :-
    not(storedDistributedRelation(_,_,_,_,_,_)),
    queryDistributedRels,!,
    storedDistributedRelation(rel(Rel,Var,Case), ObjName, 
    DistType, PartType, DistAttr, DistParam).


%check whether the relation is distributed or not
isDistributedRelation(rel(Rel,_,_)) :-
  distributedRels(rel(Rel,'*',_),_,_,_,_),
  !.

/* 

Read the values from SEC2DISTRIBUTED relation and store it to a
dynamic predicate.
Values of string attributes are passed to us as atoms with an 
opening and closing quote and have to be stripped off these.
 
*/

storeDistributedRels([]).

storeDistributedRels([[RelName, ObjName, DistType, 
  PartType, DistAttr, DistParam]|T]) :-
  storeDistributedRel(RelName, ObjName, DistType, 
    PartType, DistAttr, DistParam), 
  storeDistributedRels(T).

storeDistributedRel(RelName, ObjName, DistType, 
  PartType, DistAttr, DistParam) :-         
  spelled(RelName, Rel, CaseRel),
  spelledDistributedRel(ObjName, Arr, CaseArr),
  (checkOnlineWorkers(ObjName, PartType)
  -> assert(storedDistributedRelation(rel(Rel,'*',CaseRel), 
         rel(Arr,'*',CaseArr), 
         DistType, PartType, DistAttr, DistParam));
  ansi_format([fg(red)], 'Warning: listed object "~w" in \c
      SEC2DISTRIBUTED relation is not available => \c
      ignored for further processing \n',[ObjName])),
  !.

storeDistributedRel(_, _, _, _, _) :- !.

spelledDistributedRel(Rel, Rel2, Case) :-
    spelled(Rel,Rel2,Case);
    (ansi_format([fg(red)], 'Warning: listed object "~w" in SEC2DISTRIBUTED \c
      relation does not exist => ignored for further processing \n',[Rel]),
      fail),
    !.

/* 
The availibility of workers related to the distributed relations used
in the current query needs to be checked before creating an execution plan.

We have to distinguish between the type of distribution. Replicated 
objects and relations are shared to all workers, available at distribution
time. Therefore it's not possible to backtrack workers involved at this
moment.

Shared relations can be executed even not the complete set of workers are
online, for other distribution types all workers are necessary.

To provide a possibility to test the distributed queries without 
executing it on the worker, its possible to disable the connectivity 
check by setting the fact 'disableWorkerCheck'
 
*/

:- dynamic(disableWorkerCheck/0).
%:-   assert(disableWorkerCheck).

%check the entries in SEC2DISTRIBUTED
checkOnlineWorkers :-
    disableWorkerCheck,!.

checkOnlineWorkers :-
    secondo('query SEC2WORKERS',[_,ListOfWorkers]),!,
    maplist(maplist(stringWithoutQuotes), ListOfWorkers, StrippedListOfWorkers),
    checkOnlineWorker(StrippedListOfWorkers),
    !.

%check workers listed in d(f)array
checkOnlineWorkers(_, _) :-
    disableWorkerCheck,!.

%first parameter must be a d(f)array
checkOnlineWorkers(_, 'share').

checkOnlineWorkers(ObjName, _) :- 
    string_concat('query ',ObjName, SecondoQueryStr),
    string_to_atom(SecondoQueryStr,SecondoQuery), 
    secondo(SecondoQuery,[_, [_,_,ListOfWorkers]]),
    checkOnlineWorker(ListOfWorkers),
    !.

checkOnlineWorker([]).

checkOnlineWorker([[Host,Port,_]|T]) :-
    onlineWorker(Host,Port,_),!,
    checkOnlineWorker(T).

checkOnlineWorker([[Host,Port,Config]|T]) :-
    format(atom(SecondoQuery),'query connect("~w",~w,"~w")',
      [Host,Port,Config]), 
    secondo(SecondoQuery,[bool, Result]),!,
    (Result == true
    -> assert(onlineWorker(Host,Port,Config));
    cancelOnlineWorkerCheck(Host,Port,Config)),!,
    checkOnlineWorker(T).
    
%worker offline
cancelOnlineWorkerCheck(Host,Port,Config) :-
    ansi_format([fg(red)], 'Warning: connection to server \c
    host: "~w", port: ~w, config: "~w" failed \n', [Host,Port,Config]),
    fail,!.

/* 
The system- relation SEC2DISTRIBUTED contains information about
distributed relations in the opened database. 

SEC2WORKERS is another necessary relation when using distributed
queries. It contains the available workers in the system.

If necessary the two relations will be created without content. 

*/

queryDistributedRels :-
  retractall(storedDistributedRelation(_,_,_,_,_)),
  distributedRelsAvailable,
  secondo('query SEC2DISTRIBUTED',[_, Tuples]), 
  !,
  maplist(maplist(stringWithoutQuotes), Tuples, StrippedTuples),
  maplist(maplist(string_to_atom), StrippedTuples, ObjList),
  storeDistributedRels(ObjList),
  !.

distributedRelsAvailable :-
  retractall(storedSecondoList(_)),
  getSecondoList(ObjList),
  ( member(['OBJECT','SEC2DISTRIBUTED',_ | [[[_ | [[_ | [_]]]]]]], ObjList) ->
    true;
    secondo('let SEC2DISTRIBUTED = [const rel(tuple(\c
             [RelName: string,\c 
              ArrayRef: string, \c 
              DistType: string, \c
              PartType: string, \c
              PartAttribute: string, \c
              PartParam: string]))value()]',_),
    writeln('Created empty SEC2DISTRIBUTED system-relation \n')
  ),
  ( member(['OBJECT','SEC2WORKERS',_ | [[[_ | [[_ | [_]]]]]]], ObjList) ->
    true;
    secondo('let SEC2WORKERS = [const rel(tuple(\c
             [Host: string,\c 
              Port: int, \c 
              Config: string]))value()]',_),
    writeln('Created empty SEC2WORKERS system-relation \n')
  ),
  ( member(['OBJECT','SEC2DISTINDEXES',_ | [[[_ | [[_ | [_]]]]]]], ObjList) ->
    true;
    secondo('let SEC2DISTINDEXES = [const rel(tuple(\c
             [DistObj: string,\c 
              Attr: string, \c 
              IndexType: string, \c
              IndexObj: string]))value()]',_),
    writeln('Created empty SEC2DISTINDEXES system-relation \n')
  ),
  !.

distributedRelsAvailable :-
   writeln('no open database').

% switch to dynamic predicate sometime in the future

distributedIndex(DistRelObj, Attr, IndexType, IndexObj) :-
  secondo('query SEC2DISTINDEXES',[_, Tuples]), !,
  maplist(maplist(stringWithoutQuotes), Tuples, StrippedTuples),
  maplist(maplist(string_to_atom), StrippedTuples, AtomTuples),
  maplist(maplist(downcase_atom), AtomTuples, DowncaseAtomTuples),
  !,
  member([DistRelObj, Attr, IndexType, IndexObj], DowncaseAtomTuples).



/*
12 Extensions to File ~opsyntax.pl~

BTree2Algebra \\
MMRTreeAlgebra \\
RTreeAlgebra \\
RectangleAlgebra \\

----     _ _ range [ _ , _ ]
         _ _ itSpatialJoin [ _, _ ,_,_]
         _ _ windowintersects [ _ ]
         bbox ( _ )
         gridintersects (_, _, _, _)
         cellnumber ( _, _)
----

*/

secondoOpD(range, postfixbrackets, 4).
secondoOpD(itSpatialJoin, postfixbrackets, 4).
secondoOpD(windowintersects, postfixbrackets, 3).
secondoOpD(bbox, prefix, 1).
secondoOpD(gridintersects, prefix, 4).
secondoOpD(cellnumber, prefix, 2).

/* 

Array- Algebra 

----     _ tie [ _ ]
----

*/

secondoOpD(tie, postfixbrackets, 2).

/* 

Distributed2- Algebra 

----     _ dmap[_,_]
         _ dloop[_,_]
         _ _ dmap2[_,_]
         _ _ dloop2[_,_]
         share( _, _, _)
         _ partition[_,_,_]
         _ partitionF[_,_,_,_]
         _ collect2[ _ , _]
         _ areduce[_, _, _]
         getValue(_)
         _ dsummarize
----

*/

secondoOpD(dmap, postfixbrackets2, 3).
secondoOpD(dloop, postfixbrackets2, 3).
secondoOpD(dmap2, postfixbrackets3, 5).
secondoOpD(areduce2, postfixbrackets3, 5).
secondoOpD(dloop2, postfixbrackets3, 5).
secondoOpD(share, prefix, 3).
secondoOpD(partition, postfixbrackets3, 4).
secondoOpD(partitionF, postfixbrackets4, 5).
secondoOpD(collect2, postfixbrackets2, 3).
secondoOpD(areduce, postfixbrackets3, 4).
secondoOpD(getValue, postfix, 1).
secondoOpD(dsummarize, postfix, 1).

/*
StandardAlgebra

----     hashvalue ( _, _ )
         _ ++
----

*/

secondoOpD(hashvalue,prefix, 2).
secondoOpD(++, postfix, 1).

/*
13 Extensions to File ~operators.pl~

Some constants for cost functions.

*/

dloopTC(1.3).
dsummarizeTC(28).
dmapTC(0.12).
dmap2TC(0.07).
dloop2TC(0.2).
shareTC(447).
getValueTC(14.4).
partitionTC(11.8).
partitionFTC(8).
areduceTC(3.1).
collect2TC(0.9).

tieTC(0.006).

bboxTC(0.002).
gridintersectsTC(0.99).
cellnumberTC(0.07).
hashvalueTC(0.001).
rangeTC(10). % copied from leftrange
windowintersectsTC(0.1). %costs need to be evaluated, taken from optimizer
itSpatialJoinTC(20.0, 0.7). %costs need to be evaluated, taken from optimizer
extendstreamTC(0.0). %costs need to be evaluated
areduce2TC(0.0). %costs need to be evaluated




