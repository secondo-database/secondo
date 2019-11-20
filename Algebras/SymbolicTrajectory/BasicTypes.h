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

#include "Algebras/TemporalUnit/TemporalUnitAlgebra.h"
#include "StandardTypes.h"
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "Algebras/TemporalExt/TemporalExtAlgebra.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "NestedList.h"
#include "ListUtils.h"
// #include "Tools.h"
#include <string>
#include <algorithm>
#include <cctype>

namespace stj {
  
enum LabelFunction {TRIVIAL, EDIT};
enum DistanceFunction {FIRST, LAST, FIRST_LAST, ALL, ALL_DURATION, 
                       ALL_INTERVALS, EQUAL_LABELS};
enum GeoDistanceFunction {GEO_FIRST, GEO_LAST, GEO_FIRST_LAST, FRECHET};
  
template<class F, class S>
class NewPair {
 public:
  NewPair() {}
  NewPair(F f, S s) : first(f), second(s) {}
   
  F first;
  S second;
  
  bool operator==(const NewPair& np) const {
    return (first == np.first && second == np.second);
  }
  
  bool operator!=(const NewPair& np) const {
    return !(*this == np);
  }
  
  bool operator<(const NewPair& np) const {
    if (first < np.first) {
      return true;
    }
    if (first == np.first) {
      return second < np.second;
    }
    return false;
  }
  
  void set(const F& f, const S& s) {
    first = f;
    second = s;
  }
  
  friend std::ostream& operator<<(std::ostream& os, const NewPair& np) {
    os << np.first << ", " << np.second;
    return os;
  }
  
  template<class X>
  void copy2ndFrom(const X newValue) {
    assert(sizeof(S) == sizeof(X));
    memcpy((void*)&second, &newValue, sizeof(S));
  }
};

template<class F, class S, class T>
class NewTriple {
 public:
  NewTriple() {}
  NewTriple(F f, S s, T t) : first(f), second(s), third(t) {}
   
  F first;
  S second;
  T third;
  
  bool operator==(const NewTriple& nt) const {
    return (first == nt.first && second == nt.second && third = nt.third);
  }
  
  bool operator<(const NewTriple& nt) const {
    if (first < nt.first) {
      return true;
    }
    if (first == nt.first) {
      if (second < nt.second) {
        return true;
      }
      if (second == nt.second) {
        return third < nt.third;
      }
    }
    return false;
  }
  
  friend std::ostream& operator<<(std::ostream& os, const NewTriple& nt) {
    os << nt.first << ", " << nt.second << ", " << nt.third;
    return os;
  }
};
  
struct SymbolicUnit {
 public: 
  SymbolicUnit() {}
  SymbolicUnit(unsigned int p, unsigned int r) : iv(true), pos(p) {}
  
  temporalalgebra::SecInterval iv;
  unsigned int pos;
};

struct ExtSymbolicUnit : public SymbolicUnit {
  ExtSymbolicUnit() {}
  ExtSymbolicUnit(unsigned int p, unsigned int r) : SymbolicUnit(p,r), ref(r) {}
  
  unsigned int ref;
};

class Labels;

/*
\section{Class ~Label~}

*/
class Label : public Attribute {
 public:
  friend class Labels;
  
  typedef SymbolicUnit unitelem;
  typedef std::string base;
  typedef Labels coll;
   
  Label() {}
  Label(const std::string& text) : Attribute(true), value(8) {SetValue(text);}
  Label(const Label& rhs);
  explicit Label(const bool def) : Attribute(def), value(8) {}
  
  ~Label() {}
  
  void Clean() {
    if (value.getSize() > 0) {value.clean();}}
  void Destroy() {value.destroy();}
  void GetValue(std::string &text) const;
  std::string GetLabel() const {return GetValue();}
  std::string GetValue() const;
  unsigned int GetRef() const {return 0;}
  void GetValues(std::set<std::string>& values) const;
  static void buildValue(const std::string& text, 
                         const unitelem& unit,base& result);
  static ListExpr getList(const std::string& text) {
      return nl->TextAtom(text);
  }
  void Set(const bool def, const std::string &text) {
     std::string newText = text;
     std::replace(newText.begin(), newText.end(), '\'', '`');
     SetDefined(def); SetValue(newText);
  }
  void SetValue(const std::string &text);
  Label& operator=(const Label& lb) {CopyFrom(&lb); return *this;}
  bool operator==(const Label& lb) const;
  bool operator==(const std::string& text) const;
  double Distance(const Label& lb, const LabelFunction lf = EDIT) const;

  static bool readValueFrom(ListExpr LE, std::string& text, unitelem& unit);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  ListExpr ToListExpr(ListExpr typeInfo);
  static bool     CheckKind(ListExpr type, ListExpr& errorInfo);
  static int      SizeOfObj() {return sizeof(Label);}
  static ListExpr Property();
  static const std::string BasicType() {return "label";}
  static const bool checkType(const ListExpr type);
  void            CopyFrom(const Attribute* right);
  int NumOfFLOBs() const {return 1;}
  Flob *GetFLOB(const int i) {assert(i == 0); return &value;}
  int             Compare(const Attribute* arg) const;
  size_t          Sizeof() const {return sizeof(*this);}
  bool            Adjacent(const Attribute*) const {return false;}
  Attribute*      Clone() const {return new Label(*this);}
  size_t          HashValue() const {return value.getSize();}
  std::ostream&        Print(std::ostream& os) const;
  
 protected:
  Flob value;
};

/*
\section{Class ~Labels~}

*/
class Labels : public Attribute {
 public:
  typedef SymbolicUnit unitelem;
  typedef unsigned int arrayelem;
  typedef std::string base;
  typedef Label single;
      
  Labels() {}
  explicit Labels(const bool defined) : Attribute(defined), values(8), pos(1) {}
  Labels(const Labels& src, const bool sort = false);
  
  ~Labels() {}

  Labels& operator=(const Labels& src);
  bool operator==(const Labels& src) const;
  void Append(const Label &lb);
  void Append(const std::string& text);
  void Append(const std::set<std::string>& values);
  void Destroy() {values.destroy(); pos.destroy();}
  int GetNoValues() const {return pos.Size();}
  int GetNoComponents() const {return GetNoValues();}
  size_t GetLength() const {return values.getSize();}
  void Get(int i, Label& lb) const;
  void GetValue(int i, std::string& text) const;
  void GetValues(std::set<std::string>& values) const;
  static void getRefToLastElem(const int size, unsigned int& result);
  static unsigned int getFlobPos(const arrayelem elem);
  static void valuesToListExpr(const std::set<std::string>& values,
                                ListExpr& result);
  static void getString(const ListExpr& list, std::string& result);
  static void getElemFromList(const ListExpr& list, const unsigned int size, 
                              unsigned int& result);
  static void buildValue(const std::string& text, const unsigned int pos,
                         std::string& result);
  static void printArrayElem(const arrayelem e) {cout << "print " << e << endl;}
  const bool IsEmpty() const {return GetNoValues() == 0;}
  void Clean() {values.clean(); pos.clean();}
  bool Contains(const std::string& text) const;
  bool Intersects(const Labels &lbs) const;
  void Union(const std::set<std::string>& values1, 
             const std::set<std::string>& values2);
  void Intersection(const std::set<std::string>& values1, 
                    const std::set<std::string>& values2);
  void Minus(const std::set<std::string>& values1, 
             const std::set<std::string>& values2);
  #ifdef RECODE
  bool Recode(const std::string& from, const std::string& to, Labels& result);
  #endif
  friend std::ostream& operator<<(std::ostream& os, const Labels& lbs);
  double Distance(const Labels& lbs, const LabelFunction lf = EDIT) const;

  int NumOfFLOBs() const {return 2;}
  Flob *GetFLOB(const int i);
  int Compare(const Attribute*) const;
  bool Adjacent(const Attribute*) const {return false;}
  Attribute *Clone() const {return new Labels(*this);}
  size_t Sizeof() const {return sizeof(*this);}
  std::ostream& Print(std::ostream& os) const {return (os << *this);}
  static ListExpr Property();
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  static int SizeOfObj() {return sizeof(Labels);}
  static const std::string BasicType() {return Label::BasicType() + "s";}
  void CopyFrom(const Attribute* right) {*this = *((Labels*)right);}
  size_t HashValue() const;

 protected:
  Flob values;
  DbArray<unsigned int> pos;
};

class Places;

/*
\section{Class ~Place~}

*/
class Place : public Label {
 public: 
  friend class Places;
  
  typedef ExtSymbolicUnit unitelem;
  typedef std::pair<std::string, unsigned int> base;
  typedef Places coll;
  
  Place() : Label() {}
  Place(const std::pair<std::string, unsigned int>& v) :
        Label(v.first), ref(v.second) {}
  Place(const Place& rhs) : Label(rhs.IsDefined()) {CopyFrom(&rhs);}
  explicit Place(const bool def) : Label(def), ref(0) {}

  ~Place() {}

  void Set(const bool defined, const std::pair<std::string,
           unsigned int>& value);
  void SetValue(const std::pair<std::string, unsigned int>& value);
  void SetRef(const unsigned int value) {ref = value;}
  void GetValue(std::pair<std::string, unsigned int>& value) const;
  std::string GetLabel() const {return Label::GetLabel();}
  void GetValues(std::set<std::pair<std::string, unsigned int> >& values) const;
  static void buildValue(const std::string& text, 
              const unitelem& unit,base& result);
  static ListExpr getList(const base& value);
  unsigned int GetRef() const {return ref;}
  Place& operator=(const Place& p);
  bool operator==(const Place& p) const;
  bool operator==(const std::pair<std::string, unsigned int>& value) const;
  double Distance(const Place& p, const LabelFunction lf = EDIT) const;

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(Place);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const std::string BasicType() {return "place";}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return Label::NumOfFLOBs();}
  Flob* GetFLOB(const int i) {return Label::GetFLOB(i);}
  size_t Sizeof() const {return sizeof(*this);}
  int Compare(const Attribute* arg) const;
  bool Adjacent(const Attribute *arg) const {return false;}
  Attribute *Clone() const {return new Place(*this);}
  size_t HashValue() const {return Label::HashValue() * ref;}
  virtual void CopyFrom(const Attribute* right) {*this = *((Place*)right);}
  ListExpr ToListExpr(ListExpr typeInfo);
  static bool readValueFrom(ListExpr LE, std::string& text, unitelem& unit);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);

 protected:
  unsigned int ref;
};

/*
\section{Class ~Places~}

*/
class Places : public Attribute {
 public:
  typedef ExtSymbolicUnit unitelem;
  typedef NewPair<unsigned int, unsigned int> arrayelem;
  typedef std::pair<std::string, unsigned int> base;
  typedef Place single;
   
  Places() {}
  Places(const int n) : Attribute(n > 0), values(8), posref(n) {}
  Places(const Places& rhs, const bool sort = false);
  explicit Places(const bool def) : Attribute(def), values(8), posref(1) {}

  ~Places() {}

  void Append(const base& value);
  void Append(const Place& pl);
  void Append(const std::set<base>& values);
  void Destroy() {values.destroy(); posref.destroy();}
  int GetNoValues() const {return posref.Size();}
  int GetNoComponents() const {return GetNoValues();}
  size_t GetLength() const {return values.getSize();}
  void Get(const int i, Place& pl) const;
  void GetValue(int i, base& val) const;
  void GetValues(std::set<base>& values) const;
  bool IsEmpty() const {return (GetNoValues() == 0);}
  bool Contains(const base& val) const;
  bool Intersects(const Places &pls) const;
  void Union(const std::set<base>& values1, const std::set<base>& values2);
  void Intersection(const std::set<base>& values1, 
                    const std::set<base>& values2);
  void Minus(const std::set<base>& values1, const std::set<base>& values2);
  void Clean() {values.clean(); posref.clean();}
  static void getRefToLastElem(const int size, arrayelem& result);
  static unsigned int getFlobPos(const arrayelem elem);
  static void valuesToListExpr(const std::set<base>& values, ListExpr& result);
  static void getString(const ListExpr& list, std::string& result);
  static void getElemFromList(const ListExpr& list, const unsigned int size, 
                              arrayelem& result);
  static void buildValue(const std::string& text, 
                         const arrayelem pos, base& result);
  static void printArrayElem(const arrayelem e) {cout << e.first << " " 
                                                      << e.second;}
  void operator=(const Places& p);
  bool operator==(const Places& p) const;
  double Distance(const Places& p, const LabelFunction lf = EDIT) const;

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(Places);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const std::string BasicType() {return Place::BasicType() + "s";}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return 2;}
  Flob* GetFLOB(const int i);
  size_t Sizeof() const {return sizeof(*this);}
  int Compare(const Attribute* arg) const;
  bool Adjacent(const Attribute *arg) const {return false;}
  Attribute *Clone() const {return new Places(*this);}
  size_t HashValue() const;
  virtual void CopyFrom(const Attribute* right) {*this = *((Places*)right);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  
 protected:
  Flob values;
  DbArray<NewPair<unsigned int, unsigned int> > posref;
};

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
