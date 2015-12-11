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

#ifndef _TEMPORAL_NET2_ALGEBRA_H_
#define _TEMPORAL_NET2_ALGEBRA_H_

#include <iostream>
#include <sstream>
#include <string>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "NetworkAlgebra.h"

#ifndef __NETWORK2_ALGEBRA_H__
#error Network2Algebra.h is needed by TemporalNet2Algebra.h. \
Please include in *.cpp-File.
#endif

#ifndef _TEMPORAL_ALGEBRA_H_
#error TemporalAlgebra.h is needed by MGPoint.h. \
Please include in *.cpp-File.
#endif

namespace temporalalgebra{

namespace temporalnet2 {


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

    MGPSecUnit(bool defined, int secId,  int part, 
               network2::Side direct, double sp,
               Interval<Instant> timeInterval);

    MGPSecUnit( const MGPSecUnit& in_xOther );

    ~MGPSecUnit();

/*
2.2 Methods of class ~mgpsecunit~

Get and Set private attributes.

*/

    int GetSecId() const;

    int GetPart()const;

    network2::Side GetDirect() const;

    double GetSpeed()const;

    Interval<Instant> GetTimeInterval() const;

    double GetDurationInSeconds() const;

    void SetSecId(int secId);

    void SetPart(int p);

    void SetDirect(network2::Side dir);

    void SetSpeed(double x);

    void SetTimeInterval(Interval<Instant> time);

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

    std::ostream& Print( std::ostream& os ) const;

    static ListExpr Out(ListExpr typeInfo, Word value);

    static Word In(const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr& errorInfo, bool& correct);

    static bool CheckKind( ListExpr type, ListExpr& errorInfo );

    int NumOfFLOBs() const;

    Flob* GetFLOB(const int i);

    static void* Cast(void* addr);


/*
Function for Operations.

*/
  private:

    int m_secId; //section id
    int m_part;  //number of part of section if necessary default = 1
    network2::Side m_direct; // 0=down, 1=up, 2=none
    double m_speed; // m/s
    Interval<Instant> m_time;

};

/*
2 UGPoint

This class will be used in the ~ugpoint~ type constructor, i.e., the type
constructor for the temporal unit of gpoint values. It inherits from the
SpatialTemporalUnit class.

*/

class UGPoint : public SpatialTemporalUnit<network2::GPoint, 3>
{
  public:
  UGPoint():SpatialTemporalUnit<network2::GPoint,3>() {};

  UGPoint(bool is_defined):
    SpatialTemporalUnit<network2::GPoint, 3>(is_defined)
    {  };

  UGPoint( const Interval<Instant>& interval,
           const int in_NetworkID,
           const int in_RouteID,
           const network2::Side in_Side,
           const double in_Position0,
           const double in_Position1 ):
    SpatialTemporalUnit<network2::GPoint, 3>( interval ),
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
           const network2::GPoint& p0,
           const network2::GPoint& p1 ):
    SpatialTemporalUnit<network2::GPoint, 3>( interval ),
    p0( p0 ),
    p1( p1 )
    {  }

    UGPoint( const Interval<Instant>& interval,
           const int in_NetworkID,
           const int in_RouteID,
           const network2::Side in_Side,
           const double in_Position0,
           const double in_Position1,
           network2::Network *&pNetwork):
    SpatialTemporalUnit<network2::GPoint, 3>( interval ),
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
           const network2::GPoint& p0,
           const network2::GPoint& p1,
           network2::Network *&pNetwork):
    SpatialTemporalUnit<network2::GPoint, 3>( interval ),
    p0( p0 ),
    p1( p1 )
    {  }

  UGPoint(const UGPoint &source):
        SpatialTemporalUnit<network2::GPoint,3>(source.IsDefined())
  {
    *((TemporalUnit<network2::GPoint>*)this)
                        =*((TemporalUnit<network2::GPoint>*)&source);
    p0=source.p0;
    p1=source.p1;
    SetDefined(source.IsDefined());
  }

  std::ostream& Print( std::ostream &os ) const
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
    *((TemporalUnit<network2::GPoint>*)this) 
                     = *((TemporalUnit<network2::GPoint>*)&i);
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
    return      *((TemporalUnit<network2::GPoint>*)this) 
             == *((TemporalUnit<network2::GPoint>*)&i) &&
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
        p0 = network2::GPoint( false, 0, 0, 0.0, network2::None);
        p1 = network2::GPoint( false, 0, 0, 0.0, network2::None);
      }
  }

/*
Returns the 3 dimensional spatio-temporal BoundingBox of the ~ugpoint~.

*/

 virtual const Rectangle<3> BoundingBox(const Geoid* = 0) const;

 Rectangle<3> BoundingBox(network2::Network* &pNetwork) const;

 virtual double Distance(const Rectangle<3>& rect, const Geoid* = 0) const;

 virtual bool IsEmpty() const{ return IsDefined(); }

 /*
 Returns the 3 dimensional network bounding box of the ~ugpoint~, which is
 degenerated to be a rectangle. The two x-coordinates are identic and given by
 route id of the ~ugpoint~, the y-coordinates are given by the start and the
 end position of the ugpoint, and the z-coordinates are given by the timestamps
 of the timeinterval of the ~ugpoint~.

 */
  virtual const Rectangle<3> NetBoundingBox3d() const
  {
    return Rectangle<3> (true,
                        (double) p0.GetRouteId(),
                        (double) p0.GetRouteId(),
                        std::min(p0.GetPosition(),p1.GetPosition()),
                        std::max(p0.GetPosition(),p1.GetPosition()),
                        timeInterval.start.ToDouble(),
                        timeInterval.end.ToDouble());
  }

  /*
  The 2 dimensinal network bounding box of the ~ugpoint~ is degenerated to a
  line. the x- and y-coordinates are defined analogous to the x- and y-coord-
  inates of the ~NetBoundingBox3d~.

  */
  inline const Rectangle<2> NetBoundingBox2d() const
  {
      return Rectangle<2> (true,
                           (double) p0.GetRouteId(),
                           (double) p0.GetRouteId(),
                          std::min(p0.GetPosition(), p1.GetPosition()),
                          std::max(p0.GetPosition(), p1.GetPosition()));
  }

  /*
  Computes the network position of the ~ugpoint~ at a given time instant ~t~.

  */
  virtual void TemporalFunction( const Instant& t,
                                 network2::GPoint& result,
                                 bool ignoreLimits = false ) const;

  /*
  Returns true if the ~ugpoint~ passes a given ~gpoint~ false elsewhere.

  */
  virtual bool Passes( const network2::GPoint& val ) const;

  /*
  Returns the ~igpoint~ the ~ugpoint~ was at a given network position.

  */
  virtual bool At( const network2::GPoint& val, 
                   TemporalUnit<network2::GPoint>& result ) const;

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

  inline network2::Side GetUnitSide() const
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

  inline network2::Side MovingDirection() const
  {
    if (p0.GetPosition() < p1.GetPosition()) return network2::Up;
    else
      if (p0.GetPosition() > p1.GetPosition()) return network2::Down;
      else return network2::None;
  }

  void GetPassedSections(network2::Network* pNet, 
                         std::vector<TupleId>& pS) const;

  void GetMGPSecUnits(std::vector<MGPSecUnit>& res, double maxSectLength,
                     network2::Network *pNet) const;

/*
SetMethoden f[ue]r ~ugpoint~

*/

  inline void SetUnitRid(int rid)
  {
    p0.SetRouteId(rid);
    p1.SetRouteId(rid);
  };

  inline void SetUnitStartPos(double pos)
  {
    p0.SetPosition(pos);
  };

  inline void SetUnitEndPos(double pos)
  {
    p1.SetPosition(pos);
  };

  inline void SetUnitSide(network2::Side a)
  {
    p0.SetSide(a);
    p1.SetSide(a);
  };

  inline void SetUnitStartTime(Instant time)
  {
    timeInterval.start = time;
  };

  inline void SetUnitEndTime(Instant time)
  {
    timeInterval.end = time;
  };

  inline void SetUnitStartTimeBool(bool b)
  {
    timeInterval.lc = b;
  };

  inline void SetUnitEndTimeBool(bool b)
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

  void Deftime(Periods &per);

  /*
  Returns the time instant the ~ugpoint~ was at a given position.

  */

  Instant TimeAtPos(double pos) const;


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

    network2::GPoint p0, p1;

};

/*
3 class MGPoint

Inherits from the Mapping of the TemporalAlgebra.

*/

class MGPoint : public Mapping< UGPoint, network2::GPoint >
{
  public:
/*
The simple constructor should not be used.

*/

    MGPoint():Mapping<UGPoint,network2::GPoint>() {};

    MGPoint( const int n );

    static ListExpr Property();

    static bool Check(ListExpr type,
                      ListExpr& errorInfo);

    void Clear();

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

   network2::Network* GetNetwork() const;

/*
Computes the Euclidean Distance between two mgpoint with help of mpoint
distance function.

*/

    void Distance(MGPoint *&mgp, MReal *&result);
    void DistanceE(MGPoint* mgp, MReal* result);
    void DistanceN(MGPoint* mgp, MReal* result);
    void DistanceFunction(UGPoint*,UGPoint*,
                          network2::Network*,std::vector<UReal>&);
    void DivideUGPoint(network2::Network*);
/*
Translates an mgpoint into an mpoint value.

*/
    void Mgpoint2mpoint(MPoint *&mp);

/*
Returns the trajectory of the mgpoint as sorted gline or as DbArray of
~RouteInterval~s

*/

   void Trajectory(network2::GLine* res);

   DbArray<network2::RouteInterval>& GetTrajectory() ;

/*
Sets the Trajetory of the MGPoint from a GLine or a DbArray of ~RouteInterval~s

*/
   void SetTrajectory(network2::GLine src);

   void SetTrajectory(const DbArray<network2::RouteInterval>& tra);

   void SetTrajectoryDefined(bool defined);

/*
Returns the deftime of the mgpoint as periods value.

*/

   void Deftime(Periods &res);

/*
Returns true if the mgpoint is defined at least in one of the periods resp.
at the given Instant.

*/
   bool Present(Periods *&per);

   bool Present(Instant *&inst);

/*
Sets the length of the trip of the mgpoint.

*/

   void SetLength(double x);

/*
Returns the length of the trip of the mgpoint.

*/

   double Length();

   double GetLength() const;

/*
Returns a mgpoint representing the intersection of 2 mgpoints

*/

  void Intersection(MGPoint* &mgp, MGPoint *&res);

/*
Returns true if there exists a intersection of the two mgpoints

*/

  bool Intersects(MGPoint* mgp);

/*
Returns a mbool telling when the mgpoint was inside the gline.

*/

  void Inside(network2::GLine* &gl, MBool *&res);


/*
Returns a mgpoint restricted to the given periods respectively instant value.

*/
   void Atperiods(Periods *&per, MGPoint *&res);

   void Atinstant(Instant *&inst, Intime<network2::GPoint> *&res);

/*
Returns a mgpoint restricted to the times it was at the given gpoint resp.
gline.

*/
   void At(network2::GPoint *&gp, MGPoint *&res);

   void At(network2::GLine *&gl, MGPoint *&res);

/*
Returns the union of two time disjoint ~mgpoint~. ~undef~ elsewhere.

*/

   void Union(MGPoint *mp, MGPoint *res);
/*
Returns a mgpoint with smaller number of units because units with speed
differences lower than d are compacted to be one unit.

*/

   void Simplify(double d, MGPoint* res);

/*
Returns true if the mgpoint passes at least once the gpoint resp. gline.

*/
   bool Passes(network2::GPoint *&gp);

   bool Passes(network2::GLine *&gl);

/*
Returns the spatiotemporal 3 dimensional bounding box of the ~mgpoint~.

*/

  Rectangle<3> BoundingBox() const;

/*
  Adds a unit to the ~mgpoint~ if it is in bulkload state.

*/

  void Add( const UGPoint& unit);

/*
Restricts a ~mgpoint~ to the given unit intervals.

*/
  void Restrict( const std::vector< std::pair<int, int> >& intervals );
  /*
  Prints the mgpoint value.

  */
  std::ostream& Print( std::ostream &os ) const;

  bool operator==( const MGPoint& r ) const;

  void SetBoundingBoxDefined(bool defined);

  void SetBoundingBox(Rectangle<3> mbr);

  DbArray<network2::RouteInterval> m_trajectory;

  int Position(const Instant &inst, bool atinst=true);

  void GetMGPSecUnits(std::vector<MGPSecUnit> &res, double maxSectLength,
                     network2::Network *pNet) const;

  private:

  bool m_traj_Defined;
  double m_length;
  Rectangle<3> m_bbox;
};


} // end of namepsace temporalnet2
} // end of namespace temporalalgebra

#endif // _TEMPORAL_NET2_ALGEBRA_H_
