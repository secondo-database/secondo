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

operator tiltedtime alias TILTED pattern _op[_,_]

operator createbloomfilter alias BLOOM pattern _ op[_,_]

operator bloomcontains alias BLOOMC pattern _op[_]

operator createcountmin alias CMS pattern _op[_,_,_]

operator cmscount alias CMSCOUNT pattern _op[_]

operator createams alias AMS pattern _op[_,_,_]

operator amsestimate alias AMSESTIMATE pattern _op

operator createlossycounter alias LC pattern _op[_,_]

operator lcfrequent alias FREQ pattern _op[_]

operator outlier alias OUTLIER pattern _op[_,_]

operator streamcluster alias CLUSTER pattern _op[_,_,_]

operator pointgen alias POINTGEN pattern op(_)

operator stringgen alias STRINGGEN pattern op(_,_)

operator intgen alias INTGEN pattern op(_,_)

operator realgen alias REALGEN pattern op(_,_,_,_)

operator massquerybloom alias MASSBLOOM pattern _op[_,_]

operator inttuplegen alias INTTUPLEGEN pattern op(_)

operator stringtuplegen alias STRINGTUPLEGEN pattern op(_,_)

operator bloomfalsepositive alias BLOOMFALSEPOSITIVE pattern op(_,_,_,_)

operator geometricdist alias GEOMETRICDIST pattern op(_,_)

operator uniformdist alias UNIFORMDIST pattern op(_,_,_)

operator normaldist alias NORMALDIST pattern op (_,_,_)

operator normaldistreal alias NORMALDISTREAL pattern op (_,_,_)

operator distinctcount alias DISTINCTCOUNT pattern _op[_]

operator cmsoverreport alias CMSOVERREPORT pattern _op[_,_]

operator switchingdist alias SWITCHINGDIST pattern op(_,_,_,_)

operator samplegen alias SAMPLEGEN pattern op(_,_)

operator lossycompare alias LOSSYCOMPARE pattern _ _op[_,_,_]

operator empiricaldist alias EMPIRICALDIST pattern _op[_]
