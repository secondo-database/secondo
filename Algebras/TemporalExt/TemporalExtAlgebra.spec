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

operator atinstantext alias ATINSTANTEXT pattern _ infixop _
operator atperiodsext alias ATPERIODSEXT pattern _ infixop _
operator presentext alias PRESENTEXT pattern _ infixop _
operator passesext alias PASSESEXT pattern _ infixop _
operator initialext alias INITIALEXT pattern op ( _ )
operator finalext alias FINALEXT pattern op ( _ )
operator atext alias ATEXT pattern _ infixop _
operator deftimeext alias DEFTIMEEXT pattern op ( _ )
operator instext alias INSTEXT pattern op ( _ )
operator valext alias VALEXT pattern op ( _ )
operator derivativeext alias DERIVATIVEEXT pattern op ( _ )
operator derivableext alias DERIVABLEEXT pattern op ( _ )
operator speedext alias SPEEDEXT pattern op ( _ )
