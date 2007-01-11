/*
----
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
----

0 TODO


1 Includes and Initializationtemporallocationsext

Place for initialization of pointer variables, constants and namespaces and
inclusion of header files concerning Secondo.

*/
#include <set>
#include <time.h>
#include <vector>
#include <map>
#include <cmath>
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "TemporalAlgebra.h"
#include "MovingRegionAlgebra.h"
#include "DateTime.h"
#include "TemporalExtAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;

/*

2 Type definitions, Auxiliary Functions

2.1 Global variable for unit of time: milliseconds [*] FactorForUnitOfTime
  FactorForUnitOfTime = 1. (default)

*/
double FactorForUnitOfTime = 1.;

/*
2.2 Global variable for unit of distance: meter [*] FactorForUnitOfDistance
  FactorForUnitOfDistance = 1. (default)

*/
double FactorForUnitOfDistance = 1.;

struct USegments
{
    int unit_nr;
    Interval<Instant> unit_interval;
    vector<MSegmentData> sgms;
};

struct GroupOfIntervals
{
    int unit_nr;
    Interval<Instant> str_inst;
};

/*
2.3 Function: MinMaxValueFunction

Parameters:
  utemp: pointer to UReal
  minimum: a double (by reference)
  maximum: a double (by reference)

Return: nothing

*/

void MinMaxValueFunction(const UReal* utemp, double& minimum, double& maximum)
{
    double ts, te, ts_value, te_value, a, b, c;
    double t_extrem, t_extrem_value;
    bool lh = utemp->timeInterval.lc;
    bool rh = utemp->timeInterval.rc;
    bool conv_conc = true;


    ts = 0;
    te = utemp->timeInterval.end.ToDouble()
        - utemp->timeInterval.start.ToDouble();

    a = utemp->a;
    b = utemp->b;
    c = utemp->c;

    ts_value = c;
    te_value = a*pow(te,2) + b*te + c;

    if(0)
    {
        cout << endl << "Range: " << endl;
        cout << "ts: " << ts << endl;
        cout << "te: " << te << endl << endl;
        cout << "ts_value: " << ts_value << endl;
        cout << "te_value: " << te_value << endl << endl;
    }


    if( utemp->a != 0 )
    {
        t_extrem = - b / ( 2 * a );
        if(0)
        {
            cout << endl << "t_extrem = " << t_extrem << endl;
        }
        if( (!lh && !rh && ( t_extrem <= ts || t_extrem >= te ) ) ||
            (lh && rh && ( t_extrem < ts || t_extrem > te) ) ||
            (!lh && rh && ( t_extrem <= ts || t_extrem > te ) ) ||
            (lh && !rh && ( t_extrem < ts || t_extrem >= te ) )
        )
        {
            conv_conc = false;
        }
    }
    else
        conv_conc = false;

    if(conv_conc)
    {
        t_extrem_value = a*pow(t_extrem,2) + b*t_extrem + c;
        if(0)
        {
            cout << "t_extrem_value = " << t_extrem_value << endl;
        }
/*
Find maximum

*/
        maximum = ts_value;
        if(t_extrem_value > maximum)
            maximum = t_extrem_value;
        if(te_value > maximum)
            maximum = te_value;

/*
Find minimum

*/
        minimum = ts_value;
        if(t_extrem_value < minimum)
            minimum = t_extrem_value;
        if(te_value < minimum)
            minimum = te_value;
    }
    else
    {
        if(0)
        {
            cout << endl << "Ist NICHT conv_conc" << endl;
        }
        if(ts_value < te_value)
        {
            maximum = te_value;
            minimum = ts_value;
        }
        else
        {
            maximum = ts_value;
            minimum = te_value;
        }
    }
    if(0)
    {
        cout << endl << "RETURN:" << endl;
        cout << "minimum = " << minimum << endl;
        cout << "maximum = " << maximum << endl << endl;
    }
}

/*
2.4 Function timeIntervalOfRealInUReal

Parameters:

  value: real value
  ur: a pointer to the real unit
  t\_value: time as double value to return (by reference)

Return:

  bool: returns true, if val is contained in the ureal,
  otherwise false.

*/

bool timeIntervalOfRealInUReal(
    double value,
    const UReal* ur,
    double& t_value
)
{
    double unit_min, unit_max, ts, te;
    bool lh, rh;

    unit_min = ((URealExt*)ur)->GetUnitMin();
    unit_max = ((URealExt*)ur)->GetUnitMax();
    lh = ur->timeInterval.lc;
    rh = ur->timeInterval.rc;
    ts = (ur->timeInterval.start).ToDouble();
    te = (ur->timeInterval.end).ToDouble();
    if(0)
    {
        cout << endl << "GetUnitMin(): " << unit_min << endl;
        cout << "GetUnitMax(): " << unit_max << endl;
        cout << "Value: " << value << endl;
        cout << "ts: " << ts << endl;
        cout << "te: " << te << endl;
    }
    if( (!lh && !rh && ( value <= unit_min || value >= unit_max ) ) ||
         (lh && rh && ( value < unit_min || value > unit_max) ) ||
         (!lh && rh && ( value <= unit_min || value > unit_max ) ) ||
         (lh && !rh && ( value < unit_min || value >= unit_max ) )
      )
    {
        if(0)
        {
            cout << "Value outside of the unit range!!!!" << endl;
        }
        return false;
    }
    else
    {
        if(ur->a == 0 && ur->b != 0)
        {
/*
Find value in a linear function

*/
            t_value = ((value - ur->c) / ur->b) + ts;
            if(0)
            {
                cout << "---> value in a linear function" << endl;
            }
        }
        else
        {
            if(ur->a == 0 && ur->b == 0)
            {
/*
Find value in a constant function

*/
                t_value = ur->c;
                if(0)
                {
                    cout << "---> value in a constant function" << endl;
                }
            }
            else
            {
/*
Find value in a quadratic function with Newtons Method

*/
                if(0)
                {
                    cout << "---> value in a quadratic function" << endl;
                }

                double t_newton = te;
                double t_newton_old;
                double t, dt, t_newton_diff, t_var;
                if(0)
                {
                    cout << "t_newton(Start) = " << t_newton << endl;
                    cout << "a = " << ur->a << endl;
                    cout << "b = " << ur->b << endl;
                    cout << "c = " << ur->c << endl;
                }
                do
                {
                    t_newton_old = t_newton;
                    t_var = t_newton - ts;
                    if(ur->r)
                    {
                        t = sqrt(pow(t_var, 2)*ur->a + t_var*ur->b
                            + ur->c - value);
                        dt = (1/2)*(1/sqrt(t)) + (2*ur->a*t_var + ur->b);
                        if(0)
                        {
                            cout << "t = " << t << endl;
                            cout << "dt = " << dt << endl;
                        }
                    }
                    else
                    {
                        t = pow(t_var, 2)*ur->a + t_var*ur->b + ur->c - value;
                        dt = 2*ur->a*t_var + ur->b;
                        if(0)
                        {
                            cout << "t = " << t << endl;
                            cout << "dt = " << dt << endl;
                        }
                    }
                    t_newton = t_newton - (t/dt);
                    t_newton_diff = abs(t_newton_old) - abs(t_newton);
                    if(0)
                    {
                        cout << "t_newton = "
                                << t_newton << endl;
                        cout << "t_newton_old = "
                                << t_newton_old << endl;
                        cout << "t_newton_diff = "
                                << t_newton_diff << endl;
                    }
                }while(abs(t_newton_diff) > 0.0000001);
                t_value = t_newton;
            }
        }
        if(0)
        {
            cout << endl << "x-Value = " << t_value << endl;
        }
        return true;
    }

}

/*
2.5 Function AngleToXAxis

Parameters:

  p1: a pointer to start point
  p2: a pointer to end point

Return:

  res: a double. The angle of the line between both points to the X-axis.

NOTE: This function was copied from the SpatialAlgebra, from the
Value Mapping Function direction\_pp (LFRG)

*/
double AngleToXAxis(const Point* p1, const Point* p2, bool &defined)
{
    double res;
    double k;
    double direction;

    if (( p1->IsDefined())&&(p2->IsDefined())&&(*p1!=*p2))
    {
      Coord x1=p1->GetX();
      Coord y1=p1->GetY();
      Coord x2=p2->GetX();
      Coord y2=p2->GetY();

      if (x1==x2)
      {
        if (y2>y1)
        {
          res = 90.;
          defined = true;
        }
        else
        {
          res = 270.;
          defined = true;
        }
      }

      if (y1==y2)
      {
          if (x2>x1)
          {
            res = 0.;
            defined = true;
          }
          else
          {
            res = 180.;
            defined = true;
          }
      }
#ifdef RATIONAL_COORDINATES
  k=((y2.IsInteger()? y2.IntValue():y2.Value()) -
        (y1.IsInteger()? y1.IntValue():y1.Value())) /
       ((x2.IsInteger()? x2.IntValue():x2.Value()) -
        (x1.IsInteger()? x1.IntValue():x1.Value()));
#else
  k=(y2 - y1) / (x2 - x1);
#endif
  direction=atan(k) * 180 /  PI;

      int area;
      if ((x2>x1)&&(y2>y1))
      {
        area=1;
      }
      else if ((x2<x1)&&(y2>y1))
      {
        area=2;
        direction=180+direction;
      }
      else if ((x2<x1)&&(y2<y1))
      {
        area=3;
        direction=180+direction;
      }
      else if ((x2>x1)&&(y2<y1))
      {
        area=4;
        direction=360+direction;
      }

      res = direction;
      defined = true;
    }
    else
    {
      res = 0.;
      defined = false;
    }

    return res;
}

/*
2.6 Function ~IntersectionRPExt~

Parameters:
    mreg: a pointer to a MRegion
    mp: a MPoint (by reference)
    res: a MPoint for result (by reference)
    rp: a reference to an object
      RefinementPartition<MRegion, MPoint, URegionEmb, UPoint>
    merge: a boolean

Return: nothing

*/

void IntersectionRPExt(
    MRegion* mreg,
    MPoint& mp,
    MPoint& res,
    RefinementPartition<
        MRegion,
        MPoint,
        URegionEmb,
        UPoint>& rp,
    bool merge)
{

    res = 0;

    UPoint* pending = 0;

/*

For each interval in the refinement partition, we have to check whether
it maps to a region and point unit. If not, there is obviously no intersection
during this interval and we can skip if. Otherwise, we check if the region
and point unit, both restricted to this interval, intersect.

*/
    for (unsigned int i = 0; i < rp.Size(); i++)
    {
        Interval<Instant>* iv;
        int urPos;
        int upPos;

        rp.Get(i, iv, urPos, upPos);

        if (urPos == -1 || upPos == -1) continue;

        const URegionEmb* ur;
        const UPoint* up;

        mreg->Get(urPos, ur);
        mp.Get(upPos, up);

        ur->RestrictedIntersection(
            mreg->GetMSegmentData(), *up, *iv, res, pending, merge);
    }

    if (pending)
    {
        if (!((abs(pending->timeInterval.start.ToDouble()-
                pending->timeInterval.end.ToDouble()) <= 0.00001)
                && (!pending->timeInterval.lc
                || !pending->timeInterval.rc)))
        {
            res.Add(*pending);
        }

        delete pending;
    }
}

/*
3 Implementation for methods of classes declared in header file

3.1 Method MDirection of class MPointExt

Parameters:

  result: a pointer to a MReal object

Return: nothing

*/

void MPointExt::MDirection( MReal* result) const
{
    const UPoint* unitin;
    UReal uresult(true);
    bool defined;

    result->Clear();
    result->StartBulkLoad();
    for(int i=0;i<GetNoComponents();i++)
    {
        Get(i, unitin);
        uresult.a = 0.;
        uresult.b = 0.;
        uresult.c = 0.;
        uresult.r = false;
        uresult.timeInterval = unitin->timeInterval;
        uresult.c = AngleToXAxis( &(unitin->p0), &(unitin->p1), defined );
        uresult.SetDefined( defined );
        result->Add( uresult );
    }
    result->EndBulkLoad( false );
}

/*
3.2 Method Locations of class MPointExt

Parameters:

  result: a pointer to a Points object

Return: nothing

*/

void MPointExt::Locations( Points* result ) const
{
    if(0)
    {
        cout << "MPointExt::Locations called!" << endl;
    }
    const UPoint* unitin;
    vector<Point> points;
    vector<HalfSegment> hsegments;
    HalfSegment* temp_hs;
    bool contained;

    for(int i=0;i<GetNoComponents();i++)
    {
        Get(i, unitin);
        if(unitin->p0 == unitin->p1)
        {
            points.push_back(unitin->p0);
        }
        else
        {
            temp_hs = new HalfSegment( false, unitin->p0, unitin->p1);
            hsegments.push_back(*temp_hs);
        }
    }

    result->Clear();
    result->StartBulkLoad();
    for(size_t i=0;i<points.size();i++)
    {
        contained = false;
        for(size_t j=0;j<hsegments.size();j++)
        {
            if(hsegments[j].Contains(points[i]))
                contained = true;
        }
        if(!contained)
        {
            if(0)
            {
                cout << endl << "NOT CONTAINED!!!!!!!!!!!!!!!" << endl;
                cout << "x=" << points[i].GetX() << endl;
                cout << "y=" << points[i].GetY() << endl;
            }
            *result += points[i];
        }
        else
        {
            if(0)
            {
                cout << endl << "CONTAINED!!!!!!!!!!!!!!!" << endl;
                cout << "x=" << points[i].GetX() << endl;
                cout << "y=" << points[i].GetY() << endl;
            }
        }
    }
    result->EndBulkLoad( true );

}

/*
3.3 Method At of class MPointExt

Parameters:

  pts: a pointer to a Points object
  result: a reference to a MPoint object

Return: nothing

*/

void MPointExt::At( Points* pts, MPoint &result ) const
{
    const UPoint* unitin;
    clock_t clock1, clock2, clock3, clock4, clock_ges;
    double time1, time2;

    result.Clear();
    result.StartBulkLoad();
    clock1 = clock();
    clock_ges = 0;
    for(int i=0;i<GetNoComponents();i++)
    {
        clock3 = clock();
        Get(i, unitin);
        const Rectangle<3> temp_pbb = (Rectangle<3>)unitin->BoundingBox();
        Rectangle<2> unit_pbb;
        const Rectangle<2> obj_pbb = (Rectangle<2>)pts->BoundingBox();
        if(0)
        {
            cout << "Boundingbox of points: "
                << "MinD(0): " << obj_pbb.MinD(0)
                << ", MinD(1): " << obj_pbb.MinD(1)
                << ", MaxD(0): " << obj_pbb.MaxD(0)
                << ", MaxD(1): " << obj_pbb.MaxD(1) << endl;

        }
        double min[2] = { temp_pbb.MinD(0), temp_pbb.MinD(1) };
        double max[2] = { temp_pbb.MaxD(0), temp_pbb.MaxD(1) };

        unit_pbb.Set(true, min, max);
        if(unit_pbb.Intersects( obj_pbb ))
        {
            for(int j=0;j<pts->Size();j++)
            {
                const Point* tmp_pt;
                pts->Get( j, tmp_pt );
                if( unitin->Passes( *tmp_pt ) )
                {
                    if(0)
                    {
                        cout << "( " <<  tmp_pt->GetX()
                            << ", " << tmp_pt->GetY() << " ) ";
                    }
                    UPoint uresult(true);
                    if(unitin->At( *tmp_pt, uresult ))
                        result.Add( uresult );
                }
            }
        }
        clock4 = clock();
        clock_ges = clock_ges + (clock4 - clock3);
    }
    clock2 = clock();
    time2 = ((double)(clock_ges / GetNoComponents())/CLOCKS_PER_SEC) * 1000.;
    time1 = ((double)(clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << "Average computing time per unit: " << time2 << " ms/unit" << endl;
    cout << "Total computing time : " << time1 << " ms" << endl;
    result.EndBulkLoad( false );

}

/*
3.4 Method At of class MPointExt

Parameter:

  ln: a pointer to a Line object
  result: a reference to a MPoint object

Return: nothing

It returns a projection of ln on the moving point. The
method seeks for intersections between a unit point
and each HalfSegment in ln. Actually intersections in a point
are supported.

*/
void MPointExt::At( Line* ln, MPoint &result ) const
{
    cout << "MPointExt::At called" << endl;
    const UPoint* unitin;
    clock_t clock1, clock2, clock3, clock4, clock_ges;
    double time1, time2;

    result.Clear();
    result.StartBulkLoad();
    clock1 = clock();
    clock_ges = 0;
    for(int i=0;i<GetNoComponents();i++)
    {
        clock3 = clock();
        Get(i, unitin);
        const Rectangle<3> temp_pbb = (Rectangle<3>)unitin->BoundingBox();
        Rectangle<2> unit_pbb;
        const Rectangle<2> obj_pbb = (Rectangle<2>)ln->BoundingBox();
        if(1)
        {
            cout << "Boundingbox of points: "
                    << "MinD(0): " << obj_pbb.MinD(0)
                    << ", MinD(1): " << obj_pbb.MinD(1)
                    << ", MaxD(0): " << obj_pbb.MaxD(0)
                    << ", MaxD(1): " << obj_pbb.MaxD(1) << endl;

        }
        double min[2] = { temp_pbb.MinD(0), temp_pbb.MinD(1) };
        double max[2] = { temp_pbb.MaxD(0), temp_pbb.MaxD(1) };

        unit_pbb.Set(true, min, max);
/*
Raw intersection with Bounding Boxes

*/
        if(unit_pbb.Intersects( obj_pbb ))
        {
            HalfSegment up_chs;
            up_chs.Set( false, unitin->p0, unitin->p1 );
            if(1)
            {
                cout << "No of HS: " << ln->Size() << endl;
            }
            for(int j=0;j<ln->Size();j++)
            {
                const HalfSegment* ln_chs;
                ln->Get( j, ln_chs );
/*
Scanning of one 1 of 2 HalfSegment

*/
                if( ln_chs->GetRightPoint() == ln_chs->GetDomPoint() )
                {
                    if(1)
                    {
                        cout << "P0.X = " << ln_chs->GetLeftPoint().GetX()
                            << " P0.Y = " << ln_chs->GetLeftPoint().GetY()
                            << " P1.X = " << ln_chs->GetRightPoint().GetX()
                            << " P1.Y = " << ln_chs->GetRightPoint().GetY()
                            << " D0.X = " << ln_chs->GetDomPoint().GetX()
                            << " D0.Y = " << ln_chs->GetDomPoint().GetY()
                            << " S1.X = " << ln_chs->GetSecPoint().GetX()
                            << " S1.Y = " << ln_chs->GetSecPoint().GetY()
                            << endl;
                    }
/*
For unit points which do not have any motion

*/
                    if( unitin->p0 == unitin->p1 )
                    {
                        if( ln_chs->Contains( unitin->p0 ) )
                        {
                            if(1)
                            {
                                cout << "Intersects up " << i
                                    << " as a point at " << j << "!!" << endl
                                    << "HS: ( " << ln_chs->GetLeftPoint().GetX()
                                    << ", " << ln_chs->GetLeftPoint().GetY()
                                    << " ; " << ln_chs->GetRightPoint().GetX()
                                    << ", " << ln_chs->GetRightPoint().GetY()
                                    << " )" << endl;
                            }
                            UPoint res(true);
                            const UPoint* uttmp;
                            res.CopyFrom( unitin );
                            bool unit_present = false;
                            for(int z=0;z<result.GetNoComponents();z++)
                            {
                                result.Get( z, uttmp );
                                if( res == *uttmp )
                                    unit_present = true;
                            }
                            if( !unit_present )
                                result.Add( res );
                        }
                    }
                    else
                    {
                        if( up_chs.Intersects( *ln_chs ) )
                        {
                            if(1)
                            {
                                cout << "In unit " << i << " ..." << endl;
                                cout << "( " << up_chs.GetLeftPoint().GetX()
                                        << ", "
                                        << up_chs.GetLeftPoint().GetY()
                                        << "; "
                                        << up_chs.GetRightPoint().GetX()
                                        << ", "
                                        << up_chs.GetRightPoint().GetY()
                                        << " )" << endl;
                                cout << "intersects ( "
                                        << ln_chs->GetLeftPoint().GetX()
                                        << ", "
                                        << ln_chs->GetLeftPoint().GetY()
                                        << "; "
                                        << ln_chs->GetRightPoint().GetX()
                                        << ", "
                                        << ln_chs->GetRightPoint().GetY()
                                        << " ) at " << j << endl;
                            }

                            Point inter_p;
                            HalfSegment inter_chs;

                            if( up_chs.Intersection( *ln_chs, inter_chs ) )
                            {
                                if(1)
                                {
                                    cout << "Intersection is a line!!" << endl
                                    << "inter_chs: ( "
                                            << ln_chs->GetLeftPoint().GetX()
                                            << ", "
                                            << ln_chs->GetLeftPoint().GetY()
                                            << "; "
                                            << ln_chs->GetRightPoint().GetX()
                                            << ", "
                                            << ln_chs->GetRightPoint().GetY()
                                            << " )" << endl;
                                }
                                UPoint trash1(true);
                                UPoint trash2(true);

                                unitin->At( ln_chs->GetLeftPoint(), trash1 );
                                unitin->At( ln_chs->GetRightPoint(), trash2 );
                                bool inv_def = true, ls = true, rs = true;

                                if(!trash1.timeInterval.lc
                                    && !trash1.timeInterval.rc)
                                    inv_def = false;

                                if(!trash2.timeInterval.rc
                                    && trash2.timeInterval.lc)
                                    inv_def = false;


                                if(inv_def
                                   && trash1.timeInterval.lc
                                   && trash2.timeInterval.rc)
                                {

                                    if(!trash1.timeInterval.lc)
                                        ls = false;

                                    if(!trash2.timeInterval.rc)
                                        rs = false;
                                    Interval<Instant>* ii =
                                        new Interval<Instant>(
                                        !ls
                                        ? trash1.timeInterval.end
                                        : trash1.timeInterval.start,
                                        !rs
                                        ? trash2.timeInterval.start
                                        : trash2.timeInterval.end,
                                        true, true
                                    );

                                    UPoint* res = new UPoint(
                                        *ii,
                                        ln_chs->GetLeftPoint(),
                                        ln_chs->GetRightPoint()
                                    );
                                    if(1)
                                    {
                                        cout << "UPoint created!" << endl
                                        << "P0.X: " << res->p0.GetX() << endl
                                        << "P0.Y: " << res->p0.GetY() << endl
                                        << "P1.X: " << res->p1.GetX() << endl
                                        << "P1.Y: " << res->p1.GetY() << endl;
                                        cout << "( "
                                        << res->timeInterval.start.ToString()
                                        << " "
                                        << res->timeInterval.end.ToString()
                                        << " )" << endl;
                                    }
                                    result.Add( *res );
                                }
                            }
                            else
                            {
/*
Looks for intersections in a point

*/
                                if( up_chs.Intersection( *ln_chs, inter_p ) )
                                {
                                    if(1)
                                    {
                                        cout << "Intersection is a point!!"
                                        << endl;
                                    }
                                    UPoint res(true);
                                    unitin->At( inter_p, res );
                                    result.Add( res );
                                }
                            }
                        }
                    }
                }
            }
        }
        clock4 = clock();
        clock_ges = clock_ges + (clock4 - clock3);
    }
    clock2 = clock();
    time2 = ((double)(clock_ges / GetNoComponents())/CLOCKS_PER_SEC) * 1000.;
    time1 = ((double)(clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << "Average computing time per unit: " << time2 << " ms/unit" << endl;
    cout << "Total computing time : " << time1 << " ms" << endl;
    result.EndBulkLoad( false );
}

/*
3.5 Method Passes of class MPointExt

Parameters:

  pts: a pointer to a Points object

Return: a boolean

*/

bool MPointExt::Passes( Points* pts ) const
{
    const UPoint* unitin;
    clock_t clock1, clock2, clock3, clock4, clock_ges;
    double time1, time2;
    bool result = false;

    clock1 = clock();
    clock_ges = 0;
    int i = 0;
    while( !result && i<GetNoComponents() )
    {
        clock3 = clock();
        Get(i, unitin);
        const Rectangle<3> temp_pbb = (Rectangle<3>)unitin->BoundingBox();
        Rectangle<2> unit_pbb;
        const Rectangle<2> obj_pbb = (Rectangle<2>)pts->BoundingBox();
        if(0)
        {
            cout << "Boundingbox of points: "
                    << "MinD(0): " << obj_pbb.MinD(0)
                    << ", MinD(1): " << obj_pbb.MinD(1)
                    << ", MaxD(0): " << obj_pbb.MaxD(0)
                    << ", MaxD(1): " << obj_pbb.MaxD(1) << endl;

        }
        double min[2] = { temp_pbb.MinD(0), temp_pbb.MinD(1) };
        double max[2] = { temp_pbb.MaxD(0), temp_pbb.MaxD(1) };

        unit_pbb.Set(true, min, max);
        if(unit_pbb.Intersects( obj_pbb ))
        {
            int j = 0;
            while( !result && j<pts->Size() )
            {
                const Point* tmp_pt;
                pts->Get( j, tmp_pt );
                if( unitin->Passes( *tmp_pt ) )
                {
                    if(0)
                    {
                        cout << "( " <<  tmp_pt->GetX()
                                << ", " << tmp_pt->GetY() << " ) ";
                    }
                    result = true;
                }
                j++;
            }
        }

        clock4 = clock();
        clock_ges = clock_ges + (clock4 - clock3);
        i++;
    }
    clock2 = clock();
    time2 = ((double)(clock_ges / GetNoComponents())/CLOCKS_PER_SEC) * 1000.;
    time1 = ((double)(clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << "Average computing time per unit: " << time2 << " ms/unit" << endl;
    cout << "Total computing time : " << time1 << " ms" << endl;

    return result;
}

/*
3.6 Method Passes of class MPointExt

Parameters:

  ln: a pointer to a Line object

Return: a boolean

*/

bool MPointExt::Passes( Line* ln ) const
{
    const UPoint* unitin;
    clock_t clock1, clock2, clock3, clock4, clock_ges;
    double time1, time2;
    bool result = false;

    clock1 = clock();
    clock_ges = 0;
    int i = 0;
    while( !result && i<GetNoComponents() )
    {
        clock3 = clock();
        Get(i, unitin);
        const Rectangle<3> temp_pbb = (Rectangle<3>)unitin->BoundingBox();
        Rectangle<2> unit_pbb;
        const Rectangle<2> obj_pbb = (Rectangle<2>)ln->BoundingBox();
        if(0)
        {
            cout << "Boundingbox of points: "
                    << "MinD(0): " << obj_pbb.MinD(0)
                    << ", MinD(1): " << obj_pbb.MinD(1)
                    << ", MaxD(0): " << obj_pbb.MaxD(0)
                    << ", MaxD(1): " << obj_pbb.MaxD(1) << endl;

        }
        double min[2] = { temp_pbb.MinD(0), temp_pbb.MinD(1) };
        double max[2] = { temp_pbb.MaxD(0), temp_pbb.MaxD(1) };

        unit_pbb.Set(true, min, max);
/*
Raw intersection with Bounding Boxes ...

*/
        if(unit_pbb.Intersects( obj_pbb ))
        {
            HalfSegment up_chs;
            up_chs.Set( false, unitin->p0, unitin->p1 );
            if(0)
            {
                cout << "No of HS: " << ln->Size() << endl;
            }

            int j = 0;
            while( !result && j<ln->Size() )
            {
                const HalfSegment* ln_chs;
                ln->Get( j, ln_chs );
/*
Scanning of one 1 of 2 HalfSegment

*/
                if( ln_chs->GetRightPoint() == ln_chs->GetDomPoint() )
                {
                    if(0)
                    {
                        cout << "P0.X = " << ln_chs->GetLeftPoint().GetX()
                                << " P0.Y = " << ln_chs->GetLeftPoint().GetY()
                                << " P1.X = " << ln_chs->GetRightPoint().GetX()
                                << " P1.Y = " << ln_chs->GetRightPoint().GetY()
                                << " D0.X = " << ln_chs->GetDomPoint().GetX()
                                << " D0.Y = " << ln_chs->GetDomPoint().GetY()
                                << " S1.X = " << ln_chs->GetSecPoint().GetX()
                                << " S1.Y = " << ln_chs->GetSecPoint().GetY()
                                << endl;
                    }
/*
For unit points which do not have any motion

*/
                    if( unitin->p0 == unitin->p1 )
                    {
                        if( ln_chs->Contains( unitin->p0 ) )
                        {
                            if(0)
                            {
                                cout << "Intersects up " << i
                                     << " as a point at " << j << "!!" << endl
                                     << "HS: ( "
                                     << ln_chs->GetLeftPoint().GetX()
                                     << ", " << ln_chs->GetLeftPoint().GetY()
                                     << " ; " << ln_chs->GetRightPoint().GetX()
                                     << ", " << ln_chs->GetRightPoint().GetY()
                                     << " )" << endl;
                            }
                            result = true;
                        }
                    }
                    else
                    {
                        Point inter_p;
                        HalfSegment inter_chs;
/*
Looks for intersections in a point inter\_p

*/
                        if( up_chs.Intersection( *ln_chs, inter_p ) )
                        {
                            if(0)
                            {
                                cout << "In unit " << i << " ..." << endl;
                                cout << "( " << up_chs.GetLeftPoint().GetX()
                                        << ", "
                                        << up_chs.GetLeftPoint().GetY()
                                        << "; "
                                        << up_chs.GetRightPoint().GetX()
                                        << ", "
                                        << up_chs.GetRightPoint().GetY()
                                        << " )" << endl;
                                cout << "intersects ( "
                                        << ln_chs->GetLeftPoint().GetX()
                                        << ", "
                                        << ln_chs->GetLeftPoint().GetY()
                                        << "; "
                                        << ln_chs->GetRightPoint().GetX()
                                        << ", "
                                        << ln_chs->GetRightPoint().GetY()
                                        << " ) at " << j << endl;
                            }
                            result = true;
                        }
                    }
                }
                j++;
            }
        }
        clock4 = clock();
        clock_ges = clock_ges + (clock4 - clock3);
        i++;
    }
    clock2 = clock();
    time2 = ((double)(clock_ges / GetNoComponents())/CLOCKS_PER_SEC) * 1000.;
    time1 = ((double)(clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << "Average computing time per unit: " << time2 << " ms/unit" << endl;
    cout << "Total computing time : " << time1 << " ms" << endl;

    return result;
}

/*
3.7 Method AtMin of class MappingExt

Parameters:

  result: a reference to a Mapping<Unit, Alpha> object

Return: nothing

*/

template <class Unit, class Alpha>
void MappingExt<Unit, Alpha>::AtMin( Mapping<Unit, Alpha> &result ) const
{
    const Unit* unit;
    int compRes = 0;
    const Unit *umin;

    result.Clear();
    result.StartBulkLoad();

    Get(0, unit);
    result.Add( *unit );
    umin = unit;

    for(int i=1;i<Mapping<Unit, Alpha>::GetNoComponents();i++)
    {
      Get(i, unit);
      compRes = (unit->constValue).Compare( &(umin->constValue) );
      if(compRes > 0)
        continue;
      if(compRes == 0)
        {
          result.Add( *unit );
          continue;
        }
      if(compRes < 0)
        {
          result.Clear();
          result.Add( *unit );
          umin = unit;
        }
    }
    result.EndBulkLoad( false );
}

/*
3.7 Method AtMin of class MappingExt

Parameters:

  result: a reference to a Mapping<Unit, Alpha> object

Return: nothing

*/

template <class Unit, class Alpha>
void MappingExt<Unit, Alpha>::AtMax( Mapping<Unit, Alpha> &result ) const
{
    const Unit* unit;
    int compRes = 0;
    const Unit *umin;

    result.Clear();
    result.StartBulkLoad();

    Get(0, unit);
    result.Add( *unit );
    umin = unit;

    for(int i=1;i<Mapping<Unit, Alpha>::GetNoComponents();i++)
    {
      Get(i, unit);
      compRes = (unit->constValue).Compare( &(umin->constValue) );
      if(compRes < 0)
        continue;
      if(compRes == 0)
        {
          result.Add( *unit );
          continue;
        }
      if(compRes > 0)
        {
          result.Clear();
          result.Add( *unit );
          umin = unit;
        }
    }
    result.EndBulkLoad( false );
}

/*
3.8 Method AtMin of class MRealExt

Parameters:

  result: a reference to a MReal object

Return: nothing

*/

void MRealExt::AtMin( MReal &result ) const
{
    const UReal* utemp;
    const UReal* uresult;
    double min;
    int unit_num;
    double unit_min, unit_max;

    Get(0, utemp);
    MinMaxValueFunction(utemp, unit_min, unit_max);
    ((URealExt*)utemp)->SetUnitMin( unit_min );
    ((URealExt*)utemp)->SetUnitMax( unit_max );
    min = ((URealExt*)utemp)->GetUnitMax();
    unit_num = 0;

    if(0)
        cout << "GetUnitMax(): " << ((URealExt*)utemp)->GetUnitMax() << endl;

    for(int i=1;i<GetNoComponents();i++)
    {
        Get(i, utemp);
        MinMaxValueFunction(utemp, unit_min, unit_max);
        ((URealExt*)utemp)->SetUnitMin( unit_min );
        ((URealExt*)utemp)->SetUnitMax( unit_max );
        if(0)
        {
            cout << "GetUnitMin(): " << ((URealExt*)utemp)->GetUnitMin();
            cout << endl;
        }
        if(((URealExt*)utemp)->GetUnitMin() < min)
        {
            min = ((URealExt*)utemp)->GetUnitMin();
            unit_num = i;
        }
    }
    // ATTENTION! This seems to be wrong. A moving object can reach its extrema
    // several times! To return a single unit is NOT correct! Instead, one has
    // to find ALL units with the appropriate extremum, restrict their deftimes
    // to the interval, where they take the extreme value and add them to the
    // result!
    Get( unit_num, uresult );

    result.Clear();
    result.StartBulkLoad();
    result.Add( *uresult );
    result.EndBulkLoad( false );
}

/*
3.9 Method AtMax of class MRealExt

Parameters:

  result: a reference to a MReal object

Return: nothing

*/

void MRealExt::AtMax( MReal &result ) const
{
    const UReal* utemp;
    const UReal* uresult;
    double max;
    int unit_num;
    double unit_min, unit_max;

    Get(0, utemp);
    MinMaxValueFunction(utemp, unit_min, unit_max);
    ((URealExt*)utemp)->SetUnitMin( unit_min );
    ((URealExt*)utemp)->SetUnitMax( unit_max );
    max = ((URealExt*)utemp)->GetUnitMax();
    unit_num = 0;

    if(0)
        cout << "GetUnitMax(): " << ((URealExt*)utemp)->GetUnitMax() << endl;

    for(int i=1;i<GetNoComponents();i++)
    {
        Get(i, utemp);
        MinMaxValueFunction(utemp, unit_min, unit_max);
        ((URealExt*)utemp)->SetUnitMin( unit_min );
        ((URealExt*)utemp)->SetUnitMax( unit_max );
        if(0)
        {
            cout << "GetUnitMax(): " << ((URealExt*)utemp)->GetUnitMax();
            cout << endl;
        }
        if(((URealExt*)utemp)->GetUnitMax() > max)
        {
            max = ((URealExt*)utemp)->GetUnitMax();
            unit_num = i;
        }
    }
    // ATTENTION! This seems to be wrong. A moving object can reach its extrema
    // several times! To return a single unit is NOT correct! Instead, one has
    // to find ALL units with the appropriate extremum, restrict their deftimes
    // to the interval, where they take the extreme value and add them to the
    // result!
    Get( unit_num, uresult );

    result.Clear();
    result.StartBulkLoad();
    result.Add( *uresult );
    result.EndBulkLoad( false );
}

/*
3.10 Method AtMin of class MRealExt

Parameters:

  val: a CcReal object
  result: a reference to a MReal object

Return: nothing

*/

void MRealExt::At( CcReal val, MReal &result ) const
{
    const UReal* utemp;
    UReal uresult(true);
    double unit_min, unit_max, time1, time2;
    double value = val.GetRealval();
    clock_t clock1, clock2, clock3, clock4, clock_ges;

    result.Clear();
    result.StartBulkLoad();
    clock1 = clock();
    clock_ges = 0;
    for(int i=0;i<GetNoComponents();i++)
    {
        clock3 = clock();
        Get(i, utemp);
        MinMaxValueFunction(utemp, unit_min, unit_max);
        ((URealExt*)utemp)->SetUnitMin( unit_min );
        ((URealExt*)utemp)->SetUnitMax( unit_max );
        double t_value;
        if(timeIntervalOfRealInUReal(value, utemp, t_value))
        {
            uresult.a = utemp->a;
            uresult.b = utemp->b;
            uresult.c = utemp->c;
            uresult.r = utemp->r;
            if(utemp->a == 0 && utemp->b == 0)
            {
                uresult.timeInterval = utemp->timeInterval;
            }
            else
            {
                uresult.timeInterval = utemp->timeInterval;
                uresult.timeInterval.start.ReadFrom(
                        (const double)(t_value - 0.0000001) );
                uresult.timeInterval.end.ReadFrom(
                        (const double)(t_value + 0.0000001) );
            }
            result.Add( uresult );
        }
        clock4 = clock();
        clock_ges = clock_ges + (clock4 - clock3);
    }
    clock2 = clock();
    time2 = ((double)(clock_ges / GetNoComponents())/CLOCKS_PER_SEC) * 1000.;
    time1 = ((double)(clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << "Average computing time per unit: " << time2 << " ms/unit" << endl;
    cout << "Total computing time : " << time1 << " ms" << endl;
    result.EndBulkLoad( false );
}

/*
3.11 Method Passes of class MRealExt

Parameters:

  val: a CcReal object

Return: a boolean

*/

bool MRealExt::Passes( CcReal val ) const
{
    const UReal* utemp;
    UReal uresult(true);
    double unit_min, unit_max, time1, time2;
    double value = val.GetRealval();
    clock_t clock1, clock2, clock3, clock4, clock_ges;
    bool result = false;

    clock1 = clock();
    clock_ges = 0;
    for(int i=0;i<GetNoComponents();i++)
    {
        clock3 = clock();
        Get(i, utemp);
        MinMaxValueFunction(utemp, unit_min, unit_max);
        ((URealExt*)utemp)->SetUnitMin( unit_min );
        ((URealExt*)utemp)->SetUnitMax( unit_max );
        bool lh = utemp->timeInterval.lc;
        bool rh = utemp->timeInterval.rc;
        if( !((!lh && !rh && ( value <= unit_min || value >= unit_max ) ) ||
            (lh && rh && ( value < unit_min || value > unit_max) ) ||
            (!lh && rh && ( value <= unit_min || value > unit_max ) ) ||
            (lh && !rh && ( value < unit_min || value >= unit_max ) )
            ) )
        {
            result = true;
            break;
        }
        clock4 = clock();
        clock_ges = clock_ges + (clock4 - clock3);
    }
    clock2 = clock();
    time2 = ((double)(clock_ges / GetNoComponents())/CLOCKS_PER_SEC) * 1000.;
    time1 = ((double)(clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << "Average computing time per unit: " << time2 << " ms/unit" << endl;
    cout << "Total computing time : " << time1 << " ms" << endl;

    return result;
}

/*
3.12 Method At of class MappingExt

Parameters:

  inv: a pointer to a Range<Alpha> object
  result: a reference to a Mapping<Unit, Alpha> object

Return: nothing

*/

template <class Unit, class Alpha>
void MappingExt<Unit, Alpha>::At(
    Range<Alpha>* inv,
    Mapping<Unit, Alpha> &result
) const
{
    const Unit* utemp;
    bool is_valid = true;
    const Interval<Alpha> *lastInterval, *interval;

    if( !inv->IsOrdered() )
        is_valid = false;

    if( inv->GetNoComponents() == 1 && is_valid)
    {
        inv->Get( 0, interval );
        is_valid = interval->IsValid();
    }
    else
    {

        if(is_valid)
        {
/*
This implementation was token from method Range::IsValid() which is
private. The method Range::Contains() has an assert(!IsValid()) what
causes a unesthetic termination of DB-UI.

*/
            for( int i = 1; i < inv->GetNoComponents(); i++ )
            {
                inv->Get( i-1, lastInterval );
                if( !lastInterval->IsValid() )
                {
                    is_valid = false;
                    break;
                }
                inv->Get( i, interval );
                if( !interval->IsValid() )
                {
                    is_valid = false;
                    break;
                }
                if( (!lastInterval->Disjoint( *interval )) &&
                    (!lastInterval->Adjacent( *interval )) )
                {
                    is_valid = false;
                    break;
                }
            }
        }
    }

    if(is_valid)
    {
        result.Clear();
        result.StartBulkLoad();
        for(int i=0;i<Mapping<Unit, Alpha>::GetNoComponents();i++)
        {
            Get(i, utemp);
            if(inv->Contains(utemp->constValue))
                result.Add( *utemp );
        }
        result.EndBulkLoad( false );
    }
    else
        cout << "Invalid Range!!" << endl;

}

/*
3.13 Method At of class MappingExt

Parameters:

  inv: a pointer to a RReal object
  result: a reference to a Mapping<Unit, Alpha> object

Return: nothing

*/

void MRealExt::At( RReal* inv, MReal &result ) const
{
    const UReal* utemp;
    UReal uresult(true);
    double unit_min, unit_max;

    result.Clear();
    result.StartBulkLoad();
    for( int i=0; i<GetNoComponents(); i++ )
    {
        Get( i, utemp );
        MinMaxValueFunction( utemp, unit_min, unit_max );
        ((URealExt*)utemp)->SetUnitMin( unit_min );
        ((URealExt*)utemp)->SetUnitMax( unit_max );
        Interval<CcReal> minmax;
        minmax.start.Set( unit_min );
        minmax.end.Set( unit_max );
        minmax.lc = true;
        minmax.rc = true;
        for(int j=0;j<inv->GetNoComponents();j++)
        {
            const Interval<CcReal>* inv_tmp;
            Interval<CcReal> inv_result;
            inv->Get( j, inv_tmp);
            if(minmax.Intersects( *inv_tmp ))
            {
                minmax.Intersection( *inv_tmp, inv_result );
                if(1)
                {
                    cout << "(min max)[ " << minmax.start.GetValue()
                        << ", " << minmax.end.GetValue() << " ] "
                        << " intersects "
                        << "[ " << inv_tmp->start.GetValue()
                        << ", " << inv_tmp->end.GetValue() << " ]";
                    cout << " = [ " << inv_result.start.GetValue()
                        << ", " << inv_result.end.GetValue() << " ] " << endl;
                }
                double t_value, t_value2;
                uresult.a = utemp->a;
                uresult.b = utemp->b;
                uresult.c = utemp->c;
                uresult.r = utemp->r;
                if(utemp->a == 0 && utemp->b == 0)
                {
                    uresult.timeInterval = utemp->timeInterval;
                    result.Add( uresult );
                }
                else
                {
                    if( timeIntervalOfRealInUReal(
                        inv_result.start.GetValue(),
                        utemp,
                        t_value) )
                    {
                        if( timeIntervalOfRealInUReal(
                            inv_result.end.GetValue(),
                            utemp,
                            t_value2) )
                        {
                            uresult.timeInterval = utemp->timeInterval;
                            if( t_value < t_value2 )
                            {
                                uresult.timeInterval.start.ReadFrom(
                                    (const double)t_value );
                                uresult.timeInterval.end.ReadFrom(
                                    (const double)t_value2 );
                            }
                            else
                            {
                                uresult.timeInterval.start.ReadFrom(
                                    (const double)t_value2 );
                                uresult.timeInterval.end.ReadFrom(
                                    (const double)t_value );
                            }
                            result.Add( uresult );
                        }
                    }
                }
            }
        }
    }
    result.EndBulkLoad( false );
}

/*
3.14 Method AtPeriods of class MRegion

Parameters:

  per: a pointer to a Periods object
  mregparam: a pointer to a Mapping<Unit, Alpha> object

Return: nothing

NOTE: The declaration of this method is placed on MovingRegion.h

*/

void MRegion::AtPeriods(Periods* per, MRegion* mregparam)
{
    const URegionEmb* utemp;
    const Interval<Instant>* temp2;
    Interval<Instant> temp3;
    const MSegmentData* oldsmg;
    MSegmentData newsmg;
    vector<GroupOfIntervals> temp_intervals;
    GroupOfIntervals inter_temp;
    vector<USegments> tempsgms;
    USegments usegtemp;
    const DBArray<MSegmentData>* oldsgms;
    unsigned int starting_segments_pos;
    URegionEmb* out_ureg;

    for(int i=0;i<mregparam->GetNoComponents();i++)
    {
        mregparam->Get(i, utemp);
        for(int j=0;j<per->GetNoComponents();j++)
        {
            per->Get(j, temp2);
            if((utemp->timeInterval).Intersects(*temp2))
            {
                (utemp->timeInterval).Intersection(*temp2, temp3);
                inter_temp.unit_nr = i;
                (inter_temp.str_inst).CopyFrom(temp3);
                temp_intervals.push_back(inter_temp);
            }
        }
    }
    if(1)
    {
        for(unsigned int i=0;i<temp_intervals.size();i++)
        {
            cout << endl << "Interval:" << endl;
            cout << "unit nr.: " << temp_intervals[i].unit_nr << endl;
            cout << "interval start: "
                << ((temp_intervals[i].str_inst).start).ToString()
                << endl;
            cout << "interval end: "
                << ((temp_intervals[i].str_inst).end).ToString()
                << endl;
        }
    }

    oldsgms = mregparam->GetMSegmentData();
    for(unsigned int i=0;i<temp_intervals.size();i++)
    {
        mregparam->Get(temp_intervals[i].unit_nr, utemp);
        usegtemp.unit_nr = i;
        for(int k=0;k<utemp->GetSegmentsNum();k++)
        {
            utemp->GetSegment(oldsgms, utemp->GetStartPos() + k, oldsmg);
            oldsmg->restrictToInterval(
                utemp->timeInterval,
                temp_intervals[i].str_inst,
                newsmg
            );
            if(0)
            {
                cout << endl << "Old Segments: Unit "
                    << temp_intervals[i].unit_nr
                    << ", Segmentnr. " << k << ": ["
                    << oldsmg->GetInitialStartX() << " , "
                    << oldsmg->GetInitialStartY() << " ; "
                    << oldsmg->GetInitialEndX() << " , "
                    << oldsmg->GetInitialEndY() << " ; "
                    << oldsmg->GetFinalStartX() << " , "
                    << oldsmg->GetFinalStartY() << " ; "
                    << oldsmg->GetFinalEndX() << " , "
                    << oldsmg->GetFinalEndY()
                    << "]" << endl;
            }
            (usegtemp.unit_interval).start =
                (temp_intervals[i].str_inst).start;
            (usegtemp.unit_interval).end = (temp_intervals[i].str_inst).end;
            (usegtemp.unit_interval).lc = (temp_intervals[i].str_inst).lc;
            (usegtemp.unit_interval).rc = (temp_intervals[i].str_inst).rc;
            (usegtemp.sgms).push_back(newsmg);
        }
        tempsgms.push_back(usegtemp);
        usegtemp.unit_nr = 0;
        (usegtemp.sgms).clear();
    }

    if(0)
    {
        starting_segments_pos = 0;
        for(unsigned int i=0;i<tempsgms.size();i++)
        {
            cout << endl << "Unit Nr.: " << tempsgms[i].unit_nr << endl;
            cout << endl << "starting_segments_pos: ";
            cout << starting_segments_pos << endl;
            cout << "interval start: ";
            cout << ((tempsgms[i].unit_interval).start).ToString() << endl;
            cout << "interval end: ";
            cout << ((tempsgms[i].unit_interval).end).ToString() << endl;
            for(unsigned int j=0;j<(tempsgms[i].sgms).size();j++)
            {
                cout << endl << j << ": ["
                    << (tempsgms[i].sgms)[j].GetInitialStartX() << " , "
                    << (tempsgms[i].sgms)[j].GetInitialStartY() << " ; "
                    << (tempsgms[i].sgms)[j].GetInitialEndX() << " , "
                    << (tempsgms[i].sgms)[j].GetInitialEndY() << " ; "
                    << (tempsgms[i].sgms)[j].GetFinalStartX() << " , "
                    << (tempsgms[i].sgms)[j].GetFinalStartY() << " ; "
                    << (tempsgms[i].sgms)[j].GetFinalEndX() << " , "
                    << (tempsgms[i].sgms)[j].GetFinalEndY()
                    << "]" << endl;
                cout << "GetFaceNo: "
                    << (tempsgms[i].sgms)[j].GetFaceNo()
                    << endl;
                cout << "GetCycleNo: "
                    << (tempsgms[i].sgms)[j].GetCycleNo()
                    << endl;
                cout << "GetSegmentNo: "
                    << (tempsgms[i].sgms)[j].GetSegmentNo()
                    << endl;
                starting_segments_pos++;
            }
        }
    }

    starting_segments_pos = 0;
    Clear();
    StartBulkLoad();
    for(unsigned int i=0;i<tempsgms.size();i++)
    {
        out_ureg = new URegionEmb(
            tempsgms[i].unit_interval,
            starting_segments_pos
        );
        for(unsigned int j=0;j<(tempsgms[i].sgms).size();j++)
        {
            MSegmentData dms = (tempsgms[i].sgms)[j];
            Rectangle<3> bbox = (Rectangle<3>)out_ureg->BoundingBox();
            if (bbox.IsDefined())
            {
                double min[3] = { bbox.MinD(0), bbox.MinD(1), bbox.MinD(2) };
                double max[3] = { bbox.MaxD(0), bbox.MaxD(1), bbox.MaxD(2) };
                if (dms.GetInitialStartX() < min[0])
                    min[0] = dms.GetInitialStartX();
                if (dms.GetFinalStartX() < min[0])
                    min[0] = dms.GetFinalStartX();
                if (dms.GetInitialStartY() < min[1])
                    min[1] = dms.GetInitialStartY();
                if (dms.GetFinalStartY() < min[1])
                    min[1] = dms.GetFinalStartY();
                if (dms.GetInitialStartX() > max[0])
                    max[0] = dms.GetInitialStartX();
                if (dms.GetFinalStartX() > max[0])
                    max[0] = dms.GetFinalStartX();
                if (dms.GetInitialStartY() > max[1])
                    max[1] = dms.GetInitialStartY();
                if (dms.GetFinalStartY() > max[1])
                    max[1] = dms.GetFinalStartY();
                bbox.Set(true, min, max);
                if(0)
                {
                    cout << "bbox was defined, just update it!!" << endl;
                    cout << "bbox.MinD0 : " << min[0] << endl;
                    cout << "bbox.MinD1 : " << min[1] << endl;
                    cout << "bbox.MinD2 : " << min[2] << endl;
                    cout << "bbox.MaxD0 : " << max[0] << endl;
                    cout << "bbox.MaxD1 : " << max[1] << endl;
                    cout << "bbox.MaxD2 : " << max[2] << endl;
                }
            }
            else
            {
                double min[3] = { dms.GetInitialStartX() < dms.GetFinalStartX()
                    ? dms.GetInitialStartX() : dms.GetFinalStartX(),
                    dms.GetInitialStartY() < dms.GetFinalStartY()
                    ? dms.GetInitialStartY() : dms.GetFinalStartY(),
                    out_ureg->timeInterval.start.ToDouble() };
                double max[3] = { dms.GetInitialStartX() > dms.GetFinalStartX()
                    ? dms.GetInitialStartX() : dms.GetFinalStartX(),
                    dms.GetInitialStartY() > dms.GetFinalStartY()
                    ? dms.GetInitialStartY() : dms.GetFinalStartY(),
                    out_ureg->timeInterval.end.ToDouble() };
                bbox.Set(true, min, max);
                if(0)
                {
                    cout << "bbox was not defined!!" << endl;
                    cout << "bbox.MinD0 : " << min[0] << endl;
                    cout << "bbox.MinD1 : " << min[1] << endl;
                    cout << "bbox.MinD2 : " << min[2] << endl;
                    cout << "bbox.MaxD0 : " << max[0] << endl;
                    cout << "bbox.MaxD1 : " << max[1] << endl;
                    cout << "bbox.MaxD2 : " << max[2] << endl;
                }
            }
            out_ureg->PutSegment(
                &msegmentdata,
                j,
                dms,
                true
            );
            out_ureg->SetBBox(bbox);
            starting_segments_pos++;
        }
        if(0)
        {
            cout << endl << "GetSegmentsNum(): "
                << out_ureg->GetSegmentsNum()
                << endl;
            cout << "GetStartPos(): "
                << out_ureg->GetStartPos()
                << endl << endl;
        }

        Add(*out_ureg);
    }
    EndBulkLoad( false );

    const MSegmentData* tmp_dms;
    if(0)
    {
        cout << "Size of DBArray: " << msegmentdata.Size() << endl;
        for(int i=0;i<msegmentdata.Size();i++)
        {
            msegmentdata.Get(i, tmp_dms);
            cout << endl << i << ": ["
                << tmp_dms->GetInitialStartX() << " , "
                << tmp_dms->GetInitialStartY() << " ; "
                << tmp_dms->GetInitialEndX() << " , "
                << tmp_dms->GetInitialEndY() << " ; "
                << tmp_dms->GetFinalStartX() << " , "
                << tmp_dms->GetFinalStartY() << " ; "
                << tmp_dms->GetFinalEndX() << " , "
                << tmp_dms->GetFinalEndY()
                << "]" << endl;
            cout << "GetFaceNo: " << tmp_dms->GetFaceNo() << endl;
            cout << "GetCycleNo: " << tmp_dms->GetCycleNo() << endl;
            cout << "GetSegmentNo: " << tmp_dms->GetSegmentNo() << endl;
        }
    }
}

/*
4 Auxiliary Functions

4.1 Aux. Function ~CheckURealDerivable~

  Parameter:
    unit: a pointer t a UReal object

  Return: a boolean

*/
bool CheckURealDerivable(const UReal* unit)
{
    UReal* tmp_unit = (UReal*)unit;
    return tmp_unit->r;
}


/*

4 Type Constructor ~istring~

Type ~istring~ represents an (instant, value)-pair of strings.

The list representation of an ~istring~ is

----    ( t string-value )
----

For example:

----    ( (instant 1.0) "My String" )
----

4.1 function Describing the Signature of the Type Constructor

*/
ListExpr
IntimeStringProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(istring) "),
                             nl->StringAtom("(instant string-value)"),
                             nl->StringAtom("((instant 0.5) My String)"))));
}

/*
4.2 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckIntimeString( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "istring" ));
}

/*
4.3 Creation of the type constructor ~istring~

*/
TypeConstructor intimestring(
        "istring",  //name
        IntimeStringProperty,   //property function describing signature
        OutIntime<CcString, OutCcString>,
        InIntime<CcString, InCcString>,     //Out and In functions
        0,
        0,  //SaveToList and RestoreFromList functions
        CreateIntime<CcString>,
        DeleteIntime<CcString>, //object creation and deletion
        0,
        0,  // object open and save
        CloseIntime<CcString>,
        CloneIntime<CcString>,  //object close and clone
        CastIntime<CcString>,   //cast function
        SizeOfIntime<CcString>, //sizeof function
        CheckIntimeString       //kind checking function
);

/*
5 Type Constructor ~ustring~

Type ~ustring~ represents an (tinterval, stringvalue)-pair.

5.1 List Representation

The list representation of an ~ustring~ is

----    ( timeinterval string-value )
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   ["]My String["] )
----

5.2 function Describing the Signature of the Type Constructor

*/
ListExpr
UStringProperty()
{
    return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
    nl->FourElemList(nl->StringAtom("-> UNIT"),
                     nl->StringAtom("(ustring) "),
                     nl->StringAtom("(timeInterval string) "),
                     nl->StringAtom("((i1 i2 FALSE FALSE) My String)"))));
}

/*
5.3 Kind Checking Function

*/
bool
CheckUString( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "ustring" ));
}

/*
5.4 Creation of the type constructor ~uint~

*/
TypeConstructor unitstring(
        "ustring",  //name
        UStringProperty,    //property function describing signature
        OutConstTemporalUnit<CcString, OutCcString>,
        InConstTemporalUnit<CcString, InCcString>,  //Out and In functions
        0,
        0,  //SaveToList and RestoreFromList functions
        CreateConstTemporalUnit<CcString>,
        DeleteConstTemporalUnit<CcString>,  //object creation and deletion
        0,
        0,  // object open and save
        CloseConstTemporalUnit<CcString>,
        CloneConstTemporalUnit<CcString>,   //object close and clone
        CastConstTemporalUnit<CcString>,    //cast function
        SizeOfConstTemporalUnit<CcString>,  //sizeof function
        CheckUString    //kind checking function
);

/*
6 Type Constructor ~mstring~

Type ~mstring~ represents a moving string.

6.1 List Representation

The list representation of a ~mstring~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~ustring~.

For example:

----    (
          ( (instant 6.37)  (instant 9.9)   TRUE FALSE) ["]String 1["] )
          ( (instant 11.4)  (instant 13.9)  FALSE FALSE) ["]String 2["] )
        )
----

6.2 function Describing the Signature of the Type Constructor

*/
ListExpr
MStringProperty()
{
    return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
    nl->FourElemList(nl->StringAtom("-> MAPPING"),
                     nl->StringAtom("(mstring) "),
                     nl->StringAtom("( u1 ... un)"),
                     nl->StringAtom("(((i1 i2 TRUE TRUE) My String) ...)"))));
}

/*
6.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckMString( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "mstring" ));
}

/*
6.4 Creation of the type constructor ~mstring~

*/
TypeConstructor movingstring(
    "mstring",  //name
    MStringProperty,    //property function describing signature
    //Out and In functions
    OutMapping<MString, UString, OutConstTemporalUnit<CcString, OutCcString> >,
    InMapping<MString, UString, InConstTemporalUnit<CcString, InCcString> >,
    0,
    0,  //SaveToList and RestoreFromList functions
    CreateMapping<MString>,
    DeleteMapping<MString>,     //object creation and deletion
    0,
    0,  // object open and save
    CloseMapping<MString>,
    CloneMapping<MString>,  //object close and clone
    CastMapping<MString>,   //cast function
    SizeOfMapping<MString>, //sizeof function
    CheckMString    //kind checking function
);

/*
7 Type Constructor ~rbool~

Type ~rbool~ represents a range bool.

7.1 List Representation

The list representation of a ~rbool~ is

----    ( (bb1 eb1 lc1 rc1) (bb2 eb2 lc2 rc2) ... (bbn ebn lcn rcn) )
----


For example:

----    (
          ( (TRUE FALSE TRUE FALSE)  (FALSE FALSE TRUE TRUE) )
        )
----

7.2 function Describing the Signature of the Type Constructor

*/
ListExpr
RangeBoolProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,
  "lci means left closed interval, rci respectively right closed interval,"
    " e.g. (TRUE TRUE TRUE FALSE) means the range [TRUE, TRUE[");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> RANGE"),
                             nl->StringAtom("(rbool) "),
        nl->StringAtom("((bb1 eb1 lci rci) ... (bbn ebn lci rci))"),
        nl->StringAtom("((TRUE TRUE TRUE FALSE) (FALSE FALSE TRUE TRUE))"),
                             remarkslist)));
}

/*
7.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckRBool( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "rbool" ));
}

/*
7.4 Creation of the type constructor ~rbool~

*/
TypeConstructor rangebool(
    "rbool",  //name
     RangeBoolProperty,   //property function describing signature
     OutRange<CcBool, OutCcBool>,
     InRange<CcBool, InCcBool>,                 //Out and In functions
     0,            0,  //SaveToList and RestoreFromList functions
     CreateRange<CcBool>,DeleteRange<CcBool>,   //object creation and deletion
     OpenRange<CcBool>,  SaveRange<CcBool>,     // object open and save
     CloseRange<CcBool>, CloneRange<CcBool>,    //object close and clone
     CastRange<CcBool>,                        //cast function
     SizeOfRange<CcBool>,                      //sizeof function
     CheckRBool                          //kind checking function
);

/*
8 Type Constructor ~rstring~

Type ~rstring~ represents a range string.

8.1 List Representation

The list representation of a ~rstring~ is

----    ( (bs1 es1 lc1 rc1) (bs2 es2 lc2 rc2) ... (bsn esn lcn rcn) )
----


For example:

----    (
          ( (["]First string["] ["]Second string["] TRUE FALSE)
          (["]New York["] ["]Washington["] TRUE TRUE) )
        )
----

8.2 function Describing the Signature of the Type Constructor

*/
ListExpr
RangeStringProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,
  "lci means left closed interval, rci respectively right closed interval,"
    " e.g. (String1 String2 TRUE FALSE) means the range [String1, String2[");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> RANGE"),
                             nl->StringAtom("(rstring) "),
        nl->StringAtom("((bs1 es1 lci rci) ... (bsn esn lci rci))"),
        nl->StringAtom("((String1 String2 TRUE FALSE))"), remarkslist)));
}

/*
8.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckRString( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "rstring" ));
}

/*
8.4 Creation of the type constructor ~rstring~

*/
TypeConstructor rangestring(
    "rstring",  //name
     RangeStringProperty,   //property function describing signature
     OutRange<CcString, OutCcString>,
     InRange<CcString, InCcString>,                 //Out and In functions
     0,            0,  //SaveToList and RestoreFromList functions
     CreateRange<CcString>,DeleteRange<CcString>, //obj. creation and deletion
     OpenRange<CcString>,  SaveRange<CcString>,     // object open and save
     CloseRange<CcString>, CloneRange<CcString>,    //object close and clone
     CastRange<CcString>,                        //cast function
     SizeOfRange<CcString>,                      //sizeof function
     CheckRString                          //kind checking function
);

/*
9 Operators

9.1 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

9.1.1 Type mapping function ~MovingInstantTypeMapIntime~

It is for the operator ~atinstant~.

*/
ListExpr
MovingInstantExtTypeMapIntime( ListExpr args )
{
    if ( nl->ListLength( args ) == 2 )
    {
        ListExpr arg1 = nl->First( args ),
        arg2 = nl->Second( args );

        if( nl->IsEqual( arg2, "instant" ) )
        {
            if( nl->IsEqual( arg1, "mstring" ) )
                return nl->SymbolAtom( "istring" );
        }
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.2 Type mapping function ~MovingPeriodsTypeMapMoving~

It is for the operator ~atperiods~.

*/
ListExpr
MovingPeriodsExtTypeMapMoving( ListExpr args )
{
    if ( nl->ListLength( args ) == 2 )
    {
        ListExpr arg1 = nl->First( args ),
        arg2 = nl->Second( args );

        if( nl->IsEqual( arg2, "periods" ) )
        {
            if( nl->IsEqual( arg1, "mstring" ) )
                return nl->SymbolAtom( "mstring" );

            if( nl->IsEqual( arg1, "movingregion" ) )
                return nl->SymbolAtom( "movingregion" );
        }
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.3 Type mapping function ~MovingTypeMapeIntime~

It is for the operators ~initial~ and ~final~.

*/
ListExpr
MovingExtTypeMapIntime( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mstring" ) )
            return nl->SymbolAtom( "istring" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.4 Type mapping function ~MovingInstantPeriodsTypeMapBool~

It is for the operator ~present~.

*/
ListExpr
MovingInstantPeriodsExtTypeMapBool( ListExpr args )
{
    if ( nl->ListLength( args ) == 2 )
    {
        ListExpr arg1 = nl->First( args ),
        arg2 = nl->Second( args );

        if( nl->IsEqual( arg2, "instant" ) ||
          nl->IsEqual( arg2, "periods" ) )
        {
            if( nl->IsEqual( arg1, "mstring" ))
                return nl->SymbolAtom( "bool" );
        }
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.5 Type mapping function ~MovingBaseTypeMapBool~

It is for the operator ~at~.

*/
ListExpr
MovingBaseExtTypeMapMoving( ListExpr args )
{
    ListExpr arg1, arg2;
    if( nl->ListLength( args ) == 2 )
    {
        arg1 = nl->First( args );
        arg2 = nl->Second( args );

        if( nl->IsEqual( arg1, "mreal" )
            && nl->IsEqual( arg2, "real" ) )
            return nl->SymbolAtom( "mreal" );

        if( nl->IsEqual( arg1, "mstring" )
            && nl->IsEqual( arg2, "string" ) )
            return nl->SymbolAtom( "mstring" );

        if( nl->IsEqual( arg1, "mbool" )
            && nl->IsEqual( arg2, "rbool" ) )
            return nl->SymbolAtom( "mbool" );

        if( nl->IsEqual( arg1, "mint" )
            && nl->IsEqual( arg2, "rint" ) )
            return nl->SymbolAtom( "mint" );

        if( nl->IsEqual( arg1, "mstring" )
            && nl->IsEqual( arg2, "rstring" ) )
            return nl->SymbolAtom( "mstring" );

        if( nl->IsEqual( arg1, "mreal" )
            && nl->IsEqual( arg2, "rreal" ) )
            return nl->SymbolAtom( "mreal" );

        if( nl->IsEqual( arg1, "mpoint" )
            && nl->IsEqual( arg2, "points" ) )
            return nl->SymbolAtom( "mpoint" );

        if( nl->IsEqual( arg1, "mpoint" )
            && nl->IsEqual( arg2, "line" ) )
            return nl->SymbolAtom( "mpoint" );

        if( nl->IsEqual( arg1, "movingregion" )
            && nl->IsEqual( arg2, "point" ) )
            return nl->SymbolAtom( "mpoint" );
    }

    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.6 Type mapping function ~MovingBaseTypeMapBool~

It is for the operator ~passes~.

*/
ListExpr
MovingBaseExtTypeMapBool( ListExpr args )
{
    ListExpr arg1, arg2;
    if( nl->ListLength( args ) == 2 )
    {
        arg1 = nl->First( args );
        arg2 = nl->Second( args );

        if( (nl->IsEqual( arg1, "mreal" )
                && nl->IsEqual( arg2, "real" ))
             ||
            (nl->IsEqual( arg1, "mstring" )
                && nl->IsEqual( arg2, "string" ))
             ||
             (nl->IsEqual( arg1, "mpoint" )
                && nl->IsEqual( arg2, "points" ))
             ||
             (nl->IsEqual( arg1, "mpoint" )
                && nl->IsEqual( arg2, "line" ))
             ||
            (nl->IsEqual( arg1, "movingregion" )
                && nl->IsEqual( arg2, "point" ))
             ||
            (nl->IsEqual( arg1, "movingregion" )
                && nl->IsEqual( arg2, "points" ))
          )
            return nl->SymbolAtom( "bool" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.7 Type mapping function ~MovingTypeMapRange~

It is for the operator ~deftime~.

*/
ListExpr
MovingExtTypeMapPeriods( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mstring" ))

            return nl->SymbolAtom( "periods" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.8 Type mapping function ~IntimeTypeMapInstant~

It is for the operator ~inst~.

*/
ListExpr
IntimeExtTypeMapInstant( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "istring" ))
            return nl->SymbolAtom( "instant" );

        if( nl->IsEqual( arg1, "intimeregion" ))
            return nl->SymbolAtom( "instant" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.9 Type mapping function ~IntimeTypeMapBase~

It is for the operator ~val~.

*/
ListExpr
IntimeExtTypeMapBase( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "istring" ) )
            return nl->SymbolAtom( "string" );

        if( nl->IsEqual( arg1, "intimeregion" ) )
            return nl->SymbolAtom( "region" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.10 Type mapping function ~MovingRExtTypeMapMovingR~

It is for the operator ~derivative~.

*/
ListExpr
MovingRExtTypeMapMovingR( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mreal" ) )
            return nl->SymbolAtom( "mreal" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.11 Type mapping function ~MovingRExtTypeMapBool~

It is for the operator ~derivable~.

*/
ListExpr
MovingRExtTypeMapBool( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mreal" ) )
            return nl->SymbolAtom( "mbool" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.12 Type mapping function ~MovingPointExtTypeMapMReal~

It is for the operators ~speed~ ans ~mdirection~.

*/
ListExpr
MovingPointExtTypeMapMReal( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mpoint" ) )
            return nl->SymbolAtom( "mreal" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.13 Type mapping function RangeRangevaluesExtTypeMapRange

It is for the operator ~rangevalues~.

*/
ListExpr
RangeRangevaluesExtTypeMapRange( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mbool" ) )
            return nl->SymbolAtom( "rbool" );

        if( nl->IsEqual( arg1, "mint" ) )
            return nl->SymbolAtom( "rint" );

        if( nl->IsEqual( arg1, "mstring" ) )
            return nl->SymbolAtom( "rstring" );

        if( nl->IsEqual( arg1, "mreal" ) )
            return nl->SymbolAtom( "rreal" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.14 Type mapping function MovingSANExtTypeMap

It is for the operators ~sometimes~, ~always~ and ~never~.

*/
ListExpr
MovingSANExtTypeMap( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mbool" ) )
            return nl->SymbolAtom( "bool" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.15 Type mapping function ~MPointExtTypeMapMPoint~

It is for the operator ~velocity~.

*/
ListExpr
MPointExtTypeMapMPoint( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mpoint" ) )
            return nl->SymbolAtom( "mpoint" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.16 Type mapping function RealPhysicalUnitsExtTypeMap

It is for the operators ~setunitoftime~ and ~setunitofdistance~.

*/
ListExpr
RealPhysicalUnitsExtTypeMap( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "real" ) )
            return nl->SymbolAtom( "real" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.17 Type mapping function MovingPointExtTypeMapPoints

It is for the operator ~locations~.

*/
ListExpr
MovingPointExtTypeMapPoints( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mpoint" ) )
            return nl->SymbolAtom( "points" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.1.18 Type mapping function MovingExtTypeMapMoving

It is for the operators ~atmin~ and ~atmax~.

*/
ListExpr
MovingExtTypeMapMoving( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mint" ) )
            return nl->SymbolAtom( "mint" );
        if( nl->IsEqual( arg1, "mbool" ) )
            return nl->SymbolAtom( "mbool" );
        if( nl->IsEqual( arg1, "mstring" ) )
            return nl->SymbolAtom( "mstring" );
        if( nl->IsEqual( arg1, "mreal" ) )
            return nl->SymbolAtom( "mreal" );

    }
    return nl->SymbolAtom( "typeerror" );
}

/*
9.2 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

9.2.1 Selection function ~MovingSimpleSelect~

Is used for the ~deftime~, ~initial~, ~final~, ~inst~, ~val~,
~atinstant~,
~atperiods~  operations.

*/
int
MovingExtSimpleSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args );

    if( nl->SymbolValue( arg1 ) == "mstring" )
        return 0;

    return -1; // This point should never be reached
}

/*
9.2.2 Selection function ~MovingInstantPeriodsSelect~

Is used for the ~present~ operations.

*/
int
MovingExtInstantPeriodsSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args ),
    arg2 = nl->Second( args );

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "instant" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "periods" )
        return 1;

    return -1; // This point should never be reached
}

/*
9.2.3 Selection function MovingBaseRangeSelect

Is used for the ~at~ operations.

*/
int
MovingExtBaseRangeSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args ),
    arg2 = nl->Second( args );

    if( nl->SymbolValue( arg1 ) == "mreal" &&
        nl->SymbolValue( arg2 ) == "real" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "string" )
        return 1;

    if( nl->SymbolValue( arg1 ) == "mbool" &&
        nl->SymbolValue( arg2 ) == "rbool" )
        return 2;

    if( nl->SymbolValue( arg1 ) == "mint" &&
        nl->SymbolValue( arg2 ) == "rint" )
        return 3;

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "rstring" )
        return 4;

    if( nl->SymbolValue( arg1 ) == "mreal" &&
        nl->SymbolValue( arg2 ) == "rreal")
        return 5;

    if( nl->SymbolValue( arg1 ) == "mpoint" &&
        nl->SymbolValue( arg2 ) == "points")
        return 6;

    if( nl->SymbolValue( arg1 ) == "mpoint" &&
        nl->SymbolValue( arg2 ) == "line")
        return 7;

    if( nl->SymbolValue( arg1 ) == "movingregion" &&
        nl->SymbolValue( arg2 ) == "point")
        return 8;

    return -1; // This point should never be reached
}

/*
9.2.4 Selection function ~MovingBaseSelect~

Is used for the ~passes~ operations.

*/
int
MovingExtBaseSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args ),
    arg2 = nl->Second( args );

    if( nl->SymbolValue( arg1 ) == "mreal" &&
        nl->SymbolValue( arg2 ) == "real" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "string" )
        return 1;

    if( nl->SymbolValue( arg1 ) == "mpoint" &&
        nl->SymbolValue( arg2 ) == "points")
        return 2;

    if( nl->SymbolValue( arg1 ) == "mpoint" &&
        nl->SymbolValue( arg2 ) == "line")
        return 3;

    if( nl->SymbolValue( arg1 ) == "movingregion" &&
        nl->SymbolValue( arg2 ) == "point" )
        return 4;

    if( nl->SymbolValue( arg1 ) == "movingregion" &&
           nl->SymbolValue( arg2 ) == "points" )
        return 5;

    return -1; // This point should never be reached
}

/*
9.2.5 Selection function ~IntimeSimpleSelect~

Is used for the ~inst~ and ~val~ operations.

*/
int
IntimeExtSimpleSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args );

    if( nl->SymbolValue( arg1 ) == "istring" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "intimeregion" )
        return 1;

    return -1; // This point should never be reached
}

/*
9.2.6 Selection function RangeRangevaluesExtBaseSelect

Is used for the ~rangevalues~ operations.

*/
int
RangeRangevaluesExtBaseSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args );

    if( nl->SymbolValue( arg1 ) == "mbool" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "mint" )
        return 1;

    if( nl->SymbolValue( arg1 ) == "mstring" )
        return 2;

    if( nl->SymbolValue( arg1 ) == "mreal" )
        return 3;

    return -1; // This point should never be reached
}

/*
9.2.7 Selection function ~MovingPeriodsSelect~

Is used for the ~atperiods~  operation.

*/
int
MovingPeriodsSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args );

    if( nl->SymbolValue( arg1 ) == "mstring" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "movingregion" )
        return 1;

    return -1; // This point should never be reached
}

/*
9.2.8 Selection function ~MovingAtMinMaxSelect~

Is used for the operators ~atmin~ and ~atmax~.

*/
int
MovingAtMinMaxSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args );

    if( nl->SymbolValue( arg1 ) == "mbool" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "mint" )
        return 1;

    if( nl->SymbolValue( arg1 ) == "mstring" )
        return 2;

    if( nl->SymbolValue( arg1 ) == "mreal" )
        return 3;

    return -1; // This point should never be reached
}

/*
9.3 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are
several value mapping functions, one for each possible combination of input
parameter types.

9.3.1 Value mapping functions of operator ~atinstant~

*/
template <class Mapping, class Alpha>
int MappingAtInstantExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );
    Intime<Alpha>* pResult = (Intime<Alpha>*)result.addr;

    ((Mapping*)args[0].addr)->AtInstant( *((Instant*)args[1].addr), *pResult );

    return 0;
}

/*
9.3.2 Value mapping functions of operator ~atperiods~

*/
template <class Mapping>
int MappingAtPeriodsExt( Word* args,
                         Word& result,
                         int message,
                         Word& local,
                         Supplier s )
{
    result = qp->ResultStorage( s );
    ((Mapping*)args[0].addr)->AtPeriods( *((Periods*)args[1].addr),
    *((Mapping*)result.addr) );
    return 0;
}

int MappingAtPeriodsExtMRegion(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );
    MRegion* mr = (MRegion*)args[0].addr;
    Periods* per = (Periods*)args[1].addr;
    MRegion* pResult = (MRegion*)result.addr;

    pResult->AtPeriods(per, mr);

    return 0;
}

/*
9.3.3 Value mapping functions of operator ~initial~

*/
template <class Mapping, class Unit, class Alpha>
int MappingInitialExt( Word* args,
                       Word& result,
                       int message,
                       Word& local,
                       Supplier s )
{
    result = qp->ResultStorage( s );

    Mapping *m = ((Mapping*)args[0].addr);
    Intime<Alpha>* pResult = ((Intime<Alpha>*)result.addr);

    const Unit *unit;
    m->Get( 0, unit );

    if( m->IsDefined() && unit->timeInterval.lc )
        m->Initial( *pResult );
    else
        pResult->SetDefined(false);

    return 0;
}

/*
9.3.4 Value mapping functions of operator ~final~

*/
template <class Mapping, class Unit, class Alpha>
int MappingFinalExt( Word* args,
                     Word& result,
                     int message,
                     Word& local,
                     Supplier s )
{
    result = qp->ResultStorage( s );

    Mapping *m = ((Mapping*)args[0].addr);
    Intime<Alpha>* pResult = ((Intime<Alpha>*)result.addr);

    const Unit *unit;
    m->Get( m->GetNoComponents()-1, unit );

    if( m->IsDefined() && unit->timeInterval.rc )
        m->Final( *pResult );
    else
        pResult->SetDefined(false);

    return 0;
}

/*
9.3.5 Value mapping functions of operator ~present~

*/
template <class Mapping>
int MappingPresentExt_i( Word* args,
                         Word& result,
                         int message,
                         Word& local,
                         Supplier s )
{
    result = qp->ResultStorage( s );

    Mapping *m = ((Mapping*)args[0].addr);
    Instant* inst = ((Instant*)args[1].addr);

    if( !inst->IsDefined() )
        ((CcBool *)result.addr)->Set( false, false );
    else if( m->Present( *inst ) )
        ((CcBool *)result.addr)->Set( true, true );
    else
        ((CcBool *)result.addr)->Set( true, false );

    return 0;
}

template <class Mapping>
int MappingPresentExt_p( Word* args,
                         Word& result,
                         int message,
                         Word& local,
                         Supplier s )
{
    result = qp->ResultStorage( s );

    Mapping *m = ((Mapping*)args[0].addr);
    Periods* periods = ((Periods*)args[1].addr);

    if( periods->IsEmpty() )
        ((CcBool *)result.addr)->Set( false, false );
    else if( m->Present( *periods ) )
        ((CcBool *)result.addr)->Set( true, true );
    else
        ((CcBool *)result.addr)->Set( true, false );

    return 0;
}

/*
9.3.6 Value mapping functions of operator ~at~

*/

int MappingStringAtExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
  result = qp->ResultStorage( s );
  MString *m = ((MString*)args[0].addr);
  CcString* val = ((CcString*)args[1].addr);
  MString* pResult = ((MString*)result.addr);
  pResult->Clear();
  m->At( *val, *pResult );

  return 0;
}

int MappingRealAtExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );
    MRealExt *m = ((MRealExt*)args[0].addr);
    CcReal* val = ((CcReal*)args[1].addr);
    MReal* pResult = ((MReal*)result.addr);
    pResult->Clear();
    m->At( *val, *pResult );

    return 0;
}

template <class Mapping, class Unit, class Alpha, class Range>
int MappingAtExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );
    MappingExt<Unit, Alpha> *m = ((MappingExt<Unit, Alpha>*)args[0].addr);
    Range* rng = ((Range*)args[1].addr);
    Mapping* pResult = ((Mapping*)result.addr);
    pResult->Clear();
    m->At( rng, *pResult );

    return 0;
}

int MappingRRealAtExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );
    MRealExt *m = ((MRealExt*)args[0].addr);
    RReal* rng = ((RReal*)args[1].addr);
    MReal* pResult = ((MReal*)result.addr);
    pResult->Clear();
    m->At( rng, *pResult );

    return 0;
}

int MappingMPointPointsAtExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );
    MPointExt *m = ((MPointExt*)args[0].addr);
    Points* pts = ((Points*)args[1].addr);
    MPoint* pResult = ((MPoint*)result.addr);
    pResult->Clear();
    m->At( pts, *pResult );

    return 0;
}


int MappingMPointLineAtExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );
    MPointExt *m = ((MPointExt*)args[0].addr);
    Line* ln = ((Line*)args[1].addr);
    MPoint* pResult = ((MPoint*)result.addr);
    pResult->Clear();
    m->At( ln, *pResult );

    return 0;
}

int MRegionPointAtExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );
    MRegion *m = ((MRegion*)args[0].addr);
    Point* pt = ((Point*)args[1].addr);
    MPoint* pResult = ((MPoint*)result.addr);
    clock_t clock1, clock2;
    double time1 = 0.;

    pResult->Clear();

    clock1 = clock();
    MPoint* tmp = new MPoint(1);
    const URegionEmb* urtmp1;
    const URegionEmb* urtmp2;
    m->Get( 0, urtmp1 );
    m->Get( m->GetNoComponents()-1, urtmp2 );

    Interval<Instant> inv_tmp;
    inv_tmp.start = urtmp1->timeInterval.start;
    inv_tmp.end = urtmp2->timeInterval.end;
    inv_tmp.lc = true;
    inv_tmp.rc = true;

    UPoint* utmp = new UPoint(
        inv_tmp,
        pt->GetX(),
        pt->GetY(),
        pt->GetX(),
        pt->GetY());

    tmp->Add( *utmp );

    m->Intersection( *tmp, *pResult );
    clock1 = clock();
    time1 = ((double)(clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << "Total computing time : " << time1 << " ms" << endl;


    return 0;
}


/*
9.3.7 Value mapping functions of operator ~passes~

*/
template <class Mapping, class Alpha>
int MappingPassesExt( Word* args,
                      Word& result,
                      int message,
                      Word& local,
                      Supplier s )
{
    result = qp->ResultStorage( s );

    if(1)
    {
        cout << "TemporalExtAlgebra: passes" << endl;
    }

    Mapping *m = ((Mapping*)args[0].addr);
    Alpha* val = ((Alpha*)args[1].addr);

    if( !val->IsDefined() )
        ((CcBool *)result.addr)->Set( false, false );
    else if( m->Passes( *val ) )
        ((CcBool *)result.addr)->Set( true, true );
    else
        ((CcBool *)result.addr)->Set( true, false );

    return 0;
}

int MRealRealPassesExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MRealExt *m = ((MRealExt*)args[0].addr);
    CcReal* v = ((CcReal*)args[1].addr);

    if( !v->IsDefined() )
        ((CcBool *)result.addr)->Set( false, false );
    else if( m->Passes( *v ) )
        ((CcBool *)result.addr)->Set( true, true );
    else
        ((CcBool *)result.addr)->Set( true, false );

    return 0;
}

int MPointPointsPassesExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MPointExt *m = ((MPointExt*)args[0].addr);
    Points* pts = ((Points*)args[1].addr);

    if( !pts->IsDefined() )
        ((CcBool *)result.addr)->Set( false, false );
    else if( m->Passes( pts ) )
        ((CcBool *)result.addr)->Set( true, true );
    else
        ((CcBool *)result.addr)->Set( true, false );

    return 0;
}

int MPointLinePassesExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MPointExt *m = ((MPointExt*)args[0].addr);
    Line* ln = ((Line*)args[1].addr);

    if( !ln->IsDefined() )
        ((CcBool *)result.addr)->Set( false, false );
    else if( m->Passes( ln ) )
        ((CcBool *)result.addr)->Set( true, true );
    else
        ((CcBool *)result.addr)->Set( true, false );

    return 0;
}

int MRegionPointPassesExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );
    MRegion *m = ((MRegion*)args[0].addr);
    Point* pt = ((Point*)args[1].addr);
    CcBool* pResult = ((CcBool*)result.addr);
    clock_t clock1, clock2;
    double time1 = 0.;

    clock1 = clock();

    if( !pt->IsDefined() )
        pResult->Set( false, false );
    else
    {
        MPoint* mp = new MPoint(1);
        const URegionEmb* urtmp1;
        const URegionEmb* urtmp2;
        m->Get( 0, urtmp1 );
        m->Get( m->GetNoComponents()-1, urtmp2 );

        Interval<Instant> inv_tmp;
        inv_tmp.start = urtmp1->timeInterval.start;
        inv_tmp.end = urtmp2->timeInterval.end;
        inv_tmp.lc = true;
        inv_tmp.rc = true;

        UPoint* utmp = new UPoint(
            inv_tmp,
            pt->GetX(),
            pt->GetY(),
            pt->GetX(),
            pt->GetY());

        mp->Add( *utmp );

        if(0)
        {
            cout << "timeInterval of utmp: "
                << utmp->timeInterval.start.ToString()
                << " " << utmp->timeInterval.end.ToString()
                << endl;
        }

        bool result = false;

        RefinementPartition<
            MRegion,
            MPoint,
            URegionEmb,
            UPoint> rp(*m, *mp);

        MPoint resMp;
        IntersectionRPExt( m, *mp, resMp, rp, false);

        int mpPos = 0;

        if(0)
        {
            cout << "Data for resMp ..." << endl;
            for(int i=0;i<resMp.GetNoComponents();i++)
            {
                const UPoint* utemp;
                resMp.Get( i, utemp );
                cout << "timeInterval of up of resMp: "
                        << utemp->timeInterval.start.ToString()
                        << " " << utemp->timeInterval.end.ToString()
                        << endl;
                cout << "p0.x: " << utemp->p0.GetX()
                    << "  p0.y: " << utemp->p0.GetY()
                    << "  p1.x: " << utemp->p1.GetX()
                    << "  p1.y: " << utemp->p1.GetY() << endl;
            }
        }

        for (unsigned int rpPos = 0; rpPos < rp.Size(); rpPos++)
        {
            if (0)
                cerr << "MRegion::Inside() rpPos=" << rpPos
                        << " mpPos=" << mpPos
                        << endl;

            Interval<Instant>* iv;
            int urPos;
            int upPos;

            rp.Get(rpPos, iv, urPos, upPos);

            if (upPos < 0) continue;

            double prevtime = iv->start.ToDouble();
            Instant prev(instanttype);
            prev.ReadFrom(prevtime);

            for (; mpPos < resMp.GetNoComponents(); mpPos++)
            {
                if (0)
                    cerr << "MRegion::Inside()   mpPos=" << mpPos
                            << endl;

                const UPoint *up;
                resMp.Get(mpPos, up);

                if(0)
                {
                    cout << "timeInterval of up: "
                            << up->timeInterval.start.ToString()
                            << " " << up->timeInterval.end.ToString()
                            << endl;
                    cout << "timeInterval of iv: "
                            << iv->end.ToString()
                            << endl;
                    cout << "up->timeInterval.start.Compare(&iv->end) > 0"
                        <<  "|| (up->timeInterval.start.Compare(&iv->end) == 0"
                        << "&& up->timeInterval.lc"
                        << "&& !iv->rc))" << endl;
                }

                if ( !(up->timeInterval.start.Compare(&iv->end) > 0
                    || (up->timeInterval.start.Compare(&iv->end) == 0
                    && up->timeInterval.lc
                     && !iv->rc)) )
                {
                    result = true;
                    break;
                }
            }

            if(result)
                break;
            if (mpPos == resMp.GetNoComponents())
                mpPos = 0;
        }

        pResult->Set( true, result );
    }

    clock2 = clock();
    time1 = ((double)(clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << "Total computing time : " << time1 << " ms" << endl;


    return 0;
}


int MRegionPointsPassesExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );
    MRegion *m = ((MRegion*)args[0].addr);
    Points* pts = ((Points*)args[1].addr);
    CcBool* pResult = ((CcBool*)result.addr);
    clock_t clock1, clock2, clock3, clock4;
    double time1 = 0., time2 = 0., clock_ges = 0.;

    clock1 = clock();

    if( !pts->IsDefined() )
        pResult->Set( false, false );
    else
    {
        MPoint* mp = new MPoint(1);
        const URegionEmb* urtmp1;
        const URegionEmb* urtmp2;
        m->Get( 0, urtmp1 );
        m->Get( m->GetNoComponents()-1, urtmp2 );
        Interval<Instant> inv_tmp;
        inv_tmp.start = urtmp1->timeInterval.start;
        inv_tmp.end = urtmp2->timeInterval.end;
        inv_tmp.lc = true;
        inv_tmp.rc = true;
        bool result = false;

        clock3 = clock();

        for(int i=0;i<pts->Size();i++)
        {
            const Point* pt;
            pts->Get( i, pt );

            UPoint* utmp = new UPoint(
                inv_tmp,
                pt->GetX(),
                pt->GetY(),
                pt->GetX(),
                pt->GetY());

            mp->Clear();
            mp->Add( *utmp );

            if(0)
            {
                cout << "timeInterval of utmp: "
                        << utmp->timeInterval.start.ToString()
                        << " " << utmp->timeInterval.end.ToString()
                        << endl;
            }

            RefinementPartition<
                    MRegion,
            MPoint,
            URegionEmb,
            UPoint> rp(*m, *mp);

            MPoint resMp;
            IntersectionRPExt( m, *mp, resMp, rp, false);

            int mpPos = 0;

            if(0)
            {
                cout << "Data for resMp ..." << endl;
                for(int i=0;i<resMp.GetNoComponents();i++)
                {
                    const UPoint* utemp;
                    resMp.Get( i, utemp );
                    cout << "timeInterval of up of resMp: "
                            << utemp->timeInterval.start.ToString()
                            << " " << utemp->timeInterval.end.ToString()
                            << endl;
                    cout << "p0.x: " << utemp->p0.GetX()
                            << "  p0.y: " << utemp->p0.GetY()
                            << "  p1.x: " << utemp->p1.GetX()
                            << "  p1.y: " << utemp->p1.GetY() << endl;
                }
            }

            for (unsigned int rpPos = 0; rpPos < rp.Size(); rpPos++)
            {
                if (0)
                    cerr << "MRegion::Inside() rpPos=" << rpPos
                            << " mpPos=" << mpPos
                            << endl;

                Interval<Instant>* iv;
                int urPos;
                int upPos;

                rp.Get(rpPos, iv, urPos, upPos);

                if (upPos < 0) continue;

                double prevtime = iv->start.ToDouble();
                Instant prev(instanttype);
                prev.ReadFrom(prevtime);

                for (; mpPos < resMp.GetNoComponents(); mpPos++)
                {
                    if (0)
                        cerr << "MRegion::Inside()   mpPos=" << mpPos
                                << endl;

                    const UPoint *up;
                    resMp.Get(mpPos, up);

                    if(0)
                    {
                        cout << "timeInterval of up: "
                                << up->timeInterval.start.ToString()
                                << " " << up->timeInterval.end.ToString()
                                << endl;
                        cout << "timeInterval of iv: "
                                << iv->end.ToString()
                                << endl;
                        cout << "up->timeInterval.start.Compare(&iv->end) > 0"
                             <<  "|| (up->timeInterval.start.Compare(&iv->end)"
                                << " == 0 && up->timeInterval.lc"
                             << "&& !iv->rc))" << endl;
                    }

                    if ( !(up->timeInterval.start.Compare(&iv->end) > 0
                           || (up->timeInterval.start.Compare(&iv->end) == 0
                           && up->timeInterval.lc
                           && !iv->rc)) )
                    {
                        result = true;
                        break;
                    }
                }

                if(result)
                    break;
                if (mpPos == resMp.GetNoComponents())
                    mpPos = 0;
            }
            clock4 = clock();
            clock_ges = clock_ges + (clock4 - clock3);
            if(result)
                break;
        }
        pResult->Set( true, result );
    }

    clock2 = clock();
    time2 = ((double)(clock_ges / pts->Size())/CLOCKS_PER_SEC) * 1000.;
    time1 = ((double)(clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << "Average computing time per elemtn of Points: "
            << time2 << " ms/unit" << endl;
    cout << "Total computing time : " << time1 << " ms" << endl;

    return 0;
}

/*
9.3.8 Value mapping functions of operator ~deftime~

*/
template <class Mapping>
int MappingDefTimeExt( Word* args,
                       Word& result,
                       int message,
                       Word& local,
                       Supplier s )
{
    result = qp->ResultStorage( s );
    ((Mapping*)args[0].addr)->DefTime( *(Periods*)result.addr );
    return 0;
}

/*
9.3.9 Value mapping functions of operator ~inst~

*/
template <class Alpha>
int IntimeInstExt( Word* args,
                   Word& result,
                   int message,
                   Word& local,
                   Supplier s )
{
    result = qp->ResultStorage( s );
    Intime<Alpha>* i = (Intime<Alpha>*)args[0].addr;

    if( i->IsDefined())
        ((Instant*)result.addr)->CopyFrom(
        &((Intime<Alpha>*)args[0].addr)->instant);
    else
        ((Instant*)result.addr)->SetDefined( false );

    return 0;
}

/*
9.3.10 Value mapping functions of operator ~val~

*/
template <class Alpha>
int IntimeValExt( Word* args,
                  Word& result,
                  int message,
                  Word& local,
                  Supplier s )
{
    result = qp->ResultStorage( s );
    Intime<Alpha>* i = (Intime<Alpha>*)args[0].addr;

    if( i->IsDefined() )
        ((Alpha*)result.addr)->CopyFrom(
        &((Intime<Alpha>*)args[0].addr)->value );
    else
        ((Alpha*)result.addr)->SetDefined( false );

    return 0;
}

/*
9.3.11 Value mapping functions of operator ~derivativeext~

*/
template <class Mapping>
int MovingDerivativeExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    Mapping* m = ((Mapping*)args[0].addr);
    Mapping* pResult = ((Mapping*)result.addr);
    const UReal* unitin;
    UReal unitout(true);

    pResult->Clear();
    pResult->StartBulkLoad();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, unitin);
        if(!CheckURealDerivable(unitin))
        {
            unitout.a = 0.;
            unitout.b = 2*unitin->a;
            unitout.c = unitin->b;
            unitout.r = false;
            unitout.timeInterval = unitin->timeInterval;
            pResult->MergeAdd(unitout);
        }
    }
    pResult->EndBulkLoad( false );

    return 0;
}

/*
9.3.12 Value mapping functions of operator ~derivableext~

*/
template <class Mapping>
int MovingDerivableExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    Mapping* m = ((Mapping*)args[0].addr);
    MBool* pResult = ((MBool*)result.addr);
    const UReal* unitin;
    UBool unitout(true);
    CcBool myValue;

    pResult->Clear();
    pResult->StartBulkLoad();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, unitin);
        if(i==0)
        {
/*
Steps for the first Unit

*/
            myValue.Set(true, !unitin->r);
	    unitout.SetDefined(true);
            unitout.constValue.CopyFrom(&myValue);

            if(m->GetNoComponents()==1)
            {
/*
Only one Unit available

*/
                unitout.timeInterval = unitin->timeInterval;
                pResult->Add(unitout);
            }
            else
            {
/*
Just the first Unit ...

*/
                unitout.timeInterval.start = unitin->timeInterval.start;
                unitout.timeInterval.end = unitin->timeInterval.end;
                unitout.timeInterval.lc = unitin->timeInterval.lc;
                unitout.timeInterval.rc = unitin->timeInterval.rc;
            }
        }
        else
        {
            if(!unitin->r == unitout.constValue.GetBoolval())
            {
/*
The Unit has the same Bool value as the
previous. So, the time interval of the previous
Unit will be extended.

*/
                unitout.timeInterval.end = unitin->timeInterval.end;
                unitout.timeInterval.rc = unitin->timeInterval.rc;
            }
            else
            {
/*
The current Unit has another Bool value as the
previous. The previous Unit must be created. New
values must be assumed from the current Unit.

*/
                pResult->Add(unitout);
                myValue.Set(true, !unitin->r);
                unitout.constValue.CopyFrom(&myValue);
                unitout.timeInterval.start = unitin->timeInterval.start;
                unitout.timeInterval.end = unitin->timeInterval.end;
                unitout.timeInterval.lc = unitin->timeInterval.lc;
                unitout.timeInterval.rc = unitin->timeInterval.rc;
            }
            if(i==m->GetNoComponents()-1)
            {
/*
Last Unit must be created.

*/
                pResult->Add(unitout);
            }
        }
    }
    pResult->EndBulkLoad( false );

    return 0;
}

/*
9.3.12 Value mapping functions of operator ~speedext~

*/
template <class Mapping>
int MovingSpeedExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    Mapping* m = ((Mapping*)args[0].addr);
    MReal* pResult = ((MReal*)result.addr);

    double speed, distance, t;
    const Point p0, p1;
    const UPoint* unitin;
    UReal unitout(true);

    pResult->Clear();
    pResult->StartBulkLoad();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, unitin);
        distance = (unitin->p0.Distance(unitin->p1)) * FactorForUnitOfDistance;
	t = (((unitin->timeInterval.end) -
	      (unitin->timeInterval.start)).ToDouble()) * FactorForUnitOfTime;

        if (t != 0.0)
	{
          speed = distance / t;
          unitout.a = 0.0;
          unitout.b = 0.0;
          unitout.c = speed;
          unitout.r = false;
          unitout.timeInterval = unitin->timeInterval;
	  unitout.SetDefined(true);
          pResult->Add(unitout);
	}
    }

    cout << endl << endl;
    cout << "Unit of speed: " << FactorForUnitOfDistance;
    cout << " * m / " << FactorForUnitOfTime;
    cout << " * day" << endl << endl;
    pResult->EndBulkLoad( false );

    return 0;
}

/*
9.3.13 Value mapping functions of operator ~rangevalues~

*/
int RangeRangevaluesBoolExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MBool* m = ((MBool*)args[0].addr);
    RBool* pResult = ((RBool*)result.addr);

    const UBool* utemp;
    CcBool min, max;
    bool findmin=false, findmax=false, temp;

    m->Get(0, utemp);
    temp = utemp->constValue.GetBoolval();
    min.Set(true, temp);
    max.Set(true, temp);

    for(int i=1;i<m->GetNoComponents();i++)
    {
        m->Get(i, utemp);
        temp = utemp->constValue.GetBoolval();

        if(temp)
        {
          max.Set(true, temp);
          findmax = true;
        }
        else
        {
          min.Set(true, temp);
          findmin = true;
        }

        if(findmin && findmax) break;
    }

    Interval<CcBool> inter(min, max, true, true);
    pResult->Add(inter);

    return 0;
}

int RangeRangevaluesIntExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MInt* m = ((MInt*)args[0].addr);
    RInt* pResult = ((RInt*)result.addr);

    const UInt* utemp;
    int temp;
    set<int> BTree;
    clock_t clock1, clock2, clock3, clock4;
    double time1, time2;

    clock1 = clock();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, utemp);
        temp = utemp->constValue.GetIntval();
        BTree.insert(temp);
    }
    clock2 = clock();
    time1 = ((clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << endl << "Time to insert values: "
          << time1 << " milliseconds" << endl;

    set<int>::iterator iter;
    CcInt mincc, maxcc;
    int min=0, max=0;
    bool start=true;
    Interval<CcInt> inter;

    clock3 = clock();
    pResult->Clear();
    pResult->StartBulkLoad();
    for(iter=BTree.begin(); iter!=BTree.end(); ++iter)
    {
        if(start)
        {
          min = *iter;
          max = min;
          start = false;
        }
        else
        {
          if(*iter-max != 1)
          {
            mincc.Set(true, min);
            maxcc.Set(true, max);
            inter.start = mincc;
            inter.end = maxcc;
            inter.lc = true;
            inter.rc = true;
            pResult->Add(inter);
            min = *iter;
            max = min;
          }
          else
          {
            max = *iter;
          }
        }
    }
    mincc.Set(true, min);
    maxcc.Set(true, max);
    inter.start = mincc;
    inter.end = maxcc;
    inter.lc = true;
    inter.rc = true;
    pResult->Add(inter);
    clock4 = clock();
    time2 = ((clock4-clock3)/CLOCKS_PER_SEC) * 1000.;
    cout << "Time to scan and build intervals: "
          << time2 << " milliseconds" << endl;
    pResult->EndBulkLoad( false );

    return 0;
}

int RangeRangevaluesStringExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MString* m = ((MString*)args[0].addr);
    RString* pResult = ((RString*)result.addr);

    const UString* utemp;
    set<string> BTree;
    string temp;
    clock_t clock1, clock2, clock3, clock4;
    double time1, time2;

    clock1 = clock();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, utemp);
        temp = utemp->constValue.GetValue();
        BTree.insert(temp);
    }
    clock2 = clock();
    time1 = ((clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << endl << "Time to insert values: "
          << time1 << " milliseconds" << endl;

    set<string>::iterator iter;
    CcString minmaxcc;
    STRING minmax;
    Interval<CcString> inter;

    clock3 = clock();
    pResult->Clear();
    pResult->StartBulkLoad();
    for(iter=BTree.begin(); iter!=BTree.end(); ++iter)
    {
      temp = *iter;
      for(size_t i=0;i<temp.size();++i)
        minmax[i] = temp[i];
      minmaxcc.Set(true, &minmax);
      inter.start = minmaxcc;
      inter.end = minmaxcc;
      inter.lc = true;
      inter.rc = true;
      pResult->Add(inter);
    }
    clock4 = clock();
    time2 = ((clock4-clock3)/CLOCKS_PER_SEC) * 1000.;
    cout << "Time to scan and build intervals: "
          << time2 << " milliseconds" << endl;
    BTree.clear();
    pResult->EndBulkLoad( false );

    return 0;
}


int RangeRangevaluesRealExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MReal* m = ((MReal*)args[0].addr);
    RReal* pResult = ((RReal*)result.addr);

    clock_t clock1, clock2, clock3, clock4;
    double time1, time2;

    const UReal* utemp;
    double min=0.,max=0.;
    CcReal mincc, maxcc;
    Interval<CcReal> inter;
    multimap< double,const Interval<CcReal> > intermap;

    clock1 = clock();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, utemp);
        MinMaxValueFunction(utemp, min, max);
        mincc.Set(true, min);
        maxcc.Set(true, max);
        inter.start = mincc;
        inter.end = maxcc;
        inter.lc = true;
        inter.rc = true;
        intermap.insert(pair< double,const Interval<CcReal> >(max, inter));
    }
    clock2 = clock();
    time1 = ((clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << endl << "Time to insert values: "
          << time1 << " milliseconds" << endl;

    multimap< double,const Interval<CcReal> >::iterator iter = intermap.end();
    pResult->Clear();
    pResult->StartBulkLoad();
    --iter;
    bool start=true;
    clock3 = clock();
    inter = (*iter).second;
    while(iter != intermap.end())
    {
        if(start)
        {
            start = false;
        }
        else
        {
            if(1)
            {
                cout << "[" << (((*iter).second).start).GetValue();
                cout << "," << (((*iter).second).end).GetValue() << "]";
                cout << " intersects ";
                cout << "[" << (inter.start).GetValue();
                cout << "," << (inter.end).GetValue() << "]?" << endl;
            }
            if(inter.Intersects((*iter).second))
            {
                cout << "Yes" << endl;
                if(inter.start.GetValue() > ((*iter).second).start.GetValue())
                {
                    inter.start = ((*iter).second).start;
                    if(1)
                    {
                        cout << endl << "... expanding interval" << endl;
                    }
                }
            }
            else
            {
                pResult->Add(inter);
                inter = (*iter).second;
                if(1)
                {
                    cout << endl << "... creating new interval" << endl;
                }
            }
        }
        --iter;
    }
    if(inter.Intersects((*iter).second))
    {
        if(inter.start.GetValue() > ((*iter).second).start.GetValue())
            inter.start = ((*iter).second).start;
    }
    pResult->Add(inter);
    clock4 = clock();
    time2 = ((clock4-clock3)/CLOCKS_PER_SEC) * 1000.;
    cout << "Time to scan and build intervals: "
          << time2 << " milliseconds" << endl;

    pResult->EndBulkLoad( false );

    return 0;
}

/*
9.3.14 Value mapping functions of operator ~sometimes~

*/
int MovingSometimesExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MBool* m = ((MBool*)args[0].addr);

    const UBool* utemp;
    bool temp=false;

    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, utemp);
        temp = utemp->constValue.GetBoolval();
        if(temp) break;
    }

    if(!m->IsDefined())
        ((CcBool *)result.addr)->Set( false, false );
    else
        ((CcBool *)result.addr)->Set( true, temp );

    return 0;
}

/*
9.3.15 Value mapping functions of operator ~always~

*/
int MovingAlwaysExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MBool* m = ((MBool*)args[0].addr);

    const UBool* utemp;
    bool temp=true;

    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, utemp);
        temp = utemp->constValue.GetBoolval();
        if(!temp) break;
    }

    if(!m->IsDefined())
        ((CcBool *)result.addr)->Set( false, false );
    else
        ((CcBool *)result.addr)->Set( true, temp );

    return 0;
}

/*
9.3.16 Value mapping functions of operator ~never~

*/
int MovingNeverExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MBool* m = ((MBool*)args[0].addr);

    const UBool* utemp;
    bool temp=true;

    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, utemp);
        temp = utemp->constValue.GetBoolval();
        if(temp) break;
    }

    if(!m->IsDefined())
        ((CcBool *)result.addr)->Set( false, false );
    else
        ((CcBool *)result.addr)->Set( true, !temp );

    return 0;
}

/*
9.3.17 Value mapping functions of operator ~velocity~

*/
int MovingVelocityExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MPoint* m = ((MPoint*)args[0].addr);
    MPoint* pResult = ((MPoint*)result.addr);
    const UPoint* unitin;
    UPoint* unitout;
    double dx, dy, dt;

    pResult->Clear();
    pResult->StartBulkLoad();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, unitin);
        dx = unitin->p1.GetX()-unitin->p0.GetX();
        dy = unitin->p1.GetY()-unitin->p0.GetY();
        dt = ((unitin->timeInterval.end).ToDouble() -
                (unitin->timeInterval.start).ToDouble()) *
                FactorForUnitOfTime;
        unitout = new UPoint(
            unitin->timeInterval,
            0, 0, dx/dt, dy/dt
        );
        pResult->Add(*unitout);
    }
    pResult->EndBulkLoad( false );

    return 0;
}

/*
9.3.18 Value mapping function of operator ~setunitoftime~

*/
int GlobalUnitOfTimeExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );
    CcReal* f = ((CcReal*)args[0].addr);

    if(f->GetRealval() > 0.)
    {
        FactorForUnitOfTime = f->GetRealval();
        ((CcReal *)result.addr)->Set( FactorForUnitOfTime );
        ((CcReal *)result.addr)->SetDefined( true );
    }
    else
    {
        ((CcReal *)result.addr)->SetDefined( false );
    }

    return 0;
}

/*
9.3.19 Value mapping function of operator ~setunitofdistance~

*/
int GlobalUnitOfDistanceExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );
    CcReal* f = ((CcReal*)args[0].addr);

    if(f->GetRealval() > 0.)
    {
        FactorForUnitOfDistance = f->GetRealval();
        ((CcReal *)result.addr)->Set( FactorForUnitOfDistance );
        ((CcReal *)result.addr)->SetDefined( true );
    }
    else
    {
        ((CcReal *)result.addr)->SetDefined( false );
    }

    return 0;
}




/*
9.3.20 Value mapping function of operator ~mdirection~

*/
int MovingMDirectionExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MPointExt* m = ((MPointExt*)args[0].addr);
    MReal* pResult = ((MReal*)result.addr);

    m->MDirection( pResult );

    return 0;
}

/*
9.3.21 Value mapping function of operator ~locations~

*/
int MovingLocationsExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MPointExt* m = ((MPointExt*)args[0].addr);
    Points* pResult = ((Points*)result.addr);

    m->Locations( pResult );

    return 0;
}


/*
9.3.22 Value mapping functions of operator ~atmin~

*/

template <class Mapping, class Unit, class Alpha>
int MappingAtminExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MappingExt<Unit, Alpha>* m = ((MappingExt<Unit, Alpha>*)args[0].addr);
    Mapping* pResult = ((Mapping*)result.addr);
    m->AtMin( *pResult );

    return 0;
}

template <class Mapping, class Unit, class Alpha>
int MappingAtmaxExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MappingExt<Unit, Alpha>* m = ((MappingExt<Unit, Alpha>*)args[0].addr);
    Mapping* pResult = ((Mapping*)result.addr);
    //pResult->Clear();
    m->AtMax( *pResult );

    return 0;
}

int MappingAtminExt_r(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MRealExt* m = ((MRealExt*)args[0].addr);
    MReal* pResult = ((MReal*)result.addr);
    //pResult->Clear();
    m->AtMin( *pResult );

    return 0;
}

int MappingAtmaxExt_r(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MRealExt* m = ((MRealExt*)args[0].addr);
    MReal* pResult = ((MReal*)result.addr);
    //pResult->Clear();
    m->AtMax( *pResult );

    return 0;
}

/*
9.4 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of
value
mapping functions for each operator. For nonoverloaded operators there is also
such and array
defined, so it easier to make them overloaded.

*/

ValueMapping temporalatinstantextmap[] = {
    MappingAtInstantExt<MString, CcString> };

ValueMapping temporalatperiodsextmap[] = {
    MappingAtPeriodsExt<MString>,
    MappingAtPeriodsExtMRegion};

ValueMapping temporalinitialextmap[] = {
    MappingInitialExt<MString, UString, CcString> };

ValueMapping temporalfinalextmap[] = {
    MappingFinalExt<MString, UString, CcString> };

ValueMapping temporalpresentextmap[] = {
    MappingPresentExt_i<MString>,
    MappingPresentExt_p<MString>, };

ValueMapping temporalatextmap[] = {
    MappingRealAtExt,
    MappingStringAtExt,
    MappingAtExt<MBool, UBool, CcBool, RBool>,
    MappingAtExt<MInt, UInt, CcInt, RInt>,
    MappingAtExt<MString, UString, CcString, RString>,
    MappingRRealAtExt,
    MappingMPointPointsAtExt,
    MappingMPointLineAtExt,
    MRegionPointAtExt,
    };

ValueMapping temporalpassesextmap[] = {
    MRealRealPassesExt,
    MappingPassesExt<MString, CcString>,
    MPointPointsPassesExt,
    MPointLinePassesExt,
    MRegionPointPassesExt,
    MRegionPointsPassesExt};

ValueMapping temporaldeftimeextmap[] = {
    MappingDefTimeExt<MString> };

ValueMapping temporalinstextmap[] = {
    IntimeInstExt<CcString>,
    IntimeInstExt<Region> };


ValueMapping temporalvalextmap[] = {
    IntimeValExt<CcString>,
    IntimeValExt<Region> };

ValueMapping temporalderivativeextmap[] = {
    MovingDerivativeExt<MReal> };

ValueMapping temporalderivableextmap[] = {
    MovingDerivableExt<MReal> };

ValueMapping temporalspeedextmap[] = {
    MovingSpeedExt<MPoint> };

ValueMapping rangerangevaluesextmap[] = {
    RangeRangevaluesBoolExt,
    RangeRangevaluesIntExt,
    RangeRangevaluesStringExt,
    RangeRangevaluesRealExt};

ValueMapping temporalsometimesextmap[] = {
    MovingSometimesExt };

ValueMapping temporalalwaysextmap[] = {
    MovingAlwaysExt };

ValueMapping temporalneverextmap[] = {
    MovingNeverExt };

ValueMapping globalunitoftimeextmap[] = {
    GlobalUnitOfTimeExt };

ValueMapping globalunitofdistanceextmap[] = {
    GlobalUnitOfDistanceExt };

ValueMapping temporalvelocityextmap[] = {
    MovingVelocityExt };

ValueMapping temporalmdirectionextmap[] = {
    MovingMDirectionExt };

ValueMapping temporallocationsextmap[] = {
    MovingLocationsExt };

ValueMapping temporalatminextmap[] = {
    MappingAtminExt<MBool, UBool, CcBool>,
    MappingAtminExt<MInt, UInt, CcInt>,
    MappingAtminExt<MString, UString, CcString>,
    MappingAtminExt_r,};

ValueMapping temporalatmaxextmap[] = {
    MappingAtmaxExt<MBool, UBool, CcBool>,
    MappingAtmaxExt<MInt, UInt, CcInt>,
    MappingAtmaxExt<MString, UString, CcString>,
    MappingAtmaxExt_r,};

/*
9.5 Specification strings

*/
const string TemporalSpecAtInstantExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region},\n"
    "mT x instant  -> iT</text--->"
    "<text>_ atinstant _ </text--->"
    "<text>get the Intime value corresponding to the instant.</text--->"
    "<text>mpoint1 atinstant instant1</text--->"
    ") )";

const string TemporalSpecAtPeriodsExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region*},\n"
    "mT x periods -> mT\n"
    "(*) Not yet implemented for this type constructor.</text--->"
    "<text>_ atperiods _ </text--->"
    "<text>restrict the movement to the given periods.</text--->"
    "<text>mpoint1 atperiods periods1</text--->"
    ") )";

const string TemporalSpecInitialExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region*},\n"
    "(*) Not yet implemented for this type constructor.\n"
    "mT -> iT</text--->"
    "<text> initial( _ )</text--->"
    "<text>Get intime value corresponding to the initial instant.</text--->"
    "<text>initial( mpoint1 )</text--->"
    ") )";

const string TemporalSpecFinalExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region*},\n"
    "(*) Not yet implemented for this type constructor.\n"
    "mT -> iT</text--->"
    "<text> final( _ )</text--->"
    "<text>get the intime value corresponding to the final instant.</text--->"
    "<text>final( mpoint1 )</text--->"
    ") )";

const string TemporalSpecPresentExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mstring x instant -> bool,\n"
    "mstring x periods -> bool</text--->"
    "<text>_ present _ </text--->"
    "<text>whether the object is present at the given instant "
    "or period.</text--->"
    "<text>ms1 present instant1</text--->"
    ") )";

const string TemporalSpecAtExt =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T in {real, string},\n"
    "moving(T) x T -> moving(T);\n"
    "T in {int, bool, real, string},\n"
    "moving(T) x range(T) -> moving(T);\n"
    "T in {points, line},\n"
    "mpoint x T -> mpoint\n"
    "movingregion x point -> mpoint**\n"
    "(*) Not yet implemented for this type constructor.\n"
    "(**) Operator signature is not implemented yet.</text--->"
    "<text> _ at _ </text--->"
    "<text>restrict the movement at the times where the equality "
    "occurs.</text--->"
    "<text>mpoint1 at point1</text--->"
    ") )";

const string TemporalSpecPassesExt =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T in {int, bool, real, string},\n"
    "mT x T -> bool;\n"
    "T in {points, line},\n"
    "mpoint x T -> bool\n"
    "T in {point, points},\n"
    "movingregion x T -> bool</text--->"
    "<text>_ passes _ </text--->"
    "<text>whether the object passes the given value.</text--->"
    "<text>mpoint1 passes point1</text--->"
    ") )";

const string TemporalSpecInstExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region},\n"
    "iT -> instant </text--->"
    "<text>inst( _ ) </text--->"
    "<text>Extract time instant from an intime value. </text--->"
    "<text>inst( i1 )</text--->"
    ") )";

const string TemporalSpecValExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region},\n"
    "iT -> T</text--->"
    "<text>val( _ )</text--->"
    "<text>Extract value from an intime value.</text--->"
    "<text>val ( i1 )</text--->"
    ") )";

const string TemporalSpecDefTimeExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region*},\n"
    "mT -> periods\n"
    "(*) Not yet implemented for this type constructor.</text--->"
    "<text> deftime( _ )</text--->"
    "<text>get the defined time intervals for the corresponding moving "
    "data object as a periods value.</text--->"
    "<text>deftime( mp1 )</text--->"
    ") )";

const string TemporalSpecDerivativeExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mreal -> mreal</text--->"
    "<text>derivative_new ( _ )</text--->"
    "<text>Derivative of a mreal.</text--->"
    "<text>derivative_new ( mr1 )</text--->"
    ") )";

const string TemporalSpecDerivableExt =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mreal -> mbool</text--->"
    "<text>derivable_new ( _ )</text--->"
    "<text>Checking if mreal is derivable.</text--->"
    "<text>derivable_new ( mr1 )</text--->"
    ") )";

const string TemporalSpecSpeedExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint -> mreal</text--->"
    "<text>speed_new ( _ )</text--->"
    "<text>Return scalar Velocity of a mpoint as a mreal.</text--->"
    "<text>speed_new ( mp1 )</text--->"
    ") )";

const string RangeSpecRangevaluesExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T in {int, bool, real, string},\n"
    "mT -> rT</text--->"
    "<text>rangevalues ( _ )</text--->"
    "<text>Returns the smallest interval, that contains all "
    "values assumed by the argument within its definition time "
    "as a range value.</text--->"
    "<text>rangevalues ( mb1 )</text--->"
    ") )";

const string BoolSpecSometimesExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mbool -> bool</text--->"
    "<text>sometimes ( _ )</text--->"
    "<text>Returns true if a unit at least once assumes value 'TRUE'.</text--->"
    "<text>sometimes ( mb1 )</text--->"
    ") )";

const string BoolSpecAlwaysExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mbool -> bool</text--->"
    "<text>always ( _ )</text--->"
    "<text>Returns 'true' iff the moving bool does not assume value 'FALSE' "
    "within its definition time.</text--->"
    "<text>always ( mb1 )</text--->"
    ") )";

const string BoolSpecNeverExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>moving(bool) -> bool</text--->"
    "<text>never ( _ )</text--->"
    "<text>Returns 'true' iff the moving bool does not "
    "assume value 'TRUE' within its definition time.</text--->"
    "<text>never ( mb1 )</text--->"
    ") )";

const string TemporalSpecVelocityExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint -> mpoint</text--->"
    "<text>velocity_new ( _ )</text--->"
    "<text>Return the component-wise velocity vector of a mpoint as "
    "a mpoint.</text--->"
    "<text>velocity_new ( mp1 )</text--->"
    ") )";

const string GlobalSpecUnitOfTimeExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>real -> real</text--->"
    "<text>setunitoftime ( _ )</text--->"
    "<text>Set factor for unit of time: ms * real.</text--->"
    "<text>setunitoftime ( 0.001 )</text--->"
    ") )";

const string GlobalSpecUnitOfDistanceExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>real -> real</text--->"
    "<text>setunitofdistance ( _ )</text--->"
    "<text>Set factor for unit of distance: m * real.</text--->"
    "<text>setunitofdistance ( 1000.0 )</text--->"
    ") )";

const string TemporalSpecMDirectionExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint -> mreal</text--->"
    "<text>mdirection ( _ )</text--->"
    "<text>Compute the angle between X-axis and the mpoints tangent.</text--->"
    "<text>mdirection ( mp1 )</text--->"
    ") )";

const string TemporalSpecLocationsExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint -> points</text--->"
    "<text>locations( _ )</text--->"
    "<text>Project a mpoint onto its immobile and isolated points. "
    "Return the result as a points value.</text--->"
    "<text>locations( mp1 )</text--->"
    ") )";

const string TemporalSpecAtminExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T in {int, bool, real, string},\n"
    "mT -> mT</text--->"
    "<text>atmin ( _ )</text--->"
    "<text>Get moving(T) restricted to the least value.</text--->"
    "<text>atmin ( mi1 )</text--->"
    ") )";

const string TemporalSpecAtmaxExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T in {int, bool, real, string},\n"
    "mT -> mT</text--->"
    "<text>atmax ( _ )</text--->"
    "<text>Get moving object restricted to its largest value.</text--->"
    "<text>atmax ( mi1 )</text--->"
    ") )";

/*
9.6 Operators

*/
Operator temporalatinstantext(
    "atinstant",
    TemporalSpecAtInstantExt,
    5,
    temporalatinstantextmap,
    MovingExtSimpleSelect,
    MovingInstantExtTypeMapIntime );

Operator temporalatperiodsext(
    "atperiods",
    TemporalSpecAtPeriodsExt,
    2,
    temporalatperiodsextmap,
    MovingPeriodsSelect,
    MovingPeriodsExtTypeMapMoving );

Operator temporalinitialext(
    "initial",
    TemporalSpecInitialExt,
    5,
    temporalinitialextmap,
    MovingExtSimpleSelect,
    MovingExtTypeMapIntime );

Operator temporalfinalext(
    "final",
    TemporalSpecFinalExt,
    5,
    temporalfinalextmap,
    MovingExtSimpleSelect,
    MovingExtTypeMapIntime );

Operator temporalpresentext(
    "present",
    TemporalSpecPresentExt,
    2,
    temporalpresentextmap,
    MovingExtInstantPeriodsSelect,
    MovingInstantPeriodsExtTypeMapBool);

Operator temporalatext(
    "at",
    TemporalSpecAtExt,
    9,
    temporalatextmap,
    MovingExtBaseRangeSelect,
    MovingBaseExtTypeMapMoving );

Operator temporalpassesext(
    "passes",
    TemporalSpecPassesExt,
    6,
    temporalpassesextmap,
    MovingExtBaseSelect,
    MovingBaseExtTypeMapBool);

Operator temporaldeftimeext(
    "deftime",
    TemporalSpecDefTimeExt,
    5,
    temporaldeftimeextmap,
    MovingExtSimpleSelect,
    MovingExtTypeMapPeriods );

Operator temporalinstext(
    "inst",
    TemporalSpecInstExt,
    2,
    temporalinstextmap,
    IntimeExtSimpleSelect,
    IntimeExtTypeMapInstant );

Operator temporalvalext(
    "val",
    TemporalSpecValExt,
    2,
    temporalvalextmap,
    IntimeExtSimpleSelect,
    IntimeExtTypeMapBase );

Operator temporalderivativeext(
    "derivative_new",
    TemporalSpecDerivativeExt,
    1,
    temporalderivativeextmap,
    Operator::SimpleSelect,
    MovingRExtTypeMapMovingR);

Operator temporalderivableext(
    "derivable_new",
    TemporalSpecDerivableExt,
    1,
    temporalderivableextmap,
    Operator::SimpleSelect,
    MovingRExtTypeMapBool);

Operator temporalspeedext(
    "speed_new",
    TemporalSpecSpeedExt,
    1,
    temporalspeedextmap,
    Operator::SimpleSelect,
    MovingPointExtTypeMapMReal);

Operator rangerangevaluesext(
    "rangevalues",
    RangeSpecRangevaluesExt,
    4,
    rangerangevaluesextmap,
    RangeRangevaluesExtBaseSelect,
    RangeRangevaluesExtTypeMapRange );

Operator sometimesext(
    "sometimes",
    BoolSpecSometimesExt,
    1,
    temporalsometimesextmap,
    Operator::SimpleSelect,
    MovingSANExtTypeMap );

Operator alwaysext(
    "always",
    BoolSpecAlwaysExt,
    1,
    temporalalwaysextmap,
    Operator::SimpleSelect,
    MovingSANExtTypeMap );

Operator neverext(
    "never",
    BoolSpecNeverExt,
    1,
    temporalneverextmap,
    Operator::SimpleSelect,
    MovingSANExtTypeMap );

Operator setunitoftimeext(
    "setunitoftime",
    GlobalSpecUnitOfTimeExt,
    1,
    globalunitoftimeextmap,
    Operator::SimpleSelect,
    RealPhysicalUnitsExtTypeMap );

Operator setunitofdistanceext(
    "setunitofdistance",
    GlobalSpecUnitOfDistanceExt,
    1,
    globalunitofdistanceextmap,
    Operator::SimpleSelect,
    RealPhysicalUnitsExtTypeMap );

Operator temporalvelocityext(
    "velocity_new",
    TemporalSpecVelocityExt,
    1,
    temporalvelocityextmap,
    Operator::SimpleSelect,
    MPointExtTypeMapMPoint);

Operator temporalmdirectionext(
    "mdirection",
    TemporalSpecMDirectionExt,
    1,
    temporalmdirectionextmap,
    Operator::SimpleSelect,
    MovingPointExtTypeMapMReal);

Operator temporallocationsext(
    "locations",
    TemporalSpecLocationsExt,
    1,
    temporallocationsextmap,
    Operator::SimpleSelect,
    MovingPointExtTypeMapPoints);

Operator temporalatminext(
    "atmin",
    TemporalSpecAtminExt,
    4,
    temporalatminextmap,
    MovingAtMinMaxSelect,
    MovingExtTypeMapMoving);

Operator temporalatmaxext(
    "atmax",
    TemporalSpecAtmaxExt,
    4,
    temporalatmaxextmap,
    MovingAtMinMaxSelect,
    MovingExtTypeMapMoving);

class TemporalExtAlgebra : public Algebra
{
  public:
    TemporalExtAlgebra() : Algebra()
    {
        AddTypeConstructor( &intimestring );

        AddTypeConstructor( &unitstring );

        AddTypeConstructor( &movingstring );

        AddTypeConstructor( &rangebool );
        AddTypeConstructor( &rangestring );

        intimestring.AssociateKind( "TEMPORAL" );
        intimestring.AssociateKind( "DATA" );

        unitstring.AssociateKind( "TEMPORAL" );
        unitstring.AssociateKind( "DATA" );

        movingstring.AssociateKind( "TEMPORAL" );
        movingstring.AssociateKind( "DATA" );

        rangebool.AssociateKind( "RANGE" );
        rangebool.AssociateKind( "DATA" );

        rangestring.AssociateKind( "RANGE" );
        rangestring.AssociateKind( "DATA" );

        AddOperator( &temporalatinstantext );
        AddOperator( &temporalatperiodsext );
        AddOperator( &temporalinitialext );
        AddOperator( &temporalfinalext );
        AddOperator( &temporalpresentext );
        AddOperator( &temporalatext );
        AddOperator( &temporalpassesext );
        AddOperator( &temporaldeftimeext );
        AddOperator( &temporalinstext );
        AddOperator( &temporalvalext );
        AddOperator( &temporalderivativeext );
        AddOperator( &temporalderivableext );
        AddOperator( &temporalspeedext );
        AddOperator( &temporalvelocityext );
        AddOperator( &temporalmdirectionext );
        AddOperator( &temporallocationsext );
        AddOperator( &temporalatminext );
        AddOperator( &temporalatmaxext );

        AddOperator( &rangerangevaluesext );

        AddOperator( &sometimesext );
        AddOperator( &alwaysext );
        AddOperator( &neverext );
        AddOperator( &setunitoftimeext );
        AddOperator( &setunitofdistanceext );

    }
    ~TemporalExtAlgebra() {}
};

TemporalExtAlgebra tempExtAlgebra;

/*

10 Initialization

*/

extern "C"
Algebra*
InitializeTemporalExtAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return (&tempExtAlgebra);
}

