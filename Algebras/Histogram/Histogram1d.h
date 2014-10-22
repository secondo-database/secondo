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
//[TOC] [\tableofcontents]

""[1]

[1] Header File Histogram1d

December 2007, S. H[oe]cher, M. H[oe]ger, A. Belz, B. Poneleit


[TOC]

1 Overview

The file "Histogram1d.h" contains declarations of the class and methods
specific to histogram1d objects.

2 Defines and includes

It includes "HistogramBase.h" which contains declarations of functions common
to both histogram1d and histogram2d. "HistogramUtils.h" contains further
helper functions that are not attached to specific classes or operators.

*/
#ifndef HISTOGRAM1D_H_
#define HISTOGRAM1D_H_

#include "HistogramBase.h"
#include "HistogramUtils.h"


namespace hgr
{

/*
3 The class Histogram1d

The class "Histogram1d" is derived from BaseHistogram, thus inheriting
important includes and some basic methods common to 1d and 2d histograms.

*/
  class Histogram1d : public hgr::BaseHistogram
  {
    public:

    Histogram1d(bool _defined, size_t size = 0);
    Histogram1d(const Histogram1d& rhs);
    ~Histogram1d();

    Histogram1d& operator = (const Histogram1d& h);

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

returns TRUE if both the numbers of ranges and the
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
3.1.4 GetRange(int)

returns a pointer to the value of the range at the specified index.

*/
    HIST_REAL GetRange(int i) const;

/*
3.1.5 GetNoRange()

returns the number of ranges of the owning histogram.

*/
    int GetNoRange() const;

/*
3.1.6 FindBin(real)

returns the index number of the bin the given real value would be assigned to.
returns undefined if the parameter value is outside the histogram range.

*/
    CcInt FindBin(const HIST_REAL& val) const;

/*
3.1.7 GetCount(int)

returns the value of the bin indicated by the given index number.
returns undefined if the index is invalid.

*/
    CcReal GetCount(const int i) const;

/*
3.1.8 Compare(BaseHistogram)

compares two histogram1d objects and returns 0, -1 or 1, according to the
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

*/
    int Compare(const BaseHistogram& h) const;

/*
3.1.9 CompareAlmost(Histogram)

works exactly like Compare(Histogram) but performs the comparison of values
by means of the helper function "AlmostEqual(HIST[_]REAL, HIST[_]REAL)",
defined in HistogramUtils.h, thus
allowing to accept slight differences between the real values as "equal".

*/
    int CompareAlmost(const BaseHistogram& h) const;

/*
3.1.10 operators == and $<$

The operator == returns true if CompareAlmost returns 0.
The operator $<$ returns true if CompareAlmost returns -1.

*/
    bool operator ==(const BaseHistogram& h) const;
    bool operator <(const BaseHistogram& h) const;

/*
3.2.11 Distance(BaseHistogram p)

Compares the similarity of two histograms: Returns the sum of the squared
difference values between the couples of bins with equal number.

Example:

histogram1: ((0.0 1.0 2.0)(0.5 0.5))

histogram2: ((0.0 1.0 2.0)(0.6 0.7))

return value: (0.1 x 0.1) + (0.2 x 0.2) = 0.001 + 0.004 = 0.005

*/
    HIST_REAL Distance(const BaseHistogram* h);

/*
3.2.12 Mean()

computes the bin-weighted arithmetic mean M of a histogram using the
recurrence relation

        $M(n) = M(n-1) + (x[n] - M(n-1)) (w(n)/(W(n-1) + w(n)))$

        $W(n) = W(n-1) + w(n)$

We skip negative values in the bins, since those correspond to negative
weights (BJG).

The implementation is taken from gsl[_]histogram[_]stat.c of the
GNU Scientific Library (gsl-1.9), Author: Simone Piccardi, Jan. 2000

*/
    CcReal Mean() const;

/*
3.2.13 Variance()

computes the variance of the histogram. We skip negative values in the bins,
since those correspond to negative weights (BJG).

The implementation is taken from gsl[_]histogram[_]stat.c of the
GNU Scientific Library (gsl-1.9), Author: Simone Piccardi, Jan. 2000

*/
    CcReal Variance() const;

/*
3.3 Construction and manipulation

3.3.1 AppendRange(real)

appends the given real value to the array of ranges.

*/
    void AppendRange(const HIST_REAL& r);

/*
3.3.2 ResizeRange(int)

sets the initial size of the ranges array to the given integer value.

*/
    void ResizeRange(const int newSize);

/*
3.3.3 Clear()

clears the arrays of ranges and bins.

*/
    void Clear();

/*
3.3.4 Destroy()

destroys the histogram, i.e. it's ranges and bins arrays.

*/
    void Destroy();

/*
3.3.5 Insert(int, real), Insert(real, real)

The first function inserts the given real value into the bin with the given
index. The second function adds the real value given as second parameter to
the appropriate bin for the first parameter. It returns TRUE if such a bin has
been found, and FALSE if not.

*/
    void Insert(const int bin, const HIST_REAL& weight = 1.0);
    bool Insert(const HIST_REAL& val, const HIST_REAL& weight = 1.0);

/*
3.3.6 Coarsen(BaseHistogram)

coarsens the owning histogram to the pattern of the histogram given as
parameter. The function requires as a precondition that the owning histogram
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

3.4 Mandatory functions

*/
    static Word In (  const ListExpr typeInfo,
                      const ListExpr instance,
                      const int errorPos,
                      ListExpr& errorInfo,
                      bool& correct );

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

    inline static const string BasicType() { return "histogram1d";}

    static const bool checkType(ListExpr list){
      return listutils::isSymbol(list, BasicType());
    }



private:

    friend struct ConstructorFunctions<Histogram1d>;

    DbArray<HIST_REAL> range;

    Histogram1d();

  }; //class Histogram1d : public StandardAlgebra

  ostream& operator << (ostream& os, const Histogram1d& h);


/*

4 ConstructorInfo and functions structs

*/
  struct histogram1dInfo : ConstructorInfo
  {
    histogram1dInfo();
  }; // struct histogram1dInfo : ConstructorInfo {

  struct histogram1dFunctions : ConstructorFunctions<Histogram1d>
  {
    histogram1dFunctions();
  }; // struct histogram1dFunctions : ConstructorFunctions<Histogram1d> {


/*
5 Type and value mapping functions of the operators

5.1 SetHistogram1d

*/
  ListExpr SetHistogram1dTypeMap(ListExpr args);
  int SetHistogram1dFun(Word* args, Word& result, int message, Word& local,
      Supplier s);

/*
5.2 NoComponents

*/
  ListExpr NoComponentsTypeMap(ListExpr args);
  int NoComponentsFun(Word* args, Word& result, int message, Word& local,
      Supplier s);

/*
5.3 binrange[_]min, binrange[_]max

*/
  ListExpr binrange_min_maxTypeMap(ListExpr args);
  int binrange_minFun(Word* args, Word& result, int message, Word& local,
      Supplier s);
  int binrange_maxFun(Word* args, Word& result, int message, Word& local,
      Supplier s);

/*
5.4 CreateHistogram1d

*/
  ListExpr CreateHistogram1dTypeMap(ListExpr args);
  int CreateHistogram1dFun(Word* args, Word& result, int message, Word& local,
      Supplier s);

/*
5.5 CreateHistogram1dEquicount

*/
  int CreateHistogram1dEquicountFun(Word* args, Word& result, int message,
      Word& local, Supplier s);
  ListExpr CreateHistogram1dEquicountTypeMap(ListExpr args);

/*
5.6 CreateHistogram1dEquiwidth

*/
  ListExpr CreateHistogram1dEquiwidthTypeMap(ListExpr args);
  int CreateHistogram1dEquiwidthFun(Word* args, Word& result, int message,
      Word& local, Supplier s);

/*
5.7 FindBin

*/
  int FindBinFun(Word* args, Word& result, int message, Word& local,
      Supplier s);

/*
5.8 FindMinMaxBin

*/
  template<bool isMin>
  int FindMinMaxBinFun1d(Word* args, Word& result, int message, Word& local,
        Supplier s);

/*
5.9 Getcount1d

*/
  ListExpr Getcount1dTypeMap(ListExpr args);
  int Getcount1dFun(Word* args, Word& result, int message, Word& local,
      Supplier s);

/*
5.10 Insert1d

*/
  template<bool incValSupplied> ListExpr Insert1dTypeMap(ListExpr args);
  template<bool incValSupplied> int Insert1dFun(Word* args, Word& result,
      int message, Word& local, Supplier s);

/*
5.11 Mean

*/
  int MeanFun(Word* args, Word& result, int message, Word& local, Supplier s);

/*
5.11 Shrink1d

*/
  ListExpr Shrink1dTypeMap(ListExpr args);
  template<bool eager> int Shrink1dFun(Word* args, Word& result, int message,
      Word& local, Supplier s);

/*
5.12 Variance

*/
  ListExpr Hist1dRealTypeMap(ListExpr args);
  int VarianceFun(Word* args, Word& result, int message, Word& local,
                  Supplier s);

/*

6 Info structs

6.1 SetHistogram1d

*/
  struct SetHistogram1dInfo : OperatorInfo
  {
    inline SetHistogram1dInfo() :
      OperatorInfo()
    {
      name = "set_histogram1d";
      signature = "(stream real) -> " + Histogram1d::BasicType();
      syntax = "_ set_histogram1d";
      meaning = "Creates a histogram1d from an ordered stream of real";
      example = "";
    }
  };

/*
6.2 NoComponents

*/
  struct NoComponentsInfo : OperatorInfo
  {
    inline NoComponentsInfo() :
      OperatorInfo()
    {
      name = "no_components";
      signature = Histogram1d::BasicType() + " -> " + CcInt::BasicType();
      syntax = "no_components(_)";
      meaning = "Returns number of components.";
      example = "";
    }
  };

/*
6.3 binrange[_]min

*/
  struct binrange_minInfo : OperatorInfo
  {
    // constructor
    inline binrange_minInfo() :
      OperatorInfo()
    {
      name = "binrange_min";
      signature = "histogram1d x int -> real";
      syntax = "binrange_min(_, _)";
      meaning = "Returns the histogram's lower range of the given interval.";
    } // binrange_minInfo() : OperatorInfo() {
  }; // struct binrange_minInfo : OperatorInfo {

/*
6.4 binrange[_]max

*/
  struct binrange_maxInfo : OperatorInfo
  {
    // constructor
    inline binrange_maxInfo() :
      OperatorInfo()
    {
      name = "binrange_max";
      signature = "histogram1d x int -> real";
      syntax = "binrange_max(_, _)";
      meaning = "Returns the histogram's upper range of the given interval.";
    } // binrange_minInfo() : OperatorInfo() {
  }; // struct binrange_minInfo : OperatorInfo {

/*
6.5 CreateHistogram1d

*/
  struct CreateHistogram1dInfo : OperatorInfo
  {
    inline CreateHistogram1dInfo() :
      OperatorInfo()
    {
      name = "create_histogram1d";
      signature = "stream(tuple(X)) x ai x " +
      Histogram1d::BasicType() + " -> " + Histogram1d::BasicType();
      syntax = "_ create_histogram1d [_, _]";
      meaning = "Fills a histogram1d from an ordered stream of tuples";
      example = "";
    }
  };

/*
6.6 CreateHistogram1dEquicount

*/
  struct CreateHistogram1dEquicountInfo : OperatorInfo
  {
    inline CreateHistogram1dEquicountInfo() :
      OperatorInfo()
    {
      name = "create_histogram1d_equicount";
      signature = "stream(tuple(X)) x ai x int -> " + Histogram1d::BasicType();
      syntax = "_ create_histogram1d_equicount [_, _]";
      meaning = "Creates a histogram1d from a stream of tuples;"
        " all categories will have equal height";
      example = "";
    }
  };

/*
6.7 CreateHistogram1dEquiwidth

*/
  struct CreateHistogram1dEquiwidthInfo : OperatorInfo
  {
    inline CreateHistogram1dEquiwidthInfo() :
      OperatorInfo()
    {
      name = "create_histogram1d_equiwidth";
      signature = "((stream (tuple([a1:t1, ..., ai:real, ... ,an:tn]))) x "
        "ai x n:int) -> (histogram1d)";
      syntax = "_ create_histogram1d_equiwidth [_,_]";
      meaning = "Returns a histogram1d with max. n "
        "ranges and equal width. ai represents the input-value.";
      example = "";
    }
  };

/*
6.8 FindBin

*/
  struct FindBinInfo : OperatorInfo
  {
    inline FindBinInfo() :
      OperatorInfo()
    {
      name = "findbin";
      signature = Histogram1d::BasicType() + " x real -> int";
      syntax = "findbin(_, _)";
      meaning = "Returns the index of the bin, in which the value would fall";
      example = "";
    }
  };

/*
6.9 Getcount1d

*/
  struct Getcount1dInfo : OperatorInfo
  {
    inline Getcount1dInfo() :
      OperatorInfo()
    {
      name = "getcount1d";
      signature = Histogram1d::BasicType() + " x " + CcInt::BasicType() +
	          " -> " + CcReal::BasicType();
      syntax = "getcount1d(_, _)";
      meaning = "Return the count of the specified bin.";
      example = "";
    }
  };

/*
6.10 Insert1d

*/
  struct Insert1dInfo : OperatorInfo
  {
    inline Insert1dInfo() :
      OperatorInfo()
    {
      name = "insert1d";
      signature = Histogram1d::BasicType() + " x real -> " +
                                              Histogram1d::BasicType();
      syntax = "insert1d (_, _)";
      meaning = "Increments the bin corresponding to the value by 1.0";
      example = "";
    }
  };

/*
6.11 Insert1dValue

*/
  struct Insert1dValueInfo : OperatorInfo
  {
    inline Insert1dValueInfo() :
      OperatorInfo()
    {
      name = "insert1dvalue";
      signature = Histogram1d::BasicType() + " x real x real -> " +
	          Histogram1d::BasicType();
      syntax = "insert1dvalue (_, _, _)";
      meaning = "Increments the bin corresponding to the value by val";
      example = "";
    }
  };

/*
6.12 Mean

*/
  struct MeanInfo : OperatorInfo
  {

    inline MeanInfo() :
      OperatorInfo()
    {
      name = "mean";
      signature = Histogram1d::BasicType() + " -> " + CcReal::BasicType();
      syntax = "_ mean";
      meaning = "Compute the bin-weighted arithmetic mean.";
      example = "";
    }

  };

/*
6.13 ShrinkEager

*/
  struct ShrinkEagerInfo : OperatorInfo
  {
    inline ShrinkEagerInfo() :
      OperatorInfo()
    {
      name = "shrink_eager";
      signature = Histogram1d::BasicType() + " x real x real -> " +
	          Histogram1d::BasicType();
      syntax = "shrink_eager(_, _, _)";
      meaning = "constricts the value range to bins "
        "completely contained in [lower;upper[";
      example = "";
    }
  };

/*
6.14 ShrinkLazy

*/
  struct ShrinkLazyInfo : OperatorInfo
  {
    inline ShrinkLazyInfo() :
      OperatorInfo()
    {
      name = "shrink_lazy";
      signature = Histogram1d::BasicType() + " x real x real -> " +
	          Histogram1d::BasicType();
      syntax = "shrink_lazy(_, _, _)";
      meaning = "constricts the value range to bins "
        "so that [lower;upper[ is completely contained";
      example = "";
    }
  };

/*
6.15 Variance

*/
  struct VarianceInfo : OperatorInfo
  {
    inline VarianceInfo() :
      OperatorInfo()
    {
      name = "variance";
      signature = Histogram1d::BasicType() + " -> " + CcReal::BasicType();
      syntax = "_ variance";
      meaning = "Compute the variance.";
      example = "";
    }
  };

} // namespace hgr


#endif /*HISTOGRAM1D_H_*/
