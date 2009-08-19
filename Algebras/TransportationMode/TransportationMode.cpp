/*
----
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
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Source File of the Spatiotemporal Pattern Algebra

August, 2009 Jianqiu Xu

[TOC]

1 Overview

This source file essentially contains the necessary implementations for
queries moving objects with transportation modes.

2 Defines and includes

*/

#include "TransportationMode.h"

extern NestedList* nl;
extern QueryProcessor *qp;

namespace TransportationMode{

/*
Subclass for manage bus network

*/
class BusNetwork{


};

/*
Main Class for Transportation Mode

*/
class TransportationModeAlgebra : public Algebra
{
 public:
  TransportationModeAlgebra() : Algebra()
  {

  }
  ~TransportationModeAlgebra() {};
};

};


extern "C"
Algebra*
InitializeTransportationModeAlgebra( NestedList* nlRef,
    QueryProcessor* qpRef )
    {
  // The C++ scope-operator :: must be used to qualify the full name
  return new TransportationMode::TransportationModeAlgebra();
    }
