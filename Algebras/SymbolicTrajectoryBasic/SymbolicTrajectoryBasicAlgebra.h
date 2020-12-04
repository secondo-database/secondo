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

split from the SymbolicTrajectoryAlgebra in April 2020.

*/

#include "Algebras/TemporalUnit/TemporalUnitAlgebra.h"
#include "StandardTypes.h"
#include "Stream.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "Algebras/Collection/CollectionAlgebra.h"
#include "Algebras/Trie/InvertedFile.h"

namespace stj {
  
enum LabelFunction {TRIVIAL, EDIT};
enum DistanceFunction {FIRST, LAST, FIRST_LAST, ALL, ALL_DURATION, 
                       ALL_INTERVALS, EQUAL_LABELS, COSINUS_SIM, TF_IDF};
enum DistanceFunSym {ERROR = -1, EQUALLABELS, PREFIX, SUFFIX, PREFIXSUFFIX};

template<class F, class S>
class NewPair {
 public:
  NewPair() {}
  NewPair(const F f, const S s) : first(f), second(s) {}
   
  F first;
  S second;
  
  bool operator==(const NewPair& np) const {
    return (first == np.first && second == np.second);
  }
  
  bool operator!=(const NewPair& np) const {
    return !(*this == np);
  }
  
  bool operator<(const NewPair& np) const {
    if (first == np.first) {
      return second < np.second;
    }
    return first < np.first;
  }
  
  void set(const F& f, const S& s) {
    first = f;
    second = s;
  }
  
  void operator=(const NewPair& np) {
    set(np.first, np.second);
  }
  
  friend std::ostream& operator<<(std::ostream& os, const NewPair& np) {
    os << "(" << np.first << ", " << np.second << ")";
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

struct BasicDistanceFuns {
  static double distance(const std::string& str1, const std::string& str2, 
                         const LabelFunction lf = TRIVIAL);
  static double distance(const std::pair<std::string, unsigned int>& val1, 
                       const std::pair<std::string, unsigned int>& val2, 
                       const LabelFunction lf = TRIVIAL);
  static double distance(const std::set<std::string>& values1, 
           const std::set<std::string>& values2, const LabelFunction lf = EDIT);
  static double distance(std::set<std::pair<std::string, 
                         unsigned int> >& values1, 
                         std::set<std::pair<std::string, 
                       unsigned int> >& values2, const LabelFunction lf = EDIT);
};

struct RecodeFun {
  #ifdef RECODE
  static bool recode(const std::string &src, const std::string &from,
                     const std::string &to, std::string &result);
  #endif
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
  void InsertLabels(std::vector<std::string>& result) const;
  void InsertLabels(std::set<std::string>& result) const;
  void UpdateFrequencies(InvertedFile& inv, std::vector<double>& fv) const;

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
  void InsertLabels(std::vector<std::string>& result) const;
  void InsertLabels(std::set<std::string>& result) const;
  void UpdateFrequencies(InvertedFile& inv, std::vector<double>& fv) const;

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
  void InsertLabels(std::vector<std::string>& result) const;
  void InsertLabels(std::set<std::string>& result) const;
  void UpdateFrequencies(InvertedFile& inv, std::vector<double>& fv) const;

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

/*
\section{Class ~IBasic~}

*/
template<class B>
class IBasic : public temporalalgebra::Intime<B> {
 public:
  IBasic() {}
  explicit IBasic(const Instant& inst, const B& val);
  explicit IBasic(const IBasic& rhs);
  IBasic(const bool def) : temporalalgebra::Intime<B>(def) {}

  ~IBasic() {}

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(IBasic<B>);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const std::string BasicType() {return "i" + B::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return this->value.NumOfFLOBs();}
  Flob* GetFLOB(const int i) {return this->value.GetFLOB(i);}
  size_t Sizeof() const {return sizeof(*this);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  
  void Val(B& result) const;
};

/*
\section{Class ~IBasics~}

*/
template<class B>
class IBasics : public temporalalgebra::Intime<B> {
 public:
  IBasics() {}
  explicit IBasics(const Instant& inst, const B& values);
  IBasics(const IBasics& rhs) : temporalalgebra::Intime<B>(rhs) {}
  IBasics(const bool def) : temporalalgebra::Intime<B>(def) {}
  
  static ListExpr Property();
  static int SizeOfObj() {return sizeof(IBasics<B>);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const std::string BasicType() {return "i" + B::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return this->value.NumOfFLOBs();}
  Flob* GetFLOB(const int i) {return this->value.GetFLOB(i);}
  size_t Sizeof() const {return sizeof(*this);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  
  void Val(B& result) const;
};

/*
\section{Class ~UBasic~}

*/
template<class B>
class UBasic : public temporalalgebra::ConstTemporalUnit<B> {
 public:
  UBasic() : temporalalgebra::ConstTemporalUnit<B>(true) {}
  explicit UBasic(bool def) : temporalalgebra::ConstTemporalUnit<B>(def) {}
  explicit UBasic(const temporalalgebra::SecInterval &iv, const B& val);
  UBasic(const UBasic& ub);
  
  ~UBasic() {}

  bool operator==(const UBasic<B>& rhs) const;

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(UBasic<B>);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const std::string BasicType() {return "u" + B::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return this->constValue.NumOfFLOBs();}
  Flob* GetFLOB(const int i) {return this->constValue.GetFLOB(i);}
  size_t Sizeof() const {return sizeof(*this);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);

  void Initial(IBasic<B>& result) const;
  void Final(IBasic<B>& result) const;
  void GetInterval(temporalalgebra::SecInterval& result) const;
};

/*
\section{Class ~UBasics~}

*/
template<class B>
class UBasics : public temporalalgebra::ConstTemporalUnit<B> {
 public:
  UBasics() : temporalalgebra::ConstTemporalUnit<B>(true) {}
  UBasics(const temporalalgebra::SecInterval& iv, const B& values);
  UBasics(const UBasics& rhs) : temporalalgebra::ConstTemporalUnit<B>(rhs) {}
  explicit UBasics(const bool def) : 
              temporalalgebra::ConstTemporalUnit<B>(def) {}

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(UBasics<B>);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const std::string BasicType() {return "u" + B::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return this->constValue.NumOfFLOBs();}
  Flob* GetFLOB(const int i) {return this->constValue.GetFLOB(i);}
  size_t Sizeof() const {return sizeof(*this);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  
  void Initial(IBasics<B>& result) const;
  void Final(IBasics<B>& result) const;
  void GetInterval(temporalalgebra::SecInterval& result) const;
};

class MLabel;

/*
\section{Class ~MBasic~}

*/
template<class B>
class MBasic : public Attribute {
 public:
  typedef B base;
   
  MBasic() {}
  explicit MBasic(unsigned int n) : Attribute(n>0), values(1024), units(n),
                                    noChars(0) {}
  explicit MBasic(const MBasic& mb);
  
  ~MBasic() {}

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(MBasic<B>);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const std::string BasicType() {return "m" + B::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return 2;}
  Flob* GetFLOB(const int i);
  size_t Sizeof() const {return sizeof(*this);}
  int Compare(const Attribute *arg) const;
  Attribute* Clone() const {return new MBasic<B>(*this);}
  bool Adjacent(const Attribute *arg) const {return false;}
  size_t HashValue() const;
  MBasic<B>& operator=(const MBasic<B>& src);
  void CopyFrom(const Attribute *arg);
  
  ListExpr unitToListExpr(const int i);
  ListExpr ToListExpr(ListExpr typeInfo);
  bool readUnitFrom(ListExpr LE);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);

  int GetNoChars() const {return noChars;}
  void serialize(size_t &size, char *&bytes) const;
  static MBasic<B>* deserialize(const char *bytes);
  int Position(const Instant& inst, const bool ignoreClosedness = false) const;
  int FirstPosFrom(const Instant& inst) const;
  int LastPosUntil(const Instant& inst) const;
  void Get(const int i, UBasic<B>& result) const;
  void GetInterval(const int i, temporalalgebra::SecInterval& result) const;
  void GetInterval(temporalalgebra::SecInterval& result) const;
  void GetDuration(datetime::DateTime& result) const;
  void GetBasic(const int i, B& result) const;
  void GetValue(const int i, typename B::base& result) const;
  bool IsEmpty() const {return units.Size() == 0;}
  int GetNoComponents() const {return units.Size();}
  bool IsValid() const;
  void Clear() {values.clean(); units.clean(); noChars = 0;}
  void StartBulkLoad() {assert(IsDefined());}
  void EndBulkLoad(const bool sort = true, const bool checkvalid = true);
  void Add(const temporalalgebra::SecInterval& iv, const B& value);
  void Add(const UBasic<B>& ub);
  void MergeAdd(const UBasic<B>& ub);
  bool Passes(const B& basic) const;
  template<class T>
  bool Passes(const T& value) const;
  void At(const B& basic, MBasic<B>& result) const;
  template<class T>
  void At(const T& value, MBasic<B>& result) const;
  void DefTime(temporalalgebra::Periods& per) const;
  void AtInstant(const Instant& inst, IBasic<B>& result) const;
  void AtPeriods(const temporalalgebra::Periods& per, MBasic<B>& result) const;
  void Initial(IBasic<B>& result) const;
  void Final(IBasic<B>& result) const;
  void InitialInstant(Instant& result) const;
  void FinalInstant(Instant& result) const;
  void Inside(const typename B::coll& coll, 
              temporalalgebra::MBool& result) const;
  void Fill(MBasic<B>& result, datetime::DateTime& duration) const;
  void Concat(const MBasic<B>& src1, const MBasic<B>& src2);
  void Compress(MBasic<B>& result) const;
  void GetPart(const int from, const int to, MBasic<B>& result);
  #ifdef RECODE
  void Recode(const std::string& from, const std::string& to,MBasic<B>& result);
  #endif
  NewPair<int, int> LongestCommonSubsequence(const MBasic<B>& mb);
  std::ostream& Print(std::ostream& os) const;  
  double Distance_FIRST(const MBasic<B>& mb) const;
  double Distance_LAST(const MBasic<B>& mb) const;
  double Distance_FIRST_LAST(const MBasic<B>& mb, const LabelFunction lf) const;
  double Distance_ALL(const MBasic<B>& mb, const LabelFunction lf) const;
  double Distance_ALL_DURATION(const MBasic<B>& mb, const LabelFunction lf)
         const;
  double Distance_ALL_INTERVALS(const MBasic<B>& mb, const LabelFunction lf)
         const;
  double Distance_EQUAL_LABELS(const MBasic<B>& mb, const LabelFunction lf)
         const;
  double Distance_COSINUS_SIM(const MBasic<B>& mb);
  double Distance_TF_IDF(const MBasic<B>& mb);
  double Distance(const MBasic<B>& mb, const DistanceFunction df = ALL, 
          const LabelFunction lf = TRIVIAL, const double threshold = 1.0) const;
  int CommonPrefixSuffix(const MBasic<B>& mb, const bool prefix);
  double DistanceSym(const MBasic<B>& mb, const DistanceFunSym distfun);
  void InsertLabels(std::vector<std::string>& result) const;
  void InsertLabels(std::set<std::string>& result) const;
  void FrequencyVector(InvertedFile& inv, std::vector<double>& fv,
                       const bool useIdf = false) const;
  
 protected:
  Flob values;
  DbArray<typename B::unitelem> units;
  int noChars;
};

/*
\section{Class ~MBasics~}

*/
template<class B>
class MBasics : public Attribute {
 public:
  typedef B base;
   
  MBasics() {}
  explicit MBasics(int n) : Attribute(n > 0), values(8), units(n), pos(1),
                            noChars(0) {}
  explicit MBasics(const MBasics& mbs);
  
  ~MBasics() {}
  
  static ListExpr Property();
  static int SizeOfObj() {return sizeof(MBasics<B>);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const std::string BasicType() {return "m" + B::BasicType();}
  static bool checkType(ListExpr t);
  int NumOfFLOBs() const {return 3;}
  Flob* GetFLOB(const int i);
  size_t Sizeof() const {return sizeof(*this);}
  int Compare(const Attribute* arg) const;
  bool Adjacent(const Attribute *arg) const {return false;}
  Attribute *Clone() const;
  size_t HashValue() const {return pos.Size() * units.Size();}
  void CopyFrom(const Attribute* right) {*this = *((MBasics<B>*)right);}
  MBasics<B>& operator=(const MBasics<B>& src);
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  void Destroy() {values.destroy(); units.destroy(); pos.destroy();}
  
  int getUnitEndPos(const int i) const;
  ListExpr valuesToListExpr(int start, int end);
  ListExpr unitToListExpr(int i);
  bool readValues(ListExpr valuelist);
  bool readUnit(ListExpr unitlist);
  
  int GetNoChars() const {return noChars;}
  void serialize(size_t &size, char *&bytes) const;
  static MBasics<B>* deserialize(const char *bytes);
  int Position(const Instant& inst, const bool ignoreClosedness = false) const;
  int FirstPosFrom(const Instant& inst) const;
  int LastPosUntil(const Instant& inst) const;
  void Get(const int i, UBasics<B>& result) const;
  void GetBasics(const int i, B& result) const;
  bool IsEmpty() const {return units.Size() == 0;}
  void GetValues(const int i, std::set<typename B::base>& result) const;
  void GetInterval(const int i, temporalalgebra::SecInterval& result) const;
  void GetInterval(temporalalgebra::SecInterval& result) const;
  int GetNoComponents() const {return units.Size();}
  int GetNoValues() const {return pos.Size();}
  void Clear();
  void StartBulkLoad() {assert(IsDefined());}
  void EndBulkLoad(const bool sort = true, const bool checkvalid = true);
  bool IsValid() const;
  void Add(const UBasics<B>& ut);
  void Add(const temporalalgebra::SecInterval& iv, const B& values);
  void MergeAdd(const UBasics<B>& ut);
  void MergeAdd(const temporalalgebra::SecInterval& iv, const B& values);
  bool Passes(const typename B::single& sg) const;
  bool Passes(const B& bs) const;
  template<class T>
  bool Passes(const T& value) const;
  void At(const typename B::single& sg, MBasics<B>& result) const;
  void At(const B& bs, MBasics<B>& result) const;
  template<class T>
  void At(const T& value, MBasics<B>& result) const;
  void DefTime(temporalalgebra::Periods& per) const;
  void AtInstant(const Instant& inst, IBasics<B>& result) const;
  void AtPeriods(const temporalalgebra::Periods& per, MBasics<B>& result) const;
  void Initial(IBasics<B>& result) const;
  void Final(IBasics<B>& result) const;
  void InitialInstant(Instant& result) const;
  void FinalInstant(Instant& result) const;
  void Fill(MBasics<B>& result, datetime::DateTime& duration) const;
  void Concat(const MBasics<B>& src1, const MBasics<B>& src2);
  void Compress(MBasics<B>& result) const;
  #ifdef RECODE
  void Recode(const std::string& from,const std::string& to,MBasics<B>& result);
  #endif
  std::ostream& Print(std::ostream& os) const;
  double Distance_FIRST_LAST(const MBasics<B>& mbs, 
                             const LabelFunction lf) const;
  double Distance_ALL(const MBasics<B>& mbs, const LabelFunction lf) const;
  double Distance(const MBasics<B>& mbs, const DistanceFunction df = ALL, 
          const LabelFunction lf = TRIVIAL, const double threshold = 1.0) const;
  int CommonPrefixSuffix(const MBasics<B>& mbs, const bool prefix);
  double DistanceSym(const MBasics<B>& mbs, const DistanceFunSym distfun);
  void InsertLabels(std::vector<std::string>& result) const;
  void InsertLabels(std::set<std::string>& result) const;
  void FrequencyVector(InvertedFile& inv, std::vector<double>& fv,
                       const bool useIdf = false) const;
  
 protected:
  Flob values;
  DbArray<SymbolicUnit> units;
  DbArray<typename B::arrayelem> pos;
  int noChars;
};


/*
\section{Class ~MLabel~}

*/
class MLabel : public MBasic<Label> {
 public:
  MLabel() {}
  MLabel(unsigned int n) : MBasic<Label>(n) {}

  void createML(const int size, const int number, 
                std::vector<std::string>& labels);
  void convertFromMString(const temporalalgebra::MString& source);
};

typedef IBasic<Label> ILabel;
typedef IBasic<Place> IPlace;
typedef IBasics<Labels> ILabels;
typedef IBasics<Places> IPlaces;
typedef UBasic<Label> ULabel;
typedef UBasic<Place> UPlace;
typedef UBasics<Labels> ULabels;
typedef UBasics<Places> UPlaces;
typedef MBasics<Labels> MLabels;
typedef MBasic<Place> MPlace;
typedef MBasics<Places> MPlaces;

/*
\section{Implementation of Class ~IBasic~}

\subsection{Constructors}

*/
template<class B>
IBasic<B>::IBasic(const Instant& inst, const B& val) : 
  temporalalgebra::Intime<B>(inst, val) {
  SetDefined(inst.IsDefined() && val.IsDefined());
}

template<class B>
IBasic<B>::IBasic(const IBasic& rhs) : 
  temporalalgebra::Intime<B>(rhs.instant, rhs.value) {
  SetDefined(rhs.IsDefined());
}

/*
\subsection{Function ~Property~}

*/
template<class B>
ListExpr IBasic<B>::Property() {
  if (BasicType() == "ilabel") {
    return gentc::GenProperty("-> DATA", BasicType(), "(<instant> <label>)",
                              "(\"2014-02-26\" \'Dortmund\')");
  }
  if (BasicType() == "iplace") {
    return gentc::GenProperty("-> DATA", BasicType(), "(<instant> <place>)",
                              "(\"2014-02-18\" (\'Dortmund\' 1909))");
  }
  return gentc::GenProperty("-> DATA", BasicType(), "Error: invalid type.", "");
}

/*
\subsection{Function ~CheckKind~}

*/
template<class B>
bool IBasic<B>::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, IBasic<B>::BasicType());
}

/*
\subsection{Function ~ToListExpr~}

*/
template<class B>
ListExpr IBasic<B>::ToListExpr(ListExpr typeInfo) {
  if (!temporalalgebra::Intime<B>::IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  return nl->TwoElemList(this->instant.ToListExpr(false), 
                         this->value.ToListExpr(nl->Empty()));
}

/*
\subsection{Function ~ReadFrom~}

*/
template<class B>
bool IBasic<B>::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  this->SetDefined(false);
  if (listutils::isSymbolUndefined(LE)) {
    return true;
  }
  if (!nl->HasLength(LE, 2)) {
    return false;
  }
  if (this->instant.ReadFrom(nl->First(LE), nl->Empty()) &&
      this->value.ReadFrom(nl->Second(LE), nl->Empty())) {
    this->SetDefined(this->instant.IsDefined() && this->value.IsDefined());
    return true;
  }
  return false;
}

/*
\subsection{Function ~Val~}

*/
template<class B>
void IBasic<B>::Val(B& result) const {
  if (!this->IsDefined()) {
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(&(this->value));
}

/*
\section{Implementation of Class ~IBasics~}

\subsection{Constructors}

*/
template<class B>
IBasics<B>::IBasics(const Instant& inst, const B& values) : 
                   temporalalgebra::Intime<B>(inst, values) {}

/*
\subsection{Function ~Property~}

*/
template<class B>
ListExpr IBasics<B>::Property() {
  if (B::BasicType() == Labels::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(), "(<instant> <labels>)",
      "(\"2014-02-18\" (\"Dortmund\" \"Mailand\"))");
  }
  if (B::BasicType() == Places::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(), "(<instant> <places>)",
      "(\"2014-02-18\" ((\"Dortmund\" 1909) (\"Mailand\" 1899)))");
  }
  return gentc::GenProperty("-> DATA", BasicType(), "Error: invalid type.", "");
}

/*
\subsection{Function ~CheckKind~}

*/
template<class B>
bool IBasics<B>::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, IBasics<B>::BasicType());
}

/*
\subsection{Function ~ToListExpr~}

*/
template<class B>
ListExpr IBasics<B>::ToListExpr(ListExpr typeInfo) {
  if (!this->IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  return nl->TwoElemList(this->instant.ToListExpr(false), 
                         this->value.ToListExpr(nl->Empty()));
}

/*
\subsection{Function ~ReadFrom~}

*/
template<class B>
bool IBasics<B>::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  this->SetDefined(false);
  if (listutils::isSymbolUndefined(LE)) {
    return true;
  }
  if (!nl->HasLength(LE, 2)) {
    return false;
  }
  if (this->instant.ReadFrom(nl->First(LE), nl->Empty()) &&
      this->value.ReadFrom(nl->Second(LE), nl->Empty())) {
    this->SetDefined(this->instant.IsDefined() && this->value.IsDefined());
    return true;
  }
  return false;
}

/*
\subsection{Function ~Val~}

*/
template<class B>
void IBasics<B>::Val(B& result) const {
  if (!this->IsDefined()) {
    result.SetDefined(false);
    return;
  }
  result = this->value;
}

/*
\section{Implementation of class ~UBasic~}

\subsection{Constructors}

*/
template<class B>
UBasic<B>::UBasic(const temporalalgebra::SecInterval &iv, const B& val)
    : temporalalgebra::ConstTemporalUnit<B>(iv, val) {
  this->SetDefined(iv.IsDefined() && val.IsDefined());
}

template<class B>
UBasic<B>::UBasic(const UBasic<B>& ub) : 
  temporalalgebra::ConstTemporalUnit<B>(ub) {
  this->SetDefined(ub.IsDefined());
}

/*
\subsection{Operator ~==~}

*/
template<class B>
bool UBasic<B>::operator==(const UBasic<B>& rhs) const {
  return Compare(&rhs) == 0;
}

/*
\subsection{Function ~Property~}

*/
template<class B>
ListExpr UBasic<B>::Property() {
  if (B::BasicType() == Label::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(), "(<interval> <label>)",
      "((\"2014-02-18\" \"2014-02-19-13:00\" TRUE FALSE) \"Dortmund\")");
  }
  if (B::BasicType() == Place::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(), "(<interval> <place>)",
      "((\"2014-02-18\" \"2014-02-19-13:00\" TRUE FALSE) (\"Dortmund\" 1909))");
  }
  return gentc::GenProperty("-> DATA", BasicType(), "Error: invalid type.", "");
}

/*
\subsection{Function ~CheckKind~}

*/
template<class B>
bool UBasic<B>::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, UBasic<B>::BasicType());
}

/*
\subsection{Function ~ToListExpr~}

*/
template<class B>
ListExpr UBasic<B>::ToListExpr(ListExpr typeInfo) {
  if (!this->IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  return nl->TwoElemList((
    (temporalalgebra::SecInterval)this->timeInterval).ToListExpr (nl->Empty()),
    this->constValue.ToListExpr(nl->Empty()));
}

/*
\subsection{Function ~ReadFrom~}

*/
template<class B>
bool UBasic<B>::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  this->SetDefined(false);
  if (listutils::isSymbolUndefined(LE)) {
    return true;
  }
  if (!nl->HasLength(LE, 2)) {
    return false;
  }
  temporalalgebra::SecInterval iv(true);
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  if (iv.ReadFrom(nl->First(LE), 
      sc->GetTypeExpr(temporalalgebra::SecInterval::BasicType())) &&
      this->constValue.ReadFrom(nl->Second(LE), nl->Empty())) {
    this->timeInterval = iv;
    this->SetDefined(this->timeInterval.IsDefined() && 
                     this->constValue.IsDefined());
    return true;
  }
  return false;
}

/*
\subsection{Function ~Initial~}

*/
template<class B>
void UBasic<B>::Initial(IBasic<B>& result) const {
  if (this->IsDefined()) {
    result.instant = this->timeInterval.start;
    result.value.CopyFrom(&(this->constValue));
    result.SetDefined(true);
  }
  else {
    result.SetDefined(false);
  }
}

/*
\subsection{Function ~Final~}

*/
template<class B>
void UBasic<B>::Final(IBasic<B>& result) const {
  if (this->IsDefined()) {
    result.instant = this->timeInterval.end;
    result.value.CopyFrom(&(this->constValue));
    result.SetDefined(true);
  }
  else {
    result.SetDefined(false);
  }
}

/*
\subsection{Function ~GetInterval~}

*/
template<class B>
void UBasic<B>::GetInterval(temporalalgebra::SecInterval& result) const {
  if (this->IsDefined()) {
    result = this->timeInterval;
  }
  else {
    result.SetDefined(false);
  }
}

/*
\section{Implementation of class ~UBasics~}

\subsection{Constructors}

*/
template<class B>
UBasics<B>::UBasics(const temporalalgebra::SecInterval &iv, const B& values)
           : temporalalgebra::ConstTemporalUnit<B>(iv, values) {}

/*
\subsection{Function ~Property~}

*/
template<class B>
ListExpr UBasics<B>::Property() {
  if (B::BasicType() == Labels::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(), "(<interval> <labels>)",
      "((\"2014-02-18\" \"2014-02-19-13:00\" TRUE FALSE) (\"Dortmund\" "
      "\"Mailand\"))");
  }
  if (B::BasicType() == Places::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(), "(<interval> <places>)",
      "((\"2014-02-18\" \"2014-02-19-13:00\" TRUE FALSE) ((\"Dortmund\" 1909) "
      "(\"Mailand\" 1899)))");
  }
  return gentc::GenProperty("-> DATA", BasicType(), "Error: invalid type.", "");
}

/*
\subsection{Function ~CheckKind~}

*/
template<class B>
bool UBasics<B>::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, UBasics<B>::BasicType());
}

/*
\subsection{Function ~ToListExpr~}

*/
template<class B>
ListExpr UBasics<B>::ToListExpr(ListExpr typeInfo) {
  if (!this->IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  return nl->TwoElemList((
    (temporalalgebra::SecInterval)this->timeInterval).ToListExpr (nl->Empty()),
     this->constValue.ToListExpr(nl->Empty()));
}

/*
\subsection{Function ~ReadFrom~}

*/
template<class B>
bool UBasics<B>::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  this->SetDefined(false);
  if (listutils::isSymbolUndefined(LE)) {
    return true;
  }
  if (!nl->HasLength(LE, 2)) {
    return false;
  }
  temporalalgebra::SecInterval iv(true);
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  if (iv.ReadFrom(nl->First(LE), 
      sc->GetTypeExpr(temporalalgebra::SecInterval::BasicType())) &&
      this->constValue.ReadFrom(nl->Second(LE), nl->Empty())) {
    this->timeInterval = iv;
    this->SetDefined(this->timeInterval.IsDefined() &&
                     this->constValue.IsDefined());
    return true;
  }
  return false;
}

/*
\subsection{Function ~Initial~}

*/
template<class B>
void UBasics<B>::Initial(IBasics<B>& result) const {
  if (this->IsDefined()) {
    result.instant = this->timeInterval.start;
    result.value = this->constValue;
    result.SetDefined(true);
  }
  else {
    result.SetDefined(false);
  }
}

/*
\subsection{Function ~Final~}

*/
template<class B>
void UBasics<B>::Final(IBasics<B>& result) const {
  if (this->IsDefined()) {
    result.instant = this->timeInterval.end;
    result.value = this->constValue;
    result.SetDefined(true);
  }
  else {
    result.SetDefined(false);
  }
}

/*
\subsection{Function ~GetInterval~}

*/
template<class B>
void UBasics<B>::GetInterval(temporalalgebra::SecInterval& result) const {
  if (this->IsDefined()) {
    result = this->timeInterval;
  }
  else {
    result.SetDefined(false);
  }
}

/*
\section{Implementation of class ~MBasic~}

\subsection{Constructors}

*/
template<class B>
MBasic<B>::MBasic(const MBasic &mb) : Attribute(mb.IsDefined()),
                       values(mb.values.getSize()), units(mb.GetNoComponents()),
                                                noChars(mb.GetNoChars()) {
  if (IsDefined()) {
    values.copyFrom(mb.values);
    units.copyFrom(mb.units);
  }
}

/*
\subsection{Function ~Property~}

*/
template<class B>
ListExpr MBasic<B>::Property() {
  if (B::BasicType() == Label::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(),
      "((<interval> <label>) (<interval> <label>) ...)",
      "(((\"2014-02-27\" \"2014-02-27-09:48\" TRUE FALSE) \"Dortmund\") "
      "((\"2014-05-17\" \"2014-05-17-22:00\" TRUE FALSE) \"Berlin\"))");
  }
  if (B::BasicType() == Place::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(),
      "((<interval> <place>) (<interval> <place>) ...)",
      "(((\"2014-02-27\" \"2014-02-27-09:48\" TRUE FALSE) (\"Dortmund\" 1909)) "
      "((\"2014-05-17\" \"2014-05-17-22:00\" TRUE FALSE) (\"Berlin\" 4)))");
  }
  return gentc::GenProperty("-> DATA", BasicType(), "Error: invalid type.", "");
}

/*
\subsection{Function ~GetFLOB~}

*/
template<class B>
Flob* MBasic<B>::GetFLOB(const int i) {
  assert((i >= 0) && (i < NumOfFLOBs()));
  return (i == 0 ? &values : &units);
}

/*
\subsection{Function ~KindCheck~}

*/
template<class B>
bool MBasic<B>::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, MBasic<B>::BasicType());
}

/*
\subsection{Function ~Compare~}

*/
template<class B>
int MBasic<B>::Compare(const Attribute* arg) const {
  if (GetNoComponents() > ((MBasic<B>*)arg)->GetNoComponents()) {
    return 1;
  }
  if (GetNoComponents() < ((MBasic<B>*)arg)->GetNoComponents()) {
    return -1;
  }
  UBasic<B> ub1(true), ub2(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    Get(i, ub1);
    ((MBasic<B>*)arg)->Get(i, ub2);
    int comp = ub1.Compare(&ub2);
    if (comp != 0) {
      return comp;
    }
  }
  return 0;
}

/*
\subsection{Function ~HashValue~}

*/
template<class B>
size_t MBasic<B>::HashValue() const {
  if (!IsDefined() || IsEmpty()) {
    return 0;
  }
  typename B::unitelem unit;
  units.Get(0, unit);
  return values.getSize() * unit.iv.HashValue();
}

/*
\subsection{Function ~CopyFrom~}

*/
template<class B>
void MBasic<B>::CopyFrom(const Attribute *arg) {
  if (!arg->IsDefined()) {
    SetDefined(false);
    return;
  }
  SetDefined(true);
  values.copyFrom(((MBasic<B>*)arg)->values);
  units.copyFrom(((MBasic<B>*)arg)->units);
  noChars = ((MBasic<B>*)arg)->noChars;
}

/*
\subsection{Function ~=~}

*/
template<class B>
MBasic<B>& MBasic<B>::operator=(const MBasic<B>& src) {
  Attribute::operator=(src);
  units.copyFrom(src.units);
  values.copyFrom(src.values);
  noChars = src.noChars;
  return *this;
}

/*
\subsection{Function ~unitToListExpr~}

*/
template<class B>
ListExpr MBasic<B>::unitToListExpr(const int i) {
  assert(i < GetNoComponents());
  typename B::base value;
  GetValue(i, value);
  typename B::unitelem unit;
  units.Get(i, unit);
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  return nl->TwoElemList(unit.iv.ToListExpr(sc->GetTypeExpr(
                        temporalalgebra::SecInterval::BasicType())),
                        B::getList(value));
}

/*
\subsection{Function ~ToListExpr~}

*/
template<class B>
ListExpr MBasic<B>::ToListExpr(ListExpr typeInfo) {
  if (!IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  if (IsEmpty()) {
    return nl->Empty();
  }
  ListExpr result = nl->OneElemList(unitToListExpr(0));
  ListExpr last = result;
  for (int i = 1; i < GetNoComponents(); i++) {
    last = nl->Append(last, unitToListExpr(i));
  }
  return result;
}

/*
\subsection{Function ~ReadFrom~}

*/
template<class B>
bool MBasic<B>::readUnitFrom(ListExpr LE) {
  if (!nl->HasLength(LE, 2)) {
    return false;
  }
  temporalalgebra::SecInterval iv;
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  if (!iv.ReadFrom(nl->First(LE), 
      sc->GetTypeExpr(temporalalgebra::SecInterval::BasicType()))) {
    return false;
  }
  typename B::unitelem unit(GetNoChars(), 0);
  unit.iv = iv;
  std::string text;
  if (!B::readValueFrom(nl->Second(LE), text, unit)) {
    return false;
  }
  units.Append(unit);
  text = text.substr(1, text.length() - 2);
  if (text.length() > 0) {
    const char *bytes = text.c_str();
    if (values.getSize() == 0) {
      values.resize(1024);
    }
    if (noChars + text.length() >= values.getSize()) {
      values.resize(2 * values.getSize());
    }
    values.write(bytes, text.length(), noChars);
    noChars += text.length();
  }
  return true;
}

/*
\subsection{Function ~ReadFrom~}

*/
template<class B>
bool MBasic<B>::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  SetDefined(false);
  Clear();
  if (listutils::isSymbolUndefined(LE)) {
    return true;
  }
  ListExpr rest = LE;
  while (!nl->IsEmpty(rest)) {
    if (!readUnitFrom(nl->First(rest))) {
      return false;
    }
    rest = nl->Rest(rest);
  }
  SetDefined(true);
  return true;
}

/*
\subsection{Function ~serialize~}

*/
template<class B>
void MBasic<B>::serialize(size_t &size, char *&bytes) const {
  size = 0;
  bytes = 0;
  if (!IsDefined()) {
    return;
  }
  size_t rootSize = Sizeof();
  size = 2 * sizeof(size_t) + values.getSize() + units.GetFlobSize();
  bytes = new char[size];
  cout << "memcpy1" << endl;
  memcpy(bytes, (void*)&rootSize, sizeof(size_t));
  cout << "ok, now memcpy2" << endl;
  memcpy(bytes + sizeof(size_t), (void*)this, rootSize);
  size_t offset = sizeof(size_t) + rootSize;
  char* data = values.getData();
  cout << "ok, now memcpy3" << endl;
  memcpy(bytes + offset, data, values.getSize());
  delete[] data;
  offset += values.getSize();
  data = units.getData();
  cout << "ok, now memcpy4" << endl;
  memcpy(bytes + offset, data, units.GetFlobSize());
  cout << "ok" << endl;
  delete[] data;
  cout << "buffer \'data\' deleted" << endl;
}

/*
\subsection{Function ~deserialize~}

*/
template<class B>
MBasic<B>* MBasic<B>::deserialize(const char *bytes) {
  ListExpr typeExpr = nl->SymbolAtom(BasicType());
  int algebraId, typeId;
  std::string typeName;
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  sc->LookUpTypeExpr(typeExpr, typeName, algebraId, typeId);
  size_t rootSize;
  memcpy((void*)&rootSize, bytes, sizeof(size_t));
  char* root = new char[rootSize];
  memcpy(root, bytes + sizeof(size_t), rootSize);
  AlgebraManager* am = SecondoSystem::GetAlgebraManager();
  ListExpr nType = sc->NumericType(typeExpr);
  Attribute* attr = (Attribute*)(am->CreateObj(algebraId, typeId)(nType)).addr;
  attr = attr->Create(attr, root, rootSize, algebraId, typeId);
  delete[] root;
  size_t offset = rootSize + sizeof(size_t);
  for (int i = 0; i < attr->NumOfFLOBs(); i++) {
    Flob* flob = attr->GetFLOB(i);
    size_t size = flob->getSize();
    flob->kill();
    char* fb = new char[size];
    memcpy(fb, bytes + offset, size);
    flob->createFromBlock(*flob, fb, size, false);
    delete[] fb;
    offset += size;
  }
  return (MBasic<B>*)attr;
}

/*
\subsection{Function ~Position~}

*/
template<class B>
int MBasic<B>::Position(const Instant& inst, 
                        const bool ignoreClosedness /* = false */) const {
  assert(IsDefined());
  assert(inst.IsDefined());
  int first = 0, last = GetNoComponents() - 1;
  Instant t1 = inst;
  while (first <= last) {
    int mid = (first + last) / 2;
    if ((mid < 0) || (mid >= GetNoComponents())) {
      return -1;
    }
    typename B::unitelem unit;
    units.Get(mid, unit);
    if (((temporalalgebra::Interval<Instant>)(unit.iv)).Contains(t1, 
                                                            ignoreClosedness)) {
      return mid;
    }
    else { // not contained
      if ((t1 > unit.iv.end) || (t1 == unit.iv.end)) {
        first = mid + 1;
      }
      else if ((t1 < unit.iv.start) || (t1 == unit.iv.start)) {
        last = mid - 1;
      }
      else {
        return -1; // should never be reached.
      }
    }
  }
  return -1;
}

/*
\subsection{Function ~FirstPosFrom~}

*/
template<class B>
int MBasic<B>::FirstPosFrom(const Instant& inst) const {
  assert(IsDefined());
  assert(inst.IsDefined());
  if (IsEmpty()) {
    return -1;
  }
  typename B::unitelem unit;
  units.Get(0, unit);
  if (inst < ((temporalalgebra::Interval<Instant>)(unit.iv)).start) {
    return 0; //
  }
  units.Get(GetNoComponents() - 1, unit);
  if (inst > ((temporalalgebra::Interval<Instant>)(unit.iv)).end) {
    return -1;
  }
  int first(0), last(GetNoComponents() - 1);
  while (first <= last) {
    int mid = (first + last) / 2;
    units.Get(mid, unit);
    if (inst > ((temporalalgebra::Interval<Instant>)(unit.iv)).start) {
      if (inst < ((temporalalgebra::Interval<Instant>)(unit.iv)).end) {
        return mid;
      }
      first = mid + 1;
    }
    else if (inst < ((temporalalgebra::Interval<Instant>)(unit.iv)).start) {
      int beforeMid = mid - 1;
      units.Get(beforeMid, unit);
      if (inst > ((temporalalgebra::Interval<Instant>)(unit.iv)).start) {
        return mid;
      }
      else if (inst < ((temporalalgebra::Interval<Instant>)(unit.iv)).start) {
        last = mid - 1;
      }
      else {
        return mid;
      }
    }
    else {
      return mid;
    }
  }
  return -1;
}

/*
\subsection{Function ~LastPosUntil~}

*/
template<class B>
int MBasic<B>::LastPosUntil(const Instant& inst) const {
  assert(IsDefined());
  assert(inst.IsDefined());
  if (IsEmpty()) {
    return -1;
  }
  typename B::unitelem unit;
  units.Get(0, unit);
  if (inst < ((temporalalgebra::Interval<Instant>)(unit.iv)).start) {
    return -1; //
  }
  units.Get(GetNoComponents() - 1, unit);
  if (inst > ((temporalalgebra::Interval<Instant>)(unit.iv)).end) {
    return GetNoComponents() - 1;
  }
  int first(0), last(GetNoComponents() - 1);
  while (first <= last) {
    int mid = (first + last) / 2;
    units.Get(mid, unit);
    if (inst < ((temporalalgebra::Interval<Instant>)(unit.iv)).end) {
      if (inst > ((temporalalgebra::Interval<Instant>)(unit.iv)).start) {
        return mid;
      }
      last = mid - 1;
    }
    else if (inst > ((temporalalgebra::Interval<Instant>)(unit.iv)).end) {
      int afterMid = mid + 1;
      units.Get(afterMid, unit);
      if (inst < ((temporalalgebra::Interval<Instant>)(unit.iv)).end) {
        return mid;
      }
      else if (inst > ((temporalalgebra::Interval<Instant>)(unit.iv)).end) {
        first = mid + 1;
      }
      else {
        return mid;
      }
    }
    else {
      return mid;
    }
  }
  return -1;
}

/*
\subsection{Function ~Get~}

*/
template<class B>
void MBasic<B>::Get(const int i, UBasic<B>& result) const {
  assert((i >= 0) && (i < GetNoComponents()));
  result.SetDefined(IsDefined());
  if (IsDefined()) {
    typename B::unitelem unit;
    units.Get(i, unit);
    result.timeInterval = unit.iv;
    GetBasic(i, result.constValue);
  }
}

/*
\subsection{Function ~GetDuration~}

*/
template<class B>
void MBasic<B>::GetDuration(datetime::DateTime& result) const {
  temporalalgebra::SecInterval iv(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetInterval(i, iv);
    result += (iv.end - iv.start);
  }
}

/*
\subsection{Function ~GetInterval~}

*/
template<class B>
void MBasic<B>::GetInterval(const int i, 
       temporalalgebra::SecInterval& result) const {
  assert((i >= 0) && (i < GetNoComponents()));
  result.SetDefined(true);
  typename B::unitelem unit;
  units.Get(i, unit);
  result = unit.iv;
}

template<class B>
void MBasic<B>::GetInterval(temporalalgebra::SecInterval& result) const {
  if (this->IsDefined() && !this->IsEmpty()) {
    temporalalgebra::SecInterval first(true), last(true);
    this->GetInterval(0, first);
    this->GetInterval(this->GetNoComponents() - 1, last);
    result.Set(first.start, last.end, first.lc, last.rc);
  }
  else {
    result.SetDefined(false);
  }
}

/*
\subsection{Function ~GetBasic~}

*/
template<class B>
void MBasic<B>::GetBasic(const int i, B& result) const {
  assert((i >= 0) && (i < GetNoComponents()));
  result.SetDefined(IsDefined());
  if (IsDefined()) {
    typename B::base value;
    GetValue(i, value);
    result.SetValue(value);
  }
}

/*
\subsection{Function ~GetValue~}

*/
template<class B>
void MBasic<B>::GetValue(const int i, typename B::base& result) const {
  assert((i >= 0) && (i < GetNoComponents()));
  typename B::unitelem cur, next;
  units.Get(i, cur);
  unsigned int size;
  if (cur.pos != UINT_MAX) {
    if (i + 1 < GetNoComponents()) {
      units.Get(i + 1, next);
      if (next.pos != UINT_MAX) {
        size = next.pos - cur.pos;
      }
      else {
        size = GetNoChars() - cur.pos;
      }
    }
    else {
      size = GetNoChars() - cur.pos;
    }
    char* bytes = new char[size];
    values.read(bytes, size, cur.pos);
    std::string text(bytes, size);
    delete[] bytes;
    B::buildValue(text, cur, result);
  }
  else {
    B::buildValue("", cur, result);
  }
}

/*
\subsection{Function ~IsValid~}

*/
template<class B>
bool MBasic<B>::IsValid() const {
  if (!IsDefined() || IsEmpty()) {
    return true;
  }
  typename B::unitelem lastUnit, unit;
  units.Get(0, lastUnit);
  if (!lastUnit.iv.IsValid()) {
    return false;
  }
  if (GetNoComponents() == 1) {
    return true;
  }
  for (int i = 1; i < GetNoComponents(); i++) {
    units.Get(i, unit);
    if (!unit.iv.IsValid()) {
      return false;
    }
    if (lastUnit.iv.end > unit.iv.start) {
      return false;
    }
    if (!lastUnit.iv.Disjoint(unit.iv)) {
      return false;
    }
    lastUnit = unit;
  }
  return true;
}

/*
\subsection{Function ~EndBulkLoad~}

*/
template<class B>
void MBasic<B>::EndBulkLoad(const bool sort, const bool checkvalid) {
  if (!IsDefined()) {
    units.clean();
  }
  units.TrimToSize();
  values.resize(GetNoChars());
  assert(IsValid());
}

/*
\subsection{Function ~Add~}

*/
template<class B>
void MBasic<B>::Add(const UBasic<B>& ub) {
  assert(ub.IsDefined());
  assert(ub.IsValid());
  UBasic<B> ub2(ub);
  if (!IsDefined()) {
    return;
  }
  temporalalgebra::SecInterval iv;
  typename B::unitelem unit(GetNoChars(), ub.constValue.GetRef());
  unit.iv = ub.timeInterval;
  units.Append(unit);
  std::string text = ub.constValue.GetLabel();
  if (text.length() > 0) {
    const char *bytes = text.c_str();
    if (values.getSize() == 0) {
      values.resize(1024);
    }
    if (noChars + text.length() >= values.getSize()) {
      values.resize(2 * values.getSize());
    }
    values.write(bytes, text.length(), noChars);
    noChars += text.length();
  }
}

template<class B>
void MBasic<B>::Add(const temporalalgebra::SecInterval& iv, const B& value) {
  assert(iv.IsDefined() && value.IsDefined());
  B value2(value);
  ListExpr unitlist = nl->TwoElemList(iv.ToListExpr(nl->Empty()),
                                      value2.ToListExpr(nl->Empty()));
  if (!readUnitFrom(unitlist)) {
    SetDefined(false);
  }
}

/*
\subsection{Function ~MergeAdd~}

*/
template<class B>
void MBasic<B>::MergeAdd(const UBasic<B>& ub) {
  assert(IsDefined());
  if (!ub.IsDefined() || !ub.IsValid()) {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
      << " MergeAdd(Unit): Unit is undefined or invalid:";
    ub.Print(cout); cout << endl;
    assert(false);
  }
  if (!IsEmpty()) {
    bool extend = false;
    typename B::unitelem lastUnit;
    units.Get(GetNoComponents() - 1, lastUnit);
    if ((lastUnit.iv.end == ub.timeInterval.start) &&
        (lastUnit.iv.rc || ub.timeInterval.lc)) { // adjacent intervals
      B value1(true), value2(true);
      GetBasic(GetNoComponents() - 1, value1);
      value2 = ub.constValue;
      if (value1 == value2) {
        lastUnit.iv.end = ub.timeInterval.end; // extend last interval
        lastUnit.iv.rc = ub.timeInterval.rc;
        units.Put(GetNoComponents() - 1, lastUnit);
        extend = true;
      }
    }
    if (!extend && lastUnit.iv.Before(ub.timeInterval)) {
      Add(ub);
    }
  }
  else { // first unit
    Add(ub);
  }
}

/*
\subsection{Function ~Passes~}

*/
template<class B>
bool MBasic<B>::Passes(const B& basic) const {
  if (!IsDefined() || !basic.IsDefined()) {
    return false;
  }
  typename B::base value1, value2;
  basic.GetValue(value2);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetValue(i, value1);
    if (value1 == value2) {
      return true;
    }
  }
  return false;
}

template<class B>
template<class T>
bool MBasic<B>::Passes(const T& value) const {
  if (!IsDefined() || !value.IsDefined()) {
    return false;
  }
  Label lb(value.GetValue());
  return Passes(lb);
}

/*
\subsection{Function ~At~}

*/
template<class B>
void MBasic<B>::At(const B& basic, MBasic<B>& result) const {
  result.Clear();
  if (!IsDefined() || !basic.IsDefined()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  typename B::base value1, value2;
  basic.GetValue(value2);
  typename B::unitelem unit;
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  for (int i = 0; i < GetNoComponents(); i++) {
    GetValue(i, value1);
    if (value1 == value2) {
      units.Get(i, unit);
      result.readUnitFrom(nl->TwoElemList(unit.iv.ToListExpr(sc->GetTypeExpr(
                   temporalalgebra::SecInterval::BasicType())),
                                          B::getList(value1)));
    }
  }
}

template<class B>
template<class T>
void MBasic<B>::At(const T& value, MBasic<B>& result) const {
  Label lb(value.GetValue());
  At(lb, result);
}

/*
\subsection{Function ~DefTime~}

*/
template<class B>
void MBasic<B>::DefTime(temporalalgebra::Periods& per) const {
  per.Clear();
  per.SetDefined(IsDefined());
  if (IsDefined()) {
    typename B::unitelem unit;
    for (int i = 0; i < GetNoComponents(); i++) {
      units.Get(i, unit);
      per.MergeAdd(unit.iv);
    }
  }
}

/*
\subsection{Function ~AtInstant~}

*/
template<class B>
void MBasic<B>::AtInstant(const Instant& inst, IBasic<B>& result) const {
  if(!IsDefined() || !inst.IsDefined()) {
    result.SetDefined(false);
    return;
  }
  int pos = this->Position(inst);
  if (pos == -1) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.value.SetDefined(true);
  typename B::unitelem unit;
  units.Get(pos, unit);
  GetBasic(pos, result.value);
  result.instant = inst;
}

/*
\subsection{Function ~AtPeriods~}

*/
template<class B>
void MBasic<B>::AtPeriods(const temporalalgebra::Periods& per, 
                          MBasic<B>& result) const {
  result.Clear();
  result.SetDefined(IsDefined() && per.IsDefined());
  if (!IsDefined() || !per.IsDefined()) {
    return;
  }
  if (IsEmpty() || per.IsEmpty()) {
    return;
  }
  if (IsMaximumPeriods(per)) {
    result.CopyFrom(this);
    return;
  }
  assert(per.IsOrdered());
  temporalalgebra::SecInterval ivS(true), ivP(true), ivR(true);
  GetInterval(0, ivS);
  GetInterval(GetNoComponents() - 1, ivP);
  if (per.Before(ivS.start) || per.After(ivP.end)) {
    return;
  }
  result.StartBulkLoad();
  B val(true); // string | pair<string, unsigned int>
  int i = 0, j = 0;
  while ((i < GetNoComponents()) && (j < per.GetNoComponents())) {
    GetInterval(i, ivS); // get source interval
    per.Get(j, ivP); // get interval from periods
    GetBasic(i, val); // get source value
    if (ivS.Before(ivP)) {
      i++;
    }
    else if (ivS.After(ivP)) {
      j++;
    }
    else { // intervals overlap
      ivS.Intersection(ivP, ivR);
      ivR.SetDefined(true);
      result.Add(ivR, val);
      if (ivS.end == ivP.end) {
        if (ivS.rc == ivP.rc) {
          i++;
          j++;
        }
        else if (ivP.rc == true) {
          i++;
        }
        else {
          j++;
        }
      }
      else if (ivS.end < ivP.end) {
        i++;
      }
      else {
        j++;
      }
    }
  }
  result.EndBulkLoad(false);
}

/*
\subsection{Function ~Initial~}

*/
template<class B>
void MBasic<B>::Initial(IBasic<B>& result) const {
  if (!IsDefined() || IsEmpty()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  typename B::unitelem unit;
  units.Get(0, unit);
  GetBasic(0, result.value);
  result.instant = unit.iv.start;
}

/*
\subsection{Function ~Final~}

*/
template<class B>
void MBasic<B>::Final(IBasic<B>& result) const {
  if (!IsDefined() || IsEmpty()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  typename B::unitelem unit;
  units.Get(GetNoComponents() - 1, unit);
  GetBasic(GetNoComponents() - 1, result.value);
  result.instant = unit.iv.end;
}

/*
\subsection{Function ~InitialInstant~}

*/
template<class B>
void MBasic<B>::InitialInstant(Instant& result) const {
  if (!IsDefined() || IsEmpty()) {
    result.SetDefined(false);
    return;
  }
  typename B::unitelem unit;
  units.Get(0, unit);
  result = unit.iv.start;
}

/*
\subsection{Function ~FinalInstant~}

*/
template<class B>
void MBasic<B>::FinalInstant(Instant& result) const {
  if (!IsDefined() || IsEmpty()) {
    result.SetDefined(false);
    return;
  }
  typename B::unitelem unit;
  units.Get(GetNoComponents() - 1, unit);
  result = unit.iv.end;
}

/*
\subsection{Function ~Inside~}

*/
template<class B>
void MBasic<B>::Inside(const typename B::coll& coll, 
                       temporalalgebra::MBool& result) const {
  result.Clear();
  if (!IsDefined() || !coll.IsDefined()) {
    result.SetDefined(false);
    return;
  }
  typename B::unitelem unit;
  typename B::base value;
  for (int i = 0; i < GetNoComponents(); i++) {
    units.Get(i, unit);
    GetValue(i, value);
    CcBool res(true, coll.Contains(value));
    temporalalgebra::UBool ub(unit.iv, res);
    result.Add(ub);
  }
}

/*
\subsection{Function ~Fill~}

*/
template<class B>
void MBasic<B>::Fill(MBasic<B>& result, datetime::DateTime& dur) const {
  result.Clear();
  result.SetDefined(true);
  if (!IsDefined()) {
    result.SetDefined(false);
    return;
  }
  if (IsEmpty()) {
    return;
  }
  UBasic<B> unit(true), lastUnit(true);
  Get(0, lastUnit);
  for (int i = 1; i < GetNoComponents(); i++) {
    Get(i, unit);
    if ((lastUnit.constValue == unit.constValue) && 
        (unit.timeInterval.start - lastUnit.timeInterval.end <= dur)) {
      lastUnit.timeInterval.end = unit.timeInterval.end;
      lastUnit.timeInterval.rc = unit.timeInterval.rc;
    }
    else {
      result.MergeAdd(lastUnit);
      lastUnit = unit;
    }
  }
  result.MergeAdd(lastUnit);
}

/*
\subsection{Function ~concat~}

*/
template<class B>
void MBasic<B>::Concat(const MBasic<B>& src1, const MBasic<B>& src2) {
  Clear();
  if (src1.IsEmpty()) {
    CopyFrom(&src2);
    return;
  }
  if (src2.IsEmpty()) {
    CopyFrom(&src1);
    return;
  }
  temporalalgebra::SecInterval iv1, iv2;
  src1.GetInterval(src1.GetNoComponents() - 1, iv1);
  src2.GetInterval(0, iv2);
  SetDefined(src1.IsDefined() && src2.IsDefined() && iv1.Before(iv2));
  if (!IsDefined()) {
    return;
  }
  CopyFrom(&src1);
  UBasic<B> unit(true);
  for (int i = 0; i < src2.GetNoComponents(); i++) {
    src2.Get(i, unit);
    Add(unit);
  }
}

/*
\subsection{Function ~Compress~}

*/
template<class B>
void MBasic<B>::Compress(MBasic<B>& result) const {
  result.Clear();
  result.SetDefined(IsDefined());
  if(!IsDefined()) {
    return;
  }
  UBasic<B> ub(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    Get(i, ub);
    result.MergeAdd(ub);
  }
}

/*
\subsection{Function ~GetPart~}

*/
template<class B>
void MBasic<B>::GetPart(const int from, const int to, MBasic<B>& result) {
  assert(from >= 0);
  assert(to < GetNoComponents());
  assert(from <= to);
  result.SetDefined(true);
  result.Clear();
  UBasic<B> unit(true);
  for (int i = from; i <= to; i++) {
    Get(i, unit);
    result.MergeAdd(unit);
  }
}

#ifdef RECODE
/*
\subsection{Function ~Recode~}

*/
template<class B>
void MBasic<B>::Recode(const std::string& from, const std::string& to,
                       MBasic<B>& result) {
  result.SetDefined(IsDefined());
  if (!IsDefined()) {
    return;
  }
  result.Clear();
  typename B::base value, recoded;
  temporalalgebra::SecInterval iv(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetInterval(i, iv);
    GetValue(i, value);
    if (!RecodeFun::recode(value, from, to, recoded)) {
      result.SetDefined(false);
      return;
    }
    else {
      B val(recoded);
      result.Add(iv, val);
    }
  }
}
#endif

/*
\subsection{Function ~LongestCommonSubsequence~}

*/
template<class B>
NewPair<int, int> MBasic<B>::LongestCommonSubsequence(const MBasic<B>& mb) {
  NewPair<int, int> result(0, 0);
  if (IsEmpty() || mb.IsEmpty()) {
    return result;
  }
  int dp[GetNoComponents() + 1][mb.GetNoComponents() + 1];
  int lcsSize (0), maxPos(-1);
  typename B::base value1, value2;
  for (int i = 0; i <= GetNoComponents(); i++) {
    for (int j = 0; j <= mb.GetNoComponents(); j++) {
      if (i == 0 || j == 0) {
        dp[i][j] = 0;
      }
      else {
        GetValue(i - 1, value1);
        mb.GetValue(j - 1, value2);
        if (value1 == value2) {
          dp[i][j] = dp[i - 1][j - 1] + 1;
          if (dp[i][j] > lcsSize) {
            lcsSize = dp[i][j];
            maxPos = i;
          }
        }
        else {
          dp[i][j] = 0;
        }
      }
    }
  }
  result.first = maxPos - lcsSize;
  result.second = maxPos - 1;
  return result;
}

/*
\subsection{Function ~Print~}

*/
template<class B>
std::ostream& MBasic<B>::Print(std::ostream& os) const {
  if (!IsDefined()) {
    os << "(undefined)" << endl;
    return os;
  }
  os << BasicType() << ":" << endl;
  UBasic<B> ub(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    Get(i, ub);
    ub.Print(os);
  }
  return os;
}

/*
\subsection{Collection of Distance Functions}

*/
template<class B>
double MBasic<B>::Distance_FIRST(const MBasic<B>& mb) const {
  typename B::base b1, b2;
  GetValue(0, b1);
  mb.GetValue(0, b2);
  return (b1 == b2 ? 0.0 : 1.0);
}

template<class B>
double MBasic<B>::Distance_LAST(const MBasic<B>& mb) const {
  typename B::base b1, b2;
  GetValue(GetNoComponents() - 1, b1);
  mb.GetValue(mb.GetNoComponents() - 1, b2);
  return (b1 == b2 ? 0.0 : 1.0);
}

template<class B>
double MBasic<B>::Distance_FIRST_LAST(const MBasic<B>& mb, 
                                      const LabelFunction lf) const {
  int n = GetNoComponents();
  int m = mb.GetNoComponents();
  typename B::base s1, s2, e1, e2;
  GetValue(0, s1);
  mb.GetValue(0, s2);
  if (n == 1) {
    if (m == 1) {
      return BasicDistanceFuns::distance(s1, s2, lf);
    }
    else {
      mb.GetValue(m - 1, e2);
      return (BasicDistanceFuns::distance(s1, s2, lf) +
              BasicDistanceFuns::distance(s1, e2, lf)) / 2;
    }
  }
  else {
    GetValue(n - 1, e1);
    if (m == 1) {
      return (BasicDistanceFuns::distance(s1, s2, lf) + 
              BasicDistanceFuns::distance(e1, s2, lf)) / 2;
    }
    else {
      mb.GetValue(m - 1, e2);
      return (BasicDistanceFuns::distance(s1, s2, lf) + 
              BasicDistanceFuns::distance(e1, e2, lf)) / 2;
    }
  }
}

template<class B>
double MBasic<B>::Distance_ALL(const MBasic<B>& mb, const LabelFunction lf) 
                  const {
  int m = GetNoComponents();
  int n = mb.GetNoComponents();
  int dp[m + 1][n + 1];
  typename B::base b1, b2;
  for (int i = 0; i <= m; i++) {
    dp[i][0] = i;
  }
  for (int j = 0; j <= n; j++) {
    dp[0][j] = j;
  }
  for (int i = 1; i <= m; i++) {
    for (int j = 1; j <= n; j++) {
      GetValue(i - 1, b1);
      mb.GetValue(j - 1, b2);
      if (BasicDistanceFuns::distance(b1, b2, lf) == 0) {
        dp[i][j] = dp[i - 1][j - 1];
      }
      else {
        dp[i][j] = std::min(dp[i][j - 1], std::min(dp[i - 1][j], 
                                                   dp[i - 1][j - 1])) + 1;
      }
    }
  }
  return (double)(dp[m][n]) / std::max(m, n);
}

template<class B>
double MBasic<B>::Distance_ALL_DURATION(const MBasic<B>& mb, 
                                        const LabelFunction lf) const {
  double result = this->Distance_ALL(mb, lf);
  datetime::DateTime dur1(datetime::durationtype);
  datetime::DateTime dur2(datetime::durationtype);
  GetDuration(dur1);
  mb.GetDuration(dur2);
  double quotient = dur1 / dur2;
  if (quotient > 1) {
    quotient = dur2 / dur1;
  } // quotient is in (0, 1]
  return 0.5 * result + 0.5 * (1 - quotient);
}

template<class B>
double MBasic<B>::Distance_ALL_INTERVALS(const MBasic<B>& mb,
                                         const LabelFunction lf) const {
  return 0.0;  
}

template<class B>
double MBasic<B>::Distance_EQUAL_LABELS(const MBasic<B>& mb,
                                        const LabelFunction lf) const {
  if (GetNoComponents() != mb.GetNoComponents()) {
    return false;
  }
  typename B::base b1, b2;
  for (int i = 0; i < GetNoComponents(); i++) {
    GetValue(i, b1);
    mb.GetValue(i, b2);
    if (BasicDistanceFuns::distance(b1, b2, lf)) {
      return false;
    }
  }
  return true;
}

template<class B>
double MBasic<B>::Distance_COSINUS_SIM(const MBasic<B>& mb) {
  return 0.0;
  // TODO: a lot
}

template<class B>
double MBasic<B>::Distance_TF_IDF(const MBasic<B>& mb) {
  return 0.0;
  // TODO: a lot
}


/*
\subsection{Function ~Distance~}

*/
template<class B>
double MBasic<B>::Distance(const MBasic<B>& mb, 
                           const DistanceFunction df /* = ALL */, 
                           const LabelFunction lf /* = TRIVIAL */,
                           const double threshold /* = 1.0 */) const {
  if (!IsDefined() && !mb.IsDefined()) {
    return 0.0;
  }
  if (!IsDefined() || !mb.IsDefined()) {
    return 1.0;
  }
  if (IsEmpty() && mb.IsEmpty()) {
    return 0.0;
  }
  if (IsEmpty() || mb.IsEmpty()) {
    return 1.0;
  }
  switch (df) {
    case FIRST: {
      return this->Distance_FIRST(mb);
    }
    case LAST: {
      return this->Distance_LAST(mb);
    }
    case FIRST_LAST: {
      return this->Distance_FIRST_LAST(mb, lf);
    }
    case ALL: {
      return this->Distance_ALL(mb, lf);
    }
    case ALL_DURATION: {
      return this->Distance_ALL_DURATION(mb, lf);
    }
    case ALL_INTERVALS: {
      return this->Distance_ALL_INTERVALS(mb, lf);
    }
    case EQUAL_LABELS: {
      return this->Distance_EQUAL_LABELS(mb, lf);
    }
    default: {
      return -1.0;
    }
  }
}

/*
\subsection{Function ~CommonPrefixSuffix~}

*/
template<class B>
int MBasic<B>::CommonPrefixSuffix(const MBasic<B>& mb, const bool prefix) {
  typename B::base b1, b2;
  int result = 0;
  int minLength = std::min(GetNoComponents(), mb.GetNoComponents());
  if (prefix) {
    for (int i = 0; i < minLength; i++) {
      GetValue(i, b1);
      mb.GetValue(i, b2);
      if (b1 == b2) {
        result++;
      }
      else {
        return result;
      }
    }
  }
  else {
    for (int i = 1; i <= minLength; i++) {
      GetValue(GetNoComponents() - i, b1);
      mb.GetValue(mb.GetNoComponents() - i, b2);
      if (b1 == b2) {
        result++;
      }
      else {
        return result;
      }
    }
  }
  return result;
}

/*
\subsection{Function ~DistanceSym~}

*/
template<class B>
double MBasic<B>::DistanceSym(const MBasic<B>& mb, 
                              const DistanceFunSym distfun) {
  if (IsEmpty() && mb.IsEmpty()) {
    return 0.0;
  }
  typename B::base b1, b2;
  switch (distfun) {
    case EQUALLABELS: {
      if (GetNoComponents() != mb.GetNoComponents()) {
        return 1.0;
      }
      for (int i = 0; i < GetNoComponents(); i++) {
        GetValue(i, b1);
        mb.GetValue(i, b2);
        if (b1 != b2) {
          return 1.0;
        }
      }
      return 0.0;
    }
    case PREFIX: {
      int prefix = CommonPrefixSuffix(mb, true);
      if (prefix == std::min(GetNoComponents(), mb.GetNoComponents())) {
        return 0.0;
      }
      return (prefix == 0 ? 2.0 : 1.0 / prefix);
    }
    case SUFFIX: {
      int suffix = CommonPrefixSuffix(mb, false);
      if (suffix == std::min(GetNoComponents(), mb.GetNoComponents())) {
        return 0.0;
      }
      return (suffix == 0 ? 2.0 : 1.0 / suffix);
    }
    case PREFIXSUFFIX: {
      int prefix = CommonPrefixSuffix(mb, true);
      int suffix = CommonPrefixSuffix(mb, false);
      if (prefix+suffix >= std::min(GetNoComponents(), mb.GetNoComponents())) {
        return 0.0;
      }
      return (prefix + suffix == 0 ? 2.0 : 1.0 / (prefix + suffix));
    }
    default: {
      return -1.0;
    }
  }
}

template<class B>
void MBasic<B>::InsertLabels(std::vector<std::string>& result) const {
  B b(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetBasic(i, b);
    b.InsertLabels(result);
  }
}

template<class B>
void MBasic<B>::InsertLabels(std::set<std::string>& result) const {
  B b(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetBasic(i, b);
    b.InsertLabels(result);
  }
}

template<class B>
void MBasic<B>::FrequencyVector(InvertedFile& inv, std::vector<double>& fv,
                                const bool useIdf /* = false */) const {
  B b(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetBasic(i, b);
    b.UpdateFrequencies(inv, fv);
  }
  if (useIdf) {
    double noDocs = 0.0;
    InvertedFile::prefixIterator* pit = 0;
    TupleId id;
    uint32_t wc, cc;
    std::string dummy = "zzzzz";
    pit = inv.getPrefixIterator(dummy);
    if (pit->next(dummy, id, wc, cc)) { // retrieve noTuples from inv node
      noDocs = (double)id;
    }
    else {
      return;
    }
    delete pit;
    std::string emptyPrefix = "";
    pit = inv.getPrefixIterator(emptyPrefix);
    int pos = 0;
    int noEntries = inv.getNoEntries() - 1;
    while (pos < noEntries && pit->next(emptyPrefix, id, wc, cc)) {
      if (fv[pos] > 0.0) {
        fv[pos] *= log(noDocs / wc);
      }
      pos++;
    }
    delete pit;
  }
}

/*
\section{Implementation of class ~MBasics~}

\subsection{Constructors}

*/
template<class B>
MBasics<B>::MBasics(const MBasics &mbs) : Attribute(mbs.IsDefined()), 
  values(mbs.GetNoValues()), units(mbs.GetNoComponents()), pos(mbs.pos.Size()),
                                               noChars(mbs.GetNoChars()) {
  values.copyFrom(mbs.values);
  units.copyFrom(mbs.units);
  pos.copyFrom(mbs.pos);
}

/*
\subsection{Function ~Property~}

*/
template<class B>
ListExpr MBasics<B>::Property() {
  if (B::BasicType() == "labels") {
    return gentc::GenProperty("-> DATA", BasicType(),
      "((<interval> <labels>) (<interval> <labels>) ...)",
      "(((\"2014-01-29\" \"2014-01-30\" TRUE FALSE) (\"home\" \"Dortmund\")))");
  }
  if (B::BasicType() == "places") {
    return gentc::GenProperty("-> DATA", BasicType(),
      "((<interval> <places>) (<interval> <places>) ...)",
      "(((\"2014-01-29\" \"2014-01-30\" TRUE FALSE) ((\"home\" 2012) "
      "(\"Dortmund\" 1909))))");
  }
  return gentc::GenProperty("-> DATA", BasicType(), "Error: invalid type.", "");
}

/*
\subsection{Function ~CheckKind~}

*/
template<class B>
bool MBasics<B>::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, MBasics<B>::BasicType());
}

/*
\subsection{Function ~checkType~}

*/
template<class B>
bool MBasics<B>::checkType(ListExpr t) {
  return listutils::isSymbol(t, MBasics<B>::BasicType());
}

/*
\subsection{Function ~GetFLOB~}

*/
template<class B>
Flob* MBasics<B>::GetFLOB(const int i) {
  assert((i >= 0) && (i < NumOfFLOBs()));
  if (i == 0) {
    return &values;
  }
  if (i == 1) {
    return &units;
  }
  return &pos;
}

/*
\subsection{Function ~Compare~}

*/
template<class B>
int MBasics<B>::Compare(const Attribute* arg) const {
  if (GetNoComponents() == ((MBasics<B>*)arg)->GetNoComponents()) {
    if (GetNoValues() == ((MBasics<B>*)arg)->GetNoValues()) {
      return 0;
    }
    return (GetNoValues() > ((MBasics<B>*)arg)->GetNoValues() ? 1 : -1);
  }
  else {
    return (GetNoComponents() > ((MBasics<B>*)arg)->GetNoComponents() ? 
            1 : -1);
  }
}

/*
\subsection{Function ~Clone~}

*/
template<class B>
Attribute* MBasics<B>::Clone() const {
  return new (MBasics<B>)(*(MBasics<B>*)this);
}

/*
\subsection{Function ~=~}

*/
template<class B>
MBasics<B>& MBasics<B>::operator=(const MBasics<B>& src) {
  Attribute::operator=(src);
  units.copyFrom(src.units);
  values.copyFrom(src.values);
  pos.copyFrom(src.pos);
  noChars = src.noChars;
  return *this;
}

/*
\subsection{Function ~getEndPos~}

*/
template<class B>
int MBasics<B>::getUnitEndPos(const int i) const {
  if (i < GetNoComponents() - 1) {
    SymbolicUnit nextUnit;
    units.Get(i + 1, nextUnit);
    return nextUnit.pos - 1;
  }
  else { // last unit
    return pos.Size() - 1;
  }
}

/*
\subsection{Function ~unitToListExpr~}

*/
template<class B>
ListExpr MBasics<B>::unitToListExpr(int i) {
  assert ((i >= 0) && (i < GetNoComponents()));
  SymbolicUnit unit;
  units.Get(i, unit);
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  std::set<typename B::base> values;
  GetValues(i, values);
  ListExpr valuelist;
  B::valuesToListExpr(values, valuelist);
  return nl->TwoElemList(unit.iv.ToListExpr(sc->GetTypeExpr(
     temporalalgebra::SecInterval::BasicType())),
                         valuelist);
}

/*
\subsection{Function ~ToListExpr~}

*/
template<class B>
ListExpr MBasics<B>::ToListExpr(ListExpr typeInfo) {
  if (!IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  if (IsEmpty()) {
    return nl->Empty();
  }
  ListExpr result = nl->OneElemList(unitToListExpr(0));
  ListExpr last = result;
  for (int i = 1; i < GetNoComponents(); i++) {
    last = nl->Append(last, unitToListExpr(i));
  }
  return result;
}

/*
\subsection{Function ~readValues~}

*/
template<class B>
bool MBasics<B>::readValues(ListExpr valuelist) {
  ListExpr rest = valuelist;
  std::string text;
  typename B::arrayelem elem;
  while (!nl->IsEmpty(rest)) {
    if (listutils::isSymbolUndefined(nl->First(rest))) {
      return false;
    }
    B::getString(nl->First(rest), text);
    text = text.substr(1, text.length() - 2);
    unsigned int newPos = (text.length() > 0 ? GetNoChars() : UINT_MAX);
    B::getElemFromList(nl->First(rest), newPos, elem);
    pos.Append(elem);
    if (text.length() > 0) {
      const char *bytes = text.c_str();
      if (values.getSize() == 0) {
        values.resize(1024);
      }
      if (noChars + text.length() >= values.getSize()) {
        values.resize(2 * values.getSize());
      }
      values.write(bytes, text.length(), noChars);
      noChars += text.length();
    }
    rest = nl->Rest(rest);
  }
  return true;
}

/*
\subsection{Function ~readUnit~}

*/
template<class B>
bool MBasics<B>::readUnit(ListExpr unitlist) {
  if (!nl->HasLength(unitlist, 2)) {
    return false;
  }
  SymbolicUnit unit(GetNoValues(), 0);
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  if (!unit.iv.ReadFrom(nl->First(unitlist), 
           sc->GetTypeExpr(temporalalgebra::SecInterval::BasicType()))) {
    return false;
  }
  if (!readValues(nl->Second(unitlist))) {
    return false;
  }
  if (GetNoComponents() > 0) {
    SymbolicUnit prevUnit;
    units.Get(GetNoComponents() - 1, prevUnit);
    if (!(prevUnit.iv.Before(unit.iv))) { // check time intervals
      return false;
    }
  }
  units.Append(unit);
  return true;
}

/*
\subsection{Function ~ReadFrom~}

*/
template<class B>
bool MBasics<B>::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  values.clean();
  units.clean();
  pos.clean();
  if (listutils::isSymbolUndefined(LE)) {
    SetDefined(false);
    return true;
  }
  ListExpr rest = LE;
  while (!nl->IsEmpty(rest)) {
    if (!readUnit(nl->First(rest))) {
      SetDefined(false);
      return true;
    }
    rest = nl->Rest(rest);
  }
  SetDefined(true);
  return true;
}

/*
\subsection{Function ~serialize~}

*/
template<class B>
void MBasics<B>::serialize(size_t &size, char *&bytes) const {
  size = 0;
  bytes = 0;
  if (!IsDefined()) {
    return;
  }
  size_t rootSize = Sizeof();
  size = rootSize + sizeof(size_t) + values.getSize() + units.GetFlobSize()
         + pos.GetFlobSize();
  bytes = new char[size];
  memcpy(bytes, (void*)&rootSize, sizeof(size_t));
  memcpy(bytes + sizeof(size_t), (void*)this, rootSize);
  size_t offset = sizeof(size_t) + rootSize;
  char* data = values.getData();
  memcpy(bytes + offset, data, values.getSize());
  delete[] data;
  offset += values.getSize();
  data = units.getData();
  memcpy(bytes + offset, data, units.GetFlobSize());
  delete[] data;
  offset += units.GetFlobSize();
  data = pos.getData();
  memcpy(bytes + offset, data, pos.GetFlobSize());
  delete[] data;
}

/*
\subsection{Function ~deserialize~}

*/
template<class B>
MBasics<B>* MBasics<B>::deserialize(const char *bytes) {
  ListExpr typeExpr = nl->SymbolAtom(BasicType());
  int algebraId, typeId;
  std::string typeName;
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  sc->LookUpTypeExpr(typeExpr, typeName, algebraId, typeId);
  size_t rootSize;
  memcpy((void*)&rootSize, bytes, sizeof(size_t));
  char* root = new char[rootSize];
  memcpy(root, bytes + sizeof(size_t), rootSize);
  AlgebraManager* am = SecondoSystem::GetAlgebraManager();
  ListExpr nType = sc->NumericType(typeExpr);
  Attribute* attr = (Attribute*)(am->CreateObj(algebraId, typeId)(nType)).addr;
  attr = attr->Create(attr, root, rootSize, algebraId, typeId);
  delete[] root;
  size_t offset = rootSize + sizeof(size_t);
  for (int i = 0; i < attr->NumOfFLOBs(); i++) {
    Flob* flob = attr->GetFLOB(i);
    size_t size = flob->getSize();
    flob->kill();
    char* fb = new char[size];
    memcpy(fb, bytes + offset, size);
    flob->createFromBlock(*flob, fb, size, false);
    delete[] fb;
    offset += size;
  }
  return (MBasics<B>*)attr;
}

/*
\subsection{Operator ~<<~}

*/
template<class B>
std::ostream& operator<<(std::ostream& o, MBasics<B>& mbs) {
  ListExpr typeInfo = nl->Empty();
  o << nl->ToString(mbs.ToListExpr(typeInfo));
  return o;
}

/*
\subsection{Function ~Position~}

*/
template<class B>
int MBasics<B>::Position(const Instant& inst,
                         const bool ignoreClosedness /* = false */) const {
  assert(IsDefined());
  assert(inst.IsDefined());
  int first = 0, last = GetNoComponents() - 1;
  Instant t1 = inst;
  while (first <= last) {
    int mid = (first + last) / 2;
    if ((mid < 0) || (mid >= GetNoComponents())) {
      return -1;
    }
    SymbolicUnit unit;
    units.Get(mid, unit);
    if (((temporalalgebra::Interval<Instant>)(unit.iv)).Contains(t1, 
                                                            ignoreClosedness)) {
      return mid;
    }
    else { // not contained
      if ((t1 > unit.iv.end) || (t1 == unit.iv.end)) {
        first = mid + 1;
      }
      else if ((t1 < unit.iv.start) || (t1 == unit.iv.start)) {
        last = mid - 1;
      }
      else {
        return -1; // should never be reached.
      }
    }
  }
  return -1;
}

/*
\subsection{Function ~FirstPosFrom~}

*/
template<class B>
int MBasics<B>::FirstPosFrom(const Instant& inst) const {
  assert(IsDefined());
  assert(inst.IsDefined());
  typename B::unitelem unit;
  units.Get(0, unit);
  if (inst < ((temporalalgebra::Interval<Instant>)(unit.iv)).start) {
    return 0; //
  }
  units.Get(GetNoComponents() - 1, unit);
  if (inst > ((temporalalgebra::Interval<Instant>)(unit.iv)).end) {
    return -1;
  }
  int first(0), last(GetNoComponents() - 1);
  while (first <= last) {
    int mid = (first + last) / 2;
    units.Get(mid, unit);
    if (inst > ((temporalalgebra::Interval<Instant>)(unit.iv)).start) {
      if (inst < ((temporalalgebra::Interval<Instant>)(unit.iv)).end) {
        return mid;
      }
      first = mid + 1;
    }
    else if (inst < ((temporalalgebra::Interval<Instant>)(unit.iv)).start) {
      int beforeMid = mid - 1;
      units.Get(beforeMid, unit);
      if (inst > ((temporalalgebra::Interval<Instant>)(unit.iv)).start) {
        return mid;
      }
      else if (inst < ((temporalalgebra::Interval<Instant>)(unit.iv)).start) {
        last = mid - 1;
      }
      else {
        return mid;
      }
    }
    else {
      return mid;
    }
  }
  return -1;
}

/*
\subsection{Function ~LastPosUntil~}

*/
template<class B>
int MBasics<B>::LastPosUntil(const Instant& inst) const {
  assert(IsDefined());
  assert(inst.IsDefined());
  typename B::unitelem unit;
  units.Get(0, unit);
  if (inst < ((temporalalgebra::Interval<Instant>)(unit.iv)).start) {
    return -1; //
  }
  units.Get(GetNoComponents() - 1, unit);
  if (inst > ((temporalalgebra::Interval<Instant>)(unit.iv)).end) {
    return GetNoComponents() - 1;
  }
  int first(0), last(GetNoComponents() - 1);
  while (first <= last) {
    int mid = (first + last) / 2;
    units.Get(mid, unit);
    if (inst < ((temporalalgebra::Interval<Instant>)(unit.iv)).end) {
      if (inst > ((temporalalgebra::Interval<Instant>)(unit.iv)).start) {
        return mid;
      }
      last = mid - 1;
    }
    else if (inst > ((temporalalgebra::Interval<Instant>)(unit.iv)).end) {
      int afterMid = mid + 1;
      units.Get(afterMid, unit);
      if (inst < ((temporalalgebra::Interval<Instant>)(unit.iv)).end) {
        return mid;
      }
      else if (inst > ((temporalalgebra::Interval<Instant>)(unit.iv)).end) {
        first = mid + 1;
      }
      else {
        return mid;
      }
    }
    else {
      return mid;
    }
  }
  return -1;
}

/*
\subsection{Function ~Get~}

*/
template<class B>
void MBasics<B>::Get(const int i, UBasics<B>& result) const {
  assert(IsDefined());
  assert((i >= 0) && (i < GetNoComponents()));
  result.SetDefined(true);
  result.constValue.Clean();
  std::set<typename B::base> values;
  typename std::set<typename B::base>::iterator it;
  GetValues(i, values);
  for (it = values.begin(); it != values.end(); it++) {
    result.constValue.Append(*it);
  }
  temporalalgebra::SecInterval iv(true);
  GetInterval(i, iv);
  result.timeInterval = iv;
}

/*
\subsection{Function ~GetBasics~}

*/
template<class B>
void MBasics<B>::GetBasics(const int i, B& result) const {
  assert(IsDefined());
  assert((i >= 0) && (i < GetNoComponents()));
  result.SetDefined(true);
  result.Clean();
  std::set<typename B::base> values;
  typename std::set<typename B::base>::iterator it;
  GetValues(i, values);
  result.Append(values);
}

/*
\subsection{Function ~GetValues~}

*/
template<class B>
void MBasics<B>::GetValues(const int i, 
                           std::set<typename B::base>& result) const{
  assert (IsDefined() && (i >= 0) && (i < GetNoComponents()));
  result.clear();
  SymbolicUnit unit;
  units.Get(i, unit);
  typename B::base val;
  typename B::arrayelem elem1, elem2;
  std::pair<unsigned int, unsigned int> flobPos; // pos, size
  for (int j = unit.pos; j <= getUnitEndPos(i); j++) {
    flobPos = std::make_pair(0, 0);
    pos.Get(j, elem1);
    unsigned int start = B::getFlobPos(elem1);
    if (start != UINT_MAX) {
      int k = j + 1;
      bool finished = false;
      while (!finished && (k < GetNoValues())) {
        pos.Get(k, elem2);
        unsigned int next = B::getFlobPos(elem2);
        if (next != UINT_MAX) { // valid reference
          flobPos = std::make_pair(start, next - start);
          finished = true;
        }
        k++;
      }
      if (!finished) { // end of array
        flobPos = std::make_pair(start, GetNoChars() - start);
      }
    }
    if (flobPos.second > 0) {
      char *bytes = new char[flobPos.second];
      values.read(bytes, flobPos.second, flobPos.first);
      std::string text(bytes, flobPos.second);
      delete[] bytes;
      B::buildValue(text, elem1, val);
      result.insert(val);
    }
    else {
      B::buildValue("", elem1, val);
      result.insert(val);
    }
  }
}

/*
\subsection{Function ~GetInterval~}

*/
template<class B>
void MBasics<B>::GetInterval(const int i, 
                temporalalgebra::SecInterval& result) const {
  assert((i >= 0) && (i < GetNoComponents()));
  result.SetDefined(true);
  SymbolicUnit unit;
  units.Get(i, unit);
  result = unit.iv;
}

template<class B>
void MBasics<B>::GetInterval(temporalalgebra::SecInterval& result) const {
  if (this->IsDefined() && !this->IsEmpty()) {
    temporalalgebra::SecInterval first(true), last(true);
    this->GetInterval(0, first);
    this->GetInterval(this->GetNoComponents() - 1, last);
    result.Set(first.start, last.end, first.lc, last.rc);
  }
  else {
    result.SetDefined(false);
  }
}

/*
\subsection{Function ~Clear~}

*/
template<class B>
void MBasics<B>::Clear() {
  units.clean();
  values.clean();
  pos.clean();
  noChars = 0;
}

/*
\subsection{Function ~EndBulkLoad~}

*/
template<class B>
void MBasics<B>::EndBulkLoad(const bool sort /*= true*/, 
                             const bool checkvalid /*= true*/) {
  if (!IsDefined()) {
    units.clean();
    values.clean();
    pos.clean();
  }
  units.TrimToSize();
  pos.TrimToSize();
  values.resize(GetNoChars());
  assert(IsValid());
}

/*
\subsection{Function ~IsValid~}

*/
template<class B>
bool MBasics<B>::IsValid() const {
  if (!IsDefined() || IsEmpty()) {
    return true;
  }
  typename B::unitelem lastUnit, unit;
  units.Get(0, lastUnit);
  if (!lastUnit.iv.IsValid()) {
    return false;
  }
  if (GetNoComponents() == 1) {
    return true;
  }
  for (int i = 1; i < GetNoComponents(); i++) {
    units.Get(i, unit);
    if (!unit.iv.IsValid()) {
      return false;
    }
    if (lastUnit.iv.end > unit.iv.start) {
      return false;
    }
    if (!lastUnit.iv.Disjoint(unit.iv)) {
      return false;
    }
    lastUnit = unit;
  }
  return true;
}

/*
\subsection{Function ~Add~}

*/
template<class B>
void MBasics<B>::Add(const UBasics<B>& ut) {
  assert(IsDefined() && ut.IsDefined());
  UBasics<B> ut2(ut);
  if (!readUnit(ut2.ToListExpr(nl->Empty()))) {
    SetDefined(false);
  }
}

template<class B>
void MBasics<B>::Add(const temporalalgebra::SecInterval& iv, const B& values) {
  assert(IsDefined() && iv.IsDefined() && values.IsDefined());
  B values2(values);
  ListExpr unitlist = nl->TwoElemList(iv.ToListExpr(nl->Empty()),
                                      values2.ToListExpr(nl->Empty()));
  if (!readUnit(unitlist)) {
    SetDefined(false);
  }
  assert(IsValid());
}

/*
\subsection{Function ~MergeAdd~}

*/
template<class B>
void MBasics<B>::MergeAdd(const temporalalgebra::SecInterval& iv,
                          const B& values) {
  assert(IsDefined() && iv.IsDefined() && values.IsDefined());
  if (!iv.IsDefined() || !iv.IsValid()) {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
      << " MergeAdd(Unit): Unit is undefined or invalid:";
    iv.Print(cout); cout << endl;
    assert(false);
  }
  if (GetNoComponents() > 0) {
    SymbolicUnit prevUnit;
    units.Get(GetNoComponents() - 1, prevUnit);
    int numOfValues = getUnitEndPos(GetNoComponents() - 1) - prevUnit.pos + 1;
    bool equal = (values.GetNoValues() == numOfValues);
    std::set<typename B::base> mvalues, bvalues;
    typename std::set<typename B::base>::iterator it;
    GetValues(GetNoComponents() - 1, mvalues);
    values.GetValues(bvalues);
    equal = (mvalues == bvalues);
    if (equal && (prevUnit.iv.end == iv.start) && (prevUnit.iv.rc || iv.lc)) {
      prevUnit.iv.end = iv.end; // adjacent invervals \& equal labels
      prevUnit.iv.rc = iv.rc;
      units.Put(GetNoComponents() - 1, prevUnit);
    }
    else if (prevUnit.iv.Before(iv)) {
      Add(iv, values);
    }
  }
  else { // first unit
    Add(iv, values);
  }
}

template<class B>
void MBasics<B>::MergeAdd(const UBasics<B>& ub) {
  MergeAdd(ub.timeInterval, ub.constValue);
}

/*
\subsection{Function ~Passes~}

*/
template<class B>
bool MBasics<B>::Passes(const typename B::single& sg) const {
  std::set<typename B::base> vals;
  typename B::base val;
  sg.GetValue(val);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetValues(i, vals);
    if (vals.find(val) != vals.end()) {
      return true;
    }
  }
  return false;
}

template<class B>
template<class T>
bool MBasics<B>::Passes(const T& value) const {
  if (!IsDefined() || !value.IsDefined()) {
    return false;
  }
  Label lb(value.GetValue());
  return Passes(lb);
}

/*
\subsection{Function ~Passes~}

*/
template<class B>
bool MBasics<B>::Passes(const B& bs) const {
  B basics(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetBasics(i, basics);
    if (basics == bs) {
      return true;
    }
  }
  return false;
}

/*
\subsection{Function ~At~}

*/
template<class B>
void MBasics<B>::At(const typename B::single& sg, MBasics<B>& result) const {
  result.Clear();
  if (!IsDefined()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    B basics(true);
    GetBasics(i, basics);
    bool found = false;
    int j = 0;
    typename B::base val;
    while ((j < basics.GetNoValues()) && !found) {
      basics.GetValue(j, val);
      if (sg == val) {
        found = true;
      }
      j++;
    }
    if (found) {
      temporalalgebra::SecInterval iv;
      GetInterval(i, iv);
      UBasics<B> ut(iv, basics);
      result.Add(ut);
    }
  }
}

/*
\subsection{Function ~At~}

*/
template<class B>
void MBasics<B>::At(const B& bs, MBasics<B>& result) const {
  result.Clear();
  if (!IsDefined()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    B basics(true);
    GetBasics(i, basics);
    if (basics == bs) {
      temporalalgebra::SecInterval iv;
      GetInterval(i, iv);
      result.Add(iv, basics);
    }
  }
}

template<class B>
template<class T>
void MBasics<B>::At(const T& value, MBasics<B>& result) const {
  Label lb(value.GetValue());
  At(lb, result);
}


/*
\subsection{Function ~DefTime~}

*/
template<class B>
void MBasics<B>::DefTime(temporalalgebra::Periods& per) const {
  per.Clear();
  per.SetDefined(IsDefined());
  if (IsDefined()) {
    SymbolicUnit unit;
    for (int i = 0; i < GetNoComponents(); i++) {
      units.Get(i, unit);
      per.MergeAdd(unit.iv);
    }
  }
}

/*
\subsection{Function ~Atinstant~}

*/
template<class B>
void MBasics<B>::AtInstant(const Instant& inst, 
                           IBasics<B>& result) const {
  if(!IsDefined() || !inst.IsDefined()) {
    result.SetDefined(false);
    return;
  }
  int pos = Position(inst);
  if (pos == -1) {
    result.SetDefined(false);
  }
  else {
    GetBasics(pos, result.value);
    result.SetDefined(true);
    result.instant = inst;
  }
}

/*
\subsection{Function ~AtPeriods~}

*/
template<class B>
void MBasics<B>::AtPeriods(const temporalalgebra::Periods& per, 
                           MBasics<B>& result) const {
  result.Clear();
  result.SetDefined(IsDefined() && per.IsDefined());
  if (!IsDefined() || !per.IsDefined()) {
    return;
  }
  if (IsEmpty() || per.IsEmpty()) {
    return;
  }
  if (IsMaximumPeriods(per)) {
    result.CopyFrom(this);
    return;
  }
  assert(per.IsOrdered());
  temporalalgebra::SecInterval ivS(true), ivP(true), ivR(true);
  GetInterval(0, ivS);
  GetInterval(GetNoComponents() - 1, ivP);
  if (per.Before(ivS.start) || per.After(ivP.end)) {
    return;
  }
  result.StartBulkLoad();
  B vals;
  int i = 0, j = 0;
  while ((i < GetNoComponents()) && (j < per.GetNoComponents())) {
    GetInterval(i, ivS); // get source interval
    per.Get(j, ivP); // get interval from periods
    GetBasics(i, vals); // get source value
    if (ivS.Before(ivP)) {
      i++;
    }
    else if (ivS.After(ivP)) {
      j++;
    }
    else { // intervals overlap
      ivS.Intersection(ivP, ivR);
      result.Add(ivR, vals);
      if (ivS.end == ivP.end) {
        if (ivS.rc == ivP.rc) {
          i++;
          j++;
        }
        else if (ivP.rc == true) {
          i++;
        }
        else {
          j++;
        }
      }
      else if (ivS.end < ivP.end) {
        i++;
      }
      else {
        j++;
      }
    }
  }
  result.EndBulkLoad(false);
}

/*
\subsection{Function ~Initial~}

*/
template<class B>
void MBasics<B>::Initial(IBasics<B>& result) const {
  if (!IsDefined() || !GetNoComponents()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  temporalalgebra::SecInterval iv;
  GetInterval(0, iv);
  result.instant = iv.start;
  GetBasics(0, result.value);
}

/*
\subsection{Function ~Final~}

*/
template<class B>
void MBasics<B>::Final(IBasics<B>& result) const {
  if (!IsDefined() || !GetNoComponents()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  temporalalgebra::SecInterval iv;
  GetInterval(GetNoComponents() - 1, iv);
  result.instant = iv.end;
  GetBasics(GetNoComponents() - 1, result.value);
}

/*
\subsection{Function ~InitialInstant~}

*/
template<class B>
void MBasics<B>::InitialInstant(Instant& result) const {
  if (!IsDefined() || IsEmpty()) {
    result.SetDefined(false);
    return;
  }
  typename B::unitelem unit;
  units.Get(0, unit);
  result = unit.iv.start;
}

/*
\subsection{Function ~FinalInstant~}

*/
template<class B>
void MBasics<B>::FinalInstant(Instant& result) const {
  if (!IsDefined() || IsEmpty()) {
    result.SetDefined(false);
    return;
  }
  typename B::unitelem unit;
  units.Get(GetNoComponents() - 1, unit);
  result = unit.iv.end;
}

/*
\subsection{Function ~Fill~}

*/
template<class B>
void MBasics<B>::Fill(MBasics<B>& result, datetime::DateTime& dur) const {
  if (!IsDefined()) {
    result.SetDefined(false);
    return;
  }
  result.Clear();
  result.SetDefined(true);
  if (IsEmpty()) {
    return;
  }
  UBasics<B> unit(true), lastUnit(true);
  Get(0, lastUnit);
  for (int i = 1; i < GetNoComponents(); i++) {
    Get(i, unit);
    if ((lastUnit.constValue == unit.constValue) && 
        (unit.timeInterval.start - lastUnit.timeInterval.end <= dur)) {
      lastUnit.timeInterval.end = unit.timeInterval.end;
      lastUnit.timeInterval.rc = unit.timeInterval.rc;
    }
    else {
      result.MergeAdd(lastUnit);
      lastUnit = unit;
    }
  }
  result.MergeAdd(lastUnit);
}

/*
\subsection{Function ~concat~}

*/
template<class B>
void MBasics<B>::Concat(const MBasics<B>& src1, const MBasics<B>& src2) {
  Clear();
  if (src1.IsEmpty()) {
    CopyFrom(&src2);
    return;
  }
  if (src2.IsEmpty()) {
    CopyFrom(&src1);
    return;
  }
  temporalalgebra::SecInterval iv1, iv2;
  src1.GetInterval(src1.GetNoComponents() - 1, iv1);
  src2.GetInterval(0, iv2);
  SetDefined(src1.IsDefined() && src2.IsDefined() && iv1.Before(iv2));
  if (!IsDefined()) {
    return;
  }
  CopyFrom(&src1);
  UBasics<B> unit(true);
  for (int i = 0; i < src2.GetNoComponents(); i++) {
    src2.Get(i, unit);
    Add(unit);
  }
}

/*
\subsection{Function ~Compress~}

*/
template<class B>
void MBasics<B>::Compress(MBasics<B>& result) const {
  result.Clear();
  result.SetDefined(IsDefined());
  if(!IsDefined()) {
    return;
  }
  UBasics<B> ub(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    Get(i, ub);
    result.MergeAdd(ub);
  }
}

#ifdef RECODE
/*
\subsection{Function ~Recode~}

*/
template<class B>
void MBasics<B>::Recode(const std::string& from, const std::string& to,
                        MBasics<B>& result) {
  result.SetDefined(IsDefined());
  if (!IsDefined()) {
    return;
  }
  result.Clear();
  B values(true), recoded_values(true);
  temporalalgebra::SecInterval iv(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetInterval(i, iv);
    GetBasics(i, values);
    if (!values.Recode(from, to, recoded_values)) {
      result.SetDefined(false);
      return;
    }
    result.Add(iv, recoded_values);
  }
}
#endif

/*
\subsection{Function ~Print~}

*/
template<class B>
std::ostream& MBasics<B>::Print(std::ostream& os) const {
  if (!IsDefined()) {
    os << "(undefined)" << endl;
    return os;
  }
  os << BasicType() << ":" << endl;
  UBasics<B> ubs(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    Get(i, ubs);
    ubs.Print(os);
  }
  return os;
}

/*
\subsection{Function ~DISTANCE\_FIRST\_LAST~}

*/
template<class B>
double MBasics<B>::Distance_FIRST_LAST(const MBasics<B>& mbs, 
                                       const LabelFunction lf) const {
  int n = GetNoComponents();
  int m = mbs.GetNoComponents();
  std::set<typename B::base> bs1, bs2, be1, be2;
  GetValues(0, bs1);
  mbs.GetValues(0, bs2);
//   GetValue(n - 1, be1);
//   mbs.GetValue(m - 1, be2);
  if (n == 1) {
    if (m == 1) {
      return BasicDistanceFuns::distance(bs1, bs2, lf);
    }
    mbs.GetValues(m - 1, be2);
    return (BasicDistanceFuns::distance(bs1, bs2, lf) +
            BasicDistanceFuns::distance(bs1, be2, lf)) / 2;
  }
  else {
    GetValues(n - 1, be1);
    if (m == 1) {
      return (BasicDistanceFuns::distance(bs1, bs2, lf) + 
              BasicDistanceFuns::distance(be1, bs2, lf)) /2;
    }
    mbs.GetValues(m - 1, be2);
    return (BasicDistanceFuns::distance(bs1, bs2, lf) +
            BasicDistanceFuns::distance(be1, be2, lf)) / 2;
  }
}

/*
\subsection{Function ~DISTANCE\_ALL~}

*/
template<class B>
double MBasics<B>::Distance_ALL(const MBasics<B>& mbs, const LabelFunction lf) 
                   const {
  int m = GetNoComponents();
  int n = mbs.GetNoComponents();
  int dp[m + 1][n + 1];
  std::set<typename B::base> b1, b2;
  double unitdist;
  for (int i = 0; i <= m; i++) {
    dp[i][0] = i;
  }
  for (int j = 0; j <= n; j++) {
    dp[0][j] = j;
  }
  for (int i = 1; i <= m; i++) {
    for (int j = 1; j <= n; j++) {
      GetValues(i - 1, b1);
      mbs.GetValues(j - 1, b2);
      unitdist = BasicDistanceFuns::distance(b1, b2, lf);
      if (unitdist == 0) {
        dp[i][j] = dp[i - 1][j - 1];
      }
      else {
        dp[i][j] = std::min(dp[i][j - 1], std::min(dp[i - 1][j], 
                                                  dp[i - 1][j - 1])) + unitdist;
      }
    }
  }
  return (double)(dp[m][n]) / std::max(m, n);
}

/*
\subsection{Function ~Distance~}

*/
template<class B>
double MBasics<B>::Distance(const MBasics<B>& mbs, 
                            const DistanceFunction df /* = ALL */, 
                            const LabelFunction lf /* = TRIVIAL */,
                            const double threshold /* = 1.0 */) const {
  if (!IsDefined() && !mbs.IsDefined()) {
    return 0.0;
  }
  if (!IsDefined() || !mbs.IsDefined()) {
    return 1.0;
  }
  if (IsEmpty() && mbs.IsEmpty()) {
    return 0.0;
  }
  if (IsEmpty() || mbs.IsEmpty()) {
    return 1.0;
  }
  switch (df) {
//     case FIRST: {
//       return this->Distance_FIRST(mbs);
//     }
//     case LAST: {
//       return this->Distance_LAST(mbs);
//     }
    case FIRST_LAST: {
      return this->Distance_FIRST_LAST(mbs, lf);
    }
    case ALL: {
      return this->Distance_ALL(mbs, lf);
    }
//     case ALL_DURATION: {
//       return this->Distance_ALL_DURATION(mbs, lf);
//     }
//     case ALL_INTERVALS: {
//       return this->Distance_ALL_INTERVALS(mbs, lf);
//     }
//     case EQUAL_LABELS: {
//       return this->Distance_EQUAL_LABELS(mbs, lf);
//     }
    default: {
      return -1.0;
    }
  }
}

/*
\subsection{Function ~CommonPrefixSuffix~}

*/
template<class B>
int MBasics<B>::CommonPrefixSuffix(const MBasics<B>& mbs, const bool prefix) {
  std::set<typename B::base> b1, b2;
  int result = 0;
  int minLength = std::min(GetNoComponents(), mbs.GetNoComponents());
  for (int i = 0; i < minLength; i++) {
    GetValues(i, b1);
    mbs.GetValues(i, b2);
    if (b1 == b2) {
      result++;
    }
    else {
      return result;
    }
  }
  return result;
}

/*
\subsection{Function ~DistanceSym~}

*/
template<class B>
double MBasics<B>::DistanceSym(const MBasics<B>& mbs, 
                               const DistanceFunSym distfun) {
  if (IsEmpty() && mbs.IsEmpty()) {
    return 0.0;
  }
  std::set<typename B::base> b1, b2;
  switch (distfun) {
    case EQUALLABELS: {
      if (GetNoComponents() != mbs.GetNoComponents()) {
        return 1.0;
      }
      for (int i = 0; i < GetNoComponents(); i++) {
        GetValues(i, b1);
        mbs.GetValues(i, b2);
        if (b1 != b2) {
          return 1.0;
        }
      }
      return 0.0;
    }
    case PREFIX: {
      int prefix = CommonPrefixSuffix(mbs, true);
      if (prefix == std::min(GetNoComponents(), mbs.GetNoComponents())) {
        return 0.0;
      }
      return (prefix == 0 ? 2.0 : 1.0 / prefix);
    }
    case SUFFIX: {
      int suffix = CommonPrefixSuffix(mbs, false);
      if (suffix == std::min(GetNoComponents(), mbs.GetNoComponents())) {
        return 0.0;
      }
      return (suffix == 0 ? 2.0 : 1.0 / suffix);
    }
    case PREFIXSUFFIX: {
      int prefix = CommonPrefixSuffix(mbs, true);
      int suffix = CommonPrefixSuffix(mbs, false);
      if (prefix+suffix >= std::min(GetNoComponents(), mbs.GetNoComponents())) {
        return 0.0;
      }
      return (prefix + suffix == 0 ? 2.0 : 1.0 / (prefix + suffix));
    }
    default: {
      return -1.0;
    }
  }
}

template<class B>
void MBasics<B>::InsertLabels(std::vector<std::string>& result) const {
  B b(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetBasics(i, b);
    b.InsertLabels(result);
  }
}

template<class B>
void MBasics<B>::InsertLabels(std::set<std::string>& result) const {
  B b(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetBasics(i, b);
    b.InsertLabels(result);
  }
}

template<class B>
void MBasics<B>::FrequencyVector(InvertedFile& inv, std::vector<double>& fv,
                                 const bool useIdf /* = false */) const {
  B b(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetBasics(i, b);
    b.UpdateFrequencies(inv, fv);
  }
  if (useIdf) {
    double noDocs = 0.0;
    InvertedFile::prefixIterator* pit = 0;
    TupleId id;
    uint32_t wc, cc;
    std::string dummy = "zzzzz";
    pit = inv.getPrefixIterator(dummy);
    if (pit->next(dummy, id, wc, cc)) { // retrieve noTuples from inv node
      noDocs = (double)id;
    }
    else {
      return;
    }
    delete pit;
    std::string emptyPrefix = "";
    pit = inv.getPrefixIterator(emptyPrefix);
    int pos = 0;
    int noEntries = inv.getNoEntries() - 1;
    while (pos < noEntries && pit->next(emptyPrefix, id, wc, cc)) {
      if (fv[pos] > 0.0) {
        fv[pos] *= log(noDocs / wc);
      }
      pos++;
    }
    delete pit;
  }
}

/*
\section{Class ~UnitsLI~}

*/
class UnitsLI {
 public:
  UnitsLI() : index(0) {}
  ~UnitsLI() {}

  int index;
};

}
