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

*/

#include "RobustPlaneSweepAlgebra.h"
#include "TemporalAlgebra.h"

#include "Algorithm/Hobby.h"

namespace RobustPlaneSweep
{

  class LineMinize : public IntersectionAlgorithmData
  {
  private:
    const Line& _source;
    int _currentSourceIndex;
    Line& _result;
    int _outputSegments;

  public:
    LineMinize(const Line& src, Line& result) : 
      _source(src),_currentSourceIndex(0),_result(result),_outputSegments(0)
    {
    }

    ~LineMinize()
    {
    }

    void InitializeFetch()
    {
      _currentSourceIndex=0;
    }

    virtual HalfSegmentIntersectionId 
      GetHalfSegmentId(const HalfSegment& segment)
    {
      return segment.attr.edgeno;
    }

    bool FetchInputHalfSegment(HalfSegment &segment)
    {
      // cout << "I:" << _currentSourceIndex << "/" << _source.Size() << "\n";
      if(_currentSourceIndex < _source.Size()) {
        _source.Get(_currentSourceIndex++,segment);
        return true;
      } else {
        return false;
      }
    }

    void OutputHalfSegment(const HalfSegment& segment)
    {
      // cout << "O:" << _outputSegments << "\n";
      HalfSegment s1=segment;
      HalfSegment s2=segment;
      s1.SetLeftDomPoint(true);
      s2.SetLeftDomPoint(false);
      s1.attr.edgeno=_outputSegments;
      s2.attr.edgeno=_outputSegments;
      _result+=s1;
      _result+=s2;
      _outputSegments++;
    }

    const Rectangle<2> GetBoundingBox()
    {
      return _source.BoundingBox();
    }

    bool RemoveOverlappingSegments() 
    {
      return true;
    }

    AttrType MergeAttributes(const AttrType& a1,const AttrType&)
    {
      return a1;
    }

    bool IsInputOrderedByX()
    {
      return false;
    }

    int GetRoundToDecimals()
    {
      return 7; // wegen AlmostEqual
    }

    void OutputFinished()
    {
    }
  };


  void Realminize2Robust(const Line& src,Line& result){

    result.Clear();
    if(!src.IsDefined()){
      result.SetDefined(false);
      return;
    }
    result.SetDefined(true);
    if(src.Size()==0){ // empty line, nothing to realminize
      return;
    }

    result.StartBulkLoad();
    LineMinize data(src,result);
    Hobby hobby(&data);
    hobby.DetermineIntersections();
    //  BentleyOttmann bo(&data);
    //  bo.DetermineIntersections();

    result.StartBulkLoad(); // ordered = true
    result.EndBulkLoad(true,false);
  }


  const string TemporalSpecTrajectory =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint -> line</text--->"
    "<text> trajectory2( _ )</text--->"
    "<text>Get the trajectory of the corresponding"
    " moving point object.</text--->"
    "<text>trajectory2( mp1 )</text--->"
    ") )";

  void MPointTrajectory(MPoint* mpoint, Line& line )
  {
    line.Clear();
    if(!mpoint->IsDefined()){
      line.SetDefined( false );
      return;
    }
    line.SetDefined( true );
    line.StartBulkLoad();

    HalfSegment hs;
    UPoint unit;
    int edgeno = 0;

    int size = mpoint->GetNoComponents();
    if (size>0)
      line.Resize(size);

    Point p0(false);      // starting point
    Point p1(false);      // end point of the first unit
    Point p_last(false);  // last point of the connected segment

    for( int i = 0; i < size; i++ ) {
      mpoint->Get( i, unit );

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

    Line line2(0);
    Realminize2Robust(line,line2);
    line.CopyFrom(&line2);
    line2.Destroy();
  }

  int MPointTrajectory( Word* args, Word& result, int message,
    Word& local, Supplier s )
  {
    result = qp->ResultStorage( s );

    Line *line = ((Line*)result.addr);
    MPoint *mpoint = ((MPoint*)args[0].addr);

    MPointTrajectory( mpoint, *line);

    return 0;
  }

  ListExpr MovingTypeMapSpatial( ListExpr args )
  {
    if ( nl->ListLength( args ) == 1 )
    {
      ListExpr arg1 = nl->First( args );

      if( nl->IsEqual( arg1, MPoint::BasicType() ) )
        return nl->SymbolAtom( Line::BasicType() );
    }
    return nl->SymbolAtom( Symbol::TYPEERROR() );
  }

  Operator temporaltrajectory2( "trajectory2",
    TemporalSpecTrajectory,
    MPointTrajectory,
    Operator::SimpleSelect,
    MovingTypeMapSpatial);

  RobustPlaneSweepAlgebra::RobustPlaneSweepAlgebra()
  {
    AddOperator(&temporaltrajectory2);
  }

}

extern "C"
  Algebra* InitializeRobustPlaneSweepAlgebra( 
  NestedList* nlRef, 
  QueryProcessor* qpRef )
{
  // The C++ scope-operator :: must be used to qualify the full name
  return new RobustPlaneSweep::RobustPlaneSweepAlgebra();
}

