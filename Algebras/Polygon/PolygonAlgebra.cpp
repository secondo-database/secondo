/*
//paragraph [1] title: [{\Large \bf ]   [}]



[1] Polygon Algebra

January 2003 VTA

This ~polygon~ example algebra is intended as a documentation of the state diagram of objects inside the Secondo system. This state diagram shown in Figure 1 is needed because objects can have some persistent part handled by its own algebra. 
Every object in the Secondo system must have a memory part and may have a persistent part not handled by the Secondo system, but by the algebra from which the object belongs.
In this algebra, the vertices of the polygon are persistent using a ~PArray~ (from Persistent Array) structure. 

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
#include "PArray.h"


#ifdef SECONDO_WIN32
#define Polygon SecondoPolygon
#endif

static NestedList* nl;
static QueryProcessor* qp;

/*

2 Data structures

2.1 Struct Vertex

*/
struct Vertex
{
  Vertex( int xcoord, int ycoord ):
    x( xcoord ), y( ycoord )
    {}
 
  Vertex():
    x( 0 ), y( 0 )
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
    Edge( Vertex& s, Vertex& e ) :
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

enum PolygonState { partial, complete, closed };

/*

2.3 Class Polygon

*/
class Polygon //: public Attribute 
{
	
  public:
    Polygon( SmiRecordFile *recordFile );
    Polygon( SmiRecordFile *recordFile, SmiRecordId recordId, bool update = true );
    ~Polygon();
    
//    int NumOfFLOBs();
//    FLOB *GetFLOB(int i);
//    int Compare(Attribute*);
//    int Sizeof();
    Polygon *Clone();
    bool IsDefined();

    void Append( Vertex& v );
    void Complete();
    bool Correct();
    void Close();
    void Destroy();
    int NoEdges();
    int NoVertices() { return NoEdges(); };
    Edge& GetEdge( int i );
    Vertex& GetVertex( int i );
    string GetState();
    SmiRecordId GetRecordId();

    friend ostream& operator <<( ostream& os, Polygon& p );

  private:
    int noVertices;
    PArray<Vertex> *vertices;
    PolygonState state;
};

	
/* 
2.3.1 Constructors.

This first constructor creates a new polygon.

*/
Polygon::Polygon( SmiRecordFile *recordFile ) :
  noVertices( 0 ),
  vertices( new PArray<Vertex>( recordFile ) ),
  state( partial ) 
{}

/*

This second constructor opens a polygon stored in
a file with ~id~ as record identification. The
state of the polygon created is ~complete~ because
it is necessary to be completed for a polygon to
be closed.

*/
Polygon::Polygon( SmiRecordFile *recordFile, SmiRecordId recordId, bool update ) :
  vertices( new PArray<Vertex>( recordFile, recordId, update ) ),
  state( complete ) 
{
  noVertices = vertices->Size();
}

/*

2.3.2 Destructor.

*Precondition* ~state == closed~.

*/
Polygon::~Polygon()
{ 
  assert( state == closed ); 
}
    
/*
2.3.3 NumOfFLOBs.

Not yet implemented. Needed to be a tuple attribute.

*/
//int Polygon::NumOfFLOBs() 
//{
//  return 0; 
//}
	
/*
2.3.4 GetFLOB

Not yet implemented. Needed to be a tuple attribute.

*/
//FLOB *Polygon::GetFLOB(int i) 
//{ 
//  return 0;
//}
    
/*
2.3.5 Compare

Not yet implemented. Needed to be a tuple attribute.

*/
//int Polygon::Compare(Attribute*)
//{ 
//  return 0; 
//}
	
/*

2.3.6 Sizeof

Not yet implemented. Needed to be a tuple attribute.

*/
//int Polygon::Sizeof()
//{ 
//  return 0; 
//}
	
/*
2.3.7 Clone

Returns a new created polygon (clone) which is a 
copy of ~this~.

*/
Polygon *Polygon::Clone() 
{
  assert( state == complete );
  Polygon *p = new Polygon( SecondoSystem::GetLobFile() );
  for( int i = 0; i < noVertices; i++ )
    p->Append( this->GetVertex( i ) );
  p->Complete();
  return p;
}
	
/*
2.3.8 IsDefined

*/
bool Polygon::IsDefined()
{
  return true;
}

/*
2.3.9 Append

Appends a vertex ~v~ at the end of the polygon.

*Precondition* ~state == partial~.

*/
void Polygon::Append( Vertex& v )
{
  assert( state == partial );
  vertices->Put( noVertices++, v );
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
2.3.12 Close

Turns the polygon into the ~closed~ state.

*Precondition* ~state == complete~.

*/
void Polygon::Close()
{
  assert( state == complete );
  delete vertices;
  state = closed;
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
  vertices->MarkDelete();
  delete vertices;
  state = closed;
}

/*
2.3.14 NoEdges

Returns the number of edges of the polygon.

*Precondition* ~state == complete~.

*/
int Polygon::NoEdges()
{
  assert( state == complete );
  return noVertices;
}

/*
2.3.15 GetVertex

Returns a vertex indexed by ~i~.

*Precondition* ~state == complete \&\& 0 <= i < noVertices~.

*/
Vertex& Polygon::GetVertex( int i )
{
  assert( state == complete );
  assert( 0 <= i && i < noVertices );

  static Vertex v;
  vertices->Get( i, v );

  return v;
}

/*
2.3.15 GetEdge

Returns an edge indexed by ~i~.

*Precondition* ~state == complete \&\& 0 <= i < noVertices~.

*/
Edge& Polygon::GetEdge( int i )
{
  assert( state == complete );
  assert( 0 <= i && i < noVertices );

  Vertex v, w;
  vertices->Get( i, v );
  vertices->Get( i+1, w );

  static Edge e( v, w );

  return e;
}

/*
2.3.16 GetState

Returns the state of the polygon in string format.

*/
string Polygon::GetState()
{
  switch( state )
  {
    case partial:
      return "partial";
    case complete:
      return "complete";
    case closed:
      return "closed";
  }
  return "";
}


/*
2.3.17 GetRecordId

Returns the record identification (~SmiRecordId~) of the
vertices array.

*/
SmiRecordId Polygon::GetRecordId()
{
  return vertices->Id();
}

/*
2.3.18 Print functions

*/
ostream& operator<<(ostream& os, Vertex& v)
{
  os << "(" << v.x << "," << v.y << ")";
  return os;
}


ostream& operator<<(ostream& os, Polygon& p)
{
  os << "RecordId: " << p.GetRecordId() 
     << " State: " << p.GetState()
     << "<";

  for(int i = 0; i < p.NoVertices(); i++)
    os << p.GetVertex( i ) << " ";

  os << ">";
  
  return os;
}


/*
3 Polygon Algebra.

3.1 List Representation

The list representation of a polygon is

----    ( (<recordId>) (x1 y1) (x2 y2) ... (xn yn) )
----

3.2 ~In~ and ~Out~ Functions

*/

static ListExpr
OutPolygon( ListExpr typeInfo, Word value )
{
  cout << "Polygon Algebra: Out" << endl;

  Polygon* polygon = (Polygon*)(value.addr);
  ListExpr result = nl->OneElemList( nl->OneElemList( nl->IntAtom( polygon->GetRecordId() ) ) );
  ListExpr last = result;

  for( int i = 0; i < polygon->NoVertices(); i++ )
  {
    last = nl->Append( last,
                       nl->TwoElemList( nl->IntAtom( polygon->GetVertex(i).x ), nl->IntAtom( polygon->GetVertex(i).y ) ) );
  }

  return result;
}

static Word
InPolygon( const ListExpr typeInfo, const ListExpr instance,
           const int errorPos, ListExpr& errorInfo, bool& correct )
{
  cout << "Polygon Algebra: In" << endl;

  Polygon* polygon;
  ListExpr first = nl->First( instance );

  if( nl->IsEmpty( first ) )
    // new polygon. Recordid unknown
  {
    polygon = new Polygon( SecondoSystem::GetLobFile() );
    ListExpr rest = nl->Rest( instance );
    while( !nl->IsEmpty( rest ) )
    {
      first = nl->First( rest ); 
      rest = nl->Rest( rest );

      if( nl->ListLength( first ) == 2 && 
          nl->IsAtom( nl->First( first ) ) && nl->AtomType( nl->First( first ) ) == IntType &&
          nl->IsAtom( nl->Second( first ) ) && nl->AtomType( nl->Second( first ) ) == IntType )
      {
        Vertex v( nl->IntValue( nl->First( first ) ), nl->IntValue( nl->Second( first ) ) );
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
  else if( nl->ListLength( first ) == 1 && nl->IsAtom( nl->First( first ) ) && nl->AtomType( nl->First( first ) ) == IntType )
    // persistent polygon. First list contains the recordid
  {
    polygon = new Polygon( SecondoSystem::GetLobFile(), nl->IntValue( nl->First( first ) ) );
    correct = true;
    return SetWord( polygon );
  }

  correct = false;
  return SetWord(Address(0));
}

/*
3.3 Function Describing the Signature of the Type Constructor

*/

static ListExpr
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
			     nl->StringAtom("(<point>*) where <point> is (<x> <y>)"), 
			     nl->StringAtom("( (3 4) (10 10) (8 2) (6 4) (3 4) )"),
			     nl->StringAtom("x- and y-coordinates must be of type int."))));
}

/*
3.4 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~polygon~ does not have arguments, this is trivial.

*/
static bool
CheckPolygon( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "polygon" ));
}

/*

3.5 ~Create~-function 

*/
Word CreatePolygon(const ListExpr typeInfo)
{
  cout << "Polygon Algebra: Create" << endl;

  Polygon* polygon = new Polygon( SecondoSystem::GetLobFile() );
  return ( SetWord(polygon) );
}

/*
3.6 ~Delete~-function

*/
void DeletePolygon(Word& w)
{
  cout << "Polygon Algebra: Delete" << endl;

  Polygon* polygon = (Polygon*)w.addr;

  polygon->Destroy();
  delete polygon;
}

/*
3.6 ~Open~-function

*/
bool
OpenPolygon( SmiRecord& valueRecord,
             const ListExpr typeInfo,
             Word& value )
{
  cout << "Polygon Algebra: Open" << endl;

  SmiRecordId recordId;
  SmiSize bytesRead;

  if( ( bytesRead = valueRecord.Read( &recordId, sizeof( SmiRecordId ), 0 ) ) != sizeof( SmiRecordId ) )
  {
    value = SetWord( Address(0) );
    return (false);
  }

  Polygon *polygon = new Polygon( SecondoSystem::GetLobFile(), recordId );
  value = SetWord( polygon );

  return (true);
}

/*
3.7 ~Save~-function

*/
bool
SavePolygon( SmiRecord& valueRecord,
             const ListExpr typeInfo,
             Word& value )
{
  cout << "Polygon Algebra: Save" << endl;

  Polygon *polygon = (Polygon*)value.addr;
  SmiRecordId recordId = polygon->GetRecordId();
  SmiSize bytesWritten;

  if( ( bytesWritten = valueRecord.Write( &recordId, sizeof( SmiRecordId ), 0 ) ) != sizeof( SmiRecordId ) )
    return (false);
  
  return (true);
}

/*
3.8 ~Close~-function

*/
void ClosePolygon(Word& w)
{
  cout << "Polygon Algebra: Close" << endl;

  Polygon* polygon = (Polygon*)w.addr;
  polygon->Close();
  delete polygon;
}

/*
3.9 ~Clone~-function

*/
Word ClonePolygon(const Word& w)
{
  cout << "Polygon Algebra: Clone" << endl;

  return SetWord( ((Polygon*)w.addr)->Clone() );
}

/*
3.10 ~Cast~-function

*/
void* CastPolygon(void* addr)
{
  return ( 0 );
}

/*
3.11 Creation of the Type Constructor Instance

*/
TypeConstructor polygon(
        "polygon",				//name
        PolygonProperty,			//property function describing signature
        OutPolygon,	InPolygon,		//Out and In functions
        CreatePolygon,  DeletePolygon,		//object creation and deletion
        OpenPolygon, SavePolygon,               //object open and save 
        ClosePolygon, ClonePolygon,		//object close and clone
        CastPolygon,                    	//cast function
        CheckPolygon,				//kind checking function
        0,					//predefined persistence function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
4 PolygonAlgebra

*/
class PolygonAlgebra : public Algebra 
{
  public:
    PolygonAlgebra() : Algebra() 
    {
      AddTypeConstructor( &polygon );
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


