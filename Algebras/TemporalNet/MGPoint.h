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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Definition of MGPoint in Module TemporalNet

Mai-Oktober 2007 Martin Scheppokat

[TOC]
 
1 Overview


This file contains the implementation of ~gline~


2 Defines, includes, and constants

*/
#ifndef MGPOINT_H_
#define MGPOINT_H_

#ifndef _TEMPORAL_ALGEBRA_H_
#error TemporalAlgebra.h is needed by MGPoint.h. \
Please include in *.cpp-File.
#endif

#ifndef UGPOINT_H_
#error UGPoint.h is needed by MGPoint.h. \
Please include in *.cpp-File.
#endif




/*
3.12 Class ~MGPoint~

*/
class MGPoint : public Mapping< UGPoint, GPoint >
{ 
  public:
/*
3.12.1 Constructors and Destructor

*/
/*
The simple constructor. This constructor should not be used.

*/
    MGPoint();
    
/*
The constructor. Initializes space for ~n~ elements.

*/
    MGPoint( const int n );

    static ListExpr Property();

/*
4.12.3 Kind Checking Function

*/
    static bool Check(ListExpr type, 
                      ListExpr& errorInfo);


};

#endif /*MGPOINT_H_*/
