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

operator reservoir alias RESERVOIR pattern _ op[_]

operator tilted alias TILTED pattern _ op[_]

operator createbloomfilter alias BLOOM pattern _ op[_,_]

operator bloomcontains alias BLOOMC pattern _op[_]

operator createcountmin alias CMS pattern _op[_,_,_]

operator cmscount alias CMSCOUNT pattern _op[_]

operator createams alias AMS pattern _op[_,_,_]

operator amsestimate alias AMSESTIMATE pattern _op

operator createlossycounter alias LC pattern _op[_,_]

operator lcfrequent alias FREQ pattern _op[_]

operator outlier alias OUTLIER pattern _op[_,_]
