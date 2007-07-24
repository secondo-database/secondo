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

[1] Implementation of GLine in Module Network Algebra

March 2004 Victor Almeida

Mai-Oktober 2007 Martin Scheppokat

[TOC]

1 Overview


This file contains the implementation of ~gline~


2 Defines, includes, and constants

*/
#ifndef OPSHORTESTPATH_H_
#define OPSHORTESTPATH_H_

class OpShortestPath
{
  public:
  
/*
4.1.2 Typemap function of the operator

*/
  static ListExpr TypeMap(ListExpr args);

/*
4.1.2 Value mapping function of the operator

*/
  static int ValueMapping( Word* in_pArgs, 
                           Word& in_xResult, 
                           int in_iMessage, 
                           Word& in_xLocal, 
                           Supplier in_xSupplier );


/*
4.1.3 Specification of the operator

*/
  static const string Spec;    
};

#endif /*OPSHORTESTPATH_H_*/
