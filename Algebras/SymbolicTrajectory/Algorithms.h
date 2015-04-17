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
#include "RelationAlgebra.h"
#include "Stream.h"
#include "InvertedFile.h"
#include "RTreeAlgebra.h"
#include "FTextAlgebra.h"
#include "IntNfa.h"
#include "TemporalUnitAlgebra.h"
#include "GenericTC.h"
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

using namespace std;

int parsePatternRegEx(const char* argument, IntNfa** T);

union Word;

namespace stj {

class Pattern;
class PatElem;
class Assign;
class ClassifyLI;
class IndexLI;

Pattern* parseString(const char* input, bool classify, Tuple *t);
void patternFlushBuffer();

enum ExtBool {FALSE, TRUE, UNDEF};
enum Wildcard {NO, STAR, PLUS};

/*
\section{Class ~IBasic~}

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

/*
\section{Class ~IBasics~}

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
\section{Class ~UBasic~}

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
\section{Class ~UBasics~}

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

class MLabel;

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
  MBasic<B>& operator=(const MBasic<B>& src);
  void CopyFrom(const Attribute *arg);
  
  ListExpr unitToListExpr(const int i);
  ListExpr ToListExpr(ListExpr typeInfo);
  bool readUnitFrom(ListExpr LE);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);

  void serialize(size_t &size, char *bytes) const;
  void deserialize(const char *bytes);
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
  void Atperiods(const Periods& per, MBasic<B>& result) const;
  void Initial(IBasic<B>& result) const;
  void Final(IBasic<B>& result) const;
  void Inside(const typename B::coll& coll, MBool& result) const;
  void Fill(MBasic<B>& result, DateTime& duration) const;
  void Concat(const MBasic<B>& src1, const MBasic<B>& src2);
  void Compress(MBasic<B>& result) const;
  ostream& Print(ostream& os) const;
  double Distance(const MBasic<B>& mb) const;
  
 protected:
  Flob values;
  DbArray<typename B::unitelem> units;
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
  MBasics<B>& operator=(const MBasics<B>& src);
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  void Destroy() {values.destroy(); units.destroy(); pos.destroy();}
  
  int getUnitEndPos(const int i) const;
  ListExpr valuesToListExpr(int start, int end);
  ListExpr unitToListExpr(int i);
  bool readValues(ListExpr valuelist);
  bool readUnit(ListExpr unitlist);
  
  void serialize(size_t &size, char *bytes) const;
  void deserialize(const char *bytes);
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
  void Atperiods(const Periods& per, MBasics<B>& result) const;
  void Initial(IBasics<B>& result) const;
  void Final(IBasics<B>& result) const;
  void Fill(MBasics<B>& result, DateTime& duration) const;
  void Concat(const MBasics<B>& src1, const MBasics<B>& src2);
  void Compress(MBasics<B>& result) const;
  ostream& Print(ostream& os) const;
  double Distance(const MBasics<B>& mbs) const;
  
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
   
  void createML(const int size, const int number, vector<string>& labels);
  void convertFromMString(const MString& source);
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
  vector<pair<vector<pair<Word, ValueType> >, SetRel> > values;
  Wildcard wc;
  SetRel setRel;
  bool ok;

 public:
  PatElem() : var(""), ivs(), lbs(), pls(), values(), wc(NO), setRel(STANDARD),
              ok(true) {}
  PatElem(const char* contents, Tuple *tuple);
  PatElem(const PatElem& elem) : var(elem.var), ivs(elem.ivs), lbs(elem.lbs),
                pls(elem.pls), values(elem.values), wc(elem.wc),
                setRel(elem.setRel), ok(elem.ok) {}
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
  bool     extractValues(string &input, Tuple *tuple);
  vector<pair<vector<pair<Word, ValueType> >, SetRel> > getValues() const 
                                                          {return values;}
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
                      map<string, int> &varPosInSeq, stj::MLabel const &ml);
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
  bool isCompatible(TupleType *ttype, const int majorAttrNo);
  static Pattern* getPattern(string input, bool classify, 
                             Tuple *tuple = 0);
  template<class M>
  ExtBool matches(M *m);
  ExtBool tmatches(Tuple *tuple, const int attrno);
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
    Pattern *p = Pattern::getPattern(toText(), false);
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

extern TypeConstructor classifierTC;

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
                   map<string, pair<int, int> > &binding);
  void cleanPaths(map<int, int> &atomicToElem);
  bool cleanPath(unsigned int ulId, unsigned int pId);
  bool conditionsMatch(vector<Condition> &conds,
                 const map<string, pair<int, int> > &binding);
  bool evaluateEmptyM();
  bool isSensiblyBound(const map<string, pair<int, int> > &b, string& var);
  bool evaluateCond(Condition &cond,
                 const map<string, pair<int, int> > &binding);
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
//   map<string, pair<int, int> > binding;
};

template<class M>
class RewriteLI {
 public:
  RewriteLI(M *src, Pattern *pat);
  RewriteLI(int i) : match(0) {}

  ~RewriteLI() {}

  M* getNextResult();
  static M* rewrite(M *src, map<string, pair<int, int> > binding, 
                    vector<Assign> &assigns);
  void resetBinding(int limit);
  bool findNextBinding(unsigned int ulId, unsigned int pId, Pattern *p,
                       int offset);
  
 protected:
  stack<BindingStackElem> bindingStack;
  Match<M> *match;
  map<string, pair<int, int> > binding;
  set<map<string, pair<int, int> > > rewBindings;
};

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
  vector<Pattern*> pats;
  TupleType* classifyTT;
  set<int> matchingPats;
};

template<class M>
class MultiRewriteLI : public ClassifyLI, public RewriteLI<M> {
 public:
  MultiRewriteLI(Word _mlstream, Word _pstream, int _pos);

  ~MultiRewriteLI();

  M* nextResult();
  void getStartStates(set<int> &states);
  void initStack(set<int> &startStates);

 private:
  Stream<Tuple> tStream;
  int attrpos;
  bool streamOpen;
  M* traj;
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
  IndexMatchInfo(bool r, int n = 0) : 
       range(r), next(n), prevElem(-1) {binding.clear();}
  IndexMatchInfo(bool r, int n, const map<string, pair<int, int> >& b, int e) :
       range(r), next(n), prevElem(e) {if (b.size() > 0) {binding = b;}}
  
  void print(const bool printBinding);
  bool finished(const int size) const {return range || (next >= size);}
  bool exhausted(const int size) const {return next >= size;}
  bool matches(const int unit) const {return (range ? next<=unit : next==unit);}

  bool range;
  int next, prevElem;
  map<string, pair<int, int> > binding;
};

struct IndexMatchSlot {
  IndexMatchSlot() : pred(0), succ(0) {}
  
  unsigned int pred, succ;
  vector<IndexMatchInfo> imis;
};

class IndexMatchesLI {
 public:
  IndexMatchesLI(Relation *rel, InvertedFile *inv, 
    R_Tree<1, NewPair<TupleId, int> > *rt, int _attrNr, Pattern *_p, 
    bool deleteP, DataType type);

  ~IndexMatchesLI() {mRel = 0;}

  Tuple* nextTuple();
  void simplifyNFA(vector<map<int, int> >& result);
  void findNFApaths(const vector<map<int, int> >& nfa, 
                const set<int>& finalStates, set<pair<set<int>, int> >& result);
  void getCrucialElems(const set<pair<set<int>, int> >& paths,set<int>& result);
  void retrieveValue(vector<set<int> >& part, vector<set<int> >& part2,
                     SetRel rel, bool first, const string& label,
                     unsigned int ref = UINT_MAX);
  void retrieveTime(vector<set<int> >& oldPart, vector<set<int> >& newPart, 
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
  void applyNFA();
  template<class M>
  bool imiMatch(Match<M>& match, const int e, const TupleId id, 
                IndexMatchInfo& imi, const int unit, const int newState);
  bool valuesMatch(const int e, const TupleId id, IndexMatchInfo& imi,
                   const int newState, const int unit);
  void applySetRel(const SetRel setRel, 
                   vector<set<pair<TupleId, int> > >& valuePosVec,
                   set<pair<TupleId, int> >*& result);
  bool simpleMatch(const int e, const int state, const int newState);
  bool hasIdIMIs(const TupleId id, const int state = -1);
  bool wildcardMatch(const int state, pair<int, int> trans);
  bool timesMatch(const TupleId id,const unsigned int unit,const PatElem& elem);
  bool checkConditions(const TupleId id, IndexMatchInfo& imi);

 protected:
  Pattern p;
  Relation *mRel;
  vector<vector<IndexRetrieval> > indexResult;
  set<int> indexMismatch;
  vector<TupleId> matches;
  vector<int> trajSize;
  vector<bool> deactivated;
  int activeTuples, counter;
  vector<vector<IndexMatchSlot> > matchInfo, newMatchInfo;
  vector<vector<IndexMatchSlot> > *matchInfoPtr, *newMatchInfoPtr;
  InvertedFile* invFile;
  R_Tree<1, NewPair<TupleId, int> > *rtree;
  int attrNr;
  size_t maxMLsize;
  DataType mtype;
};

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
  queue<pair<string, TupleId> > results;
  int currentPat;
};

class UnitsLI {
 public:
  UnitsLI() : index(0) {}
  ~UnitsLI() {}

  int index;
};

template<class M>
class DeriveGroupsLI {
 public: 
  DeriveGroupsLI(Word _stream, double threshold, int attrNo);
  
  ~DeriveGroupsLI() {}
  
  Tuple *getNextTuple();
  
 private:
  vector<pair<TupleId, unsigned int> > result;
};

/*
\section{Implementation of Class ~IBasic~}

\subsection{Constructors}

*/
template<class B>
IBasic<B>::IBasic(const Instant& inst, const B& val) : Intime<B>(inst, val) {
  SetDefined(inst.IsDefined() && val.IsDefined());
}

template<class B>
IBasic<B>::IBasic(const IBasic& rhs) : Intime<B>(rhs.instant, rhs.value) {
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
  if (!Intime<B>::IsDefined()) {
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
                                                     Intime<B>(inst, values) {}

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
UBasic<B>::UBasic(const SecInterval &iv, const B& val)
                                            : ConstTemporalUnit<B>(iv, val) {
  this->SetDefined(iv.IsDefined() && val.IsDefined());
}

template<class B>
UBasic<B>::UBasic(const UBasic<B>& ub) : ConstTemporalUnit<B>(ub) {
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
  return nl->TwoElemList(((SecInterval)this->timeInterval).ToListExpr
          (nl->Empty()), this->constValue.ToListExpr(nl->Empty()));
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
  SecInterval iv(true);
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  if (iv.ReadFrom(nl->First(LE), sc->GetTypeExpr(SecInterval::BasicType())) &&
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
\section{Implementation of class ~UBasics~}

\subsection{Constructors}

*/
template<class B>
UBasics<B>::UBasics(const SecInterval &iv, const B& values)
                                           : ConstTemporalUnit<B>(iv, values) {}

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
  return nl->TwoElemList(((SecInterval)this->timeInterval).ToListExpr
          (nl->Empty()), this->constValue.ToListExpr(nl->Empty()));
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
  SecInterval iv(true);
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  if (iv.ReadFrom(nl->First(LE), sc->GetTypeExpr(SecInterval::BasicType())) &&
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
\section{Implementation of class ~MBasic~}

\subsection{Constructors}

*/
template<class B>
MBasic<B>::MBasic(const MBasic &mb) : Attribute(mb.IsDefined()),
                      values(mb.values.getSize()), units(mb.GetNoComponents()) {
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
}

/*
\subsection{Function ~=~}

*/
template<class B>
MBasic<B>& MBasic<B>::operator=(const MBasic<B>& src) {
  Attribute::operator=(src);
  units.copyFrom(src.units);
  values.copyFrom(src.values);
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
                                                     SecInterval::BasicType())),
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
  SecInterval iv;
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  if (!iv.ReadFrom(nl->First(LE), sc->GetTypeExpr(SecInterval::BasicType()))) {
    return false;
  }
  typename B::unitelem unit(values.getSize());
  unit.iv = iv;
  string text;
  if (!B::readValueFrom(nl->Second(LE), text, unit)) {
    return false;
  }
  units.Append(unit);
  text = text.substr(1, text.length() - 2);
  if (text.length() > 0) {
    const char *bytes = text.c_str();
    values.write(bytes, text.length(), values.getSize());
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
void MBasic<B>::serialize(size_t &size, char *bytes) const {
  size = 0;
  bytes = 0;
  if (!IsDefined()) {
    return;
  }
  size = sizeof(size_t) + sizeof(*this) + values.getSize() + units.getSize();
  bytes = new char[size];
  memcpy(bytes, (void*)&size, sizeof(size_t));
  memcpy(bytes, (void*)this, sizeof(*this));
  memcpy(bytes, (void*)&values, values.getSize());
  memcpy(bytes, (void*)&units, units.getSize());
}

/*
\subsection{Function ~deserialize~}

*/
template<class B>
void MBasic<B>::deserialize(const char *bytes) {
  size_t size;
  memcpy((void*)&size, bytes, sizeof(size_t));
  memcpy((void*)this, bytes + sizeof(size_t), size);
  memcpy((void*)&values, bytes + sizeof(size_t) + sizeof(this), 
         values.getSize());
  memcpy((void*)&units, bytes + sizeof(size_t) + sizeof(this) +values.getSize(),
         units.getSize());
}

/*
\subsection{Function ~Position~}

*/
template<class B>
int MBasic<B>::Position(const Instant& inst) const {
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
    if (((Interval<Instant>)(unit.iv)).Contains(t1)) {
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
\subsection{Function ~GetInterval~}

*/
template<class B>
void MBasic<B>::GetInterval(const int i, SecInterval& result) const {
  assert((i >= 0) && (i < GetNoComponents()));
  result.SetDefined(true);
  typename B::unitelem unit;
  units.Get(i, unit);
  result = unit.iv;
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
    int j = i + 1;
    bool finished = false;
    while (!finished && (j < units.Size())) {
      units.Get(j, next);
      if (next.pos != UINT_MAX) {
        finished = true;
        size = next.pos - cur.pos;
      }
    }
    if (!finished) {
      size = values.getSize() - cur.pos;
    }
    char* bytes = new char[size];
    values.read(bytes, size, cur.pos);
    string text(bytes, size);
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
}

/*
\subsection{Function ~Add~}

*/
template<class B>
void MBasic<B>::Add(const UBasic<B>& ub) {
  assert(ub.IsDefined());
  assert(ub.IsValid());
  UBasic<B> ub2(ub);
  if (IsDefined()) {
    readUnitFrom(ub2.ToListExpr(nl->Empty()));
  }
  assert(IsValid());
}

template<class B>
void MBasic<B>::Add(const SecInterval& iv, const B& value) {
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
      typename B::base value1, value2;
      GetValue(GetNoComponents() - 1, value1);
      ub.constValue.GetValue(value2);
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
  assert(IsValid());
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
                                                     SecInterval::BasicType())),
                                          B::getList(value1)));
    }
  }
}

/*
\subsection{Function ~DefTime~}

*/
template<class B>
void MBasic<B>::DefTime(Periods& per) const {
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
\subsection{Function ~Atperiods~}

*/
template<class B>
void MBasic<B>::Atperiods(const Periods& per, MBasic<B>& result) const {
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
  SecInterval ivS(true), ivP(true), ivR(true);
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
void MBasic<B>::Inside(const typename B::coll& coll, MBool& result) const {
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
    UBool ub(unit.iv, res);
    result.Add(ub);
  }
}

/*
\subsection{Function ~Fill~}

*/
template<class B>
void MBasic<B>::Fill(MBasic<B>& result, DateTime& dur) const {
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
  SecInterval iv1, iv2;
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
ostream& MBasic<B>::Print(ostream& os) const {
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
\subsection{Function ~Distance~}

*/
template<class B>
double MBasic<B>::Distance(const MBasic<B>& mb) const {
  if (!IsDefined() && !mb.IsDefined()) {
    return 0;
  }
  if (!IsDefined() || !mb.IsDefined()) {
    return 1;
  }
  if (IsEmpty() && mb.IsEmpty()) {
    return 0;
  }
  if (IsEmpty() || mb.IsEmpty()) {
    return 1;
  }
  int n = GetNoComponents() + 1;
  int m = mb.GetNoComponents() + 1;
  double dp[n][m];
  for (int i = 0; i < n; i++) {
    dp[i][0] = i;
  }
  for (int j = 0; j < m; j++) {
    dp[0][j] = j;
  }
  int labelFun = 0; // TODO: change
  typename B::base basic1, basic2;
  for (int i = 1; i < n; i++) {
    GetValue(i - 1, basic1);
    for (int j = 1; j < m; j++) {
      mb.GetValue(j - 1, basic2);
      dp[i][j] = min(dp[i - 1][j] + 1,
                 min(dp[i][j - 1] + 1, 
                     dp[i -1][j - 1] + Tools::distance(basic1, basic2, 
                                                       labelFun)));
    }
  }
  return dp[n - 1][m - 1] / max(n, m);
}

/*
\section{Implementation of class ~MBasics~}

\subsection{Constructors}

*/
template<class B>
MBasics<B>::MBasics(const MBasics &mbs) : Attribute(mbs.IsDefined()), 
  values(mbs.GetNoValues()), units(mbs.GetNoComponents()), pos(mbs.pos.Size()) {
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
  set<typename B::base> values;
  GetValues(i, values);
  ListExpr valuelist;
  B::valuesToListExpr(values, valuelist);
  return nl->TwoElemList(unit.iv.ToListExpr(sc->GetTypeExpr(
                                            SecInterval::BasicType())),
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
  string text;
  typename B::arrayelem elem;
  while (!nl->IsEmpty(rest)) {
    if (listutils::isSymbolUndefined(nl->First(rest))) {
      return false;
    }
    B::getString(nl->First(rest), text);
    text = text.substr(1, text.length() - 2);
    unsigned int newPos = (text.length() > 0 ? values.getSize() : UINT_MAX);
    B::getElemFromList(nl->First(rest), newPos, elem);
    pos.Append(elem);
    if (text.length() > 0) {
      const char *bytes = text.c_str();
      values.write(bytes, text.length(), values.getSize());
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
  SymbolicUnit unit(GetNoValues());
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  if (!unit.iv.ReadFrom(nl->First(unitlist), 
                        sc->GetTypeExpr(SecInterval::BasicType()))) {
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
void MBasics<B>::serialize(size_t &size, char *bytes) const {
  size = 0;
  bytes = 0;
  if (!IsDefined()) {
    return;
  }
  size = sizeof(size_t) + sizeof(*this) + values.getSize() + units.getSize()
         + pos.getSize();
  bytes = new char[size];
  memcpy(bytes, (void*)&size, sizeof(size_t));
  memcpy(bytes, (void*)this, sizeof(*this));
  memcpy(bytes, (void*)&values, values.getSize());
  memcpy(bytes, (void*)&units, units.getSize());
  memcpy(bytes, (void*)&pos, pos.getSize());
}

/*
\subsection{Function ~deserialize~}

*/
template<class B>
void MBasics<B>::deserialize(const char *bytes) {
  size_t size;
  memcpy((void*)&size, bytes, sizeof(size_t));
  memcpy((void*)this, bytes + sizeof(size_t), size);
  memcpy((void*)&values, bytes + sizeof(size_t) + sizeof(this), 
         values.getSize());
  memcpy((void*)&units, bytes + sizeof(size_t) + sizeof(this) +values.getSize(),
         units.getSize());
  memcpy((void*)&pos, bytes + sizeof(size_t) + sizeof(this) + values.getSize() 
         + units.getSize(), pos.getSize());
}

/*
\subsection{Operator ~<<~}

*/
template<class B>
ostream& operator<<(ostream& o, const MBasics<B>& mbs) {
  o << nl->ToString(mbs.ToListExpr());
  return o;
}

/*
\subsection{Function ~Position~}

*/
template<class B>
int MBasics<B>::Position(const Instant& inst) const {
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
    if (((Interval<Instant>)(unit.iv)).Contains(t1)) {
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
\subsection{Function ~Get~}

*/
template<class B>
void MBasics<B>::Get(const int i, UBasics<B>& result) const {
  assert(IsDefined());
  assert((i >= 0) && (i < GetNoComponents()));
  result.SetDefined(true);
  result.constValue.Clean();
  set<typename B::base> values;
  typename set<typename B::base>::iterator it;
  GetValues(i, values);
  for (it = values.begin(); it != values.end(); it++) {
    result.constValue.Append(*it);
  }
  SecInterval iv(true);
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
  set<typename B::base> values;
  typename set<typename B::base>::iterator it;
  GetValues(i, values);
  result.Append(values);
}

/*
\subsection{Function ~GetValues~}

*/
template<class B>
void MBasics<B>::GetValues(const int i, set<typename B::base>& result) const{
  assert (IsDefined() && (i >= 0) && (i < GetNoComponents()));
  result.clear();
  SymbolicUnit unit;
  units.Get(i, unit);
  typename B::base val;
  typename B::arrayelem elem1, elem2;
  pair<unsigned int, unsigned int> flobPos; // pos, size
  for (int j = unit.pos; j <= getUnitEndPos(i); j++) {
    flobPos = make_pair(0, 0);
    if (pos.Size() == 0) {
      cout << "try to get elem " << j << " of " << pos.Size();
    }
    pos.Get(j, elem1);
    unsigned int start = B::getFlobPos(elem1);
    if (start != UINT_MAX) {
      int k = j + 1;
      bool finished = false;
      while (!finished && (k < GetNoValues())) {
        pos.Get(k, elem2);
        unsigned int next = B::getFlobPos(elem2);
        if (next != UINT_MAX) { // valid reference
          flobPos = make_pair(start, next - start);
          finished = true;
        }
        k++;
      }
      if (!finished) { // end of array
        flobPos = make_pair(start, values.getSize() - start);
      }
    }
    if (flobPos.second > 0) {
      char *bytes = new char[flobPos.second];
      values.read(bytes, flobPos.second, flobPos.first);
      string text(bytes, flobPos.second);
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
void MBasics<B>::GetInterval(const int i, SecInterval& result) const {
  assert((i >= 0) && (i < GetNoComponents()));
  result.SetDefined(true);
  SymbolicUnit unit;
  units.Get(i, unit);
  result = unit.iv;
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
void MBasics<B>::Add(const SecInterval& iv, const B& values) {
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
void MBasics<B>::MergeAdd(const SecInterval& iv, const B& values) {
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
    set<typename B::base> mvalues, bvalues;
    typename set<typename B::base>::iterator it;
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
  assert(IsValid());
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
  set<typename B::base> vals;
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
      SecInterval iv;
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
      SecInterval iv;
      GetInterval(i, iv);
      result.Add(iv, basics);
    }
  }
}

/*
\subsection{Function ~DefTime~}

*/
template<class B>
void MBasics<B>::DefTime(Periods& per) const {
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
\subsection{Function ~Atperiods~}

*/
template<class B>
void MBasics<B>::Atperiods(const Periods& per, MBasics<B>& result) const {
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
  SecInterval ivS(true), ivP(true), ivR(true);
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
  SecInterval iv;
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
  SecInterval iv;
  GetInterval(GetNoComponents() - 1, iv);
  result.instant = iv.end;
  GetBasics(GetNoComponents() - 1, result.value);
}

/*
\subsection{Function ~Fill~}

*/
template<class B>
void MBasics<B>::Fill(MBasics<B>& result, DateTime& dur) const {
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
  SecInterval iv1, iv2;
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
ostream& MBasics<B>::Print(ostream& os) const {
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
  set<typename B::base> basics1, basics2;
  int fun = 0; // TODO: change
  int labelFun = 0; // TODO: change
  for (int i = 1; i < n; i++) {
    GetValues(i - 1, basics1);
    for (int j = 1; j < m; j++) {
      mbs.GetValues(j - 1, basics2);
      dp[i][j] = min(dp[i - 1][j] + 1,
                 min(dp[i][j - 1] + 1, 
           dp[i -1][j - 1] + Tools::distance(basics1, basics2, fun, labelFun)));
    }
  }
  return dp[n - 1][m - 1] / max(n, m);
}
 
/*
\subsection{Function ~matches~}

Checks the pattern and the condition and (if no problem occurs) invokes the NFA
construction and the matching procedure.

*/
template<class M>
ExtBool Pattern::matches(M *m) {
  ExtBool result = UNDEF;
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
    return UNDEF;
  }
  if (!p->initEasyCondOpTrees()) {
    cout << "Error: EasyCondOpTrees not initialized" << endl;
    return UNDEF;
  }
  set<int> states;
  states.insert(0);
  if (!p->hasConds() && !p->hasAssigns()) {
    for (int i = 0; i < m->GetNoComponents(); i++) {
      if (!updateStates(i, p->nfa, p->elems, p->finalStates, states,
                        p->easyConds, p->easyCondPos, p->atomicToElem)) {
//           cout << "mismatch at unit " << i << endl;
        return FALSE;
      }
    }
    if (!p->containsFinalState(states)) {
//         cout << "no final state is active" << endl;
      return FALSE;
    }
  }
  else {
    createSetMatrix(m->GetNoComponents(), p->elemToVar.size());
    for (int i = 0; i < m->GetNoComponents(); i++) {
      if (!updateStates(i, p->nfa, p->elems, p->finalStates, states,
                        p->easyConds, p->easyCondPos, p->atomicToElem, true)){
//           cout << "mismatch at unit " << i << endl;
        Tools::deleteSetMatrix(matching, m->GetNoComponents());
        return FALSE;
      }
    }
    if (!p->containsFinalState(states)) {
//         cout << "no final state is active" << endl;
      Tools::deleteSetMatrix(matching, m->GetNoComponents());
      return FALSE;
    }
    if (!p->initCondOpTrees()) {
      Tools::deleteSetMatrix(matching, m->GetNoComponents());
      return UNDEF;
    }
    if (!p->hasAssigns()) {
      bool result = findMatchingBinding(p->nfa, 0, p->elems, p->conds, 
                                        p->atomicToElem, p->elemToVar);
      Tools::deleteSetMatrix(matching, m->GetNoComponents());
      return (result ? TRUE : FALSE);
    }
    return TRUE; // happens iff rewrite is called
  }
  return TRUE;
}

/*
\subsection{Function ~states2Str~}

Writes the set of currently active states into a string.

*/
template<class M>
string Match<M>::states2Str(int ulId, set<int> &states) {
  stringstream result;
  if (!states.empty()) {
    set<int>::iterator it = states.begin();
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
string Match<M>::matchings2Str(unsigned int dim1, unsigned int dim2) {
  stringstream result;
  for (unsigned int i = 0; i < dim1; i++) {
    for (unsigned int j = 0; j < dim2; j++) {
      if (matching[i][j].empty()) {
        result << "                    ";
      }
      else {
        string cell;
        set<unsigned int>::iterator it, it2;
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

template<class T>
bool Tools::relationHolds(const set<T>& s1, const set<T>& s2, SetRel rel) {
  set<T> temp;
  switch (rel) {
    case STANDARD: {
      set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(), 
                     std::inserter(temp, temp.begin()));
      return temp.empty();
    }
    case DISJOINT: {
      set_union(s1.begin(), s1.end(), s2.begin(), s2.end(),
                std::inserter(temp, temp.begin()));
      return (temp.size() == s1.size() + s2.size());
    }
    case SUPERSET: {
      set_difference(s2.begin(), s2.end(), s1.begin(), s1.end(), 
                     std::inserter(temp, temp.begin()));
      return temp.empty();
    }
    case EQUAL: {
      return s1 == s2;
    }
    case INTERSECT: {
      set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), 
                std::inserter(temp, temp.begin()));
      return (temp.size() < s1.size() + s2.size());
    }
    default: // cannot occur
      return false;
  }
}

/*
\subsection{Function ~valuesMatch~}

*/
template<class M>
bool Match<M>::valuesMatch(int i, const PatElem& elem) {
  set<pair<string, unsigned int> > ppls, mpls;
  set<string> plbs, mlbs;
  if (type < 2) { // label or labels
    elem.getL(plbs);
  }
  else { // place or places
    elem.getP(ppls);
  }
  if (plbs.empty() && ppls.empty() && (elem.getSetRel() < SUPERSET)) {
    return true; // easiest case
  }
  switch (type) {
    case MLABEL: {
      string mlb;
      ((MLabel*)m)->GetValue(i, mlb);
      if (elem.getSetRel() == STANDARD) {
        return plbs.find(mlb) != plbs.end();
      }
      mlbs.insert(mlb);
      return Tools::relationHolds<string>(mlbs, plbs, elem.getSetRel());
    }
    case MLABELS: {
      ((MLabels*)m)->GetValues(i, mlbs);
      return Tools::relationHolds<string>(mlbs, plbs, elem.getSetRel());
    }
    case MPLACE: {
      pair<string, unsigned int> mpl;
      ((MPlace*)m)->GetValue(i, mpl);
      if (elem.getSetRel() == STANDARD) {
        return ppls.find(mpl) != ppls.end();
      }
      mpls.insert(mpl);
      return Tools::relationHolds<pair<string, unsigned int> >(mpls, ppls, 
                                                              elem.getSetRel());
    }  
    case MPLACES: {
      ((MPlaces*)m)->GetValues(i, mpls);
      return Tools::relationHolds<pair<string, unsigned int> >(mpls, ppls, 
                                                              elem.getSetRel());
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
bool Match<M>::updateStates(int ulId, vector<map<int, int> > &nfa,
               vector<PatElem> &elems, set<int> &finalStates, set<int> &states,
               vector<Condition> &easyConds, map<int, set<int> > &easyCondPos,
               map<int, int> &atomicToElem, bool store /* = false */) {
  set<int>::iterator its;
  set<unsigned int>::iterator itu;
  map<int, int> transitions;
  for (its = states.begin(); its != states.end(); its++) { // collect possible
    map<int, int> trans = nfa[*its];                       // transitions
    transitions.insert(trans.begin(), trans.end());
  }
  if (transitions.empty()) {
    return false;
  }
  states.clear();
  map<int, int>::iterator itm, itn;
  SecInterval iv;
  m->GetInterval(ulId, iv);
  set<string> ivs;
  if (store) {
    if (ulId < m->GetNoComponents() - 1) { // usual case
      for (itm = transitions.begin(); itm != transitions.end(); itm++) {
        elems[itm->first].getI(ivs);
        if (valuesMatch(ulId, elems[itm->first]) && Tools::timesMatch(iv, ivs)
         && easyCondsMatch(ulId, elems[itm->first], easyConds,
                           easyCondPos[itm->first])) {
          states.insert(states.end(), itm->second);
          map<int, int> nextTrans = nfa[itm->second];
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
void Match<M>::cleanPaths(map<int, int> &atomicToElem) {
  map<int, int> transitions = p->getTransitions(0);
  map<int, int>::reverse_iterator itm;
  for (itm = transitions.rbegin(); itm != transitions.rend(); itm++) {
    cleanPath(0, atomicToElem[itm->first]);
  }
}

/*
\subsection{Function ~findMatchingBinding~}

Searches for a binding which fulfills every condition.

*/
template<class M>
bool Match<M>::findMatchingBinding(vector<map<int, int> > &nfa, int startState,
                                vector<PatElem> &elems,vector<Condition> &conds,
                                map<int, int> &atomicToElem,
                                map<int, string> &elemToVar) {
  if ((startState < 0) || (startState > (int)nfa.size() - 1)) {
    return false; // illegal start state
  }
  if (conds.empty()) {
    return true;
  }
  map<int, int> transitions = nfa[startState];
  map<string, pair<int, int> > binding;
  map<int, int>::reverse_iterator itm;
  for (itm = transitions.rbegin(); itm != transitions.rend(); itm++) {
    if (findBinding(0, atomicToElem[itm->first], elems, conds, elemToVar,
                    binding)) {
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
                          vector<PatElem> &elems, vector<Condition> &conds,
                          map<int, string> &elemToVar,
                          map<string, pair<int, int> > &binding) {
  string var = elemToVar[pId];
  bool inserted = false;
  if (!var.empty()) {
    if (binding.count(var)) { // extend existing binding
      binding[var].second++;
    }
    else { // add new variable
      binding[var] = make_pair(ulId, ulId);
      inserted = true;
    }
  }
  if (*(matching[ulId][pId].begin()) == UINT_MAX) { // complete match
    if (conditionsMatch(conds, binding)) {
      return true;
    }
  }
  else {
    for (set<unsigned int>::reverse_iterator it = matching[ulId][pId].rbegin();
         it != matching[ulId][pId].rend(); it++) {
      if (findBinding(ulId + 1, *it, elems, conds, elemToVar, binding)) {
        return true;
      }
    }
  }
  if (!var.empty()) { // unsuccessful: reset binding
    if (inserted) {
      binding.erase(var);
    }
    else {
      binding[var].second--;
    }
  }
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
  set<unsigned int>::iterator it;
  vector<unsigned int> toDelete;
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
                              vector<Condition> &easyConds, set<int> pos) {
  if (elem.getW() || pos.empty() || easyConds.empty()) {
    return true;
  }
  map<string, pair<int, int> > binding;
  string var;
  elem.getV(var);
  binding[var] = make_pair(ulId, ulId);
  for (set<int>::iterator it = pos.begin(); it != pos.end(); it++) {
    if (!evaluateCond(easyConds[*it], binding)) {
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
bool Match<M>::conditionsMatch(vector<Condition> &conds,
                const map<string, pair<int, int> > &binding) {
  if (!m->GetNoComponents()) { // empty MLabel
    return evaluateEmptyM();
  }
  for (unsigned int i = 0; i < conds.size(); i++) {
    if (!evaluateCond(conds[i], binding)) {
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
bool Match<M>::isSensiblyBound(const map<string, pair<int, int> > &b, 
                               string& var) {
  int first = b.find(var)->second.first;
  int second = b.find(var)->second.second;
  return (first >= 0 && first < m->GetNoComponents() &&
          second >= 0 && second < m->GetNoComponents() && first <= second);
}

/*
\subsection{Function ~evaluateCond~}

This function is invoked by ~conditionsMatch~ and checks whether a binding
matches a certain condition.

*/
template<class M>
bool Match<M>::evaluateCond(Condition &cond,
                            const map<string, pair<int, int> > &binding) {
  Word qResult;
  unsigned int from, to;
  SecInterval iv(true);
  for (int i = 0; i < cond.getVarKeysSize(); i++) {
    string var = cond.getVar(i);
    if ((var != "") && binding.count(var)) {
      from = binding.find(var)->second.first;
      to = binding.find(var)->second.second;
      switch (cond.getKey(i)) {
        case 0: { // label
          string value;
          ((MLabel*)m)->GetValue(from, value);
          cond.setValuePtr(i, value);
          break;
        }
        case 1: { // place
          pair<string, unsigned int> value;
          ((MPlace*)m)->GetValue(from, value);
          cond.setValuePtr(i, value);
          break;
        }
        case 2: { // time
          cond.clearTimePtr(i);
          for (unsigned int j = from; j <= to; j++) {
            m->GetInterval(j, iv);
            cond.mergeAddTimePtr(i, iv);
          }
          break;
        }
        case 3: { // start
          m->GetInterval(from, iv);
          cond.setStartEndPtr(i, iv.start);
          break;
        }
        case 4: { // end
          m->GetInterval(to, iv);
          cond.setStartEndPtr(i, iv.end);
          break;
        }
        case 5: { // leftclosed
          m->GetInterval(from, iv);
          cond.setLeftRightclosedPtr(i, iv.lc);
          break;
        }
        case 6: { // rightclosed
          m->GetInterval(to, iv);
          cond.setLeftRightclosedPtr(i, iv.rc);
          break;
        }
        case 7: { // card
          cond.setCardPtr(i, to - from + 1);
          break;
        }
        case 8: { // labels
          cond.cleanLabelsPtr(i);
          if (type == MLABEL) {
            for (unsigned int j = from; j <= to; j++) {
              string value;
              ((MLabel*)m)->GetValue(j, value);
              cond.appendToLabelsPtr(i, value);
            }
          }
          else {
            for (unsigned int j = from; j <= to; j++) {
              set<string> values;
              ((MLabels*)m)->GetValues(j, values);
              cond.appendToLabelsPtr(i, values);
            }
          }
          break;
        }
        default: { // places
          cond.cleanPlacesPtr(i);
          if (type == MPLACE) {
            for (unsigned int j = from; j <= to; j++) {
              pair<string, unsigned int> value;
              ((MPlace*)m)->GetValue(j, value);
              cond.appendToPlacesPtr(i, value);
            }
          }
          else {
            for (unsigned int j = from; j <= to; j++) {
              set<pair<string, unsigned int> > values;
              ((MPlaces*)m)->GetValues(j, values);
              cond.appendToPlacesPtr(i, values);
            }
          }
        }
      }
    }
    else { // variable bound to empty sequence
      switch (cond.getKey(i)) {
        case 0: { // label
          string value("");
          cond.setValuePtr(i, value);
          break;
        }
        case 1: { // place
          pair<string, unsigned int> value;
          cond.setValuePtr(i, value);
          break;
        }
        case 2: {
          cond.clearTimePtr(i);
          m->GetInterval(m->GetNoComponents() - 1, iv);
          iv.SetStart(iv.end, false);
          cond.mergeAddTimePtr(i, iv);
          break;
        }
        case 3: // start
        case 4: { // end
          m->GetInterval(m->GetNoComponents() - 1, iv);
          cond.setStartEndPtr(i, iv.end);
          break;
        }
        case 5: // leftclosed
        case 6: { // rightclosed
          m->GetInterval(m->GetNoComponents() - 1, iv);
          cond.setLeftRightclosedPtr(i, iv.rc);
          break;
        }
        case 7: {
          cond.setCardPtr(i, 0);
          break;
        }
        case 8: {
          cond.cleanLabelsPtr(i);
          break;
        }
        case 9: {
          cond.cleanPlacesPtr(i);
          break;
        }
        default: {
          return true;
        }
      }
    }
  }
  cond.getQP()->EvalS(cond.getOpTree(), qResult, OPEN);
  return ((CcBool*)qResult.addr)->GetValue();
}


/*
\subsection{Constructors for class ~FilterMatchesLI~}

*/
template<class M>
FilterMatchesLI<M>::FilterMatchesLI(Word _stream, int _attrIndex, string& pText)
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
    if (match->matches() == TRUE) {
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
      match->initCondOpTrees();
      match->initAssignOpTrees();
      if (!src->GetNoComponents()) {
        BindingStackElem dummy(0, 0);
        bindingStack.push(dummy);
      }
      else {
        map<int, int> transitions = pat->getTransitions(0);
        for (map<int, int>::iterator itm = transitions.begin();
                                    itm != transitions.end(); itm++) {
          BindingStackElem bE(0, itm->first); // init stack
    //       cout << "push (0, " << itm->first << ") to stack" << endl;
          bindingStack.push(bE);
        }
      }
    }
    else {
      match->deletePattern();
      delete match;
      match = 0;
    }
  }
  else {
    cout << "pattern is not suitable for type " << MLabel::BasicType() << endl;
    match = 0;
  }
}

/*
\subsection{Function ~rewrite~}

*/
template<class M>
M* RewriteLI<M>::rewrite(M *src, map<string, pair<int, int> > binding,
                         vector<Assign> &assigns){
  M *result = new M(true);
  Word qResult;
  Instant start(instanttype), end(instanttype);
  SecInterval iv(true), iv2(true);
  bool lc(false), rc(false);
  pair<unsigned int, unsigned int> segment;
  assert(src->IsValid());
  for (unsigned int i = 0; i < assigns.size(); i++) {
    for (int j = 0; j <= 9; j++) {
      if (!assigns[i].getText(j).empty()) {
        for (int k = 0; k < assigns[i].getRightSize(j); k++) {
          if (binding.count(assigns[i].getRightVar(j, k))) {
            segment = binding[assigns[i].getRightVar(j, k)];
            switch (assigns[i].getRightKey(j, k)) {
              case 0: { // label
                string lvalue;
                ((MLabel*)src)->GetValue(segment.first, lvalue);
                assigns[i].setLabelPtr(k, lvalue);
                break;
              }
              case 1: { // place
                pair<string, unsigned int> pvalue;
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
                set<Labels::base> lvalues;
                for (unsigned int m = segment.first; m <= segment.second; m++) {
                  ((MLabels*)src)->GetValues(m, lvalues);
                  assigns[i].appendToLabelsPtr(k, lvalues);
                }
                break;
              }
              case 9: { // places
                set<Places::base> pvalues;
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
      iv2 = *((SecInterval*)qResult.addr);
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
    if (binding.count(assigns[i].getV())) { // variable occurs in binding
      segment = binding[assigns[i].getV()];
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
            string lvalue;
            ((MLabel*)src)->GetValue(segment.first, lvalue);
            ((MLabel*)result)->Add(iv, lvalue);
          }
          else {
            ((MLabel*)result)->Add(iv, lb);
          }
        }
        else if (M::BasicType() == MPlace::BasicType()) {
          if (!assigns[i].getQP(1)) {
            pair<string, unsigned int> pvalue;
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
          } // TODO: collect values from src
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
  assert(result->IsValid());
  return result;
}

template<class M>
M* RewriteLI<M>::getNextResult() {
  if (!match) {
    return 0;
  }
  if (!match->m->GetNoComponents()) { // empty mlabel
    if (bindingStack.empty()) {
      return 0;
    }
    bindingStack.pop();
    vector<Condition> *conds = match->p->getConds();
    if (match->conditionsMatch(*conds, binding)) {
      M *source = match->m;
      return rewrite(source, binding, match->p->getAssigns());
    }
    return 0;
  }
  else { // non-empty mlabel
    BindingStackElem bE(0, 0);
    while (!bindingStack.empty()) {
      bE = bindingStack.top();
  //    cout << "take (" << bE.ulId << ", " << bE.pId << ") from stack" << endl;
      bindingStack.pop();
      resetBinding(bE.ulId);
      if (findNextBinding(bE.ulId, bE.peId, match->p, 0)) {
//         match->printBinding(binding);
        if (!rewBindings.count(binding)) {
          rewBindings.insert(binding);
          M *source = match->m;
          return rewrite(source, binding, match->p->getAssigns());
        }
      }
    }
  //   cout << "stack is empty" << endl;
    match->deletePattern();
    match->deleteSetMatrix();
    delete match;
    return 0;
  }
}

template<class M>
void RewriteLI<M>::resetBinding(int limit) {
  vector<string> toDelete;
  map<string, pair<int, int> >::iterator it;
  for (it = binding.begin(); it != binding.end(); it++) {
    if (it->second.first >= limit) {
      toDelete.push_back(it->first);
    }
    else if (it->second.second >= limit) {
      it->second.second = limit - 1;
    }
  }
  for (unsigned int i = 0; i < toDelete.size(); i++) {
    binding.erase(toDelete[i]);
  }
}

template<class M>
bool RewriteLI<M>::findNextBinding(unsigned int ulId, unsigned int peId,
                                   Pattern *p, int offset) {
//   cout << "findNextBinding(" << ulId << ", " << peId << ", " << offset
//        << ") called" << endl;
  string var = p->getVarFromElem(peId - offset);
  if (!var.empty() && p->isRelevant(var)) {
    if (binding.count(var)) { // extend existing binding
      binding[var].second++;
    }
    else { // add new variable
      binding[var] = make_pair(ulId, ulId);
    }
  }
  if (*(match->matching[ulId][peId].begin()) == UINT_MAX) { // complete match
    vector<Condition> *conds = p->getConds();
    return match->conditionsMatch(*conds, binding);
  }
  if (match->matching[ulId][peId].empty()) {
    return false;
  }
  else { // push all elements except the first one to stack; process first elem
    set<unsigned int>::reverse_iterator it, it2;
    it2 = match->matching[ulId][peId].rbegin();
    it2++;
    for (it = match->matching[ulId][peId].rbegin();
         it2 != match->matching[ulId][peId].rend(); it++) {
      it2++;
      BindingStackElem bE(ulId + 1, *it);
//       cout << "push (" << ulId + 1 << ", " << *it << ") to stack" << endl;
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
  set<unsigned int> emptyset;
  set<int> states, finalStates, matchCands;
  set<int>::iterator it;
  Pattern *p = 0;
  PatElem elem;
  vector<PatElem> patElems;
  vector<Condition> easyConds;
  vector<int> startStates;
  map<int, set<int> > easyCondPos;
  map<int, int> atomicToElem; // TODO: use this sensibly
  map<int, string> elemToVar; // TODO: use this sensibly
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
      map<int, set<int> > easyOld = p->getEasyCondPos();
      for (map<int, set<int> >::iterator im = easyOld.begin();
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
  vector<map<int, int> > nfa;
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
    vector<Condition>* conds = pats[*it]->getConds();
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
  set<int>::iterator it;
  map<int, set<int> >::iterator im;
  int elemCount(0);
  string var;
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
              patElems.push_back(elem);
              patOffset[elemCount + p->getElemFromAtom(i)] =
                                          make_pair(pats.size() - 1, elemCount);
            }
            elemCount += p->getElemFromAtom(p->getSize() - 1) + 1;
            map<int, set<int> > easyOld = p->getEasyCondPos();
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
  set<int> startStates;
  set<int>::iterator it;
  while (!this->bindingStack.empty()) {
    BindingStackElem bE(0, 0);
    bE = this->bindingStack.top();
//     cout << "take (" << bE.ulId << ", " << bE.pId << ") from stack" << endl;
    this->bindingStack.pop();
    this->resetBinding(bE.ulId);
    pair<int, int> patNo = patOffset[bE.peId];
    if (this->findNextBinding(bE.ulId, bE.peId, pats[patNo.first], 
        patNo.second)) {
      return RewriteLI<M>::rewrite(traj, this->binding, 
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
    while (!this->bindingStack.empty()) {
      BindingStackElem bE(0, 0);
      bE = this->bindingStack.top();
//      cout << "take (" << bE.ulId << ", " << bE.pId << ") from stack" << endl;
      this->bindingStack.pop();
      this->resetBinding(bE.ulId);
      pair<int, int> patNo = patOffset[bE.peId];
      if (this->findNextBinding(bE.ulId, bE.peId, pats[patNo.first], 
                                patNo.second)) {
        return RewriteLI<M>::rewrite(traj, this->binding, 
                                     pats[patNo.first]->getAssigns());
      }
    }
    tuple->DeleteIfAllowed();
    tuple = 0;
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
void MultiRewriteLI<M>::initStack(set<int> &startStates) {
  set<int>::iterator it;
  map<int, int>::iterator itm;
  for (it = startStates.begin(); it != startStates.end(); it++) {
    if (matchCands.count(-state2Pat[*it])) {
      map<int, int> transitions = nfa[*it];
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
void MultiRewriteLI<M>::getStartStates(set<int> &states) {
  states.clear();
  states.insert(0);
  map<int, int>::iterator it;
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
    p = *pat;
    delete pat;
    initialize();
    applyNFA();
    for (unsigned int i = 0; i < matches.size(); i++) {
      results.push(make_pair(c->getDesc(currentPat), matches[i]));
    }
    p.deleteCondOpTrees();
    cout << "desc for " << currentPat << " is " << c->getDesc(currentPat)<<endl;
    currentPat++;
  }
  pair<string, TupleId> resultPair = results.front();
  results.pop();
  cout << "tuple id " << resultPair.second << " popped, " << results.size() 
       << " left" << endl;
   
  Tuple* tuple = mRel->GetTuple(resultPair.second, false);
  int noValues = ((MLabels*)(tuple->GetAttribute(attrNr)))->GetNoValues();
  cout << "Trajectory has "
       << ((MLabels*)(tuple->GetAttribute(attrNr)))->GetNoComponents()
       << " units and " << noValues << " labels" << endl;

  ((MLabels*)(tuple->GetAttribute(attrNr)))->Print(cout);
  
  
  
  Attribute* traj = (tuple->GetAttribute(attrNr))->Copy();
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
  p.getElem(e, elem);
  if (unit >= 0) { // exact position from index
//     cout << "   unit=" << unit << ", imi: next=" << imi.next << ", range=" 
//          << (imi.range ? "TRUE" : "FALSE") << endl;
    if (imi.matches(unit) && match.valuesMatch(unit, elem) &&
        timesMatch(id, unit, elem) &&
        match.easyCondsMatch(unit, elem, p.easyConds, p.getEasyCondPos(e))) {
      if (unit + 1 >= counter) {
        IndexMatchInfo newIMI(false, unit + 1, imi.binding, imi.prevElem);
        extendBinding(newIMI, e);
//         if (newIMI.finished(trajSize[id])) {
//           newIMI.print(true); cout << "   ";
//         }
        if (p.isFinalState(newState) && newIMI.finished(trajSize[id]) && 
            checkConditions(id, newIMI)) { // complete match
          removeIdFromIndexResult(id);
          removeIdFromMatchInfo(id);
  //         cout << id << " removed (index match) " << activeTuples 
  //              << " active tuples" << endl;
          matches.push_back(id);
          return true;
        }
        else if (!newIMI.exhausted(trajSize[id])) { // continue
          (*newMatchInfoPtr)[newState][id].imis.push_back(newIMI);
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
          match.easyCondsMatch(i, elem, p.easyConds, p.getEasyCondPos(e))) {
        if (i + 1 >= counter) {
          IndexMatchInfo newIMI(false, i + 1, imi.binding, imi.prevElem);
          extendBinding(newIMI, e);
          if (p.isFinalState(newState) && imi.finished(trajSize[id]) &&
              checkConditions(id, newIMI)) { // complete match
            removeIdFromMatchInfo(id);
  //           cout << id << " removed (range match) " << activeTuples 
  //                << " active tuples" << endl;
            removeIdFromIndexResult(id);
            matches.push_back(id);
            return true;
          }
          else if (!newIMI.exhausted(trajSize[id])) { // continue
            if ((*newMatchInfoPtr)[newState].size() == 0) {
              (*newMatchInfoPtr)[newState].resize(mRel->GetNoTuples());
            }
            (*newMatchInfoPtr)[newState][id].imis.push_back(newIMI);
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
        match.easyCondsMatch(imi.next, elem, p.easyConds, p.getEasyCondPos(e))){
      if (imi.next + 1 >= counter) {
        IndexMatchInfo newIMI(false, imi.next + 1, imi.binding, imi.prevElem);
        extendBinding(newIMI, e);
        if (p.isFinalState(newState) && imi.finished(trajSize[id]) && 
            checkConditions(id, newIMI)) { // complete match
          removeIdFromMatchInfo(id);
  //         cout << id << " removed (match) " << activeTuples
  //              << " active tuples" << endl;
          matches.push_back(id);
          return true;
        }
        else if (!newIMI.exhausted(trajSize[id])) { // continue
          if ((*newMatchInfoPtr)[newState].size() == 0) {
            (*newMatchInfoPtr)[newState].resize(mRel->GetNoTuples());
          }
          (*newMatchInfoPtr)[newState][id].imis.push_back(newIMI);
          return true;
        }
      }
    }
  }
  return false;
}

/*
\section{Implementation of Class ~DeriveGroupsLI~}

\subsection{Constructor}

*/
template<class M>
DeriveGroupsLI<M>::DeriveGroupsLI(Word _stream, double threshold, int attrNo) {
  Stream<Tuple> stream(_stream);
  vector<M*> trajStore;
  stream.open();
  Tuple *src = stream.request();
  int noTuples = 0;
  while (src != 0) {
    trajStore.push_back((M*)(src->GetAttribute(attrNo)->Clone()));
    src = stream.request();
    noTuples++;
  }
  stream.close();
//   vector<double> dist[noTuples];
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
