

/*
5 Auxiliary structures for plane sweep algorithms

5.1 Definition of ~ownertype~

This enumeration is used to indicate the source of an ~AVLSegment~.

*/

#ifndef _AVLSEGMENT_H_
#define _AVLSEGMENT_H_

#include "HalfSegment.h"
#include "Coord.h"


class myexception: public exception
{
  public:
    myexception(const string& s) : err(s){}

    ~myexception() throw() {}

  private:
  virtual const char* what() const throw()
  {
    return err.c_str();
  }

     string err;

};


namespace avlseg{


class ExtendedHalfSegment: public HalfSegment{
  public:
    ExtendedHalfSegment(){initialized=false;}
    
    ExtendedHalfSegment(const ExtendedHalfSegment& ehs):
      originX1(ehs.originX1),
      originX2(ehs.originX2),
      originY1(ehs.originY1),
      originY2(ehs.originY2),
      initialized(ehs.initialized){
      if(ehs.initialized){
         HalfSegment::operator=(ehs);
      }
    }

    ExtendedHalfSegment(const HalfSegment& hs):
     HalfSegment(hs){
     Point p = hs.GetLeftPoint();
     originX1 = p.GetX();
     originY1 = p.GetY();
     p = hs.GetRightPoint();
     originX2 = p.GetX();
     originY2 = p.GetY();
     initialized = true;
    }


    ExtendedHalfSegment& operator=(const ExtendedHalfSegment& hs){
        if(hs.initialized){
          HalfSegment::operator=(hs);
        }
        originX1 = hs.originX1;
        originY1 = hs.originY1;
        originX2 = hs.originX2;
        originY2 = hs.originY2;
        initialized = hs.initialized;
        return *this;
    }

    void CopyFrom(const HalfSegment& hs){
        HalfSegment::operator=(hs);
        Point p = hs.GetLeftPoint();
        originX1 = p.GetX();
        originY1 = p.GetY();
        p = hs.GetRightPoint();
        originX2 = p.GetX();
        originY2 = p.GetY();
        initialized = true;
    }


    void setOrigin(const double x1, const double y1, 
                   const double x2, const double y2){
       originX1 = x1;
       originX2 = x2;
       originY1 = y1;
       originY2 = y2;
    }

    double getOriginX1() const { return originX1; };
    double getOriginX2() const { return originX2; };
    double getOriginY1() const { return originY1; };
    double getOriginY2() const { return originY2; };

    bool isInitialized() const { return initialized; }


/*
~Print~

the usual Print function.  

*/
  ostream& Print(ostream& out) const{
    out << "[Extended: {";
    HalfSegment::Print(out);
    out << "}; " ; 
    out << "initialized = " << initialized << ", "
        << "originX1 = " << originX1 << ", " 
        << "originY1 = " << originY1 << ", " 
        << "originX2 = " << originX2 << ", " 
        << "originY2 = " << originY2 
        << "]";
    return out;  

  }


  private:

    double originX1;
    double originX2;
    double originY1;
    double originY2;
    bool initialized;
};




enum ownertype{none, first, second, both};

enum SetOperation{union_op, intersection_op, difference_op};

ostream& operator<<(ostream& o, const ownertype& owner);


const uint32_t LEFT      = 1;
const uint32_t RIGHT     = 2;
const uint32_t COMMON = 4;

/*
3.2 The Class ~AVLSegment~

This class is used for inserting into an avl tree during a plane sweep.


*/

class AVLSegment{

public:

/*
3.1.1 Constructors

~Standard Constructor~

*/
  AVLSegment();


/*
~Constructor~

This constructor creates a new segment from the given HalfSegment.
As owner only __first__ and __second__ are the allowed values.

*/

  AVLSegment(const ExtendedHalfSegment& hs, const ownertype owner);


/*
~Constructor~

Create a Segment only consisting of a single point.

*/

  AVLSegment(const Point& p, const ownertype owner);


/*
~Copy Constructor~

*/
   AVLSegment(const AVLSegment& src);


/*
3.2.1 Destructor

*/
   ~AVLSegment() {}


/*
3.3.1 Operators

*/

  AVLSegment& operator=(const AVLSegment& src);

  bool operator==(const AVLSegment& s) const;

  bool operator<(const AVLSegment& s) const;

  bool operator>(const AVLSegment& s) const;

/*
3.3.1 Further Needful Functions

~Print~

This function writes this segment to __out__.

*/
  void Print(ostream& out)const;

/*
~CheckPoints~

This function checks whether the points (x1,y1) and (x2,y2) are
located on the segment defined by (orinigX1, originY1) and 
(originY2, originY2). Furthermore, the distance from (x1,y1) to
(originX1,originY1) must be smaller than the distance to 
(originX2, originY2).

*/
  bool CheckPoints() const;



/*
3.5.1 Geometric Functions

~crosses~

Checks whether this segment and __s__ have an intersection point of their
interiors.

*/
 bool crosses(const AVLSegment& s) const;

/*
~crosses~

This function checks whether the interiors of the related
segments are crossing. If this function returns true,
the parameters ~x~ and ~y~ are set to the intersection point.

*/
 bool crosses(const AVLSegment& s,double& x, double& y) const;


/*
~commonPoint~

This function checks whether this AVLSegment and s have exactly one
common point. If so, the coordinates of this point are returned in
(x,y). 

*/
 bool commonPoint(const AVLSegment& s,double& x, double& y) const;



/*
~innerBoxContains~

chesks whether (x,y) is not an endpoint and within the 
bounding box of this segment.

*/
bool innerBoxContains(const double x, const double y) const;


/*
~extends~

This function returns true, iff this segment is an extension of
the argument, i.e. if the right point of ~s~ is the left point of ~this~
and the slopes are equal.

*/
  bool extends(const AVLSegment& s) const;


/*
~exactEqualsTo~

This function checks if s has the same geometry like this segment, i.e.
if both endpoints are equal.

*/
bool exactEqualsTo(const AVLSegment& s)const;

/*
~isVertical~

Checks whether this segment is vertical.

*/

 bool isVertical() const;


/*
~isPoint~

Checks if this segment consists only of a single point.

*/
  bool isPoint() const;

/*
~length~

Returns the length of this segment.

*/
  double length() const;



/*
~InnerDisjoint~

This function checks whether this segment and s have at most a
common endpoint.

*/

  bool innerDisjoint(const AVLSegment& s)const;
  bool printInnerDisjoint(const AVLSegment& s)const;

/*
~Intersects~

This function checks whether this segment and ~s~ have at least a
common point.

*/

  bool intersects(const AVLSegment& s)const;


/*
~overlaps~

Checks whether this segment and ~s~ have a common segment.

*/
   bool overlaps(const AVLSegment& s) const;

/*
~ininterior~

This function checks whether the point defined by (x,y) is
part of the interior of this segment.

*/
   bool ininterior(const double x,const  double y)const;


/*
~contains~

Checks whether the point defined by (x,y) is located anywhere on this
segment.

*/
   bool contains(const double x,const  double y)const;


/*
3.6.1 Comparison

Compares this with s. The x intervals must overlap.

*/

 int compareTo(const AVLSegment& s) const;

/*
~SetOwner~

This function changes the owner of this segment.

*/
  void setOwner(const ownertype o);


/*
3.7.1 Some ~Get~ Functions

~getInsideAbove~

Returns the insideAbove value for such segments for which this value is unique,
e.g. for segments having owner __first__ or __second__.

*/
  bool getInsideAbove() const;


  inline double getX1() const { return x1; }

  inline double getX2() const { return x2; }

  inline double getY1() const { return y1; }

  inline double getY2() const { return y2; }

  inline ownertype getOwner() const { return owner; }

  inline bool getInsideAbove_first() const { return insideAbove_first; }

  inline bool getInsideAbove_second() const { return insideAbove_second; }


/*
3.8.1 Split Functions

~split~

This function splits two overlapping segments.
Preconditions:

1) this segment and ~s~ have to overlap.

2) the owner of this and ~s~ must be different

~left~, ~common~ and ~right~ will contain the
explicitely left part, a common part, and
an explecitely right part. The left and/or right part
my be empty. The existence can be checked using the return
value of this function. Let ret the return value. It holds:

  __ret | LEFT__: the left part exists

  __ret | COMMON__: the common part exist (always true)

  __ret | RIGHT__: the right part exists


The constants LEFT, COMMON, and RIGHT have been defined
earlier.

*/

  uint32_t split(const AVLSegment& s, AVLSegment& left, AVLSegment& common,
            AVLSegment& right, const bool checkOwner = true) const;


/*
~splitAt~

This function divides a segment into two parts at the point
provided by (x, y). The point must be on the interior of this segment.

*/

  void splitAt(const double x, const double y,
               AVLSegment& left,
               AVLSegment& right)const;

  void splitAtRight(const double x, const double y,
                    AVLSegment& right)const;

/*
~splitCross~

Splits two crossing segments into the 4 corresponding parts.
Both segments have to cross each other.

*/
void splitCross(const AVLSegment& s, AVLSegment& left1, AVLSegment& right1,
                AVLSegment& left2, AVLSegment& right2) const;


/*
3.9.1 Converting Functions

~ConvertToExtendedHs~

This functions creates a ~HalfSegment~ from this segment.
The owner must be __first__ or __second__.

*/
ExtendedHalfSegment convertToExtendedHs(const bool lpd, 
                             const ownertype owner = both )const;


/*
3.10.1 Public Data Members

These members are not used in this class. So the user of
this class can change them without any problems within this
class itself.

*/
 int con_below;  // should be used as a coverage number
 int con_above;  // should be used as a coverage number



 static bool isError(){ return x_error; }

 static double getErrorValue() { return error_value; }
  
 static void clearError(){
   x_error = false;
 }


/*
3.11.1 Private Part

Here the data members as well as some auxiliary functions are
collected.

*/

 


private:
  /* data members  */
  double x1, x2, y1, y2; // the geometry of this segment
  bool insideAbove_first;
  bool insideAbove_second;
  ownertype owner;    // who is the owner of this segment
  double  originX1, originX2, originY1, originY2; 
                     // this segments comes from 

  static bool x_error; 
  static double error_value;

public: // for debugging only

/*
~pointequal~

This function checks if the points defined by (x1, y1) and
(x2,y2) are equals using the ~AlmostEqual~ function.

*/
  static bool pointEqual(const double x1, const double y1,
                         const double x2, const double y2);


/*
~pointSmaller~

This function checks if the point defined by (x1, y1) is
smaller than the point defined by (x2, y2).

*/

 static bool pointSmaller(const double x1, const double y1,
                          const double x2, const double y2);

/*
~comparePoints~

*/
  static int comparePoints(const double x1,const  double y1,
                           const double x2,const double y2);

/*
~compareSlopes~

compares the slopes of __this__ and __s__. The slope of a vertical
segment is greater than all other slopes.

*/
   int compareSlopes(const AVLSegment& s) const;

/*
~roundPoint~

whenever the point (x,y) is almostequal to an endpoint, its rounded
to that endpoint.

*/

   void roundPoint(double& x , double& y) const;



/*
~XOverlaps~

Checks whether the x interval of this segment overlaps the
x interval of ~s~.

*/

public: // for debugging only

  bool xOverlaps(const AVLSegment& s) const;

/*
~YOverlaps~

Checks whether the y interval of this segment overlaps the
y interval of ~s~.

*/

  bool yOverlaps(const AVLSegment& s) const;


/*
~XContains~

Checks if the x coordinate provided by the parameter __x__ is contained
in the x interval of this segment;

*/
  bool xContains(const double x) const;

/*
~yContains~

Checks if the y coordinate provided by the parameter __y__ is contained
in the y interval of this segment;

*/
  bool yContains(const double y) const;


/*
~GetY~

Computes the y value for the specified  __x__.
__x__ must be contained in the x-interval of this segment.
If the segment is vertical, the minimum y value of this
segment is returned.

*/
  double getY(const double x) const;
};

ostream& operator<<(ostream& o, const AVLSegment& s);

} // end of namespace


ostream& operator<<(ostream& o, const avlseg::ExtendedHalfSegment& hs);

#endif


