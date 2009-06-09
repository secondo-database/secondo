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

February 2008 - June 2008 Simone Jandt

1.2 Defines, includes, and constants

*/

#ifndef _TEMPORAL_NET_ALGEBRA_H_
#define _TEMPORAL_NET_ALGEBRA_H_

#include <iostream>
#include <sstream>
#include <string>
#include "NestedList.h"
#include "QueryProcessor.h"

#ifndef __NETWORK_ALGEBRA_H__
#error NetworkAlgebra.h is needed by TemporalNetAlgebra.h. \
Please include in *.cpp-File.
#endif

#ifndef _TEMPORAL_ALGEBRA_H_
#error TemporalAlgebra.h is needed by MGPoint.h. \
Please include in *.cpp-File.
#endif



using namespace datetime;

/*
Instant

This class represents a time instant, or a point in time. It will be
used in the ~instant~ type constructor.

*/
typedef DateTime Instant;

/*
2 UGPoint

This class will be used in the ~ugpoint~ type constructor, i.e., the type
constructor for the temporal unit of gpoint values. It inherits from the
SpatialTemporalUnit class.

*/
class UGPoint : public SpatialTemporalUnit<GPoint, 3>
{
  public:
  UGPoint() {
    del.refs=1;
    del.isDelete=true;
  };

  UGPoint(bool is_defined):
    SpatialTemporalUnit<GPoint, 3>(is_defined)
    {
      del.refs=1;
      del.isDelete=true;
      TemporalUnit<GPoint>::defined = is_defined;
    };

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
    {
      del.refs=1;
      del.isDelete=true;
    }

  UGPoint( const Interval<Instant>& interval,
           const GPoint& p0,
           const GPoint& p1 ):
    SpatialTemporalUnit<GPoint, 3>( interval ),
    p0( p0 ),
    p1( p1 )
    {
      del.refs=1;
      del.isDelete=true;
      TemporalUnit<GPoint>::defined=true;
    }

    UGPoint( const Interval<Instant>& interval,
           const int in_NetworkID,
           const int in_RouteID,
           const Side in_Side,
           const double in_Position0,
           const double in_Position1,
           Network *&pNetwork):
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
    {
      del.refs=1;
      del.isDelete=true;
    }

  UGPoint( const Interval<Instant>& interval,
           const GPoint& p0,
           const GPoint& p1,
           Network *&pNetwork):
    SpatialTemporalUnit<GPoint, 3>( interval ),
    p0( p0 ),
    p1( p1 )
    {
      del.refs=1;
      del.isDelete=true;
      TemporalUnit<GPoint>::defined=true;
    }

  UGPoint(const UGPoint(&source)){
    *((TemporalUnit<GPoint>*)this)=*((TemporalUnit<GPoint>*)&source);
    p0=source.p0;
    p1=source.p1;
    del.refs=1;
    del.isDelete=true;
    TemporalUnit<GPoint>::defined = source.defined;
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
   TemporalUnit<GPoint>::defined=i.defined;
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

  inline virtual void CopyFrom( const StandardAttribute* right )
  {
    const UGPoint* i = (const UGPoint*)right;

    if(i->defined)
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
  
 virtual const Rectangle<3> BoundingBox() const;

 Rectangle<3> BoundingBox(Network* &pNetwork) const;

 virtual double Distance(const Rectangle<3>& rect) const;

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
  inline const Rectangle<2> NetBoundingBox2d() const
  {
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
  virtual bool Passes( const GPoint& val ) const;

  /*
  Returns the ~igpoint~ the ~ugpoint~ was at a given network position.

  */
  virtual bool At( const GPoint& val, TemporalUnit<GPoint>& result ) const;

  /*
  Returns the route id of the ~ugpoint~.

  */

  int GetUnitRid();

  /*
  Returns the startposition of the ~ugpoint~.

  */
  double GetUnitStartPos();

  /*
  Returns the end position of the ~ugpoint~.

  */

  double GetUnitEndPos();

  /*
  Returns the start timeinstant of the ~ugpoint~s time interval as double value.

  */

  double GetUnitStartTime();

  /*
  Returns the end timeinstant of the ~ugpoint~s time interval as double value.

  */

  double GetUnitEndTime();

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

  Instant TimeAtPos(double pos);


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

    MGPoint(){};

    MGPoint( const int n );

    static ListExpr Property();

    static bool Check(ListExpr type,
                      ListExpr& errorInfo);

    void Clear();

    virtual MGPoint* Clone() const;

    void CopyFrom(const StandardAttribute* right)
    {
      const MGPoint *src = (const MGPoint*) right;
      Clear();
      StartBulkLoad();
      const UGPoint *u;
      for (int i = 0; i < src->GetNoComponents(); i++) {
        src->Get(i,u);
        Add(*u);
      }
      EndBulkLoad(true);
      if (src->m_traj_Defined) SetTrajectory(src->m_trajectory);
      SetTrajectoryDefined(src->m_traj_Defined);
      m_trajectory.TrimToSize();
      if (src->m_bbox.IsDefined()) SetBoundingBox(src->BoundingBox());
      SetBoundingBoxDefined(m_bbox.IsDefined());
    }


inline int NumOfFLOBs() const
{
  return 2;
}


inline FLOB* GetFLOB(const int i)
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
Computes the Euclidean Distance between two mgpoint with help of mpoint
distance function.

*/

    void Distance(MGPoint *&mgp, MReal *&result);
    void DistanceE(MGPoint* mgp, MReal* result);
    void DistanceN(MGPoint* mgp, MReal* result);
    void DistanceFunction(UGPoint*,UGPoint*,Network*,vector<UReal>&);
    void DivideUGPoint(Network*);
/*
Translates an mgpoint into an mpoint value.

*/
    void Mgpoint2mpoint(MPoint *&mp);

/*
Returns the trajectory of the mgpoint as sorted gline or as DBArray of
~RouteInterval~s

*/

   void Trajectory(GLine* res);

   DBArray<RouteInterval>& GetTrajectory() ;

/*
Sets the Trajetory of the MGPoint from a GLine or a DBArray of ~RouteInterval~s

*/
   void SetTrajectory(GLine src);

   void SetTrajectory(const DBArray<RouteInterval>& tra);

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
Returns a mbool telling when the mgpoint was inside the gline.

*/

  void Inside(GLine* &gl, MBool *&res);


/*
Returns a mgpoint restricted to the given periods respectively instant value.

*/
   void Atperiods(Periods *&per, MGPoint *&res);

   void Atinstant(Instant *&inst, Intime<GPoint> *&res);

/*
Returns a mgpoint restricted to the times it was at the given gpoint resp.
gline.

*/
   void At(GPoint *&gp, MGPoint *&res);

   void At(GLine *&gl, MGPoint *&res);

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
   bool Passes(GPoint *&gp);

   bool Passes(GLine *&gl);

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
  void Restrict( const vector< pair<int, int> >& intervals );
  /*
  Prints the mgpoint value.

  */
  ostream& Print( ostream &os ) const;
  
  bool operator==( const MGPoint& r ) const;
  
  void SetBoundingBoxDefined(bool defined);
  
  void SetBoundingBox(Rectangle<3> mbr);

  DBArray<RouteInterval> m_trajectory;

  int Position(Instant &inst);

  private:

  bool m_traj_Defined;
  double m_length;
  Rectangle<3> m_bbox;
};


#endif // _TEMPORAL_NET_ALGEBRA_H_
