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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]

[1] Header File of the NauticalMap Algebra

February, 2003 Victor Teixeira de Almeida

March-July, 2003 Zhiming DING
      
1 Overview

This header file essentially contains the definition of the classes ~Point~,
~Points~, ~Line~, and ~Region~ used in the Spatial Algebra. These classes
respectively correspond to the memory representation for the type constructors
~point~, ~points~, ~line~, and ~region~.  Figure \cite{fig:spatialdatatypes.eps}
shows examples of these spatial data types.

2 Defines and includes

*/
#ifndef __NAUTICALMAP_ALGEBRA_H__
#define __NAUTICALMAP_ALGEBRA_H__

#include "StandardAttribute.h"
//#include "StandardTypes.h"
#include "DBArray.h"
#include "RectangleAlgebra.h"
#include "SpatialAlgebra.h"

 /* //#define RATIONAL_COORDINATES
#define DOUBLE_COORDINATES

#ifdef DOUBLE_COORDINATES
typedef double Coord;
#else

#ifdef RATIONAL_COORDINATES
#ifdef WITH_ARBITRARY_PRECISION

#include "Rational.h"

typedef Rational Coord;
#else
typedef Rational Coord;
#endif
#else
typedef long Coord;
#endif

#endif

 */

/*
There are two main defines that will control how the coordinate system is
implemented, namely ~RATIONAL\_COORDINATES~ and ~WITH\_ARBITRARY\_PRECISION~.
The first one defines if the system will use rational coordinates instead
of simple integer (~long~) ones. The second one is used only if rational
coordinates are being used and defines if the system will use rational
numbers with arbitrary precision (~not implemented yet~) instead of fixed
precision (~Rational~).

3 Class StandardNauticalMapAttribute

*/
class StandardNauticalMapAttribute : public StandardAttribute
{
  public:
    virtual const Rectangle BoundingBox() const = 0;
};

/*
3 Class NauticalObject

At first a data structure for storing an ~nauticalObject~ in the main memory is
defined. The object is represented as a storage Word (which is often a pointer t
o the actual object).

*/

class NauticalObjects;
/*
A forward declaration of the class ~NauticalObjects~.

*/

class NauticalObject: public StandardSpatialAttribute
{
  public:
/*
3.1. Constructors and Destructor

There are two ways of constructing a NauticalObject:

*/
    NauticalObject();
/*
This constructor should not be used. 

*/
    NauticalObject( const bool d, const char* oName, const Point& oPosition);
/*
The first one receives a boolean value ~d~ indicating if the nauticalObject is 
defined. ~oName~ represents the object name and ~oPosition~ includes the
object position. 

*/
    NauticalObject( const NauticalObject& nObject );
/*
The second one receives a NauticalObject ~nobject~ as argument and creates a 
NauticalObject that is a copy of ~nobject~.

*/
    ~NauticalObject();
/*
The destructor.

3.2 Member functions

*/
     const char* GetObjectName() const;
/*
Returns the ~name~ coordinate of the nauticalobject.

*Precondition:* ~IsDefined()~

*/
    const Point& GetPosition() const;
/*
Returns the ~position~ coordinate of the nauticalobject.

*Precondition:* ~IsDefined()~

*/
    const Rectangle BoundingBox() const;
/*
Returns the nauticalobject bounding box which is also a point.

*/

void SetObjectName(const char* oName);
/*
Sets the object name for the nauticalobject.

*/

void SetPosition(const Point& oPosition);
/*
Sets the object position for the nauticalobject.

*/

void Set(const char* oName, const Point& oPosition);
/*

This function set the value of the nautical object.

*/
    NauticalObject& operator=(const NauticalObject& n);
/*
Assignement operator redefinition.

*Precondition:* ~n.IsDefined()~

*/

/*
Sets the point to defined or undefined depending on the value of ~d~.

3.3 Operations

3.3.1 Operation $=$ (~equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u = v$

*Complexity:* $O(1)$

*/
    bool operator==(const NauticalObject& n) const;
/*
3.3.2 Operation $\neq$ (~not equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \neq v$

*Complexity:* $O(1)$

*/
    bool operator!=(const NauticalObject& n) const;
/*
3.3.3 Operation $\leq$ (~less or equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \leq v$

*Complexity:* $O(1)$

*/
    bool operator<=(const NauticalObject& n) const;
/*
3.3.4 Operation $<$ (~less than~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u < v$

*Complexity:* $O(1)$

*/
    bool operator<(const NauticalObject& n) const;
/*
3.3.5 Operation $\geq$ (~greater or equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \geq v$

*Complexity:* $O(1)$

*/
    bool operator>=(const NauticalObject& n) const;
/*
3.3.6 Operation $>$ (~greater than~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u > v$

*Complexity:* $O(1)$

*/
    bool operator>(const NauticalObject& n) const;
/*
3.3.7 Operation ~inside~

*Precondition:* ~u.IsDefined()~

*Semantics:* $u \in V$

*Complexity:* $O(log(n))$, where ~n~ is the size of the nauticalobject set ~V~

*/
//    bool Inside( NauticalObject& ns ) const;
/*
3.3.8 Operation ~intersection~ (with ~nauticalobject~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* if $u = v$ then $u$ else $\perp$

*Complexity:* $O(1)$

*/
//    void Intersection( const NauticalObject& n, NauticalObject& result ) const;
/*
3.3.9 Operation ~intersection~ (with ~nauticalobjects~)

*Precondition:* ~u.IsDefined()~

*Semantics:* if $u \in V$ then $u$ else $\perp$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
//    void Intersection( NauticalObjects& ns, NauticalObject& result ) const;
/*
3.3.10 Operation ~minus~ (with ~point~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* if $u = v$ then $\perp$ else $u$

*Complexity:* $O(1)$

*/
//    void Minus( const Point& p, Point& result ) const;
/*
3.3.9 Operation ~minus~ (with ~points~)

*Precondition:* ~u.IsDefined()~

*Semantics:* if $u \in V$ then $\perp$ else $u$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
//    void Minus( Points& ps, Point& result ) const;
/*
3.3.10 Operation ~distance~

*Precondition:* ~u.IsDefined()~ and ~v.IsDefined()~

*Semantics:* compute the distance between u and v

*Complexity:* $O(1)$

*/
//    double distance( const NauticalObject& n ) const;
/*
3.3.11 Functions needed to import the the ~NauticalObject~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    bool     IsDefined() const;
    void     SetDefined(bool Defined);
    size_t   HashValue();
    void     CopyFrom(StandardAttribute* right);
    int      Compare(Attribute * arg);
    bool     Adjacent(Attribute * arg);
    int      Sizeof() const;
    NauticalObject*    Clone();
    ostream& Print( ostream &os );

/*
3.4 Attributes

*/
    private:

    char name[49];
/*
The ~name~ of the nautical object.

*/
    Point position;
/*
The ~position~ of the nautical object.

*/
    bool defined;
/*
A flag that tells if the nautical object is defined or not.

*/

};

typedef NauticalObject CNauticalObject;


/*
4 Class NauticalObjects

This class implements the memory representation of the ~nauticalobjects~ type 
constructor.  A nauticalobjects value is a finite set of nauticalobjects. 
An example of a nauticalobjects value can be seen in the Figure 
\cite{fig:spatialdatatypes.eps}.

The implementation of the nauticalobjects type constructor is a persistent array
of nauticalobjects ordered by lexicographic order.

*/
class NauticalObjects: public StandardSpatialAttribute
{
  public:
/*
4.1 Constructors and Destructor

There are three ways of constructing a point set:

*/
    NauticalObjects() {}
/*
This constructor should not be used.

*/
    NauticalObjects( const int initsize );
/*
The first one constructs an empty nautical object set but open space for 
~initsize~ nautical objects.

*/
    NauticalObjects( NauticalObjects& ns);
/*
The second one receives another nautical object set ~ns~ as argument and
constructs a nautical object set which is a copy of ~ns~. 

*/
    void Destroy();
/*
This function should be called before the destructor if one wants to destroy the
persistent array of nautical objects. It marks the persistent array for 
destroying. The destructor will perform the real destroying.

*/
    ~NauticalObjects();
/*
The destructor.

4.2 Functions for Bulk Load of Points

As said before, the nautical object set is implemented as an ordered persistent
array of nautical objetcs.  The time complexity of an insertion operation in an ordered array is $O(n)$, where ~n~ is the size of the nautical object set. In 
some cases, bulk load of nautical objects for example, it is good
to relax the ordered condition to improve the performance. We have relaxed this
ordered condition only for bulk load of nautical objects. All other operations 
assume that the nautical object set is ordered.

*/
    bool IsOrdered() const;
/*
Returns if the nautical object set is ordered. There is a flag ~ordered~ (see 
attributes) in order to avoid a scan in the nautical object set to answer this 
question.

*/
    void     setOrdered(bool isordered);

    void StartBulkLoad();
/*
Marks the begin of a bulk load of nautical objects relaxing the condition that
the nautical objects must be ordered.

*/
    void EndBulkLoad();
/*
Marks the end of a bulk load and sorts the nautical object set.

4.3 Member functions

*/
    const Rectangle BoundingBox() const;
/*
Returns the bounding box that spatially contains all nautical objects.

*/
    bool IsEmpty() const;
/*
Returns if the set is empty of not.

*/
    int Size() const;
/*
Returns the size of the point set. Returns ~0~ if the set is empty.

*/
    void Get( const int i, NauticalObject& n );
/*
Retrieves the nautical object ~n~ at position ~i~ in the nautical object set.

*Precondition:* $0 \leq i < Size()$

*/
    NauticalObjects& operator=(NauticalObjects& ns);
/*
Assignement operator redefinition.

*Precondition:* ~ps.IsOrdered()~

*/
    bool Contains(const NauticalObject& n);
/*
Searches (binary search algorithm) for a nautical object in the nautical object
set and return ~true~ if found and ~false~ if not.

*Precondition:* ~this.IsOrdered() $\&\&$ n.IsDefined()~

*/
    bool Contains(NauticalObjects& ns);
/*
Returns ~true~ if this nautical object set contains the ~ns~ point set and
~false~ otherwise.

*Precondition:* ~this.IsOrdered() $\&\&$ ns.IsOrdered()~

4.4 Operations

4.4.1 Operation $=$ (~equal~)

*Precondition:* ~this.IsOrdered() $\&\&$ ns.IsOrdered()~

*Semantics:* $this = ns$

*Complexity:* $O(n+m)$, where ~n~ is the size of this nautical object set and m
the size of ~ns~.

*/
    bool operator==(NauticalObjects&);
/*
4.4.2 Operation $\neq$ (~not equal~)

*Precondition:* ~this.IsOrdered() $\&\&$ ns.IsOrdered()~

*Semantics:* $this = ns$

*Complexity:* $O(n+m)$, where ~n~ is the size of this nautical object set and m
the size of ~ns~.

*/
    bool operator!=(NauticalObjects&);
/*
4.4.3 Operation ~union~ (with ~nautical object~)

*Precondition:* ~n.IsDefined()~

*Semantics:* $this \cup \{n\}$

*Complexity:* $O(1)$, if the set is not ordered, and $O(log(n)+n)$, otherwise. 
~n~ is the size of this nautical object set.

*/
    NauticalObjects& operator+=(const NauticalObject& n);
/*
4.4.4 Operation ~union~ (with ~nautical object~)

*Precondition:*

*Semantics:* $this \cup rnps$

*Complexity:* $O(m)$, if the set is not ordered, and $O(m+(m+n)log(m+n))$, 
otherwise. ~n~ is the size of this nautical object set and ~m~ is the size of 
~ns~.

*/
    NauticalObjects& operator+=(NauticalObjects& ns);
/*
4.4.5 Operation ~minus~ (with ~point~)

*Precondition:* ~p.IsDefined() $\&\&$ this.IsOrdered()~

*Semantics:* $this \backslash \{p\}$

*Complexity:* $O(log(n)+n)$

*/
    NauticalObjects& operator-=(const NauticalObject& n);
/*
4.4.6 Operation ~inside~

*Precondition:* ~this.IsOrdered() $\&\&$ ns.IsOrdered()~

*Semantics:* $this \subseteq ns$

*Complexity:* $O(n+m)$, where ~n~ is the size of this nautical object set and m
 the size of ~ns~.

*/
    bool Inside(NauticalObjects& ns);
/*
4.4.7 Operation ~intersects~

*Precondition:* ~this.IsOrdered() $\&\&$ ns.IsOrdered()~

*Semantics:* $this \cap ns$

*Complexity:* $O(n+m)$, where ~n~ is the size of this nautical object set and m
 the size of ~ns~.

*/
    bool Intersects(NauticalObjects& ns);
/*
4.4.8 Object Traversal Functions

*Precondition:* ~this.IsOrdered()~

*Semantics:*  These functions are object traversal functions which is useful when we are
using ROSE algebra algorithms (DZM).

*Complexity:* $O(1)$.

*/
    void SelectFirst();
    void SelectNext();
    bool EndOfPt();
    void GetPt( NauticalObject& nobject );
    void InsertPt( NauticalObject& nobject );

    void     Clear();

    private:
/*
4.5 Private member functions

*/
    void Sort();
/*
Sorts the persistent array of nautical objects.

*/
    int Position(const NauticalObject&);
/*
Searches (binary search algorithm) for a nautical object in the nautical object
set and returns its position. Returns -1 if the nautical object is not found.

4.6 Functions needed to import the the ~NauticalObjects~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    int NumOfFLOBs();
    FLOB *GetFLOB(const int i);

    bool     IsDefined() const;
    void     SetDefined(bool Defined);
    size_t   HashValue();
    void	   CopyFrom(StandardAttribute* right);
    int      Compare(Attribute * arg);
    bool     Adjacent(Attribute * arg);
    int      Sizeof() const;
    NauticalObjects*    Clone() ;
    ostream& Print( ostream &os );

/*
4.6 Attributes

*/
    DBArray<NauticalObject> nobjects;
/*
The persistent array of nautical objects.

*/
    Rectangle bbox;
/*
The bounding box that spatially contains all nautical objects.

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

typedef NauticalObjects CNauticalObjects;

/*
4.7 Auxiliary functions

*/
ostream& operator<<( ostream& o, const NauticalObject& n );
/*
Print the nautical object ~n~ in the out stream ~o~.

*/
ostream& operator<<( ostream& o, NauticalObjects& ns );
/*
Print the point set ~ns~ int the out stream ~o~.

*/

class NauticalMap
{
   public:
     NauticalMap(const char *mName = 0, int mScale =1, Relation *nobjects = 0, Relation *nlines = 0, Relation *nregions = 0);
     ~NauticalMap();
     void Destroy();
     const char* GetMapName() const;
     void        SetMapName(const char* oName);
     const int GetMapScale() const;
     void      SetMapScale(const int scale);
     Word      GetObjects();
     Word      GetLines();
     Word      GetRegions();

    static ListExpr GetObjectsTypeInfo();
    static ListExpr GetLinesTypeInfo();
    static ListExpr GetRegionsTypeInfo();


    static ListExpr objectsTypeInfo;
    static ListExpr linesTypeInfo;
    static ListExpr regionsTypeInfo;

   private:
     static void CreateAllTypeInfos();
     char     name[49];
     int      scale;
     Relation *objects;
     Relation *lines;
     Relation *regions;
};



/*
8 Operations of the Nautical Algebra

See ~Operations to be Implemented in the Nautical Algebra~ for the discription 
of operations of the nautical algebra.

*/

#endif // __NAUTICALMAP_ALGEBRA_H__
