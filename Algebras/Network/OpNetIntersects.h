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

1.1 Declaration of Operator intresects

Returns true if two glines intersect false elsewhere.

June 2008 Simone Jandt

*/

#ifndef OPNETINTERSECTS_H_
#define OPNETINTERSECTS_H_

class OpNetIntersects
{
public:
/*

Type Mapping of operator ~intersects~

*/
static ListExpr TypeMap(ListExpr args);

/*
Value mapping function of operator ~intersects~

*/
static int ValueMapping(Word* args, Word& result, int message, Word& local,
                        Supplier in_xSupplier);


/*
4.4.3 Specification of operator ~intersects~

*/
static const string Spec;
};

#endif /*OPNETINTERSECTS_H_*/
