/*
----
This file is part of SECONDO.

Copyright (C) 2017, 
University in Hagen, 
Faculty of Mathematics and Computer Science,
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

//[_] [\_]

*/

#ifndef LINET_H
#define LINET_H


#include "PointsT.h"
#include "RegionT.h"
#include "../../Tools/Flob/DbArray.h"

#include <stack>


/*
6 Class Line

This class implements the memory representation of the ~line~ type constructor.
A line value is actually composed of a set of arbitrarily arranged line 
segments. In the ROSE algebra paper, it is called ~lines~.

A ~line~ value is a set of half segments. In the external (nested list) 
representation, a line value is expressed as a set of segments. However, in the
internal (class) representation, it is expressed
as a set of sorted half segments, which are stored in a DBArray.

*/


template<template<typename T>class ArrayT>
class LineT: public StandardSpatialAttribute<2>
{
  public:
/*
6.1 Constructors and Destructor

*/
    explicit LineT( const int n );
/*
Constructs an empty line allocating space for ~n~ half segments.

*/
    LineT( const LineT<ArrayT>& cl );
/*
The copy constructor.

*/
    void Destroy();
/*
This function should be called before the destructor if one wants to destroy the
persistent array of half segments. It marks the persistent array for destroying.
The destructor will perform the real destroying.

*/
    inline ~LineT() {
    }
/*
The destructor.

6.2 Functions for Bulk Load

As said before, the line is implemented as an ordered persistent array of half 
segments.  The time complexity of an insertion operation in an ordered array
is $O(n)$, where ~n~ is the number of half segments. In some cases, bulk load 
of segments for example, it is good to relax the ordered condition to improve
the performance. We have relaxed this ordered condition only for bulk load of
half segments. All other operations assume that the set of half segments is 
ordered.

*/
    bool IsOrdered() const;
/*
Returns whether the set of half segments is ordered. There is a flag 
~ordered~ (see attributes) in order to avoid a scan in the half segments set 
to answer this question.

*/
    void StartBulkLoad();
/*
Marks the begin of a bulk load of half segments relaxing the condition that 
the points must be ordered.

*/
     void EndBulkLoad (const bool sort = true,
                       const bool realminize = true,
                       const bool robust = false);
/*

Marks the end of a bulk load for this line.
If all parameters are set to __true__, the only condition to the content
of the Halfsegment array is that for each segment both corresponding 
Halfsegments are included.

If ~sort~ is set to __false__, the halfsegments must be sorted using the
halfsegment order.

If ~realminize~ is set to __false__, the halfsegments has to be realminized.
This means each pair of different halfsegments has at most a common endpoint.
Furthermore, the edge numbers of the halfsegments must be the same for the
two halfsegments of a segment. The allowed range for the edge numbers 
is [0..Size()/2-1].

*/

/*
6.2 Member functions

*/

/*
length computed for metric (X,Y)-coordinates

*/
    double Length() const;

/*
length computed for geographic (LON,LAT)-coordinates and a Geoid
If any coordinate is invalid, ~valid~ is set to false (true otherwise).

*/
    double Length(const Geoid& g, bool& valid) const;

/*
Returns the length of the line, i.e. the sum of the lengths of all segments.

*/
    inline double SpatialSize() const{
      return Length();
    }

/*
Returns the length computed for geographic (LON,LAT)-coordinates and a Geoid
If any coordinate is invalid, ~valid~ is set to false (true otherwise).

*/
    inline double SpatialSize(const Geoid& g, bool& valid) const{
      return Length(g, valid);
    }


//    inline void SetLength( double length );
/*
Sets the length of the line.

*/
    const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;
/*
Returns the bounding box of the line.

*/
  //  inline void SetBoundingBox( const Rectangle<2>& bbox );
/*
Sets the bounding box of the line.

*/
    bool IsEmpty() const;
/*
Returns true iff the line is undefined or empty.

*/
    int Size() const;
/*
Returns the number of half segments in the line value.

*/
    bool Contains( const Point& p, const Geoid* geoid=0 ) const;
/*
Checks whether the point ~p~ is contained in the line

*/
    void Get( const int i, HalfSegment& hs ) const;
/*
Reads the ith half segment from the line value.

*/

    void Resize(const int newSize);
/*
Sets the new capacity of the halfsegment array to the
maximum of its original size and the argument.

*/
    void TrimToSize();
/*
Sets the new capacity of the halfsegment array to the
amount really required.

*/

    void Put( const int i, const HalfSegment& hs );
/*
Writes the the half segment ~hs~ to the ith position.

*/
    LineT& operator=( const LineT<ArrayT>& cl );
/*
Assignement operator redefinition.

6.4 Operations

6.4.1 Operation $=$ (~equal~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U == V$

*Complexity:* $O(m + n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool operator==( const LineT<ArrayT>& cl ) const;
/*
6.4.2 Operation $\neq$ (~not equal~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U != V$

*Complexity:* $O(m + n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool operator!=( const LineT<ArrayT>& cl ) const;
/*
6.4.3 Operation ~union~

*Semantics:* $U \cup \{v\}$

*Complexity:* $O( 1 )$, if the set is not ordered; and $O(\log n + n)$, 
otherwise; where ~n~ is the size of ~U~.

*/
    LineT<ArrayT>& operator+=( const HalfSegment& hs );


/*
6.4.4 Oeration ~plus~

Appends all halfsegments from l to that line.
This instance must must be in bulkload mode.

*/
   LineT<ArrayT>& operator+=(const LineT<ArrayT>& l);

/*
6.4.4 Operation ~minus~

*Precondition:* ~U.IsOrdered()~

*Semantics:* $U \ \{v\}$

*Complexity:* $O(log(n)+n)$, where ~n~ is the size of ~U~.

*/
    LineT<ArrayT>& operator-=( const HalfSegment& hs );
/*
6.4.4 Operation ~intersects~ (with ~line~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
  bool Intersects( const LineT<ArrayT>& l, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~intersects~ (with ~region~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O(m(n + \log n))$, where ~m~ is the size of ~U~ and ~n~ 
the size of ~V~.

*/
bool Intersects( const RegionT<ArrayT>& r, const Geoid* geoid=0 ) const;



/*
6.4.4 Operation ~inside~ (with ~line~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \in V$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Inside( const LineT<ArrayT>& l, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~inside~ (with ~region~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \in V$

*Complexity:* $O(m.log(n))$, where ~m~ is the size of ~U~ and ~n~ the 
size of ~V~.

*/
    bool Inside( const RegionT<ArrayT>& r, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~adjacent~ (with ~region~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $\partial U \cap \partial V \neq \emptyset \land 
U^0 \cap V^0 = \emptyset$

*Complexity:* $O(n.m)$, where ~n~ is the size of ~U~ and m the size of ~V~.

*/
    bool Adjacent( const RegionT<ArrayT>& r, const Geoid* geoid=0 ) const;

/*
6.4.4 Operation ~intersection~

*/
  void Intersection(const Point& p, PointsT<ArrayT>& result, 
                    const Geoid* geoid=0) const;
  void Intersection(const PointsT<ArrayT>& ps, PointsT<ArrayT>& result,
                    const Geoid* geoid=0) const;
  void Intersection(const LineT<ArrayT>& l, LineT<ArrayT>& result, 
                    const Geoid* geoid=0 ) const;
  void Intersection( const RegionT<ArrayT>& l, LineT<ArrayT>& result,
                     const Geoid* geoid=0 ) const;
  void Intersection( const SimpleLineT<ArrayT>& l, SimpleLineT<ArrayT>& result,
                     const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~minus~

*/
  void Minus( const Point& l, LineT<ArrayT>& result, 
              const Geoid* geoid=0 ) const;
  void Minus( const PointsT<ArrayT>& l, LineT<ArrayT>& result, 
              const Geoid* geoid=0 ) const;
  void Minus( const LineT<ArrayT>& l, LineT<ArrayT>& result, 
              const Geoid* geoid=0 ) const;
  void Minus( const RegionT<ArrayT>& l, LineT<ArrayT>& result, 
              const Geoid* geoid=0 ) const;
  void Minus( const SimpleLineT<ArrayT>& l, LineT<ArrayT>& result, 
              const Geoid* geoid=0 ) const;

/*
6.4.4 Operation ~union~

*/
  void Union( const Point& l, LineT<ArrayT>& result, 
              const Geoid* geoid=0 ) const;
  void Union( const PointsT<ArrayT>& l, LineT<ArrayT>& result, 
              const Geoid* geoid=0 ) const;
  void Union( const LineT<ArrayT>& l, LineT<ArrayT>& result, 
              const Geoid* geoid=0 ) const;
  void Union( const RegionT<ArrayT>& l, RegionT<ArrayT>& result, 
              const Geoid* geoid=0 ) const;
  void Union( const SimpleLineT<ArrayT>& l, LineT<ArrayT>& result, 
              const Geoid* geoid=0 ) const;

/*
6.4.5 Operation ~crossings~

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\{ p \in U \cap V | p \textrm{ is isolated in } U \cap V \}$

*Complexity:* $O(m.n + r\log r)$, where ~m~ is the size of ~U~, ~n~ the size 
of ~V~, and ~r~ is the ~points~ result size.

*/
  void Crossings( const LineT<ArrayT>& l, PointsT<ArrayT>& result, 
                  const Geoid* geoid=0 ) const;


/*
6.4.5 Operation ~Crossings~

This operation returns all internal crossing nodes, i.e. all
points where more than 2 segments have a common endpoint.

*/
  void Crossings(PointsT<ArrayT>& result, const Geoid* geoid=0) const;

/*
6.4.5 Operation ~distance~ (with ~point~)

*Precondition:* ~!U.IsEmpty() and v.IsDefined()~

*Semantics:* $\min \{ dist(u, v) | u \in U \}$

*Complexity:* $O(n)$, where ~n~ is the size of ~U~.

*/
    double Distance( const Point& p, const Geoid* geoid=0 ) const;
    double MaxDistance(const Point& p, const Geoid* geoid=0 ) const;
/*
6.4.5 Operation ~distance~ (with ~points~)

*Precondition:* ~!U.IsEmpty() and !V.IsEmpty()~

*Semantics:* $\min \{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ is the size of ~V~.

*/
    double Distance( const PointsT<ArrayT>& ps, const Geoid* geoid=0 ) const;
/*
6.4.5 Operation ~distance~ (with ~line~)

*Precondition:* ~!U.IsEmpty() and !V.IsEmpty()~

*Semantics:* $\min \{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ is the size of ~V~.

*/
    template<class LineType>
    double Distance(const LineType& l , const Geoid* geoid=0) const;


    void DistanceSmallerThan(const LineT<ArrayT>& l,
                            const double  maxDist,
                            const bool allowEqual,
                            CcBool& result,
                            const Geoid* geoid=0) const;


/*
6.4.5 Operation ~distance~ (with ~rect2~)

*Precondition:* ~!U.IsEmpty() and !V.IsEmpty()~

*/
  double Distance( const Rectangle<2>& r, const Geoid* geoid=0 ) const;
  
  bool Intersects( const Rectangle<2>& r, const Geoid* geoid=0 ) const;


/*
5.4.9 Operation ~distance~ (with ~points~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~

*/
  double MaxDistance(const Rectangle<2>& r, const Geoid* geoid=0 ) const;
/*
6.4.5 Operation ~no\_components~

*Precondition:* ~U.IsOrdered()~

*Semantics:* the number of components of ~U~.

*Complexity:* $O(1)$

*/
    int NoComponents() const;
/*
4.3.14 Operation ~translate~

*Precondition:* ~U.IsOrdered()~

*Semantics:* ~U + (x, y)~

*Complexity:* $O(n)$, where ~n~ is the size of ~U~

*/
    void Translate( const Coord& x, const Coord& y, LineT<ArrayT>& l ) const;

    void Rotate( const Coord& x, const Coord& y, double alpha,
                    LineT<ArrayT>& l ) const;
    
    void Scale( const Coord& x, const Coord& y, LineT<ArrayT>& l ) const;

/*
4.3.14 Operation ~transform~

*Precondition:* ~U.IsOrdered()~

*Semantics:* ~U -> R~

*Complexity:* $O(n)$, where ~n~ is the size of ~U~

*/
    void Transform( RegionT<ArrayT>& r ) const;

/*
6.4.6 ~Simplify~

This function stores a simplified version into the argument __result__.
The simplification is performed within three steps.
In an initial step, simple lines are extracted from the original one.
The reason is, that junctions within the line should be kept.
In the second step. each simple line is simplified by removing
sampling points using the well known Douglas Peucker algorithm. By using this
algorithm, it's guarantet that the maximum derivation from the original line
is smaller or equal to epsilon (or zero if epsilon is smaller than zero).
Unfortunately by simplifying the line, new selfintersections can be
created. We remove them in a final step.


*/  
    void Simplify(LineT<ArrayT>& result, const double epsilon,
                  const PointsT<ArrayT>& importantPoint,
                  const Geoid* geoid=0 ) const;



/*
~Realminize~

Removes overlapping segments and splits the line at all crossings
of the segments. May be that simple segments are represented by
many parts in the result.

*/
   void Realminize();


/*
6.4.6 Operation ~vertices~

*Precondition:* ~U.IsOrdered()~

*Semantics:* the vertices of ~U~

*Complexity:* $O(m)$, where ~m~ is the size of ~U~.

*/ 
    void Vertices( PointsT<ArrayT>* result ) const;

/*
6.4.6 Operation ~boundary~

*Precondition:* ~U.IsOrdered()~

*Semantics:* the boundary of ~U~

*Complexity:* $O(m)$, where ~m~ is the size of ~U~.

*/
    void Boundary( PointsT<ArrayT>* result ) const;

/*
4.4 Object Traversal Functions

These functions are object traversal functions which are useful when we are
using ROSE algebra algorithms.

*Pre-condition:* ~IsOrdered()~

*Complexity:* All these functions have a complexity of $O( 1 )$ .

*/
    void SelectFirst() const;
/*
Puts the pointer ~pos~ to the first half segment in the ~line~ value.

*/
    void SelectNext() const;
/*
Moves the pointer ~pos~ to the next half segment in the ~line~ value.

*/
    inline bool EndOfHs() const;
/*
Decides whether ~pos~ is -1, which indicates that no more half segments
in the ~line~ value need to be processed.

*/
    bool GetHs( HalfSegment& hs ) const;
/*
Gets the current half segment from the ~line~ value according to the
~pos~ pointer.

6.7 Window clipping functions

6.7.1 WindowClippingIn

This function returns the part of the line that is inside the window.
The inside parameter is set to true if there is at least one segment
part inside the window. If the intersection part is a point, then
it is not considered in the result.

*/
    void WindowClippingIn( const Rectangle<2> &window,
                           LineT<ArrayT> &clippedLine,
                           bool &inside ) const;
/*
6.7.2 WindowClippingOut

This function returns the part of the line that is outside the window.
The outside parameter is set to true if there is at least one part of
the segment that is outside of the window. If the intersection part is
a point, then it is not considered in the result.

*/
    void WindowClippingOut( const Rectangle<2> &window,
                            LineT<ArrayT> &clippedLine,
                            bool &outside ) const;
/*
6.8 Functions needed to import the the ~line~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple 
definition as an attribute.

*/
    inline int NumOfFLOBs() const
    {
      return 1;
    }

    inline Flob *GetFLOB( const int i )
    {
        return &line;
    }

    inline size_t Sizeof() const
    {
      return sizeof( *this );
    }

    inline bool Adjacent( const Attribute* arg ) const
    {
      return false;
    }

    size_t HashValue() const;
    void CopyFrom( const Attribute* right );
    int Compare( const Attribute *arg ) const;
//     int CompareAlmost( const Attribute *arg ) const;
    virtual LineT<ArrayT> *Clone() const;
    std::ostream& Print( std::ostream &os ) const;
    void Clear();



    virtual uint32_t getshpType() const{
       return 3; // PolyLine Type
    }

    virtual bool hasBox() const{
       return IsDefined();
    }

    virtual double getMinX() const{
      return bbox.MinD(0);
    }
    virtual double getMaxX() const{
      return bbox.MaxD(0);
    }
    virtual double getMinY() const{
      return bbox.MinD(1);
    }
    virtual double getMaxY() const{
      return bbox.MaxD(1);
    }

    virtual void writeShape(std::ostream& o, uint32_t RecNo) const{

       // first, write the record header
       WinUnix::writeBigEndian(o,RecNo);
       int size = line.Size();
       if(!IsDefined() || size==0){
         uint32_t length = 2;
         WinUnix::writeBigEndian(o,length);
         uint32_t type = 0;
         WinUnix::writeLittleEndian(o,type);
       } else {

         // first version: store each halfsegment as a single
         // polyline
         uint32_t segs = line.Size()/2;

         uint32_t length =  (44 + segs*4 + segs*4 * 8 )/ 2;

         WinUnix::writeBigEndian(o,length);
         // header end

         // type
         WinUnix::writeLittleEndian(o,getshpType());
         // box
         WinUnix::writeLittle64(o,getMinX());
         WinUnix::writeLittle64(o,getMinY());
         WinUnix::writeLittle64(o,getMaxX());
         WinUnix::writeLittle64(o,getMaxY());
         // numparts
         WinUnix::writeLittleEndian(o,segs);
         // numpoints
         WinUnix::writeLittleEndian(o,2*segs);
         // parts
         for(uint32_t i=0;i<segs;i++){
            WinUnix::writeLittleEndian(o,i*2);
         }
         // points
         HalfSegment hs;
         for(int i=0;i<line.Size();i++){
           line.Get(i,&hs);
           if(hs.IsLeftDomPoint()){
              Point p = hs.GetLeftPoint();
              WinUnix::writeLittle64(o,p.GetX());
              WinUnix::writeLittle64(o,p.GetY());
              p = hs.GetRightPoint();
              WinUnix::writeLittle64(o,p.GetX());
              WinUnix::writeLittle64(o,p.GetY());
           }
         }


       }
    }

   static void* Cast(void* addr){
      return new (addr) LineT<ArrayT>();
   }

   static const std::string BasicType(){
      return "line";
   }
   static const bool checkType(const ListExpr type){
     return listutils::isSymbol(type, BasicType());
   }

   const ArrayT<HalfSegment>& GetArray() const{
     return line;
   }

   inline LineT() {} // This constructor should only be used
                    // within the Cast function.
  private:

/*
6.10 Private member functions

*/
    void Sort();
/*
Sorts (quick-sort algorithm) the persistent array of half segments 
in the line value.

*/
    void RemoveDuplicates();
/*
Remove duplicates in the (ordered) array of half-segments.

*/
    bool Find( const HalfSegment& hs, int& pos,
               const bool& exact = false ) const;
/*
Searches (binary search algorithm) for a half segment in the line value and
returns its position. Returns false if the half segment is not found.

*/
    bool Find( const Point& p, int& pos, const bool& exact = false ) const;
/*
Searches (binary search algorithm) for a point in the line value and
returns its position, i.e. the first half segment with dominating point
less than or equal to ~p~. Returns true if the half segment dominating
point is equal to ~p~ and false otherwise.

*/
    void SetPartnerNo();
/*
Sets the partnerno attribute for all half segments of the line. The left half
segment partnerno points to the position to right one and the right half 
segment partnerno points to the position to the left one.

*/
    bool
    GetNextSegment( const int poshs, const HalfSegment& hs,
                    int& posnexths, HalfSegment& nexths );
    bool
    GetNextSegments( const int poshs,
                     const HalfSegment& hs,
                     std::vector<bool>& visited,
                     int& posnexths,
                     HalfSegment& nexths,
                     std::stack< std::pair<int, HalfSegment> >& nexthss );
    void computeComponents();

    void collectFace(int faceno, int startPos, ArrayT<bool>& used);
    int getUnusedExtension(int startPos, const ArrayT<bool>& used) const;

/*
Calculates and sets the number of components for the line. For every half 
segment, the following information is stored: faceno contains the number of 
the component, cycleno contains the ramification that the segment belongs, 
and edgeno contains the segment number.

The method ~VisitHalfSegments~ is a recursive function that does the job for
~SetNoComponents~.

6.11 Attributes

*/
    ArrayT<HalfSegment> line;
/*
The persisten array of half segments.

*/
    Rectangle<2> bbox;
/*
The bounding box that fully encloses all half segments of the line.

*/
    mutable int pos;
/*
The pointer to the current half segments. The pointer is important in object 
traversal algorithms.

*/
    bool ordered;
/*
Whether the half segments in the line value are sorted.

*/
    int noComponents;
/*
The number of components for the line.

*/
    double length;
/*
The length of the line.

*/

    int currentHS;
/*
Contains the number of the current HalfSegment for linear iteration of
a ~simple~ line.

*/
};


#endif


