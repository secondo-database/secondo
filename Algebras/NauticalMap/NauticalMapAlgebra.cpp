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

    cout << "extractIds:" << endl;
    nl->WriteListExpr(numType, cout);
    cout << endl;

  ListExpr pair;

  if (nl->IsAtom(nl->First(numType))) {
    pair = numType;
  }
  else
  {
    pair = nl->First(numType);
  }

    cout << "extractIds:" << endl;
    nl->WriteListExpr(nl->First(pair), cout);
    cout << endl;
  algebraId = nl->IntValue(nl->First(pair));
  cout << "Algebra : " << algebraId << endl;
  typeId = nl->IntValue(nl->Second(pair));
  cout << "Type : " << typeId << endl;
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
3 Auxiliary Functions

*/
ListExpr CreateObjectsTypeInfo()
{
  return
    nl->TwoElemList( nl->SymbolAtom( "rel" ),
      nl->TwoElemList( nl->SymbolAtom( "tuple" ),
        nl->TwoElemList( nl->TwoElemList( nl->SymbolAtom( "name" ),
                                          nl->SymbolAtom( "string" ) ),
                         nl->TwoElemList( nl->SymbolAtom( "object" ),
                                          nl->SymbolAtom( "point" ) ) ) ) );
}

ListExpr CreateLinesTypeInfo()
{
  return
    nl->TwoElemList( nl->SymbolAtom( "rel" ),
      nl->TwoElemList( nl->SymbolAtom( "tuple" ),
        nl->TwoElemList( nl->TwoElemList( nl->SymbolAtom( "name" ),
                                          nl->SymbolAtom( "string" ) ),
                         nl->TwoElemList( nl->SymbolAtom( "data" ),
                                          nl->SymbolAtom( "line" ) ) ) ) );
}

ListExpr CreateRegionsTypeInfo()
{
  return
    nl->TwoElemList( nl->SymbolAtom( "rel" ),
      nl->TwoElemList( nl->SymbolAtom( "tuple" ),
        nl->TwoElemList( nl->TwoElemList( nl->SymbolAtom( "name" ),
                                          nl->SymbolAtom( "string" ) ),
                         nl->TwoElemList( nl->SymbolAtom( "area" ),
                                          nl->SymbolAtom( "region" ) ) ) ) );
}

/*
2 Type Constructor ~nauticalObject~

A value of type ~nauticalObject~ represents an point-object in a seachart or is undefined.

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
  ****************************************************************
  The following 10 functions are used for porting points to Tuple.
  ****************************************************************

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
cout << "NauticalObject1" << endl;
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

cout << "NauticalObject2" << endl;
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
cout << "NauticalObject3" << endl;
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
cout << "NauticalObject4" << endl;
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
cout << "NauticalObject5" << endl;
  delete (NauticalObject *)w.addr;
  w.addr = 0;
}

static void
CloseNauticalObject( Word& w )
{
cout << "NauticalObject6" << endl;
  delete (NauticalObject *)w.addr;
  w.addr = 0;
}
static Word
CloneNauticalObject( const Word& w )
{
cout << "NauticalObject7" << endl;
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
cout << "NauticalObject8" << endl;
  return (nl->IsEqual( type, "nobject" ));
}
int
SizeOfNauticalObject()
{
cout << "NauticalObject9" << endl;
  return sizeof(NauticalObject);
}

/*
2.11 ~Cast~-function

*/
void* CastNauticalObject(void* addr)
{
cout << "NauticalObject10" << endl;
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


ListExpr NauticalMap::objectsTypeInfo = 0;
ListExpr NauticalMap::linesTypeInfo = 0;
ListExpr NauticalMap::regionsTypeInfo = 0;

NauticalMap::NauticalMap(const char *mName, int mScale, Relation *nobjects, Relation *nlines, Relation *nregions)
{
   cout << "Konstruktor 2" << endl;
   strcpy(name, mName);
   scale   = mScale;
   objects = nobjects;
   lines   =  nlines; 
   regions = nregions;
}
NauticalMap::~NauticalMap()
{
  delete objects;
  delete lines;
  delete regions;
}

void NauticalMap::Destroy()
{
  if (objects != 0)
     objects->Delete(); objects = 0;
  if (lines != 0)
     lines->Delete(); lines = 0;
  if (regions != 0)
     regions->Delete(); regions = 0;
}

const char* NauticalMap::GetMapName() const
{
   return name;
}

void NauticalMap::SetMapName(const char *mName)
{
   strcpy(name,mName);
}

const int NauticalMap::GetMapScale() const
{
   return scale;
}

void NauticalMap::SetMapScale(const int mScale)
{
   scale = mScale;
}

Word NauticalMap::GetObjects() {
    return SetWord(objects);
}
Word NauticalMap::GetLines() {
    return SetWord(lines);
}
Word NauticalMap::GetRegions() {
    return SetWord(regions);
}
void NauticalMap::CreateAllTypeInfos()
{
  objectsTypeInfo = CreateObjectsTypeInfo();
  linesTypeInfo = CreateLinesTypeInfo();
  regionsTypeInfo = CreateRegionsTypeInfo();
}

ListExpr NauticalMap::GetObjectsTypeInfo()
{
  if( objectsTypeInfo == 0 )
    CreateAllTypeInfos();

  return objectsTypeInfo;
}

ListExpr NauticalMap::GetLinesTypeInfo()
{
  if( linesTypeInfo == 0 )
    CreateAllTypeInfos();

  return linesTypeInfo;
}

ListExpr NauticalMap::GetRegionsTypeInfo()
{
  if( regionsTypeInfo == 0 )
    CreateAllTypeInfos();

  return regionsTypeInfo;
}

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
    cout << "InNauticalMap" << endl;
    AlgebraManager* am = SecondoSystem::GetAlgebraManager();
    SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);

    NauticalMap* newnauticalmap;
    int algebraId;
    int typeId;

    // Variables to create the NauticalMapAlgebra (members)
    Relation* nobjects=0;
    Relation* nlines=0;
    Relation* nregions=0;
    string mapName;
    int mapScale;

    // The nested list includes minimum one relation
    if (nl->ListLength(instance) > 2) {

        ListExpr First = nl->First(instance);
        ListExpr Second = nl->Second(instance);

        if ( nl->IsAtom(First) && nl->AtomType(First) == StringType
          && nl->IsAtom(Second) && nl->AtomType(Second) == IntType )
        {
            correct = true;
            mapName = nl->StringValue(First);
            mapScale = nl->IntValue(Second);

            cout << "Name/ Scale: " << mapName << "/ " << mapScale << endl;

            ListExpr firstRelation = nl->Third(instance);
            ListExpr pointRelInfo = sc->NumericType(CreateObjectsTypeInfo());
            ListExpr lineRelInfo = sc->NumericType(CreateLinesTypeInfo());
            ListExpr regionRelInfo = sc->NumericType(CreateRegionsTypeInfo());
   
            ListExpr pointTupleTypeInfo = nl->TwoElemList(nl->Second(pointRelInfo),
                     nl->IntAtom(nl->ListLength(nl->Second(nl->Second(pointRelInfo)))));
            ListExpr lineTupleTypeInfo = nl->TwoElemList(nl->Second(lineRelInfo),
                     nl->IntAtom(nl->ListLength(nl->Second(nl->Second(lineRelInfo)))));
            ListExpr regionTupleTypeInfo = nl->TwoElemList(nl->Second(regionRelInfo),
                     nl->IntAtom(nl->ListLength(nl->Second(nl->Second(regionRelInfo)))));
            Tuple* tupleaddr;
            tupleaddr = new Tuple(nl->First(pointTupleTypeInfo));
            int attrno = 1;
            algebraId = tupleaddr->GetTupleType().GetAttributeType( attrno ).algId;
            typeId = tupleaddr->GetTupleType().GetAttributeType( attrno ).typeId;
            cout << "AID: " << algebraId << "TID: " << typeId << endl;
    
  
            ListExpr valuelist = nl->First(firstRelation);
            ListExpr firstvalue = nl->Second(valuelist);
            nl->WriteListExpr(firstvalue, cout); 
            cout << endl;
            ListExpr attrlist =  nl->Second(nl->First(pointTupleTypeInfo));
            ListExpr secondAttr = nl->Second(attrlist);
       
            bool valueCorrect1, valueCorrect2, valueCorrect3;
     
            Word attr = (am->InObj(algebraId, typeId))(nl->First( nl->Rest(secondAttr) ),
                        firstvalue, 0, errorInfo, valueCorrect1);
   
            if (valueCorrect1 == true)
            {
                // nautical object relation
                correct = true;
                nobjects = Relation::In(pointRelInfo, firstRelation, errorPos, errorInfo, correct);
                cout << "Hallo2" << endl;
            }
            else 
            {
                tupleaddr = new Tuple(nl->First(lineTupleTypeInfo));
                algebraId = tupleaddr->GetTupleType().GetAttributeType( attrno ).algId;
                typeId = tupleaddr->GetTupleType().GetAttributeType( attrno ).typeId;
                cout << "AID: " << algebraId << "TID: " << typeId << endl;
                attrlist =  nl->Second(nl->First(lineTupleTypeInfo));
                secondAttr = nl->Second(attrlist);
                attr = (am->InObj(algebraId, typeId))(nl->First( nl->Rest(secondAttr) ),
                        firstvalue, 0, errorInfo, valueCorrect2);
                if (valueCorrect2 == true)
                {
                    // nautical line relation
                    correct = true;
                    nlines = Relation::In(lineRelInfo, firstRelation, errorPos, errorInfo, correct);
                    cout << "Hallo3" << endl;
                }
                else 
                {
                    tupleaddr = new Tuple(nl->First(regionTupleTypeInfo));
                    algebraId = tupleaddr->GetTupleType().GetAttributeType( attrno ).algId;
                    typeId = tupleaddr->GetTupleType().GetAttributeType( attrno ).typeId;
                    cout << "AID: " << algebraId << "TID: " << typeId << endl;
                    attrlist =  nl->Second(nl->First(regionTupleTypeInfo));
                    secondAttr = nl->Second(attrlist);
                    attr = (am->InObj(algebraId, typeId))(nl->First( nl->Rest(secondAttr) ),
                           firstvalue, 0, errorInfo, valueCorrect3);
                    if (valueCorrect3 == true)
                    {
                        // nautical area relation
                        correct = true;
                        nregions = Relation::In(regionRelInfo, firstRelation, errorPos, errorInfo, correct);
                       cout << "Hallo4" << endl;
                    }
                    else
                    {
                        correct = false;
                    }
                }
            }            

            // The nested list includes minimum two relations
            if ((correct == true) && (nl->ListLength(instance) > 3))
            {
                ListExpr secondRelation = nl->Fourth(instance);
                tupleaddr = new Tuple(nl->First(lineTupleTypeInfo));
                int attrno = 1;
                algebraId = tupleaddr->GetTupleType().GetAttributeType( attrno ).algId;
                typeId = tupleaddr->GetTupleType().GetAttributeType( attrno ).typeId;
                cout << "AID: " << algebraId << "TID: " << typeId << endl;

                valuelist = nl->First(secondRelation);
                firstvalue = nl->Second(valuelist);
                attrlist =  nl->Second(nl->First(lineTupleTypeInfo));
                secondAttr = nl->Second(attrlist);


                attr = (am->InObj(algebraId, typeId))(nl->First( nl->Rest(secondAttr) ),
                       firstvalue, 0, errorInfo, valueCorrect1);
 
                if (valueCorrect1 == true)
                {
                    // nautical line relation
                    correct = true;
                    nlines = Relation::In(lineRelInfo, secondRelation, errorPos, errorInfo, correct);
                    cout << "Hallo5" << endl;
                }
                else
                {
                    tupleaddr = new Tuple(nl->First(regionTupleTypeInfo));
                    algebraId = tupleaddr->GetTupleType().GetAttributeType( attrno ).algId;
                    typeId = tupleaddr->GetTupleType().GetAttributeType( attrno ).typeId;
                    cout << "AID: " << algebraId << "TID: " << typeId << endl;
                    attrlist =  nl->Second(nl->First(regionTupleTypeInfo));
                    secondAttr = nl->Second(attrlist);
                    attr = (am->InObj(algebraId, typeId))(nl->First( nl->Rest(secondAttr) ),
                            firstvalue, 0, errorInfo, valueCorrect2);
                    if (valueCorrect2 == true)
                    {
                        // nautical region relation
                        correct = true;
                        nregions = Relation::In(regionRelInfo, secondRelation, errorPos, errorInfo, correct);
                        cout << "Hallo6" << endl;
                    }
                    else
                    {
                        correct = false;
                    }
                }
                // The nested list includes three relations
                if ((correct == true) && (nl->ListLength(instance) > 4))
                {
                    ListExpr thirdRelation = nl->Fifth(instance);
                    tupleaddr = new Tuple(nl->First(regionTupleTypeInfo));
                    int attrno = 1;
                    algebraId = tupleaddr->GetTupleType().GetAttributeType( attrno ).algId;
                    typeId = tupleaddr->GetTupleType().GetAttributeType( attrno ).typeId;
                    cout << "AID: " << algebraId << "TID: " << typeId << endl;
     
                    valuelist = nl->First(thirdRelation);
                    firstvalue = nl->Second(valuelist);
                    attrlist =  nl->Second(nl->First(regionTupleTypeInfo));
                    secondAttr = nl->Second(attrlist);
     
  
                    attr = (am->InObj(algebraId, typeId))(nl->First( nl->Rest(secondAttr) ),
                           firstvalue, 0, errorInfo, valueCorrect1);
  
                    if (valueCorrect1 == true)
                    {
                        // nautical region relation
                        correct = true;
                        nregions = Relation::In(regionRelInfo, thirdRelation, errorPos, errorInfo, correct);
                        cout << "Hallo6" << endl;
                    }
                    else
                    {
                        correct = false;
                    }
                } // instance > 4
            } // instance > 3
        } // check the attributes from the nmap algebra

    } // instance > 2

    if (correct) {
        // Create the nautical map
        newnauticalmap = new NauticalMap(mapName.c_str(), mapScale, nobjects, nlines, nregions);
        return SetWord(newnauticalmap);
    }

    // return with error
    correct = false;
    return SetWord(Address(0));
}

static ListExpr
OutNauticalMap( ListExpr typeInfo, Word value )
{
   //nl->WriteListExpr( typeInfo, cout );
   cout << "OutNauticalMap" << endl;
   AlgebraManager* am = SecondoSystem::GetAlgebraManager();

   NauticalMap* nauticalMap = (NauticalMap*)(value.addr);

   // Lists to create the result list
   ListExpr list = nl->TheEmptyList();
   ListExpr firstRelation = nl->TheEmptyList();
   ListExpr secondRelation = nl->TheEmptyList();
   ListExpr thirdRelation = nl->TheEmptyList();

   // Get kind of relation

   if ((Relation *)nauticalMap->GetObjects().addr != 0)
   {
      // nautical object relation
      firstRelation = ((Relation *)nauticalMap->GetObjects().addr)->Out( CreateObjectsTypeInfo());
      if ((Relation *)nauticalMap->GetLines().addr != 0)
      {
         // nautical line relation
         secondRelation = ((Relation *)nauticalMap->GetLines().addr)->Out( CreateLinesTypeInfo());
         if ((Relation *)nauticalMap->GetRegions().addr != 0)
         {
            // nautical area relation
            thirdRelation = ((Relation *)nauticalMap->GetRegions().addr)->Out( CreateRegionsTypeInfo());
         }
      }
      else if ((Relation *)nauticalMap->GetRegions().addr != 0)
      {
         // nautical area relation
         secondRelation = ((Relation *)nauticalMap->GetRegions().addr)->Out( CreateRegionsTypeInfo());
      }
   }
   else if ((Relation *)nauticalMap->GetLines().addr != 0)
   {
      // nautical line relation
      firstRelation = ((Relation *)nauticalMap->GetLines().addr)->Out( CreateLinesTypeInfo());
      if ((Relation *)nauticalMap->GetRegions().addr != 0)
      {
         // nautical area relation
         secondRelation = ((Relation *)nauticalMap->GetRegions().addr)->Out( CreateRegionsTypeInfo());
      }
   }
   else if ((Relation *)nauticalMap->GetRegions().addr != 0)
   {
      // nautical area relation
      firstRelation = ((Relation *)nauticalMap->GetRegions().addr)->Out( CreateRegionsTypeInfo());
   }

   // Create the nested list structure and return it
   if (!(nl->IsEmpty(firstRelation)) && (nl->IsEmpty(secondRelation)) && 
        (nl->IsEmpty(thirdRelation)) ) 
   {
      list = nl->ThreeElemList(nl->StringAtom(nauticalMap->GetMapName()), 
                               nl->IntAtom(nauticalMap->GetMapScale()),
                                firstRelation);
   }
   else if (!(nl->IsEmpty(firstRelation)) && !(nl->IsEmpty(secondRelation)) && 
             (nl->IsEmpty(thirdRelation)) ) 
   {
      list = nl->FourElemList(nl->StringAtom(nauticalMap->GetMapName()), 
                              nl->IntAtom(nauticalMap->GetMapScale()),
                              firstRelation, secondRelation);
   }
   else if (!(nl->IsEmpty(firstRelation)) && !(nl->IsEmpty(secondRelation)) && 
            !(nl->IsEmpty(thirdRelation)) ) 
   {
      list = nl->FiveElemList(nl->StringAtom(nauticalMap->GetMapName()), 
                              nl->IntAtom(nauticalMap->GetMapScale()),
                              firstRelation, secondRelation, thirdRelation);
   }
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
cout << "NauticalMap0" << endl;
  return SetWord(new NauticalMap());
}

 /*
void 
DeleteNauticalMap( Word& w ) 
{
cout << "NauticalMap 1" << endl;
  w.addr = 0;
}
 */

/*
6.7 ~Delete~-function of type constructor ~network~

*/
void DeleteNauticalMap(Word& w)
{
cout << "NauticalMap1" << endl;
  NauticalMap* n = (NauticalMap*)w.addr;
  n->Destroy();
  delete n;
}

void CloseNauticalMap( Word& w ) 
{
cout << "NauticalMap2" << endl;
  NauticalMap* n = (NauticalMap*)w.addr;
  n->Destroy();
  delete n;
cout << "NauticalMap2a" << endl;
}

/*
6.6 ~Clone~-function of type constructor ~network~

Not implemented yet.

*/
Word CloneNauticalMap(const Word& w)
{
cout << "NauticalMap3" << endl;
  return SetWord( Address(0) );
}

 /*
Word
CloneNauticalMap( const Word& w )
{
cout << "NauticalMap3" << endl;
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
cout << "NauticalMap4" << endl;
  return sizeof(NauticalMap);
}

/*
4.7 Function Describing the Signature of the Type Constructor

The type of the elements of the array may be described by any valid type constructor, but the elements must have an internal list representation.

*/
static ListExpr
NauticalMapProperty()
{

  ListExpr exampletypelist = nl->TextAtom();
  nl->AppendText(exampletypelist,"(nmap (name string) (scale int) "
  "(rel(tuple((name string) (object point)))) "
  "(rel(tuple((name string) (sline line)))) "
  "(rel (tuple ((name string) (area region)))) ) ");
  ListExpr listreplist = nl->TextAtom();
  nl->AppendText(listreplist,"((name)(mapscale)(objects*)(lines*)(areas*)); "
  "where name is a string, mapscale is an int," 
  "objects is a relation (rel(tuple((name string)(object point)))),"
  "lines is a relation (rel(tuple((name string)(sline line)))),"
  "regions is a relation (rel(tuple((name string)(area region))))");
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"( \"Seekarte1\" 10000 ( ((boje2 (1.0 2.0))) ))" );
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist, "In the example only one nautical object is set. "
                              "Also nauticalmap can exists only with lines "
                              "or with regions");
  return (nl->TwoElemList(
            nl->FourElemList(
              nl->StringAtom("Signature"),
              nl->StringAtom("Example Type List"),
              //nl->StringAtom("List Rep"),
              nl->StringAtom("Example List"),
              nl->StringAtom("Remarks")),
            nl->FourElemList(
              nl->StringAtom("typeconstructor -> DATA"),
              //nl->StringAtom("nmap"),
              exampletypelist,
              //listreplist,
              examplelist,
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
  int length = nl->ListLength(type);
cout << "NauticalMap5" << endl;
cout << "Check lenght" << length  << endl;
   if (length >= 4) 
   {
      ListExpr First = nl->First(type);
      ListExpr stringDesc = nl->Second(type);
      ListExpr intDesc = nl->Third(type);
      ListExpr relDesc1 = nl->Fourth(type);

      // Is the nested list a nautical map?
      if (nl->IsEqual(First, "nmap")) 
      {
         // Is the first variable from type string and the second from type int?
         if ((nl->IsEqual(nl->Second(stringDesc), "string")) &&
             (nl->IsEqual(nl->Second(intDesc), "int"))) 
         {
            if (IsRelDescription(relDesc1))
            {
               // Is the first relation a nautical object relation?
               if (CompareSchemas( relDesc1, CreateObjectsTypeInfo()))
               {
                  if (length >= 5) 
                  {
                     ListExpr relDesc2 = nl->Fifth(type);
                     if (IsRelDescription(relDesc2))
                     {
                        // Is the second relation a nautical line relation?
                        if (CompareSchemas( relDesc2, CreateLinesTypeInfo()))
                        {
                           if (length == 6)
                           {
                              ListExpr relDesc3 = nl->Sixth(type);
                              if (IsRelDescription(relDesc3))
                              {
                                 // Is the second relation a nautical region relation?
                                 if (CompareSchemas( relDesc3, CreateRegionsTypeInfo()))
                                    return true;
                              }
                           }
                           else if (length == 5)
                              return true;
                        }
                        else if (CompareSchemas( relDesc2, CreateRegionsTypeInfo()))
                           return true;
                     }
                  }
                  else if (length == 4)
                     return true;
               }
               // Is the first relation a nautical line relation?
               else if (CompareSchemas( relDesc1, CreateLinesTypeInfo()))
               {
                  if (length == 5)
                  {
                     ListExpr relDesc2 = nl->Fifth(type);
                     if (IsRelDescription(relDesc2))
                     {
                        // Is the second relation a nautical region relation?
                        if (CompareSchemas( relDesc2, CreateRegionsTypeInfo()))
                           return true;
                     }
                  }
                  else if (length == 4)
                     return true;
               }
               // Is the first relation a nautical region relation?
               else if (CompareSchemas(relDesc1, CreateRegionsTypeInfo()) &&
                       (length == 4))
                  return true;
            }  // Has the first realtion the right description?
         }  // Are the first variables from type string and int?
      } // Is nested list a nautical map?
   } // length >= 4?

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
NauticalMapTypeMapObjectsRel( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    cout << "NauticalMapTypeMapObjectsRel:" << endl;
    nl->WriteListExpr(arg1, cout); 
    cout << endl;
    ListExpr First = nl->First(arg1);
    ListExpr stringDesc = nl->Second(arg1);
    ListExpr intDesc = nl->Third(arg1);
    ListExpr relDesc = nl->Fourth(arg1);
    cout << "Rel:" << endl;
    nl->WriteListExpr(relDesc, cout); 
    cout << endl;

      // Is the nested list a nautical map?
      if (nl->IsEqual(First, "nmap"))
      {
         // Is the first variable from type string and the second from type int?
         if ((nl->IsEqual(nl->Second(stringDesc), "string")) &&
             (nl->IsEqual(nl->Second(intDesc), "int")))
         {
            if (IsRelDescription(relDesc))
            {
               // Is the first relation a nautical object relation?
               if (CompareSchemas( relDesc, CreateObjectsTypeInfo()))
               {

                  return nl->TwoElemList(nl->SymbolAtom("rel"),
                                         nl->Second(relDesc));
               }
            }
        }
     }
  }
  return (nl->SymbolAtom( "typeerror" ));
}


static ListExpr
NauticalMapTypeMapLinesRel( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    cout << "NauticalMapTypeMapLinesRel:" << endl;
    nl->WriteListExpr(arg1, cout);
    cout << endl;
    ListExpr First = nl->First(arg1);
    ListExpr stringDesc = nl->Second(arg1);
    ListExpr intDesc = nl->Third(arg1);
    ListExpr relDesc1 = nl->Fourth(arg1);
    cout << "Rel:" << endl;
    nl->WriteListExpr(relDesc1, cout);
    cout << endl;

      // Is the nested list a nautical map?
      if (nl->IsEqual(First, "nmap"))
      {
         // Is the first variable from type string and the second from type int?
         if ((nl->IsEqual(nl->Second(stringDesc), "string")) &&
             (nl->IsEqual(nl->Second(intDesc), "int")))
         {
            if (IsRelDescription(relDesc1))
            {
               // Is the first relation a nautical lines relation?
               if (CompareSchemas( relDesc1, CreateLinesTypeInfo()))
               {
cout << "HAALLOO" << endl;

                  return nl->TwoElemList(nl->SymbolAtom("rel"),
                                         nl->Second(relDesc1));
               }
               else
               {
                  if (nl->ListLength( arg1 ) > 4)
                  {
                     ListExpr relDesc2 = nl->Fifth(arg1);
                     // Is the second relation a nautical lines relation?
                     if (CompareSchemas( relDesc2, CreateLinesTypeInfo()))
                     {
      
                        return nl->TwoElemList(nl->SymbolAtom("rel"),
                                               nl->Second(relDesc2));
                     }
                  }

               }
            }
        }
     }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

static ListExpr
NauticalMapTypeMapRegionsRel( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    cout << "NauticalMapTypeMapObjectsRel:" << endl;
    nl->WriteListExpr(arg1, cout);
    cout << endl;
    ListExpr First = nl->First(arg1);
    ListExpr stringDesc = nl->Second(arg1);
    ListExpr intDesc = nl->Third(arg1);
    ListExpr relDesc1 = nl->Fourth(arg1);
    cout << "Rel:" << endl;
    nl->WriteListExpr(relDesc1, cout);
    cout << endl;

      // Is the nested list a nautical map?
      if (nl->IsEqual(First, "nmap"))
      {
         // Is the first variable from type string and the second from type int?
         if ((nl->IsEqual(nl->Second(stringDesc), "string")) &&
             (nl->IsEqual(nl->Second(intDesc), "int")))
         {
            if (IsRelDescription(relDesc1))
            {
               // Is the first relation a nautical regions relation?
               if (CompareSchemas( relDesc1, CreateRegionsTypeInfo()))
               {

                  return nl->TwoElemList(nl->SymbolAtom("rel"),
                                         nl->Second(relDesc1));
               }
               else
               {
                  if (nl->ListLength( arg1 ) > 4)
                  {
                     ListExpr relDesc2 = nl->Fifth(arg1);
                     // Is the second relation a nautical regions relation?
                     if (CompareSchemas( relDesc2, CreateRegionsTypeInfo()))
                     {
                        return nl->TwoElemList(nl->SymbolAtom("rel"),
                                               nl->Second(relDesc2));
                     }
                     else
                     {
                        if (nl->ListLength( arg1 ) > 5)
                        {
                              ListExpr relDesc3 = nl->Sixth(arg1);
                              // Is the third relation a nautical regions relation?
                              if (CompareSchemas( relDesc3, CreateRegionsTypeInfo()))
                              {
                                 return nl->TwoElemList(nl->SymbolAtom("rel"),
                                                        nl->Second(relDesc3));
                              }
                        }
                     }

                  }
               }
            }
        }
     }
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

static int
getNauticalObjectsFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
cout << "getNauticalObjects" << endl;
   NauticalMap* nauticalMap;
   nauticalMap= ((NauticalMap*)args[0].addr);

    SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);

    ListExpr resultType = qp->GetType(s);
    cout << "resultType:" << endl;
    nl->WriteListExpr(resultType, cout);
    cout << endl;

    resultType = sc->NumericType(resultType);

    Word element = nauticalMap->GetObjects();

    Word clonedElement;

    int aId;
    int tId;

    extractIds(sc->NumericType(CreateObjectsTypeInfo()), aId, tId);
    cout << "AlgebraId: " << aId << "TypeId: " << tId << endl;
    clonedElement = genericClone(aId, tId, resultType, element);


   // result = nauticalMap->GetRelation();

    result.addr = clonedElement.addr;

   return 0;
}

const string getNauticalObjectsSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>(nmap) -> (rel)</text--->"
      "<text>_ getNauticalObjects</text--->"
      "<text> Translate nmap to rel.</text--->"
      "<text>query seekarte getNauticalObjects</text---> ))";

Operator getNauticalObjects (
      "getnauticalobjects",
       getNauticalObjectsSpec,
       getNauticalObjectsFun,
       Operator::DummyModel,
       simpleSelect,
       NauticalMapTypeMapObjectsRel );


static int
getNauticalLinesFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
cout << "getNauticalLines" << endl;
   NauticalMap* nauticalMap;
   nauticalMap= ((NauticalMap*)args[0].addr);

    SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);

    ListExpr resultType = qp->GetType(s);
    cout << "resultType:" << endl;
    nl->WriteListExpr(resultType, cout);
    cout << endl;

    resultType = sc->NumericType(resultType);

    Word element = nauticalMap->GetLines();

    Word clonedElement;

    int aId;
    int tId;

    extractIds(sc->NumericType(CreateLinesTypeInfo()), aId, tId);
    cout << "AlgebraId: " << aId << "TypeId: " << tId << endl;
    clonedElement = genericClone(aId, tId, resultType, element);


   // result = nauticalMap->GetRelation();

    result.addr = clonedElement.addr;

   return 0;
}

const string getNauticalLinesSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>(nmap) -> (rel)</text--->"
      "<text>_ getnauticallines</text--->"
      "<text> Translate nmap to rel.</text--->"
      "<text>query seekarte getnauticallines</text---> ))";

Operator getNauticalLines (
      "getnauticallines",
       getNauticalLinesSpec,
       getNauticalLinesFun,
       Operator::DummyModel,
       simpleSelect,
       NauticalMapTypeMapLinesRel );


static int
getNauticalRegionsFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
cout << "getNauticalRegionObjects" << endl;
   NauticalMap* nauticalMap;
   nauticalMap= ((NauticalMap*)args[0].addr);

    SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);

    ListExpr resultType = qp->GetType(s);
    cout << "resultType:" << endl;
    nl->WriteListExpr(resultType, cout);
    cout << endl;

    resultType = sc->NumericType(resultType);

    Word element = nauticalMap->GetRegions();

    Word clonedElement;

    int aId;
    int tId;

    extractIds(sc->NumericType(CreateRegionsTypeInfo()), aId, tId);
    cout << "AlgebraId: " << aId << "TypeId: " << tId << endl;
    clonedElement = genericClone(aId, tId, resultType, element);


   // result = nauticalMap->GetRelation();

    result.addr = clonedElement.addr;

   return 0;
}

const string getNauticalRegionsSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>(nmap) -> (rel)</text--->"
      "<text>_ getnauticalregions</text--->"
      "<text> Translate nmap to rel.</text--->"
      "<text>query seekarte getnauticalregions</text---> ))";

Operator getNauticalRegions (
      "getnauticalregions",
       getNauticalRegionsSpec,
       getNauticalRegionsFun,
       Operator::DummyModel,
       simpleSelect,
       NauticalMapTypeMapRegionsRel );



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

    AddOperator( &getNauticalObjects );
    AddOperator( &getNauticalLines );
    AddOperator( &getNauticalRegions );

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

