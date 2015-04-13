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

namespace spatial3DOperatorBBox {
  
  /*

  12 Operator ~bbox3d~

  Computes the bounding box of a 3D object (represented by a TransformObject,
  i.e. a Point3d, a surface3d or a volume3d).

  12.1 Type mapping function for operator ~bbox3d~

  ----
    geo3d --> rect3
  ----

  */

  ListExpr
  Spatial3dBBoxMap( ListExpr args )
  {
    string errmsg = "Expected a point3d, or a surface3d, or a volume3d.";
  
    if (!nl->HasLength(args, 1)){
      return listutils::typeError(errmsg + " (wrong number of arguments)");
    }
  
    if( !Point3d::checkType(nl->First(args)) &&
        !Surface3d::checkType(nl->First(args)) &&
        !Volume3d::checkType(nl->First(args)) ){
      return listutils::typeError(errmsg);
    }
    
    return ( nl->SymbolAtom( Rectangle<3>::BasicType() ));
  }

  /*

  12.2 Selection function of operator ~bbox3d~

  */

  int
  Spatial3dBBoxSelect( ListExpr args)
  {
    if( Point3d::checkType(nl->First(args)) )
      return 0;
  
    if( Surface3d::checkType(nl->First(args)) || 
         Volume3d::checkType(nl->First(args)) )
      return 1;
  
    return -1;  //This point should never be reached
  }  


  /* 
  12.3 Value mapping functions of operator ~bbox3d~

  */
  int
  SpatialPoint3dBBox(Word* args, Word& result, int message,
              Word& local, Supplier s ){

    result = qp->ResultStorage( s );
    Rectangle<3>* box = static_cast<Rectangle<3>* >(result.addr);
    Point3d* p = static_cast<Point3d*>(args[0].addr);
  
    if( !p->IsDefined() ){
      box->SetDefined(false);
      return 0;
    }
  
    (*box) = p->BoundingBox(0);
    return 0;
  }

  int
  SpatialSurVol3dBBox(Word* args, Word& result, int message,
              Word& local, Supplier s ){

    result = qp->ResultStorage( s );
    Rectangle<3>* box = static_cast<Rectangle<3>* >(result.addr);
    TriangleContainer* tc = static_cast<TriangleContainer*>(args[0].addr);
  
    if( !tc->IsDefined() ){
      box->SetDefined(false);
      return 0;
    }
  
    (*box) = tc->BoundingBox(0);
    return 0;
  }

  /*

  12.4 Specification of operator ~bbox3d~

  */

  OperatorSpec bbox3dSpec(
    "geo3d -> rect3",
    "bbox3d()",
    "Computes the Bounding Box of a 3d object.",
    "query bbox3d([const surface3d value(((0 0 0) (0 1 0) (1 1 0)) "
      "((1 1 0) (0 1 0) (1 1 2)))])"
  );

  /*

  12.5 Instance of operator ~bbox3d~

  */

  ValueMapping bbox3dVM[] = { SpatialPoint3dBBox, SpatialSurVol3dBBox, 0 };

  Operator* getBBoxPtr(){
    return new Operator(
    "bbox3d",
    bbox3dSpec.getStr(),
    2,
    bbox3dVM,
    Spatial3dBBoxSelect,
    Spatial3dBBoxMap
    );
  }
  
}  // end of namespace spatial3DOperatorBBox
  
