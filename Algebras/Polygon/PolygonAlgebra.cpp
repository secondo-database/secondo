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



[1] Polygon Algebra

January 2003 VTA

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

This ~polygon~ example algebra is intended as a documentation of the state diagram of objects inside the Secondo system. This state diagram shown in Figure 1 is needed because objects can have some persistent part handled by its own algebra.
Every object in the Secondo system must have a memory part and may have a persistent part not handled by the Secondo system, but by the algebra from which the object belongs.
In this algebra, the vertices of the polygon are persistent using a ~DBArray~ (from Persistent Array) structure.

In this way, objects, when they exist in the Secondo system, can stay into two states: \emph{opened} and \emph{closed}. Figure 1 shows the state diagram for objects in the Secondo system. When an object is in ~opened~ state, it has the memory part already loaded into memory and the files/records needed for the persistent part (if used) are opened and ready for use. When an object is in ~closed~ state the files/records needed for the persistent part are closed and the memory part is freed.


                Figure 1: Object state diagram [objstatediag.eps]

These six transition functions are implemented in the ~polygon~ algebra by the functions: ~CreatePolygon~, ~DeletePolygon~, ~OpenPolygon~, ~ClosePolygon~, ~SavePolygon~, and ~ClonePolygon~.

1 Preliminaries

1.1 Includes and global declarations

*/

#include <iostream>

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "SecondoSystem.h"
#include "DBArray.h"
#include "Attribute.h"

/*
GNU gcc 3.2 includes the header 'windows.h' from standard headers.
Therefore we need to change internally the name of the Polygon
class since Windows defines an API function 'Polygon'.

*/
#ifdef SECONDO_WIN32
#define Polygon SecondoPolygon
#endif

extern NestedList* nl;
extern QueryProcessor *qp;


namespace polygonalg{

/*

2 Data structures

2.1 Struct Vertex

*/
struct Vertex
{
  Vertex() {}
/*
Do not use this constructor.

*/

  Vertex( int xcoord, int ycoord ):
    x( xcoord ), y( ycoord )
    {}

  int x;
  int y;
};

/*

2.2 Class Edge

*/
class Edge
{
  public:
    Edge( const Vertex& s, const Vertex& e ) :
      start( s ), end( e )
      {}

    Vertex& Start()
      { return start; }

    Vertex& End()
      { return end; }

  private:
    Vertex start;
    Vertex end;
};

enum PolygonState { partial, complete };

/*

2.3 Class Polygon

*/
class Polygon : public Attribute
{

  public:
    Polygon() {}
/*
This constructor should not be used.

*/
    Polygon( const int n, const int *X = 0, const int *Y = 0 );
    ~Polygon();

    int NumOfFLOBs() const;
    FLOB *GetFLOB(const int i);
    int Compare(const Attribute*) const;
    bool Adjacent(const Attribute*) const;
    Polygon *Clone() const;
    bool IsDefined() const;
    void SetDefined( bool defined );
    size_t Sizeof() const;
    ostream& Print( ostream& os ) const;

    void Append( const Vertex &v );
    void Complete();
    bool Correct();
    void Destroy();
    int GetNoEdges() const { return GetNoVertices(); }
    int GetNoVertices() const;
    Edge& GetEdge( int i ) const;
    const Vertex *GetVertex( int i ) const;
    string GetState() const;
    const bool IsEmpty() const;

    friend ostream& operator <<( ostream& os, const Polygon& p );

  private:
    DBArray<Vertex> vertices;
    PolygonState state;
};

/*
2.3.18 Print functions

*/
ostream& operator<<(ostream& os, const Vertex& v)
{
  os << "(" << v.x << "," << v.y << ")";
  return os;
}


ostream& operator<<(ostream& os, const Polygon& p)
{
  os << " State: " << p.GetState()
     << "<";

  for(int i = 0; i < p.GetNoVertices(); i++)
    os << p.GetVertex( i ) << " ";

  os << ">";

  return os;
}

/*
2.3.1 Constructors.

This first constructor creates a new polygon.

*/
Polygon::Polygon( const int n, const int *X, const int *Y ) :
  vertices( n ),
  state( partial )
{
  if( n > 0 )
  {
    for( int i = 0; i < n; i++ )
    {
      Vertex v( X[i], Y[i] );
      Append( v );
    }
    Complete();
  }
}

/*

2.3.2 Destructor.

*/
Polygon::~Polygon()
{
}

/*
2.3.3 NumOfFLOBs.

Not yet implemented. Needed to be a tuple attribute.

*/
int Polygon::NumOfFLOBs() const
{
  return 1;
}

/*
2.3.4 GetFLOB

Not yet implemented. Needed to be a tuple attribute.

*/
FLOB *Polygon::GetFLOB(const int i)
{
  assert( i >= 0 && i < NumOfFLOBs() );
  return &vertices;
}

/*
2.3.5 Compare

Not yet implemented. Needed to be a tuple attribute.

*/
int Polygon::Compare(const Attribute*) const
{
  return 0;
}

/*
2.3.5 Adjacent

Not yet implemented. Needed to be a tuple attribute.

*/
bool Polygon::Adjacent(const Attribute*) const
{
  return 0;
}

/*
2.3.7 Clone

Returns a new created polygon (clone) which is a
copy of ~this~.

*/
Polygon *Polygon::Clone() const
{
  assert( state == complete );
  Polygon *p = new Polygon( 0 );
  for( int i = 0; i < GetNoVertices(); i++ )
    p->Append( *this->GetVertex( i ) );
  p->Complete();
  return p;
}

/*
2.3.8 IsDefined

*/
bool Polygon::IsDefined() const
{
  return true;
}

/*
2.3.8 SetDefined

*/
void Polygon::SetDefined( bool defined )
{
}

/*
2.3.8 Sizeof 

*/
size_t Polygon::Sizeof() const
{
  return sizeof( *this );
}

/*
2.3.8 Print

*/
ostream& Polygon::Print( ostream& os ) const
{
  return (os << *this);
}

/*
2.3.9 Append

Appends a vertex ~v~ at the end of the polygon.

*Precondition* ~state == partial~.

*/
void Polygon::Append( const Vertex& v )
{
  assert( state == partial );
  vertices.Append( v );
}

/*
2.3.10 Complete

Turns the polygon into the ~complete~ state.

*Precondition* ~state == partial~.

*/
void Polygon::Complete()
{
  assert( state == partial );
  state = complete;
}

/*
2.3.11 Correct

Not yet implemented.

*/
bool Polygon::Correct()
{
  return true;
}

/*
2.3.13 Destroy

Turns the polygon into the ~closed~ state destroying the
vertices array.

*Precondition* ~state == complete~.

*/
void Polygon::Destroy()
{
  assert( state == complete );
  vertices.Destroy();
}

/*
2.3.14 NoEdges

Returns the number of edges of the polygon.

*Precondition* ~state == complete~.

*/
int Polygon::GetNoVertices() const
{
  return vertices.Size();
}

/*
2.3.15 GetVertex

Returns a vertex indexed by ~i~.

*Precondition* ~state == complete \&\& 0 <= i < noVertices~.

*/
const Vertex *Polygon::GetVertex( int i ) const
{
  assert( state == complete );
  assert( 0 <= i && i < GetNoVertices() );

  const Vertex *v;
  vertices.Get( i, v );
  return v;
}

/*
2.3.15 GetEdge

Returns an edge indexed by ~i~.

*Precondition* ~state == complete \&\& 0 <= i < noVertices~.

*/
Edge& Polygon::GetEdge( int i ) const
{
  assert( state == complete );
  assert( 0 <= i && i < GetNoVertices() );

  const Vertex *v, *w;
  vertices.Get( i, v );
  vertices.Get( i+1, w );

  static Edge e( *v, *w );

  return e;
}

/*
2.3.16 GetState

Returns the state of the polygon in string format.

*/
string Polygon::GetState() const
{
  switch( state )
  {
    case partial:
      return "partial";
    case complete:
      return "complete";
  }
  return "";
}


/*
2.3.18 IsEmpty

Returns if the polygon is empty or not.

*/
const bool Polygon::IsEmpty() const
{
  assert( state == complete );
  return GetNoVertices() == 0;
}

/*
3 Polygon Algebra.

3.1 List Representation

The list representation of a polygon is

----    ( (<recordId>) (x1 y1) (x2 y2) ... (xn yn) )
----

3.2 ~In~ and ~Out~ Functions

*/

ListExpr
OutPolygon( ListExpr typeInfo, Word value )
{
  Polygon* polygon = (Polygon*)(value.addr);

  if( polygon->IsEmpty() )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    ListExpr result = 
      nl->OneElemList( 
        nl->TwoElemList( 
          nl->IntAtom( polygon->GetVertex(0)->x ), 
          nl->IntAtom( polygon->GetVertex(0)->y ) ) );
    ListExpr last = result;

    for( int i = 1; i < polygon->GetNoVertices(); i++ )
    {
      last = nl->Append( last,
                         nl->TwoElemList( 
                           nl->IntAtom( polygon->GetVertex(i)->x ), 
                           nl->IntAtom( polygon->GetVertex(i)->y ) ) );
    }
    return result;
  }
}

Word
InPolygon( const ListExpr typeInfo, const ListExpr instance,
           const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Polygon* polygon = new Polygon( 0 );

  ListExpr first = nl->Empty();
  ListExpr rest = instance;
  while( !nl->IsEmpty( rest ) )
  {
    first = nl->First( rest );
    rest = nl->Rest( rest );

    if( nl->ListLength( first ) == 2 &&
        nl->IsAtom( nl->First( first ) ) && 
        nl->AtomType( nl->First( first ) ) == IntType &&
        nl->IsAtom( nl->Second( first ) ) && 
        nl->AtomType( nl->Second( first ) ) == IntType )
    {
      Vertex v( nl->IntValue( nl->First( first ) ), 
                nl->IntValue( nl->Second( first ) ) );
      polygon->Append( v );
    }
    else
    {
      correct = false;
      return SetWord( Address(0) );
    }
  }
  polygon->Complete();
  correct = true;
  return SetWord( polygon );
}

/*
3.3 Function Describing the Signature of the Type Constructor

*/

ListExpr
PolygonProperty()
{
  return (nl->TwoElemList(
         nl->FiveElemList(nl->StringAtom("Signature"),
                          nl->StringAtom("Example Type List"),
                          nl->StringAtom("List Rep"),
                          nl->StringAtom("Example List"),
                          nl->StringAtom("Remarks")),
         nl->FiveElemList(nl->StringAtom("-> DATA"),
                          nl->StringAtom("polygon"),
                          nl->StringAtom("(<point>*) where <point> is "
                          "(<x> <y>)"),
                          nl->StringAtom("( (3 4) (10 10) (8 2) (6 4) "
                          "(3 4) )"),
                          nl->StringAtom("x- and y-coordinates must be of "
                          "type int."))));
}

/*
3.4 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~polygon~ does not have arguments, this is trivial.

*/
bool
CheckPolygon( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "polygon" ));
}

/*

3.5 ~Create~-function

*/
Word CreatePolygon(const ListExpr typeInfo)
{
  Polygon* polygon = new Polygon( 0 );
  return ( SetWord(polygon) );
}

/*
3.6 ~Delete~-function

*/
void DeletePolygon(const ListExpr typeInfo, Word& w)
{
  Polygon* polygon = (Polygon*)w.addr;

  polygon->Destroy();
  delete polygon;
}

/*
3.6 ~Open~-function

*/
bool
OpenPolygon( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  Polygon *p = (Polygon*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord( p );
  return true;
}

/*
3.7 ~Save~-function

*/
bool
SavePolygon( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  Polygon *p = (Polygon *)value.addr;
  Attribute::Save( valueRecord, offset, typeInfo, p );
  return true;
}

/*
3.8 ~Close~-function

*/
void ClosePolygon(const ListExpr typeInfo, Word& w)
{
  Polygon* polygon = (Polygon*)w.addr;
  delete polygon;
}

/*
3.9 ~Clone~-function

*/
Word ClonePolygon(const ListExpr typeInfo, const Word& w)
{
  return SetWord( ((Polygon*)w.addr)->Clone() );
}

/*
3.9 ~SizeOf~-function

*/
int SizeOfPolygon()
{
  return sizeof(Polygon);
}

/*
3.10 ~Cast~-function

*/
void* CastPolygon(void* addr)
{
  return (new (addr) Polygon);
}

/*
3.11 Creation of the Type Constructor Instance

*/
TypeConstructor polygon(
        "polygon",                      //name
        PolygonProperty,                //property function describing signature
        OutPolygon,     InPolygon,      //Out and In functions
        0,              0,              //SaveTo and RestoreFrom List functions
        CreatePolygon,  DeletePolygon,  //object creation and deletion
        OpenPolygon,    SavePolygon,    //object open and save
        ClosePolygon,   ClonePolygon,   //object close and clone
        CastPolygon,                    //cast function
        SizeOfPolygon,                  //sizeof function
        CheckPolygon );                 //kind checking function

/*
4 PolygonAlgebra

*/
class PolygonAlgebra : public Algebra
{
  public:
    PolygonAlgebra() : Algebra()
    {
      AddTypeConstructor( &polygon );

      polygon.AssociateKind( "DATA" );
    }
    ~PolygonAlgebra() {};
};

PolygonAlgebra polygonAlgebra;

/*

5 Initialization

*/

extern "C"
Algebra*
InitializePolygonAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return (&polygonAlgebra);
}

} // end of namespace
