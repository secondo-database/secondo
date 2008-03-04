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

1.1 Declaration of Operator initial

This operator reduces a moving gpoint to the initial instant.

March 2008 Simone Jandt

*/

#ifndef OPTEMPNETINITIAL_H_
#define OPTEMPNETINITIAL_H_

class OpTempNetInitial
{
public:
/*

Type Mapping of operator ~atinstant~

*/
static ListExpr TypeMap(ListExpr args);

/*
4.4.3 Specification of operator ~atinstant~

*/
static const string Spec;
};

#endif /*OPTEMPNETINITIAL_H_*/
