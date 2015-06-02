/*
bla

*/
#ifndef __MMOVE_H
#define __MMOVE_H

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <cmath>
#include <limits>
#include <stack>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <math.h>
#include <time.h>

#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
//#include "DBArray.h"
#include "../../Tools/Flob/DbArray.h"
#include "Progress.h"
//#include "CellGrid.h"
#include "TemporalAlgebra.h"
#include "Point3.h"

#include "RectangleAlgebra.h"
#include "DateTime.h"
#include "AlmostEqual.h"
#include "Geoid.h"
#include "ListUtils.h"
#include "../../include/CharTransform.h"
#include "UMove.h"

#include "PolySolver.h"
#include "RelationAlgebra.h"
#include "MMRTree.h"
#include "Symbols.h"
#include "MovingRegionAlgebra.h"
#include "RefinementStream.h"
#include "TemporalUnitAlgebra.h"
#include "GenericTC.h"
#include "GenOps.h"
#include "Stream.h"
#include "DLine.h"
//#include "CellGrid.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace datetime;


class SecInterval;

/*
3.12 Class ~MMove~

*/
class MMove : public Mapping< UMove, Point3 >
{
  public:
/*
3.12.1 Constructors and Destructor

*/
    MMove() {}
/*
The simple constructor. This constructor should not be used.

*/

    MMove( const int n ):
      Mapping< UMove, Point3 >( n )
      {
        del.refs=1;
        del.SetDelete();
        del.isDefined = true;
        bbox = Rectangle<4>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
      }
/*
The constructor. Initializes space for ~n~ elements.

*/

/*
3.12.2 Modifications of Inherited Functions

Overwrites the function defined in Mapping, mostly in order to
maintain the object's bounding box. Also, some methods can be improved
using a check on bbox.

*/

  void Clear();
  void Add( const UMove& unit );
  void MergeAdd(const UMove& unit);
  bool EndBulkLoad( const bool sort = true, const bool checkvalid = false );
  void Restrict( const vector< pair<int, int> >& intervals );
  ostream& Print( ostream &os ) const;
  bool operator==( const MMove& r ) const;
  bool Present( const Instant& t ) const;
  bool Present( const Periods& t ) const;
  void AtInstant( const Instant& t, Intime<Point3>& result ) const;
  void AtPeriods( const Periods& p, MMove& result ) const;
  void AtRect(const Rectangle<3>& rect, MMove& result) const;


  virtual Attribute* Clone() const
  {
    assert( IsOrdered() );
    MMove *result;
    if( !this->IsDefined() ){
      result = new MMove( 0 );
    } else {
      result = new MMove( GetNoComponents() );
      if(GetNoComponents()>0){
        result->units.resize(GetNoComponents());
      }
      result->StartBulkLoad();
      UMove unit;
      for( int i = 0; i < GetNoComponents(); i++ ){
        Get( i, unit );
        result->Add( unit );
      }
      result->EndBulkLoad( false );
    }
    result->SetDefined(this->IsDefined());
    return (Attribute*) result;
  }

  void CopyFrom( const Attribute* right )
  {
    const MMove *r = (const MMove*)right;
    assert( r->IsOrdered() );
    Clear();
    this->SetDefined(r->IsDefined());
    if( !this->IsDefined() ) {
      return;
    }
    StartBulkLoad();
    UMove unit;
    for( int i = 0; i < r->GetNoComponents(); i++ ){
      r->Get( i, unit );
      Add( unit );
    }
    EndBulkLoad( false );
  }

/*
3.10.5.3 Operation ~trajectory~

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( n )$, where ~n~ is the number of units of this ~MMove~

*/
  void Trajectory( Line& line ) const;

  // The scalar velocity as a temporal function
  // If geoid = 0, metric (X.Y)-coordinates are used within the MMove.
  // If geoid points to a valid Geoid object, geografic coordinates (LON,LAT)
  // are used within the MMove (speed over ground).
  void MSpeed(  MReal& result, const Geoid* geoid = 0 ) const;

  // The vectorial velocity --- (X,Y)-components --- as temporal function
  void MVelocity( MMove& result ) const;

/*
3.10.5.3 Operation ~distance~

If ~geoid~ is NULL, euclidean geometry is used, spherical geometry otherwise.
If invalid geographic coordinates are found, the result is UNDEFINED.

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( n )$, where ~n~ is the number of units of this ~MMove~

*/
  void Distance( const Point3& p, MReal& result,
                 const Geoid* geoid=0 ) const;
  void SquaredDistance( const Point3& p, MReal& result,
                        const Geoid* geoid=0 ) const;
  void SquaredDistance( const MMove& p, MReal& result,
                        const Geoid* geoid=0 ) const;

/*
3.10.5.4 Operation ~Simplify~

*Precondition*: ~X.IsOrdered()~

*Semantics*: Simplifies the tracjectory this moving point.

*Complexity*: worst case: O(n * n), in average O(n * log (n))

*/

   void Simplify(const double epsilon, MMove& result,
                 const bool checkBreakPoints,
                 const DateTime& duration) const;

/*
3.10.5.5 Operation ~BreakPoints~

*Precondition*: ~X.IsOrdered()~
*Semantics*: Computes all points where this mpoints stays longer than the given
             time.

*/
//    void BreakPoints(Points& result, const DateTime& dur) const;
//    void BreakPoints(Points& result, const DateTime& dur,
//                     const CcReal& epsilon,
//                     const Geoid* geoid=0) const;

/*
3.10.5.6 Operatiopn ~Breaks~

This function computes the timeIntervalls for Breaks

*/

//    void Breaks(Periods& result, const DateTime& dur,
//                const CcReal& epsilon,
//                const Geoid* geoid=0) const;


/*
3.10.5.5 Operation ~Vertices~


This operations stores the ends of the units into a ~points~ value.

*/

   void Vertices(Points& result) const;


/*
3.10.5.5 Operation ~gk~

This method performs a gauss krueger projection to that mpoint.
If the coordinates are not in geo range (-180-180, -90-90), the result
will be undefined. The same holds for zone < 0 and zone > 119.

*/
//   void gk(const int &zone, MMove& result) const;


/*
3.10.5.6 Operation ~TranslateAppend~

Appends the mpoint argument to this moving point. The point will stay on its
last position for an interval of length dur. If dur is smaller or equals to
zero, the new movement starts directly after the definition time of the
original object. The movement is continued at the last position of this mpoint.

*/

//  void TranslateAppend(const MMove& mp, const DateTime& dur);


/*
3.10.5.7 Reverse

Store the reverse of the movement of this instance of a mpoint into result.

*/
  void Reverse(MMove& result);


/*
3.10.5.7 Direction

Compute the ~direction~ (~useHeading~ is ~false~) resp. ~heading~ (~true~) of a
moving point.
If ~geoid~ is not NULL, spherical geometry is applied. In this case, each unit
may produce several result units, since the true course changes along the
orthodrome. Parameter ~epsilon~ determines how exact the approximation will be.

*/
//  void Direction( MReal* result,
//                  const bool useHeading = false,
//                  const Geoid* geoid    = 0,
//                  const double epsilon  = 0.0000001) const;

/*
3.10.5.8 ~Sample~

This operator creates a new mpoint from the original one.
All units will have the length of the duration given as
parameter. The starting time is the same as for the original
mpoint. If gaps longer than the given duration exist,
the next unit will also have the starting time of the unit
directly after the gap.
If __KeepEndPoint__ is set to __true__, the last position will be
part of the resuolt even if at the corresponding time no
sample point exist. If the parameter __exactPath__ is set to
__true__, additional sample points are inserted to be sure that the
path of the object is the same as in the original, i.e. no shortcuts
for corners.

*/
//  void Sample(const DateTime& duration, MMove& result,
//              const bool KeepEndPoint = false,
//              const bool exactPath = false  )const;

/*
3.10.5.8 ~Append~

The ~Append~ function appends all units of the argument to this
MMove. If this mpoint or the argument is undefined or if the
argument starts before this mpoint ends, this mpoint will be set
to be undefined. The return value is the defined state of this
mpoint after the operation (indicating the success).

*/
   bool Append(const MMove& p, const bool autoresize = true);

/*
3.10.5.9 ~Disturb~

The ~disturb~ operation changes the position of the moving point
using a random generator.

*/
//   void Disturb(MMove& result,
//                const double maxDerivation,
//                double maxDerivationPerStep);


/*
3.10.5.10 ~length~

Determines the drive distance of this moving point (odometer).
Coordinates are interpreted as metric (X,Y) coordinates.
Will return a value smaller than zero if this mpoint is not defined

*/
  double Length() const;

/*
Determines the drive distance of this moving point (odometer).
Coordinates are interpreted as geographic (LON,LAT) coordinates.

If an invalid geographic coordinate is encountered, ~valid~ is set to false,
otherwise the result is calculated and ~valid~ is set to true.

The same happens, if the Mpoint is undefined. In addition to setting ~valid~ to
false, the return value will be negative.

*/
  double Length(const Geoid& g, bool& valid) const;

/*
3.10.5.11 ~BoundingBox~

Returns the MMove's minimum bounding rectangle. If geoid is NULL, euclidean
geometry is used, otherwise spherical geometry is applied.

*/
  //FIXME
  // return the stored bbox
  Rectangle<4> BoundingBox(const Geoid* geoid = 0) const;

  static unsigned GetDim(){ return 4; }
  //FIXME
  // return the spatial bounding box (2D: X/Y)
  const Rectangle<3> BoundingBoxSpatial(const Geoid* geoid = 0) const;

  // recompute bbox, if necessary
  void RestoreBoundingBox(const bool force = false);


  void EqualizeUnitsSpatial(const double epsilon,
                            MMove& result,
                            const bool skipSplit = false) const;

  static const string BasicType(){ return "mpoint"; }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }

/*
3.10.5.11 ~Delay Operator~

Considering this MMove instance as the schedule, and a given MMove as the
actual movement, the goal is to compute the continuous delay between the two
MMoves (i.e. How many seconds is the actual movement delayed from the
schedule).

If ~geoid~ is NULL, euclidean geometry is used, otherwise spherical geometry
using the provided geoid.

If invalid coordinates are found, the result is UNDEFINED.

*/
    MReal* DelayOperator(const MMove *actual, const Geoid* geoid = 0 );
    MReal* DistanceTraversed(double*, const Geoid* geoid = 0 ) const;
    MReal* DistanceTraversed( const Geoid* geoid = 0 ) const;

/*
3.10.5.11 ~at region~ operators

This operator returns a copy of the mpoint's restriction to a given region

*/
    void AtRegion(const Region *reg, MMove &result) const;


private:
   int IntervalRelation(Interval<Instant> &int_a_b,
        Interval<Instant> &int_c_d  ) const;
   double* MergePartitions(double* first, int firstSize,
        double* second, int secondSize, int& count );
   void Simplify(const int min, const int max,
                 bool* useleft, bool* useright,
                 const double epsilon) const;
//FIXME
   Rectangle<4> bbox;
}; 

#endif