/*
//paragraph [1] title: [{\Large \bf ]	[}]
//[ae] [\"{a}]
//[ue] [\"{u}]

[1] NauticalMapAlgebra

Feburary 2004 Anja Lopper

This algebra provides a type constructor ~nauticalMap~, which defines a nautical map. The elements of the nautical map must have an internal list representation.

Funktionenbeschreibung folgt ...

1 Preliminaries

1.1 Includes

*/
using namespace std;


#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
//#include "StandardTypes.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "RelationAlgebra.h"
#include "time.h"
#include "NauticalMapAlgebra.h"

namespace {

NestedList* nl;
QueryProcessor* qp;

enum NauticalType { stnobject, stnobjects, stnline, stnlines, stnregion,
                    sterror };

static NauticalType
TypeOfSymbol( ListExpr symbol )
{
  if ( nl->AtomType( symbol ) == SymbolType )
  {
    string s = nl->SymbolValue( symbol );
    if ( s == "nobject"  ) return (stnobject);
    if ( s == "nobjects" ) return (stnobjects);
    if ( s == "nline" ) return (stnline);
    if ( s == "nlines" ) return (stnlines);
    if ( s == "nregion" ) return (stnregion);
  }
  return (sterror);
}

/*
1.2 Dummy Functions

These functions are needed for the definition of a type constructor (function ~DummyCast~) or for the definition of a non-overloaded operator (function ~simpleSelect~).

*/
static void*
DummyCast( void* addr )
{
  return (0);
}

static int
simpleSelect( ListExpr args )
{
  return 0;
}

/*
1.3 Auxiliary Functions

The function ~toString~ just converts an integer value to a string.

The function ~extractIds~ ["]extracts["] the id-numbers of the algebra and the type from a given type expression (nested list). This type expression must already be in the numeric format.

*/
string toString( int number ) 
{
  ostringstream o;
  o << number << char(0);
  return o.str();
}

void extractIds( const ListExpr numType, int& algebraId, int& typeId )
{
  ListExpr pair;

  if (nl->IsAtom(nl->First(numType))) {
    pair = numType;
  }
  else
  {
    pair = nl->First(numType);
  }

  algebraId = nl->IntValue(nl->First(pair));
  typeId = nl->IntValue(nl->Second(pair));
}

/*
The following ["]generic["] clone function is used by several operators in order to clone objects. Some types may provide just a dummy clone function. In this case the list representation for input and output of objects (if defined) may be used for cloning.

*/
static Word
genericClone( int algebraId, int typeId, ListExpr typeInfo, Word object )
{
  AlgebraManager* am = SecondoSystem::GetAlgebraManager();

  Word clone;

  // Try cloning with the clone function of the appropriate type

  clone = (am->CloneObj(algebraId, typeId))(object);

  if (clone.addr == 0) {

    // Try cloning via the object's list representation

    ListExpr objectLE;

    int errorPos;
    ListExpr errorInfo;
    bool correct;

    objectLE = (am->OutObj(algebraId, typeId))(typeInfo, object);

    clone = (am->InObj(algebraId, typeId))
                   (typeInfo, objectLE, errorPos, errorInfo, correct);

    assert (correct);
  }

  return clone;
}

/*
2 Type Constructor ~nauticalObject~

A value of type ~nauticalOjbject~ represents an point-object in a seachart or is undefined.

2.1 Implementation of the class ~NauticalObject~

At first a data structure for storing an ~nauticalObject~ in the main memory is defined. The object is represented as a storage Word (which is often a pointer to the actual object).

*/

NauticalObject::NauticalObject() 
{
}
NauticalObject::NauticalObject(const bool d,
                               const char* oName, 
                               const Point& oPosition)
{
   defined = d;
   strcpy(name, oName); 
   position = oPosition;
}

NauticalObject::NauticalObject(const NauticalObject& nObject) :
   defined(nObject.IsDefined())
{
   if ( defined )
   {
      char hilf[49];
      strcpy(hilf,nObject.GetObjectName()); 
      strcpy(name,hilf); 
      position = nObject.GetPosition();
   }
}

/* Deconstructor */

NauticalObject::~NauticalObject() {}

const char* NauticalObject::GetObjectName() const
{
   assert( IsDefined() );
   return name;
}
const Point& NauticalObject::GetPosition() const
{
   assert( IsDefined() );
   return position;
}

void NauticalObject::SetObjectName(const char* oName) 
{
   strcpy(name,oName);
}
void NauticalObject::SetPosition(const Point& oPosition) 
{
   position = oPosition;
}

void  NauticalObject::Set( const char* oName, const Point& oPosition)
    {
        strcpy(name,oName);
        position=oPosition;
    }

const Rectangle NauticalObject::BoundingBox() const
{
  return Rectangle( true, position.GetX(), position.GetX(), 
                          position.GetY(), position.GetY() );
}

NauticalObject& NauticalObject::operator=( const NauticalObject& object )
{
  defined = object.IsDefined();
  if (defined)
  {
     strcpy(name, object.GetObjectName());
     position = object.GetPosition();
  }
  return *this;
}

bool NauticalObject::operator==( const NauticalObject& object ) const
{
  assert( defined && object.IsDefined() );
  return (position == object.GetPosition() && 
          strcmp(name, object.GetObjectName()) == 0);
}

bool NauticalObject::operator!=( const NauticalObject& object ) const
{
  assert( defined && object.IsDefined() );
  return (position != object.GetPosition() || 
          strcmp(name, object.GetObjectName()) != 0);
}

bool NauticalObject::operator<=( const NauticalObject& object ) const
{
  assert( defined && object.IsDefined() );
  if( position <= object.GetPosition() )
     return 1;
  else
     return 0;
/*
  if( position.GetX() < object.GetPosition().GetX() )
    return 1;
  else if( position.GetX() == object.GetPosition().GetX() && 
           position.GetY() <= object.GetPosition().GetY() )
    return 1;
  return 0;
*/
}

bool NauticalObject::operator<( const NauticalObject& object ) const
{
  assert( defined && object.IsDefined() );
  if( position < object.GetPosition() )
     return 1;
  else
     return 0;
/*
  if( position.GetX() < object.GetPosition().GetX() )
    return 1;
  else if( position.GetX() == object.GetPosition().GetX() && 
           position.GetY() < object.GetPosition().GetY() )
    return 1;
  return 0;
*/
}

bool NauticalObject::operator>=( const NauticalObject& object ) const
{
  assert( defined && object.IsDefined() );
  if( position >= object.GetPosition() )
     return 1;
  else
     return 0;
/*
  if( position.GetX() > object.GetPosition().GetX() )
    return 1;
  else if( position.GetX() == object.GetPosition().GetX() && 
           position.GetY() >= object.GetPosition().GetY() )
    return 1;
  return 0;
*/
}

bool NauticalObject::operator>( const NauticalObject& object ) const
{
  assert( defined && object.IsDefined() );
  if( position > object.GetPosition() )
     return 1;
  else
     return 0;
/*
  if( position.GetX() > object.GetPosition().GetX() )
    return 1;
  else if( position.GetX() == object.GetPosition().GetX() && 
           position.GetY() > object.GetPosition().GetY() )
    return 1;
  return 0;
*/
}

ostream& operator<<( ostream& o, const NauticalObject& n )
{
  if( n.IsDefined() )
    o << "(" << n.GetObjectName() << ", " << 
                n.GetPosition().GetX() << ", " << 
                n.GetPosition().GetY() << ")";
  else
    o << "undef";

  return o;
}

/*
  ************************************************************************
  The following 10 functions are used for porting points to Tuple.
  ************************************************************************

*/

/*
int NauticalObject::NumOfFLOBs()
{
  return 1;
}

FLOB *NauticalObject::GetFLOB(const int i)
{
  NauticalObject nobject;
  nobject.SetPosition(position);
  nobject.SetObjectName(name);
  assert( i >= 0 && i < NumOfFLOBs() );
  return nobject;
}
*/
bool NauticalObject::IsDefined() const
{
  return defined;
}

void NauticalObject::SetDefined( bool Defined )
{
  defined = Defined;
}

size_t   NauticalObject::HashValue()
{
    if(!defined)  return (0);
    unsigned long h1 = 0;
    char* s = name;
    //strcpy(s, name);
    while(*s != 0)
    {
        h1 = 5 * h1 + *s;
        s++;
    }
    Coord x=position.GetX();
    Coord y=position.GetY();
    unsigned long h2;
#ifdef RATIONAL_COORDINATES
    h2=(unsigned long)
        (5*(x.IsInteger()? x.IntValue():x.Value())
          + (y.IsInteger()? y.IntValue():y.Value()));
#else
    h2=(unsigned long)(5*x + y);
#endif
    return size_t(h1+h2);
}

void  NauticalObject::CopyFrom(StandardAttribute* right)
{
//  cout<<"classcopy ////////////////////"<<endl;

  NauticalObject* nObject = (NauticalObject*)right;
  defined = nObject->IsDefined();
  if (defined)
  {
      Set( nObject->GetObjectName(), nObject->GetPosition());
  }
  //cout<<*this<<" .vs. "<<*nObject<<endl;
}

int   NauticalObject::Compare(Attribute * arg)
{
    int res=0;
    NauticalObject* nObject = (NauticalObject* )(arg);
    if ( !nObject ) return (-2);

    if (!IsDefined() && !(arg->IsDefined()))  res=0;
    else if (!IsDefined())  res=-1;
    else  if (!(arg->IsDefined())) res=1;
    else
    {
        if (*this > *nObject) res=1;
        else if (*this < *nObject) res=-1;
        else res=0;
    }
    return (res);
}

bool   NauticalObject::Adjacent(Attribute * arg)
{
    return 0;
    //for points which takes double values, we can not decides whether they are
    //adjacent or not.
}

int  NauticalObject::Sizeof() const
{
    return sizeof(NauticalObject);
}

NauticalObject*  NauticalObject::Clone()
{
  // cout<<"classclone ////////////////////"<<endl;
    return (new NauticalObject( *this));
}

ostream& NauticalObject::Print( ostream &os )
{
    if (defined)
        return (os << GetObjectName() << ", "<<position.GetX() << ", " << position.GetY());
    else    return (os << "undefined");
}
/*
  ***************************************************
   End of the definition of the virtual functions.
  ***************************************************

*/


/*
2.2 List Representation

The list representation of a NauticalPoint is

----    ( Name (X Y) )
----

2.3 ~In~ and ~Out~ Functions

*/

static ListExpr
OutNauticalObject( ListExpr typeInfo, Word value )
{
//cout << "NauticalObject1" << endl;
  NauticalObject* object = (NauticalObject*)(value.addr);
  return nl->TwoElemList(nl->StringAtom(object->GetObjectName()),
                         nl->TwoElemList(
                             nl->RealAtom(object->GetPosition().GetX()),
                             nl->RealAtom(object->GetPosition().GetY())));
}

static Word
InNauticalObject( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  NauticalObject* newobject;

//cout << "NauticalObject2" << endl;
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr First = nl->First(instance);
    ListExpr Second = nl->Second(instance);
    ListExpr X = nl->First(Second);
    ListExpr Y = nl->Second(Second);

    if ( nl->IsAtom(First) && nl->AtomType(First) == StringType
      && nl->IsAtom(X) && nl->AtomType(X) == RealType 
      && nl->IsAtom(Y) && nl->AtomType(Y) == RealType )
    {
      float x = nl->RealValue(X);
      float y = nl->RealValue(Y);
      Point rp;
      rp.Set(x, y);
      char name[49];
      strcpy(name, nl->StringValue(First).c_str());

      correct = true;
      newobject = new NauticalObject(true, name, rp);
      return SetWord(newobject);
    }
  }
  correct = false;
  return SetWord(Address(0));
}


/*
2.4 Property Function - Signature of the Type Constructor

Functions describing type property of type constructor ~nauticalobject~

*/

static ListExpr
NauticalObjectProperty()
{
//cout << "NauticalObject3" << endl;
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,"all coordinates must be "
  "of type real.");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("nobject"),
                             nl->StringAtom("(name (<x> <y>) )"),
                             nl->StringAtom("(Boje (8.0 12.0))"),
                             remarkslist)));
}

static Word
CreateNauticalObject( const ListExpr typeInfo )
{
//cout << "NauticalObject4" << endl;
  Point rp;
  rp.Set(0.0, 0.0);
  //string name= "";
      char name[49] = "";
 //     (strcpy(name, nl->StringValue(First).c_str());

  return (SetWord( new NauticalObject( true, name, rp ) ));
}

static void
DeleteNauticalObject( Word& w )
{
//cout << "NauticalObject5" << endl;
  delete (NauticalObject *)w.addr;
  w.addr = 0;
}

static void
CloseNauticalObject( Word& w )
{
//cout << "NauticalObject6" << endl;
  delete (NauticalObject *)w.addr;
  w.addr = 0;
}
static Word
CloneNauticalObject( const Word& w )
{
//cout << "NauticalObject7" << endl;
  return SetWord( ((NauticalObject *)w.addr)->Clone() );
}
/*
3.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~nauticalobject~ does not have arguments, this is trivial.

*/
static bool
CheckNauticalObject( ListExpr type, ListExpr& errorInfo )
{
//cout << "NauticalObject8" << endl;
  return (nl->IsEqual( type, "nobject" ));
}
int
SizeOfNauticalObject()
{
//cout << "NauticalObject9" << endl;
  return sizeof(NauticalObject);
}

/*
2.11 ~Cast~-function

*/
void* CastNauticalObject(void* addr)
{
  return (new (addr) NauticalObject);
}

/*
2.6 Creation of the Type Constructor Instance

*/
TypeConstructor nauticalobject( 
      "nobject",
      NauticalObjectProperty,
      OutNauticalObject, InNauticalObject,
      0, 0,            //SaveToList and RestoreFromList functio
      CreateNauticalObject, DeleteNauticalObject,
      0, 0,
      CloseNauticalObject, CloneNauticalObject,
      CastNauticalObject,
      SizeOfNauticalObject, CheckNauticalObject,
      0,
      TypeConstructor::DummyInModel,
      TypeConstructor::DummyOutModel,
      TypeConstructor::DummyValueToModel,
      TypeConstructor::DummyValueListToModel );

/*
3 Type Constructor ~nauticalobjects~

A ~nauticalobjects~ value is a finite set of nauticalobjects.

3.1 Implementation of the class ~NauticalObjects~

*/
NauticalObjects::NauticalObjects( const int initsize ) :
  nobjects( initsize ),
  bbox( false ),
  ordered( true )
{}

NauticalObjects::NauticalObjects( NauticalObjects& ns ) :
  nobjects( ns.Size() ),
  bbox( ns.BoundingBox() ),
  ordered( true )
{
  assert( ns.IsOrdered() );

  for( int i = 0; i < ns.Size(); i++ )
  {
    NauticalObject n;
    ns.Get( i, n );
    nobjects.Put( i, n );
  }
}

void NauticalObjects::Destroy()
{
  nobjects.Destroy();
}

NauticalObjects::~NauticalObjects()
{
}

const Rectangle NauticalObjects::BoundingBox() const
{
  return bbox;
}

void NauticalObjects::Get( const int i, NauticalObject& n )
{
  assert( i >= 0 && i < Size() );

  nobjects.Get( i, n );
}

int NauticalObjects::Size() const
{
  return nobjects.Size();
}

bool NauticalObjects::IsEmpty() const
{
  return nobjects.Size() == 0;
}

/*
The following 5 functions, SelectFirst(), SelectNext(), EndOfPt(), GetPt(), InsertPt() are added by DZM
as a basis for object traversal operations

*/

void NauticalObjects::SelectFirst()
{
    if (IsEmpty()) pos=-1;
    else pos=0;
}

void NauticalObjects::SelectNext()
{
    if ((pos>=0) && (pos<Size()-1)) pos++;
    else pos=-1;
}

bool NauticalObjects::EndOfPt()
{
    return (pos==-1);
}
void NauticalObjects::GetPt( NauticalObject& n )
{
    if (( pos>=0) && (pos<=Size()-1)) nobjects.Get( pos, n);
    else n.SetDefined(false);
}

void NauticalObjects::InsertPt( NauticalObject& n )
{
    assert(n.IsDefined());

    bbox = bbox.Union( n.BoundingBox() );

    if( !IsOrdered() )
    {
        pos=nobjects.Size();
        nobjects.Put( nobjects.Size(), n );
    }
    else
    {
        int insertpos = Position( n );
        if( insertpos != -1 )
        {
            int i;
            NauticalObject auxp;
            for( i = nobjects.Size() - 1; i >= insertpos; i++ )
                nobjects.Get( i, auxp );
                nobjects.Put( i+1, auxp );
            }
            nobjects.Put( insertpos, n );
            pos=insertpos;
        }
    }
}

int NauticalObjects::Position( const NauticalObject& n )
{
  assert( IsOrdered() && n.IsDefined() );

  int first = 0, last = Size();

  while (first <= last)
  {
    int mid = ( first + last ) / 2;
    NauticalObject midNauticalObject;
    nobjects.Get( mid, midNauticalObject );
    if( n > midNauticalObject )
      first = mid + 1;
    else if( n < midNauticalObject )
      last = mid - 1;
    else
      return mid;
   }
   return -1;
}

NauticalObjects& NauticalObjects::operator=( NauticalObjects& ns )
{
  assert( ns.IsOrdered() );

  nobjects.Clear();
  nobjects.Resize( ns.Size() );
  for( int i = 0; i < ns.Size(); i++ )
  {
    NauticalObject n;
    ns.Get( i, n );
    nobjects.Put( i, n );
  }
  bbox = ns.BoundingBox();
  ordered = true;
  return *this;
}

void NauticalObjects::StartBulkLoad()
{
  assert( IsOrdered() );
  ordered = false;
}

void NauticalObjects::EndBulkLoad()
{
  assert( !IsOrdered() );
  Sort();
  ordered = true;
}

bool NauticalObjects::IsOrdered() const
{
  return ordered;
}

void  NauticalObjects::setOrdered(bool isordered)
{
  ordered = isordered;
}

bool NauticalObjects::operator==( NauticalObjects& ns )
{
  assert( IsOrdered() && ns.IsOrdered() );

  if( Size() != ns.Size() )
    return 0;

  if( bbox != ns.BoundingBox() )
    return 0;

  for( int i = 0; i < Size(); i++ )
  {
    NauticalObject n1, n2;
    nobjects.Get( i, n1 );
    ns.Get( i, n2 );
    if( n1 != n2 )
      return 0;
  }
  return 1;
}

bool NauticalObjects::operator!=( NauticalObjects& ns )
{
  assert( IsOrdered() && ns.IsOrdered() );

  return !( *this == ns );
}

NauticalObjects& NauticalObjects::operator+=(const NauticalObject& n)
{
  assert( n.IsDefined() );

  bbox = bbox.Union( n.BoundingBox() );

  if( !IsOrdered() )
  {
      bool found=false;
      NauticalObject auxn;

      for( int i = 0; ((i < nobjects.Size())&&(!found)); i++ )
      {
          nobjects.Get( i, auxn );
          if (auxn==n) found=true;
      }

      if (!found)
          nobjects.Put( nobjects.Size(), n );
  }
  else
  {
    int pos = Position( n );
    if( pos != -1 )
    {
      for( int i = nobjects.Size() - 1; i >= pos; i++ )
      {
        NauticalObject auxn;
        nobjects.Get( i, auxn );
        nobjects.Put( i+1, auxn );
      }
      nobjects.Put( pos, n );
    }
  }
  return *this;
}

NauticalObjects& NauticalObjects::operator+=(NauticalObjects& ns)
{
  bbox = bbox.Union( ns.BoundingBox() );

  for( int i = 0; i < ns.Size(); i++ )
  {
    NauticalObject n;
    ns.Get( i, n );
    nobjects.Put( nobjects.Size(), n );
  }
  if( IsOrdered() )
  {
    ordered = false;
    Sort();
  }

  return *this;
}

NauticalObjects& NauticalObjects::operator-=(const NauticalObject& n)
{
  assert( IsOrdered() && n.IsDefined() );

  int pos = Position( n );
  if( pos != -1 )
  {
    for( int i = pos; i < Size(); i++ )
    {
      NauticalObject auxn;
      nobjects.Get( i+1, auxn );
      nobjects.Put( i, auxn );
    }
  }

  // Naive way to redo the bounding box.
  bbox.SetDefined( false );
  for( int i = 0; i < Size(); i++ )
  {
    NauticalObject auxn;
    nobjects.Get( i, auxn );
    bbox = bbox.Union( auxn.BoundingBox() );
  }

  return *this;
}
ostream& operator<<( ostream& o, NauticalObjects& ns )
{
  o << "<";
  for( int i = 0; i < ns.Size(); i++ )
  {
    NauticalObject n;
    ns.Get( i, n );
    o << " " << &n;
  }
  o << ">";

  return o;
}

int NauticalObjectCompare(const void *a, const void *b)
{
  NauticalObject *na = new ((void*)a) NauticalObject,
        *nb = new ((void*)b) NauticalObject;

  if( *na < *nb )
      return 0;
  if( *na == *nb )
      return -1;

  return 1;
}

void NauticalObjects::Sort()
{
  assert( !IsOrdered() );

  nobjects.Sort( NauticalObjectCompare );
  cout << "sort1" << endl;

  ordered = true;
}

bool NauticalObjects::Contains( const NauticalObject& n )
{
  assert( IsOrdered() && n.IsDefined() );

  if( IsEmpty() )
    return false;

  if( !bbox.Contains( n.GetPosition() ) )
    return false;

  int first = 0, last = Size() - 1;

  while (first <= last)
  {
    int mid = ( first + last ) / 2;
    NauticalObject midNauticalObject;
    nobjects.Get( mid, midNauticalObject );
    if( n > midNauticalObject )
      first = mid + 1;
    else if( n < midNauticalObject )
      last = mid - 1;
    else
      return true;
   }
   return false;
}

bool NauticalObjects::Contains( NauticalObjects& ns )
{
  assert( IsOrdered() && ns.IsOrdered() );

  if( ns.IsEmpty() )
    return true;
  if( IsEmpty() )
    return false;
  if( !bbox.Contains( ns.BoundingBox() ) )
    return false;

  NauticalObject n1, n2;
  int i = 0, j = 0;

  Get( i, n1 );
  ns.Get( j, n2 );
  while( true )
  {
    if( n1 == n2 )
    {
      if( ++j == ns.Size() )
        return true;
      ns.Get( j, n2 );
      if( ++i == Size() )
        return false;
      Get( i, n1 );
    }
    else if( n1 < n2 )
    {
      if( ++i == Size() )
        return false;
      Get( i, n1 );
    }
    else // n1 > n2
    {
      return false;
    }
  }
  // This part of the code should never be reached.LineSeg
  assert( true );
  return true;
}

bool NauticalObjects::Inside( NauticalObjects& ns )
{
  assert( IsOrdered() && ns.IsOrdered() );

  return ns.Contains( *this );
}

bool NauticalObjects::Intersects( NauticalObjects& ns )
{
  assert( IsOrdered() && ns.IsOrdered() );

  if( IsEmpty() || ns.IsEmpty() )
    return false;

  if( !bbox.Intersects( ns.BoundingBox() ) )
    return false;

  NauticalObject n1, n2;
  int i = 0, j = 0;

  Get( i, n1 );
  ns.Get( j, n2 );

  while( 1 )
  {
    if( n1 == n2 )
      return true;
    if( n1 < n2 )
    {
      if( ++i == Size() )
        return false;
      Get( i, n1 );
    }
    else // n1 > n2
    {
      if( ++j == ns.Size() )
        return false;
      ns.Get( j, n2 );
    }
  }
  // this part of the code should never be reached
  assert( false );
  return false;
}

/*
  ************************************************************************
  The following 10 functions are used for porting points to Tuple.
  ************************************************************************

*/
int NauticalObjects::NumOfFLOBs()
{
  return 1;
}

FLOB *NauticalObjects::GetFLOB(const int i)
{
  assert( i >= 0 && i < NumOfFLOBs() );
  return &nobjects;
}

bool NauticalObjects::IsDefined() const
{
  return true;
}

void NauticalObjects::SetDefined( bool Defined )
{
    //defined = Defined;
    //since every points is defined, so the function does nothing.
}

size_t   NauticalObjects::HashValue()
{
    if(IsEmpty())  return (0);
    unsigned long h1 = 0;
    unsigned long h2 = 0;

    NauticalObject n;
    Coord x;
    Coord y;
    char *s;

    for( int i = 0; ((i < Size())&&(i<5)); i++ )
    {
        Get( i, n );
        strcpy(s, n.GetObjectName());
        while(*s != 0)
        {
            h1 = 5 * h1 + *s;
            s++;
        }

        x=n.GetPosition().GetX();
        y=n.GetPosition().GetY();
#ifdef RATIONAL_COORDINATES
        h2=h2+(unsigned long)
          (5*(x.IsInteger()? x.IntValue():x.Value())
           + (y.IsInteger()? y.IntValue():y.Value()));
#else
        h2=h2+(unsigned long)(5*x + y);
#endif
    }
    return size_t(h1+h2);
}

void  NauticalObjects::Clear()
{
    nobjects.Clear();
    pos=-1;
    ordered=true;
    bbox.SetDefined(false);
}

void  NauticalObjects::CopyFrom(StandardAttribute* right)
{
    NauticalObjects *ns = (NauticalObjects*)right;
    ordered = true;
    assert( ns->IsOrdered());
    Clear();
    for( int i = 0; i < ns->Size(); i++ )
    {
        NauticalObject n;
        ns->Get( i, n );
        nobjects.Put( i, n );
    }
    bbox = ns->BoundingBox();
}

int   NauticalObjects::Compare(Attribute * arg)
{
    int res=0;
    NauticalObjects* ns = (NauticalObjects* )(arg);
    if ( !ns ) return (-2);
    if (IsEmpty() && (ns->IsEmpty()))  res=0;
    else if (IsEmpty())  res=-1;
    else  if ((ns->IsEmpty())) res=1;
    else
    {
        if (Size() > ns->Size()) res=1;
        else if (Size() < ns->Size()) res=-1;
        else  //their sizes are equal
        {
            bool decided;
            for( int i = 0; ((i < Size())&&(!decided)); i++ )
            {
                NauticalObject n1, n2;
                Get( i, n1);
                ns->Get( i, n2 );

                if (n1 > n2) {res=1;decided=true;}
                else if (n1 < n2) {res=-1;decided=true;}
            }
            if (!decided) res=0;
        }
    }
    return (res);
}

bool   NauticalObjects::Adjacent(Attribute * arg)
{
    return 0;
    //for points which takes double values, we can not decides whether they are
    //adjacent or not.
}

int  NauticalObjects::Sizeof() const
{
    return sizeof(NauticalObjects);
}

NauticalObjects*  NauticalObjects::Clone()
{
    return (new NauticalObjects(*this));
}

ostream& NauticalObjects::Print( ostream &os )
{
    os << "<";
    for( int i = 0; i < Size(); i++ )
    {
        NauticalObject n;
        Get( i, n );
        os << " " << &n;
    }
    os << ">";

    return os;
}
/*
  ***************************************************
   End of the definition of the virtual functions.
  ***************************************************

*/

/*
3.2 List Representation

The list representation of a point is

----    (x y)
----
3.3 ~Out~-function

*/
static ListExpr
OutNauticalObjects( ListExpr typeInfo, Word value )
{
  //cout << "OutNauticalObjects" << endl;

  NauticalObjects* nobjects = (NauticalObjects*)(value.addr);
  if( nobjects->IsEmpty() )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    NauticalObject n;
    nobjects->Get( 0, n );
    ListExpr result = nl->OneElemList( OutNauticalObject( nl->TheEmptyList(), SetWord( &n ) ) );
    ListExpr last = result;

    for( int i = 1; i < nobjects->Size(); i++ )
    {
      nobjects->Get( i, n );
      last = nl->Append( last,
                         OutNauticalObject( nl->TheEmptyList(), SetWord( &n ) ) );
    }

    return result;
  }
}

/*
3.4 ~In~-function

*/
static Word
InNauticalObjects( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  cout << "InNauticalObjects" << endl;
  cout << "Anzahl Objekte:" << nl->ListLength( instance ) << endl;

  NauticalObjects* nobjects = new NauticalObjects( nl->ListLength( instance ) );
  nobjects->StartBulkLoad();
  ListExpr rest = instance;
  while( !nl->IsEmpty( rest ) )
  {
    ListExpr first = nl->First( rest );
    rest = nl->Rest( rest );

    NauticalObject *n = (NauticalObject*)InNauticalObject( nl->TheEmptyList(), first, 0, errorInfo, correct ).addr;
    if( correct )
    {
      (*nobjects) += (*n);
      delete n;
    }
    else
    {
      return SetWord( Address(0) );
    }
  }
  nobjects->EndBulkLoad();
  correct = true;
  return SetWord( nobjects );
}
/*
3.5 ~Create~-function

*/
static Word
CreateNauticalObjects( const ListExpr typeInfo )
{
//  cout << "CreateNauticalObjects" << endl;

  return (SetWord( new NauticalObjects( 0 ) ));
}

/*
3.6 ~Delete~-function

*/
static void
DeleteNauticalObjects( Word& w )
{
//  cout << "DeleteNauticalObjects" << endl;

  NauticalObjects *ns = (NauticalObjects *)w.addr;
  ns->Destroy();
  delete ns;
  w.addr = 0;
}

/*
3.7 ~Close~-function

*/
static void
CloseNauticalObjects( Word& w )
{
//  cout << "CloseNauticalObjects" << endl;

  delete (NauticalObjects *)w.addr;
  w.addr = 0;
}

/*
3.8 ~Clone~-function

*/
static Word
CloneNauticalObjects( const Word& w )
{
//  cout << "CloneNauticalObjects" << endl;

  NauticalObjects *p = new NauticalObjects( *((NauticalObjects *)w.addr) );
  return SetWord( p );
}

/*
3.8 ~SizeOf~-function

*/
static int
SizeOfNauticalObjects()
{
  return sizeof(NauticalObjects);
}

/*
3.11 Function describing the signature of the type constructor

*/
static ListExpr
NauticalObjectsProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("points"),
                             nl->StringAtom("(<point>*) where point is (<x><y>)"),
                             nl->StringAtom("( (10 1)(4 5) )"))));
}

/*
3.12 Kind checking function

This function checks whether the type constructor is applied correctly. Since
type constructor ~point~ does not have arguments, this is trivial.

*/
static bool
CheckPoints( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "nobjects" ));
}

/*
3.13 ~Cast~-function

*/
void* CastNauticalObjects(void* addr)
{
  return (new (addr) NauticalObjects);
}

/*
3.14 Creation of the type constructor instance

*/
TypeConstructor nauticalobjects(
        "nobjects",                  //name
        NauticalObjectsProperty,     //property function describing signature
        OutNauticalObjects,      InNauticalObjects,       //Out and In functions
        0,              0,           //SaveToList and RestoreFromList functions
        CreateNauticalObjects,   DeleteNauticalObjects,   //object creation and deletion
        0,     0,                    // object open and save
        CloseNauticalObjects,    CloneNauticalObjects,    //object close and clone
        CastNauticalObjects,         //cast function
        SizeOfNauticalObjects,       //sizeof function
        CheckPoints,                 //kind checking function
        0,                           //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );



/*
4 Class ~NauticalMap~

To define the Secondo type ~nauticalmap~, we need to (i) define a data structure,
that is, a class, (ii) to decide about a nested list representation, and (iii)
write conversion functions from and to nested list representation.

The function for converting from the list representation is the most involved
one, since it has to check that the given list structure is entirely correct.

Later we need (iv) a property function, (v) a kind checking function. Finally
the type constructor instance can be created.

*/


class NauticalMap
{
   public:
     NauticalMap();
     NauticalMap(Word rel, int relAlgebraId, int relTypeId);
     NauticalMap(string mapName, vector<NauticalObject> nObjects);
     ~NauticalMap();
     string                 GetMapName();
     vector<NauticalObject> GetObjects();
     void                   SetMapName(string mname);
     void                   SetObjects(vector<NauticalObject> nobjects);
     Word                   GetRelation();
     int                    GetRelAlgebraId();
     int                    GetRelTypeId();
     void                   SetRelation(Word rel);
     void                   SetRelAlgebraId(int aId);
     void                   SetRelTypeId(int tId);
     NauticalMap* Clone() { return new NauticalMap( *this ); }


   private:
     vector<NauticalObject> objects;
     string name; 
     Word   relation;
     int    relAlgebraId;
     int    relTypeId;
};

NauticalMap::NauticalMap(string oName, vector<NauticalObject> nObjects)
{
   name = oName;
   objects = nObjects;
}

NauticalMap::NauticalMap() 
{
   objects.clear();
   name = "";
}

NauticalMap::NauticalMap(Word rel, int aId, int tId) 
{
   objects.clear();
   name = "";
   relation = rel;
   relAlgebraId = aId;
   relTypeId = tId;
}

NauticalMap::~NauticalMap() {}

string NauticalMap::GetMapName() {return name;}
vector<NauticalObject> NauticalMap::GetObjects() {return objects;}

void NauticalMap::SetMapName(string mname) {name = mname;}
void NauticalMap::SetObjects(vector<NauticalObject> nobjects) {objects = nobjects;}

Word NauticalMap::GetRelation() {return relation;}
int  NauticalMap::GetRelAlgebraId() {return relAlgebraId;}
int  NauticalMap::GetRelTypeId() {return relTypeId;}
void NauticalMap::SetRelation(Word rel) {relation = rel;}
void NauticalMap::SetRelAlgebraId(int aId) {relAlgebraId = aId;}
void NauticalMap::SetRelTypeId(int tId) {relTypeId = tId;}
/*
4.2 List Representation

The list representation of an nauticalmap is:

---- (name nauticalObjects)
----
NauticalObjects is a nested lists itselve.

4.3 Object ~In~ and ~Out~ Functions

These functions use the ~In~ and ~Out~ functions of the elements of the nauticalMap.

*/
static Word 
InNauticalMap( const ListExpr typeInfo, const ListExpr instance,
         const int errorPos, ListExpr& errorInfo, bool& correct )
{
cout << "NauticalMap A" << endl;
  AlgebraManager* am = SecondoSystem::GetAlgebraManager();

  int relAlgebraId = am->GetAlgebraId("RelationAlgebra");
  cout << "Nummer Rel:" << relAlgebraId << endl;

  NauticalMap* newnauticalmap;
  Word relation;
  int algebraId;
  int typeId;
  ListExpr pair;

  if (nl->ListLength(instance) > 0) {

    ListExpr typeOfElement = nl->Second(typeInfo);
    ListExpr element = nl->First(instance);
//    ListExpr tuple = nl->First(element);

    if (nl->IsAtom(nl->First(typeOfElement))) {
       correct = false;
    }
    else
    {
       pair = nl->First(typeOfElement);
       algebraId = nl->IntValue(nl->First(pair));
       typeId = nl->IntValue(nl->Second(pair));

cout << "AlgebraId: " << algebraId << "TypeId: " << typeId << endl;
       if (algebraId == relAlgebraId) {
           correct = true;

           relation = ((am->InObj(algebraId, typeId))
                    (typeOfElement, element, errorPos, errorInfo, correct));

int aId;
int tId;
ListExpr tupleTypeInfo = nl->Second(typeOfElement);
extractIds(tupleTypeInfo, aId, tId);

extractIds(nl->First(tupleTypeInfo), aId, tId);

Relation* rel;
rel = Relation::In(typeOfElement, element, errorPos, errorInfo, correct);
RelationIterator* relIt;
relIt = rel->MakeScan();
Tuple* tuple;
tuple=relIt->GetNextTuple();

ListExpr TupleTypeInfo = nl->TwoElemList(nl->Second(typeOfElement),
          nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeOfElement)))));

ListExpr valuelist;
valuelist = tuple->Out(TupleTypeInfo);

if (nl->ListLength(valuelist) == 2)
{
    ListExpr Second = nl->Second(valuelist);

   cout <<  nl->IntValue(Second) <<  endl;
}
else
{
//       cout << "Halloabc" << endl;
    ListExpr NOBJ = nl->First(valuelist);

    if ( nl->IsAtom(nl->First(NOBJ)) && nl->AtomType(nl->First(NOBJ)) == StringType)
    {
       ListExpr position = nl->Second(NOBJ);

    }
}

TupleType tupleType = tuple->GetTupleType();
AttributeType attributeType = tupleType.GetAttributeType(0);


Attribute* attr;
attr=tuple->GetAttribute(0);
Word result = SetWord(attr);

SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);

string attrName = sc->GetTypeName( attributeType.algId, attributeType.typeId );

ListExpr attrType1 =  sc->GetTypeExpr(attrName);

relIt->~RelationIterator();
rel->Close();
       }
    }
    if (correct) {
      newnauticalmap = new NauticalMap(relation, algebraId, typeId);
      return SetWord(newnauticalmap);
    }
  }

  correct = false;
  return SetWord(Address(0));
}

static ListExpr
OutNauticalMap( ListExpr typeInfo, Word value )
{
cout << "NauticalMap B" << endl;
  AlgebraManager* am = SecondoSystem::GetAlgebraManager();

  NauticalMap* nauticalMap = (NauticalMap*)(value.addr);

  ListExpr typeOfElement = nl->Second(typeInfo);
  int pair = nl->First(typeOfElement);
  int algebraId = nl->IntValue(nl->First(pair));
  int typeId = nl->IntValue(nl->Second(pair));


  ListExpr list;
  ListExpr element;


  element = (am->OutObj(algebraId, typeId))
                             (typeOfElement, nauticalMap->GetRelation());
  list = nl->OneElemList(element);

  return list;
}

/*
4.4 Object ~RestoreFromList~ and ~SaveToList~ Functions

These functions use the ~RestoreFromList~ and ~SaveToList~ functions of the elements of the array.

*/

/*
4.5 Object ~Open~ and ~Save~ Functions

These functions are similar to the default ~Open~ and ~Save~ functions, but they are based on the *internal* list representation.

The original aim of this change was to handle arrays of ~btrees~, which do not have a list representation (for input and output), but which do have an *internal* list representation (namely a ["]list["] containing a file-id).

*/

/*
4.6 Object ~Creation~, ~Deletion~, ~Close~, ~Clone~ and ~SizeOf~ Functions

The ~Clone~ and the ~Close~ functions use the appropriate functions of the elements of the array. Additional details are explained within these function.

*/
Word 
CreateNauticalMap( const ListExpr typeInfo )
{
cout << "NauticalMap 0" << endl;
  return SetWord(new NauticalMap());
}

void 
DeleteNauticalMap( Word& w ) 
{
cout << "NauticalMap 1" << endl;
  w.addr = 0;
}

void CloseNauticalMap( Word& w ) 
{
cout << "NauticalMap 2" << endl;
  NauticalMap* nauticalMap = (NauticalMap*)w.addr;
  Word relation = nauticalMap->GetRelation();

  ((Relation*)relation.addr)->Delete();
  delete nauticalMap;
  w.addr = 0;
}

Word 
CloneNauticalMap( const Word& w )
{
  return SetWord( ((NauticalMap *)w.addr)->Clone() );
}

/*
Word
CloneNauticalMap( const Word& w )
{
cout << "NauticalMap 3" << endl;
  AlgebraManager* am = SecondoSystem::GetAlgebraManager();

  NauticalMap* nauticalMap = (NauticalMap*)w.addr;
  NauticalMap* newnauticalmap;

  bool ok = true;

  int n = array->getSize();
  int algebraId = array->getElemAlgId();
  int typeId = array->getElemTypeId();

  Word a[array->getSize()];

  for (int i=0; i < n; i++) {
    a[i] = (am->CloneObj(algebraId, typeId))(array->getElement(i));

    // Check whether cloning was successful

    ok = ok && (a[i].addr != 0);
  }

  if (ok) {
    newarray = new Array(algebraId, typeId, n, a);
  }
  else {

    // If the element's type just provides a dummy clone function, the clone
    // function of the array is also a dummy function.

    newarray = 0;
  }

  return SetWord(newarray);
}
*/

int
SizeOfNauticalMap()
{
cout << "NauticalMap 4" << endl;
  return sizeof(NauticalMap);
}

/*
4.7 Function Describing the Signature of the Type Constructor

The type of the elements of the array may be described by any valid type constructor, but the elements must have an internal list representation.

*/
static ListExpr
NauticalMapProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist, "The elements of the array must have an "
                              "internal list representation.");
  return (nl->TwoElemList(
            nl->FiveElemList(
              nl->StringAtom("Signature"),
              nl->StringAtom("Example Type List"),
              nl->StringAtom("List Rep"),
              nl->StringAtom("Example List"),
              nl->StringAtom("Remarks")),
            nl->FiveElemList(
              nl->StringAtom("typeconstructor -> NMAP"),
              nl->StringAtom("(nmap)"),
              nl->StringAtom("((rel(tuple((name string) (objects nobjects))))"),
              nl->StringAtom("(Seekarte (Boje (1.0 2.0)))"),
              remarkslist)));
}

/*
4.8 Kind Checking Function

The type constructor of an array is a list (array type). The first element of that list is the symbol atom "array" and the second element has to be a valid type constructor for the elements of the array.

So the second element can be an atom (e.g. int) or - in case of a more complex type - a nested list itself.

In order to achieve great flexibility, the element's type is not restricted to the tested types (see introduction). The user of an array has to make sure that the elements have an internal list representation, because this is not checked here.

*/
static bool
CheckNauticalMap( ListExpr type, ListExpr& errorInfo )
{
cout << "NauticalMap 5" << endl;
cout << nl->ListLength(type)  << endl;
  if (nl->ListLength(type) == 2) {

    ListExpr First = nl->First(type);
    ListExpr relDesc = nl->Second(type);

    if (nl->IsEqual(First, "nmap")) {

      // Check whether Second is a valid type constructor

      SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);

      if (sc->KindCorrect(relDesc, errorInfo)) {
         if (nl->IsEqual(nl->First(relDesc), "rel")) {
            ListExpr tupleDesc = nl->Second(relDesc);
            if (nl->IsEqual(nl->First(tupleDesc), "tuple"))
            {
               return true;
            }
         }
      } 
    }
  }

  return false;
}
/*
4.9 Creation of the Type Constructor Instance

Here an object of the class TypeConstructor is created. The constructor for an instance of the class TypeConstructor is called with the properties and functions for the array as parameters. The name of the type constructor is ~array~.

*/
TypeConstructor nauticalmap (
      "nmap",
      NauticalMapProperty,
      OutNauticalMap, InNauticalMap,
      0, 0,
      CreateNauticalMap, DeleteNauticalMap,
      0, 0,
      CloseNauticalMap, CloneNauticalMap,
      DummyCast,
      SizeOfNauticalMap,
      CheckNauticalMap,
      0,
      TypeConstructor::DummyInModel, 	
      TypeConstructor::DummyOutModel,
      TypeConstructor::DummyValueToModel,
      TypeConstructor::DummyValueListToModel );

/*
10 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

10.1 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

10.1.1 Type mapping function NauticalTypeMapBool

It is for the compare operators which have ~bool~ as resulttype, like =, !=, <,
<=, >, >=.

*/
static ListExpr
NauticalTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == stnobject && 
         TypeOfSymbol( arg2 ) == stnobject)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stnobjects && 
         TypeOfSymbol( arg2 ) == stnobjects)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1) == stnline && 
         TypeOfSymbol( arg2 ) == stnline)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stnregion && 
         TypeOfSymbol( arg2 ) == stnregion)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stnobject && 
         TypeOfSymbol( arg2 ) == stnobjects)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stnobjects && 
         TypeOfSymbol( arg2 ) == stnobject)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stnobject && 
         TypeOfSymbol( arg2 ) == stnline)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stnobject && 
         TypeOfSymbol( arg2 ) == stnregion)
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.2 Type mapping function GeoGeoMapBool

It is for the binary operators which have ~bool~ as result type, such as interscets,
inside, onborder, ininterior, etc.

*/

static ListExpr
GeoGeoMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if (((TypeOfSymbol( arg1 ) == stnobject)  ||
         (TypeOfSymbol( arg1 ) == stnobjects) ||
         (TypeOfSymbol( arg1 ) == stnline)     ||
         (TypeOfSymbol( arg1 ) == stnregion)) &&
        ((TypeOfSymbol( arg2 ) == stnobject)  ||
         (TypeOfSymbol( arg2 ) == stnobjects) ||
         (TypeOfSymbol( arg2 ) == stnline)     ||
         (TypeOfSymbol( arg2 ) == stnregion)))
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.3 Type mapping function SpatialTypeMapBool1

It is for the operator ~isempty~ which have ~nobject~, ~nobjects~, ~line~, and ~region~ as input and ~bool~ resulttype.

*/

static ListExpr
SpatialTypeMapBool1( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( TypeOfSymbol( arg1 ) == stnobject )
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stnobjects )
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stnline )
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stnregion )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.4 Type mapping function for operator ~intersection~

This type mapping function is the one for ~intersection~ operator. This is a SET operation
so that the result type is a set such as points, line, or region.

*/
static ListExpr
intersectionMap( ListExpr args )
{
    ListExpr arg1, arg2;
    if ( nl->ListLength( args ) == 2 )
    {
        arg1 = nl->First( args );
        arg2 = nl->Second( args );

        if ( TypeOfSymbol( arg1 ) == stnobject &&
             TypeOfSymbol( arg2 ) == stnobject )
            return (nl->SymbolAtom( "nobject" ));

        if ( TypeOfSymbol( arg1 ) == stnobject &&
             TypeOfSymbol( arg2 ) == stnobjects )
            return (nl->SymbolAtom( "nobject" ));

        if ( TypeOfSymbol( arg1 ) == stnobjects &&
             TypeOfSymbol( arg2 ) == stnobject )
            return (nl->SymbolAtom( "nobject" ));
        if ( TypeOfSymbol( arg1 ) == stnobject &&
             TypeOfSymbol( arg2 ) == stnline )
            return (nl->SymbolAtom( "nobject" ));

        if ( TypeOfSymbol( arg1 ) == stnline &&
             TypeOfSymbol( arg2 ) == stnobject )
            return (nl->SymbolAtom( "nobject" ));

        if ( TypeOfSymbol( arg1 ) == stnobject &&
             TypeOfSymbol( arg2 ) == stnregion )
            return (nl->SymbolAtom( "nobject" ));

        if ( TypeOfSymbol( arg1 ) == stnregion &&
             TypeOfSymbol( arg2 ) == stnobject )
            return (nl->SymbolAtom( "nobject" ));

        if ( TypeOfSymbol( arg1 ) == stnobjects &&
             TypeOfSymbol( arg2 ) == stnobjects )
            return (nl->SymbolAtom( "nobjects" ));

        if ( TypeOfSymbol( arg1 ) == stnobjects &&
             TypeOfSymbol( arg2 ) == stnline )
            return (nl->SymbolAtom( "nobjects" ));

        if ( TypeOfSymbol( arg1 ) == stnline &&
             TypeOfSymbol( arg2 ) == stnobjects )
            return (nl->SymbolAtom( "nobjects" ));

        if ( TypeOfSymbol( arg1 ) == stnobjects &&
             TypeOfSymbol( arg2 ) == stnregion )
            return (nl->SymbolAtom( "nobjects" ));

        if ( TypeOfSymbol( arg1 ) == stnregion &&
             TypeOfSymbol( arg2 ) == stnobjects )
            return (nl->SymbolAtom( "nobjects" ));

        if ( TypeOfSymbol( arg1 ) == stnline &&
             TypeOfSymbol( arg2 ) == stnline )
            return (nl->SymbolAtom( "nline" ));

        if ( TypeOfSymbol( arg1 ) == stnline &&
             TypeOfSymbol( arg2 ) == stnregion )
            return (nl->SymbolAtom( "nline" ));

        if ( TypeOfSymbol( arg1 ) == stnregion &&
             TypeOfSymbol( arg2 ) == stnline )
            return (nl->SymbolAtom( "nline" ));

        if ( TypeOfSymbol( arg1 ) == stnregion &&
             TypeOfSymbol( arg2 ) == stnregion )
            return (nl->SymbolAtom( "nregion" ));

    }
    return (nl->SymbolAtom( "typeerror" ));
}

static ListExpr
nmaptorelTypeMap( ListExpr args )
{
cout << "nmaptorelTypeMap" << endl;
  if (nl->ListLength(args) == 1)
  {
    ListExpr nmapDesc = nl->First(args);

    if (nl->ListLength(nmapDesc) == 2
        && nl->IsEqual(nl->First(nmapDesc), "nmap"))
    {
      ListExpr relDesc = nl->Second(nmapDesc);

      if (nl->ListLength(relDesc) == 2
          && nl->IsEqual(nl->First(relDesc), "rel"))
      {
        ListExpr tupleDesc = nl->Second(relDesc);
        if (nl->IsEqual(nl->First(tupleDesc), "tuple"))
        {
          return nl->TwoElemList(nl->SymbolAtom("rel"),
                                 nl->Second(relDesc));
        }
      }
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
nmaptorelFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
cout << "nmaptorelFun" << endl;
   NauticalMap* nauticalMap;
   nauticalMap= ((NauticalMap*)args[0].addr);

    SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);

    ListExpr resultType = qp->GetType(s);
    resultType = sc->NumericType(resultType);

    Word element = nauticalMap->GetRelation();

    Word clonedElement;

    int algebraId = nauticalMap->GetRelAlgebraId();
    int typeId = nauticalMap->GetRelTypeId();

    clonedElement = genericClone(algebraId, typeId, resultType, element);


   // result = nauticalMap->GetRelation();

    result.addr = clonedElement.addr;

   return 0;
}

const string nmaptorelSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>(nmap) -> (rel)</text--->"
      "<text>_ nmaptorel</text--->"
      "<text> Translate nmap to rel.</text--->"
      "<text>query seekarte nmaptorel consume</text---> ))";

Operator nmaptorel (
      "nmaptorel",
       nmaptorelSpec,
       nmaptorelFun,
       Operator::DummyModel,
       simpleSelect,
       nmaptorelTypeMap );



/*
5 Creating the Algebra

*/
class NauticalMapAlgebra : public Algebra
{
 public:
  NauticalMapAlgebra() : Algebra()
  {
    AddTypeConstructor( &nauticalmap );
    AddTypeConstructor( &nauticalobject );
    AddTypeConstructor( &nauticalobjects );

    AddOperator( &nmaptorel );

    nauticalmap.AssociateKind("DATA");
    nauticalobject.AssociateKind("DATA");
    nauticalobjects.AssociateKind("DATA");

  }
  ~NauticalMapAlgebra() {};
};

NauticalMapAlgebra nauticalmapAlgebra; 

/*
6 Initialization

["]Each algebra module needs an initialization function. The algebra manager has a reference to this function if this algebra is included in the list of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance of the algebra class and to provide references to the global nested list container (used to store constructor, type, operator and object information) and to the query processor.

The function has a C interface to make it possible to load the algebra dynamically at runtime.["] [Point02]

*/
extern "C"
Algebra*
InitializeNauticalMapAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&nauticalmapAlgebra);
}

/*
7 References

[Point02] Algebra Module PointRectangleAlgebra. FernUniversit[ae]t Hagen, Praktische Informatik IV, Secondo System, Directory ["]Algebras/PointRectangle["], file ["]PointRectangleAlgebra.cpp["], since July 2002

*/

