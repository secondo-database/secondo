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
#include <string>
#include <set>

#define YYDEBUG 1
#define YYERROR_VERBOSE 1

using namespace std;

union Word;

namespace stj {

class Pattern;
class UPat;

enum Key {LABEL, TIME, START, END, CARD, ERROR};
enum Wildcard {NO, STAR, PLUS};
//enum Operations {EQUALS, IN, CONTAINS};

Pattern* parseString(const char* input);
void patternFlushBuffer();

struct CharLink {
  CharLink() {}
  CharLink(char symbol, int cf, int ct, int f, int t) :
           c(symbol), charFrom(cf), charTo(ct), posFrom(f), posTo(t) {}

  void set(char symbol, int cf, int ct, int pf, int pt) {
    c = symbol;
    charFrom = cf;
    charTo = ct;
    posFrom = pf;
    posTo = pt;
  }

  char c;
  int charFrom;
  int charTo;
  int posFrom;
  int posTo;
};

class MLabel : public MString {
 public:
  MLabel() {}
  MLabel(int i): MString(i) {}
//   MLabel(int i): MString(i), hasIndex(false), links(i), positions(i) {}
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
  void rewrite(MLabel const &ml, pair<vector<size_t>, vector<size_t> > seq,
               vector<UPat> assigns, map<string, int> varPos);

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
  vector<Key> keys;
  vector<string> vars;
  vector<int> pIds;

 public:
  Condition() {}
  ~Condition() {}
  
  string toString() const;
  Key convertVarKey(const char *varKey);
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
  Key     getKey(unsigned int pos) {return (pos < keys.size() ?
                                                  keys[pos] : ERROR);}
  int     getPId(unsigned int pos) {return (pos < pIds.size() ?
                                                  pIds[pos] : -1);}
  void    clearVectors()           {vars.clear(); keys.clear(); pIds.clear();}
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

class Pattern {
 private:
  vector<UPat> patterns;
  vector<UPat> results;
  vector<Condition> conds;
  string text;
  map<string, int> varPos;
  set<string> assignedVars; // variables on the right side of an assignment

 public:
  Pattern() {}

  Pattern(const Pattern& rhs) {
    patterns = rhs.patterns;
    results = rhs.results;
    conds = rhs.conds;
  }

  Pattern& operator=(const Pattern& rhs){
    patterns = rhs.patterns;
    results = rhs.results;
    conds = rhs.conds;
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
  static bool     KindCheck(ListExpr type, ListExpr& errorInfo);
  static int      SizeOfObj();
  static ListExpr Property();
  // other functions
  static const string BasicType();
  static const bool checkType(const ListExpr type);
  bool verifyConditions();
  static Pattern* getPattern(string input);
  bool matches(MString const &ml);
  bool verifyPattern();
  set<pair<vector<size_t>, vector<size_t> > > getRewriteSeqs(MLabel const &ml);
  map<string, int> getVarPosInSeq();

  vector<UPat>      getPats()               {return patterns;}
  vector<Condition> getConds()              {return conds;}
  vector<UPat>      getResults()            {return results;}
  UPat              getPat(int pos)         {return patterns[pos];}
  UPat              getResult(int pos)      {return results[pos];}
  bool              hasResults()            {return !results.empty();}
  void              addUPat(UPat upat)      {patterns.push_back(upat);}
  void              addCond(Condition cond) {conds.push_back(cond);}
  void              addResult(UPat res)     {results.push_back(res);}
  void              setText(string newText) {text = newText;}
  void              clearResLbs(int pos)    {results[pos].clearL();}
  void      insertResLb(int pos, string lb) {results[pos].insertL(lb);}
  void              clearResIvs(int pos)    {results[pos].clearI();}
  void      insertResIv(int pos, string iv) {results[pos].insertI(iv);}
  void       addVarPos(char* var, int pos)  {varPos[convert(var)] = pos;}
  int               getVarPos(string var)   {return varPos[var];}
  int               getSize()               {return patterns.size();}
  map<string, int>  getVarPos()             {return varPos;}
  void              insertAssVar(string v)  {assignedVars.insert(v);}
  set<string>       getAssVars()            {return assignedVars;}
};

struct DoubleParsInfo {
  int pId;
  size_t start;
  size_t length;
};

class NFA {
 private:
  map<int, set<int> > *delta; // array pos: old state;
                           // first: unit pattern id; second: new state.
  set<int> currentStates;
  vector<UPat> patterns;
  vector<Condition> conds;
  int maxCardPos, f; // number of the final state
  ULabel ul;
  size_t ulId, numOfLabels, seqCounter, seqMax, numOfNegEvals;
  set<size_t> *match, *cardsets;
  set<multiset<size_t> > sequences; // all possible matching sequences
  set<vector<size_t> > condMatchings; // for condition evaluation
  set<pair<vector<size_t>, vector<size_t> > > rewriteSeqs; // rewrite sequences
  map<int, int> resultVars; // (0,3) means: 0th result var occurs in 3rd unit p.
  set<string> assignedVars;
  map<string, int> varPos, varPosInSeq;
  set<int> doublePars; // positions of nonempty patterns in double parentheses
  int *seqOrder;
  map<string, bool> knownEval; // condition evaluation history
  map<pair<size_t, size_t>, string> knownPers; // periods string history

 public:
  NFA(const int size) {
    f = size - 1;
    delta = new map<int, set<int> >[f];
    currentStates.insert(0);
    match = new set<size_t>[f];
    cardsets = new set<size_t>[f];
    seqOrder = new int[f];
  }

  ~NFA() {
    delete[] delta;
    delete[] match;
    delete[] cardsets;
    delete[] seqOrder;
  }

  void buildNFA(Pattern p);
  bool matches(MString const &ml, bool rewrite = false);
  void printCurrentStates();
  void printCards();
  void printSequences(size_t max);
  void printRewriteSeqs(size_t max);
  void printCondMatchings(size_t max);
  void updateStates();
  bool labelsMatch(int pos);
  bool timesMatch(int pos);
  void computeCardsets();
  void correctCardsets(int nonStars);
  void processDoublePars(int pos);
  bool checkDoublePars(multiset<size_t> sequence);
  void buildSequences(); // for rewrite, every sequence must be built
  multiset<size_t> getNextSeq(); // for matches, we just need the next sequence
  void filterSequences(MString const &ml);
  void buildRewriteSeq(multiset<size_t> sequence);
  void computeResultVars(vector<UPat> results);
  void computeAssignedVars(vector<UPat> results);
  set<pair<vector<size_t>, vector<size_t> > > getRewriteSeqs()
                                              {return rewriteSeqs;}
  bool conditionsMatch(MString const &ml);
  void computeSeqOrder();
  size_t getRelevantCombs();
  bool isFixed(int pos, bool start);
  bool evaluateEmptyML();
  void buildCondMatchings(int condId, multiset<size_t> sequence);
  bool evaluateCond(MString const &ml, int cId, multiset<size_t> sequence);
  string getLabelSubst(MString const &ml, int pos);
  string getTimeSubst(MString const &ml, Key key, size_t from, size_t to);
  set<multiset<size_t> > getSequences() {return sequences;}
  set<string> getAssVars() {return assignedVars;}
  void setAssVars(set<string> aV) {assignedVars = aV;}
  void setVarPos(map<string, int> vP) {varPos = vP;}
  map<string, int> getVarPosInSeq() {return varPosInSeq;}
  string toString();
  void copyFromPattern(Pattern p) {
    patterns = p.getPats();
    conds = p.getConds();
    assignedVars = p.getAssVars(); // TODO why is it empty?
    varPos = p.getVarPos(); // TODO why is it empty?
  }
  void resetStates() {
    currentStates.clear();
    currentStates.insert(0);
  }
};

class RewriteResult {
 private:
  set<pair<vector<size_t>, vector<size_t> > > sequences; // matching sequences
  set<pair<vector<size_t>, vector<size_t> > >::iterator it;
  MLabel *inputML;
  vector<UPat> assigns;
  map<string, int> varPosInSeq;
  
 public:
  RewriteResult(set<pair<vector<size_t>, vector<size_t> > > seqs, MLabel *ml,
                vector<UPat> assigns, map<string, int> vPIS){
    sequences = seqs;
    it = sequences.begin();
    inputML = ml;
    this->assigns = assigns;
    varPosInSeq = vPIS;
  }

  ~RewriteResult() {}

  bool             finished()       {return (it == sequences.end());}
  void             killMLabel()     {delete inputML; inputML = 0;}
  MLabel           getML()          {return *inputML;}
  vector<UPat>     getAssignments() {return assigns;}
  void             next()           {it++;}
  map<string, int> getVarPosInSeq() {return varPosInSeq;}
  pair<vector<size_t>, vector<size_t> > getCurrentSeq() {return *it;}
};

}
