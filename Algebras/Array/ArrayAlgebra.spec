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

# Begin ArrayAlgebra.spec


operator makearray alias MAKEARRAY pattern op ( _, _ )
# Note: ( _, _ ) means also a comma separated list of arbitrary length 

operator get alias GET pattern op ( _, _ )

operator put alias PUT pattern op ( _, _, _ )

operator size alias SIZE pattern op ( _ )

operator sortarray alias SORTARRAY pattern _ op [ fun ]
         implicit parameter element type ELEMENT

operator tie alias TIE pattern _ op [ fun ] 
         implicit parameters first, second types ELEMENT, ELEMENT

operator cumulate alias CUMULATE pattern _ op [ fun ] 
         implicit parameter first type ELEMENT

operator distribute alias DISTRIBUTE pattern _ op [ _ ]

operator summarize alias SUMMARIZE pattern _ op

operator loop alias LOOP pattern _ op [ fun ]
         implicit parameter element type ELEMENT

operator loopa alias LOOPA pattern _ _ op [ fun ] 
         implicit parameters first, second types ELEMENT, ELEMENT2

operator loopb alias LOOPB pattern _ _ op [ fun ] 
         implicit parameters first, second types ELEMENT, ELEMENT2

operator loopswitch alias LOOPSWITCH pattern _ op [ funlist ] 
         implicit parameter element type ELEMENT

operator loopswitcha alias LOOPSWITCHA pattern _ _ op [ funlist ]

operator loopswitchb alias LOOPSWITCHB pattern _ _ op [ funlist ]

operator loopselect alias LOOPSELECT pattern _ op [ funlist; _, _ ] 
         implicit parameter element type ELEMENT

operator loopselecta alias LOOPSELECTA pattern _ _ op [ funlist; _, _ ]

operator loopselectb alias LOOPSELECTB pattern _ _ op [ funlist; _, _ ]

operator partjoin alias PARTJOIN pattern _ _ op [ fun ] 
         implicit parameters first, second types ELEMENT, ELEMENT2

operator partjoinswitch alias PARTJOINSWITCH pattern _ _ op [ funlist ]

operator partjoinselect alias PARTJOINSELECT pattern _ _ op [ funlist; _, _ ]

# End ArrayAlgebra.spec
