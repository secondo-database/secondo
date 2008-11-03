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
1.3 UGPoint

This class will be used in the ~ugpoint~ type constructor, i.e., the type
constructor for the temporal unit of gpoint values.

*/
class UGPoint : public SpatialTemporalUnit<GPoint, 3>
{
  public:
  UGPoint() {
    del.refs=1;
    del.isDelete=true;
    m_bbox = Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  };

  UGPoint(bool is_defined):
    SpatialTemporalUnit<GPoint, 3>(is_defined)
    {
      del.refs=1;
      del.isDelete=true;
      TemporalUnit<GPoint>::defined = is_defined;
      m_bbox = Rectangle<3> (false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
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
      Point *s = p0.ToPoint();
      Point *e = p1.ToPoint();
      double ts = interval.start.ToDouble();
      double te = interval.end.ToDouble();
      m_bbox = Rectangle<3> (true,
                             min(s->GetX(), e->GetX()),
                             max(s->GetX(), e->GetX()),
                             min(s->GetY(), e->GetY()),
                             max(s->GetY(), e->GetY()),
                            ts, te);
      delete s;
      delete e;
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
      GPoint sgp = p0;
      GPoint egp = p1;
      Point *s = sgp.ToPoint();
      Point *e = egp.ToPoint();
      double ts = interval.start.ToDouble();
      double te = interval.end.ToDouble();
      m_bbox = Rectangle<3> (true,
                             min(s->GetX(), e->GetX()),
                             max(s->GetX(), e->GetX()),
                             min(s->GetY(), e->GetY()),
                             max(s->GetY(), e->GetY()),
                            ts, te);
      delete s;
      delete e;
    }

  UGPoint(const UGPoint(&source)){

    *((TemporalUnit<GPoint>*)this)=*((TemporalUnit<GPoint>*)&source);
    p0=source.p0;
    p1=source.p1;
    del.refs=1;
    del.isDelete=true;
    TemporalUnit<GPoint>::defined = source.defined;
    m_bbox = source.m_bbox;
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
   m_bbox = i.m_bbox;
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
    UGPoint *res;
    res = new UGPoint(timeInterval, p0, p1);
    res->defined = TemporalUnit<GPoint>::defined;
    return res;
  }

  inline virtual void CopyFrom( const StandardAttribute* right )
  {
    const UGPoint* i = (const UGPoint*)right;

    if(i->defined)
      {
        timeInterval.CopyFrom( i->timeInterval );
        p0 = i->p0;
        p1 = i->p1;
        m_bbox = i->m_bbox;
      }
    else
      {
        timeInterval = Interval<Instant>();
        p0 = GPoint( false, 0, 0, 0.0, None);
        p1 = GPoint( false, 0, 0, 0.0, None);
        m_bbox = Rectangle<3> (false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
      }
  }

  virtual const Rectangle<3> BoundingBox() const {

    return m_bbox;
  }

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

  inline const Rectangle<2> NetBoundingBox2d() const
  {
      return Rectangle<2> (true,
                           (double) p0.GetRouteId(),
                           (double) p0.GetRouteId(),
                          min(p0.GetPosition(), p1.GetPosition()),
                          max(p0.GetPosition(), p1.GetPosition()));
  }

  virtual void TemporalFunction( const Instant& t,
                                 GPoint& result,
                                 bool ignoreLimits = false ) const;
  virtual bool Passes( const GPoint& val ) const;
  virtual bool At( const GPoint& val, TemporalUnit<GPoint>& result ) const;

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

    int GetUnitRid();

    double GetUnitStartPos();

    double GetUnitEndPos();

    double GetUnitStartTime();

    double GetUnitEndTime();

    void Distance (const UGPoint &ugp, UReal &ur) const;

    void Deftime(Periods &per);

    Instant TimeAtPos(double pos);

    GPoint p0, p1;

    Rectangle <3> m_bbox;
};

/*
1.4 MGPoint

*/

class MGPoint : public Mapping< UGPoint, GPoint >
{
  public:
/*
The simple constructor should not be used.

*/
    MGPoint();

    MGPoint( const int n );

    ~MGPoint() {}

    static ListExpr Property();

    static bool Check(ListExpr type,
                      ListExpr& errorInfo);

    int GetNetworkId();

    void Clear();

    virtual Attribute* Clone() const
    {
      MGPoint *res = new MGPoint(GetNoComponents());
      res->m_trajectory = m_trajectory;
      res->m_length = m_length;
      res->m_deftime = m_deftime;
      res->m_bbox = m_bbox;
      res->StartBulkLoad();
      const UGPoint *u;
      for (int i=0; i<GetNoComponents() ; i++) {
        Get(i,u);
        res->Add(*u);
      }
      res->EndBulkLoad();
      return (Attribute*) res;
    }

    void CopyFrom(const StandardAttribute* right)
    {
      const MGPoint *src = (const MGPoint*) right;
      Clear();
      m_trajectory = src->m_trajectory;
      m_deftime = src->m_deftime;
      m_length = src->m_length;
      m_bbox = src->m_bbox;
      StartBulkLoad();
      const UGPoint *u;
      for (int i = 0; i < src->GetNoComponents(); i++) {
        src->Get(i,u);
        Add(*u);
      }
      EndBulkLoad();
    }

/*
Computes the Euclidean Distance between two mgpoint with help of mpoint
distance function.

*/

    void Distance(MGPoint *mgp, MReal *result);

/*
Translates an mgpoint into an mpoint value.

*/

    void Mgpoint2mpoint(MPoint *mp);

/*
Returns the trajectory of the mgpoint as sorted gline.

*/

   void Trajectory(GLine *res);

/*
Returns the deftime of the mgpoint as periods value.

*/

   Periods* Deftime();

/*
Returns true if the mgpoint is defined at least in one of the periods resp.
at the given Instant.

*/
   bool Present(Periods *per);

   bool Present(Instant *inst);

/*
Returns the length of the trip of the mgpoint.

*/

   double Length();

/*
Returns a mgpoint representing the intersection of 2 mgpoints

*/

  void Intersection(MGPoint* mgp, MGPoint *res);

/*
Returns a mbool telling when the mgpoint was inside the gline.

*/

  void Inside(GLine* gl, MBool *res);


/*
Returns a mgpoint restricted to the given periods value.

*/
   void Atperiods(Periods *per, MGPoint *res);

/*
Returns a mgpoint restricted to the times it was at the given gpoint resp.
gline.

*/
   void At(GPoint *gp, MGPoint *res);

   void At(GLine *gl, MGPoint *res);

/*
Returns a mgpoint with smaller number of units because units with speed
differences lower than d are compacted to be one unit.

*/

   MGPoint* Simplify(double d);

/*
Returns true if the mgpoint passes at least once the gpoint resp. gline.

*/
   bool Passes(GPoint *gp);

   bool Passes(GLine *gl);

   void EndBulkLoad(const bool sort = true);
   void Add(const UGPoint& unit);
   void Restrict(const vector <pair<int,int> >& intervals);
   bool operator==(const MGPoint &mgp) const;
   Rectangle<3u> BoundingBox() const;

   void AtInstant( const Instant& t, Intime<GPoint>& result ) const;

    Periods m_deftime;
    GLine m_trajectory;
    double m_length;
    Rectangle<3> m_bbox;

    void RestoreBoundingBox();
//     void RestoreDeftime();
//     void RestoreTrajectory();
//     void RestoreLength();
    void RestoreAllThree();


};


#endif // _TEMPORAL_NET_ALGEBRA_H_
