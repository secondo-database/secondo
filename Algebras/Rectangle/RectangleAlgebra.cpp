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

[1] Implementation of the Rectangle Algebra

October, 2003. Victor Teixeira de Almeida

1 Overview

This implementation file essentially contains the implementation of the
struct ~Rectangle~, and the definitions of the type constructur
~rect~ with its associated operations.

2 Defines and Includes

*/
using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "SpatialAlgebra.h"
#include "RectangleAlgebra.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;

/*
3 Type Constructor ~rect~

A value of type ~rect~ represents a 2-dimensional rectangle alligned with
the axes x and y. A rectangle in such a way can be represented by four
numbers, the upper and lower values for the two dimensions.

3.1 Implementation of the class ~Rectangle~

The constructor. First one can set if the rectangle is defined, and if it is,
the four coordinates can be set.

*/
Rectangle::Rectangle( const bool defined,
                      const double left, const double right,
                      const double bottom, const double top ) :
defined( defined ),
bottom( bottom ),
top( top ),
left( left ),
right( right )
{
  assert( Proper() );
}

/*
The copy constructor.

*/
Rectangle::Rectangle( const Rectangle& r ) :
defined( r.defined ),
bottom( r.bottom ),
top( r.top ),
left( r.left ),
right( r.right )
{
  assert( Proper() );
}

/*
Checks if the rectangle is defined.

*/
bool Rectangle::IsDefined() const
{
  return defined;
}

/*
Redefinition of operator ~=~.

*/
Rectangle& Rectangle::operator = ( const Rectangle& r )
{
  this->defined = r.defined;
  if( this->defined )
  {
    this->bottom = r.bottom;
    this->top = r.top;
    this->left = r.left;
    this->right = r.right;
  }
  assert( Proper() );
  return *this;
}

/*
Checks if the rectangle contains the point ~p~.

*/
bool Rectangle::Contains( const Point& p ) const
{
  assert( p.IsDefined() );

  if( !IsDefined() )
    return false;

  if( p.GetX() < this->left || p.GetX() > this->right )
    return false;
  else if( p.GetY() < this->bottom || p.GetY() > this->top )
    return false;

  return true;
}

/*
Checks if the rectangle contains the rectangle ~r~.

*/
bool Rectangle::Contains( const Rectangle& r ) const
{
  assert( r.IsDefined() );

  if( !IsDefined() )
    return false;

  if( this->bottom <= r.bottom &&
      this->top >= r.top &&
      this->left <= r.left &&
      this->right >= r.right )
    return true;
  else
    return false;
}

/*
Checks if the rectangle intersects with rectangle ~r~.

*/
bool Rectangle::Intersects( const Rectangle& r ) const
{
  if( !IsDefined() || !r.IsDefined() )
    return false;

  if( this->right < r.left ) return false;
  if( r.right < this->left ) return false;
  if( this->top < r.bottom ) return false;
  if( r.top < this->bottom ) return false;

  return true;
}

/*
Redefinition of operator ~==~.

*/
bool Rectangle::operator == ( const Rectangle& r ) const
{
  assert( IsDefined() && r.IsDefined() );
  return this->bottom == r.bottom &&
         this->top == r.top &&
         this->left == r.left &&
         this->right == r.right;
}

/*
Redefinition of operator ~!=~.

*/
bool Rectangle::operator != ( const Rectangle& r ) const
{
  return !(*this == r);
}

/*
Returns the area of a rectangle.

*/
double Rectangle::Area() const
{
  if( !IsDefined() )
    return 0.0;

  return (this->right - this->left) *
         (this->top - this->bottom);
}

/*
Returns the perimeter of a rectangle.

*/
double Rectangle::Perimeter () const
{
  if( !IsDefined() )
    return 0;

  return 2 * (this->left - this->right + this->top - this->bottom);
}

/*
Returns the min coord value for the given dimension ~dim~.

*/
const double Rectangle::MinD( int dim ) const
{
  assert( dim == 0 || dim == 1 );
  return (dim == 0 ? this->left : this->bottom);
}

/*
Returns the max coord value for the given dimension ~dim~.

*/
const double Rectangle::MaxD( int dim ) const
{
  assert( dim == 0 || dim == 1 );
  return (dim == 0 ? this->right : this->top);
}

/*
Translates the rectangle given ~x~ and ~y~ which can be negative values.

*/
Rectangle& Rectangle::Translate( const double x, const double y )
{
  left += x; right += x;
  bottom += y; top += y;
  return *this;
}


/*
Returns the bounding box that contains both this and the rectangle ~r~.

*/
Rectangle Rectangle::Union( const Rectangle& r ) const
{
  if( !IsDefined() )
    return r;

  if( !r.IsDefined() )
    return *this;

  return Rectangle( true,
                    MIN( this->left, r.left ),
                    MAX( this->right, r.right ),
                    MIN( this->bottom, r.bottom ),
                    MAX( this->top, r.top ) );
}

/*
Returns the intersection between this and the rectangle ~r~.

*/
Rectangle Rectangle::Intersection( const Rectangle& r ) const
{
  if( !IsDefined() )
    return *this;

  if( !r.IsDefined() )
    return r;

  if( Intersects( r ) )
    return Rectangle( true,
                      MAX( this->left, r.left ),
                      MIN( this->right, r.right ),
                      MAX( this->bottom, r.bottom ),
                      MIN( this->top, r.top ) );
  else return Rectangle( false );
}

/*
Returns ~true~ if this is a "proper" rectangle, i.e. it does not
represent an empty set.

*/
bool Rectangle::Proper() const
{
  return !IsDefined() ||
         (IsDefined() && this->bottom <= this->top && this->left <= this->right);
}

/*
3.2 List Representation

The list representation of a rectangle is

----    (x1 y1 x2 y2)
----

3.3 ~Out~-function

*/
ListExpr
OutRectangle( ListExpr typeInfo, Word value )
{
  Rectangle* r = (Rectangle*)(value.addr);
  if( r->IsDefined() )
  {
    return nl->FourElemList(
             nl->RealAtom( r->Left() ),
             nl->RealAtom( r->Right() ),
             nl->RealAtom( r->Bottom() ),
             nl->RealAtom( r->Top() ) );
  }
  else
  {
    return (nl->SymbolAtom("undef"));
  }
}

/*
3.4 ~In~-function

*/
Word
InRectangle( const ListExpr typeInfo, const ListExpr instance,
             const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if( nl->ListLength( instance ) == 4 &&
      nl->IsAtom( nl->First( instance ) ) && nl->AtomType( nl->First( instance ) ) == RealType &&
      nl->IsAtom( nl->Second( instance ) ) && nl->AtomType( nl->Second( instance ) ) == RealType &&
      nl->IsAtom( nl->Third( instance ) ) && nl->AtomType( nl->Third( instance ) ) == RealType &&
      nl->IsAtom( nl->Fourth( instance ) ) && nl->AtomType( nl->Fourth( instance ) ) == RealType  )
  {
    Rectangle *r = new Rectangle( true,
                                  nl->RealValue( nl->First( instance ) ),
                                  nl->RealValue( nl->Second( instance ) ),
                                  nl->RealValue( nl->Third( instance ) ),
                                  nl->RealValue( nl->Fourth( instance ) ) );
    correct = true;
    return SetWord( r );
  }
  else if ( nl->IsAtom( instance ) &&
            nl->AtomType( instance ) == SymbolType &&
            nl->SymbolValue( instance ) == "undef" )
  {
    correct = true;
    return SetWord( new Rectangle( false ) );
  }

  correct = false;
  return SetWord( Address( 0 ) );
}

/*
3.5 ~Create~-function

*/
Word
CreateRectangle( const ListExpr typeInfo )
{
  return SetWord( new Rectangle( false ) );
}

/*
3.6 ~Delete~-function

*/
void
DeleteRectangle( Word& w )
{
  delete (Rectangle *)w.addr;
  w.addr = 0;
}

/*
3.7 ~Close~-function

*/
void
CloseRectangle( Word& w )
{
  delete (Rectangle *)w.addr;
  w.addr = 0;
}

/*
3.8 ~Clone~-function

*/
Word
CloneRectangle( const Word& w )
{
  Rectangle *r = new Rectangle( *((Rectangle *)w.addr) );
  return SetWord( r );
}

/*
3.9 Function describing the signature of the type constructor

*/
ListExpr
RectangleProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("rect"),
                             nl->StringAtom("(<left> <right> <bottom> <top>)"),
                             nl->StringAtom("(0 1 0 1)"))));
}

/*
3.10 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~rect~ does not have arguments, this is trivial.

*/
bool
CheckRectangle( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "rect" ));
}

/*
3.11 ~Cast~-function

*/
void* CastRectangle(void* addr)
{
  return new (addr) Rectangle;
}

/*
3.11 ~SizeOf~-function

*/
int SizeOfRectangle()
{
  return sizeof(Rectangle);
}

/*
3.12 Creation of the type constructor instance

*/
TypeConstructor rect(
        "rect",                           //name
        RectangleProperty,                //property function describing signature
        OutRectangle,    InRectangle,     //Out and In functions
        0,               0,               //SaveToList and RestoreFromList functions
        CreateRectangle, DeleteRectangle, //object creation and deletion
        0,               0,               //open and save functions
        CloseRectangle,  CloneRectangle,  //object close, and clone
        CastRectangle,                    //cast function
        SizeOfRectangle,          //sizeof function
        CheckRectangle,                   //kind checking function
        0,                                //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
4 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

4.1 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

4.1.1 Type mapping function ~RectangleTypeMapBool~

It is for the compare operators which have ~bool~ as resulttype, like =, !=.

*/
ListExpr
RectangleTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if( nl->AtomType( arg1 ) == SymbolType &&
        nl->SymbolValue( arg1 ) == "rect" &&
        nl->AtomType( arg2 ) == SymbolType &&
        nl->SymbolValue( arg2 ) == "rect" )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.1.3 Type mapping function ~RectangleTypeMapBool1~

It is for the operator ~isempty~ which have ~rect~ as input and ~bool~ resulttype.

*/

ListExpr
RectangleTypeMapBool1( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( nl->AtomType( arg1 ) == SymbolType &&
        nl->SymbolValue( arg1 ) == "rect" )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.1.3 Type mapping function ~StreamRectangleTypeMapRectangle~

It is used for the operator ~unionbbox~ and ~intersectionbbox~

Type mapping for ~unionbbox~ and ~intersectionbbox~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) xi)  -> ti
              APPEND (i ti)
----

*/
template<bool isUnion> ListExpr
StreamRectangleTypeMapRectangle( ListExpr args )
{
  ListExpr first, second, attrtype;
  string  attrname, argstr, argstrtmp;
  int j;

  const char* errorMessage1 =
  isUnion ?
    "Operator unionbbox expects a list of length two."
  : "Operator intersectionbbox expects a list of length two.";

  CHECK_COND(nl->ListLength(args) == 2,
    errorMessage1);

  first = nl->First(args);
  second = nl->Second(args);

  nl->WriteToString(argstr, first);
  string errorMessage2 =
  isUnion ?
    "Operator unionbbox expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator unionbbox gets as first argument '" + argstr + "'."
  : "Operator intersectionbbox expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator intersectionbbox gets as first argument '" + argstr + "'.";
  CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       (nl->ListLength(nl->Second(first)) == 2) &&
       (IsTupleDescription(nl->Second(nl->Second(first)))),
       errorMessage2);

  nl->WriteToString(argstr, second);
  string errorMessage3 =
  isUnion ?
    "Operator unionbbox expects as second argument an atom (attributename).\n"
    "Operator unionbbox gets '" + argstr + "'.\n"
    "Atrributename may not be the name of a Secondo object!"
  : "Operator intersectionbbox expects as second argument an atom (attributename).\n"
    "Operator intersectionbbox gets '" + argstr + "'.\n"
    "Atrributename may not be the name of a Secondo object!";
  CHECK_COND((nl->IsAtom(second)) &&
             (nl->AtomType(second) == SymbolType),
       errorMessage3);

  attrname = nl->SymbolValue(second);
  nl->WriteToString(argstr, nl->Second(nl->Second(first)));
  j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);
  string errorMessage4 =
    "Attributename '" + attrname + "' is not known.\n"
    "Known Attribute(s): " + argstr;
  string errorMessage5 =
    "Attribute type is not of type rect.";
  if ( j )
  {
    CHECK_COND( (nl->SymbolValue(attrtype) == "rect"),
    errorMessage5);
    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
           nl->OneElemList(nl->IntAtom(j)), attrtype);
  }
  else
  {
    nl->WriteToString( argstr, nl->Second(nl->Second(first)) );
    ErrorReporter::ReportError(errorMessage4);
    return nl->SymbolAtom("typeerror");
  }
}

/*
10.1.17 Type mapping function ~RectRealRealTypeMapRect~

This type mapping function is used for the ~translate~ operator.

*/
ListExpr RectRealRealTypeMapRect( ListExpr args )
{
  if ( nl->ListLength( args ) == 3 )
  {
    if ( nl->IsEqual( nl->First( args ), "rect" ) &&
         nl->IsEqual( nl->Second( args ), "real" ) &&
         nl->IsEqual( nl->Third( args ), "real" ) )
      return nl->SymbolAtom( "rect" );
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.4 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are
several value mapping functions, one for each possible combination of input
parameter types.

4.4.1 Value mapping functions of operator ~isempty~

*/
int
RectangleIsEmpty( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Rectangle*)args[0].addr)->IsDefined() )
  {
    ((CcBool*)result.addr)->Set( true, false );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, true );
  }
  return (0);
}

/*
4.4.2 Value mapping functions of operator ~$=$~

*/
int
RectangleEqual( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((Rectangle*)args[0].addr)->IsDefined() &&
       ((Rectangle*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, *((Rectangle*)args[0].addr) == *((Rectangle*)args[1].addr) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.4.3 Value mapping functions of operator ~$\neq$~

*/
int
RectangleNotEqual( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((Rectangle*)args[0].addr)->IsDefined() &&
       ((Rectangle*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, *((Rectangle*)args[0].addr) != *((Rectangle*)args[1].addr) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.4.4 Value mapping functions of operator ~intersects~

*/
int
RectangleIntersects( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  ((CcBool *)result.addr)->
    Set( true, ((Rectangle*)args[0].addr)->Intersects( *((Rectangle*)args[1].addr) ) );

  return (0);
}

/*
4.4.5 Value mapping functions of operator ~inside~

*/

int
RectangleInside( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((Rectangle*)args[0].addr)->IsDefined() &&
       ((Rectangle*)args[0].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((Rectangle*)args[1].addr)->Contains( *((Rectangle*)args[0].addr) ) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.4.5 Value mapping function of operator ~unionbbox~ and ~intersectionbbox~

*/
template<bool isUnion>
int RectangleUnionIntersectionBBox( Word* args, Word& result, int message, Word& local, Supplier s )
{
  Word currentTupleWord;
  Rectangle* aggr = (Rectangle*)(qp->ResultStorage(s)).addr;
  aggr->SetDefined(false);
  result = SetWord(aggr);

  assert(args[2].addr != 0);
  int attributeIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);
  while(qp->Received(args[0].addr))
  {
    Tuple* currentTuple = (Tuple*)currentTupleWord.addr;
    Rectangle* currentAttr =
      (Rectangle*)currentTuple->GetAttribute(attributeIndex);

    if( isUnion )
      *aggr = aggr->Union( *currentAttr );
    else
      *aggr = aggr->Intersection( *currentAttr );

    currentTuple->DeleteIfAllowed();
    qp->Request(args[0].addr, currentTupleWord);
  }
  qp->Close(args[0].addr);

  return 0;
}

/*
10.4.26 Value mapping functions of operator ~translate~

*/

static int
RectangleTranslate( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Rectangle *pResult = (Rectangle *)result.addr;

  Rectangle *r = ((Rectangle*)args[0].addr);
  CcReal *x = (CcReal *)args[1].addr;
  CcReal *y = (CcReal *)args[2].addr;

  if( r->IsDefined() )
  {
    *pResult = *r;
    pResult->Translate( x->GetRealval(), y->GetRealval() );
    return (0);
  }
  else
  {
    pResult->SetDefined( false );
    return (0);
  }
}

/*
4.5 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of value
mapping functions for each operator. For nonoverloaded operators there is also such and array
defined, so it easier to make them overloaded.

4.5.1 Definition of specification strings

*/
const string RectangleSpecIsEmpty  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
           "( <text>rect -> bool</text---> <text>isempty ( _ )</text--->"
             "<text>Returns whether the value is defined or not.</text--->"
             "<text>query isempty ( rect1 )</text--->"
             ") )";

const string RectangleSpecEqual  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text>(rect x rect) -> bool</text--->"
        "<text>_ = _</text--->"
        "<text>Equal.</text--->"
        "<text>query rect1 = rect2</text--->"
        ") )";

const string RectangleSpecNotEqual  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text>(rect x rect) -> bool</text--->"
        "<text>_ # _</text--->"
        "<text>Not equal.</text--->"
        "<text>query rect1 # rect2</text--->"
        ") )";

const string RectangleSpecIntersects  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text>(rect x rect) -> bool </text--->"
        "<text>_ intersects _</text--->"
        "<text>Intersects.</text--->"
        "<text>query rect1 intersects rect2</text--->"
        ") )";

const string RectangleSpecInside  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text>(rect x rect) -> bool</text--->"
        "<text>_ inside _</text--->"
        "<text>Inside.</text--->"
        "<text>query rect1 inside rect2</text--->"
        ") )";

const string RectangleSpecUnionBBox  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
        "( <text>((stream (tuple([a1:d1, ... ,an:dn]))) x ai) -> di</text--->"
        "<text>_ unionbbox [ _ ]</text--->"
        "<text>Returns the union of the bounding boxes of attribute "
        "ai over the input stream.</text--->"
        "<text>query cities feed unionbbox [ bbox(citypoint) ]"
        "</text--->"
        ") )";

const string RectangleSpecIntersectionBBox  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
        "( <text>((stream (tuple([a1:d1, ... ,an:dn]))) x ai) -> di</text--->"
        "<text>_ unionbbox [ _ ]</text--->"
        "<text>Returns the intersection of the bounding boxes of attribute "
        "ai over the input stream.</text--->"
        "<text>query cities feed intersectionbbox [ bbox(citypoint) ]"
        "</text--->"
        ") )";
const string RectangleSpecTranslate  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(rect x real x real) -> rect</text--->"
  "<text> translate(_, _, _)</text--->"
  "<text> move the rectangle parallely for some distance.</text--->"
  "<text> query translate(rect1, 3.5, 15.1)</text--->"
  ") )";


/*
4.5.3 Definition of the operators

*/
Operator rectangleisempty( "isempty",
                           RectangleSpecIsEmpty,
                           RectangleIsEmpty,
                           Operator::DummyModel,
                           Operator::SimpleSelect,
                           RectangleTypeMapBool1 );

Operator rectangleequal( "=",
                         RectangleSpecEqual,
                         RectangleEqual,
                         Operator::DummyModel,
                         Operator::SimpleSelect,
                         RectangleTypeMapBool );

Operator rectanglenotequal( "#",
                            RectangleSpecNotEqual,
                            RectangleNotEqual,
                            Operator::DummyModel,
                            Operator::SimpleSelect,
                            RectangleTypeMapBool );

Operator rectangleintersects( "intersects",
                              RectangleSpecIntersects,
                              RectangleIntersects,
                              Operator::DummyModel,
                              Operator::SimpleSelect,
                              RectangleTypeMapBool );

Operator rectangleinside( "inside",
                          RectangleSpecInside,
                          RectangleInside,
                          Operator::DummyModel,
                          Operator::SimpleSelect,
                          RectangleTypeMapBool );

Operator rectangleunionbbox( "unionbbox",
                             RectangleSpecUnionBBox,
                             RectangleUnionIntersectionBBox<true>,
                             Operator::DummyModel,
                             Operator::SimpleSelect,
                             StreamRectangleTypeMapRectangle<true> );

Operator rectangleintersectionbbox( "intersectionbbox",
                                    RectangleSpecIntersectionBBox,
                                    RectangleUnionIntersectionBBox<false>,
                                    Operator::DummyModel,
                                    Operator::SimpleSelect,
                                    StreamRectangleTypeMapRectangle<false> );

Operator rectangletranslate( "translate",
                             RectangleSpecTranslate,
                             RectangleTranslate,
                             Operator::DummyModel,
                             Operator::SimpleSelect,
                             RectRealRealTypeMapRect );

/*
5 Creating the Algebra

*/

class RectangleAlgebra : public Algebra
{
 public:
  RectangleAlgebra() : Algebra()
  {
    AddTypeConstructor( &rect );
    rect.AssociateKind("DATA");

    AddOperator( &rectangleisempty );
    AddOperator( &rectangleequal );
    AddOperator( &rectanglenotequal );
    AddOperator( &rectangleintersects );
    AddOperator( &rectangleinside );
    AddOperator( &rectangleunionbbox );
    AddOperator( &rectangleintersectionbbox );
    AddOperator( &rectangletranslate );
  }
  ~RectangleAlgebra() {};
};

RectangleAlgebra rectangleAlgebra;

/*
6 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeRectangleAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&rectangleAlgebra);
}


