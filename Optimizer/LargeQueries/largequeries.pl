/*
  Include Files for Large Query Optimization
*/

:- ['./LargeQueries/querygraph.pl'].        % query graph utilities
:- ['./LargeQueries/querygenerator.pl'].    % query graph generator

/*
  Include File for query graph decomposition (qgd) optimization
*/
initLargeQueries:-   
  optimizerOption(largeQueries(qgd)),
  clearRedefinitions, 
  ['./LargeQueries/qgd.pl'].     

/*
  Include File for query graph decomposition and materialization (qgdm) optimization
*/
initLargeQueries:-
  optimizerOption(largeQueries(qgdm)),
  clearRedefinitions, ['./LargeQueries/qgdm.pl'].     

/*
  Include File for ascending cost order (aco) optimization 
*/
initLargeQueries:-
  optimizerOption(largeQueries(aco)),
  clearRedefinitions, 
  ['./LargeQueries/aco.pl'].      

initLargeQueries:-
  true.

clearRedefinitions:-
  abolish(completesCycle/3),
  abolish(completesCycle/2),
  abolish(kruskal/5),
  abolish(kruskal2/4),
  abolish(extractSelectionPredicates/2),
  abolish(removeSelectionPredicates/1),
  abolish(globalizeSelectionPredicates/1),
  abolish(qgedgeList2predList/2),
  abolish(createComponents/2),
  abolish(reoptimize/1),
  abolish(joinComponents/3),
  abolish(optimizeAllRemainingComponents/0),
  abolish(optimizeRemainingComponents/1),
  abolish(optimizeRemainingComponent/1),
  abolish(largerList/3),
  abolish(compareComponents/4),
  abolish(isIncidentEdgeToA/2),
  abolish(isIncidentEdgeToB/2),
  abolish(joinableComponents/3),
  abolish(countComponents/1),
  abolish(reoptimizePlan/4),
  abolish(splitEdges/3),
  abolish(splitEdges2/3),
  abolish(incResultCost/1),
  abolish(initResultCost/0),
  abolish(setSelect/1),
  abolish(setRels/1),
  abolish(setMaxEdgesPerComponent/1),
  abolish(removePredinfos/5),
  abolish(removePredinfos2/4),
  abolish(removePredinfo/5),
  abolish(embeddedPredinfo/5),
  abolish(embedJoinEdge/3),
  abolish(embedJoinEdges/3),
  abolish(embedJoinEdges2/3),
  abolish(embedComponentPlans/2),
  abolish(embeddedComponentPlan/2).

