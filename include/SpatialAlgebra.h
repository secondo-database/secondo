/*
//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]

[1] Header File of the Spatial Algebra

February, 2003 Victor Teixeira de Almeida

March-April, 2003 Zhiming DING

1 Overview

This header file essentially contains the definition of the classes ~Point~, 
~Points~, ~Line~, and ~Region~ used in the Spatial Algebra. These classes
respectively correspond to the memory representation for the type constructors
~point~, ~points~, ~line~, and ~region~.  Figure \cite{fig:spatialdatatypes.eps} 
shows examples of these spatial data types.

2 Defines and includes

*/
#ifndef __SPATIAL_ALGEBRA_H__
#define __SPATIAL_ALGEBRA_H__

#include "Rational.h"
#include "PArray.h"

#define RATIONAL_COORDINATES
#ifdef RATIONAL_COORDINATES
#ifdef WITH_ARBITRARY_PRECISION
typedef Rational Coord;
#else // WITH_ARBITRARY_PRECISION
typedef Rational Coord;
#endif
#else // RATIONAL_COORDINATES
typedef long Coord;
#endif

/*
There are two main defines that will control how the coordinate system is 
implemented, namely ~RATIONAL\_COORDINATES~ and ~WITH\_ARBITRARY\_PRECISION~.
The first one defines if the system will use rational coordinates instead 
of simple integer (~long~) ones. The second one is used only if rational 
coordinates are being used and defines if the system will use rational 
numbers with arbitrary precision (~not implemented yet~) instead of fixed 
precision (~Rational~).

3 Class Point

This class implements the memory representation of the ~point~ type 
constructor. A point represents a point in the Euclidean plane or is 
undefined. The figure \cite{fig:spatialdatatypes.eps} shows an example
of a point data type.

*/
class Points;
/*
A forward declaration of the class ~Points~.

*/

class Point
{
  public:
/*
3.1. Constructors and Destructor

There are two ways of constructing a point:

*/
    Point( const bool d = false, const Coord& x = 0L, const Coord& y = 0L );
/*
The first one receives a boolean value ~d~ indicating if the point is defined
and two coordinate ~x~ and ~y~ values. Note that this constructor can be
called without arguments because all of them have default values.

*/
    Point( const Point& p );
/*
The second one receives a point ~p~ as argument and creates a point that is a 
copy of ~p~.

*/
    ~Point();
/*
The destructor.

3.2 Member functions

*/
    const bool IsDefined() const;
/*
Returns if the point is defined.

*/
    const Coord& GetX() const;
/*
Returns the ~x~ coordinate of the point.

*Precondition:* ~IsDefined()~

*/
    const Coord& GetY() const;
/*
Returns the ~y~ coordinate of the point.

*Precondition:* ~IsDefined()~

*/
    void  Set( const bool D, const Coord& X, const Coord& Y)
    {
	defined=D;
	x=X;
	y=Y;
    }
/*
  
This function set the value of the point object.

*/
    Point& operator=(const Point& p);
/*
Assignement operator redefinition.

*Precondition:* ~p.IsDefined()~

*/
    void SetDefined( const bool d = true );
    
/*
Sets the point to defined or undefined depending on the value of ~d~.

3.3 Operations

3.3.1 Operation $=$ (~equal~) 

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u = v$

*Complexity:* $O(1)$

*/
    int operator==(const Point& p) const;
/*
3.3.2 Operation $\neq$ (~not equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \neq v$

*Complexity:* $O(1)$

*/
    int operator!=(const Point& p) const;
/*
3.3.3 Operation $\leq$ (~less or equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \leq v$

*Complexity:* $O(1)$

*/
    int operator<=(const Point& p) const;
/*
3.3.4 Operation $<$ (~less than~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u < v$

*Complexity:* $O(1)$

*/
    int operator<(const Point& p) const;
/*
3.3.5 Operation $\geq$ (~greater or equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \geq v$

*Complexity:* $O(1)$

*/
    int operator>=(const Point& p) const;
/*
3.3.6 Operation $>$ (~greater than~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u > v$

*Complexity:* $O(1)$

*/
    int operator>(const Point& p) const;
/*
3.3.7 Operation ~inside~ 

*Precondition:* ~u.IsDefined()~

*Semantics:* $u \in V$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
    const bool Inside( const Points& ps ) const;
/*
3.3.8 Operation ~intersection~ (with ~point~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* if $u = v$ then $u$ else $\perp$

*Complexity:* $O(1)$

*/
    void Intersection( const Point& p, Point& result ) const;
/*
3.3.9 Operation ~intersection~ (with ~points~)

*Precondition:* ~u.IsDefined()~

*Semantics:* if $u \in V$ then $u$ else $\perp$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
    void Intersection( const Points& ps, Point& result ) const;
/*
3.3.10 Operation ~minus~ (with ~point~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* if $u = v$ then $\perp$ else $u$

*Complexity:* $O(1)$

*/
    void Minus( const Point& p, Point& result ) const;
/*
3.3.9 Operation ~minus~ (with ~points~)

*Precondition:* ~u.IsDefined()~

*Semantics:* if $u \in V$ then $\perp$ else $u$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
    void Minus( const Points& ps, Point& result ) const;
/*
3.3.10 Operation ~distance~ 

*Precondition:* ~u.IsDefined()~ and ~v.IsDefined()~

*Semantics:* compute the distance between u and v

*Complexity:* $O(1)$

*/
    double distance( const Point& p ) const;
    
  private:
/*
3.4 Attributes

*/
    Coord x;
/*
The ~x~ coordinate.

*/
    Coord y;
/*
The ~y~ coordinate.

*/
    bool defined;
/*
A flag that tells if the point is defined or not.

*/
};

/*
4 Class Points

This class implements the memory representation of the ~points~ type constructor.
A points value is a finite set of points. An example of a points value can be seen
in the Figure \cite{fig:spatialdatatypes.eps}.

The implementation of the points type constructor is a persistent array of points
ordered by lexicographic order.

*/
class Points
{
  public:
/*
4.1 Constructors and Destructor

There are three ways of constructing a point set:

*/
    Points( SmiRecordFile *recordFile );
/*
The first one receives no arguments and constructs an empty point set.

*/
    Points( SmiRecordFile *recordFile, const Points& ps );
/*
The second one receives another point set ~ps~ as argument and constructs a point
set which is a copy of ~ps~.

*/
    Points( SmiRecordFile *recordFile, const SmiRecordId recordId, bool update = true );
/* 
The third and the last one receives a ~recordId~ and a flag ~update~ as arguments.
This constructor is applied not to create a new point set, but to read it from
the disk. The ~recordId~ gives the position of the point set in the persistent
array (~PArray~) structure and the flag ~update~ is used to open the array for 
update or read-only.

*/
    void Destroy();
/*
This function should be called before the destructor if one wants to destroy the 
persistent array of points. It marks the persistent array for destroying. The 
destructor will perform the real destroying.

*/
    ~Points();
/*
The destructor.

4.2 Functions for Bulk Load of Points

As said before, the point set is implemented as an ordered persistent array of points.
The time complexity of an insertion operation in an ordered array is $O(n)$, where ~n~
is the size of the point set. In some cases, bulk load of points for example, it is good
to relax the ordered condition to improve the performance. We have relaxed this ordered 
condition only for bulk load of points. All other operations assume that the point set is
ordered.

*/
    const bool IsOrdered() const;
/*
Returns if the point se is ordered. There is a flag ~ordered~ (see attributes) in order
to avoid a scan in the point set to answer this question.

*/
    void StartBulkLoad();
/*
Marks the begin of a bulk load of points relaxing the condition that the points must be 
ordered.

*/
    void EndBulkLoad();
/*
Marks the end of a bulk load and sorts the point set.

4.3 Member functions

*/
    const bool IsEmpty() const;
/*
Returns if the set is empty of not.

*/
    const int Size() const;
/*
Returns the size of the point set. Returns ~0~ if the set is empty.

*/
    void Get( const int i, Point& p ) const;
/*
Retrieves the point ~p~ at position ~i~ in the point set.

*Precondition:* $0 \leq i < Size()$

*/
    const SmiRecordId GetPointsRecordId() const;
/*
Returns the record identification of the points persistent array.

*/
    Points& operator=(const Points& ps);
/*
Assignement operator redefinition.

*Precondition:* ~ps.IsOrdered()~

*/
    const bool Contains(const Point& p) const;
/*
Searches (binary search algorithm) for a point in the point set and
return ~true~ if found and ~false~ if not.

*Precondition:* ~this.IsOrdered() $\&\&$ p.IsDefined()~

*/
    const bool Contains(const Points& ps) const;
/*
Returns ~true~ if this point set contains the ~ps~ point set and 
~false~ otherwise.

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

4.4 Operations

4.4.1 Operation $=$ (~equal~)

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

*Semantics:* $this = ps$

*Complexity:* $O(n+m)$, where ~n~ is the size of this point set and m the size of ~ps~.

*/
    int operator==(const Points&) const;
/*
4.4.2 Operation $\neq$ (~not equal~)

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

*Semantics:* $this = ps$

*Complexity:* $O(n+m)$, where ~n~ is the size of this point set and m the size of ~ps~.

*/
    int operator!=(const Points&) const;
/*
4.4.2 Operation ~union~ (with ~point~)

*Precondition:* ~p.IsDefined()~

*Semantics:* $this \cup \{p\}$

*Complexity:* $O(1)$, if the set is not ordered, and $O(log(n)+n)$, otherwise. ~n~ is the size 
of this point set.

*/
    Points& operator+=(const Point& p);
/*
4.4.3 Operation ~union~ (with ~points~)

*Precondition:* 

*Semantics:* $this \cup ps$

*Complexity:* $O(m)$, if the set is not ordered, and $O(m+(m+n)log(m+n))$, otherwise. ~n~ is the size 
of this point set and ~m~ is the size of ~ps~.

*/
    Points& operator+=(const Points& ps);
/*
4.4.4 Operation ~minus~ (with ~point~)

*Precondition:* ~p.IsDefined() $\&\&$ this.IsOrdered()~

*Semantics:* $this \backslash \{p\}$

*Complexity:* $O(log(n)+n)$

*/
    Points& operator-=(const Point& p);
/*
4.4.5 Operation ~inside~

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

*Semantics:* $this \subseteq ps$

*Complexity:* $O(n+m)$, where ~n~ is the size of this point set and m the size of ~ps~.

*/
    const bool Inside(const Points& ps) const;
/*
4.4.6 Operation ~intersects~

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

*Semantics:* $this \cap ps$

*Complexity:* $O(n+m)$, where ~n~ is the size of this point set and m the size of ~ps~.

*/
    const bool Intersects(const Points& ps) const;
    
    void SelectFirst();
    void SelectNext();
    bool EndOfPt();
    void GetPt( Point& p );
    void InsertPt( Point& p );
/*
 Added by DZM to comply with ROSE algebra
  
*/

  private:
/*
4.5 Private member functions

*/
    void Sort();
    void QuickSortRecursive( const int low, const int high );
/*
Sorts (quick-sort algorithm) the persistent array of points.

*/
    const int Position(const Point&) const;
/*
Searches (binary search algorithm) for a point in the point set and
returns its position. Returns -1 if the point is not found.

*/
  
/*
4.6 Atrtibutes

*/
    PArray<Point>* points;
/*
The persisten array of points.

*/
    bool ordered;
/*
The flag that indicates whether the persistent array is in ordered state.

*/
    int pos;
/*
 Added by DZM. According to ROSE algebra, the carrier set of points should contain a pos pointer
  
*/
};

/*
4.7 Auxiliary functions

*/
ostream& operator<<( ostream& o, const Point& p );
/*
Print the point ~p~ in the out stream ~o~.

*/
ostream& operator<<( ostream& o, const Points& ps );
/*
Print the point set ~ps~ int the out stream ~o~.

*/

/*
The following type definition indicates the structure of the ~attr~ value associated with 
half segments. This attribute is utilized only when we are handling regions. In line values
this attibute is ignored.

*/
typedef struct  
{
    int faceno;
    int cycleno;
    int edgeno;
    //set<int> attr;
    int attr;
} attrtype;

/*
5 Class Half Segment

This class implements the memory representation of the ~halfsegment~ type constructor.
A ~halfsegment~ value is a pair of points, with a boolean flag indicating the dominating point.


The name for the class is started with a ~C~ prefix which means that this is a class name. 
Since very similar names appear in different places with different meaning, it is a good idea
to put this prefix to eliminate confusion.

*/

class CHalfSegment 
{
  public:
    
/*
5.1 Constructors and Destructor

A Half Segment is composed of two points which are called ~left point~ LP and ~right point~ RP 
(LR \verb+<+ RP), and a ~flag~ LDP (~Left Dominating Point~) which tells which point is
the dominating point. The Boolean Flag ~Defined~ allows us to use an ~undifined~ value.

*/
    CHalfSegment(bool Defined, bool LDP, Point& LP, Point& RP);
    CHalfSegment( const CHalfSegment& chs );
    CHalfSegment();
    ~CHalfSegment();
    
/*
5.2 Functions Returning Property Values from an Object

*/
    const bool IsDefined() const;
/*
returns a boolean value indicating whether the half segment is defined.

*/
    const Point&  GetLP() const;
/*
returns the left point of the half segment. 

*/
    const Point&  GetRP() const;
/*
returns the right point of the half segment. 

*/
    const Point&  GetDPoint() const;
/*
returns the dominating point of the half segment. 

*/
    const Point&  GetSPoint() const; 
/*
returns the secondary point of the half segment. 

*/
    const bool GetLDP() const;
/*
returns the boolean flag which indicates whether the dominating point is on the 
left side. 

*/
    const attrtype&  GetAttr() const;
/*
returns the "attribute" value associated with a half segment. The "attribute" argument is useful when
 we process region values. Currently the "attribute" value is composed of a set of int, but it will soon be 
implemented as a structure which contain more information.


5.3 Functions Setting Property Values of an Object

*/
    void     Set(bool Defined,  bool LDP, Point& LP, Point& RP);
/*
sets the value of a half segment. 

*/
    void     SetDefined(bool Defined);
/*
sets the value of the "defined" argument of a half segment. 

*/
    void     SetAttr(attrtype& ATTR);
/*
sets the value of the "attr" argument of a half segment. 

*/
    void     SetLDP(bool LDP); 
/*
sets the value of the "Left Dominating Point" flag of a half segment. 

5.4 Overloaded class operators

5.4.1 Operation $=$ (~assignment~)

*Semantics:* $this = chs$. It assigns the value of a half segment to another.

*Complexity:* $O( 1)$ 

*/
    CHalfSegment& operator=(const CHalfSegment& chs);
/*
5.4.2 Auxiliary Function (~compare~)

*Semantics:* This function make comparison between two halfsegments. These two half segments are compared
according to the following order: dominating points -\verb+>+  LDP flages  -\verb+>+ directions (rotations), as 
indicated in the ROSE paper.

*Complexity:* $O( 1 )$ 

*/
    int chscmp(const CHalfSegment& chs) const;
/*
5.4.3 Operation $==$ (~equal~)

*Semantics:* $this ==chs$. It decides whether two half segments are identical.

*Complexity:* $O( 1 )$ 

*/   
    int operator==(const CHalfSegment& chs) const;
    int operator!=(const CHalfSegment& chs) const;
/*
5.4.4 Operation $<$ (~less than~)

*Semantics:* $u < v$. It decides whether whether one half segment is less than the other.

*Complexity:* $O( 1 )$ 

*/
    int operator<(const CHalfSegment& chs) const;
/*
5.4.5 Operation $>$ (~greater than~)

*Semantics:* $u > v$. It decides whether whether one half segment is greater than the other.

*Complexity:* $O( 1 )$ 

*/
    int operator>(const CHalfSegment& chs) const;
/*
5.5 Clone Function

*/
    CHalfSegment*  Clone() {return (new CHalfSegment(*this));}
    
/*
5.6 Intersects Function

*Semantics:*  These two functions decide whether two half segments intersect with or cross each other. 
Since there is no Realm here as a precondition, two half segments may intersect each other in their middle,
which is decided by ~cross~. That is: cross=intersect with middle points. 

*Complexity:* $O( 1 )$ 

*/
    const bool Intersects( const CHalfSegment& chs) const;    
/*
This first intersects function compute whether two half segments intersect each other. 

*/
    const bool Intersects( const CHalfSegment& chs, CHalfSegment& reschs) const;
/*
This second intersects function compute whether two half segments intersect each other. 
If they intersect, then the intersected part will be returned so that the function initiates the 
call can know where they intersect. The intersected part is a segment. If it is a point, then
the two endpoints of the segment will be the same.

*/
    const bool cross( const CHalfSegment& chs ) const;
/*
This third intersect function, named as ~cross~, compute whether two half segments intersect
 with their mid-points. If they only meet with their endpoints, they are considered as "not cross".

*/    
    
    const bool crossings( const CHalfSegment& chs, Point& p ) const;
/*
This last intersect function, named crossings, is ued for the ~crossings~ operator. It computes 
whether two half segments crossing each other. If they do not intersect, or they are parellel, 
they are consider not crossing each other. 

*/
    
/*
5.7 Inside Function

*Semantics:* This operation computes whether one half segment is inside another. If segment A is part of 
another segment B, then we say A is inside B. eg. -------~------~-------.

*Complexity:* $O( 1 )$ 

*/
    const bool Inside(const CHalfSegment& chs) const ;
/*
5.8 Contain Function

*Semantics:* This operation computes whether one point is contained in a half segment. If a point P is inside 
a segment S, then we say P is contained by S. eg. ---------o---------.

*Complexity:* $O( 1 )$ 

*/
    const bool Contains( const Point& p ) const;
/*
5.9 rayAbove Function

*Semantics:* This function decides whether a half segment is above a point. This is 
useful when we want to decide whether a point is inside a face or region.

*Complexity:* $O( 1 )$ 

*/
    const bool rayAbove( const Point& p, double &abovey0 ) const;
    
/*
5.10 Operation ~distance~ (with ~point~)

*Precondition:* ~u.IsDefined()~

*Semantics:*  compute the distance between a line segment and a given point

*Complexity:* $O(1)$ 

*/
    double distance( const Point& p ) const;
    
/*
5.11 attribute comparison Functions

*Semantics:* These two operations compare two half segments according to their attribute values. They are 
used for the logicsort() function.

*Complexity:* $O( 1 )$ 

*/
    int logicless(const CHalfSegment& chs) const;
    int logicgreater(const CHalfSegment& chs) const;
    
  private:
/*
5.11 Properties

*/
    bool defined;
/*
This boolean property indicates whether the half segment is defined.

*/  
    bool ldp; 
/*
This boolean property indicates whether the half segment has its left point as its dominating point.

*/  
    Point lp;
    Point rp;
/*
These two properties give the left and right point of the half segment.

*/  
    public:    
    attrtype attr;
/*
This ~attribute~ property is useful if we process region values in a way similar to that indicated in the ROSE
paper. Currently the ~attribute~ value is composed of a set of int, but it will soon be implemented as a structure
 which contains more information.

*/  
};
/*
5.12 overloaded output operator

*/
ostream& operator<<( ostream& o, const CHalfSegment& chs );

/*
6 Class Line

This class implements the memory representation of the ~line~ type constructor.

*/

class CLine
{
  public:
/*
6.1 Constructors and Destructor

A ~line~ value is a set of halfsegments. In the external (nestlist) representation, a line value is 
expressed as a set of segments. However, in the internal (class) representation, it is expressed
as a set of sorted halfsegments, which are stored as a PArray.

*/    
    CLine(SmiRecordFile *recordFile);
    CLine(SmiRecordFile *recordFile, const CLine& cl );
    CLine(SmiRecordFile *recordFile, const SmiRecordId recordId, 
	 bool update = true );
    void Destroy();
    ~CLine();
    
/*
6.2 Functions Reading Property Values from an Object

*/    
    const bool IsOrdered() const;
/*
judges whether the half segments in the line value is sorted.

*/    
    const bool IsEmpty() const;
/*
judges whether the line value is empty.

*/        
    const int Size() const;
/*
returns the number of half segments in the line value.

*/        
    void Get( const int i, CHalfSegment& chs ) const;
/*
reads the ith half segment from the line value.

*/        
    const SmiRecordId GetLineRecordId() const;
/*
gets the Record ID of the PArray which store half segments of the line value.

6.3 Bulkload Functions

*/        
    void StartBulkLoad();
/*
Marks the begin of a bulk load of line relaxing the condition that the half segments must be 
ordered.

*/

    void EndBulkLoad();
/*
Marks the end of a bulk load and sorts the half segments.

6.4 Overloaded Class Operators

6.4.1 Operation $=$ (~assignment~)

*Semantics:* $this = cl$. It assigns the value of a line to another.

*Complexity:* $O( n )$ 

*/
    CLine& operator=(const CLine& cl);
/*
6.4.2 Operation $==$ (~equal~)

*Semantics:* $this ==cl$. It decides whether two line values are identical.

*Complexity:* $O( n )$ 

*/
    int operator==(const CLine& cl) const;
/*
6.4.3 Operation $+=$ (~Union~)

*Semantics:* $this +=chs$. It adds a half segment into a line value.

*Complexity:* $O( 1 )$ , if the set is not ordered; and $O( log(n)+n)$ , if the set is ordered.

*/
    CLine& operator+=(const CHalfSegment& chs);
/*
6.4.4 Operation $-=$ (~Minus~)

*Semantics:* $this -=chs$. It deletes a half segment from a line value.

*Complexity:* $O( log(n)+n)$ .

*/
    CLine& operator-=(const CHalfSegment& chs);
/*

6.5 Clone Function

*/
    CLine*    Clone();
    
/*
6.6 Object Traversal Functions

*Semantics:* The following functions are implemented as basic utilities. They will be useful 
if we want to adapt our algorithms to object-traversal-based ones.

*Complexity:* All these functions have a complexity of $O( 1 )$ .

*/
    void SelectFirst();
/*
put the pointer ~pos~ to the first half segment in the line value.

*/
    void SelectNext();
/*
move the pointer ~pos~ to the next half segment in the line value.

*/
    bool EndOfHs();
/*
decide whether ~pos~ is -1, which indicates that no more half segments in the line value
 need to be processed.

*/
    void GetHs( CHalfSegment& chs );
/*
get the current half segment from the line value according to the ~pos~ pointer.

*/
    void InsertHs( CHalfSegment& chs );
/*
insert a half segment into the line value, and put the ~pos~ pointer to this newly inserted 
half segment.

*/
    
  private:
/*
6.7 Private member functions

*/
    void Sort();
    void QuickSortRecursive( const int low, const int high );
/*
Sorts (quick-sort algorithm) the persistent array of half segments in the line value.

*/
    const int Position(const CHalfSegment&) const;
/*
Searches (binary search algorithm) for a half segment in the line value and
returns its position. Returns -1 if the half segment is not found.

6.8 Atrtibutes

*/
    PArray<CHalfSegment>* line;
/*
The persisten array of half segments.

*/  
    int pos;
/*
The pointer to the current half segments. The pointer is important in object traversal algorithms.

*/  
    bool ordered;
/*
Whether the half segments in the line value are sorted.

*/  
};

/*
6.9 overloaded output operator

*/
ostream& operator<<( ostream& o, const CLine& cl );

/*
7 Class Region

This class implements the memory representation of the ~region~ type constructor.

*/

class CRegion
{
  public:
/*
7.1 Constructors and Destructor

A ~region~ value is a set of halfsegments. In the external (nestlist) representation, a region value is 
expressed as a set of faces, and each face is composed of a set of cycles.  However, in the internal 
(class) representation, it is expressed as a set of sorted halfsegments, which are stored as a PArray.

The system will do the basic check on the validity of the region data (see the explaination of the 
insertOK() function). 

*/    
    
    CRegion(SmiRecordFile *recordFile);
    CRegion(SmiRecordFile *recordFile, const CRegion& cr );
    CRegion(const CRegion& cr, SmiRecordFile *recordFile );
    CRegion(SmiRecordFile *recordFile, const SmiRecordId recordId, 
	     bool update = true );
    void Destroy();
    ~CRegion();
    
/*
7.2 Functions Reading Property Values from an Object

*/    
    const bool IsOrdered() const;
/*
decides whether the half segments in the region value is sorted.

*/
    const bool IsEmpty() const;
/*
decides whether the region value is empty.

*/        
    const int Size() const;
/*
returns the number of half segments in the region value.

*/      
    void Get( const int i, CHalfSegment& chs ) const;
/*
reads the ith half segment from the region value.

*/            
    const SmiRecordId GetRegionRecordId() const;
/*
gets the Record ID of the PArray which store half segments of the line value.

7.3 Bulkload Functions

*/      
    void StartBulkLoad();
/*
Marks the begin of a bulk load of line relaxing the condition that the half segments must be 
ordered.

*/    
    void EndBulkLoad();
/*
Marks the end of a bulk load and sorts the half segments.

7.4 Validity Checking Function

*/    
    const bool insertOK(const CHalfSegment& chs);
/*
This function check whether a region value is valid after the insertion of a new half segment.
Whenever a half segment is about to be inserted, the state of the region is checked. 
A valid region must satisfy the following conditions:

1)  any two cycles of the same region must be disconnect, which means that no edges 
of different cycles can intersect each other;

2) edges of the same cycle can only intersect with their endpoints, but no their middle points;

3)  For a certain face, the holes must be inside the outer cycle;

4)  For a certain face, any two holes can not contain each other;

5)  Faces must have the outer cycle, but they can have no holes;

6)  for a certain cycle, any two vertex can not be the same;

7)  any cycle must be made up of at least 3 edges;

8)  It is allowed that one face is inside another provided that their edges do not intersect.


7.5 Overloaded Class Operators

7.5.1 Operation $=$ (~assignment~)

*Semantics:* $this = cr$. It assigns the value of a region to another.

*Complexity:* $O( n )$ 

*/    
    CRegion& operator=(const CRegion& cr);
/*
7.5.2 Operation $==$ (~equal~)

*Semantics:* $this ==cr$. It decides whether two region values are identical.

*Complexity:* $O( n )$ 

*/    
    int operator==(const CRegion& cr) const;
/*
7.5.3 Operation $+=$ (~Union~)

*Semantics:* $this +=chs$. It adds a half segment into a region value.

*Complexity:* $O( 1 )$ , if the set is not ordered; and $O( log(n)+n)$ , if the set is ordered.

*/    
    CRegion& operator+=(const CHalfSegment& chs);
/*
7.5.4 Operation $-=$ (~Minus~)

*Semantics:* $this -=chs$. It delete a half segment from a region value.

*Complexity:* $O( log(n)+n)$ .

*/    
    CRegion& operator-=(const CHalfSegment& chs);
/*

7.6 Clone Function

*/    
    CRegion*    Clone();
    
/*
7.7 Object Traversal Functions

*Semantics:* The following functions are implemented as basic utilities. They will be useful 
if we want to adapt our algorithms to object-traversal-based ones.

*Complexity:* All the following functions have a complexity of $O( 1)$ .

*/    
    void SelectFirst();
/*
Put the pointer ~pos~ to the first half segment in the region value.

*/    
    void SelectNext();
/*
Move the pointer "pos" to the next half segment in the region value.

*/    
    bool EndOfHs();
/*
decide whether ~pos~ is -1, which indicates that no more half segments in the region value
 need to be processed.

*/    
    void GetHs( CHalfSegment& chs );
/*
get the current half segment from the region value according to the ~pos~ pointer.

*/    
    void InsertHs( CHalfSegment& chs );
/*
Insert a half segment into the region value, and put the ~pos~ pointer to this newly inserted 
half segment.

*/    
    const attrtype& GetAttr();
/*
read the ~attr~ value of the current half segment from the region value. The current 
half segment is indicated by ~pos~

*/        
    void UpdateAttr( attrtype& attr );
/*
update the ~attr~ value of the current half segment from the region value.The current 
half segment is indicated by ~pos~

*/     
    bool contain( const Point& p ) const;
/*
decide whether a point is inside the region.

*/ 
    bool contain( const CHalfSegment& chs ) const;
/*
decide whether a half segment is inside the region.

*/ 
    bool holeedgecontain( const CHalfSegment& chs ) const;
/*
to decide whether a half segment is inside a hole edge of the region.

*/     
    void logicsort();
    void logicQuickSortRecursive( const int low, const int high );
/*
these two function are used to sort the half segments according to their attributes;

*/    
    
  private:
/*
7.8 Private member functions

*/    
    void Sort();
    void QuickSortRecursive( const int low, const int high );
/*
sorts (quick-sort algorithm) the persistent array of half segments in the region value.

*/    
    const int Position(const CHalfSegment&) const;
/*
searches (binary search algorithm) for a half segment in the region value and
returns its position. Returns -1 if the half segment is not found.

7.9 Atrtibutes

*/    
    PArray<CHalfSegment>* region;
    
/*
The persisten array of half segments.

*/      
    int pos;
/*
The pointer to the current half segments. The pointer is important in object traversal algorithms.

*/      
    bool ordered;
/*
Whether the half segments in the region value are sorted.

*/ 
};

/*
7.10 overloaded output operator

*/
ostream& operator<<( ostream& o, const CRegion& cr );

#endif // __SPATIAL_ALGEBRA_H__


/*
8 Operations of the Spatial Algebra

See ~Operations to be Implemented in the Spatial Algebra~ for the discription of operations of the spatial algebra.

*/
