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
//#include "SymbolicTrajectoryTools.h"
#include "ListUtils.h"
#include "RelationAlgebra.h"
#include "Stream.h"
#include "InvertedFile.h"
#include "RTreeAlgebra.h"
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

int parsePatternRegEx(const char* argument, IntNfa** T);

union Word;

namespace stj {

class Pattern;
class PatElem;
class Assign;
class ClassifyLI;
class IndexLI;

Pattern* parseString(const char* input, bool classify);
void patternFlushBuffer();

enum ExtBool {FALSE, TRUE, UNDEF};
enum Wildcard {NO, STAR, PLUS};
enum DataType {LABEL, LABELS, PLACE, PLACES};
enum SetRel {STANDARD, DISJOINT, SUPERSET, EQUAL, INTERSECT};

struct NFAtransition {
  int oldState;
  int trigger;
  int newState;
};

template<class F, class S>
class NewPair {
 public:
  F first;
  S second;
};

struct SymbolicUnit {
 public: 
  SymbolicUnit() {}
  SymbolicUnit(unsigned int p) : iv(true), pos(p) {}
  
  SecInterval iv;
  unsigned int pos;
};

struct ExtSymbolicUnit : public SymbolicUnit {
  ExtSymbolicUnit() {}
  ExtSymbolicUnit(unsigned int p) : SymbolicUnit(p), ref(0) {}
  
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
  typedef string base;
  typedef Labels coll;
   
  Label() {}
  Label(const string& text) : Attribute(true), value(0) {SetValue(text);}
  Label(const Label& rhs);
  explicit Label(const bool def) : Attribute(def), value(0) {}
  
  ~Label() {}
  
  void Clean() {value.clean();}
  void Destroy() {value.destroy();}
  void GetValue(string &text) const;
  static void buildValue(const string& text, const unitelem& unit,base& result);
  static ListExpr getList(const string& text) {return nl->TextAtom(text);}
  void Set(const bool def, const string &text) {SetDefined(def);SetValue(text);}
  void SetValue(const string &text);
  Label& operator=(const Label& lb) {CopyFrom(&lb); return *this;}
  bool operator==(const Label& lb) const;
  bool operator==(const string& text) const;

  static bool readValueFrom(ListExpr LE, string& text, unitelem& unit);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  ListExpr ToListExpr(ListExpr typeInfo);
  static bool     CheckKind(ListExpr type, ListExpr& errorInfo);
  static int      SizeOfObj() {return sizeof(Label);}
  static ListExpr Property();
  static const string BasicType() {return "label";}
  static const bool checkType(const ListExpr type);
  void            CopyFrom(const Attribute* right);
  int NumOfFLOBs() const {return 1;}
  Flob *GetFLOB(const int i) {assert(i == 0); return &value;}
  int             Compare(const Attribute* arg) const;
  size_t          Sizeof() const {return sizeof(*this);}
  bool            Adjacent(const Attribute*) const {return false;}
  Attribute*      Clone() const {return new Label(*this);}
  size_t          HashValue() const {return value.getSize();}
  ostream&        Print(ostream& os) const;
  
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
  typedef string base;
  typedef Label single;
      
  Labels() {}
  explicit Labels(const bool defined) : Attribute(defined), values(8), pos(1) {}
  Labels(const Labels& src, const bool sort = false);
  
  ~Labels() {}

  Labels& operator=(const Labels& src);
  bool operator==(const Labels& src) const;
  void Append(const Label &lb);
  void Append(const string& text);
  void Append(const set<string>& values);
  void Destroy() {values.destroy(); pos.destroy();}
  int GetNoValues() const {return pos.Size();}
  size_t GetLength() const {return values.getSize();}
  void Get(int i, Label& lb) const;
  void GetValue(int i, string& text) const;
  void GetValues(set<string>& values) const;
  static void getRefToLastElem(const int size, unsigned int& result);
  static unsigned int getFlobPos(const arrayelem elem);
  static void valuesToListExpr(const set<string>& values, ListExpr& result);
  static void getString(const ListExpr& list, string& result);
  static void getElemFromList(const ListExpr& list, const unsigned int size, 
                              unsigned int& result);
  static void buildValue(const string& text, const unsigned int pos,
                         string& result);
  static void printArrayElem(const arrayelem e) {cout << "print " << e << endl;}
  const bool IsEmpty() const {return GetNoValues() == 0;}
  void Clean() {values.clean(); pos.clean();}
  bool Contains(const string& text) const;
  friend ostream& operator<<(ostream& os, const Labels& lbs);
  
  int NumOfFLOBs() const {return 2;}
  Flob *GetFLOB(const int i);
  int Compare(const Attribute*) const;
  bool Adjacent(const Attribute*) const {return false;}
  Attribute *Clone() const {return new Labels(*this);}
  size_t Sizeof() const {return sizeof(*this);}
  ostream& Print(ostream& os) const {return (os << *this);}
  static ListExpr Property();
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  static int SizeOfObj() {return sizeof(Labels);}
  static const string BasicType() {return Label::BasicType() + "s";}
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
  typedef pair<string, unsigned int> base;
  typedef Places coll;
  
  Place() : Label() {}
  Place(const pair<string, unsigned int>& v) : Label(v.first), ref(v.second) {}
  Place(const Place& rhs) : Label(rhs.IsDefined()) {CopyFrom(&rhs);}
  explicit Place(const bool def) : Label(def), ref(0) {}

  ~Place() {}

  void Set(const bool defined, const pair<string, unsigned int>& value);
  void SetValue(const pair<string, unsigned int>& value);
  void SetRef(const unsigned int value) {ref = value;}
  void GetValue(pair<string, unsigned int>& value) const;
  static void buildValue(const string& text, const unitelem& unit,base& result);
  static ListExpr getList(const base& value);
  unsigned int GetRef() const {return ref;}
  Place& operator=(const Place& p);
  bool operator==(const Place& p) const;
  bool operator==(const pair<string, unsigned int>& value) const;

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(Place);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "place";}
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
  static bool readValueFrom(ListExpr LE, string& text, unitelem& unit);
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
  typedef pair<string, unsigned int> base;
  typedef Place single;
   
  Places() {}
  Places(const int n) : Attribute(n > 0), values(8), posref(n) {}
  Places(const Places& rhs, const bool sort = false);
  explicit Places(const bool def) : Attribute(def), values(8), posref(1) {}

  ~Places() {}

  void Append(const base& value);
  void Append(const Place& pl);
  void Append(const set<base>& values);
  void Destroy() {values.destroy(); posref.destroy();}
  int GetNoValues() const {return posref.Size();}
  size_t GetLength() const {return values.getSize();}
  void Get(const int i, Place& pl) const;
  void GetValue(int i, base& val) const;
  void GetValues(set<base>& values) const;
  bool IsEmpty() const {return (GetNoValues() == 0);}
  bool Contains(const base& val) const;
  void Clean() {values.clean(); posref.clean();}
  static void getRefToLastElem(const int size, arrayelem& result);
  static unsigned int getFlobPos(const arrayelem elem);
  static void valuesToListExpr(const set<base>& values, ListExpr& result);
  static void getString(const ListExpr& list, string& result);
  static void getElemFromList(const ListExpr& list, const unsigned int size, 
                              arrayelem& result);
  static void buildValue(const string& text, const arrayelem pos, base& result);
  static void printArrayElem(const arrayelem e) {cout << e.first << " " 
                                                      << e.second;}
  void operator=(const Places& p);
  bool operator==(const Places& p) const;

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(Places);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return Place::BasicType() + "s";}
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
\section{Class ~IPlace~}

*/
template<class B>
class IBasic : public Intime<B> {
 public:
  IBasic() {}
  explicit IBasic(const Instant& inst, const B& val);
  explicit IBasic(const IBasic& rhs);
  IBasic(const bool def) : Intime<B>(def) {}

  ~IBasic() {}

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(IBasic<B>);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "i" + B::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return this->value.NumOfFLOBs();}
  Flob* GetFLOB(const int i) {return this->value.GetFLOB(i);}
  size_t Sizeof() const {return sizeof(*this);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  
  void Val(B& result) const;
};

// class MPlace; // forward declaration

/*
\section{Class ~UPlace~}

*/
template<class B>
class UBasic : public ConstTemporalUnit<B> {
 public:
  UBasic() {}
  explicit UBasic(bool def) : ConstTemporalUnit<B>(def) {}
  explicit UBasic(const SecInterval &iv, const B& val);
  UBasic(const UBasic& ub);
  
  ~UBasic() {}

  bool operator==(const UBasic<B>& rhs) const;

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(UBasic<B>);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "u" + B::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return this->constValue.NumOfFLOBs();}
  Flob* GetFLOB(const int i) {return this->constValue.GetFLOB(i);}
  size_t Sizeof() const {return sizeof(*this);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);

  void Initial(IBasic<B>& result) const;
  void Final(IBasic<B>& result) const;
};

/*
\section{Class ~MBasic~}

*/
template<class B>
class MBasic : public Attribute {
 public:
  typedef B base;
   
  MBasic() {}
  explicit MBasic(unsigned int n) : Attribute(n>0), values(8), units(n) {}
  explicit MBasic(const MBasic& mb);
  
  ~MBasic() {}

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(MBasic<B>);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "m" + B::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return 2;}
  Flob* GetFLOB(const int i);
  size_t Sizeof() const {return sizeof(*this);}
  int Compare(const Attribute *arg) const;
  Attribute* Clone() const {return new MBasic<B>(*this);}
  bool Adjacent(const Attribute *arg) const {return false;}
  size_t HashValue() const;
  void CopyFrom(const Attribute *arg);
  
  ListExpr unitToListExpr(const int i);
  ListExpr ToListExpr(ListExpr typeInfo);
  bool readUnitFrom(ListExpr LE);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);

  int Position(const Instant& inst) const;
  void Get(const int i, UBasic<B>& result) const;
  void GetInterval(const int i, SecInterval& result) const;
  void GetBasic(const int i, B& result) const;
  void GetValue(const int i, typename B::base& result) const;
  bool IsEmpty() const {return units.Size() == 0;}
  int GetNoComponents() const {return units.Size();}
  bool IsValid() const;
  void Clear() {values.clean(); units.clean();}
  void StartBulkLoad() {assert(IsDefined());}
  void EndBulkLoad(const bool sort = true, const bool checkvalid = true);
  void Add(const SecInterval& iv, const B& value);
  void Add(const UBasic<B>& ub);
  void MergeAdd(const UBasic<B>& ub);
  bool Passes(const B& basic) const;
  void At(const B& basic, MBasic<B>& result) const;
  void DefTime(Periods& per) const;
  void Atinstant(const Instant& inst, IBasic<B>& result) const;
  void Initial(IBasic<B>& result) const;
  void Final(IBasic<B>& result) const;
  void Inside(const typename B::coll& coll, MBool& result) const;
  void Fill(MBasic<B>& result, DateTime& duration) const;
  void Compress(MBasic<B>& result) const;
  ostream& Print(ostream& os) const;
  
 protected:
  Flob values;
  DbArray<typename B::unitelem> units;
};

/*
\section{Class ~IPlaces~}

*/
template<class B>
class IBasics : public Intime<B> {
 public:
  IBasics() {}
  explicit IBasics(const Instant& inst, const B& values);
  IBasics(const IBasics& rhs) : Intime<B>(rhs) {}
  IBasics(const bool def) : Intime<B>(def) {}
  
  static ListExpr Property();
  static int SizeOfObj() {return sizeof(IBasics<B>);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "i" + B::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return this->value.NumOfFLOBs();}
  Flob* GetFLOB(const int i) {return this->value.GetFLOB(i);}
  size_t Sizeof() const {return sizeof(*this);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  
  void Val(B& result) const;
};

/*
\section{Class ~UPlaces~}

*/
template<class B>
class UBasics : public ConstTemporalUnit<B> {
 public:
  UBasics() {}
  UBasics(const SecInterval& iv, const B& values);
  UBasics(const UBasics& rhs) : ConstTemporalUnit<B>(rhs) {}
  explicit UBasics(const bool def) : ConstTemporalUnit<B>(def) {}

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(UBasics<B>);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "u" + B::BasicType();}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return this->constValue.NumOfFLOBs();}
  Flob* GetFLOB(const int i) {return this->constValue.GetFLOB(i);}
  size_t Sizeof() const {return sizeof(*this);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  
  void Initial(IBasics<B>& result) const;
  void Final(IBasics<B>& result) const;
};

/*
\section{Class ~MBasics~}

*/
template<class B>
class MBasics : public Attribute {
 public:
  typedef B base;
   
  MBasics() {}
  explicit MBasics(int n) : Attribute(n > 0), values(8), units(n), pos(1) {}
  explicit MBasics(const MBasics& mbs);
  
  ~MBasics() {}
  
  static ListExpr Property();
  static int SizeOfObj() {return sizeof(MBasics<B>);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "m" + B::BasicType();}
  static bool checkType(ListExpr t);
  int NumOfFLOBs() const {return 3;}
  Flob* GetFLOB(const int i);
  size_t Sizeof() const {return sizeof(*this);}
  int Compare(const Attribute* arg) const;
  bool Adjacent(const Attribute *arg) const {return false;}
  Attribute *Clone() const;
  size_t HashValue() const {return pos.Size() * units.Size();}
  void CopyFrom(const Attribute* right) {*this = *((MBasics<B>*)right);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  void Destroy() {values.destroy(); units.destroy(); pos.destroy();}
  
  int getUnitEndPos(const int i) const;
  ListExpr valuesToListExpr(int start, int end);
  ListExpr unitToListExpr(int i);
  bool readValues(ListExpr valuelist);
  bool readUnit(ListExpr unitlist);
  int Position(const Instant& inst) const;
  
  void Get(const int i, UBasics<B>& result) const;
  void GetBasics(const int i, B& result) const;
  bool IsEmpty() const {return units.Size() == 0;}
  void GetValues(const int i, set<typename B::base>& result) const;
  void GetInterval(const int i, SecInterval& result) const;
  int GetNoComponents() const {return units.Size();}
  int GetNoValues() const {return pos.Size();}
  void Clear();
  void StartBulkLoad() {assert(IsDefined());}
  void EndBulkLoad(const bool sort = true, const bool checkvalid = true);
  bool IsValid() const;
  void Add(const UBasics<B>& ut);
  void Add(const SecInterval& iv, const B& values);
  void MergeAdd(const UBasics<B>& ut);
  void MergeAdd(const SecInterval& iv, const B& values);
  bool Passes(const typename B::single& sg) const;
  bool Passes(const B& bs) const;
  void At(const typename B::single& sg, MBasics<B>& result) const;
  void At(const B& bs, MBasics<B>& result) const;
  void DefTime(Periods& per) const;
  void Atinstant(const Instant& inst, IBasics<B>& result) const;
  void Initial(IBasics<B>& result) const;
  void Final(IBasics<B>& result) const;
  void Fill(MBasics<B>& result, DateTime& duration) const;
  void Compress(MBasics<B>& result) const;
  ostream& Print(ostream& os) const;
  
 protected:
  Flob values;
  DbArray<SymbolicUnit> units;
  DbArray<typename B::arrayelem> pos;
};

/*
\section{Class ~MLabel~}

*/
class MLabel : public MBasic<Label> {
 public:
  MLabel() {}
  MLabel(unsigned int n) : MBasic<Label>(n) {}
   
  void createML(int size, bool text, double rate);
  void convertFromMString(const MString& source);
};

typedef IBasic<Label> ILabel;
typedef UBasic<Label> ULabel;
typedef IBasic<Place> IPlace;
typedef UBasic<Place> UPlace;
typedef MBasic<Place> MPlace;

typedef IBasics<Labels> ILabels;
typedef UBasics<Labels> ULabels;
typedef MBasics<Labels> MLabels;
typedef IBasics<Places> IPlaces;
typedef UBasics<Places> UPlaces;
typedef MBasics<Places> MPlaces;

class Tools {
 public:
  static void intersect(const vector<set<TupleId> >& tidsets, 
                        set<TupleId>& result);
  static void intersectPairs(vector<set<pair<TupleId, int> > >& posVec, 
                             set<pair<TupleId, int> >*& result);
  static void uniteLast(unsigned int size, vector<set<TupleId> >& tidsets);
  static void uniteLastPairs(unsigned int size, 
                             vector<set<pair<TupleId, int> > >& posVec);
  static void filterPairs(set<pair<TupleId, int> >* pairs,
                    const set<TupleId>& pos, set<pair<TupleId, int> >*& result);
  template<class T>
  static bool relationHolds(const set<T>& set1, const set<T>& set2, SetRel rel);
  static string int2String(int i);
  static int str2Int(string const &text);
  static void deleteSpaces(string& text);
  static string setToString(const set<string>& input);
  static int prefixCount(string str, set<string> strings);
  static void splitPattern(string& input, vector<string>& result);
  static char* convert(string arg);
  static void eraseQM(string& arg); // QM = quotation marks
  static void addQM(string& arg);
  static void simplifyRegEx(string &regEx);
  static set<unsigned int>** createSetMatrix(unsigned int dim1, 
                                             unsigned int dim2);
  static void deleteSetMatrix(set<unsigned int>** &victim, unsigned int dim1);
  static int getKey(const string& type);
  static string getDataType(const int key);
  static DataType getDataType(const string& type);
  static string extractVar(const string& input);
  static string extendDate(string input, const bool start);
  static bool checkSemanticDate(const string &text, const SecInterval &uIv,
                                const bool resultNeeded);
  static bool checkDaytime(const string& text, const SecInterval& uIv);
  static bool isInterval(const string& str);
  static void stringToInterval(const string& str, SecInterval& result);
  static bool timesMatch(const Interval<DateTime>& iv, const set<string>& ivs);
  static pair<QueryProcessor*, OpTree> processQueryStr(string query, int type);
  // static Word evaluate(string input);
  static void createTrajectory(int size, vector<string>& result);
  static void printNfa(vector<map<int, int> > &nfa, set<int> &finalStates);
  static void makeNFApersistent(vector<map<int, int> > &nfa,
     set<int> &finalStates, DbArray<NFAtransition> &trans, DbArray<int> &fs, 
     map<int, int> &final2Pat);
  static void createNFAfromPersistent(DbArray<NFAtransition> &trans, 
          DbArray<int> &fs, vector<map<int, int> > &nfa, set<int> &finalStates);
  static void printBinding(map<string, pair<unsigned int, unsigned int> > &b);
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
  int     getKey(unsigned int pos) const {return (pos < varKeys.size() ?
                                            varKeys[pos].second : -1);}
//   int     getPId(unsigned int pos) {return (pos < pIds.size() ?
//                                                   pIds[pos] : -1);}
  void    clearVectors()           {varKeys.clear();}
//   string  getVar(unsigned int pos) {return (pos < vars.size() ?
//                                                   vars[pos] : "");}
  void    setOpTree(pair<QueryProcessor*, OpTree> qp_op) {opTree = qp_op;}
  void    setPointers(vector<Attribute*> ptrs)           {pointers = ptrs;}
  void    setValuePtr(unsigned int pos, string& value);
  void    setValuePtr(unsigned int pos, pair<string, unsigned int>& value);
  void    clearTimePtr(unsigned int pos);
  void    mergeAddTimePtr(unsigned int pos, Interval<Instant>& value);
  void    setStartEndPtr(unsigned int pos, Instant& value);
  void    setCardPtr(unsigned int pos, int value);
  void    cleanLabelsPtr(unsigned int pos);
  void    appendToLabelsPtr(unsigned int pos, string& value);
  void    appendToLabelsPtr(unsigned int pos, set<string>& values);
  void    cleanPlacesPtr(unsigned int pos);
  void    appendToPlacesPtr(unsigned int pos, pair<string,unsigned int>& value);
  void    appendToPlacesPtr(unsigned int pos, 
                            set<pair<string, unsigned int> >& values);
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
  set<pair<string, unsigned int> > pls;
  Wildcard wc;
  SetRel setRel;
  bool ok;

 public:
  PatElem() : var(""), ivs(), lbs(), pls(), wc(NO), setRel(STANDARD), ok(true){}
  PatElem(const char* contents);
  PatElem(const PatElem& elem) : var(elem.var), ivs(elem.ivs), lbs(elem.lbs),
                pls(elem.pls), wc(elem.wc), setRel(elem.setRel), ok(elem.ok) {}
  ~PatElem() {}

  void stringToSet(const string& input, const bool isTime);
  void setVar(const string& v) {var = v;}
  PatElem& operator=(const PatElem& elem) {
    var = elem.var;
    ivs = elem.ivs;
    lbs = elem.lbs; 
    pls = elem.pls;
    wc = elem.wc; 
    setRel = elem.setRel;
    ok = elem.ok;
    return *this;
  }

  void     getV(string& result) const                     {result = var;}
  void     getL(set<string>& result) const                {result = lbs;}
  void     getP(set<pair<string, unsigned int> >& result) const {result = pls;}
  SetRel   getSetRel() const                              {return setRel;}
  void     getI(set<string>& result) const                {result = ivs;}
  Wildcard getW() const                                   {return wc;}
  bool     isOk() const                                   {return ok;}
  void     clearL()                                       {lbs.clear();}
  void     clearP()                                       {pls.clear();}
  void     insertL(const string& lb)                      {lbs.insert(lb);}
  void     insertP(const pair<string, unsigned int>& pl)  {pls.insert(pl);}
  void     clearI()                                       {ivs.clear();}
  void     insertI(string& iv)                            {ivs.insert(iv);}
  void     clearW()                                       {wc = NO;}
  bool     hasLabel() const                             {return lbs.size() > 0;}
  bool     hasPlace() const                             {return pls.size() > 0;}
  bool     hasInterval() const                          {return ivs.size() > 0;}
  bool     hasRealInterval() const;
  bool     hasIndexableContents() const {return (hasLabel() || hasPlace() ||
                                                 hasRealInterval());}
};

class Assign {
 private:
  int resultPos;
  bool occurrence; // ~true~ if and only if ~var~ occurs in the pattern
  string text[10]; // one for each possible attribute
  string var; // the assigned variable
  vector<pair<string, int> > right[11]; // list of vars and keys for every type
  pair<QueryProcessor*, OpTree> opTree[10];
  vector<Attribute*> pointers[10]; // for each expression like X.card
  bool treesOk;

 public:
  Assign() {treesOk = false;}
  ~Assign() {}

  bool convertVarKey(const char* vk);
  bool prepareRewrite(int key, const vector<size_t> &assSeq,
                      map<string, int> &varPosInSeq, MLabel const &ml);
  bool hasValue() {return (!text[0].empty() || !text[1].empty() ||
                           !text[8].empty() || !text[9].empty());}
  bool hasTime() {return (!text[2].empty() || 
                         (!text[3].empty() && !text[4].empty()));}
  bool initOpTrees();
  void clear();
  void deleteOpTrees();
  void setLabelPtr(unsigned int pos, const string& value);
  void setPlacePtr(unsigned int pos, const pair<string, unsigned int>& value);
  void setTimePtr(unsigned int pos, const SecInterval& value);
  void setStartPtr(unsigned int pos, const Instant& value);
  void setEndPtr(unsigned int pos, const Instant& value);
  void setLeftclosedPtr(unsigned int pos, bool value);
  void setRightclosedPtr(unsigned int pos, bool value);
  void cleanLabelsPtr(unsigned int pos);
  void appendToLabelsPtr(unsigned int pos, const string& value);
  void appendToLabelsPtr(unsigned int pos, const set<string>& value);
  void cleanPlacesPtr(unsigned int pos);
  void appendToPlacesPtr(unsigned int pos, 
                         const pair<string,unsigned int>& value);
  void appendToPlacesPtr(unsigned int pos, 
                         const set<pair<string,unsigned int> >& value);
  
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
 public:
  Pattern() {}

  Pattern(int i) {}

  ~Pattern() {
    deleteEasyCondOpTrees();
    deleteCondOpTrees();
    deleteAssignOpTrees();
  }

  string GetText() const;
  bool isValid(const string& type) const;
  static Pattern* getPattern(string input, bool classify = false);
  template<class M>
  ExtBool matches(M *m);
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
  void              getElem(int pos, PatElem& elem) const  {elem = elems[pos];}
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
  set<int>          getEasyCondPos(const int e) {return easyCondPos[e];}
  void              insertAssVar(string v)  {assignedVars.insert(v);}
  set<string>       getAssVars()            {return assignedVars;}
  void setAssign(int posR, int posP, int key, string arg) {
            assigns[posR].setText(key, arg); assigns[posR].setOccurrence(posP);}
  void addAssignRight(int pos, int key, pair<string, int> varKey)
                                           {assigns[pos].addRight(key, varKey);}
  void              getNFA(vector<map<int, int> >& result) {result = nfa;}
  int               getNFAsize() const      {return nfa.size();}
  bool              isNFAempty() const      {return (nfa.size() == 0);}
  map<int, int>     getTransitions(int pos) {assert(pos >= 0);
    assert(pos < (int)nfa.size()); return nfa[pos];}
  bool              isFinalState(int state) {return finalStates.find(state)
                                                    != finalStates.end();}
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
};

class PatPersistent : public Label {
 public:
  PatPersistent() {}
  PatPersistent(int i) : Label(i > 0) {}
  PatPersistent(PatPersistent& src) : Label(src.toText()) {}
  
  ~PatPersistent() {}
   
  string toText() const {string value; Label::GetValue(value); return value;}
  template<class M>
  ExtBool matches(M *traj) {
    Pattern *p = Pattern::getPattern(toText());
    if (p) {
      ExtBool result = p->matches(traj);
      delete p;
      return result;
    }
    else {
      return UNDEF;
    }
  }
  
  static const string BasicType() {return "pattern";}
  static const bool checkType(const ListExpr type) {
    return listutils::isSymbol(type, BasicType());
  }
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
    Tools::makeNFApersistent(nfa, finalSt, delta, s2p, state2Pat);
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

template<class M>
class Match {
 public:
  Pattern *p;
  M *m; // mlabel, mplace, mlabels, mplaces
  set<unsigned int>** matching; // stores the whole matching process
  DataType type; // enum

  Match(Pattern *pat, M *traj) {
    p = pat;
    m = traj;
    matching = 0;
    type = getMType();
  }

  ~Match() {}
  
  DataType getMType();
  ExtBool matches();
  bool valuesMatch(int i, const PatElem& elem);
  bool updateStates(int i, vector<map<int, int> > &nfa, vector<PatElem> &elems,
                    set<int> &finalStates, set<int> &states,
                    vector<Condition> &easyConds, 
                    map<int, set<int> > &easyCondPos,
                    map<int, int> &atomicToElem, bool store = false);
  bool easyCondsMatch(int ulId, PatElem const &elem,
                      vector<Condition> &easyConds, set<int> pos);
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
  bool evaluateEmptyM();
  bool evaluateCond(Condition &cond,
                 const map<string, pair<unsigned int, unsigned int> > &binding);
  void deletePattern() {if (p) {delete p; p = 0;}}
  bool initEasyCondOpTrees() {return p->initEasyCondOpTrees();}
  bool initCondOpTrees() {return p->initCondOpTrees();}
  bool initAssignOpTrees() {return p->initAssignOpTrees();}
  void deleteSetMatrix() {if (matching) {
                       Tools::deleteSetMatrix(matching, m->GetNoComponents());}}
  void createSetMatrix(unsigned int dim1, unsigned int dim2) {
    matching = Tools::createSetMatrix(dim1, dim2);}
  void setM(M *newM) {m = newM;}
  vector<int> applyConditions(ClassifyLI* c);
  
  
  void setNFA(vector<map<int, int> > &nfa, set<int> &fs) {p->setNFA(nfa, fs);}
  void setNFAfromDbArrays(DbArray<NFAtransition> &trans, DbArray<int> &fs) {
    vector<map<int, int> > nfa;
    set<int> finalStates;
    Tools::createNFAfromPersistent(trans, fs, nfa, finalStates);
    p->setNFA(nfa, finalStates);
  }
  void setPattern(Pattern *pat) {p = pat;}
  Pattern* getPattern() {return p;}
  M* getM() {return m;}
  pair<string, Attribute*> getPointer(int key);
  static pair<QueryProcessor*, OpTree> processQueryStr(string query, int type);
};

struct BindingStackElem {
  BindingStackElem(unsigned int ul, unsigned int pe) : ulId(ul), peId(pe) {}

  unsigned int ulId, peId;
//   map<string, pair<unsigned int, unsigned int> > binding;
};

template<class M>
class RewriteLI {
 public:
  RewriteLI(M *src, Pattern *pat);
  RewriteLI(int i) : match(0) {}

  ~RewriteLI() {}

  M* getNextResult();
  static M* rewrite(M *src, map<string, pair<unsigned int, unsigned int> > 
                    binding, vector<Assign> &assigns);
  void resetBinding(unsigned int limit);
  bool findNextBinding(unsigned int ulId, unsigned int pId, Pattern *p,
                       int offset);
  
 protected:
  stack<BindingStackElem> bindingStack;
  Match<M> *match;
  map<string, pair<unsigned int, unsigned int> > binding;
  set<map<string, pair<unsigned int, unsigned int> > > rewBindings;
};

class ClassifyLI {

friend class Match<MLabel>;

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

class MultiRewriteLI : public ClassifyLI, public RewriteLI<MLabel> {

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

template<class M>
class FilterMatchesLI {
 public:
  FilterMatchesLI(Word _stream, int _attrIndex, string& pText);

  ~FilterMatchesLI();

  Tuple* getNextResult();
  
 private:
  Stream<Tuple> stream;
  int attrIndex;
  Match<M>* match;
  bool streamOpen, deleteP;
};

struct IndexRetrieval {
  IndexRetrieval() : pred(0), succ(0) {}
  IndexRetrieval(unsigned int p, unsigned int s = 0) : pred(p), succ(s) {}
  IndexRetrieval(unsigned int p, unsigned int s, set<int>& u) : 
                 pred(p), succ(s), units(u) {}
  
  unsigned int pred, succ;
  set<int> units;
};

struct IndexMatchInfo {
  IndexMatchInfo(bool r, int n = 0) : range(r), next(n) {}
  IndexMatchInfo(bool r, int n, 
            map<string, pair<unsigned int, unsigned int> > b, const string& v) :
       range(r), next(n), binding(b), prevVar(v) {}
  
  void print(const bool printBinding);
  bool finished(const int size) const {return range || (next >= size);}
  bool exhausted(const int size) const {return next >= size;}
  bool matches(const int unit) const {return (range ? next<=unit : next==unit);}

  bool range;
  int next;
  map<string, pair<unsigned int, unsigned int> > binding;
  string prevVar;
};

struct IndexMatchSlot {
  IndexMatchSlot() : pred(0), succ(0) {}
  
  unsigned int pred, succ;
  vector<IndexMatchInfo> imis;
};

class IndexClassifyLI {

friend class Match<MLabel>;
friend class IndexMatchesLI;

 public:
  IndexClassifyLI(Relation *rel, InvertedFile *inv, R_Tree<1, TupleId> *rt, 
                  Word _classifier, int _attrNr, DataType type);
  IndexClassifyLI(Relation *rel, InvertedFile *inv, R_Tree<1, TupleId> *rt, 
                  int _attrNr, DataType type);

  ~IndexClassifyLI();

  Tuple* nextResultTuple();
  void simplifyNFA(vector<map<int, int> >& result);
  void findNFApaths(const vector<map<int, int> >& nfa, 
                const set<int>& finalStates, set<pair<set<int>, int> >& result);
  void getCrucialElems(const set<pair<set<int>, int> >& paths,set<int>& result);
  void retrieveValue(vector<set<int> >& part, vector<set<int> >& part2,
                     SetRel rel, bool first, const string& label,
                     unsigned int ref = UINT_MAX);
  void retrieveTime(vector<bool>& time, vector<bool>& time2, bool first, 
                    const string& ivstr);
  void removeIdFromIndexResult(const TupleId id);
  void removeIdFromMatchInfo(const TupleId id);
  void clearMatchInfo();
  void storeIndexResult(const int e);
  void initMatchInfo(const set<int>& cruElems);
  void initialize();
  int getMsize(TupleId tId);
  void getInterval(const TupleId tId, const int pos, SecInterval& iv);
  void extendBinding(IndexMatchInfo& imi, const int e);
  template<class M>
  bool imiMatch(Match<M>& match, const int e, const TupleId id, 
                IndexMatchInfo& imi, const int unit, const int newState);
  bool valuesMatch(const int e, const TupleId id, IndexMatchInfo& imi,
                   const int newState, const int unit);
  void applySetRel(const SetRel setRel, 
                   vector<set<pair<TupleId, int> > >& valuePosVec,
                   set<pair<TupleId, int> >*& result);
  bool simpleMatch(const int e, const int state, const int newState);
  bool canIdBeRemoved(const TupleId id, const int e);
  bool wildcardMatch(const int state, pair<int, int> trans);
  bool timesMatch(const TupleId id,const unsigned int unit,const PatElem& elem);
  bool checkConditions(const TupleId id, IndexMatchInfo& imi);

 private:
  Pattern p;
  Classifier *c;
  Relation *mRel;
  queue<pair<string, TupleId> > classification;
  vector<vector<IndexRetrieval> > indexResult;
  set<int> indexMismatch;
  vector<TupleId> matches;
  vector<int> trajSize;
  int activeTuples;
  vector<vector<IndexMatchSlot> > matchInfo, newMatchInfo;
  vector<vector<IndexMatchSlot> > *matchInfoPtr, *newMatchInfoPtr;
  TupleType* classifyTT;
  InvertedFile* invFile;
  R_Tree<1, TupleId> *rtree;
  int attrNr;
  size_t maxMLsize;
  DataType mtype;
};

class IndexMatchesLI : public IndexClassifyLI {
 public:
  IndexMatchesLI(Relation *rel, InvertedFile *inv, R_Tree<1, TupleId> *rt, 
                 int _attrNr, Pattern *_p, bool deleteP, DataType type);

  ~IndexMatchesLI() {}

  Tuple* nextTuple();
  void applyNFA();
};

class UnitsLI {
 public:
  UnitsLI() : index(0) {}
  ~UnitsLI() {}

  int index;
};

}
