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

operator contains alias CONTAINS pattern _ infixop _
operator in alias IN pattern _ infixop _
operator insert alias INSERT pattern _ _ op
operator + alias ADD pattern _ infixop _
operator create_vector alias CREATE_VECTOR pattern op (_, _)
operator create_set alias CREATE_SET pattern op (_, _)
operator create_multiset alias CREATE_MULTISET pattern op (_, _)
operator collect_set alias COLLET_SET pattern _ op
operator collect_multiset alias COLLECT_MULTISET pattern _ op
operator collect_vector alias COLLECT_VECTOR pattern _ op
operator components alias COMPONENTS pattern op (_)
operator get alias GET pattern op ( _, _ )
operator deleteelem alias DELETE pattern op ( _, _ )
operator concat alias CONCAT pattern _ _ op
operator union alias UNION pattern _ infixop _
operator intersection alias INTERSECTION pattern op ( _, _ )
operator difference alias DIFFERENCE pattern op ( _, _ )
operator > alias GT pattern _ infixop _
operator < alias LT pattern _ infixop _
operator <= alias LE pattern _ infixop _
operator >= alias GE pattern _ infixop _
operator = alias EQ pattern _ infixop _
operator # alias NE pattern _ infixop _
operator size alias SIZE pattern op ( _ )
operator feedIS alias FEEDIS pattern _ op
operator collect_intset alias COLLECT_INTSET pattern _ op [_] 
operator sdiff alias SDIFF pattern _ infixop _ 

