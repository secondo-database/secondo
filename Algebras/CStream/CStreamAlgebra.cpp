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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of datatype vstream and operators.

[toc]

1 CStream class implementation

*/

#include "CStreamAlgebra.h"
#include "TupleDescr.h"



namespace cstream {

extern TypeConstructor TupleDescrTC;
extern Operator testop_Op;
//extern Operator printvtstream_Op;
extern TypeConstructor VTupleTC;
extern Operator transformtstream_Op;
extern Operator transformvtstream_Op;
extern Operator receiveStream_Op;
extern Operator provideTupleTypes_Op;
extern Operator requestTupleTypes_Op;
extern Operator registerForStream_Op;
extern Operator distributeStream_Op;
extern Operator count_Op;
extern Operator head_Op;
extern Operator printstream_Op;

extern Operator TTYPE3_Op;

CStreamAlgebra::CStreamAlgebra() {
    AddTypeConstructor(&TupleDescrTC);
    TupleDescrTC.AssociateKind(Kind::DATA());

    AddTypeConstructor(&VTupleTC);
    TupleDescrTC.AssociateKind(Kind::SIMPLE());

    AddOperator(&testop_Op);
    AddOperator(&count_Op);
    AddOperator(&printstream_Op);
    AddOperator(&head_Op);
    AddOperator(&transformtstream_Op);
    AddOperator(&transformvtstream_Op);
    transformvtstream_Op.SetUsesArgsInTypeMapping();
    //AddOperator(&printvtstream_Op);

    AddOperator(&receiveStream_Op);  
    receiveStream_Op.SetUsesArgsInTypeMapping();   
    AddOperator(&provideTupleTypes_Op);
    provideTupleTypes_Op.SetUsesArgsInTypeMapping();
    AddOperator(&requestTupleTypes_Op);
    requestTupleTypes_Op.SetUsesArgsInTypeMapping();
    AddOperator(&registerForStream_Op);
    registerForStream_Op.SetUsesArgsInTypeMapping();
    AddOperator(&distributeStream_Op);
    distributeStream_Op.SetUsesArgsInTypeMapping();

    AddOperator(&TTYPE3_Op);
    TTYPE3_Op.SetUsesArgsInTypeMapping();

}

extern "C" 
Algebra* InitializeCStreamAlgebra(NestedList* nlRef, QueryProcessor* qpRef) {
    return new CStreamAlgebra();
}

} /* namespace cstream */
