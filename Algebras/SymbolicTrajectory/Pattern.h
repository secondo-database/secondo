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
#include <string>
#include <set>

#define YYDEBUG 1
#define YYERROR_VERBOSE 1

using namespace std;

union Word;

namespace stj {

class Pattern;
class UPat;
class Assign;
class ClassifyLI;
class Label;

enum Wildcard {NO, STAR, PLUS};

Pattern* parseString(const char* input, bool classify);
void patternFlushBuffer();

class MLabel : public MString {
 public:
  MLabel() {}
  MLabel(int i): MString(i) {}
  
  ~MLabel() {}

  static const string BasicType() {return "mlabel";}
  static bool checkType(ListExpr t) {
    return listutils::isSymbol(t, BasicType());
  }
  static ListExpr MLabelProperty();
  static bool CheckMLabel(ListExpr type, ListExpr& errorInfo);
//   int NumOfFLOBs() const;
//   Flob* GetFLOB(const int i);
//   CharLink getCharLink(const int index) const;
//   int getPosition(const int index) const;
//   void destroyIndex();
//   bool indexExists() {return hasIndex;}
//   bool buildIndex(MLabel const &source);
  MLabel* compress();
  void create(int size, bool text, double rate);
  void rewrite(MLabel const &ml,
               const pair<vector<size_t>, vector<size_t> > &seq,
               vector<Assign> assigns, map<string, int> varPos);

//  private:
//   bool hasIndex;
//   DbArray<CharLink> links;
//   DbArray<int> positions;
};

class ULabel : public UString {
 public:
  ULabel() {}
  ULabel(int i): UString(i) {}
  
  static const string BasicType() {return "ulabel";}
  static ListExpr ULabelProperty();
  static bool CheckULabel(ListExpr type, ListExpr& errorInfo);
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
  string textSubst; // the condition after replacing, see maps below
  map<int, string> subst;
  vector<int> keys;
  vector<string> vars;
  vector<int> pIds;
  pair<QueryProcessor*, OpTree> opTree;
  vector<Attribute*> pointers; // for each expression like X.card

 public:
  Condition() {pointers.clear();}
  ~Condition() {}
  
  string toString() const;
  int convertVarKey(const char *varKey);
  void clear();
  void substitute();
  void substitute(int pos, string subst);
  static string getType(int t);
  static string getSubst(int s);
  
  string  getText() const          {return text;}
  void    setText(string newText)  {text = newText;}
  string  getSubst() const         {return textSubst;}
  void    resetSubst()             {textSubst = text;}
  void    setSubst(string newSub)  {textSubst = newSub;}
  int     getKeysSize() const      {return keys.size();}
  int     getKey(unsigned int pos) {return (pos < keys.size() ?
                                                  keys[pos] : -1);}
  int     getPId(unsigned int pos) {return (pos < pIds.size() ?
                                                  pIds[pos] : -1);}
  void    clearVectors()           {vars.clear(); keys.clear(); pIds.clear();}
  string  getVar(unsigned int pos) {return (pos < vars.size() ?
                                                  vars[pos] : "");}
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
  QueryProcessor* getQP()          {return opTree.first;}
  OpTree  getOpTree()              {return opTree.second;}
  void    deleteOpTree()           {if (opTree.first) {
                                     opTree.first->Destroy(opTree.second, true);
                                     delete opTree.first;}}
  void    deletePointers() {for (unsigned int i = 0; i < pointers.size(); i++) {
                              deleteIfAllowed(pointers[i]);
                           }}
};

class UPat {
 private:
  string var;
  set<string> ivs;
  set<string> lbs;
  Wildcard wc;

 public:
  UPat() {}
  ~UPat() {}

  UPat(const string v, const string i, const string l, const Wildcard w){
    var = v;
    ivs = stringToSet(i);
    lbs = stringToSet(l);
    wc = w;
  }

  void setUnit(const char *v, const char *i, const char *l, const char *w);
  void getUnit(const char *v, bool assignment);
  void createUnit(const char *v, const char *pat);

  string      getV() const                {return var;}
  set<string> getL() const                {return lbs;}
  set<string> getI() const                {return ivs;}
  Wildcard    getW() const                {return wc;}
  void        clearL()                    {lbs.clear();}
  void        insertL(string newLabel)    {lbs.insert(newLabel);}
  void        clearI()                    {ivs.clear();}
  void        insertI(string newInterval) {ivs.insert(newInterval);}
  void        clearW()                    {wc = NO;}
};

class Assign {
 private:
  int resultPos;
  int patternPos; // -1 if ~var~ does not occur in the pattern
  string text[4]; // one for label, one for time, one for start, one for end
  string textSubst[4];
  string var;
  vector<pair<string, int> > right[5]; // a list of vars and keys for every type
  //pair<QueryProcessor*, OpTree> opTree[4];
  //vector<Attribute*> pointers[4]; // for each expression like X.card

 public:
  Assign() {}
  ~Assign() {}

  void clear() {
    resultPos = -1;
    for (int i = 0; i < 4; i++) {
      text[i].clear(); textSubst[i].clear(); right[i].clear();
    }
  }

  static string getDataType(int key);
  void convertVarKey(const char* vk);
  void substitute(int key);
  bool prepareRewrite(int key, const vector<size_t> &assSeq,
                      map<string, int> &varPosInSeq, MLabel const &ml);
  bool initOpTrees();

  void    init(string v, int pp)             {clear(); var=v; patternPos=pp;}
  int     getResultPos() const               {return resultPos;}
  void    setResultPos(int p)                {resultPos = p;}
  int     getPatternPos() const              {return patternPos;}
  void    setPatternPos(int p)               {patternPos = p;}
  string  getText(int key) const             {return text[key];}
  void    setText(int key, string newText)   {if (!text[key].empty()) {
                                               right[key].clear();}
                                              text[key] = newText;}
  string  getSubst(int key) const            {return textSubst[key];}
  void    setSubst(int key, string newSubst) {textSubst[key] = newSubst;}
  int     getRightSize(int key) const        {return right[key].size();}
  string  getV() const                       {return var;}
  int     getRightKey(int lkey, int j) const {return right[lkey][j].second;}
  pair<string, int> getVarKey(int key, int i) const {return right[key][i];}
  pair<string, int> getVarKey(int key) const {return right[key].back();}
  string  getRightVar(int lkey, int j) const {return right[lkey][j].first;}
  void    addRight(int key,
               pair<string, int> newRight)   {right[key].push_back(newRight);}
  void    removeUnordered()                  {right[4].pop_back();}
};

class Pattern {
 private:
  vector<UPat> patterns;
  vector<Assign> assigns;
  vector<Condition> conds;
  string text, description;
  map<string, int> varPos;
  set<string> assignedVars; // variables on the right side of an assignment
  vector<map<int, set<int> > > delta; // vector pos: old state;
                        // map.first: unit pattern id; map.second: new state.
  bool verified;
 public:
  Pattern() {}

  Pattern(vector<UPat> ps, vector<Assign> as, vector<Condition> cs, string t,
          vector<map<int, set<int> > > d, bool v) {
    patterns = ps;
    assigns = as;
    conds = cs;
    text = t;
    delta = d;
    verified = v;
    collectAssVars();
    for (int i = 0; i < (int)patterns.size(); i++) {
      addVarPos(patterns[i].getV(), i);
    }
  }

  Pattern(const Pattern& rhs) {
    patterns = rhs.patterns;
    assigns = rhs.assigns;
    conds = rhs.conds;
    text = rhs.text;
    varPos = rhs.varPos;
    assignedVars = rhs.assignedVars;
    delta = rhs.delta;
    verified = rhs.verified;
    description = rhs.description;
  }

  Pattern& operator=(const Pattern& rhs){
    patterns = rhs.patterns;
    assigns = rhs.assigns;
    conds = rhs.conds;
    text = rhs.text;
    varPos = rhs.varPos;
    assignedVars = rhs.assignedVars;
    delta = rhs.delta;
    verified = rhs.verified;
    description = rhs.description;
    return (*this);
  }  

  ~Pattern() {}

  string toString() const;
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
  bool matches(MString const &ml) const;
  bool verifyPattern() const;
  set<pair<vector<size_t>, vector<size_t> > > getRewriteSeqs(MLabel const &ml);
  map<string, int> getVarPosInSeq();
  int getResultPos(const string v);
  void collectAssVars();
  int getPatternPos(const string v);
  bool checkAssignTypes();
  void buildNFA();
  string nfa2String() const;
  static pair<string, Attribute*> getPointer(int key);
  bool initAssignOpTrees();

  vector<UPat>      getPats()               {return patterns;}
  vector<Condition> getConds()              {return conds;}
  vector<Assign>&   getAssigns()            {return assigns;}
  UPat              getPat(int pos) const   {return patterns[pos];}
  Condition         getCond(int pos) const  {return conds[pos];}
  Assign           getAssign(int pos) const {return assigns[pos];}
  bool              hasAssigns()            {return !assigns.empty();}
  void              addUPat(UPat upat)      {patterns.push_back(upat);}
  void              addCond(Condition cond) {conds.push_back(cond);}
  void              addAssign(Assign ass)   {assigns.push_back(ass);}
  void              setText(string newText) {text = newText;}
  void       addVarPos(string var, int pos) {varPos[var] = pos;}
  int               getVarPos(string var)   {return varPos[var];}
  int               getSize()               {return patterns.size();}
  map<string, int>  getVarPos()             {return varPos;}
  void              insertAssVar(string v)  {assignedVars.insert(v);}
  set<string>       getAssVars()            {return assignedVars;}
  void setAssign(int posR, int posP, int key, string arg) {
            assigns[posR].setText(key, arg); assigns[posR].setPatternPos(posP);}
  void addAssignRight(int pos, int key, pair<string, int> varKey)
                                           {assigns[pos].addRight(key, varKey);}
  void        substAssign(int pos, int key) {assigns[pos].substitute(key);}
  bool              isVerified() const      {return verified;}
  void              setVerified(bool v)     {verified = v;}
  vector<map<int, set<int> > > getDelta() const     {return delta;}
  void              initDelta()             {map<int, set<int> > emptyMapping;
                         for (unsigned int i = 0; i <= patterns.size(); i++) {
                                               delta.push_back(emptyMapping);
                                             }
                                            }
  void              setDescr(string desc)   {description = desc;}
  string            getDescr()              {return description;}
};

struct DoubleParsInfo {
  int pId;
  size_t start;
  size_t length;
};

class Match {
 private:
  vector<map<int, set<int> > > delta;
  set<int> currentStates;
  vector<UPat> patterns;
  vector<Condition> conds;
  int maxCardPos, numOfW, f; // number of the final state
  ULabel ul;
  size_t ulId, numOfLabels, seqCounter, seqMax, numOfNegEvals;
  set<size_t> *match, *cardsets;
  set<multiset<size_t> > sequences; // all possible matching sequences
  set<pair<vector<size_t>, vector<size_t> > > rewriteSeqs; // rewrite sequences
  map<int, int> resultVars; // (0,3) means: 0th result var occurs in 3rd unit p.
  set<string> assignedVars;
  map<string, int> varPos, varPosInSeq;
  set<int> doublePars; // positions of nonempty patterns in double parentheses
  int *seqOrder;

 public:
  Match(const int size) {
    f = size - 1;
    numOfLabels = 0;
    currentStates.insert(0);
    map<int, set<int> > emptyMapping;
    for (int i = 0; i < f; i++) {
      delta.push_back(emptyMapping);
    }
    match = new set<size_t>[f];
    cardsets = new set<size_t>[f];
    seqOrder = new int[f];
  }

  ~Match() {
    delete[] match;
    delete[] cardsets;
    delete[] seqOrder;
    deleteOpTrees();
  }

  bool matches(MString const &ml, bool rewrite = false);
  void printCurrentStates();
  void printCards();
  void printSequences(size_t max);
  void printRewriteSeqs(size_t max);
  void updateStates();
  bool labelsMatch(int pos, ClassifyLI* c = 0, int pat = -1);
  bool timesMatch(int pos, ClassifyLI* c = 0, int pat = -1);
  void computeCardsets();
  void correctCardsets(int nonStars);
  void processDoublePars(int pos);
  bool checkDoublePars(multiset<size_t> sequence);
  void buildSequences(); // for rewrite, every sequence must be built
  multiset<size_t> getNextSeq(); // for matches, we just need the next sequence
  void filterSequences(MString const &ml);
  void buildRewriteSeq(multiset<size_t> sequence);
  void computeResultVars(vector<Assign> assigns);
  set<pair<vector<size_t>, vector<size_t> > > getRewriteSeqs()
                                              {return rewriteSeqs;}
  bool conditionsMatch(MString const &ml);
  void computeSeqOrder();
  size_t getRelevantCombs();
  bool isFixed(int pos, bool start);
  bool evaluateEmptyML();
  bool evaluateCond(MString const &ml, int cId, multiset<size_t> sequence);
  set<multiset<size_t> > getSequences() {return sequences;}
  set<string> getAssVars() {return assignedVars;}
  void setAssVars(set<string> aV) {assignedVars = aV;}
  void setVarPos(map<string, int> vP) {varPos = vP;}
  void setFinalState(int final) {f = final;}
  map<string, int>& getVarPosInSeq() {return varPosInSeq;}
  void copyFromPattern(Pattern p) {
    delta = p.getDelta();
    patterns = p.getPats();
    conds = p.getConds();
    assignedVars = p.getAssVars(); // TODO why is it empty?
    varPos = p.getVarPos(); // TODO why is it empty?
  }
  void resetStates() {
    currentStates.clear();
    currentStates.insert(0);
  }
  void buildMultiNFA(ClassifyLI* c);
  void printMultiNFA();
  vector<int> applyMultiNFA(ClassifyLI* c, bool rewrite = false);
  vector<int> applyConditions(ClassifyLI* c);
  void multiRewrite(ClassifyLI* c);
  pair<string, Attribute*> getPointer(int key);
  pair<QueryProcessor*, OpTree> processQueryStr(string query, string type);
  bool initCondOpTrees();
  void deleteOpTrees();
  void deletePointers();
};

class RewriteResult {
 private:
  set<pair<vector<size_t>, vector<size_t> > > sequences; // matching sequences
  set<pair<vector<size_t>, vector<size_t> > >::iterator it;
  MLabel *inputML;
  vector<Assign> assigns;
  map<string, int> varPosInSeq;
  
 public:
  RewriteResult(set<pair<vector<size_t>, vector<size_t> > > seqs, MLabel *ml,
                vector<Assign> assigns, map<string, int> vPIS){
    sequences = seqs;
    it = sequences.begin();
    inputML = ml;
    this->assigns = assigns;
    varPosInSeq = vPIS;
  }

  ~RewriteResult() {}

  bool initAssignOpTrees();

  bool             finished()       {return (it == sequences.end());}
  void             killMLabel()     {delete inputML; inputML = 0;}
  MLabel           getML()          {return *inputML;}
  vector<Assign>   getAssignments() {return assigns;}
  void             next()           {it++;}
  map<string, int> getVarPosInSeq() {return varPosInSeq;}
  pair<vector<size_t>, vector<size_t> > getCurrentSeq() {return *it;}
};

class ClassifyLI {

friend class Match;

public:
  ClassifyLI(Word _pstream, Word _mlstream);
  ClassifyLI(Word _pstream, Word _mlstream, bool rewrite); // dummy parameter

  ~ClassifyLI();

  static TupleType* getTupleType();
  Tuple* nextResultTuple();
  MLabel* nextResultML();
  void computeCardsets();
  void printMatches();

private:
  vector<Pattern*> pats;
  Stream<MLabel> mlStream;
  TupleType* classifyTT;
  map<int, int> start2pat, end2pat, pat2start;
  set<int> initialStates;
  vector<int> matched;
  int numOfStates;
  MLabel* currentML;
  Match* mainMatch;
  map<int, vector<set<size_t> > > matches;//pattern_id -> (upat -> set(ulabel))
  vector<MLabel*> rewritten;
};

}
