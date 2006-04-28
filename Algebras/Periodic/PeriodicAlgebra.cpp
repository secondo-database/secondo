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
//[titlepage] [\title{PeriodicAlgabra}\author{Thomas Behr}\maketitle]
//[startpicture][\begin{center}\leavevmode \epsfbox{Figures/]
//[endpicture] [}\end{center}]
//[newpage] [\newpage]
//[emptypage] [\thispagestyle{empty}]
//[emptypages] [\pagestyle{empty} \thispagestyle{empty}]
//[normalpages] [\pagestyle{plain} \thispagestyle{plain}]
//[romanpagenum] [\pagenumbering{Roman}]
//[arabicpagenum] [\pagenumbering{arabic}]
//[resetpagenum] [\setcounter{page}{1}]


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

2 Preconsiderations

A periodic moving object is modelled as an object consisting on several types
of movements. The types are linear move, composite move and periodic move.
A linear move modells a unit in earlier papers. A composite move is the
concatenation of movements of type linear or periodic. A periodic move
repeats a composite or linear move a number of times. We can think about a
periodic moving object as a tree. The leafs are the linear moves. The inner nodes
are periodic or composite ones. The maximal number of all nodes depends on the
number of the leafs. So the maximal number of periodic moves is L+C , where
L and C are the numbers of linear and composite moves. On the other hand, the
number of composite moves is L-1. The reason is, that a composite move requires at
least two submoves. Summarized we get:[newline]
maxN = maxP + maxC + L = P + L-1 + L = L-1+L + L-1 + L = 4L-2 = O(L)

*/

/*

1 Includes and Definitions

*/


#include <string>
#include <iostream>
#include <sstream>
#include <math.h>
#include <vector>

using namespace std;

#include "NestedList.h"
#include "Algebra.h"
#include "QueryProcessor.h"
#include "SecondoSystem.h"
#include "Attribute.h"
#include "StandardAttribute.h"
#include "StandardTypes.h"
#include "DateTime.h"
#include "SpatialAlgebra.h"
#include "List2.h" 
#include "RepTree.h"
#include "NodeTypes.h"
#include "TopRel.h"
#include "DBArray.h"

// the __POS__ macro can be used in debug-messages
#define __POS__ __FILE__ << ".." << __PRETTY_FUNCTION__ << "@" << __LINE__ 



extern NestedList* nl;
extern QueryProcessor *qp;
using namespace datetime;

#include "TemporalAlgebra.h"


/*
1 Definitions of some constants

*/

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

static const int LINEAR = 0;
static const int COMPOSITE = 1;
static const int PERIOD = 2;
static const bool DEBUG_MODE = true;
// a constant to handle numeric inaccuraccies in computations with doubles
static const double EPSILON = 0.00001;

static const string TYPE_ERROR = "typeerror";


/* 

1.1 Some forward declarations 

*/


static bool GetNumeric(const ListExpr List, double &value);

/*

2 Class Declarations

2.1 Some simple classes

2.1.1 The Class PBBox

This class is the implementation of the minimal bounding box. This class is
also an algebra class.

*/
class PBBox: public StandardAttribute {
 friend ostream& operator<< (ostream&, const PBBox); 
 public:
    PBBox();
    PBBox(int dummy);
    PBBox(const double x, const double y);
    PBBox(const double minX, const double minY,
          const double maxX, const double maxY);
    ~PBBox(){}
    int NumOfFLOBs() { return 0;}
    FLOB* GetFLOB() { assert(false);}
    int Compare(const Attribute* arg)const;
    bool Adjacent(const Attribute*) const;
    int Sizeof()const;
    bool IsDefined() const;
    bool IsEmpty() const;
    void SetDefined( const bool defined );
    size_t HashValue() const;
    void CopyFrom(const StandardAttribute* arg);
    ListExpr ToListExpr() const;
    bool ReadFrom(const ListExpr LE);
    int  CompareTo(const PBBox* B2)const;
    bool Contains(const double x, const  double y)const;
    bool Contains(const PBBox* B2)const;
    PBBox* Clone() const;
    void Equalize(const PBBox* B2);
    void Intersection(const PBBox* B2);
    bool Intersects(const PBBox* B2)const;
    double Size()const;
    void Union(const PBBox* B2);
    void Union(const double x, const double y);
    void SetUndefined();
    bool GetVertex(const int No,double& x, double& y) const;
    void SetEmpty();
    string ToString(){
       ostringstream res;
       if(!defined)
          res << "[undefined]";
       else if(isEmpty)
          res << "[empty]";
       else{
          res << "[" << minX <<", " << minY << ", ";
          res << maxX << ", " << maxY << "]";
           
       }
       return res.str(); 
    }
  private:
    double minX;
    double maxX;
    double minY;
    double maxY;
    bool defined;
    bool isEmpty;
};

/*

2.1.2 The class RelInterval

This class represents a relative interval. This means that a RelInterval
has all properties of a familiar interval (Closeness,length) without a
fixed start point. A RelInterval can be unbounded in the past or in
the future.

*/

class RelInterval : public StandardAttribute{
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
~Destructor~

Destroys this objects. Because all data are included in the 'root' block, 
this destructor must do nothing.

*/
    ~RelInterval(){}

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
    bool Contains(const DateTime* T) const;

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
    void Destroy(){canDelete=true;}
    int NumOfFLOBs(){ return 0;}
    FLOB* GetFLOB(const int i){ assert(false);}

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
    bool Adjacent(const Attribute*)const{return false;}
    bool IsDefined() const{ return defined;}
    void SetDefined(bool defined){ this->defined = defined;}
    size_t HashValue() const;
    void CopyFrom(const StandardAttribute* arg);


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
  private:
    DateTime length;
    bool leftClosed;
    bool rightClosed;
    bool defined;
    bool canDelete;
    RelInterval(const DateTime* length, const bool leftClosed,
                const bool rightClosed);
};


/*
2.1.3 The class PInterval

This class provides a structure for an interval. In the contrary to
a RelInterval an Interval has a fixed start time and a fixed end time.

*/
class PInterval : public StandardAttribute{
  friend ostream& operator<< (ostream&, const PInterval); 
  public:
    PInterval();
    PInterval(int dummy);
    PInterval(const DateTime startTime, const RelInterval relinterval);
    ~PInterval(){}
    bool ReadFrom(ListExpr list, bool typeIncluded);
    bool Append(const RelInterval* D2);
    bool CanAppended(const RelInterval* D2)const;
    bool Contains(const DateTime* T)const;
    bool Contains(const PInterval* I)const;
    bool Intersects(const PInterval* I)const;
    PInterval* Clone() const;
    void Destroy(){startTime.Destroy();relinterval.Destroy();}
    int NumOfFLOBs(){ return 0;}
    FLOB* GetFLOB(const int i){ assert(false);}
    int CompareTo(const PInterval* D2)const;
    int Compare(const Attribute* arg) const;
    bool Adjacent(const Attribute*) const{return false;}
    bool IsDefined() const;
    void SetDefined( bool defined);
    size_t HashValue()const;
    void CopyFrom(const StandardAttribute* arg);
    void Equalize(const PInterval* D2);
    DateTime* GetLength()const;
    DateTime* GetStart()const;
    DateTime* GetEnd()const;
    ListExpr ToListExpr(const bool typeincluded)const;
    bool IsLeftClosed()const;
    bool IsRightClosed()const;
    bool Set(const DateTime* startTime,const DateTime* length, 
             const bool leftClosed, const bool rightClosed);
    bool SetLength(const DateTime* T);
    string ToString()const;
  private:
    DateTime startTime;
    RelInterval relinterval;
};


/*
2.2 Classes for building periodic moves from units

2.2.1 CompositeMoves [&] SpatialCompositeMove

This class defines a composition of several submoves.
These submoves are stored in an  array and
all moves in range [minIndex,maxIndex] of this array are
part of this composite move. To accerelate some operations,
the whole relinterval of all submoves are also stored here.
For spatial data types additionally the minimal bounding box is
stored.

*/
class CompositeMove{
   friend ostream& operator<< (ostream&, const CompositeMove); 
   public:
      CompositeMove(){}
      CompositeMove(int dummy):interval(1){
           minIndex = -1;
           maxIndex = -1;
      }
      string ToString(){
         ostringstream res;
         res << "(" << interval.ToString();
         res << " " << minIndex << " -> " << maxIndex << ")";
         return res.str();
      }
   RelInterval interval;
   int minIndex;
   int maxIndex;
};

class SpatialCompositeMove: public CompositeMove{
  friend ostream& operator<< (ostream&, const SpatialCompositeMove); 
  public:
     SpatialCompositeMove(){}
     SpatialCompositeMove(int dummy):bbox(1){
        interval = RelInterval(1);
        minIndex = -1;
        maxIndex = -1;
     }
     string ToString(){
         ostringstream res;
         res << "(" << interval.ToString();
         res << " " << minIndex << " -> " << maxIndex << ")";
         res << " " << bbox.ToString();
         return res.str();

     }
     CompositeMove ToCompositeMove()const {
       CompositeMove result;
       ToCompositeMove(result);
       return result;
     }
     void ToCompositeMove(CompositeMove& result)const{
       result.interval.Equalize(&interval);
       result.minIndex=minIndex;
       result.maxIndex=maxIndex;
     }

 
     PBBox bbox;
};

/*

2.2.2 SubMoves

A submove is defined by an array-number and an index in this array.
This class is used to store the submoves of a composite and periodic move .

*/
class SubMove{
   friend ostream& operator<< (ostream&, const SubMove); 
   public:
      int arrayNumber;
      int arrayIndex;
      void Equalize(const SubMove* SM){
         arrayNumber = SM->arrayNumber;
         arrayIndex = SM->arrayIndex;
      }
      string ToString(){
         ostringstream res;
         if(arrayNumber==LINEAR)
           res << "linear["<<arrayIndex<<"]";
         else if(arrayNumber==PERIOD)
           res << "period[i"<<arrayIndex<<"]";
         else if(arrayNumber==COMPOSITE)
           res << "composite["<<arrayIndex<<"]";
         return res.str();
      }
};

/*

2.2.3 Periodic Move [&] SpatialPeriodicMove.

A Periodic Move is determined by the number of repeatations 
and the repeated submove. To accelerate operations
also the whole relinterval is stored. For Spatial types also the
minimal bounding box is contained as member.

*/
class PeriodicMove{
  friend ostream& operator<< (ostream&, const PeriodicMove); 
  public:
     PeriodicMove(){}
     PeriodicMove(int dummy):interval(1){}
     int repeatations;
     SubMove submove;
     RelInterval interval;
     string ToString(){
          ostringstream res;
          res << "R("<<repeatations<<", ";
          res << submove.ToString() <<" ,";
          res << interval.ToString() <<")";
          return res.str(); 
     }
};



class SpatialPeriodicMove: public PeriodicMove{   
  friend ostream& operator<< (ostream&, const SpatialPeriodicMove); 
  public:
     SpatialPeriodicMove(){}
     SpatialPeriodicMove(int dummy):bbox(1){
        interval = RelInterval(0);
     }
     string ToString(){
          ostringstream res;
          res << "R("<<repeatations<<", ";
          res << submove.ToString() <<" ,";
          res << interval.ToString() <<", ";
          res << bbox.ToString() << ")";
          return res.str(); 
     }
     PeriodicMove ToPeriodicMove()const{
          PeriodicMove result;
          ToPeriodicMove(result);
          return result;
     }

     void ToPeriodicMove(PeriodicMove& result)const{
          result.repeatations = repeatations;
          result.submove = submove;
          result.interval.Equalize(&interval);
     }

     PBBox bbox;
 };


/*
2.3 Classes for building unit types

2.3.1 The class LinearConstantMove

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

/*
~Constructor~

This constructor is used in the cast function. It does nothing
and should not be used at other places.

*/
   LinearConstantMove(){}

/*
~Constructor~

This constructor creates a new unit with given value at an
interval with length zero.

*/
   LinearConstantMove(const T value){
    interval= RelInterval(0);
    this->value = value;
    defined=true;
   }

/*
~Constructor~

This constructor creates a new unit from the given value and
the given interval.

*/
   LinearConstantMove(const T value, const RelInterval interval){
      this->value=value;
      this->interval=interval;
      this->defined=true;
   }

/*
~Destructor~

The destructor destroys this object. because we don't have any 
pointer structures, this destructor makes nothing.

*/
   virtual ~LinearConstantMove(){}


/*
~Set function~

This function sets the value as well as the interval of this unit
to the given values.

*/
   void Set(const T value, const RelInterval interval){
      this->value=value;
      this->interval=interval;
      this->defined=true;
   }


/*
~At function~

This function returns just the value of this unit. 
This function can't be called when the duration is
outside the interval or when this unit is not defined.
This should be checked by calling the isDefinedAt function.

*/
   T At(const DateTime* duration) const {
      assert(interval.Contains(duration));
      assert(defined);
      return value;
   }

/*
~IsDefinedAt~

This function checks whether this unit is defined at the 
given duration.

*/
   bool IsDefinedAt(const DateTime* duration)const{
      if(!interval.Contains(duration)) 
        return false;
      return defined;
   }

/*
~ToListExpr~

This function returns the external representation of this unit
as nested list.

*/
   ListExpr ToListExpr()const{
    if(defined){
        return nl->TwoElemList(
                      nl->SymbolAtom("linear"),
                      nl->TwoElemList(
                            interval.ToListExpr(true),
                            ToConstantListExpr(value)));
     } else {
        return nl->TwoElemList(
                       nl->SymbolAtom("linear"),
                       nl->TwoElemList(
                               interval.ToListExpr(true),
                               nl->SymbolAtom("undefined")));
     }
   }


/*
~ReadFrom~

This function reads this unit from its external representation.
If the list is not a valid representation for this unit, the 
value of this unit remains unchanged and the result will be __false__.

*/
   bool ReadFrom(const ListExpr value){
     if(nl->ListLength(value)!=2){
         if(DEBUG_MODE){
            cerr << __POS__ << ": Wrong ListLength" << endl;
            cerr << "expected : 2, received :" 
                 << nl->ListLength(value) << endl;
         }
         return false;
      }
      if(!interval.ReadFrom(nl->First(value),true)){
          if(DEBUG_MODE){
             cerr << __POS__ << ": Can't read the interval" << endl;
          }
          defined = false;
          return false;
      }

      ListExpr V = nl->Second(value);
      if(nl->IsEqual(V,"undefined")){
          defined = false;
          return true;
      }
      T tmp;
      if(!ReadFromListExpr(V,tmp))
         return false;
      this->value = tmp;
      return true;
   }

/*
~CanBeSummarized~

This function checks whether this unit and __LCM__ can be put together
to a single unit. This is the case iff the intervals are adjacent
and the values are the same.

*/

   virtual bool CanSummarized(LinearConstantMove<T> LCM) {
        if(!interval.CanAppended(&LCM.interval)) // no consecutive intervals
          return false;
        if(!defined && ! LCM.defined) // both are not defined
          return true;
        if(!defined || !LCM.defined) // only one is not defined
          return false;
        return value==LCM.value;

   }
  
/*
~Initial~

Returns the value at the begin of this unit. Because the value is never changed,
just the value is returned. This function is implemented for supporting the 
map properties for constant values. 

*/
 
   virtual T* Initial()const{
      return new T(value);
   }

/*
~Final~

The Final function returns the value of this unit at the end of this interval.
Because the value is never changed, the value is returned. This is support function
for the map class.

*/
   virtual T* Final(){
      return new T(value);
   }


/*
~SetDefined~

This function changes the defined state of this unit.

*/
   void SetDefined(bool defined){
      this->defined = defined;
   }

/*
~GetDefined~

This function returns the defined state of this unit.

*/
   bool GetDefined(){
      return defined;
   }


/*
~Split~

This function splits this unit into two parts at the given point in time.
If the splitting instant is outside the definition time of this unit,
this unit or right will be set to be undefined. This function returns
always true because an simple unit can be splitted in each case.  

*/
bool Split(const DateTime duration,const bool toLeft, 
              LinearConstantMove<T>& right){
     right.Equalize(*this);
     interval.Split(duration,toLeft,right.interval);
     this->defined = interval.IsDefined();
     right.defined = right.interval.IsDefined();
     return true;
   }


/*

~Equalize~

By calling this function, the value of this LinearConstantMOve is 
taken from the argument of this function.

*/
  void Equalize(const LinearConstantMove<T>& source){
    this->defined=source.defined;
    this->interval.Equalize(&(source.interval));
    this->value = source.value;
  } 


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


/*
2.3.2 The class LinearBoolMove

This class is an example for the __LinearConstant__ class.
Whenever you want to implement a periodic moving 'constant',
you have to define such a class.


~ToConstantListExpr~

This function is used to convert a boolean value into a
nested list representation.

*/
inline ListExpr ToConstantListExpr(const bool value){
   return nl->BoolAtom(value);
}

/*
~ReadFromListExpr~

This function is used to get a boolean value from 
its nested list representation. The list must be
of type __BoolType__ to be a correct representation for this
class.

*/
inline bool ReadFromListExpr(ListExpr le, bool& v){
   if(nl->AtomType(le)!=BoolType)
      return false;
    v = nl->BoolValue(le);
    return true;
}

/*
~LinearBoolMove class~

Because a bool can be only changed discretely, we can use the
LinearConstMove class as template for it.

*/
typedef LinearConstantMove<bool> LinearBoolMove ; 


/*
2.3.3 The class LinearInt9mMove 

This class provides the unit type for a periodic moving 9 intersection matrix.


~ToConstantList~

This function converts a 9 intersection matrix to its external representation.

*/
inline ListExpr ToConstantListExpr(const Int9M value){
    return value.ToListExpr();
}

/*
~ReadFrom~

This function reads a 9 intersection matrix from its 
external representation.

*/
inline bool ReadFromListExpr(ListExpr le, Int9M& value){
    return value.ReadFrom(le);
}

/*
~LinearInt9MMove~

*/

class LinearInt9MMove: public LinearConstantMove<Int9M> {
public:
/*
~Constructor~

The constructor makes nothing.

*/
   LinearInt9MMove(){}

/*
~Constructor~

This constructor calls the constructor of the superclass.

*/
   LinearInt9MMove(int dummy):LinearConstantMove<Int9M>(dummy){}

/*
~Transpose~

This operator transposes the matrix managed by this unit.

*/
   void Transpose(){
        value.Transpose();
   }
};




/* 
2.3.4 The class MRealMap 

This class defines a mapping for moving reals. This means, this class 
can be used for defining units of moving reals.

*/
class MRealMap{
  friend class MovingRealUnit;
  public: 

/*
~Constructor~

The needed empty constructor.

*/
   MRealMap(){}


/*
~constructor~

This constructor sets the value of this moving real map to the
given values. 

*/

  MRealMap(double a, double b, double c, bool root){
     Set(a,b,c,root);
  }

/*
~Set~

This function sets this MRealMap to represent the function:

                Figure 5: Formula of an Moving Real Map  [MRealFormula.eps]

*/
  void Set(double a, double b, double c, bool root){
      this->a=a;
      this->b=b;
      this->c=c;
      this->root=root;
      Unify();
  }

 
 
/*
~ReadFrom function~

This function reads the value of this map from a list expression. 
If the list does not represent a valid moving real map,
the value of this map remains unchanged and the result will be __false__.
Otherwise, this value is changed to the value given in the list and 
__true__ is returned.

*/
  bool ReadFrom(ListExpr le){
    if(nl->ListLength(le)!=4)
       return false;
    double tmpa,tmpb,tmpc;
    if(nl->AtomType(nl->Fourth(le))!=BoolType)
       return false;
    bool tmproot=nl->BoolValue(nl->Fourth(le));
    if(!GetNumeric(nl->First(le),tmpa))
       return false;
    if(!GetNumeric(nl->Second(le),tmpb))
       return false;
    if(!GetNumeric(nl->Third(le),tmpc))
       return false;
    a = tmpa;
    b = tmpb;
    c = tmpc;
    root = tmproot;
    Unify();
    return true;
  }

/*
~ToListExpr~

This function returns the nested list representation of this map.

*/
   ListExpr ToListExpr()const{
      return nl->FourElemList(nl->RealAtom(a),
                              nl->RealAtom(b),
                              nl->RealAtom(c),
                              nl->BoolAtom(root));
   }
   

/*
~At~

Computes the value of this map for a given instant. Ensure, that the
map is defined at this instant using the ~IsDefinedAt~ function.

*/
  double At(const DateTime* duration) const {
      double d = duration->ToDouble();
      return At(d);
  }

/*
~At~

This function may be used for a faster computing of a value of this 
map.

*/
   inline double At(const double d) const{
      double r1 = (a*d+b)*d+c;
      if(root) return sqrt(r1);
      return r1;
   }


/*
~IsDefinedAt~

This function checks whether this map is defined at the given instant.

*/
  bool IsDefinedAt(const DateTime* duration) const {
       if(!root) // defined all the time
           return true;
       double d = duration->ToDouble();
       double r1 = (a*d+b)*d+c;
       return r1>=0; // defined only if expression under the root is positive
  }


/*
~IsDefinedAt~

Returns __true__ if this map is defined for the given value.

*/
  inline bool IsDefinedAt(const double d) const{
       if(!root)
           return true;
       double r1 = (a*d+b)*d+c;
       return r1>=0; // defined only if expression under the root is positive
  }


/*
~EqualsTo~

This function checks for equality between this map and the argument.

*/

  bool EqualsTo(const MRealMap RM2)const{
    return a == RM2.a && 
           b == RM2.b && 
           c == RM2.c &&
           root == RM2.root;
  }


/*
~Equalize~

If this function is called, the valu of this map is taken from the argument.

*/
   void Equalize(const MRealMap* RM){
       a = RM->a;
       b = RM->b;
       c = RM->c;
       root = RM->root;
   }

/*
~EqualsTo~ 

This function checks whether the argument is an extension for this map when the
arguments starts __timediff__ later. This means:

[forall] x [in] [R]: this[->]GetValueAt(x+timediff)==RM[->]GetValueAt(x)


*/
  bool EqualsTo(const MRealMap& RM, double timediff) const{
     if(root!=RM.root)
         return false;
     return RM.a == a &&
            RM.b == 2*a*timediff+b &&
            RM.c == timediff*timediff + b*timediff + c; 

  }

  private:
/*
~Member variables~

The following variables describe the map represented by this unit. The formular for
computing the value at time ~t~ is given by:

[start_formula]
  v = \left\{ \begin{array}{ll}
          a^2+bt+t & \mbox{if root = false} \\
          \sqrt{a^2+bt+t}   & \mbox{if root = true}
      \end{array}
      \right.
[end_formula]

*/

     double a;
     double b;
     double c;
     bool root;
/*
~Unify~ 

This functions converts this moving real into a unified representation.

*/
   void Unify(){
      if(!root) return;
      if(a==0 && b==0){
          c = sqrt(c);
          root = false;
          return;
      }
      if(b==0 && c==0){
          b=sqrt(a);
          a=0;
          root=false;
          return;
      }
   }
};


/*
2.3.5 The class MovingRealUnit

This class represents a single unit of a moving real. This means, this class
manages a relative interval and a mrealmap.

*/

class MovingRealUnit{
  public:

/*
~constructor~

This constructor does nothing.

*/
   MovingRealUnit(){}

/*
~constructor~

This constructor creates a MovingRealUnit with the given values for the
interval and for the map.

*/
   MovingRealUnit(MRealMap map, RelInterval interval){
         Set(map,interval);
   }


/*
~constructor~ 

This constructor creates a MovingRealUnit representing the constant value 
zero. The length of the interval will also be zero.

*/

   MovingRealUnit(int dummy){
       MRealMap aMap(0,0,0,false);
       RelInterval aInterval(0);
       Set(aMap,aInterval); 
   }
      
 
 
/*
~Set function~

This functions sets the values of the internal variables to defined values.

*/
   void Set(MRealMap map, RelInterval interval){
         this->map.Equalize(&map);
         this->interval.Equalize(&interval);
         defined=true;
   }

/*
~GetFrom Function~

When this function is called, the value of this unit is computed to 
change from value __start__ to value __end__ whithin __interval__
in a linear way. If __start__ and __end__ differs, the length of the 
interval must be greater than zero. For equal value also length zero
is allowed. If the interval does not fullfills this conditions, the
value of this unit remains unchanged. The return value indicates 
the success of the call of this function. 

*/
   bool GetFrom(double start, double end, RelInterval interval){
     // check the preconditions
     DateTime length;
     interval.StoreLength(length);
     if(length.LessThanZero()) 
       return false;
     if(length.IsZero()){
        if(start!=end)
           return false;
        if(!interval.IsRightClosed() || !interval.IsLeftClosed())
           return false;  
     }
     // special case, no change while interval
     if(start==end){
        map.Set(0,0,start,false);
        this->interval.Equalize(&interval);
        return true;
     }
     double b = (end-start) / (length.ToDouble());
     map.Set(0,b,start,false);
     this->interval.Equalize(&interval);
     return true;
   } 


/*
~At~

This function computes the value of this unit for a given instant. 
This unit must be defined at this instant. This should be checked
by calling the ~IsDefinedAt~ function before. 

*/
   double At(const DateTime* duration) const {
        assert(interval.Contains(duration));
        assert(defined);
        assert(map.IsDefinedAt(duration));
        return map.At(duration);
   }
  
/*
~IsDefinedAt~

This function cvhecks whether this unit is defined at the given point in time. 

*/
   bool IsDefinedAt(const DateTime* duration) const{
       return interval.Contains(duration) &&
              defined &&
              map.IsDefinedAt(duration);
   }
 

/*
~ToListExpr~

This function computes the representation of this unit in nested list format.

*/
   ListExpr ToListExpr()const{
     if(!defined)
          return nl->TwoElemList(nl->SymbolAtom("linear"),
                                 nl->SymbolAtom("undefined"));
     return nl->TwoElemList(nl->SymbolAtom("linear"),
                            nl->TwoElemList(
                                interval.ToListExpr(true),
                                map.ToListExpr()));
          
   }

/*
~ReadFrom~

If the argument contains a valid representation of a moving real unit,
the value of this unit is changed to the value from the argument. Otherwise,
this unit remains unchanged.

*/
   bool ReadFrom(ListExpr le){
      if(nl->ListLength(le)!=2)
          return false;
      if(nl->AtomType(nl->First(le))!=SymbolType)
          return false;
      if(nl->SymbolValue(nl->First(le))!="linear")
          return false;
      ListExpr v = nl->Second(le);
      if(nl->ListLength(v)!=2)
          return false;
      RelInterval tmpinterval;
      if(!tmpinterval.ReadFrom(nl->First(v),true))
          return false;
      if(!map.ReadFrom(nl->Second(v)))
         return false;
      interval.Equalize(&tmpinterval);
      defined=true;
      return true;
   }

/*
~CanSummarized~

This function checkes whether two moving real units can summarized. This means, the
intervals are adjacent and the coefficients are choosen that the functions are equal.

*/

  bool CanSummarized(const MovingRealUnit* MRU) const{
     if(!interval.CanAppended(&(MRU->interval)))  
        return false;
     DateTime * DT = interval.GetLength();
     double d = DT->ToDouble();
     delete DT;
     DT = NULL;
     return map.EqualsTo(MRU->map,d);
  }
/*
~Initial~

This function returns the value of this unit at the start of its interval.

*/
  double* Initial()const{
        if(!map.IsDefinedAt(0.0))
             return NULL;
        else
           return new double(map.At(0.0)); 
  }
 

/*
~Final~

This function returns the value of this unit at the end of its interval.

*/ 
  double* Final(){
        DateTime* end = interval.GetLength();
        double* res;
        if(!map.IsDefinedAt(end))
             res = NULL;
        else
            res = new double(map.At(end)); 
        delete end;
        end = NULL;
        return res;
  }

/*
~Split~

This function splits this unit into 2 ones. The splitting point is 
given by the argument __duration__. The left part will be the 
__this__ objects, the right part will be stored in the argument __right__.
The affiliation of the splitting point is ruled by the parameter __toLeft__.
The function will return __true__ if this unit can splittet, i.e. the 
splitting point is contained in this unit. In the other case, this unit 
left as it is, the __right__ parameter is set to be undefined, and the 
return value will be __false__.

*/

bool Split(const DateTime duration, const bool toLeft, MovingRealUnit& unit){
   // case duration not contained
   if(!interval.Contains(&duration)){
      unit.defined=false;
      return false;
   }
   // case left will be empty
   if(duration.IsZero() && !toLeft){
      unit.defined=false;
      return false;
   }
   // all ok, split this unit
   if(!interval.Split(duration,toLeft,unit.interval)){
      unit.defined=true;
      return false;
   }
   unit.defined=true;
   double d = duration.ToDouble();
   unit.map.Set(map.a,map.b-2*d, d*d-d*map.b+map.c,map.root); 
   return true;
} 


/*
~SetDefined~

Using this function, we can set the defined flag for this instance.
Use carefully this function with an value of true. The internal stored 
values are nmot checked to hold meaningful values, just the flag is
changed.

*/

  void SetDefined(const bool defined){
    this->defined=defined;
  }




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
2.3.6 The Class LinearPointMove

The LinearPointMove class is the unit type for moving points.
This class defines a point moving from
(startX,startY) to (endX,endY) while a defined duration.
The bounding box is stored in the member __bbox__.
The __isStatic__ flag indicates whether the start and
the end point are equals. If the __defined__ flag is not
set, all values without the relinterval are ignored. The defined
flag is needed to represent gaps in the definition time.

*/
class LinearPointMove{
 friend ostream& operator<< (ostream&, const LinearPointMove); 
 friend class PMPoint;

 public:
   LinearPointMove(){}
   LinearPointMove(int dummy);
   ListExpr ToListExpr()const;
   bool ReadFrom(const ListExpr value);
   Point* At(const DateTime* duration)const;
   bool IsDefinedAt(const DateTime* duration)const;
   CHalfSegment GetHalfSegment(const bool LeftDominatingPoint)const;
   bool Intersects(const PBBox* window)const;
   bool IsDefined()const { return defined;}
   bool IsStatic()const { return isStatic;}
   void SetUndefined();
   bool CanBeExtendedBy(const LinearPointMove* P2)const;
   bool ExtendWith(const LinearPointMove* P2);
   void Equalize(const LinearPointMove* P2);
   bool Split(const double delta, const bool closeFirst,
              LinearPointMove& Rest);
   bool SplitX(const double X, const bool closeFirst,
               LinearPointMove& Rest);
   bool SplitY(const double Y, const bool closeFirst, 
               LinearPointMove& Rest);
   size_t HashValue()const{
     if(!defined) return 0;
     return bbox.HashValue() + interval.HashValue()+(size_t)startX;
   }
   int CompareTo(LinearPointMove* LPM);
   string ToString()const;

   bool Connected(LinearPointMove* P2){
       return (endX==P2->startX) & (endY == P2->startY);
   }

   int Toprel(const Point P,LinearInt9MMove* Result)const;   
   void Toprel(Points& P, vector<LinearInt9MMove>& Result)const;  

   void DistanceTo(const double x, const double y,MovingRealUnit& result)const;
 
 private:
   RelInterval interval;
   PBBox bbox;
   double startX;
   double startY;
   double endX;
   double endY;
   bool isStatic;
   bool defined;
};

/*
~An stream operator~

This operator can be used for debugging purposes. Its defines
a formatted output for a LinearPointMove. 

*/

ostream& operator<<(ostream& os, const LinearPointMove LPM){
      os << "("<<LPM.startX<<", "<<LPM.startY<<")->("
         <<LPM.endX<<", "<<LPM.endY<<") "<<LPM.interval;
      return os; 
   }



/*
2.3.7 The Class LinearPointsMove

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
   TwoPoints(){}
   TwoPoints(const double xs, const double ys,
             const double xe, const double ye);
   inline bool InnerIntersects(const TwoPoints &TP) const;
   inline bool IsStatic() const;
   int  CompareTo(const TwoPoints TP) const {
        if(startX<TP.startX) return -1;
        if(startX>TP.startX) return 1;
        if(startY<TP.startY) return -1;
        if(startY>TP.startY) return 1;
        if(endX<TP.endX) return -1;
        if(endX>TP.endX) return 1;
        if(endY<TP.endY) return -1;
        if(endY>TP.endY) return 1;
        return 0; 
   }
   bool operator< (const TwoPoints TP)const { return CompareTo(TP)<0; }
   bool operator> (const TwoPoints TP)const { return CompareTo(TP)>0; }
   bool operator== (const TwoPoints TP)const { return CompareTo(TP)==0; }
   inline bool IsSpatialExtension(const TwoPoints* TP) const;
   inline double Speed(const RelInterval interval) const;
   inline double GetStartX()const { return startX; } 
   inline double GetEndX()const {return endX;}
   inline double GetStartY()const {return startY;}
   inline double GetEndY()const {return endY;}
  private:
      double startX;
      double startY;
      double endX;
      double endY;
};

/*
2.3.7.2 The LinearPointsMove Class


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
   LinearPointsMove(){}
   LinearPointsMove(int dummy);
   ListExpr ToListExpr(const DBArray<TwoPoints> &Points) const;
   bool ReadFrom(const ListExpr value, 
                 DBArray<TwoPoints> &Points, 
     int &Index);
   Points* At(const DateTime* duration, 
                     const DBArray<TwoPoints> &Pts) const ;
   bool IsDefinedAt(const DateTime* duration) const;
   bool ProhablyIntersects(const PBBox* window) const;
   bool IsDefined() const{ return defined;}
   bool IsStatic() const { return isStatic;}
   void SetUndefined();
   bool CanBeExtendedBy(const LinearPointsMove* P2,
                        DBArray<TwoPoints> &MyPoints,
                        DBArray<TwoPoints> &PointsOfP2) const;
   bool ExtendWith(const LinearPointsMove* P2,
                   DBArray<TwoPoints> &MyPoints,
                   DBArray<TwoPoints> &PointsOfP2);
   void Equalize(const LinearPointsMove* P2);
   unsigned int GetStartIndex()const { return startIndex; }
   unsigned int GetEndIndex()const {return endIndex; } 
 private:
   RelInterval interval;
   PBBox bbox;
   unsigned int startIndex;
   unsigned int endIndex;
   bool isStatic;
   bool defined;
};

/*
2.3.7.3 ArrayRange

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
2.3.8 The Class PMSimple

This class represents a simple periodic moving type.
The managed type is T, the used unit representation is defined by Unit.
This template can be used for all type with an unit representation of
fixed size without any further summarizing attributes (e.g. bounding box)
in the nodes of the repetition tree.

*/
template <class T, class Unit>
class PMSimple : public StandardAttribute {
  public:
/*
~The Standard Constructor~

This constructor makes nothing. 

*/
     PMSimple() {}

/*
~Constructor~

This constructor should be used for creating an empty
object of this type. The argument is ignored. It's only
used to make this constructor different to the standard
constructor.

*/ 
     PMSimple(int dummy): 
         linearMoves(1),
         compositeMoves(1),
         compositeSubMoves(1),
         periodicMoves(1),
         defined(false),
         canDelete(false),
         interval(1),
         startTime(instanttype){
           __TRACE__
     }

/*
~The Destructor~

The Destructor destroys all flobs if allowed.

*/

    virtual  ~PMSimple() {
       __TRACE__
       if(canDelete){
          linearMoves.Destroy();
          compositeMoves.Destroy();
          compositeSubMoves.Destroy();
          periodicMoves.Destroy();
       }
     }

/*
~Destroy~

If this function is called, the destructor can destroy the
FLOB objects.

[3] O(1)

*/
    void Destroy(){
        __TRACE__
       canDelete = true;
    }

/*
~Equalize~

This function takes the value for this PMSimple from another instance __B__.

[3] O(L), where L is the number of contained linear moves

*/
  void Equalize(const PMSimple<T,Unit>* B2){
      __TRACE__
    // equalize the linear moves
    const Unit* LM=NULL;;
    linearMoves.Clear();
    if(B2->linearMoves.Size()>0) linearMoves.Resize(B2->linearMoves.Size());
    for(int i=0;i<B2->linearMoves.Size();i++){
       B2->linearMoves.Get(i,LM);
       linearMoves.Append(*LM);
    }
    // equalize the composite moves
    const CompositeMove* CM=NULL;
    compositeMoves.Clear();
    if(compositeMoves.Size()>0)
       compositeMoves.Resize(B2->compositeMoves.Size());
    for(int i=0;i<B2->compositeMoves.Size();i++){
       B2->compositeMoves.Get(i,CM);
       compositeMoves.Append(*CM);
    }

    // equalize the composite submoves
    const SubMove* SM=NULL;
    compositeSubMoves.Clear();
    if(compositeSubMoves.Size()>0)
       compositeSubMoves.Resize(B2->compositeSubMoves.Size());
    for(int i=0;i<B2->compositeSubMoves.Size();i++){
       B2->compositeSubMoves.Get(i,SM);
       compositeSubMoves.Append(*SM);
    }

    // equalize the periodic moves
    const PeriodicMove* PM = NULL;
    periodicMoves.Clear();
    if(periodicMoves.Size()>0)periodicMoves.Resize(B2->periodicMoves.Size());
    for(int i=0;i<B2->periodicMoves.Size();i++){
       B2->periodicMoves.Get(i,PM);
       periodicMoves.Append(*PM);
    }

    defined = B2->defined;
    interval.Equalize(&(B2->interval));
    startTime.Equalize(&(B2->startTime));
    submove.Equalize(&(B2->submove));
  }


/*
~NumOfFLOBs~

Returns four because four DBArrays are managed.

[3] O(1)

*/
     int NumOfFLOBs(){
        __TRACE__
        return 4;
     }

/*
~GetFLOB~

This function returns the FLOB at the specified index. If the index
is out of range, the system will going down. The flobs are:

  * 0: the units

  * 1: the composite moves

  * 2: the composite submoves

  * 3: the periodic moves

[3] O(1)

*/
    FLOB* GetFLOB(const int i){
        __TRACE__
      assert(i>=0 && i<NumOfFLOBs());
       if(i==0) return &linearMoves;
       if(i==1) return &compositeMoves;
       if(i==2) return &compositeSubMoves;
       if(i==3) return &periodicMoves;
       return 0;
    }

/*
~Compare~

Because we have no unique representation of the periodic moves, this function is
not implemented in this moment.

[3] O(-1)

*/
    int Compare(const Attribute* arg) const{
        __TRACE__
      cout << "PMSImple::Compare not implemented yet " << endl;
      return -1;
    }
     
/*
~Adjacent~

Returns false because it's not clear when two moving constants should be
adjacent.

[3] O(1)

*/
    bool Adjacent(const Attribute*)const{
        __TRACE__
       return false;
    }

/*
~Clone~

This functions creates a new periodic moving constant with the same value
like the calling instance and returns it.

[3] O(L), where L is the number of contained linear moves

*/
    PMSimple<T,Unit>* Clone() const{
        __TRACE__
      PMSimple<T,Unit>* res = new PMSimple<T,Unit>(1);
      res->Equalize(this);
      return res;
    }

/*
~SizeOf~

Returns the size of the class PMSimple.

[3] O(1)

*/
    int Sizeof()const{
        __TRACE__
       return sizeof(PMSimple<T,Unit>);
    }
/*
~IsDefined~

Returns the value of the __defined__ flag.

*/

     bool IsDefined() const{return defined;}

/*
~SetDefined~

Sets the __defined__ flag to the given value. You can always set a
PMSimple to be undefined. If this flag is set to true but the 
state of this PMSimple is not correct, the results will be 
inpredictable. 

*/
     void SetDefined( bool defined ){this->defined = defined;}

/*
~HashValue~

This functions computes a HashValue for this PMSimple from the sizes of
the contained FLOBs.

[3] O(1)

*/
    size_t HashValue() const{
        __TRACE__
      return (size_t) (interval.HashValue()+linearMoves.Size()+
                       compositeMoves.Size()+ periodicMoves.Size()+
                       compositeSubMoves.Size());
    }

/*
~CopyFrom~

By calling this function the value of this instance will be
equalized with this one of the attribute. The attribute must be
of Type PMSimple. If __arg__ is of another type, the result is
not determined and can lead to a crash of [secondo].

[3] O(L), where L is the number of contained linear moves

*/
    void CopyFrom(const StandardAttribute* arg){
        __TRACE__
       Equalize((PMSimple<T,Unit>*) arg);
    }
     
/*
~ToListExpr~

This function converts a periodic moving constant into its
nested list representation.

[3] O(L), where L is the number of contained linear moves

*/
   ListExpr ToListExpr()const {
      __TRACE__
     ListExpr timelist = startTime.ToListExpr(true);
     ListExpr SubMoveList = GetSubMoveList(submove);
         return nl->TwoElemList(timelist,SubMoveList);
   }
     


/*
~ReadFrom~

Takes the value of this simple periodic moving object from a
nested list. The result informs about the success of this
call. If the List don't represent a valid periodic moving
object, this instance will be undefined. This behavior 
differns to other ~readFrom~ functions to avoid some
expensive tests before creating the actual object.

[3] O(L), where L is the number of contained linear moves

*/
    bool ReadFrom(const ListExpr value){
        __TRACE__
       /* The list is scanned twice. In the first scan we
         compute only the needed size of the contained arrays.
         The reason is, that we want to avoid frequently ~Resize~
         on the arrays.
       */
      if(nl->ListLength(value)!=2){
         if(DEBUG_MODE){
            cerr << __POS__ << ": wrong listlength (";
            cerr << (nl->ListLength(value)) << ")" << endl;
         }
         SetDefined(false);
         return false;
      }

      if(!ResizeArrays(value)){
         if(DEBUG_MODE){
            cerr << __POS__ << ": resize array failed" << endl;
         }
         SetDefined(false);
         return false;
      }

      if(!startTime.ReadFrom(nl->First(value),true)){
         if(DEBUG_MODE){
            cerr << __POS__ << "reading of the start time failed" << endl;
            cerr << "The list is " << endl;
            nl->WriteListExpr(nl->First(value));
         }
         SetDefined(false);
         return false;
      }

      // now we have to append the included submove
      ListExpr SML = nl->Second(value);
      if(nl->ListLength(SML)!=2){
         if(DEBUG_MODE){
            cerr << __POS__ << ": wrong list length for submove" << endl;
         }
         SetDefined(false);
         return false;
      }

      ListExpr SMT = nl->First(SML);
      int LMIndex = 0;
      int CMIndex = 0;
      int SMIndex = 0;
      int PMIndex = 0;
      if(nl->IsEqual(SMT,"linear")){
         submove.arrayNumber = LINEAR;
         submove.arrayIndex = 0;
         if(!AddLinearMove(nl->Second(SML),LMIndex,CMIndex,SMIndex,PMIndex)){
             if(DEBUG_MODE){
                cerr << __POS__ << " Error in reading linear move" << endl;
             }
             SetDefined(false);
             return false;
         }
         defined=true;

         const Unit* LM;
         linearMoves.Get(0,LM);
         interval.Equalize(&(LM->interval));
         return true;
      }
      if(nl->IsEqual(SMT,"composite")){
         submove.arrayNumber=COMPOSITE;
         submove.arrayIndex = 0;
         if(!AddCompositeMove(nl->Second(SML),LMIndex,
                              CMIndex,SMIndex,PMIndex)){
            if(DEBUG_MODE){
               cerr << __POS__ << "error in reading composite move" << endl;
            }
            SetDefined(false);
            return false;
         }
         defined = true;
         const CompositeMove* CM;
         compositeMoves.Get(0,CM);
         interval.Equalize(&(CM->interval));
         return true;
      }
      if(nl->IsEqual(SMT,"period")){
         submove.arrayNumber = PERIOD;
         submove.arrayIndex = 0;
         if(!AddPeriodMove(nl->Second(SML),LMIndex,CMIndex,SMIndex,PMIndex)){
            if(DEBUG_MODE){
              cerr << __POS__ << " error in reading periodic move" << endl;
            }
            SetDefined(false);
            return false;
         }
         defined = true;
         const PeriodicMove* PM;
         periodicMoves.Get(0,PM);
         interval.Equalize(&(PM->interval));
         return true;
      }
      if(DEBUG_MODE){
         cerr << __POS__ << "unknown subtype" << endl;
         nl->WriteListExpr(SMT);
      }
      return false;
    }
/*
~IsEmpty~

This function yields __true__ if this periodic moving constant don't 
contain any unit.

*/
     bool IsEmpty()const{ return linearMoves.Size()==0;}
  

/*
~At~

This function returns a pointer to a new created instance of type 
T. If this periodic moving constant is not defined at this
timepoint, the result will be 0. The caller of this function has to
delete the returned value of this function if the result is not null.

[3] O(L), where L is the number of contained linear moves

*/
T* At(const DateTime* instant)const{
    __TRACE__
   DateTime* duration = new DateTime(*instant);
   duration->Minus(&startTime); // now it is a duration
   T* res = 0;
   if(interval.Contains(duration)){
   // in the other case, we have nothing to do
   // because the result is initialized with "undefined"
   const SubMove* sm = &submove;
   const CompositeMove* CM;
   const PeriodicMove* PM;
   RelInterval RI;
   while(sm->arrayNumber!=LINEAR){
      if(sm->arrayNumber==COMPOSITE){
         // in this stage of implementation a linear search is
         // executed. i have to make it better in the future
         int i = sm->arrayIndex;
         compositeMoves.Get(i,CM);
         int cur = CM->minIndex;
         int max = CM->maxIndex;
         bool found=false;
         while( (cur<=max) && ! found){
             compositeSubMoves.Get(cur,sm); // get the submove
             if(sm->arrayNumber==LINEAR){
                const Unit* LM;
                linearMoves.Get(sm->arrayIndex,LM);
                RI = LM->interval;
              } else if(sm->arrayNumber==PERIOD){
                periodicMoves.Get(sm->arrayIndex,PM);
                RI = PM->interval;
              } else { //another submoves are not allowed
                assert(false);
              }
              if(RI.Contains(duration)) // be happy
                 found=true;
              else{  // search again
                 DateTime* L = RI.GetLength();
                 duration->Minus(L);
                 delete L;
                 L = NULL;
                 cur++;
              }
         }
         assert(found); //otherwise we have an error in computation
      } else if(sm->arrayNumber==PERIOD){
         int index = sm->arrayIndex;
         periodicMoves.Get(index,PM);
         // this is a very slow implementation
         // i have to speed up it in the future
         sm = &PM->submove;
         RelInterval RI;
         if(sm->arrayNumber==LINEAR){
             const Unit* LM;
             linearMoves.Get(sm->arrayIndex,LM);
             RI = LM->interval;
          } else if(sm->arrayNumber==COMPOSITE){
             compositeMoves.Get(sm->arrayIndex,CM);
             RI = CM->interval;
          } else { //another submoves are not allowed
             assert(false);
          }
          while(!RI.Contains(duration)){
             DateTime* L = RI.GetLength();
             duration->Minus(L);
             delete L;
             L = NULL;
          }
          } else{
             // this case should never occurs
             assert(false);
          }
       }
       const Unit* LM;
       linearMoves.Get(sm->arrayIndex,LM);
       if(LM->IsDefinedAt(duration)){
          T tmpres = LM->At(duration);
          res = new T();
          *res = tmpres;
       }
    }
    delete duration;
    duration = NULL;
    return res;
}


/*
~Initial~

This function computes the first defined value of this moving constant.

*/
T* Initial() const{
    __TRACE__
  T* res = 0;
  if(!defined){
     return res;
  }
  if(IsEmpty()){
    return res;
  }
  /* the first units is also the first
     unit in the dbarray of the units.
  */
  const Unit* lm;
  linearMoves.Get(0,lm);
  res = lm->Initial(); 
  return res;
}

/*
~Final~

The ~Final~ function returns the last defined value of this periodic
moving object.
If none exists, NULL is returned.

*/
T* Final(){
    __TRACE__
 T* res = 0;
 if(!defined){
    return res;
 }
 if(IsEmpty()){
     return res;
  }       
  Unit lm =  GetLastUnit();
  res = lm.Final();
  return res;
}

 
/*
~Minimize~

When this function is called, consecutive units with the same value are
summarized into single units.

*/
void Minimize(){
   if(MinimizationRequired()){
     DBArray<Unit> newLinearMoves(1);
     DBArray<CompositeMove>          newCompositeMoves(1);
     DBArray<SubMove>                newCompositeSubMoves(1);
     DBArray<PeriodicMove>           newPeriodicMoves(1);
     Unit                            Summarization;
     bool                            CompleteSummarized;
     SubMove SM3 = MinimizeRec(submove,newLinearMoves,newCompositeMoves,
                       newCompositeSubMoves,
                       newPeriodicMoves,Summarization,CompleteSummarized);
     if(CompleteSummarized){
        // the whole movement can be represented in a single linear move
        compositeMoves.Clear();
        compositeSubMoves.Clear();   
        periodicMoves.Clear();
        linearMoves.Clear();
        linearMoves.Append(Summarization);
        submove.arrayNumber = LINEAR;
        submove.arrayIndex = 0;
     } else{
       submove = SM3;
       
     // copy the new created objects into the local dbarrays
       int size;
       
       linearMoves.Clear();
       const Unit* LM;
       size = newLinearMoves.Size();
       if(size>0)
           linearMoves.Resize(size);
       for(int i=0;i<size;i++){
          newLinearMoves.Get(i,LM);
          linearMoves.Put(i,*LM);
       }
 
       compositeMoves.Clear();
       const CompositeMove* CM;
       size = newCompositeMoves.Size();
       if(size>0)
           compositeMoves.Resize(size);
       for(int i=0;i<size;i++){
          newCompositeMoves.Get(i,CM);
          compositeMoves.Put(i,*CM);
       }

       compositeSubMoves.Clear();
       const SubMove* SM;
       size = newCompositeSubMoves.Size();
       if(size>0)
           compositeSubMoves.Resize(size);
       for(int i=0;i<size;i++){
          newCompositeSubMoves.Get(i,SM);
          compositeSubMoves.Put(i,*SM);
       }
       
       periodicMoves.Clear();
       const PeriodicMove* PM;
       size = newPeriodicMoves.Size();
       if(size>0)
           periodicMoves.Resize(size);
       for(int i=0;i<size;i++){
          newPeriodicMoves.Get(i,PM);
          periodicMoves.Put(i,*PM);
       }

     } 
   }
}

/*
~Statistical Information~ 

The following functions can be used in debugging for get some 
additional information about the inner structure of this instant.

*/
 int NumberOfLinearMoves(){
    return linearMoves.Size();
 }

 int NumberOfCompositeMoves(){
    return compositeMoves.Size();
 }

 int NumberOfCompositeSubMoves(){
    return compositeSubMoves.Size();
 }

 int NumberOfPeriodicMoves(){
    return periodicMoves.Size();
 }

/* 
~SetRightClosed~

This function sets the property of the right closeness 
to the given value. 

*/
void SetRightClosed(SubMove submove, bool value){
  if(submove.arrayNumber == LINEAR){
      Unit u;
      linearMoves.Get(submove.arrayIndex,u);
      RelInterval i;
      u.GetInterval(i);
      if(i.IsRightClosed()==value) // no change required
          return;
      i.SetRightClosed(value);
      u.SetInterval(i);
      linearMoves.Out(submove.arrayIndex,u);
      return; 
  }
  if(submove.arrayNumber == COMPOSITE){
      CompositeMove CM;
      compositeMoves.Get(submove.arrayIndex,CM);
      compositeSubMoves.Get(CM.maxIndex,submove);
      SetRightClosed(submove,value);
      return;
  }
  if(submove.arrayNumber == PERIOD){
      PeriodicMove PM;
      periodicMoves.Get(submove.arrayIndex,CM);
             
  }
}


/*
~Split~

Calling this function splits this moving object at the given instant. 
The splitpoint is given by an instant as well as a flag describing whether 
this instant should be contained in the 'left' part. The results are stored
into the corresponding arguments. When the instant is outside the definition
time of this moving object, oe part will contain a copy of this, the other
part will be undefined.

*/
void Split(const DateTime instant,const bool toLeft, 
           PMSimple<T,Unit>& leftPart, PMSimple<T,Unit>& rightPart){
   SubMove SMLeft,SMRight;
   SMLeft.arrayNumber = -1;
   SMRight.arrayNumber = -1;
   DateTime stCopy = startTime;
   splitRec(instant, toLeft,leftPart,rightPart,submove,stCopy,SMLeft,SMRight);
   // set the values of the root record of the left part
   if(SMLeft.arrayNumber<0){ // leftpart is empty
        leftPart.defined=false;  
   } else{ // leftpart is non-empty
        leftPart.defined = true;
        leftPart.startTime = startTime;
        leftPart.GetInterval(SMLeft,leftPart.interval);
   }
   // set the values of the root record of the right part
   if(SMRight.arrayNumber<0){
        rightPart.defined=false;
   }else{
        rightPart.defined=true;
        rightPart.startTime = instant;
        rightPart.GetInterval(SMRight,rightPart.interval);
   }
   cerr << "Missing correction for splitted infinite periodic  moves !!!" << endl;
}


/*
~SpliRec~

This function splits the given submove of this periodic moving objects into 
two part. The leftPart will be extended by the part of this submove before
the given instant. The remainder of this submove will stored into rightPart.
The new added submove of leftPart and rightPart is returned in the arguments
SMLeft and SMRight respectively. The argument toLeft controls whereto put
in the object at the given instant (closure properties). The starttime is changed
to be after the processed submove. If the submove is not defined before the 
given instant, leftPart is not changed and SMLeft will hold an negative number
for the arrayNumber indicating an invalid value. When submove is'nt defined 
after this instant, the same holds for rightPart and SMRight.

*/
void splitRec(const DateTime instant, const bool toLeft,
              PMSimple<T,Unit>& leftPart, PMSimple<T,Unit>& rightPart,
              SubMove submove, DateTime& startTime, SubMove& SMLeft, SubMove& SMRight){
 

  if(submove.arrayNumber==LINEAR){
    // here we process a single unit. this case is easy to handle because a
    // unit have to provide a split function
    const Unit* LMLtmp=NULL;
    Unit LML;
    linearMoves.Get(submove.arrayIndex,LMLtmp);
    LML=(*LMLtmp);
    Unit LMR;
    DateTime Dur = instant - startTime;
    startTime = startTime + (*LML.interval.GetLength()); 
    bool saveLeft,saveRight;
    if(LML.GetDefined()){
       LML.Split(Dur,toLeft,LMR);
       saveLeft = LML.GetDefined();
       saveRight = LMR.GetDefined();
     } else{ // original is undefined
        LML.interval.Split(Dur,toLeft,LMR.interval);
        LMR.SetDefined(false);
        saveLeft = LML.interval.IsDefined();
        saveRight = LMR.interval.IsDefined();
     }
     if(saveLeft){
         SMLeft.arrayNumber=LINEAR;
         SMLeft.arrayIndex = leftPart.linearMoves.Size();
         leftPart.linearMoves.Append(LML);
     }else{
         SMLeft.arrayNumber = -1;
     }
     // do the same thing with the right part
     if(saveRight){
        SMRight.arrayNumber=LINEAR;
        SMRight.arrayIndex=rightPart.linearMoves.Size();
        rightPart.linearMoves.Append(LML);
     } else{
        SMRight.arrayNumber = -1;
     }
     return;  // linear moves processed
  }
  if(submove.arrayNumber==COMPOSITE){
     // we store the submoves of this composite moves into an vector 
     // to ensure a single block representing this submove
     const CompositeMove* CM=NULL;
     compositeMoves.Get(submove.arrayIndex,CM);
     vector<SubMove> submovesLeft(CM->maxIndex-CM->minIndex+1);
     vector<SubMove> submovesRight(CM->maxIndex-CM->minIndex+1);
     
     const SubMove* SMtmp;
     SubMove SM;
     for(int i=CM->minIndex;i<=CM->maxIndex;i++){
        compositeSubMoves.Get(i,SMtmp); // get the submove
        SM = (*SMtmp);
        // call recursive
        splitRec(instant,toLeft,leftPart,rightPart,SM,startTime,SMLeft,SMRight);
        // collect valid submoves
       if(SMLeft.arrayNumber>=0)
           submovesLeft.push_back(SMLeft);
       if(SMRight.arrayNumber>=0)
           submovesRight.push_back(SMRight);
     }
     // process the left part
     int size = submovesLeft.size();
     if(size==0){
        SMLeft.arrayNumber = -1;
     } else if (size>1){ // in case size==1 is nothing to do
        CompositeMove CMLeft;
        CMLeft.minIndex = leftPart.compositeSubMoves.Size();
        for(int i=0;i<size;i++){
            leftPart.compositeSubMoves.Append(submovesLeft[i]); 
        }
        CMLeft.maxIndex = leftPart.compositeSubMoves.Size()-1;
        SMLeft.arrayNumber = COMPOSITE;
        SMLeft.arrayIndex = leftPart.compositeMoves.Size();
        leftPart.compositeMoves.Append(CMLeft); 
     }
     // process the right part
     size = submovesRight.size();
     if(size==0){ // nothing remains at the right
        SMRight.arrayNumber = -1;
     } else if(size>1){
         CompositeMove CMRight;
         CMRight.minIndex = rightPart.compositeSubMoves.Size();
         for(int i=0;i<size;i++){
             rightPart.compositeSubMoves.Append(submovesRight[i]);
         }
         CMRight.maxIndex = rightPart.compositeSubMoves.Size()-1;
         SMRight.arrayNumber = COMPOSITE;
         SMRight.arrayIndex = rightPart.compositeMoves.Size();
         rightPart.compositeMoves.Append(CMRight);
     }
     return; // composite moves processed
  }
  if(submove.arrayNumber==PERIOD){
     // first, we check whether the split instant is outside the interval
     const PeriodicMove*  PMtmp=NULL;
     periodicMoves.Get(submove.arrayIndex,PMtmp);
     PeriodicMove PM = (*PMtmp);
     RelInterval sminterval;
     GetInterval(PM.submove,sminterval);
     DateTime smlength;
     sminterval.StoreLength(smlength);
     DateTime ZeroTime(instanttype);


     sminterval.StoreLength(smlength);
     
     // find out how many repeatations left on both sides
     // first compute the duration which should be transferred into the left part
     DateTime LeftDuration = instant - startTime;
     
     



     return; // periodic moves processed
  }
  assert(false); // unknown submove
}

/*
~SplitLeft~

This function equalizes leftPart with the part of this move which is located
before the instant.

*/
void SplitLeft(const DateTime& instant,const bool toLeft, PMSimple<T,Unit>* result){
  SubMove LastMove;
  DateTime startTimecopy;
  startTimecopy.Equalize(&startTime);

  if(!SplitLeftRec(instant,toLeft,*result,submove,startTimecopy,LastMove))
      result->SetDefined(false);
  else{
     result->startTime = startTime;
     DateTime len = instant-startTime;
     result->interval.Set(&len,interval.IsLeftClosed(),toLeft);
     result->submove = LastMove;      
  }  
}

bool SplitLeftRec(const DateTime& instant,const bool toLeft,
                  PMSimple<T,Unit>& result,const SubMove& submove, 
                  DateTime& startTime,SubMove& lastSubmove){

/*
First, we handle the a single unit.

*/
   if(submove.arrayNumber==LINEAR){
        if(startTime > instant) // is right of the instant
            return false;
        const Unit* utmp;
        linearMoves.Get(submove.arrayIndex,utmp);
        Unit u = (*utmp);
        RelInterval interval;
        interval.Equalize(&u.interval);
        DateTime dur = instant-startTime;
        if(!u.interval.Contains(&dur)){
           return false;
        }
        Unit rightUnit;
        u.Split(dur,toLeft,rightUnit); 
        result.linearMoves.Append(u);
        DateTime len;
        interval.StoreLength(len);
        startTime = startTime + len;
        lastSubmove.arrayNumber=LINEAR;
        lastSubmove.arrayIndex = result.linearMoves.Size()-1;
        return true;   
   }
/*
Second, handling of composite moves

*/
   if(submove.arrayNumber == COMPOSITE){
      const CompositeMove* CMtmp=NULL;
      compositeMoves.Get(submove.arrayIndex,CMtmp);
      CompositeMove CM = *CMtmp;
      DateTime dur = instant-startTime;
      if(!CM.interval.Contains(&dur))
          return false;        
      SubMove SM;
      const SubMove* SMtmp;
      vector<SubMove> mySubmoves(CM.maxIndex-CM.minIndex);
      bool done = false;
      for(int i =CM.minIndex;i<=CM.maxIndex && !done;i++){
           compositeSubMoves.Get(i,SMtmp);
           SM = (*SMtmp);
           if(SplitLeftRec(instant,toLeft,result,SM,startTime,lastSubmove)){
                mySubmoves.push_back(lastSubmove);
           } else { // after this move can't follow any further moves
             done=true;
           }
      } 
      if(mySubmoves.size()<2){ // ok, this submoves is end in smoke
        return true;
      }else{ // we have to build a composite move from the remaining submoves
         DateTime length(durationtype);
         DateTime nextLength(durationtype);
         int size = mySubmoves.size();
         RelInterval interval; 
         CM.minIndex = result.compositeSubMoves.Size();
         for(int i=0;i<size;i++){
            SM = mySubmoves[i];
            if(i==0)
                result.GetInterval(SM,CM.interval);
            else{
                result.GetInterval(SM,interval);
                CM.interval.Append(&interval);
            }
            result.compositeSubMoves.Append(SM);
         }      
         CM.maxIndex = result.compositeSubMoves.Size()-1;
         // append the composite move
         lastSubmove.arrayIndex=COMPOSITE;
         lastSubmove.arrayIndex = result.compositeMoves.Size();
         result.compositeMoves.Append(CM);
         return true;
      }
   }

   if(submove.arrayIndex==PERIOD){
       const PeriodicMove* PM;
       periodicMoves.Get(submove.arrayIndex,PM);
       DateTime dur = instant-startTime;
       
   }
   assert(false); // the program should never reach this position


}







/*
~Intersection~

This function reduces the definition time of this PMSimple to the 
interval defined by the arguments. This function cannot be constant, i.e. 
it changes the *this* argument,  because
we have to scan the dbarrays in some cases.

*/
 void  Intersection( const DateTime minTime, 
                     const bool minIncluded, 
                     const DateTime maxTime, 
                     const bool maxIncluded,
                    PMSimple<T,Unit>* res){

      PMSimple<T,Unit> TMPPM1;
      PMSimple<T,Unit> TMPPM2;
      Split(minTime,!minIncluded,TMPPM2,TMPPM1);
      //TMPPM1.Split(maxTime,maxIncluded,TMPPM1,*result);
      TMPPM1.SplitLeft(maxTime,maxIncluded,res);
   }

/*
~CopyValuesFrom~

By calling this function, the value of this pmsimple is set to the given values.
There is no check whether the values are valid. Use this function very carefully.
A possible application of this function is to take the tree from another periodic 
moving object.

*/
  void CopyValuesFrom( DBArray<Unit>& linearMoves,
                      DBArray<CompositeMove>& compositeMoves,
                      DBArray<SubMove>& compositeSubMoves,
                      DBArray<PeriodicMove>& periodicMoves,
                      bool defined,
                      RelInterval interval,
                      DateTime startTime,
                      SubMove submove){

    // first, clear all contained arrays
    this->linearMoves.Clear();
    this->compositeMoves->Clear();
    this->compositeSubMoves.Clear();
    this->periodicMoves.Clear();
    // resize the array and copy contents
    int size;
    if((size=linearMoves.Size())>0){
         this->linearMoves.Resize(size);
         Unit U;
         for(int i=0;i<size;i++){
            linearMoves.Get(i,U);
            this->linearMoves.Put(i,U);
         } 
    }
    if((size=compositeMoves.Size())>0){
        this->compositeMoves.Resize(size);
        CompositeMove CM;
        for(int i=0;i<size;i++){
           compositeMoves.Get(i,CM);
           this->compositeMoves.Put(i,CM);
        } 
    }
    if((size=compositeSubMoves.Size())>0){
       this->compositeSubMoves.Resize(size);
       SubMove SM;
       for(int i=0;i<size;i++){
          compositeSubMoves.Get(i,SM);
          this->compositeSubMoves.Put(i,SM);
       } 
    }
    if((size=periodicMoves.Size())>0){
        this->periodicMoves.Resize(size);
        PeriodicMove PM;
        for(int i=0;i<size;i++){
           periodicMoves.Get(i,PM);
           this->periodicMoves.Put(PM);
        }
    }
    this.defined=defined;
    this.canDelete=false;
    this->interval.Equalize(&interval);
    this->startTime.Equalize(&startTime);
    this.submove=submove; 
  } 

/*
~TakeValuesFrom~

This function works similar to the ~CopyValuesFrom~ function.
The difference is, that the values are assigned to the internal
variables. This variables are cleaned before.

*Note*: There is no check for the integrity of the given data. 

*/

  void TakeValuesFrom( DBArray<Unit>& linearMoves,
                      DBArray<CompositeMove>& compositeMoves,
                      DBArray<SubMove>& compositeSubMoves,
                      DBArray<PeriodicMove>& periodicMoves,
                      bool defined,
                      RelInterval interval,
                      DateTime startTime,
                      SubMove submove){

     this->linearMoves.Clear(); 
     this->linearMoves.Destroy();
     this->compositeMoves.Clear();
     this->compositeMoves.Destroy();
     this->compositeSubMoves.Clear();
     this->compositeSubMoves.Destroy();
     this->periodicMoves.Clear();
     this->periodicMoves.Destroy();
     this->linearMoves = linearMoves();
     this->compositeMoves = compositeMoves();
     this->compositeSubMoves = compositeSubMoves();
     this->periodicMoves = periodicMoves();
     this->defined = defined;
     this.interval = interval;
     this.startTimes = startTime;
     this.submove = submove;
  }

/*
~SetStartTime~

This function can be used for moving this PMsimple in time.

*/
  void SetStartTime(DateTime newStart){
      startTime.Equalize(&newStart);
  }

/*
~Function returning pointers to the members~

Each of the next functions returns a pointer for a member.
This way bypasses the protected declaration of the members 
because this function enables the possibility to manipulate the
members direcly without control of the PMSimple class. This may 
be not good for encapsulating the code but the only way to have
an efficient implementation for that. The caller of this functions
must be very very carefully to ensure that all the manipulated
members results in a consistent instance of type PMSimple. 
A possible application of this function is to make a copy of the
tree structure of another periodic moving object. 

*/
  DBArray<Unit>* GetLinearMoves(){return &linearMoves;}
  DBArray<CompositeMove>* GetCompositeMoves(){return &compositeMoves;}
  DBArray<SubMove>* GetCompositeSubMoves(){return &compositeSubMoves;}
  DBArray<PeriodicMove>* GetPeriodicMoves(){return &periodicMoves;}
  RelInterval* GetInterval(){return &interval;}
  SubMove* GetSubmove(){ return &submove;}


  protected:
     DBArray<Unit> linearMoves;
     DBArray<CompositeMove> compositeMoves;
     DBArray<SubMove> compositeSubMoves;
     DBArray<PeriodicMove> periodicMoves;
     bool defined;
     bool canDelete;
     RelInterval interval;
     DateTime startTime;
     SubMove submove;


/*

~GetLength~

This function returns the lenght of the interval of the given 
submove. 

*/
   void GetLength(SubMove sm, DateTime& result){
      switch(sm.arrayNumber){
        case LINEAR: { Unit U;
                       linearMoves.Get(sm.arrayIndex,U);
                        U.interval.StoreLength(result);
                     } break;
        case COMPOSITE: {
                         CompositeMove CM; 
                         CM.interval.StoreLength(result); 
                        }break;

       case PERIOD: {
                     PeriodicMove CM;
                     CM.interval.StoreLength(result);
                    } break;
       default: assert(false); // unknown move type

      }

   }

/*

~GetLeftClosed~

This function returns the state of the LeftClosed flag  of the interval of the given 
submove. 

*/
   bool GetLeftClosed(SubMove sm){
      switch(sm.arrayNumber){
        case LINEAR: { Unit U;
                       linearMoves.Get(sm.arrayIndex,U);
                        return  U.interval.IsLeftClosed();
                     } 
        case COMPOSITE: {
                         CompositeMove CM; 
                         return CM.interval.IsLeftClosed(); 
                        }

       case PERIOD: {
                     PeriodicMove PM;
                     return PM.interval.IsLeftClosed();
                    }
       default: assert(false); // unknown move type

      }

   }

/*

~GetRightClosed~

This function returns the state of the RightClosed flag  of the interval of the given 
submove. 

*/
   bool GetRightClosed(SubMove sm){
      switch(sm.arrayNumber){
        case LINEAR: { Unit U;
                       linearMoves.Get(sm.arrayIndex,U);
                        return  U.interval.IsRightClosed();
                     } 
        case COMPOSITE: {
                         CompositeMove CM; 
                         return CM.interval.IsRightClosed(); 
                        }

       case PERIOD: {
                     PeriodicMove PM;
                     return PM.interval.IsRightClosed();
                    }
       default: assert(false); // unknown move type
      }
   }

/*

~GetInterval~

This function returns the returns the interval of the given submove independly
of the type of the submove. 

*/
   void GetInterval(SubMove sm,RelInterval& interval){
      switch(sm.arrayNumber){
        case LINEAR: { const Unit* U;
                       linearMoves.Get(sm.arrayIndex,U);
                       interval.Equalize(&U->interval);
                     } break;
        case COMPOSITE: {
                         const CompositeMove* CM; 
                         interval.Equalize(&CM->interval); 
                        }break;

       case PERIOD: {
                     const PeriodicMove* PM;
                     interval.Equalize(&PM->interval);
                    }break;
       default: assert(false); // unknown move type
      }
   }

     /* the next four functions are needed to convert a
        periodic moving bool to its nested list representation.
     */


/*
~GetSubMoveList~

This function returns the list representation for the submove in the
argument.

[3] O(L), where L is the number of contained linear moves

*/
    ListExpr GetSubMoveList(const SubMove SM) const{
      __TRACE__
      ListExpr SubMoveList;
      int SubMoveType = SM.arrayNumber;
      int index = SM.arrayIndex;
      if(SubMoveType==LINEAR)
          SubMoveList = GetLinearMoveList(index);
      else if(SubMoveType==COMPOSITE)
          SubMoveList = GetCompositeMoveList(index);
      else if(SubMoveType==PERIOD)
          SubMoveList = GetPeriodicMoveList(index);
      else{
           cerr << __POS__ << " Error in creating ListExpr" << endl;
           SubMoveList = nl->TheEmptyList();
       }
      return SubMoveList;
    }
     
/*
~GetLinearMoveList~

This functions takes the linear move at the specified index and
returns its nested list representation.

[3] O(1)

*/
    ListExpr GetLinearMoveList(const int index)const{
        __TRACE__
       const Unit* LM=NULL;
       linearMoves.Get(index,LM);
       return LM->ToListExpr();
    }

/*
~GetPeriodicMoveList~

Creates a nested list for the periodic move at the specified index.

[3] O(L), where L is the number of contained linear moves

*/
    ListExpr GetPeriodicMoveList(const int index)const{
        __TRACE__
      const PeriodicMove* PM=NULL;
      periodicMoves.Get(index,PM);
      ListExpr periodtype = nl->SymbolAtom("period");
      ListExpr RepList = nl->IntAtom(PM->repeatations);
      ListExpr SML = GetSubMoveList(PM->submove);
      ListExpr RC = nl->BoolAtom(interval.IsRightClosed());
      ListExpr LC = nl->BoolAtom(interval.IsLeftClosed());
      return  nl->TwoElemList(periodtype,nl->FourElemList(RepList,LC,RC,SML));
    }
     
/*
~GetCompositeMoveList~

Returns the CompositeMove at the specified index in its nested list
representation.

[3] O(L), where L is the number of contained linear moves

*/
    ListExpr GetCompositeMoveList(const int index)const{
       __TRACE__
      const CompositeMove* CM=NULL;
      compositeMoves.Get(index,CM);
      ListExpr CType = nl->SymbolAtom("composite");
      int minIndex = CM-> minIndex;
      int maxIndex = CM->maxIndex;
      ListExpr SubMovesList;
      if(maxIndex<minIndex){
        cerr << __POS__ << "empty composite move" << endl;
        SubMovesList = nl->TheEmptyList();
      }
      else{
       // construct the List of submoves
        const SubMove* SM=NULL;
        compositeSubMoves.Get(minIndex,SM);
        SubMovesList = nl->OneElemList(GetSubMoveList(*SM));
        ListExpr Last = SubMovesList;
        for(int i=minIndex+1;i<=maxIndex;i++){
          compositeSubMoves.Get(i,SM);
          Last = nl->Append(Last,GetSubMoveList(*SM));
        }
      }
      return nl->TwoElemList(CType,SubMovesList);
    }


     /* The next functions help to read in a periodic moving
        bool from a nested list representation.
     */


/*
~ResizeArrays~

This functions resizes all array to be able to insert the moves
contained in the argument list.This function should be called 
before this instance is read from a nested list to avoid 
frequently resize of the contained DB-Arrays. The return
value of this function is an indicator for the correctness 
of the structure of this list. If the result is __false__,
the list don't represent a valid periodic moving object.
But a return value of __true__ don't guarantees a correct
list representation. 

[3] O(L), where L is the number of contained linear moves

*/
    bool ResizeArrays(const ListExpr value){
        __TRACE__
       // first all entries in the arrays are removed
       linearMoves.Clear();
       compositeMoves.Clear();
       compositeSubMoves.Clear();
       periodicMoves.Clear();
       int LMSize = 0;
       int CMSize = 0;
       int SMSize = 0;
       int PMSize = 0;
       if(!AddSubMovesSize(nl->Second(value),LMSize,CMSize,SMSize,PMSize))
          return false;
       // set the arrays to the needed size
       if(LMSize>0) linearMoves.Resize(LMSize);
       if(CMSize>0) compositeMoves.Resize(CMSize);
       if(SMSize>0) compositeSubMoves.Resize(SMSize);
       if(PMSize>0) periodicMoves.Resize(PMSize);
       return true;
    }


/*
~AddSubMovesSize~

Adds the number of in value contained submoves to the appropriate
size. If the result is__false__, the list is not a correct 
representation of a periodic moving object. If it is __true__
the content of the list can be wrong but the structure is correct.

[3] O(L), where L is the number of contained linear moves

*/
    bool AddSubMovesSize(const ListExpr value,int &LMSize,int &CMSize,
                         int &SMSize,int &PMSize){
        __TRACE__
     // all moves have the length 2
      if(nl->ListLength(value)!=2)
         return false;
      ListExpr type = nl->First(value);
      if(nl->AtomType(type)!=SymbolType)
        return false;
      // in a linear move we have only to increment the size of LM
      if(nl->IsEqual(type,"linear")){
         LMSize = LMSize +1;
         return true;
      }
      if(nl->IsEqual(type,"composite")){
         CMSize = CMSize+1; // the composite move itself
         ListExpr rest = nl->Second(value);
         SMSize = SMSize+nl->ListLength(rest); // the contained submoves
         while(!nl->IsEmpty(rest)){
            if(!AddSubMovesSize(nl->First(rest),LMSize,CMSize,SMSize,PMSize))
               return false;
            rest = nl->Rest(rest);
         }
         return true;
      }
      if(nl->IsEqual(type,"period")){
         PMSize = PMSize+1;
         int len = nl->ListLength(value);
         ListExpr PMove;
         if(len==2)
             PMove = nl->Second(value);
         else if(len==4)
              PMove = nl->Fourth(value);
         else // incorrect listlength
             return false;
         
         return AddSubMovesSize(nl->Second(PMove),
                                LMSize,CMSize,SMSize,PMSize);
      }
      // a unknown type description
      return false;
    }

/*
~AddLinearMove~

This function adds the LinearMove given in value to this PMSimple.
If the list represents a valid Unit, this unit is append to to
appropriate dbarray and the argument __LMIndex__ is increased.
If the list is incorrect, this periodic moving object is not
changed and the result will be __false__.

[3] O(1)

*/
    bool AddLinearMove(const ListExpr value,int &LMIndex, int &CMIndex,
                       int &SMIndex, int &PMIndex){
       __TRACE__

        Unit LM = Unit(0);
        if(!LM.ReadFrom(value))
           return false;
        linearMoves.Put(LMIndex,LM);
        LMIndex++;
        return true;
    }

/*
~AddCompositeMove~

This function creates a CompositeMove from value and adds it
(and all contained submoves) to this PMSimple. The return 
value corresponds to the correctness of the list for this
composite move. The arguments are increased by the number of
contained submoves in this composite move. *Note*: This
function can change this periodic moving object even when 
the list is not correct. In the case of a result of __false__,
the state of this object is not defined and the defined flag
should be set to false.

[3] O(L), where L is the number of contained linear moves

*/
    bool AddCompositeMove(const ListExpr value,int &LMIndex, int &CMIndex,
                       int &SMIndex, int &PMIndex){
       __TRACE__
       // a composite move has to contains at least two submoves
       int len = nl->ListLength(value);
       if(len<2){
          if(DEBUG_MODE){
             cerr << __POS__ << " less than 2 submoves (" 
                  << len << ")" << endl;
          }
          return false;
       }
       CompositeMove CM=CompositeMove(1);
       int CMPos=CMIndex;
       int SMPos=SMIndex;
       // ensure that no submove used the positions for this composite move
       CMIndex++;
       CM.minIndex=SMIndex;
       CM.maxIndex=SMIndex+len-1;
       SMIndex= SMIndex+len;
       // Append the contained Submoves
       ListExpr rest = value;
       ListExpr SML,TL,VL;
       bool isFirst = true;
       while(!nl->IsEmpty(rest)){
          SML = nl->First(rest);
          rest = nl->Rest(rest);
          if(nl->ListLength(SML)!=2){ // all submoves have the 
                                     // format (type value)
             if(DEBUG_MODE){
                cerr << __POS__ << " submove has wrong length (";
                cerr << nl->ListLength(SML) << ")" << endl;
             }
             return false;
          }
          TL = nl->First(SML);
          VL = nl->Second(SML);
          if(nl->IsEqual(TL,"linear")){
             // process a linear submove
             int LMPos = LMIndex;
             if(!AddLinearMove(VL,LMIndex,CMIndex,SMIndex,PMIndex)){
                if(DEBUG_MODE){
                   cerr << __POS__ << " can't add a linear move " << endl;
                }
                return false;
             }
             const Unit* LM=NULL;
             linearMoves.Get(LMPos,LM);
             // Append the interval of LM to CM
             if(isFirst){
                isFirst=false;
                CM.interval.Equalize(&(LM->interval));
             }else{
                if(!CM.interval.Append(&(LM->interval))){
                   if(DEBUG_MODE){
                       cerr << __POS__ << " can't append interval " << endl;
                       cerr << "The original interval is";
                       cerr << CM.interval.ToString() << endl;
                       cerr << "The interval to append is";
                       cerr << LM->interval.ToString() << endl;
                   }
                   return false;
                }
             }
             // put the submove in the array
             SubMove SM;
             SM.arrayNumber = LINEAR;
             SM.arrayIndex = LMPos;
             compositeSubMoves.Put(SMPos,SM);
             SMPos++;
          } else if(nl->IsEqual(TL,"period")){
            // process a periodic submove
            int PMPos = PMIndex;
            if(!AddPeriodMove(VL,LMIndex,CMIndex,SMIndex,PMIndex)){
               if(DEBUG_MODE){
                  cerr << __POS__ << "can't add period move " << endl;
                }
                return  false;
            }
            const PeriodicMove* PM=NULL;
            periodicMoves.Get(PMPos,PM);
            if(isFirst){
               isFirst=false;
               CM.interval.Equalize(&(PM->interval));
            }else{
               if(!CM.interval.Append(&(PM->interval))){
                  if(DEBUG_MODE){
                     cerr << __POS__  << " can't append interval" << endl;
                  }
                  return false;
               }
            }
            SubMove SM;
            SM.arrayNumber = PERIOD;
            SM.arrayIndex = PMPos;
            compositeSubMoves.Put(SMPos,SM);
            SMPos++;
       } else{ // not of type linear or period
            if(DEBUG_MODE){
                cerr << __POS__ << " submove not of type ";
                cerr << "linear od period" << endl;
             }
             return false;
          }
       }
       // put the compositeMove itself
       compositeMoves.Put(CMPos,CM);
       return true;
    }

/*
~AddPeriodMove~

Adds the period move described in value to this PMSimple.
*Note*: This function can change this objects even wehn 
the list does not represent a valid period move. If the 
result is __false__ the __defined__ flag of this move
should be set to false.

[3] O(L), where L is the number of contained linear moves

*/
    bool AddPeriodMove(const ListExpr value,int &LMIndex, int &CMIndex,
                       int &SMIndex, int &PMIndex){
       __TRACE__
       int len = nl->ListLength(value);
       if((len!=2) && (len!=4)){  // (repeatations <submove>)
          if(DEBUG_MODE)
            cerr << __POS__ << ": wrong listlength" << endl;
          return false;
       }
       if(nl->AtomType(nl->First(value))!=IntType){
         if(DEBUG_MODE){
           cerr << __POS__ << ": wrong type for repeatations" << endl;
         }
         return false;
       }
       int rep = nl->IntValue(nl->First(value));
       // rep must be greater than 1 
       if(rep<=1 ){
          if(DEBUG_MODE){
             cerr << __POS__ <<  " wrong number of repeatations" << endl;
          }
          return false;
       }
       
       ListExpr SML;
       if(len==2)
          SML = nl->Second(value);
       if(len==4)
          SML = nl->Fourth(value);

       if(nl->ListLength(SML)!=2){
          if(DEBUG_MODE){
             cerr << __POS__ << ": wrong length for submove" << endl;
          }
          return false;
       }
       PeriodicMove PM=PeriodicMove(1);
       PM.repeatations = rep;
       int IncludePos = PMIndex; // store the positiuon
       PMIndex++;
       ListExpr SMT = nl->First(SML); // take the submove type
       if(nl->IsEqual(SMT,"linear")){
         int LMPos = LMIndex;
         if(!AddLinearMove(nl->Second(SML),LMIndex,CMIndex,SMIndex,PMIndex)){
            if(DEBUG_MODE){
              cerr << __POS__ << ": can't add linear submove" << endl;
            }
            return false;
         }
         PM.submove.arrayNumber = LINEAR;
         PM.submove.arrayIndex = LMPos;
         const Unit* LM=NULL;
         linearMoves.Get(LMPos,LM);
         RelInterval SMI = LM->interval;
         PM.interval.Equalize(&SMI);
         PM.interval.Mul(rep);
         if(len==4){
             ListExpr LC = nl->Second(value);
             ListExpr RC = nl->Third(value);
            if((nl->AtomType(LC)!=BoolType) || (nl->AtomType(RC)!=BoolType))
              return false;
            PM.interval.SetLeftClosed(nl->BoolValue(LC));
            PM.interval.SetRightClosed(nl->BoolValue(RC));
         }

         periodicMoves.Put(IncludePos,PM);
         return true;
       }else if(nl->IsEqual(SMT,"composite")){
         int CMPos = CMIndex;
         if(!AddCompositeMove(nl->Second(SML),LMIndex,CMIndex,
                              SMIndex,PMIndex)){
            if(DEBUG_MODE){
               cerr << __POS__ << ": can't add composite submove" << endl;
            }
            return false;
         }
         PM.submove.arrayNumber = COMPOSITE;
         PM.submove.arrayIndex = CMPos;
         const CompositeMove* CM=NULL;
         compositeMoves.Get(CMPos,CM);
         RelInterval SMI = CM->interval;
         PM.interval.Equalize(&SMI);
         PM.interval.Mul(rep);
         if(len==4){
             ListExpr LC = nl->Second(value);
             ListExpr RC = nl->Third(value);
            if((nl->AtomType(LC)!=BoolType) || (nl->AtomType(RC)!=BoolType))
              return false;
            PM.interval.SetLeftClosed(nl->BoolValue(LC));
            PM.interval.SetRightClosed(nl->BoolValue(RC));
         }
         periodicMoves.Put(IncludePos,PM);
         return true;
       }
       return false; // invalid type
    }


/*
~GetLastUnit~

This function returns the temporal last unit of this periodic moving point.
It is realized by going down the repetition tree up to a unit.

*/
Unit GetLastUnit(){
    __TRACE__
    const SubMove* s = &submove;
    while(s->arrayNumber!=LINEAR){
        if(s->arrayNumber==PERIOD){
           const PeriodicMove* PM=NULL;
           periodicMoves.Get(s->arrayIndex,PM);
           s = &PM->submove;
        } else if(s->arrayNumber==COMPOSITE){
           const CompositeMove* CSM=NULL;
           compositeMoves.Get(s->arrayIndex,CSM);
           compositeSubMoves.Get(CSM->maxIndex,s);   
        } else{
          assert(false); // unknown arraynumber
        }
    }
    const Unit* res=NULL;
    linearMoves.Get(s->arrayIndex,res);
    return *res;
}
  
/*
~MinimizationRequired~

This function checks, whether an call of ~Minimize~ will change the
tree. Particularly, it checks whether two consecutive units can
be summarized to a single one.

*/

bool MinimizationRequired(){
  __TRACE__
  const Unit* LM=NULL;
  const Unit* LM2=NULL;
  const CompositeMove* CM=NULL;
  const PeriodicMove*  PM=NULL;
  const SubMove* SM=NULL;
  const SubMove* SM2=NULL;
  // check periodic moves
  int size = periodicMoves.Size();
  for(int i=0;i<size;i++){
      periodicMoves.Get(i,PM);
      SM = &PM->submove;
      if(SM->arrayIndex==LINEAR){
         linearMoves.Get(SM->arrayIndex,LM);
         if(LM->interval.CanAppended(&LM->interval))
             return true;
      }  
  }
  // check composite moves
  size = compositeMoves.Size();
  for(int i=0;i<size;i++){
     compositeMoves.Get(i,CM);
     for(int k=CM->minIndex;k< CM->maxIndex;k++){
        compositeSubMoves.Get(k,SM);
        compositeSubMoves.Get(k+1,SM2);
        if( (SM->arrayNumber==LINEAR) && (SM2->arrayNumber==LINEAR)){
            linearMoves.Get(SM->arrayIndex,LM);
            linearMoves.Get(SM2->arrayIndex,LM2);
            if( (LM->value==LM2->value) && 
                (LM->interval.CanAppended(&LM2->interval)))
                return true;
        }
     } 
  }
  return false;
}



/*
~MinimizeRec~

This function summarizes consecutive units with the same value.
The result is stored in the arguments. Note that no detection of
equal linear moves is performed. This means, when a unit occurs
twice or more, this unit will be also stored several times in
the corresponding dbarray.

*/

SubMove MinimizeRec(SubMove SM, 
                    DBArray<Unit>&                   newLinearMoves,
                    DBArray<CompositeMove>&          newCompositeMoves,
                    DBArray<SubMove>&                newCompositeSubMoves,
                    DBArray<PeriodicMove>&           newPeriodicMoves,
                    Unit&                            Summarization,
                    bool&                            CompleteSummarized){

   if(SM.arrayNumber==LINEAR){
       const Unit* Sum1;
       linearMoves.Get(SM.arrayIndex,Sum1);
       Summarization = (*Sum1);
       CompleteSummarized=true;
       return SM;   
   }
   if(SM.arrayNumber==PERIOD){
       const PeriodicMove* PM=NULL;
       periodicMoves.Get(SM.arrayIndex,PM);
       PeriodicMove PM2 = (*PM);
       SubMove SM2 = MinimizeRec(PM2.submove, 
                                 newLinearMoves,newCompositeMoves,
                                 newCompositeSubMoves,newPeriodicMoves,
                                 Summarization, CompleteSummarized);
       if(!CompleteSummarized){
          PM2.submove = SM2; 
          int Pos = newPeriodicMoves.Size();
          newPeriodicMoves.Append(PM2);
          SM2.arrayNumber=PERIOD;
          SM2.arrayIndex= Pos;
          CompleteSummarized = false;
          return SM2;
       }else{ // the result is a single linear move
          if(!Summarization.interval.CanAppended(
                               &Summarization.interval)){ // a little gap
              int LinPos = newLinearMoves.Size();
              newLinearMoves.Append(Summarization);
              PM2.submove.arrayNumber=LINEAR;
              PM2.submove.arrayIndex=LinPos;
              // store this periodic move
              int PerPos = newPeriodicMoves.Size();
              newPeriodicMoves.Append(PM2);
              SM2.arrayNumber=PERIOD;
              SM2.arrayIndex=PerPos;
              CompleteSummarized=false;
              return SM2;
          }else{ // we don't need longer this periodic move
             Summarization.interval.Mul(PM2.repeatations);
             return SM2;
         } 
       }
   }
   if(SM.arrayNumber==COMPOSITE){
        const CompositeMove* CMtmp;
        compositeMoves.Get(SM.arrayIndex,CMtmp);
        CompositeMove CM = *CMtmp;
        Unit LM(0);
        bool LMdefined = false;
        vector<SubMove> MySubMoves;
        for(int i=CM.minIndex;i<=CM.maxIndex;i++){
           const SubMove* Current;
           compositeSubMoves.Get(i,Current);
           SubMove SM2 = MinimizeRec(*Current,
                                     newLinearMoves,newCompositeMoves,
                                     newCompositeSubMoves,newPeriodicMoves,
                                     Summarization, CompleteSummarized);
           if(!CompleteSummarized){
              // store summarized submove if present
              if(LMdefined){
                 int LinPos = newLinearMoves.Size();
                 newLinearMoves.Append(LM);
                 LMdefined=false;
                 SubMove SM3;
                 SM3.arrayNumber=LINEAR;
                 SM3.arrayIndex=LinPos;
                 MySubMoves.push_back(SM3);
              } 
              MySubMoves.push_back(SM2); 
           }else{ // submove complete summarized
              if(!LMdefined){ // first summarized LinearMove;
                LM = Summarization;
                LMdefined=true;
              }else{
                  
                if(LM.CanSummarized(Summarization)){
                    // append the new summarization to LM
                    LM.interval.Append(&Summarization.interval);
                } else{
                    // the new summarization cannot appendend to LM
                    // store LM
                    int LinPos = newLinearMoves.Size();
                    newLinearMoves.Append(LM);
                    SubMove SM3;
                    SM3.arrayNumber=LINEAR;
                    SM3.arrayIndex=LinPos;
                    MySubMoves.push_back(SM3);
                    LM = Summarization; 
                }  
              }  
          }
        } // all submoves processed;
        if(MySubMoves.size()==0){ // all its collected into LM
            CompleteSummarized=true;
            Summarization = LM;
            return SM; // unimportant in this case
        }else{
            if(LMdefined){ // store the last summarization
              int LinPos = newLinearMoves.Size();
              newLinearMoves.Append(LM);
              SubMove SM3;
              SM3.arrayNumber = LINEAR;
              SM3.arrayIndex  = LinPos;
              MySubMoves.push_back(SM3); 
            }
            CM.minIndex = newCompositeSubMoves.Size();
            CM.maxIndex = CM.minIndex + MySubMoves.size()-1;
            // store the submoves
            for(unsigned int k=0;k<MySubMoves.size();k++){
               newCompositeSubMoves.Append(MySubMoves[k]);  
            }
            SubMove SM3;
            SM3.arrayNumber=COMPOSITE;
            SM3.arrayIndex=newCompositeMoves.Size();
            newCompositeMoves.Append(CM);
            CompleteSummarized=false;
            return SM3;
        }
   } 
   assert(false);
}



};

/*
2.3.9 The Class PMBool

This class is just an instantiation of the
PMSimple class.

*/

typedef PMSimple<bool,LinearBoolMove> PMBool;


/*
2.3.10 The class PMReal

This class is an instantiation of the 
PMSimple class.

*/

typedef PMSimple<double,MovingRealUnit> PMReal;
 


/*
2.3.11 The Class PMInt9M

This class is derived from an instatiation of the
PMSimple class. 

*/
class PMInt9M : public PMSimple<Int9M,LinearInt9MMove> {
public:

/*
~Constructor~

This is the defualt constructor making nothing.

*/
  PMInt9M() {}

/*
~Constructor~

This constructor should be used for creating a PMInt9M
instance. It calls the constructor of the superclass.

*/
  PMInt9M(int dummy):
     PMSimple<Int9M,LinearInt9MMove>(dummy) 
     {
           __TRACE__
     }

/*
~Transpose~

The Transpose function will change the roles of the arguments for which 
this periodic moving 9 intersection matrix is computed.

*/
  void Transpose(){
    int size = linearMoves.Size();
    const LinearInt9MMove* LM;
    LinearInt9MMove LM2;
    for(int i=0;i<size;i++){
       linearMoves.Get(i,LM);
       LM2 = (*LM); 
       LM2.Transpose();
       linearMoves.Put(i,LM2);
    } 

  }



/*
~CreateFrom~

When this function is called, the linearConstantMove is build from the
given values with an additional level between the tree and the linear moves.
This means, the value of this pmsimple is given by the structure showed in
figure [ref]{fig:RepTreeWithAdditionalLevel.eps}.

                Figure 3: Repetition Tree with an additional Level [RepTreeWithAdditionalLevel.eps]

This can be usedful in operation where the actual repetions remains but 
the units can be split. An example is the computation of the 
topological relationship between a periodic moving point and a non-moving
spatial object. The repetitions in the movement of the points are preserved
but it is possible that the topological relationship changes in single
units of this moving point. In this case, additional composite moves
must be inserted in the structure. This is exactly what this function does.

*/
bool CreateFrom( DBArray<LinearInt9MMove>& linearMoves, 
                 ArrayRange*                     level,
                 int                             levelsize,
                 DBArray<CompositeMove>&          compositeMoves,
                 DBArray<SubMove>&                compositeSubMoves,
                 DBArray<PeriodicMove>&           periodicMoves,
                 DateTime                        startTime,
                 SubMove                         submove) {
   __TRACE__

   defined =true;
   this->startTime.Equalize(&startTime);
   canDelete=false;
   this->submove.Equalize(&submove);
   const PeriodicMove* PM;
   const SubMove* SM;
   const CompositeMove* CM;
   const LinearInt9MMove* LM;
   switch(submove.arrayNumber){
     case PERIOD: { periodicMoves.Get(submove.arrayIndex,PM);
                    this->interval.Equalize(&(PM->interval));
                    break;
                   }
     case LINEAR: { linearMoves.Get(submove.arrayIndex,LM);
                    this->interval.Equalize(&(LM->interval));
                    break;
                  }
     case COMPOSITE: { compositeMoves.Get(submove.arrayIndex,CM);
                       this->interval.Equalize(&(CM->interval));
                       break;
                     }
     default:   assert(false);

   }
   this->linearMoves.Clear();
   if(linearMoves.Size()>0)
      this->linearMoves.Resize(linearMoves.Size());
   for(int i=0;i<linearMoves.Size();i++){
       linearMoves.Get(i,LM);
       LinearInt9MMove LM2 = (*LM);
       this->linearMoves.Put(i,LM2);  
   }
 
   this->compositeMoves.Clear();
   if(compositeMoves.Size()>0)
      this->compositeMoves.Resize(compositeMoves.Size());
   for(int i=0;i<compositeMoves.Size();i++){
        compositeMoves.Get(i,CM);
        CompositeMove CM2 = (*CM);
        this->compositeMoves.Put(i,CM2);
   }
   this->periodicMoves.Clear();
   if(periodicMoves.Size()>0)
      this->periodicMoves.Resize(periodicMoves.Size());
   for(int i=0;i<periodicMoves.Size();i++){
        periodicMoves.Get(i,PM);
        PeriodicMove PM2 = (*PM);
        this->periodicMoves.Put(i,PM2);
   }

   if(levelsize==linearMoves.Size()){// easy case: no additional Moves
        this->compositeSubMoves.Clear();
        if(compositeSubMoves.Size()>0)
            this->compositeSubMoves.Resize(compositeSubMoves.Size());
        for(int i=0;i<compositeSubMoves.Size();i++) {
            compositeSubMoves.Get(i,SM);
            SubMove SM2 = (*SM);
            this->compositeSubMoves.Put(i,SM2);
        }
        return true;
   }
   // we have to restructure the tree :-(
   // for the periodicMoves, we have to change the arrayindex of an 
   // linear submove or we have to build a new composite move
   // "pointers" to composite moves are not affected
   ArrayRange ar;
   int minsize = compositeSubMoves.Size()>0?compositeSubMoves.Size():1;
   this->compositeSubMoves.Resize(minsize); 

   // process the compositeMoves
   for(int i=0;i<compositeMoves.Size();i++){
      compositeMoves.Get(i,CM);
      int pos = this->compositeSubMoves.Size();
      int count = 0;
      for(int j=CM->minIndex;j<=CM->maxIndex;j++){
          compositeSubMoves.Get(j,SM);
          if(SM->arrayNumber!=LINEAR){ // copy the submove
             SubMove SM2 = (*SM);
             this->compositeSubMoves.Append(SM2);
             count++;
          } else{ // a linear submove
             ar = level[SM->arrayIndex];
             if(ar.minIndex==ar.maxIndex){
                SubMove SM2 = (*SM);
                SM2.arrayIndex=ar.minIndex;
                this->compositeSubMoves.Append(SM2);
                count++;
             } else{ // insert new submoves
               for(int k=ar.minIndex;k<=ar.maxIndex;k++){
                  SubMove SM2 = (*SM);
                  SM2.arrayNumber=LINEAR;
                  SM2.arrayIndex=k;
                  this->compositeSubMoves.Append(SM2);
                  count++;
               }
            }
          }
      }
      CompositeMove CM2 = (*CM);
      CM2.minIndex=pos;
      CM2.maxIndex=pos+count-1;
      this->compositeMoves.Put(i,CM2); 
   }

   // process the periodic moves 
   for(int i=0;i<this->periodicMoves.Size();i++){
        this->periodicMoves.Get(i,PM);
        PeriodicMove PM2 = (*PM);
        SubMove SM2 = PM2.submove;
        if(submove.arrayNumber==LINEAR){ // otherwise is nothing to do
          ar = level[SM2.arrayIndex];
          if(ar.minIndex==ar.maxIndex){ // ok, just correct the index
            SM2.arrayIndex=ar.minIndex;
          } else{ // create a new composite move
             // first create the appropriate submoves
             int pos = this->compositeSubMoves.Size(); 
             RelInterval i;
             for(int j=ar.minIndex;j<=ar.maxIndex;j++){
                SM2.arrayNumber = LINEAR;
                SM2.arrayIndex  = j;
                this->compositeSubMoves.Append(SM2);
                linearMoves.Get(j,LM);
                if(j==ar.minIndex){
                    i =  LM->interval;
                }else{
                    i.Append(&(LM->interval));
                }
             }
             CompositeMove CM2; 
             CM2.minIndex=pos;
             CM2.maxIndex=pos+ar.maxIndex-ar.minIndex+1;
             CM2.interval.Equalize(&i);
             this->compositeMoves.Append(CM2);
             PM2.submove.arrayNumber=COMPOSITE;
             PM2.submove.arrayIndex=this->compositeMoves.Size()-1;
         } 
         // write back the periodic move
         this->periodicMoves.Put(i,PM2); 
        }
   }
   return true;
}

};




/*
2.3.12 The Class PMPoint

This class represents a single periodic moving point.

*/
class PMPoint : public StandardAttribute {
  friend ostream& operator<< (ostream&, PMPoint);
  public:
     PMPoint() {} // never use this constructor
     /* normally we need an constructor without any
        arguments. but this constructor is used in a
        special way to cast a database object to a
        PMPoint. So i have decided to insert a constructor
        with dummy parameter
     */
     PMPoint(int dummy);
     ~PMPoint();
     void Destroy();
     void Equalize(const PMPoint* P2);
     int NumOfFLOBs();
     FLOB *GetFLOB(const int i);
     int Compare(const Attribute* arg) const;
     bool Adjacent(const Attribute*)const;
     PMPoint* Clone() const;
     int Sizeof()const;
     bool IsDefined() const;
     void SetDefined( bool defined );
     size_t HashValue() const;
     void CopyFrom(const StandardAttribute* arg);
     ListExpr ToListExpr(const bool typeincluded)const;
     bool ReadFrom(const ListExpr value);
     PMPoint* Intersection(const PInterval* interval);
     bool IsEmpty()const;
     Point* At(const DateTime* instant)const;
     Point* Initial()const;
     Point* Final();
     inline Points* Breakpoints();
     Points* Breakpoints(const DateTime* minDuration,const bool inclusive);
     CLine*  Trajectory();
     DateTime GetStart()const;
     DateTime GetEnd()const;
     PInterval GetInterval()const;
     PBBox GetBbox()const;
     MPoint Expand();
     void ReadFromMPoint(MPoint& P);
     PMInt9M* Toprel(const Point P); 
     PMInt9M* Toprel(Points& P);
     bool DistanceTo(const double x, const double y, PMReal& result)const;
     bool CheckCorrectness();

  private:
     DBArray<LinearPointMove> linearMoves;
     DBArray<SpatialCompositeMove> compositeMoves;
     DBArray<SubMove> compositeSubMoves;
     DBArray<SpatialPeriodicMove> periodicMoves;
     bool defined;
     bool canDelete;
     RelInterval interval;
     DateTime startTime;
     PBBox bbox;
     SubMove submove;
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

     void AppendUnits(MPoint& P, DateTime* Time, const SubMove S);
     int NumberOfExpandedUnits();
     int NumberOfExpandedUnits(const SubMove S);
     LinearPointMove GetLastUnit();
     bool FillFromRepTree(int& cpos, int& cspos, int& ppos, RepTree TR);

  };

/*
2.3.13 The Class PMPoints

This class represents a set of periodic moving points.

*/
class PMPoints : public StandardAttribute {
  friend ostream& operator<< (ostream& ,class PMPoints&) ;
  public:
     PMPoints() {}
     /* normally we need an constructor without any
        arguments. but this constructor is used in a
        special way to cast a database object to a
        PMPoint. So i have decided to insert a constructor
        with dummy parameter
     */
     PMPoints(int dummy);
     ~PMPoints();
     void Destroy();
     void Equalize(const  PMPoints* P2);
     int NumOfFLOBs();
     FLOB *GetFLOB(const int i);
     int Compare(const Attribute* arg) const;
     bool Adjacent(const Attribute*)const;
     PMPoints* Clone() const;
     int Sizeof()const;
     bool IsDefined() const;
     void SetDefined( bool defined );
     size_t HashValue() const;
     void CopyFrom(const StandardAttribute* arg);
     ListExpr ToListExpr(const bool typeincluded)const;
     bool ReadFrom(const ListExpr value);
     bool IsEmpty()const;
     inline Points* Breakpoints();
     Points* Breakpoints(const DateTime* duration,const bool inclusive);
     Points* At(const DateTime* T)const;
     Points* Initial()const{
         if(!defined)
            return NULL;
         return At(&startTime);
     }
     Points* Final(){
         if(!defined)
            return NULL;
         DateTime DT(startTime);
         DT.Add(interval.GetLength());
         return At(&DT); 
     }
 private:
     DBArray<LinearPointsMove> linearMoves;
     DBArray<TwoPoints> thePoints;
     DBArray<SpatialCompositeMove> compositeMoves;
     DBArray<SubMove> compositeSubMoves;
     DBArray<SpatialPeriodicMove> periodicMoves;
     bool defined;
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

  };


/*

2.4 Implementation of Aid Classes and Functions


2.4.1 Aid Classes

2.4.1.1 The class SimplePoint

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

    int compareTo(const SimplePoint P2)const{
       if(x<P2.x) return -1;
       if(x>P2.x) return 1;
       if(y<P2.y) return -1;
       if(y>P2.y) return 1;
       return 0;
    }
    bool operator< (const SimplePoint P2)const{
      return compareTo(P2)<0;
    }
    bool operator> (const SimplePoint P2)const{
      return compareTo(P2)>0;
    }
    bool operator== (const SimplePoint P2)const{
      return compareTo(P2)==0;
    }
    bool operator!= (const SimplePoint P2)const{
      return compareTo(P2)!=0;
    }
/*    SimplePoint operator= (SimplePoint P2){
       x = P2.x;
       y = P2.y;
       intinfo=P2.intinfo;
       boolinfo=P2.boolinfo;
       return *this;
    }*/
};


/*
2.4.2 A few Functions which can be useful for several classes

~About~

The ~About~ function checks whether the distance between the 
arguments is less than EPSILON. EPSILON is a macro defined at 
the begin of this file.

*/
bool About(const double a, const double b){
  __TRACE__
  double c = abs (a-b);
  return c < EPSILON;
}


/*
~Signum~

This function returns:
  
  * -1 if the argument is less than zero
  
  * 0 if the argument is zero
  
  * 1 if the argument is greater than zero

*/
int Signum(const double arg){
  __TRACE__
  if(arg<0) return -1;
  if(arg>0) return 1;
  return 0;
}


/*
~Distance~

The distance function computes the Euclidean distance between the
points defined by (Ax,Ay) and (Bx,By).

[3] O(1)

*/
double Distance(const double Ax, const double Ay,
                const double Bx, const double By){
   __TRACE__              
   return sqrt( (Ax-Bx)*(Ax-By)+(Ay-By)*(Ay-By));               
}                

/*
~SignedArea~

This function computes the signed area of the triangle defined
by (p,q,r). The result will be positive if r is located
left to the straight line defined by p,q. A negative result 
indicates that r is right to this line. If r is located 
on this line, the result will be zero.

[3] O(1)

*/
double SignedArea(const double Px, const double Py,
                  const double Qx, const double Qy,
                  const double Rx, const double Ry){
  __TRACE__      
  return ((Rx-Px)*(Ry+Py) + (Qx-Rx)*(Qy+Ry) + (Px-Qx)*(Py+Qy))/2;
}

/*
~PointOnSegment~

The function ~PointOnSegment~ checks whether the point (Px,Py)
is contained in the pointset of the segment ( (Ax,Ay)[->](Bx,By))

[3] O(1)

*/
bool PointOnSegment(const double Px, const double Py,
                    const double Ax, const double Ay,
                    const double Bx, const double By){

    /* 
    The point Px,Py is on the segment A->B   iff
    the distance beween A and B is equals to the sum
    of the distances between A, P  and P,B
    */               
    __TRACE__
    double DistAB = Distance(Ax,Ay,Bx,By);
    double DistAP = Distance(Ax,Ay,Px,Py);
    double DistPB = Distance(Px,Py,Bx,By);
    return About(DistAB,DistAP+DistPB);
}                    

/* 
~PointPosOnSegment~

This function computes, where the directed segment defined by s=(x1,y1,x2,y2)
is splitted by the point p=(x,y). The result is :

  * 0: if p is the startpoint of s

  * 1: if p is the endpoint of s

  * a value in 0..1 if s is splitted by p

  * a value outside of [0,1] if p is not on s 

*/

double PointPosOnSegment(double x1, double y1, 
                         double x2, double y2, 
                         double x, double y){

  if(x1==x2 && y1==y2){ // s is a point
     if(x1==x && y1==y)
        return 0;
     else
        return -1;  
  }
  // check wether p is on the line defined by s
  if(! ((x-x1)*(y2-y1)==(y-y1)*(x2-x1))){
       return -1;
  }
  if(x1==x2){ // special case vertical segment
     return (y-y1)/(y2-y1);
  }
  return (x-x1)/(x2-x1);
}



/*
~IsSpatialExtension~

This function checks whether the line defined by TP is
a extension of the segment defined by this. 

[3] O(1)

*/
bool TwoPoints::IsSpatialExtension(const TwoPoints* TP) const {
   __TRACE__
     if(endX!=TP->startX) return false;
      if(endY!=TP->startY) return false;
      double dx = endX-startX;
      double dy = endY-startY;
      double TPdx = TP->endX-TP->startX;
      double TPdy = TP->endY-TP->startY;
      return About(dy*TPdx,dx*TPdy); 
}

/*
~Speed~

This function computes the speed for a points moving from the
startpoint to the endpoint in the given interval. The speed is allways
greater than zero. This function will return -1 if an error is occurred
e.g. division by zero.

[3] O(1)

*/

double TwoPoints::Speed(const RelInterval interval)const {
   __TRACE__
  DateTime* D = interval.GetLength();
   double L = D->ToDouble();
   delete D; 
   D = NULL;
   if(L<0) return -1;
   double dx = endX-startX;
   double dy = endY-startY;
   double distance=sqrt(dx*dx+dy*dy);
   if(L==0 && distance!=0) return -1;
   if(distance==0) return 0;
   return distance/L;
}

/* 
~ReHeap~

This function performs a reheap of a given (partially) heap.
Is a support function for heapsort. This function places the
element at position in in __values__ at the right place 
according to the definition of a heap where the heap itself is
given by the range i ... k in the __values__ array.


[3]   log(k-i)


*/
template <typename T>
void reheap(T values[], int i, int k){
   __TRACE__
  int j,son;
   j = i;
   bool done = false; 
   while(!done){
     if( 2*j > k ){ // end of array reached
         done = true;
     }
     else{ if(2*j+1<=k){
               if(values[2*j-1]<values[2*j])
                   son = 2*j;
               else
                   son = 2*j+1;
           }else {
                son = 2*j;
           }
           if(values[j-1]>values[son-1]){
              // swap values
              T tmp = values[j-1];
              values[j-1]=values[son-1];
              values[son-1] = tmp;
              j = son;
           }else{ // final position reached
               done = true;
           }
     }
   } 
}
/*
~HeapSort~

This function sorts an array with elements supporting
comparisons. After calling this function the elements in
the array are sorted in decreasing order.

[3]   O(n log(n)) where n is the number of elements in the array.


*/
template <typename T> 
void heapsort(const int size, T values[]){
   __TRACE__
   int n = size;
    int i;
    for (i=n/2;i>=1;i--)
         reheap(values,i,n);
    for(i=n;i>=2;i--){
       T tmp = values[0];
       values[0] = values[i-1];
       values[i-1] = tmp;
       reheap(values,1,i-1);
    }
}

/*
~find~

This function finds an entry in an sorted array applying 
binary search. If nothing is found, -1 is returned otherwise
the first index of a matching element.

*/
template <typename T>
int find(const int size,const T field[],const T elem){
   __TRACE__
  int min=0;
   int max=size;
   bool found = false;
   while(min<max && !found){
     int mid = (min+max)/2;
     if(field[mid]>elem)
         max=mid-1;
     else if(field[mid]<elem)
         min=mid+1;
     else
         found=true;
   }
   if(!found) return -1;
   // At this point we know that mid is the index of one
   // element equals to *elem*. Now we have to find the 
   // smallest index.
   max = mid;
   while(max!=min){
     mid=(max+min)/2;
     if(field[mid]<elem)
        min==mid+1;
     else
        max=mid;
   } 
   return max;
}


/*
~GetNumeric~

Numbers have different representations in nested lists (IntAtom, RealAtom
or Rationals). All Classes should be have the possibility to read any
number format. To realize this, the function ~GetNumeric~ can be used.
The result is __true__, if LE represents a valid numeric value. This value
is stored in the argument __value__ as a double.

[3] O(1)

*/
static bool GetNumeric(const ListExpr List, double &value){
   __TRACE__
 ListExpr LE = List;
  int AT = nl->AtomType(LE);
  if(AT==IntType){
     value = (double)nl->IntValue(LE);
     return true;
  }
  if(AT==RealType){
     value = nl->RealValue(LE);
     return true;
  }
  // check for a rational
  int L = nl->ListLength(LE);
  if(L!=5 && L!=6)
     return false;
  if(!nl->IsEqual(nl->First(LE),"rat"))
     return false;

  LE = nl->Rest(LE); // read over the "rat"
  double sign=1.0;
  if(L==6){ // signum is included
    ListExpr SL = nl->First(LE);
    if(nl->IsEqual(SL,"+"))
      sign=1.0;
    else if(nl->IsEqual(SL,"-"))
      sign=-1.0;
    else
      return false;
    LE = nl->Rest(LE); // read over the signum
  }
  if(!nl->IsEqual(nl->Third(LE),"/"))
    return false;
  if ( (nl->AtomType(nl->First(LE))!=IntType) ||
       (nl->AtomType(nl->Second(LE))!=IntType) ||
       (nl->AtomType(nl->Fourth(LE))!=IntType))
       return false;
  double ip = nl->IntValue(nl->First(LE));
  double num = nl->IntValue(nl->Second(LE));
  double denom = nl->IntValue(nl->Fourth(LE));
  if(ip<0 || num<0 || denom<=0)
     return false;
  value = sign*(ip + num/denom);
  return true;
}


/*
~WriteListToStreamRec~

This function supports the WriteListToStream function.
It write a ListExpr given as argumnet __Lorig__ to __os__
with the given indent. If the list is corrupt, __error__
will be set to false.


*/

void WriteListToStreamRec(ostream &os, const ListExpr Lorig,const int indent,
                          bool &error){
   __TRACE__
 if(error) return;
  ListExpr L = Lorig;
  NodeType type= nl->AtomType(L);
  bool res;
  switch(type){
    case BoolType   : res = nl->BoolValue(L);
                      if(res)
              os << "TRUE";
          else
              os << "FALSE";
          break;
    case IntType    : os << (nl->IntValue(L));break;
    case RealType   : os << nl->RealValue(L);break;
    case SymbolType : os << nl->SymbolValue(L);break;
    case StringType : os << "\"" << nl->StringValue(L) << "\""; break;
    case TextType   : os << "<<A Text>>"; break;
    case NoAtom     : os << endl;
                      for(int i=0;i<indent;i++)
             os << " ";
          os << "(";
                      while(!nl->IsEmpty(L) && !error){
             WriteListToStreamRec(os,nl->First(L),indent+4,error);
       os << " ";
       L = nl->Rest(L);
          }   
          os << ")";
          break;
    default : os << "unknkow AtomType"; error=true;
  }
}


/*
~WriteListExprToStream~

This function writes a ListExpr given by __L__ to __os__.


*/
void WriteListExprToStream(ostream &os, const ListExpr L){
   __TRACE__
  bool error = false;
   WriteListToStreamRec(os,L,0,error);
   if(error) os << "The given List was corrupt";
}

/*
~Shift Operator for Bounding Boxes~

This operator overloads the the shift operator for an
output stream. It can be used for easy writing a such
box to __cout__, __cerr__ or whatever.


*/
ostream& operator<< (ostream &os, const PBBox BB){
   __TRACE__
 os << "PBBox[";
  if(!BB.defined)
    os << "undefined]";
  else if (BB.isEmpty)
    os << "empty]";
  else{
    os << "(" << BB.minX << "," << BB.minY << ")";
    os << "->";
    os << "(" << BB.maxX << "," << BB.maxY << ")";
  }
  return os;  
}

/*
~Shift operator for RelInterval~

Overloads the shift operator for easy outputing a
relative interval.

*/
ostream& operator<< (ostream& os, const RelInterval I){
   __TRACE__
  os << I.ToString();
   return os;
}

/*
~Shift operator for the TwoPoints Type~

Overloads the shift operator for easy output of 
a TwoPoints instance.

*/
ostream& operator<< (ostream& os, const TwoPoints TP){
   __TRACE__
 os << "TP[(" << TP.startX << "," << TP.startY << ") ->(";
  os << TP.endX << "," << TP.endY << ")]";
  return os;
}

/*
~Shift operator for the PInterval Type~

Overloads the shift operator for easy output of 
a PInterval instance.

*/
ostream& operator<< (ostream& os, const PInterval I){
   __TRACE__
  os << I.ToString();
   return os;
}

/*
~Shift operator for the CompositeMove Type~

Overloads the shift operator for easy output of 
a CompositeMove instance.

*/
ostream& operator<< (ostream& os, const CompositeMove CM){
   __TRACE__
  os << "CM[" << CM.minIndex << "," << CM.maxIndex << "..";
  os << CM.interval << "]";
   return os;
}

/*
~Shift operator for the SpatialCompositeMove Type~

Overloads the shift operator for easy output of 
a SpatialCompositeMove instance.

*/
ostream& operator<< (ostream& os, const SpatialCompositeMove SCM){
   __TRACE__
  os << "CM[" << SCM.minIndex << "," << SCM.maxIndex << "..";
   os << SCM.interval << ".." << SCM.bbox << "]";
   return os;
}

/*
~Shift operator for the SubMove Type~

Overloads the shift operator for easy output of 
a SubMove instance.

*/
ostream& operator<< (ostream& os, const SubMove SM){
   __TRACE__
 switch(SM.arrayNumber){
    case LINEAR    : os << "SM_Linear["; break;
    case COMPOSITE : os << "SM_COMPOSITE[";break;
    case PERIOD    : os << "SM_PERIODIC[";break;
    default        : os << "SM_UNKNOWN[";
  }
  os << SM.arrayIndex << "]";
  return os;
}

/*
~Shift operator for the PeriodicMove Type~

Overloads the shift operator for easy output of 
a PeriodicMove instance.

*/
ostream& operator<< (ostream& os, const PeriodicMove PM){
   __TRACE__
  os << "R["<< PM.repeatations <<"](" << PM.submove <<")";
  return os;
}

/*
~Shift operator for the LinearPointsMove Type~

Overloads the shift operator for easy output of 
a LinearPointsMove instance.

*/
ostream& operator<< (ostream& os, const LinearPointsMove LM){
   __TRACE__
  os << "LPsM[" ;
   os << LM.startIndex << "," << LM.endIndex << "]";
   // add also bbox, interval isstatic and so on if required !
   return os;
}

/*
~Shift operator for the PMPoints Type~

Overloads the shift operator for easy output of 
a PMPoints instance.

*/
ostream& operator<< (ostream &os, class PMPoints &P){
   __TRACE__
 os << " <<<< PMPoints >>>>" << endl;
  if(!P.defined){
     os << "undefined" << endl;
     return os;
  }   
  os << "starttime: " << P.startTime.ToString() << endl;
  os << P.submove << endl;
  os << "defined :" << P.defined << endl;
  // the contents of the contained arrays
  // the linear moves
  os << "linear Moves " << P.linearMoves.Size() << endl;
  const LinearPointsMove* LM1;
  const TwoPoints* TP1;
  unsigned int thePointsSize = P.thePoints.Size();
  os << "SizeOf thePoints: " << thePointsSize << endl;
  for(int i=0;i<P.linearMoves.Size();i++){
     P.linearMoves.Get(i,LM1);
     LinearPointsMove LM2 = (*LM1);
     os << LM2 << endl;
     os << "Content of this linear Move " << endl;
     for(int unsigned j=LM2.startIndex;j<LM2.endIndex;j++){
   if(j>=thePointsSize){
       os << "Array out of bounds " << __POS__ << endl;
       os << "Try to access element number " << j << "in an array ";
       os << "of size " << thePointsSize << endl;
   }else {
       os << j << " ." ;
       P.thePoints.Get(j,TP1);
       TwoPoints TP2 = (*TP1);
       os << TP2 << endl;
       os << j << " ." ;
       P.thePoints.Get(j,TP1);
       TP2 = (*TP1);
       os << TP2 << endl;
       os << j << " ." ;
       P.thePoints.Get(j,TP1);
       TP2 = (*TP1);
       os << TP2 << endl;
  }     
     }
     os << "end content of this linear Move " << endl;
  }
 // os << " please extend the << operator for pmpoints values " << endl;
  os << " <<<< end PMPoints >>> " << endl; 
  return os;
}



/*
3 Implementation of the classes of this Algebra


3.1 The Implementation of the Class Interval

~Constructor~

The familiar empty constructor.

[3] O(1)

*/
PInterval::PInterval(){}

/*
~Constructor~

[3] O(1)

*/
PInterval::PInterval(int dummy){
   __TRACE__
  relinterval=RelInterval(dummy);
   startTime = DateTime(instanttype);
}

/*
~Constructor~

[3] O(1)

*/
PInterval::PInterval(const DateTime startTime, const RelInterval relinterval){
   __TRACE__
  (this->startTime).Equalize(&startTime);
   (this->relinterval).Equalize(&relinterval);
}

/*
~Append~

If possible, the argument make this interval longer. D2 has to be
of type duration.

[3] O(1)

*/
bool PInterval::Append(const RelInterval* D2){
   __TRACE__
  return relinterval.Append(D2);
}

/*
~CanAppended~

This function checks whether D2 can be used to make longer
this interval.

[3] O(1)

*/
bool PInterval::CanAppended(const RelInterval* D2)const{
   __TRACE__
   return relinterval.CanAppended(D2);
}

/*
~Contains~

This function checks wether the instant T is is contained in this
interval.

[3] O(1)

*/
bool PInterval::Contains(const DateTime* T)const{
   __TRACE__
 DateTime tmp = (*T) - startTime;
  return relinterval.Contains(&tmp);
}

/*
~Contains~

This function checks wether the interval I is is contained in this
interval.

[3] O(1)

*/
bool PInterval::Contains(const PInterval* I)const{
   __TRACE__
  if(I->startTime<startTime)
      return false;
   DateTime* thisEnd = GetEnd();
   DateTime* IEnd = I->GetEnd();
   bool res = true;
   if(IEnd > thisEnd){
      res = false;
   }
   else {
      if(I->startTime == startTime){
         if(!IsLeftClosed() && I->IsLeftClosed())
            res = false;
      }
      if(thisEnd == IEnd){
         if(!IsRightClosed() && I->IsRightClosed())
            res = false;
      }
   }
   delete thisEnd;
   thisEnd = NULL;
   delete IEnd;
   IEnd = NULL;
   return res;
}

/*
~Intersects~

This functions checks whether this and I share a common
instant.

[3] O(1)

*/
bool PInterval::Intersects(const PInterval* I)const{
   __TRACE__
  // two intervals intersects if one of the
   // four endpoints is contained in the
   bool res = true;
   DateTime* thisEnd = GetEnd();
   DateTime* IEnd = I->GetEnd();
   if((*IEnd) < startTime){
      res = false;
   } else if(I->startTime> (*thisEnd)){
      res = false;
   } else if((*(IEnd)) == startTime){
       res = (I->IsRightClosed()) && IsLeftClosed();
   } else if(I->startTime == (*(thisEnd))){
       res = (I->IsLeftClosed()) && IsRightClosed();
   }
   delete thisEnd;
   delete IEnd;
   thisEnd = NULL;
   IEnd = NULL;
   return res;
}

/*
~Clone~

Returns a copy of this.

[3] O(1)

*/
PInterval* PInterval::Clone() const{
   __TRACE__
  PInterval* clone = new PInterval(1);
   clone->Equalize(this);
   return clone;
}



/*
~CompareTo~

This functions compares this Interval with D2.

[3] O(1)

*/
int PInterval::CompareTo(const PInterval* D2)const{
   __TRACE__
  int cmp;
   cmp = startTime.CompareTo(&(D2->startTime));
   if(cmp!=0)
     return cmp;
   return relinterval.CompareTo(&(D2->relinterval));
}
/*
~Compare~

The ~Compare~ function compares this Interval with an Attribute.
The argument has to be of type Interval.

[3] O(1)

*/
int PInterval::Compare(const Attribute* arg)const{
   __TRACE__
  return CompareTo( (PInterval*) arg);
}

/*
~IsDefined~

[3] O(1)

*/
bool PInterval::IsDefined() const{
   __TRACE__
  return startTime.IsDefined() && relinterval.IsDefined();
}

/*
~SetDefined~

[3] O(1)

*/
void PInterval::SetDefined(bool defined){
   __TRACE__
   startTime.SetDefined(defined);
    relinterval.SetDefined(defined);
}

/*
~HashValue~

Computes a HashValue for this.

[3] O(1)

*/
size_t PInterval::HashValue() const{
   __TRACE__
  return startTime.HashValue()+relinterval.HashValue();
}

/*
~CopyFrom~

When this function is called, this Interval takes its value
from the argument.

[3] O(1)

*/
void PInterval::CopyFrom(const StandardAttribute* arg){
   __TRACE__
  Equalize( (PInterval*) arg);
}

/*
~Equalize~

When this function is called, this Interval takes its value
from the argument.

[3] O(1)

*/
void PInterval::Equalize(const PInterval* D2){
   __TRACE__
   startTime.Equalize(&(D2->startTime));
    relinterval.Equalize(&(D2->relinterval));
}

/*
~GetLength~

Returns the length of this interval as a duration.

[3] O(1)

*/
DateTime* PInterval::GetLength()const{
   __TRACE__
  return relinterval.GetLength();
}

/*
~GetStart~

Returns a clone of the StartTime.

[3] O(1)

*/
DateTime* PInterval::GetStart()const{
   __TRACE__
  DateTime* res = new DateTime(instanttype);
   res->Equalize(&startTime);
   return res;
}

/*
~GetEnd~

Returns the end time of this interval.

[3] O(1)

*/
DateTime* PInterval::GetEnd()const{
   __TRACE__
  DateTime* L = relinterval.GetLength();
   DateTime T = startTime + (*L);
   delete L;
   L = NULL;
   DateTime* res = new DateTime(instanttype);
   res->Equalize(&T);
   return res;
}

/*
~ToListExpr~

Returns the list representing this interval.
This requires, that this interval is bounded.

[3] O(1)

*/
ListExpr PInterval::ToListExpr(const bool typeincluded)const {
   __TRACE__
  DateTime* EndTime = GetEnd();
   ListExpr result = nl->FourElemList(
                            startTime.ToListExpr(true),
                            EndTime->ToListExpr(true),
                            nl->BoolAtom(IsLeftClosed()),
                            nl->BoolAtom(IsRightClosed()));
   delete EndTime;
   EndTime = NULL;
   return result;
}

/*
~IsLeftClosed~

[3] O(1)

*/
bool PInterval::IsLeftClosed()const {
   __TRACE__
  return relinterval.IsLeftClosed();
}


/*
~IsRightClosed~

[3] O(1)

*/
bool PInterval::IsRightClosed()const{
   __TRACE__
  return relinterval.IsRightClosed();
}

/*
~ReadFrom~

Reads the value of this interval from LE. typeincluded specified if
LE is in format (type value)  or LE only represents the value list.

[3] O(1)

*/
bool PInterval::ReadFrom(const ListExpr LE, const bool typeincluded){
   __TRACE__
  ListExpr value;
   if(typeincluded){
      if(nl->ListLength(LE)!=2)
         return false;
      if(nl->IsEqual(nl->First(LE),"pinterval"))
         return false;
      value = nl->Second(LE);
   } else
       value = LE;
   if(nl->ListLength(value)!=4)
      return false;
   bool lc,rc;
   if( nl->AtomType(nl->Third(value))!=BoolType ||
       nl->AtomType(nl->Fourth(value))!=BoolType)
       return false;
   DateTime start,end;
   if(!start.ReadFrom(nl->First(value),true))
      return false;
   if(!end.ReadFrom(nl->Second(value),true))
      return false;
   lc = nl->BoolValue(nl->Third(value));
   rc = nl->BoolValue(nl->Fourth(value));
   if( (start==end) && !( rc && lc)) // a single instant has to be closed
      return false;
   if( start > end)  // start has to be before end
      return false;
   DateTime duration = end-start;
   Set(&start,&duration,lc,rc);
   return true;
}

/*
~Set~

Sets this interval to  the given values.

[3] O(1)

*/
bool PInterval::Set(const DateTime* startTime, const DateTime* length,
                    const bool leftClosed, const bool rightClosed){
   __TRACE__
   if (!(this->relinterval).Set(length,leftClosed,rightClosed))
       return false;
    (this->startTime).Equalize(startTime);
    return true;
}



/*
~SetLength~

Sets a new length for this interval.

[3] O(1)

*/
bool PInterval::SetLength(const DateTime* T){
   __TRACE__
   return relinterval.SetLength(T);
}

/*
~ToString~

This function computes a string representation of this interval.

[3] o(1)

*/
string PInterval::ToString()const {
   __TRACE__
  stringstream ss;
   if(IsLeftClosed())
      ss << "[";
   else
      ss << "]";
   ss << startTime.ToString();
   ss << ",";
   DateTime* end = GetEnd();
   ss << end->ToString();
   delete end;
   end = NULL;
   if(IsRightClosed())
      ss << "]";
   else
      ss << "[";
  return ss.str();
}


/*
3.3 Implementation of the __LinearPointMove__ class

~Constructor~

This constructor creates a new LinearPointMOve ignoring the argument.

[3] O(1)

*/
LinearPointMove::LinearPointMove(int dummy){
   __TRACE__
 interval=RelInterval(1);
  bbox = PBBox(1);
  isStatic=true;
  defined=true;
}

/*
~LinearPointMove::ToListExpr~

Converts a linear point unit into its nested list representation.

[3] O(1)

*/
ListExpr LinearPointMove::ToListExpr()const{
   __TRACE__
   if(defined)
        return nl->TwoElemList(
                      nl->SymbolAtom("linear"),
                      nl->ThreeElemList(
                            interval.ToListExpr(true),
                            nl->TwoElemList(
                                 nl->RealAtom((float)startX),
                                 nl->RealAtom((float)startY)),
                            nl->TwoElemList(
                                 nl->RealAtom((float)endX),
                                 nl->RealAtom((float)endY))));
    else
       return nl->TwoElemList(
                      nl->SymbolAtom("linear"),
                      nl->TwoElemList(
                                 interval.ToListExpr(true),
                                 nl->SymbolAtom("undefined")));
   }

/*
~LinearPointMove::ReadFrom~

Reads the content of this LinearPointMove from value.
The return values indicates the success.

[3] O(1)

*/
bool LinearPointMove::ReadFrom(const ListExpr value){
   __TRACE__
      ListExpr V = value;
       int L = nl->ListLength(V);
       if(L<2 || L>3)
          return false;
       if(!interval.ReadFrom(nl->First(V),true))
          return false;

       if(L==2){ // potentially an undefined move
           if(nl->IsEqual(nl->Second(V),"undefined")){
               defined = false;
               bbox.SetUndefined();
               isStatic = true;
               return true;
           }
           return false;
       }
       // L == 3
      ListExpr SP = nl->Second(V);
      ListExpr EP = nl->Third(V);
      double dv;
      if(!GetNumeric(nl->First(SP),dv)){
        if(DEBUG_MODE){
          cerr << __POS__ << "StartX is not a number" << endl;
        }
        return false;
      }
      startX=dv;
      if(!GetNumeric(nl->Second(SP),dv)){
        if(DEBUG_MODE){
          cerr << __POS__ << "StartY is not a number" << endl;
        }
        return false;
      }
      startY=dv;
      if(!GetNumeric(nl->First(EP),dv)){
        if(DEBUG_MODE){
           cerr << __POS__ << "EndX is not a number" << endl;
        }
        return false;
      }
      endX=dv;
      if(!GetNumeric(nl->Second(EP),dv)){
         if(DEBUG_MODE){
           cerr << __POS__ << "EndY is not a number" << endl;
         }
         return false;
      }
      endY= dv;
      bbox.SetUndefined();
      bbox.Union(startX,startY);
      bbox.Union(endX,endY);
      defined=true;
      isStatic= (startX==endX) && (startY==endY);
      return true;
}

/*
~At~

Returns the position of this linear moving point at the given time.
It's required, that the time value is contained in the interval of this
linear moving point.

[3] O(1)

*/
Point* LinearPointMove::At(const DateTime* duration) const{
   __TRACE__
  assert(interval.Contains(duration));
   assert(defined);
   double delta = interval.Where(duration);
   double x = startX+delta*(endX-startX);
   double y = startY+delta*(endY-startY);
   return new Point(true,x,y);
}

/*
~IsDefinedAt~

~IsDefinedAt~ checks whether this LinearPointMOve is defined
at the specified time.

[3] O(1)

*/
bool LinearPointMove::IsDefinedAt(const DateTime* duration)const{
   __TRACE__
     return interval.Contains(duration) && defined;
}

/*
~GetHalfSegment~

This function returns the trajectory of this LinearPointMove as an
CHalfSegment.  If the LinearPointMove is not defined or static,
an undefined HalfSegment is returned.

[3] O(1)

*/
CHalfSegment LinearPointMove::GetHalfSegment(
                       const bool LeftDominatingPoint)const{
   __TRACE__
  if(defined && !isStatic){
      Point P1(true,startX,startY);
      Point P2(true,endX,endY);
      return CHalfSegment(true,LeftDominatingPoint,P1,P2);
   } else{
      return CHalfSegment(false);
   }
}

/*
~Intersects~

This function checks wether the trajectory of this linear
point move intersects __window__. This check is finer than the
check for intersection of the bounding boxes but can also
computed in constant time. Note that the constant time can not
ensured when another structures (like lines, points or regions)
are involved.

[3] O(1)

*/
bool LinearPointMove::Intersects(const PBBox* window)const{
   __TRACE__
    if(!bbox.Intersects(window))
        return false;
      // Now, the position of all vertices of window related to the
      // segment defined by this moving point is computed .
      // This is done by using the formula of the directed area of the
      // triangles. We use only the signum of this area.
      double A,r1,r2;
      bool isLeft;
      double p1 = startX;
      double p2 = startY;
      double q1 = endX;
      double q2 = endY;
      window->GetVertex(0,r1,r2);
      A = (r1-p1)*(r2+p2)+(q1-r1)*(q2+r2)+(p1-q1)*(p2+q2);
      if(A==0)
         return true;
      isLeft=A<0;
      for(int i=1;i<4;i++){
        window->GetVertex(i,r1,r2);
        A = (r1-p1)*(r2+p2)+(q1-r1)*(q2+r2)+(p1-q1)*(p2+q2);
        if(A==0) return true; // a point on the segment is found
        if( (A<0)!=isLeft) // a point on the other side 
                           // of the segment is found
           return true;
      }
      return false;
}


/*
~Split~

This function splits a moving point unit into two parts. The value of delta
has to be in the interval [0,1]. The return value states whether a rest exists.
The first part is the this instance the second part is returned by the argument
Rest. If holds delta==0 and the relative interval of this moving point is not
leftclosed, no rest exists and the result will be false. The same will be when
delta==1  and not rightclosed. This function can only used, when the interval is
bounded. But this is no restriction because a linear moving point can not have
any unbounded interval. If the closeFirst flag is true, the first part will be
rightClosed (if any rest) and the second part (rest) will be left-open. In the
other case the values are negated.

[3] O(1)

*/
bool LinearPointMove::Split(const double delta, const bool closeFirst,
                            LinearPointMove& Rest){
   __TRACE__
   assert((delta>=0) && (delta<=1));
    RelInterval inter = RelInterval(0);
    if(!interval.Split(delta,closeFirst,inter))
       return false;

    Rest.Equalize(this);
    Rest.interval.Equalize(&inter);
    double SplitX = startX+delta*(endX-startX);
    double SplitY = startY+delta*(endY-startY);
    endX = SplitX;
    endY = SplitY;
    Rest.startX = SplitX;
    Rest.startY = SplitY;
    return true;
}


/*
~SplitX~

Splits this unit into two ones at the horizontal line x==X.
If the splitLine is outside of the trajectory of this unit, nothing is
changed and false is returned.

[3] O(1)

*/
bool LinearPointMove::SplitX(const double X, const bool closeFirst, 
                             LinearPointMove& Rest){
   __TRACE__
 if(startX==endX)
     return false;
  // start and endX are on the same side of the splitline
  if( ((X-startX)<0) && ((X-endX)<0))
     return false;
  if( ((X-startX)>0) && ((X-endX)>0))
     return false;

  double delta = (X - startX)/(endX-startX);
  return Split(delta,closeFirst,Rest);
}

/*
~SplitY~

Splits this unit into two ones at the vertical line y==Y.
If the splitLine is outside of the trajectory of this unit, nothing is
changed and false is returned.

[3] O(1)

*/
bool LinearPointMove::SplitY(const double Y, const bool closeFirst, 
                             LinearPointMove& Rest){
   __TRACE__
 if(startY==endY)
     return false;
  // start and endX are on the same side of the splitline
  if( ((Y-startY)<0) && ((Y-endY)<0))
     return false;
  if( ((Y-startY)>0) && ((Y-endY)>0))
     return false;
  double delta = (Y - startY)/(endY-startY);
  return Split(delta,closeFirst,Rest);
}

/*
~SetUndefined~

This function makes this instance of a LinearPointMove undefined.

*/
void LinearPointMove::SetUndefined(){
   __TRACE__
     defined = false;
      isStatic=true;
      bbox.SetEmpty();
}


/*
~CanBeExtendedBy~

This function yields true iff the Intervals can summarized,
The end point of this is the startpoint pf P2 and direction and
speed are equals.

[3] O(1)

*/
bool LinearPointMove::CanBeExtendedBy(const LinearPointMove* P2)const {
   __TRACE__
 // check the intervals
  if(!interval.CanAppended(&(P2->interval)))
    return false;
  // check the combinations of defined
  if(!defined && !P2->defined)
    return true;
  if(defined && !P2->defined)
    return false;
  if(!defined && P2->defined)
    return false;
  // check Start and Endpoint
  if(endX!=P2->startX)
    return false;
  if(endY!=P2->startY)
    return false;
  if(isStatic)
     return P2->isStatic;
  // check direction
    if(abs ( ((endX-startX)*(P2->endY-P2->startY))-
             ((P2->endX-P2->startX)*(endY-startY)))>EPSILON)
        return false;
  // check speed
  double L = sqrt( (startX-endX)*(startX-endX) + (startY-endY)*(startY-endY));
  double P2L = sqrt( (P2->startX-P2->endX)*(P2->startX-P2->endX) +
                     (P2->startY-P2->endY)*(P2->startY-P2->endY));
  DateTime* IL = interval.GetLength();
  double T = IL->ToDouble();
  delete IL;
  IL = NULL;
  DateTime* IL2 = ((P2->interval).GetLength());
  double P2T = IL2->ToDouble();
  delete IL2;
  IL2 = NULL;
  if( abs(L*P2T - P2L*T)>EPSILON)
     return false;
  return true;
}

/*
~ExtendWith~

This function concatenates the linear point moves if possible. If not, the
result will be false.

[3] O(1)

*/
bool LinearPointMove::ExtendWith(const LinearPointMove* P2){
   __TRACE__
  if(!CanBeExtendedBy(P2))
      return false;
   interval.Append(&(P2->interval));
   endX = P2->endX;
   endY = P2->endY;
   return true;
}

/*
~Equalize~

This LinearPointMove takes its value from the argument if this
function is called.

[3] O(1)

*/

void LinearPointMove::Equalize(const LinearPointMove* P2){
   __TRACE__
 interval.Equalize(&(P2->interval));
  bbox.Equalize(&(P2->bbox));
  startX = P2->startX;
  startY = P2->startY;
  endX = P2->endX;
  endY = P2->endY;
  isStatic = P2->isStatic;
  defined = P2->defined;
}


/*
~CompareTo~

This functions implements the familiar compare function.

*/
   int LinearPointMove::CompareTo(LinearPointMove* LPM){
       int comp = interval.CompareTo(&(LPM->interval));  
       if(!defined && !LPM->defined)
          return comp;
       if(!defined && LPM->defined)
          return -1;
       if(defined && !LPM->defined)
          return  1;
       if(comp!=0) return comp; // different intervals
       if(startX < LPM->startX) return -1;
       if(startX > LPM->startX) return 1;
       if(startY < LPM->startY) return -1;
       if(startY > LPM->startY) return 1;
       if(endX < LPM->endX) return -1;
       if(endX > LPM->endX) return 1;
       if(endY < LPM->endY) return -1;
       if(endY > LPM->endY) return 1;
       return 0;
   }


/*
~ToString~

This function returns a string representation of this LinearPointsMove

*/
string LinearPointMove::ToString()const{
  ostringstream res;
  res << "(" << interval.ToString();
  if(!defined){
     res << " undefined)";
  } else {
     res << " (" << startX << ", " << startY <<") -> (";
     res << endX << ", " << endY << "))";
  }
  return res.str();
}

/*
~Toprel~

This function computes the evolution of the topological relationship
between this LinearPointMove and the given static point. The caller has
to allocate enough memory in the resultbuffer, this means that the
result array has a minimum size of 3. The return value is the number of 
used array slots (1..3). The resulting LinearInt9M values covers the
same relative interval as this point it does. 

[3] O(1)

*/
int LinearPointMove::Toprel(const Point P, LinearInt9MMove* result)const{
  // first handle undefined values
  Int9M R;
  if(!defined && !P.IsDefined()){
     R.Set(false,false,false,false,false,false,false,false,true);
     result[0].Set(R,interval);
     return 1;
  }
  if(!defined){
     R.Set(false,false,false,false,false,false,true,false,true);
     result[0].Set(R,interval);
     return 1;
  }
  if(!P.IsDefined()){
     R.Set(false,false,true,false,false,false,false,false,true);
     result[0].Set(R,interval);
     return 1;
  }

  // The point describes a vertical line in the three dimensional space
  // the linear moving point describes an arbitrary segment in this space
  // Because the coordinates of the time dimension is equal for both lines,
  // we can reduce the problem of the problem whether a point is located 
  // on a segment.
  double dx = endX-startX;
  double dy = endY-startY; 
  double x = P.GetX();
  double y = P.GetY();
  // two simple points, which are the case here, can only be equal
  // or disjoint; first we define the corresponding matrices
  Int9M Requal;
  Requal.Set(true,false,false,false,false,false,false,false,true);
  Int9M Rdisjoint;
  Rdisjoint.Set(false,false,true,false,false,false,true,false,true);
  // special case: moving point is static
  if((dx==0 && dy==0 ) || interval.GetLength()->ToDouble()==0){
     if( (x==startX) && (y==startY))
        result[0].Set(Requal,interval);
     else
        result[0].Set(Rdisjoint,interval);
     return 1; 
  }
  
  // at this point we know that this point is moving
  // we can conclude that in at most one timepoint the 
  // points are equal and they are disjoint in all other times 
  
  // point, we need a closed interval of length 0
  DateTime DT(durationtype);
  DT.ReadFrom(0.0);
  RelInterval EmptyInt;
  EmptyInt.Set(&DT,true,true);

  // special case: P is startpoint of this
  if( (x==startX) && (y==startY)){
     if(!interval.IsLeftClosed()){
         result[0].Set(Rdisjoint,interval);
         return 1;
     }
     else{
         result[0].Set(Requal,EmptyInt);
         RelInterval FI;
         FI.Set(interval.GetLength(),false,interval.IsRightClosed());
         result[1].Set(Rdisjoint,FI);        
         return 2; 
     }
  }  
  // special case: P is endpoint of this
  if( (x==endX) && (y==endY)){
    if(!interval.IsRightClosed()){
         result[0].Set(Rdisjoint,interval);
         return 1;
    }else{
       RelInterval I;
       I.Set(interval.GetLength(),interval.IsLeftClosed(),false);
       result[0].Set(Rdisjoint,I);
       result[1].Set(Requal,EmptyInt);
       return 2;
    }
  }

  // check wether P is on the segment 
  if( (x-startX)*dy == (y-startY)*dx){ // on the line ? 
     // one of dx and dy is different to 0.0 because this case is
     // aready handled above
   
     double delta = dx==0?(y-startY)/dy:(x-startX)/dx;
     if(delta<0 || delta>1){ // not on the segment
         result[0].Set(Rdisjoint,interval);
         return 1;
     }
     RelInterval I1;
     RelInterval I3;
     I1.Equalize(&interval);
     I1.Split(delta,true,I3);
     I1.Set(I1.GetLength(),I1.IsLeftClosed(),false);
     DateTime ST(instanttype);
     result[0].Set(Rdisjoint,I1);
     result[1].Set(Requal,EmptyInt);
     result[2].Set(Rdisjoint,I3);
     return 3;
  }else{ //point not on the segment
     result[0].Set(Rdisjoint,interval);
     return 1;
 }
} 

/*
~Toprel~

This function computes the moving topological relationship between this
LinearPointMove and a set of points given by __P__. The number of 
resulting LinearInt9MMove's depends linearly on the number of the 
points contained in __P__. Because the size of the result is not 
constant, we use a vector for storing it.  

*/
void LinearPointMove::Toprel(Points& P, vector<LinearInt9MMove>& Result)const{
   Result.clear();
   Int9M R;
   if(!defined && P.IsEmpty()){
       R.Set(false,false,false,false,false,false,false,false,true);
       LinearInt9MMove res1(0);
       res1.Set(R,interval);
       Result.push_back(res1);
       return;
   }
   if(!defined && !P.IsEmpty()){
       R.Set(false,false,false,false,false,false,true,false,true);
       LinearInt9MMove res1(0);
       res1.Set(R,interval);
       Result.push_back(res1);
       return;
   }
   // the pmpoint is defined
   if(P.IsEmpty()){
       R.Set(false,false,true,false,false,false,false,false,true);
       LinearInt9MMove res1(0);
       res1.Set(R,interval);
       Result.push_back(res1);
       return;
   }
   // the pmpoint and the points value contain elements
   assert(P.IsOrdered());
   /* If the points value contains more than one point, the 
      9 intersection matrix will contain an 1 entry at position
      interior exterior at each position. In the other case, it will
      have an 1 entry either at this position or at the position 
      interior/interior
   */
   R.Set(false,false,false,false,false,false,false,false,true);
   // special case, the points value contains a single point
   int size = P.Size();
   if(size>1){
      R.SetEI(true);
   } else{
     LinearInt9MMove buffer[3];
     const Point* tP1;
     Point tP2;
     P.Get(0,tP1);
     tP2 = (*tP1);
     int n = Toprel(tP2,buffer);
     for(int i=0;i<n;i++)
        Result.push_back(buffer[i]);
     return;
   }

   // special case: point is staying
   if((startX==endX && startY == endY) || 
       interval.GetLength()->ToDouble()==0){
        Point tmpP(startX,startY);
        if(P.Contains(tmpP))
             R.SetII(true);  
        LinearInt9MMove r1(1);
        r1.Set(R,interval);
        Result.push_back(r1);
        return;
   }
   // we can scan all point in a direction to get the split point ordered
   // first, we have to determine the direction;
   
  // we create an interval of length zero
   DateTime DT(durationtype);
   DT.ReadFrom(0.0);
   RelInterval emptyInterval;
   emptyInterval.Set(&DT,true,true);

   bool forward;
   if(startX<endX)
      forward=true;
   else if(startX==endX)
      forward = startY<endY;
   else // startX>endX
      forward = false;
      

   const Point* currentPoint1;
   Point currentPoint2;
   RelInterval lastInterval = interval;
   RelInterval newInterval;
   double lastX1 = startX;
   double lastY1 = startY;
   double currentDelta=0;
   bool done = false;
   double currentX, currentY;
   LinearInt9MMove currentMove;
   for(int i=0;i<size && !done;i++){
      if(forward){
          P.Get(i,currentPoint1);
          currentPoint2 = (*currentPoint1);
      }
      else{
          P.Get(size-i-1,currentPoint1);
          currentPoint2 = (*currentPoint1);
      }
   
      currentX = currentPoint2.GetX();
       currentY = currentPoint2.GetY();
       currentDelta=PointPosOnSegment(lastX1,lastY1,endX,endY,
                                      currentX,currentY); 
       if(currentDelta==0){ // split at starting point
         // split the interval at the begin 
         if(lastInterval.IsLeftClosed()){
            R.SetII(true); 
            currentMove.Set(R,emptyInterval);
            Result.push_back(currentMove);
            lastInterval.Set(lastInterval.GetLength(),false,
                             lastInterval.IsRightClosed());
         }
       }  else if(currentDelta>0 && currentDelta<1){ // proper split
             R.SetII(false);
            lastInterval.Split(currentDelta,false,newInterval);
            currentMove.Set(R,lastInterval);
            Result.push_back(currentMove);
            R.SetII(true);
            currentMove.Set(R,emptyInterval);
            Result.push_back(currentMove);
            newInterval.Set(newInterval.GetLength(),false,
                            newInterval.IsRightClosed());
            lastInterval=newInterval;
            lastX1 = currentX;
            lastY1 = currentY;
       } else if(currentDelta==1){ // split at endpoint
            if(lastInterval.IsRightClosed()){
               R.SetII(false);
               lastInterval.Set(lastInterval.GetLength(),
                                lastInterval.IsLeftClosed(),false);
               currentMove.Set(R,lastInterval);
               Result.push_back(currentMove);
               R.SetII(true);
               currentMove.Set(R,emptyInterval);
               Result.push_back(currentMove);  
            } else{
               R.SetII(false);
               currentMove.Set(R,lastInterval);
               Result.push_back(currentMove);
            }
            done = true; 
      } else if(currentDelta>1){ // no more splitpoints are possible
           R.SetII(false);
           currentMove.Set(R,lastInterval);
           Result.push_back(currentMove);
           done=true;
       }   
    }
   if(!done) { // there is an unprocessed rest from the interval
       R.SetII(false);
       currentMove.Set(R,lastInterval);
       Result.push_back(currentMove);
   }
}

/*
~DistanceTo~

This function computes the distance of this LinearPointMove to 
a fixed point. The result will be of type MovingRealUnit.If this 
unit is not defined, also the reault will be not defined. In each 
case, the interval of the result will be the same like the interval
of this unit.

*/

   void LinearPointMove::DistanceTo(const double x, const double y, MovingRealUnit& result)const{
      if(!defined){
        result.GetFrom(0,0,interval);
        result.SetDefined(false);
        return;
      }
      double startDist = sqrt( (startX-x)*(startX-x)+(startY-y)*(startY-y));
      double endDist = sqrt( (endX-x)*(endX-x)+(endY-y)*(endY-y));
      result.GetFrom(startDist,endDist,interval);
   }





/*
3.4 The TwoPoints Class


*/

/*
~Constructor~

This constructor sets the starting point to (xs,ys) and the
endpoint to (xe,ye).

[3] O(1)

*/
TwoPoints::TwoPoints(const double xs, const double ys,
                     const double xe, const double ye){
  __TRACE__
  startX = xs;
  startY = ys;
  endX = xe;
  endY = ye;
}

/*
~InnerIntersects~

This function checks wether the segments defined by the
start and endpoint have any intersection which is not an
endpoint of one of the  segments. For overlapping segments,
the result will also __false__.

[3] O(1)

*/
bool TwoPoints::InnerIntersects(const TwoPoints& TP) const{
   __TRACE__
 // first we check the cases of segments degenerated to points
  if( IsStatic() && TP.IsStatic()){
     return false;
  }
  if(IsStatic()){
    // check for endpoint
    if(((startX==TP.startX) && (startY==TP.startY)) ||
       ((startX==TP.endX) && (startY==TP.endY)))
       return false;     
    return PointOnSegment(startX,startY,TP.startX,TP.startY,TP.endX,TP.endY);
  }
  if(TP.IsStatic()){
    if(((startX==TP.startX) && (startY==TP.startY)) ||
         ((endX==TP.startX) && (endY==TP.startY)))
      return false;     
    return PointOnSegment(TP.startX,TP.startY,startX,startY,endX,endY);      
  }
  // At this point we handle with two non-degenerated segments
  int S1 = Signum(SignedArea(startX,startY,endX,endY,TP.startX,TP.startY));
  int S2 = Signum(SignedArea(startX,startY,endX,endY,TP.endX,TP.endY));
  if(S1==S2) return false;
  if(S1==0 || S2==0) return false; // endpoint
  S1 = Signum(SignedArea(TP.startX,TP.startY,TP.endX,TP.endY,startX,startY));
  S2 = Signum(SignedArea(TP.startX,TP.startY,TP.endX,TP.endY,endX,endY));
  return (S1!=S2) && (S1!=0) && (S2!=0);
}

/*
~IsStatic~

This function checks whether the start and the endpoint are
equal.

[3] O(1)

*/
bool TwoPoints::IsStatic() const{
   __TRACE__
 return startX==endX &&
         startY==endY;
}



/*
3.5 The class LinearPointsMove

~Constructor~

This Constructor creates an undefined instance of this class.

[3] O(1)

*/
LinearPointsMove::LinearPointsMove(int dummy):interval(0),bbox(0){
  __TRACE__
  startIndex=0;
  endIndex=0;
  isStatic = true;
  defined = false;
}
  
/*
~ToListEpr~

This function returns the nested list corresponding to this 
LinearPointsMove. Because this class can't manage the location
of the points, the array containing the points is given as an
argument. If the array not contains enough entries, the result 
will be zero.

[3] O(P) where P is the number of contained moving points

*/  
ListExpr LinearPointsMove::ToListExpr(const DBArray<TwoPoints> &Points) const{
   __TRACE__
   if(!defined)
      return nl->SymbolAtom("undefined");
   const TwoPoints* Entry;
   unsigned int s = Points.Size();
   if(s<endIndex){
      if(DEBUG_MODE){
        cerr << "Error in creating the nested list for";
        cerr << "a LinearPointsMove " << endl;
        cerr << "Endindex (" << endIndex << ") ";
        cerr << "is located after the last ArrayElement (";
        cerr << Points.Size() << ")" << endl;
      }
      return 0;
   }   
   if(endIndex<=startIndex){
      cerr << __POS__ << "endIndex < startIndex " << endl;  
      return 0;
   }


   ListExpr starts, ends;
   ListExpr Ps,Pe,LastStart,LastEnd;

   Points.Get(startIndex,Entry);   
   Ps = nl->TwoElemList( nl->RealAtom(Entry->startX),
                         nl->RealAtom(Entry->startY));
   Pe = nl->TwoElemList( nl->RealAtom(Entry->endX),
                         nl->RealAtom(Entry->endY));
   starts = nl->OneElemList(Ps);
   ends = nl->OneElemList(Pe);
   LastStart = starts;
   LastEnd = ends;

   for(unsigned int i=startIndex+1;i<endIndex;i++){
      Points.Get(i,Entry);
      Ps = nl->TwoElemList( nl->RealAtom(Entry->startX),
                            nl->RealAtom(Entry->startY));
      Pe = nl->TwoElemList( nl->RealAtom(Entry->endX),
                            nl->RealAtom(Entry->endY));
      LastStart = nl->Append(LastStart,Ps);
      LastEnd = nl->Append(LastEnd,Pe); 
   }

   ListExpr res = nl->TwoElemList(nl->SymbolAtom("linear"),
                         nl->ThreeElemList( interval.ToListExpr(true),
                                     starts,ends));        
   return res;             
}
   
/*
~ReadFrom~

This function reads the value of this LinearPointsMove from 
value. The points are appended to the Points array beginning at
the given index. If value don't represent a valid LinearPointsMove, 
the return value will be false. Nevertheless it may be that the Points
array is changed. After calling this function the index value will be 
points to the elements after the last inserted one.
If an error is occured this instance will be undefined.

[3] O(P) where P is the number of contained moving points

*/
bool LinearPointsMove::ReadFrom(const ListExpr value, DBArray<TwoPoints> &Pts,
                                int &Index){
   __TRACE__
// value has to be a list
 if(nl->AtomType(value)!=NoAtom){
   if(DEBUG_MODE){
     cerr << "List for LinearPointsMove is an atom " << endl;
   }
   defined = false;
   return false;
 }  
 // check the ListLength
 if(nl->ListLength(value)!=3){ // (interval starts ends)
    if(DEBUG_MODE){
      cerr << "Wrong listlength for LinearPointsMove; should be 3, is ";
      cerr << nl->ListLength(value) << endl; 
    }
    defined = false;
    return false;
 }   
 // read the Interval
 if(!interval.ReadFrom(nl->First(value),true)){
    if(DEBUG_MODE){
      cerr << "LinearPointsMove::ReadFFrom: error in reding interval" << endl;
    }
    defined = false;
    return false;
 }   
 ListExpr starts = nl->Second(value);
 ListExpr ends   = nl->Third(value);
 int L1 = nl->ListLength(starts);
 if(L1 != nl->ListLength(ends)){
   if(DEBUG_MODE){
     cerr << "Error in LinearPointsMove::ReadFrom" << endl;
     cerr << "Different lengths for start and end points " << endl;
   }
   return false;
 }   
 if(L1==0){
    isStatic = true;
    startIndex = 0;
    endIndex = 0;
    defined = true;
    bbox.SetEmpty();
    return  true;
 }
 
 double xs,ys,xe,ye;
 TwoPoints TP(0,0,0,0);
 ListExpr Start,End;
 startIndex = Index;
 endIndex = Index+L1;
 while(! nl->IsEmpty(starts)){
    Start = nl->First(starts);
    End = nl->First(ends);
    if(!nl->AtomType(Start)==NoAtom  ||
       !nl->AtomType(End)==NoAtom){
       if(DEBUG_MODE){
          cerr << "Error in LinearPointsMove::ReadFrom" << endl;
          cerr << "start or end is not a list" << endl;
       }
       defined = false;
       return false;
    }   
    if(nl->ListLength(Start)!=2 ||
       nl->ListLength(End)!=2) {
       if(DEBUG_MODE){
          cerr << "Error in LinearPointsMove::ReadFrom" << endl;
          cerr << "Wrong listlength for Start or for End " << endl;
    cerr << "Expected 2 for both, received " << nl->ListLength(Start);
    cerr << "(start) and "<<nl->ListLength(End) << "(end)" << endl;
       }
       defined = false;  
       return false;
    }   
    if(!GetNumeric(nl->First(Start),xs) ||
       !GetNumeric(nl->Second(Start),ys) ||
       !GetNumeric(nl->First(End),xe) ||
       !GetNumeric(nl->Second(End),ye)){
       if(DEBUG_MODE){
          cerr << "Error in LinearPointsMove::ReadFrom" << endl;
          cerr << "Cannot find numerical value " << endl;
       }
       defined = false;
       return false;
    }
    
    TP = TwoPoints(xs,ys,xe,ye);
    bbox.Union(xe,ye);
    bbox.Union(xs,ys);
    if(xe!=xs || ye!=ys)
       isStatic=false;
    Pts.Put(Index,TP);        
    Index++;
    starts=nl->Rest(starts);
    ends=nl->Rest(ends);
 }     
 
 defined = true;
 return true;
}


/*
~At~

This function computes the locations of the contained points
at the given point of time. If the duration is outside of the relative
interval of this LinearPointsMove, NULL is returned.

[3]  O(P log(P)) where P is the number of contained moving points.
     The reason is the sorting of the points in the result. I'm not
     sure whether this time is reached because i don't know the 
     implementation of the sorting of the points.

*/
Points* LinearPointsMove::At(const DateTime* duration,
                         const DBArray<TwoPoints> &Pts) const{
   __TRACE__
  if(!interval.Contains(duration))
      return 0;
   double w = interval.Where(duration);   
   Points* Result = new Points(Pts.Size());
   const TwoPoints* Element;
   Point aPoint;
   double x,y;
   Result->StartBulkLoad();
   for(unsigned int i=startIndex;i<endIndex;i++){
      Pts.Get(i,Element);
      x = Element->startX + w * (Element->endX - Element->startX);
      y = Element->startY + w * (Element->endY - Element->startY); 
      aPoint = Point(x,y);
      Result->InsertPt(aPoint); 
   }  
   Result->EndBulkLoad();
   return  Result;
}
   
/*
~IsDefinedAt~

The function ~IsDefinedAt~ checks whether the given duration is
part of the relative interval of this LinearPointsMove. This means,
if the duration is zero, the relative interval has to be leftclosed.
If the given duration is less than zero, false is returned. Otherwise
this function checks wether the duration is less or equlas to the duration
of the relative interval. In the case of equality, the relative interval
has to be rightclosed to get the result true. 

[3] O(1)

*/ 
bool LinearPointsMove::IsDefinedAt(const DateTime* duration)const{
   __TRACE__
  return interval.Contains(duration);
}

/*
~Intersects~

This function checks whether the bounding box of this
LinearPointsMove has common points with the bounding box which is
given as an arguments.

[3] O(1)

*/
bool LinearPointsMove::ProhablyIntersects(const PBBox* window)const {
   __TRACE__
  return bbox.Intersects(window);
}

/*
  ~SetUndefined~

This function sets this LinearPointsMove to be undefined.

[3] O(1)

*/
void LinearPointsMove::SetUndefined(){
   __TRACE__
  defined = false;
}


/*
~CanBeExtendedBy~

This function checks whether this LinearPointsmove can be appended
by the one of the arguments to yielding a single LinearPointsMove.

[3] unknow because not implemented up to now

*/
bool LinearPointsMove::CanBeExtendedBy(const LinearPointsMove* P2,
                                    DBArray<TwoPoints> &MyPoints,
                                    DBArray<TwoPoints> &PointsOfP2) const{
   __TRACE__
    if(!interval.CanAppended(&(P2->interval)))
        return false;
     // First we check for defined
     if(!defined && !P2->defined)
        return true;
     if(defined != P2->defined)
        return false;
     // now both LinearPointsMove are defined
     // check for the same number of contained points
     if( (endIndex-startIndex) != (P2->endIndex-P2->startIndex))
         return false;
     // now we have to check wether each endpoint of this LinearPointsMove
     // has a matching start point in P2.
     // To do this, we insert all endpoints of this into an array and
     // all start points of P2 to another one. After sorting both arrays
     // we have to check the equality of this ones.
     int size = endIndex-startIndex;
     SimplePoint endPoints[size];
     SimplePoint startPoints[size];
     const TwoPoints* CurrentPoint;
     for(unsigned int i=startIndex;i<endIndex;i++){
        MyPoints.Get(i,CurrentPoint);
        endPoints[i-startIndex].x=CurrentPoint->endX;
        endPoints[i-startIndex].y=CurrentPoint->endY;
        endPoints[i-startIndex].intinfo=i;
        endPoints[i-startIndex].boolinfo=false;
     }
     for(unsigned int i=P2->startIndex;i<P2->endIndex;i++){
        PointsOfP2.Get(i,CurrentPoint);
        startPoints[i-P2->startIndex].x=CurrentPoint->startX;
        startPoints[i-P2->startIndex].y=CurrentPoint->startY;
        startPoints[i-P2->startIndex].intinfo=i;
        startPoints[i-P2->startIndex].boolinfo=false;
     }
     heapsort(size,endPoints);
     heapsort(size,startPoints);
     for(int i=0;i<size;i++){
         if(endPoints[i] != startPoints[i])
            return false;
     }
     // we check for same speed and direction of the points
     bool found = false; // found a matching (extending) upoint
     int pos,cpos;
     pos=cpos=0;
     int endIndex,startIndex;
     const TwoPoints* TestPoint;
     bool morethanone;
     for(int i=0;i<size;i++){
        endIndex=endPoints[i].intinfo;
        startIndex=startPoints[pos].intinfo;
        MyPoints.Get(endIndex,CurrentPoint);
        // check all Points of P2 with the same startpoint like the current
        // endpoint
        if(pos<size-1)
           morethanone=startPoints[pos]==startPoints[pos+1];
        else
           morethanone=false;
        cpos = pos; 
        while(!found && cpos<size){
            if(endPoints[i]!=startPoints[cpos]) return false; // nothing found
            if(startPoints[cpos].boolinfo) // point allready used
                 cpos++;
            else{
                PointsOfP2.Get(cpos,TestPoint);
                if(CurrentPoint->IsSpatialExtension(TestPoint)){ 
                   // check for same speed
                   if(CurrentPoint->Speed(interval)==
                      TestPoint->Speed(P2->interval)){ //mathc found
                         startPoints[cpos].boolinfo=true; // mark as used
                         found = true;
                   }
                }else{
                   cpos++; // try the next
                }
            } 
        } 
        if(!found) return false; 
        if(!morethanone) pos++; 
     }

     return true; 
}
  
/*
~ExtendsWith~

This function summarized two LinearPointsMove together to a
single one if possible. The return values indicates the
success of this function. The result shows whether the
LinearPointsMoves can be put together. If the result is
__false__, this LinearPointsMove is not changed.

*/ 
bool LinearPointsMove::ExtendWith(const LinearPointsMove* P2,
                                  DBArray<TwoPoints> &MyPoints,
                                  DBArray<TwoPoints> &PointsOfP2){
   __TRACE__
 if(!CanBeExtendedBy(P2,MyPoints,PointsOfP2)) return false;
  interval.Append(&(P2->interval));
  bbox.Union(&(P2->bbox));
  // this is very closed to the CanBeExtededFunction but here we need no
  // checks because we know that this units can be putted together.
  cout << __POS__ << endl;
  cout << "ExtendWith(...) notimplemented " << endl;
  return false;
}

/*
~Equalize~

This functions makes this equal to the argument.

[3] O(1)

*/
void LinearPointsMove::Equalize(const LinearPointsMove* P2){
   __TRACE__
  interval.Equalize(&(P2->interval));
   bbox.Equalize(&(P2->bbox));
   startIndex=P2->startIndex;
   endIndex=P2->endIndex;
   isStatic = P2->isStatic;
   defined = P2->defined;
}


/*

3.6 The Implementation of the Class PBBox

*/

/*
~Constructor~

The standard constructor for special use;

[3] O(1)

*/
PBBox::PBBox(){
   __TRACE__
}

/*
~Constructor~

This constructor creates an undefined bounding box.

[3] O(1)

*/
PBBox::PBBox(int dummy){
   __TRACE__
   defined = true;
   isEmpty = true;
}

/*
~Constructor~

This constructor creates a new bounding box
containing only the point (x,y).

[3] O(1)

*/
PBBox::PBBox(const double x, const double y){
   __TRACE__
  defined=true;
  isEmpty=false;
  minX=maxX=x;
  minY=maxY=y;
}

/*
~Constructor~

This constructor creates a bounding box from the
given rectangle.

[3] O(1)

*/
PBBox::PBBox(const double minX, const double minY, 
             const double maxX, const double maxY){
   __TRACE__
  this->minX = minX<maxX?minX:maxX;
  this->maxX = minX<maxX?maxX:minX;
  this->minY = minY<maxY?minY:maxY;
  this->maxY = minY<maxY?maxY:minY;
  defined = true;
  isEmpty = false;
}




/*
~Compare~

This function compares this PBBox with  __arg__.

[3] O(1)

*/
int PBBox::Compare(const Attribute* arg) const{
    __TRACE__
  return CompareTo((PBBox*) arg);
}

/*
~Adjacent~

Because a bounding box is defined in the Euclidean Plane, the
adjacent function cannot be implemented. Hence the return value is
allways false;

[3] O(1)

*/
bool PBBox::Adjacent(const Attribute*)const{
    __TRACE__
 return false;
}


/*
~Sizeof~

This function returns the size of the PBBox class.

[3] O[1]

*/
int PBBox::Sizeof()const{
    __TRACE__
  return sizeof(PBBox);
}

/*
~IsDefined~

This function returns true if this PBBox is in a defined state.

[3] O(1)

*/
bool PBBox::IsDefined() const{
    __TRACE__
 return defined;
}

/*
~IsEmpty~

This function returns whether this bounding box don't contains
any point of the Euclidean Plane.

[3] O(1)

*/
bool PBBox::IsEmpty() const{
    __TRACE__
  return isEmpty;
}


/*
~SetDefined~

The ~SetDefined~ function sets the defined state of this bounding box.
Because the internal values of this box can have invalid value, this
function will only have an effect if the argument is false.

[3] O(1)

*/
void PBBox::SetDefined( bool defined ){
    __TRACE__
 if(!defined)
    this->defined = defined;
}

/*
~HashValue~

This fuction returns a Hash-Value of this bounding box.

[3] O(1)

*/
size_t PBBox::HashValue() const{
    __TRACE__
  if(!defined) return (size_t) 0;
  if(isEmpty) return (size_t) 1;
   return (size_t)  abs(minX + maxX*(maxY-minY));
}


/*
~CopyFrom~

If this function is called the bounding box takes its value
from the given argument.

[3] O(1)

*/
void PBBox::CopyFrom(const StandardAttribute* arg){
    __TRACE__
  Equalize((PBBox*) arg);
}

/*
~ToListExpr~

This function translates this bounding box to its nested list representation.

[3] O(1)

*/
ListExpr PBBox::ToListExpr()const{
    __TRACE__
   if(!defined)
       return nl->SymbolAtom("undefined");
    if(isEmpty)
       return nl->SymbolAtom("empty");
    return nl->FourElemList(nl->RealAtom(minX),
                            nl->RealAtom(minY),
                            nl->RealAtom(maxX),
                            nl->RealAtom(maxY));
}

/*
~ReadFrom~

This function reads the value of this bounding box from the given
ListExpr.

[3] O(1)

*/
bool PBBox::ReadFrom(const ListExpr LE){
    __TRACE__
 if(nl->IsEqual(LE,"undefined")){
      defined = false;
      isEmpty=true;
      return true;
  }
  if(nl->IsEqual(LE,"empty")){
     defined = true;
     isEmpty = true;
     return true;
  }
  if(nl->ListLength(LE)!=4)
    return false;

  double x1,x2,y1,y2;
  if(!GetNumeric(nl->First(LE),x1)) return false;
  if(!GetNumeric(nl->Second(LE),y1)) return false;
  if(!GetNumeric(nl->Third(LE),x2)) return false;
  if(!GetNumeric(nl->Fourth(LE),y2)) return false;
  minX = x1<x2?x1:x2;
  maxX = x1<x2?x2:x1;
  minY = y1<y2?y1:y2;
  maxY = y1<y2?y2:y1;
  isEmpty=false;
  defined =true;
  return true;
}


/*
~CompareTo~

The ~CompareTo~ function compares two bounding boxes
lexicographically (order minX maxX minY maxY).
An undefined Bounding box is less than a defined one.

[3] O(1)

*/
int PBBox::CompareTo(const PBBox* B2)const {
    __TRACE__
 if(!defined&&!B2->defined) return 0;
  if(!defined&&B2->defined) return -1;
  if(defined&&!B2->defined) return 1;
  // Now holds that this and B2 are defined
  if(isEmpty&&B2->isEmpty) return 0;
  if(isEmpty&&!B2->isEmpty) return -1;
  if(!isEmpty&&B2->isEmpty) return 1;
  // both boxes are not empty
  if(minX<B2->minX) return -1;
  if(minX>B2->minX) return 1;
  if(maxX<B2->maxX) return -1;
  if(maxX>B2->maxX) return 1;
  if(minY<B2->minY) return -1;
  if(minY>B2->minY) return 1;
  if(maxY<B2->maxY) return -1;
  if(maxY>B2->maxY) return 1;
  return 0;
}

/*
~Contains~

This function checks whether the point defined by (x,y) is
contained in this bounding box.

[3] O(1)

*/
bool PBBox::Contains(const double x,const double y)const{
    __TRACE__
 if(!defined) return false;
  if(isEmpty) return false;
  return x>=minX && x<=maxX && y>=minY && y<=maxY;
}


/*
~Contains~

The following function checks whether B2 is contained in this PBBox.

[3] O(1)

*/
bool PBBox::Contains(const PBBox* B2)const {
    __TRACE__
  if(!defined) return false;
   if(isEmpty) return B2->isEmpty;
   return Contains(B2->minX,B2->minY) && Contains(B2->maxX,B2->maxY);
}

/*
~Clone~

Returns a clone of this;

[3] O(1)

*/
PBBox* PBBox::Clone() const{
    __TRACE__
 PBBox* res = new PBBox(minX,minY,maxX,maxY);
  res->defined = defined;
  res->isEmpty = isEmpty;
  return res;
}

/*
~Equalize~

The ~Equalize~ function changed this instance to have the same value
as B2.

[3] O(1)

*/
void PBBox::Equalize(const PBBox* B2){
    __TRACE__
  minX = B2->minX;
   maxX = B2->maxX;
   minY = B2->minY;
   maxY = B2->maxY;
   defined = B2->defined;
   isEmpty = B2->isEmpty;
}

/*
~Intersection~

This function computes the intersection beween this instance and
B2. If both boxes are disjoint the result will be undefined.

[3] O(1)

*/
void PBBox::Intersection(const PBBox* B2){
    __TRACE__
 if(!defined) return;
  if(!B2->defined){
    defined=false;
    return;
  }
  if(isEmpty) return;
  if(B2->isEmpty){
     isEmpty=true;
     return;
  }
  minX = minX>B2->minX? minX : B2->minX;
  maxX = maxX<B2->maxX? maxX : B2->maxX;
  minY = minY>B2->minY? minY : B2->minY;
  maxY = maxY<B2->maxY? maxY : B2->maxY;
  isEmpty = minX<=maxX && minY<=maxY;
}

/*
~Intersects~

~Intersects~ checks whether this and B2 share any common point.

[3] O(1)

*/
bool PBBox::Intersects(const PBBox* B2)const{
    __TRACE__
 if(!defined || !B2->defined) return false;
  if(isEmpty || B2->isEmpty) return false;
  if(minX>B2->maxX) return false; //right of B2
  if(maxX<B2->minX) return false; //left of B2
  if(minY>B2->maxY) return false; //over B2
  if(maxY<B2->minY) return false; //under B2
  return true;
}

/*
~Size~

This function returns the size of the box. If this box is
undefined -1.0 is returned. Otherwise the result will be a
non-negative double number. Note that a empty box and a box
containing a single point will yield the same result.

[3] O(1)

*/
double PBBox::Size()const {
    __TRACE__
 if(!defined) return -1;
  if(isEmpty) return 0;
  return (maxX-minX)*(maxY-minY);
}

/*
~Union~

The function ~Union~ computes the bounding box which contains
both this bounding box as well as B2.

[3] O(1)

*/
void PBBox::Union(const PBBox* B2){
    __TRACE__
 if(!B2->defined){ // operators are only allowed on defined arguments
     defined = false;
     return;
  }
  if(!defined) return; // see above

  if(B2->isEmpty) return; // no change

  // this and B2 are defined and not empty
  if(isEmpty){
      Equalize(B2);
      return;
  }
  minX = minX<B2->minX? minX: B2->minX;
  maxX = maxX>B2->maxX? maxX: B2->maxX;
  minY = minY<B2->minY? minY: B2->minY;
  maxY = maxY>B2->maxY? maxY: B2->maxY;
}

/*
~Union~

This variant of the ~Union~ functions extends this bounding
box to contain the point defined by (x,y)

[3] O(1)

*/
void PBBox::Union(const double x, const double y){
    __TRACE__
if(!defined)return;
 if(isEmpty){
    isEmpty=false;
    minX=maxX=x;
    minY=maxY=y;
    return;
 }
 else{ // really extend this if needed
    minX = minX<x? minX : x;
    maxX = maxX>x? maxX : x;
    minY = minY<y? minY : y;
    maxY = maxY>y? maxY : y;
 }
}

/*
~SetUndefined~

The ~SetUndefined~ function sets the state of this BBox to be
undefined.

[3] O(1)

*/
void PBBox::SetUndefined(){
    __TRACE__
  defined=false;
}

/*
~GetVertex~

This functions returns a vertex of this box.

  * __No__=0: left bottom

  * __No__=1: right bottom

  * __No__=2: left top

  * __No__=3: right top

  * otherwise: the return value is false, x,y remains unchanged

[3] O(1)

*/
bool PBBox::GetVertex(const int No, double& x, double& y)const{
    __TRACE__
  if(!defined) return false;
   if(isEmpty) return false;

   if(No==0){
     x = minX;
     y = minY;
     return true;
   }
   if(No==1){
     x=maxX;
     y=minY;
     return true;
   }
   if(No==2){
     x=minX;
     y=maxY;
     return true;
   }
   if(No==3){
     x=maxX;
     y=maxY;
     return true;
   }
   if(DEBUG_MODE){
     cerr << "PBBox::GetVertex called with wrong value :" << No << endl;
   }
   return  false;
}

void PBBox::SetEmpty(){
   isEmpty = true;
   defined = true;
}

/*

3.7 The implementation of the class __RelInterval__

~Constructor~

The special constructor for the cast function.

[3] O(1)

*/
RelInterval::RelInterval(){   
   __TRACE__
}

/*
~Constructor~

This constructor creates a defined single instant with length 0.

[3] O(1)

*/
RelInterval::RelInterval(int dummy){
    __TRACE__
   length = DateTime(durationtype);
   leftClosed=true;
   rightClosed=true;
   defined=true;
   canDelete=false;
}

/*
~A private constructor~

Creates a RelInterval from the given values. Is private because
the values can leads to a invalid state of this RelInterval.

[3] O(1)

*/
RelInterval::RelInterval(const DateTime* length, const bool leftClosed,
                         const bool rightClosed){
    __TRACE__
 DateTime Zero= DateTime(durationtype);
  int comp=length->CompareTo(&Zero);
  assert(comp>=0);
  assert(comp>0 || (leftClosed && rightClosed));
  this->leftClosed=leftClosed;
  this->rightClosed=rightClosed;
  this->length.Equalize(length);
}

/*
~Append~

This function appends D2 to this RelInterval if possible.
When D2 can't appended to this false is returned.

[3] O(1)

*/
 bool RelInterval::Append(const RelInterval* R2){
    __TRACE__
   if(!CanAppended(R2))
       return false;
    length.Add(&(R2->length));
    rightClosed = R2->rightClosed;
    return true;
 }

/*
~CanAppended~

This function checks whether D2 can appended to this RelInterval.

[3] O(1)

*/
bool RelInterval::CanAppended(const RelInterval* D2)const {
    __TRACE__
  if(rightClosed)
     return ! D2->leftClosed;
  else
     return D2->leftClosed; 
}

/*
~Contains~

Checks whether T is contained in this RelInterval 

[3] O(1)

*/
bool RelInterval::Contains(const DateTime* T)const {
    __TRACE__
  DateTime Zero=DateTime(durationtype);
   int compz = T->CompareTo(&Zero);
   if(compz<0)
     return false;
   if(compz==0)
     return leftClosed;
   int compe = T->CompareTo(&length);
   if(compe<0)
      return true;
   if(compe==0)
      return rightClosed;
   return false;
}

/*
~Mul~

This functions extends this relinterval to be factor[mul]oldlength.

[3] O(1)

*/
void RelInterval::Mul(const long factor){
    __TRACE__
  length.Mul(factor);
}

/*
~Clone~

This function creates a new RelInterval with the same value as
the this object.

[3] O(1)

*/
RelInterval* RelInterval::Clone() const{
    __TRACE__
  RelInterval* Copy = new RelInterval(1);
   Copy->Equalize(this);
   return Copy;
}


/*
~CompareTo~

This function compares two RelIntervals.

[3] O(1)

*/
int RelInterval::CompareTo(const RelInterval* D2)const {
    __TRACE__
 if(!defined && !D2->defined) return 0;
  if(!defined && D2->defined) return -1;
  if(defined && !D2->defined) return 1;
  // at this point both involved intervals are defined
  if(leftClosed && !D2->leftClosed) return -1;
  if(!leftClosed && D2->leftClosed) return 1;
  int tc = length.CompareTo(&(D2->length));
  if(tc!=0) return tc;
  if(!rightClosed && D2->rightClosed) return -1;
  if(rightClosed && !D2->rightClosed) return 1;
  return 0;
}

/*
~Compare~

This functions compares an attribute with this relinterval.

[3] O(1)

*/
int RelInterval::Compare(const Attribute* arg) const{
    __TRACE__
  RelInterval* D2 = (RelInterval*) arg;
   return CompareTo(D2);
}

/*
~CopyFrom~

This function take the value for this relinterval from the
given argument.

[3] O(1)

*/
void RelInterval::CopyFrom(const StandardAttribute* arg){
    __TRACE__
  Equalize((RelInterval*)arg);
}

/*
~HashValue~

[3] O(1)

*/
size_t RelInterval::HashValue() const{
    __TRACE__
  size_t lhv = length.HashValue();
  if(leftClosed) lhv = lhv +1;
  if(rightClosed) lhv = lhv +1;
  return lhv;
}


/*
~Split~

Splits an interval at the specified position. __delta__ has to be in [0,1].
The return value indicates whether a rest exists. 

[3] O(1)

*/
bool RelInterval::Split(const double delta, const bool closeFirst,
                        RelInterval& Rest){
    __TRACE__
  if((delta==0) &&  (!leftClosed || !closeFirst))
     return false;
  if((delta==1) && (!rightClosed || closeFirst))
     return false;

   if(length.IsZero())
     return false;

  Rest.Equalize(this);
  rightClosed=closeFirst;
  Rest.leftClosed=!closeFirst;
  length.Split(delta,Rest.length);
  return true;
}

/*
~Split~

This function splits an interval at a given point in time. 
If the splitting instant is outside the interval, this interval or
Rest will be undefined. The other one will contain this interval.
If the spitting point is inside the interval, this interval will 
end at this endpoint and Rest will begin at duration. The return 
value is always true.

*/

bool RelInterval::Split(const DateTime duration, const bool closeFirst,
                        RelInterval& Rest){


  // at this point all cases with unbounded intervals are processed

   // duration left of this interval
   if(duration.LessThanZero()){
      // the complete interval is picked up by Rest
      Rest.Equalize(this);
      this->defined=false; 
      return true;
   }
   // duration left of this interval
   if(duration.IsZero() & (!closeFirst || ! leftClosed)){
      Rest.Equalize(this);
      this->defined=false;
      return true;
   }
   // duration right on this interval
   if(duration>length){
      // Rest will not be defined
      Rest.defined=false;
      return true;
   }
   // duration right on this interval
   if(duration==length & (!rightClosed || closeFirst)){
       // Rest will not be defined
       Rest.defined=false;
       return true;
   } 
  // the splitting instance is inside this interval
   Rest.length = this->length - duration;
   Rest.rightClosed=this->rightClosed;
   Rest.leftClosed=!closeFirst;
   this->length = duration;
   this->rightClosed=closeFirst;
   return true;
}


/*
~Equalize~

The ~Equalize~ function changes the value of this RelInterval to
the value of D2.

[3] O(1)

*/
void RelInterval::Equalize(const RelInterval* D2){
    __TRACE__
 length.Equalize(&(D2->length));
  leftClosed=D2->leftClosed;
  rightClosed=D2->rightClosed;
}

/*
~GetLength~

This function returns a clone of the contained time value.
Note that this function
creates a new DateTime instance. The caller of this function has to make free the
memory occupied by this instance after using it.

[3] O(1)

*/
DateTime* RelInterval::GetLength() const {
    __TRACE__
   DateTime* res = new DateTime(durationtype);
   res->Equalize(&length);
   return res;
}

/*
~StoreLength~

This function stored the length of this interval in the argument of this function.
The advantage of this function  in contrast to the GetLength function is that
no memory is allocated by this function. 

*/
void RelInterval::StoreLength(DateTime& result) const{
  __TRACE__
  result.Equalize(&length);

}



/*
~ToListExpr~

This function computes the list representation of this RelInterval value.

[3] O(1)

*/
ListExpr RelInterval::ToListExpr(const bool typeincluded)const{
  __TRACE__
  ListExpr time;
  time = length.ToListExpr(true);
  if(typeincluded)
       return nl->TwoElemList(nl->SymbolAtom("rinterval"),
                   nl->ThreeElemList(
                       nl->BoolAtom(leftClosed),
                       nl->BoolAtom(rightClosed),
                       time));
   else
      return nl->ThreeElemList(
                   nl->BoolAtom(leftClosed),
                   nl->BoolAtom(rightClosed),
                   time);
}

/*
~IsLeftClosed~

This function returns true if the left endpoint of this interval is
contained in it.

[3] O(1)

*/
bool RelInterval::IsLeftClosed()const{
    __TRACE__
  return leftClosed;
}

/*
~IsRightClosed~

This function will return true iff the interval is right closed.

[3] O(1)

*/
bool RelInterval::IsRightClosed()const{
    __TRACE__
 return rightClosed;
}


/*
~ReadFrom~

This function read the value from the argument. If the argument list don't
contains a valid RelInterval, false will be returned and the value of this
remains unchanged. In the other case the value is taken from this parameter
and true is returned.

[3] O(1)

*/
bool RelInterval::ReadFrom(const ListExpr LE, const bool typeincluded){
   __TRACE__

   ListExpr V;
   if(typeincluded){
      if(nl->ListLength(LE)!=2){
         if(DEBUG_MODE)
            cerr << __POS__ << ": wrong length for typed interval" << endl;
         return false;
      }
      if(!nl->IsEqual(nl->First(LE),"rinterval")){
         if(DEBUG_MODE){
            cerr << __POS__ << ": wrong type for interval" << endl;
            cerr << "expected : rinterval , received :";
            nl->WriteListExpr(nl->First(LE));
         }
         return false;
      }
      V = nl->Second(LE);
   } else
     V = LE;
   if(nl->ListLength(V)!=3){
       if(DEBUG_MODE)
          cerr << __POS__ << ": wrong length for interval" << endl;
       return false;
   }
   if(nl->AtomType(nl->First(V))!=BoolType){
     if(DEBUG_MODE)
        cerr << __POS__ << ": wrong type in interval" << endl;
     return false;
   }
   if(nl->AtomType(nl->Second(V))!=BoolType){
     if(DEBUG_MODE)
        cerr << __POS__ << ": wrong type in interval" << endl;
     return false;
   }
   DateTime time=DateTime(durationtype);
   if(!(time.ReadFrom(nl->Third(V),true))){
         if(DEBUG_MODE)
           cerr << __POS__ << ": error in reading length of interval" << endl;
         return false;
   }
   bool LC = nl->BoolValue(nl->First(V));
   bool RC = nl->BoolValue(nl->Second(V));
   // a single instant has to be both left- and rightclosed
   if( (time.IsZero()) && (!LC || !RC)){
     if(DEBUG_MODE)
        cerr << __POS__ << ": invalid values in interval" << endl;
      return false;
   }
   leftClosed=LC;
   rightClosed=RC;
   length.Equalize(&time);
   
   return true;
}


/*
~Set~

This function sets this RelINterval to be finite with determined length.
If the arguments don't represent a valid RelINterval value, the return value
will be false and this instance remains unchanged. In the other case true
is returned and the value of this is taken from the arguments.

[3] O(1)

*/
bool RelInterval::Set(const DateTime* length, const bool leftClosed,
                      const bool rightClosed){
    __TRACE__
 if((length->IsZero()) && (!leftClosed || !rightClosed)) return false;
 this->leftClosed=leftClosed;
 this->rightClosed=rightClosed;
 this->length.Equalize(length);
 return true;
}


/*
~SetLeftClosed~


This function sets the closure of this interval at its left end.

*/
void RelInterval::SetLeftClosed(bool LC){
   this->leftClosed = LC;
}  

/*
~SetRightClosed~

This function works like the function ~SetLeftClosed~ but for the
right end of this interval.

*/
void RelInterval::SetRightClosed(bool RC){
   this->rightClosed = RC;
}  

/*
~SetClosure~

This functions is a summarization of the ~SetLeftClosed~ and the
~SetRightClosed~ function. The effect will be the same like calling
this two functions.

*/
void RelInterval::SetClosure(bool LC,bool RC){
   this->leftClosed = LC;
   this->rightClosed = RC;
}




/*
~SetLength~

This function is used to set the length of this RelINterval.
If the given parameter collides with internal settings, false
is returned.

[3] O(1)

*/
bool RelInterval::SetLength(const DateTime* T){
    __TRACE__
   if(T->IsZero() && (!leftClosed || !rightClosed)) return false;
   length.Equalize(T);
   return true;
}


/*
~ToString~

This function returns a string representation of this RelInterval.

[3] O(1)

*/
string RelInterval::ToString()const{
    __TRACE__
  ostringstream tmp;
   tmp << (leftClosed?"[":"]");
   tmp << " 0.0 , ";
   tmp << length.ToString();
   tmp << (rightClosed?"]":"[");
   return tmp.str();
}

/*

~Where~

This function returns at which procentual part of this interval T is located.
The result will be a value in $[0,1]$. In all errors cases the value -1
is returned.

[3] O(1)

*/
double RelInterval::Where(const DateTime* T)const{
    __TRACE__
  if(length.CompareTo(T)<0)
     return -1;
  if(T->LessThanZero()) return -1;
  unsigned long ms1 = 86400000L*T->GetDay()+T->GetAllMilliSeconds();
  unsigned long ms2 = 86400000L*length.GetDay()+length.GetAllMilliSeconds();
  return (double)ms1/(double)ms2;
}


/*
~Plus~

This function  adds the argument to this interval. 
The 'weld point' is included to the new interval regardless
to the closure properties of the source intervals. The closure on the
left of this interval will not be changed and the closure on the 
right is taken from the argument.

*/
bool RelInterval::Plus(const RelInterval* I){
  rightClosed = I->rightClosed;
  length.Add(&(I->length));
  return true;
}


/*
3.8 The Implementation of the Class PMPoint

*/

/*
~Constructor~

This constructor creates an undefined periodic moving point.
The argument of this constructor is ignored. The reason for the
parameter is to make this constructor distinct from the standard
constructor which must be nothing.

[3] O(1)

*/
PMPoint::PMPoint(int dummy):
   linearMoves(0),
   compositeMoves(0),
   compositeSubMoves(0),
   periodicMoves(0),
   defined(false),
   canDelete(false),
   interval(0),
   startTime(instanttype),
   bbox(0)
{   __TRACE__
}

/*
~Destructor~

[3] O(1)

*/
PMPoint::~PMPoint(){
    __TRACE__
  if(canDelete){
      if(linearMoves.Size()>0) linearMoves.Destroy();
      if(compositeMoves.Size()>0) compositeMoves.Destroy();
      if(compositeSubMoves.Size()>0) compositeSubMoves.Destroy();
      if(periodicMoves.Size()>0) periodicMoves.Destroy();
   }
}

/*
~Destroy~

[3] O(1)

*/
void PMPoint::Destroy(){
    __TRACE__
   canDelete=true;
}


/*
~Equalize~

This functions changes the value of this periodic moving point
to be equals to the given parameter.

[3] O(L), where L is the number of linear moves

*/
void PMPoint::Equalize(const PMPoint* P2){
    __TRACE__
 // equalize the linear moves
  const LinearPointMove* LM;
  linearMoves.Clear();
  if(P2->linearMoves.Size()>0)
     linearMoves.Resize(P2->linearMoves.Size());
  for(int i=0;i<P2->linearMoves.Size();i++){
     P2->linearMoves.Get(i,LM);
     linearMoves.Append(*LM);
  }

  // equalize the composite moves
  const SpatialCompositeMove* CM;
  compositeMoves.Clear();
  if(compositeMoves.Size()>0)
     compositeMoves.Resize(P2->compositeMoves.Size());
  for(int i=0;i<P2->compositeMoves.Size();i++){
     P2->compositeMoves.Get(i,CM);
     compositeMoves.Append(*CM);
  }

  // equalize the composite submoves
  const SubMove* SM;
  compositeSubMoves.Clear();
  if(compositeSubMoves.Size()>0)
     compositeSubMoves.Resize(P2->compositeSubMoves.Size());
  for(int i=0;i<P2->compositeSubMoves.Size();i++){
     P2->compositeSubMoves.Get(i,SM);
     compositeSubMoves.Append(*SM);
  }

  // equalize the periodic moves
  const SpatialPeriodicMove* PM;
  periodicMoves.Clear();
  if(periodicMoves.Size()>0)
     periodicMoves.Resize(P2->periodicMoves.Size());
  for(int i=0;i<P2->periodicMoves.Size();i++){
     P2->periodicMoves.Get(i,PM);
     periodicMoves.Append(*PM);
  }

  defined = P2->defined;
  interval.Equalize(&(P2->interval));
  startTime.Equalize(&(P2->startTime));
  bbox.Equalize(&(P2->bbox));
  submove.Equalize(&(P2->submove));

}

/*
~NumOfFLOBs~

This function returns the number of contained FLOBs in this
class. Because four DBarrays are managed her, the return value
is 4.

[3] O(1)

*/
int PMPoint::NumOfFLOBs(){
    __TRACE__
   return 4;
}

/*
~GetFLOB~

This function returns the FLOB with index i.

[3] O(1)

*/
FLOB* PMPoint::GetFLOB(const int i){
    __TRACE__
   cout << "GetFlob " << i << endl;
   assert(i>=0 && i<NumOfFLOBs());
   FLOB* res=0;
   switch(i){
      case 0 : res = &linearMoves; break;
      case 1 : res = &compositeMoves;break;
      case 2 : res = &compositeSubMoves;break;
      case 3 : res = &periodicMoves;break;
   }

   cout << "Size is " << (res->Size()) << endl;
   return res; 
}

/*
~Compare~

This function is not implemented at this moment.

[3] O(-1)

*/
int PMPoint::Compare(const Attribute*)const{
    __TRACE__
  cout << " Warning! PMPoint::Compare  not implemented" << endl;
   return 0;
}

/*
~Adjacent~

We can't define a adjacent relation beween two periodic moving
points. For this reason the return value is allways __false__.

[3] O(1)

*/
bool PMPoint::Adjacent(const Attribute* ) const{
    __TRACE__
  return false;
}

/*
~Clone~

The ~Clone~ function returns a copy of this.

[3] O(L)

*/
PMPoint* PMPoint::Clone() const{
    __TRACE__
  PMPoint* copy = new PMPoint(0);
  copy->Equalize(this);
  return copy;
}


/*
~Sizeof~

This function returns the size of the PMPoint-class

[3] O(1)

*/
int PMPoint::Sizeof() const{
    __TRACE__
  return sizeof(PMPoint);
}

/*
~IsDefined~

This Function returns the defined state of a periodic moving point.

[3] O(1)

*/
bool PMPoint::IsDefined() const{
    __TRACE__
 return defined;
}

/*
~SetDefined~

Here the defined state of this point is set. If the argument has the
value __false__, the content of this will be lost. If the parameter
holds the value __true__, the call of this function has no effect.

[3] O(1)

*/
void PMPoint::SetDefined(const bool defined){
    __TRACE__
 if(defined) return;
  this->defined = false;
  linearMoves.Clear();
  compositeMoves.Clear();
  compositeSubMoves.Clear();
  periodicMoves.Clear();
  bbox.SetUndefined();
}

/*
~HashValue~

Returns the HashValue for this Point.

[3] O(1)

*/
size_t PMPoint::HashValue()const{
    __TRACE__
 DateTime* L = interval.GetLength();
  size_t res = (size_t) (bbox.Size()+(int)L->GetDay());
  delete L;
  L = NULL;
  return res;
}

/*
~CopyFrom~

The PMPoint instance takes its value from the given argument.
The caller has to ensure that __arg__ is of type PMPoint.

[3] O(L)

*/
void PMPoint::CopyFrom(const StandardAttribute* arg){
    __TRACE__
  Equalize((PMPoint*)arg);
}

/*
~ToListExpr~

This function returns the ListExpr representing this point.
The flag which is the argument constrols whether only the
value list is returned or whether a list with structure
(type value) will be returned.


[3] O(L)

*/
ListExpr PMPoint::ToListExpr(const bool typeincluded)const{
    __TRACE__
  ListExpr timelist = startTime.ToListExpr(true);
  ListExpr SubMoveList;
  if(defined)
     SubMoveList = GetSubMoveList(&submove);
  else
     SubMoveList = nl->SymbolAtom("undefined");

   if(typeincluded)
      return nl->TwoElemList(
                  nl->SymbolAtom("pmpoint"),
                  nl->TwoElemList(
                      timelist,
                      SubMoveList));
   else
      return nl->TwoElemList(timelist,SubMoveList);
}

/*
~GetSubMove~

This functions determines the move from the given argument and
returns its nested list representation.

[3] O($L_{SM}$) , where $L_{SM}$ is the number of linear moves contained in SM

*/
ListExpr PMPoint::GetSubMoveList(const SubMove* SM)const{
    __TRACE__
 ListExpr SubMoveList;
  int SubMoveType = SM->arrayNumber;
  int index = SM->arrayIndex;
  if(SubMoveType==LINEAR)
      SubMoveList = GetLinearMoveList(index);
  else if(SubMoveType==COMPOSITE)
      SubMoveList = GetSpatialCompositeMoveList(index);
  else if(SubMoveType==PERIOD)
      SubMoveList = GetSpatialPeriodicMoveList(index);
  else{
       cerr << "unknown submove type detected" << SubMoveType << endl;
       cerr << __POS__ << " Error in creating ListExpr" << endl;
       assert(false);
   }
  return SubMoveList;
}


/*
~GetLinearMove~

This functions returns the nested list representation of the
linear move at the specified index.

[3] O(1)

*/
ListExpr PMPoint::GetLinearMoveList(const int index)const{
    __TRACE__
   const LinearPointMove* LM;
   linearMoves.Get(index,LM);
   return LM->ToListExpr();
}

/*
~GetSpatialPeriodicMove~

This function converts the periodic move at the specified index
to its nested list representation.

[3] O($L_{P}$), where $L_{P}$ is the number of linear moves in the periodic move at index

*/
ListExpr PMPoint::GetSpatialPeriodicMoveList(const int index)const{
    __TRACE__
  const SpatialPeriodicMove* PM;;
  periodicMoves.Get(index,PM);
  ListExpr periodtype = nl->SymbolAtom("period");
  ListExpr RepList = nl->IntAtom(PM->repeatations);
  ListExpr SML = GetSubMoveList(&(PM->submove));
  ListExpr LC = nl->BoolAtom(PM->interval.IsLeftClosed());
  ListExpr RC = nl->BoolAtom(PM->interval.IsRightClosed());
  return  nl->TwoElemList(periodtype,nl->FourElemList(RepList,LC,RC,SML));
}

/*
~GetSpatialCompositeMoveList~

This function returns the nested list representation of the composite
move at the specified array index.

[3] O(L) , where L is the number of submoves contained in the linear move at index

*/
ListExpr PMPoint::GetSpatialCompositeMoveList(const int index)const{
    __TRACE__
 const SpatialCompositeMove* CM;
 compositeMoves.Get(index,CM);
 ListExpr CType = nl->SymbolAtom("composite");
 int minIndex = CM->minIndex;
 int maxIndex = CM->maxIndex;
 ListExpr SubMovesList;
 if(maxIndex<minIndex){
    cerr << __POS__ << "empty composite move" << endl;
    SubMovesList = nl->TheEmptyList();
 }
 else{
   // construct the List of submoves
   const SubMove* SM;
   compositeSubMoves.Get(minIndex,SM);
   SubMovesList = nl->OneElemList(GetSubMoveList(SM));
   ListExpr Last = SubMovesList;
   for(int i=minIndex+1;i<=maxIndex;i++){
     compositeSubMoves.Get(i,SM);
     Last = nl->Append(Last,GetSubMoveList(SM));
   }
 }
 return nl->TwoElemList(CType,SubMovesList);
}



/*
~ReadFrom~

This function reads the value of this p.m. point from the
given nested list. If the nested list don't contains a
valid point, the return value will be false and this point
is set to be undefined. Otherwise the point has the value
described in the nested list. The list consist only of the
the value, this means the type description is not included
in this list.

[3] O(L)  where L = number of linear moves

*/
bool PMPoint::ReadFrom(const ListExpr value){
    __TRACE__
 /* The list is scanned twice. In the first scan we
     compute only the needed size for each of the  contained arrays. 
     This is done to avoid a frequently resize of the arrays which 
     would lead to a lot of overhead for copying the contents.
  */

  if(nl->ListLength(value)!=2){
     if(DEBUG_MODE){
        cerr << __POS__ << ": wrong listlength (";
        cerr << (nl->ListLength(value)) << ")" << endl;
     }
     SetDefined(false);
     return false;
  }

  if(!ResizeArrays(value)){
     if(DEBUG_MODE){
        cerr << __POS__ << ": resize array failed" << endl;
     }
     SetDefined(false);
     return false;
  }

  if(!startTime.ReadFrom(nl->First(value),true)){
     if(DEBUG_MODE){
        cerr << __POS__ << "reading of the start time failed" << endl;
        cerr << "The list is " << endl;
        nl->WriteListExpr(nl->First(value));
     }
     SetDefined(false);
     return false;
  }
  // now we have to append the included submove
  ListExpr SML = nl->Second(value);
  if(nl->ListLength(SML)!=2){
     if(DEBUG_MODE){
        cerr << __POS__ << ": wrong list length for submove" << endl;
     }
     SetDefined(false);
     return false;
  }

  // get the submove type
  ListExpr SMT = nl->First(SML);
  int LMIndex = 0;
  int CMIndex = 0;
  int SMIndex = 0;
  int PMIndex = 0;
  if(nl->IsEqual(SMT,"linear")){
     submove.arrayNumber = LINEAR;
     submove.arrayIndex = 0;
     if(!AddLinearMove(nl->Second(SML),LMIndex,CMIndex,SMIndex,PMIndex)){
         if(DEBUG_MODE){
            cerr << __POS__ << " Error in reading linear move" << endl;
         }
         SetDefined(false);
         return false;
     }
     defined=true;
     // read out the interval and the bounding box from the 
     // created linear move
     const LinearPointMove* LM;
     linearMoves.Get(0,LM);
     interval.Equalize(&(LM->interval));
     bbox.Equalize(&(LM->bbox));
     return true;
  }

  if(nl->IsEqual(SMT,"composite")){
     submove.arrayNumber=COMPOSITE;
     submove.arrayIndex = 0;
     if(!AddSpatialCompositeMove(nl->Second(SML),LMIndex,
        CMIndex,SMIndex,PMIndex)){
        if(DEBUG_MODE){
           cerr << __POS__ << "error in reading composite move" << endl;
        }
        SetDefined(false);
        return false;
     }
     defined = true;
     // get interval and bounding box from this move
     const SpatialCompositeMove* CM;
     compositeMoves.Get(0,CM);
     interval.Equalize(&(CM->interval));
     bbox.Equalize(&(CM->bbox));
     return true;
  }
  if(nl->IsEqual(SMT,"period")){
     submove.arrayNumber = PERIOD;
     submove.arrayIndex = 0;
     if(!AddPeriodMove(nl->Second(SML),LMIndex,CMIndex,SMIndex,PMIndex)){
        if(DEBUG_MODE){
          cerr << __POS__ << " error in reading periodic move" << endl;
        }
        SetDefined(false);
        return false;
     }
     defined = true;
     // get interval as well as bounding box
     const SpatialPeriodicMove* PM;
     periodicMoves.Get(0,PM);
     interval.Equalize(&(PM->interval));
     bbox.Equalize(&(PM->bbox));
     return true;
  }
  if(DEBUG_MODE){
     cerr << __POS__ << "unknown subtype" << endl;
     nl->WriteListExpr(SMT);
  }
  return false;
}


PMPoint* PMPoint::Intersection(const PInterval* interval){
    __TRACE__
  PMPoint* result = new PMPoint(1);
   PInterval MyInterval=PInterval(this->startTime,this->interval);
   if(!(MyInterval.Intersects(interval))){
      return result;  // return an empty pmpoint
   }
   if(interval->Contains(&MyInterval)){
        result->Equalize(this);
        return result; // return a copy of this
   }
   int Lcount,Ccount,Scount,Pcount;  // sizes for the arrays
   Lcount = Ccount = Scount = Pcount = 0; // no move
   DateTime* tmpTime = startTime.Clone();
   AddSubMovesSizeForIntersection(tmpTime,submove,interval,
                                  Lcount,Ccount,Scount,Pcount);
   if(Lcount>0) result->linearMoves.Resize(Lcount);
   if(Ccount>0) result->compositeMoves.Resize(Ccount);
   if(Scount>0) result->compositeSubMoves.Resize(Scount);
   if(Pcount>0) result->compositeSubMoves.Resize(Pcount);


   // in a first step we count the array-sizes
   cout << "PMPOINT::Intersection(Interval) not implemented ";
   cout  << __POS__ << endl;
   return 0;
}

/*
~computeSubMovesSizeForIntersection~

This function computes the array sizes needed by the result of the intersection
of the calling periodic moving point instance with the given interval.

*/
void PMPoint::AddSubMovesSizeForIntersection(DateTime* startTime,
               const SubMove submove, const PInterval* interval,
               int &Lcount,int &Ccount, int &Scount,int &Pcount){
    __TRACE__
  int number = submove.arrayNumber;
   if(number==LINEAR){
      const LinearPointMove* LM;
      linearMoves.Get(submove.arrayIndex,LM);
      PInterval i = PInterval((*startTime) , LM->interval);
      if(i.Intersects(interval))
           Lcount++;
      startTime->Add(LM->interval.GetLength());
   }
   if(number==COMPOSITE){
      const SpatialCompositeMove* CM;
      compositeMoves.Get(submove.arrayIndex,CM);
      int start = CM->minIndex;
      int end = CM->maxIndex;
      int sm=0;
      bool stop = false;
      for(int s=start;s<=end && !stop;s++){
         int oldL = Lcount;
         int oldC = Ccount;
         int oldP = Pcount;
         const SubMove* SM;
         compositeSubMoves.Get(s,SM);
         AddSubMovesSizeForIntersection(startTime,*SM,interval,Lcount,
                                        Ccount,Scount,Pcount);
         if( (Lcount!=oldL) || (Ccount != oldC) || (Pcount != oldP))
             sm++;
         else
             stop = true;
      }
      if(sm>1){
         Ccount ++;
         Scount += sm;
      }
   }
   if(number==PERIOD){
         const SpatialPeriodicMove* PM;
         periodicMoves.Get(submove.arrayIndex,PM);
         SubMove PSM = PM->submove;
         cout << __POS__ << "NOT IMPLEMENTED" << endl;
         /* wenn PeriodicMOve-Intervall in geg. Interval enthalten
            erhoehe um alle enthaltenen Submoves
            wenn Intervall vor SubMOve-Intervall beendet,
            mache mit SubMoves weiter
            sonst fuege alle vollstaendig enthaltenen Perioden hinzu
            weiterhin ein Composite Move (welches Periode und Rest verbindet
           bearbeite den Rest */
   }
}


/*
~CheckCorrectness~

This function checks whether the representation of this
periodic moving point is correct. This means

  *  no directly nested composite moves exists

  *  no directly nested periodic moves exists

  *  a composite move has at least two submoves

[3] O(n) , where n is the number of moves

*/
bool PMPoint::CheckCorrectness(){
  // check for directly nested composite moves
    __TRACE__
  const SubMove* SM;
  size_t linearSize = linearMoves.Size();
  size_t periodSize = periodicMoves.Size();
  size_t compositeSize = compositeMoves.Size();
  size_t compositeSubSize = compositeSubMoves.Size();

  for(size_t i=0; i<compositeSubSize;i++){
    compositeSubMoves.Get(i,SM);
    size_t an = SM->arrayNumber;
    size_t index = SM->arrayIndex;
    switch(an){
      case COMPOSITE:
          if(DEBUG_MODE){
             cerr << __POS__ << "nested compositeMove detected" << endl;
           }
           return false;
      case PERIOD:
          if(index>=periodSize){
             if(DEBUG_MODE){
               cerr << __POS__ << "array index " << index 
                    << "out of bounds " << periodSize << endl;
             }
             return false;
          }
          break;
      case LINEAR:
          if(index>=linearSize){
             if(DEBUG_MODE){
               cerr << __POS__ << "array index " << index 
                    << "out of bounds " << linearSize << endl;
             }
             return false;
          }
          break;
     default:
          if(DEBUG_MODE){
              cerr << __POS__ << "unknown submove found " << endl;
          }
          return false;     
    } 
  }

  // check for direcly nested periodic moves
  const SpatialPeriodicMove* PM;
  for(int i=0; i<periodicMoves.Size();i++){
      periodicMoves.Get(i,PM);
      int an = PM->submove.arrayNumber;
      size_t index = PM->submove.arrayIndex;
			switch(an){
				case COMPOSITE:
						if(index>=compositeSize){
							 if(DEBUG_MODE){
								 cerr << __POS__ << "array index " << index 
											<< "out of bounds " << compositeSize << endl;
							 }
							 return false;
						}
				case PERIOD:
             cerr << __POS__ << "nested periodic move detected" << endl;
				     return false;
				case LINEAR:
						if(index>=linearSize){
							 if(DEBUG_MODE){
								 cerr << __POS__ << "array index " << index 
											<< "out of bounds " << linearSize << endl;
							 }
							 return false;
						}
						break;
			 default:
						if(DEBUG_MODE){
								cerr << __POS__ << "unknown submove found " << endl;
						}
						return false;     
			}
      if(PM->repeatations<2){
         cerr << __POS__ << "invalid number of repetitions detected" << endl;
         return false;
      } 
  }

  // check for composite moves with only one submove
  const SpatialCompositeMove* CM;
  for(int i=0;i<compositeMoves.Size();i++){
     compositeMoves.Get(i,CM);
     if(CM->minIndex>=CM->maxIndex){
        if(DEBUG_MODE){
           cerr << __POS__ << "composite move with a single submove detected" << endl;
        }
        return false;
     }
     if(CM->maxIndex>=(int)compositeSubSize){
        if(DEBUG_MODE){
           cerr << __POS__ << "invalid submove position detcetd" << endl;
        }
        return false;
     }
  }
  return true;
}


/*
~ResizeArrays~

This function resizes the array to the values needed to include
all move types in the list. Note that a call of this function
changes this point  even though the list don't represent a
valid periodic moving point. This function should be used before
a periodic moving point is read from a nested list. The disadvantage 
is the twice scanning of the lsit, but a frequently resize of the 
array can be avoided.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoint::ResizeArrays(const ListExpr value){
    __TRACE__
 // first all entries in the arrays are removed
   linearMoves.Clear();
   compositeMoves.Clear();
   compositeSubMoves.Clear();
   periodicMoves.Clear();
   int LMSize = 0;
   int CMSize = 0;
   int SMSize = 0;
   int PMSize = 0;
   if(!AddSubMovesSize(nl->Second(value),LMSize,CMSize,SMSize,PMSize))
      return false;
   // set the arrays to the needed size
   if(LMSize>0) linearMoves.Resize(LMSize);
   if(CMSize>0) compositeMoves.Resize(CMSize);
   if(SMSize>0) compositeSubMoves.Resize(SMSize);
   if(PMSize>0) periodicMoves.Resize(PMSize);
   return true;
}

/*
~AddSubMovesSize~

This function computes the needed sizes for the arrays to hold the
value of the p.m. point represented in the value list.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoint::AddSubMovesSize(const ListExpr value,int &LMSize,int &CMSize,
                              int &SMSize,int &PMSize){
    __TRACE__
   // all moves have the length 2
   if(nl->ListLength(value)!=2){
       return false;
   }
   ListExpr type = nl->First(value);

   // the type has to be one of {linear, composite, period}
   if(nl->AtomType(type)!=SymbolType){
       return false;
  }
  // in a linear move we have only to increment the size of LM
  if(nl->IsEqual(type,"linear")){
     LMSize = LMSize +1;
     return true;
  }
  if(nl->IsEqual(type,"composite")){
     CMSize = CMSize+1; // the composite move itself
     ListExpr rest = nl->Second(value);
     while(!nl->IsEmpty(rest)){
        SMSize++; // a new submove
        if(!AddSubMovesSize(nl->First(rest),LMSize,CMSize,SMSize,PMSize))
           return false;
        rest = nl->Rest(rest);
     }
     return true;
  }
  if(nl->IsEqual(type,"period")){
     PMSize = PMSize+1;
     ListExpr PMove;
     int len = nl->ListLength(value);
     if(len==2){
        PMove = nl->Second(value);
     }
     else{ // invalid listlength
        return false;
     }
     return AddSubMovesSize(nl->Second(PMove),LMSize,CMSize,SMSize,PMSize);
  }
  // a unknown type description
  return false;
}

/*
~AddLinearMove~

This functions appends the linear move given as nested list in __value__
to the LinearMoves -array.

[3] O(1)

*/
bool PMPoint::AddLinearMove(const ListExpr value,int &LMIndex, int &CMIndex,
                            int &SMIndex, int &PMIndex){
    __TRACE__
  LinearPointMove LM = LinearPointMove(0);
   if(!LM.ReadFrom(value))
      return false;
   linearMoves.Put(LMIndex,LM);
   LMIndex++;
   return true;
}


/*
~AddSpatialCompositeMove~

This Functions appends the composite move given as nested list together with
all contained submoves at the appropriate arrays. The return value indiciates
the success of this call.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoint::AddSpatialCompositeMove(const ListExpr value,int &LMIndex,
                   int &CMIndex, int &SMIndex, int &PMIndex){
    __TRACE__
  // a composite move has to contains at least two submoves
   int len = nl->ListLength(value);
   if(len<2){
      if(DEBUG_MODE){
         cerr << __POS__ << " less than 2 submoves (" << len << ")" << endl;
      }
      return false;
   }
   SpatialCompositeMove CM=SpatialCompositeMove(1);
   int CMPos=CMIndex;
   // at this position we have to include this composite move
   int SMPos=SMIndex;
   // beginning at this position we have to include the submoves
   // ensure that no submove used the positions for this composite move
   CMIndex++;
   CM.bbox.SetUndefined();
   CM.minIndex=SMIndex;
   CM.maxIndex=SMIndex+len-1;
   SMIndex= SMIndex+len;
   // Append the contained Submoves
   ListExpr rest = value;
   ListExpr SML,TL,VL;
   bool isFirst = true;
   while(!nl->IsEmpty(rest)){
      SML = nl->First(rest);
      rest = nl->Rest(rest);
      if(nl->ListLength(SML)!=2){ // all submoves have the format (type value)
         if(DEBUG_MODE){
            cerr << __POS__ << " submove has wrong length (";
            cerr << nl->ListLength(SML) << ")" << endl;
         }
         return false;
      }
      TL = nl->First(SML);
      VL = nl->Second(SML);
      if(nl->IsEqual(TL,"linear")){
         // process a linear submove
         int LMPos = LMIndex;
         if(!AddLinearMove(VL,LMIndex,CMIndex,SMIndex,PMIndex)){
            if(DEBUG_MODE){
               cerr << __POS__ << " can't add a linear move " << endl;
            }
            return false;
         }
         const LinearPointMove* LM;
         linearMoves.Get(LMPos,LM);
         // Append the interval of LM to CM
         if(isFirst){
            isFirst=false;
            CM.interval.Equalize(&(LM->interval));
         }else{
            if(!CM.interval.Append(&(LM->interval))){
               if(DEBUG_MODE){
                   cerr << __POS__ << " can't append interval ";
                   cerr << endl;
                   cerr << "The original interval is";
                   cerr << CM.interval.ToString() << endl;
                   cerr << "The interval to append is";
                   cerr << LM->interval.ToString() << endl;
               }
               return false;
            }
         }
         // build the union of the bounding boxes of CM and LM
         CM.bbox.Union(&(LM->bbox));
         // put the submove in the array
         SubMove SM;
         SM.arrayNumber = LINEAR;
         SM.arrayIndex = LMPos;
         compositeSubMoves.Put(SMPos,SM);
         SMPos++;
      } else if(nl->IsEqual(TL,"period")){
        // process a periodic submove
        int PMPos = PMIndex;
        if(!AddPeriodMove(VL,LMIndex,CMIndex,SMIndex,PMIndex)){
           if(DEBUG_MODE){
              cerr << __POS__ << "can't add period move " << endl;
            }
            return  false;
        }
        const SpatialPeriodicMove* PM;
        periodicMoves.Get(PMPos,PM);
        if(isFirst){
           isFirst=false;
           CM.interval.Equalize(&(PM->interval));
        }else{
           if(!CM.interval.Append(&(PM->interval))){
              if(DEBUG_MODE){
                 cerr << __POS__  << " can't append interval" << endl;
              }
              return false;
           }
        }
        CM.bbox.Union(&(PM->bbox));
        SubMove SM;
        SM.arrayNumber = PERIOD;
        SM.arrayIndex = PMPos;
        compositeSubMoves.Put(SMPos,SM);
        SMPos++;
   } else{ // not of type linear or period
      if(DEBUG_MODE){
          cerr << __POS__ 
               << " submove not of type linear or period" 
               << endl;
       }
       return false;
      }
   }
   // put the compositeMove itself
   compositeMoves.Put(CMPos,CM);
   return true;
}

/*
~AddPeriodMove~

This functions append the periodic move contained in the nested list
 __value__ to this periodic moving point.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoint::AddPeriodMove(const ListExpr value,int &LMIndex, int &CMIndex,
                             int &SMIndex, int &PMIndex){

  __TRACE__

  cerr << "add a periodic move at index " << PMIndex << endl;

 int len = nl->ListLength(value); 
 if(len!=2 ){  // (repeatations <submove>) 
    if(DEBUG_MODE)
       cerr << __POS__ << ": wrong listlength" << endl;
    return false;
 }
 if(nl->AtomType(nl->First(value))!=IntType){
    if(DEBUG_MODE){
       cerr << __POS__ << ": wrong type for repeatations" << endl;
    }
    return false;
 }
 int rep = nl->IntValue(nl->First(value));
 // rep must be greater than 1 
 if(rep<=1){
     if(DEBUG_MODE){
        cerr << __POS__ <<  " wrong number of repeatations" << endl;
     }
     return false;
 }

 ListExpr SML;
 SML = nl->Second(value);
 if(nl->ListLength(SML)!=2){
     if(DEBUG_MODE){
        cerr << __POS__ << ": wrong length for submove" << endl;
     }
     return false;
 }
 SpatialPeriodicMove PM=SpatialPeriodicMove(1);
 PM.repeatations = rep;
 int IncludePos = PMIndex; // store the positiuon
 PMIndex++;
 ListExpr SMT = nl->First(SML); // take the submove type
 if(nl->IsEqual(SMT,"linear")){
    int LMPos = LMIndex;
    if(!AddLinearMove(nl->Second(SML),LMIndex,CMIndex,SMIndex,PMIndex)){
       if(DEBUG_MODE){
          cerr << __POS__ << ": can't add linear submove" << endl;
       }
       return false;
    }
    PM.submove.arrayNumber = LINEAR;
    PM.submove.arrayIndex = LMPos;
    const LinearPointMove* LM;
    linearMoves.Get(LMPos,LM);
    PM.bbox.Equalize(&(LM->bbox));
    RelInterval SMI = LM->interval;
    PM.interval.Equalize(&SMI);
    PM.interval.Mul(rep);
    if(len==4){
      ListExpr LC = nl->Second(value);
      ListExpr RC = nl->Third(value);
      if((nl->AtomType(LC)!=BoolType) || (nl->AtomType(RC)!=BoolType))
          return false;
      PM.interval.SetLeftClosed(nl->BoolValue(LC));
      PM.interval.SetRightClosed(nl->BoolValue(RC));

     }
    periodicMoves.Put(IncludePos,PM);
    return true;
 }else if(nl->IsEqual(SMT,"composite")){
    int CMPos = CMIndex;
    if(!AddSpatialCompositeMove(nl->Second(SML),LMIndex,
                                CMIndex,SMIndex,PMIndex)){
       if(DEBUG_MODE){
          cerr << __POS__ << ": can't add composite submove" << endl;
       }
       return false;
    }
    PM.submove.arrayNumber = COMPOSITE;
    PM.submove.arrayIndex = CMPos;
    const SpatialCompositeMove* CM;
    compositeMoves.Get(CMPos,CM);
    PM.bbox.Equalize(&(CM->bbox));
    RelInterval SMI = CM->interval;
    PM.interval.Equalize(&SMI);
    PM.interval.Mul(rep);
    periodicMoves.Put(IncludePos,PM);
    return true;
 }
 // not of type linear or composite
 if(DEBUG_MODE){
     cerr << __POS__ << ": invalid submove-type for periodic move" << endl;
     cerr << "This list is : " << endl;
     nl->WriteListExpr(SMT);
     cerr << endl << "end of list " << endl;
 }
 return false;

}

/*
~IsEmpty~

The ~IsEmpty~ function returns true if this periodic moving point
contains no units.

*/
bool PMPoint::IsEmpty()const {
    __TRACE__
  return linearMoves.Size()==0;
}


/*
~At~

This Function computes the location of this Point at the given instant.
If the periodic moving point is not defined at this point of time, the
result will be an undefined Point instance.

[3] O(L), where L is the number of contained linear moves

*/
Point* PMPoint::At(const DateTime* instant)const{
    __TRACE__
    DateTime* duration = new DateTime(instanttype);
    duration->Equalize(instant);
    duration->Minus(&startTime); // now it is really a duration
    Point* res;
    if(interval.Contains(duration)){
       const SubMove* sm = &submove;
       const SpatialCompositeMove* CM;
       const SpatialPeriodicMove* PM;
       RelInterval RI;
       // i have to find the linear move which is "active" at instant
       while(sm->arrayNumber!=LINEAR){
          if(sm->arrayNumber==COMPOSITE){
             // in this stage of implementation a linear search is
             // executed. i have to make it better in the future
             int i = sm->arrayIndex;
             compositeMoves.Get(i,CM);
             int cur = CM->minIndex;
             int max = CM->maxIndex;
             bool found=false;
             while( (cur<=max) && ! found){
                compositeSubMoves.Get(cur,sm); // get the submove
                if(sm->arrayNumber==LINEAR){
                   const LinearPointMove* LM;
                   linearMoves.Get(sm->arrayIndex,LM);
                   RI = LM->interval;
                } else if(sm->arrayNumber==PERIOD){
                   periodicMoves.Get(sm->arrayIndex,PM);
                   RI = PM->interval;
                } else { //another submoves are not allowed
                   assert(false);
                }
                if(RI.Contains(duration)) // be happy
                   found=true;
                else{  // search again
                   DateTime* L = RI.GetLength();
                   duration->Minus(L);
                   delete L;
                   L = NULL;
                   cur++;
                }
             }
             assert(found); //otherwise we have an error in computation
          } else if(sm->arrayNumber==PERIOD){
             int index = sm->arrayIndex;
             periodicMoves.Get(index,PM);
             // this is a very slow implementation
             // i have to speed up it in the future
             RelInterval RI;
             sm = &(PM->submove);
             if(PM->submove.arrayNumber==LINEAR){
                 const LinearPointMove* LM;
                 linearMoves.Get(PM->submove.arrayIndex,LM);
                 RI = LM->interval;
             } else if(PM->submove.arrayNumber==COMPOSITE){
                 compositeMoves.Get(PM->submove.arrayIndex,CM);
                 RI = CM->interval;
             } else { //another submoves are not allowed
                 assert(false);
             }
             while(!RI.Contains(duration)){
                DateTime* L = RI.GetLength();
                duration->Minus(L);
                delete L;
                L = NULL;
             }
          } else{
             // this case should never occurs
             assert(false);
          }
       }
       const LinearPointMove* LM;
       linearMoves.Get(sm->arrayIndex,LM);
       if(LM->IsDefinedAt(duration))
          res=LM->At(duration);
       else
          res = new Point(false,0.0,0.0);
    } else { // the duration does not contains the argument instant
         res = new Point(false,0.0,0.0);
    }
    delete duration;
    duration = NULL;
    return res;
}

/*
~Initial~

This function computes the first location of this moving point.

*/
Point* PMPoint::Initial()const{
    __TRACE__
 Point* res = new Point(0,0);
  if(IsEmpty()){
    res->SetDefined(false);
    return res;
  }
    
  const LinearPointMove* lm;
  linearMoves.Get(0,lm);
  res->Set(lm->startX,lm->startY);
  res->SetDefined(true);
  return res;
}

/*
~Final~

The ~Final~ function returns the last defined position of this point.
If the value is empty, an undefined point is returned.

*/
Point* PMPoint::Final(){
    __TRACE__
 Point* res = new Point(0,0);
  if(IsEmpty()){
     res->SetDefined(false);
     return res;
  }       
  LinearPointMove lm =  GetLastUnit();
  res->Set(lm.endX,lm.endY);
  res->SetDefined(true);
  return res;
}

/*
~GetLastUnit~

This function returns the temporal last unit of this periodic moving point.

*/
LinearPointMove PMPoint::GetLastUnit(){
    __TRACE__
 // In the current version of the implementation we can just returm the 
  // last unit in the array. If the implementation is changed to reuse 
  // a subtree, we have to go the tree downwards to find it.
  int pos = linearMoves.Size()-1;
  const LinearPointMove* res;
  linearMoves.Get(pos,res);
  return *res; 
}


/*
~Breakpoints~

This function computes the coordinates on which halt's the 
periodic moving point. The result corresponds to the 
positions of static units. A call of this function is
an abbreviation to a call of breakpoints(d,false), where
__d__ is a duration of length zero.

[3] O(L) where L is the number of the contained linear moves.

*/
Points* PMPoint::Breakpoints(){
    __TRACE__
    DateTime DT(durationtype);
    Points* res = Breakpoints(&DT,false);
    cout << (*res) << endl;
    return res;
    
}

/*
~BreakPoints~

This function computes the breakpoints for this pmpoint. Only
Breakpoints with a minimum break with lenth __duration__ are included in the
result. If the duration is less than zero, the result will be 
undefined. The same result will be occur if the duration is zero
and the inclusive argument is true.

*/
Points* PMPoint::Breakpoints(const DateTime* duration,const bool inclusive){
  __TRACE__
  cout << "breakpoints 2 entered\n";
  Points* Res = new Points(1);
  if(!defined || !duration->IsDefined()){
      Res->SetDefined(false);
      return Res;
  }
  if(duration->LessThanZero()){
      Res->SetDefined(false);
      return Res; 
  }
  if(duration->IsZero() && inclusive){
     Res ->SetDefined(false);
     return Res;
  }
  int size = linearMoves.Size();
  Res->Clear();
  Res->StartBulkLoad();
  const LinearPointMove* LM;
  for(int i=0; i<size; i++){
      linearMoves.Get(i,LM);
      if(LM->IsDefined() && LM->IsStatic()){
         DateTime* L = LM->interval.GetLength();
         int cmp = L->CompareTo(duration);
         delete L;
         if(cmp>0 || (cmp==0 && inclusive)){
            Point P(true,LM->startX,LM->startY);
            Res->InsertPt(P);
         }
      }
  }
  Res->EndBulkLoad();
  return Res;
}



/*
~Trajectory~

This function computes the trajectory of a single periodic moving point.

[3] O(L), where L is the number of contained linear moves

*/
CLine*  PMPoint::Trajectory(){
    __TRACE__
 const LinearPointMove* LM;
  // each linear moves needs 2 halfsegments
  CLine* Res = new CLine(linearMoves.Size()*2);  
  Res->Clear();
  Res->StartBulkLoad();
  CHalfSegment HS1;
  CHalfSegment HS2;
  for(int i=0;i<linearMoves.Size();i++){
     linearMoves.Get(i,LM);
     if(LM->IsDefined() && !LM->IsStatic() ){
        HS1=LM->GetHalfSegment(true);
        HS2=LM->GetHalfSegment(false);
        if(HS1.IsDefined() && HS2.IsDefined()){
          *Res+=HS1;
          *Res+=HS2;
        }
     }
  }
  Res->EndBulkLoad();
  return Res;
}

/*
~GetStart~

This function returns the first point in time at which this
PMPoint is defined.

*/
DateTime PMPoint::GetStart()const{
    __TRACE__
   return startTime;
}

/*
~GetEnd~

This function returns the last point in time at which this
PMPoint is defined.

*/
DateTime PMPoint::GetEnd()const{
    __TRACE__
   DateTime end;
    end.Equalize(&startTime);
    end.Add(interval.GetLength());
    return end;
}

/*
~GetInterval~

Returns the interval of this periodic moving point. 

*/
PInterval PMPoint::GetInterval()const{
    __TRACE__
   PInterval res=PInterval(startTime,interval);
   return res;
}


/*
~Expand~

This function converts a periodic moving point to a linearly moving one.
The used data type comes from the TemporalAlgebra. 

*/
MPoint PMPoint::Expand(){
    __TRACE__
  // In a first step we compute the number of resulting units.
  // The reason is, to avoid frequently growing of the result.
  int size = NumberOfExpandedUnits();
  MPoint Result = MPoint(size);
  if(size==0)
    return Result; 
  DateTime* CurrentTime = startTime.Clone();
  Result.StartBulkLoad();
  AppendUnits(Result, CurrentTime,submove);
  Result.EndBulkLoad();
  delete CurrentTime;
  CurrentTime = NULL;
  return Result;
}

/*
~AppendUnits~

The function ~AppendUnits~ adds all mpoint-units resulting from S to P.

*/
void PMPoint::AppendUnits(MPoint& P, DateTime* Time, const SubMove S){
    __TRACE__
   if(S.arrayNumber==LINEAR){
        // first create the Intervall
        const LinearPointMove* LM;
        linearMoves.Get(S.arrayIndex,LM);
        DateTime* StartTime = new DateTime((*Time));
        DateTime* length = LM->interval.GetLength();
        Time->Add(length);
        delete length;
        length = NULL;
        Interval<DateTime> I((*StartTime),(*Time),
                     LM->interval.IsLeftClosed(),
                     LM->interval.IsRightClosed());
        UPoint up(I,LM->startX,LM->startY,LM->endX,LM->endY);
        P.Add(up);
        delete StartTime;
        StartTime = NULL;
        return;
    }
    if (S.arrayNumber==COMPOSITE){
       const SpatialCompositeMove* CM;
       compositeMoves.Get(S.arrayIndex,CM);
       const SubMove* sm;
       for(int i=CM->minIndex;i<=CM->maxIndex;i++){
            compositeSubMoves.Get(i,sm);
            AppendUnits(P,Time,*sm);
       }    
       return;
    }
    if(S.arrayNumber==PERIOD){
       const SpatialPeriodicMove* PM;
       periodicMoves.Get(S.arrayIndex,PM);
       long repeats = PM->repeatations;
       for(int i=0;i<repeats;i++)
          AppendUnits(P,Time,PM->submove);
       return;      
    }
    assert(false);
}


/*
~NumberOfExpandedUnits~

This functions returns the number of needed units of a MPoint of the
TemporalAlgebra to represent this periodic moving point.

*/
int PMPoint::NumberOfExpandedUnits(){
    __TRACE__
  if(!defined)
      return 0;
   if(IsEmpty())
      return 0;
   return NumberOfExpandedUnits(submove);
}

/*
~NumberOfExpandedUnits~

This function computed the needed size of a MPoint of the TemporalAlgebra
to represent this periodic moving one. 

*/
int PMPoint::NumberOfExpandedUnits(const SubMove S){
    __TRACE__
   if(S.arrayNumber==LINEAR)
       return 1;
    if(S.arrayNumber==COMPOSITE){
       const SpatialCompositeMove* CM;
       compositeMoves.Get(S.arrayIndex,CM);
       int num = 0;
       const SubMove* sm;
       for(int i=CM->minIndex;i<=CM->maxIndex;i++){
           compositeSubMoves.Get(i,sm);
           num += NumberOfExpandedUnits(*sm);
       }    
       return num;
    } 
    if(S.arrayNumber==PERIOD){
       const SpatialPeriodicMove* PM;
       periodicMoves.Get(S.arrayIndex,PM);
       int repeats = PM->repeatations;
       return repeats*NumberOfExpandedUnits(PM->submove);
    }
    assert(false); // this should never be reached
}

/*
ReadFrom

Thgis function reads a Periodic moving point from 
a lineary moving one.

*/
void PMPoint::ReadFromMPoint(MPoint& P){
  /* This function works as follow:
     First, we create a list containing all LinearMovingPoints for this
     Periodic Moving Points. After that, we find equal units in this list
     and assign each different unit with an unique number. From this array
     of numbers a repetition tree is build. This tree automatically  detect
     repetitions in this numberlist. From this tree, the final periodic moving
     point is created.
  */
  int noUPoints = P.GetNoComponents();
  // Unfortunately, we can't use a array directly because we need to
  // represent non-defined units directly witch is not required in
  // the MPoint representation.
  List<LinearPointMove>* L= new List<LinearPointMove>();  
  const UPoint* UP;
  bool lc,rc,wlc=false;
  DateTime start,end;
  double x1,y1,x2,y2;

  // standard-init

  defined = true;
  canDelete = false;
  bbox.SetEmpty();
  linearMoves.Clear();
  compositeMoves.Clear();
  compositeSubMoves.Clear();
  periodicMoves.Clear();
  UPoint LastMove;
  if(noUPoints>0){ // we have any units
     for(int i=0;i<noUPoints;i++){
        P.Get(i,UP);
        // get the values from the unit
        lc = UP->timeInterval.lc;   
        rc = UP->timeInterval.rc;
        start = UP->timeInterval.start;
        end   = UP->timeInterval.end;
        x1 = UP->p0.GetX();
        y1 = UP->p0.GetY();
        x2 = UP->p1.GetX();
        y2 = UP->p1.GetY();    
        DateTime Length = end-start;       
        LinearPointMove theMove(0);
        theMove.defined=true;
        theMove.isStatic = (x1==x2) && (y1==y2);
        theMove.startX = x1;
        theMove.startY = y1;
        theMove.endX = x2;
        theMove.endY = y2;
        theMove.bbox = PBBox(x1,y1,x2,y2);
        theMove.interval.Set(&Length,lc,rc);
        // Add the bounding to to the complete bb
        bbox.Union(&(theMove.bbox));
        if(i==0){
           startTime = start; // sets the starttime
           interval.SetDefined(true);
           wlc = UP->timeInterval.lc;
        }else {
           if( (!(LastMove.timeInterval.end == start)) ||
               (!LastMove.timeInterval.rc && !lc)){
               // a gap found between the last unit and this unit
               LinearPointMove* GapMove = new LinearPointMove(0);
               GapMove->defined=false; // an undefined move
               DateTime GapLength = start-LastMove.timeInterval.end;
               GapMove->interval.Set(&GapLength,
                                     !LastMove.timeInterval.rc, !lc);
               L->Append(*GapMove);
           }
        }
        if(i==noUPoints-1){ // set new interval;
           DateTime Len = end-startTime;
           interval.Set(&Len,wlc,UP->timeInterval.rc); 
        }
        L->Append(theMove);
        LastMove = *UP;
     }
  }else{ // no units available
    // here we have to initialize some components
  }
  // At this point, we have stored all LinearPointsMoves in a List.
  // now we have to build an array of numbers for it. To make this fast,
  // we use a hashtable with double maximum size. We use closed hashing 
  // with a linear collision strategy.
  int hashsize = L->GetLength()*2;

  int MinIndex[hashsize]; // Hashtable containing the minimum 
                          //index of this move
                          // in the AllMoves array
  int LMIndex[hashsize]; // The same table but holding the indices in the 
                         // resulting linear moves

  for(int i=0;i<hashsize;i++){ 
       MinIndex[i]=-1;
       LMIndex[i] =-1;
  }

  int listlength=L->GetLength();
  LinearPointMove* AllMoves = L->ConvertToArray();

  int indexInAllMoves[listlength];
  int indexInLinearMoves[listlength];

  for(int i=0;i<listlength;i++){
     indexInAllMoves[i] = -1;
     indexInLinearMoves[i] = -1;
  }

  int differentMoves =0; // number of different linear moves
  int lastusedindex = -1;

  // we assign each different value in the array to an unique number
  for(int i=0;i<listlength;i++){
     LinearPointMove theMove = AllMoves[i];
     size_t hashvalue = theMove.HashValue()%hashsize;
 
     // search a free index or the same move
     bool done = false;
     while(!done){
        if(MinIndex[hashvalue]<0){ // empty slot found
           // put the index of the move into the hashtable
           MinIndex[hashvalue] = i;
           LMIndex[hashvalue] = differentMoves;
           lastusedindex=i;
           differentMoves++;
           done = true;
        }
        else{
           if((AllMoves[MinIndex[hashvalue]]).CompareTo(&theMove)==0) {
              //equal element found; we are done
              done = true;
           }
           else{
              hashvalue = (hashvalue+1)%hashsize; // collision detected
           }
        }
     }
     indexInAllMoves[i]= MinIndex[hashvalue];
     indexInLinearMoves[i] = LMIndex[hashvalue];

     if(LMIndex[hashvalue]==93){ // error in my example
       cout << "Linear[" << i <<"]" << endl;
       cout << "53 => " << AllMoves[indexInAllMoves[53]].ToString() << endl;
       cout << "93 => " << AllMoves[indexInAllMoves[i]].ToString() << endl;

     }

  } 

  /* At this in, in assigned numbers the complete moving point is stored
     as sequel of array indices in the array AllMoves. Now we can build a
     repetition tree from this numbers.
  */
 
 
  RepTree RT(indexInLinearMoves,listlength);
  
  /* Debugging output */
  RT.PrintNL();

  linearMoves.Clear();
  compositeMoves.Clear();
  compositeSubMoves.Clear();
  periodicMoves.Clear();

  if(listlength==0){
     defined=false;
     canDelete=false;
   }else{
     defined = true;
     canDelete = false;
     // set all array to the required sizes
     if(differentMoves>0)
        linearMoves.Resize(differentMoves);
     int size = RT.NumberOfNodesWithType(COMPOSITION);
     if(size>0)
        compositeMoves.Resize(size);
     size = RT.NumberOfNodesWithType(REPETITION);
     if(size>0)
        periodicMoves.Resize(size);
     size = RT.NumberOfCompositeSons();
     if(size>0)
        compositeSubMoves.Resize(size);
     // in the first step, we copy the linear moves into the
     // appropriate array
     int lastnumber = -1;
     for(int i=0;i<listlength;i++){
         if(indexInLinearMoves[i]>lastnumber){
             lastnumber = indexInLinearMoves[i];
             linearMoves.Put(lastnumber,AllMoves[indexInAllMoves[i]]); 
         }
     }

      // analyse the tree and make the entries in the arrays
     int cp,csp,pp;
     cp=0,csp=0,pp=0;
     FillFromRepTree(cp,csp,pp,RT);

     int type = RT.GetNodeType();
     this->submove.arrayIndex=0;
     if(type==SIMPLE)
       this->submove.arrayNumber=LINEAR;
     else if(type==COMPOSITE)
       this->submove.arrayNumber=COMPOSITE;
     else if(type==REPETITION)
       this->submove.arrayNumber=PERIOD;
     else
       assert(false);

     defined=true;
   } // moves exist
 
   L->Destroy();
   delete L; 
   L = NULL;
}

/*
~FillFromRepTree~

This function fills the composite Array, the compositeSubMoveArray and the
periodic Move array. The linearPointArray must be initializes before calling
this function. The Parameters describe the first free index positions for
this tree. After alling this function, the arguments are incremented with the
space occupied by this tree.

*/ 
bool PMPoint::FillFromRepTree(int& cp,int& csp, int& pp, RepTree TR){
  int type = TR.GetNodeType();
  if(type==SIMPLE){
     // simple nodes correspond to linearpointmoves
     // for this reason, we have to do nothing in this case.
     return true;
  }
  int oldcp=cp;
  int oldcsp=csp;
  int oldpp=pp;
  
  if(type==REPETITION){
    // first include the son of this repetition
    pp++;
    RepTree* son = TR.GetSon(0);
    if(!FillFromRepTree(cp,csp,pp,*son)) // error
       return  false;
    // create a new SpatialPeriodicMove
    SpatialPeriodicMove SPM;
    int sontype = son->GetNodeType();
    if(sontype==SIMPLE){
      // we need the son for receiving some information
      const LinearPointMove* LPM;
      linearMoves.Get(son->Content(),LPM);
      SPM.repeatations = TR.RepCount();
      SPM.submove.arrayNumber=LINEAR;
      SPM.submove.arrayIndex=son->Content();
      SPM.interval.Equalize(&LPM->interval);
      SPM.interval.Mul(SPM.repeatations);
      SPM.bbox = LPM->bbox;
    } else if(sontype==COMPOSITE){
      const SpatialCompositeMove* SCM;
      compositeMoves.Get(oldcp,SCM); 
      SPM.repeatations = TR.RepCount();
      SPM.submove.arrayNumber=COMPOSITE;
      SPM.submove.arrayIndex=oldcp;
      SPM.interval.Equalize(&SCM->interval);
      SPM.interval.Mul(SPM.repeatations);
      SPM.bbox = SCM->bbox;    
    } else{
      // we don't allow other types to be a son of a 
      // repetition
      return false; 
    }
    periodicMoves.Put(oldpp,SPM);
    return true; 
  } // type==repetition

  if(type==COMPOSITE){
    int nos = TR.NumberOfSons();
    csp = csp + nos;  
    cp++;
    SpatialCompositeMove SCM;
    SCM.minIndex=oldcsp;
    SCM.maxIndex=csp-1;
    int currentrep=oldpp;
    // insert all submoves
    for(int i=0;i<nos;i++){
      RepTree* CurrentSon = TR.GetSon(i);
      //insert the son
      if(!FillFromRepTree(cp,csp,pp,*CurrentSon))
        return false;
      // create submove
      SubMove SM;
      int sontype = CurrentSon->GetNodeType();
      if(sontype==SIMPLE){
          SM.arrayNumber=LINEAR;
          SM.arrayIndex=CurrentSon->Content();
          const LinearPointMove* LPM;
          linearMoves.Get(CurrentSon->Content(),LPM);
          if(i==0){ // the first submove
             SCM.interval.Equalize(&LPM->interval);
             SCM.bbox.Equalize(&LPM->bbox);
          }else{
            SCM.interval.Plus(&(LPM->interval));
            SCM.bbox.Union(&LPM->bbox);
          }
       }else if(sontype==REPETITION){
          SM.arrayNumber=PERIOD;
          SM.arrayIndex=currentrep;
          currentrep=pp;
          const SpatialPeriodicMove* SPM;
          periodicMoves.Get(SM.arrayIndex,SPM);
          if(i==0){
            SCM.interval.Equalize(&SPM->interval);
            SCM.bbox.Equalize(&SPM->bbox);
          } else{
            SCM.interval.Plus(&SPM->interval);
            SCM.bbox.Union(&SPM->bbox);
          }

       } else{
         // we don't allow a composite move to be a
         // son of another composite move
         return false;
       }
       compositeSubMoves.Put(oldcsp+i,SM);
    } // for each submove
    compositeMoves.Put(oldcp,SCM); // insert the move
    return true;
  }
  // other type are not present
  assert(false);

}

/*
~GetBBox~

The function GetBBox returns the bounding Box of this periodic
moving point.

*/
PBBox PMPoint::GetBbox()const{
     __TRACE__
  return bbox;
}


/*
~Toprel~

This operator computed the topological relationship between this and 
the argument. 

*/
PMInt9M* PMPoint::Toprel(const Point P){
  __TRACE__
  // first, we create an array of the same size as the 
  // size of the linearmoves
  int rs = linearMoves.Size();
  ArrayRange ranges[rs];
  int lastPos=0;
  const LinearPointMove* LPM;
  DBArray<LinearInt9MMove> UnitTopRels(linearMoves.Size()); 
  LinearInt9MMove buffer[3]; 
  for(int i=0;i<linearMoves.Size();i++){
      linearMoves.Get(i,LPM);
      int count = LPM->Toprel(P,buffer);     
      for(int j=0;j<count;j++){
         UnitTopRels.Append(buffer[j]);  
      }
      ranges[i].minIndex=lastPos;
      ranges[i].maxIndex=lastPos+count-1;
      lastPos=lastPos+count;
  }
  DBArray<CompositeMove> CMs(compositeMoves.Size());
  DBArray<PeriodicMove> PMs(periodicMoves.Size());
  const SpatialCompositeMove* SCM;
  CompositeMove CM;
  for(int i=0;i<compositeMoves.Size();i++){
      compositeMoves.Get(i,SCM);
      CM = SCM->ToCompositeMove();   
      CMs.Put(i,CM);
  }
  const SpatialPeriodicMove* SPM;
  PeriodicMove PM;
  for(int i=0;i<periodicMoves.Size();i++){
     periodicMoves.Get(i,SPM);
     PM = SPM->ToPeriodicMove();
     PMs.Put(i,PM);
  } 
  PMInt9M* result = new PMInt9M(1);
  result->CreateFrom(UnitTopRels,ranges,rs,CMs,compositeSubMoves,
                     PMs,startTime,submove);
  result->Minimize();
  return result;
}


/*
~Toprel~

This operator computed the topological relationship between this and 
the argument. 

*/
PMInt9M* PMPoint::Toprel(Points& P){
  __TRACE__
  // first, we create an array of the same size as the 
  // size of the linearmoves
  int rs = linearMoves.Size();
  ArrayRange ranges[rs];
  int lastPos=0;
  const LinearPointMove* LPM;
  DBArray<LinearInt9MMove> UnitTopRels(linearMoves.Size()); 
  vector<LinearInt9MMove> MyBuffer; 
  for(int i=0;i<linearMoves.Size();i++){
      linearMoves.Get(i,LPM);
      LPM->Toprel(P,MyBuffer);
      int count = MyBuffer.size();    
      for(int j=0;j<count;j++){
         UnitTopRels.Append(MyBuffer[j]);  
      }
      ranges[i].minIndex=lastPos;
      ranges[i].maxIndex=lastPos+count-1;
      lastPos=lastPos+count;
  }
  DBArray<CompositeMove> CMs(compositeMoves.Size());
  DBArray<PeriodicMove> PMs(periodicMoves.Size());
  const SpatialCompositeMove* SCM;
  CompositeMove CM;
  for(int i=0;i<compositeMoves.Size();i++){
      compositeMoves.Get(i,SCM);
      CM = SCM->ToCompositeMove();   
      CMs.Put(i,CM);
  }
  const SpatialPeriodicMove* SPM;
  PeriodicMove PM;
  for(int i=0;i<periodicMoves.Size();i++){
     periodicMoves.Get(i,SPM);
     PM = SPM->ToPeriodicMove();
     PMs.Put(i,PM);
  } 
  PMInt9M* result = new PMInt9M(1);
  result->CreateFrom(UnitTopRels,ranges,rs,CMs,compositeSubMoves,PMs,
                     startTime,submove);
  cout << "Linear Moves before minimizations : " 
       << result->NumberOfLinearMoves() << endl;
  result->Minimize();
  cout << "Linear Moves after minimization : " 
       << result->NumberOfLinearMoves() << endl;
  return result;
}

/*
~DistanceTo~

This function computes the distance to a fixed position in [R2] as 
periodic moving real;


*/
bool PMPoint::DistanceTo(const double x, const double y, PMReal& result)const {
   // spcial case: this pmpoint is not defined
   if(!defined){
     result.SetDefined(false);
     return true; 
   }
   // copying the tree structure as well as the interval and the
   // submove into the result.
   RelInterval* resInterval = result.GetInterval();
   resInterval->Equalize(&interval);
   SubMove* SM =result.GetSubmove();
   SM->Equalize(&submove);
   

   // copy periodic moves
   DBArray<PeriodicMove>* resPMs = result.GetPeriodicMoves();   
   resPMs->Clear();
   int size;
   if((size=periodicMoves.Size())>0){
      PeriodicMove PM;
      const SpatialPeriodicMove* SPM;
      resPMs->Resize(size);
      for(int i=0;i<size;i++){
         periodicMoves.Get(i,SPM);
         SPM->ToPeriodicMove(PM);
         resPMs->Put(i,PM);
      }
   }
   // copy composite moves
   DBArray<CompositeMove>* resCMs = result.GetCompositeMoves();
   resCMs->Clear();
   if((size=compositeMoves.Size())>0){
       CompositeMove CM;
       const SpatialCompositeMove* SCM;
       resCMs->Resize(size);
       for(int i=0;i<size;i++){
          compositeMoves.Get(i,SCM);
          SCM->ToCompositeMove(CM);
          resCMs->Put(i,CM);    
       }
   }

   // copy composite submoves
   DBArray<SubMove>* resSMs = result.GetCompositeSubMoves();
   resSMs->Clear();
   if((size=compositeSubMoves.Size())>0){
      const SubMove* SM;
      resSMs->Resize(size);
      for(int i=0;i<size;i++){
        compositeSubMoves.Get(i,SM);
        resSMs->Put(i,*SM);
      }
   }
  
  // now, we build the linear moves for the 
  // periodic moving real
  DBArray<MovingRealUnit>* resLin = result.GetLinearMoves();
  MovingRealUnit Unit;
  resLin->Clear();
  if((size=linearMoves.Size())>0){
    resLin->Resize(size);
    const LinearPointMove* LPM;
    for(int i=0;i<size;i++){
      linearMoves.Get(i,LPM);
      LPM->DistanceTo(x,y,Unit);
      resLin->Put(i,Unit); 
    }
  }
  return true;
}

/*
3.9 Implementation of the class PMPoints

~Constructor~

This Constructor creates an undefined value of type PMPoints.
The argument is ignored. 

*/
PMPoints::PMPoints(int dummy):
   linearMoves(0),
   thePoints(0),
   compositeMoves(0),
   compositeSubMoves(0),
   periodicMoves(0),
   defined(false),
   canDelete(false),
   interval(0),
   startTime(instanttype),
   bbox(0)
{   __TRACE__
}

/*
~Destructor~

[3] O(1)

*/
PMPoints::~PMPoints(){
    __TRACE__
   if(canDelete){
      linearMoves.Destroy();
      thePoints.Destroy();
      compositeMoves.Destroy();
      compositeSubMoves.Destroy();
      periodicMoves.Destroy();
   }
}


/*
~Destroy~

[3] O(1)

*/
void PMPoints::Destroy(){
    __TRACE__
   canDelete=true;
}


/*
~Equalize~

This functions changes the value of this periodic moving point
to be equals to the given parameter.

[3] O(L), where L is the number of linear moves

*/
void PMPoints::Equalize(const PMPoints* P2){
    __TRACE__
 // equalize the linear moves
  const LinearPointsMove* LM;
  linearMoves.Clear();
  if(P2->linearMoves.Size()>0)
     linearMoves.Resize(P2->linearMoves.Size());
  for(int i=0;i<P2->linearMoves.Size();i++){
     P2->linearMoves.Get(i,LM);
     linearMoves.Append(*LM);
  }
  // equalize the contained points
  const TwoPoints* TP;
  thePoints.Clear();
  if(P2->thePoints.Size()>0)
     thePoints.Resize(P2->thePoints.Size());
  for(int i=0;i<P2->thePoints.Size();i++){
     P2->thePoints.Get(i,TP);
     thePoints.Append(*TP);
  }

  // equalize the composite moves
  const SpatialCompositeMove* CM;
  compositeMoves.Clear();
  if(compositeMoves.Size()>0)
     compositeMoves.Resize(P2->compositeMoves.Size());
  for(int i=0;i<P2->compositeMoves.Size();i++){
     P2->compositeMoves.Get(i,CM);
     compositeMoves.Append(*CM);
  }

  // equalize the composite submoves
  const SubMove* SM;
  compositeSubMoves.Clear();
  if(compositeSubMoves.Size()>0)
     compositeSubMoves.Resize(P2->compositeSubMoves.Size());
  for(int i=0;i<P2->compositeSubMoves.Size();i++){
     P2->compositeSubMoves.Get(i,SM);
     compositeSubMoves.Append(*SM);
  }

  // equalize the periodic moves
  const SpatialPeriodicMove* PM;
  periodicMoves.Clear();
  if(periodicMoves.Size()>0)
     periodicMoves.Resize(P2->periodicMoves.Size());
  for(int i=0;i<P2->periodicMoves.Size();i++){
     P2->periodicMoves.Get(i,PM);
     periodicMoves.Append(*PM);
  }

  defined = P2->defined;
  interval.Equalize(&(P2->interval));
  startTime.Equalize(&(P2->startTime));
  bbox.Equalize(&(P2->bbox));
  submove.Equalize(&(P2->submove));

}

/*
~NumOfFLOBs~

This function returns the number of contained FLOBs in this
class. Because five DBarrays are managed here, the return value
is 5.

[3] O(1)

*/
int PMPoints::NumOfFLOBs(){
    __TRACE__
   return 5;
}

/*
~GetFLOB~

This function returns the FLOB with index i.

[3] O(1)

*/
FLOB* PMPoints::GetFLOB(const int i){
    __TRACE__
  assert(i>=0 && i<NumOfFLOBs());
   if(i==0) return &linearMoves;
   if(i==1) return &thePoints;
   if(i==2) return &compositeMoves;
   if(i==3) return &compositeSubMoves;
   if(i==4) return &periodicMoves;
   return 0;
}

/*
~Compare~

This function is not implemented at this moment.

[3] O(-1)

*/
int PMPoints::Compare(const Attribute*)const{
    __TRACE__
  cout << " Warning! PMPoints::Compare  not implemented" << endl;
   return 0;
}

/*
~Adjacent~

We can't defined a adjacent relation beween two periodic moving
pointsets. For this reason, the return value is allways __false__.

[3] O(1)

*/
bool PMPoints::Adjacent(const Attribute* )const{
    __TRACE__
  return false;
}

/*
~Clone~

The ~Clone~ function returns a copy of this.

[3] O(L)

*/
PMPoints* PMPoints::Clone() const{
    __TRACE__
 PMPoints* copy = new PMPoints(0);
  copy->Equalize(this);
  return copy;
}


/*
~Sizeof~

This function returns the size of the PMPoints-class

[3] O(1)

*/
int PMPoints::Sizeof()const{
    __TRACE__
  return sizeof(PMPoints);
}

/*
~IsDefined~

This Function returns the defined state of a periodic moving point.

[3] O(1)

*/
bool PMPoints::IsDefined() const{
    __TRACE__
 return defined;
}

/*
~SetDefined~

Here the defined state of this points is set. If the argument has the
value __false__, the content of this will be lost. If the parameter
holds the value __true__, the call of this function has no effect.

[3] O(1)

*/
void PMPoints::SetDefined(const bool defined){
    __TRACE__
 if(defined) return;
  this->defined = false;
  linearMoves.Clear();
  compositeMoves.Clear();
  compositeSubMoves.Clear();
  periodicMoves.Clear();
  bbox.SetUndefined();
}

/*
~HashValue~

Returns the HashValue for this Point.

[3] O(1)

*/
size_t PMPoints::HashValue() const{
    __TRACE__
 DateTime* L = interval.GetLength();
  size_t res = (size_t) (bbox.Size()+(int)L->GetDay());
  delete L;
  L = NULL;
  return res;
}

/*
~CopyFrom~

The PMPoints instances takes its value from the given argument.

[3] O(L)

*/
void PMPoints::CopyFrom(const StandardAttribute* arg){
    __TRACE__
  Equalize((PMPoints*)arg);
}

/*
~ToListExpr~

This function returns the ListExpr representing this points.
The list will also contains the type if the flas is set. 
This means, this list
is a complete list in format (type value)

[3] O(L)

*/
ListExpr PMPoints::ToListExpr(const bool typeincluded)const{
    __TRACE__
   ListExpr value;
   if(!defined)
      value = nl->BoolAtom(false);
   else{   
      ListExpr timelist = startTime.ToListExpr(true);
      ListExpr SubMoveList = GetSubMoveList(submove);
      value = nl->TwoElemList(timelist,SubMoveList);
   }
   ListExpr res;
   if(typeincluded)
      res =  nl->TwoElemList( nl->SymbolAtom("pmpoints"),value );
   else
      res =  value;
   return res;
}

/*
~GetSubMove~

This functions determines the move from the given argument and
returns its nested list representation.

[3] O($L_{SM}$) , where $L_{SM}$ is the number of linear moves contained in SM

*/
ListExpr PMPoints::GetSubMoveList(const SubMove SM)const{
  __TRACE__
  ListExpr SubMoveList;
  int SubMoveType = SM.arrayNumber;
  int index = SM.arrayIndex;
  if(SubMoveType==LINEAR)
      SubMoveList = GetLinearMoveList(index);
  else if(SubMoveType==COMPOSITE)
      SubMoveList = GetSpatialCompositeMoveList(index);
  else if(SubMoveType==PERIOD)
      SubMoveList = GetSpatialPeriodicMoveList(index);
  else{
       cerr << __POS__ << " Error in creating ListExpr" << endl;
       SubMoveList = nl->TheEmptyList();
   }
  return SubMoveList;
}


/*
~GetLinearMove~

This functions returns the nested list representation of the
linear move at the specified index.

[3] O(1)

*/
ListExpr PMPoints::GetLinearMoveList(const int index)const{
    __TRACE__
   const LinearPointsMove* LM;
   linearMoves.Get(index,LM);
   ListExpr res =  LM->ToListExpr(thePoints);
   return res;
}

/*
~GetSpatialPeriodicMove~

This function converts the periodic move at the specified index
to its nested list representation.

[3] O($L_{P}$), where $L_{P}$ is the number of linear moves in the periodic move at index

*/
ListExpr PMPoints::GetSpatialPeriodicMoveList(const int index)const{
    __TRACE__
  const SpatialPeriodicMove* PM;
  periodicMoves.Get(index,PM);
  ListExpr periodtype = nl->SymbolAtom("period");
  ListExpr RepList = nl->IntAtom(PM->repeatations);
  ListExpr SML = GetSubMoveList(PM->submove);
  ListExpr LC = nl->BoolAtom(PM->interval.IsLeftClosed());
  ListExpr RC = nl->BoolAtom(PM->interval.IsRightClosed());
  return  nl->TwoElemList(periodtype,nl->FourElemList(RepList,LC,RC,SML));
}

/*
~GetSpatialCompositeMoveList~

This function returns the nested list representation of the composite
move at the specified array index.

[3] O(L) , where L is the number of submoves contained in the linear move at index

*/
ListExpr PMPoints::GetSpatialCompositeMoveList(const int index)const{
    __TRACE__
 const SpatialCompositeMove* CM;
 compositeMoves.Get(index,CM);
 ListExpr CType = nl->SymbolAtom("composite");
 int minIndex = CM->minIndex;
 int maxIndex = CM->maxIndex;
 ListExpr SubMovesList;
 if(maxIndex<minIndex){
    cerr << __POS__ << "empty composite move" << endl;
    SubMovesList = nl->TheEmptyList();
 }
 else{
   // construct the List of submoves
   const SubMove* SM;
   compositeSubMoves.Get(minIndex,SM);
   SubMovesList = nl->OneElemList(GetSubMoveList(*SM));
   ListExpr Last = SubMovesList;
   for(int i=minIndex+1;i<=maxIndex;i++){
     compositeSubMoves.Get(i,SM);
     Last = nl->Append(Last,GetSubMoveList(*SM));
   }
 }
 return nl->TwoElemList(CType,SubMovesList);
}


/*
~ReadFrom~

This function reads the value of this p.m. points from the
given nested list. If the nested list don't contains a
valid points value, the return value will be false and this points
is set to be undefined. Otherwise the points has the value
described in the nested list. The list consists only of the
the value, this means the type description is not included
in this list.

[3] O(L)  where L = number of linear moves

*/
bool PMPoints::ReadFrom(const ListExpr value){
    __TRACE__
 /* The list is scanned twice. In the first scan we
     compute only the needed size of the contained arrays. The reason is,
     that we want to avoid frequently ~Resize~ on the arrays to ensure the
     given time complexity.
  */

  // for Debugging only
  //nl->WriteListExpr(value);

if(nl->ListLength(value)!=2){
     if(DEBUG_MODE){
        cerr << __POS__ << ": wrong listlength (";
        cerr << (nl->ListLength(value)) << ")" << endl;
     }
     SetDefined(false);
     return false;
  }

  if(!ResizeArrays(value)){
     if(DEBUG_MODE){
        cerr << __POS__ << ": resizing arrays failed" << endl;
        cerr << "*************************************" << endl; 
     }
     SetDefined(false);
     return false;
  }

  if(!startTime.ReadFrom(nl->First(value),true)){
     if(DEBUG_MODE){
        cerr << __POS__ << "reading of the start time failed" << endl;
        cerr << "The list is " << endl;
     }
     SetDefined(false);
     return false;
  }
  // now we have to append the included submove
  ListExpr SML = nl->Second(value);
  if(nl->ListLength(SML)!=2){ // (submovetype value)
     if(DEBUG_MODE){
        cerr << __POS__ << ": wrong list length for submove" << endl;
     }
     SetDefined(false);
     return false;
  }


  ListExpr SMT = nl->First(SML);
  int LMIndex = 0;
  int PtsIndex = 0;
  int CMIndex = 0;
  int SMIndex = 0;
  int PMIndex = 0;
  bool res = false;
  if(nl->IsEqual(SMT,"linear")){
     submove.arrayNumber = LINEAR;
     submove.arrayIndex = 0;

     if(!AddLinearMove(nl->Second(SML),LMIndex,PtsIndex,
                       CMIndex,SMIndex,PMIndex)){
         if(DEBUG_MODE){
            cerr << __POS__ << " Error in reading linear move" << endl;
         }
         SetDefined(false);
         res = false;
     }else {
   defined=true;
   const LinearPointsMove* LM;
   linearMoves.Get(0,LM);
   interval.Equalize(&(LM->interval));
   bbox.Equalize(&(LM->bbox));
   res = true;
     }
  } else if(nl->IsEqual(SMT,"composite")){
     submove.arrayNumber=COMPOSITE;
     submove.arrayIndex = 0;
     if(!AddSpatialCompositeMove(nl->Second(SML),LMIndex,PtsIndex,
        CMIndex,SMIndex,PMIndex)){
        if(DEBUG_MODE){
           cerr << __POS__ << "error in reading composite move" << endl;
        }
        SetDefined(false);
        res = false;
     } else {
   defined = true;
   const SpatialCompositeMove* CM;
   compositeMoves.Get(0,CM);
   interval.Equalize(&(CM->interval));
   bbox.Equalize(&(CM->bbox));
   res = true;
     }   
  } else if(nl->IsEqual(SMT,"period")){
     submove.arrayNumber = PERIOD;
     submove.arrayIndex = 0;
     if(!AddPeriodMove(nl->Second(SML),LMIndex,PtsIndex,
                       CMIndex,SMIndex,PMIndex)){
        if(DEBUG_MODE){
          cerr << __POS__ << " error in reading periodic move" << endl;
        }
        SetDefined(false);
        res = false;
     } else {
   defined = true;
   const SpatialPeriodicMove* PM;
   periodicMoves.Get(0,PM);
   interval.Equalize(&(PM->interval));
   bbox.Equalize(&(PM->bbox));
   res = true;
    }
  } else {
      if(DEBUG_MODE){
   cerr << __POS__ << "unknown subtype" << endl;
   nl->WriteListExpr(SMT);
      }
      res = false;
  }

  return res;
}



/*
~ResizeArrays~

This function resizes the arrays to the needed values.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoints::ResizeArrays(const ListExpr value){
    __TRACE__
 // first all entries in the arrays are removed
   linearMoves.Clear();
   compositeMoves.Clear();
   compositeSubMoves.Clear();
   periodicMoves.Clear();
   int LMSize = 0;
   int PtsSize = 0;
   int CMSize = 0;
   int SMSize = 0;
   int PMSize = 0;
   if(!AddSubMovesSize(nl->Second(value),LMSize,PtsSize,CMSize,SMSize,PMSize))
      return false;
   // set the arrays to the needed size
   if(LMSize>0) linearMoves.Resize(LMSize);
   if(PtsSize>0) thePoints.Resize(PtsSize);
   if(CMSize>0) compositeMoves.Resize(CMSize);
   if(SMSize>0) compositeSubMoves.Resize(SMSize);
   if(PMSize>0) periodicMoves.Resize(PMSize);
   return true;
}

/*
~AddSubMovesSize~

This function computes the needed sizes for the arrays to hold the
value of the p.m. points represented in the value list.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoints::AddSubMovesSize(const ListExpr value,int &LMSize, int &PtsSize,
                              int &CMSize, int &SMSize,int &PMSize){
    __TRACE__
// all moves have the length 2
if(nl->ListLength(value)!=2)
   return false;
ListExpr type = nl->First(value);
if(nl->AtomType(type)!=SymbolType)
  return false;
  // in a linear move we have to increment the size of LM
  // and to add the number of contained Points 
  if(nl->IsEqual(type,"linear")){
     LMSize = LMSize +1;
     ListExpr val = nl->Second(value);
     if(nl->AtomType(val)!=NoAtom)
        return false;
     if(nl->ListLength(val)==2)
        if(nl->AtomType(nl->Second(val))==BoolType) // undefined 
           return true;
        else
     return false;
     if(nl->ListLength(val)!=3)
        return false;          
     if(nl->AtomType(nl->Second(val))!=NoAtom ||
        nl->AtomType(nl->Third(val))!=NoAtom )
  return false;
     int L1 = nl->ListLength(nl->Second(val));
     if(L1 != nl->ListLength(nl->Third(val)))
        return false;         
     PtsSize += L1; // add the Points of this Linear Move              
     return true;
  }
  if(nl->IsEqual(type,"composite")){
     CMSize = CMSize+1; // the composite move itself
     ListExpr rest = nl->Second(value);
     SMSize = SMSize+nl->ListLength(rest); // the contained submoves
     while(!nl->IsEmpty(rest)){
        if(!AddSubMovesSize(nl->First(rest),LMSize,PtsSize,
                      CMSize,SMSize,PMSize))
           return false;
        rest = nl->Rest(rest);
     }
     return true;
  }
  if(nl->IsEqual(type,"period")){
     PMSize = PMSize+1;
     int len = nl->ListLength(value);
     ListExpr PMove;
     if(len==2)
        PMove = nl->Second(value);
     else if(len==4)
        PMove = nl->Fourth(value);
     else // invalid listlength
        return false;
     return AddSubMovesSize(nl->Second(PMove),LMSize,PtsSize,
                            CMSize,SMSize,PMSize);
  }
  // a unknown type description
  return false;
}


/*
~AddLinearMove~

This functions appends the linear move given as nested list in __value__
to the LinearMoves -array.

[3] O(1)

*/
bool PMPoints::AddLinearMove(const ListExpr value, 
                             int &LMIndex, int &PtsIndex,
                             int &CMIndex, int &SMIndex, 
                             int &PMIndex){
    __TRACE__
  LinearPointsMove LM = LinearPointsMove(0);
   if(!LM.ReadFrom(value,thePoints,PtsIndex))
      return false;
   linearMoves.Put(LMIndex,LM);
   LMIndex++;
   return true;
}



/*
~AddSpatialCompositeMove~

This Functions appends the composite move given as a nested list together with
all contained submoves at the appropriate arrays. The return value indiciates
the success of this call.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoints::AddSpatialCompositeMove(const ListExpr value,int &LMIndex,
                   int &PtsIndex,int &CMIndex, int &SMIndex, int &PMIndex){
    __TRACE__
  // a composite move has to contains at least two submoves
   int len = nl->ListLength(value);
   if(len<2){
      if(DEBUG_MODE){
         cerr << __POS__ << " less than 2 submoves (" << len << ")" << endl;
      }
      return false;
   }
   SpatialCompositeMove CM=SpatialCompositeMove(1);
   int CMPos=CMIndex;
   // at this position we have to include this composite move
   int SMPos=SMIndex;
   // beginning at this position we have to include the submoves
   // ensure that no submove used the positions for this composite move
   CMIndex++;
   CM.bbox.SetUndefined();
   CM.minIndex=SMIndex;
   CM.maxIndex=SMIndex+len-1;
   SMIndex= SMIndex+len;
   // Append the contained Submoves
   ListExpr rest = value;
   ListExpr SML,TL,VL;
   bool isFirst = true;
   while(!nl->IsEmpty(rest)){
      SML = nl->First(rest);
      rest = nl->Rest(rest);
      if(nl->ListLength(SML)!=2){ // all submoves have the format (type value)
         if(DEBUG_MODE){
            cerr << __POS__ << " submove has wrong length (";
            cerr << nl->ListLength(SML) << ")" << endl;
         }
         return false;
      }
      TL = nl->First(SML);
      VL = nl->Second(SML);
      if(nl->IsEqual(TL,"linear")){
         // process a linear submove
         int LMPos = LMIndex;
         if(!AddLinearMove(VL,LMIndex,PtsIndex,CMIndex,SMIndex,PMIndex)){
            if(DEBUG_MODE){
               cerr << __POS__ << " can't add a linear move " << endl;
            }
            return false;
         }
         const LinearPointsMove* LM;
         linearMoves.Get(LMPos,LM);
         // Append the interval of LM to CM
         if(isFirst){
            isFirst=false;
            CM.interval.Equalize(&(LM->interval));
         }else{
            if(!CM.interval.Append(&(LM->interval))){
               if(DEBUG_MODE){
                   cerr << __POS__ << " can't append interval ";
                   cerr << endl;
                   cerr << "The original interval is";
                   cerr << CM.interval.ToString() << endl;
                   cerr << "The interval to append is";
                   cerr << LM->interval.ToString() << endl;
               }
               return false;
            }
         }
         // build the union of the bounding boxes of CM and LM
         CM.bbox.Union(&(LM->bbox));
         // put the submove in the array
         SubMove SM;
         SM.arrayNumber = LINEAR;
         SM.arrayIndex = LMPos;
         compositeSubMoves.Put(SMPos,SM);
         SMPos++;
      } else if(nl->IsEqual(TL,"period")){
        // process a periodic submove
        int PMPos = PMIndex;
        if(!AddPeriodMove(VL,LMIndex,PtsIndex,CMIndex,SMIndex,PMIndex)){
           if(DEBUG_MODE){
              cerr << __POS__ << "can't add period move " << endl;
            }
            return  false;
        }
        const SpatialPeriodicMove* PM;
        periodicMoves.Get(PMPos,PM);
        if(isFirst){
           isFirst=false;
           CM.interval.Equalize(&(PM->interval));
        }else{
           if(!CM.interval.Append(&(PM->interval))){
              if(DEBUG_MODE){
                 cerr << __POS__  << " can't append interval" << endl;
              }
              return false;
           }
        }
        CM.bbox.Union(&(PM->bbox));
        SubMove SM;
        SM.arrayNumber = PERIOD;
        SM.arrayIndex = PMPos;
        compositeSubMoves.Put(SMPos,SM);
        SMPos++;
   } else{ // not of type linear or period
      if(DEBUG_MODE){
         cerr << __POS__ << " submove not of type linear od period" << endl;
       }
       return false;
      }
   }
   // put the compositeMove itself
   compositeMoves.Put(CMPos,CM);
   return true;
}

/*
~AddPeriodMove~

This functions append the periodic move contained in the nested list
__value__ to this periodic moving point.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoints::AddPeriodMove(const ListExpr value,int &LMIndex, int &PtsIndex,
                             int &CMIndex, int &SMIndex, int &PMIndex){
   __TRACE__
 int len = nl->ListLength(value);
 if((len!=2) && (len!=4)){  // (repeatations <submove>)
    if(DEBUG_MODE)
       cerr << __POS__ << ": wrong listlength" << endl;
    return false;
 }
 if(nl->AtomType(nl->First(value))!=IntType){
    if(DEBUG_MODE){
       cerr << __POS__ << ": wrong type for repeatations" << endl;
    }
    return false;
 }
 int rep = nl->IntValue(nl->First(value));
 // rep must be greater than 1 
 if(rep<=1){
     if(DEBUG_MODE){
        cerr << __POS__ <<  " wrong number of repeatations" << endl;
     }
     return false;
 }
 
 ListExpr SML;
 if(len==2)
     SML = nl->Second(value);
 else
     SML = nl->Fourth(value);

 if(nl->ListLength(SML)!=2){
     if(DEBUG_MODE){
        cerr << __POS__ << ": wrong length for submove" << endl;
     }
     return false;
 }
 SpatialPeriodicMove PM=SpatialPeriodicMove(1);
 PM.repeatations = rep;
 int IncludePos = PMIndex; // store the positiuon
 PMIndex++;
 ListExpr SMT = nl->First(SML); // take the submove type
 if(nl->IsEqual(SMT,"linear")){
    int LMPos = LMIndex;
    if(!AddLinearMove(nl->Second(SML),LMIndex,PtsIndex,
                      CMIndex,SMIndex,PMIndex)){
       if(DEBUG_MODE){
          cerr << __POS__ << ": can't add linear submove" << endl;
       }
       return false;
    }
    PM.submove.arrayNumber = LINEAR;
    PM.submove.arrayIndex = LMPos;
    const LinearPointsMove* LM;
    linearMoves.Get(LMPos,LM);
    PM.bbox.Equalize(&(LM->bbox));
    RelInterval SMI = LM->interval;
    PM.interval.Equalize(&SMI);
    PM.interval.Mul(rep);
    if(len==4){
      ListExpr LC = nl->Second(value);
      ListExpr RC = nl->Third(value);
      if((nl->AtomType(LC)!=BoolType) || (nl->AtomType(RC)!=BoolType))
          return false;
      PM.interval.SetLeftClosed(nl->BoolValue(LC));
      PM.interval.SetRightClosed(nl->BoolValue(RC));

     }
    periodicMoves.Put(IncludePos,PM);
    return true;
 }else if(nl->IsEqual(SMT,"composite")){
    int CMPos = CMIndex;
    if(!AddSpatialCompositeMove(nl->Second(SML),LMIndex,PtsIndex,
                                CMIndex,SMIndex,PMIndex)){
       if(DEBUG_MODE){
          cerr << __POS__ << ": can't add composite submove" << endl;
       }
       return false;
    }
    PM.submove.arrayNumber = COMPOSITE;
    PM.submove.arrayIndex = CMPos;
    const SpatialCompositeMove* CM;
    compositeMoves.Get(CMPos,CM);
    PM.bbox.Equalize(&(CM->bbox));
    RelInterval SMI = CM->interval;
    PM.interval.Equalize(&SMI);
    PM.interval.Mul(rep);
    if(len==4){
      ListExpr LC = nl->Second(value);
      ListExpr RC = nl->Third(value);
      if((nl->AtomType(LC)!=BoolType) || (nl->AtomType(RC)!=BoolType))
          return false;
      PM.interval.SetLeftClosed(nl->BoolValue(LC));
      PM.interval.SetRightClosed(nl->BoolValue(RC));

     }
    periodicMoves.Put(IncludePos,PM);
    return true;
 }
 // not of type linear or composite
 if(DEBUG_MODE){
     cerr << __POS__ << ": invalid submove-type for periodic move" << endl;
     cerr << "This list is : " << endl;
     nl->WriteListExpr(SMT);
     cerr << endl << "end of list " << endl;
 }
 return false;

}


/*
~BreakPoints~

This function computes all locations where a point of this pointset has
halted.

*/
Points* PMPoints::Breakpoints(){
    DateTime DT(durationtype);
    return Breakpoints(&DT,false);
}
/*
~BreakPoints~

This function computes all locations where a point of this pointsset was
staying longer than duration. If the duration is less than zero or the 
duration is zero and inclusive is true, the result will be an undefined
points value.

*/
Points* PMPoints::Breakpoints(const DateTime* duration, const bool inclusive){
   Points* Res = new Points(1);
   if(!defined || !duration->IsDefined()){
       Res->SetDefined(false);
       return Res;
   }
   if(duration->LessThanZero()){
        Res->SetDefined(false);
        return Res;
   }
   if(duration->IsZero() && inclusive){
       Res->SetDefined(false);
       return Res;
   }
   const LinearPointsMove* LM;
   const TwoPoints* TP;
   Res->Clear();
   Res->StartBulkLoad();
   int size = linearMoves.Size();
   for(int i=0;i<size;i++){
      linearMoves.Get(i,LM);
      DateTime* L = LM->interval.GetLength();
      int cmp = L->CompareTo(duration);
      delete L;
      L = NULL;
      if(cmp>0 || (cmp==0 && inclusive)){
          unsigned int a = LM->GetStartIndex();
          unsigned int b = LM->GetEndIndex();
          for(unsigned int j = a; j<=b; j++){
              thePoints.Get(j,TP);
              if(TP->IsStatic()){
                Point P(true,TP->GetStartX(),TP->GetStartY());
                Res->InsertPt(P);
              }

          }
      }
   }
   Res->EndBulkLoad();
   return Res;
}

/*
~At~

This function computes the state of this pmpoints at time __instant__.
If this pmpoints is not defined at this instant, the result will be 
an undefined points object.

*/

Points* PMPoints::At(const DateTime* instant)const{
    __TRACE__
    DateTime* duration = new DateTime(instanttype);
    duration->Equalize(instant);
    duration->Minus(&startTime); // now it is really a duration
    Points* res;
    if(interval.Contains(duration)){
       const SubMove* sm;
       sm  = &submove;
       const SpatialCompositeMove* CM;
       const SpatialPeriodicMove* PM;
       RelInterval RI;
       // i have to find the linear move which is "active" at instant
       while(sm->arrayNumber!=LINEAR){
          if(sm->arrayNumber==COMPOSITE){
             // in this stage of implementation a linear search is
             // executed. i have to make it better in the future
             int i = sm->arrayIndex;
             compositeMoves.Get(i,CM);
             int cur = CM->minIndex;
             int max = CM->maxIndex;
             bool found=false;
             while( (cur<=max) && ! found){
                compositeSubMoves.Get(cur,sm); // get the submove
                if(sm->arrayNumber==LINEAR){
                   const LinearPointsMove*  LM;
                   linearMoves.Get(sm->arrayIndex,LM);
                   RI = LM->interval;
                } else if(sm->arrayNumber==PERIOD){
                   periodicMoves.Get(sm->arrayIndex,PM);
                   RI = PM->interval;
                } else { //another submoves are not allowed
                   assert(false);
                }
                if(RI.Contains(duration)) // be happy
                   found=true;
                else{  // search again
                   DateTime* L = RI.GetLength();
                   duration->Minus(L);
                   delete L;
                   L = NULL;
                   cur++;
                }
             }
             assert(found); //otherwise we have an error in computation
          } else if(sm->arrayNumber==PERIOD){
             int index = sm->arrayIndex;
             periodicMoves.Get(index,PM);
             // this is a very slow implementation
             // i have to speed up it in the future
             sm = &PM->submove;
             RelInterval RI;
             if(sm->arrayNumber==LINEAR){
                 const LinearPointsMove* LM;
                 linearMoves.Get(sm->arrayIndex,LM);
                 RI = LM->interval;
             } else if(sm->arrayNumber==COMPOSITE){
                 compositeMoves.Get(sm->arrayIndex,CM);
                 RI = CM->interval;
             } else { //another submoves are not allowed
                 assert(false);
             }
             while(!RI.Contains(duration)){
                DateTime* L = RI.GetLength();
                duration->Minus(L);
                delete L;
                L = NULL;
             }
          } else{
             // this case should never occurs
             assert(false);
          }
       }
       const LinearPointsMove* LM;
       linearMoves.Get(sm->arrayIndex,LM);
       if(LM->IsDefinedAt(duration))
          res=LM->At(duration,thePoints);
       else
          res = new Points(false);
    } else { // the duration does not contains the argument instant
         res = new Points(false);
    }
    delete duration;
    duration = NULL;
    return res;
}

namespace periodic{


/*
4 Implementing Type Constructors 

4.1 ListConversions

4.1.1 Out functions

~ Outfunction for PBBox~

*/
ListExpr OutPBBox(ListExpr typeInfo,Word value){
    __TRACE__
   return ((PBBox*) value.addr)->ToListExpr();
}


/*
~Out Function for RelInterval~

*/
ListExpr OutRelInterval( ListExpr typeInfo, Word value ){
    __TRACE__
  RelInterval* D = (RelInterval*) value.addr;
   return D->ToListExpr(false);
}

/*
~Out Function for Interval~

*/
ListExpr OutPInterval( ListExpr typeInfo, Word value ){
    __TRACE__
  PInterval* I = (PInterval*) value.addr;
   return I->ToListExpr(false);
}

/*
~Out-Function for PMPoint~

*/
ListExpr OutPMPoint( ListExpr typeInfo, Word value ){
    __TRACE__
  PMPoint* P = (PMPoint*) value.addr;
   return P->ToListExpr(false);
}

/*
~Out-Function for PMPoints~

*/
ListExpr OutPMPoints(ListExpr typeInfo, Word value){
    __TRACE__
   PMPoints* P = (PMPoints*) value.addr;
   ListExpr res =  P->ToListExpr(false);
   return res;
}


/*
~Out-Functions for PMBool~

*/
ListExpr OutPMBool( ListExpr typeInfo, Word value ){
   __TRACE__
   PMBool* B = (PMBool*) value.addr;
   return B->ToListExpr();
}

/*
~Out-Functions for PMInt9M~

*/
ListExpr OutPMInt9M( ListExpr typeInfo, Word value ){
   __TRACE__
   PMInt9M* B = (PMInt9M*) value.addr;
   return B->ToListExpr();
}


/*
~Out-Functions for PMReal~

*/
ListExpr OutPMReal( ListExpr typeInfo, Word value ){
   __TRACE__
   PMReal* B = (PMReal*) value.addr;
   return B->ToListExpr();
}




/*
4.1.2 In functions

~In-Function for PBBox~

*/
Word InPBBox( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct ){
  __TRACE__
  PBBox* B = new PBBox(0);
  if(B->ReadFrom(instance)){
    correct=true;
    return SetWord(B);
  }
  correct = false;
  delete B;
  B = NULL;
  return SetWord(Address(0));
}

/*
~ In-Function for RelInterval~

*/
Word InRelInterval( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct ){
  __TRACE__
  RelInterval* D = new RelInterval(0);
  if(D->ReadFrom(instance,false)){
    correct=true;
    return SetWord(D);
  }
  correct = false;
  delete D;
  D = NULL;
  return SetWord(Address(0));
}

/*
~In Function for PInterval~

*/
Word InPInterval( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct ){
  __TRACE__
  PInterval* I = new PInterval(0);
  if(I->ReadFrom(instance,false)){
    correct=true;
    return SetWord(I);
  }
  correct = false;
  delete I;
  I = NULL;
  return SetWord(Address(0));
}

/*
~In-Function for PMPoint~

*/
Word InPMPoint( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct ){
  __TRACE__
  PMPoint* P = new PMPoint(0);
  if(P->ReadFrom(instance)){
    correct=true;
    P->CheckCorrectness();
    return SetWord(P);
  }
  correct = false;
  delete P;
  P = NULL;
  return SetWord(Address(0));
}

/*
~In-Function for PMPoints~

*/
Word InPMPoints(const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr &errorInfo, bool &correct){
  __TRACE__ 
  PMPoints* P = new PMPoints(0);
  if(P->ReadFrom(instance)){
    correct=true;
    return SetWord(P);
  }
  correct = false;
  delete P;
  P = NULL;
  return SetWord(Address(0));    
}       


/*
~In-Function for PMBool~

*/
Word InPMBool( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct ){
  __TRACE__
  PMBool* B = new PMBool(0);
  if(B->ReadFrom(instance)){
    correct=true;
    return SetWord(B);
  }
  correct = false;
  delete B;
  B = NULL;
  return SetWord(Address(0));
}

/*
~In-Function for PMInt9M~

*/
Word InPMInt9M( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct ){
  __TRACE__
  PMInt9M* B = new PMInt9M(0);
  if(B->ReadFrom(instance)){
    correct=true;
    return SetWord(B);
  }
  correct = false;
  delete B;
  B = NULL;
  return SetWord(Address(0));
}

/*
~In-Function for PMReal~

*/
Word InPMReal( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct ){
  __TRACE__
  PMReal* B = new PMReal(0);
  if(B->ReadFrom(instance)){
    correct=true;
    return SetWord(B);
  }
  correct = false;
  delete B;
  B = NULL;
  return SetWord(Address(0));
}

/*
4.2  ~Property~ Functions

The following functions describe the types of the 
periodic algebra.

*/
ListExpr PBBoxProperty(){
  __TRACE__
  return (nl->TwoElemList(
            nl->FiveElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"),
                nl->StringAtom("Remarks")),
            nl->FiveElemList(
                nl->StringAtom("-> Data"),
                nl->StringAtom("pbbox"),
                nl->StringAtom("(minx miny maxx maxy)"),
                nl->StringAtom("(12.0 23.3  100.987 5245.978)"),
                nl->StringAtom("All numeric values are valid."))
         ));
}


ListExpr RelIntervalProperty(){
  __TRACE__
  return (nl->TwoElemList(
            nl->FiveElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"),
                nl->StringAtom("Remarks")),
            nl->FiveElemList(
                nl->StringAtom("-> Data"),
                nl->StringAtom("rinterval"),
                nl->StringAtom("(<datetime> lC rC lI rI)"),
                nl->StringAtom("((time (2 120000)) TRUE FALSE FALSE FALSE)"),
                nl->StringAtom("a interval without fixed start"))
         ));
}

ListExpr PIntervalProperty(){
  __TRACE__
  return (nl->TwoElemList(
        nl->FiveElemList(
            nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
            nl->StringAtom("List Rep"),
            nl->StringAtom("Example List"),
            nl->StringAtom("Remarks")),
        nl->FiveElemList(
            nl->StringAtom("-> Data"),
            nl->StringAtom("pinterval"),
            nl->StringAtom("(<instant> <instant> leftClosed rightClosed)"),
            nl->StringAtom("((instant 1.1)(instant 1.5) TRUE FALSE)"),
            nl->StringAtom(""))
     ));
}


ListExpr PMPointProperty(){
  __TRACE__
  return (nl->TwoElemList(
            nl->FiveElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"),
                nl->StringAtom("Remarks")),
            nl->FiveElemList(
                nl->StringAtom("-> Data"),
                nl->StringAtom("pmpoint"),
                nl->StringAtom("(<startTime> <submove>)"),
                nl->StringAtom("..."),
                nl->StringAtom("see in the documentation"))
         ));
}

ListExpr PMPointsProperty(){
  __TRACE__
  return (nl->TwoElemList(
            nl->FiveElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"),
                nl->StringAtom("Remarks")),
            nl->FiveElemList(
                nl->StringAtom("-> Data"),
                nl->StringAtom("pmpoints"),
                nl->StringAtom("(<startTime> <submove>)"),
                nl->StringAtom("..."),
                nl->StringAtom("see in the documentation"))
         ));
}

ListExpr PMBoolProperty(){
  __TRACE__
  return (nl->TwoElemList(
            nl->FiveElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"),
                nl->StringAtom("Remarks")),
            nl->FiveElemList(
                nl->StringAtom("-> Data"),
                nl->StringAtom("pmbool"),
                nl->StringAtom("(<startTime> <submove>)"),
                nl->StringAtom(" ... "),
                nl->StringAtom("see in the documentation"))
         ));
}

ListExpr PMInt9MProperty(){
  __TRACE__
  return (nl->TwoElemList(
            nl->FiveElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"),
                nl->StringAtom("Remarks")),
            nl->FiveElemList(
                nl->StringAtom("-> Data"),
                nl->StringAtom("pmint9m"),
                nl->StringAtom("(<startTime> <submove>)"),
                nl->StringAtom(" ... "),
                nl->StringAtom("see in the documentation"))
         ));
}


ListExpr PMRealProperty(){
  __TRACE__
  return (nl->TwoElemList(
            nl->FiveElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"),
                nl->StringAtom("Remarks")),
            nl->FiveElemList(
                nl->StringAtom("-> Data"),
                nl->StringAtom("pmreal"),
                nl->StringAtom("(<startTime> <submove>)"),
                nl->StringAtom(" ... "),
                nl->StringAtom("see in the documentation"))
         ));
}
/*
4.3 ~Create~ Functions

The following functions are used to create instances of the 
types of this algebra. 

*/
Word CreatePBBox(const ListExpr typeInfo){
  __TRACE__
  return SetWord(new PBBox(1));
}

Word CreateRelInterval(const ListExpr typeInfo){
  __TRACE__
  return SetWord(new RelInterval(1));
}

Word CreatePInterval(const ListExpr typeInfo){
  __TRACE__
  return SetWord(new PInterval(1));
}

Word CreatePMPoint(const ListExpr typeInfo){
  __TRACE__
  return SetWord(new PMPoint(1));
}

Word CreatePMPoints(const ListExpr typeInfo){
  __TRACE__
  return SetWord(new PMPoints(1));
}

Word CreatePMBool(const ListExpr typeInfo){
  __TRACE__
  return SetWord(new PMBool(1));
}

Word CreatePMInt9M(const ListExpr typeInfo){
  __TRACE__
  return SetWord(new PMInt9M(1));
}

Word CreatePMReal(const ListExpr typeInfo){
  __TRACE__
  return SetWord(new PMReal(1));
}

/*
4.4. ~Delete~ Functions

The ~delete~ functions can be used to destroy the 
instances of types of this algebra.

*/
void DeletePBBox(const ListExpr typeInfo,Word &w){
  __TRACE__
  PBBox* B = (PBBox*) w.addr;
  delete B;
  B = NULL;
  w.addr=0;
}


void DeleteRelInterval(const ListExpr typeInfo,Word &w){
  __TRACE__
  RelInterval* D = (RelInterval*) w.addr;
  D->Destroy();
  delete D;
  D = NULL;
  w.addr=0;
}

void DeletePInterval(const ListExpr typeInfo,Word &w){
  __TRACE__
  PInterval* I = (PInterval*) w.addr;
  I->Destroy();
  delete I;
  I = NULL;
  w.addr=0;
}

void DeletePMPoint(const ListExpr typeInfo,Word &w){
  __TRACE__
  PMPoint* P = (PMPoint*) w.addr;
  P->Destroy();
  delete P;
  P = NULL;
  w.addr=0;
}

void DeletePMPoints(const ListExpr typeInfo,Word &w){
  __TRACE__
  PMPoints* P = (PMPoints*) w.addr;
  P->Destroy();
  delete P;
  P = NULL;
  w.addr=0;
}

void DeletePMBool(const ListExpr typeInfo,Word &w){
  __TRACE__
  PMBool* B = (PMBool*) w.addr;
  B->Destroy();
  delete B;
  B = NULL;
  w.addr=0;
}

void DeletePMInt9M(const ListExpr typeInfo,Word &w){
  __TRACE__
  PMInt9M* B = (PMInt9M*) w.addr;
  B->Destroy();
  delete B;
  B = NULL;
  w.addr=0;
}

void DeletePMReal(const ListExpr typeInfo,Word &w){
  __TRACE__
  PMReal* B = (PMReal*) w.addr;
  B->Destroy();
  delete B;
  B = NULL;
  w.addr=0;
}

/*
4.5 ~Open~ Functions

The open functions are called to read objects from 
a SmiRecord.

*/
bool OpenPBBox( SmiRecord& valueRecord,
                size_t& offset, 
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  PBBox *bb = (PBBox*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord( bb );
  return true;
}

bool OpenRelInterval( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  RelInterval *ri = (RelInterval*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord( ri );
  return true;
}

bool OpenPInterval( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  PInterval *e = (PInterval*)Attribute::Open( valueRecord, offset,
                                                  typeInfo );
  value = SetWord(e);
  return true;
}

bool OpenPMPoint( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  PMPoint *e = (PMPoint*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord(e);
  return true;
}

bool OpenPMPoints( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  PMPoints *e = (PMPoints*)Attribute::Open( valueRecord, offset,typeInfo);
  value = SetWord(e);
  return true;
}


bool OpenPMBool( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  PMBool *e = (PMBool*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord(e);
  return true;
}

bool OpenPMInt9M( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  PMInt9M *e = (PMInt9M*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord( e );
  return true;
}

bool OpenPMReal( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  PMReal *e = (PMReal*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord( e );
  return true;
}


/*
4.6 ~Save~ Functions

The save functions are used by the query processor to store objects in
SmiRecords.

*/
bool SavePBBox( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  PBBox* B = (PBBox *)value.addr;
  Attribute::Save( valueRecord,offset, typeInfo,B );
  return true;
}

bool SaveRelInterval( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  RelInterval* D = (RelInterval *)value.addr;
  Attribute::Save( valueRecord,offset, typeInfo,D );
  return true;
}

bool SavePInterval( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  PInterval* I = (PInterval *)value.addr;
  Attribute::Save( valueRecord,offset, typeInfo,I );
  return true;
}

bool SavePMPoint( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  PMPoint* P = (PMPoint *)value.addr;
  Attribute::Save( valueRecord, offset, typeInfo,P );
  return true;
}

bool SavePMPoints( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  PMPoints* P = (PMPoints *)value.addr;
  Attribute::Save( valueRecord,offset, typeInfo,P );
  return true;
}

bool SavePMBool( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  PMBool* B = (PMBool *)value.addr;
  Attribute::Save( valueRecord, offset,typeInfo,B );
  return true;
}

bool SavePMInt9M( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value ){
  __TRACE__
  PMInt9M* B = (PMInt9M *)value.addr;
  Attribute::Save( valueRecord, offset,typeInfo,B );
  return true;
}

bool SavePMReal( SmiRecord& valueRecord,
                 size_t& offset,
                 const ListExpr typeInfo,
                 Word& value ){
  __TRACE__
  PMReal* B = (PMReal *)value.addr;
  Attribute::Save( valueRecord, offset,typeInfo,B );
  return true;
}

/*
4.7 ~Close~-Functions

The close functions destroy the instances.

*/
void ClosePBBox(const ListExpr typeInfo, Word& w ){
  __TRACE__
  delete (PBBox *)w.addr;
  w.addr = 0;
}

void CloseRelInterval(const ListExpr typeInfo, Word& w ){
    __TRACE__
 delete (RelInterval *)w.addr;
  w.addr = 0;
}

void ClosePInterval(const ListExpr typeInfo, Word& w ){
    __TRACE__
 delete (PInterval *)w.addr;
  w.addr = 0;
}

void ClosePMPoint(const ListExpr typeInfo, Word& w ){
    __TRACE__
 delete (PMPoint *)w.addr;
  w.addr = 0;
}

void ClosePMPoints(const ListExpr typeInfo, Word& w ){
    __TRACE__
 delete (PMPoints *)w.addr;
  w.addr = 0;
}

void ClosePMBool(const ListExpr typeInfo, Word& w ){
    __TRACE__
 delete (PMBool *)w.addr;
  w.addr = 0;
}

void ClosePMInt9M(const ListExpr typeInfo, Word& w ){
    __TRACE__
  delete (PMInt9M *)w.addr;
  w.addr = 0;
}

void ClosePMReal(const ListExpr typeInfo, Word& w ){
    __TRACE__
  delete (PMReal *)w.addr;
  w.addr = 0;
}

/*
4.8 ~Clone~-Functions

The clone functions can be used for get a copy
of an existing object.

*/
Word ClonePBBox(const ListExpr typeInfo, const Word& w )
{
     __TRACE__
return SetWord( ((PBBox *)w.addr)->Clone() );
}

Word CloneRelInterval(const ListExpr typeInfo, const Word& w )
{
     __TRACE__
return SetWord( ((RelInterval *)w.addr)->Clone() );
}

Word ClonePInterval(const ListExpr typeInfo, const Word& w )
{
    __TRACE__
 return SetWord( ((PInterval *)w.addr)->Clone() );
}

Word ClonePMPoint(const ListExpr typeInfo, const Word& w )
{
    __TRACE__
 return SetWord( ((PMPoint *)w.addr)->Clone() );
}

Word ClonePMPoints(const ListExpr typeInfo, const Word& w )
{
    __TRACE__
 return SetWord( ((PMPoints *)w.addr)->Clone() );
}

Word ClonePMBool(const ListExpr typeInfo, const Word& w )
{
    __TRACE__
 return SetWord( ((PMBool *)w.addr)->Clone() );
}

Word ClonePMInt9M(const ListExpr typeInfo, const Word& w )
{
    __TRACE__
 return SetWord( ((PMInt9M *)w.addr)->Clone() );
}

Word ClonePMReal(const ListExpr typeInfo, const Word& w )
{
    __TRACE__
 return SetWord( ((PMReal *)w.addr)->Clone() );
}

/*
4.9 ~SizeOf~-Functions

This functions must be provided for allocating the
correct memory space for an object in query processing.

*/
int SizeOfPBBox(){
    __TRACE__
 return sizeof(PBBox);
}

int SizeOfRelInterval(){
    __TRACE__
 return sizeof(RelInterval);
}

int SizeOfPInterval(){
    __TRACE__
 return sizeof(PInterval);
}

int SizeOfPMPoint(){
    __TRACE__
 return sizeof(PMPoint);
}

int SizeOfPMPoints(){
    __TRACE__
 return sizeof(PMPoints);
}

int SizeOfPMBool(){
    __TRACE__
 return sizeof(PMBool);
}

int SizeOfPMInt9M(){
    __TRACE__
 return sizeof(PMInt9M);
}

int SizeOfPMReal(){
    __TRACE__
 return sizeof(PMReal);
}

/*
4.10 ~Cast~-Functions

Some functions proving dynamic casts.

*/
void* CastPBBox( void* addr )
{
    __TRACE__
 return new (addr) PBBox;
}

void* CastRelInterval( void* addr )
{
    __TRACE__
 return new (addr) RelInterval;
}

void* CastPInterval( void* addr )
{
    __TRACE__
 return new (addr) PInterval;
}

void* CastPMPoint( void* addr )
{
    __TRACE__
 return new (addr) PMPoint;
}

void* CastPMPoints( void* addr )
{
    __TRACE__
 return new (addr) PMPoints;
}

void* CastPMBool( void* addr )
{
    __TRACE__
 return new (addr) PMBool;
}

void* CastPMInt9M( void* addr )
{
    __TRACE__
 return new (addr) PMInt9M;
}

void* CastPMReal( void* addr )
{
    __TRACE__
 return new (addr) PMReal;
}

/*
4.11 Kind Checking Functions

This function checks whether the type constructor is applied correctly. Since
all type constructors don't have arguments, this is trivial.

*/
bool CheckPBBox( ListExpr type, ListExpr& errorInfo )
{
    __TRACE__
 return (nl->IsEqual( type, "pbbox" ));
}

bool CheckRelInterval( ListExpr type, ListExpr& errorInfo )
{
    __TRACE__
 return (nl->IsEqual( type, "rinterval" ));
}

bool CheckPInterval( ListExpr type, ListExpr& errorInfo )
{
    __TRACE__
 return (nl->IsEqual( type, "pinterval" ));
}

bool CheckPMPoint( ListExpr type, ListExpr& errorInfo )
{
    __TRACE__
 return (nl->IsEqual( type, "pmpoint" ));
}

bool CheckPMPoints( ListExpr type, ListExpr& errorInfo )
{
    __TRACE__
  return (nl->IsEqual( type, "pmpoints" ));
}

bool CheckPMBool( ListExpr type, ListExpr& errorInfo )
{
    __TRACE__
 return (nl->IsEqual( type, "pmbool" ));
}

bool CheckPMInt9M( ListExpr type, ListExpr& errorInfo )
{
    __TRACE__
 return (nl->IsEqual( type, "pmint9m" ));
}

bool CheckPMReal( ListExpr type, ListExpr& errorInfo )
{
    __TRACE__
  return (nl->IsEqual( type, "pmreal" ));
}

/*
5. TypeConstructors

In this section, the type constructors for using in the 
algebra are defined. 

*/
TypeConstructor pbbox(
        "pbbox",            //name
        PBBoxProperty,      //property function describing signature
        OutPBBox, InPBBox,  //Out and In functions
        0,                  //SaveToList and
        0,                  //RestoreFromList functions
        CreatePBBox, DeletePBBox, //object creation and deletion
        OpenPBBox,    SavePBBox,  //object open and save
        ClosePBBox,  ClonePBBox,  //object close and clone
        CastPBBox,                //cast function
        SizeOfPBBox,              //sizeof function
        CheckPBBox               //kind checking function
    );

TypeConstructor relinterval(
        "rinterval",                //name
        RelIntervalProperty,        //property function describing signature
        OutRelInterval, InRelInterval, //Out and In functions
        0,                             //SaveToList and
        0,                             //RestoreFromList functions
        CreateRelInterval, DeleteRelInterval, //object creation and deletion
        OpenRelInterval, SaveRelInterval, //object open and save
        CloseRelInterval,  CloneRelInterval, //object close and clone
        CastRelInterval,      //cast function
        SizeOfRelInterval,    //sizeof function
        CheckRelInterval     //kind checking function
    );

TypeConstructor pinterval(
        "pinterval",                //name
        PIntervalProperty,        //property function describing signature
        OutPInterval, InPInterval, //Out and In functions
        0,                             //SaveToList and
        0,                             //RestoreFromList functions
        CreatePInterval, DeletePInterval, //object creation and deletion
        OpenPInterval, SavePInterval, //object open and save
        ClosePInterval,  ClonePInterval, //object close and clone
        CastPInterval,      //cast function
        SizeOfPInterval,    //sizeof function
        CheckPInterval     //kind checking function
  );

TypeConstructor pmpoint(
        "pmpoint",             //name
        PMPointProperty,       //property function describing signature
        OutPMPoint, InPMPoint, //Out and In functions
        0,                     //SaveToList and
        0,                     // RestoreFromList functions
        CreatePMPoint, DeletePMPoint, //object creation and deletion
        0,0,
        //OpenPMPoint,    SavePMPoint, //object open and save
        ClosePMPoint,  ClonePMPoint,  //object close and clone
        CastPMPoint,                  //cast function
        SizeOfPMPoint,                //sizeof function
        CheckPMPoint                //kind checking function
    );

TypeConstructor pmpoints(
        "pmpoints",             //name
        PMPointsProperty,       //property function describing signature
        OutPMPoints, InPMPoints, //Out and In functions
        0,                     //SaveToList and
        0,                     // RestoreFromList functions
        CreatePMPoints, DeletePMPoints, //object creation and deletion
        0,0,
        //OpenPMPoints,    SavePMPoints , //object open and save
        ClosePMPoints,  ClonePMPoints,  //object close and clone
        CastPMPoints,                  //cast function
        SizeOfPMPoints,                //sizeof function
        CheckPMPoints                //kind checking function
     );

TypeConstructor pmbool(
        "pmbool",            //name
        PMBoolProperty,      //property function describing signature
        OutPMBool, InPMBool, //Out and In functions
        0,                   //SaveToList and
        0,                   //RestoreFromList functions
        CreatePMBool, DeletePMBool,//object creation and deletion
        0,0,
        //OpenPMBool,    SavePMBool, //object open and save
        ClosePMBool,  ClonePMBool,  //object close and clone
        CastPMBool,                 //cast function
        SizeOfPMBool,               //sizeof function
        CheckPMBool                //kind checking function
     );

TypeConstructor pmint9m(
        "pmint9m",            //name
        PMInt9MProperty,      //property function describing signature
        OutPMInt9M, InPMInt9M, //Out and In functions
        0,                   //SaveToList and
        0,                   //RestoreFromList functions
        CreatePMInt9M, DeletePMInt9M,//object creation and deletion
        0,0,
        //OpenPMInt9M,    SavePMInt9M, //object open and save
        ClosePMInt9M,  ClonePMInt9M,  //object close and clone
        CastPMInt9M,                 //cast function
        SizeOfPMInt9M,               //sizeof function
        CheckPMInt9M                //kind checking function
    );


TypeConstructor pmreal(
        "pmreal",            //name
        PMRealProperty,      //property function describing signature
        OutPMReal, InPMReal, //Out and In functions
        0,                   //SaveToList and
        0,                   //RestoreFromList functions
        CreatePMReal, DeletePMReal,//object creation and deletion
        //0,0,
        OpenPMReal, SavePMReal, //object open and save
        ClosePMReal,  ClonePMReal,  //object close and clone
        CastPMReal,                 //cast function
        SizeOfPMReal,               //sizeof function
        CheckPMReal                //kind checking function
     );

/*
5 Implementing Operators

4.1 Type Mappings

A type mapping checks the signature of an operator. This must be done
very carefully. A wrong implementation here will lead to a crash
of secondo.

*/
ListExpr PBBoxPBBoxBoolTypeMap(ListExpr args){
    __TRACE__
  if(nl->ListLength(args)==2) {
        if(nl->IsEqual(nl->First(args),"pbbox") &&
           nl->IsEqual(nl->First(args),"pbbox"))
          return nl->SymbolAtom("bool");
        else
          ErrorReporter::ReportError("Two values of type pbbox expected\n");
        return nl->SymbolAtom(TYPE_ERROR);
   }
   ErrorReporter::ReportError("Wrong number of arguments \n");
   return nl->SymbolAtom(TYPE_ERROR);
}

ListExpr PBBoxPBBoxPBBoxTypeMap(ListExpr args){
    __TRACE__
  if(nl->ListLength(args)==2) {
        if(nl->IsEqual(nl->First(args),"pbbox") &&
           nl->IsEqual(nl->Second(args),"pbbox"))
           return nl->SymbolAtom("pbbox");
        ErrorReporter::ReportError("Two Elements of type pbbox expected\n");
        return nl->SymbolAtom(TYPE_ERROR);
   }
   ErrorReporter::ReportError("Wrong number of arguments\n");
   return nl->SymbolAtom(TYPE_ERROR);
}

ListExpr PIntervalInstantTypeMap(ListExpr args){
    __TRACE__
  if(nl->ListLength(args)!=1){
       ErrorReporter::ReportError("Invalid number of arguments\n");
       return nl->SymbolAtom(TYPE_ERROR);
  }
  if(nl->IsEqual(nl->First(args),"pinterval"))
       return nl->SymbolAtom("instant");
  ErrorReporter::ReportError("value of type pinterval expected\n");
   return nl->SymbolAtom(TYPE_ERROR);
}

ListExpr EqualsTypeMap(ListExpr args){
    __TRACE__
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("two arguments expected\n");
     return nl->SymbolAtom(TYPE_ERROR);
  }
  if(nl->IsEqual(nl->First(args),"pbbox") &&
     nl->IsEqual(nl->Second(args),"pbbox"))
     return nl->SymbolAtom("bool");
  if(nl->IsEqual(nl->First(args),"pinterval") &&
     nl->IsEqual(nl->Second(args),"pinterval"))
     return nl->SymbolAtom("bool");

  ErrorReporter::ReportError("Invalid arguments \n");
  return nl->SymbolAtom(TYPE_ERROR);
}

ListExpr AtTypeMap(ListExpr args){
    __TRACE__
  if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("At expects two arguments\n");
      return nl->SymbolAtom(TYPE_ERROR);
   }
   if(!nl->IsEqual(nl->Second(args),"instant")){
     ErrorReporter::ReportError("The second argument must be an instant\n");
     return nl->SymbolAtom(TYPE_ERROR);
   }
   if(!nl->AtomType(nl->First(args))==SymbolType){
      ErrorReporter::ReportError("At can't handle composite types \n");
      return nl->SymbolAtom(TYPE_ERROR);
   }
   string arg = nl->SymbolValue(nl->First(args));

   if(arg=="pmpoint")
     return nl->SymbolAtom("point");
   if(arg=="pmbool")
     return nl->SymbolAtom("bool");
   if(arg=="pmint9m")
     return nl->SymbolAtom("int9m");
   if(arg=="pmpoints")
     return nl->SymbolAtom("points");
   if(arg=="pmreal")
     return nl->SymbolAtom("real");

   ErrorReporter::ReportError(
        "at can not handle a value of type " + arg +  "\n");
   return nl->SymbolAtom(TYPE_ERROR);
}

ListExpr InitialOrFinalTypeMap(ListExpr args){
    __TRACE__
  if(nl->ListLength(args)!=1){
      ErrorReporter::ReportError("Wrong number of arguments\n");
      return nl->SymbolAtom(TYPE_ERROR);
   }
   if(nl->AtomType(nl->First(args))==SymbolType){
      ErrorReporter::ReportError("Only simple types are allowed here.\n");
      return nl->SymbolAtom(TYPE_ERROR);
   }
   string arg = nl->SymbolValue(nl->First(args));
   if(arg=="pmpoint")
     return nl->SymbolAtom("point");
   if(arg=="pmbool")
     return nl->SymbolAtom("bool");
   if(arg=="pmint9m")
     return nl->SymbolAtom("int9m");
   if(arg=="pmpoints")
     return nl->SymbolAtom("points");
   if(arg=="pmreal")
     return nl->SymbolAtom("real");
   ErrorReporter::ReportError("Can't handle values of type "+arg+"\n");
   return nl->SymbolAtom(TYPE_ERROR);
}

ListExpr BreakpointsTypeMap(ListExpr args){
    __TRACE__
 cout << "Breakpoints type map called" <<  endl;
 int length = nl->ListLength(args);
   if(length!=1 && length!=3){
       ErrorReporter::ReportError(
         "Wrong number of arguments, one or three arguments expected\n");
       return nl->SymbolAtom(TYPE_ERROR);
   }
   if(nl->AtomType(nl->First(args))!=SymbolType){
      ErrorReporter::ReportError("breakpoints can only handle simple types ");
      return nl->SymbolAtom(TYPE_ERROR);
   }
   string arg = nl->SymbolValue(nl->First(args));
   if(length==3){
     // check the second argument to be a duration type
     if(!nl->IsEqual(nl->Second(args),"duration")){
        ErrorReporter::ReportError(
             "The second argument must be of type duration\n");
        return nl->SymbolAtom(TYPE_ERROR);
     }
     // check the third argument for bool type 
     if(!nl->IsEqual(nl->Third(args),"bool")){
        ErrorReporter::ReportError(
             "The third argument must be of type bool\n");
        return nl->SymbolAtom(TYPE_ERROR);
     } 
   }

   if(arg=="pmpoint")
      return nl->SymbolAtom("points");
   if(arg=="pmpoints")
      return nl->SymbolAtom("points");

   ErrorReporter::ReportError(
          "Invalid type for breakpoints operator : "+arg+"\n");
   return nl->SymbolAtom(TYPE_ERROR);   
}

ListExpr PMPointInstantTypeMap(ListExpr args){
    __TRACE__
   if(nl->ListLength(args)!=1){
      ErrorReporter::ReportError("Invalid number of arguments \n");
      return nl->SymbolAtom(TYPE_ERROR);
   }
   if(nl->IsEqual(nl->First(args),"pmpoint"))
      return nl->SymbolAtom("instant");

   ErrorReporter::ReportError(
        "invalid type detected, value of type pmpoint expected\n");
   return nl->SymbolAtom(TYPE_ERROR);
}

ListExpr PMPointPIntervalTypeMap(ListExpr args){
    __TRACE__
   if(nl->ListLength(args)!=1){
      ErrorReporter::ReportError("Invalid number of arguments\n");
      return nl->SymbolAtom(TYPE_ERROR);
   }
   if(nl->IsEqual(nl->First(args),"pmpoint"))
      return nl->SymbolAtom("pinterval");
   ErrorReporter::ReportError(
         "Wrong types detected, required are pmpoint x pinterval\n");
   return nl->SymbolAtom(TYPE_ERROR);
}

ListExpr PMPointPBBoxTypeMap(ListExpr args){
    __TRACE__
   if(nl->ListLength(args)!=1){
      ErrorReporter::ReportError("Wrong number of arguments\n");
      return nl->SymbolAtom(TYPE_ERROR);
   }
   if(nl->IsEqual(nl->First(args),"pmpoint"))
      return nl->SymbolAtom("pbbox");
   ErrorReporter::ReportError("pmpoint requiered\n");
   return nl->SymbolAtom(TYPE_ERROR);
}

ListExpr TrajectoryTypeMap(ListExpr args){
    __TRACE__
  if(nl->ListLength(args)==1) {
        if(nl->IsEqual(nl->First(args),"pmpoint"))
           return nl->SymbolAtom("line");
        else{
          ErrorReporter::ReportError(
                        "invalid type for trajectory operator\n");
          return nl->SymbolAtom(TYPE_ERROR);
        }
   }
   ErrorReporter::ReportError(
               "Wrong number of arguments for the trajectory operator\n");
   return nl->SymbolAtom(TYPE_ERROR);
}

ListExpr ContainsTypeMap(ListExpr args){
    __TRACE__
   if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("Contains requires 2 arguments\n");
      return nl->SymbolAtom(TYPE_ERROR);
   }
   if(nl->IsEqual(nl->First(args),"pbbox") &&
      nl->IsEqual(nl->Second(args),"pbbox"))
      return nl->SymbolAtom("bool");
   if(nl->IsEqual(nl->First(args),"pinterval")){
      if(nl->IsEqual(nl->Second(args),"pinterval"))
         return nl->SymbolAtom("bool");
      if(nl->IsEqual(nl->Second(args),"instant"))
         return nl->SymbolAtom("bool");
   }
   ErrorReporter::ReportError("Invalid types for the contains operator\n");
   return nl->SymbolAtom(TYPE_ERROR);
}

ListExpr IntersectsTypeMap(ListExpr args){
    __TRACE__
   if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("intersects requires two operands\n");
      return nl->SymbolAtom(TYPE_ERROR);
   }
   if(nl->IsEqual(nl->First(args),"pbbox") &&
      nl->IsEqual(nl->Second(args),"pbbox"))
      return nl->SymbolAtom("bool");
   if(nl->IsEqual(nl->First(args),"pinterval") &&
      nl->IsEqual(nl->Second(args),"pinterval"))
         return nl->SymbolAtom("bool");
   ErrorReporter::ReportError("Invalid types for intersects operator\n");
   return nl->SymbolAtom(TYPE_ERROR);
}

ListExpr LengthTypeMap(ListExpr args){
    __TRACE__
   if(nl->ListLength(args)!=1){
       ErrorReporter::ReportError("Lengths requires one argument\n");
       return nl->SymbolAtom(TYPE_ERROR);
   }
   if(nl->IsEqual(nl->First(args),"pinterval"))
       return nl->SymbolAtom("duration");
   ErrorReporter::ReportError("value of type pinterval expected\n");
   return nl->SymbolAtom(TYPE_ERROR);
}


ListExpr ExpandTypeMap(ListExpr args){
    __TRACE__
  if(nl->ListLength(args)!=1){
       ErrorReporter::ReportError("Invalid number of arguments\n");
       return nl->SymbolAtom(TYPE_ERROR);
  }
   if(nl->IsEqual(nl->First(args),"pmpoint")){ 
       return nl->SymbolAtom("mpoint");
   }
   ErrorReporter::ReportError(
                "expand expects a pmpoint as operand\n");
   return nl->SymbolAtom(TYPE_ERROR);
}


ListExpr CreatePMPointMap(ListExpr args){
   if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("One argument expected.\n");
     return nl->SymbolAtom(TYPE_ERROR);
   }
   if(nl->IsEqual(nl->First(args),"mpoint"))
     return nl->SymbolAtom("pmpoint");
   ErrorReporter::ReportError("mpoint expected \n");
   return nl->SymbolAtom(TYPE_ERROR);
}


ListExpr ToprelTypeMap(ListExpr args){
   if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("Two arguments required\n");
      return nl->SymbolAtom(TYPE_ERROR);
   }
   if(nl->IsEqual(nl->First(args),"pmpoint")){ 
      if(nl->IsEqual(nl->Second(args),"point") ||
         nl->IsEqual(nl->Second(args),"points"))
         return nl->SymbolAtom("pmint9m");
      else{
         ErrorReporter::ReportError("pmpoints x <invalid type>\n");
         return nl->SymbolAtom(TYPE_ERROR);
      }
   }
   if((nl->IsEqual(nl->First(args),"point")||
       nl->IsEqual(nl->First(args),"points"))){
     if(nl->IsEqual(nl->Second(args),"pmpoint")){
         return nl->SymbolAtom("pmint9m");
     }
     else{
        ErrorReporter::ReportError(
            "second argument is not of type pmpoint\n");
     }
   }
   ErrorReporter::ReportError(
          "first argument is not in {pmpoint,point,points}\n");
   return nl->SymbolAtom(TYPE_ERROR);
}

ListExpr IntersectionTypeMap(ListExpr args){
    if(nl->ListLength(args)!=2){
        ErrorReporter::ReportError("Two arguments needed");
        return nl->SymbolAtom(TYPE_ERROR);  
    }
    // for tests we use only a pmbool 
    if(!nl->AtomType(nl->First(args))==SymbolType){
        ErrorReporter::ReportError("The first argument can't be composite");
        return nl->SymbolAtom(TYPE_ERROR);
    }
    if(!nl->AtomType(nl->Second(args))==SymbolType){
        ErrorReporter::ReportError("The second argument can't be composite");
        return nl->SymbolAtom(TYPE_ERROR);
    }
    string arg1 = nl->SymbolValue(nl->First(args));
    string arg2 = nl->SymbolValue(nl->Second(args));
    if(arg1=="pbbox" && arg2=="pbbox") // intersection of two boxes
       return nl->SymbolAtom("pbbox");
    if(arg1!="pmbool"){
        ErrorReporter::ReportError("The first argument type is not accepted");
        return nl->SymbolAtom(TYPE_ERROR);
    }
    if(arg2!="pinterval"){
        ErrorReporter::ReportError("The second argument is not accepted");
        return nl->SymbolAtom(TYPE_ERROR);
    }
    return nl->SymbolAtom(arg1);
}

ListExpr DistanceTypeMap(ListExpr args){
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("Two arguments required");
     return nl->SymbolAtom(TYPE_ERROR);  
  }
  // for tests we use only a pmbool 
	if(!nl->AtomType(nl->First(args))==SymbolType){
			ErrorReporter::ReportError("The first argument can't be composite");
			return nl->SymbolAtom(TYPE_ERROR);
	}
	if(!nl->AtomType(nl->Second(args))==SymbolType){
			ErrorReporter::ReportError("The second argument can't be composite");
			return nl->SymbolAtom(TYPE_ERROR);
	}
	string arg1 = nl->SymbolValue(nl->First(args));
	string arg2 = nl->SymbolValue(nl->Second(args));
  // up to now only pmpoint x point -> pmreal is supported
  if( ( arg1=="pmpoint" && arg2=="point" ) ||
      ( arg1=="point" && arg2=="pmpoint")){
     return nl->SymbolAtom("pmreal");
  }
  ErrorReporter::ReportError(arg1 + " x  " + arg2 + " is not a valid argument pair");
  return nl->SymbolAtom(TYPE_ERROR); 
  
}


/*
5.2 Value Mappings

The value mappings compute the result of an operator. For each type 
combination of the operators a single value mapping must be provided.

*/
int EqualsFun_PBBoxPBBox_Bool(Word* args, Word& result, int message,
                         Word& local, Supplier s){
    __TRACE__
  result = qp->ResultStorage(s);
   PBBox* B1 = (PBBox*) args[0].addr;
   PBBox* B2 = (PBBox*) args[1].addr;
   ((CcBool*) result.addr)->Set(true,B1->CompareTo(B2)==0);
   return 0;
}

int EqualsFun_PIntervalPInterval_Bool(Word* args, Word& result, 
                int message, Word& local, Supplier s){
    __TRACE__
  result = qp->ResultStorage(s);
   PInterval* I1 = (PInterval*) args[0].addr;
   PInterval* I2 = (PInterval*) args[1].addr;
   ((CcBool*) result.addr)->Set(true,I1->CompareTo(I2)==0);
   return 0;
}

int ContainsFun_PBBoxPBBox_Bool(Word* args, Word& result, int message,
                           Word& local, Supplier s){
    __TRACE__
  result = qp->ResultStorage(s);
   PBBox* B1 = (PBBox*) args[0].addr;
   PBBox* B2 = (PBBox*) args[1].addr;
   ((CcBool*) result.addr)->Set(true,B1->Contains(B2));
   return 0;
}

int IntersectsFun_PBBoxPBBox_Bool(Word* args, Word& result, int message,
                             Word& local, Supplier s){
    __TRACE__
  result = qp->ResultStorage(s);
   PBBox* B1 = (PBBox*) args[0].addr;
   PBBox* B2 = (PBBox*) args[1].addr;
   bool res = B1->Intersects(B2);
   ((CcBool*) result.addr)->Set(true,res);
   return 0;
}

int UnionFun_PBBoxPBBox(Word* args, Word& result, int message,
                        Word& local, Supplier s){
    __TRACE__
  result = qp->ResultStorage(s);
   PBBox* B1 = (PBBox*) args[0].addr;
   PBBox* B2 = (PBBox*) args[1].addr;
   PBBox* TMP = B1->Clone();
   TMP->Union(B2);
   ((PBBox*) result.addr)->Equalize(TMP);
   delete TMP;
   TMP = NULL;
   return 0;
}

int Intersection_PBBox_PBBox(Word* args, Word& result,
                           int message, Word& local, Supplier s){
    __TRACE__
  result = qp->ResultStorage(s);
   PBBox* B1 = (PBBox*) args[0].addr;
   PBBox* B2 = (PBBox*) args[1].addr;
   PBBox* TMP = B1->Clone();
   TMP->Intersection(B2);
   ((PBBox*) result.addr)->Equalize(TMP);
   delete TMP;
   TMP = NULL;
   return 0;
}

int AtFun_PI(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
   result = qp->ResultStorage(s);
    PMPoint* P = (PMPoint*) args[0].addr;
    DateTime* I = (DateTime*) args[1].addr;
    Point* Res = P->At(I);
    ((Point*) result.addr)->CopyFrom(Res);
    delete Res;
    Res = NULL;
    return 0;
}

int AtFun_BI(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
   result = qp->ResultStorage(s);
    PMBool* B = (PMBool*) args[0].addr;
    DateTime* I = (DateTime*) args[1].addr;
    bool* Res = B->At(I);
    if(!Res) 
       ((CcBool*) result.addr)->Set(false,false);
     else
       ((CcBool*) result.addr)->Set(true,*Res);
    delete Res;
    Res = NULL;
    return 0;
}

int AtFun_PMInt9MInstant(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
   result = qp->ResultStorage(s);
   PMInt9M* B = (PMInt9M*) args[0].addr;
   DateTime* I = (DateTime*) args[1].addr;
   Int9M* Res  = B->At(I);
   if(!Res) 
       ((Int9M*) result.addr)->SetDefined(false);
     else
       ((Int9M*) result.addr)->Equalize(Res);
    delete Res;
    Res = NULL;
    return 0;
}

int AtFun_PMPointsInstant(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
   result = qp->ResultStorage(s);
   PMPoints* B = (PMPoints*) args[0].addr;
   DateTime* I = (DateTime*) args[1].addr;
   Points* Res  = B->At(I);
   assert(false);
   if(!Res) 
       ((Points*) result.addr)->SetDefined(false);
     else
       ((Points*) result.addr)->CopyFrom(Res);
  
   delete Res;
   Res = NULL;
   return 0;
}

int AtFun_PMRealInstant(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
   result = qp->ResultStorage(s);
   PMReal* B = (PMReal*) args[0].addr;
   DateTime* I = (DateTime*) args[1].addr;
   double* res  = B->At(I);
   if(!res)
       ((CcReal*) result.addr)->Set(false,0);
   else
       ((CcReal*) result.addr)->Set(true,*res);
   delete res;
   res = NULL;
   return 0;
}


int InitialFun_P(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
    result = qp->ResultStorage(s);
    PMPoint* P = (PMPoint*) args[0].addr;
    Point* Res = P->Initial();
    ((Point*) result.addr)->CopyFrom(Res);
    delete Res;
    Res = NULL;
    return 0;
}

int InitialFun_B(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
   result = qp->ResultStorage(s);
    PMBool* B = (PMBool*) args[0].addr;
    bool* Res = B->Initial();
    if(!Res)
       ((CcBool*) result.addr)->Set(false,false);
    else
       ((CcBool*) result.addr)->Set(true,*Res);
    delete Res;
    Res = NULL;
    return 0;
}

int InitialFun_PMInt9M(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
    result = qp->ResultStorage(s);
    PMInt9M* B = (PMInt9M*) args[0].addr;
    Int9M* Res = B->Initial();
    if(!Res)
       ((Int9M*) result.addr)->SetDefined(false);
    else
       ((Int9M*) result.addr)->Equalize(Res);
    delete Res;
    Res = NULL;
    return 0;
}

int InitialFun_PMPoints(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
    result = qp->ResultStorage(s);
    PMPoints* B = (PMPoints*) args[0].addr;
    Points* Res = B->Initial();
    if(!Res)
       ((Points*) result.addr)->SetDefined(false);
    else
       ((Points*) result.addr)->CopyFrom(Res);
    delete Res;
    Res = NULL;
    return 0;
}

int InitialFun_PMReal(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
    result = qp->ResultStorage(s);
    PMReal* B = (PMReal*) args[0].addr;
    double* Res = B->Initial();
    if(!Res)
       ((CcReal*) result.addr)->Set(false,0.0);
    else
       ((CcReal*) result.addr)->Set(true,*Res);
    delete Res;
    Res = NULL;
    return 0;
}

int FinalFun_P(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
    result = qp->ResultStorage(s);
    PMPoint* P = (PMPoint*) args[0].addr;
    Point* Res = P->Final();
    ((Point*) result.addr)->CopyFrom(Res);
    delete Res;
    Res = NULL;
    return 0;
}

int FinalFun_PMInt9M(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
    result = qp->ResultStorage(s);
    PMInt9M* P = (PMInt9M*) args[0].addr;
    Int9M* Res = P->Final();
    if(!Res)
       ((Int9M*) result.addr)->SetDefined(false);
    else
       ((Int9M*) result.addr)->Equalize(Res);
    delete Res;
    Res = NULL;
    return 0;
}
int FinalFun_B(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
   result = qp->ResultStorage(s);
    PMBool* B = (PMBool*) args[0].addr;
    bool* Res = B->Final();
    if(!Res)
        ((CcBool*) result.addr)->Set(false,false);
    else
        ((CcBool*) result.addr)->Set(true,*Res);
    delete Res;
    Res = NULL;
    return 0;
}

int FinalFun_PMPoints(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
    result = qp->ResultStorage(s);
    PMPoints* B = (PMPoints*) args[0].addr;
    Points* Res = B->Final();
    if(!Res)
        ((Points*) result.addr)->SetDefined(false);
    else
        ((Points*) result.addr)->CopyFrom(Res);
    delete Res;
    Res = NULL;
    return 0;
}

int FinalFun_PMReal(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
    result = qp->ResultStorage(s);
    PMReal* B = (PMReal*) args[0].addr;
    double* Res = B->Final();
    if(!Res)
        ((CcReal*) result.addr)->Set(false,0.0);
    else
        ((CcReal*) result.addr)->Set(true,*Res);
    delete Res;
    Res = NULL;
    return 0;
}

int TrajectoryFun(Word* args, Word& result,
                   int message, Word& local, Supplier s){
   __TRACE__
   PMPoint* P = (PMPoint*) args[0].addr;
   CLine* L = P->Trajectory();
   result.addr = L;
   return 0;
}

int BreakpointsFun_PMPoint(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
    int sons = qp->GetNoSons(s);
    PMPoint* P = (PMPoint*) args[0].addr;
    Points* Res = 0;
    if(sons==1){
        cout << "simple breakpoints fun";
        Res = P->Breakpoints();
    } else{
      cout << "extended breakpoints fun";
      DateTime* DT = (DateTime*) args[1].addr;
      CcBool* Inclusive = (CcBool*) args[2].addr;
      if(!Inclusive->IsDefined()){
         Res->SetDefined(false);
      } else{
          Res = P->Breakpoints(DT,Inclusive->GetBoolval());
      }
    }
    cout << "before copying" << (*Res) << endl;
    //((Points* )result.addr)->CopyFrom(Res);
    result.addr = Res;
    //cout <<  "the copy\n" << (*((Points*)result.addr)) << endl;
    //delete Res;
    return 0;
}

int BreakpointsFun_PMPoints(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
    int sons = qp->GetNoSons(s);
    PMPoints* P = (PMPoints*) args[0].addr;
    Points* Res = 0;
    if(sons==1){
      Res = P->Breakpoints();
    } else{
       DateTime* DT = (DateTime*) args[1].addr;
       CcBool* Inclusive = (CcBool*) args[2].addr;
       if(!Inclusive->IsDefined()){
          Res->SetDefined(false);
       }else{
          Res = P->Breakpoints(DT,Inclusive->GetBoolval());
       }
    }
    ((Points*)result.addr)->CopyFrom(Res);
    delete Res;
    return 0;
}


int IntersectsFun_PIntervalPInterval_Bool(Word* args, Word& result,
                   int message, Word& local, Supplier s){

    __TRACE__
   result = qp->ResultStorage(s);
    PInterval* I1 = (PInterval*) args[0].addr;
    PInterval* I2 = (PInterval*) args[1].addr;
    bool res = I1->Intersects(I2);
    ((CcBool*) result.addr)->Set(true,res);
    return 0;
}

int ContainsFun_PIntervalInstant_Bool(Word* args, Word& result,
                   int message, Word& local, Supplier s){
   __TRACE__

    result = qp->ResultStorage(s);
    PInterval* I = (PInterval*) args[0].addr;
    DateTime* T = (DateTime*) args[1].addr;
    bool res = I->Contains(T);
    ((CcBool*) result.addr)->Set(true,res);
    return 0;
}

int ContainsFun_PIntervalPInterval_Bool(Word* args, Word& result,
                   int message, Word& local, Supplier s){
   __TRACE__

    result = qp->ResultStorage(s);
    PInterval* I1 = (PInterval*) args[0].addr;
    PInterval* I2 = (PInterval*) args[1].addr;
    bool res = I1->Contains(I2);
    ((CcBool*) result.addr)->Set(true,res);
    return 0;
}

int LengthFun_PInterval_Duration(Word* args, Word& result,
                   int message, Word& local, Supplier s){
   __TRACE__

    result = qp->ResultStorage(s);
    PInterval* I = (PInterval*) args[0].addr;
    DateTime* Res = I->GetLength();
    ((DateTime*) result.addr)->Equalize(Res);
    delete Res;
    Res = NULL;
    return 0;
}

int StartFun_PInterval_Instant(Word* args, Word& result,
                   int message, Word& local, Supplier s){
   __TRACE__

    result = qp->ResultStorage(s);
    PInterval* I = (PInterval*) args[0].addr;
    DateTime* Res = I->GetStart();
    ((DateTime*) result.addr)->Equalize(Res);
    delete Res;
    Res = NULL;
    return 0;
}

int EndFun_PInterval_Instant(Word* args, Word& result,
                   int message, Word& local, Supplier s){
   __TRACE__

    result = qp->ResultStorage(s);
    PInterval* I = (PInterval*) args[0].addr;
    DateTime* Res = I->GetEnd();
    ((DateTime*) result.addr)->Equalize(Res);
    delete Res;
    Res = NULL;
    return 0;
}

int StartFun_PMPoint_Instant(Word* args, Word& result,
                   int message, Word& local, Supplier s){
   __TRACE__

    result = qp->ResultStorage(s);
    PMPoint* P = (PMPoint*) args[0].addr;
    DateTime S = P->GetStart();
    ((DateTime*) result.addr)->Equalize(&S);
    return 0;
}

int EndFun_PMPoint_Instant(Word* args, Word& result,
                   int message, Word& local, Supplier s){
   __TRACE__

    result = qp->ResultStorage(s);
    PMPoint* P = (PMPoint*) args[0].addr;
    DateTime End = P->GetEnd();
    ((DateTime*) result.addr)->Equalize(&End);
    return 0;
}

int IntervalFun_PMPoint_Instant(Word* args, Word& result,
                   int message, Word& local, Supplier s){
   __TRACE__

    result = qp->ResultStorage(s);
    PMPoint* P = (PMPoint*) args[0].addr;
    PInterval I = P->GetInterval();
    ((PInterval*) result.addr)->Equalize(&I);
    return 0;
}

int ExpandFun_PMPoint_MPoint(Word* args, Word& result,
                   int message, Word& local, Supplier s){
   __TRACE__

   result = qp->ResultStorage(s);
   PMPoint* P = (PMPoint*) args[0].addr;
   MPoint Res = P->Expand();
   ((MPoint*) result.addr)->CopyFrom(&Res);
   return 0;
}                   

int CreateFun_P_PM(Word* args, Word& result, int message, 
                  Word&local, Supplier s){
  result = qp->ResultStorage(s);
  MPoint* MP = (MPoint*) args[0].addr;
  ((PMPoint*)result.addr)->ReadFromMPoint(*MP);
  return 0; 
}


int Toprel_PMPoint_Point(Word* args, Word& result, int message, 
                  Word&local, Supplier s){
  __TRACE__
  result = qp->ResultStorage(s);
  PMPoint* MP = (PMPoint*) args[0].addr;
  Point*    P = (Point*) args[1].addr;
  PMInt9M* res = MP->Toprel(*P);
  ((PMInt9M*)result.addr)->Equalize(res);
  delete res; 
  res = NULL;
  return 0; 
}

int Toprel_Point_PMPoint(Word* args, Word& result, int message, 
                  Word&local, Supplier s){
  result = qp->ResultStorage(s);
  PMPoint* MP = (PMPoint*) args[1].addr;
  Point*    P = (Point*) args[0].addr;
  PMInt9M* res = MP->Toprel(*P);
  res->Transpose(); 
  ((PMInt9M*)result.addr)->Equalize(res);
  delete res;
  res = NULL; 
  return 0; 
}

int Toprel_PMPoint_Points(Word* args, Word& result, int message, 
                  Word&local, Supplier s){
  __TRACE__
  result = qp->ResultStorage(s);
  PMPoint* MP = (PMPoint*) args[0].addr;
  Points*    P = (Points*) args[1].addr;
  PMInt9M* res = MP->Toprel(*P);
  ((PMInt9M*)result.addr)->Equalize(res);
  delete res;
  res = NULL; 
  return 0; 
}

int Toprel_Points_PMPoint(Word* args, Word& result, int message, 
                  Word&local, Supplier s){
  result = qp->ResultStorage(s);
  PMPoint* MP = (PMPoint*) args[1].addr;
  Points*    P = (Points*) args[0].addr;
  PMInt9M* res = MP->Toprel(*P);
  res->Transpose(); 
  ((PMInt9M*)result.addr)->Equalize(res);
  delete res; 
  res = NULL;
  return 0; 
}


int Intersection_PMBool_PInterval(Word* args, Word& result, int message, 
                  Word&local, Supplier s){
  result = qp->ResultStorage(s);
  PMBool*   MB = (PMBool*) args[0].addr;
  PInterval*  I = (PInterval*) args[1].addr;
  DateTime* T1 = I->GetStart();
  DateTime* T2 = I->GetEnd();
  PMBool* res = (PMBool*)result.addr;
  MB->Intersection((*T1),I->IsLeftClosed(),
                   (*T2),I->IsRightClosed(),
                   res
                   );
  delete T1;
  delete T2;
  return 0; 
}

int Distance_PMPoint_Point(Word* args, Word& result, int message, 
                  Word&local, Supplier s){

  result = qp->ResultStorage(s);
  PMPoint* pmpoint = (PMPoint*) args[0].addr;
  Point* point = (Point*) args[1].addr;
  PMReal* res = (PMReal*) result.addr;
  if(!pmpoint->IsDefined() || !point->IsDefined())
     res->SetDefined(false);
  else
     pmpoint->DistanceTo(point->GetX(),point->GetY(),(*res));
  return 0;
}
int Distance_Point_PMPoint(Word* args, Word& result, int message, 
                  Word&local, Supplier s){

  result = qp->ResultStorage(s);
  PMPoint* pmpoint = (PMPoint*) args[1].addr;
  Point* point = (Point*) args[0].addr;
  PMReal* res = (PMReal*) result.addr;
  if(!pmpoint->IsDefined() || !point->IsDefined())
     res->SetDefined(false);
  else
     pmpoint->DistanceTo(point->GetX(),point->GetY(),(*res));
  return 0;
}


/*
5.3 Specifications of the Operators

The following strings contain textual descriptions for
operators of this algebra.

*/
const string EqualsSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pbbox x pbbox -> bool\""
   " \" _ = _ \" "
   "   \"checks whether the arguments are equal\" "
   "   \" query B1 = B2\" ))";

const string ContainsSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pbbox x pbbox -> bool\""
   " \" _ contains _ \" "
   "   \"returns true if a contains b\""
   "   \" query B1 contains B2\" ))";

const string IntersectsSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pbbox x pbbox -> bool\""
   " \" _ intersects _ \" "
   "   \"checks if the arguments share a common point\" "
   "   \" query B1 intersects B2\" ))";

const string IntersectionSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pbbox x pbbox -> pbbox\""
   " \" _ intersection _ \" "
   "   \"computes the intersection between the arguments\" "
   "   \" query B1 intersection B2\" ))";

const string UnionSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pbbox x pbbox -> pbbox\""
   " \" _ union _ \" "
   " \"computes the union of the arguments\" "
   " \" query B1 union B2\" ))";

const string AtSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pmobject x instant -> object\""
   " \" _ at(_) \" "
   " \"computes the value  of the argument at the given time\" "
   " \" query P5 at( [const instant value 1.5])\" ))";

const string InitialSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pmobject -> object\""
   " \" initial(_) \" "
   " \"computes the first defined value of the argument\" "
   " \" query initial(P1)\" ))";

const string FinalSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pmobject -> object\""
   " \" final(_) \" "
   " \"computes the last defined value of the argument\" "
   " \" query last(P1)\" ))";

const string TrajectorySpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pmpoint -> line\""
   " \" trajectory( _ ) \" "
   " \"computes the trajectory of the argument\" "
   " \" query trajectory(P5)\" ))";
   
const string BreakpointsSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pmpoint | pmpoints  -> points\""
   " \" breakpoints( _ ) \" "
   " \"computes the breakpoints of the argument\" "
   " \" query breakpoints(P5)\" ))";

const string StartSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pinterval -> instant\""
   " \" start( _ ) \" "
   " \"computes the start time of a pinterval\" "
   " \" query start(I)\" ))";

const string EndSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pinterval -> instant\""
   " \" end( _ ) \" "
   " \"computes the start time of a pinterval\" "
   " \" query end(I)\" ))";

const string LengthSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pinterval -> duration\""
   " \" length( _ ) \" "
   " \"computes the duration of a pinterval\" "
   " \" query length(I)\" ))";

const string ExpandSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" \"Example\" )"
   " ( \"pmpoint -> mpoint\""
   " \" expand(_) \" "
   " \"creates a moving point from a periodic one\""
   " \"\""
   " \" query expand(I)\" ))";
   
const string CreatePMPointSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" \"Example\" )"
   " ( \"mpoint  -> pmpoint\""
   " \" createpmpoint(_) \" "
   " \"creates a periodic moving point from a linearly moving one\""
   " \"it's is development\""
   " \" query createpmpoint(p1)\" ))";

const string ToprelSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" \"Example\" )"
   " ( \"pmobject x spatial[temporal] object -> pmint9m\""
   " \" toprel(_,_) \" "
   " \"computes the moving 9 intersection matrix for this combination\""
   " \"\""
   " \" query toprel(p1,p2)\" ))";

const string DistanceSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" \"Example\" )"
   " ( \"pmpoint x point -> pmreal\""
   " \" distance(_, _) \" "
   " \"computes the distance between a static point and a periodic moving one\""
   " \"\""
   " \" query distance(pm1,p1)\" ))";


/*
5.4 ValueMappings of overloaded Operators

For overloaded operators an array of value mappings must 
exists. Using the Select function (Section [ref]{select_function}),
the correct map is choosen.

*/
ValueMapping EqualsValueMap[] = {
        EqualsFun_PBBoxPBBox_Bool,EqualsFun_PIntervalPInterval_Bool };

ValueMapping AtValueMap[] =
   {AtFun_PI,AtFun_BI,AtFun_PMInt9MInstant,AtFun_PMPointsInstant,
    AtFun_PMRealInstant};

ValueMapping ContainsValueMap[] = {
    ContainsFun_PBBoxPBBox_Bool, ContainsFun_PIntervalPInterval_Bool,
    ContainsFun_PIntervalInstant_Bool
};

ValueMapping IntersectsValueMap[] = {
    IntersectsFun_PBBoxPBBox_Bool, IntersectsFun_PIntervalPInterval_Bool
};

ValueMapping InitialValueMap[] = {
    InitialFun_B,InitialFun_P,InitialFun_PMInt9M,InitialFun_PMPoints,
    InitialFun_PMReal
};

ValueMapping FinalValueMap[] = {
    FinalFun_B,FinalFun_P,InitialFun_PMInt9M,FinalFun_PMPoints,
    FinalFun_PMReal
};

ValueMapping BreakpointsValueMap[] = {
     BreakpointsFun_PMPoint,BreakpointsFun_PMPoints
};

ValueMapping ToprelValueMap[] = {
     Toprel_Point_PMPoint, Toprel_PMPoint_Point,
     Toprel_Points_PMPoint,Toprel_PMPoint_Points
};

ValueMapping IntersectionValueMap[] = {
     Intersection_PBBox_PBBox, Intersection_PMBool_PInterval
};

ValueMapping DistanceValueMap[] = {
  Distance_PMPoint_Point, Distance_Point_PMPoint
}; 

/*
5.6 SelectionFunctions

[label]{select_function}

The selection function selects a valuemapping for the given arguments.
The returned number corresponds to the array index of the value mapping
array.

*/
static int EqualsSelect(ListExpr args){
    __TRACE__
 if(nl->IsEqual(nl->First(args),"pbbox"))
     return 0;
  if(nl->IsEqual(nl->First(args),"pinterval"))
     return 1;
  return -1;
}

static int AtSelect(ListExpr args){
    __TRACE__
  if(nl->IsEqual(nl->First(args),"pmpoint"))
     return 0;
  if(nl->IsEqual(nl->First(args),"pmbool"))
     return 1;
  if(nl->IsEqual(nl->First(args),"pmint9m"))
     return 2;
  if(nl->IsEqual(nl->First(args),"pmpoints"))
     return 3;
  if(nl->IsEqual(nl->First(args),"pmreal"))
     return 4;
  return -1;
}

static int FinalSelect(ListExpr args){
    __TRACE__
 if(nl->IsEqual(nl->First(args),"pmbool"))
     return 0;
  if(nl->IsEqual(nl->First(args),"pmpoint"))
     return 1;
  if(nl->IsEqual(nl->First(args),"pmint9m"))
     return 2;
  if(nl->IsEqual(nl->First(args),"pmpoints"))
     return 3;
  if(nl->IsEqual(nl->First(args),"pmreal"))
     return 4;
  return -1;
}

static int InitialSelect(ListExpr args){
    __TRACE__
 if(nl->IsEqual(nl->First(args),"pmbool"))
     return 0;
  if(nl->IsEqual(nl->First(args),"pmpoint"))
     return 1;
  if(nl->IsEqual(nl->First(args),"pmint9m"))
     return 2;
  if(nl->IsEqual(nl->First(args),"pmpoints"))
     return 3;
  if(nl->IsEqual(nl->First(args),"pmreal"))
     return 4;
  return -1;
}

static int ContainsSelect(ListExpr args){
    __TRACE__
  if(nl->ListLength(args)!=2)
      return -1;
   if(nl->IsEqual(nl->First(args),"pbbox") &&
      nl->IsEqual(nl->Second(args),"pbbox"))
         return 0;
   if(nl->IsEqual(nl->First(args),"pinterval") &&
      nl->IsEqual(nl->Second(args),"pinterval"))
         return 1;
   if(nl->IsEqual(nl->First(args),"pinterval") &&
      nl->IsEqual(nl->Second(args),"instant"))
         return 2;
   return -1;
}

static int IntersectsSelect(ListExpr args){
    __TRACE__
  if(nl->ListLength(args)!=2)
      return -1;
   if(nl->IsEqual(nl->First(args),"pbbox") &&
      nl->IsEqual(nl->Second(args),"pbbox"))
         return 0;
   if(nl->IsEqual(nl->First(args),"pinterval") &&
      nl->IsEqual(nl->Second(args),"pinterval"))
         return 1;
   return -1;
}

static int BreakpointsSelect(ListExpr args){
   __TRACE__
   int len = nl->ListLength(args);
   if(len!=1 && len!=3)
      return -1;
   if(nl->IsEqual(nl->First(args),"pmpoint"))
        return 0;
   if(nl->IsEqual(nl->First(args),"pmpoints"))
        return 1;
   return -1;
}

static int ToprelSelect(ListExpr args){
  __TRACE__
  if(nl->ListLength(args)!=2)
      return -1; // error in type mapping detected
  if(nl->IsEqual(nl->First(args),"point") &&
     nl->IsEqual(nl->Second(args),"pmpoint"))
       return 0;
  if(nl->IsEqual(nl->First(args),"pmpoint") &&
     nl->IsEqual(nl->Second(args),"point"))
       return 1;
  if(nl->IsEqual(nl->First(args),"points") &&
     nl->IsEqual(nl->Second(args),"pmpoint"))
       return 2;
  if(nl->IsEqual(nl->First(args),"pmpoint") &&
     nl->IsEqual(nl->Second(args),"points"))
       return 3;
  return -1; 

}

static int IntersectionSelect(ListExpr args){
   __TRACE__
   string arg1 = nl->SymbolValue(nl->First(args)); 
   string arg2 = nl->SymbolValue(nl->Second(args));
   if(arg1=="pbbox" && arg2=="pbbox")
       return 0;
   if(arg1=="pmbool" && arg2=="pinterval")
       return 1;
   return -1; // should never occur
}

static int DistanceSelect(ListExpr args){
  __TRACE__
  string arg1 = nl->SymbolValue(nl->First(args));
  string arg2 = nl->SymbolValue(nl->Second(args));
  if(arg1=="pmpoint" && arg2=="point")
     return 0;
  if(arg1=="point" && arg2=="pmoint")
     return 1;
  return -1; // should never be the case
}

/*
5.7 Definition of the Operators

5.7.1 Non overloaded Operators

*/

Operator punion(
        "union",      // name
        UnionSpec,    // specification
        UnionFun_PBBoxPBBox, // value mapping
        Operator::SimpleSelect, // selection function
        PBBoxPBBoxPBBoxTypeMap); // type mapping

Operator ptrajectory(
        "trajectory",
        TrajectorySpec,
        TrajectoryFun,
        Operator::SimpleSelect,
        TrajectoryTypeMap);

Operator plength(
        "length",
        LengthSpec,
        LengthFun_PInterval_Duration,
        Operator::SimpleSelect,
        LengthTypeMap);

Operator pstart(
        "start",
        StartSpec,
        StartFun_PInterval_Instant,
        Operator::SimpleSelect,
        PIntervalInstantTypeMap);

Operator pend(
        "end",
        EndSpec,
        EndFun_PInterval_Instant,
        Operator::SimpleSelect,
        PIntervalInstantTypeMap);
        
Operator pexpand(
        "expand",
        ExpandSpec,
        ExpandFun_PMPoint_MPoint,
        Operator::SimpleSelect,
        ExpandTypeMap);
        

Operator createpmpoint(
        "createpmpoint",
        CreatePMPointSpec,
        CreateFun_P_PM,
        Operator::SimpleSelect,
        CreatePMPointMap);


/*
5.7.2 Overloaded Operators

*/
Operator pequals(
       "=",                        // name
       EqualsSpec,                 // specification
       2,                          // number of functions
       EqualsValueMap,             // array with value mappings
       EqualsSelect,               // selection function
       EqualsTypeMap);             // type mapping

Operator pcontains(
        "contains",
        ContainsSpec,
        3,
        ContainsValueMap,
        ContainsSelect,
        ContainsTypeMap);

Operator pintersects(
        "intersects",
        IntersectsSpec,
        2,
        IntersectsValueMap,
        IntersectsSelect,
        IntersectsTypeMap);

Operator pat(
       "at",               // name
       AtSpec,              // specification
       5,                         // number of functions
       AtValueMap,
       AtSelect,
       AtTypeMap);

Operator pinitial(
       "initial",               // name
       InitialSpec,              // specification
       5,                         // number of functions
       InitialValueMap,
       InitialSelect,
       InitialOrFinalTypeMap);

Operator pfinal(
       "final",               // name
       FinalSpec,              // specification
       5,                         // number of functions
       FinalValueMap,
       FinalSelect,
       InitialOrFinalTypeMap);

Operator pbreakpoints(
       "breakpoints",               // name
       BreakpointsSpec,              // specification
       2,                         // number of functions
       BreakpointsValueMap,
       BreakpointsSelect,
       BreakpointsTypeMap);

Operator ptoprel(
       "toprel",             // name
       ToprelSpec,           // specification
       4,                    // number of functions
       ToprelValueMap,
       ToprelSelect,
       ToprelTypeMap);

Operator pintersection(
       "intersection",             // name
       IntersectionSpec,           // specification
       2,                    // number of functions
       IntersectionValueMap,
       IntersectionSelect,
       IntersectionTypeMap);

Operator pdistance(
       "distance",             // name
       DistanceSpec,           // specification
       2,                    // number of functions
       DistanceValueMap,
       DistanceSelect,
       DistanceTypeMap);

} // namespace periodic

/*
6 Creating the Algebra

6.1 Definition of the PeriodicAlgebra

*/
class PeriodicMoveAlgebra : public Algebra
{
 public:
  PeriodicMoveAlgebra() : Algebra()
  {
    // type constructors
    AddTypeConstructor(&periodic::pbbox);
    periodic::pbbox.AssociateKind("DATA");
    AddTypeConstructor( &periodic::relinterval );
    periodic::relinterval.AssociateKind("DATA");
    AddTypeConstructor(&periodic::pmpoint);
    periodic::pmpoint.AssociateKind("DATA");
    AddTypeConstructor(&periodic::pmbool);
    periodic::pmbool.AssociateKind("DATA");
    AddTypeConstructor(&periodic::pmint9m);
    periodic::pmint9m.AssociateKind("DATA");
    AddTypeConstructor(&periodic::pinterval);
    periodic::pinterval.AssociateKind("DATA");
    AddTypeConstructor(&periodic::pmpoints);
    periodic::pmpoints.AssociateKind("DATA");
    AddTypeConstructor(&periodic::pmreal);
    periodic::pmreal.AssociateKind("DATA");
   // operators
    AddOperator(&periodic::pequals);
    AddOperator(&periodic::pcontains);
    AddOperator(&periodic::pintersects);
    AddOperator(&periodic::pintersection);
    AddOperator(&periodic::punion);
    AddOperator(&periodic::pat);
    AddOperator(&periodic::ptrajectory);
    AddOperator(&periodic::pbreakpoints);
    AddOperator(&periodic::plength);
    AddOperator(&periodic::pstart);
    AddOperator(&periodic::pend);
    AddOperator(&periodic::pexpand);
    AddOperator(&periodic::pinitial);
    AddOperator(&periodic::pfinal);
    AddOperator(&periodic::createpmpoint);
    AddOperator(&periodic::ptoprel);
    AddOperator(&periodic::pdistance);
  }
  ~PeriodicMoveAlgebra() {};
};

  PeriodicMoveAlgebra periodicMoveAlgebra;


/*
6.2 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/


extern "C"
Algebra*
InitializePeriodicAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  __TRACE__
  nl = nlRef;
  qp = qpRef;
  return (&periodicMoveAlgebra);
}
