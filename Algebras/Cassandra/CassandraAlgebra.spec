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


operator sleep alias SLEEP pattern _ op [ _ ]
operator statistics alias STATISTICS pattern _ op [ _ , _ ]


operator cdelete alias CDELETE pattern op ( _ , _ , _ )
operator clist alias CLIST pattern op ( _ , _ )
operator cspread alias CSPREAD pattern _ op [ _ , _ , _ , _ , _ , _ ]
operator ccollect alias CCOLLECT pattern op ( _ , _ , _ , _ )
operator ccollectlocal alias CCOLLECTLOCAL pattern op ( _ , _ , _ , _ )
operator ccollectrange alias CCOLLECTRANGE pattern op ( _ , _ , _ , _ , _ , _ )

operator cquerylist alias CQUERYLIST pattern op ( _ , _ )
operator cqueryinsert alias CQUERYINSERT pattern op ( _ , _ , _ , _ )
operator cquerywait alias CQUERYWAIT pattern op ( _ , _ , _ )
operator cqueryreset alias CQUERYRESET pattern op ( _ , _ )

