#This file is part of SECONDO. 

#Copyright (C) 2004, University in Hagen, Department of Computer Science, 
#Database Systems for New Applications.

#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


operator head alias HEAD pattern _ op [ _ ]

operator max alias MAX pattern _ op [ _ ]
operator min alias MIN pattern _ op [ _ ]
operator avg alias AVG pattern _ op [ _ ]
operator sum alias SUM pattern _ op [ _ ]
operator var alias VAR pattern _ op [ _ ]
operator stats alias STATS pattern _ op [ _ , _ ]

operator concat alias CONCAT pattern _ _ op
operator cancel alias CANCEL pattern _ op [ fun ] implicit parameter tuple type TUPLE
operator rdup alias RDUP pattern _ op
operator krdup alias KRDUP pattern _ op [ list ]
operator krduph alias KRDUPH pattern _ op [ _,_ ]
operator sort_old alias SORTOLD pattern _ op
operator extend alias EXTEND pattern _ op [ funlist ] implicit parameter tuple type TUPLE
operator projectextend alias PROJECTEXTEND pattern _ op [ list ; funlist ] implicit parameter tuple type TUPLE
operator extendstream alias EXTENDSTREAM pattern _ op [funlist] implicit parameter tuple type TUPLE
operator projectextendstream alias PROJECTEXTENDSTREAM pattern _ op [list; funlist] implicit parameter tuple type TUPLE

operator groupby alias GROUPBY pattern _ op [list; funlist] implicit parameter group type GROUP
operator slidingwindow alias SLIDINGWINDOW pattern _ op [_, _; funlist] implicit parameter group type GROUP
operator mergesec alias MERGESEC pattern _ _ op
operator mergediff alias MERGEDIFF pattern _ _ op
operator mergeunion alias MERGEUNION pattern _ _ op

operator sortby_old alias SORTBYOLD pattern _ op [list]
operator mergejoin alias MERGEJOIN pattern _ _ op [_, _] !!
operator sortmergejoin_old alias SORTMERGEJOINOLD pattern _ _ op [_, _] !!
operator smouterjoin alias SMOUTERJOIN pattern _ _ op [_, _] !!
operator hashjoin alias HASHJOIN pattern _ _ op [_, _, _] !!

operator loopjoin alias LOOPJOIN pattern _ op [ fun ] implicit parameter tuple type TUPLE !!
operator loopsel alias LOOPSEL pattern _ op [ fun ] implicit parameter tuple type TUPLE

operator extract alias EXTRACT pattern _ op [ _ ]

operator printrefs alias PRINTREFS pattern _ op

operator sample alias SAMPLEFEED pattern _ op [_, _]

operator aggregate alias AGGREGATE pattern _ op [ _; _; _ ] 
operator aggregateB alias AGGREGATEB pattern _ op [ _; _; _ ] 
operator aggregateC alias AGGREGATEC pattern _ op [  _  ] 

operator symmjoin alias SYMMJOIN pattern _ _ op [ fun ] implicit parameters lefttuple, righttuple types TUPLE, TUPLE2 !!

operator symmouterjoin alias SYMMOUTERJOIN pattern _ _ op [ fun ] implicit parameters lefttuple, righttuple types TUPLE, TUPLE2 !!

operator symmproductextend alias SYMMPRODUCTEXTEND pattern _ _ op [ funlist ] implicit parameters lefttuple, righttuple types TUPLE, TUPLE2
operator symmproduct alias SYMMPRODUCT pattern _ _ op
operator addcounter alias ADDCOUNTER pattern _  op [_ , _]

operator ksmallest alias KSMALLEST pattern _ op [_ ; list  ] 
operator kbiggest alias KBIGGEST pattern _ op [_ ; list  ] 

operator extend_aggr alias EXTEND_AGGR pattern _ op [list  ; funlist ]

operator extend_last alias EXTEND_LAST pattern _ op [funlist ] implicit parameters currenttuple, lasttuple types TUPLE, TUPLE

operator extend_next alias EXTEND_NEXT pattern _ op [funlist ] implicit parameters currenttuple, lasttuple types TUPLE, TUPLE

operator toFields alias TOFIELDS pattern _ op [ _ ]
operator fromFields alias FROMFIELDS pattern _ _ op


operator applyToAll alias APPLYTOALL pattern _ op [ _ ]


operator replaceAttr alias REPLACEATTR  pattern _ op [ funlist ] implicit parameter tuple type TUPLE

operator pfilter alias PFILTER pattern _ op [fun] implicit parameters currenttuple, lasttuple types TUPLE, TUPLE

operator extendX alias EXTENDX pattern _ op[list ; fun ; _] implicit parameter tuple type TUPLE

