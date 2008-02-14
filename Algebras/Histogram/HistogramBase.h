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
//[<][\<]

//characters [1] verbatim:   [\verb@]    [@]

""[1]

[1] Header File of the Histogram Algebra

December 2007, S. H[oe]cher, M. H[oe]ger, A. Belz, B. Poneleit


[TOC]

1 Overview 

The file "HistogramBase.h" contains basic defines and includes as well as 
declarations of functions and operators common to both 1d and 2d histograms. 
This documentations will explain their interfaces ("what") in the header file 
and restrict itself to shere commentaries ("how") in the implementation file 
"BaseHistogram.cpp".


2 Defines and includes

*/
#ifndef HISTOGRAMBASE_H_
#define HISTOGRAMBASE_H_


#include "Symbols.h"
#include "Algebra.h"
#include "NestedList.h"
#include "NList.h" 
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h" 
#include "StandardTypes.h"
#include "DBArray.h"
#include "RelationAlgebra.h"
#include <vector>
#include <list>
#include <set>
#include <queue>
#include <limits.h>


#ifndef HIST_REAL
#define HIST_REAL double
#endif // #ifndef HIST_REAL

namespace hgr {

  class BaseHistogram : public StandardAttribute
  {
public:
  
/*  

3 Functions   

The functions and operators mentioned in this section are applicable to 
any kind of histogram ("BaseHistogram").

3.1 Constructors and destructor
  
*/  
    inline BaseHistogram() {}
    inline BaseHistogram(const BaseHistogram& rhs) {}
    virtual ~BaseHistogram();

/*    
3.2 Functions and operators  

3.2.1 GetBin(int)

Takes a bin index number and 
returns a pointer to the value of this bin.
    
*/    
    const HIST_REAL* GetBin(int i) const;
/*
3.2.2 AppendBin(HIST[_]REAL)

Appends the passed real value to the array of bins.

*/
    void AppendBin(const HIST_REAL& b);
/*
3.2.3 GetNoBin()

Returns the number of bins in the histogram.
    
*/    
    int GetNoBin() const;
/*
3.2.4 ResizeBin(int)

Resizes the bin array to the given initial size.

*/    
    void ResizeBin(const int newSize);
/*
3.2.5 IsDefined()

Returns TRUE if the histogram is defined and FALSE if it is not.

*/    
    bool IsDefined() const;
/*
3.2.6 SetDefined(bool)

Sets the histogram's "defined" value to TRUE or FALSE, according to the given 
bool value.
 
*/    
    void SetDefined(bool defined);    
/*
3.2.7 Distance(BaseHistogram p)

Compares the similarity of two histograms: Returns the sum of the squared 
difference values between the couples of bins with equal number.

Example:

histogram1: ((0.0 1.0 2.0)(0.5 0.5))

histogram2: ((0.0 1.0 2.0)(0.6 0.7))

return value: (0.1 x 0.1) + (0.2 x 0.2) = 0.001 + 0.004 = 0.005 

*/    
    HIST_REAL Distance(const BaseHistogram* h); 
/*    
3.2.8 GetMinBin(), GetMaxBin()

Returns the number of the bin with the lowest(highest) value. If there is more 
than one bin with equal low (high) value, the number of the first of them 
is returned.

*/
    CcInt GetMinBin() const;
    CcInt GetMaxBin() const;
/*    
3.2.9 CopyBinsFrom(BaseHistogram)

Copies all bin values from the given histogram and appends them to the 
calling histogram's bin array.

*/
    void CopyBinsFrom(const BaseHistogram* h);
/*
3.2.10 Virtual functions

to be overwritten in histogram1d and histogram2d:

*/

    virtual void CopyRangesFrom(const BaseHistogram* h) = 0;
    
    virtual bool IsEmpty() const = 0;
    virtual bool IsConsistent(const bool checkOrder = true) const = 0;
    
    virtual int Compare(const BaseHistogram& h) const = 0;
    virtual int CompareAlmost(const BaseHistogram& h) const = 0;
    virtual bool IsRefinement(const BaseHistogram& h) const = 0;

    virtual bool operator ==(const BaseHistogram& h) const = 0;
    virtual bool operator <(const BaseHistogram& h) const = 0;

    virtual void Coarsen(const BaseHistogram* h) = 0;
    
    virtual ostream& Print( ostream& os ) const = 0;
    
/*    
3.2.11 Definition of bin

The bin array is declared here, whereas the ranges array depends on the 
histogram's number of dimensions. 

*/
protected:
    DBArray<HIST_REAL> bin;
    bool defined;

/*
3.2.12 CmpBinSearch(real, real)

The function takes two pointers to real values, compares them and returns 
-1 (the first real value is smaller than the second) or 1 (else).

It is passed as comparison function to the method DBArray::Find() 
to find the right bin for a given value using binary search.

*/
    static int CmpBinSearch(const void* v1, const void* v2);

  }; //  class BaseHistogram : public StandardAttribute
  
/*    
          
2.4 Operators    

2.4.1 is[_]undefined

Sets the result value to (true, true) if the given histogram is defined, 
and to (true, false) if it is not. 

*/      
  ListExpr is_undefinedTypeMap( ListExpr args );
  
  int is_undefinedSelect( ListExpr args );  
  
  int is_undefined1dFun ( Word* args, Word& result, int message, 
                        Word& local, Supplier s ); 

  
  int is_undefined2dFun(Word* args, Word& result, int message, Word& local,
      Supplier s);



/*
2.4.2 IsEqual and IsLess (operators = and $<$)

Compares two histograms of the same type; the same type is assured by the 
HistHistBoolTypeMap function. The operator returns TRUE, if the histograms are 
equal or the first is smaller than the second, and FALSE else.

Two histograms are equal if their ranges are equal and the corresponding pairs 
of bins are equal.

Histgram A is smaller than histogram B if their ranges are equal and each bin 
value in A is smaller than the corresponding bin value in B.
  
  
*/
  ListExpr HistHistBoolTypeMap(ListExpr args);
  
  int IsEqualFun(Word* args, Word& result, int message, Word& local, 
      Supplier s);

  int IsLessFun(Word* args, Word& result, int message, Word& local,
      Supplier s);   

/*
2.4.3 is[_]refinement

Sets the result value to (true, true) if the first given histogram is a 
refinement of the second one.

This is the case if all ranges of the first histogram are completely contained 
in the ranges of the second histogram.

*/
  int IsRefinementFun(Word* args, Word& result, int message, Word& local,
      Supplier s);  
  
/*  
2.4.4 translate

Transforms the first histogram into the second, provided that the first is a 
refinement of the second. I.e., the first histogram will be coarsed: If in the 
first histogram bin 1 = 200.0 and bin 2 = 100.0 and their ranges get merged to 
one common range, its bin value will be 300.0.

*/
  ListExpr TranslateTypeMap(ListExpr args);

  inline int TranslateSelect(ListExpr args)
  {
    NList type(args);
    if (type.first().isSymbol(HISTOGRAM2D) )
      return 1;
    else
      return 0;
  }
  
  int TranslateFun(Word* args, Word& result, int message, Word& local,
          Supplier s);
  
/*      
2.4.5 use

Transforms the given histogram by recalculating all its bins as defined by 
the given parameter function with at least one bin value parameter. 
This allows f. ex. to scale the histogram. 

*/
  ListExpr UseTypeMap(ListExpr args);

  int UseFun(Word* args, Word& result, int message, Word& local, Supplier s);

/*  
2.4.6 use2

Works like "use" but accepts two histograms of the same type, which need to 
have the same ranges or one to be a refinement of the other. If the latter is 
the case, "use2" first uses "translate" to coarsen the finer one. If that is 
not possible, the operator returns "UNDEF".

The parameter function expects at least two bin values, one for each histogram. 
The operator allows thus f. ex. to add two histograms by adding their 
corresponding bin values.  

*/  
  ListExpr Use2TypeMap(ListExpr args);
  
  int Use2Fun(Word* args, Word& result, int message, Word& local, Supplier s);

/*  
2.4.7 fold

Expects three parameters: a histogram, a parameter function and a start value. 
The parameter function accepts two real values, representing an intermediate 
value and a bin value.

If the given histogram is not defined, the operator returns UNDEF. Otherwise it 
assigns the start value to the first function parameter and the content of the 
first bin the second function parameter, executes the function, assigns its 
result to the first function parameter and the content of the next bin to the 
second, thus taking all bin values into account. The return value is the result 
of the last function call.  

*/  
  ListExpr FoldTypeMap(ListExpr args);
  
  int FoldFun(Word* args, Word& result, int message, Word& local, Supplier s);

/*
2.4.8 distance

Calculates the distance between two histograms of equal type.

If the two histograms are equal, the distance is 0.0. If one is a refinement 
of the other, it is first "translated" (coarsened) to make it comparable to 
the other. If none of the histograms is a refinement of the other, the result 
is UNDEF.

The distance is calculated by summing up the squared differences between 
corresponding bins.

Example:

hist1 = ((0.0 1.0 2.0)(5.0 6.0))

hist2 = ((0.0 1.0 2.0)(5.2 6.1))

result = (0.2 x 0.2) + (0.1 x 0.1) = 0.04 + 0.01 = 0.05

*/  
  ListExpr DistanceTypeMap(ListExpr args);
  
  int DistanceFun(Word* args, Word& result, int message, Word& local,
            Supplier s);
        
/* 
2.4.9 The type mapping function of findbin, findbinX and findbinY

*/  
  template<bool histogram1d>
  ListExpr FindBinTypeMap(ListExpr args);  

/*  
2.4.10 find[_]minbin, find[_]maxbin

Expects a histogram and returns a stream of integer values (histogram1d) or 
of couples of integer values (histogram2d) that designates the index numbers 
of all bins having minimum resp. maximum values. 

*/  
  ListExpr FindMinMaxBinTypeMap(ListExpr args);
  
  inline int FindMinMaxBinSelect(ListExpr args)
  {
    NList type(args);
    
    if (type.first().isSymbol(HISTOGRAM1D))
      return 0;
    else
      return 1;
  }    
  
/*
3.5 Helper functions and classes for operators

3.5.1 GetResultTuple2d()

Returns the ListExpr: (tuple ((X int)(Y int)))
It is used by the operators find[_]minbin and find[_]maxbin 
to construct the resultstream.

*/
  ListExpr GetResultTuple2d();

/*
3.5.2 GetResultTupleTypeInfo2d()

Returns the numeric type of the ListExpr: (tuple ((X int)(Y int)))
using the class SecondoCatalog.
It is used by the operators find[_]minbin and find[_]maxbin
to construct the resultstream.

*/
  ListExpr GetResultTupleTypeInfo2d();

/*  
3.5.3 class MinMaxBuffer

Represents the local storage of the operators find[_]minbin and find[_]maxbin.

*/
  class MinMaxBuffer
  {
  public:
    
    MinMaxBuffer(const CcInt& _index, 
                 const HIST_REAL& _value,
                 const ListExpr& _tupleTypeInfo = 0) :
    index(_index), value(_value), tupleTypeInfo(_tupleTypeInfo)
    {
      
    }
    
    CcInt index;
    const HIST_REAL value;
    const ListExpr tupleTypeInfo;
  };  
  
/*
4 Info structs  

4.1 is[_]undefinedInfo
  
*/  
  struct is_undefinedInfo : OperatorInfo {
    // constructor
    is_undefinedInfo() : OperatorInfo() {
      name = IS_UNDEFINED;
      signature = "histogram1d -> bool";
      appendSignature( "histogram2d -> bool" );
      syntax = IS_UNDEFINED + "(_)";
      meaning = "Returns TRUE if the histogram is undefined.";
    } // is_undefinedInfo() : OperatorInfo() {
  }; // struct is_undefinedInfo : OperatorInfo {    

/*
4.2 isRefinementInfo
    
*/ 
  struct IsRefinementInfo : OperatorInfo
  {
    inline IsRefinementInfo() :
      OperatorInfo()
    {
      name = "is_refinement";
      signature = HISTOGRAM1D + " x " + HISTOGRAM1D + " -> " + BOOL;
      appendSignature(HISTOGRAM2D + " x " + HISTOGRAM2D + " -> " + BOOL);
      syntax = "is_refinement(_, _)";
      meaning = "Returns true, if the first histogram is a refinement "
        "of the second histogram.";
      example = "";
    }
  };

/*
4.3 isEqualInfo
      
*/
  struct IsEqualInfo : OperatorInfo
  {
    inline IsEqualInfo() :
      OperatorInfo()
    {
      name = "=";
      signature = HISTOGRAM1D + " x " + HISTOGRAM1D + " -> " + BOOL;
      appendSignature(HISTOGRAM2D + " x " + HISTOGRAM2D + " -> " + BOOL);
      syntax = "_ = _";
      meaning = "Equals predicate.";
      example = "";
    }
  };

/*
4.4 isLessInfo

*/   
  struct IsLessInfo : OperatorInfo
  {
    inline IsLessInfo() :
      OperatorInfo()
    {
      name = "<";
      signature = HISTOGRAM1D + " x " + HISTOGRAM1D + " -> " + BOOL;
      appendSignature(HISTOGRAM2D + " x " + HISTOGRAM2D + " -> " + BOOL);
      syntax = "_ < _";
      meaning = "Less predicate.";
      example = "";
    }
  };

/*
4.5 TranslateInfo

*/
  struct TranslateInfo : OperatorInfo
  {
    inline TranslateInfo() :
      OperatorInfo()
    {
      name = "translatehistogram";
      signature = "histogram1d x histogram1d -> histogram1d";
      appendSignature("histogram2d x histogram2d -> histogram2d");
      syntax = "_ translatehistogram (_)";
      meaning = "Coarsens the first argument to a histogram of the same type"
        "as the second argument.";
      example = "";
    }
  };  

/*
4.6 UseInfo

*/
  struct UseInfo : OperatorInfo
  {
    inline UseInfo() :
      OperatorInfo()
    {
      name = "usehistogram";
      signature = "For T in kind DATA: " + HISTOGRAM1D
          + " x (real x T* -> real) -> " + HISTOGRAM1D;
      appendSignature(HISTOGRAM2D + " x (real x T* -> real) -> "
          + HISTOGRAM2D);
      syntax = "_ use[ list; fun ]";
      meaning = "Apply the function fun "
        "to each bin of the first argument.";
      example = "";
    }
  };  
  
/*
4.7 Use2Info

*/  
  struct Use2Info : OperatorInfo
  {
    inline Use2Info() :
      OperatorInfo()
    {
      name = "usehistogram2";
      signature = "For T in kind DATA: " + HISTOGRAM1D + " x " + HISTOGRAM1D
          + " x (real x real x T* -> real) -> " + HISTOGRAM1D;
      appendSignature(HISTOGRAM2D + " x (real x real x T* -> real) -> "
          + HISTOGRAM2D);
      syntax = "_ _ use2[ list; fun ]";
      meaning = "Apply the function fun "
        "to each bin of the first and the second argument.";
      example = "";
    }
  };

/*
4.8 FoldInfo

*/
  struct FoldInfo : OperatorInfo
  {
    inline FoldInfo() :
      OperatorInfo()
    {
      name = "fold";
      signature = "histogram1d x (T x real -> T) x T -> T";
      appendSignature("histogram2d x (T x real -> T) x T -> T");
      syntax = "_ fold [_; _]";
      meaning = "Apply the fold function to the histogram, "
        "T of kind DATA";
      example = "";
    }
  };     

/*
4.9 DistanceInfo

*/
  struct DistanceInfo : OperatorInfo
  {
    inline DistanceInfo() :
      OperatorInfo()
    {
      name = "distance";
      signature = "histogram1d x histogram1d -> real";
      appendSignature("histogram2d x histogram2d -> real");
      syntax = "distance (_, _)";
      meaning = "Calculate the distance between two histograms.";
      example = "";
    }
  };

/*
4.6 FindMinBinInfo

*/  
  struct FindMinBinInfo : OperatorInfo
  {
    inline FindMinBinInfo() :
      OperatorInfo()
    {
      name = "find_minbin";
      signature = HISTOGRAM1D + " -> " + STREAM + "(" + INT + ")";
      appendSignature(HISTOGRAM2D + " -> " + STREAM + "(" + TUPLE + "((" + "x "
          + INT + ")(" + "y " + INT + ")))");
      syntax = "_ find_minbin";
      meaning = "Returns a stream of the indices of all bins "
        "with minimal value.";
      example = "";
    }
  };

/*
4.7 FindMaxBinInfo

*/
  struct FindMaxBinInfo : OperatorInfo
  {
    inline FindMaxBinInfo() :
      OperatorInfo()
    {
      name = "find_maxbin";
      signature = HISTOGRAM1D + " -> " + STREAM + "(" + INT + ")";
      appendSignature(HISTOGRAM2D + " -> " + STREAM + "(" + TUPLE + "((" + "x "
          + INT + ")(" + "y " + INT + ")))");
      syntax = "_ find_maxbin";
      meaning = "Returns a stream of the indices of all bins "
        "with maximal value.";
      example = "";
    }
  };

}  // namespace hgr

#endif /*HISTOGRAMBASE_H_*/
