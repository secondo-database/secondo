:- assert(
optimizerOptionInfo(correlations, none, yes,
                     'Try to take predicate interdependence into account.',
                     ( delOption(entropy),
                       delOption(intOrders(on)),
                       delOption(intOrders(quick)),
                       delOption(intOrders(path)),
                       delOption(intOrders(test)),
                       loadFiles(correlations)
                     ),
                     true )).

:- assert(
optimizerOptionInfo(adaptiveJoin, none, yes,
                     'Allow usage of adaptive join operators.',
                     ( delOption(entropy),
                       delOption(intOrders(on)),
                       delOption(intOrders(quick)),
                       delOption(intOrders(path)),
                       delOption(intOrders(test)),
                       loadFiles(adaptiveJoin)
                     ),
                     true )).

:- assert(
optimizerOptionInfo(entropy, none, yes,
                    'Use entropy to estimate selectivities.',
                    ( delOption(intOrders(on)),
                      delOption(intOrders(quick)),
                      delOption(intOrders(path)),
                      delOption(intOrders(test)),
                      delOption(immediatePlan),
                      loadFiles(entropy),
                      (   notIsDatabaseOpen
%                        ; ensureSmallObjectsExist
                        ; true
                      )
                    ),
	            true )).

:- assert(
optimizerOptionInfo(rewriteNonempty, rewriteInference,
                    'Handle \'nonempty\' in select statements.',
                    true, true)).
