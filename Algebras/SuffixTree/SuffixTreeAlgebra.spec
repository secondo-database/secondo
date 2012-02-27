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


operator createsuffixtree alias CREATESUFFIXTREE pattern op ( _ )

operator createsuffixtree_quadratic alias CREATESUFFIXTREE_QUADRATIC pattern op ( _ )

operator patternoccurs alias PATTERNOCCURS pattern _ infixop _

operator patternpositions alias PATTERNPOSITIONS pattern op(_ , _)

operator patterncount alias PATTERNCOUNT pattern _ infixop _

operator longestrepeatedsubstring alias LONGESTREPEATEDSUBSTRING 
pattern op ( _ )

operator shortestuniquesubstring alias SHORTESTUNIQUESUBSTRING pattern op ( _ )

operator longestcommonsubstring alias LONGESTCOMMONSUBSTRING pattern op ( _, _ )

operator maximaluniquematches alias MAXIMALUNIQUEMATCHES pattern op ( _, _ )

operator circularstringlinearization alias CIRCULARSTRINGLINEARIZATION 
pattern op ( _ )

operator kmismatch alias KMISMATCH pattern op ( _, _, _ )

operator = alias GLEICH pattern _ infixop _


operator patternFilter alias PATTERNFILTER pattern _ op [_,_]

