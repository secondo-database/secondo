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

1.1 Declaration of Operator deftime

The operator returns the periods of time in which the given moving gpoint is
defined.

March 2008 Simone Jandt

Defines, includes, and constants

*/

#ifndef OPTEMPNETDEFTIME_H_
#define OPTEMPNETDEFTIME_H_

class OpTempNetDeftime
{
public:
/*
Type Mapping of operator ~deftime~

*/
static ListExpr TypeMap(ListExpr args);

/*
Value mapping function of operator ~deftime~

*/
static int ValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s );

/*
Specification of operator ~deftime~

*/
static const string Spec;

};

#endif /*OPTEMPNETDEFTIME_H_*/
