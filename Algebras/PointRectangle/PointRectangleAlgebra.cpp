/*
//paragraph [1] title: [{\Large \bf ]	[}]


[1] PointRectangle Algebra

July 2002 RHG

The little example algebra provides two type constructors ~xpoint~ and
~xrectangle~ and two operators: (i) ~inside~, which checks whether a point is
within a rectangle, and (ii) ~intersects~ which checks two rectangles for
intersection.

1 Preliminaries

1.1 Includes

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"	//needed because we return a CcBool in an op.
#include <string>

extern NestedList* nl;
extern QueryProcessor *qp;

/*
1.2 Dummy Functions

Not interesting, but needed in the definition of a type constructor.

*/
void* DummyCast( void* addr ) {return (0);}
/*
2 Type Constructor ~xpoint~

2.1 Data Structure - Class ~XPoint~

*/
class XPoint
{
 public:
  XPoint( int x, int y );
  ~XPoint();
  int      GetX();
  int	   GetY();
  void     SetX( int x );
  void     SetY( int y );
  XPoint*   Clone();
 private:
  int x;
  int y;
};

XPoint::XPoint(int X, int Y) {x = X; y = Y;}

XPoint::~XPoint() {}

int XPoint::GetX() {return x;}

int XPoint::GetY() {return y;}

void XPoint::SetX(int X) {x = X;}

void XPoint::SetY(int Y) {y = Y;}

XPoint* XPoint::Clone() { return new XPoint( *this ); }

/*
2.2 List Representation

The list representation of a xpoint is

----	(x y)
----

2.3 ~In~ and ~Out~ Functions

*/

ListExpr
OutXPoint( ListExpr typeInfo, Word value )
{
  XPoint* point;
  point = (XPoint*)(value.addr);
  return nl->TwoElemList(nl->IntAtom(point->GetX()), nl->IntAtom(point->GetY()));
}

Word
InXPoint( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{
  XPoint* newpoint;

  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr First = nl->First(instance);
    ListExpr Second = nl->Second(instance);

    if ( nl->IsAtom(First) && nl->AtomType(First) == IntType
      && nl->IsAtom(Second) && nl->AtomType(Second) == IntType )
    {
      correct = true;
      newpoint = new XPoint(nl->IntValue(First), nl->IntValue(Second));
      return SetWord(newpoint);
    }
  }
  correct = false;
  return SetWord(Address(0));
}

/*
2.4 Functions Describing the Signature of the Type Constructors

This one works for type constructors ~xpoint~.

*/
ListExpr
XPointProperty()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
	                     nl->StringAtom("Example Type List"),
			     nl->StringAtom("List Rep"),
			     nl->StringAtom("Example List"),
			     nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
	                     nl->StringAtom("xpoint"),
			     nl->StringAtom("(<x> <y>)"),
			     nl->StringAtom("(-3 15)"),
			     nl->StringAtom("x- and y-coordinates must be "
			     "of type int."))));
}
Word
CreateXPoint( const ListExpr typeInfo )
{
  return (SetWord( new XPoint( 0, 0 ) ));
}

void
DeleteXPoint( Word& w )
{
  delete (XPoint *)w.addr;
  w.addr = 0;
}

void
CloseXPoint( Word& w )
{
  delete (XPoint *)w.addr;
  w.addr = 0;
}

Word
CloneXPoint( const Word& w )
{
  return SetWord( ((XPoint *)w.addr)->Clone() );
}

int
SizeOfXPoint()
{
  return sizeof(XPoint);
}

/*
Function describing type property of type constructor ~xrectangle~

*/

ListExpr
XRectangleProperty()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
	                     nl->StringAtom("Example Type List"),
			     nl->StringAtom("List Rep"),
			     nl->StringAtom("Example List"),
			     nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
	                     nl->StringAtom("xrectangle"),
			     nl->StringAtom("(<xleft> <xright> <ybottom> <ytop>)"),
			     nl->StringAtom("(4 12 8 2)"),
			     nl->StringAtom("all coordinates must be of "
			     "type int."))));
}

/*
2.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~xpoint~ does not have arguments, this is trivial.

*/
bool
CheckXPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "xpoint" ));
}
/*
2.6 Creation of the Type Constructor Instance

*/
TypeConstructor xpoint(
	"xpoint",			//name
	XPointProperty, 	                //property function describing signature
        OutXPoint,   	  InXPoint,	//Out and In functions
        0,                0,	        //SaveToList and RestoreFromList functions
	CreateXPoint,	  DeleteXPoint,	//object creation and deletion
        0, 0, CloseXPoint, CloneXPoint,    //object open, save, close, and clone
	DummyCast,			//cast function
        SizeOfXPoint, 			//sizeof function
	CheckXPoint,	                //kind checking function
	0, 				//predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );
/*
3 Class ~XRectangle~

To define the Secondo type ~xrectangle~, we need to (i) define a data structure,
that is, a class, (ii) to decide about a nested list representation, and (iii)
write conversion functions from and to nested list representation.

The function for converting from the list representation is the most involved
one, since it has to check that the given list structure is entirely correct.

Later we need (iv) a property function, (v) a kind checking function. Finally
the type constructor instance can be created.

*/
class XRectangle
{
 public:
  XRectangle( int XLeft, int XRight, int YBottom, int YTop )
	{xl = XLeft; xr = XRight; yb = YBottom; yt = YTop;}
  ~XRectangle() {}
  int GetXLeft() {return xl;}
  int GetXRight() {return xr;}
  int GetYBottom() {return yb;}
  int GetYTop() {return yt;}
  XRectangle* Clone() { return new XRectangle( *this ); }
  bool intersects( XRectangle r);

 private:
  int xl;
  int xr;
  int yb;
  int yt;

};

/*
3.2 Implementation of Operations

To implement rectangle intersection, we first introduce a function for
interval intersection.

*/

int overlap ( int low1, int high1, int low2, int high2 )
{
  if ( high1 < low2 || high2 < low1 ) return false;
  else return true;
}

bool
XRectangle::intersects( XRectangle r )
{
  return ( overlap(xl, xr, r.GetXLeft(), r.GetXRight())
    && overlap(yb, yt, r.GetYBottom(), r.GetYTop()) );
}


/*
3.3 List Representation and ~In~/~Out~ Functions

The list representation of a xrectangle is

----	(XLeft XRight YBottom YTop)
----

*/
ListExpr
OutXRectangle( ListExpr typeInfo, Word value )
{
  XRectangle* rectangle;
  rectangle = (XRectangle*)(value.addr);
  return nl->FourElemList(
    nl->IntAtom(rectangle->GetXLeft()),
    nl->IntAtom(rectangle->GetXRight()),
    nl->IntAtom(rectangle->GetYBottom()),
    nl->IntAtom(rectangle->GetYTop())		);
}

Word
InXRectangle( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
  XRectangle* newrectangle;

  if ( nl->ListLength( instance ) == 4 )
  {
    ListExpr First = nl->First(instance);
    ListExpr Second = nl->Second(instance);
    ListExpr Third = nl->Third(instance);
    ListExpr Fourth = nl->Fourth(instance);

    if ( nl->IsAtom(First) && nl->AtomType(First) == IntType
      && nl->IsAtom(Second) && nl->AtomType(Second) == IntType
      && nl->IsAtom(Third) && nl->AtomType(Third) == IntType
      && nl->IsAtom(Fourth) && nl->AtomType(Fourth) == IntType )
    {
      int xl = nl->IntValue(First);
      int xr = nl->IntValue(Second);
      int yb = nl->IntValue(Third);
      int yt = nl->IntValue(Fourth);

      if ( xl < xr && yb < yt )
      {
	correct = true;
	newrectangle = new XRectangle(xl, xr, yb, yt);
	return SetWord(newrectangle);
      }
    }
  }
  correct = false;
  return SetWord(Address(0));
}

Word
CreateXRectangle( const ListExpr typeInfo )
{
  return (SetWord( new XRectangle( 0, 0, 0, 0 ) ));
}

void
DeleteXRectangle( Word& w )
{
  delete (XRectangle *)w.addr;
  w.addr = 0;
}

void
CloseXRectangle( Word& w )
{
  delete (XRectangle *)w.addr;
  w.addr = 0;
}

Word
CloneXRectangle( const Word& w )
{
  return SetWord( ((XRectangle *)w.addr)->Clone() );
}

int
SizeOfXRectangle()
{
  return sizeof(XRectangle);
}

/*
3.4 Property Function - Signature of the Type Constructor

We can reuse the one written for ~xpoint~.

3.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~xrectangle~ does not have arguments, this is trivial.

*/
bool
CheckXRectangle( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "xrectangle" ));
}
/*
3.6 Creation of the Type Constructor Instance

*/
TypeConstructor xrectangle( "xrectangle",
							XRectangleProperty,
                           	OutXRectangle,    	InXRectangle,
                           	0,               	0,
			   				CreateXRectangle, 	DeleteXRectangle,
        		   			0,               	0,
                           	CloseXRectangle, 	CloneXRectangle,
                           	DummyCast,       	SizeOfXRectangle,
                           	CheckXRectangle,
                           	0);
/*
4 Creating Operators

4.1 Type Mapping Function

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol
~typeerror~.

*/

ListExpr
RectRectBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, "xrectangle") && nl->IsEqual(arg2, "xrectangle") )
    return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

ListExpr
XPointRectBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, "xpoint") && nl->IsEqual(arg2, "xrectangle") )
    return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

/*
4.3 Value Mapping Function

*/
int
intersectsFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Intersects predicate for two rectangles.

*/
{
  XRectangle* r1;
  XRectangle* r2;
  r1 = ((XRectangle*)args[0].addr);
  r2 = ((XRectangle*)args[1].addr);

  result = qp->ResultStorage(s);	//query processor has provided
  					//a CcBool instance to take the result
  ((CcBool*)result.addr)->Set(true, r1->intersects(*r2));
  					//the first argument says the boolean
  					//value is defined, the second is the
  					//real boolean value)
  return 0;
}

int
insideFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Inside predicate for xpoint and xrectangle.

*/
{
  XPoint* p;
  XRectangle* r;
  p = ((XPoint*)args[0].addr);
  r = ((XRectangle*)args[1].addr);

  result = qp->ResultStorage(s);	//query processor has provided
  					//a CcBool instance to take the result

  bool res = ( p->GetX() >= r->GetXLeft() && p->GetX() <= r->GetXRight()
	&& p->GetY() >= r->GetYBottom() && p->GetY() <= r->GetYTop() );

  ((CcBool*)result.addr)->Set(true, res);
  					//the first argument says the boolean
  					//value is defined, the second is the
  					//real boolean value)
  return 0;
}

/*
4.4 Specification of Operators

*/

const string intersectsSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>(xrectangle xrectangle) -> bool</text--->"
			 "<text>_ intersects _</text--->"
			 "<text>Intersection predicate for two"
                         " xrectangles.</text--->"
			 "<text>r1 intersects r2</text--->"
			 ") )";

const string insideSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>(xpoint xrectangle) -> bool</text--->"
			 "<text>_ inside _</text--->"
			 "<text>Inside predicate.</text--->"
			 "<text>p inside r</text--->"
			 ") )";

/*
4.5 Definition of Operators

*/

Operator intersects (
	"intersects", 		//name
	intersectsSpec,         //specification
	intersectsFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	Operator::SimpleSelect,		//trivial selection function
	RectRectBool		//type mapping
);

Operator inside (
	"inside", 		//name
	insideSpec,		//specification
	insideFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	Operator::SimpleSelect,		//trivial selection function
	XPointRectBool		//type mapping
);

/*
5 Creating the Algebra

*/

class PointRectangleAlgebra : public Algebra
{
 public:
  PointRectangleAlgebra() : Algebra()
  {
    AddTypeConstructor( &xpoint );
    AddTypeConstructor( &xrectangle );

    xpoint.AssociateKind("SIMPLE");   	  //this means that xpoint and xrectangle
    xrectangle.AssociateKind("SIMPLE");   //can be used in places where types
    					  //of kind SIMPLE are expected
    AddOperator( &intersects );
    AddOperator( &inside );
  }
  ~PointRectangleAlgebra() {};
};

PointRectangleAlgebra pointRectangleAlgebra;

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
InitializePointRectangleAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&pointRectangleAlgebra);
}


