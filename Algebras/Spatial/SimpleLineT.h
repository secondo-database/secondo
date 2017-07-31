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





#ifndef SIMPLELINET_H
#define SIMPLELINET_H

#include "PointsT.h"
#include "LRS.h"
#include "HalfSegment.h"


/*
7 The class ~SimpleLineT~

This class represents a line building a simple polyline, i.e. a line
without branches and zero or one components.

*/
template<template<typename T>class Array>
class SimpleLineT: public StandardSpatialAttribute<2>
{
  public:
/*
7.1 Constructors

~Constructor~

This constructor creates an undefined SimpleLineT object and initializes the
contained arrays to have ~size~ number od slots.

*/
  explicit SimpleLineT(int size):
            StandardSpatialAttribute<2>(false),
            segments(size),lrsArray(size/2),
            startSmaller(true),
            isCycle(false),isOrdered(true),length(0.0),
            bbox(false),currentHS(-1){ }

/*
~Constructor~


Constructs a ~SimpleLineT~ from a complex one.

*/
  template<template<typename T2> class Array2>
  explicit SimpleLineT(const LineT<Array2>& src);

/*
~CopyConstructor~

*/
  SimpleLineT(const SimpleLineT& src):
    StandardSpatialAttribute<2>(src.IsDefined()),
    segments(src.Size()+1),lrsArray(src.Size()/2 + 2),
    startSmaller(src.GetStartSmaller()), isCycle(src.IsCycle()),
    isOrdered(src.IsOrdered()), length(src.Length()), bbox(src.BoundingBox()),
    currentHS(src.currentHS)
  {
    segments.copyFrom(src.segments);
    lrsArray.copyFrom(src.lrsArray);
    lrsArray.TrimToSize();
    segments.TrimToSize();
    //Equalize(src);
  }
  
  template<template<typename T2> class Array2>
  SimpleLineT(const SimpleLineT<Array2>& src):
    StandardSpatialAttribute<2>(src.IsDefined()),
    segments(src.Size()+1),lrsArray(src.Size()/2 + 2),
    startSmaller(src.GetStartSmaller()), isCycle(src.IsCycle()),
    isOrdered(src.IsOrdered()), length(src.Length()), bbox(src.BoundingBox()),
    currentHS(src.currentHS)
  {
    convertDbArrays(src.segments, segments);
    convertDbArrays(src.lrsArray, lrsArray);
    lrsArray.TrimToSize();
    segments.TrimToSize();
  }

/*
~Destructor~

*/
  ~SimpleLineT() {}

/*
~Assignment Operator~

*/
  SimpleLineT<Array>& operator=(const SimpleLineT& src){
     Equalize(src);
     return *this;
  }
  
  template<template<typename T2> class Array2>
  SimpleLineT<Array>& operator=(const SimpleLineT<Array2>& src){
     Equalize(src);
     return *this;
  }

/*
~Destroy~

*/
  void Destroy(){
     segments.Destroy();
     lrsArray.Destroy();
  }


/*
~BulkLoading~

*/
  void StartBulkLoad();

  bool EndBulkLoad();

  SimpleLineT<Array>& operator+=( const HalfSegment& hs);

  inline bool IsOrdered() const{
    return isOrdered;
  }

/*
~Length~

Returns the length of this SimpleLineT (computed using metric
 (X,Y)-coordinates);

*/
  inline double Length() const{
    return length;
  }


/*
Returns the simple line's length computed for geographic (LON,LAT)-coordinates
and a Geoid. If any coordinate is invalid, ~valid~ is set to false
(true otherwise).

*/
  double Length(const Geoid& g, bool& valid) const;

  inline double SpatialSize() const{
    return Length();
  }

  inline double SpatialSize(const Geoid& g, bool& valid) const{
     return Length(g, valid);
  }

/*
Splits the simple line at the given pts into sublines. The set of sublines
is returned in the result vector

*/
  std::vector<SimpleLineT<Array> >* SplitAtPPoints(PointsT<Array>* pts);

/*
~BoundingBox~

Returns the MBR of this SimpleLineT object.

*/
  inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const{
    if(geoid){ // spherical geometry case:
      if(!geoid->IsDefined() || !IsDefined()){
        return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0);
      }
      Rectangle<2> geobbox = Rectangle<2>(false);
      for (int i=0; i<Size() ;i++){
        HalfSegment hs;
        Get( i, hs );
        if( hs.IsLeftDomPoint() ){
          if(!geobbox.IsDefined()){
            geobbox = hs.BoundingBox(geoid);
          } else {
            geobbox = geobbox.Union(hs.BoundingBox(geoid));
          }
        } // else: ignore inverse HalfSegments
      } // end for
      return geobbox;
    } // else: euclidean MBR
    return bbox;
  }

/*
~IsEmpty~

Checks wether this line has no geometry.

*/
  inline bool IsEmpty() const{
    return !IsDefined() || (segments.Size() == 0);
  }

/*
~Size~

This function returns the number of halfsegments within this SimpleLineT value.

*/
  inline int Size() const{
    return segments.Size();
  }

/*
~StartPoint~

Returns the start point of this simple line.

*/
  Point StartPoint(const bool startSmaller) const;
  Point StartPoint() const;

/*
~EndPoint~

Returns the end point of this simple line.

*/
  Point EndPoint(const bool startSmaller) const;
  Point EndPoint() const;

/*
~Contains~

Checks whether ~p~ is located on this line.

*/
  bool Contains(const Point& p, const Geoid* geoid=0) const;

/*
Inside

*/

template<template<typename T2> class Array2>
bool Inside( const SimpleLineT<Array2>& l, const Geoid* geoid=0 ) const;

/*
6.4.4 Operation ~intersection~

*/
template<template<typename T2> class Array2>
void Intersection(const Point& p, PointsT<Array2>& result, 
                  const Geoid* geoid=0) const;

template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void Intersection(const PointsT<Array2>& ps, PointsT<Array3>& result,
                  const Geoid* geoid=0) const;

template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void Intersection( const LineT<Array2>& l, SimpleLineT<Array3>& result,
                   const Geoid* geoid=0 ) const;

template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void Intersection( const RegionT<Array2>& l, SimpleLineT<Array3>& result,
                   const Geoid* geoid=0 ) const;

template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void Intersection( const SimpleLineT<Array2>& l, SimpleLineT<Array3>& result,
                   const Geoid* geoid = 0) const;

/*
6.4.4 Operation ~minus~

*/

template<template<typename T2> class Array2>
void Minus( const Point& l, SimpleLineT<Array2>& result, 
            const Geoid* geoid=0 ) const;

template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void Minus( const PointsT<Array2>& l, SimpleLineT<Array3>& result, 
            const Geoid* geoid=0 ) const;

template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void Minus( const LineT<Array2>& l, SimpleLineT<Array3>& result, 
            const Geoid* geoid=0 ) const;

template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void Minus( const RegionT<Array2>& l, SimpleLineT<Array3>& result, 
            const Geoid* geoid=0 ) const;

template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void Minus( const SimpleLineT<Array2>& l, SimpleLineT<Array3>& result,
            const Geoid* geoid=0 ) const;

/*
6.4.4 Operation ~union~

*/

template<template<typename T2> class Array2>
void Union( const Point& l, SimpleLineT<Array2>& result, 
            const Geoid* geoid=0 ) const;

template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void Union( const PointsT<Array2>& l, SimpleLineT<Array3>& result, 
            const Geoid* geoid=0 ) const;

template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void Union( const LineT<Array2>& l, LineT<Array3>& result, 
            const Geoid* geoid=0 ) const;

template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void Union( const RegionT<Array2>& l, RegionT<Array3>& result, 
            const Geoid* geoid=0 ) const;

template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void Union( const SimpleLineT<Array2>& l, LineT<Array3>& result, 
            const Geoid* geoid=0 ) const;


/*
~TrimToSize~

Changes the capacities of the contained arrays to the required size.

*/
  inline void TrimToSize(){
     segments.TrimToSize();
     lrsArray.TrimToSize();
  }

/*
~StartsSmaller~

Returns true if SimpleLineT start at smaller end point.

*/
  inline bool StartsSmaller() const{
    return startSmaller;
  }

/*
~Comparison~

The operator __==__ checks whether the structures of two simple lines are equal.
This operator may return a __false__ result even if the lines have the same
geometry.

*/
  bool operator==(const SimpleLineT<Array>& sl) const{
     if(!IsDefined() && !sl.IsDefined()){
       return true;
     }
     if(!IsDefined() || !sl.IsDefined()){
       return false;
     }
     if(bbox != sl.bbox){
       return false;
     }
     if(segments.Size() != sl.segments.Size()){
        return false;
     }
     if(startSmaller!=sl.startSmaller){
        return false;
     }
     if(!AlmostEqual(length,sl.length)){
       return false;
     }
     HalfSegment hs1;
     HalfSegment hs2;
     for(int i=0;i<segments.Size();i++){
       segments.Get(i,&hs1);
       sl.segments.Get(i,&hs2);
       if(!AlmostEqual(hs1, hs2)){
         return false;
       }
     }
     // the lrsArray is equals if the segments are equals
     return  true;
  }

  bool operator!=(const SimpleLineT<Array>& sl) const{
     return !(*this == sl);
  }

/*
~Distance Operators~

*/

  double Distance(const Point& p, const Geoid* geoid=0 )const;

  template<template<typename T2> class Array2>
  double Distance(const PointsT<Array2>& ps, const Geoid* geoid=0 ) const;

  template<template<typename T2> class Array2>
  double Distance(const SimpleLineT<Array2>& sl, const Geoid* geoid=0) const;

  double Distance(const Rectangle<2>& r, const Geoid* geoid=0) const;

  bool Intersects(const Rectangle<2>& r, const Geoid* geoid=0) const;

/*
~SetStartSmaller~

*/
  void SetStartSmaller(const bool smaller) {
    startSmaller = smaller;
  }

/*
~GetStartSmaller~

*/
  bool GetStartSmaller() const {
    return IsDefined() && startSmaller;
  }

/*
~AtPosition~


*/
  bool AtPosition(double pos, const bool startsSmaller,
                  Point& p, const Geoid* geoid=0) const;
  bool AtPosition(double pos, Point& p, const Geoid* geoid = 0) const;

/*
~AtPoint~

*/

  bool AtPoint(const Point& p, const bool startsSmaller, double& result,
               const Geoid* geoid= 0) const;

  bool AtPoint(const Point& p, const bool startsSmaller, const double tolerance,
               double& result, const Geoid* geoid=0) const;
  bool AtPoint(const Point& p, double& result, const double tolerance = 0.0,
               const Geoid* geoid=0) const;

/*
~SubLine~

*/
template< template<typename T1> class Array1>
void SubLine(double pos1, double pos2,
             bool startsSmaller, SimpleLineT<Array1>& l) const;

template< template<typename T1> class Array1>
void SubLine(double pos1, double pos2, SimpleLineT<Array1>& l) const;

/*
~Crossings~

*/

template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void Crossings(const SimpleLineT<Array2>& l, PointsT<Array3>& result,
               const Geoid* geoid=0) const;


/*
~Union~

*/

  template<template<typename T2> class Array2>
  bool Intersects(const SimpleLineT<Array2>& l, const Geoid* geoid=0) const;

/*
~Attribute Functions~

The following functions are needed to act as an attribute type.

*/
  inline int NumOfFLOBs() const{
    return 2;
  }

  inline Flob* GetFLOB(const int i){
    if(i==0)
       return &segments;
    return &lrsArray;
  }


  inline size_t Sizeof() const{
     return sizeof(*this);
  }

  inline bool Adjacent(const Attribute* arg) const{
    return false;
  }
  size_t HashValue() const{
    return bbox.HashValue() + segments.Size();
  }

  void CopyFrom(const Attribute* right){
     Equalize(*(static_cast<const SimpleLineT<Array>*>(right)));
  }

  int Compare(const Attribute* arg) const;

  template< template<typename T1> class Array1>
  int Compare( const SimpleLineT<Array1>* line) const;

  bool operator<(SimpleLineT *sl) const{
    return (Compare(sl) < 0);
  }

  virtual SimpleLineT* Clone() const{
     return new SimpleLineT(*this);
  }

  std::ostream& Print(std::ostream& os) const;

  void Clear(){
     segments.clean();
     lrsArray.clean();
     SetDefined( true );
     bbox.SetDefined(false);
     length=0.0;
     isOrdered=true;
     currentHS = -1;
  }

  bool SelectInitialSegment( const Point& startPoint,
                             const double tolerance = 1.0,
                             const Geoid* geoid=0);

  bool SelectSubsequentSegment();

  bool getWaypoint(Point& destination) const;

  bool IsCycle()const {
    return isCycle;
  }

  template<template<typename T2> class Array2>
  void toLine(LineT<Array2>& result) const;

  template<template<typename T2> class Array2>
  void fromLine(const LineT<Array2>& src);
  template<template<typename T2> class Array2>
  void fromLine(const LineT<Array2>& src, const bool smaller);

  static void* Cast(void* addr){
    return new (addr) SimpleLineT<Array>();
  }

  inline void Get( const int i, HalfSegment& hs ) const{
    segments.Get(i,&hs);
  }


  inline int lrsSize()const{
    return lrsArray.Size();
  }

  bool Get(LRS &lrs, int &i){
    return Find(lrs, i);
  }

  inline void Put(const int i, const HalfSegment& hs){
    segments.Put(i,hs);
  }


  inline void Get( const int i,  LRS& lrs ) const{
    lrsArray.Get(i,&lrs);
  }

  inline void Put(const int i, const LRS& lrs){
    lrsArray.Put(i,lrs);
  }

  inline void Resize(const int newSize){
    if(newSize>segments.Size()){
        segments.resize(newSize);
    }
  }

  static const std::string BasicType(){
    return "sline";
  }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }

   bool Find( const LRS& lrs, int& pos ) const {
   assert( IsOrdered() );

   if( IsEmpty() ){ // subsumes !IsDefined()
     return false;
   }

   if( (lrs.lrsPos < 0 && !AlmostEqual( lrs.lrsPos, 0 )) ||
       (lrs.lrsPos > Length() && !AlmostEqual( lrs.lrsPos, Length() ) )){
     return false;
   }

   lrsArray.Find( &lrs, LRSCompare, pos );
   if( pos > 0 ){
     pos--;
   }

   return true;
 }

 void printLRS(std::ostream& o) const{
    LRS lrs;
    for(int i=0;i<lrsArray.Size();i++){
       lrsArray.Get(i,lrs);
       o << "lsr[" << i << "] = "  << lrs << std::endl;
    }
 }



  private:
    Array<HalfSegment> segments;
    Array<LRS> lrsArray;
    bool startSmaller;
    bool isCycle;
    bool isOrdered;
    double length;
    Rectangle<2> bbox;
    int currentHS;

    template<template <typename T2> class Array2>
    void Equalize(const SimpleLineT<Array2>& src){
        convertDbArrays(src.segments, segments);
        convertDbArrays(src.lrsArray, lrsArray);
        this->SetDefined( src.IsDefined() );
        this->startSmaller = src.startSmaller;
        this->isCycle = src.isCycle;
        this->isOrdered = src.isOrdered;
        this->length = src.length;
        this->bbox = src.bbox;
        this->currentHS = src.currentHS;
    }
/*
~StandardConstructor~

Only for use within the Cast function.

*/
   SimpleLineT() { }

/*
~Find~

*/
bool Find( const Point& p, int& pos, const bool& exact = false ) const {
  assert( IsOrdered() );
  if( exact ){
    return segments.Find( &p, PointHalfSegmentCompare, pos );
  } else {
    return segments.Find( &p, PointHalfSegmentCompareAlmost, pos );
  }
}


/*
~Sort~

Sorts the array of HalfSegments.

*/
 void Sort(){
   segments.Sort(HalfSegmentCompare);
   isOrdered = true;
 }

/*
~SetPartnerNo~

Changes the partnerno of each HalfSegment to the index of the reverse
segment within the halfsegmenst array.

*/
 void SetPartnerNo();

/*
~computePolyline~

This function searches for a polyline within the halfsegment array and
creates the lrsarray. Additionally, the edge number of each segment is set
to the corresponding entry within the lrs array. If the segments does not
represent a simple polyLine, i.e. several components or branches, the result
of this function will be __false__.

*/
bool computePolyline();


};


#endif

