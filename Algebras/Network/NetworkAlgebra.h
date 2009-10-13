/*
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

*/
/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]

1.1 Declarations Necessary for Algebra Network

*/
#ifndef __NETWORK_ALGEBRA_H__
#define __NETWORK_ALGEBRA_H__

#include "NestedList.h"
#include "TupleIdentifier.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"
#include "StandardAttribute.h"
#include "SpatialAlgebra.h"
#include "RTreeAlgebra.h"

/*
1.1.2 struct SectTreeEntry

The struct ~SectTreeEntry~ is used for the shortest path computation. The
sections already visited by the Dijkstra's algorithm for shortest path search
will be stored sorted by their section tuple ids in a tree. ~secttid~ is the
tuple id of the section in the sections relation of the network. ~rid~ is
the route identifier of the route the section belongs to. ~start~ respectively
~end~ gives the position on the route as distance from the start of the route
for the start and the end point of the section. ~startbool~ and ~endbool~ tell
us if the whole section is part described or only a part of the section.

*/

struct SectTreeEntry{
  SectTreeEntry() {};

  SectTreeEntry(TupleId n, int r, double st, double e, bool stb, bool eb) {
    secttid = n;
    rid = r;
    start = st;
    end = e;
    startbool = stb;
    endbool = eb;
  };

  ~SectTreeEntry(){};

  SectTreeEntry& operator=(const SectTreeEntry nEntry) {
    secttid = nEntry.secttid;
    rid = nEntry.rid;
    start = nEntry.start;
    end = nEntry.end;
    startbool = nEntry.startbool;
    endbool = nEntry.endbool;
    return *this;
  };

  TupleId secttid;
  int rid;
  double start, end;
  bool startbool, endbool;
};

class GPoints;
class Network;
class GLine;

/*
2 GLine

Every ~gline~ consists of a set of ~RouteInterval~s stored in a ~DBArray~.

2.1 struct RouteInterval
Each ~RouteInterval~ consists of a ~rid~, ~startpos~ and ~endpos~. Telling to
which route of the network the route interval belongs to. And giving the start
and end position of the ~RouteInterval~ of this route.

Different from the definitions in the original paper the implemented
~RouteInterval~ does not contain a side value. This should be changed to enable
us for example to store traffic jams as possibly ~moving(gline)~ values.

*/

class RouteInterval
{
  public:
   RouteInterval()
  {
  };

  RouteInterval(int in_iRouteId,
                double in_dStart,
                double in_dEnd):
      m_iRouteId(in_iRouteId),
      m_dStart(in_dStart),
      m_dEnd(in_dEnd) {};

  RouteInterval(RouteInterval& ri) :
    m_iRouteId (ri.m_iRouteId),
    m_dStart (ri.m_dStart),
    m_dEnd (ri.m_dEnd)
    {};

  RouteInterval(const RouteInterval& ri):
      m_iRouteId(ri.m_iRouteId),
      m_dStart(ri.m_dStart),
      m_dEnd(ri.m_dEnd) {};

  ~RouteInterval() {};

/*
Computes the spatial Bounding Box of the given ~RouteInterval~ as 2 dimensional
rectangle. Using the subline of the route curve defined by the given ~route-
interval~.

*/
  
  Rectangle<2> BoundingBox(Network* pNetwork) const;

/*
Get and Set private values.

*/
  inline int GetRouteId() const {return m_iRouteId;};
  inline double GetStartPos() const {return m_dStart;};
  inline double GetEndPos() const {return m_dEnd;};
  inline void SetRouteId(int rid) {m_iRouteId = rid;};
  inline void SetStartPos(double pos) {m_dStart = pos;};
  inline void SetEndPos(double pos){m_dEnd = pos;};

  private:
/*
The route id.

*/
  
  int m_iRouteId;

/*
Start position on route.

*/

  double m_dStart;

/*
End position on route.

*/

  double m_dEnd;
};



/*
3 class GPoint

3.1 enumeration of side

Attention in the original paper the enumeration of the side value is given by
Down, Up, None!!!

*/
enum Side { Down, Up, None };

/*
3.2 struct for route location (~rloc~)
Every single position in the network is given by an route location. That means
by a route id ~rid~, the distance of the position from the start of the route
~d~, and the side ~side~ value. If the route is simple side is always ~None~.

*/

struct RLoc
{
  RLoc() {}
/*

Constuructors:

The simple constructor. Should not be used.

*/
  RLoc( const int rid,
        const double d,
        const Side side ):
    rid( rid ), d( d ), side( side )
  {}

  RLoc( const RLoc& rloc ):
    rid( rloc.rid ), d( rloc.d ), side( rloc.side )
  {}

/*
Assignment operator

*/

  RLoc& operator=( const RLoc& rloc )
  {
    rid = rloc.rid; d = rloc.d; side = rloc.side;
    return *this;
  }

  void SetRouteId(int r){rid = r;}
  void SetPosition(double p) {d = p;}
  void SetSide(Side s){side = s;}
/*

Private fields.

Route Id

*/

  int rid;

/*
The distance from the 0.0 point of the route.

*/

  double d;

/*
side value

*/

  Side side;
};

/*
3.4 class ~gpoint~

Represents single positions in the network. Each ~GPoint~ consists of a network
id the ~gpoint~ belongs to, a ~rloc~ and a boolean defined flag. Because
~gpoint~ should be usable in relations the class ~gpoint~ derives from
class StandardAttribute.

*/

class GPoint : public StandardAttribute
{
  public:

/*
3.4.1 Constructors and Destructor

*/

    GPoint()
    {
      del.refs=1;
      del.isDelete=true;
    }

    GPoint( bool in_bDefined,
            int in_iNetworkId = 0,
            int in_xRid = 0,
            double in_dLocation = 0.0,
            Side in_xSide = None ):
    m_iNetworkId( in_iNetworkId ),
    m_xRouteLocation( in_xRid, in_dLocation, in_xSide ),
    m_bDefined( in_bDefined )
    {
      del.refs=1;
      del.isDelete=true;
    }

    GPoint( const GPoint& in_xOther ):
    m_iNetworkId( in_xOther.m_iNetworkId ),
    m_xRouteLocation( in_xOther.m_xRouteLocation ),
    m_bDefined( in_xOther.m_bDefined )
    {
      del.refs=1;
      del.isDelete=true;
    }

    ~GPoint(){};

/*
3.4.2 Methods of class ~gpoint~

*/
    
    GPoint& operator=( const GPoint& in_xOther )
    {
      m_bDefined = in_xOther.m_bDefined;
      if( m_bDefined )
      {
        m_iNetworkId = in_xOther.m_iNetworkId;
        m_xRouteLocation = in_xOther.m_xRouteLocation;
      }
      return *this;
    }

/*
Get Methods of ~gpoint~

*/

    inline int GetNetworkId() const
    {
      return m_iNetworkId;
    }

    inline int GetRouteId() const
    {
      return m_xRouteLocation.rid;
    }

    inline double GetPosition() const
    {
      return m_xRouteLocation.d;
    }

    inline Side GetSide() const
    {
      return m_xRouteLocation.side;
    }

    inline bool IsDefined() const
    {
      return m_bDefined;
    }

/*
Set Methods of ~gpoint~

*/

    inline void SetRouteId( int in_rid )
    {
      m_xRouteLocation.SetRouteId(in_rid);
    }

    inline void SetPosition( double pos )
    {
      m_xRouteLocation.SetPosition(pos);
    }

    inline void SetSide( Side s )
    {
      m_xRouteLocation.SetSide(s);
    }
    
    inline void SetDefined( bool in_bDefined )
    {
      m_bDefined = in_bDefined;
    }

    size_t Sizeof() const
    {
      return sizeof(GPoint);
    }

    size_t HashValue() const
    {
      size_t hash = m_iNetworkId + m_xRouteLocation.rid +
          (size_t) m_xRouteLocation.d;
      return hash;
    }

    void CopyFrom( const StandardAttribute* right )
    {
      const GPoint* gp = (const GPoint*)right;
      *this = *gp;
    }

    int Compare( const Attribute* arg ) const
    {
      const GPoint *p = (const GPoint*) arg;
      if (m_iNetworkId < p->GetNetworkId()) {
        return -1;
      } else {
        if (m_iNetworkId > p->GetNetworkId()) {
          return 1;
        } else { // same network
          if (m_xRouteLocation.rid < p->GetRouteId()){
            return -1;
          }else {
            if (m_xRouteLocation.rid > p->GetRouteId()){
              return 1;
            } else{ //same route
              if (m_xRouteLocation.d < p->GetPosition()){
                return -1;
              } else {
                if (m_xRouteLocation.d > p->GetPosition()){
                  return 1;
                } else { //same Position
                  if (m_xRouteLocation.side == 2 || p->GetSide() == 2 ||
                      m_xRouteLocation.side == p->GetSide()){
                    return 0;
                  } else {
                    if (m_xRouteLocation.side < p->GetSide()) {
                      return -1;
                    } else {
                      return 1;
                    }
                  }
                }
              }
            }
          }
        }
      }
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
      os << "NetworkId: " << m_iNetworkId
          << " RouteId: " << m_xRouteLocation.rid
          << "  Position: " << m_xRouteLocation.d
          << " Side: " << m_xRouteLocation.side << endl;
      return os;
    }

/*
Functions for Secondo integration.

*/
    static ListExpr OutGPoint( ListExpr typeInfo, Word value );

    static Word InGPoint( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct );

    static Word CreateGPoint( const ListExpr typeInfo );

    static void DeleteGPoint( const ListExpr typeInfo, Word& w );

    static void CloseGPoint( const ListExpr typeInfo, Word& w );

    static Word CloneGPoint( const ListExpr typeInfo, const Word& w );

    static void* CastGPoint( void* addr );

    static int SizeOfGPoint();

    static ListExpr GPointProperty();

    static bool CheckGPoint( ListExpr type, ListExpr& errorInfo );

/*
Returns the network distance between 2 ~gpoint~

*/

    double Netdistance (GPoint* toGPoint);
    double NewNetdistance(GPoint* pToGPoint,GLine* res);//new

/*
Computes the euclidean distance of 2 glines.

*/
    double Distance (GPoint* toGPoint);

/*
Returns true if the gpoint is inside the gline false elsewhere.

*/

    bool Inside(GLine *gl);

/*
Returns true if two gpoint are identical.

*/

    bool operator== (const GPoint& p) const;

    /*
    Returns false if two gpoint are identical.

    */

    bool operator!= (const GPoint&p) const;

    /*
    Translates a ~gpoint~ into a ~point~ in the 2D plane.

    */

    void ToPoint(Point *& res);

    Point* ToPoint() const;

    Point* ToPoint(Network *&pNetwork) const;

/*
Returns the spatial Bounding Box of the point which is a rectangle degenerated
to a single point.

*/

    inline const Rectangle<2> BoundingBox(){
      if (m_bDefined) {
        Point *p = ToPoint();
        Rectangle<2> result = p->BoundingBox();
        delete p;
        return result;
      } else return Rectangle<2> (false, 0.0, 0.0, 0.0, 0.0);
    };

/*
Returns a point degenerated rectangle as network bounding box of the gpoint

*/

    inline const Rectangle<2> NetBoundingBox() const {
      if (m_bDefined)
        return Rectangle<2>(true,
                            (double) m_xRouteLocation.rid,
                            (double) m_xRouteLocation.rid,
                            m_xRouteLocation.d - 0.000001,
                            m_xRouteLocation.d + 0.000001);
                  //0.000001 correcture of rounding differences
      else return Rectangle<2> (false, 0.0, 0.0, 0.0, 0.0);
    };

/*
Returns a gline representing the shortest path between two GPoint.

*/

  bool ShortestPath(GPoint *ziel, GLine *result);

  private:

/*

3.4 private Fields of class ~gpoint~

Network id

*/

    int m_iNetworkId;

/*
Route location see struct ~rloc~ for detailed fields

*/
    RLoc m_xRouteLocation;

/*
Defined flag.

*/
    bool m_bDefined;
};

/*
4. Network

4.1 Enumerations of columns for relations

*/

    enum PositionRoutesRelation { ROUTE_ID = 0,
                                  ROUTE_LENGTH,
                                  ROUTE_CURVE,
                                  ROUTE_DUAL,
                                  ROUTE_STARTSSMALLER };
    enum PositionJunctionsRelation { JUNCTION_ROUTE1_ID = 0,
                                     JUNCTION_ROUTE1_MEAS,
                                     JUNCTION_ROUTE2_ID,
                                     JUNCTION_ROUTE2_MEAS,
                                     JUNCTION_CC,
                                     JUNCTION_POS,
                                     JUNCTION_ROUTE1_RC,
                                     JUNCTION_ROUTE2_RC,
                                     JUNCTION_SECTION_AUP_RC,
                                     JUNCTION_SECTION_ADOWN_RC,
                                     JUNCTION_SECTION_BUP_RC,
                                     JUNCTION_SECTION_BDOWN_RC};
    enum PositionSectionsRelation { SECTION_RID = 0,
                                    SECTION_MEAS1,
                                    SECTION_MEAS2,
                                    SECTION_DUAL,
                                    SECTION_CURVE,
                                    SECTION_CURVE_STARTS_SMALLER,
                                    SECTION_RRC,
                                    SECTION_SID};


/*
4.2 Class ConnectivityCode

Enum defining binary coding for the possible transitions between to routes.

*/
  enum Transition
  {
    AUP_AUP     = 0x0001,
    AUP_ADOWN   = 0x0002,
    AUP_BUP     = 0x0004,
    AUP_BDOWN   = 0x0008,
    ADOWN_AUP   = 0x0010,
    ADOWN_ADOWN = 0x0020,
    ADOWN_BUP   = 0x0040,
    ADOWN_BDOWN = 0x0080,
    BUP_AUP     = 0x0100,
    BUP_ADOWN   = 0x0200,
    BUP_BUP     = 0x0400,
    BUP_BDOWN   = 0x0800,
    BDOWN_AUP   = 0x1000,
    BDOWN_ADOWN = 0x2000,
    BDOWN_BUP   = 0x4000,
    BDOWN_BDOWN = 0x8000
  };

class ConnectivityCode
{

  public:


/*
4.2.1 Constructor.

Constructs the connectivity code given an integer value ~cc~.

*/
  ConnectivityCode( int in_iCc ):
    m_iConnectivityCode( in_iCc )
  {
  }

/*
4.2.2 Constructor for boolean values.

Constructs the connectivity code given the Boolean values for
all possibilities.

*/
  ConnectivityCode( bool in_bAup_Aup,
                    bool in_bAup_Adown,
                    bool in_bAup_Bup,
                    bool in_bAup_Bdown,
                    bool in_bAdown_Aup,
                    bool in_bAdown_Adown,
                    bool in_bAdown_Bup,
                    bool in_bAdown_Bdown,
                    bool in_bBup_Aup,
                    bool in_bBup_Adown,
                    bool in_bBup_Bup,
                    bool in_bBup_Bdown,
                    bool in_bBdown_Aup,
                    bool in_bBdown_Adown,
                    bool in_bBdown_Bup,
                    bool in_bBdown_Bdown ):
    m_iConnectivityCode( (in_bAup_Aup && AUP_AUP) |
                         (in_bAup_Adown && AUP_ADOWN) |
                         (in_bAup_Bup && AUP_BUP) |
                         (in_bAup_Bdown && AUP_BDOWN) |
                         (in_bAdown_Aup && ADOWN_AUP) |
                         (in_bAdown_Adown && AUP_ADOWN) |
                         (in_bAdown_Bup && ADOWN_BUP) |
                         (in_bAdown_Bdown && AUP_BDOWN) |
                         (in_bBup_Aup && BUP_AUP) |
                         (in_bBup_Adown && BUP_ADOWN) |
                         (in_bBup_Bup && BUP_BUP) |
                         (in_bBup_Bdown && BUP_BDOWN) |
                         (in_bBdown_Aup && BDOWN_AUP) |
                         (in_bBdown_Adown && BUP_ADOWN) |
                         (in_bBdown_Bup && BDOWN_BUP) |
                         (in_bBdown_Bdown && BUP_BDOWN) )
  {
  }



/*
4.2.3 Method isPossible

Checks if a transition is possible.

*/
  bool IsPossible( Transition in_xTransition ) const
  {
    return in_xTransition & m_iConnectivityCode;
  }

  private:

/*
4.2.4 private fields ConnectivtyCode

The connectivity code

*/
    int m_iConnectivityCode;
};



/*
4.2.5 Class DirectedSection

This class is needed for the list of sections used in each entry
in the adjacency-list

*/
class DirectedSection
{
  public:

/*
Constructor

*/
  DirectedSection()
  {
  }

/*
Constructor giving a section

*/
  DirectedSection( TupleId in_iSectionTid,
                   bool in_bUpDown):
    m_iSectionTid( in_iSectionTid ),
    m_bUpDown( in_bUpDown )
  {
  }

/*
Copy-Constructor

*/

  DirectedSection( const DirectedSection& in_xSection ):
    m_iSectionTid( in_xSection.m_iSectionTid),
    m_bUpDown( in_xSection.m_bUpDown )
  {
  }

/*
Redefinition of the assignment operator.

*/
  DirectedSection& operator=( const DirectedSection& in_xSection )
  {
    m_bUpDown = in_xSection.m_bUpDown;
    m_iSectionTid = in_xSection.m_iSectionTid;
    return *this;
  }

  bool GetUpDownFlag()
  {
    return m_bUpDown;
  }

  TupleId GetSectionTid()
  {
    return m_iSectionTid;
  }

  private:

/*
Field with section-pointer

A pointer to the section.

*/
  TupleId m_iSectionTid;

/*
Field Direction-flag

A flag indicating the direction: ~true~ means up and ~false~ means down.

*/
  bool m_bUpDown;
};

/*
4.2.7 Class DirectedSectionPair

This class is needed for the list of sections used in each entry
in the adjacency-list

*/
class DirectedSectionPair
{
  public:

/*
Constructor

*/
  DirectedSectionPair()
  {
  }

/*
Constructor giving a section

*/
  DirectedSectionPair(TupleId in_iFirstSectionTid,
                      bool in_bFirstUpDown,
                      TupleId in_iSecondSectionTid,
                      bool in_bSecondUpDown):
    m_iFirstSectionTid( in_iFirstSectionTid ),
    m_bFirstUpDown( in_bFirstUpDown ),
    m_iSecondSectionTid( in_iSecondSectionTid ),
    m_bSecondUpDown( in_bSecondUpDown )
  {
  }

/*
Copy-Constructor

*/
  DirectedSectionPair( const DirectedSectionPair& in_xSection ):
    m_iFirstSectionTid(in_xSection.m_iFirstSectionTid ),
    m_bFirstUpDown(in_xSection.m_bFirstUpDown ),
    m_iSecondSectionTid(in_xSection.m_iSecondSectionTid ),
    m_bSecondUpDown(in_xSection.m_bSecondUpDown )
  {
  }

  bool operator<(const DirectedSectionPair& in_xOther) const
  {
    if(m_iFirstSectionTid != in_xOther.m_iFirstSectionTid)
    {
      return m_iFirstSectionTid < in_xOther.m_iFirstSectionTid;
    }
    if(m_bFirstUpDown != in_xOther.m_bFirstUpDown)
    {
      return m_bFirstUpDown;
    }
    if(m_iSecondSectionTid != in_xOther.m_iSecondSectionTid)
    {
      return m_iSecondSectionTid < in_xOther.m_iSecondSectionTid;
    }
    return m_bSecondUpDown;
  }



/*
Field with a pointer to the first section.

*/
  TupleId m_iFirstSectionTid;

/*

Field Direction-flag

Indicating the first sections direction: ~true~ means up and ~false~ means down.

*/
  bool m_bFirstUpDown;

/*
Field with a pointer to the second section.

*/
  TupleId m_iSecondSectionTid;

/*
Field Direction-flag

Indicating the second section direction: ~true~ means up and ~false~ means down.

*/
  bool m_bSecondUpDown;
};


/*
4.2.8 Class AdjacencyListEntry

Used for the adjacency-list of the network

*/
struct AdjacencyListEntry
{
/*
The simple constructor.

*/
  AdjacencyListEntry() {}

/*
The constructor.

*/
  AdjacencyListEntry( int in_iLow,
                      int in_iHigh ):
  m_iLow( in_iLow ),
  m_iHigh( in_iHigh )
  {
  }

/*
The copy constructor.

*/
  AdjacencyListEntry( const AdjacencyListEntry& in_xEntry ):
  m_iLow( in_xEntry.m_iLow ),
  m_iHigh( in_xEntry.m_iHigh )
  {
  }

/*
The lower index in the adjacency lists sub-array.

*/
  int m_iLow;

/*
The higher index in the adjacency lists sub-array.

*/
  int m_iHigh;
};

/*
4.2.9 Class JunctionSortEntry

A helper struct for junction lists.

*/
struct JunctionSortEntry
{
  JunctionSortEntry()
  {
  }

  JunctionSortEntry(bool in_bFirstRoute,
                    Tuple* in_pJunction):
    m_bFirstRoute(in_bFirstRoute),
    m_pJunction(in_pJunction)
  {
  }

  bool m_bFirstRoute;

  Tuple* m_pJunction;

  bool operator<(const JunctionSortEntry& in_xOther) const
  {
    CcReal* xMeas1;
    if(m_bFirstRoute)
    {
      xMeas1 = (CcReal*)m_pJunction->GetAttribute(JUNCTION_ROUTE1_MEAS);
    }
    else
    {
      xMeas1 = (CcReal*)m_pJunction->GetAttribute(JUNCTION_ROUTE2_MEAS);
    }

    CcReal* xMeas2;
    if(in_xOther.m_bFirstRoute)
    {
      xMeas2 =
          (CcReal*)in_xOther.m_pJunction->GetAttribute(JUNCTION_ROUTE1_MEAS);
    }
    else
    {
      xMeas2 =
          (CcReal*)in_xOther.m_pJunction->GetAttribute(JUNCTION_ROUTE2_MEAS);
    }

    return (xMeas1->GetRealval() < xMeas2->GetRealval());
  }

  double GetRouteMeas()
  {
    CcReal* pMeas;
    if(m_bFirstRoute)
    {
      pMeas = (CcReal*)m_pJunction->GetAttribute(JUNCTION_ROUTE1_MEAS);
    }
    else
    {
      pMeas = (CcReal*)m_pJunction->GetAttribute(JUNCTION_ROUTE2_MEAS);
    }
    return pMeas->GetRealval();

  }

  double GetOtherRouteMeas()
  {
    CcReal* pMeas;
    if(m_bFirstRoute)
    {
      pMeas = (CcReal*)m_pJunction->GetAttribute(JUNCTION_ROUTE2_MEAS);
    }
    else
    {
      pMeas = (CcReal*)m_pJunction->GetAttribute(JUNCTION_ROUTE1_MEAS);
    }
    return pMeas->GetRealval();

  }

  int GetOtherRouteId() {
    CcInt* pRouteId;
    if (m_bFirstRoute) {
      pRouteId = (CcInt*)m_pJunction->GetAttribute(JUNCTION_ROUTE2_ID);
    } else {
      pRouteId = (CcInt*)m_pJunction->GetAttribute(JUNCTION_ROUTE1_ID);
    }
    return pRouteId->GetIntval();
  }


  TupleId GetUpSectionId()
  {
    TupleId pTid;
    if(m_bFirstRoute)
    {
      pTid = ((TupleIdentifier*)
          m_pJunction->GetAttribute(JUNCTION_SECTION_AUP_RC))->GetTid();
    }
    else
    {
      pTid =((TupleIdentifier*)
          m_pJunction->GetAttribute(JUNCTION_SECTION_BUP_RC))->GetTid();
    }
    return pTid;
  }

  TupleId GetDownSectionId()
  {
    TupleId pTid;
    if(m_bFirstRoute)
    {
      pTid = ((TupleIdentifier*)
          m_pJunction->GetAttribute(JUNCTION_SECTION_ADOWN_RC))->GetTid();
    }
    else
    {
      pTid =((TupleIdentifier*)
          m_pJunction->GetAttribute(JUNCTION_SECTION_BDOWN_RC))->GetTid();
    }
    return pTid;
  }
};



/*
4.2.9 Class ~Network~

Central object of network concept. All other positions are given related to a
network object.

*/
class Network
{
  public:

/*

4.2.9.1 The public methods of the class ~network~

The internal and external (they are equal) ~routes~ relation type info
as string.

*/

  static string routesTypeInfo;


/*
The B-Tree in the ~routes~ relation type info as string.

*/

  static string routesBTreeTypeInfo;

/*
The R-Tree in the ~routes~ relation type info as string.

*/

  static string routesRTreeTypeInfo;

/*
The B-Tree in the ~junctions~ relation type info as string.

*/

  static string junctionsBTreeTypeInfo;


/*
The external ~junctions~ relation type info as string.

*/

  static string junctionsTypeInfo;

/*
The internal ~junctions~ relation type info as string.

*/

  static string junctionsInternalTypeInfo;

/*
The internal ~sections~ relation type info as string.

*/

  static string sectionsInternalTypeInfo;

/*
The sectionsBTreeTypeInfo

*/

static string sectionsBTreeTypeInfo;


/*
~distancestoreageTypeInfo~ only used for experimental netdistance precomputing.

*/
static string distancestorageTypeInfo;
/*

4.2.9.2 Constructors of the class ~network~

The simple constructor.

*/
  Network();

/*
Relation-Constuctor

The constructor that receives all information to create a network.

*/
  Network(SmiRecord& in_xValueRecord,
          size_t& inout_iOffset,
          const ListExpr in_xTypeInfo);

/*
List-Constructor

The constructor that receives a list containing a network

*/
  Network(ListExpr in_xValue,
          int in_iErrorPos,
          ListExpr& inout_xErrorInfo,
          bool& inout_bCorrect);

/*
The destructor.

*/
  ~Network();

/*

4.2.9.3 Methods of the class ~network~

Destroy-Method

This function sets all information inside the network to be
destroyed when the destructor is called.

*/
    void Destroy();

/*
Load-Method

Loads a network given two relations containing the routes and junctions.
This function corresponds to the operator ~thenetwork~.

*/
    void Load(int in_iId,
              const Relation *in_pRoutes,
              const Relation *in_pJunctions);

/*
Out-Method

Outputs a network

*/
  ListExpr Out(ListExpr typeInfo);

/*
Save-Method

Saves all relations of the network

*/
  ListExpr Save( SmiRecord& valueRecord,
                 size_t& offset,
                 const ListExpr typeInfo );

/*
Open-Method

Opens a network

*/
  static Network* Open( SmiRecord& valueRecord,
                        size_t& offset,
                        const ListExpr typeInfo );

/*
The ~Out~, ~Save~, and ~Open~ functions of the type constructor ~network~.
The ~In~ function is not provided, given that the creation of the network
is only done via the ~thenetwork~ operator.

*/


/*
Method GetId

Returns the id of this network

*/
    int GetId();

/*
GetRoutes

Returns a copy of the ~routes~ relation in external representation.
This function is used in the ~routes~ operator.

*/
    Relation *GetRoutes();

/*
GetJunctions

Returns a copy of the ~junctions~ relation in external representation.
This function is used in the ~junctions~ operator.

*/
    Relation *GetJunctions();

/*
GetJunctionsOnRoute

Returns the junction from the start of the route to the end.

*/
    void GetJunctionsOnRoute(CcInt* in_pRouteId,
                             vector<JunctionSortEntry>& inout_xJunctions);

/*
Returns the section ~tuple~ of the network which includes the ~GPoint~.
Attention: The tupleId of the returned Tuple is not the TupleId in the sections
relation! If you need the correct tupleId of the sections relation use
~GetTupleIdSectionOnRoute~ instead.

*/

 Tuple*  GetSectionOnRoute(GPoint* in_xGPoint);

/*
Returns the ~tupleid~  of the section which includes the ~GPoint~ .

*/


 TupleId GetTupleIdSectionOnRoute(GPoint* in_xGPoint);

/*
GetPointOnRoute

Returns the point value of the GPoint on the route. Used for translation of
network positions into spatial 2D positions.

*/
  void GetPointOnRoute(const GPoint* in_xGPoint, Point *&res);

/*
GetRoute

Returns the route tuple for the given route id.

*/

  Tuple* GetRoute(int in_RouteId);
  Tuple* GetRoute(TupleId in_routeTID);

/*
GetRouteCurve

Returns the route curve for the given route id.

*/

  SimpleLine GetRouteCurve(int in_iRouteId);

/*
GetDual

Returns the dual value of the given route id.

*/

  bool GetDual(int in_iRouteId);

/*
GetLineValueOfRouteInterval

Returns the ~sline~ representing the ~RouteInterval~ in spatial data. Used
to translate network values into spatial 2D values.

*/

  void GetLineValueOfRouteInterval(const RouteInterval* in_rI,
                                   SimpleLine *out_line);

/*
GetSections

Returns a copy of the ~sections~ relation in external representation.
This function is used in the ~sections~ operator.

*/
  Relation *GetSections();

/*
GetNetworkPosOfPoint

Returns a GPoint representing the point value in the network if possible, undef
elsewhere. Used to translate spatial 2D positions into network positions.

*/

  GPoint* GetNetworkPosOfPoint(Point p);

/*
GetSectionsInternal

Returns the ~sections~ relation (in internal representation).

*/
    Relation *GetSectionsInternal();

/*
Method GetAdjacentSection
Returns a vector of sections which can be reached next upwards respectively
downwards bool from the section given by TupleId.

*/

    void GetAdjacentSections(TupleId in_iSectionId,
                             bool in_bUpDown,
                             vector<DirectedSection> &inout_xSections);

/*
GetSections on RouteInterval.

Returns a set of sections which are covered by the given ~RouteInterval~
~inri~ as ~DBArray~ of SectTreeEntries.

*/

    void GetSectionsOfRouteInterval(const RouteInterval *in_ri,
                                    DBArray<SectTreeEntry> *io_SectionIds);

/*
Get Routeinterval for Halfsegment defined by point interval. Used to translate
spatial 2D values into network values.

*/

    RouteInterval* Find(Point p1, Point p2);

/*
Get Routepart passed limited by the two points.

*/

    RouteInterval* FindInterval(Point p1, Point p2);


/*
Computes the junction of the two given routes and returns the route measure
values of the junction on this routes.

*/

void GetJunctionMeasForRoutes(CcInt *pLastRouteId, CcInt *pCurrentSectionRid,
                                  double& rid1meas, double& rid2meas);

/*
Returns the section with the given id.

*/
    Tuple* GetSection(TupleId n);

    void FindSP(TupleId j1,TupleId j2,double& length,GLine* res);
/*
4.2.9.4 Secondo Integration Methods of Class ~Network~

NetworkProp

*/
  static ListExpr NetworkProp();

/*
OutNetwork

*/
  static ListExpr OutNetwork(ListExpr typeInfo, Word value);

/*
InNetwork

*/
  static Word InNetwork(ListExpr in_xTypeInfo,
                        ListExpr in_xValue,
                        int in_iErrorPos,
                        ListExpr& inout_xErrorInfo,
                        bool& inout_bCorrect);

/*
CreateNetwork

*/
  static Word CreateNetwork(const ListExpr typeInfo);

/*
CloseNetwork

*/
  static void CloseNetwork(const ListExpr typeInfo, Word& w);

/*
CloneNetwork

*/
  static Word CloneNetwork(const ListExpr typeInfo, const Word& w);

/*
DeleteNetwork

*/
  static void DeleteNetwork(const ListExpr typeInfo, Word& w);

/*
CheckNetwork

*/
  static bool CheckNetwork(ListExpr type, ListExpr& errorInfo);

/*
CastNetwork

*/
  static void* CastNetwork(void* addr);

/*
SaveNetwork

*/
  static bool SaveNetwork( SmiRecord& valueRecord,
                           size_t& offset,
                           const ListExpr typeInfo,
                           Word& value );

/*
OpenNetwork

*/
  static bool OpenNetwork( SmiRecord& valueRecord,
                           size_t& offset,
                           const ListExpr typeInfo,
                           Word& value );

/*
SizeOfNetwork

*/
  static int SizeOfNetwork();

/*
isDefined

*/

 int IsDefined();
 void SetDefined(bool def)
 {
   m_bDefined = def;
 };

private:


/*
4.2.9.5 Private methods of class ~Network~

FillRoutes

Copies the relation given as argument to the ~routes~ relations sorted by
the ~id~ attribute and creates the B-Tree in this attribute.

*/
  void FillRoutes( const Relation *in_pRoutes );

/*
FillJunctions

Copies the relation given as argument to the ~junctions~ relation appending
the new attributes.

*/
  void FillJunctions( const Relation *in_pJunctions );

/*
FillSections

Given the ~routes~ and ~junctions~ relation, the ~sections~ relation
is retrieved.

*/
  void FillSections();

/*
FillAdjacencyLists

Given that all relations are set up, the adjacency lists are created.

*/
  void FillAdjacencyLists();

  void FillAdjacencyPair(TupleId in_pFirstSection,
                         bool in_bFirstUp,
                         TupleId in_pSecondSection,
                         bool in_bSecondUp,
                         ConnectivityCode in_xCc,
                         Transition in_xTransition,
                         vector<DirectedSectionPair> &inout_xPairs);

/*
Used for experiments with network distance computation to store precomputed
distances. Because of the big data overhead and computation time this methods
and parameters should not be used if not needed. Therefore their calls are
comment out in the code. Of network constructors.
  
*/
  
  void FillDistanceStorage();

/*
Helpful functions for Find respectively FindInterval. Tests Network if there
is a shorter connection between the two points p1 and p2 than given by
start, end.
Function doubled because FindInterval is used for ~mpoint~ to ~mgpoint~
translation and therefore the values of end and start respectively
dpos and dpos2 may not be changed to each other than it is done in Find
for ~gline~ ~routeInterval~.

*/

  bool ShorterConnection(Tuple *route, double &start,
                   double &end, double &dpos, double &dpos2, int &rid,
                   int &ridt, Point p1, Point p2 );

  bool ShorterConnection2(Tuple *route, double &start,
                   double &end, double &dpos, double &dpos2, int &rid,
                   int &ridt, Point p1, Point p2 );

  bool InShortestPath(GPoint* start, GPoint* end, GLine *result);


/*
4.2.9.6 Private fields of Class ~Network~

The ID of the network

*/
  int m_iId;

/*
True if all values of this network have been defined

*/
  int m_bDefined;


/*
The ~routes~ relation.

*/
  Relation* m_pRoutes;

/*
The ~junctions~ relation.

*/
  Relation* m_pJunctions;

/*
The ~sections~ relation.

*/
  Relation* m_pSections;

/*
The BTree in the ~routes~ relation.

*/
  BTree* m_pBTreeRoutes;

/*
The BTree in the ~routes~ relation.

*/

  R_Tree<2,TupleId> *m_pRTreeRoutes;

/*
The RTree in the ~routes~ relation

*/
  BTree* m_pBTreeJunctionsByRoute1;

/*
The BTree in the ~junctions~ relation.

*/
  BTree* m_pBTreeJunctionsByRoute2;


/*
The adjacency lists of sections.

*/

  DBArray<AdjacencyListEntry> m_xAdjacencyList;

/*
The adjacency lists of sections.

*/
  DBArray<DirectedSection> m_xSubAdjacencyList;

/*
The BTree of the sections route ids.

*/

  BTree* m_pBTreeSectionsByRoute;

/*
Stores the precomputed distances between every possible pair of two junction
points. Only for experimental use in network distance computing. Comment out
in the code if not needed the computational and storage space overhead is to
big.

*/

  Relation* alldistance;

};


/*
5 class ~gline~

*/


class GLine : public StandardAttribute
{
/*
5.1 Constructors and Destructor

The simple constructor. Should not be used.

*/
  public:
    GLine();

/*
~iniSize~ gives the number of expected ~RouteIntervals~. If it is to small
the size of the ~DBArray~ is dynamically extended later.

*/
    GLine(int in_iSize);

    GLine(const GLine* in_xOther);

    ~GLine() {};

    GLine( ListExpr in_xValue,
         int in_iErrorPos,
         ListExpr& inout_xErrorInfo,
         bool& inout_bCorrect);

/*
5.2 Methods of class ~gline~

*/

    ostream& Print( ostream& os ) const;

    void SetNetworkId(int in_iNetworkId);

    void AddRouteInterval(int in_iRouteId,
                          double in_dStart,
                          double in_dEnd);

    void AddRouteInterval(RouteInterval *ri);

    double GetLength ();

    int GetNetworkId();

    void Get(const int i, const RouteInterval* &ri) const;

    int NoOfComponents();

    bool IsSorted();

    void SetSorted(bool);

    DBArray<RouteInterval>* GetRouteIntervals();

/*
Computes the network distance of 2 glines.

*/

    double Netdistance(GLine* pgl2);

/*
Computes the euclidean distance of 2 glines.

*/

    double Distance (GLine* pgl2);

/*
Computes the union of 2 glines.

*/


void Uniongl (GLine* pgl2, GLine *res);

/*
Translates a gline value into a line value.

*/

    void Gline2line (Line *res);


/*
Returns true if the glines intersect false elswhere.

*/


    bool Intersects (GLine* pgl);


/*
Returns the Bounding GPoints of the GLine.
    
*/

    GPoints* GetBGP();

    void Clear();

    GLine& operator=( const GLine& l );

    bool operator== (const GLine& p) const;

/*
5.3 Secondo Integration

*/

    static ListExpr Out( ListExpr typeInfo, Word value );

    static Word In( const ListExpr typeInfo,
                    const ListExpr instance,
                    const int errorPos,
                    ListExpr& errorInfo,
                    bool& correct );

    static Word Create( const ListExpr typeInfo );

    static void Delete( const ListExpr typeInfo, Word& w );

    static void Close( const ListExpr typeInfo, Word& w );

    static Word CloneGLine( const ListExpr typeInfo,
                       const Word& w );

    static void* Cast( void* addr );

    int NumOfFLOBs() const;

    FLOB *GetFLOB( const int i );

    static int SizeOf();

    size_t Sizeof() const;

    int Size() const;

    static ListExpr Property();

    static bool Check( ListExpr type, ListExpr& errorInfo );

    bool IsDefined() const;

    void SetDefined(bool);

    int Compare(const Attribute*) const;

    bool Adjacent(const Attribute*) const;

    GLine* Clone() const;

    size_t HashValue() const;

    void CopyFrom(const StandardAttribute*);

    void TrimToSize()
     {
       m_xRouteIntervals.TrimToSize();
     };

  private:

/*
5.4 Private Fields of class ~gline~

The network id.

*/

    int m_iNetworkId;

/*
Defined Flag: True if all members are defined

*/

    bool m_bDefined;

/*
Sorted Flag: True if route intervals of the gline are sorted.

*/

    bool m_bSorted;

/*
Length of the gline.

*/

    double m_dLength;

/*
The array of route intervals.

*/

    DBArray<RouteInterval> m_xRouteIntervals;


};



/*
1.2.3 ~struct RITree~

Used to compress and sort ~RouteInterval~s. Because many operators take profit
from sorted ~RouteInterval~s. We sort and compress resulting ~DBArray~s of
~RouteInterval~s whenever it is possible.

*/

struct RITree {

  RITree(){};

  RITree( int ri,double pos1, double pos2, RITree *left = 0, RITree *right = 0){
    m_iRouteId = ri;
    m_dStart = pos1;
    m_dEnd = pos2;
    m_left = left;
    m_right = right;
  };

  ~RITree(){};
  
/*
Checks if in the ~RITree~ exist son intervals which are covered by the
previos changed interval of the father.

*/

  double CheckTree(RITree& father, int rid, double pos1, double pos2,
                   bool bleft) {
    if (rid < this->m_iRouteId) {
      if (this->m_left != 0) {
        return this->m_left->CheckTree(*this, rid, pos1, pos2, bleft);
      } else {
        if (bleft) return pos1;
        else return pos2;
      }
    } else {
      if (rid > this->m_iRouteId) {
        if (this->m_right != 0) {
          return this->m_right->CheckTree(*this, rid, pos1, pos2,bleft);
        } else {
          if (bleft) return pos1;
          else return pos2;
        }
      } else {
        if (pos2 < this->m_dStart) {
          if (this->m_left != 0) {
            return this->m_left->CheckTree(*this, rid, pos1, pos2,bleft);
          } else {
            if (bleft) return pos1;
            else return pos2;
          }
        } else {
          if (pos1 > this->m_dEnd) {
            if (this->m_right != 0 ) {
              return this->m_right->CheckTree(*this, rid, pos1, pos2,bleft);
            } else {
              if (bleft) return pos1;
              else return pos2;
            }
          } else {
            // Overlapping interval found. Rebuild Tree and return new interval
            // limit.
            if (bleft) {
              if (this->m_dStart <= pos1) {
                  pos1 = this->m_dStart;
              }
              if (father.m_left == this) {
                father.m_left = this->m_left;
                this->m_left = 0;
              } else {
                father.m_right = this->m_left;
                this->m_left = 0;
              }
              this->RemoveTree();
              if (father.m_left != 0) {
                return father.m_left->CheckTree(father, rid, pos1, pos2, bleft);
              } else {
                return pos1;
              }
            } else {
              if (this->m_dEnd >= pos2) {
                pos2 = this->m_dEnd;
              }
              if (father.m_left == this) {
                father.m_left = this->m_right;
                this->m_right = 0;
              } else {
                father.m_right = this->m_right;
                this->m_right = 0;
              }
              this->RemoveTree();
              if (father.m_right != 0 ) {
                return father.m_right->CheckTree(father, rid, pos1, pos2,bleft);
              } else {
                return pos2;
              }
            }
          }
        }
      }
    }
    if (bleft) return pos1;
    else return pos2;
  };

/*
Inserts a ~RouteInterval~ in the ~RITree~ checking if there are already
~RouteIntervals~ which overlap or are adjacent to the current ~RouteInterval~.

*/
  void InsertUnit(int rid, double pos1, double pos2)
  {
    if (pos1 < pos2) Insert(rid, pos1, pos2);
    else Insert(rid, pos2, pos1);
  }

  void Insert (int rid, double pos1, double pos2) {
    double test;
    if (rid < this->m_iRouteId) {
      if (this->m_left != 0) {
        this->m_left->Insert(rid, pos1, pos2);
      } else {
        this->m_left = new RITree(rid, pos1, pos2,0,0);
      }
    } else {
      if (rid > this->m_iRouteId) {
        if (this->m_right != 0) {
          this->m_right->Insert(rid, pos1, pos2);
        } else {
          this->m_right = new RITree(rid, pos1, pos2,0,0);
        }
      }else{
        if(rid == this->m_iRouteId) {
          if (pos2 < this->m_dStart) {
            if (this->m_left != 0) {
               this->m_left->Insert(rid, pos1, pos2);
            } else {
                this->m_left = new RITree(rid, pos1, pos2,0,0);
            }
          } else {
            if (pos1 > this->m_dEnd) {
              if (this->m_right != 0) {
                this->m_right->Insert(rid, pos1, pos2);
              } else {
                this->m_right =
                    new RITree(rid, pos1, pos2,0,0);
              }
            } else {
              // Overlapping route intervals merge and check sons if they need
              // to be corrected too.
              if (this->m_dStart > pos1) {
                this->m_dStart = pos1;
                if (this->m_left != 0) {
                  test = this->m_left->CheckTree(*this, rid, this->m_dStart,
                                                this->m_dEnd, true);
                  if (this->m_dStart > test) {
                    this->m_dStart = test;
                  }
                }
              }
              if (this->m_dEnd < pos2) {
                this->m_dEnd = pos2;
                if (this->m_right != 0) {
                  test = this->m_right->CheckTree(*this, rid, this->m_dStart,
                                                  this->m_dEnd, false);
                  if (this->m_dEnd < test) {
                    this->m_dEnd = test;
                  }
                }
              }
            }
          }
        } // endif rid=rid
      }
    }
  };

/*
Stores the ~RouteInterval~s of the ~RITree~ sorted by their route ids and
start positions into ~gline~.

*/

  void TreeToGLine (GLine* gline) {
    if (this->m_left != 0) {
      this->m_left->TreeToGLine (gline);
    }
    gline->AddRouteInterval(this->m_iRouteId, this->m_dStart, this->m_dEnd);
    if (this->m_right != 0) {
      this->m_right->TreeToGLine (gline);
    }
  };

/*
Stores the ~RouteInterval~s of the ~RITree~ sorted by their route ids and
start positions into a ~DBArray~. Used to build trajectory of ~mgpoint~.

*/

  void TreeToDBArray (DBArray<RouteInterval>* arr) {
    if (this->m_left != 0) {
      this->m_left->TreeToDBArray (arr);
    }
    arr->Append(RouteInterval(this->m_iRouteId, this->m_dStart, this->m_dEnd));
    if (this->m_right != 0) {
      this->m_right->TreeToDBArray (arr);
    }
  };

/*
Deletes the tree.

*/
  void RemoveTree(){
    if (m_left != 0) m_left->RemoveTree();
    if (m_right != 0) m_right->RemoveTree();
    delete this;
  };

  int m_iRouteId;
  double m_dStart, m_dEnd;
  RITree *m_left, *m_right;
};

/*
6. class GPoints by Jianqiu Xu.

*/

extern string edistjoinpointlist;

class GPoints: public StandardAttribute{
public:
  GPoints();
  GPoints(int in_iSize);
  GPoints(GPoints* in_xOther);
  ~GPoints(){}
  ostream& Print(ostream& os)const;
  inline bool IsEmpty()const;
  size_t Sizeof()const;
  GPoints& operator+=(const GPoint &gp);
  GPoints& operator-=(const GPoint &gp);
  bool IsDefined()const{return m_defined;}
  int NumOfFLOBs()const;
  FLOB* GetFLOB(const int i);
  void SetDefined( bool in_bDefined ){m_defined = in_bDefined;}
  void Get(int i,const GPoint*&)const;
  inline int Size()const;
  static int SizeOf();
  static ListExpr Out(ListExpr typeInfo,Word value);
  static Word In(const ListExpr typeInfo,const ListExpr instance,
                 const int errorPos,ListExpr& errorInfo,bool& correct);
  static Word Create(const ListExpr typeInfo);
  static void Delete(const ListExpr typeInfo,Word& w);
  static void Close(const ListExpr typeInfo,Word& w);
  static Word CloneGPoints(const ListExpr typeInfo,const Word& w);
  static void* Cast(void* addr);
  static ListExpr Property();
  static bool Check(ListExpr type, ListExpr& errorInfo);
  static bool SaveGPoints(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo, Word& value);
  static bool OpenGPoints(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo, Word& value);
  int Compare(const Attribute*)const;
  bool Adjacent(const Attribute*)const;
  GPoints* Clone()const;
  size_t HashValue()const;
  void CopyFrom(const StandardAttribute* right);
  GPoints& operator=(const GPoints& gps);
  void FilterAliasGPoints(Network *pNetwork);
  void TrimToSize () {m_xGPoints.TrimToSize();}

private:
  bool m_defined;
  DBArray<GPoint> m_xGPoints;
};


#endif // __NETWORK_ALGEBRA_H__
