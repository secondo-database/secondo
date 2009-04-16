:- assert(
optimizerOptionInfo(correlations, none, yes,
                     'Derive joint probabilities for selection predicates before computing a best plan',
                     ( delOption(entropy),
                       delOption(intOrders(on)),
                       delOption(intOrders(quick)),
                       delOption(intOrders(path)),
                       delOption(intOrders(test)),
                       loadFiles(correlations)
                     ),
                     ( delOption(joinCorrelations), delOption(joinCorrelations2) ) )).
% Recompute selectivities and cardinalities in the POG using the principle of entropy maximization. In contrast to the entropy option don't run an initial query on the small database'


:- assert(
optimizerOptionInfo(joinCorrelations, correlations, no,
                    'Additionally derive joint probabilities for join predicates (nested-loop)',
                    ( setOption(correlations), delOption(joinCorrelations2)), true)).

:- assert(
optimizerOptionInfo(joinCorrelations2, correlations, no,
                    'The same behavior as joinCorrelations but use product-filter',
                    ( setOption(correlations), delOption(joinCorrelations) ), true)).


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
   'Use entropy maximization together with an exploration query on a small sample database',
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

% Compute joint probabilities along the chosen path of the POG, maximize entropy to estimate selectivities again and compute a new best plan with the revised POG.'




:- assert(
optimizerOptionInfo(rewriteNonempty, rewriteInference,
                    'Handle \'nonempty\' in select statements.',
                    true, true)).
