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

Declaration of operator sections

Mai-Oktober 2007 Martin Scheppokat

Parts of the source taken from Victor Almeida

Defines, includes, and constants

*/
#ifndef OPNETWORKSECTIONS_H_
#define OPNETWORKSECTIONS_H_

class OpNetworkSections
{
public:
/*
Type Mapping of operator ~sections~

*/
static ListExpr TypeMap(ListExpr args);

/*
Value mapping function of operator ~sections~

*/
static int ValueMapping( Word* args, Word& result, int message, 
                             Word& local, Supplier s );

/*
Specification of operator ~sections~

*/
static const string Spec;
};

#endif /*OPNETWORKSECTIONS_H_*/
