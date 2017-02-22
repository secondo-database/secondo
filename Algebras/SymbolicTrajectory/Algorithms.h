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
This is the implementation of the Symbolic Trajectory Algebra.

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
#include "ListUtils.h"
#include "Stream.h"
#include "RTreeAlgebra.h"
#include "IntNfa.h"
#include "TemporalUnitAlgebra.h"
#include "GenericTC.h"
#include "OrderedRelationAlgebra.h"
#include "Tools.h"
#include <string>
#include <set>
#include <stack>
#include <vector>
#include <math.h>
#include <time.h>

#include "BasicTypes.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE 1


int parsePatternRegEx(const char* argument, IntNfa** T);

union Word;

namespace stj {

class Pattern;
class PatElem;
class Assign;
class ClassifyLI;
class IndexLI;
struct IndexMatchInfo;

Pattern* parseString(const char* input, bool classify, Tuple *t,ListExpr ttype);
void patternFlushBuffer();

enum ExtBool {ST_FALSE, ST_TRUE, ST_UNDEF};
enum Wildcard {NO, STAR, PLUS};
enum IndexType {TRIE, BTREE, RTREE1, RTREE2, NONE};
enum DistanceFunction {FIRST, LAST, FIRST_LAST, ALL, ALL_DURATION, 
                       ALL_INTERVALS};

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
  UBasic() {}
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
  UBasics() {}
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
  void Clear() {values.clean(); units.clean();}
  void StartBulkLoad() {assert(IsDefined());}
  void EndBulkLoad(const bool sort = true, const bool checkvalid = true);
  void Add(const temporalalgebra::SecInterval& iv, const B& value);
  void Add(const UBasic<B>& ub);
  void MergeAdd(const UBasic<B>& ub);
  bool Passes(const B& basic) const;
  void At(const B& basic, MBasic<B>& result) const;
  void DefTime(temporalalgebra::Periods& per) const;
  void Atinstant(const Instant& inst, IBasic<B>& result) const;
  void AtPeriods(const temporalalgebra::Periods& per, MBasic<B>& result) const;
  void Initial(IBasic<B>& result) const;
  void Final(IBasic<B>& result) const;
  void Inside(const typename B::coll& coll, 
              temporalalgebra::MBool& result) const;
  void Fill(MBasic<B>& result, datetime::DateTime& duration) const;
  void Concat(const MBasic<B>& src1, const MBasic<B>& src2);
  void Compress(MBasic<B>& result) const;
  std::ostream& Print(std::ostream& os) const;  
  double Distance_FIRST(const MBasic<B>& mb) const;
  double Distance_LAST(const MBasic<B>& mb) const;
  double Distance_FIRST_LAST(const MBasic<B>& mb) const;
  double Distance_ALL(const MBasic<B>& mb, const LabelFunction lf) const;
  double Distance_ALL_DURATION(const MBasic<B>& mb, const LabelFunction lf)
         const;
  double Distance_ALL_INTERVALS(const MBasic<B>& mb, const LabelFunction lf)
         const;
  double Distance(const MBasic<B>& mb) const;
  
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
  void At(const typename B::single& sg, MBasics<B>& result) const;
  void At(const B& bs, MBasics<B>& result) const;
  void DefTime(temporalalgebra::Periods& per) const;
  void Atinstant(const Instant& inst, IBasics<B>& result) const;
  void AtPeriods(const temporalalgebra::Periods& per, MBasics<B>& result) const;
  void Initial(IBasics<B>& result) const;
  void Final(IBasics<B>& result) const;
  void Fill(MBasics<B>& result, datetime::DateTime& duration) const;
  void Concat(const MBasics<B>& src1, const MBasics<B>& src2);
  void Compress(MBasics<B>& result) const;
  std::ostream& Print(std::ostream& os) const;
  double Distance(const MBasics<B>& mbs) const;
  
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
typedef MBasic<Place> MPlace;
typedef MBasics<Labels> MLabels;
typedef MBasics<Places> MPlaces;

class ExprList {
 public: 
  std::vector<std::string> exprs;

  ExprList() {}
  ~ExprList() {}

  std::string toString();
};

class Condition {
 private:
  std::string text;
  std::vector<std::pair<std::string, int> > varKeys;
  std::pair<QueryProcessor*, OpTree> opTree;
  std::vector<Attribute*> pointers; // for each expression like X.card
  bool treeOk;

 public:
  Condition() : treeOk(false) {}
  ~Condition() {}
  
  std::string toString() const;
  int convertVarKey(const char *varKey, Tuple *t = 0, ListExpr tupleType = 0);
  void clear();
  static std::string getType(int t, Tuple *tuple = 0, ListExpr ttype = 0);
  bool initOpTree(Tuple *tuple = 0, ListExpr ttype = 0);
  void deleteOpTree();
  
  std::string  getText() const          {return text;}
  void    setText(std::string newText)  {text = newText;}
  int     getVarKeysSize() const   {return varKeys.size();}
  std::string  getVar(unsigned int pos) {
                      std::string s; 
                      return (pos < varKeys.size() ?
                             varKeys[pos].first : s);}
  int     getKey(unsigned int pos) const {return (pos < varKeys.size() ?
                                            varKeys[pos].second : -1);}
//   int     getPId(unsigned int pos) {return (pos < pIds.size() ?
//                                                   pIds[pos] : -1);}
  void    clearVectors()           {varKeys.clear();}
//   string  getVar(unsigned int pos) {return (pos < vars.size() ?
//                                                   vars[pos] : "");}
  void    setOpTree(std::pair<QueryProcessor*, OpTree> qp_op) {opTree = qp_op;}
  void    setPointers(std::vector<Attribute*> ptrs)  {pointers = ptrs;}
  void    setValuePtr(unsigned int pos, std::string& value);
  void    setValuePtr(unsigned int pos, std::pair<std::string, 
                      unsigned int>& value);
  void    clearTimePtr(unsigned int pos);
  void    mergeAddTimePtr(unsigned int pos, 
                         temporalalgebra::Interval<Instant>& value);
  void    setStartEndPtr(unsigned int pos, Instant& value);
  void    setCardPtr(unsigned int pos, int value);
  void    cleanLabelsPtr(unsigned int pos);
  void    appendToLabelsPtr(unsigned int pos, std::string& value);
  void    appendToLabelsPtr(unsigned int pos, std::set<std::string>& values);
  void    cleanPlacesPtr(unsigned int pos);
  void    appendToPlacesPtr(unsigned int pos, 
                            std::pair<std::string,unsigned int>& value);
  void    appendToPlacesPtr(unsigned int pos, 
                  std::set<std::pair<std::string, unsigned int> >& values);
  void    setLeftRightclosedPtr(unsigned int pos, bool value);
  QueryProcessor* getQP()          {return opTree.first;}
  OpTree  getOpTree()              {return opTree.second;}
  int     getPointersSize()        {return pointers.size();}
  bool    isTreeOk()               {return treeOk;}
  void    setTreeOk(bool value)    {treeOk = value;}
  template<class M>
  void    restrictPtr(const int pos, M *traj, const int from, const int to, 
                      Tuple *tuple, ListExpr ttype, const int key);
  template<class M>
  void    setPointerToValue(const int pos, M *traj, const int from, 
                            const int to);
  template<class M>
  void    setPointerToEmptyValue(const int pos, M *traj, Tuple *t = 0, 
                                 ListExpr ttype = 0);
  template<class M>
  bool    evaluate(const IndexMatchInfo &imi, M *traj,
                   std::map<std::string, int> &varToElem,
                   Tuple *tuple = 0, ListExpr ttype = 0);
};

/*
\section{Class ~PatElem~}

*/
class PatElem {
  friend class TMatch;
  friend class TMatchIndexLI;
 private:
  std::string var;
  std::set<std::string> ivs;
  std::set<std::string> lbs;
  std::vector<std::pair<Word, SetRel> > values;
  std::vector<std::string> types;
  Wildcard wc;
  SetRel setRel;
  bool ok;

 public:
  PatElem() : var(""), ivs(), lbs(), values(), types(), wc(NO), 
              setRel(STANDARD), ok(true) {}
  PatElem(const char* contents, Tuple *tuple);
  PatElem(const PatElem& elem) : var(elem.var), ivs(elem.ivs), lbs(elem.lbs),
           values(elem.values), types(elem.types), wc(elem.wc), 
           setRel(elem.setRel), ok(elem.ok) {}
  ~PatElem() {}

  void stringToSet(const std::string& input, const bool isTime);
  void setVar(const std::string& v) {var = v;}
  PatElem& operator=(const PatElem& elem) {
    var = elem.var;
    ivs = elem.ivs;
    lbs = elem.lbs; 
    values = elem.values;
    types = elem.types;
    wc = elem.wc; 
    setRel = elem.setRel;
    ok = elem.ok;
    return *this;
  }

  void     getV(std::string& result) const                     {result = var;}
  void     getL(std::set<std::string>& result) const           {result = lbs;}
  SetRel   getSetRel() const                                   {return setRel;}
  void     getI(std::set<std::string>& result) const           {result = ivs;}
  Wildcard getW() const                                        {return wc;}
  bool     isOk() const                                        {return ok;}
  void     clearL()                                            {lbs.clear();}
  void     insertL(const std::string& lb)                      {lbs.insert(lb);}
  void     clearI()                                            {ivs.clear();}
  void     insertI(std::string& iv)                            {ivs.insert(iv);}
  void     clearW()                                            {wc = NO;}
  bool     hasLabel() const                             {return lbs.size() > 0;}
  bool     hasInterval() const                          {return ivs.size() > 0;}
  bool     hasRealInterval() const;
  bool     hasIndexableContents() const            {return (hasLabel() ||
                                                            hasRealInterval());}
  int      getNoValues() const                           {return values.size();}
  bool     hasValuesWithContent() const;
  bool     isRelevantForTupleIndex() const;
  bool     extractValues(std::string &input, Tuple *tuple);
  std::vector<std::pair<Word, SetRel> > getValues() const      {return values;}
  void   deleteValues(std::vector<std::pair<int, std::string> > &relevantAttrs);
};

/*
\section{Class ~Assign~}

*/
class Assign {
 private:
  int resultPos;
  bool occurrence; // ~true~ if and only if ~var~ occurs in the pattern
  std::string text[10]; // one for each possible attribute
  std::string var; // the assigned variable
  std::vector<std::pair<std::string, int> > right[11];
                      // list of vars and keys for every type
  std::pair<QueryProcessor*, OpTree> opTree[10];
  std::vector<Attribute*> pointers[10]; // for each expression like X.card
  bool treesOk;

 public:
  Assign() {treesOk = false;}
  ~Assign() {}

  bool convertVarKey(const char* vk, Tuple *tuple = 0, ListExpr tupleType = 0);
  bool prepareRewrite(int key, const std::vector<size_t> &assSeq,
                      std::map<std::string, int> &varPosInSeq, 
                      stj::MLabel const &ml);
  bool hasValue() {return (!text[0].empty() || !text[1].empty() ||
                           !text[8].empty() || !text[9].empty());}
  bool hasTime() {return (!text[2].empty() || 
                         (!text[3].empty() && !text[4].empty()));}
  bool initOpTrees();
  void clear();
  void deleteOpTrees();
  void setLabelPtr(unsigned int pos, const std::string& value);
  void setPlacePtr(unsigned int pos, const std::pair<std::string, 
                   unsigned int>& value);
  void setTimePtr(unsigned int pos, const temporalalgebra::SecInterval& value);
  void setStartPtr(unsigned int pos, const Instant& value);
  void setEndPtr(unsigned int pos, const Instant& value);
  void setLeftclosedPtr(unsigned int pos, bool value);
  void setRightclosedPtr(unsigned int pos, bool value);
  void cleanLabelsPtr(unsigned int pos);
  void appendToLabelsPtr(unsigned int pos, const std::string& value);
  void appendToLabelsPtr(unsigned int pos, const std::set<std::string>& value);
  void cleanPlacesPtr(unsigned int pos);
  void appendToPlacesPtr(unsigned int pos, 
                         const std::pair<std::string,unsigned int>& value);
  void appendToPlacesPtr(unsigned int pos, 
               const std::set<std::pair<std::string,unsigned int> >& value);
  
  void    init(std::string v, int pp)             {clear(); var = v;
                                              occurrence = (pp > -1);}
  int     getResultPos() const               {return resultPos;}
  void    setResultPos(int p)                {resultPos = p;}
  bool    occurs() const                     {return occurrence;}
  void    setOccurrence(int p)               {occurrence = (p > -1);}
  std::string  getText(int key) const             {return text[key];}
  void    setText(int key, std::string newText)   {if (!text[key].empty()) {
                                               right[key].clear();}
                                              text[key] = newText;}
  int     getRightSize(int key) const        {return right[key].size();}
  std::string  getV() const                       {return var;}
  int     getRightKey(int lkey, int j) const {return right[lkey][j].second;}
  std::pair<std::string, int> getVarKey(int key, int i) const {
       return right[key][i];
  }
  std::pair<std::string, int> getVarKey(int key) const {
      return right[key].back();
  }
  std::string  getRightVar(int lkey, int j) const {return right[lkey][j].first;}
  void    addRight(int key,
               std::pair<std::string, int> newRight)   {
    right[key].push_back(newRight);
  }
  void    removeUnordered()                  {if (!right[10].empty()) {
                                                right[10].pop_back();
                                              }}
  QueryProcessor* getQP(unsigned int key)    {if (key < 10) {
                                                return opTree[key].first;}
                                              else return 0;}
  OpTree  getOpTree(unsigned int key)        {if (key < 10) {
                                                return opTree[key].second;}
                                              else return 0;}
  bool    areTreesOk()                       {return treesOk;}
  void    setTreesOk(bool value)             {treesOk = value;}
};

/*
\section{Class ~Pattern~}

*/
class Pattern {
  friend class TMatch;  
 public:
  Pattern() {}

  Pattern(int i) {}

  ~Pattern() {
    deleteEasyCondOpTrees();
    deleteCondOpTrees();
    deleteAssignOpTrees();
  }

  std::string GetText() const;
  bool isValid(const std::string& type) const;
  bool isCompatible(TupleType *ttype, const int majorAttrNo, 
                  std::vector<std::pair<int, std::string> >& relevantAttrs,
                  int& majorValueNo);
  static Pattern* getPattern(std::string input, bool classify, Tuple *tuple = 0,
                             ListExpr ttype = 0);
  template<class M>
  ExtBool matches(M *m);
  ExtBool tmatches(Tuple *tuple, const int attrno, ListExpr ttype);
  int getResultPos(const std::string v);
  void collectAssVars();
  void addVarPos(std::string var, int pos);
  void addAtomicPos();
  int getPatternPos(const std::string v);
  bool checkAssignTypes();
  static std::pair<std::string, Attribute*> getPointer(int key, 
        Tuple *tuple = 0);
  bool initAssignOpTrees();
  void deleteAssignOpTrees(bool conds);
  bool parseNFA();
  void simplifyNFA(std::vector<std::map<int, int> >& result);
  void findNFApaths(std::vector<std::map<int, int> >& simpleNFA, 
                    std::set<std::pair<std::set<int>, int> >& result);
  void getCrucialElems(const std::set<std::pair<std::set<int>,int> >& paths, 
                       std::set<int>& result);
  bool containsFinalState(std::set<int> &states);
  bool initCondOpTrees(Tuple *tuple = 0, ListExpr ttype = 0);
  bool initEasyCondOpTrees(Tuple *tuple = 0, ListExpr ttype = 0);
  void deleteCondOpTrees();
  void deleteEasyCondOpTrees();
  void deleteAtomValues(std::vector<std::pair<int, std::string> > &attrs);
  bool startsWithAsterisk() const;
  bool startsWithPlus() const;
  bool nfaHasLoop(const int state) const;

  std::vector<PatElem>   getElems()              {return elems;}
  std::vector<Condition>* getConds()             {return &conds;}
  bool              hasConds()              {return conds.size() > 0;}
  bool              hasEasyConds()          {return easyConds.size() > 0;}
  std::vector<Condition> getEasyConds()          {return easyConds;}
  std::vector<Assign>&   getAssigns()            {return assigns;}
  void              getElem(int pos, PatElem& elem) const  {elem = elems[pos];}
  Condition         getCond(int pos) const  {return conds[pos];}
  Condition         getEasyCond(int pos) const {return easyConds[pos];}
  Assign           getAssign(int pos) const {return assigns[pos];}
  std::set<int>          getFinalStates() const  {return finalStates;}
  bool              hasAssigns()            {return !assigns.empty();}
  void              addPatElem(PatElem pElem)      {elems.push_back(pElem);}
  void              addRegExSymbol(const char* s) {regEx += s;}
  void              addCond(Condition cond) {conds.push_back(cond);}
  void              addEasyCond(int pos, Condition cond) {
    easyCondPos[pos].insert(easyConds.size());
    easyConds.push_back(cond);
  }
  void              addAssign(Assign ass)  {assigns.push_back(ass);}
  void              setText(std::string newText) {text = newText;}
  std::pair<int, int>    getVarPos(std::string var)   {return varPos[var];}
  int               getSize() const         {return elems.size();}
  std::map<std::string, std::pair<int, int> > getVarPos()  {return varPos;}
  std::map<int, std::set<int> > getEasyCondPos()      {return easyCondPos;}
  std::set<int>          getEasyCondPos(const int e) {return easyCondPos[e];}
  void              insertAssVar(std::string v)  {assignedVars.insert(v);}
  std::set<std::string>       getAssVars()            {return assignedVars;}
  void setAssign(int posR, int posP, int key, std::string arg) {
            assigns[posR].setText(key, arg); assigns[posR].setOccurrence(posP);}
  void addAssignRight(int pos, int key, std::pair<std::string, int> varKey)
                                           {assigns[pos].addRight(key, varKey);}
  void              getNFA(std::vector<std::map<int, int> >& result) {
        result = nfa;
  }
  int               getNFAsize() const      {return nfa.size();}
  bool              isNFAempty() const      {return (nfa.size() == 0);}
  std::map<int, int>     getTransitions(int pos) {assert(pos >= 0);
    assert(pos < (int)nfa.size()); return nfa[pos];}
  bool              isFinalState(int state) {return finalStates.find(state)
                                                    != finalStates.end();}
  void              setNFA(std::vector<std::map<int, int> > &_nfa, 
                    std::set<int> &fs) {
    nfa = _nfa; finalStates = fs;
  }
  void              eraseTransition(int state, int pE) {nfa[state].erase(pE);}
  void              setDescr(std::string desc)   {description = desc;}
  std::string            getDescr()              {return description;}
  void deleteAssignOpTrees()   {for (unsigned int i = 0;i < assigns.size();i++){
                                  assigns[i].deleteOpTrees();}}
  std::string            getRegEx()              {return regEx;}
  bool              containsRegEx()         {return
                        regEx.find_first_of("()|") != std::string::npos;}
  void  addRelevantVar(std::string var) {relevantVars.insert(var);}
  bool  isRelevant(std::string var)  {return relevantVars.count(var);}
  std::string       getVarFromElem(int elem){return elemToVar[elem];}
  std::map<std::string, int> getVarToElem() {return varToElem;}
  void              setVarToElem(std::map<std::string, int> &vte)
                                                              {varToElem = vte;}
  int               getElemFromVar(std::string var) {
                                   if (varToElem.find(var) == varToElem.end()) {
                                     return -1;
                                   }
                                   return varToElem[var];}
  int               getElemFromAtom(int atom) {return atomicToElem[atom];}
  int               get1stAtomFromElem(int elem) {
                    for (std::map<int, int>::iterator it = atomicToElem.begin();
                                            it != atomicToElem.end(); it++) {
                      if (it->second == elem) {
                        return it->first;
                      }
                    }
                    return -1;
  }

  std::vector<PatElem> elems;
  std::vector<Assign> assigns;
  std::vector<Condition> easyConds; // evaluated during matching
  std::vector<Condition> conds; // evaluated after matching
  std::string text, description, regEx;
  std::map<std::string, std::pair<int, int> > varPos;
  std::map<int, int> atomicToElem;
  std::map<int, std::string> elemToVar;
  std::map<std::string, int> varToElem;
  std::map<int, std::set<int> > easyCondPos;
  std::set<std::string> assignedVars; 
                         // variables on the right side of an assignment
  std::set<std::string> relevantVars; 
                         // variables that occur in conds, results, assigns
  std::vector<std::map<int, int> > nfa;
  std::set<int> finalStates;
};

/*
\section{Class ~PatPersistent~}

*/
class PatPersistent : public Label {
 public:
  PatPersistent() {}
  PatPersistent(int i) : Label(i > 0) {}
  PatPersistent(PatPersistent& src) : Label(src.toText()) {}
  
  ~PatPersistent() {}
   
  std::string toText() const {
          std::string value; 
          Label::GetValue(value); 
          return value;
  }
  template<class M>
  ExtBool matches(M *traj) {
    Pattern *p = Pattern::getPattern(toText(), false);
    if (p) {
      ExtBool result = p->matches(traj);
      delete p;
      return result;
    }
    else {
      return ST_UNDEF;
    }
  }
  
  static const std::string BasicType() {return "pattern";}
  static const bool checkType(const ListExpr type) {
    return listutils::isSymbol(type, BasicType());
  }
};

extern TypeConstructor classifierTC;

/*
\section{Class ~Classifier~}

*/
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

  static const std::string BasicType() {return "classifier";}
  int getCharPosSize() const {return charpos.Size();}
  int getNumOfP() const {return charpos.Size() / 2;}
  int getCharSize() const {return chars.Size();}
  void appendCharPos(int pos) {charpos.Append(pos);}
  void appendChar(char ch) {chars.Append(ch);}
  void SetDefined(const bool def) {defined = def;}
  bool IsDefined() const {return defined;}
  bool IsEmpty() const {return (chars.Size() == 0);}
  std::string getDesc(int pos);
  std::string getPatText(int pos);
  void buildMultiNFA(std::vector<Pattern*> patterns, 
                     std::vector<std::map<int, int> > &nfa,
                     std::set<int> &finalStates, std::map<int, int> &state2Pat);
  void setPersistentNFA(std::vector<std::map<int, int> > &nfa, 
                        std::set<int> &finalSt,
                        std::map<int, int> &state2Pat) {
    delta.clean();
    s2p.clean();
    Tools::makeNFApersistent(nfa, finalSt, delta, s2p, state2Pat);
  }
  DbArray<NFAtransition> *getDelta() {return &delta;}
  int getNumOfState2Pat() {return s2p.Size();}
  void getStartStates(std::set<int> &startStates);

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
  std::vector<std::map<int, int> > nfa; // multiNFA (not persistent)
  DbArray<int> charpos;
  DbArray<char> chars;
  DbArray<NFAtransition> delta; // multiNFA (persistent)
  DbArray<int> s2p; // neg: start; pos: final; INT_MAX: default
  bool defined;
};

/*
\section{Class ~TMatch~}

*/
class TMatch {
 private:
  Pattern *p;
  Tuple *t;
  ListExpr ttype;
  std::set<int>** matching;
  int attrno, valueno;
  DataType type;
  std::vector<std::pair<int, std::string> > relevantAttrs;
  std::set<unsigned int>** pathMatrix; // stores the whole matching process
  
 public:
  TMatch(Pattern *pat, Tuple *tuple, ListExpr tt, const int _attrno, 
         std::vector<std::pair<int, std::string> >& _relevantAttrs, 
         const int _valueno);
  
  ~TMatch() {}
  
  ExtBool matches();
  int GetNoMainComponents() const;
  void GetInterval(const int u, temporalalgebra::SecInterval& iv) const;
  bool labelsMatch(const std::set<std::string>& tlabels, 
                   const int atom, const int pos);
  bool placesMatch(
            const std::set<std::pair<std::string, unsigned int> >& tlabels, 
            const int atom, const int pos);
  bool mainValuesMatch(const int u, const int atom);
  bool otherValuesMatch(const int pos, 
                        const temporalalgebra::SecInterval& iv, const int atom);
  bool valuesMatch(const int u, const int atom);
  bool easyCondsMatch(const int u, const int atom);
  bool performTransitions(const int u, std::set<int>& states);
  bool performTransitionsWithMatrix(const int u, std::set<int>& states);
  bool findMatchingBinding(const int startState);
  bool conditionsMatch(const IndexMatchInfo& imi);
  bool findBinding(const int u, const int atom, IndexMatchInfo& imi);
};

extern TypeConstructor tupleindexTC;

/*
\section{Class ~TupleIndex~}

*/
class TupleIndex {
  friend class TMatchIndexLI;
 public:
  TupleIndex() {}
  TupleIndex(std::vector<InvertedFile*> t, std::vector<BTree_t<LongInt>*> b,
             std::vector<RTree1TLLI*> r1, 
             std::vector<RTree2TLLI*> r2, RTree1TLLI *tI, 
             std::map<int, std::pair<IndexType, int> > aI,
             std::map<std::pair<IndexType, int>, int> iA, int mA);
  TupleIndex(bool dummy) {}
  TupleIndex(TupleIndex &src);
  ~TupleIndex() {deleteIndexes();}
  
  static const std::string BasicType() {return "tupleindex";}
  static bool checkType(const ListExpr list);
  static ListExpr Property();
  static Word In(const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr& errorInfo, bool& correct);
  static ListExpr Out(ListExpr typeInfo, Word value);
  static Word Create(const ListExpr typeInfo);
  static void Delete(const ListExpr typeInfo, Word& w);
  static bool Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value);
  static bool Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value);
  static void Close(const ListExpr typeInfo, Word& w);
  static Word Clone(const ListExpr typeInfo, const Word& w);
  static int SizeOfObj();
  static bool TypeCheck(ListExpr type, ListExpr& errorInfo);

  void initialize(TupleType *ttype, int _mainAttr);
  bool addTuple(Tuple *tuple);
  static void insertIntoTrie(InvertedFile *inv, TupleId tid, Attribute *traj, 
                           DataType type, appendcache::RecordAppendCache* cache,
                             TrieNodeCacheType* trieCache);
  static bool fillTimeIndex(RTree1TLLI* rt, TupleId tid, Attribute *traj,
                            DataType type);
  static void insertIntoBTree(BTree_t<LongInt> *bt, TupleId tid,
                              temporalalgebra::MInt *mint);
  static bool insertIntoRTree1(RTree1TLLI *rt, TupleId tid, Attribute *m);
  static bool insertIntoRTree2(RTree2TLLI *rt, TupleId tid, Attribute *m,
                               std::string type);
  void deleteIndexes();
  
  void processTimeIntervals(Relation *rel, const int attr, 
                            const std::string &typeName);
  void processRTree2(Relation *rel, const int attrNo,
                     const std::string &typeName);
  void processRTree1(Relation *rel, const int attr);
  void processBTree(Relation *rel, const int attr);
  void processTrie(Relation *rel, const int attr, const std::string &typeName,
                   const size_t memSize);
  void collectSortInsert(Relation *rel, const int attrPos, 
                         const std::string &typeName, const size_t memSize);

 private:
  std::vector<InvertedFile*> tries;
  std::vector<BTree_t<LongInt>*> btrees;
  std::vector<RTree1TLLI*> rtrees1;
  std::vector<RTree2TLLI*> rtrees2;
  RTree1TLLI *timeIndex;
  std::map<int, std::pair<IndexType, int> > attrToIndex;
  std::map<std::pair<IndexType, int>, int> indexToAttr;
  int mainAttr;
};

/*
\section{Struct ~UnitSet~}

*/
struct UnitSet {
  UnitSet() {}
  UnitSet(std::set<int>& u) : units(u) {}
  UnitSet(int unit) {units.insert(unit);}
  
  std::set<int> units;
};

/*
\section{Struct ~IndexRetrieval~}

*/
struct IndexRetrieval : public UnitSet {
  IndexRetrieval() : pred(0), succ(0) {}
  IndexRetrieval(unsigned int p, unsigned int s = 0) : pred(p), succ(s) {}
  IndexRetrieval(unsigned int p, unsigned int s, std::set<int>& u) : 
                 UnitSet(u), pred(p), succ(s) {}
  IndexRetrieval(unsigned int p, unsigned int s, std::vector<int>& u) : 
                 pred(p), succ(s) {units.insert(u.begin(), u.end());}
  
  unsigned int pred, succ;
};

/*
\section{Struct ~IndexMatchInfo~}

*/
struct IndexMatchInfo {
  IndexMatchInfo() {}
  
  IndexMatchInfo(std::vector<int> &b) : binding(b) {}
  
  IndexMatchInfo(bool r, int n = 0) : 
       range(r), next(n), prevElem(-1) {binding.clear();}
       
  IndexMatchInfo(bool r, int n, const std::vector<int> b, const int e) :
       range(r), next(n), prevElem(e) {if (b.size() > 0) {binding = b;}}
       
  IndexMatchInfo(const int elem, const int u) : range(false), next(-1), 
       prevElem(-1) {
    for (int i = 0; i < elem - 1; i++) {
      binding.push_back(-1);
    }
    if (u > 0) {
      binding.push_back(u - 1);
    }
    binding.push_back(u);
  }
  
  void print(const bool printBinding);
  bool finished(const int size) const {return range || (next >= size);}
  bool exhausted(const int size) const {return next >= size;}
  bool matches(const int unit) const {return (range ? next<=unit : next==unit);}
  
  void set(const int elem, const int to) {
    if (elem >= 0 && (int)binding.size() > elem) {
      binding[elem] = to;
    }
    else if (elem >= 0) {
      while ((int)binding.size() < elem) {
        binding.push_back(-1);
      }
      binding.push_back(to);
    }
  }
  
  int getTo(const int elem) const {
    if ((int)binding.size() > elem) {
      return binding[elem];
    }
    return -1;
  }
  
  int getFrom(const int elem) const {
    if (elem >= -1 && (int)binding.size() > elem) {
      if (binding[elem] == -1) {
        return -1;
      }
      for (int i = elem - 1; i >= 0; i--) {
        if (binding[i] != -1) {
          return binding[i] + 1;
        }
      }
      return 0;
    }
    return -1;
  }
  
  bool extendOrInsert(const int elem, const int u) {
    if ((int)binding.size() > elem) {
      binding[elem]++;
      return false;
    }
    else {
      for (int i = binding.size(); i < elem; i++) {
        binding.push_back(-1);
      }
      binding.push_back(u);
      return true;
    }
  }
  
  void reduceOrErase(const int elem, const bool inserted) {
    if (inserted) {
      binding.pop_back();
    }
    else {
      binding[elem]--;
    }
  }
  
  void reset(const int oldEnd) {
    if (prevElem >= 0 && (int)binding.size() > prevElem) {
      binding[prevElem] = oldEnd;
    }
  }
  
  bool range;
  int next, prevElem;
  std::vector<int> binding; //holds right ends of bindings for each pattern elem
};

struct cmp {
  bool operator()(const IndexMatchInfo& imi1, const IndexMatchInfo& imi2) {
    return imi1.binding < imi2.binding;
  }
};

/*
\section{Struct ~IndexMatchSlot~}

*/
struct IndexMatchSlot {
  IndexMatchSlot() : pred(0), succ(0) {}
  IndexMatchSlot(IndexMatchInfo &imi) : pred(0), succ(0) {imis.push_back(imi);}
  IndexMatchSlot(IndexMatchSlot *src) : pred(src->pred), succ(src->succ),
                                        imis(src->imis) {}
  
  unsigned int pred, succ;
  std::vector<IndexMatchInfo> imis;
};

/*
\section{struct ~DoubleUnitSet~}

*/
struct DoubleUnitSet {
  DoubleUnitSet() {}
  
  DoubleUnitSet(std::set<int>& u, bool m = true) {
    if (m) {
      match = u;
    }
    else {
      mismatch = u;
    }
  }
  
  DoubleUnitSet(int u, bool m = true) {
    if (m) {
      match.insert(u);
    }
    else {
      mismatch.insert(u);
    }
  }
  
  ExtBool getMatchRecord(int u) {
    if (match.find(u) != match.end()) {
      return ST_TRUE;
    }
    if (mismatch.find(u) != mismatch.end()) {
      return ST_FALSE;
    }
    return ST_UNDEF;
  }
  
  void addMatchRecord(int u, bool m = true) {
    if (m) {
      match.insert(u);
    }
    else {
      mismatch.insert(u);
    }
  }
  
  std::set<int> match;
  std::set<int> mismatch;
};

/*
\section{struct ~DoubleBindingSet~}

*/
struct DoubleBindingSet {
  DoubleBindingSet() {}
  
  DoubleBindingSet(IndexMatchInfo& imi, bool m = true) {
    if (m) {
      match.insert(imi.binding);
    }
    else {
      mismatch.insert(imi.binding);
    }
  }
  
  ExtBool getCondRecord(IndexMatchInfo &imi) {
    if (match.find(imi.binding) != match.end()) {
      return ST_TRUE;
    }
    if (mismatch.find(imi.binding) != mismatch.end()) {
      return ST_FALSE;
    }
    return ST_UNDEF;
  }
  
  void addCondRecord(IndexMatchInfo &imi, bool m = true) {
    if (m) {
      match.insert(imi.binding);
    }
    else {
      mismatch.insert(imi.binding);
    }
  }
  
  std::set<std::vector<int> > match;
  std::set<std::vector<int> > mismatch;
};

/*
\section{class ~IndexMatchSuper~}

*/
class IndexMatchSuper {
 public:
  IndexMatchSuper(Relation *r, Pattern *_p, int a, DataType t) :
    rel(r), p(_p), attrNo(a), mtype(t), matchRecord(0), condRecord(0),
    trajSize(0), activeTuples(0), unitCtr(0), indexResult(0), matchInfo(0), 
    newMatchInfo(0) {}
  
  ~IndexMatchSuper();
  
  int getTrajSize(const TupleId tId, const DataType type);
  void getInterval(const TupleId tId, const int pos, 
                   temporalalgebra::SecInterval& iv);
  void periodsToUnits(const temporalalgebra::Periods *per, 
                      const TupleId tId, std::set<int> &units);
  template<class M>
  void periodsToUnits(const temporalalgebra::Periods *per, 
                      const TupleId tId, std::set<int> &units);
  void remove(std::vector<TupleId> &toRemove);
  void removeIdFromIndexResult(const TupleId id);
  void removeIdFromMatchInfo(const TupleId id);
  bool canBeDeactivated(const TupleId id, const int state, const int atom,
                        const bool checkRange = false) const;
  void clearMatchInfo();
  bool hasIdIMIs(const TupleId id, const int state = -1);
  void extendBinding(IndexMatchInfo& imi, const int atom, const bool wc,
                     const TupleId id = 0);
  void deletePattern();
  
  Relation *rel;
  Pattern *p;
  int attrNo;
  std::vector<std::pair<int, std::string> > relevantAttrs;
  DataType mtype;
  std::vector<TupleId> matches;
  std::vector<NewPair<TupleId, IndexMatchInfo> > matchesR; // rewrite
  DoubleUnitSet ***matchRecord; // state x tuple id
  DoubleBindingSet **condRecord; // tuple id
  bool *active;
  int *trajSize;
  int activeTuples, unitCtr;
  std::set<int> loopStates, crucialAtoms;
  IndexRetrieval ***indexResult;
  IndexMatchSlot ***matchInfo, ***newMatchInfo;
};

/*
\section{Class ~TMatchIndexLI~}

*/
class TMatchIndexLI : public IndexMatchSuper {
  
 public:
  TMatchIndexLI(Relation *r, ListExpr tt, TupleIndex *t, int a, Pattern *pat,
                int majorValueNo, DataType type);
  
  ~TMatchIndexLI() {};
  
  bool tiCompatibleToRel();
  bool getSingleIndexResult(
             std::pair<int, std::pair<IndexType, int> > indexInfo, 
             std::pair<Word, SetRel> values, std::string type,
             int valueNo, std::vector<std::set<int> > &result);
  int getNoComponents(const TupleId tId, const int attrNo);
  void unitsToPeriods(const std::set<int> &units, const TupleId tId, 
                      const int attr, temporalalgebra::Periods *per);
  template<class M, class U>
  void unitsToPeriods(Attribute *traj, const std::set<int> &units, 
                      temporalalgebra::Periods *per);
  void getResultForAtomPart(
                           std::pair<int, std::pair<IndexType, int> > indexInfo,
                           std::pair<Word, SetRel> values, std::string type,
                           std::vector<temporalalgebra::Periods*> &prev,
                           std::vector<temporalalgebra::Periods*> &result,
                           bool checkPrev = false);
  bool getResultForAtomTime(const int atomNo, 
                            std::vector<temporalalgebra::Periods*> &result);
  void storeIndexResult(int atomNo, int &noResults);
  void initMatchInfo();
  bool atomMatch(int state, std::pair<int, int> trans,
                 const bool rewrite = false);
  void applyNFA(const bool rewrite = false);
  bool initialize(const bool rewrite = false);
  Tuple* nextTuple();
  
 private:
  ListExpr ttList;
  TupleIndex *ti;
  int valueNo;
};

/*
\section{Class ~IndexRewriteLI~}

*/
template<class M>
class IndexRewriteLI : public TMatchIndexLI {
 public:
  IndexRewriteLI(Relation *r, ListExpr tt, TupleIndex *t, int a, Pattern *pat,
                int majorValueNo, DataType type) : 
    TMatchIndexLI(r, tt, t, a, pat, majorValueNo, type) {}
 
  M* nextResult();
 private:

};

/*
\section{Class ~Match~}

*/
template<class M>
class Match {
 public:
  Pattern *p;
  M *m; // mlabel, mplace, mlabels, mplaces
  std::set<unsigned int>** matching; // stores the whole matching process
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
  bool updateStates(int i, std::vector<std::map<int, int> > &nfa, 
                    std::vector<PatElem> &elems,
                    std::set<int> &finalStates, std::set<int> &states,
                    std::vector<Condition> &easyConds, 
                    std::map<int, std::set<int> > &easyCondPos,
                    std::map<int, int> &atomicToElem, bool store = false);
  bool easyCondsMatch(int ulId, PatElem const &elem,
                      std::vector<Condition> &easyConds, std::set<int> pos);
  std::string states2Str(int ulId, std::set<int> &states);
  std::string matchings2Str(unsigned int dim1, unsigned int dim2);
  bool findMatchingBinding(std::vector<std::map<int, int> > &nfa, 
                     int startState,
                     std::vector<PatElem> &elems, std::vector<Condition> &conds,
                     std::map<int, int> &atomicToElem, 
                     std::map<int, std::string> &elemToVar);
  bool findBinding(unsigned int ulId, 
                   unsigned int pId, std::vector<PatElem> &elems,
                   std::vector<Condition> &conds, 
                   std::map<int, std::string> &elemToVar, IndexMatchInfo &imi);
  void cleanPaths(std::map<int, int> &atomicToElem);
  bool cleanPath(unsigned int ulId, unsigned int pId);
  bool conditionsMatch(std::vector<Condition> &conds,const IndexMatchInfo &imi);
  bool evaluateEmptyM();
  bool isSensiblyBound(const std::map<std::string, std::pair<int, int> > &b,
                       std::string& var);
  void deletePattern() {if (p) {delete p; p = 0;}}
  bool initEasyCondOpTrees() {return p->initEasyCondOpTrees();}
  bool initCondOpTrees() {return p->initCondOpTrees();}
  bool initAssignOpTrees() {return p->initAssignOpTrees();}
  void deleteSetMatrix() {if (matching) {
                       Tools::deleteSetMatrix(matching, m->GetNoComponents());}}
  void createSetMatrix(unsigned int dim1, unsigned int dim2) {
    matching = Tools::createSetMatrix(dim1, dim2);}
  void setM(M *newM) {m = newM;}
  std::vector<int> applyConditions(ClassifyLI* c);
  
  
  void setNFA(std::vector<std::map<int, int> > &nfa, std::set<int> &fs) {
         p->setNFA(nfa, fs);
   }
  void setNFAfromDbArrays(DbArray<NFAtransition> &trans, DbArray<int> &fs) {
    std::vector<std::map<int, int> > nfa;
    std::set<int> finalStates;
    Tools::createNFAfromPersistent(trans, fs, nfa, finalStates);
    p->setNFA(nfa, finalStates);
  }
  void setPattern(Pattern *pat) {p = pat;}
  Pattern* getPattern() {return p;}
  M* getM() {return m;}
  std::pair<std::string, Attribute*> getPointer(int key);
  static std::pair<QueryProcessor*, OpTree> processQueryStr(
           std::string query, int type);
};

/*
\section{Struct ~BindingStackElem~}

*/
struct BindingStackElem {
  BindingStackElem(unsigned int ul, unsigned int pe) : ulId(ul), peId(pe) {}

  unsigned int ulId, peId;
//   map<string, std::pair<int, int> > binding;
};

/*
\section{Class ~RewriteLI~}

*/
template<class M>
class RewriteLI {
 public:
  RewriteLI(M *src, Pattern *pat);
  RewriteLI(int i) : match(0) {}

  ~RewriteLI() {clearMatch(true);}

  void clearMatch(const bool deletePattern);
  M* getNextResult();
  M* rewrite(M *src, IndexMatchInfo &imi, std::vector<Assign> &assigns);
  void resetBinding(int limit);
  bool findNextBinding(unsigned int ulId, unsigned int pId, Pattern *p,
                       int offset);
  
 protected:
  std::stack<BindingStackElem> bindingStack;
  Match<M> *match;
  IndexMatchInfo imi;
  std::set<IndexMatchInfo, cmp> rewBindings;
};

/*
\section{Class ~ClassifyLI~}

*/
class ClassifyLI {

 friend class Match<MLabel>;
 friend class Match<MLabels>;
 friend class Match<MPlace>;
 friend class Match<MPlaces>;

 public:
  template<class M>
  ClassifyLI(M *traj, Word _classifier);
  ClassifyLI(int i) : classifyTT(0) {}

  ~ClassifyLI();

  static TupleType* getTupleType();
  FText* nextResultText();
  void printMatches();

 protected:
  std::vector<Pattern*> pats;
  TupleType* classifyTT;
  std::set<int> matchingPats;
};

/*
\section{Class ~MultiRewriteLI~}

*/
template<class M>
class MultiRewriteLI : public ClassifyLI, public RewriteLI<M> {
 public:
  MultiRewriteLI(Word _mlstream, Word _pstream, int _pos);

  ~MultiRewriteLI();

  M* nextResult();
  void getStartStates(std::set<int> &states);
  void initStack(std::set<int> &startStates);

 private:
  Stream<Tuple> tStream;
  int attrpos;
  bool streamOpen;
  M* traj;
  Classifier* c;
  std::map<int, int> state2Pat;
  std::set<int> finalStates, states, matchCands;
  std::vector<std::map<int, int> > nfa;
  std::vector<PatElem> patElems;
  std::vector<Condition> easyConds;
  std::map<int, std::set<int> > easyCondPos;
  std::map<int, std::pair<int, int> > patOffset; 
                // elem no |-> (pat no, first pat elem)
  std::map<int, int> atomicToElem;
  std::map<int, std::string> elemToVar;
  std::map<std::string, int> varToElem;
};

/*
\section{Class ~FilterMatchesLI~}

*/
template<class M>
class FilterMatchesLI {
 public:
  FilterMatchesLI(Word _stream, int _attrIndex, std::string& pText);

  ~FilterMatchesLI();

  Tuple* getNextResult();
  
 private:
  Stream<Tuple> stream;
  int attrIndex;
  Match<M>* match;
  bool streamOpen, deleteP;
};

/*
\section{Class ~IndexMatchesLI~}

*/
class IndexMatchesLI : public IndexMatchSuper {
 public:
  IndexMatchesLI(Relation *_rel, InvertedFile *inv, 
    R_Tree<1, NewPair<TupleId, int> > *rt, int _attrNr, Pattern *_p,
    DataType type);

  ~IndexMatchesLI();

  Tuple* nextTuple();
  void retrieveValue(std::vector<std::set<int> >& part, 
                     std::vector<std::set<int> >& part2,
                     SetRel rel, bool first, const std::string& label,
                     unsigned int ref = UINT_MAX);
  void retrieveTime(std::vector<std::set<int> >& oldPart, 
                    std::vector<std::set<int> >& newPart, 
                    const std::string& ivstr);
  void removeIdFromMatchInfo(const TupleId id);
  void storeIndexResult(const int e);
  void initMatchInfo(const std::set<int>& cruElems);
  void initialize();
  void applyNFA();
  template<class M>
  bool imiMatch(Match<M>& match, const int e, const TupleId id, 
                IndexMatchInfo& imi, const int unit, const int newState);
  bool valuesMatch(const int e, const TupleId id, IndexMatchInfo& imi,
                   const int newState, const int unit);
  void applySetRel(const SetRel setRel, 
              std::vector<std::set<std::pair<TupleId, int> > >& valuePosVec,
              std::set<std::pair<TupleId, int> >*& result);
  bool simpleMatch(const int e, const int state, const int newState);
  bool wildcardMatch(const int state, std::pair<int, int> trans);
  bool timesMatch(const TupleId id,const unsigned int unit,const PatElem& elem);
  bool checkConditions(const TupleId id, IndexMatchInfo& imi);

 protected:
  std::set<int> indexMismatch;
  InvertedFile* invFile;
  R_Tree<1, NewPair<TupleId, int> > *rtree;
  size_t maxMLsize;
  std::vector<bool> deactivated;
};

/*
\section{Class ~IndexClassifyLI~}

*/
class IndexClassifyLI : public IndexMatchesLI {
 public:
  IndexClassifyLI(Relation *rel, InvertedFile *inv, 
    R_Tree<1, NewPair<TupleId, int> > *rt, Word _classifier, int _attrNr, 
    DataType type);

  ~IndexClassifyLI();

  template<class M>
  Tuple* nextResultTuple();
  
 protected:
  TupleType* classifyTT;
  Classifier *c;
  std::queue<std::pair<std::string, TupleId> > results;
  int currentPat;
};

/*
\section{Class ~UnitsLI~}

*/
class UnitsLI {
 public:
  UnitsLI() : index(0) {}
  ~UnitsLI() {}

  int index;
};

/*
\section{Class ~DeriveGroupsLI~}

*/
template<class M>
class DeriveGroupsLI {
 public: 
  DeriveGroupsLI(Word _stream, double threshold, int attrNo);
  
  ~DeriveGroupsLI() {}
  
  Tuple *getNextTuple();
  
 private:
  std::vector<std::pair<TupleId, unsigned int> > result;
};

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
  size = rootSize + sizeof(size_t) + values.getSize() + units.GetFlobSize();
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
\subsection{Function ~Atinstant~}

*/
template<class B>
void MBasic<B>::Atinstant(const Instant& inst, IBasic<B>& result) const {
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
double MBasic<B>::Distance_FIRST_LAST(const MBasic<B>& mb) const {
  double result = 0.0;
  int n = GetNoComponents();
  int m = mb.GetNoComponents();
  typename B::base bs1, bs2, be1, be2;
  GetValue(0, bs1);
  mb.GetValue(0, bs2);
  GetValue(n - 1, be1);
  mb.GetValue(m - 1, be2);
  if (n == 1) {
    if (m == 1) {
      result = (bs1 == bs2 ? 0 : 1);
    }
    else {
      mb.GetValue(m - 1, be2);
      result = ((bs1 == bs2 ? 0 : 1) + 
                (bs1 == be2 ? 0 : 1)) / 2;
    }
  }
  else {
    GetValue(n - 1, be1);
    if (m == 1) {
      result = ((bs1 == bs2 ? 0 : 1) + 
                (be1 == bs2 ? 0 : 1)) / 2;
    }
    else {
      mb.GetValue(m - 1, be2);
      result = ((bs1 == bs2 ? 0 : 1) + 
                (be1 == be2 ? 0 : 1)) / 2;
    }
  }
  return result;
}

template<class B>
double MBasic<B>::Distance_ALL(const MBasic<B>& mb, const LabelFunction lf) 
                  const {
  int n = GetNoComponents();
  int m = mb.GetNoComponents();
  typename B::base b1, b2;
  double dp[n][m];
  for (int i = 0; i < n; i++) {
    dp[i][0] = i;
  }
  for (int j = 0; j < m; j++) {
    dp[0][j] = j;
  }
  for (int i = 1; i < n; i++) {
    GetValue(i - 1, b1);
    for (int j = 1; j < m; j++) {
      mb.GetValue(j - 1, b2);
      dp[i][j] = std::min(dp[i - 1][j] + 1,
                std::min(dp[i][j - 1] + 1, 
                    dp[i -1][j - 1] + Tools::distance(b1, b2, lf)));
    }
  }
  return dp[n - 1][m - 1] / std::max(n, m);
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


/*
\subsection{Function ~Distance~}

*/
template<class B>
double MBasic<B>::Distance(const MBasic<B>& mb) const {
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
  DistanceFunction df = ALL; // TODO: change
  LabelFunction lf = TRIVIAL;
  typename B::base b1, b2, bs1, bs2, be1, be2;
  switch (df) {
    case FIRST: {
      return this->Distance_FIRST(mb);
    }
    case LAST: {
      return this->Distance_LAST(mb);
    }
    case FIRST_LAST: {
      return this->Distance_FIRST_LAST(mb);
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
    default: {
      return -1.0;
    }
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
std::ostream& operator<<(std::ostream& o, const MBasics<B>& mbs) {
  o << nl->ToString(mbs.ToListExpr());
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
\subsection{Function ~GetValues~}

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
void MBasics<B>::Atinstant(const Instant& inst, 
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
\subsection{Function ~Distance~}

*/
template<class B>
double MBasics<B>::Distance(const MBasics<B>& mbs) const {
  if (!IsDefined() && !mbs.IsDefined()) {
    return 0;
  }
  if (!IsDefined() || !mbs.IsDefined()) {
    return 1;
  }
  if (IsEmpty() && mbs.IsEmpty()) {
    return 0;
  }
  if (IsEmpty() || mbs.IsEmpty()) {
    return 1;
  }
  int n = GetNoComponents() + 1;
  int m = mbs.GetNoComponents() + 1;
  double dp[n][m];
  for (int i = 0; i < n; i++) {
    dp[i][0] = i;
  }
  for (int j = 0; j < m; j++) {
    dp[0][j] = j;
  }
  std::set<typename B::base> basics1, basics2;
  int fun = 0; // TODO: change
  LabelFunction lf = TRIVIAL;
  for (int i = 1; i < n; i++) {
    GetValues(i - 1, basics1);
    for (int j = 1; j < m; j++) {
      mbs.GetValues(j - 1, basics2);
      dp[i][j] = std::min(dp[i - 1][j] + 1,
                 std::min(dp[i][j - 1] + 1, 
           dp[i -1][j - 1] + Tools::distance(basics1, basics2, fun, lf)));
    }
  }
  return dp[n - 1][m - 1] / std::max(n, m);
}

/*
\subsection{Function ~restrict~}

*/
template<class M>
void Condition::restrictPtr(const int pos, M *traj, const int from,
                    const int to, Tuple *tuple, ListExpr ttype, const int key) {
  temporalalgebra::Periods per(true);
  if (from > -1 && to > -1 && to >= from) {
    temporalalgebra::SecInterval iv(true), ivtemp(true);
    traj->GetInterval(from, iv);
    traj->GetInterval(to, ivtemp);
    iv.end = ivtemp.end;
    iv.rc = ivtemp.rc;
    per.Add(iv);
  }
  std::string attrtype = nl->ToString(nl->Second(nl->Nth(key, 
                                      nl->Second(ttype))));
  if (attrtype == "mbool") {
    ((temporalalgebra::MBool*)tuple->GetAttribute(key - 1))->AtPeriods(per,
                                   *((temporalalgebra::MBool*)pointers[pos]));
  }
  else if (attrtype == "mint") {
    ((temporalalgebra::MInt*)tuple->GetAttribute(key - 1))->AtPeriods(per, 
                                  *((temporalalgebra::MInt*)pointers[pos]));
  }
  else if (attrtype == "mlabel") {
    ((MLabel*)tuple->GetAttribute(key - 1))->AtPeriods(per, 
                                  *((MLabel*)pointers[pos]));
  }
  else if (attrtype == "mlabels") {
    ((MLabels*)tuple->GetAttribute(key - 1))->AtPeriods(per, 
                                  *((MLabels*)pointers[pos]));
  }
  else if (attrtype == "mplace") {
    MPlace mp(true);
    Place::base value;
    Region tmp(true);
    std::string type;
    Word geo;
    ((MPlace*)tuple->GetAttribute(key - 1))->AtPeriods(per, mp);
    for (int i = 0; i < mp.GetNoComponents(); i++) {
      mp.GetValue(i, value);
      if (value.second > 0) {
        Tools::getGeoFromORel("Places", value.second, false, geo, type);
        if (type == "point") {
          tmp.Union(*((Point*)geo.addr), (*((Region*)pointers[pos])));
        }
        else if (type == "line") {
          tmp.Union(*((Line*)geo.addr), (*((Region*)pointers[pos])));
        }
        else if (type == "region") {
          tmp.Union(*((Region*)geo.addr), (*((Region*)pointers[pos])));
        }
        else {
          cout << "ERROR: type is " << type << endl; // cannot occur
        }
      }
    }
  }
  else if (attrtype == "mplaces") {
    ((MPlaces*)tuple->GetAttribute(key - 1))->AtPeriods(per, 
                                                    *((MPlaces*)pointers[pos]));
  }
  else if (attrtype == "mpoint") {
    ((temporalalgebra::MPoint*)tuple->GetAttribute(key - 1))->AtPeriods(per, 
                                    *((temporalalgebra::MPoint*)pointers[pos]));
  }
  else if (attrtype == "mreal") {
    ((temporalalgebra::MReal*)tuple->GetAttribute(key - 1))->AtPeriods(per, 
                                 *((temporalalgebra::MReal*)pointers[pos]));
  }
  else if (attrtype == "mregion") {
    ((temporalalgebra::MRegion*)tuple->GetAttribute(key - 1))->AtPeriods(&per, 
                                     (temporalalgebra::MRegion*)pointers[pos]);
  }
}

/*
\subsection{Function ~setPointerToValue~}

*/
template<class M>
void Condition::setPointerToValue(const int pos, M *traj, const int from, 
                                  const int to) {
  temporalalgebra::SecInterval iv(true);
  switch (getKey(pos)) {
    case 0: { // label
      std::string value;
      ((MLabel*)traj)->GetValue(from, value);
      setValuePtr(pos, value);
      break;
    }
    case 1: { // place
      std::pair<std::string, unsigned int> value;
      ((MPlace*)traj)->GetValue(from, value);
      setValuePtr(pos, value);
      break;
    }
    case 2: { // time
      clearTimePtr(pos);
      for (unsigned int j = from; j <= to; j++) {
        traj->GetInterval(j, iv);
        mergeAddTimePtr(pos, iv);
      }
      break;
    }
    case 3: { // start
      traj->GetInterval(from, iv);
      setStartEndPtr(pos, iv.start);
      break;
    }
    case 4: { // end
      traj->GetInterval(to, iv);
      setStartEndPtr(pos, iv.end);
      break;
    }
    case 5: { // leftclosed
      traj->GetInterval(from, iv);
      setLeftRightclosedPtr(pos, iv.lc);
      break;
    }
    case 6: { // rightclosed
      traj->GetInterval(to, iv);
      setLeftRightclosedPtr(pos, iv.rc);
      break;
    }
    case 7: { // card
      setCardPtr(pos, to - from + 1);
      break;
    }
    case 8: { // labels
      cleanLabelsPtr(pos);
      if (M::BasicType() == MLabel::BasicType()) {
        for (unsigned int j = from; j <= to; j++) {
          std::string value;
          ((MLabel*)traj)->GetValue(j, value);
          appendToLabelsPtr(pos, value);
        }
      }
      else {
        for (unsigned int j = from; j <= to; j++) {
          std::set<std::string> values;
          ((MLabels*)traj)->GetValues(j, values);
          appendToLabelsPtr(pos, values);
        }
      }
      break;
    }
    default: { // places
      cleanPlacesPtr(pos);
      if (M::BasicType() == MPlace::BasicType()) {
        for (unsigned int j = from; j <= to; j++) {
          std::pair<std::string, unsigned int> value;
          ((MPlace*)traj)->GetValue(j, value);
          appendToPlacesPtr(pos, value);
        }
      }
      else {
        for (unsigned int j = from; j <= to; j++) {
          std::set<std::pair<std::string, unsigned int> > values;
          ((MPlaces*)traj)->GetValues(j, values);
          appendToPlacesPtr(pos, values);
        }
      }
    }
  }
}

/*
\subsection{Function ~setPointerToEmptyValue~}

*/
template<class M>
void Condition::setPointerToEmptyValue(const int pos, M *traj, 
                                 Tuple *t /* = 0 */, ListExpr ttype /* = 0 */) {
  temporalalgebra::SecInterval iv(true);
  switch (getKey(pos)) {
    case 0: { // label
      std::string value("");
      setValuePtr(pos, value);
      break;
    }
    case 1: { // place
      std::pair<std::string, unsigned int> value;
      setValuePtr(pos, value);
      break;
    }
    case 2: {
      clearTimePtr(pos);
      traj->GetInterval(traj->GetNoComponents() - 1, iv);
      iv.SetStart(iv.end, false);
      mergeAddTimePtr(pos, iv);
      break;
    }
    case 3: // start
    case 4: { // end
      traj->GetInterval(traj->GetNoComponents() - 1, iv);
      setStartEndPtr(pos, iv.end);
      break;
    }
    case 5: // leftclosed
    case 6: { // rightclosed
      traj->GetInterval(traj->GetNoComponents() - 1, iv);
      setLeftRightclosedPtr(pos, iv.rc);
      break;
    }
    case 7: {
      setCardPtr(pos, 0);
      break;
    }
    case 8: {
      cleanLabelsPtr(pos);
      break;
    }
    case 9: {
      cleanPlacesPtr(pos);
      break;
    }
    default: {
      restrictPtr(pos, traj, -1, -1, t, ttype, getKey(pos) - 99);
      break;
    }
  }
}

/*
\subsection{Function ~evaluate~}

This function is invoked by ~conditionsMatch~ (and others) and checks whether a 
binding matches a certain condition.

*/
template<class M>
bool Condition::evaluate(const IndexMatchInfo& imi, M *traj, 
                         std::map<std::string, int> &varToElem,
                         Tuple *tuple /* = 0 */, ListExpr ttype /* = 0 */) {
//   cout << "EVAL \'" << text << "\'; " << getVarKeysSize() << endl;
  Word qResult;
  int from, to;
  for (int i = 0; i < getVarKeysSize(); i++) {
    int elem = varToElem[getVar(i)];
    from = imi.getFrom(elem);
    to = imi.getTo(elem);
    if (elem != -1 && from != -1 && to != -1 && from <= to) {
      int key = getKey(i);
      if (key > 99) { // reference to attribute of tuple
        if (!tuple || !ttype) {
          return false;
        }
        if (Tools::isMovingAttr(ttype, key - 99)) {
//           cout << "restrict: " << getVar(i) << " " << from << " " << to 
//                << " " << key-99 << endl;
          restrictPtr(i, traj, from, to, tuple, ttype, key - 99);
        }
        else {
          pointers[i]->CopyFrom(tuple->GetAttribute(key - 100));
        }
      }
      else {
        setPointerToValue<M>(i, traj, from, to);
      }
    }
    else { // variable bound to empty sequence
      setPointerToEmptyValue<M>(i, traj, tuple, ttype);
    }
  }
  getQP()->EvalS(getOpTree(), qResult, OPEN);
//   if (!((CcBool*)qResult.addr)->GetValue()) {
//     cout << "FALSE result: FROM " << from << " TO " << to << "; result for |"
//          << text 
//          << "| is " << (((CcBool*)qResult.addr)->GetValue() ? "TRUE":"FALSE")
//          << endl;
//   }
  return ((CcBool*)qResult.addr)->GetValue();
}
 
/*
\subsection{Function ~matches~}

Checks the pattern and the condition and (if no problem occurs) invokes the NFA
construction and the matching procedure.

*/
template<class M>
ExtBool Pattern::matches(M *m) {
  ExtBool result = ST_UNDEF;
  if (!isValid(M::BasicType())) {
    cout << "pattern is not suitable for type " << M::BasicType() << endl;
    return result;
  }
  Match<M> *match = new Match<M>(this, m);
  if (initEasyCondOpTrees()) {
    result = match->matches();
  }
  delete match;
  return result;
}

/*
\subsection{Function ~getType~}

*/
template<class M>
DataType Match<M>::getMType() {
  if (M::BasicType() == "mlabels") {
    return MLABELS;
  }
  if (M::BasicType() == "mplace") {
    return MPLACE;
  }
  if (M::BasicType() == "mplaces") {
    return MPLACES;
  }
  return MLABEL;
}

/*
\subsection{Function ~match~}

Loops through the MLabel calling updateStates() for every ULabel. True is
returned if and only if the final state is an element of currentStates after
the loop.

*/
template<class M>
ExtBool Match<M>::matches() {
  if (p->isNFAempty()) {
    cout << "empty nfa" << endl;
    return ST_UNDEF;
  }
  if (!p->initEasyCondOpTrees()) {
    cout << "Error: EasyCondOpTrees not initialized" << endl;
    return ST_UNDEF;
  }
  std::set<int> states;
  states.insert(0);
  if (!p->hasConds() && !p->hasAssigns()) {
    for (int i = 0; i < m->GetNoComponents(); i++) {
      if (!updateStates(i, p->nfa, p->elems, p->finalStates, states,
                        p->easyConds, p->easyCondPos, p->atomicToElem)) {
//           cout << "mismatch at unit " << i << endl;
        return ST_FALSE;
      }
    }
    if (!p->containsFinalState(states)) {
//         cout << "no final state is active" << endl;
      return ST_FALSE;
    }
  }
  else {
    createSetMatrix(m->GetNoComponents(), p->elemToVar.size());
    for (int i = 0; i < m->GetNoComponents(); i++) {
      if (!updateStates(i, p->nfa, p->elems, p->finalStates, states,
                        p->easyConds, p->easyCondPos, p->atomicToElem, true)){
//           cout << "mismatch at unit " << i << endl;
        Tools::deleteSetMatrix(matching, m->GetNoComponents());
        return ST_FALSE;
      }
    }
    if (!p->containsFinalState(states)) {
//         cout << "no final state is active" << endl;
      Tools::deleteSetMatrix(matching, m->GetNoComponents());
      matching = 0;
      return ST_FALSE;
    }
    if (!p->initCondOpTrees()) {
      Tools::deleteSetMatrix(matching, m->GetNoComponents());
      matching = 0;
      return ST_UNDEF;
    }
    if (!p->hasAssigns()) {
      bool result = findMatchingBinding(p->nfa, 0, p->elems, p->conds, 
                                        p->atomicToElem, p->elemToVar);
      Tools::deleteSetMatrix(matching, m->GetNoComponents());
      matching = 0;
      return (result ? ST_TRUE : ST_FALSE);
    }
    return ST_TRUE; // happens iff rewrite is called
  }
  return ST_TRUE;
}

/*
\subsection{Function ~states2Str~}

Writes the set of currently active states into a string.

*/
template<class M>
std::string Match<M>::states2Str(int ulId, std::set<int> &states) {
  std::stringstream result;
  if (!states.empty()) {
    std::set<int>::iterator it = states.begin();
    result << "after unit # " << ulId << ", active states are {" << *it;
    it++;
    while (it != states.end()) {
      result << ", " << *it;
      it++;
    }
    result << "}" << endl;
  }
  else {
    result << "after unit # " << ulId << ", there is no active state" << endl;
  }
  return result.str();
}

/*
\subsection{Function ~matchings2Str~}

Writes the matching table into a string.

*/
template<class M>
std::string Match<M>::matchings2Str(unsigned int dim1, unsigned int dim2) {
  std::stringstream result;
  for (unsigned int i = 0; i < dim1; i++) {
    for (unsigned int j = 0; j < dim2; j++) {
      if (matching[i][j].empty()) {
        result << "                    ";
      }
      else {
        std::string cell;
        std::set<unsigned int>::iterator it, it2;
        for (it = matching[i][j].begin(); it != matching[i][j].end(); it++) {
          it2 = it;
          it2++;
          cell += int2Str(*it) + (it2 != matching[i][j].end() ? "," : "");
        }
        result << cell;
        for (unsigned int k = 20; k > cell.size(); k--) {
          result << " ";
        }
      }
    }
    result << endl;
  }
  return result.str();
}

/*
\subsection{Function ~valuesMatch~}

*/
template<class M>
bool Match<M>::valuesMatch(int i, const PatElem& elem) {
  std::set<std::pair<std::string, unsigned int> > mpls;
  std::set<std::string> lbs, mlbs;
  elem.getL(lbs);
  if (lbs.empty() && (elem.getSetRel() < SUPERSET)) {
    return true; // easiest case
  }
  switch (type) {
    case MLABEL: {
      std::string mlb;
      ((MLabel*)m)->GetValue(i, mlb);
      if (elem.getSetRel() == STANDARD) {
        return lbs.find(mlb) != lbs.end();
      }
      mlbs.insert(mlb);
      return Tools::relationHolds(mlbs, lbs, elem.getSetRel());
    }
    case MLABELS: {
      ((MLabels*)m)->GetValues(i, mlbs);
      return Tools::relationHolds(mlbs, lbs, elem.getSetRel());
    }
    case MPLACE: {
      std::pair<std::string, unsigned int> mpl;
      ((MPlace*)m)->GetValue(i, mpl);
      mpls.insert(mpl);
      return Tools::relationHolds(mpls, lbs, elem.getSetRel());
    }  
    case MPLACES: {
      ((MPlaces*)m)->GetValues(i, mpls);
      return Tools::relationHolds(mpls, lbs, elem.getSetRel());
    }
    default: { // cannot occur
      return false;
    }
  }
}

/*
\subsection{Function ~updateStates~}

Applies the NFA. Each valid transaction is processed. If ~store~ is true,
each matching is stored.

*/
template<class M>
bool Match<M>::updateStates(int ulId, std::vector<std::map<int, int> > &nfa,
               std::vector<PatElem> &elems, 
               std::set<int> &finalStates, std::set<int> &states,
               std::vector<Condition> &easyConds, 
               std::map<int, std::set<int> > &easyCondPos,
               std::map<int, int> &atomicToElem, bool store /* = false */) {
  std::set<int>::iterator its;
  std::set<unsigned int>::iterator itu;
  std::map<int, int> transitions;
  for (its = states.begin(); its != states.end(); its++) { // collect possible
    std::map<int, int> trans = nfa[*its];                       // transitions
    transitions.insert(trans.begin(), trans.end());
  }
  if (transitions.empty()) {
    return false;
  }
  states.clear();
  std::map<int, int>::iterator itm, itn;
  temporalalgebra::SecInterval iv;
  m->GetInterval(ulId, iv);
  std::set<std::string> ivs;
  if (store) {
    if (ulId < m->GetNoComponents() - 1) { // usual case
      for (itm = transitions.begin(); itm != transitions.end(); itm++) {
        elems[itm->first].getI(ivs);
        if (valuesMatch(ulId, elems[itm->first]) && Tools::timesMatch(iv, ivs)
         && easyCondsMatch(ulId, elems[itm->first], easyConds,
                           easyCondPos[itm->first])) {
          states.insert(states.end(), itm->second);
          std::map<int, int> nextTrans = nfa[itm->second];
          for (itn = nextTrans.begin(); itn != nextTrans.end(); itn++) {
            itu = matching[ulId][atomicToElem[itm->first]].end();
            matching[ulId][atomicToElem[itm->first]].insert
                               (itu, atomicToElem[itn->first]);// store matching
          }
        }
      }
    }
    else { // last row; mark final states with -1
      for (itm = transitions.begin(); itm != transitions.end(); itm++) {
        elems[itm->first].getI(ivs);
        if (valuesMatch(ulId, elems[itm->first]) && Tools::timesMatch(iv, ivs)
         && easyCondsMatch(ulId, elems[itm->first], easyConds,
                           easyCondPos[itm->first])) {
          states.insert(states.end(), itm->second);
          if (finalStates.count(itm->second)) { // store last matching
            matching[ulId][atomicToElem[itm->first]].insert(UINT_MAX);
          }
        }
      }
    }
  }
  else {
    for (itm = transitions.begin(); itm != transitions.end(); itm++) {
      elems[itm->first].getI(ivs);
      if (valuesMatch(ulId, elems[itm->first]) && Tools::timesMatch(iv, ivs)
       && easyCondsMatch(ulId, elems[itm->first], easyConds,
                         easyCondPos[itm->first])) {
        states.insert(states.end(), itm->second);
      }
    }
  }
  return !states.empty();
}

/*
\subsection{Function ~cleanPaths~}

Deletes all paths inside ~matching~ which do not end at a final state.

*/
template<class M>
void Match<M>::cleanPaths(std::map<int, int> &atomicToElem) {
  std::map<int, int> transitions = p->getTransitions(0);
  std::map<int, int>::reverse_iterator itm;
  for (itm = transitions.rbegin(); itm != transitions.rend(); itm++) {
    cleanPath(0, atomicToElem[itm->first]);
  }
}

/*
\subsection{Function ~findMatchingBinding~}

Searches for a binding which fulfills every condition.

*/
template<class M>
bool Match<M>::findMatchingBinding(std::vector<std::map<int, int> > &nfa, 
                                int startState,
                                std::vector<PatElem> &elems,
                                std::vector<Condition> &conds,
                                std::map<int, int> &atomicToElem,
                                std::map<int, std::string> &elemToVar) {
  if ((startState < 0) || (startState > (int)nfa.size() - 1)) {
    return false; // illegal start state
  }
  if (conds.empty()) {
    return true;
  }
  std::map<int, int> transitions = nfa[startState];
  IndexMatchInfo imi;
  std::map<int, int>::reverse_iterator itm;
  for (itm = transitions.rbegin(); itm != transitions.rend(); itm++) {
    if (findBinding(0, atomicToElem[itm->first], elems, conds, elemToVar, imi)){
      return true;
    }
  }
  return false;
}

/*
\subsection{Function ~findBinding~}

Recursively finds all bindings in the matching set matrix and checks whether
they fulfill every condition, stopping immediately after the first success.

*/
template<class M>
bool Match<M>::findBinding(unsigned int ulId, unsigned int pId,
                   std::vector<PatElem> &elems, std::vector<Condition> &conds,
                   std::map<int, std::string> &elemToVar, IndexMatchInfo &imi) {
  std::string var = elemToVar[pId];
  bool inserted = imi.extendOrInsert(pId, ulId);
  if (*(matching[ulId][pId].begin()) == UINT_MAX) { // complete match
    if (conditionsMatch(conds, imi)) {
      return true;
    }
  }
  else {
    for (std::set<unsigned int>::reverse_iterator 
            it = matching[ulId][pId].rbegin();
         it != matching[ulId][pId].rend(); it++) {
      if (findBinding(ulId + 1, *it, elems, conds, elemToVar, imi)) {
        return true;
      }
    }
  }
  imi.reduceOrErase(pId, inserted);
  return false;
}

/*
\subsection{Function ~cleanPath~}

Recursively deletes all paths starting from (ulId, pId) that do not end at a
final state.

*/
template<class M>
bool Match<M>::cleanPath(unsigned int ulId, unsigned int pId) {
//   cout << "cleanPaths called, ul " << ulId << ", pE " << pId << endl;
  if (matching[ulId][pId].empty()) {
    return false;
  }
  if (*(matching[ulId][pId].begin()) == UINT_MAX) {
    return true;
  }
  bool result = false;
  std::set<unsigned int>::iterator it;
  std::vector<unsigned int> toDelete;
  for (it = matching[ulId][pId].begin(); it != matching[ulId][pId].end(); it++){
    if (cleanPath(ulId + 1, *it)) {
      result = true;
    }
    else {
      toDelete.push_back(*it);
    }
  }
  for (unsigned int i = 0; i < toDelete.size(); i++) {
    matching[ulId][pId].erase(toDelete[i]);
  }
  return result;
}

/*
\subsection{Function ~easyCondsMatch~}

*/
template<class M>
bool Match<M>::easyCondsMatch(int ulId, PatElem const &elem,
                              std::vector<Condition> &easyConds, 
                              std::set<int> pos) {
  if (elem.getW() || pos.empty() || easyConds.empty()) {
    return true;
  }
  std::vector<int> binding;
  IndexMatchInfo imi(binding);
  std::string var;
  elem.getV(var);
  int elemPos = p->getElemFromVar(var);
  imi.set(elemPos - 1, ulId - 1);
  imi.set(elemPos, ulId);
  std::map<std::string, int> varToElem = p->getVarToElem();
  for (std::set<int>::iterator it = pos.begin(); it != pos.end(); it++) {
    if (!easyConds[*it].evaluate<M>(imi, m, varToElem)) {
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~conditionsMatch~}

Checks whether the specified conditions are fulfilled. The result is true if
and only if there is (at least) one binding that matches every condition.

*/
template<class M>
bool Match<M>::conditionsMatch(std::vector<Condition> &conds,
                const IndexMatchInfo &imi) {
  if (!m->GetNoComponents()) { // empty MLabel
    return evaluateEmptyM();
  }
  std::map<std::string, int> varToElem = p->getVarToElem();
  for (unsigned int i = 0; i < conds.size(); i++) {
    if (!conds[i].evaluate<M>(imi, m, varToElem)) {
//       cout << conds[i].getText() << " | ";
//       Tools::printBinding(binding);
      return false;
    }
  }
//   cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!! MATCH" << endl;
  return true;
}

/*
\subsection{Function ~evaluateEmptyML~}

This function is invoked in case of an empty moving label (i.e., with 0
components). A match is possible for a pattern like 'X [*] Y [*]' and conditions
X.card = 0, X.card = Y.card [*] 7. Time or label constraints are invalid.

*/
template<class M>
bool Match<M>::evaluateEmptyM() {
  Word res;
  for (unsigned int i = 0; i < p->conds.size(); i++) {
    for (int j = 0; j < p->conds[i].getVarKeysSize(); j++) {
      if (p->conds[i].getKey(j) != 4) { // only card conditions possible
        cout << "Error: Only cardinality conditions allowed" << endl;
        return false;
      }
      p->conds[i].setCardPtr(j, 0);
    }
    p->conds[i].getQP()->EvalS(p->conds[i].getOpTree(), res, OPEN);
    if (!((CcBool*)res.addr)->IsDefined() || !((CcBool*)res.addr)->GetValue()) {
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~isSensiblyBound~}

This function checks whether a variable is bound in a sensible and valid way.

*/
template<class M>
bool Match<M>::isSensiblyBound(const std::map<std::string, 
                               std::pair<int, int> > &b, 
                               std::string& var) {
  int first = b.find(var)->second.first;
  int second = b.find(var)->second.second;
  return (first >= 0 && first < m->GetNoComponents() &&
          second >= 0 && second < m->GetNoComponents() && first <= second);
}

/*
\subsection{Constructors for class ~FilterMatchesLI~}

*/
template<class M>
FilterMatchesLI<M>::FilterMatchesLI(Word _stream, int _attrIndex, 
                 std::string& pText)
               : stream(_stream), attrIndex(_attrIndex), match(0), 
                 streamOpen(false), deleteP(true) {
  Pattern *p = Pattern::getPattern(pText, false);
  if (p) {
    if (p->isValid(M::BasicType())) {
      match = new Match<M>(p, 0);
      stream.open();
      streamOpen = true;
    }
    else {
      cout << "pattern is not suitable for type " << M::BasicType() << endl;
    }
  }
}

/*
\subsection{Destructor for class ~FilterMatchesLI~}

*/
template<class M>
FilterMatchesLI<M>::~FilterMatchesLI() {
  if (match) {
    if (deleteP) {
      match->deletePattern();
    }
    delete match;
    match = 0;
  }
  if (streamOpen) {
    stream.close();
  }
}

/*
\subsection{Function ~getNextResult~}

*/
template<class M>
Tuple* FilterMatchesLI<M>::getNextResult() {
  if (!match) {
    return 0;
  }
  Tuple* cand = stream.request();
  while (cand) {
    match->setM((M*)cand->GetAttribute(attrIndex));
    if (match->matches() == ST_TRUE) {
      return cand;
    }
    cand->DeleteIfAllowed();
    cand = stream.request();
  }
  return 0;
}

template<class M>
RewriteLI<M>::RewriteLI(M *src, Pattern *pat) {
  if (pat->isValid(M::BasicType())) {
    match = new Match<M>(pat, src);
    if (match->matches()) {
      if (match->initCondOpTrees() && match->initAssignOpTrees()) {
        if (!src->GetNoComponents()) {
          BindingStackElem dummy(0, 0);
          bindingStack.push(dummy);
        }
        else {
          std::map<int, int> transitions = pat->getTransitions(0);
          for (std::map<int, int>::iterator itm = transitions.begin();
                                      itm != transitions.end(); itm++) {
            BindingStackElem bE(0, itm->first); // init stack
            bindingStack.push(bE);
          }
        }
      }
    }
  }
  else {
    cout << "pattern is not suitable for type " << MLabel::BasicType() << endl;
    match = 0;
  }
}

template<class M>
void RewriteLI<M>::clearMatch(const bool removePattern) {
  if (match) {
    if (removePattern) {
      match->deletePattern();
    }
    match->deleteSetMatrix();
    delete match;
    match = 0;
  }
}

/*
\subsection{Function ~rewrite~}

*/
template<class M>
M* RewriteLI<M>::rewrite(M *src, IndexMatchInfo &imi,
                         std::vector<Assign> &assigns) {
  Pattern *p = match->getPattern();
  M *result = new M(true);
  Word qResult;
  Instant start(datetime::instanttype), end(datetime::instanttype);
  temporalalgebra::SecInterval iv(true), iv2(true);
  bool lc(false), rc(false);
  int elem = -1;
  std::pair<unsigned int, unsigned int> segment;
  assert(src->IsValid());
  for (unsigned int i = 0; i < assigns.size(); i++) {
    for (int j = 0; j <= 9; j++) {
      if (!assigns[i].getText(j).empty()) {
        for (int k = 0; k < assigns[i].getRightSize(j); k++) {
          elem = p->getElemFromVar(assigns[i].getRightVar(j, k));
          if (imi.getTo(elem) > -1) {
            segment.first = imi.getFrom(elem);
            segment.second = imi.getTo(elem);
            switch (assigns[i].getRightKey(j, k)) {
              case 0: { // label
                std::string lvalue;
                ((MLabel*)src)->GetValue(segment.first, lvalue);
                assigns[i].setLabelPtr(k, lvalue);
                break;
              }
              case 1: { // place
                std::pair<std::string, unsigned int> pvalue;
                ((MPlace*)src)->GetValue(segment.first, pvalue);
                assigns[i].setPlacePtr(k, pvalue);
                break;
              }
              case 2: { // time
                src->GetInterval(segment.first, iv);
                src->GetInterval(segment.second, iv2);
                iv.end = iv2.end;
                iv.rc = iv2.rc;
                assigns[i].setTimePtr(k, iv);
                break;
              }
              case 3: { // start
                src->GetInterval(segment.first, iv);
                if (j == 3) {
                  assigns[i].setStartPtr(k, iv.start);
                }
                else {
                  assigns[i].setEndPtr(k, iv.start);
                }
                break;
              }
              case 4: { // end
                src->GetInterval(segment.second, iv);
                if (j == 3) {
                  assigns[i].setStartPtr(k, iv.end);
                }
                else {
                  assigns[i].setEndPtr(k, iv.end);
                }
                break;
              }
              case 5: { // leftclosed
                src->GetInterval(segment.first, iv);
                if (j == 5) {
                  assigns[i].setLeftclosedPtr(k, iv.lc);
                }
                else {
                  assigns[i].setRightclosedPtr(k, iv.lc);
                }
                break;
              }
              case 6: { // rightclosed
                src->GetInterval(segment.second, iv);
                if (j == 5) {
                  assigns[i].setLeftclosedPtr(k, iv.rc);
                }
                else {
                  assigns[i].setRightclosedPtr(k, iv.rc);
                }
                break;
              }
              case 8: { // labels
                std::set<Labels::base> lvalues;
                for (unsigned int m = segment.first; m <= segment.second; m++) {
                  ((MLabels*)src)->GetValues(m, lvalues);
                  assigns[i].appendToLabelsPtr(k, lvalues);
                }
                break;
              }
              case 9: { // places
                std::set<Places::base> pvalues;
                for (unsigned int m = segment.first; m <= segment.second; m++) {
                  ((MPlaces*)src)->GetValues(m, pvalues);
                  assigns[i].appendToPlacesPtr(k, pvalues);
                }
                break;
              }
              default: { // cannot occur
                cout << "Error: assigns[" << i << "].getRightKey(" << j << ", "
                     << k << ") = " << assigns[i].getRightKey(j, k) << endl;
                result->SetDefined(false);
                return result;
              }
            }
          }
          else { // variable from right size unbound
            result->SetDefined(false);
            return result;
          }
        }
      }
    } // all pointers are set now
    Label lb(true);
    if (!assigns[i].getText(0).empty()) {
      assigns[i].getQP(0)->EvalS(assigns[i].getOpTree(0), qResult, OPEN);
      lb = *((Label*)qResult.addr);
    }
    Place pl(true);
    if (!assigns[i].getText(1).empty()) {
      assigns[i].getQP(1)->EvalS(assigns[i].getOpTree(1), qResult, OPEN);
      pl = *((Place*)qResult.addr);
    }
    if (!assigns[i].getText(2).empty()) {
      assigns[i].getQP(2)->EvalS(assigns[i].getOpTree(2), qResult, OPEN);
      iv2 = *((temporalalgebra::SecInterval*)qResult.addr);
    }
    if (!assigns[i].getText(3).empty()) {
      assigns[i].getQP(3)->EvalS(assigns[i].getOpTree(3), qResult, OPEN);
      start = *((Instant*)qResult.addr);
    }
    if (!assigns[i].getText(3).empty()) {
      assigns[i].getQP(4)->EvalS(assigns[i].getOpTree(4), qResult, OPEN);
      end = *((Instant*)qResult.addr);
    }
    if (!assigns[i].getText(5).empty()) {
      assigns[i].getQP(5)->EvalS(assigns[i].getOpTree(5), qResult, OPEN);
      lc = ((CcBool*)qResult.addr)->GetValue();
    }
    if (!assigns[i].getText(6).empty()) {
      assigns[i].getQP(6)->EvalS(assigns[i].getOpTree(6), qResult, OPEN);
      rc = ((CcBool*)qResult.addr)->GetValue();
    }
    Labels lbs(true);
    if (!assigns[i].getText(8).empty()) {
      assigns[i].getQP(8)->EvalS(assigns[i].getOpTree(8), qResult, OPEN);
      lbs = *((Labels*)qResult.addr);
    }
    Places pls(true);
    if (!assigns[i].getText(9).empty()) {
      assigns[i].getQP(9)->EvalS(assigns[i].getOpTree(9), qResult, OPEN);
      pls = *((Places*)qResult.addr);
    }
     // information from assignment i collected
    elem = p->getElemFromVar(assigns[i].getV());
    if (imi.getTo(elem) > -1) { // var occurs
      segment.first = imi.getFrom(elem);
      segment.second = imi.getTo(elem);
      if (segment.second == segment.first) { // 1 source ul
        src->GetInterval(segment.first, iv);
        if (!assigns[i].getText(2).empty()) {
          iv = iv2;
        }
        if (!assigns[i].getText(3).empty()) {
          iv.start = start;
        }
        if (!assigns[i].getText(4).empty()) {
          iv.end = end;
        }
        if (!assigns[i].getText(5).empty()) {
          iv.lc = lc;
        }
        if (!assigns[i].getText(6).empty()) {
          iv.rc = rc;
        }
        if (!iv.IsValid()) {
          iv.Print(cout);
          cout << " is an invalid interval" << endl;
          result->SetDefined(false);
          return result;
        }
        if (M::BasicType() == MLabel::BasicType()) {
          if (!assigns[i].getQP(0)) {
            std::string lvalue;
            ((MLabel*)src)->GetValue(segment.first, lvalue);
            ((MLabel*)result)->Add(iv, lvalue);
          }
          else {
            ((MLabel*)result)->Add(iv, lb);
          }
        }
        else if (M::BasicType() == MPlace::BasicType()) {
          if (!assigns[i].getQP(1)) {
            std::pair<std::string, unsigned int> pvalue;
            ((MPlace*)src)->GetValue(segment.first, pvalue);
            ((MPlace*)result)->Add(iv, pvalue);
          }
          else {
            ((MPlace*)result)->Add(iv, pl);
          }
        }
        else if (M::BasicType() == MLabels::BasicType()) {
          if (!assigns[i].getQP(8)) {
            ((MLabels*)src)->GetBasics(segment.first, lbs);
          }
          ((MLabels*)result)->Add(iv, lbs);
        }
        else if (M::BasicType() == MPlaces::BasicType()) {
          if (!assigns[i].getQP(9)) {
            ((MPlaces*)src)->GetBasics(segment.first, pls);
          }
          ((MPlaces*)result)->Add(iv, pls);
        }
      }
      else { // arbitrary many source uls
        for (unsigned int m = segment.first; m <= segment.second; m++) {
          src->GetInterval(m, iv);
          if ((m == segment.first) && // first unit label
            (!assigns[i].getText(2).empty() || !assigns[i].getText(3).empty())){
            iv.start = start;
          }
          if ((m == segment.second) && // last unit label
            (!assigns[i].getText(2).empty() || !assigns[i].getText(4).empty())){
            iv.end = end;
          }
          if ((m == segment.first) && !assigns[i].getText(5).empty()) {
            iv.lc = lc;
          }
          if ((m == segment.second) && !assigns[i].getText(6).empty()) {
            iv.rc = rc;
          }
          if (!iv.IsValid()) {
            iv.Print(cout);
            cout << " is an invalid interval" << endl;
            result->SetDefined(false);
            return result;
          }
          if (M::BasicType() == "mlabel") {
            Label lb(true);
            ((MLabel*)src)->GetBasic(m, lb);
            ((MLabel*)result)->Add(iv, lb);
          }
          else if (M::BasicType() == "mplace") {
            Place pl(true);
            ((MPlace*)src)->GetBasic(m, pl);
            ((MPlace*)result)->Add(iv, pl);
          }
          if (!assigns[i].getText(8).empty()) {
            ((MLabels*)result)->Add(iv, lbs);
          }
          if (!assigns[i].getText(9).empty()) {
            ((MPlaces*)result)->Add(iv, pls);
          }
        }
      }
    }
    else { // variable does not occur in binding
      if (!assigns[i].occurs()) { // and not in pattern
        if (!assigns[i].getText(2).empty()) {
          iv = iv2;
        }
        else {
          iv.start = start;
          iv.end = end;
        }
        if (!assigns[i].getText(5).empty()) {
          iv.lc = lc;
        }
        if (!assigns[i].getText(6).empty()) {
          iv.rc = rc;
        }
        if (!assigns[i].getText(0).empty()) {
          ((MLabel*)result)->Add(iv, lb);
        }
        if (!assigns[i].getText(1).empty()) {
          ((MPlace*)result)->Add(iv, pl);
        }
        if (!assigns[i].getText(8).empty()) {
          ((MLabels*)result)->Add(iv, lbs);
        }
        if (!assigns[i].getText(9).empty()) {
          ((MPlaces*)result)->Add(iv, pls);
        }
      }
    }
  }
//   assert(result->IsValid());
  if (!result->IsValid()) {
    result->SetDefined(false);
  }
  return result;
}

template<class M>
M* RewriteLI<M>::getNextResult() {
  if (!match) {
    return 0;
  }
  if (bindingStack.empty()) {
    return 0;
  }
  M* result = 0;
  if (!match->m->GetNoComponents()) { // empty mlabel
    bindingStack.pop();
    std::vector<Condition> *conds = match->p->getConds();
    if (match->conditionsMatch(*conds, imi)) {
      M *source = match->m;
      result = rewrite(source, imi, match->p->getAssigns());
    }
  }
  else { // non-empty mlabel
    BindingStackElem bE(0, 0);
    while (!bindingStack.empty()) {
      bE = bindingStack.top();
      bindingStack.pop();
      resetBinding(bE.ulId);
      if (findNextBinding(bE.ulId, bE.peId, match->p, 0)) {
        if (rewBindings.find(imi) == rewBindings.end()) {
//           cout << "FOUND and INSERTED: ";
//           imi.print(true);
          rewBindings.insert(imi);
          M *source = match->m;
          result = rewrite(source, imi, match->p->getAssigns());
        }
      }
    }
  }
  match->deleteSetMatrix();
  return result;
}

template<class M>
void RewriteLI<M>::resetBinding(int limit) {
  if (limit == 0) {
    imi.binding.clear();
    return;
  }
  bool found = false;
  unsigned int pos = 0;
  while (pos < imi.binding.size() && !found) {
    if (imi.getTo(pos) >= limit) {
      imi.set(pos, limit - 1);
      found = true;
      for (unsigned int j = pos + 1; j < imi.binding.size(); pos++) {
        imi.binding.pop_back();
      }
    }
    pos++;
  }
}

template<class M>
bool RewriteLI<M>::findNextBinding(unsigned int ulId, unsigned int peId,
                                   Pattern *p, int offset) {
//   std::string var = p->getVarFromElem(peId - offset);
  imi.set(peId - offset, ulId);
  if (*(match->matching[ulId][peId].begin()) == UINT_MAX) { // complete match
    std::vector<Condition> *conds = p->getConds();
    match->p = p;
    PatElem atom;
    p->getElem(p->get1stAtomFromElem(peId), atom);
    return match->easyCondsMatch(ulId, atom, p->easyConds, 
                              p->getEasyCondPos(p->get1stAtomFromElem(peId))) &&
           match->conditionsMatch(*conds, imi);
  }
  if (match->matching[ulId][peId].empty()) {
    return false;
  }
  else { // push all elements except the first one to stack; process first elem
    std::set<unsigned int>::reverse_iterator it, it2;
    it2 = match->matching[ulId][peId].rbegin();
    it2++;
    for (it = match->matching[ulId][peId].rbegin();
         it2 != match->matching[ulId][peId].rend(); it++) {
      it2++;
      BindingStackElem bE(ulId + 1, *it);
      bindingStack.push(bE);
    }
    return findNextBinding(ulId + 1, *(match->matching[ulId][peId].begin()), p,
                           offset);
  }
}

/*
\subsection{Constructor for class ~ClassifyLI~}

This constructor is used for the operator ~classify~.

*/
template<class M>
ClassifyLI::ClassifyLI(M *traj, Word _classifier) : classifyTT(0) {
  Classifier *c = static_cast<Classifier*>(_classifier.addr);
  int startState(0), pat(0);
  std::set<unsigned int> emptyset;
  std::set<int> states, finalStates, matchCands;
  std::set<int>::iterator it;
  Pattern *p = 0;
  PatElem elem;
  std::vector<PatElem> patElems;
  std::vector<Condition> easyConds;
  std::vector<int> startStates;
  std::map<int, std::set<int> > easyCondPos;
  std::map<int, int> atomicToElem; // TODO: use this sensibly
  std::map<int, std::string> elemToVar; // TODO: use this sensibly
  bool condsOccur = false;
  for (int i = 0; i < c->getCharPosSize() / 2; i++) {
    states.insert(states.end(), startState);
    startStates.push_back(startState);
    p = Pattern::getPattern(c->getPatText(i), true); // single NFA are not built
    if (p) {
      p->setDescr(c->getDesc(i));
      if (p->hasConds()) {
        condsOccur = true;
      }
      pats.push_back(p);
      std::map<int, std::set<int> > easyOld = p->getEasyCondPos();
      for (std::map<int, std::set<int> >::iterator im = easyOld.begin();
                                         im != easyOld.end(); im++) {
        for (it = im->second.begin(); it != im->second.end(); it++) {
          easyCondPos[im->first+patElems.size()].insert(*it + easyConds.size());
        }
      }
      for (unsigned int j = 0; j < p->getEasyConds().size(); j++) {
        easyConds.push_back(p->getEasyCond(j));
        easyConds.back().initOpTree();
      }
      for (int j = 0; j < p->getSize(); j++) {
        p->getElem(j, elem);
        patElems.push_back(elem);
      }
      do { // get start state
        startState++;
        c->s2p.Get(startState, pat);
      } while ((i < c->getCharPosSize() / 2 - 1) && (pat >= 0));
    }
    else {
      cout << "pattern could not be parsed" << endl;
    }
  }
  if (!pats.size()) {
    cout << "no classification data specified" << endl;
    return;
  }
  std::vector<std::map<int, int> > nfa;
  Tools::createNFAfromPersistent(c->delta, c->s2p, nfa, finalStates);
  Match<M> *match = new Match<M>(0, traj);
  if (condsOccur) {
    match->createSetMatrix(traj->GetNoComponents(), patElems.size());
  }
  for (int i = 0; i < traj->GetNoComponents(); i++) {
    if (!match->updateStates(i, nfa, patElems, finalStates, states, easyConds,
                             easyCondPos, atomicToElem, condsOccur)){
      for (unsigned int j = 0; j < easyConds.size(); j++) {
        easyConds[j].deleteOpTree();
      }
      return;
    }
  }
  for (unsigned int j = 0; j < easyConds.size(); j++) {
    easyConds[j].deleteOpTree();
  }
  for (it = states.begin(); it != states.end(); it++) { // active states final?
    c->s2p.Get(*it, pat);
    if ((*it > 0) && (pat != INT_MAX) && (pat >= 0)) {
      matchCands.insert(matchCands.end(), pat);
//       cout << "pattern " << pat << " matches" << endl;
    }
  }
  for (it = matchCands.begin(); it != matchCands.end(); it++) { // check conds
    pats[*it]->initCondOpTrees();
    std::vector<Condition>* conds = pats[*it]->getConds();
//     cout << "call fMB(nfa, " << startStates[*it] << ", " << patElems.size()
//          << ", " << conds->size() << ")" << endl;
    if (match->findMatchingBinding(nfa, startStates[*it], patElems, *conds,
                                   atomicToElem, elemToVar)) {
      matchingPats.insert(*it);
//       cout << "p " << *it << " matches after condition check" << endl;
    }
    else {
//       cout << "p " << *it << " has non-matching conditions" << endl;
    }
    pats[*it]->deleteCondOpTrees();
  }
  match->deleteSetMatrix();
  delete match;
}

/*
\subsection{Constructor for class ~MultiRewriteLI~}

This constructor is used for the operator ~rewrite~.

*/
template<class M>
MultiRewriteLI<M>::MultiRewriteLI(Word _tstream, Word _pstream, int _pos) : 
  ClassifyLI(0), RewriteLI<M>(0), tStream(_tstream), attrpos(_pos),
  streamOpen(false), traj(0), c(0) {
  Stream<FText> pStream(_pstream);
  pStream.open();
  FText* inputText = pStream.request();
  Pattern *p = 0;
  PatElem elem;
  std::set<int>::iterator it;
  std::map<int, std::set<int> >::iterator im;
  int elemCount(0);
  std::string var;
  while (inputText) {
    if (!inputText->IsDefined()) {
      cout << "undefined input is ignored" << endl;
    }
    else {
      p = Pattern::getPattern(inputText->GetValue(), true); // no single NFA
      if (p) {
        if (!p->hasAssigns()) {
          cout << "pattern without rewrite part ignored" << endl;
        }
        else {
          if (p->initCondOpTrees()) {
            p->initAssignOpTrees();
            pats.push_back(p);
            for (int i = 0; i < p->getSize(); i++) {
              atomicToElem[patElems.size()] = elemCount + p->getElemFromAtom(i);
              p->getElem(i, elem);
              elem.getV(var);
              elemToVar[elemCount+p->getElemFromAtom(i)] = var;
              varToElem[var] = elemCount+p->getElemFromAtom(i);
              patElems.push_back(elem);
              patOffset[elemCount + p->getElemFromAtom(i)] =
                                 std::make_pair(pats.size() - 1, elemCount);
            }
            elemCount += p->getElemFromAtom(p->getSize() - 1) + 1;
            std::map<int, std::set<int> > easyOld = p->getEasyCondPos();
            for (im = easyOld.begin(); im != easyOld.end(); im++) {
              for (it = im->second.begin(); it != im->second.end(); it++) {
                easyCondPos[im->first + patElems.size()].insert
                                                       (*it + easyConds.size());
              }
            }
            for (unsigned int j = 0; j < p->getEasyConds().size(); j++) {
              easyConds.push_back(p->getEasyCond(j));
              easyConds.back().initOpTree();
            }
          }
        }
      }
    }
    inputText->DeleteIfAllowed();
    inputText = pStream.request();
  }
  pStream.close();
  if (!pats.size()) {
    cout << "no classification data specified" << endl;
  }
  else {
    tStream.open();
    streamOpen = true;
    Classifier *c = new Classifier(0);
    c->buildMultiNFA(pats, nfa, finalStates, state2Pat);
    c->getStartStates(states);
    this->match = new Match<M>(0, traj);
    c->DeleteIfAllowed();
  }
}


/*
\subsection{Function ~nextResult~}

This function is used for the operator ~multirewrite~.

*/
template<class M>
M* MultiRewriteLI<M>::nextResult() {
  if (!pats.size()) {
    return 0;
  }
  std::set<int> startStates;
  std::set<int>::iterator it;
  while (!this->bindingStack.empty()) {
    BindingStackElem bE(0, 0);
    bE = this->bindingStack.top();
//     cout << "take (" << bE.ulId << ", " << bE.pId << ") from stack" << endl;
    this->bindingStack.pop();
    this->resetBinding(bE.ulId);
    std::pair<int, int> patNo = patOffset[bE.peId];
    if (this->findNextBinding(bE.ulId, bE.peId, pats[patNo.first], 
        patNo.second)) {
      return RewriteLI<M>::rewrite(traj, this->imi, 
                                   pats[patNo.first]->getAssigns());
    }
  }
  Tuple *tuple = 0;
  while (this->bindingStack.empty()) { // new ML from stream necessary
    this->match->deleteSetMatrix();
    delete this->match;
    this->match = 0;
    deleteIfAllowed(traj);
    tuple = tStream.request();
    if (!tuple) {
      return 0;
    }
    traj = (M*)(tuple->GetAttribute(attrpos))->Copy();
    this->match = new Match<M>(0, traj);
    this->match->createSetMatrix(traj->GetNoComponents(), patElems.size());
    getStartStates(startStates);
    states = startStates;
    matchCands.clear();
    int i = 0;
    while (!states.empty() && (i < traj->GetNoComponents())) { // loop over traj
      this->match->updateStates(i, nfa, patElems, finalStates, states,
                                easyConds, easyCondPos, atomicToElem, true);
      i++;
    }
    for (it = states.begin(); it != states.end(); it++) { //active states final?
      if (finalStates.count(*it)) {
        matchCands.insert(matchCands.end(), state2Pat[*it]);
      }
    }
    initStack(startStates);
    tuple->DeleteIfAllowed();
    tuple = 0;
    while (!this->bindingStack.empty()) {
      BindingStackElem bE(0, 0);
      bE = this->bindingStack.top();
//      cout << "take (" << bE.ulId << ", " << bE.pId << ") from stack" << endl;
      this->bindingStack.pop();
      this->resetBinding(bE.ulId);
      std::pair<int, int> patNo = patOffset[bE.peId];
      pats[patNo.first]->setVarToElem(varToElem);
      if (this->findNextBinding(bE.ulId, bE.peId, pats[patNo.first], 
                                patNo.second)) {
        return RewriteLI<M>::rewrite(traj, this->imi, 
                                     pats[patNo.first]->getAssigns());
      }
    }
  }
  cout << "SHOULD NOT OCCUR" << endl;
  return 0;
}

/*
\subsection{Function ~initStack~}

Determines the start states of the match candidate patterns and pushes the
corresponding initial transitions onto the stack.

*/
template<class M>
void MultiRewriteLI<M>::initStack(std::set<int> &startStates) {
  std::set<int>::iterator it;
  std::map<int, int>::iterator itm;
  for (it = startStates.begin(); it != startStates.end(); it++) {
    if (matchCands.count(-state2Pat[*it])) {
      std::map<int, int> transitions = nfa[*it];
      for (itm = transitions.begin(); itm != transitions.end(); itm++) {
        BindingStackElem bE(0, atomicToElem[itm->first]);
        this->bindingStack.push(bE);
//         cout << "(0, " << itm->first << ") pushed onto stack" << endl;
      }
    }
  }
}

/*
\subsection{Function ~getStartStates~}

*/
template<class M>
void MultiRewriteLI<M>::getStartStates(std::set<int> &states) {
  states.clear();
  states.insert(0);
  std::map<int, int>::iterator it;
  for (it = state2Pat.begin(); it != state2Pat.end(); it++) {
    if (it->second < 0) {
      states.insert(it->first);
    }
  }
}

/*
\subsection{Destructor for class ~MultiRewriteLI~}

*/
template<class M>
MultiRewriteLI<M>::~MultiRewriteLI() {
  for (unsigned int i = 0; i < pats.size(); i++) {
    if (pats[i]) {
      pats[i]->deleteCondOpTrees();
      delete pats[i];
      pats[i] = 0;
    }
  }
  if (this->match) {
    this->match->deleteSetMatrix();
    delete this->match;
  }
  this->match = 0;
  if (traj) {
    deleteIfAllowed(traj);
  }
  traj = 0;
  if (streamOpen) {
    tStream.close();
  }
  for (unsigned int i = 0; i < easyConds.size(); i++) {
    easyConds[i].deleteOpTree();
  }
}

/*
\subsection{Function ~nextResultTuple~}

*/
template<class M>
Tuple* IndexClassifyLI::nextResultTuple() {
  if (c == 0) {
    return 0;
  }
  while (results.empty()) {
    if (currentPat >= c->getNumOfP()) {
      return 0;
    }
    cout << "results empty, fill now. currentPat=" << currentPat << ", " 
         << c->getPatText(currentPat) << endl;
    Pattern *pat = Pattern::getPattern(c->getPatText(currentPat), false);
    p = pat;
    delete pat;
    initialize();
    applyNFA();
    for (unsigned int i = 0; i < matches.size(); i++) {
      results.push(std::make_pair(c->getDesc(currentPat), matches[i]));
    }
    p->deleteCondOpTrees();
    cout << "desc for " << currentPat << " is " << c->getDesc(currentPat)<<endl;
    currentPat++;
  }
  std::pair<std::string, TupleId> resultPair = results.front();
  results.pop();
  cout << "tuple id " << resultPair.second << " popped, " << results.size() 
       << " left" << endl;
   
  Tuple* tuple = rel->GetTuple(resultPair.second, false);
  int noValues = ((MLabels*)(tuple->GetAttribute(attrNo)))->GetNoValues();
  cout << "Trajectory has "
       << ((MLabels*)(tuple->GetAttribute(attrNo)))->GetNoComponents()
       << " units and " << noValues << " labels" << endl;

  ((MLabels*)(tuple->GetAttribute(attrNo)))->Print(cout);
  Attribute* traj = (tuple->GetAttribute(attrNo))->Copy();
  Tuple *result = new Tuple(classifyTT);
  result->PutAttribute(0, new FText(true, resultPair.first));
  result->PutAttribute(1, traj);
  deleteIfAllowed(tuple);
  return result;
}

/*
\subsection{Function ~imiMatch~}

*/
template<class M>
bool IndexMatchesLI::imiMatch(Match<M>& match, const int e, const TupleId id,
                     IndexMatchInfo& imi, const int unit, const int newState) {
  PatElem elem;
  p->getElem(e, elem);
  if (unit >= 0) { // exact position from index
//     cout << "   unit=" << unit << ", imi: next=" << imi.next << ", range=" 
//          << (imi.range ? "TRUE" : "FALSE") << endl;
    if (imi.matches(unit) && match.valuesMatch(unit, elem) &&
        timesMatch(id, unit, elem) &&
        match.easyCondsMatch(unit, elem, p->easyConds, p->getEasyCondPos(e))) {
      if (unit + 1 >= unitCtr) {
        IndexMatchInfo newIMI(false, unit + 1, imi.binding, imi.prevElem);
        extendBinding(newIMI, e, false);
//         if (newIMI.finished(trajSize[id])) {
//           newIMI.print(true); cout << "   ";
//         }
        if (p->isFinalState(newState) && newIMI.finished(trajSize[id]) && 
            checkConditions(id, newIMI)) { // complete match
          removeIdFromIndexResult(id);
          removeIdFromMatchInfo(id);
  //         cout << id << " removed (index match) " << activeTuples 
  //              << " active tuples" << endl;
          matches.push_back(id);
          return true;
        }
        else if (!newIMI.exhausted(trajSize[id])) { // continue
          newMatchInfo[newState][id]->imis.push_back(newIMI);
  //         cout << "   IMI pushed back for new state " << newState
  //              << " and id " << id << endl;
          return true;
        }
      }
    }
    return false;
  }
  if (imi.range) {
    bool result = false;
    int numOfNewIMIs = 0;
    for (int i = imi.next; i < trajSize[id]; i++) {
      if (match.valuesMatch(i, elem) && timesMatch(id, i, elem) &&
          match.easyCondsMatch(i, elem, p->easyConds, p->getEasyCondPos(e))) {
        if (i + 1 >= unitCtr) {
          IndexMatchInfo newIMI(false, i + 1, imi.binding, imi.prevElem);
          extendBinding(newIMI, e, false);
          if (p->isFinalState(newState) && imi.finished(trajSize[id]) &&
              checkConditions(id, newIMI)) { // complete match
            removeIdFromMatchInfo(id);
  //           cout << id << " removed (range match) " << activeTuples 
  //                << " active tuples" << endl;
            removeIdFromIndexResult(id);
            matches.push_back(id);
            return true;
          }
          else if (!newIMI.exhausted(trajSize[id])) { // continue
            if (newMatchInfo[newState] == 0) {
              newMatchInfo[newState] = new IndexMatchSlot*[rel->GetNoTuples()];
              memset(newMatchInfo[newState], 0, 
                     rel->GetNoTuples() * sizeof(void*));
            }
            newMatchInfo[newState][id]->imis.push_back(newIMI);
            numOfNewIMIs++;
            result = true;
          }
        }
      }
    }
    return result;
  }
  else {
    if (match.valuesMatch(imi.next, elem) && timesMatch(id, imi.next, elem) &&
     match.easyCondsMatch(imi.next, elem, p->easyConds, p->getEasyCondPos(e))) {
      if (imi.next + 1 >= unitCtr) {
        IndexMatchInfo newIMI(false, imi.next + 1, imi.binding, imi.prevElem);
        extendBinding(newIMI, e, false);
        if (p->isFinalState(newState) && imi.finished(trajSize[id]) && 
            checkConditions(id, newIMI)) { // complete match
          removeIdFromMatchInfo(id);
  //         cout << id << " removed (match) " << activeTuples
  //              << " active tuples" << endl;
          matches.push_back(id);
          return true;
        }
        else if (!newIMI.exhausted(trajSize[id])) { // continue
          if (newMatchInfo[newState] == 0) {
            newMatchInfo[newState] = new IndexMatchSlot*[rel->GetNoTuples()];
            memset(newMatchInfo[newState], 0, 
                   rel->GetNoTuples() * sizeof(void*));
          }
          newMatchInfo[newState][id]->imis.push_back(newIMI);
          return true;
        }
      }
    }
  }
  return false;
}

/*
\subsection{Function ~periodsToUnits~}

*/
template<class M>
void IndexMatchSuper::periodsToUnits(const temporalalgebra::Periods *per,
                                     const TupleId tId,
                                     std::set<int> &units) {
  int start, end;
  temporalalgebra::Interval<Instant> iv;
  Tuple *t = rel->GetTuple(tId, false);
  M *traj = (M*)t->GetAttribute(attrNo);
  for (int i = 0; i < per->GetNoComponents(); i++) {
    per->Get(i, iv);
    start = traj->Position(iv.start);
    end = traj->Position(iv.end);
    if (end > start && !iv.rc) {
      temporalalgebra::SecInterval trajIv;
      traj->GetInterval(end, trajIv);
      if (!trajIv.rc) {
        end--;
      }
    }
    if (start == -1) {
      start = traj->FirstPosFrom(iv.start);
    }
    if (end == -1) {
      end = traj->LastPosUntil(iv.end);
    }
    if (end == start - 1) {
      end++;
    }
    if (start > -1 && end > -1) {
      if (start == -1 && end >= 0) {
        start = 0;
      }
      else if (end == -1 && start >= 0) {
        end = getTrajSize(tId, mtype) - 1;
      }
      for (int j = start; j <= end; j++) {
        units.insert(units.end(), j);
      }
    }
  }
  t->DeleteIfAllowed();
}

/*
\subsection{Function ~unitsToPeriods~}

*/
template<class M, class U>
void TMatchIndexLI::unitsToPeriods(Attribute *traj, const std::set<int> &units,
                                   temporalalgebra::Periods *per) {
  M *m = (M*)traj;
  U u(true);
  for (std::set<int>::iterator it = units.begin(); it != units.end(); it++) {
    m->Get(*it, u);
    per->MergeAdd(u.timeInterval);
  }
}

/*
\section{Functions for class ~IndexRewriteLI~} 
 
\subsection{Function ~nextResult~}

*/
template<class M>
M* IndexRewriteLI<M>::nextResult() {
  if (matchesR[0].first == 0 || matchesR[0].first >= matchesR.size() ||
      matchesR.size() <= 1) {
    return 0;
  }
  NewPair<TupleId, IndexMatchInfo> match = matchesR[matchesR[0].first];
  Tuple *srcTuple = rel->GetTuple(match.first, false);
  M *src = (M*)(srcTuple->GetAttribute(attrNo));
  RewriteLI<M> rew(src, p);
  M *result = rew.rewrite(src, match.second, p->getAssigns());
  matchesR[0].first++;
  rew.clearMatch(false);
  srcTuple->DeleteIfAllowed();
  return result;
}

/*
\section{Implementation of Class ~DeriveGroupsLI~}

\subsection{Constructor}

*/
template<class M>
DeriveGroupsLI<M>::DeriveGroupsLI(Word _stream, double threshold, int attrNo) {
  Stream<Tuple> stream(_stream);
  std::vector<M*> trajStore;
  stream.open();
  Tuple *src = stream.request();
  int noTuples = 0;
  while (src != 0) {
    trajStore.push_back((M*)(src->GetAttribute(attrNo)->Clone()));
    src = stream.request();
    noTuples++;
  }
  stream.close();
//   std::vector<double> dist[noTuples];
//   for (int i = 0; i < noTuples; i++) {
//     for (int j = 0; j < i; j++) {
//       double distance = 0;
//       //double distance = trajStore[i]->Distance(*trajStore[j]);
//       if (distance <= threshold) {
//         dist[i].push_back(j);
//         cout << "pair (" << i << ", " << j << ") found" << endl;
//       }
//     }
//   }
}

/*
\subsection{Function ~getNextTuple~}

*/
template<class M>
Tuple* DeriveGroupsLI<M>::getNextTuple() {
  return 0;
}

}
