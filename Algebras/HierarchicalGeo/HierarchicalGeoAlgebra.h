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
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardAttribute.h"
#include "StandardTypes.h"
#include "TemporalAlgebra.h"
#include "SpatialAlgebra.h"
#include "MovingRegionAlgebra.h"
#include "DBArray.h"
#include "RectangleAlgebra.h"
#include "DateTime.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace datetime;






/*
3 C++ Classes (Definition)

3.1 Uncertain

This class represents an epsilon-value of type real. It will be used in type 
constructors of all ~uncertain~ Datatypes.

*/
template <class Alpha>
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

  Uncertain( bool is_defined):defined(is_defined) {}
  
/*
Use this constructor when declaring uncertain object variables etc.

*/

  Uncertain( const double& epsilon ):
    epsilon ( epsilon ), defined ( true ) 
    {}

/*
The creation of an uncertain value, setting the epsilon value.

*/

  virtual ~Uncertain() {}

/*
3.1.2 Member functions

*/
  
  virtual bool IsValid() const
  {
    if (epsilon >= 0 && defined)
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

/*
Returns the epsilon value of the Uncertain value.

*/

  virtual bool UncertainIsDefined() const
  {
    return defined;
  }

/*
Checks if the Uncertain value is defined or not. If it is not defined, only the
epsilon value is set. The Alpha value is left to be set later.

*/

  virtual void UncertainSetDefined( bool def )
  {
    this->defined = def;
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

  bool defined;
/*
The flag that indicates if the value is defined or not.

*/

};

/*
3.2 CBool

This datatype represents an ~uncertain~ boolean value. To define a ~boolean 
uncertainty~ a 3rd State (beyond the two known States TRUE and FALSE), called 
MAYBE, is introduced. 

*/

#define CBool int
#define maybe 2


/*
3.3 CPoint

CPoint represents a Point value containing an epsilon value. It implements 
Uncertain.

*/
struct CPoint : public Uncertain<Point>
{
  public:
/*
3.3.1 Constructors and Destructor
  
*/

  inline CPoint() {}
  
/*
The default constructor which should not be used.

*/
  
  CPoint(const double epsilon):
      Uncertain<Point>( epsilon ) 
      {}
      
/*
The undefined constructor. Only the epsilon value is set. The point value is 
left undefined for later definition.

*/
  
  CPoint( const double epsilon, const StandardAttribute* point):
      Uncertain<Point>( epsilon )
      {
        p.CopyFrom(point);
        p.SetDefined( defined );
      }
/*
The copy-constructor.

*/
  
  inline ~CPoint() {}
/*
The destructor.

3.3.2 Member functions

*/
  
  bool IsDefined() const
  {
    return UncertainIsDefined() && p.IsDefined();
  }
  
  bool IsValid() const
  {
    return Uncertain<Point>::IsValid() && p.IsDefined();
  }
  
  int ToCPoint( Word* args, Word& result, int message, Word& local,
                                        Supplier s );

/*
Transforms a given Point and a given positive real-value to a new cpoint-value.

*/
    
  void Set( CPoint cp )
  {
    this->epsilon = cp.epsilon;
    this->p = cp.p;
    this->defined = true;
    this->p.SetDefined( true );
  }
  
//  inline const Rectangle<2> BoundingBox() const;
/*
Returns the bounding box of the uncertain point, i.e. a rectangle area, 
bounding the area where the point may be.

*/
  
/*
3.3.3 Attributes

*/
  Point p;
  
};


/*
3.4 CUPoint

This class will be used in the ~cupoint~ type constructor, i.e., the type 
constructor for the uncertain temporal unit of point values.

*/
class CUPoint : public Uncertain<Point>, 
                         public SpatialTemporalUnit<Point, 3>
{
  public:
/*
3.4.1 Constructors and Destructor

*/
  CUPoint() {}
/*
The simple constructor. This constructor should not be used.

*/
  
  CUPoint( const bool is_defined ):
      Uncertain<Point>(is_defined),
      SpatialTemporalUnit<Point, 3>(false) {
    del.refs=1;
    del.isDelete=true;
    
    // +++++ nur zu Testzwecken: +++
    cout << "del.refs = " << int(del.refs) << "\n";
    if (del.isDelete)
      cout << "del.isDelete = TRUE \n";
    else
      cout << "del.isDelete = FALSE \n";
  }
  
  CUPoint( const double epsilon ):
      Uncertain<Point>(epsilon) {
    del.refs=1;
    del.isDelete=true;
    
    // +++++ nur zu Testzwecken: +++
    cout << "del.refs = " << int(del.refs) << "\n";
    if (del.isDelete)
      cout << "del.isDelete = TRUE \n";
    else
      cout << "del.isDelete = FALSE \n";
  }
/*
The simple constructor, only defining the epsilon-value. 
  
*/
  
  /*CUPoint( const double epsilon, const StandardAttribute* upoint):
      Uncertain<UPoint>( true, epsilon ) 
  {
    this.CopyFrom(upoint);
  }*/
/*
The copy-constructor.
  
*/
  CUPoint( const double epsilon, const Interval<Instant>& interval,
      const Point& p0, const Point& p1 ):
    Uncertain<Point> (epsilon),
    SpatialTemporalUnit<Point, 3>( interval ),
    p0( p0 ),
    p1( p1 )
    {
      del.refs=1;
      del.isDelete=true;
      
      // +++++ nur zu Testzwecken: +++
    cout << "del.refs = " << int(del.refs) << "\n";
    if (del.isDelete)
      cout << "del.isDelete = TRUE \n";
    else
      cout << "del.isDelete = FALSE \n";
    }

  CUPoint( const double epsilon, const Interval<Instant>& interval,
      const double x0, const double y0,
      const double x1, const double y1 ):
    Uncertain<Point> (epsilon),
    SpatialTemporalUnit<Point, 3>( interval ),
    p0( true, x0, y0 ),
    p1( true, x1, y1 )
    {
      del.refs=1;
      del.isDelete=true;
      
      // +++++ nur zu Testzwecken: +++
    cout << "del.refs = " << int(del.refs) << "\n";
    if (del.isDelete)
      cout << "del.isDelete = TRUE \n";
    else
      cout << "del.isDelete = FALSE \n";
    }
  
  //inline virtual ~CUPoint() {}
  
/*
The destructor.

3.4.2 Operator redefinitions

*/
  //virtual CUPoint& operator=( const CUPoint& i )
  //{
  //  epsilon = i.epsilon;
  //  UncertainSetDefined( i.IsDefined());
  //  timeInterval.CopyFrom( i.timeInterval );
  //  p0 = i.p0;
  //  p1 = i.p1;
  //  
  //  return *this;
  //}
/*
Redefinition of the copy operator ~=~.

*/

  //virtual bool operator==( const CUPoint& i ) const
  //{
  //  return epsilon == i.epsilon && value == i.value;
  //}
/*
Returns ~true~ if this cupoint is equal to the cupoint ~i~ and ~false~ if they 
are different.

*/
  
  //virtual bool operator!=( const CUPoint& i ) const
  //{
  //  return!( *this == i );
  //}
/*
Returns ~true~ if this cupoint is different to the cupoint ~i~ and ~false~ if 
they are equal.

*/
  
/*
3.4.3 The Temporal Functions

*/
// +++++ Todo: Funktionen in .cpp-Datei implementieren  +++

  virtual void TemporalFunction( const Instant& t,
                                 Point& result,
                                 bool ignoreLimits = false ) const;
  virtual bool Passes( const Point& val ) const {return false;}
  bool Passes( const Region& val ) const {return false;}
  virtual bool At( const Point& val, TemporalUnit<Point>& result ) const 
  {return false;}
  virtual void AtInterval( const Interval<Instant>& i,
                           TemporalUnit<Point>& result ) const;
  void Distance( const Point& p, UReal& result ) const {}
  //  void UTrajectory( UPoint& unit,Line& line ) const;
  void UTrajectory( Region& region ) const;
  void USpeed( UReal& result ) const {}
  void UVelocity( UPoint& result ) const {}
  void Intersection(const UPoint &other, UPoint &result) const {}
  
  virtual bool EqualValue( const CUPoint& i )
  {
  return   AlmostEqual( p0, i.p0 ) &&
           AlmostEqual( p1, i.p1 );
  }

  void Translate(const double x, const double y, 
                 const DateTime& duration) {}
  
  
/*
3.4.4 Functions to be part of relations

*/
  
  void UnitSetDefined( bool def ) 
  {
    SpatialTemporalUnit<Point, 3>::SetDefined( def );
  }
  
  bool UnitIsDefined() const
  {
    return SpatialTemporalUnit<Point, 3>::IsDefined();
  }
  
  void SetDefined( bool def )
  {
    UnitSetDefined( def );
    UncertainSetDefined( def );
  }
  
  bool IsDefined() const
  {
    return UnitIsDefined() && UncertainIsDefined();
  }
  
  bool UnitIsValid() const
  {
    return SpatialTemporalUnit<Point, 3>::IsValid();
  }
  
  bool IsValid() const
  {
    return UnitIsValid() && Uncertain<Point>::IsValid();
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
    if(cmp){
       return cmp;
    }
    if(p0<up2->p0){
      return -1;
    }
    if(p0>up2->p0){
      return 1;
    }
    if(p1>up2->p1){
       return 1;
    }
    if(p1<up2->p1){
       return -1;
    }
    return 0;
  }
  
  inline virtual bool Adjacent( const Attribute* arg ) const
  {
    return false;
  }
  
  inline virtual ostream& Print( ostream &os ) const
  {
// +++++++ ueberarbeiten! ++++++++++++++++++
    if(UnitIsDefined())
      {
        os << "CUPoint: " << "( ";
        //epsilon.Print(os);
        //os << ", ";
        timeInterval.Print(os);
        os << ", ";
        p0.Print(os);
        os << ", ";
        p1.Print(os);
        os << " ) ";
        return os;
      }
    else
      return os << "UPoint: (undef) ";
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
  
  virtual void CopyFrom( const StandardAttribute* right )
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
        timeInterval = Interval<Instant>();
        p0 = Point( false, 0.0, 0.0);
        p1 = Point( false, 0.0, 0.0);
      }
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
/*
For this is an uncertain UPoint-value, the returned Rectangle<3> is enlarged by
the epsilon-value.


3.4.4 Attributes

*/

  Point p0, p1;
  
};

/*
3.5 CMPoint

the implementation of an uncertain MPoint

*/
class CMPoint : public Uncertain<Point>, 
                         public Mapping< CUPoint, Point >
{
  public:
/*
3.5.1 Constructors and Destructor

*/
  CMPoint() {}
/*
The simple constructor. This constructor should not be used.
  
*/
  CMPoint( const int n ):
      Uncertain<Point>( true ),    // +++++ nur zu Testzwecken true uebergeben!
      Mapping< CUPoint, Point >( n )
      {
        // +++++ nur zu Testzwecken initialisiert: +++
        epsilon = 0.0;
        del.refs=1;
        del.isDelete=true;
        bbox = Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
      }
/*
The constructor. Initializes space for ~n~ elements.

*/

/*
3.5.2 Modifications of Inherited Functions

Overwrites the function defined in Mapping, mostly in order to
maintain the object's bounding box. Also, some methods can be improved
using a check on bbox.

*/

  void Clear();
  void Add( const CUPoint& unit );
  void MergeAdd(const CUPoint& unit);
  void EndBulkLoad( const bool sort = true );
  void Restrict( const vector< pair<int, int> >& intervals );
  ostream& Print( ostream &os ) const;
  //bool operator==( const CMPoint& r ) const;
  bool Present( const Instant& t ) const;
  bool Present( const Periods& t ) const;
  void AtInstant( const Instant& t, Intime<Region>& result ) const;
  void AtPeriods( const Periods& p, CMPoint& result ) const;
  void Trajectory( Region& region );
  
  virtual Attribute* Clone() const
  {
    assert( IsOrdered() );
    CMPoint *result = new CMPoint( GetNoComponents() );
    if(GetNoComponents()>0){
      result->units.Resize(GetNoComponents());
    }
    result->StartBulkLoad();
    const CUPoint *unit;
    for( int i = 0; i < GetNoComponents(); i++ )
    {
      Get( i, unit );
      result->Add( *unit );
    }
    result->EndBulkLoad( false );
    return (Attribute*) result;
  }
  
  void CopyFrom( const StandardAttribute* right )
  {
    const CMPoint *r = (const CMPoint*)right;
    assert( r->IsOrdered() );
    Clear();
    StartBulkLoad();
    const CUPoint *unit;
    for( int i = 0; i < r->GetNoComponents(); i++ )
    {
      r->Get( i, unit );
      Add( *unit );
    }
    EndBulkLoad( false );
  }
  
/*
3.5.3.1 Operation ~trajectory~

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( n )$, where ~n~ is the number of units of this ~MPoint~

*/
  //void Trajectory( Line& line ) const;
  
/*
3.5.3.2 Operation ~distance~

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( n )$, where ~n~ is the number of units of this ~MPoint~

*/
  //void Distance( const Point& p, MReal& result ) const;
  
/*
3.5.3.3 Operation ~BreakPoints~

*Precondition*: ~X.IsOrdered()~
*Semantics*: Computes all points where this mpoints stays longer than the given
             time.

*/
  //void BreakPoints(Points& result, const DateTime& dur) const;
  
/*
3.5.3.4 ~Append~

The ~Append~ function appends all units of the argument to this 
MPoint. If this mpoint or the argument is undefined or if the
argument starts before this mpoint ends, this mpoint will be set
to be undefined. The return value is the defined state of this 
mpoint after the operation (indicating the success). 

*/
  //bool Append(const MPoint& p, const bool autoresize = true);
  
/*
3.5.3.5 ~length~

Determines the drive distance of this moving point.
Will return a value smaller than zero if this mpoint is not defined

*/
  //double Length() const;
  
/*
3.5.3.6 ~epsilon~  

*/
  void RestoreEpsilon();
  
/*
3.5.3.7 ~BoundingBox~

Returns the MPoint's minimum bounding rectangle

*/
  // return the stored bbox
  Rectangle<3> BoundingBox() const;
  
  // recompute bbox, if necessary
  void RestoreBoundingBox(const bool force = false);
  
  private:
  
  Rectangle<3> bbox;
};

/*
3.6 HierarchicalMapping

a template class to bind all (uncertain) representations of one object to one 
HierarchicalMapping object. 

Attributes: 
- iDX: DBArray
- eLEM: DBArray
- canDestroy : bool
- ordered : bool

*/


/*
3.7 HMPoint

the HierarchicalMovingPoint Type, containing a set of CUPoints from which
every Generalization of the corresponding MPoint can be extracted. This
type also contains the UPoints of the origin MPoint as CUPoint-Objects with
an epsilon-value = 0.

*/

/*
3.8 HCMPoint

the type HierarchicalUncertainMovingPoint is a restricted variation of the
type HierarchicalMovingPoint. It just contains those UPoints of the origin
MPoint, that are necessary to build a particular minimal Generalization of
this MPoint.

*/


/*
3.9 Some auxiliary functions

*/
void Circle( const Point p, const double radius, const int n, Region& result);


/*
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
template <class Alpha>
Word CreateUncertain( const ListExpr typeInfo )
{
  return (SetWord( new Uncertain<Alpha>() ));
}


/*
4.2.5 ~Delete~-function

*/
template <class Alpha>
void DeleteUncertain( const ListExpr typeInfo, Word& w )
{
  delete (Uncertain<Alpha> *)w.addr;
  w.addr = 0;
}

/*
4.2.6 ~Close~-function

*/
template <class Alpha>
void CloseUncertain( const ListExpr typeInfo, Word& w )
{
  delete (Uncertain<Alpha> *)w.addr;
  w.addr = 0;
}

/*
4.2.7 ~Clone~-function

*/
template <class Alpha>
Word CloneUncertain( const ListExpr typeInfo, const Word& w )
{
  Uncertain<Alpha> *uncertain = (Uncertain<Alpha> *)w.addr;
  return SetWord( new Uncertain<Alpha>( *uncertain ) );
}

/*
4.2.8 ~Sizeof~-function

*/
template <class Alpha>
int SizeOfUncertain()
{
  return sizeof(Uncertain<Alpha>);
}

/*
4.2.9 ~Cast~-function

*/
template <class Alpha>
void* CastUncertain(void* addr)
{
  return new (addr) Uncertain<Alpha>;
}


/*
4.3 Value mapping functions for class Uncertain


4.3.1 Value mapping functions of operator ~epsilon~

*/
template <class Alpha>
int UncertainEpsilon( Word* args, Word& result, int message, Word& local, 
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  Uncertain<Alpha>* u = (Uncertain<Alpha>*)args[0].addr;

  if( u->UncertainIsDefined() )
    ((CcReal*)result.addr)->Set( u->epsilon );
  else
    ((CcReal*)result.addr)->SetDefined( false );

  return 0;
}




