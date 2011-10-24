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

[1] Header File of the Graph Algebra

December 2007, S.H[oe]cher,M.H[oe]ger,A.Belz,B.Poneleit


[TOC]

1 Defines and includes

*/
#include "Histogram2d.h"
#include "ListUtils.h"
#include <limits>
#include "../Algebras/ExtRelation-C++/Tupleorder.h"
#include "AlmostEqual.h"
#include "Symbols.h"

using namespace std;

namespace hgr
{

/*

2 Type constructor

*/

  histogram2dInfo h2_i;
  histogram2dFunctions h2_f;
  TypeConstructor histogram2dTC( h2_i, h2_f );

/*
3 Constructors and destructor

*/
  Histogram2d::Histogram2d()
  {
  }

  Histogram2d::Histogram2d(bool _defined, size_t sizeX, size_t sizeY) :
    BaseHistogram(_defined, sizeX * sizeY), rangeX(sizeX + 1), rangeY(sizeY + 1)
  {
  }

  Histogram2d::Histogram2d(const Histogram2d& rhs) :
    BaseHistogram(rhs), rangeX(rhs.rangeX.Size()), rangeY(rhs.rangeY.Size())
  {
    for (int i = 0; i < rhs.GetNoRangeX(); i++)
      rangeX.Append( rhs.GetRangeX(i) );

    for (int i = 0; i < rhs.GetNoRangeY(); i++)
      rangeY.Append( rhs.GetRangeY(i) );
  }

  Histogram2d::~Histogram2d()
  {
  }

  Histogram2d& Histogram2d::operator = (const Histogram2d& h)
  {
    CopyFrom(&h);
    return *this;
  }

/*

4 Helper functions

4.1 GetRangeX, GetRangeY

*/
  HIST_REAL Histogram2d::GetRangeX(int i) const
  {
    //cout << "rangeX" << endl;
    assert( 0 <= i && i < GetNoRangeX() );

    HIST_REAL p;
    rangeX.Get(i, &p);
    return p;
  }

  HIST_REAL Histogram2d::GetRangeY(int i) const
  {
    //cout << "rangeY" << endl;
    assert( 0 <= i && i < GetNoRangeY() );

    HIST_REAL p;
    rangeY.Get(i, &p);
    return p;
  }

/*
4.2 AppendRangeX, AppendRangeY

*/
  void Histogram2d::AppendRangeX(const HIST_REAL& r)
  {
    rangeX.Append(r);
  }

  void Histogram2d::AppendRangeY(const HIST_REAL& r)
  {
    rangeY.Append(r);
  }

/*
4.3 GetNoRangeX, GetNoRangeY

*/
  int Histogram2d::GetNoRangeX() const
  {
    return rangeX.Size();
  }

  int Histogram2d::GetNoRangeY() const
  {
    return rangeY.Size();
  }

/*
4.4 IsEmpty

*/
  bool Histogram2d::IsEmpty() const
  {
    return (rangeX.Size() == 0) && (rangeY.Size() == 0) && (bin.Size() == 0);
  }

/*
4.5 Clear, Destroy

*/
  void Histogram2d::Clear()
  {
    rangeX.clean();
    rangeY.clean();
    bin.clean();
  }

  void Histogram2d::Destroy()
  {
    rangeX.Destroy();
    rangeY.Destroy();
    bin.Destroy();
  }

/*
4.6 ResizeRangeX, ResizeRangeY

*/
  void Histogram2d::ResizeRangeX(const int newSize)
  {
    if (newSize > 0)
    {
      rangeX.clean();
      rangeX.resize(newSize);
    }
  }

  void Histogram2d::ResizeRangeY(const int newSize)
  {
    if (newSize > 0)
    {
      rangeY.clean();
      rangeY.resize(newSize);
    }
  }

/*
4.7 Compare

*/
  int Histogram2d::Compare(const BaseHistogram& b) const
  {
    const Histogram2d* h = static_cast<const Histogram2d*>(&b);

    if ( !IsDefined() && !h->IsDefined() )
      return 0;
    if ( !IsDefined() && h->IsDefined() )
      return -1;
    if (IsDefined() && !h->IsDefined() )
      return 1;

    // Both histograms are defined...

    if (!IsConsistent(false) || !h->IsConsistent(false))
      return 1;

    // and consistent (without order test)...

    if (GetNoRangeX() != h->GetNoRangeX())
      return 1;

    if (GetNoRangeY() != h->GetNoRangeY())
      return 1;

    // and are of the same size ...

    if (IsEmpty() && h->IsEmpty())
      return 0;

    // and are not empty.

    // Comparison of all couples of ranges and bins:

    bool equalBins = true;
    bool lessBins = true;
    int i = 0;

    while ((equalBins || lessBins) && i < GetNoBin())
    {
      if (i < GetNoRangeX() && (GetRangeX(i) != h->GetRangeX(i)))
        return 1;

      if (i < GetNoRangeY() && (GetRangeY(i) != h->GetRangeY(i)))
        return 1;

      if (GetBin(i) == h->GetBin(i))
      {
        lessBins = false;
      }
      else if (GetBin(i) < h->GetBin(i))
      {
        equalBins = false;
      }
      else // GetBin(i) > h->GetBin(i)
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

    // Neither smaller nor equal:
    return 1;
  }

/*
4.8 CompareAlmost

*/
  int Histogram2d::CompareAlmost(const BaseHistogram& b) const
  {
    const Histogram2d* h = static_cast<const Histogram2d*>(&b);

    if ( !IsDefined() && !h->IsDefined() )
      return 0;
    if ( !IsDefined() && h->IsDefined() )
      return -1;
    if (IsDefined() && !h->IsDefined() )
      return 1;

    // Both histograms are defined...

    if (!IsConsistent(false) || !h->IsConsistent(false))
      return 1;

    // and consistent (without order test)...

    if (GetNoRangeX() != h->GetNoRangeX())
      return 1;

    if (GetNoRangeY() != h->GetNoRangeY())
      return 1;

    // and are of the same size...

    if (IsEmpty() && h->IsEmpty())
      return 0;

    // and are not empty.

    // Comparison of all couples of ranges and bins:

    bool equalBins = true;
    bool lessBins = true;
    int i = 0;
    int cmp;

    while ((equalBins || lessBins) && i < GetNoBin())
    {
      if (i < GetNoRangeX() && !AlmostEqual(GetRangeX(i), h->GetRangeX(i)))
        return 1;

      if (i < GetNoRangeY() && !AlmostEqual(GetRangeY(i), h->GetRangeY(i)))
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

    // Neither smaller nor equal:
    return 1;
  }

/*
4.9 IsRefinement

*/
  bool Histogram2d::IsRefinement(const BaseHistogram& b) const
  {
    const Histogram2d* h = static_cast<const Histogram2d*>(&b);

    if ( !IsDefined() || !h->IsDefined() )
      return false;

    // Both histograms are defined...

    if (!IsConsistent(false) || !h->IsConsistent(false))
      return false;

    // and consistent (without order test)...

    if (IsEmpty() || h->IsEmpty())
      return false;

    // and are not empty...

    if (!AlmostEqual( GetRangeX(GetNoRangeX()-1),
                      h->GetRangeX(h->GetNoRangeX()-1)))
      return false;

    if (!AlmostEqual( GetRangeY(GetNoRangeY()-1),
                      h->GetRangeY(h->GetNoRangeY()-1)))
      return false;

    // and their ends are equal.

    // Now we check each element of the given range array to be also an element
    // of our own array. If that is the case, our own histogram is a
    // refinement of the given one.

    int i = 0; // Index of the range array given as parameter.
    int j = 0; // Index of our own range array.

    while (i < h->GetNoRangeX())
    {
      while (!AlmostEqual(h->GetRangeX(i), this->GetRangeX(j)))
      {
        if (++j == this->GetNoRangeX())
          return false;
      }

      i++;
    }

    i = 0;
    j = 0;

    while (i < h->GetNoRangeY())
    {
      while (!AlmostEqual(h->GetRangeY(i), this->GetRangeY(j)))
      {
        if (++j == this->GetNoRangeY())
          return false;
      }

      i++;
    }

    return true;
  }

/*
4.10 IsConsistent

A defined histogram2d is consistent if

it is either empty or all of the following statements are true:

  (1) GetNoRangeX() $>$ 1 and

  (2) GetNoRangeY() $>$ 1 and

  (3) GetNoBin() == (GetNoRangeX() - 1) * (GetNoRangeY() - 1) and

  (4) both range arrays are sorted in ascending order and

  (5) it contains no duplicate entries.

*/
  bool Histogram2d::IsConsistent(const bool checkOrder) const
  {
    if (IsEmpty())
      return true;

    if (!(GetNoRangeX() > 1 && GetNoRangeY() > 1 &&
        GetNoBin() == (GetNoRangeX() - 1) * (GetNoRangeY() - 1)))
      return false;

    if (checkOrder)
    {
      HIST_REAL min = GetRangeX(0);
      for (int i = 1; i < GetNoRangeX(); i++)
      {
        if (CmpReal(GetRangeX(i), min) < 1) // GetRangeX(i) <= min
          return false;

        min = GetRangeX(i);
      }

      min = GetRangeY(0);
      for (int i = 1; i < GetNoRangeY(); i++)
      {
        if (CmpReal(GetRangeY(i), min) < 1) // GetRangeY(i) <= min
          return false;

        min = GetRangeY(i);
      }
    }

    return true;
  }

/*
4.11 Coarsen

*/
  void Histogram2d::Coarsen(const  BaseHistogram* b)
  {
    const Histogram2d* pattern = static_cast<const Histogram2d*>(b);
    // Precondition: this->IsRefinement(pattern) == true
    DbArray<HIST_REAL>* newBin = new DbArray<HIST_REAL>(GetNoBin());
    HIST_REAL binVal;

    // Upper range of the current bin in the original histogram
    int myRangeX = 1;
    int myRangeY = 1;

    // Lower range of the current bin in the example histogram
    int patternRangeX = 1;
    int patternRangeY = 1;

    //
    int maxY = myRangeY;
    int initY = myRangeY;
    int maxX = myRangeX;
    int initX = myRangeX;

    for (int i = 0; i < pattern->GetNoBin(); i++)
    {
      binVal = 0;

      // Traversing the bins by columns
      patternRangeY = i%(pattern->GetNoRangeY() - 1) + 1;
      patternRangeX = i/(pattern->GetNoRangeY() - 1) + 1;


      if (maxY == GetNoRangeY())
      {
        maxY = 1;
        initX = maxX;
      }
      initY = maxY;

      myRangeX = initX;

      // Merging bins
      while (myRangeX < GetNoRangeX() &&
          CmpReal( GetRangeX(myRangeX),
                   pattern->GetRangeX(patternRangeX)) != 1)
      {
        myRangeY = initY;
        while ( myRangeY < GetNoRangeY() &&
                CmpReal(GetRangeY(myRangeY),
                        pattern->GetRangeY(patternRangeY)) != 1)
        {
          binVal += GetBin((myRangeX - 1)*(GetNoRangeY() - 1) +
              myRangeY - 1);
          myRangeY++;
          maxY = max(maxY, myRangeY);
        }
        myRangeX++;
        maxX = max(maxX, myRangeX);
      }
      newBin->Append(binVal);

    } // (int i = 0; i < pattern->GetNoBin(); i++)

    // Copying new bins and ranges
    HIST_REAL p;

    rangeX.clean();
    rangeY.clean();
    bin.clean();
    for (int i = 0; i < pattern->GetNoRangeX(); i++)
    {
      rangeX.Append(pattern->GetRangeX(i));
    }
    for (int i = 0; i < pattern->GetNoRangeY(); i++)
    {
      rangeY.Append(pattern->GetRangeY(i));
    }
    for (int i = 0; i < newBin->Size(); i++)
    {
      newBin->Get(i, p);
      bin.Append(p);
    }
    delete newBin;
  }

/*

5 Mandatory functions

5.1 Operators == and $<$

*/
  bool Histogram2d::operator == (const BaseHistogram& h) const
  {
    return CompareAlmost(h) == 0;
  }

  bool Histogram2d::operator < (const BaseHistogram& h) const
  {
    return CompareAlmost(h) == -1;
  }
/*
5.2 In

*/
  Word Histogram2d::In(const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct)
  {
    NList in(instance);
    if ( listutils::isSymbolUndefined(instance) || ((in.length()==1) &&
                        (listutils::isSymbolUndefined(nl->First(instance))))){
      correct = true;
      return SetWord(new Histogram2d(false));
    }

    Word w = SetWord(Address(0) );

    Histogram2d* newHist = new Histogram2d(true);
    const ListExpr *concise = &instance;
    ListExpr rangeXList;
    ListExpr rangeYList;
    ListExpr binList;
    ListExpr restItems;
    HIST_REAL minimum = -1000 * FLT_MAX;
    HIST_REAL currentValue = 0.0;
    HIST_REAL lastValue = minimum;

    /* We expect the incoming data to be formatted as follows:
     * ((rangeX*)(rangeY*)(bin*))
     * with rangeX a list of (n+1), rangeY a list of (m+1) class limits
     * each in ascending order and  bin a list of (n * m) values.
     * To address the content of a certain bin we will indicate
     * first the rangeX value and second the rangeY value.
     * All values must be of type real.
     *
     * Example:
     * y=2 0.1 0.5 1.0 1.5    1.5 will be addressed as rangeX=3, rangeY=2.
     * y=1 1.1 2.2 6.6 7.7    4.4 will be addressed as rangeX=1, rangeY=0.
     * y=0 5.5 4.4 3.3 2.2
     *     x=0 x=1 x=2 x=3
     */

    if (    (nl->IsEmpty(*concise))          // incoming list is empty
        ||  (nl->ListLength(*concise) != 3)  // or is too short or too long
        ||  (nl->IsAtom(nl->First(*concise))) //or does not contain lists
        ||  (nl->IsAtom(nl->Second(*concise)))
        ||  (nl->IsAtom(nl->Third(*concise)))
        //or number of bins is not (number of rangeX -1 )(number of rangeY - 1)
        ||  (nl->ListLength(nl->Third(*concise))
              != (nl->ListLength(nl->First(*concise))-1) //
                 * (nl->ListLength(nl->Second(*concise))-1))
       ){
      correct = false;
      newHist->DeleteIfAllowed();
      return SetWord( Address(0) );
    } // if (    (nl->IsEmpty(*concise))      // incoming list is empty

    rangeXList = nl->First(*concise);
    rangeYList = nl->Second(*concise);
    binList = nl->Third(*concise);

    // Examine the rangeXList
    restItems = rangeXList;

    while ( !nl->IsEmpty(restItems) ){
      const ListExpr curr = nl->First(restItems);
      if(nl->IsAtom(curr) && (nl->AtomType(curr) == RealType)){
        currentValue = nl->RealValue(curr);
        restItems = nl->Rest(restItems);
        if (currentValue > lastValue){
          newHist->AppendRangeX(currentValue);
          lastValue = currentValue;
        } else{// if (currentValue > lastValue)
          correct = false;
          newHist->DeleteIfAllowed();
          return SetWord( Address(0) );
        } // else // if (currentItem > lastItem)
      } else{// if(nl->IsAtom(curr) && (nl->AtomType(curr) == RealType))
        correct = false;
        newHist->DeleteIfAllowed();
        return SetWord( Address(0) );
      } // else // if(nl->IsAtom(nl->First(rangeList) &&
    } // while ( !nl->IsEmpty(rest) )

    currentValue = 0.0;
    lastValue = minimum;


    // Examine the rangeYList
    restItems = rangeYList;

    while ( !nl->IsEmpty(restItems) ){
      const ListExpr curr = nl->First(restItems);
      if(nl->IsAtom(curr) && (nl->AtomType(curr) == RealType)){
        currentValue = nl->RealValue(curr);
        restItems = nl->Rest(restItems);
        if (currentValue > lastValue){
          newHist->AppendRangeY(currentValue);
          lastValue = currentValue;
        } else {// if (currentValue > lastValue)
          correct = false;
          newHist->DeleteIfAllowed();
          return SetWord( Address(0) );
        } // else // if (currentValue > lastValue)
      } else { // if(nl->IsAtom(curr) && (nl->AtomType(curr) == RealType))
        correct = false;
        newHist->DeleteIfAllowed();
        return SetWord( Address(0) );
      } // else // if(nl->IsAtom(curr) && (nl->AtomType(curr) == RealType))
    } // while ( !nl->IsEmpty(rest) )

    currentValue = 0.0;

    // Now examine the bin list
    restItems = binList;
    while ( !nl->IsEmpty(restItems) ){
      const ListExpr curr = nl->First(restItems);
      if(nl->IsAtom(curr) && (nl->AtomType(curr) == RealType)){
        currentValue = nl->RealValue(curr);
        restItems = nl->Rest(restItems);
        newHist->AppendBin(currentValue);
      } // if(nl->IsAtom(curr) && (nl->AtomType(curr) == RealType))
      else {// if(nl->IsAtom(nl->First(rangeList) && ...
        correct = false;
        newHist->DeleteIfAllowed();
        return SetWord( Address(0) );
      } // if(nl->IsAtom(nl->First(binList) && ...
    } // while ( !nl->IsEmpty(restItems) )

    correct = true;
    //cout << *newHist << " hash = " << newHist->HashValue() << endl;
    return SetWord(newHist);

  } // Word Histogram2d::In(...)

/*
5.3 Out

*/
  ListExpr Histogram2d::Out(ListExpr typeInfo, Word value)
  {
    Histogram2d* hist = static_cast<Histogram2d*>(value.addr);
    const int numberRangesX = hist->GetNoRangeX();
    const int numberRangesY = hist->GetNoRangeY();
    const int numberBins    = hist->GetNoBin();

    //cout << *hist << " hash = " << hist->HashValue() << endl;

    if (!hist->IsDefined())
    {
      NList result = NList(Symbol::UNDEFINED());
      return result.listExpr();
    }
    else if(hist->IsEmpty())
    {
      return (nl->TheEmptyList());
    }
    else // else if( newHist->IsEmpty() ) {
    {
      ListExpr rangeXExpr = nl->Empty();
      ListExpr rangeYExpr = nl->Empty();
      ListExpr binExpr    = nl->Empty();

      // set up rangeXList
      ListExpr last = rangeXExpr;
      int i = 0;
      for (int i=0; i < numberRangesX; i++)
      {
        const ListExpr newElem = nl->RealAtom(hist->GetRangeX(i));
        if (nl->IsEmpty(rangeXExpr))
        {
          rangeXExpr = nl->OneElemList(newElem);
          last = rangeXExpr;
        } // if (nl->IsEmpty(rangeXExpr))
        else
          last = nl->Append(last, newElem);
      } // for (i=0; i < numberRangesX; i++)

      // set up rangeYList
      last = rangeYExpr;
      for (i=0; i < numberRangesY; i++)
      {
        const ListExpr newElem = nl->RealAtom(hist->GetRangeY(i));
        if (nl->IsEmpty(rangeYExpr))
        {
          rangeYExpr = nl->OneElemList(newElem);
          last = rangeYExpr;
        } // if (nl->IsEmpty(rangeYExpr))
        else
          last = nl->Append(last, newElem);
      } // for (i=0; i < numberRangesY; i++)

      // set up binList
      i = 0;
      last = binExpr;
      for (i=0; i < numberBins; i++)
      {
        const ListExpr newElem = nl->RealAtom(hist->GetBin(i));
        if (nl->IsEmpty(binExpr))
        {
          binExpr = nl->OneElemList(newElem);
          last = binExpr;
        }
        else
          last = nl->Append(last, newElem);
      } // for (i=0; i < numberBins; i++)

      return nl->ThreeElemList(rangeXExpr, rangeYExpr, binExpr);
    } // else { // if( newHist->IsEmpty() ) {

    // will (hopefully) never be reached:
    return (nl->TheEmptyList());
  } // ListExpr Histogram2d::Out(ListExpr typeInfo, Word value)

/*
5.4 Insert

*/
  void Histogram2d::Insert(const int index, const HIST_REAL& val)
  {
    assert(index >= 0 && index < GetNoBin());

    HIST_REAL oldVal;
    bin.Get(index, &oldVal);
    HIST_REAL newVal = val + oldVal;
    bin.Put(index, newVal);
  }

/*
Increases the content of the corresponding container
with the value weight.

Returns true if a correspondig container (bin) exists, false if not.

*/
  bool Histogram2d::Insert(const HIST_REAL& x, const HIST_REAL& y,
      const HIST_REAL& weight)
  {
    CcInt index = FindBin(x, y);

    if (index.IsDefined())
    {
      Insert(index.GetValue(), weight);
      return true;
    }
    else
    return false;
  }

/*
5.5 FindBin, FindBinX, FindBinY

*/
  const CcInt Histogram2d::FindBinX(const HIST_REAL& x) const
  {
    if (GetNoRangeX() < 2 ||
        x < GetRangeX(0) ||
        x >= GetRangeX(GetNoRangeX() - 1))
    {
      return CcInt(false, 0);
    }

    int posX;

    rangeX.Find( &x, BaseHistogram::CmpBinSearch, posX);

    // bin-Koordinate ist Range-Koordinate - 1
    return CcInt(true, posX-1);
  }

  const CcInt Histogram2d::FindBinY(const HIST_REAL& y) const
  {
    if (GetNoRangeY() < 2 ||
        y < GetRangeY(0) ||
        y >= GetRangeY(GetNoRangeY() - 1))
    {
      return CcInt(false, 0);
    }

    int posY;

    rangeY.Find( &y, BaseHistogram::CmpBinSearch, posY);

    // bin-Koordinate ist Range-Koordinate - 1
    return CcInt(true, posY-1);
  }

  CcInt Histogram2d::FindBin(const HIST_REAL& x, const HIST_REAL& y) const
  {
    // Vorbedingung: rangeX und rangeY sind aufsteigend sortiert.

    if (GetNoRangeX() < 2 ||
        x < GetRangeX(0) ||
        x >= GetRangeX(GetNoRangeX() - 1))
    {
      return CcInt(false, 0);
    }

    if (GetNoRangeY() < 2 ||
        y < GetRangeY(0) ||
        y >= GetRangeY(GetNoRangeY() - 1))
    {
      return CcInt(false, 0);
    }

    // Binaere Suche nach dem richtigen Behaelter:
    int posX, posY;

    rangeX.Find( &x, BaseHistogram::CmpBinSearch, posX);
    rangeY.Find( &y, BaseHistogram::CmpBinSearch, posY);

    int binArrayIndex = (posX - 1) * (GetNoRangeY() - 1) + (posY - 1);

    return CcInt(true, binArrayIndex);
  }

/*
5.6 GetBinCoords

Projection of the bin array index to 2d coordinates

*/
  pair<CcInt, CcInt> Histogram2d::GetBinCoords(const int index) const
  {
    if (IsEmpty() || index < 0 || index >= GetNoBin())
      return pair<CcInt, CcInt>(CcInt(false, -1), CcInt(false, -1));

    int x = index / (GetNoRangeY() - 1);
    int y = index % (GetNoRangeY() - 1);

    return pair<CcInt, CcInt>(CcInt(true, x), CcInt(true, y));
  }

/*
5.7 CopyRangesFrom

*/
  void Histogram2d::CopyRangesFrom(const BaseHistogram* h)
    {
      Histogram2d* src = (Histogram2d*)h;

      rangeX.clean();
      rangeY.clean();

      for (int i = 0; i < src->GetNoRangeX(); i++)
        this->AppendRangeX(src->GetRangeX(i));

      for (int i = 0; i < src->GetNoRangeY(); i++)
        this->AppendRangeY(src->GetRangeY(i));
    }

/*
5.8 NoBinsX, NoBinsY

*/
  int Histogram2d::NoBinsX() const
  {
    return max(0, GetNoRangeX() - 1);
  }

  int Histogram2d::NoBinsY() const
  {
    return max(0, GetNoRangeY() - 1);
  }

/*
5.9 GetCount

*/
  CcReal Histogram2d::GetCount(const int x, const int y) const
  {
    if (!IsDefined() || IsEmpty() || !IsConsistent(false))
      return CcReal(false, 0.0);

    if (x < 0 || x >= NoBinsX())
      return CcReal(false, 0.0);

    if (y < 0 || y >= NoBinsY())
      return CcReal(false, 0.0);

    return CcReal(true, GetBin(x * NoBinsY() + y));
  }

/*
6 Operators

6.1 MeanX, MeanY

Implementation copied from gsl[_]histogram[_]stat2d.c of the GNU Scientific
Library (gsl-1.9). Author: Achim Gaedke, Jan. 2002.

*/

  CcReal Histogram2d::MeanX() const
  {
    if (!IsDefined() || IsEmpty() || !IsConsistent(false))
      return CcReal(false, 0.0);

    const size_t nx = this->NoBinsX();
    const size_t ny = this->NoBinsY();
    size_t i;
    size_t j;

/*
Compute the bin-weighted arithmetic mean M of a histogram using the
recurrence relation

        M(n) = M(n-1) + (x[n] - M(n-1)) (w(n)/(W(n-1) + w(n)))

        W(n) = W(n-1) + w(n)

We skip negative values in the bins, since those correspond to negative
weights (BJG).

*/
    long double wmean = 0;
    long double W = 0;

    for (i = 0; i < nx; i++)
    {
      double xi = (GetRangeX(i + 1) + GetRangeX(i)) / 2.0;
      double wi = 0;

      for (j = 0; j < ny; j++)
      {
        double wij = GetBin(i * ny + j);
        if (wij > 0)
        wi += wij;
      }

      if (wi > 0)
      {
        W += wi;
        wmean += (xi - wmean) * (wi / W);
      }
    }

    return CcReal(true, wmean);
  }


  CcReal Histogram2d::MeanY() const
  {
    if (!IsDefined() || IsEmpty() || !IsConsistent(false))
      return CcReal(false, 0.0);

    const size_t nx = this->NoBinsX();
    const size_t ny = this->NoBinsY();
    size_t i;
    size_t j;

/*
Compute the bin-weighted arithmetic mean M of a histogram using the
recurrence relation

        M(n) = M(n-1) + (x[n] - M(n-1)) (w(n)/(W(n-1) + w(n)))

        W(n) = W(n-1) + w(n)

*/
    long double wmean = 0;
    long double W = 0;

    for (j = 0; j < ny; j++)
    {
      double yj = (GetRangeY(j + 1) + GetRangeY(j)) / 2.0;
      double wj = 0;

      for (i = 0; i < nx; i++)
      {
        double wij = GetBin(i * ny + j);
        if (wij > 0)
        wj += wij;
      }

      if (wj > 0)
      {
        W += wj;
        wmean += (yj - wmean) * (wj / W);
      }
    }

    return CcReal(true, wmean);
  }

/*
6.2 VarianceX, VarianceY

Implementation copied from gsl[_]histogram[_]stat2d.c of
the GNU Scientific Library (gsl-1.9). Author: Achim Gaedke, Jan. 2002.

*/
  CcReal Histogram2d::VarianceX() const
  {
    if (!IsDefined() || IsEmpty() || !IsConsistent(false))
      return CcReal(false, 0.0);

    const double xmean = MeanX().GetValue();
    const size_t nx = this->NoBinsX();
    const size_t ny = this->NoBinsY();
    size_t i;
    size_t j;

    long double wvariance = 0;
    long double W = 0;

    for (i = 0; i < nx; i++)
    {
      double xi = (GetRangeX(i + 1) + GetRangeX(i)) / 2.0 - xmean;
      double wi = 0;

      for (j = 0; j < ny; j++)
      {
        double wij = GetBin(i * ny + j);
        if (wij > 0)
        wi += wij;
      }

      if (wi > 0)
      {
        W += wi;
        wvariance += ((xi * xi) - wvariance) * (wi / W);
      }
    }

    return CcReal(true, wvariance);
  }


  CcReal Histogram2d::VarianceY() const
  {
    if (!IsDefined() || IsEmpty() || !IsConsistent(false))
      return CcReal(false, 0.0);

    const double ymean = MeanY().GetValue();
    const size_t nx = this->NoBinsX();
    const size_t ny = this->NoBinsY();
    size_t i;
    size_t j;

    long double wvariance = 0;
    long double W = 0;

    for (j = 0; j < ny; j++)
    {
      double yj = (GetRangeY(j + 1) + GetRangeY(j)) / 2.0 - ymean;
      double wj = 0;

      for (i = 0; i < nx; i++)
      {
        double wij = GetBin(i * ny + j);
        if (wij > 0)
        wj += wij;
      }
      if (wj > 0)
      {
        W += wj;
        wvariance += ((yj * yj) - wvariance) * (wj / W);
      }
    }

    return CcReal(true, wvariance);
  }

/*
6.3 Covariance

Implementation copied from gsl[_]histogram[_]stat2d.c of
the GNU Scientific Library (gsl-1.9). Author: Achim Gaedke, Jan. 2002.

*/
  CcReal Histogram2d::Covariance() const
  {
    if (!IsDefined() || IsEmpty() || !IsConsistent(false))
      return CcReal(false, 0.0);

    const double xmean = MeanX().GetValue();
    const double ymean = MeanY().GetValue();
    const size_t nx = this->NoBinsX();
    const size_t ny = this->NoBinsY();
    size_t i;
    size_t j;

    long double wcovariance = 0;
    long double W = 0;

    for (j = 0; j < ny; j++)
    {
      for (i = 0; i < nx; i++)
      {
        double xi = (GetRangeX(i + 1) + GetRangeX(i)) / 2.0 - xmean;
        double yj = (GetRangeY(j + 1) + GetRangeY(j)) / 2.0 - ymean;
        double wij = GetBin(i * ny + j);

        if (wij > 0)
        {
          W += wij;
          wcovariance += ((xi * yj) - wcovariance) * (wij / W);
        }
      }
    }

    return CcReal(true, wcovariance);
  }

/*
6.4 Sizeof

*/
  size_t Histogram2d::Sizeof() const
  {
    return sizeof(*this);
  }

/*
6.5 Compare, CompareAlmost

*/
  int Histogram2d::Compare(const Attribute *rhs) const
  {
    return Compare(*(Histogram2d*)rhs);
  }

  int Histogram2d::CompareAlmost(const Attribute *rhs) const
  {
    return CompareAlmost(*(Histogram2d*)rhs);
  }

/*
6.6 Adjacent

*/
  bool Histogram2d::Adjacent(const Attribute *attrib) const
  {
    return false;
  }

/*
6.7 Clone

*/
  Attribute* Histogram2d::Clone() const
  {
    return new Histogram2d(*this);
  }

/*
6.8 HashValue

Algorithm according to John Hamer.

Hashing revisited.

In ITiCSE2002 Seventh Conference on
Innovation and Technology in Computer Science Education,
pages 80-83, Aarhus, Denmark, June 2002.

*/
  size_t Histogram2d::HashValue() const
  {
    if (!IsDefined())
    return 0;

    size_t h = 0;

    for (int i = 0; i < GetNoRangeX(); i++)
    {
      h = hgr::Rotate(h) ^ hgr::HashValue(GetRangeX(i));
    }

    for (int i = 0; i < GetNoRangeY(); i++)
    {
      h = hgr::Rotate(h) ^ hgr::HashValue(GetRangeY(i));
    }

    for (int i = 0; i < GetNoBin(); i++)
    {
      h = hgr::Rotate(h) ^ hgr::HashValue(GetBin(i));
    }

    return h;
  }

/*
6.9 CopyFrom

*/
  void Histogram2d::CopyFrom(const Attribute* right)
  {
    const Histogram2d* hist = static_cast<const Histogram2d*>(right);

    SetDefined(hist->IsDefined());
    CopyRangesFrom(hist);
    CopyBinsFrom(hist);
  }

/*
6.10 NumOfFLOBs

*/
  int Histogram2d::NumOfFLOBs() const
  {
    return 3;
  }

/*
6.11 GetFLOB

*/
  Flob* Histogram2d::GetFLOB( const int i )
  {
    assert( i >= 0 && i < NumOfFLOBs() );

    if (i == 0)
      return &rangeX;
    else if(i == 1)
      return &rangeY;
    else
      return &bin;
  }

/*
6.12 Cast

*/
  void* Histogram2d::Cast(void* addr)
  {
    return (new (addr) Histogram2d);
  }

/*
6.13 Create

*/
  Word Histogram2d::Create( const ListExpr typeInfo )
  {
    return SetWord( new Histogram2d( true ) );
  }

/*
6.14 KindCheck or CheckKind function

*/
  bool Histogram2d::KindCheck(ListExpr type, ListExpr& errorInfo)
  {
    return (nl->IsEqual(type, Histogram2d::BasicType()));
  }

/*
6.15 Print

*/
  ostream& Histogram2d::Print( ostream& os ) const
  {
    return (os << *this);
  }

/*
6.16 Operator <<

*/
  ostream& operator << (ostream& os, const Histogram2d& h)
  {
    if (!h.IsDefined())
    {
      os << "(" << Histogram2d::BasicType() << " undef)";
      return os;
    }

    os << "(" << Histogram2d::BasicType() << "( ( ";

    for (int i = 0; i < h.GetNoRangeX(); i++)
      os << h.GetRangeX(i) << " ";

    os << ") ( ";

    for (int i = 0; i < h.GetNoRangeY(); i++)
      os << h.GetRangeY(i) << " ";

    os << ") ( ";

    for (int i = 0; i < h.GetNoBin(); i++)
      os << h.GetBin(i) << " ";

    os << ") ))";

    return os;
  }

/*
7 The histogram2dInfo struct

*/
  histogram2dInfo::histogram2dInfo()
  {
    name = Histogram2d::BasicType();
    signature = "-> DATA";
    typeExample = Histogram2d::BasicType();
    listRep = "((rangesX*)(rangesY*)(bins*))";
    valueExample= "( (0.0 1.0 ) (0.0 1.5) (2.0) )";
    remarks = "All coordinates must be of type real.";
  } // histogram2dInfo() {


/*
8 Creation of the Type Constructor Instance

*/
  histogram2dFunctions::histogram2dFunctions()
  {
    //reassign some function pointers
    in = Histogram2d::In;
    out = Histogram2d::Out;
    cast = Histogram2d::Cast;
    kindCheck = Histogram2d::KindCheck;
    create = Histogram2d::Create;
  } // histogram2dFunctions() {

/*
9 Type and value mapping functions

9.1 set[_]histogram2d

*/
  ListExpr SetHistogram2dTypeMap(ListExpr args)
  {
    NList list(args);
    string errMsg = "Operator set_histogram2d "
      "expects a list of length two.";

    if (list.length() != 2)
      return list.typeError(errMsg);

    errMsg = "Operator set_histogram2d "
      "expects as first and second argument "
      "a list with structure: (stream real).";

    NList arg1 = list.first();
    NList arg2 = list.second();

    // (stream real) x (stream real) -> histogram2d
    if (arg1.first().isSymbol(Symbol::STREAM()) &&
      arg1.second().isSymbol(CcReal::BasicType()) &&
        arg2.first().isSymbol(Symbol::STREAM()) &&
        arg2.second().isSymbol(CcReal::BasicType()) )
      return NList(Histogram2d::BasicType()).listExpr();

    return list.typeError(errMsg);
  }

  int SetHistogram2dFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    Word elemX, elemY;
    CcReal* streamObjX;
    CcReal* streamObjY;
    HIST_REAL valueX, valueY, lastValueX, lastValueY;
    bool firstElemX, firstElemY;
    bool receivedX, receivedY;

    // The query processor provided an empty Histogram2d-instance:
    result = qp->ResultStorage(s);
    Histogram2d* resultHg = (Histogram2d*) result.addr;
    resultHg->Clear();

    qp->Open(args[0].addr);
    qp->Open(args[1].addr);

    firstElemX = true;
    firstElemY = true;

    // Request the first elements of the streams:
    qp->Request(args[0].addr, elemX);
    qp->Request(args[1].addr, elemY);

    receivedX = qp->Received(args[0].addr);
    receivedY = qp->Received(args[1].addr);

    while (receivedX || receivedY)
    {
      if (receivedX)
      {
        streamObjX = (CcReal*) elemX.addr;
        valueX = streamObjX->GetRealval();

        if (!streamObjX->IsDefined())
        {
          cout << "Error in operator set_histogram2d: "
          << "The first stream contains an undefined value." << endl;
          resultHg->Clear();
          resultHg->SetDefined(false);
          qp->Close(args[0].addr);
          qp->Close(args[1].addr);
          return 0;
        }

        if (!firstElemX && (CmpReal(valueX, lastValueX) < 1))
        {
          cout << "Error in operator set_histogram2d: "
          << "The first stream is not sorted or contains duplicates." << endl;
          resultHg->Clear();
          resultHg->SetDefined(false);
          qp->Close(args[0].addr);
          qp->Close(args[1].addr);
          return 0;
        }

        resultHg->AppendRangeX(valueX);
        lastValueX = valueX;
        firstElemX = false;

        // Consume the stream object:
        streamObjX->DeleteIfAllowed();

        // Request the next element of the first stream:
        qp->Request(args[0].addr, elemX);
        receivedX = qp->Received(args[0].addr);

        if (!receivedX)
          qp->Close(args[0].addr);

      } // if (receivedX)

      if (receivedY)
      {
        streamObjY = (CcReal*) elemY.addr;
        valueY = streamObjY->GetRealval();

        if (!streamObjY->IsDefined())
        {
          cout << "Error in operator set_histogram2d: "
          << "The second stream contains an undefined value." << endl;
          resultHg->Clear();
          resultHg->SetDefined(false);
          qp->Close(args[0].addr);
          qp->Close(args[1].addr);
          return 0;
        }

        if (!firstElemY && (CmpReal(valueY, lastValueY) < 1))
        {
          cout << "Error in operator set_histogram2d: "
          << "The second stream is not sorted or contains duplicates." << endl;
          resultHg->Clear();
          resultHg->SetDefined(false);
          qp->Close(args[0].addr);
          qp->Close(args[1].addr);
          return 0;
        }

        resultHg->AppendRangeY(valueY);
        lastValueY = valueY;
        firstElemY = false;

        // Consume the current stream object:
        streamObjY->DeleteIfAllowed();

        // Request the next element of the second stream:
        qp->Request(args[1].addr, elemY);
        receivedY = qp->Received(args[1].addr);

        if (!receivedY)
          qp->Close(args[1].addr);

      } // if (receivedY)

    } // while (receivedX || receivedY)

    if (resultHg->GetNoRangeX() < 2 || resultHg->GetNoRangeY() < 2)
    {
      cout << "Error in operator set_histogram2d: "
      << "A stream contains less than two elements." << endl;
      resultHg->Clear();
      resultHg->SetDefined(false);
      return 0;
    }

    int noOfBins = (resultHg->GetNoRangeX() - 1)
    * (resultHg->GetNoRangeY() - 1);

    resultHg->ResizeBin(noOfBins);

    for (int i = 0; i < noOfBins; i++)
      resultHg->AppendBin(0.0);

    return 0;
  }

/*
9.2 BinsX, BinsY

*/
  int BinsXFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    const Histogram2d* h1 = static_cast<Histogram2d*>(args[0].addr );
    result = qp->ResultStorage(s);
    CcInt* i = static_cast<CcInt*>(result.addr);

    if (!h1->IsDefined())
    {
      i->Set(false, -1);
      return 0;
    }

    i->Set(true, h1->NoBinsX());
    return 0;
  }

  int BinsYFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    const Histogram2d* h1 = static_cast<Histogram2d*>(args[0].addr );
    result = qp->ResultStorage(s);
    CcInt* i = static_cast<CcInt*>(result.addr);

    if (!h1->IsDefined())
    {
      i->Set(false, -1);
      return 0;
    }

    i->Set(true, h1->NoBinsY());
    return 0;
  }

  ListExpr BinsXYTypeMap(ListExpr args)
  {
    NList list(args);
    const string errMsg = "Expecting (" + Histogram2d::BasicType() + ")";

    if (list.length() != 1)
    return list.typeError(errMsg);

    NList arg1 = list.first();

    // histogram1d -> int
    if (arg1.isSymbol(Histogram2d::BasicType()))
    return NList(CcInt::BasicType()).listExpr();

    return list.typeError(errMsg);
  }

/*
9.3 binrange[_]minX, binrange[_]maxX, binrange[_]minY, binrange[_]maxY

*/
  int binrange_minXFun( Word* args, Word& result,
      int message, Word& local, Supplier s )
  {
    const Histogram2d *hg = static_cast<Histogram2d*>(args[0].addr);
    const CcInt* binNumber = static_cast<CcInt*>(args[1].addr);
    // cerr << "binNumber: " << binNumber->GetIntval( ) << endl;
    result = qp->ResultStorage(s);
    CcReal *r = (CcReal*)result.addr;

    if (!hg->IsDefined() || !binNumber->IsDefined())
    {
      r->Set(false, 0.0);
      return 0;
    }

    int i = binNumber->GetIntval();
    // binNumber i must be on the histogram's X axis
    if ((i >= 0) && (i < (hg->GetNoRangeX()-1)))
    {
      r->Set(hg->GetRangeX(i));
      return 0;
    } // if ((i >= 0) && (i < hg->GetNoRangeX()))

    else
    {
      cerr << "Please indicate as second parameter "
      "an integer value between 0 and " << hg->GetNoRangeX() -2 << "." << endl;
      return 0;
    } // else // if ((i >= 0) && (i < hg->GetNoRangeX()))

  } // int binrange_minXFun( Word* args, Word& result, ...


  int binrange_maxXFun( Word* args, Word& result,
      int message, Word& local, Supplier s )
  {
    const Histogram2d *hg = static_cast<Histogram2d*>(args[0].addr);
    const CcInt* binNumber = static_cast<CcInt*>(args[1].addr);
    // cerr << "binNumber: " << binNumber->GetIntval( ) << endl;
    result = qp->ResultStorage(s);
    CcReal *r = (CcReal*)result.addr;

    if (!hg->IsDefined() || !binNumber->IsDefined())
    {
      r->Set(false, 0.0);
      return 0;
    }

    int i = binNumber->GetIntval();
    // binNumber i must be on the histogram's X axis
    if ((i >= 0) && (i < (hg->GetNoRangeX()-1)))
    {
      r->Set(hg->GetRangeX(i+1));
      return 0;
    } // if ((i >= 0) && (i < hg->GetNoRangeX()))

    else
    {
      cerr << "Please indicate as second parameter "
      "an integer value between 0 and " << hg->GetNoRangeX() -2 << "." << endl;
      return 0;
    } // else // if ((i >= 0) && (i < hg->GetNoRangeX()))

  } // int binrange_maxXFun( Word* args, Word& result, ...


  int binrange_minYFun( Word* args, Word& result,
      int message, Word& local, Supplier s )
  {
    const Histogram2d *hg = static_cast<Histogram2d*>(args[0].addr);
    const CcInt* binNumber = static_cast<CcInt*>(args[1].addr);
    // cerr << "binNumber: " << binNumber->GetIntval( ) << endl;
    result = qp->ResultStorage(s);
    CcReal *r = (CcReal*)result.addr;

    if (!hg->IsDefined() || !binNumber->IsDefined())
    {
      r->Set(false, 0.0);
      return 0;
    }

    int i = binNumber->GetIntval();
    // binNumber i must be on the histogram's Y axis
    if ((i >= 0) && (i < (hg->GetNoRangeY()-1)))
    {
      r->Set(hg->GetRangeY(i));
      return 0;
    } // if ((i >= 0) && (i < hg->GetNoRangeY()))

    else
    {
      cerr << "Please indicate as second parameter "
      "an integer value between 0 and " << hg->GetNoRangeY() -2 << "." << endl;
      return 0;
    } // else // if ((i >= 0) && (i < hg->GetNoRangeY()))

  } // int binrange_minYFun( Word* args, Word& result, ...


  int binrange_maxYFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    const Histogram2d *hg = static_cast<Histogram2d*>(args[0].addr);
    const CcInt* binNumber = static_cast<CcInt*>(args[1].addr);
    // cerr << "binNumber: " << binNumber->GetIntval( ) << endl;
    result = qp->ResultStorage(s);
    CcReal *r = (CcReal*)result.addr;

    if (!hg->IsDefined() || !binNumber->IsDefined())
    {
      r->Set(false, 0.0);
      return 0;
    }

    int i = binNumber->GetIntval();
    // binNumber i must be on the histogram's Y axis
    if ((i >= 0) && (i < (hg->GetNoRangeY()-1)))
    {
      r->Set(hg->GetRangeY(i+1));
      return 0;
    } // if ((i >= 0) && (i < hg->GetNoRangeY()))

    else
    {
      cerr << "Please indicate as second parameter an integer "
      "value between 0 and " << hg->GetNoRangeY() -2 << "." << endl;
      return 0;
    } // else // if ((i >= 0) && (i < hg->GetNoRangeY()))

  } // int binrange_minYFun( Word* args, Word& result, ...



  ListExpr binrange_minXY_maxXYTypeMap (ListExpr args)
  {
    NList argList = NList(args);
    short int ok = 0;

    // we are expecting exactly two arguments
    if (argList.length() == 2)
    {
      NList hist = argList.first();
      NList numberBin = argList.second();

      // the first argument is to be a histogram2d
      if ((hist.isSymbol(Histogram2d::BasicType())))
      {
        ok++;
      } // if ( (hist.isSymbol(Histogram2d::BasicType()) ) )

      else
      {
        argList.typeError("Expecting a histogram2d as first input parameter.");
      } // else // if ( (hist.isSymbol(Histogram2d::BasicType()) ) )

      // and the second argument is to be an integer value
      if (numberBin.isSymbol(CcInt::BasicType()))
      {
        ok++;
      } // if (numberBin.isSymbol(CcInt::BasicType()))

      else
      {
        argList.typeError("Expecting an integer value as "
            "second input parameter.");
      } // else // if (numberBin.isSymbol(CcInt::BasicType()))

    } // if (argList.length() == 2)

    else
    {
      // return list.typeError(errMsg);
      argList.typeError("Expecting exactly two input parameters: "
          "(1) a histogram2d, (2) an integer value "
          "indicating the bin number.");
    }
    if (ok == 2)
    return NList(CcReal::BasicType()).listExpr();
    else
    return argList.typeError( "Expecting exactly two input parameters: "
        "(1) a histogram2d, (2) an integer value "
        "indicating the bin number.");
  } // ListExpr binrange_minX_maxXTypeMap (ListExpr args)

/*
9.4 create[_]histogram2d

Argument 0 tuple stream, 1 attribute name X, 2 attribute name Y,
3 histogram2d, 4 index of attribute X, 5 index of attribute Y

*/
  int CreateHistogram2dFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    Word elem;
    CcInt bin;
    HIST_REAL valX, valY;
    CcReal* attrValX;
    CcReal* attrValY;

    const Histogram2d* hist = (Histogram2d*)args[3].addr;
    const CcInt* indexX = (CcInt*)args[4].addr;
    const CcInt* indexY = (CcInt*)args[5].addr;

    //cout << "indexX " << indexX->GetIntval() << endl;
    //cout << "indexY " << indexY->GetIntval() << endl;

    Tuple* currentTuple;

    qp->Open(args[0].addr);
    qp->Request(args[0].addr, elem);
    result = qp->ResultStorage(s);
    Histogram2d* res = (Histogram2d*)result.addr;
    res->Clear();
    res->CopyFrom(hist);

    while (qp->Received(args[0].addr) )
    {
      // Find bin for current attribute values
      currentTuple = (Tuple*)elem.addr;
      attrValX = (CcReal*)currentTuple->GetAttribute(
          indexX->GetIntval()-1);
      attrValY = (CcReal*)currentTuple->GetAttribute(
          indexY->GetIntval()-1);

      valX = attrValX->GetRealval();
      valY = attrValY->GetRealval();

      // catch empty (== undefined) histograms
      if (res->IsDefined())
      {

        bin = res->FindBin(valX, valY);

        // Increment bin, if it was found
        if (bin.IsDefined())
          res->Insert(bin.GetIntval());
      }

      currentTuple->DeleteIfAllowed();// consume the stream objects
      qp->Request(args[0].addr, elem);
    }

    // return filled histogram2d

    qp->Close(args[0].addr);

    return 0;
  } // int CreateHistogram2dFun


  ListExpr CreateHistogram2dTypeMap(ListExpr args)
  {
    NList argList = NList(args);

    if(argList.length() != 4) {
      return listutils::typeError("Expects 4 arguments.");
    }

    NList stream = argList.first();
    NList attrNameX = argList.second();
    NList attrNameY = argList.third();
    NList hist = argList.fourth();


    if(stream.length() != 2
        || !stream.first().isSymbol(Symbol::STREAM())
        || stream.second().length() != 2
        || !stream.second().first().isSymbol(Tuple::BasicType())
        || !IsTupleDescription(stream.second().second().listExpr())) {
      return listutils::typeError("Expecting stream of tuples");
    }

    // Check if tuple has supplied attributes
    NList tupleDescr = stream.second().second();
    int len = tupleDescr.length();
    bool foundX = false, foundY = false;
    int indexX;
    int indexY;

    int index = 1;
    for (; index <= len; ++index)
    {
      NList attr = tupleDescr.first();
      tupleDescr.rest();

      if (attrNameX == attr.first())
      {
        if(!attr.second().isSymbol(CcReal::BasicType())) {
          return listutils::typeError("AttributeX not of type real");
        }
        foundX = true;
        indexX = index;
      }

      if (attrNameY == attr.first())
      {
        if(!attr.second().isSymbol(CcReal::BasicType())) {
          return listutils::typeError("AttributeY not of type real");
        }
        foundY = true;
        indexY = index;
      }

      // found both attributes
      if (foundX && foundY)
        break;
    }

    if(!foundX) {
      return listutils::typeError("AttributeX not found");
    }
    if(!foundY) {
      return listutils::typeError("AttributeY not found");
    }

    if(!hist.isSymbol(Histogram2d::BasicType())) {
      return listutils::typeError("Histogram2d has wrong type");
    }

    ListExpr result = nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                                        nl->TwoElemList(nl->IntAtom(indexX),
                                                        nl->IntAtom(indexY)),
                                      nl->SymbolAtom(Histogram2d::BasicType()));
    return result;
  } // CreateHistogram2dTypeMap(ListExpr args)

/*
9.5 CreateHistogram2dEquiwidth

*/
   ListExpr CreateHistogram2dEquiwidthTypeMap(ListExpr args)
   {
     string errorMsg;
     NList list(args);

     // Check the list:
     errorMsg = "Operator create_histogram2d_equiwidth "
       "expects a list of length five.";

     if (list.length() != 5)
       return list.typeError(errorMsg);

     NList arg1 = list.first(); // (stream (tuple ((a1 x1) ... (an xn))))
     NList arg2 = list.second(); // (ax)
     NList arg3 = list.third(); // (ay)
     NList arg4 = list.fourth(); // (int)
     NList arg5 = list.fifth(); // (int)


     // Check the first argument:
     // (stream (tuple ((a1 x1) ... (an xn))))
     errorMsg
         = "Operator create_histogram2d_equiwidth expects as first argument "
           "a list with structure\n"
           "(stream (tuple ((a1 t1)...(an tn))))";

     if (!IsStreamDescription(arg1.listExpr()))
       return list.typeError(errorMsg);


     // Check the second argument:
     // (ax)
     errorMsg = "Operator create_histogram2d_equiwidth expects as "
       "second argument an attribute.";

     if (!arg2.isSymbol())
       return list.typeError(errorMsg);

     ListExpr attrlist;
     string attrname;
     int attrindex;
     ListExpr attrtype;
     NList indexlist;

     // Find the attribute in the tuple:
     attrname = arg2.str();
     attrlist = arg1.second().second().listExpr();
     attrindex = FindAttribute(attrlist, attrname, attrtype);

     errorMsg = "Attributename '" + attrname + "' is not known.\n"
       "Known Attribute(s): " + NList(attrlist).convertToString();

     if (attrindex == 0)
       return list.typeError(errorMsg);

     errorMsg = "Operator create_histogram2d_equiwidth expects as "
           "second argument an attribute of type real.";

     if (!NList(attrtype).isSymbol(CcReal::BasicType()))
       return list.typeError(errorMsg);

     // Remember the position of the attribute:
     indexlist.append(NList(attrindex));


     // Check the third argument:
     // (ay)
     errorMsg = "Operator create_histogram2d_equiwidth expects as "
       "third argument an attribute.";

     if (!arg3.isSymbol())
       return list.typeError(errorMsg);

     // Find the attribute in the tuple:
     attrname = arg3.str();
     attrindex = FindAttribute(attrlist, attrname, attrtype);

     errorMsg = "Attributename '" + attrname + "' is not known.\n"
       "Known Attribute(s): " + NList(attrlist).convertToString();

     if (attrindex == 0)
       return list.typeError(errorMsg);

     errorMsg = "Operator create_histogram2d_equiwidth expects as "
               "second argument an attribute of type real.";

     if (!NList(attrtype).isSymbol(CcReal::BasicType()))
       return list.typeError(errorMsg);


     // Remember the position of the attribute:
     indexlist.append(NList(attrindex));


     // Check the fourth argument:
     // (int)
     errorMsg = "Operator create_histogram2d_equiwidth expects as "
       "fourth argument the symbol 'int'.";

     if (!arg4.isSymbol(CcInt::BasicType()))
       return list.typeError(errorMsg);


     // Check the fifth argument:
     // (int)
     errorMsg = "Operator create_histogram2d_equiwidth expects as "
       "fifth argument the symbol 'int'.";

     if (!arg5.isSymbol(CcInt::BasicType()))
       return list.typeError(errorMsg);


     // Everything should be fine.
     // We can build the outlist:
     NList outlist(NList(Symbol::APPEND()), indexlist,
         NList(Histogram2d::BasicType()) );

     //cout << "outlist: " << outlist.convertToString() << endl;

     return outlist.listExpr();
   }

   int CreateHistogram2dEquiwidthFun(Word* args, Word& result, int message,
      Word& local, Supplier s)
  {
    Word elem;
    const size_t MAX_MEMORY = qp->FixedMemory();
    TupleBuffer* buffer = new TupleBuffer(MAX_MEMORY);

    // The query processor provided an empty Histogram2d-instance:
    result = qp->ResultStorage(s);
    Histogram2d* resultHg = (Histogram2d*) result.addr;
    resultHg->Clear();

    // We don't use the attrnames,
    // delivered in args[1].addr and args[2].addr.
    // Get the index of the attributes instead:
    const int attrXIndex = ((CcInt*)args[5].addr)->GetIntval() - 1;
    const int attrYIndex = ((CcInt*)args[6].addr)->GetIntval() - 1;

    if ( !((CcInt*)args[3].addr)->IsDefined() ||
        !((CcInt*)args[4].addr)->IsDefined() )
    {
      resultHg->SetDefined(false);
      delete buffer;
      return 0;
    }

    // Get the max. number of bins:
    int maxBinsX = ((CcInt*)args[3].addr)->GetIntval();
    int maxBinsY = ((CcInt*)args[4].addr)->GetIntval();

    if (maxBinsX < 1)
    {
      cout << "Error in operator create_histogram2d_equiwidth: "
      << "The fourth argument must be greater than zero." << endl;
      resultHg->SetDefined(false);
      delete buffer;
      return 0;
    }

    if (maxBinsY < 1)
    {
      cout << "Error in operator create_histogram2d_equiwidth: "
      << "The fifth argument must be greater than zero." << endl;
      resultHg->SetDefined(false);
      delete buffer;
      return 0;
    }

    // Open the tuple-stream:
    qp->Open(args[0].addr);

    // Request the first tuple of the stream:
    qp->Request(args[0].addr, elem);

    Tuple* tuplePtr;

    CcReal* attrXPtr;
    CcReal* attrYPtr;

    HIST_REAL attrXValue;
    HIST_REAL attrYValue;

    HIST_REAL maxX;
    HIST_REAL minX;

    HIST_REAL maxY;
    HIST_REAL minY;

    bool firstTuple = true;

    while (qp->Received(args[0].addr))
    {
      // Get a pointer to the current tuple:
      tuplePtr = (Tuple*) elem.addr;

      // Get a pointer to the current attributes:
      attrXPtr = (CcReal*)(tuplePtr->GetAttribute(attrXIndex));
      attrYPtr = (CcReal*)(tuplePtr->GetAttribute(attrYIndex));

      // Get the value of the current attributes:
      attrXValue = attrXPtr->GetRealval();
      attrYValue = attrYPtr->GetRealval();

      // We accept only defined attribute-pairs:
      if (attrXPtr->IsDefined() && attrYPtr->IsDefined())
      {
        if (firstTuple)
        {
          maxX = minX = attrXValue;
          maxY = minY = attrYValue;
          firstTuple = false;
        }

        // Is it a new min./max. value?
        if (attrXValue < minX)
        minX = attrXValue;
        else if (attrXValue > maxX)
        maxX = attrXValue;

        if (attrYValue < minY)
        minY = attrYValue;
        else if (attrYValue > maxY)
        maxY = attrYValue;

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
      cout << "Error in operator create_histogram2d_equiwidth: "
      << "The stream contains no valid elements." << endl;
      resultHg->SetDefined(false);
      delete buffer;
      return 0;
    }

    if (AlmostEqual(minX, maxX))
    {
      // We got just one, or equal x-values:
      maxX += FACTOR;
      minX -= FACTOR;
      // We need just one bin:
      maxBinsX = 1;
    }

    if (AlmostEqual(minY, maxY))
    {
      // We got just one, or equal y-values:
      maxY += FACTOR;
      minY -= FACTOR;
      // We need just one bin:
      maxBinsY = 1;
    }

    const HIST_REAL valueRangeX = fabs(maxX - minX);
    const HIST_REAL valueRangeY = fabs(maxY - minY);
    const HIST_REAL binWidthX = valueRangeX / maxBinsX;
    const HIST_REAL binWidthY = valueRangeY / maxBinsY;
    const HIST_REAL firstBoundX = minX;
    const HIST_REAL firstBoundY = minY;

    // Create the rangeX-array.
    // It's size is: maxBinsX + 1

    resultHg->ResizeRangeX(maxBinsX + 1);

    for (int i = 0; i < maxBinsX; i++)
    {
      resultHg->AppendRangeX(firstBoundX + binWidthX * i);
    }

    // Append the last rangeX:
    resultHg->AppendRangeX(maxX + 10 * FACTOR);

    // Create the rangeY-array.
    // It's size is: maxBinsY + 1

    resultHg->ResizeRangeY(maxBinsY + 1);

    for (int i = 0; i < maxBinsY; i++)
    {
      resultHg->AppendRangeY(firstBoundY + binWidthY * i);
    }

    // Append the last rangeY:
    resultHg->AppendRangeY(maxY + 10 * FACTOR);

    // Init the bin-array with empty bins.
    // It's size is: maxBinsX * maxBinsY
    const int noBins = maxBinsX * maxBinsY;

    resultHg->ResizeBin(noBins);

    for (int i = 0; i < noBins; i++)
    {
      resultHg->AppendBin(0.0);
    }

    GenericRelationIterator* it = buffer->MakeScan();

    // Fill the histogram with the buffered values:
    while ((tuplePtr = it->GetNextTuple()) != 0)
    {
      attrXPtr = (CcReal*)(tuplePtr->GetAttribute(attrXIndex));
      attrYPtr = (CcReal*)(tuplePtr->GetAttribute(attrYIndex));

      attrXValue = attrXPtr->GetRealval();
      attrYValue = attrYPtr->GetRealval();

      resultHg->Insert(attrXValue, attrYValue);

      tuplePtr->DeleteIfAllowed();
    }

    delete it;
    buffer->Clear();
    delete buffer;

    return 0;
  }


/*
9.6 Shrink2d

Argument 0 histogram, 1 lower bound X, 2 upper bound X,
3 lower bound Y, 4 upper bound Y

*/
  template<bool eager>
  int Shrink2dFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    const Histogram2d* hist = (const Histogram2d*)args[0].addr;
    const CcReal* loX = (const CcReal*)args[1].addr;
    const CcReal* hiX = (const CcReal*)args[2].addr;
    const CcReal* loY = (const CcReal*)args[3].addr;
    const CcReal* hiY = (const CcReal*)args[4].addr;

    result = qp->ResultStorage(s);
    Histogram2d* res = (Histogram2d*)result.addr;
    res->Clear();

    if (!hist->IsDefined() ||
        hist->IsEmpty() ||
        !hist->IsConsistent(false) ||
        !loX->IsDefined() ||
        !hiX->IsDefined() ||
        !loY->IsDefined() ||
        !hiY->IsDefined())
    {
      res->SetDefined(false);
      return 0;
    }

    const HIST_REAL lowX = loX->GetRealval();
    const HIST_REAL highX = hiX->GetRealval();
    const HIST_REAL lowY = loY->GetRealval();
    const HIST_REAL highY = hiY->GetRealval();

    if (CmpReal(highX, lowX) != 1 || CmpReal(highY, lowY) != 1)
    {
      cout << "Error in operator shrink_x: "
      << "The upper bound must be greater than the lower bound." << endl;
      res->SetDefined(false);
      return 0;
    }

    if ((lowX < hist->GetRangeX(0) && highX < hist->GetRangeX(0)) || (lowX
            > hist->GetRangeX(hist->GetNoRangeX()-1) && highX
            > hist->GetRangeX(hist->GetNoRangeX()-1)))
    {
      cout << "Error in operator shrink_x: "
      << "The specified x-interval is outside the histogram-xrange."
      << endl;
      res->SetDefined(false);
      return 0;
    }

    if ((lowY < hist->GetRangeY(0) && highY < hist->GetRangeY(0)) || (lowY
            > hist->GetRangeY(hist->GetNoRangeY()-1) && highY
            > hist->GetRangeY(hist->GetNoRangeY()-1)))
    {
      cout << "Error in operator shrink_x: "
      << "The specified y-interval is outside the histogram-yrange."
      << endl;
      res->SetDefined(false);
      return 0;
    }

    CcInt lowerX = hist->FindBinX(lowX);
    CcInt upperX = hist->FindBinX(highX);
    CcInt lowerY = hist->FindBinY(lowY);
    CcInt upperY = hist->FindBinY(highY);

    int loRangeX = 0;
    int upRangeX = hist->GetNoRangeX() - 1;
    int loRangeY = 0;
    int upRangeY = hist->GetNoRangeY() - 1;

    if (lowerX.IsDefined())
    loRangeX = lowerX.GetIntval();

    if (upperX.IsDefined())
    upRangeX = upperX.GetIntval() + 1;

    if (lowerY.IsDefined())
    loRangeY = lowerY.GetIntval();

    if (upperY.IsDefined())
    upRangeY = upperY.GetIntval() + 1;

    if (eager)
    {
      if (lowerX.IsDefined() && !AlmostEqual(lowX, hist->GetRangeX(loRangeX)))
      {
        // Drop the first xrange,
        // if the lower bound is inside the histogram-xrange and
        // we do not hit the bound of the bin.
        loRangeX++;
      }

      if (upperX.IsDefined())
      {
        // Drop the last xrange,
        // if the upper bound is inside the histogram-xrange.
        upRangeX--;
      }

      if (lowerY.IsDefined() && !AlmostEqual(lowY, hist->GetRangeY(loRangeY)))
      {
        // Drop the first yrange,
        // if the lower bound is inside the histogram-yrange and
        // we do not hit the bound of the bin.
        loRangeY++;
      }

      if (upperY.IsDefined())
      {
        // Drop the last yrange,
        // if the upper bound is inside the histogram-yrange.
        upRangeY--;
      }
    }

    res->ResizeRangeX(upRangeX - loRangeX + 1);

    for (int i = loRangeX; i <= upRangeX; ++i)
    res->AppendRangeX(hist->GetRangeX(i));

    res->ResizeRangeY(upRangeY - loRangeY + 1);

    for (int j = loRangeY; j <= upRangeY; ++j)
    res->AppendRangeY(hist->GetRangeY(j));

    const int ny = hist->NoBinsY();

    res->ResizeBin((upRangeX - loRangeX) * (upRangeY - loRangeY));

    for (int i = loRangeX; i < upRangeX; ++i)
    for (int j = loRangeY; j < upRangeY; ++j)
    res->AppendBin(hist->GetBin(i * ny + j));

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
  Shrink2dFun<false>(Word* args, Word& result, int message,
      Word& local, Supplier s);

  template int
  Shrink2dFun<true>(Word* args, Word& result, int message,
      Word& local, Supplier s);

  ListExpr Shrink2dTypeMap(ListExpr args)
  {
    NList argList(args);

    if(argList.length() != 5) {
      return listutils::typeError("Expects 5 arguments.");
    }
    NList hist = argList.first();
    NList loX = argList.second();
    NList hiX = argList.third();
    NList loY = argList.fourth();
    NList hiY = argList.fifth();

    if(!hist.isSymbol(Histogram2d::BasicType())) {
      return listutils::typeError("Expects '"+
                         Histogram2d::BasicType()+"' as 1st argument");
    }

    if(!loX.isSymbol(CcReal::BasicType()) ||
      !hiX.isSymbol(CcReal::BasicType())
        || !loY.isSymbol(CcReal::BasicType()) ||
        !hiY.isSymbol(CcReal::BasicType())) {
      return listutils::typeError("Expects 'real' for lower an upper bound");
    }
    return hist.listExpr();
  }

/*
9.7 Getcount2d

*/
  int Getcount2dFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    const Histogram2d* h = static_cast<const Histogram2d*>(args[0].addr);
    const CcInt* binIndexX = static_cast<const CcInt*>(args[1].addr);
    const CcInt* binIndexY = static_cast<const CcInt*>(args[2].addr);
    result = qp->ResultStorage(s);
    CcReal* res = static_cast<CcReal*>(result.addr );

    if (!binIndexX->IsDefined() || !binIndexY->IsDefined())
    {
      res->SetDefined(false);
      return 0;
    }

    CcReal count = h->GetCount(binIndexX->GetValue(), binIndexY->GetValue());
    res->Set(count.IsDefined(), count.GetValue());
    return 0;
  }

  ListExpr Getcount2dTypeMap(ListExpr args)
  {
    NList list(args);
    const string errMsg = "Expecting (" + Histogram2d::BasicType() + " " +
                          CcInt::BasicType() + " " + CcInt::BasicType() + ")";

    if (list.length() != 3)
      return list.typeError(errMsg);

    NList arg1 = list.first();
    NList arg2 = list.second();
    NList arg3 = list.third();

    // histogram2d x int x int -> real
    if (arg1.isSymbol(Histogram2d::BasicType()) &&
      arg2.isSymbol(CcInt::BasicType()) &&
      arg3.isSymbol(CcInt::BasicType()))
      return NList(CcReal::BasicType()).listExpr();

    return list.typeError(errMsg);
  }

/*
9.8 Insert2d

Argument 0 Histogram2d, 1 X real value, 2 Y real value, 3 incVal

*/
  template<bool incValSupplied>
  int Insert2dFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    const Histogram2d* h = (const Histogram2d*)args[0].addr;
    const CcReal* x = (const CcReal*)args[1].addr;
    const CcReal* y = (const CcReal*)args[2].addr;

    // return histogram2d
    result = qp->ResultStorage(s);
    Histogram2d* hist = (Histogram2d*)result.addr;
    hist->Clear();

    if (!h->IsDefined() || !x->IsDefined() || !y->IsDefined())
    {
      hist->SetDefined(false);
      return 0;
    }

    hist->CopyFrom(h);

    CcInt bin = hist->FindBin(x->GetRealval(), y->GetRealval());

    if (bin.IsDefined()) {
      if (incValSupplied) {
        const CcReal* incVal = (const CcReal*)args[3].addr;

        if (!incVal->IsDefined()){
          hist->SetDefined(false);
          return 0;
        }

        hist->Insert(bin.GetIntval(), incVal->GetRealval());
      } else{
        hist->Insert(bin.GetIntval());
      }
    }

    return 0;
  }

  template<bool incValSupplied>
  ListExpr Insert2dTypeMap(ListExpr args)
  {
    NList argList = NList(args);

    if (incValSupplied) {
      if(argList.length() != 4) {
        return listutils::typeError("Expects 4 arguments.");
      }
    } else {
      if(argList.length() != 3) {
        return listutils::typeError("Expects 3 arguments.");
      }
    }

    NList hist = argList.first();
    NList x = argList.second();
    NList y = argList.third();

    if(!hist.isSymbol(Histogram2d::BasicType())) {
      return listutils::typeError("1st argument not of type " +
                                    Histogram2d::BasicType());
    }

    if(!x.isSymbol(CcReal::BasicType())) {
      return listutils::typeError("2nd argument not of type " +
        CcReal::BasicType());
    }

    if(!y.isSymbol(CcReal::BasicType())) {
      return listutils::typeError("3rd argument not of type " +
                                                          CcReal::BasicType());
    }
    if (incValSupplied) {
      NList incVal = argList.fourth();
      if(!incVal.isSymbol(CcReal::BasicType())) {
        return listutils::typeError("4th argument not of type " +
                                                          CcReal::BasicType());
      }
    }

    return NList(Histogram2d::BasicType()).listExpr();
  }

/*
9.10 FindBin2d

Argument 0 Histogram2d, 1 real value

*/
  template<bool X>
  int FindBin2dFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    const Histogram2d* hist = (const Histogram2d*)args[0].addr;
    const CcReal* value = (const CcReal*)args[1].addr;

    CcInt bin;

    result = qp->ResultStorage(s);

    if (value->IsDefined()) {
      if (X)
        bin = hist->FindBinX(value->GetRealval());
      else
        bin = hist->FindBinY(value->GetRealval());
    }
    else
    {
      bin = CcInt(false, 0);
    }

    ((CcInt*)result.addr)->Set(bin.IsDefined(), bin.GetIntval());
    return 0;
  }

/*
9.11 Find[_]minbin, find[_]maxbi

*/
  template<bool isMin>
  int FindMinMaxBinFun2d(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    const Histogram2d* h = static_cast<const Histogram2d*>(args[0].addr);
    MinMaxBuffer* buffer;
    CcInt index;
    HIST_REAL value;
    int i;
    Tuple* resultTuple;
    pair<CcInt, CcInt> coords;

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
        value = h->GetBin(index.GetValue());
      else
        value = -1;
      local.addr = new MinMaxBuffer(index, value, GetResultTupleTypeInfo2d());

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
      resultTuple = new Tuple(buffer->tupleTypeInfo);
      coords = h->GetBinCoords(index.GetValue());
      resultTuple->PutAttribute(0, new CcInt(coords.first)); // x
      resultTuple->PutAttribute(1, new CcInt(coords.second)); // y
      result.addr = resultTuple;

      // Find the next bin with min./max. value:
      while (++i < h->GetNoBin() && !AlmostEqual(h->GetBin(i), value))
        ;

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
/*
9.12 MeanX, MeanY

*/
  int MeanXFun(Word* args, Word& result, int message, Word& local, Supplier s)
  {
    const Histogram2d* h = static_cast<const Histogram2d*>(args[0].addr );

    result = qp->ResultStorage(s);

    CcReal* res = static_cast<CcReal*>(result.addr );
    CcReal mean = h->MeanX();
    res->Set(mean.IsDefined(), mean.GetValue());

    return 0;
  }

  ListExpr Hist2dRealTypeMap(ListExpr args)
  {
    NList list(args);
    const string errMsg = "Expecting " + Histogram2d::BasicType();

    if (list.length() != 1)
      return list.typeError(errMsg);

    NList arg1 = list.first();

    // histogram2d -> real
    if (arg1.isSymbol(Histogram2d::BasicType()))
      return NList(CcReal::BasicType()).listExpr();

    return list.typeError(errMsg);
  }

  int MeanYFun(Word* args, Word& result, int message, Word& local, Supplier s)
   {
     const Histogram2d* h = static_cast<const Histogram2d*>(args[0].addr );

     result = qp->ResultStorage(s);

     CcReal* res = static_cast<CcReal*>(result.addr );
     CcReal mean = h->MeanY();
     res->Set(mean.IsDefined(), mean.GetValue());

     return 0;
   }


/*
9.13 Class TupleAndRelPos

*/
//   class TupleAndRelPos
//   {
//   public:
//
//     TupleAndRelPos() :
//     tuple(0),
//     pos(0),
//     cmpPtr(0)
//     {};
//
//     TupleAndRelPos(Tuple* newTuple, TupleCompareBy* cmpObjPtr = 0,
//         int newPos = 0) :
//     tuple(newTuple),
//     pos(newPos),
//     cmpPtr(cmpObjPtr)
//     {};
//
//     inline bool operator<(const TupleAndRelPos& ref) const
//     {
//       // by default < is used to define a sort order
//       // the priority queue creates a maximum heap, hence
//       // we change the result to create a minimum queue.
//       // It would be nice to have also an < operator in the class
//       // Tuple. Moreover lexicographical comparison should be done by means
//       // of TupleCompareBy and an appropriate sort order specification,
//
//       if (!this->tuple || !ref.tuple)
//       {
//         return true;
//       }
//       if ( cmpPtr )
//       {
//         return !(*(TupleCompareBy*)cmpPtr)( this->tuple, ref.tuple );
//       }
//       else
//       {
//         return !lexSmaller( this->tuple, ref.tuple );
//       }
//     }
//
//     Tuple* tuple;
//     int pos;
//
//   private:
//     void* cmpPtr;
//
//   };

/*
9.14 Class SortStream

*/
  class SortStream
  {
  public:
    SortStream( Word stream,
        const bool lexicographic,
        void *tupleSmaller ):
    stream( stream ),
    currentIndex( 0 ),
    lexiTupleSmaller( lexicographic ?
        (LexicographicalTupleSmaller*)tupleSmaller :
        0 ),
    tupleCmpBy( lexicographic ? 0 : (TupleCompareBy*)tupleSmaller ),
    lexicographic( lexicographic ),
    count( 0 )
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

      MAX_MEMORY = qp->FixedMemory();
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
          nextTuple.ref->IncReference();
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
            nextTuple.ref->IncReference();
            currentRun->push(nextTuple);
            minTuple = currentRun->top();
            minTuple.ref->DeleteIfAllowed();
            rel->AppendTuple( minTuple.ref );
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
              nextTuple.ref->IncReference();
              currentRun->push(nextTuple);
              minTuple = currentRun->top();
              minTuple.ref->DeleteIfAllowed();
              rel->AppendTuple( minTuple.ref );
              lastTuple = minTuple;
              currentRun->pop();
              m++;
            }
            else
            { // nextTuple is smaller, save it for the next relation
              nextTuple.ref->IncReference();
              nextRun->push(nextTuple);
              n++;
              if ( !currentRun->empty() )
              {
                // Append the minimum to the current relation
                minTuple = currentRun->top();
                minTuple.ref->DeleteIfAllowed();
                rel->AppendTuple( minTuple.ref );
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
            if ( copyOfLast.ref && (copyOfLast.ref != minTuple.ref) )
            {
              copyOfLast.ref->DeleteIfAllowed();
            }

          } // check if nextTuple can be saved in current relation
        }// memory is completely used

        qp->Request(stream.addr, wTuple);
      }
      ShowPartitionInfo(count,a,n,m,r,rel);

      // delete lastTuple and minTuple if allowed
      if ( lastTuple.ref )
      {
        lastTuple.ref->DeleteIfAllowed();
      }
      if ( (minTuple.ref != lastTuple.ref) )
      {
        minTuple.ref->DeleteIfAllowed();
      }

      // the lastRun and NextRun runs in memory having
      // less than MAX_TUPLE elements
      if( !queue[0].empty() )
      {
        Tuple* t = queue[0].top().ref;
        queue[0].pop();
        mergeTuples.push( TupleAndRelPos(t, tupleCmpBy, -2) );
      }
      if( !queue[1].empty() )
      {
        Tuple* t = queue[1].top().ref;
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

    ~SortStream()
    {
      while( !mergeTuples.empty() )
      {
        mergeTuples.top().ref->DeleteIfAllowed();
        mergeTuples.pop();
      }

      for( int i = 0; i < 2; i++ )
      {
        while( !queue[i].empty() )
        {
          queue[i].top().ref->DeleteIfAllowed();
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

      delete lexiTupleSmaller;
      lexiTupleSmaller = 0;
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
        //p.ref->DecReference();
        mergeTuples.pop();
        Tuple *result = p.ref;
        tuples.push(p);
        Tuple *t = 0;

        if (p.pos > 0)
        t = relations[p.pos-1].second->GetNextTuple();
        else
        {
          int idx = p.pos+2;
          if ( !queue[idx].empty() )
          {
            t = queue[idx].top().ref;
            t->DeleteIfAllowed();
            queue[idx].pop();
          }
          else
          t = 0;
        }

        if( t != 0 )
        { // run not finished
          p.ref = t;
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

    priority_queue<TupleAndRelPos> GetTuples()
    {
      return tuples;
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
    LexicographicalTupleSmaller *lexiTupleSmaller;
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
    TupleQueue tuples;
  };

/*
9.15 Class SortQueue

*/
  class SortQueue
  {
  public:
    SortQueue( priority_queue<TupleAndRelPos> stream,
        const bool lexicographic,
        void *tupleSmaller ):
    stream( stream ),
    currentIndex( 0 ),
    lexiTupleSmaller( lexicographic ?
        (LexicographicalTupleSmaller*)tupleSmaller :
        0 ),
    tupleCmpBy( lexicographic ? 0 : (TupleCompareBy*)tupleSmaller ),
    lexicographic( lexicographic ),
    count( 0 )
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

      //Word wTuple = SetWord(Address(0));

      size_t i = 0, a = 0, n = 0, m = 0, r = 0; // counter variables
      bool newRelation = true;

      MAX_MEMORY = qp->FixedMemory();
      cmsg.info("ERA:ShowMemInfo")
      << "Sortby.MAX_MEMORY (" << MAX_MEMORY/1024 << " kb)" << endl;
      cmsg.send();

      TupleBuffer *rel=0;
      TupleAndRelPos lastTuple(0, tupleCmpBy);

      //qp->Request(stream.addr, wTuple);
      TupleAndRelPos currentTuple;

      TupleAndRelPos minTuple(0, tupleCmpBy);
      while(!stream.empty()) // consume the stream completely

      {
        currentTuple = stream.top();
        currentTuple.ref->DeleteIfAllowed();
        stream.pop();
        count++; // tuple counter;
        Tuple* t = static_cast<Tuple*>( currentTuple.ref );
        TupleAndRelPos nextTuple(t, tupleCmpBy);
        if( MAX_MEMORY > (size_t)t->GetSize() )
        {
          nextTuple.ref->IncReference();
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
            nextTuple.ref->IncReference();
            currentRun->push(nextTuple);
            minTuple = currentRun->top();
            minTuple.ref->DeleteIfAllowed();
            rel->AppendTuple( minTuple.ref );
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
              nextTuple.ref->IncReference();
              currentRun->push(nextTuple);
              minTuple = currentRun->top();
              minTuple.ref->DeleteIfAllowed();
              rel->AppendTuple( minTuple.ref );
              lastTuple = minTuple;
              currentRun->pop();
              m++;
            }
            else
            { // nextTuple is smaller, save it for the next relation
              nextTuple.ref->IncReference();
              nextRun->push(nextTuple);
              n++;
              if ( !currentRun->empty() )
              {
                // Append the minimum to the current relation
                minTuple = currentRun->top();
                minTuple.ref->DeleteIfAllowed();
                rel->AppendTuple( minTuple.ref );
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
            if ( copyOfLast.ref && (copyOfLast.ref != minTuple.ref) )
            {
              copyOfLast.ref->DeleteIfAllowed();
            }

          } // check if nextTuple can be saved in current relation
        }// memory is completely used

        //qp->Request(stream.addr, wTuple);
      }
      ShowPartitionInfo(count,a,n,m,r,rel);

      // delete lastTuple and minTuple if allowed
      if ( lastTuple.ref )
      {
        lastTuple.ref->DeleteIfAllowed();
      }
      if ( (minTuple.ref != lastTuple.ref) )
      {
        minTuple.ref->DeleteIfAllowed();
      }

      // the lastRun and NextRun runs in memory having
      // less than MAX_TUPLE elements
      if( !queue[0].empty() )
      {
        Tuple* t = queue[0].top().ref;
        queue[0].pop();
        mergeTuples.push( TupleAndRelPos(t, tupleCmpBy, -2) );
      }
      if( !queue[1].empty() )
      {
        Tuple* t = queue[1].top().ref;
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

    ~SortQueue()
    {
      while( !mergeTuples.empty() )
      {
        mergeTuples.top().ref->DeleteIfAllowed();
        mergeTuples.pop();
      }

      for( int i = 0; i < 2; i++ )
      {
        while( !queue[i].empty() )
        {
          queue[i].top().ref->DeleteIfAllowed();
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

      delete lexiTupleSmaller;
      lexiTupleSmaller = 0;
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
        //p.ref->DecReference();
        mergeTuples.pop();
        tuples.push(p);
        Tuple *result = p.ref;
        Tuple *t = 0;

        if (p.pos > 0)
        t = relations[p.pos-1].second->GetNextTuple();
        else
        {
          int idx = p.pos+2;
          if ( !queue[idx].empty() )
          {
            t = queue[idx].top().ref;
            t->DeleteIfAllowed();
            queue[idx].pop();
          }
          else
          t = 0;
        }

        if( t != 0 )
        { // run not finished
          p.ref = t;
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

    priority_queue<TupleAndRelPos> GetTuples()
    {
      return tuples;
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

    priority_queue<TupleAndRelPos> stream;
    size_t currentIndex;

    // tuple information
    LexicographicalTupleSmaller *lexiTupleSmaller;
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
    TupleQueue tuples;
  };

/*
9.16 CreateHistogram2dEquicount

Argument 0 tuple stream, 1 attribute name X, 2 attribute name Y,
3 maxCategories X, 4 maxCategories Y, 5 index of attribute X,
6 Index of attribute Y

*/
  int CreateHistogram2dEquicountFun(Word* args, Word& result, int message,
      Word& local, Supplier s)
  {

    Tuple* currentTuple;
    // The query processor provided an empty Histogram1d-instance:
    result = qp->ResultStorage(s);
    Histogram2d* hist = (Histogram2d*) result.addr;
    hist->Clear();
    const CcInt* indexX = (const CcInt*)args[5].addr;
    const CcInt* indexY = (const CcInt*)args[6].addr;
    const CcInt* maxCategoriesX = (const CcInt*)args[3].addr;
    const CcInt* maxCategoriesY = (const CcInt*)args[4].addr;

    qp->Open(args[0].addr);

    // number of bins must be a positive integer value,
    // else the result is undefined
    if (maxCategoriesX->IsDefined() && maxCategoriesY->IsDefined()
        && maxCategoriesX->GetIntval() > 0 && maxCategoriesY->GetIntval() > 0) {

      // First sort values
      SortOrderSpecification spec;
      spec.push_back(pair<int, bool>(indexX->GetIntval(), true));
      void *tupleCmp = new TupleCompareBy(spec);

      SortStream* sli;
      sli = new SortStream( args[0],
          false,
          tupleCmp);

      int noTuples = sli->TupleCount();
      int noBins = noTuples / maxCategoriesX->GetIntval();
      int rest = noTuples % maxCategoriesX->GetIntval();

      // maxCategories > tupleCount
      if (noBins == 0) {
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

      while ((currentTuple = sli->NextResultTuple()) != 0) {
        tupleCount++;
        CcReal* val =
        (CcReal*)currentTuple->GetAttribute(indexX->GetIntval()-1);

        if (!val->IsDefined())
        continue;

        lastvalue = value;
        value = val->GetRealval();

        //currentTuple->DecReference();
        //currentTuple->DeleteIfAllowed();

        // reached next bin?
        // If we have got a remainder r, the count of the first r bins is
        // = noBins + 1
        if ((currentBin >= rest && count % noBins == 0) || (currentBin < rest
                && count % (noBins + 1) == 0)) {
          // current value not yet greater than precedent value
          if (CmpReal(value, lastvalue) != 1) {
            //hist->Insert(currentBin);
            continue;
          }
          // if only a few values are left, merge the last bins
          else if ((count + noTuples - tupleCount) < noBins * 1.1) {
            //hist->Insert(currentBin);
            continue;
          } else {
            hist->AppendRangeX(value);
            //hist->AppendBin(0.0);
            currentBin++;
            count = 0;
          }
        } // (count % noBins == 0)

        //hist->Insert(currentBin);
        count++;
      }

      // add the last range
      // Given that FACTOR does not cause value to be $<$ lastrange, here
      // the less elegant way will do.
      HIST_REAL lastrange = value+FACTOR;
      HIST_REAL fact = FACTOR;
      int max = 0;
      while (max < 10 && !(value<lastrange)) {
        fact *= 10;
        lastrange = value+fact;
        max++;
      }
      hist->AppendRangeX(lastrange);

      // Now applying the same to the Y values

      // First sort values
      //SortOrderSpecification spec;
      spec.clear();
      spec.push_back(pair<int, bool>(indexY->GetIntval(), true));
      tupleCmp = new TupleCompareBy(spec);

      //SortByLocalInfo* sli;
      SortQueue* sq = new SortQueue( sli->GetTuples(),
          false,
          tupleCmp);

      noTuples = sq->TupleCount();
      noBins = noTuples / maxCategoriesY->GetIntval();
      rest = noTuples % maxCategoriesY->GetIntval();

      // maxCategories > tupleCount
      if (noBins == 0) {
        noBins = 1;
        rest = 0;
      }

      if (noTuples == 0)
      hist->SetDefined(false);

      count = 0;
      value = -numeric_limits<HIST_REAL>::max();
      currentBin = -1;
      tupleCount = 0;

      while ((currentTuple = sq->NextResultTuple()) != 0)
      {
        tupleCount++;
        CcReal* val =
        (CcReal*)currentTuple->GetAttribute(indexY->GetIntval()-1);

        if (!val->IsDefined())
        continue;

        lastvalue = value;
        value = val->GetRealval();

        //currentTuple->DecReference();
        //currentTuple->DeleteIfAllowed();

        // next bin reached?
        // If we have got a remainder r, the count of the first r bins is
        // = noBins+1
        if ((currentBin >= rest && count % noBins == 0) || (currentBin < rest
                && count % (noBins + 1) == 0))
        { // current value not yet greater than precedent value
          if (CmpReal(value, lastvalue) != 1)
          { //hist->Insert(currentBin);
            continue;
          } // If only a few values are left, merge the last bins.
          else if ((count + noTuples - tupleCount) < noBins * 1.1) {
            //hist->Insert(currentBin);
            continue;
          } else  {
            hist->AppendRangeY(value);
            //hist->AppendBin(0.0);
            currentBin++;
            count = 0;
          }
        } // (count % noBins == 0)

        //hist->Insert(currentBin);
        count++;
      }

      // add the last range
      // Given that FACTOR does not cause value to be $<$ lastrange, here
      // less elegant way will do.
      lastrange = value+FACTOR;
      fact = FACTOR;
      max = 0;
      while (max < 10 && !(value<lastrange)) {
        fact *= 10;
        lastrange = value+fact;
        max++;
      }
      hist->AppendRangeY(lastrange);

      int noBin = (hist->GetNoRangeX()-1)*(hist->GetNoRangeY()-1);

      for (int i=0; i < noBin; ++i) {
        hist->AppendBin(0.0);
      }

      priority_queue<TupleAndRelPos> tuples = sq->GetTuples();
      TupleAndRelPos tuplePos;
      Tuple* t;
      CcReal* x;
      CcReal* y;

      while (!tuples.empty()) {
        tuplePos = tuples.top();
        tuples.pop();
        t = tuplePos.ref;
        x = (CcReal*)t->GetAttribute(indexX->GetIntval()-1);
        y = (CcReal*)t->GetAttribute(indexY->GetIntval()-1);
        hist->Insert((HIST_REAL)x->GetRealval(), (HIST_REAL)y->GetRealval());
        t->DeleteIfAllowed();
      }

      delete sli;
      delete sq;
    } else {
      Word elem;
      qp->Request(args[0].addr, elem);
      while(qp->Received(args[0].addr)) {
        ((Tuple*)elem.addr)->DeleteIfAllowed();
        qp->Request(args[0].addr, elem);
      }
      hist->SetDefined(false);
    }

    qp->Close(args[0].addr);
    return 0;
  }

  ListExpr CreateHistogram2dEquicountTypeMap(ListExpr args)
  {

    if(nl->ListLength(args)!=5){
      return listutils::typeError("5 Arguments expected");
    }

    ListExpr stream = nl->First(args);
    ListExpr attrX = nl->Second(args);
    ListExpr attrY = nl->Third(args);
    ListExpr maxCatX = nl->Fourth(args);
    ListExpr maxCatY = nl->Fifth(args);

    if(!listutils::isTupleStream(stream)){
      return listutils::typeError("first argument must be a tuplestream");
    }


    if(!listutils::isSymbol(attrX)){
      return listutils::typeError("second argument is not a "
                                  "valid attribute name");
    }
    if(!listutils::isSymbol(attrY)){
      return listutils::typeError("third argument is not a "
                                  "valid attribute name");
    }

    if(!listutils::isSymbol(maxCatX, CcInt::BasicType())){
      return listutils::typeError("fourth argument mut be of type int");
    }

    if(!listutils::isSymbol(maxCatY, CcInt::BasicType())){
      return listutils::typeError("fifth argument mut be of type int");
    }

    ListExpr attrList = nl->Second(nl->Second(stream));

    ListExpr attrTypeX;
    string attrXname = nl->SymbolValue(attrX);
    int indexX = listutils::findAttribute(attrList, attrXname , attrTypeX);
    if(indexX==0){
      return listutils::typeError(attrXname + " is not an attribute "
                                  "of the stream");
    }
    if(!listutils::isSymbol(attrTypeX,CcReal::BasicType())){
      return listutils::typeError("attribute " + attrXname +
                                  " is not of type real");
    }


    ListExpr attrTypeY;
    string attrYname = nl->SymbolValue(attrY);
    int indexY = listutils::findAttribute(attrList, attrYname , attrTypeY);
    if(indexY==0){
      return listutils::typeError(attrYname + " is not an attribute "
                                  "of the stream");
    }
    if(!listutils::isSymbol(attrTypeY,CcReal::BasicType())){
      return listutils::typeError("attribute " + attrYname +
                                  " is not of type real");
    }

    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                             nl->TwoElemList(nl->IntAtom(indexX),
                                             nl->IntAtom(indexY)),
                             nl->SymbolAtom(Histogram2d::BasicType()));

  } // CreateHistogram2dEquicountTypeMap(ListExpr args)

/*
9.17 VarianceX, VarianceY, Covariance

*/

  int VarianceXFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    const Histogram2d* h = static_cast<const Histogram2d*>(args[0].addr );

    result = qp->ResultStorage(s);

    CcReal* res = static_cast<CcReal*>(result.addr );
    CcReal var = h->VarianceX();
    res->Set(var.IsDefined(), var.GetValue());

    return 0;
  }

  int VarianceYFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    const Histogram2d* h = static_cast<const Histogram2d*>(args[0].addr );

    result = qp->ResultStorage(s);

    CcReal* res = static_cast<CcReal*>(result.addr );
    CcReal var = h->VarianceY();
    res->Set(var.IsDefined(), var.GetValue());

    return 0;
  }

  int CovarianceFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {
    const Histogram2d* h = static_cast<const Histogram2d*>(args[0].addr );

    result = qp->ResultStorage(s);

    CcReal* res = static_cast<CcReal*>(result.addr );
    CcReal covar = h->Covariance();
    res->Set(covar.IsDefined(), covar.GetValue());

    return 0;
  }

/*
9.18 Instantiation of Template Functions

The compiler cannot expand these template functions in
the file ~HistogramAlgebra.cpp~.

*/

  template int
  Insert2dFun<false>(Word* args, Word& result, int message,
      Word& local, Supplier s);

  template int
  Insert2dFun<true>(Word* args, Word& result, int message,
      Word& local, Supplier s);

  template ListExpr
  Insert2dTypeMap<true>(ListExpr args);

  template ListExpr
  Insert2dTypeMap<false>(ListExpr args);


  template int
  FindBin2dFun<true>(Word* args, Word& result, int message,
                Word& local, Supplier s);

  template int
  FindBin2dFun<false>(Word* args, Word& result, int message,
               Word& local, Supplier s);

  template int
  FindMinMaxBinFun2d<true>(Word* args, Word& result, int message,
        Word& local, Supplier s);

  template int
  FindMinMaxBinFun2d<false>(Word* args, Word& result, int message,
        Word& local, Supplier s);

} // namespace hgr
