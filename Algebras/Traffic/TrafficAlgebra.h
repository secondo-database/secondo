/*
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]

October 2009- Simone Jandt

1 Declarations Necessary for Traffic Algebra

*/
#ifndef __TRAFFIC_ALGEBRA_H__
#define __TRAFFIC_ALGEBRA_H__

/*
Enumeration of the coloums of the resulting relation of trafficflow operations.

*/

enum TrafficFlowRelation{TRAFFICFLOW_SECID = 0,
                         TRAFFICFLOW_PARTNO,
                         TRAFFICFLOW_DIR,
                         TRAFFICFLOW_FLOW};

enum TrafficJamRelation{TRAFFICJAM_SECID = 0,
                        TRAFFICJAM_PARTNO,
                        TRAFFICJAM_DIR,
                        TRAFFICJAM_SPEED,
                        TRAFFICJAM_FLOW};


#endif // __TRAFFIC_ALGEBRA_H__
