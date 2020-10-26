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
3 Creation of the type constructor ~irgrid2d~

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
  //IrregularGrid2D::OpenIrGrid2D, IrregularGrid2D::SaveIrGrid2D,
  // object close and clone
  IrregularGrid2D::CloseIrGrid2D, IrregularGrid2D::CloneIrGrid2D,
  // cast function
  0,
  // sizeof function
  IrregularGrid2D::SizeOfIrGrid2D,
  // kind checking function
  IrregularGrid2D::KindCheckIrGrid2D );

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

/*
4.4 Definition of operators

4.4.1 Definition of value mapping vectors

*/
ValueMapping irgdrid2dCreateMap[] = { IrGrid2dValueMapCreate };
ValueMapping irgdrid2dFeedMap[] = { IrregularGrid2D::IrGrid2dValueMapFeed };
ValueMapping irgdrid2dCellnosMap[]
  = { IrregularGrid2D::IrGrid2dValueMapCellnos };

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

/*
5 Creating the Algebra

*/
class SPart : public Algebra {
  public:
    SPart() : Algebra() {
      AddTypeConstructor( &irgrid2d );
      irgrid2d.AssociateKind(Kind::SIMPLE());

      AddOperator( &createirgrid2d );
      AddOperator( &feedirgrid2d );
      AddOperator( &cellnosirgrid2d );
    }

    ~SPart() { };
};

} // end of namespace spart

/*
6 Initialization

*/
extern "C"
Algebra*
InitializeSPartAlgebra( NestedList* nlRef, QueryProcessor* qpRef ) {
  nl = nlRef;
  qp = qpRef;

  return new spart::SPart;
}
