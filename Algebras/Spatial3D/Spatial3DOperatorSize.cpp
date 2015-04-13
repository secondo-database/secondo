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
 
 01590 Fachpraktikum "Erweiterbare Datenbanksysteme" 
 WS 2014 / 2015

<our names here>

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of a Spatial3D algebra

[TOC]

1 Includes and Defines

*/

#include "Spatial3D.h"
#include "RelationAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace std;

namespace spatial3DOperatorSize {
  
  /*

  11 Operator ~size~

  Computes the number of triangles of a 3D object (surface3d or volume3d)

  11.1 Type mapping function for operator ~size~

  ----
    geo3d --> int
  
    (For the representation of a geo3d the data structure TransformObject
      is used)
  ----

  */
  ListExpr
  Spatial3dSizeMap( ListExpr args )
  {
    string errmsg = "Expected a surface3d, or a volume3d.";
  
    if (!nl->HasLength(args, 1)){
      return listutils::typeError(errmsg + " (wrong number of arguments)");
    }
  
    if( !Surface3d::checkType(nl->First(args)) &&
      !Volume3d::checkType(nl->First(args)) ){
    return listutils::typeError(errmsg);
    }
    
    return ( nl->SymbolAtom( CcInt::BasicType() ));
  }

  /*

  11.2 Value mapping function of operator ~size~

  */
  int
  Spatial3dSize( Word* args, Word& result, int message,
                 Word& local, Supplier s )
  {
    result = qp->ResultStorage( s );
    CcInt* res = static_cast<CcInt*>(result.addr);
    TriangleContainer*  c = static_cast<TriangleContainer*>(args[0].addr);
  
    if(!c->IsDefined()){
      res->SetDefined(false);
      return 0;
    }
  
    res->Set(true,c->size());
    return 0;
  }
  
  /*

  11.3 Specification of operator ~size~

  */

  OperatorSpec sizeSpec(
    "geo3d --> int",
    "size()",
    "Computes the number of triangles of a 3D object (surface3d or volume3d).",
    "query size([const surface3d value (((0 0 0) (0 1 0) (1 1 0)) "
      "((1 1 0) (0 1 0) (1 1 2)))])"
  );
  
  /*

  11.4 Definition of operator ~size~

  */
  Operator* getSizePtr(){
    return new Operator(
    "size",
    sizeSpec.getStr(),
    Spatial3dSize,
    Operator::SimpleSelect,
    Spatial3dSizeMap
    );
  }
  
}  // end of namespace spatial3DOperatorSize
  
