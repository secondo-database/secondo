/*
----
This file is part of SECONDO.

Copyright (C) 2019,
University in Hagen,
Faculty of Mathematics and Computer Science,
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

[1] Implementation of the SPart Algebra

April, 2020. Markus Zajac

1 Overview

This implementation file essentially contains the implementation of the type
~irgrid2d~, and the definitions of the type constructur ~irgrid2d~
with its associated operations.

2 Defines and Includes

*/
#include "IrregularGrid2D.h"
#include "IrregularGrid3D.h"
#include "KDTree2D.h"
#include "KDTree3D.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

extern NestedList* nl;
extern QueryProcessor *qp;

#include "TypeMapUtils.h"
#include "Symbols.h"

#include <string>
using namespace std;

namespace spart {

/*
3.1 Creation of the type constructor ~irgrid2d~

See also IrregularGrid2D.h

*/
TypeConstructor irgrid2d(
  // name of the type in SECONDO
  IrregularGrid2D::BasicType(),
  // property function describing signature
  IrregularGrid2D::PropertyIrGrid2D,
  // Out and In functions
  IrregularGrid2D::OutIrGrid2D, IrregularGrid2D::InIrGrid2D,
  // SaveToList, RestoreFromList functions
  0, 0,
  // object creation and deletion
  IrregularGrid2D::CreateIrGrid2D, IrregularGrid2D::DeleteIrGrid2D,
  // object open, save
  0, 0,
  // object close and clone
  IrregularGrid2D::CloseIrGrid2D, IrregularGrid2D::CloneIrGrid2D,
  // cast function
  0,
  // sizeof function
  IrregularGrid2D::SizeOfIrGrid2D,
  // kind checking function
  IrregularGrid2D::KindCheckIrGrid2D );


/*
  3.2 Creation of type constructor ~irgrid3d~

*/
TypeConstructor irgrid3d(
  // name of the type in SECONDO
  IrregularGrid3D:: BasicType(),
  // property function describing signature
  IrregularGrid3D::PropertyIrGrid3D,
  // Out and In functions
  IrregularGrid3D::OutIrGrid3D, IrregularGrid3D::InIrGrid3D,
  // SaveToList, RestoreFromList functions
  0,0,
  // object creation and deletion
  IrregularGrid3D::CreateIrGrid3D, IrregularGrid3D::DeleteIrGrid3D,
  // object open, save
  0, 0,
  // object close and clone
  IrregularGrid3D::CloseIrGrid3D, IrregularGrid3D::CloneIrGrid3D,
  // cast function
  0,
  // sizeof function
  IrregularGrid3D::SizeOfIrGrid3D,
  // kind checking function
  IrregularGrid3D::KindCheckIrGrid3D 
);

/*
3.3 Creation of type constructor ~kdtree2d~

*/
TypeConstructor kdtree2d(
  // name of the type in SECONDO
  KDTree2D:: BasicType(),
  // property function describing signature
  KDTree2D::Property2DTree,
  // Out and In functions
  KDTree2D::Out2DTree, KDTree2D::In2DTree,
  // SaveToList, RestoreFromList functions
  0,0,
  // object creation and deletion
  KDTree2D::Create2DTree, KDTree2D::Delete2DTree,
  // object open, save
  0, 0,
  // object close and clone
  KDTree2D::Close2DTree, KDTree2D::Clone2DTree,
  // cast function
  0,
  // sizeof function
  KDTree2D::SizeOf2DTree,
  // kind checking function
  KDTree2D::KindCheck2DTree 
);

/*
3.4 Creation of type constructor ~kdtree3d~

*/
TypeConstructor kdtree3d(
  // name of the type in SECONDO
  KDTree3D:: BasicType(),
  // property function describing signature
  KDTree3D::Property3DTree,
  // Out and In functions
  KDTree3D::Out3DTree, KDTree3D::In3DTree,
  // SaveToList, RestoreFromList functions
  0,0,
  // object creation and deletion
  KDTree3D::Create3DTree, KDTree3D::Delete3DTree,
  // object open, save
  0, 0,
  // object close and clone
  KDTree3D::Close3DTree, KDTree3D::Clone3DTree,
  // cast function
  0,
  // sizeof function
  KDTree3D::SizeOf3DTree,
  // kind checking function
  KDTree3D::KindCheck3DTree 
);

/*
4 Operators

4.1 Type mapping functions

See also IrregularGrid2D.h

*/
// It is used for the ~create\_irgrid2d~ operator.
ListExpr IrGrid2dCreateTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 4)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);
    ListExpr fourth = nl->Fourth(args);

    if (Stream<Rectangle<2>>::checkType(first)
      && Rectangle<2>::checkType(second)
      && CcInt::checkType(third)
      && CcInt::checkType(fourth)) {

       return nl->SymbolAtom(IrregularGrid2D::BasicType());
    }
  }

  const string errMsg = "The following four arguments are expected:"
      " stream(rect) x rect x int x int";

  return  listutils::typeError(errMsg);
}

/*
See also IrregularGrid3D.h

*/

// It is used for the ~create\_irgrid3d~ operator.
ListExpr IrGrid3dCreateTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 5)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);
    ListExpr fourth = nl->Fourth(args);
    ListExpr fifth = nl->Fifth(args);

    if (Stream<Rectangle<3>>::checkType(first)
      && Rectangle<3>::checkType(second)
      && CcInt::checkType(third)
      && CcInt::checkType(fourth)
      && CcInt::checkType(fifth)) {

       return nl->SymbolAtom(IrregularGrid3D::BasicType());
    }
  }

  const string errMsg = "The following five arguments are expected:"
      " stream(rect3) x rect3 x int x int x int";

  return  listutils::typeError(errMsg);
}

/*
 2D-Tree Typemap

*/
ListExpr KDTree2DCreateTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 3)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (Stream<Rectangle<2>>::checkType(first) 
      && Rectangle<2>::checkType(second)
      && CcInt::checkType(third)) {
        return nl->SymbolAtom(KDTree2D::BasicType());
      }
  }
  
  const string errMsg = "The following three arguments are expected:"
      " stream(rect2) x rect2 x int";

  return  listutils::typeError(errMsg);
}

/*
 3D-Tree Typemap

*/
ListExpr KDTree3DCreateTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 3)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);
    if (Stream<Rectangle<3>>::checkType(first) 
      && Rectangle<3>::checkType(second)
      && CcInt::checkType(third)) {
        return nl->SymbolAtom(KDTree3D::BasicType());
      }
  }
  
  const string errMsg = "The following three arguments are expected:"
      " stream(rect3) x rect3 x int";

  return  listutils::typeError(errMsg);
}


/*
4.2 Selection functions

*/
// Is used for the ~create\_irgrid2d~ operator.
int IrGrid2dCreateSelect( ListExpr args )
{
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  ListExpr third = nl->Third(args);
  ListExpr fourth = nl->Fourth(args);

  if (Stream<Rectangle<2>>::checkType(first) && Rectangle<2>::checkType(second)
      && CcInt::checkType(third) && CcInt::checkType(fourth)) {

      return 0;
  }

  return -1; // should never occur
}

// Is used for the ~feed~ operator.
int IrGrid2dFeedSelect( ListExpr args )
{
  if (nl->ListLength(args) == 1) {
    ListExpr first = nl->First(args);

    if (IrregularGrid2D::checkType(first)) {
        return 0;
    }
  }
  return -1; // should never occur
}

// Is used for the ~cellnos\_ir~ operator.
int IrGrid2dCellnosSelect( ListExpr args )
{
  if (nl->ListLength(args) == 2) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if (IrregularGrid2D::checkType(first)
      && Rectangle<2>::checkType(second)) {
        return 0;
    }
  }
  return -1; // should never occur
}

/*
  Smallest Common Cellnumber 2D Select

*/
int IrGrid2dSCCSelect( ListExpr args )
{
  if (nl->ListLength(args) == 4) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);
    ListExpr fourth = nl->Fourth(args);

    if (IrregularGrid2D::checkType(first)
      && Rectangle<2>::checkType(second)
      && Rectangle<2>::checkType(third)
      && CcInt::checkType(fourth)) {
        return 0;
    }
  }
  return -1;
}

/*
  GetCell Select

*/
int IrGrid2dGetCellSelect( ListExpr args )
{
  if (nl->ListLength(args) == 2) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if (IrregularGrid2D::checkType(first)
      && CcInt::checkType(second)) {
        return 0;
    }
  }
  return -1;
}

/* 
  Bbox 2D Select 

*/
int IrGrid2dBBoxSelect( ListExpr args )
{
  if (nl->ListLength(args) == 1) {
    ListExpr first = nl->First(args);

    if (Stream<Rectangle<2>>::checkType(first)) {
        return 0;
    }
  }
  return -1;
}

//4.2.2 Irgrid3d
// Is used for the ~create\_irgrid3d~ operator.
int IrGrid3dCreateSelect( ListExpr args )
{
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  ListExpr third = nl->Third(args);
  ListExpr fourth = nl->Fourth(args);
  ListExpr fifth = nl->Fifth(args);

  if (Stream<Rectangle<3>>::checkType(first) && Rectangle<3>::checkType(second)
      && CcInt::checkType(third) && 
      CcInt::checkType(fourth) && CcInt::checkType(fifth)) {

      return 0;
  }

  return -1; // should never occur
}

// Is used for the ~feed~ operator.
int IrGrid3dFeedSelect( ListExpr args )
{
  if (nl->ListLength(args) == 1) {
    ListExpr first = nl->First(args);

    if (IrregularGrid3D::checkType(first)) {
        return 0;
    }
  }
  return -1; // should never occur
}

// Is used for the ~cellnos\_ir~ operator.
int IrGrid3dCellnosSelect( ListExpr args )
{
  if (nl->ListLength(args) == 2) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if (IrregularGrid3D::checkType(first)
      && Rectangle<3>::checkType(second)) {
        return 0;
    }
  }
  return -1; // should never occur
}

/* 
  Toprightclass 3D Select 

*/
int IrGrid3dTRCSelect( ListExpr args )
{
  if (nl->ListLength(args) == 3) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (IrregularGrid3D::checkType(first)
      && Rectangle<3>::checkType(second)
      && Rectangle<3>::checkType(third)) {
        return 0;
    }
  }
  return -1;
}

/*
  Toprightclass 3D Select 

*/
int IrGrid3dTRCCellSelect( ListExpr args )
{
  if (nl->ListLength(args) == 3) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (IrregularGrid3D::checkType(first)
      && Rectangle<3>::checkType(second)
      && Rectangle<3>::checkType(third)) {
        return 0;
    }
  }
  return -1;
}

/*
  Smallest Common Cellnumber 3D Select

*/
int IrGrid3dSCCSelect( ListExpr args )
{
  if (nl->ListLength(args) == 4) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);
    ListExpr fourth = nl->Fourth(args);

    if (IrregularGrid3D::checkType(first)
      && Rectangle<3>::checkType(second)
      && Rectangle<3>::checkType(third)
      && CcInt::checkType(fourth)) {
        return 0;
    }
  }
  return -1;
}

/*
  GetCell 3D Select

*/
int IrGrid3dGetCellSelect( ListExpr args )
{
  if (nl->ListLength(args) == 2) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if (IrregularGrid3D::checkType(first)
      && CcInt::checkType(second)) {
        return 0;
    }
  }
  return -1;
}

/* 
  Bbox 3D Select 

*/
int IrGrid3dBBoxSelect( ListExpr args )
{
  if (nl->ListLength(args) == 1) {
    ListExpr first = nl->First(args);

    if (Stream<Rectangle<3>>::checkType(first)) {
        return 0;
    }
  }
  return -1; // should never occur
}

// 4.2.3 2D-Tree

// Is used for the ~create\_2dtree~ operator.
int KDTree2DCreateSelect( ListExpr args )
{
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args); 

  if (Stream<Rectangle<2>>::checkType(first) 
      && Rectangle<2>::checkType(second)) {

    return 0;
  }
  return -1; // should never occur
}

// Is used for the ~feed~ operator.
int KDTree2dFeedSelect( ListExpr args )
{
  if (nl->ListLength(args) == 1) {
    ListExpr first = nl->First(args);

    if (KDTree2D::checkType(first)) {
        return 0;
    }
  }
  return -1; // should never occur
}

// Is used for the ~cellnos\_kd~ operator.
int KDTree2dCellnosSelect( ListExpr args )
{
  if (nl->ListLength(args) == 2) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if (KDTree2D::checkType(first)
      && Rectangle<2>::checkType(second)) {
        return 0;
    }
  }
  return -1;
}

/* ToprightClass 2DTree Select */
int KDTree2dTRCSelect( ListExpr args )
{
  if (nl->ListLength(args) == 3) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (KDTree2D::checkType(first)
      && Rectangle<2>::checkType(second)
      && Rectangle<2>::checkType(third)) {
        return 0;
    }
  }
  return -1; // should never occur
}

int KDTree2dTRCCellSelect( ListExpr args )
{
  if (nl->ListLength(args) == 3) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (KDTree2D::checkType(first)
      && Rectangle<2>::checkType(second)
      && Rectangle<2>::checkType(third)) {
        return 0;
    }
  }
  return -1; // should never occur
}

/* 
  Smallest common cellnumber 2DTree Select 

*/
int KDTree2dSCCSelect( ListExpr args )
{
  if (nl->ListLength(args) == 4) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);
    ListExpr fourth = nl->Fourth(args);

    if (KDTree2D::checkType(first)
      && Rectangle<2>::checkType(second)
      && Rectangle<2>::checkType(third)
      && CcInt::checkType(fourth)) {
        return 0;
    }
  }
  return -1; // should never occur
}

/* 
  GetCell 2DTree Select 

*/
int KDTree2dGetCellSelect( ListExpr args )
{
  if (nl->ListLength(args) == 2) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if (KDTree2D::checkType(first)
      && CcInt::checkType(second)) {
        return 0;
    }
  }
  return -1; // should never occur
}

//4.2.4 3DTree
// Is used for the ~create\_3dtree~ operator.
int KDTree3DCreateSelect( ListExpr args )
{
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args); 

  if (Stream<Rectangle<3>>::checkType(first) 
      && Rectangle<3>::checkType(second)) {
    return 0;
  }
  return -1; // should never occur
}

// Is used for the ~feed~ operator.
int KDTree3dFeedSelect( ListExpr args )
{
  if (nl->ListLength(args) == 1) {
    ListExpr first = nl->First(args);
    if (KDTree3D::checkType(first)) {
        return 0;
    }
  }
  return -1; // should never occur
}

// Is used for the ~cellnos\_3dtree~ operator.
int KDTree3dCellnosSelect( ListExpr args )
{
  if (nl->ListLength(args) == 2) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if (KDTree3D::checkType(first)
      && Rectangle<3>::checkType(second)) {
        return 0;
    }
  }
  return -1; // should never occur
}

/* 
  toprightclass 3dtree select 
  
*/
int KDTree3dTRCSelect( ListExpr args )
{
  if (nl->ListLength(args) == 3) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (KDTree3D::checkType(first)
      && Rectangle<3>::checkType(second)
      && Rectangle<3>::checkType(third)) {
        return 0;
    }
  }
  return -1;
}

int KDTree3dTRCCellSelect( ListExpr args )
{
  if (nl->ListLength(args) == 3) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (KDTree3D::checkType(first)
      && Rectangle<3>::checkType(second)
      && Rectangle<3>::checkType(third)) {
        return 0;
    }
  }
  return -1;
}

/* 
  GetCell 3dtree select 

*/
int KDTree3dGetCellSelect( ListExpr args )
{
  if (nl->ListLength(args) == 2) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if (KDTree3D::checkType(first)
      && CcInt::checkType(second)) {
        return 0;
    }
  }
  return -1; // should never occur
}

/* 
  smallest common cellnumber 3dtree select 

*/
int KDTree3dSCCSelect( ListExpr args )
{
  if (nl->ListLength(args) == 4) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);
    ListExpr fourth = nl->Fourth(args);

    if (KDTree3D::checkType(first)
      && Rectangle<3>::checkType(second)
      && Rectangle<3>:: checkType(third)
      && CcInt::checkType(fourth)) {
        return 0;
    }
  }
  return -1; // should never occur
}


/*
4.3 Value mapping functions

See also IrregularGrid2D.h

*/
// Value mapping functions of operator ~create\_irgrid2d~
int IrGrid2dValueMapCreate( Word* args, Word& result, int message,
                        Word& local, Supplier s ) {
  Stream<Rectangle<2>> input_rect_ptr(args[0]);
  Rectangle<2> *bbox_ptr = static_cast<Rectangle<2>*>( args[1].addr );
  CcInt *row_cnt_ptr =  static_cast<CcInt*>( args[2].addr );
  CcInt *cell_cnt_ptr =  static_cast<CcInt*>( args[3].addr );

  result = qp->ResultStorage(s);
  if (bbox_ptr != nullptr
      && row_cnt_ptr != nullptr && cell_cnt_ptr != nullptr) {
    int row_cnt = row_cnt_ptr->GetIntval();
    int cell_cnt = cell_cnt_ptr->GetIntval();

    if (row_cnt >  0 && cell_cnt > 0) {
      ((IrregularGrid2D*)result.addr)->Set(
        input_rect_ptr, *bbox_ptr, row_cnt, cell_cnt);
      return 0;
    }
  }
  return (0);
}

// Value mapping functions of operator ~create\_irgrid3d~
int IrGrid3dValueMapCreate( Word* args, Word& result, int message,
                        Word& local, Supplier s ) {
  Stream<Rectangle<3>> input_rect_ptr(args[0]);
  Rectangle<3> *bbox_ptr = static_cast<Rectangle<3>*>( args[1].addr );
  CcInt *row_cnt_ptr =  static_cast<CcInt*>( args[2].addr );
  CcInt *cell_cnt_ptr =  static_cast<CcInt*>( args[3].addr );
  CcInt *layer_cnt_ptr = static_cast<CcInt*>( args[4].addr );

  result = qp->ResultStorage(s);

  if (bbox_ptr != nullptr
      && row_cnt_ptr != nullptr 
      && cell_cnt_ptr != nullptr 
      && layer_cnt_ptr != nullptr) {
    int row_cnt = row_cnt_ptr->GetIntval();
    int cell_cnt = cell_cnt_ptr->GetIntval();
    int layer_cnt = layer_cnt_ptr->GetIntval();

    if (row_cnt >  0 && cell_cnt > 0 && layer_cnt > 0) {
      ((IrregularGrid3D*)result.addr)->Set(
        input_rect_ptr, *bbox_ptr, row_cnt, cell_cnt, layer_cnt);

      return 0;
    }
  }

  return (0);
}

/*
  Valuemapping function for create 2D-Tree

*/
int KDTree2DValueMapCreate( Word* args, Word& result, int message,
                        Word& local, Supplier s ) {

    Stream<Rectangle<2>> input_rect_ptr(args[0]);
    Rectangle<2> *bbox_ptr = static_cast<Rectangle<2>*>( args[1].addr );
    CcInt *mode_ptr =  static_cast<CcInt*>( args[2].addr );

    result = qp->ResultStorage(s);

    if (mode_ptr != nullptr) {
      int mode = mode_ptr->GetIntval(); 

    if (bbox_ptr != nullptr) {
      ((KDTree2D*)result.addr)->Set(
        input_rect_ptr, *bbox_ptr, mode);
      
      return 0;
    }
    }

    return (0);
}

/*
  Valuemapping function for create 3D-Tree

*/
int KDTree3DValueMapCreate( Word* args, Word& result, int message,
                        Word& local, Supplier s ) {
    Stream<Rectangle<3>> input_rect_ptr(args[0]);
    Rectangle<3> *bbox_ptr = static_cast<Rectangle<3>*>( args[1].addr );
    CcInt *mode_ptr =  static_cast<CcInt*>( args[2].addr );
    result = qp->ResultStorage(s);

    if (mode_ptr != nullptr) {
      int mode = mode_ptr->GetIntval(); 

    if (bbox_ptr != nullptr) {
      ((KDTree3D*)result.addr)->Set(
        input_rect_ptr, *bbox_ptr, mode);
      return 0;
    }
    }

    return (0);
}

/*
4.4 Definition of operators

4.4.1 Definition of value mapping vectors

*/
ValueMapping irgdrid2dCreateMap[] = { IrGrid2dValueMapCreate };
ValueMapping irgdrid2dFeedMap[] = { IrregularGrid2D::IrGrid2dValueMapFeed };
ValueMapping irgdrid2dCellnosMap[]
  = { IrregularGrid2D::IrGrid2dValueMapCellnos };
ValueMapping irgrid2dSCCMap[] = {IrregularGrid2D::IrGrid2dValueMapSCC };
ValueMapping irgrid2dGetCellMap[] = {IrregularGrid2D::IrGrid2dValueMapGetCell };
ValueMapping irgrid2dBBoxMap[] = {IrregularGrid3D::IrGrid2dValueMapBBox};


ValueMapping irgdrid3dCreateMap[] = { IrGrid3dValueMapCreate };
ValueMapping irgdrid3dFeedMap[] = { IrregularGrid3D::IrGrid3dValueMapFeed };
ValueMapping irgdrid3dCellnosMap[]
  = { IrregularGrid3D::IrGrid3dValueMapCellnos };
ValueMapping irgrid3dTRCMap[] = {IrregularGrid3D::IrGrid3dValueMapTRC };
ValueMapping irgrid3dTRCCellMap[] 
  = {IrregularGrid3D::IrGrid3dValueMapTRCCellId };
ValueMapping irgrid3dSCCMap[] = {IrregularGrid3D::IrGrid3dValueMapSCC };
ValueMapping irgrid3dGetCellMap[] = {IrregularGrid3D::IrGrid3dValueMapGetCell };
ValueMapping irgrid3dBBoxMap[] = {IrregularGrid3D::IrGrid3dValueMapBBox };


ValueMapping kdtree2dCreateMap[] = { KDTree2DValueMapCreate };
ValueMapping kdtree2dFeedMap[] = { KDTree2D::KdTree2dValueMapFeed };
ValueMapping kdtree2dCellnosMap[] = {KDTree2D::Kdtree2dValueMapCellnos};
ValueMapping kdtree2dTRCMap[] = {KDTree2D::Kdtree2dValueMapTRC};
ValueMapping kdtree2dTRCCellMap[] = {KDTree2D::Kdtree2dValueMapTRCCellId};
ValueMapping kdtree2dSCCMap[] = {KDTree2D::Kdtree2dValueMapSCC};
ValueMapping kdtree2dGetCellMap[] = {KDTree2D::Kdtree2dValueMapGetCell};

ValueMapping kdtree3dCreateMap[] = { KDTree3DValueMapCreate };
ValueMapping kdtree3dFeedMap[] = { KDTree3D::KdTree3dValueMapFeed };
ValueMapping kdtree3dCellnosMap[] = {KDTree3D::Kdtree3dValueMapCellnos};
ValueMapping kdtree3dTRCMap[] = {KDTree3D::Kdtree3dValueMapTRC};
ValueMapping kdtree3dTRCCellMap[] = {KDTree3D::Kdtree3dValueMapTRCCellId};
ValueMapping kdtree3dSCCMap[] = {KDTree3D::Kdtree3dValueMapSCC};
ValueMapping kdtree3dGetCellMap[] = {KDTree3D::Kdtree3dValueMapGetCell};


/*
4.4.2 Definition of specification strings

*/
const string createIrGrid2dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(stream(rect) x rect x int x int) -> irgrid2d "
      "</text--->"
      "<text>_ create_irgrid2d[_, _, _]</text--->"
      "<text>creates a two-dimensional irregular grid "
      "from the given parameters.</text--->"
      ") )";

const string feedIrGrid2dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>irgrid2d -> stream(tuple(Id : int , Count : int , "
      "Cell : rect))</text--->"
      "<text>_ feed</text--->"
      "<text>creates a tuple stream "
      "from irgrid2d.</text--->"
      ") )";

const string cellnosIrGrid2dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(irgrid2d x rect) -> intset</text--->"
      "<text>cellnos_ir(_, _)</text--->"
      "<text>get the ids of the irregular grid cells "
      "covered by the given rectangle.</text--->"
      ") )";

const string sccIrGrid2dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(irgrid2d x rect2 x rect2 x int) -> bool</text--->"
      "<text>ssc_ir2d(_, _, _, _)</text--->"
      "<text>returns true if int equals "
      " the id of the smallest common cellnumber "
      "of the two rectangles<2>.</text--->"
      ") )";

const string getcellIrGrid2dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(irgrid2d x int) -> rect2</text--->"
      "<text>getcell_ir2d(_, _)</text--->"
      "<text>returns cell to a given id </text--->"
      ") )";

const string bboxIrGrid2dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(stream<rect>) -> rect</text--->"
      "<text>bbox_grid(_)</text--->"
      "<text>returns the bbox of a stream of "
      "rectangles.</text--->"
      ") )";

const string createIrGrid3dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(stream(rect3) x rect3 x int x int x int) -> irgrid3d "
      "</text--->"
      "<text>_ create_irgrid3d[_, _, _]</text--->"
      "<text>creates a three-dimensional irregular grid "
      "from the given parameters. Three "
      " int values are needed for amount of "
      " cells in all dimension. </text--->"
      ") )";

const string feedIrGrid3dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>irgrid3d -> stream(tuple(Id : int , Count : int , "
      "Cell : rect))</text--->"
      "<text>_ feed</text--->"
      "<text>creates a tuple stream "
      "from irgrid3d.</text--->"
      ") )";

const string cellnosIrGrid3dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(irgrid3d x rect3) -> intset</text--->"
      "<text>cellnos_ir(_, _)</text--->"
      "<text>get the ids of the irregular 3d grid cells "
      "covered by the given 3d rectangle.</text--->"
      ") )";

const string trcIrGrid3dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(irgrid3d x rect3 x rect3) -> int</text--->"
      "<text>trc_ir3d(_, _,_)</text--->"
      "<text>get the toprightclass value "
      " of two given cells/cuboids.</text--->"
      ") )";

const string trcCellIrGrid3dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(irgrid3d x rect3 x rect3) -> int</text--->"
      "<text>trc_ir3d(_, _,_)</text--->"
      "<text>get the id of the cell which should be "
      "reported in case of join.</text--->"
      ") )";

const string sccIrGrid3dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(irgrid3d x rect3 x rect3 x int) -> bool</text--->"
      "<text>ssc_ir3d(_, _, _,_)</text--->"
      "<text>returns true if int eqauls "
      " the id of the smallest common cellnumber "
      "of the two rectangles<3>.</text--->"
      ") )";

const string getcellIrGrid3dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(irgrid3d x int) -> rect3</text--->"
      "<text>getcell_ir3d(_, _)</text--->"
      "<text>returns cell to a given id </text--->"
      ") )";

const string bboxIrGrid3dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(stream<rect3>) -> rect3</text--->"
      "<text>bbox_grid3d(_, _)</text--->"
      "<text>returns the bounding box of a stream"
      " of rectangles.</text--->"
      ") )";

const string create2DTreeSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(stream(rect2) x rect2) -> 2dTree "
      "</text--->"
      "<text>_ create_2dtree</text--->"
      "<text>creates a 2dTree "
      "from the given parameters.</text--->"
      ") )";

const string cellnosKDTree2dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(kdtree2d x rect2) -> intset</text--->"
      "<text>cellnos_kd(_, _)</text--->"
      "<text>get the ids of the 2d-tree cells "
      "covered by the given rectangle.</text--->"
      ") )"; 
      
const string trcCellKDTree2dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(kdtree2d x rect2 x rect2) -> int</text--->"
      "<text>trc_kd(_, _,_)</text--->"
      "<text>get the id of the cell which should"
      " be reported.</text--->"
      ") )";

const string trcKDTree2dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(kdtree2d x rect2 x rect2) -> int</text--->"
      "<text>trc_kd(_, _,_)</text--->"
      "<text>get the toprightclass value of"
      " two given rectangles/cells.</text--->"
      ") )";

const string sccKDTree2dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(kdtree2d x rect2 x rect2 x int) -> bool</text--->"
      "<text>scc_kd(_, _,_;_)</text--->"
      "<text>returns true if int equals "
      " the id of the smallest common"
      " cellnumber of two cells.</text--->"
      ") )";   

const string getcellKDTree2dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(kdtree2d x int) -> rect2</text--->"
      "<text>getcell_kd(_, _)</text--->"
      "<text>returns cell to a given id </text--->"
      ") )";         

const string feedKDTree2dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>2dtree -> stream(tuple(Id : int , Count : int , "
      "Cell : rect))</text--->"
      "<text>_ feed</text--->"
      "<text>creates a tuple stream "
      "from 2dtree.</text--->"
      ") )";

const string create3DTreeSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(stream(rect3) x rect3) -> 3dTree "
      "</text--->"
      "<text>_ create_3dtree</text--->"
      "<text>creates a 3dTree "
      "from the given parameters.</text--->"
      ") )";

const string cellnosKDTree3dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(kdtree3d x rect3) -> intset</text--->"
      "<text>cellnos_kd(_, _)</text--->"
      "<text>get the ids of the 3d-tree cells "
      "covered by the given 3d rectangle.</text--->"
      ") )";      

const string feedKDTree3dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>3dtree -> stream(tuple(Id : int , Count : int , "
      "Cell : rect))</text--->"
      "<text>_ feed</text--->"
      "<text>creates a tuple stream "
      "from 3dtree.</text--->"
      ") )";

const string trcCellKDTree3dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(kdtree3d x rect3 x rect3) -> int</text--->"
      "<text>trc_3d(_, _,_)</text--->"
      "<text>get the id of the cell which should be" 
      " reported.</text--->"
      ") )";  

const string trcKDTree3dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(kdtree3d x rect3 x rect3) -> int</text--->"
      "<text>trc_3d(_, _,_)</text--->"
      "<text>get the toprightclass value of"
      "the given cuboid and cell.</text--->"
      ") )"; 

const string sccKDTree3dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(kdtree3d x rect3 x rect3 x int) -> bool</text--->"
      "<text>scc_3d(_, _,_,_)</text--->"
      "<text> return true if int equals "
      " id of the smallest common"
      " cellnumber of two rectangles.</text--->"
      ") )"; 

const string getcellKDTree3dSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" \"Remarks\")"
      "( <text>(kdtree3d x int) -> rect3</text--->"
      "<text>getcell_3d(_, _)</text--->"
      "<text>returns cell to a given id </text--->"
      ") )";

/*
4.4.3 Definition of the operators

*/
Operator createirgrid2d( "create_irgrid2d",
    createIrGrid2dSpec,
    1,
    irgdrid2dCreateMap,
    IrGrid2dCreateSelect,
    IrGrid2dCreateTypeMap );

Operator feedirgrid2d( "feed",
    feedIrGrid2dSpec,
    1,
    irgdrid2dFeedMap,
    IrGrid2dFeedSelect,
    IrregularGrid2D::IrGrid2dFeedTypeMap );

Operator cellnosirgrid2d( "cellnos_ir",
    cellnosIrGrid2dSpec,
    1,
    irgdrid2dCellnosMap,
    IrGrid2dCellnosSelect,
    IrregularGrid2D::IrGrid2dCellnosTypeMap );

Operator sccirgrid2d( "scc_ir2d",
    sccIrGrid2dSpec,
    1,
    irgrid2dSCCMap,
    IrGrid2dSCCSelect,
    IrregularGrid2D::IrGrid2dSCCTypeMap );

Operator getcellirgrid2d( "getcell_ir2d",
    getcellIrGrid2dSpec,
    1,
    irgrid2dGetCellMap,
    IrGrid2dGetCellSelect,
    IrregularGrid2D::IrGrid2dGetCellTypeMap );

Operator bboxirgrid2d( "bbox_grid",
    bboxIrGrid2dSpec,
    1,
    irgrid2dBBoxMap,
    IrGrid2dBBoxSelect,
    IrregularGrid3D::IrGrid2dBBoxTypeMap );

Operator createirgrid3d( "create_irgrid3d",
    createIrGrid3dSpec,
    1,
    irgdrid3dCreateMap,
    IrGrid3dCreateSelect,
    IrGrid3dCreateTypeMap );

Operator feedirgrid3d( "feed",
    feedIrGrid3dSpec,
    1,
    irgdrid3dFeedMap,
    IrGrid3dFeedSelect,
    IrregularGrid3D::IrGrid3dFeedTypeMap );

Operator cellnosirgrid3d( "cellnos_ir",
    cellnosIrGrid3dSpec,
    1,
    irgdrid3dCellnosMap,
    IrGrid3dCellnosSelect,
    IrregularGrid3D::IrGrid3dCellnosTypeMap );

Operator trcirgrid3d( "trc_ir3d",
    trcIrGrid3dSpec,
    1,
    irgrid3dTRCMap,
    IrGrid3dTRCSelect,
    IrregularGrid3D::IrGrid3dTRCTypeMap );

Operator trccellirgrid3d( "trccell_ir3d",
    trcCellIrGrid3dSpec,
    1,
    irgrid3dTRCCellMap,
    IrGrid3dTRCCellSelect,
    IrregularGrid3D::IrGrid3dTRCCellIdTypeMap );

Operator sccirgrid3d( "scc_ir3d",
    sccIrGrid3dSpec,
    1,
    irgrid3dSCCMap,
    IrGrid3dSCCSelect,
    IrregularGrid3D::IrGrid3dSCCTypeMap );

Operator getcellirgrid3d( "getcell_ir3d",
    getcellIrGrid3dSpec,
    1,
    irgrid3dGetCellMap,
    IrGrid3dGetCellSelect,
    IrregularGrid3D::IrGrid3dGetCellTypeMap );

Operator bboxirgrid3d( "bbox_grid3d",
    bboxIrGrid3dSpec,
    1,
    irgrid3dBBoxMap,
    IrGrid3dBBoxSelect,
    IrregularGrid3D::IrGrid3dBBoxTypeMap );

Operator create2dtree( "create_2dtree",
    create2DTreeSpec,
    1,
    kdtree2dCreateMap,
    KDTree2DCreateSelect,
    KDTree2DCreateTypeMap );

Operator cellnoskdtree2d( "cellnos_kd",
    cellnosKDTree2dSpec,
    1,
    kdtree2dCellnosMap,
    KDTree2dCellnosSelect,
    KDTree2D::Kdtree2dCellnosTypeMap );


Operator trckdtree2d( "trc_kd",
    trcKDTree2dSpec,
    1,
    kdtree2dTRCMap,
    KDTree2dTRCSelect,
    KDTree2D::Kdtree2dTRCTypeMap );

Operator trccellkdtree2d( "trccell_kd",
    trcCellKDTree2dSpec,
    1,
    kdtree2dTRCCellMap,
    KDTree2dTRCCellSelect,
    KDTree2D::Kdtree2dTRCCellIdTypeMap );

Operator scckdtree2d( "scc_kd",
    sccKDTree2dSpec,
    1,
    kdtree2dSCCMap,
    KDTree2dSCCSelect,
    KDTree2D::Kdtree2dSCCTypeMap );

Operator getcellkdtree2d( "getcell_kd",
    getcellKDTree2dSpec,
    1,
    kdtree2dGetCellMap,
    KDTree2dGetCellSelect,
    KDTree2D::Kdtree2dGetCellTypeMap );
    
Operator feedkdtree2d( "feed",
    feedKDTree2dSpec,
    1,
    kdtree2dFeedMap,
    KDTree2dFeedSelect,
    KDTree2D::KdTree2dFeedTypeMap );

Operator create3dtree( "create_3dtree",
    create3DTreeSpec,
    1,
    kdtree3dCreateMap,
    KDTree3DCreateSelect,
    KDTree3DCreateTypeMap );

Operator cellnoskdtree3d( "cellnos_kd",
    cellnosKDTree3dSpec,
    1,
    kdtree3dCellnosMap,
    KDTree3dCellnosSelect,
    KDTree3D::Kdtree3dCellnosTypeMap );
    
Operator feedkdtree3d( "feed",
    feedKDTree3dSpec,
    1,
    kdtree3dFeedMap,
    KDTree3dFeedSelect,
    KDTree3D::KdTree3dFeedTypeMap );  

Operator trckdtree3d( "trc_3d",
    trcKDTree3dSpec,
    1,
    kdtree3dTRCMap,
    KDTree3dTRCSelect,
    KDTree3D::Kdtree3dTRCTypeMap );  

Operator trccellkdtree3d( "trccell_3d",
    trcCellKDTree3dSpec,
    1,
    kdtree3dTRCCellMap,
    KDTree3dTRCCellSelect,
    KDTree3D::Kdtree3dTRCCellIdTypeMap ); 

Operator scckdtree3d( "scc_3d",
    sccKDTree3dSpec,
    1,
    kdtree3dSCCMap,
    KDTree3dSCCSelect,
    KDTree3D::Kdtree3dSCCTypeMap );   

Operator getcellkdtree3d( "getcell_3d",
    getcellKDTree3dSpec,
    1,
    kdtree3dGetCellMap,
    KDTree3dGetCellSelect,
    KDTree3D::Kdtree3dGetCellTypeMap );     

/*
5 Creating the Algebra

*/
class SPart : public Algebra {
  public:
    SPart() : Algebra() {
      AddTypeConstructor( &irgrid2d );
      irgrid2d.AssociateKind(Kind::SIMPLE());

      AddTypeConstructor( &irgrid3d );
      irgrid3d.AssociateKind(Kind::SIMPLE());

      AddTypeConstructor ( &kdtree2d );
      kdtree2d.AssociateKind(Kind::SIMPLE());

      AddTypeConstructor ( &kdtree3d );
      kdtree3d.AssociateKind(Kind::SIMPLE());

      AddOperator( &createirgrid2d );
      AddOperator( &feedirgrid2d );
      AddOperator( &cellnosirgrid2d );
      AddOperator( &sccirgrid2d );
      AddOperator( &getcellirgrid2d );
      AddOperator( &createirgrid3d );
      AddOperator( &feedirgrid3d );
      AddOperator( &cellnosirgrid3d );

      AddOperator( &create2dtree );
      AddOperator( &cellnoskdtree2d );
      AddOperator( &feedkdtree2d );
      AddOperator( &create3dtree );
      AddOperator( &cellnoskdtree3d );
      AddOperator( &feedkdtree3d );

      AddOperator( &trckdtree2d );
      AddOperator( &trccellkdtree2d );
      AddOperator( &scckdtree2d );
      AddOperator( &getcellkdtree2d );
      AddOperator( &trckdtree3d );
      AddOperator( &trccellkdtree3d );
      AddOperator( &scckdtree3d );
      AddOperator( &getcellkdtree3d );
      AddOperator( &trcirgrid3d );
      AddOperator( &trccellirgrid3d );
      AddOperator( &sccirgrid3d );
      AddOperator( &getcellirgrid3d );
      
      AddOperator( &bboxirgrid2d );
      AddOperator( &bboxirgrid3d );
    }

    ~SPart() { };
};

} // end of namespace spart

/*
6 Initialization

*/
extern "C"
Algebra*
InitializeSPart2Algebra( NestedList* nlRef, QueryProcessor* qpRef ) {
  nl = nlRef;
  qp = qpRef;

  return new spart::SPart;
}
