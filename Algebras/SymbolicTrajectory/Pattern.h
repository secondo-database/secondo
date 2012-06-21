/*

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

enum Key {LABEL, TIME, START, END, CARD, ERROR};
enum Wildcard {NO, ASTERISK, PLUS};

Pattern* parseString(const char* input);
bool evaluate(string condStr, const bool eval);

class MLabel : public MString {
  public:
    static const string BasicType() {
      return "mlabel";
    }
    static ListExpr MLabelProperty();
    static bool CheckMLabel(ListExpr type, ListExpr& errorInfo);
};

class ULabel : public UString {
  public:
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

  Condition() {
    types[0] = ".label";
    types[1] = ".time";
    types[2] = ".start";
    types[3] = ".end";
    types[4] = ".card";
    types[5] = ".ERROR";
    subst[0] = "\"a\"";
    subst[1] = "const periods value ((\"2003-11-20-07:01:40\" "
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
};

class UnitPattern {
 public:
  string var;
  set<string> ivs;
  set<string> lbs;
  string wc;

  UnitPattern() {}

  ~UnitPattern() {}

  UnitPattern(const string v, const string i, const string l, const string w) {
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
  vector<UnitPattern> patterns;
  vector<UnitPattern> results;
  vector<UnitPattern> assigns;
  vector<Condition> conds;
  string text;

  Pattern() {}

  Pattern(const Pattern& rhs) {
    patterns = rhs.patterns;
    results = rhs.results;
    assigns = rhs.assigns;
    conds = rhs.conds;
  }

  Pattern& operator=(const Pattern& rhs){
    patterns = rhs.patterns;
    results = rhs.results;
    assigns = rhs.assigns;
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
  void verifyConditions();
  static Pattern* getPattern(string input);
  bool matches(MLabel const &ml);
  bool verifyIntervals();
};

class NFA {
 private:
  set<int> **transitions; // 1st coord: old state; 2nd coord: unit pattern id;
                          // contents: new state(s).
  set<int> currentStates;
  vector<UnitPattern> patterns;
  vector<Condition> conds;
  int numOfStates;
  ULabel ul;
  size_t ulId, maxLabelId;
  set<size_t> *matchings;
  set<size_t> *cardsets;
  set<vector<size_t> > sequences; // all possible matching sequences
  set<vector<size_t> > condMatchings; // for condition evaluation

 public:
  NFA(const int size) {
    numOfStates = size;
    transitions = new set<int>*[numOfStates];
    for (int i = 0; i < numOfStates; i++) {
      transitions[i] = new set<int>[numOfStates];
    }
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
  }

  void buildNFA(Pattern p);
  bool match(MLabel const &ml);
  void printCurrentStates();
  void printCards();
  void printSequences(size_t max);
  void printCondMatchings(size_t max);
  void updateStates();
  void storeMatch(int state);
  bool labelsMatch(int pos);
  bool timesMatch(int pos);
  void buildSequences();
  bool conditionsMatch(MLabel const &ml);
  void buildCondMatchings(unsigned int condId, vector<size_t> sequence);
  bool evaluateCond(MLabel const &ml, unsigned int condId,
                    vector<size_t> sequence);
  string substituteCond(unsigned int condId, unsigned int pos, string subst);
  string getNextSubst(MLabel const &ml, Key key, unsigned int pos);
  string toString();
};

}
