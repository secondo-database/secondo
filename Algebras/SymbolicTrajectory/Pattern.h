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
    static const string BasicType() {
      return "mlabel";
    }
    static ListExpr MLabelProperty();
    static bool CheckMLabel(ListExpr type, ListExpr& errorInfo);
    void compress();
    void rewrite(MLabel const &ml, vector<size_t> seq, vector<UPat> assigns);
};

class ULabel : public UString {
  public:
    ULabel() {}
    ULabel(int i): UString(i){}
    static const string BasicType() {
      return "ulabel";
    }
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
 public:
  string text;
  string textSubst; // the condition after replacing, see maps below
  map<int, string> types;
  map<int, string> subst;
  vector<Key> keys;
  vector<string> vars;
  vector<int> pIds;
  set<string> falseExprs; // strings whose evaluation yields a negative/positive
  set<string> trueExprs; // result are stored in these sets

  Condition() {
    types[0] = ".label";
    types[1] = ".time";
    types[2] = ".start";
    types[3] = ".end";
    types[4] = ".card";
    types[5] = ".ERROR";
    subst[0] = "\"a\"";
    subst[1] = "[const periods value ((\"2003-11-20-07:01:40\" "
               "\"2003-11-20-07:45\" TRUE TRUE))]";
    subst[2] = "[const instant value \"1909-12-19\"]";
    subst[3] = "[const instant value \"2012-05-12\"]";
    subst[4] = "1";
    subst[5] = "";
  }

  ~Condition() {}
  
  void toString();
  Key convertVarKey(const char *varKey);
  void clear();
  void substitute();
  void substitute(unsigned int pos, string subst);
};

class UPat {
 public:
  string var;
  set<string> ivs;
  set<string> lbs;
  Wildcard wc;

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
};

class Pattern {
 public:
  vector<UPat> patterns;
  vector<UPat> results;
  vector<Condition> conds;
  string text;

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
};

class NFA {
 private:
  set<int> **transitions; // 1st coord: old state; 2nd coord: unit pattern id;
                          // contents: new state(s).
  set<int> currentStates;
  vector<UPat> patterns;
  vector<Condition> conds;
  //bool *relevantPos; // stores the positions of unit patterns with a var
  int numOfStates;
  ULabel ul;
  size_t ulId, maxLabelId;
  set<size_t> *matchings;
  set<size_t> *cardsets;
  set<vector<size_t> > sequences; // all possible matching sequences
  set<vector<size_t> > condMatchings; // for condition evaluation
  set<vector<size_t> > rewriteSeqs; // matching sequences for rewriting
  vector<int> resultVars; // [3, 1] means: 1st result var is the one from the
                          // 3rd up, 2nd result var is the one from the 1st up

 public:
  NFA(const int size) {
    numOfStates = size;
    transitions = new set<int>*[numOfStates];
    //relevantPos = new bool[numOfStates - 1];
    for (int i = 0; i < numOfStates - 1; i++) {
      transitions[i] = new set<int>[numOfStates];
      //relevantPos[i] = false;
    }
    transitions[numOfStates - 1] = new set<int>[numOfStates];
    currentStates.insert(0);
    matchings = new set<size_t>[numOfStates - 1];
    cardsets = new set<size_t>[numOfStates - 1];
  }

  ~NFA() {
    for (int i = 0; i < numOfStates; i++) {
      delete[] transitions[i];
    }
    delete[] transitions;
    delete[] matchings;
    delete[] cardsets;
    //delete[] relevantPos;
  }

  void buildNFA(Pattern p);
  bool match(MLabel const &ml, bool rewrite);
  void printCurrentStates();
  void printCards();
  void printSequences(size_t max);
  void printRewriteSequences(size_t max);
  void printCondMatchings(size_t max);
  void updateStates();
  void storeMatch(int state);
  bool labelsMatch(int pos);
  bool timesMatch(int pos);
  void buildSequences();
  void filterSequences(MLabel const &ml);
  void buildRewriteSequence(vector<size_t> sequence);
  void computeResultVars(vector<UPat> results);
  set<vector<size_t> > getRewriteSequences();
  bool conditionsMatch(MLabel const &ml);
  void buildCondMatchings(unsigned int condId, vector<size_t> sequence);
  bool evaluateCond(MLabel const &ml, unsigned int condId,
                    vector<size_t> sequence);
  string getLabelSubst(MLabel const &ml, unsigned int pos);
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

  bool finished() {
    return (it == sequences.end());
  }

  MLabel getML() {
    return *inputML;
  }

  vector<size_t> getCurrentSeq() {
    return *it;
  }

  vector<UPat> getAssignments() {
    return assigns;
  }

  void next() {
    it++;
  }
};

}
