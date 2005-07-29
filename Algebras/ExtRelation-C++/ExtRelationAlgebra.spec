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

operator remove alias REMOVE pattern _ op [list]
operator head alias HEAD pattern _ op [ _ ]

operator max alias MAX pattern _ op [ _ ]
operator min alias MIN pattern _ op [ _ ]
operator avg alias AVG pattern _ op [ _ ]
operator sum alias SUM pattern _ op [ _ ]

operator concat alias CONCAT pattern _ _ op
operator cancel alias CANCEL pattern _ op [ fun ] implicit parameter tuple type TUPLE
operator rdup alias RDUP pattern _ op
operator sort alias SORT pattern _ op
operator extend alias EXTEND pattern _ op [ funlist ] implicit parameter tuple type TUPLE
operator extendstream alias EXTENDSTREAM pattern _ op [funlist] implicit parameter tuple type TUPLE
operator projectextendstream alias PROJECTEXTENDSTREAM pattern _ op [list; funlist] implicit parameter tuple type TUPLE

operator groupby alias GROUPBY pattern _ op [list; funlist] implicit parameter group type GROUP
operator mergesec alias MERGESEC pattern _ _ op
operator mergediff alias MERGEDIFF pattern _ _ op
operator mergeunion alias MERGEUNION pattern _ _ op

operator sortby alias SORTBY pattern _ op [list]
operator mergejoin alias MERGEJOIN pattern _ _ op [_, _]
operator sortmergejoin alias SORTMERGEJOIN pattern _ _ op [_, _]
operator oldhashjoin alias OLDHASHJOIN pattern _ _ op [_, _, _]
operator hashjoin alias HASHJOIN pattern _ _ op [_, _, _]

operator loopjoin alias LOOPJOIN pattern _ op [ fun ] implicit parameter tuple type TUPLE
operator loopsel alias LOOPSEL pattern _ op [ fun ] implicit parameter tuple type TUPLE

operator extract alias EXTRACT pattern _ op [ _ ]

operator sample alias SAMPLEFEED pattern _ op [_, _]

operator aggregate alias AGGREGATE pattern _ op [ _; fun; _ ] implicit parameter tuple type TUPLE

operator symmproduct alias SYMMPRODUCT pattern _ _ op [ fun ] implicit parameters lefttuple, righttuple types TUPLE, TUPLE2

operator createinsertrel alias CREATEINSERTREL pattern _ op

operator createdeleterel alias CREATEDELETEREL pattern _ op

operator createupdaterel alias CREATEUPDATEREL pattern _ op

operator insert alias INSERT pattern _ _ op

operator insertsave alias INSERTSAVE pattern _ _ _ op

operator inserttuple alias INSERTTUPLE pattern _ op [list]

operator inserttuplesave alias INSERTTUPLESAVE pattern _ _ op [list]

operator deletedirect alias DELETEDIRECT pattern _ _ op

operator deletesearch alias DELETESEARCH pattern _ _ op

operator deletesearchsave alias DELETESEARCHSAVE pattern _ _ _ op

operator deletedirectsave alias DELETEDIRECTSAVE pattern _ _ _ op

operator updatesearch alias UPDATESEARCH pattern _ _ op [funlist]implicit parameter tuple type TUPLE

operator updatedirect alias UPDATEDIRECT pattern _ _ op [funlist]implicit parameter tuple type TUPLE

operator updatesearchsave alias UPDATESEARCHSAVE pattern _ _ _ op [funlist]implicit parameter tuple type TUPLE

operator updatedirectsave alias UPDATEDIRECTSAVE pattern _ _ _ op [funlist]implicit parameter tuple type TUPLE

operator appendidentifier alias APPENDIDENTIFIER pattern _ op

operator deletebyid alias DELETEBYID pattern _ op [ _ ]

operator updatebyid alias UPDATEBYID pattern _ _ op [ _; funlist]implicit parameter tuple type TUPLE












