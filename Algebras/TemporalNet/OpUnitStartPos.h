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

1.1 Declaration of Operator ~unitstartpos~

Returns the startpos of a given ugpoint.

June 2008 Simone Jandt

*/

#ifndef OPUNITSTARTPOS_H_
#define OPUNITSTARTPOS_H_

class OpUnitStartPos
{
public:
/*

Type Mapping of operator ~startpos~

*/
static ListExpr TypeMap(ListExpr args);

/*
Value mapping function of operator ~startpos~

*/
static int ValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s );


/*
4.4.3 Specification of operator ~startpos~

*/
static const string Spec;
};

#endif /*OPUNITSTARTPOS_H_*/
