:- assert(
optimizerOptionInfo(adaptiveJoin, none,   
                     '\tAllow usage of adaptive join operators.',
                     ( delOption(entropy), 
                       delOption(intOrders(on)),
                       delOption(intOrders(quick)),
                       delOption(intOrders(path)),
                       delOption(intOrders(test)),
                       loadFiles(adaptiveJoin) 
                     ), 
                     true )).

:- assert(
optimizerOptionInfo(entropy, none,
                    '\tUse entropy to estimate selectivities.',
                    ( delOption(intOrders(on)),
                      delOption(intOrders(quick)),
                      delOption(intOrders(path)),
                      delOption(intOrders(test)),
                      delOption(immediatePlan),
                      loadFiles(entropy), 
                      (   notIsDatabaseOpen
                        ; ( retractall(storedSecondoList(_)),
                            getSecondoList(ObjList),
                            checkForAddedIndices(ObjList), 
                            checkForRemovedIndices(ObjList),
                            checkIfSmallRelationsExist(ObjList),
                            retractall(storedSecondoList(_))
                          )
                      )
                    ), 
	            true )).

:- assert(
optimizerOptionInfo(rewriteNonempty, rewriteInference,
                    'Handle \'nonempty\' in select statements.',
                    true, true)).
