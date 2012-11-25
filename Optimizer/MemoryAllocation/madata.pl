/*

$Header$
@author Nikolai van Kempen

Some ~memory allocation~-related data about operators.

*/


/*

Facts about operators and plan-terms. Note that the operator name and the parameters usually matches the plan terms.

The needed information here is in particular an example signature to obtain the operator's indexes. Currently, the signature for a operator can't be obtained by a predicate which analyses the plan because there is no such predicate as far as I konw.

In addition, the substreams needed to be identified within the plan parameter which must be analyzed further on. A value t within the SubStream list means that the parameter at the same position within the SubStreamList is a substream, otherwise use f. For other operators without a CostEstimation implementation, the same procedure is done with the opSigMap facts.

maOpSig(+OpName, +SubStream, +Signature)

No CostEstimation implementation:
maOpSig(sort,		[[stream,[tuple,[[a,int]]]]]).
maOpSig(sortby,	[[stream,[tuple,[[a,int]]]], string]).

Only operators with a maOpSig facts are respected in the memory optimization.
*/
maOpSig(mergejoin, [[stream, [tuple, [[a, int]]]], [stream, [tuple, [[b, int]]]], a, b]).
maOpSig(sortmergejoin, [[stream, [tuple, [[a, int]]]], [stream, [tuple, [[b, int]]]], a, b]).
maOpSig(symmjoin, [[stream,[tuple,[[a,int]] ]], [stream,[tuple,[[b,int]]]], [map,[tuple,[[a,int]]],[tuple,[[b,int]]],bool]]).

% Operators which are not currently used within the optimizer:
maOpSig(gracehashjoin, [[stream, [tuple, [[a, int]]]], [stream, [tuple, [[b, int]]]], a, b, int]).
maOpSig(hybridhashjoin, [[stream, [tuple, [[a, int]]]], [stream, [tuple, [[b, int]]]], a, b, int]).
maOpSig(itHashJoin, [[stream, [tuple, [[a, int]]]], [stream, [tuple, [[b, int]]]], a, b]).
maOpSig(itSpatialJoin, [[stream, [tuple, [[a, upoint]]]], [stream, [tuple, [[b, upoint]]]], a, b, int, int, int]).
% Add more operators here if needed.

% Refer to the cost functions about these operators.
opSigMap(sortLeftThenMergejoin,  [t,t,f,f]).
opSigMap(sortRightThenMergejoin, [t,t,f,f]).
% Overwritten because the opSignature result can not recognise the secondo argument as a stream:
opSigMap(loopjoin, 			 [t,t]). % differs from the plan terms.
opSigMap(res, 					 [f]).   % not known as a plan term.
opSigMap(predinfo, 			 [t,f,f]). % not known as a plan term.

% Missing operators.pl facts/predicates.
% just added here to make maOpInfo working.
opSigMap(gracehashjoin,  [t,t,f,f,f]).  
opSigMap(hybridhashjoin, [t,t,f,f,f]).  
opSigMap(itHashJoin,     [t,t,f,f]). 
opSigMap(itSpatialJoin,  [t,t,f,f,f,f,f]). 

/*
old stuff that is now extracted from the operators.pl data.
opSigMap(rename, 				[t]).
opSigMap(remove, 				[t]).
opSigMap(filter, 				[t,f]).
opSigMap(project,				[t,f]).
opSigMap(feedproject,		[f,f]).
opSigMap(afeed,					[f]).
opSigMap(extend,				[t,f]).
*/

% eof
