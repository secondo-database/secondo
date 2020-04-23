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

Started July 2014, Fabio Vald\'{e}s

*/

#ifndef SYMB_BASICTYPES_H
#define SYMB_BASICTYPES_H

#include "Algebras/SymbolicTrajectoryBasic/SymbolicTrajectoryBasicAlgebra.h"
#include "Algebras/TemporalExt/TemporalExtAlgebra.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "NestedList.h"
#include "ListUtils.h"
// #include "Tools.h"
#include <string>
#include <algorithm>
#include <cctype>

namespace stj {

enum GeoDistanceFunction {GEO_FIRST, GEO_LAST, GEO_FIRST_LAST, FRECHET};

struct HybridDistanceParameters {
  HybridDistanceParameters() {
    labelFun = getDefaultLabelFun();
    distFun = getDefaultDistFun();
    geoDistFun = getDefaultGeoDistFun();
    threshold = getDefaultThreshold();
    scaleFactor = getDefaultScaleFactor();
    geoid = getDefaultGeoid();
    memberNo = 0;
    tt = 0;
  }
  
  ~HybridDistanceParameters() {
    if (tt) {
      tt->DeleteIfAllowed();
    }
    if (geoid) {
      geoid->DeleteIfAllowed();
    }
  }
  
  static bool isCorrectType(std::string& name, ListExpr type);
  TupleType* getTupleType();
  void storeTuples();
  Tuple* getNextTuple();
  CcString* getName(unsigned int memberNo);
  CcString* getType(unsigned int memberNo);
  CcString* getDefault(unsigned int memberNo);
  CcString* getValue(unsigned int memberNo);
  FText* getDescription(unsigned int memberNo);
  LabelFunction getDefaultLabelFun() {return TRIVIAL;}
  DistanceFunction getDefaultDistFun() {return FIRST_LAST;}
  GeoDistanceFunction getDefaultGeoDistFun() {return GEO_FIRST_LAST;}
  double getDefaultThreshold() {return 0.7;}
  double getDefaultScaleFactor() {return 500000.0;}
  Geoid* getDefaultGeoid() {return new Geoid("WGS1984");}
  bool setLabelFun(const int value);
  bool setDistFun(const int value);
  bool setGeoDistFun(const int value);
  bool setThreshold(const double value);
  bool setScaleFactor(const double value);
  bool setGeoid(Geoid *value);

  LabelFunction labelFun; // function used for comparing two label values
  DistanceFunction distFun; // function used for comparing two mlabel values
  GeoDistanceFunction geoDistFun; // function used for comparing two mpoints
  double threshold;// Fréchet dist is computed if symb dist is below this value
  double scaleFactor; // Fréchet dist is divided by this value
  Geoid *geoid; // required for correct Fréchet dist computation, e.g., WGS1984
  
  unsigned int memberNo;
  TupleType *tt;
};

}

#endif
