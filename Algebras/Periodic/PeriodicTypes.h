/*
//paragraph [3] Time: [{\tt\small Time:}{\it] [}]
//paragraph [4] Big: [\begin{center}\Huge{] [}\end{center}]
//[_] [\_]
//[&] [\&]
//[newline] [\\]
//[content] [\tableofcontents]
//[start_formula] [$]
//[end_formula] [$]
//[secondo] [\textsc{secondo}]
//[R] [I\hspace*{-0.25em}R]
//[->] [\ensuremath{\rightarrow}]
//[<] [\textless]
//[label] [\label]
//[ref] [\ref]
//[infty] [\ensuremath{\infty}]
//[mul] [\ensuremath{\cdot}]
//[titlepage] [\title{PeriodicAlgebra}\author{Thomas Behr}\maketitle]
//[startpicture][\begin{center}\leavevmode \epsfbox{Figures/]
//[endpicture] [}\end{center}]
//[newpage] [\newpage]
//[emptypage] [\thispagestyle{empty}]
//[emptypages] [\pagestyle{empty} \thispagestyle{empty}]
//[normalpages] [\pagestyle{plain} \thispagestyle{plain}]
//[romanpagenum] [\pagenumbering{Roman}]
//[arabicpagenum] [\pagenumbering{arabic}]
//[resetpagenum] [\setcounter{page}{1}]

//[tabstart] [\begin{tabular}]
//[tabend] [\end{tabular}]
//[colsep] [&]
//[rowsep] [\\]


\makeatletter
\renewcommand{\l@subsection}{\@dottedtocline{2}{1.5em}{3.3em}}
\makeatother


[titlepage]
[emptypage]

[4] An Algebra Implementation in 

[startpicture]SecondoLogo.eps[endpicture]

[newpage]
[romanpagenum]
[resetpagenum]

[content]

[startpicture]UniLogo.eps[endpicture]
[newpage]
[arabicpagenum]
[resetpagenum]


1 Introduction

This algebra models periodic moving objects. The underlying model can be used
for any model of linear moving objects. The objects can move freely or on
networks. The objects can be spatial or not and so on.
The periodic moving model extends linear moving objects by the
possibility of periodic moves. Exceptions and nested periods can be modelled
here. A period is an interval of fixed length. This means that periods of a
natural language (like each second monday of the month) can't be modelled.


*/
#ifndef PERIODICTYPES_H
#define PERIODICTYPES_H

#include <string>
#include <iostream>
#include <sstream>

#include "NestedList.h"
#include "DateTime.h"
#include "SpatialAlgebra.h"
#include "TopRel.h"
#include "../../Tools/Flob/DbArray.h"
#include "RepTree.h"
#include "GenericTC.h"
#include "ListUtils.h"

using namespace datetime;
using namespace toprel;

#include "TemporalAlgebra.h"

namespace periodic {

/* 
~[_][_]TRACE[_][_]~

This macro is for debugging purposes. At the begin of all functions should the
[_][_]TRACE[_][_] symbol.

*/
//#define TRACEON 
#ifdef TRACEON
#define __TRACE__ cout << __POS__ << endl;
#else
#define __TRACE__
#endif

#define TTRACE 
#ifdef TTRACE
#define __TTRACE__ cout << __POS__ << endl;
#else
#define __TTRACE__
#endif


// the __POS__ macro can be used in debug-messages
#define __POS__ __FILE__ << ".." << __PRETTY_FUNCTION__ << "@" << __LINE__ 

static const int LINEAR = 0;
static const int COMPOSITE = 1;
static const int PERIOD = 2;
static const bool DEBUG_MODE = true;


/*

2 Class Declarations

2.1 Some simple classes

2.1.1 ~PBBox~

This class is the implementation of the minimum bounding box. This class is
also an algebra class.

*/
class PBBox: public Attribute {
 friend ostream& operator<< (ostream&, const PBBox); 
 public:
    PBBox();
    PBBox(int dummy);
    PBBox(const PBBox& source);
    PBBox(const double x, const double y);
    PBBox(const double minX, const double minY,
          const double maxX, const double maxY);
    virtual ~PBBox();
    PBBox& operator=(const PBBox& source);
    int NumOfFLOBs() const;
    Flob* GetFLOB(int i);
    int Compare(const Attribute* arg)const;
    bool Adjacent(const Attribute*) const;
    size_t Sizeof() const;
    bool IsEmpty() const;
    void SetEmpty(const bool empty );
    size_t HashValue() const;
    void CopyFrom(const Attribute* arg);
    ListExpr ToListExpr(const ListExpr typeInfo) const;
    bool ReadFrom(const ListExpr LE, const ListExpr typeInfo);
    int  CompareTo(const PBBox* B2)const;
    bool Contains(const double x, const  double y)const;
    bool Contains(const PBBox* B2)const;
    PBBox* Clone() const;
    void Equalize(const PBBox* B2);
    void Equalize(const PBBox& B2);
    void Intersection(const PBBox* B2);
    void Intersection(const PBBox* b2, PBBox& res) const;
    bool Intersects(const PBBox* B2)const;
    double Size()const;
    void Union(const PBBox* B2);
    void Union(const PBBox* b2, PBBox& res) const;
    void Union(const double x, const double y);
    void SetUndefined();
    bool GetVertex(const int No,double& x, double& y) const;
    void SetEmpty();
    string ToString() const;
    static bool CheckKind(ListExpr type, ListExpr& errorInfo){
       return nl->IsEqual(type,BasicType());
    }
    static const string BasicType(){
      return "pbbox";
    }
    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }
    static ListExpr Property(){
       __TRACE__
       return gentc::GenProperty("-> DATA", 
                          BasicType(),
                          "(minx miny maxx maxy)",
                          "(12.0 23.3  100.987 5245.978)");
     }
  private:
    double minX;
    double maxX;
    double minY;
    double maxY;

/*
~state~

This variable describes the state (consisting of defined and empty) 
in the following way:

[tabstart]{lllll}
  empty   [colsep] 0 [colsep] 1 [colsep] 0 [colsep] 1  [rowsep]
  state   [colsep] 0 [colsep] 1 [colsep] 2 [colsep] 3  
[tabend]

*/
    unsigned char  state;
};

/*

2.1.2 ~RelInterval~

This class represents a relative interval. This means that a RelInterval
has all properties of a familiar interval (Closeness,length) without a
fixed start point. 

*/

class RelInterval : public Attribute{
  friend ostream& operator<< (ostream&, const RelInterval); 
  public:
/*
~Constructor~

The standard constructor doing nothing. Use it only in the cats function.

*/
    RelInterval();
/*
~Constructor~

This constructor creates a new relative interval of length zero. The argument
is only used for make a difference between this constructor and the constructor
without any arguments. Here, the argument is ignored.

*/
    RelInterval(int dummy);

/*
~Copy Constructor ~

*/
    RelInterval(const RelInterval& source);

/*
~Destructor~

Destroys this objects. Because all data are included in the 'root' block, 
this destructor must do nothing.

*/
    virtual ~RelInterval();

/*
~Assignment operator~

*/
    RelInterval & operator=(const RelInterval& source);


/*
~Append~

This function append D2 to this interval if possible. If the closure 
properties of both interval don't allow a merge between the intervals,
this interval remains unchanged and the result will be false.

*/
    bool Append(const RelInterval* D2);

/*
~CanAppended~

This function checks whether D2 can be used to extend this interval.

*/
    bool CanAppended(const RelInterval* D2)const ;

/*
~Contains~

This function checks whether this interval constains T.

*/
    bool Contains(const DateTime* T,bool relax = false) const;

/*
~Clone~

This function returns a copy of this interval. The caller must be
destroy the new created object.

*/
    RelInterval* Clone() const;

/*
~Algebra Supporting functions~

The next functions support some operations for using this type within
an algebra.

*/
    void Destroy();
    int NumOfFLOBs() const;
    Flob* GetFLOB(const int i);

/*
~CompareTo~

This function compares this relinterval with D2. 

*/
    int CompareTo(const RelInterval* D2)const;

/*
~Compare~

This function compares this interval with arg. arg must be of type 
Relinterval. 

*/
    int Compare(const Attribute* arg) const;

/*
The next functions are implemented for using this class as
an algebra type.

*/
    bool Adjacent(const Attribute*)const;
    size_t HashValue() const;
    void CopyFrom(const Attribute* arg);
    size_t Sizeof() const;

/*
~Mul~

This function extends this interval to the factor-multiple
of the original.

*/
    void Mul(const long factor);

/*
~Equalize~

When this function is called, the value of this relative interval
is taken from argumnet of this function.

*/
    void Equalize(const RelInterval* D2);

/*
~GetLength~

This function returns the length of this interval. If this interval is
unbounded, the result will have no meaning.
The caller of this function have to delete the new created object.

*/
    void GetLength(DateTime& result)const;
    DateTime* GetLength()const;

/*
~StoreLength~

This function changes the argument to hold the value of the length of 
this interval. The length will have only a menaing when the interval is
bounded.  

*/

    void StoreLength(DateTime& result)const;

/*
~ToListExpr~

This function converts this relative interval into it's
nested list representation.

*/
    ListExpr ToListExpr(const bool typeincluded) const;
    ListExpr ToListExpr(const ListExpr typeInfo) const;

/*
~IsLeftClosed~

This two functions can be used for exploring the left end of this interval.
If ~IsLeftInfinite~ returns true, the value returned by IsLeftClosed will 
have no meaning.

*/
    bool IsLeftClosed()const;

/*
~IsRightClosed~

The same functions for the right side of this interval.


*/
    bool IsRightClosed()const;

/*
~ReadFrom~

This functions tries to get the value of this interval from a nested
list. The second argument specifies whether the list is the pure value
list or a list consisting of type and value. The return value indicates 
the success of this function. This means, if this fucntion returns false,
the nested list does not represent a correct relative interval and the 
value of this interval is not canged.

*/
    bool ReadFrom(const ListExpr LE,const bool typeincluded);

/*
~Set~

Sets this interval to be finite with the specified length and closure 
properties.

*/
    bool Set(const DateTime* length,const bool leftClosed,
             const bool rightClosed);

/*
~SetLeftClosed~

This function sets the leftclosed property of this interval. 
Note that calling this function can lead to an invalid interval when
the lengt of this interval is zero.

*/
    void SetLeftClosed(bool LC);
    

/*
~SetRightClosed~

This is the symmetric function to ~SetLeftClosed~.

*/
    void SetRightClosed(bool RC);


/*
~SetClosure~

This function can be used to set the closure properties of this 
interval at the right end as well as the left end at the same time.


*/
    void SetClosure(bool LeftClosed, bool RightClosed);

    bool SetLength(const DateTime* T);
    string ToString()const;
    double Where(const DateTime* T) const;
    bool Split(const double delta,const bool closeFirst,RelInterval& Rest);
    bool Split(const DateTime duration, const bool closeFirst,
               RelInterval& Rest);
    bool Plus(const RelInterval* I); 
    static bool CheckKind(ListExpr type, ListExpr& errorInfo){
      return nl->IsEqual(type,BasicType());
    }
    static const string BasicType(){ 
        return "rinterval";
    }
    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }
    static ListExpr Property(){
      __TRACE__
      return gentc::GenProperty( "-> DATA",
                                 BasicType(),
                                 "(<datetime> lC rC lI rI)",
                                 "((time (2 120000)) TRUE FALSE FALSE FALSE)",
                                 "an interval without fixed start");
    }
  private:
    DateTime length;

/*
~state~

The ~state~ variable describes a set of flags, in particular
leftClosed, rightClosed,  and canDelete.




*/    
    unsigned char state;

    RelInterval(const DateTime* length, const bool leftClosed,
                const bool rightClosed);

    void ChangeLeftClosed(const bool value);
    void ChangeRightClosed(const bool value);
    void ChangeDefined(const bool value);
    void ChangeCanDelete(const bool value);
    bool GetLeftClosed() const;
    bool GetRightClosed() const;
    bool GetDefined() const;
    bool GetCanDelete() const;
};


/*
2.1.3 ~PInterval~

This class provides a structure for an interval. In the contrary to
a RelInterval an Interval has a fixed start time and a fixed end time.

*/
class PInterval : public Attribute{
  friend ostream& operator<< (ostream&, const PInterval); 
  public:
    PInterval();
    PInterval(int dummy);
    PInterval(const DateTime startTime, const RelInterval relinterval);
    PInterval(const PInterval& source);
    ~PInterval();
    PInterval& operator=(const PInterval& source);

    bool ReadFrom(ListExpr list, bool typeIncluded);
    bool ReadFrom(ListExpr list, const ListExpr typeInfo);
    bool Append(const RelInterval* D2);
    bool CanAppended(const RelInterval* D2)const;
    bool Contains(const DateTime* T)const;
    bool Contains(const PInterval* I)const;
    bool Intersects(const PInterval* I)const;
    PInterval* Clone() const;
    void Destroy();
    int NumOfFLOBs() const;
    Flob* GetFLOB(const int i);
    int CompareTo(const PInterval* D2)const;
    int Compare(const Attribute* arg) const;
    bool Adjacent(const Attribute*) const{return false;}
    size_t HashValue()const;
    size_t Sizeof() const;
    void CopyFrom(const Attribute* arg);
    void Equalize(const PInterval* D2);
    DateTime* GetLength()const;
    void Length(DateTime& res) const; 
    DateTime* GetStart()const;
    void GetStart(DateTime& result)const;
    DateTime* GetEnd()const;
    void GetEnd(DateTime& result)const;
    ListExpr ToListExpr(const bool typeincluded)const;
    ListExpr ToListExpr(const ListExpr typeInfo)const;
    bool IsLeftClosed()const;
    bool IsRightClosed()const;
    bool Set(const DateTime* startTime,const DateTime* length, 
             const bool leftClosed, const bool rightClosed);
    bool SetLength(const DateTime* T);
    string ToString()const;
    static bool CheckKind(ListExpr type, ListExpr& errorInfo){
      return nl->IsEqual(type,BasicType());
    }
    static const string BasicType(){
      return "pinterval";
    }
    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }
    static ListExpr Property(){
      __TRACE__
      return gentc::GenProperty("-> Data",
                                BasicType(),
                                "(<instant> <instant> leftClosed rightClosed)",
                                "((instant 1.1)(instant 1.5) TRUE FALSE)");
    }
  private:
    DateTime startTime;
    RelInterval relinterval;
};


/*
2.2 Classes for building periodic moves from units

2.2.1 ~CompositeMoves~ [&] ~SpatialCompositeMove~

This class defines a composition of several submoves.
These submoves are stored in an  array and
all moves in range [minIndex,maxIndex] of this array are
part of this composite move. To accerelate some operations,
the whole relinterval of all submoves are also stored here.
For spatial data types additionally the minimum bounding box is
stored.

*/
class CompositeMove{
   friend ostream& operator<< (ostream&, const CompositeMove); 
   public:
      CompositeMove();
      CompositeMove(int dummy);
      CompositeMove(const CompositeMove& source);
      ~CompositeMove();
      CompositeMove& operator=(const CompositeMove& source);
      void Equalize(const CompositeMove* source);


      string ToString() const;
      RelInterval interval;
      int minIndex;
      int maxIndex;
};

class SpatialCompositeMove: public CompositeMove{
  friend ostream& operator<< (ostream&, const SpatialCompositeMove); 
  public:
     SpatialCompositeMove();
     SpatialCompositeMove(int dummy);
     SpatialCompositeMove(const SpatialCompositeMove& source);
     ~SpatialCompositeMove();
     SpatialCompositeMove& operator=(const SpatialCompositeMove & source);
     void Equalize(const SpatialCompositeMove* source);
     string ToString() const;
     CompositeMove ToCompositeMove()const ;
     void ToCompositeMove(CompositeMove& result) const;

 
     PBBox bbox;
};


/*

2.2.2 ~SubMoves~

A submove is defined by an array-number and an index in this array.
This class is used to store the submoves of a composite and periodic move .

*/
class SubMove{
   friend ostream& operator<< (ostream&, const SubMove); 
   public:
      int arrayNumber;
      int arrayIndex;
      void Equalize(const SubMove* SM);
      string ToString()const;

};

/*
2.2.3 ~CSubMove~

A CSubmove extend the SubMove type by a duration.
This class is used for implementing the sons of a
composite move. The duration contains the sum of durations
of all predecessors within this composite move. This allows
a binary search for a given (relative) instant within the
sons of a composite move.

*/
class CSubMove: public SubMove{
public:
   DateTime duration;
   void Equalize(const CSubMove* SM);
};

/*

2.2.3 ~PeriodicMove~ [&] ~SpatialPeriodicMove~

A periodic move is determined by the number of repetitions 
and the repeated submove. To accelerate operations
also the whole relinterval is stored. For Spatial types also the
minimum bounding box is contained as member.

*/
class PeriodicMove{
  friend ostream& operator<< (ostream&, const PeriodicMove); 
  public:
     PeriodicMove();
     PeriodicMove(int dummy);
     PeriodicMove(const PeriodicMove& source);
     ~PeriodicMove();
     PeriodicMove& operator=(const PeriodicMove& source);
     void Equalize(const PeriodicMove* source);
     string ToString() const;

     int repeatations;
     SubMove submove;
     RelInterval interval;
};



class SpatialPeriodicMove: public PeriodicMove{   
  friend ostream& operator<< (ostream&, const SpatialPeriodicMove); 
  public:
     SpatialPeriodicMove();
     SpatialPeriodicMove(int dummy);
     SpatialPeriodicMove(const SpatialPeriodicMove& source);
     ~SpatialPeriodicMove();
     SpatialPeriodicMove& operator=(const SpatialPeriodicMove& source);
     void Equalize(const SpatialPeriodicMove* source);
     string ToString()const;
     PeriodicMove ToPeriodicMove()const;
     void ToPeriodicMove(PeriodicMove& result)const;

     PBBox bbox;
 };


/*
2.3 Classes for building unit types

2.3.1 ~LinearConstantMove~

This is a template class which can be used for representing any
discrete changing periodic moving objects. Examples include
boolean values, strings, and 9-intersection matrices. Before you can use
this function, you have to implement a function

  * ListExpr ToConstantListExpr(const T)
  * bool ReadFrom(const ListExpr, [&]T);


*/


template <class T>
class LinearConstantMove{
 public:
   LinearConstantMove();

   LinearConstantMove(const T value);

   LinearConstantMove(const T value, const RelInterval interval);

   LinearConstantMove(const LinearConstantMove<T>& source);

   virtual ~LinearConstantMove();

   LinearConstantMove<T>& operator=(const LinearConstantMove<T> source);
   void Set(const T value, const RelInterval interval);

   void At(const DateTime* duration, T& res) const;

   bool IsDefinedAt(const DateTime* duration)const;
   
   ListExpr ToListExpr()const;

   bool ReadFrom(const ListExpr value);

   virtual bool CanSummarized(const LinearConstantMove<T>* LCM) const;

   virtual bool Initial(T& res)const;

   virtual bool Final(T& res)const;

   void SetDefined(bool defined);
   bool GetDefined();
   bool Split(const DateTime duration,const bool toLeft, 
               LinearConstantMove<T>& right);

   void Equalize(const LinearConstantMove<T>& source);
   void Equalize(const LinearConstantMove<T>* source);


/*
~interval~

Hold the interval of this unit.

*/
   RelInterval interval;

/*
~value~

hold the value for this unit

*/
   T value;

/*
~defined~

flag for the defined state of this unit.

*/
   bool defined;
};


// include the implementation
}
#include "LinearConstantMove.h"
namespace periodic{


/*
2.3.2 ~LinearBoolMove~

This class is an example for the __LinearConstant__ class.
Whenever you want to implement a periodic moving 'constant',
you have to define such a class.


*/
ListExpr ToConstantListExpr(const bool value);

bool ReadFromListExpr(ListExpr le, bool& v);

typedef LinearConstantMove<bool> LinearBoolMove ; 


/*
2.3.3 ~LinearInt9MMove~


*/

ListExpr ToConstantListExpr(const Int9M value);
bool ReadFromListExpr(ListExpr le, Int9M& value);


class LinearInt9MMove: public LinearConstantMove<Int9M> {
public:
   LinearInt9MMove();
   LinearInt9MMove(int dummy);
   void Transpose();
};



/* 
2.3.4 ~MRealMap~ 

This class defines a mapping for moving reals. This means, this class 
can be used for defining units of moving reals.

*/
class MRealMap{
  friend class MovingRealUnit;
  public: 
   MRealMap();
   MRealMap(double a, double b, double c, bool root);
   MRealMap(int dummy);
   MRealMap(const MRealMap& source);
   ~MRealMap();
   MRealMap& operator=(const MRealMap& source);
   void Set(double a, double b, double c, bool root);
   DateTime ExtremumAt()const;
   bool ReadFrom(ListExpr le);
   ListExpr ToListExpr()const;
   double At(const DateTime* duration) const;
   double At(const double d) const;
   bool IsDefinedAt(const DateTime* duration) const;
   bool IsDefinedAt(const double d) const;
   bool EqualsTo(const MRealMap RM2)const;
   void Equalize(const MRealMap* RM);
   bool EqualsTo(const MRealMap& RM, double timediff) const;
  private:
/*
~Member variables~

The following variables describe the map represented by this unit. The formular for
computing the value at time ~t~ is given by:

[start_formula]
  v = \left\{ \begin{array}{ll}
          at^2+bt+c & \mbox{if root = false} \\
          \sqrt{at^2+bt+c}   & \mbox{if root = true}
      \end{array}
      \right.
[end_formula]

*/

     double a;
     double b;
     double c;
     bool root;
   

     void Unify();
};


/*
2.3.5 ~MovingRealUnit~

This class represents a single unit of a moving real. This means, this class
manages a relative interval and a mrealmap.

*/

class MovingRealUnit{
  public:
   MovingRealUnit();
   MovingRealUnit(MRealMap map, RelInterval interval);
   MovingRealUnit(int dummy);
   MovingRealUnit(const MovingRealUnit& source);
   ~MovingRealUnit();
   MovingRealUnit& operator=(const MovingRealUnit& source);
   void Set(MRealMap map, RelInterval interval);
   double min() const;
   double max() const;
   bool GetFrom(double start, double end, RelInterval interval);
   void At(const DateTime* duration,double& res) const;
   bool IsDefinedAt(const DateTime* duration) const;
   ListExpr ToListExpr()const;
   bool ReadFrom(ListExpr le);
   bool CanSummarized(const MovingRealUnit* MRU) const;
   bool Initial(double& result )const;
   bool Final(double& res)const;
   bool Split(const DateTime duration, const bool toLeft, 
              MovingRealUnit& unit);

   void SetDefined(const bool defined);
   bool IsDefined()const{return defined;}
   void Equalize(const MovingRealUnit* source);



/*
~interval~

The interval of this unit.

*/

  RelInterval interval;
  private:

/*
~map~

The value mapping for this unit.

*/
      MRealMap    map;
/*
~defined~

Flag for the defined state of this unit.

*/
      bool        defined;
};


/*
2.3.6 ~LinearPointMove~

The LinearPointMove class is the unit type for moving points.
This class defines a point moving from
(startX,startY) to (endX,endY) during a defined duration.
The bounding box is stored in the member __bbox__.
The __isStatic__ flag indicates whether the start and
the end points are equals. If the __defined__ flag is not
set, all values without the relinterval are ignored. The defined
flag is needed to represent gaps in the definition time.

*/
class LinearPointMove{
 friend ostream& operator<< (ostream&, const LinearPointMove); 
 friend class PMPoint;

 public:
/*
~Standard Constructor~

This constructor does nothing and should only be used within 
the cast function.

*/
   LinearPointMove();

/*
~Constructor~

Creates an instance of LinearPointMove. The argument is ignored.

*/
   LinearPointMove(int dummy);

/*
~Constructor~

The implementation of the copy constructor.

*/
   LinearPointMove(const LinearPointMove& source);


/*
~Destructor~

Destroys a LinearPointMove instance.

*/
   ~LinearPointMove();

/*
~Assignment operator~

*/
   LinearPointMove& operator=(const LinearPointMove& source);

/*
~ToListExpr~

Converts this LinearPointMove into its external 
representation.

*/
   ListExpr ToListExpr()const;

/*
~ReadFrom~

Reads this instance from the given ListExpr. If the list does
not represent a valid LinearPointMove, this instance is not 
changed and the result of the call will be __false__.

*/
   bool ReadFrom(const ListExpr value);

/*
~At~

Implementation of the atinstant operaton using relative intervals.
This units has to be defined at the given duration.


*/
   void At(const DateTime* duration, Point& res)const;

/*
~IsDefinedAt~

Checks if this LinearPointMOve is defined at the given duration.

*/
   bool IsDefinedAt(const DateTime* duration)const;

/*
~GetHalfSegment~

Returns the spatial projection of this LinearPointMove as a 
halfsegment.

*/
   bool GetHalfSegment(const bool LeftDominatingPoint,
                       HalfSegment& seg)const;
   bool Intersects(const PBBox* window)const;
   bool IsDefined()const; 
   bool IsStatic()const;
   void SetUndefined();
   PBBox BoundingBox()const;
   bool CanBeExtendedBy(const LinearPointMove* P2)const;
   bool ExtendWith(const LinearPointMove* P2);
   void Equalize(const LinearPointMove* P2);
   bool Split(const double delta, const bool closeFirst,
              LinearPointMove& Rest);
   bool SplitX(const double X, const bool closeFirst,
               LinearPointMove& Rest);
   bool SplitY(const double Y, const bool closeFirst, 
               LinearPointMove& Rest);
   size_t HashValue()const;
   int CompareTo(LinearPointMove* LPM);
   int CompareSpatial(LinearPointMove* LPM);
   string ToString()const;

   bool Connected(LinearPointMove* P2);

   int Toprel(const Point P,LinearInt9MMove* Result)const;   
   void Toprel(const Points& P, vector<LinearInt9MMove>& Result)const;  

   void DistanceTo(const double x, const double y,MovingRealUnit& result)const;

   bool nearOverlap(const LinearPointMove& lpm,
                    const double epsilonSpeed,
                    const double epsilonDirection, 
                    const double epsilonSpatial) const;

   double Length()const;

   double Speed(unsigned int timefactor=1)const;

   void Speed(MovingRealUnit& result) const;

   double Direction() const;
   void Direction(MovingRealUnit& result) const;
 
 private:
   RelInterval interval;
   double startX;
   double startY;
   double endX;
   double endY;
   bool isStatic;
   bool defined;
};

/*
~Stream Operator~

*/
ostream& operator<<(ostream& os, const LinearPointMove LPM);


/*
2.3.7 ~LinearPointsMove~

This is the unit representation for MovingPoints values.
This class manages a set of point in a relative time interval.
Because we can't use FLOB's in this class directly (This is only 
allowed in the MainClass (PMPoints), we use ranges in an array
for representing this set. The enties in this array are pairs of
points in the Euclidean Plane. 

2.3.7.1 The  Array Entries

A moving points unit consists of a set of points. Each of them moves
from its startpoint to its endpoint while the same duration for all
points in this set. So, a moving points unit consists basically of an
relative interval as well as the start and endpoints of the contained
moving points. Because we want to store the start and endpoints in an
array, we need a data structure representing this. This structure is 
implemented  by the TwoPoints class.

*/
class TwoPoints{
   friend ostream& operator<< (ostream&, const TwoPoints);
   friend class LinearPointsMove;
   public:
   TwoPoints();
   TwoPoints(const double xs, const double ys,
             const double xe, const double ye);
   TwoPoints(const TwoPoints& source);
   ~TwoPoints();
   TwoPoints& operator=(const TwoPoints& source);

   bool InnerIntersects(const TwoPoints &TP) const;
   bool IsStatic() const;
   int  CompareTo(const TwoPoints TP) const;
   bool operator< (const TwoPoints TP)const;
   bool operator> (const TwoPoints TP)const;
   bool operator== (const TwoPoints TP)const;
   bool IsSpatialExtension(const TwoPoints* TP) const;
   double Speed(const RelInterval interval) const;
   double GetStartX()const; 
   double GetEndX()const;
   double GetStartY()const;
   double GetEndY()const;
   void Equalize(const TwoPoints* source);
  private:
      double startX;
      double startY;
      double endX;
      double endY;
};

/*


If we take the structure for units of fixed size for moving points 
values, we obtain the structure in figure [ref]{fig:ObviousSolution.eps}.

                Figure 1: Obvious Solution  [ObviousSolution.eps] 


Unfortunately, in [secondo] each class must consist of a root record
and optional some FLOBs. The number of flobs can be choosen arbitrary,
but the number can't be change at runtime. Because the structure showed 
in the figure above contains nested FLOBs, we can't use this structure in
[secondo]. We solve this problem in the following way:
We store all maps for single points together in a big DBArray (FLOB)
contained in the root record. In a further DBArray, all unit records 
are stored. Instead of storing the maps directly in the units, we just
store the start index as well as the end index of the maps in the big 
array. As a consequence, all function of such a unit which needs to access
the maps, must have an argument containing the big array. The leads to the
structure showed in figure [ref]{fig:UnFoldedSolution.eps}.

                Figure 2: Used unfolded Solution [UnFoldedSolution.eps] 


*/
class LinearPointsMove{
  friend ostream& operator<< (ostream&, const  LinearPointsMove);
  friend ostream& operator<< (ostream&, class PMPoints&);
  friend class PMPoints;
  public:
   LinearPointsMove();
   LinearPointsMove(int dummy);
   LinearPointsMove(const LinearPointsMove& source);
   ~LinearPointsMove();
   LinearPointsMove& operator=(const LinearPointsMove& source);
   ListExpr ToListExpr(const DbArray<TwoPoints> &Points) const;
   bool ReadFrom(const ListExpr value, 
                 DbArray<TwoPoints> &Points, 
     int &Index);
   void At(const DateTime* duration, 
           const DbArray<TwoPoints> &Pts,
           Points& res         ) const;
   bool IsDefinedAt(const DateTime* duration) const;
   bool ProhablyIntersects(const PBBox* window) const;
   bool IsDefined() const; 
   bool IsStatic() const;
   void SetUndefined();
   bool CanBeExtendedBy(const LinearPointsMove* P2,
                        DbArray<TwoPoints> &MyPoints,
                        DbArray<TwoPoints> &PointsOfP2) const;
   bool ExtendWith(const LinearPointsMove* P2,
                   DbArray<TwoPoints> &MyPoints,
                   DbArray<TwoPoints> &PointsOfP2);
   void Equalize(const LinearPointsMove* P2);
   unsigned int GetStartIndex()const;
   unsigned int GetEndIndex()const; 
 private:
   RelInterval interval;
   PBBox bbox;
   unsigned int startIndex;
   unsigned int endIndex;
   bool isStatic;
   bool defined;
};

/*
2.3.7.3 ~ArrayRange~

This is a very simple class providing a range in an array
represented by the minimum and maximum index in this array.
A range is assumed to be empty if the maximum index is smaller
than the miminum one.

*/
class ArrayRange {
  public: 
    int minIndex;
    int maxIndex;
};


/*
2.3.8 ~PMSimple~

This class represents a simple periodic moving type.
The managed type is __T__, the used unit representation is defined by Unit.
This template can be used for all types with a unit representation of
fixed size (without FLOBs).
This class provides no functionality to add further informations, e.g. 
bounding box, to the nodes of the repetition tree.

*/
template <class T, class Unit>
class PMSimple : public Attribute {
  public:
     PMSimple();
     PMSimple(int dummy);
     PMSimple(const PMSimple<T,Unit>& source);

     virtual  ~PMSimple();
     
     PMSimple<T,Unit> operator=(const PMSimple<T, Unit>& source);

     void Destroy();
  
     void Equalize(const PMSimple<T,Unit>* B2);

     int NumOfFLOBs()const;

     Flob* GetFLOB(const int i);
     int Compare(const Attribute* arg) const;
     
     bool Adjacent(const Attribute*)const;
     PMSimple<T,Unit>* Clone() const;

     size_t Sizeof()const;
     size_t HashValue() const;
     void CopyFrom(const Attribute* arg);
     ListExpr ToListExpr(const ListExpr typeInfo)const;
     bool ReadFrom(const ListExpr value,const ListExpr typeInfo);
     bool IsEmpty()const;
  
     void At(const DateTime* instant,T& res)const;
     bool Initial(T& result) const;
     bool Final(T& result)const;
     void Translate(const DateTime& duration);
     void Translate(const DateTime* duration,
                    PMSimple<T,Unit>& res) const;

     void Minimize();
     int NumberOfLinearMoves();
     int NumberOfCompositeMoves();
     int NumberOfCompositeSubMoves();
     int NumberOfPeriodicMoves();

     void Split(const DateTime instant,const bool toLeft, 
           PMSimple<T,Unit>& leftPart, PMSimple<T,Unit>& rightPart);

     void splitRec(const DateTime instant, const bool toLeft,
              PMSimple<T,Unit>& leftPart, PMSimple<T,Unit>& rightPart,
              SubMove submove, DateTime& startTime, SubMove& SMLeft, 
              SubMove& SMRight);
     void SplitLeft(const DateTime& instant,const bool toLeft, 
                    PMSimple<T,Unit>* result);
     bool SplitLeftRec(const DateTime& instant,const bool toLeft,
                  PMSimple<T,Unit>& result,const SubMove& submove, 
                  DateTime& startTime,SubMove& lastSubmove);

     void  Intersection( const DateTime minTime, 
                         const bool minIncluded, 
                         const DateTime maxTime, 
                         const bool maxIncluded,
                         PMSimple<T,Unit>* res);

      void CopyValuesFrom( DbArray<Unit>& linearMoves,
                      DbArray<CompositeMove>& compositeMoves,
                      DbArray<SubMove>& compositeSubMoves,
                      DbArray<PeriodicMove>& periodicMoves,
                      bool defined,
                      RelInterval interval,
                      DateTime startTime,
                      SubMove submove);

      void TakeValuesFrom( DbArray<Unit>& linearMoves,
                      DbArray<CompositeMove>& compositeMoves,
                      DbArray<SubMove>& compositeSubMoves,
                      DbArray<PeriodicMove>& periodicMoves,
                      bool defined,
                      RelInterval interval,
                      DateTime startTime,
                      SubMove submove);
      void SetStartTime(DateTime newStart);

      DbArray<Unit>* GetLinearMoves();
      DbArray<CompositeMove>* GetCompositeMoves();
      DbArray<CSubMove>* GetCompositeSubMoves();
      DbArray<PeriodicMove>* GetPeriodicMoves();
      RelInterval* GetInterval();
      SubMove* GetSubmove();
  protected:
     DbArray<Unit> linearMoves;
     DbArray<CompositeMove> compositeMoves;
     DbArray<CSubMove> compositeSubMoves;
     DbArray<PeriodicMove> periodicMoves;
     bool canDelete;
     RelInterval interval;
     DateTime startTime;
     SubMove submove;

     void GetLength(SubMove sm, DateTime& result);

     bool GetLeftClosed(SubMove sm);
     bool GetRightClosed(SubMove sm);
     void GetInterval(SubMove sm,RelInterval& interval);

     /* the next four functions are needed to convert a
        periodic moving bool to its nested list representation.
     */

    ListExpr GetSubMoveList(const SubMove SM) const;
    ListExpr GetLinearMoveList(const int index)const;
    ListExpr GetPeriodicMoveList(const int index)const;
    ListExpr GetCompositeMoveList(const int index)const;
     /* The next functions help to read in a periodic moving
        bool from a nested list representation.
     */
    bool ResizeArrays(const ListExpr value);
    bool AddSubMovesSize(const ListExpr value,int &LMSize,int &CMSize,
                         int &SMSize,int &PMSize);

    bool AddLinearMove(const ListExpr value,int &LMIndex, int &CMIndex,
                       int &SMIndex, int &PMIndex);

    bool AddCompositeMove(const ListExpr value,int &LMIndex, int &CMIndex,
                       int &SMIndex, int &PMIndex);

    bool AddPeriodMove(const ListExpr value,int &LMIndex, int &CMIndex,
                       int &SMIndex, int &PMIndex);

    Unit GetLastUnit()const;
    bool MinimizationRequired();

    SubMove MinimizeRec(SubMove SM, 
                    DbArray<Unit>&                   newLinearMoves,
                    DbArray<CompositeMove>&          newCompositeMoves,
                    DbArray<CSubMove>&                newCompositeSubMoves,
                    DbArray<PeriodicMove>&           newPeriodicMoves,
                    Unit&                            Summarization,
                    bool&                            CompleteSummarized);

    /*
     The following function corrects the suration sums within the 
     composite submove to the correct values.
     This simplifies the operations changing the periodic moving object.
     Just call this function after the object is changed.
    */
    void CorrectDurationSums();
};

// include the implementation
}
#include "PMSimple.h"
namespace periodic{


/*
2.3.9 ~PMBool~

This class is an extension of the
~PMSimple~ class.

*/
class PMBool: public PMSimple<bool,LinearConstantMove<bool> >{
  public:
  PMBool() {};

  PMBool(int dummy):  PMSimple<bool,LinearConstantMove<bool> >(dummy){
  }
  static bool CheckKind(ListExpr type, ListExpr& errorInfo){
    return nl->IsEqual(type,BasicType());
  }
  static const string BasicType(){
    return "pmbool";
  }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }
  static ListExpr Property(){
    __TRACE__
    return gentc::GenProperty("-> DATA",
                              BasicType(),
                              "(<startTime> <submove>)",
                              " ... ",
                              "see in the documentation");
  }
};

/*
2.3.10 ~PMReal~

This class is a simple extension of the ~PMSimple~ class.

*/

class PMReal: public  PMSimple<double,MovingRealUnit> {
 public: 
/* 
~Constructors~

*/
  PMReal() {};

  PMReal(int dummy):  PMSimple<double,MovingRealUnit>(dummy){
  }

/*
~min~ and ~max~

*/  
  void min(CcReal& result) const{
    result.SetDefined(false);
    if(!IsDefined()){
       return;
    }
    MovingRealUnit unit;
    for(int i=0;i<linearMoves.Size();i++){
       linearMoves.Get(i,unit);
       if(unit.IsDefined()){
          if(!result.IsDefined()){
            result.Set(true,unit.min());
          } else {
            result.Set(true,::min(result.GetRealval(), unit.min()));
          }
       }
    }
  }

  void max(CcReal& result){
    result.SetDefined(false);
    if(!IsDefined()){
       return;
    }
    MovingRealUnit unit;
    for(int i=0;i<linearMoves.Size();i++){
       linearMoves.Get(i,unit);
       if(unit.IsDefined()){
          if(!result.IsDefined()){
            result.Set(true,unit.max());
          } else {
            result.Set(true,::max(result.GetRealval(), unit.max()));
          }
       }
    }

  }
 
  static bool CheckKind(ListExpr type, ListExpr& errorInfo){
    return nl->IsEqual(type,BasicType());
  } 
  static const string BasicType(){
       return "pmreal";
  }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }
  static ListExpr Property(){
    __TRACE__
    return gentc::GenProperty("-> DATA",
                              BasicType(),
                              "(<startTime> <submove>)",
                              " ... ",
                              "see in the documentation");
  }
};



/*
2.3.11 ~PMInt9M~

This class is derived from an instatiation of the
~PMSimple~ class. 

*/
class PMInt9M : public PMSimple<Int9M,LinearInt9MMove> {
public:
  PMInt9M();
  PMInt9M(int dummy);
  void Transpose();
  bool CreateFrom( const DbArray<LinearInt9MMove>& linearMoves, 
                  const ArrayRange*                     level,
                 const int                             levelsize,
                 const DbArray<CompositeMove>&          compositeMoves,
                 const DbArray<CSubMove>&                compositeSubMoves,
                 const DbArray<PeriodicMove>&           periodicMoves,
                 const DateTime                        startTime,
                 const SubMove                         submove);
  static bool CheckKind(ListExpr type, ListExpr& errorInfo){
      return  nl->IsEqual(type,BasicType());
  }
  static const string BasicType(){
     return "pmint9m"; 
  }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }
  static ListExpr Property(){
    __TRACE__
    return gentc::GenProperty( "-> DATA", BasicType(),
                               "(<startTime> <submove>)",
                               " ... ","see in the documentation");
  }

};


/*
2.3.12 ~PMPoint~

This class represents a single periodic moving point.

*/
class PMPoint : public Attribute {
  friend ostream& operator<< (ostream&, PMPoint);
  public:
     PMPoint();
     PMPoint(int dummy);
     PMPoint(const PMPoint& source);
     ~PMPoint();
     PMPoint& operator=(const PMPoint& source);
     void Destroy();
     void Equalize(const PMPoint* P2);
     int NumOfFLOBs() const;
     Flob *GetFLOB(const int i);
     int Compare(const Attribute* arg) const;
     bool Adjacent(const Attribute*)const;
     PMPoint* Clone() const;
     size_t Sizeof()const;
     size_t HashValue() const;
     void CopyFrom(const Attribute* arg);
     ListExpr ToListExpr(const bool typeincluded)const;
     ListExpr ToListExpr(const ListExpr typeInfo)const;
     bool ReadFrom(const ListExpr value, const ListExpr typeInfo);
     PMPoint* Intersection(const PInterval* interval);
     bool IsEmpty()const;
     void At(const DateTime* instant, Point& res)const;
     void Initial(Point& res)const;
     bool Final(Point& res);
     void Translate(const DateTime& duration);
     void Translate(const DateTime* duration, PMPoint& res)const;
     Points* Breakpoints()const;
     void Breakpoints(Points& res)const;
     Points* Breakpoints(const DateTime* minDuration,
                         const bool inclusive)const;
     void Breakpoints(const DateTime* minDuration,
                      const bool inclusive,
                      Points& res)const;
     void Trajectory(Line& res)const;
     DateTime GetStart()const;
     DateTime GetEnd()const;
     PInterval GetInterval()const;
     void GetInterval(SubMove sm, RelInterval& result)const;
     PBBox GetBbox()const;
     MPoint Expand()const;
     void Expand(MPoint& res)const;
     void ReadFrom(const MPoint& P,const bool twostep = true);

     PMInt9M* Toprel(const Point& P)const; 
     void Toprel(const Point& P,PMInt9M& res)const;

     PMInt9M* Toprel(const Points& P)const;

     void Toprel(const Points& P, PMInt9M& res)const;

     bool DistanceTo(const double x, const double y, PMReal& result)const;
     bool CheckCorrectness();
     void PrintArrayContents();
     void SpeedAndDirection(bool isSpeed,PMReal& result) const;

     size_t NumberOfNodes() const;
     size_t NumberOfPeriodicNodes()const;
     size_t NumberOfUnits() const;
     size_t NumberOfCompositeNodes()const;
     size_t NumberOfFlatUnits() const; 
     double Length()const;
     void Length(CcReal& res)const;
     static bool CheckKind(ListExpr type, ListExpr& errorInfo){
       return nl->IsEqual(type,"pmpoint");
     }
     static const string BasicType(){
       return "pmpoint";
     }
     static const bool checkType(const ListExpr type){
       return listutils::isSymbol(type, BasicType());
     }
     static ListExpr Property(){
       __TRACE__
       return gentc::GenProperty("-> DATA",
                                 BasicType(),
                                 "(<startTime> <submove>)",
                                 "...",
                                 "see the documentation");
      }

     void TrimToSize(){
       linearMoves.TrimToSize();
       compositeMoves.TrimToSize();
       compositeSubMoves.TrimToSize();
       periodicMoves.TrimToSize();
     }
  private:
     DbArray<LinearPointMove> linearMoves;
     DbArray<SpatialCompositeMove> compositeMoves;
     DbArray<CSubMove> compositeSubMoves;
     DbArray<SpatialPeriodicMove> periodicMoves;
     bool canDelete;
     RelInterval interval;
     DateTime startTime;
     PBBox bbox;
     SubMove submove;


     size_t NumberOfFlatNodes(const SubMove sm) const;
     /* the next four functions are needed to convert a
        periodic moving point to its nested list representation.
     */
     ListExpr GetSubMoveList(const SubMove* SM)const;
     ListExpr GetLinearMoveList(const int index)const;
     ListExpr GetSpatialPeriodicMoveList(const int index)const;
     ListExpr GetSpatialCompositeMoveList(const int index)const;
     /* The next functions help to read in a periodic moving
        point from a nested list representation.
     */
     bool ResizeArrays(const ListExpr Value);
     bool AddSubMovesSize(const ListExpr value,int &LMSize,int &CMSize,
                          int &SMSize,int &PMSize);
     bool AddLinearMove(const ListExpr value,int &LMIndex, int &CMIndex,
                        int &SMIndex, int &PMIndex);
     bool AddSpatialCompositeMove(const ListExpr value,int &LMIndex, 
                                  int &CMIndex, int &SMIndex, int &PMIndex);
     bool AddPeriodMove(const ListExpr value,int &LMIndex, int &CMIndex,
                        int &SMIndex, int &PMIndex);

     void AddSubMovesSizeForIntersection(DateTime* startTime,
                                    const SubMove submove,
                                    const PInterval* interval,
                                    int &Lcount,int &Ccount,
                                    int &Scount,int &Pcount);

     void AppendUnits(MPoint& P, DateTime* Time, const SubMove S)const;
     int NumberOfExpandedUnits()const;
     int NumberOfExpandedUnits(const SubMove S)const;
     LinearPointMove GetLastUnit();
     bool FillFromRepTree(int& cpos, int& cspos, int& ppos, RepTree TR);
     
     void CorrectDurationSums();
     void GetLength(SubMove sm, DateTime& result);

     double Length(const SubMove& sm) const;

  };


/*
2.3.13 ~PMPoints~

This class represents a set of periodic moving points.

*/
class PMPoints : public Attribute {
  friend ostream& operator<< (ostream& ,class PMPoints&) ;
  public:
     PMPoints();
     PMPoints(const PMPoints& source);
     PMPoints(int dummy);
     ~PMPoints();
     PMPoints& operator=(const PMPoints& source);
     void Destroy();
     void Equalize(const  PMPoints* P2);
     int NumOfFLOBs() const;
     Flob *GetFLOB(const int i);
     int Compare(const Attribute* arg) const;
     bool Adjacent(const Attribute*)const;
     PMPoints* Clone() const;
     size_t Sizeof()const;
     size_t HashValue() const;
     void CopyFrom(const Attribute* arg);
     ListExpr ToListExpr(const bool typeincluded)const;
     ListExpr ToListExpr(const ListExpr typeInfo)const;
     bool ReadFrom(const ListExpr value, const ListExpr typeInfo);
     bool IsEmpty()const;
     Points* Breakpoints()const;
     void Breakpoints(Points& res) const;
     
     Points* Breakpoints(const DateTime* duration,const bool inclusive)const;
     void Breakpoints(const DateTime* duration, 
                      const bool inclusive,
                      Points& res)const;
     
     void At(const DateTime* T, Points& res)const;
     void  Initial(Points& res)const;
     bool Final(Points& res) const;
     void Translate(const DateTime& duration);
     void Translate(const DateTime* duration,PMPoints& res)const;
     static bool CheckKind(ListExpr type, ListExpr& errorInfo){
       return nl->IsEqual(type,BasicType());
     }
     static const string BasicType(){
       return "pmpoints";
     }
     static const bool checkType(const ListExpr type){
       return listutils::isSymbol(type, BasicType());
     }
     static ListExpr Property(){
       __TRACE__
       return gentc::GenProperty("-> DATA",
                                 BasicType(),
                                 "(<startTime> <submove>)",
                                 "...",
                                 "see the documentation");
      }
 private:
     DbArray<LinearPointsMove> linearMoves;
     DbArray<TwoPoints> thePoints;
     DbArray<SpatialCompositeMove> compositeMoves;
     DbArray<CSubMove> compositeSubMoves;
     DbArray<SpatialPeriodicMove> periodicMoves;
     bool canDelete;
     RelInterval interval;
     DateTime startTime;
     PBBox bbox;
     SubMove submove;
     /* the next four functions are needed to convert a
        periodic moving point to its nested list representation.
     */
     ListExpr GetSubMoveList(const SubMove SM)const;
     ListExpr GetLinearMoveList(const int index)const;
     ListExpr GetSpatialPeriodicMoveList(const int index)const;
     ListExpr GetSpatialCompositeMoveList(const int index)const;
     /* The next functions help to read in a periodic moving
        point from a nested list representation.
     */
     bool ResizeArrays(const ListExpr Value);
     bool AddSubMovesSize(const ListExpr value, int &LMSize, int &PtsSize, 
                          int &CMSize, int &SMSize, int &PMSize);
     bool AddLinearMove(const ListExpr value, int &LMIndex, int &PtsIndex,
                        int &CMIndex, int &SMIndex, int &PMIndex);
     bool AddSpatialCompositeMove(const ListExpr value, int &LMIndex, 
                        int &PtsIndex, int &CMIndex, int &SMIndex, 
      int &PMIndex);
     bool AddPeriodMove(const ListExpr value, int &LMIndex, int &PtsIndex,
                        int &CMIndex, int &SMIndex, int &PMIndex);

     void CorrectDurationSums();
   
     void GetLength(SubMove SM, DateTime& result);

  };

/*
2.4.1.1 ~SimplePoint~

This class provides a single point with
coordinates in [R].


*/

class SimplePoint{
friend ostream& operator<< (ostream& , const SimplePoint);
public:
    double x;
    double y;
    /*
      The info members are  not  real values of this SimplePoint,
      this means, that in comparisons this members are not used.
      The purpose of this members is to store some additional information.
    */
    int intinfo;
    bool boolinfo;
    SimplePoint();
    SimplePoint(const SimplePoint& source);
    ~SimplePoint();
    void Equalize(const SimplePoint* source);

    int compareTo(const SimplePoint P2)const;
    bool operator< (const SimplePoint P2)const;
    bool operator> (const SimplePoint P2)const;
    bool operator== (const SimplePoint P2)const;
    bool operator!= (const SimplePoint P2)const;
    SimplePoint& operator=(const SimplePoint& P2);
};


/*
~Shift operators~

The following operators can be used for simple writing 
instances of the classes above to an output stream.

*/
ostream& operator<< (ostream &os, const PBBox BB);
ostream& operator<< (ostream& os, const RelInterval I);
ostream& operator<< (ostream& os, const TwoPoints TP);
ostream& operator<< (ostream& os, const PInterval I);
ostream& operator<< (ostream& os, const CompositeMove CM);
ostream& operator<< (ostream& os, const SpatialCompositeMove SCM);
ostream& operator<< (ostream& os, const SubMove SM);
ostream& operator<< (ostream& os, const PeriodicMove PM);
ostream& operator<< (ostream& os, const LinearPointsMove LM);
ostream& operator<< (ostream &os, class PMPoints &P);




} // namespace periodic 

#endif


