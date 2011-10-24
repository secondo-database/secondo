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

[1] Header File of the Histogram Algebra

December 2007, S.H[oe]cher,M.H[oe]ger,A.Belz,B.Poneleit


[TOC]

1 Overview

2 Defines and includes

*/
#include "Histogram2d.h"
#include "Histogram1d.h"
#include "Histogram.h"
#include "Symbols.h"

namespace hgr
{

  extern TypeConstructor histogram1dTC;
  extern TypeConstructor histogram2dTC;

  HistogramAlgebra::HistogramAlgebra() :
    Algebra()
  {

    AddTypeConstructor( &histogram1dTC);
    AddTypeConstructor( &histogram2dTC);

    histogram1dTC.AssociateKind(Kind::DATA());
    histogram2dTC.AssociateKind(Kind::DATA());


  AddOperator(SetHistogram1dInfo(), SetHistogram1dFun, SetHistogram1dTypeMap);

  AddOperator(SetHistogram2dInfo(), SetHistogram2dFun, SetHistogram2dTypeMap);

  AddOperator(CreateHistogram1dInfo(), CreateHistogram1dFun,
      CreateHistogram1dTypeMap);
  AddOperator(CreateHistogram2dInfo(), CreateHistogram2dFun,
      CreateHistogram2dTypeMap);

  AddOperator(CreateHistogram1dEquiwidthInfo(),
      CreateHistogram1dEquiwidthFun, CreateHistogram1dEquiwidthTypeMap);

  AddOperator(CreateHistogram2dEquiwidthInfo(),
    CreateHistogram2dEquiwidthFun, CreateHistogram2dEquiwidthTypeMap);

  Operator* create_histogram1d_equicount =
  AddOperator(CreateHistogram1dEquicountInfo(),
      CreateHistogram1dEquicountFun, CreateHistogram1dEquicountTypeMap);
    create_histogram1d_equicount->SetUsesMemory();




  AddOperator(CreateHistogram2dEquicountInfo(),
      CreateHistogram2dEquicountFun, CreateHistogram2dEquicountTypeMap);

  AddOperator(NoComponentsInfo(), NoComponentsFun, NoComponentsTypeMap);

  AddOperator(BinsXInfo(), BinsXFun, BinsXYTypeMap);
  AddOperator(BinsYInfo(), BinsYFun, BinsXYTypeMap);

  AddOperator(binrange_minInfo(), binrange_minFun, binrange_min_maxTypeMap);
  AddOperator(binrange_maxInfo(), binrange_maxFun, binrange_min_maxTypeMap);
  AddOperator(binrange_minXInfo(), binrange_minXFun,
      binrange_minXY_maxXYTypeMap);
  AddOperator(binrange_maxXInfo(), binrange_maxXFun,
      binrange_minXY_maxXYTypeMap);
  AddOperator(binrange_minYInfo(), binrange_minYFun,
      binrange_minXY_maxXYTypeMap);
  AddOperator(binrange_maxYInfo(), binrange_maxYFun,
      binrange_minXY_maxXYTypeMap);

  AddOperator(Getcount1dInfo(), Getcount1dFun, Getcount1dTypeMap);
  AddOperator(Getcount2dInfo(), Getcount2dFun, Getcount2dTypeMap);

  AddOperator(FindBinInfo(), FindBinFun, FindBinTypeMap<true>);
  AddOperator(FindBinXInfo(), FindBin2dFun<true>, FindBinTypeMap<false>);
  AddOperator(FindBinYInfo(), FindBin2dFun<false>, FindBinTypeMap<false>);

  AddOperator(IsRefinementInfo(), IsRefinementFun, HistHistBoolTypeMap);

  AddOperator(IsLessInfo(), IsLessFun, HistHistBoolTypeMap);
  AddOperator(IsEqualInfo(), IsEqualFun, HistHistBoolTypeMap);

  ValueMapping FindMinBinFuns[] = { FindMinMaxBinFun1d<true>,
                                    FindMinMaxBinFun2d<true>, 0 };
  AddOperator(FindMinBinInfo(), FindMinBinFuns,
      FindMinMaxBinSelect, FindMinMaxBinTypeMap);

  ValueMapping FindMaxBinFuns[] = { FindMinMaxBinFun1d<false>,
                                    FindMinMaxBinFun2d<false>, 0 };
  AddOperator(FindMaxBinInfo(), FindMaxBinFuns,
      FindMinMaxBinSelect, FindMinMaxBinTypeMap);

  AddOperator(MeanInfo(), MeanFun, Hist1dRealTypeMap);
  AddOperator(MeanXInfo(), MeanXFun, Hist2dRealTypeMap);
  AddOperator(MeanYInfo(), MeanYFun, Hist2dRealTypeMap);

  AddOperator(VarianceInfo(), VarianceFun, Hist1dRealTypeMap);
  AddOperator(VarianceXInfo(), VarianceXFun, Hist2dRealTypeMap);
  AddOperator(VarianceYInfo(), VarianceYFun, Hist2dRealTypeMap);

  AddOperator(CovarianceInfo(), CovarianceFun, Hist2dRealTypeMap);

  AddOperator(DistanceInfo(), DistanceFun, DistanceTypeMap);

  AddOperator(TranslateInfo(), TranslateFun, TranslateTypeMap);

  AddOperator(UseInfo(), UseFun, UseTypeMap);
  AddOperator(Use2Info(), Use2Fun, Use2TypeMap);

  AddOperator(FoldInfo(), FoldFun, FoldTypeMap);

  AddOperator(ShrinkEagerInfo(), Shrink1dFun<true>, Shrink1dTypeMap);
  AddOperator(ShrinkLazyInfo(), Shrink1dFun<false>, Shrink1dTypeMap);
  AddOperator(ShrinkEager2Info(), Shrink2dFun<true>, Shrink2dTypeMap);
  AddOperator(ShrinkLazy2Info(), Shrink2dFun<false>, Shrink2dTypeMap);

  AddOperator(Insert1dValueInfo(),
      Insert1dFun<true>, Insert1dTypeMap<true>);
  AddOperator(Insert1dInfo(),
      Insert1dFun<false>, Insert1dTypeMap<false>);

  AddOperator(Insert2dValueInfo(),
      Insert2dFun<true>, Insert2dTypeMap<true>);
  AddOperator(Insert2dInfo(),
      Insert2dFun<false>, Insert2dTypeMap<false>);
}
  HistogramAlgebra::~HistogramAlgebra()
  {
  }
} // namespace Histogram

extern "C"
Algebra* InitializeHistogramAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  // The C++ scope-operator :: must be used to qualify the full name
  return new hgr::HistogramAlgebra();
}

