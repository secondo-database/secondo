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

//paragraph [1] title: [{\Large \bf ]   [}]
//characters    [2]    verbatim:   [\verb@]    [@]

""[2]

[1] PointRectangle Algebra

July 2002, R. H. Gueting

2003 - 2006, V. Almeida. Code changes due to interface changes. 

October 2006, M. Spiekermann. Introduction of a namespace, string constants and
usage of the "static_cast<>" templates. Additonally some more comments and hints
are made. 

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
#include "NList.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"

/*
In the next line declarations of the types defined in the ~StandardAlgebra~
are imported. We need this since we have an operator which maps to type ~bool~
which is implementd as class ~CcBool~.
   
*/  
#include "StandardTypes.h"

#include <string>

extern NestedList* nl;
extern QueryProcessor *qp;


/*
First we introduce a new namespace ~PointRectangle~ in order 
to avoid name conflicts with other modules.
   
*/   

namespace PointRectangle {


/*
 
Now we define some string constants which correspond to the symbols for type
constructors and operators used in SECONDO algebra modules. These constants will
be used throughout the code below to avoid redundant use of string constants in
the code. This is important (i) to avoid strange runtime errors, e.g. in the
type mapping, which may be caused by a misspelled type name and (ii) to make
type or operator renaming easier.

*/
 
const string t_XPOINT = "xpoint";
const string t_BOOL = "bool";
const string t_XRECTANGLE = "xrectangle";
const string o_INTERSECTS = "intersects";
const string o_INSIDE = "inside";
const string k_SIMPLE = "SIMPLE";

/*
2 Type Constructor ~xpoint~

2.1 Data Structure - Class ~XPoint~

*/

 
class XPoint
{
 public:
  // Warning: Do never initializations in the default constructor !!!
  // It will be used in a special way in the cast function (see below).
  XPoint() {} 
  
  XPoint( int x, int y );
  ~XPoint();
  int      GetX();
  int      GetY();
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

The list representation of an xpoint is

----    (x y)
----

2.3 ~In~ and ~Out~ Functions

The in functions gets a nested list representation of an ~xpoint~ value passed
in the variable "instance".  It is represented by the C++ type "ListExpr".
Morover ther is a global pointer variable "nl" which points to the (single)
instance of class ~NestedList~.  This class provides a set of functions which
can investigate and manipulate nested lists. For details refer to the file
"NestedList.h".

The parameter "errorInfo" can be used to return specific 
error information if the retrieved list is not correct. In the latter case
the boolean parameter "correct" needs to be set to false.

The return value of the function is of type ~Word~ which can simply be
regarded as a pointer. The query processor operates with this typeless
abstraction for objects. If all integrity checks are correct we will return
a pointer to a new instance of class ~XPoint~. 

*/

Word
InXPoint( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr First = nl->First(instance);
    ListExpr Second = nl->Second(instance);

    if ( nl->IsAtom(First) && nl->AtomType(First) == IntType
      && nl->IsAtom(Second) && nl->AtomType(Second) == IntType )
    {
      correct = true;
      XPoint* newpoint = new XPoint(nl->IntValue(First), nl->IntValue(Second));
      return SetWord(newpoint);
    }
  }
  correct = false;
  return SetWord(Address(0));
}

/*
The out function will get a pointer to an ~XPoint~ representation.
Before we can use member function of class ~XPoint~ we need to do
a type cast in order to tell the compiler about the objects type.

Note: At this point we can be sure that it is a pointer to type ~XPoint~ hence
it is safe to do it. But in general type casts can be a source for ~strange~
errors, e.g. segementation faults if you cast to a type which is not compatible
to the object where the pointer belongs to. 

*/


ListExpr
OutXPoint( ListExpr typeInfo, Word value )
{
  XPoint* point = static_cast<XPoint*>( value.addr );

  return nl->TwoElemList(nl->IntAtom(point->GetX()),
                         nl->IntAtom(point->GetY()));
}


Word
CreateXPoint( const ListExpr typeInfo )
{
  return (SetWord( new XPoint( 0, 0 ) ));
}

void
DeleteXPoint( const ListExpr typeInfo, Word& w )
{ 
  delete static_cast<XPoint*>( w.addr );
  w.addr = 0;
}

void
CloseXPoint( const ListExpr typeInfo, Word& w )
{
  delete static_cast<XPoint*>( w.addr );
  w.addr = 0;
}

Word
CloneXPoint( const ListExpr typeInfo, const Word& w )
{
  return SetWord( static_cast<XPoint*>( w.addr )->Clone() );
}

int
SizeOfXPoint()
{
  return sizeof(XPoint);
}

/*
The Cast function for XPoint. It is needed for the type constructor. Note,
that an empty constructor is needed for this function to work properly.

*/
void* CastXPoint( void* addr ) {
  return (new (addr) XPoint);
}


/*
2.4 Functions Describing the Signature of the Type Constructors

This one works for type constructors ~xpoint~.

*/
ListExpr
XPointProperty()
{
  ConstructorInfo ci; 
  
  ci.name = t_XPOINT;
  ci.signature = "-> " + k_SIMPLE;
  ci.typeExample =  t_XPOINT;
  ci.listRep = "(<x> <y>)";
  ci.valueExample = "(-3 15)";
  ci.remarks = "x- and y-coordinates must be of type int.";

  // the list() function of class ~Constructorinfo~ will create a list with two
  // sublist of the structure ((S1 S2 S3 S4 S5)(T1 T2 T3 T4 T5)). The Sn are
  // string atoms and used as labels and Tn are text atoms wich contain the
  // values above. The list typeconstructors command will use these
  // descriptions. 
  // 
  return ci.list(); }



/*
2.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~xpoint~ does not have arguments, this is trivial.

*/
bool
CheckXPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, t_XPOINT ));
}
/*
2.6 Creation of the Type Constructor Instance

*/
TypeConstructor xpointTC(
      t_XPOINT,                        //name
       XPointProperty,                 //property function describing signature
       OutXPoint, InXPoint,            //Out and In functions
       0, 0,                           //SaveToList, RestoreFromList functions
       CreateXPoint, DeleteXPoint,     //object creation and deletion
       0, 0, CloseXPoint, CloneXPoint, //object open, save, close, and clone
       CastXPoint,                     //cast function
       SizeOfXPoint,                   //sizeof function
       CheckXPoint );                  //kind checking function

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
  XRectangle() {}
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

The list representation of an xrectangle is

----    (XLeft XRight YBottom YTop)
----

In contrast to the code examples above we will use here the class
~NList~ instead of the static functions "nl->f(...)". Its interface is described
in file "NList.h". It is a simple wrapper for calls like "nl->f(...)" and allows
a more object-oriented access to a nested list.

These class was implemented more recently hence you will find much code which uses
the other interface. But as you can observe the code based on ~NList~ is much more
compact, easier to read, understand, and maintain. 

*/

Word
InXRectangle( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = false;
  Word result = SetWord(Address(0));
  
  NList list(instance);
  // When you check list structures it will be a good advice to detect
  // errors as early as possible to avoid deep nestings of if-statements.
  if ( list.length() != 4 )
    return result;

  NList First = list.first();
  NList Second = list.second();
  NList Third = list.third();
  NList Fourth = list.fourth();
     
  
  if ( First.isInt() && Second.isInt()
           && Third.isInt() && Fourth.isInt() )
  {
    int xl = First.intval();
    int xr = Second.intval();
    int yb = Third.intval();
    int yt = Fourth.intval();
    
    if ( xl < xr && yb < yt )     
    {
      correct = true;
      XRectangle* r = new XRectangle(xl, xr, yb, yt);
      result.addr = r;
    }
  }
  
  return result;
}

ListExpr
OutXRectangle( ListExpr typeInfo, Word value )
{
  XRectangle* rectangle = static_cast<XRectangle*>( value.addr );
  NList fourElems(
           NList( rectangle->GetXLeft()   ),
           NList( rectangle->GetXRight()  ),
           NList( rectangle->GetYBottom() ),
           NList( rectangle->GetYTop()    )  );

  return fourElems.listExpr();
}


Word
CreateXRectangle( const ListExpr typeInfo )
{
  return (SetWord( new XRectangle( 0, 0, 0, 0 ) ));
}

/*

As you may have observed most implementations of the support functions needed for registering
a secondo type are trivial to implement. Hence we offer a template class which provides default
implementations (see below) hence only functions which need to do special things need to
be implemented.

*/

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
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    
    if ( nl->IsEqual(arg1, t_XRECTANGLE) && nl->IsEqual(arg2, t_XRECTANGLE) )
      return nl->SymbolAtom(t_BOOL);
    
    if ((nl->AtomType(arg1) == SymbolType) &&
        (nl->AtomType(arg2) == SymbolType))
      ErrorReporter::ReportError("Type mapping function got parameters of type "
                                 +nl->SymbolValue(arg1)+" and "
                                 +nl->SymbolValue(arg2));
    else
      ErrorReporter::ReportError("Type mapping functions got wrong "
                                 "types as parameters.");
  }
  ErrorReporter::ReportError("Type mapping function got a "
                             "parameter of length != 2.");
  
  return nl->TypeError(); 
}

ListExpr
insideTypeMap( ListExpr args )
{
  if ( nl->ListLength(args) != 2 )
  {
   ErrorReporter::ReportError("Type mapping function got a "
                               "parameter of length != 2.");
    return nl->TypeError();
  } 
  
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  
  if ( nl->IsEqual(arg1, t_XPOINT) && nl->IsEqual(arg2, t_XRECTANGLE) )
    return nl->SymbolAtom(t_BOOL);
  
  // second alternative of expected arguments
  if ( nl->IsEqual(arg1, t_XRECTANGLE) && nl->IsEqual(arg2, t_XRECTANGLE) )
    return nl->SymbolAtom(t_BOOL);
  
  if ((nl->AtomType(arg1) == SymbolType) &&
      (nl->AtomType(arg2) == SymbolType))
  { 
    ErrorReporter::ReportError("Type mapping function got parameters of type "
                               +nl->SymbolValue(arg1)+" and "
                               +nl->SymbolValue(arg2));
  }  
  else
  { 
    ErrorReporter::ReportError("Type mapping function got wrong "
                               "types as parameters.");
  }  
  return nl->TypeError();
  
}

/*
4.3 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

*/

int
insideSelect( ListExpr args )
{
  NList list(args);
  if ( list.first().isSymbol( t_XRECTANGLE ) )
    return 1;
  else
    return 0;
}  


/*
4.3 Value Mapping Function

*/

/*
Intersects predicate for two rectangles.

*/
int
intersectsFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  XRectangle *r1 = static_cast<XRectangle*>( args[0].addr );
  XRectangle *r2 = static_cast<XRectangle*>( args[1].addr );

  result = qp->ResultStorage(s);  //query processor has provided
                                  //a CcBool instance to take the result

  CcBool* b = static_cast<CcBool*>( result.addr );
  b->Set(true, r1->intersects(*r2));
                                     //the first argument says the boolean
                                     //value is defined, the second is the
                                     //real boolean value)
  return 0;
}

/*
Inside predicate for xpoint and xrectangle.

*/
int
insideFun_PR (Word* args, Word& result, int message, Word& local, Supplier s)
{
  cout << "insideFun_PR" << endl;
  XPoint* p = static_cast<XPoint*>( args[0].addr );
  XRectangle* r = static_cast<XRectangle*>( args[1].addr );

  result = qp->ResultStorage(s);   //query processor has provided
                                   //a CcBool instance to take the result

  CcBool* b = static_cast<CcBool*>( result.addr );
  
  bool res = ( p->GetX() >= r->GetXLeft() && p->GetX() <= r->GetXRight()
        && p->GetY() >= r->GetYBottom() && p->GetY() <= r->GetYTop() );

  b->Set(true, res); //the first argument says the boolean
                     //value is defined, the second is the
                     //real boolean value)
  return 0;
}

/*
Inside predicate for xrectangle and xrectangle.

*/
int
insideFun_RR (Word* args, Word& result, int message, Word& local, Supplier s)
{
  cout << "insideFun_RR" << endl;
  XRectangle* r1 = static_cast<XRectangle*>( args[0].addr );
  XRectangle* r2 = static_cast<XRectangle*>( args[1].addr );

  result = qp->ResultStorage(s);   //query processor has provided
                                   //a CcBool instance to take the result

  CcBool* b = static_cast<CcBool*>( result.addr );
  
  bool res = true;
  res = res && r1->GetXLeft() >= r2->GetXLeft(); 
  res = res && r1->GetXLeft() <= r2->GetXRight(); 
  
  res = res && r1->GetXRight() >= r2->GetXLeft(); 
  res = res && r1->GetXRight() <= r2->GetXRight(); 
  
  res = res && r1->GetYBottom() >= r2->GetYBottom(); 
  res = res && r1->GetYBottom() <= r2->GetYTop(); 
  
  res = res && r1->GetYTop() >= r2->GetYBottom(); 
  res = res && r1->GetYTop() <= r2->GetYTop(); 

  b->Set(true, res); //the first argument says the boolean
                     //value is defined, the second is the
                     //real boolean value)
  return 0;
}


/*
5 Implementation of the Algebra Class

*/

class PointRectangleAlgebra : public Algebra
{
 public:
  PointRectangleAlgebra() : Algebra()
  {

/*
5.1 Specification of Operators

Each operator specification consist of a two elem list (labels entries). The
~label~ list is typically a list with four strings ("Example" ) and the
~entries~ list has four text atoms.  Text atoms can be notated by using singele
quotes, e.g.  (1 symbol "string" 'this is a text atom'). 

In constrast to the specification of a type operator we will pass the
specification as a textual nested list stored in a C++ string object. As a
consequence this string will be parsed by the nested list parser at startup time
of secondo. If the parse fails, SECONDO aborts directly. 

To avoid errors by concatenating strings to a list which cannot be parsed
directly one can use the class ~OperatorInfo~ which gets the specifications
encoded in C++-string and offers a function ~str()~ which returns the assembled
list as a string.

*/

    OperatorInfo intersectsSpec;
    OperatorInfo* oi = &intersectsSpec;

    oi->name      = o_INTERSECTS;
    oi->signature = t_XRECTANGLE + " x " + t_XRECTANGLE + " -> " + t_BOOL;
    oi->syntax    = "_" + o_INTERSECTS + "_";
    oi->meaning   = "Intersection predicate for two xrectangles.";
    oi->example   = "r1 " + o_INTERSECTS + " r2";


    OperatorInfo insideSpec;
    oi = &insideSpec;
    
    oi->name      = o_INSIDE; 
    oi->signature = t_XPOINT + " x " + t_XRECTANGLE + " -> " + t_BOOL;
    oi->syntax    = "_" + o_INSIDE + "_";
    oi->meaning   = "Inside predicate.";
    oi->example   = "p " + o_INSIDE + " r";

    // add the alternative signatures
    oi->appendSignature( t_XRECTANGLE + " x " + t_XRECTANGLE 
                                              + " -> " + t_BOOL );

    // define an array of function pointers which must be null terminated!
    ValueMapping insideFuns[] = { insideFun_PR, insideFun_RR, 0 };
    

/*
5.2 Definition of the Operators

The code below instatiates two objects of class ~Operator~.

*/

     static Operator intersectsOP (
                           intersectsSpec,
                           intersectsFun,    //value mapping
                           RectRectBool      //type mapping
     );

     static Operator insideOP (
                          insideSpec,
                          insideFuns,  // array of 
                          insideSelect,
                          insideTypeMap  
     );
  
/*
5.3 Registration of Types and Operators

*/

  
    ConstructorInfo ci;
    ci.name         = t_XRECTANGLE;
    ci.signature    = "-> " + k_SIMPLE;
    ci.typeExample  = t_XRECTANGLE;
    ci.listRep      =  "(<xleft> <xright> <ybottom> <ytop>)";
    ci.valueExample = "(4 12 8 2)";
    ci.remarks      = "all coordinates must be of type int.";

/*

The class ~ConstructorFunctions~ is a template class and will create many
default implementations of functions used by a secondo type for deatails refer
to "ConstructorFunctions.h". However some functions need to be implemented since
the default may not be sufficient. The default kind check function assumes, that the
type constructor does not have any arguments like function ~CheckXPoint~ does.

*/    
    ConstructorFunctions<XRectangle> cf;

    // re-assign some function pointers
    cf.create = CreateXRectangle;
    cf.in = InXRectangle;
    cf.out = OutXRectangle;

    // the default implementations for open and save are only 
    // suitable for a class which is derived from class ~Attribute~.
    cf.open = 0;
    cf.save = 0;
    
    static TypeConstructor xrectangleTC( ci, cf );

    AddTypeConstructor( &xpointTC );
    AddTypeConstructor( &xrectangleTC );

    //the lines below define that xpoint and xrectangle
    //can be used in places where types of kind SIMPLE are expected
    xpointTC.AssociateKind( k_SIMPLE );
    xrectangleTC.AssociateKind( k_SIMPLE );   

    AddOperator( &intersectsOP );
    AddOperator( &insideOP );
  }
  ~PointRectangleAlgebra() {};
};

/* 
6 Instantiation of an algebra object

*/
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
dynamically at runtime (if it is built as dynamic link library). The name
of the Initialization function defines the name of the algebra module by
convention it must start with "Initialize<AlgebraName>". 

In order to link the algebra togehter with the system you must create an 
entry in the file makefile.algebra for it and to 

*/

} // end of namespace ~PointRectangle~

extern "C"
Algebra*
InitializePointRectangleAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  // The C++ scope-operator :: must be used to qualify the full name 
  return (&PointRectangle::pointRectangleAlgebra); 
}

/*
 
In order to setup automated test procedures one can write a test specification
for the ~TestRunner~ application. You will find the file example.test in
directory bin and others in the directory "Tests/Testspecs". There is also one for
this algebra.
   
*/
