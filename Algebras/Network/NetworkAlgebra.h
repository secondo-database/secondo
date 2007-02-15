/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header File of Module Network Algebra

March 2004 Victor Almeida

[TOC]

1 Overview

This file contains the C++ headers necessary for the type constructors ~network~,
~gpoint~, and ~gline~.

A network is composed by routes, junctions, and sections. For that, the ~network~
class contains three relations: ~routes~, ~junctions~, and ~sections~. An operator
for the network creation is provided, namely ~thenetwork~. This operator receives
two relations containing the information about routes and junctions in the following
format:

----    routes( id: int, length: real, geometry: line, dual: bool, 
                startsSmaller: bool )
        junctions( r1id: int, r1meas: real, r2id: int, r2meas: real, 
                   cc: int )
----

The internal representation of the ~routes~ relation is the same as the relation passed
in the creation operator. For the ~junctions~ relation, some attributes are appended.
First, record identifications (~r1rc~, ~r2rc~, ~s1rc~, ~s2rc~) are added for 
direct access to the ~routes~ and ~sections~ relations. Second, a point (~pos~) containing 
the (x,y) position of the junction is also stored internally. Finally, as the junctions
are duplicated in this internal representation with the routes mixed, a pointer to the
partner junction ~partrc~ is added. The internal representation of the three relations 
is then:

----    routes( id: int, length: real, curve: line, dual: bool, 
                startsSmaller: bool )
        junctions( r1id: int, r1meas: real, r2id: int, r2meas: real, 
                   cc: int, pos: point, r1rc: tid, r2rc: tid,
                   s1rc: tid, s2rc: tid, partrc: tid )
        sections( rid: int, meas1: real, meas2: real, dual: bool, 
                  curve: line, rrc: tid )
----

The sections internal structure also contains the record identification for routes (~rrc~).

Three operators are provided to retrieve the information about routes, junctions, and 
sections. They return relations with the following format:

----    routes( id: int, length: real, geometry: line, dual: bool, 
                startsSmaller: bool )
        junctions( r1id: int, r1meas: real, r2id: int, r2meas: real, 
                   cc: int )
        sections( rid: int, meas1: real, meas2: real, dual: bool, 
                  curve: line )
----

Given that, we have internal and external representations of the ~routes~, ~junctions~, 
and ~sections~ relations. A B-Tree on the ~id~ attribute of the ~routes~ relation is
also provided.
 
2 Defines, includes, and constants

*/
#ifndef __NETWORK_ALGEBRA_H__
#define __NETWORK_ALGEBRA_H__

#include "NestedList.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "TupleIdentifier.h"
#include "SecondoSMI.h"

extern NestedList* nl;
extern QueryProcessor* qp;

/*
3 C++ Classes

3.2 Class ConnectivityCode

*/
enum Transition
{ Aup_Aup     = 0x0001,
  Aup_Adown   = 0x0002,
  Aup_Bup     = 0x0004,
  Aup_Bdown   = 0x0008,
  Adown_Aup   = 0x0010,
  Adown_Adown = 0x0020,
  Adown_Bup   = 0x0040,
  Adown_Bdown = 0x0080,
  Bup_Aup     = 0x0100,
  Bup_Adown   = 0x0200,
  Bup_Bup     = 0x0400,
  Bup_Bdown   = 0x0800,
  Bdown_Aup   = 0x1000,
  Bdown_Adown = 0x2000,
  Bdown_Bup   = 0x4000,
  Bdown_Bdown = 0x8000 };

class ConnectivityCode
{
  public:
    ConnectivityCode() {}
/*
The simple constructor

*/
    ConnectivityCode( int cc ):
    cc( cc )
    {}
/*
The first constructor. Constructs the connectivity code given an integer value ~cc~.

*/
    ConnectivityCode( bool aup_aup, bool aup_adown, 
                      bool aup_bup, bool aup_bdown,
                      bool adown_aup, bool adown_adown, 
                      bool adown_bup, bool adown_bdown,
                      bool bup_aup, bool bup_adown, 
                      bool bup_bup, bool bup_bdown,
                      bool bdown_aup, bool bdown_adown, 
                      bool bdown_bup, bool bdown_bdown ):
    cc( (aup_aup && Aup_Aup) | (aup_adown && Aup_Adown) |
        (aup_bup && Aup_Bup) | (aup_bdown && Aup_Bdown) |
        (adown_aup && Adown_Aup) | (adown_adown && Aup_Adown) |
        (adown_bup && Adown_Bup) | (adown_bdown && Aup_Bdown) |
        (bup_aup && Bup_Aup) | (bup_adown && Bup_Adown) |
        (bup_bup && Bup_Bup) | (bup_bdown && Bup_Bdown) |
        (bdown_aup && Bdown_Aup) | (bdown_adown && Bup_Adown) |
        (bdown_bup && Bdown_Bup) | (bdown_bdown && Bup_Bdown) )
    {}
/*
The second constructor. Constructs the connectivity code given the Boolean values for
all possibilities.

*/
    ConnectivityCode( const ConnectivityCode& c ):
    cc( c.cc )
    {}
/*
The copy constructor.

*/

    bool IsPossible( Transition t ) const
    {
      return t & cc;
    }
/*
Checks if a transition is possible.

*/

  private: 
    int cc;
};

/*
3.2 Class DirectedSection

*/
class DirectedSection
{
  public:
    DirectedSection() {}
/*
The simple constructor.

*/
    DirectedSection( bool updown, TupleIdentifier section ):
    updown( updown ), section( section )
    {}
/*
The constructor.

*/
    DirectedSection( const DirectedSection& s ):
    updown( s.updown ), section( s.section )
    {}
/*
The copy constructor.

*/

    DirectedSection& operator=( const DirectedSection& s )
    {
      updown = s.updown;
      section = s.section;
      return *this;
    }
/*
Redefinition of the assignment operator.

*/

  private:
    bool updown;
/*
A flag indicating the direction: ~true~ means up and ~false~ means down.

*/
    TupleIdentifier section;
/*
A pointer to the section.

*/
};

/*
3.2 Class AdjacencyListEntry

*/
struct AdjacencyListEntry
{
  AdjacencyListEntry() {}
/*
The simple constructor.

*/

  AdjacencyListEntry( int low, int high ):
  low( low ),
  high( high )
  {
  }
/*
The constructor.

*/

  AdjacencyListEntry( const AdjacencyListEntry& e ):
  low( e.low ), high( e.high )
  {
  }
/*
The copy constructor.

*/

  int low;
/*
The lower index in the adjacency lists sub-array.

*/
  int high;
/*
The higher index in the adjacency lists sub-array.

*/
};

/*
3.1 Class ~Network~

*/
class Network
{
  public:

    Network();
/*
The simple constructor.

*/

    Network( Relation *routes, Relation *junctions, 
             Relation *sections, BTree *routesBTree );
/*
The constructor that receives all information to create a network.

*/

    ~Network();
/*
The destructor.

*/

    void Destroy();
/*
This function sets all information inside the network to be destroyed when the destructor
is called.

*/

    void Load( const Relation *routes, const Relation *junctions );
/*
Loads a network given two relations containing the routes and junctions. This function corresponds
to the operator ~thenetwork~.

*/

    ListExpr Out( ListExpr typeInfo );
    ListExpr Save( SmiRecord& valueRecord, size_t& offset, 
                   const ListExpr typeInfo );
    static Network *Open( SmiRecord& valueRecord, size_t& offset, 
                          const ListExpr typeInfo );
/*
The ~Out~, ~Save~, and ~Open~ functions of the type constructor ~network~. The ~In~ function is not
provided, given that the creation of the network is only done via the ~thenetwork~ operator.

*/

    Relation *GetRoutes();
/*
Returns a copy of the ~routes~ relation in external representation. This function is used
in the ~routes~ operator.

*/

    Relation *GetRoutesInternal();
/*
Returns the ~routes~ relation (in internal representation).

*/

    Relation *GetJunctions();
/*
Returns a copy of the ~junctions~ relation in external representation. This function is used
in the ~junctions~ operator.

*/  

    Relation *GetJunctionsInternal();
/*
Returns the ~junctions~ relation (in internal representation).

*/

    Relation *GetSections();
/*
Returns a copy of the ~sections~ relation in external representation. This function is used in 
the ~sections~ operator.

*/

    Relation *GetSectionsInternal();
/*
Returns the ~sections~ relation (in internal representation).

*/

    static ListExpr GetRoutesTypeInfo();
/*
Returns the internal and external (they are equal) ~routes~ relation type info.

*/

    static ListExpr GetRoutesBTreeTypeInfo();
/*
Returns the B-Tree in the ~routes~ relation type info.

*/

    static ListExpr GetJunctionsTypeInfo();
/*
Returns the external ~junctions~ relation type info.

*/

    static ListExpr GetJunctionsIntTypeInfo();
/*
Returns the internal ~junctions~ relation type info.

*/

    static ListExpr GetSectionsTypeInfo();
/*
Returns the external ~sections~ relation type info.

*/

    static ListExpr GetSectionsInternalTypeInfo();
/*
Returns the internal ~sections~ relation type info.

*/

  private:

    enum PositionRoutesRelation { POS_RID = 0, POS_RLENGTH, POS_RCURVE, 
                                  POS_RDUAL, POS_RSTARTSSMALLER };
    enum PositionJunctionsRelation { POS_JR1ID = 0, POS_JMEAS1, POS_JR2ID, 
                                     POS_JMEAS2, POS_JCC, POS_JPOS, 
                                     POS_JR1RC, POS_JR2RC, POS_JS1RC, 
                                     POS_JS2RC, POS_JPARTRC };
    enum PositionSectionsRelation { POS_SRID = 0, POS_SMEAS1, POS_SMEAS2, 
                                    POS_SDUAL, POS_SCURVE, POS_SRRC };
/*
These enumerators are used to store the positions of each attribute in the relations to avoid
using numbers.

*/

    void FillRoutes( const Relation *routes );
/*
Copies the relation given as argument to the ~routes~ relations sorted by the ~id~ attribute
and creates the B-Tree in this attribute.

*/

    void FillJunctions( const Relation *junctions );
/*
Copies the relation given as argument to the ~junctions~ relation appending the new attributes.

*/

    void FillSections();
/*
Given the two ~routes~ and ~junctions~ relations, the ~sections~ relation is retrieved.

*/
    void FillAdjacencyLists();
/*
Given that all relations are set up, the adjacency lists are created.

*/
    static string routesTypeInfo;
/*
The internal and external (they are equal) ~routes~ relation type info as string.

*/

    static string routesBTreeTypeInfo;
/*
The B-Tree in the ~routes~ relation type info as string.

*/

    static string junctionsTypeInfo;
/*
The external ~junctions~ relation type info as string.

*/

    static string junctionsInternalTypeInfo;
/*
The internal ~junctions~ relation type info as string.

*/

    static string sectionsTypeInfo;
/*
The external ~sections~ relation type info as string.

*/

    static string sectionsInternalTypeInfo;
/*
The internal ~sections~ relation type info as string.

*/

    Relation *routes;
/*
The ~routes~ relation.

*/

    Relation *junctions;
/*
The ~junctions~ relation.

*/

    Relation *sections;
/*
The ~sections~ relation.

*/

    BTree *routesBTree;
/*
The B-Tree in the ~routes~ relation.

*/
    DBArray<AdjacencyListEntry> adjacencyList;
    DBArray<DirectedSection> subAdjacencyList;
/*
The adjacency lists of sections.

*/
};

enum Side { Up, Down, None };

struct RLoc
{
  RLoc() {}
/*
The simple constructor. Should not be used.

*/
  RLoc( const TupleId rid, const double d, const Side side ):
  rid( rid ), d( d ), side( side )
  {}
/*
The constructor.

*/
  RLoc( const RLoc& rloc ):
  rid( rloc.rid ), d( rloc.d ), side( rloc.side )
  {}
/*
The copy constructor.

*/
  RLoc& operator=( const RLoc& rloc )
  {
    rid = rloc.rid; d = rloc.d; side = rloc.side;
    return *this;
  }

  TupleId rid;
/*
The route id.

*/
  double d;
/*
The distance in the route.

*/
  Side side;
/*
The side in the route.

*/
};

class GPoint : public StandardAttribute
{
  public:
    GPoint() {}
/*
The simple constructor.

*/
    GPoint( bool defined,
            SmiRecordId nid = 0, 
            TupleId rid = 0, 
            double d = 0.0, 
            Side side = None ):
    nid( nid ), rloc( rid, d, side ),
    defined( defined )
    {}
/*
The constructor.

*/
    GPoint( const GPoint& gp ):
    nid( gp.nid ), rloc( gp.rloc ), defined( gp.defined )
    {}
/*
The copy constructor.

*/
    GPoint& operator=( const GPoint& gp )
    {
      defined = gp.defined;
      if( defined )
      {
        nid = gp.nid; 
        rloc = gp.rloc; 
      }
      return *this;
    }
/*
The assignement operator redefinition.

*/

    SmiRecordId GetNetworkId() const
    {
      return nid;
    }
/*
Returns the network id.

*/
    TupleId GetRouteId() const
    {
      return rloc.rid;
    }
/*
Returns the route id.

*/
    double GetPosition() const
    {
      return rloc.d;
    }
/*
Returns the relative position of the graph point in the route.

*/
    Side GetSide() const
    {
      return rloc.side;
    }
/*
Returns the side on the route of the graph point.

*/
    

    bool IsDefined() const
    {
      return defined;
    }

    void SetDefined( bool d )
    {
      defined = d;
    }

    size_t Sizeof() const
    {
      return sizeof(GPoint);
    }

    size_t HashValue() const
    {
      return 0;
    }

    void CopyFrom( const StandardAttribute* right )
    {
      const GPoint* gp = (const GPoint*)right;
      *this = *gp;
    }

    int Compare( const Attribute* arg ) const
    {
      return 0;
    }

    bool Adjacent( const Attribute *arg ) const
    {
      return false;
    }

    GPoint *Clone() const
    {
      return new GPoint( *this );
    }

    ostream& Print( ostream& os ) const
    {
      os << "GPoint::Print" << endl;
      return os;
    }

  private:
    
    SmiRecordId nid;
/*
The network id.

*/
    RLoc rloc;
/*
The route location.

*/
    bool defined;
/*
A flag indicating whether the instance is defined or not.

*/ 
};

struct RouteInterval
{
  TupleId rid;
/*
The route id.

*/
  double d1;
  double d2;
/*
The distance interval in the route.

*/
  Side side;
/*
The side in the route.

*/
};

class GLine
{
  public:
    GLine() {}
/*
The simple constructor. Should not be used.

*/
    GLine( const int nid, const int n ):
    nid( nid ), rints( n ) 
    {}
/*
The constructor.

*/
    GLine( const GLine& gl ):
    nid( gl.nid ), rints( gl.rints.Size() )
    {
      const RouteInterval *ri;
      for( int i = 0; i < gl.rints.Size(); i++ )
      {
        gl.rints.Get( i, ri );
        rints.Put( i, *ri );
      }
    }

  private:
    
    SmiRecordId nid;
/*
The network id.

*/
    DBArray<RouteInterval> rints;
/*
The array of route intervals.

*/
};

#endif // __NETWORK_ALGEBRA_H__
