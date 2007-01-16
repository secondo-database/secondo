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

January 2004 Victor Almeida

March - April 2004 Zhiming Ding

12.03.2005 Juergen Schmidt:
Adding ~Mapping<Unit, Alpha>::MergeAdd~ for merging two adjacent
units with the same values. This is for simplify the result of
operations. For this adding ~TemporalUnit::EqualValue~ to decide
the equality of values.
16.03.2006 Juergen Schmidt:
Added ~ConstTemporalUnit:EqualValue~

31.05.2006 Christian D[ue]ntgen:
Adding ~Range<Alpha>::RBBox~ for constructing the range consisting
of only one minimal interval, that contains all intervals from a range type

Sept 2006 Christian D[ue]ntgen implemented ~defined~ flag for unit types
[TOC]

Dec 2006 Christian D[ue]ntgen: Moved RefimentPartition from MovingRegionAlgbera
to here and replaced the error-prone implementation in TemporalLiftedAlgbra by
this version.

1 Overview

The type system of the Temporal Algebra can be seen below.

\begin{displaymath}
\begin{array}{lll}
        & \to \textrm{BASE}     & {\underline{\smash{\mathit{int}}}}, {\underline{\smash{\mathit{real}}}},
                                  {\underline{\smash{\mathit{bool}}}}, {\underline{\smash{\mathit{string}}}} \\
        & \to \textrm{SPATIAL}  & {\underline{\smash{\mathit{point}}}}, {\underline{\smash{\mathit{points}}}},
                                  {\underline{\smash{\mathit{line}}}}, {\underline{\smash{\mathit{region}}}} \\
        & \to \textrm{TIME}     & {\underline{\smash{\mathit{instant}}}} \\
\textrm{BASE} \cup \textrm{TIME}        & \to \textrm{RANGE}    & {\underline{\smash{\mathit{range}}}} \\
\textrm{BASE} \cup \textrm{SPATIAL}     & \to \textrm{TEMPORAL} & {\underline{\smash{\mathit{intime}}}},
                                                                  {\underline{\smash{\mathit{moving}}}}
\end{array}
\end{displaymath}

2 Defines, includes, and constants

*/
#ifndef _TEMPORAL_ALGEBRA_H_
#define _TEMPORAL_ALGEBRA_H_

#include <iostream>
#include <sstream>
#include <string>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardAttribute.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "NestedList.h"
#include "DBArray.h"
#include "RectangleAlgebra.h"
#include "DateTime.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace datetime;

const bool REFINEMENT_DEBUG = false;
//const bool REFINEMENT_DEBUG = true;

string int2string(const int& number);

/*
3 C++ Classes (Defintion)

3.1 Instant

This class represents a time instant, or a point in time. It will be
used in the ~instant~ type constructor.

*/
typedef DateTime Instant;

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

  Interval( const Interval<Alpha>& interval );
/*
The copy constructor.

*/

  Interval( const Alpha& start,
            const Alpha& end,
            const bool lc,
            const bool rc );
/*
The creation of the interval setting all attributes.

3.2.2 Member functions

*/

  void CopyFrom( const Interval<Alpha>& interval );

  bool IsValid() const;
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

  bool operator==( const Interval<Alpha>& i ) const;
/*
Returns ~true~ if this interval is equal to the interval ~i~ and ~false~ if they are different.

*/

  bool operator!=( const Interval<Alpha>& i ) const;
/*
Returns ~true~ if this interval is different to the interval ~i~ and ~false~ if they are equal.

*/

  bool R_Disjoint( const Interval<Alpha>& i ) const;
/*
Returns ~true~ if this interval is r-disjoint with the interval ~i~ and ~false~ otherwise.

*/

  bool Disjoint( const Interval<Alpha>& i ) const;
/*
Returns ~true~ if this interval is disjoint with the interval ~i~ and ~false~ otherwise.

*/

  bool R_Adjacent( const Interval<Alpha>& i ) const;
/*
Returns ~true~ if this interval is r-adjacent with the interval ~i~ and ~false~ otherwise.

*/

  bool Adjacent( const Interval<Alpha>& i ) const;
/*
Returns ~true~ if this interval is adjacent with the interval ~i~ and ~false~ otherwise.

*/

  bool Inside( const Interval<Alpha>& i ) const;
/*
Returns ~true~ if this interval is inside the interval ~i~ and ~false~ otherwise.

*/

  bool Contains( const Alpha& a ) const;
/*
Returns ~true~ if this interval contains the value ~a~ and ~false~ otherwise.

*Precondition:* ~a.IsDefined()~

*/

  bool Intersects( const Interval<Alpha>& i ) const;
/*
Returns ~true~ if this interval intersects with the interval ~i~ and ~false~ otherwise.

*/

  bool Before( const Interval<Alpha>& i ) const;
/*
Returns ~true~ if this interval is before the interval ~i~ and ~false~ otherwise.

*/

  bool Before( const Alpha& a ) const;
  bool After( const Alpha& a ) const;
/*
Returns ~true~ if this interval is before/after the value ~a~ and ~false~ otherwise.

*/

  void Intersection( const Interval<Alpha>& i, Interval<Alpha>& result ) const;
/*
Return the intersection of this interval and ~i~ into ~result~.

*/

  int CompareTo(const Interval<Alpha>& i) const;
/*
Compares this and the argument;

*/

/*

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

    void Get( const int i, const Interval<Alpha>*& ai ) const;
/*
Returns the interval ~ai~ at the position ~i~ in the range.

*/

    void Add( const Interval<Alpha>& i );
/*
Adds an interval ~i~ to the range. We will assume that the only way of adding intervals
is in bulk loads, i.e., in a non-ordered array.

*Precondition:* ~IsOrdered() == false~

*/

    void Merge( Range<Alpha>& result ) const;
/*
Merges a range into ~result~ concatenating adjacent intervals.

*/

    void Clear();
/*
Remove all intervals in the range.

3.3.4 Functions to be part of relations

*/
    inline bool IsDefined() const;
    inline void SetDefined( bool Defined );
    inline size_t Sizeof() const;
    inline int Compare( const Attribute* arg ) const;
    inline bool Adjacent( const Attribute* arg ) const;
    inline Range<Alpha>* Clone() const;
    inline ostream& Print( ostream &os ) const;
    inline size_t HashValue() const;
    inline void CopyFrom( const StandardAttribute* right );

    inline int NumOfFLOBs() const;
    inline FLOB *GetFLOB(const int i);

/*
3.3.5 Operations

3.3.5.1 Operation $=$ (~equal~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X = Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool operator==( const Range<Alpha>& r ) const;

/*
3.3.5.2 Operation $\neq$ (~not equal~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \neq Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool operator!=( const Range<Alpha>& r ) const;

/*
3.3.5.3 Operation ~intersects~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \cap Y \neq \emptyset$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool Intersects( const Range<Alpha>& r ) const;

/*
3.3.5.4 Operation ~inside~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \subseteq Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool Inside( const Range<Alpha>& r ) const;

/*
3.3.5.5 Operation ~contains~

*Precondition:* ~X.IsOrdered() $\&\&$ y.IsDefined()~

*Semantics:* $y \in X$

*Complexity:* $O(log(n))$, where ~n~ is the size of this range ~X~.

*/
    bool Contains( const Alpha& a ) const;

/*
3.3.5.6 Operation ~before~ (with ~range~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $\forall x \in X, \forall y \in Y: x \leq y$

*Complexity:* $O(1)$.

*/
    bool Before( const Range<Alpha>& r ) const;

/*
3.3.5.7 Operation ~before~ (with ~BASE~ type)

*Precondition:* ~X.IsOrdered() $\&\&$ y.IsDefined()~

*Semantics:* $\forall x \in X: x \leq y$

*Complexity:* $O(1)$.

*/
    bool Before( const Alpha& a ) const;

/*
3.3.5.8 Operation ~after~

This operation works only with ~BASE~ type, because it is an attempt to implement the operation
before on a contrary order, i.e., ~x before Y~.

*Precondition:* ~Y.IsOrdered() $\&\&$ x.IsDefined()~

*Semantics:* $\forall y \in Y: x \leq y$

*Complexity:* $O(1)$.

*/
    bool After( const Alpha& a ) const;

/*
3.3.5.9 Operation ~intersection~

*Precondition:* ~X.IsOrdered() $\&\&$ X.IsOrdered() $\&\&$ Result.IsEmpty()~

*Semantics:* $X \cap Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    void Intersection( const Range<Alpha>& r, Range<Alpha>& result ) const;


/*
3.3.5.10 Operation ~union~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered() $\&\&$ Result.IsEmpty()~

*Semantics:* $X \cup Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    void Union( const Range<Alpha>& r, Range<Alpha>& result ) const;

/*
3.3.5.11 Operation ~minus~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered() $\&\&$ Result.IsEmpty()~

*Semantics:* $X \backslash Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    void Minus( const Range<Alpha>& r, Range<Alpha>& result ) const;

/*
3.3.5.12 Operation ~max~

*Precondition:* ~X.IsOrdered()~

*Semantics:* $max(\rho(X))$

*Complexity:* $O(1)$

*/
    void Maximum( Alpha& result ) const;

/*
3.3.5.13 Operation ~min~

*Precondition:* ~X.IsOrdered()~

*Semantics:* $min(\rho(X))$

*Complexity:* $O(1)$

*/
    void Minimum( Alpha& result ) const;

/*
3.3.5.14 Operation ~no\_components~

*Precondition:* ~X.IsOrdered()~

*Semantics:* $\| intvls(X) \|$

*Complexity:* $O(1)$

*/
    int GetNoComponents() const;

/*
3.3.5.15 Operation ~bbox~ for ~range~-types

*Precondition:* ~X.IsOrdered() $\&\&$ Result.IsEmpty()~

*Semantics:* The range with the smallest interval, that contains all intervals within $X$.

*Complexity:* $O(1)$

*/
    void RBBox( Range<Alpha>& result ) const;


    bool IsValid() const;
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
  Intime() {}
/*
The simple constructor.

*/

  Intime( const Instant& instant, const Alpha& alpha ):
    instant( instant ),
    value(),
    defined( true )
  {
    value.CopyFrom( &alpha );
  }
/*
The first constructor.

*/

  Intime( const Intime<Alpha>& intime ):
    instant( intime.instant ),
    value(),
    defined( intime.defined )
  {
    if( defined )
      value.CopyFrom( &intime.value );
  }
/*
The second constructor.

3.4.2 Functions to be part of relations

*/
  bool IsDefined() const
  {
    return defined;
  }

  void SetDefined( bool defined )
  {
    this->defined = defined;
  }

  size_t Sizeof() const
  {
    return sizeof( *this );
  }

  int Compare( const Attribute* arg ) const
  {
    return 0;
  }

  bool Adjacent( const Attribute* arg ) const
  {
    return false;
  }

  Intime<Alpha>* Clone() const
  {
    return (new Intime<Alpha>( *this));
  }

  ostream& Print( ostream &os ) const
  {
    return os << "Temporal Algebra---Intime" << endl;
  }

  size_t HashValue() const
  {
    return 0;
  }

  void CopyFrom( const StandardAttribute* right )
  {
    const Intime<Alpha>* i = (const Intime<Alpha>*)right;

    defined = i->defined;
    if( defined )
    {
      instant.Equalize(&(i->instant));
      value.CopyFrom( &i->value );
    }
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
  bool defined;
/*
The flag that indicates if the value is defined or not.

*/

};

/*
3.5 TemporalUnit

This class will generically implements a temporal unit. It is an abstract class
that enforces each kind of temporal unit to have a function that computes a value
inside the temporal unit given a time instant (also inside the temporal unit).

*/
template <class Alpha>
struct TemporalUnit
{

  public:

/*
3.5.1 Constructors and Destructor

*/
  TemporalUnit() {}
/*
The simple constructor. This constructor should not be used.

*/

  TemporalUnit( bool is_defined):defined(is_defined) {}

/*
Use this constructor when declaring temporal object variables etc.

*/

  TemporalUnit( const Interval<Instant>& interval ):
    timeInterval( interval ),
    defined( true )
    {}
/*
This constructor sets the time interval of the temporal unit.

*/

  virtual ~TemporalUnit() {}
/*
The destructor.

3.5.2 Member Functions

*/

  bool IsValid() const;
/*
Checks if the Temporal Unit is valid or not. This function should be used for debugging purposes
only.  A TemoralUnit is valid if its timeInterval is valid.

*/

  virtual TemporalUnit<Alpha>& operator=( const TemporalUnit<Alpha>& i );
/*
Redefinition of the copy operator ~=~.

*/

  virtual bool operator==( const TemporalUnit<Alpha>& i ) const;
/*
Returns ~true~ if this temporal unit is equal to the temporal unit ~i~ and ~false~ if they are different.

*/

  virtual bool operator!=( const TemporalUnit<Alpha>& i ) const;
/*
Returns ~true~ if this temporal unit is different to the temporal unit ~i~ and ~false~ if they are equal.

*/

  bool R_Disjoint( const TemporalUnit<Alpha>& i ) const;
/*
Returns ~true~ if this temporal unit is r-disjoint with the temporal unit ~i~ and ~false~ otherwise.

*/

  bool Disjoint( const TemporalUnit<Alpha>& i ) const;
/*
Returns ~true~ if this temporal unit is disjoint with the temporal unit ~i~ and ~false~ otherwise.

*/

  bool R_Adjacent( const TemporalUnit<Alpha>& i ) const;
/*
Returns ~true~ if this temporal unit is r-adjacent with the temporal unit ~i~ and ~false~ otherwise.

*/

  bool TU_Adjacent( const TemporalUnit<Alpha>& i ) const;
    // VTA - It should be Adjacent but it does not compile.
/*
Returns ~true~ if this temporal unit is adjacent with the temporal unit ~i~ and ~false~ otherwise.

*/

  bool Inside( const TemporalUnit<Alpha>& i ) const;
/*
Returns ~true~ if this temporal unit is inside the temporal unit ~i~ and ~false~ otherwise.

*/

  bool Contains( const Instant& a ) const;
/*
Returns ~true~ if this temporal unit contains the value ~a~ and ~false~ otherwise.

*Precondition:* ~a.IsDefined()~

*/

  bool Intersects( const TemporalUnit<Alpha>& i ) const;
/*
Returns ~true~ if this temporal unit intersects with the temporal unit ~i~ and ~false~ otherwise.

*/

  bool Before( const TemporalUnit<Alpha>& i ) const;
/*
Returns ~true~ if this temporal unit is before the temporal unit ~i~ and ~false~ otherwise.

*/

  bool Before( const Instant& a ) const;
  bool After( const Instant& a ) const;
/*
Returns ~true~ if this temporal unit is before/after the value ~a~ and ~false~ otherwise.

*/

  virtual void TemporalFunction( const Instant& t,
                                 Alpha& result,
                                 bool ignoreLimits = false ) const = 0;
/*
The temporal function that receives a time instant ~t~ and returns the value
associated with time ~t~ in the output argument ~result~.
If ~ignoreLimits = true~, the limits given by the ~timeinterval~ will be ignored.
You can use this feature e.g. to calculate values for the ~initial~ and ~final~
instants of left/right open intervals.

Otherwise (~ignoreLimits = false~, resp. unspecified) there will be following

*Precondition:* t must be inside the temporal unit time interval.

*/

  virtual bool Passes( const Alpha& val ) const = 0;
/*
Checks if inside the unit the function passes by the value ~val~.

*/

  virtual bool At( const Alpha& val, TemporalUnit<Alpha>& result ) const = 0;
/*
Returns a unit restricted to the parts where the temporal function is equal
to ~val~.

*/

  virtual void AtInterval( const Interval<Instant>& i,
                           TemporalUnit<Alpha>& result ) const
  {
    timeInterval.Intersection( i, result.timeInterval );
    result.defined = defined;
  }

/*
Returns a unit restricted to the time interval ~i~.
This can be used to create sub-units for refinement partitions.

*/

  virtual bool EqualValue( TemporalUnit<Alpha>& i )
  {
    return false;
  }

/*
Returns ~true~ if the value of this temporal unit is equal to the
value of the temporal unit ~i~ and ~false~ if they are different.
Equality is computed with respect to temporal evolution.

*/

/*
3.5.3 Attributes

*/

  Interval<Instant> timeInterval;
/*
The time interval of the temporal unit.

*/

  protected:

  bool defined;

/*
A Flag indicating whether the unit is defined or not.

*/

};

/*
3.6 StandardTemporalUnit

This class inherits from ~StandardAttribute~ and allows temporal units
of standard types to be part of relations. One should note that it is
still an abstract class, because the functions ~CopyFrom~ and ~Clone~
are not implemented.

*/
template<class Alpha>
class StandardTemporalUnit :
  public StandardAttribute,
  public TemporalUnit<Alpha>
{
  public:

    StandardTemporalUnit() {}
/*
The simple constructor. This constructor should not be used.

*/

    StandardTemporalUnit( bool is_defined):TemporalUnit<Alpha>(is_defined) {}

/*
Use this constructor when declaring temporal object variables etc.

*/

    StandardTemporalUnit( const Interval<Instant>& interval ):
      TemporalUnit<Alpha>( interval )
      {}
/*
This constructor sets the time interval of the temporal unit.

*/

    virtual ~StandardTemporalUnit() {}
/*
The destructor.

3.5.2 Member Functions

3.6.4.1 Functions to be part of relations

*/
    bool IsDefined() const
    {
      return TemporalUnit<Alpha>::defined;
    }

    void SetDefined( bool Defined )
    {
      TemporalUnit<Alpha>::defined = Defined;
    }

    virtual int Compare( const Attribute* arg ) const
    {
      return 0;
    }

    virtual bool Adjacent( const Attribute* arg ) const
    {
      return false;
    }

    virtual ostream& Print( ostream &os ) const
    {
      if( IsDefined() )
        {
          os << "TemporalUnit: " << "( ";
          os << " NO SPECIFIC Print()-Method!";
          os << " )" << endl;
          return os;
        }
      else
        return os << "TemporalUnit: (undef)" << endl;
    }

    virtual size_t HashValue() const
    {
      return 0;
    }

    virtual StandardTemporalUnit<Alpha>* Clone() const = 0;
    virtual void CopyFrom( const StandardAttribute* right ) = 0;
    virtual size_t Sizeof() const = 0;

};

/*
3.7 SpatialTemporalUnit

This class inherits from ~SpatialStandardAttribute~ and allows temporal units
of spatial types to be part of relations. This class is a template also on the
dimensionality. One should note that it is still an abstract class, because
the functions ~CopyFrom~ and ~Clone~
are not implemented.

*/
template <class Alpha, unsigned dim>
class SpatialTemporalUnit :
  public StandardSpatialAttribute<dim>,
  public TemporalUnit<Alpha>
{
  public:
  SpatialTemporalUnit() {}
/*
The simple constructor. This constructor should not be used.

*/

    SpatialTemporalUnit( bool is_defined):TemporalUnit<Alpha>(is_defined) {}

/*
Use this constructor when declaring temporal object variables etc.

*/

    SpatialTemporalUnit( const Interval<Instant>& interval ):
      TemporalUnit<Alpha>( interval )
      {}
/*
This constructor sets the time interval of the temporal unit.

*/

    virtual ~SpatialTemporalUnit() {}
/*
The destructor.

3.5.2 Member Functions

3.6.4.1 Functions to be part of relations

*/
    bool IsDefined() const
    {
      return TemporalUnit<Alpha>::defined;
    }

    void SetDefined( bool Defined )
    {
      TemporalUnit<Alpha>::defined = Defined;
    }

    virtual int Compare( const Attribute* arg ) const
    {
      return 0;
    }

    virtual bool Adjacent( const Attribute* arg ) const
    {
      return false;
    }

    virtual ostream& Print( ostream &os ) const
    {
      if( IsDefined() )
        {
          os << "SpatialTemporalUnit: " << "( (";
          os << TemporalUnit<Alpha>::timeInterval.start.ToString();
          os << " ";
          os << TemporalUnit<Alpha>::timeInterval.end.ToString();
          os << " "
             << (TemporalUnit<Alpha>::timeInterval.lc ? "TRUE " : "FALSE ");
          os << " "
             << (TemporalUnit<Alpha>::timeInterval.rc ? "TRUE) " : "FALSE) ");
          // print specific stuff:
          os << " NO SPECIFIC Print()-Method!";
          os << " )" << endl;
          return os;
        }
      else
      return os << "SpatialTemporalUnit: (undef)" << endl;
    }

    virtual size_t HashValue() const
    {
      return 0;
    }

    virtual SpatialTemporalUnit<Alpha, dim>* Clone() const = 0;
    virtual void CopyFrom( const StandardAttribute* right ) = 0;
    virtual size_t Sizeof() const = 0;
    virtual const Rectangle<dim> BoundingBox() const = 0;
};

/*
3.6 Class ~RInt~

*/
typedef Range<CcInt> RInt;

/*
3.6 Class ~RReal~

*/
typedef Range<CcReal> RReal;

/*
3.6 Class ~Periods~

*/
typedef Range<Instant> Periods;

/*
3.6 ConstTemporalUnit

This class will be used in the ~const~ type constructor. It constructs constant
temporal units, i.e. it has a constant value and the temporal function always
return this value. The explicit purpose of the ~const~ type constructor is to
define temporal units for ~int~, ~string~, and ~bool~, i.e., for types where
their values change only in discrete steps.

*/
template <class Alpha>
struct ConstTemporalUnit : public StandardTemporalUnit<Alpha>
{
/*
3.6.1 Constructors, Destructor

*/
  ConstTemporalUnit() {}

  ConstTemporalUnit(bool is_defined):StandardTemporalUnit<Alpha>(is_defined) {}

  ConstTemporalUnit( const Interval<Instant>& interval, const Alpha& a ):
    StandardTemporalUnit<Alpha>( interval )
  {
    constValue.CopyFrom( &a );
  }

  ConstTemporalUnit( const ConstTemporalUnit<Alpha>& u ):
    StandardTemporalUnit<Alpha>( u.timeInterval )
  {
    constValue.CopyFrom( &u.constValue );
  }

/*
3.6.2 Operator redefinitions

*/

  virtual ConstTemporalUnit<Alpha>&
  operator=( const ConstTemporalUnit<Alpha>& i )
  {
    *((TemporalUnit<Alpha>*)this) = *((TemporalUnit<Alpha>*)&i);
    constValue.CopyFrom( &i.constValue );

    return *this;
  }
/*
Redefinition of the copy operator ~=~.

*/

  virtual bool operator==( const ConstTemporalUnit<Alpha>& i ) const
  {
    return *((TemporalUnit<Alpha>*)this) == *((TemporalUnit<Alpha>*)&i) &&
           constValue.Compare( &i.constValue ) == 0;
  }
/*
Returns ~true~ if this temporal unit is equal to the temporal unit ~i~ and ~false~ if they are different.

*/

  virtual bool operator!=( const ConstTemporalUnit<Alpha>& i ) const
  {
    return !( *this == i );
  }
/*
Returns ~true~ if this temporal unit is different to the temporal unit ~i~ and ~false~ if they are equal.

*/

/*
3.6.2 The Temporal Functions

*/
  virtual void TemporalFunction( const Instant& t,
                                 Alpha& result,
                                 bool ignoreLimits = false ) const
  {
    if ( !this->IsDefined() ||
         !t.IsDefined() ||
         (!this->timeInterval.Contains( t ) && !ignoreLimits))
      {
        result.SetDefined( false );
      }
    else
      {
        result.CopyFrom( &constValue );
        result.SetDefined( true );
      }
  }

  virtual bool Passes( const Alpha& val ) const
  {
    if( constValue.Compare( &val ) == 0 )
      return true;
    return false;
  }

  virtual bool At( const Alpha& val, TemporalUnit<Alpha>& result ) const
  {
    if( constValue.Compare( &val ) == 0 )
    {
      ((ConstTemporalUnit<Alpha>*)&result)->CopyFrom( this );
      return true;
    }
    return false;
  }

  virtual void AtInterval( const Interval<Instant>& i,
                           TemporalUnit<Alpha>& result ) const
  {
    TemporalUnit<Alpha>::AtInterval( i, result );
    ((ConstTemporalUnit<Alpha>*)&result)->constValue.CopyFrom( &constValue );
  }

  virtual bool EqualValue( ConstTemporalUnit<Alpha>& i )
  {
    return constValue.Compare( &i.constValue ) == 0;
  }
/*
Returns ~true~ if the value of this temporal unit is equal to the value of the temporal unit ~i~ and ~false~ if they are different.

*/

/*
3.6.3 Functions to be part of relations

*/

  virtual size_t Sizeof() const
  {
    return sizeof( *this );
  }

  virtual int Compare( const Attribute* arg ) const
  {
    ConstTemporalUnit<Alpha>*  ctu = (ConstTemporalUnit<Alpha>*)arg;
    // SPM: this pointer added since my windows gcc (v3.4.2) reports:
    // 'timeInterval' undeclared (first use this function) which
    // seems to be a compiler bug!
    if (TemporalUnit<Alpha>::defined && !ctu->defined)
      return 0;
    if (!TemporalUnit<Alpha>::defined)
      return -1;
    if (!ctu->defined)
      return 1;

    int cmp = this->timeInterval.CompareTo(ctu->timeInterval);
    if(cmp){
       return cmp;
    }
    return constValue.Compare(&(ctu->constValue));
  }

  virtual bool Adjacent( const Attribute* arg ) const
  {
    return false;
  }

  virtual ostream& Print( ostream &os ) const
  {
    if( StandardTemporalUnit<Alpha>::IsDefined() )
      {
        os << "ConstUnit: " << "( (";
        os << TemporalUnit<Alpha>::timeInterval.start.ToString();
        os << " ";
        os << TemporalUnit<Alpha>::timeInterval.end.ToString();
        os<<" "<<(TemporalUnit<Alpha>::timeInterval.lc ? "TRUE " : "FALSE ");
        os<<" "<<(TemporalUnit<Alpha>::timeInterval.rc ? "TRUE) " : "FALSE) ");
        constValue.Print(os);
        os << " )" << endl;
        return os;
      }
    else
      return os << "ConstUnit: (undef)" << endl;
  }

  virtual size_t HashValue() const
  {
    return 0;
  }

  virtual ConstTemporalUnit<Alpha>* Clone() const
  {
    ConstTemporalUnit<Alpha> *res;
    res = new ConstTemporalUnit<Alpha>( this->timeInterval, constValue);
    res->SetDefined(TemporalUnit<Alpha>::defined);
    return res;
  }

  virtual void CopyFrom( const StandardAttribute* right )
  {
    const ConstTemporalUnit<Alpha>* i = (const ConstTemporalUnit<Alpha>*)right;

    TemporalUnit<Alpha>::defined = i->defined;
    this->timeInterval.CopyFrom( i->timeInterval );
    constValue.CopyFrom( &i->constValue );
  }

/*
3.6.4 Attributes

*/

  Alpha constValue;
/*
The constant value of the temporal unit.

*/
};

/*
3.7 Class ~UBool~

*/
typedef ConstTemporalUnit<CcBool> UBool;

/*
3.7 Class ~UInt~

*/
typedef ConstTemporalUnit<CcInt> UInt;

/*
3.7 UReal

This class will be used in the ~ureal~ type constructor, i.e., the type constructor
for the temporal unit of real numbers.

*/
struct UReal : public StandardTemporalUnit<CcReal>
{
/*
3.7.1 Constructors and Destructor

*/
  UReal() {};

  UReal(bool is_defined):StandardTemporalUnit<CcReal>(is_defined) {};

  UReal( const Interval<Instant>& interval,
         const double a,
         const double b,
         const double c,
         const bool r ):
    StandardTemporalUnit<CcReal>( interval ),
    a( a ), b( b ), c( c ),
    r( r )
    {}

  // linear approximation between v1 and v2
  UReal(const Interval<Instant>& interval,
        const double v1,
        const double v2): StandardTemporalUnit<CcReal>(interval){

       Instant diff = interval.end-interval.start;
       a = 0;
       r = false;
       c = v1;
       if(diff.IsZero()){
          b = 0;
          return;
       }
       b = (v2-v1) / diff.ToDouble();
  }

/*
3.6.2 Operator redefinitions

*/

  virtual UReal& operator=( const UReal& i )
  {
    *((TemporalUnit<CcReal>*)this) = *((TemporalUnit<CcReal>*)&i);
    a = i.a;
    b = i.b;
    c = i.c;
    r = i.r;

    return *this;
  }
/*
Redefinition of the copy operator ~=~.

*/

  virtual bool operator==( const UReal& i ) const
  {
    return *((TemporalUnit<CcReal>*)this) == *((TemporalUnit<CcReal>*)&i) &&
           AlmostEqual( a, i.a ) &&
           AlmostEqual( b, i.b ) &&
           AlmostEqual( c, i.c ) &&
           r == i.r;
  }
/*
Returns ~true~ if this temporal unit is equal to the temporal unit ~i~ and ~false~ if they are different.

*/

  virtual bool operator!=( const UReal& i ) const
  {
    return !( *this == i );
  }
/*
Returns ~true~ if this temporal unit is different to the temporal unit ~i~ and ~false~ if they are equal.

*/

/*
3.7.2 The Temporal Functions

*/
  virtual void TemporalFunction( const Instant& t,
                                 CcReal& result,
                                 bool ignoreLimits = false ) const;
  virtual bool Passes( const CcReal& val ) const;
  virtual bool At( const CcReal& val, TemporalUnit<CcReal>& result ) const;
  virtual void AtInterval( const Interval<Instant>& i,
                           TemporalUnit<CcReal>& result ) const;

  virtual bool EqualValue( UReal& i )
  {
    double offset = i.timeInterval.start.ToDouble()
    - timeInterval.start.ToDouble();

    return (!IsDefined() && !i.IsDefined()) ||
      (AlmostEqual( a, i.a ) &&
       AlmostEqual( 2 * a * offset + b, i.b ) &&
       AlmostEqual( a * pow(offset, 2) + b * offset + c, i.c )
       && r == i.r );
  }

/*
Returns ~true~ if the value of this temporal unit is equal to the
value of the temporal unit ~i~ and ~false~ if they are different.
Equality is calculated with respect to temporal evolution.

*/

/*
3.7.3 Functions to be part of relations

*/
/*

----
  virtual bool IsDefined() const
  {
    return true;
  }

  virtual void SetDefined( bool Defined )
  {
  }

----

*/

  virtual size_t Sizeof() const
  {
    return sizeof( *this );
  }

  virtual int Compare( const Attribute* arg ) const
  {
    UReal* mr2 = (UReal*) arg;

    if (!IsDefined() && !mr2->IsDefined())
      return 0;
    if (!IsDefined())
      return -1;
    if (!mr2->IsDefined())
      return 1;

    // both ureals are defined...
    int cmp = timeInterval.CompareTo(mr2->timeInterval);
    if(cmp){
       return cmp;
    }
   // because we can't compare the functions themself, we
   // use the lexicographical order of the parameters
   if(a<mr2->a) return -1;
   if(a>mr2->a) return 1;
   if(b<mr2->b) return -1;
   if(b>mr2->b) return 1;
   if(c<mr2->c) return -1;
   if(c>mr2->c) return 1;
   if(r && !mr2->r) return -1;
   if(!r && mr2->r) return 1;
   return 0;
  }

  virtual bool Adjacent( const Attribute* arg ) const
  {
    return false;
  }

  virtual ostream& Print( ostream &os ) const
  {
    if( IsDefined() )
      {
        os << "UReal: " << "( (";
        os << timeInterval.start.ToString();
        os << " ";
        os << timeInterval.end.ToString();
        os << " "
           << (timeInterval.lc ? "TRUE " : "FALSE ");
        os << " "
           << (timeInterval.rc ? "TRUE) " : "FALSE) ");
        // print specific stuff:
        os << " ( " << a << " " << b << " " << c
           << (timeInterval.rc ? "TRUE) " : "FALSE) ");
        os << " )" << endl;
        return os;
      }
    else
      return os << "UReal: (undef)" << endl;
  }

  virtual size_t HashValue() const
  {
    return 0;
  }

  virtual UReal* Clone() const
  {
    UReal *res;
    res = new UReal( timeInterval, a, b, c, r);
    res->defined = TemporalUnit<CcReal>::defined;
    return res;
  }

  virtual void CopyFrom( const StandardAttribute* right )
  {
    const UReal* i = (const UReal*)right;

    TemporalUnit<CcReal>::defined = i->defined;
    if(i->defined)
      {
        timeInterval.CopyFrom(i->timeInterval);
        a = i->a;
        b = i->b;
        c = i->c;
        r = i->r;
      }
    else
      {
        timeInterval = Interval<Instant>();
        a = 0;
        b = 0;
        c = 0;
        r = false;
      }
  }

/*
3.7.4 Other Methods

*/

  virtual void TranslateParab( const double& t);

/*
   This operator will translate the apex of the parabolic curve
   used within the ureal representation by (t) on the x/time-axes.

*/

  double Integrate() const;
/*
Precondition this[->]IsDefined() is true

*/

  int PeriodsAtVal( const double& value, Periods& times) const;

/*
Sets the Periods value to the times, where this takes the
specified value. Returns the number of results (0-2).

WARNING: May return points, that are not inside this->timeInterval,
         if a value is located at an open start/end instant.

*/

  double PeriodsAtMin(bool& correct, Periods& times) const;
/*
Sets the Periods value to the times, where this takes the
minimum value. Returns the minimum value.

WARNING: May return points, that are not inside this->timeInterval,
         if a value is located at an open start/end instant.

*/

  double PeriodsAtMax(bool& correct, Periods& times) const;
/*
Sets the Periods value to the times, where this takes the
minimum value. Returns the maximum value.

WARNING: May return points, that are not inside this->timeInterval,
         if a value is located at an open start/end instant.

*/

  int PeriodsAtEqual( const UReal& other, Periods& times) const;

/*
Sets the Periods value to the times, where both UReals take the
same value. Returns the number of results (0-2).

*/

  int AtMin( vector<UReal>& result ) const;

/*
Creates a vector of units, which are the restriction of this to
the periods, where it takes its minimum value.

Precondition: this[->]IsDefined()

Result: stores the resultununit into vector result and returns
        the number of results (1-2) found.

WARNING: AtMin may return points, that are not inside this->timeInterval,
         if a minimum is located at an open start/end instant.

*/

  int AtMax( vector<UReal>& result ) const;

/*
Creates a vector of units, which are the restriction of this to
the periods, where it takes its maximum value.

Precondition: this[->]IsDefined()

Result: stores the resultunit into vector result and returns
        the number of results (1-2) found.

WARNING: AtMax may return points, that are not inside this->timeInterval,
         if a maximum is located at an open start/end instant.

*/

  int AtValue(CcReal value, vector<UReal>& result) const;
/*
Creates a vector of units, which are the restriction of this to
the periods, where it takes a certain value.

*Precondition*: this[->]IsDefined() && value.IsDefined()

*Result*: stores the resultununit into vector result and returns
          the number of results (1-2) found.

WARNING: AtMax may return points, that are not inside this->timeInterval,
         if a maximum is located at an open start/end instant.

*/

  int IsEqual(const UReal& other, vector<UBool> result) const;
/*
Creates a vector of ubool, that cover the UReals common deftime and
indicate wheter their temporal values are equal or not.

Precondition: this[->]IsDefined() && value.IsDefined()

Result: stores the resultunit into vector result and returns
        the number of results found.

*/

  double Max(bool& correct) const;
/*

*Precondition*: this[->]IsDefined() is true

*/

  double Min(bool& correct) const;
/*

*Precondition*: this[->]IsDefined() is true

*/
  
  void Linearize(UReal& result) const;

 
   void Linearize(UReal& result1, UReal& result2) const; 
   /*
     Stores a linear approximation of this UReal in result1 and result2.
     If the extremum is outside of the corresponding interval, result2 will 
     be undefined and result 1 contains a linear approximation between the
     value at the interval boundaries. Otherwise this unit is split at the 
     extremum.
   */


/*
3.7.5 Attributes

*/
  double a, b, c;
  bool r;
};

/*
3.8 UPoint

This class will be used in the ~upoint~ type constructor, i.e., the type constructor
for the temporal unit of point values.

*/
struct UPoint : public SpatialTemporalUnit<Point, 3>
{
/*
3.8.1 Constructors and Destructor

*/
  UPoint() {};

  UPoint(bool is_defined):SpatialTemporalUnit<Point, 3>(is_defined) {};

  UPoint( const Interval<Instant>& interval,
          const double x0, const double y0,
          const double x1, const double y1 ):
    SpatialTemporalUnit<Point, 3>( interval ),
    p0( true, x0, y0 ),
    p1( true, x1, y1 )
    {}

  UPoint( const Interval<Instant>& interval,
          const Point& p0, const Point& p1 ):
    SpatialTemporalUnit<Point, 3>( interval ),
    p0( p0 ),
    p1( p1 )
    {}

/*
3.6.2 Operator redefinitions

*/

  virtual UPoint& operator=( const UPoint& i )
  {
    *((TemporalUnit<Point>*)this) = *((TemporalUnit<Point>*)&i);
    p0 = i.p0;
    p1 = i.p1;

    return *this;
  }
/*
Redefinition of the copy operator ~=~.

*/

  virtual bool operator==( const UPoint& i ) const
  {
    return *((TemporalUnit<Point>*)this) == *((TemporalUnit<Point>*)&i) &&
           AlmostEqual( p0, i.p0 ) &&
           AlmostEqual( p1, i.p1 );
  }
/*
Returns ~true~ if this temporal unit is equal to the temporal unit ~i~ and ~false~ if they are different.

*/

  virtual bool operator!=( const UPoint& i ) const
  {
    return !( *this == i );
  }
/*
Returns ~true~ if this temporal unit is different to the temporal unit ~i~ and ~false~ if they are equal.

*/

/*
3.8.2 The Temporal Functions

*/

  virtual void TemporalFunction( const Instant& t,
                                 Point& result,
                                 bool ignoreLimits = false ) const;
  virtual bool Passes( const Point& val ) const;
  bool Passes( const Region& val ) const;
  virtual bool At( const Point& val, TemporalUnit<Point>& result ) const;
  virtual void AtInterval( const Interval<Instant>& i,
                           TemporalUnit<Point>& result ) const;
  void Distance( const Point& p, UReal& result ) const;
  //  void UTrajectory( UPoint& unit,Line& line ) const;
  void UTrajectory( Line& line ) const;
  void USpeed( UReal& result ) const;
  void UVelocity( UPoint& result ) const;
  void Intersection(const UPoint &other, UPoint &result) const;
  virtual bool EqualValue( UPoint& i )
  {
  return   AlmostEqual( p0, i.p0 ) &&
           AlmostEqual( p1, i.p1 );
  }

/*
3.8.3 Functions to be part of relations

*/

/*

----

  inline virtual bool IsDefined() const
  {
    return true;
  }

  inline virtual void SetDefined( bool Defined )
  {
  }

----

*/

  inline virtual size_t Sizeof() const
  {
    return sizeof( *this );
  }

  inline virtual int Compare( const Attribute* arg ) const
  {
    UPoint* up2 = (UPoint*) arg;
    if (!IsDefined() && !up2->IsDefined())
      return 0;
    if (!IsDefined())
      return -1;
    if (!up2->IsDefined())
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

    if(IsDefined())
      {
        os << "UPoint: " << "( (";
        os << timeInterval.start.ToString();
        os << " ";
        os << timeInterval.end.ToString();
        os << " " << (timeInterval.lc ? "TRUE " : "FALSE ");
        os << " " << (timeInterval.rc ? "TRUE) " : "FALSE) ");
        p0.Print(os);
        os << " ";
        p1.Print(os);
        os << " )" << endl;
        return os;
      }
    else
      return os << "UPoint: (undef)" << endl;
  }

  inline virtual size_t HashValue() const
  {
    return 0;
  }

  inline virtual UPoint* Clone() const
  {
    UPoint *res;
    res = new UPoint( timeInterval, p0, p1 );
    res->defined = TemporalUnit<Point>::defined;
    return res;
  }

  inline virtual void CopyFrom( const StandardAttribute* right )
  {
    const UPoint* i = (const UPoint*)right;

    TemporalUnit<Point>::defined = i->defined;
    if(i->defined)
      {
        timeInterval.CopyFrom( i->timeInterval );
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
    return Rectangle<3>( true, MIN( p0.GetX(), p1.GetX() ),
                               MAX( p0.GetX(), p1.GetX() ),
                               MIN( p0.GetY(), p1.GetY() ),
                               MAX( p0.GetY(), p1.GetY() ),
                               timeInterval.start.ToDouble(),
                               timeInterval.end.ToDouble() );
  }

/*
3.8.4 Attributes

*/
  Point p0, p1;
};

/*
3.9 Mapping

This class will implement the functionalities of the ~mapping~ type constructor.
It contains a database array of temporal units. For that, ~Unit~ must implement
the class ~TemporalUnit~ or ~ConstTemporalUnit~, because functions of these classes will be used.

*/
template <class Unit, class Alpha>
class Mapping : public StandardAttribute
{
  public:
/*
3.9.1 Constructors and Destructor

*/
    Mapping() {}
/*
The simple constructor. This constructor should not be used.

*/

    Mapping( const int n );
/*
The constructor. Initializes space for ~n~ elements.

*/

    virtual ~Mapping();
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

    void Get( const int i, const Unit*& upi ) const;
/*
Returns the unit ~upi~ at the position ~i~ in the mapping.

*/

    void Add( const Unit& upi );
/*
Adds an unit ~upi~ to the mapping. We will assume that the only way of adding units
is in bulk loads, i.e., in a non-ordered array.

*Precondition:* ~IsOrdered() == false~

*/

    void MergeAdd( Unit& upi );
/*
Adds an unit ~upi~ to the mapping. If the new unit and the last
unit in the Mapping are equalValue it merges the two units.
We will assume that the only way of adding units
is in bulk loads, i.e., in a non-ordered array.
Without defining the function ~equalValue~ for units
~MergeAdd~ works the same way as ~Add~.

*Precondition:* ~IsOrdered() == false~

*/

    void Clear();
/*
Remove all units in the mapping.

*/

    bool IsValid() const;
/*
This functions tests if a ~mapping~ is in a valid format. It is used for debugging
purposes only. The ~mapping~ is valid, if the following conditions are true:

  1 Each unit is valid

  2 Start of each unit $>=$ end of the unit before

  3 If start of an unit = end of the unit before, then one needs to
    make sure that the unit is not left-closed or the unit before
    is not right-closed

3.10.4 Functions to be part of relations

*/
    inline bool IsDefined() const;
    inline void SetDefined( bool Defined );
    inline virtual size_t Sizeof() const;
    inline int Compare( const Attribute* arg ) const;
    inline bool Adjacent( const Attribute* arg ) const;
    inline Mapping<Unit, Alpha>* Clone() const;
    inline ostream& Print( ostream &os ) const;
    inline size_t HashValue() const;
    inline void CopyFrom( const StandardAttribute* right );
    inline void Restrict( const vector< pair<int, int> >& intervals );

    inline int NumOfFLOBs() const;
    inline FLOB *GetFLOB(const int i);

/*
3.10.5 Operations

3.10.5.1 Operation $=$ (~equal~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X = Y$

*Complexity:* $O(n+m)$, where ~n~ is the number of units of this mapping ~X~ and m the
number of units of the mapping ~Y~.

*/
    bool operator==( const Mapping<Unit, Alpha>& mp ) const;

/*
3.10.5.2 Operation $\neq$ (~not equal~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \neq Y$

*Complexity:* $O(n+m)$, where ~n~ is the number of units of this mapping ~X~ and m the
number of units of the mapping ~Y~.

*/
    bool operator!=( const Mapping<Unit, Alpha>& mp ) const;

/*
3.10.5.3 Operation ~no\_components~

*Precondition:* ~X.IsOrdered()~

*Semantics:* $\| intvls(X) \|$

*Complexity:* $O(1)$

*/
    int GetNoComponents() const;

/*
3.10.5.3 Operation ~position~

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( \log n )$, where ~n~ is the number of units of this mapping ~X~

*/
    int Position( const Instant& t ) const;

/*
3.10.5.3 Operation ~atinstant~

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( \log n )$, where ~n~ is the number of units of this mapping ~X~

*/
    void AtInstant( const Instant& t, Intime<Alpha>& result ) const;

/*
3.10.5.3 Operation ~atperiods~

*Precondition:* ~X.IsOrdered() \&\& Y.IsOrdered()~

*Semantics:*

*Complexity:* $O( m + n )$, where ~n~ is the number of units of this mapping ~X~
and ~m~ is the number of intervals of the periods ~Y~

*/
    void AtPeriods( const Periods& periods,
                    Mapping<Unit, Alpha>& result ) const;

/*
3.10.5.3 Operation ~present~

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( \log n )$, where ~n~ is the number of units of this mapping ~X~

*/
    bool Present( const Instant& t ) const;
    bool Present( const Periods& periods ) const;

/*
3.10.5.3 Operation ~passes~

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( n )$, where ~n~ is the number of units of this mapping ~X~

*/
    template <class Beta>
    bool Passes( const Beta& val ) const;

/*
3.10.5.3 Operation ~deftime~

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( n )$, where ~n~ is the number of units of this mapping ~X~

*/
    void DefTime( Periods& result ) const;

/*
3.10.5.3 Operation ~at~

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( n )$, where ~n~ is the number of units of this mapping ~X~

*/
    void At( const Alpha& val, Mapping<Unit, Alpha>& result ) const;

/*
3.10.5.3 Operation ~initial~

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( 1 )$

*/
    void Initial( Intime<Alpha>& result ) const;

/*
3.10.5.3 Operation ~final~

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( 1 )$

*/
    void Final( Intime<Alpha>& result ) const;


/*
3.10.5.3 ~Resize~

Changes the unit's array to have space for ~n~ entries.

*/
    void Resize(const size_t n);


/*
3.10.4.4 ~ExtendDefTime~

The result takes all units from this Mapping. Additionally the deftime
is extended to the union with the interval of the given unit. Missing parts
in the definition time of this Mapping are filled with the value given by the
unit.  Note that this implementation works only for instances of
ConstTempralUnit - Mappings.

*/
    void ExtendDefTime(Unit u, Mapping<Unit, Alpha>& result);

/*
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

    DBArray< Unit > units;
/*
The database array of temporal units.

*/
};

/*
3.10 Class ~MBool~

*/
typedef Mapping< UBool, CcBool > MBool;

/*
3.10 Class ~MInt~

*/
class MInt : public  Mapping< UInt, CcInt > {
public:
/*
~Simple Constructor~

*/
   MInt(){}

/*
~Constructor~

Initializes the size of the array of units

*/
   MInt(const int n):
     Mapping<UInt, CcInt>(n){}

/*
~ReadFrom~

Reads the value of this moving(int) from the value
of the given moving(bool). The temporal structure
is equal to the temporal structure of the moving(bool).
A unit holding the value false is converted to a unit with
value zero. Units with value true are converted into units
representing the  value 1 over their interval.

*/
   void ReadFrom(const MBool& arg);



};

/*
3.11 Class ~MReal~

*/
class MReal: public Mapping< UReal, CcReal > {
public:

/*
3.11.1 Constructory

*/

   MReal() {}

   MReal( const int n ):
      Mapping< UReal, CcReal >( n )
      {}

/*
3.11.2 Operation ~Simplify~

Simplifies the connected linear approximated parts of this moving real.

*/
   void Simplify(const double epsilon, MReal& result) const;

/*
3.11.3 Operation ~Integrate~

Compute the integral of this moving real.

*/
   double Integrate();

/*
3.11.4 Operation ~Max~

*/

   double Max(bool& correct) const;

/*
3.11.5 Operation ~Min~

*/
   double Min(bool& correct) const;

/*
3.11.6 Operation ~Linearize~

This function replaces all units by linear approximations between their
start and end value.

*/
   void Linearize(MReal& result) const;


/*
3.11.6 Operation ~Linearize2~

This function replaces all units by linear approximations between their
start and end value and possible the extremum.

*/
   void Linearize2(MReal& result) const;


/*
3.11.6 Operation ~AtMin~

Restrict to periods where the temporal value is maximal.

*/
   void AtMin( MReal& result ) const;

/*
3.11.7 Operation ~AtMax~

Restrict to periods where the temporal value is minimum.

*/
   void AtMax( MReal& result ) const;

/*
3.11.8 Operation ~AtMax~

Restrict to periods where the temporal value is equal to a const value.

Precondition: ccvalue.IsDefined() == true

*/
   void AtValue( const CcReal& ccvalue, MReal& result ) const;


private:
   void Simplify(const int min, const int max,
                 bool* useleft, bool* useright,
                 const double epsilon) const;

};

/*
3.12 Class ~MPoint~

*/
class MPoint : public Mapping< UPoint, Point >
{
  public:
/*
3.12.1 Constructors and Destructor

*/
    MPoint() {}
/*
The simple constructor. This constructor should not be used.

*/

    MPoint( const int n ):
      Mapping< UPoint, Point >( n )
      {}
/*
The constructor. Initializes space for ~n~ elements.

*/

/*
3.10.5.3 Operation ~trajectory~

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( n )$, where ~n~ is the number of units of this ~MPoint~

*/
  void Trajectory( Line& line ) const;
  void MSpeed(  MReal& result ) const;
  void MVelocity( MPoint& result ) const;

/*
3.10.5.3 Operation ~distance~

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( n )$, where ~n~ is the number of units of this ~MPoint~

*/
  void Distance( const Point& p, MReal& result ) const;


/*
3.10.5.4 Operation ~Simplify~

*Precondition*: ~X.IsOrdered()~

*Semantics*: Simplifies the tracjectory this moving point.

*Complexity*: worst case: O(n * n), in average O(n * log (n))

*/

   void Simplify(const double epsilon, MPoint& result,
                 const bool checkBreakPoints,
                 const DateTime& duration) const;

/*
3.10.5.5 Operation ~BreakPoints~

*Precondition*: ~X.IsOrdered()~
*Semantics*: Computes all points where this mpoints stays longer than the given
             time.

*/
    void BreakPoints(Points& result, const DateTime& dur) const;


private:
   void Simplify(const int min, const int max,
                 bool* useleft, bool* useright,
                 const double epsilon) const;



};

/*
4 Implementation of C++ Classes

4.1 Interval

4.1.1 Constructors and Destructor

*/
template <class Alpha>
Interval<Alpha>::Interval( const Interval<Alpha>& interval ):
start(),
end(),
lc( interval.lc ),
rc( interval.rc )
{
  this->start.CopyFrom( &interval.start );
  this->end.CopyFrom( &interval.end );
}

template <class Alpha>
Interval<Alpha>::Interval( const Alpha& start, const Alpha& end,
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
void Interval<Alpha>::CopyFrom( const Interval<Alpha>& i )
{
  start.CopyFrom( &i.start );
  end.CopyFrom( &i.end );
  lc = i.lc;
  rc = i.rc;
}

template <class Alpha>
bool Interval<Alpha>::IsValid() const
{
  if( !start.IsDefined() || !end.IsDefined() )
    return false;

  int cmp = start.Compare( &end );
  if( cmp < 0 ) // start < end
  {
    if( start.Adjacent( &end ) )
      return lc || rc;
    else
      return true;
  }
  else if( cmp == 0 ) // start == end
  {
    return rc && lc;
  }
  // start > end
  return false;
}

/*
4.1.2 Member functions

*/
template <class Alpha>
Interval<Alpha>& Interval<Alpha>::operator=( const Interval<Alpha>& i )
{
  assert( i.IsValid() );

  start.CopyFrom( &i.start );
  end.CopyFrom( &i.end );
  lc = i.lc;
  rc = i.rc;

  return *this;
}

template <class Alpha>
bool Interval<Alpha>::operator==( const Interval<Alpha>& i ) const
{
  assert( IsValid() && i.IsValid() );

  return( ( start.Compare( &i.start ) == 0 && lc == i.lc &&
            end.Compare( &i.end ) == 0 && rc == i.rc ) ||
          ( start.Compare( &i.start ) == 0 && lc == i.lc &&
            end.Compare( &i.end ) != 0 &&
            end.Adjacent( &i.end ) && ( !rc || !i.rc ) ) ||
          ( end.Compare( &i.end ) == 0 && rc == i.rc &&
            start.Compare( &i.start ) != 0 && start.Adjacent( &i.start ) &&
            ( !lc || !i.lc ) ) ||
          ( start.Compare( &i.start ) != 0 && start.Adjacent( &i.start )
            && (!lc || !i.lc) &&
            end.Compare( &i.end ) != 0 && end.Adjacent( &i.end ) &&
            ( !rc || !i.rc ) ) );
}

template <class Alpha>
bool Interval<Alpha>::operator!=( const Interval<Alpha>& i ) const
{
  return !( *this == i );
}

template <class Alpha>
bool Interval<Alpha>::R_Disjoint( const Interval<Alpha>& i ) const
{
    bool res= ((end.Compare( &i.start ) < 0) ||
               ( end.Compare( &i.start ) == 0 && !( rc && i.lc ) ));
    return( res );
}

template <class Alpha>
bool Interval<Alpha>::Disjoint( const Interval<Alpha>& i ) const
{
  assert( IsValid() && i.IsValid() );
  bool res=( R_Disjoint( i ) || i.R_Disjoint( *this ) );
  return( res );
}

template <class Alpha>
bool Interval<Alpha>::R_Adjacent( const Interval<Alpha>& i ) const
{
    bool res=( Disjoint( i ) &&
               ( end.Compare( &i.start ) == 0 && (rc || i.lc) ) ||
               ( ( end.Compare( &i.start ) < 0 && rc && i.lc ) &&
                 end.Adjacent( &i.start ) ) );
    return( res );
}

template <class Alpha>
bool Interval<Alpha>::Adjacent( const Interval<Alpha>& i ) const
{
  assert( IsValid() && i.IsValid() );
  bool res= ( R_Adjacent( i ) || i.R_Adjacent( *this ) );
  return( res );
}

template <class Alpha>
bool Interval<Alpha>::Inside( const Interval<Alpha>& i ) const
{
  assert( IsValid() && i.IsValid() );

  return( ( start.Compare( &i.start ) > 0 ||
            ( start.Compare( &i.start ) == 0 && ( !lc || i.lc ) ) ) &&
          ( end.Compare( &i.end ) < 0 ||
            ( end.Compare( &i.end ) == 0 && ( !rc || i.rc ) ) ) );
}

template <class Alpha>
bool Interval<Alpha>::Contains( const Alpha& a ) const
{
  assert( IsValid() && a.IsDefined() );

  return ( ( start.Compare( &a ) < 0 ||
             ( start.Compare( &a ) == 0 && lc ) ) &&
           ( end.Compare( &a ) > 0 ||
             ( end.Compare( &a ) == 0 && rc ) ) );
}

template <class Alpha>
bool Interval<Alpha>::Intersects( const Interval<Alpha>& i ) const
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
bool Interval<Alpha>::Before( const Interval<Alpha>& i ) const
{
  assert( IsValid() && i.IsValid() );

  if( Before( i.start ) )
    return true;

  return end.Compare( &i.start ) == 0 && i.lc == false;
}

template <class Alpha>
bool Interval<Alpha>::Before( const Alpha& a ) const
{
  assert( IsValid() && a.IsDefined() );

  return ( end.Compare( &a ) < 0 ||
           ( end.Compare( &a ) == 0 && rc == false ) );
}

template <class Alpha>
bool Interval<Alpha>::After( const Alpha& a ) const
{
  assert( IsValid() && a.IsDefined() );

  return ( start.Compare( &a ) > 0 ||
           ( start.Compare( &a ) == 0 && lc == false ) );
}

template <class Alpha>
void Interval<Alpha>::Intersection( const Interval<Alpha>& i,
                                    Interval<Alpha>& result ) const
{
  assert( IsValid() && i.IsValid() );
  assert( Intersects( i ) );

  if( Inside( i ) )
    result = *this;
  else if( i.Inside( *this ) )
    result = i;
  else
    // Normal intersection
  {
    int comp = start.Compare( &i.start );
    if( comp < 0 )
    {
      result.start.CopyFrom( &i.start );
      result.lc = i.lc;
    }
    else if( comp == 0 )
    {
      result.start.CopyFrom( &i.start );
      result.lc = lc || i.lc;
    }
    else
    {
      result.start.CopyFrom( &start );
      result.lc = lc;
    }

    comp = end.Compare( &i.end );
    if( comp > 0 )
    {
      result.end.CopyFrom( &i.end );
      result.rc = i.rc;
    }
    else if( comp == 0 )
    {
      result.end.CopyFrom( &i.end );
      result.rc = rc || i.rc;
    }
    else
    {
      result.end.CopyFrom( &this->end );
      result.rc = rc;
    }
  }
}

template <class Alpha>
int  Interval<Alpha>::CompareTo( const Interval<Alpha>& i) const{
   if(start<i.start){
      return -1;
   }
   if(start>i.start){
      return 1;
   }
   if(!lc && i.lc){
      return -1;
   }
   if(lc && !i.lc){
      return 1;
   }
   if(end<i.end){
       return -1;
   }
   if(end>i.end){
       return 1;
   }
   if(rc && !i.rc){
      return 1;
   }
   if(!rc && i.rc){
      return -1;
   }
   return 0;
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
  //assert( IsValid() );
  if (!IsValid() )  //do merge first  ????????? I will do it afterwords. DZM
  {
   //   Range<Alpha> *copy; //= new Range<Alpha>( 0 );
   //   copy=this->Clone();
   //   this->Clear();
   //   copy->Merge(this);
   //   cout<<"invalid range found!!!"<<endl;
  }
  //assert( IsValid() );
}

template <class Alpha>
bool Range<Alpha>::IsEmpty() const
{
  return intervals.Size() == 0;
}

template <class Alpha>
void Range<Alpha>::Get( const int i, const Interval<Alpha>*& interval ) const
{
  intervals.Get( i, interval );
  assert( interval->IsValid() );
}

template <class Alpha>
void Range<Alpha>::Add( const Interval<Alpha>& interval )
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
inline bool Range<Alpha>::IsDefined() const
{
  return true;
}

template <class Alpha>
inline void Range<Alpha>::SetDefined( bool Defined )
{
}

template <class Alpha>
inline size_t Range<Alpha>::Sizeof() const
{
  return sizeof( *this );
}

template <class Alpha>
inline int Range<Alpha>::Compare( const Attribute* arg ) const
{
  return 0;
}

template <class Alpha>
inline bool Range<Alpha>::Adjacent( const Attribute* arg ) const
{
  return false;
}

template <class Alpha>
inline Range<Alpha>* Range<Alpha>::Clone() const
{
  assert( IsOrdered() );

  Range *result = new Range( GetNoComponents() );

  result->StartBulkLoad();
  const Interval<Alpha> *interval;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, interval );
    result->Add( *interval );
  }
  result->EndBulkLoad( false );

  return result;
}

template <class Alpha>
inline ostream& Range<Alpha>::Print( ostream &os ) const
{
  return os << "Range Algebra" << endl;
}

template <class Alpha>
inline size_t Range<Alpha>::HashValue() const
{
  return 0;
}

template <class Alpha>
inline void Range<Alpha>::CopyFrom( const StandardAttribute* right )
{
  const Range<Alpha> *r = (const Range<Alpha>*)right;
  assert( r->IsOrdered() );

  Clear();

  StartBulkLoad();
  const Interval<Alpha> *interval;
  for( int i = 0; i < r->GetNoComponents(); i++ )
  {
    r->Get( i, interval );
    Add( *interval );
  }
  EndBulkLoad( false );
}

template <class Alpha>
inline int Range<Alpha>::NumOfFLOBs() const
{
  return 1;
}

template <class Alpha>
inline FLOB *Range<Alpha>::GetFLOB(const int i)
{
  assert( i == 0 );
  return &intervals;
}

/*
4.2.4 Operator functions

*/
template <class Alpha>
bool Range<Alpha>::operator==( const Range<Alpha>& r ) const
{
  assert( IsValid() && r.IsValid() );

  if( GetNoComponents() != r.GetNoComponents() )
    return false;

  bool result = true;
  const Interval<Alpha> *thisInterval, *interval;

  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, thisInterval );
    r.Get( i, interval );

    if( *thisInterval != *interval )
    {
      result = false;
      break;
    }
  }

  return result;
}

template <class Alpha>
bool Range<Alpha>::operator!=( const Range<Alpha>& r ) const
{
  return !( *this == r );
}

template <class Alpha>
bool Range<Alpha>::Intersects( const Range<Alpha>& r ) const
{
  assert( IsValid() && r.IsValid() );

  if( IsEmpty() || r.IsEmpty() )
    return false;

  bool result = false;
  const Interval<Alpha> *thisInterval, *interval;

  int i = 0, j = 0;
  Get( i, thisInterval );
  r.Get( j, interval );

  while( 1 )
  {
    if( thisInterval->Intersects( *interval ) )
    {
      result = true;
      break;
    }

    if( thisInterval->Before( *interval ) )
    {
      if( ++i == GetNoComponents() )
      {
        result = false;
        break;
      }
      Get( i, thisInterval );
    }

    if( interval->Before( *thisInterval ) )
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
bool Range<Alpha>::Inside( const Range<Alpha>& r ) const
{
  assert( IsValid() && r.IsValid() );

  if( IsEmpty() ) return true;
  if( r.IsEmpty() ) return false;

  bool result = true;
  const Interval<Alpha> *thisInterval, *interval;

  int i = 0, j = 0;
  Get( i, thisInterval );
  r.Get( j, interval );

  while( 1 )
  {
    if( interval->Before( *thisInterval ) )
    {
      if( ++j == r.GetNoComponents() )
      {
        result = false;
        break;
      }
      r.Get( j, interval );
    }
    else if( thisInterval->Inside( *interval ) )
    {
      if( ++i == GetNoComponents() )
      {
        break;
      }
      Get( i, thisInterval );
    }
    else if( thisInterval->Before( *interval ) )
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
bool Range<Alpha>::Contains( const Alpha& a ) const
{
  assert( IsValid() && a.IsDefined() );

  if( IsEmpty() )
    return false;

  bool result = false;
  const Interval<Alpha> *midInterval;

  int first = 0, last = GetNoComponents() - 1;

  while (first <= last)
  {
    int mid = ( first + last ) / 2;
    Get( mid, midInterval );
    if( midInterval->Contains( a ) )
    {
      result = true;
      break;
    }
    else if( midInterval->Before( a ) )
      first = mid + 1;
    else if( midInterval->After( a ) )
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
bool Range<Alpha>::Before( const Range<Alpha>& r ) const
{
  assert( IsValid() && r.IsValid() );
  assert( !IsEmpty() && !r.IsEmpty() );

  const Interval<Alpha> *thisInterval, *interval;
  Get( GetNoComponents() - 1, thisInterval );
  r.Get( 0, interval );

  return thisInterval->Before( *interval );
}

template <class Alpha>
bool Range<Alpha>::Before( const Alpha& a ) const
{
  assert( IsValid() && !IsEmpty() && a.IsDefined() );

  const Interval<Alpha> *thisInterval;
  Get( GetNoComponents() - 1, thisInterval );
  return thisInterval->Before( a );
}

template <class Alpha>
bool Range<Alpha>::After( const Alpha& a ) const
{
  assert( IsValid() && !IsEmpty() && a.IsDefined() );

  const Interval<Alpha> *thisInterval;
  Get( 0, thisInterval );

  return thisInterval->After( a );
}

template <class Alpha>
void Range<Alpha>::Intersection( const Range<Alpha>& r,
                                 Range<Alpha>& result ) const
{
  assert( IsValid() && r.IsValid() && result.IsEmpty() );

  const Interval<Alpha> *thisInterval, *interval;

  int i = 0, j = 0;
  Get( i, thisInterval );
  r.Get( j, interval );

  result.StartBulkLoad();
  while( i < GetNoComponents() && j < r.GetNoComponents() )
  {
    if( thisInterval->start.Compare( &interval->start ) == 0 &&
        thisInterval->end.Compare( &interval->end ) == 0 )
    {
      Interval<Alpha> newInterval( thisInterval->start, thisInterval->end,
                            thisInterval->lc && interval->lc,
                            thisInterval->rc && interval->rc );
      if( newInterval.IsValid() )
        result.Add( newInterval );
      if( ++i < GetNoComponents() )
        Get( i, thisInterval );
      if( ++j < r.GetNoComponents() )
        r.Get( j, interval );
    }
    else if( thisInterval->Inside( *interval ) )
    {
      Interval<Alpha> newInterval( *thisInterval );
      if( newInterval.IsValid() )
        result.Add( newInterval );
      if( ++i < GetNoComponents() )
        Get( i, thisInterval );
    }
    else if( interval->Inside( *thisInterval ) )
    {
      Interval<Alpha> newInterval( *interval );
      if( newInterval.IsValid() )
        result.Add( newInterval );
      if( ++j < r.GetNoComponents() )
        r.Get( j, interval );
    }
    else if( thisInterval->Intersects( *interval ) )
    {
      if( thisInterval->start.Compare( &interval->end ) == 0 &&
          thisInterval->lc && interval->rc )
      {
        Interval<Alpha> newInterval( interval->end, interval->end, true, true );
        result.Add( newInterval );
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( thisInterval->end.Compare( &interval->start ) == 0 &&
               thisInterval->rc && interval->lc )
      {
        Interval<Alpha> newInterval( interval->start,
                                     interval->start, true, true );
        result.Add( newInterval );
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( thisInterval->start.Compare( &interval->start ) < 0 )
      {
        Interval<Alpha> newInterval( interval->start,
                                     thisInterval->end,
                                     interval->lc, thisInterval->rc );
        if( newInterval.IsValid() )
          result.Add( newInterval );
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( thisInterval->start.Compare( &interval->start ) == 0 )
      {
        assert( !thisInterval->lc || !interval->lc );
        if( thisInterval->end.Compare( &interval->end ) > 0 )
        {
          Interval<Alpha> newInterval( interval->start,
                                       interval->end,
                                       interval->lc && thisInterval->lc,
                                       interval->rc );
          if( newInterval.IsValid() )
            result.Add( newInterval );
          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else
        {
          assert( thisInterval->end.Compare( &interval->end ) < 0 );
          Interval<Alpha> newInterval( thisInterval->start,
                                       thisInterval->end,
                                       interval->lc && thisInterval->lc,
                                       thisInterval->rc );
          if( newInterval.IsValid() )
            result.Add( newInterval );
          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
      }
      else
      {
        Interval<Alpha> newInterval( thisInterval->start,
                                     interval->end,
                                     thisInterval->lc, interval->rc );
        if( newInterval.IsValid() )
        result.Add( newInterval );
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
    }
    else if( thisInterval->start.Compare( &interval->start ) <= 0 )
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
void Range<Alpha>::Union( const Range<Alpha>& r, Range<Alpha>& result ) const
{
  assert( IsValid() && r.IsValid() && result.IsEmpty() );

  const Interval<Alpha> *thisInterval, *interval;

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
      if( thisInterval->start.Compare( &interval->start ) == 0 &&
          thisInterval->end.Compare( &interval->end ) == 0 )
      {
        Interval<Alpha> newInterval( thisInterval->start, thisInterval->end,
                                     thisInterval->lc || interval->lc,
                                     thisInterval->rc || interval->rc );
        result.Add( newInterval );

        if( ++i < GetNoComponents() )
          Get( i, thisInterval );

        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( interval->Inside( *thisInterval ) )
      {
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( thisInterval->Inside( *interval ) )
      {
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( !thisInterval->Intersects( *interval ) )
      {
        if( thisInterval->end.Compare( &interval->start ) < 0 )
        {
          if( thisInterval->Adjacent( *interval ) )
          {
            if( start != NULL && end != NULL )
            {
              delete end; end = NULL;
            }
            else
            {
              assert( start == NULL );
              start = thisInterval->start.Clone();
              lc = thisInterval->lc;
            }
            assert( end == NULL );
            end = interval->end.Clone();
            rc = interval->rc;
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
              Interval<Alpha> newInterval( *thisInterval );
              result.Add( newInterval );
            }
          }

          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
        else if( thisInterval->start.Compare( &interval->end ) > 0 )
        {
          if( thisInterval->Adjacent( *interval ) )
          {
            if( start != NULL && end != NULL )
            {
              delete end;
            }
            else
            {
              assert( start == NULL );
              start = interval->start.Clone();
              lc = interval->lc;
            }
            assert( end == NULL );
            end = thisInterval->end.Clone();
            rc = thisInterval->rc;
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
              Interval<Alpha> newInterval( *interval );
              result.Add( newInterval );
            }
          }

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else if( thisInterval->start.Compare( &interval->end ) == 0 )
        {
          if( !thisInterval->lc && !interval->rc )
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
              Interval<Alpha> newInterval( *interval );
              result.Add( newInterval );
            }
          }
          else
          {
            if( start != NULL && end != NULL )
            {
              if( end->Compare( &thisInterval->end ) < 0 )
              {
                delete end;
                end = thisInterval->end.Clone();
                rc = thisInterval->rc;
              }
              else if( end->Compare( &thisInterval->end ) == 0 )
              {
                rc = rc || thisInterval->rc;
              }
            }
            else
            {
              assert( start == NULL );
              start = interval->start.Clone();
              lc = interval->lc;
              assert( end == NULL );
              end = thisInterval->end.Clone();
              rc = thisInterval->rc;
            }
          }

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else if( interval->start.Compare( &thisInterval->end ) == 0 )
        {
          if( !interval->lc && !thisInterval->rc )
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
              Interval<Alpha> newInterval( *thisInterval );
              result.Add( newInterval );
            }
          }
          else
          {
            if( start != NULL && end != NULL )
            {
              if( end->Compare( &interval->end ) < 0 )
              {
                delete end;
                end = interval->end.Clone();
                rc = interval->rc;
              }
              else if( end->Compare( &interval->end ) == 0 )
              {
                rc = rc || interval->rc;
              }
            }
            else
            {
              assert( start == NULL );
              start = thisInterval->start.Clone();
              lc = thisInterval->lc;
              assert( end == NULL );
              end = interval->end.Clone();
              rc = interval->rc;
            }
          }

          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
      }
      else if( thisInterval->start.Compare( &interval->start ) < 0 )
      {
        if( start == NULL && end == NULL )
        {
          start = thisInterval->start.Clone();
          lc = thisInterval->lc;
          end = interval->end.Clone();
          rc = interval->rc;
        }
        else
        {
          if( end->Compare( &interval->end ) < 0 )
          {
            assert( end == NULL );
            end = interval->end.Clone();
            rc = interval->rc;
          }
          if( end->Compare( &interval->end ) == 0 )
          {
            rc = rc || interval->rc;
          }
        }

        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( interval->start.Compare( &thisInterval->start ) < 0 )
      {
        if( start == NULL && end == NULL )
        {
          start = interval->start.Clone();
          lc = interval->lc;
          end = thisInterval->end.Clone();
          rc = thisInterval->rc;
        }
        else
        {
          if( end->Compare( &thisInterval->end ) < 0 )
          {
            assert( end == NULL );
            end = thisInterval->end.Clone();
            rc = thisInterval->rc;
          }
          if( end->Compare( &thisInterval->end ) == 0 )
          {
            rc = rc || thisInterval->rc;
          }
        }

        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( thisInterval->start.Compare( &interval->start ) == 0 )
      {
        assert( start == NULL && end == NULL );
        start = thisInterval->start.Clone();
        lc = thisInterval->lc || interval->lc;
        if( thisInterval->end.Compare( &interval->end ) < 0 )
        {
          end = interval->end.Clone();
          rc = interval->rc;

          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
        else
        {
          end = thisInterval->end.Clone();
          rc = thisInterval->rc;

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
      }
      else if( thisInterval->end.Compare( &interval->end ) == 0 )
      {
        assert( start != NULL && end != NULL );
        rc = thisInterval->rc || interval->rc;

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
    assert( start == NULL && end == NULL );
  }

  while( i < GetNoComponents() )
  {
    Interval<Alpha> newInterval( *thisInterval );
    result.Add( newInterval );

    if( ++i < GetNoComponents() )
      Get( i, thisInterval );
  }

  while( j < r.GetNoComponents() )
  {
    Interval<Alpha> newInterval( *interval );
    result.Add( newInterval );

    if( ++j < r.GetNoComponents() )
      r.Get( j, interval );
  }
  result.EndBulkLoad( false );
}

template <class Alpha>
void Range<Alpha>::Minus( const Range<Alpha>& r, Range<Alpha>& result ) const
{
  assert( IsValid() && r.IsValid() && result.IsEmpty() );

  if( IsEmpty() )
    return;
  result.StartBulkLoad();

  const Interval<Alpha> *thisInterval, *interval;

  int i = 0, j = 0;
  Get( i, thisInterval );

  if( !r.IsEmpty() )
  {
    r.Get( j, interval );

    Alpha *start = NULL, *end = NULL;
    bool lc = false, rc = false;

    while( i < GetNoComponents() && j < r.GetNoComponents() )
    {
      if( thisInterval->start.Compare( &interval->start ) == 0 &&
          thisInterval->end.Compare( &interval->end ) == 0 )
      {
        if( thisInterval->lc && !interval->lc )
        {
          Interval<Alpha> newInterval( thisInterval->start,
                                       thisInterval->start, true, true );
          result.Add( newInterval );
        }
        if( thisInterval->rc && !interval->rc )
        {
          Interval<Alpha> newInterval( thisInterval->end,
                                       thisInterval->end, true, true );
          result.Add( newInterval );
        }

        if( ++i < GetNoComponents() )
          Get( i, thisInterval );

        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( !thisInterval->Intersects( *interval ) )
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
        else if( thisInterval->start.Compare( &interval->start ) <= 0 )
        {
          Interval<Alpha> newInterval( *thisInterval );
          result.Add( newInterval );
        }

        if( thisInterval->start.Compare( &interval->start ) <= 0 )
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
      else if( thisInterval->Inside( *interval ) )
      {
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( interval->Inside( *thisInterval ) )
      {
        if( interval->start.Compare( &thisInterval->start ) == 0 )
        {
          assert( start == NULL && end == NULL );
          if( thisInterval->lc && !interval->lc )
          {
            Interval<Alpha> newInterval( thisInterval->start,
                                         thisInterval->start, true, true );
            result.Add( newInterval );
          }
          start = interval->end.Clone();
          lc = !interval->rc;
          end = thisInterval->end.Clone();
          rc = thisInterval->rc;

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else if( interval->end.Compare( &thisInterval->end ) == 0 )
        {
          if( start == NULL && end == NULL )
          {
            Interval<Alpha> newInterval( thisInterval->start,
                                         interval->start,
                                         thisInterval->lc, !interval->lc );
            if( newInterval.IsValid() )
              result.Add( newInterval );
          }
          else
          {
            Interval<Alpha> newInterval( *start, interval->start,
                                         lc, !interval->lc );
            if( newInterval.IsValid() )
              result.Add( newInterval );
            delete start; start = NULL;
            delete end; end = NULL;
            lc = false; rc = false;
          }

          if( thisInterval->rc && !interval->rc )
          {
            Interval<Alpha> newInterval( thisInterval->end,
                                         thisInterval->end, true, true );
            result.Add( newInterval );
          }

          if( ++i < GetNoComponents() )
            Get( i, thisInterval );

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else
        {
          assert( thisInterval->start.Compare( &interval->start ) < 0 &&
                  thisInterval->end.Compare( &interval->end ) > 0 );
          if( start == NULL && end == NULL )
          {
            Interval<Alpha> newInterval( thisInterval->start,
                                         interval->start,
                                         thisInterval->lc, !interval->lc );
            if( newInterval.IsValid() )
              result.Add( newInterval );
          }
          else
          {
            assert( end->Compare( &thisInterval->end ) == 0 &&
                    rc == thisInterval->rc );

            Interval<Alpha> newInterval( *start, interval->start,
                                         lc, !interval->lc        );
            if( newInterval.IsValid() )
              result.Add( newInterval );
            delete start; start = NULL;
            delete end; end = NULL;
            lc = rc = false;
          }

          start = interval->end.Clone();
          lc = !interval->rc;
          end = thisInterval->end.Clone();
          rc = thisInterval->rc;

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
      }
      else
      {
        assert( thisInterval->Intersects( *interval ) );

        if( interval->start.Compare( &thisInterval->start ) < 0 )
        {
          assert( start == NULL && end == NULL );

          if( interval->end.Compare( &thisInterval->end ) == 0 )
          {
            if( thisInterval->rc && !interval->rc )
            {
              Interval<Alpha> newInterval( thisInterval->end,
                                           thisInterval->end, true, true );
              result.Add( newInterval );
            }

            if( ++i < GetNoComponents() )
              Get( i, thisInterval );

            if( ++j < r.GetNoComponents() )
              r.Get( j, interval );
          }
          else
          {
            start = interval->end.Clone();
            if( interval->end.Compare( &thisInterval->start ) == 0 )
            {
              lc = thisInterval->lc && !interval->rc;
            }
            else
            {
              lc = !interval->rc;
            }
            end = thisInterval->end.Clone();
            rc = thisInterval->rc;

            if( ++j < r.GetNoComponents() )
              r.Get( j, interval );
          }
        }
        else if( interval->start.Compare( &thisInterval->start ) == 0 )
        {
          assert( start == NULL & end == NULL );

          if( thisInterval->lc && !interval->lc )
          {
            Interval<Alpha> newInterval( thisInterval->start,
                                         thisInterval->start, true, true );
            result.Add( newInterval );
          }

          if( thisInterval->end.Compare( &interval->end ) > 0 )
          {
            start = interval->end.Clone();
            lc = !interval->rc;
            end = thisInterval->end.Clone();
            rc = thisInterval->rc;

            if( ++j < r.GetNoComponents() )
              r.Get( j, interval );
          }
          else
          {
            assert( thisInterval->end.Compare( &interval->end ) < 0 );
            if( ++i < GetNoComponents() )
              Get( i, thisInterval );
          }
        }
        else if( interval->end.Compare( &thisInterval->end ) > 0 )
        {
          if( thisInterval->start.Compare( &interval->start ) == 0 )
          {
            assert( start == NULL && end == NULL );
            cerr << "I think that there is an error here!!!" << endl;
          }
          else
          {
            if( start != NULL && end != NULL )
            {
              if( interval->start.Compare( start ) > 0 ||
                  ( interval->start.Compare( start ) == 0 &&
                    interval->lc && !lc ) )
              {
                delete end;
                end = interval->start.Clone();
                if( interval->start.Compare( &thisInterval->end ) == 0 )
                  rc = thisInterval->rc && !interval->lc;
                else
                  rc = !interval->lc;

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
              Interval<Alpha> newInterval( thisInterval->start,
                                           interval->start,
                                           thisInterval->lc, !interval->lc );
              if( newInterval.IsValid() )
                result.Add( newInterval );
            }
            if( ++i < GetNoComponents() )
              Get( i, thisInterval );
          }
        }
        else
        {
          assert( interval->end.Compare( &thisInterval->end ) == 0 );

          if( interval->start.Compare( &thisInterval->start ) < 0 )
          {
            assert( start == NULL && end == NULL );
            if( thisInterval->rc && !interval->rc )
            {
              Interval<Alpha> newInterval( interval->end,
                                           interval->end, true, true );
              result.Add( newInterval );
            }
          }
          else
          {
            assert( interval->start.Compare( &thisInterval->start ) > 0 );

            if( start != NULL && end != NULL )
            {
              delete end;
              end = interval->start.Clone();
              rc = !interval->lc;

              Interval<Alpha> newInterval( *start, *end, lc, rc );
              if( newInterval.IsValid() )
                result.Add( newInterval );
              delete start; start = NULL;
              delete end; end = NULL;
              lc = rc = false;
            }
            else
            {
              Interval<Alpha> newInterval( thisInterval->start,
                                           interval->start,
                                           thisInterval->lc, !interval->lc );
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
    Interval<Alpha> newInterval( *thisInterval );
    result.Add( newInterval );

    if( ++i < GetNoComponents() )
      Get( i, thisInterval );
  }
  result.EndBulkLoad( false );
}

template <class Alpha>
void Range<Alpha>::Maximum( Alpha& result ) const
{
  assert( IsValid() );

  if( IsEmpty() )
    result.SetDefined( false );
  else
  {
    const Interval<Alpha> *interval;
    Get( GetNoComponents()-1, interval );
    result.CopyFrom( &interval->end );
  }
}

template <class Alpha>
void Range<Alpha>::Minimum( Alpha& result ) const
{
  assert( IsValid() );

  if( IsEmpty() )
    result.SetDefined( false );
  else
  {
    const Interval<Alpha> *interval;
    Get( 0, interval );
    result.CopyFrom( &interval->start );
  }
}

template <class Alpha>
int Range<Alpha>::GetNoComponents() const
{
  return intervals.Size();
}

template <class Alpha>
void Range<Alpha>::RBBox( Range<Alpha>& result ) const
{
  assert( IsValid() );

  result.Clear();
  if( !IsEmpty() )
  {
    Alpha minIntervalMin;
    Alpha maxIntervalMax;

    Minimum( minIntervalMin );
    Maximum( maxIntervalMax );

    Interval<Alpha> interval( minIntervalMin, maxIntervalMax, true, true);
    result.Add(interval);
  }
}


template <class Alpha>
bool Range<Alpha>::IsValid() const
{
  if( canDestroy )
    return false;

  if( !IsOrdered() )
    return false;

  if( IsEmpty() )
    return true;

  bool result = true;
  const Interval<Alpha> *lastInterval, *interval;

  if( GetNoComponents() == 1 )
  {
    Get( 0, interval );
    return( interval->IsValid() );
  }

  for( int i = 1; i < GetNoComponents(); i++ )
  {
    Get( i-1, lastInterval );
    if( !lastInterval->IsValid() )
    {
      result = false;
      break;
    }
    Get( i, interval );
    if( !interval->IsValid() )
    {
      result = false;
      break;
    }
    if( (!lastInterval->Disjoint( *interval )) &&
        (!lastInterval->Adjacent( *interval )) )
    {
      result = false;
      break;
    }
  }

  return result;
}

template <class Alpha>
void Range<Alpha>::Merge( Range<Alpha>& result ) const
{
  assert( IsOrdered() );

  result.Clear();
  result.StartBulkLoad();

  const Interval<Alpha> *ii, *ji;
  int i = 0, j;
  bool jcont = true;

  while( i < GetNoComponents() )
  {
    Get( i, ii );

    j = i + 1;
    jcont = true;

    Interval<Alpha> copyii( *ii );
    while( j < GetNoComponents() && jcont )
    {
      Get( j, ji );
      if( copyii.Adjacent( *ji ) )
      {
        copyii.end = ji->end;
        copyii.rc = ji->rc;
        j++;
      }
      else
        jcont=false;
    }

    result.Add( copyii );
    i = j;
  }

  result.EndBulkLoad( false );
}

/*
4.3 TemporalUnit

4.3.1 Constructors and destructor

*/
template <class Alpha>
bool TemporalUnit<Alpha>::IsValid() const
{
  return timeInterval.IsValid();
}

template <class Alpha>
TemporalUnit<Alpha>&
TemporalUnit<Alpha>::operator=( const TemporalUnit<Alpha>& i )
{
  defined = i.defined;
  if(defined)
    {
      assert( i.timeInterval.IsValid() );
      timeInterval = i.timeInterval;
    }
  return *this;
}

template <class Alpha>
bool TemporalUnit<Alpha>::operator==( const TemporalUnit<Alpha>& i ) const
{
  if(defined && i.defined)
    {
      assert( timeInterval.IsValid() && i.timeInterval.IsValid() );
      return( timeInterval == i.timeInterval);
    }
  return (defined == i.defined);
}

template <class Alpha>
bool TemporalUnit<Alpha>::operator!=( const TemporalUnit<Alpha>& i ) const
{
  return !( *this == i );
}

template <class Alpha>
bool TemporalUnit<Alpha>::R_Disjoint( const TemporalUnit<Alpha>& i ) const
{
  return( timeInterval.R_Disjoint( i.timeInterval ) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::Disjoint( const TemporalUnit<Alpha>& i ) const
{
  assert( IsValid() && i.IsValid() );

  return( R_Disjoint( i ) || i.R_Disjoint( *this ) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::R_Adjacent( const TemporalUnit<Alpha>& i ) const
{
  return( timeInterval.R_Adjacent(i.timeInterval));
}

template <class Alpha>
bool TemporalUnit<Alpha>::TU_Adjacent( const TemporalUnit<Alpha>& i ) const
{
  assert( IsValid() && i.IsValid() );

  return( R_Adjacent( i ) || i.R_Adjacent( *this ) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::Inside( const TemporalUnit<Alpha>& i ) const
{
  assert( IsValid() && i.IsValid() );

  return( timeInterval.Inside(i.timeInterval) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::Contains( const Instant& a ) const
{
  assert( IsValid() && a.IsDefined() );

  return ( timeInterval.Contains(a) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::Intersects( const TemporalUnit<Alpha>& i ) const
{
  assert( IsValid() && i.IsValid() );

  return ( timeInterval.Intersects(i.timeInterval) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::Before( const TemporalUnit<Alpha>& i ) const
{
  assert( IsValid() && i.IsValid() );

  return ( timeInterval.Before(i.timeInterval) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::Before( const Instant& a ) const
{
  assert( IsValid() && a.IsDefined() );

  return ( timeInterval.Before( a ) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::After( const Instant& a ) const
{
  assert( IsValid() && a.IsDefined() );

  return ( timeInterval.After( a ) );
}

/*
4.4 Mapping

4.4.1 Constructors and destructor

*/
template <class Unit, class Alpha>
Mapping<Unit, Alpha>::Mapping( const int n ):
canDestroy( false ),
ordered( true ),
units( n )
{}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::Destroy()
{
  canDestroy = true;
}

template <class Unit, class Alpha>
Mapping<Unit, Alpha>::~Mapping()
{
  if( canDestroy )
    units.Destroy();
}

/*
4.4.2 Member functions

*/
template <class Unit, class Alpha>
bool Mapping<Unit, Alpha>::IsOrdered() const
{
  return ordered;
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::StartBulkLoad()
{
  assert( ordered );
  ordered = false;
}

template <class Unit>
int UnitCompare( const void *a, const void *b )
{
  Unit *unita = new ((void*)a) Unit,
       *unitb = new ((void*)b) Unit;

  if( *unita == *unitb )
    return 0;
  else if( unita->Before( *unitb ) )
    return -1;
  else
    return 1;
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::EndBulkLoad( const bool sort )
{
  assert( !ordered );
  if( sort )
    units.Sort( UnitCompare<Unit> );
  ordered = true;
  assert( IsValid() );
}

template <class Unit, class Alpha>
bool Mapping<Unit, Alpha>::IsEmpty() const
{
  return units.Size() == 0;
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::Get( const int i, const Unit*& unit ) const
{
  units.Get( i, unit );
  assert( unit->IsValid() );
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::Add( const Unit& unit )
{
  assert( unit.IsValid() );
  units.Append( unit );
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::MergeAdd( Unit& unit )
{
  Unit lastunit;
  const Unit *u1transfer;
  int size = units.Size();

  assert( unit.IsValid() );

  if (size > 0) {
      units.Get( size - 1, u1transfer );
      lastunit = *u1transfer;
      assert( unit.IsValid() );

      if (lastunit.EqualValue(unit) &&
      (AlmostEqual(lastunit.timeInterval.end.ToDouble(),
       unit.timeInterval.start.ToDouble())) &&
      (lastunit.timeInterval.rc || unit.timeInterval.lc)) {
          lastunit.timeInterval.end = unit.timeInterval.end;
          lastunit.timeInterval.rc = unit.timeInterval.rc;
          units.Put(size - 1, lastunit);
      }
      else {
          units.Append( unit);
      }
  }
  else {
      units.Append( unit );
  }
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::Clear()
{
  ordered = true;
  units.Clear();
}

/*
4.4.3 Functions to be part of relations

*/
template <class Unit, class Alpha>
inline bool Mapping<Unit, Alpha>::IsDefined() const
{
  return true;
}

template <class Unit, class Alpha>
inline void Mapping<Unit, Alpha>::SetDefined( bool Defined )
{
}

template <class Unit, class Alpha>
inline size_t Mapping<Unit, Alpha>::Sizeof() const
{
  return sizeof( *this );
}

template <class Unit, class Alpha>
inline int Mapping<Unit, Alpha>::Compare( const Attribute *arg ) const
{
   Mapping<Unit,Alpha>* map2 = (Mapping<Unit,Alpha>*) arg;
   size_t size1 = units.Size();
   size_t size2 = map2->units.Size();
   size_t index = 0;
   const Unit* u1;
   const Unit* u2;
   int cmp;
   while( (index < size1) && (index < size2)){
      units.Get(index,u1);
      map2->units.Get(index,u2);
      cmp = u1->Compare(u2);
      if(cmp){ // different units
         return cmp;
      }
      index++;
   }
   // the common entries all equals
   if(size1<size2){
      cmp =  -1;
   } else if(size1>size2){
      cmp = 1;
   } else{
      cmp= 0;
   }
   return cmp;
}

template <class Unit, class Alpha>
inline bool Mapping<Unit, Alpha>::Adjacent( const Attribute *arg ) const
{
  return false;
}

template <class Unit, class Alpha>
inline Mapping<Unit, Alpha>* Mapping<Unit, Alpha>::Clone() const
{
  assert( IsOrdered() );

  Mapping<Unit, Alpha> *result = new Mapping<Unit, Alpha>( GetNoComponents() );

  result->StartBulkLoad();
  const Unit *unit;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    result->Add( *unit );
  }
  result->EndBulkLoad( false );
  return result;
}

template <class Unit, class Alpha>
inline ostream& Mapping<Unit, Alpha>::Print( ostream &os ) const
{
  return os << "Temporal Algebra---Mapping" << endl;
}

template <class Unit, class Alpha>
inline size_t Mapping<Unit, Alpha>::HashValue() const
{
  return 0;
}

template <class Unit, class Alpha>
inline void Mapping<Unit, Alpha>::CopyFrom( const StandardAttribute* right )
{
  const Mapping<Unit, Alpha> *r = (const Mapping<Unit, Alpha>*)right;
  assert( r->IsOrdered() );

  Clear();

  StartBulkLoad();
  const Unit *unit;
  for( int i = 0; i < r->GetNoComponents(); i++ )
  {
    r->Get( i, unit );
    Add( *unit );
  }
  EndBulkLoad( false );
}

template <class Unit, class Alpha>
inline void
Mapping<Unit, Alpha>::Restrict( const vector< pair<int, int> >& intervals )
{
  units.Restrict( intervals );
}

template <class Unit, class Alpha>
inline int Mapping<Unit, Alpha>::NumOfFLOBs() const
{
  return 1;
}

template <class Unit, class Alpha>
inline FLOB *Mapping<Unit, Alpha>::GetFLOB(const int i)
{
  assert( i == 0 );
  return &units;
}

/*
4.4.4 Operator functions

*/
template <class Unit, class Alpha>
bool Mapping<Unit, Alpha>::operator==( const Mapping<Unit, Alpha>& r ) const
{
  assert( IsOrdered() && r.IsOrdered() );

  if( GetNoComponents() != r.GetNoComponents() )
    return false;

  bool result = true;
  const Unit *thisunit, *unit;

  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, thisunit );
    r.Get( i, unit );

    if( *thisunit != *unit )
    {
      result = false;
      break;
    }
  }

  return result;
}

template <class Unit, class Alpha>
bool Mapping<Unit, Alpha>::operator!=( const Mapping<Unit, Alpha>& r ) const
{
  return !( *this == r );
}

template <class Unit, class Alpha>
int Mapping<Unit, Alpha>::GetNoComponents() const
{
  return units.Size();
}

template <class Unit, class Alpha>
bool Mapping<Unit, Alpha>::IsValid() const
{
  if( canDestroy )
    return false;

  if( !IsOrdered() )
    return false;

  if( IsEmpty() )
    return true;

  bool result = true;
  const Unit *lastunit, *unit;

  if( GetNoComponents() == 1 )
  {
    Get( 0, unit );
    return( unit->IsValid() );
  }

  for( int i = 1; i < GetNoComponents(); i++ )
  {
    Get( i-1, lastunit );
    if( !lastunit->IsValid() )
    {
      result = false;
      break;
    }
    Get( i, unit );
    if( !unit->IsValid() )
    {
      result = false;
      break;
    }
    if( (!lastunit->Disjoint( *unit )) && (!lastunit->TU_Adjacent( *unit )) )
    {
      result = false;
      break;
    }
  }

  return result;
}

template <class Unit, class Alpha>
int Mapping<Unit, Alpha>::Position( const Instant& t ) const
{
  assert( IsOrdered() && t.IsDefined() );

  int first = 0, last = units.Size() - 1;
  Instant t1 = t;

  while (first <= last)
  {
    int mid = ( first + last ) / 2;

    if( (mid < 0) || (mid >= units.Size()) )
      return -1;

    const Unit *midUnit;
    units.Get( mid, midUnit );

    if( midUnit->timeInterval.Contains(t1) )
      return mid;
    else  //not contained
      if( ( t1 > midUnit->timeInterval.end ) ||
          ( t1 == midUnit->timeInterval.end ) )
        first = mid + 1;
      else if( ( t1 < midUnit->timeInterval.start ) ||
               ( t1 == midUnit->timeInterval.start ) )
        last = mid - 1;
      else
        return -1; //should never be reached.
    }
    return -1;
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::AtInstant( const Instant& t,
                                      Intime<Alpha>& result ) const
{
  assert( IsOrdered() && t.IsDefined() );

  int pos = Position( t );

  if( pos == -1 )  // not contained in any unit
    result.SetDefined( false );
  else
  {
    const Unit *posUnit;
    units.Get( pos, posUnit );

    result.SetDefined( true );
    posUnit->TemporalFunction( t, result.value );
    result.instant.CopyFrom( &t );
  }
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::AtPeriods( const Periods& periods,
                                      Mapping<Unit, Alpha>& result ) const
{
  assert( IsOrdered() && periods.IsOrdered() );
  result.Clear();

  if( IsEmpty() || periods.IsEmpty() )
    return;

  result.StartBulkLoad();

  const Unit *unit;
  const Interval<Instant> *interval;
  int i = 0, j = 0;
  Get( i, unit );
  periods.Get( j, interval );

  while( 1 )
  {
    if( unit->timeInterval.Before( *interval ) )
    {
      if( ++i == GetNoComponents() )
        break;
      Get( i, unit );
    }
    else if( interval->Before( unit->timeInterval ) )
    {
      if( ++j == periods.GetNoComponents() )
        break;
      periods.Get( j, interval );
    }
    else
    {
      Unit r;
      unit->AtInterval( *interval, r );
      result.Add( r );

      if( interval->end == unit->timeInterval.end )
      {
        if( interval->rc == unit->timeInterval.rc )
        {
          if( ++i == GetNoComponents() )
            break;
          Get( i, unit );
          if( ++j == periods.GetNoComponents() )
            break;
          periods.Get( j, interval );
        }
        else if( interval->rc == true )
        {
          if( ++i == GetNoComponents() )
            break;
          Get( i, unit );
        }
        else
        {
          assert( unit->timeInterval.rc == true );
          if( ++j == periods.GetNoComponents() )
            break;
          periods.Get( j, interval );
        }
      }
      else if( interval->end > unit->timeInterval.end )
      {
        if( ++i == GetNoComponents() )
          break;
        Get( i, unit );
      }
      else
      {
        assert( interval->end < unit->timeInterval.end );
        if( ++j == periods.GetNoComponents() )
          break;
        periods.Get( j, interval );
      }
    }
  }

  result.EndBulkLoad( false );

// VTA - The merge of the result is not implemented yet.
}

template <class Unit, class Alpha>
bool Mapping<Unit, Alpha>::Present( const Instant& t ) const
{
  assert( t.IsDefined() && IsOrdered() );

  int pos = Position(t);

  if( pos == -1 )  //not contained in any unit
    return false;
  return true;
}

template <class Unit, class Alpha>
bool Mapping<Unit, Alpha>::Present( const Periods& t ) const
{
  assert( t.IsOrdered() && IsOrdered() );

  Periods defTime( 0 );
  DefTime( defTime );

  return t.Intersects( defTime );
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::DefTime( Periods& r ) const
{
  assert( IsOrdered() );

  Periods result( GetNoComponents() );

  const Unit *unit;
  result.StartBulkLoad();
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    result.Add( unit->timeInterval );
  }
  result.EndBulkLoad( false );
  result.Merge( r );
}

template <class Unit, class Alpha>
template <class Beta>
bool Mapping<Unit, Alpha>::Passes( const Beta& val ) const
{
  assert( IsOrdered() && val.IsDefined() );

  const Unit *unit;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    if( unit->Passes( val ) )
      return true;
  }
  return false;
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::At( const Alpha& val,
                               Mapping<Unit, Alpha>& result ) const
{
  assert( IsOrdered() && val.IsDefined() );

  result.Clear();

  result.StartBulkLoad();

  const Unit *unit;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    Unit resUnit;
    if( unit->At( val, resUnit ) )
      result.Add( resUnit );
  }

  result.EndBulkLoad( false );

// VTA - The merge of the result is not implemented yet.
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::Initial( Intime<Alpha>& result ) const
{
  assert( IsOrdered() );

  if( IsEmpty() )
    result.SetDefined( false );
  else
  {
    const Unit *unit;
    units.Get( 0, unit );

    result.SetDefined( true );
    unit->TemporalFunction( unit->timeInterval.start, result.value, true );
    result.instant.CopyFrom( &unit->timeInterval.start );
  }
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::Final( Intime<Alpha>& result ) const
{
  assert( IsOrdered() );

  if( IsEmpty() )
    result.SetDefined( false );
  else
  {
    const Unit *unit;
    units.Get( GetNoComponents()-1, unit );

    result.SetDefined( true );
    unit->TemporalFunction( unit->timeInterval.end, result.value, true );
    result.instant.CopyFrom( &unit->timeInterval.end );
  }
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::Resize(size_t n){
   units.Resize(n);
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::ExtendDefTime(Unit u,
                                         Mapping<Unit,Alpha>& result){

   // special case undefined unit -> donst make any changes
   if(!u.IsDefined()){
      result.CopyFrom(this);
      return;
   }

   // make an empty result
   result.Clear();
   if(!IsDefined()){
     result.SetDefined(false);
     return;
   }
   // resize for two additional units (estimated a good choice)
   result.Resize(units.Size()+2);

   Instant CI(u.timeInterval.start); // current instant
   bool    Cc = u.timeInterval.lc;   // current closed
   const Unit* unit;
   result.canDestroy=false;
   int size = units.Size();

   // we have to connect units holding the same value, so
   // we hold a unit and extend it if it is possible

   result.StartBulkLoad();

   for(int i=0; i<size;i++){
      units.Get(i,unit);
      // check whether a gap exits between the currect position in time
      // and this unit
      if(CI<unit->timeInterval.start || // time is before
         (( CI==unit->timeInterval.start && !Cc && !unit->timeInterval.lc))){
         // create a Unit filling the gap
         Interval<Instant> interval(CI,unit->timeInterval.start,
                                    !Cc,!unit->timeInterval.lc);
         Unit gap(interval,u.constValue);
         result.MergeAdd(gap); // append the gap filling unit
      }
      Unit st(*unit);
      result.MergeAdd(st);
      // change the current instant and closed information
      CI = unit->timeInterval.end;
      Cc = unit->timeInterval.rc;
   }
   // fill the gap between the last inserted unit and the end of the given one
   // if one exists
   if( CI<u.timeInterval.end ||
       ((CI==u.timeInterval.end && !Cc && u.timeInterval.rc))){
       Interval<Instant> interval(CI,u.timeInterval.end,!Cc,u.timeInterval.rc);
       Unit gap(interval,u.constValue);
       result.MergeAdd(gap);
   }
   result.EndBulkLoad(false);
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
      const Interval<Alpha> *interval;
      range->Get( i, interval );
      Alpha *start = (Alpha*)&interval->start,
            *end = (Alpha*)&interval->end;
      intervalList = nl->FourElemList(
              OutFun( nl->TheEmptyList(), SetWord(start) ),
              OutFun( nl->TheEmptyList(), SetWord(end) ),
              nl->BoolAtom( interval->lc ),
              nl->BoolAtom( interval->rc));
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
template <class Alpha, Word (*InFun)( const ListExpr, const ListExpr,
                                      const int, ListExpr&, bool&     )>
Word InRange( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Range<Alpha>* range = new Range<Alpha>( 0 );
  range->StartBulkLoad();

  ListExpr rest = instance;

  correct = true;

  while( !nl->IsEmpty( rest ) && correct )
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
      Alpha *start = (Alpha *)InFun( nl->TheEmptyList(),
                                     nl->First( first ),
                                     errorPos, errorInfo, correct ).addr;
      if( !correct )
      {
        delete start;
        break;
      }

      Alpha *end = (Alpha *)InFun( nl->TheEmptyList(),
                                   nl->Second( first ),
                                   errorPos, errorInfo, correct ).addr;

      if( !correct )
      {
        delete start;
        delete end;
        break;
      }
      // get closedness parameters
      bool lc = nl->BoolValue( nl->Third( first ) );
      bool rc = nl->BoolValue( nl->Fourth( first ) );

      // check, wether interval is well defined
      Interval<Alpha> interval( *start, *end, lc, rc );
      correct = interval.IsValid();

      delete start;
      delete end;

      if ( !correct )
        break;

      range->Add( interval );
    }
    else
      {
        correct = false;
        break;
      }
  }
  if ( !correct )
  {
    range->Destroy();
    delete range;
    return SetWord( Address(0) );
  }
  range->EndBulkLoad( false ); // Do not sort. We expect sorted input
  correct = range->IsValid();  // Check, wether input was well defined

  return SetWord( range );
}

/*
5.1.3 ~Open~-function

*/
template <class Alpha>
bool OpenRange( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{
  Range<Alpha> *range = (Range<Alpha>*)Attribute::Open( valueRecord, offset,
                                                        typeInfo             );
  value = SetWord( range );
  return true;
}

/*
5.1.4 ~Save~-function

*/
template <class Alpha>
bool SaveRange( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{
  Range<Alpha> *range = (Range<Alpha> *)value.addr;
  Attribute::Save( valueRecord, offset, typeInfo, range );

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
void DeleteRange( const ListExpr typeInfo, Word& w )
{
  ((Range<Alpha> *)w.addr)->Destroy();
  delete (Range<Alpha> *)w.addr;
  w.addr = 0;
}

/*
5.1.7 ~Close~-function

*/
template <class Alpha>
void CloseRange( const ListExpr typeInfo, Word& w )
{
  delete (Range<Alpha> *)w.addr;
  w.addr = 0;
}

/*
5.1.8 ~Clone~-function

*/
template <class Alpha>
Word CloneRange( const ListExpr typeInfo, const Word& w )
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

  if( intime->IsDefined() )
    return nl->TwoElemList(
          OutDateTime( nl->TheEmptyList(), SetWord(&intime->instant) ),
          OutFun( nl->TheEmptyList(), SetWord( &intime->value ) ) );
  else
    return nl->SymbolAtom( "undef" );
}

/*
5.2.2 ~In~-function

*/
template <class Alpha, Word (*InFun)( const ListExpr,
                                      const ListExpr,
                                      const int, ListExpr&, bool& )>
Word InIntime( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{

  if ( nl->IsAtom( instance ) &&
       nl->AtomType( instance ) == SymbolType &&
       nl->SymbolValue( instance ) == "undef" )
  {
    Intime<Alpha> *intime = new Intime<Alpha>;
    intime->SetDefined( false );
    correct = true;
    return SetWord( intime );
  }

  if( !nl->IsAtom( instance ) &&
      nl->ListLength( instance ) == 2 )
  {
    Instant *instant = (Instant *)InInstant( nl->TheEmptyList(),
                                             nl->First( instance ),
                                                         errorPos,
                                                         errorInfo,
                                                         correct ).addr;

    if( correct == false )
    {
      delete instant;
      return SetWord( Address(0) );
    }

    Alpha *value = (Alpha *)InFun( nl->TheEmptyList(),
                                   nl->Second( instance ),
                                   errorPos, errorInfo, correct ).addr;
    if( correct  )
    {
      Intime<Alpha> *intime = new Intime<Alpha>( *instant, *value );
      delete value;
      return SetWord( intime );
    }
    delete value;
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
void DeleteIntime( const ListExpr typeInfo, Word& w )
{
  delete (Intime<Alpha> *)w.addr;
  w.addr = 0;
}

/*
5.2.5 ~Close~-function

*/
template <class Alpha>
void CloseIntime( const ListExpr typeInfo, Word& w )
{
  delete (Intime<Alpha> *)w.addr;
  w.addr = 0;
}

/*
5.2.6 ~Clone~-function

*/
template <class Alpha>
Word CloneIntime( const ListExpr typeInfo, const Word& w )
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
  ConstTemporalUnit<Alpha>* constunit = (ConstTemporalUnit<Alpha>*)(value.addr);

  //2.test for undefined value
  if ( !constunit->IsDefined() )
    return (nl->SymbolAtom("undef"));

  //3.get the time interval NL
  ListExpr intervalList = nl->FourElemList(
    OutDateTime( nl->TheEmptyList(), SetWord(&constunit->timeInterval.start) ),
    OutDateTime( nl->TheEmptyList(), SetWord(&constunit->timeInterval.end) ),
    nl->BoolAtom( constunit->timeInterval.lc ),
    nl->BoolAtom( constunit->timeInterval.rc));

  //4. return the final result
  return nl->TwoElemList( intervalList,
                          OutFun( nl->TheEmptyList(),
                                  SetWord( &constunit->constValue ) ) );
}

/*
5.3.2 ~In~-function

*/
template <class Alpha, Word (*InFun)( const ListExpr, const ListExpr,
                                      const int, ListExpr&, bool&     )>
Word InConstTemporalUnit( const ListExpr typeInfo,
                          const ListExpr instance,
                          const int errorPos,
                          ListExpr& errorInfo,
                          bool& correct             )
{
  string errmsg;

  if( nl->ListLength( instance ) == 2 &&
      nl->IsAtom( nl->Second( instance ) ) )
  {
    //1. deal with the time interval
    ListExpr first = nl->First( instance );

    if( nl->ListLength( first ) == 4 &&
              nl->IsAtom( nl->Third( first ) ) &&
        nl->AtomType( nl->Third( first ) ) == BoolType &&
        nl->IsAtom( nl->Fourth( first ) ) &&
        nl->AtomType( nl->Fourth( first ) ) == BoolType )
    {
      Instant *start =
        (Instant *)InInstant( nl->TheEmptyList(), nl->First( first ),
                              errorPos, errorInfo, correct ).addr;
      if( !correct )
      {
        errmsg = "InConstTemporalUnit(): Error in first instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end =
        (Instant *)InInstant( nl->TheEmptyList(), nl->Second( first ),
                              errorPos, errorInfo, correct ).addr;
      if( !correct )
      {
        errmsg = "InConstTemporalUnit(): Error in second instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        delete end;
        return SetWord( Address(0) );
      }
      // get closedness parameters
      bool lc = nl->BoolValue( nl->Third( first ) );
      bool rc = nl->BoolValue( nl->Fourth( first ) );

      Interval<Instant> tinterval( *start, *end, lc, rc );

      delete start;
      delete end;

      // check, wether interval is well defined
      correct = tinterval.IsValid();
      if ( !correct )
        {
          errmsg = "InConstTemporalUnit(): Non valid time interval.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
        }

      //2. deal with the alpha value
      Alpha *value = (Alpha *)InFun( nl->TheEmptyList(), nl->Second( instance ),
                                     errorPos, errorInfo, correct ).addr;

      //3. create the class object
      if( correct  )
      {
        ConstTemporalUnit<Alpha> *constunit =
          new ConstTemporalUnit<Alpha>( tinterval, *value );

        if( constunit->IsValid() )
        {
          delete value;
          return SetWord( constunit );
        }
        delete constunit;
      }
      delete value;
    }
  }
  else if ( nl->IsAtom( instance ) &&
            nl->AtomType( instance ) == SymbolType &&
            nl->SymbolValue( instance ) == "undef" )
    {
      ConstTemporalUnit<Alpha> *constunit =
        new ConstTemporalUnit<Alpha>();
      constunit->SetDefined(false);
      constunit->timeInterval=
        Interval<DateTime>(DateTime(instanttype),
                           DateTime(instanttype),true,true);
      correct = true;
      return (SetWord( constunit ));
    }
  errmsg = "InConstTemporalUnit(): Error in representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
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
void DeleteConstTemporalUnit( const ListExpr typeInfo, Word& w )
{
  delete (ConstTemporalUnit<Alpha> *)w.addr;
  w.addr = 0;
}

/*
5.3.5 ~Close~-function

*/
template <class Alpha>
void CloseConstTemporalUnit( const ListExpr typeInfo, Word& w )
{
  delete (ConstTemporalUnit<Alpha> *)w.addr;
  w.addr = 0;
}

/*
5.3.6 ~Clone~-function

*/
template <class Alpha>
Word CloneConstTemporalUnit( const ListExpr typeInfo, const Word& w )
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
5.4 Type Constructor ~moving~

5.4.1 ~Out~-function

*/
template <class Mapping, class Unit,
          ListExpr (*OutUnit)( ListExpr, Word )>
ListExpr OutMapping( ListExpr typeInfo, Word value )
{
  Mapping* m = (Mapping*)(value.addr);

  if( m->IsEmpty() )
    return (nl->TheEmptyList());
  else
  {
    assert( m->IsOrdered() );
    ListExpr l = nl->TheEmptyList(),
             lastElem, unitList;

    for( int i = 0; i < m->GetNoComponents(); i++ )
    {
      const Unit *unit;
      m->Get( i, unit );
      Unit *aux = (Unit*)unit;
      unitList = OutUnit( nl->TheEmptyList(), SetWord(aux) );
      if( l == nl->TheEmptyList() )
      {
        l = nl->Cons( unitList, nl->TheEmptyList() );
        lastElem = l;
      }
      else
        lastElem = nl->Append( lastElem, unitList );
    }
    return l;
  }
}

/*
5.6.2 ~In~-function

*/
template <class Mapping, class Unit,
          Word (*InUnit)( const ListExpr, const ListExpr,
                          const int, ListExpr&, bool& )>
Word InMapping( const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct )
{

  int numUnits = nl->ListLength(instance);
  Mapping* m = new Mapping( numUnits );
  correct = true;
  int unitcounter = 0;
  string errmsg;

  m->StartBulkLoad();

  ListExpr rest = instance;
  while( !nl->IsEmpty( rest ) )
  {
    ListExpr first = nl->First( rest );
    rest = nl->Rest( rest );

    Unit *unit = (Unit*)InUnit( nl->TheEmptyList(), first,
                                errorPos, errorInfo, correct ).addr;

    if( correct && (!unit->IsDefined() || !unit->IsValid() ) )
    {
      errmsg = "InMapping(): Unit " + int2string(unitcounter) + " is undef.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      correct = false;
      delete unit;
      delete m;
      return SetWord( Address(0) );
    }
    if ( !correct )
    {
      errmsg = "InMapping(): Representation of Unit "
                + int2string(unitcounter) + " is wrong.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      m->Destroy();
      delete m;
      return SetWord( Address(0) );
    }
    m->Add( *unit );
    unitcounter++;
    delete unit;
  }

  m->EndBulkLoad( true ); // if this succeeds, all is OK

  return SetWord( m );
}

/*
5.6.3 ~Open~-function

*/
template <class Mapping>
bool OpenMapping( SmiRecord& valueRecord,
                  const ListExpr typeInfo, Word& value )
{
  Mapping *m = new Mapping( 0 );
  m->Open( valueRecord, typeInfo );
  value = SetWord( m );
  return true;
}

/*
5.6.4 ~Save~-function

*/
template <class Mapping>
bool SaveMapping( SmiRecord& valueRecord,
                  const ListExpr typeInfo, Word& value )
{
  Mapping *m = (Mapping *)value.addr;
  m->Save( valueRecord, typeInfo );
  return true;
}

/*
5.6.5 ~Create~-function

*/
template <class Mapping>
Word CreateMapping( const ListExpr typeInfo )
{
  return (SetWord( new Mapping( 0 ) ));
}

/*
5.6.6 ~Delete~-function

*/
template <class Mapping>
void DeleteMapping( const ListExpr typeInfo, Word& w )
{
  ((Mapping *)w.addr)->Destroy();
  delete (Mapping *)w.addr;
  w.addr = 0;
}

/*
5.6.7 ~Close~-function

*/
template <class Mapping>
void CloseMapping( const ListExpr typeInfo, Word& w )
{
  delete (Mapping *)w.addr;
  w.addr = 0;
}

/*
5.6.8 ~Clone~-function

*/
template <class Mapping>
Word CloneMapping( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((Mapping *)w.addr)->Clone() );
}

/*
5.6.9 ~Sizeof~-function

*/
template <class Mapping>
int SizeOfMapping()
{
  return sizeof(Mapping);
}

/*
5.6.10 ~Cast~-function

*/
template <class Mapping>
void* CastMapping(void* addr)
{
  return new (addr) Mapping;
}

/*
6 Operators value mapping template functions

6.1 Value mapping functions of operator ~isempty~

*/
template <class Mapping>
int MappingIsEmpty( Word* args, Word& result,
                    int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Mapping*)args[0].addr)->IsEmpty() )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

template <class Unit>
int UnitIsEmpty( Word* args, Word& result,
                 int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( !((Unit*)args[0].addr)->IsDefined() )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

/*
6.2 Value mapping functions of operator $=$ (~equal~)

*/
template <class Mapping>
int MappingEqual( Word* args, Word& result,
                  int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( *((Mapping*)args[0].addr) == *((Mapping*)args[1].addr) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

/*
6.3 Value mapping functions of operator $\#$ (~not equal~)

*/
template <class Mapping>
int MappingNotEqual( Word* args, Word& result,
                     int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( *((Mapping*)args[0].addr) != *((Mapping*)args[1].addr) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

/*
6.4 Value mapping functions of operator ~inst~

*/
template <class Alpha>
int IntimeInst( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Intime<Alpha>* i = (Intime<Alpha>*)args[0].addr;

  if( i->IsDefined() )
    ((Instant*)result.addr)->CopyFrom(
                               &((Intime<Alpha>*)args[0].addr)->instant );
  else
    ((Instant*)result.addr)->SetDefined( false );

  return 0;
}

/*
6.5 Value mapping functions of operator ~val~

*/
template <class Alpha>
int IntimeVal( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Intime<Alpha>* i = (Intime<Alpha>*)args[0].addr;

  if( i->IsDefined() )
    ((Alpha*)result.addr)->CopyFrom( &((Intime<Alpha>*)args[0].addr)->value );
  else
    ((Alpha*)result.addr)->SetDefined( false );

  return 0;
}

/*
6.6 Value mapping functions of operator ~no\_components~

*/
template <class Mapping, class Alpha>
int MappingNoComponents( Word* args, Word& result,
                         int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcInt*)result.addr)->Set( true,
                              ((Mapping*)args[0].addr)->GetNoComponents() );
  return 0;
}

/*
6.7 Value mapping functions of operator ~atinstant~

*/
template <class Mapping, class Alpha>
int MappingAtInstant( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Intime<Alpha>* pResult = (Intime<Alpha>*)result.addr;

  ((Mapping*)args[0].addr)->AtInstant( *((Instant*)args[1].addr), *pResult );

  return 0;
}

/*
6.8 Value mapping functions of operator ~atperiods~

*/
template <class Mapping>
int MappingAtPeriods( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Mapping*)args[0].addr)->AtPeriods( *((Periods*)args[1].addr),
                                       *((Mapping*)result.addr) );
  return 0;
}

/*
6.9 Value mapping functions of operator ~deftime~

*/
template <class Mapping>
int MappingDefTime( Word* args, Word& result,
                    int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Mapping*)args[0].addr)->DefTime( *(Periods*)result.addr );
  return 0;
}

/*
6.10 Value mapping functions of operator ~present~

*/
template <class Mapping>
int MappingPresent_i( Word* args, Word& result,
                      int message, Word& local, Supplier s )
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
int MappingPresent_p( Word* args, Word& result,
                      int message, Word& local, Supplier s )
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
6.11 Value mapping functions of operator ~passes~

*/
template <class Mapping, class Alpha, class Beta>
int MappingPasses( Word* args, Word& result,
                   int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Mapping *m = ((Mapping*)args[0].addr);
  Beta* val = ((Beta*)args[1].addr);

  if( !val->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( m->Passes( *val ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

/*
6.12 Value mapping functions of operator ~initial~

*/
template <class Mapping, class Unit, class Alpha>
int MappingInitial( Word* args, Word& result,
                    int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Mapping*)args[0].addr)->Initial( *((Intime<Alpha>*)result.addr) );
  return 0;
}

/*
6.13 Value mapping functions of operator ~final~

*/
template <class Mapping, class Unit, class Alpha>
int MappingFinal( Word* args, Word& result,
                  int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Mapping*)args[0].addr)->Final( *((Intime<Alpha>*)result.addr) );
  return 0;
}
/*
6.14 Value mapping functions of operator ~at~

*/
template <class Mapping, class Unit, class Alpha>
int MappingAt( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Mapping *m = ((Mapping*)args[0].addr);
  Alpha* val = ((Alpha*)args[1].addr);
  Mapping* pResult = ((Mapping*)result.addr);

  pResult->Clear();
  m->At( *val, *pResult );

  return 0;
}

/*
6.15 Value mapping functions of operator ~units~

*/
struct UnitsLocalInfo
{
  Word mWord;     // the address of the moving point/int/real value
  int unitIndex;  // current item index
};

template <class Mapping, class Unit>
int MappingUnits(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Mapping* m;
  const Unit* unit;
  UnitsLocalInfo *localinfo;

  switch( message )
  {
    case OPEN:

      localinfo = new UnitsLocalInfo;
      localinfo->mWord = args[0];
      localinfo->unitIndex = 0;
      local = SetWord(localinfo);
      return 0;

    case REQUEST:

      if( local.addr == 0 )
        return CANCEL;
      localinfo = (UnitsLocalInfo *) local.addr;
      m = (Mapping*)localinfo->mWord.addr;
      if( (0 <= localinfo->unitIndex)
          && (localinfo->unitIndex < m->GetNoComponents()) )
      {
        m->Get( localinfo->unitIndex++, unit );
        Unit *aux = new Unit( *unit );
        result = SetWord( aux );
        return YIELD;
      }
      return CANCEL;

    case CLOSE:

      if( local.addr != 0 )
        delete (UnitsLocalInfo *)local.addr;
      return 0;
  }
  /* should not happen */
  return -1;
}


/*
7.0 Refinement Partition

*/

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
class RefinementPartition {
private:
/*
Private attributes:

  * ~iv~: Array (vector) of sub-intervals, which has been calculated from the
    unit intervals of the ~Mapping~ instances.

  * ~vur~: Maps intervals in ~iv~ to indices of original units in first
    ~Mapping~ instance. A $-1$ values indicates that interval in ~iv~ is no
    sub-interval of any unit interval in first ~Mapping~ instance.

  * ~vup~: Same as ~vur~ for second mapping instance.

*/
    vector< Interval<Instant>* > iv;
    vector<int> vur;
    vector<int> vup;

/*
~AddUnit()~ is a small helper method to create a new interval from
~start~ and ~end~ instant and ~lc~ and ~rc~ flags and to add these to the
~iv~, ~vur~ and ~vup~ vectors.

*/
    void AddUnits(const int urPos,
                  const int upPos,
                  const Instant& start,
                  const Instant& end,
                  const bool lc,
                  const bool rc);

public:
/*
The constructor creates the refinement partition from the two ~Mapping~
instances ~mr~ and ~mp~.

Runtime is $O(\max(n, m))$ with $n$ and $m$ the numbers of units in
~mr~ and ~mp~.

*/
    RefinementPartition(Mapping1& mr, Mapping2& mp);

/*
Since the elements of ~iv~ point to dynamically allocated objects, we need
a destructor.

*/
    ~RefinementPartition();

/*
Return the number of intervals in the refinement partition.

*/
    unsigned int Size(void);

/*
Return the interval and indices in original units of position $pos$ in
the refinement partition in the referenced variables ~civ~, ~ur~ and
~up~. Remember that ~ur~ or ~up~ may be $-1$ if interval is no sub-interval
of unit intervals in the respective ~Mapping~ instance.

Runtime is $O(1)$.

You can use ~void TemporalUnit<Alpha>::AtInterval(const Interval<Instant>
&i, TemporalUnit<Alpha> &result)~ to access the broken-down units

*/
    void Get(unsigned int pos,
             Interval<Instant>*& civ,
             int& ur,
             int& up);
};


template<class Mapping1, class Mapping2, class Unit1, class Unit2>
unsigned int RefinementPartition<Mapping1, Mapping2, Unit1, Unit2>
::Size(void) {

        return iv.size();
    }

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
void RefinementPartition<Mapping1, Mapping2, Unit1, Unit2>
::Get(unsigned int pos, Interval<Instant>*& civ, int& ur,
     int& up) {

        assert(pos < iv.size());

        civ = iv[pos];
        ur = vur[pos];
        up = vup[pos];
    }

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
void RefinementPartition<Mapping1, Mapping2, Unit1,
 Unit2>::AddUnits(
    const int urPos,
    const int upPos,
    const Instant& start,
    const Instant& end,
    const bool lc,
    const bool rc) {
    if(REFINEMENT_DEBUG){
      cout<<"RP::AddUnits()                ["<<start.ToString()<<" "
      <<end.ToString()<<" "<<lc<<" "<<rc<<"] "<<urPos<<" "<<upPos<<endl;
    }

    Interval<Instant>* civ = new Interval<Instant>(start, end,
     lc, rc);

    iv.push_back(civ);
    vur.push_back(urPos);
    vup.push_back(upPos);
}

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
RefinementPartition<Mapping1, Mapping2, Unit1,
 Unit2>::RefinementPartition(
    Mapping1& mr,
    Mapping2& mp) {
    if(REFINEMENT_DEBUG)
      cout << "RP::RP() called" << endl;

    int mrUnit = 0;
    int mpUnit = 0;

    const Unit1 *ur;
    const Unit2 *up;

    Instant t, rpstart, rpend, test;
    bool c, rplc, rprc, tc;
    int before = 0;
    int addu = 0;
    int subu = 0;

    if(mr.GetNoComponents() > 0 && mp.GetNoComponents() > 0){
       mr.Get(0, ur);
       mp.Get(0, up);

       if (ur->timeInterval.start < up->timeInterval.start) {
          test = ur->timeInterval.start;
          tc = !ur->timeInterval.lc;
       }
       else if(ur->timeInterval.start == up->timeInterval.start) {
          test = ur->timeInterval.start;
          tc = !(ur->timeInterval.lc || up->timeInterval.lc);
       }
       else {
          test = up->timeInterval.start;
          tc = !up->timeInterval.lc;
       }
    }
    else if(mr.GetNoComponents() > 0){
       mr.Get(0, ur);
       test = ur->timeInterval.start;
       tc = !ur->timeInterval.lc;
       t = ur->timeInterval.start;
       c = !ur->timeInterval.lc;
    }
    else if(mp.GetNoComponents() > 0){
       mp.Get(0, up);
       test = up->timeInterval.start;
       tc = !up->timeInterval.lc;
       t = up->timeInterval.start;
       c = !up->timeInterval.lc;
    }
    while (mrUnit < mr.GetNoComponents()
       && mpUnit < mp.GetNoComponents()) {

        t = test;
        c = tc;

        if(REFINEMENT_DEBUG){
          cout<<"RP::RP() mrUnit= "<<mrUnit<<" mpUnit= "<<mpUnit
          <<" t "<<t.ToString()<<" c "<<c<<endl;
          cout<<"RP::RP() mpUnit interval=["<<up->timeInterval.start.ToString()
          <<" "<<up->timeInterval.end.ToString()<<" "<<up->timeInterval.lc
          <<" "<<up->timeInterval.rc<<"]"<<endl;
          cout<<"RP::RP() mrUnit interval=["<<ur->timeInterval.start.ToString()
          <<" "<<ur->timeInterval.end.ToString()<<" "<<ur->timeInterval.lc
          <<" "<<ur->timeInterval.rc<< "]"<< endl;}

        addu = 0;
        subu = 0;
        if (t == up->timeInterval.end
        && (up->timeInterval.rc == c)) {
          if(REFINEMENT_DEBUG)
            cout<<"up ends now"<<endl;
          rpend = t;
          rprc = up->timeInterval.rc;
          subu -= 1;
          if (++mpUnit < mp.GetNoComponents()) {
            mp.Get(mpUnit, up);
          }
        }
        if (t == ur->timeInterval.end
        && (ur->timeInterval.rc == c)) {
          if(REFINEMENT_DEBUG)
            cout<<"ur ends now"<<endl;
          rpend = t;
          rprc = ur->timeInterval.rc;
          subu -= 2;
          if (++mrUnit < mr.GetNoComponents()) {
            mr.Get(mrUnit, ur);
          }
        }

        if (t == up->timeInterval.start
        && (up->timeInterval.lc != c)
        && mpUnit < mp.GetNoComponents()) {
          if(REFINEMENT_DEBUG)
            cout<<"up starts now"<<endl;
          addu += 1;
        }
        if (t == ur->timeInterval.start
        && (ur->timeInterval.lc != c)
        && mrUnit < mr.GetNoComponents()) {
          if(REFINEMENT_DEBUG)
            cout<<"ur starts now"<<endl;
          addu += 2;
        }
        if(REFINEMENT_DEBUG)
          cout<<"before "<<before<<" subu "<<subu<<" addu "<<addu<<endl;
        if (before == 3 && subu == -3)
          AddUnits(mrUnit-1, mpUnit-1, rpstart, rpend,
           rplc, rprc);
        else if (before == 3 && subu == -2)
          AddUnits(mrUnit-1, mpUnit, rpstart, rpend, rplc, rprc);
        else if (before == 3 && subu == -1)
          AddUnits(mrUnit, mpUnit-1, rpstart, rpend, rplc, rprc);
        else if (before == 2 && subu == -2)
          AddUnits(mrUnit-1, -1, rpstart, rpend, rplc, rprc);
        else if (before == 1 && subu == -1)
          AddUnits(-1, mpUnit-1, rpstart, rpend, rplc, rprc);
        else if (before == 1 && addu == 2) {
          AddUnits(-1, mpUnit, rpstart, t,
          rplc, c);
        }
        else if (before == 2 && addu == 1) {
          AddUnits(mrUnit, -1, rpstart, t,
          rplc, c);
        }
        rpstart = t;
        rplc = !c;

        before += addu;
        before += subu;

        test = t;
        tc = c;
        if (up->timeInterval.start > t
         || ((up->timeInterval.start == t) && ( !c && !up->timeInterval.lc))){
          test = up->timeInterval.start;
          tc = !up->timeInterval.lc;
        }
        if (ur->timeInterval.start > t
         || ((ur->timeInterval.start == t) && ( !c && !ur->timeInterval.lc))){
          if (((test == t) && (c == tc)) || (ur->timeInterval.start < test
           || ((ur->timeInterval.start == test)
           && (tc && ur->timeInterval.lc )))){
            test = ur->timeInterval.start;
            tc = !ur->timeInterval.lc;
          }
        }
        if (up->timeInterval.end > t
         || ((up->timeInterval.end == t) && (!c && up->timeInterval.rc))) {
          if (((test == t) && (c == tc)) || (up->timeInterval.end < test
           || ((up->timeInterval.end == test)
           && (tc && !up->timeInterval.rc )))){
            test = up->timeInterval.end;
            tc = up->timeInterval.rc;;
          }
        }
        if (ur->timeInterval.end > t
         || ((ur->timeInterval.end == t) && (!c && ur->timeInterval.rc))) {
          if (((test == t) && (tc == c)) || (ur->timeInterval.end < test
           || ((ur->timeInterval.end == test)
           && (tc && !ur->timeInterval.rc)))){
            test = ur->timeInterval.end;
            tc = ur->timeInterval.rc;
          }
        }
        if(REFINEMENT_DEBUG)
          cout<<"next t "<<test.ToString()<<" c "<<tc<<endl;
    }//while
    if (mrUnit < mr.GetNoComponents()) {
        if (t < ur->timeInterval.end){
            if(REFINEMENT_DEBUG)
              cout<<"Add rest of ur"<<endl;
            AddUnits(mrUnit, -1, t, ur->timeInterval.end,
                 c, ur->timeInterval.rc);
            }
        mrUnit++;

        while (mrUnit < mr.GetNoComponents()) {
            mr.Get(mrUnit, ur);

            if(REFINEMENT_DEBUG)
              cout<<"Add all solo units of r"<<endl;
            AddUnits(mrUnit, -1, ur->timeInterval.start, ur->timeInterval.end,
                ur->timeInterval.lc, ur->timeInterval.rc);
            mrUnit++;
        }
    }
    if (mpUnit < mp.GetNoComponents()) {
        if (t < up->timeInterval.end){
            if(REFINEMENT_DEBUG)
              cout<<"Add rest of up"<<endl;
            AddUnits(-1, mpUnit, t, up->timeInterval.end,
                c, up->timeInterval.rc);
            }
        mpUnit++;

        while (mpUnit < mp.GetNoComponents()) {
            mp.Get(mpUnit, up);

            if(REFINEMENT_DEBUG)
              cout<<"Add all solo units of p"<<endl;
            AddUnits(-1, mpUnit, up->timeInterval.start, up->timeInterval.end,
                up->timeInterval.lc, up->timeInterval.rc);
            mpUnit++;
        }
    }
    if(REFINEMENT_DEBUG){
      Interval<Instant> *ivtest;
      cout<<"Complete RefinementPartition"<<endl;
      for(unsigned int m = 0; m < iv.size(); m++){
        ivtest = iv[m];
        cout<<"["<<ivtest->start.ToString()<<" "<<ivtest->end.ToString()
        <<" "<<ivtest->lc<<" "<<ivtest->rc<<"] "<<vur[m]<<" "<<vup[m]<<endl;
      }
    }
}

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
RefinementPartition<Mapping1, Mapping2, Unit1,
 Unit2>::~RefinementPartition() {

    if(REFINEMENT_DEBUG)
      cout << "RP::~RP() called" << endl;

    for (unsigned int i = 0; i < iv.size(); i++) delete iv[i];
}


#endif // _TEMPORAL_ALGEBRA_H_
