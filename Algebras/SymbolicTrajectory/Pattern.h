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

class MLabel : public MString {
  public:
    MLabel() {}
    MLabel(int i): MString(i) {}
    
    static const string BasicType() {return "mlabel";}
    static bool checkType(ListExpr t) {
       return listutils::isSymbol(t,BasicType());
    }  
    static ListExpr MLabelProperty();
    static bool CheckMLabel(ListExpr type, ListExpr& errorInfo);
    MLabel* compress();
    void create(int size);
    void rewrite(MLabel const &ml, vector<size_t> seq, vector<UPat> assigns);
};

class ULabel : public UString {
  public:
    ULabel() {}
    ULabel(int i): UString(i){}
    
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
  
  string getText() const           {return text;}
  void   setText(string newText)   {text = newText;}
  string getSubst() const          {return textSubst;}
  void   resetSubst()              {textSubst = text;}
  void   setSubst(string newSubst) {textSubst = newSubst;}
  int    getKeysSize() const       {return keys.size();}
  Key    getKey(unsigned int pos)  {return keys[pos];}
  int    getPId(unsigned int pos)  {return pIds[pos];}
  void   clearVectors()            {vars.clear(); keys.clear(); pIds.clear();}
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
  bool matches(MLabel const &ml);
  bool verifyPattern();
  bool hasResults();
  set<vector<size_t> > getRewriteSequences(MLabel const &ml);

  vector<UPat>      getPats()               {return patterns;}
  vector<Condition> getConds()              {return conds;}
  vector<UPat>      getResults()            {return results;}
  UPat              getPat(int pos)         {return patterns[pos];}
  UPat              getResult(int pos)      {return results[pos];}
  void              addUPat(UPat upat)      {patterns.push_back(upat);}
  void              addCond(Condition cond) {conds.push_back(cond);}
  void              addResult(UPat res)     {results.push_back(res);}
  void              setText(string newText) {text = newText;}
  void              clearResLbs(int pos)    {results[pos].clearL();}
  void      insertResLb(int pos, string lb) {results[pos].insertL(lb);}
  void              clearResIvs(int pos)    {results[pos].clearI();}
  void      insertResIv(int pos, string iv) {results[pos].insertI(iv);}
};

struct DoubleParsInfo {
  int pId;
  size_t start;
  size_t length;
};

class NFA {
 private:
  set<int> **delta; // 1st coord: old state; 2nd coord: unit pattern id;
                    // set contents: new state(s).
  set<int> currentStates;
  vector<UPat> patterns;
  vector<Condition> conds;
  int maxCardPos, f; // number of the final state
  ULabel ul;
  size_t ulId, numOfLabels, seqCounter, seqMax;
  set<size_t> *matchings;
  set<size_t> *cardsets;
  set<multiset<size_t> > sequences; // all possible matching sequences
  set<vector<size_t> > condMatchings; // for condition evaluation
  set<vector<size_t> > rewriteSeqs; // matching sequences for rewriting
  vector<int> resultVars; // [3, 1] means: 1st result var is the one from the
                          // 3rd up, 2nd result var is the one from the 1st up
  set<int> doublePars; // positions of nonempty patterns in double parentheses
  map<string, bool> knownEval; // condition evaluation history

 public:
  NFA(const int size) {
    f = size - 1;
    delta = new set<int>*[f + 1];
    for (int i = 0; i < f; i++) {
      delta[i] = new set<int>[f + 1];
    }
    delta[f] = new set<int>[f + 1];
    currentStates.insert(0);
    matchings = new set<size_t>[f];
    cardsets = new set<size_t>[f];
  }

  ~NFA() {
    for (int i = 0; i <= f; i++) {
      delete[] delta[i];
    }
    delete[] delta;
    delete[] matchings;
    delete[] cardsets;
  }

  void buildNFA(Pattern p);
  bool match(MLabel const &ml, bool rewrite);
  void printCurrentStates();
  void printCards();
  void printSequences(size_t max);
  void printRewriteSequences(size_t max);
  void printCondMatchings(size_t max);
  void updateStates();
  bool labelsMatch(int pos);
  bool timesMatch(int pos);
  void computeCardsets();
  void processDoublePars(int pos);
  bool checkDoublePars(multiset<size_t> sequence);
  void buildSequences(); // for rewrite, every sequence must be built
  multiset<size_t> getNextSeq(); // for matches, we just need the next sequence
  void filterSequences(MLabel const &ml);
  void buildRewriteSequence(multiset<size_t> sequence);
  void computeResultVars(vector<UPat> results);
  set<vector<size_t> > getRewriteSequences();
  bool conditionsMatch(MLabel const &ml);
  bool evaluateEmptyML();
  void buildCondMatchings(int condId, multiset<size_t> sequence);
  bool evaluateCond(MLabel const &ml, int cId, multiset<size_t> sequence);
  string getLabelSubst(MLabel const &ml, int pos);
  string getTimeSubst(MLabel const &ml, Key key, size_t from, size_t to);
  string toString();
};

class RewriteResult {
 private:
  set<vector<size_t> > sequences; // all matching sequences
  set<vector<size_t> >::iterator it;
  MLabel *inputML;
  vector<UPat> assigns;
  
 public:
  RewriteResult(set<vector<size_t> > seqs, MLabel *ml, vector<UPat> assigns){
    sequences = seqs;
    it = sequences.begin();
    inputML = ml;
    this->assigns = assigns;
  }

  ~RewriteResult() {}

  bool           finished()       {return (it == sequences.end());}
  void           killMLabel()     {delete inputML; inputML = 0;}
  MLabel         getML()          {return *inputML;}
  vector<size_t> getCurrentSeq()  {return *it;}
  vector<UPat>   getAssignments() {return assigns;}
  void           next()           {it++;}
};

}
