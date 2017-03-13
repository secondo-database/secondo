# simple prefix operator having one argument

operator hlCalcRank alias hlCalcRank pattern op(_,_,_)

operator hlOneHopReverseSearch alias hlOneHopReverseSearch pattern op(_,_,_)

operator hlHHopForwardSearch alias hlHHopForwardSearch pattern op(_,_,_,_,_)

operator hlForwardSearchGetDist alias hlForwardSearchGetDist pattern op(_,_,_)

operator hlRemoveTFromCurrentWitnessList alias hlRemoveTFromCurrentWitnessList pattern op(_,_)

operator hlForwardSearchCheckForWitnessPath alias hlForwardSearchCheckForWitnessPath pattern op(_,_,_,_,_)

operator hlInsertOrUpdateTupleInNotYetVisitedList alias hlInsertOrUpdateTupleInNotYetVisitedList pattern op(_,_,_,_)

operator hlForwardSearchIterativeStepsScanNewVertices alias hlForwardSearchIterativeStepsScanNewVertices pattern op(_,_,_,_,_,_,_)

operator hlForwardSearchProcessIncomingEdgeIterativeSteps alias hlForwardSearchProcessIncomingEdgeIterativeSteps pattern op(_,_,_,_,_,_,_)

operator hlForwardSearchProcessIncomingEdgeInitialSteps alias hlForwardSearchProcessIncomingEdgeInitialSteps pattern op(_,_,_,_,_,_)

operator hlForwardSearchCreateAndAppendShortcuts alias hlForwardSearchCreateAndAppendShortcuts pattern op(_,_,_,_)

operator hlForwardSearchProcessIncomingEdge alias hlForwardSearchProcessIncomingEdge pattern op(_,_,_,_,_,_,_,_)

operator hlAddShortcutsToEdgesRealtions alias hlAddShortcutsToEdgesRealtions pattern op(_,_,_)

operator hlRemoveContractedEdgesFromEdgesRelations alias hlRemoveContractedEdgesFromEdgesRelations pattern op(_,_,_)

operator hlRemoveParallelEdgesFromEdgesRelations alias hlRemoveParallelEdgesFromEdgesRelations pattern op(_,_,_)

operator hlDoContraction alias hlDoContraction pattern op(_,_,_,_,_)

operator hlIterateOverAllNodesByRankAscAndDoContraction alias hlIterateOverAllNodesByRankAscAndDoContraction pattern op(_,_,_,_,_)

operator hlCreateLabelCheckForWitnessScanNewVertices alias hlCreateLabelCheckForWitnessScanNewVertices pattern op(_,_,_,_,_,_)

operator hlCreateLabelCheckForWitness alias hlCreateLabelCheckForWitness pattern op(_,_,_,_,_,_,_)

operator hlCreateLabelScanNewVertices alias hlCreateLabelScanNewVertices pattern op(_,_,_,_,_,_,_)

operator hlGetRankBaId alias hlGetRankBaId pattern op(_,_,_)

operator hlCreateLabelByDijkstraWithStalling alias hlCreateLabelByDijkstraWithStalling pattern op(_,_,_,_,_,_,_)

operator hlFillForwardOrReverseLabel alias hlFillForwardOrReverseLabel pattern op(_)

operator hlGetPathViaPoints alias hlGetPathViaPoints pattern op(_,_,_,_,_,_)

operator hlQuery alias hlQuery pattern op(_,_,_,_,_,_)

operator hlPruneLabelByBootstrapping alias hlPruneLabelByBootstrapping pattern op(_,_,_,_,_,_)

operator hlReorderLabels alias hlReorderLabels pattern op(_,_,_,_,_,_)

operator hlCreateLabels alias hlCreateLabels pattern op(_,_,_,_,_)



operator hlContractNew alias hlContractNew pattern op(_,_,_)



operator hlTransformOrelToHlGraph alias hlTransformOrelToHlGraph pattern op(_,_)

operator hlDoContractionOfHlGraph alias hlDoContractionOfHlGraph pattern op(_,_,_,_,_)

operator hlDoChSearchInHlGraph alias hlDoChSearchInHlGraph pattern op(_,_,_,_,_)

operator hlCalcWeightsOrel alias hlCalcWeightsOrel pattern op(_,_)

operator hlCreateLabelsFromHlGraph alias hlCreateLabelsFromHlGraph pattern op(_,_)
