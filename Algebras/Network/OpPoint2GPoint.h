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

1.1 Declaration of Operator Point2GPoint

The operator translates a ~point~ into a network based ~gpoints~

February 2008 Simone Jandt

Defines, includes, and constants

*/

#ifndef OPPOINT2GPOINT_H_
#define OPPOINT2GPOINT_H_

class OpPoint2GPoint
{
public:
/*
Type Mapping of operator ~point2gpoint~

*/
static ListExpr TypeMap(ListExpr args);

/*
Value mapping function of operator ~point2gpoint~

*/
static int ValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s );

/*
Specification of operator ~point2gpoint~

*/
static const string Spec;

private:

/*
Send a message to all clients

*/
static void sendMessage(string in_strMessage);

};

#endif /*OPPOINT2GPOINT_H_*/
