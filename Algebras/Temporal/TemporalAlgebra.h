/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header file of the Temporal Algebra

January 2004 Victor Almeida

March - April 2004 Zhiming Ding

[TOC]

1 Overview

The type system of the Temporal Algebra can be seen below.

\begin{displaymath}
\begin{array}{lll}
	& \to \textrm{BASE} 	& {\underline{\smash{\mathit{int}}}}, {\underline{\smash{\mathit{real}}}},
				  {\underline{\smash{\mathit{bool}}}}, {\underline{\smash{\mathit{string}}}} \\
	& \to \textrm{SPATIAL} 	& {\underline{\smash{\mathit{point}}}}, {\underline{\smash{\mathit{points}}}},
				  {\underline{\smash{\mathit{line}}}}, {\underline{\smash{\mathit{region}}}} \\
	& \to \textrm{TIME} 	& {\underline{\smash{\mathit{instant}}}} \\
\textrm{BASE} \cup \textrm{TIME} 	& \to \textrm{RANGE} 	& {\underline{\smash{\mathit{range}}}} \\
\textrm{BASE} \cup \textrm{SPATIAL} 	& \to \textrm{TEMPORAL}	& {\underline{\smash{\mathit{intime}}}},
								  {\underline{\smash{\mathit{moving}}}} 
\end{array}
\end{displaymath}

2 Defines, includes, and constants

*/
#ifndef _TEMPORAL_ALGEBRA_H_
#define _TEMPORAL_ALGEBRA_H_

#include <iostream>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardAttribute.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "NestedList.h"
#include "DBArray.h"

ListExpr OutInstant( ListExpr typeinfo, Word value );
Word InInstant( ListExpr typeInfo, ListExpr value, int errorPos, ListExpr& errorInfo, bool& correct );
/*
3 C++ Classes (Defintion)

*/

/*
3.1 Instant

This class represents a time instant, or a point in time. It will be
used in the ~instant~ type constructor.

*/
typedef CcReal Instant;

/*
3.2 Interval

The class ~Interval~ implements the closure of an $\alpha$-interval. To be a generic
class, this class uses templates with parameter ~Alpha~. An interval contains a
~start~, an ~end~ and two flags ~lc~ and ~rc~ indicating if the interval is
left-closed and right-closed (or left-right-closed), respectively.

*/
template <class Alpha>
struct Interval
{
/*
3.2.1 Constructors

*/

  Interval() {}
/*
The simple constructor. This constructor should not be used.

*/

  Interval( Interval<Alpha>& interval );
/*
The copy constructor.

*/

  Interval( Alpha& start,
            Alpha& end,
            const bool lc,
            const bool rc );
/*
The creation of the interval setting all attributes. 

3.2.2 Member functions

*/
  
  void CopyFrom( Interval<Alpha>& interval );

  bool IsValid();
/*
Checks if the interval is valid or not. This function should be used for debugging purposes
only.  An interval is valid if the following conditions are true:

  1 ~start~ and ~end~ are defined

  2 ~start~ $<=$ ~end~

  3 if ~start~ $==$ ~end~, then must ~lc~ $=$ ~rc~ $=$ ~true~

*/

  Interval<Alpha>& operator=( const Interval<Alpha>& i );
/*
Redefinition of the copy operator ~=~.

*/

  bool operator==( Interval<Alpha>& i );
/*
Returns ~true~ if this interval is equal to the interval ~i~ and ~false~ if they are different.

*/

  bool operator!=( Interval<Alpha>& i );
/*
Returns ~true~ if this interval is different to the interval ~i~ and ~false~ if they are equal.

*/

  bool R_Disjoint( Interval<Alpha>& i );
/*
Returns ~true~ if this interval is r-disjoint with the interval ~i~ and ~false~ otherwise.

*/

  bool Disjoint( Interval<Alpha>& i );
/*
Returns ~true~ if this interval is disjoint with the interval ~i~ and ~false~ otherwise.

*/

  bool R_Adjacent( Interval<Alpha>& i );
/*
Returns ~true~ if this interval is r-adjacent with the interval ~i~ and ~false~ otherwise.

*/

  bool adjacent( Interval<Alpha>& i );  //DZM changed Adjacent() (used by relational functions) to adjacent().
/*
Returns ~true~ if this interval is adjacent with the interval ~i~ and ~false~ otherwise.

*/

  bool Inside( Interval<Alpha>& i );
/*
Returns ~true~ if this interval is inside the interval ~i~ and ~false~ otherwise.

*/

  bool Contains( Alpha& a );
/*
Returns ~true~ if this interval contains the value ~a~ and ~false~ otherwise.

*Precondition:* ~a.IsDefined()~

*/

  bool Intersects( Interval<Alpha>& i );
/*
Returns ~true~ if this interval intersects with the interval ~i~ and ~false~ otherwise.

*/

  bool Before( Interval<Alpha>& i );
/*
Returns ~true~ if this interval is before the interval ~i~ and ~false~ otherwise.

*/

  bool Before( Alpha& a );
  bool After( Alpha& a );
/*
Returns ~true~ if this interval is before/after the value ~a~ and ~false~ otherwise.

3.2.3 Attributes

*/
  Alpha start;
/*
The starting value of the interval.

*/

  Alpha end;
/*
The ending value of the interval.

*/

  bool lc;
/*
A flag indicating that the interval is left-closed.

*/

  bool rc;
/*
A flag indicating that the interval is right-closed.

*/

};

/*
3.3 Range

The ~Range~ class implements a set of disjoint, non-adjacent $\alpha$-~intervals~.
For this implementation, it is used a database array of ordered intervals.

*/
template <class Alpha>
class Range : public StandardAttribute
{
  public:
/*
3.3.1 Constructors and Destructor

*/
    Range() {}
/*
The simple constructor. This constructor should not be used.

*/

    Range( const int n );
/*
The constructor. Initializes space for ~n~ elements.

*/

    ~Range();
/*
The destructor.

*/

    void Destroy();
/*
This function should be called before the destructor if one wants to destroy the
persistent array of intervals. It marks the persistent array for destroying. The
destructor will perform the real destroying.

3.3.2 Functions for Bulk Load of Range

As said before, the point set is implemented as an ordered persistent array of intervals.
The time complexity of an insertion operation in an ordered array is $O(n)$, where ~n~
is the size of the interval set. In some cases, bulk load of intervals for example, it is good
to relax the ordered condition to improve the performance. We have relaxed this ordered
condition only for bulk load of intervals. All other operations assume that the interval set is
ordered.

*/

    bool IsOrdered() const;
/*
Returns if the interval set is ordered. There is a flag ~ordered~ (see attributes) in order
to avoid a scan in the interval set to answer this question.

*/

    void StartBulkLoad();
/*
Marks the start of a bulk load of intervals relaxing the condition that the intervals must be
ordered. We will assume that the only way to add intervals to an interval set is inside bulk
loads, i.e., into non-ordered ranges.

*/

    void EndBulkLoad( const bool sort = true );
/*
Marks the end of a bulk load and sorts the interval set if the flag ~sort~ is set to true.

3.3.3 Member functions

*/
    bool IsEmpty() const;
/*
Returns if the range is empty of intervals or not.

*/

    void Get( const int i, Interval<Alpha>& ai );
/*
Returns the interval ~ai~ at the position ~i~ in the range.

*/

    void Add( Interval<Alpha>& i );
/*
Adds an interval ~i~ to the range. We will assume that the only way of adding intervals 
is in bulk loads, i.e., in a non-ordered array.

*Precondition:* ~IsOrdered() == false~

*/

    void Clear();
/*
Remove all intervals in the range.

3.3.4 Functions to be part of relations

*/
    bool IsDefined() const;
    void SetDefined( bool Defined );
    int Compare( Attribute * arg );
    bool Adjacent( Attribute * arg );
    Range<Alpha>* Clone();
    ostream& Print( ostream &os );
    size_t HashValue();
    void CopyFrom( StandardAttribute* right );

    int NumOfFLOBs();
    FLOB *GetFLOB(const int i);

/*
3.3.5 Operations

3.3.5.1 Operation $=$ (~equal~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X = Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool operator==( Range<Alpha>& r );

/*
3.3.5.2 Operation $\neq$ (~not equal~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \neq Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool operator!=( Range<Alpha>& r );

/*
3.3.5.3 Operation ~intersects~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \cap Y \neq \emptyset$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool Intersects( Range<Alpha>& r );

/*
3.3.5.4 Operation ~inside~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \subseteq Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool Inside( Range<Alpha>& r );

/*
3.3.5.5 Operation ~contains~

*Precondition:* ~X.IsOrdered() $\&\&$ y.IsDefined()~

*Semantics:* $y \in X$

*Complexity:* $O(log(n))$, where ~n~ is the size of this range ~X~.

*/
    bool Contains( Alpha& a );

/*
3.3.5.6 Operation ~before~ (with ~range~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $\forall x \in X, \forall y \in Y: x \leq y$

*Complexity:* $O(1)$.

*/
    bool Before( Range<Alpha>& r );

/*
3.3.5.7 Operation ~before~ (with ~BASE~ type)

*Precondition:* ~X.IsOrdered() $\&\&$ y.IsDefined()~

*Semantics:* $\forall x \in X: x \leq y$

*Complexity:* $O(1)$.

*/
    bool Before( Alpha& a );

/*
3.3.5.8 Operation ~after~

This operation works only with ~BASE~ type, because it is an attempt to implement the operation
before on a contrary order, i.e., ~x before Y~.

*Precondition:* ~Y.IsOrdered() $\&\&$ x.IsDefined()~

*Semantics:* $\forall y \in Y: x \leq y$

*Complexity:* $O(1)$.

*/
    bool After( Alpha& a );

/*
3.3.5.9 Operation ~intersection~

*Precondition:* ~X.IsOrdered() $\&\&$ X.IsOrdered() $\&\&$ Result.IsEmpty()~

*Semantics:* $X \cap Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    void Intersection( Range<Alpha>& r, Range<Alpha>& result );

/*
3.3.5.10 Operation ~union~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered() $\&\&$ Result.IsEmpty()~

*Semantics:* $X \cup Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    void Union( Range<Alpha>& r, Range<Alpha>& result );

/*
3.3.5.11 Operation ~minus~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered() $\&\&$ Result.IsEmpty()~

*Semantics:* $X \backslash Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    void Minus( Range<Alpha>& r, Range<Alpha>& result );

/*
3.3.5.12 Operation ~max~

*Precondition:* ~X.IsOrdered()~

*Semantics:* $max(\rho(X))$

*Complexity:* $O(1)$

*/
    void Maximum( Alpha& result );

/*
3.3.5.13 Operation ~min~

*Precondition:* ~X.IsOrdered()~

*Semantics:* $min(\rho(X))$

*Complexity:* $O(1)$

*/
    void Minimum( Alpha& result );

/*
3.3.5.14 Operation ~no\_components~

*Precondition:* ~X.IsOrdered()~

*Semantics:* $\| intvls(X) \|$

*Complexity:* $O(1)$

*/
    int GetNoComponents() const;

  private:
/*
3.3.6 Private member functions

*/
    bool IsValid();
/*
This functions tests if a ~range~ is in a valid format. It is used for debugging
purposes only. The ~range~ is valid, if the following conditions are true:

  1 Each interval is valid

  2 Start of each interval $>=$ end of the interval before

  3 If start of an interval = end of the interval before, then one needs to
    make sure that the interval is not left-closed or the interval before
    is not right-closed

3.3.7 Attributes

*/
    bool canDestroy;
/*
A flag indicating if the destructor should destroy also the persistent
array of intervals.

*/

    bool ordered;
/*
A flag indicating whether the interval set is ordered or not.

*/

    DBArray< Interval<Alpha> > intervals;
/*
The intervals database array.

*/
};

/*
3.4 Intime

This class implements the ~intime~ type constructor, which converts a given type
$\alpha$ into a type that associates instants of time with values of $\alpha$.

3.4.1 Constructors

*/
template <class Alpha>
struct Intime: public StandardAttribute
{
  Intime() :
    instant( false, 0 )
    {}
/*
The simple constructor.

*/

  Intime( Instant& instant, Alpha& alpha ):
    instant( instant.IsDefined(), instant.GetRealval() ),
    value()
  {
    value.CopyFrom( &alpha ); 
  }
/*
The first constructor.

*/

  Intime( Intime& intime ):
    instant( intime.instant.IsDefined(), intime.instant.GetRealval() ),
    value()
  {
    value.CopyFrom( &intime.value );
  }
/*
The second constructor.

3.4.2 Functions to be part of relations

*/
  bool IsDefined() const
  {
      return true;
  }

  void SetDefined( bool Defined )
  {
  }

  int Compare( Attribute * arg )
  {
      return 0;
  }

  bool Adjacent( Attribute * arg )
  {
      return false;
  }

  Intime<Alpha>* Clone()
  {
      return (new Intime<Alpha>( *this));
  }

  ostream& Print( ostream &os )
  {
      return os << "Temporal Algebra---Intime" << endl;
  }

  size_t HashValue()
  {
      return 0;
  }

  void CopyFrom( StandardAttribute* right )
  {
      Intime<Alpha>* i = (Intime<Alpha>*)right;
      
      instant.Set(i->instant.IsDefined(), i->instant.GetRealval() );
      
      value.CopyFrom( &i->value ); 
  }

int NumOfFLOBs()
{
  return 1;
}

FLOB * GetFLOB(const int i)
{
  assert( i == 0 ); 
  return 0; //&units;
}
   
/*
3.4.3 Attributes:

*/
  Instant instant;
/*
The time instant associated.

*/

  Alpha value;
/*
The $\alpha$ value.

*/
};

/*
3.5 TemporalUnit

This class will generically implements a temporal unit. It is an abstract class
that enforces each kind of temporal unit to have a function that computes a value
inside the temporal unit given a time instant (also inside the temporal unit).

*/
template <class Alpha>
class TemporalUnit //: public StandardAttribute
{
  public:
    
/*
3.5.1 Constructors and Destructor

*/
    TemporalUnit() {}
    
    TemporalUnit( Interval<Instant>& interval ): timeInterval( interval ) {}

    virtual ~TemporalUnit() {}
    
/*
3.5.2 Member Functions

*/
    bool IsValid();
/*
Checks if the TemporalUnit is valid or not. This function should be used for debugging purposes
only.  A TemoralUnit is valid if its timeInterval is valid.

*/

    TemporalUnit& operator=( const TemporalUnit& i );
/*
Redefinition of the copy operator ~=~.

*/

    bool operator==( TemporalUnit& i );
/*
Returns ~true~ if this temporal unit is equal to the temporal unit ~i~ and ~false~ if they are different.

*/

    bool operator!=( TemporalUnit& i );
/*
Returns ~true~ if this temporal unit is different to the temporal unit ~i~ and ~false~ if they are equal.

*/

    bool R_Disjoint( TemporalUnit& i );
/*
Returns ~true~ if this temporal unit is r-disjoint with the temporal unit ~i~ and ~false~ otherwise.

*/

    bool Disjoint( TemporalUnit& i );
/*
Returns ~true~ if this temporal unit is disjoint with the temporal unit ~i~ and ~false~ otherwise.

*/

    bool R_Adjacent( TemporalUnit& i );
/*
Returns ~true~ if this temporal unit is r-adjacent with the temporal unit ~i~ and ~false~ otherwise.

*/

    bool adjacent( TemporalUnit& i ); //DZM changed Adjacent() <used by relational functions> to adjacent().
/*
Returns ~true~ if this temporal unit is adjacent with the temporal unit ~i~ and ~false~ otherwise.

*/
    
    
    bool Inside( TemporalUnit& i );
/*
Returns ~true~ if this temporal unit is inside the temporal unit ~i~ and ~false~ otherwise.

*/

    bool Contains( Instant& a );
/*
Returns ~true~ if this temporal unit contains the value ~a~ and ~false~ otherwise.

*Precondition:* ~a.IsDefined()~

*/

    bool Intersects( TemporalUnit& i );
/*
Returns ~true~ if this temporal unit intersects with the temporal unit ~i~ and ~false~ otherwise.

*/

    bool Before( TemporalUnit& i );
/*
Returns ~true~ if this temporal unit is before the temporal unit ~i~ and ~false~ otherwise.

*/

    bool Before( Instant& a );
    bool After( Instant& a );
/*
Returns ~true~ if this temporal unit is before/after the value ~a~ and ~false~ otherwise.

*/
    virtual bool TemporalFunction( Instant& t, Alpha& result ) = 0;
    
/*
The temporal function that receives a time instant ~t~ and returns the value 
associated with time ~t~ in the output argument ~result~.

*Precondition:* t must be inside the temporal unit time interval

3.3.4 Functions to be part of relations

*/
  
/*
3.5.3 Attributes

*/
    Interval<Instant> timeInterval;
/*
The time interval of the temporal unit.

*/
    
};

/*
3.6 ConstTemporalUnit

This class will be used in the ~const~ type constructor. It constructs constant
temporal units, i.e. it has a constant value and the temporal function always
return this value. The explicit purpose of the ~const~ type constructor is to
define temporal units for ~int~, ~string~, and ~bool~, i.e., for types where
their values change only in discrete steps.

*/
template <class Alpha>
class ConstTemporalUnit : public StandardAttribute, public TemporalUnit<Alpha> 
{
  public:
    
/*
3.6.1 Constructors, Destructor, and the Temp-Function

*/
    ConstTemporalUnit() {}
    
    ConstTemporalUnit( Interval<Instant>& interval, Alpha& a ):
    TemporalUnit<Alpha>( interval )
    {
      constValue.CopyFrom( &a );
    }

    bool TemporalFunction( Instant& t, Alpha& result )
    {
      if( !timeInterval.Contains( t ) )
        return false;
      result = constValue;
      return true;
    }
    
/*
3.6.2 Functions to be part of relations

*/
  bool IsDefined() const
  {
      return true;
  }

  void SetDefined( bool Defined )
  {
  }

  int Compare( Attribute * arg )
  {
      return 0;
  }

  bool Adjacent( Attribute * arg )
  {
      return false;
  }

  ostream& Print( ostream &os )
  {
      return os << "Temporal Algebra---constunit" << endl;
  }

  size_t HashValue()
  {
      return 0;
  }
  
  ConstTemporalUnit<Alpha>* Clone()
  {
      return (new ConstTemporalUnit<Alpha>( timeInterval, constValue));
  }
  
  void CopyFrom( StandardAttribute* right )
  {
      ConstTemporalUnit<Alpha>* i = (ConstTemporalUnit<Alpha>*)right;
      
      timeInterval.CopyFrom(i->timeInterval);
      
      constValue.CopyFrom( &i->constValue ); 
  }

  int NumOfFLOBs()
  {
      return 1;
  }

  FLOB * GetFLOB(const int i)
  {
      assert( i == 0 ); 
      return 0; //&units;
  }

    
/*
3.6.3 Attributes

*/
    Alpha constValue;
/*
The constant value of the temporal unit.

*/
};

/*
3.7 UReal

This class will be used in the ~ureal~ type constructor, i.e., the type constructor
for the temporal unit of real numbers.

*/
class UReal : public StandardAttribute,  public TemporalUnit<CcReal>
{
  public:
/*
3.7.1 Constructors and Destructor

*/
    UReal() {};
	
    UReal( Interval<Instant>& interval,
           const double a, 
           const double b,
           const double c,
           const bool r ):
    TemporalUnit<CcReal>( interval ),  //be careful about time-interval and real-interval, and check semantically the validity??????
    a( a ),
    b( b ),
    c( c ),
    r( r )
    {}
    
/*
3.7.2 Member Functions

*/
    bool TemporalFunction( Instant& t, CcReal& result )
    {
      assert( t.IsDefined() );

      if( !timeInterval.Contains( t ) )
        return false;

      double res = a * t.GetRealval() * t.GetRealval() + b * t.GetRealval() + c;
      if( r ) res = sqrt( res );

      result.Set( true, res );
      return true;
    }
/*
3.7.3 Functions to be part of relations

*/
  bool IsDefined() const
  {
      return true;
  }

  void SetDefined( bool Defined )
  {
  }

  int Compare( Attribute * arg )
  {
      return 0;
  }

  bool Adjacent( Attribute * arg )
  {
      return false;
  }

  ostream& Print( ostream &os )
  {
      return os << "Temporal Algebra---ureal" << endl;
  }

  size_t HashValue()
  {
      return 0;
  }
  
  UReal* Clone()
  {
      return (new UReal( timeInterval, a, b, c, r));
  }
  
  void CopyFrom( StandardAttribute* right )
  {
      UReal* i = (UReal*)right;
      
      timeInterval.CopyFrom(i->timeInterval);
      
      a=i->a;
      b=i->b;
      c=i->c;
      r=i->r;
  }

  int NumOfFLOBs()
  {
      return 1;
  }

  FLOB * GetFLOB(const int i)
  {
      assert( i == 0 ); 
      return 0; //&units;
  }

    
/*
3.7.4 Attributes

*/
    //private:
    double a, b, c;
    bool r;
};

/*
3.8 UPoint

This class will be used in the ~upoint~ type constructor, i.e., the type constructor
for the temporal unit of point values.

*/
class UPoint : public StandardAttribute, public TemporalUnit<Point>
{
  public:
/*
3.8.1 Constructors and Destructor

*/
    UPoint() {}; 
	
    UPoint( Interval<Instant>& interval, const double x0, const double x1, const double y0, const double y1 ): 
	    TemporalUnit<Point>( interval ), 
	    x0( x0 ),
	    x1( x1 ),
	    y0( y0 ),
	    y1( y1)
    {}
    
/*
3.8.2 Member Functions

*/
    bool TemporalFunction( Instant& t, Point& result )
    {
      assert( t.IsDefined() );

      if( !timeInterval.Contains( t ) )
        return false;

      double x = x0 + x1 * t.GetRealval();
      double y = y0 + y1 * t.GetRealval();
      
      result.Set( x, y );
      return true;
    }
    
/*
3.8.3 Functions to be part of relations

*/
  bool IsDefined() const
  {
      return true;
  }

  void SetDefined( bool Defined )
  {
  }

  int Compare( Attribute * arg )
  {
      return 0;
  }

  bool Adjacent( Attribute * arg )
  {
      return false;
  }

  ostream& Print( ostream &os )
  {
      return os << "Temporal Algebra---upoint" << endl;
  }

  size_t HashValue()
  {
      return 0;
  }
  
  UPoint* Clone()
  {
      return (new UPoint( timeInterval, x0, x1, y0, y1));
  }
  
  void CopyFrom( StandardAttribute* right )
  {
      UPoint* i = (UPoint*)right;
      
      timeInterval.CopyFrom(i->timeInterval);
      
      x0=i->x0;
      x1=i->x1;
      y0=i->y0;
      y1=i->y1;
  }

  int NumOfFLOBs()
  {
      return 1;
  }

  FLOB * GetFLOB(const int i)
  {
      assert( i == 0 ); 
      return 0; //&units;
  }

    
/*
3.8.4 Attributes

*/
    //private:  (parameters in computing the value inside the time interval)
    double x0, x1, y0, y1;
};

/*
3.9 Mapping

This class will implement the functionalities of the ~mapping~ type constructor.
It contains a database array of temporal units.

*/

template <class Alpha>
class Mapping
{
  public:

  private:

    DBArray< TemporalUnit<Alpha> > units;
    
/*
The database array of temporal units.

*/ 
};

/*
3.10 Mapping(UPoint)

This class is implement the functionalities of the ~mapping(UPoint)~ type constructor.
It contains a database array of temporal point units.

*/
class MPoint : public StandardAttribute
{
  public:
/*
3.10.1 Constructors and Destructor

*/
    MPoint() {}
/*
The simple constructor. This constructor should not be used.

*/

    MPoint( const int n );
/*
The constructor. Initializes space for ~n~ elements.

*/

    ~MPoint();
/*
The destructor.

*/

    void Destroy();
/*
This function should be called before the destructor if one wants to destroy the
persistent array of moving units. It marks the persistent array for destroying. The
destructor will perform the real destroying.

3.10.2 Functions for Bulk Load of moving units

As said before, the moving unit set is implemented as an ordered persistent array of units.
The time complexity of an insertion operation in an ordered array is $O(n)$, where ~n~
is the size of the unit set. In some cases, bulk load of units for example, it is good
to relax the ordered condition to improve the performance. We have relaxed this ordered
condition only for bulk load of units. All other operations assume that the unit set is
ordered.

*/

    bool IsOrdered() const;
/*
Returns if the unit set is ordered. There is a flag ~ordered~ (see attributes) in order
to avoid a scan in the unit set to answer this question.

*/

    void StartBulkLoad();
/*
Marks the start of a bulk load of units relaxing the condition that the units must be
ordered. We will assume that the only way to add units to an unit set is inside bulk
loads, i.e., into non-ordered mappings.

*/

    void EndBulkLoad( const bool sort = true );
/*
Marks the end of a bulk load and sorts the unit set if the flag ~sort~ is set to true.

3.10.3 Member functions

*/
    bool IsEmpty() const;
/*
Returns if the mapping is empty of units or not.

*/

    void Get( const int i, UPoint& upi );
/*
Returns the unit ~upi~ at the position ~i~ in the mapping.

*/

    void Add( UPoint& upi );
/*
Adds an unit ~upi~ to the mapping. We will assume that the only way of adding units 
is in bulk loads, i.e., in a non-ordered array.

*Precondition:* ~IsOrdered() == false~

*/

    void Clear();
/*
Remove all units in the mapping.

3.10.4 Functions to be part of relations

*/
    bool IsDefined() const;
    void SetDefined( bool Defined );
    int Compare( Attribute * arg );
    bool Adjacent( Attribute * arg );
    MPoint* Clone();
    ostream& Print( ostream &os );
    size_t HashValue();
    void CopyFrom( StandardAttribute* right );

    int NumOfFLOBs();
    FLOB *GetFLOB(const int i);
    
    int Position( const Instant& t );
    bool TemporalFunction( Instant& t, Point& result );

/*
3.10.5 Operations

3.10.5.1 Operation $=$ (~equal~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X = Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool operator==( MPoint& mp );

/*
3.10.5.2 Operation $\neq$ (~not equal~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \neq Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool operator!=( MPoint& mp );

/*
3.10.5.3 Operation ~no\_components~

*Precondition:* ~X.IsOrdered()~

*Semantics:* $\| intvls(X) \|$

*Complexity:* $O(1)$

*/
    int GetNoComponents() const;

//  private:
/*
3.10.6 Private member functions

*/
    bool IsValid();
/*
This functions tests if a ~mapping~ is in a valid format. It is used for debugging
purposes only. The ~mapping~ is valid, if the following conditions are true:

  1 Each unit is valid

  2 Start of each unit $>=$ end of the unit before

  3 If start of an unit = end of the unit before, then one needs to
    make sure that the unit is not left-closed or the unit before
    is not right-closed

3.10.7 Attributes

*/  
    private:
    
    bool canDestroy;
/*
A flag indicating if the destructor should destroy also the persistent
array of intervals.

*/

    bool ordered;
/*
A flag indicating whether the unit set is ordered or not.

*/

    DBArray< UPoint > units;
    
/*
The database array of temporal units.

*/     
};

/*
3.11 Mapping(ConstTemporalUnit(int))

This class is implement the functionalities of the ~mapping(ConstTemporalUnit(int))~ type constructor.
It contains a database array of temporal ConstTemporalUnit(Int) units.

*/
    
class MInt : public StandardAttribute
{
  public:
/*
3.11.1 Constructors and Destructor

*/
    MInt() {}
/*
The simple constructor. This constructor should not be used.

*/

    MInt( const int n );
/*
The constructor. Initializes space for ~n~ elements.

*/

    ~MInt();
/*
The destructor.

*/

    void Destroy();
/*
This function should be called before the destructor if one wants to destroy the
persistent array of intervals. It marks the persistent array for destroying. The
destructor will perform the real destroying.

3.11.2 Functions for Bulk Load of units

As said before, the point set is implemented as an ordered persistent array of intervals.
The time complexity of an insertion operation in an ordered array is $O(n)$, where ~n~
is the size of the interval set. In some cases, bulk load of intervals for example, it is good
to relax the ordered condition to improve the performance. We have relaxed this ordered
condition only for bulk load of intervals. All other operations assume that the interval set is
ordered.

*/

    bool IsOrdered() const;
/*
Returns if the interval set is ordered. There is a flag ~ordered~ (see attributes) in order
to avoid a scan in the interval set to answer this question.

*/

    void StartBulkLoad();
/*
Marks the start of a bulk load of intervals relaxing the condition that the intervals must be
ordered. We will assume that the only way to add intervals to an interval set is inside bulk
loads, i.e., into non-ordered ranges.

*/

    void EndBulkLoad( const bool sort = true );
/*
Marks the end of a bulk load and sorts the interval set if the flag ~sort~ is set to true.

3.11.3 Member functions

*/
    bool IsEmpty() const;
/*
Returns if the range is empty of intervals or not.

*/

    void Get( const int i, ConstTemporalUnit<CcInt>& ui );
/*
Returns the interval ~ai~ at the position ~i~ in the range.

*/

    void Add( ConstTemporalUnit<CcInt>& ui );
/*
Adds an interval ~i~ to the range. We will assume that the only way of adding intervals 
is in bulk loads, i.e., in a non-ordered array.

*Precondition:* ~IsOrdered() == false~

*/
    int Position( const Instant& t );
    bool TemporalFunction( Instant& t, CcInt& result );
    
    void Clear();
/*
Remove all intervals in the range.


3.11.4 Functions to be part of relations

*/
    bool IsDefined() const;
    void SetDefined( bool Defined );
    int Compare( Attribute * arg );
    bool Adjacent( Attribute * arg );
    MInt* Clone();
    ostream& Print( ostream &os );
    size_t HashValue();
    void CopyFrom( StandardAttribute* right );

    int NumOfFLOBs();
    FLOB *GetFLOB(const int i);

/*
3.11.5 Operations

3.11.5.1 Operation $=$ (~equal~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X = Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool operator==( MInt& mi );

/*
3.11.5.2 Operation $\neq$ (~not equal~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \neq Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool operator!=( MInt& mi );

/*
3.11.5.3 Operation ~no\_components~

*Precondition:* ~X.IsOrdered()~

*Semantics:* $\| intvls(X) \|$

*Complexity:* $O(1)$

*/
    int GetNoComponents() const;

//  private:
/*
3.11.6 Private member functions

*/
    bool IsValid();
/*
This functions tests if a ~range~ is in a valid format. It is used for debugging
purposes only. The ~range~ is valid, if the following conditions are true:

  1 Each interval is valid

  2 Start of each interval $>=$ end of the interval before

  3 If start of an interval = end of the interval before, then one needs to
    make sure that the interval is not left-closed or the interval before
    is not right-closed

3.11.7 Attributes

*/ 
    private:
    
    bool canDestroy;
/*
A flag indicating if the destructor should destroy also the persistent
array of intervals.

*/

    bool ordered;
/*
A flag indicating whether the interval set is ordered or not.

*/

    DBArray< ConstTemporalUnit<CcInt> > units;
    
/*
The database array of temporal units.

*/     
};

/*
3.12 Mapping(UReal)

This class is implement the functionalities of the ~mapping(UReal)~ type constructor.
It contains a database array of temporal real units.

*/
class MReal : public StandardAttribute
{
  public:
/*
3.12.1 Constructors and Destructor

*/
    MReal() {}
/*
The simple constructor. This constructor should not be used.

*/

    MReal( const int n );
/*
The constructor. Initializes space for ~n~ elements.

*/

    ~MReal();
/*
The destructor.

*/

    void Destroy();
/*
This function should be called before the destructor if one wants to destroy the
persistent array of intervals. It marks the persistent array for destroying. The
destructor will perform the real destroying.

3.12.2 Functions for Bulk Load of units

As said before, the point set is implemented as an ordered persistent array of intervals.
The time complexity of an insertion operation in an ordered array is $O(n)$, where ~n~
is the size of the interval set. In some cases, bulk load of intervals for example, it is good
to relax the ordered condition to improve the performance. We have relaxed this ordered
condition only for bulk load of intervals. All other operations assume that the interval set is
ordered.

*/

    bool IsOrdered() const;
/*
Returns if the interval set is ordered. There is a flag ~ordered~ (see attributes) in order
to avoid a scan in the interval set to answer this question.

*/

    void StartBulkLoad();
/*
Marks the start of a bulk load of intervals relaxing the condition that the intervals must be
ordered. We will assume that the only way to add intervals to an interval set is inside bulk
loads, i.e., into non-ordered ranges.

*/

    void EndBulkLoad( const bool sort = true );
/*
Marks the end of a bulk load and sorts the interval set if the flag ~sort~ is set to true.

3.12.3 Member functions

*/
    bool IsEmpty() const;
/*
Returns if the range is empty of intervals or not.

*/

    void Get( const int i, UReal& uri );
/*
Returns the interval ~ai~ at the position ~i~ in the range.

*/

    void Add( UReal& uri );
/*
Adds an interval ~i~ to the range. We will assume that the only way of adding intervals 
is in bulk loads, i.e., in a non-ordered array.

*Precondition:* ~IsOrdered() == false~

*/

    void Clear();
/*
Remove all intervals in the range.

3.12.4 Functions to be part of relations

*/
    bool IsDefined() const;
    void SetDefined( bool Defined );
    int Compare( Attribute * arg );
    bool Adjacent( Attribute * arg );
    MReal* Clone();
    ostream& Print( ostream &os );
    size_t HashValue();
    void CopyFrom( StandardAttribute* right );

    int NumOfFLOBs();
    FLOB *GetFLOB(const int i);
    int Position( const Instant& t );
    bool TemporalFunction( Instant& t, CcReal& result );

/*
3.12.5 Operations

3.12.5.1 Operation $=$ (~equal~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X = Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool operator==( MReal& mr );

/*
3.12.5.2 Operation $\neq$ (~not equal~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \neq Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool operator!=( MReal& mr );

/*
3.12.5.3 Operation ~no\_components~

*Precondition:* ~X.IsOrdered()~

*Semantics:* $\| intvls(X) \|$

*Complexity:* $O(1)$

*/
    int GetNoComponents() const;

//  private:
/*
3.12.6 Private member functions

*/
    bool IsValid();
/*
This functions tests if a ~range~ is in a valid format. It is used for debugging
purposes only. The ~range~ is valid, if the following conditions are true:

  1 Each interval is valid

  2 Start of each interval $>=$ end of the interval before

  3 If start of an interval = end of the interval before, then one needs to
    make sure that the interval is not left-closed or the interval before
    is not right-closed

3.12.7 Attributes

*/
    private:
    
    bool canDestroy;
/*
A flag indicating if the destructor should destroy also the persistent
array of intervals.

*/

    bool ordered;
/*
A flag indicating whether the interval set is ordered or not.

*/

    DBArray< UReal > units;
           
/*
The database array of temporal units.

*/     
};

/*
4 Implementation of C++ Classes

4.1 Interval

4.1.1 Constructors and Destructor

*/
template <class Alpha>
Interval<Alpha>::Interval( Interval<Alpha>& interval ):
start(),
end(),
lc( interval.lc ),
rc( interval.rc )
{
  this->start.CopyFrom( &interval.start );
  this->end.CopyFrom( &interval.end );
}

template <class Alpha>
Interval<Alpha>::Interval( Alpha& start, Alpha& end,
                           const bool lc, const bool rc ):
start(),
end(),
lc( lc ),
rc( rc )
{
  this->start.CopyFrom( &start );
  this->end.CopyFrom( &end );
}

template <class Alpha>
void Interval<Alpha>::CopyFrom( Interval<Alpha>& interval )
{
  this->start.CopyFrom( &interval.start );
  this->end.CopyFrom( &interval.end );
  this->lc= interval.lc ;
  this->rc= interval.rc ;
}

template <class Alpha>
bool Interval<Alpha>::IsValid()
{
  if( !start.IsDefined() || !end.IsDefined() )
    return false;

  if( start.Compare( &end ) < 0 )
  {
    if( start.Adjacent( &end ) )
      return lc || rc;
  }
  else if( start.Compare( &end ) == 0 )
  {
    return rc && lc;
  }
  return true;
}

/*
4.1.2 Member functions

*/
template <class Alpha>
Interval<Alpha>& Interval<Alpha>::operator=( const Interval<Alpha>& i )
{
  assert( i.IsValid );

  start.CopyFrom( &i.start );
  end.CopyFrom( &i.end );
  lc = i.lc;
  rc = i.rc;

  return *this;
}

template <class Alpha>
bool Interval<Alpha>::operator==( Interval<Alpha>& i )
{
  assert( IsValid() && i.IsValid() );

  return( ( start.Compare( &i.start ) == 0 && lc == i.lc &&
            end.Compare( &i.end ) == 0 && rc == i.rc ) ||
          ( start.Compare( &i.start ) == 0 && lc == i.lc &&
            end.Compare( &i.end ) != 0 && end.Adjacent( &i.end ) && ( !rc || !i.rc ) ) ||
          ( end.Compare( &i.end ) == 0 && rc == i.rc &&
            start.Compare( &i.start ) != 0 && start.Adjacent( &i.start ) && ( !lc || !i.lc ) ) ||
          ( start.Compare( &i.start ) != 0 && start.Adjacent( &i.start ) && (!lc || !i.lc) &&
            end.Compare( &i.end ) != 0 && end.Adjacent( &i.end ) && ( !rc || !i.rc ) ) );
}

template <class Alpha>
bool Interval<Alpha>::operator!=( Interval<Alpha>& i )
{
  return !( *this == i );
}

template <class Alpha>
bool Interval<Alpha>::R_Disjoint( Interval<Alpha>& i )
{
    bool res= ((end.Compare( &i.start ) < 0) || ( end.Compare( &i.start ) == 0 && !( rc && i.lc ) ));
    return( res );
}

template <class Alpha>
bool Interval<Alpha>::Disjoint( Interval<Alpha>& i )
{
  assert( IsValid() && i.IsValid() );
  bool res=( R_Disjoint( i ) || i.R_Disjoint( *this ) );
  return( res );
}

template <class Alpha>
bool Interval<Alpha>::R_Adjacent( Interval<Alpha>& i ) 
{
    bool res=( Disjoint( i ) &&
	       ( end.Compare( &i.start ) == 0 && (rc || i.lc) ) ||
	       ( ( end.Compare( &i.start ) < 0 && rc && i.lc ) && end.Adjacent( &i.start ) ) );
    return( res );
}

template <class Alpha>
bool Interval<Alpha>::adjacent( Interval<Alpha>& i )
{
  assert( IsValid() && i.IsValid() );
  bool res= ( R_Adjacent( i ) || i.R_Adjacent( *this ) );
  return( res );
}

template <class Alpha>
bool Interval<Alpha>::Inside( Interval<Alpha>& i ) 
{
  assert( IsValid() && i.IsValid() );

  return( ( start.Compare( &i.start ) > 0 ||
            ( start.Compare( &i.start ) == 0 && ( !lc || i.lc ) ) ) &&
          ( end.Compare( &i.end ) < 0 ||
            ( end.Compare( &i.end ) == 0 && ( !rc || i.rc ) ) ) );
}

template <class Alpha>
bool Interval<Alpha>::Contains( Alpha& a ) 
{
  assert( IsValid() && a.IsDefined() );

  return ( ( start.Compare( &a ) < 0 ||
             ( start.Compare( &a ) == 0 && lc ) ) &&
           ( end.Compare( &a ) > 0 ||
             ( end.Compare( &a ) == 0 && rc ) ) );
}

template <class Alpha>
bool Interval<Alpha>::Intersects( Interval<Alpha>& i ) 
{
  assert( IsValid() && i.IsValid() );

  return !(
            ( ( (start.Compare( &i.start ) < 0) ||
                (start.Compare( &i.start ) == 0 && !(lc && i.lc)) ) &&
              ( (end.Compare( &i.start ) < 0) ||
                (end.Compare( &i.start ) == 0 && !(rc && i.lc)) ) ) ||
            ( ( (start.Compare( &i.end ) > 0) ||
                (start.Compare( &i.end ) == 0 && !(lc && i.rc)) ) &&
              ( (end.Compare( &i.end ) > 0) ||
                (end.Compare( &i.end ) == 0 && !(rc && i.rc)) ) )
          );
}

template <class Alpha>
bool Interval<Alpha>::Before( Interval<Alpha>& i ) 
{
  assert( IsValid() && i.IsValid() );

  return ( Before( i.start ) ||
           ( !Before( i.start ) && end.Compare( &i.start ) && rc == true ) );
}

template <class Alpha>
bool Interval<Alpha>::Before( Alpha& a ) 
{
  assert( IsValid() && a.IsDefined() );

  return ( end.Compare( &a ) <= 0 ||
           ( end.Compare( &a ) == 0 && rc == false ) );
}

template <class Alpha>
bool Interval<Alpha>::After( Alpha& a ) 
{
  assert( IsValid() && a.IsDefined() );

  return ( start.Compare( &a ) >= 0 ||
           ( start.Compare( &a ) == 0 && lc == false ) );
}

/*
4.2 Range

4.2.1 Constructors and destructor

*/
template <class Alpha>
Range<Alpha>::Range( const int n ):
canDestroy( false ),
ordered( true ),
intervals( n )
{}

template <class Alpha>
void Range<Alpha>::Destroy()
{
  canDestroy = true;
}

template <class Alpha>
Range<Alpha>::~Range()
{
  if( canDestroy )
    intervals.Destroy();
}

/*
4.2.2 Member functions

*/
template <class Alpha>
bool Range<Alpha>::IsOrdered() const
{
  return ordered;
}


template <class Alpha>
void Range<Alpha>::StartBulkLoad()
{
  assert( ordered );
  ordered = false;
}

template <class Alpha>
int IntervalCompare( const void *a, const void *b )
{
  Interval<Alpha> *inta = new ((void*)a) Interval<Alpha>,
                  *intb = new ((void*)b) Interval<Alpha>;

  if( *inta == *intb )
    return 0;
  else if( inta->Before( *intb ) )
    return -1;
  else
    return 1;
}

template <class Alpha>
void Range<Alpha>::EndBulkLoad( const bool sort )
{
  assert( !ordered );
  if( sort )
    intervals.Sort( IntervalCompare<Alpha> );
  ordered = true;
  assert( IsValid() );
}

template <class Alpha>
bool Range<Alpha>::IsEmpty() const
{
  return intervals.Size() == 0;
}

template <class Alpha>
void Range<Alpha>::Get( const int i, Interval<Alpha>& interval )
{
  intervals.Get( i, interval );
  assert( interval.IsValid() );
}

template <class Alpha>
void Range<Alpha>::Add( Interval<Alpha>& interval )
{
  assert( interval.IsValid() );
  intervals.Append( interval );
}

template <class Alpha>
void Range<Alpha>::Clear()
{
  ordered = true;
  intervals.Clear();
}

/*
4.2.3 Functions to be part of relations

*/
template <class Alpha>
bool Range<Alpha>::IsDefined() const
{
  return true;
}

template <class Alpha>
void Range<Alpha>::SetDefined( bool Defined )
{
}

template <class Alpha>
int Range<Alpha>::Compare( Attribute * arg )
{
  return 0;
}

template <class Alpha>
bool Range<Alpha>::Adjacent( Attribute * arg )
{
  return false;
}

template <class Alpha>
Range<Alpha>* Range<Alpha>::Clone()
{
  assert( IsOrdered() );

  Range *result = new Range( GetNoComponents() );

  result->StartBulkLoad();
  Interval<Alpha> interval;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, interval );
    result->Add( interval );
  }
  result->EndBulkLoad( false );

  return result;
}

template <class Alpha>
ostream& Range<Alpha>::Print( ostream &os )
{
  return os << "Range Algebra" << endl;
}

template <class Alpha>
size_t Range<Alpha>::HashValue()
{
  return 0;
}

template <class Alpha>
void Range<Alpha>::CopyFrom( StandardAttribute* right )
{
  Range<Alpha> *r = (Range<Alpha>*)right;
  assert( r->IsOrdered() );

  Clear();

  StartBulkLoad();
  Interval<Alpha> interval;
  for( int i = 0; i < r->GetNoComponents(); i++ )
  {
    r->Get( i, interval );
    Add( interval );
  }
  EndBulkLoad( false );
}

template <class Alpha>
int Range<Alpha>::NumOfFLOBs()
{
  return 1;
}

template <class Alpha>
FLOB *Range<Alpha>::GetFLOB(const int i)
{
  assert( i == 0 ); 
  return &intervals;
}

/*
4.2.4 Operator functions

*/
template <class Alpha>
bool Range<Alpha>::operator==( Range<Alpha>& r )
{
  assert( IsValid() && r.IsValid() );

  if( GetNoComponents() != r.GetNoComponents() )
    return false;

  bool result = true;
  Interval<Alpha> thisInterval, interval;

  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, thisInterval );
    r.Get( i, interval );

    if( thisInterval != interval )
    {
      result = false;
      break;
    }
  }

  return result;
}

template <class Alpha>
bool Range<Alpha>::operator!=( Range<Alpha>& r )
{
  return !( *this == r );
}

template <class Alpha>
bool Range<Alpha>::Intersects( Range<Alpha>& r )
{
  assert( IsValid() && r.IsValid() );

  if( IsEmpty() || r.IsEmpty() )
    return false;

  bool result = false;
  Interval<Alpha> thisInterval, interval;

  int i = 0, j = 0;
  Get( i, thisInterval );
  r.Get( j, interval );

  while( 1 )
  {
    if( thisInterval.Intersects( interval ) )
    {
      result = true;
      break;
    }

    if( thisInterval.Before( interval ) )
    {
      if( ++i == GetNoComponents() )
      {
        result = false;
        break;
      }
      Get( i, thisInterval );
    }

    if( interval.Before( thisInterval ) )
    {
      if( ++j == r.GetNoComponents() )
      {
        result = false;
        break;
      }
      r.Get( j, interval );
    }
  }

  return result;
}

template <class Alpha>
bool Range<Alpha>::Inside( Range<Alpha>& r )
{
  assert( IsValid() && r.IsValid() );

  if( IsEmpty() ) return true;
  if( r.IsEmpty() ) return false;

  bool result = true;
  Interval<Alpha> thisInterval, interval;

  int i = 0, j = 0;
  Get( i, thisInterval );
  r.Get( j, interval );

  while( 1 )
  {
    if( interval.Before( thisInterval ) )
    {
      if( ++j == r.GetNoComponents() )
      {
        result = false;
        break;
      }
      r.Get( j, interval );
    }
    else if( thisInterval.Inside( interval ) )
    {
      if( ++i == GetNoComponents() )
      {
        break;
      }
      Get( i, thisInterval );
    }
    else if( thisInterval.Before( interval ) )
    {
      result = false;
      break;
    }
    else
    {
      // Intersects but not inside.
      result = false;
      break;
    }
  }

  return result;
}

template <class Alpha>
bool Range<Alpha>::Contains( Alpha& a )
{
  assert( IsValid() && a.IsDefined() );

  if( IsEmpty() )
    return false;

  bool result = false;
  Interval<Alpha> midInterval;

  int first = 0, last = GetNoComponents() - 1;

  while (first <= last)
  {
    int mid = ( first + last ) / 2;
    Get( mid, midInterval );
    if( midInterval.Contains( a ) )
    {
      result = true;
      break;
    }
    else if( midInterval.Before( a ) )
      first = mid + 1;
    else if( midInterval.After( a ) )
      last = mid - 1;
    else
    {
      result = true;
      break;
    }
  }

  return result;
}

template <class Alpha>
bool Range<Alpha>::Before( Range<Alpha>& r )
{
  assert( IsValid() && r.IsValid() );
  assert( !IsEmpty() && !r.IsEmpty() );

  Interval<Alpha> thisInterval, interval;
  Get( GetNoComponents() - 1, thisInterval );
  r.Get( 0, interval );

  return thisInterval.Before( interval );
}

template <class Alpha>
bool Range<Alpha>::Before( Alpha& a )
{
  assert( IsValid() && !IsEmpty() && a.IsDefined() );

  Interval<Alpha> thisInterval;
  Get( GetNoComponents() - 1, thisInterval );
  return thisInterval.Before( a );
}

template <class Alpha>
bool Range<Alpha>::After( Alpha& a )
{
  assert( IsValid() && !IsEmpty() && a.IsDefined() );

  Interval<Alpha> thisInterval;
  Get( 0, thisInterval );

  return thisInterval.After( a );
}

template <class Alpha>
void Range<Alpha>::Intersection( Range<Alpha>& r, Range<Alpha>& result )
{
  assert( IsValid() && r.IsValid() && result.IsEmpty() );

  Interval<Alpha> thisInterval, interval;

  int i = 0, j = 0;
  Get( i, thisInterval );
  r.Get( j, interval );

  result.StartBulkLoad();
  while( i < GetNoComponents() && j < r.GetNoComponents() )
  {
    if( thisInterval.start.Compare( &interval.start ) == 0 &&
        thisInterval.end.Compare( &interval.end ) == 0 )
    {
      Interval<Alpha> newInterval( thisInterval.start, thisInterval.end,
                            thisInterval.lc && interval.lc,
                            thisInterval.rc && interval.rc );
      if( newInterval.IsValid() )
        result.Add( newInterval );
      if( ++i < GetNoComponents() )
        Get( i, thisInterval );
      if( ++j < r.GetNoComponents() )
        r.Get( j, interval );
    }
    else if( thisInterval.Inside( interval ) )
    {
      Interval<Alpha> newInterval( thisInterval );
      if( newInterval.IsValid() )
        result.Add( newInterval );
      if( ++i < GetNoComponents() )
        Get( i, thisInterval );
    }
    else if( interval.Inside( thisInterval ) )
    {
      Interval<Alpha> newInterval( interval );
      if( newInterval.IsValid() )
        result.Add( newInterval );
      if( ++j < r.GetNoComponents() )
        r.Get( j, interval );
    }
    else if( thisInterval.Intersects( interval ) )
    {
      if( thisInterval.start.Compare( &interval.end ) == 0 && ( thisInterval.lc && interval.rc ) )
      {
        Interval<Alpha> newInterval( interval.end, interval.end, true, true );
        result.Add( newInterval );
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( thisInterval.end.Compare( &interval.start ) == 0 && ( thisInterval.rc && interval.lc ) )
      {
        Interval<Alpha> newInterval( interval.start, interval.start, true, true );
        result.Add( newInterval );
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( thisInterval.start.Compare( &interval.start ) < 0 )
      {
        Interval<Alpha> newInterval( interval.start, thisInterval.end, interval.lc, thisInterval.rc );
        if( newInterval.IsValid() )
          result.Add( newInterval );
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( thisInterval.start.Compare( &interval.start ) == 0 )
      {
        assert( !thisInterval.lc || !interval.lc );
        if( thisInterval.end.Compare( &interval.end ) > 0 )
        {
          Interval<Alpha> newInterval( interval.start, interval.end, interval.lc && thisInterval.lc, interval.rc );
          if( newInterval.IsValid() )
            result.Add( newInterval );
          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else
        {
          assert( thisInterval.end.Compare( &interval.end ) < 0 );
          Interval<Alpha> newInterval( thisInterval.start, thisInterval.end, interval.lc && thisInterval.lc, thisInterval.rc );
          if( newInterval.IsValid() )
            result.Add( newInterval );
          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
      }
      else
      {
        Interval<Alpha> newInterval( thisInterval.start, interval.end, thisInterval.lc, interval.rc );
        if( newInterval.IsValid() )
        result.Add( newInterval );
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
    }
    else if( thisInterval.start.Compare( &interval.start ) <= 0 )
    {
      if( ++i < GetNoComponents() )
        Get( i, thisInterval );
    }
    else
    {
      if( ++j < r.GetNoComponents() )
        r.Get( j, interval );
    }
  }
  result.EndBulkLoad( false );
}

template <class Alpha>
void Range<Alpha>::Union( Range<Alpha>& r, Range<Alpha>& result )
{
  assert( IsValid() && r.IsValid() && result.IsEmpty() );

  Interval<Alpha> thisInterval, interval;

  result.StartBulkLoad();
  int i = 0, j = 0;

  if( !IsEmpty() )
    Get( i, thisInterval );
  if( !r.IsEmpty() )
    r.Get( j, interval );

  if( !IsEmpty() && !r.IsEmpty() )
  {
    Alpha *start = NULL, *end = NULL;
    bool lc = false, rc = false;

    while( i < GetNoComponents() && j < r.GetNoComponents() )
    {
      if( thisInterval.start.Compare( &interval.start ) == 0 &&
          thisInterval.end.Compare( &interval.end ) == 0 )
      {
        Interval<Alpha> newInterval( thisInterval.start, thisInterval.end,
                              thisInterval.lc || interval.lc,
                              thisInterval.rc || interval.rc );
        result.Add( newInterval );

        if( ++i < GetNoComponents() )
          Get( i, thisInterval );

        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( interval.Inside( thisInterval ) )
      {
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( thisInterval.Inside( interval ) )
      {
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( !thisInterval.Intersects( interval ) )
      {
        if( thisInterval.end.Compare( &interval.start ) < 0 )
        {
          if( thisInterval.adjacent( interval ) )
          {
            if( start != NULL && end != NULL )
            {
              delete end;
            }
            else
            {
              start = thisInterval.start.Clone();
              lc = thisInterval.lc;
            }
            end = interval.end.Clone();
            rc = interval.rc;
          }
          else
          {
            if( start != NULL && end != NULL )
            {
              Interval<Alpha> newInterval( *start, *end, lc, rc );
              result.Add( newInterval );
              delete start; start = NULL; 
              delete end; end = NULL;
              lc = false; rc = false;
            }
            else
            {
              Interval<Alpha> newInterval( thisInterval );
              result.Add( newInterval );
            }
          }

          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
        else if( thisInterval.start.Compare( &interval.end ) > 0 )
        {
          if( thisInterval.adjacent( interval ) )
          {
            if( start != NULL && end != NULL )
            {
              delete end;
            }
            else
            {
              start = interval.start.Clone();
              lc = interval.lc;
            }
            end = thisInterval.end.Clone();
            rc = thisInterval.rc;
          }
          else
          {
            if( start != NULL && end != NULL )
            {
              Interval<Alpha> newInterval( *start, *end, lc, rc );
              result.Add( newInterval );
              delete start; start = NULL; 
              delete end; end = NULL;
              lc = false; rc = false;
            }
            else
            {
              Interval<Alpha> newInterval( interval );
              result.Add( newInterval );
            }
          }

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else if( thisInterval.start.Compare( &interval.end ) == 0 )
        {
          if( !thisInterval.lc && !interval.rc )
          {
            if( start != NULL && end != NULL )
            {
              Interval<Alpha> newInterval( *start, *end, lc, rc );
              result.Add( newInterval );
              delete start; start = NULL; 
              delete end; end = NULL;
              lc = false; rc = false;
            }
            else
            {
              Interval<Alpha> newInterval( interval );
              result.Add( newInterval );
            }
          }
          else
          {
            if( start != NULL && end != NULL )
            {
              if( end->Compare( &thisInterval.end ) < 0 )
              {
                delete end;
                end = thisInterval.end.Clone();
                rc = thisInterval.rc;
              }
              else if( end->Compare( &thisInterval.end ) == 0 )
              {
                rc = rc || thisInterval.rc;
              }
            }
            else
            {
              start = interval.start.Clone();
              lc = interval.lc;
              end = thisInterval.end.Clone();
              rc = thisInterval.rc;
            }
          }

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else if( interval.start.Compare( &thisInterval.end ) == 0 )
        {
          if( !interval.lc && !thisInterval.rc )
          {
            if( start != NULL && end != NULL )
            {
              Interval<Alpha> newInterval( *start, *end, lc, rc );
              result.Add( newInterval );
              delete start; start = NULL; 
              delete end; end = NULL;
              lc = false; rc = false;
            }
            else
            {
              Interval<Alpha> newInterval( thisInterval );
              result.Add( newInterval );
            }
          }
          else
          {
            if( start != NULL && end != NULL )
            {
              if( end->Compare( &interval.end ) < 0 )
              {
                delete end; 
                end = interval.end.Clone();
                rc = interval.rc;
              }
              else if( end->Compare( &interval.end ) == 0 )
              {
                rc = rc || interval.rc;
              }
            }
            else
            {
              start = thisInterval.start.Clone();
              lc = thisInterval.lc;
              end = interval.end.Clone();
              rc = interval.rc;
            }
          }

          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
      }
      else if( thisInterval.start.Compare( &interval.start ) < 0 )
      {
        if( start == NULL && end == NULL )
        {
          start = thisInterval.start.Clone();
          lc = thisInterval.lc;
          end = interval.end.Clone();
          rc = interval.rc;
        }
        else
        {
          if( end->Compare( &interval.end ) < 0 )
          {
            end = interval.end.Clone();
            rc = interval.rc;
          }
          if( end->Compare( &interval.end ) == 0 )
          {
            rc = rc || interval.rc;
          }
        }

        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( interval.start.Compare( &thisInterval.start ) < 0 )
      {
        if( start == NULL && end == NULL )
        {
          start = interval.start.Clone();
          lc = interval.lc;
          end = thisInterval.end.Clone();
          rc = thisInterval.rc;
        }
        else
        {
          if( end->Compare( &thisInterval.end ) < 0 )
          {
            end = thisInterval.end.Clone();
            rc = thisInterval.rc;
          }
          if( end->Compare( &thisInterval.end ) == 0 )
          {
            rc = rc || thisInterval.rc;
          }
        }

        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( thisInterval.start.Compare( &interval.start ) == 0 )
      {
        assert( start == NULL && end == NULL );
        start = thisInterval.start.Clone();
        lc = thisInterval.lc || interval.lc;
        if( thisInterval.end.Compare( &interval.end ) < 0 )
        {
          end = interval.end.Clone();
          rc = interval.rc;

          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
        else
        {
          end = thisInterval.end.Clone();
          rc = thisInterval.rc;

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
      }
      else if( thisInterval.end.Compare( &interval.end ) == 0 )
      {
        assert( start != NULL && end != NULL );
        rc = thisInterval.rc || interval.rc;

        Interval<Alpha> newInterval( *start, *end, lc, rc );
        result.Add( newInterval );
        delete start; start = NULL; 
        delete end; end = NULL;
        lc = false; rc = false;

        if( ++i < GetNoComponents() )
          Get( i, thisInterval );

        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
    }

    if( start != NULL && end != NULL )
    {
      Interval<Alpha> newInterval( *start, *end, lc, rc );
      result.Add( newInterval );
      delete start; start = NULL;
      delete end; end = NULL;
      lc = rc = false;

      if( j >= r.GetNoComponents() )
      {
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( i >= GetNoComponents() )
      {
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
    }
  }

  while( i < GetNoComponents() )
  {
    Interval<Alpha> newInterval( thisInterval );
    result.Add( newInterval );

    if( ++i < GetNoComponents() )
      Get( i, thisInterval );
  }

  while( j < r.GetNoComponents() )
  {
    Interval<Alpha> newInterval( interval );
    result.Add( newInterval );

    if( ++j < r.GetNoComponents() )
      r.Get( j, interval );
  }
  result.EndBulkLoad( false );
}

template <class Alpha>
void Range<Alpha>::Minus( Range<Alpha>& r, Range<Alpha>& result )
{
  assert( IsValid() && r.IsValid() && result.IsEmpty() );

  if( IsEmpty() )
    return;
  result.StartBulkLoad();

  Interval<Alpha> thisInterval, interval;

  int i = 0, j = 0;
  Get( i, thisInterval );

  if( !r.IsEmpty() )
  {
    r.Get( j, interval );

    Alpha *start = NULL, *end = NULL;
    bool lc = false, rc = false;

    while( i < GetNoComponents() && j < r.GetNoComponents() )
    {
      if( thisInterval.start.Compare( &interval.start ) == 0 &&
          thisInterval.end.Compare( &interval.end ) == 0 )
      {
        if( thisInterval.lc && !interval.lc )
        {
          Interval<Alpha> newInterval( thisInterval.start, thisInterval.start, true, true );
          result.Add( newInterval );
        }
        if( thisInterval.rc && !interval.rc )
        {
          Interval<Alpha> newInterval( thisInterval.end, thisInterval.end, true, true );
          result.Add( newInterval );
        }

        if( ++i < GetNoComponents() )
          Get( i, thisInterval );

        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( !thisInterval.Intersects( interval ) )
      {
        if( start != NULL && end != NULL )
        {
          Interval<Alpha> newInterval( *start, *end, lc, rc );
          if( newInterval.IsValid() )
            result.Add( newInterval );
          delete start; start = NULL;
          delete end; end = NULL;
          lc = rc = false;
        }
        else if( thisInterval.start.Compare( &interval.start ) <= 0 )
        {
          Interval<Alpha> newInterval( thisInterval );
          result.Add( newInterval );
        }

        if( thisInterval.start.Compare( &interval.start ) <= 0 )
        {
          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
        else
        {
          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
      }
      else if( thisInterval.Inside( interval ) )
      {
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( interval.Inside( thisInterval ) )
      {
        if( interval.start.Compare( &thisInterval.start ) == 0 )
        {
          assert( start == NULL && end == NULL );
          if( thisInterval.lc && !interval.lc )
          {
            Interval<Alpha> newInterval( thisInterval.start, thisInterval.start, true, true );
            result.Add( newInterval );
          }
          start = interval.end.Clone();
          lc = !interval.rc;
          end = thisInterval.end.Clone();
          rc = thisInterval.rc;

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else if( interval.end.Compare( &thisInterval.end ) == 0 )
        {
          if( start == NULL && end == NULL )
          {
            Interval<Alpha> newInterval( thisInterval.start, interval.start, thisInterval.lc, !interval.lc );
            if( newInterval.IsValid() )
              result.Add( newInterval );
          }
          else
          {
            Interval<Alpha> newInterval( *start, interval.start, lc, !interval.lc );
            if( newInterval.IsValid() )
              result.Add( newInterval );
            delete start; start = NULL; 
            delete end; end = NULL;
            lc = false; rc = false;
          }

          if( thisInterval.rc && !interval.rc )
          {
            Interval<Alpha> newInterval( thisInterval.end, thisInterval.end, true, true );
            result.Add( newInterval );
          }

          if( ++i < GetNoComponents() )
            Get( i, thisInterval );

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else
        {
          assert( thisInterval.start.Compare( &interval.start ) < 0 &&
                  thisInterval.end.Compare( &interval.end ) > 0 );
          if( start == NULL && end == NULL )
          {
            Interval<Alpha> newInterval( thisInterval.start, interval.start, thisInterval.lc, !interval.lc );
            if( newInterval.IsValid() )
              result.Add( newInterval );
          }
          else
          {
            assert( end->Compare( &thisInterval.end ) == 0 && rc == thisInterval.rc );

            Interval<Alpha> newInterval( *start, interval.start, lc, !interval.lc );
            if( newInterval.IsValid() )
              result.Add( newInterval );
            delete start; start = NULL;
            delete end; end = NULL;
            lc = rc = false;
          }

          start = interval.end.Clone();
          lc = !interval.rc;
          end = thisInterval.end.Clone();
          rc = thisInterval.rc;

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
      }
      else
      {
        assert( thisInterval.Intersects( interval ) );

        if( interval.start.Compare( &thisInterval.start ) < 0 )
        {
          assert( start == NULL && end == NULL );

          if( interval.end.Compare( &thisInterval.end ) == 0 )
          {
            if( thisInterval.rc && !interval.rc )
            {
              Interval<Alpha> newInterval( thisInterval.end, thisInterval.end, true, true );
              result.Add( newInterval );
            }

            if( ++i < GetNoComponents() )
              Get( i, thisInterval );

            if( ++j < r.GetNoComponents() )
              r.Get( j, interval );
          }
          else
          {
            start = interval.end.Clone();
            if( interval.end.Compare( &thisInterval.start ) == 0 )
            {
              lc = thisInterval.lc && !interval.rc;
            }
            else
            {
              lc = !interval.rc;
            }
            end = thisInterval.end.Clone();
            rc = thisInterval.rc;

            if( ++j < r.GetNoComponents() )
              r.Get( j, interval );
          }
        }
        else if( interval.start.Compare( &thisInterval.start ) == 0 )
        {
          assert( start == NULL & end == NULL );

          if( thisInterval.lc && !interval.lc )
          {
            Interval<Alpha> newInterval( thisInterval.start, thisInterval.start, true, true );
            result.Add( newInterval );
          }

          if( thisInterval.end.Compare( &interval.end ) > 0 )
          {
            start = interval.end.Clone();
            lc = !interval.rc;
            end = thisInterval.end.Clone();
            rc = thisInterval.rc;

            if( ++j < r.GetNoComponents() )
              r.Get( j, interval );
          }
          else
          {
            assert( thisInterval.end.Compare( &interval.end ) < 0 );
            if( ++i < GetNoComponents() )
              Get( i, thisInterval );
          }
        }
        else if( interval.end.Compare( &thisInterval.end ) > 0 )
        {
          if( thisInterval.start.Compare( &interval.start ) == 0 )
          {
            assert( start == NULL && end == NULL );
            cerr << "I think that there is an error here!!!" << endl;
          }
          else
          {
            if( start != NULL && end != NULL )
            {
              if( interval.start.Compare( start ) > 0 ||
                  ( interval.start.Compare( start ) == 0 && interval.lc && !lc ) )
              {
                delete end;
                end = interval.start.Clone();
                if( interval.start.Compare( &thisInterval.end ) == 0 )
                  rc = thisInterval.rc && !interval.lc;
                else
                  rc = !interval.lc;

                Interval<Alpha> newInterval( *start, *end, lc, rc );
                if( newInterval.IsValid() )
                  result.Add( newInterval );
              }
              delete start; start = NULL;
              delete end; end = NULL;
              lc = rc = false;
            }
            else
            {
              Interval<Alpha> newInterval( thisInterval.start, interval.start, thisInterval.lc, !interval.lc );
              if( newInterval.IsValid() )
                result.Add( newInterval );
            }
            if( ++i < GetNoComponents() )
              Get( i, thisInterval );
          }
        }
        else
        {
          assert( interval.end.Compare( &thisInterval.end ) == 0 );

          if( interval.start.Compare( &thisInterval.start ) < 0 )
          {
            assert( start == NULL && end == NULL );
            if( thisInterval.rc && !interval.rc )
            {
              Interval<Alpha> newInterval( interval.end, interval.end, true, true );
              result.Add( newInterval );
            }
          }
          else
          {
            assert( interval.start.Compare( &thisInterval.start ) > 0 );

            if( start != NULL && end != NULL )
            {
              delete end;
              end = interval.start.Clone();
              rc = !interval.lc;

              Interval<Alpha> newInterval( *start, *end, lc, rc );
              if( newInterval.IsValid() )
                result.Add( newInterval );
              delete start; start = NULL;
              delete end; end = NULL;
              lc = rc = false;
            }
            else
            {
              Interval<Alpha> newInterval( thisInterval.start, interval.start, thisInterval.lc, !interval.lc );
              if( newInterval.IsValid() )
                result.Add( newInterval );
            }
          }

          if( ++i < GetNoComponents() )
            Get( i, thisInterval );

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
      }
    }

    if( start != NULL && end != NULL )
    {
      Interval<Alpha> newInterval( *start, *end, lc, rc );
      if( newInterval.IsValid() )
        result.Add( newInterval );

      delete start; 
      delete end;

      if( j >= r.GetNoComponents() )
      {
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( i >= GetNoComponents() )
      {
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
    }
  }

  while( i < GetNoComponents() )
  {
    Interval<Alpha> newInterval( thisInterval );
    result.Add( newInterval );

    if( ++i < GetNoComponents() )
      Get( i, thisInterval );
  }
  result.EndBulkLoad( false );
}

template <class Alpha>
void Range<Alpha>::Maximum( Alpha& result )
{
  assert( IsValid() );

  if( IsEmpty() )
    result.SetDefined( false );
  else
  {
    Interval<Alpha> interval;
    Get( GetNoComponents()-1, interval );
    result.CopyFrom( &interval.end );
  }
}

template <class Alpha>
void Range<Alpha>::Minimum( Alpha& result )
{
  assert( IsValid() );

  if( IsEmpty() )
    result.SetDefined( false );
  else
  {
    Interval<Alpha> interval;
    Get( 0, interval );
    result.CopyFrom( &interval.start );
  }
}

template <class Alpha>
int Range<Alpha>::GetNoComponents() const
{
  return intervals.Size();
}

template <class Alpha>
bool Range<Alpha>::IsValid()
{
  if( canDestroy )
    return false;

  if( !IsOrdered() )
    return false;

  if( IsEmpty() )
    return true;

  bool result = true;
  Interval<Alpha> lastInterval, interval;

  if( GetNoComponents() == 1 )
  {
    Get( 0, interval );
    return( interval.IsValid() );
  }

  for( int i = 1; i < GetNoComponents(); i++ )
  {
    Get( i-1, lastInterval );
    if( !lastInterval.IsValid() )
    {
      result = false;
      break;
    }
    Get( i, interval );
    if( !interval.IsValid() )
    {
      result = false;
      break;
    }
    if( (!lastInterval.Disjoint( interval )) && (!lastInterval.adjacent( interval )) )
    {
      result = false;
      break;
    }
  }

  return result;
}

/*
4.3 TemporalUnit

4.3.1 Constructors and destructor

*/
template <class Alpha>
bool TemporalUnit<Alpha>::IsValid()
{
    return timeInterval.IsValid();
}

template <class Alpha>
TemporalUnit<Alpha>& TemporalUnit<Alpha>::operator=( const TemporalUnit& i )
{
  assert( i.timeInterval.IsValid );

  timeInterval=i.timeInterval;
  
  return *this;
}

template <class Alpha>
bool TemporalUnit<Alpha>::operator==( TemporalUnit& i )
{
  assert( timeInterval.IsValid() && i.timeInterval.IsValid() );

  return( timeInterval==i.timeInterval);
}

template <class Alpha>
bool TemporalUnit<Alpha>::operator!=( TemporalUnit& i )
{
  return !( *this == i );
}

template <class Alpha>
bool TemporalUnit<Alpha>::R_Disjoint( TemporalUnit<Alpha>& i )
{
  return( timeInterval.R_Disjoint( i.timeInterval ) );  
}

template <class Alpha>
bool TemporalUnit<Alpha>::Disjoint( TemporalUnit<Alpha>& i )
{
  assert( IsValid() && i.IsValid() );

  return( R_Disjoint( i ) || i.R_Disjoint( *this ) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::R_Adjacent( TemporalUnit<Alpha>& i ) 
{
  return( timeInterval.R_Adjacent(i.timeInterval));
}

template <class Alpha>
bool TemporalUnit<Alpha>::adjacent( TemporalUnit<Alpha>& i )
{
  assert( IsValid() && i.IsValid() );

  return( R_Adjacent( i ) || i.R_Adjacent( *this ) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::Inside( TemporalUnit<Alpha>& i ) 
{
  assert( IsValid() && i.IsValid() );

  return( timeInterval.Inside(i.timeInterval) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::Contains( Instant& a ) 
{
  assert( IsValid() && a.IsDefined() );

  return ( timeInterval.Contains(a) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::Intersects( TemporalUnit<Alpha>& i ) 
{
  assert( IsValid() && i.IsValid() );

  return ( timeInterval.Intersects(i.timeInterval) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::Before( TemporalUnit<Alpha>& i ) 
{
  assert( IsValid() && i.IsValid() );

  return ( timeInterval.Before(i.timeInterval) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::Before( Instant& a ) 
{
  assert( IsValid() && a.IsDefined() );

  return ( timeInterval.Before( a ) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::After( Instant& a ) 
{
  assert( IsValid() && a.IsDefined() );

  return ( timeInterval.After( a ) );
}

/*
4.4 MappingPoint

4.4.1 Constructors and destructor

*/

// MPoint::MPoint() {}  have been defined inline

MPoint::MPoint( const int n ):
canDestroy( false ),
ordered( true ),
units( n )
{}

void MPoint::Destroy()
{
  canDestroy = true;
}

MPoint::~MPoint()
{
  if( canDestroy )
    units.Destroy();
}

/*
4.4.2 Member functions

*/
bool MPoint::IsOrdered() const
{
  return ordered;
}

void MPoint::StartBulkLoad()
{
  assert( ordered );
  ordered = false;
}


int mp_unitCompare( const void *a, const void *b )  
{
  UPoint *unita = new ((void*)a) UPoint,
               *unitb = new ((void*)b) UPoint;

  if( *unita == *unitb )
    return 0;
  else if( unita->Before( *unitb ) )
    return -1;
  else
    return 1;
}

void MPoint::EndBulkLoad( const bool sort )
{
  assert( !ordered );
  if( sort )
    units.Sort( mp_unitCompare );
  ordered = true;
  //assert( IsValid() );
}

bool MPoint::IsEmpty() const
{
  return units.Size() == 0;
}

void MPoint::Get( const int i, UPoint& unit )
{
  units.Get( i, unit );
  assert( unit.IsValid() );
}

void MPoint::Add( UPoint& unit )
{
    //assert( unit.IsValid() );
    if (unit.IsValid())
	units.Append( unit );
    else 
    {	
	    cout<<"Invalid time interval found, please check!"<<endl;
	    assert( false );
    }
}

void MPoint::Clear()
{
  ordered = true;
  units.Clear();
}

/*
4.4.3 Functions to be part of relations

*/
bool MPoint::IsDefined() const
{
  return true;
}

void MPoint::SetDefined( bool Defined )
{
}

int MPoint::Compare( Attribute * arg )
{
  return 0;
}

bool MPoint::Adjacent( Attribute * arg )
{
  return false;
}

MPoint* MPoint::Clone()
{
  assert( IsOrdered() );

  MPoint *result = new MPoint( GetNoComponents() );

  result->StartBulkLoad();
  UPoint unit;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    result->Add( unit );
  }
  result->EndBulkLoad( false );
  return result;
}

ostream& MPoint::Print( ostream &os )
{
  return os << "Temporal Algebra---MPoint" << endl;
}

size_t MPoint::HashValue()
{
  return 0;
}

void MPoint::CopyFrom( StandardAttribute* right )
{
  MPoint *r = (MPoint*)right;
  assert( r->IsOrdered() );

  Clear();

  StartBulkLoad();
  UPoint unit;
  for( int i = 0; i < r->GetNoComponents(); i++ )
  {
    r->Get( i, unit );
    Add( unit );
  }
  EndBulkLoad( false );
}

int MPoint::NumOfFLOBs()
{
  return 1;
}

FLOB *MPoint::GetFLOB(const int i)
{
  assert( i == 0 ); 
  return &units;
}

/*
4.4.4 Operator functions

*/

bool MPoint::operator==( MPoint& r )
{
  assert( IsValid() && r.IsValid() );

  if( GetNoComponents() != r.GetNoComponents() )
    return false;

  bool result = true;
  UPoint thisunit, unit;

  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, thisunit );
    r.Get( i, unit );

    if( thisunit != unit )
    {
      result = false;
      break;
    }
  }

  return result;
}

bool MPoint::operator!=( MPoint& r )
{
  return !( *this == r );
}

int MPoint::GetNoComponents() const
{
  return units.Size();
}

bool MPoint::IsValid()
{
  if( canDestroy )
    return false;

  if( !IsOrdered() )
    return false;

  if( IsEmpty() )
    return true;

  bool result = true;
  UPoint lastunit, unit;

  if( GetNoComponents() == 1 )
  {
    Get( 0, unit );
    return( unit.IsValid() );
  }

  for( int i = 1; i < GetNoComponents(); i++ )
  {
    Get( i-1, lastunit );
    if( !lastunit.IsValid() )
    {
      result = false;
      break;
    }
    Get( i, unit );
    if( !unit.IsValid() )
    {
      result = false;
      break;
    }
    if( (!lastunit.Disjoint( unit )) && (!lastunit.adjacent( unit )) )
    {
      result = false;
      break;
    }
  }

  return result;
}

int MPoint::Position( const Instant& t ) 
{
    assert( IsOrdered() && t.IsDefined() );
  
    int first = 0, last = units.Size();
    Instant t1=t;
  
    while (first <= last)
    {
	int mid = ( first + last ) / 2;
	if ((mid<0) || (mid>=units.Size())) return -1;
	    
	UPoint midUPoint;
	units.Get( mid, midUPoint );
	if( t1.GetRealval() > midUPoint.timeInterval.end.GetRealval() )
	    first = mid + 1;
	else if( t1.GetRealval() < midUPoint.timeInterval.start.GetRealval() )
	    last = mid - 1;
	else  // (midUPoint.begin <= t <= midUPoint.end)
	{
		if (midUPoint.timeInterval.Contains(t1)) 
		    return mid;
		else return -1;
	}
    }
    return -1;
}

bool MPoint::TemporalFunction( Instant& t, Point& result )
{
    assert( t.IsDefined() );

    int pos=Position(t);
      
    if( pos==-1)  //not contained in any unit
	return false;
    UPoint posUPoint;
    units.Get( pos, posUPoint );
      
    Point resPoint;
    if (posUPoint.TemporalFunction(t, resPoint))
    {
	result.Set( resPoint.GetX(), resPoint.GetY() );
	return true;
    }
    else return false;
}

/*
4.5 MappingInt

4.5.1 Constructors and destructor

*/
// MInt::MInt() {}  have been defined inline

MInt::MInt( const int n ):
canDestroy( false ),
ordered( true ),
units( n )
{}

void MInt::Destroy()
{
  canDestroy = true;
}

MInt::~MInt()
{
  if( canDestroy )
    units.Destroy();
}

/*
4.5.2 Member functions

*/
bool MInt::IsOrdered() const
{
  return ordered;
}

void MInt::StartBulkLoad()
{
  assert( ordered );
  ordered = false;
}


int mi_unitCompare( const void *a, const void *b )
{
  ConstTemporalUnit<CcInt> *unita = new ((void*)a) ConstTemporalUnit<CcInt>,
	  *unitb = new ((void*)b) ConstTemporalUnit<CcInt>;

  if( *unita == *unitb )
    return 0;
  else if( unita->Before( *unitb ) )
    return -1;
  else
    return 1;
}

void MInt::EndBulkLoad( const bool sort )
{
  assert( !ordered );
  if( sort )
    units.Sort( mi_unitCompare );
  ordered = true;
  //assert( IsValid() );
}

bool MInt::IsEmpty() const
{
  return units.Size() == 0;
}

void MInt::Get( const int i, ConstTemporalUnit<CcInt>& unit )
{
  units.Get( i, unit );
  assert( unit.IsValid() );
}

void MInt::Add( ConstTemporalUnit<CcInt>& unit )
{
  assert( unit.IsValid() );
  units.Append( unit );
}

void MInt::Clear()
{
  ordered = true;
  units.Clear();
}

/*
4.5.3 Functions to be part of relations

*/
bool MInt::IsDefined() const
{
  return true;
}

void MInt::SetDefined( bool Defined )
{
}

int MInt::Compare( Attribute * arg )
{
  return 0;
}

bool MInt::Adjacent( Attribute * arg )
{
  return false;
}

MInt* MInt::Clone()
{
  assert( IsOrdered() );

  MInt *result = new MInt( GetNoComponents() );

  result->StartBulkLoad();
  ConstTemporalUnit<CcInt> unit;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    result->Add( unit );
  }
  result->EndBulkLoad( false );
  return result;
}

ostream& MInt::Print( ostream &os )
{
  return os << "Temporal Algebra---MInt" << endl;
}

size_t MInt::HashValue()
{
  return 0;
}

void MInt::CopyFrom( StandardAttribute* right )
{
  MInt *r = (MInt*)right;
  assert( r->IsOrdered() );

  Clear();

  StartBulkLoad();
  ConstTemporalUnit<CcInt> unit;
  for( int i = 0; i < r->GetNoComponents(); i++ )
  {
    r->Get( i, unit );
    Add( unit );
  }
  EndBulkLoad( false );
}

int MInt::NumOfFLOBs()
{
  return 1;
}

FLOB *MInt::GetFLOB(const int i)
{
  assert( i == 0 ); 
  return &units;
}

int MInt::Position( const Instant& t ) 
{
  assert( IsOrdered() && t.IsDefined() );
  
  int first = 0, last = units.Size();
  Instant t1=t;
  
  while (first <= last)
  {
    int mid = ( first + last ) / 2;
    if ((mid<0)||(mid>=units.Size())) return -1;
    
    ConstTemporalUnit<CcInt> midUInt;
    units.Get( mid, midUInt );
    if( t1.GetRealval() > midUInt.timeInterval.end.GetRealval() )
      first = mid + 1;
    else if( t1.GetRealval() < midUInt.timeInterval.start.GetRealval() )
      last = mid - 1;
    else  // (midUPoint.begin <= t <= midUPoint.end)
    {
	  if (midUInt.timeInterval.Contains(t1)) 
	      return mid;
	  else return -1;
     }
   }
   return -1;
}

bool MInt::TemporalFunction( Instant& t, CcInt& result )
{
    assert( t.IsDefined() );

    int pos=Position(t);
      
    if( pos==-1)  //not contained in any unit
	return false;
    ConstTemporalUnit<CcInt> posUInt;
    units.Get( pos, posUInt );
      
    CcInt resInt;
    if (posUInt.TemporalFunction(t, resInt))
    {
	result.Set( resInt.GetIntval() );
	return true;
    }
    else return false;
}


/*
4.5.4 Operator functions

*/

bool MInt::operator==( MInt& r )
{
  assert( IsValid() && r.IsValid() );

  if( GetNoComponents() != r.GetNoComponents() )
    return false;

  bool result = true;
  ConstTemporalUnit<CcInt> thisunit, unit;

  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, thisunit );
    r.Get( i, unit );

    if( thisunit != unit )
    {
      result = false;
      break;
    }
  }

  return result;
}

bool MInt::operator!=( MInt& r )
{
  return !( *this == r );
}

int MInt::GetNoComponents() const
{
  return units.Size();
}

bool MInt::IsValid()
{
  if( canDestroy )
    return false;

  if( !IsOrdered() )
    return false;

  if( IsEmpty() )
    return true;

  bool result = true;
  ConstTemporalUnit<CcInt> lastunit, unit;

  if( GetNoComponents() == 1 )
  {
    Get( 0, unit );
    return( unit.IsValid() );
  }

  for( int i = 1; i < GetNoComponents(); i++ )
  {
    Get( i-1, lastunit );
    if( !lastunit.IsValid() )
    {
      result = false;
      break;
    }
    Get( i, unit );
    if( !unit.IsValid() )
    {
      result = false;
      break;
    }
    
    //if (lastunit.Disjoint( unit )) {cout<<"Disjoint!"<<endl;} else cout<<"not Disjoint!"<<endl;
    //if (lastunit.Adjacent( unit ) )  {cout<<"Adjacent!"<<endl;} else cout<<"not Adjacent!"<<endl;
    
    if( (!lastunit.Disjoint( unit )) && (!lastunit.adjacent( unit )) )
    {
      result = false;
      break;
    }
  }

  return result;
}

/*
4.6 MappingReal

4.6.1 Constructors and destructor

*/
// MReal::MReal() {}  have been defined inline

MReal::MReal( const int n ):
canDestroy( false ),
ordered( true ),
units( n )
{}

void MReal::Destroy()
{
  canDestroy = true;
}

MReal::~MReal()
{
  if( canDestroy )
    units.Destroy();
}

/*
4.6.2 Member functions

*/
bool MReal::IsOrdered() const
{
  return ordered;
}

void MReal::StartBulkLoad()
{
  assert( ordered );
  ordered = false;
}


int mr_unitCompare( const void *a, const void *b ) 
{
  UReal *unita = new ((void*)a) UReal,
               *unitb = new ((void*)b) UReal;

  if( *unita == *unitb )
    return 0;
  else if( unita->Before( *unitb ) )
    return -1;
  else
    return 1;
}

void MReal::EndBulkLoad( const bool sort )
{
  assert( !ordered );
  if( sort )
    units.Sort( mr_unitCompare );
  ordered = true;
  //assert( IsValid() );
}

bool MReal::IsEmpty() const
{
  return units.Size() == 0;
}

void MReal::Get( const int i, UReal& unit )
{
  units.Get( i, unit );
  assert( unit.IsValid() );
}

void MReal::Add( UReal& unit )
{
  assert( unit.IsValid() );
  units.Append( unit );
}

void MReal::Clear()
{
  ordered = true;
  units.Clear();
}

/*
4.6.3 Functions to be part of relations

*/
bool MReal::IsDefined() const
{
  return true;
}

void MReal::SetDefined( bool Defined )
{
}

int MReal::Compare( Attribute * arg )
{
  return 0;
}

bool MReal::Adjacent( Attribute * arg )
{
  return false;
}

MReal* MReal::Clone()
{
  assert( IsOrdered() );

  MReal *result = new MReal( GetNoComponents() );

  result->StartBulkLoad();
  UReal unit;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    result->Add( unit );
  }
  result->EndBulkLoad( false );
  return result;
}

ostream& MReal::Print( ostream &os )
{
  return os << "Temporal Algebra---MReal" << endl;
}

size_t MReal::HashValue()
{
  return 0;
}

void MReal::CopyFrom( StandardAttribute* right )
{
  MReal *r = (MReal*)right;
  assert( r->IsOrdered() );

  Clear();

  StartBulkLoad();
  UReal unit;
  for( int i = 0; i < r->GetNoComponents(); i++ )
  {
    r->Get( i, unit );
    Add( unit );
  }
  EndBulkLoad( false );
}

int MReal::NumOfFLOBs()
{
  return 1;
}

FLOB *MReal::GetFLOB(const int i)
{
  assert( i == 0 ); 
  return &units;
}

/*
4.6.4 Operator functions

*/

bool MReal::operator==( MReal& r )
{
  assert( IsValid() && r.IsValid() );

  if( GetNoComponents() != r.GetNoComponents() )
    return false;

  bool result = true;
  UReal thisunit, unit;

  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, thisunit );
    r.Get( i, unit );

    if( thisunit != unit )
    {
      result = false;
      break;
    }
  }

  return result;
}

bool MReal::operator!=( MReal& r )
{
  return !( *this == r );
}

int MReal::GetNoComponents() const
{
  return units.Size();
}

bool MReal::IsValid()
{
  if( canDestroy )
    return false;

  if( !IsOrdered() )
    return false;

  if( IsEmpty() )
    return true;

  bool result = true;
  UReal lastunit, unit;

  if( GetNoComponents() == 1 )
  {
    Get( 0, unit );
    return( unit.IsValid() );
  }

  for( int i = 1; i < GetNoComponents(); i++ )
  {
    Get( i-1, lastunit );
    if( !lastunit.IsValid() )
    {
      result = false;
      break;
    }
    Get( i, unit );
    if( !unit.IsValid() )
    {
      result = false;
      break;
    }
    if( (!lastunit.Disjoint( unit )) && (!lastunit.adjacent( unit )) )
    {
      result = false;
      break;
    }
  }

  return result;
}

int MReal::Position( const Instant& t ) 
{
  assert( IsOrdered() && t.IsDefined() );
  
  int first = 0, last = units.Size();
  Instant t1=t;
  
  while (first <= last)
  {
    int mid = ( first + last ) / 2;
    if ((mid<0)||(mid>=units.Size())) return -1;
    
    UReal midUReal;
    units.Get( mid, midUReal );
    if( t1.GetRealval() > midUReal.timeInterval.end.GetRealval() )
      first = mid + 1;
    else if( t1.GetRealval() < midUReal.timeInterval.start.GetRealval() )
      last = mid - 1;
    else  // midUPoint.begin <= t <= midUPoint.end
    {
	  if (midUReal.timeInterval.Contains(t1)) 
	      return mid;
	  else return -1;
     }
   }
   return -1;
}

bool MReal::TemporalFunction( Instant& t, CcReal& result )
{
    assert( t.IsDefined() );

    int pos=Position(t);
      
    if( pos==-1)  //not contained in any unit
	return false;
    UReal posUReal;
    units.Get( pos, posUReal );
      
    CcReal resReal;
    if (posUReal.TemporalFunction(t, resReal))
    {
	result.Set( resReal.GetRealval() );
	return true;
    }
    else return false;
}

/*
5 Type Constructor template functions

5.1 Type Constructor ~range~

5.1.1 ~Out~-function

*/
template <class Alpha, ListExpr (*OutFun)( ListExpr, Word )>
ListExpr OutRange( ListExpr typeInfo, Word value )
{
  Range<Alpha>* range = (Range<Alpha>*)(value.addr);

  if( range->IsEmpty() )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    assert( range->IsOrdered() );
    ListExpr l = nl->TheEmptyList(), lastElem, intervalList;

    for( int i = 0; i < range->GetNoComponents(); i++ )
    {
      Interval<Alpha> interval;
      range->Get( i, interval );
      intervalList = nl->FourElemList( 
	      OutFun( nl->TheEmptyList(), SetWord(&interval.start) ),
	      OutFun( nl->TheEmptyList(), SetWord(&interval.end) ),
	      nl->BoolAtom( interval.lc ), 
	      nl->BoolAtom( interval.rc));
      if (l == nl->TheEmptyList())
      {
        l = nl->Cons( intervalList, nl->TheEmptyList());
        lastElem = l;
      }
      else
        lastElem = nl->Append(lastElem, intervalList);
    }
    return l;
  }
}

/*
5.1.2 ~In~-function

*/
template <class Alpha, Word (*InFun)( const ListExpr, const ListExpr, const int, ListExpr&, bool& )>
Word InRange( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Range<Alpha>* range = new Range<Alpha>( 0 );
  range->StartBulkLoad();

  ListExpr rest = instance;
  while( !nl->IsEmpty( rest ) )
  {
    ListExpr first = nl->First( rest );
    rest = nl->Rest( rest );

    if( nl->ListLength( first ) == 4 &&
        nl->IsAtom( nl->First( first ) ) &&
        nl->IsAtom( nl->Second( first ) ) &&
        nl->IsAtom( nl->Third( first ) ) &&
        nl->AtomType( nl->Third( first ) ) == BoolType &&
        nl->IsAtom( nl->Fourth( first ) ) &&
        nl->AtomType( nl->Fourth( first ) ) == BoolType )
    {
      Alpha *start = (Alpha *)InFun( nl->TheEmptyList(), nl->First( first ), errorPos, errorInfo, correct ).addr;
      if( correct == false )
      {
        return SetWord( Address(0) );
      }

      Alpha *end = (Alpha *)InFun( nl->TheEmptyList(), nl->Second( first ), errorPos, errorInfo, correct ).addr;
      if( correct == false )
      {
        delete start;
        return SetWord( Address(0) );
      }

      Interval<Alpha> interval( *start, *end,
                                nl->BoolValue( nl->Third( first ) ),
                                nl->BoolValue( nl->Fourth( first ) ) );

      delete start;
      delete end;

      range->Add( interval );
    }
    else
    {
      correct = false;
      return SetWord( Address(0) );
    }
  }
  range->EndBulkLoad( true );
  correct = true;

  return SetWord( range );
}

/*
5.1.3 ~Open~-function

*/
template <class Alpha>
bool OpenRange( SmiRecord& valueRecord,
                const ListExpr typeInfo,
                Word& value )
{
  Range<Alpha> *range = new Range<Alpha>( 0 );

  range->Open( valueRecord, typeInfo );

  value = SetWord( range );
  return true;
}

/*
5.1.4 ~Save~-function

*/
template <class Alpha>
bool SaveRange( SmiRecord& valueRecord,
                const ListExpr typeInfo,
                Word& value )
{
  Range<Alpha> *range = (Range<Alpha> *)value.addr;

  range->Save( valueRecord, typeInfo );

  return true;
}

/*
5.1.5 ~Create~-function

*/
template <class Alpha>
Word CreateRange( const ListExpr typeInfo )
{
  return (SetWord( new Range<Alpha>( 0 ) ));
}

/*
5.1.6 ~Delete~-function

*/
template <class Alpha>
void DeleteRange( Word& w )
{
  ((Range<Alpha> *)w.addr)->Destroy();
  delete (Range<Alpha> *)w.addr;
  w.addr = 0;
}

/*
5.1.7 ~Close~-function

*/
template <class Alpha>
void CloseRange( Word& w )
{
  delete (Range<Alpha> *)w.addr;
  w.addr = 0;
}

/*
5.1.8 ~Clone~-function

*/
template <class Alpha>
Word CloneRange( const Word& w )
{
  Range<Alpha> *r = (Range<Alpha> *)w.addr;
  return SetWord( r->Clone() );
}

/*
5.1.9 ~Sizeof~-function

*/
template <class Alpha>
int SizeOfRange()
{
  return sizeof(Range<Alpha>);
}

/*
5.1.10 ~Cast~-function

*/
template <class Alpha>
void* CastRange(void* addr)
{
  return new (addr) Range<Alpha>;
}

/*
5.2 Type Constructor ~intime~

5.2.1 ~Out~-function

*/
template <class Alpha, ListExpr (*OutFun)( ListExpr, Word )>
ListExpr OutIntime( ListExpr typeInfo, Word value )
{
  Intime<Alpha>* intime = (Intime<Alpha>*)(value.addr);

  return nl->TwoElemList( 
	  //nl->RealAtom( intime->instant.GetRealval() ),
	  OutInstant( nl->TheEmptyList(), SetWord(&intime->instant) ),
	  OutFun( nl->TheEmptyList(), SetWord( &intime->value ) ) );
}

/*
5.2.2 ~In~-function

*/
template <class Alpha, Word (*InFun)( const ListExpr, const ListExpr, const int, ListExpr&, bool& )>
Word InIntime( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
    if( nl->ListLength( instance ) == 2 )
    {
	//1.deal with the instant value
	//Instant instant( true, nl->RealValue( nl->First( instance ) ) );
	Instant *instant = (Instant *)InInstant(
		    nl->TheEmptyList(), 
		    nl->First( instance ), 
		    errorPos, errorInfo, correct ).addr;
 
	if ( correct == false )
	{
	    return SetWord( Address(0) );
	}
	
	
	//2.deal with the alpha value
	Alpha *value = (Alpha *)InFun( nl->TheEmptyList(), nl->Second( instance ), errorPos, errorInfo, correct ).addr;
	if( correct  )
	{
	    Intime<Alpha> *intime = new Intime<Alpha>( *instant, *value );
	    delete value;
	    return SetWord( intime );
	}
    }
    correct = false;
    return SetWord( Address(0) );
}

/*
5.2.3 ~Create~-function

*/
template <class Alpha>
Word CreateIntime( const ListExpr typeInfo )
{
  return (SetWord( new Intime<Alpha>() ));
}

/*
5.2.4 ~Delete~-function

*/
template <class Alpha>
void DeleteIntime( Word& w )
{
  delete (Intime<Alpha> *)w.addr;
  w.addr = 0;
}

/*
5.2.5 ~Close~-function

*/
template <class Alpha>
void CloseIntime( Word& w )
{
  delete (Intime<Alpha> *)w.addr;
  w.addr = 0;
}

/*
5.2.6 ~Clone~-function

*/
template <class Alpha>
Word CloneIntime( const Word& w )
{
  Intime<Alpha> *intime = (Intime<Alpha> *)w.addr;
  return SetWord( new Intime<Alpha>( *intime ) );
}

/*
5.2.7 ~Sizeof~-function

*/
template <class Alpha>
int SizeOfIntime()
{
  return sizeof(Intime<Alpha>);
}

/*
5.2.8 ~Cast~-function

*/
template <class Alpha>
void* CastIntime(void* addr)
{
  return new (addr) Intime<Alpha>;
}

/*
5.3 Type Constructor ~constunit~

5.3.1 ~Out~-function

*/
template <class Alpha, ListExpr (*OutFun)( ListExpr, Word )>
ListExpr OutConstTemporalUnit( ListExpr typeInfo, Word value )
{ 
    //1.get the address of the object and have a class object
    ConstTemporalUnit<Alpha>* 
	    constunit = (ConstTemporalUnit<Alpha>*)(value.addr);

    //2.get the time interval NL
    ListExpr intervalList = nl->FourElemList(
	    OutInstant( nl->TheEmptyList(), SetWord(&constunit->timeInterval.start) ),
	    OutInstant( nl->TheEmptyList(), SetWord(&constunit->timeInterval.end) ),
	    nl->BoolAtom( constunit->timeInterval.lc ), 
	    nl->BoolAtom( constunit->timeInterval.rc));
    
    //3. return the final result
    return nl->TwoElemList(
	    intervalList, 
	    OutFun( nl->TheEmptyList(), SetWord( &constunit->constValue ) ) );
}

/*
5.3.2 ~In~-function

*/
template <class Alpha, Word (*InFun)( const ListExpr, const ListExpr, const int, ListExpr&, bool& )>
Word InConstTemporalUnit( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
    if( nl->ListLength( instance ) == 2 &&
        nl->IsAtom( nl->Second( instance ) ) )
    {
	//1. deal with the time interval
	ListExpr first = nl->First( instance );
	
	if( nl->ListLength( first ) == 4 &&
	    //nl->IsAtom( nl->First( first ) ) &&
	    //nl->IsAtom( nl->Second( first ) ) &&
	    nl->IsAtom( nl->Third( first ) ) &&
	    nl->AtomType( nl->Third( first ) ) == BoolType &&
	    nl->IsAtom( nl->Fourth( first ) ) &&
	    nl->AtomType( nl->Fourth( first ) ) == BoolType )
	{
	    Instant *start = (Instant *)InInstant( 
		    nl->TheEmptyList(), 
		    nl->First( first ), 
		    errorPos, errorInfo, correct ).addr;
	    if( correct == false )
	    {
		return SetWord( Address(0) );
	    }

	    Instant *end = (Instant *)InInstant(
		    nl->TheEmptyList(), 
		    nl->Second( first ), 
		    errorPos, errorInfo, correct ).addr;
	    if( correct == false )
	    {
		delete start;
		return SetWord( Address(0) );
	    }

	    Interval<Instant> tinterval( *start, *end,
				      nl->BoolValue( nl->Third( first ) ),
				      nl->BoolValue( nl->Fourth( first ) ) );

	    delete start;
	    delete end;
	    
	    //2. deal with the alpha value
	    Alpha *value = (Alpha *)InFun( 
		    nl->TheEmptyList(), 
		    nl->Second( instance ), 
		    errorPos, errorInfo, correct ).addr;
	
	    //3. create the class object
	    if( correct  )
	    {
		ConstTemporalUnit<Alpha> *constunit = 
			new ConstTemporalUnit<Alpha>( tinterval, *value );
		delete value;
		return SetWord( constunit );
	    }
	}
	else
	{
	    correct = false;
	    return SetWord( Address(0) );
	}
    }
  correct = false;
  return SetWord( Address(0) );
}

/*
5.3.3 ~Create~-function

*/
template <class Alpha>
Word CreateConstTemporalUnit( const ListExpr typeInfo )
{
  return (SetWord( new ConstTemporalUnit<Alpha>() ));
}

/*
5.3.4 ~Delete~-function

*/
template <class Alpha>
void DeleteConstTemporalUnit( Word& w )
{
  delete (ConstTemporalUnit<Alpha> *)w.addr;
  w.addr = 0;
}

/*
5.3.5 ~Close~-function

*/
template <class Alpha>
void CloseConstTemporalUnit( Word& w )
{
  delete (ConstTemporalUnit<Alpha> *)w.addr;
  w.addr = 0;
}

/*
5.3.6 ~Clone~-function

*/
template <class Alpha>
Word CloneConstTemporalUnit( const Word& w )
{
  ConstTemporalUnit<Alpha> *constunit = (ConstTemporalUnit<Alpha> *)w.addr;
  return SetWord( new ConstTemporalUnit<Alpha>( *constunit ) );
}

/*
5.3.7 ~Sizeof~-function

*/
template <class Alpha>
int SizeOfConstTemporalUnit()
{
  return sizeof(ConstTemporalUnit<Alpha>);
}

/*
5.3.8 ~Cast~-function

*/
template <class Alpha>
void* CastConstTemporalUnit(void* addr)
{
  return new (addr) ConstTemporalUnit<Alpha>;
}

/*
5.4 Type Constructor ~ureal~ 

The Nested List form of ureal is:  (timeinterval (a b c r)),  where a, b, c are real numbers, and r is a boolean flag

for instance: ( ( 6.37  9.9   TRUE FALSE)   (1.0 2.3 4.1 TRUE) )

5.4.1 ~Out~-function

*/
ListExpr OutUreal( ListExpr typeInfo, Word value )
{
  //1.get the address of the object and have a class object
  UReal* ureal = (UReal*)(value.addr);
  
  //2.output the time interval -> NL
  ListExpr timeintervalList = nl->FourElemList(
	  OutInstant( nl->TheEmptyList(), SetWord(&ureal->timeInterval.start) ),
	  //nl->RealAtom( ureal->timeInterval.start.GetRealval()),
	  OutInstant( nl->TheEmptyList(), SetWord(&ureal->timeInterval.end) ),
	  //nl->RealAtom( ureal->timeInterval.end.GetRealval()),
	  nl->BoolAtom( ureal->timeInterval.lc ), 
	  nl->BoolAtom( ureal->timeInterval.rc));
  
  //3. get the real function NL (a b c r)
    ListExpr realfunList = nl->FourElemList( 
	    nl->RealAtom( ureal->a),
	    nl->RealAtom( ureal->b),
	    nl->RealAtom( ureal->c ),
	    nl->BoolAtom( ureal->r));
  
  //4. return the final result
  return nl->TwoElemList(timeintervalList, realfunList );
}

/*
5.4.2 ~In~-function

the Nested list form is like this:  ( ( 6.37 9.9 TRUE FALSE)   (1.0 2.3 4.1 TRUE) )

*/
Word InUreal( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
    if ( nl->ListLength( instance ) == 2 )
    {
	//1. deal with the time interval  ( 6.37  9.9  T F) or ((instant 1.0) (instant 2.3) T F)
	ListExpr first = nl->First( instance );
	if( nl->ListLength( first ) == 4 &&
	    //nl->IsAtom( nl->First( first ) ) &&
	    //nl->IsAtom( nl->Second( first ) ) &&
	    nl->IsAtom( nl->Third( first ) ) &&
	    nl->AtomType( nl->Third( first ) ) == BoolType &&
	    nl->IsAtom( nl->Fourth( first ) ) &&
	    nl->AtomType( nl->Fourth( first ) ) == BoolType )
	{
	    Instant *start = (Instant *)InInstant( 
		    nl->TheEmptyList(), 
		    nl->First( first ), 
		    errorPos, errorInfo, correct ).addr; 
	    //Instant *start = new Instant ( true, nl->RealValue( nl->First( first ) ) );
	    if( correct == false )
	    {
		return SetWord( Address(0) );
	    }

	    Instant *end = (Instant *)InInstant( 
		    nl->TheEmptyList(),
		    nl->Second( first ),
		    errorPos, errorInfo, correct ).addr;
    	    //Instant *end = new Instant ( true, nl->RealValue( nl->Second( first ) ) );
	    if( correct == false )
	    {
		delete start;
		return SetWord( Address(0) );
	    }

	    Interval<Instant> tinterval( *start, *end,
				      nl->BoolValue( nl->Third( first ) ),
				      nl->BoolValue( nl->Fourth( first ) ) );

	    delete start;
	    delete end;
	    
	    //2. deal with the unit-function: (1.0 2.3 4.1 TRUE)
	    ListExpr second = nl->Second( instance );
	    if( nl->ListLength( second ) == 4 &&
	        nl->IsAtom( nl->First( second ) ) &&
	        nl->AtomType( nl->First( second ) ) == RealType &&
	        nl->IsAtom( nl->Second( second ) ) &&
	        nl->AtomType( nl->Second( second ) ) == RealType &&
	        nl->IsAtom( nl->Third( second ) ) &&
  	        nl->AtomType( nl->Third( second ) ) == RealType &&
	        nl->IsAtom( nl->Fourth( second ) ) &&
	        nl->AtomType( nl->Fourth( second ) ) == BoolType )
	    {
		//3. create the class object
		correct = true;
		UReal *ureal = new UReal( tinterval, 
					  nl->RealValue( nl->First( second ) ),
					  nl->RealValue( nl->Second( second ) ),
					  nl->RealValue( nl->Third( second ) ),
					  nl->BoolValue( nl->Fourth( second ) ) );
		return SetWord( ureal );
	    }
	    else
	    {
		correct = false;
		return SetWord( Address(0) );
	    }
	    
	}
	else
	{
	    correct = false;
	    return SetWord( Address(0) );
	}
    }
    correct = false;
    return SetWord( Address(0) );
}

/*
5.4.3 ~Create~-function

*/
Word CreateUreal( const ListExpr typeInfo )
{
  return (SetWord( new UReal() ));
}

/*
5.4.4 ~Delete~-function

*/
void DeleteUreal( Word& w )
{
  delete (UReal *)w.addr;
  w.addr = 0;
}

/*
5.4.5 ~Close~-function

*/
void CloseUreal( Word& w )
{
  delete (UReal *)w.addr;
  w.addr = 0;
}

/*
5.4.6 ~Clone~-function

*/
Word CloneUreal( const Word& w )
{
  UReal *ureal = (UReal *)w.addr;
  return SetWord( new UReal( *ureal ) );
}

/*
5.4.7 ~Sizeof~-function

*/
int SizeOfUreal()
{
  return sizeof(UReal);
}

/*
5.4.8 ~Cast~-function

*/
void* CastUreal(void* addr)
{
  return new (addr) UReal;
}


/*
5.5 Type Constructor ~upoint~ 

The Nested List form of upoint is:  (timeinterval (x0 x1 y0 y1)),  where x0, x1, y0, y1 are real numbers.
for instance: ( ( 6.37 9.9 TRUE FALSE)   (1.0 2.3 4.1 2.1) )

5.5.1 ~Out~-function

*/
ListExpr OutUPoint( ListExpr typeInfo, Word value )
{
  //1.get the address of the object and have a class object
  UPoint* upoint = (UPoint*)(value.addr);
  
  //2.output the time interval -> NL
  ListExpr timeintervalList = nl->FourElemList( 
	  OutInstant( nl->TheEmptyList(), SetWord(&upoint->timeInterval.start) ),
	  //nl->RealAtom( upoint->timeInterval.start.GetRealval()),
	  OutInstant( nl->TheEmptyList(), SetWord(&upoint->timeInterval.end) ),
	  //nl->RealAtom( upoint->timeInterval.end.GetRealval()),
	  nl->BoolAtom( upoint->timeInterval.lc ), 
	  nl->BoolAtom( upoint->timeInterval.rc));
  
  //3. get the real function NL (x0 x1 y0 y1)
  ListExpr pointfunList = nl->FourElemList(  
	  nl->RealAtom( upoint->x0),
	  nl->RealAtom( upoint->x1),
	  nl->RealAtom( upoint->y0),
	  nl->RealAtom( upoint->y1));
  
  //4. return the final result
  return nl->TwoElemList(timeintervalList, pointfunList );
}

/*
5.5.2 ~In~-function

the Nested list form is like this:  ( ( 6.37  9.9  TRUE FALSE)   (1.0 2.3 4.1 2.1) )

*/
Word InUPoint( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
    if ( nl->ListLength( instance ) == 2 )
    {
	//1. deal with the time interval  ( 6.37  9.9  T F) or ((instant 2.0) (instant 3.0) T F)
	ListExpr first = nl->First( instance );
	
	if( nl->ListLength( first ) == 4 &&
	    //nl->IsAtom( nl->First( first ) ) &&
	    //nl->IsAtom( nl->Second( first ) ) &&
	    nl->IsAtom( nl->Third( first ) ) &&
	    nl->AtomType( nl->Third( first ) ) == BoolType &&
	    nl->IsAtom( nl->Fourth( first ) ) &&
	    nl->AtomType( nl->Fourth( first ) ) == BoolType )
	{
	    correct = true;
	    Instant *start = (Instant *)InInstant( 
		    nl->TheEmptyList(), 
		    nl->First( first ), 
		    errorPos, errorInfo, correct ).addr;
	    //Instant *start = new Instant ( true, nl->RealValue( nl->First( first ) ) );
	    
	    if( correct == false )
	    {
		return SetWord( Address(0) );
	    }

	    Instant *end = (Instant *)InInstant( 
		    nl->TheEmptyList(), 
		    nl->Second( first ), 
		    errorPos, errorInfo, correct ).addr;
    	    //Instant *end = new Instant ( true, nl->RealValue( nl->Second( first ) ) );
	    	    
	    if( correct == false )
	    {
		delete start;
		return SetWord( Address(0) );
	    }

	    Interval<Instant> tinterval( *start, *end,
			 nl->BoolValue( nl->Third( first ) ),
			 nl->BoolValue( nl->Fourth( first ) ) );

	    delete start;
	    delete end;
	    
	    //2. deal with the unit-function: (1.0 2.3 4.1 2.1)
	    ListExpr second = nl->Second( instance );
	    if( nl->ListLength( second ) == 4 &&
	        nl->IsAtom( nl->First( second ) ) &&
	        nl->AtomType( nl->First( second ) ) == RealType &&
	        nl->IsAtom( nl->Second( second ) ) &&
	        nl->AtomType( nl->Second( second ) ) == RealType &&
	        nl->IsAtom( nl->Third( second ) ) &&
  	        nl->AtomType( nl->Third( second ) ) == RealType &&
	        nl->IsAtom( nl->Fourth( second ) ) &&
	        nl->AtomType( nl->Fourth( second ) ) == RealType )
	    {
		//3. create the class object
		correct = true;
		UPoint *upoint = new UPoint( tinterval, 
					  nl->RealValue( nl->First( second ) ),
					  nl->RealValue( nl->Second( second ) ),
					  nl->RealValue( nl->Third( second ) ),
					  nl->RealValue( nl->Fourth( second ) ) );
		return SetWord( upoint );
	    }
	    else
	    {
		correct = false;
		return SetWord( Address(0) );
	    }
	}
	else
	{
	    correct = false;
	    return SetWord( Address(0) );
	}
    }
    correct = false;
    return SetWord( Address(0) );
}

/*
5.5.3 ~Create~-function

*/
Word CreateUPoint( const ListExpr typeInfo )
{
  return (SetWord( new UPoint() ));
}

/*
5.5.4 ~Delete~-function

*/
void DeleteUPoint( Word& w )
{
  delete (UPoint *)w.addr;
  w.addr = 0;
}

/*
5.5.5 ~Close~-function

*/
void CloseUPoint( Word& w )
{
  delete (UPoint *)w.addr;
  w.addr = 0;
}

/*
5.5.6 ~Clone~-function

*/
Word CloneUPoint( const Word& w )
{
  UPoint *upoint = (UPoint *)w.addr;
  return SetWord( new UPoint( *upoint ) );
}

/*
5.5.7 ~Sizeof~-function

*/
int SizeOfUPoint()
{
  return sizeof(UPoint);
}

/*
5.5.8 ~Cast~-function

*/
void* CastUPoint(void* addr)
{
  return new (addr) UPoint;
}

/*
5.6 Type Constructor ~mpoint~

5.6.1 ~Out~-function

*/
ListExpr OutMPoint( ListExpr typeInfo, Word value )
{  
    MPoint* mpoint = (MPoint*)(value.addr);

    if( mpoint->IsEmpty() )
    {
	return (nl->TheEmptyList());
    }
    else
    {
	assert( mpoint->IsOrdered() );
	ListExpr l = nl->TheEmptyList(), lastElem, upointList;

	for( int i = 0; i < mpoint->GetNoComponents(); i++ )
	{
	    UPoint unit;
	    mpoint->Get( i, unit );
	    upointList = OutUPoint( nl->TheEmptyList(), SetWord(&unit) );
	    if (l == nl->TheEmptyList())
	    {
		l = nl->Cons( upointList, nl->TheEmptyList());
		lastElem = l;
	    }
	    else
		lastElem = nl->Append(lastElem, upointList);
	}
	return l;
    }
}

/*
5.6.2 ~In~-function

*/

Word InMPoint( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
    MPoint* mpoint = new MPoint( 0 );
    mpoint->StartBulkLoad();

    ListExpr rest = instance;
    while( !nl->IsEmpty( rest ) )
    {
	ListExpr first = nl->First( rest );
	rest = nl->Rest( rest );

	UPoint *upoint = (UPoint*)InUPoint( 
		nl->TheEmptyList(), 
		first, 
		errorPos, errorInfo, correct ).addr;
	if( correct == false ) return SetWord( Address(0) );
	mpoint->Add( *upoint );
    }
    mpoint->EndBulkLoad( true );
    if (mpoint->IsValid()) 
    {
	correct = true;
	return SetWord( mpoint );
    }
    else
    {
	correct = false;
	return SetWord( 0 );
    }
}

/*
5.6.3 ~Open~-function

*/
bool OpenMPoint( SmiRecord& valueRecord, const ListExpr typeInfo, Word& value )
{
  MPoint *mpoint = new MPoint( 0 );

  mpoint->Open( valueRecord, typeInfo );

  value = SetWord( mpoint );
  return true;
}

/*
5.6.4 ~Save~-function

*/
bool SaveMPoint( SmiRecord& valueRecord, const ListExpr typeInfo, Word& value )
{
  MPoint *mpoint = (MPoint *)value.addr;

  mpoint->Save( valueRecord, typeInfo );

  return true;
}

/*
5.6.5 ~Create~-function

*/
Word CreateMPoint( const ListExpr typeInfo )
{
  return (SetWord( new MPoint( 0 ) ));
}

/*
5.6.6 ~Delete~-function

*/
void DeleteMPoint( Word& w )
{
  ((MPoint *)w.addr)->Destroy();
  delete (MPoint *)w.addr;
  w.addr = 0;
}

/*
5.6.7 ~Close~-function

*/
void CloseMPoint( Word& w )
{
  delete (MPoint *)w.addr;
  w.addr = 0;
}

/*
5.6.8 ~Clone~-function

*/
Word CloneMPoint( const Word& w )
{
  MPoint *r = (MPoint *)w.addr;
  return SetWord( r->Clone() );
}

/*
5.6.9 ~Sizeof~-function

*/
int SizeOfMPoint()
{
  return sizeof(MPoint);
}

/*
5.6.10 ~Cast~-function

*/
void* CastMPoint(void* addr)
{
  return new (addr) MPoint;
}

/*
5.7 Type Constructor ~mint~

5.7.1 ~Out~-function

*/
ListExpr OutMInt( ListExpr typeInfo, Word value )
{ 
    MInt* mint = (MInt*)(value.addr);

    if( mint->IsEmpty() )
    {
	return (nl->TheEmptyList());
    }
    else
    {
	assert( mint->IsOrdered() );
	ListExpr l = nl->TheEmptyList(), lastElem, uintList;

	for( int i = 0; i < mint->GetNoComponents(); i++ )
	{
	    ConstTemporalUnit<CcInt> unit;
	    mint->Get( i, unit );
	    uintList = OutConstTemporalUnit<CcInt, OutCcInt>( 
		    nl->TheEmptyList(), 
		    SetWord(&unit) );
      
	    if (l == nl->TheEmptyList())
	    {
		l = nl->Cons( uintList, nl->TheEmptyList());
		lastElem = l;
	    }
	    else
		lastElem = nl->Append(lastElem, uintList);
	}
	return l;
    }
}

/*
5.7.2 ~In~-function

*/

Word InMInt( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
    MInt* mint = new MInt( 0 );
    mint->StartBulkLoad();

    ListExpr rest = instance;
    while( !nl->IsEmpty( rest ) )
    {
	ListExpr first = nl->First( rest );
	rest = nl->Rest( rest );

	ConstTemporalUnit<CcInt> *constuint = 
		(ConstTemporalUnit<CcInt>*)
		InConstTemporalUnit<CcInt, InCcInt>( 
			nl->TheEmptyList(), 
			first, 
			errorPos, errorInfo, correct ).addr;
	if( correct == false ) return SetWord( Address(0) );

	mint->Add( *constuint );
    }
    mint->EndBulkLoad( true );
    if (mint->IsValid()) 
    {
	correct = true;
	return SetWord( mint );
    }
    else
    {
	correct = false;
	return SetWord( 0 );
    }
}

/*
5.7.3 ~Open~-function

*/
bool OpenMInt( SmiRecord& valueRecord, const ListExpr typeInfo, Word& value )
{
  MInt *mint = new MInt( 0 );

  mint->Open( valueRecord, typeInfo );

  value = SetWord( mint );
  return true;
}

/*
5.7.4 ~Save~-function

*/
bool SaveMInt( SmiRecord& valueRecord, const ListExpr typeInfo, Word& value )
{
  MInt *mint = (MInt *)value.addr;

  mint->Save( valueRecord, typeInfo );

  return true;
}

/*
5.7.5 ~Create~-function

*/
Word CreateMInt( const ListExpr typeInfo )
{
  return (SetWord( new MInt( 0 ) ));
}

/*
5.7.6 ~Delete~-function

*/
void DeleteMInt( Word& w )
{
  ((MInt *)w.addr)->Destroy();
  delete (MInt *)w.addr;
  w.addr = 0;
}

/*
5.7.7 ~Close~-function

*/
void CloseMInt( Word& w )
{
  delete (MInt *)w.addr;
  w.addr = 0;
}

/*
5.7.8 ~Clone~-function

*/
Word CloneMInt( const Word& w )
{
  MInt *r = (MInt *)w.addr;
  return SetWord( r->Clone() );
}

/*
5.7.9 ~Sizeof~-function

*/
int SizeOfMInt()
{
  return sizeof(MInt);
}

/*
5.7.10 ~Cast~-function

*/
void* CastMInt(void* addr)
{
  return new (addr) MInt;
}

/*
5.8 Type Constructor ~mreal~

5.8.1 ~Out~-function

*/
ListExpr OutMReal( ListExpr typeInfo, Word value )
{ 
    MReal* mreal = (MReal*)(value.addr);

    if( mreal->IsEmpty() )
    {
	return (nl->TheEmptyList());
    }
    else
    {
	assert( mreal->IsOrdered() );
	ListExpr l = nl->TheEmptyList(), lastElem, urealList;

	for( int i = 0; i < mreal->GetNoComponents(); i++ )
	{
	    UReal unit;
	    mreal->Get( i, unit );
	    urealList = OutUreal( nl->TheEmptyList(), SetWord(&unit) );
	    if (l == nl->TheEmptyList())
	    {
		l = nl->Cons( urealList, nl->TheEmptyList());
		lastElem = l;
	    }
	    else
		lastElem = nl->Append(lastElem, urealList);
	}
	return l;
    }
}

/*
5.8.2 ~In~-function

*/

Word InMReal( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
    MReal* mreal = new MReal( 0 );
    mreal->StartBulkLoad();

    ListExpr rest = instance;
    while( !nl->IsEmpty( rest ) )
    {
	ListExpr first = nl->First( rest );
	rest = nl->Rest( rest );

	UReal *ureal = (UReal*)InUreal( 
		nl->TheEmptyList(), 
		first, 
		errorPos, errorInfo, correct ).addr;
	if( correct == false ) return SetWord( Address(0) );

	mreal->Add( *ureal );
    }
    
    mreal->EndBulkLoad( true );
    if (mreal->IsValid())
    {
	correct = true;
	return SetWord( mreal );
    }
    else
    {
	correct = false;
	return SetWord( 0 );
     }
}

/*
5.8.3 ~Open~-function

*/
bool OpenMReal( SmiRecord& valueRecord, const ListExpr typeInfo, Word& value )
{
  MReal *mreal = new MReal( 0 );

  mreal->Open( valueRecord, typeInfo );

  value = SetWord( mreal );
  return true;
}

/*
5.8.4 ~Save~-function

*/
bool SaveMReal( SmiRecord& valueRecord, const ListExpr typeInfo, Word& value )
{
  MReal *mreal = (MReal *)value.addr;

  mreal->Save( valueRecord, typeInfo );

  return true;
}

/*
5.8.5 ~Create~-function

*/
Word CreateMReal( const ListExpr typeInfo )
{
  return (SetWord( new MReal( 0 ) ));
}

/*
5.8.6 ~Delete~-function

*/
void DeleteMReal( Word& w )
{
  ((MReal *)w.addr)->Destroy();
  delete (MReal *)w.addr;
  w.addr = 0;
}

/*
5.8.7 ~Close~-function

*/
void CloseMReal( Word& w )
{
  delete (MReal *)w.addr;
  w.addr = 0;
}

/*
5.8.8 ~Clone~-function

*/
Word CloneMReal( const Word& w )
{
  MReal *r = (MReal *)w.addr;
  return SetWord( r->Clone() );
}

/*
5.8.9 ~Sizeof~-function

*/
int SizeOfMReal()
{
  return sizeof(MReal);
}

/*
5.8.10 ~Cast~-function

*/
void* CastMReal(void* addr)
{
  return new (addr) MReal;
}

#endif // _TEMPORAL_ALGEBRA_H_
