/*
//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]

[1] Header File of the Range Algebra

February, 2002 Victor Teixeira de Almeida

1 Overview

This header file essentially contains the definition of the classes ~Interval~ and
~Range~. The class ~Range~ corresponds to the memory representation of the type
constructor ~range($\alpha$)~. The class ~Interval~ is used by the class ~Range~.

Let $\alpha$ be a data type to which the ~range~ type constructor is applicable
(and hence on which a total order $<$ exists). An ~$\alpha$-interval~ is a set
$X \subseteq \overline{A}_{\alpha}$ such that 
$\forall x, y \in X, \forall z \in \overline{A}_{\alpha}: x < z < y \Rightarrow z \in X$.

Two $\alpha$-intervals are ~adjacent~, if they are disjoint and their union is an
$\alpha$-interval. An $\alpha$-range is a finite set of disjoint, non-adjacent 
$\alpha$-intervals. 

An $\alpha$-interval ~X~ is ~left-closed~ if $inf(X) \in X$, it is ~right-closed~ if 
$sup(X) \in X$, and it is ~closed~ if it is both ~left-closed~ and ~right-closed~. Dually, 
it is (~left-~/~right-~) ~open~ iff it is not (~left-~/~right-~) closed. The ~closure~ of an 
$\alpha$-interval ~X~ is defined as 
$closure(X) = X \cup \{ inf(X) \} \cup \{ sup(X) \}$.

2 Defines and includes

*/
#ifndef __RANGE_H__
#define __RANGE_H__

#include "StandardAttribute.h"
#include "SecondoSMI.h"

/*

3 Struct ~Interval~

3.1 Overview

The class ~Interval~ implements the closure of an $\alpha$-interval. To be a generic
class, this class stores attributes of ~StandardAttribute~ type. In this way, any 
child class of ~StandardAttribute~ can have an interval counterpart.

An interval contains a ~start~, an ~end~ and two flags ~lc~ and ~rc~ indicating if
the interval is left-closed and right-closed, respectively. 


*/
struct Interval
{
/*
3.2 Constructors and Destructor

*/
  Interval( const int algebraId, const int typeId );
/*
The simple constructor.

*/
  Interval( const Interval& interval );
/*
The copy constructor.

*/
  Interval( const int algebraId,
            const int typeId,
            StandardAttribute* start,
            StandardAttribute* end,
            const bool lc,
            const bool rc );
/*
The creation of the interval setting all attributes. One should note that the ~start~
and ~end~ attributes are cloned.

*/
  ~Interval();

/*
The destructor.

3.3 Member functions

*/
  bool IsDefined() const
    { return start->IsDefined() && end->IsDefined(); }
/*
Returns true if both the start and the end of the interval are defined. Returns
false otherwise.

*/
  bool IsValid() const;
/*
Checks if the interval is valid or not. This function should be used for debugging purposes
only.  An interval is valid if the following conditions are true:

  1 is defined

  2 ~start~ $<=$ ~end~

  3 if ~start~ $==$ ~end~, then must ~lc~ $=$ ~rc~ $=$ ~true~

*/
  Interval& operator=( const Interval& i );
/*
Redefinition of the copy operator ~=~.

*/
  bool operator==(const Interval& i) const;
/*
Returns ~true~ if this interval is equal to the interval ~i~ and ~false~ if they are different.

*Precondition:* ~this.IsDefined() $\&\&$ i.IsDefined()~

*/
  bool operator!=(const Interval& i) const;
/*
Returns ~true~ if this interval is different to the interval ~i~ and ~false~ if they are equal.

*Precondition:* ~this.IsDefined() $\&\&$ i.IsDefined()~

*/
  bool R_Disjoint(const Interval& i) const;
/*
Returns ~true~ if this interval is r-disjoint with the interval ~i~ and ~false~ otherwise.

*Precondition:* ~this.IsDefined() $\&\&$ i.IsDefined()~

*/
  bool Disjoint(const Interval& i) const;
/*
Returns ~true~ if this interval is disjoint with the interval ~i~ and ~false~ otherwise.

*Precondition:* ~this.IsDefined() $\&\&$ i.IsDefined()~

*/
  bool R_Adjacent(const Interval& i) const;
/*
Returns ~true~ if this interval is r-adjacent with the interval ~i~ and ~false~ otherwise.

*Precondition:* ~this.IsDefined() $\&\&$ i.IsDefined()~

*/
  bool Adjacent(const Interval& i) const;
/*
Returns ~true~ if this interval is adjacent with the interval ~i~ and ~false~ otherwise.

*Precondition:* ~this.IsDefined() $\&\&$ i.IsDefined()~

*/
  bool Inside(const Interval& i) const;
/*
Returns ~true~ if this interval is inside the interval ~i~ and ~false~ otherwise.

*Precondition:* ~this.IsDefined() $\&\&$ i.IsDefined()~

*/
  bool Contains(StandardAttribute* a) const;
/*
Returns ~true~ if this interval contains the value ~a~ and ~false~ otherwise.

*Precondition:* ~this.IsDefined() $\&\&$ a.IsDefined()~

*/
  bool Intersects(const Interval& i) const;
/*
Returns ~true~ if this interval intersects with the interval ~i~ and ~false~ otherwise.

*Precondition:* ~this.IsDefined() $\&\&$ i.IsDefined()~

*/
  bool Before(const Interval& i) const;
/*
Returns ~true~ if this interval is before the interval ~i~ and ~false~ otherwise.

*Precondition:* ~this.IsDefined() $\&\&$ i.IsDefined()~

*/
  bool Before(StandardAttribute* a) const;
  bool After(StandardAttribute* a) const;
/*
Returns ~true~ if this interval is before/after the value ~a~ and ~false~ otherwise.

*Precondition:* ~this.IsDefined() $\&\&$ a.IsDefined()~

3.3 Attributes

*/
  int algebraId;
/*
The algebra identification of the interval's type.

*/
  int typeId;
/*
The type identification of the interval's type.

*/
  StandardAttribute* start;
/*
The start of the interval.

*/
  StandardAttribute* end;
/*
The end of the interval.

*/
  bool lc;
/*
Tells if the interval is left-closed.

*/
  bool rc;
/*
Tells if the interval is right-closed.

*/
};

/*
4 Class Range

The ~Range~ class implements a set of disjoint, non-adjacent ~Intervals~.
For this implementation, it is used a persistent array of ordered intervals.
We could not use the ~PArray~ package because the ~Interval~ class is made
of memory pointers to a generic type ~StandardAttribute~, but the main 
funcionalities of the persistent array (~PArray~) were included in this 
class ~Range~.

*/
class Range
{
  public:
/*
4.1 Constructors and Destructor

*/
    Range( const int algebraId, const int typeId, const int size, const int n = 0 );
/*
The ~range~ constructor receives the ~algebraId~, the ~typeId~, and the ~size~ of
the $\alpha$-type. The size will be used for reading and writing the range values into 
disk. One can also initialize the allocation with size ~n~.

*/
    void Destroy();
/*
This function should be called before the destructor if one wants to destroy the
persistent array of intervals. It marks the persistent array for destroying. The
destructor will perform the real destroying.

*/
    ~Range();
/*
The destructor.

4.2 Functions for Bulk Load of Points

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
Marks the end of a bulk load and sorts the interval set.

4.3 Member functions

*/
    bool IsEmpty() const
      { return noComponents == 0; }
/*
Returns if the range is empty of intervals or not.

*/
    int GetAlgebraId() const
      { return algebraId; }
/*
Returns the algebra ~id~ of the $\alpha$-element of the range.

*/
    int GetTypeId() const
      { return typeId; }
/*
Returns the type ~id~ of the $\alpha$-element of the range.

*/
    int GetElemSize() const
      { return size; }
/*
Returns the size of the $\alpha$-element of the range.

*/
    void Get( const int i, Interval& ai );
/*
Returns the interval ~ai~ at the position ~i~ in the range.

*/
    enum IntervalPosition { Begin, End };
    void Get( const int i, const IntervalPosition pos, StandardAttribute *a, bool& closed );
/*
Returns the attribute at the ~i~-th interval at position ~pos~ (~Begin~ or ~End~).

*/
    void Add( const Interval& i );
/*
Add an interval ~i~ to the range. We will assume that the only way of
adding intervals is in bulk loads, i.e., in a non-ordered array.

*Precondition:* ~IsOrdered() == false~

4.4 Operations

4.4.1 Operation $=$ (~equal~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~ 

*Semantics:* $X = Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool operator==(Range& r);
/*
4.4.2 Operation $\neq$ (~not equal~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \neq Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool operator!=(Range& r);
/*
4.4.3 Operation ~intersects~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \cap Y \neq \emptyset$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool Intersects(Range& r);
/*
4.4.3 Operation ~inside~

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \subseteq Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    bool Inside(Range& r);
/*
4.4.4 Operation ~contains~

*Precondition:* ~X.IsOrdered() $\&\&$ y.IsDefined()~

*Semantics:* $y \in X$

*Complexity:* $O(log(n))$, where ~n~ is the size of this range ~X~.

*/
    bool Contains(StandardAttribute *a);
/*
4.4.5 Operation ~before~ (with ~range~)

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $\forall x \in X, \forall y \in Y: x \leq y$

*Complexity:* $O(1)$.

*/
    bool Before(Range& r);
/*
4.4.6 Operation ~before~ (with ~BASE~ type)

*Precondition:* ~X.IsOrdered() $\&\&$ y.IsDefined()~

*Semantics:* $\forall x \in X: x \leq y$

*Complexity:* $O(1)$.

*/
    bool Before(StandardAttribute *a);
/*
4.4.7 Operation ~after~ 

This operation works only with ~BASE~ type, because it is an attempt to implement the operation
before on a contrary order, i.e., ~x before Y~.

*Precondition:* ~Y.IsOrdered() $\&\&$ x.IsDefined()~

*Semantics:* $\forall y \in Y: x \leq y$

*Complexity:* $O(1)$.

*/
    bool After(StandardAttribute *a);
/*
4.4.8 Operation ~intersection~ 

*Precondition:* ~X.IsOrdered() $\&\&$ X.IsOrdered() $\&\&$ Result.IsEmpty()~

*Semantics:* $X \cap Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    void Intersection(Range& r, Range& result);
/*
4.4.8 Operation ~union~ 

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered() $\&\&$ Result.IsEmpty()~

*Semantics:* $X \cup Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    void Union(Range& r, Range& result);
/*
4.4.8 Operation ~minus~ 

*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered() $\&\&$ Result.IsEmpty()~

*Semantics:* $X \backslash Y$

*Complexity:* $O(n+m)$, where ~n~ is the size of this range ~X~ and m the size of the range ~Y~.

*/
    void Minus(Range& r, Range& result);
/*
4.4.8 Operation ~max~ 

*Precondition:* ~X.IsOrdered()~

*Semantics:* $max(\rho(X))$

*Complexity:* $O(1)$

*/
    void Maximum(StandardAttribute *result);
/*
4.4.8 Operation ~min~ 

*Precondition:* ~X.IsOrdered()~

*Semantics:* $min(\rho(X))$

*Complexity:* $O(1)$

*/
    void Minimum(StandardAttribute *result);
/*
4.4.8 Operation ~no\_components~ 

*Precondition:* ~X.IsOrdered()~

*Semantics:* $\| intvls(X) \|$

*Complexity:* $O(1)$

*/
    int GetNoComponents() const;

  private:
/*
4.5 Private member functions

*/
    bool IsValid();
/*
This functions tests if a ~range~ is in a valid form. It is used for debugging
purposes only. The ~range~ is valid, if the following conditions are true:

  1 Each interval is valid

  2 Start of each interval $>=$ end of the interval before

  3 If start of an interval = end of the interval before, then one needs to 
    make sure that the interval is not left-closed or the interval before 
    is not right-closed

*/
    void Sort();
/*
Sorts (STL ~sort~ algorithm) the range.

*/
    void Put( const int i, const Interval& interval );
/*
Puts an interval in the ~i~-th position of the array.

*Precondition:* ~0 $\leq$ i $<$ noComponents~

4.6 Attributes

*/
    int algebraId;
    int typeId;
    int size;
/*
The algebraId, typeId, and size of the $\alpha$-element of the range.

*/
    bool canDelete;
/*
A flag indicating if the destructor should destroy also the persistent
array of intervals.

*/
    int noComponents;
/*
Stores the number of intervals of the range.

*/
    bool ordered;
/*
A flag indicating whether the interval set is ordered or not.

*/
    FLOB intervals;
/*
The intervals array as a FLOB. We cannot use DBArrays here because the
range contains pointers to StandardAttribute, and cannot be saved in
this way. 

*/
};

#endif // __RANGE_H__
