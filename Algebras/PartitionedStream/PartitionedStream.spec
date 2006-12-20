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

operator shuffle alias SHUFFLE pattern _ op
operator memshuffle alias MEMSHUFFLE pattern _ op

operator pfeed alias PFEED pattern _ op [ _ ]

operator pcreate alias PCREATE pattern _ op [ _ ]
operator pcreate2 alias PCREATE2 pattern _ op [ _, _ ]

operator pdelete alias PDELETE pattern _ op

operator pshow alias PSHOW pattern _ op

operator puse alias PUSE pattern _ op [ fun ] 
         implicit parameter element type PSTREAM1 

operator pjoin2 alias PJOIN2 pattern _ _ op [ funlist ]
         implicit parameters left, right types PSTREAM1, PSTREAM2

operator pjoin1 alias PJOIN1 pattern _ _ op [ _; funlist ]
         implicit parameters left, right types PSTREAM1, ANY2



#operator loopa alias LOOPA pattern _ _ op [ fun ] 
#         implicit parameters first, second types ELEMENT, ELEMENT2

#operator loopswitcha alias LOOPSWITCHA pattern _ _ op [ funlist; _ ]
#

#operator loopselect alias LOOPSELECT pattern _ op [ funlist; _, _ ] 
#         implicit parameter element type ELEMENT

#operator loopselecta alias LOOPSELECTA pattern _ _ op [ funlist; _, _ ]


#operator partjoin alias PARTJOIN pattern _ _ op [ fun ] 
#         implicit parameters first, second types ELEMENT, ELEMENT2

#operator partjoinswitch alias PARTJOINSWITCH pattern _ _ op [ funlist ]

#operator partjoinselect alias PARTJOINSELECT pattern _ _ op [ funlist; _, _ ]

