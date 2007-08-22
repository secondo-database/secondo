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

[1] Implementation of GLine in Module Network Algebra

Mai-Oktober 2007 Martin Scheppokat

[TOC]
 
1 Overview


This file contains the implementation of ~gline~


2 Defines, includes, and constants

*/

#ifndef UGPOINT_H_
#define UGPOINT_H_

#ifndef GPOINT_H_
#error GPoint.h is needed by UGPoint.h. \
Please include in *.cpp-File.
#endif

#ifndef _TEMPORAL_ALGEBRA_H_
#error TemporalAlgebra.h is needed by UGPoint.h. \
Please include in *.cpp-File.
#endif


/*
3.8 UGPoint

This class will be used in the ~ugpoint~ type constructor, i.e., the type constructor
for the temporal unit of gpoint values.

*/
struct UGPoint : public SpatialTemporalUnit<GPoint, 3>
{
/*
3.8.1 Constructors and Destructor

*/
  UGPoint() {};

  UGPoint(bool is_defined):
    SpatialTemporalUnit<GPoint, 3>(is_defined) {};

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
    {}

  UGPoint( const Interval<Instant>& interval,
           const GPoint& p0, 
           const GPoint& p1 ):
    SpatialTemporalUnit<GPoint, 3>( interval ),
    p0( p0 ),
    p1( p1 )
    {}

/*
3.6.2 Operator redefinitions

*/

  virtual UGPoint& operator=( const UGPoint& i )
  {
    *((TemporalUnit<GPoint>*)this) = *((TemporalUnit<GPoint>*)&i);
    p0 = i.p0;
    p1 = i.p1;

    return *this;
  }
/*
Returns ~true~ if this temporal unit is equal to the temporal unit ~i~ and ~false~ if they are different.

*/

  virtual bool operator!=( const UGPoint& i ) const
  {
    return !( *this == i );
  }
/*
Returns ~true~ if this temporal unit is different to the temporal unit ~i~ and ~false~ if they are equal.

*/

/*
3.8.3 Functions to be part of relations

*/

  inline virtual size_t Sizeof() const
  {
    return sizeof( *this );
  }

  inline virtual UGPoint* Clone() const
  {
    UGPoint *res;
    res = new UGPoint( );
    res->defined = TemporalUnit<GPoint>::defined;
    return res;
  }

  inline virtual void CopyFrom( const StandardAttribute* right )
  {
    const UGPoint* i = (const UGPoint*)right;

    TemporalUnit<GPoint>::defined = i->defined;
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

  virtual const Rectangle<3> BoundingBox() const
  {
    throw SecondoException("Method UGPoint::BoundingBox not implemented.");
//    return Rectangle<3>( true, 1, //MIN( p0.GetX(), p1.GetX() ),
//                               2, //MAX( p0.GetX(), p1.GetX() ),
//                               3, //MIN( p0.GetY(), p1.GetY() ),
//                               4, //MAX( p0.GetY(), p1.GetY() ),
//                               timeInterval.start.ToDouble(),
//                               timeInterval.end.ToDouble() );
  }

  virtual void TemporalFunction( const Instant& t,
                                 GPoint& result,
                                 bool ignoreLimits = false ) const;
  virtual bool Passes( const GPoint& val ) const;
  virtual bool At( const GPoint& val, TemporalUnit<GPoint>& result ) const;


/*
4.9.3 Property Function

*/
    static ListExpr Property();

/*
4.9.3 Kind Checking Function

*/
    static bool Check(ListExpr type, 
                      ListExpr& errorInfo );

/*
4.9.4 ~Out~-function

*/
    static ListExpr Out(ListExpr typeInfo, 
                        Word value );

/*
4.9.5 ~In~-function

*/
    static Word In(const ListExpr typeInfo, 
                   const ListExpr instance,
                   const int errorPos, 
                   ListExpr& errorInfo, 
                   bool& correct );

/*
4.9.6 ~Create~-function

*/
    static Word Create(const ListExpr typeInfo );

/*
4.9.7 ~Delete~-function

*/
    static void Delete(const ListExpr typeInfo, 
                       Word& w );

/*
4.9.8 ~Close~-function

*/
    static void Close(const ListExpr typeInfo, 
                      Word& w );

/*
4.9.9 ~Clone~-function

*/
    static Word Clone(const ListExpr typeInfo, 
                      const Word& w );

/*
4.9.10 ~Sizeof~-function

*/
    static int SizeOf();

/*
4.9.11 ~Cast~-function

*/
    static void* Cast(void* addr);


/*
3.8.4 Attributes

*/
    GPoint p0, p1;
};


#endif /*UGPOINT_H_*/
