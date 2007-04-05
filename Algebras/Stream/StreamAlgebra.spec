#This file is part of SECONDO.

#Copyright (C) 2006, University in Hagen, Department of Computer Science,
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


operator transformstream alias TRANSFORMSTREAM pattern _ op
operator projecttransformstream alias PROJECTTRANSFORMSTREAM pattern _ op [_]
operator namedtransformstream alias NAMEDTRANSFORMSTREAM pattern _ op [_]
operator printstream alias PRINTSTREAM pattern _ op
operator use alias USE pattern _ op [ _ ] implicit parameter streamelem type STREAMELEM
operator use2 alias USE2 pattern _ _ op [ _ ] implicit parameters streamelem1, streamelem2 types STREAMELEM, STREAMELEM2
operator aggregateS alias AGGREGATES pattern _ op [ _ ; _ ] implicit parameter streamelem type STREAMELEM

# Operator signatures already defined elsewhere:
#operator feed alias FEED pattern op ( _ )
#operator filter alias FILTER pattern _ op [ fun ] implicit parameter streamelem type STREAMELEM
#operator count alias COUNT pattern _ op

