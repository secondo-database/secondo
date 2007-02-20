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

operator count alias COUNT pattern _ op
operator count2 alias COUNT2 pattern _ op
operator roottuplesize alias ROOTTUPLESIZE pattern _ op
operator exttuplesize alias EXTTUPLESIZE pattern _ op
operator tuplesize alias TUPLESIZE pattern _ op
operator rootattrsize alias ROOTATTRSIZE pattern _ op[ _ ]
operator extattrsize alias EXTATTRSIZE pattern _ op[ _ ]
operator attrsize alias ATTRSIZE pattern _ op[ _ ]
operator sizecounters alias SIZECOUNTERS pattern _ op[ _ ]
operator dumpstream alias DUMPSTREAM pattern _ op[ _ ]
operator feed alias FEED pattern _ op
operator consume alias CONSUME pattern _ op
operator attr alias ATTR pattern op (_, _)
operator project alias PROJECT pattern _ op [list]
operator filter alias FILTER pattern _ op [ fun ] implicit parameter tuple type TUPLE
operator reduce alias REDUCE pattern _ op [ fun, _ ] implicit parameter tuple type TUPLE
operator product alias PRODUCT pattern _ _ op
operator rename alias RENAME pattern _ op [ _ ]
operator mconsume alias MCONSUME pattern _ op
