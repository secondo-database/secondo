operator head alias HEAD pattern _ op [ _ ]

operator max alias MAX pattern _ op [ _ ]
operator min alias MIN pattern _ op [ _ ]
operator avg alias AVG pattern _ op [ _ ]
operator sum alias SUM pattern _ op [ _ ]
operator count alias COUNT pattern _ op

operator feed alias FEED pattern _ op
operator consume alias CONSUME pattern _ op
operator concat alias CONCAT pattern _ _ op
operator attr alias ATTR pattern op (_, _)
operator project alias PROJECT pattern _ op [list]
operator remove alias REMOVE pattern _ op [list]
operator filter alias FILTER pattern _ op [ fun ] implicit parameter tuple type TUPLE
operator cancel alias CANCEL pattern _ op [ fun ] implicit parameter tuple type TUPLE
operator rdup alias RDUP pattern _ op
operator sort alias SORT pattern _ op
operator extend alias EXTEND pattern _ op [funlist] implicit parameter tuple type TUPLE
operator groupby alias GROUPBY pattern _ op [list; funlist] implicit parameter group type GROUP
operator gfeed alias GFEED pattern _ op
operator product alias PRODUCT pattern _ _ op
# operator loopjoin alias LOOPJOIN pattern _ _ op [ fun ] implicit parameters lefttuple, righttuple types TUPLE, TUPLE2
operator mergesec alias MERGESEC pattern _ _ op
operator mergediff alias MERGEDIFF pattern _ _ op
operator mergeunion alias MERGEUNION pattern _ _ op


operator sortby alias SORTBY pattern _ op [list]
operator mergejoin alias MERGEJOIN pattern _ _ op [_, _]
operator sortmergejoin alias SORTMERGEJOIN pattern _ _ op [_, _]
operator hashjoin alias HASHJOIN pattern _ _ op [_, _, _]

operator loopjoin alias LOOPJOIN pattern _ op [ fun ] implicit parameter tuple type TUPLE
operator loopjoinrel alias LOOPJOINREL pattern _ op [ fun ] implicit parameter tuple type TUPLE

operator loopsel alias LOOPSEL pattern _ op [ fun ] implicit parameter tuple type TUPLE

operator rename alias RENAME pattern _ op [ _ ]
operator extract alias EXTRACT pattern _ op [ _ ]

operator sample alias SAMPLEFEED pattern _ op [_, _]
