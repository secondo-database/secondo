/*
class Methods of temporalalgebra::MPoint
refactored into free templatized functions

*/

#ifndef ALGEBRAS_TEMPORAL2_TEMPORALALGEBRAFUNCTIONS_H_
#define ALGEBRAS_TEMPORAL2_TEMPORALALGEBRAFUNCTIONS_H_

#include "Algebras/Rectangle/RectangleAlgebra.h" //Rectangle<>
#include "Algebras/Spatial/SpatialAlgebra.h" //Line
#include "Algebras/Temporal/TemporalAlgebra.h" //UPoint, etc.

namespace temporal2algebra {

template <class MPOINT>
bool IsValid(const MPOINT& mp)
{
  if( !mp.IsDefined() )
    return true;

  //  not used in temporal2algebra:
  //  if( canDestroy )
  //    return false;

  if( !mp.IsOrdered() )
    return false;

  if( mp.IsEmpty() )
    return true;

  bool result = true;
 temporalalgebra::UPoint lastunit, unit;

  mp.Get( 0, lastunit );
  if ( !lastunit.IsValid() )
  {
    std::cerr << "Mapping<Unit, Alpha>::IsValid(): "
            "unit is invalid: i=0" << std::endl;
    return false;
  }
  if ( mp.GetNoComponents() == 1 ){
    return true;
  }

  for( int i = 1; i < mp.GetNoComponents(); i++ )
  {
    mp.Get( i, unit );
    if( !unit.IsValid() )
    {
      result = false;
      std::cerr << "Mapping<Unit, Alpha>::IsValid(): "
              "unit is invalid: i=" << i << std::endl;
      return false;
    }
    if(lastunit.timeInterval.end > unit.timeInterval.start){
       std::cerr << "Units are not ordered by time" << std::endl;
       std::cerr << "lastUnit.timeInterval =  ";
       lastunit.timeInterval.Print(std::cerr);
       std::cerr << std::endl;
       std::cerr << "unit.timeInterval =  ";
       unit.timeInterval.Print(std::cerr);
       std::cerr << std::endl;
       return false;
    }


    if( (!lastunit.timeInterval.Disjoint(unit.timeInterval)) )
    {
      result = false;
      std::cerr << "Mapping<Unit, Alpha>::IsValid(): "
              "unit and lastunit not disjoint: i=" << i << std::endl;
      std::cerr << "\n\tlastunit = "; lastunit.timeInterval.Print(std::cerr);
      std::cerr << "\n\tunit     = "; unit.timeInterval.Print(std::cerr);
      std::cerr << std::endl;
      return false;
    }
    lastunit = unit;
  }
  return result;
}

template <class MPOINT>
void AtPeriods( const MPOINT& mp,
        const temporalalgebra::Periods& p,
        MPOINT& result )
{
  result.Clear();
  result.SetDefined(true);
  if( mp.IsDefined() && p.IsDefined() )
  {
    if( !mp.GetBBox().IsDefined())
    { // result is undefined
      result.SetDefined(false);
    } else if( mp.IsEmpty() || p.IsEmpty())
    { // result is defined but empty
      result.SetDefined(true);
    } else if( IsMaximumPeriods(p) )
    { // p is [begin of time, end of time]. Copy the input into result.
      result.CopyFrom(&mp);
    }
    else
    { // compute result
      assert( mp.IsOrdered() );
      assert( p.IsOrdered() );
      Instant perMinInst; p.Minimum(perMinInst);
      Instant perMaxInst; p.Maximum(perMaxInst);
      double permind = perMinInst.ToDouble();
      double permaxd = perMaxInst.ToDouble();
      double mind = mp.GetBBox().MinD(2);
      double maxd = mp.GetBBox().MaxD(2);
      if( (mind > permaxd && !AlmostEqual(mind,permaxd)) ||
          (maxd < permind && !AlmostEqual(maxd,permind)))
      {
//         cout << __PRETTY_FUNCTION__<< "(" << __FILE__ << __LINE__
//           << "): Bounding box check failed:" << endl;
//         cout << "\tPeriod : "; p.Print(cout); cout << endl;
//         cout << "\tperMinInst : "; perMinInst.Print(cout); cout << endl;
//         cout << "\tperMaxInst : "; perMaxInst.Print(cout); cout << endl;
//         cout << "\tpermind/permaxd : " << permind  << "\t/\t"
//              << permaxd << endl;
//         cout << "\tmind/maxd :" << mind << "\t/\t" << maxd << endl;
//         cout << "\tBBox = "; bbox.Print(cout); cout << endl;
        result.SetDefined(true);
      } else
      {
        result.StartBulkLoad();
        temporalalgebra::UPoint unit;
        temporalalgebra::Interval<Instant> interval;
        int i = 0, j = 0;
        mp.Get( i, unit );
        p.Get( j, interval );

        while( 1 )
        {
          if( unit.timeInterval.Before( interval ) )
          {
            if( ++i == mp.GetNoComponents() )
              break;
            mp.Get( i, unit );
          }
          else if( interval.Before( unit.timeInterval ) )
          {
            if( ++j == p.GetNoComponents() )
              break;
            p.Get( j, interval );
          }
          else
          { // we have overlapping intervals, now
            temporalalgebra::UPoint r(1);
            unit.AtInterval( interval, r );
            assert( r.IsDefined() );
            assert( r.IsValid()   );
            result.Add( r );
//          cout << "\n\tunit = "; unit.Print(cout); cout << endl;
//          cout << "\tinterval =       "; interval.Print(cout); cout << endl;
//          cout << "\tr    = "; r.Print(cout); cout << endl;

            if( interval.end == unit.timeInterval.end )
            { // same ending instant
              if( interval.rc == unit.timeInterval.rc )
              { // same ending instant and rightclosedness: Advance both
                if( ++i == mp.GetNoComponents() )
                  break;
                mp.Get( i, unit );
                if( ++j == p.GetNoComponents() )
                  break;
                p.Get( j, interval );
              }
              else if( interval.rc == true )
              { // Advanve in mapping
                if( ++i == mp.GetNoComponents() )
                  break;
                mp.Get( i, unit );
              }
              else
              { // Advance in periods
                assert( unit.timeInterval.rc == true );
                if( ++j == p.GetNoComponents() )
                  break;
                p.Get( j, interval );
              }
            }
            else if( interval.end > unit.timeInterval.end )
            { // Advance in mpoint
              if( ++i == mp.GetNoComponents() )
                break;
              mp.Get( i, unit );
            }
            else
            { // Advance in periods
              assert( interval.end < unit.timeInterval.end );
              if( ++j == p.GetNoComponents() )
                break;
              p.Get( j, interval );
            }
          }
        }
        result.EndBulkLoad();
      }
    }
  } else
  {
    result.SetDefined(false);
  }
}

template <class MPOINT>
Rectangle<3> BoundingBox(const MPOINT& mp,
        const Geoid* geoid /*=0*/)
{
  if(geoid){ // spherical geometry case:
    if(!mp.IsDefined() || (mp.GetNoComponents()<=0) ){
      return Rectangle<3>(false);
    }
    temporalalgebra::UPoint u;
    Rectangle<3> bbx(false);
    for(int i = 0; i < mp.GetNoComponents(); i++){
      mp.Get(i,u);
      assert( u.IsDefined() );
      if(bbx.IsDefined()){
        bbx.Union(u.BoundingBox(geoid));
      } else {
        bbx = u.BoundingBox(geoid);
      }
    }
    return bbx;
  } // else: euclidean case
 // return bbox2;
 return mp.GetBBox();
}


template <class MPOINT>
void Trajectory(const MPOINT& mp, Line& line )
{
  line.Clear();
  if(!mp.IsDefined()){
    line.SetDefined( false );
    return;
  }
  line.SetDefined( true );
  line.StartBulkLoad();

  HalfSegment hs;
  temporalalgebra::UPoint unit;
  int edgeno = 0;

  int size = mp.GetNoComponents();
  if (size>0)
    line.Resize(size);

  Point p0(false);      // starting point
  Point p1(false);      // end point of the first unit
  Point p_last(false);  // last point of the connected segment

  for( int i = 0; i < size; i++ ) {
    mp.Get( i, unit );

    if( !AlmostEqual( unit.p0, unit.p1 ) )    {
      if(!p0.IsDefined()){ // first unit
        p0 = unit.p0;
        p1 = unit.p1;
        p_last = unit.p1;
      } else { // segment already exists
        if(p_last!=unit.p0){ // spatial jump
           hs.Set(true,p0,p_last);
           hs.attr.edgeno = ++edgeno;
           line += hs;
           hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
           line += hs;
           p0 = unit.p0;
           p1 = unit.p1;
           p_last = unit.p1;
        } else { // an extension, check direction
           if(!AlmostEqual(p0,unit.p1)){
             HalfSegment tmp(true,p0,unit.p1);
             double dist = tmp.Distance(p1);
             double dist2 = tmp.Distance(p_last);
             if(AlmostEqual(dist,0.0) && AlmostEqual(dist2,0.0)){
               p_last = unit.p1;
             } else {
               hs.Set(true,p0,p_last);
               hs.attr.edgeno = ++edgeno;
               line += hs;
               hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
               line += hs;
               p0 = unit.p0;
               p1 = unit.p1;
               p_last = unit.p1;
             }
          }
        }
      }
    }
  }
  if(p0.IsDefined() && p_last.IsDefined() && !AlmostEqual(p0,p_last)){
    hs.Set(true,p0,p_last);
    hs.attr.edgeno = ++edgeno;
    line += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    line += hs;
  }
  line.EndBulkLoad();
}

// special case: non-const in TemporalAlgebra:
//void MPoint::TranslateAppend(const MPoint& mp, const datetime::DateTime& dur);
template <class MPOINT>
void TranslateAppend(const MPOINT& mp,
        const datetime::DateTime& dur,
        MPOINT& destMp) {

    if( !destMp.IsDefined() || !mp.IsDefined() || !dur.IsDefined() ){
        destMp.Clear();
        destMp.SetDefined(false);
        return;
    }
    if(mp.GetNoComponents()==0){ // nothing to do (already defined)
        return;
    }
    if(destMp.GetNoComponents()==0){
        destMp.CopyFrom(&mp);
        return;
    }

    int newSize = destMp.GetNoComponents()+mp.GetNoComponents();
    destMp.Resize(newSize);
    temporalalgebra::UPoint lastUnit;

    destMp.StartBulkLoad();

    temporalalgebra::UPoint firstUnit;
    mp.Get(0,firstUnit);

    // add a staying unit
    if(!dur.IsZero() && !dur.LessThanZero()){
        destMp.Get(destMp.GetNoComponents()-1,lastUnit);
        temporalalgebra::Interval<Instant> interval = lastUnit.timeInterval;
        Point lastPoint = lastUnit.p1;
        // append a unit of staying
        temporalalgebra::Interval<Instant> gapInterval(
                interval.end,interval.end +dur,
                !interval.rc,!firstUnit.timeInterval.lc);
        temporalalgebra::UPoint gap(gapInterval,lastPoint,lastPoint);
        destMp.Add(gap);
    }

    destMp.Get(destMp.GetNoComponents()-1,lastUnit);
    Instant end = lastUnit.timeInterval.end;
    datetime::DateTime timediff = end - firstUnit.timeInterval.start;
    double xdiff  = lastUnit.p1.GetX() - firstUnit.p0.GetX();
    double ydiff  = lastUnit.p1.GetY() - firstUnit.p0.GetY();

    temporalalgebra::UPoint Punit;
    mp.Get(0,Punit);
    temporalalgebra::UPoint unit = Punit;
    unit.Translate(xdiff,ydiff,timediff);
    if(!(lastUnit.timeInterval.rc)){
        unit.timeInterval.lc = true;
    } else {
        unit.timeInterval.lc = false;
    }
    destMp.Add(unit);

    for(int i=1; i< mp.GetNoComponents(); i++){
        mp.Get(i,Punit);
        unit = Punit;
        unit.Translate(xdiff,ydiff,timediff);
        destMp.Add(unit);
    }
    destMp.EndBulkLoad(false);
}




} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_TEMPORALALGEBRAFUNCTIONS_H_ */
