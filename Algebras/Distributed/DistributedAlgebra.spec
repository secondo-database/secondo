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

# Begin DestributedAlgebra.spec


operator test alias TEST pattern _ op

operator makeDarray alias MAKEDARRAY pattern op (_, _ )

operator get alias GET pattern op ( _, _ )

operator put alias PUT pattern op ( _, _, _ )

operator sendD alias SENDD pattern op ( _, _ ,_)

operator receiveD alias RECEIVED pattern op ( _ , _ )

operator d_receive_rel alias D_RECEIVE_REL pattern op (_, _)

operator d_send_rel alias D_SEND_REL pattern op (_, _, _)

operator d_send_shuffle alias D_SEND_SHUFFLE pattern _ op [fun, _, _] implicit parameter tuple type TUPLE

operator d_receive_shuffle alias D_RECEIVE_shuffle pattern op (_, _)

operator ddistribute alias DDISTRIBUTE pattern _ op [_, _]

operator dshuffle alias DSHUFFLE pattern _ op [fun, _, _] 
         implicit parameter tuple type DRELATION

operator dshuffle2 alias DSHUFFLE2 pattern _ op [fun, _] 
         implicit parameter tuple type DRELATION

operator dshuffle1 alias DSHUFFLE1 pattern _ op [fun] 
         implicit parameter tuple type DRELATION

operator d_idx alias DAINDEX pattern op ()

operator dloop alias DLOOP pattern _ op [ fun ] implicit parameter element type DELEMENT

operator dloopa alias DLOOPA pattern _ _ op [ fun ] 
         implicit parameters first, second types DELEMENT, DELEMENT2

operator dtie alias DTIE pattern _ op [ fun ] 
         implicit parameters first, second types DELEMENT, DELEMENT 

operator dsummarize alias DSUMMARIZE pattern _ op

operator check_workers alias CHECKWORKERS pattern _ op

operator startup alias STARTUP pattern op (_, _, _)

operator shutdown alias SHUTDOWN pattern op (_, _)

# End DistributedAlgebra.spec
