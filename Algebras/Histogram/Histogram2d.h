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
//[TOC] [\tableofcontents]
//[_][\_]
//[&][\&]
//characters [1] verbatim:   [\verb@]    [@]

""[1]

[1] Header File of the Histogram Algebra

December 2007, S. H[oe]cher, M. H[oe]ger, A. Belz, B. Poneleit


[TOC]

1 Overview

The file "Histogram2d.h" contains declarations of the class and methods
specific to histogram2d objects.

2 Defines and includes

It includes "HistogramBase.h" which contains declarations of functions common
to both histogram1d and histogram2d. "HistogramUtils.h" contains further
helper functions that are not attached to specific classes or operators.

*/
#ifndef HISTOGRAM2D_H_
#define HISTOGRAM2D_H_

#include "HistogramBase.h"
#include "HistogramUtils.h"


namespace hgr {
/*
3 The class Histogram2d

The class "Histogram2d" is derived from BaseHistogram, thus inheriting
important includes and some basic methods common to 1d and 2d histograms.

*/
  class Histogram2d : public BaseHistogram
  {
    public:

      Histogram2d(bool _defined, size_t sizeX = 0, size_t sizeY = 0);
      Histogram2d(const Histogram2d& rhs);
      ~Histogram2d();

      Histogram2d& operator = (const Histogram2d& h);

/*
3.1 Testing and reading

3.1.1 IsConsistent(bool)

returns TRUE if the histogram1d is empty or
   all of the following conditions are true:

  * The number of ranges is 2 or more.

  * The number of ranges is one more than the number of bins.

  * The range array is sorted in ascending order.

  * There are no duplicate ranges.

Note: To skip the (expensive) tests 3 and 4, set the parameter to false.

*/
      bool IsConsistent(const bool checkOrder = true) const;

/*
3.1.2 IsEmpty()

returns TRUE if both the number of ranges X and Y the and
number of bins are 0.

*/
      bool IsEmpty() const;

/*
3.1.3 IsRefinement(BaseHistogram)

returns TRUE if the first (acting) histogram1d is a
refinement of the histogram1d handed over as parameter. h1 is a refinement
of h2 if

  * both h1 and h2 are defined and

  * both h1 and h2 are consistent and

  * both h1 and h2 are not empty and

  * the first and last ranges of h1 and h2 are equal and

  * every range of h2 does also occur in h1.

*/
      bool IsRefinement(const BaseHistogram& h) const;

/*
3.1.4 GetRangeX(int), GetRangeY(int)

returns the X or Y range value at the specified index.

*/
      HIST_REAL GetRangeX(int i) const;
      HIST_REAL GetRangeY(int i) const;

/*
3.1.5 GetNoRangeX(), GetNoRangeY(real)

returns the number of ranges in the X or Y dimension of the owning histogram.

*/
      int GetNoRangeX() const;
      int GetNoRangeY() const;

/*
3.1.6 FindBin(real, real), FindBinX(real), FindBinY(real)

returns the index number of the bin the given real value(s) would be
assigned to.
returns undefined if the parameter value is outside the histogram range.

FindBin(real, real) returns the index number of the bin that is to take the
couple of values (x, y).

FindBinX(real) returns the x coordinate of the bin for a couple (x, y)
of values with a given x and any y. FindBinY(real) returns the y coordinate
of the bin for a couple (x, y) of values with any x and a given y.

*/
      CcInt FindBin(const HIST_REAL& x, const HIST_REAL& y) const;
      const CcInt FindBinX(const HIST_REAL& x) const;
      const CcInt FindBinY(const HIST_REAL& y) const;

/*
3.1.7 GetBinCoords(int)

takes the index number of a bin and returns the X and Y coordinates of that
bin.

*/
      pair<CcInt, CcInt> GetBinCoords(const int index) const;

/*
3.1.8 NoBinsX(), NoBinsY()

returns the number of bins in the X resp. Y ranges.

*/
      int NoBinsX() const;
      int NoBinsY() const;

/*
3.1.9 GetCount(int, int)

returns the value of the bin indicated by the two given index numbers.
returns undefined if one of the index values is invalid.

*/
      CcReal GetCount(const int x, const int y) const;

/*
3.1.10 Compare(BaseHistogram), CompareAlmost(BaseHistogram)

compares two histogram2d objects and returns 0, -1 or 1, according to the
result of the comparison:

  * 0: this == b: all bin values and all range values are equal
                  or
                  both objects are not defined
                  or
                  both objects are empty.

  * -1: this $<$ b: all range values are equal and
                  each bin value of this object is smaller than
                  the corresponding bin value of object b
                  or
                  this object is undefined while object b is defined.

  * 1: this $>$ b:  all other cases.


"CompareAlmost" works exactly like Compare(BaseHistogram) but performs the comparison of values by means
of the helper function "AlmostEqual(HIST[_]REAL, HIST[_]REAL)", defined in HistogramUtils.h, thus
allowing to accept slight differences between the real values as "equal".

*/
      int Compare(const BaseHistogram& h) const;
      int CompareAlmost(const BaseHistogram& h) const;

/*
3.1.11 operators == and $<$

The operator == returns true if CompareAlmost returns 0.
The operator $<$ returns true if CompareAlmost returns -1.

*/
      bool operator ==(const BaseHistogram& h) const;
      bool operator <(const BaseHistogram& h) const;

/*
3.1.12 Distance(BaseHistogram p)

Compares the similarity of two histograms: Returns the sum of the squared
difference values between the couples of bins with equal number.

Example:

histogram1: ((0.0 1.0 2.0)(0.0 1.0)(0.5 0.5))

histogram2: ((0.0 1.0 2.0)(0.0 1.0)(0.6 0.7))

return value: (0.1 x 0.1) + (0.2 x 0.2) = 0.01 + 0.04 = 0.05

*/
      HIST_REAL Distance(const BaseHistogram* h);

/*
3.1.13 MeanX(), MeanY()

compute the bin-weighted arithmetic mean M of a histogram on the x resp. y
axis using the recurrence relation

        $M(n) = M(n-1) + (x[n] - M(n-1)) (w(n)/(W(n-1) + w(n)))$

        $W(n) = W(n-1) + w(n)$

We skip negative values in the bins, since those correspond to negative
weights (BJG).

The implementation is taken from gsl[_]histogram[_]stat2d.c of the
GNU Scientific Library (gsl-1.9), Author: Achim Gaedke, Jan. 2002

*/
      CcReal MeanX() const;
      CcReal MeanY() const;

/*
3.1.14 VarianceX(), VarianceY(), Covariance()

compute the variance resp. covariance of the histogram on the x or y axis.

We skip negative values in the bins,
since those correspond to negative weights (BJG).

The implementation is taken from gsl[_]histogram[_]stat2d.c of the
GNU Scientific Library (gsl-1.9), Author: Achim Gaedke, Jan. 2002

*/
      CcReal Variance() const;
      CcReal VarianceX() const;
      CcReal VarianceY() const;
      CcReal Covariance() const;


/*
3.2 Construction and manipulation

3.2.1 AppendRangeX(real), AppendRangeY(real)

append the given real value to the X or Y array of ranges.

*/
      void AppendRangeX(const HIST_REAL& r);
      void AppendRangeY(const HIST_REAL& r);

/*
3.2.2 ResizeRangeX(int), ResizeRangeY(int)

sets the initial size of the X or Y ranges array to the given integer value.

*/
      void ResizeRangeX(const int newSize);
      void ResizeRangeY(const int newSize);

/*
3.2.3 Clear()

clears the arrays of ranges and bins.

*/
      void Clear();

/*
3.3.4 Destroy()

destroys the histogram, i.e. it's ranges and bins arrays.

*/
      void Destroy();

/*
3.3.5 Insert(int, real), Insert(real, real, real)

The first function inserts the given real value into the bin with the given
int index. The second function adds the real value given as third parameter to
the appropriate bin for the coordinates given as first two parameters.
It returns TRUE if such a bin has been found, and FALSE if not.

*/

      void Insert(const int bin, const HIST_REAL& weight = 1.0);
      bool Insert(const HIST_REAL& x, const HIST_REAL& y,
                  const HIST_REAL& weight = 1.0);

/*
3.3.6 Coarsen(BaseHistogram)

coarsens the owning histogram according to the pattern of the histogram given
as parameter. The function requires as a precondition that the owning histogram
is a refinement of the parameter histogram.

*/
      virtual void Coarsen(const BaseHistogram* h);

/*
3.3.7 CopyRangesFrom(BaseHistogram)

sets the ranges of the owning histogram to the ranges pattern specified by the
histogram given as parameter.

*/
      void CopyRangesFrom(const BaseHistogram* h);

/*
3.3 Mandatory functions

*/
      static Word In( const ListExpr typeInfo,
                      const ListExpr instance,
                      const int errorPos,
                      ListExpr& errorInfo,
                      bool& correct);

      static ListExpr Out ( ListExpr typeInfo, Word value );
      static void* Cast(void* addr);
      static Word Create(const ListExpr typeInfo);
      static bool KindCheck(ListExpr type, ListExpr& errorInfo);

      int Compare(const Attribute *rhs) const;
      int CompareAlmost(const Attribute *rhs) const;
      bool Adjacent(const Attribute *attrib) const;
      Attribute *Clone() const;
      size_t HashValue() const;
      void CopyFrom(const Attribute* right);
      int NumOfFLOBs() const;
      Flob* GetFLOB( const int i );
      ostream& Print( ostream& os ) const;

      size_t Sizeof() const;

      inline static const string BasicType() { return "histogram2d";}

      static const bool checkType(ListExpr list){
        return listutils::isSymbol(list, BasicType());
      }


    private:

      friend struct ConstructorFunctions<Histogram2d>;

      DbArray<HIST_REAL> rangeX;
      DbArray<HIST_REAL> rangeY;

      Histogram2d();

  }; // class Histogram2d : public Attribute

  ostream& operator << (ostream& os, const Histogram2d& h);

/*

4 ConstructorInfo and functions structs

*/
  struct histogram2dInfo : ConstructorInfo
  {
    histogram2dInfo();
  }; // struct histogram1dInfo : ConstructorInfo {

  struct histogram2dFunctions : ConstructorFunctions<Histogram2d>
  {
    histogram2dFunctions();
  }; // struct histogram1dFunctions : ConstructorFunctions<Histogram1d> {


/*
5 Type and value mapping functions of the operators

5.1 binrange[_]minX, binrange[_]maxX, binrange[_]minY, binrange[_]maxY

*/
  ListExpr binrange_minXY_maxXYTypeMap(ListExpr args);
  int binrange_minXFun(Word* args, Word& result, int message, Word& local,
      Supplier s);
  int binrange_maxXFun(Word* args, Word& result, int message, Word& local,
      Supplier s);
  int binrange_minYFun(Word* args, Word& result, int message, Word& local,
      Supplier s);
  int binrange_maxYFun(Word* args, Word& result, int message, Word& local,
      Supplier s);

/*
5.2 BinsX, BinsY

*/
  ListExpr BinsXYTypeMap(ListExpr args);
  int BinsXFun(Word* args, Word& result, int message, Word& local, Supplier s);
  int BinsYFun(Word* args, Word& result, int message, Word& local, Supplier s);

/*
5.3 Covariance

*/
  int CovarianceFun(Word* args, Word& result, int message, Word& local,
      Supplier s);

/*
5.4 CreateHistogram2d

*/
  ListExpr CreateHistogram2dTypeMap(ListExpr args);
  int CreateHistogram2dFun(Word* args, Word& result, int message, Word& local,
      Supplier s);

/*
5.5 CreateHistogram2dEquicount

*/
  ListExpr CreateHistogram2dEquicountTypeMap(ListExpr args);
  int CreateHistogram2dEquicountFun(Word* args, Word& result, int message,
      Word& local, Supplier s);

/*
5.6 CreateHistogram2dEquiwidth

*/
  ListExpr CreateHistogram2dEquiwidthTypeMap(ListExpr args);
  int CreateHistogram2dEquiwidthFun(Word* args, Word& result, int message,
      Word& local, Supplier s);

/*
5.7 FindBin2d

*/
  template<bool X>
  int FindBin2dFun(Word* args, Word& result, int message, Word& local,
      Supplier s);

/*
5.8 FindMinMaxBin

*/
  template<bool isMin>
  int FindMinMaxBinFun2d(Word* args, Word& result, int message, Word& local,
        Supplier s);

/*
5.9 Getcount2d

*/
  int Getcount2dFun(Word* args, Word& result, int message, Word& local,
      Supplier s);
  ListExpr Getcount2dTypeMap(ListExpr args);

/*
5.10 Hist2dReal

*/
  ListExpr Hist2dRealTypeMap(ListExpr args);

/*
5.11 Insert2d

*/
  template<bool incValSupplied> ListExpr Insert2dTypeMap(ListExpr args);
  template<bool incValSupplied> int Insert2dFun(Word* args, Word& result,
      int message, Word& local, Supplier s);

/*
5.12 MeanX, MeanY

*/
  int MeanXFun(Word* args, Word& result,
      int message, Word& local, Supplier s);

  int MeanYFun(Word* args, Word& result,
      int message, Word& local, Supplier s);

/*
5.13 SetHistogram2d

*/
  ListExpr SetHistogram2dTypeMap(ListExpr args);
  int SetHistogram2dFun(Word* args, Word& result, int message, Word& local,
      Supplier s);

/*
5.14 Shrink2d

*/
  ListExpr Shrink2dTypeMap(ListExpr args);
  template<bool eager> int Shrink2dFun(Word* args, Word& result, int message,
      Word& local, Supplier s);

/*
5.15 VarianceX, VarianceY

*/
  int VarianceXFun(Word* args, Word& result, int message, Word& local,
      Supplier s);

  int VarianceYFun(Word* args, Word& result, int message, Word& local,
      Supplier s);

/*

6 Info structs

6.1 BinsX, BinsY

*/
  struct BinsXInfo : OperatorInfo
  {
    inline BinsXInfo() :
      OperatorInfo()
    {
      name = "binsX";
      signature = Histogram2d::BasicType() + " -> " + CcInt::BasicType();
      syntax = "binsX(_)";
      meaning = "Returns number of bins on x axis.";
      example = "";
    }
  };

  struct BinsYInfo : OperatorInfo
  {
    inline BinsYInfo() :
      OperatorInfo()
    {
      name = "binsY";
      signature = Histogram2d::BasicType() + " -> " + CcInt::BasicType();
      syntax = "binsY(_)";
      meaning = "Returns number of bins on y axis.";
      example = "";
    }
  };

/*
6.2 binrange[_]minX, binrange[_]maxX, binrange[_]minY, binrange[_]maxY

*/
  struct binrange_minXInfo : OperatorInfo
  {
    // constructor
    inline binrange_minXInfo() :
      OperatorInfo()
    {
      name = "binrange_minX";
      signature = "histogram2d x int -> real";
      syntax = "binrange_minX(_, _)";
      meaning = "Returns the histogram's lower range on the x axis "
        "of the given interval.";
    }
  };

  struct binrange_maxXInfo : OperatorInfo
  {
    // constructor
    inline binrange_maxXInfo() :
      OperatorInfo()
    {
      name = "binrange_maxX";
      signature = "histogram2d x int -> real";
      syntax = "binrange_maxX(_, _)";
      meaning = "Returns the histogram's upper range on the x axis "
        "of the given interval.";
    }
  };

  struct binrange_minYInfo : OperatorInfo
  {
    // constructor
    inline binrange_minYInfo() :
      OperatorInfo()
    {
      name = "binrange_minY";
      signature = "histogram2d x int -> real";
      syntax = "binrange_minY(_, _)";
      meaning = "Returns the histogram's lower range on the y axis "
        "of the given interval.";
    }
  };

  struct binrange_maxYInfo : OperatorInfo
  {
    // constructor
    inline binrange_maxYInfo() :
      OperatorInfo()
    {
      name = "binrange_maxY";
      signature = "histogram2d x int -> real";
      syntax = "binrange_maxY(_, _)";
      meaning = "Returns the histogram's upper range on the y axis "
        "of the given interval.";
    }
  };

/*
6.3 Covariance

*/
  struct CovarianceInfo : OperatorInfo
  {
    inline CovarianceInfo() :
      OperatorInfo()
    {
      name = "covariance";
      signature = Histogram2d::BasicType() + " -> " + CcReal::BasicType();
      syntax = "_ covariance";
      meaning = "Compute the covariance.";
      example = "";
    }
  };

/*
6.4 CreateHistogram2d

*/
  struct CreateHistogram2dInfo : OperatorInfo
  {
    CreateHistogram2dInfo() :
      OperatorInfo()
    {
      name = "create_histogram2d";
      signature = "stream(tuple(X)) x ax x ay x " +
      Histogram2d::BasicType() + " -> " + Histogram2d::BasicType();
      syntax = "_ create_histogram2d [_, _, _]";
      meaning = "Fills a histogram2d from an ordered stream of tuples";
      example = "";
    }
  };

/*
6.5 CreateHistogram2dEquicount

*/
  struct CreateHistogram2dEquicountInfo : OperatorInfo
  {
    inline CreateHistogram2dEquicountInfo() :
      OperatorInfo()
    {
      name = "create_histogram2d_equicount";
      signature = "stream(tuple(X)) x ax x ay x int x int -> " +
	           Histogram2d::BasicType();
      syntax = "_ create_histogram2d_equicount [_, _, _, _]";
      meaning = "Creates a histogram2d from a stream of tuples;"
        " all categories will have equal height";
      example = "";
    }
  };

/*
6.6 CreateHistogram2dEquiwidth

*/
  struct CreateHistogram2dEquiwidthInfo : OperatorInfo
  {
    CreateHistogram2dEquiwidthInfo() :
      OperatorInfo()
    {
      name = "create_histogram2d_equiwidth";
      signature = "((stream (tuple([a1:t1, ..., ax:real, ..., "
        "ay:real, ...,an:tn]))) x ax x ay x "
        "nx:int x ny:int) -> (histogram2d)";
      syntax = "_ create_histogram1d_equiwidth [_,_,_,_]";
      meaning = "Returns a histogram2d with max. nx "
        "x-ranges and max. ny y-ranges and equal area. "
        "ax and ay represents the input-value pairs.";
      example = "";
    }
  };

/*
6.7 FindBinX, FindBinY

*/
  struct FindBinXInfo : OperatorInfo
  {
    inline FindBinXInfo() :
      OperatorInfo()
    {
      name = "findbinX";
      signature = Histogram2d::BasicType() + " x real -> int";
      syntax = "findbinX(_, _)";
      meaning = "Returns the x-coordinate of the bin, "
          "in which value would fall";
      example = "";
    }
  };

  struct FindBinYInfo : OperatorInfo
  {
    inline FindBinYInfo() :
      OperatorInfo()
    {
      name = "findbinY";
      signature = Histogram2d::BasicType() + " x real -> int";
      syntax = "findbinY(_, _)";
      meaning = "Returns the y-coordinate of the bin, "
          "in which the value would fall";
      example = "";
    }
  };

/*
6.8 GetCount2d

*/
  struct Getcount2dInfo : OperatorInfo
  {
    inline Getcount2dInfo() :
      OperatorInfo()
    {
      name = "getcount2d";
      signature = Histogram2d::BasicType() + " x " +
            CcInt::BasicType() + " x " +
	          CcInt::BasicType() + " -> " + CcReal::BasicType();
      syntax = "getcount2d(_, _, _)";
      meaning = "Return the count of the specified bin.";
      example = "";
    }
  };

/*
6.9 Insert2d, Insert2dValue

*/
  struct Insert2dInfo : OperatorInfo
  {
    inline Insert2dInfo() :
      OperatorInfo()
    {
      name = "insert2d";
      signature = Histogram2d::BasicType() + " x real x real -> " +
	          Histogram2d::BasicType();
      syntax = "insert2d(_, _, _)";
      meaning = "Increments the bin corresponding to the pair by 1.0";
      example = "";
    }
  };

  struct Insert2dValueInfo : OperatorInfo
  {
    inline Insert2dValueInfo() :
      OperatorInfo()
    {
      name = "insert2dvalue";
      signature = Histogram2d::BasicType() + " x real x real x real -> " +
	          Histogram2d::BasicType();
      syntax = "insert2dvalue(_, _, _, _)";
      meaning = "Increments the bin corresponding to the pair by value";
      example = "";
    }
  };

/*
6.10 MeanX, MeanY

*/
  struct MeanXInfo : OperatorInfo
  {
    inline MeanXInfo() :
      OperatorInfo()
    {
      name = "meanX";
      signature = Histogram2d::BasicType() + " -> " + CcReal::BasicType();
      syntax = "_ meanX";
      meaning = "Compute the bin-weighted arithmetic mean.";
      example = "";
    }
  };

  struct MeanYInfo : OperatorInfo
  {
    inline MeanYInfo() :
      OperatorInfo()
    {
      name = "meanY";
      signature = Histogram2d::BasicType() + " -> " + CcReal::BasicType();
      syntax = "_ meanY";
      meaning = "Compute the bin-weighted arithmetic mean.";
      example = "";
    }
  };

/*
6.11 SetHistogram2d

*/
  struct SetHistogram2dInfo : OperatorInfo
  {
    SetHistogram2dInfo() :
      OperatorInfo()
    {
      name = "set_histogram2d";
      signature = "(stream real) x (stream real) -> " +
                                        Histogram2d::BasicType();
      syntax = "_ _ set_histogram2d";
      meaning = "Creates a histogram2d from two ordered real-streams.";
      example = "";
    }
  };

/*
6.12 ShrinkEager2d, ShrinkLazy2d

*/
  struct ShrinkEager2Info : OperatorInfo
  {
    inline ShrinkEager2Info() :
      OperatorInfo()
    {
      name = "shrink_eager2";
      signature = Histogram2d::BasicType() + " x real x real x real x real-> "
      + Histogram2d::BasicType();
      syntax = "shrink_eager2(_, _, _, _, _)";
      meaning = "constricts the value range to bins "
        "completely contained in [lowerX;upperX[ x [lowerY;upperY[";
      example = "";
    }
  };

  struct ShrinkLazy2Info : OperatorInfo
  {
    inline ShrinkLazy2Info() :
      OperatorInfo()
    {
      name = "shrink_lazy2";
      signature = Histogram2d::BasicType() + " x real x real x real x real -> "
      + Histogram2d::BasicType();
      syntax = "shrink_lazy2(_, _, _, _, _)";
      meaning = "constricts the value range to bins "
        "so that [lX;hX[ x [lY;hY[ is completely contained";
      example = "";
    }
  };

/*
6.13 VarianceX, VarianceY

*/
  struct VarianceXInfo : OperatorInfo
  {
    inline VarianceXInfo() :
      OperatorInfo()
    {
      name = "varianceX";
      signature = Histogram2d::BasicType() + " -> " + CcReal::BasicType();
      syntax = "_ varianceX";
      meaning = "Compute the variance.";
      example = "";
    }
  };

  struct VarianceYInfo : OperatorInfo
  {

    inline VarianceYInfo() :
      OperatorInfo()
    {
      name = "varianceY";
      signature = Histogram2d::BasicType() + " -> " + CcReal::BasicType();
      syntax = "_ varianceY";
      meaning = "Compute the variance.";
      example = "";
    }

  };




}  // namespace Histogram

#endif /*HISTOGRAM2D_H_*/
