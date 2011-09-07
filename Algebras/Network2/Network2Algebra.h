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

1 Declarations Necessary for Algebra Network

*/
#ifndef __NETWORK2_ALGEBRA_H__
#define __NETWORK2_ALGEBRA_H__


#include "NestedList.h"
#include "TupleIdentifier.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "../../Tools/Flob/DbArray.h"
#include "../../Tools/Flob/Flob.h"
#include "Attribute.h"
#include "SpatialAlgebra.h"
#include "RTreeAlgebra.h"
#include "Geoid.h"
#include "ListUtils.h"

#include <cstring>

namespace network2 {
/*
2 Helping structs

2.1 struct SectTreeEntry

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
2.2 class RouteInterval

Every ~gline~ consists of a set of ~RouteInterval~s stored in a ~DBArray~.

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
Methode ~BoundingBox~

Computes the spatial Bounding Box of the given ~RouteInterval~ as 2 dimensional
rectangle. Using the subline of the route curve defined by the given ~route-
interval~.

*/

  Rectangle<2> BoundingBox(Network* pNetwork, const Geoid* geoid = 0) const;

/*
Get- and Set-Methodes for private values.

*/
  inline int GetRouteId() const {return m_iRouteId;};
  inline double GetStartPos() const {return m_dStart;};
  inline double GetEndPos() const {return m_dEnd;};
  inline void SetRouteId(int rid) {m_iRouteId = rid;};
  inline void SetStartPos(double pos) {m_dStart = pos;};
  inline void SetEndPos(double pos){m_dEnd = pos;};

  private:
/*
Private fields:

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
3 GPoint

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

Private fields:

Route Id

*/

  int rid;

/*
The distance from the 0.0 point of the route.

*/

  double d;

/*
Side value

*/

  Side side;
};

/*
3.3 class ~gpoint~

Represents single positions in the network. Each ~GPoint~ consists of a network
id the ~gpoint~ belongs to, a ~rloc~ and a boolean defined flag. Because
~gpoint~ should be usable in relations the class ~gpoint~ derives from
class Attribute.

*/

class GPoint : public Attribute
{
  public:

/*
3.3.1 Constructors and Destructor of class ~GPoint~

*/

    GPoint():Attribute()
    {}

    GPoint( bool in_bDefined,
            int in_iNetworkId = 0,
            int in_xRid = 0,
            double in_dLocation = 0.0,
            Side in_xSide = None ):
    Attribute(in_bDefined),
    m_iNetworkId( in_iNetworkId ),
    m_xRouteLocation( in_xRid, in_dLocation, in_xSide )
    {
      SetDefined(in_bDefined);
    }

    GPoint( const GPoint& in_xOther ):
    Attribute(in_xOther.IsDefined()),
    m_iNetworkId( in_xOther.m_iNetworkId ),
    m_xRouteLocation( in_xOther.m_xRouteLocation )
    {
      SetDefined(in_xOther.IsDefined());
    }

    ~GPoint(){};

/*
3.3.2 Methods of class ~gpoint~


Assignment operator

*/
    GPoint& operator=( const GPoint& in_xOther )
    {
      SetDefined(in_xOther.IsDefined());
      if( IsDefined())
      {
        m_iNetworkId = in_xOther.m_iNetworkId;
        m_xRouteLocation = in_xOther.m_xRouteLocation;
      }
      return *this;
    }

/*
Equal operator

Returns true if two gpoint are identical.

*/

    bool operator== (const GPoint& p) const;

/*
Unequal operator

Returns false if two gpoint are identical.

*/

    bool operator!= (const GPoint&p) const;

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

/*
Support Methods of ~gpoint~

*/

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

    void CopyFrom( const Attribute* right )
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

    inline static const string BasicType() { return "gpoint"; }
    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }

/*
Method ~Netdistance~ and ~NewNetdistance~

Returns the network distance between 2 ~gpoint~

*/

    double Netdistance (GPoint* toGPoint);
    double NewNetdistance(GPoint* pToGPoint,GLine* res);//new

/*
Method ~Distance~

Computes the euclidean distance of 2 ~gpoint~.

*/
    double Distance (GPoint* toGPoint);

/*
Method ~Inside~

Returns ~true~ if the gpoint is inside the gline false elsewhere.

*/

    bool Inside(GLine *gl);


/*
Method ~ToPoint~

Translates a ~gpoint~ into a ~point~ in the 2D plane.

*/

    void ToPoint(Point *& res);

    Point* ToPoint() const;

    Point* ToPoint(Network *&pNetwork) const;

/*
Method ~BoundingBox~

Returns the spatial Bounding Box of the point which is a rectangle degenerated
to a single point.

*/

    inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0){
      if(geoid){
        cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
        << endl;
        assert( !geoid ); // TODO: implement spherical geometry case
      }
      if (IsDefined()) {
        Point *p = ToPoint();
        Rectangle<2> result = p->BoundingBox();
        p->DeleteIfAllowed();
        return result;
      } else return Rectangle<2> (false, 0.0, 0.0, 0.0, 0.0);
    };

/*
Method ~NetBoundingBox~

Returns a point degenerated rectangle as network bounding box of the gpoint

*/

    inline const Rectangle<2> NetBoundingBox(const Geoid* geoid = 0) const {
      if(geoid){
        cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
        << endl;
        assert( !geoid ); // TODO: implement spherical geometry case
      }
      if (IsDefined())
        return Rectangle<2>(true,
                            (double) m_xRouteLocation.rid,
                            (double) m_xRouteLocation.rid,
                            m_xRouteLocation.d - 0.000001,
                            m_xRouteLocation.d + 0.000001);
                  //0.000001 correcture of rounding differences
      else return Rectangle<2> (false, 0.0, 0.0, 0.0, 0.0);
    };

/*
Method ~ShortestPath~

Returns a gline representing the shortest path between two GPoint using
the Dijkstras Algorithm.

*/

  bool ShortestPath(GPoint *ziel, GLine *result);

/*
Method ~ShortestPathAStarPlus~

Returns a gline representing the shortest path between two GPoint using
the AStar Algorithm.

*/

  bool ShortestPathAStarPlus ( GPoint *pToGPoint, GLine *result, Word func );

  private:

/*

3.3.3 Private Fields of class ~gpoint~


Network id

*/

    int m_iNetworkId;

/*
Route location see struct ~rloc~ for detailed fields

*/
    RLoc m_xRouteLocation;


};

/*
4 Network

4.1 Enumerations of columns for relations

*/

    enum PositionRoutesRelation { ROUTE_ID = 0,
                                  ROUTE_LENGTH,
                                  ROUTE_CURVE,
                                  ROUTE_DUAL,
                                  ROUTE_STARTSSMALLER,
                                  ROUTE_STARTPOS_ID };
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
                                    SECTION_SID,
                                    SECTION_PNO_UP,
                                    SECTION_PNO_DOWN,
                                    SECTION_SLOT_UP,
                                    SECTION_SLOT_DOWN,
                                    SECTION_COST,
                                    SECTION_DURATION};


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
4.2.1 Constructors:

Constructs the connectivity code given an integer value ~cc~.

*/
  ConnectivityCode( int in_iCc ):
    m_iConnectivityCode( in_iCc )
  {
  }

/*
Constructor for boolean values.

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
4.2.2 Methods of class ~ConnectivityCode~

Method ~isPossible~

Checks if a transition is possible.

*/
  bool IsPossible( Transition in_xTransition ) const
  {
    return in_xTransition & m_iConnectivityCode;
  }

   private:
/*
4.2.3 Private fields:

The connectivity code

*/
    int m_iConnectivityCode;
};



/*
4.3 Class DirectedSection

This class is needed for the list of sections used in each entry
in the adjacency-list

*/
class DirectedSection
{
  public:

/*
4.3.1 Constructors of class DirectedSection:

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
4.3.2 Methods of class DirectedSection

Redefinition of the assignment operator.

*/
  DirectedSection& operator=( const DirectedSection& in_xSection )
  {
    m_bUpDown = in_xSection.m_bUpDown;
    m_iSectionTid = in_xSection.m_iSectionTid;
    return *this;
  }

/*
Get-Methods

*/
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
4.3.3 Private fields of class DirectedSection

Field with section-pointer. A pointer to the section.

*/
  TupleId m_iSectionTid;

/*
Field Direction-flag.
A flag indicating the direction: ~true~ means up and ~false~ means down.

*/
  bool m_bUpDown;
};

/*
4.4 Class DirectedSectionInfo

This class is needed for the list of sections used in each entry
in the new adjacency-list

*/
class DirectedSectionInfo
{
  public:

/*
4.4.1 Constructors of class DirectedSectionInfo

*/
  DirectedSectionInfo()
  {
  }

/*
Constructor giving a section

*/
  DirectedSectionInfo( TupleId in_iSectionTid, bool in_bUpDown,
                       double in_meas1, double in_meas2, int in_rid,
                       double in_cost, double in_duration):
    m_iSectionTid( in_iSectionTid ),
    m_bUpDown( in_bUpDown ),
    m_meas1(in_meas1),
    m_meas2(in_meas2),
    m_rid(in_rid),
    m_cost(in_cost),
    m_duration(in_duration)
  {
  }

/*
Copy-Constructor

*/

  DirectedSectionInfo( const DirectedSectionInfo& in_xSection ):
    m_iSectionTid( in_xSection.m_iSectionTid),
    m_bUpDown( in_xSection.m_bUpDown ),
    m_meas1(in_xSection.m_meas1),
    m_meas2(in_xSection.m_meas2),
    m_rid(in_xSection.m_rid) ,
    m_cost(in_xSection.m_cost),
    m_duration(in_xSection.m_duration)
  {
  }

/*
4.4.2 Methods of class DirectedSectionInfo

Redefinition of the assignment operator.

*/
  DirectedSectionInfo& operator=( const DirectedSectionInfo& in_xSection )
  {
    m_bUpDown = in_xSection.m_bUpDown;
    m_iSectionTid = in_xSection.m_iSectionTid;
    m_meas1 = in_xSection.m_meas1;
    m_meas2 = in_xSection.m_meas2;
    m_rid = in_xSection.m_rid;
    m_cost = in_xSection.m_cost;
    m_duration = in_xSection.m_duration;
    return *this;
  }
/*
Get-Methodes for private Fields of class DirectedSectionInfo

*/

  bool GetUpDownFlag()
  {
    return m_bUpDown;
  }

  TupleId GetSectionTid()
  {
    return m_iSectionTid;
  }

  double GetMeas1(){
      return m_meas1;
  }
  double GetMeas2(){
      return m_meas2;
  }
  double GetLength(){
      return (m_meas2 - m_meas1);
  }
  int GetRid(){
      return m_rid;
  }
  double GetCost(){
      return m_cost;
  }
  double GetDuration(){
      return m_duration;
  }

  private:
/*
4.4.3 Private fields of class DirectedSectionInfo

Field with section-pointer. A pointer to the section.

*/
  TupleId m_iSectionTid;

/*
Direction-flag:
A flag indicating the direction: ~true~ means up and ~false~ means down.

*/
  bool m_bUpDown;

/*
Meas1 of the section

*/
  double m_meas1;

/*
Meas2 of the section

*/
  double m_meas2;

/*
RId of the section

*/
   int m_rid;

/*
Cost of the section

*/
    double m_cost;

/*
Duration of the section

*/
    double m_duration;
};


/*
4.5 Class PageRecord

This class is needed for the list of sections used in each entry in the
adjacency-List (it is an alternative of the DirectedSections)
It saves Sections on a Page. One Page can contain 48 sections with four
neighbours. If sections have more then four neighbours, it can happen, that
not all Slots are used.

*/
class PageRecord{
    private:
/*
4.5.1 Constants for class PageRecord

*/
    const static int MAX_SLOTINDEXSIZE = 181;
    const static int MAX_PAGESIZE = 3719;
    const static int MAX_SLOTS =45;
    const static int OFFSET_SID = 4;
    const static int OFFSET_UPDOWN = 8;
    const static int OFFSET_MEAS_1 = 9;
    const static int OFFSET_MEAS_2 = 17;
    const static int OFFSET_RID = 25;
    const static int OFFSET_COST = 29;
    const static int OFFSET_DURATION = 37;
    const static int OFFSET_NO_NEIGHBOUR =45;
    const static int OFFSET_FIRST_NEIGHBOUR = 49;
    const static int OFFSET_NEXT_NEIGHBOUR = 8;

    public:
/*
4.5.2 Constructors

Default Constructor

*/
    PageRecord(){
    }
/*
simple Constructor

*/
    PageRecord(int size){
        m_nextFree=0;
        m_isFull=false;

        //m_pageContent = {0};
        for (int i=0; i< MAX_PAGESIZE; i++){
            m_pageContent[i]=0;
        }
        //m_slotIndex ={0};
        for (int i=0; i< MAX_SLOTINDEXSIZE; i++){
            m_slotIndex[i]=0;
        }



    }
/*
Copy-Constructor

*/
    PageRecord( const PageRecord& in_xPage ):
        m_nextFree(in_xPage.m_nextFree),
        m_isFull(in_xPage.m_isFull)
        {

        for (int i=0; i< MAX_PAGESIZE; i++){
            m_pageContent[i]=in_xPage.m_pageContent[i];
        }
        for (int i=0; i< MAX_SLOTINDEXSIZE; i++){
            m_slotIndex[i]=in_xPage.m_slotIndex[i];
        }
    }

/*
4.5.3 Methods of class PageRecord

Redefinition of the assignment operator.

*/
    PageRecord& operator=( const PageRecord& in_xPage )
    {
        m_nextFree = in_xPage.m_nextFree;
        m_isFull= in_xPage.m_isFull;
        for (int i=0; i< MAX_PAGESIZE; i++){
            m_pageContent[i]=in_xPage.m_pageContent[i];
        }
        for (int i=0; i< MAX_SLOTINDEXSIZE; i++){
            m_slotIndex[i]=in_xPage.m_slotIndex[i];
        }
        return *this;
    }

/*
Method ~SetSectionContent~

stored the content of a section , which ist given by the tid and its
Updown-Flag, on the next free space.
return ~true~, if the section can be inserted, otherwise ~false~

*/
    bool SetSectionContent(TupleId tId, int sId, bool upDown, double meas1,
                           double meas2, int rid, double cost, double duration,
                           int noNeighbours, int sindex){
        //calculate the needed space
        int space = OFFSET_FIRST_NEIGHBOUR
                    + (noNeighbours*OFFSET_NEXT_NEIGHBOUR);
        // check whether the free space is enough
        if((MAX_PAGESIZE-m_nextFree)< space || sindex > MAX_SLOTS){
            // not enough space
            m_isFull = true;
            return false;
        }else{
            // enough space, so put section on the page
            int actpos = m_nextFree;
            // convert tID in char and store it at actpos schreiben
            char c_tid[sizeof(TupleId)];
            memcpy(c_tid, &tId, sizeof(TupleId));
            SetPageContent(c_tid, actpos, sizeof(TupleId));

            // convert sID in char and store it at actpos schreiben
            actpos = m_nextFree + OFFSET_SID;
            char c_sid[sizeof(int)];
            memcpy(c_sid, &sId, sizeof(int));
            SetPageContent(c_sid, actpos, sizeof(int));

            // convert UpDown in char and store it
            actpos = m_nextFree + OFFSET_UPDOWN;
            char c_updown[sizeof(bool)];
            memcpy(c_updown, &upDown, sizeof(bool));
            SetPageContent(c_updown,actpos,sizeof(bool));

            // convert meas1 (douple) in char and store it
            actpos = m_nextFree + OFFSET_MEAS_1;
            char c_meas1[sizeof(double)];
            memcpy(c_meas1, &meas1, sizeof(double));
            SetPageContent(c_meas1,actpos,sizeof(double));

            // convert meas2 (douple) in char and store it
            actpos = m_nextFree + OFFSET_MEAS_2;
            char c_meas2[sizeof(double)];
            memcpy(c_meas2, &meas2, sizeof(double));
            SetPageContent(c_meas2,actpos,sizeof(double));

            // convert rid (int) in char and store it
            actpos = m_nextFree + OFFSET_RID;
            char c_rid[sizeof(int)];
            memcpy(c_rid, &rid, sizeof(int));
            SetPageContent(c_rid,actpos,sizeof(int));

            // convert cost (double) in char and store it
            actpos = m_nextFree + OFFSET_COST;
            char c_cost[sizeof(double)];
            memcpy(c_cost, &cost, sizeof(double));
            SetPageContent(c_cost,actpos,sizeof(double));

            // convert duration (double) in char and store it
            actpos = m_nextFree + OFFSET_DURATION;
            char c_duration[sizeof(double)];
            memcpy(c_duration, &duration, sizeof(double));
            SetPageContent(c_duration,actpos,sizeof(double));

            // convert NoNeightbours in char and store it
            actpos = m_nextFree + OFFSET_NO_NEIGHBOUR;
            char c_noNeighbours[sizeof(int)];
            memcpy(c_noNeighbours, &noNeighbours, sizeof(int));
            SetPageContent(c_noNeighbours, actpos, sizeof(int));

            // store the startpos of this section in the slotindex
            char c_adress[sizeof(int)];
            memcpy(c_adress, &m_nextFree, sizeof(int));
            SetSlotContent(c_adress, sindex, sizeof(int));

            // calculate next startpos
            m_nextFree +=space;
            return true;
        }
    }

/*
Method ~SetSectionNeighbour~

stored the pagenumber and the slotnumber of a given neighbour
by the slotIndex and the neighbourIndex

*/
    void SetSectionNeighbour(int slotIndex, int neighbourIndex,
                             int nSPage, int nSSlot){
        // read slotstart-adress from slotindex
        int start = GetContentIndex(slotIndex);
        // calculate start-Adress from actual neighbour
        int startN = start+OFFSET_FIRST_NEIGHBOUR
                          +(neighbourIndex*OFFSET_NEXT_NEIGHBOUR);
        //put neighbourSectionPagenumber and neighbourSecionSlotnumber
        char c_spage[sizeof(int)];
        memcpy(c_spage, &nSPage, sizeof(int));
        SetPageContent(c_spage, startN, sizeof(int));
        startN += sizeof(int);
        char c_sslot[sizeof(int)];
        memcpy(c_sslot, &nSSlot, sizeof(int));
        SetPageContent(c_sslot, startN, sizeof(int));
    }
/*
Method ~SetSectionCost~

stored the cost of a given section
by the slotIndex

*/
    void SetSectionCost(int slotIndex, double cost){
        int start = GetContentIndex(slotIndex)+OFFSET_COST;
        char c_cost[sizeof(double)];
        memcpy(c_cost, &cost, sizeof(double));
        SetPageContent(c_cost, start, sizeof(double));
    }
/*
Method ~SetSectionDuration~

stored the duration of a given section
by the slotIndex

*/
    void SetSectionDuration(int slotIndex, double duration){
        int start = GetContentIndex(slotIndex)+OFFSET_DURATION;
        char c_duration[sizeof(double)];
        memcpy(c_duration, &duration, sizeof(double));
        SetPageContent(c_duration, start, sizeof(double));
    }
/*
Method ~GetSectionNeighbourPage~

gets the neighbour-pagenumber of a spezified neighbour

*/
    void GetSectionNeighbourPage(int slotIndex,
                                 int neighbourIndex, int &inout_nPage){
        //calculate startAdress
        int start = GetContentIndex(slotIndex)
                    +OFFSET_FIRST_NEIGHBOUR
                    +(neighbourIndex*OFFSET_NEXT_NEIGHBOUR);
        //get content
        int npage;
        char* c;
        GetPageContent(start,sizeof(int), c);
        memcpy(&npage, c, sizeof(int));
        inout_nPage = npage;
    }
/*
Method ~GetSectionNeighbourSlot~

gets the neighbour-slotnumber of a spezified neighbour

*/
    void GetSectionNeighbourSlot(int slotIndex,
                                 int neighbourIndex, int &inout_nSlot){
        //calculate startAdress
        int start = GetContentIndex(slotIndex)
                    +OFFSET_FIRST_NEIGHBOUR
                    +(neighbourIndex*OFFSET_NEXT_NEIGHBOUR)+ sizeof(int);
        //get content
        int nslot;
        char* c;
        GetPageContent(start,sizeof(int), c);
        memcpy(&nslot, c, sizeof(int));
        inout_nSlot = nslot;

    }

/*
Method ~GetSectionTid~

gets the section-TupleId which is stored in the given slot

*/
    void GetSectionTid(int slotIndex, TupleId &inout_tid){
        int pos = GetContentIndex(slotIndex);
        TupleId ntid;
        char* c;
        GetPageContent(pos,sizeof(int), c);
        memcpy(&ntid, c, sizeof(int));
        inout_tid = ntid;
    }

/*
Method ~GetSectionId~

gets the section-Id which is stored in the given slot

*/
    void GetSectionId(int slotIndex, int &inout_sid){
        int pos = GetContentIndex(slotIndex)+OFFSET_SID;
        int nsid;
        char* c;
        GetPageContent(pos,sizeof(int), c);
        memcpy(&nsid, c, sizeof(int));
        inout_sid = nsid;
    }

/*
Method ~GetSectionUpdown~
gets the updownflag of a section which is stores in the given slot

*/
    void GetSectionUpdown(int slotIndex, bool &inout_updown){
        int pos = GetContentIndex(slotIndex)+OFFSET_UPDOWN;
        bool nupDown;
        char *b;
        GetPageContent(pos,sizeof(bool), b);
        memcpy(&nupDown, b, sizeof(bool));
        inout_updown = nupDown;
    }

/*
Method ~GetSectionMeas1~
gets the meas1-attribute of a section which is stores in the given slot

*/
    void GetSectionMeas1(int slotIndex, double &inout_meas1){
        int pos = GetContentIndex(slotIndex)+OFFSET_MEAS_1;
        double nmeas1;
        char *m;
        GetPageContent(pos,sizeof(double), m);
        memcpy(&nmeas1, m, sizeof(double));
        inout_meas1 = nmeas1;
    }

/*
Method ~GetSectionMeas2~
gets the meas2-attribute of a section which is stores in the given slot

*/
    void GetSectionMeas2(int slotIndex, double &inout_meas2){
        int pos = GetContentIndex(slotIndex)+OFFSET_MEAS_2;
        double nmeas2;
        char *m;
        GetPageContent(pos,sizeof(double), m);
        memcpy(&nmeas2, m, sizeof(double));
        inout_meas2 = nmeas2;
    }

/*
Method ~GetSectionRid~
gets the rid of a section which is stores in the given slot

*/
    void GetSectionRid(int slotIndex, int &inout_rid){
        int pos = GetContentIndex(slotIndex)+OFFSET_RID;
        int nrid;
        char* r;
        GetPageContent(pos,sizeof(int), r);
        memcpy(&nrid, r, sizeof(int));
        inout_rid = nrid;
    }

/*
Method ~GetSectionCost~
gets the cost of a section which is stored in the given slot

*/
    void GetSectionCost(int slotIndex, double &inout_cost){
        int pos = GetContentIndex(slotIndex)+OFFSET_COST;
        double ncost;
        char* c;
        GetPageContent(pos,sizeof(double), c);
        memcpy(&ncost, c, sizeof(double));
        inout_cost = ncost;
    }

/*
Method ~GetSectionDuration~
gets the Duration of a section which is stored in the given slot

*/
    void GetSectionDuration(int slotIndex, double &inout_duration){
        int pos = GetContentIndex(slotIndex)+OFFSET_DURATION;
        double nduration;
        char* d;
        GetPageContent(pos,sizeof(double), d);
        memcpy(&nduration, d, sizeof(double));
        inout_duration = nduration;
    }

/*
Method ~GetSectionNoNeighbours~
gets number of neighbours of section, which is stored in the given
Slotindex

*/
    void GetSectionNoNeighbours(int slotIndex, int &inout_noN){
        int pos = GetContentIndex(slotIndex)+OFFSET_NO_NEIGHBOUR;
        int nnoN;
        char* n;
        GetPageContent(pos,sizeof(int), n);
        memcpy(&nnoN, n, sizeof(int));
        inout_noN = nnoN;
    }



    private:
/*
4.5.4 Private methods of class ~PageRecord~

private Set-methode to stored content on the page

*/
       void SetPageContent(char* in_content, int position, unsigned int count){
            for (unsigned int i=0; i < count; i++){
                m_pageContent[position] = in_content[i];
                position++;
            }
        }
/*
private set-methode to stored content in the slotIndex

*/
       void SetSlotContent(char* in_content, int slotindex, unsigned int count){
            int sindex = sizeof(int)*(slotindex-1);
            for (unsigned int i=0; i < count; i++){
                m_slotIndex[sindex] = in_content[i];
                sindex++;
            }
        }
/*
private get-methode to get content from the page

*/
        void GetPageContent(int position, unsigned int count,
                            char* &inout_content){
            char c[count];
            for (unsigned int i=0; i< count; i++){
                c[i]=m_pageContent[position];
                position++;
            }
            inout_content = c;
        }
/*
private get-methode to get content from the slotindex

*/
        int GetContentIndex(int slotIndex){
            //den Contenindex aus dem Slotindex lesen
            int sIndex = sizeof(int)*(slotIndex-1);
            int posContent;
            char c_pos[sizeof(int)];
            for (unsigned int i=0; i<sizeof(int); i++){
                c_pos[i]=m_slotIndex[sIndex];
                sIndex++;
            }
            memcpy(&posContent, c_pos, sizeof(int));
            return posContent;
        }

/*
4.5.5 Private fields of class ~PageRecord~

Field NextFree, stored the index of the PageContent, where the next
section can be inserted

*/
        int m_nextFree;

/*
Field SlotIndex, that contains MAX\_SLOTS by 4bit for
the Index of the Content, to indicate, where a slot is beginning

*/
        char m_slotIndex[MAX_SLOTINDEXSIZE];

/*
Field PageContent contains ca. 81 Byte for one Slot
the size of a slot is variable, unused bits are filled with 0

*/
        char m_pageContent[MAX_PAGESIZE];
/*
Field m\_isFull is a bool, if true , when no more space for a
new, otherwise false

*/
        bool m_isFull;
};

/*
4.6 Class DirectedSectionPair

This class is needed for the list of sections used in each entry
in the adjacency-list

*/
class DirectedSectionPair
{
  public:

/*
4.6.1 Constructors:

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

/*
4.6.2 Methods of class ~DirectedSectionPair~

less than operator

*/
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
4.6.3 Private fields of class ~DirectedSectionPair~

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
4.7 Struct AdjacencyListEntry

Used for the adjacency-list of the network

*/
struct AdjacencyListEntry
{
/*
4.7.1 Constructors:

*/
  AdjacencyListEntry() {}

  AdjacencyListEntry( int in_iLow,
                      int in_iHigh ):
  m_iLow( in_iLow ),
  m_iHigh( in_iHigh )
  {
  }

  AdjacencyListEntry( const AdjacencyListEntry& in_xEntry ):
  m_iLow( in_xEntry.m_iLow ),
  m_iHigh( in_xEntry.m_iHigh )
  {
  }

/*
4.7.2 Private fields of struct ~AdjacencyListEntry~

The lower index in the adjacency lists sub-array.

*/
  int m_iLow;

/*
The higher index in the adjacency lists sub-array.

*/
  int m_iHigh;
};

/*
4.8 Struct JunctionSortEntry

A helper struct for junction lists.

*/
struct JunctionSortEntry
{

/*
4.8.1 Constructors:

*/
  JunctionSortEntry()
  {
  }

  JunctionSortEntry(bool in_bFirstRoute,
                    Tuple* in_pJunction):
    m_bFirstRoute(in_bFirstRoute),
    m_pJunction(in_pJunction)
  {
  }

/*
4.8.2 Methods of struct JunctionSortEntry

less than operator

*/
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

/*
Method ~GetRouteMeas~

*/
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

/*
Method ~GetOtherRouteMeas~

*/
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

/*
Method ~GetOtherRouteId~

*/
  int GetOtherRouteId() {
    CcInt* pRouteId;
    if (m_bFirstRoute) {
      pRouteId = (CcInt*)m_pJunction->GetAttribute(JUNCTION_ROUTE2_ID);
    } else {
      pRouteId = (CcInt*)m_pJunction->GetAttribute(JUNCTION_ROUTE1_ID);
    }
    return pRouteId->GetIntval();
  }

/*
Method ~GetUpSectionId~

*/
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

/*
Method ~GetDownSectionId~

*/
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

/*
4.8.3 Fields of struct JunctionSortEntry

*/
  bool m_bFirstRoute;

  Tuple* m_pJunction;
};



/*
4.9 Class ~Network~

Central object of network concept. All other positions are given related to a
network object.

*/
class Network
{
  public:

/*

4.9.1 Type info of relation of the class ~network~

The external ~routes~ relation type info as string.

*/
  static string routesTypeInfo;

/*
The internal ~routes~ relation type info as string.

*/
  static string routesInternalTypeInfo;


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
The B-Tree in the ~section~ relation type info as string.

*/
static string sectionsBTreeTypeInfo;


/*
~distancestoreageTypeInfo~ only used for experimental netdistance precomputing.

*/
static string distancestorageTypeInfo;

/*

4.9.2 Constructors and destructor of the class ~network~

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
4.9.3 Support function of class ~network~

The ~Out~, ~Save~, and ~Open~ functions of the type constructor ~network~.
The ~In~ function is not provided, given that the creation of the network
is only done via the ~thenetwork~ operator.

~Out~-Method

Outputs a network

*/
  ListExpr Out(ListExpr typeInfo);

/*
~Save~-Method

Saves all relations of the network

*/
  ListExpr Save( SmiRecord& valueRecord,
                 size_t& offset,
                 const ListExpr typeInfo );

/*
~Open~-Method

Opens a network

*/
  static Network* Open( SmiRecord& valueRecord,
                        size_t& offset,
                        const ListExpr typeInfo );
/*
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

/*
SetDefined

*/
 void SetDefined(bool def)
 {
   m_bDefined = def;
 };

/*
4.9.4 Methods of the class ~network~

Method ~Destroy~

This function sets all information inside the network to be
destroyed when the destructor is called.

*/
    void Destroy();

/*
Method ~Load~

Loads a network given two relations containing the routes and junctions.
This function corresponds to the operator ~thenetwork~.

*/
    void Load(int in_iId,
              const Relation *in_pRoutes,
              const Relation *in_pJunctions);

/*
Method ~GetId~

Returns the id of this network

*/
    int GetId();

/*
Method ~GetRoutes~

Returns a copy of the ~routes~ relation in external representation.
This function is used in the ~routes~ operator.

*/
    Relation *GetRoutes();

/*
Method ~GetJunctions~

Returns a copy of the ~junctions~ relation in external representation.
This function is used in the ~junctions~ operator.

*/
    Relation *GetJunctions();

/*
Method ~GetJunctionsOnRoute~

Returns the junction from the start of the route to the end.

*/
    void GetJunctionsOnRoute(CcInt* in_pRouteId,
                             vector<JunctionSortEntry>& inout_xJunctions);

/*
Method ~GetSectionOnRoute~

Returns the section ~tuple~ of the network which includes the ~GPoint~.
Attention: The tupleId of the returned Tuple is not the TupleId in the sections
relation! If you need the correct tupleId of the sections relation use
~GetTupleIdSectionOnRoute~ instead.

*/
 Tuple*  GetSectionOnRoute(GPoint* in_xGPoint);

/*
Method ~GetTupleIdSectionOnRoute~
Returns the ~tupleid~  of the section which includes the ~GPoint~ .

*/
 TupleId GetTupleIdSectionOnRoute(GPoint* in_xGPoint);

/*
Method ~GetPointOnRoute~

Returns the point value of the GPoint on the route. Used for translation of
network positions into spatial 2D positions.

*/
  void GetPointOnRoute(const GPoint* in_xGPoint, Point *&res);

/*
Mehtod ~GetRoute~

Returns the route tuple for the given route id.

*/
  Tuple* GetRoute(int in_RouteId);
  Tuple* GetRoute(TupleId in_routeTID);

/*
Method ~GetRouteCurve~

Returns the route curve for the given route id.

*/
  SimpleLine GetRouteCurve(int in_iRouteId);

/*
Method ~GetDual~

Returns the dual value of the given route id.

*/
  bool GetDual(int in_iRouteId);

/*
Method ~GetLineValueOfRouteInterval~

Returns the ~sline~ representing the ~RouteInterval~ in spatial data. Used
to translate network values into spatial 2D values.

*/
  void GetLineValueOfRouteInterval(const RouteInterval* in_rI,
                                   SimpleLine *out_line);

/*
Method ~GetSections~

Returns a copy of the ~sections~ relation in external representation.
This function is used in the ~sections~ operator.

*/
  Relation *GetSections();

/*
Method ~GetNetworkPosOfPoint~

Returns a GPoint representing the point value in the network if possible, undef
elsewhere. Used to translate spatial 2D positions into network positions.

*/
  GPoint* GetNetworkPosOfPoint(Point p);

/*
Methode ~GetSectionsInternal~

Returns the ~sections~ relation (in internal representation).

*/
    Relation *GetSectionsInternal();

/*
Method ~GetAdjacentSection~

Returns a vector of sections which can be reached next upwards respectively
downwards bool from the section given by TupleId.

*/

    void GetAdjacentSections(TupleId in_iSectionId,
                             bool in_bUpDown,
                             vector<DirectedSection> &inout_xSections);

/*
Method ~GetAdjacentSectionsInfo~

Returns a vector of sections which can be reached next upwards respectively
downwards bool from the section given by TupleId.

*/
    void GetAdjacentSectionsInfo ( TupleId in_iSectionTId,
         bool in_bUpDown,vector<DirectedSectionInfo> &inout_xSections );

/*
Method ~GetSectionsOfRouteInterval~

Returns a set of sections which are covered by the given ~RouteInterval~
~inri~ as ~DbArray~ of SectTreeEntries.

*/

    void GetSectionsOfRouteInterval(const RouteInterval *in_ri,
                                    DbArray<SectTreeEntry> *io_SectionIds);

    void GetSectionsOfRoutInterval(const RouteInterval *in_ri,
                                  vector<TupleId> &res);

/*
Methode ~Find~

Get Routeinterval for Halfsegment defined by point interval. Used to translate
spatial 2D values into network values.

*/

    RouteInterval* Find(Point p1, Point p2);

/*
Method ~FindInterval~

Get Routepart passed limited by the two points.

*/
    RouteInterval* FindInterval(Point p1, Point p2);


/*
Method ~GetJunctionMeasForRoutes~

Computes the junction of the two given routes and returns the route measure
values of the junction on this routes.

*/

void GetJunctionMeasForRoutes(CcInt *pLastRouteId, CcInt *pCurrentSectionRid,
                                  double& rid1meas, double& rid2meas);

/*
Method ~GetSection~

Returns the section with the given id.

*/
    Tuple* GetSection(TupleId n);
/*
Method ~FindSP~

*/
    void FindSP(TupleId j1,TupleId j2,double& length,GLine* res);

/*
Method ~UpdateSectionCost~ and ~UpdateMoreSectionCost~

Update the cost of one or more sections

*/
    bool UpdateSectionCost(int secId,double newCost);
    bool UpdateMoreSectionCost(const Relation* in_pCosts);
/*
Methode ~UpdateSectionDuration~ and ~UpdateMoreSectionDuration~
Update the duration of a section

*/
    bool UpdateSectionDuration(int secId,double newDuration);
    bool UpdateMoreSectionDuration(const Relation* in_pDurations);
/*
Methode ~OptimizeNetwork~

optimizes the order of the ~routes~, after that all ~sections~ are generated new
and  all structures based on these ~sections~ are adjusted

*/
    bool OptimizeNetwork();
/*
Method ~PrintAdjacentSectionInfo~

Prints informations of ~sections~ which are stored in a ~PageRecord~.

*/
    bool PrintAdjacentSectionInfo();

/*
Method ~GetTupleIdSectionOnRouteJun~

*/
  void GetTupleIdSectionOnRouteJun(GPoint* in_xGPoint,vector<TupleId>&);

/*
Method ~GetNoJunctions~

*/
  int GetNoJunctions(){return m_pJunctions->GetNoTuples();}

/*
Method ~GetNoRoutes~

*/
  int GetNoRoutes(){return m_pRoutes->GetNoTuples();}

private:


/*
4.9.5 Private methods of class ~Network~

Method ~FillRoutes~

Copies the relation given as argument to the ~routes~ relations sorted by
the ~id~ attribute and creates the B-Tree in this attribute.

*/
  void FillRoutes( const Relation *in_pRoutes );

/*
Method ~OptimizationRoutes~

determined for all ~routes~ their lexicographic start point, stores them in an
array and calls the sort method ~QuicksortRoutes~

*/
  bool OptimizationRoutes();

/*
Method ~QuicksortRoutes~

calculates the pivot-element-index with the method
~QuicksortRoutesPartition~. After that ~QuicksortRoutes~ is recursively called
for the upper and lower part of the array

*/
  void QuicksortRoutes(int a[][2], int l, int r);

/*
Method ~QuicksortRoutesPartition~

split the given array and sorts the array so that all elements before the
pivot-element are smaller and all elements that are above the pivot element
are larger.

*/
  int QuicksortRoutesPartition(int a[][2], int l, int r);

/*
Method ~CreateRoutesIndexes~

*/
  void CreateRoutesIndexes();

/*
Method ~FillJunctions~

Copies the relation given as argument to the ~junctions~ relation appending
the new attributes.

*/
  void FillJunctions( const Relation *in_pJunctions );

/*
Method ~FillSections~

Given the ~routes~ and ~junctions~ relation, the ~sections~ relation
is retrieved.

*/
  void FillSections();

/*
Method ~FillAdjacencyLists~

Given that all relations are set up, the adjacency lists are created.

*/
  void FillAdjacencyLists();

/*
Method ~FillAdjacencyPair~

*/
  void FillAdjacencyPair(TupleId in_pFirstSection,
                         bool in_bFirstUp,
                         TupleId in_pSecondSection,
                         bool in_bSecondUp,
                         ConnectivityCode in_xCc,
                         Transition in_xTransition,
                         vector<DirectedSectionPair> &inout_xPairs);

/*
Method ~FillDistanceStorage~

Used for experiments with network distance computation to store precomputed
distances. Because of the big data overhead and computation time this methods
and parameters should not be used if not needed. Therefore their calls are
comment out in the code. Of network constructors.

*/

  void FillDistanceStorage();

/*
Methods ~ShorterConnection~, ~ShorterConnection2~ and ~InShortestPath~

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
Method ~CountNeighbours~
count the neighbours of all sections

*/
void CountNeighbours(vector<DirectedSectionPair> in_xList);


/*
4.9.6 Private fields of Class ~Network~

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
The BTree in the ~routes~ relation by startpos\_id.

*/

  BTree* m_pBTreeRoutesByStartposId;

/*
The RTree in the ~routes~ relation

*/

  R_Tree<2,TupleId> *m_pRTreeRoutes;

/*
The BTree in the ~junctions~ relation by route1.

*/
  BTree* m_pBTreeJunctionsByRoute1;

/*
The BTree in the ~junctions~ relation by route2.

*/
  BTree* m_pBTreeJunctionsByRoute2;


/*
The adjacency lists of sections.

*/

  DbArray<PageRecord> m_xAdjacencyList;

/*
The adjacency lists of sections.

*/
  DbArray<DirectedSection> m_xSubAdjacencyList;

/*
The BTree in the ~sections~ relation by routeId.

*/

  BTree* m_pBTreeSectionsByRoute;
/*
The BTree in the ~junctions~ relation.

*/
  BTree* m_pBTreeSections;

/*
Field alldistance

Stores the precomputed distances between every possible pair of two junction
points. Only for experimental use in network distance computing. Comment out
in the code if not needed the computational and storage space overhead is to
big.

*/

  Relation* alldistance;

};


/*
5 ~gline~

5.1 class ~GLine~

*/


class GLine : public Attribute
{
/*
5.1.1 Constructors and Destructor:

The simple constructor. Should not be used.

*/
  public:
    GLine();

/*
Constructor

~iniSize~ gives the number of expected ~RouteIntervals~. If it is to small
the size of the ~DbArray~ is dynamically extended later.

*/
    GLine(int in_iSize);

    GLine(const GLine* in_xOther);

    GLine( ListExpr in_xValue,
         int in_iErrorPos,
         ListExpr& inout_xErrorInfo,
         bool& inout_bCorrect);

    ~GLine() {};

/*
5.1.2 Methods of class ~gline~

*/

    ostream& Print( ostream& os ) const;

    void SetNetworkId(int in_iNetworkId);

    void AddRouteInterval(int in_iRouteId,
                          double in_dStart,
                          double in_dEnd);

    void AddRouteInterval(RouteInterval ri);

    double GetLength ();

    int GetNetworkId();

    void Get(const int i, RouteInterval &ri) const;

    int NoOfComponents();

    bool IsSorted();

    void SetSorted(bool);

    DbArray<RouteInterval>* GetRouteIntervals();

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
5.1.3 Support function of ~gline~ for Secondo Integration

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

    Flob *GetFLOB( const int i );

    static int SizeOf();

    size_t Sizeof() const;

    int Size() const;

    static ListExpr Property();

    static bool Check( ListExpr type, ListExpr& errorInfo );

    int Compare(const Attribute*) const;

    bool Adjacent(const Attribute*) const;

    GLine* Clone() const;

    size_t HashValue() const;

    void CopyFrom(const Attribute*);

    void TrimToSize()
     {
       m_xRouteIntervals.TrimToSize();
     };

    inline static const string BasicType() { return "gline"; }
    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }

  private:

/*
5.1.4 Private Fields of class ~gline~

The network id.

*/

    int m_iNetworkId;


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

    DbArray<RouteInterval> m_xRouteIntervals;


};



/*
5.2 ~struct RITree~

Used to compress and sort ~RouteInterval~s. Because many operators take profit
from sorted ~RouteInterval~s. We sort and compress resulting ~DbArray~s of
~RouteInterval~s whenever it is possible.

*/

struct RITree {
/*
5.2.1 Constructors and Destructor:

*/
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
5.2.2 Methods of Struct RITree

Method ~CheckTree~

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
Method ~InsertUnit~

Inserts a ~RouteInterval~ in the ~RITree~ checking if there are already
~RouteIntervals~ which overlap or are adjacent to the current ~RouteInterval~.

*/
  void InsertUnit(int rid, double pos1, double pos2)
  {
    if (pos1 < pos2) Insert(rid, pos1, pos2);
    else Insert(rid, pos2, pos1);
  }

/*
Method ~Insert~

*/
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
Method ~TreeToGLine~

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
Method ~TreeToDbArray~

Stores the ~RouteInterval~s of the ~RITree~ sorted by their route ids and
start positions into a ~DbArray~. Used to build trajectory of ~mgpoint~.

*/

  void TreeToDbArray (DbArray<RouteInterval>* arr) {
    if (this->m_left != 0) {
      this->m_left->TreeToDbArray (arr);
    }
    arr->Append(RouteInterval(this->m_iRouteId, this->m_dStart, this->m_dEnd));
    if (this->m_right != 0) {
      this->m_right->TreeToDbArray (arr);
    }
  };

/*
Method ~RemoveTree~

Deletes the tree.

*/
  void RemoveTree(){
    if (m_left != 0) m_left->RemoveTree();
    if (m_right != 0) m_right->RemoveTree();
    delete this;
  };

/*
5.2.3 Fields of Struct RITree

*/
  int m_iRouteId;
  double m_dStart, m_dEnd;
  RITree *m_left, *m_right;
};

/*
6 GPoints by Jianqiu Xu.

6.1 string edistjoinpointlist

*/

extern string edistjoinpointlist;

/*
6.2 class GPoints

*/
class GPoints: public Attribute{
public:
/*
6.2.1 Constructors and Destructor of class ~GPoints~

*/
  GPoints();
  GPoints(int in_iSize);
  GPoints(GPoints* in_xOther);
  ~GPoints(){}
/*
6.2.2 Methods of class ~GPoints~

*/
  ostream& Print(ostream& os)const;
  inline bool IsEmpty()const;
  size_t Sizeof()const;
  GPoints& operator+=(const GPoint &gp);
  GPoints& operator-=(const GPoint &gp);
  int NumOfFLOBs()const;
  Flob* GetFLOB(const int i);
  void Get(int i, GPoint& gp)const;
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
  inline static const string BasicType() { return "gpoints"; }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }
  int Compare(const Attribute*)const;
  bool Adjacent(const Attribute*)const;
  GPoints* Clone()const;
  size_t HashValue()const;
  void CopyFrom(const Attribute* right);
  GPoints& operator=(const GPoints& gps);
  void FilterAliasGPoints(Network *pNetwork);
  void TrimToSize () {m_xGPoints.TrimToSize();}

private:
/*
6.2.3 Private fields of class ~GPoints~

*/
  DbArray<GPoint> m_xGPoints;
};

} // end of namspace network2

#endif // __NETWORK_ALGEBRA_H__
