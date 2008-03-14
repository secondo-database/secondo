
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
//[&][\&]

//[_][\_]
//characters [1] verbatim:   [\verb@]    [@]

""[1]



[1] Implementation file of the Histogram Algebra

December 2007, S. H[oe]cher, M. H[oe]ger, A. Belz, B. Poneleit

1 Defines and includes 

The TEST macro serves to enable and disable test output: To enable test output, 
uncomment the line "#define TEST(x, y) cerr << x << cerr << y << endl;" 
and comment the second one. To switch off test output, uncomment the line 
"#define TEST(x, y)" und comment the first one.

*/

#define TEST(x, y) cerr << x << y << endl;
// #define TEST(x, y)


#include "Histogram1d.h"

namespace hgr
{
  
/*
  
2 Type Constructor

*/  
  histogram1dInfo h1_i;
  histogram1dFunctions h1_f;
  TypeConstructor histogram1dTC( h1_i, h1_f );  

/*
 
3  Constructors and destructor
 
*/  
 
  Histogram1d::Histogram1d()
  {
  }

  Histogram1d::Histogram1d(bool _defined, size_t size) :
    BaseHistogram(_defined, size), range(size + 1)
  {
  }

  Histogram1d::Histogram1d(const Histogram1d& rhs) : 
    BaseHistogram(rhs), range(rhs.range.Size())
  {
    for (int i = 0; i < rhs.GetNoRange(); i++)
    {
      range.Append( *rhs.GetRange(i) );
    }
  }

  Histogram1d::~Histogram1d()
  {
  }
  
  Histogram1d& Histogram1d::operator = (const Histogram1d& h)
  {
    CopyFrom(&h);
    return *this;
  }

/*  
  
3 Helper functions  

*/  
  void Histogram1d::AppendRange(const HIST_REAL& r)
  {
    range.Append(r);
  }

  void Histogram1d::Clear()
  {
    range.Clear();
    bin.Clear();
  }
  
/*

  "Compare(const BaseHistogram[&] b)": compares two histgram1d objects and 
   returns 0, -1 or 1, according to the result of the comparison:
   
  * 0: this == b: both objects are not defined or empty or all bin values 
                    are equal.

  * -1: this < b: this object is not defined or at least one bin value of this 
                    object is smaller than the corresponding bin value of 
                    object b.
    
  * 1: else: object b is not defined or one object is not consistent or the 
               number of ranges differs or the first ranges of both objects 
               differ or at least one range differs or at least one bin value 
               of this object is greater than the corresponding bin value of 
               object b. 

*/
  
  int Histogram1d::Compare(const BaseHistogram& b) const
  {
    const Histogram1d* h = static_cast<const Histogram1d*>(&b);

    if ( !IsDefined() && !h->IsDefined() )
      return 0;
    if ( !IsDefined() && h->IsDefined() )
      return -1;
    if (IsDefined() && !h->IsDefined() )
      return 1;

    // Now, both histograms are defined ...

    if (!IsConsistent(false) || !h->IsConsistent(false))
      return 1;

    // ... and consistent (without testing the order of ranges) ...

    if (GetNoRange() != h->GetNoRange())
      return 1;

    // ... and are of equal size ...

    if (IsEmpty() && h->IsEmpty())
      return 0;

    // ... and are not empty.

    // Now, comparison starts: first of the first couple of ranges: 
    if (*GetRange(0) != *h->GetRange(0))
      return 1;

    // and now of all other couples of ranges and bins: 
    // (a consistent histogram has at least two range values) 

    int i = 0;
    bool equalBins = true;
    bool lessBins = true;

    while ((equalBins || lessBins) && i < GetNoBin())
    {
      if (*GetRange(i+1) != *h->GetRange(i+1))
        return 1;

      if (*GetBin(i) == *h->GetBin(i))
      {
        lessBins = false;
      }
      else if (*GetBin(i) < *h->GetBin(i))
      {
        equalBins = false;
      }
      else // *GetBin(i) > *h->GetBin(i)
      {
        equalBins = false;
        lessBins = false;
      }

      i++;
    }

    if (equalBins)
      return 0;

    if (lessBins)
      return -1;

    // neither less nor equal:
    return 1;
  }

  int Histogram1d::CompareAlmost(const BaseHistogram& b) const
  {
    const Histogram1d* h = static_cast<const Histogram1d*>(&b);
    
    if( !IsDefined() && !h->IsDefined() )
      return 0;
    if( !IsDefined() && h->IsDefined() )
      return -1;
    if( IsDefined() && !h->IsDefined() )
      return 1;
    
    // Now, both histograms are defined ...
    
    if (!IsConsistent(false) || !h->IsConsistent(false))
      return 1;
    
    // ... and consistent (without testing the order of ranges) ...

    if (GetNoRange() != h->GetNoRange())
      return 1;
    
    // ... and are of equal size ...
    
    if (IsEmpty() && h->IsEmpty())
      return 0;
    
    // ... and are not empty.
    
    // Now, comparison starts: first of the first couple of ranges: 
    if (!AlmostEqual(GetRange(0), h->GetRange(0)))
      return 1;
    
    // and now of all other couples of ranges and bins: 
    // (a consistent histogram has at least two range values) 
    
    int cmp;
    int i = 0;
    bool equalBins = true;
    bool lessBins = true;
    
    while ((equalBins || lessBins) && i < GetNoBin())
    {
      if (!AlmostEqual(GetRange(i+1), h->GetRange(i+1)))
        return 1;

      cmp = CmpReal(GetBin(i), h->GetBin(i));

      if (cmp == 0)
      {
        lessBins = false;
      }
      else if (cmp == -1)
      {
        equalBins = false;
      }
      else // cmp == 1
      {
        equalBins = false;
        lessBins = false;
      }

      i++;
    }
    
    if (equalBins)
      return 0;

    if (lessBins)
      return -1;

    // neither less nor equal:
    return 1;
  }
  
  
  void Histogram1d::Destroy()
  {
    range.Destroy();
    bin.Destroy();
  }

  CcReal Histogram1d::GetCount(const int i) const
  {
    if (!IsDefined() || IsEmpty() || !IsConsistent(false))
      return CcReal(false, 0.0);
    
    if (i < 0 || i >= GetNoBin())
      return CcReal(false, 0.0);
    
    return CcReal(true, *GetBin(i));
  }  
  
  int Histogram1d::GetNoRange() const
  {
    return range.Size();
  }
  
  const HIST_REAL* Histogram1d::GetRange(int i) const
  {
    assert( 0 <= i && i < GetNoRange() );

    const HIST_REAL* p;
    range.Get(i, p);
    return p;
  }

/*

  "IsConsistent(...)": returns TRUE if the histogram1d is either empty or 
   all following conditions are true:
     
  * The number of ranges is 2 or more ( GetNoRange() > 1 ).

  * The number of ranges is one more than the number of bins 
      ( GetNoRange() == GetNoBin()+1 ).
    
  * The range array is sorted in ascending order.
    
  * There are no duplicate ranges.  

*/
  
  bool Histogram1d::IsConsistent(const bool checkOrder) const
  {
    if (IsEmpty())
      return true;
    
    if (!(GetNoRange() > 1 && GetNoRange() == GetNoBin() + 1))
      return false;
    
    if (checkOrder)
    {
      const HIST_REAL* min = GetRange(0);
      for (int i = 1; i < GetNoRange(); i++)
      {
        if (CmpReal(GetRange(i), min) < 1) // GetRange(i) <= min
          return false;

        min = GetRange(i);
      }
    }
    
    return true;
  }

  bool Histogram1d::IsEmpty() const
  {
    return (range.Size() == 0) && (bin.Size() == 0);
  }

  void Histogram1d::ResizeRange(const int newSize)
  {
    if (newSize > 0)
    {
      range.Clear();
      range.Resize(newSize);
    }
  }

  bool Histogram1d::IsRefinement(const BaseHistogram& b) const
  {
    const Histogram1d* h = static_cast<const Histogram1d*>(&b);
    
    if ( !IsDefined() || !h->IsDefined() )
      return false;

    // Both histograms are defined ...

    if (!IsConsistent(false) || !h->IsConsistent(false))
      return false;

    // and consistent (without testing the correct order)...

    if (IsEmpty() || h->IsEmpty())
      return false;

    // and not empty ...
    
    if (!AlmostEqual(GetRange(GetNoRange()-1), 
                     h->GetRange(h->GetNoRange()-1)))
      return false;
    
    // and have identical ends.
    
    // Now we check each element of the given range array to be also an element
    // of our own array. If that is the case, our own histogram is a 
    // refinement of the given one.
    
    int i = 0; // Index of the range arry given as parameter.
    int j = 0; // Index of our own range array.
    
    while (i < h->GetNoRange())
    {
      while (!AlmostEqual(h->GetRange(i), this->GetRange(j)))
      {
        if (++j == this->GetNoRange())
          return false;
      }
      
      i++;
    }
    
    return true;
  }
  
  
  bool Histogram1d::operator == (const BaseHistogram& h) const
  {
    return CompareAlmost(h) == 0;
  }
  
  bool Histogram1d::operator < (const BaseHistogram& h) const
  {
    return CompareAlmost(h) == -1;
  }

  Word Histogram1d::In(const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct)
  {
    NList in(instance);
 
    if (in.isSymbol("undef") || 
       (in.length() == 1 && in.first().isSymbol("undef")))
    {
      correct = true;
      return SetWord(new Histogram1d(false));
    }
        
    Word w = SetWord(Address(0));
    Histogram1d* newHist = new Histogram1d(true);
    newHist->Clear();
    const ListExpr *concise = &instance;
    ListExpr rangeList;
    ListExpr binList;
    ListExpr restItems;
    HIST_REAL minimum = -1000 * FLT_MAX;
    HIST_REAL currentValue = 0.0;
    HIST_REAL lastValue = minimum;
  

      /*  We expect the incoming data to be formatted as follows:
       * ((range*)(bin*)) 
       *  with range a list of (n+1) class limits in ascending order
       *  and  bin a list of (n) values.
       *  All values must be of type real.
       */
      if (    (nl->IsEmpty(*concise))      // incoming list is empty 
          ||  (nl->ListLength(*concise) != 2)  // or is too short or too long
          ||  (nl->IsAtom(nl->First(*concise))) //or does not contain two lists 
          ||  (nl->IsAtom(nl->Second(*concise))) 
          ||  (nl->ListLength(nl->First(*concise)) //or number of ranges is not 
                != (nl->ListLength(nl->Second(*concise))+1)))//number of bins +1
      {
        correct = false;
        return SetWord( Address(0) );
      }
        
      rangeList = nl->First(*concise);
      binList = nl->Second(*concise);
      
      // Examine the range list
      restItems = rangeList;
          
      while ( !nl->IsEmpty(restItems) )
      {
        const ListExpr curr = nl->First(restItems);
        if(nl->IsAtom(curr) &&
           (nl->AtomType(curr) == RealType))
        {
          currentValue = nl->RealValue(curr);
          restItems = nl->Rest(restItems);
          if (currentValue > lastValue)
          {
            newHist->AppendRange(currentValue);
            lastValue = currentValue;
          }  
          else // if (currentValue > lastValue)
          {
            correct = false;
            return SetWord( Address(0) );
          } // else // if (currentItem > lastItem)
        } // if(nl->IsAtom(nl->First(rangeList) && ...
        else // if(nl->IsAtom(nl->First(rangeList) && ...
        {
          correct = false;
          return SetWord( Address(0) );
        } // if(nl->IsAtom(nl->First(rangeList) &&      

      } // while ( !nl->IsEmpty(rest) )


      // Now examine the bin list
      restItems = binList;
      while ( !nl->IsEmpty(restItems) )
      {
        const ListExpr curr = nl->First(restItems);
        if(nl->IsAtom(curr) &&
           (nl->AtomType(curr) == RealType))
        {
          currentValue = nl->RealValue(curr);
          restItems = nl->Rest(restItems);

          newHist->AppendBin(currentValue);
        } // if(nl->IsAtom(nl->First(rangeList) && ...
        else // if(nl->IsAtom(nl->First(rangeList) && ...
        {
          correct = false;
          return SetWord( Address(0) );
        } // if(nl->IsAtom(nl->First(binList) && ...      
      } // while ( !nl->IsEmpty(restItems) )
      
      correct = true;
      return SetWord(newHist);
  } // static Word In (  const ListExpr typeInfo,

  
  
  
  ListExpr Histogram1d::Out(ListExpr typeInfo, Word value)
  {
    Histogram1d* hist = static_cast<Histogram1d*>(value.addr);
    const int numberRanges = hist->GetNoRange();
    const int numberBins   = hist->GetNoBin();
    
    if (!hist->IsDefined())
    {
      NList result = NList("undef");      
      return result.listExpr();
    }
    else if(hist->IsEmpty()) 
    {
      return (nl->TheEmptyList());
    }
    else // if( newHist->IsEmpty() ) { 
    {
      ListExpr rangeExpr = nl->Empty();
      ListExpr binExpr = nl->Empty();

      int i = 0;
      ListExpr last = rangeExpr;
      for (i=0; i < numberRanges; i++)
      {
        const ListExpr newElem = nl->RealAtom(*(hist->GetRange(i)));

        if (nl->IsEmpty(rangeExpr))
        {
          rangeExpr = nl->OneElemList( newElem);
          last = rangeExpr;
        }
        else
          last = nl->Append( last, newElem);
      }

      last = binExpr;
      for (i=0; i < numberBins; i++)
      {
        const ListExpr newElem = nl->RealAtom(*(hist->GetBin(i)));
        
        if (nl->IsEmpty(binExpr))
        {
          binExpr = nl->OneElemList( newElem);
          last = binExpr;
        }
        else
          last = nl->Append( last, newElem);
      }//for
      
      return nl->TwoElemList( rangeExpr, binExpr);
    } // else { // if( newHist->IsEmpty() ) {  
      
    // will (hopefully) never be reached:
    return (nl->TheEmptyList());  
      
    
  } // static ListExpr Out(ListExpr typeInfo, Word value){
  
  //Add val to specified bin
  void Histogram1d::Insert(const int index, const HIST_REAL& weight) 
  {
    const HIST_REAL* oldVal;
    bin.Get(index, oldVal);
    const HIST_REAL newVal = weight + *oldVal;
    bin.Put(index, newVal);
  }
  
  /*
   * Increases the content of the corresponding bin with the given weight value.
   * 
   * Returns "true" if such a corresponding bin exists, "false" else.
   */
  bool Histogram1d::Insert(const HIST_REAL& value, const HIST_REAL& weight)
  {
    CcInt index = FindBin(value);
    
    if (index.IsDefined())
    {
      Insert(index.GetValue(), weight);
      return true;
    }
    else
      return false;
  }
  
  CcInt Histogram1d::FindBin(const HIST_REAL& val) const  
  {
    // Precondition: range is sorted in ascending order.
    
    if (!IsDefined() || 
        !IsConsistent(false) || 
        val < *GetRange(0) || 
        val >= *GetRange(GetNoRange() - 1))
    {
      return CcInt(false, 0);
    }
    
    // Binary search for the corresponding bin:
    int pos;
    range.Find( &val, BaseHistogram::CmpBinSearch, pos);
    
    return CcInt(true, pos - 1);
  }
  
  /*
   * Implementation copied from
   * gsl_histogram_stat.c of
   * the GNU Scientific Library.
   * (gsl-1.9)
   * 
   * Author: Simone Piccardi
   * Jan. 2000
   * 
   */
  CcReal Histogram1d::Mean() const
  {
    if (!IsDefined() || IsEmpty() || !IsConsistent(false))
    return CcReal(false, 0.0);

    const size_t n = this->GetNoBin();
    size_t i;

    /* Compute the bin-weighted arithmetic mean M of a histogram using the
     recurrence relation

     M(n) = M(n-1) + (x[n] - M(n-1)) (w(n)/(W(n-1) + w(n))) 
     W(n) = W(n-1) + w(n)

     We skip negative values in the bins,
     since those correspond to negative weights (BJG).
     
     */

    long double wmean = 0;
    long double W = 0;

    for (i = 0; i < n; i++)
    {
      HIST_REAL xi = (*GetRange(i + 1) + *GetRange(i)) / 2;
      HIST_REAL wi = *GetBin(i);

      if (wi > 0)
      {
        W += wi;
        wmean += (xi - wmean) * (wi / W);
      }
    }

    return CcReal(true, wmean);
  }

  /*
   * Implementation copied from
   * gsl_histogram_stat.c of
   * the GNU Scientific Library.
   * (gsl-1.9)
   * 
   * Author: Simone Piccardi
   * Jan. 2000
   * 
   */
  CcReal Histogram1d::Variance() const
  {
    if (!IsDefined() || IsEmpty() || !IsConsistent(false))
    return CcReal(false, 0.0);

    const size_t n = this->GetNoBin();
    size_t i;

    long double wvariance = 0;
    long double wmean = Mean().GetValue();
    long double W = 0;

    /* 
     Compute the variance.
     We skip negative values in the bins,
     since those correspond to negative weights (BJG).
     */

    for (i = 0; i < n; i++)
    {
      double xi = (*GetRange(i + 1) + *GetRange(i)) / 2;
      double wi = *GetBin(i);

      if (wi > 0)
      {
        const long double delta = (xi - wmean);
        W += wi;
        wvariance += (delta * delta - wvariance) * (wi / W);
      }
    }

    return CcReal(true, wvariance);
  }
    
  size_t Histogram1d::Sizeof() const
  {
    return sizeof(*this);
  }
  
  int Histogram1d::Compare(const Attribute *rhs) const
  {
    return Compare(*(Histogram1d*)rhs);
  }
  
  int Histogram1d::CompareAlmost(const Attribute *rhs) const
  {
    return CompareAlmost(*(Histogram1d*)rhs);
  }
  
  bool Histogram1d::Adjacent(const Attribute *attrib) const
  {
    return false;
  }
  
  Attribute* Histogram1d::Clone() const 
  {
    return new Histogram1d(*this);
  }
  
/*

1.1 HashValue

The algorithm is taken from
 
        John Hamer. Hashing revisited. In ITiCSE2002 Seventh Conference on 
        Innovation and Technology in Computer Science Education,
        pages 80-83, Aarhus, Denmark, June 2002
        
*/
  
  size_t Histogram1d::HashValue() const
  {
    if (!IsDefined())
      return 0;
    
    size_t h = 0;
    
    for (int i = 0; i < GetNoRange(); i++)
    {
      h = hgr::Rotate(h) ^ hgr::HashValue(GetRange(i));
    }
    
    for (int i = 0; i < GetNoBin(); i++)
    {
      h = hgr::Rotate(h) ^ hgr::HashValue(GetBin(i));
    }
    
    return h;
  }
  
  void Histogram1d::CopyFrom(const StandardAttribute* right) 
  {
    const Histogram1d* hist = static_cast<const Histogram1d*>(right);
    
    SetDefined(hist->IsDefined());
    CopyRangesFrom(hist);
    CopyBinsFrom(hist);
  }  
  
  int Histogram1d::NumOfFLOBs() const
  {
    return 2;
  }
  
  FLOB* Histogram1d::GetFLOB( const int i )
  {
    assert( i >= 0 && i < NumOfFLOBs() );
    
    if (i == 0)
      return &range;
    else
      return &bin;
  }
  
  void* Histogram1d::Cast(void* addr)
  {
    return (new (addr) Histogram1d);
  }
    
  
  Word Histogram1d::Create( const ListExpr typeInfo )
  {
    return SetWord(new Histogram1d(true));
  }
  
  // the KindCheck or CheckKind function
  bool Histogram1d::KindCheck(ListExpr type, ListExpr& errorInfo)
  {
    return (nl->IsEqual(type, HISTOGRAM1D));
  }
  
  ostream& Histogram1d::Print( ostream& os ) const
  {
    return (os << *this);
  }
  
/*

1.1 Coarsen   
   
*/
  void Histogram1d::Coarsen(const BaseHistogram* b)
  {
    const Histogram1d* pattern = static_cast<const Histogram1d*>(b);
// Precondition: this->IsRefinement(pattern) == true
    DBArray<HIST_REAL>* newBin = new DBArray<HIST_REAL>(GetNoBin());
    HIST_REAL binVal;
    int myRange = 1;
    
    for (int i = 1; i < pattern->GetNoRange(); i++) {
         binVal = 0;
      
      // conflate bins (zusammenfassen)
      while(myRange < GetNoRange() 
          && CmpReal(GetRange(myRange), pattern->GetRange(i)) != 1)
      {
        binVal += *GetBin(myRange - 1);
        myRange++;
      }
      newBin->Append(binVal);
    } // while(CmpReal(GetRange(myRange), pattern->GetRange(i)) != 1)
    
    // copy new bins and ranges
    const HIST_REAL* p;
    
    range.Clear();
    bin.Clear();
    for (int i = 0; i < pattern->GetNoRange(); ++i) {
      range.Append(*(pattern->GetRange(i)));
      if (i < newBin->Size())
      {
        newBin->Get(i, p);
        bin.Append(*p);
      }        
    }    
    delete newBin;
  }

/*  
  
1.1 CopyRangesFrom  
  
*/  
  void Histogram1d::CopyRangesFrom(const BaseHistogram* h)
  {
    Histogram1d* src = (Histogram1d*)h;
    
    range.Clear();
    
    for (int i = 0; i < src->GetNoRange(); i++)
      this->AppendRange(*src->GetRange(i));
  }
  
  ostream& operator << (ostream& os, const Histogram1d& h)
  {
    if (!h.IsDefined())
    {
      os << "(" << HISTOGRAM1D << " undef)";
      return os;
    }
    
    os << "(" << HISTOGRAM1D << "( ( ";

    for (int i = 0; i < h.GetNoRange(); i++)
      os << *h.GetRange(i) << " ";

    os << ") ( ";

    for (int i = 0; i < h.GetNoBin(); i++)
      os << *h.GetBin(i) << " ";

    os << ") ))";

    return os;
  }

/*  
  
1 histogram1dInfo  
  
*/  
  histogram1dInfo::histogram1dInfo()
  {
    name        = HISTOGRAM1D;
    signature   = "-> DATA";
    typeExample = HISTOGRAM1D;
    listRep     = "((ranges*)(bins*))";
    valueExample= "( (0.0 1.0 2.0 3.0) (2.0 4.8 5.2) )";
    remarks     = "All coordinates must be of type real.";
  } // histogram1dInfo() {
  
/* 

1 Creation of the Type Constructor Instance

*/
  histogram1dFunctions::histogram1dFunctions()
  {
    //reassign some function pointers
    in = Histogram1d::In;
    out = Histogram1d::Out;
    cast = Histogram1d::Cast;
    kindCheck = Histogram1d::KindCheck;
    create = Histogram1d::Create;
  } // histogram1dFunctions() {

    
/*

5 Operators

5.1 set[_]histogram1d

5.1.1 Type mapping function
  
*/  
  ListExpr SetHistogram1dTypeMap(ListExpr args)
  {
    NList list(args);
    string errMsg = "Operator set_histogram1d "
      "expects as argument "
      "a list with structure: ((stream real)).";

    if (list.length() != 1)
      return list.typeError(errMsg);
    
    list = list.first();
    
    if (list.length() != 2)
      return list.typeError(errMsg);

    NList part1 = list.first();
    NList part2 = list.second();

    // (stream real) -> histogram1d
    if (part1.isSymbol(STREAM) && part2.isSymbol(REAL) )
      return NList(HISTOGRAM1D).listExpr();

    return list.typeError(errMsg);
  }

/*

5.1.2 Value Mapping Function

*/
  int SetHistogram1dFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    Word elem;
    CcReal* streamObj;
    HIST_REAL value, lastValue;
    bool firstElem = true;

    // The query processor provided an empty Histogram1d-instance:
    result = qp->ResultStorage(s);
    Histogram1d* resultHg = (Histogram1d*) result.addr;
    resultHg->Clear();

    qp->Open(args[0].addr);

    // Request the first element of the stream:
    qp->Request(args[0].addr, elem);

    while (qp->Received(args[0].addr))
    {
      // There is a next element:
      streamObj = (CcReal*) elem.addr;
      value = streamObj->GetRealval();

      if (!streamObj->IsDefined())
      {
        cout << "Error in operator set_histogram1d: "
            << "The stream contains an undefined value." << endl;
        resultHg->Clear();
        resultHg->SetDefined(false);
        qp->Close(args[0].addr);
        return 0;
      }

      if (!firstElem && (CmpReal(value, lastValue) < 1))
      {
        cout << "Error in operator set_histogram1d: "
            << "The stream is not sorted or contains duplicates." << endl;
        resultHg->Clear();
        resultHg->SetDefined(false);
        qp->Close(args[0].addr);
        return 0;
      }

      resultHg->AppendRange(value);
      //cout << "resultHg->AppendRange(" << value << ");" << endl;

      lastValue = value;
      firstElem = false;

      // Consume the stream object:
      streamObj->DeleteIfAllowed();

      // Request the next element of the stream:
      qp->Request(args[0].addr, elem);

    } // while (qp->Received(args[0].addr))

    if (resultHg->GetNoRange() < 2)
    {
      cout << "Error in operator set_histogram1d: "
          << "The stream contains less than two elements." << endl;
      resultHg->Clear();
      resultHg->SetDefined(false);
      qp->Close(args[0].addr);
      return 0;
    }

    int noOfBins = resultHg->GetNoRange() - 1;
    
    resultHg->ResizeBin(noOfBins);

    for (int i = 0; i < noOfBins; i++)
      resultHg->AppendBin(0.0);

    qp->Close(args[0].addr);
    return 0;
  }
  
/*

5.2 no[_]components
    
5.2.1 Type mapping
    
*/  
  ListExpr NoComponentsTypeMap(ListExpr args)
  {
    NList list(args);
    const string errMsg = "Expecting (" + HISTOGRAM1D + ")";

    if (list.length() != 1)
    return list.typeError(errMsg);

    NList arg1 = list.first();

    // histogram1d -> int
    if (arg1.isSymbol(HISTOGRAM1D))
    return NList(INT).listExpr();

    return list.typeError(errMsg);
  }

/*  

5.2.2 Value mapping

*/  
  int NoComponentsFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    Histogram1d* h1 = static_cast<Histogram1d*>(args[0].addr);
    result = qp->ResultStorage(s);
    CcInt* i = static_cast<CcInt*>(result.addr);
    
    if (!h1->IsDefined())
    {
      i->Set(false, -1);
      return 0;
    }

    i->Set(true, h1->GetNoBin());
    return 0;
  }

/*

5.3 binrange[_]min and binrange[_]max

5.3.1 Type mapping
      
*/  
  ListExpr binrange_min_maxTypeMap( ListExpr args )
  {
    NList argList = NList(args);
    short int ok = 0;

    // we are expecting exactly two arguments
    if (argList.length() == 2)
    {
      NList hist = argList.first();
      NList numberBin = argList.second();

      // the first argument is to be a histogram 1d
      if ((hist.isSymbol(HISTOGRAM1D)))
      {
        ok++;
      } // if ( (hist.isSymbol(HISTOGRAM1D) ) )

      else
      {
        argList.typeError("Expecting a histogram1d as first input parameter.");
      } // else // if ( (hist.isSymbol(HISTOGRAM1D) ) )

      // and the second argument is to be an integer value
      if (numberBin.isSymbol(INT))
      {
        ok++;
      } // if (numberBin.isSymbol(INTEGER))

      else
      {
        argList.typeError("Expecting an integer value as "
            "second input parameter.");
      } // else // if (numberBin.isSymbol(INTEGER))

    } // if (argList.length() == 2)

    else
    {
      // return list.typeError(errMsg);
      argList.typeError("Expecting exactly two input parameters: "
          "(1) a histogram1d, (2) an integer value "
          "indicating the bin number.");
    }
    if (ok == 2)
      return NList(REAL).listExpr();
    else
      return argList.typeError( "Expecting exactly two input parameters: "
        "(1) a histogram1d, (2) an integer value "
        "indicating the bin number.");
  } // ListExpr binrange_min_maxTypeMap( ListExpr args )  

/*  
  
5.3.2 Value mapping

*/  
  int binrange_minFun( Word* args, Word& result,
      int message, Word& local, Supplier s )
  {
    Histogram1d *hg = static_cast<Histogram1d*>(args[0].addr);
    CcInt* binNumber = static_cast<CcInt*>(args[1].addr);
    
    result = qp->ResultStorage(s);
    CcReal *r = (CcReal*)result.addr;

    if (!hg->IsDefined() || !binNumber->IsDefined())
    {
      r->Set(false, 0.0);
      return 0;
    }

    int i = binNumber->GetIntval( );
    // binNumber i must be in the histogram's size
    if ((i >= 0) && (i < (hg->GetNoRange()) -1 ))
    {
      r->Set(*(hg->GetRange(i)));
      return 0;
    } // if ((i >= 0) && (i < hg->GetNoRange()))  

    else
    {
      cerr << "Please indicate as second parameter "
      "an integer value between 0 and " << hg->GetNoRange() -2 << "." << endl;
      return 0;
    } // else // if ((i >= 0) && (i < hg->GetNoRange()))

  } // ListExpr binrange_minFun( Word* args, Word& result, ...


  int binrange_maxFun( Word* args, Word& result,
      int message, Word& local, Supplier s )
  {
    Histogram1d *hg = static_cast<Histogram1d*>(args[0].addr);
    CcInt *binNumber = static_cast<CcInt*>(args[1].addr );
    
    result = qp->ResultStorage(s);
    CcReal *r = (CcReal*)result.addr;

    if (!hg->IsDefined() || !binNumber->IsDefined())
    {
      r->Set(false, 0.0);
      return 0;
    }

    int i = binNumber->GetIntval( );
    // binNumber i must be in the histogram's size
    if ((i >= 0) && (i < (hg->GetNoRange() -1 )))
    {
      r->Set(*(hg->GetRange(i+1)));
      return 0;
    } // if ((i >= 0) && (i < hg->GetNoRange()))  

    else
    {
      cerr << "Please indicate as second parameter "
      "an integer value between 0 and " << hg->GetNoRange() -2 << "." << endl;
      return 0;
    } // else // if ((i >= 0) && (i <X hg->GetNoRange()))

  } // ListExpr binrange_maxFun( Word* args, Word& result, ...

/*
  
5.4 create[_]histogram1d

5.4.1 Type mapping

*/
  ListExpr CreateHistogram1dTypeMap(ListExpr args)
  {
    NList argList = NList(args);
    
    TEST("argList: ", argList)
    
    CHECK_COND(argList.length() == 3, "Expecting three arguments.");

    NList stream = argList.first();
    NList attrName = argList.second();
    NList hist = argList.third(); 
    
    TEST("stream: ", stream)    
    TEST("attrName: ", attrName)
    TEST("hist: ", hist)

    CHECK_COND(stream.length() == 2
        && stream.first().isSymbol(STREAM)
        && stream.second().length() == 2
        && stream.second().first().isSymbol(TUPLE)
        && IsTupleDescription(stream.second().second().listExpr()),
        "Expecting a stream of tuples.");

    // Check if tuple has supplied attribute
    NList tupleDescr = stream.second().second();    
    int len = tupleDescr.length();
    bool found = false;
    NList attrType;
    
    TEST("tupleDesc: ", tupleDescr)
    
    int index = 1;
    for (; index <= len; ++index)
    {
      NList attr = tupleDescr.first();
      tupleDescr.rest();
      
      //cout << "attr" << endl << attr << endl;
      TEST("attr: ", attr)
      
      if (attrName == attr.first())
      {
        found = true;
        attrType = attr.second();
        break;
      }
    } // for (; index <= len; ++index)
    
    TEST("attrType: ", attrType)
    
    CHECK_COND(found, "Attribute not found");

    CHECK_COND(attrType.isSymbol(REAL), "Attribute not of type real.");

    CHECK_COND(hist.isSymbol(HISTOGRAM1D), "Histogram1d has wrong type");

    NList result = NList();
    result.append(NList("APPEND", false));
    NList idx = NList();
    idx.append(NList(index));
    result.append(idx);
    result.append(HISTOGRAM1D);

    TEST("Result of type mapping of CreateHistogram1d: ", result)
    
    return result.listExpr();
  } // CreateHistogram1dTypeMap(ListExpr args)    
  
/*  
  
5.4.2 Value mapping 

The function makes use of four arguments:

  0. stream of tuples

  1. attribute name   

  2. histogram1d

  3. attribute index
  
*/

  // arg 0 tuple stream, 1 attribute name, 2 histogram1d, 3 index of attribute
  int CreateHistogram1dFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    Word elem;
    CcInt bin;
    HIST_REAL val;    
    CcReal* attrVal;
    
    Histogram1d* hist = (Histogram1d*)args[2].addr;
    CcInt* index = (CcInt*)args[3].addr;
    
    Tuple* currentTuple;

    qp->Open(args[0].addr);
    qp->Request(args[0].addr, elem);

    while (qp->Received(args[0].addr) )
    {
      // Find bin for current value of tuple attribute
      currentTuple = (Tuple*)elem.addr;
      attrVal = (CcReal*)(currentTuple->GetAttribute(index->GetIntval()-1));  
      val = attrVal->GetRealval();
      
      
      // catch empty (== undefined) histograms
      if (hist->IsDefined())
      {
        bin = hist->FindBin(val);
        
        // increment bin, if it was found
        if (bin.IsDefined())
          hist->Insert(bin.GetIntval());
        
        currentTuple->DeleteIfAllowed();// consume the stream objects
        qp->Request(args[0].addr, elem);
      }
    }
    
    // return filled histogram1d
    result = qp->ResultStorage(s);
    Histogram1d* res = (Histogram1d*)result.addr;
    res->Clear();
    res->CopyFrom(hist);

    qp->Close(args[0].addr);

    return 0;
  } // int CreateHistogram1dFun

/*  
  
5.5 create[_]histogram1d[_]equiwidth

5.5.1 Type mapping
  
*/  
  ListExpr CreateHistogram1dEquiwidthTypeMap(ListExpr args)
  {
    string errorMsg;
    NList list(args);

    // Check the list:
    errorMsg = "Operator create_histogram1d_equiwidth "
      "expects a list of length three.";

    if (list.length() != 3)
      return list.typeError(errorMsg);

    NList arg1 = list.first(); // (stream (tuple ((a1 x1) ... (an xn))))
    NList arg2 = list.second(); // (ai)
    NList arg3 = list.third(); // (n)
    

    // Check the first argument:
    // (stream (tuple ((a1 x1) ... (an xn))))
    errorMsg
        = "Operator create_histogram1d_equiwidth expects as first argument "
          "a list with structure\n"
          "(stream (tuple ((a1 t1)...(an tn))))";

    if (!IsStreamDescription(arg1.listExpr()))
      return list.typeError(errorMsg);
    

    // Check the second argument (ai):
    errorMsg = "Operator create_histogram1d_equiwidth expects as "
      "second argument an attribute.";

    if (!arg2.isSymbol())
      return list.typeError(errorMsg);

    // Find the attribute in the tuple:
    string attrname = arg2.str();
    ListExpr attrlist = arg1.second().second().listExpr();
    ListExpr attrtype;

    int attrindex = FindAttribute(attrlist, attrname, attrtype);
    //cout << "attrtype = " << NList(attrtype).convertToString() << endl;
    
    errorMsg = "Attribute name '" + attrname + "' is not known.\n"
          "Known Attribute(s): " + NList(attrlist).convertToString();
    
    if (attrindex == 0)
      return list.typeError(errorMsg);
    
    errorMsg = "Operator create_histogram1d_equiwidth expects as "
          "second argument an attribute of type real.";
    
    if (!NList(attrtype).isSymbol(REAL))
      return list.typeError(errorMsg);
    
    
    
    // Check the third argument (int):
    errorMsg = "Operator create_histogram1d_equiwidth expects as "
      "third argument the symbol 'int'.";

    if (!arg3.isSymbol(INT))
      return list.typeError(errorMsg);
    

    // Everything should be fine.
    // We can build the outlist:
    NList outlist(NList("APPEND"), NList(attrindex).enclose(),
        NList(HISTOGRAM1D) );

    //cout << "outlist: " << outlist.convertToString() << endl;

    return outlist.listExpr();
  }
  

    
/*

5.5.3 Value mapping

*/
  int CreateHistogram1dEquiwidthFun(Word* args, Word& result, int message,
      Word& local, Supplier s)
  {
    Word elem;
    const size_t MAX_MEMORY = qp->MemoryAvailableForOperator();
    TupleBuffer* buffer = new TupleBuffer(MAX_MEMORY);

    // The query processor provided an empty Histogram1d-instance:
    result = qp->ResultStorage(s);
    Histogram1d* resultHg = (Histogram1d*) result.addr;
    resultHg->Clear();

    // We don't use the attrname, delivered in args[1].addr.
    // Get the index of the attribute instead:
    const int attrIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

    if ( !((CcInt*)args[2].addr)->IsDefined() )
    {
      resultHg->SetDefined(false);
      return 0;
    }

    // Get the max. number of bins:
    const int maxBins = ((CcInt*)args[2].addr)->GetIntval();
    //cout << "maxBins = " << maxBins << endl;

    if (maxBins < 1)
    {
      cout << "Error in operator create_histogram1d_equiwidth: "
      << "The last argument must be greater than zero." << endl;
      resultHg->SetDefined(false);
      return 0;
    }

    // Open the tuple-stream:
    qp->Open(args[0].addr);

    // Request the first tuple of the stream:
    qp->Request(args[0].addr, elem);

    Tuple* tuplePtr;
    CcReal* attrPtr;
    HIST_REAL attrValue;
    HIST_REAL min;
    HIST_REAL max;
    bool firstTuple = true;

    while (qp->Received(args[0].addr))
    {
      // Get a pointer to the current tuple:
      tuplePtr = (Tuple*) elem.addr;

      // Get a pointer to the current attribute:
      attrPtr = (CcReal*)(tuplePtr->GetAttribute(attrIndex));

      // Get the value of the current attribute:
      attrValue = attrPtr->GetRealval();

      // We ignore undefined attributes:
      if (attrPtr->IsDefined())
      {
        if (firstTuple)
        {
          max = min = attrValue;
          firstTuple = false;
        }

        // Is it a new min./max. value?
        if (attrValue < min)
        min = attrValue;
        else if (attrValue > max)
        max = attrValue;

        // Push the current tuple in the buffer:
        buffer->AppendTuple(tuplePtr);
      }

      // Consume the tuple:
      //tuplePtr->DecReference();
      tuplePtr->DeleteIfAllowed();

      // Request the next tuple of the stream:
      qp->Request(args[0].addr, elem);

    } // while (qp->Received(args[0].addr))

    qp->Close(args[0].addr);

    if (buffer->IsEmpty())
    {
      cout << "Error in operator create_histogram1d_equiwidth: "
      << "The stream contains no valid elements." << endl;
      resultHg->SetDefined(false);
      return 0;
    }

    if (AlmostEqual(min, max))
    {
      // We got just one or equal values:
      // Create a small bin...
      resultHg->AppendRange(min - FACTOR);
      resultHg->AppendRange(max + FACTOR);

      // and fill in all values:
      resultHg->AppendBin(buffer->GetNoTuples());

      return 0;
    }

    const HIST_REAL valueRange = fabs(max - min);
    const HIST_REAL binWidth = valueRange / maxBins;
    const HIST_REAL firstBound = min;

    // Create the range-array.
    // Its size is: maxBins + 1

    resultHg->ResizeRange(maxBins + 1);

    for (int i = 0; i < maxBins; i++)
    {
      resultHg->AppendRange(firstBound + binWidth * i);
    }

    // Append the last range:
    resultHg->AppendRange(max + 10 * FACTOR);

    // Init the bin-array with empty bins.
    // Its size is: maxBins

    resultHg->ResizeBin(maxBins);

    for (int i = 0; i < maxBins; i++)
    {
      resultHg->AppendBin(0.0);
    }

    GenericRelationIterator* it = buffer->MakeScan();

    while ((tuplePtr = it->GetNextTuple()) != 0)
    {
      attrPtr = (CcReal*)(tuplePtr->GetAttribute(attrIndex));
      resultHg->Insert(attrPtr->GetRealval());
      
      //tuplePtr->DecReference();
      tuplePtr->DeleteIfAllowed();
    }
    
    delete it;
    buffer->Clear();
    delete buffer;

    return 0;
  }  
  
  /*
   Argument 0 Histogramm, 1 lower bound, 2 upper bound
   */
  template<bool eager>
  int Shrink1dFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    Histogram1d* hist = (Histogram1d*)args[0].addr;
    CcReal* lo = (CcReal*)args[1].addr;
    CcReal* hi = (CcReal*)args[2].addr;

    result = qp->ResultStorage(s);
    Histogram1d* res = (Histogram1d*)result.addr;

    if (!hist->IsDefined() ||
        hist->IsEmpty() ||
        !hist->IsConsistent(false) ||
        !lo->IsDefined() ||
        !hi->IsDefined())
    {
      res->SetDefined(false);
      return 0;
    }

    const HIST_REAL low = lo->GetRealval();
    const HIST_REAL high = hi->GetRealval();

    if (CmpReal(high, low) != 1)
    {
      cout << "Error in operator shrink_x: "
      << "The upper bound must be greater than the lower bound." << endl;
      res->SetDefined(false);
      return 0;
    }

    if ((low < *hist->GetRange(0) && high < *hist->GetRange(0)) || (low
            > *hist->GetRange(hist->GetNoRange()-1) && high
            > *hist->GetRange(hist->GetNoRange()-1)))
    {
      cout << "Error in operator shrink_x: "
      << "The specified interval is outside the histogram-range."
      << endl;
      res->SetDefined(false);
      return 0;
    }

    CcInt lower = hist->FindBin(low);
    CcInt higher = hist->FindBin(high);
    int lowBin = 0;
    int highBin = hist->GetNoBin() - 1;

    if (lower.IsDefined())
    lowBin = lower.GetIntval();

    if (higher.IsDefined())
    highBin = higher.GetIntval();

    if (eager)
    {
      if (lower.IsDefined() && !AlmostEqual(low, *hist->GetRange(lowBin)))
      {
        // Drop the first bin,
        // if the lower bound is inside the histogram-range and
        // we do not hit the bound of the bin.
        lowBin++;
      }

      if (higher.IsDefined())
      {
        // Drop the last bin, 
        // if the upper bound is inside the histogram-range.
        highBin--;
      }
    }

    res->ResizeRange(highBin - lowBin + 2);
    res->ResizeBin(highBin - lowBin + 1);

    for (int i = lowBin; i <= highBin; ++i)
    {
      res->AppendBin(*hist->GetBin(i));
      res->AppendRange(*hist->GetRange(i));
    }
    res->AppendRange(*hist->GetRange(highBin+1));

    if (res->IsEmpty() || !res->IsConsistent(false))
    {
      res->Clear();
      res->SetDefined(false);
    }

    return 0;
  }
  
  /*
   Instantiation of Template Functions

   The compiler cannot expand these template functions in
   the file ~HistogramAlgebra.cpp~.

   */

  template int
  Shrink1dFun<false>(Word* args, Word& result, int message,
      Word& local, Supplier s);

  template int
  Shrink1dFun<true>(Word* args, Word& result, int message,
      Word& local, Supplier s);
  
  ListExpr Shrink1dTypeMap(ListExpr args)
  {
    NList argList(args);

    CHECK_COND(argList.length() == 3,
        "Expecting list of length 3");

    NList hist = argList.first();
    NList lo = argList.second();
    NList hi = argList.third();

    CHECK_COND(hist.isSymbol(HISTOGRAM1D),
        "Expecting " + HISTOGRAM1D + " as first argument");

    CHECK_COND(lo.isSymbol(REAL) && hi.isSymbol(REAL),
        "Expecting real values for lower an upper bound");

    return hist.listExpr();
  }
  

  int Getcount1dFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    Histogram1d* h = static_cast<Histogram1d*>(args[0].addr);
    CcInt* binIndex = static_cast<CcInt*>(args[1].addr);
    result = qp->ResultStorage(s);
    CcReal* res = static_cast<CcReal*>(result.addr );
    
    if (!binIndex->IsDefined())
    {
      res->SetDefined(false);
      return 0;
    }
    
    CcReal count = h->GetCount(binIndex->GetValue());
    res->Set(count.IsDefined(), count.GetValue());
    return 0;
  }

  ListExpr Getcount1dTypeMap(ListExpr args)
  {
    NList list(args);
    const string errMsg = "Expecting (" + HISTOGRAM1D + " " + INT + ")";

    if (list.length() != 2)
      return list.typeError(errMsg);

    NList arg1 = list.first();
    NList arg2 = list.second();

    // histogram1d x int -> real
    if (arg1.isSymbol(HISTOGRAM1D) && arg2.isSymbol(INT))
      return NList(REAL).listExpr();

    return list.typeError(errMsg);
  }  
  
  /*
   Argument 0 Histogram1d, 1 Real value
   */
  template<bool incValSupplied>
  int Insert1dFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    Histogram1d* h = (Histogram1d*)args[0].addr;
    CcReal* x = (CcReal*)args[1].addr;
    
    // return histogram1d
    result = qp->ResultStorage(s);
    Histogram1d* hist = (Histogram1d*)result.addr;
    hist->Clear();
    
    if (!h->IsDefined() || !x->IsDefined())
    {
      hist->SetDefined(false);
      return 0;
    }
    
    hist->CopyFrom(h);

    CcInt bin = hist->FindBin(x->GetRealval());

    if (bin.IsDefined())
    {
      if (incValSupplied)
      {
        CcReal* incVal = (CcReal*)args[2].addr;
        
        if (!incVal->IsDefined())
        {
          hist->SetDefined(false);
          return 0;
        }
        
        hist->Insert(bin.GetIntval(), incVal->GetRealval());
      }
      else
        hist->Insert(bin.GetIntval());
    }

    return 0;
  }

  template<bool incValSupplied>
  ListExpr Insert1dTypeMap(ListExpr args)
  {
    NList argList = NList(args);

    if (incValSupplied)
    {
      CHECK_COND(argList.length() == 3, "Expecting List of length 3");
    }
    else
    {
      CHECK_COND(argList.length() == 2, "Expecting List of length 2");
    }

    NList hist = argList.first();
    NList x = argList.second();

    CHECK_COND(hist.isSymbol(HISTOGRAM1D), "First argument not of type " +
        HISTOGRAM1D);

    CHECK_COND(x.isSymbol(REAL), "Second argument not of type " +
        REAL);

    if (incValSupplied)
    {
      NList incVal = argList.third();
      CHECK_COND(incVal.isSymbol(REAL), "Third argument not of type " + REAL);
    }

    NList result = NList(HISTOGRAM1D);
    return result.listExpr();
  }
  
  /*
   Argument 0 Histogram1d, 1 real value
   */
  int FindBinFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    Histogram1d* hist = (Histogram1d*)args[0].addr;
    CcReal* value = (CcReal*)args[1].addr;
    
    CcInt bin;
    
    result = qp->ResultStorage(s);
    
    if (value->IsDefined()) 
    {
      bin = hist->FindBin(value->GetRealval());    
    }
    else 
    {
      bin = CcInt(false, 0);
    }
    
    ((CcInt*)result.addr)->Set(bin.IsDefined(), bin.GetIntval());
    return 0;
  }  
  
  template<bool isMin>
  int FindMinMaxBinFun1d(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    Histogram1d* h = static_cast<Histogram1d*>(args[0].addr);
    MinMaxBuffer* buffer;
    CcInt index;
    HIST_REAL value;
    int i;

    switch (message)
    {
    case OPEN:
      
      // Find the first bin with min./max. value,
      // and store it's index and value in local.addr:
      
      if (isMin)
        index = h->GetMinBin();
      else
        index = h->GetMaxBin();
      
      if (index.IsDefined())
        value = *h->GetBin(index.GetValue());
      else
        value = -1;
      
      local.addr = new MinMaxBuffer(index, value);
      return 0;

    case REQUEST:

      // Read the buffer:
      buffer = static_cast<MinMaxBuffer*>(local.addr);
      index = buffer->index;
      i = index.GetValue();
      value = buffer->value;
      
      if (!index.IsDefined())
      {
        // We are ready:
        result.addr = 0;
        return CANCEL;
      }
    
      // Create the next stream object;
      result.addr = new CcInt(index);
      
      // Find the next bin with min./max. value:
      while (++i < h->GetNoBin() && !AlmostEqual(*h->GetBin(i), value));
      
      if (i < h->GetNoBin())
      {
        // Store it for the next request:
        buffer->index = CcInt(true, i);
      }
      else
      {
        // Where is no next bin anymore:
        buffer->index = CcInt(false, -1);
      }
      
      return YIELD;

    case CLOSE:
      
      if (local.addr != 0)
      {
        delete (MinMaxBuffer*)local.addr;
        local = SetWord(Address(0));
      }
      
      return 0;
    }

    return 1; // This should never happen...
  }  
  
  int MeanFun(Word* args, Word& result, int message, Word& local, Supplier s)
  {
    Histogram1d* h = static_cast<Histogram1d*>(args[0].addr );

    result = qp->ResultStorage(s);

    CcReal* res = static_cast<CcReal*>(result.addr );
    CcReal mean = h->Mean();
    res->Set(mean.IsDefined(), mean.GetValue());

    return 0;
  }

  ListExpr Hist1dRealTypeMap(ListExpr args)
  {
    NList list(args);
    const string errMsg = "Expecting " + HISTOGRAM1D;

    if (list.length() != 1)
      return list.typeError(errMsg);

    NList arg1 = list.first();

    // histogram1d -> real
    if (arg1.isSymbol(HISTOGRAM1D))
      return NList(REAL).listExpr();

    return list.typeError(errMsg);
  }  
  
  static LexicographicalTupleCompare lexCmp;
  
  class TupleAndRelPos
  {
  public:

    TupleAndRelPos() :
    tuple(0),
    pos(0),
    cmpPtr(0)
    {};

    TupleAndRelPos(Tuple* newTuple, TupleCompareBy* cmpObjPtr = 0,
        int newPos = 0) :
    tuple(newTuple),
    pos(newPos),
    cmpPtr(cmpObjPtr)
    {};

    inline bool operator<(const TupleAndRelPos& ref) const
    {
      // by default < is used to define a sort order
      // the priority queue creates a maximum heap, hence
      // we change the result to create a minimum queue.
      // It would be nice to have also an < operator in the class
      // Tuple. Moreover lexicographical comparison should be done by means of
      // TupleCompareBy and an appropriate sort order specification, 

      if (!this->tuple || !ref.tuple)
      {
        return true;
      }
      if ( cmpPtr )
      {
        return !(*(TupleCompareBy*)cmpPtr)( this->tuple, ref.tuple );
      }
      else
      {
        return !lexCmp( this->tuple, ref.tuple );
      }
    }

    Tuple* tuple;
    int pos;

  private:
    void* cmpPtr;

  };

  class SortStream1d
  {
  public:
    SortStream1d( Word stream,
        const bool lexicographic,
        void *tupleCmp ):
    stream( stream ),
    currentIndex( 0 ),
    count( 0 ),
    lexiTupleCmp( lexicographic ?
        (LexicographicalTupleCompare*)tupleCmp :
        0 ),
    tupleCmpBy( lexicographic ? 0 : (TupleCompareBy*)tupleCmp ),
    lexicographic( lexicographic )
    {
      // Note: Is is not possible to define a Cmp object using the 
      // constructor 
      // mergeTuples( PairTupleCompareBy( tupleCmpBy )). 
      // It does only work if mergeTuples is a local variable which 
      // does not help us in this case. Is it a Compiler bug or C++ feature?
      // Hence a new class TupleAndRelPos was defined which implements 
      // the comparison operator '<'. 
      TupleQueue* currentRun = &queue[0];
      TupleQueue* nextRun = &queue[1];

      Word wTuple = SetWord(Address(0));

      size_t i = 0, a = 0, n = 0, m = 0, r = 0; // counter variables
      bool newRelation = true;

      MAX_MEMORY = qp->MemoryAvailableForOperator();
      cmsg.info("ERA:ShowMemInfo")
      << "Sortby.MAX_MEMORY (" << MAX_MEMORY/1024 << " kb)" << endl;
      cmsg.send();

      TupleBuffer *rel=0;
      TupleAndRelPos lastTuple(0, tupleCmpBy);

      qp->Request(stream.addr, wTuple);
      TupleAndRelPos minTuple(0, tupleCmpBy);
      while(qp->Received(stream.addr)) // consume the stream completely

      {

        count++; // tuple counter;
        Tuple* t = static_cast<Tuple*>( wTuple.addr );
        TupleAndRelPos nextTuple(t, tupleCmpBy);
        if( MAX_MEMORY > (size_t)t->GetSize() )
        {
          nextTuple.tuple->IncReference();
          currentRun->push(nextTuple);
          i++; // increment Tuples in memory counter
          MAX_MEMORY -= t->GetSize();
        }
        else
        { // memory is completely used 
          if ( newRelation )
          { // create new relation
            r++;
            rel = new TupleBuffer( 0 );
            GenericRelationIterator *iter = 0;
            relations.push_back( make_pair( rel, iter ) );
            newRelation = false;

            // get first tuple and store it in an relation
            nextTuple.tuple->IncReference();
            currentRun->push(nextTuple);
            minTuple = currentRun->top();
            minTuple.tuple->DecReference();
            rel->AppendTuple( minTuple.tuple );
            lastTuple = minTuple;
            currentRun->pop();
          }
          else
          { // check if nextTuple can be saved in current relation
            TupleAndRelPos copyOfLast = lastTuple;
            if ( nextTuple < lastTuple )
            { // nextTuple is in order              
              // Push the next tuple int the heap and append the minimum to 
              // the current relation and push
              nextTuple.tuple->IncReference();
              currentRun->push(nextTuple);
              minTuple = currentRun->top();
              minTuple.tuple->DecReference();
              rel->AppendTuple( minTuple.tuple );
              lastTuple = minTuple;
              currentRun->pop();
              m++;
            }
            else
            { // nextTuple is smaller, save it for the next relation
              nextTuple.tuple->IncReference();
              nextRun->push(nextTuple);
              n++;
              if ( !currentRun->empty() )
              {
                // Append the minimum to the current relation    
                minTuple = currentRun->top();
                minTuple.tuple->DecReference();
                rel->AppendTuple( minTuple.tuple );
                lastTuple = minTuple;
                currentRun->pop();
              }
              else
              { //create a new run 
                newRelation = true;

                // swap queues
                TupleQueue *helpRun = currentRun;
                currentRun = nextRun;
                nextRun = helpRun;
                ShowPartitionInfo(count,a,n,m,r,rel);
                i=n;
                a=0;
                n=0;
                m=0;
              } // end new run               
            } // end next tuple is smaller

            // delete last tuple if saved to relation and
            // not referenced by minTuple
            if ( copyOfLast.tuple && (copyOfLast.tuple != minTuple.tuple) )
            {
              copyOfLast.tuple->DeleteIfAllowed();
            }

          } // check if nextTuple can be saved in current relation
        }// memory is completely used

        qp->Request(stream.addr, wTuple);
      }
      ShowPartitionInfo(count,a,n,m,r,rel);

      // delete lastTuple and minTuple if allowed
      if ( lastTuple.tuple )
      {
        lastTuple.tuple->DeleteIfAllowed();
      }
      if ( (minTuple.tuple != lastTuple.tuple) )
      {
        minTuple.tuple->DeleteIfAllowed();
      }

      // the lastRun and NextRun runs in memory having 
      // less than MAX_TUPLE elements
      if( !queue[0].empty() )
      {
        Tuple* t = queue[0].top().tuple;
        queue[0].pop();
        mergeTuples.push( TupleAndRelPos(t, tupleCmpBy, -2) );
      }
      if( !queue[1].empty() )
      {
        Tuple* t = queue[1].top().tuple;
        queue[1].pop();
        mergeTuples.push( TupleAndRelPos(t, tupleCmpBy, -1) );
      }

      // Get next tuple from each relation and push it into the heap.
      for( size_t i = 0; i < relations.size(); i++ )
      {
        relations[i].second = relations[i].first->MakeScan();
        Tuple *t = relations[i].second->GetNextTuple();
        if( t != 0 )
        {
          t->IncReference();
          mergeTuples.push( TupleAndRelPos(t, tupleCmpBy, i+1) );
        }
      }
      Counter::getRef("Sortby:ExternPartitions") = relations.size();
    }

    /*
     It may happen, that the localinfo object will be destroyed
     before all internal buffered tuples are delivered stream
     upwards, e.g. queries which use a ~head~ operator.
     In this case we need to delete also all tuples stored in memory.

     */

    ~SortStream1d()
    {
      while( !mergeTuples.empty() )
      {
        mergeTuples.top().tuple->DecReference();
        mergeTuples.top().tuple->DeleteIfAllowed();
        mergeTuples.pop();
      }

      for( int i = 0; i < 2; i++ )
      {
        while( !queue[i].empty() )
        {
          queue[i].top().tuple->DecReference();
          queue[i].top().tuple->DeleteIfAllowed();
          queue[i].pop();
        }
      }

      // delete information about sorted runs
      for( size_t i = 0; i < relations.size(); i++ )
      {
        delete relations[i].second;
        relations[i].second = 0;
        delete relations[i].first;
        relations[i].first = 0;
      }

      delete lexiTupleCmp;
      lexiTupleCmp = 0;
      delete tupleCmpBy;
      tupleCmpBy = 0;
    }

    Tuple *NextResultTuple()
    {
      if( mergeTuples.empty() ) // stream finished
      return 0;
      else
      {
        // Take the first one.
        TupleAndRelPos p = mergeTuples.top();
        p.tuple->DecReference();
        mergeTuples.pop();
        Tuple *result = p.tuple;
        Tuple *t = 0;

        if (p.pos > 0)
        t = relations[p.pos-1].second->GetNextTuple();
        else
        {
          int idx = p.pos+2;
          if ( !queue[idx].empty() )
          {
            t = queue[idx].top().tuple;
            t->DecReference();
            queue[idx].pop();
          }
          else
          t = 0;
        }

        if( t != 0 )
        { // run not finished
          p.tuple = t;
          t->IncReference();
          mergeTuples.push( p );
        }
        return result;
      }
    }

    size_t TupleCount()
    {
      return count;
    }

  private:

    void ShowPartitionInfo( int c, int a, int n,
        int m, int r, GenericRelation* rel )
    {
      int rs = (rel != 0) ? rel->GetNoTuples() : 0;
      if ( RTFlag::isActive("ERA:Sort:PartitionInfo") )
      {
        cmsg.info() << "Current run finished: "
        << "  processed tuples=" << c
        << ", append minimum=" << m
        << ", append next=" << n << endl
        << "  materialized runs=" << r
        << ", last partition's tuples=" << rs << endl
        << "  Runs in memory: queue1= " << queue[0].size()
        << ", queue2= " << queue[1].size() << endl;
        cmsg.send();
      }
    }

    Word stream;
    size_t currentIndex;

    // tuple information
    LexicographicalTupleCompare *lexiTupleCmp;
    TupleCompareBy *tupleCmpBy;
    bool lexicographic;

    size_t count;

    // sorted runs created by in memory heap filtering 
    size_t MAX_MEMORY;
    typedef pair<TupleBuffer*, GenericRelationIterator*> SortedRun;
    vector< SortedRun > relations;

    typedef priority_queue<TupleAndRelPos> TupleQueue;
    TupleQueue queue[2];
    TupleQueue mergeTuples;
  };

  /*
   Argument 0 tuple stream, 
            1 attribute name, 
            2 maxCategories, 
            3 Index of attribute
   */
  int CreateHistogram1dEquicountFun(Word* args, Word& result, int message,
      Word& local, Supplier s)
  {

    Tuple* currentTuple;
    // The query processor provided an empty Histogram1d-instance:
    result = qp->ResultStorage(s);
    Histogram1d* hist = (Histogram1d*) result.addr;
    hist->Clear();
    CcInt* index = (CcInt*)args[3].addr;
    CcInt* maxCategories = (CcInt*)args[2].addr;

    qp->Open(args[0].addr);

    // number of bins must be a positive integer number, 
    // if not, our result is undefined
    if (maxCategories->IsDefined() && maxCategories->GetIntval() > 0)
    {

      // First sort values
      SortOrderSpecification spec;
      spec.push_back(pair<int, bool>(index->GetIntval(), true));
      void *tupleCmp = new TupleCompareBy(spec);

      SortStream1d* sli;
      sli = new SortStream1d( args[0],
          false,
          tupleCmp);

      int noTuples = sli->TupleCount();
      int noBins = noTuples / maxCategories->GetIntval();
      int rest = noTuples % maxCategories->GetIntval();

      // maxCategories > tupleCount
      if (noBins == 0)
      {
        noBins = 1;
        rest = 0;
      }

      if (noTuples == 0)
      hist->SetDefined(false);

      int count = 0;
      HIST_REAL value = -numeric_limits<HIST_REAL>::max();
      HIST_REAL lastvalue;
      int currentBin = -1;
      int tupleCount = 0;

      while ((currentTuple = sli->NextResultTuple()) != 0)
      {
        tupleCount++;
        CcReal* val = (CcReal*)currentTuple->GetAttribute(index->GetIntval()-1);

        if (!val->IsDefined())
        continue;

        lastvalue = value;
        value = val->GetRealval();

        currentTuple->DecReference();
        currentTuple->DeleteIfAllowed();

        // next bin reached?
        // If we have got a remainder r, 
        // the count of the first r bins is = noBins+1
        if ((currentBin >= rest && count % noBins == 0) || (currentBin < rest
                && count % (noBins + 1) == 0))
        {
          // if the current value is not yet greater than the precedent value
          if (CmpReal(value, lastvalue) != 1)
          {
            hist->Insert(currentBin);
            continue;
          }
          // if only a few values are left, we can merge the last bins
          else if ((count + noTuples - tupleCount) < noBins * 1.1)
          {
            hist->Insert(currentBin);
            continue;
          }
          else
          {
            hist->AppendRange(value);
            hist->AppendBin(0.0);
            currentBin++;
            count = 0;
          }
        } // (count % noBins == 0)      

        hist->Insert(currentBin);
        count++;
      }

      // Adding the last range.
      // Given that FACTOR does not cause "value < lastrange" to be true,
      // the less elegant way will do here:
      HIST_REAL lastrange = value+FACTOR;
      HIST_REAL fact = FACTOR;
      int max = 0;
      while (max < 10 && !(value<lastrange))
      {
        fact *= 10;
        lastrange = value+fact;
        max++;
      }
      hist->AppendRange(lastrange);

      // clean up the memory
      delete sli;
    } // (maxCategories->IsDefined() && maxCategories->GetIntval() > 0)

    else
    {
      Word elem;
      qp->Request(args[0].addr, elem);
      while(qp->Received(args[0].addr))
      {
        ((Tuple*)elem.addr)->DeleteIfAllowed();
        qp->Request(args[0].addr, elem);
      }
      hist->SetDefined(false);
    }

    qp->Close(args[0].addr);

    return 0;
  }

  ListExpr CreateHistogram1dEquicountTypeMap(ListExpr args)
  {
    NList argList = NList(args);

    //cout << "argList" << endl << argList << endl;

    CHECK_COND(argList.length() == 3, "Expecting three arguments");

    NList stream = argList.first();
    NList attrName = argList.second();
    NList maxCategories = argList.third();

    //cout << "stream" << endl << stream << endl;
    //cout << "attrName" << endl << attrName << endl;
    //cout << "maxCategories" << endl << maxCategories << endl;

    CHECK_COND(stream.length() == 2
        && stream.first().isSymbol(STREAM)
        && stream.second().length() == 2
        && stream.second().first().isSymbol(TUPLE)
        && IsTupleDescription(stream.second().second().listExpr()),
        "Expecting stream of tuples");

    // Check if tuple has supplied attribute
    NList tupleDescr = stream.second().second();
    int len = tupleDescr.length();
    bool found = false;
    NList attrType;

    //cout << "tupleDescr" << endl << tupleDescr << endl;

    int index = 1;
    for (; index <= len; ++index)
    {
      NList attr = tupleDescr.first();
      tupleDescr.rest();

      //cout << "attr" << endl << attr << endl;

      if (attrName == attr.first())
      {
        found = true;
        attrType = attr.second();
        break;
      }
    } // for (; index <= len; ++index)

    //cout << "attrType" << endl << attrType << endl;

    CHECK_COND(found, "Attribute not found");

    CHECK_COND(attrType.isSymbol(REAL), "Attribute not of type real");

    CHECK_COND(maxCategories.isSymbol(INT),
        "Number of categories is not an int");

    NList result = NList();
    result.append(NList("APPEND", false));
    NList idx = NList();
    idx.append(NList(index));
    result.append(idx);
    result.append(HISTOGRAM1D);

    //cout << "Result of CreateHistogram1dEquicount:\n\t" << result << endl;

    return result.listExpr();

  } // CreateHistogram1dEquicountTypeMap(ListExpr args)
  
  int VarianceFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    Histogram1d* h = static_cast<Histogram1d*>(args[0].addr );

    result = qp->ResultStorage(s);

    CcReal* res = static_cast<CcReal*>(result.addr );
    CcReal var = h->Variance();
    res->Set(var.IsDefined(), var.GetValue());

    return 0;
  } 
  
  /*
   Instantiation of Template Functions

   The compiler cannot expand these template functions in
   the file ~HistogramAlgebra.cpp~.

   */

  template int
  Insert1dFun<false>(Word* args, Word& result, int message,
      Word& local, Supplier s);

  template int
  Insert1dFun<true>(Word* args, Word& result, int message,
      Word& local, Supplier s);

  template ListExpr
  Insert1dTypeMap<true>(ListExpr args);

  template ListExpr
  Insert1dTypeMap<false>(ListExpr args);

  template int 
  FindMinMaxBinFun1d<true>(Word* args, Word& result, int message,
      Word& local, Supplier s);

  template int 
  FindMinMaxBinFun1d<false>(Word* args, Word& result, int message,
        Word& local, Supplier s);
  
} // namespace hgr
