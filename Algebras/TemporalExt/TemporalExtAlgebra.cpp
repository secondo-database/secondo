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

2 Type definitions, Auxiliary Functions, Implementations

Global variable for unit of time: milliseconds * FactorForUnitOfTime
    FactorForUnitOfTime = 1. (default)

*/
float FactorForUnitOfTime = 1.;

/*
Global variable for unit of distance: meter * FactorForUnitOfDistance
    FactorForUnitOfDistance = 1. (default)

*/
float FactorForUnitOfDistance = 1.;

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
Function: MinMaxValueFunction

Parameters:
    utemp:
    minimum:
    maximum:

Result:
    return 0

*/
void MinMaxValueFunction(const UReal* utemp, float& minimum, float& maximum)
{
    float ts, te, ts_value, te_value, a, b, c;
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
    te_value = a * pow( te-ts, 2 ) + b * (te-ts) + c;

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
        t_extrem_value = a * pow( t_extrem-ts, 2 ) + b * (t_extrem-ts) + c;
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
Function AngleToXAxis

Parameters:

  p1: a pointer to start point
  p2: a pointer to end point

Result:

  Float res: the angle of the line between both points to the X-axis

This function was copied from the SpatialAlgebra, from the
Value Mapping Function direction_pp

*/
double AngleToXAxis(const Point* p1, const Point* p2, bool &defined)
{
    double res;
    double k;
    double direction; //from p1 to p2

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


void MPointExt::MDirection( MReal* result) const
{
    const UPoint* unitin;
    UReal uresult;
    bool defined;

    result->Clear();
    result->StartBulkLoad();
    for(int i=0;i<GetNoComponents();i++)
    {
        Get(i, unitin);
        // Initializing uresult
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

void MPointExt::Locations( Points* result ) const
{
    const UPoint* unitin;
    vector<Point> points;
    vector<CHalfSegment> hsegments;
    CHalfSegment* temp_hs;
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
            temp_hs = new CHalfSegment( true, false, unitin->p0, unitin->p1);
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
                result->InsertPt(points[i]);
            }
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
    result->EndBulkLoad( false );

}

template <class Unit, class Alpha>
void MappingExt<Unit, Alpha>::AtMin( Mapping<Unit, Alpha> &result ) const
{
    const Unit* utemp;
    const Unit* umin;

    Get(0, umin);
    for(int i=1;i<GetNoComponents();i++)
    {
        Get(i, utemp);
        if(0)
        {
            cout << endl << "Compare: ";
            cout << (utemp->constValue).Compare( &(umin->constValue) ) << endl;
        }
        if((utemp->constValue).Compare( &(umin->constValue) ) < 0)
            ((Unit*)umin)->CopyFrom( utemp );
    }

    result.Clear();
    result.StartBulkLoad();
    result.Add( *umin );
    result.EndBulkLoad( false );
}

template <class Unit, class Alpha>
void MappingExt<Unit, Alpha>::AtMax( Mapping<Unit, Alpha> &result ) const
{
    const Unit* utemp;
    const Unit* umax;

    Get(0, umax);
    for(int i=1;i<GetNoComponents();i++)
    {
        Get(i, utemp);
        if(0)
        {
            cout << endl << "Compare: ";
            cout << (utemp->constValue).Compare( &(umax->constValue) ) << endl;
        }
        if((utemp->constValue).Compare( &(umax->constValue) ) > 0)
            ((Unit*)umax)->CopyFrom( utemp );
    }

    result.Clear();
    result.StartBulkLoad();
    result.Add( *umax );
    result.EndBulkLoad( false );
}

template <class Unit, class Alpha>
void MappingExt<Unit, Alpha>::At(
    Range<Alpha>* inv,
    Mapping<Unit, Alpha> &result ) const
{
    const Unit* utemp;

    for(int i=0;i<GetNoComponents();i++)
    {
        Get(i, utemp);
    }
}


void MRealExt::AtMin( MReal &result ) const
{
    const UReal* utemp;
    const UReal* uresult;
    float min;
    int unit_num;
    float unit_min, unit_max;

    Get(0, utemp);
    MinMaxValueFunction(utemp, unit_min, unit_max);
    ((URealExt*)utemp)->SetUnitMin( unit_min );
    ((URealExt*)utemp)->SetUnitMax( unit_max );
    min = ((URealExt*)utemp)->GetUnitMin();
    unit_num = 0;

    if(0)
        cout << "GetUnitMin(): " << ((URealExt*)utemp)->GetUnitMin() << endl;

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
    Get( unit_num, uresult );

    result.Clear();
    result.StartBulkLoad();
    result.Add( *uresult );
    result.EndBulkLoad( false );
}

void MRealExt::AtMax( MReal &result ) const
{
    const UReal* utemp;
    const UReal* uresult;
    float max;
    int unit_num;
    float unit_min, unit_max;

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
    Get( unit_num, uresult );

    result.Clear();
    result.StartBulkLoad();
    result.Add( *uresult );
    result.EndBulkLoad( false );
}

void MRealExt::At( CcReal val, MReal &result ) const
{
    const UReal* utemp;
    UReal uresult;
    float unit_min, unit_max, ts, te, t_value, time1, time2;
    bool lh, rh;
    float value = val.GetRealval();
    Interval<Instant> temp_interval;
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
        unit_min = ((URealExt*)utemp)->GetUnitMin();
        unit_max = ((URealExt*)utemp)->GetUnitMax();
        lh = utemp->timeInterval.lc;
        rh = utemp->timeInterval.rc;
        ts = (utemp->timeInterval.start).ToDouble();
        te = (utemp->timeInterval.end).ToDouble();
        temp_interval = utemp->timeInterval;
        if(0)
        {
            cout << endl << "GetUnitMin(): " << unit_min << endl;
            cout << "GetUnitMax(): " << unit_max << endl;
            cout << "Value: " << val.GetRealval() << endl;
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
        }
        else
        {
            if(utemp->a == 0 && utemp->b != 0)
            {
                /*
                Find value in a linear function

                */
                t_value = ((value - utemp->c) / utemp->b) + ts;
                temp_interval.start.ReadFrom((const double)
                    (t_value - 0.0000001));
                temp_interval.end.ReadFrom((const double)
                    (t_value + 0.0000001));
                if(0)
                {
                    cout << "---> value in a linear function" << endl;
                }
            }
            else
            {
                if(utemp->a == 0 && utemp->b == 0)
                {
                    /*
                    Find value in a constant function

                    */
                    t_value = utemp->c;
                    temp_interval = utemp->timeInterval;
                    if(0)
                    {
                        cout << "---> value in a constant function" << endl;
                    }
                }
                else
                {
                    /*
                    Find value in a quadratic function with Newton's Method

                    */
                    if(0)
                    {
                        cout << "---> value in a quadratic function" << endl;
                    }

                    float t_newton = te;
                    float t_newton_old;
                    float t, dt, t_newton_diff, t_var;
                    if(0)
                    {
                        cout << "t_newton(Start) = " << t_newton << endl;
                        cout << "a = " << utemp->a << endl;
                        cout << "b = " << utemp->b << endl;
                        cout << "c = " << utemp->c << endl;
                    }
                    do
                    {
                        t_newton_old = t_newton;
                        t_var = t_newton - ts;
                        if(utemp->r)
                        {
                            t = sqrt(pow(t_var, 2)*utemp->a
                                + t_var*utemp->b
                                + utemp->c
                                - value);
                            dt = (1/2)*(1/sqrt(t))
                                + (2*utemp->a*t_var + utemp->b);
                            if(0)
                            {
                                cout << "t = " << t << endl;
                                cout << "dt = " << dt << endl;
                            }
                        }
                        else
                        {
                            t = pow(t_var, 2)*utemp->a
                                + t_var*utemp->b
                                + utemp->c
                                - value;
                            dt = 2*utemp->a*t_var + utemp->b;
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
                    temp_interval.start.ReadFrom((const double)
                        (t_value - 0.0000001));
                    temp_interval.end.ReadFrom((const double)
                        (t_value + 0.0000001));
                }
            }
            if(0)
            {
                cout << endl << "x-Value = " << t_value << endl;
            }
            uresult.a = utemp->a;
            uresult.b = utemp->b;
            uresult.c = utemp->c;
            uresult.r = utemp->r;
            uresult.timeInterval = temp_interval;
            result.Add( uresult );
        }
        clock4 = clock();
        clock_ges = clock_ges + (clock4 - clock3);
    }
    clock2 = clock();
    time2 = ((float)(clock_ges / GetNoComponents())/CLOCKS_PER_SEC) * 1000.;
    time1 = ((float)(clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << "Average computing time per unit: " << time2 << " ms/unit" << endl;
    cout << "Total computing time : " << time1 << " ms" << endl;
    result.EndBulkLoad( false );
}


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
    if(0)
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
2.1 Auxiliary Functions

2.1.1 Aux. Function ~CheckURealDerivable~


*/
bool CheckURealDerivable(const UReal* unit)
{
    UReal* tmp_unit = (UReal*)unit;
    return tmp_unit->r;
}


/*
3.1 Type Constructor ~istring~

Type ~istring~ represents an (instant, value)-pair of strings.

The list representation of an ~istring~ is

----    ( t string-value )
----

For example:

----    ( (instant 1.0) "My String" )
----

3.1.1 function Describing the Signature of the Type Constructor

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
3.1.2 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckIntimeString( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "istring" ));
}

/*
3.1.3 Creation of the type constructor ~istring~

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
3.2 Type Constructor ~ustring~

Type ~ustring~ represents an (tinterval, stringvalue)-pair.

3.2.1 List Representation

The list representation of an ~ustring~ is

----    ( timeinterval string-value )
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   "My String" )
----

3.2.2 function Describing the Signature of the Type Constructor

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
3.2.3 Kind Checking Function

*/
bool
CheckUString( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "ustring" ));
}

/*
3.2.4 Creation of the type constructor ~uint~

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
3.3 Type Constructor ~mstring~

Type ~mstring~ represents a moving string.

3.3.1 List Representation

The list representation of a ~mstring~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~ustring~.

For example:

----    (
          ( (instant 6.37)  (instant 9.9)   TRUE FALSE) "String 1" )
          ( (instant 11.4)  (instant 13.9)  FALSE FALSE) "String 2" )
        )
----

3.3.2 function Describing the Signature of the Type Constructor

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
3.3.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckMString( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "mstring" ));
}

/*
3.3.4 Creation of the type constructor ~mstring~

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
3.4 Type Constructor ~rbool~

Type ~rbool~ represents a range bool.

3.4.1 List Representation

The list representation of a ~rbool~ is

----    ( (bb1 eb1 lc1 rc1) (bb2 eb2 lc2 rc2) ... (bbn ebn lcn rcn) )
----


For example:

----    (
          ( (TRUE FALSE TRUE FALSE)  (FALSE FALSE TRUE TRUE) )
        )
----

3.4.2 function Describing the Signature of the Type Constructor

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
3.3.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckRBool( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "rbool" ));
}

/*
3.3.4 Creation of the type constructor ~rbool~

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
3.5 Type Constructor ~rstring~

Type ~rstring~ represents a range string.

3.5.1 List Representation

The list representation of a ~rstring~ is

----    ( (bs1 es1 lc1 rc1) (bs2 es2 lc2 rc2) ... (bsn esn lcn rcn) )
----


For example:

----    (
          ( ("First string" "Second string" TRUE FALSE)
          ("New York" "Washington" TRUE TRUE) )
        )
----

3.5.2 function Describing the Signature of the Type Constructor

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
3.3.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckRString( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "rstring" ));
}

/*
3.3.4 Creation of the type constructor ~rstring~

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
4 Operators

4.1 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

4.1.1 Type mapping function ~MovingInstantTypeMapIntime~

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
4.1.2 Type mapping function ~MovingPeriodsTypeMapMoving~

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
4.1.3 Type mapping function ~MovingTypeMapeIntime~

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
4.1.4 Type mapping function ~MovingInstantPeriodsTypeMapBool~

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
4.1.5 Type mapping function ~MovingBaseTypeMapBool~

It is for the operator ~at~.

*/
ListExpr
MovingBaseExtTypeMapMoving( ListExpr args )
{
    cout << "----->Type Mapping Function: MovingBaseTypeMapMoving" << endl;
    ListExpr arg1, arg2;
    if( nl->ListLength( args ) == 2 )
    {
        arg1 = nl->First( args );
        arg2 = nl->Second( args );

/*        if( nl->IsEqual( arg1, "mbool" ) && nl->IsEqual( arg2, "bool" ) )
            return nl->SymbolAtom( "mbool" );

        if( nl->IsEqual( arg1, "mint" ) &&
            ( nl->IsEqual( arg2, "int" ) ||
            nl->IsEqual( arg2, "rint" ) ) )
        return nl->SymbolAtom( "mint" );*/

// VTA - This operator is not yet implemented for the type of ~mreal~
        if( nl->IsEqual( arg1, "mreal" ) && nl->IsEqual( arg2, "real" ) )
            return nl->SymbolAtom( "mreal" );

        if( nl->IsEqual( arg1, "mstring" ) && nl->IsEqual( arg2, "string" ) )
            return nl->SymbolAtom( "mstring" );

    }
    cout << "---------->Return NICHT OK!!" << endl;
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.6 Type mapping function ~MovingBaseTypeMapBool~

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

        if( (nl->IsEqual( arg1, "mbool" )
                && nl->IsEqual( arg2, "bool" ))
             ||
            (nl->IsEqual( arg1, "mint" )
                && nl->IsEqual( arg2, "int" ))
             ||
// VTA - This operator is not yet implemented for the type of ~mreal~
            (nl->IsEqual( arg1, "mreal" )
                && nl->IsEqual( arg2, "real" ))
             ||
            (nl->IsEqual( arg1, "mstring" )
                && nl->IsEqual( arg2, "string" ))
             ||
            (nl->IsEqual( arg1, "mpoint" )
                && nl->IsEqual( arg2, "region" ))
          )
            return nl->SymbolAtom( "bool" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.7 Type mapping function ~MovingTypeMapRange~

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
4.1.8 Type mapping function ~IntimeTypeMapInstant~

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
4.1.9 Type mapping function ~IntimeTypeMapBase~

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
4.1.10 Type mapping function ~MovingRExtTypeMapMovingR~

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
4.1.11 Type mapping function ~MovingRExtTypeMapBool~

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
4.1.12 Type mapping function ~MovingPointExtTypeMapMReal~

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
4.1.13 Type mapping function RangeRangevaluesExtTypeMapRange

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
4.1.14 Type mapping function MovingSANExtTypeMap

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
4.1.15 Type mapping function ~MPointExtTypeMapMPoint~

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
4.1.16 Type mapping function RealPhysicalUnitsExtTypeMap

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
4.1.17 Type mapping function MovingPointExtTypeMapPoints

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
4.1.18 Type mapping function MovingExtTypeMapMoving

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
4.2 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

4.2.1 Selection function ~MovingSimpleSelect~

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
4.2.2 Selection function ~MovingInstantPeriodsSelect~

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
4.2.3 Selection function MovingBaseRangeSelect

Is used for the ~at~ operations.

*/
int
MovingExtBaseRangeSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args ),
    arg2 = nl->Second( args );

/*    if( nl->SymbolValue( arg1 ) == "mbool" &&
        nl->SymbolValue( arg2 ) == "bool" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "mint" &&
        nl->SymbolValue( arg2 ) == "int" )
    return 1;*/

    if( nl->SymbolValue( arg1 ) == "mreal" &&
        nl->SymbolValue( arg2 ) == "real" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "string" )
        return 1;

/*    if( nl->SymbolValue( arg1 ) == "mint" &&
        nl->SymbolValue( arg2 ) == "rint" )
    return 4;*/

  /*if( nl->SymbolValue( arg1 ) == "mreal" &&
    nl->SymbolValue( arg2 ) == "rreal"
)
    return 6;*/

    return -1; // This point should never be reached
}

/*
4.2.4 Selection function ~MovingBaseSelect~

Is used for the ~passes~ operations.

*/
int
MovingExtBaseSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args ),
    arg2 = nl->Second( args );

    if( nl->SymbolValue( arg1 ) == "mbool" &&
        nl->SymbolValue( arg2 ) == "bool" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "mint" &&
        nl->SymbolValue( arg2 ) == "int" )
        return 1;

    if( nl->SymbolValue( arg1 ) == "mreal" &&
        nl->SymbolValue( arg2 ) == "real" )
        return 2;

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "string" )
        return 3;

    if( nl->SymbolValue( arg1 ) == "mpoint" &&
        nl->SymbolValue( arg2 ) == "region" )
        return 4;

    return -1; // This point should never be reached
}

/*
4.2.5 Selection function ~IntimeSimpleSelect~

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
4.2.6 Selection function RangeRangevaluesExtBaseSelect

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
4.2.7 Selection function ~MovingPeriodsSelect~

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
4.2.8 Selection function ~MovingAtMinMaxSelect~

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
4.3 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are
several value mapping functions, one for each possible combination of input
parameter types.

4.3.1 Value mapping functions of operator ~atinstant~

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
4.3.2 Value mapping functions of operator ~atperiods~

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

    //mr->AtPeriods(per, *pResult);
    pResult->AtPeriods(per, mr);

    return 0;
}

/*
4.3.3 Value mapping functions of operator ~initial~

*/
template <class Mapping, class Unit, class Alpha>
int MappingInitialExt( Word* args,
                       Word& result,
                       int message,
                       Word& local,
                       Supplier s )
{
    result = qp->ResultStorage( s );
    ((Mapping*)args[0].addr)->Initial( *((Intime<Alpha>*)result.addr) );
    return 0;
}

/*
4.3.4 Value mapping functions of operator ~final~

*/
template <class Mapping, class Unit, class Alpha>
int MappingFinalExt( Word* args,
                     Word& result,
                     int message,
                     Word& local,
                     Supplier s )
{
    result = qp->ResultStorage( s );
    ((Mapping*)args[0].addr)->Final( *((Intime<Alpha>*)result.addr) );
    return 0;
}

/*
4.3.5 Value mapping functions of operator ~present~

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
4.3.6 Value mapping functions of operator ~at~

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

/*
4.3.7 Value mapping functions of operator ~passes~

*/
template <class Mapping, class Alpha>
int MappingPassesExt( Word* args,
                      Word& result,
                      int message,
                      Word& local,
                      Supplier s )
{
    result = qp->ResultStorage( s );

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

/*
4.3.8 Value mapping functions of operator ~deftime~

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
4.3.9 Value mapping functions of operator ~inst~

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
4.3.10 Value mapping functions of operator ~val~

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
4.3.11 Value mapping functions of operator ~derivativeext~

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
    UReal unitout;

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
            pResult->Add(unitout);
        }
    }
    pResult->EndBulkLoad( false );

    return 0;
}

/*
4.3.12 Value mapping functions of operator ~derivableext~

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
    UBool unitout;
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
4.3.12 Value mapping functions of operator ~speedext~

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
    UReal unitout;

    pResult->Clear();
    pResult->StartBulkLoad();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, unitin);
        distance = (unitin->p0.Distance(unitin->p1)) * FactorForUnitOfDistance;
        t = ((unitin->timeInterval.end).GetAllMilliSeconds() -
                (unitin->timeInterval.start).GetAllMilliSeconds())
                * FactorForUnitOfTime;
        speed = distance / t;
        unitout.a = 0.;
        unitout.b = 0.;
        unitout.c = speed;
        unitout.r = false;
        unitout.timeInterval = unitin->timeInterval;
        pResult->Add(unitout);
    }
    /* Units */
    cout << endl << endl;
    cout << "Unit of speed: " << FactorForUnitOfDistance;
    cout << " * m / " << FactorForUnitOfTime;
    cout << " * ms" << endl << endl;
    pResult->EndBulkLoad( false );

    return 0;
}

/*
4.3.13 Value mapping functions of operator ~rangevalues~

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
    float time1, time2;

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
    float time1, time2;

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
    float time1, time2;

    const UReal* utemp;
    float min=0.,max=0.;
    CcReal mincc, maxcc;
    Interval<CcReal> inter;
    multimap< float,const Interval<CcReal> > intermap;

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
        intermap.insert(pair< float,const Interval<CcReal> >(max, inter));
    }
    clock2 = clock();
    time1 = ((clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << endl << "Time to insert values: "
          << time1 << " milliseconds" << endl;

    multimap< float,const Interval<CcReal> >::iterator iter = intermap.end();
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
4.3.14 Value mapping functions of operator ~sometimes~

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
4.3.15 Value mapping functions of operator ~always~

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
4.3.16 Value mapping functions of operator ~never~

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
4.3.17 Value mapping functions of operator ~velocity~

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
    float dx, dy, dt;

    pResult->Clear();
    pResult->StartBulkLoad();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, unitin);
        dx = unitin->p1.GetX()-unitin->p0.GetX();
        dy = unitin->p1.GetY()-unitin->p0.GetY();
        dt = ((unitin->timeInterval.end).GetAllMilliSeconds() -
                (unitin->timeInterval.start).GetAllMilliSeconds()) *
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
4.3.18 Value mapping function of operator ~setunitoftime~

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
4.3.19 Value mapping function of operator ~setunitofdistance~

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
4.3.20 Value mapping function of operator ~mdirection~

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
4.3.21 Value mapping function of operator ~locations~

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
4.3.22 Value mapping functions of operator ~atmin~

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
    //pResult->Clear();
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
4.4 Definition of operators

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
    MappingStringAtExt};

ValueMapping temporalpassesextmap[] = {
    MappingPassesExt<MBool, CcBool>,
    MappingPassesExt<MInt, CcInt>,
    MappingPassesExt<MReal, CcReal>,
    MappingPassesExt<MString, CcString>};

ValueMapping temporaldeftimeextmap[] = {
    MappingDefTimeExt<MString> };

ValueMapping temporalinstextmap[] = {
    IntimeInstExt<CcString>,
    IntimeInstExt<CRegion> };


ValueMapping temporalvalextmap[] = {
    IntimeValExt<CcString>,
    IntimeValExt<CRegion> };

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
4.5 Specification strings

*/
const string TemporalSpecAtInstantExt  =
    "( ( \"Algebra\" \"Signature\" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>T in {int, bool, real, string, point, region},</text--->"
    "<text>moving(T) x instant  -> intime(T)</text--->"
    "<text>_ atinstant _ </text--->"
    "<text>get the Intime value corresponding to the instant.</text--->"
    "<text>mpoint1 atinstant instant1</text--->"
    ") )";

const string TemporalSpecAtPeriodsExt  =
    "( ( \"Algebra\" \"Signature\" \" \" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>T in {int, bool, real, string, point, region*},</text--->"
    "<text>moving(T) x periods -> moving(T)</text--->"
    "<text>(*) Not yet implemented for this type constructor.</text--->"
    "<text>_ atperiods _ </text--->"
    "<text>restrict the movement to the given periods.</text--->"
    "<text>mpoint1 atperiods periods1</text--->"
    ") )";

const string TemporalSpecInitialExt  =
    "( ( \"Algebra\" \"Signature\" \" \" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>T in {int, bool, real, string, point, region*},</text--->"
    "<text>(*) Not yet implemented for this type constructor.</text--->"
    "<text>moving(T) -> intime(T)</text--->"
    "<text> initial( _ )</text--->"
    "<text>Get intime value corresponding to the initial instant.</text--->"
    "<text>initial( mpoint1 )</text--->"
    ") )";

const string TemporalSpecFinalExt  =
    "( ( \"Algebra\" \"Signature\" \" \" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>T in {int, bool, real, string, point, region*},</text--->"
    "<text>(*) Not yet implemented for this type constructor.</text--->"
    "<text>moving(T) -> intime(T)</text--->"
    "<text> final( _ )</text--->"
    "<text>get the intime value corresponding to the final instant.</text--->"
    "<text>final( mpoint1 )</text--->"
    ") )";

const string TemporalSpecPresentExt  =
    "( ( \"Algebra\" \"Signature\" \" \" \"Syntax\" \"Meaning\" \" \""
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>moving(string) x instant -> bool,</text--->"
    "<text>moving(string) x periods -> bool</text--->"
    "<text>_ present _ </text--->"
    "<text>whether the object is present at the given instant</text--->"
    "<text>or period.</text--->"
    "<text>ms1 present instant1</text--->"
    ") )";

const string TemporalSpecAtExt =
    "( ( \"Algebra\" \"Signature\" \" \" \" \" \" \" \" \""
    "\" \" \" \" \"Syntax\" "
    "\"Meaning\" \" \" \"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>T in {real, string},</text--->"
    "<text>moving(T) x T -> moving(T);</text--->"
    "<text>T in {point, points*, line*, region*},</text--->"
    "<text>moving(T) x range(T) -> moving(T);</text--->"
    "<text>moving(region) x point -> mpoint**</text--->"
    "<text>(*) Not yet implemented for this type constructor.</text--->"
    "<text>(**) Operator combination is not implemented yet.</text--->"
    "<text> _ at _ </text--->"
    "<text>restrict the movement at the times where the equality </text--->"
    "<text>occurs.</text--->"
    "<text>mpoint1 at point1</text--->"
    ") )";

const string TemporalSpecPassesExt =
    "( ( \"Algebra\" \"Signature\" \" \" \" \" \" \""
    "\" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>T in {int, bool, real, string},</text--->"
    "<text>moving(T) x T -> bool;</text--->"
    "<text>T in {point, points*, line*, region*},</text--->"
    "<text>moving(point) x T -> bool</text--->"
    "<text>(*) Not yet implemented for this type constructor.</text--->"
    "<text>_ passes _ </text--->"
    "<text>whether the object passes the given value.</text--->"
    "<text>mpoint1 passes point1</text--->"
    ") )";

const string TemporalSpecDefTimeExt  =
    "( ( \"Algebra\" \"Signature\" \" \" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>T in {int, bool, real, string, point, region*},</text--->"
    "<text>moving(T) -> periods</text--->"
    "<text>(*) Not yet implemented for this type constructor.</text--->"
    "<text> deftime( _ )</text--->"
    "<text>get the defined time of the corresponding moving </text--->"
    "<text>data objects.</text--->"
    "<text>deftime( mp1 )</text--->"
    ") )";

const string TemporalSpecInstExt  =
    "( ( \"Algebra\" \"Signature\" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>T in {int, bool, real, string, point, region},</text--->"
    "<text>intime(T) -> instant</text--->"
    "<text>inst ( _ )</text--->"
    "<text>Intime time instant.</text--->"
    "<text>inst ( i1 )</text--->"
    ") )";

const string TemporalSpecValExt  =
    "( ( \"Algebra\" \"Signature\" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>T in {int, bool, real, string, point, region},</text--->"
    "<text>intime(T) -> T</text--->"
    "<text>val ( _ )</text--->"
    "<text>Intime value.</text--->"
    "<text>val ( i1 )</text--->"
    ") )";

const string TemporalSpecDerivativeExt  =
    "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>moving(real) -> moving(real)</text--->"
    "<text>derivative_new ( _ )</text--->"
    "<text>Derivative of a mreal.</text--->"
    "<text>derivative_new ( mr1 )</text--->"
    ") )";

const string TemporalSpecDerivableExt =
    "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>moving(mreal) -> moving(bool)</text--->"
    "<text>derivable_new ( _ )</text--->"
    "<text>Checking if mreal is derivable.</text--->"
    "<text>derivable_new ( mr1 )</text--->"
    ") )";

const string TemporalSpecSpeedExt  =
    "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>moving(point) -> moving(real)</text--->"
    "<text>speed_new ( _ )</text--->"
    "<text>Velocity of a mpoint given as mreal.</text--->"
    "<text>speed_new ( mp1 )</text--->"
    ") )";

const string RangeSpecRangevaluesExt  =
    "( ( \"Algebra\" \"Signature\" \" \" \"Syntax\" \"Meaning\" \" \" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>T in {int, bool, real, string},</text--->"
    "<text>moving(T) -> range(T)</text--->"
    "<text>rangevalues ( _ )</text--->"
    "<text>Returns all the values assumed by the argument over time,</text--->"
    "<text>as a set of intervals.</text--->"
    "<text>rangevalues ( mb1 )</text--->"
    ") )";

const string BoolSpecSometimesExt  =
    "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>moving(bool) -> bool</text--->"
    "<text>sometimes ( _ )</text--->"
    "<text>Returns true if a unit at least is TRUE.</text--->"
    "<text>sometimes ( mb1 )</text--->"
    ") )";

const string BoolSpecAlwaysExt  =
    "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>moving(bool) -> bool</text--->"
    "<text>always ( _ )</text--->"
    "<text>Returns true if all units are TRUE.</text--->"
    "<text>always ( mb1 )</text--->"
    ") )";

const string BoolSpecNeverExt  =
    "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>moving(bool) -> bool</text--->"
    "<text>never ( _ )</text--->"
    "<text>Returns true if all units are FALSE.</text--->"
    "<text>never ( mb1 )</text--->"
    ") )";

const string TemporalSpecVelocityExt  =
    "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>moving(point) -> moving(point)</text--->"
    "<text>velocity_new ( _ )</text--->"
    "<text>Velocity of a mpoint given as mpoint(a vector function).</text--->"
    "<text>velocity_new ( mp1 )</text--->"
    ") )";

const string GlobalSpecUnitOfTimeExt  =
    "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>real -> real</text--->"
    "<text>setunitoftime ( _ )</text--->"
    "<text>Set factor for unit of time: ms * real.</text--->"
    "<text>setunitoftime ( 0.001 )</text--->"
    ") )";

const string GlobalSpecUnitOfDistanceExt  =
    "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>real -> real</text--->"
    "<text>setunitofdistance ( _ )</text--->"
    "<text>Set factor for unit of distance: m * real.</text--->"
    "<text>setunitofdistance ( 1000. )</text--->"
    ") )";

const string TemporalSpecMDirectionExt  =
    "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>mpoint -> mreal</text--->"
    "<text>mdirection ( _ )</text--->"
    "<text>Compute the angle between X-axis and the mpoints tangent.</text--->"
    "<text>mdirection ( mp1 )</text--->"
    ") )";

const string TemporalSpecLocationsExt  =
    "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>mpoint -> points</text--->"
    "<text>locations ( _ )</text--->"
    "<text>Get projection of immobile and isolated mpoints.</text--->"
    "<text>locations ( mp1 )</text--->"
    ") )";

const string TemporalSpecAtminExt  =
    "( ( \"Algebra\" \"Signature\" \"\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>T in {int, bool, real, string},</text--->"
    "<text>moving(T) -> moving(T)</text--->"
    "<text>atmin ( _ )</text--->"
    "<text>Get moving(T) restricted to the least value.</text--->"
    "<text>atmin ( mi1 )</text--->"
    ") )";

const string TemporalSpecAtmaxExt  =
    "( ( \"Algebra\" \"Signature\" \"\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>TemporalExtAlgebra</text--->"
    "<text>T in {int, bool, real, string},</text--->"
    "<text>moving(T) -> moving(T)</text--->"
    "<text>atmax ( _ )</text--->"
    "<text>Get moving(T) restricted to the largest value.</text--->"
    "<text>atmax ( mi1 )</text--->"
    ") )";

/*
4.6 Operators

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
    10,
    temporalpresentextmap,
    MovingExtInstantPeriodsSelect,
    MovingInstantPeriodsExtTypeMapBool);

Operator temporalatext(
    "at",
    TemporalSpecAtExt,
    2,
    temporalatextmap,
    MovingExtBaseRangeSelect,
    MovingBaseExtTypeMapMoving );

Operator temporalpassesext(
    "passes",
    TemporalSpecPassesExt,
    5,
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

5 Initialization

*/

extern "C"
Algebra*
InitializeTemporalExtAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return (&tempExtAlgebra);
}
