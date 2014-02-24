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

Started March 2012, Fabio Vald\'{e}s

Some basic implementations were done by Frank Panse.

[TOC]

\section{Overview}
This is the header file for the Symbolic Trajectory Algebra.

\section{Defines and Includes}

*/
#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include "TemporalAlgebra.h"
#include "TemporalExtAlgebra.h"
#include "DateTime.h"
#include "CharTransform.h"
#include "Stream.h"
#include "SecParser.h"
#include "NestedList.h"
#include "SymbolicTrajectoryTools.h"
#include "ListUtils.h"
#include "RelationAlgebra.h"
#include "Stream.h"
#include "InvertedFile.h"
#include "FTextAlgebra.h"
#include "IntNfa.h"
#include "TemporalUnitAlgebra.h"
#include "GenericTC.h"
#include <string>
#include <set>
#include <stack>
#include <vector>
#include <math.h>
#include <time.h>

#define YYDEBUG 1
#define YYERROR_VERBOSE 1

using namespace std;

union Word;

int parsePatternRegEx(const char* argument, IntNfa** T);

namespace stj {

class Pattern;
class PatElem;
class Assign;
class ClassifyLI;
class IndexLI;
struct LabelValue;
struct PlaceValue;
class ILabels;
class ULabels;
class MLabels;

enum ExtBool {FALSE, TRUE, UNDEF};
enum Wildcard {NO, STAR, PLUS};

Pattern* parseString(const char* input, bool classify);
void patternFlushBuffer();

struct LabelValue {
  ListExpr ToListExpr() const {return nl->StringAtom(string(v));}
  bool operator==(const LabelValue& lv) const {return string(v) ==string(lv.v);}
  
  char v[MAX_STRINGSIZE + 1];
};

struct PlaceValue{
  ListExpr ToListExpr() const {return nl->TwoElemList(name.ToListExpr(), 
                                      nl->IntAtom(ref));}
  bool operator==(const PlaceValue& pv) const {return (name == pv.name) &&
                                                      (ref == pv.ref);}
  
  LabelValue name;
  unsigned int ref;
};

/*
\section{Class ~Label~}

*/
class Label : public Attribute {
 public:
  friend class Labels;
  friend class MLabels;
   
  Label() {}
  explicit Label(const string& val);
  Label(const Label& rhs);
  Label(const bool def);
  
  ~Label() {}

  string GetValue() const {string value = val.v; return value;}
  void Set(const bool def, const string &value);
//   Label* Clone() const {return new Label(*this);}
  bool operator==(const Label lb) const {return GetValue() == lb.GetValue();}
  bool operator==(const LabelValue& lv) const;
  void ToBase(LabelValue &lv) const {lv = val;}

  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  ListExpr ToListExpr(ListExpr typeInfo);
  static bool     CheckKind(ListExpr type, ListExpr& errorInfo);
  static int      SizeOfObj();
  static ListExpr Property();
  static const string BasicType() {return "label";}
  static const bool checkType(const ListExpr type);
  void            CopyFrom(const Attribute* right);
  int             Compare(const Attribute* arg) const;
  size_t          Sizeof() const {return sizeof(*this);}
  bool            Adjacent(const Attribute*) const {return false;}
  Attribute*      Clone() const {return new Label(*this);}
  size_t          HashValue() const {return val.v[0];}
  void            SetValue(const string &value);
  ostream&        Print(ostream& os) const {return os << GetValue();}
  
 protected:
  LabelValue val;
};

int CompareLabels(const void *a, const void *b);

/*
\section{Class ~Labels~}

*/
class Labels : public Attribute {
 public:
  typedef LabelValue base;
  typedef Label single;
  typedef ILabels itype;
  typedef ULabels utype;
  typedef MLabels mtype;
   
  friend class MLabels;
   
  Labels() {}
  explicit Labels(const int n, const Label *Lb = 0);
  explicit Labels(const bool defined) : Attribute(defined), values(0) {}
  explicit Labels(vector<Label>* lbs);
  Labels(const Labels& src, const bool sort = false);
  
  ~Labels() {}

  Labels& operator=(const Labels& src);
  bool operator==(const Labels& src) const;
  int NumOfFLOBs() const {return 1;}
  Flob *GetFLOB(const int i);
  int Compare(const Attribute*) const;
  bool Adjacent(const Attribute*) const {return false;}
  Labels *Clone() const;
  size_t Sizeof() const {return sizeof(*this);}
  ostream& Print(ostream& os) const {return (os << *this);}

  void Append(const Label &lb) {values.Append(lb.val);}
  void Append(const LabelValue& lv) {values.Append(lv);}
  void Destroy() {values.destroy();}
  int GetNoValues() const {return values.Size();}
  void GetValue(int i, Label& lb) const;
  void GetBasicValue(int i, LabelValue& lv) const;
  const bool IsEmpty() const {return GetNoValues() == 0;}
  void CopyFrom(const Attribute* right) {*this = *((Labels*)right);}
  size_t HashValue() const;

  friend ostream& operator <<( ostream& os, const Labels& p );

  static Word     In(const ListExpr typeInfo, const ListExpr instance,
                    const int errorPos, ListExpr& errorInfo, bool& correct);
  static ListExpr Out(ListExpr typeInfo, Word value);
  static Word     Create(const ListExpr typeInfo);
  static void     Delete(const ListExpr typeInfo, Word& w);
  static void     Close(const ListExpr typeInfo, Word& w);
  static bool     Save(SmiRecord& valueRecord, size_t& offset,
                       const ListExpr typeInfo, Word& value);
  static bool     Open(SmiRecord& valueRecord, size_t& offset,
                       const ListExpr typeInfo, Word& value);
  static Word     Clone(const ListExpr typeInfo, const Word& w);
  static bool     KindCheck(ListExpr type, ListExpr& errorInfo);
  static int      SizeOfObj() {return sizeof(Labels);}
  static ListExpr Property();
  static void*    Cast(void* addr) {return (new (addr)Labels);}
  static const    string BasicType() {return Label::BasicType() + "s";}
  static const    bool checkType(const ListExpr type) {
                                 return listutils::isSymbol(type, BasicType());}
  void Sort() {values.Sort(CompareLabels);}
  bool Find(Label& lb) {int p; return values.Find(&(lb.val), CompareLabels, p);}
  void Clean() {if (values.Size()) {values.clean();}}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool Contains(Label& lb) const;

 private:
  DbArray<LabelValue> values;
};

/*
\section{Class ~ILabel~}

*/
class ILabel : public IString {
public:
  static const string BasicType() {return "ilabel";}
  static ListExpr IntimeLabelProperty();
  static bool CheckIntimeLabel(ListExpr type, ListExpr& errorInfo);
  static bool checkType(ListExpr t) {
    return listutils::isSymbol(t, BasicType());
  }
  
  void Val(Label& result) const;
};

/*
\section{Class ~ULabel~}

*/
class ULabel : public UString {
 public:
  ULabel() {}
  explicit ULabel(int i): UString(i) {}
  ULabel(const Interval<Instant>& interval, const Label& label);

  static const string BasicType() {return "ulabel";}
  static ListExpr ULabelProperty();
  static bool CheckULabel(ListExpr type, ListExpr& errorInfo);
  static bool checkType(ListExpr t) {
    return listutils::isSymbol(t, BasicType());
  }

  void Initial(ILabel& result) const;
  void Final(ILabel& result) const;
};

/*
\section{Class ~MLabel~}

*/
class MLabel : public MString {
 public:
  MLabel() {}
  explicit MLabel(int i): MString(i) {}
  explicit MLabel(MString* ms);
  explicit MLabel(MLabel* ml);
  MLabel(const MLabel &ml);
  
  ~MLabel() {}

  static const string BasicType() {return "mlabel";}
  static bool checkType(ListExpr t) {
    return listutils::isSymbol(t, BasicType());
  }
  static ListExpr MLabelProperty();
  static bool CheckMLabel(ListExpr type, ListExpr& errorInfo);

  Word Create(const ListExpr typeInfo);
  void Close(const ListExpr typeInfo, Word& w);
  MLabel* Clone() const;
  int Compare(const Attribute * arg) const;
  void CopyFrom(const Attribute* right);
  int NumOfFLOBs() const;
  Flob* GetFLOB(const int i);
  size_t Sizeof() const{return sizeof(*this);}

  void Initialize() {}
  void Finalize() {}
  
  MLabel* compress();
  void createML(int size, bool text, double rate);
  void convertFromMString(MString* source);
  MLabel* rewrite(map<string, pair<unsigned int, unsigned int> > binding,
                  vector<Assign> &assigns) const;
  bool Passes(const Label& label) const;
  void At(const Label& label, MLabel& result) const;
  void DefTime(Periods& per) const;
  void Atinstant(const Instant& inst, ILabel& result) const;
  void Inside(const Labels& lbs, MBool& result) const;
  void Initial(ILabel& result) const;
  void Final(ILabel& result) const;
};

/*
\section{Class ~ILabels~}

*/
class ILabels : public Intime<Labels> {
 public:
  ILabels() {}
  explicit ILabels(const bool defined);
  explicit ILabels(const ILabels& ils);
  ILabels(const Instant &inst, const Labels &lbs);
  
  static ListExpr Property();
  static Word Create(const ListExpr typeInfo);
  static void Delete(const ListExpr typeInfo, Word &w);
  static void Close(const ListExpr typeInfo, Word &w);
  static Word Clone(const ListExpr typeInfo, const Word& w);
  static void* Cast(void* addr) {return (new (addr)ILabels);}
  static int SizeOfObj() {return sizeof(ILabels);}
  static bool KindCheck(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "i" + Labels::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return value.NumOfFLOBs();}
  Flob* GetFLOB(const int i) {return value.GetFLOB(i);}
  
  void Val(Labels& result) const;
};

/*
\section{Class ~ULabels~}

*/
class ULabels : public ConstTemporalUnit<Labels> {
 private:
  friend class MLabels;
  
 public:
  ULabels() {}
  explicit ULabels(bool defined);
  explicit ULabels(const SecInterval &iv, const Labels &lbs);
  ULabels(const ULabels& uls);
  ULabels(int i, MLabels &mls);
  
  static ListExpr Property();
  static Word Create(const ListExpr typeInfo);
  static void Delete(const ListExpr typeInfo, Word &w);
  static void Close(const ListExpr typeInfo, Word &w);
  static Word Clone(const ListExpr typeInfo, const Word& w);
  static void* Cast(void* addr) {return (new (addr)ULabels);}
  static int SizeOfObj() {return sizeof(ULabels);}
  static bool KindCheck(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "u" + Labels::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return constValue.NumOfFLOBs();}
  Flob* GetFLOB(const int i) {return constValue.GetFLOB(i);}
  
  ULabels* Clone() const {return new ULabels(*this);}
  void Initial(ILabels& result) const;
  void Final(ILabels& result) const;
};

class SymbolicUnit {
 public: 
  SymbolicUnit() {}
  SymbolicUnit(int pos) : interval(true), startPos(pos) {}
  ~SymbolicUnit() {}
  
  SecInterval interval;
  int startPos;
};

/*
\section{Class ~MLabels~}

*/
template<class B>
class MBasics : public Attribute {
 private:
  DbArray<SymbolicUnit> units;
  DbArray<typename B::base> values;
  
 public:
  MBasics() {}
  explicit MBasics(int n) : Attribute(n > 0), units(n), values(0) {}
  explicit MBasics(const MBasics& mbs);
  
  ~MBasics() {}
  
  static ListExpr Property();
  static int SizeOfObj() {return sizeof(B::mtype);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "m" + B::BasicType();}
  static bool checkType(ListExpr t);
  int NumOfFLOBs() const {return 2;}
  Flob* GetFLOB(const int i);
  size_t Sizeof() const {return sizeof(*this);}
  int Compare(const Attribute* arg) const;
  bool Adjacent(const Attribute *arg) const {return false;}
  Attribute *Clone() const;
  size_t HashValue() const {return values.Size() * units.Size();}
  void CopyFrom(const Attribute* right) {*this = *((typename B::mtype*)right);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  void Destroy() {units.destroy(); values.destroy();}
  
  int getEndPos(int i) const;
  ListExpr valuesToListExpr(int start, int end);
  ListExpr unitToListExpr(int i);
  void readValues(ListExpr valuelist);
  bool readUnit(ListExpr unitlist);
  int Position(const Instant& inst) const;
  
  void Get(const int i, typename B::utype& result) const;
  bool IsEmpty() const {return units.Size() == 0;}
  void GetValues(const int i, B& result) const;
  SecInterval GetInterval(int i) const;
  int GetNoComponents() const {return units.Size();}
  int GetNoValues() const {return values.Size();}
  void Clear();
  void StartBulkLoad() {assert(IsDefined());}
  void EndBulkLoad(const bool sort = true, const bool checkvalid = true);
  void Add(const typename B::utype& ut);
  void MergeAdd(const typename B::utype& ut);
  bool Passes(const typename B::single& sg) const;
  bool Passes(const B& bs) const;
  void At(const typename B::single& sg, typename B::mtype& result) const;
  void At(const B& bs, typename B::mtype& result) const;
  void DefTime(Periods& per) const;
  void Atinstant(const Instant& inst, typename B::itype& result) const;
  void Initial(typename B::itype& result) const;
  void Final(typename B::itype& result) const;
};

class MLabels : public MBasics<Labels> {
 public: 
  MLabels() {}
  explicit MLabels(int n) : MBasics<Labels>(n) {}
  explicit MLabels(const MLabels& mls) : MBasics<Labels>(mls) {}
  
  ~MLabels() {}
  
  static ListExpr Property();
};

/*
\section{Class ~Place~}

*/
class Place : public Label {
  friend class Places;
  
 public: 
  Place() : Label() {}
  explicit Place(const string& n, const unsigned int r) : Label(n), ref(r) {}
  Place(const Place& rhs) : Label(rhs.GetName()), ref(rhs.GetRef()) {}
  explicit Place(const bool def) : Label(def), ref(0) {}

  ~Place() {}

  void Set(PlaceValue pv) {val = pv.name; ref = pv.ref;}
  string GetName() const {string text = val.v; return text;}
  unsigned int GetRef() const {return ref;}
  void SetName(const string& n) {Label::Set(IsDefined(), n);}
  void SetRef(const unsigned int r) {ref = r;}
  bool operator==(const Place& p) const;
  string toString() {return nl->ToString(ToListExpr(nl->Empty()));}

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(Place);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "place";}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return 0;}
  Flob* GetFLOB(const int i) {return 0;}
  size_t Sizeof() const {return sizeof(*this);}
  int Compare(const Attribute* arg) const;
  bool Adjacent(const Attribute *arg) const {return false;}
  Attribute *Clone() const {return new Place(*this);}
  size_t HashValue() const {return val.v[0] * val.v[9] * ref;}
  virtual void CopyFrom(const Attribute* right) {*this = *((Place*)right);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);

 private:
  unsigned int ref;
};

/*
\section{Class ~IPlace~}

*/
class IPlace : public Intime<Place> {
 public:
  IPlace() {}
  explicit IPlace(const Instant& inst, const Place& pl);
  explicit IPlace(const IPlace& rhs);
  IPlace(const bool def) : Intime<Place>(def) {}

  ~IPlace() {}

  string GetName() const {return value.GetName();}
  unsigned int GetRef() const {return value.GetRef();}
  void SetRef(const unsigned int r) {value.SetRef(r);}
  bool operator==(const IPlace& rhs) const;
  string toString() {return nl->ToString(ToListExpr(nl->Empty()));}

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(IPlace);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "i" + Place::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return 0;}
  Flob* GetFLOB(const int i) {return 0;}
  size_t Sizeof() const {return sizeof(*this);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  
  void Val(Place& result) const;
};

class MPlace; // forward declaration

/*
\section{Class ~UPlace~}

*/
class UPlace : public ConstTemporalUnit<Place> {
 public:
  UPlace() {}
  explicit UPlace(bool defined) : ConstTemporalUnit<Place>(defined) {}
  explicit UPlace(const SecInterval &iv, const Place& pl);
  UPlace(const UPlace& up);
  UPlace(int i, MPlace &mp);
  
  ~UPlace() {}

  bool operator==(const UPlace& rhs) const;
  string toString() {return nl->ToString(ToListExpr(nl->Empty()));}

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(UPlace);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "u" + Place::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return 0;}
  Flob* GetFLOB(const int i) {return 0;}
  size_t Sizeof() const {return sizeof(*this);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);

  void Initial(IPlace& result) const;
  void Final(IPlace& result) const;
};

/*
\section{Class ~MPlace~}

*/
class MPlace : public Mapping<UPlace, Place> {
 public:
  MPlace() {}
  explicit MPlace(const int n);
  explicit MPlace(const MPlace& mp);
  
  ~MPlace() {}

  string toString() {return nl->ToString(ToListExpr(nl->Empty()));}

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(MPlace);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "m" + Place::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return 1;}
  Flob* GetFLOB(const int i) {return &units;}
  size_t Sizeof() const {return sizeof(*this);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);

  void Get(const int i, UPlace& result) const;
  bool IsEmpty() const {return units.Size() == 0;}
  int GetNoComponents() const {return units.Size();}
  void Clear() {Mapping<UPlace, Place>::Clear();}
  void StartBulkLoad() {assert(IsDefined());}
  void EndBulkLoad(const bool sort = true, const bool checkvalid = true);
  void Add(const UPlace& up);
  void MergeAdd(const UPlace& up);
  bool Passes(const Place& pl) const;
  void At(const Place& pl, MPlace& result) const;
  void DefTime(Periods& per) const;
  void Atinstant(const Instant& inst, IPlace& result) const;
  void Initial(IPlace& result) const;
  void Final(IPlace& result) const;
};

int ComparePlaces(const void *a, const void *b);

class IPlaces;
class UPlaces;
class MPlaces;

/*
\section{Class ~Places~}

*/
class Places : public Attribute {
 public:
  typedef PlaceValue base;
  typedef Place single;
  typedef IPlaces itype;
  typedef UPlaces utype;
  typedef MPlaces mtype;
   
  Places() {}
  Places(const int n) : Attribute(true), values(n) {}
  Places(const Places& rhs);
  Places(const bool def) : Attribute(def), values(0) {}

  ~Places() {}

  void Append(const PlaceValue &pv) {values.Append(pv);}
  void Append(const Place& pl);
  void Destroy() {values.destroy();}
  int GetNoValues() const {return values.Size();}
  void GetPlace(const int i, Place& result) const;
  void GetPlaceValue(const int i, PlaceValue& result) const;
  bool IsEmpty() const {return (GetNoValues() == 0);}
  void Sort() {values.Sort(ComparePlaces);}
  void Clean() {if (values.Size()) {values.clean();}}
  void operator=(const Places& p);
  bool operator==(const Places& p) const;
  string toString() {return nl->ToString(ToListExpr(nl->Empty()));}

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(Places);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return Place::BasicType() + "s";}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return 1;}
  Flob* GetFLOB(const int i) {assert(i == 0); return &values;}
  size_t Sizeof() const {return sizeof(*this);}
  int Compare(const Attribute* arg) const;
  bool Adjacent(const Attribute *arg) const {return false;}
  Attribute *Clone() const {return new Places(*this);}
  size_t HashValue() const;
  virtual void CopyFrom(const Attribute* right) {*this = *((Places*)right);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  
  bool Contains(Place& pl) const;

 private:
  DbArray<PlaceValue> values;
};

/*
\section{Class ~IPlaces~}

*/
class IPlaces : public Intime<Places> {
 public:
  IPlaces() {}
  explicit IPlaces(const Instant& inst, const Places& pls);
  explicit IPlaces(const IPlaces& rhs);
  IPlaces(const bool def) : Intime<Places>(def) {}
  
  string toString() {return nl->ToString(ToListExpr(nl->Empty()));}

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(IPlaces);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return IPlace::BasicType() + "s";}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return value.NumOfFLOBs();}
  Flob* GetFLOB(const int i) {return value.GetFLOB(i);}
  size_t Sizeof() const {return sizeof(*this);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  
  void Val(Places& result) const;
};

class MPlaces; // forward declaration

/*
\section{Class ~UPlaces~}

*/
class UPlaces : public ConstTemporalUnit<Places> {
 public:
  UPlaces() {}
  explicit UPlaces(const SecInterval& iv, const Places& pls);
  UPlaces(int i, MPlaces &mps);
  explicit UPlaces(const UPlaces& rhs);
  UPlaces(const bool def) : ConstTemporalUnit<Places>(def) {}

  string toString() {return nl->ToString(ToListExpr(nl->Empty()));}

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(UPlaces);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return UPlace::BasicType() + "s";}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return constValue.NumOfFLOBs();}
  Flob* GetFLOB(const int i) {return constValue.GetFLOB(i);}
  size_t Sizeof() const {return sizeof(*this);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  
  void Initial(IPlaces& result) const;
  void Final(IPlaces& result) const;
};

class ExprList {
 public: 
  vector<string> exprs;

  ExprList() {}
  ~ExprList() {}

  string toString();
};

class Condition {
 private:
  string text;
  vector<pair<string, int> > varKeys;
  pair<QueryProcessor*, OpTree> opTree;
  vector<Attribute*> pointers; // for each expression like X.card
  bool treeOk;

 public:
  Condition() : treeOk(false) {}
  ~Condition() {}
  
  string toString() const;
  int convertVarKey(const char *varKey);
  void clear();
  static string getType(int t);
  bool initOpTree();
  void deleteOpTree();
  
  string  getText() const          {return text;}
  void    setText(string newText)  {text = newText;}
  int     getVarKeysSize() const   {return varKeys.size();}
  string  getVar(unsigned int pos) {string s; return (pos < varKeys.size() ?
                                                     varKeys[pos].first : s);}
  int     getKey(unsigned int pos) {return (pos < varKeys.size() ?
                                            varKeys[pos].second : -1);}
//   int     getPId(unsigned int pos) {return (pos < pIds.size() ?
//                                                   pIds[pos] : -1);}
  void    clearVectors()           {varKeys.clear();}
//   string  getVar(unsigned int pos) {return (pos < vars.size() ?
//                                                   vars[pos] : "");}
  void    setOpTree(pair<QueryProcessor*, OpTree> qp_op) {opTree = qp_op;}
  void    setPointers(vector<Attribute*> ptrs)           {pointers = ptrs;}
  void    setLabelPtr(unsigned int pos, string value);
  void    clearTimePtr(unsigned int pos);
  void    mergeAddTimePtr(unsigned int pos, SecInterval value);
  void    setStartEndPtr(unsigned int pos, Instant value);
  void    setCardPtr(unsigned int pos, int value);
  void    cleanLabelsPtr(unsigned int pos);
  void    appendToLabelsPtr(unsigned int pos, Label value);
  void    completeLabelsPtr(unsigned int pos);
  void    setLeftRightclosedPtr(unsigned int pos, bool value);
  QueryProcessor* getQP()          {return opTree.first;}
  OpTree  getOpTree()              {return opTree.second;}
  int     getPointersSize()        {return pointers.size();}
  bool    isTreeOk()               {return treeOk;}
  void    setTreeOk(bool value)    {treeOk = value;}
};

class PatElem {
 private:
  string var;
  set<string> ivs;
  set<string> lbs;
  Wildcard wc;
  bool ok;

 public:
  PatElem() {}
  PatElem(const char* contents);
  ~PatElem() {}

  void setUnit(const char *v, const char *i, const char *l, const char *w);
  void getUnit(const char *v, bool assignment);
  void setVar(string v) {var = v;}

  string      getV() const                {return var;}
  set<string> getL() const                {return lbs;}
  set<string> getI() const                {return ivs;}
  Wildcard    getW() const                {return wc;}
  bool        isOk() const                {return ok;}
  void        clearL()                    {lbs.clear();}
  void        insertL(string newLabel)    {lbs.insert(newLabel);}
  void        clearI()                    {ivs.clear();}
  void        insertI(string newInterval) {ivs.insert(newInterval);}
  void        clearW()                    {wc = NO;}
};

class Assign {
 private:
  int resultPos;
  bool occurrence; // ~true~ if and only if ~var~ occurs in the pattern
  string text[6]; // one for label, time, start, end, leftclosed, rightclosed
  string var; // the assigned variable
  vector<pair<string, int> > right[7]; // a list of vars and keys for every type
  pair<QueryProcessor*, OpTree> opTree[6];
  vector<Attribute*> pointers[6]; // for each expression like X.card
  bool treesOk;

 public:
  Assign() {treesOk = false;}
  ~Assign() {}

  static string getDataType(int key);
  bool convertVarKey(const char* vk);
  bool prepareRewrite(int key, const vector<size_t> &assSeq,
                      map<string, int> &varPosInSeq, MLabel const &ml);
  bool initOpTrees();
  void clear();
  void deleteOpTrees();
  void setLabelPtr(unsigned int pos, string value);
  void setTimePtr(unsigned int pos, SecInterval value);
  void setStartPtr(unsigned int pos, Instant value);
  void setEndPtr(unsigned int pos, Instant value);
  void setLeftclosedPtr(unsigned int pos, bool value);
  void setRightclosedPtr(unsigned int pos, bool value);
  
  void    init(string v, int pp)             {clear(); var = v;
                                              occurrence = (pp > -1);}
  int     getResultPos() const               {return resultPos;}
  void    setResultPos(int p)                {resultPos = p;}
  bool    occurs() const                     {return occurrence;}
  void    setOccurrence(int p)               {occurrence = (p > -1);}
  string  getText(int key) const             {return text[key];}
  void    setText(int key, string newText)   {if (!text[key].empty()) {
                                               right[key].clear();}
                                              text[key] = newText;}
  int     getRightSize(int key) const        {return right[key].size();}
  string  getV() const                       {return var;}
  int     getRightKey(int lkey, int j) const {return right[lkey][j].second;}
  pair<string, int> getVarKey(int key, int i) const {return right[key][i];}
  pair<string, int> getVarKey(int key) const {return right[key].back();}
  string  getRightVar(int lkey, int j) const {return right[lkey][j].first;}
  void    addRight(int key,
               pair<string, int> newRight)   {right[key].push_back(newRight);}
  void    removeUnordered()                  {right[6].pop_back();}
  QueryProcessor* getQP(unsigned int key)    {if (key < 6) {
                                                return opTree[key].first;}
                                              else return 0;}
  OpTree  getOpTree(unsigned int key)        {if (key < 6) {
                                                return opTree[key].second;}
                                              else return 0;}
  bool    areTreesOk()                       {return treesOk;}
  void    setTreesOk(bool value)             {treesOk = value;}
};

class Pattern {
  friend class Match;
  
 private:
  vector<PatElem> elems;
  vector<Assign> assigns;
  vector<Condition> easyConds; // evaluated during matching
  vector<Condition> conds; // evaluated after matching
  string text, description, regEx;
  map<string, pair<int, int> > varPos;
  map<int, int> atomicToElem;
  map<int, string> elemToVar;
  map<int, set<int> > easyCondPos;
  set<string> assignedVars; // variables on the right side of an assignment
  set<string> relevantVars; // variables that occur in conds, results, assigns
  vector<map<int, int> > nfa;
  set<int> finalStates;
  
 public:
  Pattern() {}

  Pattern(int i) {}

  Pattern(const Pattern& rhs) {
    elems = rhs.elems;
    assigns = rhs.assigns;
    easyConds = rhs.easyConds;
    conds = rhs.conds;
    text = rhs.text;
    description = rhs.description;
    regEx = rhs.regEx;
    varPos = rhs.varPos;
    easyCondPos = rhs.easyCondPos;
    assignedVars = rhs.assignedVars;
    nfa = rhs.nfa;
    finalStates = rhs.finalStates;
  }

  ~Pattern() {
    deleteEasyCondOpTrees();
    deleteCondOpTrees();
    deleteAssignOpTrees();
  }

  string GetText() const;
   // algebra support functions
  static Word     In(const ListExpr typeInfo, const ListExpr instance,
                     const int errorPos, ListExpr& errorInfo, bool& correct);
  static ListExpr Out(ListExpr typeInfo, Word value);
  static Word     Create(const ListExpr typeInfo);
  static void     Delete(const ListExpr typeInfo, Word& w);
  static void     Close(const ListExpr typeInfo, Word& w);
  static Word     Clone(const ListExpr typeInfo, const Word& w);
  static bool     Open(SmiRecord& valueRecord, size_t& offset,
                       const ListExpr typeInfo, Word& value);
  static bool     Save(SmiRecord& valueRecord, size_t& offset,
                       const ListExpr typeInfo, Word& value);
  static bool     KindCheck(ListExpr type, ListExpr& errorInfo);
  static int      SizeOfObj();
  static ListExpr Property();
  // other functions
  static const string BasicType();
  static const bool checkType(const ListExpr type);
  const bool      IsDefined() {return true;}
  
  static Pattern* getPattern(string input, bool classify = false);
  ExtBool matches(MLabel *ml);
  int getResultPos(const string v);
  void collectAssVars();
  void addVarPos(string var, int pos);
  void addAtomicPos();
  int getPatternPos(const string v);
  bool checkAssignTypes();
  static pair<string, Attribute*> getPointer(int key);
  bool initAssignOpTrees();
  void deleteAssignOpTrees(bool conds);
  bool parseNFA();
  bool containsFinalState(set<int> &states);
  bool initCondOpTrees();
  bool initEasyCondOpTrees();
  void deleteCondOpTrees();
  void deleteEasyCondOpTrees();

  vector<PatElem>   getElems()              {return elems;}
  vector<Condition>* getConds()             {return &conds;}
  bool              hasConds()              {return conds.size() > 0;}
  bool              hasEasyConds()          {return easyConds.size() > 0;}
  vector<Condition> getEasyConds()          {return easyConds;}
  vector<Assign>&   getAssigns()            {return assigns;}
  PatElem           getElem(int pos) const  {return elems[pos];}
  Condition         getCond(int pos) const  {return conds[pos];}
  Condition         getEasyCond(int pos) const {return easyConds[pos];}
  Assign           getAssign(int pos) const {return assigns[pos];}
  set<int>          getFinalStates() const  {return finalStates;}
  bool              hasAssigns()            {return !assigns.empty();}
  void              addPatElem(PatElem pElem)      {elems.push_back(pElem);}
  void              addRegExSymbol(const char* s) {regEx += s;}
  void              addCond(Condition cond) {conds.push_back(cond);}
  void              addEasyCond(int pos, Condition cond) {
    easyCondPos[pos].insert(easyConds.size());
    easyConds.push_back(cond);
  }
  void              addAssign(Assign ass)  {assigns.push_back(ass);}
  void              setText(string newText) {text = newText;}
  pair<int, int>    getVarPos(string var)   {return varPos[var];}
  int               getSize() const         {return elems.size();}
  map<string, pair<int, int> > getVarPos()  {return varPos;}
  map<int, set<int> > getEasyCondPos()      {return easyCondPos;}
  void              insertAssVar(string v)  {assignedVars.insert(v);}
  set<string>       getAssVars()            {return assignedVars;}
  void setAssign(int posR, int posP, int key, string arg) {
            assigns[posR].setText(key, arg); assigns[posR].setOccurrence(posP);}
  void addAssignRight(int pos, int key, pair<string, int> varKey)
                                           {assigns[pos].addRight(key, varKey);}
  vector<map<int, int> >* getNFA()          {return &nfa;}
  map<int, int>     getTransitions(int pos) {assert(pos >= 0);
    assert(pos < (int)nfa.size()); return nfa[pos];}
  void              setNFA(vector<map<int, int> > &_nfa, set<int> &fs) {
    nfa = _nfa; finalStates = fs;
  }
  void              eraseTransition(int state, int pE) {nfa[state].erase(pE);}
  void              setDescr(string desc)   {description = desc;}
  string            getDescr()              {return description;}
  void deleteAssignOpTrees()   {for (unsigned int i = 0;i < assigns.size();i++){
                                  assigns[i].deleteOpTrees();}}
  string            getRegEx()              {return regEx;}
  bool              containsRegEx()         {return
                                    regEx.find_first_of("()|") != string::npos;}
  void              addRelevantVar(string var) {relevantVars.insert(var);}
  bool              isRelevant(string var)  {return relevantVars.count(var);}
  string            getVarFromElem(int elem){return elemToVar[elem];}
  int               getElemFromAtom(int atom) {return atomicToElem[atom];}
};

class Classifier : public Attribute {
  friend class ClassifyLI;
 public:
  Classifier() {}
  Classifier(int i) : Attribute(true), charpos(0), chars(0), delta(0),
                      s2p(0), defined(true) {}
  Classifier(const Classifier& src);

  ~Classifier() {
    charpos.Destroy();
    chars.Destroy();
    delta.Destroy();
    s2p.Destroy();
  }

  static const string BasicType() {return "classifier";}
  int getCharPosSize() const {return charpos.Size();}
  int getNumOfP() const {return charpos.Size() / 2;}
  int getCharSize() const {return chars.Size();}
  void appendCharPos(int pos) {charpos.Append(pos);}
  void appendChar(char ch) {chars.Append(ch);}
  void SetDefined(const bool def) {defined = def;}
  bool IsDefined() const {return defined;}
  bool IsEmpty() const {return (chars.Size() == 0);}
  string getDesc(int pos);
  string getPatText(int pos);
  void buildMultiNFA(vector<Pattern*> patterns, vector<map<int, int> > &nfa,
                     set<int> &finalStates, map<int, int> &state2Pat);
  void setPersistentNFA(vector<map<int, int> > &nfa, set<int> &finalSt,
                        map<int, int> &state2Pat) {
    delta.clean();
    s2p.clean();
    makeNFApersistent(nfa, finalSt, delta, s2p, state2Pat);
  }
  DbArray<NFAtransition> *getDelta() {return &delta;}
  int getNumOfState2Pat() {return s2p.Size();}
  void getStartStates(set<int> &startStates);

     // algebra support functions
  static Word     In(const ListExpr typeInfo, const ListExpr instance,
                     const int errorPos, ListExpr& errorInfo, bool& correct);
  static ListExpr Out(ListExpr typeInfo, Word value);
  static Word     Create(const ListExpr typeInfo);
  static void     Delete(const ListExpr typeInfo, Word& w);
  static void     Close(const ListExpr typeInfo, Word& w);
  static Word     Clone(const ListExpr typeInfo, const Word& w);
  static bool     Open(SmiRecord& valueRecord, size_t& offset,
                       const ListExpr typeInfo, Word& value);
  static bool     Save(SmiRecord& valueRecord, size_t& offset,
                       const ListExpr typeInfo, Word& value);
  static bool     KindCheck(ListExpr type, ListExpr& errorInfo);
  static int      SizeOfObj();
  static void*    Cast(void* addr);
         int      Compare(const Attribute* arg) const;
         size_t   HashValue() const;
         bool     Adjacent(const Attribute* arg) const;
         Classifier* Clone() const;
         void     CopyFrom(const Attribute* right);
         size_t   Sizeof() const;
  static ListExpr Property();
  static bool     checkType(ListExpr t) {
    return listutils::isSymbol(t, BasicType());
  }
  
 private:
  vector<map<int, int> > nfa; // multiNFA (not persistent)
  DbArray<int> charpos;
  DbArray<char> chars;
  DbArray<NFAtransition> delta; // multiNFA (persistent)
  DbArray<int> s2p; // neg: start; pos: final; INT_MAX: default
  bool defined;
};

struct BindingElem {
  BindingElem() : from(-1), to(-1) {}

  BindingElem(string v, unsigned int f, unsigned int t) :
    var(v), from(f), to(t) {}
    
  string var;
  unsigned int from, to;
};

struct StateWithULs {
  StateWithULs(unsigned int i, unsigned int range = UINT_MAX)
                  : rangeStart(range), state(i) {if (i == 0) {items.insert(0);}}
  StateWithULs() : rangeStart(UINT_MAX), state(UINT_MAX) {}

  bool isActive(unsigned int pos) {
    return (items.count(pos) || rangeStart <= pos);}
  bool mismatch(unsigned int mlSize) {
    return (items.empty() && rangeStart > mlSize);}
  bool match(set<int> finalStates, unsigned int mlSize) {
    return (finalStates.count(state) &&
           (rangeStart <= mlSize || items.count(mlSize)));}
  void activateNextItems(StateWithULs old) {
    if (old.rangeStart != UINT_MAX) {
      rangeStart++;
    }
    set<unsigned int>::iterator it;
    for (it = old.items.begin(); it != old.items.end(); it++) {
      items.insert(*it + 1);
    }
  }
  void clear() {
    items.clear(); rangeStart = UINT_MAX; state = UINT_MAX;}

  unsigned int rangeStart, state;
  set<unsigned int> items;
};

class Match {
  friend class RewriteLI;
 private:
  Pattern *p;
  MLabel *ml;
  set<unsigned int>** matching; // stores the whole matching process

 public:
  Match(Pattern *pat, MLabel *mlabel) {
    p = pat;
    ml = mlabel;
    matching = 0;
  }

  ~Match() {}
  
  ExtBool matches();
  bool updateStates(int i, vector<map<int, int> > &nfa, vector<PatElem> &elems,
                    set<int> &finalStates, set<int> &states,
                    vector<Condition> &easyConds, 
                    map<int, set<int> > &easyCondPos,
                    map<int, int> &atomicToElem, bool store = false);
  bool easyCondsMatch(int ulId, int pId, PatElem const &up,
                      vector<Condition> &easyConds, set<int> &pos);
  string states2Str(int ulId, set<int> &states);
  string matchings2Str(unsigned int dim1, unsigned int dim2);
  bool findMatchingBinding(vector<map<int, int> > &nfa, int startState,
                           vector<PatElem> &elems, vector<Condition> &conds,
                      map<int, int> &atomicToElem, map<int, string> &elemToVar);
  bool findBinding(unsigned int ulId, unsigned int pId, vector<PatElem> &elems,
                   vector<Condition> &conds, map<int, string> &elemToVar,
                   map<string, pair<unsigned int, unsigned int> > &binding);
  void cleanPaths(map<int, int> &atomicToElem);
  bool cleanPath(unsigned int ulId, unsigned int pId);
  bool conditionsMatch(vector<Condition> &conds,
                 const map<string, pair<unsigned int, unsigned int> > &binding);
  bool evaluateEmptyML();
  bool evaluateCond(Condition &cond,
                 const map<string, pair<unsigned int, unsigned int> > &binding);
  void printBinding(map<string, pair<unsigned int, unsigned int> > &b);
  void deletePattern() {if (p) {delete p; p = 0;}}
  bool initEasyCondOpTrees() {return p->initEasyCondOpTrees();}
  bool initCondOpTrees() {return p->initCondOpTrees();}
  bool initAssignOpTrees() {return p->initAssignOpTrees();}
  void deleteSetMatrix() {if (matching) {
                           ::deleteSetMatrix(matching, ml->GetNoComponents());}}
  void createSetMatrix(unsigned int dim1, unsigned int dim2) {
    matching = ::createSetMatrix(dim1, dim2);}
  void setML(MLabel *newML) {ml = newML;}
  bool indexMatch(StateWithULs swu);
  void filterTransitions(vector<map<int, int> > &nfaSimple,
                         string regExSimple = "");
  bool nfaIsViable();
  bool reachesFinalState(vector<map<int, int> > &nfa);
  vector<int> applyConditions(ClassifyLI* c);
  
  
  void setNFA(vector<map<int, int> > &nfa, set<int> &fs) {p->setNFA(nfa, fs);}
  void setNFAfromDbArrays(DbArray<NFAtransition> &trans, DbArray<int> &fs) {
    vector<map<int, int> > nfa;
    set<int> finalStates;
    createNFAfromPersistent(trans, fs, nfa, finalStates);
    p->setNFA(nfa, finalStates);
  }
  void setPattern(Pattern *pat) {p = pat;}
  Pattern* getPattern() {return p;}
  MLabel* getML() {return ml;}
  pair<string, Attribute*> getPointer(int key);
  static pair<QueryProcessor*, OpTree> processQueryStr(string query, int type);
};

struct BindingStackElem {
  BindingStackElem(unsigned int ul, unsigned int pe) : ulId(ul), peId(pe) {}

  unsigned int ulId, peId;
//   map<string, pair<unsigned int, unsigned int> > binding;
};

class RewriteLI {
 public:
  RewriteLI(MLabel *src, Pattern *pat);
  RewriteLI(int i) : match(0) {}

  ~RewriteLI() {}

  MLabel* getNextResult();
  void resetBinding(unsigned int limit);
  bool findNextBinding(unsigned int ulId, unsigned int pId, Pattern *p,
                       int offset);
  
 protected:
  stack<BindingStackElem> bindingStack;
  Match *match;
  map<string, pair<unsigned int, unsigned int> > binding;
  set<map<string, pair<unsigned int, unsigned int> > > rewBindings;
};

class ClassifyLI {

friend class Match;

public:
  ClassifyLI(MLabel *ml, Word _classifier);
  ClassifyLI(int i) : classifyTT(0) {}

  ~ClassifyLI();

  static TupleType* getTupleType();
  FText* nextResultText();
  void printMatches();

protected:
  vector<Pattern*> pats;
  TupleType* classifyTT;
  set<int> matchingPats;
};

class MultiRewriteLI : public ClassifyLI, public RewriteLI {

 public:
  MultiRewriteLI(Word _mlstream, Word _pstream);

  ~MultiRewriteLI();

  MLabel* nextResultML();
  void getStartStates(set<int> &states);
  void initStack(set<int> &startStates);

 private:
  Stream<MLabel> mlStream;
  bool streamOpen;
  MLabel* ml;
  Classifier* c;
  map<int, int> state2Pat;
  set<int> finalStates, states, matchCands;
  vector<map<int, int> > nfa;
  vector<PatElem> patElems;
  vector<Condition> easyConds;
  map<int, set<int> > easyCondPos;
  map<int, pair<int, int> > patOffset; // elem no |-> (pat no, first pat elem)
  map<int, int> atomicToElem;
  map<int, string> elemToVar;
};

class FilterMatchesLI {
 public:
  FilterMatchesLI(Word _stream, int _attrIndex, FText* text);
  FilterMatchesLI(Word _stream, int _attrIndex, Pattern* p);

  ~FilterMatchesLI();

  Tuple* getNextResult();
  
 private:
  Stream<Tuple> stream;
  int attrIndex;
  Match* match;
  bool streamOpen, deleteP;
};

struct IndexMatchInfo {
  IndexMatchInfo(int s);
  
  void print(TupleId tId, int pE);
  bool isActive(int patElem);
  void processWildcard(Wildcard w, int patElem);
  void processSimple(set<int> found);
  void processSimple();
  bool matches(int patSize);
  void insert(int item) {items.insert(items.end(), item);}

  bool range;
  int start, processed, size;
  set<int> items;
};

class IndexClassifyLI {

friend class Match;
friend class IndexMatchesLI;

 public:
  IndexClassifyLI(Word _mlrel, InvertedFile *inv, Word _classifier,int _attrNr);
  IndexClassifyLI(Word _mlrel, InvertedFile *inv, int _attrNr);

  ~IndexClassifyLI();

  Tuple* nextResultTuple();
  void applyPattern(Pattern *p);
  int getMLsize(TupleId tId);
  ULabel getUL(TupleId tId, unsigned int ulId);
  bool timesMatch(TupleId tId, unsigned int ulId, set<string> ivs);

 private:
  Classifier *c;
  Relation *mlRel;
  queue<pair<string, TupleId> > classification;
  TupleType* classifyTT;
  InvertedFile* invFile;
  int attrNr, processedP;
  size_t maxMLsize;
};

class IndexMatchesLI : public IndexClassifyLI {
 public:
  IndexMatchesLI(Word _mlrel, InvertedFile *inv, int _attrNr, Pattern *p, 
                 bool deleteP);

  ~IndexMatchesLI() {}

  Tuple* nextTuple();
};

class UnitsLI {
 public:
  UnitsLI() : index(0) {}
  ~UnitsLI() {}

  int index;
};

}
