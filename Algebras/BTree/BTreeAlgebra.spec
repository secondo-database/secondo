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

operator createbtree alias CREATEBTREE pattern _ op [ _ ]
operator leftrange alias LEFTRANGE pattern _ _ op [ _ ]
operator rightrange alias RIGHTRANGE pattern _ _ op [ _ ]
operator range alias RANGE pattern _ _ op [ _, _ ]
operator insertbtree alias INSERTBTREE pattern _ _ op [ _ ]
operator deletebtree alias DELETEBTREE pattern _ _ op [ _ ]
operator updatebtree alias UPDATEBTREE pattern _ _ op [ _ ]
operator exactmatch alias EXACTMATCH pattern _ _ op [ _ ]
operator exactmatchS alias EXACTMATCHS pattern _ op [ _ ]
operator leftrangeS alias LEFTRANGES pattern _ op [ _ ]
operator rightrangeS alias RIGHTRANGES pattern _ op [ _ ]
operator rangeS alias RANGES pattern _ op [ _, _ ]
