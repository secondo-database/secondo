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

Dec 2006 Christian D[ue]ntgen: Moved RefimentPartition from MovingRegionAlgebra
here and replaced the error-prone implementation in TemporalLiftedAlgbra by
this version.

29.09.2009 Mahmoud Sakr: Added the operators: delay, distancetraversed, and
mint2mbool

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
#include <ostream>
#include <sstream>
#include <string>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "NestedList.h"
//#include "DBArray.h"
#include "../../Tools/Flob/DbArray.h"
#include "Progress.h"
#include "CellGrid.h"


#include "RectangleAlgebra.h"
#include "DateTime.h"
#include "AlmostEqual.h"
#include "Geoid.h"
#include "ListUtils.h"
#include "../../include/CharTransform.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace datetime;


class SecInterval;



//#define REF_DEBUG(msg) cout << msg << endl;
#define REF_DEBUG(msg)

string int2string(const int& number);

/*
3 C++ Classes (Defintion)

3.1 Instant

This class represents a time instant, or a point in time. It will be
used in the ~instant~ type constructor.

*/
// the following typedef has been moved to file DateTime.h:
// typedef DateTime Instant;

/*
3.2 Interval

The class ~Interval~ implements the closure of an $\alpha$-interval. To be a generic
class, this class uses templates with parameter ~Alpha~. An interval contains a
~start~, an ~end~ and two flags ~lc~ and ~rc~ indicating if the interval is
left-closed and right-closed (or left-right-closed), respectively.

*/
template <class Alpha>
class Interval
{

  public:
/*
3.2.1 Constructors

*/

  Interval() {}
/*
The simple constructor. This constructor should not be used.

*/
  explicit Interval(bool dummy):
    start(instanttype), end(instanttype), lc(true), rc(true){}


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
  const Interval<Instant>& getTimeInterval() const {
     return  *this;
  }

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

  bool operator<(const Interval<Alpha>& i) const;
  bool operator>(const Interval<Alpha>& i) const;

/*
Returns always true, because it is not an attribute data type.
Required e.g., for RefinementStream for Periods values.

*/
  bool IsDefined() const{
    return true;
  }


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

  bool Contains( const Alpha& a,
                 const bool ignoreCloseness = false ) const;
/*
Returns ~true~ if this interval contains the value ~a~ and ~false~ otherwise.
If ignoreCloseness is set to be true, the return value will also be true, if
a is one of the borders of this interval regardless of the closeness at this
point.

*Precondition:* ~a.IsDefined()~

*/
  bool Contains(const Interval<Alpha>& i,
                const bool ignoreCloseness = false) const;



  bool Intersects( const Interval<Alpha>& i ) const;
/*
Returns ~true~ if this interval intersects with the interval ~i~ and ~false~ otherwise.

*/

  bool StartsBefore( const Interval<Alpha>& i ) const;

/*
Returns ~true~ iff this interval starts earlier than interval ~i~

*/

  bool EndsAfter( const Interval<Alpha>& i ) const;

/*
Returns ~true~ iff this interval ends after interval ~i~

*/

  bool Before( const Interval<Alpha>& i ) const;
/*
Returns ~true~ if this interval is before the interval ~i~ and ~false~ otherwise.

*/

  bool Before( const Alpha& a ) const;

  bool After( const Alpha& a ) const;
  bool After( const Interval<Alpha>& iv ) const;
/*
Returns ~true~ if this interval is before/after the value ~a~ and ~false~ otherwise.

*/

  void Intersection( const Interval<Alpha>& i, Interval<Alpha>& result ) const;
  void IntersectionWith( const Interval<Alpha>& i);
/*
Return the intersection of this interval and ~i~ into ~result~.

*/

  void Union(const Interval<Alpha>& iv, Interval<Alpha> result) const;
/*
Constructs the mininum interval containing both, this interval and iv.

*/
  void Union(const Interval<Alpha>& iv);
/*
Changes this interval to be the union of this is iv. gaps between the
intervals are included in the result.

*/



int Minus(const Interval<Alpha>& iv,
          Interval<Alpha>& res1,
          Interval<Alpha>& res2);
/*
Remove iv from this interval. The result(s) are stored in res1 and res2.
The number of results is returned.

*/


  int CompareTo(const Interval<Alpha>& i) const;
/*
Compares this and the argument;

*/

  ostream& Print(ostream& os) const{
    os << (lc?"[":"(");
    start.Print(os) << ", ";
    end.Print(os) << (rc?"]":")");
    return os;
  }



/*
~SplitAround~

Divides this interval into several pieces around the given __value__.
If the value is not included in the interval, the result will only
consists of the original interval itself. If the __value__ is one of the
borders of the interval, the result will hav two entries. One entry consists
of an interval containing only __value__, the other entry is the remaining
interval. If __value__ is located inside the interval, the result will have
three entries.

*/
vector<Interval<Alpha> > splitAround(Alpha value) const{
  vector<Interval<Alpha> > result;
  if( (value<start) || (value > end) ){
     result.push_back(*this);
     return result;
  }
  if((value==start) && !lc){
     result.push_back(*this);
     return result;
  }
  if(value==start){ // lc == true
    Interval<Alpha> i1(start,start,true,true);
    result.push_back(i1);
    if(end>start){ // there is a remaining interval
       Interval<Alpha> i2(start,end,false,rc);
       result.push_back(i2);
    }
    return result;
  }
  if(value==end && !rc){
     result.push_back(*this);
     return result;
  }
  if(value==end){
    if(start<end){
       Interval<Alpha> i1(start,end,lc,false);
       result.push_back(i1);
    }
    Interval<Alpha> i2(end,end,true,true);
    result.push_back(i2);
    return result;
  }
  // value is located inside the interval
  Interval<Alpha> i1(start,value,lc,false);
  Interval<Alpha> i2(value,value,true,true);
  Interval<Alpha> i3(value,end,false,rc);
  result.push_back(i1);
  result.push_back(i2);
  result.push_back(i3);
  return result;
}








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

template<class alpha>
ostream& operator<<(ostream& o, const Interval<alpha>& u);

/*
3.3 Range

The ~Range~ class implements a set of disjoint, non-adjacent $\alpha$-~intervals~.
For this implementation, it is used a database array of ordered intervals.

Since intervals may contain only defined start and end values, a Range cannot
contain undefined values.

*/
template <class Alpha>
class Range : public Attribute
{
  public:
/*
3.3.1 Constructors and Destructor

*/
    Range():Attribute() {}
/*
The simple constructor. This constructor should not be used.

*/

    Range( const int n );
/*
The constructor. Initializes space for ~n~ elements.

*/

    Range(const Range<Alpha>& src);


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
condition only for bulk load of   All other operations assume that the interval set is
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

    void EndBulkLoad( const bool sort = true, const bool checkvalid = false );
/*
Marks the end of a bulk load and sorts the interval set if the flag ~sort~ is set to true.
Checkvalid indicated, whether the validity (non-overlapping intervals) should
be checked. If the mapping is found to be invalid, it is marked undefined

3.3.3 Member functions

*/
    bool IsEmpty() const;
/*
Returns true iff the range is undefined or empty of intervals.

*/

    void Get( const int i, Interval<Alpha>& ai ) const;
/*
Returns the interval ~ai~ at the position ~i~ in the range.

*Precondition:* ~IsDefined() == true~


*/

    void Add( const Interval<Alpha>& i );
/*
Adds an interval ~i~ to the range. We will assume that the only way of adding intervals
is in bulk loads, i.e., in a non-ordered array.

*Precondition:* ~IsOrdered() == false~

*Precondition:* ~IsDefined() == true~


*/

    void Merge( Range<Alpha>& result ) const;
/*
Merges a range into ~result~ concatenating adjacent intervals.


*Precondition:* ~IsDefined() == true~

*/

    void MergeAdd(const Interval<Alpha>& i);
/*

Adds an interval at the end of that Range. If the last interval and the i can be
merged, the last interval is extended by the new one.

*/



    void Clear();
/*
Remove all intervals in the range.

3.3.4 Functions to be part of relations

*/
    inline size_t Sizeof() const;
    inline int Compare( const Attribute* arg ) const;
    inline bool Adjacent( const Attribute* arg ) const;
    inline Range<Alpha>* Clone() const;
    inline ostream& Print( ostream &os ) const;
    inline size_t HashValue() const;
    inline void CopyFrom( const Attribute* right );

    inline int NumOfFLOBs() const;
    inline Flob *GetFLOB(const int i);

/*
3.3.5 Operations

3.3.5.1 Operation $=$ (~equal~)

Is true, iff both arguments are undefined or both are defined and contain the same intervals.

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

*Precondition:* ~IsDefined() == true~

*Precondition:* ~r.IsDefined() == true~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \cap Y \neq \emptyset$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool Intersects( const Range<Alpha>& r ) const;
    bool Intersects( const Interval<Alpha>& iv ) const;

/*
3.3.5.4 Operation ~inside~


*Precondition:* ~IsDefined() == true~

*Precondition:* ~r.IsDefined() == true~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \subseteq Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool Inside( const Range<Alpha>& r ) const;
    bool Inside( const Interval<Alpha>& iv ) const;

/*
3.3.5.4 Operation ~GetIndexOf~

This function returns the index of the interval within this periods value containing
the given Alpha. If the parameter ignoreCloseness is set to be true, the index of
the interval is also returned, iff alpha is a border of the interval regardless
of the closeness of the interval. If no interval exist containing alpha, -1 is returned.

*/
   int GetIndexOf(const Alpha& alpha,const bool ignoreCloseness = false) const;


/*
3.3.5.5 Operation ~contains~

*Precondition:* ~IsDefined() == true~

*Precondition:* ~a.IsDefined() == true~

*Precondition:* ~X.IsOrdered() $\&\&$ y.IsDefined()~

*Semantics:* $y \in X$

*Complexity:* $O(log(n))$, where ~n~ is the size of this range ~X~.

*/
    bool Contains( const Alpha& a ) const;
    bool Contains( const Interval<Alpha>& iv,
                   const bool ignoreCloseness = false ) const;

/*
3.3.5.6 Operation ~before~ (with ~range~)

*Precondition:* ~IsDefined() == true~

*Precondition:* ~r.IsDefined() == true~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $\forall x \in X, \forall y \in Y: x \leq y$

*Complexity:* $O(1)$.

*/
    bool Before( const Range<Alpha>& r ) const;
    bool Before( const Interval<Alpha>& iv ) const;

/*
3.3.5.7 Operation ~before~ (with ~BASE~ type)

*Precondition:* ~IsDefined() == true~

*Precondition:* ~a.IsDefined() == true~

*Precondition:* ~X.IsOrdered() $\&\&$ y.IsDefined()~

*Semantics:* $\forall x \in X: x \leq y$

*Complexity:* $O(1)$.

*/
    bool Before( const Alpha& a ) const;

/*
3.3.5.8 Operation ~after~

This operation works only with ~BASE~ type, because it is an attempt to implement the operation
before on a contrary order, i.e., ~x before Y~.

*Precondition:* ~IsDefined() == true~

*Precondition:* ~a.IsDefined() == true~

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


    void Intersection(const Interval<Alpha>& r, Range<Alpha>& result) const;



/*
3.3.5.10 Operation ~union~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered() $\&\&$ Result.IsEmpty()~

*Semantics:* $X \cup Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    void Union( const Range<Alpha>& r, Range<Alpha>& result ) const;
    void Union( const Interval<Alpha>& iv, Range<Alpha>& result ) const;

/*
3.3.5.11 Operation ~minus~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered() $\&\&$ Result.IsEmpty()~

*Semantics:* $X \backslash Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.


Note, it's not allowed to use ([*]this) as the result of this operation. This will result in
an empty result. To do so, please use the variant without result parameter.


*/
    void Minus( const Range<Alpha>& r, Range<Alpha>& result ) const;

    void Minus( const Interval<Alpha>& r, Range<Alpha>& result ) const;


/*
Changes this Range value to contain the current intervals without the given ones.

*/
   void Minus(const Range<Alpha>& r);

   void Minus( const Interval<Alpha>& r);





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

*Precondition:* ~IsDefined() == true~

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
purposes only. The ~range~ is valid, iff the range is either undefined, or it
is defined and all the following conditions are true:

  1 Each interval is valid

  2 Start of each interval $>=$ end of the interval before

  3 If start of an interval = end of the interval before, then one needs to
    make sure that the interval is not left-closed or the interval before
    is not right-closed

*/

  inline static const string BasicType()
  {
    if(Alpha::BasicType()==DateTime::BasicType()){
      return "periods";
    } else {
      return "r"+Alpha::BasicType();
    }
  }

  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }

/*
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

    DbArray< Interval<Alpha> > intervals;
/*
The intervals database array.

*/
};

/*
3.4 Intime

This class implements the ~intime~ type constructor, which converts a given type
$\alpha$ into a type that associates instants of time with values of $\alpha$.

A defined Intime object may not contain an undefined instant, but an undefined
value.

3.4.1 Constructors

*/
template <class Alpha>
class Intime: public Attribute
{
  public:

  Intime() {}
/*
The simple constructor.

*/

  Intime(const int i):instant((int64_t)i),value(i){}

  Intime( const Instant& _instant, const Alpha& alpha ):
    instant( _instant ), value(false)
  {
    value.CopyFrom( &alpha );
    this->del.isDefined = instant.IsDefined();
  }
/*
The first constructor.

*/

  Intime( const Intime<Alpha>& intime ):
    instant( intime.instant )
  {
    if( intime.IsDefined() ){
      value.CopyFrom( &intime.value );
      this->del.isDefined=true;
    } else {
      this->del.isDefined = false;
    }
  }
/*
The second constructor.

3.4.2 Functions to be part of relations

*/
  size_t Sizeof() const
  {
    return sizeof( *this );
  }

  int Compare( const Attribute* arg ) const
  {
    //    return 0; // Original implementation
    Intime<Alpha>* other = (Intime<Alpha>*) arg;
    if (!IsDefined() && !other->IsDefined())
      return 0;
    if (!IsDefined())
      return -1;
    if (!other->IsDefined())
      return 1;

    int cmp = instant.Compare( &(other->instant) );
    if(cmp)
      return cmp;
    return value.Compare( &(other->value) );
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
    os << Intime<Alpha>::BasicType() << ": (";
    if ( IsDefined() )
    {
      instant.Print(os);
      os << ", ";
      value.Print(os);
    }
    else
    {
      os << " undefined";
    }
    os << " ) ";
    return os;
  }

  size_t HashValue() const
  {
    if(!IsDefined()){
      return 0;
    }
    return static_cast<size_t>(   instant.HashValue()
                                ^ value.HashValue()   ) ;
  }

  void CopyFrom( const Attribute* right )
  {
    const Intime<Alpha>* i = (const Intime<Alpha>*)right;

    this->SetDefined(i->IsDefined());
    if( IsDefined() )
    {
      instant.Equalize(&(i->instant));
      value.CopyFrom( &i->value );
    }
  }

  bool operator==(const Intime<Alpha>& other) const
  {
    return (Compare((Attribute*) (&other) ) == 0);
  }

  bool operator!=(const Intime<Alpha>& other) const
  {
    return (Compare((Attribute*) (&other) ) != 0);
  }

  // type name used in Secondo:
  inline static const string BasicType()
  {
    return "i"+Alpha::BasicType();
  }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
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

typedef Intime<CcBool> IBool;
typedef Intime<CcInt> IInt;
typedef Intime<CcReal> IReal;
typedef Intime<CcString> IString;
typedef Intime<Point> IPoint;


/*
3.5 TemporalUnit

This class will generically implements a temporal unit. It is an abstract class
that enforces each kind of temporal unit to have a function that computes a value
inside the temporal unit given a time instant (also inside the temporal unit).

The TemporalUnit does not yet contain a ~defined~ flag!

*/
template <class Alpha>
class TemporalUnit
{

  public:

/*
3.5.1 Constructors and Destructor

*/
  TemporalUnit() {}
/*
The simple constructor. This constructor should not be used.

*/
  TemporalUnit(bool defined): timeInterval(defined) {}


/*
Use this constructor when declaring temporal object variables etc.

*/

  TemporalUnit( const Interval<Instant>& interval ):
    timeInterval( interval )
    { }
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


  bool StartsBefore( const TemporalUnit<Alpha>& i ) const;
/*
Returns ~true~ if this temporal unit starts before the temporal unit ~i~ and ~false~ otherwise.

*/

  bool EndsAfter( const TemporalUnit<Alpha>& i ) const;
/*
Returns ~true~ if this temporal unit ends after the temporal unit ~i~ and ~false~ otherwise.

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

 inline bool IsInstantUnit() const {
  assert(IsValid());
  return(    timeInterval.lc
          && timeInterval.rc
          && (timeInterval.start == timeInterval.end) );
}
/*
Returns ~true~ iff this unit's timeInterval is a single instant.

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

*Precondition:* IsDefined() == true AND val.IsDefined() == true

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
  }

/*
Returns a unit restricted to the time interval ~i~.
This can be used to create sub-units for refinement partitions.

*/

  virtual bool EqualValue( const TemporalUnit<Alpha>& i ) const
  {
    return false;
  }


/*
Returns ~true~ if the value of this temporal unit is equal to the
value of the temporal unit ~i~ and ~false~ if they are different.
Equality is computed with respect to temporal evolution.

*/

  virtual bool Merge( const TemporalUnit<Alpha>& i )
  {
    return false;
  }

/*
Type name used in Secondo

*/
  inline static const string BasicType()
  {
    return "u"+Alpha::BasicType();
  }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }
/*
Tries to merge the other unit into this unit. Returns ~true~ iff this was
successful (and this unit was modified).

If the units cannot be merged, the result is ~false~ and this unit remains
unchanged.

Merge might work, if the definition time is disjunct or overlapping, but does
not have a temporal gap.

It will fail, if the temporal functions cannot be unified with each other, or
if the unified function is not representable as a single unit.

*/

/*
3.5.3 Attributes

*/
  inline const Interval<Instant>& getTimeInterval() const{
    return timeInterval;
  }



  Interval<Instant> timeInterval;
/*
The time interval of the temporal unit.

*/

};

/*
3.6 StandardTemporalUnit

This class inherits from ~Attribute~ and allows temporal units
of standard types to be part of relations. One should note that it is
still an abstract class, because the functions ~CopyFrom~ and ~Clone~
are not implemented.

This class contains a defined flag.

*/
template<class Alpha>
class StandardTemporalUnit :
  public Attribute,
  public TemporalUnit<Alpha>
{
  public:

    StandardTemporalUnit() {}
/*
The simple constructor. This constructor should not be used.

*/

    StandardTemporalUnit( bool is_defined):Attribute(is_defined),
                                           TemporalUnit<Alpha>(is_defined)
     {}

/*
Use this constructor when declaring temporal object variables etc.

*/

    StandardTemporalUnit( const Interval<Instant>& interval ):
      Attribute(true),
      TemporalUnit<Alpha>( interval )
      {
        del.refs=1;
        del.SetDelete();
        del.isDefined=true;
      }
/*
This constructor sets the time interval of the temporal unit.

*/

    virtual ~StandardTemporalUnit() {}
/*
The destructor.

3.5.2 Member Functions

3.6.4.1 Functions to be part of relations

*/
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
          os << StandardTemporalUnit<Alpha>::BasicType() << ": ( ";
          TemporalUnit<Alpha>::timeInterval.Print(os);
          os << ", NO SPECIFIC Print()-Method for this StandardTemporalUnit! )";
          return os;
        }
      else
        return os << StandardTemporalUnit<Alpha>::BasicType() <<": (undef) ";
    }

    virtual size_t HashValue() const
    {
      if(!IsDefined()){
        return 0;
      }
      return static_cast<size_t>(   this->timeInterval.start.HashValue()
          ^ this->timeInterval.end.HashValue()   ) ;
    }

    virtual StandardTemporalUnit<Alpha>* Clone() const = 0;
    virtual void CopyFrom( const Attribute* right ) = 0;
    virtual size_t Sizeof() const = 0;

    inline static const string BasicType()
    {
      return "u"+Alpha::BasicType();
    }
    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }

};

/*
The output operator:

*/

template<class Alpha>
ostream& operator<<(ostream& o, const StandardTemporalUnit<Alpha> u)
{
  return  u.Print(o);
}



/*
3.7 SpatialTemporalUnit

This class inherits from ~SpatialAttribute~ and allows temporal units
of spatial types to be part of relations. This class is a template also on the
dimensionality. One should note that it is still an abstract class, because
the functions ~CopyFrom~ and ~Clone~
are not implemented.

This class contains a defined flag!

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

    SpatialTemporalUnit( bool is_defined )
      :StandardSpatialAttribute<dim>(is_defined){
      this->del.isDefined=is_defined;
    }

/*
Use this constructor when declaring temporal object variables etc.

*/

    SpatialTemporalUnit( const Interval<Instant>& interval ):
      StandardSpatialAttribute<dim>(true),
      TemporalUnit<Alpha>( interval )
      {
        this->del.isDefined = true;
      }
/*
This constructor sets the time interval of the temporal unit.

*/

    virtual ~SpatialTemporalUnit() {}
/*
The destructor.

3.5.2 Member Functions

3.6.4.1 Functions to be part of relations

*/
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
      if( this->del.isDefined )
        {
          os << SpatialTemporalUnit<Alpha,dim>::BasicType() << ": " << "( ";
          TemporalUnit<Alpha>::timeInterval.Print(os);
          os << ", ";
          // print specific stuff:
          os << " NO SPECIFIC Print()-Method for this SpatioTemporalUnit! ) ";
          return os;
        }
      else
        return os << SpatialTemporalUnit<Alpha,dim>::BasicType() <<": (undef) ";
    }

    virtual size_t HashValue() const
    {
      if(!this->IsDefined()){
        return 0;
      }
      return static_cast<size_t>(   this->timeInterval.start.HashValue()
          ^ this->timeInterval.end.HashValue()   ) ;
    }

    virtual SpatialTemporalUnit<Alpha, dim>* Clone() const = 0;
    virtual void CopyFrom( const Attribute* right ) = 0;
    virtual size_t Sizeof() const = 0;
    virtual const Rectangle<dim> BoundingBox(const Geoid* geoid = 0) const = 0;

    inline static const string BasicType()
    {
      return "u"+Alpha::BasicType();
    }
    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }

    bool Intersects(const Rectangle<3>& rect, const Geoid* geoid=0) const{
     assert(false); // not implemented yet
     return false;
    }


};

/*
The output operator

*/
template<class Alpha, unsigned dim>
ostream& operator<<(ostream& o, const SpatialTemporalUnit<Alpha, dim> u)
{
  return  u.Print(o);
}

/*
3.6 Class ~RInt~

*/
typedef Range<CcInt> RInt;

/*
3.6 Class ~RReal~

*/
typedef Range<CcReal> RReal;

/*
3.6 Class ~RBool~

*/
typedef Range<CcBool> RBool;

/*
3.6 Class ~RString~

*/
typedef Range<CcString> RString;

/*
3.6 Class ~Periods~

*/
class Periods : public Range<Instant> {
  public:
/*
Constructors

*/
  Periods() {}

/*
The constructor. Initializes space for ~n~ elements.

*/
  Periods(const int n) : Range<Instant>(n) {};

  Periods(const Periods& src) : Range<Instant>(src) {};

/*
The ~contains~ functions

*/
  const bool Contains(const SecInterval& si) const;

  const bool Contains(const Periods& per) const;

  inline bool Contains(const Instant& a) const{
    return Range<Instant>::Contains(a);
  }

  inline bool Contains(const Interval<Instant>& iv,
                       const bool ignoreCloseness = false ) const {
    return Range<Instant>::Contains(iv,ignoreCloseness);
  }

};



/*
3.6 ConstTemporalUnit

This class will be used in the ~const~ type constructor. It constructs constant
temporal units, i.e. it has a constant value and the temporal function always
return this value. The explicit purpose of the ~const~ type constructor is to
define temporal units for ~int~, ~string~, and ~bool~, i.e., for types where
their values change only in discrete steps.

This class inherits a defined flag!

*/
template <class Alpha>
class ConstTemporalUnit : public StandardTemporalUnit<Alpha>
{
  public:
/*
3.6.1 Constructors, Destructor

*/
  ConstTemporalUnit() {}

  ConstTemporalUnit(bool is_defined):StandardTemporalUnit<Alpha>(is_defined),
                                     constValue(is_defined)
  { }

  ConstTemporalUnit( const Interval<Instant>& _interval, const Alpha& a ):
    StandardTemporalUnit<Alpha>( _interval )
  {
    this->del.isDefined = true;
    constValue.CopyFrom( &a );
  }

  // the following constructor is for implementation compatibility with
  // UnitTypes for continious value range types (like UReal, UPoint)
  ConstTemporalUnit( const Interval<Instant>& _interval, const Alpha& a,
                                                         const Alpha& b ):
    StandardTemporalUnit<Alpha>( _interval )
  {
    assert(a == b);
    this->del.isDefined = true;
    constValue.CopyFrom( &a );
  }

  ConstTemporalUnit( const ConstTemporalUnit<Alpha>& u ):
    StandardTemporalUnit<Alpha>( u.timeInterval )
  {
    this->del.isDefined = u.del.isDefined;
    constValue.CopyFrom( &u.constValue );
  }

/*
3.6.2 Operator redefinitions

*/

  virtual ConstTemporalUnit<Alpha>&
  operator=( const ConstTemporalUnit<Alpha>& i )
  {
    this->del.isDefined = i.del.isDefined;
    if( !i.IsDefined() ){
      return *this;
    }
    *((TemporalUnit<Alpha>*)this) = *((TemporalUnit<Alpha>*)&i);
    constValue.CopyFrom( &i.constValue );
    return *this;
  }
/*
Redefinition of the copy operator ~=~.

Two ConstTemporalUnits are equal, if both are either undefined, or both are
defined and represent the same temporal function

*/

  virtual bool operator==( const ConstTemporalUnit<Alpha>& i ) const
  {
    if( !this->IsDefined() && !i.IsDefined() ){
      return true;
    }
    return (this->IsDefined()) && (i.IsDefined())
        && *((TemporalUnit<Alpha>*)this) == *((TemporalUnit<Alpha>*)&i)
        && constValue.Compare( &i.constValue ) == 0;
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

~TemporalFunction~ returns an undefined result if the ConstUnit or the Instant
is undefined, or the Instant is not within the unit's timeInterval.

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
    if( this->IsDefined() && (constValue.Compare( &val ) == 0) )
      return true;
    return false;
  }

  virtual bool At( const Alpha& val, TemporalUnit<Alpha>& result ) const
  {
    if( this->IsDefined() && (constValue.Compare( &val ) == 0) )
    {
      ((ConstTemporalUnit<Alpha>*)&result)->CopyFrom( this );
      return true;
    }
    ((ConstTemporalUnit<Alpha>*)&result)->SetDefined( false );
    return false;
  }

  virtual void AtInterval( const Interval<Instant>& i,
                           TemporalUnit<Alpha>& result ) const
  {
    if( !this->IsDefined() || !this->timeInterval.Intersects( i ) ){
      ((ConstTemporalUnit<Alpha>*)&result)->SetDefined( false );
    } else {
      TemporalUnit<Alpha>::AtInterval( i, result );
      ((ConstTemporalUnit<Alpha>*)&result)->constValue.CopyFrom( &constValue );
    }
  }

  virtual bool EqualValue( const ConstTemporalUnit<Alpha>& i ) const
  {
    return this->IsDefined() && (constValue.Compare( &i.constValue ) == 0);
  }
/*
Returns ~true~ if the value of this temporal unit is defined and equal to the
value of the temporal unit ~i~ and ~false~ if they are different.

*/

  virtual bool Merge( const ConstTemporalUnit<Alpha>& i ) {
    if(!this->IsDefined() && !i.IsDefined()) { // mergeable, but nothing to do
      return true;
    } else if(!this->IsDefined() || !i.IsDefined()) { // not mergable
      return false;
    } else if(    !this->timeInterval.Adjacent(i.timeInterval)
               && !this->timeInterval.Intersects(i.timeInterval) ){
      return false; // have a gap in between --> not mergeable
    } else if(!this->EqualValue(i)) { // temporal functions are NOT equal
      return false;
    }
    // merge the units (i.e. their timeIntervals)
    ConstTemporalUnit<Alpha> res(false);
    if(this->StartsBefore(i)){
      res.timeInterval.start = this->timeInterval.start;
      res.timeInterval.lc    = this->timeInterval.lc;
    } else {
      res.timeInterval.start = i.timeInterval.start;
      res.timeInterval.lc    = i.timeInterval.lc;
    }
    if(this->EndsAfter(i)){
      res.timeInterval.end   = this->timeInterval.end;
      res.timeInterval.rc    = this->timeInterval.rc;
    } else {
      res.timeInterval.end   = i.timeInterval.end;
      res.timeInterval.rc    = i.timeInterval.rc;
    }
    res.constValue = this->constValue;
    if(res.IsDefined() && res.IsValid()){ // invalid result -- do nothing!
      *this = res;
      return true;
    } else {
      return false;
    }
  }
/*
Merges unit ~i~ into this unit if possible and return ~true~. Otherwise do
not modify this unit and return ~false~.

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
    if (this->IsDefined() && !ctu->IsDefined())
      return 0;
    if (!this->IsDefined())
      return -1;
    if (!ctu->IsDefined())
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
    if( this->IsDefined() )
      {
        os << ConstTemporalUnit<Alpha>::BasicType() << ": ( ";
        TemporalUnit<Alpha>::timeInterval.Print(os);
        os << ", ";
        constValue.Print(os);
        os << " ) " << endl;
        return os;
      }
    else
      return os << ConstTemporalUnit<Alpha>::BasicType()<<": (undef) ";
  }

  virtual size_t HashValue() const
  {
    if(!this->IsDefined()){
      return 0;
    }
    return static_cast<size_t>(   this->timeInterval.start.HashValue()
                                ^ this->timeInterval.end.HashValue()   ) ;
  }

  virtual ConstTemporalUnit<Alpha>* Clone() const
  {
    return new ConstTemporalUnit<Alpha>(*this);
  }

  virtual void CopyFrom( const Attribute* right )
  {
    const ConstTemporalUnit<Alpha>* i = (const ConstTemporalUnit<Alpha>*)right;
    this->SetDefined(i->IsDefined());
    this->timeInterval.CopyFrom( i->timeInterval );
    constValue.CopyFrom( &(i->constValue) );
  }

  // type name used in Secondo
  inline static const string BasicType()
  {
    return "u"+Alpha::BasicType();
  }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
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

Inherits a denined flag.

*/
class UReal : public StandardTemporalUnit<CcReal>
{
  public:
/*
3.7.1 Constructors and Destructor

*/
  UReal() {};

  UReal(bool is_defined):StandardTemporalUnit<CcReal>(is_defined),
                         a(0.0), b(0.0), c(0.0), r(false)
  { };

  UReal( const Interval<Instant>& _interval,
         const double a,
         const double b,
         const double c,
         const bool r ):
    StandardTemporalUnit<CcReal>( _interval ),
    a( a ), b( b ), c( c ),
    r( r )
    {
      del.refs=1;
      del.SetDelete();
      del.isDefined=true;
    }

  // linear approximation between v1 and v2
  UReal(const Interval<Instant>& _interval,
        const double v1,
        const double v2): StandardTemporalUnit<CcReal>(_interval){

       Instant diff = _interval.end - _interval.start;
       a = 0;
       r = false;
       c = v1;
       if(diff.IsZero()){
          b = 0;
          return;
       }
       b = (v2-v1) / diff.ToDouble();
       del.refs=1;
       del.SetDelete();
       del.isDefined = true;
  }
  // linear approximation between v1 and v2
  UReal(const Interval<Instant>& _interval,
        const CcReal& ccv1,
        const CcReal& ccv2): StandardTemporalUnit<CcReal>(_interval){
    if(!ccv1.IsDefined() || !ccv2.IsDefined()){
      SetDefined(false);
      return;
    }
    double v1 = ccv1.GetValue();
    double v2 = ccv2.GetValue();

    Instant diff = _interval.end - _interval.start;
    a = 0;
    r = false;
    c = v1;
    if(diff.IsZero()){
      b = 0;
      return;
    }
    b = (v2-v1) / diff.ToDouble();
    del.refs=1;
    del.SetDelete();
    del.isDefined = true;
  }

/*
Symbol for use in typemappings

*/
  static const string BasicType(){ return "ureal"; }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }
/*
3.6.2 Operator redefinitions

*/

  virtual UReal& operator=( const UReal& i )
  {
    del.isDefined = i.del.isDefined;
    if( !i.IsDefined() ){
      return *this;
    }
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
    if( !this->IsDefined() && !i.IsDefined() ) {
      return true;
    }
    return this->IsDefined() && i.IsDefined()
        && *((TemporalUnit<CcReal>*)this) == *((TemporalUnit<CcReal>*)&i)
        && AlmostEqual( a, i.a )
        && AlmostEqual( b, i.b )
        && AlmostEqual( c, i.c )
        && r == i.r;
  }
/*
Returns ~true~ if this temporal unit is equal to the temporal unit ~i~ and ~false~ if they are different.
Two undefined units are always equal.

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

  virtual bool EqualValue( const UReal& i ) const
  {
    if( !this->IsDefined() && !i.IsDefined() ) {
      return true;
    }
    if( !this->IsDefined() || !i.IsDefined() ) {
      return false;
    }
    double offset = (i.timeInterval.start - timeInterval.start).ToDouble();

    return
      (AlmostEqual( a, i.a ) &&
       (r == i.r) &&
       AlmostEqual( 2 * a * offset + b, i.b ) &&
       AlmostEqual( a * pow(offset, 2) + b * offset + c, i.c )
       );
  }

/*
Returns ~true~ if the value of this temporal unit is equal to the
value of the temporal unit ~i~ and ~false~ if they are different.
Equality is calculated with respect to temporal evolution.

*/

/*
3.7.3 Functions to be part of relations


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
        os << "UReal: " << "( ";
        timeInterval.Print(os);
        os << ", ";
        // print specific stuff:
        os << " ( " << a << ", " << b << ", " << c
           << (r ? ", TRUE) " : ", FALSE) ");
        os << " ) ";
        return os;
      }
    else
      return os << "UReal: (undef) ";
  }

  virtual size_t HashValue() const
  {
    if(!IsDefined()){
      return 0;
    }
    return static_cast<size_t>(   timeInterval.start.HashValue()
                                ^ timeInterval.end.HashValue()   ) ;
  }

  virtual UReal* Clone() const
  {
    UReal *res;
    if( !this->IsDefined() ){
      res = new UReal( false );
    } else {
      res = new UReal( timeInterval, a, b, c, r);
    }
    res->del.isDefined = del.isDefined;
    return res;
  }

  virtual void CopyFrom( const Attribute* right )
  {
    const UReal* i = (const UReal*)right;
    del.isDefined = i->del.isDefined;
    if(i->IsDefined())
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
*Precondition*: this[->]IsDefined() is true

*/

  int PeriodsAtVal( const double& value, Periods& times) const;

/*
Sets the Periods value to the times, where this takes the
specified value. Returns the number of results (0-2).

*WARNING*: May return points, that are not inside this->timeInterval,
           if a value is located at an open start/end instant.

*/

  double PeriodsAtMin(bool& correct, Periods& times) const;
/*
Sets the Periods value to the times, where this takes the
minimum value. Returns the minimum value.

*WARNING*: May return points, that are not inside this->timeInterval,
           if a value is located at an open start/end instant.

*/

  double PeriodsAtMax(bool& correct, Periods& times) const;
/*
Sets the Periods value to the times, where this takes the
minimum value. Returns the maximum value.

*WARNING*: May return points, that are not inside this->timeInterval,
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

*Precondition*: this[->]IsDefined()

*Result*: stores the resultununit into vector result and returns
          the number of results (1-2) found.

*WARNING*: AtMin may return points, that are not inside this->timeInterval,
           if a minimum is located at an open start/end instant.

*/

  int AtMax( vector<UReal>& result ) const;

/*
Creates a vector of units, which are the restriction of this to
the periods, where it takes its maximum value.

*Precondition*: this[->]IsDefined()

*Result*: stores the resultunit into vector result and returns
          the number of results (1-2) found.

*WARNING*: AtMax may return points, that are not inside this->timeInterval,
           if a maximum is located at an open start/end instant.

*/

  int AtValue(CcReal value, vector<UReal>& result) const;
/*
Creates a vector of units, which are the restriction of this to
the periods, where it takes a certain value.

*Precondition*: this[->]IsDefined() AND value.IsDefined()

*Result*: stores the resultunit into vector result and returns
          the number of results (1-2) found.

*WARNING*: AtMax may return points, that are not inside this->timeInterval,
           if a maximum is located at an open start/end instant.

*/

  int IsEqual(const UReal& other, vector<UBool>& result) const;
/*
Creates a vector of ubool, that cover the UReals common deftime and
indicate whether their temporal values are equal or not.

*Precondition*: this[->]IsDefined() AND value.IsDefined()

*Result*: stores the resultunit into vector result and returns
          the number of results found.

*/

int Abs(vector<UReal>& result) const;
/*
Creates the absolute value for an UReal value.
~result~ may contain 0-3 UReal values.

*Precondition*: this[->]IsDefined()

*Result*: stores the resultunits into vector result and returns
          the number of results.

*/

  int Distance(const UReal& other, vector<UReal>& result) const;
/*
Creates the distance to an other UReal value.
~result~ may contain 0-3 UReal values.

*Preconditions*: this[->]IsDefined() AND other.IsDefined()
                 this->r == other.r == false

*Result*: stores the resultunits into vector result and returns
          the number of results.

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

    void CompUReal(UReal& ur2, int opcode, vector<UBool>& res);
    /*
    Computes for two given ~ureal~ values the resulting ~ubool~ values,
    sorted by their time intervals, depending on the compare result for
    ~opcode~. Possible opcode values are:
    opcode == 0 =
    opcode == 1 #
    opcode == 2 <
    opcode == 3 >
    opcode == 4 <=
    opcode == 5 >=
    */

/*
3.7.5 Attributes

*/
  double a, b, c;
  bool r;
};





/*
The following class describes an event created by an object moving over a
regular grid.

*/
class GridCellSeq {
  public:
    GridCellSeq(); // standard constructor - creates undefined instance
    GridCellSeq(const DateTime &enter,
                const DateTime &leave,
                const int32_t &cellNo); // automatically updates defined
    GridCellSeq(const GridCellSeq &other); // copy-constructor
    GridCellSeq& operator=(const GridCellSeq &other); // assignment
    ~GridCellSeq(); // destructor

    DateTime getEnterTime() const;   // read attribute
    DateTime getLeaveTime() const;   // read attribute
    int32_t getCellNo() const;       // read attribute
    void setCellNo(const int32_t &n);     // set attr and update definedness
    void setEnterTime(const DateTime &t); // set attr and updates definedness
    void setLeaveTime(const DateTime &t); // set attr and updates definedness
    void setUndefined();                  // sets defined to false
    bool IsDefined() const;          // returns definedness
    void set(const int32_t &c, const DateTime &s, const DateTime &e);
                                // set all attributes and update definedness
    ostream& Print( ostream &os ) const;

  private:
    DateTime my_enterTime; // initial instant of presence in the cell
    DateTime my_leaveTime; // final instant of presence in the cell
    int32_t my_cellNo;     // the cell concerned.
    bool defined;          // well-definedness of the data
};



/*
3.8 UPoint

This class will be used in the ~upoint~ type constructor, i.e., the type constructor
for the temporal unit of point values.

*/
class UPoint : public SpatialTemporalUnit<Point, 3>
{
/*
3.8.1 Constructors and Destructor

*/
  public:

  UPoint() {};

  UPoint(bool is_defined):
    SpatialTemporalUnit<Point, 3>(is_defined)
  {
  };
  UPoint( const Interval<Instant>& _interval,
          const double x0, const double y0,
          const double x1, const double y1 ):
    SpatialTemporalUnit<Point, 3>( _interval ),
    p0( true, x0, y0 ),
    p1( true, x1, y1 )
    { }

  UPoint( const Interval<Instant>& _interval,
          const Point& _p0, const Point& _p1 ):
    SpatialTemporalUnit<Point, 3>( _interval ),
    p0( _p0 ),
    p1( _p1 )
    {
      SetDefined(p0.IsDefined() && p1.IsDefined());
    }

  UPoint(const UPoint& source):
    SpatialTemporalUnit<Point, 3>(source.IsDefined()) {
     *((TemporalUnit<Point>*)this) = *((TemporalUnit<Point>*)&source);
     p0 = source.p0;
     p1 = source.p1;
     del.refs=1;
     del.SetDelete();
     del.isDefined = source.del.isDefined;
  }

/*
3.6.2 Operator redefinitions

*/

  virtual UPoint& operator=( const UPoint& i )
  {
    del.isDefined = i.del.isDefined;
    if( !i.IsDefined() ){
      return *this;
    }
    *((TemporalUnit<Point>*)this) = *((TemporalUnit<Point>*)&i);
    p0 = i.p0;
    p1 = i.p1;
    return *this;
  }

/*
~GetNoComponents~

Returns the constant number 1. This function allows for
templates using UPoint and MPoint.

*/
  int GetNoComponents() const{
     return 1;
  }


/*
~Get~


*/

    void Get( const int i, UPoint& upi ) const{
      assert(i==0);
      upi = *this;
    }



/*
Redefinition of the copy operator ~=~.

*/

  virtual bool operator==( const UPoint& i ) const
  {
    if( !this->IsDefined() && !i.IsDefined() ){
      return true;
    }
    return this->IsDefined() && i.IsDefined()
        && *((TemporalUnit<Point>*)this) == *((TemporalUnit<Point>*)&i)
        && AlmostEqual( p0, i.p0 )
        && AlmostEqual( p1, i.p1 );
  }
/*
Returns ~true~ if both units are undefined, or if both are defined and this temporal unit is equal to the temporal unit ~i~ and ~false~ if they are different.

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
  void TemporalFunction( const Instant& t,
                         Point& result,
                         const Geoid* geoid,
                         bool ignoreLimits = false) const;
  virtual bool Passes( const Point& val ) const;
  bool Passes( const Point& val, const Geoid* geoid ) const;
  bool Passes( const Region& val ) const;
  bool Passes( const Rectangle<2>& rect ) const;
  virtual bool At( const Point& val, TemporalUnit<Point>& result ) const;
  bool At( const Point& val, TemporalUnit<Point>& result,
           const Geoid* geoid ) const;
  virtual void AtInterval( const Interval<Instant>& i,
                           TemporalUnit<Point>& result ) const;
  void AtInterval( const Interval<Instant>& i,
                   TemporalUnit<Point>& result,
                   const Geoid* geoid ) const;
  void At(const Rectangle<2>& rect, UPoint& result) const;

/*
Computes the temporal distance to a Point ~p~. Result is a single unit.

Only to be used with euclidean coordinate data!

*/
  void Distance( const Point& p, UReal& result ) const;


/*
Computes the temporal distance to a Point ~p~. All result units will be appended
to the ~result~ vector of UReal values. If ~geoid~ in NULL, euclidean geometry is
applied and only a single result unit is appended. Otherwise, spherical geometry
is used ~result~ is appended with a series uf UReal units approximating the
actual moving distance.

UNDEFINED UReal values may be appended!.

*/
  void Distance( const Point& p, vector<UReal>& result,
                 const Geoid* geoid = 0, const double epsilon = 0.00001 ) const;

/*
Computes the temporal distance to a Point ~p~ using an iterative method. The
result is passed in output parameter ~result~. The vector may contain UNDEFINED
units.

The function works recursively. It is possible to pass down pre-computed data
using parameters ~tMin~ (instant of nearest approach), ~distMin~ (distance at
nearest approach), ~distStart~ (distance at initial instant of THIS unit),
~distEnd~ (distance at final instant of THIS unit). Doing so avoids repeated
calculation of these values. The default parameters make sure, that the values
will be computed once.

Computation stops, once the absolute distance difference between consecutive
steps drops below ~epsilon~.

*/
  void DistanceOrthodrome( const Point& p, vector<UReal>& result,
                           const Geoid geoid,
                           const double epsilon  = 0.00001,
                           const Instant* tMin   = 0,
                           const double distMin  =-666.666,
                           const double distStart=-666.666,
                           const double distEnd  =-666.666            ) const;
/*
Computes the spatial projection of THIS UPoint.

*/
  void UTrajectory( Line& line ) const;

/*
The scalar velocity as a temporal function
If geoid = 0, metric (X,Y)-coordinates are used within the UPoint.
If geoid points to a valid Geoid object, geografic coordinates (LON,LAT)
are used within the UPoint. Speed is interpreted as speed over ground!

*/
  void USpeed( UReal& result, const Geoid* geoid = 0 ) const;

/*
The vectorial velocity --- (X,Y)-components --- as temporal function

*/
  void UVelocity( UPoint& result ) const;
  void Intersection(const UPoint &other, UPoint &result) const;

/*
~EqualValue~ returns true iff both units describe parts if the same linear
temporal function.

*/
  virtual bool EqualValue( const UPoint& i ) const {
    if( !IsDefined() && !i.IsDefined() ){
      // both undefined
//       cout << "\t" << __PRETTY_FUNCTION__ << " SUCCEEDED: both undefined."
//            << endl;
      return true;
    } else if( !IsDefined() || !i.IsDefined() ){
      // one of *this and i undefined
//       cout << "\t" << __PRETTY_FUNCTION__ << " FAILED: one undefined."
//            << endl;
      return false;
    } // else: both are defined
    Point v(false);
    TemporalFunction(i.timeInterval.start, v, true);
    if( !v.IsDefined() || !AlmostEqual(i.p0,v) ){
//       cout << "\t" << __PRETTY_FUNCTION__ << " FAILED: start1 unmatched."
//            << endl;
      return false;
    }
    TemporalFunction(i.timeInterval.end, v, true);
    if( !v.IsDefined() || !AlmostEqual(i.p1,v) ){
//       cout << "\t" << __PRETTY_FUNCTION__ << " FAILED: end1 unmatched."
//            << endl;
//       cout << "\t" << __PRETTY_FUNCTION__ << " i.p1 = " << i.p1 << endl;
//       cout << "\t" << __PRETTY_FUNCTION__ << " v    = " << v    << endl;
      return false;
    }
    TemporalFunction(timeInterval.start, v, true);
    if( !v.IsDefined() || !AlmostEqual(p0,v) ){
//       cout << "\t" << __PRETTY_FUNCTION__ << " FAILED: start2 unmatched."
//            << endl;
      return false;
    }
    TemporalFunction(timeInterval.end, v, true);
    if( !v.IsDefined() || !AlmostEqual(p1,v) ){
//       cout << "\t" << __PRETTY_FUNCTION__ << " FAILED: end2 unmatched."
//            << endl;
      return false;
    }
//       cout << "\t" << __PRETTY_FUNCTION__
//            << " SUCCEEDED: all points matched." << endl;
    return true;
  }

  virtual bool Merge( const UPoint& i ) {
    // check for gap in definition time
    if(!IsDefined() && !i.IsDefined()) { // mergeable, but nothing to do
//       cout << __PRETTY_FUNCTION__ << " SUCCEEDED: both undefined." << endl;
      return true;
    } else if(!IsDefined() || !i.IsDefined()) { // not mergable
//       cout << __PRETTY_FUNCTION__ << " FAILED: one undefined." << endl;
      return false;
    } else if(    !this->timeInterval.Adjacent(i.timeInterval)
               && !this->timeInterval.Intersects(i.timeInterval) ){
//       cout << __PRETTY_FUNCTION__ << " FAILED: found definition gap."
//            << endl;
      return false; // have a gap in between --> not mergeable
    } else if(!EqualValue(i)) { // temporal functions are NOT equal
//       cout << __PRETTY_FUNCTION__ << " FAILED: functions not equal." << endl;
      return false;
    }
    // merge the units (i.e. their timeIntervals)
    UPoint res(false);
    if(StartsBefore(i)){
      res.timeInterval.start = this->timeInterval.start;
      res.timeInterval.lc    = this->timeInterval.lc;
      res.p0                 = this->p0;
    } else {
      res.timeInterval.start = i.timeInterval.start;
      res.timeInterval.lc    = i.timeInterval.lc;
      res.p0                 = i.p0;
    }
    if(EndsAfter(i)){
      res.timeInterval.end   = this->timeInterval.end;
      res.timeInterval.rc    = this->timeInterval.rc;
      res.p1                 = this->p1;
    } else {
      res.timeInterval.end   = i.timeInterval.end;
      res.timeInterval.rc    = i.timeInterval.rc;
      res.p1                 = i.p1;
    }
    if(res.IsDefined() && res.IsValid()){ // invalid result -- do nothing!
//       cout << __PRETTY_FUNCTION__ << " SUCCEEDED: created result unit."
//            << endl;
      *this = res;
      return true;
    } else {
//       cout << __PRETTY_FUNCTION__ << " FAILED: result invalid." << endl;
      return false;
    }
  }
/*
Merges UPoint ~i~ into this unit if possible and return ~true~. Otherwise do
not modify this unit and return ~false~.

*/

  void Translate(const double x, const double y,
                 const DateTime& duration);
/*
Translates a moving point spatially and temporally.

*/

  void GetGridCellSequence(CellGrid2D &g, vector<GridCellSeq> &res);
/*
Computes all events created by a UPoint moving across a regular grid.

*/

/*
3.8.3 Functions to be part of relations

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
    UPoint* up = (UPoint*) arg;
    if(timeInterval.end == up->timeInterval.start){ // up after this
       return AlmostEqual(p1, up->p0) &&
             (timeInterval.rc || up->timeInterval.lc);
    }
    if(up->timeInterval.end == timeInterval.start){
       return AlmostEqual(up->p1,p0) &&
              (up->timeInterval.rc || timeInterval.lc);
    }
    return false;
  }

  inline virtual ostream& Print( ostream &os ) const
  {

    if(IsDefined())
      {
        os << "UPoint: " << "( ";
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
    if(!IsDefined()){
      return 0;
    }
    return static_cast<size_t>(   timeInterval.start.HashValue()
                                ^ timeInterval.end.HashValue()   ) ;
  }

  inline virtual UPoint* Clone() const
  {
    UPoint *res;
    if(this->IsDefined()){
      res = new UPoint( timeInterval, p0, p1 );
      res->del.isDefined = del.isDefined;
    } else {
      res = new UPoint( false );
//       res->timeInterval = Interval<Instant>();
      res->p0 = Point( false, 0.0, 0.0);
      res->p1 = Point( false, 0.0, 0.0);
    }
    return res;
  }

  inline virtual void CopyFrom( const Attribute* right )
  {
    const UPoint* i = static_cast<const UPoint*>(right);

    if(i->del.isDefined)
      {
        timeInterval.CopyFrom( i->timeInterval );
        p0 = i->p0;
        p1 = i->p1;
      }
    else
      {
//         timeInterval = Interval<Instant>();
        p0 = Point( false, 0.0, 0.0);
        p1 = Point( false, 0.0, 0.0);
      }
    del.isDefined = i->del.isDefined;
  }

  virtual const Rectangle<3> BoundingBox(const Geoid* geoid = 0) const
  {
    if(geoid){
      if(!geoid->IsDefined() || !IsDefined()){
        return Rectangle<3>(false,0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
      }
      Rectangle<2> geobbox(false);
      if(AlmostEqual(p0,p1)){
        Rectangle<2> geobbox = p0.GeographicBBox(p1, *geoid);
        return Rectangle<3>( true, geobbox.MinD(0),
                             geobbox.MaxD(0),
                             geobbox.MinD(1),
                             geobbox.MaxD(1),
                             timeInterval.start.ToDouble(),
                             timeInterval.end.ToDouble() );
      } // else: use HalfSegment::BoundingBox(...)
      geobbox = HalfSegment(true,p0,p1).BoundingBox(geoid);
      return Rectangle<3>( true, geobbox.MinD(0),
                                 geobbox.MaxD(0),
                                 geobbox.MinD(1),
                                 geobbox.MaxD(1),
                                 timeInterval.start.ToDouble(),
                                 timeInterval.end.ToDouble() );
    } // else: euclidean geometry
    if(this->IsDefined()){
      return Rectangle<3>( true, MIN( p0.GetX(), p1.GetX() ),
                                 MAX( p0.GetX(), p1.GetX() ),
                                 MIN( p0.GetY(), p1.GetY() ),
                                 MAX( p0.GetY(), p1.GetY() ),
                                 timeInterval.start.ToDouble(),
                                 timeInterval.end.ToDouble() );
    } else {
      return Rectangle<3>( false );
    }
  }

  virtual const Rectangle<3> BoundingBox(const double scaleTime,
                                         const Geoid* geoid = 0) const
  {
    Rectangle<3> bbx = this->BoundingBox(geoid);
    if(bbx.IsDefined()){
      return Rectangle<3>( true, bbx.MinD(0),
                                 bbx.MaxD(0),
                                 bbx.MinD(1),
                                 bbx.MaxD(1),
                                 timeInterval.start.ToDouble()*scaleTime,
                                 timeInterval.end.ToDouble()*scaleTime );
    } else {
      return Rectangle<3>( false );
    }
  }

  const Rectangle<2> BoundingBoxSpatial(const Geoid* geoid = 0) const
  {
    Rectangle<3> bbx = this->BoundingBox(geoid);
    if(bbx.IsDefined()){
      return Rectangle<2>( true, bbx.MinD(0),
                           bbx.MaxD(0),
                           bbx.MinD(1),
                           bbx.MaxD(1) );
    } else {
      return Rectangle<2>( false );
    }
  }

/*
Calculates the distance between 2 upoints as a real value.
If ~geoid~ is NULL, euclidean geometry is used, spherical geometry otherwise.
If invalid geographic coordinates are found, the result is UNDEFINED.

*Precondition*: intersecting timeIntervals.

*Result*: the distance of two upoints as a ureal

*/

  void Distance( const UPoint& up, UReal& result, const Geoid* geoid = 0) const;

/*
Computes the distance between the three dimensional line defined by
that unit and the rectangle.

*/
virtual double Distance(const Rectangle<3>& rect, const Geoid* geoid=0) const;

virtual bool Intersects(const Rectangle<3>& rect, const Geoid* geoid=0) const;


  virtual bool IsEmpty() const{
    return !IsDefined();
 }

/*
Calculates the spatial length of the unit using metric (X,Y) coordinates.

*Precondition*: none

*Result*: the distance of the unit's initial and final value

*/
void Length( CcReal& result ) const;


/*
Calculates the spatial length of the unit using geographic (LON,LAT)-coordinates.

*Precondition*: none

*Result*: the distance of the unit's initial and final value

*/
void Length( const Geoid& g, CcReal& result ) const;

/*
Calculates the intersection of the UPoint and Region r.

*Precondition*: none

*Result*: The ~result~ is a vector of UPoints, sorted ascendingly by their starting instants.
The boolean return value is ~false~, iff either the UPoint or the Region is UNDEFINED.

*/

  bool AtRegion(const Region *r, vector<UPoint> &result) const;

/*
Calculates the ~direction~ (when ~useHeading~ is ~false~) resp. the heading
(when ~true~) of this UPoint. If ~geoid~ is not NULL, spherical geometry is
applied and the true course is approximated as a series of units according
to preciseness parameter ~epsilon~.

Any results are appended to the ~result~ vector.
Attention: UNDEFINED units my be appended!

*/
  void Direction( vector<UReal> &result,
                  const bool useHeading = false,
                  const Geoid* geoid    = 0,
                  const double epsilon  = 0.0000001) const;

  static const string BasicType(){ return "upoint"; }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }

/*
~IsStatic~

Returns true, iff this unit is defined and not moving during its definition time.

*/
   bool IsStatic() const{

      return IsDefined() && AlmostEqual(p0,p1);

   }

/*
3.8.4 Attributes

*/
  Point p0, p1;
};


ostream& operator<<(ostream& o, const UPoint& u);
ListExpr OutUPoint( ListExpr typeInfo, Word value );
Word InUPoint( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct );
/*
3.9 Mapping

This class will implement the functionalities of the ~mapping~ type constructor.
It contains a database array of temporal units. For that, ~Unit~ must implement
the class ~TemporalUnit~ or ~ConstTemporalUnit~, because functions of these classes will be used.

*/
bool IsMaximumPeriods(const Periods& p);

template <class Unit, class Alpha>
class Mapping : public Attribute
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
Returns true, if the unit set is ordered.
There is a flag ~ordered~ (see attributes) in order
to avoid a scan in the unit set to answer this question.

*/

    void StartBulkLoad();
/*
Marks the start of a bulk load of units relaxing the condition that the units must be
ordered. We will assume that the only way to add units to an unit set is inside bulk
loads, i.e., into non-ordered mappings.

*/

    virtual void EndBulkLoad( const bool sort = true,
                              const bool checkvalid = false );
/*
Marks the end of a bulk load and sorts the unit set if the flag ~sort~ is set to true.
Checkvalid indicated, wheter the validity (temporally non-overlapping untis) should
be checked. In this case, if the Range is invalid, it is set to undefined!

3.10.3 Member functions

*/
    bool IsEmpty() const;
/*
Returns true, if the mapping is undefined or empty of units.

*/

    void Get( const int i, Unit& upi ) const;
/*
Returns the unit ~upi~ at the position ~i~ in the mapping.

*Precondition*: this[->]IsDefined() == true

*Precondition*: 0 <= this[->]NoComponents() <= i

*/

    virtual void Add( const Unit& upi );
/*
Adds an unit ~upi~ to the mapping. We will assume that the only way of adding units
is in bulk loads, i.e., in a non-ordered array.

*Precondition:* this[->]IsDefined() == true

*Precondition:* ~IsOrdered() == false~

*/

    virtual void MergeAdd( const Unit& upi );
/*
Adds an unit ~upi~ to the mapping. If the new unit and the last
unit in the Mapping are equalValue it merges the two units.
We will assume that the only way of adding units
is in bulk loads, i.e., in a non-ordered array.
Without defining the function ~equalValue~ for units
~MergeAdd~ works the same way as ~Add~.

*Precondition:* this[->]IsDefined() == true

*Precondition:* ~IsOrdered() == false~

*/

    inline void Put(const int i, const Unit& u){
       units.Put(i,u);
    }
/*
Replaces the unit at position ~i~ within the mapping's DbArray.

*Precondition:* this[->]IsDefined() == true

*/

    virtual void Clear();
/*
Remove all units in the mapping and set the defined flag.

*/

    bool IsValid() const;
/*
This functions tests if a ~mapping~ is in a valid format. It is used for debugging
purposes only. The ~mapping~ is valid, if the following conditions are true:

Either the mapping is undefined, or the mapping is defined and conditions 1--3 hold:

  1 Each unit is valid

  2 Start of each unit $>=$ end of the unit before

  3 If start of an unit = end of the unit before, then one needs to
    make sure that the unit is not left-closed or the unit before
    is not right-closed

3.10.4 Functions to be part of relations

*/
    inline virtual size_t Sizeof() const;
    inline virtual int Compare( const Attribute* arg ) const;
    inline bool Adjacent( const Attribute* arg ) const;
    inline Attribute* Clone() const;
    inline virtual ostream& Print( ostream &os ) const;
    inline size_t HashValue() const;
    inline virtual void CopyFrom( const Attribute* right );
    inline virtual void Restrict( const vector< pair<int, int> >& intervals );

    inline int NumOfFLOBs() const;
    inline Flob *GetFLOB(const int i);

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

*Precondition:* ~IsDefined() == true~

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
3.3.3.3 Operation ~Temporal Function~
same as ~AtInstant~, but returns an Alpha, not an Intime.

*/
    void TemporalFunction( const Instant& t,
                           Alpha& result,
                           bool ignoreLimits = true) const;

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
3.10.5.4 Operator compress

Creates a compressed version of this mapping.

*/
   Mapping<Unit,Alpha>* compress() const;



/*
3.10.5.3 Operation ~passes~

*Precondition:* ~X.IsDefined() AND X.IsOrdered()~

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
3.10.5.4 ~TrimToSize~

Changes the unit's array to have space to contain exactly the
number of actually contained elements.

*/
    void TrimToSize();


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
type name used in Secondo

*/
  inline static const string BasicType()
  {
    return "m"+Alpha::BasicType();
  }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }

/*
~Move in time~

*/
  void timeMove(const DateTime& duration, Mapping<Unit, Alpha>& result) const;

  void moveTo(const DateTime& instant, Mapping<Unit,Alpha>& result) const;



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

  protected:
    DbArray< Unit > units;
/*
The database array of temporal units.

*/
};

/*
3.10 Class ~MBool~

*/
typedef Mapping< UBool, CcBool > MBool;
class MReal;
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
~MergeAddFillUp~

Append the unit to this MInt. If a gap in time between the last
Unit and the new units exists, it is filled up using a unit having value
as it's value.

*/

  void MergeAddFillUp(const UInt& unit, const int fillValue);


/*
Fills gaps in the definition time eith the given value.

*/
  void fillUp(int value, MInt& result) const;

  int maximum() const;

  int minimum() const;

  int Min(bool& correct) const;
  int Max(bool& correct) const;



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

/*
~WriteTo(MBool)~

Writes the value of this moving(int) into the value
of the result moving(bool). The temporal structure of
the result is equal to the temporal structure of this
moving(int). A unit holding the value zero is converted
to a unit with value false. Otherwise true.

*/
  void WriteTo(MBool& arg);

/*
~WriteTo(MReal)~

Casts this MInt into an MReal.

*/
    void WriteTo(MReal& arg);


   static const string BasicType(){ return "mint"; }
   static const bool checkType(const ListExpr type){
     return listutils::isSymbol(type, BasicType());
   }
   void Hat(MInt& mint);

/*
~Restrict~

This operator removes the first (last) unit of this mint if it
starts (ends) with endoftime (beginoftime). If __useValue__
is set to true, the units are only removed if the stored value is equals to
the given value.

*/
   void Restrict(MInt& result,
                 const bool useValue = false,
                 const int value = 0) const;


   void  PlusExtend(const MInt* arg2, MInt& result) const;

   void SortbyUnitTime();

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
      {
        del.refs=1;
        del.SetDelete();
        del.isDefined = true;
      }

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

Restrict to periods where the temporal value is minimal.

*/
   void AtMin( MReal& result ) const;

/*
3.11.7 Operation ~AtMax~

Restrict to periods where the temporal value is maximal.

*/
   void AtMax( MReal& result ) const;

/*
3.11.8 Operation ~AtValue~

Restrict to periods where the temporal value is equal to a const value.

Precondition: ccvalue.IsDefined() == true

*/
   void AtValue( const CcReal& ccvalue, MReal& result ) const;


   static const string BasicType(){ return "mreal"; }
   static const bool checkType(const ListExpr type){
     return listutils::isSymbol(type, BasicType());
   }

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
      {
        del.refs=1;
        del.SetDelete();
        del.isDefined = true;
        bbox = Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
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
  void Add( const UPoint& unit );
  void MergeAdd(const UPoint& unit);
  void EndBulkLoad( const bool sort = true, const bool checkvalid = false );
  void Restrict( const vector< pair<int, int> >& intervals );
  ostream& Print( ostream &os ) const;
  bool operator==( const MPoint& r ) const;
  bool Present( const Instant& t ) const;
  bool Present( const Periods& t ) const;
  void AtInstant( const Instant& t, Intime<Point>& result ) const;
  void AtPeriods( const Periods& p, MPoint& result ) const;
  void AtRect(const Rectangle<2>& rect, MPoint& result) const;


  virtual Attribute* Clone() const
  {
    assert( IsOrdered() );
    MPoint *result;
    if( !this->IsDefined() ){
      result = new MPoint( 0 );
    } else {
      result = new MPoint( GetNoComponents() );
      if(GetNoComponents()>0){
        result->units.resize(GetNoComponents());
      }
      result->StartBulkLoad();
      UPoint unit;
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
    const MPoint *r = (const MPoint*)right;
    assert( r->IsOrdered() );
    Clear();
    this->SetDefined(r->IsDefined());
    if( !this->IsDefined() ) {
      return;
    }
    StartBulkLoad();
    UPoint unit;
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

*Complexity:* $O( n )$, where ~n~ is the number of units of this ~MPoint~

*/
  void Trajectory( Line& line ) const;

  // The scalar velocity as a temporal function
  // If geoid = 0, metric (X.Y)-coordinates are used within the MPoint.
  // If geoid points to a valid Geoid object, geografic coordinates (LON,LAT)
  // are used within the MPoint (speed over ground).
  void MSpeed(  MReal& result, const Geoid* geoid = 0 ) const;

  // The vectorial velocity --- (X,Y)-components --- as temporal function
  void MVelocity( MPoint& result ) const;

/*
3.10.5.3 Operation ~distance~

If ~geoid~ is NULL, euclidean geometry is used, spherical geometry otherwise.
If invalid geographic coordinates are found, the result is UNDEFINED.

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( n )$, where ~n~ is the number of units of this ~MPoint~

*/
  void Distance( const Point& p, MReal& result,
                 const Geoid* geoid=0 ) const;
  void SquaredDistance( const Point& p, MReal& result,
                        const Geoid* geoid=0 ) const;
  void SquaredDistance( const MPoint& p, MReal& result,
                        const Geoid* geoid=0 ) const;

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
    void BreakPoints(Points& result, const DateTime& dur,
                     const CcReal& epsilon,
                     const Geoid* geoid=0) const;

/*
3.10.5.6 Operatiopn ~Breaks~

This function computes the timeIntervalls for Breaks

*/

    void Breaks(Periods& result, const DateTime& dur,
                const CcReal& epsilon,
                const Geoid* geoid=0) const;


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
   void gk(const int &zone, MPoint& result) const;


/*
3.10.5.6 Operation ~TranslateAppend~

Appends the mpoint argument to this moving point. The point will stay on its
last position for an interval of length dur. If dur is smaller or equals to
zero, the new movement starts directly after the definition time of the
original object. The movement is continued at the last position of this mpoint.

*/

  void TranslateAppend(const MPoint& mp, const DateTime& dur);


/*
3.10.5.7 Reverse

Store the reverse of the movement of this instance of a mpoint into result.

*/
  void Reverse(MPoint& result);


/*
3.10.5.7 Direction

Compute the ~direction~ (~useHeading~ is ~false~) resp. ~heading~ (~true~) of a
moving point.
If ~geoid~ is not NULL, spherical geometry is applied. In this case, each unit
may produce several result units, since the true course changes along the
orthodrome. Parameter ~epsilon~ determines how exact the approximation will be.

*/
  void Direction( MReal* result,
                  const bool useHeading = false,
                  const Geoid* geoid    = 0,
                  const double epsilon  = 0.0000001) const;

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
  void Sample(const DateTime& duration, MPoint& result,
              const bool KeepEndPoint = false,
              const bool exactPath = false  )const;

/*
3.10.5.8 ~Append~

The ~Append~ function appends all units of the argument to this
MPoint. If this mpoint or the argument is undefined or if the
argument starts before this mpoint ends, this mpoint will be set
to be undefined. The return value is the defined state of this
mpoint after the operation (indicating the success).

*/
   bool Append(const MPoint& p, const bool autoresize = true);

/*
3.10.5.9 ~Disturb~

The ~disturb~ operation changes the position of the moving point
using a random generator.

*/
   void Disturb(MPoint& result,
                const double maxDerivation,
                double maxDerivationPerStep);


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

Returns the MPoint's minimum bounding rectangle. If geoid is NULL, euclidean
geometry is used, otherwise spherical geometry is applied.

*/
  // return the stored bbox
  Rectangle<3> BoundingBox(const Geoid* geoid = 0) const;

  // return the spatial bounding box (2D: X/Y)
  const Rectangle<2> BoundingBoxSpatial(const Geoid* geoid = 0) const;

  // recompute bbox, if necessary
  void RestoreBoundingBox(const bool force = false);


  void EqualizeUnitsSpatial(const double epsilon,
                            MPoint& result,
                            const bool skipSplit = false) const;

  static const string BasicType(){ return "mpoint"; }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }

/*
3.10.5.11 ~Delay Operator~

Considering this MPoint instance as the schedule, and a given MPoint as the
actual movement, the goal is to compute the continuous delay between the two
MPoints (i.e. How many seconds is the actual movement delayed from the
schedule).

If ~geoid~ is NULL, euclidean geometry is used, otherwise spherical geometry
using the provided geoid.

If invalid coordinates are found, the result is UNDEFINED.

*/
    MReal* DelayOperator(const MPoint *actual, const Geoid* geoid = 0 );
    MReal* DistanceTraversed(double*, const Geoid* geoid = 0 ) const;
    MReal* DistanceTraversed( const Geoid* geoid = 0 ) const;

/*
3.10.5.11 ~at region~ operators

This operator returns a copy of the mpoint's restriction to a given region

*/
    void AtRegion(const Region *reg, MPoint &result) const;


private:
   int IntervalRelation(Interval<Instant> &int_a_b,
        Interval<Instant> &int_c_d  ) const;
   double* MergePartitions(double* first, int firstSize,
        double* second, int secondSize, int& count );
   void Simplify(const int min, const int max,
                 bool* useleft, bool* useright,
                 const double epsilon) const;

   Rectangle<3> bbox;
};

/*
4 Implementation of C++ Classes

4.1 Interval

4.1.1 Constructors and Destructor

*/
template <class Alpha>
Interval<Alpha>::Interval( const Interval<Alpha>& interval ):
  start(interval.start),
  end(interval.end),
  lc( interval.lc ),
  rc( interval.rc )
{
}

template <class Alpha>
Interval<Alpha>::Interval( const Alpha& start1, const Alpha& end1,
                           const bool lc1, const bool rc1 ):
start(start1),
end(end1),
lc( lc1 ),
rc( rc1 )
{
//  assert(IsValid());
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
  if( !start.IsDefined() || !end.IsDefined() ){
    return false;
  }

  int cmp = start.Compare( &end );
  if( cmp < 0 ) // start < end
  {
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
bool Interval<Alpha>::operator<( const Interval<Alpha>& i ) const
{
   return CompareTo(i) <0;
}


template <class Alpha>
bool Interval<Alpha>::operator>( const Interval<Alpha>& i ) const
{
   return CompareTo(i) >0;
}


template <class Alpha>
bool Interval<Alpha>::R_Disjoint( const Interval<Alpha>& i ) const
{
    bool res= ((end.Compare( &i.start ) < 0) ||
               ( (end.Compare( &i.start ) == 0) && !( rc && i.lc ) ));
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
    bool res=( (Disjoint( i ) &&
               ( end.Compare( &i.start ) == 0 && (rc || i.lc) )) ||
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
bool Interval<Alpha>::Contains( const Alpha& a,
                                const bool ignoreCloseness /* = false */ ) const
{
  assert(this->IsValid());
  assert(a.IsDefined());

  bool lc = this->lc || ignoreCloseness;
  bool rc = this->rc || ignoreCloseness;
  return ( ( start.Compare( &a ) < 0 ||
             ( start.Compare( &a ) == 0 && lc ) ) &&
           ( end.Compare( &a ) > 0 ||
             ( end.Compare( &a ) == 0 && rc ) ) );
}


template <class Alpha>
bool Interval<Alpha>::Contains( const Interval<Alpha>& i,
                                const bool ignoreCloseness /* = false */ ) const
{
  int cmp1 = start.CompareTo(&(i.start));
  int cmp2 = end.CompareTo(&(i.end));
  if(cmp1>0) {    // i starts before this
    return false;
  }
  if((cmp1==0) &&  !lc && i.lc && !ignoreCloseness){ // i starts before this
    return false;
  }
  // start is ok
  if(cmp2<0){ // this ends before i
    return false;
  }
  if(cmp2==0 && !rc && i.rc && !ignoreCloseness){
     return false;
  }
  return true;
}

template <class Alpha>
bool Interval<Alpha>::Intersects( const Interval<Alpha>& i ) const
{
  assert( this->IsValid());
  assert( i.IsValid() );
  return !( (start.Compare(&i.end) > 0 ) ||
            ((start.Compare(&i.end) == 0) && ( !lc || !i.rc)) ||
            (end.Compare(&i.start) < 0 ) ||
            ((end.Compare(&i.start) == 0 ) && (!rc || !i.lc))
          );

}




template <class Alpha>
bool Interval<Alpha>::StartsBefore( const Interval<Alpha>& i ) const
{
  assert( IsValid() && i.IsValid() );

  return ( (start < i.start) || ( (start == i.start) && lc && !i.lc ));
}

template <class Alpha>
bool Interval<Alpha>::EndsAfter( const Interval<Alpha>& i ) const
{
  assert( IsValid() && i.IsValid() );

  return ( end > i.end || ( (end == i.end) && rc && !i.rc ));
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
bool Interval<Alpha>::After( const Interval<Alpha>& iv ) const
{
  assert( IsValid() && iv.IsValid() );

  return (iv.end < start) ||
         ((iv.end == start) && (!iv.rc || !lc));
}



template <class Alpha>
void Interval<Alpha>::Intersection( const Interval<Alpha>& i,
                                    Interval<Alpha>& result ) const
{
  assert( IsValid() && i.IsValid() );
  assert( Intersects( i ) );

  if( Inside( i ) )
  {
    result = *this;
  }
  else if( i.Inside( *this ) )
  {
    result = i;
  }
  else
  { // Normal intersection
    int comp = start.Compare( &i.start );
    if( comp < 0 )
    { // this starts smaller
      result.start.CopyFrom( &i.start );
      result.lc = i.lc;
    }
    else if( comp == 0 )
    { // equal start
      result.start.CopyFrom( &i.start );
      result.lc = (lc && i.lc);
    }
    else
    { // i starts smaller
      result.start.CopyFrom( &start );
      result.lc = lc;
    }

    comp = end.Compare( &i.end );
    if( comp > 0 )
    { // i ends smaller
      result.end.CopyFrom( &i.end );
      result.rc = i.rc;
    }
    else if( comp == 0 )
    { // equal end
      result.end.CopyFrom( &i.end );
      result.rc = (rc && i.rc);
    }
    else
    { // this ends smaller
      result.end.CopyFrom( &this->end );
      result.rc = rc;
    }
  }
}

template<class Alpha>
void Interval<Alpha>::IntersectionWith( const Interval<Alpha>& i){
  assert(this->IsValid());
  assert(i.IsValid());
  assert(this->Intersects(i));
  if(start < i.start){
      start = i.start;
      lc = i.lc;
  }else if(start==i.start){
    lc = lc && i.lc;
  }

  if(i.end < end){
     end = i.end;
     rc = i.rc;
  } else if(end == i.end){
     rc = rc && i.rc;
  }



}



template<class Alpha>
void Interval<Alpha>::Union(const Interval<Alpha>& iv,
                            Interval<Alpha> result) const{

   // set start
   if(start<iv.start){
       result.start = start;
       result.lc = lc;
    } else if(start > iv.start){
       result.start = iv.start;
       result.lc = iv.lc;
    } else {  // start == iv.start
      result.start = start;
      result.lc = lc || iv.lc;
    }

    // set end
    if(end>iv.end){
      result.end = end;
      result.rc = rc;
    } else if(end<iv.end){
       result.end = iv.end;
       result.rc = iv.rc;
    } else {
        result.end = end;
        result.rc = rc || iv.rc;
    }
}

template<class Alpha>
void Interval<Alpha>::Union(const Interval<Alpha>& iv) {

     if(iv.start < this->start){
        this->start = iv.start;
        this->lc = iv.lc;
     } else if(iv.start == this->start){
         this->lc = this->lc || iv.lc;
     }

    if(this->end < iv.end){
        this->end = iv.end;
        this->rc = iv.rc;
    } else if(iv.end == this->end){
        this->rc = this->rc || iv.rc;
    }
}





template<class Alpha>
int Interval<Alpha>::Minus(const Interval<Alpha>& iv,
                           Interval<Alpha>& res1,
                           Interval<Alpha>& res2){

    if(iv.Before(*this)){
        res1 = *this;
        return 1;
    }
    if(iv.After(*this)){
      res1=*this;
      return 1;
    }

    int no = 0;
    // check for an interval left of iv
    if( (start < iv.start) || ((start==iv.start) && lc && !iv.lc)){
       no++;
       res1 = *this;
       if(end > iv.start){
          res1.end = iv.start;
          res1.rc = !iv.lc;
       } else if(end == iv.start){
          res1.rc = rc && !iv.lc;
       }
    }

    // check for an remaining interval right of iv
    if( (end > iv.end) ||  ((end==iv.end) && rc && !iv.rc)){
        no++;
        Interval<Alpha>
        res = *this;
        if(start < iv.end){
          res.start = iv.end;
          res.lc = !iv.rc;
        } else if(start == iv.end) {
          res.lc = lc && !iv.rc;
        }
        if(no==1){
          res1 = res;
        } else {
          res2 = res;
        }
    }
    return  no;
}





template <class Alpha>
int  Interval<Alpha>::CompareTo( const Interval<Alpha>& i) const{
  int cmp = start.Compare( &(i.start) );
  if( cmp != 0 ){
    return cmp;
  }
  if(!lc && i.lc){
    return 1;
  }
  if(lc && !i.lc){
    return -1;
  }
  cmp = end.Compare( &(i.end) );
    if( cmp != 0 ){
    return cmp;
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
  Attribute(true),
  canDestroy( false ),
  ordered( true ),
  intervals( n )
{
  del.refs=1;
  del.SetDelete();
  del.isDefined=true;
}

template<class Alpha>
Range<Alpha>::Range(const Range<Alpha>& src):
   Attribute(src),
   canDestroy(false),
   ordered(src.ordered),
   intervals(src.intervals.Size()){
   intervals.copyFrom(src.intervals);
}




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
  assert( IsDefined() );
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
void Range<Alpha>::EndBulkLoad( const bool sort, const bool checkvalid )
{
  assert( IsDefined() );
  assert( !ordered );
  if( !IsDefined() ){
    intervals.clean();
  } else if( sort ){
    intervals.Sort( IntervalCompare<Alpha> );
  }
  ordered = true;
  intervals.TrimToSize();
  if( checkvalid && !IsValid() ){
    SetDefined( false );
    cerr << __PRETTY_FUNCTION__<< " found invalid range and marked it "
         << "undefined!"<< endl;
//     assert( isvalid );
  }
}

template <class Alpha>
bool Range<Alpha>::IsEmpty() const
{
  return !IsDefined() || (intervals.Size() == 0);
}

template <class Alpha>
void Range<Alpha>::Get( const int i, Interval<Alpha> &interval ) const
{
  assert( IsDefined() );
  assert(i>=0);
  assert(i<intervals.Size());
  intervals.Get( i, &interval );
  assert( interval.IsValid() );
}

template <class Alpha>
void Range<Alpha>::Add( const Interval<Alpha>& interval )
{
  assert( IsDefined() );
  assert( interval.IsValid() );
  intervals.Append( interval );
}

template<class Alpha>
void Range<Alpha>::MergeAdd(const Interval<Alpha>& interval){
  assert(IsDefined());
  assert(interval.IsValid());
  if(intervals.Size()==0){
    intervals.Append(interval);
  } else {
    Interval<Alpha> last;
    intervals.Get(intervals.Size()-1, last);
    if (last.Adjacent(interval)){
      last.end = interval.end;
      last.rc = interval.rc;
      intervals.Put(intervals.Size()-1, last);
    }  else {
      intervals.Append(interval);
    }
  }
}



template <class Alpha>
void Range<Alpha>::Clear()
{
  ordered = true;
  intervals.clean();
}

/*
4.2.3 Functions to be part of relations

*/

template <class Alpha>
inline size_t Range<Alpha>::Sizeof() const
{
  return sizeof( *this );
}

template <class Alpha>
inline int Range<Alpha>::Compare( const Attribute* arg ) const
{
  //    return 0; // Original implementation
  Range<Alpha>* other = (Range<Alpha>*) arg;
  if (!IsDefined() && !other->IsDefined())
    return 0;
  if (!IsDefined())
    return -1;
  if (!other->IsDefined())
    return 1;

  int cmp = 0;
  Interval<Alpha> my_interval, other_interval;
  int maxindex = MIN(GetNoComponents(),other->GetNoComponents());
  for( int i = 0; i < maxindex; i++ )
  {
    Get( i, my_interval );
    other->Get( i, other_interval );
    cmp = my_interval.CompareTo(other_interval);
    if ( cmp != 0 )
      return cmp;
  }
  if ( GetNoComponents() < other->GetNoComponents() )
    return -1;
  if ( GetNoComponents() > other->GetNoComponents() )
    return 1;
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
  Range *result;
  if( !this->IsDefined() ){
    result = new Range( 0 );
    result->del.isDefined = false;
  } else {
    assert( IsOrdered() );
    result = new Range( GetNoComponents() );
    result->del.isDefined = this->del.isDefined;
    result->StartBulkLoad();
    Interval<Alpha> interval;
    for( int i = 0; i < GetNoComponents(); i++ )
    {
      Get( i, interval );
      result->Add( interval );
    }
    result->EndBulkLoad( false );
  }
  return result;
}

template <class Alpha>
inline ostream& Range<Alpha>::Print( ostream &os ) const
{
  os << Range<Alpha>::BasicType()<<": ";
  if( !IsDefined() ){
    os << "UNDEFINED.";
  } else {
    os << " defined, contains " << GetNoComponents() << " intervals: [" << endl;
    Interval<Alpha> interval;
    for( int i = 0; i < GetNoComponents(); i++ )
    {
      Get( i, interval );
      os << "\t"; interval.Print( os ); os << endl;
    }
    os << "]." << endl;
  }
  return os << endl;
}

template <class Alpha>
inline size_t Range<Alpha>::HashValue() const
{
  if(!IsDefined()){
    return 0;
  }
  Interval<Alpha> min, max;
  intervals.Get(0,&min);
  intervals.Get(GetNoComponents()-1,&max);
  return static_cast<size_t>(   min.start.HashValue()
                              ^ max.end.HashValue()   ) ;
  return 0;
}

template <class Alpha>
inline void Range<Alpha>::CopyFrom( const Attribute* right )
{
  Clear();
  const Range<Alpha> *r = (const Range<Alpha>*)right;
  this->SetDefined(r->IsDefined());
  if(r->IsDefined()){
    assert( r->IsOrdered() );
    StartBulkLoad();
    Interval<Alpha> interval;
    for( int i = 0; i < r->GetNoComponents(); i++ ){
      r->Get( i, interval );
      Add( interval );
    }
    EndBulkLoad( false );
  }
}

template <class Alpha>
inline int Range<Alpha>::NumOfFLOBs() const
{
  return 1;
}

template <class Alpha>
inline Flob *Range<Alpha>::GetFLOB(const int i)
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
  if( !this->IsDefined() && !r.IsDefined() ) {
    return true;
  }
  if( !this->IsDefined() || !r.IsDefined() ) {
    return false;
  }
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
bool Range<Alpha>::operator!=( const Range<Alpha>& r ) const
{
  return !( *this == r );
}

template <class Alpha>
bool Range<Alpha>::Intersects( const Range<Alpha>& r ) const
{
  assert( IsDefined() );
  assert( IsValid() );
  assert( r.IsDefined() );
  assert( r.IsValid() );

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
bool Range<Alpha>::Intersects( const Interval<Alpha>& iv ) const
{
    if(!IsDefined() || IsEmpty()){
       return false;
    }

    // TODO: Use binary search to accelerate computations

    Interval<Alpha> this_iv;
    for(int i=0;i<GetNoComponents();i++){
        Get(i,this_iv);
        if(iv.Intersects(this_iv)){
          return true;
        }
    }
    return false;
}


template <class Alpha>
bool Range<Alpha>::Inside( const Range<Alpha>& r ) const
{
  assert( IsDefined() );
  assert( IsValid() );
  assert( r.IsDefined() );
  assert( r.IsValid() );

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
bool Range<Alpha>::Inside( const Interval<Alpha>& iv ) const
{
  if(IsEmpty()){
     return true;
  }
  Interval<Alpha> first;
  Interval<Alpha> last;
  Get(0,first);
  Get(GetNoComponents()-1,last);
  return iv.Contains(first) && iv.Contains(last);
}


/*
Returns the index of the interval containing a, or -1 if not found

*/
template<class Alpha>
int Range<Alpha>::GetIndexOf(const Alpha& alpha,
                             const bool ignoreCloseness /* = false */) const{

  assert( IsDefined() );
  assert( IsValid() );
  assert( alpha.IsDefined() );

  if( IsEmpty() ){
    return -1;
  }

  Interval<Alpha> midInterval;

  int first = 0;
  int  last = GetNoComponents() - 1;

  while (first <= last)
  {
    int mid = ( first + last ) / 2;
    Get( mid, midInterval );
    if( midInterval.Contains( alpha, ignoreCloseness ) )
    {
      return mid;
    } else if( midInterval.Before( alpha ) ){
      first = mid + 1;
    } else if( midInterval.After( alpha ) ) {
      last = mid - 1;
    } else {
      return mid;
    }
  }
  return -1;
}

template <class Alpha>
bool Range<Alpha>::Contains( const Alpha& a ) const
{
   return GetIndexOf(a) >=0;
}

template<class Alpha>
bool Range<Alpha>::Contains(const Interval<Alpha>&  iv,
                            const bool ignoreCloseness /* = false */) const {
   int index = GetIndexOf(iv.start, ignoreCloseness);
   if(index < 0){
      return false;
   }
   Interval<Alpha> miv;
   Get(index,miv);
   return  miv.Contains(iv, ignoreCloseness);
}





template <class Alpha>
bool Range<Alpha>::Before( const Range<Alpha>& r ) const
{
  assert(  IsDefined() );
  assert(  IsValid() );
  assert( !IsEmpty() );
  assert(  r.IsDefined() );
  assert(  r.IsValid() );
  assert( !r.IsEmpty() );

  Interval<Alpha> thisInterval, interval;
  Get( GetNoComponents() - 1, thisInterval );
  r.Get( 0, interval );

  return thisInterval.Before( interval );
}


template <class Alpha>
bool Range<Alpha>::Before( const Interval<Alpha>& iv ) const
{
   if(IsEmpty()){
      return false;
   }
   Interval<Alpha> last;
   Get(GetNoComponents()-1,last);
   return last.Before(iv);

}

template <class Alpha>
bool Range<Alpha>::Before( const Alpha& a ) const
{
  assert(  IsDefined() );
  assert(  IsValid() );
  assert( !IsEmpty() );
  assert(  a.IsDefined() );

  Interval<Alpha> thisInterval;
  Get( GetNoComponents() - 1, thisInterval );
  return thisInterval.Before( a );
}

template <class Alpha>
bool Range<Alpha>::After( const Alpha& a ) const
{
  assert(  IsDefined() );
  assert(  IsValid() );
  assert( !IsEmpty() );
  assert(  a.IsDefined() );

  Interval<Alpha> thisInterval;
  Get( 0, thisInterval );

  return thisInterval.After( a );
}

template <class Alpha>
void Range<Alpha>::Intersection( const Range<Alpha>& r,
                                 Range<Alpha>& result ) const
{
  assert( IsValid() );
  assert( r.IsValid() );

  result.Clear();
  if( !IsDefined() || !r.IsDefined() ){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  if( (GetNoComponents()==0) || (r.GetNoComponents()==0)){
     return;
  }

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
      if( thisInterval.start.Compare( &interval.end ) == 0 &&
          thisInterval.lc && interval.rc )
      {
        Interval<Alpha> newInterval( interval.end, interval.end, true, true );
        result.Add( newInterval );
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( thisInterval.end.Compare( &interval.start ) == 0 &&
               thisInterval.rc && interval.lc )
      {
        Interval<Alpha> newInterval( interval.start,
                                     interval.start, true, true );
        result.Add( newInterval );
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( thisInterval.start.Compare( &interval.start ) < 0 )
      {
        Interval<Alpha> newInterval( interval.start,
                                     thisInterval.end,
                                     interval.lc, thisInterval.rc );
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
          Interval<Alpha> newInterval( interval.start,
                                       interval.end,
                                       interval.lc && thisInterval.lc,
                                       interval.rc );
          if( newInterval.IsValid() )
            result.Add( newInterval );
          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else
        {
          assert( thisInterval.end.Compare( &interval.end ) < 0 );
          Interval<Alpha> newInterval( thisInterval.start,
                                       thisInterval.end,
                                       interval.lc && thisInterval.lc,
                                       thisInterval.rc );
          if( newInterval.IsValid() )
            result.Add( newInterval );
          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
      }
      else
      {
        Interval<Alpha> newInterval( thisInterval.start,
                                     interval.end,
                                     thisInterval.lc, interval.rc );
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
void Range<Alpha>::Intersection( const Interval<Alpha>& iv,
                                 Range<Alpha>& result ) const
{
   Interval<Alpha> tiv;
   result.Clear();
   result.StartBulkLoad();
   for(int i=0;i<GetNoComponents(); i++){
      Get(i,tiv);
      if(tiv.Intersects(iv)){
         Interval<Alpha> res;
         tiv.Intersection(iv, res);
         result.Add(res);
      }
   }
   result.EndBulkLoad();
}

/*
1.1 Auxiliary class ParallelRangeScan

This class makes an parallel Scan over two range values.
The returned intevals are sorted by their starting time and
leftClosed properties. If two intervals start at the same
time, the interval of r1 is the next result.


*/
template<class Alpha>
class ParallelRangeScan{
  public:
/*
1.1.1 Constructor

Both Ranges must be defined.


*/

     ParallelRangeScan(const Range<Alpha>* _r1, const Range<Alpha>* _r2):
        r1(_r1), r2(_r2), pos1(0), pos2(0){
      assert(r1->IsDefined());
      assert(r2->IsDefined());
    }

/*
1.1.2 ~next~

If there is a next interval, it's returned using the parameter. In this case, the
result will be true. If both ranges are exhausted, the parameter is not changed
and the result of this function is false

*/

     bool next(Interval<Alpha>& nextInterval){
       if( (pos1>=r1->GetNoComponents() ) && (pos2>=r2->GetNoComponents())){
         return false;
       }
       if(pos1>=r1->GetNoComponents()){
          r2->Get(pos2, nextInterval);
          pos2++;
          return true;
       }
       if(pos2>=r2->GetNoComponents()){
          r1->Get(pos1, nextInterval);
          pos1++;
          return true;
       }
       // both ranges have more entries
       Interval<Alpha> iv1;
       Interval<Alpha> iv2;
       r1->Get(pos1, iv1);
       r2->Get(pos2,iv2);
       if(iv2.StartsBefore(iv1)){
          nextInterval = iv2;
          pos2++;
       } else {
          nextInterval = iv1;
          pos1++;
       }
       return true;
     }

  private:
     const Range<Alpha>* r1;
     const Range<Alpha>* r2;
     int pos1;
     int pos2;
};



template<class Alpha>
void Range<Alpha>::Union(const Range<Alpha>& r, Range<Alpha>& result) const{

   assert(IsValid());
   assert(r.IsValid());
   result.Clear();
   if(!IsDefined() || !r.IsDefined()){
      result.SetDefined(false);
      return;
   }
   result.SetDefined(true);
   if(IsEmpty()){
     result.CopyFrom(&r);
     return;
   }
   if(r.IsEmpty()){
      result.CopyFrom(this);
      return;
   }

   // both ranges have elements
   Interval<Alpha> ivUnion; // current interval union
   Interval<Alpha> nextInterval;
   ParallelRangeScan<Alpha> scan(this, &r);
   // we already know that both intervals have entries, so
   // the first call of scan.next must return true
   bool ok = scan.next(ivUnion);
   assert(ok);
   while(scan.next(nextInterval)){
      if(nextInterval.Intersects(ivUnion)){
        ivUnion.Union(nextInterval);
      } else {
         result.MergeAdd(ivUnion);
         ivUnion = nextInterval;
      }

   }
   result.MergeAdd(ivUnion);
}




template <class Alpha>
void Range<Alpha>::Union( const Interval<Alpha>& iv,
                          Range<Alpha>& result ) const
{
  Interval<Alpha> enlargedInterval(iv);
  Interval<Alpha> tiv;
  bool done = false;

  result.Clear();
  for(int i=0;i< this->GetNoComponents();i++){
     Get(i,tiv);
     if(tiv.Before(iv) ){
        result.Add(tiv);
     } else if(tiv.After(enlargedInterval)){
        if(!done){
           result.Add(enlargedInterval);
           done = true;
        }
        result.Add(tiv);
     } else {
       enlargedInterval.Union(tiv);
     }
  }
  if(!done){
    result.Add(enlargedInterval);
  }
}


template <class Alpha>
void Range<Alpha>::Minus( const Range<Alpha>& r, Range<Alpha>& result ) const
{
  assert( IsValid() );
  assert( r.IsValid() );

  result.Clear();
  if( !IsDefined() || !r.IsDefined() ){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );

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
          Interval<Alpha> newInterval( thisInterval.start,
                                       thisInterval.start, true, true );
          result.Add( newInterval );
        }
        if( thisInterval.rc && !interval.rc )
        {
          Interval<Alpha> newInterval( thisInterval.end,
                                       thisInterval.end, true, true );
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
            Interval<Alpha> newInterval( thisInterval.start,
                                         thisInterval.start, true, true );
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
            Interval<Alpha> newInterval( thisInterval.start,
                                         interval.start,
                                         thisInterval.lc, !interval.lc );
            if( newInterval.IsValid() )
              result.Add( newInterval );
          }
          else
          {
            Interval<Alpha> newInterval( *start, interval.start,
                                         lc, !interval.lc );
            if( newInterval.IsValid() )
              result.Add( newInterval );
            delete start; start = NULL;
            delete end; end = NULL;
            lc = false; rc = false;
          }

          if( thisInterval.rc && !interval.rc )
          {
            Interval<Alpha> newInterval( thisInterval.end,
                                         thisInterval.end, true, true );
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
            Interval<Alpha> newInterval( thisInterval.start,
                                         interval.start,
                                         thisInterval.lc, !interval.lc );
            if( newInterval.IsValid() )
              result.Add( newInterval );
          }
          else
          {
            assert( end->Compare( &thisInterval.end ) == 0 &&
                    rc == thisInterval.rc );

            Interval<Alpha> newInterval( *start, interval.start,
                                         lc, !interval.lc        );
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
              Interval<Alpha> newInterval( thisInterval.end,
                                           thisInterval.end, true, true );
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
          assert( (start == NULL) & (end == NULL) );

          if( thisInterval.lc && !interval.lc )
          {
            Interval<Alpha> newInterval( thisInterval.start,
                                         thisInterval.start, true, true );
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
                  ( interval.start.Compare( start ) == 0 &&
                    interval.lc && !lc ) )
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
              Interval<Alpha> newInterval( thisInterval.start,
                                           interval.start,
                                           thisInterval.lc, !interval.lc );
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
              Interval<Alpha> newInterval( interval.end,
                                           interval.end, true, true );
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
              Interval<Alpha> newInterval( thisInterval.start,
                                           interval.start,
                                           thisInterval.lc, !interval.lc );
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
    result.Add( thisInterval );

    if( ++i < GetNoComponents() )
      Get( i, thisInterval );
  }
  result.EndBulkLoad( false );
}


template <class Alpha>
void Range<Alpha>::Minus( const Interval<Alpha>& iv,
                          Range<Alpha>& result ) const
{
   Interval<Alpha> tiv;
   Interval<Alpha> iv_1;
   Interval<Alpha> iv_2;
   result.Clear();
   for(int i=0;i<GetNoComponents(); i++){
      Get(i,tiv);
      if(tiv.Before(iv)){
         result.Add(tiv);
      } else if(tiv.After(iv)){
         result.Add(tiv);
      } else {
         int num = tiv.Minus(iv,iv_1,iv_2);
         if(num>0){
            result.Add(iv_1);
         }
         if(num>1){
            result.Add(iv_2);
         }
      }
   }
}


template <class Alpha>
void Range<Alpha>::Minus(const Range<Alpha>& r){
   // TODO: improve this naive implementation to avoid a lot of work
   Range<Alpha> tmp(*this);
   tmp.Minus(r, *this);
   tmp.Destroy();
}


template <class Alpha>
void Range<Alpha>::Minus(const Interval<Alpha>& i){
   // TODO: improve this naive implementation to avoid a lot of work
   Range<Alpha> tmp(*this);
   tmp.Minus(i, *this);
   tmp.Destroy();
}




template <class Alpha>
void Range<Alpha>::Maximum( Alpha& result ) const
{
  result.SetDefined( IsDefined() );
  if( !IsDefined() ){
    return;
  }
  assert( IsValid() );
  if( IsEmpty() ) {
    result.SetDefined( false );
  } else {
    Interval<Alpha> interval;
    Get( GetNoComponents()-1, interval );
    result.CopyFrom( &interval.end );
  }
}

template <class Alpha>
void Range<Alpha>::Minimum( Alpha& result ) const
{
  result.SetDefined( IsDefined() );
  if( !IsDefined() ){
    return;
  }
  assert( IsValid() );
  if( IsEmpty() ) {
    result.SetDefined( false );
  } else {
    Interval<Alpha> interval;
    Get( 0, interval );
    result.CopyFrom( &interval.start );
  }
}

template <class Alpha>
int Range<Alpha>::GetNoComponents() const
{
  assert( IsDefined() );
  return intervals.Size();
}

template <class Alpha>
void Range<Alpha>::RBBox( Range<Alpha>& result ) const
{
  result.Clear();
  result.SetDefined( IsDefined() );
  if( !IsDefined() ){
    return;
  }
  assert( IsValid() );
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
  if( !IsDefined() )
    return true;

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
    if( (!lastInterval.Disjoint( interval )) &&
        (!lastInterval.Adjacent( interval )) )
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
  result.Clear();
  result.SetDefined( IsDefined() );
  if(!IsDefined()){
    return;
  }

  assert( IsOrdered() );
  result.StartBulkLoad();

  Interval<Alpha> ii, ji;
  int i = 0, j;
  bool jcont = true;

  while( i < GetNoComponents() )
  {
    Get( i, ii );

    j = i + 1;
    jcont = true;

    Interval<Alpha> copyii( ii );
    while( j < GetNoComponents() && jcont )
    {
      Get( j, ji );
      if( copyii.Adjacent( ji ) )
      {
        copyii.end = ji.end;
        copyii.rc = ji.rc;
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
  assert( i.timeInterval.IsValid() );
  timeInterval = i.timeInterval;
  return *this;
}

template <class Alpha>
bool TemporalUnit<Alpha>::operator==( const TemporalUnit<Alpha>& i ) const
{
  assert( timeInterval.IsValid() && i.timeInterval.IsValid() );
  return( timeInterval == i.timeInterval);
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
bool TemporalUnit<Alpha>::StartsBefore( const TemporalUnit<Alpha>& i ) const
{
  assert( IsValid() && i.IsValid() );

  return ( timeInterval.StartsBefore(i.timeInterval) );
}

template <class Alpha>
bool TemporalUnit<Alpha>::EndsAfter( const TemporalUnit<Alpha>& i ) const
{
  assert( IsValid() && i.IsValid() );

  return ( timeInterval.EndsAfter(i.timeInterval) );
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
  Attribute(true),
  canDestroy( false ),
  ordered( true ),
  units( n )
{
  del.refs=1;
  del.SetDelete();
  del.isDefined = true;
}

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
  assert( IsDefined() );
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
void Mapping<Unit, Alpha>::EndBulkLoad( const bool sort, const bool checkvalid )
{
  assert( !ordered );
  if( !IsDefined() ){
    units.clean();
  } else if( sort ){
    units.Sort( UnitCompare<Unit> );
  }
  ordered = true;
  units.TrimToSize();
  if( checkvalid && !IsValid() ){
    SetDefined( false );
    cerr << __PRETTY_FUNCTION__<< " found invalid range and marked it "
        << "undefined!"<< endl;
//     assert(isvalid);
  }
}

template <class Unit, class Alpha>
bool Mapping<Unit, Alpha>::IsEmpty() const
{
  return !IsDefined() || (units.Size() == 0);
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::Get( const int i, Unit &unit ) const
{
  assert( IsDefined() );
  assert(i>=0);
  assert(i<units.Size());
  bool ok = units.Get( i, unit );
  if(!ok){
    cout << "Problem in getting data from " << units << endl;
    assert(ok);
  }
  if ( !unit.IsValid() )
  {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
      << " Get(" << i << ", Unit): Unit is invalid:";
    unit.Print(cout); cout << endl;
    assert( unit.IsValid());
  }
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::Add( const Unit& unit )
{
  assert( IsDefined() );
  if ( !unit.IsDefined() || !unit.IsValid() )
  {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
      << " Add(Unit): Unit is undefined or invalid:";
    unit.Print(cout); cout << endl;
    assert( false );
  }
  units.Append( unit );
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::MergeAdd( const Unit& unit )
{
  assert( IsDefined() );
  Unit lastunit;
  int size = units.Size();
  if ( !unit.IsDefined() || !unit.IsValid() )
  {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
      << " MergeAdd(Unit): Unit is undefined or invalid:";
    unit.Print(cout); cout << endl;
    assert( false );
  }

  if (size > 0) {
      units.Get( size - 1, &lastunit );
      if (lastunit.EqualValue(unit) &&
      (lastunit.timeInterval.end == unit.timeInterval.start) &&
      (lastunit.timeInterval.rc || unit.timeInterval.lc)) {
          lastunit.timeInterval.end = unit.timeInterval.end;
          lastunit.timeInterval.rc = unit.timeInterval.rc;
          if ( !lastunit.IsValid() )
          {
            cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
              << "\nMapping::MergeAdd(): lastunit is invalid:";
            lastunit.Print(cout); cout << endl;
            assert( false );
          }
          units.Put(size - 1, lastunit);
      }
      else {
          units.Append( unit );
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
  units.clean();
  this->del.isDefined = true;
}

/*
4.4.3 Functions to be part of relations

*/

template <class Unit, class Alpha>
inline size_t Mapping<Unit, Alpha>::Sizeof() const
{
  return sizeof( *this );
}

template <class Unit, class Alpha>
inline int Mapping<Unit, Alpha>::Compare( const Attribute *arg ) const
{
   Mapping<Unit,Alpha>* map2 = (Mapping<Unit,Alpha>*) arg;
   if( !IsDefined() && !map2->IsDefined() )
     return 0;
   if( !IsDefined() &&  map2->IsDefined() )
     return -1;
   if(  IsDefined() && !map2->IsDefined() )
     return 1;
   size_t size1 = units.Size();
   size_t size2 = map2->units.Size();
   size_t index = 0;
   Unit u1;
   Unit u2;
   int cmp;
   while( (index < size1) && (index < size2)){
      units.Get(index,&u1);
      map2->units.Get(index,&u2);
      cmp = u1.Compare(&u2);
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
inline Attribute* Mapping<Unit, Alpha>::Clone() const
{
  Mapping<Unit, Alpha> *result;

  if( !IsDefined() ){
    result = new Mapping<Unit, Alpha>( 0 );
    result->SetDefined( false );
    return result;
  }
  result = new Mapping<Unit, Alpha>( GetNoComponents() );
  result->SetDefined( true );

  assert( IsOrdered() );

  if(GetNoComponents()>0){
     result->units.resize(GetNoComponents());
  }

  result->StartBulkLoad();
  Unit unit;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    result->Add( unit );
  }
  result->EndBulkLoad( false );
  return result;
}

template <class Unit, class Alpha>
inline ostream& Mapping<Unit, Alpha>::Print( ostream &os ) const
{
  if( !IsDefined() )
  {
    return os << "("<< Mapping<Unit,Alpha>::BasicType() <<": undefined)";
  }
  os << "("<< Mapping<Unit,Alpha>::BasicType()
     <<": defined, contains " << GetNoComponents() << " units: ";
  for(int i=0; i<GetNoComponents(); i++)
  {
    Unit unit;
    Get( i , unit );
    os << "\n\t";
    unit.Print(os);
  }
  os << "\n)" << endl;
  return os;
}

template <class Unit, class Alpha>
inline size_t Mapping<Unit, Alpha>::HashValue() const
{
  if(!IsDefined()){
    return 0;
  }
  Unit min, max;
  units.Get(0,&min);
  units.Get(GetNoComponents()-1,max);
  return static_cast<size_t>(   min.HashValue()
                              ^ max.HashValue()   ) ;
  return 0;
}

template <class Unit, class Alpha>
inline void Mapping<Unit, Alpha>::CopyFrom( const Attribute* right )
{
  const Mapping<Unit, Alpha> *r = (const Mapping<Unit, Alpha>*)right;
  Clear();
  SetDefined( r->IsDefined() );
  if( !r->IsDefined() ){
    return;
  }

  assert( r->IsOrdered() );
  StartBulkLoad();
  Unit unit;
  for( int i = 0; i < r->GetNoComponents(); i++ )
  {
    r->Get( i, unit );
    Add( unit );
  }
  EndBulkLoad( false );
  this->SetDefined(r->IsDefined());
}

template <class Unit, class Alpha>
inline void
Mapping<Unit, Alpha>::Restrict( const vector< pair<int, int> >& intervals )
{
  if( !IsDefined() ){
    Clear();
    SetDefined( false );
  }
  units.Restrict( intervals, units );
}

template <class Unit, class Alpha>
inline int Mapping<Unit, Alpha>::NumOfFLOBs() const
{
  return 1;
}

template <class Unit, class Alpha>
inline Flob *Mapping<Unit, Alpha>::GetFLOB(const int i)
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
  if( !IsDefined() && !r.IsDefined() )
    return true;
  if( !IsDefined() || !r.IsDefined() )
    return false;

  assert( IsOrdered() && r.IsOrdered() );

  if( GetNoComponents() != r.GetNoComponents() )
    return false;

  bool result = true;
  Unit thisunit, unit;

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

template <class Unit, class Alpha>
bool Mapping<Unit, Alpha>::operator!=( const Mapping<Unit, Alpha>& r ) const
{
  return !( *this == r );
}

template <class Unit, class Alpha>
int Mapping<Unit, Alpha>::GetNoComponents() const
{
  assert( IsDefined() );
  return units.Size();
}

template <class Unit, class Alpha>
bool Mapping<Unit, Alpha>::IsValid() const
{
  if( !IsDefined() )
    return true;

  if( canDestroy )
    return false;

  if( !IsOrdered() )
    return false;

  if( IsEmpty() )
    return true;

  bool result = true;
  Unit lastunit, unit;

  Get( 0, lastunit );
  if ( !lastunit.IsValid() )
  {
    cerr << "Mapping<Unit, Alpha>::IsValid(): "
            "unit is invalid: i=0" << endl;
    return false;
  }
  if ( GetNoComponents() == 1 ){
    return true;
  }

  for( int i = 1; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    if( !unit.IsValid() )
    {
      result = false;
      cerr << "Mapping<Unit, Alpha>::IsValid(): "
              "unit is invalid: i=" << i << endl;
      return false;
    }
    if(lastunit.timeInterval.end > unit.timeInterval.start){
       cerr << "Units are not ordered by time" << endl;
       cerr << "lastUnit.timeInterval =  "; lastunit.timeInterval.Print(cerr);
       cerr << endl;
       cerr << "unit.timeInterval =  "; unit.timeInterval.Print(cerr);
       cerr << endl;
       return false;
    }


    if( (!lastunit.timeInterval.Disjoint(unit.timeInterval)) )
    {
      result = false;
      cerr << "Mapping<Unit, Alpha>::IsValid(): "
              "unit and lastunit not disjoint: i=" << i << endl;
      cerr << "\n\tlastunit = "; lastunit.timeInterval.Print(cerr);
      cerr << "\n\tunit     = "; unit.timeInterval.Print(cerr); cerr << endl;
      return false;
    }
    lastunit = unit;
  }
  return result;
}

template <class Unit, class Alpha>
int Mapping<Unit, Alpha>::Position( const Instant& t ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( t.IsDefined() );

  int first = 0, last = units.Size() - 1;
  Instant t1 = t;

  while (first <= last)
  {
    int mid = ( first + last ) / 2;

    if( (mid < 0) || (mid >= units.Size()) )
      return -1;

    Unit midUnit;
    units.Get( mid, &midUnit );

    if( midUnit.timeInterval.Contains(t1) )
      return mid;
    else  //not contained
      if( ( t1 > midUnit.timeInterval.end ) ||
          ( t1 == midUnit.timeInterval.end ) )
        first = mid + 1;
      else if( ( t1 < midUnit.timeInterval.start ) ||
               ( t1 == midUnit.timeInterval.start ) )
        last = mid - 1;
      else
        return -1; //should never be reached.
    }
    return -1;
}

template<class Unit, class Alpha>
Mapping<Unit,Alpha>* Mapping<Unit,Alpha>::compress() const{
    Mapping<Unit,Alpha>* result = new Mapping<Unit,Alpha>(GetNoComponents());
    if(!IsDefined()){
      result->SetDefined(false);
      return result;
    }
    result->SetDefined(true);
    Unit u;
    for(int i=0;i<GetNoComponents();i++){
       Get(i,u);
       result->MergeAdd(u);
    }
    return result;
}



template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::AtInstant( const Instant& t,
                                      Intime<Alpha>& result ) const
{
  if( !IsDefined() || !t.IsDefined() ) {
    result.SetDefined( false );
    return;
  }
  assert( IsOrdered() );

  int pos = Position( t );

  if( pos == -1 )  // not contained in any unit
    result.SetDefined( false );
  else
  {
    Unit posUnit;
    units.Get( pos, &posUnit );

    result.SetDefined( true );
    posUnit.TemporalFunction( t, result.value );
    result.instant.CopyFrom( &t );
  }
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::TemporalFunction( const Instant& t,
                                             Alpha& result,
                                             bool ignoreLimits) const
{
  if( !IsDefined() || !t.IsDefined() ) {
    result.SetDefined( false );
    return;
  }
  assert( IsOrdered() );
  int pos = Position( t );
  if( pos == -1 )  // not contained in any unit
    result.SetDefined( false );
  else
  {
    Unit posUnit;
    units.Get( pos, &posUnit );
    result.SetDefined( true );
    posUnit.TemporalFunction( t, result, ignoreLimits );
  }
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::AtPeriods( const Periods& periods,
                                      Mapping<Unit, Alpha>& result ) const
{
  result.Clear();
  if( !IsDefined() || !periods.IsDefined() ) {
    result.SetDefined( false );
    return;
  }
  assert( IsOrdered() );
  assert( periods.IsOrdered() );

  if( IsEmpty() || periods.IsEmpty() )
    return;

  if( IsMaximumPeriods(periods) )
  { // p is [begin of time, end of time]. Copy the input into result.
    result.CopyFrom(this);
    return;
  }
  result.StartBulkLoad();

  Unit unit;
  Interval<Instant> interval;
  int i = 0, j = 0;
  Get( i, unit );
  periods.Get( j, interval );

  while( 1 )
  {
    if( unit.timeInterval.Before( interval ) )
    {
      if( ++i == GetNoComponents() )
        break;
      Get( i, unit );
    }
    else if( interval.Before( unit.timeInterval ) )
    {
      if( ++j == periods.GetNoComponents() )
        break;
      periods.Get( j, interval );
    }
    else
    {
      Unit r(1);
      unit.AtInterval( interval, r );
      result.Add( r );

      if( interval.end == unit.timeInterval.end )
      {
        if( interval.rc == unit.timeInterval.rc )
        {
          if( ++i == GetNoComponents() )
            break;
          Get( i, unit );
          if( ++j == periods.GetNoComponents() )
            break;
          periods.Get( j, interval );
        }
        else if( interval.rc == true )
        {
          if( ++i == GetNoComponents() )
            break;
          Get( i, unit );
        }
        else
        {
          assert( unit.timeInterval.rc == true );
          if( ++j == periods.GetNoComponents() )
            break;
          periods.Get( j, interval );
        }
      }
      else if( interval.end > unit.timeInterval.end )
      {
        if( ++i == GetNoComponents() )
          break;
        Get( i, unit );
      }
      else
      {
        assert( interval.end < unit.timeInterval.end );
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
  assert( IsDefined() && t.IsDefined() && IsOrdered() );

  int pos = Position(t);

  if( pos == -1 )  //not contained in any unit
    return false;
  return true;
}

template <class Unit, class Alpha>
bool Mapping<Unit, Alpha>::Present( const Periods& t ) const
{
  assert( IsDefined() && t.IsOrdered() && IsOrdered() );

  Periods defTime( 0 );
  DefTime( defTime );

  return t.Intersects( defTime );
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::DefTime( Periods& r ) const
{
  r.Clear();
  r.SetDefined( IsDefined() );
  if( !IsDefined() ){
    return;
  }
  assert( IsOrdered() );

  Unit unit;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    r.MergeAdd( unit.timeInterval );
  }
}

template <class Unit, class Alpha>
template <class Beta>
bool Mapping<Unit, Alpha>::Passes( const Beta& val ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( val.IsDefined() );

  Unit unit;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    if( unit.Passes( val ) )
      return true;
  }
  return false;
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::At( const Alpha& val,
                               Mapping<Unit, Alpha>& result ) const
{
  result.Clear();

  if( !IsDefined() || !val.IsDefined() ){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );

  assert( IsOrdered() );
  result.StartBulkLoad();

  Unit unit;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    Unit resUnit;
    if( unit.At( val, resUnit ) )
      result.Add( resUnit );
  }

  result.EndBulkLoad( false );

// VTA - The merge of the result is not implemented yet.
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::Initial( Intime<Alpha>& result ) const
{
  if( !IsDefined() ){
    result.SetDefined( false );
    return;
  }

  assert( IsOrdered() );

  if( IsEmpty() ){
    result.SetDefined( false );
  } else
  {
    Unit unit;
    units.Get( 0, &unit );

    result.SetDefined( true );
    unit.TemporalFunction( unit.timeInterval.start, result.value, true );
    result.instant.CopyFrom( &unit.timeInterval.start );
  }
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::Final( Intime<Alpha>& result ) const
{
  if( !IsDefined() ){
    result.SetDefined( false );
    return;
  }

  assert( IsOrdered() );

  if( IsEmpty() ){
    result.SetDefined( false );
  } else
  {
    Unit unit;
    bool ok = units.Get( GetNoComponents()-1, unit );
    assert(ok);
    result.SetDefined( true );
    unit.TemporalFunction( unit.timeInterval.end, result.value, true );
    result.instant.CopyFrom( &unit.timeInterval.end );
  }
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::Resize(size_t n){
   if (n>0)
   {
     units.resize(n);
   } else
   {
     units.clean();
   }
}

template <class Unit, class Alpha>
void Mapping<Unit, Alpha>::TrimToSize(){
   units.TrimToSize();
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
   result.SetDefined(true);
   // resize for two additional units (estimated a good choice)
   result.Resize(units.Size()+2);

   Instant CI(u.timeInterval.start); // current instant
   bool    Cc = u.timeInterval.lc;   // current closed
   Unit unit;
   result.canDestroy=false;
   int size = units.Size();

   // we have to connect units holding the same value, so
   // we hold a unit and extend it if it is possible

   result.StartBulkLoad();

   for(int i=0; i<size;i++){
      units.Get(i,&unit);
      // check whether a gap exits between the currect position in time
      // and this unit
      if(CI<unit.timeInterval.start || // time is before
         (( CI==unit.timeInterval.start && !Cc && !unit.timeInterval.lc))){
         // create a Unit filling the gap
         Interval<Instant> interval(CI,unit.timeInterval.start,
                                    !Cc,!unit.timeInterval.lc);
         Unit gap(interval,u.constValue);
         result.MergeAdd(gap); // append the gap filling unit
      }
      Unit st(unit);
      result.MergeAdd(st);
      // change the current instant and closed information
      CI = unit.timeInterval.end;
      Cc = unit.timeInterval.rc;
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

template<class Unit, class Alpha>
void Mapping<Unit,Alpha>::timeMove(const DateTime& duration,
                                   Mapping<Unit,Alpha>& result) const{
  assert(duration.GetType()==datetime::durationtype);
  if(!IsDefined()){
     result.SetDefined(false);
     return;
  }
  result.SetDefined(true);
  result.StartBulkLoad();
  Unit unit;
  for(int i=0;i<GetNoComponents();i++){
    Get(i,unit);
    unit.timeInterval.start += duration;
    unit.timeInterval.end += duration;
    result.Add(unit);
  }
  result.EndBulkLoad();
}


template<class Unit, class Alpha>
void Mapping<Unit,Alpha>::moveTo(const DateTime& instant,
                                 Mapping<Unit,Alpha>& result) const{

  assert(instant.GetType()==datetime::instanttype);
  result.Clear();
  if(!IsDefined()){
    result.SetDefined(false);
    return;
  }
  if(IsEmpty()){
    result.SetDefined(true);
    return;
  }
  Unit unit;
  Get(0,unit);
  DateTime dur = instant - unit.timeInterval.start;
  timeMove(dur, result);
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

  if( !range->IsDefined() ){
    return nl->SymbolAtom( Symbol::UNDEFINED() );
  }

  if( range->IsEmpty() )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    assert( range->IsOrdered() );
    ListExpr l = nl->TheEmptyList(), lastElem=nl->TheEmptyList(), intervalList;

    for( int i = 0; i < range->GetNoComponents(); i++ )
    {
      Interval<Alpha> interval;
      range->Get( i, interval );
      Alpha *start = (Alpha*)&interval.start,
            *end = (Alpha*)&interval.end;
      intervalList = nl->FourElemList(
              OutFun( nl->TheEmptyList(), SetWord(start) ),
              OutFun( nl->TheEmptyList(), SetWord(end) ),
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
template <class Alpha, Word (*InFun)( const ListExpr, const ListExpr,
                                      const int, ListExpr&, bool&     )>
Word InRange( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Range<Alpha>* range;

  if ( listutils::isSymbolUndefined( instance ) )
  {
    range = new Range<Alpha>( 0 );
    range->SetDefined( false );
    correct = true;
    return SetWord( range );
  }

  range = new Range<Alpha>( 0 );
  range->SetDefined( true );
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
    return nl->SymbolAtom( Symbol::UNDEFINED() );
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

  if ( listutils::isSymbolUndefined( instance ) )
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
      delete instant;
      delete value;
      return SetWord( intime );
    }
    delete instant;
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
  return intime->Clone();
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
    return (nl->SymbolAtom(Symbol::UNDEFINED()));

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

  if( nl->ListLength( instance ) == 2 )
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
  else if ( listutils::isSymbolUndefined( instance ) )
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
  return (SetWord( new ConstTemporalUnit<Alpha>(false) ));
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
  if(! m->IsDefined()){
    return nl->SymbolAtom(Symbol::UNDEFINED());
  } else
  if( m->IsEmpty() )
    return (nl->TheEmptyList());
  else
  {
    assert( m->IsOrdered() );
    ListExpr l = nl->TheEmptyList();
    ListExpr lastElem = nl->TheEmptyList();
    ListExpr unitList;

    for( int i = 0; i < m->GetNoComponents(); i++ )
    {
      Unit unit;
      m->Get( i, unit );
      unitList = OutUnit( nl->TheEmptyList(), SetWord(&unit) );
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
  int numUnits = 0;
  if(nl->AtomType(instance)==NoAtom){
    numUnits = nl->ListLength(instance);
  }
  Mapping* m = new Mapping( numUnits );
  correct = true;
  int unitcounter = 0;
  string errmsg;

  m->StartBulkLoad();

  ListExpr rest = instance;
  if (nl->AtomType( rest ) != NoAtom)
  { if(listutils::isSymbolUndefined(rest)){
       m->EndBulkLoad();
       m->SetDefined(false);
       return SetWord( Address( m ) );
    } else {
      correct = false;
      m->Destroy();
      delete m;
      return SetWord( Address( 0 ) );
    }
  }
  else while( !nl->IsEmpty( rest ) )
  {
    ListExpr first = nl->First( rest );
    rest = nl->Rest( rest );

    Unit *unit = (Unit*)InUnit( nl->TheEmptyList(), first,
                                errorPos, errorInfo, correct ).addr;

    if ( !correct )
    {
      errmsg = "InMapping(): Representation of Unit "
          + int2string(unitcounter) + " is wrong.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      m->Destroy();
      delete m;
      return SetWord( Address(0) );
    }
    if( /* correct && (...)*/ !unit->IsDefined() || !unit->IsValid() )
    {
      errmsg = "InMapping(): Unit " + int2string(unitcounter) + " is undef.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      correct = false;
      delete unit;
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
  ((Mapping *)w.addr)->DeleteIfAllowed();
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
  ((CcBool*)result.addr)->Set( true, ((Mapping*)args[0].addr)->IsEmpty() );
  return 0;
}

template <class Unit>
int UnitIsEmpty( Word* args, Word& result,
                 int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool*)result.addr)->Set( true, !((Unit*)args[0].addr)->IsDefined() );
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
  ((CcBool*)result.addr)->Set(
      true,
      *((Mapping*)args[0].addr) == *((Mapping*)args[1].addr)
     );
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
  ((CcBool*)result.addr)->Set(
       true,
       *((Mapping*)args[0].addr) != *((Mapping*)args[1].addr)
      );
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
6.5 Value mapping functions of operator ~uval~

Return a constant unit's value

*/
template <class Alpha,class Beta>
int UIntimeVal( Word* args, Word& result, int message, Word& local, Supplier s )
{

  result = qp->ResultStorage( s );
  Alpha* i = (Alpha*)args[0].addr;
  if( i->IsDefined())
    ((Beta*)result.addr)->Set(true,i->constValue.GetValue());
  else
    ((Beta*)result.addr)->SetDefined( false );

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
  Mapping* m = ((Mapping*)args[0].addr);
  if( m->IsDefined() ){
    ((CcInt*)result.addr)->Set( true,
                              ((Mapping*)args[0].addr)->GetNoComponents() );
  } else {
    ((CcInt*)result.addr)->Set( false, 0 );
  }
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
Value mapping functions of operator ~when~

*/
template <class Mapping>
int MappingWhen( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MBool* mb= static_cast<MBool*>(args[1].addr);
  MBool mbTrue(0);
  CcBool tru(true, true);
  mb->At(tru, mbTrue);
  Periods p(0);
  mbTrue.DefTime(p);
  ((Mapping*)args[0].addr)->AtPeriods( p, *((Mapping*)result.addr) );
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

  if( !m->IsDefined() || !inst->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else
    ((CcBool *)result.addr)->Set( true, m->Present( *inst ) );

  return 0;
}

template <class Mapping>
int MappingPresent_p( Word* args, Word& result,
                      int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Mapping *m = ((Mapping*)args[0].addr);
  Periods* periods = ((Periods*)args[1].addr);

  if( !m->IsDefined() || !periods->IsDefined() || periods->IsEmpty() )
    ((CcBool *)result.addr)->Set( false, false );
  else
    ((CcBool *)result.addr)->Set( true, m->Present( *periods ) );

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

  if( !m->IsDefined() || !val->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else
    ((CcBool *)result.addr)->Set( true, m->Passes( *val ) );

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

  m->At( *val, *pResult );

  return 0;
}

/*
6.15 Value mapping functions of operator ~units~

*/
template <class Mapping, class Unit>
class UnitsLocalInfo
{
  public:
    int unitIndex;      // current item index (unit to be processed next)
    int noUnits;        // no of all units within the mapping m
    double totalSize;   // attrExtSize of the mapping m (incl. FLOBs)
    double attrSize;    // attrSize (without FLOBs)
    double attrSizeExt; // average attrSizeExt (withFlob)
    bool progressinitialized;
    double feedunittime;
    bool sonIsObjectNode;

    UnitsLocalInfo(Mapping *m, const Supplier &s) :
        unitIndex(0), noUnits(-1), totalSize(0.0), attrSize(0.0),
        attrSizeExt(0.0), progressinitialized(false), feedunittime(0.0001),
        sonIsObjectNode(false) {
        Unit u(false);
        attrSizeExt = ((double)(u.Sizeof()));
        attrSize = attrSizeExt;
        if(m->IsDefined()) {
          noUnits = m->GetNoComponents();
          if(noUnits > 0) { // compute and add FLOB size
            for(int f = 0; f < m->NumOfFLOBs(); f++) {
              totalSize += m->GetFLOB(f)->getSize();
            }
            attrSizeExt += ( totalSize / ((double)noUnits) );
          }
        } else {
          unitIndex = -1; // will provoke "CANCEL" on first REQUEST
        }
        feedunittime = 0.00042 * attrSizeExt;
        sonIsObjectNode = qp->IsObjectNode(qp->GetSupplierSon(s,0));
    }

    ~UnitsLocalInfo() {}
};

template <class Mapping, class Unit>
int MappingUnits(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Mapping* m = static_cast<Mapping*>(args[0].addr);
  UnitsLocalInfo<Mapping, Unit> *localinfo =
                      static_cast<UnitsLocalInfo<Mapping, Unit> *>(local.addr);

#ifdef USE_PROGRESS
  ProgressInfo *pRes = (ProgressInfo*) result.addr;
  ProgressInfo p1;
#endif

  switch( message )
  {
    case OPEN:
      if(localinfo){
        delete localinfo;
      }
      localinfo = new UnitsLocalInfo<Mapping, Unit>(m,  s);
      local = SetWord(localinfo);
      return 0;

    case REQUEST:

      if( local.addr == 0 ) {
        return CANCEL;
      } else if( localinfo->unitIndex < localinfo->noUnits ) {
        Unit unit(true);
        m->Get( localinfo->unitIndex++, unit );
        result = SetWord( unit.Clone());
        return YIELD;
      } else {
        return CANCEL;
      }

    case CLOSE:
#ifndef USE_PROGRESS
      if( local.addr != 0 )
      {
        delete localinfo;
        local = SetWord(Address(0));
      }
#endif
      return 0;

#ifdef USE_PROGRESS
    case REQUESTPROGRESS:
      if(local.addr == 0) {
        return CANCEL;
      }
      if(    !localinfo->sonIsObjectNode
          && qp->RequestProgress(args[0].addr, &p1) ) {
        // the son is a computed result node
        // just copy everything
        pRes->CopyBlocking(p1);
        pRes->Time = p1.Time;
      } else {
        // the son is a database object or constant
        pRes->BTime = 0.00001; // no blocking time
        pRes->BProgress = 1.0; // non-blocking
        pRes->Time = 0.00001;  // (almost) zero runtime
      }
      if(!localinfo->progressinitialized){
        pRes->Card = (double) max(0,localinfo->noUnits); // cardinality
        pRes->Size = localinfo->attrSize;                // total size
        pRes->SizeExt = localinfo->attrSizeExt;          // size w/o FLOBS
        pRes->noAttrs = 1;                               //no of attributes
        pRes->attrSize = &(localinfo->attrSize);
        pRes->attrSizeExt = &(localinfo->attrSizeExt);
        localinfo->progressinitialized = true;
        pRes->sizesChanged = true;  //sizes have been recomputed
        localinfo->progressinitialized = true;
      } else {
        pRes->Card = (double) max(0,localinfo->noUnits); // cardinality
        pRes->sizesChanged = false;
      }
      if(    (localinfo->noUnits > 0)
          && (localinfo->unitIndex < localinfo->noUnits) ){
        pRes->Progress =   ((double)localinfo->unitIndex)
                         / ((double)localinfo->noUnits);
        pRes->Time +=   ((double)(localinfo->noUnits - localinfo->unitIndex))
                      * localinfo->feedunittime;
      } else {
        pRes->Progress = 1.0;
        pRes->Time = 0.00001;
      }
      return YIELD;

    case CLOSEPROGRESS:
      if( local.addr != 0 )
      {
        delete localinfo;
        local = SetWord(Address(0));
      }
      return 0;
#endif

  }
  /* should not happen */
  return -1;
}

template <class Mapping, class Unit>
int MappingGetUnit
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Mapping* m = static_cast<Mapping*>(args[0].addr);
  result = qp->ResultStorage( s );

  CcInt* index = ((CcInt*)args[1].addr);
  Unit* pResult = ((Unit*)result.addr);

  if(!m->IsDefined() || m->GetNoComponents()==0 || index->GetIntval() < 0 ||
      index->GetIntval() >= m->GetNoComponents())
  {
    pResult->SetDefined(false);
    return 0;
  }
  m->Get( index->GetIntval(), *pResult );
  return 0;
}

template <class Mapping, class Unit>
int MappingTimeShift( Word* args, Word& result,
                    int message, Word& local, Supplier s )
{
    Word t;
    DateTime* dd;
    Unit unit;
    Mapping* mapping, *mpResult;

    result = qp->ResultStorage( s );

    mapping= (Mapping*)args[0].addr,
    mpResult = (Mapping*)result.addr;
    mpResult->Clear();

    dd = (DateTime *)args[1].addr;

    if( mapping->IsDefined() &&
        dd->IsDefined() )
    {
      mpResult->SetDefined( true );
      if(mapping->GetNoComponents() == 0)
        return 0;
      mapping->Get(0, unit);

      mpResult->StartBulkLoad();
      for( int i = 0; i < mapping->GetNoComponents(); i++ )
      {
        mapping->Get( i, unit );
        Unit aux( unit );
        aux.timeInterval.start.Add(dd);
        aux.timeInterval.end.Add(dd);
        mpResult->Add(aux);
      }
      mpResult->EndBulkLoad();
      return 0;
    }
    else
    {
      mpResult->SetDefined( false );
      return 0;
    }
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
    vector< Interval<Instant> > iv;
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
                  const bool rc,
                  const Mapping1& map1,
                  const Mapping2& map2);

public:
/*
The constructor creates the refinement partition from the two ~Mapping~
instances ~mr~ and ~mp~.

Runtime is $O(\max(n, m))$ with $n$ and $m$ the numbers of units in
~mr~ and ~mp~.

*Preconditions*: mr.IsDefined AND mp.IsDefiened()

*/
    RefinementPartition(const Mapping1& mr, const Mapping2& mp);

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

You can use

----

void TemporalUnit<Alpha>::AtInterval(const Interval<Instant> &i,
                                     TemporalUnit<Alpha> &result)

----

 to access the broken-down units

*/
    void Get(const unsigned int pos,
             Interval<Instant>& civ,
             int& ur,
             int& up);
};


template<class Mapping1, class Mapping2, class Unit1, class Unit2>
unsigned int RefinementPartition<Mapping1, Mapping2, Unit1, Unit2>
::Size(void)
{
  return iv.size();
}

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
void RefinementPartition<Mapping1, Mapping2, Unit1, Unit2>
::Get(const unsigned int pos, Interval<Instant>& civ, int& ur,
     int& up)
{
  assert(pos < iv.size());
  civ = iv[pos];
  ur = vur[pos];
  up = vup[pos];
}

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
void RefinementPartition<Mapping1, Mapping2, Unit1,
 Unit2>::AddUnits( const int urPos,      const int upPos,
                   const Instant& start, const Instant& end,
                   const bool lc,        const bool rc,
                   const Mapping1& map1, const Mapping2& map2)
{
  REF_DEBUG(
    "RP::AddUnits()                ["<<start.ToString()<<" "
    <<end.ToString()<<" "<<lc<<" "<<rc<<"] "<<urPos<<" "<<upPos)

  assert(urPos!=-1 || upPos!=-1);
  assert( start.IsDefined() && end.IsDefined() );


  if( (start==end) && !(lc && rc)){ // invalid interval
      return;
  }
  if(start.Adjacent(&end) && !(lc || rc)){ // invalid interval
      return;
  }

  Interval<Instant> civ(start, end, lc, rc);

  iv.push_back(civ);
  vur.push_back(urPos);
  vup.push_back(upPos);
}


template<class Mapping1, class Mapping2, class Unit1, class Unit2>
RefinementPartition<Mapping1, Mapping2, Unit1,
 Unit2>::RefinementPartition( const Mapping1& m1, const Mapping2& m2 )
{
   assert( m1.IsDefined() );
   assert( m2.IsDefined() );

   iv.clear();
   vur.clear();
   vup.clear();

   REF_DEBUG("RefinedmentPartition called ");
   int no1 = m1.GetNoComponents();
   int no2 = m2.GetNoComponents();
   if(no1 + no2 == 0){ // both mappings are empty
      REF_DEBUG("empty mappings");
      return;
   }
   if(no2==0){  // m2 is empty
     REF_DEBUG("m2 is empty");
     iv.reserve(no1);
     vur.reserve(no1);
     vup.reserve(no1);
     Unit1 u1;
     for(int i=0;i<no1;i++){
       m1.Get(i,u1);
       AddUnits(i,-1,u1.timeInterval.start, u1.timeInterval.end,
                     u1.timeInterval.lc, u1.timeInterval.rc,m1, m2);
     }
     return;
   }
   if(no1==0){   // m1 is empty
     REF_DEBUG("m1 is empty " );
     iv.reserve(no2);
     vur.reserve(no2);
     vup.reserve(no2);
     Unit2 u2;
     for(int i=0;i<no2;i++){
       m2.Get(i,u2);
       AddUnits(-1,i,u2.timeInterval.start, u2.timeInterval.end,
                     u2.timeInterval.lc, u2.timeInterval.rc,m1, m2);
     }
     return;
   }

   // both arguments are non-empty
   int maxsize = (no1 + no2 + 2) * 2;
   iv.reserve(maxsize);
   vur.reserve(maxsize);
   vup.reserve(maxsize);
   Unit1 u1p;
   Unit2 u2p;
   m1.Get(0,u1p);
   m2.Get(0,u2p);
   // create editable units from the constant ones
   Interval<Instant> t1(u1p.timeInterval);
   Interval<Instant> t2(u2p.timeInterval);

   int pos1 = 0;
   int pos2 = 0;

   REF_DEBUG("both arguments are non-empty");
   REF_DEBUG("no1 = " << no1 );
   REF_DEBUG("no2 = " << no2 );

   while( (pos1<no1) && (pos2<no2) ){
     REF_DEBUG("pos1 = " << pos1 );
     REF_DEBUG("pos2 = " << pos2 );
     REF_DEBUG("t1 = " << t1);
     REF_DEBUG("t2 = " << t2);

     // both arguments have units
     if(t1.start < t2.start){
       REF_DEBUG("case 1: t1 starts before t2 " );
       // t1 starts before t2
       if(t1.end < t2.start){ // t1 before t2
         REF_DEBUG("case 1.1: t1 ends before t2 starts" );
         AddUnits(pos1, -1, t1.start, t1.end, t1.lc, t1.rc,m1,m2);
         pos1++;
         if(pos1 < no1){
           m1.Get(pos1, u1p);
           t1 = u1p.timeInterval;
         }
       } else if(t1.end > t2.start){
         REF_DEBUG("case 1.2: t1 ends after t2 starts" );
         // overlapping intervals
         AddUnits(pos1, -1, t1.start, t2.start, t1.lc, !t2.lc,m1,m2);
         t1.start = t2.start;
         t1.lc = t2.lc;
       } else { // u1.timeInterval.end == u2.timeInterval.start
         REF_DEBUG("case 1.3: t1 ends when t2 starts ");
         if( !t1.rc  || !t2.lc){
           REF_DEBUG("case 1.3.1: t1 ends before t2 starts (closeness) " );
           // u1 before u2
           AddUnits(pos1, -1, t1.start, t1.end, t1.lc, t1.rc,m1,m2);
           pos1++;
           if(pos1 < no1){
             m1.Get(pos1,u1p);
             t1 = u1p.timeInterval;
           }
         } else { // intervals have a common instant
           REF_DEBUG("case 1.3.2: t2 ends when t2 starts (common instant)");
           AddUnits(pos1, -1, t1.start, t1.end, t1.lc, false,m1,m2);
           t1.lc = true;
           t1.start = t2.start;
         }
       }
     } else if(t2.start < t1.start){
        REF_DEBUG("case 2: t2 starts before t1 starts" );
        // symmetric case , u2 starts before u1
       if(t2.end < t1.start){ // u2 before u1
         REF_DEBUG("case 2.1: t2 ends before t1 ends ");
         AddUnits(-1, pos2, t2.start, t2.end, t2.lc, t2.rc,m1,m2);
         pos2++;
         if(pos2 < no2){
           m2.Get(pos2,u2p);
           t2 = u2p.timeInterval;
         }
       } else if(t2.end > t1.start){
         REF_DEBUG("case 2.2: t2 ends after t1 starts");
         // overlapping intervals
         AddUnits(-1, pos2, t2.start, t1.start, t2.lc, !t1.lc,m1,m2);
         t2.start = t1.start;
         t2.lc = t1.lc;
       } else { // u1.timeInterval.end == u2.timeInterval.start
         REF_DEBUG("case 2.3: t2 ends when t1 starts" );
         if( !t2.rc  || !t1.lc){
           REF_DEBUG("case 2.3.1: t2 ends before t1 starts (closeness)" );
           // u1 before u2
           AddUnits(-1, pos2, t2.start, t2.end, t2.lc, t2.rc,m1,m2);
           pos2++;
           if(pos2 < no2){
             m2.Get(pos2,u2p);
             t2 = u2p.timeInterval;
           }
         } else { // intervals have a common instant
           REF_DEBUG("case 2.3.2: t2 ends when t1 starts (common instant)");
           AddUnits(-1, pos2, t2.start, t2.end, t2.lc, false,m1,m2);
           t2.lc = true;
           t2.start = t1.start;
         }
       }
     } else { // u1.timeInterval.start == u2.timeInterval.start
       REF_DEBUG("case 3: t1 and t2 start at the same instant" );
       // both intervals start at the same instant
       if(t1.lc != t2.lc){
         REF_DEBUG("case 3.1: membership of the instant differs");
         if(t1.lc){ // add a single instant interval
            AddUnits(pos1, -1, t1.start, t1.start, true,true,m1,m2);
            if(t1.start == t1.end){ // u1 exhausted
              pos1++;
              if(pos1< no1){
                m1.Get(pos1,u1p);
                t1 = u1p.timeInterval;
              }
            } else {
              t1.lc = false;
            }
         } else {
            // symmetric case
            AddUnits(-1, pos2, t2.start, t2.start, true, true,m1,m2);
            if(t2.start == t2.end){
              pos2++;
              if(pos2 < no2){
                 m2.Get(pos2, u2p);
                 t2 = u2p.timeInterval;
              }
            } else {
               t2.lc = false;
            }
         }
       } else { // intervals start exact at the same instant
         REF_DEBUG("case 3.2: intervalls start exact together");
         if(t1.end < t2.end){
            REF_DEBUG("case 3.2.1: t1 ends before t2 ends");
            AddUnits(pos1, pos2, t1.start, t1.end, t1.lc, t1.rc,m1,m2);
            t2.start = t1.end;
            t2.lc = !t1.rc;
            pos1++;
            if(pos1<no1){
              m1.Get(pos1,u1p);
              t1 = u1p.timeInterval;
            }
         } else if (t2.end < t1.end){
            REF_DEBUG("case 3.2.2: t2 ends before t1 ends" );
            AddUnits(pos1, pos2, t2.start, t2.end, t2.lc, t2.rc,m1,m2);
            t1.start = t2.end;
            t1.lc = !t2.rc;
            pos2++;
            if(pos2 < no2){
              m2.Get(pos2,u2p);
              t2 = u2p.timeInterval;
            }
         } else { // both units end at the same instant
            REF_DEBUG("case 3.2.3: both intervals ends at the same instant");
            if(t1.rc == t2.rc){  // equal intervals
              REF_DEBUG("case 3.2.3.1: intervals are equal" );
              AddUnits(pos1, pos2, t1.start, t1.end, t1.lc, t1.rc,m1,m2);
              pos1++;
              if(pos1 < no1){
                m1.Get(pos1,u1p);
                t1 = u1p.timeInterval;
              }
              pos2++;
              if(pos2 < no2){
                m2.Get(pos2, u2p);
                t2 = u2p.timeInterval;
              }
            } else {
              REF_DEBUG("case 3.2.3.2: intervals differ at right closeness");
              // process common part
              AddUnits(pos1,pos2,t1.start, t1.end, t1.lc, false,m1,m2);
              if(t1.rc){
                 pos2++;
                 if(pos2<no2){
                   m2.Get(pos2,u2p);
                   t2 = u2p.timeInterval;
                 }
                 t1.lc = true;
                 t1.start = t1.end;
              } else {
                 pos1++;
                 if(pos1 < no1){
                   m1.Get(pos1,u1p);
                   t1 = u1p.timeInterval;
                 }
                 t2.lc = true;
                 t2.start = t2.end;
              }
            }
         }
       }
     }
   }

   REF_DEBUG("one of the arguments is finished");

   // process remainder of m1
   while(pos1 < no1){
     AddUnits(pos1, -1, t1.start, t1.end, t1.lc, t1.rc,m1,m2);
     pos1++;
     if(pos1<no1){
       m1.Get(pos1,u1p);
       t1 = u1p.timeInterval;
     }
   }
   // process remainder of m2
   while(pos2 < no2){
     AddUnits(-1, pos2, t2.start, t2.end, t2.lc, t2.rc,m1,m2);
     pos2++;
     if(pos2<no2){
        m2.Get(pos2,u2p);
        t2 = u2p.timeInterval;
     }
   }
}

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
RefinementPartition<Mapping1, Mapping2, Unit1,
 Unit2>::~RefinementPartition() {

    REF_DEBUG("RP::~RP() called" );

 }


template<class Unittype>
class CComparator_before{ // create a strict ordering function
    public:
      bool operator() (const Unittype &i, const Unittype &j) {
        return i.StartsBefore(j);
      }
};


/*
The following template function consolidates a vector of units:
As a preliminary step, the argument vector will be sorted by starting
instant and leftclosedness. Then, all undefined or invalid units are removed.

It restricts remaining units to the Interval ~intv~ and arranges them in such a
way, that consecutive units do not overlap temporally.

The elements in the result of this function can be consecutively added to any ~Mapping<UnitType,AlphaType>~

The return value is ~false~, iff the units pairwisely overlap in more than a
single instant.

*Precondition*: Units may not intersect despite in single points, where they
must have the same temporal value.

*Precondition*: The class parameter ~UnitType~ must be a class inheriting from ~Attribute~ AND ~TemporalUnit<AlphaType>~.

*Precondition*: The class parameter ~AlphaType~ must be a class inheriting from ~Attribute~.

*/
template<class UnitType, class AlphaType>
bool ConsolidateUnitVector(
                      const Interval<Instant> &intv,
                      vector<UnitType> &arg,
                      vector<UnitType> &result) {
  assert( intv.IsValid() );
  result.clear();

  // sort tmpresult by starting time (and leftclosedness)
  CComparator_before<UnitType> my_comparator_before;
  sort(arg.begin(), arg.end(), my_comparator_before);

  // do the consolidation
  UnitType lastUnit(false), currUnit(false);
  for(unsigned int i=0; i<arg.size(); i++) {
//     cout << "======================================================" << endl;
//     cout << "Iteration " << i << ":" << endl;
//     cout << "\tintv     = " << intv << endl;
//     cout << "\tlastUnit = " << lastUnit << endl;
//     cout << "\tcurrUnit = " << arg[i] << endl;
    UnitType u_full = arg[i];
    if( !u_full.IsDefined() || !u_full.IsValid() ) { // drop invalid units
//       cout << "\tcurrUnit is UNDEF or invalid. --> CONTINUE."
//            << arg[i] << endl;
      continue;
    }
    if(!intv.Intersects(u_full.timeInterval)){
      continue;
    }
    u_full.AtInterval( intv, currUnit ); // restrict to intv
//     cout << "\tRestricting currUnit to intv:" << endl;
//     cout << "\tcurrUnit := " << currUnit << endl;
    if(!currUnit.IsDefined() || !currUnit.IsValid() ) { // drop invaild result
//       cout << "\tcurrUnit is UNDEF or invalid. --> CONTINUE."
//            << arg[i] << endl;
      continue;
    }
    if(!lastUnit.IsDefined()) { // nothing to match with, keep currUnit
//       cout << "\tlastUnit is UNDEF. --> lastUnit := currUnit." << endl;
      lastUnit = currUnit;
      continue;
    }
    if(lastUnit.timeInterval  == currUnit.timeInterval) {
      // identical timeIntervals
      if(lastUnit != currUnit) {
      cerr << __PRETTY_FUNCTION__
           << "WARNING: same interval, but different values" << endl;
      result.clear();
      return false;
      } else { // same units --> drop currUnit
//         cout << "\tlastUnit and currUnit have same time Interval. "
//              << "--> CONTINUE." << endl;
        continue;
      }
    }
    if(lastUnit.timeInterval.Inside(currUnit.timeInterval)){
      // lastUnit contained by currUnit
      lastUnit = currUnit; // drop lastUnit
//       cout << "\tlastUnit inside currUnit. --> lastUnit := currUnit."
//            << endl;
      continue;
    }
    if(currUnit.timeInterval.Inside(lastUnit.timeInterval)){
      // lastUnit contains currUnit
//       cout << "\tlastUnit contains currUnit. --> CONTINUE." << endl;
      continue; // drop currUnit
    }
    if(lastUnit.timeInterval.end == currUnit.timeInterval.start) {
//       cout << "\tlastUnit and currUnit have common endpoint..." << endl;
      // lastUnit and currUnit share a common boundary point at least
      //             (but are not identical - that has been handled above)
      // handle all the closedness and point/interval combinations
      if(lastUnit.timeInterval.rc){
        if(currUnit.timeInterval.lc){
          // rc - lc: ...a][a...
          AlphaType lastVal, currVal;
          lastUnit.TemporalFunction(lastUnit.timeInterval.end,  lastVal, true);
          currUnit.TemporalFunction(currUnit.timeInterval.start,currVal, true);
          if( lastVal.CompareAlmost(static_cast<Attribute*>(&currVal)) != 0) {
            cerr << __PRETTY_FUNCTION__ << " ERROR: Two units share an endpoint"
                 << ", but have different temporal values at that instant: "
                 << endl << "\tlastUnit = " << lastUnit << ", lastVal = " ;
            lastVal.Print(cerr);
            cerr << endl << "\tcurrUnit = " << currUnit << ", currVal = " ;
            lastVal.Print(cerr);
            cerr << endl;
            result.clear();
            return false;
          }
          if(lastUnit.IsInstantUnit()){
            if(currUnit.IsInstantUnit()){
              // [aa][aa]: Drop lastUnit, keep currUnit
              // should already be dealt with
              lastUnit = currUnit;
//               cout << "\tCase: [aa][aa]. ---> lastUnit = currUnit." << endl;
              continue;
            } else {
              // [aa] [a---: Drop lastUnit, keep currUnit
              lastUnit = currUnit;
//               cout << "\tCase: [aa] [a---. ---> lastUnit = currUnit."
//                    << endl;
              continue;
            }
          } else {
            if(currUnit.IsInstantUnit()){
              // ---a] [aa]: drop currUnit, keep lastUnit
//               cout << "\tCase: ---a] [aa]. ---> CONTINUE." << endl;
              continue;
            } else {
              // ---a] [a---: append rightopen lastUnit. Keep currUnit
              // one could also try to merge the units and keep the merged unit
              if(!lastUnit.Merge(currUnit)) {
                lastUnit.timeInterval.rc = false;
                result.push_back(lastUnit);
//                 cout << "\tCase: ---a] [a---. ---> Cannot merge units. "
//                      << "Change and append lastUnit to the result. lastUnit "
//                      << ":= currUnit" << endl;
//                 cout << "\t\tappending: " << lastUnit << endl;
                lastUnit = currUnit;
                continue;
              } else {
//                 cout << "\tCase: ---a] [a---. ---> Merged units. lastUnit "
//                      << ":= " << lastUnit << endl;
                continue;
              }
            }
          }
        } else {
          // rc - lo: ...]]...
          if(lastUnit.IsInstantUnit()){
            // [aa] ]a---: modify currUnit and keep it
//             cout << "\tCase: [aa] ]a---. ---> merge lastUnit with currUnit:"
//                  << endl;
            currUnit.timeInterval.lc = true;
            lastUnit = currUnit;
//             cout << "\t\tmerged unit: " << lastUnit << endl;
            continue;
          } else {
            // ---a] ]a---: No problem.
            result.push_back(lastUnit);
//             cout << "\tCase: ---a] ]a---. ---> Append lastUnit:" << endl;
//             cout << "\t\tAppended: " << lastUnit << endl;
            lastUnit = currUnit;
            continue;
          }
        }
      } else { // lastUnit has rightopen interval
        if(currUnit.timeInterval.rc){
          // ro - lc: ...a[[a...:
          if(currUnit.IsInstantUnit()){
            // ..a[ [aa]: Merge currUnit into lastUnit. Keep modified lastUnit
            AlphaType lastVal, currVal;
            lastUnit.TemporalFunction(lastUnit.timeInterval.end,  lastVal,true);
            currUnit.TemporalFunction(currUnit.timeInterval.start,currVal,true);
            if( lastVal.CompareAlmost(static_cast<Attribute*>(&currVal)) == 0 ){
              // units can be merged: Merge both into lastUnit, drop currUnit
              lastUnit.timeInterval.rc = true;
//               cout << "\tCase: ..a[ [aa]. ---> merge lastUnit with currUnit:"
//                    << endl;
//               cout << "\t\tmerged unit: " << lastUnit << endl;
              continue;
            } else {
              // units cannot be merged: Output lastUnit, keep currUnit
              result.push_back(lastUnit);
//               cout << "\tCase: ..a[ [aa]. ---> cannot merge lastUnit with "
//                    << "currUnit:" << endl;
//               cout << "\t\tAppended: " << lastUnit << endl;
              lastUnit = currUnit;
              continue;
            }
          } else {
            // ...a[ [a---: No problem: Output lastUnit, keep currUnit.
            result.push_back(lastUnit);
//             cout << "\tCase: ...a[ [a---" << endl;
//             cout << "\t\tAppended: " << lastUnit << endl;
            lastUnit = currUnit;
            continue;
          }
        } else {
          // ro - lo: ...a[]a...: No problem: Output lastUnit, keep currUnit.
            result.push_back(lastUnit);
//             cout << "\tCase: ...a[]a..." << endl;
//             cout << "\t\tAppended: " << lastUnit << endl;
            lastUnit = currUnit;
            continue;
        }
      }
    }
    if(!currUnit.timeInterval.Intersects(lastUnit.timeInterval)){
      // lastUnit and lastUnit do NOT overlap at all
//       cout << "\tCase: lastUnit and currUnit do not overlap and have no "
//            <<common endpoint." << endl;
//       cout << "\t\tAppended: " << lastUnit << endl;
      result.push_back(lastUnit);
      lastUnit = currUnit;
      continue;
    } else {
      // lastUnit and currUnit overlap, but have not a single common boundary
      // point. This shoulöd be a misstake! There is no way to merge the units:
      cerr << __PRETTY_FUNCTION__ << " ERROR: Two units overlap, but in more "
          << "than a single point:" << endl << "\tlastUnit = " << lastUnit
          << endl << "\tcurrUnit = " << currUnit << endl;
      result.clear();
      return false;
    }
  }
  // possibly insert remaining lastUnit:
//   cout << "========================================================" << endl;
//   cout << "Aftermath:" << endl;
//   cout << "\tintv     = " << intv << endl;
//   cout << "\tlastUnit = " << lastUnit << endl;
  if(lastUnit.IsDefined()) {
//    cout << "\t\tAppended: " << lastUnit << endl;
    result.push_back(lastUnit);
  }
//   cout << "========================================================" << endl;
//   cout << "Finished." << endl;
  return true;
}

/*
3 Type definitions moved here from TemporalExtAlgebra.h

The types UString and MString was defined to implement the data types
~ustring~ and ~mapping(string)~ according to the declared defintions in the
signature for moving objects. The types RBool and RString was declared in order
to implement the data types ~range(bool)~ and ~range(string)~. They are
also part of the signature for moving objects.

*/
typedef ConstTemporalUnit<CcString> UString;
typedef Mapping< UString, CcString > MString;
typedef Range<CcBool> RBool;
typedef Range<CcString> RString;


#endif // _TEMPORAL_ALGEBRA_H_
