/*
//paragraph [1] title: [{\Large \bf ]	[}]


[1] PointRectangle Algebra

July 2002 RHG

The little example algebra provides two type constructors ~point~ and
~rectangle~ and two operators: (i) ~inside~, which checks whether a point is
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

/*
GNU gcc 3.2 includes the header 'windows.h' from standard headers.
Therefore we need to change internally the name of the Rectangle
class since Windows defines an API function 'Rectangle'.

*/
#ifdef SECONDO_WIN32
#define Rectangle SecondoRectangle
#endif

static NestedList* nl;
static QueryProcessor* qp;

/*
1.2 Dummy Functions

Not interesting, but needed in the definition of a type constructor.

*/
static void* DummyCast( void* addr, SmiRecordFile* ) {return (0);}
/*
2 Type Constructor ~point~

2.1 Data Structure - Class ~Point~

*/
class Point	
{
 public:
  Point( int x, int y );
  ~Point();
  int      GetX();
  int	   GetY();
  void     SetX( int x );
  void     SetY( int y );
  Point*   Clone();
 private:
  int x;
  int y;
};

Point::Point(int X, int Y) {x = X; y = Y;}

Point::~Point() {}

int Point::GetX() {return x;}

int Point::GetY() {return y;}

void Point::SetX(int X) {x = X;}

void Point::SetY(int Y) {y = Y;}

Point* Point::Clone() { return new Point( *this ); }

/*
2.2 List Representation

The list representation of a point is

----	(x y)
----

2.3 ~In~ and ~Out~ Functions

*/

static ListExpr
OutPoint( ListExpr typeInfo, Word value )
{
  Point* point;
  point = (Point*)(value.addr);
  return nl->TwoElemList(nl->IntAtom(point->GetX()), nl->IntAtom(point->GetY()));
}

static Word
InPoint( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Point* newpoint;

  if ( nl->ListLength( instance ) == 2 )
  { 
    ListExpr First = nl->First(instance); 
    ListExpr Second = nl->Second(instance);

    if ( nl->IsAtom(First) && nl->AtomType(First) == IntType 
      && nl->IsAtom(Second) && nl->AtomType(Second) == IntType )
    {
      correct = true;
      newpoint = new Point(nl->IntValue(First), nl->IntValue(Second));
      return SetWord(newpoint);
    }
  }
  correct = false;
  return SetWord(Address(0));
}

static Word
CreatePoint( const ListExpr typeInfo ) 
{
  return (SetWord( new Point( 0, 0 ) ));
}

static void
DeletePoint( Word& w ) 
{
  delete (Point *)w.addr;
  w.addr = 0;
}
 
static void
ClosePoint( Word& w ) 
{
  delete (Point *)w.addr;
  w.addr = 0;
}
 
static Word
ClonePoint( const Word& w ) 
{
  return SetWord( ((Point *)w.addr)->Clone() );
}

static int
SizeOfPoint()
{
  return sizeof(Point);
}
 
/*
2.4 Function Describing the Signature of the Type Constructor

This one works for both type constructors ~point~ and ~rectangle~, in fact,
for all ``atomic'' types.

*/

static ListExpr
PointRectangleProperty()
{
  return (nl->TwoElemList(
		nl->TheEmptyList(),
		nl->SymbolAtom("DATA") ));
}

/*
2.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~point~ does not have arguments, this is trivial.

*/
static bool
CheckPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "point" ));
}
/*
2.6 Creation of the Type Constructor Instance

*/
TypeConstructor point(
	"point",			//name		
	PointRectangleProperty, 	//property function describing signature
        OutPoint,   	InPoint,	//Out and In functions
	CreatePoint,	DeletePoint,	//object creation and deletion
        0, 0, ClosePoint, ClonePoint,    //object open, save, and close
	DummyCast,			//cast function
        SizeOfPoint, 			//sizeof function
	CheckPoint,	                //kind checking function
	0, 				//predef. pers. function for model
        TypeConstructor::DummyInModel, 	
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );
/*
3 Class ~Rectangle~

To define the Secondo type ~rectangle~, we need to (i) define a data structure,
that is, a class, (ii) to decide about a nested list representation, and (iii)
write conversion functions from and to nested list representation.

The function for converting from the list representation is the most involved
one, since it has to check that the given list structure is entirely correct.

Later we need (iv) a property function, (v) a kind checking function. Finally
the type constructor instance can be created.

*/
class Rectangle	
{
 public:
  Rectangle( int XLeft, int XRight, int YBottom, int YTop )
	{xl = XLeft; xr = XRight; yb = YBottom; yt = YTop;}
  ~Rectangle() {}
  int GetXLeft() {return xl;}
  int GetXRight() {return xr;}
  int GetYBottom() {return yb;}
  int GetYTop() {return yt;}
  Rectangle* Clone() { return new Rectangle( *this ); }
  bool intersects( Rectangle r);

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
Rectangle::intersects( Rectangle r )
{
  return ( overlap(xl, xr, r.GetXLeft(), r.GetXRight())
    && overlap(yb, yt, r.GetYBottom(), r.GetYTop()) );
}


/*
3.3 List Representation and ~In~/~Out~ Functions

The list representation of a rectangle is

----	(XLeft XRight YBottom YTop)
----

*/
static ListExpr
OutRectangle( ListExpr typeInfo, Word value )
{
  Rectangle* rectangle;
  rectangle = (Rectangle*)(value.addr);
  return nl->FourElemList(
    nl->IntAtom(rectangle->GetXLeft()), 
    nl->IntAtom(rectangle->GetXRight()),
    nl->IntAtom(rectangle->GetYBottom()), 
    nl->IntAtom(rectangle->GetYTop())		);
}

static Word
InRectangle( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Rectangle* newrectangle;

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
	newrectangle = new Rectangle(xl, xr, yb, yt);
	return SetWord(newrectangle);
      }
    }
  }
  correct = false;
  return SetWord(Address(0));
}

static Word
CreateRectangle( const ListExpr typeInfo )
{
  return (SetWord( new Rectangle( 0, 0, 0, 0 ) ));
}

static void
DeleteRectangle( Word& w )
{
  delete (Rectangle *)w.addr;
  w.addr = 0;
}

static void
CloseRectangle( Word& w )
{
  delete (Rectangle *)w.addr;
  w.addr = 0;
}

static Word
CloneRectangle( const Word& w ) 
{
  return SetWord( ((Rectangle *)w.addr)->Clone() );
}

static int
SizeOfRectangle()
{
  return sizeof(Rectangle);
}

/*
3.4 Property Function - Signature of the Type Constructor

We can reuse the one written for ~point~.

3.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~rectangle~ does not have arguments, this is trivial.

*/
static bool
CheckRectangle( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "rectangle" ));
}
/*
3.6 Creation of the Type Constructor Instance

*/
TypeConstructor rectangle( "rectangle",	PointRectangleProperty,
        		OutRectangle, 		InRectangle,         
			CreateRectangle,	DeleteRectangle, 	
        		0, 0, 			CloseRectangle,     CloneRectangle,
                        DummyCast,    		SizeOfRectangle,    CheckRectangle,	
                        0);
/*
4 Creating Operators

4.1 Type Mapping Function

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol
~typeerror~.

*/

static ListExpr
RectRectBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, "rectangle") && nl->IsEqual(arg2, "rectangle") )  
    return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

static ListExpr
PointRectBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, "point") && nl->IsEqual(arg2, "rectangle") )  
    return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

/*
4.2 Selection Function

Is used to select one of several evaluation functions for an overloaded
operator, based on the types of the arguments. In case of a non-overloaded
operator, we just have to return 0.

*/

static int
simpleSelect (ListExpr args ) { return 0; }

/*
4.3 Value Mapping Function

*/
static int
intersectsFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Intersects predicate for two rectangles.

*/
{
  Rectangle* r1;
  Rectangle* r2;
  r1 = ((Rectangle*)args[0].addr);
  r2 = ((Rectangle*)args[1].addr);

  result = qp->ResultStorage(s);	//query processor has provided
  					//a CcBool instance to take the result
  ((CcBool*)result.addr)->Set(true, r1->intersects(*r2));
  					//the first argument says the boolean
  					//value is defined, the second is the
  					//real boolean value)
  return 0;
}

static int
insideFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Inside predicate for point and rectangle.

*/
{
  Point* p;
  Rectangle* r;
  p = ((Point*)args[0].addr);
  r = ((Rectangle*)args[1].addr);

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
4.4 Definition of Operators

*/

const string intersectsSpec =
  "(<text>(rectangle rectangle) -> bool</text---><text>Intersection predicate for two rectangles.</text--->)";

const string insideSpec =
  "(<text>(point rectangle) -> bool</text---><text>Inside predicate.</text--->)";

/*
Used to explain the signature and the meaning of the ~intersects~ and ~inside~ operators.

*/

Operator intersects (
	"intersects", 		//name
	intersectsSpec,         //specification
	intersectsFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function 
	RectRectBool		//type mapping 
);	
	
Operator inside (
	"inside", 		//name
	insideSpec,		//specification
	insideFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function 
	PointRectBool		//type mapping 
);	
/*
5 Creating the Algebra

*/

class PointRectangleAlgebra : public Algebra
{
 public:
  PointRectangleAlgebra() : Algebra()
  {
    AddTypeConstructor( &point );
    AddTypeConstructor( &rectangle );

    point.AssociateKind("DATA");   	//this means that point and rectangle
    rectangle.AssociateKind("DATA");    //can be used in places where types
    					//of kind DATA are expected, e.g. in
    					//tuples.
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


