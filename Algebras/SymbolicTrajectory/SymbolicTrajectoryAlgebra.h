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
#include "NestedList.h"
#include "SymbolicTrajectoryTools.h"
#include "StandardTypes.h"
#include "TemporalAlgebra.h"
#include "ListUtils.h"
#include "RelationAlgebra.h"
#include "Stream.h"
#include "InvertedFile.h"
#include "FTextAlgebra.h"
#include "IntNfa.h"
#include <string>
#include <set>
#include <stack>

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

enum ExtBool {FALSE, TRUE, UNDEF};
enum Wildcard {NO, STAR, PLUS};
enum LabelsState {partial, complete};

Pattern* parseString(const char* input, bool classify);
void patternFlushBuffer();

class Label : public Attribute {
 public:
  Label() {};
  Label(const string& val);
  Label(const Label& rhs);
  Label(const bool def) {SetDefined(def); strncpy(text, "", MAX_STRINGSIZE);}
  ~Label();

  string GetValue() const;
  void Set(const string &value);
  Label* Clone();

  static Word     In(const ListExpr typeInfo, const ListExpr instance,
                     const int errorPos, ListExpr& errorInfo, bool& correct);
  static ListExpr Out(ListExpr typeInfo, Word value);
  static Word     Create(const ListExpr typeInfo);
  static void     Delete(const ListExpr typeInfo, Word& w);
  static void     Close(const ListExpr typeInfo, Word& w);
  static Word     Clone(const ListExpr typeInfo, const Word& w);
  static bool     KindCheck(ListExpr type, ListExpr& errorInfo);
  static int      SizeOfObj();
  static ListExpr Property();
  static const string BasicType() {return "label";}
  static const bool checkType(const ListExpr type);
  const bool      IsDefined() {return true;}
  void            CopyFrom(const Attribute* right);
  int             Compare(const Attribute* arg) const;
  size_t          Sizeof() const;
  bool            Adjacent(const Attribute*) const;
  Label*          Clone() const;
  size_t          HashValue() const;
  void            SetValue(const string &value);
  ostream&        Print(ostream& os) const {return os << GetValue();}

  char text[MAX_STRINGSIZE + 1];
};

int CompareLabels(const void *a, const void *b);

class Labels : public Attribute {
 public:
  Labels(const int n, const Label *Lb = 0);
  Labels(const Labels& src);
  ~Labels();

  Labels& operator=(const Labels& src);

  int NumOfFLOBs() const;
  Flob *GetFLOB(const int i);
  int Compare(const Attribute*) const;
  bool Adjacent(const Attribute*) const;
  Labels *Clone() const;
  size_t Sizeof() const;
  ostream& Print(ostream& os) const;

  void Append( const Label &lb );
  void Complete();
  void Destroy();
  int GetNoLabels() const;
  Label GetLabel(int i) const;
  string GetState() const;
  const bool IsEmpty() const;
  void CopyFrom(const Attribute* right);
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
  static int      SizeOfObj();
  static ListExpr Property();
  static void*    Cast(void* addr);
  static const    string BasicType() {return "labels";}
  static const    bool checkType(const ListExpr type) {
                                 return listutils::isSymbol(type, BasicType());}
  DbArray<Label> GetDbArray() {return labels;}
  void Sort() {labels.Sort(CompareLabels);}
  void Clean() {if (labels.Size()) {labels.clean();} state = partial;}

 private:
  Labels() {} // this constructor is reserved for the cast function.
  DbArray<Label> labels;
  LabelsState state;
};

class ILabel : public IString {
public:
  static const string BasicType() { return "ilabel"; }
  static ListExpr IntimeLabelProperty();
  static bool CheckIntimeLabel(ListExpr type, ListExpr& errorInfo);
  static bool checkType(ListExpr t) {
    return listutils::isSymbol(t, BasicType());
  }
};

class MLabel : public MString {
 public:
  MLabel() {}
  explicit MLabel(int i): MString(i), index(0) {}
  MLabel(MString* ms);
  MLabel(MLabel* ml);
  
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
  size_t Sizeof() const{
    return sizeof(*this);
  }

  void Initialize() {index.initRoot();}
  void Finalize() {index.initRoot();}
  
  MLabel* compress();
  void createML(int size, bool text, double rate);
  void convertFromMString(MString* source);
  MLabel* rewrite(map<string, pair<unsigned int, unsigned int> > binding,
                  vector<Assign> &assigns) const;
  const bool hasIndex() {
    return (index.getNodeRefSize() && index.getNodeLinkSize() &&
            index.getLabelIndexSize());
  }
  bool Passes(Label *label);
  MLabel* At(Label *label);
  void DefTime(Periods *per);

  MLabelIndex index;
};

class ULabel : public UString {
 public:
  ULabel() {}
  ULabel(int i): UString(i) {}
  ULabel(const Interval<Instant>& interval, const Label& label);

  static const string BasicType() {return "ulabel";}
  static ListExpr ULabelProperty();
  static bool CheckULabel(ListExpr type, ListExpr& errorInfo);
  static bool checkType(ListExpr t) {
    return listutils::isSymbol(t, BasicType());
  }

  void Initial(ILabel *result);
  void Final(ILabel *result);
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
  static Pattern* getPattern(string input, bool classify = false);
  ExtBool matches(MLabel *ml);

  int getResultPos(const string v);
  void collectAssVars();
  void addVarPos(string var, int pos);
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
          set<int> &finalStates, set<int> &states, vector<Condition> &easyConds,
          map<int, set<int> > &easyCondPos, bool store = false);
  bool easyCondsMatch(int ulId, int pId, PatElem const &up,
                      vector<Condition> &easyConds, set<int> &pos);
  string states2Str(int ulId, set<int> &states);
  string matchings2Str(unsigned int dim1, unsigned int dim2);
  bool findMatchingBinding(vector<map<int, int> > &nfa, int startState,
                           vector<PatElem> &elems, vector<Condition> &conds);
  bool findBinding(unsigned int ulId, unsigned int pId,
                   vector<PatElem> &elems, vector<Condition> &conds,
                   map<string, pair<unsigned int, unsigned int> > &binding);
  void cleanPaths();
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
  BindingStackElem(unsigned int ul, unsigned int pe) : ulId(ul), pId(pe) {}

  unsigned int ulId, pId;
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
  bool streamOpen;
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
  IndexMatchesLI(Word _mlrel, InvertedFile *inv, int _attrNr, Pattern *p);

  ~IndexMatchesLI() {}

  Tuple* nextTuple();
};

class UnitsLI {
 public:
  UnitsLI(MLabel *source) : ml(source), index(0) {}
  ~UnitsLI() {}
  
  ULabel* getNextUnit();

 private:
  MLabel* ml;
  int index;
};

}
