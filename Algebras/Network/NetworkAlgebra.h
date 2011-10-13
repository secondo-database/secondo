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

1.1 Declarations and Inclusions Necessary for Algebra Network

*/
#ifndef __NETWORK_ALGEBRA_H__
#define __NETWORK_ALGEBRA_H__

#include "../../include/NestedList.h"
#include "../TupleIdentifier/TupleIdentifier.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "../BTree/BTreeAlgebra.h"
#include "../../Tools/Flob/DbArray.h"
#include "../../Tools/Flob/Flob.h"
#include "../../include/Attribute.h"
#include "../Spatial/SpatialAlgebra.h"
#include "../RTree/RTreeAlgebra.h"
#include "../../include/ListUtils.h"


/*
2 Helpful Data Types and Data Structures

2.1 ~SectionValue ~
For some operations in connection with paths we need lists containing
sections ids and corresponding directions, e.g. in a list of adjacent sections.
The data type ~SectionValue~ is defined to be used in this lists.

*/

class SectionValue
{
public:
  SectionValue(){};

  SectionValue(const int sid, const bool up): sectId(sid), upDown(up){};

  ~SectionValue(){};

  inline int GetSectionID() const
  {
    return sectId;
  }

  inline bool GetUpDownFlag() const
  {
    return upDown;
  }

  inline void SetSectionID(const int sid)
  {
    sectId = sid;
  }

  inline void SetUpDownFlag(const bool up)
  {
    upDown = up;
  }

  ostream& Print(ostream& os) const
  {
    os << "SectId: " << sectId << ", Direction: ";
    if (upDown) os << "up";
    else os << "down";
    os << endl;
    return os;
  }

  int Compare(const SectionValue sv) const
  {
    if (sectId < sv.GetSectionID()) return -1;
    if (sectId > sv.GetSectionID()) return 1;
    if (upDown == sv.GetUpDownFlag()) return 0;
    if (upDown < sv.GetUpDownFlag()) return -1;
    return 1;
  }

  void operator=(const SectionValue sv)
  {
    sectId = sv.GetSectionID();
    upDown = sv.GetUpDownFlag();
  }

private:

  int sectId;
  bool upDown;

};

/*
2.2 ~Entry~
Extends a arbirtray class object by two int values used as pointers to the
index of the left respectively right son, to use them as nodes in a tree
data structure which is embedded in a DbArray to be not limited by available
main memory ressources.

The class Value must support Compare,Print and operator=.

*/

template <class Value>
class Entry
{
public:
  Entry<Value>(){};

  Entry<Value>(const Value val, const int left = -1, const int right = -1):
      value(val), leftSonIndex(left), rightSonIndex(right) {};

  ~Entry(){};

  inline Value GetValue() const  {
    return value;
  };

  inline int GetLeftSonIndex() const  {
    return leftSonIndex;
  };

  inline int GetRightSonIndex() const  {
    return rightSonIndex;
  };

  inline void SetValue(const Value v)  {
    value = v;
  }

  inline void SetLeftSonIndex(const int left)  {
    leftSonIndex = left;
  };

  inline void SetRightSonIndex(const int right)  {
    rightSonIndex = right;
  };

  inline int Compare(const Entry ev) const  {
    return value.Compare(ev.GetValue());
  };

  inline int Compare (const Value v) const{
    return value.Compare(v);
  }

  ostream& Print(ostream& os) const  {
    os << "EntryValue: ";
    value.Print(os);
    os << "LeftSon: " << leftSonIndex;
    os << ", RigthSon: " << rightSonIndex << endl;
    return os;
  };

private:

  Value value;
  int leftSonIndex;
  int rightSonIndex;
};

/*
2.3 ~SortedTree~
Uses a DbArray to store a sorted tree of ~TreeEntry~. The elements can be
searched, inserted and removed.

*/

template <class TreeEntry>
class SortedTree {

public:

  SortedTree<TreeEntry>():tree(0) {
    freePos = 0;
    numOfElements = 0;
  };

  SortedTree<TreeEntry>(const int n): tree(n){
    freePos = 0;
    numOfElements = 0;
  };

  ~SortedTree(){};

  void Destroy(){
    tree.Destroy();
  };

  ostream& Print(ostream& os) const
  {
    TreeEntry elem;
    for (int i = 0; i < freePos; i++){
      tree.Get(i,elem);
      os << i << ".Element: ";
      elem.Print(os);
      os << endl;
    }
    return os;
  }

/*
Returns the index of entry if entry is in the tree. Elsewhere the position
where entry would have to be inserted. If the tree is empty -1 is returned.

*/
  int Find(const TreeEntry entry, const int pos, int& posFather) const{
    assert((0 <= pos && pos < freePos) || (freePos == 0 && pos == 0));
    if (isEmpty()) return -1;
    TreeEntry actEntry;
    tree.Get(pos,actEntry);
    switch (actEntry.Compare(entry)){
      case -1:{
        if (actEntry.GetRightSonIndex() > -1){
          posFather = pos;
          return Find(entry, actEntry.GetRightSonIndex(), posFather);
        } else {
          posFather = -1;
          return pos;
        }
      }

      case 1:{
        if (actEntry.GetLeftSonIndex() > -1){
          posFather = pos;
          return Find(entry, actEntry.GetLeftSonIndex(),posFather);
        }
        else{
          posFather = -1;
          return pos;
        }
      }

      case 0: {
        return pos;
      }

      default:{ //should never been reached
        posFather = -1;
        return -1;
      }
    }
  }


/*
Inserts ~TreeEntry~ if it is not inserted before.
If te is already in the tree, the tree remains unchanged and false is returned.

*/
  void Insert(const TreeEntry te) {
    int posFather = -1;
    int pos = Find(te,0,posFather);
    if (pos < 0){
      tree.Put(freePos, te);
      freePos++;
      numOfElements++;
    } else {
      TreeEntry test;
      tree.Get(pos,test);
      switch (test.Compare(te)){
        case -1:{
          tree.Put(freePos, te);
          test.SetRightSonIndex(freePos);
          tree.Put(pos,test);
          freePos++;
          numOfElements++;
          break;
        }

        case 1: {
          tree.Put(freePos, te);
          test.SetLeftSonIndex(freePos);
          tree.Put(pos,test);
          freePos++;
          numOfElements++;
          break;
        }

        default:{
          break;
        }
      }
    }
  }

/*
Removes te from the tree, if it is in there.

*/

  void Remove(const TreeEntry te){
    int posFather = -1;
    int pos = Find(te,0,posFather);
    if (pos >= 0){
      TreeEntry test;
      tree.Get(pos,test);
      if (test.Compare(te) == 0){
        if (pos > 0){
          TreeEntry father;
          tree.Get(posFather, father);
          if (test.GetRightSonIndex() > 0 && test.GetLeftSonIndex() > 0) {
            int newPos = test.GetRightSonIndex();
            int newFatherPos = pos;
            TreeEntry smallestBigger;
            tree.Get(newPos,smallestBigger);
            while (smallestBigger.GetLeftSonIndex() > 0){
              newFatherPos = newPos;
              newPos = smallestBigger.GetLeftSonIndex();
              tree.Get(newPos, smallestBigger);
            }
            if (newFatherPos == pos){
              if (father.GetRightSonIndex() == pos){
                father.SetRightSonIndex(newPos);
              } else {
                father.SetLeftSonIndex(newPos);
              }
              smallestBigger.SetLeftSonIndex(test.GetLeftSonIndex());
              tree.Put(newPos, smallestBigger);
            } else {
              TreeEntry newFather;
              tree.Get(newFatherPos, newFather);
              newFather.SetLeftSonIndex(smallestBigger.GetRightSonIndex());
              tree.Put(newFatherPos,newFather);
              if (father.GetRightSonIndex() == pos){
                father.SetRightSonIndex(newPos);
              } else {
                father.SetLeftSonIndex(newPos);
              }
              smallestBigger.SetLeftSonIndex(test.GetLeftSonIndex());
              smallestBigger.SetRightSonIndex(test.GetRightSonIndex());
              tree.Put(newPos, smallestBigger);
            }
          } else {
            if (test.GetLeftSonIndex() > 0 || test.GetRightSonIndex() > 0){
              if (test.GetLeftSonIndex() > 0){
                if (father.GetRightSonIndex() == pos){
                  father.SetRightSonIndex(test.GetLeftSonIndex());
                } else {
                  father.SetLeftSonIndex(test.GetLeftSonIndex());
                }
              } else {
                if (father.GetRightSonIndex() == pos){
                  father.SetRightSonIndex(test.GetRightSonIndex());
                } else {
                  father.SetLeftSonIndex(test.GetRightSonIndex());
                }
              }
            } else {
              if (father.GetRightSonIndex() == pos){
                father.SetRightSonIndex(-1);
              } else {
                father.SetLeftSonIndex(-1);
              }
            }
          }
          tree.Put(posFather,father);
        } else {
          if (test.GetRightSonIndex() > 0 && test.GetLeftSonIndex() > 0) {
            int newPos = test.GetRightSonIndex();
            int newFatherPos = pos;
            TreeEntry smallestBigger;
            tree.Get(newPos,smallestBigger);
            while (smallestBigger.GetLeftSonIndex() > 0){
              newFatherPos = newPos;
              newPos = smallestBigger.GetLeftSonIndex();
              tree.Get(newPos, smallestBigger);
            }
            TreeEntry newFather;
            tree.Get(newFatherPos, newFather);
            newFather.SetLeftSonIndex(smallestBigger.GetRightSonIndex());
            tree.Put(newFatherPos,newFather);
            smallestBigger.SetLeftSonIndex(test.GetLeftSonIndex());
            smallestBigger.SetRightSonIndex(test.GetRightSonIndex());
            tree.Put(pos, smallestBigger);
          } else {
            if (test.GetLeftSonIndex() > 0 || test.GetRightSonIndex() > 0){
              TreeEntry newRoot;
              if (test.GetLeftSonIndex() > 0){
                tree.Get(test.GetLeftSonIndex(),newRoot);
              } else {
                tree.Get(test.GetRightSonIndex(),newRoot);
              }
              tree.Put(pos,newRoot);
            } else {
              freePos = 0;
            }
          }
        }
        numOfElements--;
      }
    }
  }

/*
Returns true if the tree is empty.

*/
  inline bool isEmpty() const{
    return numOfElements == 0;
  }

private:
  DbArray<TreeEntry> tree;
  int freePos;
  int numOfElements;

};


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

  SectTreeEntry(const TupleId n, const int r, const double st, const double e,
                const bool stb, const bool eb) {
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

ostream& Print ( ostream& os ) const
{
  os << "SectTreeEntry: TupleId: " << secttid << endl;
  os << "RouteId: " << rid << ", start: " << start;
  os << ", end: " << end << ", startbool: " << startbool;
  os << ", endbool: " << endbool << endl;
  os << endl;
  return os;
};

  TupleId secttid;
  int rid;
  double start, end;
  bool startbool, endbool;
};


class ShortestPathTreeEntry
{
public:
  ShortestPathTreeEntry(){};

  ShortestPathTreeEntry(const double d, const bool up):
        dist(d),upDown(up)
  {};

  ~ShortestPathTreeEntry(){};

  inline void SetDist(const double d)
  {
    dist = d;
  }

  inline void SetUpDown (const bool up)
  {
    upDown = up;
  }

  inline double GetDist() const
  {
    return dist;
  }

  inline  bool GetUpDown() const
  {
    return upDown;
  }

  ostream& Print(ostream& os) const
  {
    os << "Distance: " << dist << ", Up: " << upDown << endl;
    return os;
  }

  int Compare(const ShortestPathTreeEntry sp) const
  {
    if (dist < sp.dist) return -1;
    if (dist > sp.dist) return 1;
    return 0;
  }

private:
  double dist;
  bool upDown;
};


class GPoints;
class Network;
class GLine;
class GPoint;

/*
2 GLine

Every ~gline~ consists of a set of ~RouteInterval~s stored in a ~DbArray~.

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

  RouteInterval(const int in_iRouteId,
                const double in_dStart,
                const double in_dEnd):
      m_iRouteId(in_iRouteId),
      m_dStart(in_dStart),
      m_dEnd(in_dEnd) {};

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

  Rectangle<2> BoundingBox(const Network* pNetwork) const;

/*
Get and Set private values.

*/
  inline int GetRouteId() const
  {
    return m_iRouteId;
  };

  inline double GetStartPos() const
  {
    return m_dStart;
  };

  inline double GetEndPos() const
  {
    return m_dEnd;
  };

  inline double GetValue(const bool start) const
  {
    if (start) return m_dStart;
    else return m_dEnd;
  };

  inline void SetRouteId(const int rid)
  {
    m_iRouteId = rid;
  };

  inline void SetStartPos(const double pos)
  {
    m_dStart = pos;
  };

  inline void SetEndPos(const double pos)
  {
    m_dEnd = pos;
  };

  inline double Length() const
  {
    return fabs(GetEndPos() - GetStartPos());
  }

  RouteInterval& operator=(const RouteInterval& ri)
  {
    m_iRouteId = ri.m_iRouteId;
    m_dStart = ri.m_dStart;
    m_dEnd = ri.m_dEnd;
    return *this;
  }

  bool Contains(const RouteInterval *ri, const double tolerance) const;
  bool Contains(const GPoint *gp)const;
  bool Intersects(const RouteInterval *ri, const double tolerance) const;
  ostream& Print(ostream& os) const;


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

Assignment operator.

*/

  RLoc& operator=( const RLoc& rloc )
  {
    rid = rloc.rid; d = rloc.d; side = rloc.side;
    return *this;
  }

  void SetRouteId(const int r){rid = r;}
  void SetPosition(const double p) {d = p;}
  void SetSide(const Side s){side = s;}

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
class Attribute.

*/

class GPoint : public Attribute
{
  public:

/*
3.4.1 Constructors and Destructor

*/

    GPoint():Attribute()
    {}

    GPoint(const bool def)
      : Attribute(def)
    {}

    GPoint( const bool in_bDefined,
            const int in_iNetworkId,
            const int in_xRid,
            const double in_dLocation,
            const Side in_xSide = None ):
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
3.4.2 Methods of class ~gpoint~

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

    inline void SetRouteId( const int in_rid )
    {
      m_xRouteLocation.SetRouteId(in_rid);
    }

    inline void SetPosition(const  double pos )
    {
      m_xRouteLocation.SetPosition(pos);
    }

    inline void SetSide( const Side s )
    {
      m_xRouteLocation.SetSide(s);
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

    void CopyFrom( const Attribute* right )
    {
      const GPoint* gp = (const GPoint*)right;
      *this = *gp;
    }

    int Compare( const Attribute* arg ) const
    {
      const GPoint *p = (const GPoint*) arg;
      return Compare(*p);
    }

    int Compare(const GPoint p) const
    {
      if (m_iNetworkId < p.GetNetworkId())
      {
        return -1;
      }
      else
      {
        if (m_iNetworkId > p.GetNetworkId())
        {
          return 1;
        }
        else
        { // same network
          if (m_xRouteLocation.rid < p.GetRouteId())
          {
            return -1;
          }
          else
          {
            if (m_xRouteLocation.rid > p.GetRouteId())
            {
              return 1;
            }
            else
            { //same route
              if (m_xRouteLocation.d < p.GetPosition())
              {
                return -1;
              }
              else
              {
                if (m_xRouteLocation.d > p.GetPosition())
                {
                  return 1;
                }
                else
                { //same Position
                  if (m_xRouteLocation.side == 2 || p.GetSide() == 2 ||
                      m_xRouteLocation.side == p.GetSide())
                  {
                    return 0;
                  }
                  else
                  {
                    if (m_xRouteLocation.side < p.GetSide())
                    {
                      return -1;
                    }
                    else
                    {
                      return 1;
                    }
                  }
                }
              }
            }
          }
        }
      }
      return -1; //should never been reached
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
      if(IsDefined())
      {
      os << "NetworkId: " << m_iNetworkId
          << " RouteId: " << m_xRouteLocation.rid
          << "  Position: " << m_xRouteLocation.d
          << " Side: " << m_xRouteLocation.side << endl;
      }
      else
        os << "not defined." << endl;
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

    static bool CheckGPoint( ListExpr type, ListExpr& errorInfo );

    inline static const string BasicType() { return "gpoint"; }
    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }


/*
Returns the network distance between 2 ~gpoint~ using DijkstrasAlgorithm.

*/

    double Netdistance (const GPoint* toGPoint) const;

/*
Returns the network distance between 2 ~gpoint~ using AStar-Algorithm.

*/

    double NetdistanceNew (const GPoint* toGPoint) const;
    double NetdistanceNew (const GPoint* toGPoint,
                           const Network* pNetwork) const;

    double NetdistanceNew (const GLine* toGLine) const;
    double NetdistanceNew (const GLine* toGLine,
                           const Network* pNetwork) const;

    double NetdistanceNew (const GPoints* toGPoints) const;
    double NetdistanceNew (const GPoints* toGPoints,
                           const Network* pNetwork) const;

/*
Never used ?

*/
    double NewNetdistance(const GPoint* pToGPoint, GLine* res) const;//new

/*
Computes the euclidean distance of 2 glines.

*/
    double Distance (const GPoint* toGPoint) const ;

/*
Returns true if the gpoint is inside the gline false elsewhere.

*/

    bool Inside( const GLine *gl, const double tolerance) const;

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

    void ToPoint(Point *& res) const;

    Point* ToPoint() const;

    Point* ToPoint(const Network *&pNetwork) const;

/*
Returns the spatial Bounding Box of the point which is a rectangle degenerated
to a single point.

*/

    inline const Rectangle<2> BoundingBox(){
      if (IsDefined()) {
        Point *p = ToPoint();
        Rectangle<2> result = p->BoundingBox();
        p->DeleteIfAllowed();
        return result;
      } else return Rectangle<2> (false, 0.0, 0.0, 0.0, 0.0);
    };

/*
Returns a point degenerated rectangle as network bounding box of the gpoint

*/

    inline const Rectangle<2> NetBoundingBox() const {
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
Returns a gline representing the shortest path between two GPoint.
Using DijkstrasAlgorithm

*/

  bool ShortestPath(const GPoint *ziel, GLine *result,
                    DbArray<TupleId>* touchedSects = 0) const;
  bool ShortestPath(const GPoint* ziel, GLine *result,
                    const Network* pNetwork,
                    DbArray<TupleId>* touchedSects = 0) const;

/*
Returns a gline representing the shortest path between a GPoint.and
a gpoint or gline.

Using AStarAlgorithm

*/

  bool ShortestPathAStar(const GPoint *ziel, GLine *result,
                         DbArray<TupleId>* touchedSects = 0) const;
  bool ShortestPathAStar(const GPoint* ziel, GLine* result,
                         const Network *pNetwork,
                         DbArray<TupleId>* touchedSects = 0) const;

  bool ShortestPathAStar(const GLine *ziel, GLine *result,
                         DbArray<TupleId>* touchedSects = 0) const;
  bool ShortestPathAStar(const GLine* ziel, GLine* result,
                         const Network *pNetwork,
                         DbArray<TupleId>* touchedSects = 0) const;

  bool ShortestPathAStar(const GPoints *ziel, GLine *result,
                         DbArray<TupleId>* touchedSects = 0) const;
  bool ShortestPathAStar(const GPoints* ziel, GLine* result,
                         const Network *pNetwork,
                         DbArray<TupleId>* touchedSects = 0) const;

/*
Returns the shortest path tree from the gpoint to all sections of the
network. The distances are stored in an DbArray<double>, where the index
of the Array-Field is two times the section number for up sections and
two times the section number plus one for down sections.

*/

 void ShortestPathTree(const Network* pNetwork,
                       DbArray<ShortestPathTreeEntry> *res) const;

/*
Almost analogous to shortest path tree but stops computation if all sections
of ~toReach~ are inserted.

*/

 void ShortestPathTree(const Network* pNetwork,
                       DbArray<ShortestPathTreeEntry> *res,
                       SortedTree<Entry<SectionValue> > *toReach) const;


/*
Returns the reverse shortest path tree of the gpoint from all sections of the
network. The distances are stored in an DbArray<double>, where the index
of the Array-Field is two times the section number for up sections and
two times the section number plus one for down sections.

*/

void ReverseShortestPathTree(const Network* pNetwork,
                             DbArray<ShortestPathTreeEntry> *res) const;

/*
Almost analogous to reverse shortest path tree but stops computation if all
sections of ~toReach~ are inserted.

*/

void ReverseShortestPathTree(const Network* pNetwork,
                             DbArray<ShortestPathTreeEntry> *res,
                             SortedTree<Entry<SectionValue> > *toReach) const;

/*
Returns the route intervals from which the ~GPoint~ can be reached within a
given distance.

*/

void In_Circle(const Network* pNetwork, GLine *res,
               const double maxdist) const;

/*
Returns the route intervals which can be reached from the ~GPoint~ within a
given distance.

*/

void Out_Circle(const Network* pNetwork, GLine *res,
                               const double maxdist) const;

/*
Returns the route intervals whithin distance maxdist from ~GPoint~ ignoring
connectivity in the junctions.

*/
void Circle(const Network* pNetwork, GLine *res,
            const double maxdist) const;


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
  ConnectivityCode( const int in_iCc ):
    m_iConnectivityCode( in_iCc )
  {
  }

/*
4.2.2 Constructor for boolean values.

Constructs the connectivity code given the Boolean values for
all possibilities.

*/
  ConnectivityCode( const bool in_bAup_Aup,
                    const bool in_bAup_Adown,
                    const bool in_bAup_Bup,
                    const bool in_bAup_Bdown,
                    const bool in_bAdown_Aup,
                    const bool in_bAdown_Adown,
                    const bool in_bAdown_Bup,
                    const bool in_bAdown_Bdown,
                    const bool in_bBup_Aup,
                    const bool in_bBup_Adown,
                    const bool in_bBup_Bup,
                    const bool in_bBup_Bdown,
                    const bool in_bBdown_Aup,
                    const bool in_bBdown_Adown,
                    const bool in_bBdown_Bup,
                    const bool in_bBdown_Bdown ):
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
  bool IsPossible( const Transition in_xTransition ) const
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
  DirectedSection( const TupleId in_iSectionTid,
                   const bool in_bUpDown):
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

  bool GetUpDownFlag() const
  {
    return m_bUpDown;
  }

  TupleId GetSectionTid() const
  {
    return m_iSectionTid;
  }

  ostream& Print(ostream& os) const
  {
    os << "Directed Section: TupleId: " << (long) m_iSectionTid;
    os << ", UpDownFlag(false = 0 = down): " << m_bUpDown << endl;
    return os;
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
  DirectedSectionPair(const TupleId in_iFirstSectionTid,
                      const bool in_bFirstUpDown,
                      const TupleId in_iSecondSectionTid,
                      const bool in_bSecondUpDown):
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

  ostream& Print(ostream& os) const {
    os << "1.Section: " << m_iFirstSectionTid
       << " Up(!=0): " << m_bFirstUpDown
       << " 2.Section: " << m_iSecondSectionTid
       << " Up(!=0): " << m_bSecondUpDown << endl;
    return os;
  }

  ~DirectedSectionPair(){};

  bool operator<(const DirectedSectionPair& in_xOther) const
  {
    if(m_iFirstSectionTid < in_xOther.m_iFirstSectionTid)
      return true;
    if (m_iFirstSectionTid > in_xOther.m_iFirstSectionTid)
      return false;
    if(m_bFirstUpDown != in_xOther.m_bFirstUpDown)
      return m_bFirstUpDown;
    if (m_iSecondSectionTid < in_xOther.m_iSecondSectionTid)
      return true;
    if (m_iSecondSectionTid > in_xOther.m_iSecondSectionTid)
      return false;
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
  AdjacencyListEntry( const int in_iLow,
                      const int in_iHigh ):
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

  JunctionSortEntry(const bool in_bFirstRoute,
                    Tuple* in_pJunction):
    m_bFirstRoute(in_bFirstRoute),
    m_pJunction(in_pJunction)
  {
  }

  ~JunctionSortEntry(){}
  bool m_bFirstRoute;

  Tuple* m_pJunction;

  bool operator<(const JunctionSortEntry& in_xOther) const
  {
    return GetRouteMeas() < in_xOther.GetRouteMeas();
    /*
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

    return (xMeas1->GetRealval() < xMeas2->GetRealval());*/
  }

  double GetRouteMeas() const
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

  double GetOtherRouteMeas() const
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

  int GetOtherRouteId() const {
    CcInt* pRouteId;
    if (m_bFirstRoute) {
      pRouteId = (CcInt*)m_pJunction->GetAttribute(JUNCTION_ROUTE2_ID);
    } else {
      pRouteId = (CcInt*)m_pJunction->GetAttribute(JUNCTION_ROUTE1_ID);
    }
    return pRouteId->GetIntval();
  }


  TupleId GetUpSectionId() const
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

  TupleId GetDownSectionId() const
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

  ostream& Print(ostream& os) const
  {
    os << "JunctionSortEntry First Route : " << m_bFirstRoute;
    os << "Tuple: ";
    m_pJunction->Print(os);
    os << endl;
    return os;
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
    void Load(int in_iId, double scalef,
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
    int GetId() const;

double GetScalefactor() const
{
  return m_scalefactor;
}

/*
GetRoutes

Returns a copy of the ~routes~ relation in external representation.
This function is used in the ~routes~ operator.

*/
    Relation *GetRoutes() const;

/*
GetJunctions

Returns a copy of the ~junctions~ relation in external representation.
This function is used in the ~junctions~ operator.

*/
    Relation *GetJunctions() const;

/*
GetJunctionsOnRoute

Returns the junction from the start of the route to the end.

*/
    void GetJunctionsOnRoute(CcInt* in_pRouteId,
                             vector<JunctionSortEntry>& inout_xJunctions) const;

/*
Returns the section ~tuple~ of the network which includes the ~GPoint~.
Attention: The tupleId of the returned Tuple is not the TupleId in the sections
relation! If you need the correct tupleId of the sections relation use
~GetTupleIdSectionOnRoute~ instead.

*/

 Tuple*  GetSectionOnRoute(const GPoint* in_xGPoint) const;

/*
Returns the ~tupleid~  of the section which includes the ~GPoint~ .

*/


 TupleId GetTupleIdSectionOnRoute(const GPoint* in_xGPoint) const;

/*
GetPointOnRoute

Returns the point value of the GPoint on the route. Used for translation of
network positions into spatial 2D positions.

*/
  void GetPointOnRoute(const GPoint* in_xGPoint, Point *&res) const;

/*
GetRoute

Returns the route tuple for the given route id.

*/

  Tuple* GetRoute(const int in_RouteId) const;
  Tuple* GetRoute(const TupleId in_routeTID) const;

/*
GetRouteCurve

Returns the route curve for the given route id.

*/

  SimpleLine GetRouteCurve(const int in_iRouteId) const;

/*
GetDual

Returns the dual value of the given route id.

*/

  bool GetDual(const int in_iRouteId) const;

/*
GetLineValueOfRouteInterval

Returns the ~sline~ representing the ~RouteInterval~ in spatial data. Used
to translate network values into spatial 2D values.

*/

  void GetLineValueOfRouteInterval(const RouteInterval* in_rI,
                                   SimpleLine *out_line) const;

/*
GetSections

Returns a copy of the ~sections~ relation in external representation.
This function is used in the ~sections~ operator.

*/
  Relation *GetSections() const;

/*
GetNetworkPosOfPoint

Returns a GPoint representing the point value in the network if possible, undef
elsewhere. Used to translate spatial 2D positions into network positions.

*/

  GPoint* GetNetworkPosOfPoint(const Point p) const;
  GPoint* GetNetworkPosOfPointOnRoute(const Point p, const int rid) const;

/*
GetSectionsInternal

Returns the ~sections~ relation (in internal representation).

*/
    Relation *GetSectionsInternal() const;

/*
Method GetAdjacentSection
Returns a vector of sections which can be reached next upwards respectively
downwards bool from the section given by TupleId.

*/

    void GetAdjacentSections(const TupleId in_iSectionId,
                             const bool in_bUpDown,
                             vector<DirectedSection> &inout_xSections) const;

    void GetAdjacentSections(const int sectId, const bool upDown,
                             DbArray<SectionValue> *resArray) const;

    void GetReverseAdjacentSections(const TupleId in_iSectionId,
                                    const bool in_bUpDown,
                                    vector<DirectedSection> &inout_xSections)
      const;

    void GetReverseAdjacentSections(const int sectId,
                                    const bool upDown,
                                    DbArray<SectionValue> *resArray)
      const;

/*
GetSections on RouteInterval.

Returns a set of sections which are covered by the given ~RouteInterval~
~inri~ as ~DbArray~ of SectTreeEntries.

*/

    void GetSectionsOfRouteInterval(const RouteInterval *in_ri,
                                    vector<SectTreeEntry>& io_SectionIds) const;

    void GetSectionsOfRoutInterval(const RouteInterval *in_ri,
                                  vector<TupleId> &res) const;

/*
Get Routeinterval for Halfsegment defined by point interval. Used to translate
spatial 2D values into network values.

*/

    RouteInterval* Find(const Point p1, const Point p2) const;

/*
Get Routepart passed limited by the two points.

*/

    RouteInterval* FindInterval(const Point p1, const Point p2) const;


/*
Computes the junction of the two given routes and returns the route measure
values of the junction on this routes.

*/

void GetJunctionMeasForRoutes( CcInt *pLastRouteId,
                               CcInt *pCurrentSectionRid,
                              double& rid1meas, double& rid2meas) const;

/*
Returns the section with the given id.

*/
    Tuple* GetSection(const TupleId n) const ;

    void FindSP(const TupleId j1, const TupleId j2,
                double& length, GLine* res) const;
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

 int IsDefined() const;

 void SetDefined(const bool def)
 {
   m_bDefined = def;
 };


  void GetTupleIdSectionOnRouteJun(const GPoint* in_xGPoint,
                                   vector<TupleId>&) const;
  inline int GetNoJunctions() const
  {
    return m_pJunctions->GetNoTuples();
  }

  inline int GetNoRoutes() const
  {
    return m_pRoutes->GetNoTuples();
  }

  inline int GetNoSections() const
  {
    return m_pSections->GetNoTuples();
  }

  R_Tree<2,TupleId>* GetRTree(){ return m_pRTreeRoutes;}

  inline static const string BasicType() { return "network"; }

  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }

  inline Rectangle<2> BoundingBox() const{
    return m_pRTreeRoutes->BoundingBox();
  }

private:


/*
4.2.9.5 Private methods of class ~Network~

FillRoutes

Copies the relation given as argument to the ~routes~ relations sorted by
the ~id~ attribute and creates the B-Tree in this attribute.

*/
  void FillRoutes( const Relation *in_pRoutes );

  void CreateRoutesIndexes();

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

  void FillAdjacencyPair(const TupleId in_pFirstSection,
                         const bool in_bFirstUp,
                         const TupleId in_pSecondSection,
                         const bool in_bSecondUp,
                         const ConnectivityCode in_xCc,
                         const Transition in_xTransition,
                         vector<DirectedSectionPair> &inout_xPairs);

  void FillReverseAdjacencyPair (const TupleId in_pFirstSection,
                                 const bool in_bFirstUp,
                                 const TupleId in_pSecondSection,
                                 const bool in_bSecondUp,
                                 const ConnectivityCode in_xCc,
                                 const Transition in_xTransition,
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
                   int &ridt, Point p1, Point p2 ) const;

  bool ShorterConnection2(Tuple *route, double &start,
                   double &end, double &dpos, double &dpos2, int &rid,
                   int &ridt, Point p1, Point p2 ) const ;

  bool InShortestPath(GPoint* start, GPoint* end, GLine *result) const;


/*
4.2.9.6 Private fields of Class ~Network~

The ID of the network

*/
  int m_iId;

/*
MapMatching needs information about dimension of data values.
The scalefactor 1.0 tells, position of meter is right in front of the decimal
point. 0.001 tells the position of meter  in the coordinates is at the third
position behind the decimal point.

*/

  double m_scalefactor;

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

  DbArray<AdjacencyListEntry> m_xAdjacencyList;

  DbArray<DirectedSection> m_xSubAdjacencyList;

/*
The reverse adjacency lists of sections.

*/

  DbArray<AdjacencyListEntry> m_reverseAdjacencyList;
  DbArray<DirectedSection> m_reverseSubAdjancencyList;

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


class GLine : public Attribute
{
/*
5.1 Constructors and Destructor

The simple constructor. Should not be used.

*/
  public:
    GLine();

/*
~iniSize~ gives the number of expected ~RouteIntervals~. If it is to small
the size of the ~DbArray~ is dynamically extended later.

*/
    GLine(const int in_iSize);

    GLine(const GLine* in_xOther);

    GLine(const bool def);

    GLine(const int networkid, const bool sorted,
          const DbArray<RouteInterval>* routeIntervals);

    ~GLine() {};

    GLine( ListExpr in_xValue,
         int in_iErrorPos,
         ListExpr& inout_xErrorInfo,
         bool& inout_bCorrect);

/*
5.2 Methods of class ~gline~

*/

    ostream& Print( ostream& os ) const;

    void SetNetworkId(const int in_iNetworkId);

    void AddRouteInterval(const int in_iRouteId,
                          const double in_dStart,
                          const double in_dEnd);

    void AddRouteInterval(const RouteInterval ri);

    double GetLength () const;
    void SetLength(const double l);

    int GetNetworkId() const;

    void Get(const int i, RouteInterval &ri) const;

    int NoOfComponents() const;

    bool IsSorted() const;

    void SetSorted(const bool b);

    DbArray<RouteInterval>* GetRouteIntervals() const ;

/*
Returns true if the gline includes at least one of the gpoints.

*/
    bool Includes(const GPoints* gps) const;
    bool Includes(const GPoints* gps, const Network* pNetwork) const;


/*
Computes the network distance of 2 glines.

*/

    double Netdistance(const GLine* pgl2) const;
    double Netdistance (const GLine* pgl2, const Network* pNetwork )const ;

    double NetdistanceNew(const GLine* pgl2)const ;
    double NetdistanceNew(const GLine* pgl2, const Network* pNetwork)const ;

    double NetdistanceNew(const GPoint* pgl2)const ;
    double NetdistanceNew(const GPoint* pgl2, const Network* pNetwork)const ;

    double NetdistanceNew(const GPoints* pgl2) const;
    double NetdistanceNew(const GPoints* pgl2, const Network* pNetwork)const ;


    bool ShortestPathAStar(const GLine *to, GLine *result,
                      DbArray<TupleId>* touchedSects = 0)const ;
    bool ShortestPathAStar(const GLine *to, GLine *result,
                           const Network* pNetwork,
                      DbArray<TupleId>* touchedSects = 0)const ;

    bool ShortestPathAStar(const GPoint *to, GLine *result,
                      DbArray<TupleId>* touchedSects = 0)const ;
    bool ShortestPathAStar(const GPoint *to, GLine *result,
                           const Network* pNetwork,
                      DbArray<TupleId>* touchedSects = 0)const ;

    bool ShortestPathAStar(const GPoints *to, GLine *result,
                      DbArray<TupleId>* touchedSects = 0)const ;
    bool ShortestPathAStar(const GPoints *to, GLine *result,
                           const Network* pNetwork,
                      DbArray<TupleId>* touchedSects = 0)const ;

    bool ShortestPathBF(const GLine *pgl2, GLine *result,
                        DbArray<TupleId>* touchedSects)const ;
    bool ShortestPathBF(const GLine *pgl2, GLine *result,
                        const Network *pNetwork,
                        DbArray<TupleId>* touchedSects)const ;
/*
Computes the euclidean distance of 2 glines.

*/

    double Distance (const GLine* pgl2)const ;

/*
Computes the union of 2 glines.

*/


void Uniongl (const GLine* pgl2, GLine *res) const ;

/*
Translates a gline value into a line value.

*/

    void Gline2line (Line *res)const ;


/*
Returns true if the glines intersect false elswhere.

*/


    bool Intersects (const GLine* pgl) const ;


/*
Returns the Bounding GPoints of the GLine.

*/

    void GetBGP(const Network* pNetwork, GPoints* result)const ;
    void GetBGP(GPoints* result)const ;

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

    bool Contains(const RouteInterval* ri, const double tolerance)const ;
    bool Intersects(const RouteInterval* ri, const double tolerance)const ;

  private:

/*
5.4 Private Fields of class ~gline~

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
1.2.3 ~struct RITree~

Used to compress and sort ~RouteInterval~s. Because many operators take profit
from sorted ~RouteInterval~s. We sort and compress resulting ~DbArray~s of
~RouteInterval~s whenever it is possible.

*/

struct RITree {

  RITree(){};

  RITree( const int ri, const double pos1, const double pos2
        , RITree *left = 0, RITree *right = 0){
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

  double CheckTree(RITree& father, int rid, double pos1,
                   double pos2, bool bleft) {
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
  void InsertUnit(const int rid, const double pos1, const double pos2)
  {
    if (pos1 < pos2) Insert(rid, pos1, pos2);
    else Insert(rid, pos2, pos1);
  }

  void Insert (const int rid, const double pos1, const double pos2) {
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

  void TreeToGLine (GLine* gline) const {
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
start positions into a ~DbArray~. Used to build trajectory of ~mgpoint~.

*/

  void TreeToDbArray (DbArray<RouteInterval>* arr) const {
    if (this->m_left != 0) {
      this->m_left->TreeToDbArray (arr);
    }
    arr->Append(RouteInterval(this->m_iRouteId, this->m_dStart, this->m_dEnd));
    if (this->m_right != 0) {
      this->m_right->TreeToDbArray (arr);
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
struct RouteIntervalEntry

EntryType for RITreeP

*/

struct RouteIntervalEntry
{
  RouteIntervalEntry(){};

  RouteIntervalEntry(const RouteIntervalEntry& rie)
  {
    ri = rie.ri;
    left = rie.left;
    right = rie.right;
  }

  RouteIntervalEntry(const RouteInterval& rin, int l = -1, int r = -1)
  {
    ri = rin;
    left = l;
    right = r;
  }

  RouteIntervalEntry( const int rid, const double pos1, const double pos2,
                      const int l = -1, const int r = -1)
  {
    ri = RouteInterval(rid, pos1, pos2);
    left = l;
    right = r;
  };

  ~RouteIntervalEntry(){};

  ostream& Print(ostream& os) const
  {
    os << "RouteInterval:";
    ri.Print(os);
    os << ", left: " << left << ", right: " << right << endl;
    return os;
  }

  void operator=(const RouteIntervalEntry& rie)
  {
    ri = rie.ri;
    left = rie.left;
    right = rie.right;
  }

  inline RouteInterval GetEntry() const
  {
    return ri;
  }

  inline int GetLeft() const
  {
    return left;
  }

  inline int GetRight() const
  {
    return right;
  }

  inline void SetLeft(const int l)
  {
    left = l;
  }

  inline void SetRight(const int r)
  {
    right = r;
  }

  inline void SetEntry(const int select, const double value)
  {
    switch(select)
    {
      case(0):
        ri.SetRouteId((int) value);
        break;

      case(1):
        ri.SetStartPos(value);
        break;

      case(2):
        ri.SetEndPos(value);
        break;

      default:
        break;
    }
  }

  RouteInterval ri;
  int left, right;
};

/*
struct RITreeP

Almost the same like RITree but with memory independent data structure.auto

*/

struct RITreeP
{
  RITreeP():ritreep(0)
  {
    firstFree = 0;
  }

  RITreeP(const int n):ritreep(n)
  {
    firstFree = 0;
  }

  ~RITreeP(){};

  void Destroy()
  {
    ritreep.Destroy();
  };

  bool IsEmpty() const
  {
    return (firstFree == 0);
  }

  ostream& Print(ostream& os) const
  {
    RouteIntervalEntry rie;
    for (int i = 0; i < firstFree; i++)
    {
      ritreep.Get(i,rie);
      os << i << ".Entry: ";
      rie.Print(os);
      os << endl;
    }
    return os;
  }
/*
Checks if in the ~RITree~ exist son intervals which are covered by the
previos changed interval of the father.

*/

 double CheckTree(RouteIntervalEntry& father, const int posfather,
                  RouteIntervalEntry& testRI, const int postest,
                  const bool bleft)
 {
    assert(postest > -1 && postest < firstFree &&
           posfather > -1 && posfather < firstFree);
    RouteIntervalEntry test;
    ritreep.Get(postest,test);
    if (testRI.GetEntry().GetRouteId() < test.GetEntry().GetRouteId())
    {
      if (test.GetLeft() > -1)
        return CheckTree(test, postest, testRI, test.GetLeft(), bleft);
      else
        return testRI.GetEntry().GetValue(bleft);
    }
    else
    {
      if (testRI.GetEntry().GetRouteId() > test.GetEntry().GetRouteId())
      {
        if (test.GetRight() > -1)
          return CheckTree(test, postest, testRI, test.GetRight(), bleft);
        else
          return testRI.GetEntry().GetValue(bleft);
      }
      else
      {
        if (testRI.GetEntry().GetEndPos() < test.GetEntry().GetStartPos())
        {
          if (test.GetLeft() > -1)
            return CheckTree(test,  postest, testRI, test.GetLeft(), bleft);
          else
            return testRI.GetEntry().GetValue(bleft);
        }
        else
        {
          if (testRI.GetEntry().GetStartPos() > test.GetEntry().GetEndPos())
          {
            if (test.GetRight() > -1 )
              return CheckTree(test, postest, testRI, test.GetRight(), bleft);
            else
              return testRI.GetEntry().GetValue(bleft);
          }
          else
          {
            // Overlapping interval found. Rebuild Tree and return new interval
            // limit.
            if (bleft)
            {
              if (test.GetEntry().GetStartPos() <
                    testRI.GetEntry().GetStartPos())
                testRI.SetEntry(1,test.GetEntry().GetStartPos());
              if (father.GetLeft() == postest)
              {
                father.SetLeft(test.GetLeft());
                ritreep.Put(posfather,father);
                test.SetLeft(-1);
                ritreep.Put(postest,test);
              }
              else
              {
                father.SetRight(test.GetLeft());
                ritreep.Put(posfather,father);
                test.SetLeft(-1);
                ritreep.Put(postest,test);
              }
              if (father.GetLeft() > -1)
                return CheckTree(father, posfather, testRI, father.GetLeft(),
                                 bleft);
              else
                return testRI.GetEntry().GetStartPos();
            }
            else
            {
              if (test.GetEntry().GetEndPos() > testRI.GetEntry().GetEndPos())
                testRI.SetEntry(2, test.GetEntry().GetEndPos());
              if (father.GetLeft() == postest)
              {
                father.SetLeft(test.GetRight());
                ritreep.Put(posfather,father);
                test.SetRight(-1);
                ritreep.Put(postest,test);
              }
              else
              {
                father.SetRight(test.GetRight());
                ritreep.Put(posfather,father);
                test.SetRight(-1);
                ritreep.Put(postest,test);
              }
              if (father.GetRight() > -1 )
                return CheckTree(father, posfather, testRI, father.GetRight(),
                                 bleft);
              else
                return testRI.GetEntry().GetEndPos();
            }
          }
        }
      }
    }
    return testRI.GetEntry().GetValue(bleft);
  };


/*
Inserts a ~RouteInterval~ in the ~RITree~ checking if there are already
~RouteIntervals~ which overlap or are adjacent to the current ~RouteInterval~.

*/
  void InsertUnit(const int rid, const double pos1, const double pos2)
  {
    if (pos1 < pos2) Insert(rid, pos1, pos2, 0);
    else Insert(rid, pos2, pos1, 0);
  }

  void Insert (const bool left, const int pos, RouteIntervalEntry& testRI,
               const int r, const double p1, const double p2)
  {
    if (left) testRI.SetLeft(firstFree);
    else testRI.SetRight(firstFree);
    ritreep.Put(pos,testRI);
    ritreep.Put(firstFree, RouteIntervalEntry(RouteInterval(r, p1, p2),
                                             -1,-1));
    firstFree++;
  }

  void Insert(const RouteInterval ri)
  {
    Insert(ri.GetRouteId(), ri.GetStartPos(), ri.GetEndPos());
  }

  void Insert (const int rid, const double pos1, const double pos2, int pos = 0)
  {
    if (IsEmpty())
    {
      ritreep.Put(firstFree, RouteIntervalEntry(RouteInterval(rid,pos1,pos2),
                                             -1,-1));
      firstFree++;
    }
    else
    {
      RouteIntervalEntry testRI;
      ritreep.Get(pos, testRI);
      double test;
      if (rid < testRI.GetEntry().GetRouteId())
      {
        if (testRI.GetLeft() > -1)
          Insert(rid, pos1, pos2, testRI.GetLeft());
        else
          Insert(true, pos, testRI, rid, pos1, pos2);
      }
      else
      {
        if (rid > testRI.GetEntry().GetRouteId())
        {
          if (testRI.GetRight() > -1)
            Insert(rid, pos1, pos2, testRI.GetRight());
          else
            Insert(false, pos, testRI, rid, pos1, pos2);
        }
        else
        {
          if (pos2 < testRI.GetEntry().GetStartPos())
          {
            if (testRI.GetLeft() > -1 )
              Insert(rid, pos1, pos2, testRI.GetLeft());
            else
              Insert(true, pos, testRI, rid, pos1, pos2);
          }
          else
          {
            if (pos1 > testRI.GetEntry().GetEndPos())
            {
              if (testRI.GetRight() > -1)
                Insert(rid, pos1, pos2, testRI.GetRight());
              else
              {
                Insert(false, pos, testRI, rid, pos1, pos2);
              }
            }
            else
            {
              // Overlapping route intervals merge and check sons if they need
              // to be corrected too.
              if (testRI.GetEntry().GetStartPos() > pos1)
              {
                testRI.SetEntry(1, pos1);
                ritreep.Put(pos,testRI);
                if (testRI.GetLeft() > -1)
                {
                  test = CheckTree(testRI, pos, testRI, testRI.GetLeft(),true);
                  if (testRI.GetEntry().GetStartPos() > test)
                  {
                    testRI.SetEntry(1, test);
                    ritreep.Put(pos,testRI);
                  }
                }
              }
              if (testRI.GetEntry().GetEndPos() < pos2)
              {
                testRI.SetEntry(2,pos2);
                ritreep.Put(pos,testRI);
                if (testRI.GetRight() > -1)
                {
                  test =
                    CheckTree(testRI, pos, testRI, testRI.GetRight(),false);
                  if (testRI.GetEntry().GetEndPos() < test)
                  {
                    testRI.SetEntry(2, test);
                    ritreep.Put(pos,testRI);
                  }
                }
              }
            }
          } // endif rid=rid
        }
      }
    }
  };

/*
Stores the ~RouteInterval~s of the ~RITree~ sorted by their route ids and
start positions into ~gline~.

*/

  void TreeToGLine (GLine* gline, int pos) const
  {
    assert(pos >= 0 && pos < firstFree);
    RouteIntervalEntry test;
    ritreep.Get(pos, test);
    if (test.GetLeft() > -1)
      TreeToGLine(gline, test.GetLeft());
    gline->AddRouteInterval(test.GetEntry());
    if (test.GetRight() > -1)
      TreeToGLine (gline, test.GetRight());
  };

/*
Stores the ~RouteInterval~s of the ~RITree~ sorted by their route ids and
start positions into a ~DbArray~. Used to build trajectory of ~mgpoint~.

*/

  void TreeToDbArray (DbArray<RouteInterval>* arr, int pos) const
  {
    assert(pos > -1 && pos < firstFree);
    RouteIntervalEntry test;
    ritreep.Get(pos,test);
    if (test.GetLeft() > -1)
      TreeToDbArray(arr, test.GetLeft());
    arr->Append(test.GetEntry());
    if (test.GetRight() > -1)
      TreeToDbArray (arr, test.GetRight());
  };

  DbArray<RouteIntervalEntry> ritreep;
  int firstFree;
};

class GPointsSections
{
  public:
    GPointsSections(){};

    GPointsSections(const GPoint g, const TupleId t, const Point p)
    : m_gp(g), m_tid(t), m_p(p)
    {
    };

    ~GPointsSections(){};

    GPoint GetGP() const {return m_gp;};

    TupleId GetTid() const {return m_tid;};

    Point GetPoint() const {return m_p;};

    ostream& Print(ostream& os) const
    {
      os << "GPoint: ";
      m_gp.Print(os);
      os << ", Point: ";
      m_p.Print(os);
      os << ", TupleId: " << (long int) m_tid << endl;
      return os;
    };

  private:
    GPoint m_gp;
    TupleId m_tid;
    Point m_p;
};
/*
6. class GPoints by Jianqiu Xu.

*/

extern string edistjoinpointlist;

class GPoints: public Attribute{
public:
  GPoints();
  GPoints(const int in_iSize);
  GPoints(const bool defined);
  GPoints(const GPoints* in_xOther);
  ~GPoints(){}
  ostream& Print(ostream& os)const;
  inline bool IsEmpty()const;
  size_t Sizeof()const;
  GPoints& operator+=(const GPoint &gp);
  GPoints& operator-=(const GPoint &gp);
  GPoints& operator=(const GPoints& otherGPoints);
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
  GPoints* Clone() const;
  size_t HashValue()const;
  void CopyFrom(const Attribute* right);
  void FilterAliasGPoints(Network *pNetwork);
  void TrimToSize () {m_xGPoints.TrimToSize();}
  void MergeAdd(const GPoint gp, const Network* pNetwork);
  bool Contains(const GPoint gp, const Network* pNetwork)const;
  bool Contains(const GPoint gp)const;
  double NetdistanceNew(const GPoints* bgp, const Network* pNetwork)const ;
  double NetdistanceNew(const GPoints* bgp)const ;
  double NetdistanceNew(const GPoint* gp, const Network* pNetwork)const ;
  double NetdistanceNew(const GPoint* gp) const ;
  double NetdistanceNew(const GLine* gl, const Network* pNetwork)const ;
  double NetdistanceNew(const GLine* gl)const ;
  int GetNetworkId()const ;
  bool ShortestPathAStar(const GPoints* bgp, GLine* res,
                    DbArray<TupleId>* touchedSects = 0)const ;
  bool ShortestPathAStar(const GPoints* bgp, GLine* res,
                         const Network* pNetwork,
                    DbArray<TupleId>* touchedSects = 0)const ;
  bool ShortestPathAStar(const GPoint* gp, GLine* res,
                    DbArray<TupleId>* touchedSects = 0)const ;
  bool ShortestPathAStar(const GPoint* gp, GLine* res,
                         const Network* pNetwork,
                    DbArray<TupleId>* touchedSects = 0)const ;
  bool ShortestPathAStar(const GLine* gl, GLine* res,
                    DbArray<TupleId>* touchedSects = 0)const ;
  bool ShortestPathAStar(const GLine* gl, GLine* res,
                         const Network* pNetwork,
                    DbArray<TupleId>* touchedSects = 0)const ;
  bool Inside(const GPoint gp)const ;
  bool Intersects(const GPoints* bgp, const Network* pNetwork)const;
  bool Intersects(const GPoints* bgp)const ;
  void Clear();

private:
  void GetSectionTupleIds(DbArray<GPointsSections>* gpSections,
                         const Network* pNetwork)const ;

  DbArray<GPoint> m_xGPoints;
};



#endif // __NETWORK_ALGEBRA_H__
