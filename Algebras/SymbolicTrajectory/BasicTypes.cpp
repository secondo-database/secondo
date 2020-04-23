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
#include "BasicTypes.h"
#include "Tools.h"

using namespace std;

namespace stj {

/*
\section{Functions for HybridDistanceParameters}

*/
bool HybridDistanceParameters::isCorrectType(std::string& name, ListExpr type) {
  transform(name.begin(), name.end(), name.begin(), ::tolower);
  if (name == "labelfun")    return CcInt::checkType(type);
  if (name == "distfun")     return CcInt::checkType(type);
  if (name == "geodistfun")  return CcInt::checkType(type);
  if (name == "threshold")   return CcReal::checkType(type);
  if (name == "scalefactor") return CcReal::checkType(type);
  if (name == "geoid")       return Geoid::checkType(type);
  return false;
}

TupleType* HybridDistanceParameters::getTupleType() {
  if (tt) {
    return tt;
  }
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  ListExpr ttlist = nl->TwoElemList(
    nl->SymbolAtom(Tuple::BasicType()),
                   nl->FiveElemList(
                      nl->TwoElemList(nl->SymbolAtom("Name"),
                                      nl->SymbolAtom(CcString::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("InputType"),
                                      nl->SymbolAtom(CcString::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("DefaultValue"),
                                      nl->SymbolAtom(CcString::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("CurrentValue"),
                                      nl->SymbolAtom(CcString::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Description"),
                                      nl->SymbolAtom(FText::BasicType()))));
  ListExpr numttlist = sc->NumericType(ttlist);
  tt = new TupleType(numttlist);
  return tt;
}

Tuple* HybridDistanceParameters::getNextTuple() {
  if (memberNo > 5) {
    return 0;
  }
  Tuple *tuple = new Tuple(getTupleType());
  tuple->PutAttribute(0, getName(memberNo));
  tuple->PutAttribute(1, getType(memberNo));
  tuple->PutAttribute(2, getDefault(memberNo));
  tuple->PutAttribute(3, getValue(memberNo));
  tuple->PutAttribute(4, getDescription(memberNo));
  memberNo++;
  return tuple;
}

CcString* HybridDistanceParameters::getName(unsigned int memberNo) {
  CcString *result = new CcString(false);
  switch (memberNo) {
    case 0: {
      result->Set(true, "labelFun");
      break;
    }
    case 1: {
      result->Set(true, "distFun");
      break;
    }
    case 2: {
      result->Set(true, "geoDistFun");
      break;
    }
    case 3: {
      result->Set(true, "threshold");
      break;
    }
    case 4: {
      result->Set(true, "scaleFactor");
      break;
    }
    case 5: {
      result->Set(true, "geoid");
      break;
    }
    default: {
      return result;
    }
  }
  return result;
}

CcString* HybridDistanceParameters::getType(unsigned int memberNo) {
  CcString *result = new CcString(false);
  switch (memberNo) {
    case 0:
    case 1:
    case 2: {
      result->Set(true, CcInt::BasicType());
      break;
    }
    case 3:
    case 4: {
      result->Set(true, CcReal::BasicType());
      break;
    }
    case 5: {
      result->Set(true, Geoid::BasicType());
      break;
    }
    default: {
      return result;
    }
  }
  return result;
}

CcString* HybridDistanceParameters::getDefault(unsigned int memberNo) {
  CcString *result = new CcString(false);
  stringstream valuestr;
  switch (memberNo) {
    case 0: {
      valuestr << getDefaultLabelFun();
      break;
    }
    case 1: {
      valuestr << getDefaultDistFun();
      break;
    }
    case 2: {
      valuestr << getDefaultGeoDistFun();
      break;
    }
    case 3: {
      valuestr << getDefaultThreshold();
      break;
    }
    case 4: {
      valuestr << getDefaultScaleFactor();
      break;
    }
    case 5: {
      if (getDefaultGeoid() == 0) {
        valuestr << "null" << endl;
      }
      else {
        valuestr << getDefaultGeoid()->getName();
      }
      break;
    }
    default: {
      return result;
    }
  }
  result->Set(true, valuestr.str());
  return result;
}

CcString* HybridDistanceParameters::getValue(unsigned int memberNo) {
  CcString *result = new CcString(false);
  stringstream valuestr;
  switch (memberNo) {
    case 0: {
      valuestr << labelFun;
      break;
    }
    case 1: {
      valuestr << distFun;
      break;
    }
    case 2: {
      valuestr << geoDistFun;
      break;
    }
    case 3: {
      valuestr << threshold;
      break;
    }
    case 4: {
      valuestr << scaleFactor;
      break;
    }
    case 5: {
      if (geoid == 0) {
        valuestr << "null";
      }
      else {
        valuestr << geoid->getName();
      }
      break;
    }
    default: {
      return result;
    }
  }
  result->Set(true, valuestr.str());
  return result;
}

FText* HybridDistanceParameters::getDescription(unsigned int memberNo) {
  FText *result = new FText(false);
  switch (memberNo) {
    case 0: {
      result->Set(true, "Describes the function that compares two label values "
                        "x and y. 0 means: dist(x,y) = (x == y ? 0 : 1).   1 "
                        "means that the Levenshtein/edit distance is applied.");
      break;
    }
    case 1: {
      result->Set(true, "Describes the function that compares two mlabel values"
                        "m and n. 0 means that the Levenshtein/edit distance is"
                        " applied, where a unit is considered as a character.");
      break;
    }
    case 2: {
      result->Set(true, "Describes the function that compares two mpoint values"
                        ". A value of 3 invokes the Fréchet distance, 2 only "
                        "considers the start and end point.");
      break;
    }
    case 3: {
      result->Set(true, "If the symbolic distance exceeds this value, it is "
                        "returned as result. Otherwise, the Fréchet distance "
                        "is computed and returned.");
      break;
    }
    case 4: {
      result->Set(true, "If the Fréchet distance is computed, it is divided by"
                        " this value, in order to make it comparable to the "
                        "symbolic distance that is always in [0,1].");
      break;
    }
    case 5: {
      result->Set(true, "This parameter enables a certain projection, e.g., "
                        "WGS1984, for computing the Fréchet distance.");
      break;
    }
    default: {
      result->SetDefined(false);
    }
  }
  return result;
}

bool HybridDistanceParameters::setLabelFun(const int value) {
  if (value < 0 || value >= 2) {
    cout << "value must be either 0 or 1." << endl;
    return false;
  }
  labelFun = (LabelFunction)value;
  return true;
}

bool HybridDistanceParameters::setDistFun(const int value) {
  if (value < 0 || value >= 7) {
    cout << "value must be between 0 and 6." << endl;
    return false;
  }
  distFun = (DistanceFunction)value;
  return true;
}

bool HybridDistanceParameters::setGeoDistFun(const int value) {
  if (value < 0 || value >= 4) {
    cout << "value must be between 0 and 4." << endl;
    return false;
  }
  geoDistFun = (GeoDistanceFunction)value;
  return true;
}

bool HybridDistanceParameters::setThreshold(const double value) {
  if (value < 0 || value > 1) {
    cout << "value must be between 0 and 1." << endl;
    return false;
  }
  threshold = value;
  return true;
}

bool HybridDistanceParameters::setScaleFactor(const double value) {
  if (value <= 0) {
    cout << "value must be positive." << endl;
    return false;
  }
  scaleFactor = value;
  return true;
}

bool HybridDistanceParameters::setGeoid(Geoid *value) {
  geoid = value->Clone();
  return true;
}
 
}
