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

operator tsmotifbf alias TSMOTIFBF pattern _ op [ _, _ ]

operator tsmotifbffun alias TSMOTIFBFFUN pattern _ op [ _, _, fun ] implicit parameters lefttuple, righttuple types TUPLE, TUPLE2 !!

operator tsdistance alias TSDISTANCE pattern op(_, _)

operator tsdistancefun alias TSDISTANCEFUN pattern _  _ op [ fun ] implicit parameters lefttuple, righttuple types TUPLE, TUPLE2 !!

operator tsdtw alias TSDTW pattern op(_,_)

operator tsdif alias TSDIF pattern _ op[_]

operator tswhitenoise alias TSWHITENOISE pattern _ op[_, _]

operator predictma alias PREDICTMA pattern _ op[_ ,_]

operator predictar alias PREDICTAR pattern _ op[_, _]

operator predictarma alias PREDICTARMA pattern _op[_, _, _]

operator predictarima alias PREDICTARIMA pattern _ op[_, _, _, _]

operator ddtw alias DDTW pattern op(_,_)

operator paa alias PAA pattern _ op[_]

operator pacf alias PACF pattern _ op[_]

operator acf alias ACF pattern _ op[_]

operator dtwsc alias DTWSC pattern op(_, _, _)

operator motifmk alias MOTIFMK pattern _ op[_]
