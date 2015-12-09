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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header file of the Temporal Algebra

September - November 2007 Sascha Vaut

1 Overview

The type system of the HierarchicalGeo Algebra can be seen below.



2 Defines, includes, and constants

*/
#ifndef _HIERARCHICALGEO_ALGEBRA_H_
#define _HIERARCHICALGEO_ALGEBRA_H_
#endif

#include <iostream>
#include <sstream>
#include <string>
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "TemporalAlgebra.h"
#include "SpatialAlgebra.h"
#include "MovingRegionAlgebra.h"
#include "../../Tools/Flob/DbArray.h"
#include "RectangleAlgebra.h"
#include "DateTime.h"

extern NestedList* nl;
extern QueryProcessor* qp;


class CUPoint;
class CMPoint;

template <class Alpha>
class HierarchicalEntity;

typedef HierarchicalEntity<CUPoint> HCUPoint;
class HCMPoint;
class HMPoint;
/*
Forward declarations.


3 C++ Classes (Definition)

3.1 Uncertain

This class represents an epsilon-value of type real. It will be used in type
constructors of all ~uncertain~ Datatypes.

*/
//template <class Alpha>
struct Uncertain
{

  public:

/*
3.1.1 Constructors

*/
  Uncertain() {}
/*
The simple constructor. This constructor should not be used.

*/

  Uncertain( bool is_defined):def(is_defined) {}

/*
Use this constructor when declaring uncertain object variables etc.

*/

  Uncertain( const double& epsilon ):
    epsilon ( epsilon ), def ( true )
    {}

/*
The creation of an uncertain value, setting the epsilon value.

*/
  Uncertain( const Uncertain& rhs) :
	  epsilon(rhs.epsilon), def(rhs.def) {}

  virtual ~Uncertain() {}

/*
3.1.2 Member functions

*/

  virtual bool IsValid() const
  {
    if (epsilon >= 0 && def)
      return true;
    return false;
  }

/*
Checks if the Uncertain is valid or not. This function should be used for
debugging purposes only. An uncertain is valid if the following conditions are
true:

  1 ~alpha~ and ~epsilon~ are defined

  2 ~epsilon~ $>=$ 0

  3 ~defined~ $==$ TRUE

*/

  double GetEpsilon() const
  {
    return epsilon;
  }

  void SetEpsilon(double e)
  {
    epsilon = e;
  }
/*
Returns the epsilon value of the Uncertain value.

*/

  virtual bool UncertainIsDefined() const
  {
    return def;
  }

/*
Checks if the Uncertain value is defined or not. If it is not defined, only the
epsilon value is set. The Alpha value is left to be set later.

*/

  virtual void UncertainSetDefined( bool defined )
  {
    this->def = defined;
  }

/*
Sets the argument ~defined~ to the given boolean value.

*/


/*
3.1.3 Attributes

*/

  double epsilon;
/*
The possible difference between the original value and the given value.

*/
  protected:

  bool def;
/*
The flag that indicates if the value is defined or not.

*/

};

/*
3.2 CUPoint

This class will be used in the ~cupoint~ type constructor, i.e., the type
constructor for the uncertain temporal unit of point values.

*/
class CUPoint : public temporalalgebra::UPoint,
                public Uncertain
{
  public:
/*
3.2.1 Constructors and Destructor

*/
  CUPoint() {}
/*
The simple constructor. This constructor should not be used.

*/
  CUPoint( const bool is_defined ):
      temporalalgebra::UPoint(false),
      Uncertain(is_defined)
  {
  }

  CUPoint( const double epsilon ):
      temporalalgebra::UPoint(false),
      Uncertain(epsilon)
  {
  }
/*
The simple constructor, only defining the epsilon-value.

*/

  CUPoint( const temporalalgebra::UPoint& source):
    temporalalgebra::UPoint( source ),
    Uncertain( 0.0 )
  {
  }
/*
A constructor to create an uncertain unit point from a unit point.

*/
  CUPoint( const double epsilon, const temporalalgebra::UPoint& source):
    temporalalgebra::UPoint( source ),
    Uncertain( epsilon )
  {
  }

/*
A constructor to create an uncertain unit point from the given epsilon-value
and unit point. This constructor should only be used to create test-data!

*/
  CUPoint( const double epsilon, 
      const temporalalgebra::Interval<Instant>& interval,
      const Point& p0, const Point& p1 ):
    temporalalgebra::UPoint( interval, p0, p1 ),
    Uncertain (epsilon)
    {
    }

  CUPoint( const double epsilon, 
      const temporalalgebra::Interval<Instant>& interval,
      const double x0, const double y0,
      const double x1, const double y1 ):
    temporalalgebra::UPoint( interval, x0, y0, x1, y1 ),
    Uncertain (epsilon)
    {
    }


  CUPoint( const CUPoint& source ) :
    temporalalgebra::UPoint(source),
    Uncertain(source)
  {
  }
/*
The copy-constructor.

*/
  virtual ~CUPoint() {}

/*
The destructor.


3.2.3 The Temporal Functions

*/

  virtual void TemporalFunction( const Instant& t,
                                 Point& result,
                                 bool ignoreLimits = false ) const;

  virtual bool Passes( const Point& val ) const
  {
    return D_Passes( val );
  }

  bool Passes( const Region& val ) const
  {
    return D_Passes( val );
  }

  virtual bool At( const Point& val, TemporalUnit<Point>& result ) const
  {
    return false;
  }

  virtual void AtInterval( const temporalalgebra::Interval<Instant>& i,
                           TemporalUnit<Point>& result ) const;

  void Distance( const Point& p, temporalalgebra::UReal& result ) const {}


  void USpeed( temporalalgebra::UReal& result ) const {}
  void UVelocity( temporalalgebra::UPoint& result ) const {}
  void Intersection(const temporalalgebra::UPoint &other, 
                    temporalalgebra::UPoint &result) const {}

  virtual bool EqualValue( const CUPoint& i )
  {
    return ( AlmostEqual( p0, i.p0 ) && AlmostEqual( p1, i.p1 ) );
  }

  void Translate(const double x, const double y, const DateTime& duration) {}

/*
3.2.4 Additional Uncertain-Temporal Functions

*/
  void UTrajectory( const double e, Region& region ) const;

/*
The only difference of this function to the function 'UTrajectory()' of the
TemporalAlgebra is the result-Type. To include the uncertainty of an uncertain
spatio-temporal object, the trajectory of such an object is defined as a
region-value, which represents the area, the uncertain object may possibly
pass.

*/
  bool D_Passes( const Point& p ) const;
  bool D_Passes( const Region& r ) const;

/*
The function-name D\_Passes() is a shorthand for 'Definitely\_Passes'. It
returns TRUE, if there is a point in time, when the uncertain spatio-temporal
object lies on or inside the given spatial object, and FALSE, when there may
be no such point in time. Refering to a (certain) point-object, this will
never occur if the uncertainty-value epsilon is greater than 0.

*/

  bool P_Passes( const Point& val ) const;
  bool P_Passes( const Region& val ) const;

/*
P\_Passes() stands for 'Possibly\_Passes' and returns TRUE if there may be a
point in time, when the uncertain spatio-temporal object lies on or inside the
given spatial object. FALSE is only returned when there is certainly no such
point in time.

*/

  void D_At( const Point& p, CUPoint& result ) const;
  void D_At( const Region& r, CMPoint& result ) const;

/*
The function D\_At ('Definitely\_At') returns that part(s) of the CUPoint, that
lie(s) definitely on or inside the given spatal object. Refering to a (certain)
point-object, the returned CUPoint is empty if the uncertainty-value epsilon
is greater than 0. Refering to a region-object, there may be more than one
CUPoints to be returned, so the result-type is a CMPoint.

*/

  void P_At( const Point& p, CUPoint& result ) const;
  void P_At( const Region& r, CMPoint& result ) const;

/*
The function P\_At is a shorthand for 'Possibly\_At' and returns that part of
the CUPoint, which possibly lies on or inside the given spatal object.

*/

  void AtInstant( const Instant& t, 
                  temporalalgebra::Intime<Region>& result ) const;

  void Set(double e, temporalalgebra::Interval<Instant> t, Point p0, Point p1)
  {

  }

  void UnitSetDefined( bool def )
  {
    temporalalgebra::UPoint::SetDefined( def );
  }

  bool UnitIsDefined() const
  {
    return temporalalgebra::UPoint::IsDefined();
  }

  bool UnitIsValid() const
  {
    return TemporalUnit<Point>::IsValid();
  }

  bool UncertainIsValid() const
  {
    return Uncertain::IsValid();
  }

  bool IsValid() const
  {
    return UnitIsValid() && Uncertain::IsValid();
  }

virtual const Rectangle<3> BoundingBox() const
  {
    return Rectangle<3>( true, MIN( p0.GetX(), p1.GetX() )-epsilon,
                               MAX( p0.GetX(), p1.GetX() )+epsilon,
                               MIN( p0.GetY(), p1.GetY() )-epsilon,
                               MAX( p0.GetY(), p1.GetY() )+epsilon,
                               timeInterval.start.ToDouble(),
                               timeInterval.end.ToDouble() );
  }

  virtual const Rectangle<2> BBox2D() const
  {
    return Rectangle<2>( true, MIN( p0.GetX(), p1.GetX() )-epsilon,
                               MAX( p0.GetX(), p1.GetX() )+epsilon,
                               MIN( p0.GetY(), p1.GetY() )-epsilon,
                               MAX( p0.GetY(), p1.GetY() )+epsilon);
  }
/*
For this is an uncertain temporalalgebra::UPoint-value, the 
returned Rectangle<3> is enlarged by
the epsilon-value.


3.2.5 Functions to be part of relations

*/

  bool IsDefined() const
  {
    return UnitIsDefined() && UncertainIsDefined();
  }

  void SetDefined( const bool defined )
  {
    UnitSetDefined( defined );
    UncertainSetDefined( defined );
  }

  inline virtual size_t Sizeof() const
  {
    return sizeof( *this );
  }

  inline virtual int Compare( const Attribute* arg ) const
  {
    CUPoint* up2 = (CUPoint*) arg;
    if (!UnitIsDefined() && !up2->UnitIsDefined())
      return 0;
    if (!UnitIsDefined())
      return -1;
    if (!up2->UnitIsDefined())
      return 1;

    int cmp = timeInterval.CompareTo(up2->timeInterval);
    if(cmp)
      return cmp;

    if(p0<up2->p0)
      return -1;
    if(p0>up2->p0)
      return 1;
    if(p1>up2->p1)
      return 1;
    if(p1<up2->p1)
      return -1;

    if( epsilon < up2->GetEpsilon() )
      return -1;
    if( epsilon > up2->GetEpsilon() )
      return 1;

    return 0;
  }

  inline virtual bool Adjacent( const Attribute* arg ) const
  {
    return false;
  }

  ostream& Print( ostream& os ) const
  {
    if(IsDefined())
    {
      os << "CUPoint: " << "( ";
      os << epsilon << "( ";
      timeInterval.Print(os);
      os << ", ";
      p0.Print(os);
      os << ", ";
      p1.Print(os);
      os << " ) ) ";
      return os;
    }
    else
      return os << "CUPoint: (undef) ";
  }

  inline virtual size_t HashValue() const
  {
    return 0;
  }

  inline virtual CUPoint* Clone() const
  {
    CUPoint *res;
    res = new CUPoint(epsilon, timeInterval, p0, p1 );
    res->SetDefined( IsDefined() );
    return res;
  }

  virtual void CopyFrom( const Attribute* right )
  {
    const CUPoint* i = (const CUPoint*)right;

    if( i->UncertainIsDefined() )
      epsilon = i->epsilon;
    else
      epsilon = 0.0;

    UncertainSetDefined( true );

    UnitSetDefined( i->UnitIsDefined() );
    if(i->UnitIsDefined() )
      {
        timeInterval = i->timeInterval;
        p0 = i->p0;
        p1 = i->p1;
      }
    else
      {
        timeInterval = temporalalgebra::Interval<Instant>();
        p0 = Point( false, 0.0, 0.0);
        p1 = Point( false, 0.0, 0.0);
      }
  }

  static const string BasicType() {return "cupoint"; }
  static const bool checkType(const ListExpr type){
     return listutils::isSymbol(type,BasicType());
  }

/*
3.2.6 Attributes

*/

  //Point p0, p1;

};

/*
3.3 CMPoint

the implementation of an uncertain temporalalgebra::MPoint

*/
class CMPoint : public temporalalgebra::Mapping< CUPoint, Point >,
                public Uncertain
{
  public:
/*
3.3.1 Constructors and Destructor

*/
  CMPoint() {}
/*
The simple constructor. This constructor should not be used.

*/
  CMPoint( const int n ):
      temporalalgebra::Mapping< CUPoint, Point >( n ),
      Uncertain( true )
  {
    epsilon = 0.0;
    bbox = Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  }
/*
The constructor. Initializes space for ~n~ elements.

*/

/*
3.3.2 Modifications of Inherited Functions

Overwrites the function defined in temporalalgebra::Mapping, mostly in order to
maintain the object's bounding box. Also, some methods can be improved
using a check on bbox.

*/

  void Clear();
  void Add( const CUPoint& unit );
  void MergeAdd(const CUPoint& unit);
  bool EndBulkLoad( const bool sort = true, const bool dummy = false );
  void Restrict( const vector< pair<int, int> >& intervals );
  bool Present( const Instant& t ) const;
  bool Present( const temporalalgebra::Periods& t ) const;
  void AtInstant( const Instant& t, 
                  temporalalgebra::Intime<Region>& result ) const;
  void AtPeriods( const temporalalgebra::Periods& p, CMPoint& result ) const;
  void Trajectory( Region& region );

/*
Functions to be part of relations

*/

  ostream& Print( ostream &os ) const;

  virtual Attribute* Clone() const;

  void CopyFrom( const Attribute* right );

  static const string BasicType() {return "cmpoint"; }
  static const bool checkType(const ListExpr type){
     return listutils::isSymbol(type,BasicType());
  }


  inline virtual size_t Sizeof() const
  {
    return sizeof( *this );
  }


/*
3.3.3 Additional Uncertain-Temporal Functions

*/
  //void Trajectory( Region& region ) const;

/*
The only difference of this function to the function 'Trajectory()' of the
TemporalAlgebra is the result-Type. To include the uncertainty of an uncertain
spatio-temporal object, the trajectory of such an object is defined as a
region-value, which represents the area, the uncertain object may possibly
pass.

*/
  bool D_Passes( const Point& p ) const;
  bool D_Passes( const Region& r ) const;

/*
The function-name D\_Passes() is a shorthand for 'Definitely\_Passes'. It
returns TRUE, if there is a point in time, when the uncertain spatio-temporal
object lies on or inside the given spatial object, and FALSE, when there may be
no such point in time. Refering to a (certain) point-object, this will never
occur if the uncertainty-value epsilon is greater than 0.

*/

  bool P_Passes( const Point& p ) const;
  bool P_Passes( const Region& r ) const;

/*
P\_Passes() stands for 'Possibly\_Passes' and returns TRUE if there may be a
point in time, when the uncertain spatio-temporal object lies on or inside the
given spatial object. FALSE is only returned when there is certainly no such
point in time.

*/

  void D_At( const Point& p, CMPoint& result ) const;
  void D_At( const Region& r, CMPoint& result ) const;

/*
The function D\_At ('Definitely\_At') returns that part of the CMPoint, which
lies definitely on or inside the given spatial object. Refering to a (certain)
point-object, the returned CMPoint is empty if the uncertainty-value epsilon
is greater than 0.

*/

  void P_At( const Point& val, CMPoint& result ) const;
  void P_At( const Region& val, CMPoint& result ) const;

/*
The function P\_At is a shorthand for 'Possibly\_At' and returns that part of
the CMPoint, which possibly lies on or inside the given spatial object.


3.3.3.6 ~epsilon~

*/
  void RestoreEpsilon();

/*
3.3.3.7 ~BoundingBox~

Returns the CMPoint's minimum bounding rectangle

*/
  // return the stored bbox
  Rectangle<3> BoundingBox() const;

  Rectangle<2> BBox2D() const;

  // recompute bbox, if necessary
  void RestoreBoundingBox(const bool force = false);

  private:

  Rectangle<3> bbox;
};

/*
3.4 HierarchicalEntity

This template class offers attributes and operations to store datatypes in a
hierarchical structure.

*/
template <class Alpha>
class HierarchicalEntity: public Attribute
{
  public:
/*
3.4.1 Constructors

*/

  HierarchicalEntity() {}
/*
The simple constructor. This constructor should not be used.

*/

  HierarchicalEntity( const HierarchicalEntity<Alpha>& entity):
      Attribute(true),
      value(),
      generalizedby( entity.GetGeneralizedby() ),
      layer( entity.GetLayer() ),
      index( entity.GetIndex() ),
      originstart( entity.GetOriginstart() ),
      originend( entity.GetOriginend() )
  {
    value.CopyFrom( &entity.value );
  }
/*
The copy-constructor.

*/

  HierarchicalEntity( const Alpha& alpha, const int l, const int idx,
                      const int start, const int end ):
    Attribute(true),
    value(),
    generalizedby( -1 ),
    layer( l ),
    index( idx ),
    originstart( start ),
    originend( end )
  {
    value.CopyFrom( &alpha );
  }
/*
The creation of a HierarchicalEntity, setting the typically allready known
attributes.

*/
  HierarchicalEntity( const Alpha& alpha, const int genby, const int l,
                      const int idx, const int start, const int end ):
    Attribute(true),
    value(),
    generalizedby( genby ),
    layer( l ),
    index( idx ),
    originstart( start ),
    originend( end )
  {
    value.CopyFrom( &alpha );
  }
/*
A constructor which sets all attributes (usually unsed by the in-function).

*/
  ~HierarchicalEntity() {}


/*
3.4.2 Member functions

*/

  inline bool IsValid() const
  {
    if( index > -1 )
      return true;
    return false;
  }

  inline bool IsLeaf() const
  {
    if( index > -1 && originstart = originend )
      return true;
    return false;
  }

  inline bool IsRoot() const
  {
    if( index > -1 && generalizedby == -1 )
      return true;
    return false;
  }

  inline void SetIndex(const int idx)
  {
    index = idx;
  }

  inline void SetLayer(const int l)
  {
    layer = l;
  }

  inline int GetLayer() const
  {
    return layer;
  }

  inline int GetIndex() const
  {
    return index;
  }

  inline int GetGeneralizedby() const
  {
    return generalizedby;
  }

  inline void SetGeneralizedby(const int idx)
  {
    if( generalizedby == -1 )
      generalizedby = idx;
    else
    {
      cout << "Error in HierarchicalEntity::SetGeneralizedby(): The Index"
        "of the Generalization of this Entity is already set!\n";
      cerr << "Error in HierarchicalEntity::SetGeneralizedby(): The Index"
        "of the Generalization of this Entity is already set!\n";
    }
  }

  inline int GetOriginstart() const
  {
    return originstart;
  }

  inline void SetOriginstart(const int start)
  {
    originstart = start;
  }

  inline int GetOriginend() const
  {
    return originend;
  }

  inline void SetOriginend(const int end)
  {
    originend = end;
  }

/*
3.4.3 Functions to be part of relations

*/

  inline bool IsDefined() const
  {
    return IsValid();
  }

  inline void SetDefined(bool defined) {}

  inline size_t Sizeof() const
  {
    return sizeof( *this );
  }

/*
For HierarchicalEntity is a container-element, providing an order of units
within a hierarchical structure, only the layer- and index-values are
compared. The order of the contained units is ignored here!

*/
  inline int Compare( const Attribute* arg ) const
  {
    HierarchicalEntity<Alpha>* other = (HierarchicalEntity<Alpha>*) arg;
    if( !IsDefined() && !other->IsDefined() )
      return 0;
    if( !IsDefined() )
      return -1;
    if( !other->IsDefined() )
      return 1;

    if( layer < other->GetLayer() )
      return -1;
    if( layer > other->GetLayer() )
      return 1;
    else
    { // the layer-values are equal
      if( index < other->GetIndex() )
        return -1;
      if( index > other->GetIndex() )
        return 1;
    }
    return 0;
  }

  bool Adjacent( const Attribute* arg ) const
  {
    HierarchicalEntity<Alpha>* other = (HierarchicalEntity<Alpha>*) arg;
    if( IsDefined() && other->IsDefined() && layer == other->GetLayer() )
    {
      if( index+1 == other->GetIndex() || index-1 == other->GetIndex() )
        return true;
    }
    return false;
  }

  HierarchicalEntity<Alpha>* Clone() const
  {
    return (new HierarchicalEntity<Alpha>( *this) );
  }

  inline ostream& Print( ostream &os ) const
  {
    if( !IsDefined() )
    {
      return os << "(HierarchicalEntity: undefined)";
    }
    os << "(HierarchicalEntity: " << generalizedby << " " << layer << " "
      << index << " " << originstart << " " << originend << "\n\t";
    value.Print(os);
    os << "\n)" << endl;
    return os;
  }

  virtual size_t HashValue() const
  {
    return 0;
  }

  virtual void CopyFrom( const Attribute* right )
  {
    const HierarchicalEntity<Alpha>* i =
      (const HierarchicalEntity<Alpha>*)right;

    generalizedby = i->GetGeneralizedby();
    layer = i->GetLayer();
    index = i->GetIndex();
    originstart = i->GetOriginstart();
    originend = i->GetOriginend();
    value.CopyFrom( &i->value );
  }

/*
3.4.4 Attributes

*/
  Alpha value;
/*
The object to be stored in the hierarchical structure

*/
  protected:

    int generalizedby;
/*
The index of the fathernode, containing a generalization that includes all of
its sons.

*/
    int layer;
/*
The position in the array.

*/
    int index;
/*
The position in the array.

*/
    int originstart;
/*
The index of the first entity, generalized by this entity.

*/
    int originend;
/*
The index of the last entity, generalized by this entity.

*/
};


/*
3.6 HCUPoint

represents an uncertain unit point (CUPoint) within an hierarchical structure.

*/
typedef HierarchicalEntity<CUPoint> HCUPoint;


/*
3.7 HCMPoint

The type Hierarchical Uncertain Moving Point defines a hierarchical structure
in which up to 5 various Generalizations of a particular moving point can be
stored. Every unit of such an uncertain moving point is encapsulated within an
object called HierarchicalEntity. Such an Entity defines the position in the
hierarchical structure.

Every uncertain unit point of an upper layer (one of the layers 0 to 4)
generalizes a number of uncertain unit points within the layer below.

*/
class HCMPoint : public Attribute
{
  public:
/*
3.7.1 Constructors and Destructor

*/
  HCMPoint() {}
/*
The simple constructor. This constructor should not be used.

*/
  HCMPoint( const int n ):
    Attribute(true),
    layer0epsilon( -1 ), layer1epsilon( -1 ), layer2epsilon( -1 ),
    layer3epsilon( -1 ), layer4epsilon( -1 ),
    layer0( 0 ), layer1( 0 ), layer2( 0 ), layer3( 0 ), layer4( n ),
    canDestroy( false ), epsilon( -1 ), factor( -1 )
  {
    bbox = Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  }

  HCMPoint( const int n, const double e, const double f ):
    Attribute(true),
    layer0epsilon( -1 ), layer1epsilon( -1 ), layer2epsilon( -1 ),
    layer3epsilon( -1 ), layer4epsilon( -1 ),
    layer0( 0 ), layer1( 0 ), layer2( 0 ), layer3( 0 ), layer4( n ),
    canDestroy( false ), epsilon( e ), factor( f )
  {
    bbox = Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  }
/*
A constructor, initializing space for ~n~ entities in the bottom layer.

*/
  ~HCMPoint()
  {
    if( canDestroy )
    {
      layer0.Destroy();
      layer1.Destroy();
      layer2.Destroy();
      layer3.Destroy();
      layer4.Destroy();
    }
  }
/*
The destructor.

*/
  void Destroy()
  {
    canDestroy = true;
  }
/*
This function should be called before the destructor if one wants to destroy
the persistent arrays of hierarchical entities. It marks the persistent arrays
for destroying. The destructor will perform the real destroying.

3.7.2 Member Functions

*/
  inline bool IsEmpty() const;

  double GetLayerepsilon( const int layer ) const;

  void SetLayerepsilon( const int layer, const double epsilon );

  inline double GetLayer0epsilon() const
  {
    return layer0epsilon;
  }

  inline void SetLayer0epsilon(const double epsilon)
  {
    layer0epsilon = epsilon;
  }

  inline double GetLayer1epsilon() const
  {
    return layer1epsilon;
  }

  inline void SetLayer1epsilon(const double epsilon)
  {
    layer1epsilon = epsilon;
  }

  inline double GetLayer2epsilon() const
  {
    return layer2epsilon;
  }

  inline void SetLayer2epsilon(const double epsilon)
  {
    layer2epsilon = epsilon;
  }

  inline double GetLayer3epsilon() const
  {
    return layer3epsilon;
  }

  inline void SetLayer3epsilon(const double epsilon)
  {
    layer3epsilon = epsilon;
  }

  inline double GetLayer4epsilon() const
  {
    return layer4epsilon;
  }

  inline void SetLayer4epsilon(const double epsilon)
  {
    layer4epsilon = epsilon;
  }

  inline void SetEpsilon(const double e)
  {
    epsilon = e;
  }

  inline double GetEpsilon() const
  {
    return epsilon;
  }

  inline void SetFactor(const double f)
  {
    factor = f;
  }

  inline double GetFactor() const
  {
    return factor;
  }

  inline void Get( const int layer, const int i, HCUPoint& hcup ) const;

  inline void Get( const int layer, const int i, CUPoint& cup ) const;

  inline void Put( const int layer, const int i, HCUPoint& hcup);

  inline int GetNoComponents() const;

  inline void ResizeLayer( const int layer, const int n );

  inline void TrimLayerToSize( const int layer );

  inline int LayerSize( const int layer ) const;

  int Position( int layer, const Instant& t );
  int Position( int layer, const Instant& t, const int start, const int end );

  inline void GetFirstLayer( int& layer, int& size ) const;

  void DefTime( temporalalgebra::Periods& p );
  bool Present( const Instant& i );

  int Generalize(const int layer, const bool checkBreakPoints,
                          const DateTime dur);

  void Simplify(const int min, const int max, const int layer,
                    bool* useleft, bool* useright, double* realepsilon,
                    const double epsilon);

/*
Checks all Units in the most uncertain Layer of the HCMPoint, if the given
temporalalgebra::Periods-Value is completely inside the Definition-time of 
the HCMPoint-object.

*/
  bool Present( const temporalalgebra::Periods& p );

/*
Checks, if the given Point-Value lies inside the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine if the HCMPoint definitely-
passes the given Point-value.

*/
  bool D_Passes( const Point& p );

/*
This recursive function determines, by a pre-order run through the hierarchical
 structure, if the HCMPoint definitely-passes the given Point-value.

*/
  bool D_Passes( const int layer, const int start, const int end,
                  const Point& p );

/*
Checks, if the given Region-Value intersects the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine if the HCMPoint definitely-
passes the given Region-value.

*/
  bool D_Passes( const Region& r );

/*
This recursive Function determines, by an in-order run through the hierarchical
 structure, if the HCMPoint definitely-passes the given Region-value.

*/
  bool D_Passes( const int layer, const int start, const int end,
                  const Region& r );

/*
Checks, if the given Point-Value lies inside the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine if the HCMPoint possibly-
passes the given Point-value.

*/
  bool P_Passes( const Point& p );

/*
This recursive function determines, by a pre-order run through the hierarchical
 structure, if the HCMPoint possibly-passes the given Point-value.

*/
  bool P_Passes( const int layer, const int start, const int end,
                  const Point& p );

/*
Checks, if the given Region-Value intersects the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine if the HCMPoint possibly-
passes the given Region-value.

*/
  bool P_Passes( const Region& r );

/*
This recursive Function determines, by a pre-order run through the hierarchical
 structure, if the HCMPoint possibly-passes the given Region-value.

*/
  bool P_Passes( const int layer, const int start, const int end,
                  const Region& r );

  void AtInstant( const Instant& t, temporalalgebra::Intime<Region>& result );
  int AtInstant( const int layer, const int start, const int end,
                  const Instant& t, temporalalgebra::Intime<Region>& result );

  void AtPeriods( const temporalalgebra::Periods& p, CMPoint& result);
  int AtPeriods( const int layer, const int start, const int end,
                  const temporalalgebra::Periods& p, CMPoint& result );

  void D_At( const Point& p, CMPoint& result );
  bool D_At( const int layer, const int start, const int end, const Point& p,
                  CMPoint& result);

  void D_At( const Region& r, CMPoint& result );
  bool D_At( const int layer, const int start, const int end,
                  const Region& r, CMPoint& result );

  void P_At( const Point& p, CMPoint& result );
  bool P_At( const int layer, const int start, const int end, const Point& p,
                  CMPoint& result);

  void P_At( const Region& r, CMPoint& result );
  bool P_At( const int layer, const int start, const int end,
                  const Region& r, CMPoint& result );

/*


*/
  void Get( const int i, HCUPoint& hcup ) const;


  void GetCMPoint( const double epsilon, CMPoint& result );


  void Add( const HCUPoint& hcup );

  //void GetGeneralization( const double epsilon, const Unit*& upi ) const;
/*
Returns the unit ~upi~ at the position ~i~ in the mapping.

*/
  void Clear();

/*
Remove all entities in all DBArrays.

*/
  bool IsValid() const;
/*
This functions tests if a ~HMPoint~ is in a valid format. It is used for
debugging purposes only. The ~HMPoint~ is valid, if the following conditions
are true:

  1 Each entity is valid

  2 Each entity within the layers 0 to 4 has defined indices pointing to its
    origin-entity.

  3 Each entity within the layers 1 to 5 has a defined index 'generalizedby'.

3.7.3 Functions to be part of relations

*/
  inline bool IsDefined() const
  {
    return true;
  }

  inline void SetDefined( bool Defined ) {}


  inline size_t Sizeof() const
  {
    return sizeof( *this );
  }

  inline int Compare( const Attribute *arg ) const;


  inline bool Adjacent( const Attribute *arg ) const
  {
    return false;
  }

  inline Attribute* Clone() const;


  inline ostream& Print( ostream &os ) const;


  inline size_t HashValue() const
  {
    return 0;
  }

  inline void CopyFrom( const Attribute* right );


  inline int NumOfFLOBs() const
  {
    return 5;
  }

  inline Flob* GetFLOB( const int i);

  static const string BasicType() {return "hcmpoint"; }
  static const bool checkType(const ListExpr type){
     return listutils::isSymbol(type,BasicType());
  }


/*
3.7.3.7 ~BoundingBox~

Returns the HCMPoint's minimum bounding rectangle

*/
  // return the stored bbox
  Rectangle<3> BoundingBox() const;

  // return a bbox without the 3rd (temporal) Dimension
  Rectangle<2> BBox2D() const;

  // recompute bbox, if necessary
  void RestoreBoundingBox(const bool force = false);

/*
3.7.4 Attributes

*/
    double layer0epsilon, layer1epsilon, layer2epsilon, layer3epsilon,
            layer4epsilon;
/*
5 variables of type double, to store the uncertainty-value of each layer.

*/
    DbArray<HCUPoint> layer0, layer1, layer2, layer3, layer4;
/*
5 DBArrays to store the entities depending on their epsilon-value.

*/

  protected:

    bool canDestroy;
    double epsilon;
    double factor;
/*
A flag indicating if the destructor should destroy also the persistent
array of intervals.

*/

    Rectangle<3> bbox;
/*
Represents the bounding box of the hcmpoint.

*/
};

/*
3.8 HMPoint

the HierarchicalMovingPoint Type, contains a set of CUPoints from which
every Generalization of the corresponding temporalalgebra::MPoint can be 
extracted. This
type also contains the UPoints of the origin temporalalgebra::MPoint as 
CUPoint-Objects with an epsilon-value = 0.

*/
class HMPoint : public HCMPoint
{
  public:
/*
3.8.1 Constructors and Destructor

*/
  HMPoint() {}
/*
The simple constructor. This constructor should not be used.

*/
  HMPoint( const int n ):
    HCMPoint( 0 ), certainlayer( n )
  { }
/*
A constructor, initializing space for ~n~ entities in the bottom layer.

*/
  HMPoint( const double e, const double f, const temporalalgebra::MPoint& m ):
    HCMPoint( 0, e, f ), certainlayer( m.GetNoComponents() )
  {
  }
/*
This constructor creates a new HMPoint from the given 
temporalalgebra::MPoint-object. The units
of the temporalalgebra::MPoint will be stored in hierarchical entities 
in the DBArray layer5.
The layers 4 to 0 will be filled with generalizations of this i
temporalalgebra::MPoint, which
are computed using the given values epsilon and faktor.

*/
  ~HMPoint()
  {
    if( canDestroy )
      certainlayer.Destroy();
  }
/*
The destructor.

*/

/*
3.8.2 member functions

*/

  void clear()
  {
    HCMPoint::Clear();
    certainlayer.clean();
  }

/*
It is necessary to overload the function IsEmpty to ensure that it uses the
right function GetNoComponents.

*/
  inline bool IsEmpty() const
  {
    return (GetNoComponents() == 0);
  }

  int GetNoComponents() const
  {
    int noComponents = HCMPoint::GetNoComponents() + certainlayer.Size();
    return noComponents;
  }

  void Get( const int layer, const int i, HCUPoint& hcup ) const;

  void Get( const int layer, const int i, CUPoint& cup ) const;

  //void Get( const int layer, const int i, temporalalgebra::UPoint 
  // const*& up ) const;

  void Get( const int i, HCUPoint& ntt ) const;

  void GetCMPoint( const double epsilon, CMPoint& result );

  void GetMPoint( temporalalgebra::MPoint& result );

  void Put( const int layer, const int i, HCUPoint& hcup);

  inline int LayerSize( const int layer ) const;

  void ResizeLayer( const int layer, const int n );

  void Add( const HCUPoint& hcup );

  int Position( int layer, const Instant& t, const int start, const int end );

  int Generalize(const int layer, const bool checkBreakPoints,
                  const DateTime dur);


  void Simplify(const int min, const int max, const int layer, bool* useleft,
              bool* useright, double* realepsilon, const double epsilon);

  bool D_Passes( const Point& p );
  bool D_Passes( const int layer, const int start, const int end,
                  const Point& p );

  bool D_Passes( const Region& r );
  bool D_Passes( const int layer, const int start, const int end,
                  const Region& r );

  void D_At( const Point& p, temporalalgebra::MPoint& result );
  bool D_At( const int layer, const int start, const int end,
                  const Point& p, temporalalgebra::MPoint& result);

  void D_At( const Region& r, temporalalgebra::MPoint& result );
  bool D_At( const int layer, const int start, const int end,
                  const Region& r, temporalalgebra::MPoint& result );

  void AtInstant( const Instant& t, temporalalgebra::Intime<Point>& result );
  int AtInstant( const int layer, const int start, const int end,
                  const Instant& t, temporalalgebra::Intime<Point>& result );

  void AtPeriods( const temporalalgebra::Periods& p, 
                  temporalalgebra::MPoint& result);
  int AtPeriods( const int layer, const int start, const int end,
                 const temporalalgebra::Periods& p, 
                 temporalalgebra::MPoint& result );

  void ReduceHierarchy( const double epsilon, HCMPoint& result );
/*
3.8.3 functions to be part of relations

*/
  inline ostream& Print( ostream &os ) const;

  inline int Compare( const Attribute *arg ) const;


  inline Attribute* Clone() const;


  inline void CopyFrom( const Attribute* right );


  inline size_t Sizeof() const
  {
    return sizeof( *this );
  }


  inline int NumOfFLOBs() const
  {
    return 6;
  }


  inline Flob* GetFLOB( const int i);

  static const string BasicType() {return "hmpoint"; }
  static const bool checkType(const ListExpr type){
     return listutils::isSymbol(type,BasicType());
  }


/*
3.8.4 Attributes

*/
  DbArray<HCUPoint> certainlayer;
/*
The DBArray ~certainlayer~ stores only HCUPoint-objects with an epsilon-value
of 0.

*/
};


/*
3.9 Some auxiliary functions

*/

void Circle( const Point p, const double radius, const int n, Region& result);
/*
Computes a cirular shaped region around Point p, with the given radius. This
region consists of n pairs of halfsegments.

*/

bool FindDefPassingPoint( const HalfSegment& chs, const HalfSegment& rgnhs,
                    const double epsilon, Point& defPP);
/*
This function tries to find a point on halfsegment chs, that has exactly the
distance of espilon to the halfsegment rgnhs and lies on the inner side of
rgnhs. Therefore rgnhs must belong to a region, so that the Attribute
insideAbove is evaluable. If such a point exists, the parameter defPP is set
to this value and TRUE is returned. Otherwise FALSE is returned.

*/

bool FindPosPassingPoint( const HalfSegment& chs, const HalfSegment& rgnhs,
                    const double epsilon, Point& posPP);
/*
Does the same as 'FindDefPassingPoint', but it tries to find that point on the
outer side of the halfsegment rgnhs. If such a point exists, the parameter
posPP is set to this value and TRUE is returned. Otherwise FALSE is returned.

*/

void Generalize( const double epsilon, const double factor,
                 const temporalalgebra::MPoint& source, 
                 const bool checkBreakPoints,
                 const DateTime dur, HMPoint& result);
/*
This Function inserts a temporalalgebra::MPoint-object into the 
certainlayer of a HMPoint and
computes up to 5 generalizations of this temporalalgebra::MPoint 
which are stored in the layers
0 to 4.

*/

void Generalize( const double epsilon, const double factor, const int layer,
                  const CMPoint& source, const bool checkBreakPoints,
                  const DateTime dur, HMPoint& result);


static bool IsBreakPoint(const CUPoint* u,const DateTime& duration);
/*
This is an auxiliary function for the 'generalize'-algorithm.

*/

static bool connected(const CUPoint* u1, const temporalalgebra::UPoint* u2);
/*
This function checks whether the end point of the first unit is equal
to the start point of the second unit and if the time difference is
at most a single instant.


4 Type Constructors


4.1 Type Constructor ~uncertain~

Type ~uncertain~ represents a pair ( epsilon ( <Alpha> )).

4.1.1 List Representation

The list representation of an ~uncertain~ is

----    ( epsilon ( <Alpha> ) )
----

For example a cpoint:

----    ( 20.5 ( 329.456 22.289 ) )
----

4.2 Function describing the signature of the Type Constructor

4.2.2 ~In~-function

*/
/*template <class Alpha, Word (*InFun)( const ListExpr,
                                         const ListExpr,
                                         const int, ListExpr&, bool& )>
Word InUncertain( const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct)
{
  string errmsg;
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr first = nl->First( instance );             // the epsilon value
    ListExpr second = nl->Second( instance );  // the Alpha-Object
    {
      if ( nl->IsAtom(first) && nl->AtomType(first) == RealType )
      {
        correct = true;

        Alpha *value = (Alpha *)InFun( nl->TheEmptyList(),
                                        second, errorPos,
                                        errorInfo, correct ).addr;
        if ( correct == false )
        {
          delete value;
          return SetWord( Address(0) );
        }
        else  // if correct
        {
          Uncertain<Alpha> *uncertain = new Uncertain<Alpha> (
                                        nl->RealValue(first), *value);

          delete value;
          return SetWord( uncertain );
        }
      }
      else
      {
        errmsg = "InUncertain(): First arg must be of type Real.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      }
    }
  }
  correct = false;
  return SetWord(Address (0) );
}*/


/*
4.2.3 ~Out~-function

*/
/*template <class Alpha, ListExpr (*OutFun)( ListExpr, Word )>
ListExpr OutUncertain( ListExpr typeInfo, Word value )
{
  Uncertain<Alpha>* uncertain = (Uncertain<Alpha>*)(value.addr);

  if( uncertain->IsDefined() )
    return nl->TwoElemList(
      nl->RealAtom( &uncertain->GetEpsilon()),
      OutFun( nl->TheEmptyList(), SetWord( &uncertain->value ) ) );
      // Up to now the previous line sems to contain an error:
      // If the OutUncertain-function is mentioned in the typeconstructor
      // ~uncertainpoint~ (see HierarchicalGeoAlgebra.cpp line 316)
      // the compiler returns the following error message:
      //     'HierarchicalGeoAlgebra.h: In function ‘ListExpr OutUncertain
      //      (ListExpr, Word) [with Alpha = Point, ListExpr (* OutFun)
      //      (ListExpr, Word) = OutPoint]’:
      //      HierarchicalGeoAlgebra.cpp:328:   instantiated from here
      //      HierarchicalGeoAlgebra.h:613: error: invalid lvalue in unary ‘&'
      // I got no idea for the reason of this message. (Sascha Vaut)
  else
    return nl->SymbolAtom("undef");
}*/

/*
4.2.4 ~Create~-function

*/
//template <class Alpha>
Word CreateUncertain( const ListExpr typeInfo )
{
  return (SetWord( new Uncertain() ));
}


/*
4.2.5 ~Delete~-function

*/
//template <class Alpha>
void DeleteUncertain( const ListExpr typeInfo, Word& w )
{
  delete (Uncertain *)w.addr;
  w.addr = 0;
}

/*
4.2.6 ~Close~-function

*/
//template <class Alpha>
void CloseUncertain( const ListExpr typeInfo, Word& w )
{
  delete (Uncertain *)w.addr;
  w.addr = 0;
}

/*
4.2.7 ~Clone~-function

*/
//template <class Alpha>
Word CloneUncertain( const ListExpr typeInfo, const Word& w )
{
  Uncertain *uncertain = (Uncertain *)w.addr;
  return SetWord( new Uncertain( *uncertain ) );
}

/*
4.2.8 ~Sizeof~-function

*/
//template <class Alpha>
int SizeOfUncertain()
{
  return sizeof(Uncertain);
}

/*
4.2.9 ~Cast~-function

*/
//template <class Alpha>
void* CastUncertain(void* addr)
{
  return new (addr) Uncertain;
}


/*
4.3 Value mapping functions for class Uncertain


4.3.1 Value mapping functions of operator ~epsilon~

*/

//template <class Alpha>
int UncertainEpsilon( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  Uncertain* u = (Uncertain*)args[0].addr;

  if( u->UncertainIsDefined() )
  {
    // +++++ for debugging purposes only +++
    cout << "epsilon ist = " << u->GetEpsilon() << "\n";

    ((CcReal*)result.addr)->Set( u->GetEpsilon() );
  }
  else
    ((CcReal*)result.addr)->SetDefined( false );

  return 0;
}




