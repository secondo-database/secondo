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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

1 Declarations needed by Algebra TemporalNet

Mai-Oktober 2007 Martin Scheppokat

February 2008 -  Simone Jandt

1.2 Defines, includes, and constants

*/

#ifndef _TEMPORAL_NET_ALGEBRA_H_
#define _TEMPORAL_NET_ALGEBRA_H_

#include <iostream>
#include <sstream>
#include <string>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "../Spatial/Geoid.h"
#include "../Network/NetworkAlgebra.h"

#ifndef __NETWORK_ALGEBRA_H__
#error NetworkAlgebra.h is needed by TemporalNetAlgebra.h. \
Please include in *.cpp-File.
#endif

#ifndef _TEMPORAL_ALGEBRA_H_
#error TemporalAlgebra.h is needed by MGPoint.h. \
Please include in *.cpp-File.
#endif



using namespace datetime;
using namespace network;

namespace temporalnet{
/*
IGPointt

This class represents an intime(GPoint) .

*/
typedef Intime<GPoint> IGPoint;

/*
2 class MGPSecUnit

*/

class MGPSecUnit : public Attribute
{
  public:

/*
2.1 Constructors and Destructor

*/

    MGPSecUnit();

    MGPSecUnit(const bool defined, const int secId, const int part,
               const Side direct, const double sp,
               const Interval<Instant> timeInterval);

    MGPSecUnit( const MGPSecUnit& in_xOther );

    ~MGPSecUnit();

/*
2.2 Methods of class ~mgpsecunit~

Get and Set private attributes.

*/

    int GetSecId() const;

    int GetPart()const;

    Side GetDirect() const;

    double GetSpeed()const;

    Interval<Instant> GetTimeInterval() const;

    double GetDurationInSeconds() const;

    void SetSecId(const int secId);

    void SetPart(const int p);

    void SetDirect(const Side dir);

    void SetSpeed(const double x);

    void SetTimeInterval(const Interval<Instant> time);

    MGPSecUnit& operator=( const MGPSecUnit& in_xOther);


/*
Functions for Secondo integration.

*/

    size_t Sizeof() const;

    size_t HashValue() const;

    void CopyFrom( const Attribute* right );

    int Compare( const Attribute* arg ) const;

    bool operator<(const MGPSecUnit arg) const;
    bool operator>(const MGPSecUnit arg) const;
    bool operator==(const MGPSecUnit arg)const;
    bool operator!=(const MGPSecUnit arg)const;
    bool operator<=(const MGPSecUnit arg)const;
    bool operator>=(const MGPSecUnit arg)const;

    bool Adjacent( const Attribute *arg ) const;

    MGPSecUnit *Clone() const;

    ostream& Print( ostream& os ) const;

    static ListExpr Out(ListExpr typeInfo, Word value);

    static Word In(const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr& errorInfo, bool& correct);

    static bool CheckKind( ListExpr type, ListExpr& errorInfo );

    int NumOfFLOBs() const;

    Flob* GetFLOB(const int i) const;

    static void* Cast(void* addr);

    inline static const string BasicType() { return "mgpsecunit"; }

    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }



/*
Function for Operations.

*/
  private:

    int m_secId; //section id
    int m_part;  //number of part of section if necessary default = 1
    Side m_direct; // 0=down, 1=up, 2=none
    double m_speed; // m/s
    Interval<Instant> m_time;

};

/*
2 UGPoint

This class will be used in the ~ugpoint~ type constructor, i.e., the type
constructor for the temporal unit of gpoint values. It inherits from the
SpatialTemporalUnit class.

*/

class UGPoint : public SpatialTemporalUnit<GPoint, 3>
{
  public:
  UGPoint():SpatialTemporalUnit<GPoint,3>() {};

  UGPoint(const bool is_defined):
    SpatialTemporalUnit<GPoint, 3>(is_defined)
    {  };

  UGPoint( const Interval<Instant>& interval,
           const int in_NetworkID,
           const int in_RouteID,
           const Side in_Side,
           const double in_Position0,
           const double in_Position1 ):
    SpatialTemporalUnit<GPoint, 3>( interval ),
    p0( true,        // defined
        in_NetworkID,    // NetworkID
        in_RouteID,      // RouteID
        in_Position0,    // d
        in_Side),      // Side
    p1( true,        // defined
        in_NetworkID,    // NetworkID
        in_RouteID,      // RouteID
        in_Position1,    // d
        in_Side)       // Side
    {};

  UGPoint( const Interval<Instant>& interval,
           const GPoint& ip0,
           const GPoint& ip1 ):
    SpatialTemporalUnit<GPoint, 3>( interval ),
    p0( ip0 ),
    p1( ip1 )
    {
      if (p0.GetPosition() < p1.GetPosition())
      {
        p0.SetSide(Up);
        p1.SetSide(Up);
      }
      else
      {
        if (p0.GetPosition() > p1.GetPosition())
        {
          p0.SetSide(Down);
          p1.SetSide(Down);
        }
        else
        {
          p0.SetSide(None);
          p1.SetSide(None);
        }
      }
    }

    UGPoint( const Interval<Instant>& interval,
           const int in_NetworkID,
           const int in_RouteID,
           const Side in_Side,
           const double in_Position0,
           const double in_Position1,
           const Network *pNetwork):
    SpatialTemporalUnit<GPoint, 3>( interval ),
    p0( true,        // defined
        in_NetworkID,    // NetworkID
        in_RouteID,      // RouteID
        in_Position0,    // d
        in_Side),      // Side
    p1( true,        // defined
        in_NetworkID,    // NetworkID
        in_RouteID,      // RouteID
        in_Position1,    // d
        in_Side)       // Side
    { }

  UGPoint( const Interval<Instant>& interval,
           const GPoint& p0,
           const GPoint& p1,
           const Network *pNetwork):
    SpatialTemporalUnit<GPoint, 3>( interval ),
    p0( p0 ),
    p1( p1 )
    {  }

  UGPoint(const UGPoint &source):
        SpatialTemporalUnit<GPoint,3>(source.IsDefined())
  {
    *((TemporalUnit<GPoint>*)this)=*((TemporalUnit<GPoint>*)&source);
    p0=source.p0;
    p1=source.p1;
    SetDefined(source.IsDefined());
  }

  ostream& Print( ostream &os ) const
  {
    timeInterval.Print(os);
    p0.Print(os);
    p1.Print(os);
    return os;
  }
/*
Operator redefinitions

Redefinition of the copy operator ~=~.

*/

  virtual UGPoint& operator=( const UGPoint& i )
  {
    *((TemporalUnit<GPoint>*)this) = *((TemporalUnit<GPoint>*)&i);
    p0 = i.p0;
    p1 = i.p1;
    SetDefined(i.IsDefined());
    return *this;
  }

/*
Returns ~true~ if this temporal unit is equal to the temporal unit ~i~ and
~false~ if they are different.

*/

   virtual bool operator==( const UGPoint& i ) const
  {
    return *((TemporalUnit<GPoint>*)this) == *((TemporalUnit<GPoint>*)&i) &&
            (p0 == i.p0 ) && ( p1 == i.p1 );
  }

/*
Returns ~true~ if this temporal unit is different to the temporal unit ~i~
and ~false~ if they are equal.

*/

    virtual bool operator!=( const UGPoint& i ) const
  {
    return !( *this == i );
  }

/*
Functions to be part of relations

*/

  inline virtual size_t Sizeof() const
  {
    return sizeof( *this );
  }

  inline virtual UGPoint* Clone() const
  {
    return new UGPoint(*this);
  }

  inline virtual void CopyFrom( const Attribute* right )
  {
    const UGPoint* i = (const UGPoint*)right;

    if(i->IsDefined())
      {
        timeInterval.CopyFrom( i->timeInterval );
        p0 = i->p0;
        p1 = i->p1;
      }
    else
      {
        timeInterval = Interval<Instant>();
        p0 = GPoint( false, 0, 0, 0.0, None);
        p1 = GPoint( false, 0, 0, 0.0, None);
      }
  }

/*
Returns the 3 dimensional spatio-temporal BoundingBox of the ~ugpoint~.

*/

 virtual const Rectangle<3> BoundingBox(const Geoid* geoid = 0) const;

 Rectangle<3> BoundingBox(const Network* pNetwork, const Geoid* geoid=0) const;

 virtual double Distance(const Rectangle<3>& rect,const Geoid* geoid = 0) const;

 virtual bool IsEmpty() const{ return IsDefined(); }

 /*
 Returns the 3 dimensional network bounding box of the ~ugpoint~, which is
 degenerated to be a rectangle. The two x-coordinates are identic and given by
 route id of the ~ugpoint~, the y-coordinates are given by the start and the
 end position of the ugpoint, and the z-coordinates are given by the timestamps
 of the timeinterval of the ~ugpoint~.

 */
  virtual const Rectangle<3> NetBoundingBox3d(const Geoid* geoid = 0) const
  {
    if(geoid){
      cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
           << endl;
      assert( !geoid ); // TODO: implement spherical geometry case
    }
    return Rectangle<3> (true,
                        (double) p0.GetRouteId(),
                        (double) p0.GetRouteId(),
                        min(p0.GetPosition(),p1.GetPosition()),
                        max(p0.GetPosition(),p1.GetPosition()),
                        timeInterval.start.ToDouble(),
                        timeInterval.end.ToDouble());
  }

  /*
  The 2 dimensinal network bounding box of the ~ugpoint~ is degenerated to a
  line. the x- and y-coordinates are defined analogous to the x- and y-coord-
  inates of the ~NetBoundingBox3d~.

  */
  inline const Rectangle<2> NetBoundingBox2d(const Geoid* geoid = 0) const
  {
    if(geoid){
      cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
      << endl;
      assert( !geoid ); // TODO: implement spherical geometry case
    }
    return Rectangle<2> (true,
                           (double) p0.GetRouteId(),
                           (double) p0.GetRouteId(),
                          min(p0.GetPosition(), p1.GetPosition()),
                          max(p0.GetPosition(), p1.GetPosition()));
  }

  /*
  Computes the network position of the ~ugpoint~ at a given time instant ~t~.

  */
  virtual void TemporalFunction( const Instant& t,
                                 GPoint& result,
                                 bool ignoreLimits = false ) const;

  /*
  Returns true if the ~ugpoint~ passes a given ~gpoint~ false elsewhere.

  */
  virtual bool Passes( const GPoint& val )const ;

  /*
  Returns the ~igpoint~ the ~ugpoint~ was at a given network position.

  */
  virtual bool At( const GPoint& val, TemporalUnit<GPoint>& result ) const;

  /*
  Returns the length of the route passed within the ~ugpoint~

  */
  inline double Length() const
  {
    return fabs(p1.GetPosition() - p0.GetPosition());
  };

  /*
  Get Methods of ~ugpoint~

  */

  inline int GetNetworkId() const
  {
    return p0.GetNetworkId();
  };

  inline int GetUnitRid() const
  {
    return p0.GetRouteId();
  };

  inline double GetUnitStartPos() const
  {
    return p0.GetPosition();
  };

  inline double GetUnitEndPos() const
  {
    return p1.GetPosition();
  };

  inline GPoint GetStartPoint()const
  {
    return p0;
  };

  inline GPoint GetEndPoint() const
  {
    return p1;
  };

  inline void SetStartPoint(const GPoint gp)
  {
    p0 = gp;
  };

  inline void SetEndPoint(const GPoint gp)
  {
    p1 = gp;
  };

  inline Side GetUnitSide() const
  {
    return p0.GetSide();
  };

  inline Interval<Instant> GetUnitTimeInterval() const
  {
    return timeInterval;
  };

  inline Instant GetUnitStartTime() const
  {
    return timeInterval.start;
  };

  inline Instant GetUnitEndTime() const
  {
    return timeInterval.end;
  };

  inline bool GetUnitStartTimeBool() const
  {
    return timeInterval.lc;
  };

  inline bool GetUnitEndTimeBool() const
  {
    return timeInterval.rc;
  };

  inline double GetDoubleUnitStartTime() const
  {
    return timeInterval.start.ToDouble();
  };

  inline double GetDoubleUnitEndTime() const
  {
    return timeInterval.end.ToDouble();
  };

  inline double Duration() const //miliseconds
  {
    return (timeInterval.end.ToDouble() - timeInterval.start.ToDouble());
  }

  inline double DurationSeconds() const//seconds
  {
    return (Duration()/0.00001157);
  }

  inline double Speed() const// in m/s
  {
    return (Length() / DurationSeconds());
  }

  inline Side MovingDirection() const
  {
    if (p0.GetPosition() < p1.GetPosition()) return Up;
    else
      if (p0.GetPosition() > p1.GetPosition()) return Down;
      else return None;
  }

  void GetPassedSections(const Network* pNet, vector<TupleId>& pS) const;

  void GetMGPSecUnits(vector<MGPSecUnit>& res, const double maxSectLength,
                     const Network *pNet) const;

/*
SetMethoden f[ue]r ~ugpoint~

*/

  inline void SetUnitRid(const int rid)
  {
    p0.SetRouteId(rid);
    p1.SetRouteId(rid);
  };

  inline void SetUnitStartPos(const double pos)
  {
    p0.SetPosition(pos);
  };

  inline void SetUnitEndPos(const double pos)
  {
    p1.SetPosition(pos);
  };

  inline void SetUnitSide(const Side a)
  {
    p0.SetSide(a);
    p1.SetSide(a);
  };

  inline void SetUnitStartTime(const Instant time)
  {
    timeInterval.start = time;
  };

  inline void SetUnitEndTime(const Instant time)
  {
    timeInterval.end = time;
  };

  inline void SetUnitStartTimeBool(const bool b)
  {
    timeInterval.lc = b;
  };

  inline void SetUnitEndTimeBool(const bool b)
  {
    timeInterval.rc = b;
  };

  /*
  Returns the euclidean Distance of two ~ugpoint~ as ~ureal~

  */

  void Distance (const UGPoint &ugp, UReal &ur) const;

  /*
  Returns the deftime of the ~ugpoint~.

  */

  void Deftime(Periods &per) const;

  /*
  Returns the time instant the ~ugpoint~ was at a given position.

  */

  Instant TimeAtPos(const double pos) const;

/*
  Returns the network distance from the ~gpoint~ to the ~ugpoint~

*/

  void NetdistanceFromArg(const GPoint* gp, UReal* result) const;

/*
Returns the network distance from the ~ugpoint~ to the ~gpoint~

*/

  void NetdistanceToArg(const GPoint* gp, UReal* result) const;


/*
Returns the network distance from the ~ugpoint~ to the ~gpoint~

*/

  void Netdistance(const UGPoint* ugp, UReal* result) const;

/*
Restricts the ugpoint to the given timeInterval

*/

void AtInterval( const Interval<Instant>& i,
                 TemporalUnit<GPoint>& result ) const;

/*
Methods for Secondo integration.

*/

    static ListExpr Property();

    static bool Check(ListExpr type,
                      ListExpr& errorInfo );

    static ListExpr Out(ListExpr typeInfo,
                        Word value );

    static Word In(const ListExpr typeInfo,
                   const ListExpr instance,
                   const int errorPos,
                   ListExpr& errorInfo,
                   bool& correct );

    static Word Create(const ListExpr typeInfo );

    static void Delete(const ListExpr typeInfo,
                       Word& w );

    static void Close(const ListExpr typeInfo,
                      Word& w );

    static Word Clone(const ListExpr typeInfo,
                      const Word& w );

    static int SizeOf();

    static void* Cast(void* addr);

    inline static const string BasicType() { return "ugpoint"; }

    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }


    GPoint p0, p1;

};

/*
3 class MGPoint

Inherits from the Mapping of the TemporalAlgebra.

*/

class MGPoint : public Mapping< UGPoint, GPoint >
{
  public:
/*
The simple constructor should not be used.

*/

    MGPoint():Mapping<UGPoint,GPoint>() {};

    MGPoint( const int n );

    static ListExpr Property();

    static bool Check(ListExpr type,
                      ListExpr& errorInfo);

    void Clear();

    static void* Cast(void* addr);

    virtual MGPoint* Clone() const;

    void CopyFrom(const Attribute* right)
    {
      const MGPoint *src = (const MGPoint*) right;
      SetDefined(src->IsDefined());
      if (IsDefined())
      {
        units.copyFrom(src->units);
        if (src->m_traj_Defined) SetTrajectory(src->m_trajectory);
        SetTrajectoryDefined(src->m_traj_Defined);
        m_trajectory.TrimToSize();
        if (src->m_bbox.IsDefined()) SetBoundingBox(src->BoundingBox());
        SetBoundingBoxDefined(m_bbox.IsDefined());
      }
    }


inline int NumOfFLOBs() const
{
  return 2;
}


inline Flob* GetFLOB(const int i)
{
  if (i == 0) return &units;
  if (i == 1) return &m_trajectory;
  return 0;
}

/*
Returns the network id the ~mgpoint~ belongs to.

*/
   int GetNetworkId() const;

/*
Returns a pointer to the network object the ~mgpoint~ belongs to.

*/

   Network* GetNetwork() const;

/*
Computes the Euclidean Distance between two mgpoint with help of mpoint
distance function.

*/

    void Distance(const MGPoint *mgp, MReal *result) const;
    void DistanceE(const MGPoint* mgp, MReal* result) const;
    void DistanceN(MGPoint* mgp, MReal* result);
    void DistanceFunction(const UGPoint*, const UGPoint*, const Network*,
                          vector<UReal>&) const;
    void DivideUGPoint(const Network*);

/*
  Returns the network distance from the ~gpoint~ to the ~mgpoint~

*/

  void NetdistanceFromArg(const GPoint* gp, MReal* result) const;
  void NetdistanceFromArgShort(const GPoint* gp, MReal* result)const;

/*
Returns the network distance from the ~mgpoint~ to the ~gpoint~

*/

 void NetdistanceToArg(const GPoint* gp, MReal* result) const;
 void NetdistanceToArgShort(const GPoint* gp, MReal* result) const;


/*
Translates an mgpoint into an mpoint value.

*/
    void Mgpoint2mpoint(MPoint *mp) const;

/*
Returns the trajectory of the mgpoint as sorted gline or as DbArray of
~RouteInterval~s

*/

   void Trajectory(GLine* res);

   DbArray<RouteInterval>& GetTrajectory() ;

/*
Sets the Trajetory of the MGPoint from a GLine or a DbArray of ~RouteInterval~s

*/
   void SetTrajectory(const GLine src);

   void SetTrajectory(const DbArray<RouteInterval>& tra);

   void SetTrajectoryDefined(const bool defined);

/*
Returns the deftime of the mgpoint as periods value.

*/

   void Deftime(Periods &res) const;

/*
Returns true if the mgpoint is defined at least in one of the periods resp.
at the given Instant.

*/
   bool Present(Periods *per) const;

   bool Present(Instant *inst) const;

/*
Sets the length of the trip of the mgpoint.

*/

   void SetLength(const double x);

/*
Returns the length of the trip of the mgpoint.

*/

   double Length() const;

   double GetLength() const;

/*
Returns a mgpoint representing the intersection of 2 mgpoints

*/

  void Intersection(const MGPoint* mgp, MGPoint *res) const;

/*
Returns true if there exists a intersection of the two mgpoints

*/

  bool Intersects(const MGPoint* mgp) const;

/*
Returns a mbool telling when the mgpoint was inside the gline.

*/

  void Inside(const GLine* gl, MBool *res) const;


/*
Returns a mgpoint restricted to the given periods respectively instant value.

*/
   void Atperiods(const Periods *per, MGPoint *res) const;

   void Atinstant(const Instant *inst, Intime<GPoint> *res) const;

/*
Returns a mgpoint restricted to the times it was at the given gpoint resp.
gline.

*/
   void At(const GPoint *gp, MGPoint *res) const;

   void At(const GLine *gl, MGPoint *res) const;

/*
Returns the union of two time disjoint ~mgpoint~. ~undef~ elsewhere.

*/

   void Union(const MGPoint *mp, MGPoint *res) const;
/*
Returns a mgpoint with smaller number of units because units with speed
differences lower than d are compacted to be one unit.

*/

   void Simplify(const double d, MGPoint* res) const;

/*
Returns true if the mgpoint passes at least once the gpoint resp. gline.

*/
   bool Passes(const GPoint *gp) ;

   bool Passes(GLine *gl) ;

/*
Returns the spatiotemporal 3 dimensional bounding box of the ~mgpoint~.

*/

  Rectangle<3> BoundingBox(const Geoid* geoid = 0) const;

/*
  Adds a unit to the ~mgpoint~ if it is in bulkload state.

*/

  void Add( const UGPoint& unit);

/*
Restricts a ~mgpoint~ to the given unit intervals.

*/

  void Restrict( const vector< pair<int, int> >& intervals );

/*
Prints the mgpoint value.

*/

  ostream& Print( ostream &os ) const;

/*
Returns the sections passed by the ~mgpoint~

*/

  void GetPassedSections(SortedTree<Entry<SectionValue> > *result) const;
  void GetPassedSections(const Network *pNetwork,
                         SortedTree<Entry<SectionValue> > *result) const;

  bool operator==( const MGPoint& r ) const;

  void SetBoundingBoxDefined(const bool defined);

  void SetBoundingBox(const Rectangle<3> mbr);

  int Position(const Instant &inst, bool atinst=true) const;

  void GetMGPSecUnits(vector<MGPSecUnit> &res, double maxSectLength,
                      Network *pNet) const;

  inline static const string BasicType() { return "mgpoint"; }

  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }

  DbArray<RouteInterval> m_trajectory;
  private:

  bool m_traj_Defined;
  double m_length;
  Rectangle<3> m_bbox;
};

} // end of namespace temporalnet

#endif // _TEMPORAL_NET_ALGEBRA_H_
